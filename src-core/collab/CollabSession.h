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
// - Settings / palette key changes and effect moves (IsOptimisticEdit,
//   alone or as a batch of only those) apply on the client at
//   once. The client remembers, per (effect, key) and per moved effect, the
//   host's value and its own newest pending write. Before applying any commit it puts the host's
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
// - Joining: the host sends a manifest of the show files the session needs
//   (CollabManifest.h). A joiner whose files all match opens the session
//   against its own show folder. Otherwise, with SessionOptions::fetchFiles,
//   it fetches what differs and opens against a copy of the host's show in
//   its session folder; without it, a layout or audio difference refuses the
//   join (with a readable account of the difference) and media differences
//   are only reported. Then the host sends the document as a blob.
// - Media a participant adds by file reference during the session reaches
//   everyone: the author sends the file to the host, which keeps a copy in
//   its session folder and passes it on. No participant ever writes into a
//   show folder.
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
#include "CollabManifest.h"
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

    // The show folder the document's file references resolve against. Empty
    // when the document can't say: the session then checks no files.
    virtual std::string ShowFolder() { return {}; }
    // Host: the sequence's audio file and alternate tracks.
    virtual std::vector<std::string> AudioFiles() { return {}; }
    // Client: open later snapshots (LoadXsq) against `folder`, a copy of the
    // host's show the session made in its session folder, instead of the
    // user's own show folder. False when the document can't.
    virtual bool UseShowFolder(const std::string& /*folder*/) { return false; }
    // Also look for media in `folder` (files sent during the session), from
    // now on and across LoadXsq.
    virtual void AddMediaFolder(const std::string& /*folder*/) {}
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

    // Where the session may write: the copy of the host's show a joiner opens
    // against, files sent during the session, snapshots and blobs on their
    // way. Inside the app's own container, never a show folder. Each session
    // makes its own folder inside this one (empty: the temporary folder).
    // That folder is deleted when the session object goes; after
    // KeepSessionFiles, only its show copy (SessionShowFolder) stays.
    std::string sessionFolder;
    // Client: fetch the files that don't match the host's and open the
    // session against a copy of the host's show (see the top of this file).
    bool fetchFiles = false;
    uint64_t fileRetryMs = 10000;       // ask again for files that haven't come
    // A transfer from the host that has reported no progress this long has
    // stalled; until then nothing it may be carrying is asked for again.
    uint64_t blobStallMs = 5000;
    uint64_t maxFileBytes = 4ull << 30; // largest file accepted from another participant
    // Show files are hashed on the session thread when no more than this is
    // to be read (unchanged files are cached); more goes to a worker thread,
    // so the session keeps answering meanwhile.
    uint64_t syncHashBytes = 64ull << 20;
    // This app edits the layout live (see CollabLayout.h). The session does
    // when the host and the client both do.
    bool liveLayout = false;
};

// What a client found when it compared its show folder with the host's.
struct JoinReport {
    // The folder the session's sequence opens against: the client's own show
    // folder, or the copy of the host's show in the session folder.
    std::string showFolder;
    bool sessionCopy = false;
    std::vector<std::string> fetched;        // files copied from the host
    std::vector<std::string> missingMedia;   // media this device lacks: those effects render blank
    std::vector<std::string> differentMedia; // media that differs: those effects render differently
    // How this device's layout differs from the host's, when it does.
    std::string layoutDifference;
    // Why the join was refused; empty when it went ahead.
    std::string refusal;
};

// Deliberate faults, for tests that prove the checks catch them. Never set
// in the app.
struct SessionFaults {
    bool hostIgnoresClientOrder = false; // host: process proposals as they arrive
    bool clientAppliesOutOfOrder = false; // client: apply commits as they arrive
    bool clientSkipsRebase = false;       // client: don't restore host values around commits
    bool clientSharesHostIdPrefix = false; // client: allocate ids with the host's prefix
    bool clientIgnoresDrift = false;      // client: never compare hashes, skip commits that don't apply
    bool clientSkipsLayoutUpdate = false; // client: bring memory along but keep the old session layout
};

