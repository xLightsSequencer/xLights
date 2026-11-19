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

// The app-facing side of a live collaboration session: one CollabController
// per app (desktop frame / iPad document), the same on both. It owns the
// network transport and the session (CollabHost / CollabClient), routes the
// app's own edits and undo / redo into the session, applies other people's
// edits without the app's help, and tells the app what to redraw.
//
// ---------------------------------------------------------------------------
// Wiring it into an app
// ---------------------------------------------------------------------------
//
//   class MyCollabApp : public Collab::ICollabApp { ... };   // see ICollabApp
//   MyCollabApp hooks(...);
//   Collab::CollabController collab(hooks,
//                                   Collab::CollabNetFactory::Lan(dispatch),
//                                   dispatch,               // runs fn on the main thread soon
//                                   { appVersion, "mac" }); // ControllerOptions
//
//   // A main-thread timer, every 50-100 ms while Role() != None:
//   collab.Tick(nowMs);
//
//   if (!collab.Available()) hide the menu items (no LAN transport: Windows / Linux).
//
//   Host (needs an open sequence):
//     std::string code, error;
//     if (collab.Host(myName, sequenceFileName, code, error)) show `code`;
//     ... collab.End();    // ends it for everyone; then save as usual (see below)
//
//   Join:
//     collab.StartBrowsing();            // ICollabApp::OnSessionsFound / Sessions()
//     collab.Join(myName, advert.id, code, fetchFiles, error);
//     ... OnSessionStateChanged / StatusText() track Connecting -> Joining ->
//     Syncing -> Active; OnJoinReport says what differed / was fetched.
//     collab.Leave();
//
//   Remote (SharePlay), where GroupAvailable() after
//   collab.SetGroupNetwork(Collab::CollabNetFactory::Group(dispatch)):
//     collab.HostGroup(myName, sequenceFileName, error);  // Connecting -> Active
//     collab.JoinGroup(myName, collab.OfferedGroupSession().id, fetchFiles, error);
//     End / Leave and everything below work as on the LAN.
//
//   Presence: call SetLocalPresence whenever the visible time range, the
//   focused rows, the selection or the effect open in the settings panel /
//   inspector changes (cheap; the controller sends at most ~5 Hz, only
//   changes). Draw others from Participants(), PresenceOf(), SoftLockHolder()
//   and SelectedBy().
//
// Edits: nothing to do. Every edit the app makes through its sequence's
// CommandExecutor (Execute, batches, live preview, ExecuteInStep,
// RecordExternal, EditEffect, SequenceEditCapture, element / media / timing
// commands) reaches the session exactly once: the controller observes the
// executor and, at the next safe point (a Dispatcher turn, Tick, or before
// it handles anything from the network), sends everything recorded since as
// one edit. Edits recorded in the same UndoManager step across several turns
// (an EffectEditCoalescer slider drag) undo as one step.
//   - Host: its edits are committed as they are (they are already applied).
//   - Client: settings / palette key changes and effect moves (alone or as
//     a batch of only those, e.g. a live-preview drag) stay applied: the
//     session overlays them until the host's order is known, and a rejected
//     one falls back to the host's state. Anything structural (adds,
//     deletes, layers, rows, media, batches mixing those) is
//     taken back out of the local document and proposed; it reappears, with
//     its local selection, when the host commits it (a few ms on a LAN), or
//     the app gets OnRejected. Until then PendingGhosts() lists it for
//     drawing. The app gets OnSequenceChanged (cause OwnEditPending) when it
//     is taken out, so it can drop pointers to those effects.
// Edits made outside the CommandExecutor are not seen by the session (and
// will make peers drift until a re-sync).
//
// Undo: nothing to do. While a session runs the controller is installed as
// the sequence UndoManager's redirect, so UndoLastStep / RedoLastStep /
// CanUndo / CanRedo / GetUndoString / GetRedoString (every menu, shortcut and
// toolbar button) work on this user's own committed edits. The local undo
// history is cleared when a session starts and when it ends. A client's undo
// is proposed like any edit: it shows up when committed, or OnRejected says
// "Can't undo - that effect was changed by <name>". A client's structural
// edit can only be undone once the host has committed it.
//
// Remote edits: applied by the controller, inside
// CommandExecutor::ExternalApplyScope (so SequenceElements::IsEditInProgress()
// is true and the desktop doesn't yield to the event loop while they apply),
// with the render stopped first when the command adds, removes or renames
// rows. Network deliveries that would change the document wait while a local
// edit is open (a batch, a live-preview drag, an import capture, an undo
// replay) and run once it closes. Afterwards the app gets OnSequenceChanged
// with the rows, time range and effects to redraw / re-render; the host also
// gets MarkSequenceModified. Hooks are never called from inside an apply.
//
// Saving: only the host saves the sequence, through its normal save path,
// whenever it likes during the session and after End() (End() doesn't save;
// the document then holds every committed edit). A client never marks its
// copy modified and must not save over the sequence file (it may be the very
// file the host saves). After the session it may "Save a copy" (Save As);
// when it joined against a session copy of the host's show, call
// KeepSessionCopy() first so the copied show files outlive the session.
//
// Layout lock: ICollabApp::SetCollaborationLock(true, reason) is called when
// hosting starts or a join begins, and (false) when the session ends, is
// left, fails or the controller goes away.
//
// Re-entry: hooks such as LoadXsq may run a nested event loop (progress
// dialogs, render aborts, modal prompts). Anything the dispatcher, timer or
// transport delivers meanwhile (messages, peer events, blobs, browse results,
// Tick) is queued and runs once the outer call unwinds; it never re-enters
// the session. Calls the app makes from inside such a loop: local edits are
// recorded as usual and sent at the next safe point; Undo / Redo /
// PutOnNewLayer return false; Host / Join fail with an error; End / Leave are
// queued; SetLocalPresence is kept and sent later.
//
// All calls on the main thread.

