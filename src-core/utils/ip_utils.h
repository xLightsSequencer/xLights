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

#include <list>
#include <string>
#include <functional>

namespace ip_utils
{
	bool IsIPValid(const std::string& ip);

	bool IsIPValidOrHostname(const std::string &ip, bool iponly = false);
    bool IsValidHostname(const std::string& ip);
	bool IsIPv6(const std::string& ip);
	std::string CleanupIP(const std::string& ip);
	std::string ResolveIP(const std::string& ip);

    void ResolveIP(const std::string& ip, std::function<void(const std::string &)> &&func);
    void waitForAllToResolve();
    void shutdownResolvePool();

	std::list<std::string> GetLocalIPs();
    bool IsValidLocalIP(const std::string& ip);
    bool IsInSameSubnet(const std::string& ip1, const std::string& ip2, const std::string& mask = "255.255.255.0");

};
