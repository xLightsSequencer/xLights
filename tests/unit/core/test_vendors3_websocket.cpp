// Loopback sockets only; POSIX/Itanium-ABI only like the other vendor tests.
#ifndef _WIN32

#include <catch2/catch_test_macros.hpp>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <cstdint>
#include <string>
#include <thread>
#include <vector>

#include <spdlog/spdlog.h>

#include "outputs/SocketAbstraction.h"

// The client's socket and line reader are private; widen access for this header only.
#define private public
#define class struct
#include "controllers/WebSocketClient.h"
#undef class
#undef private

namespace {

// A listening loopback TCP socket that hands out one accepted connection.
struct Listener {
    int fd = -1;
    uint16_t port = 0;
    explicit Listener(uint16_t want = 0) {
        fd = ::socket(AF_INET, SOCK_STREAM, 0);
        int one = 1;
        ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
        sockaddr_in a{};
        a.sin_family = AF_INET;
        a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        a.sin_port = htons(want);
        if (::bind(fd, (sockaddr*)&a, sizeof(a)) != 0 || ::listen(fd, 4) != 0) {
            ::close(fd);
            fd = -1;
            return;
        }
        socklen_t len = sizeof(a);
        ::getsockname(fd, (sockaddr*)&a, &len);
        port = ntohs(a.sin_port);
    }
    ~Listener() {
        if (fd >= 0) ::close(fd);
    }
    int Accept() {
        int c = ::accept(fd, nullptr, nullptr);
        timeval tv{ 2, 0 };
        ::setsockopt(c, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        return c;
    }
};

std::vector<uint8_t> ReadN(int fd, size_t n) {
    std::vector<uint8_t> r;
    uint8_t buf[4096];
    while (r.size() < n) {
        ssize_t got = ::recv(fd, buf, std::min(sizeof(buf), n - r.size()), 0);
        if (got <= 0) break;
        r.insert(r.end(), buf, buf + got);
    }
    return r;
}

void WriteAll(int fd, const std::vector<uint8_t>& d) {
    size_t off = 0;
    while (off < d.size()) {
        ssize_t n = ::send(fd, d.data() + off, d.size() - off, 0);
        if (n <= 0) break;
        off += (size_t)n;
    }
}

std::vector<uint8_t> Frame(const std::string& payload, bool fin = true, bool mask = false) {
    std::vector<uint8_t> f;
    f.push_back((fin ? 0x80 : 0x00) | 0x01);
    uint8_t m = mask ? 0x80 : 0;
    if (payload.size() < 126) {
        f.push_back(m | (uint8_t)payload.size());
    } else {
        f.push_back(m | 126);
        f.push_back((payload.size() >> 8) & 0xFF);
        f.push_back(payload.size() & 0xFF);
    }
    const uint8_t key[4] = { 0xA1, 0x02, 0x33, 0x4C };
    if (mask) f.insert(f.end(), key, key + 4);
    for (size_t i = 0; i < payload.size(); ++i) f.push_back((uint8_t)payload[i] ^ (mask ? key[i & 3] : 0));
    return f;
}

// A client already connected (without the HTTP upgrade) to a loopback peer.
struct WsPair {
    Listener l;
    WebSocketClient ws;
    int peer = -1;
    WsPair() {
        REQUIRE(l.fd >= 0);
        REQUIRE(ws._socket.Connect("127.0.0.1", l.port, "", false));
        ws._connected = true;
        peer = l.Accept();
        REQUIRE(peer >= 0);
    }
    ~WsPair() {
        if (peer >= 0) ::close(peer);
    }
};

} // namespace

TEST_CASE("WebSocket3 Send frames short, medium and long text messages", "[vendors3][websocket3]") {
    WsPair p;
    REQUIRE(p.ws.Send("hi"));
    auto f = ReadN(p.peer, 4);
    REQUIRE(f.size() == 4);
    CHECK(f[0] == 0x81);
    CHECK(f[1] == 2);
    CHECK(f[2] == 'h');
    CHECK(f[3] == 'i');

    std::string medium(300, 'm');
    REQUIRE(p.ws.Send(medium));
    f = ReadN(p.peer, 4 + 300);
    REQUIRE(f.size() == 304);
    CHECK(f[1] == 126);
    CHECK(((f[2] << 8) | f[3]) == 300);
    CHECK(f[4] == 'm');

    std::string big(70000, 'b');
    big.back() = 'z';
    REQUIRE(p.ws.Send(big));
    f = ReadN(p.peer, 10 + 70000);
    REQUIRE(f.size() == 70010);
    CHECK(f[1] == 127);
    uint64_t len = 0;
    for (int i = 2; i < 10; ++i) len = (len << 8) | f[i];
    CHECK(len == 70000);
    CHECK(f[10] == 'b');
    CHECK(f.back() == 'z');
}

TEST_CASE("WebSocket3 Receive decodes plain, masked, extended and fragmented frames", "[vendors3][websocket3]") {
    WsPair p;
    WriteAll(p.peer, Frame("hello"));
    CHECK(p.ws.Receive() == "hello");

    WriteAll(p.peer, Frame("secret", true, true));
    CHECK(p.ws.Receive() == "secret");

    std::string medium(500, 'x');
    medium[0] = 'A';
    WriteAll(p.peer, Frame(medium));
    CHECK(p.ws.Receive() == medium);

    auto a = Frame("frag", false);
    auto b = Frame("ment");
    a.insert(a.end(), b.begin(), b.end());
    WriteAll(p.peer, a);
    CHECK(p.ws.Receive() == "fragment");
}

TEST_CASE("WebSocket3 Receive returns nothing when no frame arrives", "[vendors3][websocket3]") {
    WsPair p;
    p.ws._socket.SetReadTimeoutMs(50);
    CHECK(p.ws.Receive().empty());
}

TEST_CASE("WebSocket3 ReadSocket reads one line and stops on a closed peer", "[vendors3][websocket3]") {
    WsPair p;
    std::string line = "HTTP/1.1 101 Switching\r\nUpgrade: websocket\r\n";
    WriteAll(p.peer, std::vector<uint8_t>(line.begin(), line.end()));
    CHECK(p.ws.ReadSocket() == "HTTP/1.1 101 Switching\r\n");
    ::close(p.peer);
    p.peer = -1;
    p.ws.ClearIncomingData();
    CHECK(p.ws.ReadSocket().empty());
}

TEST_CASE("WebSocket3 Connect fails when nothing listens", "[vendors3][websocket3]") {
    WebSocketClient ws;
    CHECK_FALSE(ws.Connect("not an address", "/ws"));
    CHECK_FALSE(ws.IsConnected());
}

TEST_CASE("WebSocket3 Connect upgrades on port 80 and rejects other answers", "[vendors3][websocket3]") {
    // the client always uses port 80; skip when the loopback port can not be taken
    Listener l(80);
    if (l.fd < 0) SKIP("loopback port 80 is not available");
    for (bool accept : { true, false }) {
        INFO("accept " << accept);
        std::string request;
        std::thread server([&] {
            int c = l.Accept();
            auto got = ReadN(c, 60);
            request.assign(got.begin(), got.end());
            std::string reply = accept ? "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\n\r\n" : "HTTP/1.1 404 Not Found\r\n\r\n";
            WriteAll(c, std::vector<uint8_t>(reply.begin(), reply.end()));
            std::this_thread::sleep_for(std::chrono::milliseconds(1200));
            ::close(c);
        });
        WebSocketClient ws;
        bool ok = ws.Connect("127.0.0.1", "");
        server.join();
        CHECK(ok == accept);
        CHECK(ws.IsConnected() == accept);
        CHECK(request.rfind("GET / HTTP/1.1\r\nHost: 127.0.0.1\r\n", 0) == 0);
    }
}

#endif
