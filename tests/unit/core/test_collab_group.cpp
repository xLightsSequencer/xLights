// Sessions over an operating-system group (CollabGroup.h), against an
// in-process fake of the platform: members joining and leaving, reliable
// messages per sender with a size limit, an optional attachment store, and
// the group ending. Every callback runs from the fake's queue, as the
// platform runs them on the main thread.

#include <catch2/catch_test_macros.hpp>

#include "test_collab_join_common.h"

#include "collab/CollabController.h"
#include "collab/CollabFiles.h"
#include "collab/CollabGroup.h"
#include "collab/CollabGroupWire.h"
#include "collab/CollabLayout.h"
#include "collab/CollabNet.h"
#include "collab/CollabNetWire.h"
#include "models/Model.h"
#include "render/ShowMergeApply.h"
#include "XmlSerializer/XmlSerializer.h"
#include "XmlSerializer/XmlSerializingVisitor.h"

#include <pugixml.hpp>
#include <spdlog/sinks/null_sink.h>
#include <spdlog/spdlog.h>

#include <atomic>
#include <chrono>
#include <deque>
#include <filesystem>
#include <list>
#include <map>
#include <mutex>
#include <optional>
#include <random>
#include <set>
#include <thread>

using namespace Collab;
using namespace CollabTest;
namespace fs = std::filesystem;

namespace {

using Ready = std::function<void(std::unique_ptr<IGroupChannel>, const std::string&)>;

// A device's Dispatcher queue. File copies finish on a worker thread and
// post from there.
class PostQueue {
public:
    void push_back(std::function<void()> fn) {
        std::lock_guard<std::mutex> l(_m);
        _q.push_back(std::move(fn));
    }
    // The next posted function, or an empty one.
    std::function<void()> take() {
        std::lock_guard<std::mutex> l(_m);
        if (_q.empty()) {
            return {};
        }
        auto fn = std::move(_q.front());
        _q.pop_front();
        return fn;
    }

private:
    std::mutex _m;
    std::deque<std::function<void()>> _q;
};

// Copies of outgoing files are made on a worker thread; the fake group's
// clock doesn't move while one is, so a copy takes no virtual time.
bool WaitForCopies() {
    if (Files::Test::CopiesInFlight() == 0) {
        return false;
    }
    while (Files::Test::CopiesInFlight() != 0) {
        std::this_thread::sleep_for(std::chrono::microseconds(200));
    }
    return true;
}

class FakeGroup;
class FakeSource;

class FakeChannel : public IGroupChannel {
public:
    FakeChannel(FakeGroup& g, std::string id, bool initiator) : _g(g), _id(std::move(id)), _initiator(initiator) {}
    ~FakeChannel() override;
    std::string LocalId() const override { return _id; }
    bool IsInitiator() const override { return _initiator; }
    size_t MaxMessageBytes() const override;
    void Send(const std::string& to, const std::string& bytes) override;
    uint64_t MaxAttachmentBytes() const override;
    bool Attach(const std::string& key, const std::string& path) override;
    void Detach(const std::string& key) override;
    void Leave(bool endForEveryone) override;

private:
    FakeGroup& _g;
    std::string _id;
    bool _initiator;
};

// One call: at most one activity, its members, and the frames between them.
class FakeGroup {
public:
    FakeGroup() : _rng(7) {}

    size_t maxMessage = 1024;
    uint64_t maxAttachment = 0;
    bool attachFails = false;
    bool attachmentsLost = false; // Attach succeeds, but nobody ever gets the file
    bool uploadsFail = false; // Attach takes the file, then reports the upload failed
    uint64_t latency = 10;
    uint64_t jitter = 0; // extra random delay; frames on one link stay in order

    // What went over the channel.
    size_t oversize = 0;
    std::map<GroupWire::Type, size_t> sentByType;
    size_t attaches = 0;
    size_t failedUploads = 0;
    // Each Leave: the member, and whether it ended the activity for everyone.
    std::vector<std::pair<std::string, bool>> leaves;
    // Attachments each member downloaded, and the ones it skipped.
    std::map<std::string, size_t> downloads;
    std::map<std::string, size_t> skippedDownloads;
    struct Delivered {
        std::string from;
        std::string to;
        GroupWire::Type type;
    };
    std::vector<Delivered> delivered;

    // Runs around every callback into `member`'s device.
    std::function<void(const std::string& member, const std::function<void()>& fn)> wrapper;

    uint64_t Now() const { return _now; }
    size_t InFlight() const { return _q.size(); }
    bool Step() {
        if (_q.empty()) {
            return false;
        }
        auto it = _q.begin();
        _now = std::max(_now, it->first.first);
        Ev ev = std::move(it->second);
        _q.erase(it);
        if (wrapper) {
            wrapper(ev.target, ev.fn);
        } else {
            ev.fn();
        }
        return true;
    }
    void RunUntil(uint64_t t) {
        while (!_q.empty() && _q.begin()->first.first <= t) {
            Step();
        }
        _now = std::max(_now, t);
    }

    // ---- the platform side ----
    void AddSource(FakeSource* s) { _sources.push_back(s); }
    void RemoveSource(FakeSource* s) { _sources.erase(std::remove(_sources.begin(), _sources.end(), s), _sources.end()); }
    void Start(FakeSource& s, const SessionAdvert& a, Ready ready);
    void Join(FakeSource& s, const std::string& id, Ready ready);
    SessionAdvert OfferFor(const std::string& member) const {
        return (_ended || _sessionId.empty() || member == _starter) ? SessionAdvert{} : _offer;
    }
    const std::string& SessionId() const { return _sessionId; }

    // The group ends for everyone (the call hangs up).
    void End(const std::string& reason);
    // `member` disappears without a word.
    void Drop(const std::string& member) {
        _channels.erase(member);
        RemoveMember(member);
    }
    // A member with no transport: the test speaks for it.
    void AddRaw(const std::string& id, const SessionAdvert* startWith = nullptr) {
        if (startWith != nullptr) {
            Begin(id, *startWith);
        }
        _raw.insert(id);
        AddMember(id, id);
    }
    void Inject(const std::string& from, const std::string& to, const std::string& bytes) { Send(from, to, bytes); }
    void InjectAttachment(const std::string& from, const std::string& key, const std::string& content) {
        std::string p = _dir.Write("raw-" + std::to_string(++_files), content);
        _store[key] = p;
        Offer(from, key);
    }
    std::vector<std::pair<std::string, std::string>>& Inbox(const std::string& raw) { return _inbox[raw]; }
    size_t Stored() const { return _store.size(); }

    // ---- the channel side ----
    // Sees every frame as it is sent.
    std::function<void(const std::string& from, const std::string& to, const std::string& bytes)> onSend;

    // True: the frame is lost on the way (counted as sent, never delivered).
    std::function<bool(const std::string& from, const std::string& to, const std::string& bytes)> lose;

    void Send(const std::string& from, const std::string& to, const std::string& bytes) {
        if (onSend) {
            onSend(from, to, bytes);
        }
        if (lose && lose(from, to, bytes)) {
            GroupWire::Type t = bytes.size() >= 2 ? (GroupWire::Type)(uint8_t)bytes[1] : (GroupWire::Type)0;
            ++sentByType[t];
            return;
        }
        if (bytes.size() > maxMessage) {
            ++oversize;
        }
        GroupWire::Type type = bytes.size() >= 2 ? (GroupWire::Type)(uint8_t)bytes[1] : (GroupWire::Type)0;
        ++sentByType[type];
        if (_ended || !IsMember(from)) {
            return;
        }
        std::vector<std::string> targets;
        if (to.empty()) {
            for (const auto& m : _members) {
                if (m.id != from) {
                    targets.push_back(m.id);
                }
            }
        } else if (IsMember(to)) {
            targets.push_back(to);
        }
        for (const auto& t : targets) {
            uint64_t at = _now + latency + (jitter ? _rng() % (jitter + 1) : 0);
            uint64_t& last = _linkLast[{ from, t }];
            at = std::max(at, last);
            last = at;
            if (_raw.count(t)) {
                Queue(at, t, [this, t, from, bytes] { _inbox[t].emplace_back(from, bytes); });
                continue;
            }
            Queue(at, t, [this, t, from, bytes, type] {
                auto it = _channels.find(t);
                if (it == _channels.end()) {
                    return;
                }
                delivered.push_back({ from, t, type });
                if (it->second->onMessage) {
                    it->second->onMessage(from, bytes);
                }
            });
        }
    }
    bool Attach(const std::string& from, const std::string& key, const std::string& path) {
        std::error_code ec;
        uint64_t size = fs::file_size(path, ec);
        if (attachFails || ec || size > maxAttachment) {
            return false;
        }
        if (uploadsFail) {
            ++attaches;
            ++failedUploads;
            Queue(_now + latency, from, [this, from, key] {
                auto it = _channels.find(from);
                if (it != _channels.end() && it->second->onAttachFailed) {
                    it->second->onAttachFailed(key);
                }
            });
            return true;
        }
        std::string copy = _dir.File("att-" + std::to_string(++_files));
        fs::copy_file(path, copy, ec);
        REQUIRE_FALSE(ec);
        _store[key] = copy;
        ++attaches;
        if (!attachmentsLost) {
            Offer(from, key);
        }
        return true;
    }
    void Detach(const std::string& key) {
        auto it = _store.find(key);
        if (it != _store.end()) {
            std::error_code ec;
            fs::remove(it->second, ec);
            _store.erase(it);
        }
    }
    void Leave(const std::string& member) {
        _channels.erase(member);
        RemoveMember(member);
    }
    void Forget(FakeChannel* ch) {
        auto it = _channels.find(ch->LocalId());
        if (it != _channels.end() && it->second == ch) {
            Leave(ch->LocalId());
        }
    }

private:
    struct Ev {
        std::string target;
        std::function<void()> fn;
    };

    void Queue(uint64_t at, const std::string& target, std::function<void()> fn) { _q.emplace(std::make_pair(at, _order++), Ev{ target, std::move(fn) }); }

    bool IsMember(const std::string& id) const {
        return std::any_of(_members.begin(), _members.end(), [&](const GroupMember& m) { return m.id == id; });
    }

    void Begin(const std::string& starter, const SessionAdvert& a) {
        _sessionId = "activity-" + std::to_string(++_activities);
        _starter = starter;
        _offer = a;
        _offer.id = _sessionId;
        for (FakeSource* s : _sources) {
            Tell(s, _offer);
        }
    }
    void Tell(FakeSource* s, const SessionAdvert& a);

    void AddMember(const std::string& id, const std::string& name) {
        _members.push_back({ id, name });
        Announce();
    }
    void RemoveMember(const std::string& id) {
        _members.erase(std::remove_if(_members.begin(), _members.end(), [&](const GroupMember& m) { return m.id == id; }), _members.end());
        Announce();
    }
    void Announce() {
        auto list = _members;
        for (const auto& m : _members) {
            if (_raw.count(m.id)) {
                continue;
            }
            std::string id = m.id;
            Queue(_now + latency, id, [this, id, list] {
                auto it = _channels.find(id);
                if (it != _channels.end() && it->second->onMembers) {
                    it->second->onMembers(list);
                }
            });
        }
    }
    void Offer(const std::string& from, const std::string& key) {
        for (const auto& m : _members) {
            if (m.id == from || _raw.count(m.id)) {
                continue;
            }
            std::string id = m.id;
            Queue(_now + latency, id, [this, id, from, key] {
                auto it = _channels.find(id);
                auto st = _store.find(key);
                if (it == _channels.end() || st == _store.end() || !it->second->onAttachment) {
                    return;
                }
                if (it->second->wantsAttachment && !it->second->wantsAttachment(key)) {
                    ++skippedDownloads[id];
                    return;
                }
                // Each member downloads its own copy, valid during the call.
                ++downloads[id];
                std::string got = _dir.File("got-" + std::to_string(++_files));
                std::error_code ec;
                fs::copy_file(st->second, got, ec);
                REQUIRE_FALSE(ec);
                it->second->onAttachment(from, key, got);
                fs::remove(got, ec);
            });
        }
    }