// A layout change the session asks the app to bring memory to.
struct LayoutCommitInfo {
    Rev rev = 0;
    ParticipantId author = kNoParticipant;
    uint64_t clientSeq = 0;
    // For the author: what of its proposal was dropped (text), else empty.
    std::string notice;
    // The layout of a snapshot, not a commit; `firstSnapshot` when joining
    // (there is no earlier session layout: memory is whatever was loaded).
    bool snapshot = false;
    bool firstSnapshot = false;
};

struct CommitInfo {
    Rev rev = 0;
    ParticipantId author = kNoParticipant;
    uint64_t clientSeq = 0;
    EditKind kind = EditKind::Edit;
    Rev undoOf = 0;
};

// The undo menu text for an edit: a batch's label, else what the command
// does ("Effect(s) Added", "Row(s) Deleted", ...).
std::string CommandLabel(const SequenceCommand& cmd);

struct SessionEvents {
    // After a commit was applied to the local document.
    std::function<void(const CommitInfo&, const SequenceCommand& applied)> onCommit;
    // The same, with the inverse Apply returned (what the commit replaced).
    std::function<void(const CommitInfo&, const SequenceCommand& applied, const SequenceCommand& inverse)> onCommitApplied;
    // Everyone in the session changed (host first). Both roles.
    std::function<void(const std::vector<RosterEntry>& roster)> onRoster;
    // One of this participant's proposals (or undo / redo) was not committed.
    // `by` is who changed the target, when known.
    std::function<void(uint64_t clientSeq, EditKind kind, const std::string& reason, ParticipantId by)> onReject;
    // Client: an edit already applied locally was taken back out (the host
    // rejected it): `applied` is what the document just had done to it and
    // `inverse` undoes that, as for onCommitApplied. Comes before onReject.
    std::function<void(const SequenceCommand& applied, const SequenceCommand& inverse)> onRolledBack;
    std::function<void(const std::string& reason)> onEnded;
    // The document was replaced by a fresh snapshot (client).
    std::function<void()> onResync;
    // `lockOrder`: when the host saw this participant take its layout lock
    // (the earliest holder of an object holds it); 0 = none.
    std::function<void(ParticipantId who, const PresenceInfo&, uint64_t lockOrder)> onPresence;
    std::function<void(ParticipantId who, const std::string& name, bool joined)> onParticipant;
    std::function<void(const std::string& line)> onLog;
    // Client: the show folder check is done (refused or going ahead).
    std::function<void(const JoinReport&)> onJoinChecked;
    // A file another participant added by reference arrived and is now in
    // `local` (effects using it should re-render).
    std::function<void(const std::string& reference, const std::string& local)> onFileArrived;

    // Live layout. Brings memory from the session layout `before` to `after`
    // (a layout commit, or a snapshot's layout before its sequence loads).
    // False: not now (the render did not stop); the session tries again.
    std::function<bool(const LayoutCommitInfo&, const LayoutFiles& before, const LayoutFiles& after)> onLayout;
    // Client: a snapshot's sequence was loaded after onLayout brought memory
    // to its layout (an app may have reloaded the layout with it).
    std::function<void()> onSnapshotLoaded;
    // Host: a client's layout proposal is about to be merged; the host's own
    // pending layout change goes first.
    std::function<void()> onBeforeLayoutProposal;
    // One of this participant's layout proposals was not committed.
    std::function<void(uint64_t clientSeq, const std::string& reason, const std::string& text)> onLayoutRejected;
    // Something for the user from the host (Reason::LockTaken, ...).
    std::function<void(const std::string& reason, const std::string& text)> onNotice;
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
    // Submit, and when it commits fold it into this participant's newest
    // undo entry if that entry is still the commit of its own proposal
    // `joinUndoTo` (one undo then reverts both; a slider drag sent as many
    // commits undoes as one step). The host checks such an undo against
    // everything after the first of them.
    SubmitResult SubmitJoined(std::unique_ptr<SequenceCommand> cmd, uint64_t joinUndoTo);
    bool CanUndo() const { return !_undo.empty(); }
    bool CanRedo() const { return !_redo.empty(); }
    SubmitResult Undo();
    SubmitResult Redo();
    // The label of what Undo / Redo would revert (CommandLabel of the edit);
    // empty when there is nothing.
    std::string UndoLabel() const { return _undo.empty() ? std::string() : _undo.back().label; }
    std::string RedoLabel() const { return _redo.empty() ? std::string() : _redo.back().label; }
    // The command Undo / Redo would propose, or nullptr.
    const SequenceCommand* NextUndoCommand(bool redo) const {
        const auto& l = redo ? _redo : _undo;
        return l.empty() ? nullptr : l.back().cmd.get();
    }
    // Everyone in the session as the host last listed them (host first).
    const std::vector<RosterEntry>& Roster() const { return _roster; }
    virtual void SetPresence(const PresenceInfo& p) = 0;
    virtual void Tick(uint64_t nowMs) = 0;

