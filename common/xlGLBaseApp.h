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

#include <wx/glcanvas.h>

#include "xlBaseApp.h"

class xlGLBaseApp : public wxGLApp, public xlCrashHandler
{
public:
    xlGLBaseApp(std::string const& appName) :
        wxGLApp(),
        xlCrashHandler(appName)
    {
    }

    virtual xlFrame* GetTopWindow() override
    {
        return (xlFrame*)wxGLApp::GetTopWindow();
    }

    virtual void OnAssertFailure(wxChar const* file, int line, wxChar const* func, wxChar const* cond, wxChar const* msg) override
    {
        HandleAssertFailure(file, line, func, cond, msg);
        wxGLApp::OnAssertFailure(file, line, func, cond, msg);
    }

    virtual bool OnExceptionInMainLoop() override
    {
        HandleCrash(true, "Exception from main loop. " + xlCrashHandler::DescribeCurrentException());
        return false;
    }

    virtual void OnFatalException() override
    {
        HandleCrash(true, "Fatal exception occurred.");
        wxGLApp::OnFatalException();
    }

    virtual void OnUnhandledException() override
    {
        HandleUnhandledException();
        wxGLApp::OnUnhandledException();
    }
};
