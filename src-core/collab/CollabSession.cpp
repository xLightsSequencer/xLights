/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "CollabSession.h"

#include <algorithm>
#include <atomic>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <random>
#include <system_error>
#include <thread>

#include "CollabCommandInfo.h"
#include "CollabUidTable.h"
#include "render/Effect.h"
#include "render/SequenceElements.h"
#include "render/SequenceIds.h"
#include "render/SequenceMedia.h"
#include "render/SequenceMediaCommands.h"
#include "render/SequenceMutationGuard.h"
#include "utils/FileUtils.h"

#include <log.h>

using nlohmann::json;

namespace Collab {

namespace {

const char* ReasonFor(CommandStatus s) {
    switch (s) {
    case CommandStatus::TargetMissing: return Reason::TargetGone;
    case CommandStatus::Overlap: return Reason::Overlap;
    case CommandStatus::Locked: return Reason::Locked;
    default: return Reason::Invalid;
    }
}

std::optional<std::string> ValueOf(const SettingsMap& m, const std::string& key) {
    if (!m.Contains(key)) {
        return std::nullopt;
    }
    return m.Get(key, std::string());
}

namespace fs = std::filesystem;

bool ReadWhole(const std::string& path, std::string& out) {
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        return false;
    }
    out.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
    return !f.bad();
}

fs::path Canonical(const std::string& p) {
    std::error_code ec;
    fs::path c = fs::weakly_canonical(fs::path(p), ec);
    return ec ? fs::path(p) : c;
}

void RemoveFile(const std::string& path) {
    if (!path.empty()) {
        std::error_code ec;
        fs::remove(path, ec);
    }
}

std::string CommandSummary(const json& j) {
    std::string type = j.is_object() && j.contains("type") && j["type"].is_string() ? j["type"].get<std::string>() : "?";
    if (type == "Batch" && j.contains("commands") && j["commands"].is_array()) {
        type += "[" + std::to_string(j["commands"].size()) + "]";
    }
    return type;
}

} // namespace

// ---------------------------------------------------------------------------
// CollabParticipant
// ---------------------------------------------------------------------------

CollabParticipant::CollabParticipant(ICollabTransport& transport, ICollabDocument& doc, SessionOptions opts, SessionEvents events) :
    _transport(transport), _doc(doc), _opts(std::move(opts)), _events(std::move(events)) {
}

CollabParticipant::~CollabParticipant() {
    _transport.onMessage = nullptr;
    _transport.onPeer = nullptr;
    _transport.onBlob = nullptr;
    if (_sessionFolder.empty()) {
        return;
    }
    std::error_code ec;
    if (_ownsSessionFolder && !_keepFiles) {
        fs::remove_all(_sessionFolder, ec);
        return;
    }
    for (const char* sub : { "incoming", "outgoing", "compare" }) {
        fs::remove_all(fs::path(_sessionFolder) / sub, ec);
    }
    if (!_keepFiles) {
        fs::remove_all(fs::path(_sessionFolder) / "show", ec);
    }
}

const std::string& CollabParticipant::SessionFolder() {
    if (_sessionFolder.empty()) {
        std::error_code ec;
        if (!_opts.sessionFolder.empty()) {
            _sessionFolder = _opts.sessionFolder;
        } else {
            std::random_device rd;
            _sessionFolder = (fs::temp_directory_path(ec) / ("xl_collab_" + std::to_string(rd()) + "_" + std::to_string(rd()))).string();
            _ownsSessionFolder = true;
        }
        fs::create_directories(_sessionFolder, ec);
    }
    return _sessionFolder;
}

std::string CollabParticipant::SessionShowFolder() {
    return (fs::path(SessionFolder()) / "show").string();
}

std::string CollabParticipant::NewSessionFile(const std::string& sub, const std::string& ext) {
    fs::path dir = fs::path(SessionFolder()) / sub;
    std::error_code ec;
    fs::create_directories(dir, ec);
    return (dir / ("f" + std::to_string(++_sessionFiles) + ext)).string();
}

void CollabParticipant::OnBlobArrived(PeerId from, BlobId id, const std::string& path) {
    std::string owned;
    if (!path.empty()) {
        owned = NewSessionFile("incoming");
        if (!MoveFileTo(path, owned)) {
            owned.clear();
        }
    }
    auto key = std::make_pair(from, id);
    auto info = _blobInfos.find(key);
    if (info != _blobInfos.end()) {
        Message m = std::move(info->second);
        _blobInfos.erase(info);
        OnBlobReady(from, m, owned);
        return;
    }
    if (_blobFiles.count(key)) {
        RemoveFile(owned);
        return;
    }
    _blobFiles[key] = owned;
    _blobFileOrder.push_back(key);
    // Blobs nobody names (a lost or hostile message) don't pile up.
    while (_blobFileOrder.size() > 64) {
        auto old = _blobFileOrder.front();
        _blobFileOrder.pop_front();
        auto it = _blobFiles.find(old);
        if (it != _blobFiles.end()) {
            RemoveFile(it->second);
            _blobFiles.erase(it);
        }
    }
}

void CollabParticipant::OnBlobInfo(PeerId from, const Message& m) {
    if (m.blob == 0) {
        OnBlobReady(from, m, "");
        return;
    }
    auto key = std::make_pair(from, (BlobId)m.blob);
    auto file = _blobFiles.find(key);
    if (file != _blobFiles.end()) {
        std::string path = std::move(file->second);
        _blobFiles.erase(file);
        OnBlobReady(from, m, path);
        return;
    }
    if (_blobInfos.count(key)) {
        return;
    }
    _blobInfos[key] = m;
    _blobInfoOrder.push_back(key);
    while (_blobInfoOrder.size() > 4096) {
        _blobInfos.erase(_blobInfoOrder.front());
        _blobInfoOrder.pop_front();
    }
}

BlobId CollabParticipant::SendFile(PeerId to, const FileEntry& e, const std::string& local) {
    Message m;
    m.type = MsgType::File;
    m.blob = NextBlob();
    m.files = { e };
    _transport.SendBlob(to, m.blob, local);
    _transport.Send(to, m);
    return m.blob;
}

void CollabParticipant::SendFileUnavailable(PeerId to, const std::string& path, const std::string& reason) {
    Message m;
    m.type = MsgType::File;
    FileEntry e;
    e.path = path;
    m.files = { e };
    m.reason = reason;
    _transport.Send(to, m);
}

bool CollabParticipant::ShareFile(const std::string& reference) {
    if (!IsActive()) {
        return false;
    }
    std::string local = ResolvePath(CurrentShowFolder(), reference);
    FileEntry e;
    e.path = reference;
    e.role = FileRole::Media;
    if (local.empty() || CachePathFor(reference).empty() || !_hashes.Hash(local, e.size, e.hash)) {
        Log("can't share '" + reference + "': not found on this device");
        return false;
    }
    SendShared(e, local);
    return true;
}

