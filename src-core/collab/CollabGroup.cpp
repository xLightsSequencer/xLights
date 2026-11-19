/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "CollabGroup.h"
#include "CollabFiles.h"
#include "CollabGroupWire.h"
#include "CollabNetWire.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <deque>
#include <filesystem>
#include <map>
#include <optional>
#include <set>
#include <system_error>

#ifdef __APPLE__
#include <sys/clonefile.h>
#endif

#include <spdlog/spdlog.h>

namespace fs = std::filesystem;

namespace Collab {

namespace {

using GroupWire::Type;

bool CopyInto(const std::string& from, const std::string& to) {
#ifdef __APPLE__
    if (clonefile(from.c_str(), to.c_str(), 0) == 0) {
        return true;
    }
#endif
    std::error_code ec;
    fs::copy_file(from, to, fs::copy_options::overwrite_existing, ec);
    if (ec) {
        fs::remove(to, ec);
        return false;
    }
    return true;
}

std::string Hex(std::string_view s) {
    return Wire::HexOf(s.substr(0, std::min<size_t>(s.size(), 16)));
}

struct OutBlob {
    BlobId id = 0;
    std::string path; // our own copy, deleted once sent or dropped
    uint64_t size = 0;
    uint64_t sent = 0;
    uint64_t acked = 0;        // of this blob, as the recipient last said
    uint64_t lastProgress = 0; // _clock() when it started or an ack moved on
    FILE* f = nullptr;
    bool started = false;
    bool cut = false; // our copy ran short: the recipient drops it
};

// A blob sent in full that waits for the recipient to acknowledge all of it.
struct Waiting {
    BlobId id = 0;
    uint64_t size = 0;
    uint64_t acked = 0;
    uint64_t lastProgress = 0;
};

struct Partial {
    uint32_t count = 0;
    uint32_t next = 0;
    std::string bytes;
};

// The other end of the session as this device sees it: on the host, a
// welcomed client; on a client, its host.
struct Remote {
    std::string member;
    PeerId id = kNoPeer;

    // Blobs to send, the first one going out now. Acknowledgements are
    // offsets into one blob, so bytes lost on the way hold up only that
    // blob (until it stalls), never the ones after it.
    std::deque<OutBlob> out;
    std::deque<Waiting> awaitingAck;

    bool inBlob = false;
    BlobId inId = 0;
    uint64_t inSize = 0;
    uint64_t inGot = 0;
    uint64_t ackPending = 0; // bytes received since the last ack
    BlobId droppedAcked = 0; // the last blob this side told the sender it dropped
    FILE* inFile = nullptr;
    std::string inPath;

    std::map<uint64_t, Partial> partials;

    void DropIn() {
        if (inFile != nullptr) {
            fclose(inFile);
            inFile = nullptr;
        }
        if (!inPath.empty()) {
            std::error_code ec;
            fs::remove(inPath, ec);
            inPath.clear();
        }
        inBlob = false;
    }
    void DropFiles() {
        std::error_code ec;
        for (auto& b : out) {
            if (b.f != nullptr) {
                fclose(b.f);
            }
            fs::remove(b.path, ec);
        }
        out.clear();
        awaitingAck.clear();
        DropIn();
        partials.clear();
    }
};

bool ParseAttachKey(const std::string& key, BlobId& id, std::string& target) {
    std::string_view k(key);
    std::string_view prefix(GroupWire::kAttachPrefix);
    if (k.substr(0, prefix.size()) != prefix) {
        return false;
    }
    k.remove_prefix(prefix.size());
    size_t a = k.find(':');
    if (a == std::string_view::npos || a == 0 || a > 20) {
        return false;
    }
    size_t b = k.find(':', a + 1);
    if (b == std::string_view::npos || b == a + 1 || b + 1 >= k.size()) {
        return false;
    }
    std::string num(k.substr(0, a));
    for (char c : num) {
        if (c < '0' || c > '9') {
            return false;
        }
    }
    id = std::strtoull(num.c_str(), nullptr, 10);
    target = std::string(k.substr(b + 1));
    return true;
}

class GroupImpl : public std::enable_shared_from_this<GroupImpl> {
public:
    GroupImpl(IGroupSessionSource& source, Dispatcher d, INetTransport* owner) :
        _source(source), _dispatch(std::move(d)), _owner(owner), _alive(std::make_shared<std::atomic<bool>>(true)) {}

    ~GroupImpl() {
        Shutdown();
        if (!_folder.empty()) {
            std::error_code ec;
            fs::remove_all(_folder, ec);
        }
    }

    bool Listen(const SessionAdvert& advert, std::string& error) {
        if (_state != NetState::Idle) {
            error = "The transport is already in use.";
            return false;
        }
        _isHost = true;
        _localPeer = kLanHostPeer;
        SetState(NetState::Connecting, "");
        _inStart = true;
        _startError.reset();
        std::weak_ptr<GroupImpl> w = weak_from_this();
        _source.Start(advert, [w](std::unique_ptr<IGroupChannel> ch, const std::string& err) {
            if (auto self = w.lock()) {
                self->OnChannel(std::move(ch), err);
            } else if (ch) {
                ch->Leave(true);
            }
        });
        _inStart = false;
        if (_startError) {
            error = *_startError;
            _localPeer = kNoPeer;
            SetState(NetState::Failed, error);
            return false;
        }
        return true;
    }

    void Connect(const std::string& sessionId) {
        if (_state != NetState::Idle) {
            return;
        }
        _isHost = false;
        SetState(NetState::Connecting, "");
        std::weak_ptr<GroupImpl> w = weak_from_this();
        _source.JoinOffered(sessionId, [w](std::unique_ptr<IGroupChannel> ch, const std::string& err) {
            if (auto self = w.lock()) {
                self->OnChannel(std::move(ch), err);
            } else if (ch) {
                ch->Leave(false);
            }
        });
    }

