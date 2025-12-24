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

#include <wx/wx.h>
#include <wx/string.h>
#include <wx/regex.h>
#include <wx/sckaddr.h>

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

	bool IsIPValid(const std::string& ip)
    {
        wxString ips = wxString(ip).Trim(false).Trim(true);
        if (ips == "") {
            return false;
        } else if (wxIsMainThread()) {
            static wxRegEx regxIPAddr("^(([0-9]{1}|[0-9]{2}|[0-1][0-9]{2}|2[0-4][0-9]|25[0-5])\\.){3}([0-9]{1}|[0-9]{2}|[0-1][0-9]{2}|2[0-4][0-9]|25[0-5])$");
            if (regxIPAddr.Matches(ips)) {
                return true;
            }
        } else {
            wxRegEx regxIPAddr("^(([0-9]{1}|[0-9]{2}|[0-1][0-9]{2}|2[0-4][0-9]|25[0-5])\\.){3}([0-9]{1}|[0-9]{2}|[0-1][0-9]{2}|2[0-4][0-9]|25[0-5])$");
            if (regxIPAddr.Matches(ips)) {
                return true;
            }
        }

        return false;
    }

    bool IsIPValidOrHostname(const std::string& ip, bool iponly)
    {
        //check if "ip" is a valid IP address
        if (IsIPValid(ip)) {
            return true;
        }

        if (iponly) {
            return false;
        }

        wxString ips = wxString(ip).Trim(false).Trim(true);
        if (wxIsMainThread()) {
            //hosts only, IP address should have already passed above
            static wxRegEx hostAddr(R"(^([a-zA-Z0-9\-]+)(\.?)([a-zA-Z0-9\-]{2,})$)");
            if (hostAddr.Matches(ips)) {
                return true;
            }
        } else {
            wxRegEx hostAddr(R"(^([a-zA-Z0-9\-]+)(\.?)([a-zA-Z0-9\-]{2,})$)");
            if (hostAddr.Matches(ips)) {
                return true;
            }
        }
        //IP address should fall through to this false if not valid host too
        return false;
    }

    bool IsValidHostname(const std::string& ip) {
        wxString ips = wxString(ip).Trim(false).Trim(true);
        if (wxIsMainThread()) {
            static wxRegEx hostAddr(R"(^([a-zA-Z0-9\-]+)(\.?)([a-zA-Z0-9\-]{2,})$)");
            return hostAddr.Matches(ips);
        }
        wxRegEx hostAddr(R"(^([a-zA-Z0-9\-]+)(\.?)([a-zA-Z0-9\-]{2,})$)");
        return hostAddr.Matches(ips);
    }

    bool IsIPv6(const std::string& ip) {
        wxIPV6address ipv6Addr;
        return ipv6Addr.Hostname(ip)  && ip.find(':') != std::string::npos;
    }

    std::string CleanupIP(const std::string& ip)
    {
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
        wxString IpAddr(ip.c_str());
        if (wxIsMainThread()) {
            static wxRegEx leadingzero1("(^0+)(?:[1-9]|0\\.)", wxRE_ADVANCED);
            if (leadingzero1.Matches(IpAddr)) {
                wxString s0 = leadingzero1.GetMatch(IpAddr, 0);
                wxString s1 = leadingzero1.GetMatch(IpAddr, 1);
                leadingzero1.ReplaceFirst(&IpAddr, "" + s0.Right(s0.size() - s1.size()));
            }
            static wxRegEx leadingzero2("(\\.0+)(?:[1-9]|0\\.|0$)", wxRE_ADVANCED);
            while (leadingzero2.Matches(IpAddr)) { // need to do it several times because the results overlap
                wxString s0 = leadingzero2.GetMatch(IpAddr, 0);
                wxString s1 = leadingzero2.GetMatch(IpAddr, 1);
                leadingzero2.ReplaceFirst(&IpAddr, "." + s0.Right(s0.size() - s1.size()));
            }
        } else {
            wxRegEx leadingzero1("(^0+)(?:[1-9]|0\\.)", wxRE_ADVANCED);
            if (leadingzero1.Matches(IpAddr)) {
                wxString s0 = leadingzero1.GetMatch(IpAddr, 0);
                wxString s1 = leadingzero1.GetMatch(IpAddr, 1);
                leadingzero1.ReplaceFirst(&IpAddr, "" + s0.Right(s0.size() - s1.size()));
            }
            wxRegEx leadingzero2("(\\.0+)(?:[1-9]|0\\.|0$)", wxRE_ADVANCED);
            while (leadingzero2.Matches(IpAddr)) { // need to do it several times because the results overlap
                wxString s0 = leadingzero2.GetMatch(IpAddr, 0);
                wxString s1 = leadingzero2.GetMatch(IpAddr, 1);
                leadingzero2.ReplaceFirst(&IpAddr, "." + s0.Right(s0.size() - s1.size()));
            }
        }
        return IpAddr.ToStdString();
    }

    std::string ResolveIP(const std::string& ip)
    {
        // Dont resolve partially entered ip addresses as these resolve into unexpected addresses
        if (IsIPValid(ip) || (ip == "MULTICAST") || ip == "" || StartsWith(ip, ".") || (ip[0] >= '0' && ip[0] <= '9')) {
            return ip;
        }
        std::unique_lock<std::mutex> lock(__resolvedIPMapLock);
        const std::string& resolvedIp = __resolvedIPMap[ip];
        if (resolvedIp == "") {
            lock.unlock();
            wxIPV4address add;
            add.Hostname(ip);
            std::string r = add.IPAddress();
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
            wxMilliSleep(2);
            //wxYieldIfNeeded();
        }
    }
};
