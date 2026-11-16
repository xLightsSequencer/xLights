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

	enum class OverlayAnchor { Start, Centre, End };
	// Offset of an overlay of size 'source' inside an area of size 'target', measured
	// from the low-index edge (left for X, bottom for buffer Y). Never negative.
	int OverlayOffset(OverlayAnchor anchor, int target, int source);
};