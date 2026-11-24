/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// The LAN transport for Windows and Linux. It speaks exactly what the Apple
// transport (CollabNetApple.mm) speaks, so either can host the other:
// DNS-SD service _xlights-collab._tcp (Avahi's dns_sd.h on Linux, the
// DnsService* API on Windows 10+), TLS 1.2 PSK with
// TLS_PSK_WITH_AES_128_GCM_SHA256 (Mbed TLS) and the frames of
// CollabNetWire.h. Each transport has one socket thread; every callback
// goes through the Dispatcher.
//
// Where the network can't be browsed (Windows before 10, Linux without
// avahi-daemon, networks that drop multicast) a client can join by
// "host:port". It gets the session's salt with a small request a host of
// this transport answers before TLS (Wire::kSaltRequest); Apple hosts don't
// answer it.

#if !defined(__APPLE__) || defined(XL_COLLAB_PORTABLE_NET)

// The default build compiles Mbed TLS from dependencies/mbedtls with our own
// configuration. XL_MBEDTLS_EXTERNAL uses an installed one (a distro's 2.28,
// a bundled 3.6) with whatever configuration it was built with.
#ifndef XL_MBEDTLS_EXTERNAL
#define MBEDTLS_CONFIG_FILE "collab/CollabTlsConfig.h"
#endif

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <mstcpip.h>
#include <windns.h>
#if defined(_MSC_VER) && !defined(XLIGHTS_CMAKE_BUILD)
#pragma comment(lib, "ws2_32.lib")
#endif
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#if __has_include(<dns_sd.h>)
#include <dns_sd.h>
#define XL_COLLAB_DNSSD 1
#endif
#endif

#include "CollabFiles.h"
#include "CollabNet.h"
#include "CollabNetWire.h"
#include "utils/ip_utils.h"

#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/error.h>
#include <mbedtls/md.h>
#include <mbedtls/pkcs5.h>
#include <mbedtls/sha256.h>
#include <mbedtls/ssl.h>
#include <mbedtls/version.h>
#if MBEDTLS_VERSION_NUMBER >= 0x03000000 && defined(MBEDTLS_PSA_CRYPTO_C)
#include <psa/crypto.h>
#endif

#if MBEDTLS_VERSION_NUMBER < 0x02100000
#error "The LAN transport needs Mbed TLS 2.16 or later"
#endif
#if !defined(MBEDTLS_KEY_EXCHANGE_PSK_ENABLED) || !defined(MBEDTLS_GCM_C) || !defined(MBEDTLS_AES_C) || !defined(MBEDTLS_SHA256_C) || \
    !defined(MBEDTLS_PKCS5_C) || !defined(MBEDTLS_SSL_CLI_C) || !defined(MBEDTLS_SSL_SRV_C)
#error "This Mbed TLS build lacks TLS_PSK_WITH_AES_128_GCM_SHA256 or PBKDF2, which the LAN transport needs"
#endif

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <new>
#include <set>
#include <system_error>
#include <thread>
#include <vector>

#include <spdlog/spdlog.h>

namespace fs = std::filesystem;

namespace Collab {

namespace {

constexpr size_t kMaxInFlight = 128u << 10;
constexpr unsigned kPbkdfRounds = 100000;
constexpr int64_t kConnectTimeoutMs = 15000;
constexpr int64_t kHandshakeTimeoutMs = 15000;
constexpr int64_t kCloseGraceMs = 2000;
// A host of this transport answers a salt request at once; anything else
// (an Apple host waits for the rest of a TLS record) doesn't answer at all.
constexpr int64_t kSaltReplyTimeoutMs = 3000;
constexpr int kPairingFailuresAllowed = 5;
constexpr int64_t kPairingBackoffFirstMs = 5000;
constexpr int64_t kPairingBackoffMaxMs = 60000;
constexpr int kKeepAliveIdleSec = 10;
constexpr int kKeepAliveIntervalSec = 5;
constexpr int kKeepAliveCount = 3;
// Reading stops after this much in one pass so one busy peer can't starve
// the others; the rest is read on the next pass.
constexpr size_t kReadBurst = 4u << 20;
constexpr int kBioRecvFailed = -0x004C; // MBEDTLS_ERR_NET_RECV_FAILED

int64_t NowMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

// ---------------------------------------------------------------------------
// Sockets
// ---------------------------------------------------------------------------

#ifdef _WIN32
using Sock = SOCKET;
using PollFd = WSAPOLLFD;
const Sock kNoSock = INVALID_SOCKET;
int SockErr() {
    return WSAGetLastError();
}
bool WouldBlock(int e) {
    return e == WSAEWOULDBLOCK;
}
bool ConnectPending(int e) {
    return e == WSAEWOULDBLOCK || e == WSAEINPROGRESS;
}
void CloseSock(Sock s) {
    closesocket(s);
}
int PollFds(PollFd* fds, size_t n, int ms) {
    return WSAPoll(fds, (ULONG)n, ms);
}
bool NetStartup() {
    static bool ok = [] {
        WSADATA d;
        return WSAStartup(MAKEWORD(2, 2), &d) == 0;
    }();
    return ok;
}
constexpr int kSendFlags = 0;
#else
using Sock = int;
using PollFd = pollfd;
const Sock kNoSock = -1;
int SockErr() {
    return errno;
}
bool WouldBlock(int e) {
    return e == EAGAIN || e == EWOULDBLOCK;
}
bool ConnectPending(int e) {
    return e == EINPROGRESS;
}
void CloseSock(Sock s) {
    ::close(s);
}
int PollFds(PollFd* fds, size_t n, int ms) {
    return ::poll(fds, (nfds_t)n, ms);
}
bool NetStartup() {
    return true;
}
#ifdef MSG_NOSIGNAL
constexpr int kSendFlags = MSG_NOSIGNAL;
#else
constexpr int kSendFlags = 0;
#endif
#endif

void SetNonBlocking(Sock s) {
#ifdef _WIN32
    u_long on = 1;
    ioctlsocket(s, FIONBIO, &on);
#else
    int fl = fcntl(s, F_GETFL, 0);
    fcntl(s, F_SETFL, fl | O_NONBLOCK);
#ifdef SO_NOSIGPIPE
    int one = 1;
    setsockopt(s, SOL_SOCKET, SO_NOSIGPIPE, &one, sizeof(one));
#endif
#endif
}

// The same TCP settings the Apple transport asks Network.framework for: no
// Nagle delay, and a peer that vanished (sleep, Wi-Fi gone) is noticed in
// well under a minute even when nothing is being sent.
void ConfigureTcp(Sock s) {
    int one = 1;
    setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (const char*)&one, sizeof(one));
    setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (const char*)&one, sizeof(one));
#ifdef _WIN32
    tcp_keepalive ka{};
    ka.onoff = 1;
    ka.keepalivetime = kKeepAliveIdleSec * 1000;
    ka.keepaliveinterval = kKeepAliveIntervalSec * 1000;
    DWORD got = 0;
    WSAIoctl(s, SIO_KEEPALIVE_VALS, &ka, sizeof(ka), nullptr, 0, &got, nullptr, nullptr);
#ifdef TCP_KEEPCNT
    // Windows 10 1703 and later; earlier systems keep their fixed count.
    DWORD cnt = kKeepAliveCount;
    setsockopt(s, IPPROTO_TCP, TCP_KEEPCNT, (const char*)&cnt, sizeof(cnt));
#endif
#else
    int idle = kKeepAliveIdleSec, intvl = kKeepAliveIntervalSec, cnt = kKeepAliveCount;
#ifdef TCP_KEEPIDLE
    setsockopt(s, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(idle));
#elif defined(TCP_KEEPALIVE)
    setsockopt(s, IPPROTO_TCP, TCP_KEEPALIVE, &idle, sizeof(idle));
#endif
    setsockopt(s, IPPROTO_TCP, TCP_KEEPINTVL, &intvl, sizeof(intvl));
    setsockopt(s, IPPROTO_TCP, TCP_KEEPCNT, &cnt, sizeof(cnt));
#endif
}

std::string ConnectErrorText(int e) {
#ifdef _WIN32
    if (e == WSAECONNREFUSED) {
        return "The host is not accepting connections.";
    }
    if (e == WSAETIMEDOUT || e == WSAEHOSTUNREACH || e == WSAENETUNREACH) {
        return "The host did not answer.";
    }
#else
    if (e == ECONNREFUSED) {
        return "The host is not accepting connections.";
    }
    if (e == ETIMEDOUT || e == EHOSTUNREACH || e == ENETUNREACH) {
        return "The host did not answer.";
    }
#endif
    return "Could not reach the host (error " + std::to_string(e) + ").";
}

// Wakes a thread blocked in poll: a UDP socket connected to itself, which
// works with poll / WSAPoll everywhere (Windows can't poll a pipe).
class Waker {
public:
    Waker() {
        _s = socket(AF_INET, SOCK_DGRAM, 0);
        if (_s == kNoSock) {
            return;
        }
        sockaddr_in a{};
        a.sin_family = AF_INET;
        a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        socklen_t len = sizeof(a);
        if (bind(_s, (sockaddr*)&a, sizeof(a)) != 0 || getsockname(_s, (sockaddr*)&a, &len) != 0 || connect(_s, (sockaddr*)&a, sizeof(a)) != 0) {
            CloseSock(_s);
            _s = kNoSock;
            return;
        }
        SetNonBlocking(_s);
    }
    ~Waker() {
        if (_s != kNoSock) {
            CloseSock(_s);
        }
    }
    Sock Fd() const { return _s; }
    void Wake() {
        char c = 1;
        send(_s, &c, 1, 0);
    }
    void Drain() {
        char buf[64];
        while (recv(_s, buf, sizeof(buf), 0) > 0) {
        }
    }

private:
    Sock _s = kNoSock;
};

// ---------------------------------------------------------------------------
// A thread that owns sockets: work posted to it, poll, timers.
// ---------------------------------------------------------------------------

// Runs on the lookup thread before a name lookup (tests hold it up there).
using LookupHook = std::function<void(const std::string& host)>;
std::mutex& LookupHookLock() {
    static std::mutex m;
    return m;
}
LookupHook& LookupHookSlot() {
    static LookupHook hook;
    return hook;
}
LookupHook BeforeLookup() {
    std::lock_guard<std::mutex> l(LookupHookLock());
    return LookupHookSlot();
}

class LoopThread : public std::enable_shared_from_this<LoopThread> {
public:
    virtual ~LoopThread() = default;

    void StartThread() {
        NetStartup();
        std::shared_ptr<LoopThread> self = shared_from_this();
        {
            std::lock_guard<std::mutex> l(_m);
            _running = true;
        }
        // The thread keeps the object alive (connections may still be saying
        // goodbye after the owner is gone) and ends once Finished().
        std::thread([self] { self->Run(); }).detach();
    }

    void Post(std::function<void()> fn) {
        {
            std::lock_guard<std::mutex> l(_m);
            if (!_running) {
                return;
            }
            _tasks.push_back(std::move(fn));
        }
        _waker.Wake();
    }

    // Runs `fn` on the thread and waits; inline once the thread has ended.
    void Sync(std::function<void()> fn) {
        auto done = std::make_shared<bool>(false);
        {
            std::unique_lock<std::mutex> l(_m);
            if (!_running) {
                l.unlock();
                fn();
                return;
            }
            _tasks.push_back([this, fn, done] {
                fn();
                std::lock_guard<std::mutex> l2(_m);
                *done = true;
                _cv.notify_all();
            });
        }
        _waker.Wake();
        std::unique_lock<std::mutex> l(_m);
        _cv.wait(l, [&] { return *done || !_running; });
        if (!*done) {
            l.unlock();
            fn();
        }
    }

protected:
    struct Watch {
        Sock fd;
        short events;
        std::function<void(short revents)> fn;
    };
    // Called on the thread before every poll.
    virtual void Watches(std::vector<Watch>& out) = 0;
    // The next time OnTimers wants to run (0 = no timer).
    virtual int64_t NextDeadline() = 0;
    virtual void OnTimers(int64_t now) = 0;
    virtual bool Finished() = 0;
    // Run once on the thread as it ends.
    virtual void OnThreadEnd() {}

private:
    void Run() {
        std::vector<Watch> watches;
        std::vector<PollFd> fds;
        while (true) {
            std::deque<std::function<void()>> tasks;
            {
                std::lock_guard<std::mutex> l(_m);
                tasks.swap(_tasks);
            }
            for (auto& t : tasks) {
                t();
            }
            OnTimers(NowMs());
            if (Finished()) {
                break;
            }
            watches.clear();
            Watches(watches);
            fds.clear();
            PollFd w{};
            w.fd = _waker.Fd();
            w.events = POLLIN;
            fds.push_back(w);
            for (auto& wt : watches) {
                PollFd p{};
                p.fd = wt.fd;
                p.events = wt.events;
                fds.push_back(p);
            }
            int timeout = 1000;
            int64_t next = NextDeadline();
            if (next > 0) {
                timeout = (int)std::clamp<int64_t>(next - NowMs(), 0, 1000);
            }
            {
                std::lock_guard<std::mutex> l(_m);
                if (!_tasks.empty()) {
                    timeout = 0;
                }
            }
            int n = PollFds(fds.data(), fds.size(), timeout);
            if (n <= 0) {
                continue;
            }
            if (fds[0].revents != 0) {
                _waker.Drain();
            }
            for (size_t i = 1; i < fds.size(); ++i) {
                if (fds[i].revents != 0) {
                    watches[i - 1].fn(fds[i].revents);
                }
            }
        }
        OnThreadEnd();
        std::deque<std::function<void()>> left;
        {
            std::lock_guard<std::mutex> l(_m);
            _running = false;
            left.swap(_tasks);
            _cv.notify_all();
        }
        // Sync callers waiting on these run them themselves.
        left.clear();
    }