#include <cstdint>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "CollabGroup.h"
#include "CollabLayout.h"
#include "CollabNet.h"
#include "CollabSession.h"
#include "render/CommandExecutor.h"
#include "render/UndoManager.h"
#include "utils/Color.h"

class SequenceElements;

namespace Collab {

// The hidden preference both apps read to show the collaboration menu items
// (default off).
inline constexpr const char* kCollaborationEnabledPref = "CollaborationEnabled";

// A participant's colour (presence bands, outlines, initials). Fixed per
// participant id, so every device shows the same colours; the host is
// participant 1.
xlColor ParticipantColour(ParticipantId id);

struct Participant {
    ParticipantId id = kNoParticipant;
    std::string name;
    std::string deviceKind; // "mac", "ipad", ...
    xlColor colour;
    bool isHost = false;
    bool isLocal = false; // this device
};

// What a UI can offer next to a reject message.
enum class RejectAction {
    None,
    PutOnNewLayer // CollabController::PutOnNewLayer(notice.id)
};

struct RejectText {
    std::string text;
    RejectAction action = RejectAction::None;
};

// Short user text for a reject reason (Reason::*, or a SubmitResult reason),
// shared by both UIs. `byName` is who changed the target, when known.
RejectText DescribeReject(const std::string& reason, EditKind kind, const std::string& byName);

struct RejectNotice {
    uint64_t id = 0; // the proposal (pass to PutOnNewLayer)
    EditKind kind = EditKind::Edit;
    std::string reason; // Reason::*
    ParticipantId by = kNoParticipant;
    std::string text; // DescribeReject
    RejectAction action = RejectAction::None;
};

// What changed in the document, for redrawing and re-rendering.
struct SequenceChange {
    enum class Cause {
        RemoteEdit,     // someone else's edit (or undo) was applied
        OwnEdit,        // this client's structural edit came back committed
        OwnUndo,        // this participant's undo / redo was applied
        OwnEditPending, // this client's structural edit was taken out until it commits
        Resync,         // the whole document was replaced (joined, or re-synced)
        FileArrived,    // a media file another participant added arrived
        // This client's edit won't be committed (the host rejected it, or it
        // was refused here) and what it changed was taken back out, or never
        // comes back: re-render what it touched.
        OwnEditRejected
    };
    Cause cause = Cause::RemoteEdit;
    ParticipantId author = kNoParticipant;
    std::string label; // the edit's undo label
    // Top-level rows (models, timing tracks) touched, and their names. A row
    // deleted by the change has a uid but may have no name.
    std::vector<SeqObjectId> rows;
    std::vector<std::string> rowNames;
    // Time range to re-render; -1 / -1 for the whole sequence.
    int startMS = -1;
    int endMS = -1;
    bool rowsAdded = false;
    bool rowsRemoved = false;
    bool rowsReordered = false;
    bool rowsRenamed = false;
    bool mediaChanged = false;
    bool everything = false; // treat as a newly opened sequence
    // Effects added, moved or changed (for a brief highlight); deleted ones
    // are included but no longer exist.
    std::vector<SeqObjectId> effects;
};

// One of this client's structural edits waiting for the host's commit.
struct PendingGhost {
    enum class Kind { Add, Move, Delete };
    Kind kind = Kind::Add;
    uint64_t proposal = 0;
    EffectLocator effect; // Delete / Move: an effect in the document; Add: where it will be
    std::string effectName;
    int startMS = 0; // Add / Move: where it will be; Delete: where it is
    int endMS = 0;
};

// What the app provides. The ICollabDocument part is what the session needs
// (Sequence / SaveXsq / LoadXsq / ShowFolder / AudioFiles / UseShowFolder /
// AddMediaFolder, see CollabSession.h); LoadXsq must replace the open
// sequence the way opening a file does, stopping the render first. The rest
// are notifications, always called outside session applies, on the main
// thread.
class ICollabApp : public ICollabDocument, public ICollabLayout {
public:
    // Sequence() can be called (false, e.g., on an iPad with no sequence
    // open yet: the controller then leaves it alone until LoadXsq).
    virtual bool HasSequence() { return true; }
    // xLightsShowContext::SetCollaborationLock.
    virtual void SetCollaborationLock(bool locked, const std::string& reason) = 0;