    void Close() {
        _alive->store(false);
        if (_state == NetState::Closed) {
            return;
        }
        _state = NetState::Closed;
        Shutdown();
    }

    NetState State() const { return _state; }
    PeerId LocalPeer() const { return _localPeer; }

    void Send(PeerId to, const Message& m) {
        Remote* r = RemoteById(to);
        if (r == nullptr) {
            return;
        }
        std::string payload = EncodeMessage(m);
        if (payload.size() > Wire::kMaxMessageBytes) {
            TooLarge(to, m, payload.size());
            return;
        }
        SendEncoded(*r, payload);
    }

    void Broadcast(const Message& m) {
        if (!Usable() || _remotes.empty()) {
            return;
        }
        std::string payload = EncodeMessage(m);
        if (payload.size() > Wire::kMaxMessageBytes) {
            TooLarge(kNoPeer, m, payload.size());
            return;
        }
        for (auto& [member, r] : _remotes) {
            if (_isHost || r.id == kLanHostPeer) {
                SendEncoded(r, payload);
            }
        }
    }

    // The copy is made off this thread (a large file, or one on another
    // volume, would freeze the app); the transfer starts when it is done.
    void SendBlob(PeerId to, BlobId id, const std::string& path) {
        Remote* r = RemoteById(to);
        if (r == nullptr) {
            return;
        }
        FILE* src = Files::OpenForCopy(path);
        if (src == nullptr) {
            spdlog::warn("collab group: could not open {} to send it", path);
            PostBlobDone(to, id, false);
            return;
        }
        std::string copy = (fs::path(Folder()) / ("out-" + std::to_string(++_files))).string();
        std::weak_ptr<GroupImpl> w = weak_from_this();
        auto alive = _alive;
        Dispatcher dispatch = _dispatch;
        std::string member = r->member;
        Files::FileCopier::Job job;
        job.src = src;
        job.dest = copy;
        job.done = [w, alive, dispatch, member, to, id, copy](bool ok, uint64_t size) {
            dispatch([w, alive, member, to, id, copy, ok, size] {
                auto self = w.lock();
                if (!self || !alive->load()) {
                    std::error_code ec;
                    fs::remove(copy, ec);
                    return;
                }
                self->Copied(member, to, id, copy, ok, size);
            });
        };
        _copier->Add(std::move(job));
    }

private:
    void Copied(const std::string& member, PeerId to, BlobId id, const std::string& copy, bool ok, uint64_t size) {
        Remote* r = Usable() ? RemoteFor(member) : nullptr;
        if (!ok || size > Wire::kMaxBlobBytes || r == nullptr || r->id != to) {
            if (!ok) {
                spdlog::warn("collab group: could not copy a file to send it");
            } else if (size > Wire::kMaxBlobBytes) {
                spdlog::warn("collab group: a file is too large to send ({} bytes)", size);
            }
            std::error_code ec;
            fs::remove(copy, ec);
            PostBlobDone(to, id, false);
            return;
        }
        uint64_t maxAttach = _ch->MaxAttachmentBytes();
        if (maxAttach > 0 && size <= maxAttach) {
            std::string key = std::string(GroupWire::kAttachPrefix) + std::to_string(id) + ":" + std::to_string(++_attachSeq) + ":" + r->member;
            // Our copy stays until the recipient acknowledges it: the upload
            // can still fail after Attach returns, and then it goes in chunks.
            if (_ch->Attach(key, copy)) {
                _attached[key] = Attached{ r->member, id, copy, size, _clock() };
                return;
            }
        }
        QueueChunks(*r, id, copy, size);
    }

private:
    struct Attached {
        std::string member;
        BlobId id = 0;
        std::string path; // our copy
        uint64_t size = 0;
        uint64_t lastProgress = 0; // _clock() when attached, or the recipient last showed progress
    };

    // An attachment the recipient hasn't acknowledged, with no sign of it
    // getting anything (an ack of other data) for this long plus time for
    // its size at a slow rate, never will be (the store lost it): it goes
    // again in chunks.
    static constexpr uint64_t kAttachAckMs = 60000;
    static constexpr uint64_t kAttachMinBytesPerMs = 64; // 64 KB/s

    // The recipient `member` acknowledged something: it is getting data.
    void NoteProgress(const std::string& member) {
        uint64_t now = _clock();
        for (auto& [key, a] : _attached) {
            if (a.member == member) {
                a.lastProgress = now;
            }
        }
    }

    void ExpireAttachments(const std::string& member) {
        uint64_t now = _clock();
        std::vector<std::string> late;
        for (const auto& [key, a] : _attached) {
            if (a.member == member && now - a.lastProgress >= kAttachAckMs + a.size / kAttachMinBytesPerMs) {
                late.push_back(key);
            }
        }
        for (const auto& key : late) {
            spdlog::warn("collab group: an attachment was never acknowledged; sending it in chunks");
            OnAttachFailed(key);
        }
    }

    void QueueChunks(Remote& r, BlobId id, const std::string& copy, uint64_t size) {
        OutBlob b;
        b.id = id;
        b.path = copy;
        b.size = size;
        r.out.push_back(std::move(b));
        Pump(r);
    }

    void DropAttached(std::map<std::string, Attached>::iterator it) {
        if (_ch) {
            _ch->Detach(it->first);
        }
        std::error_code ec;
        fs::remove(it->second.path, ec);
        _attached.erase(it);
    }

