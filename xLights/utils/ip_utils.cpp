/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ip_utils.h"
#include "string_utils.h"

#include <chrono>
#include <cstring>
#include <regex>
#include <thread>

#ifndef __WXMSW__
#include <netdb.h>
#include <arpa/inet.h>
#else
#include <ws2tcpip.h>
#endif

#include <map>
#include <mutex>

#include "../JobPool.h"

namespace ip_utils
{
    static std::map<std::string, std::string> __resolvedIPMap;
    static std::mutex __resolvedIPMapLock;

    namespace {
        bool ResolveHostnameToIP(const std::string& host, std::string& outIp) {
            struct addrinfo hints;
            struct addrinfo* result = nullptr;
            std::memset(&hints, 0x0, sizeof(hints));
            hints.ai_family = PF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_flags |= AI_CANONNAME;

            if (getaddrinfo(host.c_str(), nullptr, &hints, &result) != 0 || result == nullptr) {
                return false;
            }

            std::string ip4Addr;
            std::string ip6Addr;
            for (auto* res = result; res != nullptr; res = res->ai_next) {
                char addrstr[100] = { 0 };
                void* ptr = nullptr;
                if (res->ai_family == AF_INET) {
                    ptr = &((struct sockaddr_in*)res->ai_addr)->sin_addr;
                    inet_ntop(res->ai_family, ptr, addrstr, sizeof(addrstr));
                    ip4Addr = addrstr;
                } else if (res->ai_family == AF_INET6) {
                    ptr = &((struct sockaddr_in6*)res->ai_addr)->sin6_addr;
                    inet_ntop(res->ai_family, ptr, addrstr, sizeof(addrstr));
                    ip6Addr = addrstr;
                }
            }
            freeaddrinfo(result);

            if (!ip4Addr.empty()) {
                outIp = ip4Addr;
                return true;
            }
            if (!ip6Addr.empty()) {
                outIp = ip6Addr;
                return true;
            }
            return false;
        }
    }

	bool IsIPValid(const std::string& ip) {
        const std::string ips = Trim(ip);
        if (ips.empty()) {
            return false;
        }
        struct in_addr addr;
        return inet_pton(AF_INET, ips.c_str(), &addr) == 1;
    }

    bool IsIPValidOrHostname(const std::string& ip, bool iponly) {
        //check if "ip" is a valid IP address
        if (IsIPValid(ip)) {
            return true;
        }

        if (iponly) {
            return false;
        }

        const std::string ips = Trim(ip);
        static const std::regex hostAddr(R"(^([a-zA-Z0-9\-]+)(\.?)([a-zA-Z0-9\-]{2,})$)");
        if (std::regex_match(ips, hostAddr)) {
            return true;
        }
        //IP address should fall through to this false if not valid host too
        return false;
    }

    bool IsValidHostname(const std::string& ip) {
        const std::string ips = Trim(ip);
        static const std::regex hostAddr(R"(^([a-zA-Z0-9\-]+)(\.?)([a-zA-Z0-9\-]{2,})$)");
        return std::regex_match(ips, hostAddr);
    }

    bool IsIPv6(const std::string& ip) {
        if (ip.find(':') == std::string::npos) {
            return false;
        }
        struct in6_addr addr6;
        return inet_pton(AF_INET6, ip.c_str(), &addr6) == 1;
    }

