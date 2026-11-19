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
// - The host can make a participant a viewer (sees everything, changes
//   nothing) and can assign rows to participants (only the owner may change
//   them); editors can claim rows for themselves. Both travel in the Roster
//   and the host rejects proposals that break them (viewer / assigned), see
//   CollabRoles.h. A participant's claims go when it leaves or becomes a
//   viewer, a row's when the row is deleted.
// - If the host goes away, another participant takes over (host
//   migration). The host keeps ranking who would (clients that have kept
//   up first, then the lowest participant id) and sends that order with
//   every ping; when it is lost, every client takes the first one still in
//   the session without asking anyone. That one listens where the others
//   will look; the others reconnect and say what they had applied. The
//   new host first fetches any commits a rejoiner had that it lacks (so no
//   commit anyone received is lost), then becomes host at that rev, keeping
//   the rev numbers, every undo stack and the commit log. Proposals the old
//   host never answered are proposed again; ones it decided whose answer
//   was lost are rejected (host_changed). A client that finds no new host
//   ends with host_lost and keeps its copy. CollabController drives this.
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
#include "CollabRoles.h"
#include "CollabTransport.h"
#include "render/SequenceCommands.h"

class SequenceElements;

namespace Collab {

class CollabClient;
struct HashJob;

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
    // Client: commits kept to hand to a new host that lacks them (host
    // migration).
    size_t clientLogLimit = 2000;
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
    bool hostIgnoresRoles = false;        // host: commit viewers' edits and edits to rows assigned to others
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
    // Everyone in the session, their roles or the row assignments changed
    // (host first; Assignments() has the assignments). Host and clients.
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
    // Client: the host is gone. True: someone takes over (host migration);
    // the client waits in Migrating for Rejoin / BeginSuccession instead of
    // ending. Never called while joining.
    std::function<bool()> onHostLost;
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
    // Undo / redo go to this participant's newest edit. While one of its
    // proposals is still undecided (a client's edits enter the undo stack
    // only when their commits come back), they wait for it (SubmitResult
    // deferred) and run once it is decided; an undo and a redo still
    // waiting cancel out.
    bool CanUndo() const { return !_undo.empty() || !PendingEditLabel().empty(); }
    bool CanRedo() const {
        return (!_queuedUndo.empty() && _queuedUndo.back() == EditKind::Undo) || (_queuedUndo.empty() && !UndoWaits() && !_redo.empty());
    }
    SubmitResult Undo();
    SubmitResult Redo();
    // The label of what Undo / Redo would revert (CommandLabel of the edit);
    // empty when there is nothing.
    std::string UndoLabel() const {
        std::string pending = _queuedUndo.empty() ? PendingEditLabel() : std::string();
        return !pending.empty() ? pending : _undo.empty() ? std::string() : _undo.back().label;
    }
    std::string RedoLabel() const { return _redo.empty() ? std::string() : _redo.back().label; }
    // The command Undo / Redo would propose, or nullptr.
    const SequenceCommand* NextUndoCommand(bool redo) const {
        const auto& l = redo ? _redo : _undo;
        return l.empty() ? nullptr : l.back().cmd.get();
    }
    // Everyone in the session as the host last listed them (host first).
    const std::vector<RosterEntry>& Roster() const { return _roster; }