    void OnAttachFailed(const std::string& key) {
        auto it = _attached.find(key);
        if (it == _attached.end()) {
            return;
        }
        spdlog::warn("collab group: an attachment could not be uploaded; sending it in chunks");
        Attached a = it->second;
        _attached.erase(it);
        _ch->Detach(key);
        Remote* r = _dead ? nullptr : RemoteFor(a.member);
        if (r == nullptr) {
            std::error_code ec;
            fs::remove(a.path, ec);
            return;
        }
        QueueChunks(*r, a.id, a.path, a.size);
    }

    bool Usable() const { return _ch != nullptr && !_dead && _state != NetState::Closed; }

    std::string Folder() {
        if (_folder.empty()) {
            std::error_code ec;
            fs::path dir = fs::temp_directory_path(ec) / ("xlgroup-" + Wire::HexOf(Wire::NewSalt()));
            fs::create_directories(dir, ec);
            _folder = dir.string();
        }
        return _folder;
    }

    void Post(std::function<void(INetTransport&)> fn) {
        auto alive = _alive;
        INetTransport* owner = _owner;
        _dispatch([alive, owner, fn]() {
            if (alive->load()) {
                fn(*owner);
            }
        });
    }

    void SetState(NetState s, const std::string& reason) {
        if (_state == NetState::Closed) {
            return;
        }
        _state = s;
        Post([s, reason](INetTransport& t) {
            if (t.onState) {
                t.onState(s, reason);
            }
        });
    }

    void PostBlobDone(PeerId to, BlobId id, bool sent) {
        Post([to, id, sent](INetTransport& t) {
            if (t.onBlobDone) {
                t.onBlobDone(to, id, sent);
            }
        });
    }

    void PostBlobProgress(PeerId from, BlobId id, uint64_t received, uint64_t size) {
        Post([from, id, received, size](INetTransport& t) {
            if (t.onBlobProgress) {
                t.onBlobProgress(from, id, received, size);
            }
        });
    }

    void TooLarge(PeerId to, const Message& m, size_t bytes) {
        std::string why = "a message of " + std::to_string(bytes) + " bytes is too large to send";
        spdlog::warn("collab group: {}", why);
        Message summary = SendFailedSummary(m);
        Post([to, summary, why](INetTransport& t) {
            if (t.onSendFailed) {
                t.onSendFailed(to, summary, why);
            }
        });
    }

    void PostPeer(PeerId id, PeerEvent e) {
        Post([id, e](INetTransport& t) {
            if (t.onPeer) {
                t.onPeer(id, e);
            }
        });
    }

    // Drops the channel and every transfer. No callbacks follow.
    void Shutdown() {
        if (_ch) {
            if (!_dead) {
                for (auto& [member, r] : _remotes) {
                    SendFrame(member, Type::Bye);
                }
            }
            while (!_attached.empty()) {
                DropAttached(_attached.begin());
            }
            _ch->onMessage = nullptr;
            _ch->onMembers = nullptr;
            _ch->onAttachment = nullptr;
            _ch->wantsAttachment = nullptr;
            _ch->onAttachFailed = nullptr;
            _ch->onEnded = nullptr;
            if (!_left) {
                _ch->Leave(_isHost);
            }
            _ch.reset();
        }
        _dead = true;
        _closed = true;
        for (auto& [member, r] : _remotes) {
            r.DropFiles();
        }
        _remotes.clear();
    }

    void OnChannel(std::unique_ptr<IGroupChannel> ch, const std::string& err) {
        if (_closed || _state == NetState::Closed || _state == NetState::Failed) {
            if (ch) {
                ch->Leave(_isHost);
            }
            return;
        }
        if (!ch) {
            std::string why = err.empty() ? (_isHost ? "SharePlay could not start the session." : "Couldn't join the SharePlay session.") : err;
            if (_inStart) {
                _startError = why;
                return;
            }
            SetState(NetState::Failed, why);
            return;
        }
        if (ch->MaxMessageBytes() < GroupWire::kMinMessageBytes) {
            ch->Leave(_isHost);
            std::string why = "The SharePlay connection can't carry a session.";
            if (_inStart) {
                _startError = why;
                return;
            }
            SetState(NetState::Failed, why);
            return;
        }
        _ch = std::move(ch);
        _local = _ch->LocalId();
        _ch->onMessage = [this](const std::string& from, const std::string& bytes) { OnMessage(from, bytes); };
        _ch->onMembers = [this](const std::vector<GroupMember>& members) { OnMembers(members); };
        _ch->onAttachment = [this](const std::string& from, const std::string& key, const std::string& path) { OnAttachment(from, key, path); };
        _ch->wantsAttachment = [local = _local](const std::string& key) { return GroupAttachmentIsFor(key, local); };
        _ch->onAttachFailed = [this](const std::string& key) { OnAttachFailed(key); };
        _ch->onEnded = [this](const std::string& reason) { OnEnded(reason); };
        if (_isHost) {
            if (!_ch->IsInitiator()) {
                spdlog::info("collab group: hosting on a session this device did not start");
            }
            SetState(NetState::Listening, "");
            std::string p;
            GroupWire::PutU32(p, (uint32_t)kProtocolVersion);
            SendFrame("", Type::Claim, p);
        } else {
            SendFrame("", Type::Hello);
        }
    }

    // ---- outgoing ----

    void SendFrame(const std::string& to, Type type, std::string_view payload = {}) {
        if (!_ch || _left) {
            return;
        }
        _ch->Send(to, GroupWire::Frame(type, payload));
    }

