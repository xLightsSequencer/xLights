/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "AppCallbacks.h"

#include <cstdlib>

namespace AppCallbacks {

static std::function<void(std::function<void()>)> s_postToMainThread;
static std::function<void()> s_handleUnhandledException;
static std::function<void()> s_setupThreadCrashHandler;

void PostToMainThread(std::function<void()> fn) {
    if (s_postToMainThread) {
        s_postToMainThread(std::move(fn));
    } else {
        fn();
    }
}

void SetPostToMainThread(std::function<void(std::function<void()>)> handler) {
    s_postToMainThread = std::move(handler);
}

void HandleUnhandledException() {
    if (s_handleUnhandledException) {
        s_handleUnhandledException();
    } else {
        std::abort();
    }
}

void SetHandleUnhandledException(std::function<void()> handler) {
    s_handleUnhandledException = std::move(handler);
}

void SetupThreadCrashHandler() {
    if (s_setupThreadCrashHandler) {
        s_setupThreadCrashHandler();
    }
}

void SetSetupThreadCrashHandler(std::function<void()> handler) {
    s_setupThreadCrashHandler = std::move(handler);
}

} // namespace AppCallbacks
