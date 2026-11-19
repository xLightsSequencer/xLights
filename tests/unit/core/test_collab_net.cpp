#include <catch2/catch_test_macros.hpp>

#include "collab/CollabNet.h"
#include "collab/CollabNetWire.h"

#include <random>
#include <string>
#include <vector>

// The LAN transport's framing and pairing helpers run everywhere; the
// transport itself (Network.framework, real TLS sockets on loopback) only on
// Apple platforms.

using namespace Collab;

TEST_CASE("Collab wire frames survive being split at any byte", "[collab][net]") {
    Message m;
    m.type = MsgType::Hello;
    m.name = "someone";
    std::string stream = Wire::WelcomeFrame(7) + Wire::EncodeFrame(Wire::FrameType::Message, EncodeMessage(m)) + Wire::BlobStartFrame(42, 3) +
                         Wire::BlobDataFrame(42, "abc") + Wire::BlobEndFrame(42);
    for (size_t step : { (size_t)1, (size_t)2, (size_t)7, stream.size() }) {
        Wire::FrameReader r;
        std::vector<Wire::Frame> got;
        for (size_t at = 0; at < stream.size(); at += step) {
            r.Feed(stream.data() + at, std::min(step, stream.size() - at));
            Wire::Frame f;
            Wire::FrameReader::Result res;
            while ((res = r.Next(f)) == Wire::FrameReader::Result::Frame) {
                got.push_back(f);
            }
            REQUIRE(res == Wire::FrameReader::Result::More);
        }
        REQUIRE(got.size() == 5);
        uint32_t peer = 0;
        CHECK(got[0].type == Wire::FrameType::Welcome);
        CHECK(Wire::ReadU32(got[0].payload, 0, peer));
        CHECK(peer == 7);
        REQUIRE(got[1].type == Wire::FrameType::Message);
        auto back = DecodeMessage(got[1].payload);
        REQUIRE(back);
        CHECK(back->name == "someone");
        CHECK(got[3].payload.substr(8) == "abc");
        CHECK(got[4].type == Wire::FrameType::BlobEnd);
        CHECK(r.Buffered() == 0);
    }
}

TEST_CASE("Collab wire reader refuses frames that can't be part of a session", "[collab][net]") {
    auto header = [](uint32_t len, uint8_t type) {
        std::string s;
        for (int i = 3; i >= 0; --i) {
            s.push_back((char)((len >> (8 * i)) & 0xFF));
        }
        s.push_back((char)type);
        return s;
    };
    auto verdict = [](const std::string& bytes) {
        Wire::FrameReader r;
        r.Feed(bytes.data(), bytes.size());
        Wire::Frame f;
        return r.Next(f);
    };
    using R = Wire::FrameReader::Result;
    // Refused on the header alone, before any payload is buffered.
    CHECK(verdict(header(0xFFFFFFF0u, (uint8_t)Wire::FrameType::Message)) == R::Bad);
    CHECK(verdict(header((uint32_t)Wire::kMaxMessageBytes + 2, (uint8_t)Wire::FrameType::Message)) == R::Bad);
    CHECK(verdict(header(5, 0x7F)) == R::Bad);
    CHECK(verdict(header(0, (uint8_t)Wire::FrameType::Message)) == R::Bad);
    CHECK(verdict(header(1, (uint8_t)Wire::FrameType::Message)) == R::Bad);
    CHECK(verdict(header(4, (uint8_t)Wire::FrameType::BlobEnd)) == R::Bad);
    CHECK(verdict(header(1 + 8 + (uint32_t)Wire::kBlobChunkBytes + 1, (uint8_t)Wire::FrameType::BlobData)) == R::Bad);
    // The largest legal message is only waited for.
    CHECK(verdict(header((uint32_t)Wire::kMaxMessageBytes + 1, (uint8_t)Wire::FrameType::Message)) == R::More);
    // Once bad, it stays bad even if good frames follow.
    Wire::FrameReader r;
    std::string bad = header(5, 0x7F) + "xxxx" + Wire::BlobEndFrame(1);
    r.Feed(bad.data(), bad.size());
    Wire::Frame f;
    CHECK(r.Next(f) == R::Bad);
    CHECK(r.Next(f) == R::Bad);
}