    SessionFaults faults;
    // Tick leaves what it would apply from held work (layout changes memory
    // could not take yet) to the caller, who runs ApplyHeld when the
    // document may change.
    bool deferHeldApply = false;
    virtual bool HeldApplyDue() const { return false; }
    virtual void ApplyHeld() {}
    const std::string& EndReason() const { return _endReason; }
    // A readable account of why, when there is one (a refused join).
    const std::string& EndText() const { return _endText; }
    size_t HashVerifyFailures() const { return _hashVerifyFailures; }

    // Sends the file `reference` names (as the document stores it, resolved
    // against this participant's show folder) to the other participants:
    // a client sends it to the host, which keeps a copy and passes it on.
    // For media added by file reference; Submit does it for PutMedia file
    // references. False when the file can't be found or read.
    bool ShareFile(const std::string& reference);
    // The session folder (created on first use).
    const std::string& SessionFolder();
    // Where session copies of show files live: SessionFolder()/show.
    std::string SessionShowFolder();
    // Keep what the session put in the session folder's show copy when this
    // object goes ("Save a copy of this show"); by default it is deleted.
    void KeepSessionFiles(bool keep) { _keepFiles = keep; }
    // Files other participants sent during the session: reference -> the
    // copy in the session folder.
    const std::map<std::string, std::string>& ReceivedFiles() const { return _received; }

    // ---- Live layout ----
    bool LiveLayout() const { return _liveLayout; }
    // The session layout: the files as of the last layout commit applied.
    const LayoutFiles& Layout() const { return _layout; }
    uint64_t LayoutHash() const { return _layoutHash; }

protected:
    struct UndoEntry {
        Rev rev = 0;
        std::unique_ptr<SequenceCommand> cmd;
        uint64_t seq = 0; // the proposal whose commit this is (the newest, when joined)
        std::string label;
    };
    // What a proposal carries for undo bookkeeping until it commits.
    struct OwnMeta {
        std::string label;   // undo / redo: the label of the entry reverted
        uint64_t joinTo = 0; // SubmitJoined
    };
    virtual SubmitResult SubmitKind(std::unique_ptr<SequenceCommand> cmd, EditKind kind, Rev undoOf) = 0;
    // Undo bookkeeping for one of this participant's own commits.
    void RecordOwnCommit(EditKind kind, Rev rev, uint64_t seq, const SequenceCommand& applied, const SequenceCommand& inverse);
    // SubmitKind calls this once it has numbered a proposal.
    void NoteOwnProposal(uint64_t seq);
    void NotifyCommit(const CommitInfo& info, const SequenceCommand& applied, const SequenceCommand& inverse);
    uint64_t HashAfter(const SequenceCommand& cmd);
    void Log(const std::string& s);
    void NoteRecent(const CommitInfo& c, const nlohmann::json& cmd);
    std::string RecentCommands() const;

