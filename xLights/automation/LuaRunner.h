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

#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"
#include <lua.h>

#include <map>
#include <string>

class xLightsFrame;
class wxJSONValue;

class LuaRunner 
{
public:
    LuaRunner(xLightsFrame* frame);

    bool Run_Script(wxString const& filepath, std::function<void(std::string const& msg)> SendResponce);

    [[nodiscard]] std::map<std::string, std::string> RunCommand(std::string const& cmd, std::map<std::string, std::string> parms);
    void ShowMessage(std::string const& text) const;
    [[nodiscard]] std::string PromptString(std::string const& text) const;
    [[nodiscard]] std::string PromptSelection(std::list<std::string> const& items, std::string const& message) const;
    [[nodiscard]] std::list<std::string> PromptSequences() const;
    [[nodiscard]] std::list<std::string> GetModels() const;
    [[nodiscard]] std::list<std::string> GetContollers() const;

private:
    xLightsFrame* _frame = nullptr;

    [[nodiscard]] wxString JSONtoString(wxJSONValue const& json) const;
    [[nodiscard]] wxString CommandtoString(std::string const& cmd, std::map<std::string, std::string> const& parms) const;
    [[nodiscard]] std::map<std::string, std::string> JSONStringToMap(wxString const& json) const;
    //[[nodiscard]] sol::object JSONStringToObject(sol::this_state s, wxString const& json) const;
};