TEST_CASE("Collab pairing codes and session names", "[collab][net]") {
    for (int i = 0; i < 50; ++i) {
        std::string code = NewPairingCode();
        REQUIRE(code.size() == 6);
        for (char c : code) {
            REQUIRE(c >= '0');
            REQUIRE(c <= '9');
        }
    }
    std::string salt = Wire::NewSalt();
    REQUIRE(salt.size() == Wire::kSaltBytes);
    std::string name = Wire::ServiceNameFor(salt);
    std::string back;
    REQUIRE(Wire::SaltFromServiceName(name, back));
    CHECK(back == salt);
    // Bonjour renames a clashing instance "name (2)".
    back.clear();
    REQUIRE(Wire::SaltFromServiceName(name + " (2)", back));
    CHECK(back == salt);
    CHECK_FALSE(Wire::SaltFromServiceName("xlc-1234", back));
    CHECK_FALSE(Wire::SaltFromServiceName("abc-0011223344556677", back));
    CHECK_FALSE(Wire::SaltFromServiceName("xlc-00112233445566zz", back));
    CHECK_FALSE(Wire::SaltFromServiceName("xlc-00112233445566778", back));
}

#ifdef __APPLE__

#include "test_collab_common.h"

#include "render/SequenceCommands.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <thread>

namespace {

using Clock = std::chrono::steady_clock;

// The "main thread" of every transport in a test: work they post is queued
// here and run when the test pumps.
class TestLoop {
public:
    Dispatcher For(int tag) {
        return [this, tag](std::function<void()> fn) {
            {
                std::lock_guard<std::mutex> l(_m);
                _q.emplace_back(tag, std::move(fn));
            }
            _cv.notify_all();
        };
    }

    // Runs posted work until `done` holds or `ms` pass. `idle` runs between
    // batches (session timers).
    bool Until(const std::function<bool()>& done, int ms = 10000) {
        auto end = Clock::now() + std::chrono::milliseconds(ms);
        while (true) {
            if (done && done()) {
                return true;
            }
            if (Clock::now() >= end) {
                return done ? done() : true;
            }
            std::deque<std::pair<int, std::function<void()>>> batch;
            {
                std::unique_lock<std::mutex> l(_m);
                if (_q.empty()) {
                    _cv.wait_until(l, std::min(end, Clock::now() + std::chrono::milliseconds(5)));
                }
                batch.swap(_q);
            }
            for (auto& [tag, fn] : batch) {
                if (wrap) {
                    wrap(tag, fn);
                } else {
                    fn();
                }
            }
            if (idle) {
                idle();
            }
        }
    }
    void For_ms(int ms) { Until(nullptr, ms); }

    std::function<void(int tag, const std::function<void()>& fn)> wrap;
    std::function<void()> idle;

private:
    std::mutex _m;
    std::condition_variable _cv;
    std::deque<std::pair<int, std::function<void()>>> _q;
};

// One transport and what came out of it.
struct Peer {
    std::unique_ptr<INetTransport> t;
    std::vector<std::pair<PeerId, PeerEvent>> events;
    std::vector<std::pair<PeerId, Message>> messages;
    struct Blob {
        PeerId from;
        BlobId id;
        std::string path;
        std::string content;
        size_t messagesBefore;
    };
    std::vector<Blob> blobs;
    std::vector<std::pair<NetState, std::string>> states;
    bool closed = false;
    int callbacksAfterClose = 0;