    // Blobs and the messages naming them (File, Snapshot) come in either
    // order; OnBlobReady runs once both are here (path empty: the transfer
    // failed, or a File that says why nothing is coming). It owns the file.
    void OnBlobArrived(PeerId from, BlobId id, const std::string& path);
    void OnBlobInfo(PeerId from, const Message& m);
    virtual void OnBlobReady(PeerId from, const Message& info, const std::string& path) = 0;
    BlobId NextBlob() { return ++_nextBlob; }
    BlobId SendFile(PeerId to, const FileEntry& e, const std::string& local);
    void SendFileUnavailable(PeerId to, const std::string& path, const std::string& reason);
    // The folder this participant's document resolves files against now.
    virtual std::string CurrentShowFolder() { return _doc.ShowFolder(); }
    virtual void SendShared(const FileEntry& e, const std::string& local) = 0;
    void ShareCommandFiles(const SequenceCommand& cmd);
    enum class Stored { Stored, AlreadyHad, Shadowed, Bad };
    // A file sent during the session: check it, put it in the session show
    // copy unless this participant already has it, make the document see it.
    Stored StoreReceivedFile(const FileEntry& e, const std::string& path, bool docLoaded, bool addMediaFolder, std::string& local);
    void EnsureMediaFolder();
    std::string NewSessionFile(const std::string& sub, const std::string& ext = "");

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
    std::string _endText;
    size_t _hashVerifyFailures = 0;
    std::deque<std::string> _recent;
    HashCache _hashes;
    std::map<std::string, std::string> _received;
    bool _mediaFolderAdded = false;
    std::vector<RosterEntry> _roster;
    OwnMeta _submitMeta; // for the proposal SubmitKind is about to number
    std::map<uint64_t, OwnMeta> _ownMeta;

    void SetLayout(const LayoutFiles& files);
    bool _liveLayout = false;
    LayoutFiles _layout;
    uint64_t _layoutHash = 0;

private:
    std::string _sessionFolder;
    bool _ownsSessionFolder = false;
    bool _keepFiles = false;
    BlobId _nextBlob = 0;
    uint64_t _sessionFiles = 0;
    std::map<std::pair<PeerId, BlobId>, std::string> _blobFiles;
    std::map<std::pair<PeerId, BlobId>, Message> _blobInfos;
    std::deque<std::pair<PeerId, BlobId>> _blobFileOrder;
    std::deque<std::pair<PeerId, BlobId>> _blobInfoOrder;
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
    // The host's own edit that is already in its document (`applied`, undone
    // by `inverse`; ids filled in): committed as it is, without validating
    // or applying it again. For edits the app made through its
    // CommandExecutor.
    SubmitResult CommitLocal(std::unique_ptr<SequenceCommand> applied, std::unique_ptr<SequenceCommand> inverse, uint64_t joinUndoTo = 0);
    void SetPresence(const PresenceInfo& p) override;
    void Tick(uint64_t nowMs) override;
    bool HeldApplyDue() const override;
    void ApplyHeld() override;

    Rev HeadRev() const { return _applied; }
    size_t ClientCount() const;
    // Participant ids of the joined clients.
    std::vector<ParticipantId> Clients() const;
    // Live layout from now on, starting from `files` (what memory holds).
    void EnableLiveLayout(const LayoutFiles& files);
    // The host's own layout edit (Diff from Layout() to memory), already in
    // memory: committed as it is.
    SubmitResult CommitLayout(const nlohmann::json& delta);
    // Layout versions kept for merging proposals made against older revs.
    size_t LayoutVersionsKept() const { return _layoutAt.size(); }
    // Every reject the host has sent: reason -> count.
    const std::map<std::string, size_t>& RejectCounts() const { return _rejectCounts; }
    // The first 10000 rejects: (reason, detail).
    const std::vector<std::pair<std::string, std::string>>& RejectLog() const { return _rejectLog; }

private:
    struct Client {
        PeerId peer = kNoPeer;
        ParticipantId id = kNoParticipant;
        std::string name;
        std::string deviceKind;
        std::vector<FileEntry> manifest;
        std::map<std::string, std::string> files; // manifest path -> host's file
        uint64_t nextSeq = 1;
        std::map<uint64_t, Message> early; // proposals ahead of nextSeq
        std::map<uint64_t, Message> rejects; // recent rejects, by clientSeq, for resending
        Rev applied = 0;
        uint64_t lastHeard = 0;
        uint64_t lastPing = 0;
        std::set<uint64_t> faultSeen; // SessionFaults::hostIgnoresClientOrder only
        bool liveLayout = false;
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
        bool held = false; // a layout proposal memory could not take yet
    };
    struct LayoutLogEntry {
        Rev rev = 0;
        ParticipantId author = kNoParticipant;
        std::vector<std::string> touched; // LayoutDelta::Describe
    };

