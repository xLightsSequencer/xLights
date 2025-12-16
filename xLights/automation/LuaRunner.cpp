/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "LuaRunner.h"
#include "xLightsMain.h"
#include "BatchRenderDialog.h"
#include "UtilFunctions.h"
#include "../ExternalHooks.h"

#include <log4cpp/Category.hh>

#include <wx/stdpaths.h>

#include <cmath>

LuaRunner::LuaRunner(xLightsFrame* frame) :
    _frame(frame)
{}

std::string LuaRunner::GetUserScriptFolder() const
{
    return _frame->GetShowDirectory() + GetPathSeparator() + "scripts" + GetPathSeparator();
}

std::string LuaRunner::GetSystemScriptFolder()
{
#ifndef __WXMSW__
    return wxStandardPaths::Get().GetResourcesDir() + GetPathSeparator() + "scripts";
#else
    return wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath() + GetPathSeparator() + "scripts";
#endif
}

void LuaRunner::ShowMessage(std::string const& text) const
{
    wxMessageBox(text, text, wxOK);
}

std::string LuaRunner::PromptOption(std::string const& question, std::string const& title, std::string const& button1, std::string const& button2) const
{
    wxMessageDialog dlg(_frame, question, title, wxYES_NO | wxICON_QUESTION);
    dlg.SetYesNoLabels(button1.c_str(), button2.c_str());
    return dlg.ShowModal() == wxID_YES ? button1 : button2;
}

std::string LuaRunner::PromptString(std::string const& message) const
{
    wxTextEntryDialog dialog(_frame, message, message);
    if (dialog.ShowModal() == wxID_OK) {
        return dialog.GetValue();
    }
    return {};
}

std::string LuaRunner::PromptSelection(sol::object const& items, std::string const& message) const
{
    wxArrayString itemList = getArrayString(items);

    itemList.Sort();
    wxSingleChoiceDialog dlg(_frame, message, message, itemList);

    if (dlg.ShowModal() == wxID_OK) {
        return dlg.GetStringSelection();
    }
    return {};
}

std::pair<std::list<std::string>, bool> LuaRunner::PromptSequences() const
{
    std::list<std::string> sequenceList;
    bool forceHighDefinitionRender = false;
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    BatchRenderDialog dlg(_frame, _frame->GetOutputManager());
    dlg.SetTitle("Select Sequences");
    if (dlg.Prepare(_frame->GetShowDirectory()) && dlg.ShowModal() == wxID_OK) {
        wxArrayString files = dlg.GetFileList();

        for (auto const& f : files) {
            wxFileName fname(_frame->GetShowDirectory() + GetPathSeparator() + f);
            if (FileExists(fname)) {
                sequenceList.push_back(fname.GetFullPath());
            } else {
                logger_base.info("PromptSequences: Sequence File not Found: %s.", (const char*)fname.GetFullPath().c_str());
            }
        }
        if (dlg.CheckBox_ForceHighDefinition->IsChecked()) {
            forceHighDefinitionRender = true;
        }
    }
    return std::make_pair(sequenceList, forceHighDefinitionRender);
}

std::list<std::string> LuaRunner::SplitString(std::string const& text, char const& delimiter) const
{
    auto items = wxSplit(text, delimiter);
    std::list<std::string> itemList;

    std::transform(items.begin(), items.end(), std::back_inserter(itemList),
                   [](auto const& str) { return str; });
    return itemList;
}

std::string LuaRunner::JoinString(sol::object const& list, char const& delimiter) const
{
    wxArrayString const itemList = getArrayString(list);

    return wxJoin(itemList, delimiter);
}