    Peer(TestLoop& loop, int tag) {
        t = CreateLanTransport(loop.For(tag));
        REQUIRE(t);
        t->onPeer = [this](PeerId p, PeerEvent e) {
            callbacksAfterClose += closed;
            events.emplace_back(p, e);
        };
        t->onMessage = [this](PeerId p, const Message& m) {
            callbacksAfterClose += closed;
            messages.emplace_back(p, m);
        };
        t->onBlob = [this](PeerId p, BlobId id, const std::string& path) {
            callbacksAfterClose += closed;
            blobs.push_back({ p, id, path, CollabTest::ReadFile(path), messages.size() });
        };
        t->onState = [this](NetState s, const std::string& why) {
            callbacksAfterClose += closed;
            states.emplace_back(s, why);
        };
    }
    void Close() {
        t->Close();
        closed = true;
    }
    bool Has(PeerId p, PeerEvent e) const {
        return std::find(events.begin(), events.end(), std::make_pair(p, e)) != events.end();
    }
    bool Joined() const { return t->State() == NetState::Connected && Has(kLanHostPeer, PeerEvent::Joined); }
    PeerId FirstJoined() const {
        for (auto& [p, e] : events) {
            if (e == PeerEvent::Joined) {
                return p;
            }
        }
        return kNoPeer;
    }
};

Message Ping(uint64_t n) {
    Message m;
    m.type = MsgType::Ping;
    m.time = n;
    return m;
}

void Host(Peer& h, const std::string& code = "123456") {
    SessionAdvert a;
    a.sequenceName = "test.xsq";
    a.hostName = "Unit test host";
    a.deviceKind = "mac";
    std::string error;
    REQUIRE(h.t->Listen(code, a, error));
    CHECK(error.empty());
    CHECK(h.t->LocalPeer() == kLanHostPeer);
}

// Connects `c` to `h` and waits for both sides to know.
PeerId Join(TestLoop& loop, Peer& h, Peer& c, const std::string& code = "123456") {
    size_t before = h.events.size();
    c.t->Connect(LanTest::DirectSessionId(*h.t), code);
    REQUIRE(loop.Until([&] { return c.Joined() && h.events.size() > before; }));
    auto [peer, e] = h.events[before];
    CHECK(e == PeerEvent::Joined);
    CHECK(c.t->LocalPeer() == peer);
    return peer;
}

std::string RandomBytes(size_t n, uint32_t seed) {
    std::mt19937 r(seed);
    std::string s(n, '\0');
    for (auto& ch : s) {
        ch = (char)(r() & 0xFF);
    }
    return s;
}

size_t FilesIn(const std::string& dir) {
    size_t n = 0;
    std::error_code ec;
    for (auto it = std::filesystem::directory_iterator(dir, ec); !ec && it != std::filesystem::directory_iterator(); it.increment(ec)) {
        ++n;
    }
    return n;
}

} // namespace

TEST_CASE("Collab LAN key depends on the code and the salt", "[collab][net]") {
    std::string a = LanTest::DeriveKey("123456", "saltsalt");
    CHECK(a.size() == 32);
    CHECK(a == LanTest::DeriveKey("123456", "saltsalt"));
    CHECK(a != LanTest::DeriveKey("123457", "saltsalt"));
    CHECK(a != LanTest::DeriveKey("123456", "saltsalu"));
    CHECK(LanTransportAvailable());
}

TEST_CASE("Collab LAN host and clients connect and keep message order", "[collab][net]") {
    TestLoop loop;
    Peer h(loop, 0), c1(loop, 1), c2(loop, 2);
    Host(h);
    REQUIRE(loop.Until([&] { return h.t->State() == NetState::Listening; }));
    PeerId p1 = Join(loop, h, c1);
    PeerId p2 = Join(loop, h, c2);
    CHECK(p1 != p2);
    CHECK(p1 != kLanHostPeer);
    CHECK(p1 != kNoPeer);

    const int n = 400;
    for (int i = 0; i < n; ++i) {
        c1.t->Send(kLanHostPeer, Ping(i));
        c2.t->Send(kLanHostPeer, Ping(100000 + i));
        h.t->Send(p1, Ping(200000 + i));
        h.t->Broadcast(Ping(300000 + i));
    }
    REQUIRE(loop.Until([&] { return h.messages.size() == 2 * n && c1.messages.size() == 2 * n && c2.messages.size() == n; }));
    loop.For_ms(100);
    CHECK(h.messages.size() == 2 * n);
    CHECK(c2.messages.size() == n);
    std::map<PeerId, std::vector<uint64_t>> atHost;
    for (auto& [from, m] : h.messages) {
        atHost[from].push_back(m.time);
    }
    std::vector<uint64_t> direct, broadcast;
    for (auto& [from, m] : c1.messages) {
        CHECK(from == kLanHostPeer);
        (m.time >= 300000 ? broadcast : direct).push_back(m.time);
    }
    for (int i = 0; i < n; ++i) {
        REQUIRE(atHost[p1][i] == (uint64_t)i);
        REQUIRE(atHost[p2][i] == (uint64_t)(100000 + i));
        REQUIRE(direct[i] == (uint64_t)(200000 + i));
        REQUIRE(broadcast[i] == (uint64_t)(300000 + i));
        REQUIRE(c2.messages[i].second.time == (uint64_t)(300000 + i));
    }
    // A client leaving cleanly is Left at the host; the other stays.
    c1.Close();
    REQUIRE(loop.Until([&] { return h.Has(p1, PeerEvent::Left) || h.Has(p1, PeerEvent::Lost); }));
    CHECK(h.Has(p1, PeerEvent::Left));
    h.t->Broadcast(Ping(7));
    REQUIRE(loop.Until([&] { return c2.messages.size() == n + 1; }));
    CHECK(c1.callbacksAfterClose == 0);
}

