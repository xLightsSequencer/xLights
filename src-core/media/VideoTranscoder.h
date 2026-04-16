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

// Transcodes a video file to a MOV container decodable by AVFoundation.
// If the source is uncompressed (rawvideo and friends) the output uses
// rawvideo/rgb24 so the result is bit-exact with the source. Otherwise
// the output uses HEVC (hevc_videotoolbox on macOS) at near-visually-
// lossless quality to keep file size reasonable.
//
// Dimensions and frame rate are preserved from the source.

#include <functional>
#include <string>

class VideoTranscoder {
public:
    // Progress callback: (currentFrame, totalFrames). Return false to cancel.
    using ProgressCallback = std::function<bool(int currentFrame, int totalFrames)>;

    // Returns empty string on success, human-readable error message on failure.
    static std::string Transcode(const std::string& inputPath,
                                 const std::string& outputPath,
                                 ProgressCallback progress = nullptr);

    // Returns inputPath with its extension replaced by ".mov".
    static std::string SuggestedOutputPath(const std::string& inputPath);
};