    virtual void OnSessionStateChanged() {}
    virtual void OnParticipantsChanged() {}
    virtual void OnPresenceChanged(ParticipantId /*who*/) {}
    virtual void OnSequenceChanged(const SequenceChange& /*change*/) {}
    // Host: the sequence has unsaved changes.
    virtual void MarkSequenceModified() {}
    virtual void OnRejected(const RejectNotice& /*notice*/) {}
    // Undo / redo availability or labels changed.
    virtual void OnUndoStateChanged() {}
    virtual void OnJoinReport(const JoinReport& /*report*/) {}
    virtual void OnSessionsFound(const std::vector<SessionAdvert>& /*sessions*/) {}
    virtual void OnBrowseError(const std::string& /*reason*/) {}
    virtual void OnLog(const std::string& /*line*/) {}
};

// How the controller reaches the network. Apps use Lan(); tests supply
// in-process transports.
struct CollabNetFactory {
    std::function<std::unique_ptr<INetTransport>()> transport;
    std::function<std::unique_ptr<ISessionBrowser>()> browser;
    std::function<bool()> available;
    std::function<std::string()> pairingCode;
    // Group networks only: a call / conversation is active now.
    std::function<bool()> inGroup;

    static CollabNetFactory Lan(Dispatcher dispatch) {
        CollabNetFactory f;
        f.transport = [dispatch] { return CreateLanTransport(dispatch); };
        f.browser = [dispatch] { return CreateLanBrowser(dispatch); };
        f.available = [] { return LanTransportAvailable(); };
        f.pairingCode = [] { return NewPairingCode(); };
        return f;
    }
    static CollabNetFactory Group(Dispatcher dispatch, IGroupSessionSource& source = PlatformGroupSessions()) {
        CollabNetFactory f;
        f.transport = [dispatch, &source] { return CreateGroupTransport(source, dispatch); };
        f.browser = [dispatch, &source] { return CreateGroupBrowser(source, dispatch); };
        f.available = [&source] { return source.Available(); };
        f.pairingCode = [] { return std::string(); };
        f.inGroup = [&source] { return source.InGroup(); };
        return f;
    }
};

struct ControllerOptions {
    std::string appVersion;
    std::string deviceKind; // "mac", "ipad", ...
    // Where a session may write (CollabSession.h SessionOptions::sessionFolder):
    // inside the app's container. Empty: a private temporary folder.
    std::string sessionFolder;
    uint64_t presenceIntervalMs = 200; // ~5 Hz
    uint64_t softLockIdleMs = 60000;
    uint64_t closeDelayMs = 2000; // keep the transport after the end so the goodbye gets out
    uint64_t layoutQuietMs = 250; // a layout change is proposed once memory has rested this long
    // Session timing and limits; displayName / appVersion / deviceKind /
    // sessionFolder / fetchFiles are filled in by the controller.
    SessionOptions session;
};

class CollabController : private ICommandObserver, private IUndoRedirect {
public:
    enum class Role { None, Host, Client };
    // Connecting: reaching the host. Joining: joined, comparing (and
    // fetching) show files. Syncing: receiving the sequence. Ended: over;
    // EndReason / EndText say why.
    enum class State { Idle, Connecting, Joining, Syncing, Active, Ended };

