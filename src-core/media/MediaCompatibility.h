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

// Checks whether media files are compatible with AVFoundation/AudioToolbox
// (i.e., will work on iPad without FFmpeg).
// On non-Apple platforms, all files are reported as compatible (no check needed).

#include <string>
#include <vector>

struct MediaCompatibilityIssue {
    std::string filePath;
    std::string reason;     // e.g. "Unsupported video codec", "Cannot open audio file"
    bool isVideo;           // true = video, false = audio

    // Returns true if the file can be transcoded (i.e. it opened successfully).
    // Files that failed to open ("Cannot open file" / "Cannot read stream info")
    // cannot be converted — only warned about.
    bool canConvert() const {
        return reason.rfind("Cannot open", 0) != 0 &&
               reason.rfind("Cannot read", 0) != 0;
    }
};

class MediaCompatibility {
public:
    // Check if a single video file is compatible with AVFoundation.
    // Returns empty string if compatible, or a reason string if not.
    static std::string CheckVideoFile(const std::string& filePath);

    // Check if a single audio file is compatible with AudioToolbox.
    // Returns empty string if compatible, or a reason string if not.
    static std::string CheckAudioFile(const std::string& filePath);

    // Check all media in a sequence for iPad compatibility.
    // audioFile: the sequence's audio file path (empty if none)
    // videoFiles: list of video file paths from SequenceMedia
    // Returns list of issues found (empty = all compatible).
    static std::vector<MediaCompatibilityIssue> CheckSequenceMedia(
        const std::string& audioFile,
        const std::vector<std::string>& videoFiles);
};