TEST_CASE("Collab LAN refuses a wrong pairing code", "[collab][net]") {
    TestLoop loop;
    Peer h(loop, 0), bad(loop, 1), good(loop, 2);
    Host(h, "424242");
    bad.t->Connect(LanTest::DirectSessionId(*h.t), "424243");
    REQUIRE(loop.Until([&] { return bad.t->State() == NetState::Failed; }));
    REQUIRE_FALSE(bad.states.empty());
    CHECK(bad.states.back().first == NetState::Failed);
    INFO(bad.states.back().second);
    CHECK(bad.states.back().second.find("pairing code") != std::string::npos);
    CHECK(bad.events.empty());
    // Long enough for anything the host might have queued to show up.
    loop.For_ms(300);
    CHECK(h.events.empty());
    CHECK(h.messages.empty());
    // The host still takes someone with the right code.
    PeerId p = Join(loop, h, good, "424242");
    good.t->Send(kLanHostPeer, Ping(5));
    REQUIRE(loop.Until([&] { return h.messages.size() == 1; }));
    CHECK(h.messages[0].first == p);
}

TEST_CASE("Collab LAN host backs off after repeated wrong pairing codes", "[collab][net]") {
    TestLoop loop;
    Peer h(loop, 0);
    Host(h, "111222");
    LanTest::SetPairingBackoff(*h.t, { 3, 1500, 2500 });
    std::vector<std::unique_ptr<Peer>> others;
    auto attempt = [&](const std::string& code) -> Peer& {
        others.push_back(std::make_unique<Peer>(loop, 1));
        Peer& p = *others.back();
        p.t->Connect(LanTest::DirectSessionId(*h.t), code);
        REQUIRE(loop.Until([&] { return p.t->State() == NetState::Failed || p.Joined(); }));
        return p;
    };
    auto failures = [&] { return LanTest::Pairing(*h.t).failedHandshakes; };

    // A good participant joining in between doesn't reset the count.
    Peer& first = attempt("111222");
    CHECK(first.Joined());
    for (int i = 0; i < 2; ++i) {
        CHECK(attempt("999999").t->State() == NetState::Failed);
        REQUIRE(loop.Until([&] { return failures() == i + 1; }));
        CHECK_FALSE(LanTest::Pairing(*h.t).refusing);
    }
    CHECK(attempt("111222").Joined());
    CHECK(attempt("999999").t->State() == NetState::Failed);
    REQUIRE(loop.Until([&] { return failures() == 3; }));
    auto st = LanTest::Pairing(*h.t);
    CHECK(st.refusing);
    CHECK(st.windowMs == 1500);

    // During the pause even the right code is turned away, before TLS.
    size_t hostEvents = h.events.size();
    Peer& refused = attempt("111222");
    CHECK(refused.t->State() == NetState::Failed);
    CHECK(LanTest::Pairing(*h.t).refusedConnections == 1);
    CHECK(failures() == 3);
    loop.For_ms(100);
    CHECK(h.events.size() == hostEvents);
    // Those already in stay connected.
    h.t->Send(first.t->LocalPeer(), Ping(1));
    REQUIRE(loop.Until([&] { return first.messages.size() == 1; }));

    // Once it lapses the right code works again.
    REQUIRE(loop.Until([&] { return !LanTest::Pairing(*h.t).refusing; }, 5000));
    CHECK(attempt("111222").Joined());

    // The next round pauses longer, up to the cap.
    for (int round = 0; round < 2; ++round) {
        int base = failures();
        for (int i = 0; i < 3; ++i) {
            CHECK(attempt("999999").t->State() == NetState::Failed);
            REQUIRE(loop.Until([&] { return failures() == base + i + 1; }));
        }
        st = LanTest::Pairing(*h.t);
        CHECK(st.refusing);
        CHECK(st.windowMs == 2500);
        if (round == 0) {
            REQUIRE(loop.Until([&] { return !LanTest::Pairing(*h.t).refusing; }, 5000));
        }
    }
}

