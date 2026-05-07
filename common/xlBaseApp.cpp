/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/
#include <iomanip>
#include <thread>
#include <inttypes.h>

#include <wx/buffer.h>
#include <wx/datetime.h>
#include <wx/dirdlg.h>
#include <wx/msgdlg.h>
#include <wx/protocol/http.h>
#include "settings/XLightsConfigAdapter.h"
#ifdef __WXMSW__
#include <wx/msw/crashrpt.h>
#include <wx/msw/seh.h>
#else
#include <execinfo.h>
#endif

#include <log.h>

#include "utils/ExternalHooks.h"
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
    if (!isFatalException) {
        spdlog::warn("Non fatal exception: {}", msg);
    } else {
        try {
#if defined(_DEBUG)
            // During debug, don't generate crash dumps. Instead, just bring debugger here.
            wxTrap();
#else
            // Protect against simultaneous crashes from different threads.
            std::unique_lock<std::mutex> lock(m_crashMutex);

            spdlog::critical("Crashed: " + msg);

            wxDebugReportCompress report;
            m_report = &report;

#ifdef __WXOSX__
            wxMessageBox("If you haven't already, please turn on the system settings to share crash data with the app developers.\n\n To do that, go to:\n"
                         "System Preferences -> Security and Privacy -> Privacy -> Analytics & Improvements\n\n"
                         "and turn on the \"Share Mac Analytics\" setting and also the \"Share with App Developers\" setting.\n\n"
                         "This provides more information to the xLights developers than just our normal crash logs.");
#endif

            wxString backtrace_txt = wxString::Format("%s version %s\n", m_appName.c_str(), GetDisplayVersionString());
            backtrace_txt += "Time: " + wxDateTime::Now().FormatISOCombined() + "\n";

            wxString userEmail;
            auto* config = GetXLightsConfig();
            if (config != nullptr) {
                config->Read("xLightsUserEmail", &userEmail, "noone@nowhere.xlights.org");

                if (userEmail != "noone@nowhere.xlights.org" && userEmail != "")
                    backtrace_txt += "<email>" + userEmail + "</email>\n";
            }

#if (wxUSE_STACKWALKER || wxUSE_CRASHREPORT)
            wxDebugReport::Context const ctx = isFatalException ? wxDebugReport::Context_Exception : wxDebugReport::Context_Current;
#endif

#if wxUSE_STACKWALKER
#ifdef __WXMSW__
            wxCrashContext c;
            backtrace_txt += wxString::Format("Context address 0x%016" PRIx64 "\n", (uint64_t)c.addr);
            backtrace_txt += "Exception: " + c.GetExceptionString() + "\n";
#endif
            report.AddContext(ctx);
#endif

#if wxUSE_CRASHREPORT
            wxFileName fn(report.GetDirectory(), report.GetReportName(), wxT("dmp"));
            wxCrashReport::SetFileName(fn.GetFullPath());

            int const crashRptFlags = wxCRASH_REPORT_LOCATION | wxCRASH_REPORT_STACK; // | wxCRASH_REPORT_GLOBALS; - remove globals to limit size of DMP file

            extern EXCEPTION_POINTERS* wxGlobalSEInformation;
            if (wxGlobalSEInformation != nullptr) {
                backtrace_txt += wxString::Format("Structured exception at 0x%016" PRIx64 "\n", (uint64_t)wxGlobalSEInformation->ExceptionRecord->ExceptionAddress);
            }

            if ((ctx == wxDebugReport::Context_Exception) ? wxCrashReport::Generate(crashRptFlags) : wxCrashReport::GenerateNow(crashRptFlags)) {
                report.AddFile(fn.GetFullName(), _("dump of the process state (binary)"));
            }
#endif

            std::ostringstream threadIdStr;
            threadIdStr << std::showbase // show the 0x prefix
                        << std::internal // fill between the prefix and the number
                        << std::setfill('0') << std::setw(10)
                        << std::hex << std::this_thread::get_id();

            backtrace_txt += wxString::Format("Crashed Thread ID: %s", threadIdStr.str());

            if (wxThread::IsMain()) {
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
            for (int i = 0; i < frames; ++i) {
                backtrace_txt += strs[i];
                backtrace_txt += "\n";
            }
            free(strs);
#endif

            report.AddText("backtrace.txt", backtrace_txt, "Backtrace");
            spdlog::critical("{}", backtrace_txt.ToStdString());

            std::string const logFilePath = GetLogFilePath().string();
            std::string const logFileName = GetLogFileName();
            xlFrame* const topFrame = GetTopWindow();
            if (FileExists(logFilePath)) {
                report.AddFile(logFilePath, logFileName);
            } else if ((topFrame != nullptr) && FileExists(wxFileName(topFrame->GetCurrentDir(), logFileName.c_str()).GetFullPath())) {
                report.AddFile(wxFileName(topFrame->GetCurrentDir(), logFileName.c_str()).GetFullPath(), logFileName.c_str());
            } else if (FileExists(wxFileName(wxGetCwd(), logFileName.c_str()).GetFullPath())) {
                report.AddFile(wxFileName(wxGetCwd(), logFileName.c_str()).GetFullPath(), logFileName.c_str());
            }

            if (topFrame == nullptr) {
                spdlog::critical("Unable to tell user about debug report. Crash report saved to {}.", report.GetCompressedFileName().ToStdString());
            } else {
                if (wxThread::IsMain()) {
                    topFrame->CreateDebugReport(this);
                } else {
                    topFrame->CallAfter(&xlFrame::CreateDebugReport, this);
                    m_crashDoneSignal.wait(lock);
                }
            }
#endif // (defined(_DEBUG))
        } catch (...) {
            spdlog::critical("We had an exception within the HandleCrash() function.");
        }
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

std::string xlCrashHandler::DescribeCurrentException()
{
    // Caller must be inside a catch handler; rethrow to recover the type.
    try
    {
        throw;
    }
    catch (char const* eMsg)
    {
        return std::string("C-string exception: \"") + (eMsg ? eMsg : "(null)") + "\".";
    }
    catch (std::exception& e)
    {
        std::string msg = "Standard STD exception of type \"";
        msg += typeid(e).name();
        msg += "\" with message \"";
        msg += e.what();
        msg += "\".";
        return msg;
    }
    catch (...)
    {
        return "An unknown exception occurred.";
    }
}

void xlCrashHandler::HandleUnhandledException()
{
    HandleCrash(true, DescribeCurrentException());
    wxAbort();
}

void xlCrashHandler::ProcessCrashReport(SendReportOptions sendOption)
{
    

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

    spdlog::critical("Created debug report: " + m_report->GetCompressedFileName().ToStdString());
    m_crashDoneSignal.notify_all();
}

void xlCrashHandler::SendReport(std::string const& appName, std::string const& loc, wxDebugReportCompress& report)
{
    

    wxHTTP http;
    http.Connect("dankulp.com");

    static char const bound[] = "--------------------------b29a7c2fe47b9481";
    wxDateTime now = wxDateTime::Now();
    int millis = wxGetUTCTimeMillis().GetLo() % 1000;

    wxString ver = xlights_version_string + xlights_qualifier;
    ver.Trim();
    for (int x = 0; x < (int)ver.length(); x++) {
        if (ver[x] == ' ') ver[x] = '-';
    }

    wxString ts = wxString::Format("%04d-%02d-%02d_%02d-%02d-%02d-%03d", now.GetYear(), now.GetMonth()+1, now.GetDay(), now.GetHour(), now.GetMinute(), now.GetSecond(), millis);

    wxString arch = wxEmptyString;
#ifdef __WXOSX__
    arch = wxPlatformInfo::Get().GetBitnessName();
#endif
    wxString fn;
    if (!arch.empty()) {
        fn = wxString::Format("%s-%s_%s_%s_%s.zip", appName.c_str(), wxPlatformInfo::Get().GetOperatingSystemFamilyName().c_str(), arch, ver, ts);
    } else {
        fn = wxString::Format("%s-%s_%s_%s.zip", appName.c_str(), wxPlatformInfo::Get().GetOperatingSystemFamilyName().c_str(), ver, ts);
    }
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
    spdlog::debug("Sent debug log to server: {}", fn.ToStdString());
    spdlog::debug("{}", (const char*) buf);
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
