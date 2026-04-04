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

namespace xlThreadUtils {
    inline std::thread::id& mainThreadId() {
        static std::thread::id id;
        return id;
    }
}

// Call once from the main thread during app init to record the main thread ID.
inline void SetMainThreadId() {
    xlThreadUtils::mainThreadId() = std::this_thread::get_id();
}

// Returns true if called from the main thread.
inline bool IsMainThread() {
    return std::this_thread::get_id() == xlThreadUtils::mainThreadId();
}