    std::mutex _m;
    std::condition_variable _cv;
    std::deque<std::function<void()>> _tasks;
    bool _running = false;
    Waker _waker;
};

// ---------------------------------------------------------------------------
// DNS-SD
// ---------------------------------------------------------------------------

std::string TxtValue(const std::string& s) {
    constexpr size_t kMax = 200;
    if (s.size() <= kMax) {
        return s;
    }
    size_t n = kMax;
    while (n > 0 && ((unsigned char)s[n] & 0xC0) == 0x80) {
        --n;
    }
    return s.substr(0, n);
}

#ifdef _WIN32
typedef DNS_STATUS(WINAPI* PFN_DnsServiceBrowse)(PDNS_SERVICE_BROWSE_REQUEST, PDNS_SERVICE_CANCEL);
typedef DNS_STATUS(WINAPI* PFN_DnsServiceBrowseCancel)(PDNS_SERVICE_CANCEL);
typedef DNS_STATUS(WINAPI* PFN_DnsServiceResolve)(PDNS_SERVICE_RESOLVE_REQUEST, PDNS_SERVICE_CANCEL);
typedef DNS_STATUS(WINAPI* PFN_DnsServiceResolveCancel)(PDNS_SERVICE_CANCEL);
typedef VOID(WINAPI* PFN_DnsServiceFreeInstance)(PDNS_SERVICE_INSTANCE);
typedef VOID(WINAPI* PFN_DnsRecordListFree)(PDNS_RECORD, DNS_FREE_TYPE);
typedef PDNS_SERVICE_INSTANCE(WINAPI* PFN_DnsServiceConstructInstance)(PCWSTR, PCWSTR, PIP4_ADDRESS, PIP6_ADDRESS, WORD, WORD, WORD, DWORD, PCWSTR*,
                                                                      PCWSTR*);
typedef DWORD(WINAPI* PFN_DnsServiceRegister)(PDNS_SERVICE_REGISTER_REQUEST, PDNS_SERVICE_CANCEL);
typedef DWORD(WINAPI* PFN_DnsServiceDeRegister)(PDNS_SERVICE_REGISTER_REQUEST, PDNS_SERVICE_CANCEL);

// The DnsService* functions arrived in Windows 10; loading them by name
// keeps xLights starting on Windows 8, where only joining by address works.
struct WinDns {
    PFN_DnsServiceBrowse browse = nullptr;
    PFN_DnsServiceBrowseCancel browseCancel = nullptr;
    PFN_DnsServiceResolve resolve = nullptr;
    PFN_DnsServiceResolveCancel resolveCancel = nullptr;
    PFN_DnsServiceFreeInstance freeInstance = nullptr;
    PFN_DnsRecordListFree recordListFree = nullptr;
    PFN_DnsServiceConstructInstance construct = nullptr;
    PFN_DnsServiceRegister reg = nullptr;
    PFN_DnsServiceDeRegister dereg = nullptr;
    bool browseOk = false;
    bool registerOk = false;
};

const WinDns& Dns() {
    static const WinDns api = [] {
        WinDns a;
        if (HMODULE h = LoadLibraryW(L"dnsapi.dll")) {
            a.browse = (PFN_DnsServiceBrowse)GetProcAddress(h, "DnsServiceBrowse");
            a.browseCancel = (PFN_DnsServiceBrowseCancel)GetProcAddress(h, "DnsServiceBrowseCancel");
            a.resolve = (PFN_DnsServiceResolve)GetProcAddress(h, "DnsServiceResolve");
            a.resolveCancel = (PFN_DnsServiceResolveCancel)GetProcAddress(h, "DnsServiceResolveCancel");
            a.freeInstance = (PFN_DnsServiceFreeInstance)GetProcAddress(h, "DnsServiceFreeInstance");
            a.recordListFree = (PFN_DnsRecordListFree)GetProcAddress(h, "DnsRecordListFree");
            a.construct = (PFN_DnsServiceConstructInstance)GetProcAddress(h, "DnsServiceConstructInstance");
            a.reg = (PFN_DnsServiceRegister)GetProcAddress(h, "DnsServiceRegister");
            a.dereg = (PFN_DnsServiceDeRegister)GetProcAddress(h, "DnsServiceDeRegister");
            a.browseOk = a.browse && a.browseCancel && a.resolve && a.freeInstance && a.recordListFree;
            a.registerOk = a.construct && a.reg && a.dereg && a.freeInstance;
        }
        return a;
    }();
    return api;
}

std::wstring Wide(const std::string& s) {
    if (s.empty()) {
        return {};
    }
    int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
    std::wstring w(n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), w.data(), n);
    return w;
}

std::string Narrow(const wchar_t* w) {
    if (w == nullptr || *w == 0) {
        return {};
    }
    int len = (int)wcslen(w);
    int n = WideCharToMultiByte(CP_UTF8, 0, w, len, nullptr, 0, nullptr, nullptr);
    std::string s(n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w, len, s.data(), n, nullptr, nullptr);
    return s;
}

std::wstring FullServiceName(const std::string& instance) {
    return Wide(instance) + L"." + Wide(Wire::kServiceType) + L".local";
}

// "xlc-...._xlights-collab._tcp.local" -> "xlc-..."
std::string InstanceOf(const std::string& full) {
    std::string marker = std::string(".") + Wire::kServiceType;
    size_t at = full.find(marker);
    return at == std::string::npos ? full : full.substr(0, at);
}

// One DnsServiceRegister. Registering takes a moment (the name is probed on
// the network first), and a deregistration sent before it completes is
// refused, which left the session advertised after its host had gone. So a
// stop that comes early waits for the registration to finish. The transport
// and the completion callbacks share it (Wire::RegistrationLifetime): it is
// deleted by whichever of them is last, possibly after the transport is gone,
// and never while the transport still holds it.
class WinRegistration {
public:
    static WinRegistration* Start(PDNS_SERVICE_INSTANCE inst) {
        auto* r = new WinRegistration();
        r->_req.Version = DNS_QUERY_REQUEST_VERSION1;
        r->_req.InterfaceIndex = 0;
        r->_req.pServiceInstance = inst;
        r->_req.pRegisterCompletionCallback = &WinRegistration::Completed;
        r->_req.pQueryContext = r;
        r->_req.unicastEnabled = FALSE;
        r->_inst = inst;
        DWORD st = Dns().reg(&r->_req, nullptr);
        if (st != DNS_REQUEST_PENDING) {
            spdlog::warn("collab: could not advertise the session ({})", (int)st);
            Dns().freeInstance(inst);
            delete r;
            return nullptr;
        }
        return r;
    }

    // The transport lets go of it; it must not use `r` afterwards.
    static void Stop(WinRegistration* r) {
        Wire::RegistrationLifetime::Action a;
        {
            std::lock_guard<std::mutex> l(r->_m);
            a = r->_life.Stop();
        }
        r->Act(a);
    }

    // The registration failed after Start returned (it is not advertised).
    bool Failed() {
        std::lock_guard<std::mutex> l(_m);
        return _life.Failed();
    }

private:
    void Act(Wire::RegistrationLifetime::Action a) {
        switch (a) {
        case Wire::RegistrationLifetime::Action::Deregister:
            if (Dns().dereg(&_req, nullptr) != DNS_REQUEST_PENDING) {
                {
                    std::lock_guard<std::mutex> l(_m);
                    a = _life.DeregisterFailed();
                }
                Act(a);
            }
            break;
        case Wire::RegistrationLifetime::Action::Free:
            Dns().freeInstance(_inst);
            delete this;
            break;
        default:
            break;
        }
    }

    static VOID WINAPI Completed(DWORD status, PVOID ctx, PDNS_SERVICE_INSTANCE instance) {
        auto* r = static_cast<WinRegistration*>(ctx);
        if (instance != nullptr && instance != r->_inst) {
            Dns().freeInstance(instance);
        }
        if (status != ERROR_SUCCESS) {
            spdlog::warn("collab: advertising the session failed ({})", (int)status);
        }
        Wire::RegistrationLifetime::Action a;
        {
            std::lock_guard<std::mutex> l(r->_m);
            a = r->_life.Completed(status == ERROR_SUCCESS);
        }
        r->Act(a);
    }

    std::mutex _m;
    DNS_SERVICE_REGISTER_REQUEST _req{};
    PDNS_SERVICE_INSTANCE _inst = nullptr;
    Wire::RegistrationLifetime _life;
};
#endif

bool DiscoveryCompiled() {
#if defined(XL_COLLAB_DNSSD)
    return true;
#elif defined(_WIN32)
    return Dns().browseOk;
#else
    return false;
#endif
}

#if defined(XL_COLLAB_DNSSD)
std::string DnsSdError(DNSServiceErrorType e) {
    // -65563 is kDNSServiceErr_ServiceNotRunning, which Avahi's dns_sd.h
    // predates; Avahi reports a missing daemon as kDNSServiceErr_Unknown.
    if (e == (DNSServiceErrorType)-65563 || e == kDNSServiceErr_Unknown) {
        return "Finding sessions needs the Avahi service (avahi-daemon), which isn't running. You can still join by address.";
    }
    return "Finding sessions failed (" + std::to_string((int)e) + ").";
}
#endif

std::string MakeFolder() {
    std::error_code ec;
    fs::path base = fs::temp_directory_path(ec);
    if (ec) {
        base = ".";
    }
    fs::path dir = base / ("xlcollab-" + Wire::HexOf(Wire::NewSalt()));
    fs::create_directories(dir, ec);
    return dir.string();
}

// ---------------------------------------------------------------------------
// TLS
// ---------------------------------------------------------------------------

// A 3.x library built with TLS 1.3 or PSA-backed hashing needs PSA started
// even for TLS 1.2. Repeated calls are cheap.
bool StartPsa() {
#if MBEDTLS_VERSION_NUMBER >= 0x03000000 && defined(MBEDTLS_PSA_CRYPTO_C)
    return psa_crypto_init() == PSA_SUCCESS;
#else
    return true;
#endif
}

struct TlsConfig {
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context drbg;
    mbedtls_ssl_config conf;
    std::string key;
    bool ok = false;

    TlsConfig(bool server, const std::string& psk, bool failStart = false) : key(psk) {
        // First, so the destructor can free them whatever fails below.
        mbedtls_entropy_init(&entropy);
        mbedtls_ctr_drbg_init(&drbg);
        mbedtls_ssl_config_init(&conf);
        if (failStart || !StartPsa()) {
            return;
        }
        static const int kSuites[] = { MBEDTLS_TLS_PSK_WITH_AES_128_GCM_SHA256, 0 };
        const char* pers = "xlights-collab";
        if (key.size() != 32 ||
            mbedtls_ctr_drbg_seed(&drbg, mbedtls_entropy_func, &entropy, (const unsigned char*)pers, strlen(pers)) != 0 ||
            mbedtls_ssl_config_defaults(&conf, server ? MBEDTLS_SSL_IS_SERVER : MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM,
                                        MBEDTLS_SSL_PRESET_DEFAULT) != 0) {
            return;
        }
        mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &drbg);
        // The PSK suites are TLS 1.2 only, as on the Apple side.
#if MBEDTLS_VERSION_NUMBER >= 0x03020000
        mbedtls_ssl_conf_min_tls_version(&conf, MBEDTLS_SSL_VERSION_TLS1_2);
        mbedtls_ssl_conf_max_tls_version(&conf, MBEDTLS_SSL_VERSION_TLS1_2);
#else
        mbedtls_ssl_conf_min_version(&conf, MBEDTLS_SSL_MAJOR_VERSION_3, MBEDTLS_SSL_MINOR_VERSION_3);
        mbedtls_ssl_conf_max_version(&conf, MBEDTLS_SSL_MAJOR_VERSION_3, MBEDTLS_SSL_MINOR_VERSION_3);
#endif
        mbedtls_ssl_conf_ciphersuites(&conf, kSuites);
        ok = mbedtls_ssl_conf_psk(&conf, (const unsigned char*)key.data(), key.size(), (const unsigned char*)Wire::kPskIdentity,
                                  strlen(Wire::kPskIdentity)) == 0;
    }
    ~TlsConfig() {
        mbedtls_ssl_config_free(&conf);
        mbedtls_ctr_drbg_free(&drbg);
        mbedtls_entropy_free(&entropy);
        std::fill(key.begin(), key.end(), '\0');
    }
    TlsConfig(const TlsConfig&) = delete;
    TlsConfig& operator=(const TlsConfig&) = delete;
};

std::string TlsErrorText(int ret) {
#if defined(MBEDTLS_ERROR_C)
    char buf[160];
    mbedtls_strerror(ret, buf, sizeof(buf));
    return buf;
#else
    char buf[32];
    std::snprintf(buf, sizeof(buf), "-0x%04X", (unsigned)-ret);
    return buf;
#endif
}

// ---------------------------------------------------------------------------
// Connections
// ---------------------------------------------------------------------------

struct OutBlob {
    BlobId id = 0;
    std::string path;
    uint64_t size = 0;
    uint64_t sent = 0;
    FILE* f = nullptr;
    bool started = false;
};

