#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <string>

namespace ip_utils
{
	bool IsIPValid(const std::string& ip);

	bool IsIPValidOrHostname(const std::string &ip, bool iponly = false);
	std::string CleanupIP(const std::string& ip);
	std::string ResolveIP(const std::string& ip);

};