    CollabController(ICollabApp& app, CollabNetFactory net, Dispatcher dispatch, ControllerOptions opts = {});
    ~CollabController() override;
    CollabController(const CollabController&) = delete;
    CollabController& operator=(const CollabController&) = delete;

    // A transport exists on this platform.
    bool Available() const;

    // ---- Finding sessions ----
    void StartBrowsing();
    void StopBrowsing();
    const std::vector<SessionAdvert>& Sessions() const { return _sessions; }

    // ---- Remote sessions (SharePlay; see CollabGroup.h) ----
    // Which network a session runs on. Lan: the Host / Join below (Bonjour
    // and a pairing code). Group: a FaceTime call or Messages conversation,
    // no pairing code.
    enum class Network { Lan, Group };
    // Gives the controller the group network (CollabNetFactory::Group). Until
    // then GroupAvailable() is false. Call before hosting or joining.
    void SetGroupNetwork(CollabNetFactory group);
    bool GroupAvailable() const;
    // In a call / conversation now, so hosting reaches people at once
    // (otherwise the platform asks whom to invite).
    bool GroupInCall() const;
    // A session someone in the call started, waiting to be joined (id empty:
    // none). Reported through OnSessionsFound as well (SessionAdvert::group
    // set), from SetGroupNetwork on, whether or not StartBrowsing was called.
    SessionAdvert OfferedGroupSession() const;
    // Starts hosting the open sequence over the group. Asynchronous like
    // Join: watch GetState(); it is Active once the platform has started
    // the activity.
    bool HostGroup(const std::string& displayName, const std::string& sequenceName, std::string& error);
    // Joins the offered group session. Asynchronous: watch GetState().
    bool JoinGroup(const std::string& displayName, const std::string& advertId, bool fetchFiles, std::string& error);
    // The network the current (or last) session used.
    Network SessionNetwork() const;

    // ---- Lifecycle ----
    // Starts hosting the open sequence. On success `pairingCode` is the code
    // joiners type. False with `error` set otherwise.
    bool Host(const std::string& displayName, const std::string& sequenceName, std::string& pairingCode, std::string& error);
    // Joins the session `advertId` (SessionAdvert::id). Asynchronous: watch
    // GetState(). `fetchFiles`: copy show files that differ from the host's
    // (else a layout / audio difference refuses the join).
    bool Join(const std::string& displayName, const std::string& advertId, const std::string& pairingCode, bool fetchFiles,
              std::string& error);
    void Leave(); // client
    void End();   // host: ends the session for everyone

    Role GetRole() const { return _role; }
    State GetState() const { return _state; }
    bool InSession() const { return _state == State::Active; }
    // One line for a status bar / toolbar chip.
    std::string StatusText() const;
    const std::string& EndReason() const { return _endReason; }
    // Readable account of why the session ended or the join failed.
    std::string EndText() const;
    // Client: the show folder comparison, once it is done.
    const std::optional<JoinReport>& GetJoinReport() const { return _joinReport; }