struct Conn {
    enum class Phase {
        Resolving,  // client: finding the host's address
        Connecting, // client: TCP connect in progress
        SaltWait,   // client: asked a host found by address for its salt
        Sniff,      // host: waiting for the first bytes (TLS or a salt request)
        Handshake,  // TLS handshake
        Open,       // TLS up
        Lingering,  // closing: sending what's left, then the socket goes
    };
    Phase phase = Phase::Connecting;
    Sock s = kNoSock;
    PeerId id = kNoPeer;
    bool ready = false;
    bool joined = false;
    bool gone = false;
    bool gotBytes = false; // anything at all came from the other side
    int64_t deadline = 0;  // handshake / connect / linger limit, 0 = none

    mbedtls_ssl_context ssl;
    bool sslInit = false;
    std::string preTls;      // bytes read before TLS started, handed to TLS first
    size_t preTlsPos = 0;
    std::string sockOut;     // ciphertext (or salt bytes) waiting for the socket
    size_t outPos = 0;
    bool shutWr = false;
    int recvError = 0;
    bool moreToRead = false;

    // client connect
    std::vector<sockaddr_storage> addrs;
    std::vector<socklen_t> addrLens;
    size_t addrAt = 0;
    int lastConnectError = 0;
    bool needSalt = false;

    Wire::FrameReader reader;
    std::deque<std::string> control;
    std::deque<OutBlob> blobs;

    Files::IncomingBlob in;

#if defined(XL_COLLAB_DNSSD)
    DNSServiceRef resolveRef = nullptr;
    DNSServiceRef addrRef = nullptr;
#endif

    Conn() { mbedtls_ssl_init(&ssl); }
    ~Conn() {
        CloseSocket();
        mbedtls_ssl_free(&ssl);
        DropDns();
    }
    Conn(const Conn&) = delete;
    Conn& operator=(const Conn&) = delete;

    size_t Pending() const { return sockOut.size() - outPos; }
    // Something to write now, or frames Pump hasn't encrypted yet (it
    // stops after a while so other connections get their turn).
    bool WantsWrite() const { return Pending() > 0 || (ready && !gone && (!control.empty() || !blobs.empty())); }

    void CloseSocket() {
        if (s != kNoSock) {
            CloseSock(s);
            s = kNoSock;
        }
    }

    void DropDns() {
#if defined(XL_COLLAB_DNSSD)
        if (resolveRef != nullptr) {
            DNSServiceRefDeallocate(resolveRef);
            resolveRef = nullptr;
        }
        if (addrRef != nullptr) {
            DNSServiceRefDeallocate(addrRef);
            addrRef = nullptr;
        }
#endif
    }

    void DropFiles() {
        std::error_code ec;
        for (auto& b : blobs) {
            if (b.f != nullptr) {
                fclose(b.f);
            }
            fs::remove(b.path, ec);
        }
        blobs.clear();
        in.Abort();
    }

    // Sends what it can without blocking. False when the socket failed.
    bool Flush() {
        while (Pending() > 0 && s != kNoSock) {
            size_t n = std::min<size_t>(Pending(), 1u << 20);
            int sent = (int)send(s, sockOut.data() + outPos, (int)n, kSendFlags);
            if (sent < 0) {
                return WouldBlock(SockErr());
            }
            outPos += (size_t)sent;
        }
        if (outPos > 0 && (outPos == sockOut.size() || outPos > (1u << 20))) {
            sockOut.erase(0, outPos);
            outPos = 0;
        }
        return true;
    }

    static int BioSend(void* ctx, const unsigned char* buf, size_t len) {
        // Everything is accepted here and goes out as the socket allows;
        // Pump keeps the backlog small.
        static_cast<Conn*>(ctx)->sockOut.append((const char*)buf, len);
        return (int)len;
    }

    static int BioRecv(void* ctx, unsigned char* buf, size_t len) {
        Conn* c = static_cast<Conn*>(ctx);
        if (c->preTlsPos < c->preTls.size()) {
            size_t n = std::min(len, c->preTls.size() - c->preTlsPos);
            memcpy(buf, c->preTls.data() + c->preTlsPos, n);
            c->preTlsPos += n;
            return (int)n;
        }
        int got = (int)recv(c->s, (char*)buf, (int)std::min<size_t>(len, 1u << 20), 0);
        if (got > 0) {
            c->gotBytes = true;
            return got;
        }
        if (got == 0) {
            return 0;
        }
        int e = SockErr();
        if (WouldBlock(e)) {
            return MBEDTLS_ERR_SSL_WANT_READ;
        }
        c->recvError = e;
        return kBioRecvFailed;
    }
};

// ---------------------------------------------------------------------------
// The transport
// ---------------------------------------------------------------------------

class TransportImpl : public LoopThread {
public:
    TransportImpl(Dispatcher d, INetTransport* owner) :
        _dispatch(std::move(d)), _owner(owner), _alive(std::make_shared<std::atomic<bool>>(true)) {
        _folder = MakeFolder();
    }
    ~TransportImpl() override {
        if (_listen != kNoSock) {
            CloseSock(_listen);
        }
        std::error_code ec;
        fs::remove_all(_folder, ec);
    }

    std::shared_ptr<TransportImpl> Self() { return std::static_pointer_cast<TransportImpl>(shared_from_this()); }

    // ---- called on the session thread ----

    // `salt`: a successor's derived salt (host migration); else a new one.
    bool Listen(const std::string& code, const SessionAdvert& advert, std::string& error, const std::string* givenSalt = nullptr) {
        if (_state.load() != NetState::Idle) {
            error = "The transport is already in use.";
            return false;
        }
        std::string salt = givenSalt != nullptr ? *givenSalt : Wire::NewSalt();
        auto tls = std::make_shared<TlsConfig>(true, Portable::DeriveKey(code, salt));
        if (!tls->ok) {
            error = "Could not set up the secure connection.";
            return false;
        }
        Sock ls = OpenListener(error);
        if (ls == kNoSock) {
            _state = NetState::Failed;
            return false;
        }
        int port = LocalPort(ls);
        _isHost = true;
        _localId = kLanHostPeer;
        _state = NetState::Listening;
        Sync([&] {
            _tls = tls;
            _salt = salt;
            _serviceName = Wire::ServiceNameFor(salt);
            _listen = ls;
            _port = port;
            Advertise(advert);
        });
        SetState(NetState::Listening, "");
        return true;
    }

    void Connect(const std::string& sessionId, const std::string& code) {
        if (_state.load() != NetState::Idle) {
            return;
        }
        _isHost = false;
        SetState(NetState::Connecting, "");
        std::weak_ptr<TransportImpl> w = Self();
        Post([w, sessionId, code] {
            if (auto self = w.lock()) {
                self->StartConnect(sessionId, code);
            }
        });
    }

    void Close() {
        _alive->store(false);
        if (_state.load() == NetState::Closed) {
            return;
        }
        _state = NetState::Closed;
        Sync([this] { CloseOnThread(); });
    }

    NetState State() const { return _state.load(); }
    PeerId LocalPeer() const { return _localId.load(); }

    void Send(PeerId to, const Message& m) {
        std::string frame;
        if (!MessageFrame(to, m, frame)) {
            return;
        }
        std::weak_ptr<TransportImpl> w = Self();
        Post([w, to, frame] {
            if (auto self = w.lock()) {
                self->Enqueue(to, frame);
            }
        });
    }

    void Broadcast(const Message& m) {
        std::string frame;
        if (!MessageFrame(kNoPeer, m, frame)) {
            return;
        }
        std::weak_ptr<TransportImpl> w = Self();
        Post([w, frame] {
            auto self = w.lock();
            if (!self) {
                return;
            }
            std::vector<PeerId> ids;
            for (auto& [id, cn] : self->_conns) {
                if (cn->joined) {
                    ids.push_back(id);
                }
            }
            for (PeerId id : ids) {
                self->Enqueue(id, frame);
            }
        });
    }

    void SendBlob(PeerId to, BlobId id, const std::string& path) {
        if (_state.load() == NetState::Closed) {
            return;
        }
        FILE* src = Files::OpenForCopy(path);
        if (src == nullptr) {
            spdlog::warn("collab: could not open {} to send it", path);
            PostBlobDone(to, id, false);
            return;
        }
        std::string copy = (fs::path(_folder) / ("out-" + std::to_string(++_outCopies))).string();
        std::weak_ptr<TransportImpl> w = Self();
        Files::FileCopier::Job job;
        job.src = src;
        job.dest = copy;
        job.done = [w, to, id, copy](bool ok, uint64_t size) {
            auto self = w.lock();
            std::error_code ec;
            if (!self) {
                fs::remove(copy, ec);
                return;
            }
            if (!ok || size > Wire::kMaxBlobBytes) {
                spdlog::warn("collab: could not copy a file to send it");
                fs::remove(copy, ec);
                self->PostBlobDone(to, id, false);
                return;
            }
            self->Post([w, to, id, copy, size] {
                auto self2 = w.lock();
                std::error_code e2;
                if (!self2) {
                    fs::remove(copy, e2);
                    return;
                }
                auto it = self2->_conns.find(to);
                if (it == self2->_conns.end() || !it->second->joined || it->second->gone || self2->_closed) {
                    fs::remove(copy, e2);
                    self2->PostBlobDone(to, id, false);
                    return;
                }
                OutBlob b;
                b.id = id;
                b.path = copy;
                b.size = size;
                it->second->blobs.push_back(b);
                self2->Pump(it->second);
            });
        };
        _copier->Add(std::move(job));
    }

    std::vector<std::string> JoinAddresses() {
        int port = 0;
        Sync([&] { port = _listen != kNoSock && !_closed ? _port : 0; });
        std::vector<std::string> out;
        if (port == 0) {
            return out;
        }
        for (const auto& ip : ip_utils::GetLocalIPs()) {
            if (ip.rfind("127.", 0) == 0 || ip.rfind("169.254.", 0) == 0) {
                continue;
            }
            out.push_back(ip + ":" + std::to_string(port));
        }
        return out;
    }

    std::string DirectSessionId(const std::string& address) {
        std::string id;
        Sync([&] {
            if (_listen != kNoSock) {
                id = "tcp:" + address + ":" + std::to_string(_port) + ":" + Wire::HexOf(_salt);
            }
        });
        return id;
    }

    void SetBackoff(const LanTest::PairingBackoff& b) {
        Sync([&] { _backoff = b; });
    }

    LanTest::PairingStatus Pairing() {
        LanTest::PairingStatus st;
        Sync([&] {
            st.failedHandshakes = _failedHandshakes;
            st.refusedConnections = _refused;
            st.windowMs = _windowMs;
            st.refusing = NowMs() < _refuseUntil;
        });
        return st;
    }

    void SendRaw(PeerId peer, const std::string& bytes) {
        std::weak_ptr<TransportImpl> w = Self();
        Post([w, peer, bytes] {
            if (auto self = w.lock()) {
                self->Enqueue(peer, bytes);
            }
        });
    }

    bool SocketOptions(PeerId peer, Portable::SocketOptions& o) {
        bool found = false;
        Sync([&] {
            auto it = _conns.find(peer);
            if (it == _conns.end() || it->second->s == kNoSock) {
                return;
            }
            found = ReadSocketOptions(it->second->s, o);
        });
        return found;
    }

    bool Advertising() {
        bool on = false;
        Sync([&] {
#if defined(_WIN32) && !defined(XL_COLLAB_DNSSD)
            if (_winReg != nullptr && _winReg->Failed()) {
                _advertising = false;
            }
#endif
            on = _advertising;
        });
        return on;
    }

    const std::string& Folder() const { return _folder; }

protected:
    // ---- everything below runs on the socket thread ----

    void Watches(std::vector<Watch>& out) override {
        std::weak_ptr<TransportImpl> w = Self();
        if (_listen != kNoSock && !_closed) {
            out.push_back({ _listen, POLLIN, [this](short) { Accept(); } });
        }
#if defined(XL_COLLAB_DNSSD)
        if (_registerRef != nullptr) {
            out.push_back({ (Sock)DNSServiceRefSockFD(_registerRef), POLLIN, [this](short) {
                               if (_registerRef != nullptr && DNSServiceProcessResult(_registerRef) != kDNSServiceErr_NoError) {
                                   DNSServiceRefDeallocate(_registerRef);
                                   _registerRef = nullptr;
                                   _advertising = false;
                               }
                           } });
        }
#endif
        auto add = [&](const std::shared_ptr<Conn>& cn) {
#if defined(XL_COLLAB_DNSSD)
            for (DNSServiceRef* r : { &cn->resolveRef, &cn->addrRef }) {
                if (*r != nullptr) {
                    std::weak_ptr<Conn> wc = cn;
                    out.push_back({ (Sock)DNSServiceRefSockFD(*r), POLLIN, [r, wc](short) {
                                       auto c = wc.lock();
                                       if (c && *r != nullptr) {
                                           DNSServiceProcessResult(*r);
                                       }
                                   } });
                }
            }
#endif
            if (cn->s == kNoSock) {
                return;
            }
            short ev = POLLIN;
            if (cn->phase == Conn::Phase::Connecting || cn->WantsWrite()) {
                ev |= POLLOUT;
            }
            std::weak_ptr<Conn> wc = cn;
            out.push_back({ cn->s, ev, [this, wc](short re) {
                               if (auto c = wc.lock()) {
                                   OnSocket(c, re);
                               }
                           } });
        };
        for (auto& [id, cn] : _conns) {
            add(cn);
        }
        for (auto& cn : _pendingConns) {
            add(cn);
        }
        for (auto& cn : _lingering) {
            add(cn);
        }
    }