    friend class FakeSource;
    TempDir _dir;
    std::mt19937 _rng;
    uint64_t _now = 0;
    uint64_t _order = 0;
    std::map<std::pair<uint64_t, uint64_t>, Ev> _q;
    std::map<std::pair<std::string, std::string>, uint64_t> _linkLast;
    std::vector<FakeSource*> _sources;
    std::string _sessionId;
    std::string _starter;
    SessionAdvert _offer;
    bool _ended = false;
    int _activities = 0;
    std::vector<GroupMember> _members;
    std::map<std::string, FakeChannel*> _channels;
    std::set<std::string> _raw;
    std::map<std::string, std::vector<std::pair<std::string, std::string>>> _inbox;
    std::map<std::string, std::string> _store;
    uint64_t _files = 0;
};

FakeChannel::~FakeChannel() {
    _g.Forget(this);
}
size_t FakeChannel::MaxMessageBytes() const {
    return _g.maxMessage;
}
void FakeChannel::Send(const std::string& to, const std::string& bytes) {
    _g.Send(_id, to, bytes);
}
uint64_t FakeChannel::MaxAttachmentBytes() const {
    return _g.maxAttachment;
}
bool FakeChannel::Attach(const std::string& key, const std::string& path) {
    return _g.Attach(_id, key, path);
}
void FakeChannel::Detach(const std::string& key) {
    _g.Detach(key);
}
void FakeChannel::Leave(bool endForEveryone) {
    _g.leaves.emplace_back(_id, endForEveryone);
    _g.Forget(this);
}

class FakeSource : public IGroupSessionSource {
public:
    FakeSource(FakeGroup& g, std::string id, std::string name) : g(g), id(std::move(id)), name(std::move(name)) { g.AddSource(this); }
    ~FakeSource() override { g.RemoveSource(this); }

    bool Available() const override { return available; }
    bool InGroup() const override { return inCall; }
    void Start(const SessionAdvert& a, Ready ready) override {
        if (failStart) {
            ready(nullptr, "The activity could not start.");
            return;
        }
        g.Start(*this, a, std::move(ready));
    }
    void JoinOffered(const std::string& advertId, Ready ready) override { g.Join(*this, advertId, std::move(ready)); }
    SessionAdvert OfferedSession() const override { return g.OfferFor(id); }

    FakeGroup& g;
    std::string id;
    std::string name;
    bool available = true;
    bool inCall = true;
    bool failStart = false;
};

void FakeGroup::Tell(FakeSource* s, const SessionAdvert& a) {
    if (s->id == _starter) {
        return;
    }
    Queue(_now + latency, s->id, [this, s, a] {
        if (std::find(_sources.begin(), _sources.end(), s) != _sources.end() && s->onSessionOffered) {
            s->onSessionOffered(a);
        }
    });
}

void FakeGroup::Start(FakeSource& s, const SessionAdvert& a, Ready ready) {
    FakeSource* sp = &s;
    Queue(_now + latency, s.id, [this, sp, a, ready] {
        if (_ended) {
            ready(nullptr, "The call has ended.");
            return;
        }
        if (_sessionId.empty()) {
            Begin(sp->id, a);
        }
        auto ch = std::make_unique<FakeChannel>(*this, sp->id, true);
        _channels[sp->id] = ch.get();
        AddMember(sp->id, sp->name);
        ready(std::move(ch), "");
    });
}

void FakeGroup::Join(FakeSource& s, const std::string& id, Ready ready) {
    FakeSource* sp = &s;
    Queue(_now + latency, s.id, [this, sp, id, ready] {
        if (_ended || id != _sessionId) {
            ready(nullptr, "That session has ended.");
            return;
        }
        auto ch = std::make_unique<FakeChannel>(*this, sp->id, false);
        _channels[sp->id] = ch.get();
        AddMember(sp->id, sp->name);
        ready(std::move(ch), "");
    });
}

void FakeGroup::End(const std::string& reason) {
    _ended = true;
    for (const auto& [id, ch] : _channels) {
        std::string m = id;
        Queue(_now + latency, m, [this, m, reason] {
            auto it = _channels.find(m);
            if (it != _channels.end() && it->second->onEnded) {
                it->second->onEnded(reason);
            }
        });
    }
    for (FakeSource* s : _sources) {
        Tell(s, SessionAdvert{});
    }
}

// ---------------------------------------------------------------------------
// Transports alone
// ---------------------------------------------------------------------------

struct Dev {
    std::string id;
    std::unique_ptr<FakeSource> src;
    PostQueue posted;
    std::unique_ptr<INetTransport> t;

    std::vector<std::pair<PeerId, PeerEvent>> events;
    std::vector<std::pair<PeerId, Message>> messages;
    struct Blob {
        PeerId from;
        BlobId id;
        std::string content;
    };
    std::vector<Blob> blobs;
    std::vector<std::pair<NetState, std::string>> states;
    bool closed = false;
    int afterClose = 0;

    Dispatcher Dispatch() {
        return [this](std::function<void()> fn) { posted.push_back(std::move(fn)); };
    }
    void Close() {
        t->Close();
        closed = true;
    }
    bool Has(PeerId p, PeerEvent e) const { return std::find(events.begin(), events.end(), std::make_pair(p, e)) != events.end(); }
    std::vector<NetState> StateList() const {
        std::vector<NetState> out;
        for (auto& [s, why] : states) {
            out.push_back(s);
        }
        return out;
    }
};

class Rig {
public:
    Rig() = default;
    ~Rig() {
        for (auto& d : devs) {
            d->t.reset();
        }
    }

    Dev& Add(const std::string& id) {
        auto d = std::make_unique<Dev>();
        Dev* dp = d.get();
        dp->id = id;
        dp->src = std::make_unique<FakeSource>(g, id, "Name " + id);
        dp->t = CreateGroupTransport(*dp->src, dp->Dispatch());
        REQUIRE(dp->t);
        dp->t->onPeer = [dp](PeerId p, PeerEvent e) {
            dp->afterClose += dp->closed;
            dp->events.emplace_back(p, e);
        };
        dp->t->onMessage = [dp](PeerId p, const Message& m) {
            dp->afterClose += dp->closed;
            dp->messages.emplace_back(p, m);
        };
        dp->t->onBlob = [dp](PeerId p, BlobId id, const std::string& path) {
            dp->afterClose += dp->closed;
            dp->blobs.push_back({ p, id, ReadFile(path) });
        };
        dp->t->onState = [dp](NetState s, const std::string& why) {
            dp->afterClose += dp->closed;
            dp->states.emplace_back(s, why);
        };
        devs.push_back(std::move(d));
        return *dp;
    }

    void Host(Dev& d) {
        SessionAdvert a;
        a.sequenceName = "song.xsq";
        a.hostName = "Host " + d.id;
        a.deviceKind = "mac";
        std::string error;
        REQUIRE(d.t->Listen("", a, error));
        CHECK(error.empty());
        CHECK(d.t->LocalPeer() == kLanHostPeer);
    }
    void Join(Dev& d) { d.t->Connect(g.SessionId(), ""); }

    bool Pump() {
        bool any = false;
        for (bool more = true; more;) {
            more = false;
            for (auto& d : devs) {
                while (auto fn = d->posted.take()) {
                    fn();
                    more = any = true;
                }
            }
            if (WaitForCopies()) {
                more = true;
            }
        }
        return any;
    }
    // Until nothing is in flight or posted.
    void Settle() {
        for (size_t n = 0; n < 10000000; ++n) {
            Pump();
            if (!g.Step()) {
                if (!Pump()) {
                    return;
                }
            }
        }
        FAIL("the fake group never went quiet");
    }

    FakeGroup g; // destroyed after the devices
    std::vector<std::unique_ptr<Dev>> devs;
};

Message Ping(uint64_t n) {
    Message m;
    m.type = MsgType::Ping;
    m.time = n;
    return m;
}

// A message whose encoding is exactly `bytes` long.
Message Sized(size_t bytes, char fill = 'a') {
    Message m;
    m.type = MsgType::Notice;
    m.reason = "pad";
    size_t base = EncodeMessage(m).size();
    REQUIRE(bytes >= base);
    m.text.assign(bytes - base, fill);
    REQUIRE(EncodeMessage(m).size() == bytes);
    return m;
}

bool Same(const Message& a, const Message& b) {
    return EncodeMessage(a) == EncodeMessage(b);
}

std::string RandomBytes(size_t n, uint32_t seed) {
    std::mt19937 r(seed);
    std::string s(n, '\0');
    for (auto& ch : s) {
        ch = (char)(r() & 0xFF);
    }
    return s;
}

std::string U32(uint32_t v) {
    std::string s;
    GroupWire::PutU32(s, v);
    return s;
}
std::string U64(uint64_t v) {
    std::string s;
    GroupWire::PutU64(s, v);
    return s;
}
std::string F(GroupWire::Type t, const std::string& payload = "") {
    return GroupWire::Frame(t, payload);
}

} // namespace

TEST_CASE("Group sessions: the initiator hosts and clients get peer ids in join order", "[collab][group]") {
    Rig r;
    Dev& h = r.Add("m-1");
    Dev& a = r.Add("m-2");
    Dev& b = r.Add("m-3");
    r.Host(h);
    r.Settle();
    CHECK(h.StateList() == std::vector<NetState>{ NetState::Connecting, NetState::Listening });
    CHECK(h.t->State() == NetState::Listening);
    r.Join(a);
    r.Settle();
    r.Join(b);
    r.Settle();
    CHECK(a.StateList() == std::vector<NetState>{ NetState::Connecting, NetState::Connected });
    CHECK(a.t->LocalPeer() == 2);
    CHECK(b.t->LocalPeer() == 3);
    CHECK(h.events == std::vector<std::pair<PeerId, PeerEvent>>{ { 2, PeerEvent::Joined }, { 3, PeerEvent::Joined } });
    CHECK(a.events == std::vector<std::pair<PeerId, PeerEvent>>{ { kLanHostPeer, PeerEvent::Joined } });
    CHECK(b.events == std::vector<std::pair<PeerId, PeerEvent>>{ { kLanHostPeer, PeerEvent::Joined } });

    // Ordered per sender, addressed per peer; a client's broadcast reaches
    // only the host.
    for (uint64_t i = 0; i < 40; ++i) {
        h.t->Send(i % 2 ? 2 : 3, Ping(i));
        a.t->Send(kLanHostPeer, Ping(100 + i));
    }
    h.t->Broadcast(Ping(999));
    b.t->Broadcast(Ping(555));
    r.Settle();
    std::vector<uint64_t> toA, fromA;
    for (auto& [p, m] : a.messages) {
        CHECK(p == kLanHostPeer);
        toA.push_back(m.time);
    }
    for (auto& [p, m] : h.messages) {
        if (p == 2) {
            fromA.push_back(m.time);
        }
    }
    std::vector<uint64_t> wantA, wantFromA;
    for (uint64_t i = 1; i < 40; i += 2) {
        wantA.push_back(i);
    }
    wantA.push_back(999);
    for (uint64_t i = 0; i < 40; ++i) {
        wantFromA.push_back(100 + i);
    }
    CHECK(toA == wantA);
    CHECK(fromA == wantFromA);
    REQUIRE(h.messages.size() == 41);
    CHECK(h.messages.back().first == 3);
    CHECK(Same(h.messages.back().second, Ping(555)));
    CHECK(b.messages.size() == 21);
    CHECK(r.g.oversize == 0);
    CHECK(GroupTest::MalformedFrames(*h.t) == 0);
    CHECK(GroupTest::MalformedFrames(*a.t) == 0);
}