    // ---- Participants and presence ----
    // Everyone in the session, host first (empty when not in one).
    std::vector<Participant> Participants() const;
    std::optional<Participant> FindParticipant(ParticipantId id) const;
    ParticipantId LocalParticipant() const;
    // What this user is looking at / working on. softLock (the effect open in
    // the settings panel or inspector) only counts while it is also in
    // selectedEffects; it drops after softLockIdleMs without local activity.
    void SetLocalPresence(const PresenceInfo& p);
    // Another participant's presence; nullptr when none has arrived.
    const PresenceInfo* PresenceOf(ParticipantId who) const;
    // Who else has `effect` open for editing (kNoParticipant: nobody).
    ParticipantId SoftLockHolder(SeqObjectId effect) const;
    // Others who have `effect` selected.
    std::vector<ParticipantId> SelectedBy(SeqObjectId effect) const;

    // ---- Layout (see CollabLayout.h) ----
    // The session edits the layout live (host and this app both support it).
    // False: the layout stays locked for the session as before.
    bool LiveLayout() const;
    // The layout objects (layout id, else name) this user has selected in
    // the layout editor.
    void SetLayoutSelection(const std::vector<std::string>& objects);
    // Others who have `object` selected in their layout editor.
    std::vector<ParticipantId> LayoutSelectedBy(const std::string& object) const;
    // Before opening a modal editor on an object: false (with the holder's
    // name in `heldBy`) when someone else holds it. Otherwise this user
    // holds it, and others see it read-only, until EndLayoutLock.
    bool BeginLayoutLock(const LayoutLockInfo& lock, std::string& heldBy);
    void EndLayoutLock();
    // Who else holds a modal-editor lock on `object` (kNoParticipant: nobody).
    ParticipantId LayoutLockHolder(const std::string& object) const;
    // `object` may not be edited here because someone else holds its lock;
    // `why` reads "Sam is editing Arch 1's submodels".
    bool LayoutObjectReadOnly(const std::string& object, std::string& why) const;
    // Sends a pending local layout change now if no edit is in progress (the
    // controller also checks on every tick).
    void FlushLayout();
    // Uploading to controllers and sending output to the lights: only the
    // host may while a session runs (true when not in one).
    bool CanDriveControllers() const;

    // ---- Undo (also reached through the UndoManager redirect) ----
    bool Undo();
    bool Redo();
    bool CanUndo() const;
    bool CanRedo() const;
    std::string UndoLabel() const; // "Undo" / "Undo: <label>"
    std::string RedoLabel() const;

    // ---- Rejects and pending edits ----
    // Proposes a rejected add again on new layers (RejectAction::PutOnNewLayer).
    bool PutOnNewLayer(uint64_t rejectId);
    std::vector<PendingGhost> PendingGhosts() const;

    // ---- After the session ----
    // Client: keep the session copy of the host's show (when the join used
    // one) after the session object goes, and say where it is.
    bool KeepSessionCopy(std::string& folder);

    // ---- Driving ----
    void Tick(uint64_t nowMs);
    // Sends local edits recorded since the last flush now (the controller
    // does this itself at its safe points). Does nothing while a local edit
    // is still open.
    void Flush();

    // For tests.
    CollabParticipant* Session() { return _session.get(); }
    size_t HeldDeliveries() const { return _held.size(); }
    size_t DeferredCalls() const { return _deferred.size(); }
    // Off only in tests that prove the re-entry guard is needed.
    bool reentryGuard = true;

private:
    class DocBridge : public ICollabDocument {
    public:
        explicit DocBridge(CollabController& c) : _c(c) {}
        SequenceElements& Sequence() override;
        bool SaveXsq(std::string& out) override;
        bool LoadXsq(const std::string& xsq) override;
        std::string ShowFolder() override;
        std::vector<std::string> AudioFiles() override;
        bool UseShowFolder(const std::string& folder) override;
        void AddMediaFolder(const std::string& folder) override;

    private:
        CollabController& _c;
    };
    friend class DocBridge;

    // Marks the controller as applying session commands: the executor counts
    // as applying, local observation is off.
    class SessionApply {
    public:
        SessionApply(CollabController& c, bool stopRender);
        ~SessionApply();