TEST_CASE("Collab LAN blobs share the link without holding up messages", "[collab][net]") {
    TestLoop loop;
    Peer h(loop, 0), c(loop, 1);
    TempDir dir;
    Host(h);
    PeerId p = Join(loop, h, c);

    std::string big = RandomBytes(6u << 20, 1);
    std::string small = RandomBytes(1000, 2);
    std::string path = dir.Write("blob.bin", big);
    h.t->SendBlob(p, 11, path);
    // The transport has its own copy (or the file open): the caller may
    // replace or remove the file at once.
    std::string replacement = dir.Write("replacement.bin", "changed");
    std::filesystem::rename(replacement, path);
    std::filesystem::remove(path);
    for (int i = 0; i < 50; ++i) {
        h.t->Send(p, Ping(i));
    }
    std::string emptyPath = dir.Write("empty.bin", "");
    h.t->SendBlob(p, 12, emptyPath);
    h.t->SendBlob(p, 13, dir.Write("small.bin", small));
    for (int i = 50; i < 100; ++i) {
        h.t->Send(p, Ping(i));
    }
    // And the other way.
    c.t->SendBlob(kLanHostPeer, 99, dir.Write("up.bin", small));

    REQUIRE(loop.Until([&] { return c.blobs.size() == 3 && c.messages.size() == 100 && h.blobs.size() == 1; }, 20000));
    CHECK(c.blobs[0].id == 11);
    CHECK(c.blobs[0].from == kLanHostPeer);
    CHECK(c.blobs[0].content.size() == big.size());
    CHECK(c.blobs[0].content == big);
    // The messages sent after the snapshot didn't wait for all of it.
    CHECK(c.blobs[0].messagesBefore > 0);
    CHECK(c.blobs[1].id == 12);
    CHECK(c.blobs[1].content.empty());
    CHECK(c.blobs[2].id == 13);
    CHECK(c.blobs[2].content == small);
    CHECK(h.blobs[0].from == p);
    CHECK(h.blobs[0].id == 99);
    CHECK(h.blobs[0].content == small);
    for (int i = 0; i < 100; ++i) {
        REQUIRE(c.messages[i].second.time == (uint64_t)i);
    }
    // Received files are gone once the callback returns, and so are the
    // sender's copies once sent.
    for (auto& b : c.blobs) {
        CHECK_FALSE(std::filesystem::exists(b.path));
    }
    CHECK(FilesIn(LanTest::BlobFolder(*c.t)) == 0);
    CHECK(FilesIn(LanTest::BlobFolder(*h.t)) == 0);
}

TEST_CASE("Collab LAN drops a peer that sends something malformed", "[collab][net]") {
    struct Case {
        const char* what;
        std::string bytes;
    };
    std::string hugeLength("\xFF\xFF\xFF\xF0\x02", 5);
    std::vector<Case> cases{
        { "oversized length", hugeLength + "garbage" },
        { "unknown type", Wire::EncodeFrame((Wire::FrameType)0x55, "abcd") },
        { "not a message", Wire::EncodeFrame(Wire::FrameType::Message, "{not json") },
        { "blob data without a start", Wire::BlobDataFrame(3, "abc") },
        { "a second welcome", Wire::WelcomeFrame(9) },
    };
    for (auto& tc : cases) {
        INFO(tc.what);
        TestLoop loop;
        Peer h(loop, 0), rogue(loop, 1), ok(loop, 2);
        Host(h);
        PeerId pr = Join(loop, h, rogue);
        PeerId pk = Join(loop, h, ok);
        LanTest::SendRaw(*rogue.t, kLanHostPeer, tc.bytes);
        REQUIRE(loop.Until([&] { return h.Has(pr, PeerEvent::Lost); }));
        REQUIRE(loop.Until([&] { return rogue.t->State() == NetState::Failed; }));
        CHECK((rogue.Has(kLanHostPeer, PeerEvent::Lost) || rogue.Has(kLanHostPeer, PeerEvent::Left)));
        CHECK(h.messages.empty());
        // The host keeps serving everyone else.
        h.t->Broadcast(Ping(1));
        REQUIRE(loop.Until([&] { return ok.messages.size() == 1; }));
        CHECK_FALSE(h.Has(pk, PeerEvent::Lost));
    }
    // The same garbage from the host drops the host at the client.
    TestLoop loop;
    Peer h(loop, 0), c(loop, 1);
    Host(h);
    PeerId p = Join(loop, h, c);
    LanTest::SendRaw(*h.t, p, hugeLength);
    REQUIRE(loop.Until([&] { return c.t->State() == NetState::Failed; }));
    CHECK(c.Has(kLanHostPeer, PeerEvent::Lost));
}

