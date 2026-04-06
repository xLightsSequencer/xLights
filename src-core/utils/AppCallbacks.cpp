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
#include <mutex>

#include <log.h>

namespace AppCallbacks {

static std::function<void(std::function<void()>)> s_postToMainThread;
static std::function<void()> s_handleUnhandledException;
static std::function<void()> s_setupThreadCrashHandler;

static std::mutex s_displayMutex;
static DisplayMessageCallback s_displayCallback;

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

void SetDisplayMessageCallback(DisplayMessageCallback cb) {
    std::lock_guard<std::mutex> lock(s_displayMutex);
    s_displayCallback = std::move(cb);
}

static void DoDisplayMessage(DisplayMessageLevel level, const std::string& msg) {
    switch (level) {
    case DisplayMessageLevel::Error:
        spdlog::error("DisplayError: {}", msg);
        break;
    case DisplayMessageLevel::Warning:
        spdlog::warn("DisplayWarning: {}", msg);
        break;
    case DisplayMessageLevel::Info:
        spdlog::info("DisplayInfo: {}", msg);
        break;
    }
    DisplayMessageCallback cb;
    {
        std::lock_guard<std::mutex> lock(s_displayMutex);
        cb = s_displayCallback;
    }
    if (cb) {
        cb(level, msg);
    }
}

void DisplayError(const std::string& err) {
    DoDisplayMessage(DisplayMessageLevel::Error, err);
}

void DisplayWarning(const std::string& warn) {
    DoDisplayMessage(DisplayMessageLevel::Warning, warn);
}

void DisplayInfo(const std::string& info) {
    DoDisplayMessage(DisplayMessageLevel::Info, info);
}

} // namespace AppCallbacks
