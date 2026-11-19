/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "CollabController.h"

#include <algorithm>
#include <array>
#include <climits>
#include <filesystem>
#include <random>
#include <set>
#include <system_error>

#include "CollabCommandInfo.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/RenderContext.h"
#include "render/SequenceElementCommands.h"
#include "render/SequenceElements.h"
#include "utils/LayoutLock.h"

#include <log.h>

namespace Collab {

namespace {

const char* const kDash = " \xE2\x80\x94 ";

void Flatten(const SequenceCommand& cmd, std::vector<const SequenceCommand*>& out) {
    if (cmd.Type() == CommandType::Batch) {
        for (const auto& m : static_cast<const BatchCommand&>(cmd).GetCommands()) {
            Flatten(*m, out);
        }
        return;
    }
    out.push_back(&cmd);
}

// Collects what a command (and its inverse) touched.
class ChangeBuilder {
public:
    ChangeBuilder(const SequenceElements& seq, SequenceChange& c) : _seq(seq), _c(c) {}

    void Note(const SequenceCommand& cmd) {
        std::vector<const SequenceCommand*> leaves;
        Flatten(cmd, leaves);
        for (const SequenceCommand* l : leaves) {
            NoteLeaf(*l);
        }
    }

    void Finish() {
        std::set<SeqObjectId> tops;
        for (SeqObjectId r : _rows) {
            tops.insert(TopLevelOf(_seq, r));
        }
        for (SeqObjectId t : tops) {
            _c.rows.push_back(t);
            if (const Element* e = _seq.GetElementFromUid(t)) {
                _c.rowNames.push_back(e->GetName());
            } else if (auto n = _names.find(t); n != _names.end()) {
                _c.rowNames.push_back(n->second);
            }
        }
        _c.effects.assign(_effects.begin(), _effects.end());
        if (_whole || _lo > _hi) {
            _c.startMS = -1;
            _c.endMS = -1;
        } else {
            _c.startMS = _lo;
            _c.endMS = _hi;
        }
    }

private:
    void Range(int s, int e) {
        _lo = std::min(_lo, std::min(s, e));
        _hi = std::max(_hi, std::max(s, e));
    }
    void NoteEffect(const EffectLocator& loc, bool timesFromDocument) {
        _rows.insert(loc.element);
        if (loc.effect != kNoSeqObjectId) {
            _effects.insert(loc.effect);
        }
        if (timesFromDocument) {
            if (const Effect* e = FindEffect(_seq, loc)) {
                Range(e->GetStartTimeMS(), e->GetEndTimeMS());
            }
        }
    }
    void States(const std::vector<EffectState>& states) {
        for (const auto& s : states) {
            Range(s.startTimeMS, s.endTimeMS);
            _effects.insert(s.uid);
        }
    }

    void NoteLeaf(const SequenceCommand& cmd) {
        switch (cmd.Type()) {
        case CommandType::AddEffect: {
            const auto& c = static_cast<const AddEffectCommand&>(cmd);
            NoteEffect(c.Target(), false);
            Range(c.GetState().startTimeMS, c.GetState().endTimeMS);
            break;
        }
        case CommandType::DeleteEffect:
            // Its times come from the AddEffect on the other side.
            NoteEffect(static_cast<const DeleteEffectCommand&>(cmd).Target(), true);
            break;
        case CommandType::MoveEffect: {
            const auto& c = static_cast<const MoveEffectCommand&>(cmd);
            NoteEffect(c.Target(), false);
            Range(c.GetStartTimeMS(), c.GetEndTimeMS());
            break;
        }
        case CommandType::ModifyEffect:
            NoteEffect(static_cast<const ModifyEffectCommand&>(cmd).Target(), true);
            break;
        case CommandType::SetEffectSymbolLink:
            NoteEffect(static_cast<const SetEffectSymbolLinkCommand&>(cmd).GetTarget(), true);
            break;
        case CommandType::AddLayer: {
            const auto& c = static_cast<const AddLayerCommand&>(cmd);
            _rows.insert(c.GetElement());
            States(c.GetEffects());
            break;
        }
        case CommandType::RemoveLayer:
            // Its effects come back in the AddLayer on the other side.
            _rows.insert(static_cast<const RemoveLayerCommand&>(cmd).GetElement());
            break;
        case CommandType::SetLayerName:
            _rows.insert(static_cast<const SetLayerNameCommand&>(cmd).GetLayer().element);
            break;
        case CommandType::ReplaceLayerContents: {
            const auto& c = static_cast<const ReplaceLayerContentsCommand&>(cmd);
            _rows.insert(c.GetLayer().element);
            States(c.GetEffects());
            break;
        }
        case CommandType::PutMedia:
        case CommandType::RemoveMedia:
        case CommandType::RenameMedia:
            _c.mediaChanged = true;
            break;
        case CommandType::AddElement: {
            const auto& c = static_cast<const AddElementCommand&>(cmd);
            _rows.insert(c.GetUid());
            _names[c.GetUid()] = c.GetState().name;
            _c.rowsAdded = true;
            _whole = true;
            break;
        }
        case CommandType::DeleteElement:
            _rows.insert(static_cast<const DeleteElementCommand&>(cmd).GetElement());
            _c.rowsRemoved = true;
            _whole = true;
            break;
        case CommandType::MoveElement:
            _rows.insert(static_cast<const MoveElementCommand&>(cmd).GetElement());
            _c.rowsReordered = true;
            break;
        case CommandType::RenameTimingTrack:
            _rows.insert(static_cast<const RenameTimingTrackCommand&>(cmd).GetElement());
            _c.rowsRenamed = true;
            break;
        case CommandType::RenameModelElement:
            _rows.insert(static_cast<const RenameModelElementCommand&>(cmd).GetElement());
            _c.rowsRenamed = true;
            _whole = true;
            break;
        case CommandType::SetTimingFixed:
            _rows.insert(static_cast<const SetTimingFixedCommand&>(cmd).GetElement());
            _whole = true;
            break;
        case CommandType::SetElementRenderDisabled:
            _rows.insert(static_cast<const SetElementRenderDisabledCommand&>(cmd).GetElement());
            _whole = true;
            break;
        case CommandType::SetSequenceFace:
        case CommandType::SetEffectSymbol:
            // Effects anywhere may use them.
            _c.everything = true;
            _whole = true;
            break;
        case CommandType::SetTag:
        case CommandType::SetSongStructure:
        case CommandType::Batch:
            break;
        }
    }