TEST_CASE("Collab LAN Close with traffic in flight", "[collab][net]") {
    TempDir dir;
    std::string path = dir.Write("blob.bin", RandomBytes(4u << 20, 3));
    for (int which = 0; which < 2; ++which) {
        INFO((which == 0 ? "client closes" : "host closes"));
        TestLoop loop;
        auto h = std::make_unique<Peer>(loop, 0);
        auto c = std::make_unique<Peer>(loop, 1);
        Host(*h);
        PeerId p = Join(loop, *h, *c);
        for (int i = 0; i < 200; ++i) {
            h->t->Send(p, Ping(i));
            c->t->Send(kLanHostPeer, Ping(i));
        }
        h->t->SendBlob(p, 1, path);
        c->t->SendBlob(kLanHostPeer, 2, path);
        loop.For_ms(5);
        Peer& closer = which == 0 ? *c : *h;
        Peer& other = which == 0 ? *h : *c;
        std::string folder = LanTest::BlobFolder(*closer.t);
        closer.Close();
        CHECK(closer.t->State() == NetState::Closed);
        // Sends after Close go nowhere and don't crash.
        closer.t->Send(which == 0 ? kLanHostPeer : p, Ping(1));
        closer.t->SendBlob(which == 0 ? kLanHostPeer : p, 3, path);
        REQUIRE(loop.Until([&] { return other.Has(which == 0 ? p : kLanHostPeer, PeerEvent::Left) || other.Has(which == 0 ? p : kLanHostPeer, PeerEvent::Lost); }));
        loop.For_ms(200);
        CHECK(closer.callbacksAfterClose == 0);
        CHECK(FilesIn(folder) == 0);
        // Destroying both with work still queued is safe too.
        h.reset();
        c.reset();
        loop.For_ms(100);
    }
}

TEST_CASE("Collab LAN sessions are found by browsing and joined by advert id", "[collab][net][bonjour]") {
    TestLoop loop;
    Peer h(loop, 0), c(loop, 1);
    Host(h, "000777");
    auto browser = CreateLanBrowser(loop.For(2));
    REQUIRE(browser);
    std::vector<SessionAdvert> seen;
    std::string browseError;
    browser->onChanged = [&](const std::vector<SessionAdvert>& list) { seen = list; };
    browser->onError = [&](const std::string& why) { browseError = why; };
    browser->Start();
    auto findIt = [&]() -> const SessionAdvert* {
        for (auto& a : seen) {
            if (a.hostName == "Unit test host") {
                return &a;
            }
        }
        return nullptr;
    };
    bool found = loop.Until([&] { return findIt() != nullptr || !browseError.empty(); }, 10000) && findIt() != nullptr;
    INFO("browse error: " << browseError);
    REQUIRE(found);
    SessionAdvert a = *findIt();
    CHECK(a.sequenceName == "test.xsq");
    CHECK(a.deviceKind == "mac");
    CHECK(a.protocol == kProtocolVersion);
    c.t->Connect(a.id, "000777");
    REQUIRE(loop.Until([&] { return c.Joined() && !h.events.empty(); }, 10000));
    // The session disappears from the list once its host closes.
    h.Close();
    REQUIRE(loop.Until([&] { return findIt() == nullptr; }, 10000));
    browser->Stop();
}

// ---------------------------------------------------------------------------
// A host and two clients editing at once over real sockets.
// ---------------------------------------------------------------------------

namespace {

struct NetNode {
    std::unique_ptr<CollabTest::Doc> doc;
    std::unique_ptr<INetTransport> t;
    std::unique_ptr<CollabParticipant> session;
    CollabTest::IdState ids;
    std::vector<std::string> log;
    CollabHost& Host() {
        // Checked: after a host migration a device's session changes kind.
        auto* s = dynamic_cast<CollabHost*>(session.get());
        REQUIRE(s != nullptr);
        return *s;
    }
    CollabClient& Client() {
        // Checked: after a host migration a device's session changes kind.
        auto* s = dynamic_cast<CollabClient*>(session.get());
        REQUIRE(s != nullptr);
        return *s;
    }
};

struct NetScope {
    CollabTest::ActAs ids;
    explicit NetScope(NetNode& n) : ids(n.ids) {
        if (n.doc) {
            n.doc->InstallDirs();
        }
    }
};

class NetSim {
public:
    explicit NetSim(CollabTest::Show& show) : _show(show), _start(Clock::now()) {
        _baseCounter = SeqIds::Counter().load() + 1000;
        loop.wrap = [this](int tag, const std::function<void()>& fn) {
            NetScope s(*nodes[tag]);
            fn();
        };
        loop.idle = [this] { Tick(); };
    }
    ~NetSim() {
        for (auto& n : nodes) {
            NetScope s(*n);
            n->session.reset();
            n->t.reset();
            n->doc.reset();
        }
    }