void CollabParticipant::ShareCommandFiles(const SequenceCommand& cmd) {
    if (cmd.Type() == CommandType::PutMedia) {
        const auto& state = static_cast<const PutMediaCommand&>(cmd).GetState();
        if (!state.embedded) {
            ShareFile(state.key);
        }
    } else if (cmd.Type() == CommandType::Batch) {
        for (const auto& c : static_cast<const BatchCommand&>(cmd).GetCommands()) {
            ShareCommandFiles(*c);
        }
    }
}

void CollabParticipant::EnsureMediaFolder() {
    if (!_mediaFolderAdded) {
        _mediaFolderAdded = true;
        _doc.AddMediaFolder(SessionShowFolder());
    }
}

CollabParticipant::Stored CollabParticipant::StoreReceivedFile(const FileEntry& e, const std::string& path, bool docLoaded, bool addMediaFolder,
                                                               std::string& local) {
    local.clear();
    uint64_t size = 0;
    uint64_t hash = 0;
    std::string cachePath = CachePathFor(e.path);
    if (path.empty() || e.role != FileRole::Media || cachePath.empty() || !HashFile(path, size, hash) || size != e.size || hash != e.hash ||
        size > _opts.maxFileBytes) {
        Log("dropped a file sent for '" + e.path + "': missing, damaged or not acceptable");
        RemoveFile(path);
        return Stored::Bad;
    }
    std::string mine = ResolvePath(CurrentShowFolder(), e.path);
    uint64_t mineSize = 0;
    uint64_t mineHash = 0;
    if (!mine.empty() && _hashes.Hash(mine, mineSize, mineHash) && mineSize == size && mineHash == hash) {
        RemoveFile(path);
        local = mine;
        return Stored::AlreadyHad;
    }
    std::string dest = (fs::path(SessionShowFolder()) / cachePath).string();
    if (!mine.empty() && Canonical(mine) != Canonical(dest)) {
        // The show folder is searched before any media folder, so this
        // device's own (different) file is what renders.
        Log("'" + e.path + "' differs from this device's own copy, which stays in use");
        RemoveFile(path);
        local = mine;
        return Stored::Shadowed;
    }
    if (!MoveFileTo(path, dest)) {
        Log("could not store the file sent for '" + e.path + "'");
        RemoveFile(path);
        return Stored::Bad;
    }
    local = dest;
    _received[e.path] = dest;
    if (addMediaFolder) {
        EnsureMediaFolder();
    }
    FileUtils::ClearNonExistentFiles();
    if (docLoaded) {
        SequenceMedia& media = _doc.Sequence().GetSequenceMedia();
        media.ClearMissingImages();
        if (media.HasMedia(e.path)) {
            media.ReloadMedia(e.path);
        }
    }
    if (_events.onFileArrived) {
        _events.onFileArrived(e.path, dest);
    }
    return Stored::Stored;
}

SubmitResult CollabParticipant::Undo() {
    if (_undo.empty() || !IsActive()) {
        return { false, 0, "nothing_to_undo" };
    }
    UndoEntry e = std::move(_undo.back());
    _undo.pop_back();
    return SubmitKind(std::move(e.cmd), EditKind::Undo, e.rev);
}

SubmitResult CollabParticipant::Redo() {
    if (_redo.empty() || !IsActive()) {
        return { false, 0, "nothing_to_redo" };
    }
    UndoEntry e = std::move(_redo.back());
    _redo.pop_back();
    return SubmitKind(std::move(e.cmd), EditKind::Redo, e.rev);
}

void CollabParticipant::RecordOwnCommit(EditKind kind, Rev rev, const SequenceCommand& inverse) {
    if (kind == EditKind::Edit) {
        _redo.clear();
    }
    if (IsNoOp(inverse)) {
        return;
    }
    UndoEntry e{ rev, inverse.Clone() };
    if (kind == EditKind::Undo) {
        _redo.push_back(std::move(e));
    } else {
        _undo.push_back(std::move(e));
    }
}

uint64_t CollabParticipant::HashAfter(const SequenceCommand& cmd) {
    SequenceElements& seq = _doc.Sequence();
    _hasher.Invalidate(seq, cmd);
    uint64_t h = _hasher.Hash(seq);
    if (_opts.verifyHashes) {
        uint64_t full = DocHasher::FullHash(seq);
        if (full != h) {
            ++_hashVerifyFailures;
            Log("incremental document hash is stale after " + std::string(CommandTypeName(cmd.Type())));
            _hasher.InvalidateAll();
            h = full;
        }
    }
    return h;
}

void CollabParticipant::Log(const std::string& s) {
    std::string line = "Collab[" + std::to_string(_participant) + "] " + s;
    spdlog::debug("{}", line);
    if (_events.onLog) {
        _events.onLog(line);
    }
}

void CollabParticipant::NoteRecent(const CommitInfo& c, const json& cmd) {
    _recent.push_back("rev " + std::to_string(c.rev) + " by " + std::to_string(c.author) + ": " + CommandSummary(cmd));
    while (_recent.size() > _opts.driftLogCommands) {
        _recent.pop_front();
    }
}

std::string CollabParticipant::RecentCommands() const {
    std::string out;
    for (const auto& s : _recent) {
        out += "\n    " + s;
    }
    return out;
}

// ---------------------------------------------------------------------------
// CollabHost
// ---------------------------------------------------------------------------

CollabHost::CollabHost(ICollabTransport& transport, ICollabDocument& doc, SessionOptions opts, SessionEvents events) :
    CollabParticipant(transport, doc, std::move(opts), std::move(events)) {
    _transport.onMessage = [this](PeerId from, const Message& m) { OnMessage(from, m); };
    _transport.onPeer = [this](PeerId peer, PeerEvent e) { OnPeer(peer, e); };
    _transport.onBlob = [this](PeerId from, BlobId id, const std::string& path) {
        if (FindClient(from) != nullptr) {
            OnBlobArrived(from, id, path);
        }
    };
}

CollabHost::~CollabHost() = default;

void CollabHost::Start() {
    _started = true;
    _endReason.clear();
    _participant = kHostParticipant;
    SeqIds::SetPeer(kHostParticipant);
    _hasher.InvalidateAll();
    _uids.Reset(_doc.Sequence());
}

void CollabHost::End() {
    if (!_started) {
        return;
    }
    Message bye;
    bye.type = MsgType::Bye;
    bye.reason = Reason::Ended;
    for (const auto& [peer, c] : _clients) {
        _transport.Send(peer, bye);
    }
    _clients.clear();
    _endReason = Reason::Ended;
    _started = false;
}

size_t CollabHost::ClientCount() const {
    return _clients.size();
}

std::vector<ParticipantId> CollabHost::Clients() const {
    std::vector<ParticipantId> out;
    for (const auto& [peer, c] : _clients) {
        out.push_back(c.id);
    }
    return out;
}

CollabHost::Client* CollabHost::FindClient(PeerId peer) {
    auto it = _clients.find(peer);
    return it == _clients.end() ? nullptr : &it->second;
}

SubmitResult CollabHost::Submit(std::unique_ptr<SequenceCommand> cmd) {
    return SubmitKind(std::move(cmd), EditKind::Edit, 0);
}

