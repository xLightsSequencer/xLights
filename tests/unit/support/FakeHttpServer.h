#pragma once

// Minimal loopback HTTP server so vendor handlers can be fed canned controller
// responses without touching a real network. POSIX only; tests that use it are
// compiled out on Windows.
#ifndef _WIN32

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <poll.h>

#include <atomic>
#include <cstdlib>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

struct FakeRequest {
    std::string method;
    std::string path;
    std::string body;
};

struct FakeResponse {
    int status = 200;
    std::string body;
    std::string contentType = "application/json";
};

class FakeHttpServer {
public:
    using Handler = std::function<FakeResponse(const FakeRequest&)>;

    explicit FakeHttpServer(Handler h) : _handler(std::move(h)) {
        _fd = ::socket(AF_INET, SOCK_STREAM, 0);
        int one = 1;
        ::setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
        sockaddr_in a{};
        a.sin_family = AF_INET;
        a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        a.sin_port = 0;
        if (::bind(_fd, (sockaddr*)&a, sizeof(a)) != 0 || ::listen(_fd, 16) != 0) {
            ::close(_fd);
            _fd = -1;
            return;
        }
        socklen_t len = sizeof(a);
        ::getsockname(_fd, (sockaddr*)&a, &len);
        _port = ntohs(a.sin_port);
        _thread = std::thread([this] { Loop(); });
    }
    ~FakeHttpServer() {
        _stop = true;
        if (_thread.joinable()) _thread.join();
        for (auto& t : _workers) if (t.joinable()) t.join();
        if (_fd >= 0) ::close(_fd);
    }
    FakeHttpServer(const FakeHttpServer&) = delete;
    FakeHttpServer& operator=(const FakeHttpServer&) = delete;

    bool ok() const { return _fd >= 0; }
    std::string Address() const { return "127.0.0.1:" + std::to_string(_port); }

    std::vector<FakeRequest> Requests() const {
        std::lock_guard<std::mutex> l(_m);
        return _requests;
    }
    int Count(const std::string& path) const {
        int n = 0;
        for (auto& r : Requests()) if (r.path == path) ++n;
        return n;
    }

private:
    void Loop() {
        while (!_stop) {
            pollfd p{ _fd, POLLIN, 0 };
            if (::poll(&p, 1, 50) <= 0) continue;
            int c = ::accept(_fd, nullptr, nullptr);
            if (c < 0) continue;
            _workers.emplace_back([this, c] { Serve(c); });
        }
    }
    static bool ReadSome(int c, std::string& buf) {
        pollfd p{ c, POLLIN, 0 };
        if (::poll(&p, 1, 2000) <= 0) return false;
        char tmp[4096];
        ssize_t n = ::recv(c, tmp, sizeof(tmp), 0);
        if (n <= 0) return false;
        buf.append(tmp, (size_t)n);
        return true;
    }
    void Serve(int c) {
        std::string buf;
        size_t hdrEnd = std::string::npos;
        while ((hdrEnd = buf.find("\r\n\r\n")) == std::string::npos) {
            if (!ReadSome(c, buf)) { ::close(c); return; }
        }
        FakeRequest req;
        size_t sp1 = buf.find(' ');
        size_t sp2 = buf.find(' ', sp1 + 1);
        req.method = buf.substr(0, sp1);
        req.path = buf.substr(sp1 + 1, sp2 - sp1 - 1);
        size_t clen = 0;
        std::string head = buf.substr(0, hdrEnd);
        for (auto& ch : head) ch = (char)tolower((unsigned char)ch);
        size_t cl = head.find("content-length:");
        if (cl != std::string::npos) clen = (size_t)strtoul(head.c_str() + cl + 15, nullptr, 10);
        std::string body = buf.substr(hdrEnd + 4);
        while (body.size() < clen) {
            if (!ReadSome(c, body)) break;
        }
        req.body = body.substr(0, clen);
        {
            std::lock_guard<std::mutex> l(_m);
            _requests.push_back(req);
        }
        FakeResponse r = _handler(req);
        std::string out = "HTTP/1.1 " + std::to_string(r.status) + " X\r\nContent-Type: " + r.contentType +
                          "\r\nContent-Length: " + std::to_string(r.body.size()) + "\r\nConnection: close\r\n\r\n" + r.body;
        size_t off = 0;
        while (off < out.size()) {
            ssize_t n = ::send(c, out.data() + off, out.size() - off, 0);
            if (n <= 0) break;
            off += (size_t)n;
        }
        ::shutdown(c, SHUT_RDWR);
        ::close(c);
    }

    Handler _handler;
    int _fd = -1;
    int _port = 0;
    std::atomic<bool> _stop{ false };
    std::thread _thread;
    std::vector<std::thread> _workers;
    mutable std::mutex _m;
    std::vector<FakeRequest> _requests;
};

#endif