    uint64_t Now() const { return (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - _start).count(); }

    static SessionOptions Options() {
        SessionOptions o;
        o.verifyHashes = true;
        return o;
    }

    NetNode& AddHost(const std::string& code) {
        NetNode& n = NewNode();
        NetScope s(n);
        n.doc->Open(_show.start);
        n.session = std::make_unique<CollabHost>(*n.t, *n.doc, Options(), Events(n));
        n.Host().Start();
        SessionAdvert a;
        a.hostName = "host";
        std::string error;
        REQUIRE(n.t->Listen(code, a, error));
        return n;
    }

    NetNode& AddClient(const std::string& code) {
        NetNode& n = NewNode();
        n.session = std::make_unique<CollabClient>(*n.t, *n.doc, Options(), Events(n));
        // The app's part: join once the transport says the host is there.
        auto inner = n.t->onPeer;
        NetNode* np = &n;
        n.t->onPeer = [this, np, inner](PeerId p, PeerEvent e) {
            if (e == PeerEvent::Joined) {
                np->session->Tick(Now());
                np->Client().Join(p);
            }
            inner(p, e);
        };
        n.t->Connect(LanTest::DirectSessionId(*nodes[0]->t), code);
        return n;
    }

    void Tick() {
        uint64_t now = Now();
        for (auto& n : nodes) {
            NetScope s(*n);
            n->session->Tick(now);
        }
    }

    bool Quiet() {
        Rev head = nodes[0]->Host().HeadRev();
        for (size_t i = 1; i < nodes.size(); ++i) {
            auto& c = nodes[i]->Client();
            if (c.GetState() != CollabClient::State::Active || c.PendingCount() != 0 || c.AppliedRev() != head) {
                return false;
            }
        }
        return true;
    }

    // Quiet twice, a little apart: nothing was still on the wire.
    bool Quiesce(int ms = 30000) {
        auto end = Clock::now() + std::chrono::milliseconds(ms);
        while (Clock::now() < end) {
            if (!loop.Until([&] { return Quiet(); }, ms)) {
                return false;
            }
            Rev head = nodes[0]->Host().HeadRev();
            loop.For_ms(150);
            if (Quiet() && nodes[0]->Host().HeadRev() == head) {
                return true;
            }
        }
        return false;
    }

    TestLoop loop;
    std::vector<std::unique_ptr<NetNode>> nodes;

private:
    NetNode& NewNode() {
        auto n = std::make_unique<NetNode>();
        n->t = CreateLanTransport(loop.For((int)nodes.size()));
        REQUIRE(n->t);
        n->doc = std::make_unique<CollabTest::Doc>(_show.path, _show.dir.File("net" + std::to_string(nodes.size())));
        n->ids.counter = _baseCounter;
        nodes.push_back(std::move(n));
        return *nodes.back();
    }

    SessionEvents Events(NetNode& n) {
        SessionEvents ev;
        NetNode* np = &n;
        ev.onLog = [np](const std::string& line) { np->log.push_back(line); };
        return ev;
    }

    CollabTest::Show& _show;
    Clock::time_point _start;
    uint64_t _baseCounter;
};

class Editor {
public:
    explicit Editor(uint32_t seed) : _rng(seed) {}

