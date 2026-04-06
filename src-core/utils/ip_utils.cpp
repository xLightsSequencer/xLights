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

#include "JobPool.h"

#include <log.h>

#include <chrono>
#include <cstring>
#include <regex>
#include <thread>

#ifdef _WIN32
#include <iphlpapi.h>
#include <psapi.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#endif

#include <map>
#include <mutex>

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

    std::list<std::string> GetLocalIPs() {
        std::list<std::string> res;

#ifdef _WIN32

        ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
        PIP_ADAPTER_INFO pAdapterInfo = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
        if (pAdapterInfo == nullptr) {
            spdlog::error("Error getting adapter info.");
            return res;
        }

        if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
            free(pAdapterInfo);
            pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulOutBufLen);
            if (pAdapterInfo == nullptr) {
                spdlog::error("Error getting adapter info.");
                return res;
            }
        }

        PIP_ADAPTER_INFO pAdapter = nullptr;
        DWORD dwRetVal = 0;

        if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
            pAdapter = pAdapterInfo;
            while (pAdapter) {
                auto ip = &pAdapter->IpAddressList;
                while (ip != nullptr) {
                    if (strcmp(ip->IpAddress.String, "0.0.0.0") != 0) {
                        res.push_back(std::string(ip->IpAddress.String));
                    }
                    ip = ip->Next;
                }

                pAdapter = pAdapter->Next;
            }
        }
        free(pAdapterInfo);
#else
        struct ifaddrs *interfaces, *tmp;
        getifaddrs(&interfaces);
        tmp = interfaces;
        // loop through all the interfaces
        while (tmp) {
            if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET) {
                struct sockaddr_in* address = (struct sockaddr_in*)tmp->ifa_addr;
                std::string ip = inet_ntoa(address->sin_addr);
                if (ip != "0.0.0.0") {
                    res.push_back(ip);
                }
            } else if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET6) {
                // LogDebug(VB_SYNC, "   Inet6 interface %s\n", tmp->ifa_name);
            }
            tmp = tmp->ifa_next;
        }
        freeifaddrs(interfaces);
#endif

        return res;
    }

    bool IsValidLocalIP(const std::string& ip) {
        for (const auto& it : GetLocalIPs()) {
            if (it == ip)
                return true;
        }

        return false;
    }

    bool IsInSameSubnet(const std::string& ip1, const std::string& ip2, const std::string& mask) {
        if (ip1 == "" || ip2 == "" || mask == "")
            return false;

        struct in_addr a1, a2, m;
        if (inet_pton(AF_INET, ip1.c_str(), &a1) != 1)
            return false;
        if (inet_pton(AF_INET, ip2.c_str(), &a2) != 1)
            return false;
        if (inet_pton(AF_INET, mask.c_str(), &m) != 1)
            return false;

        return (a1.s_addr & m.s_addr) == (a2.s_addr & m.s_addr);
    }
};
