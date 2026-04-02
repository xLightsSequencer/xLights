#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <array>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <string>
#include <chrono>

#ifdef _WIN32
#include <winsock2.h>
#include <Ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace sockets {

#ifdef _WIN32
using SocketHandle = SOCKET;
constexpr SocketHandle INVALID_SOCKET_HANDLE = INVALID_SOCKET;
#else
using SocketHandle = int;
constexpr SocketHandle INVALID_SOCKET_HANDLE = -1;
#endif

inline void closeSocket(SocketHandle s)
{
#ifdef _WIN32
    closesocket(s);
#else
    close(s);
#endif
}

inline std::string getLastSocketErrorString()
{
#ifdef _WIN32
    return std::to_string(static_cast<int>(WSAGetLastError()));
#else
    return std::strerror(errno);
#endif
}

inline bool parseIPv4(const std::string& ip, in_addr& out)
{
#ifdef _WIN32
    return InetPtonA(AF_INET, ip.c_str(), &out) == 1;
#else
    return inet_pton(AF_INET, ip.c_str(), &out) == 1;
#endif
}

class TCPSocket {
public:
    TCPSocket() = default;
    ~TCPSocket() { Close(); }

    TCPSocket(const TCPSocket&) = delete;
    TCPSocket& operator=(const TCPSocket&) = delete;

    bool Connect(const std::string& remoteIp, uint16_t remotePort, const std::string& localIp, bool nonBlocking)
    {
        Close();

        SocketHandle s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (s == INVALID_SOCKET_HANDLE) {
            _lastError = getLastSocketErrorString();
            return false;
        }

        if (!localIp.empty()) {
            sockaddr_in localAddr{};
            localAddr.sin_family = AF_INET;
            localAddr.sin_port = htons(0);
            if (!parseIPv4(localIp, localAddr.sin_addr)) {
                closeSocket(s);
                _lastError = "Invalid local IPv4 address: " + localIp;
                return false;
            }

            if (bind(s, reinterpret_cast<const sockaddr*>(&localAddr), sizeof(localAddr)) != 0) {
                _lastError = getLastSocketErrorString();
                closeSocket(s);
                return false;
            }
        }

        if (nonBlocking) {
#ifdef _WIN32
            u_long mode = 1;
            if (ioctlsocket(s, FIONBIO, &mode) != 0) {
                _lastError = getLastSocketErrorString();
                closeSocket(s);
                return false;
            }
#else
            int flags = fcntl(s, F_GETFL, 0);
            if (flags < 0 || fcntl(s, F_SETFL, flags | O_NONBLOCK) != 0) {
                _lastError = getLastSocketErrorString();
                closeSocket(s);
                return false;
            }
#endif
        }

        sockaddr_in remoteAddr{};
        remoteAddr.sin_family = AF_INET;
        remoteAddr.sin_port = htons(remotePort);
        if (!parseIPv4(remoteIp, remoteAddr.sin_addr)) {
            closeSocket(s);
            _lastError = "Invalid remote IPv4 address: " + remoteIp;
            return false;
        }

        const int connectResult = connect(s, reinterpret_cast<const sockaddr*>(&remoteAddr), sizeof(remoteAddr));
        if (connectResult != 0) {
#ifdef _WIN32
            const int connectErr = WSAGetLastError();
            if (!nonBlocking || (connectErr != WSAEWOULDBLOCK && connectErr != WSAEINPROGRESS && connectErr != WSAEALREADY)) {
                _lastError = std::to_string(connectErr);
                closeSocket(s);
                return false;
            }
#else
            if (!nonBlocking || (errno != EINPROGRESS && errno != EWOULDBLOCK)) {
                _lastError = getLastSocketErrorString();
                closeSocket(s);
                return false;
            }
#endif
        }

        _socket = s;
        _connected = true;
        _lastError.clear();
        return true;
    }

    bool SetNoDelay(bool noDelay)
    {
        if (_socket == INVALID_SOCKET_HANDLE) {
            return false;
        }
        const int option = noDelay ? 1 : 0;
        return setsockopt(_socket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&option), sizeof(option)) == 0;
    }

    bool Write(const uint8_t* data, size_t length)
    {
        if (_socket == INVALID_SOCKET_HANDLE || data == nullptr || length == 0) {
            return false;
        }

        size_t totalSent = 0;
        while (totalSent < length) {
#ifdef _WIN32
            const int sent = send(_socket, reinterpret_cast<const char*>(data + totalSent), static_cast<int>(length - totalSent), 0);
#else
            const ssize_t sent = send(_socket, data + totalSent, length - totalSent, 0);
#endif
            if (sent <= 0) {
#ifdef _WIN32
                const int err = WSAGetLastError();
                if (err == WSAEWOULDBLOCK) {
                    _lastError = "Write would block";
                    return false;
                }
                _lastError = std::to_string(err);
#else
                if (errno == EWOULDBLOCK || errno == EAGAIN) {
                    _lastError = "Write would block";
                    return false;
                }
                _lastError = getLastSocketErrorString();
#endif
                _connected = false;
                return false;
            }
            totalSent += static_cast<size_t>(sent);
        }

        return totalSent == length;
    }

    bool IsConnected() const { return _connected && _socket != INVALID_SOCKET_HANDLE; }

    bool SetReadTimeoutMs(int timeoutMs)
    {
        if (_socket == INVALID_SOCKET_HANDLE || timeoutMs < 0) {
            return false;
        }
#ifdef _WIN32
        const DWORD tv = static_cast<DWORD>(timeoutMs);
        if (setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&tv), sizeof(tv)) != 0) {
            _lastError = getLastSocketErrorString();
            return false;
        }
