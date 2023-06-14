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
#include <condition_variable>
#include <mutex>
#include <string>

#include <wx/app.h>
#include <wx/debugrpt.h>
#include <wx/frame.h>
#include <wx/glcanvas.h>


class xlCrashHandler;

class xlFrame : public wxFrame
{
public:
    virtual std::string GetCurrentDir() const { return ""; }
    virtual void CreateDebugReport(xlCrashHandler* crashHandler) = 0;
};

class xlCrashHandler
{
public:
    enum SendReportOptions
    {
        ASK_USER_TO_SEND,
        ALWAYS_SEND,
        NEVER_SEND
    };

    xlCrashHandler(std::string const& appName);
    virtual ~xlCrashHandler() = default;

    wxDebugReportCompress& GetDebugReport() const { return *m_report; }
    virtual xlFrame* GetTopWindow() = 0;
    void HandleAssertFailure(wxChar const* file, int line, wxChar const* func, wxChar const* cond, wxChar const* msg);
    void HandleCrash(bool const isFatalException, std::string const& msg);
    void HandleUnhandledException();
    void ProcessCrashReport(SendReportOptions sendType);
    static void SendReport(std::string const& appName, std::string const& loc, wxDebugReportCompress& report);
    static void SetupCrashHandlerForNonWxThread();

private:
    std::string m_appName;
    std::mutex m_crashMutex;
    std::condition_variable m_crashDoneSignal;
    wxDebugReportCompress* m_report;
};

class xlBaseApp : public wxApp, public xlCrashHandler
{
public:
    xlBaseApp(std::string const& appName) :
        wxApp(),
        xlCrashHandler(appName)
    {
    }

    virtual xlFrame* GetTopWindow() override
    {
        return (xlFrame*)wxApp::GetTopWindow();
    }

    virtual void OnAssertFailure(wxChar const* file, int line, wxChar const* func, wxChar const* cond, wxChar const* msg) override
    {
        HandleAssertFailure(file, line, func, cond, msg);
        wxApp::OnAssertFailure(file, line, func, cond, msg);
    }

    virtual bool OnExceptionInMainLoop() override
    {
        HandleCrash(false, "Exception from main loop.");
        return false;
    }

    virtual void OnFatalException() override
    {
        HandleCrash(true, "Fatal exception occurred.");
        wxApp::OnFatalException();
    }

    virtual void OnUnhandledException() override
    {
        HandleUnhandledException();
        wxApp::OnUnhandledException();
    }
};

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
        HandleCrash(false, "Exception from main loop.");
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