    int64_t NextDeadline() override {
        int64_t next = 0;
        auto consider = [&](int64_t t) {
            if (t > 0 && (next == 0 || t < next)) {
                next = t;
            }
        };
        for (auto& [id, cn] : _conns) {
            consider(cn->deadline);
            if (cn->moreToRead) {
                consider(1);
            }
        }
        for (auto& cn : _pendingConns) {
            consider(cn->deadline);
        }
        for (auto& cn : _lingering) {
            consider(cn->deadline);
        }
        return next;
    }

    void OnTimers(int64_t now) override {
        std::vector<std::shared_ptr<Conn>> expired;
        std::vector<std::shared_ptr<Conn>> reading;
        for (auto& [id, cn] : _conns) {
            if (cn->deadline > 0 && now >= cn->deadline) {
                expired.push_back(cn);
            } else if (cn->moreToRead) {
                cn->moreToRead = false;
                reading.push_back(cn);
            }
        }
        // Not while walking _conns: reading can end a connection, which
        // removes it from the map.
        for (auto& cn : reading) {
            if (!cn->gone) {
                ReadOpen(cn);
            }
        }
        for (auto& cn : _pendingConns) {
            if (cn->deadline > 0 && now >= cn->deadline) {
                expired.push_back(cn);
            }
        }
        for (auto& cn : expired) {
            if (cn->gone) {
                continue;
            }
            if (_isHost) {
                // Nobody who can't finish the handshake gets to hold a connection.
                Gone(cn, "", false);
            } else {
                Gone(cn,
                     cn->phase == Conn::Phase::Resolving  ? "The session is no longer on the network."
                     : cn->phase == Conn::Phase::SaltWait ? NotByAddress()
                                                          : "The host did not answer.",
                     false);
            }
        }
        for (auto it = _lingering.begin(); it != _lingering.end();) {
            if ((*it)->s == kNoSock || now >= (*it)->deadline) {
                it = _lingering.erase(it);
            } else {
                ++it;
            }
        }
    }

    bool Finished() override { return _closed && _lingering.empty(); }

    void OnThreadEnd() override {
        _conns.clear();
        _pendingConns.clear();
        _lingering.clear();
        StopAdvertising();
    }

private:
    bool MessageFrame(PeerId to, const Message& m, std::string& frame) {
        if (_state.load() == NetState::Closed) {
            return false;
        }
        std::string payload = EncodeMessage(m);
        if (payload.size() > Wire::kMaxMessageBytes) {
            std::string why = "a message of " + std::to_string(payload.size()) + " bytes is too large to send";
            spdlog::warn("collab: {}", why);
            Message summary = SendFailedSummary(m);
            Post2([to, summary, why](INetTransport& t) {
                if (t.onSendFailed) {
                    t.onSendFailed(to, summary, why);
                }
            });
            return false;
        }
        frame = Wire::EncodeFrame(Wire::FrameType::Message, payload);
        return true;
    }

    void Post2(std::function<void(INetTransport&)> fn) {
        auto alive = _alive;
        INetTransport* owner = _owner;
        _dispatch([alive, owner, fn]() {
            if (alive->load()) {
                fn(*owner);
            }
        });
    }

    void PostBlobDone(PeerId to, BlobId id, bool sent) {
        Post2([to, id, sent](INetTransport& t) {
            if (t.onBlobDone) {
                t.onBlobDone(to, id, sent);
            }
        });
    }

    void PostBlobProgress(PeerId from, BlobId id, uint64_t received, uint64_t size) {
        Post2([from, id, received, size](INetTransport& t) {
            if (t.onBlobProgress) {
                t.onBlobProgress(from, id, received, size);
            }
        });
    }

    void SetState(NetState s, const std::string& reason) {
        if (_state.load() == NetState::Closed) {
            return;
        }
        _state = s;
        Post2([s, reason](INetTransport& t) {
            if (t.onState) {
                t.onState(s, reason);
            }
        });
    }

    static Sock OpenListener(std::string& error) {
        NetStartup();
        // One dual-stack socket where possible: a Bonjour client may try the
        // host's IPv6 address first.
        Sock s = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
        if (s != kNoSock) {
            int off = 0;
            setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&off, sizeof(off));
            sockaddr_in6 a{};
            a.sin6_family = AF_INET6;
            a.sin6_addr = in6addr_any;
            if (bind(s, (sockaddr*)&a, sizeof(a)) != 0 || listen(s, 16) != 0) {
                CloseSock(s);
                s = kNoSock;
            }
        }
        if (s == kNoSock) {
            s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (s == kNoSock) {
                error = "Could not open a network listener.";
                return kNoSock;
            }
            sockaddr_in a{};
            a.sin_family = AF_INET;
            a.sin_addr.s_addr = htonl(INADDR_ANY);
            if (bind(s, (sockaddr*)&a, sizeof(a)) != 0 || listen(s, 16) != 0) {
                error = "Could not listen for participants (error " + std::to_string(SockErr()) + ").";
                CloseSock(s);
                return kNoSock;
            }
        }
        SetNonBlocking(s);
        return s;
    }

    static int LocalPort(Sock s) {
        sockaddr_storage a{};
        socklen_t len = sizeof(a);
        if (getsockname(s, (sockaddr*)&a, &len) != 0) {
            return 0;
        }
        if (a.ss_family == AF_INET6) {
            return ntohs(((sockaddr_in6*)&a)->sin6_port);
        }
        return ntohs(((sockaddr_in*)&a)->sin_port);
    }

    static bool ReadSocketOptions(Sock s, Portable::SocketOptions& o) {
        int v = 0;
        socklen_t len = sizeof(v);
        o = {};
        if (getsockopt(s, IPPROTO_TCP, TCP_NODELAY, (char*)&v, &len) == 0) {
            o.noDelay = v != 0;
        }
        v = 0;
        len = sizeof(v);
        if (getsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (char*)&v, &len) == 0) {
            o.keepAlive = v != 0;
        }
#ifdef _WIN32
        // SIO_KEEPALIVE_VALS can't be read back; TCP_KEEPIDLE / TCP_KEEPINTVL
        // report it on Windows 10 1709 and later.
#ifdef TCP_KEEPIDLE
        DWORD d = 0;
        int dl = sizeof(d);
        if (getsockopt(s, IPPROTO_TCP, TCP_KEEPIDLE, (char*)&d, &dl) == 0) {
            o.idleSec = (int)d;
        }
        d = 0;
        dl = sizeof(d);
        if (getsockopt(s, IPPROTO_TCP, TCP_KEEPINTVL, (char*)&d, &dl) == 0) {
            o.intervalSec = (int)d;
        }
        d = 0;
        dl = sizeof(d);
        if (getsockopt(s, IPPROTO_TCP, TCP_KEEPCNT, (char*)&d, &dl) == 0) {
            o.count = (int)d;
        }
#endif
#else
        v = 0;
        len = sizeof(v);
#ifdef TCP_KEEPIDLE
        if (getsockopt(s, IPPROTO_TCP, TCP_KEEPIDLE, &v, &len) == 0) {
            o.idleSec = v;
        }
#elif defined(TCP_KEEPALIVE)
        if (getsockopt(s, IPPROTO_TCP, TCP_KEEPALIVE, &v, &len) == 0) {
            o.idleSec = v;
        }
#endif
        v = 0;
        len = sizeof(v);
        if (getsockopt(s, IPPROTO_TCP, TCP_KEEPINTVL, &v, &len) == 0) {
            o.intervalSec = v;
        }
        v = 0;
        len = sizeof(v);
        if (getsockopt(s, IPPROTO_TCP, TCP_KEEPCNT, &v, &len) == 0) {
            o.count = v;
        }
#endif
        return true;
    }

    // ---- advertising ----

    void Advertise(const SessionAdvert& advert) {
        std::map<std::string, std::string> txt{
            { "seq", TxtValue(advert.sequenceName) }, { "host", TxtValue(advert.hostName) }, { "kind", TxtValue(advert.deviceKind) },
            { "proto", std::to_string(advert.protocol) }, { "salt", Wire::HexOf(_salt) },
        };
#if defined(XL_COLLAB_DNSSD)
        TXTRecordRef rec;
        TXTRecordCreate(&rec, 0, nullptr);
        for (auto& [k, v] : txt) {
            TXTRecordSetValue(&rec, k.c_str(), (uint8_t)v.size(), v.data());
        }
        DNSServiceErrorType err =
            DNSServiceRegister(&_registerRef, 0, 0, _serviceName.c_str(), Wire::kServiceType, nullptr, nullptr, htons((uint16_t)_port),
                               TXTRecordGetLength(&rec), TXTRecordGetBytesPtr(&rec), &TransportImpl::OnRegistered, this);
        TXTRecordDeallocate(&rec);
        if (err != kDNSServiceErr_NoError) {
            _registerRef = nullptr;
            spdlog::warn("collab: could not advertise the session: {}", DnsSdError(err));
            return;
        }
        _advertising = true;
#elif defined(_WIN32)
        const WinDns& api = Dns();
        if (!api.registerOk) {
            spdlog::warn("collab: this Windows can't advertise sessions; joiners can use the host's address");
            return;
        }
        wchar_t host[256] = { 0 };
        DWORD hostLen = 255;
        if (!GetComputerNameExW(ComputerNameDnsHostname, host, &hostLen)) {
            return;
        }
        std::wstring hostName = std::wstring(host) + L".local";
        std::wstring name = FullServiceName(_serviceName);
        std::vector<std::wstring> keys, values;
        for (auto& [k, v] : txt) {
            keys.push_back(Wide(k));
            values.push_back(Wide(v));
        }
        std::vector<PCWSTR> kp, vp;
        for (size_t i = 0; i < keys.size(); ++i) {
            kp.push_back(keys[i].c_str());
            vp.push_back(values[i].c_str());
        }
        IP4_ADDRESS ip4 = 0;
        PIP4_ADDRESS pIp4 = nullptr;
        for (const auto& ip : ip_utils::GetLocalIPs()) {
            in_addr a{};
            if (ip.rfind("127.", 0) != 0 && ip.rfind("169.254.", 0) != 0 && inet_pton(AF_INET, ip.c_str(), &a) == 1) {
                ip4 = a.s_addr;
                pIp4 = &ip4;
                break;
            }
        }
        PDNS_SERVICE_INSTANCE inst = api.construct(name.c_str(), hostName.c_str(), pIp4, nullptr, (WORD)_port, 0, 0, (DWORD)kp.size(), kp.data(), vp.data());
        if (inst == nullptr) {
            return;
        }
        _winReg = WinRegistration::Start(inst);
        if (!_winReg) {
            return;
        }
        _advertising = true;
#else
        (void)txt;
#endif
    }

    void StopAdvertising() {
#if defined(XL_COLLAB_DNSSD)
        if (_registerRef != nullptr) {
            DNSServiceRefDeallocate(_registerRef);
            _registerRef = nullptr;
        }
#elif defined(_WIN32)
        if (_winReg) {
            WinRegistration::Stop(_winReg);
            _winReg = nullptr;
        }
#endif
        _advertising = false;
    }

#if defined(XL_COLLAB_DNSSD)
    static void DNSSD_API OnRegistered(DNSServiceRef, DNSServiceFlags, DNSServiceErrorType err, const char* name, const char*, const char*, void* ctx) {
        auto* self = static_cast<TransportImpl*>(ctx);
        if (err == kDNSServiceErr_NoError && name != nullptr) {
            self->_serviceName = name;
        } else if (err != kDNSServiceErr_NoError) {
            spdlog::warn("collab: advertising the session failed: {}", DnsSdError(err));
        }
    }
