/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/
#include <atomic>
#include <chrono>
#include <cstdio>
#include <exception>
#include <iomanip>
#include <thread>
#include <inttypes.h>
#include <filesystem>
#include <system_error>
#if defined(__GNUC__) || defined(__clang__)
#include <cxxabi.h>
#endif

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
#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/std.h> // fmt formatters for std::type_info (demangled), std::error_code, std::filesystem::path

#include "utils/ExternalHooks.h"
#include "xLightsVersion.h"
#include "xlBaseApp.h"
#include "xlStackWalker.h"
#include "utils/xlCrashCapture.h"
#include "utils/xlExceptionDescribe.h"
#include "utils/ShowGuid.h"
#include "utils/UtilFunctions.h"

#include <wx/platinfo.h>

namespace {

// Ring buffer of recent activity.  Written on every dispatched event, so it
// must stay allocation-free and lock-free; a torn entry read during a crash is
// an acceptable trade for not perturbing the very timing we are trying to
// observe.
struct ActivityEntry {
    uint64_t seq = 0;
    uint32_t millis = 0;
    int eventType = 0;
    int id = 0;
    wxChar const* eventClass = nullptr; // static storage from wxClassInfo
    char const* note = nullptr;         // static storage from the caller
    uint32_t repeat = 0;                // extra consecutive occurrences
    char detail[48] = { 0 };
};

constexpr size_t ACTIVITY_SLOTS = 256;
ActivityEntry g_activity[ACTIVITY_SLOTS];
std::atomic<uint64_t> g_activitySeq{ 0 };
std::chrono::steady_clock::time_point g_activityStart = std::chrono::steady_clock::now();

uint32_t ActivityMillis()
{
    return (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now() - g_activityStart).count();
}

ActivityEntry& NextActivitySlot(uint64_t& seqOut)
{
    seqOut = g_activitySeq.fetch_add(1, std::memory_order_relaxed) + 1;
    return g_activity[(seqOut - 1) % ACTIVITY_SLOTS];
}

class xlActivityTraceFilter : public wxEventFilter
{
public:
    int FilterEvent(wxEvent& event) override
    {
        wxEventType const type = event.GetEventType();
        int const id = event.GetId();

        // Idle and update-UI are pure bookkeeping and arrive in the thousands,
        // alternating with each other so they never coalesce.  Recorded, they
        // evict everything worth reading within a second of the app sitting
        // still - a crash after any quiet period would show nothing else.
        if (type == wxEVT_IDLE || type == wxEVT_UPDATE_UI) {
            return Event_Skip;
        }

        // Runs of the same event (repaints, mouse motion) collapse to a count
        // so the buffer's reach is measured in distinct actions.
        uint64_t const cur = g_activitySeq.load(std::memory_order_relaxed);
        if (cur > 0) {
            ActivityEntry& prev = g_activity[(cur - 1) % ACTIVITY_SLOTS];
            if (prev.seq == cur && prev.note == nullptr && prev.eventType == (int)type && prev.id == id) {
                ++prev.repeat;
                prev.millis = ActivityMillis();
                return Event_Skip;
            }
        }

        uint64_t seq = 0;
        ActivityEntry& e = NextActivitySlot(seq);
        e.millis = ActivityMillis();
        e.eventType = (int)type;
        e.id = id;
        wxClassInfo const* ci = event.GetClassInfo();
        e.eventClass = (ci != nullptr) ? ci->GetClassName() : nullptr;
        e.note = nullptr;
        e.repeat = 0;
        e.detail[0] = '\0';
        // Published last: a reader that sees the sequence number can assume the
        // rest of the entry is filled in.
        e.seq = seq;
        return Event_Skip; // never alters dispatch
    }
};

xlActivityTraceFilter g_activityFilter;
bool g_activityFilterAdded = false;

} // namespace

void xlCrashHandler::StartActivityTrace()
{
    if (!g_activityFilterAdded) {
        wxEvtHandler::AddFilter(&g_activityFilter);
        g_activityFilterAdded = true;
    }
}

