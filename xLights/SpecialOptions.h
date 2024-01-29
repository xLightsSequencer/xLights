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


// Special options are read from an xml file in the show folder
// its use is for options we want the user to be able to set when required but not common enough to put in the UI

struct SpecialOptions
{
    static std::string StashShowDir(const std::string& showDir = "");
    static std::string GetOption(const std::string& option, const std::string& defaultValue = "");
};