SubmitResult CollabHost::SubmitKind(std::unique_ptr<SequenceCommand> cmd, EditKind kind, Rev undoOf) {
    if (!IsActive() || cmd == nullptr) {
        return { false, 0, "not_active" };
    }
    json j;
    cmd->Serialize(j);
    uint64_t seq = _nextOwnSeq++;
    Outcome o = Process(kHostParticipant, seq, _applied, kind, undoOf, j);
    if (!o.committed) {
        if (_events.onReject) {
            _events.onReject(seq, kind, o.reason, o.by);
        }
        return { false, seq, o.reason };
    }
    ShareCommandFiles(*cmd);
    return { true, seq, "" };
}

CollabHost::Outcome CollabHost::Process(ParticipantId author, uint64_t clientSeq, Rev baseRev, EditKind kind, Rev undoOf, const json& cmdJson) {
    Outcome o;
    auto reject = [&](const char* reason, std::string text, ParticipantId by = kNoParticipant) {
        o.reason = reason;
        o.text = std::move(text);
        o.by = by;
        ++_rejectCounts[reason];
        if (_rejectLog.size() < 10000) {
            _rejectLog.push_back({ reason, o.text });
        }
        return o;
    };
    SequenceElements& seq = _doc.Sequence();
    // Everyone applies the decoded command, the host included.
    std::unique_ptr<SequenceCommand> cmd = DeserializeCommand(cmdJson);
    if (cmd == nullptr) {
        return reject(Reason::Invalid, "unreadable command");
    }
    if (auto pruned = WithoutRedundantDeletes(seq, *cmd)) {
        cmd = std::move(pruned);
    }
    if (kind == EditKind::Edit) {
        for (SeqObjectId uid : KeyWriteTargets(*cmd)) {
            auto it = _touch.find(TypeItem(uid));
            if (it != _touch.end() && it->second.rev > baseRev && it->second.author != author) {
                return reject(Reason::StaleType, "the effect's type changed since this edit was made", it->second.author);
            }
        }
    } else {
        auto r = _revs.find(undoOf);
        bool valid = r != _revs.end() && r->second.first == author &&
                     (kind == EditKind::Undo ? r->second.second != EditKind::Undo : r->second.second == EditKind::Undo);
        if (!valid) {
            return reject(Reason::Invalid, "not an undoable commit of this participant");
        }
        for (const auto& item : ConflictItems(seq, *cmd)) {
            auto it = _touch.find(item);
            if (it != _touch.end() && it->second.rev > undoOf && it->second.author != author) {
                return reject(Reason::ChangedSince, "changed since by another participant", it->second.author);
            }
        }
    }
    // A rejected command must leave no trace, not even in the effect order
    // the .xsq is written in (which clients never see change).
    LayerOrderSnapshot order(seq, *cmd);
    CommandResult result;
    std::unique_ptr<SequenceCommand> inverse = cmd->Apply(seq, ApplyMode::Strict, &result);
    if (inverse == nullptr) {
        order.Restore(seq);
        _hasher.InvalidateAll();
        return reject(ReasonFor(result.status), result.message);
    }
    SeqObjectId duplicate = kNoSeqObjectId;
    if (!_uids.Commit(*cmd, *inverse, &duplicate)) {
        // Ids are allocated per participant and should never collide; take it back.
        inverse->Apply(seq, ApplyMode::Lenient);
        order.Restore(seq);
        _hasher.InvalidateAll();
        return reject(Reason::Invalid, "object id " + std::to_string(duplicate) + " is already in use");
    }

    Rev rev = ++_applied;
    // Serialized after applying: commands that create rows record the ids
    // they gave them, so every peer creates the same ids.
    json applied;
    cmd->Serialize(applied);
    for (const auto& item : TouchedItems(seq, *cmd, *inverse)) {
        _touch[item] = { rev, author };
    }
    _revs[rev] = { author, kind };

    Message m;
    m.type = MsgType::Commit;
    m.rev = rev;
    m.participant = author;
    m.clientSeq = clientSeq;
    m.kind = kind;
    m.undoOf = undoOf;
    m.command = std::move(applied);
    m.docHash = HashAfter(*cmd);
    if (_opts.verifyHashes && !(_uids == UidRegistry::AllUids(seq))) {
        ++_hashVerifyFailures;
        Log("id registry is stale after " + std::string(CommandTypeName(cmd->Type())));
        _uids.Reset(seq);
    }
    for (auto& [peer, c] : _clients) {
        _transport.Send(peer, m);
    }
    CommitInfo info{ rev, author, clientSeq, kind, undoOf };
    NoteRecent(info, m.command);
    _log.push_back({ std::move(m) });
    if (author == kHostParticipant) {
        RecordOwnCommit(kind, rev, *inverse);
    }
    if (_events.onCommit) {
        _events.onCommit(info, *cmd);
    }
    TrimLog();
    o.committed = true;
    o.rev = rev;
    return o;
}

void CollabHost::TrimLog() {
    Rev minApplied = _applied;
    for (const auto& [peer, c] : _clients) {
        minApplied = std::min(minApplied, c.applied);
    }
    while (_log.size() > _opts.commitLogLimit && (_log.front().msg.rev <= minApplied || _log.size() > 4 * _opts.commitLogLimit)) {
        _log.pop_front();
        ++_logBase;
    }
}

void CollabHost::OnPeer(PeerId peer, PeerEvent e) {
    if (e == PeerEvent::Lost || e == PeerEvent::Left) {
        DropClient(peer, e == PeerEvent::Lost ? "lost" : "left");
    }
}

void CollabHost::DropClient(PeerId peer, const std::string& why) {
    auto it = _clients.find(peer);
    if (it == _clients.end()) {
        return;
    }
    ParticipantId id = it->second.id;
    std::string name = it->second.name;
    _clients.erase(it);
    Log("participant " + std::to_string(id) + " " + why);
    if (_events.onParticipant) {
        _events.onParticipant(id, name, false);
    }
}

void CollabHost::OnMessage(PeerId from, const Message& m) {
    if (!IsActive()) {
        if (m.type == MsgType::Hello) {
            Message refuse;
            refuse.type = MsgType::Refuse;
            refuse.reason = Reason::Ended;
            refuse.text = "This session has ended.";
            _transport.Send(from, refuse);
        }
        return;
    }
    if (m.type == MsgType::Hello) {
        OnHello(from, m);
        return;
    }
    Client* c = FindClient(from);
    if (c == nullptr) {
        return;
    }
    c->lastHeard = _now;
    switch (m.type) {
    case MsgType::Propose:
        OnPropose(*c, m);
        break;
    case MsgType::NeedSnapshot:
        Log("participant " + std::to_string(c->id) + " asked for a snapshot: " + m.reason);
        SendSnapshot(*c);
        break;
    case MsgType::NeedFiles:
        OnNeedFiles(*c, m);
        break;
    case MsgType::File:
        OnBlobInfo(from, m);
        break;
    case MsgType::Resend:
        SendCommitsFrom(*c, m.rev);
        break;
    case MsgType::Pong:
        c->applied = std::max(c->applied, std::min(m.rev, _applied));
        break;
    case MsgType::Presence: {
        Message relay = m;
        relay.participant = c->id;
        for (const auto& [peer, other] : _clients) {
            if (peer != from) {
                _transport.Send(peer, relay);
            }
        }
        if (_events.onPresence) {
            _events.onPresence(c->id, relay.presence);
        }
        break;
    }
    case MsgType::Bye:
        // A refused joiner says why (its show files don't match).
        DropClient(from, m.reason.empty() || m.reason == Reason::Left ? std::string("left") : "left: " + m.reason);
        break;
    default:
        break;
    }
}