void xlCrashHandler::StopActivityTrace()
{
    if (g_activityFilterAdded) {
        wxEvtHandler::RemoveFilter(&g_activityFilter);
        g_activityFilterAdded = false;
    }
}

void xlCrashHandler::TraceNote(char const* text, std::string const& detail)
{
    uint64_t seq = 0;
    ActivityEntry& e = NextActivitySlot(seq);
    e.millis = ActivityMillis();
    e.eventType = 0;
    e.id = 0;
    e.eventClass = nullptr;
    e.note = text;
    e.repeat = 0;
    std::snprintf(e.detail, sizeof(e.detail), "%s", detail.c_str());
    e.seq = seq;
}

wxString xlCrashHandler::FormatActivityTrace()
{
    uint64_t const total = g_activitySeq.load(std::memory_order_relaxed);
    if (total == 0) {
        return wxEmptyString;
    }

    wxString out;
    out += "Most recent activity, oldest first. 'evt' lines are dispatched wx\n";
    out += "events (class, wx event type id, control/menu id); 'note' lines are\n";
    out += "explicit markers left by the app. Times are ms since startup.\n\n";
    out += wxString::Format("Total recorded: %llu (showing last %llu)\n\n",
                            (unsigned long long)total,
                            (unsigned long long)std::min<uint64_t>(total, ACTIVITY_SLOTS));

    uint64_t const first = (total > ACTIVITY_SLOTS) ? (total - ACTIVITY_SLOTS) : 0;
    for (uint64_t s = first; s < total; ++s) {
        ActivityEntry const& e = g_activity[s % ACTIVITY_SLOTS];
        if (e.seq != s + 1) {
            continue; // overwritten while we were reading it
        }
        if (e.note != nullptr) {
            out += wxString::Format("%8u ms  note  %s%s%s\n", e.millis, e.note,
                                    (e.detail[0] != '\0') ? " " : "", e.detail);
        } else {
            wxString rep;
            if (e.repeat > 0) {
                rep = wxString::Format("  x%u", e.repeat + 1);
            }
            out += wxString::Format("%8u ms  evt   %-22s type=%d id=%d%s\n", e.millis,
                                    (e.eventClass != nullptr) ? wxString(e.eventClass) : wxString("?"),
                                    e.eventType, e.id, rep);
        }
    }
    return out;
}