    private:
        CollabController& _c;
    };
    // While the controller is out in code that may run a nested event loop
    // (a session apply, an ICollabApp hook, app notifications), deliveries,
    // timer ticks and other dispatcher work must not re-enter the session.
    class HookCall {
    public:
        explicit HookCall(CollabController& c) : _c(c) { ++_c._hookDepth; }
        ~HookCall() { --_c._hookDepth; }

    private:
        CollabController& _c;
    };
    class LoadScope {
    public:
        explicit LoadScope(CollabController& c) : _c(c) { ++_c._loadDepth; }
        ~LoadScope() { --_c._loadDepth; }

    private:
        CollabController& _c;
    };
    // Outermost entry point: on the way out, runs deferred calls and
    // delivers queued app notifications.
    class Entry {
    public:
        explicit Entry(CollabController& c) : _c(c) { ++_c._entryDepth; }
        ~Entry();

    private:
        CollabController& _c;
    };

    // ICommandObserver
    void OnCommandApplied(const SequenceCommand& applied, const SequenceCommand& inverse) override;
    // IUndoRedirect
    bool RedirectUndo() override { return Undo(); }
    bool RedirectRedo() override { return Redo(); }
    bool RedirectCanUndo() override { return CanUndo(); }
    bool RedirectCanRedo() override { return CanRedo(); }
    std::string RedirectUndoString() override { return UndoLabel(); }
    std::string RedirectRedoString() override { return RedoLabel(); }

    bool StartHosting(CollabNetFactory& net, Network network, const std::string& displayName, const std::string& sequenceName,
                      std::string& pairingCode, std::string& error);
    void ActivateHost();
    bool StartJoining(CollabNetFactory& net, Network network, const std::string& displayName, const std::string& advertId,
                      const std::string& pairingCode, bool fetchFiles, std::string& error);
    void StartGroupBrowser();
    void PublishSessions();
    SessionOptions MakeOptions(const std::string& displayName, bool fetchFiles) const;
    SessionEvents MakeEvents();
    void WrapTransport();
    void Deliver(bool touchesDocument, bool stopRender, std::function<void()> fn);
    void DrainHeld();
    bool LocalEditOpen();
    void Attach();
    void Detach();
    void AcquireApplyScope();
    void ReleaseApplyScope();
    void Teardown();
    void SessionEnded(const std::string& reason);
    void FinishEnd();
    void SyncState();
    void SetState(State s);
    void Post(std::function<void()> fn);
    void DrainAppEvents();
    bool Busy() const { return reentryGuard && (_applyDepth > 0 || _hookDepth > 0 || _draining); }
    void Defer(std::function<void()> fn);
    void Settle();
    std::string MoveAside(const std::string& path);
    void FlushHost(std::unique_ptr<SequenceCommand> applied, std::unique_ptr<SequenceCommand> inverse, uint64_t join);
    void FlushClient(std::unique_ptr<SequenceCommand> applied, std::unique_ptr<SequenceCommand> inverse, uint64_t join);
    void OnCommitApplied(const CommitInfo& info, const SequenceCommand& applied, const SequenceCommand& inverse);
    void OnReject(uint64_t seq, EditKind kind, const std::string& reason, ParticipantId by);
    void TrySendPresence();
    std::string NameOf(ParticipantId id) const;
    void ScheduleFlush();
    // Live layout
    void SetLocks();
    void CheckLayout(bool force);
    bool ApplySessionLayout(const LayoutCommitInfo& info, const LayoutFiles& before, const LayoutFiles& after);
    bool BringMemoryTo(const LayoutFiles& target, const LayoutFiles& session, ParticipantId author);
    void AfterSnapshotLoaded();
    void OnLayoutRejectedBySession(uint64_t seq, const std::string& reason, const std::string& text);
    bool TryRestoreLayout();
    void ResetLayoutState();