    // ---- Roles and assignments (CollabRoles.h) ----
    ParticipantRole RoleOf(ParticipantId who) const { return RoleIn(_roster, who); }
    ParticipantRole LocalRole() const { return RoleOf(_participant); }
    // Every row assignment in force, as the host last listed them.
    const std::vector<Assignment>& Assignments() const { return _assignments; }
    // Assigns `element` to `owner` (kNoParticipant: releases it). The host
    // decides at once (false: `reason` / `text` say why). A client checks it
    // against what it knows and sends it: the host's Roster brings the
    // result, or a Notice why not.
    virtual bool Claim(SeqObjectId element, ParticipantId owner, std::string& reason, std::string& text) = 0;
    // Why the host would refuse this participant's `cmd` now (viewer /
    // assigned), as far as this participant knows.
    EditRefusal CheckOwnEdit(const SequenceCommand& cmd);
    // Host migration: who takes over if the host goes, best first, as the
    // host last said (empty: a host that doesn't migrate).
    const std::vector<ParticipantId>& Succession() const { return _succession; }
    // How many times this session's host has changed.
    uint64_t Epoch() const { return _epoch; }
    // The highest proposal number of `who` seen committed (or, for this
    // participant, dealt with when a snapshot was taken).
    uint64_t LastSeqOf(ParticipantId who) const {
        auto it = _lastSeq.find(who);
        return it == _lastSeq.end() ? 0 : it->second;
    }
    // Commits kept for resending (the host's log, or a client's recent ones).
    size_t LoggedCommits() const { return _log.size(); }
    // Files this participant has read to hash them (show-file checks).
    size_t HashedFiles() const { return _hashes.FilesRead(); }
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
    // The document changed outside the session and was put back (a local
    // edit taken back out): hash it afresh at the next commit.
    void ForgetHashes() { _hasher.InvalidateAll(); }

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
    struct CommitRecord {
        Message msg;
    };
    struct Touch {
        Rev rev = 0;
        ParticipantId author = kNoParticipant;
    };
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
    // A proposal of this participant's that will change its undo or redo
    // stack hasn't been decided yet.
    virtual bool UndoWaits() const { return false; }
    // The label of its newest undecided edit, empty when there is none.
    virtual std::string PendingEditLabel() const { return {}; }
    // Runs undo / redo that waited, as far as nothing is undecided.
    void RunQueuedUndo();
    SubmitResult DoUndo(bool redo);
    std::deque<EditKind> _queuedUndo;
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
    // Hooks every transport callback up to this object's handlers.
    void InstallHandlers();
    virtual void OnTransportMessage(PeerId from, const Message& m) = 0;
    virtual void OnTransportPeer(PeerId peer, PeerEvent e) = 0;
    virtual void OnTransportBlob(PeerId from, BlobId id, const std::string& path) = 0;
    virtual void OnBlobProgress(PeerId /*from*/, BlobId /*id*/, uint64_t /*received*/, uint64_t /*size*/) {}
    virtual void OnBlobDone(PeerId /*to*/, BlobId /*id*/, bool /*sent*/) {}
    virtual void OnSendFailed(PeerId to, const Message& m, const std::string& reason) = 0;
    // A snapshot or welcome blob: the parts, each a length and the bytes.
    static bool WriteParts(const std::string& path, const std::vector<const std::string*>& parts);
    static bool ReadParts(const std::string& path, std::vector<std::string>& parts);

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
    std::vector<Assignment> _assignments;
    ParticipantRole _joinRole = ParticipantRole::Editor; // the host's, as it last said
    OwnMeta _submitMeta; // for the proposal SubmitKind is about to number
    std::map<uint64_t, OwnMeta> _ownMeta;

    void SetLayout(const LayoutFiles& files);
    bool _liveLayout = false;
    LayoutFiles _layout;
    uint64_t _layoutHash = 0;

    // What a successor needs to carry on, kept by every participant.
    std::unordered_map<std::string, Touch> _touch;               // conflict item -> last commit touching it
    std::map<Rev, std::pair<ParticipantId, EditKind>> _revs;     // rev -> author, kind
    std::map<ParticipantId, uint64_t> _lastSeq;                  // author -> highest clientSeq committed
    std::deque<CommitRecord> _log;                               // commits after _logBase
    Rev _logBase = 0;
    std::vector<ParticipantId> _succession;
    uint64_t _epoch = 0;
    ParticipantId _nextParticipant = kHostParticipant + 1;
    void NoteCommitted(ParticipantId author, uint64_t clientSeq);
    // Host migration: moves everything a participant carries across a role
    // change from `from` (which then owns nothing: no session folder, no
    // transport handlers).
    void TakeStateFrom(CollabParticipant& from);

private:
    bool _ownsHandlers = true;
    std::string _sessionFolder;
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

    // Host migration: the participant `from` takes over as host, keeping its
    // participant id, document, undo stacks and commit log. `from` (in
    // Succeeding) is left with nothing and should be destroyed; then call
    // ResumeAsSuccessor. Transport handlers are this object's from here on.
    CollabHost(ICollabTransport& transport, ICollabDocument& doc, SessionOptions opts, SessionEvents events, CollabClient& from);

    // Opens the session: from now on clients may join.
    void Start();
    // Host migration: opens the session as the successor. Proposes this
    // participant's own unanswered proposals again, then welcomes everyone
    // who reconnected while it was taking over.
    void ResumeAsSuccessor();
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
    // Makes a client a viewer or an editor again. A new viewer's row
    // assignments are released. False for the host or an unknown id.
    bool SetRole(ParticipantId who, ParticipantRole role);
    // The role new joiners get (editor by default).
    void SetJoinRole(ParticipantRole role);
    ParticipantRole JoinRole() const { return _joinRole; }
    bool Claim(SeqObjectId element, ParticipantId owner, std::string& reason, std::string& text) override;

