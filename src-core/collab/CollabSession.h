#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Live editing of one sequence by several people.
//
// One peer is the host: it owns the open sequence, puts every edit in one
// order (revisions) and is the only one that saves. Clients propose edits
// (SequenceCommands) and apply the host's commits in revision order, so
// every peer runs the same commands, through the same Apply code, on the
// same document state.
//
// - Structural edits (adds, deletes, moves, layers, rows, media, batches)
//   wait for the host's commit before the client applies them.
// - Settings / palette key changes (IsOptimistic) apply on the client at
//   once. The client remembers, per (effect, key), the host's value and its
//   own newest pending write. Before applying any commit it puts the host's
//   values back, so every commit applies to exactly the host's state (and
//   yields the host's inverse and hash); afterwards it re-applies its
//   pending writes. A rejected write falls back to the host's value. The
//   result: last writer (in host order) wins per key, and a client's own
//   writes never flicker.
// - The host processes each client's proposals in the order the client
//   made them, deletes of something already deleted commit as no-ops, a key
//   write made before someone else changed that effect's type is rejected
//   (stale_type), and a batch is all-or-nothing.
// - Undo is per user: each participant undoes its own commits by proposing
//   their inverses, which the host refuses (changed_since) when someone
//   else has touched the same thing since.
// - Every commit carries a hash of the host's document; a client whose
//   document hashes differently asks for a fresh snapshot.
// - If the host goes away the session ends; clients keep their copy.
//
// Nothing here touches per-user state (selection, collapsed / hidden rows,
// views): commands don't carry it, and presence travels separately.
//
// All entry points must be called on one thread (the main thread).

#include <cstdint>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "CollabCommandInfo.h"
#include "CollabDocHash.h"
#include "CollabMessages.h"
#include "CollabTransport.h"
#include "render/SequenceCommands.h"

class SequenceElements;

namespace Collab {

// The document a participant edits. The session applies commands to
// Sequence() directly (not through CommandExecutor, so they don't enter the
// local UndoManager); the host app reacts to onCommit.
class ICollabDocument {
public:
    virtual ~ICollabDocument() = default;
    virtual SequenceElements& Sequence() = 0;
    // Host: the open sequence as .xsq text.
    virtual bool SaveXsq(std::string& out) = 0;
    // Client: replace the open sequence with this .xsq text. Sequence()
    // returns the new one afterwards.
    virtual bool LoadXsq(const std::string& xsq) = 0;
};

struct SessionOptions {
    std::string displayName;
    std::string appVersion;
    std::string deviceKind;
    uint64_t pingIntervalMs = 1000;
    uint64_t peerTimeoutMs = 15000; // nothing heard for this long: the peer is gone
    // A gap this long between two Ticks means this side was blocked (a long
    // save, a slow load): peers' silence during it isn't held against them.
    uint64_t stallGapMs = 5000;
    // Host: a client that has heard nothing since its snapshot was sent may
    // be loading it; it gets this much longer before it counts as gone.
    uint64_t snapshotLoadGraceMs = 120000;
    uint64_t retryMs = 300;         // resend Hello / proposals / resend requests
    size_t maxParticipants = 64;
    size_t commitLogLimit = 20000;  // commits kept for Resend beyond what every client has
    // Compare the incremental document hash with a full recompute after
    // every commit (slow; tests).
    bool verifyHashes = false;
    // How many recent commits a drift report lists.
    size_t driftLogCommands = 8;
};

// Deliberate faults, for tests that prove the checks catch them. Never set
// in the app.
struct SessionFaults {
    bool hostIgnoresClientOrder = false; // host: process proposals as they arrive
    bool clientAppliesOutOfOrder = false; // client: apply commits as they arrive
    bool clientSkipsRebase = false;       // client: don't restore host values around commits
    bool clientSharesHostIdPrefix = false; // client: allocate ids with the host's prefix
    bool clientIgnoresDrift = false;      // client: never compare hashes, skip commits that don't apply
};

struct CommitInfo {
    Rev rev = 0;
    ParticipantId author = kNoParticipant;
    uint64_t clientSeq = 0;
    EditKind kind = EditKind::Edit;
    Rev undoOf = 0;
};

struct SessionEvents {
    // After a commit was applied to the local document.
    std::function<void(const CommitInfo&, const SequenceCommand& applied)> onCommit;
    // One of this participant's proposals (or undo / redo) was not committed.
    // `by` is who changed the target, when known.
    std::function<void(uint64_t clientSeq, EditKind kind, const std::string& reason, ParticipantId by)> onReject;
    std::function<void(const std::string& reason)> onEnded;
    // The document was replaced by a fresh snapshot (client).
    std::function<void()> onResync;
    std::function<void(ParticipantId who, const PresenceInfo&)> onPresence;
    std::function<void(ParticipantId who, const std::string& name, bool joined)> onParticipant;
    std::function<void(const std::string& line)> onLog;
};

struct SubmitResult {
    bool ok = false;
    uint64_t clientSeq = 0; // the proposal's number (0 when nothing was sent)
    std::string reason;     // when !ok
    bool optimistic = false; // already applied locally (client)
    bool deferred = false;   // undo / redo: waits for an undecided proposal
};

class CollabParticipant {
public:
    CollabParticipant(ICollabTransport& transport, ICollabDocument& doc, SessionOptions opts, SessionEvents events);
    virtual ~CollabParticipant();
    CollabParticipant(const CollabParticipant&) = delete;
    CollabParticipant& operator=(const CollabParticipant&) = delete;

