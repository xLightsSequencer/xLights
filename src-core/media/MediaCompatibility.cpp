/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "MediaCompatibility.h"

#ifndef __APPLE__
// Non-Apple platforms — everything is "compatible" (uses FFmpeg)
std::string MediaCompatibility::CheckVideoFile(const std::string&) { return ""; }
std::string MediaCompatibility::CheckAudioFile(const std::string&) { return ""; }
#endif

std::vector<MediaCompatibilityIssue> MediaCompatibility::CheckSequenceMedia(
    const std::string& audioFile,
    const std::vector<std::string>& videoFiles)
{
    std::vector<MediaCompatibilityIssue> issues;

    if (!audioFile.empty()) {
        std::string reason = CheckAudioFile(audioFile);
        if (!reason.empty()) {
            issues.push_back({audioFile, reason, false});
        }
    }

    for (const auto& videoFile : videoFiles) {
        if (videoFile.empty()) continue;
        std::string reason = CheckVideoFile(videoFile);
        if (!reason.empty()) {
            issues.push_back({videoFile, reason, true});
        }
    }

    return issues;
}
