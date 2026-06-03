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

// AVFoundation (AVAssetWriter) implementation of VideoWriterImpl for
// Apple platforms. Encodes H.264 / HEVC / ProRes into .mp4/.mov with AAC
// audio. The actual AVAssetWriter / pixel-buffer-adaptor / CoreMedia
// machinery lives behind `AppleAVFoundationVideoWriterBridge` in
// `macOS/src-apple-core/media/AVFoundationVideoWriterBridge.{h,mm}`; this
// class is a thin VideoWriterImpl shell that drives the render/append
// loop and holds an opaque bridge handle.
//
// Export formats AVFoundation cannot encode (MPEG-4, AVI, rawvideo, ...)
// are routed to FFmpegVideoWriter instead — see VideoWriter.cpp.

#ifdef __APPLE__

#include "VideoWriterImpl.h"

#include <cstdint>
#include <vector>

namespace AppleAVFoundationVideoWriterBridge {
    struct WriterHandle;
}

class AVFoundationVideoWriter : public VideoWriterImpl {
public:
    AVFoundationVideoWriter(const std::string& outPath, const VideoWriterParams& params, bool videoOnly);
    ~AVFoundationVideoWriter() override;

    void initialize() override;
    void exportFrames(int videoFrameCount) override;
    void completeExport() override;

    bool usingAVFoundation() const override { return true; }

    // True when AVFoundation can encode the requested codec into the
    // container implied by outPath's extension. Used by VideoWriter.cpp
    // to decide between this backend and the FFmpeg fallback.
    static bool CanExport(const std::string& outPath, const VideoWriterParams& params);

private:
    AppleAVFoundationVideoWriterBridge::WriterHandle* _bridge = nullptr;
    std::vector<uint8_t> _cpuBuf;  // staging buffer for the CPU-frame (rgbBuffer) path
};

#endif // __APPLE__
