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
#include "render/Element.h"
#include "render/SequenceElementCommands.h"
#include "render/SequenceElements.h"
#include "render/SequenceIds.h"
#include "render/SequenceMedia.h"
#include "render/SequenceMediaCommands.h"
#include "render/SequenceMutationGuard.h"
#include "utils/FileUtils.h"

#include <log.h>

using nlohmann::json;

namespace Collab {

// Show files hashed on a worker thread; the session thread polls `done`.
struct HashJob {
    std::atomic<bool> done{ false };
    HashCache cache;
    std::vector<SessionFile> files;          // host: what to list
    std::vector<FileEntry> manifest;         // host: the result
    std::map<std::string, std::string> local; // host: manifest path -> file
    std::vector<FileCheck> checks;           // client: resolved, to hash
};

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

std::string CommandLabel(const SequenceCommand& cmd) {
    if (cmd.Type() == CommandType::Batch) {
        const auto& b = static_cast<const BatchCommand&>(cmd);
        if (!b.GetLabel().empty()) {
            return b.GetLabel();
        }
        if (b.GetCommands().size() == 1) {
            return CommandLabel(*b.GetCommands().front());
        }
        return "Changes";
    }
    switch (cmd.Type()) {
    case CommandType::AddEffect: return "Effect(s) Added";
    case CommandType::DeleteEffect: return "Effect(s) Deleted";
    case CommandType::MoveEffect: return "Effect(s) Moved";
    case CommandType::ModifyEffect:
    case CommandType::ReplaceLayerContents: return "Effect(s) Modified";
    case CommandType::AddLayer: return "Layer(s) Added";
    case CommandType::RemoveLayer: return "Layer(s) Removed";
    case CommandType::SetLayerName: return "Layer Renamed";
    case CommandType::PutMedia:
    case CommandType::RemoveMedia:
    case CommandType::RenameMedia: return "Media Changed";
    default: break;
    }
    const char* l = ElementCommandUndoLabel(cmd.Type());
    return (l != nullptr && *l != 0) ? std::string(l) : std::string("Changes");
}

// ---------------------------------------------------------------------------
// CollabParticipant
// ---------------------------------------------------------------------------

CollabParticipant::CollabParticipant(ICollabTransport& transport, ICollabDocument& doc, SessionOptions opts, SessionEvents events) :
    _transport(transport), _doc(doc), _opts(std::move(opts)), _events(std::move(events)) {
}

CollabParticipant::~CollabParticipant() {
    if (_ownsHandlers) {
        _transport.onMessage = nullptr;
        _transport.onPeer = nullptr;
        _transport.onBlob = nullptr;
        _transport.onBlobProgress = nullptr;
        _transport.onBlobFailed = nullptr;
        _transport.onBlobDone = nullptr;
        _transport.onSendFailed = nullptr;
    }
    if (_sessionFolder.empty()) {
        return;
    }
    std::error_code ec;
    if (!_keepFiles) {
        fs::remove_all(_sessionFolder, ec);
        return;
    }
    for (const char* sub : { "incoming", "outgoing", "compare", "held" }) {
        fs::remove_all(fs::path(_sessionFolder) / sub, ec);
    }
}

const std::string& CollabParticipant::SessionFolder() {
    if (_sessionFolder.empty()) {
        std::error_code ec;
        // A folder of its own even under a given parent: one session's
        // cleanup must never reach another's files, a kept copy included.
        fs::path parent = _opts.sessionFolder.empty() ? fs::temp_directory_path(ec) : fs::path(_opts.sessionFolder);
        std::random_device rd;
        fs::path dir;
        do {
            dir = parent / ("xl_collab_" + std::to_string(rd()) + "_" + std::to_string(rd()));
        } while (fs::exists(dir, ec));
        _sessionFolder = dir.string();
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

void CollabParticipant::InstallHandlers() {
    _transport.onMessage = [this](PeerId from, const Message& m) { OnTransportMessage(from, m); };
    _transport.onPeer = [this](PeerId peer, PeerEvent e) { OnTransportPeer(peer, e); };
    _transport.onBlob = [this](PeerId from, BlobId id, const std::string& path) {
        _diskErrorShown = false;
        OnTransportBlob(from, id, path);
    };
    _transport.onBlobProgress = [this](PeerId from, BlobId id, uint64_t received, uint64_t size) { OnBlobProgress(from, id, received, size); };
    _transport.onBlobFailed = [this](PeerId from, BlobId id, const std::string& why) { OnBlobFailed(from, id, why); };
    _transport.onBlobDone = [this](PeerId to, BlobId id, bool sent) { OnBlobDone(to, id, sent); };
    _transport.onSendFailed = [this](PeerId to, const Message& m, const std::string& reason) { OnSendFailed(to, m, reason); };
}

void CollabParticipant::OnBlobFailed(PeerId /*from*/, BlobId /*id*/, const std::string& why) {
    Log(why);
    // Its retry may well fail the same way: say it once, until a file gets
    // through again.
    if (_diskErrorShown) {
        return;
    }
    _diskErrorShown = true;
    if (_events.onNotice) {
        _events.onNotice(Reason::DiskError, "A file from the session couldn't be saved on this device. Check that it has free disk space; the file will be asked for again.");
    }
}

bool CollabParticipant::WriteParts(const std::string& path, const std::vector<const std::string*>& parts) {
    std::ofstream f(path, std::ios::binary | std::ios::trunc);
    for (const std::string* p : parts) {
        uint64_t n = p->size();
        unsigned char len[8];
        for (int i = 0; i < 8; ++i) {
            len[i] = (unsigned char)(n >> (8 * i));
        }
        f.write((const char*)len, 8);
        f.write(p->data(), (std::streamsize)p->size());
    }
    f.close();
    return !f.fail();
}

bool CollabParticipant::ReadParts(const std::string& path, std::vector<std::string>& parts) {
    std::string all;
    if (!ReadWhole(path, all)) {
        return false;
    }
    size_t at = 0;
    for (auto& p : parts) {
        if (all.size() - at < 8) {
            return false;
        }
        uint64_t n = 0;
        for (int i = 0; i < 8; ++i) {
            n |= (uint64_t)(unsigned char)all[at + i] << (8 * i);
        }
        at += 8;
        if (n > all.size() - at) {
            return false;
        }
        p.assign(all, at, (size_t)n);
        at += (size_t)n;
    }
    return at == all.size();
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
    if (!IsActive() || !CanUndo()) {
        return { false, 0, "nothing_to_undo" };
    }
    if (UndoWaits() || !_queuedUndo.empty()) {
        SubmitResult r{ true, 0, "" };
        r.deferred = true;
        if (!_queuedUndo.empty() && _queuedUndo.back() == EditKind::Redo) {
            _queuedUndo.pop_back();
        } else {
            _queuedUndo.push_back(EditKind::Undo);
        }
        return r;
    }
    return DoUndo(false);
}

SubmitResult CollabParticipant::Redo() {
    if (!IsActive()) {
        return { false, 0, "nothing_to_redo" };
    }
    if (UndoWaits() || !_queuedUndo.empty()) {
        SubmitResult r{ true, 0, "" };
        r.deferred = true;
        if (!_queuedUndo.empty() && _queuedUndo.back() == EditKind::Undo) {
            _queuedUndo.pop_back();
        } else {
            _queuedUndo.push_back(EditKind::Redo);
        }
        return r;
    }
    return DoUndo(true);
}

SubmitResult CollabParticipant::DoUndo(bool redo) {
    auto& stack = redo ? _redo : _undo;
    if (stack.empty()) {
        return { false, 0, redo ? "nothing_to_redo" : "nothing_to_undo" };
    }
    UndoEntry e = std::move(stack.back());
    stack.pop_back();
    _submitMeta = { e.label, 0 };
    return SubmitKind(std::move(e.cmd), redo ? EditKind::Redo : EditKind::Undo, e.rev);
}

void CollabParticipant::RunQueuedUndo() {
    while (!_queuedUndo.empty() && IsActive() && !UndoWaits()) {
        EditKind k = _queuedUndo.front();
        _queuedUndo.pop_front();
        DoUndo(k == EditKind::Redo);
    }
}

SubmitResult CollabParticipant::SubmitJoined(std::unique_ptr<SequenceCommand> cmd, uint64_t joinUndoTo) {
    _submitMeta = { "", joinUndoTo };
    return Submit(std::move(cmd));
}

void CollabParticipant::NoteOwnProposal(uint64_t seq) {
    if (!_submitMeta.label.empty() || _submitMeta.joinTo != 0) {
        _ownMeta[seq] = std::move(_submitMeta);
    }
    _submitMeta = {};
    while (_ownMeta.size() > 10000) {
        _ownMeta.erase(_ownMeta.begin());
    }
}

void CollabParticipant::RecordOwnCommit(EditKind kind, Rev rev, uint64_t seq, const SequenceCommand& applied, const SequenceCommand& inverse) {
    OwnMeta meta;
    auto m = _ownMeta.find(seq);
    if (m != _ownMeta.end()) {
        meta = std::move(m->second);
        _ownMeta.erase(m);
    }
    if (kind == EditKind::Edit) {
        _redo.clear();
        if (meta.label.empty()) {
            meta.label = CommandLabel(applied);
        }
    }
    if (IsNoOp(inverse)) {
        return;
    }
    if (kind == EditKind::Edit && meta.joinTo != 0 && !_undo.empty() && _undo.back().seq == meta.joinTo) {
        UndoEntry& top = _undo.back();
        std::vector<std::unique_ptr<SequenceCommand>> both;
        both.push_back(inverse.Clone());
        both.push_back(std::move(top.cmd));
        top.cmd = std::make_unique<BatchCommand>(top.label, std::move(both));
        top.seq = seq;
        return;
    }
    UndoEntry e{ rev, inverse.Clone(), seq, std::move(meta.label) };
    if (kind == EditKind::Undo) {
        _redo.push_back(std::move(e));
    } else {
        _undo.push_back(std::move(e));
    }
}

void CollabParticipant::NotifyCommit(const CommitInfo& info, const SequenceCommand& applied, const SequenceCommand& inverse) {
    if (_events.onCommit) {
        _events.onCommit(info, applied);
    }
    if (_events.onCommitApplied) {
        _events.onCommitApplied(info, applied, inverse);
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

void CollabParticipant::NoteCommitted(ParticipantId author, uint64_t clientSeq) {
    uint64_t& last = _lastSeq[author];
    last = std::max(last, clientSeq);
}

void CollabParticipant::TakeStateFrom(CollabParticipant& from) {
    _participant = from._participant;
    _applied = from._applied;
    _now = from._now;
    _undo = std::move(from._undo);
    _redo = std::move(from._redo);
    _hasher = std::move(from._hasher);
    _hasher.InvalidateAll();
    _recent = std::move(from._recent);
    _hashes = std::move(from._hashes);
    _received = std::move(from._received);
    _mediaFolderAdded = from._mediaFolderAdded;
    _roster = std::move(from._roster);
    _assignments = std::move(from._assignments);
    _joinRole = from._joinRole;
    _ownMeta = std::move(from._ownMeta);
    _queuedUndo = std::move(from._queuedUndo);
    _liveLayout = from._liveLayout;
    _layout = std::move(from._layout);
    _layoutHash = from._layoutHash;
    _touch = std::move(from._touch);
    _revs = std::move(from._revs);
    _lastSeq = std::move(from._lastSeq);
    _log = std::move(from._log);
    _logBase = from._logBase;
    _succession = std::move(from._succession);
    _epoch = from._epoch;
    _nextParticipant = from._nextParticipant;
    _sessionFolder = std::move(from._sessionFolder);
    _keepFiles = from._keepFiles;
    _nextBlob = from._nextBlob;
    _sessionFiles = from._sessionFiles;
    from._sessionFolder.clear();
    from._ownsHandlers = false;
}

void CollabParticipant::SetLayout(const LayoutFiles& files) {
    _layout = files;
    _layoutHash = LayoutDelta::Hash(_layout);
}

EditRefusal CollabParticipant::CheckOwnEdit(const SequenceCommand& cmd) {
    AssignmentIndex index;
    if (!_assignments.empty()) {
        index.Build(_doc.Sequence(), _assignments);
    }
    return CheckEdit(_doc.Sequence(), _roster, index, _participant, cmd);
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
    InstallHandlers();
}

void CollabHost::OnTransportBlob(PeerId from, BlobId id, const std::string& path) {
    if (FindClient(from) != nullptr) {
        OnBlobArrived(from, id, path);
    }
}

// Whether a blob sent at `at` that has not been reported done should no
// longer stop another copy. Not every transport learns about every lost
// transfer, and this host can't see the receiver's progress. The client asks
// again only after nothing has moved for blobStallMs, so a request that late
// means the transfer is stuck; an earlier one is a duplicate.
bool CollabHost::Stale(uint64_t at) const {
    return _now - at >= _opts.blobStallMs;
}

void CollabHost::OnBlobDone(PeerId to, BlobId id, bool sent) {
    Client* c = FindClient(to);
    if (c == nullptr) {
        return;
    }
    if (c->snapshotBlob == id) {
        c->snapshotBlob = 0;
    }
    if (c->welcomeBlob == id && !sent) {
        c->welcomeBlob = 0;
    }
    for (auto it = c->filesSending.begin(); it != c->filesSending.end();) {
        it = it->second.first == id ? c->filesSending.erase(it) : std::next(it);
    }
    if (!sent) {
        Log("a transfer to participant " + std::to_string(c->id) + " failed");
    }
}

void CollabHost::OnSendFailed(PeerId to, const Message& m, const std::string& reason) {
    std::vector<PeerId> targets;
    if (to == kNoPeer) {
        for (const auto& [peer, c] : _clients) {
            targets.push_back(peer);
        }
    } else if (FindClient(to) != nullptr) {
        targets.push_back(to);
    }
    Log("could not send message type " + std::to_string((int)m.type) + ": " + reason);
    for (PeerId peer : targets) {
        Client* c = FindClient(peer);
        if (c == nullptr) {
            continue;
        }
        bool joined = c->active || c->sentDocument;
        if (m.type == MsgType::Commit) {
            // It can't get this commit as a message: the document comes as a
            // snapshot instead, which travels as a blob. One sent lately
            // that has it serves resend requests made before it arrived. A
            // joiner asks for the commits after its snapshot once it has it.
            bool covered = c->snapshotRev >= m.rev && _now - c->snapshotAt < _opts.blobStallMs;
            if (joined && !covered) {
                SendSnapshot(*c);
            }
            continue;
        }
        if (!joined) {
            Message r;
            r.type = MsgType::Refuse;
            r.reason = Reason::TooLarge;
            r.text = "This session is too large to send to this device.";
            _transport.Send(peer, r);
            DropClient(peer, "the session is too large to send to it");
            continue;
        }
        if (_events.onNotice) {
            _events.onNotice(Reason::TooLarge, "Something could not be sent to " + c->name + ": it is too large.");
        }
    }
}

void CollabHost::SendWelcome(Client& c, bool rejoin) {
    Message w;
    w.type = MsgType::Welcome;
    w.participant = c.id;
    w.rev = _applied;
    w.name = _opts.displayName;
    if (rejoin) {
        w.rejoin = true;
        w.clientSeq = LastSeqOf(c.id);
        w.liveLayout = c.liveLayout;
        _transport.Send(c.peer, w);
        return;
    }
    w.secret = c.secret;
    if (c.liveLayout) {
        // The layout can be far larger than a message may be: it goes as a
        // blob that the Welcome names.
        w.liveLayout = true;
        w.layoutHash = _layoutHash;
        // A Hello repeated before the Welcome got there (a slow link) is
        // answered with the blob already sent, not another copy of it.
        if (c.welcomeBlob != 0 && Stale(c.welcomeAt)) {
            c.welcomeBlob = 0;
        }
        if (c.welcomeBlob == 0) {
            std::string file = NewSessionFile("outgoing", ".layout");
            if (!WriteParts(file, { &_layout.rgbEffects, &_layout.networks })) {
                Log("could not write the session layout to send it");
                RemoveFile(file);
                return;
            }
            c.welcomeBlob = NextBlob();
            c.welcomeAt = _now;
            _transport.SendBlob(c.peer, c.welcomeBlob, file);
            RemoveFile(file);
        }
        w.blob = c.welcomeBlob;
    }
    _transport.Send(c.peer, w);
}

CollabHost::CollabHost(ICollabTransport& transport, ICollabDocument& doc, SessionOptions opts, SessionEvents events, CollabClient& from) :
    CollabHost(transport, doc, std::move(opts), std::move(events)) {
    // The document must be exactly the commits in rev order: take this
    // participant's own pending writes back out; they are proposed again.
    from.LiftOverlay();
    from._overlay.clear();
    from._times.clear();
    for (const auto& [seq, p] : from._pending) {
        if (p.msg.layout.is_null()) {
            _ownPending.push_back(p.msg);
        }
    }
    for (const auto& [peer, hello] : from._rejoinHellos) {
        _rejoinHellos.emplace_back(peer, hello);
    }
    _nextOwnSeq = from._nextSeq;
    _formerRoster = from._roster;
    TakeStateFrom(from);
    from._state = CollabClient::State::Ended;
    from._endReason = Reason::HostChanged;
}

CollabHost::~CollabHost() = default;

void CollabHost::Start() {
    _started = true;
    _endReason.clear();
    if (_participant == kNoParticipant) {
        _participant = kHostParticipant;
    }
    SeqIds::SetPeer(_participant);
    _hasher.InvalidateAll();
    _uids.Reset(_doc.Sequence());
    SendRoster();
}

void CollabHost::ResumeAsSuccessor() {
    ++_epoch;
    for (const auto& r : _roster) {
        _nextParticipant = std::max<ParticipantId>(_nextParticipant, r.participant + 1);
    }
    _roster.clear();
    _rankHead = _applied;
    // The old host's claims go with it; everyone else's wait for them to
    // come back (until _rejoinUntil). The host is never a viewer, even if
    // this participant was one.
    if (!_formerRoster.empty()) {
        _assignments = WithoutAssignments(_doc.Sequence(), _assignments, _formerRoster.front().participant);
    }
    _awaitingRejoin.clear();
    for (size_t i = 1; i < _formerRoster.size(); ++i) {
        if (_formerRoster[i].participant != _participant) {
            _awaitingRejoin.insert(_formerRoster[i].participant);
        }
    }
    _rejoinUntil = _now + _opts.peerTimeoutMs;
    if (_liveLayout) {
        _layoutAt.clear();
        _layoutAt[_applied] = _layout;
    }
    Log("taking over as host at rev " + std::to_string(_applied) + " (change " + std::to_string(_epoch) + ")");
    Start();
    // This participant's own proposals the old host never answered. One it
    // had decided (a later one of its commits is in) lost its answer.
    for (const Message& p : _ownPending) {
        if (p.clientSeq <= LastSeqOf(_participant)) {
            _ownMeta.erase(p.clientSeq);
            if (_events.onReject) {
                _events.onReject(p.clientSeq, p.kind, Reason::HostChanged, kNoParticipant);
            }
            continue;
        }
        Outcome o = Process(_participant, p.clientSeq, std::min(p.baseRev, _applied), p.kind, p.undoOf, p.command);
        _ownMeta.erase(p.clientSeq);
        if (!o.committed && _events.onReject) {
            _events.onReject(p.clientSeq, p.kind, o.reason, o.by);
        }
    }
    _ownPending.clear();
    RunQueuedUndo();
    auto hellos = std::move(_rejoinHellos);
    _rejoinHellos.clear();
    for (const auto& [peer, h] : hellos) {
        AddRejoiner(peer, h);
    }
    SendRoster();
}

std::vector<ParticipantId> CollabHost::RankSuccessors() const {
    struct Rank {
        bool editor;
        bool ready;
        bool layoutOk;
        Rev applied;
        ParticipantId id;
    };
    std::vector<Rank> ranks;
    for (const auto& [peer, c] : _clients) {
        // Kept up: had applied what the host had at the previous ping round
        // (so the order doesn't change with every commit on its way).
        bool ready = c.active && c.applied >= _rankHead;
        ranks.push_back({ c.role != ParticipantRole::Viewer, ready, c.liveLayout == _liveLayout, c.active ? c.applied : 0, c.id });
    }
    std::sort(ranks.begin(), ranks.end(), [](const Rank& a, const Rank& b) {
        // Viewers only after every editor: one that takes over becomes an
        // editor (the host always is).
        if (a.editor != b.editor) {
            return a.editor;
        }
        if (a.ready != b.ready) {
            return a.ready;
        }
        if (a.layoutOk != b.layoutOk) {
            return a.layoutOk;
        }
        if (!a.ready && a.applied != b.applied) {
            return a.applied > b.applied;
        }
        return a.id < b.id;
    });
    std::vector<ParticipantId> out;
    for (const auto& r : ranks) {
        out.push_back(r.id);
    }
    return out;
}

void CollabHost::SendRoster() {
    _roster.clear();
    _roster.push_back({ _participant, _opts.displayName, _opts.deviceKind, ParticipantRole::Editor });
    std::vector<RosterEntry> clients;
    for (const auto& [peer, c] : _clients) {
        clients.push_back({ c.id, c.name, c.deviceKind, c.role, c.secretHash });
    }
    std::sort(clients.begin(), clients.end(), [](const RosterEntry& a, const RosterEntry& b) { return a.participant < b.participant; });
    _roster.insert(_roster.end(), clients.begin(), clients.end());
    _ranked = RankSuccessors();
    _succession = _ranked;
    for (const auto& [peer, c] : _clients) {
        SendRosterTo(peer);
    }
    if (_events.onRoster) {
        _events.onRoster(_roster);
    }
}

void CollabHost::SendRosterTo(PeerId peer) {
    Message m;
    m.type = MsgType::Roster;
    m.roster = _roster;
    m.assignments = _assignments;
    m.epoch = _epoch;
    m.nextParticipant = _nextParticipant;
    m.succession = _ranked;
    m.joinRole = _joinRole;
    _transport.Send(peer, m);
}

bool CollabHost::SetRole(ParticipantId who, ParticipantRole role) {
    if (!IsActive() || who == _participant) {
        return false;
    }
    for (auto& [peer, c] : _clients) {
        if (c.id != who) {
            continue;
        }
        if (c.role == role) {
            return true;
        }
        c.role = role;
        if (role == ParticipantRole::Viewer) {
            PruneAssignments(who);
        }
        Log("participant " + std::to_string(who) + " is now " + (role == ParticipantRole::Viewer ? "a viewer" : "an editor"));
        Message n;
        n.type = MsgType::Notice;
        n.reason = Reason::RoleChanged;
        n.text = role == ParticipantRole::Viewer ? "The host made you a viewer: you can watch but not change the sequence"
                                                 : "The host made you an editor: you can change the sequence again";
        _transport.Send(peer, n);
        SendRoster();
        return true;
    }
    return false;
}

void CollabHost::SetJoinRole(ParticipantRole role) {
    if (_joinRole == role) {
        return;
    }
    _joinRole = role;
    if (IsActive()) {
        SendRoster(); // the others keep it in case one of them takes over
    }
}

bool CollabHost::Claim(SeqObjectId element, ParticipantId owner, std::string& reason, std::string& text) {
    if (!IsActive()) {
        reason = "not_active";
        return false;
    }
    if (!ClaimAs(_participant, element, owner, reason, text)) {
        return false;
    }
    SendRoster();
    return true;
}

bool CollabHost::ClaimAs(ParticipantId requester, SeqObjectId element, ParticipantId owner, std::string& reason, std::string& text) {
    reason.clear();
    text.clear();
    std::vector<Assignment> next = ApplyClaim(_doc.Sequence(), _roster, _assignments, requester, element, owner, reason, text);
    if (!reason.empty()) {
        Log("claim by participant " + std::to_string(requester) + " refused: " + text);
        return false;
    }
    _assignments = std::move(next);
    return true;
}

void CollabHost::OnClaim(Client& c, const Message& m) {
    bool changed = false;
    for (const auto& a : m.assignments) {
        std::string reason;
        std::string text;
        if (ClaimAs(c.id, a.element, a.owner, reason, text)) {
            changed = true;
            continue;
        }
        Message n;
        n.type = MsgType::Notice;
        n.reason = reason;
        n.text = text;
        _transport.Send(c.peer, n);
    }
    // Answered with the roster even when refused, so the claimant's view is the host's.
    if (changed) {
        SendRoster();
    } else {
        SendRosterTo(c.peer);
    }
}

bool CollabHost::PruneAssignments(ParticipantId owner) {
    if (_assignments.empty()) {
        return false;
    }
    std::vector<Assignment> next = WithoutAssignments(_doc.Sequence(), _assignments, owner);
    if (next == _assignments) {
        return false;
    }
    _assignments = std::move(next);
    return true;
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
        _submitMeta = {};
        return { false, 0, "not_active" };
    }
    json j;
    cmd->Serialize(j);
    uint64_t seq = _nextOwnSeq++;
    NoteOwnProposal(seq);
    Outcome o = Process(_participant, seq, _applied, kind, undoOf, j);
    if (!o.committed) {
        _ownMeta.erase(seq);
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
    if (!faults.hostIgnoresRoles && RoleOf(author) == ParticipantRole::Viewer) {
        return reject(Reason::Viewer, "viewers can't change the sequence");
    }
    // Everyone applies the decoded command, the host included.
    std::unique_ptr<SequenceCommand> cmd = DeserializeCommand(cmdJson);
    if (cmd == nullptr) {
        return reject(Reason::Invalid, "unreadable command");
    }
    if (auto pruned = WithoutRedundantDeletes(seq, *cmd)) {
        cmd = std::move(pruned);
    }
    if (!faults.hostIgnoresRoles && !_assignments.empty()) {
        AssignmentIndex index;
        index.Build(seq, _assignments);
        EditRefusal r = CheckEdit(seq, _roster, index, author, *cmd);
        if (r.Refused()) {
            return reject(Reason::Assigned, r.row + " is assigned to " + NameOfParticipant(r.owner), r.owner);
        }
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

    o.rev = CommitApplied(author, clientSeq, kind, undoOf, *cmd, *inverse, false);
    o.committed = true;
    return o;
}

Rev CollabHost::CommitApplied(ParticipantId author, uint64_t clientSeq, EditKind kind, Rev undoOf, const SequenceCommand& cmd, const SequenceCommand& inverse,
                              bool lenient) {
    SequenceElements& seq = _doc.Sequence();
    Rev rev = ++_applied;
    // Serialized after applying: commands that create rows record the ids
    // they gave them, so every peer creates the same ids.
    json applied;
    cmd.Serialize(applied);
    for (const auto& item : TouchedItems(seq, cmd, inverse)) {
        _touch[item] = { rev, author };
    }
    _revs[rev] = { author, kind };
    NoteCommitted(author, clientSeq);

    Message m;
    m.type = MsgType::Commit;
    m.rev = rev;
    m.participant = author;
    m.clientSeq = clientSeq;
    m.kind = kind;
    m.undoOf = undoOf;
    m.command = std::move(applied);
    m.lenient = lenient;
    m.docHash = HashAfter(cmd);
    if (_opts.verifyHashes && !(_uids == UidRegistry::AllUids(seq))) {
        ++_hashVerifyFailures;
        Log("id registry is stale after " + std::string(CommandTypeName(cmd.Type())));
        _uids.Reset(seq);
    }
    for (auto& [peer, c] : _clients) {
        _transport.Send(peer, m);
    }
    CommitInfo info{ rev, author, clientSeq, kind, undoOf };
    NoteRecent(info, m.command);
    _log.push_back({ std::move(m) });
    if (author == _participant) {
        RecordOwnCommit(kind, rev, clientSeq, cmd, inverse);
    }
    NotifyCommit(info, cmd, inverse);
    TrimLog();
    if (PruneAssignments()) {
        SendRoster();
    }
    return rev;
}

SubmitResult CollabHost::CommitLocal(std::unique_ptr<SequenceCommand> applied, std::unique_ptr<SequenceCommand> inverse, uint64_t joinUndoTo) {
    if (!IsActive() || applied == nullptr || inverse == nullptr) {
        return { false, 0, "not_active" };
    }
    uint64_t seq = _nextOwnSeq++;
    _submitMeta = { "", joinUndoTo };
    NoteOwnProposal(seq);
    SeqObjectId duplicate = kNoSeqObjectId;
    if (!_uids.Commit(*applied, *inverse, &duplicate)) {
        // The edit is in the document already; keep the registry true to it.
        Log("local edit reused object id " + std::to_string(duplicate));
        _uids.Reset(_doc.Sequence());
    }
    CommitApplied(_participant, seq, EditKind::Edit, 0, *applied, *inverse, true);
    ShareCommandFiles(*applied);
    return { true, seq, "" };
}

void CollabHost::EnableLiveLayout(const LayoutFiles& files) {
    _liveLayout = true;
    SetLayout(files);
    _layoutAt.clear();
    _layoutAt[_applied] = _layout;
}

SubmitResult CollabHost::CommitLayout(const json& delta) {
    if (!IsActive() || !_liveLayout || delta.is_null()) {
        return { false, 0, "not_active" };
    }
    auto next = LayoutDelta::Apply(_layout, delta);
    if (!next) {
        return { false, 0, Reason::Invalid };
    }
    uint64_t seq = _nextOwnSeq++;
    CommitLayoutApplied(_participant, seq, delta, *next, "");
    return { true, seq, "" };
}

Rev CollabHost::CommitLayoutApplied(ParticipantId author, uint64_t clientSeq, const json& delta, const LayoutFiles& next, const std::string& notice) {
    Rev rev = ++_applied;
    SetLayout(next);
    _layoutAt[rev] = _layout;
    NoteCommitted(author, clientSeq);
    // Memory's layout change may have renamed or removed rows.
    SequenceElements& seq = _doc.Sequence();
    _hasher.InvalidateAll();
    _uids.Reset(seq);
    Message m;
    m.type = MsgType::Commit;
    m.rev = rev;
    m.participant = author;
    m.clientSeq = clientSeq;
    m.layout = delta;
    m.layoutHash = _layoutHash;
    m.text = notice;
    m.docHash = _hasher.Hash(seq);
    for (auto& [peer, c] : _clients) {
        _transport.Send(peer, m);
    }
    CommitInfo info{ rev, author, clientSeq, EditKind::Edit, 0 };
    NoteRecent(info, json{ { "type", "Layout" } });
    _log.push_back({ std::move(m) });
    _layoutLog.push_back({ rev, author, LayoutDelta::Describe(delta) });
    while (_layoutLog.size() > 1000) {
        _layoutLog.pop_front();
    }
    TrimLog();
    PruneLayouts();
    if (PruneAssignments()) {
        SendRoster();
    }
    return rev;
}

void CollabHost::PruneLayouts() {
    Rev minApplied = _applied;
    for (const auto& [peer, c] : _clients) {
        minApplied = std::min(minApplied, c.applied);
    }
    // Keep the version every client may still propose against.
    auto keep = _layoutAt.upper_bound(minApplied);
    if (keep == _layoutAt.begin()) {
        return;
    }
    --keep;
    _layoutAt.erase(_layoutAt.begin(), keep);
}

std::string CollabHost::NameOfParticipant(ParticipantId id) const {
    for (const auto& r : _roster) {
        if (r.participant == id) {
            return r.name;
        }
    }
    return {};
}

std::string CollabHost::WhoDeleted(const std::string& record, Rev after) const {
    for (auto it = _layoutLog.rbegin(); it != _layoutLog.rend() && it->rev > after; ++it) {
        for (const auto& t : it->touched) {
            if (t.size() > record.size() + 2 && t.compare(t.size() - record.size() - 1, record.size() + 1, record + "'") == 0 &&
                t[t.size() - record.size() - 2] == '\'') {
                return NameOfParticipant(it->author);
            }
        }
    }
    return {};
}

CollabHost::Outcome CollabHost::ProcessLayout(Client& c, const Message& m) {
    Outcome o;
    auto reject = [&](const char* reason, std::string text) {
        o.reason = reason;
        o.text = std::move(text);
        ++_rejectCounts[reason];
        if (_rejectLog.size() < 10000) {
            _rejectLog.push_back({ reason, o.text });
        }
        return o;
    };
    if (!_liveLayout || !c.liveLayout) {
        return reject(Reason::Invalid, "this session doesn't edit the layout live");
    }
    if (!faults.hostIgnoresRoles && c.role == ParticipantRole::Viewer) {
        return reject(Reason::Viewer, "Viewers can't change the layout, so your layout change was undone");
    }
    if (_events.onBeforeLayoutProposal) {
        _events.onBeforeLayoutProposal();
    }
    Rev baseRev = std::min(m.baseRev, _applied);
    auto bit = _layoutAt.upper_bound(baseRev);
    if (bit == _layoutAt.begin()) {
        return reject(Reason::Invalid, "the layout this change was made against is no longer known");
    }
    --bit;
    const LayoutFiles base = bit->second;
    auto mine = LayoutDelta::Apply(base, m.layout);
    if (!mine) {
        return reject(Reason::Invalid, "unreadable layout change");
    }
    LayoutMerge::Outcome merged = LayoutMerge::Merge(base, *mine, _layout, LayoutMerge::Policy::Proposal);
    if (!merged.ok) {
        return reject(merged.reason.empty() ? Reason::Invalid : merged.reason.c_str(), merged.text);
    }
    std::string notice;
    for (const auto& d : merged.dropped) {
        std::string who = WhoDeleted(d, baseRev);
        notice += std::string(notice.empty() ? "" : "\n") + "'" + d + "' was deleted by " + (who.empty() ? std::string("someone else") : who) +
                  ", so your change to it was dropped";
    }
    json delta = LayoutDelta::Diff(_layout, merged.result);
    if (delta.is_null()) {
        std::string text = notice.empty() ? std::string("Someone else already made that layout change") : notice;
        o = reject(Reason::NoChange, text);
        return o;
    }
    // What every client will compute from the delta.
    auto next = LayoutDelta::Apply(_layout, delta);
    if (!next) {
        return reject(Reason::Invalid, "the merged layout could not be applied");
    }
    if (!faults.hostIgnoresRoles) {
        ParticipantId owner = kNoParticipant;
        std::string row = LayoutTakesAssignedRow(_doc.Sequence(), _assignments, c.id, _layout.rgbEffects, next->rgbEffects, &owner);
        if (!row.empty()) {
            return reject(Reason::Assigned, row + " is assigned to " + NameOfParticipant(owner) + ", so it can't be deleted or renamed");
        }
    }
    if (_events.onLayout) {
        LayoutCommitInfo info;
        info.rev = _applied + 1;
        info.author = c.id;
        info.clientSeq = m.clientSeq;
        if (!_events.onLayout(info, _layout, *next)) {
            o.held = true;
            return o;
        }
    }
    o.rev = CommitLayoutApplied(c.id, m.clientSeq, delta, *next, notice);
    o.committed = true;
    return o;
}

uint64_t CollabHost::StampLock(ParticipantId who, const LayoutLockInfo& lock) {
    if (lock.object.empty()) {
        _locks.erase(who);
        return 0;
    }
    auto it = _locks.find(who);
    if (it != _locks.end() && it->second.first == lock.object) {
        return it->second.second;
    }
    ParticipantId holder = kNoParticipant;
    uint64_t first = 0;
    for (const auto& [p, l] : _locks) {
        if (p != who && l.first == lock.object && (holder == kNoParticipant || l.second < first)) {
            holder = p;
            first = l.second;
        }
    }
    uint64_t order = ++_lockCounter;
    _locks[who] = { lock.object, order };
    if (holder != kNoParticipant) {
        std::string what = lock.name.empty() ? lock.object : lock.name;
        std::string text = NameOfParticipant(holder) + " started editing " + what + " first; changes you make to it may be overwritten";
        Log("participant " + std::to_string(who) + " took the layout lock on '" + lock.object + "' after " + std::to_string(holder));
        if (who == _participant) {
            if (_events.onNotice) {
                _events.onNotice(Reason::LockTaken, text);
            }
        } else {
            for (const auto& [peer, c] : _clients) {
                if (c.id == who) {
                    Message n;
                    n.type = MsgType::Notice;
                    n.reason = Reason::LockTaken;
                    n.text = text;
                    _transport.Send(peer, n);
                }
            }
        }
    }
    return order;
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

void CollabHost::DropClient(PeerId peer, const std::string& why, const std::string& tell) {
    auto it = _clients.find(peer);
    if (it == _clients.end()) {
        return;
    }
    if (!tell.empty()) {
        // It may still be there (a stall, a one-way link): without this it
        // would carry on unheard.
        Message bye;
        bye.type = MsgType::Bye;
        bye.reason = tell;
        bye.text = tell == Reason::TimedOut ? "The host stopped hearing from this device." : std::string();
        _transport.Send(peer, bye);
        _strangersTold[peer] = _now;
    }
    ParticipantId id = it->second.id;
    std::string name = it->second.name;
    _clients.erase(it);
    _locks.erase(id);
    PruneAssignments(id);
    Log("participant " + std::to_string(id) + " " + why);
    if (_events.onParticipant) {
        _events.onParticipant(id, name, false);
    }
    SendRoster();
}

// A message from a connection that isn't (or is no longer) in the session:
// it is told, now and then, rather than ignored.
void CollabHost::NotAClient(PeerId from) {
    auto told = _strangersTold.find(from);
    if (told != _strangersTold.end() && _now - told->second < _opts.peerTimeoutMs / 3) {
        return;
    }
    _strangersTold[from] = _now;
    Message bye;
    bye.type = MsgType::Bye;
    bye.reason = Reason::NotInSession;
    bye.text = "This device is no longer in the session.";
    _transport.Send(from, bye);
}

void CollabHost::StandDown(const std::string& why) {
    Log("standing down: " + why);
    Message bye;
    bye.type = MsgType::Bye;
    bye.reason = Reason::HostChanged;
    bye.text = why;
    for (const auto& [peer, c] : _clients) {
        _transport.Send(peer, bye);
    }
    _clients.clear();
    _endReason = Reason::HostChanged;
    _endText = why;
    _started = false;
    if (_events.onEnded) {
        _events.onEnded(_endReason);
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
        if (m.type != MsgType::Bye) {
            NotAClient(from);
        }
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
        c->active = m.active;
        PruneLayouts();
        break;
    case MsgType::Presence: {
        Message relay = m;
        relay.participant = c->id;
        relay.lockOrder = StampLock(c->id, m.presence.layoutLock);
        for (const auto& [peer, other] : _clients) {
            if (peer != from) {
                _transport.Send(peer, relay);
            }
        }
        if (_events.onPresence) {
            _events.onPresence(relay.participant, relay.presence, relay.lockOrder);
        }
        break;
    }
    case MsgType::Claim:
        OnClaim(*c, m);
        break;
    case MsgType::Bye:
        if (m.reason == Reason::HostChanged) {
            if (_now < _replacedWindowUntil) {
                // It moved on to a new host while this one was blocked.
                StandDown("Another device took over this session while this one was not responding.");
                break;
            }
            // Handled before this host's next tick: that tick tells whether
            // it was blocked long enough to have been replaced.
            _hostChangedSaid = true;
        }
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
    if (m.rejoin) {
        AddRejoiner(from, m);
        return;
    }
    if (Client* existing = FindClient(from)) {
        // A repeated Hello: our Welcome or the snapshot got lost.
        existing->lastHeard = _now;
        SendWelcome(*existing, false);
        SendManifest(*existing, false);
        SendRosterTo(from);
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
    c.deviceKind = m.deviceKind;
    c.lastHeard = _now;
    c.lastPing = _now;
    c.applied = _applied;
    c.liveLayout = _liveLayout && m.liveLayout;
    c.role = _joinRole;
    c.secret = NewRejoinSecret();
    c.secretHash = SecretDigest(c.secret);
    SendWelcome(c, false);
    SendManifest(c, true);
    Log("participant " + std::to_string(c.id) + " joined at rev " + std::to_string(_applied));
    if (_events.onParticipant) {
        _events.onParticipant(c.id, c.name, true);
    }
    SendRoster();
}

// A participant coming back after the host changed: it keeps its id, its
// document, its role and its undo stack. It catches up from the log (or a
// snapshot) and proposes again whatever the old host never answered.
// Accepted only for someone the old host listed, during the window after
// this host took over, and only with that participant's rejoin secret (its
// digest is in the old host's roster). With the secret it is the same
// participant, so a connection it still seems to hold (one that dropped
// unnoticed) is replaced. Anything else is refused, so nobody can take
// another participant's place or shed a viewer's role.
void CollabHost::AddRejoiner(PeerId from, const Message& m) {
    Message refuse;
    refuse.type = MsgType::Refuse;
    refuse.reason = Reason::Invalid;
    Client* c = FindClient(from);
    if (c != nullptr) {
        if (c->id != m.participant) {
            refuse.text = "This connection already belongs to someone else in the session.";
            _transport.Send(from, refuse);
            return;
        }
        // A repeated Hello: the Welcome got lost.
        c->lastHeard = _now;
        SendWelcome(*c, true);
        return;
    }
    auto entry = std::find_if(_formerRoster.begin(), _formerRoster.end(), [&](const RosterEntry& r) { return r.participant == m.participant; });
    bool listed = m.participant != kNoParticipant && m.participant != _participant && entry != _formerRoster.end();
    bool secretOk = listed && !entry->secretHash.empty() && SecretDigest(m.secret) == entry->secretHash;
    auto old = std::find_if(_clients.begin(), _clients.end(), [&](const auto& kv) { return kv.second.id == m.participant; });
    bool connected = old != _clients.end();
    if (!listed || !secretOk || _now >= _rejoinUntil) {
        refuse.text = connected ? "Someone in this session is already connected as this participant."
                                : "This device can't rejoin this session. Join it again.";
        Log("refused a rejoin as participant " + std::to_string(m.participant) +
            (!listed ? ": not expected back" : !secretOk ? ": wrong rejoin secret" : ": too late"));
        _transport.Send(from, refuse);
        return;
    }
    if (connected) {
        // While the host changes, a rejoin connection can drop and come back
        // before the drop is noticed: the new connection takes over from the
        // old one, keeping the role, claims and layout lock.
        PeerId stale = old->first;
        _clients.erase(old);
        Message bye;
        bye.type = MsgType::Bye;
        bye.reason = Reason::HostChanged;
        bye.text = "This participant reconnected from another connection.";
        _transport.Send(stale, bye);
        Log("participant " + std::to_string(m.participant) + " reconnected; its old connection is dropped");
    }
    if (_clients.size() + 1 >= _opts.maxParticipants) {
        refuse.reason = Reason::Full;
        refuse.text = "This session is full.";
        _transport.Send(from, refuse);
        return;
    }
    c = &_clients[from];
    c->peer = from;
    c->id = m.participant;
    c->name = m.name;
    c->deviceKind = m.deviceKind;
    c->lastPing = _now;
    c->lastHeard = _now;
    c->liveLayout = _liveLayout && m.liveLayout;
    c->active = true;
    c->role = RoleIn(_formerRoster, m.participant);
    c->secretHash = entry->secretHash;
    _awaitingRejoin.erase(m.participant);
    // Proposals up to the last one committed were dealt with by the old
    // host; the first one it still waits on comes next.
    c->nextSeq = std::max(LastSeqOf(m.participant) + 1, m.clientSeq);
    c->applied = std::min(m.rev, _applied);
    Log("participant " + std::to_string(c->id) + " rejoined at rev " + std::to_string(m.rev));
    SendWelcome(*c, true);
    SendRoster();
}

void CollabHost::SendSnapshot(Client& c) {
    if (c.snapshotBlob != 0 && !Stale(c.snapshotAt)) {
        // One is on its way: it brings the client to a rev the log reaches.
        return;
    }
    std::string xsq;
    if (!_doc.SaveXsq(xsq)) {
        Log("could not save a snapshot");
        return;
    }
    std::string file = NewSessionFile("outgoing", ".xsq");
    std::string none;
    SequenceElements& seq = _doc.Sequence();
    std::string uids = CaptureUids(seq).dump();
    // The id table and the layout ride in the blob with the sequence: any
    // of them can be far larger than a message may be.
    if (!WriteParts(file, { &xsq, &uids, c.liveLayout ? &_layout.rgbEffects : &none, c.liveLayout ? &_layout.networks : &none })) {
        Log("could not write a snapshot");
        RemoveFile(file);
        return;
    }
    Message s;
    s.type = MsgType::Snapshot;
    s.rev = _applied;
    s.docHash = _hasher.Hash(seq);
    s.clientSeq = c.nextSeq - 1;
    if (c.liveLayout) {
        s.layoutHash = _layoutHash;
    }
    s.blob = NextBlob();
    c.snapshotBlob = s.blob;
    c.snapshotRev = _applied;
    c.snapshotAt = _now;
    c.sentDocument = true;
    c.applied = std::max(c.applied, _applied);
    _transport.SendBlob(c.peer, s.blob, file);
    RemoveFile(file);
    _transport.Send(c.peer, s);
    // A re-syncing client may have missed roster changes too.
    SendRosterTo(c.peer);
}

void CollabHost::SendManifest(Client& c, bool rebuild) {
    if (rebuild) {
        c.files.clear();
        c.manifest.clear();
        c.manifestJob.reset();
        std::string show = _doc.ShowFolder();
        if (!show.empty()) {
            std::vector<SessionFile> files = CollectSessionFiles(show, _doc.Sequence(), _doc.AudioFiles());
            uint64_t bytes = 0;
            for (const auto& f : files) {
                bytes += _hashes.BytesToRead(f.local);
            }
            if (bytes <= _opts.syncHashBytes) {
                c.manifest = BuildManifest(files, &_hashes, &c.files);
            } else {
                // Large media: hashed off the session thread, so pings and
                // edits keep flowing; Tick sends the manifest when it's done.
                Log("hashing " + std::to_string(bytes >> 20) + " MB of show files for participant " + std::to_string(c.id));
                auto job = std::make_shared<HashJob>();
                job->cache = _hashes.Fork();
                job->files = std::move(files);
                c.manifestJob = job;
                std::thread([job] {
                    job->manifest = BuildManifest(job->files, &job->cache, &job->local);
                    job->done = true;
                }).detach();
            }
        }
    }
    if (c.manifestJob) {
        return;
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
        auto sending = c.filesSending.find(path);
        if (!done.insert(path).second || (sending != c.filesSending.end() && !Stale(sending->second.second))) {
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
        c.filesSending[path] = { SendFile(c.peer, now, local->second), _now };
    }
}

void CollabHost::OnBlobReady(PeerId from, const Message& info, const std::string& path) {
    Client* c = FindClient(from);
    if (c == nullptr || info.type != MsgType::File || info.files.size() != 1) {
        RemoveFile(path);
        return;
    }
    const FileEntry& e = info.files[0];
    std::string why = c->role == ParticipantRole::Viewer ? "a viewer" : !SentMediaFor(*c, e.path) ? "not media it added" : "";
    if (!why.empty()) {
        // Files only travel with a media entry their sender added: anything
        // else would overwrite what others shared.
        Log("refused '" + e.path + "' from participant " + std::to_string(c->id) + ": " + why);
        RemoveFile(path);
        return;
    }
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

namespace {
// The file references a command adds to the sequence's media (PutMedia,
// alone or in a batch).
void PutMediaKeys(const SequenceCommand& cmd, std::set<std::string>& out) {
    if (cmd.Type() == CommandType::PutMedia) {
        const auto& state = static_cast<const PutMediaCommand&>(cmd).GetState();
        if (!state.embedded) {
            out.insert(state.key);
        }
    } else if (cmd.Type() == CommandType::Batch) {
        for (const auto& c : static_cast<const BatchCommand&>(cmd).GetCommands()) {
            PutMediaKeys(*c, out);
        }
    }
}
} // namespace

// Whether `c` added the media entry `path`: its last commit touching that
// entry is the client's, or one of its proposals still waiting here adds it.
bool CollabHost::SentMediaFor(const Client& c, const std::string& path) const {
    auto t = _touch.find("m." + path);
    if (t != _touch.end() && t->second.author == c.id) {
        return true;
    }
    for (const auto& [seq, m] : c.early) {
        if (auto cmd = DeserializeCommand(m.command)) {
            std::set<std::string> keys;
            PutMediaKeys(*cmd, keys);
            if (keys.count(path)) {
                return true;
            }
        }
    }
    return false;
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
    if (m.clientSeq == c.nextSeq || c.early.size() < 100000) {
        c.early.emplace(m.clientSeq, m);
    }
    DrainProposals(c.peer);
}

void CollabHost::DrainProposals(PeerId peer) {
    while (true) {
        Client* c = FindClient(peer);
        if (c == nullptr) {
            return;
        }
        auto it = c->early.find(c->nextSeq);
        if (it == c->early.end()) {
            break;
        }
        Message next = it->second;
        if (!ProcessProposal(*c, next)) {
            return; // held: the next tick (or the client's resend) tries again
        }
        c = FindClient(peer);
        if (c == nullptr) {
            return;
        }
        c->early.erase(next.clientSeq);
        ++c->nextSeq;
    }
    Client* c = FindClient(peer);
    if (c != nullptr) {
        c->early.erase(c->early.begin(), c->early.lower_bound(c->nextSeq));
    }
}

bool CollabHost::ProcessProposal(Client& c, const Message& m) {
    // A proposal can't be based on a rev the host hasn't made yet.
    Rev base = std::min(m.baseRev, _applied);
    Outcome o = !m.layout.is_null() ? ProcessLayout(c, m) : Process(c.id, m.clientSeq, base, m.kind, m.undoOf, m.command);
    if (o.held) {
        return false;
    }
    if (o.committed) {
        return true;
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
    return true;
}

void CollabHost::SetPresence(const PresenceInfo& p) {
    Message m;
    m.type = MsgType::Presence;
    m.participant = _participant;
    m.presence = p;
    m.lockOrder = StampLock(_participant, p.layoutLock);
    for (const auto& [peer, c] : _clients) {
        _transport.Send(peer, m);
    }
}

void CollabHost::Tick(uint64_t nowMs) {
    uint64_t prev = _lastTick != 0 ? _lastTick : _now;
    _now = std::max(_now, nowMs);
    _lastTick = _now;
    if (!IsActive()) {
        return;
    }
    uint64_t gap = _now - prev;
    if (gap > _opts.stallGapMs) {
        // This host was blocked: what queued meanwhile was handled with the
        // time from before, and nobody could be heard. Don't count it.
        Log("ticks resumed after " + std::to_string(gap) + " ms; not counting it against the clients");
        for (auto& [peer, c] : _clients) {
            c.lastHeard = std::min(_now, c.lastHeard + gap);
        }
        if (gap > _opts.peerTimeoutMs) {
            // Long enough for the clients to have given up on this host.
            _replacedWindowUntil = _now + 2 * _opts.peerTimeoutMs;
            if (_hostChangedSaid) {
                StandDown("Another device took over this session while this one was not responding.");
                return;
            }
        }
    }
    _hostChangedSaid = false;
    std::vector<PeerId> gone;
    bool due = false;
    for (auto& [peer, c] : _clients) {
        due = due || (_now - c.lastHeard <= _opts.peerTimeoutMs && _now - c.lastPing >= _opts.pingIntervalMs);
    }
    if (due) {
        // Everyone gets the same succession at the same moment, so they
        // agree on it when the host goes.
        _ranked = RankSuccessors();
        _succession = _ranked;
    }
    for (auto& [peer, c] : _clients) {
        uint64_t limit = _opts.peerTimeoutMs;
        if (c.snapshotAt != 0 && c.lastHeard <= c.snapshotAt) {
            // Nothing since its snapshot went: it may still be loading it.
            limit += _opts.snapshotLoadGraceMs;
        }
        if (_now - c.lastHeard > limit) {
            gone.push_back(peer);
            continue;
        }
        if (due) {
            c.lastPing = _now;
            Message p;
            p.type = MsgType::Ping;
            p.rev = _applied;
            p.time = _now;
            p.succession = _ranked;
            if (c.liveLayout) {
                p.layoutHash = _layoutHash;
            }
            _transport.Send(peer, p);
        }
    }
    if (due) {
        _rankHead = _applied;
    }
    if (!_awaitingRejoin.empty() && _now >= _rejoinUntil) {
        std::vector<Assignment> next = _assignments;
        for (ParticipantId gone : _awaitingRejoin) {
            next = WithoutAssignments(_doc.Sequence(), next, gone);
        }
        _awaitingRejoin.clear();
        if (next != _assignments) {
            Log("released the claims of participants who didn't come back after the host changed");
            _assignments = std::move(next);
            SendRoster();
        }
    }
    for (PeerId p : gone) {
        DropClient(p, "timed out", Reason::TimedOut);
    }
    for (auto& [peer, c] : _clients) {
        if (c.manifestJob && c.manifestJob->done) {
            auto job = std::move(c.manifestJob);
            c.manifest = std::move(job->manifest);
            c.files = std::move(job->local);
            _hashes.Merge(job->cache);
            SendManifest(c, false);
        }
    }
    if (!deferHeldApply) {
        ApplyHeld();
    }
}

bool CollabHost::HeldApplyDue() const {
    return std::any_of(_clients.begin(), _clients.end(), [](const auto& kv) { return kv.second.early.count(kv.second.nextSeq) != 0; });
}

// Proposals held because memory could not take a layout change yet.
void CollabHost::ApplyHeld() {
    std::vector<PeerId> waiting;
    for (const auto& [peer, c] : _clients) {
        if (c.early.count(c.nextSeq)) {
            waiting.push_back(peer);
        }
    }
    for (PeerId p : waiting) {
        DrainProposals(p);
    }
}

void CollabClient::ApplyHeld() {
    if (_state == State::Active && _layoutHeld) {
        _layoutHeld = false;
        DrainCommits();
    }
}

// ---------------------------------------------------------------------------
// CollabClient
// ---------------------------------------------------------------------------

CollabClient::CollabClient(ICollabTransport& transport, ICollabDocument& doc, SessionOptions opts, SessionEvents events) :
    CollabParticipant(transport, doc, std::move(opts), std::move(events)) {
    InstallHandlers();
}

void CollabClient::OnTransportBlob(PeerId from, BlobId id, const std::string& path) {
    if (from == _host && _state != State::Ended && _state != State::Idle) {
        _lastBlobProgress = _now == 0 ? 1 : _now;
        OnBlobArrived(from, id, path);
    }
}

void CollabClient::OnBlobProgress(PeerId from, BlobId /*id*/, uint64_t /*received*/, uint64_t /*size*/) {
    if (from == _host) {
        _lastBlobProgress = _now == 0 ? 1 : _now;
    }
}

void CollabClient::OnSendFailed(PeerId /*to*/, const Message& m, const std::string& reason) {
    Log("could not send message type " + std::to_string((int)m.type) + ": " + reason);
    auto p = _pending.find(m.clientSeq);
    if (m.type == MsgType::Propose && p != _pending.end() && !p->second.withdrawn) {
        // Resending can't help: the host never gets it.
        Message stub = p->second.msg;
        Message r;
        r.type = MsgType::Reject;
        r.clientSeq = m.clientSeq;
        r.rev = _applied;
        r.kind = stub.kind;
        r.reason = Reason::TooLarge;
        r.text = "This change is too large to send to the host.";
        HandleReject(r);
        // Something must take its number, or the host waits for it before
        // any later proposal: an empty batch, which changes nothing. It is
        // resent like any proposal until the host answers.
        stub.layout = json();
        BatchCommand("Withdrawn").Serialize(stub.command);
        stub.kind = EditKind::Edit;
        stub.undoOf = 0;
        _pending[m.clientSeq] = { stub, _now, false, true };
        Send(stub);
        return;
    }
    if (_events.onNotice) {
        _events.onNotice(Reason::TooLarge, "Something could not be sent to the host: it is too large.");
    }
}

CollabClient::~CollabClient() = default;

void CollabClient::Send(const Message& m) {
    // While the host changes nothing goes out but the rejoin handshake;
    // proposals made meanwhile wait in _pending for the new host.
    if (_state == State::Migrating || _state == State::Succeeding || _host == kNoPeer) {
        return;
    }
    if (_state == State::Rejoining && m.type != MsgType::Hello && m.type != MsgType::Commit && m.type != MsgType::Bye) {
        return;
    }
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
    h.liveLayout = _opts.liveLayout;
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
    if (e != PeerEvent::Lost && e != PeerEvent::Left) {
        return;
    }
    if (_state == State::Succeeding) {
        _rejoinHellos.erase(peer);
        if (peer == _catchUpFrom) {
            _catchUpFailed = true;
        }
        return;
    }
    if (peer == _host) {
        HostGone();
    }
}

void CollabClient::HostGone() {
    if (_state == State::Ended || _state == State::Idle || _state == State::Migrating || _state == State::Succeeding) {
        return;
    }
    // Only a participant that has the document can carry on.
    bool hasDocument = _state == State::Active || _state == State::Rejoining || (_state == State::AwaitingSnapshot && _resyncs != 0);
    if (hasDocument && _events.onHostLost && _events.onHostLost()) {
        Log("lost the host at rev " + std::to_string(_applied) + "; waiting for the new one");
        if (_host != kNoPeer) {
            // A host that was only blocked learns, when it resumes, that its
            // clients moved on, and stands down instead of carrying on alone.
            Message bye;
            bye.type = MsgType::Bye;
            bye.reason = Reason::HostChanged;
            _transport.Send(_host, bye);
        }
        _state = State::Migrating;
        _host = kNoPeer;
        _buffered.clear();
        _queuedRejects.clear();
        return;
    }
    EndSession(Reason::HostLost);
}

void CollabClient::SendRejoinHello() {
    _lastHello = _now;
    Message h;
    h.type = MsgType::Hello;
    h.rejoin = true;
    h.secret = _rejoinSecret;
    h.appVersion = _opts.appVersion;
    h.name = _opts.displayName;
    h.deviceKind = _opts.deviceKind;
    h.liveLayout = _opts.liveLayout;
    h.participant = _participant;
    h.rev = _applied;
    h.clientSeq = _pending.empty() ? _nextSeq : _pending.begin()->first;
    Send(h);
}

void CollabClient::Rejoin(PeerId host) {
    if (_state != State::Migrating) {
        return;
    }
    _host = host;
    _state = State::Rejoining;
    _lastHeard = _now;
    _buffered.clear();
    _queuedRejects.clear();
    // A layout proposal is made again from memory against the session
    // layout once rejoined (the controller re-proposes it).
    for (auto it = _pending.begin(); it != _pending.end();) {
        it = it->second.msg.layout.is_null() ? std::next(it) : _pending.erase(it);
    }
    SendRejoinHello();
}

void CollabClient::BeginSuccession() {
    if (_state != State::Migrating) {
        return;
    }
    _state = State::Succeeding;
    _host = kNoPeer;
    _rejoinHellos.clear();
    _catchUpFrom = kNoPeer;
    _catchUpFailed = false;
}

void CollabClient::AbandonSuccession() {
    if (_state == State::Succeeding) {
        _state = State::Migrating;
        _rejoinHellos.clear();
        _catchUpFrom = kNoPeer;
        _buffered.clear();
    }
}

std::vector<CollabClient::Rejoiner> CollabClient::Rejoiners() const {
    std::vector<Rejoiner> out;
    for (const auto& [peer, h] : _rejoinHellos) {
        out.push_back({ peer, h.participant, h.rev });
    }
    return out;
}

void CollabClient::CatchUpFrom(PeerId peer) {
    if (_state != State::Succeeding) {
        return;
    }
    _catchUpFrom = peer;
    _catchUpFailed = false;
    _lastResendRequest = 0;
    RequestResend();
}

void CollabClient::OnRejoined(const Message& m) {
    _state = State::Active;
    _lastHeard = _now;
    _hostHead = m.rev;
    _layoutHeld = false;
    _rejoinPending = true;
    _rejoinRev = m.rev;
    _rejoinSeq = m.clientSeq;
    _liveLayout = _liveLayout && m.liveLayout;
    Log("rejoined at rev " + std::to_string(_applied) + "; the new host is at " + std::to_string(m.rev));
    // Proposals still waiting go to the new host at the next tick.
    for (auto& [seq, p] : _pending) {
        p.lastSent = 0;
    }
    if (_applied > m.rev) {
        // This participant had commits the new host could not get: its
        // document is replaced by the new host's.
        _rejoinPending = false;
        RequestResync("ahead of the new host (rev " + std::to_string(_applied) + " > " + std::to_string(m.rev) + ")");
        return;
    }
    if (_applied == m.rev) {
        DropDecided();
    } else {
        _lastResendRequest = 0;
        RequestResend();
    }
}

// The old host had dealt with these (a later proposal of this participant
// was committed), so one not committed was rejected with the answer lost.
void CollabClient::DropDecided() {
    _rejoinPending = false;
    std::vector<Message> lost;
    for (const auto& [seq, p] : _pending) {
        if (seq > _rejoinSeq) {
            break;
        }
        Message r;
        r.type = MsgType::Reject;
        r.clientSeq = seq;
        r.rev = _applied;
        r.kind = p.msg.kind;
        r.reason = Reason::HostChanged;
        r.text = "The host left before this change was confirmed.";
        lost.push_back(std::move(r));
    }
    for (const auto& r : lost) {
        HandleReject(r);
    }
}

void CollabClient::LogCommit(const Message& m) {
    _log.push_back({ m });
    while (_log.size() > _opts.clientLogLimit) {
        _log.pop_front();
        ++_logBase;
    }
}

void CollabClient::OnMessage(PeerId from, const Message& m) {
    if (_state == State::Succeeding) {
        if (m.type == MsgType::Hello && m.rejoin && m.protocol == kProtocolVersion && m.participant != kNoParticipant && m.participant != _participant) {
            _rejoinHellos[from] = m;
        } else if (m.type == MsgType::Commit && from == _catchUpFrom) {
            OnCommit(m);
        }
        return;
    }
    if (from != _host || _state == State::Ended || _state == State::Idle || _state == State::Migrating) {
        return;
    }
    _lastHeard = _now;
    if (_state == State::Rejoining) {
        if (m.type == MsgType::Welcome && m.rejoin && m.participant == _participant) {
            OnRejoined(m);
        } else if (m.type == MsgType::Resend) {
            // The new host lacks commits this participant has.
            for (const auto& r : _log) {
                if (r.msg.rev >= m.rev) {
                    _transport.Send(_host, r.msg);
                }
            }
        } else if (m.type == MsgType::Refuse || m.type == MsgType::Bye) {
            _endText = m.text;
            EndSession(m.reason.empty() ? std::string(Reason::Ended) : m.reason);
        }
        return;
    }
    switch (m.type) {
    case MsgType::Welcome:
        if (_state == State::Joining) {
            if (m.blob != 0) {
                OnBlobInfo(from, m); // the session layout comes as a blob
            } else {
                OnWelcome(m);
            }
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
        _succession = m.succession;
        Message p;
        p.type = MsgType::Pong;
        p.rev = _applied;
        p.time = m.time;
        p.active = _state == State::Active;
        Send(p);
        if (_state == State::Active && _hostHead > _applied) {
            RequestResend();
        }
        if (_state == State::Active && _liveLayout && !_layoutHeld && m.layoutHash != 0 && m.rev == _applied && m.layoutHash != _layoutHash) {
            RequestResync("layout hash " + std::to_string(_layoutHash) + " differs from the host's " + std::to_string(m.layoutHash) + " at rev " +
                          std::to_string(m.rev));
        }
        break;
    }
    case MsgType::Presence:
        if (_events.onPresence) {
            _events.onPresence(m.participant, m.presence, m.lockOrder);
        }
        break;
    case MsgType::Notice:
        Log("notice: " + m.text);
        if (_events.onNotice) {
            _events.onNotice(m.reason, m.text);
        }
        break;
    case MsgType::Bye:
        if (m.reason == Reason::HostChanged) {
            // The host stood down for one that took over: follow it there.
            Log("the host stood down: " + m.text);
            HostGone();
            break;
        }
        _endText = m.text;
        EndSession(m.reason.empty() ? std::string(Reason::Ended) : m.reason);
        break;
    case MsgType::Roster:
        _succession = m.succession;
        _epoch = m.epoch;
        _joinRole = m.joinRole;
        if (m.nextParticipant != kNoParticipant) {
            _nextParticipant = m.nextParticipant;
        }
        if (m.roster != _roster || m.assignments != _assignments) {
            _roster = m.roster;
            _assignments = m.assignments;
            if (_events.onRoster) {
                _events.onRoster(_roster);
            }
        }
        break;
    default:
        break;
    }
}

std::string CollabClient::CurrentShowFolder() {
    return _usingCopy ? SessionShowFolder() : _doc.ShowFolder();
}

void CollabClient::OnWelcome(const Message& m) {
    if (_state != State::Joining) {
        return;
    }
    _participant = m.participant;
    _rejoinSecret = m.secret;
    SeqIds::SetPeer(faults.clientSharesHostIdPrefix ? kHostParticipant : m.participant);
    _liveLayout = _opts.liveLayout && m.liveLayout && m.layoutHash != 0;
    if (_liveLayout) {
        SetLayout(m.layoutFiles);
    }
    _state = State::Checking;
    _lastHello = _now;
    _hostHead = std::max(_hostHead, m.rev);
}

void CollabClient::OnBlobReady(PeerId /*from*/, const Message& info, const std::string& path) {
    if (info.type == MsgType::Snapshot || info.type == MsgType::Welcome) {
        bool snapshot = info.type == MsgType::Snapshot;
        // Snapshot: the .xsq, the id table, the layout; Welcome: the layout.
        std::vector<std::string> parts(snapshot ? 4 : 2);
        bool ok = !path.empty() && ReadParts(path, parts);
        RemoveFile(path);
        Message m = info;
        if (ok && snapshot) {
            auto uids = ParseBoundedJson(parts[1]);
            ok = uids.has_value();
            if (ok) {
                m.uids = std::move(*uids);
            }
        }
        // A failed transfer is asked for again by the snapshot / hello retry.
        if (!ok) {
            Log(std::string("a ") + (snapshot ? "snapshot" : "welcome") + " from the host arrived damaged");
            return;
        }
        m.layoutFiles.rgbEffects = std::move(parts[snapshot ? 2 : 0]);
        m.layoutFiles.networks = std::move(parts[snapshot ? 3 : 1]);
        if (snapshot) {
            OnSnapshot(m, parts[0]);
        } else {
            OnWelcome(m);
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
    if (_state != State::Active) {
        Log("not sharing '" + e.path + "' while the host is changing");
        return;
    }
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
    _checks = ResolveManifest(_doc.ShowFolder(), m.files);
    uint64_t bytes = 0;
    for (const auto& c : _checks) {
        bytes += c.local.empty() ? 0 : _hashes.BytesToRead(c.local);
    }
    if (bytes > _opts.syncHashBytes) {
        // Large media: hashed off the session thread, so the host keeps
        // hearing from this device; Tick carries on when it's done.
        Log("hashing " + std::to_string(bytes >> 20) + " MB of this device's show files");
        auto job = std::make_shared<HashJob>();
        job->cache = _hashes.Fork();
        job->checks = std::move(_checks);
        _checks.clear();
        _checkJob = job;
        std::thread([job] {
            HashChecks(job->checks, &job->cache);
            job->done = true;
        }).detach();
        return;
    }
    HashChecks(_checks, &_hashes);
    ContinueManifest();
}

void CollabClient::ContinueManifest() {
    if (_liveLayout) {
        // The layout is compared with the session's (which may hold edits
        // the host has not saved), not with the host's files.
        LayoutFiles own;
        ReadWhole((fs::path(_doc.ShowFolder()) / kLayoutFile).string(), own.rgbEffects);
        ReadWhole((fs::path(_doc.ShowFolder()) / kNetworksFile).string(), own.networks);
        _layoutCopy = !LayoutDelta::Equivalent(own, _layout);
        for (const char* f : { kLayoutFile, kNetworksFile }) {
            std::string session = f == kLayoutFile ? _layout.rgbEffects : _layout.networks;
            std::string mine = f == kLayoutFile ? own.rgbEffects : own.networks;
            if (mine == session) {
                continue;
            }
            LayoutDifference d = DescribeLayoutDifference(f, mine, session);
            if (d.changes != 0) {
                _report.layoutDifference += (_report.layoutDifference.empty() ? "" : "\n") + std::string(f) + ": " + d.text;
            }
        }
        if (_layoutCopy && _report.layoutDifference.empty()) {
            _report.layoutDifference = "the layout ids differ from the session's";
        }
    }
    bool layout = false;
    for (const auto& c : _checks) {
        if (c.state == FileState::Match || (_liveLayout && c.entry.role == FileRole::Layout)) {
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
    std::string liveDifference = _report.layoutDifference;
    _report = {};
    _report.showFolder = _doc.ShowFolder();
    std::set<std::string> unavailable(_unavailable.begin(), _unavailable.end());
    std::string layoutText;
    size_t layoutChanges = 0;
    std::string audioText;
    std::string lostText;
    for (const auto& c : _checks) {
        if (c.state == FileState::Match || (_liveLayout && c.entry.role == FileRole::Layout)) {
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
    _report.layoutDifference = _liveLayout ? liveDifference : layoutText;
    if (!_opts.fetchFiles) {
        if (layoutChanges != 0) {
            RefuseJoin(Reason::LayoutDiffers, "This device's layout differs from the host's:\n" + layoutText);
            return;
        }
        if (!audioText.empty()) {
            RefuseJoin(Reason::AudioDiffers, "This device's audio differs from the host's: " + audioText);
            return;
        }
        if (!_layoutCopy) {
            AskForSnapshot();
            return;
        }
    } else if (!lostText.empty()) {
        RefuseJoin(Reason::FilesUnavailable, "These files could not be copied from the host: " + lostText);
        return;
    }
    if (_fetched.empty() && !_layoutCopy) {
        // Everything matched, or only media the host couldn't send.
        AskForSnapshot();
        return;
    }
    // Open against a copy of the host's show: what matched comes from this
    // device's show folder, the rest is what was fetched.
    std::string root = SessionShowFolder();
    std::vector<std::string> copyFailed;
    for (const auto& c : _checks) {
        if (_layoutCopy && c.entry.role == FileRole::Layout) {
            continue; // written from the session layout below
        }
        // Without fetching, media that differs is used as this device has it.
        bool ownDifferent = !_opts.fetchFiles && c.state == FileState::Different && c.entry.role == FileRole::Media;
        if (c.state != FileState::Match && !ownDifferent) {
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
    if (_layoutCopy) {
        std::string error;
        if (!WriteSessionLayout(_layout, error)) {
            RefuseJoin(Reason::FilesUnavailable, error);
            return;
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
            bool ownDifferent = !_opts.fetchFiles && c.state == FileState::Different;
            (ownDifferent ? _report.differentMedia : _report.missingMedia).push_back(c.entry.path);
        }
    }
    _report.missingMedia.insert(_report.missingMedia.end(), copyFailed.begin(), copyFailed.end());
    AskForSnapshot();
}

bool CollabClient::WriteSessionLayout(const LayoutFiles& files, std::string& error) {
    for (const char* f : { kLayoutFile, kNetworksFile }) {
        const std::string& text = f == kLayoutFile ? files.rgbEffects : files.networks;
        fs::path dest = fs::path(SessionShowFolder()) / f;
        std::error_code ec;
        if (text.empty()) {
            fs::remove(dest, ec);
            continue;
        }
        fs::create_directories(dest.parent_path(), ec);
        std::ofstream out(dest, std::ios::binary | std::ios::trunc);
        out << text;
        if (!out) {
            error = std::string("Could not write the session's copy of ") + f + ".";
            return false;
        }
    }
    return true;
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
    bool live = _liveLayout && m.layoutHash != 0;
    if (live) {
        if (_usingCopy) {
            // An app that reads the layout from its show folder as it opens
            // the sequence finds the session's.
            std::string error;
            if (!WriteSessionLayout(m.layoutFiles, error)) {
                Log(error);
            }
        }
        if (_events.onLayout) {
            LayoutCommitInfo info;
            info.rev = m.rev;
            info.snapshot = true;
            info.firstSnapshot = !_haveSnapshotLayout;
            if (!_events.onLayout(info, _haveSnapshotLayout ? _layout : LayoutFiles{}, m.layoutFiles)) {
                Log("the snapshot's layout could not be applied yet; asking again");
                return; // the snapshot retry asks for another
            }
        }
        SetLayout(m.layoutFiles);
        _haveSnapshotLayout = true;
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
    _log.clear();
    _logBase = m.rev;
    NoteCommitted(_participant, m.clientSeq);
    _rejoinPending = false;
    // Proposals the host had dealt with are in the snapshot (or rejected).
    for (auto it = _queuedRejects.begin(); it != _queuedRejects.end();) {
        if (it->clientSeq <= m.clientSeq) {
            auto p = _pending.find(it->clientSeq);
            if (p != _pending.end() && !p->second.msg.layout.is_null()) {
                if (_events.onLayoutRejected) {
                    _events.onLayoutRejected(it->clientSeq, it->reason, it->text);
                }
            } else if (p != _pending.end() && _events.onReject) {
                _events.onReject(it->clientSeq, it->kind, it->reason, it->participant);
            }
            it = _queuedRejects.erase(it);
        } else {
            ++it;
        }
    }
    for (auto it = _pending.begin(); it != _pending.end() && it->first <= m.clientSeq;) {
        _ownMeta.erase(it->first);
        it = _pending.erase(it);
    }
    for (auto it = _overlay.begin(); it != _overlay.end();) {
        it = it->second.seq <= m.clientSeq ? _overlay.erase(it) : std::next(it);
    }
    for (auto it = _times.begin(); it != _times.end();) {
        it = it->second.seq <= m.clientSeq ? _times.erase(it) : std::next(it);
    }
    for (auto it = _buffered.begin(); it != _buffered.end() && it->first <= m.rev;) {
        it = _buffered.erase(it);
    }
    _state = State::Active;
    _layoutHeld = false;
    LowerOverlay();
    if (live && _events.onSnapshotLoaded) {
        _events.onSnapshotLoaded();
    }
    if (resync && _events.onResync) {
        _events.onResync();
    }
    DrainCommits();
}

void CollabClient::RequestResync(const std::string& why) {
    if (_state == State::Succeeding) {
        // Taking over: the commits being fetched don't fit; carry on from
        // what this participant has.
        Log("catching up failed: " + why);
        _catchUpFailed = true;
        _buffered.clear();
        return;
    }
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
    if (_state == State::Succeeding) {
        if (_catchUpFrom != kNoPeer) {
            _transport.Send(_catchUpFrom, r);
        }
        return;
    }
    Send(r);
}

void CollabClient::OnCommit(const Message& m) {
    if (_state != State::Active && _state != State::AwaitingSnapshot && _state != State::Succeeding) {
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
    if (_state == State::Active || _state == State::Succeeding) {
        DrainCommits();
    }
}

void CollabClient::DrainCommits() {
    while (_state == State::Active || (_state == State::Succeeding && !_catchUpFailed)) {
        auto it = _buffered.find(_applied + 1);
        if (it == _buffered.end()) {
            break;
        }
        Message m = std::move(it->second);
        _buffered.erase(it);
        if (!ApplyCommit(m)) {
            if (_layoutHeld) {
                _buffered.emplace(m.rev, std::move(m));
            }
            break;
        }
    }
    for (auto it = _buffered.begin(); it != _buffered.end() && it->first <= _applied;) {
        it = _buffered.erase(it);
    }
    if ((_state == State::Active || _state == State::Succeeding) && !_buffered.empty() && !_layoutHeld) {
        RequestResend();
    }
    if (_state == State::Active && _rejoinPending && _applied >= _rejoinRev) {
        DropDecided();
    }
    DrainRejects();
    RunQueuedUndo();
}

bool CollabClient::UndoWaits() const {
    return std::any_of(_pending.begin(), _pending.end(), [](const auto& kv) { return kv.second.msg.layout.is_null() && !kv.second.withdrawn; });
}

std::string CollabClient::PendingEditLabel() const {
    for (auto it = _pending.rbegin(); it != _pending.rend(); ++it) {
        const Pending& p = it->second;
        if (!p.msg.layout.is_null() || p.withdrawn || p.msg.kind != EditKind::Edit) {
            continue;
        }
        auto meta = _ownMeta.find(it->first);
        if (meta != _ownMeta.end() && !meta->second.label.empty()) {
            return meta->second.label;
        }
        auto cmd = DeserializeCommand(p.msg.command);
        return cmd ? CommandLabel(*cmd) : std::string("Changes");
    }
    return {};
}

CollabClient::Applied CollabClient::SetKey(const EffectLocator& loc, bool palette, const std::string& key, const std::optional<std::string>& value) {
    SettingsChange change{ false, { { key, value } } };
    auto cmd = std::make_unique<ModifyEffectCommand>(loc, std::nullopt, palette ? SettingsChange{} : change, palette ? change : SettingsChange{});
    SequenceElements& seq = _doc.Sequence();
    std::unique_ptr<SequenceCommand> inverse = cmd->Apply(seq, ApplyMode::Lenient);
    if (inverse == nullptr) {
        return {};
    }
    _hasher.Invalidate(seq, *cmd);
    return { std::move(cmd), std::move(inverse) };
}

CollabClient::Applied CollabClient::SetTimes(const EffectLocator& loc, std::pair<int, int> times) {
    auto cmd = std::make_unique<MoveEffectCommand>(loc, times.first, times.second);
    SequenceElements& seq = _doc.Sequence();
    std::unique_ptr<SequenceCommand> inverse = cmd->Apply(seq, ApplyMode::Lenient);
    if (inverse == nullptr) {
        return {};
    }
    _hasher.Invalidate(seq, *cmd);
    return { std::move(cmd), std::move(inverse) };
}

void CollabClient::LiftOverlay() {
    SequenceElements& seq = _doc.Sequence();
    for (const auto& [k, e] : _overlay) {
        if (FindEffect(seq, e.loc) != nullptr) {
            SetKey(e.loc, std::get<1>(k), std::get<2>(k), e.host);
        }
    }
    for (const auto& [uid, t] : _times) {
        const Effect* e = FindEffect(seq, t.loc);
        if (e != nullptr && std::make_pair(e->GetStartTimeMS(), e->GetEndTimeMS()) != t.host) {
            SetTimes(t.loc, t.host);
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
    // Leniently: a pending move that now overlaps someone else's committed
    // edit shows until its reject arrives (right after that commit).
    for (auto it = _times.begin(); it != _times.end();) {
        Effect* e = FindEffect(seq, it->second.loc);
        if (e == nullptr) {
            it = _times.erase(it);
            continue;
        }
        it->second.host = { e->GetStartTimeMS(), e->GetEndTimeMS() };
        if (it->second.host != it->second.local) {
            SetTimes(it->second.loc, it->second.local);
        }
        ++it;
    }
}

bool CollabClient::ApplyLayoutCommit(const Message& m) {
    bool own = m.participant == _participant;
    if (!_liveLayout) {
        // This device doesn't edit the layout live; there is nothing of it
        // in memory to bring along.
        _applied = std::max(_applied, m.rev);
        if (own) {
            _pending.erase(m.clientSeq);
        }
        NoteCommitted(m.participant, m.clientSeq);
        LogCommit(m);
        return true;
    }
    auto next = LayoutDelta::Apply(_layout, m.layout);
    if (!next) {
        RequestResync("layout commit at rev " + std::to_string(m.rev) + " did not apply");
        return false;
    }
    if (LayoutDelta::Hash(*next) != m.layoutHash && !faults.clientIgnoresDrift) {
        RequestResync("layout hash differs from the host's after rev " + std::to_string(m.rev));
        return false;
    }
    if (!faults.clientSkipsRebase) {
        LiftOverlay();
    }
    if (_events.onLayout) {
        LayoutCommitInfo info;
        info.rev = m.rev;
        info.author = m.participant;
        info.clientSeq = m.clientSeq;
        info.notice = own ? m.text : std::string();
        if (!_events.onLayout(info, _layout, *next)) {
            if (!faults.clientSkipsRebase) {
                LowerOverlay();
            }
            _layoutHeld = true;
            return false;
        }
    }
    if (!faults.clientSkipsLayoutUpdate) {
        SetLayout(*next);
    }
    _applied = std::max(_applied, m.rev);
    SequenceElements& seq = _doc.Sequence();
    _hasher.InvalidateAll();
    uint64_t hash = _hasher.Hash(seq);
    if (own) {
        _pending.erase(m.clientSeq);
    }
    if (!faults.clientSkipsRebase) {
        LowerOverlay();
    }
    NoteRecent({ m.rev, m.participant, m.clientSeq, EditKind::Edit, 0 }, json{ { "type", "Layout" } });
    NoteCommitted(m.participant, m.clientSeq);
    LogCommit(m);
    if (!faults.clientIgnoresDrift && hash != m.docHash) {
        Log("document parts after layout rev " + std::to_string(m.rev) + ":\n" + DocHasher::Components(seq));
        RequestResync("document hash " + std::to_string(hash) + " differs from the host's " + std::to_string(m.docHash) + " after layout rev " +
                      std::to_string(m.rev));
        return false;
    }
    return true;
}

bool CollabClient::ApplyCommit(const Message& m) {
    if (!m.layout.is_null()) {
        return ApplyLayoutCommit(m);
    }
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
    std::unique_ptr<SequenceCommand> inverse = cmd->Apply(seq, m.lenient ? ApplyMode::Lenient : ApplyMode::Strict, &result);
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
    std::set<std::string> touched = TouchedItems(seq, *cmd, *inverse);
    for (const auto& item : touched) {
        _touch[item] = { m.rev, m.participant };
    }
    _revs[m.rev] = { m.participant, m.kind };
    NoteCommitted(m.participant, m.clientSeq);
    LogCommit(m);
    if (m.participant == _participant) {
        _pending.erase(m.clientSeq);
        for (auto it = _overlay.begin(); it != _overlay.end();) {
            it = it->second.seq == m.clientSeq ? _overlay.erase(it) : std::next(it);
        }
        for (auto it = _times.begin(); it != _times.end();) {
            it = it->second.seq == m.clientSeq ? _times.erase(it) : std::next(it);
        }
        RecordOwnCommit(m.kind, m.rev, m.clientSeq, *cmd, *inverse);
    } else {
        // A pending key write made before someone else changed the effect's
        // type will be rejected (stale_type); the host's value stands.
        for (auto it = _overlay.begin(); it != _overlay.end();) {
            it = touched.count(TypeItem(std::get<0>(it->first))) ? _overlay.erase(it) : std::next(it);
        }
    }
    if (!faults.clientSkipsRebase) {
        LowerOverlay();
    }
    CommitInfo info{ m.rev, m.participant, m.clientSeq, m.kind, m.undoOf };
    NoteRecent(info, m.command);
    NotifyCommit(info, *cmd, *inverse);
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
    if (p->second.withdrawn) {
        _pending.erase(p);
        RunQueuedUndo();
        return;
    }
    bool layout = !p->second.msg.layout.is_null();
    _pending.erase(p);
    _ownMeta.erase(m.clientSeq);
    if (layout) {
        Log("layout proposal " + std::to_string(m.clientSeq) + " not committed: " + m.reason);
        if (_events.onLayoutRejected) {
            _events.onLayoutRejected(m.clientSeq, m.reason, m.text);
        }
        RunQueuedUndo();
        return;
    }
    SequenceElements& seq = _doc.Sequence();
    // What the rollback does, for the app to re-render like any change.
    auto applied = std::make_unique<BatchCommand>("Rejected");
    std::vector<std::unique_ptr<SequenceCommand>> inverses;
    auto note = [&](Applied a) {
        if (a.first != nullptr) {
            applied->Add(std::move(a.first));
            inverses.push_back(std::move(a.second));
        }
    };
    for (auto it = _overlay.begin(); it != _overlay.end();) {
        if (it->second.seq != m.clientSeq) {
            ++it;
            continue;
        }
        if (FindEffect(seq, it->second.loc) != nullptr) {
            note(SetKey(it->second.loc, std::get<1>(it->first), std::get<2>(it->first), it->second.host));
        }
        it = _overlay.erase(it);
    }
    for (auto it = _times.begin(); it != _times.end();) {
        if (it->second.seq != m.clientSeq) {
            ++it;
            continue;
        }
        if (FindEffect(seq, it->second.loc) != nullptr) {
            note(SetTimes(it->second.loc, it->second.host));
        }
        it = _times.erase(it);
    }
    if (!inverses.empty() && _events.onRolledBack) {
        std::reverse(inverses.begin(), inverses.end());
        BatchCommand inverse("Rejected", std::move(inverses));
        _events.onRolledBack(*applied, inverse);
    }
    Log("proposal " + std::to_string(m.clientSeq) + " rejected: " + m.reason);
    if (_events.onReject) {
        _events.onReject(m.clientSeq, m.kind, m.reason, m.participant);
    }
    RunQueuedUndo();
}

SubmitResult CollabClient::Submit(std::unique_ptr<SequenceCommand> cmd) {
    return SubmitKind(std::move(cmd), EditKind::Edit, 0);
}

SubmitResult CollabClient::SubmitKind(std::unique_ptr<SequenceCommand> cmd, EditKind kind, Rev undoOf) {
    OwnMeta meta = std::move(_submitMeta);
    _submitMeta = {};
    if (!IsActive() || cmd == nullptr) {
        return { false, 0, "not_active" };
    }
    SequenceElements& seq = _doc.Sequence();
    if (kind == EditKind::Edit) {
        if (auto keys = AsKeyChanges(seq, *cmd)) {
            cmd = std::move(keys);
        } else if (cmd->Type() == CommandType::Batch) {
            // Whole-map setting changes inside a batch become key changes too,
            // so a batch of them (and moves) can apply at once.
            const auto& b = static_cast<const BatchCommand&>(*cmd);
            bool changed = false;
            auto rebuilt = std::make_unique<BatchCommand>(b.GetLabel());
            for (const auto& m : b.GetCommands()) {
                if (auto k = AsKeyChanges(seq, *m)) {
                    rebuilt->Add(std::move(k));
                    changed = true;
                } else {
                    rebuilt->Add(m->Clone());
                }
            }
            if (changed) {
                cmd = std::move(rebuilt);
            }
        }
        if (auto ordered = OrderMovesToStayClear(seq, *cmd)) {
            cmd = std::move(ordered);
        }
        if (cmd->Type() != CommandType::Batch) {
            CommandResult r = cmd->Validate(seq, ApplyMode::Strict);
            if (!r.Ok()) {
                return { false, 0, ReasonFor(r.status) };
            }
        }
    }
    bool optimistic = kind == EditKind::Edit && IsOptimisticEdit(*cmd);
    if (optimistic) {
        // What each key / time was in host order, read before applying.
        struct KeyPlan {
            OverlayKey key;
            EffectLocator loc;
            std::optional<std::string> host;
            std::optional<std::string> local;
        };
        struct TimesPlan {
            EffectLocator loc;
            std::pair<int, int> host;
            std::pair<int, int> local;
        };
        std::vector<KeyPlan> keyPlans;
        std::map<SeqObjectId, TimesPlan> timesPlans;
        std::vector<const SequenceCommand*> members;
        if (cmd->Type() == CommandType::Batch) {
            for (const auto& mem : static_cast<const BatchCommand&>(*cmd).GetCommands()) {
                members.push_back(mem.get());
            }
        } else {
            members.push_back(cmd.get());
        }
        for (const SequenceCommand* mem : members) {
            if (mem->Type() == CommandType::MoveEffect) {
                const auto& mv = static_cast<const MoveEffectCommand&>(*mem);
                const Effect* e = FindEffect(seq, mv.Target());
                if (e == nullptr) {
                    return { false, 0, Reason::TargetGone };
                }
                auto [it, inserted] = timesPlans.try_emplace(e->GetUid());
                if (inserted) {
                    it->second.loc = mv.Target();
                    it->second.host = { e->GetStartTimeMS(), e->GetEndTimeMS() };
                }
                it->second.local = { mv.GetStartTimeMS(), mv.GetEndTimeMS() };
                continue;
            }
            const auto& mc = static_cast<const ModifyEffectCommand&>(*mem);
            const Effect* e = FindEffect(seq, mc.Target());
            if (e == nullptr) {
                return { false, 0, Reason::TargetGone };
            }
            for (int pal = 0; pal < 2; ++pal) {
                const SettingsChange& ch = pal ? mc.GetPalette() : mc.GetSettings();
                const SettingsMap& map = pal ? e->GetPaletteMap() : e->GetSettings();
                for (const auto& [k, v] : ch.keys) {
                    keyPlans.push_back({ OverlayKey{ e->GetUid(), pal != 0, k }, mc.Target(), ValueOf(map, k), v });
                }
            }
        }
        CommandResult applied;
        if (cmd->Apply(seq, ApplyMode::Strict, &applied) == nullptr) {
            return { false, 0, ReasonFor(applied.status) };
        }
        _hasher.Invalidate(seq, *cmd);
        uint64_t next = _nextSeq;
        for (const auto& kp : keyPlans) {
            auto [it, inserted] = _overlay.try_emplace(kp.key);
            if (inserted) {
                it->second.loc = kp.loc;
                it->second.host = kp.host;
            }
            it->second.local = kp.local;
            it->second.seq = next;
        }
        for (const auto& [uid, tp] : timesPlans) {
            auto [it, inserted] = _times.try_emplace(uid);
            if (inserted) {
                it->second.loc = tp.loc;
                it->second.host = tp.host;
            }
            it->second.local = tp.local;
            it->second.seq = next;
        }
    }
    uint64_t s = _nextSeq++;
    Message m;
    m.type = MsgType::Propose;
    m.clientSeq = s;
    m.baseRev = _applied;
    m.kind = kind;
    m.undoOf = undoOf;
    cmd->Serialize(m.command);
    _pending[s] = { m, _now, optimistic };
    _submitMeta = std::move(meta);
    NoteOwnProposal(s);
    Send(m);
    ShareCommandFiles(*cmd);
    return { true, s, "", optimistic };
}

SubmitResult CollabClient::ProposeLayout(const json& delta) {
    if (_state != State::Active || !_liveLayout || delta.is_null()) {
        return { false, 0, "not_active" };
    }
    uint64_t s = _nextSeq++;
    Message m;
    m.type = MsgType::Propose;
    m.clientSeq = s;
    m.baseRev = _applied;
    m.layout = delta;
    _pending[s] = { m, _now, false };
    Send(m);
    return { true, s, "" };
}

bool CollabClient::Claim(SeqObjectId element, ParticipantId owner, std::string& reason, std::string& text) {
    if (_state != State::Active) {
        reason = "not_active";
        return false;
    }
    reason.clear();
    ApplyClaim(_doc.Sequence(), _roster, _assignments, _participant, element, owner, reason, text);
    if (!reason.empty()) {
        return false;
    }
    Message m;
    m.type = MsgType::Claim;
    const Element* el = _doc.Sequence().GetElementFromUid(element);
    m.assignments.push_back({ element, owner, el != nullptr ? el->GetFullName() : std::string() });
    Send(m);
    return true;
}

std::vector<CollabClient::PendingProposal> CollabClient::PendingProposals() const {
    std::vector<PendingProposal> out;
    for (const auto& [s, p] : _pending) {
        if (!p.msg.layout.is_null() || p.withdrawn) {
            continue;
        }
        out.push_back({ s, p.msg.kind, p.optimistic, p.msg.command });
    }
    return out;
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
    uint64_t prev = _lastTick != 0 ? _lastTick : _now;
    _now = std::max(_now, nowMs);
    _lastTick = _now;
    if (_now - prev > _opts.stallGapMs) {
        // This device was blocked (a slow load, a long save): the host
        // couldn't be heard meanwhile; don't count that against it.
        Log("ticks resumed after " + std::to_string(_now - prev) + " ms; not counting it against the host");
        _lastHeard = std::min(_now, _lastHeard + (_now - prev));
    }
    switch (_state) {
    case State::Joining:
        if (!TransferMoving() && _now - _lastHello >= _opts.retryMs) {
            _lastHello = _now;
            Message h;
            h.type = MsgType::Hello;
            h.appVersion = _opts.appVersion;
            h.name = _opts.displayName;
            h.deviceKind = _opts.deviceKind;
            h.liveLayout = _opts.liveLayout;
            Send(h);
        }
        break;
    case State::Checking:
        if (_checkJob && _checkJob->done) {
            auto job = std::move(_checkJob);
            _checks = std::move(job->checks);
            _hashes.Merge(job->cache);
            ContinueManifest();
            break;
        }
        if (!_haveManifest && !TransferMoving() && _now - _lastHello >= 4 * _opts.retryMs) {
            // The Welcome came but the manifest didn't: ask again.
            _lastHello = _now;
            Message h;
            h.type = MsgType::Hello;
            h.appVersion = _opts.appVersion;
            h.name = _opts.displayName;
            h.deviceKind = _opts.deviceKind;
            h.liveLayout = _opts.liveLayout;
            Send(h);
        } else if (!_wanted.empty() && !TransferMoving() && _now - _lastFileRequest >= _opts.fileRetryMs) {
            // Only once nothing has come from the host for a while: files on
            // their way are not asked for (and sent) again.
            RequestFiles();
        }
        break;
    case State::AwaitingSnapshot:
        if (!TransferMoving() && _now - _lastSnapshotRequest >= 4 * _opts.retryMs) {
            _lastSnapshotRequest = _now;
            Message n;
            n.type = MsgType::NeedSnapshot;
            n.reason = "retry";
            Send(n);
        }
        break;
    case State::Rejoining:
        if (_now - _lastHello >= _opts.retryMs) {
            SendRejoinHello();
        }
        return;
    case State::Migrating:
    case State::Succeeding:
        return;
    case State::Active:
        for (auto& [s, p] : _pending) {
            if (_now - p.lastSent >= _opts.retryMs) {
                p.lastSent = _now;
                Send(p.msg);
            }
        }
        if (!deferHeldApply) {
            ApplyHeld();
        }
        if (_state == State::Active && !_layoutHeld && (!_buffered.empty() || _hostHead > _applied)) {
            RequestResend();
        }
        break;
    default:
        return;
    }
    if (_now - _lastHeard > _opts.peerTimeoutMs) {
        HostGone();
    }
}

} // namespace Collab