    const SequenceElements& _seq;
    SequenceChange& _c;
    std::set<SeqObjectId> _rows;
    std::set<SeqObjectId> _effects;
    std::map<SeqObjectId, std::string> _names;
    bool _whole = false;
    int _lo = INT_MAX;
    int _hi = INT_MIN;
};

SequenceChange Summarize(const SequenceElements& seq, const SequenceCommand& applied, const SequenceCommand& inverse) {
    SequenceChange c;
    ChangeBuilder b(seq, c);
    b.Note(applied);
    b.Note(inverse);
    b.Finish();
    c.label = CommandLabel(applied);
    return c;
}

bool TouchesDocument(MsgType t) {
    switch (t) {
    case MsgType::Commit:
    case MsgType::Reject:
    case MsgType::Propose:
    case MsgType::Snapshot:
    case MsgType::NeedSnapshot:
    case MsgType::File:
    case MsgType::Bye:
    // A rejoin Welcome rejects what the old host decided (rolling back
    // optimistic edits); a Resend the log no longer reaches saves a snapshot.
    case MsgType::Welcome:
    case MsgType::Resend:
        return true;
    default:
        return false;
    }
}

std::string RandomCode() {
    std::random_device rd;
    std::uniform_int_distribution<int> d(0, 999999);
    std::string s = std::to_string(d(rd));
    return std::string(6 - s.size(), '0') + s;
}

} // namespace

xlColor ParticipantColour(ParticipantId id) {
    static const std::array<xlColor, 12> kPalette{ {
        xlColor(0x1F, 0x77, 0xB4), // blue (the host)
        xlColor(0xFF, 0x7F, 0x0E), // orange
        xlColor(0x2C, 0xA0, 0x2C), // green
        xlColor(0xD6, 0x27, 0x28), // red
        xlColor(0x94, 0x67, 0xBD), // purple
        xlColor(0x17, 0xBE, 0xCF), // cyan
        xlColor(0xE3, 0x77, 0xC2), // pink
        xlColor(0xBC, 0xBD, 0x22), // olive
        xlColor(0x8C, 0x56, 0x4B), // brown
        xlColor(0xF5, 0xB7, 0x00), // amber
        xlColor(0x00, 0x80, 0x80), // teal
        xlColor(0x6B, 0x6E, 0xCF), // indigo
    } };
    if (id == kNoParticipant) {
        return xlColor(0x80, 0x80, 0x80);
    }
    return kPalette[(id - 1) % kPalette.size()];
}

RejectText DescribeReject(const std::string& reason, EditKind kind, const std::string& byName) {
    const std::string who = byName.empty() ? std::string("someone else") : byName;
    std::string prefix;
    if (kind == EditKind::Undo) {
        prefix = std::string("Can't undo") + kDash;
    } else if (kind == EditKind::Redo) {
        prefix = std::string("Can't redo") + kDash;
    }
    RejectText t;
    if (reason == Reason::Overlap) {
        if (prefix.empty()) {
            t.text = "Someone placed an effect there first";
            t.action = RejectAction::PutOnNewLayer;
        } else {
            t.text = prefix + "another effect is there now";
        }
    } else if (reason == Reason::TargetGone) {
        t.text = prefix.empty() ? "That effect, layer or row was deleted by " + who : prefix + "it was deleted by " + who;
    } else if (reason == Reason::ChangedSince) {
        t.text = (prefix.empty() ? std::string("Can't undo") + kDash : prefix) + "that effect was changed by " + who;
    } else if (reason == Reason::StaleType) {
        t.text = who + " changed that effect's type first, so the change wasn't made";
    } else if (reason == Reason::Locked) {
        t.text = prefix + (prefix.empty() ? "That effect is locked" : "that effect is locked");
    } else if (reason == Reason::Invalid) {
        t.text = prefix + (prefix.empty() ? "That change conflicts with someone else's edit" : "it conflicts with someone else's edit");
    } else if (reason == "not_active") {
        t.text = "You're not connected to the session";
    } else if (reason == "nothing_to_undo") {
        t.text = "Nothing to undo";
    } else if (reason == "nothing_to_redo") {
        t.text = "Nothing to redo";
    } else {
        t.text = prefix + (prefix.empty() ? "That change couldn't be made" : "that change couldn't be made") + " (" + reason + ")";
    }
    return t;
}

// ---------------------------------------------------------------------------
// Bridges and scopes
// ---------------------------------------------------------------------------

SequenceElements& CollabController::DocBridge::Sequence() {
    return _c._app.Sequence();
}
bool CollabController::DocBridge::SaveXsq(std::string& out) {
    return _c._app.SaveXsq(out);
}
std::string CollabController::DocBridge::ShowFolder() {
    return _c._app.ShowFolder();
}
std::vector<std::string> CollabController::DocBridge::AudioFiles() {
    return _c._app.AudioFiles();
}
bool CollabController::DocBridge::UseShowFolder(const std::string& folder) {
    return _c._app.UseShowFolder(folder);
}
void CollabController::DocBridge::AddMediaFolder(const std::string& folder) {
    _c._app.AddMediaFolder(folder);
}

bool CollabController::DocBridge::LoadXsq(const std::string& xsq) {
    // The app may replace its SequenceElements: let go of the old one first.
    bool scoped = _c._applyScope.has_value();
    _c.Detach();
    _c.ReleaseApplyScope();
    bool ok = _c._app.LoadXsq(xsq);
    if (scoped) {
        _c.AcquireApplyScope();
    }
    if (ok && _c._role == Role::Client && _c._state != State::Ended) {
        _c.Attach();
    }
    return ok;
}

CollabController::SessionApply::SessionApply(CollabController& c, bool stopRender) : _c(c) {
    if (++_c._applyDepth == 1) {
        _c.AcquireApplyScope();
    }
    if (stopRender && _c._app.HasSequence()) {
        // Inside the apply scope, so the desktop waits without yielding.
        if (RenderContext* rc = _c._app.Sequence().GetRenderContext()) {
            if (!rc->AbortRender()) {
                spdlog::warn("Collab: the render did not stop before a session edit");
            }
        }
    }
}

CollabController::SessionApply::~SessionApply() {
    if (--_c._applyDepth == 0) {
        _c.ReleaseApplyScope();
    }
}

CollabController::Entry::~Entry() {
    if (--_c._entryDepth == 0) {
        _c.DrainAppEvents();
    }
}

void CollabController::AcquireApplyScope() {
    if (!_app.HasSequence()) {
        return;
    }
    SequenceElements& seq = _app.Sequence();
    _applyScope.emplace(seq.GetCommandExecutor());
    _applyScopeSeq = &seq;
}

void CollabController::ReleaseApplyScope() {
    _applyScope.reset();
    _applyScopeSeq = nullptr;
}

// ---------------------------------------------------------------------------
// Construction and lifecycle
// ---------------------------------------------------------------------------

CollabController::CollabController(ICollabApp& app, CollabNetFactory net, Dispatcher dispatch, ControllerOptions opts) :
    _app(app), _net(std::move(net)), _dispatch(std::move(dispatch)), _opts(std::move(opts)), _doc(*this) {
}

CollabController::~CollabController() {
    if (_host != nullptr && _state == State::Active) {
        _host->End();
    } else if (_client != nullptr && _state != State::Ended && _state != State::Idle) {
        _client->Leave();
    }
    _appEvents.clear();
    Teardown();
    if (_browser) {
        _browser->Stop();
        _browser.reset();
    }
}

bool CollabController::Available() const {
    return _net.transport && (!_net.available || _net.available());
}

void CollabController::StartBrowsing() {
    Entry en(*this);
    if (!_browser && _net.browser) {
        _browser = _net.browser();
        if (!_browser) {
            return;
        }
        _browser->onChanged = [this](const std::vector<SessionAdvert>& list) {
            Entry e(*this);
            _sessions = list;
            Post([this, list] { _app.OnSessionsFound(list); });
        };
        _browser->onError = [this](const std::string& why) {
            Entry e(*this);
            Post([this, why] { _app.OnBrowseError(why); });
        };
    }
    if (_browser) {
        _browser->Start();
    }
}

void CollabController::StopBrowsing() {
    if (_browser) {
        _browser->Stop();
    }
}

SessionOptions CollabController::MakeOptions(const std::string& displayName, bool fetchFiles) const {
    SessionOptions so = _opts.session;
    so.displayName = displayName;
    so.appVersion = _opts.appVersion;
    so.deviceKind = _opts.deviceKind;
    so.sessionFolder = _opts.sessionFolder;
    so.fetchFiles = fetchFiles;
    return so;
}

SessionEvents CollabController::MakeEvents() {
    SessionEvents ev;
    ev.onCommitApplied = [this](const CommitInfo& info, const SequenceCommand& applied, const SequenceCommand& inverse) {
        OnCommitApplied(info, applied, inverse);
    };
    ev.onReject = [this](uint64_t seq, EditKind kind, const std::string& reason, ParticipantId by) { OnReject(seq, kind, reason, by); };
    ev.onEnded = [this](const std::string& reason) { SessionEnded(reason); };
    ev.onResync = [this] {
        _submitted.clear();
        _reselect.clear();
        SequenceChange c;
        c.cause = SequenceChange::Cause::Resync;
        c.everything = true;
        Post([this, c] { _app.OnSequenceChanged(c); });
        Post([this] { _app.OnUndoStateChanged(); });
    };
    ev.onPresence = [this](ParticipantId who, const PresenceInfo& p) {
        _presence[who] = p;
        Post([this, who] { _app.OnPresenceChanged(who); });
    };
    ev.onRoster = [this](const std::vector<RosterEntry>& roster) {
        _roster = roster;
        for (auto it = _presence.begin(); it != _presence.end();) {
            bool present = std::any_of(roster.begin(), roster.end(), [&](const RosterEntry& r) { return r.participant == it->first; });
            if (present) {
                ++it;
            } else {
                ParticipantId gone = it->first;
                it = _presence.erase(it);
                Post([this, gone] { _app.OnPresenceChanged(gone); });
            }
        }
        Post([this] { _app.OnParticipantsChanged(); });
    };
    ev.onLog = [this](const std::string& line) { _app.OnLog(line); };
    ev.onJoinChecked = [this](const JoinReport& r) {
        _joinReport = r;
        Post([this, r] { _app.OnJoinReport(r); });
    };
    ev.onFileArrived = [this](const std::string& reference, const std::string&) {
        SequenceChange c;
        c.cause = SequenceChange::Cause::FileArrived;
        c.mediaChanged = true;
        c.label = reference;
        Post([this, c] { _app.OnSequenceChanged(c); });
    };
    return ev;
}

void CollabController::WrapTransport() {
    // The session installed its handlers when it was made; wrap them.
    auto onMessage = _transport->onMessage;
    auto onPeer = _transport->onPeer;
    auto onBlob = _transport->onBlob;
    auto onSendFailed = _transport->onSendFailed;
    _transport->onMessage = [this, onMessage](PeerId from, const Message& m) {
        if (!onMessage) {
            return;
        }
        bool touches = TouchesDocument(m.type);
        bool stop = false;
        if (m.type == MsgType::Commit || m.type == MsgType::Propose) {
            auto cmd = DeserializeCommand(m.command);
            stop = cmd && UndoManager::CommandNeedsRenderStopped(*cmd);
        }
        Deliver(touches, stop, [onMessage, from, m] { onMessage(from, m); });
    };
    _transport->onPeer = [this, onPeer](PeerId peer, PeerEvent e) {
        Entry en(*this);
        if (_role == Role::Client && e == PeerEvent::Joined && _state == State::Connecting && _client != nullptr) {
            _client->Tick(_now);
            _client->Join(peer);
            SyncState();
            return;
        }
        if (onPeer) {
            onPeer(peer, e);
        }
        SyncState();
    };
    _transport->onBlob = [this, onBlob](PeerId from, BlobId id, const std::string& path) {
        if (!onBlob) {
            return;
        }
        Entry en(*this);
        DrainHeld();
        if (!LocalEditOpen() && _held.empty()) {
            Flush();
            {
                SessionApply a(*this, false);
                onBlob(from, id, path);
            }
            SyncState();
            return;
        }
        // The file is only ours during this call: move it aside until the
        // local edit closes.
        namespace fs = std::filesystem;
        std::error_code ec;
        fs::path dir = fs::path(_session ? _session->SessionFolder() : fs::temp_directory_path(ec).string()) / "held";
        fs::create_directories(dir, ec);
        fs::path aside = dir / ("b" + std::to_string(++_heldFiles));
        fs::rename(path, aside, ec);
        if (ec) {
            ec.clear();
            fs::copy_file(path, aside, fs::copy_options::overwrite_existing, ec);
        }
        std::string asideStr = aside.string();
        _held.push_back({ [onBlob, from, id, asideStr] { onBlob(from, id, asideStr); }, false, asideStr });
    };
}

void CollabController::Deliver(bool touchesDocument, bool stopRender, std::function<void()> fn) {
    Entry en(*this);
    if (!touchesDocument) {
        fn();
        SyncState();
        return;
    }
    DrainHeld();
    if (LocalEditOpen() || !_held.empty()) {
        _held.push_back({ std::move(fn), stopRender, "" });
        return;
    }
    Flush();
    {
        SessionApply a(*this, stopRender);
        fn();
    }
    SyncState();
}

void CollabController::DrainHeld() {
    while (!_held.empty() && !LocalEditOpen()) {
        Held h = std::move(_held.front());
        _held.pop_front();
        Flush();
        {
            SessionApply a(*this, h.stopRender);
            h.fn();
        }
        if (!h.heldFile.empty()) {
            std::error_code ec;
            std::filesystem::remove(h.heldFile, ec);
        }
        SyncState();
    }
}

bool CollabController::LocalEditOpen() {
    if (_attached == nullptr || _applyDepth > 0) {
        return false;
    }
    CommandExecutor& ex = _attached->GetCommandExecutor();
    return ex.InBatch() || ex.InPreview() || ex.IsCapturing() || ex.IsApplying() || _attached->get_undo_mgr().IsReplaying();
}

void CollabController::Attach() {
    if (_attached != nullptr) {
        return;
    }
    _attached = &_app.Sequence();
    _attached->GetCommandExecutor().AddObserver(this);
    _attached->get_undo_mgr().SetRedirect(this);
    _haveLastFlush = false;
    Post([this] { _app.OnUndoStateChanged(); });
}

void CollabController::Detach() {
    _local.clear();
    if (_attached == nullptr) {
        return;
    }
    _attached->GetCommandExecutor().RemoveObserver(this);
    _attached->get_undo_mgr().SetRedirect(nullptr);
    _attached = nullptr;
    _haveLastFlush = false;
    Post([this] { _app.OnUndoStateChanged(); });
}

void CollabController::Teardown() {
    Detach();
    ReleaseApplyScope();
    _session.reset();
    _host = nullptr;
    _client = nullptr;
    if (_transport) {
        if (!_closed) {
            _transport->Close();
        }
        _transport.reset();
    }
    _closed = false;
    for (const Held& h : _held) {
        if (!h.heldFile.empty()) {
            std::error_code ec;
            std::filesystem::remove(h.heldFile, ec);
        }
    }
    _held.clear();
    _submitted.clear();
    _rejected.clear();
    _reselect.clear();
    _roster.clear();
    _presence.clear();
    _presenceSet = false;
    _presenceSent = false;
    _sentPresence = {};
    _localPresence = {};
    _joinReport.reset();
    _endPending = false;
    _everActive = false;
    if (_locked) {
        _locked = false;
        _app.SetCollaborationLock(false, "");
    }
}

bool CollabController::Host(const std::string& displayName, const std::string& sequenceName, std::string& pairingCode, std::string& error) {
    Entry en(*this);
    if (_role != Role::None && _state != State::Ended && _state != State::Idle) {
        error = "Already in a collaboration session.";
        return false;
    }
    if (!Available()) {
        error = "Live collaboration isn't available on this device.";
        return false;
    }
    Teardown();
    _transport = _net.transport();
    if (!_transport) {
        error = "Live collaboration isn't available on this device.";
        return false;
    }
    _endReason.clear();
    _endText.clear();
    auto host = std::make_unique<CollabHost>(*_transport, _doc, MakeOptions(displayName, false), MakeEvents());
    _host = host.get();
    _session = std::move(host);
    _role = Role::Host;
    WrapTransport();
    _transport->onState = [this](NetState s, const std::string& why) {
        Entry e(*this);
        if (s == NetState::Failed && _role == Role::Host && _state == State::Active) {
            _host->End();
            _endText = why.empty() ? "The network connection failed." : why;
            SessionEnded("network");
        }
    };
    _host->Start();
    SessionAdvert ad;
    ad.sequenceName = sequenceName;
    ad.hostName = displayName;
    ad.deviceKind = _opts.deviceKind;
    pairingCode = _net.pairingCode ? _net.pairingCode() : RandomCode();
    if (!_transport->Listen(pairingCode, ad, error)) {
        if (error.empty()) {
            error = "Couldn't start the session on the network.";
        }
        Teardown();
        _role = Role::None;
        SetState(State::Idle);
        return false;
    }
    Attach();
    _locked = true;
    _app.SetCollaborationLock(true, LayoutLock::DEFAULT_REASON);
    SetState(State::Active);
    return true;
}

bool CollabController::Join(const std::string& displayName, const std::string& advertId, const std::string& pairingCode, bool fetchFiles,
                            std::string& error) {
    Entry en(*this);
    if (_role != Role::None && _state != State::Ended && _state != State::Idle) {
        error = "Already in a collaboration session.";
        return false;
    }
    if (!Available()) {
        error = "Live collaboration isn't available on this device.";
        return false;
    }
    Teardown();
    _transport = _net.transport();
    if (!_transport) {
        error = "Live collaboration isn't available on this device.";
        return false;
    }
    _endReason.clear();
    _endText.clear();
    auto client = std::make_unique<CollabClient>(*_transport, _doc, MakeOptions(displayName, fetchFiles), MakeEvents());
    _client = client.get();
    _session = std::move(client);
    _role = Role::Client;
    _hostPeerName.clear();
    for (const auto& s : _sessions) {
        if (s.id == advertId) {
            _hostPeerName = s.hostName;
        }
    }
    WrapTransport();
    _transport->onState = [this](NetState s, const std::string& why) {
        Entry e(*this);
        if ((s == NetState::Failed || s == NetState::Closed) && _role == Role::Client && _state != State::Ended) {
            _endText = why.empty() ? (s == NetState::Failed ? "Couldn't connect to the session." : "The connection was closed.") : why;
            if (_client->GetState() != CollabClient::State::Idle && _client->GetState() != CollabClient::State::Ended) {
                _client->Leave();
            }
            SessionEnded(s == NetState::Failed ? "connect_failed" : Reason::HostLost);
        }
    };
    _locked = true;
    _app.SetCollaborationLock(true, LayoutLock::DEFAULT_REASON);
    SetState(State::Connecting);
    _transport->Connect(advertId, pairingCode);
    return true;
}

void CollabController::Leave() {
    Entry en(*this);
    if (_role != Role::Client || _client == nullptr || _state == State::Ended || _state == State::Idle) {
        return;
    }
    Flush();
    _client->Leave();
    SessionEnded(Reason::Left);
}

void CollabController::End() {
    Entry en(*this);
    if (_role != Role::Host || _host == nullptr || _state != State::Active) {
        return;
    }
    Flush();
    _host->End();
    SessionEnded(Reason::Ended);
}

void CollabController::SessionEnded(const std::string& reason) {
    if (_state == State::Ended) {
        return;
    }
    _endReason = reason;
    if (_session && !_session->EndText().empty()) {
        _endText = _session->EndText();
    }
    _endedAt = _now;
    _endPending = true;
    SetState(State::Ended);
    FinishEnd();
}

void CollabController::FinishEnd() {
    if (!_endPending || LocalEditOpen() || _applyDepth > 0) {
        return;
    }
    _endPending = false;
    Detach();
    for (const Held& h : _held) {
        if (!h.heldFile.empty()) {
            std::error_code ec;
            std::filesystem::remove(h.heldFile, ec);
        }
    }
    _held.clear();
    _heldApplyQueued = false;
    _submitted.clear();
    _reselect.clear();
    _presence.clear();
    if (_locked) {
        _locked = false;
        Post([this] { _app.SetCollaborationLock(false, ""); });
    }
    Post([this] { _app.OnParticipantsChanged(); });
}

void CollabController::SetState(State s) {
    if (_state == s) {
        return;
    }
    _state = s;
    Post([this] { _app.OnSessionStateChanged(); });
}

void CollabController::SyncState() {
    if (_role != Role::Client || _client == nullptr || _state == State::Ended) {
        return;
    }
    switch (_client->GetState()) {
    case CollabClient::State::Idle:
        break;
    case CollabClient::State::Joining:
    case CollabClient::State::Checking:
        SetState(State::Joining);
        break;
    case CollabClient::State::AwaitingSnapshot:
        SetState(State::Syncing);
        break;
    case CollabClient::State::Active:
        Attach();
        if (!_everActive) {
            _everActive = true;
            SequenceChange c;
            c.cause = SequenceChange::Cause::Resync;
            c.everything = true;
            Post([this, c] { _app.OnSequenceChanged(c); });
        }
        SetState(State::Active);
        break;
    case CollabClient::State::Ended:
        SessionEnded(_client->EndReason().empty() ? std::string(Reason::Ended) : _client->EndReason());
        break;
    }
}

void CollabController::Post(std::function<void()> fn) {
    _appEvents.push_back(std::move(fn));
    if (_entryDepth == 0) {
        DrainAppEvents();
    }
}

void CollabController::DrainAppEvents() {
    if (_draining || _applyDepth > 0) {
        return;
    }
    _draining = true;
    while (!_appEvents.empty()) {
        auto fn = std::move(_appEvents.front());
        _appEvents.pop_front();
        fn();
    }
    _draining = false;
}

void CollabController::Tick(uint64_t nowMs) {
    _now = std::max(_now, nowMs);
    Entry en(*this);
    FinishEnd();
    if (_session && _state != State::Ended) {
        Flush();
        DrainHeld();
        _session->Tick(_now);
        TrySendPresence();
        SyncState();
    } else if (_state == State::Ended && _transport && !_closed && _now - _endedAt >= _opts.closeDelayMs) {
        _transport->Close();
        _closed = true;
    }
}

// ---------------------------------------------------------------------------
// Local edits
// ---------------------------------------------------------------------------

void CollabController::OnCommandApplied(const SequenceCommand& applied, const SequenceCommand& inverse) {
    if (_applyDepth > 0 || !_session || _state == State::Ended) {
        return;
    }
    _local.emplace_back(applied.Clone(), inverse.Clone());
    _lastActivity = _now;
    ScheduleFlush();
}

void CollabController::ScheduleFlush() {
    if (_flushPosted || !_dispatch) {
        return;
    }
    _flushPosted = true;
    std::weak_ptr<bool> alive = _alive;
    _dispatch([this, alive] {
        if (alive.expired()) {
            return;
        }
        _flushPosted = false;
        Entry en(*this);
        Flush();
        DrainHeld();
    });
}

void CollabController::Flush() {
    if (_local.empty() || !_session || !_session->IsActive() || _applyDepth > 0 || LocalEditOpen()) {
        return;
    }
    Entry en(*this);
    auto items = std::move(_local);
    _local.clear();
    std::unique_ptr<SequenceCommand> applied;
    std::unique_ptr<SequenceCommand> inverse;
    if (items.size() == 1) {
        applied = std::move(items[0].first);
        inverse = std::move(items[0].second);
    } else {
        std::string label;
        for (const auto& it : items) {
            if (it.first->Type() == CommandType::Batch && !static_cast<const BatchCommand&>(*it.first).GetLabel().empty()) {
                label = static_cast<const BatchCommand&>(*it.first).GetLabel();
                break;
            }
        }
        if (label.empty()) {
            label = CommandLabel(*items.front().first);
        }
        auto a = std::make_unique<BatchCommand>(label);
        auto inv = std::make_unique<BatchCommand>(label);
        for (auto& it : items) {
            if (it.first->Type() == CommandType::Batch) {
                for (const auto& m : static_cast<const BatchCommand&>(*it.first).GetCommands()) {
                    a->Add(m->Clone());
                }
            } else {
                a->Add(std::move(it.first));
            }
        }
        // A batch's inverse already runs its members backwards.
        for (auto it = items.rbegin(); it != items.rend(); ++it) {
            if (it->second->Type() == CommandType::Batch) {
                for (const auto& m : static_cast<const BatchCommand&>(*it->second).GetCommands()) {
                    inv->Add(m->Clone());
                }
            } else {
                inv->Add(std::move(it->second));
            }
        }
        applied = std::move(a);
        inverse = std::move(inv);
    }
    // Recorded into the same undo step as what was sent last (an edit
    // coalescer still extending its step): undo them together.
    unsigned int step = _attached ? _attached->get_undo_mgr().GetStepSerial() : 0;
    uint64_t join = (_haveLastFlush && step == _lastFlushStep) ? _lastFlushSeq : 0;
    _lastFlushStep = step;
    if (_host != nullptr) {
        FlushHost(std::move(applied), std::move(inverse), join);
    } else {
        FlushClient(std::move(applied), std::move(inverse), join);
    }
    Post([this] { _app.OnUndoStateChanged(); });
}

void CollabController::FlushHost(std::unique_ptr<SequenceCommand> applied, std::unique_ptr<SequenceCommand> inverse, uint64_t join) {
    _inCommitLocal = true;
    SubmitResult r = _host->CommitLocal(std::move(applied), std::move(inverse), join);
    _inCommitLocal = false;
    _haveLastFlush = r.ok;
    _lastFlushSeq = r.clientSeq;
}

void CollabController::FlushClient(std::unique_ptr<SequenceCommand> applied, std::unique_ptr<SequenceCommand> inverse, uint64_t join) {
    SequenceElements& seq = _app.Sequence();
    std::vector<const SequenceCommand*> members;
    Flatten(*applied, members);
    std::vector<std::pair<EffectLocator, int>> selected;
    for (const SequenceCommand* m : members) {
        if (m->Type() == CommandType::AddEffect) {
            EffectLocator loc = static_cast<const AddEffectCommand*>(m)->Target();
            if (const Effect* e = FindEffect(seq, loc); e != nullptr && e->GetSelected() != 0) {
                selected.push_back({ loc, e->GetSelected() });
            }
        }
    }
    SequenceChange pending = Summarize(seq, *applied, *inverse);
    std::vector<RejectNotice> refused;
    SubmitResult last;
    bool anyPending = false;
    {
        SessionApply a(*this, UndoManager::CommandNeedsRenderStopped(*inverse));
        // The session's document must be exactly the host's order plus this
        // client's overlay: take the edit back out and propose it.
        CommandResult rr;
        if (!inverse->Apply(seq, ApplyMode::Lenient, &rr)) {
            spdlog::warn("Collab: a local edit could not be taken back ({}); re-syncing", rr.message);
            _client->RequestResync("local edit could not be taken back");
            return;
        }
        auto submit = [&](std::unique_ptr<SequenceCommand> cmd, uint64_t joinTo) {
            std::unique_ptr<SequenceCommand> copy = cmd->Clone();
            SubmitResult r = _client->SubmitJoined(std::move(cmd), joinTo);
            if (!r.ok) {
                RejectNotice n;
                n.reason = r.reason;
                RejectText t = DescribeReject(r.reason, EditKind::Edit, "");
                n.text = t.text;
                refused.push_back(n);
                return r;
            }
            if (!r.optimistic) {
                anyPending = true;
                _submitted[r.clientSeq] = std::move(copy);
                if (!selected.empty()) {
                    _reselect[r.clientSeq] = selected;
                }
            }
            last = r;
            return r;
        };
        // Key changes and moves (alone or as a batch of only those) apply
        // again at once inside Submit; anything else waits for the commit.
        submit(std::move(applied), join);
    }
    _haveLastFlush = last.ok;
    _lastFlushSeq = last.clientSeq;
    if (anyPending || !refused.empty()) {
        // Taken out to be proposed: it comes back with its commit. Refused
        // here, it never comes back, so what it touched renders now.
        pending.cause = refused.empty() ? SequenceChange::Cause::OwnEditPending : SequenceChange::Cause::OwnEditRejected;
        pending.author = LocalParticipant();
        Post([this, pending] { _app.OnSequenceChanged(pending); });
    }
    for (const auto& n : refused) {
        Post([this, n] { _app.OnRejected(n); });
    }
}

void CollabController::OnCommitApplied(const CommitInfo& info, const SequenceCommand& applied, const SequenceCommand& inverse) {
    if (_inCommitLocal) {
        return; // the host's own edit: already in the document and on screen
    }
    SequenceElements& seq = _app.Sequence();
    SequenceChange c = Summarize(seq, applied, inverse);
    c.author = info.author;
    bool own = info.author == LocalParticipant();
    if (!own) {
        c.cause = SequenceChange::Cause::RemoteEdit;
    } else if (info.kind == EditKind::Edit) {
        c.cause = SequenceChange::Cause::OwnEdit;
    } else {
        c.cause = SequenceChange::Cause::OwnUndo;
    }
    if (own) {
        _submitted.erase(info.clientSeq);
        auto r = _reselect.find(info.clientSeq);
        if (r != _reselect.end()) {
            for (const auto& [loc, sel] : r->second) {
                if (Effect* e = FindEffect(seq, loc)) {
                    e->SetSelected(sel);
                }
            }
            _reselect.erase(r);
        }
        Post([this] { _app.OnUndoStateChanged(); });
    }
    Post([this, c] { _app.OnSequenceChanged(c); });
    if (_role == Role::Host) {
        Post([this] { _app.MarkSequenceModified(); });
    }
}

void CollabController::OnReject(uint64_t seq, EditKind kind, const std::string& reason, ParticipantId by) {
    RejectNotice n;
    n.id = seq;
    n.kind = kind;
    n.reason = reason;
    n.by = by;
    RejectText t = DescribeReject(reason, kind, by != kNoParticipant ? NameOf(by) : "");
    n.text = t.text;
    n.action = t.action;
    auto it = _submitted.find(seq);
    if (it != _submitted.end()) {
        // A structural edit was taken out of the document while it waited
        // (OwnEditPending); it isn't coming back, so what it touched renders
        // as it is now.
        SequenceChange c = Summarize(_app.Sequence(), *it->second, *it->second);
        c.cause = SequenceChange::Cause::OwnEditRejected;
        c.author = LocalParticipant();
        Post([this, c] { _app.OnSequenceChanged(c); });
        _rejected[seq] = std::move(it->second);
        _submitted.erase(it);
        while (_rejected.size() > 32) {
            _rejected.erase(_rejected.begin());
        }
    }
    if (n.action == RejectAction::PutOnNewLayer) {
        bool adds = false;
        if (auto r = _rejected.find(seq); r != _rejected.end()) {
            std::vector<const SequenceCommand*> members;
            Flatten(*r->second, members);
            adds = !members.empty() &&
                   std::all_of(members.begin(), members.end(), [](const SequenceCommand* m) { return m->Type() == CommandType::AddEffect; });
        }
        if (!adds) {
            n.action = RejectAction::None;
        }
    }
    _reselect.erase(seq);
    Post([this, n] { _app.OnRejected(n); });
    Post([this] { _app.OnUndoStateChanged(); });
}

bool CollabController::PutOnNewLayer(uint64_t rejectId) {
    Entry en(*this);
    auto it = _rejected.find(rejectId);
    if (it == _rejected.end() || !_session || !_session->IsActive()) {
        return false;
    }
    Flush();
    SequenceElements& seq = _app.Sequence();
    std::vector<const SequenceCommand*> members;
    Flatten(*it->second, members);
    auto batch = std::make_unique<BatchCommand>("Put on New Layer");
    std::map<SeqObjectId, std::pair<SeqObjectId, SeqObjectId>> layers; // old layer -> (owner, new layer)
    for (const SequenceCommand* m : members) {
        if (m->Type() != CommandType::AddEffect) {
            return false;
        }
        const auto* add = static_cast<const AddEffectCommand*>(m);
        auto found = layers.find(add->GetLayer().layer);
        if (found == layers.end()) {
            EffectLayer* l = FindLayer(seq, add->GetLayer());
            Element* owner = l != nullptr ? l->GetParentElement() : nullptr;
            if (owner == nullptr || owner->GetEffectLayerCount() == 0) {
                return false;
            }
            int count = (int)owner->GetEffectLayerCount();
            auto al = std::make_unique<AddLayerCommand>(owner->GetUid(), kNoSeqObjectId, owner->GetEffectLayer(count - 1)->GetUid(), count);
            found = layers.emplace(add->GetLayer().layer, std::make_pair(owner->GetUid(), al->GetLayer())).first;
            batch->Add(std::move(al));
        }
        EffectState state = add->GetState();
        state.uid = kNoSeqObjectId;
        batch->Add(std::make_unique<AddEffectCommand>(LayerLocator{ found->second.first, found->second.second }, std::move(state)));
    }
    std::unique_ptr<SequenceCommand> copy = batch->Clone();
    SubmitResult r;
    {
        SessionApply a(*this, false);
        r = _session->Submit(std::move(batch));
    }
    if (!r.ok) {
        return false;
    }
    _rejected.erase(rejectId);
    if (_client != nullptr && !r.optimistic) {
        _submitted[r.clientSeq] = std::move(copy);
    }
    return true;
}

std::vector<PendingGhost> CollabController::PendingGhosts() const {
    std::vector<PendingGhost> out;
    if (_client == nullptr || _state != State::Active) {
        return out;
    }
    SequenceElements& seq = _app.Sequence();
    for (const auto& p : _client->PendingProposals()) {
        if (p.optimistic) {
            continue;
        }
        auto cmd = DeserializeCommand(p.command);
        if (!cmd) {
            continue;
        }
        std::vector<const SequenceCommand*> leaves;
        Flatten(*cmd, leaves);
        for (const SequenceCommand* l : leaves) {
            PendingGhost g;
            g.proposal = p.clientSeq;
            if (l->Type() == CommandType::AddEffect) {
                const auto* a = static_cast<const AddEffectCommand*>(l);
                g.kind = PendingGhost::Kind::Add;
                g.effect = a->Target();
                g.effectName = a->GetState().name;
                g.startMS = a->GetState().startTimeMS;
                g.endMS = a->GetState().endTimeMS;
            } else if (l->Type() == CommandType::MoveEffect) {
                const auto* mv = static_cast<const MoveEffectCommand*>(l);
                g.kind = PendingGhost::Kind::Move;
                g.effect = mv->Target();
                g.startMS = mv->GetStartTimeMS();
                g.endMS = mv->GetEndTimeMS();
                if (const Effect* e = FindEffect(seq, g.effect)) {
                    g.effectName = e->GetEffectName();
                }
            } else if (l->Type() == CommandType::DeleteEffect) {
                const auto* d = static_cast<const DeleteEffectCommand*>(l);
                g.kind = PendingGhost::Kind::Delete;
                g.effect = d->Target();
                const Effect* e = FindEffect(seq, g.effect);
                if (e == nullptr) {
                    continue;
                }
                g.effectName = e->GetEffectName();
                g.startMS = e->GetStartTimeMS();
                g.endMS = e->GetEndTimeMS();
            } else {
                continue;
            }
            out.push_back(std::move(g));
        }
    }
    return out;
}

// ---------------------------------------------------------------------------
// Undo
// ---------------------------------------------------------------------------

bool CollabController::Undo() {
    Entry en(*this);
    Flush();
    if (!_session || !_session->IsActive()) {
        return false;
    }
    _haveLastFlush = false;
    const SequenceCommand* next = _session->NextUndoCommand(false);
    bool stop = _host != nullptr && next != nullptr && UndoManager::CommandNeedsRenderStopped(*next);
    SubmitResult r;
    {
        SessionApply a(*this, stop);
        r = _session->Undo();
    }
    Post([this] { _app.OnUndoStateChanged(); });
    return r.ok;
}

bool CollabController::Redo() {
    Entry en(*this);
    Flush();
    if (!_session || !_session->IsActive()) {
        return false;
    }
    _haveLastFlush = false;
    const SequenceCommand* next = _session->NextUndoCommand(true);
    bool stop = _host != nullptr && next != nullptr && UndoManager::CommandNeedsRenderStopped(*next);
    SubmitResult r;
    {
        SessionApply a(*this, stop);
        r = _session->Redo();
    }
    Post([this] { _app.OnUndoStateChanged(); });
    return r.ok;
}

bool CollabController::CanUndo() const {
    return _session && _session->IsActive() && (_session->CanUndo() || !_local.empty());
}

bool CollabController::CanRedo() const {
    return _session && _session->IsActive() && _local.empty() && _session->CanRedo();
}

std::string CollabController::UndoLabel() const {
    std::string l;
    if (!_local.empty()) {
        l = CommandLabel(*_local.back().first);
    } else if (_session && _session->IsActive()) {
        l = _session->UndoLabel();
    }
    return l.empty() ? std::string("Undo") : "Undo: " + l;
}

std::string CollabController::RedoLabel() const {
    std::string l = CanRedo() ? _session->RedoLabel() : std::string();
    return l.empty() ? std::string("Redo") : "Redo: " + l;
}

// ---------------------------------------------------------------------------
// Participants and presence
// ---------------------------------------------------------------------------

ParticipantId CollabController::LocalParticipant() const {
    return _session ? _session->Participant() : kNoParticipant;
}

std::vector<Participant> CollabController::Participants() const {
    std::vector<Participant> out;
    if (_state == State::Ended || _state == State::Idle) {
        return out;
    }
    ParticipantId me = LocalParticipant();
    for (const auto& r : _roster) {
        Participant p;
        p.id = r.participant;
        p.name = r.name;
        p.deviceKind = r.deviceKind;
        p.colour = ParticipantColour(r.participant);
        p.isHost = r.participant == kHostParticipant;
        p.isLocal = r.participant == me;
        out.push_back(std::move(p));
    }
    return out;
}

std::optional<Participant> CollabController::FindParticipant(ParticipantId id) const {
    for (const auto& p : Participants()) {
        if (p.id == id) {
            return p;
        }
    }
    return std::nullopt;
}

std::string CollabController::NameOf(ParticipantId id) const {
    for (const auto& r : _roster) {
        if (r.participant == id) {
            return r.name;
        }
    }
    return {};
}

void CollabController::SetLocalPresence(const PresenceInfo& p) {
    Entry en(*this);
    if (!(p == _localPresence)) {
        _lastActivity = _now;
    }
    _localPresence = p;
    _presenceSet = true;
    TrySendPresence();
}

void CollabController::TrySendPresence() {
    if (!_presenceSet || !_session || !_session->IsActive()) {
        return;
    }
    PresenceInfo eff = _localPresence;
    if (eff.softLock != kNoSeqObjectId) {
        bool selected = std::find(eff.selectedEffects.begin(), eff.selectedEffects.end(), eff.softLock) != eff.selectedEffects.end();
        if (!selected || _now - _lastActivity >= _opts.softLockIdleMs) {
            eff.softLock = kNoSeqObjectId;
        }
    }
    if (_presenceSent && eff == _sentPresence) {
        return;
    }
    if (_presenceSent && _now - _lastPresenceSent < _opts.presenceIntervalMs) {
        return; // Tick sends the newest once the interval has passed
    }
    _session->SetPresence(eff);
    _sentPresence = eff;
    _presenceSent = true;
    _lastPresenceSent = _now;
}

const PresenceInfo* CollabController::PresenceOf(ParticipantId who) const {
    auto it = _presence.find(who);
    return it == _presence.end() ? nullptr : &it->second;
}

ParticipantId CollabController::SoftLockHolder(SeqObjectId effect) const {
    if (effect == kNoSeqObjectId) {
        return kNoParticipant;
    }
    ParticipantId me = LocalParticipant();
    for (const auto& [id, p] : _presence) {
        if (id != me && p.softLock == effect) {
            return id;
        }
    }
    return kNoParticipant;
}

std::vector<ParticipantId> CollabController::SelectedBy(SeqObjectId effect) const {
    std::vector<ParticipantId> out;
    ParticipantId me = LocalParticipant();
    for (const auto& [id, p] : _presence) {
        if (id != me && std::find(p.selectedEffects.begin(), p.selectedEffects.end(), effect) != p.selectedEffects.end()) {
            out.push_back(id);
        }
    }
    return out;
}

// ---------------------------------------------------------------------------
// Status
// ---------------------------------------------------------------------------

std::string CollabController::EndText() const {
    if (!_endText.empty()) {
        return _endText;
    }
    const std::string& r = _endReason;
    if (r.empty()) {
        return {};
    }
    if (r == Reason::Ended) {
        return _role == Role::Host ? "You ended the session." : "The host ended the session.";
    }
    if (r == Reason::HostLost) {
        return "Lost the connection to the host.";
    }
    if (r == Reason::Left) {
        return "You left the session.";
    }
    if (r == Reason::Full) {
        return "The session is full.";
    }
    if (r == Reason::Protocol) {
        return "This session uses a different version of xLights. Update every device to the same version.";
    }
    if (r == Reason::SnapshotFailed) {
        return "The sequence couldn't be received from the host.";
    }
    if (r == Reason::FilesUnavailable) {
        return "Some show files couldn't be fetched from the host.";
    }
    return "The session ended (" + r + ").";
}

std::string CollabController::StatusText() const {
    switch (_state) {
    case State::Idle:
        return "Not in a session";
    case State::Connecting:
        return "Connecting...";
    case State::Joining:
        return (_client != nullptr && _client->GetState() == CollabClient::State::Checking) ? "Checking show files..." : "Joining...";
    case State::Syncing:
        return _everActive ? "Re-syncing the sequence..." : "Receiving the sequence...";
    case State::Active: {
        size_t others = _roster.empty() ? 0 : _roster.size() - 1;
        if (_role == Role::Host) {
            if (others == 0) {
                return "Hosting - no one has joined yet";
            }
            return "Hosting with " + std::to_string(others) + (others == 1 ? " other" : " others");
        }
        std::string host = NameOf(kHostParticipant);
        if (host.empty()) {
            host = _hostPeerName;
        }
        std::string s = host.empty() ? std::string("In a session") : "In " + host + "'s session";
        if (others > 1) {
            s += " with " + std::to_string(others - 1) + (others - 1 == 1 ? " other" : " others");
        }
        return s;
    }
    case State::Ended:
        return "Session ended: " + EndText();
    }
    return {};
}

bool CollabController::KeepSessionCopy(std::string& folder) {
    if (_client == nullptr || !_session || !_joinReport || !_joinReport->sessionCopy) {
        return false;
    }
    _session->KeepSessionFiles(true);
    folder = _session->SessionShowFolder();
    return true;
}

} // namespace Collab