void CollabHost::OnHello(PeerId from, const Message& m) {
    Message refuse;
    refuse.type = MsgType::Refuse;
    if (m.protocol != kProtocolVersion) {
        refuse.reason = Reason::Protocol;
        refuse.text = "This session uses a different version of the collaboration protocol. Update xLights on every device to the same version and try again.";
        _transport.Send(from, refuse);
        return;
    }
    if (Client* existing = FindClient(from)) {
        // A repeated Hello: our Welcome or the snapshot got lost.
        existing->lastHeard = _now;
        Message w;
        w.type = MsgType::Welcome;
        w.participant = existing->id;
        w.rev = _applied;
        w.name = _opts.displayName;
        _transport.Send(from, w);
        SendManifest(*existing, false);
        return;
    }
    if (_clients.size() + 1 >= _opts.maxParticipants || _nextParticipant == 0xFFFF) {
        refuse.reason = Reason::Full;
        refuse.text = "This session is full.";
        _transport.Send(from, refuse);
        return;
    }
    Client& c = _clients[from];
    c.peer = from;
    c.id = _nextParticipant++;
    c.name = m.name;
    c.lastHeard = _now;
    c.lastPing = _now;
    c.applied = _applied;
    Message w;
    w.type = MsgType::Welcome;
    w.participant = c.id;
    w.rev = _applied;
    w.name = _opts.displayName;
    _transport.Send(from, w);
    SendManifest(c, true);
    Log("participant " + std::to_string(c.id) + " joined at rev " + std::to_string(_applied));
    if (_events.onParticipant) {
        _events.onParticipant(c.id, c.name, true);
    }
}

void CollabHost::SendSnapshot(Client& c) {
    std::string xsq;
    if (!_doc.SaveXsq(xsq)) {
        Log("could not save a snapshot");
        return;
    }
    std::string file = NewSessionFile("outgoing", ".xsq");
    {
        std::ofstream f(file, std::ios::binary);
        f << xsq;
        if (!f) {
            Log("could not write a snapshot");
            RemoveFile(file);
            return;
        }
    }
    Message s;
    s.type = MsgType::Snapshot;
    SequenceElements& seq = _doc.Sequence();
    s.rev = _applied;
    s.docHash = _hasher.Hash(seq);
    s.uids = CaptureUids(seq);
    s.clientSeq = c.nextSeq - 1;
    s.blob = NextBlob();
    c.applied = std::max(c.applied, _applied);
    _transport.SendBlob(c.peer, s.blob, file);
    RemoveFile(file);
    _transport.Send(c.peer, s);
}

void CollabHost::SendManifest(Client& c, bool rebuild) {
    if (rebuild) {
        c.files.clear();
        std::string show = _doc.ShowFolder();
        c.manifest = show.empty() ? std::vector<FileEntry>()
                                  : BuildManifest(CollectSessionFiles(show, _doc.Sequence(), _doc.AudioFiles()), &_hashes, &c.files);
    }
    Message m;
    m.type = MsgType::Manifest;
    m.files = c.manifest;
    _transport.Send(c.peer, m);
}

void CollabHost::OnNeedFiles(Client& c, const Message& m) {
    std::map<std::string, const FileEntry*> entries;
    for (const auto& e : c.manifest) {
        entries[e.path] = &e;
    }
    std::set<std::string> done;
    for (const auto& path : m.paths) {
        if (!done.insert(path).second) {
            continue;
        }
        auto e = entries.find(path);
        auto local = c.files.find(path);
        // Only files this client's manifest lists are ever sent.
        if (e == entries.end() || local == c.files.end()) {
            SendFileUnavailable(c.peer, path, Reason::NotInManifest);
            continue;
        }
        FileEntry now = *e->second;
        if (!_hashes.Hash(local->second, now.size, now.hash)) {
            SendFileUnavailable(c.peer, path, Reason::Unreadable);
            continue;
        }
        SendFile(c.peer, now, local->second);
    }
}

void CollabHost::OnBlobReady(PeerId from, const Message& info, const std::string& path) {
    Client* c = FindClient(from);
    if (c == nullptr || info.type != MsgType::File || info.files.size() != 1) {
        RemoveFile(path);
        return;
    }
    const FileEntry& e = info.files[0];
    std::string local;
    Stored r = StoreReceivedFile(e, path, true, true, local);
    if (r != Stored::Stored && r != Stored::AlreadyHad) {
        return;
    }
    Log("participant " + std::to_string(c->id) + " sent '" + e.path + "'");
    for (const auto& [peer, other] : _clients) {
        if (peer != from) {
            SendFile(peer, e, local);
        }
    }
}

void CollabHost::SendShared(const FileEntry& e, const std::string& local) {
    for (const auto& [peer, c] : _clients) {
        SendFile(peer, e, local);
    }
}

void CollabHost::SendCommitsFrom(Client& c, Rev from) {
    if (from <= _logBase) {
        SendSnapshot(c);
        return;
    }
    for (const auto& r : _log) {
        if (r.msg.rev >= from) {
            _transport.Send(c.peer, r.msg);
        }
    }
}

void CollabHost::OnPropose(Client& c, const Message& m) {
    if (faults.hostIgnoresClientOrder) {
        if (c.faultSeen.insert(m.clientSeq).second) {
            c.nextSeq = std::max(c.nextSeq, m.clientSeq + 1);
            ProcessProposal(c, m);
        }
        return;
    }
    if (m.clientSeq < c.nextSeq) {
        // Already dealt with. A commit reaches the author through the
        // commit stream; a reject is sent again.
        auto it = c.rejects.find(m.clientSeq);
        if (it != c.rejects.end()) {
            _transport.Send(c.peer, it->second);
        }
        return;
    }
    if (m.clientSeq > c.nextSeq) {
        if (c.early.size() < 100000) {
            c.early.emplace(m.clientSeq, m);
        }
        return;
    }
    ProcessProposal(c, m);
    ++c.nextSeq;
    for (auto it = c.early.begin(); it != c.early.end() && it->first <= c.nextSeq;) {
        if (it->first == c.nextSeq) {
            Message next = std::move(it->second);
            it = c.early.erase(it);
            ProcessProposal(c, next);
            ++c.nextSeq;
        } else {
            it = c.early.erase(it);
        }
    }
}