#endif
#ifdef _WIN32
#endif

    // ---- host side ----

    void Accept() {
        while (true) {
            sockaddr_storage a{};
            socklen_t len = sizeof(a);
            Sock s = accept(_listen, (sockaddr*)&a, &len);
            if (s == kNoSock) {
                return;
            }
            if (_closed || NowMs() < _refuseUntil) {
                ++_refused;
                CloseSock(s);
                continue;
            }
            SetNonBlocking(s);
            ConfigureTcp(s);
            auto cn = std::make_shared<Conn>();
            cn->s = s;
            cn->id = _nextId++;
            cn->phase = Conn::Phase::Sniff;
            cn->deadline = NowMs() + kHandshakeTimeoutMs;
            _conns[cn->id] = cn;
        }
    }

    // The first bytes from a new participant: a TLS handshake, or (once) a
    // request for the salt from a client that found us by address.
    void Sniff(const std::shared_ptr<Conn>& cn) {
        char buf[64];
        int got = (int)recv(cn->s, buf, sizeof(buf), 0);
        if (got == 0 || (got < 0 && !WouldBlock(SockErr()))) {
            Gone(cn, "", false);
            return;
        }
        if (got < 0) {
            return;
        }
        cn->preTls.append(buf, (size_t)got);
        const std::string ask(Wire::kSaltRequest);
        if ((unsigned char)cn->preTls[0] == 0x16) {
            StartTls(cn, true);
            return;
        }
        if (cn->preTls.size() < ask.size()) {
            if (ask.compare(0, cn->preTls.size(), cn->preTls) != 0) {
                Gone(cn, "", false);
            }
            return;
        }
        if (cn->preTls.compare(0, ask.size(), ask) != 0 || cn->needSalt) {
            Gone(cn, "", false);
            return;
        }
        // Only one salt request per connection; TLS follows on it.
        cn->needSalt = true;
        cn->preTls.erase(0, ask.size());
        cn->sockOut += Wire::SaltReply(_salt);
        if (!cn->Flush()) {
            Gone(cn, "", false);
            return;
        }
        if (!cn->preTls.empty()) {
            if ((unsigned char)cn->preTls[0] != 0x16) {
                Gone(cn, "", false);
                return;
            }
            StartTls(cn, true);
        }
    }

    // ---- client side ----

    void StartConnect(const std::string& sessionId, const std::string& code) {
        if (_closed) {
            return;
        }
        auto cn = std::make_shared<Conn>();
        cn->id = kLanHostPeer;
        cn->deadline = NowMs() + kConnectTimeoutMs;
        _code = code;
        std::string host, port, salt;
        if (sessionId.rfind("tcp:", 0) == 0) {
            // tcp:<address>:<port>:<salt hex>
            size_t a = sessionId.find(':', 4);
            size_t b = a == std::string::npos ? a : sessionId.find(':', a + 1);
            if (b == std::string::npos || !Wire::SaltFromHex(sessionId.substr(b + 1), salt)) {
                SetState(NetState::Failed, "This is not a session xLights can join.");
                return;
            }
            host = sessionId.substr(4, a - 4);
            port = sessionId.substr(a + 1, b - a - 1);
        } else if (sessionId.rfind(Wire::kAddressPrefix, 0) == 0) {
            if (!Wire::SplitHostPort(sessionId.substr(strlen(Wire::kAddressPrefix)), host, port)) {
                SetState(NetState::Failed, "That is not an address xLights can join.");
                return;
            }
            cn->needSalt = true;
        } else if (Wire::SaltFromServiceName(sessionId, salt)) {
            cn->phase = Conn::Phase::Resolving;
            _clientSalt = salt;
            _pendingConns.push_back(cn);
            Resolve(cn, sessionId);
            return;
        } else {
            SetState(NetState::Failed, "This is not a session xLights can join.");
            return;
        }
        _clientSalt = salt;
        _pendingConns.push_back(cn);
        BeginTcp(cn, host, port);
    }

    void Resolve(const std::shared_ptr<Conn>& cn, const std::string& name) {
#if defined(XL_COLLAB_DNSSD)
        _resolving = cn;
        DNSServiceErrorType err =
            DNSServiceResolve(&cn->resolveRef, 0, 0, name.c_str(), Wire::kServiceType, "local.", &TransportImpl::OnResolved, this);
        if (err != kDNSServiceErr_NoError) {
            cn->resolveRef = nullptr;
            Gone(cn, DnsSdError(err), false);
        }
#elif defined(_WIN32)
        if (!Dns().browseOk) {
            Gone(cn, "This Windows can't find sessions on the network; join by address instead.", false);
            return;
        }
        struct Ctx {
            std::weak_ptr<TransportImpl> self;
            std::wstring name;
            DNS_SERVICE_CANCEL cancel{};
            DNS_SERVICE_RESOLVE_REQUEST req{};
        };
        auto* ctx = new Ctx{ Self(), FullServiceName(name) };
        DNS_SERVICE_RESOLVE_REQUEST& req = ctx->req;
        req.Version = DNS_QUERY_REQUEST_VERSION1;
        req.QueryName = const_cast<PWSTR>(ctx->name.c_str());
        req.pQueryContext = ctx;
        req.pResolveCompletionCallback = [](DWORD status, PVOID context, PDNS_SERVICE_INSTANCE inst) {
            std::unique_ptr<Ctx> c(static_cast<Ctx*>(context));
            std::string ip;
            int port = 0;
            if (status == ERROR_SUCCESS && inst != nullptr) {
                port = inst->wPort;
                if (inst->ip4Address != nullptr) {
                    in_addr a{};
                    a.s_addr = *inst->ip4Address;
                    char buf[INET_ADDRSTRLEN] = { 0 };
                    if (inet_ntop(AF_INET, &a, buf, sizeof(buf)) != nullptr) {
                        ip = buf;
                    }
                }
            }
            if (inst != nullptr) {
                Dns().freeInstance(inst);
            }
            if (auto self = c->self.lock()) {
                self->Post([w = c->self, ip, port] {
                    if (auto s2 = w.lock()) {
                        s2->Resolved(ip, port);
                    }
                });
            }
        };
        _resolving = cn;
        DNS_STATUS st = Dns().resolve(&ctx->req, &ctx->cancel);
        if (st != DNS_REQUEST_PENDING) {
            delete ctx;
            Gone(cn, "The session is no longer on the network.", false);
        }
#else
        (void)name;
        Gone(cn, "This computer can't find sessions on the network; join by address instead.", false);
#endif
    }

    void Resolved(const std::string& ip, int port, const std::string& failure = std::string()) {
        auto cn = _resolving.lock();
        _resolving.reset();
        if (!cn || cn->gone || _closed || cn->phase != Conn::Phase::Resolving) {
            return;
        }
        cn->DropDns();
        if (ip.empty() || port == 0) {
            Gone(cn, failure.empty() ? "The session is no longer on the network." : failure, false);
            return;
        }
        BeginTcp(cn, ip, std::to_string(port));
    }

#if defined(XL_COLLAB_DNSSD)
    static void DNSSD_API OnResolved(DNSServiceRef, DNSServiceFlags, uint32_t ifIndex, DNSServiceErrorType err, const char*, const char* target,
                                     uint16_t port, uint16_t, const unsigned char*, void* ctx) {
        auto* self = static_cast<TransportImpl*>(ctx);
        auto cn = self->_resolving.lock();
        if (!cn || cn->gone || cn->addrRef != nullptr) {
            return;
        }
        std::string failure;
        if (err != kDNSServiceErr_NoError || target == nullptr) {
            failure = DnsSdError(err);
        } else {
            self->_resolvedPort = ntohs(port);
            // An explicit A query: Avahi's dns_sd shim has no DNSServiceGetAddrInfo.
            if (DNSServiceQueryRecord(&cn->addrRef, 0, ifIndex, target, kDNSServiceType_A, kDNSServiceClass_IN, &TransportImpl::OnAddress, self) !=
                kDNSServiceErr_NoError) {
                cn->addrRef = nullptr;
                failure = "The session is no longer on the network.";
            }
        }
        if (!failure.empty()) {
            // Not from inside the callback: Gone deallocates this ref.
            self->Post([w = std::weak_ptr<TransportImpl>(self->Self()), failure] {
                if (auto s = w.lock()) {
                    s->Resolved("", 0, failure);
                }
            });
        }
    }

    static void DNSSD_API OnAddress(DNSServiceRef, DNSServiceFlags flags, uint32_t, DNSServiceErrorType err, const char*, uint16_t rrtype, uint16_t,
                                    uint16_t rdlen, const void* rdata, uint32_t, void* ctx) {
        auto* self = static_cast<TransportImpl*>(ctx);
        if (err != kDNSServiceErr_NoError || (flags & kDNSServiceFlagsAdd) == 0 || rrtype != kDNSServiceType_A || rdlen != 4 || rdata == nullptr) {
            return;
        }
        char buf[INET_ADDRSTRLEN] = { 0 };
        inet_ntop(AF_INET, rdata, buf, sizeof(buf));
        // Resolved() deallocates the refs; not from inside their callback.
        std::string ip = buf;
        int port = self->_resolvedPort;
        std::weak_ptr<TransportImpl> w = self->Self();
        self->Post([w, ip, port] {
            if (auto s = w.lock()) {
                s->Resolved(ip, port);
            }
        });
    }
