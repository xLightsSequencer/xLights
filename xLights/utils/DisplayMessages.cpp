/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "DisplayMessages.h"

#include <log.h>
#include <mutex>

static std::mutex s_callbackMutex;
static DisplayMessageCallback s_displayCallback;

void RegisterDisplayCallback(DisplayMessageCallback cb) {
    std::lock_guard<std::mutex> lock(s_callbackMutex);
    s_displayCallback = std::move(cb);
}

void DisplayError(const std::string& err) {
    spdlog::error("DisplayError: {}", err);
    DisplayMessageCallback cb;
    {
        std::lock_guard<std::mutex> lock(s_callbackMutex);
        cb = s_displayCallback;
    }
    if (cb) {
        cb(DisplayMessageLevel::Error, err);
    }
}

void DisplayWarning(const std::string& warn) {
    spdlog::warn("DisplayWarning: {}", warn);
    DisplayMessageCallback cb;
    {
        std::lock_guard<std::mutex> lock(s_callbackMutex);
        cb = s_displayCallback;
    }
    if (cb) {
        cb(DisplayMessageLevel::Warning, warn);
    }
}

void DisplayInfo(const std::string& info) {
    spdlog::info("DisplayInfo: {}", info);
    DisplayMessageCallback cb;
    {
        std::lock_guard<std::mutex> lock(s_callbackMutex);
        cb = s_displayCallback;
    }
    if (cb) {
        cb(DisplayMessageLevel::Info, info);
    }
}