bool LuaRunner::Run_Script(std::string const& filepath, std::function<void(std::string const& msg)> SendResponse)
{ 
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    sol::state lua;
    // open some common libraries
    lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::table, sol::lib::os, sol::lib::io, sol::lib::string, sol::lib::math );

    std::string const path = lua["package"]["path"];

    std::string const user_scripts_folder = GetUserScriptFolder();
    std::string const system_scripts_folder = GetUserScriptFolder();
    lua["package"]["path"] = path + ";" + system_scripts_folder + "?.lua;" + user_scripts_folder + "?.lua ";

    lua.set("userscripsfolder", user_scripts_folder);
    lua.set("systemscripsfolder", system_scripts_folder);
    lua.set("showfolder", _frame->GetShowDirectory());
    lua.set_function("RunCommand", &LuaRunner::RunCommand, this);

    lua.set_function("PromptSequences", &LuaRunner::PromptSequences, this);
    lua.set_function("ShowMessage", &LuaRunner::ShowMessage, this);
    lua.set_function("PromptOption", &LuaRunner::PromptOption, this);
    lua.set_function("PromptString", &LuaRunner::PromptString, this);
    lua.set_function("PromptSelection", &LuaRunner::PromptSelection, this);
    lua.set_function("SplitString", &LuaRunner::SplitString, this);
    lua.set_function("JoinString", &LuaRunner::JoinString, this);
    lua.set_function("JSONToTable", &LuaRunner::JSONToTable, this);
    lua.set_function("TableToJSON", &LuaRunner::TableToJSON, this);

    auto SendObjectResponse = [&](sol::object const& val) {
        SendObjResponse(val, SendResponse);
    };

    lua.set_function("Log", SendObjectResponse);

    try {
        sol::load_result lr = lua.load_file(filepath);

        // check if it's successfully loaded
        if (!lr.valid()) {
            sol::error err = lr;
            logger_base.info("LuaRunner: Script is Invalid: %s.", (const char*)filepath.c_str());
            logger_base.info("LuaRunner: Error: %s.", err.what());
            wxMessageBox("Script is Invalid: " + filepath + "\n\n" + err.what(), "Load Script Error", wxOK);
            return false;
        }
        auto result2 = lr();

        // check if it was done properly
        if (!result2.valid()) {
            sol::error err2 = result2;
            logger_base.info("LuaRunner: Error Running Script: %s.", (const char*)filepath.c_str());
            logger_base.info("LuaRunner: Error: %s.", err2.what());
            SendResponse(err2.what());
            wxMessageBox("Error Running Script: " + filepath + "\n\n" + err2.what(), "Script Error", wxOK);
            return false;
        }
    } catch (std::exception& e) {
        logger_base.info("LuaRunner: Throw Running Script: %s.", (const char*)filepath.c_str());
        logger_base.info("LuaRunner: Error: %s.", e.what());
        SendResponse(e.what());
        wxMessageBox(e.what(), "Error", wxOK);
        return false;
    }
    return true;
}

void LuaRunner::SendObjResponse(sol::object const& val, std::function<void(std::string const& msg)> SendResponse) const
{
    if (val.is<bool>()) {
        return SendResponse(toStr(val.as<bool>()));
    }
    if (val.is<int>()) {
        return SendResponse(std::to_string(val.as<int>()));
    }
    if (val.is<double>()) {
        return SendResponse(std::to_string(val.as<double>()));
    }
    if (val.is<std::string>()) {
        return SendResponse(val.as<std::string>());
    }
    if (val.get_type() == sol::type::table) {
        for (auto const& it : val.as<sol::table>()) {
            SendObjResponse(it.first, SendResponse);
            SendObjResponse(it.second, SendResponse);
        }
        return;
    } 
    if (val.get_type() == sol::type::userdata) {
        if (val.is<std::list<std::string>>()) {
            auto const str_item = val.as<std::list<std::string>>();
            for (auto const& str : str_item) {
                SendResponse(str);
            }
            return;
        }
    }
    SendResponse("UnkownType");
}

sol::object LuaRunner::RunCommand(std::string const& cmd, std::map<std::string, std::string> parms, sol::this_state thislua)
{
    return JSONToTable(_frame->ProcessxlDoAutomation(CommandtoString(cmd, parms)), thislua);
}

