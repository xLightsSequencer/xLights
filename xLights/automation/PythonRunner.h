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

#include <wx/string.h>

#include "../../xSchedule/wxJSON/jsonreader.h"
#include "../../xSchedule/wxJSON/jsonwriter.h"

#include <map>
#include <string>

namespace pybind11
{
   class dict;
};

class xLightsFrame;
class wxJSONValue;

class PythonRunner 
{
public:
    explicit PythonRunner(xLightsFrame* frame);

    bool Run_Script(wxString const& filepath, std::function<void(std::string const& msg)> SendResponse);

    [[nodiscard]] std::string GetUserScriptFolder() const;
    [[nodiscard]] static std::string GetSystemScriptFolder();

    [[nodiscard]] std::string RunCommand(std::string const& cmd, const pybind11::dict& dict);

    [[nodiscard]] std::string PromptString(std::string const& text) const;

    [[nodiscard]] std::list<std::string> PromptSequences() const;

    [[nodiscard]] std::string PromptSelection(std::list<std::string> const& items, std::string const& message) const;

    [[nodiscard]] std::list<std::string> SplitString(std::string const& text, char const& delimiter) const;

    [[nodiscard]] std::string getFolder() const;

    void ShowMessage(std::string text) const;

private:
    xLightsFrame* _frame = nullptr;

    [[nodiscard]] wxString JSONtoString(wxJSONValue const& json) const;
    [[nodiscard]] wxString CommandtoString(std::string const& cmd, const pybind11::dict& dict) const;

};
