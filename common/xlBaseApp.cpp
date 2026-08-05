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
            std::unique_lock<std::timed_mutex> lock(m_crashMutex, std::defer_lock);
            if (!lock.try_lock_for(std::chrono::seconds(30))) {
                spdlog::critical("Another thread has been reporting a crash for over 30s - abandoning this report and aborting.");
                spdlog::default_logger()->flush();
                std::abort();
            }
            m_crashReportDone = false;

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

            try {
                wxString const activity = FormatActivityTrace();
                if (!activity.empty()) {
                    report.AddText("activity-trace.txt", activity, "Recent activity");
                }
            } catch (...) {
                spdlog::critical("Exception while formatting the activity trace.");
            }

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
                                                    [this] { return m_crashReportDone; })) {
                        spdlog::critical("Timed out waiting for the main thread to build the crash report - it is most likely not processing events. Writing the report here instead.");
                        if (report.Process()) {
                            spdlog::critical("Crash report saved to {}.", report.GetCompressedFileName().ToStdString());
                        } else {
                            spdlog::critical("Crash report could not be written.");
                        }
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
        return fmt::format("C-string exception: \"{}\".", eMsg ? eMsg : "(null)");
    }
    catch (std::string const& eMsg)
    {
        return fmt::format("std::string exception: \"{}\".", eMsg);
    }
    catch (std::filesystem::filesystem_error const& e)
    {
        // Common xLights crash source (throwing fs::exists/copy/etc. on sandbox/iCloud
        // edge cases). Capture the error code and both paths so the offending file shows.
        return fmt::format("Filesystem exception of type \"{}\": \"{}\" (error {}: {}) path1=\"{}\" path2=\"{}\".",
                           typeid(e), e.what(), e.code().value(), e.code().message(),
                           e.path1().string(), e.path2().string());
    }
    catch (std::system_error const& e)
    {
        return fmt::format("System exception of type \"{}\": \"{}\" (error {}: {}).",
                           typeid(e), e.what(), e.code().value(), e.code().message());
    }
    catch (std::exception const& e)
    {
        // typeid is demangled by fmt's std::type_info formatter, so macOS/Linux reports
        // read "std::length_error" rather than the raw "St12length_error".
        return fmt::format("Standard STD exception of type \"{}\" with message \"{}\".",
                           typeid(e), e.what());
    }
    catch (...)
    {
#ifdef __APPLE__
        // Cocoa/AVFoundation/Metal raise NSExceptions, which no C++ handler above
        // can match - they were reaching the reports as a bare "unknown exception".
        std::string objc = DescribeCurrentAppleException();
        if (!objc.empty())
        {
            return objc;
        }
#endif
#if defined(__GNUC__) || defined(__clang__)
        if (std::type_info const* t = abi::__cxa_current_exception_type())
        {
            return fmt::format("An exception of non-standard type \"{}\" occurred.", *t);
        }
#endif
        // Every exception the C++ or Objective-C runtime raises is named above, so
        // reaching here means the stack was unwound by neither - a forced unwind or
        // a foreign runtime.  current_exception() is null only in that case, and
        // saying so is the difference between a searchable report and a dead end:
        // there is no C++ throw site to go looking for.
        if (std::current_exception() == nullptr)
        {
            return "A foreign (non-C++) exception occurred - the stack was unwound by neither the C++ nor the Objective-C runtime, so no type information exists.";
        }
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
    

    // Whatever happens below - including an exception out of Process(), the
    // preview dialog or the upload - the crashing thread has to be released.  It
    // is sitting on m_crashDoneSignal, and stranding it there turns a crash into
    // a hung process.
    struct ReleaseWaiter {
        xlCrashHandler* self;
        ~ReleaseWaiter() {
            {
                std::lock_guard<std::timed_mutex> lg(self->m_crashMutex);
                self->m_crashReportDone = true;
            }
            self->m_crashDoneSignal.notify_all();
        }
    } releaseWaiter{ this };

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