#endif

    // The name lookup can take seconds (a typed name nobody answers for), so
    // it runs on its own thread: Close, which waits for the socket thread,
    // must not wait for it too.
    void BeginTcp(const std::shared_ptr<Conn>& cn, const std::string& host, const std::string& port) {
        cn->phase = Conn::Phase::Connecting;
        std::string h = host;
        if (h.size() > 2 && h.front() == '[' && h.back() == ']') {
            h = h.substr(1, h.size() - 2);
        }
        std::weak_ptr<TransportImpl> w = Self();
        std::weak_ptr<Conn> wc = cn;
        std::thread([w, wc, h, host, port] {
            if (auto hook = BeforeLookup()) {
                hook(h);
            }
            addrinfo hints{};
            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;
            addrinfo* res = nullptr;
            std::vector<sockaddr_storage> addrs;
            std::vector<socklen_t> lens;
            if (getaddrinfo(h.c_str(), port.c_str(), &hints, &res) == 0) {
                for (addrinfo* a = res; a != nullptr; a = a->ai_next) {
                    sockaddr_storage ss{};
                    memcpy(&ss, a->ai_addr, std::min<size_t>(a->ai_addrlen, sizeof(ss)));
                    addrs.push_back(ss);
                    lens.push_back((socklen_t)a->ai_addrlen);
                }
            }
            if (res != nullptr) {
                freeaddrinfo(res);
            }
            if (auto self = w.lock()) {
                self->Post([w, wc, host, addrs, lens] {
                    auto s = w.lock();
                    auto c = wc.lock();
                    if (s && c) {
                        s->LookedUp(c, host, addrs, lens);
                    }
                });
            }
        }).detach();
    }

    void LookedUp(const std::shared_ptr<Conn>& cn, const std::string& host, const std::vector<sockaddr_storage>& addrs,
                  const std::vector<socklen_t>& lens) {
        if (cn->gone || _closed || cn->phase != Conn::Phase::Connecting || cn->s != kNoSock || !cn->addrs.empty()) {
            return;
        }
        if (addrs.empty()) {
            Gone(cn, "Could not find the host " + host + ".", false);
            return;
        }
        cn->addrs = addrs;
        cn->addrLens = lens;
        cn->addrAt = 0;
        TryNextAddress(cn);
    }

    void TryNextAddress(const std::shared_ptr<Conn>& cn) {
        cn->CloseSocket();
        while (cn->addrAt < cn->addrs.size()) {
            const sockaddr_storage& a = cn->addrs[cn->addrAt];
            socklen_t len = cn->addrLens[cn->addrAt];
            ++cn->addrAt;
            Sock s = socket(a.ss_family, SOCK_STREAM, IPPROTO_TCP);
            if (s == kNoSock) {
                cn->lastConnectError = SockErr();
                continue;
            }
            SetNonBlocking(s);
            ConfigureTcp(s);
            if (connect(s, (const sockaddr*)&a, len) == 0) {
                cn->s = s;
                Connected(cn);
                return;
            }
            int e = SockErr();
            if (ConnectPending(e)) {
                cn->s = s;
                return;
            }
            cn->lastConnectError = e;
            CloseSock(s);
        }
        Gone(cn, ConnectErrorText(cn->lastConnectError), false);
    }

    void Connected(const std::shared_ptr<Conn>& cn) {
        if (cn->needSalt) {
            cn->phase = Conn::Phase::SaltWait;
            cn->deadline = NowMs() + kSaltReplyTimeoutMs;
            cn->sockOut += Wire::kSaltRequest;
            if (!cn->Flush()) {
                Gone(cn, "The host closed the connection.", false);
            }
            return;
        }
        StartTls(cn, false);
    }

    void SaltReply(const std::shared_ptr<Conn>& cn) {
        char buf[64];
        int got = (int)recv(cn->s, buf, sizeof(buf), 0);
        if (got < 0 && WouldBlock(SockErr())) {
            return;
        }
        if (got <= 0) {
            Gone(cn, NotByAddress(), false);
            return;
        }
        cn->gotBytes = true;
        cn->preTls.append(buf, (size_t)got);
        size_t nl = cn->preTls.find('\n');
        if (nl == std::string::npos) {
            if (cn->preTls.size() > 64) {
                Gone(cn, NotByAddress(), false);
            }
            return;
        }
        std::string salt;
        if (!Wire::SaltFromServiceName(std::string_view(cn->preTls).substr(0, nl), salt) || nl + 1 != cn->preTls.size()) {
            Gone(cn, NotByAddress(), false);
            return;
        }
        cn->preTls.clear();
        _clientSalt = salt;
        cn->needSalt = false;
        cn->deadline = NowMs() + kConnectTimeoutMs;
        StartTls(cn, false);
    }

    // ---- both ----

    void StartTls(const std::shared_ptr<Conn>& cn, bool server) {
        if (!server) {
            // Key stretching takes a moment; it runs here, off the session thread.
            auto tls = std::make_shared<TlsConfig>(false, Portable::DeriveKey(_code, _clientSalt));
            if (!tls->ok) {
                Gone(cn, "Could not set up the secure connection.", false);
                return;
            }
            _tls = tls;
        }
        if (mbedtls_ssl_setup(&cn->ssl, &_tls->conf) != 0) {
            Gone(cn, "Could not set up the secure connection.", false);
            return;
        }
        cn->sslInit = true;
        mbedtls_ssl_set_bio(&cn->ssl, cn.get(), &Conn::BioSend, &Conn::BioRecv, nullptr);
        cn->phase = Conn::Phase::Handshake;
        Handshake(cn);
    }

    void Handshake(const std::shared_ptr<Conn>& cn) {
        int ret = mbedtls_ssl_handshake(&cn->ssl);
        bool flushed = cn->Flush();
        if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE) {
            if (!flushed) {
                Gone(cn, PairingFailureText(cn), false);
            }
            return;
        }
        if (ret != 0) {
            if (_isHost) {
                spdlog::info("collab: a pairing handshake failed: {}", TlsErrorText(ret));
            }
            Gone(cn, PairingFailureText(cn), false);
            return;
        }
        if (!flushed) {
            Gone(cn, "The connection was closed.", false);
            return;
        }
        OnReady(cn);
    }

    static std::string NotByAddress() { return "That address isn't hosting a session that can be joined by address."; }

    std::string PairingFailureText(const std::shared_ptr<Conn>& cn) {
        if (!cn->gotBytes) {
            return "The host is not accepting connections right now.";
        }
        return "The pairing code was not accepted.";
    }

    void OnReady(const std::shared_ptr<Conn>& cn) {
        cn->ready = true;
        cn->phase = Conn::Phase::Open;
        cn->deadline = 0;
        if (_isHost) {
            cn->control.push_front(Wire::WelcomeFrame(cn->id));
            cn->joined = true;
            PeerId id = cn->id;
            Post2([id](INetTransport& t) {
                if (t.onPeer) {
                    t.onPeer(id, PeerEvent::Joined);
                }
            });
        } else {
            // Until the Welcome arrives the host may still refuse us.
            cn->deadline = NowMs() + kConnectTimeoutMs;
            _pendingConns.erase(std::remove(_pendingConns.begin(), _pendingConns.end(), cn), _pendingConns.end());
            _conns[kLanHostPeer] = cn;
        }
        Pump(cn);
        ReadOpen(cn);
    }

    void OnSocket(const std::shared_ptr<Conn>& cn, short re) {
        if (cn->gone && cn->phase != Conn::Phase::Lingering) {
            return;
        }
        if (cn->phase == Conn::Phase::Lingering) {
            Linger(cn, re);
            return;
        }
        if (cn->phase == Conn::Phase::Connecting) {
            if ((re & (POLLOUT | POLLERR | POLLHUP)) == 0) {
                return;
            }
            int err = 0;
            socklen_t len = sizeof(err);
            getsockopt(cn->s, SOL_SOCKET, SO_ERROR, (char*)&err, &len);
            if (err != 0 || (re & (POLLERR | POLLHUP)) != 0) {
                cn->lastConnectError = err != 0 ? err : cn->lastConnectError;
                TryNextAddress(cn);
                return;
            }
            Connected(cn);
            return;
        }
        if ((re & POLLOUT) != 0 && cn->WantsWrite()) {
            if (!cn->Flush()) {
                Gone(cn, cn->ready ? "The connection was closed." : PairingFailureText(cn), false);
                return;
            }
            if (cn->ready) {
                Pump(cn);
                if (cn->gone) {
                    return;
                }
            }
        }
        if ((re & (POLLIN | POLLERR | POLLHUP)) == 0) {
            return;
        }
        switch (cn->phase) {
        case Conn::Phase::Sniff:
            Sniff(cn);
            break;
        case Conn::Phase::SaltWait:
            SaltReply(cn);
            break;
        case Conn::Phase::Handshake:
            Handshake(cn);
            break;
        case Conn::Phase::Open:
            ReadOpen(cn);
            break;
        default:
            break;
        }
    }

    void ReadOpen(const std::shared_ptr<Conn>& cn) {
        unsigned char buf[64u << 10];
        size_t total = 0;
        while (!cn->gone && !_closed) {
            int ret = mbedtls_ssl_read(&cn->ssl, buf, sizeof(buf));
            if (ret > 0) {
                cn->reader.Feed(buf, (size_t)ret);
                if (!ProcessFrames(cn)) {
                    spdlog::warn("collab: dropping peer {} after a malformed frame", cn->id);
                    Gone(cn, "The other side sent something that isn't part of a session.", false);
                    return;
                }
                total += (size_t)ret;
                if (total >= kReadBurst) {
                    cn->moreToRead = true;
                    return;
                }
                continue;
            }
            if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE) {
                // A record may already be buffered inside TLS.
                if (mbedtls_ssl_check_pending(&cn->ssl)) {
                    cn->moreToRead = true;
                }
                break;
            }
            if (ret == 0 || ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY || ret == MBEDTLS_ERR_SSL_CONN_EOF) {
                Gone(cn, "", true);
                return;
            }
            Gone(cn, cn->joined ? "The connection was closed." : "The secure connection failed.", false);
            return;
        }
        // Reading can produce alerts or other TLS output.
        if (!cn->gone && cn->Pending() > 0 && !cn->Flush()) {
            Gone(cn, "The connection was closed.", false);
        }
    }

    bool ProcessFrames(const std::shared_ptr<Conn>& cn) {
        Wire::Frame f;
        while (!cn->gone) {
            auto r = cn->reader.Next(f);
            if (r == Wire::FrameReader::Result::More) {
                return true;
            }
            if (r == Wire::FrameReader::Result::Bad) {
                return false;
            }
            if (!HandleFrame(cn, f)) {
                return false;
            }
        }
        return true;
    }

    bool HandleFrame(const std::shared_ptr<Conn>& cn, const Wire::Frame& f) {
        PeerId from = cn->id;
        if (f.type == Wire::FrameType::Welcome) {
            uint32_t assigned = 0;
            if (_isHost || cn->joined || !Wire::ReadU32(f.payload, 0, assigned) || assigned == kNoPeer || assigned == kLanHostPeer) {
                return false;
            }
            _localId = assigned;
            cn->joined = true;
            cn->deadline = 0;
            SetState(NetState::Connected, "");
            Post2([](INetTransport& t) {
                if (t.onPeer) {
                    t.onPeer(kLanHostPeer, PeerEvent::Joined);
                }
            });
            return true;
        }
        if (!cn->joined) {
            return false;
        }
        switch (f.type) {
        case Wire::FrameType::Message: {
            auto m = DecodeMessage(f.payload);
            if (!m) {
                return false;
            }
            Message msg = std::move(*m);
            Post2([from, msg](INetTransport& t) {
                if (t.onMessage) {
                    t.onMessage(from, msg);
                }
            });
            return true;
        }
        case Wire::FrameType::BlobStart: {
            uint64_t id = 0, size = 0;
            Wire::ReadU64(f.payload, 0, id);
            Wire::ReadU64(f.payload, 8, size);
            std::string path = (fs::path(_folder) / ("in-" + std::to_string(++_inFiles))).string();
            if (cn->in.Start(path, id, size, Wire::kMaxBlobBytes) != Files::IncomingBlob::Result::Ok) {
                return false;
            }
            PostBlobProgress(from, id, 0, size);
            return true;
        }
        case Wire::FrameType::BlobData: {
            uint64_t id = 0;
            Wire::ReadU64(f.payload, 0, id);
            uint64_t before = cn->in.Received();
            if (cn->in.Data(id, std::string_view(f.payload).substr(8)) != Files::IncomingBlob::Result::Ok) {
                return false;
            }
            if (before / kBlobProgressBytes != cn->in.Received() / kBlobProgressBytes) {
                PostBlobProgress(from, id, cn->in.Received(), cn->in.Size());
            }
            return true;
        }
        case Wire::FrameType::BlobEnd: {
            uint64_t id = 0;
            Wire::ReadU64(f.payload, 0, id);
            std::string path, why;
            switch (cn->in.End(id, path, why)) {
            case Files::IncomingBlob::Result::Received:
                break;
            case Files::IncomingBlob::Result::Failed:
                // Not the peer's fault: the connection stays, and the session
                // asks for the file again once it sees the transfer stall.
                spdlog::warn("collab: dropped a file from peer {}: {}", from, why);
                Post2([from, id, why](INetTransport& t) {
                    if (t.onBlobFailed) {
                        t.onBlobFailed(from, id, why);
                    }
                });
                return true;
            default:
                return false;
            }
            Post2([from, id, path](INetTransport& t) {
                if (t.onBlob) {
                    t.onBlob(from, id, path);
                }
                std::error_code ec;
                fs::remove(path, ec);
            });
            return true;
        }
        default:
            return false;
        }
    }

    void Enqueue(PeerId to, const std::string& frame) {
        auto it = _conns.find(to);
        if (it == _conns.end() || _closed) {
            return;
        }
        auto cn = it->second;
        if (!cn->joined || cn->gone) {
            return;
        }
        cn->control.push_back(frame);
        Pump(cn);
    }

    bool Encrypt(const std::shared_ptr<Conn>& cn, const std::string& frame) {
        size_t at = 0;
        while (at < frame.size()) {
            int ret = mbedtls_ssl_write(&cn->ssl, (const unsigned char*)frame.data() + at, frame.size() - at);
            if (ret <= 0) {
                return false;
            }
            at += (size_t)ret;
        }
        return true;
    }

    // Messages first; blob data only while little is waiting on the socket,
    // so a message never queues behind a whole snapshot.
    void Pump(const std::shared_ptr<Conn>& cn) {
        if (!cn->ready || cn->gone || _closed) {
            return;
        }
        for (int round = 0; round < 16; ++round) {
            bool produced = false;
            while (cn->Pending() < kMaxInFlight) {
                std::string frame;
                if (!cn->control.empty()) {
                    frame = std::move(cn->control.front());
                    cn->control.pop_front();
                } else if (!cn->blobs.empty()) {
                    OutBlob& b = cn->blobs.front();
                    if (!b.started) {
                        b.f = fopen(b.path.c_str(), "rb");
                        if (b.f == nullptr) {
                            std::error_code ec;
                            fs::remove(b.path, ec);
                            PostBlobDone(cn->id, b.id, false);
                            cn->blobs.pop_front();
                            continue;
                        }
                        b.started = true;
                        frame = Wire::BlobStartFrame(b.id, b.size);
                    } else if (b.sent < b.size) {
                        std::string chunk(std::min<uint64_t>(Wire::kBlobChunkBytes, b.size - b.sent), '\0');
                        size_t n = fread(chunk.data(), 1, chunk.size(), b.f);
                        if (n == 0) {
                            Gone(cn, "A file being sent could not be read.", false);
                            return;
                        }
                        chunk.resize(n);
                        b.sent += n;
                        frame = Wire::BlobDataFrame(b.id, chunk);
                    } else {
                        frame = Wire::BlobEndFrame(b.id);
                        fclose(b.f);
                        std::error_code ec;
                        fs::remove(b.path, ec);
                        PostBlobDone(cn->id, b.id, true);
                        cn->blobs.pop_front();
                    }
                } else {
                    break;
                }
                if (!Encrypt(cn, frame)) {
                    Gone(cn, "The secure connection failed.", false);
                    return;
                }
                produced = true;
            }
            size_t before = cn->Pending();
            if (!cn->Flush()) {
                Gone(cn, "The connection was closed.", false);
                return;
            }
            if (!produced || cn->Pending() == before) {
                break;
            }
        }
    }

    // The connection is finished: tell the session if it knew about it.
    void Gone(const std::shared_ptr<Conn>& cn, const std::string& reason, bool graceful) {
        if (cn->gone) {
            return;
        }
        cn->gone = true;
        // Blobs still queued for it never go: the session must not wait for them.
        for (const auto& b : cn->blobs) {
            PostBlobDone(cn->id, b.id, false);
        }
        cn->DropFiles();
        cn->DropDns();
        if (cn->Pending() > 0 && cn->s != kNoSock) {
            // A TLS alert, so the other side learns why.
            cn->Flush();
        }
        cn->CloseSocket();
        auto it = _conns.find(cn->id);
        if (it != _conns.end() && it->second == cn) {
            _conns.erase(it);
        }
        _pendingConns.erase(std::remove(_pendingConns.begin(), _pendingConns.end(), cn), _pendingConns.end());
        if (_closed) {
            return;
        }
        if (_isHost && !cn->ready && cn->phase != Conn::Phase::Sniff) {
            FailedHandshake();
        } else if (_isHost && !cn->ready && !cn->preTls.empty()) {
            // Sent something that is neither TLS nor a salt request.
            FailedHandshake();
        }
        PeerId id = cn->id;
        PeerEvent e = graceful ? PeerEvent::Left : PeerEvent::Lost;
        if (cn->joined) {
            Post2([id, e](INetTransport& t) {
                if (t.onPeer) {
                    t.onPeer(id, e);
                }
            });
        }
        if (!_isHost) {
            std::string why = reason;
            if (cn->joined) {
                why = graceful ? "The host ended the session." : "The connection to the host was lost.";
            } else if (why.empty()) {
                why = "The host closed the connection.";
            }
            SetState(NetState::Failed, why);
        }
    }

    // Guessing the code online is the cheapest attack on a 6-digit code, so
    // every source shares one budget: a good handshake from one participant
    // doesn't lift the pause another caused.
    void FailedHandshake() {
        ++_failedHandshakes;
        if (++_failuresThisRound < _backoff.failures) {
            return;
        }
        _failuresThisRound = 0;
        _windowMs = _windowMs == 0 ? _backoff.firstMs : std::min(_windowMs * 2, _backoff.maxMs);
        _refuseUntil = NowMs() + _windowMs;
        spdlog::warn("collab: {} failed pairing attempts; refusing new connections for {} ms", _failedHandshakes, _windowMs);
    }

    void CloseOnThread() {
        _closed = true;
        StopAdvertising();
        if (_listen != kNoSock) {
            CloseSock(_listen);
            _listen = kNoSock;
        }
        for (auto& cn : _pendingConns) {
            cn->gone = true;
            cn->DropDns();
            cn->CloseSocket();
        }
        _pendingConns.clear();
        int64_t until = NowMs() + kCloseGraceMs;
        for (auto& [id, cn] : _conns) {
            cn->gone = true;
            cn->DropFiles();
            if (!cn->ready || cn->s == kNoSock) {
                cn->CloseSocket();
                continue;
            }
            // Whatever the session already sent (its Bye) still goes out;
            // then the connection closes cleanly, or is cut if it can't.
            bool ok = true;
            for (auto& frame : cn->control) {
                ok = ok && Encrypt(cn, frame);
            }
            cn->control.clear();
            if (ok) {
                mbedtls_ssl_close_notify(&cn->ssl);
            }
            cn->phase = Conn::Phase::Lingering;
            cn->deadline = until;
            Linger(cn, 0);
            if (cn->s != kNoSock) {
                _lingering.push_back(cn);
            }
        }
        _conns.clear();
        std::error_code ec;
        for (auto it = fs::directory_iterator(_folder, ec); !ec && it != fs::directory_iterator(); it.increment(ec)) {
            fs::remove_all(it->path(), ec);
        }
    }

    // Sends what's left, half-closes, and waits for the other side to close
    // so unread data doesn't turn our close into a reset that loses it.
    void Linger(const std::shared_ptr<Conn>& cn, short re) {
        if (!cn->Flush()) {
            cn->CloseSocket();
            return;
        }
        if (cn->Pending() == 0 && !cn->shutWr) {
            cn->shutWr = true;
#ifdef _WIN32
            shutdown(cn->s, SD_SEND);
#else
            shutdown(cn->s, SHUT_WR);
#endif
        }
        if ((re & (POLLIN | POLLHUP | POLLERR)) != 0) {
            char buf[16384];
            while (true) {
                int got = (int)recv(cn->s, buf, sizeof(buf), 0);
                if (got > 0) {
                    continue;
                }
                if (got < 0 && WouldBlock(SockErr())) {
                    break;
                }
                cn->CloseSocket();
                break;
            }
        }
    }

    Dispatcher _dispatch;
    INetTransport* _owner;
    std::shared_ptr<std::atomic<bool>> _alive;
    std::atomic<NetState> _state{ NetState::Idle };
    std::atomic<PeerId> _localId{ kNoPeer };
    bool _isHost = false;
    bool _closed = false;
    Sock _listen = kNoSock;
    int _port = 0;
    std::shared_ptr<TlsConfig> _tls;
    std::string _salt;
    std::string _serviceName;
    std::string _code;
    std::string _clientSalt;
    bool _advertising = false;
    std::map<PeerId, std::shared_ptr<Conn>> _conns;
    std::vector<std::shared_ptr<Conn>> _pendingConns;
    std::vector<std::shared_ptr<Conn>> _lingering;
    std::weak_ptr<Conn> _resolving;
    int _resolvedPort = 0;
    PeerId _nextId = kLanHostPeer + 1;
    std::string _folder;
    std::shared_ptr<Files::FileCopier> _copier = std::make_shared<Files::FileCopier>();
    std::atomic<uint64_t> _outCopies{ 0 };
    LanTest::PairingBackoff _backoff{ kPairingFailuresAllowed, kPairingBackoffFirstMs, kPairingBackoffMaxMs };
    int _failuresThisRound = 0;
    int _failedHandshakes = 0;
    int _refused = 0;
    int64_t _windowMs = 0;
    int64_t _refuseUntil = 0;
    uint64_t _inFiles = 0;