#else
        timeval tv{};
        tv.tv_sec = timeoutMs / 1000;
        tv.tv_usec = (timeoutMs % 1000) * 1000;
        if (setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) != 0) {
            _lastError = getLastSocketErrorString();
            return false;
        }
#endif
        _readTimeoutMs = timeoutMs;
        return true;
    }

    int GetReadTimeoutMs() const { return _readTimeoutMs; }

    bool WaitForData(int timeoutMs)
    {
        if (_socket == INVALID_SOCKET_HANDLE) {
            return false;
        }

        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(_socket, &readfds);

        timeval tv{};
        timeval* tvptr = nullptr;
        if (timeoutMs >= 0) {
            tv.tv_sec = timeoutMs / 1000;
            tv.tv_usec = (timeoutMs % 1000) * 1000;
            tvptr = &tv;
        }

#ifdef _WIN32
        const int ret = select(0, &readfds, nullptr, nullptr, tvptr);
#else
        const int ret = select(_socket + 1, &readfds, nullptr, nullptr, tvptr);
#endif
        if (ret <= 0) {
            return false;
        }
        return FD_ISSET(_socket, &readfds) != 0;
    }

    int Read(uint8_t* data, size_t length)
    {
        if (_socket == INVALID_SOCKET_HANDLE || data == nullptr || length == 0) {
            _lastReadCount = 0;
            return 0;
        }
#ifdef _WIN32
        const int r = recv(_socket, reinterpret_cast<char*>(data), static_cast<int>(length), 0);
#else
        const int r = static_cast<int>(recv(_socket, data, length, 0));
#endif
        if (r <= 0) {
            _lastReadCount = 0;
            if (r < 0) {
                _lastError = getLastSocketErrorString();
            }
            return 0;
        }
        _lastReadCount = r;
        return r;
    }

    int Peek(uint8_t* data, size_t length)
    {
        if (_socket == INVALID_SOCKET_HANDLE || data == nullptr || length == 0) {
            _lastReadCount = 0;
            return 0;
        }
#ifdef _WIN32
        const int r = recv(_socket, reinterpret_cast<char*>(data), static_cast<int>(length), MSG_PEEK);
#else
        const int r = static_cast<int>(recv(_socket, data, length, MSG_PEEK));
#endif
        if (r <= 0) {
            _lastReadCount = 0;
            if (r < 0) {
                _lastError = getLastSocketErrorString();
            }
            return 0;
        }
        _lastReadCount = r;
        return r;
    }

    int LastReadCount() const { return _lastReadCount; }

    void Close()
    {
        if (_socket != INVALID_SOCKET_HANDLE) {
            closeSocket(_socket);
            _socket = INVALID_SOCKET_HANDLE;
        }
        _connected = false;
    }

    const std::string& LastError() const { return _lastError; }

private:
    SocketHandle _socket = INVALID_SOCKET_HANDLE;
    bool _connected = false;
    std::string _lastError;
    int _lastReadCount = 0;
    int _readTimeoutMs = 0;
};

class UDPSocket {
public:
    UDPSocket() = default;
    ~UDPSocket() { Close(); }

    UDPSocket(const UDPSocket&) = delete;
    UDPSocket& operator=(const UDPSocket&) = delete;

    bool Open(bool broadcast = false)
    {
        Close();
        _socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (_socket == INVALID_SOCKET_HANDLE) {
            _lastError = getLastSocketErrorString();
            return false;
        }

        if (broadcast) {
            const int on = 1;
            if (setsockopt(_socket, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<const char*>(&on), sizeof(on)) != 0) {
                _lastError = getLastSocketErrorString();
                Close();
                return false;
            }
        }

        _lastError.clear();
        return true;
    }

    bool SetBroadcast(bool enabled)
    {
        if (_socket == INVALID_SOCKET_HANDLE) {
            if (!Open()) {
                return false;
            }
        }
        const int on = enabled ? 1 : 0;
        if (setsockopt(_socket, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<const char*>(&on), sizeof(on)) != 0) {
            _lastError = getLastSocketErrorString();
            return false;
        }
        _lastError.clear();
        return true;
    }