    void SendEncoded(Remote& r, const std::string& payload) {
        size_t max = _ch->MaxMessageBytes();
        if (payload.size() + GroupWire::kHeaderBytes <= max) {
            SendFrame(r.member, Type::Msg, payload);
            return;
        }
        size_t chunk = max - GroupWire::kPartHeaderBytes;
        uint32_t count = (uint32_t)((payload.size() + chunk - 1) / chunk);
        uint64_t msgId = ++_msgIds;
        for (uint32_t i = 0; i < count; ++i) {
            std::string p;
            p.reserve(GroupWire::kPartHeaderBytes + chunk);
            GroupWire::PutU64(p, msgId);
            GroupWire::PutU32(p, i);
            GroupWire::PutU32(p, count);
            p.append(payload, (size_t)i * chunk, chunk);
            SendFrame(r.member, Type::MsgPart, p);
        }
    }

    size_t BlobChunk() const {
        return std::min(GroupWire::kMaxBlobChunkBytes, _ch->MaxMessageBytes() - GroupWire::kDataHeaderBytes);
    }

    // Blob data goes out while the recipient has acknowledged all but a
    // window of it; messages never wait for the window.
    void Pump(Remote& r) {
        uint64_t window = std::max<uint64_t>(GroupWire::kBlobWindow, 2 * BlobChunk());
        while (!r.out.empty() && Usable()) {
            OutBlob& b = r.out.front();
            std::string p;
            GroupWire::PutU64(p, b.id);
            if (!b.started) {
                b.f = fopen(b.path.c_str(), "rb");
                if (b.f == nullptr) {
                    std::error_code ec;
                    fs::remove(b.path, ec);
                    PostBlobDone(r.id, b.id, false);
                    r.out.pop_front();
                    continue;
                }
                b.started = true;
                b.lastProgress = _clock();
                GroupWire::PutU64(p, b.size);
                SendFrame(r.member, Type::BlobStart, p);
                continue;
            }
            if (b.sent < b.size) {
                if (b.sent - b.acked >= window) {
                    return;
                }
                std::string chunk(std::min<uint64_t>(BlobChunk(), b.size - b.sent), '\0');
                size_t n = fread(chunk.data(), 1, chunk.size(), b.f);
                if (n == 0) {
                    // Our own copy shrank or can't be read: end it short,
                    // and the recipient drops it.
                    spdlog::warn("collab group: a file being sent could not be read");
                    b.sent = b.size;
                    b.cut = true;
                    continue;
                }
                chunk.resize(n);
                p.append(chunk);
                b.sent += n;
                SendFrame(r.member, Type::BlobData, p);
                continue;
            }
            SendFrame(r.member, Type::BlobEnd, p);
            fclose(b.f);
            std::error_code ec;
            fs::remove(b.path, ec);
            if (b.cut) {
                PostBlobDone(r.id, b.id, false);
            } else if (b.acked >= b.size) {
                PostBlobDone(r.id, b.id, true);
            } else {
                // Sent, but only done once the recipient has it all.
                r.awaitingAck.push_back({ b.id, b.size, b.acked, b.lastProgress });
            }
            r.out.pop_front();
        }
    }

    // The recipient has `got` bytes of blob `id` (kAckDropped: it dropped it).
    void OnAck(Remote& r, BlobId id, uint64_t got) {
        uint64_t now = _clock();
        if (!r.out.empty() && r.out.front().started && r.out.front().id == id) {
            OutBlob& b = r.out.front();
            if (got == GroupWire::kAckDropped) {
                AbortFront(r);
            } else if (got > b.acked) {
                b.acked = std::min(got, b.size);
                b.lastProgress = now;
            }
            Pump(r);
            return;
        }
        for (auto it = r.awaitingAck.begin(); it != r.awaitingAck.end(); ++it) {
            if (it->id != id) {
                continue;
            }
            if (got == GroupWire::kAckDropped || got >= it->size) {
                PostBlobDone(r.id, id, got != GroupWire::kAckDropped);
                r.awaitingAck.erase(it);
            } else if (got > it->acked) {
                it->acked = got;
                it->lastProgress = now;
            }
            return;
        }
    }

    // Gives up on the blob going out now; the next one starts.
    void AbortFront(Remote& r) {
        OutBlob& b = r.out.front();
        if (b.f != nullptr) {
            fclose(b.f);
        }
        std::error_code ec;
        fs::remove(b.path, ec);
        PostBlobDone(r.id, b.id, false);
        r.out.pop_front();
    }

    // A blob whose acknowledgements stopped moving (bytes lost on the way,
    // a recipient that dropped it unsaid) is given up: the session sends it
    // again if it still wants it, and what follows isn't held up.
    static constexpr uint64_t kBlobStallMs = 30000;

    void ExpireStalledBlobs(Remote& r) {
        uint64_t now = _clock();
        bool aborted = false;
        if (!r.out.empty() && r.out.front().started && now - r.out.front().lastProgress >= kBlobStallMs) {
            spdlog::warn("collab group: a file being sent stopped being acknowledged; giving it up");
            AbortFront(r);
            aborted = true;
        }
        for (auto it = r.awaitingAck.begin(); it != r.awaitingAck.end();) {
            if (now - it->lastProgress >= kBlobStallMs) {
                PostBlobDone(r.id, it->id, false);
                it = r.awaitingAck.erase(it);
            } else {
                ++it;
            }
        }
        if (aborted) {
            Pump(r);
        }
    }