TEST_CASE("Group sessions: two claimants resolve to the lower member id", "[collab][group]") {
    // Two devices host in one call: the higher id gives up.
    for (int order = 0; order < 2; ++order) {
        INFO("order " << order);
        Rig r;
        r.g.jitter = 5;
        Dev& hi = r.Add("m-b");
        Dev& lo = r.Add("m-a");
        Dev& c = r.Add("m-c");
        r.Host(order == 0 ? hi : lo);
        r.Pump();
        r.g.Step();
        r.Pump();
        r.Host(order == 0 ? lo : hi);
        r.Settle();
        CHECK(lo.t->State() == NetState::Listening);
        CHECK(hi.t->State() == NetState::Failed);
        REQUIRE_FALSE(hi.states.empty());
        CHECK(hi.states.back().second.find("already hosting") != std::string::npos);
        r.Join(c);
        r.Settle();
        CHECK(c.t->State() == NetState::Connected);
        CHECK(lo.Has(c.t->LocalPeer(), PeerEvent::Joined));
        c.t->Send(kLanHostPeer, Ping(7));
        lo.t->Broadcast(Ping(8));
        r.Settle();
        REQUIRE(lo.messages.size() == 1);
        CHECK(lo.messages[0].second.time == 7);
        CHECK(hi.messages.empty());
        CHECK(hi.events.empty());
        REQUIRE(c.messages.size() == 1);
        CHECK(c.messages[0].second.time == 8);
    }
    // A client that hears both claims before a welcome goes with the lower
    // one, whichever answers first.
    for (int order = 0; order < 2; ++order) {
        INFO("welcome order " << order);
        Rig r;
        SessionAdvert ad;
        r.g.AddRaw("m-b", &ad);
        r.g.AddRaw("m-a");
        Dev& c = r.Add("m-c");
        r.Join(c);
        r.Settle();
        r.g.Inject("m-b", "m-c", F(GroupWire::Type::Claim, U32(kProtocolVersion)));
        r.g.Inject("m-a", "m-c", F(GroupWire::Type::Claim, U32(kProtocolVersion)));
        r.Settle();
        CHECK(r.g.Inbox("m-a").back().second == F(GroupWire::Type::Hello));
        if (order == 0) {
            r.g.Inject("m-b", "m-c", F(GroupWire::Type::Welcome, U32(4)));
            r.g.Inject("m-a", "m-c", F(GroupWire::Type::Welcome, U32(5)));
        } else {
            r.g.Inject("m-a", "m-c", F(GroupWire::Type::Welcome, U32(5)));
            r.g.Inject("m-b", "m-c", F(GroupWire::Type::Welcome, U32(4)));
        }
        r.Settle();
        CHECK(c.t->State() == NetState::Connected);
        CHECK(c.t->LocalPeer() == 5);
        CHECK(c.events == std::vector<std::pair<PeerId, PeerEvent>>{ { kLanHostPeer, PeerEvent::Joined } });
        c.t->Send(kLanHostPeer, Ping(3));
        r.Settle();
        CHECK(r.g.Inbox("m-a").back().second == F(GroupWire::Type::Msg, EncodeMessage(Ping(3))));
        CHECK(r.g.Inbox("m-b").back().second != F(GroupWire::Type::Msg, EncodeMessage(Ping(3))));
    }
}

TEST_CASE("Group sessions: a client with no host yet waits for one", "[collab][group]") {
    Rig r;
    SessionAdvert ad;
    ad.hostName = "silent";
    r.g.AddRaw("m-0", &ad);
    Dev& c = r.Add("m-5");
    r.Join(c);
    r.Settle();
    CHECK(c.t->State() == NetState::Connecting);
    CHECK(c.events.empty());
    auto& inbox = r.g.Inbox("m-0");
    REQUIRE(inbox.size() == 1);
    CHECK(inbox[0].second == F(GroupWire::Type::Hello));
    // A welcome must come from a claimant the client asked.
    r.g.Inject("m-0", "m-5", F(GroupWire::Type::Claim, U32(kProtocolVersion)));
    r.Settle();
    CHECK(inbox.size() == 2); // asked again, this time directly
    r.g.Inject("m-0", "m-5", F(GroupWire::Type::Welcome, U32(7)));
    r.Settle();
    CHECK(c.t->State() == NetState::Connected);
    CHECK(c.t->LocalPeer() == 7);
    CHECK(c.events == std::vector<std::pair<PeerId, PeerEvent>>{ { kLanHostPeer, PeerEvent::Joined } });
}

TEST_CASE("Group sessions: messages above the size limit are chunked and reassembled", "[collab][group]") {
    Rig r;
    r.g.maxMessage = 300;
    r.g.jitter = 8;
    Dev& h = r.Add("m-1");
    Dev& a = r.Add("m-2");
    Dev& b = r.Add("m-3");
    r.Host(h);
    r.Settle();
    r.Join(a);
    r.Join(b);
    r.Settle();
    REQUIRE(h.events.size() == 2);

    // Exactly at the limit goes whole; one byte more is split.
    a.t->Send(kLanHostPeer, Sized(300 - GroupWire::kHeaderBytes, 'x'));
    r.Settle();
    CHECK(r.g.sentByType[GroupWire::Type::MsgPart] == 0);
    a.t->Send(kLanHostPeer, Sized(300 - GroupWire::kHeaderBytes + 1, 'y'));
    r.Settle();
    CHECK(r.g.sentByType[GroupWire::Type::MsgPart] == 2);
    REQUIRE(h.messages.size() == 2);
    CHECK(Same(h.messages[0].second, Sized(298, 'x')));
    CHECK(Same(h.messages[1].second, Sized(299, 'y')));

    // Large messages from two clients at once arrive interleaved and whole.
    h.messages.clear();
    r.g.delivered.clear();
    for (int i = 0; i < 4; ++i) {
        a.t->Send(kLanHostPeer, Sized(20000 + i, (char)('a' + i)));
        b.t->Send(kLanHostPeer, Sized(30000 + i, (char)('k' + i)));
        a.t->Send(kLanHostPeer, Ping(i));
    }
    h.t->Broadcast(Sized(9000, 'z'));
    r.Settle();
    std::vector<Message> fromA, fromB;
    for (auto& [p, m] : h.messages) {
        (p == 2 ? fromA : fromB).push_back(m);
    }
    REQUIRE(fromA.size() == 8);
    REQUIRE(fromB.size() == 4);
    for (int i = 0; i < 4; ++i) {
        CHECK(Same(fromA[2 * i], Sized(20000 + i, (char)('a' + i))));
        CHECK(Same(fromA[2 * i + 1], Ping(i)));
        CHECK(Same(fromB[i], Sized(30000 + i, (char)('k' + i))));
    }
    REQUIRE(a.messages.size() == 1);
    CHECK(Same(a.messages[0].second, Sized(9000, 'z')));
    REQUIRE(b.messages.size() == 1);
    CHECK(Same(b.messages[0].second, Sized(9000, 'z')));
    // The host really saw the two senders' parts mixed.
    int switches = 0;
    std::string last;
    for (const auto& d : r.g.delivered) {
        if (d.to == "m-1" && d.type == GroupWire::Type::MsgPart) {
            switches += !last.empty() && last != d.from;
            last = d.from;
        }
    }
    CHECK(switches > 4);
    CHECK(r.g.oversize == 0);
    CHECK(GroupTest::MalformedFrames(*h.t) == 0);
}

namespace {

struct BlobCase {
    size_t size;
    bool attached; // expected to go through the store
};

// Sends each case from a to the host, deleting the source file at once.
void SendBlobs(Rig& r, Dev& from, Dev& to, PeerId toPeer, const std::vector<BlobCase>& cases, TempDir& dir) {
    to.blobs.clear();
    size_t attachesBefore = r.g.attaches;
    size_t startsBefore = r.g.sentByType[GroupWire::Type::BlobStart];
    std::vector<std::string> contents;
    size_t expectAttached = 0;
    for (size_t i = 0; i < cases.size(); ++i) {
        contents.push_back(RandomBytes(cases[i].size, (uint32_t)i + 1));
        std::string p = dir.Write("blob" + std::to_string(i), contents.back());
        from.t->SendBlob(toPeer, 500 + i, p);
        // done with the path on return
        std::error_code ec;
        fs::remove(p, ec);
        REQUIRE_FALSE(ec);
        expectAttached += cases[i].attached;
    }
    r.Settle();
    REQUIRE(to.blobs.size() == cases.size());
    std::map<BlobId, std::string> got;
    for (auto& b : to.blobs) {
        CHECK(b.from == (toPeer == kLanHostPeer ? from.t->LocalPeer() : kLanHostPeer));
        got[b.id] = b.content;
    }
    for (size_t i = 0; i < cases.size(); ++i) {
        INFO("blob " << i << " of " << cases[i].size << " bytes");
        REQUIRE(got.count(500 + i));
        CHECK(got[500 + i].size() == contents[i].size());
        CHECK(got[500 + i] == contents[i]);
    }
    CHECK(r.g.attaches - attachesBefore == expectAttached);
    CHECK(r.g.sentByType[GroupWire::Type::BlobStart] - startsBefore == cases.size() - expectAttached);
    // every attachment was removed once it had arrived
    CHECK(r.g.Stored() == 0);
}

} // namespace

TEST_CASE("Group sessions: files go through the attachment store when they fit", "[collab][group]") {
    Rig r;
    r.g.maxMessage = 4096;
    r.g.maxAttachment = 1000;
    TempDir dir;
    Dev& h = r.Add("m-1");
    Dev& a = r.Add("m-2");
    Dev& b = r.Add("m-3");
    r.Host(h);
    r.Settle();
    r.Join(a);
    r.Join(b);
    r.Settle();
    SendBlobs(r, a, h, kLanHostPeer, { { 0, true }, { 1, true }, { 1000, true }, { 1001, false }, { 50000, false } }, dir);
    SendBlobs(r, h, a, 2, { { 0, true }, { 999, true }, { 1001, false } }, dir);
    // the store reaches everyone, but only the recipient takes it
    CHECK(b.blobs.empty());
    CHECK(GroupTest::MalformedFrames(*b.t) == 0);

    r.g.maxAttachment = 8u << 20;
    SendBlobs(r, a, h, kLanHostPeer, { { 3u << 20, true }, { 5, true } }, dir);
    // A store that refuses falls back to chunks.
    r.g.attachFails = true;
    SendBlobs(r, a, h, kLanHostPeer, { { 0, false }, { 1, false }, { 700, false }, { 2u << 20, false } }, dir);
    CHECK(r.g.oversize == 0);
    CHECK(GroupTest::MalformedFrames(*h.t) == 0);
    CHECK(GroupTest::MalformedFrames(*a.t) == 0);
}

TEST_CASE("Group sessions: a file whose upload fails is sent again in chunks", "[collab][group]") {
    Rig r;
    r.g.maxMessage = 4096;
    r.g.maxAttachment = 8u << 20;
    r.g.uploadsFail = true;
    TempDir dir;
    Dev& h = r.Add("m-1");
    Dev& a = r.Add("m-2");
    r.Host(h);
    r.Settle();
    r.Join(a);
    r.Settle();
    std::vector<size_t> sizes{ 0, 1, 4000, 2u << 20 };
    std::vector<std::string> contents;
    for (size_t i = 0; i < sizes.size(); ++i) {
        contents.push_back(RandomBytes(sizes[i], 40 + (uint32_t)i));
        std::string p = dir.Write("up" + std::to_string(i), contents.back());
        a.t->SendBlob(kLanHostPeer, 900 + i, p);
        h.t->SendBlob(2, 950 + i, p);
        // done with the path on return, even though the upload fails later
        std::error_code ec;
        fs::remove(p, ec);
        REQUIRE_FALSE(ec);
    }
    r.Settle();
    CHECK(r.g.failedUploads == 2 * sizes.size());
    CHECK(r.g.sentByType[GroupWire::Type::BlobStart] == 2 * sizes.size());
    REQUIRE(h.blobs.size() == sizes.size());
    REQUIRE(a.blobs.size() == sizes.size());
    for (size_t i = 0; i < sizes.size(); ++i) {
        INFO("size " << sizes[i]);
        CHECK(h.blobs[i].id == 900 + i);
        CHECK(h.blobs[i].content == contents[i]);
        CHECK(a.blobs[i].id == 950 + i);
        CHECK(a.blobs[i].content == contents[i]);
    }
    CHECK(r.g.Stored() == 0);
    CHECK(r.g.oversize == 0);
}

TEST_CASE("Group sessions: without a store files go in chunks that don't hold up messages", "[collab][group]") {
    Rig r;
    r.g.maxMessage = 16384;
    TempDir dir;
    Dev& h = r.Add("m-1");
    Dev& a = r.Add("m-2");
    r.Host(h);
    r.Settle();
    r.Join(a);
    r.Settle();
    size_t chunk = 16384 - GroupWire::kDataHeaderBytes;
    SendBlobs(r, h, a, 2, { { 0, false }, { 1, false }, { chunk, false }, { chunk + 1, false }, { 3u << 20, false } }, dir);
    SendBlobs(r, a, h, kLanHostPeer, { { 4u << 20, false }, { 17, false } }, dir);

    // A message sent right after a big file waits behind one window of it,
    // not the whole file.
    std::string p = dir.Write("big", RandomBytes(4u << 20, 99));
    r.g.delivered.clear();
    a.messages.clear();
    h.t->SendBlob(2, 77, p);
    h.t->Send(2, Ping(1));
    r.Settle();
    size_t dataBefore = 0, dataTotal = 0;
    bool seen = false;
    for (const auto& d : r.g.delivered) {
        if (d.to != "m-2") {
            continue;
        }
        if (d.type == GroupWire::Type::Msg) {
            seen = true;
        } else if (d.type == GroupWire::Type::BlobData) {
            ++dataTotal;
            dataBefore += !seen;
        }
    }
    CHECK(seen);
    CHECK(dataTotal > 200);
    CHECK(dataBefore <= GroupWire::kBlobWindow / chunk + 2);
    REQUIRE(a.messages.size() == 1);
    CHECK(a.blobs.back().content.size() == (4u << 20));
    CHECK(r.g.oversize == 0);
}

