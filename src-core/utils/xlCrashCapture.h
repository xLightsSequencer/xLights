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

#include <string>

// Crash-time stack capture, wx-free so the iPad links it.
//
// The desktop builds its crash report from inside wx's fatal-exception hook and
// can afford to allocate and symbolicate there.  The iPad has no such hook: the
// process is gone before anything runs, and Apple's MetricKit payload only shows
// up a day later with a truncated stack.  InstallSignalHandlers() closes that by
// writing a minimal record at the moment of the fault, which the next launch
// finds and ships with the normal diagnostic upload.
namespace xlCrashCapture {

// Installs handlers for the fatal signals (SEGV/BUS/ILL/FPE/ABRT/TRAP) on an
// alternate stack, so a stack overflow is still catchable.  On a fault the
// handler appends an async-signal-safe record to `recordPath` - raw frame
// addresses plus the image slide needed to resolve them against the build's
// dSYM - then re-raises so Apple's own reporting and MetricKit still see the
// crash.  Idempotent; only the first call takes effect.
//
// Nothing here symbolicates: doing so would need malloc and dyld locks the
// crashing thread may already hold.  Resolution happens server-side against the
// dSYM already published for the build.
void InstallSignalHandlers(std::string const& recordPath,
                           std::string const& appVersion,
                           std::string const& build);

// True once InstallSignalHandlers() has taken effect.
bool HandlersInstalled();

// Desktop counterpart, for hosts that already have a crash reporter of their
// own (wx's fatal-exception hook).  Records the faulting signal, its si_code,
// the fault address and the interrupted PC/frame chain, then chains to whatever
// handler was installed before - so this adds data to the existing report
// rather than replacing the mechanism.  Call AFTER the host installs its
// handlers (wxHandleFatalExceptions).  Runs on an alternate stack, which is
// what makes a stack overflow reportable at all - though sigaltstack is
// per-thread, so that part only covers the thread this is called from (the
// main thread); a worker-thread fault is delivered on its own stack as before.
// Idempotent.
void InstallFaultInfoCapture();

// True when a fatal signal has been seen and its details recorded.  False for
// a crash that never came through a signal (a C++ exception, a Windows SEH).
bool FaultInfoCaptured();

// "signal / code / fault address / pc" block for the crash report, empty when
// nothing was captured.  The fault address is the single most useful field the
// report was missing: it separates a null/near-null dereference from a wild
// pointer, and an address that decodes to an input value names the corruption
// source outright.
std::string FaultInfoReport();

// Backtrace of the interrupted thread walked from the signal context, so it
// starts at the faulting instruction instead of inside the crash handler.
// Frames are printed in the image-offset form BuildAllThreadsReport uses (see
// the note there on why dladdr's symbol names are not trustworthy for our own
// stripped binary).  Reads the stack with vm_read_overwrite, so a corrupt frame
// chain ends the walk instead of faulting again.  Empty when no signal was
// captured.
std::string FaultBacktraceReport();

// Snapshot of every thread other than the caller, symbolicated.  Suspends each
// thread only long enough to read its registers and walk frame pointers, never
// while symbolicating.  NOT async-signal-safe - this is for a controlled crash
// moment (the desktop's fatal-exception hook) or a diagnostic dump, not for use
// from inside a signal handler.  Returns an empty string off Apple.
std::string BuildAllThreadsReport();

// Symbolicated backtrace of the calling thread.  Same safety caveat as above.
// Returns an empty string off Apple.
std::string BuildCurrentThreadBacktrace();

} // namespace xlCrashCapture
