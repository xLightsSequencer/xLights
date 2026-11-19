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
    _submitMeta = { e.label, 0 };
    return SubmitKind(std::move(e.cmd), EditKind::Undo, e.rev);
}

SubmitResult CollabParticipant::Redo() {
    if (_redo.empty() || !IsActive()) {
        return { false, 0, "nothing_to_redo" };
    }
    UndoEntry e = std::move(_redo.back());
    _redo.pop_back();
    _submitMeta = { e.label, 0 };
    return SubmitKind(std::move(e.cmd), EditKind::Redo, e.rev);
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

void CollabParticipant::SetLayout(const LayoutFiles& files) {
    _layout = files;
    _layoutHash = LayoutDelta::Hash(_layout);
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
    SendRoster();
}

void CollabHost::SendRoster() {
    _roster.clear();
    _roster.push_back({ kHostParticipant, _opts.displayName, _opts.deviceKind });
    std::vector<RosterEntry> clients;
    for (const auto& [peer, c] : _clients) {
        clients.push_back({ c.id, c.name, c.deviceKind });
    }
    std::sort(clients.begin(), clients.end(), [](const RosterEntry& a, const RosterEntry& b) { return a.participant < b.participant; });
    _roster.insert(_roster.end(), clients.begin(), clients.end());
    Message m;
    m.type = MsgType::Roster;
    m.roster = _roster;
    for (const auto& [peer, c] : _clients) {
        _transport.Send(peer, m);
    }
    if (_events.onRoster) {
        _events.onRoster(_roster);
    }
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
    Outcome o = Process(kHostParticipant, seq, _applied, kind, undoOf, j);
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
    if (author == kHostParticipant) {
        RecordOwnCommit(kind, rev, clientSeq, cmd, inverse);
    }
    NotifyCommit(info, cmd, inverse);
    TrimLog();
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
    CommitApplied(kHostParticipant, seq, EditKind::Edit, 0, *applied, *inverse, true);
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
    CommitLayoutApplied(kHostParticipant, seq, delta, *next, "");
    return { true, seq, "" };
}

Rev CollabHost::CommitLayoutApplied(ParticipantId author, uint64_t clientSeq, const json& delta, const LayoutFiles& next, const std::string& notice) {
    Rev rev = ++_applied;
    SetLayout(next);
    _layoutAt[rev] = _layout;
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
        if (who == kHostParticipant) {
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

void CollabHost::DropClient(PeerId peer, const std::string& why) {
    auto it = _clients.find(peer);
    if (it == _clients.end()) {
        return;
    }
    ParticipantId id = it->second.id;
    std::string name = it->second.name;
    _clients.erase(it);
    _locks.erase(id);
    Log("participant " + std::to_string(id) + " " + why);
    if (_events.onParticipant) {
        _events.onParticipant(id, name, false);
    }
    SendRoster();
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
        if (existing->liveLayout) {
            w.liveLayout = true;
            w.layoutFiles = _layout;
            w.layoutHash = _layoutHash;
        }
        _transport.Send(from, w);
        SendManifest(*existing, false);
        Message r;
        r.type = MsgType::Roster;
        r.roster = _roster;
        _transport.Send(from, r);
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
    Message w;
    w.type = MsgType::Welcome;
    w.participant = c.id;
    w.rev = _applied;
    w.name = _opts.displayName;
    if (c.liveLayout) {
        w.liveLayout = true;
        w.layoutFiles = _layout;
        w.layoutHash = _layoutHash;
    }
    _transport.Send(from, w);
    SendManifest(c, true);
    Log("participant " + std::to_string(c.id) + " joined at rev " + std::to_string(_applied));
    if (_events.onParticipant) {
        _events.onParticipant(c.id, c.name, true);
    }
    SendRoster();
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
    if (c.liveLayout) {
        s.layoutFiles = _layout;
        s.layoutHash = _layoutHash;
    }
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
    m.participant = kHostParticipant;
    m.presence = p;
    m.lockOrder = StampLock(kHostParticipant, p.layoutLock);
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
            if (c.liveLayout) {
                p.layoutHash = _layoutHash;
            }
            _transport.Send(peer, p);
        }
    }
    for (PeerId p : gone) {
        DropClient(p, "timed out");
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
            _liveLayout = _opts.liveLayout && m.liveLayout && m.layoutHash != 0;
            if (_liveLayout) {
                SetLayout(m.layoutFiles);
            }
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
        EndSession(m.reason.empty() ? std::string(Reason::Ended) : m.reason);
        break;
    case MsgType::Roster:
        if (m.roster != _roster) {
            _roster = m.roster;
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
            if (_layoutHeld) {
                _buffered.emplace(m.rev, std::move(m));
            }
            break;
        }
    }
    for (auto it = _buffered.begin(); it != _buffered.end() && it->first <= _applied;) {
        it = _buffered.erase(it);
    }
    if (_state == State::Active && !_buffered.empty() && !_layoutHeld) {
        RequestResend();
    }
    DrainRejects();
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
}

SubmitResult CollabClient::Submit(std::unique_ptr<SequenceCommand> cmd) {
    return SubmitKind(std::move(cmd), EditKind::Edit, 0);
}

SubmitResult CollabClient::SubmitKind(std::unique_ptr<SequenceCommand> cmd, EditKind kind, Rev undoOf) {
    OwnMeta meta = std::move(_submitMeta);
    _submitMeta = {};
    if (_state != State::Active || cmd == nullptr) {
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
            h.liveLayout = _opts.liveLayout;
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
            h.liveLayout = _opts.liveLayout;
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
        EndSession(Reason::HostLost);
    }
}

} // namespace Collab
