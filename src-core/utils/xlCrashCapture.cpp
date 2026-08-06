/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "xlCrashCapture.h"

#include <cstdio>
#include <cstring>
#include <inttypes.h>

#ifdef __APPLE__
#include <TargetConditionals.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <mach-o/dyld.h>
#include <mach/mach.h>
#include <mach/thread_act.h>
#if !TARGET_OS_IPHONE
// mach_vm.h is macOS-only - the iOS SDK ships it as an #error. Both platforms
// are 64-bit, so the plain vm_ variant is equivalent there.
#include <mach/mach_vm.h>
#endif
#include <pthread.h>
#include <signal.h>
// sys/ucontext.h rather than ucontext.h: the latter is the deprecated XSI
// variant and errors out unless _XOPEN_SOURCE is defined.
#include <sys/time.h>
#include <sys/ucontext.h>
#include <unistd.h>
#endif

namespace xlCrashCapture {

#ifdef __APPLE__

namespace {

constexpr int kMaxFramesPerThread = 128;

// Per-thread frame addresses captured from a suspended thread. Resolved to
// symbols *after* the thread is resumed, so symbolication (which may take
// dyld/malloc-style locks) never runs while another thread is paused.
struct CapturedThread {
    thread_t machPort = MACH_PORT_NULL;
    pthread_t pthread = nullptr;
    char name[64] = { 0 };
    uint64_t frames[kMaxFramesPerThread];
    int frameCount = 0;
};

// Read 16 bytes (two pointer-sized words) from another thread's stack
// without crashing if the address is bad. vm_read_overwrite returns an
// error code instead of raising SIGBUS/SIGSEGV like a raw dereference
// would.  Also the reason the signal handler can walk a corrupt stack
// without faulting a second time.
bool safeReadPair(uint64_t addr, uint64_t outPair[2])
{
#if TARGET_OS_IPHONE
    vm_size_t got = 0;
    kern_return_t kr = vm_read_overwrite(
        mach_task_self(),
        (vm_address_t)addr,
        sizeof(uint64_t) * 2,
        (vm_address_t)outPair,
        &got);
#else
    mach_vm_size_t got = 0;
    kern_return_t kr = mach_vm_read_overwrite(
        mach_task_self(),
        (mach_vm_address_t)addr,
        sizeof(uint64_t) * 2,
        (mach_vm_address_t)outPair,
        &got);
#endif
    return kr == KERN_SUCCESS && got == sizeof(uint64_t) * 2;
}

// Walk the frame-pointer chain of a suspended thread starting from its
// current PC + FP. Apple uses frame pointers by default on both arm64
// and x86_64 release builds (xLights doesn't pass -fomit-frame-pointer),
// so this works without DWARF unwind tables. Stops on a zero LR, a
// non-monotonic FP (corrupt stack), or after maxFrames frames.
int walkFrames(uint64_t pc, uint64_t fp, uint64_t* out, int maxFrames)
{
    int n = 0;
    if (pc)
        out[n++] = pc;
    uint64_t prev = 0;
    while (fp != 0 && n < maxFrames) {
        uint64_t pair[2];
        if (!safeReadPair(fp, pair))
            break;
        uint64_t savedFP = pair[0];
        uint64_t savedLR = pair[1];
        if (savedLR == 0)
            break;
        out[n++] = savedLR;
        if (savedFP == 0 || savedFP <= fp || savedFP == prev)
            break;
        prev = fp;
        fp = savedFP;
    }
    return n;
}

// Pull the instruction and frame pointer out of a thread state.  Shared by the
// all-threads walk (which reads it via thread_get_state) and the signal handler
// (which gets it from the ucontext the kernel hands over).
void pcAndFpFromState(_STRUCT_MCONTEXT64 const* mc, uint64_t& pc, uint64_t& fp)
{
    pc = 0;
    fp = 0;
    if (mc == nullptr)
        return;
#if defined(__arm64__) || defined(__aarch64__)
    pc = (uint64_t)__darwin_arm_thread_state64_get_pc(mc->__ss);
    fp = (uint64_t)__darwin_arm_thread_state64_get_fp(mc->__ss);
#elif defined(__x86_64__)
    pc = mc->__ss.__rip;
    fp = mc->__ss.__rbp;
#endif
}

// Collect every thread's frame addresses. Threads are visited one at a
// time, each one suspended only long enough to grab its registers and
// walk frame pointers - never while we're symbolicating or formatting,
// since either could take locks the suspended thread already holds.
int captureAllThreads(CapturedThread* out, int maxThreads, thread_t selfThread)
{
    thread_act_array_t threads = nullptr;
    mach_msg_type_number_t count = 0;
    if (task_threads(mach_task_self(), &threads, &count) != KERN_SUCCESS)
        return 0;

    int captured = 0;
    for (mach_msg_type_number_t i = 0; i < count && captured < maxThreads; ++i) {
        if (threads[i] == selfThread) {
            continue;
        }

        if (thread_suspend(threads[i]) != KERN_SUCCESS) {
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
        slot.frameCount = walkFrames(pc, fp, slot.frames, kMaxFramesPerThread);

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
        if (threads[i] != selfThread) {
            mach_port_deallocate(mach_task_self(), threads[i]);
        }
    }
    vm_deallocate(mach_task_self(), (vm_address_t)threads,
                  count * sizeof(thread_t));
    return captured;
}

// Format one frame the way backtrace_symbols() does, so the server-side parser
// can apply the same FRAME_RE regex to every file we produce.
void appendSymbolicatedFrame(std::string& out, int index, uint64_t addr)
{
    Dl_info info {};
    char const* sym = "??";
    char const* image = "?";
    uint64_t offset = 0;
    if (dladdr((void*)addr, &info)) {
        if (info.dli_fname) {
            char const* slash = std::strrchr(info.dli_fname, '/');
            image = slash ? slash + 1 : info.dli_fname;
        }
        if (info.dli_sname) {
            sym = info.dli_sname;
            offset = addr - (uint64_t)info.dli_saddr;
        } else if (info.dli_fbase) {
            offset = addr - (uint64_t)info.dli_fbase;
        }
    }
    char line[512];
    std::snprintf(line, sizeof(line), "%-3d %-35s 0x%016" PRIx64 " %s + %" PRIu64 "\n",
                  index, image, addr, sym, offset);
    out += line;
}

// ---- async-signal-safe state, all resolved before any crash ----------------
//
// Everything the handler touches is filled in by InstallSignalHandlers() and
// then only read.  No allocation, no snprintf, no dyld calls once a signal has
// landed: the crashing thread may already hold the malloc or dyld lock, and
// taking it again deadlocks the process instead of reporting it.

constexpr size_t kPathMax = 1024;
constexpr size_t kVersionMax = 64;

char g_recordPath[kPathMax] = { 0 };
char g_appVersion[kVersionMax] = { 0 };
char g_build[kVersionMax] = { 0 };
uint64_t g_imageSlide = 0;
uint64_t g_imageBase = 0;
bool g_installed = false;
volatile sig_atomic_t g_handlingCrash = 0;

int const kFatalSignals[] = { SIGSEGV, SIGBUS, SIGILL, SIGFPE, SIGABRT, SIGTRAP };
constexpr int kFatalSignalCount = (int)(sizeof(kFatalSignals) / sizeof(kFatalSignals[0]));
struct sigaction g_previous[kFatalSignalCount];

void safeWrite(int fd, char const* s, size_t len)
{
    while (len > 0) {
        ssize_t n = write(fd, s, len);
        if (n <= 0)
            return;
        s += n;
        len -= (size_t)n;
    }
}

void safeWriteStr(int fd, char const* s)
{
    if (s == nullptr)
        return;
    safeWrite(fd, s, std::strlen(s));
}

// Hand-rolled because snprintf is not async-signal-safe.
void safeWriteHex(int fd, uint64_t v)
{
    char buf[19];
    buf[0] = '0';
    buf[1] = 'x';
    for (int i = 0; i < 16; ++i) {
        int nibble = (int)((v >> ((15 - i) * 4)) & 0xF);
        buf[2 + i] = (char)(nibble < 10 ? ('0' + nibble) : ('a' + nibble - 10));
    }
    safeWrite(fd, buf, sizeof(buf));
}

void safeWriteDec(int fd, long long v)
{
    if (v < 0) {
        safeWrite(fd, "-", 1);
        v = -v;
    }
    char buf[24];
    int i = (int)sizeof(buf);
    if (v == 0) {
        buf[--i] = '0';
    }
    while (v > 0 && i > 0) {
        buf[--i] = (char)('0' + (v % 10));
        v /= 10;
    }
    safeWrite(fd, buf + i, sizeof(buf) - (size_t)i);
}

char const* signalName(int sig)
{
    switch (sig) {
    case SIGSEGV:
        return "SIGSEGV";
    case SIGBUS:
        return "SIGBUS";
    case SIGILL:
        return "SIGILL";
    case SIGFPE:
        return "SIGFPE";
    case SIGABRT:
        return "SIGABRT";
    case SIGTRAP:
        return "SIGTRAP";
    default:
        return "SIG?";
    }
}

void restoreDefaultHandlers()
{
    for (int i = 0; i < kFatalSignalCount; ++i) {
        sigaction(kFatalSignals[i], &g_previous[i], nullptr);
    }
}

void fatalSignalHandler(int sig, siginfo_t* info, void* uap)
{
    // A fault inside the handler must not come back through here.  Reporting
    // has already failed at that point; restore the default disposition and let
    // the process die rather than loop.
    if (g_handlingCrash) {
        restoreDefaultHandlers();
        raise(sig);
        return;
    }
    g_handlingCrash = 1;

    int fd = open(g_recordPath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd >= 0) {
        safeWriteStr(fd, "xLights crash record\nversion: ");
        safeWriteStr(fd, g_appVersion);
        safeWriteStr(fd, "\nbuild: ");
        safeWriteStr(fd, g_build);
        safeWriteStr(fd, "\nsignal: ");
        safeWriteStr(fd, signalName(sig));
        safeWriteStr(fd, " (");
        safeWriteDec(fd, sig);
        safeWriteStr(fd, ")\ncode: ");
        safeWriteDec(fd, info ? info->si_code : 0);
        safeWriteStr(fd, "\nfault_address: ");
        safeWriteHex(fd, info ? (uint64_t)info->si_addr : 0);

        // Without these the frame addresses are unresolvable: atos needs the
        // slide to map a runtime address back to the dSYM.
        safeWriteStr(fd, "\nimage_base: ");
        safeWriteHex(fd, g_imageBase);
        safeWriteStr(fd, "\nimage_slide: ");
        safeWriteHex(fd, g_imageSlide);

        safeWriteStr(fd, "\ntime: ");
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        safeWriteDec(fd, (long long)tv.tv_sec);

        safeWriteStr(fd, "\nthread: ");
        safeWriteHex(fd, (uint64_t)(uintptr_t)pthread_self());
        char tname[64] = { 0 };
        if (pthread_getname_np(pthread_self(), tname, sizeof(tname)) == 0 && tname[0] != '\0') {
            safeWriteStr(fd, " \"");
            safeWriteStr(fd, tname);
            safeWriteStr(fd, "\"");
        }
        if (pthread_main_np() != 0) {
            safeWriteStr(fd, " (main)");
        }

        // Frames come from the ucontext the kernel handed us, so the trace
        // starts at the faulting instruction rather than inside this handler.
        uint64_t pc = 0, fp = 0;
        if (uap != nullptr) {
            ucontext_t const* uc = (ucontext_t const*)uap;
            pcAndFpFromState(uc->uc_mcontext, pc, fp);
        }
        uint64_t frames[kMaxFramesPerThread];
        int n = walkFrames(pc, fp, frames, kMaxFramesPerThread);

        safeWriteStr(fd, "\nframes: ");
        safeWriteDec(fd, n);
        safeWriteStr(fd, "\n");
        for (int i = 0; i < n; ++i) {
            safeWriteDec(fd, i);
            safeWriteStr(fd, " ");
            safeWriteHex(fd, frames[i]);
            safeWriteStr(fd, "\n");
        }
        safeWriteStr(fd, "end\n");
        close(fd);
    }

    // Hand back to the previous disposition so Apple's own crash reporting and
    // MetricKit still record this.  This adds a channel; it must not take one
    // away.
    restoreDefaultHandlers();
    raise(sig);
}

} // namespace

void InstallSignalHandlers(std::string const& recordPath,
                           std::string const& appVersion,
                           std::string const& build)
{
    if (g_installed) {
        return;
    }

    std::strncpy(g_recordPath, recordPath.c_str(), kPathMax - 1);
    std::strncpy(g_appVersion, appVersion.c_str(), kVersionMax - 1);
    std::strncpy(g_build, build.c_str(), kVersionMax - 1);

    // Resolved now because _dyld_* walks dyld's own structures and is not safe
    // to call once a fault has landed.
    g_imageSlide = (uint64_t)_dyld_get_image_vmaddr_slide(0);
    g_imageBase = (uint64_t)(uintptr_t)_dyld_get_image_header(0);

    // A stack overflow faults with the stack already exhausted, so the handler
    // needs a stack of its own or it cannot run at all.
    static char altStack[SIGSTKSZ * 4];
    stack_t ss {};
    ss.ss_sp = altStack;
    ss.ss_size = sizeof(altStack);
    ss.ss_flags = 0;
    sigaltstack(&ss, nullptr);

    struct sigaction sa {};
    sa.sa_sigaction = fatalSignalHandler;
    sa.sa_flags = SA_SIGINFO | SA_ONSTACK;
    sigemptyset(&sa.sa_mask);

    for (int i = 0; i < kFatalSignalCount; ++i) {
        sigaction(kFatalSignals[i], &sa, &g_previous[i]);
    }

    g_installed = true;
}

bool HandlersInstalled()
{
    return g_installed;
}

std::string BuildAllThreadsReport()
{
    thread_t self = mach_thread_self();
    constexpr int kMaxThreads = 96;
    static CapturedThread threads[kMaxThreads];
    int n = captureAllThreads(threads, kMaxThreads, self);
    mach_port_deallocate(mach_task_self(), self);

    std::string out;
    char header[192];
    std::snprintf(header, sizeof(header), "Threads captured: %d\n", n);
    out += header;
    out += "Note: backtrace.txt holds the crashing thread; this file is\n";
    out += "every other thread's stack at the moment of the crash.\n\n";

    for (int i = 0; i < n; ++i) {
        CapturedThread const& t = threads[i];
        char line[256];
        std::snprintf(line, sizeof(line), "Thread %d \"%s\" (mach_port=0x%x, frames=%d):\n",
                      i, t.name, (unsigned)t.machPort, t.frameCount);
        out += line;
        for (int f = 0; f < t.frameCount; ++f) {
            appendSymbolicatedFrame(out, f, t.frames[f]);
        }
        out += "\n";
    }
    return out;
}

std::string BuildCurrentThreadBacktrace()
{
    uint64_t frames[kMaxFramesPerThread];
    // __builtin_frame_address gives this frame; the walk starts from our caller
    // so BuildCurrentThreadBacktrace itself isn't the leaf.
    uint64_t fp = (uint64_t)(uintptr_t)__builtin_frame_address(0);
    int n = walkFrames(0, fp, frames, kMaxFramesPerThread);

    std::string out;
    for (int i = 0; i < n; ++i) {
        appendSymbolicatedFrame(out, i, frames[i]);
    }
    return out;
}

#else // !__APPLE__

void InstallSignalHandlers(std::string const&, std::string const&, std::string const&)
{
}

bool HandlersInstalled()
{
    return false;
}

std::string BuildAllThreadsReport()
{
    return std::string();
}

std::string BuildCurrentThreadBacktrace()
{
    return std::string();
}

#endif // __APPLE__

} // namespace xlCrashCapture