    bool Bind(const std::string& localIp, uint16_t localPort, bool reuseAddr = false)
    {
        if (_socket == INVALID_SOCKET_HANDLE) {
            if (!Open()) {
                return false;
            }
        }

        if (reuseAddr) {
            const int on = 1;
            setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&on), sizeof(on));
        }

        sockaddr_in localAddr{};
        localAddr.sin_family = AF_INET;
        localAddr.sin_port = htons(localPort);

        if (localIp.empty()) {
            localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        } else if (!parseIPv4(localIp, localAddr.sin_addr)) {
            _lastError = "Invalid local IPv4 address: " + localIp;
            return false;
        }

        if (bind(_socket, reinterpret_cast<const sockaddr*>(&localAddr), sizeof(localAddr)) != 0) {
            _lastError = getLastSocketErrorString();
            return false;
        }

        _lastError.clear();
        return true;
    }

    bool SendTo(const std::string& remoteIp, uint16_t remotePort, const uint8_t* data, size_t length)
    {
        if (_socket == INVALID_SOCKET_HANDLE || data == nullptr || length == 0) {
            return false;
        }

        sockaddr_in remoteAddr{};
        remoteAddr.sin_family = AF_INET;
        remoteAddr.sin_port = htons(remotePort);
        if (!parseIPv4(remoteIp, remoteAddr.sin_addr)) {
            _lastError = "Invalid remote IPv4 address: " + remoteIp;
            return false;
        }

#ifdef _WIN32
        const int sent = sendto(_socket, reinterpret_cast<const char*>(data), static_cast<int>(length), 0, reinterpret_cast<const sockaddr*>(&remoteAddr), sizeof(remoteAddr));
        if (sent < 0) {
#else
        const ssize_t sent = sendto(_socket, data, length, 0, reinterpret_cast<const sockaddr*>(&remoteAddr), sizeof(remoteAddr));
        if (sent < 0) {
#endif
            _lastError = getLastSocketErrorString();
            return false;
        }

        _lastError.clear();
        return true;
    }

    bool WaitForData(int timeoutMs)
    {
        if (_socket == INVALID_SOCKET_HANDLE) {
            return false;
        }

        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(_socket, &readfds);

        timeval tv{};
        timeval* tvptr = nullptr;
        if (timeoutMs >= 0) {
            tv.tv_sec = timeoutMs / 1000;
            tv.tv_usec = (timeoutMs % 1000) * 1000;
            tvptr = &tv;
        }

#ifdef _WIN32
        const int ret = select(0, &readfds, nullptr, nullptr, tvptr);
#else
        const int ret = select(_socket + 1, &readfds, nullptr, nullptr, tvptr);
#endif
        if (ret <= 0) {
            return false;
        }
        return FD_ISSET(_socket, &readfds) != 0;
    }

    int ReceiveFrom(uint8_t* buffer, size_t length, std::string* fromIp = nullptr, uint16_t* fromPort = nullptr)
    {
        if (_socket == INVALID_SOCKET_HANDLE || buffer == nullptr || length == 0) {
            return -1;
        }

        sockaddr_in remoteAddr{};
#ifdef _WIN32
        int addrLen = sizeof(remoteAddr);
        const int r = recvfrom(_socket,
            reinterpret_cast<char*>(buffer),
            static_cast<int>(length),
            0,
            reinterpret_cast<sockaddr*>(&remoteAddr),
            &addrLen);
#else
        socklen_t addrLen = sizeof(remoteAddr);
        const int r = static_cast<int>(recvfrom(_socket,
            buffer,
            length,
            0,
            reinterpret_cast<sockaddr*>(&remoteAddr),
            &addrLen));
#endif

        if (r < 0) {
            _lastError = getLastSocketErrorString();
            return -1;
        }

        if (fromPort != nullptr) {
            *fromPort = ntohs(remoteAddr.sin_port);
        }

        if (fromIp != nullptr) {
            char ipBuf[INET_ADDRSTRLEN] = { 0 };
#ifdef _WIN32
            if (InetNtopA(AF_INET, &remoteAddr.sin_addr, ipBuf, INET_ADDRSTRLEN) != nullptr) {
                *fromIp = ipBuf;
            } else {
                fromIp->clear();
            }
#else
            if (inet_ntop(AF_INET, &remoteAddr.sin_addr, ipBuf, INET_ADDRSTRLEN) != nullptr) {
                *fromIp = ipBuf;
            } else {
                fromIp->clear();
            }
#endif
        }

        _lastError.clear();
        return r;
    }

    void Close()
    {
        if (_socket != INVALID_SOCKET_HANDLE) {
            closeSocket(_socket);
            _socket = INVALID_SOCKET_HANDLE;
        }
    }

    const std::string& LastError() const { return _lastError; }

private:
    SocketHandle _socket = INVALID_SOCKET_HANDLE;
    std::string _lastError;
};

} // namespace sockets
