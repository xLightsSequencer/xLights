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

#include <thread>

#if defined(__APPLE__)
#include <pthread.h>
#elif defined(__linux__)
#include <sys/syscall.h>
#include <unistd.h>
#endif

namespace xlThreadUtils {
    inline std::thread::id& mainThreadId() {
        static std::thread::id id;
        return id;
    }

    // Whatever the platform itself can tell us, for hosts that never registered.
    inline bool platformIsMainThread() {
#if defined(__APPLE__)
        return pthread_main_np() != 0;
#elif defined(__linux__)
        return syscall(SYS_gettid) == getpid();
#else
        return false;
#endif
    }
}

// Call once from the main thread during app init to record the main thread ID.
inline void SetMainThreadId() {
    xlThreadUtils::mainThreadId() = std::this_thread::get_id();
}

// Returns true if called from the main thread.
inline bool IsMainThread() {
    const std::thread::id& main = xlThreadUtils::mainThreadId();
    // A default-constructed id matches no running thread, so a host that never
    // called SetMainThreadId would answer "not the main thread" from every
    // thread - silently disabling the main-thread-only guards in core rather
    // than failing visibly. Only the wx app registers; the iPad app and the
    // headless context do not.
    if (main == std::thread::id()) {
        return xlThreadUtils::platformIsMainThread();
    }
    return std::this_thread::get_id() == main;
}