    SubmitResult SubmitKind(std::unique_ptr<SequenceCommand> cmd, EditKind kind, Rev undoOf) override;
    void OnMessage(PeerId from, const Message& m);
    void OnPeer(PeerId peer, PeerEvent e);
    void OnHello(PeerId from, const Message& m);
    void OnPropose(Client& c, const Message& m);
    void DrainProposals(PeerId peer);
    // False when the proposal is held (try again later).
    bool ProcessProposal(Client& c, const Message& m);
    Outcome ProcessLayout(Client& c, const Message& m);
    Rev CommitLayoutApplied(ParticipantId author, uint64_t clientSeq, const nlohmann::json& delta, const LayoutFiles& next, const std::string& notice);
    void PruneLayouts();
    std::string WhoDeleted(const std::string& record, Rev after) const;
    uint64_t StampLock(ParticipantId who, const LayoutLockInfo& lock);
    std::string NameOfParticipant(ParticipantId id) const;
    Outcome Process(ParticipantId author, uint64_t clientSeq, Rev baseRev, EditKind kind, Rev undoOf, const nlohmann::json& cmd);
    // Records and broadcasts a command already applied to the document.
    Rev CommitApplied(ParticipantId author, uint64_t clientSeq, EditKind kind, Rev undoOf, const SequenceCommand& cmd, const SequenceCommand& inverse,
                      bool lenient);
    void SendRoster();
    void SendSnapshot(Client& c);
    void SendManifest(Client& c, bool rebuild);
    void OnNeedFiles(Client& c, const Message& m);
    void OnBlobReady(PeerId from, const Message& info, const std::string& path) override;
    void SendShared(const FileEntry& e, const std::string& local) override;
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
    std::map<Rev, LayoutFiles> _layoutAt; // the session layout as of each layout commit (and the start)
    std::deque<LayoutLogEntry> _layoutLog;
    std::map<ParticipantId, std::pair<std::string, uint64_t>> _locks; // layout lock object, order taken
    uint64_t _lockCounter = 0;
};

class CollabClient : public CollabParticipant {
public:
    // Checking: comparing show files with the host's (and fetching them).
    enum class State { Idle, Joining, Checking, AwaitingSnapshot, Active, Ended };

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
    bool HeldApplyDue() const override { return _state == State::Active && _layoutHeld; }
    void ApplyHeld() override;

