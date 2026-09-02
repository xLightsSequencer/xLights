/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "xlDisplayInfo.h"

#import <AppKit/AppKit.h>
#import <CoreGraphics/CoreGraphics.h>

#include <cstdio>

std::vector<std::string> GetDisplayRefreshInfo(const std::vector<xlDisplayQuery>& displays) {
    std::vector<std::string> result(displays.size());
    @autoreleasepool {
        // CGGetActiveDisplayList is the order wxDisplayImplMacOSX enumerates in,
        // so index i here is the caller's display i. Each one's NSScreen is then
        // found by CGDirectDisplayID - matching by position in [NSScreen screens]
        // would be a guess, this is not.
        uint32_t count = 0;
        if (CGGetActiveDisplayList(0, nullptr, &count) != kCGErrorSuccess || count == 0) {
            return result;
        }
        std::vector<CGDirectDisplayID> ids(count);
        if (CGGetActiveDisplayList(count, ids.data(), &count) != kCGErrorSuccess) {
            return result;
        }
        for (uint32_t i = 0; i < count && i < result.size(); i++) {
            NSScreen* match = nil;
            for (NSScreen* s in [NSScreen screens]) {
                NSNumber* num = s.deviceDescription[@"NSScreenNumber"];
                if (num != nil && (CGDirectDisplayID)num.unsignedIntValue == ids[i]) {
                    match = s;
                    break;
                }
            }
            if (match == nil) {
                continue;
            }
            // The names read backwards because these are intervals, not rates:
            // minimumRefreshInterval is the FASTEST the panel can present.
            double fastest = match.minimumRefreshInterval > 0 ? 1.0 / match.minimumRefreshInterval : 0.0;
            double slowest = match.maximumRefreshInterval > 0 ? 1.0 / match.maximumRefreshInterval : 0.0;
            if (fastest <= 0.0) {
                continue;
            }
            char buf[96];
            int fast = (int)(fastest + 0.5);
            int slow = (int)(slowest + 0.5);
            if (slowest > 0.0 && fast != slow) {
                snprintf(buf, sizeof(buf), "%dHz variable %d-%dHz", fast, slow, fast);
            } else {
                snprintf(buf, sizeof(buf), "%dHz", fast);
            }
            result[i] = buf;
        }
    }
    return result;
}

std::string GetPresentCapabilityDescription() {
    return "";
}
