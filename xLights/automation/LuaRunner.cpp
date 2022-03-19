/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "LuaRunner.h"
#include "xLightsMain.h"
#include "BatchRenderDialog.h"
#include "UtilFunctions.h"
#include "../ExternalHooks.h"

#include <log4cpp/Category.hh>

#include <wx/stdpaths.h>

LuaRunner::LuaRunner(xLightsFrame* frame) :
    _frame(frame)
{}

std::string LuaRunner::GetUserScriptFolder() const
{
    return _frame->GetShowDirectory() + wxFileName::GetPathSeparator() + "scripts" + wxFileName::GetPathSeparator();
}

std::string LuaRunner::GetSystemScriptFolder()
{
#ifndef __WXMSW__
    return wxStandardPaths::Get().GetResourcesDir() + wxFileName::GetPathSeparator() + "scripts";
#else
    return wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath() + wxFileName::GetPathSeparator() + "scripts";
#endif
}

void LuaRunner::ShowMessage(std::string const& text) const
{
    wxMessageBox(text, text, wxOK);
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

std::list<std::string> LuaRunner::PromptSequences() const
{
    std::list<std::string> sequenceList;
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    BatchRenderDialog dlg(_frame);
    dlg.SetTitle("Select Sequences");
    if (dlg.Prepare(_frame->GetShowDirectory()) && dlg.ShowModal() == wxID_OK) {
        wxArrayString files = dlg.GetFileList();

        for (auto const& f : files) {
            wxFileName fname(_frame->GetShowDirectory() + wxFileName::GetPathSeparator() + f);
            if (FileExists(fname)) {
                sequenceList.push_back(fname.GetFullPath());
            } else {
                logger_base.info("PromptSequences: Sequence File not Found: %s.", (const char*)fname.GetFullPath().c_str());
            }
        }
    }
    return sequenceList;
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

bool LuaRunner::Run_Script(wxString const& filepath, std::function<void (std::string const& msg)> SendResponce)
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
    lua.set_function("PromptString", &LuaRunner::PromptString, this);
    lua.set_function("PromptSelection", &LuaRunner::PromptSelection, this);
    lua.set_function("SplitString", &LuaRunner::SplitString, this);
    lua.set_function("JoinString", &LuaRunner::JoinString, this);
    lua.set_function("JSONToTable", &LuaRunner::JSONToTable, this);

    auto SendObjectResponce = [&](sol::object const& val) {
        SendObjResponce(val, SendResponce);
    };

    lua.set_function("Log", SendObjectResponce);

    try {
        sol::load_result lr = lua.load_file(filepath);

        // check if it's sucessfully loaded
        if (!lr.valid()) {
            sol::error err = lr;
            logger_base.info("LuaRunner: Script is Invaid: %s.", (const char*)filepath.c_str());
            logger_base.info("LuaRunner: Error: %s.", err.what());
            wxMessageBox("Script is Invaid: " + filepath + "\n\n" + err.what(), "Load Script Error", wxOK);
            return false;
        }
        auto result2 = lr();

        // check if it was done properly
        if (!result2.valid()) {
            sol::error err2 = result2;
            logger_base.info("LuaRunner: Error Runing Script: %s.", (const char*)filepath.c_str());
            logger_base.info("LuaRunner: Error: %s.", err2.what());
            SendResponce(err2.what());
            wxMessageBox("Error Runing Script: " + filepath + "\n\n" + err2.what(), "Script Error", wxOK);
            return false;
        }
    } catch (std::exception& e) {
        logger_base.info("LuaRunner: Throw Runing Script: %s.", (const char*)filepath.c_str());
        logger_base.info("LuaRunner: Error: %s.", e.what());
        SendResponce(e.what());
        wxMessageBox(e.what(), "Error", wxOK);
        return false;
    }
    return true;
}

void LuaRunner::SendObjResponce(sol::object const& val, std::function<void(std::string const& msg)> SendResponce) const
{
    if (val.is<bool>()) {
        return SendResponce(toStr(val.as<bool>()));
    }
    if (val.is<int>()) {
        return SendResponce(std::to_string(val.as<int>()));
    }
    if (val.is<double>()) {
        return SendResponce(std::to_string(val.as<double>()));
    }
    if (val.is<std::string>()) {
        return SendResponce(val.as<std::string>());
    }
    if (val.get_type() == sol::type::table) {
        for (auto const& it : val.as<sol::table>()) {
            SendObjResponce(it.first, SendResponce);
            SendObjResponce(it.second, SendResponce);
        }
        return;
    } else if (val.get_type() == sol::type::userdata) {
        if (val.is<std::list<std::string>>()) {
            auto const str_item = val.as<std::list<std::string>>();
            for (auto const& str : str_item) {
                SendResponce(str);
            }
            return;
        }
    }
    SendResponce("UnkownType");
}

sol::object LuaRunner::RunCommand(std::string const& cmd, std::map<std::string, std::string> parms, sol::this_state thislua)
{
    return JSONToTable(_frame->ProcessxlDoAutomation(CommandtoString(cmd, parms)), thislua);
}

wxString LuaRunner::JSONtoString(wxJSONValue const& json) const
{
    wxJSONWriter writer(wxJSONWRITER_NONE, 0, 0);
    wxString p;
    writer.Write(json, p);
    return p;
}

wxString LuaRunner::CommandtoString(std::string const& cmd, std::map<std::string, std::string> const& parms) const
{
    wxJSONValue cmds;
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
    wxJSONValue val;
    wxJSONReader reader;
    if (reader.Parse(wxString(json), &val) == 0) {
        return getObjectType(val, lua);
    }
    logger_base.info("LuaRunner: Could not Parse JSON: %s.", (const char*)json.c_str());
    return sol::make_object(lua, json);
}

sol::object LuaRunner::getObjectType(wxJSONValue const& val, sol::state_view lua) const
{
    if (val.IsString()) {
        return sol::make_object(lua, val.AsString().ToStdString());
    }
    if (val.IsInt()) {
        return sol::make_object(lua, val.AsInt());
    }
    if (val.IsLong()) {
        return sol::make_object(lua, val.AsLong());
    } 
    if (val.IsDouble()) {
        return sol::make_object(lua, val.AsDouble());
    } 
    if (val.IsBool()) {
        return sol::make_object(lua,val.AsBool());
    } 
    if (val.IsArray()) {
        sol::table arry = lua.create_table();
        for (int x = 0; x < val.Size(); x++) {
            arry[x] = getObjectType(val.ItemAt(x), lua);
        }
        return arry;
    } 
    if (val.IsObject()) {
        sol::table obj = lua.create_table();
        for (const auto& it : val.GetMemberNames()) {
            obj[it.ToStdString()] = getObjectType(val.ItemAt(it), lua);
        }
        return obj;
    }
    return sol::make_object(lua, nullptr);
}

wxArrayString LuaRunner::getArrayString(sol::object const& items) const
{
    wxArrayString itemList;
    if (items.get_type() == sol::type::table) {
        for (auto const& it : items.as<sol::table>()) {
            if (it.second.get_type() == sol::type::string) {
                itemList.Add(it.second.as<std::string>());
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