TEST_CASE("Group sessions: malformed or out-of-place frames are dropped", "[collab][group]") {
    using T = GroupWire::Type;
    Rig r;
    r.g.maxMessage = 512;
    Dev& h = r.Add("m-1");
    r.Host(h);
    r.Settle();
    // A rogue member that gets welcomed like any client, and one that never asks.
    r.g.AddRaw("m-7");
    r.g.AddRaw("m-8");
    r.g.Inject("m-7", "m-1", F(T::Hello));
    r.Settle();
    REQUIRE(h.events == std::vector<std::pair<PeerId, PeerEvent>>{ { 2, PeerEvent::Joined } });
    std::string good = EncodeMessage(Ping(42));
    std::string id9 = U64(9);
    std::vector<std::pair<std::string, std::string>> bad = {
        { "m-7", "" },
        { "m-7", "\x01" },
        { "m-7", std::string(1, (char)(GroupWire::kVersion + 1)) + "\x05" + good }, // another version
        { "m-7", F((T)99) },
        { "m-7", F(T::Welcome, U32(5)) },
        { "m-7", F(T::Msg, "{not a message") },
        { "m-7", F(T::MsgPart, U64(1) + U32(0) + U32(1) + "x") },          // one part
        { "m-7", F(T::MsgPart, U64(2) + U32(3) + U32(3) + "x") },          // index past count
        { "m-7", F(T::MsgPart, U64(3) + U32(1) + U32(3) + "x") },          // no start
        { "m-7", F(T::MsgPart, U64(4) + U32(0) + U32(3) + good.substr(0, 3)) },
        { "m-7", F(T::MsgPart, U64(4) + U32(2) + U32(3) + good.substr(3)) }, // skipped one
        { "m-7", F(T::MsgPart, U64(5) + U32(0)) },                          // short header
        { "m-7", F(T::BlobData, id9 + "abc") },                             // no start
        { "m-7", F(T::BlobStart, id9 + U64(Wire::kMaxBlobBytes + 1)) },
        { "m-7", F(T::BlobStart, id9 + U64(4)) },
        { "m-7", F(T::BlobData, id9 + "abcdefgh") },                        // over its size
        { "m-7", F(T::BlobStart, id9 + U64(4)) },
        { "m-7", F(T::BlobData, id9 + "ab") },
        { "m-7", F(T::BlobEnd, id9) },                                      // short
        { "m-7", F(T::BlobAck, U32(1)) },
        { "m-7", F(T::AttachAck, "xlb1:1:1:m-7") },                         // never attached
        { "m-8", F(T::Msg, good) },                                         // not in the session
        { "m-8", F(T::BlobStart, id9 + U64(1)) },
    };
    for (auto& [from, bytes] : bad) {
        r.g.Inject(from, "m-1", bytes);
    }
    r.g.InjectAttachment("m-7", "junk", "x");
    r.g.InjectAttachment("m-8", "xlb1:3:1:m-1", "x");
    r.Settle();
    CHECK(h.messages.empty());
    CHECK(h.blobs.empty());
    // All but the four that begin a part or a blob (and are then cut off),
    // plus the attachment from outside the session. The one with a
    // malformed key isn't even downloaded.
    CHECK(GroupTest::MalformedFrames(*h.t) == bad.size() - 4 + 1);
    CHECK(r.g.skippedDownloads["m-1"] == 1);
    // and the member still works
    r.g.Inject("m-7", "m-1", F(T::Msg, good));
    r.g.Inject("m-7", "m-1", F(T::BlobStart, U64(10) + U64(3)));
    r.g.Inject("m-7", "m-1", F(T::BlobData, U64(10) + "xyz"));
    r.g.Inject("m-7", "m-1", F(T::BlobEnd, U64(10)));
    r.Settle();
    REQUIRE(h.messages.size() == 1);
    CHECK(h.messages[0].first == 2);
    CHECK(Same(h.messages[0].second, Ping(42)));
    REQUIRE(h.blobs.size() == 1);
    CHECK(h.blobs[0].content == "xyz");
    CHECK(h.t->State() == NetState::Listening);
    CHECK(h.events.size() == 1);

    // A client takes nothing from a host it didn't choose, and no bad welcome.
    Dev& c = r.Add("m-9");
    r.Join(c);
    r.Settle();
    REQUIRE(c.t->State() == NetState::Connected);
    size_t before = GroupTest::MalformedFrames(*c.t);
    r.g.Inject("m-7", "m-9", F(T::Msg, good));
    r.g.Inject("m-7", "m-9", F(T::Welcome, U32(1)));
    r.g.Inject("m-7", "m-9", F(T::Welcome, std::string("\0\0\0", 3)));
    r.Settle();
    CHECK(c.messages.empty());
    CHECK(GroupTest::MalformedFrames(*c.t) == before + 3);
    CHECK(c.events.size() == 1);
}

TEST_CASE("Group sessions: members leaving and the host going", "[collab][group]") {
    Rig r;
    Dev& h = r.Add("m-1");
    Dev& a = r.Add("m-2");
    Dev& b = r.Add("m-3");
    Dev& c = r.Add("m-4");
    r.Host(h);
    r.Settle();
    r.Join(a);
    r.Join(b);
    r.Join(c);
    r.Settle();
    REQUIRE(h.events.size() == 3);
    a.Close();
    r.Settle();
    CHECK(h.Has(2, PeerEvent::Left));
    CHECK(a.afterClose == 0);
    // gone without a word
    r.g.Drop("m-3");
    r.Settle();
    CHECK(h.Has(3, PeerEvent::Left));
    CHECK(h.events.size() == 5);
    h.t->Send(3, Ping(1)); // to someone gone: nothing happens
    // The host ends it: the client is told it ended.
    h.Close();
    r.Settle();
    CHECK(c.Has(kLanHostPeer, PeerEvent::Left));
    CHECK(c.t->State() == NetState::Failed);
    CHECK(c.states.back().second == "The host ended the session.");
    CHECK(h.afterClose == 0);
}

TEST_CASE("Group sessions: a lost host and an ended group", "[collab][group]") {
    {
        Rig r;
        Dev& h = r.Add("m-1");
        Dev& a = r.Add("m-2");
        r.Host(h);
        r.Settle();
        r.Join(a);
        r.Settle();
        r.g.Drop("m-1");
        r.Settle();
        CHECK(a.Has(kLanHostPeer, PeerEvent::Lost));
        CHECK(a.t->State() == NetState::Failed);
        CHECK(a.states.back().second == "The connection to the host was lost.");
    }
    for (std::string reason : { std::string("The call ended."), std::string() }) {
        INFO("reason '" << reason << "'");
        Rig r;
        Dev& h = r.Add("m-1");
        Dev& a = r.Add("m-2");
        Dev& b = r.Add("m-3");
        r.Host(h);
        r.Settle();
        r.Join(a);
        r.Join(b);
        r.Settle();
        r.g.End(reason);
        r.Settle();
        NetState want = reason.empty() ? NetState::Closed : NetState::Failed;
        CHECK(h.Has(2, PeerEvent::Lost));
        CHECK(h.Has(3, PeerEvent::Lost));
        CHECK(h.t->State() == want);
        CHECK(h.states.back().first == want);
        CHECK(a.Has(kLanHostPeer, PeerEvent::Lost));
        CHECK(a.t->State() == want);
        CHECK(a.states.back().second.find("lost") != std::string::npos);
        // nothing goes out once it's over
        size_t sent = r.g.sentByType[GroupWire::Type::Msg];
        h.t->Broadcast(Ping(1));
        a.t->Send(kLanHostPeer, Ping(2));
        CHECK(r.g.sentByType[GroupWire::Type::Msg] == sent);
    }
}

TEST_CASE("Group sessions: starting or joining can fail", "[collab][group]") {
    Rig r;
    Dev& h = r.Add("m-1");
    h.src->failStart = true;
    SessionAdvert ad;
    std::string error;
    CHECK_FALSE(h.t->Listen("", ad, error));
    CHECK(error == "The activity could not start.");
    CHECK(h.t->State() == NetState::Failed);
    Dev& a = r.Add("m-2");
    a.t->Connect("no-such-activity", "");
    r.Settle();
    CHECK(a.StateList() == std::vector<NetState>{ NetState::Connecting, NetState::Failed });
    CHECK(a.states.back().second == "That session has ended.");
    // a closed transport ignores a channel that arrives late
    Dev& b = r.Add("m-3");
    r.Host(b);
    b.Close();
    r.Settle();
    CHECK(b.afterClose == 0);
    CHECK(r.g.SessionId() != "");
    Dev& c = r.Add("m-4");
    r.Join(c);
    r.Settle();
    CHECK(c.t->State() == NetState::Connecting);
}

TEST_CASE("Group sessions: the browser lists the offered session", "[collab][group]") {
    Rig r;
    Dev& h = r.Add("m-1");
    Dev& a = r.Add("m-2");
    std::vector<std::vector<SessionAdvert>> seen;
    auto br = CreateGroupBrowser(*a.src, a.Dispatch());
    br->onChanged = [&](const std::vector<SessionAdvert>& l) { seen.push_back(l); };
    br->Start();
    r.Settle();
    REQUIRE(seen.size() == 1);
    CHECK(seen[0].empty());
    r.Host(h);
    r.Settle();
    REQUIRE(seen.size() == 2);
    REQUIRE(seen[1].size() == 1);
    CHECK(seen[1][0].id == r.g.SessionId());
    CHECK(seen[1][0].hostName == "Host m-1");
    CHECK(seen[1][0].sequenceName == "song.xsq");
    CHECK(seen[1][0].group);
    // one started later sees it at once
    std::vector<SessionAdvert> late;
    auto br2 = CreateGroupBrowser(*a.src, a.Dispatch());
    br2->onChanged = [&](const std::vector<SessionAdvert>& l) { late = l; };
    br2->Start();
    r.Pump();
    REQUIRE(late.size() == 1);
    CHECK(late[0].id == r.g.SessionId());
    br2->Stop();
    br->Start();
    r.g.End("");
    r.Settle();
    CHECK(seen.back().empty());
    size_t n = seen.size();
    br->Stop();
    br.reset();
    a.src->onSessionOffered(SessionAdvert{ "x" });
    r.Settle();
    CHECK(seen.size() == n);
}

#ifndef __APPLE__
TEST_CASE("Group sessions: none on this platform", "[collab][group]") {
    CHECK_FALSE(PlatformGroupSessions().Available());
    std::deque<std::function<void()>> q;
    auto t = CreateGroupTransport(PlatformGroupSessions(), [&](std::function<void()> fn) { q.push_back(std::move(fn)); });
    SessionAdvert ad;
    std::string error;
    CHECK_FALSE(t->Listen("", ad, error));
    CHECK_FALSE(error.empty());
}
#endif

// ---------------------------------------------------------------------------
// The controller over a group
// ---------------------------------------------------------------------------

namespace {

pugi::xml_document ParseXml(const std::string& text) {
    pugi::xml_document d;
    d.load_string(text.c_str());
    return d;
}

std::string SaveXml(const pugi::xml_document& d) {
    std::ostringstream os;
    d.save(os, "  ");
    return os.str();
}

std::string SerializeRgb(xLightsShowContext& ctx) {
    pugi::xml_document doc;
    doc.append_child(pugi::node_declaration).append_attribute("version") = "1.0";
    pugi::xml_node root = doc.append_child("xrgb");
    XmlSerializingVisitor visitor{ root };
    XmlSerializer::SerializeAllModels(ctx.AllModels, visitor);
    XmlSerializer::SerializeAllObjects(ctx.AllObjects, visitor);
    ctx._sequenceViewManager.Save(visitor);
    XmlSerializer::SerializeAllModelSets(ctx.AllModels.GetSetManager(), visitor);
    return SaveXml(doc);
}

class GroupContext : public HeadlessRenderContext {
public:
    bool IsSequenceLoaded() const override { return sequenceOpen; }
    bool sequenceOpen = false;
};

// One device's app with live layout, its layout the show context's models.
class GroupApp : public ICollabApp {
public:
    GroupApp(const std::string& show, const std::string& file) : _show(show), _file(file) {}

