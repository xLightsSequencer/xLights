#include <catch2/catch_test_macros.hpp>

#include "collab/CollabFiles.h"
#include "collab/CollabNet.h"
#include "collab/CollabNetWire.h"

#include <random>
#include <string>
#include <vector>

// The LAN transport over real TLS sockets on loopback: Network.framework on
// Apple platforms, the portable transport (Mbed TLS) on Windows and Linux.
// Building with XL_TEST_LAN_PORTABLE on a Mac runs the same tests against the
// portable transport.

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

TEST_CASE("Collab join addresses become session ids", "[collab][net]") {
    std::string id;
    REQUIRE(SessionIdForAddress("192.168.1.20:5123", id));
    CHECK(id == "addr:192.168.1.20:5123");
    REQUIRE(SessionIdForAddress("  host.local:80 ", id));
    CHECK(id == "addr:host.local:80");
    REQUIRE(SessionIdForAddress("[fe80::1]:65535", id));
    CHECK(id == "addr:[fe80::1]:65535");
    std::string host, port;
    REQUIRE(Wire::SplitHostPort("[::1]:0080", host, port));
    CHECK(host == "[::1]");
    CHECK(port == "80");
    for (const char* bad : { "", "host", "host:", ":80", "host:0", "host:65536", "host:8x", "fe80::1:80", "[fe80::1:80", "a b:80", "host:123456" }) {
        INFO(bad);
        CHECK_FALSE(SessionIdForAddress(bad, id));
    }
    CHECK(Wire::SaltReply(std::string("\x01\x02\x03\x04\x05\x06\x07\x08", 8)) == "xlc-0102030405060708\n");
}

TEST_CASE("Collab a listener that waits for the network after it was ready keeps listening", "[collab][net]") {
    using S = Wire::ListenerState;
    using V = Wire::ListenerVerdict;
    CHECK(Wire::ListenerStateVerdict(S::Ready, false, false) == V::Listening);
    CHECK(Wire::ListenerStateVerdict(S::Ready, false, true) == V::Listening);
    CHECK(Wire::ListenerStateVerdict(S::Failed, true, true) == V::Fail);
    CHECK(Wire::ListenerStateVerdict(S::Failed, true, false) == V::Fail);
    // Before it was ever ready, waiting with an error means it can't listen.
    CHECK(Wire::ListenerStateVerdict(S::Waiting, true, false) == V::Fail);
    CHECK(Wire::ListenerStateVerdict(S::Waiting, false, false) == V::Ignore);
    // After that, a network drop: it comes back to ready by itself.
    CHECK(Wire::ListenerStateVerdict(S::Waiting, true, true) == V::Ignore);
    CHECK(Wire::ListenerStateVerdict(S::Other, true, true) == V::Ignore);
}

TEST_CASE("Collab a service registration is freed once and never while its owner holds it", "[collab][net]") {
    using A = Wire::RegistrationLifetime::Action;
    SECTION("registered, then stopped") {
        Wire::RegistrationLifetime r;
        CHECK(r.Completed(true) == A::None);
        CHECK(r.Stop() == A::Deregister);
        CHECK(r.Completed(true) == A::Free);
    }
    SECTION("stopped while registering") {
        Wire::RegistrationLifetime r;
        CHECK(r.Stop() == A::None);
        CHECK(r.Completed(true) == A::Deregister);
        CHECK(r.Completed(true) == A::Free);
    }
    SECTION("fails after the owner has it") {
        Wire::RegistrationLifetime r;
        // The owner still holds it: it must stay until the owner lets go.
        CHECK(r.Completed(false) == A::None);
        CHECK(r.Failed());
        CHECK(r.Stop() == A::Free);
    }
    SECTION("fails after the owner let go") {
        Wire::RegistrationLifetime r;
        CHECK(r.Stop() == A::None);
        CHECK(r.Completed(false) == A::Free);
    }
    SECTION("the deregistration can't be sent") {
        Wire::RegistrationLifetime r;
        CHECK(r.Completed(true) == A::None);
        CHECK(r.Stop() == A::Deregister);
        CHECK(r.DeregisterFailed() == A::Free);
    }
    SECTION("a stray completion frees nothing the owner holds") {
        Wire::RegistrationLifetime r;
        CHECK(r.Completed(true) == A::None);
        CHECK(r.Completed(true) == A::None);
        CHECK(r.Completed(false) == A::None);
        CHECK(r.Stop() == A::Deregister);
    }
}

TEST_CASE("Collab callback contexts are found by number until removed", "[collab][net]") {
    Wire::CallbackRegistry<int> reg;
    uint64_t a = reg.Add(std::make_shared<int>(1));
    uint64_t b = reg.Add(std::make_shared<int>(2));
    CHECK(a != b);
    REQUIRE(reg.Find(a));
    CHECK(*reg.Find(a) == 1);
    reg.Remove(a);
    // A callback that comes after the cancel finds nothing.
    CHECK_FALSE(reg.Find(a));
    CHECK(*reg.Find(b) == 2);
    // Numbers are never handed out again.
    uint64_t c = reg.Add(std::make_shared<int>(3));
    CHECK(c != a);
    CHECK(c != b);
    CHECK(reg.Size() == 2);
}

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

#if defined(XL_TEST_LAN_PORTABLE) || !defined(__APPLE__)
#define XL_TEST_PORTABLE_TRANSPORT 1
#endif