    std::string CleanupIP(const std::string& ip) {
        bool hasChar = false;
        bool hasDot = false;
        //hostnames need at least one char in it if fully qualified
        //if not fully qualified (no .), then the hostname only COULD be just numeric
        for (size_t y = 0; y < ip.length(); y++) {
            char x = ip[y];
            if ((x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z') || x == '-') {
                hasChar = true;
            }
            if (x == '.') {
                hasDot = true;
            }
        }
        if (hasChar || !hasDot) {
            //hostname, not ip, don't mangle it
            return ip;
        }
        std::vector<std::string> parts;
        Split(ip, '.', parts, false);
        if (parts.size() != 4) {
            return ip;
        }
        std::string out;
        for (size_t i = 0; i < parts.size(); ++i) {
            const long part = std::strtol(parts[i].c_str(), nullptr, 10);
            if (part < 0 || part > 255) {
                return ip;
            }
            if (!out.empty()) {
                out += ".";
            }
            out += std::to_string(part);
        }
        return out;
    }

    std::string ResolveIP(const std::string& ip) {
        // Dont resolve partially entered ip addresses as these resolve into unexpected addresses
        if (IsIPValid(ip) || (ip == "MULTICAST") || ip == "" || StartsWith(ip, ".") || (ip[0] >= '0' && ip[0] <= '9')) {
            return ip;
        }
        std::unique_lock<std::mutex> lock(__resolvedIPMapLock);
        const std::string& resolvedIp = __resolvedIPMap[ip];
        if (resolvedIp == "") {
            lock.unlock();
            std::string r;
            if (!ResolveHostnameToIP(ip, r)) {
                r = ip;
            }
            if (r == "0.0.0.0") {
                r = ip;
            }
            if (r == "255.255.255.255") {
                r = ip;
            }
            lock.lock();
            __resolvedIPMap[ip] = r;
            return __resolvedIPMap[ip];
        }
        return resolvedIp;
    }

    class ResolveJob : public Job {
    public:
        ResolveJob(const std::string &i,  std::function<void(const std::string &)> &f) : Job(), ip(i), func(f) {}
        virtual bool DeleteWhenComplete() { return true; }
        virtual const std::string GetName() const {
            return "RESOLVE_POOL - " + ip;
        }
        virtual void Process() {
            struct addrinfo hints, *res, *result = nullptr;
            int errcode;
            void *ptr;

            memset (&hints, 0, sizeof (hints));
            hints.ai_family = PF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_flags |= AI_CANONNAME;

            std::string ip4Addr;
            std::string ip6Addr;
            errcode = getaddrinfo(ip.c_str(), NULL, &hints, &result);
            if (errcode != 0) {
                ip4Addr = ip;
            } else {
                res = result;
                while (res)  {
                    char addrstr[100] = {0};
                    switch (res->ai_family) {
                        case AF_INET:
                            ptr = &((struct sockaddr_in *) res->ai_addr)->sin_addr;
                            inet_ntop(res->ai_family, ptr, addrstr, 100);
                            ip4Addr = addrstr;
                            break;
                        case AF_INET6:
                            ptr = &((struct sockaddr_in6 *) res->ai_addr)->sin6_addr;
                            inet_ntop(res->ai_family, ptr, addrstr, 100);
                            ip6Addr = addrstr;
                            break;
                    }
                    res = res->ai_next;
                }
            }
            freeaddrinfo(result);
            if (ip4Addr.empty()) {
                ip4Addr = ip6Addr;
            }
            if (ip4Addr.empty()) {
                ip4Addr = ip;
            }
            func(ip4Addr);
            std::unique_lock<std::mutex> lock(__resolvedIPMapLock);
            __resolvedIPMap[ip] = ip4Addr;
            lock.unlock();
        }
        std::string ip;
        std::function<void(const std::string &)> func;
    };

    static JobPool RESOLVE_POOL("RESOLVE_POOL", 0, 128);
    void ResolveIP(const std::string& ip, std::function<void(const std::string &)> &&func) {
        // Dont resolve partially entered ip addresses as these resolve into unexpected addresses
        if (IsIPValid(ip) || (ip == "MULTICAST") || ip == "" || StartsWith(ip, ".") || (ip[0] >= '0' && ip[0] <= '9')) {
            func(ip);
            return;
        }
        std::unique_lock<std::mutex> lock(__resolvedIPMapLock);
        std::string resolvedIp = __resolvedIPMap[ip];
        if (resolvedIp.empty() || hasAlpha(ip)) {
            if (hasAlpha(ip))
                __resolvedIPMap.erase(ip);

            lock.unlock();
            ResolveJob *job = new ResolveJob(ip, func);
            RESOLVE_POOL.PushJob(job);
        } else {
            lock.unlock();
            func(resolvedIp);
        }
    }

    void waitForAllToResolve() {
        int count = 0;
        while (!RESOLVE_POOL.isEmpty() && count < 10000) {
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
            //wxYieldIfNeeded();
        }
    }

    void shutdownResolvePool() {
        RESOLVE_POOL.Stop();
    }
};
