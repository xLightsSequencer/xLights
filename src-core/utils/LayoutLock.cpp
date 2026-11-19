/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "LayoutLock.h"

#include <spdlog/spdlog.h>

bool LayoutLock::Refuses(const LayoutLock* lock, const char* what) {
    if (lock == nullptr || !lock->IsLocked()) {
        return false;
    }
    spdlog::warn("Layout is locked ({}): refusing {}", lock->GetReason(), what);
    return true;
}