    void Open(const std::string& xsq) { REQUIRE(Load(xsq)); }

    SequenceElements& Sequence() override { return _ctx->GetSequenceElements(); }
    bool HasSequence() override { return _ctx != nullptr; }
    bool SaveXsq(std::string& out) override {
        _seqFile->SetFullPath(_file + ".snap.xsq");
        out = _seqFile->Save(Sequence()) ? ReadFile(_file + ".snap.xsq") : "";
        return !out.empty();
    }
    bool LoadXsq(const std::string& xsq) override {
        std::string p = _file + ".load.xsq";
        std::ofstream(p, std::ios::binary) << xsq;
        ++loads;
        return Load(p);
    }
    std::string ShowFolder() override { return _show; }
    bool UseShowFolder(const std::string& folder) override {
        _show = folder;
        InstallDirs();
        return true;
    }
    void AddMediaFolder(const std::string& folder) override {
        _media.push_back(folder);
        InstallDirs();
    }
    void InstallDirs() const {
        FileUtils::SetFixFileShowDir(_show);
        FileUtils::SetFixFileDirectories(_media);
    }

    void SetCollaborationLock(bool l, const std::string& reason) override {
        locked = l;
        if (_ctx) {
            _ctx->SetCollaborationLock(l, reason);
        }
    }
    void OnRejected(const RejectNotice& n) override { rejects.push_back(n); }
    void OnJoinReport(const JoinReport& r) override { report = r; }
    void OnSessionsFound(const std::vector<SessionAdvert>& s) override { found = s; }

    bool SupportsLiveLayout() override { return true; }
    uint64_t LayoutGeneration() override { return gen; }
    bool LayoutEditInProgress() override { return false; }
    bool SerializeLayout(LayoutFiles& out) override {
        if (!_ctx) {
            return false;
        }
        out.rgbEffects = SerializeRgb(*_ctx);
        out.networks = networks;
        return true;
    }
    bool ApplyLayout(const LayoutFiles& current, const LayoutFiles& target, LayoutApplied& out) override {
        ++gen;
        if (!_ctx->AbortRender()) {
            return false;
        }
        pugi::xml_document c = ParseXml(current.rgbEffects);
        pugi::xml_document t = ParseXml(target.rgbEffects);
        auto ch = ShowMergeApply::ApplyLayout(*_ctx, c, t);
        out.layoutChanged = ch.Any();
        out.modelsChanged = ch.modelsChanged;
        out.modelsRemoved = ch.modelsRemoved;
        out.modelsRenamed = ch.modelsRenamed;
        out.controllersChanged = current.networks != target.networks;
        networks = target.networks;
        return true;
    }
    void SetSequenceSettingsLock(bool l, const std::string& reason) override {
        if (_ctx) {
            _ctx->SetSequenceSettingsLock(l, reason);
        }
    }

    Model* M(const std::string& name) { return _ctx->AllModels.GetModel(name); }
    LayoutFiles Memory() {
        LayoutFiles f;
        REQUIRE(SerializeLayout(f));
        return f;
    }

    uint64_t gen = 1;
    std::string networks;
    bool locked = false;
    std::vector<RejectNotice> rejects;
    std::optional<JoinReport> report;
    std::vector<SessionAdvert> found;
    int loads = 0;

private:
    bool Load(const std::string& xsq) {
        _seqFile.reset();
        _ctx.reset();
        _ctx = std::make_unique<GroupContext>();
        if (!_ctx->LoadShowFolder(_show, _media)) {
            return false;
        }
        networks = ReadFile((fs::path(_show) / "xlights_networks.xml").string());
        _seqFile = std::make_unique<SequenceFile>(xsq);
        auto doc = _seqFile->Open(_show, false, xsq);
        if (!doc || !_ctx->LoadSequenceElements(*_seqFile, *doc)) {
            return false;
        }
        _ctx->sequenceOpen = true;
        if (locked) {
            _ctx->SetCollaborationLock(true, "");
        }
        return true;
    }

    std::string _show;
    std::string _file;
    std::list<std::string> _media;
    std::unique_ptr<GroupContext> _ctx;
    std::unique_ptr<SequenceFile> _seqFile;
};

struct CurlLogger {
    CurlLogger() {
        if (!spdlog::get("curl")) {
            spdlog::register_logger(std::make_shared<spdlog::logger>("curl", std::make_shared<spdlog::sinks::null_sink_mt>()));
        }
    }
};

// The collaboration test show with layout ids, a controller and a picture
// the sequence uses, written the way memory serializes it.
struct GroupShow {
    CurlLogger curl;
    Show show;
    std::string picture;

    GroupShow() {
        show.dir.Write("xlights_networks.xml",
                       "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<Networks computer=\"one\">\n"
                       "  <Controller Type=\"Null\" Id=\"64001\" Name=\"Bulk\" xlId=\"c-bulk\"><network NetworkType=\"NULL\" MaxChannels=\"500000\"/></Controller>\n"
                       "</Networks>\n");
        show.dir.Write("xlights_rgbeffects.xml",
                       "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xrgb>\n<models>\n"
                       "<model name=\"Grid\" xlId=\"m-grid\" DisplayAs=\"Matrix\" StartChannel=\"1\" NumStrings=\"3\" NodesPerString=\"4\" StrandsPerString=\"1\"/>\n"
                       "<model name=\"Line\" xlId=\"m-line\" DisplayAs=\"Single Line\" StartChannel=\"100\" NumStrings=\"1\" NodesPerString=\"4\"/>\n"
                       "<model name=\"Arch\" xlId=\"m-arch\" DisplayAs=\"Arches\" StartChannel=\"200\" NumStrings=\"1\" NodesPerString=\"8\"/>\n"
                       "<model name=\"Star\" xlId=\"m-star\" DisplayAs=\"Star\" StartChannel=\"300\" NumStrings=\"1\" NodesPerString=\"10\"/>\n"
                       "<model name=\"Pole\" xlId=\"m-pole\" DisplayAs=\"Single Line\" StartChannel=\"400\" NumStrings=\"2\" NodesPerString=\"3\"/>\n"
                       "</models>\n<modelGroups/>\n</xrgb>\n");
        HeadlessRenderContext ctx;
        REQUIRE(ctx.LoadShowFolder(show.path));
        show.dir.Write("xlights_rgbeffects.xml", SerializeRgb(ctx));
        // Large enough to need the chunked path when the store is small.
        picture = WriteFile(show.dir.path / "Images" / "a.png", RandomBytes(300000, 5));
    }
};

struct GPeer {
    std::string name;
    std::string member;
    IdState ids;
    std::unique_ptr<GroupApp> app;
    std::unique_ptr<FakeSource> src;
    PostQueue posted;
    std::unique_ptr<CollabController> ctl;

    SequenceElements& Seq() { return app->Sequence(); }
    CollabClient& Client() {
        // Checked: after a host migration a device's session changes kind.
        auto* s = dynamic_cast<CollabClient*>(ctl->Session());
        REQUIRE(s != nullptr);
        return *s;
    }
    CollabHost& Host() {
        // Checked: after a host migration a device's session changes kind.
        auto* s = dynamic_cast<CollabHost*>(ctl->Session());
        REQUIRE(s != nullptr);
        return *s;
    }
};

class GroupWorld {
public:
    explicit GroupWorld(GroupShow& show) : _show(show.show) {
        _baseCounter = SeqIds::Counter().load() + 1000;
        g.wrapper = [this](const std::string& member, const std::function<void()>& fn) {
            GPeer* p = Find(member);
            if (p == nullptr) {
                fn();
                return;
            }
            Scope s(*p);
            fn();
        };
    }
    ~GroupWorld() {
        for (auto& p : peers) {
            Scope s(*p);
            p->ctl.reset();
            p->app.reset();
        }
    }

    struct Scope {
        ActAs ids;
        explicit Scope(GPeer& p) : ids(p.ids) {
            if (p.app) {
                p.app->InstallDirs();
            }
        }
    };

    GPeer* Find(const std::string& member) {
        for (auto& p : peers) {
            if (p->member == member) {
                return p.get();
            }
        }
        return nullptr;
    }

    GPeer& Add(const std::string& name, const std::string& member, const std::string& showPath = "", const std::string& sessionFolder = "") {
        auto p = std::make_unique<GPeer>();
        GPeer* pp = p.get();
        pp->name = name;
        pp->member = member;
        pp->ids.counter = _baseCounter;
        pp->src = std::make_unique<FakeSource>(g, member, name);
        pp->app = std::make_unique<GroupApp>(showPath.empty() ? _show.path : showPath, _show.dir.File("dev" + std::to_string(peers.size())));
        peers.push_back(std::move(p));
        Scope s(*pp);
        pp->app->Open(_show.start);
        ControllerOptions opts;
        opts.appVersion = "test";
        opts.deviceKind = "ipad";
        opts.session.verifyHashes = true;
        opts.sessionFolder = sessionFolder;
        Dispatcher d = [pp](std::function<void()> fn) { pp->posted.push_back(std::move(fn)); };
        pp->ctl = std::make_unique<CollabController>(*pp->app, CollabNetFactory{}, d, opts);
        CHECK_FALSE(pp->ctl->GroupAvailable());
        pp->ctl->SetGroupNetwork(CollabNetFactory::Group(d, *pp->src));
        return *pp;
    }

    GPeer& host() { return *peers[0]; }

    void HostOn(GPeer& p) {
        Scope s(p);
        std::string err;
        REQUIRE(p.ctl->GroupAvailable());
        REQUIRE(p.ctl->HostGroup(p.name, "start.xsq", err));
        CHECK(p.ctl->GetState() == CollabController::State::Connecting);
        CHECK(p.ctl->SessionNetwork() == CollabController::Network::Group);
    }

    void JoinTo(GPeer& p, bool fetch = false) {
        RunFor(50);
        Scope s(p);
        SessionAdvert offer = p.ctl->OfferedGroupSession();
        REQUIRE_FALSE(offer.id.empty());
        CHECK(offer.group);
        CHECK(offer.hostName == host().name);
        std::string err;
        REQUIRE(p.ctl->JoinGroup(p.name, offer.id, fetch, err));
    }

    void Pump() {
        for (bool any = true; any;) {
            any = false;
            for (auto& p : peers) {
                Scope s(*p);
                while (p->ctl) {
                    auto fn = p->posted.take();
                    if (!fn) {
                        break;
                    }
                    fn();
                    any = true;
                }
            }
            if (WaitForCopies()) {
                any = true;
            }
        }
    }

    void Tick() {
        for (auto& p : peers) {
            Scope s(*p);
            if (p->ctl) {
                p->ctl->Tick(g.Now());
            }
        }
    }

    void RunFor(uint64_t ms, uint64_t step = 5) {
        uint64_t end = g.Now() + ms;
        while (g.Now() < end) {
            Pump();
            g.RunUntil(std::min(end, g.Now() + step));
            Pump();
            Tick();
        }
        Pump();
    }

    bool Quiet() {
        Pump();
        if (g.InFlight() != 0) {
            return false;
        }
        if (host().ctl->GetState() != CollabController::State::Active) {
            return false;
        }
        Rev head = host().Host().HeadRev();
        for (size_t i = 1; i < peers.size(); ++i) {
            GPeer& p = *peers[i];
            auto st = p.ctl->GetState();
            if (st == CollabController::State::Ended || st == CollabController::State::Idle) {
                continue;
            }
            if (st != CollabController::State::Active || p.Client().PendingCount() != 0 || p.Client().AppliedRev() != head ||
                p.ctl->HeldDeliveries() != 0) {
                return false;
            }
        }
        return true;
    }

    bool Quiesce(uint64_t limitMs = 60000) {
        uint64_t end = g.Now() + limitMs;
        while (g.Now() < end) {
            if (Quiet()) {
                return true;
            }
            g.RunUntil(g.Now() + 5);
            Pump();
            Tick();
        }
        return Quiet();
    }

    void Settle() {
        RunFor(600);
        REQUIRE(Quiesce());
        RunFor(600);
        REQUIRE(Quiesce());
    }