#if defined(XL_COLLAB_DNSSD)
    DNSServiceRef _registerRef = nullptr;
#endif
#ifdef _WIN32
    WinRegistration* _winReg = nullptr;
#endif
};

class LanTransport : public INetTransport {
public:
    explicit LanTransport(Dispatcher d) : _dispatch(d), _impl(std::make_shared<TransportImpl>(std::move(d), this)) { _impl->StartThread(); }
    ~LanTransport() override { _impl->Close(); }

    // Host migration, as the Apple transport does it (same derived salt, same
    // code): a fresh connection replaces what is left of the old one, which
    // makes no more callbacks.
    // Only a session found under its service name can move: the successor's
    // name is derived from that name's salt and found by browsing. One joined
    // by address or by a direct id has neither.
    bool SupportsMigration() const override { return _migratable; }
    bool ListenAsSuccessor(const Succession& s, const SessionAdvert& advert, std::string& error) override {
        std::string salt;
        if (!Wire::SuccessorSalt(s.sessionId, s.successor, salt)) {
            error = "This session can't move to another host.";
            return false;
        }
        Replace();
        _migratable = true;
        return _impl->Listen(s.pairingCode, advert, error, &salt);
    }
    void ConnectToSuccessor(const Succession& s) override {
        std::string id = SuccessorSessionId(s);
        Replace();
        _migratable = !id.empty();
        _impl->Connect(id.empty() ? std::string("-") : id, s.pairingCode);
    }
    std::string SuccessorSessionId(const Succession& s) const override {
        std::string salt;
        return Wire::SuccessorSalt(s.sessionId, s.successor, salt) ? Wire::ServiceNameFor(salt) : std::string();
    }

    PeerId LocalPeer() const override { return _impl->LocalPeer(); }
    void Send(PeerId to, const Message& m) override { _impl->Send(to, m); }
    void Broadcast(const Message& m) override { _impl->Broadcast(m); }
    void SendBlob(PeerId to, BlobId id, const std::string& path) override { _impl->SendBlob(to, id, path); }
    bool Listen(const std::string& code, const SessionAdvert& advert, std::string& error) override {
        _migratable = true;
        return _impl->Listen(code, advert, error);
    }
    void Connect(const std::string& sessionId, const std::string& code) override {
        std::string salt;
        _migratable = Wire::SaltFromServiceName(sessionId, salt);
        _impl->Connect(sessionId, code);
    }
    void Close() override { _impl->Close(); }
    NetState State() const override { return _impl->State(); }
    std::vector<std::string> JoinAddresses() const override { return _impl->JoinAddresses(); }

    TransportImpl& Impl() { return *_impl; }

private:
    void Replace() {
        _impl->Close();
        _impl = std::make_shared<TransportImpl>(_dispatch, this);
        _impl->StartThread();
    }

    bool _migratable = false;
    Dispatcher _dispatch;
    std::shared_ptr<TransportImpl> _impl;
};

// ---------------------------------------------------------------------------
// Browsing
// ---------------------------------------------------------------------------

class BrowserImpl : public LoopThread {
public:
    BrowserImpl(Dispatcher d, ISessionBrowser* owner) : _dispatch(std::move(d)), _owner(owner) {}

    std::shared_ptr<BrowserImpl> Self() { return std::static_pointer_cast<BrowserImpl>(shared_from_this()); }

    void Start() {
        Stop();
        _alive = std::make_shared<std::atomic<bool>>(true);
        auto alive = _alive;
        Sync([this, alive] { StartOnThread(alive); });
    }

    void Stop() {
        if (_alive) {
            _alive->store(false);
        }
        Sync([this] { StopOnThread(); });
    }

    void Shutdown() {
        Stop();
        Sync([this] { _done = true; });
    }

protected:
    void Watches(std::vector<Watch>& out) override {
#if defined(XL_COLLAB_DNSSD)
        if (_browseRef != nullptr) {
            out.push_back({ (Sock)DNSServiceRefSockFD(_browseRef), POLLIN, [this](short) {
                               if (_browseRef != nullptr) {
                                   DNSServiceErrorType e = DNSServiceProcessResult(_browseRef);
                                   if (e != kDNSServiceErr_NoError) {
                                       Error(DnsSdError(e));
                                       StopOnThread();
                                   }
                               }
                           } });
        }
        // Resolves end (and are freed) only here, between poll passes: a
        // watch that runs earlier in a pass can finish another's resolve or
        // stop browsing, so each watch finds its own by name and serial.
        FinishResolves();
        for (auto& [name, r] : _resolves) {
            uint64_t serial = r->serial;
            out.push_back({ (Sock)DNSServiceRefSockFD(r->ref), POLLIN, [this, name = name, serial](short) {
                               auto it = _resolves.find(name);
                               if (it != _resolves.end() && it->second->serial == serial && !it->second->finished) {
                                   DNSServiceProcessResult(it->second->ref);
                               }
                           } });
        }
#else
        (void)out;
#endif
    }
    int64_t NextDeadline() override { return 0; }
    void OnTimers(int64_t) override {}
    bool Finished() override { return _done; }
    void OnThreadEnd() override { StopOnThread(); }

private:
    void Post2(const std::shared_ptr<std::atomic<bool>>& alive, std::function<void(ISessionBrowser&)> fn) {
        ISessionBrowser* owner = _owner;
        _dispatch([alive, owner, fn]() {
            if (alive->load()) {
                fn(*owner);
            }
        });
    }

    void Error(const std::string& why) {
        if (_cur) {
            Post2(_cur, [why](ISessionBrowser& o) {
                if (o.onError) {
                    o.onError(why);
                }
            });
        }
    }

    void Changed() {
        if (!_cur) {
            return;
        }
        std::vector<SessionAdvert> list;
        for (auto& [id, a] : _found) {
            list.push_back(a);
        }
        Post2(_cur, [list](ISessionBrowser& o) {
            if (o.onChanged) {
                o.onChanged(list);
            }
        });
    }

    void StartOnThread(const std::shared_ptr<std::atomic<bool>>& alive) {
        _cur = alive;
        _found.clear();
        _interfaces.clear();
#if defined(XL_COLLAB_DNSSD)
        DNSServiceErrorType e = DNSServiceBrowse(&_browseRef, 0, 0, Wire::kServiceType, "local.", &BrowserImpl::OnBrowse, this);
        if (e != kDNSServiceErr_NoError) {
            _browseRef = nullptr;
            Error(DnsSdError(e));
        }
#elif defined(_WIN32)
        if (!Dns().browseOk) {
            Error("This version of Windows can't find sessions on the network. You can still join by address.");
            return;
        }
        // The request must outlive the call: Windows reads it again later
        // (one on the stack corrupted the heap once the browse was cancelled).
        _winCtx = std::make_shared<WinCtx>();
        _winCtx->self = Self();
        _winCtx->generation = ++_generation;
        _winCtx->query = Wide(Wire::kServiceType) + L".local";
        _winCtx->key = WinContexts().Add(_winCtx);
        _winCtx->req.Version = DNS_QUERY_REQUEST_VERSION1;
        _winCtx->req.QueryName = const_cast<PWSTR>(_winCtx->query.c_str());
        _winCtx->req.pBrowseCallback = &BrowserImpl::OnWinBrowse;
        _winCtx->req.pQueryContext = (PVOID)(uintptr_t)_winCtx->key;
        DNS_STATUS st = Dns().browse(&_winCtx->req, &_winCtx->cancel);
        if (st != DNS_REQUEST_PENDING) {
            WinContexts().Remove(_winCtx->key);
            _winCtx.reset();
            Error("Finding sessions failed (" + std::to_string((int)st) + ").");
            return;
        }
#else
        Error("This computer can't find sessions on the network. You can still join by address.");
#endif
    }

    void StopOnThread() {
#if defined(XL_COLLAB_DNSSD)
        if (_browseRef != nullptr) {
            DNSServiceRefDeallocate(_browseRef);
            _browseRef = nullptr;
        }
        for (auto& [name, r] : _resolves) {
            r->finished = true;
        }
#elif defined(_WIN32)
        if (_winCtx) {
            // Callbacks still in flight (browse or resolve) see generation 0,
            // and a browse callback that comes after the cancel finds no
            // context under its key.
            _winCtx->generation = 0;
            Dns().browseCancel(&_winCtx->cancel);
            WinContexts().Remove(_winCtx->key);
            _winCtx.reset();
        }
        // Resolves still running are dropped by the generation check: a
        // name left here would never be resolved again.
        _winPending.clear();
#endif
        _found.clear();
        _interfaces.clear();
        _cur.reset();
    }

#if defined(XL_COLLAB_DNSSD)
    static void DNSSD_API OnBrowse(DNSServiceRef, DNSServiceFlags flags, uint32_t ifIndex, DNSServiceErrorType err, const char* name, const char* type,
                                   const char* domain, void* ctx) {
        auto* self = static_cast<BrowserImpl*>(ctx);
        if (err != kDNSServiceErr_NoError || name == nullptr) {
            return;
        }
        std::string n = name;
        std::string salt;
        if (!Wire::SaltFromServiceName(n, salt)) {
            return;
        }
        if ((flags & kDNSServiceFlagsAdd) != 0) {
            self->_interfaces[n].insert(ifIndex);
            auto old = self->_resolves.find(n);
            bool resolving = old != self->_resolves.end() && !old->second->finished;
            if (self->_found.count(n) == 0 && !resolving) {
                auto r = std::make_unique<Resolving>();
                r->self = self;
                r->name = n;
                r->serial = ++self->_resolveSerial;
                if (DNSServiceResolve(&r->ref, 0, ifIndex, name, type, domain, &BrowserImpl::OnResolve, r.get()) == kDNSServiceErr_NoError) {
                    if (old != self->_resolves.end()) {
                        // A finished one still waiting to be freed: free it
                        // later, after this pass.
                        self->_retired.push_back(std::move(old->second));
                    }
                    self->_resolves[n] = std::move(r);
                } else {
                    r->ref = nullptr;
                }
            }
        } else {
            auto it = self->_interfaces.find(n);
            if (it != self->_interfaces.end()) {
                it->second.erase(ifIndex);
                if (it->second.empty()) {
                    self->_interfaces.erase(it);
                    auto r = self->_resolves.find(n);
                    if (r != self->_resolves.end()) {
                        r->second->finished = true;
                    }
                    if (self->_found.erase(n) > 0) {
                        self->_dirty = true;
                    }
                }
            }
        }
        if ((flags & kDNSServiceFlagsMoreComing) == 0) {
            if (self->_dirty) {
                self->_dirty = false;
                self->Changed();
            }
        }
    }

