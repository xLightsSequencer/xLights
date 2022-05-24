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

#include <wx/buffer.h>
#include <wx/datetime.h>
#include <wx/dirdlg.h>
#include <wx/msgdlg.h>
#include <wx/protocol/http.h>
#ifdef __WXMSW__
#include <wx/msw/crashrpt.h>
#include <wx/msw/seh.h>
#else
#include <execinfo.h>
#endif

#include <log4cpp/Category.hh>
#include <log4cpp/PropertyConfigurator.hh>
#include <log4cpp/Configurator.hh>

#include "ExternalHooks.h"
#include "xLightsVersion.h"
#include "xlBaseApp.h"
#include "xlStackWalker.h"

xlCrashHandler::xlCrashHandler(std::string const& appName) :
    m_appName(appName),
    m_crashMutex(),
    m_crashDoneSignal(),
    m_report(nullptr)
{
#if wxUSE_ON_FATAL_EXCEPTION
    wxHandleFatalExceptions();
#endif
}

void xlCrashHandler::HandleCrash(bool const isFatalException, std::string const& msg)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    try
    {
#if defined(_DEBUG)
        // During debug, don't generate crash dumps. Instead, just bring debugger here.
        wxTrap();
#else
        // Protect against simultaneous crashes from different threads.
        std::unique_lock<std::mutex> lock(m_crashMutex);

        logger_base.crit("Crashed: " + msg);

        wxDebugReportCompress report;
        m_report = &report;

#ifdef __WXOSX__
        wxMessageBox("If you haven't already, please turn on the system settings to share crash data with the app developers.\n\n To do that, go to:\n"
            "System Preferences -> Security and Privacy -> Privacy -> Analytics & Improvements\n\n"
            "and turn on the \"Share Mac Analytics\" setting and also the \"Share with App Developers\" setting.\n\n"
            "This provides more information to the xLights developers than just our normal crash logs.");
#endif

#if (wxUSE_STACKWALKER || wxUSE_CRASHREPORT)
        wxDebugReport::Context const ctx = isFatalException ? wxDebugReport::Context_Exception : wxDebugReport::Context_Current;
#endif

#if wxUSE_STACKWALKER
        report.AddContext(ctx);
#endif

#if wxUSE_CRASHREPORT
        wxFileName fn(report.GetDirectory(), report.GetReportName(), wxT("dmp"));
        wxCrashReport::SetFileName(fn.GetFullPath());

        int const crashRptFlags = wxCRASH_REPORT_LOCATION | wxCRASH_REPORT_STACK; // | wxCRASH_REPORT_GLOBALS; - remove globals to limit size of DMP file

        if ((ctx == wxDebugReport::Context_Exception) ? wxCrashReport::Generate(crashRptFlags) : wxCrashReport::GenerateNow(crashRptFlags))
        {
            report.AddFile(fn.GetFullName(), _("dump of the process state (binary)"));
        }
#endif

        wxString backtrace_txt = wxString::Format("%s version %s\n", m_appName.c_str(), GetDisplayVersionString());
        backtrace_txt += "Time: " + wxDateTime::Now().FormatISOCombined() + "\n";

        std::ostringstream threadIdStr;
        threadIdStr << std::showbase // show the 0x prefix
                    << std::internal // fill between the prefix and the number
                    << std::setfill('0') << std::setw(10)
                    << std::hex << std::this_thread::get_id();

        backtrace_txt += wxString::Format("Crashed Thread ID: %s", threadIdStr.str());

        if (wxThread::IsMain())
        {
            backtrace_txt += " (Main Thread)";
        }

        backtrace_txt += "\n\n";

    #ifdef __WXMSW__
        xlStackWalker sw(!isFatalException, isFatalException);
        backtrace_txt += sw.GetStackTrace();
    #else
        void* callstack[128];
        int frames = backtrace(callstack, 128);
        char** strs = backtrace_symbols(callstack, frames);
        for (int i = 0; i < frames; ++i)
        {
            backtrace_txt += strs[i];
            backtrace_txt += "\n";
        }
        free(strs);
    #endif

        report.AddText("backtrace.txt", backtrace_txt, "Backtrace");
        logger_base.crit("%s", (const char *)backtrace_txt.c_str());

        xlFrame* const topFrame = GetTopWindow();
        std::string const logFileName = m_appName + "_l4cpp.log";
#ifdef __WXMSW__
        wxString dir;
        wxGetEnv("APPDATA", &dir);
        std::string const logFilePath = std::string(dir.c_str()) + "/" + logFileName;
#endif
#ifdef __WXOSX__
        wxFileName home;
        home.AssignHomeDir();
        wxString const dir = home.GetFullPath();
        std::string const logFilePath = std::string(dir.c_str()) + "/Library/Logs/" + logFileName;
#endif
#ifdef __LINUX__
        std::string const logFilePath = "/tmp/" + logFileName;
#endif

        if (FileExists(logFilePath))
        {
            report.AddFile(logFilePath, logFileName.c_str());
        }
        else if ((topFrame != nullptr) && FileExists(wxFileName(topFrame->GetCurrentDir(), logFileName.c_str()).GetFullPath()))
        {
            report.AddFile(wxFileName(topFrame->GetCurrentDir(), logFileName.c_str()).GetFullPath(), logFileName.c_str());
        }
        else if (FileExists(wxFileName(wxGetCwd(), logFileName.c_str()).GetFullPath()))
        {
            report.AddFile(wxFileName(wxGetCwd(), logFileName.c_str()).GetFullPath(), logFileName.c_str());
        }

        if (topFrame == nullptr)
        {
            logger_base.crit("Unable to tell user about debug report. Crash report saved to %s.", (const char*)report.GetCompressedFileName().c_str());
        }
        else
        {
            if (wxThread::IsMain())
            {
                topFrame->CreateDebugReport(this);
            }
            else
            {
                topFrame->CallAfter(&xlFrame::CreateDebugReport, this);
                m_crashDoneSignal.wait(lock);
            }
        }
#endif // (defined(_DEBUG))
    }
    catch (...)
    {
        logger_base.crit("We had an exception within the HandleCrash() function.");
    }

    m_report = nullptr;
}

