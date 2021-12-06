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

#include <wx/wx.h>
#include <string>
#include <vector>

int Automation(bool verbose, const std::string& ip, int ab, const std::string& templateFile,
               const std::string& command, const std::vector<wxString>& parameters, const std::string& script);


int DoXLDoCommands(int argc, char **argv);