    template<class Fn>
    auto As(GPeer& p, Fn&& fn) {
        Scope s(p);
        return fn();
    }

    FakeGroup g;
    std::vector<std::unique_ptr<GPeer>> peers;

private:
    Show& _show;
    uint64_t _baseCounter;
};

std::string Desc(GPeer& p, const std::string& model) {
    Model* m = p.app->M(model);
    return m == nullptr ? std::string("<none>") : m->GetDescription();
}

void SetDesc(GroupWorld& w, GPeer& p, const std::string& model, const std::string& d) {
    w.As(p, [&] {
        Model* m = p.app->M(model);
        REQUIRE(m != nullptr);
        m->SetDescription(d);
        ++p.app->gen;
    });
}

Effect* AddEffectOn(GroupWorld& w, GPeer& p, const std::string& element, int start, int end, const std::string& name = "On",
                    const std::string& settings = "") {
    return w.As(p, [&]() -> Effect* {
        Element* el = p.Seq().GetElement(element);
        REQUIRE(el != nullptr);
        p.Seq().get_undo_mgr().CreateUndoStep();
        return p.Seq().GetCommandExecutor().AddEffect(el->GetEffectLayer(0), name, settings, "", start, end, 0, false);
    });
}

// Every active peer holds the host's layout and document.
void RequireSame(GroupWorld& w) {
    REQUIRE(w.Quiesce());
    GPeer& h = w.host();
    uint64_t layout = h.ctl->Session()->LayoutHash();
    uint64_t memory = w.As(h, [&] { return LayoutDelta::Hash(h.app->Memory()); });
    uint64_t doc = DocHasher::FullHash(h.Seq());
    std::string desc = DescribeSequence(h.Seq());
    CHECK(layout == memory);
    for (size_t i = 1; i < w.peers.size(); ++i) {
        GPeer& c = *w.peers[i];
        INFO("peer " << c.name);
        REQUIRE(c.ctl->InSession());
        CHECK(c.ctl->Session()->LayoutHash() == layout);
        CHECK(w.As(c, [&] { return LayoutDelta::Hash(c.app->Memory()); }) == memory);
        CHECK(DocHasher::FullHash(c.Seq()) == doc);
        CHECK(DescribeSequence(c.Seq()) == desc);
        CHECK(c.Client().HashVerifyFailures() == 0);
    }
    CHECK(h.Host().HashVerifyFailures() == 0);
}

} // namespace

TEST_CASE("Group sessions: a host and two clients edit the sequence and the layout over the group", "[collab][group][controller]") {
    for (uint64_t store : { uint64_t(0), uint64_t(64u << 10) }) {
        INFO("attachment store " << store);
        GroupShow show;
        GroupWorld w(show);
        w.g.maxMessage = 1024;
        w.g.maxAttachment = store;
        w.g.jitter = 20;
        GPeer& h = w.Add("Hana", "m-1");
        REQUIRE(AddEffectOn(w, h, "Line", 3000, 3400, "Pictures", "E_TEXTCTRL_Pictures_Filename=Images/a.png") != nullptr);
        w.HostOn(h);
        w.RunFor(100);
        REQUIRE(h.ctl->GetState() == CollabController::State::Active);
        CHECK(h.ctl->LiveLayout());

        GPeer& ari = w.Add("Ari", "m-2");
        w.JoinTo(ari);
        // Bea's own show folder has another layout and lacks the picture;
        // she fetches what differs and opens against a session copy.
        TempDir beaShow, beaSession;
        CopyShowFiles(show.show.path, beaShow.path, { "xlights_networks.xml", "xlights_rgbeffects.xml" });
        {
            std::string rgb = ReadFile((beaShow.path / "xlights_rgbeffects.xml").string());
            auto at = rgb.find("NodesPerString=\"10\"");
            REQUIRE(at != std::string::npos);
            rgb.replace(at, 19, "NodesPerString=\"12\"");
            WriteFile(beaShow.path / "xlights_rgbeffects.xml", rgb);
        }
        GPeer& bea = w.Add("Bea", "m-3", beaShow.path.string(), beaSession.path.string());
        w.JoinTo(bea, true);
        REQUIRE(w.Quiesce());
        for (auto& p : w.peers) {
            INFO(p->name);
            REQUIRE(p->ctl->InSession());
            CHECK(p->ctl->SessionNetwork() == CollabController::Network::Group);
        }
        CHECK(ari.ctl->StatusText() == "In Hana's session with 1 other");
        REQUIRE(bea.app->report);
        CHECK(bea.app->report->sessionCopy);
        CHECK(std::find(bea.app->report->fetched.begin(), bea.app->report->fetched.end(), "Images/a.png") != bea.app->report->fetched.end());
        CHECK(ReadFile((fs::path(bea.app->ShowFolder()) / "Images" / "a.png").string()) == ReadFile(show.picture));
        RequireSame(w);

        // Sequence edits from everyone.
        REQUIRE(AddEffectOn(w, h, "Grid", 4000, 4200) != nullptr);
        REQUIRE(AddEffectOn(w, ari, "Grid", 5000, 5200) != nullptr);
        REQUIRE(AddEffectOn(w, bea, "Line", 6000, 6200) != nullptr);
        w.RunFor(300);
        RequireSame(w);
        int effects = w.As(h, [&] { return (int)h.Seq().GetElement("Grid")->GetEffectLayer(0)->GetEffectCount(); });
        CHECK(effects == 4);

        // A layout edit on a client reaches everyone.
        SetDesc(w, ari, "Star", "from ari");
        w.Settle();
        CHECK(Desc(h, "Star") == "from ari");
        CHECK(Desc(bea, "Star") == "from ari");
        RequireSame(w);

        // A client that drifts is re-sent the whole document.
        bea.Client().faults.clientSkipsLayoutUpdate = true;
        SetDesc(w, h, "Arch", "from hana");
        w.Settle();
        bea.Client().faults.clientSkipsLayoutUpdate = false;
        w.RunFor(3000);
        CHECK(bea.Client().ResyncCount() >= 1);
        CHECK(Desc(bea, "Arch") == "from hana");
        RequireSame(w);
        CHECK(ari.Client().ResyncCount() == 0);

        // What went over the group: files (the layout and snapshots among
        // them) through the store or in chunks, and nothing too large.
        CHECK(w.g.oversize == 0);
        if (store == 0) {
            CHECK(w.g.attaches == 0);
            CHECK(w.g.sentByType[GroupWire::Type::BlobStart] >= 4);
        } else {
            CHECK(w.g.attaches >= 3);
            CHECK(w.g.sentByType[GroupWire::Type::BlobStart] >= 1); // the picture is over the store's limit
            CHECK(w.g.Stored() == 0);
        }
        for (auto& p : w.peers) {
            CHECK(p->app->rejects.empty());
        }

        w.As(h, [&] { h.ctl->End(); });
        w.RunFor(200);
        CHECK(ari.ctl->GetState() == CollabController::State::Ended);
        CHECK(ari.ctl->EndText() == "The host ended the session.");
        CHECK(bea.ctl->GetState() == CollabController::State::Ended);
    }
}

TEST_CASE("Group sessions: the controller lists the offer and reports a failed start", "[collab][group][controller]") {
    GroupShow show;
    GroupWorld w(show);
    GPeer& h = w.Add("Hana", "m-1");
    GPeer& ari = w.Add("Ari", "m-2");
    CHECK(h.ctl->GroupAvailable());
    CHECK(h.ctl->GroupInCall());
    h.src->inCall = false;
    CHECK_FALSE(h.ctl->GroupInCall());
    CHECK(ari.ctl->OfferedGroupSession().id.empty());

    // The platform refuses to start the activity.
    h.src->failStart = true;
    std::string err;
    CHECK_FALSE(w.As(h, [&] { return h.ctl->HostGroup("Hana", "start.xsq", err); }));
    CHECK(err == "The activity could not start.");
    CHECK(h.ctl->GetState() == CollabController::State::Idle);
    CHECK_FALSE(h.app->locked);

    // It starts, and the offer reaches the other device without browsing.
    h.src->failStart = false;
    w.HostOn(h);
    CHECK(h.ctl->StatusText() == "Starting the session...");
    w.RunFor(100);
    CHECK(h.ctl->GetState() == CollabController::State::Active);
    SessionAdvert offer = ari.ctl->OfferedGroupSession();
    CHECK(offer.id == w.g.SessionId());
    CHECK(offer.group);
    REQUIRE(ari.app->found.size() == 1);
    CHECK(ari.app->found[0].id == offer.id);
    REQUIRE(ari.ctl->Sessions().size() == 1);
    CHECK(ari.ctl->Sessions()[0].group);
    // No LAN here: the group advert is only for JoinGroup.
    CHECK_FALSE(w.As(ari, [&] { return ari.ctl->Join("Ari", offer.id, "", false, err); }));

    // The group ends: the host's session ends with the reason.
    w.g.End("The call ended.");
    w.RunFor(100);
    CHECK(h.ctl->GetState() == CollabController::State::Ended);
    CHECK(h.ctl->EndText().find("The call ended.") != std::string::npos);
    CHECK(ari.ctl->OfferedGroupSession().id.empty());
    CHECK(ari.app->found.empty());
}

TEST_CASE("Group sessions: ending while the activity starts never starts it", "[collab][group][controller]") {
    GroupShow show;
    GroupWorld w(show);
    GPeer& h = w.Add("Hana", "m-1");
    w.HostOn(h);
    w.As(h, [&] { h.ctl->End(); });
    CHECK(h.ctl->GetState() == CollabController::State::Ended);
    w.RunFor(200);
    CHECK(h.ctl->GetState() == CollabController::State::Ended);
    // The activity the platform started has nobody in it.
    GPeer& ari = w.Add("Ari", "m-2");
    w.RunFor(50);
    if (!ari.ctl->OfferedGroupSession().id.empty()) {
        std::string err;
        REQUIRE(w.As(ari, [&] { return ari.ctl->JoinGroup("Ari", ari.ctl->OfferedGroupSession().id, false, err); }));
        w.RunFor(500);
        CHECK(ari.ctl->GetState() == CollabController::State::Connecting);
        CHECK(w.g.sentByType[GroupWire::Type::Welcome] == 0);
    }
}

// ---------------------------------------------------------------------------
// Host migration over a group
// ---------------------------------------------------------------------------

namespace {

GPeer* GroupHostNow(GroupWorld& w) {
    for (auto& p : w.peers) {
        if (p->ctl && p->ctl->GetRole() == CollabController::Role::Host && p->ctl->GetState() == CollabController::State::Active) {
            return p.get();
        }
    }
    return nullptr;
}

// Every peer still in the group and the session caught up with `host`.
bool GroupQuietWith(GroupWorld& w, GPeer& host) {
    w.Pump();
    if (w.g.InFlight() != 0 || GroupHostNow(w) != &host) {
        return false;
    }
    Rev head = host.Host().HeadRev();
    for (auto& pp : w.peers) {
        GPeer& p = *pp;
        if (&p == &host || !p.ctl || p.ctl->GetState() == CollabController::State::Ended) {
            continue;
        }
        if (p.ctl->GetState() != CollabController::State::Active || p.Client().PendingCount() != 0 || p.Client().AppliedRev() != head) {
            return false;
        }
    }
    return true;
}

bool GroupQuiesceWith(GroupWorld& w, GPeer& host, uint64_t limitMs = 30000) {
    uint64_t end = w.g.Now() + limitMs;
    while (w.g.Now() < end) {
        if (GroupQuietWith(w, host)) {
            return true;
        }
        w.g.RunUntil(w.g.Now() + 5);
        w.Pump();
        w.Tick();
    }
    return GroupQuietWith(w, host);
}

void RequireSameAs(GroupWorld& w, GPeer& h) {
    REQUIRE(GroupQuiesceWith(w, h));
    uint64_t layout = h.ctl->Session()->LayoutHash();
    uint64_t memory = w.As(h, [&] { return LayoutDelta::Hash(h.app->Memory()); });
    uint64_t doc = DocHasher::FullHash(h.Seq());
    CHECK(layout == memory);
    for (auto& pp : w.peers) {
        GPeer& c = *pp;
        if (&c == &h || !c.ctl) {
            continue;
        }
        INFO("peer " << c.name);
        REQUIRE(c.ctl->InSession());
        CHECK(c.ctl->Session()->LayoutHash() == layout);
        CHECK(w.As(c, [&] { return LayoutDelta::Hash(c.app->Memory()); }) == memory);
        CHECK(DocHasher::FullHash(c.Seq()) == doc);
        CHECK(c.Client().HashVerifyFailures() == 0);
    }
    CHECK(h.Host().HashVerifyFailures() == 0);
}

} // namespace

