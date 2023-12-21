/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "ip_utils.h"
#include "string_utils.h"

#include <wx/wx.h>
#include <wx/string.h>
#include <wx/regex.h>
#include <wx/sckaddr.h>

#include <map>
#include <mutex>

namespace ip_utils
{
    static std::map<std::string, std::string> __resolvedIPMap;
    static std::mutex __resolvedIPMapLock;

	bool IsIPValid(const std::string& ip)
    {
        wxString ips = wxString(ip).Trim(false).Trim(true);
        if (ips == "") {
            return false;
        }
        else {
            static wxRegEx regxIPAddr("^(([0-9]{1}|[0-9]{2}|[0-1][0-9]{2}|2[0-4][0-9]|25[0-5])\\.){3}([0-9]{1}|[0-9]{2}|[0-1][0-9]{2}|2[0-4][0-9]|25[0-5])$");

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

        //hosts only, IP address should have already passed above
        static wxRegEx hostAddr(R"(^([a-zA-Z0-9\-]+)(\.?)([a-zA-Z0-9\-]{2,})$)");

        wxString ips = wxString(ip).Trim(false).Trim(true);
        if (hostAddr.Matches(ips)) {
            return true;
        }

        //IP address should fall through to this false if not valid host too
        return false;
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
            wxIPV4address add;
            add.Hostname(ip);
            std::string r = add.IPAddress();
            if (r == "0.0.0.0") {
                r = ip;
            }
            if (r == "255.255.255.255") {
                r = ip;
            }
            __resolvedIPMap[ip] = r;
            return __resolvedIPMap[ip];
        }
        return resolvedIp;
    }

};
