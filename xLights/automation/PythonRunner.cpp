/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "PythonRunner.h"
#include "xLightsMain.h"
#include "BatchRenderDialog.h"
#include "UtilFunctions.h"
#include "../ExternalHooks.h"

#include <log4cpp/Category.hh>

#include <wx/stdpaths.h>

#include <cmath>

#if __has_include("Python.h") && __has_include(<pybind11/pybind11.h>)
#include <pybind11/embed.h>
#include <pybind11/pytypes.h>
#include <pybind11/stl.h>

namespace py = pybind11;
using namespace py::literals;
#define PYTHON_RUNNER
#pragma message("Compiling with pybind11")
#endif

PythonRunner::PythonRunner(xLightsFrame* frame) :
    _frame(frame)
{}

std::string PythonRunner::GetUserScriptFolder() const
{
    return _frame->GetShowDirectory() + wxFileName::GetPathSeparator() + "scripts" + wxFileName::GetPathSeparator();
}

std::string PythonRunner::GetSystemScriptFolder()
{
#ifndef __WXMSW__
    return wxStandardPaths::Get().GetResourcesDir() + wxFileName::GetPathSeparator() + "scripts";
#else
    return wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath() + wxFileName::GetPathSeparator() + "scripts";
#endif
}

void PythonRunner::ShowMessage(std::string text) const
{
    wxMessageBox(text, text, wxOK);
}

std::string PythonRunner::PromptString(std::string const& message) const
{
    wxTextEntryDialog dialog(_frame, message, message);
    if (dialog.ShowModal() == wxID_OK) {
        return dialog.GetValue();
    }
    return {};
}

std::string PythonRunner::PromptSelection(std::list<std::string> const& items, std::string const& message) const
{
    wxArrayString itemList;

    std::transform(items.begin(), items.end(),
                           std::back_inserter(itemList), [](auto const& str) { return str; });

    itemList.Sort();
    wxSingleChoiceDialog dlg(_frame, message, message, itemList);

    if (dlg.ShowModal() == wxID_OK) {
        return dlg.GetStringSelection();
    }
    return {};
}

std::list<std::string> PythonRunner::PromptSequences() const
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

bool PythonRunner::Run_Script(wxString const& filepath, std::function<void (std::string const& msg)> SendResponse)
{
    #if defined(PYTHON_RUNNER)
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    std::string const user_scripts_folder = GetUserScriptFolder();
    std::string const system_scripts_folder = GetUserScriptFolder();
    std::string folder = _frame->GetShowDirectory();

    py::scoped_interpreter guard{};

    try {
        auto locals = py::dict("showfolder"_a=folder,"userscripsfolder"_a=user_scripts_folder,"systemscripsfolder"_a=system_scripts_folder );

        locals["showmessage"] = py::cpp_function([&](std::string const& value) { return ShowMessage( value); });
        locals["promptsequences"] = py::cpp_function([&]() { return PromptSequences(); });
        locals["PromptString"] = py::cpp_function([&](std::string const& value) { return PromptString(value); });
        locals["PromptSelection"] = py::cpp_function([&](std::list<std::string> const& items, std::string const& value) { return PromptSelection(items,value); });
        locals["log"] = py::cpp_function(SendResponse);
        locals["runcommand"] = py::cpp_function([&](std::string const& cmd, const pybind11::dict& dict) { return RunCommand( cmd, dict); });

        py::eval_file(filepath.ToStdString(), py::globals(), locals);

    } catch (std::exception& e) {
        logger_base.info("PythonRunner: Throw Running Script: %s.", (const char*)filepath.c_str());
        logger_base.info("PythonRunner: Error: %s.", e.what());
        SendResponse(e.what());
        wxMessageBox(e.what(), "Error", wxOK);
        return false;
    }
    #endif
    return true;
}

std::string PythonRunner::RunCommand(std::string const& cmd, const pybind11::dict& dict)
{
    return _frame->ProcessxlDoAutomation(CommandtoString(cmd, dict));
}

wxString PythonRunner::JSONtoString(wxJSONValue const& json) const
{
    wxJSONWriter writer(wxJSONWRITER_NONE, 0, 0);
    wxString p;
    writer.Write(json, p);
    return p;
}

wxString PythonRunner::CommandtoString(std::string const& cmd, const pybind11::dict& dict) const
{
    wxJSONValue cmds;
    cmds["cmd"] = cmd;
#if defined(PYTHON_RUNNER)
    for (auto item : dict) {

        wxString key = std::string(py::str(item.first));
        wxString value =  std::string(py::str(item.second));
        cmds[key] = value;
    }
#endif
    return JSONtoString(cmds);
}