TEST_CASE("Group sessions: a lost host's session carries on with the live layout on the same channel", "[collab][group][controller][migrate]") {
    GroupShow show;
    GroupWorld w(show);
    GPeer& h = w.Add("Hana", "m-1");
    w.HostOn(h);
    w.RunFor(100);
    REQUIRE(h.ctl->LiveLayout());
    GPeer& ari = w.Add("Ari", "m-2");
    w.JoinTo(ari);
    GPeer& bea = w.Add("Bea", "m-3");
    w.JoinTo(bea);
    REQUIRE(w.Quiesce());
    REQUIRE(AddEffectOn(w, ari, "Grid", 4000, 4200) != nullptr);
    SetDesc(w, bea, "Star", "from bea");
    w.Settle();
    // Hana has the Arch open in a modal editor; everyone sees it locked.
    std::string heldBy;
    REQUIRE(w.As(h, [&] { return h.ctl->BeginLayoutLock({ "m-arch", "Arch", "submodels" }, heldBy); }));
    w.RunFor(1500);
    REQUIRE(w.Quiesce());
    CHECK(bea.ctl->LayoutLockHolder("m-arch") == 1);
    CHECK(ari.Client().Succession() == std::vector<ParticipantId>{ 2, 3 });
    // A layout change of Ari's is on its way when Hana goes.
    SetDesc(w, ari, "Pole", "from ari");
    w.As(ari, [&] { ari.ctl->FlushLayout(); });
    w.Pump();
    // Gone without a word; what is left of its app can't reach anyone.
    w.g.Drop("m-1");
    w.As(h, [&] { h.ctl.reset(); });
    REQUIRE(GroupQuiesceWith(w, ari));
    CHECK(w.g.sentByType[GroupWire::Type::Succeed] == 1);
    CHECK(ari.ctl->GetRole() == CollabController::Role::Host);
    CHECK(ari.ctl->SessionNetwork() == CollabController::Network::Group);
    CHECK(bea.ctl->HostParticipant() == 2);
    // The live layout carried over: the session layout, memory and what is
    // read-only (Hana's lock went with her).
    for (GPeer* p : { &ari, &bea }) {
        INFO(p->name);
        CHECK(p->ctl->LiveLayout());
        CHECK(p->ctl->LayoutLockHolder("m-arch") == kNoParticipant);
        CHECK(Desc(*p, "Star") == "from bea");
    }
    CHECK(ari.Host().LayoutVersionsKept() >= 1);
    w.RunFor(1000);
    RequireSameAs(w, ari);
    CHECK(Desc(bea, "Pole") == "from ari");
    // Layout and sequence edits after the change, from both.
    SetDesc(w, bea, "Arch", "bea after");
    SetDesc(w, ari, "Line", "ari after");
    REQUIRE(AddEffectOn(w, bea, "Grid", 5000, 5200) != nullptr);
    w.RunFor(1500);
    RequireSameAs(w, ari);
    CHECK(Desc(ari, "Arch") == "bea after");
    CHECK(Desc(bea, "Line") == "ari after");
    CHECK(bea.Client().ResyncCount() == 0);
    for (auto& p : w.peers) {
        CHECK(p->app->rejects.empty());
    }
    CHECK(ari.app->locked == false);
    // A new member joining now gets the session from Ari.
    GPeer& cy = w.Add("Cy", "m-4");
    w.RunFor(100);
    std::string err;
    REQUIRE(w.As(cy, [&] { return cy.ctl->JoinGroup("Cy", cy.ctl->OfferedGroupSession().id, false, err); }));
    w.RunFor(1500);
    RequireSameAs(w, ari);
    CHECK(cy.ctl->LocalParticipant() == 4);
    CHECK(cy.ctl->HostName() == "Ari");
}

TEST_CASE("Group sessions: a client reaches only the successor it expects", "[collab][group][migrate]") {
    Rig r;
    Dev& h = r.Add("m-1");
    Dev& a = r.Add("m-2");
    Dev& b = r.Add("m-3");
    r.Host(h);
    r.Settle();
    r.Join(a);
    r.Join(b);
    r.Settle();
    r.g.Drop("m-1");
    r.Settle();
    REQUIRE(a.Has(kLanHostPeer, PeerEvent::Lost));
    REQUIRE(b.t->State() == NetState::Failed);
    CHECK(a.t->SupportsMigration());
    // b looks for participant 3 first; a takes over as participant 2.
    Succession s;
    s.epoch = 1;
    s.successor = 3;
    b.t->ConnectToSuccessor(s);
    r.Settle();
    s.successor = 2;
    s.applied = 9;
    std::string error;
    REQUIRE(a.t->ListenAsSuccessor(s, SessionAdvert{}, error));
    r.Settle();
    CHECK(a.t->LocalPeer() == kLanHostPeer);
    CHECK(a.t->State() == NetState::Listening);
    // Not the one b expects: it keeps waiting.
    CHECK(b.t->State() == NetState::Connecting);
    b.t->ConnectToSuccessor(s);
    r.Settle();
    REQUIRE(b.t->State() == NetState::Connected);
    CHECK(b.Has(kLanHostPeer, PeerEvent::Joined));
    CHECK(a.Has(2, PeerEvent::Joined));
    a.t->Send(2, Ping(5));
    b.t->Send(kLanHostPeer, Ping(6));
    r.Settle();
    REQUIRE_FALSE(b.messages.empty());
    REQUIRE_FALSE(a.messages.empty());
    CHECK(b.messages.back().second.time == 5);
    CHECK(a.messages.back().second.time == 6);
    // A member that joins the call later gets an ordinary claim from it.
    Dev& c = r.Add("m-4");
    r.Join(c);
    r.Settle();
    CHECK(c.t->State() == NetState::Connected);
    CHECK(a.Has(3, PeerEvent::Joined));
    CHECK(GroupTest::MalformedFrames(*b.t) == 0);
}

TEST_CASE("Group sessions: a client that starts reconnecting after the new host took over still reaches it", "[collab][group][migrate]") {
    Rig r;
    Dev& h = r.Add("m-1");
    Dev& a = r.Add("m-2");
    Dev& b = r.Add("m-3");
    r.Host(h);
    r.Settle();
    r.Join(a);
    r.Join(b);
    r.Settle();
    r.g.Drop("m-1");
    r.Settle();
    REQUIRE(b.t->State() == NetState::Failed);
    // a announces its takeover while b's channel still counts as finished.
    Succession s;
    s.epoch = 1;
    s.successor = 2;
    std::string error;
    REQUIRE(a.t->ListenAsSuccessor(s, SessionAdvert{}, error));
    r.Settle();
    b.t->ConnectToSuccessor(s);
    r.Settle();
    REQUIRE(b.t->State() == NetState::Connected);
    CHECK(b.Has(kLanHostPeer, PeerEvent::Joined));
    CHECK(a.Has(2, PeerEvent::Joined));
}

TEST_CASE("Group sessions: a snapshot on a slow group is asked for and sent once", "[collab][group][controller][collab_transfer]") {
    // Before the world: its teardown still sends frames.
    std::map<MsgType, int> fromAri;
    size_t blobsToAri = 0;
    GroupShow show;
    GroupWorld w(show);
    // A slow call: about one blob window (128 KB) per second.
    w.g.maxMessage = 16384;
    w.g.latency = 500;
    GPeer& h = w.Add("Hana", "m-1");
    // Settings are stored once per distinct text: each is different.
    for (int i = 0; i < 300; ++i) {
        std::string text = "E_TEXTCTRL_Text=" + std::to_string(i) + std::string(4000, 'x');
        REQUIRE(AddEffectOn(w, h, "Grid", 3000 + 20 * i, 3010 + 20 * i, "Text", text) != nullptr);
    }
    std::string xsq;
    REQUIRE(w.As(h, [&] { return h.app->SaveXsq(xsq); }));
    REQUIRE(xsq.size() > 1000000); // many seconds over this call
    w.HostOn(h);
    for (int i = 0; i < 50 && h.ctl->GetState() != CollabController::State::Active; ++i) {
        w.RunFor(100);
    }
    REQUIRE(h.ctl->GetState() == CollabController::State::Active);
    w.g.onSend = [&](const std::string& from, const std::string& to, const std::string& bytes) {
        if (bytes.size() < GroupWire::kHeaderBytes) {
            return;
        }
        auto type = (GroupWire::Type)(uint8_t)bytes[1];
        if (from == "m-2" && type == GroupWire::Type::Msg) {
            if (auto m = DecodeMessage(std::string_view(bytes).substr(GroupWire::kHeaderBytes))) {
                ++fromAri[m->type];
            }
        }
        if (from == "m-1" && to == "m-2" && type == GroupWire::Type::BlobStart) {
            ++blobsToAri;
        }
    };
    GPeer& ari = w.Add("Ari", "m-2");
    w.JoinTo(ari);
    // On this link something is nearly always on its way: wait for Ari to
    // have the document instead of for silence.
    for (int i = 0; i < 600 && !(ari.ctl->InSession() && ari.Client().IsActive()); ++i) {
        w.RunFor(100);
    }
    w.RunFor(3000);
    REQUIRE(ari.ctl->InSession());
    CHECK(DocHasher::FullHash(ari.Seq()) == DocHasher::FullHash(h.Seq()));
    // Asked for once, and sent once (with the session layout before it).
    CHECK(fromAri[MsgType::NeedSnapshot] == 1);
    CHECK(blobsToAri == 2);
}

namespace {
// Host `h` and client `c` on a fresh group, joined; done reports collected.
struct DoneRig {
    Rig r;
    Dev* h = nullptr;
    Dev* c = nullptr;
    std::vector<std::pair<BlobId, bool>> done;
    TempDir dir;
    DoneRig() {
        r.g.maxMessage = 4096;
        h = &r.Add("m-1");
        c = &r.Add("m-2");
        r.Host(*h);
        r.Settle();
        r.Join(*c);
        r.Settle();
        REQUIRE(c->t->State() == NetState::Connected);
        h->t->onBlobDone = [this](PeerId to, BlobId id, bool sent) {
            CHECK(to == c->t->LocalPeer());
            done.emplace_back(id, sent);
        };
    }
    PeerId C() const { return c->t->LocalPeer(); }
    // The transport's own copy of the file it is sending.
    std::string OutCopy() {
        std::string found;
        for (auto& e : fs::directory_iterator(GroupTest::Folder(*h->t))) {
            if (e.path().filename().string().rfind("out-", 0) == 0) {
                found = e.path().string();
            }
        }
        return found;
    }
};
} // namespace

TEST_CASE("Group sessions: a blob is done only once the recipient has acknowledged all of it", "[collab][group]") {
    DoneRig d;
    std::string body = RandomBytes(300000, 9);
    d.h->t->SendBlob(d.C(), 41, d.dir.Write("b", body));
    // Most of it is still waiting for acknowledgements.
    d.r.Pump();
    CHECK(d.done.empty());
    d.r.Settle();
    REQUIRE(d.c->blobs.size() == 1);
    CHECK(d.c->blobs[0].content == body);
    REQUIRE(d.done.size() == 1);
    CHECK(d.done[0] == std::make_pair(BlobId(41), true));
}

TEST_CASE("Group sessions: a transfer cut short is reported as failed", "[collab][group]") {
    DoneRig d;
    d.h->t->SendBlob(d.C(), 42, d.dir.Write("b", RandomBytes(400000, 10)));
    // Our own copy (made off this thread) shrinks after the first window
    // went out.
    d.r.Pump();
    std::string copy = d.OutCopy();
    REQUIRE_FALSE(copy.empty());
    fs::resize_file(copy, 200000);
    d.r.Settle();
    // The recipient drops the short blob; the sender says it didn't go, so
    // the session sends it again at once instead of waiting.
    CHECK(d.c->blobs.empty());
    REQUIRE(d.done.size() == 1);
    CHECK(d.done[0] == std::make_pair(BlobId(42), false));
}