    // Everything on its way to `r` is abandoned (it left, the group ended):
    // the session must not wait for any of it.
    void AbandonBlobs(Remote& r) {
        for (const auto& b : r.out) {
            PostBlobDone(r.id, b.id, false);
        }
        for (const auto& w : r.awaitingAck) {
            PostBlobDone(r.id, w.id, false);
        }
        for (auto a = _attached.begin(); a != _attached.end();) {
            if (a->second.member == r.member) {
                PostBlobDone(r.id, a->second.id, false);
                auto next = std::next(a);
                DropAttached(a);
                a = next;
            } else {
                ++a;
            }
        }
    }

    void Ack(Remote& r) {
        if (r.ackPending == 0 || !r.inBlob) {
            return;
        }
        r.ackPending = 0;
        SendAck(r, r.inId, r.inGot);
    }

    void SendAck(Remote& r, BlobId id, uint64_t got) {
        std::string p;
        GroupWire::PutU64(p, id);
        GroupWire::PutU64(p, got);
        SendFrame(r.member, Type::BlobAck, p);
    }

    // Data of a blob this side isn't receiving (it dropped it): the sender
    // is told once, so it stops sending it and moves on.
    void AckDropped(Remote& r, BlobId id) {
        if (r.droppedAcked != id) {
            r.droppedAcked = id;
            SendAck(r, id, GroupWire::kAckDropped);
        }
    }

    // ---- incoming ----

    Remote* RemoteById(PeerId id) {
        if (!Usable()) {
            return nullptr;
        }
        for (auto& [member, r] : _remotes) {
            if (r.id == id) {
                return &r;
            }
        }
        return nullptr;
    }

    Remote* RemoteFor(const std::string& member) {
        auto it = _remotes.find(member);
        return it == _remotes.end() ? nullptr : &it->second;
    }

    void Malformed(const std::string& from, const char* what) {
        ++_malformed;
        spdlog::warn("collab group: dropped {} from member {}", what, Hex(from));
    }

    void OnMembers(const std::vector<GroupMember>& members) {
        if (_dead) {
            return;
        }
        std::set<std::string> now;
        for (const auto& m : members) {
            now.insert(m.id);
            _seenMembers.insert(m.id);
        }
        if (_isHost) {
            for (const auto& id : now) {
                if (id != _local && _claimed.insert(id).second) {
                    std::string p;
                    GroupWire::PutU32(p, (uint32_t)kProtocolVersion);
                    SendFrame(id, Type::Claim, p);
                }
            }
            for (auto it = _claimed.begin(); it != _claimed.end();) {
                it = now.count(*it) ? std::next(it) : _claimed.erase(it);
            }
        } else {
            for (auto it = _candidates.begin(); it != _candidates.end();) {
                it = now.count(*it) ? std::next(it) : _candidates.erase(it);
            }
        }
        std::vector<std::string> gone;
        for (auto& [member, r] : _remotes) {
            // A member list from before it joined doesn't count.
            if (!now.count(member) && _seenMembers.count(member)) {
                gone.push_back(member);
            }
        }
        for (const auto& member : gone) {
            RemoteGone(member, false);
        }
        if (!_isHost && _remotes.empty() && !_hostGone) {
            ChooseHost();
        }
    }

    // `bye`: it said goodbye first.
    void RemoteGone(const std::string& member, bool bye) {
        auto it = _remotes.find(member);
        if (it == _remotes.end()) {
            return;
        }
        PeerId id = it->second.id;
        AbandonBlobs(it->second);
        it->second.DropFiles();
        _remotes.erase(it);
        if (_isHost) {
            PostPeer(id, PeerEvent::Left);
            return;
        }
        _hostGone = true;
        _dead = true;
        PostPeer(id, bye ? PeerEvent::Left : PeerEvent::Lost);
        SetState(NetState::Failed, bye ? "The host ended the session." : "The connection to the host was lost.");
    }

    void ChooseHost() {
        if (_candidates.empty() || *_candidates.begin() == _chosen) {
            return;
        }
        _chosen = *_candidates.begin();
        SendFrame(_chosen, Type::Hello);
    }

    // Two devices claim the session: the lower member id hosts.
    void OnClaim(const std::string& from) {
        if (_isHost) {
            if (from < _local) {
                LoseClaim();
            } else {
                spdlog::info("collab group: member {} also claims the session; this device keeps it", Hex(from));
            }
            return;
        }
        if (!_remotes.empty() || _hostGone) {
            if (_remotes.empty() || _remotes.begin()->first != from) {
                spdlog::info("collab group: ignoring a claim from member {}: already in a session", Hex(from));
            }
            return;
        }
        _candidates.insert(from);
        ChooseHost();
    }

    // Another member took over this session: this device stops hosting it.
    // It stays in the group (it can join the new host as anyone would);
    // its old clients are already the new host's, so they aren't told.
    void StandDown() {
        spdlog::warn("collab group: another member took over this session; standing down");
        for (auto& [member, r] : _remotes) {
            AbandonBlobs(r);
            r.DropFiles();
            PostPeer(r.id, PeerEvent::Lost);
        }
        _remotes.clear();
        _claimed.clear();
        _isHost = false;
        _dead = true;
        SetState(NetState::Failed, "Another device took over this session while this one was not responding.");
    }

    void LoseClaim() {
        spdlog::warn("collab group: another member hosts this session; giving it up");
        for (auto& [member, r] : _remotes) {
            SendFrame(member, Type::Bye);
            r.DropFiles();
            PostPeer(r.id, PeerEvent::Lost);
        }
        _remotes.clear();
        _dead = true;
        if (_ch && !_left) {
            _left = true;
            _ch->Leave(false);
        }
        SetState(NetState::Failed, "Someone else in this call is already hosting a session.");
    }