    ParticipantId Participant() const { return _participant; }
    // The last rev applied to the local document.
    Rev AppliedRev() const { return _applied; }
    virtual bool IsActive() const = 0;

    virtual SubmitResult Submit(std::unique_ptr<SequenceCommand> cmd) = 0;
    bool CanUndo() const { return !_undo.empty(); }
    bool CanRedo() const { return !_redo.empty(); }
    SubmitResult Undo();
    SubmitResult Redo();
    virtual void SetPresence(const PresenceInfo& p) = 0;
    virtual void Tick(uint64_t nowMs) = 0;

    SessionFaults faults;
    const std::string& EndReason() const { return _endReason; }
    size_t HashVerifyFailures() const { return _hashVerifyFailures; }

protected:
    struct UndoEntry {
        Rev rev = 0;
        std::unique_ptr<SequenceCommand> cmd;
    };
    virtual SubmitResult SubmitKind(std::unique_ptr<SequenceCommand> cmd, EditKind kind, Rev undoOf) = 0;
    // Undo bookkeeping for one of this participant's own commits.
    void RecordOwnCommit(EditKind kind, Rev rev, const SequenceCommand& inverse);
    uint64_t HashAfter(const SequenceCommand& cmd);
    void Log(const std::string& s);
    void NoteRecent(const CommitInfo& c, const nlohmann::json& cmd);
    std::string RecentCommands() const;

    ICollabTransport& _transport;
    ICollabDocument& _doc;
    SessionOptions _opts;
    SessionEvents _events;
    ParticipantId _participant = kNoParticipant;
    Rev _applied = 0;
    uint64_t _now = 0;
    std::vector<UndoEntry> _undo;
    std::vector<UndoEntry> _redo;
    DocHasher _hasher;
    std::string _endReason;
    size_t _hashVerifyFailures = 0;
    std::deque<std::string> _recent;
};

class CollabHost : public CollabParticipant {
public:
    CollabHost(ICollabTransport& transport, ICollabDocument& doc, SessionOptions opts = {}, SessionEvents events = {});
    ~CollabHost() override;

    // Opens the session: from now on clients may join.
    void Start();
    // Tells every client the session is over.
    void End();
    bool IsActive() const override { return _started && _endReason.empty(); }

    // The host's own edit: validated and committed at once.
    SubmitResult Submit(std::unique_ptr<SequenceCommand> cmd) override;
    void SetPresence(const PresenceInfo& p) override;
    void Tick(uint64_t nowMs) override;

    Rev HeadRev() const { return _applied; }
    size_t ClientCount() const;
    // Participant ids of the joined clients.
    std::vector<ParticipantId> Clients() const;
    // Every reject the host has sent: reason -> count.
    const std::map<std::string, size_t>& RejectCounts() const { return _rejectCounts; }
    // The first 10000 rejects: (reason, detail).
    const std::vector<std::pair<std::string, std::string>>& RejectLog() const { return _rejectLog; }

private:
    struct Client {
        PeerId peer = kNoPeer;
        ParticipantId id = kNoParticipant;
        std::string name;
        uint64_t nextSeq = 1;
        std::map<uint64_t, Message> early; // proposals ahead of nextSeq
        std::map<uint64_t, Message> rejects; // recent rejects, by clientSeq, for resending
        Rev applied = 0;
        uint64_t lastHeard = 0;
        uint64_t lastPing = 0;
        std::set<uint64_t> faultSeen; // SessionFaults::hostIgnoresClientOrder only
    };
    struct CommitRecord {
        Message msg;
    };
    struct Touch {
        Rev rev = 0;
        ParticipantId author = kNoParticipant;
    };
    struct Outcome {
        bool committed = false;
        Rev rev = 0;
        std::string reason;
        std::string text;
        ParticipantId by = kNoParticipant;
    };