void CollabHost::ProcessProposal(Client& c, const Message& m) {
    // A proposal can't be based on a rev the host hasn't made yet.
    Rev base = std::min(m.baseRev, _applied);
    Outcome o = Process(c.id, m.clientSeq, base, m.kind, m.undoOf, m.command);
    if (o.committed) {
        return;
    }
    Message r;
    r.type = MsgType::Reject;
    r.clientSeq = m.clientSeq;
    r.rev = _applied;
    r.kind = m.kind;
    r.reason = o.reason;
    r.text = o.text;
    r.participant = o.by;
    c.rejects[m.clientSeq] = r;
    while (c.rejects.size() > 1000) {
        c.rejects.erase(c.rejects.begin());
    }
    _transport.Send(c.peer, r);
}

void CollabHost::SetPresence(const PresenceInfo& p) {
    Message m;
    m.type = MsgType::Presence;
    m.participant = kHostParticipant;
    m.presence = p;
    for (const auto& [peer, c] : _clients) {
        _transport.Send(peer, m);
    }
}

void CollabHost::Tick(uint64_t nowMs) {
    _now = std::max(_now, nowMs);
    if (!IsActive()) {
        return;
    }
    std::vector<PeerId> gone;
    for (auto& [peer, c] : _clients) {
        if (_now - c.lastHeard > _opts.peerTimeoutMs) {
            gone.push_back(peer);
            continue;
        }
        if (_now - c.lastPing >= _opts.pingIntervalMs) {
            c.lastPing = _now;
            Message p;
            p.type = MsgType::Ping;
            p.rev = _applied;
            p.time = _now;
            _transport.Send(peer, p);
        }
    }
    for (PeerId p : gone) {
        DropClient(p, "timed out");
    }
}

// ---------------------------------------------------------------------------
// CollabClient
// ---------------------------------------------------------------------------

CollabClient::CollabClient(ICollabTransport& transport, ICollabDocument& doc, SessionOptions opts, SessionEvents events) :
    CollabParticipant(transport, doc, std::move(opts), std::move(events)) {
    _transport.onMessage = [this](PeerId from, const Message& m) { OnMessage(from, m); };
    _transport.onPeer = [this](PeerId peer, PeerEvent e) { OnPeer(peer, e); };
    _transport.onBlob = [this](PeerId from, BlobId id, const std::string& path) {
        if (from == _host && _state != State::Ended && _state != State::Idle) {
            OnBlobArrived(from, id, path);
        }
    };
}

CollabClient::~CollabClient() = default;

void CollabClient::Send(const Message& m) {
    _transport.Send(_host, m);
}

void CollabClient::Join(PeerId host) {
    _host = host;
    _state = State::Joining;
    _haveManifest = false;
    _checks.clear();
    _wanted.clear();
    _fetched.clear();
    _unavailable.clear();
    _compareOnly = false;
    _report = {};
    _lastHeard = _now;
    _lastHello = _now;
    Message h;
    h.type = MsgType::Hello;
    h.appVersion = _opts.appVersion;
    h.name = _opts.displayName;
    h.deviceKind = _opts.deviceKind;
    Send(h);
}

void CollabClient::Leave() {
    if (_state == State::Ended || _state == State::Idle) {
        return;
    }
    Message b;
    b.type = MsgType::Bye;
    b.reason = Reason::Left;
    Send(b);
    EndSession(Reason::Left);
}

void CollabClient::EndSession(const std::string& reason) {
    if (_state == State::Ended) {
        return;
    }
    _state = State::Ended;
    _endReason = reason;
    Log("session ended: " + reason);
    if (_events.onEnded) {
        _events.onEnded(reason);
    }
}

void CollabClient::OnPeer(PeerId peer, PeerEvent e) {
    if (peer == _host && (e == PeerEvent::Lost || e == PeerEvent::Left)) {
        EndSession(Reason::HostLost);
    }
}

void CollabClient::OnMessage(PeerId from, const Message& m) {
    if (from != _host || _state == State::Ended || _state == State::Idle) {
        return;
    }
    _lastHeard = _now;
    switch (m.type) {
    case MsgType::Welcome:
        if (_state == State::Joining) {
            _participant = m.participant;
            SeqIds::SetPeer(faults.clientSharesHostIdPrefix ? kHostParticipant : m.participant);
            _state = State::Checking;
            _lastHello = _now;
            _hostHead = std::max(_hostHead, m.rev);
        }
        break;
    case MsgType::Refuse:
        _endText = m.text;
        EndSession(m.reason.empty() ? std::string(Reason::Ended) : m.reason);
        break;
    case MsgType::Manifest:
        OnManifest(m);
        break;
    case MsgType::Snapshot:
        if (_state == State::Active && m.rev > _applied) {
            // The host answered a resend with a snapshot: it no longer keeps
            // the commits this client lacks (a new host's log starts late).
            Log("the host sent a snapshot at rev " + std::to_string(m.rev) + " for commits it no longer has");
            _state = State::AwaitingSnapshot;
            ++_resyncs;
            _lastSnapshotRequest = _now;
        }
        OnBlobInfo(from, m);
        break;
    case MsgType::File:
        OnBlobInfo(from, m);
        break;
    case MsgType::Commit:
        OnCommit(m);
        break;
    case MsgType::Reject:
        OnReject(m);
        break;
    case MsgType::Ping: {
        _hostHead = std::max(_hostHead, m.rev);
        Message p;
        p.type = MsgType::Pong;
        p.rev = _applied;
        p.time = m.time;
        Send(p);
        if (_state == State::Active && _hostHead > _applied) {
            RequestResend();
        }
        break;
    }
    case MsgType::Presence:
        if (_events.onPresence) {
            _events.onPresence(m.participant, m.presence);
        }
        break;
    case MsgType::Bye:
        EndSession(m.reason.empty() ? std::string(Reason::Ended) : m.reason);
        break;
    default:
        break;
    }
}

std::string CollabClient::CurrentShowFolder() {
    return _usingCopy ? SessionShowFolder() : _doc.ShowFolder();
}

void CollabClient::OnBlobReady(PeerId /*from*/, const Message& info, const std::string& path) {
    if (info.type == MsgType::Snapshot) {
        std::string xsq;
        bool ok = !path.empty() && ReadWhole(path, xsq);
        RemoveFile(path);
        // A failed transfer is asked for again by the snapshot retry.
        if (ok) {
            OnSnapshot(info, xsq);
        }
        return;
    }
    if (info.type != MsgType::File || info.files.size() != 1) {
        RemoveFile(path);
        return;
    }
    const FileEntry& e = info.files[0];
    if (_state == State::Checking && _wanted.count(e.path)) {
        OnJoinFile(info, path);
        return;
    }
    // While still checking, whether a media folder is needed isn't known
    // yet (AskForSnapshot adds it then).
    bool decided = _state == State::Active || _state == State::AwaitingSnapshot;
    std::string local;
    StoreReceivedFile(e, path, _state == State::Active, decided && !_usingCopy, local);
}

void CollabClient::SendShared(const FileEntry& e, const std::string& local) {
    SendFile(_host, e, local);
}