    static void DNSSD_API OnResolve(DNSServiceRef, DNSServiceFlags, uint32_t, DNSServiceErrorType err, const char*, const char*, uint16_t,
                                    uint16_t txtLen, const unsigned char* txt, void* ctx) {
        auto* r = static_cast<Resolving*>(ctx);
        BrowserImpl* self = r->self;
        if (r->finished) {
            return;
        }
        // Deallocated after the callback returns, never inside it.
        r->finished = true;
        if (err != kDNSServiceErr_NoError || self->_interfaces.count(r->name) == 0) {
            return;
        }
        SessionAdvert a;
        a.id = r->name;
        auto get = [&](const char* key) {
            uint8_t len = 0;
            const void* v = TXTRecordGetValuePtr(txtLen, txt, key, &len);
            return v != nullptr ? std::string((const char*)v, len) : std::string();
        };
        a.sequenceName = get("seq");
        a.hostName = get("host");
        a.deviceKind = get("kind");
        std::string proto = get("proto");
        a.protocol = proto.empty() ? 0 : (int)std::strtol(proto.c_str(), nullptr, 10);
        self->_found[r->name] = a;
        self->Changed();
    }

    void FinishResolves() {
        _retired.clear();
        for (auto it = _resolves.begin(); it != _resolves.end();) {
            if (it->second->finished) {
                it = _resolves.erase(it);
            } else {
                ++it;
            }
        }
    }
#endif

#ifdef _WIN32
    struct WinCtx {
        std::weak_ptr<BrowserImpl> self;
        uint64_t key = 0; // in WinContexts()
        std::atomic<uint64_t> generation{ 0 };
        std::wstring query;
        DNS_SERVICE_BROWSE_REQUEST req{};
        DNS_SERVICE_CANCEL cancel{};
    };

    // Every browse's context, for the life of the process: a callback may
    // come after its browser is gone.
    static Wire::CallbackRegistry<WinCtx>& WinContexts() {
        static auto* contexts = new Wire::CallbackRegistry<WinCtx>();
        return *contexts;
    }

    static VOID WINAPI OnWinBrowse(DWORD status, PVOID context, PDNS_RECORD records) {
        std::shared_ptr<WinCtx> ctx = WinContexts().Find((uint64_t)(uintptr_t)context);
        if (!ctx) {
            if (records != nullptr) {
                Dns().recordListFree(records, DnsFreeRecordList);
            }
            return;
        }
        std::vector<std::pair<std::string, bool>> names; // name, still there
        if (status == ERROR_SUCCESS) {
            for (auto* rec = (DNS_RECORDW*)records; rec != nullptr; rec = rec->pNext) {
                if (rec->wType == DNS_TYPE_PTR && rec->Data.PTR.pNameHost != nullptr) {
                    names.emplace_back(InstanceOf(Narrow(rec->Data.PTR.pNameHost)), rec->dwTtl != 0);
                }
            }
        }
        if (records != nullptr) {
            Dns().recordListFree(records, DnsFreeRecordList);
        }
        uint64_t gen = ctx->generation.load();
        auto self = ctx->self.lock();
        if (!self || gen == 0 || names.empty()) {
            return;
        }
        std::weak_ptr<BrowserImpl> w = self;
        self->Post([w, ctx, gen, names] {
            auto s = w.lock();
            if (s && ctx->generation.load() == gen) {
                s->WinNames(ctx, gen, names);
            }
        });
    }

    void WinNames(const std::shared_ptr<WinCtx>& ctx, uint64_t gen, const std::vector<std::pair<std::string, bool>>& names) {
        bool changed = false;
        for (auto& [name, present] : names) {
            std::string salt;
            if (!Wire::SaltFromServiceName(name, salt)) {
                continue;
            }
            if (!present) {
                changed |= _found.erase(name) > 0;
                _winPending.erase(name);
                continue;
            }
            if (_found.count(name) != 0 || _winPending.count(name) != 0) {
                continue;
            }
            _winPending.insert(name);
            struct RCtx {
                std::shared_ptr<WinCtx> ctx;
                uint64_t gen;
                std::string name;
                std::wstring full;
                DNS_SERVICE_CANCEL cancel{};
                DNS_SERVICE_RESOLVE_REQUEST req{};
            };
            auto* rc = new RCtx{ ctx, gen, name, FullServiceName(name) };
            DNS_SERVICE_RESOLVE_REQUEST& req = rc->req;
            req.Version = DNS_QUERY_REQUEST_VERSION1;
            req.QueryName = const_cast<PWSTR>(rc->full.c_str());
            req.pQueryContext = rc;
            req.pResolveCompletionCallback = [](DWORD status, PVOID context, PDNS_SERVICE_INSTANCE inst) {
                std::unique_ptr<RCtx> r(static_cast<RCtx*>(context));
                SessionAdvert a;
                bool ok = false;
                if (status == ERROR_SUCCESS && inst != nullptr) {
                    ok = true;
                    a.id = r->name;
                    for (DWORD i = 0; i < inst->dwPropertyCount; ++i) {
                        std::string k = Narrow(inst->keys[i]);
                        std::string v = Narrow(inst->values[i]);
                        if (k == "seq") {
                            a.sequenceName = v;
                        } else if (k == "host") {
                            a.hostName = v;
                        } else if (k == "kind") {
                            a.deviceKind = v;
                        } else if (k == "proto") {
                            a.protocol = v.empty() ? 0 : (int)std::strtol(v.c_str(), nullptr, 10);
                        }
                    }
                }
                if (inst != nullptr) {
                    Dns().freeInstance(inst);
                }
                auto self = r->ctx->self.lock();
                if (!self || r->ctx->generation.load() != r->gen) {
                    return;
                }
                std::weak_ptr<BrowserImpl> w = self;
                auto c = r->ctx;
                uint64_t g = r->gen;
                std::string name = r->name;
                self->Post([w, c, g, name, a, ok] {
                    auto s = w.lock();
                    if (!s || c->generation.load() != g) {
                        return;
                    }
                    s->_winPending.erase(name);
                    if (ok) {
                        s->_found[name] = a;
                        s->Changed();
                    }
                });
            };
            if (Dns().resolve(&rc->req, &rc->cancel) != DNS_REQUEST_PENDING) {
                delete rc;
                _winPending.erase(name);
            }
        }
        if (changed) {
            Changed();
        }
    }
#endif

    Dispatcher _dispatch;
    ISessionBrowser* _owner;
    std::shared_ptr<std::atomic<bool>> _alive;
    std::shared_ptr<std::atomic<bool>> _cur;
    std::map<std::string, SessionAdvert> _found;
    std::map<std::string, std::set<uint32_t>> _interfaces;
    bool _done = false;
#if defined(XL_COLLAB_DNSSD)
    struct Resolving {
        DNSServiceRef ref = nullptr;
        BrowserImpl* self = nullptr;
        std::string name;
        uint64_t serial = 0;
        bool finished = false;
        Resolving() = default;
        Resolving(const Resolving&) = delete;
        Resolving& operator=(const Resolving&) = delete;
        ~Resolving() {
            if (ref != nullptr) {
                DNSServiceRefDeallocate(ref);
            }
        }
    };
    DNSServiceRef _browseRef = nullptr;
    std::map<std::string, std::unique_ptr<Resolving>> _resolves;
    std::vector<std::unique_ptr<Resolving>> _retired; // freed at the next FinishResolves
    uint64_t _resolveSerial = 0;
    bool _dirty = false;
#endif
#ifdef _WIN32
    std::shared_ptr<WinCtx> _winCtx;
    uint64_t _generation = 0;
    std::set<std::string> _winPending;
#endif
};

class LanBrowser : public ISessionBrowser {
public:
    explicit LanBrowser(Dispatcher d) : _impl(std::make_shared<BrowserImpl>(std::move(d), this)) { _impl->StartThread(); }
    ~LanBrowser() override { _impl->Shutdown(); }
    void Start() override { _impl->Start(); }
    void Stop() override { _impl->Stop(); }

private:
    std::shared_ptr<BrowserImpl> _impl;
};

TransportImpl* ImplOf(INetTransport& t) {
    auto* lt = dynamic_cast<LanTransport*>(&t);
    return lt != nullptr ? &lt->Impl() : nullptr;
}

} // namespace

namespace Portable {

std::unique_ptr<INetTransport> CreateTransport(Dispatcher dispatch) {
    if (!NetStartup()) {
        return nullptr;
    }
    return std::make_unique<LanTransport>(std::move(dispatch));
}

std::unique_ptr<ISessionBrowser> CreateBrowser(Dispatcher dispatch) {
    if (!NetStartup()) {
        return nullptr;
    }
    return std::make_unique<LanBrowser>(std::move(dispatch));
}

bool DiscoveryAvailable() {
    return DiscoveryCompiled();
}

std::string DeriveKey(const std::string& code, const std::string& salt) {
    std::string s = std::string("xlights-collab-v1:") + salt;
    unsigned char out[32];
#if MBEDTLS_VERSION_NUMBER >= 0x03030000
    if (mbedtls_pkcs5_pbkdf2_hmac_ext(MBEDTLS_MD_SHA256, (const unsigned char*)code.data(), code.size(), (const unsigned char*)s.data(), s.size(),
                                      kPbkdfRounds, sizeof(out), out) != 0) {
        return {};
    }
#else
    // Before 3.3 PBKDF2 takes an HMAC context.
    const mbedtls_md_info_t* info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    mbedtls_md_context_t md;
    mbedtls_md_init(&md);
    int ret = info == nullptr ? -1 : mbedtls_md_setup(&md, info, 1);
    if (ret == 0) {
        ret = mbedtls_pkcs5_pbkdf2_hmac(&md, (const unsigned char*)code.data(), code.size(), (const unsigned char*)s.data(), s.size(), kPbkdfRounds,
                                        sizeof(out), out);
    }
    mbedtls_md_free(&md);
    if (ret != 0) {
        return {};
    }
#endif
    return std::string((const char*)out, sizeof(out));
}

std::string DirectSessionId(INetTransport& host, const std::string& address) {
    auto* t = ImplOf(host);
    return t != nullptr ? t->DirectSessionId(address) : "";
}

void SetPairingBackoff(INetTransport& host, const LanTest::PairingBackoff& b) {
    if (auto* t = ImplOf(host)) {
        t->SetBackoff(b);
    }
}

LanTest::PairingStatus Pairing(INetTransport& host) {
    auto* t = ImplOf(host);
    return t != nullptr ? t->Pairing() : LanTest::PairingStatus{};
}

void SendRaw(INetTransport& t, PeerId peer, const std::string& bytes) {
    if (auto* i = ImplOf(t)) {
        i->SendRaw(peer, bytes);
    }
}

void SetBeforeLookup(std::function<void(const std::string& host)> hook) {
    std::lock_guard<std::mutex> l(LookupHookLock());
    LookupHookSlot() = std::move(hook);
}

bool TlsSetupFailureCleansUp() {
    // Garbage where the contexts go, as a heap block may hold: freeing them
    // uninitialised would follow these bytes as pointers.
    alignas(TlsConfig) unsigned char raw[sizeof(TlsConfig)];
    memset(raw, 0xA5, sizeof(raw));
    TlsConfig* c = new (raw) TlsConfig(false, std::string(32, 'k'), true);
    bool failed = !c->ok;
    c->~TlsConfig();
    return failed;
}

std::string BlobFolder(INetTransport& t) {
    auto* i = ImplOf(t);
    return i != nullptr ? i->Folder() : "";
}

bool PeerSocketOptions(INetTransport& t, PeerId peer, SocketOptions& out) {
    auto* i = ImplOf(t);
    return i != nullptr && i->SocketOptions(peer, out);
}

bool Advertising(INetTransport& host) {
    auto* i = ImplOf(host);
    return i != nullptr && i->Advertising();
}

} // namespace Portable

#if !defined(__APPLE__)

std::unique_ptr<INetTransport> CreateLanTransport(Dispatcher dispatch) {
    return Portable::CreateTransport(std::move(dispatch));
}

std::unique_ptr<ISessionBrowser> CreateLanBrowser(Dispatcher dispatch) {
    return Portable::CreateBrowser(std::move(dispatch));
}

bool LanTransportAvailable() {
    return NetStartup();
}

bool LanJoinByAddressAvailable() {
    return true;
}

std::string SecretDigest(const std::string& secret) {
    unsigned char out[32];
#if MBEDTLS_VERSION_NUMBER >= 0x03000000
    int ret = mbedtls_sha256((const unsigned char*)secret.data(), secret.size(), out, 0);
#else
    int ret = mbedtls_sha256_ret((const unsigned char*)secret.data(), secret.size(), out, 0);
#endif
    if (ret != 0) {
        return {};
    }
    return Wire::HexOf(std::string_view((const char*)out, sizeof(out)));
}

namespace LanTest {

std::string DeriveKey(const std::string& code, const std::string& salt) {
    return Portable::DeriveKey(code, salt);
}
std::string DirectSessionId(INetTransport& host, const std::string& address) {
    return Portable::DirectSessionId(host, address);
}
void SetPairingBackoff(INetTransport& host, const PairingBackoff& b) {
    Portable::SetPairingBackoff(host, b);
}
PairingStatus Pairing(INetTransport& host) {
    return Portable::Pairing(host);
}
void SendRaw(INetTransport& t, PeerId peer, const std::string& bytes) {
    Portable::SendRaw(t, peer, bytes);
}
std::string BlobFolder(INetTransport& t) {
    return Portable::BlobFolder(t);
}

} // namespace LanTest

#endif // !__APPLE__

} // namespace Collab

#endif