    // Every reject the host has sent: reason -> count.
    const std::map<std::string, size_t>& RejectCounts() const { return _rejectCounts; }
    // The first 10000 rejects: (reason, detail).
    const std::vector<std::pair<std::string, std::string>>& RejectLog() const { return _rejectLog; }
    // Host migration: who would take over now (also sent with every ping).
    std::vector<ParticipantId> RankSuccessors() const;

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
        ParticipantRole role = ParticipantRole::Editor;
        bool active = false; // has the document (its pongs say so)
        // Blobs on their way to it, so a repeated request doesn't start
        // another copy of the same thing.
        BlobId snapshotBlob = 0;
        Rev snapshotRev = 0;     // the last snapshot sent, and when
        uint64_t snapshotAt = 0;
        bool sentDocument = false; // past joining: a snapshot went to it (or it rejoined)
        BlobId welcomeBlob = 0;
        uint64_t welcomeAt = 0;
        std::map<std::string, std::pair<BlobId, uint64_t>> filesSending; // manifest path -> blob, when sent
        // Rejoin secret (only for those who joined this host; it goes in
        // their Welcome) and its digest (listed in the roster).
        std::string secret;
        std::string secretHash;
        std::shared_ptr<HashJob> manifestJob; // hashing its manifest on a worker thread
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
    void OnTransportMessage(PeerId from, const Message& m) override { OnMessage(from, m); }
    void OnTransportPeer(PeerId peer, PeerEvent e) override { OnPeer(peer, e); }
    void OnTransportBlob(PeerId from, BlobId id, const std::string& path) override;
    void OnBlobDone(PeerId to, BlobId id, bool sent) override;
    void OnSendFailed(PeerId to, const Message& m, const std::string& reason) override;
    void SendWelcome(Client& c, bool rejoin);
    bool SentMediaFor(const Client& c, const std::string& path) const;
    bool Stale(uint64_t at) const;
    void OnHello(PeerId from, const Message& m);
    void AddRejoiner(PeerId from, const Message& hello);
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
    void SendRosterTo(PeerId peer);
    bool ClaimAs(ParticipantId requester, SeqObjectId element, ParticipantId owner, std::string& reason, std::string& text);
    void OnClaim(Client& c, const Message& m);
    // Drops claims on rows that are gone (and every claim of `owner`).
    bool PruneAssignments(ParticipantId owner = kNoParticipant);
    void SendSnapshot(Client& c);
    void SendManifest(Client& c, bool rebuild);
    void OnNeedFiles(Client& c, const Message& m);
    void OnBlobReady(PeerId from, const Message& info, const std::string& path) override;
    void SendShared(const FileEntry& e, const std::string& local) override;
    void SendCommitsFrom(Client& c, Rev from);
    // `tell`: say so to the client (a Bye with `reason`), for a drop it can't
    // know about (a timeout).
    void DropClient(PeerId peer, const std::string& why, const std::string& tell = "");
    // Another host has taken over from this one (its clients moved on while
    // it was blocked): the session ends here and the clients follow.
    void StandDown(const std::string& why);
    void NotAClient(PeerId from);
    void TrimLog();
    Client* FindClient(PeerId peer);

    bool _started = false;
    uint64_t _nextOwnSeq = 1;
    std::map<PeerId, Client> _clients;
    std::map<std::string, size_t> _rejectCounts;
    std::vector<std::pair<std::string, std::string>> _rejectLog;
    UidRegistry _uids;
    std::map<Rev, LayoutFiles> _layoutAt; // the session layout as of each layout commit (and the start)
    std::deque<LayoutLogEntry> _layoutLog;
    std::map<ParticipantId, std::pair<std::string, uint64_t>> _locks; // layout lock object, order taken
    uint64_t _lockCounter = 0;
    // Host migration
    Rev _rankHead = 0;                  // the head at the last ping round
    std::vector<ParticipantId> _ranked; // the order last sent
    std::vector<Message> _ownPending;   // a successor's unanswered proposals, to process on resuming
    std::vector<std::pair<PeerId, Message>> _rejoinHellos; // who reconnected while it was taking over
    std::vector<RosterEntry> _formerRoster;   // the session as the old host last listed it (roles)
    uint64_t _lastTick = 0;
    // After this host was blocked for longer than its clients wait: until
    // then a client leaving because the host changed means it was replaced.
    uint64_t _replacedWindowUntil = 0;
    bool _hostChangedSaid = false; // since the last tick, a client left saying the host changed
    std::map<PeerId, uint64_t> _strangersTold; // peer -> when told it isn't in the session
    std::set<ParticipantId> _awaitingRejoin;  // listed then, not back yet: their claims go at _rejoinUntil
    uint64_t _rejoinUntil = 0;
};