xlCrashHandler::xlCrashHandler(std::string const& appName) :
    m_appName(appName),
    m_crashMutex(),
    m_crashDoneSignal(),
    m_report(nullptr)
{
#if wxUSE_ON_FATAL_EXCEPTION
    wxHandleFatalExceptions();
#endif
    // After wx, so the capture chains into wx's hook rather than replacing it.
    // wx's hook gives us no siginfo at all, which left every signal crash
    // reported without a fault address - the one field that says whether a
    // pointer was null, freed or wild.
    xlCrashCapture::InstallFaultInfoCapture();
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
            // A crash INSIDE the crash handler must not come back through here.
            // The mutex below is not recursive, so re-entering on the same thread
            // blocks on a lock this thread already holds and the process wedges
            // at zero CPU - which is what a user reports as a hung render rather
            // than as a crash.  Reporting has already failed at that point; the
            // useful thing left is to die promptly and let the log stand.
            static thread_local bool handlingCrash = false;
            if (handlingCrash) {
                spdlog::critical("Crashed again while reporting a crash - abandoning the report and aborting.");
                spdlog::default_logger()->flush();
                std::abort();
            }
            handlingCrash = true;
            struct ClearHandling {
                bool& flag;
                ~ClearHandling() { flag = false; }
            } clearHandling{ handlingCrash };

            // Protect against simultaneous crashes from different threads, but do
            // not wait on it indefinitely: if the holder is stuck mid-report,
            // blocking here just converts a second crash into a second hang.
            std::unique_lock<std::recursive_timed_mutex> lock(m_crashMutex, std::defer_lock);
            if (!lock.try_lock_for(std::chrono::seconds(30))) {
                spdlog::critical("Another thread has been reporting a crash for over 30s - abandoning this report and aborting.");
                spdlog::default_logger()->flush();
                std::abort();
            }
            m_crashReportDone = false;
            m_crashReportStarted = false;

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

            // Appended after the existing trace, never in front of it: the
            // server-side analyzer buckets on what backtrace.txt already
            // starts with, and re-shaping the head would re-bucket every
            // signature.
            if (xlCrashCapture::FaultInfoCaptured()) {
                backtrace_txt += "\n--- Fault details (from the signal, not from this handler) ---\n";
                backtrace_txt += xlCrashCapture::FaultInfoReport();

                wxString const faultFrames = xlCrashCapture::FaultBacktraceReport();
                if (!faultFrames.empty()) {
                    // The trace above starts inside this handler and reaches the
                    // fault through the signal trampoline, which hides a
                    // mismatched frame. This one is walked from the interrupted
                    // registers, and names our own frames by image offset
                    // instead of by the nearest exported symbol - a stripped
                    // binary makes that name point at an unrelated function.
                    backtrace_txt += "\nFaulting thread, walked from the signal context:\n";
                    backtrace_txt += faultFrames;
                }
            }

            report.AddText("backtrace.txt", backtrace_txt, "Backtrace");
            spdlog::critical("{}", backtrace_txt.ToStdString());

#ifdef __APPLE__
            // Apple-only: snapshot every other thread's stack so races and
            // cross-thread state corruption are diagnosable from the crash
            // report. Goes in a separate file (all-threads.txt) so the
            // existing analyzer keeps reading backtrace.txt unchanged.
            try {
                wxString allThreads = xlCrashCapture::BuildAllThreadsReport();
                if (!allThreads.empty()) {
                    report.AddText("all-threads.txt", allThreads, "All threads backtrace");
                }
            } catch (...) {
                spdlog::critical("Exception while capturing all-thread backtraces.");
            }
#endif

            try {
                wxString const activity = FormatActivityTrace();
                if (!activity.empty()) {
                    report.AddText("activity-trace.txt", activity, "Recent activity");
                }
            } catch (...) {
                spdlog::critical("Exception while formatting the activity trace.");
            }

            AddSessionMetadata(report, isFatalException, msg);

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
                // No UI to ask through - headless, or the frame is already gone.
                // Write the report anyway: Process() is what actually produces
                // the file, so skipping it left nothing on disk and logged an
                // empty path, which is the worst of both worlds.
                if (report.Process()) {
                    spdlog::critical("No UI to report through. Crash report saved to {}.", report.GetCompressedFileName().ToStdString());
                } else {
                    spdlog::critical("No UI to report through, and the crash report could not be written.");
                }
            } else {
                if (wxThread::IsMain()) {
                    topFrame->CreateDebugReport(this);
                } else {
                    // The report is built on the main thread, so this only
                    // completes if the main thread is pumping events.  During a
                    // render it frequently is not - it can be sat waiting on the
                    // render itself - and an unbounded wait here then wedges the
                    // process at zero CPU forever, which users see as a hung
                    // render rather than as a crash.  Bound it, and carry on
                    // regardless so the process can finish dying.
                    topFrame->CallAfter(&xlFrame::CreateDebugReport, this);
                    if (!m_crashDoneSignal.wait_for(lock, std::chrono::seconds(60),
                                                    [this] { return m_crashReportStarted || m_crashReportDone; })) {
                        spdlog::critical("Timed out waiting for the main thread to build the crash report - it is most likely not processing events. Writing the report here instead.");
                        if (report.Process()) {
                            spdlog::critical("Crash report saved to {}.", report.GetCompressedFileName().ToStdString());
                        } else {
                            spdlog::critical("Crash report could not be written.");
                        }
                    } else if (!m_crashReportDone) {
                        // The main thread has the report and may be showing the
                        // preview dialog, which the user can sit on for as long
                        // as they like.  Returning now would abort the process
                        // and destroy the report while it is still in use.
                        m_crashDoneSignal.wait(lock, [this] { return m_crashReportDone; });
                    }
                }
            }