    void OnHello(const std::string& from) {
        if (!_isHost) {
            return; // another client looking for the host
        }
        Remote* r = RemoteFor(from);
        bool added = r == nullptr;
        if (added) {
            Remote nr;
            nr.member = from;
            nr.id = _nextPeer++;
            r = &_remotes.emplace(from, std::move(nr)).first->second;
        }
        _claimed.insert(from);
        std::string p;
        GroupWire::PutU32(p, r->id);
        SendFrame(from, Type::Welcome, p);
        if (added) {
            PostPeer(r->id, PeerEvent::Joined);
        }
    }

    void OnWelcome(const std::string& from, std::string_view p) {
        uint32_t assigned = 0;
        if (_isHost || p.size() != 4 || !Wire::ReadU32(p, 0, assigned) || assigned == kNoPeer || assigned == kLanHostPeer) {
            Malformed(from, "a malformed welcome");
            return;
        }
        if (!_remotes.empty() || _hostGone) {
            if (!_remotes.empty() && _remotes.begin()->first == from && assigned == _localPeer) {
                return; // a repeat
            }
            Malformed(from, "a welcome while in a session");
            return;
        }
        _candidates.insert(from);
        ChooseHost();
        if (from != _chosen) {
            return; // a lower claimant will welcome us
        }
        Remote r;
        r.member = from;
        r.id = kLanHostPeer;
        _remotes.emplace(from, std::move(r));
        _candidates.clear();
        _localPeer = assigned;
        SetState(NetState::Connected, "");
        PostPeer(kLanHostPeer, PeerEvent::Joined);
    }

    void OnMessage(const std::string& from, const std::string& bytes) {
        if (_dead || from == _local) {
            return;
        }
        if (bytes.size() < GroupWire::kHeaderBytes || (uint8_t)bytes[0] != GroupWire::kVersion) {
            Malformed(from, "a frame of an unknown version");
            return;
        }
        Type type = (Type)(uint8_t)bytes[1];
        std::string_view p(bytes.data() + GroupWire::kHeaderBytes, bytes.size() - GroupWire::kHeaderBytes);
        switch (type) {
        case Type::Claim:
            OnClaim(from);
            return;
        case Type::Hello:
            OnHello(from);
            return;
        case Type::Welcome:
            OnWelcome(from, p);
            return;
        case Type::Bye:
            if (_remotes.count(from)) {
                RemoteGone(from, true);
            } else if (!_isHost) {
                _candidates.erase(from);
                if (_chosen == from) {
                    _chosen.clear();
                    ChooseHost();
                }
            }
            return;
        case Type::Msg:
        case Type::MsgPart:
        case Type::BlobStart:
        case Type::BlobData:
        case Type::BlobEnd:
        case Type::BlobAck:
        case Type::AttachAck:
            break;
        default:
            Malformed(from, "a frame of an unknown type");
            return;
        }
        Remote* r = RemoteFor(from);
        if (r == nullptr) {
            Malformed(from, "a session frame from outside the session");
            return;
        }
        // Frames keep coming from a member in a session, so this is where an
        // attachment it never acknowledged is noticed.
        if (!_attached.empty() && type != Type::AttachAck) {
            ExpireAttachments(from);
            r = RemoteFor(from);
            if (r == nullptr) {
                return;
            }
        }
        if (type != Type::BlobAck) {
            ExpireStalledBlobs(*r);
        }
        switch (type) {
        case Type::Msg:
            Deliver(*r, p);
            return;
        case Type::MsgPart:
            OnPart(*r, p);
            return;
        case Type::BlobStart:
            OnBlobStart(*r, p);
            return;
        case Type::BlobData:
            OnBlobData(*r, p);
            return;
        case Type::BlobEnd:
            OnBlobEnd(*r, p);
            return;
        case Type::BlobAck: {
            uint64_t id = 0, got = 0;
            if (p.size() != 16 || !Wire::ReadU64(p, 0, id) || !Wire::ReadU64(p, 8, got)) {
                Malformed(from, "a malformed blob ack");
                return;
            }
            OnAck(*r, id, got);
            NoteProgress(from);
            return;
        }
        case Type::AttachAck: {
            std::string key(p);
            auto it = _attached.find(key);
            if (it == _attached.end()) {
                Malformed(from, "an ack for an unknown attachment");
                return;
            }
            BlobId id = 0;
            std::string target;
            if (!ParseAttachKey(key, id, target) || target != from) {
                Malformed(from, "an ack for someone else's attachment");
                return;
            }
            DropAttached(it);
            PostBlobDone(r->id, id, true);
            NoteProgress(from);
            return;
        }
        default:
            return;
        }
    }

    void Deliver(Remote& r, std::string_view encoded) {
        auto m = DecodeMessage(encoded);
        if (!m) {
            Malformed(r.member, "a message that doesn't decode");
            return;
        }
        PeerId from = r.id;
        Message msg = std::move(*m);
        Post([from, msg](INetTransport& t) {
            if (t.onMessage) {
                t.onMessage(from, msg);
            }
        });
    }

