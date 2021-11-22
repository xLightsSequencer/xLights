
#pragma once

#include <wx/wx.h>
#include <string>
#include <vector>

int Automation(bool verbose, const std::string& ip, int ab, const std::string& templateFile,
               const std::string& command, const std::vector<wxString>& parameters, const std::string& script);


int DoXLDoCommands(int argc, char **argv);
