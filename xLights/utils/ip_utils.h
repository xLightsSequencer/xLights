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

#include <string>
#include <functional>

namespace ip_utils
{
	bool IsIPValid(const std::string& ip);

	bool IsIPValidOrHostname(const std::string &ip, bool iponly = false);
    bool IsValidHostname(const std::string& ip);
	std::string CleanupIP(const std::string& ip);
	std::string ResolveIP(const std::string& ip);

    void ResolveIP(const std::string& ip, std::function<void(const std::string &)> &&func);
    void waitForAllToResolve();

};
