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
    explicit LuaRunner(xLightsFrame* frame);

    bool Run_Script(wxString const& filepath, std::function<void(std::string const& msg)> SendResponse);

    [[nodiscard]] std::string GetUserScriptFolder() const;
    [[nodiscard]] static std::string GetSystemScriptFolder();

    [[nodiscard]] sol::object RunCommand(std::string const& cmd, std::map<std::string, std::string> parms, sol::this_state thislua);
    void ShowMessage(std::string const& text) const;
    [[nodiscard]] std::string PromptString(std::string const& text) const;
    [[nodiscard]] std::string PromptSelection(sol::object const& items, std::string const& message) const;
    [[nodiscard]] std::list<std::string> PromptSequences() const;
    
    [[nodiscard]] sol::object JSONToTable(std::string const& json, sol::this_state s) const;
    [[nodiscard]] std::list<std::string> SplitString(std::string const& text, char const& delimiter) const;
    [[nodiscard]] std::string JoinString(sol::object const& list, char const& delimiter) const;
    [[nodiscard]] std::string TableToJSON(sol::object const& items) const;

private:
    xLightsFrame* _frame = nullptr;

    [[nodiscard]] wxString JSONtoString(wxJSONValue const& json) const;
    [[nodiscard]] wxString CommandtoString(std::string const& cmd, std::map<std::string, std::string> const& parms) const;
    [[nodiscard]] sol::object getObjectType(wxJSONValue const& val, sol::state_view lua) const;
    [[nodiscard]] wxArrayString getArrayString(sol::object const& items) const;
    void SendObjResponse(sol::object const& val, std::function<void(std::string const& msg)> SendResponse) const;
    void ObjectToJSON(sol::object const& items, wxJSONValue& json) const;
    bool is_integer(double n) const;
};