#endif // (defined(_DEBUG))
        } catch (...) {
            spdlog::critical("We had an exception within the HandleCrash() function.");
        }
    }

    m_report = nullptr;
}

namespace {
// Minimal JSON string escape - the values here are version strings, paths and
// GUIDs, but a show folder name can legitimately contain a quote or backslash.
std::string JsonEscape(std::string const& in) {
    std::string out;
    out.reserve(in.size() + 8);
    for (char c : in) {
        switch (c) {
        case '"':  out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default:
            if ((unsigned char)c < 0x20) {
                out += fmt::format("\\u{:04x}", (int)(unsigned char)c);
            } else {
                out += c;
            }
        }
    }
    return out;
}

// A stable token this code chooses, never a display name the toolkit chooses.
// The filename carries wxPlatformInfo's operating-system family name, which
// silently changed from "Unix" to "Linux" under us and dropped every Linux
// report on the floor until the server-side pattern was updated to match.
constexpr char const* PlatformToken() {
#if defined(__WXOSX__) || defined(__APPLE__)
    return "macos";
#elif defined(_WIN32)
    return "windows";
#elif defined(__linux__)
    return "linux";
#else
    return "unknown";
#endif
}
} // namespace

void xlCrashHandler::AddSessionMetadata(wxDebugReportCompress& report, bool isFatalException, std::string const& msg)
{
    try {
        // The banner, as a point-in-time snapshot. Recovering it from the log
        // instead means scanning a rolling file that can hold weeks of startup
        // banners, with no way to tell which one describes the crashing run -
        // and it may have rotated away entirely.
        std::string const machineConfig = GetMachineConfigText();
        if (!machineConfig.empty()) {
            report.AddText("machine_config.txt", wxString::FromUTF8(machineConfig), "Machine configuration");
        }
    } catch (...) {
        spdlog::critical("Exception while attaching the machine configuration.");
    }

    try {
        // Everything the consumer would otherwise have to recover by parsing
        // the upload filename, which encodes it only incidentally and in a
        // shape the toolkit is free to change.
        wxDateTime const nowLocal = wxDateTime::Now();
        int const millis = wxGetUTCTimeMillis().GetLo() % 1000;
        m_uploadFileName = BuildUploadFileName(m_appName, nowLocal, millis);

        std::string showGuid;
        if (xlFrame* const topFrame = GetTopWindow(); topFrame != nullptr) {
            std::string const dir = topFrame->GetCurrentDir();
            if (!dir.empty()) {
                showGuid = ShowGuid::ReadFromShowFolder(dir);
            }
        }

        std::string sessionType = "crash";
        if (isFatalException) {
            sessionType = "unhandled-exception";
        } else if (msg.rfind("Assert:", 0) == 0) {
            sessionType = "assert";
        }

        std::string json = "{\n";
        auto add = [&json](char const* key, std::string const& value, bool quoted = true) {
            json += fmt::format("  \"{}\": {}{}{},\n", key, quoted ? "\"" : "", quoted ? JsonEscape(value) : value, quoted ? "\"" : "");
        };
        add("schema", "1", false);
        add("app", m_appName);
        add("platform", PlatformToken());
        add("version", xlights_version_string);
        add("qualifier", wxString(xlights_qualifier).Trim().Trim(false).ToStdString());
        add("build_date", xlights_build_date);
        add("arch", wxGetCpuArchitectureName().ToStdString());
        add("bitness", wxPlatformInfo::Get().GetBitnessName().ToStdString());
        add("os_family", wxPlatformInfo::Get().GetOperatingSystemFamilyName().ToStdString());
        add("app_store", IsFromAppStore() ? "true" : "false", false);
        add("session_type", sessionType);
        add("show_guid", showGuid);
        add("timestamp_utc", nowLocal.ToUTC().FormatISOCombined('T').ToStdString() + "Z");
        add("timestamp_local", nowLocal.FormatISOCombined('T').ToStdString());
        // Last, so it carries no trailing comma.
        json += fmt::format("  \"report_file\": \"{}\"\n}}\n", JsonEscape(m_uploadFileName));

        report.AddText("report.json", wxString::FromUTF8(json), "Report metadata");
    } catch (...) {
        spdlog::critical("Exception while attaching the report metadata.");
    }
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
    // The describing itself lives in core so worker threads, the headless
    // render and the iPad all report an exception identically.
    return ::DescribeCurrentException();
}