void xlCrashHandler::HandleAssertFailure(wxChar const* file, int line, wxChar const* func, wxChar const* cond, wxChar const* msg)
{
    wxString assertMsg;
    assertMsg << wxASCII_STR("Assert: ") << cond
        << wxASCII_STR(" failed at ") << file << wxASCII_STR(":") << line
        << wxASCII_STR(" in ") << func << wxASCII_STR(" with message '")
        << msg << wxASCII_STR("'");

    HandleCrash(false, assertMsg.ToStdString());
}

void xlCrashHandler::HandleUnhandledException()
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

void xlCrashHandler::ProcessCrashReport(SendReportOptions sendOption)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if ((sendOption == SendReportOptions::ALWAYS_SEND) || ((sendOption == SendReportOptions::ASK_USER_TO_SEND) && wxDebugReportPreviewStd().Show(*m_report)))
    {
        m_report->Process();
        SendReport(m_appName, "crashUpload", *m_report);
        wxMessageBox("Crash report saved to " + m_report->GetCompressedFileName());
    }
    else
    {
        m_report->Process();
    }

    logger_base.crit("Created debug report: " + m_report->GetCompressedFileName());
    m_crashDoneSignal.notify_all();
}

void xlCrashHandler::SendReport(std::string const& appName, std::string const& loc, wxDebugReportCompress& report)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxHTTP http;
    http.Connect("dankulp.com");

    static char const bound[] = "--------------------------b29a7c2fe47b9481";
    wxDateTime now = wxDateTime::Now();
    int millis = wxGetUTCTimeMillis().GetLo() % 1000;

    wxString ver = xlights_version_string + xlights_qualifier;
    ver.Trim();
    for (int x = 0; x < ver.length(); x++) {
        if (ver[x] == ' ') ver[x] = '-';
    }

    wxString ts = wxString::Format("%04d-%02d-%02d_%02d-%02d-%02d-%03d", now.GetYear(), now.GetMonth()+1, now.GetDay(), now.GetHour(), now.GetMinute(), now.GetSecond(), millis);

    wxString qualifier = GetBitness();
#ifdef __WXOSX__
#if defined(__x86_64__)
    qualifier = "x86_64";
#elif defined(__aarch64__)
    qualifier = "arm64";
#endif
#endif
    wxString fn = wxString::Format("%s-%s_%s_%s_%s.zip", appName.c_str(), wxPlatformInfo::Get().GetOperatingSystemFamilyName().c_str(), ver, qualifier, ts);
    const char *ct = "Content-Type: application/octet-stream\n";
    std::string cd = "Content-Disposition: form-data; name=\"userfile\"; filename=\"" + fn.ToStdString() + "\"\n\n";

    wxMemoryBuffer memBuff;
    memBuff.AppendData(bound, strlen(bound));
    memBuff.AppendData("\n", 1);
    memBuff.AppendData(ct, strlen(ct));
    memBuff.AppendData(cd.c_str(), strlen(cd.c_str()));

    wxFile f_in(report.GetCompressedFileName());
    wxFileOffset fLen=f_in.Length();
    void* tmp=memBuff.GetAppendBuf(fLen);
    size_t iRead=f_in.Read(tmp, fLen);
    memBuff.UngetAppendBuf(iRead);
    f_in.Close();

    memBuff.AppendData("\n", 1);
    memBuff.AppendData(bound, strlen(bound));
    memBuff.AppendData("--\n", 3);

    http.SetMethod("POST");
    http.SetPostBuffer("multipart/form-data; boundary=------------------------b29a7c2fe47b9481", memBuff);
    wxInputStream* is = http.GetInputStream("/" + loc + "/index.php");
    char buf[1024];
    is->Read(buf, 1024);
    logger_base.debug("Sent debug log to server: %s", (const char*)fn.c_str());
    logger_base.debug("%s", (const char*) buf);
    //printf("%s\n", buf);
    delete is;
    http.Close();
}

void xlCrashHandler::SetupCrashHandlerForNonWxThread()
{
#ifdef __WXMSW__
    DisableAutomaticSETranslator();
#endif
}