    void Edit(NetNode& n) {
        SequenceElements& seq = n.doc->Sequence();
        std::unique_ptr<SequenceCommand> cmd;
        int op = (int)(_rng() % 100);
        if (op < 35) {
            if (EffectLayer* l = Layer(seq)) {
                int s = (int)(_rng() % 400) * 50;
                int e = s + 50 * (1 + (int)(_rng() % 20));
                if (l->GetRangeIsClearMS(s, e)) {
                    EffectState st;
                    st.startTimeMS = s;
                    st.endTimeMS = e;
                    st.name = l->GetParentElement()->GetType() == ElementType::ELEMENT_TYPE_TIMING ? "w" + std::to_string(_rng() % 50) : "On";
                    cmd = std::make_unique<AddEffectCommand>(LocatorOf(l), st);
                }
            }
        } else if (op < 45) {
            if (Effect* e = AnyEffect(seq)) {
                cmd = std::make_unique<DeleteEffectCommand>(LocatorOf(e));
            }
        } else if (op < 60) {
            if (Effect* e = AnyEffect(seq)) {
                int s = std::max(0, e->GetStartTimeMS() + 50 * ((int)(_rng() % 21) - 10));
                cmd = std::make_unique<MoveEffectCommand>(LocatorOf(e), s, s + (e->GetEndTimeMS() - e->GetStartTimeMS()));
            }
        } else if (op < 85) {
            if (Effect* e = AnyEffect(seq)) {
                SettingsChange st;
                st.keys["E_SLIDER_Net_" + std::string(1, (char)('A' + _rng() % 3))] = std::to_string(_rng() % 100);
                cmd = std::make_unique<ModifyEffectCommand>(LocatorOf(e), std::nullopt, std::move(st), SettingsChange{});
            }
        } else if (op < 95) {
            n.session->Undo();
            return;
        } else {
            n.session->Redo();
            return;
        }
        if (cmd) {
            n.session->Submit(std::move(cmd));
        }
    }

    uint32_t Next() { return _rng(); }

private:
    EffectLayer* Layer(SequenceElements& seq) {
        size_t count = seq.GetElementCount(MASTER_VIEW);
        if (count == 0) {
            return nullptr;
        }
        Element* el = seq.GetElement(_rng() % count, MASTER_VIEW);
        if (el->GetEffectLayerCount() == 0) {
            return nullptr;
        }
        return el->GetEffectLayer((int)(_rng() % el->GetEffectLayerCount()));
    }
    Effect* AnyEffect(SequenceElements& seq) {
        for (int tries = 0; tries < 8; ++tries) {
            EffectLayer* l = Layer(seq);
            if (l != nullptr && l->GetEffectCount() > 0) {
                return l->GetEffect((int)(_rng() % l->GetEffectCount()));
            }
        }
        return nullptr;
    }

    std::mt19937 _rng;
};

} // namespace

TEST_CASE("Collab LAN host and two clients converge over real sockets", "[collab][net]") {
    CollabTest::Show show;
    for (uint32_t seed : { 1u, 2u }) {
        INFO("seed " << seed);
        NetSim sim(show);
        sim.AddHost("314159");
        sim.AddClient("314159");
        sim.AddClient("314159");
        REQUIRE(sim.Quiesce());
        Editor ed(seed);
        for (int step = 0; step < 150; ++step) {
            NetNode& n = *sim.nodes[ed.Next() % sim.nodes.size()];
            if (n.session->IsActive()) {
                NetScope s(n);
                ed.Edit(n);
            }
            sim.loop.For_ms((int)(ed.Next() % 8));
        }
        bool quiet = sim.Quiesce();
        if (!quiet) {
            for (size_t i = 0; i < sim.nodes.size(); ++i) {
                auto& log = sim.nodes[i]->log;
                for (size_t k = log.size() > 5 ? log.size() - 5 : 0; k < log.size(); ++k) {
                    UNSCOPED_INFO("[" << i << "] " << log[k].substr(0, 300));
                }
            }
        }
        REQUIRE(quiet);
        NetNode& h = *sim.nodes[0];
        CHECK(h.Host().HeadRev() > 20);
        std::string hostDesc = DescribeSequence(h.doc->Sequence());
        uint64_t hostHash = DocHasher::FullHash(h.doc->Sequence());
        std::string hostXsq = h.doc->Save();
        for (size_t i = 0; i < sim.nodes.size(); ++i) {
            NetNode& n = *sim.nodes[i];
            INFO("participant " << i);
            CHECK(n.session->HashVerifyFailures() == 0);
            if (i == 0) {
                continue;
            }
            CHECK(n.Client().ResyncCount() == 0);
            CHECK(DescribeSequence(n.doc->Sequence()) == hostDesc);
            CHECK(DocHasher::FullHash(n.doc->Sequence()) == hostHash);
            CHECK(n.doc->Save() == hostXsq);
        }
    }
}

#endif // __APPLE__
