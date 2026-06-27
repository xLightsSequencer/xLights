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
#ifdef __APPLE__
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <mach/thread_act.h>
#include <pthread.h>
#include <dlfcn.h>
#include <cstdio>
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

#ifdef __APPLE__
namespace {

// Per-thread frame addresses captured from a suspended thread. Resolved to
// symbols *after* the thread is resumed, so symbolication (which may take
// dyld/malloc-style locks) never runs while another thread is paused.
struct CapturedThread {
    thread_t machPort = MACH_PORT_NULL;
    pthread_t pthread = nullptr;
    char name[64] = {0};
    uint64_t frames[128];
    int frameCount = 0;
};

// Read 16 bytes (two pointer-sized words) from another thread's stack
// without crashing if the address is bad. vm_read_overwrite returns an
// error code instead of raising SIGBUS/SIGSEGV like a raw dereference
// would.
static bool safeReadPair(uint64_t addr, uint64_t outPair[2])
{
    mach_vm_size_t got = 0;
    kern_return_t kr = mach_vm_read_overwrite(
        mach_task_self(),
        (mach_vm_address_t)addr,
        sizeof(uint64_t) * 2,
        (mach_vm_address_t)outPair,
        &got);
    return kr == KERN_SUCCESS && got == sizeof(uint64_t) * 2;
}

// Walk the frame-pointer chain of a suspended thread starting from its
// current PC + FP. Apple uses frame pointers by default on both arm64
// and x86_64 release builds (xLights doesn't pass -fomit-frame-pointer),
// so this works without DWARF unwind tables. Stops on a zero LR, a
// non-monotonic FP (corrupt stack), or after kMaxFrames frames.
static int walkFrames(uint64_t pc, uint64_t fp, uint64_t* out, int maxFrames)
{
    int n = 0;
    if (pc) out[n++] = pc;
    uint64_t prev = 0;
    while (fp != 0 && n < maxFrames) {
        uint64_t pair[2];
        if (!safeReadPair(fp, pair)) break;
        uint64_t savedFP = pair[0];
        uint64_t savedLR = pair[1];
        if (savedLR == 0) break;
        out[n++] = savedLR;
        if (savedFP == 0 || savedFP <= fp || savedFP == prev) break;
        prev = fp;
        fp = savedFP;
    }
    return n;
}

// Collect every thread's frame addresses. Threads are visited one at a
// time, each one suspended only long enough to grab its registers and
// walk frame pointers — never while we're symbolicating or formatting,
// since either could take locks the suspended thread already holds.
// Returns true if any non-self threads were captured.
static int captureAllThreads(CapturedThread* out, int maxThreads, thread_t selfThread)
{
    thread_act_array_t threads = nullptr;
    mach_msg_type_number_t count = 0;
    if (task_threads(mach_task_self(), &threads, &count) != KERN_SUCCESS) return 0;

    int captured = 0;
    for (mach_msg_type_number_t i = 0; i < count && captured < maxThreads; ++i) {
        if (threads[i] == selfThread) {
            mach_port_deallocate(mach_task_self(), threads[i]);
            continue;
        }

        if (thread_suspend(threads[i]) != KERN_SUCCESS) {
            mach_port_deallocate(mach_task_self(), threads[i]);
            continue;
        }

        uint64_t pc = 0, fp = 0;
#if defined(__arm64__) || defined(__aarch64__)
        arm_thread_state64_t state;
        mach_msg_type_number_t stateCount = ARM_THREAD_STATE64_COUNT;
        if (thread_get_state(threads[i], ARM_THREAD_STATE64,
                             (thread_state_t)&state, &stateCount) == KERN_SUCCESS) {
            pc = (uint64_t)__darwin_arm_thread_state64_get_pc(state);
            fp = (uint64_t)__darwin_arm_thread_state64_get_fp(state);
        }
#elif defined(__x86_64__)
        x86_thread_state64_t state;
        mach_msg_type_number_t stateCount = x86_THREAD_STATE64_COUNT;
        if (thread_get_state(threads[i], x86_THREAD_STATE64,
                             (thread_state_t)&state, &stateCount) == KERN_SUCCESS) {
            pc = state.__rip;
            fp = state.__rbp;
        }
#endif

        CapturedThread& slot = out[captured];
        slot.machPort = threads[i];
        slot.frameCount = walkFrames(pc, fp, slot.frames,
                                     (int)(sizeof(slot.frames) / sizeof(slot.frames[0])));

        // pthread_from_mach_thread_np must be called *before* releasing the
        // mach port, but the thread can already be running again.
        thread_resume(threads[i]);

        slot.pthread = pthread_from_mach_thread_np(threads[i]);
        if (slot.pthread) {
            pthread_getname_np(slot.pthread, slot.name, sizeof(slot.name));
        }
        if (slot.name[0] == '\0') {
            std::snprintf(slot.name, sizeof(slot.name), "(unnamed)");
        }
        ++captured;
    }

    for (mach_msg_type_number_t i = 0; i < count; ++i) {
        // Ports for the threads we didn't keep, plus for the ones we kept
        // (we duplicate the port in CapturedThread.machPort but the array
        // hand-off was a single reference per port).
        // It's safe to deallocate all of them here since we no longer need
        // to call thread_* on the kept ones.
        if (threads[i] != selfThread) {
            mach_port_deallocate(mach_task_self(), threads[i]);
        }
    }
    vm_deallocate(mach_task_self(), (vm_address_t)threads,
                  count * sizeof(thread_t));
    return captured;
}

// Build the all-threads.txt body. Runs after the crash zip's normal
// backtrace.txt has already been added, so the format is free to differ
// from backtrace.txt — the server-side analyze.py reads backtrace.txt by
// default and may opt to also parse this file separately.
static wxString buildAllThreadsReport()
{
    thread_t self = mach_thread_self();
    constexpr int kMaxThreads = 96;
    CapturedThread threads[kMaxThreads];
    int n = captureAllThreads(threads, kMaxThreads, self);
    mach_port_deallocate(mach_task_self(), self);

    wxString out;
    out += wxString::Format("Threads captured: %d\n", n);
    out += "Note: backtrace.txt holds the crashing thread; this file is\n";
    out += "every other thread's stack at the moment of the crash.\n\n";

    for (int i = 0; i < n; ++i) {
        const CapturedThread& t = threads[i];
        out += wxString::Format("Thread %d \"%s\" (mach_port=0x%x, frames=%d):\n",
                                i, t.name, (unsigned)t.machPort, t.frameCount);
        for (int f = 0; f < t.frameCount; ++f) {
            Dl_info info{};
            const char* sym = "??";
            const char* image = "?";
            uint64_t offset = 0;
            if (dladdr((void*)t.frames[f], &info)) {
                if (info.dli_fname) {
                    // Use just the basename of the loaded image, matching
                    // backtrace_symbols() output style.
                    const char* slash = std::strrchr(info.dli_fname, '/');
                    image = slash ? slash + 1 : info.dli_fname;
                }
                if (info.dli_sname) {
                    sym = info.dli_sname;
                    offset = (uint64_t)t.frames[f] - (uint64_t)info.dli_saddr;
                } else if (info.dli_fbase) {
                    offset = (uint64_t)t.frames[f] - (uint64_t)info.dli_fbase;
                }
            }
            // Match the column layout of backtrace_symbols() so the
            // server-side parser can use the same FRAME_RE regex:
            //   <idx> <module> <addr> <sym> + <offset>
            out += wxString::Format("%-3d %-35s 0x%016" PRIx64 " %s + %" PRIu64 "\n",
                                    f, image, (uint64_t)t.frames[f], sym, offset);
        }
        out += "\n";
    }
    return out;
}

} // anonymous namespace
#endif // __APPLE__


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

#ifdef __APPLE__
            // Apple-only: snapshot every other thread's stack so races and
            // cross-thread state corruption are diagnosable from the crash
            // report. Goes in a separate file (all-threads.txt) so the
            // existing analyzer keeps reading backtrace.txt unchanged.
            try {
                wxString allThreads = buildAllThreadsReport();
                if (!allThreads.empty()) {
                    report.AddText("all-threads.txt", allThreads, "All threads backtrace");
                }
            } catch (...) {
                spdlog::critical("Exception while capturing all-thread backtraces.");
            }
#endif

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
