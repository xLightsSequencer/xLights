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
#include <condition_variable>
#include <mutex>
#include <string>

#include <wx/app.h>
#include <wx/debugrpt.h>
#include <wx/frame.h>


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
    // Must be called from within a catch handler — rethrows and describes the in-flight exception.
    static std::string DescribeCurrentException();
    void ProcessCrashReport(SendReportOptions sendType);
    static void SendReport(std::string const& appName, std::string const& loc, wxDebugReportCompress& report);
    static void SetupCrashHandlerForNonWxThread();

    // Exercises the unwind-capture path when XL_UNWIND_HOOK_TEST=1.  Must run
    // after wx is initialised - a throw during wxEntryStart is not catchable.
    static void TestUnwindHookIfRequested();

private:
    std::string m_appName;
    // Timed, so a thread that arrives while another is stuck partway through
    // reporting gives up instead of blocking behind it forever.  Paired with
    // condition_variable_any because std::condition_variable only accepts
    // unique_lock<std::mutex>.
    std::timed_mutex m_crashMutex;
    std::condition_variable_any m_crashDoneSignal;
    // Predicate for m_crashDoneSignal.  Without it the wait can miss the notify
    // entirely: the report is built on the main thread via CallAfter, which can
    // finish and signal before the crashing thread reaches the wait, and a
    // condition_variable does not remember a notify nobody was waiting for.
    bool m_crashReportDone = false;
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
        HandleCrash(true, "Exception from main loop. " + xlCrashHandler::DescribeCurrentException());
        return false;
    }

    virtual int OnRun() override
    {
        xlCrashHandler::TestUnwindHookIfRequested();
        return wxApp::OnRun();
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

