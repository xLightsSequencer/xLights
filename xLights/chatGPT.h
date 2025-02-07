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

class xLightsFrame;

std::string CallChatGPT(xLightsFrame* frame, const std::string& prompt, const std::string& token = "");
bool TestChatGPT(xLightsFrame* frame, const std::string& token = "");
bool IsChatGPTAvailable(xLightsFrame* frame, const std::string& token = "");