void CollabClient::OnManifest(const Message& m) {
    if (_state != State::Checking || _haveManifest) {
        return;
    }
    _haveManifest = true;
    if (_doc.ShowFolder().empty()) {
        Log("this document has no show folder to compare; files not checked");
        FinishCheck();
        return;
    }
    _checks = CompareManifest(_doc.ShowFolder(), m.files, &_hashes);
    bool layout = false;
    for (const auto& c : _checks) {
        if (c.state == FileState::Match) {
            continue;
        }
        layout = layout || c.entry.role == FileRole::Layout;
        if (_opts.fetchFiles || c.entry.role == FileRole::Layout) {
            _wanted[c.entry.path] = c.entry;
        }
    }
    _compareOnly = !_opts.fetchFiles && layout;
    std::string summary = std::to_string(_checks.size()) + " show files listed";
    Log(summary + ", " + std::to_string(_wanted.size()) + " to fetch" + (_compareOnly ? " (to compare the layout)" : ""));
    if (!_wanted.empty()) {
        RequestFiles();
        return;
    }
    FinishCheck();
}

void CollabClient::RequestFiles() {
    _lastFileRequest = _now == 0 ? 1 : _now;
    Message n;
    n.type = MsgType::NeedFiles;
    for (const auto& [path, e] : _wanted) {
        n.paths.push_back(path);
    }
    Send(n);
}

void CollabClient::OnJoinFile(const Message& info, const std::string& path) {
    const FileEntry want = _wanted[info.files[0].path];
    uint64_t size = 0;
    uint64_t hash = 0;
    std::string cachePath = CachePathFor(want.path);
    bool ok = info.reason.empty() && !path.empty() && !cachePath.empty() && HashFile(path, size, hash) && size == want.size && hash == want.hash;
    if (!ok) {
        // Not sent, damaged, or changed on the host since its manifest.
        Log("could not fetch '" + want.path + "'" + (info.reason.empty() ? std::string() : ": " + info.reason));
        _unavailable.push_back(want.path);
        RemoveFile(path);
    } else {
        fs::path root = _compareOnly ? fs::path(SessionFolder()) / "compare" : fs::path(SessionShowFolder());
        std::string dest = (root / cachePath).string();
        if (MoveFileTo(path, dest)) {
            _fetched[want.path] = dest;
        } else {
            _unavailable.push_back(want.path);
            RemoveFile(path);
        }
    }
    _wanted.erase(want.path);
    if (_wanted.empty()) {
        FinishCheck();
    }
}

void CollabClient::FinishCheck() {
    _report = {};
    _report.showFolder = _doc.ShowFolder();
    std::set<std::string> unavailable(_unavailable.begin(), _unavailable.end());
    std::string layoutText;
    size_t layoutChanges = 0;
    std::string audioText;
    std::string lostText;
    for (const auto& c : _checks) {
        if (c.state == FileState::Match) {
            continue;
        }
        const std::string& p = c.entry.path;
        switch (c.entry.role) {
        case FileRole::Layout: {
            auto f = _fetched.find(p);
            std::string line;
            if (c.state == FileState::Missing) {
                line = p + " is missing on this device";
                ++layoutChanges;
            } else if (f == _fetched.end()) {
                line = p + " differs from the host's (the host's copy could not be fetched to compare)";
                ++layoutChanges;
            } else {
                std::string mine;
                std::string host;
                ReadWhole(c.local, mine);
                ReadWhole(f->second, host);
                LayoutDifference d = DescribeLayoutDifference(p, mine, host);
                layoutChanges += d.changes;
                if (d.changes != 0) {
                    line = p + ": " + d.text;
                }
            }
            if (!line.empty()) {
                layoutText += (layoutText.empty() ? "" : "\n") + line;
            }
            break;
        }
        case FileRole::Audio:
            audioText += (audioText.empty() ? "" : ", ") + p + (c.state == FileState::Missing ? " (missing)" : " (different)");
            break;
        case FileRole::Media:
            (c.state == FileState::Missing ? _report.missingMedia : _report.differentMedia).push_back(p);
            break;
        }
        if (unavailable.count(p) && c.entry.role != FileRole::Media) {
            lostText += (lostText.empty() ? "" : ", ") + p;
        }
    }
    _report.layoutDifference = layoutText;
    if (!_opts.fetchFiles) {
        if (layoutChanges != 0) {
            RefuseJoin(Reason::LayoutDiffers, "This device's layout differs from the host's:\n" + layoutText);
            return;
        }
        if (!audioText.empty()) {
            RefuseJoin(Reason::AudioDiffers, "This device's audio differs from the host's: " + audioText);
            return;
        }
        AskForSnapshot();
        return;
    }
    if (!lostText.empty()) {
        RefuseJoin(Reason::FilesUnavailable, "These files could not be copied from the host: " + lostText);
        return;
    }
    if (_fetched.empty()) {
        // Everything matched, or only media the host couldn't send.
        AskForSnapshot();
        return;
    }
    // Open against a copy of the host's show: what matched comes from this
    // device's show folder, the rest is what was fetched.
    std::string root = SessionShowFolder();
    std::vector<std::string> copyFailed;
    for (const auto& c : _checks) {
        if (c.state != FileState::Match) {
            continue;
        }
        std::string cachePath = CachePathFor(c.entry.path);
        if (cachePath.empty() || !CopyFileTo(c.local, (fs::path(root) / cachePath).string())) {
            if (c.entry.role != FileRole::Media) {
                RefuseJoin(Reason::FilesUnavailable, "Could not make a copy of " + c.entry.path + " for the session.");
                return;
            }
            copyFailed.push_back(c.entry.path);
        }
    }
    if (!_doc.UseShowFolder(root)) {
        RefuseJoin(Reason::FilesUnavailable, "This device can't open the session against a copy of the host's show.");
        return;
    }
    _usingCopy = true;
    _report.showFolder = root;
    _report.sessionCopy = true;
    for (const auto& [p, dest] : _fetched) {
        _report.fetched.push_back(p);
    }
    // The copy holds what matched and what was fetched; media that could be
    // neither is missing from it.
    _report.differentMedia.clear();
    _report.missingMedia.clear();
    for (const auto& c : _checks) {
        if (c.entry.role == FileRole::Media && c.state != FileState::Match && !_fetched.count(c.entry.path)) {
            _report.missingMedia.push_back(c.entry.path);
        }
    }
    _report.missingMedia.insert(_report.missingMedia.end(), copyFailed.begin(), copyFailed.end());
    AskForSnapshot();
}

void CollabClient::RefuseJoin(const std::string& reason, const std::string& text) {
    Log("not joining: " + text);
    _report.refusal = text;
    _endText = text;
    Message b;
    b.type = MsgType::Bye;
    b.reason = reason;
    b.text = text;
    Send(b);
    if (_events.onJoinChecked) {
        _events.onJoinChecked(_report);
    }
    EndSession(reason);
}

void CollabClient::AskForSnapshot() {
    if (!_usingCopy && !_received.empty()) {
        EnsureMediaFolder();
    }
    _state = State::AwaitingSnapshot;
    _lastSnapshotRequest = _now;
    Message n;
    n.type = MsgType::NeedSnapshot;
    n.reason = "join";
    Send(n);
    if (_events.onJoinChecked) {
        _events.onJoinChecked(_report);
    }
}

