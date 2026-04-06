/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "NodeUtils.h"

#include "string_utils.h"

#include <vector>
#include <string>

namespace NodeUtils
{
    std::string CompressNodes(const std::string& nodes) {
        std::string res;
        if (nodes.empty()) {
            return res;
		}
		// make sure it is fully expanded first
		auto s = ExpandNodes(nodes);
		int dir = 0;
		int start = -1;
		int last = -1;
		std::vector<std::string> as;
		Split(s, ',', as);

		// There is no difference between empty row and row with one blank pixel (shrug)
		// We will take removal of the last comma approach

		for (const auto& i : as) {
			if (i.empty() || i == "0") {
				// Flush out start/last if any
				if (start != -1) {
					if (last != start) {
						res += std::to_string(start) + "-" + std::to_string(last) + ",";
					} else {
						res += std::to_string(start) + ",";
					}
				}
				// Add empty and separator
				res += i + ",";
				start = last = -1;
				dir = 0;
				continue;
			}
			if (start == -1) {
				start = (int)std::strtol(i.c_str(), nullptr, 10);
				last = start;
				dir = 0;
			} else {
				int j = (int)std::strtol(i.c_str(), nullptr, 10);
				if (dir == 0) {
					if (j == last + 1) {
						dir = 1;
					} else if (j == last - 1) {
						dir = -1;
					} else {
						res += std::to_string(start) + ",";
						start = j;
						dir = 0;
					}
					last = j;
				} else {
					if (j == last + dir) {
					} else {
						res += std::to_string(start) + "-" + std::to_string(last) + ",";
						start = j;
						dir = 0;
					}
					last = j;
				}
			}
		}

		if (start == -1) {
			// nothing to do
		} else if (start == last) {
			res += std::to_string(start) + ",";
		} else {
			res += std::to_string(start) + "-" + std::to_string(last) + ",";
		}

		if (!res.empty()) {
            res = res.substr(0, res.length() - 1); // Chop last comma
        }
		return res;
	}

	std::string ExpandNodes(const std::string& nodes) {
		std::string res;

		std::vector<std::string> as;
		Split(nodes, ',', as);

		bool first = true;
		for (const auto& i : as) {
			if (i.find('-') != std::string::npos) {
				std::vector<std::string> as2;
				Split(i, '-', as2);
				if (as2.size() == 2) {
					int start = (int)std::strtol(as2[0].c_str(), nullptr, 10);
					int end = (int)std::strtol(as2[1].c_str(), nullptr, 10);
					if (start < end) {
						for (int j = start; j <= end; j++) {
							if (!first || !res.empty()) {
                                res += ",";
                            }
							res += std::to_string(j);
						}
					} else if (start == end) {
						if (!first || !res.empty()) {
                            res += ",";
                        }
						res += std::to_string(start);
					} else {
						for (int j = start; j >= end; j--) {
							if (!first || !res.empty()) {
                                res += ",";
                            }
							res += std::to_string(j);
						}
					}
				}
			} else {
				if (!first || !res.empty()) {
                    res += ",";
                }
				res += i;
			}
			first = false;
		}
		return res;
	}

	void ShiftNodes(std::map<std::string, std::string>& nodes, int shift, int min, int max) {
		for (auto& line : nodes) {
			if (line.second.empty()) {
                continue;
            }
			if (Contains(line.first, "Color")) {
                continue;
            }
			if (Contains(line.first, "Name")) {
                continue;
            }
			if (Contains(line.first, "Type")) {
                continue;
            }
			auto const oldnodes = ExpandNodes(line.second);
			std::vector<std::string> oldNodeArray;
			Split(oldnodes, ',', oldNodeArray);
			std::string newNodes;
			for (auto const& node : oldNodeArray) {
				long const val = std::strtol(node.c_str(), nullptr, 10);
                if (val != 0 || node == "0") {
                    long newVal = val + shift;
                    if (newVal > max) {
                        newVal -= max;
                    } else if (newVal < min) {
                        newVal += max;
                    }
                    if (!newNodes.empty()) {
                        newNodes += ",";
					}
					newNodes += std::to_string(newVal);
				}
			}
			if (!newNodes.empty()) {
                line.second = CompressNodes(newNodes);
            }
		}
	}

	void ReverseNodes(std::map<std::string, std::string>& nodes, int max) {
		for (auto& line : nodes) {
			if (line.second.empty()) {
                continue;
            }
			if (Contains(line.first, "Color")) {
                continue;
            }
			if (Contains(line.first, "Name")) {
                continue;
            }
			if (Contains(line.first, "Type")) {
                continue;
            }
			auto const oldnodes = ExpandNodes(line.second);
			std::vector<std::string> oldNodeArray;
			Split(oldnodes, ',', oldNodeArray);
			std::string newNodes;
			for (auto const& node : oldNodeArray) {
				long val = std::strtol(node.c_str(), nullptr, 10);
				if (val != 0 || node == "0") {
					long newVal = max - val;
                    if (!newNodes.empty()) {
                        newNodes += ",";
                    }
					newNodes += std::to_string(newVal);
				}
			}
			if (!newNodes.empty()) {
                line.second = CompressNodes(newNodes);
            }
		}
	}
};