    void OnPart(Remote& r, std::string_view p) {
        uint64_t msgId = 0;
        uint32_t index = 0, count = 0;
        if (p.size() <= 16 || !Wire::ReadU64(p, 0, msgId) || !Wire::ReadU32(p, 8, index) || !Wire::ReadU32(p, 12, count) || count < 2 ||
            index >= count) {
            Malformed(r.member, "a malformed message part");
            return;
        }
        std::string_view data = p.substr(16);
        auto it = r.partials.find(msgId);
        if (it == r.partials.end()) {
            if (index != 0) {
                Malformed(r.member, "a message part without its start");
                return;
            }
            if (r.partials.size() >= GroupWire::kMaxPartials) {
                Malformed(r.member, "a message part beyond the reassembly limit");
                r.partials.erase(r.partials.begin());
            }
            it = r.partials.emplace(msgId, Partial{ count, 0, {} }).first;
        }
        Partial& part = it->second;
        if (index != part.next || count != part.count) {
            Malformed(r.member, "a message part out of order");
            r.partials.erase(it);
            return;
        }
        if (part.bytes.size() + data.size() > Wire::kMaxMessageBytes) {
            Malformed(r.member, "a message over the size limit");
            r.partials.erase(it);
            return;
        }
        part.bytes.append(data);
        if (++part.next < part.count) {
            return;
        }
        std::string whole = std::move(part.bytes);
        r.partials.erase(it);
        Deliver(r, whole);
    }

    void OnBlobStart(Remote& r, std::string_view p) {
        uint64_t id = 0, size = 0;
        if (p.size() != 16 || !Wire::ReadU64(p, 0, id) || !Wire::ReadU64(p, 8, size) || size > Wire::kMaxBlobBytes) {
            Malformed(r.member, "a malformed blob start");
            return;
        }
        if (r.inBlob) {
            Malformed(r.member, "a blob start inside another blob");
            r.DropIn();
        }
        r.inPath = (fs::path(Folder()) / ("in-" + std::to_string(++_files))).string();
        r.inFile = fopen(r.inPath.c_str(), "wb");
        if (r.inFile == nullptr) {
            spdlog::warn("collab group: could not create {} for an incoming file", r.inPath);
            r.inPath.clear();
            return;
        }
        r.inBlob = true;
        r.inId = id;
        r.inSize = size;
        r.inGot = 0;
        PostBlobProgress(r.id, id, 0, size);
    }

    void OnBlobData(Remote& r, std::string_view p) {
        uint64_t id = 0;
        if (p.size() <= 8 || !Wire::ReadU64(p, 0, id)) {
            Malformed(r.member, "malformed blob data");
            return;
        }
        size_t n = p.size() - 8;
        r.ackPending += n;
        if (!r.inBlob || id != r.inId || r.inGot + n > r.inSize) {
            Malformed(r.member, "blob data outside a blob");
            r.DropIn();
            AckDropped(r, id);
            return;
        } else if (fwrite(p.data() + 8, 1, n, r.inFile) != n) {
            spdlog::warn("collab group: could not write an incoming file");
            r.DropIn();
            AckDropped(r, id);
            return;
        } else {
            // Every chunk: a group moves only a window of data per round
            // trip, so a megabyte can take longer than a stall.
            r.inGot += n;
            PostBlobProgress(r.id, id, r.inGot, r.inSize);
        }
        if (r.ackPending >= GroupWire::kAckEvery) {
            Ack(r);
        }
    }

    void OnBlobEnd(Remote& r, std::string_view p) {
        uint64_t id = 0;
        if (p.size() != 8 || !Wire::ReadU64(p, 0, id) || !r.inBlob || id != r.inId || r.inGot != r.inSize) {
            Malformed(r.member, "a blob end that doesn't match its blob");
            r.DropIn();
            AckDropped(r, id);
            return;
        }
        r.ackPending = 0;
        SendAck(r, id, r.inGot);
        bool ok = fclose(r.inFile) == 0;
        r.inFile = nullptr;
        r.inBlob = false;
        std::string path = r.inPath;
        r.inPath.clear();
        if (!ok) {
            std::error_code ec;
            fs::remove(path, ec);
            return;
        }
        PostBlob(r.id, id, path);
    }

    void PostBlob(PeerId from, BlobId id, const std::string& path) {
        Post([from, id, path](INetTransport& t) {
            if (t.onBlob) {
                t.onBlob(from, id, path);
            }
            std::error_code ec;
            fs::remove(path, ec);
        });
    }

    void OnAttachment(const std::string& from, const std::string& key, const std::string& path) {
        if (_dead) {
            return;
        }
        BlobId id = 0;
        std::string target;
        if (!ParseAttachKey(key, id, target)) {
            Malformed(from, "an attachment with a malformed key");
            return;
        }
        if (target != _local) {
            return; // the store reaches everyone; this one is for another member
        }
        Remote* r = RemoteFor(from);
        if (r == nullptr) {
            Malformed(from, "an attachment from outside the session");
            return;
        }
        SendFrame(from, Type::AttachAck, key);
        std::string copy = (fs::path(Folder()) / ("in-" + std::to_string(++_files))).string();
        // The file is ours only during this call: taking it is instant where
        // a copy of a large one would hold up the app.
        std::error_code ec;
        fs::rename(path, copy, ec);
        if (ec && !CopyInto(path, copy)) {
            spdlog::warn("collab group: could not keep an incoming attachment");
            return;
        }
        PostBlob(r->id, id, copy);
    }

    void OnEnded(const std::string& reason) {
        if (_dead) {
            return;
        }
        _dead = true;
        _left = true;
        bool wasConnected = !_remotes.empty();
        for (auto& [member, r] : _remotes) {
            AbandonBlobs(r);
            r.DropFiles();
            PostPeer(r.id, PeerEvent::Lost);
        }
        _remotes.clear();
        if (!_isHost && _hostGone) {
            return;
        }
        std::string why;
        if (!_isHost && !wasConnected) {
            why = "The SharePlay session ended before the host answered.";
        } else if (!_isHost) {
            why = "The connection to the host was lost.";
        } else {
            why = "The SharePlay session ended.";
        }
        if (!reason.empty()) {
            why += " (" + reason + ")";
        }
        SetState(reason.empty() ? NetState::Closed : NetState::Failed, why);
    }