void CollabClient::OnSnapshot(const Message& m, const std::string& xsq) {
    if (_state != State::AwaitingSnapshot) {
        return;
    }
    if (!_doc.LoadXsq(xsq)) {
        EndSession(Reason::SnapshotFailed);
        return;
    }
    SequenceElements& seq = _doc.Sequence();
    std::string problem;
    if (!ApplyUids(seq, m.uids, &problem)) {
        Log("snapshot ids don't match the loaded sequence: " + problem);
        EndSession(Reason::SnapshotFailed);
        return;
    }
    _hasher.InvalidateAll();
    uint64_t h = _hasher.Hash(seq);
    if (h != m.docHash && !faults.clientIgnoresDrift) {
        Log("snapshot hash " + std::to_string(h) + " differs from the host's " + std::to_string(m.docHash) + "\n" + DocHasher::Components(seq));
        EndSession(Reason::SnapshotFailed);
        return;
    }
    bool resync = _applied != 0 || _resyncs != 0;
    _applied = m.rev;
    _hostHead = std::max(_hostHead, m.rev);
    // Proposals the host had dealt with are in the snapshot (or rejected).
    for (auto it = _queuedRejects.begin(); it != _queuedRejects.end();) {
        if (it->clientSeq <= m.clientSeq) {
            if (_pending.count(it->clientSeq) && _events.onReject) {
                _events.onReject(it->clientSeq, it->kind, it->reason, it->participant);
            }
            it = _queuedRejects.erase(it);
        } else {
            ++it;
        }
    }
    for (auto it = _pending.begin(); it != _pending.end() && it->first <= m.clientSeq;) {
        it = _pending.erase(it);
    }
    for (auto it = _overlay.begin(); it != _overlay.end();) {
        it = it->second.seq <= m.clientSeq ? _overlay.erase(it) : std::next(it);
    }
    for (auto it = _buffered.begin(); it != _buffered.end() && it->first <= m.rev;) {
        it = _buffered.erase(it);
    }
    _state = State::Active;
    LowerOverlay();
    if (resync && _events.onResync) {
        _events.onResync();
    }
    DrainCommits();
}

void CollabClient::RequestResync(const std::string& why) {
    if (_state != State::Active) {
        return;
    }
    Log("asking for a snapshot: " + why + RecentCommands());
    _state = State::AwaitingSnapshot;
    ++_resyncs;
    _lastSnapshotRequest = _now;
    Message n;
    n.type = MsgType::NeedSnapshot;
    n.reason = why;
    Send(n);
}

void CollabClient::RequestResend() {
    if (_lastResendRequest != 0 && _now - _lastResendRequest < _opts.retryMs) {
        return;
    }
    _lastResendRequest = _now == 0 ? 1 : _now;
    Message r;
    r.type = MsgType::Resend;
    r.rev = _applied + 1;
    Send(r);
}

void CollabClient::OnCommit(const Message& m) {
    if (_state != State::Active && _state != State::AwaitingSnapshot) {
        return;
    }
    if (m.rev <= _applied) {
        return;
    }
    _hostHead = std::max(_hostHead, m.rev);
    if (faults.clientAppliesOutOfOrder && _state == State::Active) {
        ApplyCommit(m);
        return;
    }
    _buffered.emplace(m.rev, m);
    if (_state == State::Active) {
        DrainCommits();
    }
}

void CollabClient::DrainCommits() {
    while (_state == State::Active) {
        auto it = _buffered.find(_applied + 1);
        if (it == _buffered.end()) {
            break;
        }
        Message m = std::move(it->second);
        _buffered.erase(it);
        if (!ApplyCommit(m)) {
            break;
        }
    }
    for (auto it = _buffered.begin(); it != _buffered.end() && it->first <= _applied;) {
        it = _buffered.erase(it);
    }
    if (_state == State::Active && !_buffered.empty()) {
        RequestResend();
    }
    DrainRejects();
}

void CollabClient::SetKey(const EffectLocator& loc, bool palette, const std::string& key, const std::optional<std::string>& value) {
    SettingsChange change{ false, { { key, value } } };
    ModifyEffectCommand cmd(loc, std::nullopt, palette ? SettingsChange{} : change, palette ? change : SettingsChange{});
    SequenceElements& seq = _doc.Sequence();
    if (cmd.Apply(seq, ApplyMode::Lenient) != nullptr) {
        _hasher.Invalidate(seq, cmd);
    }
}

void CollabClient::LiftOverlay() {
    SequenceElements& seq = _doc.Sequence();
    for (const auto& [k, e] : _overlay) {
        if (FindEffect(seq, e.loc) != nullptr) {
            SetKey(e.loc, std::get<1>(k), std::get<2>(k), e.host);
        }
    }
}

void CollabClient::LowerOverlay() {
    SequenceElements& seq = _doc.Sequence();
    for (auto it = _overlay.begin(); it != _overlay.end();) {
        Effect* e = FindEffect(seq, it->second.loc);
        if (e == nullptr) {
            it = _overlay.erase(it);
            continue;
        }
        bool palette = std::get<1>(it->first);
        const std::string& key = std::get<2>(it->first);
        it->second.host = ValueOf(palette ? e->GetPaletteMap() : e->GetSettings(), key);
        if (it->second.host != it->second.local) {
            SetKey(it->second.loc, palette, key, it->second.local);
        }
        ++it;
    }
}

bool CollabClient::ApplyCommit(const Message& m) {
    SequenceElements& seq = _doc.Sequence();
    std::unique_ptr<SequenceCommand> cmd = DeserializeCommand(m.command);
    if (cmd == nullptr) {
        RequestResync("unreadable commit at rev " + std::to_string(m.rev));
        return false;
    }
    if (!faults.clientSkipsRebase) {
        LiftOverlay();
    }
    CommandResult result;
    std::unique_ptr<SequenceCommand> inverse = cmd->Apply(seq, ApplyMode::Strict, &result);
    if (inverse == nullptr && faults.clientIgnoresDrift) {
        _applied = std::max(_applied, m.rev);
        _pending.erase(m.clientSeq);
        if (!faults.clientSkipsRebase) {
            LowerOverlay();
        }
        return true;
    }
    if (inverse == nullptr) {
        RequestResync("commit at rev " + std::to_string(m.rev) + " (" + CommandSummary(m.command) + ") did not apply: " + result.message);
        return false;
    }
    _applied = std::max(_applied, m.rev);
    uint64_t hash = HashAfter(*cmd);
    if (m.participant == _participant) {
        _pending.erase(m.clientSeq);
        for (auto it = _overlay.begin(); it != _overlay.end();) {
            it = it->second.seq == m.clientSeq ? _overlay.erase(it) : std::next(it);
        }
        RecordOwnCommit(m.kind, m.rev, *inverse);
    } else {
        // A pending key write made before someone else changed the effect's
        // type will be rejected (stale_type); the host's value stands.
        std::set<std::string> touched = TouchedItems(seq, *cmd, *inverse);
        for (auto it = _overlay.begin(); it != _overlay.end();) {
            it = touched.count(TypeItem(std::get<0>(it->first))) ? _overlay.erase(it) : std::next(it);
        }
    }
    if (!faults.clientSkipsRebase) {
        LowerOverlay();
    }
    CommitInfo info{ m.rev, m.participant, m.clientSeq, m.kind, m.undoOf };
    NoteRecent(info, m.command);
    if (_events.onCommit) {
        _events.onCommit(info, *cmd);
    }
    if (!faults.clientIgnoresDrift && hash != m.docHash) {
        Log("document parts after rev " + std::to_string(m.rev) + ":\n" + DocHasher::Components(seq));
        RequestResync("document hash " + std::to_string(hash) + " differs from the host's " + std::to_string(m.docHash) + " after rev " +
                      std::to_string(m.rev));
        return false;
    }
    return true;
}

