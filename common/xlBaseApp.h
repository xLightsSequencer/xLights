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
#include <string>

#include <wx/app.h>
#include <wx/debugrpt.h>
#include <wx/glcanvas.h>


class xlCrashReporter
{
public:
    virtual void CreateDebugReport(wxDebugReportCompress& report) = 0;
};

class xlCrashHandler
{
public:
    static void HandleCrash(bool const isFatalException, std::string const& msg);
    static void OnAssertFailure(wxChar const* file, int line, wxChar const* func, wxChar const* cond, wxChar const* msg);
    static void OnUnhandledException();
    static void SetupCrashHandlerForApp(xlCrashReporter& reporter);
    static void SetupCrashHandlerForNonWxThread();

private:
    static wxDebugReportCompress* m_report;
    static xlCrashReporter* m_reporter;
};

class xlBaseApp : public wxApp, public xlCrashReporter
{
public:
    xlBaseApp()
    {
        xlCrashHandler::SetupCrashHandlerForApp(*this);
    }

    virtual void OnAssertFailure(wxChar const* file, int line, wxChar const* func, wxChar const* cond, wxChar const* msg) override
    {
        xlCrashHandler::OnAssertFailure(file, line, func, cond, msg);
        wxApp::OnAssertFailure(file, line, func, cond, msg);
    }

    virtual bool OnExceptionInMainLoop() override
    {
        xlCrashHandler::HandleCrash(false, "Exception from main loop.");
        return false;
    }

    virtual void OnFatalException() override
    {
        xlCrashHandler::HandleCrash(true, "Fatal exception occurred.");
        wxApp::OnFatalException();
    }

    virtual void OnUnhandledException() override
    {
        xlCrashHandler::OnUnhandledException();
        wxApp::OnUnhandledException();
    }
};

class xlGLBaseApp : public wxGLApp, public xlCrashReporter
{
public:
    xlGLBaseApp()
    {
        xlCrashHandler::SetupCrashHandlerForApp(*this);
    }

    virtual void OnAssertFailure(wxChar const* file, int line, wxChar const* func, wxChar const* cond, wxChar const* msg) override
    {
        xlCrashHandler::OnAssertFailure(file, line, func, cond, msg);
        wxGLApp::OnAssertFailure(file, line, func, cond, msg);
    }

    virtual bool OnExceptionInMainLoop() override
    {
        xlCrashHandler::HandleCrash(false, "Exception from main loop.");
        return false;
    }

    virtual void OnFatalException() override
    {
        xlCrashHandler::HandleCrash(true, "Fatal exception occurred.");
        wxGLApp::OnFatalException();
    }

    virtual void OnUnhandledException() override
    {
        xlCrashHandler::OnUnhandledException();
        wxGLApp::OnUnhandledException();
    }
};
