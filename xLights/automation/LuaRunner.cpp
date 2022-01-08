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

#include <log4cpp/Category.hh>

LuaRunner::LuaRunner(xLightsFrame* frame) :
    _frame(frame)
{}

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
    return std::string();
}

std::string LuaRunner::PromptSelection(std::list<std::string> const& items, std::string const& message) const
{
    wxArrayString itemList;

    std::transform(items.begin(), items.end(), std::back_inserter(itemList),
                   [](auto const& str) { return str; });

    wxSingleChoiceDialog dlg(_frame, message, message, itemList);

    if (dlg.ShowModal() == wxID_OK) {
        return dlg.GetStringSelection();
    }
    return std::string();
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
            if (fname.FileExists()) {
                sequenceList.push_back(fname.GetFullPath());
            } else {
                logger_base.info("PromptSequences: Sequence File not Found: %s.", (const char*)fname.GetFullPath().c_str());
            }
        }
    }
    return sequenceList;
}

std::list<std::string> LuaRunner::GetContollers() const
{
    return _frame->GetOutputManager()->GetControllerNames();
}

std::list<std::string> LuaRunner::GetModels() const
{
    std::list<std::string> models;
    for (auto m = (& _frame->AllModels)->begin();m != (&_frame->AllModels)->end(); ++m){
        models.push_back(m->first);
    }
    return models;
}

bool LuaRunner::Run_Script(wxString const& filepath, std::function<void (std::string const& msg)> SendResponce)
{ 
    sol::state lua;
    // open some common libraries
    lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::table);

    lua.set("showfolder", _frame->GetShowDirectory());
    lua.set_function("RunCommand", &LuaRunner::RunCommand, this);
    lua.set_function("PromptSequences", &LuaRunner::PromptSequences, this);
    lua.set_function("ShowMessage", &LuaRunner::ShowMessage, this);
    lua.set_function("PromptString", &LuaRunner::PromptString, this);
    lua.set_function("PromptSelection", &LuaRunner::PromptSelection, this);
    lua.set_function("GetContollers", &LuaRunner::GetContollers, this);
    lua.set_function("GetModels", &LuaRunner::GetModels, this);
    lua.set_function("Log", SendResponce);

    try {
        sol::load_result lr = lua.load_file(filepath);
        // check if it's sucessfully loaded
        if (!lr.valid()) {
            sol::error err = lr;
            wxMessageBox("Script is Invaid: " + filepath + "\n\n" + err.what(), "Load Script Error", wxOK);
            return false;
        }
        auto result2 = lr();
        // check if it was done properly

        if (!result2.valid()) {
            sol::error err2 = result2;
            SendResponce(err2.what());
            wxMessageBox("Error Runing Script: " + filepath + "\n\n" + err2.what(), "Script Error", wxOK);
            return false;
        }
    } catch (std::exception& e) {
        SendResponce(e.what());
        wxMessageBox(e.what(), "Error", wxOK);
        return false;
    }
    return true;
}

std::map<std::string, std::string> LuaRunner::RunCommand(std::string const& cmd, std::map<std::string, std::string> parms)
{
    return JSONStringToMap(_frame->ProcessxlDoAutomation(CommandtoString(cmd, parms)));
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

std::map<std::string, std::string> LuaRunner::JSONStringToMap(wxString const& json) const
{
    std::map<std::string, std::string> map;
    wxJSONValue val;
    wxJSONReader reader;
    if (reader.Parse(json, &val) == 0) {
        for (const auto& it : val.GetMemberNames()) {
            wxJSONValue v = val[it];
            if (v.IsString()) {
                map.insert({ it, v.AsString() });
            } else if (v.IsLong()) {
                map.insert({it, std::to_string(v.AsLong())});
            } else if (v.IsInt()) {
                map.insert({ it, std::to_string(v.AsInt()) });
            } else if (v.IsBool()) {
                map.insert({ it, toStr(v.AsBool()) });
            } else if (v.IsArray()) {
                for (int x = 0; x < v.Size(); x++) {
                    map.insert({ it + std::to_string(x), v[x].AsString() });
                }
            }
        }
    }
    return map;
}
//doesnt work for some reson
/* sol::object LuaRunner::JSONStringToObject(sol::this_state s, wxString const& json) const
{
    sol::state_view lua(s);
    sol::table obj = lua.create_table();
    wxJSONValue val;
    wxJSONReader reader;
    if (reader.Parse(json, &val) == 0) {
        for (const auto& it : val.GetMemberNames()) {
            wxJSONValue v = val[it];
            if (v.IsString()) {
                obj[it.c_str()] = sol::make_object(lua, v.AsString().ToStdString());
            } else if (v.IsLong()) {
                obj[it.c_str()] = sol::make_object(lua, (double)v.AsLong());
            } else if (v.IsInt()) {
                obj[it.c_str()] = sol::make_object(lua, (double)v.AsInt());
            } else if (v.IsBool()) {
                obj[it.c_str()] = sol::make_object(lua, v.AsBool());
            } else if (v.IsArray()) {
                for (int x = 0; x < v.Size(); x++) {
                    obj[it.c_str()][x] = sol::make_object(lua, v[x].AsString().ToStdString());
                }
            }
        }
    }
    return obj.as<sol::object>();
    //return obj;
}*/