class CollabClient : public CollabParticipant {
public:
    // Checking: comparing show files with the host's (and fetching them).
    // Host migration: Migrating (the host is gone; waiting to be told whom
    // to reach), Rejoining (reconnected, waiting for the new host's
    // welcome), Succeeding (taking over: collecting who reconnects).
    enum class State { Idle, Joining, Checking, AwaitingSnapshot, Active, Migrating, Rejoining, Succeeding, Ended };

    CollabClient(ICollabTransport& transport, ICollabDocument& doc, SessionOptions opts = {}, SessionEvents events = {});
    ~CollabClient() override;

    void Join(PeerId host);
    void Leave();
    State GetState() const { return _state; }
    // Has the document and takes edits (proposals made while the host is
    // changing wait and go to the new one).
    bool IsActive() const override {
        return _state == State::Active || _state == State::Migrating || _state == State::Rejoining || _state == State::Succeeding;
    }

    // ---- Host migration ----
    // The connection to the host is gone (the transport says so, or nothing
    // was heard for too long): Migrating if SessionEvents::onHostLost says
    // someone takes over, else ended (host_lost).
    void HostGone();
    // Reconnected to the new host at `host`: say who this is and what it had.
    void Rejoin(PeerId host);
    // This participant takes over: collect who reconnects (Rejoiners).
    void BeginSuccession();
    // Back to Migrating (taking over failed; try someone else).
    void AbandonSuccession();
    struct Rejoiner {
        PeerId peer = kNoPeer;
        ParticipantId id = kNoParticipant;
        Rev applied = 0;
    };
    std::vector<Rejoiner> Rejoiners() const;
    // Fetch the commits after this participant's applied rev from the
    // rejoiner at `peer`. CatchingUp() until they are in or it failed.
    void CatchUpFrom(PeerId peer);
    bool CatchingUp() const { return _catchUpFrom != kNoPeer && !_catchUpFailed; }
    bool CatchUpFailed() const { return _catchUpFailed; }
    void StopCatchUp() { _catchUpFrom = kNoPeer; }
    // No new host could be reached: ended (host_lost).
    void GiveUp() { EndSession(Reason::HostLost); }

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
    bool Claim(SeqObjectId element, ParticipantId owner, std::string& reason, std::string& text) override;
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
    // What proves this is the participant it says when it rejoins after a
    // host change (from the host's Welcome).
    const std::string& RejoinSecret() const { return _rejoinSecret; }

private:
    friend class CollabHost;
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
    bool UndoWaits() const override;
    std::string PendingEditLabel() const override;
    void OnMessage(PeerId from, const Message& m);
    void OnPeer(PeerId peer, PeerEvent e);
    void OnTransportMessage(PeerId from, const Message& m) override { OnMessage(from, m); }
    void OnTransportPeer(PeerId peer, PeerEvent e) override { OnPeer(peer, e); }
    void OnTransportBlob(PeerId from, BlobId id, const std::string& path) override;
    void OnBlobProgress(PeerId from, BlobId id, uint64_t received, uint64_t size) override;
    void OnSendFailed(PeerId to, const Message& m, const std::string& reason) override;
    void OnWelcome(const Message& m);
    // A transfer from the host moved recently: what it may carry is not
    // asked for again yet.
    bool TransferMoving() const { return _lastBlobProgress != 0 && _now - _lastBlobProgress < _opts.blobStallMs; }
    void OnSnapshot(const Message& m, const std::string& xsq);
    void OnManifest(const Message& m);
    void ContinueManifest();
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
    void SendRejoinHello();
    void OnRejoined(const Message& m);
    void DropDecided();
    void LogCommit(const Message& m);

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
    std::shared_ptr<HashJob> _checkJob; // hashing this device's files on a worker thread

    // Host migration
    std::map<PeerId, Message> _rejoinHellos; // Succeeding: who reconnected
    PeerId _catchUpFrom = kNoPeer;
    bool _catchUpFailed = false;
    bool _rejoinPending = false; // rejoined: drop proposals the old host decided once caught up
    std::string _rejoinSecret;
    Rev _rejoinRev = 0;
    uint64_t _rejoinSeq = 0;
};

} // namespace Collab