void xlCrashHandler::HandleUnhandledException()
{
    HandleCrash(true, DescribeCurrentException());
    wxAbort();
}

void xlCrashHandler::ProcessCrashReport(SendReportOptions sendOption)
{
    // Tell the crashing thread we have the report before anything that could
    // throw or block, so its bounded wait turns into an unbounded one and it
    // does not tear the report down while we are still using it.
    {
        std::lock_guard<std::recursive_timed_mutex> lg(m_crashMutex);
        m_crashReportStarted = true;
    }
    m_crashDoneSignal.notify_all();

    // Whatever happens below - including an exception out of Process(), the
    // preview dialog or the upload - the crashing thread has to be released.  It
    // is sitting on m_crashDoneSignal, and stranding it there turns a crash into
    // a hung process.
    struct ReleaseWaiter {
        xlCrashHandler* self;
        ~ReleaseWaiter() {
            {
                std::lock_guard<std::recursive_timed_mutex> lg(self->m_crashMutex);
                self->m_crashReportDone = true;
            }
            self->m_crashDoneSignal.notify_all();
        }
    } releaseWaiter{ this };

    if ((sendOption == SendReportOptions::ALWAYS_SEND) || ((sendOption == SendReportOptions::ASK_USER_TO_SEND) && wxDebugReportPreviewStd().Show(*m_report)))
    {
        m_report->Process();
        SendReport(m_appName, "crashUpload", *m_report, m_uploadFileName);
        wxMessageBox("Crash report saved to " + m_report->GetCompressedFileName());
    }
    else
    {
        m_report->Process();
    }

    spdlog::critical("Created debug report: " + m_report->GetCompressedFileName().ToStdString());
}

std::string xlCrashHandler::BuildUploadFileName(std::string const& appName, wxDateTime const& when, int millis)
{
    wxString ver = xlights_version_string + xlights_qualifier;
    ver.Trim();
    for (int x = 0; x < (int)ver.length(); x++) {
        if (ver[x] == ' ') ver[x] = '-';
    }

    wxString ts = wxString::Format("%04d-%02d-%02d_%02d-%02d-%02d-%03d", when.GetYear(), when.GetMonth() + 1, when.GetDay(), when.GetHour(), when.GetMinute(), when.GetSecond(), millis);

    wxString arch = wxEmptyString;
#ifdef __WXOSX__
    arch = wxPlatformInfo::Get().GetBitnessName();
#endif
    if (!arch.empty()) {
        return wxString::Format("%s-%s_%s_%s_%s.zip", appName.c_str(), wxPlatformInfo::Get().GetOperatingSystemFamilyName().c_str(), arch, ver, ts).ToStdString();
    }
    return wxString::Format("%s-%s_%s_%s.zip", appName.c_str(), wxPlatformInfo::Get().GetOperatingSystemFamilyName().c_str(), ver, ts).ToStdString();
}

void xlCrashHandler::SendReport(std::string const& appName, std::string const& loc, wxDebugReportCompress& report,
                                std::string const& fileName)
{
    wxHTTP http;
    http.Connect("dankulp.com");

    static char const bound[] = "--------------------------b29a7c2fe47b9481";

    wxString fn = fileName;
    if (fn.empty()) {
        fn = BuildUploadFileName(appName, wxDateTime::Now(), wxGetUTCTimeMillis().GetLo() % 1000);
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