    SubmitResult SubmitKind(std::unique_ptr<SequenceCommand> cmd, EditKind kind, Rev undoOf) override;
    void OnMessage(PeerId from, const Message& m);
    void OnPeer(PeerId peer, PeerEvent e);
    void OnHello(PeerId from, const Message& m);
    void OnPropose(Client& c, const Message& m);
    void ProcessProposal(Client& c, const Message& m);
    Outcome Process(ParticipantId author, uint64_t clientSeq, Rev baseRev, EditKind kind, Rev undoOf, const nlohmann::json& cmd);
    void SendSnapshot(Client& c);
    void SendCommitsFrom(Client& c, Rev from);
    void DropClient(PeerId peer, const std::string& why);
    void TrimLog();
    Client* FindClient(PeerId peer);

    bool _started = false;
    ParticipantId _nextParticipant = kHostParticipant + 1;
    uint64_t _nextOwnSeq = 1;
    std::map<PeerId, Client> _clients;
    std::deque<CommitRecord> _log; // commits after _logBase
    Rev _logBase = 0;
    std::unordered_map<std::string, Touch> _touch;
    std::map<Rev, std::pair<ParticipantId, EditKind>> _revs;
    std::map<std::string, size_t> _rejectCounts;
    std::vector<std::pair<std::string, std::string>> _rejectLog;
    UidRegistry _uids;
};

class CollabClient : public CollabParticipant {
public:
    enum class State { Idle, Joining, AwaitingSnapshot, Active, Ended };

    CollabClient(ICollabTransport& transport, ICollabDocument& doc, SessionOptions opts = {}, SessionEvents events = {});
    ~CollabClient() override;

    void Join(PeerId host);
    void Leave();
    State GetState() const { return _state; }
    bool IsActive() const override { return _state == State::Active; }

    // Optimistic edits apply locally at once; anything else is sent and
    // applies when its commit comes back.
    SubmitResult Submit(std::unique_ptr<SequenceCommand> cmd) override;
    void SetPresence(const PresenceInfo& p) override;
    void Tick(uint64_t nowMs) override;

    size_t PendingCount() const { return _pending.size(); }
    size_t OverlayCount() const { return _overlay.size(); }
    size_t ResyncCount() const { return _resyncs; }
    // Asks the host for a fresh snapshot (as on drift).
    void RequestResync(const std::string& why);

private:
    struct Pending {
        Message msg;
        uint64_t lastSent = 0;
    };
    // (effect, palette?, key)
    using OverlayKey = std::tuple<SeqObjectId, bool, std::string>;
    struct OverlayEntry {
        EffectLocator loc;
        std::optional<std::string> host;  // the value in host order
        std::optional<std::string> local; // this client's newest pending value
        uint64_t seq = 0;                 // the proposal that wrote `local`
    };

    SubmitResult SubmitKind(std::unique_ptr<SequenceCommand> cmd, EditKind kind, Rev undoOf) override;
    void OnMessage(PeerId from, const Message& m);
    void OnPeer(PeerId peer, PeerEvent e);
    void OnSnapshot(const Message& m);
    void OnCommit(const Message& m);
    void OnReject(const Message& m);
    void DrainCommits();
    bool ApplyCommit(const Message& m);
    void HandleReject(const Message& m);
    void DrainRejects();
    void LiftOverlay();
    void LowerOverlay();
    void SetKey(const EffectLocator& loc, bool palette, const std::string& key, const std::optional<std::string>& value);
    void Send(const Message& m);
    void EndSession(const std::string& reason);
    void RequestResend();

    State _state = State::Idle;
    PeerId _host = kNoPeer;
    uint64_t _nextSeq = 1;
    std::map<uint64_t, Pending> _pending;
    std::map<Rev, Message> _buffered;
    std::vector<Message> _queuedRejects;
    std::map<OverlayKey, OverlayEntry> _overlay;
    Rev _hostHead = 0;
    uint64_t _lastHeard = 0;
    uint64_t _lastHello = 0;
    uint64_t _lastSnapshotRequest = 0;
    uint64_t _lastResendRequest = 0;
    size_t _resyncs = 0;
};

} // namespace Collab