    ICollabApp& _app;
    CollabNetFactory _net;
    CollabNetFactory _groupNet;
    std::unique_ptr<ISessionBrowser> _groupBrowser;
    std::vector<SessionAdvert> _lanSessions;
    SessionAdvert _groupOffer;
    Network _network = Network::Lan;
    Dispatcher _dispatch;
    ControllerOptions _opts;
    DocBridge _doc;
    std::shared_ptr<bool> _alive = std::make_shared<bool>(true);

    std::unique_ptr<INetTransport> _transport;
    std::unique_ptr<ISessionBrowser> _browser;
    std::unique_ptr<CollabParticipant> _session;
    bool _heldApplyQueued = false; // a Deliver of the session's held work is waiting
    CollabHost* _host = nullptr;
    CollabClient* _client = nullptr;
    Role _role = Role::None;
    State _state = State::Idle;
    std::string _endReason;
    std::string _endText;
    std::string _hostPeerName;
    bool _locked = false;
    bool _closed = false;      // transport closed after the end
    bool _everActive = false;  // client: has had the document at least once
    bool _inCommitLocal = false;
    bool _endPending = false; // ended; detaching waits until no local edit is open
    uint64_t _endedAt = 0;
    uint64_t _now = 0;
    std::vector<SessionAdvert> _sessions;
    std::optional<JoinReport> _joinReport;
    std::vector<RosterEntry> _roster;

    // Local edits
    SequenceElements* _attached = nullptr;
    std::vector<std::pair<std::unique_ptr<SequenceCommand>, std::unique_ptr<SequenceCommand>>> _local;
    bool _flushPosted = false;
    bool _haveLastFlush = false;
    unsigned int _lastFlushStep = 0;
    uint64_t _lastFlushSeq = 0;
    std::map<uint64_t, std::unique_ptr<SequenceCommand>> _submitted; // client: structural proposals in flight
    std::map<uint64_t, std::unique_ptr<SequenceCommand>> _rejected;
    std::map<uint64_t, std::vector<std::pair<EffectLocator, int>>> _reselect; // selection to restore when a proposal commits

    // Session applies
    int _applyDepth = 0;
    std::optional<CommandExecutor::ExternalApplyScope> _applyScope;
    SequenceElements* _applyScopeSeq = nullptr;
    struct Held {
        std::function<void()> fn;
        bool stopRender = false;
        std::string heldFile; // a blob moved aside while held; removed after delivery
    };
    std::deque<Held> _held;
    uint64_t _heldFiles = 0;

    // Re-entry
    int _hookDepth = 0;
    // LoadXsq calls into the app still on the stack. Nothing attaches to the
    // app's sequence meanwhile: the app replaces it when the call returns, so
    // an attach from inside it (a nested event loop delivering a second
    // snapshot) left _attached pointing at a freed SequenceElements.
    int _loadDepth = 0;
    bool _settling = false;
    std::deque<std::function<void()>> _deferred; // dispatcher work that arrived while Busy()

    // App notifications
    int _entryDepth = 0;
    bool _draining = false;
    std::deque<std::function<void()>> _appEvents;

    // Presence
    std::map<ParticipantId, PresenceInfo> _presence;
    PresenceInfo _localPresence;
    PresenceInfo _sentPresence;
    bool _presenceSet = false; // the app has said something
    bool _presenceSent = false;
    uint64_t _lastPresenceSent = 0;
    uint64_t _lastActivity = 0;
    std::map<ParticipantId, uint64_t> _lockOrder; // when the host saw each participant's layout lock taken

    // Live layout
    bool _liveLayout = false;
    bool _seqSettingsLocked = false; // SetSequenceSettingsLock(true) is in force
    bool _layoutGenKnown = false;
    uint64_t _layoutGen = 0;      // LayoutGeneration() as last seen
    uint64_t _layoutChangedAt = 0;
    bool _layoutDirty = false;    // memory may hold layout edits the session lacks
    uint64_t _layoutInFlight = 0; // client: the layout proposal waiting for the host
    std::optional<LayoutFiles> _snapshotTarget; // what memory was brought to before a snapshot loaded
    std::optional<LayoutFiles> _layoutRestore;  // memory still has to be brought here
    bool _lockLost = false;       // the host saw someone else take this user's layout lock first
};

} // namespace Collab
