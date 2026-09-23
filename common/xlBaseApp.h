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
#include <wx/datetime.h>
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
    // What brought us here. `isFatalException` cannot answer this: it is true
    // for a faulting signal, for an escaped exception in the main loop and for
    // an unhandled one alike, so a plain segfault would report itself as an
    // unhandled C++ exception.
    enum class SessionType {
        Crash,               // faulting signal / SEH - the ordinary case
        MainLoopException,   // exception escaped into the main event loop
        UnhandledException,  // exception escaped everything
        Assert
    };
    static char const* SessionTypeName(SessionType t);

    void HandleAssertFailure(wxChar const* file, int line, wxChar const* func, wxChar const* cond, wxChar const* msg);
    void HandleCrash(bool const isFatalException, std::string const& msg,
                     SessionType sessionType = SessionType::Crash);
    void HandleUnhandledException();
    // Must be called from within a catch handler — rethrows and describes the in-flight exception.
    static std::string DescribeCurrentException();
    void ProcessCrashReport(SendReportOptions sendType);
    // `fileName` is the upload name; empty asks SendReport to derive one, which
    // only the manual paths need - a crash report names itself in HandleCrash
    // so report.json and the zip agree on the timestamp.
    static void SendReport(std::string const& appName, std::string const& loc, wxDebugReportCompress& report,
                           std::string const& fileName = std::string());
    // The zip's own name, as the server will see it: <app>-<osfamily>[_<arch>]_<version>_<stamp>.zip
    static std::string BuildUploadFileName(std::string const& appName, wxDateTime const& when, int millis);
    static void SetupCrashHandlerForNonWxThread();

    // Records the last few hundred dispatched events and any notes the app
    // leaves, so a crash report can say what the program was doing.  The log
    // cannot: the per-function traces that show up as "breadcrumbs" are debug
    // level, which almost nobody runs with, so reports routinely arrive with
    // nothing between the last routine message and the crash.
    static void StartActivityTrace();
    static void StopActivityTrace();
    // Free-form note, e.g. "play speed 0.5".  text must outlive the call; the
    // detail is copied.  Safe from any thread.
    static void TraceNote(char const* text, std::string const& detail = std::string());
    static wxString FormatActivityTrace();

private:
    std::string m_appName;
    // Timed, so a thread that arrives while another is stuck partway through
    // reporting gives up instead of blocking behind it forever.  Paired with
    // condition_variable_any because std::condition_variable only accepts
    // unique_lock<std::mutex>.  Recursive because the main-thread path holds
    // the lock across the synchronous CreateDebugReport() call, whose
    // ProcessCrashReport() re-enters the lock from the same thread via
    // ReleaseWaiter to signal completion.
    std::recursive_timed_mutex m_crashMutex;
    std::condition_variable_any m_crashDoneSignal;
    // Predicate for m_crashDoneSignal.  Without it the wait can miss the notify
    // entirely: the report is built on the main thread via CallAfter, which can
    // finish and signal before the crashing thread reaches the wait, and a
    // condition_variable does not remember a notify nobody was waiting for.
    bool m_crashReportDone = false;
    // Set as soon as the main thread picks the report up.  The 60s bound below
    // is only there to catch a main thread that never runs the CallAfter; once
    // it has started, the report can legitimately take as long as the user
    // takes to read the preview dialog, and giving up then would free the
    // report out from under the thread still using it.
    bool m_crashReportStarted = false;
    wxDebugReportCompress* m_report;
    // Named in HandleCrash so the name inside report.json is the name the
    // report is actually uploaded under.
    std::string m_uploadFileName;

    // machine_config.txt and report.json, added on every path that produces a
    // report - including the headless / no-top-window / main-thread-wedged
    // ones, which build the zip here rather than in CreateDebugReport and so
    // used to ship neither.
    void AddSessionMetadata(wxDebugReportCompress& report, SessionType sessionType);
};

class xlBaseApp : public wxApp, public xlCrashHandler
{
public:
    xlBaseApp(std::string const& appName) :
        wxApp(),
        xlCrashHandler(appName)
    {
        xlCrashHandler::StartActivityTrace();
    }

    ~xlBaseApp() override
    {
        // The filter is a namespace-scope object; leaving it registered would
        // let wx walk into it after static destruction has run.
        xlCrashHandler::StopActivityTrace();
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
        HandleCrash(true, "Exception from main loop. " + xlCrashHandler::DescribeCurrentException(),
                    SessionType::MainLoopException);
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

