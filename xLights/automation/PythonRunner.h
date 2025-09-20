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

#include <wx/string.h>

#include <map>
#include <string>

#include <nlohmann/json.hpp>

namespace pybind11
{
   class dict;
};

class xLightsFrame;

class PythonRunner 
{
public:
    explicit PythonRunner(xLightsFrame* frame);

    bool Run_Script(std::string const& filepath, std::function<void(std::string const& msg)> SendResponse);

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

    [[nodiscard]] std::string JSONtoString(nlohmann::json const& json) const;
    [[nodiscard]] std::string CommandtoString(std::string const& cmd, const pybind11::dict& dict) const;

};
