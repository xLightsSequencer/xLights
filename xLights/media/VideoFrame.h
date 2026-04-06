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

#include <cstdint>

enum class VideoScaleAlgorithm {
    Default,   // Platform-optimal (VTPixelTransferSession on Apple, SWS_BICUBIC on FFmpeg)
    Bicubic,   // High quality, good balance of sharpness and smoothness
    Lanczos,   // Highest quality, sharpest
    Area,      // Good for downscaling, averages source pixels
    Point      // Nearest-neighbor, fastest, pixelated
};

enum class VideoPixelFormat {
    RGB24,
    BGR24,
    RGBA,
    BGRA,
    PlatformNative // Opaque GPU-side buffer (CVPixelBufferRef on Apple, etc.)
};

struct VideoFrame {
    uint8_t* data = nullptr;       // Pointer to pixel data (null for PlatformNative)
    void* nativeHandle = nullptr;  // Platform-specific handle (CVPixelBufferRef on Apple)
    int linesize = 0;              // Bytes per row (stride)
    int width = 0;
    int height = 0;
    VideoPixelFormat format = VideoPixelFormat::RGB24;
};