    IGroupSessionSource& _source;
    Dispatcher _dispatch;
    INetTransport* _owner;
    std::shared_ptr<std::atomic<bool>> _alive;
    std::unique_ptr<IGroupChannel> _ch;
    NetState _state = NetState::Idle;
    PeerId _localPeer = kNoPeer;
    bool _isHost = false;
    bool _inStart = false;
    std::optional<std::string> _startError;
    bool _dead = false;   // the channel is finished; nothing more goes out or comes in
    bool _left = false;   // Leave() was called (or the platform ended it)
    bool _closed = false; // Close() or destruction
    bool _hostGone = false;
    std::string _local;
    std::map<std::string, Remote> _remotes; // by member id
    std::set<std::string> _seenMembers;     // every member any member list named
    std::set<std::string> _claimed;         // host: members sent a claim
    std::set<std::string> _candidates;      // client: members that claimed
    std::string _chosen;                    // client: the claimant asked for a welcome
    PeerId _nextPeer = kLanHostPeer + 1;
    std::map<std::string, Attached> _attached; // by key, until acknowledged
    std::function<uint64_t()> _clock = [] {
        return (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    };
    uint64_t _attachSeq = 0;
    uint64_t _msgIds = 0;
    uint64_t _files = 0;
    std::shared_ptr<Files::FileCopier> _copier = std::make_shared<Files::FileCopier>();
    std::string _folder;

public:
    size_t _malformed = 0;
};

class GroupTransport : public INetTransport {
public:
    GroupTransport(IGroupSessionSource& source, Dispatcher d) : _impl(std::make_shared<GroupImpl>(source, std::move(d), this)) {}
    ~GroupTransport() override { _impl->Close(); }

    PeerId LocalPeer() const override { return _impl->LocalPeer(); }
    void Send(PeerId to, const Message& m) override { _impl->Send(to, m); }
    void Broadcast(const Message& m) override { _impl->Broadcast(m); }
    void SendBlob(PeerId to, BlobId id, const std::string& path) override { _impl->SendBlob(to, id, path); }
    bool Listen(const std::string& /*pairingCode*/, const SessionAdvert& advert, std::string& error) override { return _impl->Listen(advert, error); }
    void Connect(const std::string& sessionId, const std::string& /*pairingCode*/) override { _impl->Connect(sessionId); }
    void Close() override { _impl->Close(); }
    NetState State() const override { return _impl->State(); }

    GroupImpl& Impl() { return *_impl; }

private:
    std::shared_ptr<GroupImpl> _impl;
};

class GroupBrowser : public ISessionBrowser {
public:
    GroupBrowser(IGroupSessionSource& source, Dispatcher d) : _source(source), _dispatch(std::move(d)) {}
    ~GroupBrowser() override { Stop(); }

    // The source has one slot for offers; the newest browser takes it, and
    // a stopped one leaves a hook that does nothing.
    void Start() override {
        Stop();
        auto alive = std::make_shared<std::atomic<bool>>(true);
        _alive = alive;
        GroupBrowser* self = this;
        _source.onSessionOffered = [alive, self](const SessionAdvert& a) {
            if (alive->load()) {
                self->Offered(a);
            }
        };
        Offered(_source.OfferedSession());
    }

    void Stop() override {
        if (_alive) {
            _alive->store(false);
            _alive.reset();
        }
    }

private:
    void Offered(const SessionAdvert& a) {
        std::vector<SessionAdvert> list;
        if (!a.id.empty()) {
            SessionAdvert ad = a;
            ad.group = true;
            list.push_back(ad);
        }
        auto alive = _alive;
        GroupBrowser* self = this;
        _dispatch([alive, self, list] {
            if (alive && alive->load() && self->onChanged) {
                self->onChanged(list);
            }
        });
    }

    IGroupSessionSource& _source;
    Dispatcher _dispatch;
    std::shared_ptr<std::atomic<bool>> _alive;
};

class NoGroupSessions : public IGroupSessionSource {
public:
    bool Available() const override { return false; }
    bool InGroup() const override { return false; }
    void Start(const SessionAdvert&, std::function<void(std::unique_ptr<IGroupChannel>, const std::string&)> ready) override {
        ready(nullptr, "SharePlay isn't available on this device.");
    }
    void JoinOffered(const std::string&, std::function<void(std::unique_ptr<IGroupChannel>, const std::string&)> ready) override {
        ready(nullptr, "SharePlay isn't available on this device.");
    }
};

[[maybe_unused]] IGroupSessionSource& Unavailable() {
    static NoGroupSessions none;
    return none;
}

} // namespace

std::unique_ptr<INetTransport> CreateGroupTransport(IGroupSessionSource& source, Dispatcher dispatch) {
    return std::make_unique<GroupTransport>(source, std::move(dispatch));
}

std::unique_ptr<ISessionBrowser> CreateGroupBrowser(IGroupSessionSource& source, Dispatcher dispatch) {
    return std::make_unique<GroupBrowser>(source, std::move(dispatch));
}

#ifndef __APPLE__
IGroupSessionSource& PlatformGroupSessions() {
    return Unavailable();
}
#endif


namespace GroupTest {
size_t MalformedFrames(INetTransport& t) {
    auto* g = dynamic_cast<GroupTransport*>(&t);
    return g != nullptr ? g->Impl()._malformed : 0;
}
} // namespace GroupTest

bool GroupAttachmentIsFor(const std::string& key, const std::string& member) {
    BlobId id = 0;
    std::string target;
    return ParseAttachKey(key, id, target) && !member.empty() && target == member;
}

} // namespace Collab