#if defined(XL_TEST_LAN_PORTABLE)
namespace Lan = Collab::Portable;
std::unique_ptr<INetTransport> NewLanTransport(Dispatcher d) {
    return Collab::Portable::CreateTransport(std::move(d));
}
std::unique_ptr<ISessionBrowser> NewLanBrowser(Dispatcher d) {
    return Collab::Portable::CreateBrowser(std::move(d));
}
#else
namespace Lan = Collab::LanTest;
std::unique_ptr<INetTransport> NewLanTransport(Dispatcher d) {
    return CreateLanTransport(std::move(d));
}
std::unique_ptr<ISessionBrowser> NewLanBrowser(Dispatcher d) {
    return CreateLanBrowser(std::move(d));
}
#endif

// A Mac build against the portable transport also has the Apple one, so
// each can be checked against the other.
#if defined(__APPLE__) && defined(XL_TEST_LAN_PORTABLE)
#define XL_TEST_INTEROP 1
#endif

std::unique_ptr<INetTransport> NewTransport(Dispatcher d, bool apple) {
#ifdef XL_TEST_INTEROP
    if (apple) {
        return CreateLanTransport(std::move(d));
    }
#endif
    (void)apple;
    return NewLanTransport(std::move(d));
}

std::string DirectId(INetTransport& host) {
#ifdef XL_TEST_INTEROP
    std::string id = Collab::LanTest::DirectSessionId(host);
    if (!id.empty()) {
        return id;
    }
#endif
    return Lan::DirectSessionId(host);
}

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

    Peer(TestLoop& loop, int tag, bool apple = false) {
        t = NewTransport(loop.For(tag), apple);
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
    c.t->Connect(DirectId(*h.t), code);
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
    std::string a = Lan::DeriveKey("123456", "saltsalt");
    CHECK(a.size() == 32);
    // PBKDF2-HMAC-SHA256, 100000 rounds, salt "xlights-collab-v1:" + salt:
    // every platform's transport must derive the same key or they can't pair.
    CHECK(Wire::HexOf(a) == "002106699384dbb43d5910c1213adfe49f1318e85daae2d4dbd3ee93d3549795");
    std::string salt;
    REQUIRE(Wire::SaltFromHex("0011223344556677", salt));
    CHECK(Wire::HexOf(Lan::DeriveKey("000777", salt)) == "0d9a32f7244c9799f21a54ec5827eba1c6d613ce0998d989e95245e49084b7b1");
    CHECK(a == Lan::DeriveKey("123456", "saltsalt"));
    CHECK(a != Lan::DeriveKey("123457", "saltsalt"));
    CHECK(a != Lan::DeriveKey("123456", "saltsalu"));
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
    bad.t->Connect(Lan::DirectSessionId(*h.t), "424243");
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
    Lan::SetPairingBackoff(*h.t, { 3, 1500, 2500 });
    std::vector<std::unique_ptr<Peer>> others;
    auto attempt = [&](const std::string& code) -> Peer& {
        others.push_back(std::make_unique<Peer>(loop, 1));
        Peer& p = *others.back();
        p.t->Connect(Lan::DirectSessionId(*h.t), code);
        REQUIRE(loop.Until([&] { return p.t->State() == NetState::Failed || p.Joined(); }));
        return p;
    };
    auto failures = [&] { return Lan::Pairing(*h.t).failedHandshakes; };

    // A good participant joining in between doesn't reset the count.
    Peer& first = attempt("111222");
    CHECK(first.Joined());
    for (int i = 0; i < 2; ++i) {
        CHECK(attempt("999999").t->State() == NetState::Failed);
        REQUIRE(loop.Until([&] { return failures() == i + 1; }));
        CHECK_FALSE(Lan::Pairing(*h.t).refusing);
    }
    CHECK(attempt("111222").Joined());
    CHECK(attempt("999999").t->State() == NetState::Failed);
    REQUIRE(loop.Until([&] { return failures() == 3; }));
    auto st = Lan::Pairing(*h.t);
    CHECK(st.refusing);
    CHECK(st.windowMs == 1500);

    // During the pause even the right code is turned away, before TLS.
    size_t hostEvents = h.events.size();
    Peer& refused = attempt("111222");
    CHECK(refused.t->State() == NetState::Failed);
    CHECK(Lan::Pairing(*h.t).refusedConnections == 1);
    CHECK(failures() == 3);
    loop.For_ms(100);
    CHECK(h.events.size() == hostEvents);
    // Those already in stay connected.
    h.t->Send(first.t->LocalPeer(), Ping(1));
    REQUIRE(loop.Until([&] { return first.messages.size() == 1; }));

    // Once it lapses the right code works again.
    REQUIRE(loop.Until([&] { return !Lan::Pairing(*h.t).refusing; }, 5000));
    CHECK(attempt("111222").Joined());

    // The next round pauses longer, up to the cap.
    for (int round = 0; round < 2; ++round) {
        int base = failures();
        for (int i = 0; i < 3; ++i) {
            CHECK(attempt("999999").t->State() == NetState::Failed);
            REQUIRE(loop.Until([&] { return failures() == base + i + 1; }));
        }
        st = Lan::Pairing(*h.t);
        CHECK(st.refusing);
        CHECK(st.windowMs == 2500);
        if (round == 0) {
            REQUIRE(loop.Until([&] { return !Lan::Pairing(*h.t).refusing; }, 5000));
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
    CHECK(FilesIn(Lan::BlobFolder(*c.t)) == 0);
    CHECK(FilesIn(Lan::BlobFolder(*h.t)) == 0);
}

TEST_CASE("Collab LAN sending a large file doesn't hold up the caller", "[collab][net]") {
    TestLoop loop;
    Peer h(loop, 0), c(loop, 1);
    TempDir dir;
    Host(h);
    PeerId p = Join(loop, h, c);

    std::string chunk = RandomBytes(1u << 20, 3);
    std::string path = (dir.path / "large.bin").string();
    {
        std::ofstream f(path, std::ios::binary);
        for (int i = 0; i < 128; ++i) {
            chunk[0] = (char)i;
            f.write(chunk.data(), (std::streamsize)chunk.size());
        }
    }
    uint64_t size = std::filesystem::file_size(path);
    // What copying it takes here (it is in the page cache now).
    auto t0 = Clock::now();
    std::filesystem::copy_file(path, (dir.path / "baseline.bin").string());
    double copyMs = std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
    std::filesystem::remove(dir.path / "baseline.bin");

    std::vector<std::pair<BlobId, bool>> done;
    h.t->onBlobDone = [&](PeerId to, BlobId id, bool sent) {
        CHECK(to == p);
        done.emplace_back(id, sent);
    };
    std::vector<uint64_t> progress;
    c.t->onBlobProgress = [&](PeerId from, BlobId id, uint64_t received, uint64_t total) {
        CHECK(from == kLanHostPeer);
        CHECK(id == 21);
        CHECK(total == size);
        progress.push_back(received);
    };
    auto t1 = Clock::now();
    h.t->SendBlob(p, 21, path);
    double sendMs = std::chrono::duration<double, std::milli>(Clock::now() - t1).count();
    // The caller may delete it at once.
    std::filesystem::remove(path);
    INFO("copy " << copyMs << " ms, SendBlob " << sendMs << " ms");
    // A copy made on the caller's thread takes as long as copying the file.
    // Where copies are clones, there is nothing to measure.
    if (copyMs >= 20) {
        CHECK(sendMs * 4 < copyMs);
    }
    REQUIRE(loop.Until([&] { return c.blobs.size() == 1 && done.size() == 1; }, 60000));
    CHECK(c.blobs[0].id == 21);
    REQUIRE(c.blobs[0].content.size() == size);
    for (int i = 0; i < 128; ++i) {
        chunk[0] = (char)i;
        REQUIRE(c.blobs[0].content.compare((size_t)i << 20, chunk.size(), chunk) == 0);
    }
    CHECK(done[0] == std::make_pair(BlobId(21), true));
    // Progress came along the way, at least every kBlobProgressBytes.
    REQUIRE(progress.size() >= size / kBlobProgressBytes);
    CHECK(progress.front() == 0);
    CHECK(std::is_sorted(progress.begin(), progress.end()));
    CHECK(FilesIn(Lan::BlobFolder(*h.t)) == 0);
}

TEST_CASE("Collab an incoming file that can't be written is a disk error, not the peer's", "[collab][net]") {
    TempDir dir;
    using R = Files::IncomingBlob::Result;
    {
        Files::IncomingBlob in;
        std::string missing = (dir.path / "gone" / "in-1").string();
        CHECK(in.Start(missing, 5, 6, 100) == R::Ok);
        CHECK(in.Data(5, "abc") == R::Ok);
        CHECK(in.Data(5, "def") == R::Ok);
        CHECK(in.Received() == 6);
        std::string path, why;
        CHECK(in.End(5, path, why) == R::Failed);
        CHECK(path.empty());
        CHECK(why.find("Couldn't save a file") != std::string::npos);
        CHECK_FALSE(in.Active());
        // The next one works.
        std::string ok = (dir.path / "in-2").string();
        CHECK(in.Start(ok, 6, 2, 100) == R::Ok);
        CHECK(in.Data(6, "hi") == R::Ok);
        CHECK(in.End(6, path, why) == R::Received);
        CHECK(path == ok);
        CHECK(CollabTest::ReadFile(path) == "hi");
    }
    {
        // What the peer gets wrong is still Bad.
        Files::IncomingBlob in;
        std::string p = (dir.path / "in-3").string();
        CHECK(in.Start(p, 7, 1000, 100) == R::Bad); // larger than allowed
        CHECK(in.Data(7, "x") == R::Bad);           // no start
        CHECK(in.Start(p, 7, 3, 100) == R::Ok);
        CHECK(in.Start(p, 8, 3, 100) == R::Bad);   // a second at once
        CHECK(in.Data(8, "x") == R::Bad);           // another id
        CHECK(in.Data(7, "toolong") == R::Bad);     // more than announced
        std::string path, why;
        CHECK(in.End(7, path, why) == R::Bad);      // short
        in.Abort();
        CHECK_FALSE(std::filesystem::exists(p));
    }
}

TEST_CASE("Collab LAN a file that can't be cloned is copied off the caller's thread", "[collab][net]") {
    TestLoop loop;
    Peer h(loop, 0), c(loop, 1);
    TempDir dir;
    Host(h);
    PeerId p = Join(loop, h, c);
    std::string content = RandomBytes(256u << 10, 9);
    std::string path = dir.Write("file.bin", content);
    // As across volumes: no clone, and the copy takes a while.
    std::atomic<bool> copied{ false };
    std::thread::id copier;
    Files::Test::noClone = true;
    Files::Test::beforeCopy = [&] {
        copier = std::this_thread::get_id();
        std::this_thread::sleep_for(std::chrono::milliseconds(600));
        copied = true;
    };
    auto t0 = Clock::now();
    h.t->SendBlob(p, 41, path);
    double sendMs = std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
    std::filesystem::remove(path);
    bool ranThrough = loop.Until([&] { return copied.load(); }, 5000);
    REQUIRE(loop.Until([&] { return c.blobs.size() == 1; }, 20000));
    Files::Test::beforeCopy = nullptr;
    Files::Test::noClone = false;
    INFO("SendBlob " << sendMs << " ms");
    CHECK(ranThrough);
    CHECK(copier != std::this_thread::get_id());
    CHECK(sendMs < 300);
    CHECK(c.blobs[0].content == content);
}

TEST_CASE("Collab LAN a file that can't be saved doesn't drop the connection", "[collab][net]") {
    TestLoop loop;
    Peer h(loop, 0), c(loop, 1);
    TempDir dir;
    Host(h);
    PeerId p = Join(loop, h, c);
    std::vector<std::pair<BlobId, bool>> done;
    h.t->onBlobDone = [&](PeerId, BlobId id, bool sent) { done.emplace_back(id, sent); };
    // Where incoming files go is gone, as on a full or failing disk.
    std::string folder = Lan::BlobFolder(*c.t);
    REQUIRE_FALSE(folder.empty());
    std::error_code ec;
    std::filesystem::remove_all(folder, ec);
    h.t->SendBlob(p, 51, dir.Write("a.bin", RandomBytes(3u << 20, 5)));
    REQUIRE(loop.Until([&] { return done.size() == 1; }, 20000));
    h.t->Send(p, Ping(7));
    REQUIRE(loop.Until([&] { return !c.messages.empty(); }, 10000));
    CHECK(c.messages.back().second.time == 7);
    CHECK(c.blobs.empty());
    CHECK(c.Joined());
    CHECK_FALSE(h.Has(p, PeerEvent::Lost));
    CHECK_FALSE(c.Has(kLanHostPeer, PeerEvent::Lost));
    // Once the folder is back, files arrive again on the same connection.
    std::filesystem::create_directories(folder, ec);
    h.t->SendBlob(p, 52, dir.Write("b.bin", "second"));
    REQUIRE(loop.Until([&] { return c.blobs.size() == 1; }, 20000));
    CHECK(c.blobs[0].id == 52);
    CHECK(c.blobs[0].content == "second");
}

#if defined(XL_TEST_PORTABLE_TRANSPORT)
TEST_CASE("Collab LAN closing a join by address doesn't wait for the name lookup", "[collab][net]") {
    TestLoop loop;
    std::atomic<bool> started{ false };
    std::atomic<bool> release{ false };
    Collab::Portable::SetBeforeLookup([&](const std::string&) {
        started = true;
        // A lookup nobody answers: up to a few seconds.
        for (int i = 0; i < 300 && !release.load(); ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });
    {
        Peer c(loop, 1);
        std::string id;
        REQUIRE(SessionIdForAddress("slow-host.invalid:5123", id));
        c.t->Connect(id, "123456");
        REQUIRE(loop.Until([&] { return started.load(); }, 5000));
        auto t0 = Clock::now();
        c.Close();
        double closeMs = std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
        INFO("Close " << closeMs << " ms");
        CHECK(closeMs < 1000);
    }
    release = true;
    loop.For_ms(200);
    Collab::Portable::SetBeforeLookup(nullptr);
}
#endif

TEST_CASE("Collab LAN a message too large to send is reported to the sender", "[collab][net]") {
    TestLoop loop;
    Peer h(loop, 0), c(loop, 1);
    Host(h);
    PeerId p = Join(loop, h, c);
    std::vector<std::pair<PeerId, Message>> failed;
    h.t->onSendFailed = [&](PeerId to, const Message& m, const std::string& reason) {
        CHECK_FALSE(reason.empty());
        failed.emplace_back(to, m);
    };
    Message big;
    big.type = MsgType::Commit;
    big.rev = 7;
    big.clientSeq = 3;
    big.command = std::string(kMaxMessageBytes + 1, 'x');
    h.t->Send(p, big);
    h.t->Broadcast(big);
    h.t->Send(p, Ping(1));
    REQUIRE(loop.Until([&] { return failed.size() == 2 && c.messages.size() == 1; }));
    CHECK(failed[0].first == p);
    CHECK(failed[0].second.type == MsgType::Commit);
    CHECK(failed[0].second.rev == 7);
    CHECK(failed[0].second.clientSeq == 3);
    CHECK(failed[1].first == kNoPeer);
    // Nothing of it reached the other side, and the connection is fine.
    loop.For_ms(100);
    REQUIRE(c.messages.size() == 1);
    CHECK(c.messages[0].second.type == MsgType::Ping);
}

TEST_CASE("Collab LAN a blob to a peer that goes away is reported as not sent", "[collab][net]") {
    TestLoop loop;
    Peer h(loop, 0), c(loop, 1);
    TempDir dir;
    Host(h);
    PeerId p = Join(loop, h, c);
    std::vector<std::pair<BlobId, bool>> done;
    h.t->onBlobDone = [&](PeerId to, BlobId id, bool sent) {
        CHECK(to == p);
        done.emplace_back(id, sent);
    };
    std::string big = RandomBytes(32u << 20, 4);
    h.t->SendBlob(p, 31, dir.Write("big.bin", big));
    // It goes before all of it can have arrived.
    REQUIRE(loop.Until([&] { return std::filesystem::exists(Lan::BlobFolder(*c.t)) && FilesIn(Lan::BlobFolder(*c.t)) > 0; }, 20000));
    c.Close();
    REQUIRE(loop.Until([&] { return !done.empty(); }, 20000));
    REQUIRE(done.size() == 1);
    CHECK(done[0].first == 31);
    // Not sent, unless every byte had already gone out.
    CHECK((done[0].second == false || c.blobs.size() == 1));
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
        Lan::SendRaw(*rogue.t, kLanHostPeer, tc.bytes);
        REQUIRE(loop.Until([&] { return h.Has(pr, PeerEvent::Lost); }));
        // The state callback comes after the peer event, so once it has run so has the event.
        REQUIRE(loop.Until([&] { return !rogue.states.empty() && rogue.states.back().first == NetState::Failed; }));
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
    Lan::SendRaw(*h.t, p, hugeLength);
    REQUIRE(loop.Until([&] { return !c.states.empty() && c.states.back().first == NetState::Failed; }));
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
        std::string folder = Lan::BlobFolder(*closer.t);
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
    auto browser = NewLanBrowser(loop.For(2));
    REQUIRE(browser);
    std::vector<SessionAdvert> seen;
    std::string browseError;
    browser->onChanged = [&](const std::vector<SessionAdvert>& list) { seen = list; };
    browser->onError = [&](const std::string& why) { browseError = why; };
    browser->Start();
    // This host's advert, not one a host from an earlier test left behind:
    // its service name carries the host's salt.
    std::string direct = DirectId(*h.t);
    std::string mine = std::string(Wire::kServicePrefix) + direct.substr(direct.rfind(':') + 1);
    auto findIt = [&]() -> const SessionAdvert* {
        for (auto& a : seen) {
            // The TXT record can arrive after the name; wait for both.
            if (a.id.rfind(mine, 0) == 0 && a.hostName == "Unit test host") {
                return &a;
            }
        }
        return nullptr;
    };
    bool found = loop.Until([&] { return findIt() != nullptr || !browseError.empty(); }, 10000) && findIt() != nullptr;
#ifdef XL_TEST_PORTABLE_TRANSPORT
    if (!found && !Portable::Advertising(*h.t)) {
        // No DNS-SD service on this machine (Linux without avahi-daemon, a
        // CI container): joining by address is the way in there.
        SKIP("no DNS-SD service to advertise with: " << browseError);
    }
#endif
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

    NetNode& AddHost(const std::string& code, bool apple = false) {
        NetNode& n = NewNode(apple);
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

    NetNode& AddClient(const std::string& code, bool apple = false) {
        NetNode& n = NewNode(apple);
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
        n.t->Connect(DirectId(*nodes[0]->t), code);
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
    NetNode& NewNode(bool apple) {
        auto n = std::make_unique<NetNode>();
        n->t = NewTransport(loop.For((int)nodes.size()), apple);
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

// Random edits from every participant at once; afterwards every copy must
// be the host's, byte for byte.
void EditTogether(NetSim& sim, uint32_t seed) {
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

} // namespace

TEST_CASE("Collab LAN host and two clients converge over real sockets", "[collab][net]") {
    CollabTest::Show show;
    for (uint32_t seed : { 1u, 2u }) {
        INFO("seed " << seed);
        NetSim sim(show);
        sim.AddHost("314159");
        sim.AddClient("314159");
        sim.AddClient("314159");
        EditTogether(sim, seed);
    }
}


TEST_CASE("Collab LAN a successor takes over with the same pairing code under a derived name", "[collab][net][migrate]") {
    TestLoop loop;
    Peer h(loop, 0), a(loop, 1), b(loop, 2);
    Host(h, "424242");
    PeerId pa = Join(loop, h, a, "424242");
    Join(loop, h, b, "424242");
    // The session id everyone joined: the host's service name.
    std::string direct = DirectId(*h.t);
    std::string sessionId = std::string(Wire::kServicePrefix) + direct.substr(direct.rfind(':') + 1);
    std::string hostSalt;
    REQUIRE(Wire::SaltFromServiceName(sessionId, hostSalt));
    h.Close();
    // The peer event, not just the state: the state changes before the
    // callbacks have been delivered.
    auto hostGone = [&](Peer& p) { return p.Has(kLanHostPeer, PeerEvent::Left) || p.Has(kLanHostPeer, PeerEvent::Lost); };
    REQUIRE(loop.Until([&] { return hostGone(a) && hostGone(b); }));
    CHECK(pa != kLanHostPeer);

    // Joined by a direct id: no successor name can be derived from it, so
    // the session can't move (this test derives one by hand below).
    CHECK_FALSE(a.t->SupportsMigration());
    CHECK(h.t->SupportsMigration());
    Succession s;
    s.sessionId = sessionId;
    s.pairingCode = "424242";
    s.successor = 2;
    s.epoch = 1;
    std::string expected = a.t->SuccessorSessionId(s);
    CHECK(expected == b.t->SuccessorSessionId(s));
    CHECK(expected != sessionId);
    std::string salt;
    REQUIRE(Wire::SaltFromServiceName(expected, salt));
    CHECK(salt != hostSalt);
    SessionAdvert ad;
    ad.sequenceName = "test.xsq";
    ad.hostName = "Successor";
    std::string error;
    size_t aStates = a.states.size();
    REQUIRE(a.t->ListenAsSuccessor(s, ad, error));
    CHECK(error.empty());
    CHECK(a.t->LocalPeer() == kLanHostPeer);
    REQUIRE(loop.Until([&] { return a.t->State() == NetState::Listening; }));
    // It listens under the derived salt (what Bonjour would advertise).
    std::string succDirect = DirectId(*a.t);
    CHECK(succDirect.substr(succDirect.rfind(':') + 1) == Wire::HexOf(salt));
    // The old connection made no callbacks once replaced.
    loop.For_ms(100);
    for (size_t i = aStates; i < a.states.size(); ++i) {
        CHECK(a.states[i].first != NetState::Failed);
    }

    // b reaches it with the session's code; a wrong one is refused.
    Peer wrong(loop, 3);
    wrong.t->Connect(succDirect, "000000");
    REQUIRE(loop.Until([&] { return wrong.t->State() == NetState::Failed; }));
    size_t before = a.events.size();
    b.t->Close();
    Peer b2(loop, 4);
    b2.t->Connect(succDirect, "424242");
    REQUIRE(loop.Until([&] { return b2.Joined() && a.events.size() > before; }));
    PeerId pb = a.events[before].first;
    a.t->Send(pb, Ping(11));
    b2.t->Send(kLanHostPeer, Ping(12));
    REQUIRE(loop.Until([&] { return !b2.messages.empty() && !a.messages.empty(); }));
    CHECK(b2.messages.back().second.time == 11);
    CHECK(a.messages.back().second.time == 12);

    // A session id with no salt in it can't be moved.
    Succession bad = s;
    bad.sessionId = direct;
    CHECK(a.t->SuccessorSessionId(bad).empty());
    Peer c(loop, 5);
    std::string err2;
    CHECK_FALSE(c.t->ListenAsSuccessor(bad, ad, err2));
    CHECK_FALSE(err2.empty());
}

#ifdef XL_TEST_PORTABLE_TRANSPORT

TEST_CASE("Collab LAN connections use keepalive and no delay", "[collab][net]") {
    TestLoop loop;
    Peer h(loop, 0), c(loop, 1);
    Host(h);
    PeerId p = Join(loop, h, c);
    for (auto [t, peer] : { std::make_pair(h.t.get(), p), std::make_pair(c.t.get(), kLanHostPeer) }) {
        Portable::SocketOptions o;
        REQUIRE(Portable::PeerSocketOptions(*t, peer, o));
        CHECK(o.noDelay);
        CHECK(o.keepAlive);
#ifndef _WIN32
        // Windows can only report these from 10 1709 on; the Mac and Linux
        // always can. A peer that vanishes is dropped in about 25 seconds.
        CHECK(o.idleSec == 10);
        CHECK(o.intervalSec == 5);
        CHECK(o.count == 3);
#endif
    }
}

TEST_CASE("Collab LAN clients join by address", "[collab][net]") {
    TestLoop loop;
    Peer h(loop, 0), c(loop, 1), bad(loop, 2);
    Host(h, "246810");
    auto addrs = h.t->JoinAddresses();
    // Whatever the machine's addresses, loopback reaches the same listener.
    std::string direct = Lan::DirectSessionId(*h.t);
    std::string port = direct.substr(direct.find(':', 4) + 1);
    port = port.substr(0, port.find(':'));
    for (auto& a : addrs) {
        CHECK(a.substr(a.rfind(':') + 1) == port);
    }
    std::string id;
    REQUIRE(SessionIdForAddress("127.0.0.1:" + port, id));

    // The wrong code still fails after the salt exchange, and counts.
    bad.t->Connect(id, "246811");
    REQUIRE(loop.Until([&] { return !bad.states.empty() && bad.states.back().first == NetState::Failed; }));
    CHECK(bad.states.back().second.find("pairing code") != std::string::npos);
    REQUIRE(loop.Until([&] { return Lan::Pairing(*h.t).failedHandshakes == 1; }));

    size_t before = h.events.size();
    c.t->Connect(id, "246810");
    REQUIRE(loop.Until([&] { return c.Joined() && h.events.size() > before; }));
    PeerId p = h.events[before].first;
    c.t->Send(kLanHostPeer, Ping(3));
    h.t->Send(p, Ping(4));
    REQUIRE(loop.Until([&] { return h.messages.size() == 1 && c.messages.size() == 1; }));
    CHECK(h.messages[0].second.time == 3);
    CHECK(c.messages[0].second.time == 4);
    // A session joined by address can't move to another host: no successor
    // name can be derived, and this client couldn't browse for one. Saying so
    // ends it at once when the host goes, instead of waiting on candidates.
    CHECK(h.t->SupportsMigration());
    CHECK_FALSE(c.t->SupportsMigration());
    // One found under its service name can.
    Peer named(loop, 3);
    named.t->Connect(Wire::ServiceNameFor(Wire::NewSalt()), "246810");
    CHECK(named.t->SupportsMigration());
}

#ifdef XL_TEST_PORTABLE_TRANSPORT
TEST_CASE("Collab LAN a TLS setup that fails part way cleans up", "[collab][net]") {
    CHECK(Collab::Portable::TlsSetupFailureCleansUp());
}
#endif

TEST_CASE("Collab LAN join by address fails cleanly when nobody is there", "[collab][net]") {
    TestLoop loop;
    Peer h(loop, 0);
    Host(h);
    std::string direct = Lan::DirectSessionId(*h.t);
    std::string port = direct.substr(direct.find(':', 4) + 1);
    port = port.substr(0, port.find(':'));
    h.Close();
    {
        Peer c(loop, 1);
        std::string id;
        REQUIRE(SessionIdForAddress("127.0.0.1:" + port, id));
        c.t->Connect(id, "123456");
        REQUIRE(loop.Until([&] { return !c.states.empty() && c.states.back().first == NetState::Failed; }));
        CHECK(c.events.empty());
        CHECK(c.states.back().second == "The host is not accepting connections.");
    }
    Peer c(loop, 1);
    c.t->Connect("addr:not an address", "123456");
    REQUIRE(loop.Until([&] { return !c.states.empty() && c.states.back().first == NetState::Failed; }));
    CHECK(c.states.back().second == "That is not an address xLights can join.");
}

TEST_CASE("Collab LAN a host that closes at once leaves no advert behind", "[collab][net][bonjour]") {
    TestLoop loop;
    auto browser = NewLanBrowser(loop.For(2));
    REQUIRE(browser);
    std::vector<SessionAdvert> seen;
    browser->onChanged = [&](const std::vector<SessionAdvert>& list) { seen = list; };
    browser->Start();
    std::string mine;
    {
        Peer h(loop, 0);
        Host(h, "000999");
        if (!Portable::Advertising(*h.t)) {
            SKIP("no DNS-SD service to advertise with");
        }
        std::string direct = DirectId(*h.t);
        mine = std::string(Wire::kServicePrefix) + direct.substr(direct.rfind(':') + 1);
        // Closed while the name may still be being probed on the network.
        h.Close();
    }
    // A registration still in progress finishes and is then withdrawn; it
    // may show up briefly, but must not stay.
    auto listed = [&] {
        for (auto& a : seen) {
            if (a.id.rfind(mine, 0) == 0) {
                return true;
            }
        }
        return false;
    };
    loop.For_ms(4000);
    CHECK(loop.Until([&] { return !listed(); }, 12000));
    browser->Stop();
}

#endif // XL_TEST_PORTABLE_TRANSPORT

#ifdef XL_TEST_INTEROP

TEST_CASE("Collab LAN Apple and portable transports pair both ways", "[collab][net][interop]") {
    TempDir dir;
    std::string big = RandomBytes(3u << 20, 9);
    std::string path = dir.Write("blob.bin", big);
    for (bool appleHost : { true, false }) {
        INFO((appleHost ? "Apple host, portable client" : "portable host, Apple client"));
        TestLoop loop;
        Peer h(loop, 0, appleHost), c(loop, 1, !appleHost), bad(loop, 2, !appleHost);
        Host(h, "135790");
        bad.t->Connect(DirectId(*h.t), "135791");
        REQUIRE(loop.Until([&] { return !bad.states.empty() && bad.states.back().first == NetState::Failed; }));
        INFO(bad.states.back().second);
        CHECK(bad.states.back().second.find("pairing code") != std::string::npos);
        loop.For_ms(200);
        CHECK(h.events.empty());

        PeerId p = Join(loop, h, c, "135790");
        const int n = 200;
        for (int i = 0; i < n; ++i) {
            c.t->Send(kLanHostPeer, Ping(i));
            h.t->Send(p, Ping(1000 + i));
            h.t->Broadcast(Ping(5000 + i));
        }
        h.t->SendBlob(p, 21, path);
        c.t->SendBlob(kLanHostPeer, 22, path);
        bool all = loop.Until([&] { return h.messages.size() == n && c.messages.size() == 2 * n && c.blobs.size() == 1 && h.blobs.size() == 1; }, 20000);
        INFO("host got " << h.messages.size() << " messages, " << h.blobs.size() << " blobs; client got " << c.messages.size() << " messages, " << c.blobs.size() << " blobs");
        REQUIRE(all);
        for (int i = 0; i < n; ++i) {
            REQUIRE(h.messages[i].second.time == (uint64_t)i);
        }
        CHECK(c.blobs[0].id == 21);
        CHECK(c.blobs[0].content == big);
        CHECK(h.blobs[0].id == 22);
        CHECK(h.blobs[0].from == p);
        CHECK(h.blobs[0].content == big);
        c.Close();
        REQUIRE(loop.Until([&] { return h.Has(p, PeerEvent::Left) || h.Has(p, PeerEvent::Lost); }));
        CHECK(h.Has(p, PeerEvent::Left));
    }
}

TEST_CASE("Collab LAN mixed Apple and portable sessions converge", "[collab][net][interop]") {
    CollabTest::Show show;
    for (bool appleHost : { true, false }) {
        INFO((appleHost ? "Apple host" : "portable host"));
        NetSim sim(show);
        sim.AddHost("271828", appleHost);
        sim.AddClient("271828", !appleHost);
        sim.AddClient("271828", appleHost);
        EditTogether(sim, appleHost ? 3u : 4u);
    }
}

TEST_CASE("Collab LAN Apple and portable hosts are found by each other's browsers", "[collab][net][interop][bonjour]") {
    for (bool appleHost : { true, false }) {
        INFO((appleHost ? "Apple host, portable browser" : "portable host, Apple browser"));
        TestLoop loop;
        Peer h(loop, 0, appleHost), c(loop, 1, !appleHost);
        Host(h, "112358");
        auto browser = appleHost ? NewLanBrowser(loop.For(2)) : CreateLanBrowser(loop.For(2));
        REQUIRE(browser);
        std::vector<SessionAdvert> seen;
        std::string err;
        browser->onChanged = [&](const std::vector<SessionAdvert>& list) { seen = list; };
        browser->onError = [&](const std::string& why) { err = why; };
        browser->Start();
        std::string direct = DirectId(*h.t);
        std::string mine = std::string(Wire::kServicePrefix) + direct.substr(direct.rfind(':') + 1);
        auto findIt = [&]() -> const SessionAdvert* {
            for (auto& a : seen) {
                if (a.id.rfind(mine, 0) == 0 && a.hostName == "Unit test host") {
                    return &a;
                }
            }
            return nullptr;
        };
        INFO("browse error: " << err);
        REQUIRE(loop.Until([&] { return findIt() != nullptr; }, 10000));
        SessionAdvert a = *findIt();
        CHECK(a.sequenceName == "test.xsq");
        CHECK(a.deviceKind == "mac");
        CHECK(a.protocol == kProtocolVersion);
        c.t->Connect(a.id, "112358");
        REQUIRE(loop.Until([&] { return c.Joined() && !h.events.empty(); }, 10000));
        h.Close();
        REQUIRE(loop.Until([&] { return findIt() == nullptr; }, 10000));
        browser->Stop();
    }
}

TEST_CASE("Collab LAN join by address to an Apple host fails cleanly", "[collab][net][interop]") {
    TestLoop loop;
    Peer h(loop, 0, true), c(loop, 1, false);
    Host(h);
    std::string direct = DirectId(*h.t);
    std::string port = direct.substr(direct.find(':', 4) + 1);
    port = port.substr(0, port.find(':'));
    std::string id;
    REQUIRE(SessionIdForAddress("127.0.0.1:" + port, id));
    c.t->Connect(id, "123456");
    REQUIRE(loop.Until([&] { return !c.states.empty() && c.states.back().first == NetState::Failed; }));
    CHECK(c.states.back().second == "That address isn't hosting a session that can be joined by address.");
    CHECK(h.events.empty());
    // The Apple host sees a failed handshake, one per attempt.
    REQUIRE(loop.Until([&] { return Collab::LanTest::Pairing(*h.t).failedHandshakes == 1; }));
}

TEST_CASE("Collab LAN a session moves between Apple and portable transports", "[collab][net][interop][migrate]") {
    for (bool appleSuccessor : { true, false }) {
        INFO((appleSuccessor ? "Apple successor, portable host and client" : "portable successor, Apple host and client"));
        TestLoop loop;
        Peer h(loop, 0, !appleSuccessor), a(loop, 1, appleSuccessor), b(loop, 2, !appleSuccessor);
        Host(h, "246802");
        Join(loop, h, a, "246802");
        Join(loop, h, b, "246802");
        std::string direct = DirectId(*h.t);
        Succession s;
        s.sessionId = std::string(Wire::kServicePrefix) + direct.substr(direct.rfind(':') + 1);
        s.pairingCode = "246802";
        s.successor = 2;
        s.epoch = 1;
        h.Close();
        REQUIRE(loop.Until([&] { return a.t->State() != NetState::Connected && b.t->State() != NetState::Connected; }));
        // Both transports derive the same successor name.
        REQUIRE(a.t->SuccessorSessionId(s) == b.t->SuccessorSessionId(s));
        SessionAdvert ad;
        ad.hostName = "Successor";
        std::string error;
        REQUIRE(a.t->ListenAsSuccessor(s, ad, error));
        REQUIRE(loop.Until([&] { return a.t->State() == NetState::Listening; }));
        // Reach it the way a rejoiner would, but without Bonjour: the
        // successor's own direct id carries the derived salt.
        std::string succ = DirectId(*a.t);
        std::string salt;
        REQUIRE(Wire::SaltFromServiceName(a.t->SuccessorSessionId(s), salt));
        CHECK(succ.substr(succ.rfind(':') + 1) == Wire::HexOf(salt));
        size_t before = a.events.size();
        Peer b2(loop, 3, !appleSuccessor);
        b2.t->Connect(succ, "246802");
        REQUIRE(loop.Until([&] { return b2.Joined() && a.events.size() > before; }));
        PeerId pb = a.events[before].first;
        a.t->Send(pb, Ping(21));
        b2.t->Send(kLanHostPeer, Ping(22));
        REQUIRE(loop.Until([&] { return !b2.messages.empty() && !a.messages.empty(); }));
        CHECK(b2.messages.back().second.time == 21);
        CHECK(a.messages.back().second.time == 22);
    }
}

#endif // XL_TEST_INTEROP