void CollabClient::OnReject(const Message& m) {
    if (_state != State::Active && _state != State::AwaitingSnapshot) {
        return;
    }
    // Deal with it once every commit before it has been applied, so a
    // fallback to the host's value is the value at that point.
    if (m.rev > _applied || _state != State::Active) {
        _queuedRejects.push_back(m);
        return;
    }
    HandleReject(m);
}

void CollabClient::DrainRejects() {
    if (_state != State::Active || _queuedRejects.empty()) {
        return;
    }
    std::vector<Message> later;
    std::vector<Message> now;
    for (auto& r : _queuedRejects) {
        (r.rev <= _applied ? now : later).push_back(std::move(r));
    }
    _queuedRejects = std::move(later);
    for (const auto& r : now) {
        HandleReject(r);
    }
}

void CollabClient::HandleReject(const Message& m) {
    auto p = _pending.find(m.clientSeq);
    if (p == _pending.end()) {
        return;
    }
    _pending.erase(p);
    SequenceElements& seq = _doc.Sequence();
    for (auto it = _overlay.begin(); it != _overlay.end();) {
        if (it->second.seq != m.clientSeq) {
            ++it;
            continue;
        }
        if (FindEffect(seq, it->second.loc) != nullptr) {
            SetKey(it->second.loc, std::get<1>(it->first), std::get<2>(it->first), it->second.host);
        }
        it = _overlay.erase(it);
    }
    Log("proposal " + std::to_string(m.clientSeq) + " rejected: " + m.reason);
    if (_events.onReject) {
        _events.onReject(m.clientSeq, m.kind, m.reason, m.participant);
    }
}

SubmitResult CollabClient::Submit(std::unique_ptr<SequenceCommand> cmd) {
    return SubmitKind(std::move(cmd), EditKind::Edit, 0);
}

SubmitResult CollabClient::SubmitKind(std::unique_ptr<SequenceCommand> cmd, EditKind kind, Rev undoOf) {
    if (_state != State::Active || cmd == nullptr) {
        return { false, 0, "not_active" };
    }
    SequenceElements& seq = _doc.Sequence();
    if (kind == EditKind::Edit) {
        if (auto keys = AsKeyChanges(seq, *cmd)) {
            cmd = std::move(keys);
        }
        if (cmd->Type() != CommandType::Batch) {
            CommandResult r = cmd->Validate(seq, ApplyMode::Strict);
            if (!r.Ok()) {
                return { false, 0, ReasonFor(r.status) };
            }
        }
    }
    uint64_t s = _nextSeq++;
    bool optimistic = kind == EditKind::Edit && IsOptimistic(*cmd);
    if (optimistic) {
        const auto& mc = static_cast<const ModifyEffectCommand&>(*cmd);
        Effect* e = FindEffect(seq, mc.Target());
        if (e == nullptr) {
            return { false, 0, Reason::TargetGone };
        }
        std::vector<OverlayKey> keys;
        for (int pal = 0; pal < 2; ++pal) {
            const SettingsChange& ch = pal ? mc.GetPalette() : mc.GetSettings();
            const SettingsMap& map = pal ? e->GetPaletteMap() : e->GetSettings();
            for (const auto& [k, v] : ch.keys) {
                OverlayKey key{ e->GetUid(), pal != 0, k };
                auto [it, inserted] = _overlay.try_emplace(key);
                if (inserted) {
                    it->second.loc = mc.Target();
                    it->second.host = ValueOf(map, k);
                }
                it->second.local = v;
                it->second.seq = s;
            }
        }
        if (cmd->Apply(seq, ApplyMode::Strict) == nullptr) {
            // Validated above; can't happen. Fall back to the host's values.
            for (auto it = _overlay.begin(); it != _overlay.end();) {
                if (it->second.seq == s) {
                    SetKey(it->second.loc, std::get<1>(it->first), std::get<2>(it->first), it->second.host);
                    it = _overlay.erase(it);
                } else {
                    ++it;
                }
            }
            return { false, 0, Reason::Invalid };
        }
        _hasher.Invalidate(seq, *cmd);
    }
    Message m;
    m.type = MsgType::Propose;
    m.clientSeq = s;
    m.baseRev = _applied;
    m.kind = kind;
    m.undoOf = undoOf;
    cmd->Serialize(m.command);
    _pending[s] = { m, _now };
    Send(m);
    ShareCommandFiles(*cmd);
    return { true, s, "", optimistic };
}

void CollabClient::SetPresence(const PresenceInfo& p) {
    if (_state != State::Active) {
        return;
    }
    Message m;
    m.type = MsgType::Presence;
    m.participant = _participant;
    m.presence = p;
    Send(m);
}

void CollabClient::Tick(uint64_t nowMs) {
    _now = std::max(_now, nowMs);
    switch (_state) {
    case State::Joining:
        if (_now - _lastHello >= _opts.retryMs) {
            _lastHello = _now;
            Message h;
            h.type = MsgType::Hello;
            h.appVersion = _opts.appVersion;
            h.name = _opts.displayName;
            h.deviceKind = _opts.deviceKind;
            Send(h);
        }
        break;
    case State::Checking:
        if (!_haveManifest && _now - _lastHello >= 4 * _opts.retryMs) {
            // The Welcome came but the manifest didn't: ask again.
            _lastHello = _now;
            Message h;
            h.type = MsgType::Hello;
            h.appVersion = _opts.appVersion;
            h.name = _opts.displayName;
            h.deviceKind = _opts.deviceKind;
            Send(h);
        } else if (!_wanted.empty() && _now - _lastFileRequest >= _opts.fileRetryMs) {
            RequestFiles();
        }
        break;
    case State::AwaitingSnapshot:
        if (_now - _lastSnapshotRequest >= 4 * _opts.retryMs) {
            _lastSnapshotRequest = _now;
            Message n;
            n.type = MsgType::NeedSnapshot;
            n.reason = "retry";
            Send(n);
        }
        break;
    case State::Active:
        for (auto& [s, p] : _pending) {
            if (_now - p.lastSent >= _opts.retryMs) {
                p.lastSent = _now;
                Send(p.msg);
            }
        }
        if (!_buffered.empty() || _hostHead > _applied) {
            RequestResend();
        }
        break;
    default:
        return;
    }
    if (_now - _lastHeard > _opts.peerTimeoutMs) {
        EndSession(Reason::HostLost);
    }
}

} // namespace Collab