std::string LuaRunner::JSONtoString(nlohmann::json const& json) const {
    return json.dump(3);
}

std::string LuaRunner::CommandtoString(std::string const& cmd, std::map<std::string, std::string> const& parms) const
{
    nlohmann::json cmds;
    cmds["cmd"] = cmd;
    for (auto const&[key, value] : parms) {
        cmds[key] = value;
    }
    return JSONtoString(cmds);
}

sol::object LuaRunner::JSONToTable(std::string const& json, sol::this_state s) const
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    sol::state_view lua = s;
    try {
        nlohmann::json val = nlohmann::json::parse(json);
        return getObjectType(val, lua);
    } catch (std::exception&) {
    }
    logger_base.info("LuaRunner: Could not Parse JSON: %s.", (const char*)json.c_str());
    return sol::make_object(lua, json);
}

std::string LuaRunner::TableToJSON(sol::object const& item) const
{
    nlohmann::json json;
    ObjectToJSON(item, json);
    return json.dump(3);
}

void LuaRunner::ObjectToJSON(sol::object const& items, nlohmann::json& json) const
{
    if (items.get_type() == sol::type::table) {
        for (auto const& it : items.as<sol::table>()) {
            auto key = it.first.as<std::string>();
            if (it.second.get_type() == sol::type::table) {
                ObjectToJSON(it.second, json[key]);
            } else if(it.second.get_type() == sol::type::string) {
                json[key] = it.second.as<std::string>();
            } else if (it.second.get_type() == sol::type::boolean) {
                json[key] = toStr(it.second.as<bool>());
            } else if (it.second.get_type() == sol::type::number) {
                if (is_integer(it.second.as<double>())) {
                    json[key] = std::to_string(it.second.as<int>());
                } else {
                    json[key] = std::to_string(it.second.as<double>());
                }
            }
        }
    } else if (items.get_type() == sol::type::userdata) {
        if (items.is<std::map<std::string, std::string>>()) {
            for (auto const& [key, val] : items.as<std::map<std::string, std::string>>()) {
                json[key] = val;
            }
        }
    }
}

sol::object LuaRunner::getObjectType( nlohmann::json const& val, sol::state_view lua) const
{
    if (val.is_string()) {
        return sol::make_object(lua, val.get<std::string>());
    }
    if (val.is_number_integer()) {
        return sol::make_object(lua, val.get<int>());
    }

    if (val.is_number_float()) {
        return sol::make_object(lua, val.get<float>());
    }
    if (val.is_boolean()) {
        return sol::make_object(lua, val.get<bool>());
    }
    if (val.is_array()) {
        sol::table arry = lua.create_table();
        for (int x = 0; x < val.size(); x++) {
            arry[x] = getObjectType(val.at(x), lua);
        }
        return arry;
    }
    if (val.is_object()) {
        sol::table obj = lua.create_table();
        for (auto [nn, item] : val.items()) {
            obj[nn] = getObjectType(item, lua);
        }
        return obj;
    }
    return sol::make_object(lua, nullptr);
}

std::vector <std::string> LuaRunner::getArrayString(sol::object const& items) const
{
    std::vector<std::string> itemList;
    if (items.get_type() == sol::type::table) {
        for (auto const& it : items.as<sol::table>()) {
            if (it.second.get_type() == sol::type::string) {
                itemList.emplace_back(it.second.as<std::string>());
            }
        }
    } else if (items.get_type() == sol::type::userdata) {
        if (items.is<std::list<std::string>>()) {
            auto const str_item = items.as<std::list<std::string>>();
            std::transform(str_item.begin(), str_item.end(),
                           std::back_inserter(itemList), [](auto const& str) { return str; });
        }
    }
    return itemList;
}

bool LuaRunner::is_integer(double n) const
{
    int c = std::ceil(n);
    int f = std::floor(n);
    return f == c;
}