    // A layout edit (LayoutDelta from Layout() to memory), proposed like any
    // edit; the commit comes back through SessionEvents::onLayout.
    SubmitResult ProposeLayout(const nlohmann::json& delta);
    // The proposal is still waiting for the host.
    bool IsPending(uint64_t clientSeq) const { return _pending.count(clientSeq) != 0; }
    // Joining: this device's layout differs from the session's, so the
    // session opens against a copy (see JoinReport::sessionCopy).
    bool LayoutCopy() const { return _layoutCopy; }
    size_t PendingCount() const { return _pending.size(); }
    // This client's proposals still waiting for the host, oldest first.
    // Optimistic ones are already applied locally; the others will be
    // applied when they commit.
    struct PendingProposal {
        uint64_t clientSeq = 0;
        EditKind kind = EditKind::Edit;
        bool optimistic = false;
        nlohmann::json command;
    };
    std::vector<PendingProposal> PendingProposals() const;
    // Pending optimistic keys and moves.
    size_t OverlayCount() const { return _overlay.size() + _times.size(); }
    size_t ResyncCount() const { return _resyncs; }
    // Asks the host for a fresh snapshot (as on drift).
    void RequestResync(const std::string& why);
    const JoinReport& GetJoinReport() const { return _report; }
    // The manifest check: every file the host listed and how this device's
    // show folder compares.
    const std::vector<FileCheck>& FileChecks() const { return _checks; }

private:
    struct Pending {
        Message msg;
        uint64_t lastSent = 0;
        bool optimistic = false;
        bool withdrawn = false; // stands in for one too large to send (already rejected)
    };
    // (effect, palette?, key)
    using OverlayKey = std::tuple<SeqObjectId, bool, std::string>;
    struct OverlayEntry {
        EffectLocator loc;
        std::optional<std::string> host;  // the value in host order
        std::optional<std::string> local; // this client's newest pending value
        uint64_t seq = 0;                 // the proposal that wrote `local`
    };
    // An effect this client moved before the host committed it.
    struct TimesEntry {
        EffectLocator loc;
        std::pair<int, int> host;  // its times in host order
        std::pair<int, int> local; // this client's newest pending times
        uint64_t seq = 0;
    };

    SubmitResult SubmitKind(std::unique_ptr<SequenceCommand> cmd, EditKind kind, Rev undoOf) override;
    void OnMessage(PeerId from, const Message& m);
    void OnPeer(PeerId peer, PeerEvent e);
    void OnSnapshot(const Message& m, const std::string& xsq);
    void OnManifest(const Message& m);
    void RequestFiles();
    void OnJoinFile(const Message& info, const std::string& path);
    void FinishCheck();
    void RefuseJoin(const std::string& reason, const std::string& text);
    void AskForSnapshot();
    void OnBlobReady(PeerId from, const Message& info, const std::string& path) override;
    void SendShared(const FileEntry& e, const std::string& local) override;
    std::string CurrentShowFolder() override;
    void OnCommit(const Message& m);
    void OnReject(const Message& m);
    void DrainCommits();
    bool ApplyCommit(const Message& m);
    bool ApplyLayoutCommit(const Message& m);
    bool WriteSessionLayout(const LayoutFiles& files, std::string& error);
    void HandleReject(const Message& m);
    void DrainRejects();
    void LiftOverlay();
    void LowerOverlay();
    // Both return what they did and its inverse (nullptr when nothing applied).
    using Applied = std::pair<std::unique_ptr<SequenceCommand>, std::unique_ptr<SequenceCommand>>;
    Applied SetKey(const EffectLocator& loc, bool palette, const std::string& key, const std::optional<std::string>& value);
    Applied SetTimes(const EffectLocator& loc, std::pair<int, int> times);
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
    std::map<SeqObjectId, TimesEntry> _times;
    Rev _hostHead = 0;
    uint64_t _lastHeard = 0;
    uint64_t _lastHello = 0;
    uint64_t _lastSnapshotRequest = 0;
    uint64_t _lastResendRequest = 0;
    uint64_t _lastBlobProgress = 0;
    uint64_t _lastTick = 0;
    size_t _resyncs = 0;

    // Joining
    bool _haveManifest = false;
    std::vector<FileCheck> _checks;
    std::map<std::string, FileEntry> _wanted;     // path -> still to come from the host
    std::map<std::string, std::string> _fetched;  // path -> where it was put
    std::vector<std::string> _unavailable;        // paths the host couldn't send intact
    bool _compareOnly = false;                    // fetching layout files only to describe the difference
    bool _usingCopy = false;                      // opened against the session show copy
    bool _layoutCopy = false;                     // live layout: this device's layout differs from the session's
    bool _haveSnapshotLayout = false;             // live layout: _layout came with a snapshot
    bool _layoutHeld = false;                     // a layout commit waits until memory can take it
    uint64_t _lastFileRequest = 0;
    JoinReport _report;
};

} // namespace Collab
