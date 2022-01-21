/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/
#include <iomanip>
#include <thread>

#include <wx/dirdlg.h>
#include <wx/msw/crashrpt.h>
#include <wx/msw/seh.h>

#include <log4cpp/Category.hh>
#include <log4cpp/PropertyConfigurator.hh>
#include <log4cpp/Configurator.hh>

#if 0
#ifndef __WXMSW__
#include <execinfo.h>
#endif
#endif
#include "xLightsVersion.h"
#include "xlBaseApp.h"
#include "xlStackWalker.h"

static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
xlCrashReporter* xlCrashHandler::m_reporter = nullptr;

void xlCrashHandler::SetupCrashHandlerForNonWxThread()
{
#ifdef __WXMSW__
    DisableAutomaticSETranslator();
#endif
}

void xlCrashHandler::HandleCrash(bool const isFatalException, std::string const& msg)
{
//#if defined(_DEBUG)
//    // During debug, don't generate crash dumps. Instead, just bring debugger here.
//    wxTrap();
//#else
    static bool inCrashHandler = false;
    wxDebugReportCompress report;

    try
    {
        logger_base.crit("Crashed: " + msg);

        // Protect against simultaneous crashes from different threads.
        if (inCrashHandler)
        {
            //need to ignore any crashes in the crash handler
            return;
        }
        inCrashHandler = true;

        wxDebugReport::Context const ctx = isFatalException ? wxDebugReport::Context_Exception : wxDebugReport::Context_Current;

        report.AddContext(ctx);

        wxFileName fn(report.GetDirectory(), report.GetReportName(), wxT("dmp"));
        wxCrashReport::SetFileName(fn.GetFullPath());

        int const crashRptFlags = wxCRASH_REPORT_LOCATION | wxCRASH_REPORT_STACK | wxCRASH_REPORT_GLOBALS;

        if ((ctx == wxDebugReport::Context_Exception) ? wxCrashReport::Generate(crashRptFlags) : wxCrashReport::GenerateNow(crashRptFlags))
        {
            report.AddFile(fn.GetFullName(), _("dump of the process state (binary)"));
        }

        wxString backtrace = wxString::Format("xLights version %s\n", GetDisplayVersionString());
        backtrace += "Time: " + wxDateTime::Now().FormatISOCombined() + "\n";

        std::ostringstream threadIdStr;
        threadIdStr << std::showbase // show the 0x prefix
                    << std::internal // fill between the prefix and the number
                    << std::setfill('0') << std::setw(10)
                    << std::hex << std::this_thread::get_id();

        backtrace += wxString::Format("Crashed Thread ID: %s", threadIdStr.str());

        if (wxThread::IsMain())
        {
            backtrace += " (Main Thread)";
        }

        backtrace += "\n\n";

    #ifdef __WXMSW__
        xlStackWalker sw(!isFatalException, isFatalException);
        backtrace += sw.GetStackTrace();
    #else
        void* callstack[128];
        int i, frames = backtrace(callstack, 128);
        char** strs = backtrace_symbols(callstack, frames);
        for (i = 0; i < frames; ++i)
        {
            backtrace += strs[i];
            backtrace += "\n";
        }
        free(strs);
    #endif

        report.AddText("backtrace.txt", backtrace, "Backtrace");
        logger_base.crit("%s", backtrace.c_str());

        if (m_reporter != nullptr)
        {
            m_reporter->CreateDebugReport(report);
        }
    }
    catch (...)
    {
        logger_base.crit("We had an exception within the HandleCrash() function.");
    }

    inCrashHandler = false;
//#endif // (defined(_DEBUG))
}

void xlCrashHandler::SetupCrashHandlerForApp(xlCrashReporter& reporter)
{
    m_reporter = &reporter;
    wxHandleFatalExceptions();
}

void xlCrashHandler::OnAssertFailure(wxChar const* file, int line, wxChar const* func, wxChar const* cond, wxChar const* msg)
{
//#if defined(_DEBUG)
//    // During debug, don't generate crash dumps. Instead, just allow debugger to stop at failed ASSERT.
//    wxTrapInAssert = true;
//#else
    wxString assertMsg;
    assertMsg << wxASCII_STR("Assert: ") << cond
        << wxASCII_STR(" failed at ") << file << wxASCII_STR(":") << line
        << wxASCII_STR(" in ") << func << wxASCII_STR(" with message '")
        << msg << wxASCII_STR("'");

    HandleCrash(false, assertMsg.ToStdString());
//#endif
}

void xlCrashHandler::OnUnhandledException()
{
    // we're called from an exception handler so we can re-throw the exception to recover its type
    std::string msg;
    
    try
    {
        throw;
    }
    catch (char const* eMsg)
    {
        msg = std::string(eMsg);
    }
    catch (std::exception& e)
    {
        msg = "Standard STD exception of type \"";
        msg += typeid(e).name();
        msg += "\" with message \"";
        msg += e.what();
        msg += "\".";
    }
    catch (...)
    {
        msg = "An unknown exception occurred.";
    }

    HandleCrash(false, msg);
    wxAbort();
}
