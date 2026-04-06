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
#include <map>

namespace NodeUtils
{
	std::string ExpandNodes(const std::string& nodes);
	std::string CompressNodes(const std::string& nodes);

	//shift nodes  numbering 1->21, 50->70
	void ShiftNodes(std::map<std::string, std::string> & nodes, int shift, int min, int max);
	//reverse nodes, numbering 1->100, 100->1
	void ReverseNodes(std::map<std::string, std::string> & nodes, int max);
};