TEST_CASE("Group sessions: blobs to a member that leaves are reported as failed", "[collab][group]") {
    DoneRig d;
    d.r.g.maxAttachment = 1u << 20;
    d.r.g.attachmentsLost = true;
    d.h->t->SendBlob(d.C(), 43, d.dir.Write("big", RandomBytes(3u << 20, 11))); // in chunks
    d.h->t->SendBlob(d.C(), 44, d.dir.Write("small", RandomBytes(1000, 12)));   // through the store
    d.r.Pump();
    d.r.g.Drop("m-2");
    d.r.Settle();
    std::map<BlobId, bool> got(d.done.begin(), d.done.end());
    REQUIRE(d.done.size() == 2);
    CHECK(got.at(43) == false);
    CHECK(got.at(44) == false);
}

TEST_CASE("Group sessions: an attachment that is never acknowledged goes again in chunks", "[collab][group]") {
    DoneRig d;
    uint64_t now = 1000;
    GroupTest::SetClock(*d.h->t, [&] { return now; });
    d.r.g.maxAttachment = 1u << 20;
    d.r.g.attachmentsLost = true;
    std::string body = RandomBytes(1000, 13);
    d.h->t->SendBlob(d.C(), 45, d.dir.Write("small", body));
    d.r.Settle();
    CHECK(d.c->blobs.empty());
    CHECK(d.done.empty());
    // Frames from the client after a long silence about it.
    now += 61000;
    d.c->t->Send(kLanHostPeer, Ping(1));
    d.r.Settle();
    REQUIRE(d.c->blobs.size() == 1);
    CHECK(d.c->blobs[0].content == body);
    REQUIRE(d.done.size() == 1);
    CHECK(d.done[0] == std::make_pair(BlobId(45), true));
}

TEST_CASE("Group sessions: bytes lost from one blob don't hold up the blobs after it", "[collab][group]") {
    DoneRig d;
    int data = 0;
    // The second chunk of the first blob never arrives.
    d.r.g.lose = [&](const std::string& from, const std::string&, const std::string& bytes) {
        return from == "m-1" && bytes.size() > 10 && (GroupWire::Type)(uint8_t)bytes[1] == GroupWire::Type::BlobData &&
               bytes.compare(GroupWire::kHeaderBytes, 8, U64(51)) == 0 && ++data == 2;
    };
    std::string first = RandomBytes(300000, 21);
    std::string second = RandomBytes(300000, 22);
    d.h->t->SendBlob(d.C(), 51, d.dir.Write("a", first));
    d.h->t->SendBlob(d.C(), 52, d.dir.Write("b", second));
    d.r.Settle();
    // The damaged one is dropped and said not to have gone (the session
    // sends it again); the next one arrives whole.
    std::map<BlobId, bool> got(d.done.begin(), d.done.end());
    REQUIRE(d.done.size() == 2);
    CHECK(got.at(51) == false);
    CHECK(got.at(52) == true);
    REQUIRE(d.c->blobs.size() == 1);
    CHECK(d.c->blobs[0].id == 52);
    CHECK(d.c->blobs[0].content == second);
    // Sent again, it goes through.
    d.r.g.lose = nullptr;
    d.h->t->SendBlob(d.C(), 53, d.dir.Write("a2", first));
    d.r.Settle();
    REQUIRE(d.c->blobs.size() == 2);
    CHECK(d.c->blobs[1].content == first);
    CHECK(d.done.back() == std::make_pair(BlobId(53), true));
}

TEST_CASE("Group sessions: a blob whose acknowledgements stop is given up, and the next one flows", "[collab][group]") {
    DoneRig d;
    uint64_t now = 1000;
    GroupTest::SetClock(*d.h->t, [&] { return now; });
    // Every acknowledgement of blob 61 is lost.
    d.r.g.lose = [&](const std::string& from, const std::string&, const std::string& bytes) {
        return from == "m-2" && bytes.size() >= GroupWire::kHeaderBytes + 8 && (GroupWire::Type)(uint8_t)bytes[1] == GroupWire::Type::BlobAck &&
               bytes.compare(GroupWire::kHeaderBytes, 8, U64(61)) == 0;
    };
    d.h->t->SendBlob(d.C(), 61, d.dir.Write("a", RandomBytes(400000, 23)));
    std::string next = RandomBytes(200000, 24);
    d.h->t->SendBlob(d.C(), 62, d.dir.Write("b", next));
    d.r.Settle();
    CHECK(d.done.empty()); // stuck on its window
    now += 31000;
    d.c->t->Send(kLanHostPeer, Ping(1)); // anything from the recipient
    d.r.Settle();
    std::map<BlobId, bool> got(d.done.begin(), d.done.end());
    REQUIRE(got.count(61) == 1);
    CHECK(got.at(61) == false);
    REQUIRE(got.count(62) == 1);
    CHECK(got.at(62) == true);
    REQUIRE(!d.c->blobs.empty());
    CHECK(d.c->blobs.back().id == 62);
    CHECK(d.c->blobs.back().content == next);
}

TEST_CASE("Group sessions: a large attachment gets time for its size before it is resent", "[collab][group]") {
    DoneRig d;
    uint64_t now = 1000;
    GroupTest::SetClock(*d.h->t, [&] { return now; });
    d.r.g.maxAttachment = 8u << 20;
    d.r.g.attachmentsLost = true;
    std::string body = RandomBytes(4u << 20, 25); // 4 MB: about 64 s more at a slow rate
    d.h->t->SendBlob(d.C(), 71, d.dir.Write("big", body));
    d.r.Settle();
    // 61 s on: a small one would have been resent by now, this one not yet.
    now += 61000;
    d.c->t->Send(kLanHostPeer, Ping(1));
    d.r.Settle();
    CHECK(d.c->blobs.empty());
    CHECK(d.done.empty());
    // An acknowledgement from it (it is getting data) restarts the wait.
    now += 60000;
    d.h->t->SendBlob(d.C(), 72, d.dir.Write("small", RandomBytes(100, 26))); // through the store too: lost
    d.r.g.maxAttachment = 0;
    d.h->t->SendBlob(d.C(), 73, d.dir.Write("chunked", RandomBytes(100000, 27)));
    d.r.Settle();
    std::map<BlobId, bool> got(d.done.begin(), d.done.end());
    CHECK(got.count(73) == 1);
    CHECK(got.count(71) == 0);
    // Past its allowance with nothing more: it goes in chunks.
    now += 130000;
    d.c->t->Send(kLanHostPeer, Ping(2));
    d.r.Settle();
    got = std::map<BlobId, bool>(d.done.begin(), d.done.end());
    REQUIRE(got.count(71) == 1);
    CHECK(got.at(71) == true);
}

TEST_CASE("Group sessions: a host that hears its successor stands down", "[collab][group][migrate]") {
    Rig r;
    Dev& h = r.Add("m-1");
    Dev& a = r.Add("m-2");
    Dev& b = r.Add("m-3");
    r.Host(h);
    r.Settle();
    r.Join(a);
    r.Join(b);
    r.Settle();
    // a takes over while h is still in the group (it was only silent).
    Succession s;
    s.epoch = 1;
    s.successor = 2;
    std::string error;
    REQUIRE(a.t->ListenAsSuccessor(s, SessionAdvert{}, error));
    r.Settle();
    REQUIRE_FALSE(h.states.empty());
    CHECK(h.states.back().first == NetState::Failed);
    CHECK(h.states.back().second == "Another device took over this session while this one was not responding.");
    CHECK(h.Has(2, PeerEvent::Lost));
    // A later joiner reaches the successor, not the old host.
    size_t hEvents = h.events.size();
    Dev& c = r.Add("m-4");
    r.Join(c);
    r.Settle();
    CHECK(c.t->State() == NetState::Connected);
    CHECK(c.Has(kLanHostPeer, PeerEvent::Joined));
    CHECK(h.events.size() == hEvents);
    CHECK(std::any_of(a.events.begin(), a.events.end(), [](const auto& e) { return e.second == PeerEvent::Joined; }));
    // Having stood down, it leaves without ending the successor's activity.
    h.Close();
    REQUIRE(r.g.leaves.size() == 1);
    CHECK(r.g.leaves[0] == std::make_pair(std::string("m-1"), false));
}

TEST_CASE("Group sessions: an attachment key names the member it is for", "[collab][group]") {
    CHECK(GroupAttachmentIsFor("xlb1:12:3:member-a", "member-a"));
    CHECK_FALSE(GroupAttachmentIsFor("xlb1:12:3:member-a", "member-b"));
    CHECK_FALSE(GroupAttachmentIsFor("xlb1:12:3:member-a", ""));
    CHECK_FALSE(GroupAttachmentIsFor("xlb1:12:3:", ""));
    CHECK_FALSE(GroupAttachmentIsFor("other:12:3:member-a", "member-a"));
    CHECK_FALSE(GroupAttachmentIsFor("xlb1:x2:3:member-a", "member-a"));
    CHECK_FALSE(GroupAttachmentIsFor("xlb1:12:member-a", "member-a"));
}

TEST_CASE("Group sessions: members download only the attachments addressed to them", "[collab][group]") {
    Rig r;
    r.g.maxAttachment = 8u << 20;
    TempDir dir;
    Dev& h = r.Add("m-1");
    Dev& a = r.Add("m-2");
    Dev& b = r.Add("m-3");
    r.Host(h);
    r.Settle();
    r.Join(a);
    r.Join(b);
    r.Settle();
    std::string body = RandomBytes(100000, 61);
    h.t->SendBlob(2, 71, dir.Write("for-a", body));
    r.Settle();
    REQUIRE(a.blobs.size() == 1);
    CHECK(a.blobs[0].content == body);
    CHECK(b.blobs.empty());
    CHECK(r.g.downloads["m-2"] == 1);
    // b sees the attachment in the store but never fetches it.
    CHECK(r.g.downloads["m-3"] == 0);
    CHECK(r.g.skippedDownloads["m-3"] == 1);
}

TEST_CASE("Group sessions: only the current host ends the activity when it leaves", "[collab][group][migrate]") {
    Rig r;
    Dev& h = r.Add("m-1");
    Dev& a = r.Add("m-2");
    Dev& b = r.Add("m-3");
    r.Host(h);
    r.Settle();
    r.Join(a);
    r.Join(b);
    r.Settle();
    // The device that started the activity stops hosting: a takes over and
    // h comes back as one of its clients.
    Succession s;
    s.epoch = 1;
    s.successor = 2;
    std::string error;
    REQUIRE(a.t->ListenAsSuccessor(s, SessionAdvert{}, error));
    h.t->ConnectToSuccessor(s);
    r.Settle();
    REQUIRE(h.t->State() == NetState::Connected);
    REQUIRE(a.t->LocalPeer() == kLanHostPeer);
    auto leftWith = [&](const std::string& member) -> std::optional<bool> {
        for (auto& [m, end] : r.g.leaves) {
            if (m == member) {
                return end;
            }
        }
        return std::nullopt;
    };
    // The former host leaving must not end the new host's session.
    h.Close();
    REQUIRE(leftWith("m-1").has_value());
    CHECK_FALSE(*leftWith("m-1"));
    // A client only leaves.
    b.Close();
    REQUIRE(leftWith("m-3").has_value());
    CHECK_FALSE(*leftWith("m-3"));
    // The current host ends it for everyone, though it didn't start it.
    a.Close();
    REQUIRE(leftWith("m-2").has_value());
    CHECK(*leftWith("m-2"));
}

TEST_CASE("Group sessions: a file is copied off the caller's thread", "[collab][group]") {
    Rig r;
    TempDir dir;
    Dev& h = r.Add("m-1");
    Dev& a = r.Add("m-2");
    r.Host(h);
    r.Settle();
    r.Join(a);
    r.Settle();
    std::string body = RandomBytes(50000, 62);
    std::string path = dir.Write("f", body);
    std::atomic<bool> copied{ false };
    std::thread::id copier;
    Files::Test::noClone = true;
    Files::Test::beforeCopy = [&] {
        copier = std::this_thread::get_id();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        copied = true;
    };
    auto t0 = std::chrono::steady_clock::now();
    h.t->SendBlob(2, 81, path);
    double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
    // The caller may delete it at once.
    fs::remove(path);
    r.Settle();
    Files::Test::beforeCopy = nullptr;
    Files::Test::noClone = false;
    INFO("SendBlob " << ms << " ms");
    CHECK(copied.load());
    CHECK(copier != std::this_thread::get_id());
    CHECK(ms < 250);
    REQUIRE(a.blobs.size() == 1);
    CHECK(a.blobs[0].content == body);
}
