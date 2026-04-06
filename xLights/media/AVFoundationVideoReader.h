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

// AVFoundation-based video reader for Apple platforms (macOS 13+ / iPadOS 26+).
// Uses AVAssetReader for decoding and VTPixelTransferSession for hardware-accelerated scaling.
// No FFmpeg dependency.

#ifdef __APPLE__

#include "VideoReaderImpl.h"
#include <string>

// Opaque pointer to ObjC implementation
struct AVFoundationVideoReaderImpl;

class AVFoundationVideoReader : public VideoReaderImpl {
public:
    AVFoundationVideoReader(const std::string& filename, int maxwidth, int maxheight, bool keepaspectratio,
                            bool usenativeresolution, bool wantAlpha, bool bgr, bool wantsHWType);
    ~AVFoundationVideoReader() override;

    int GetLengthMS() const override;
    void Seek(int timestampMS, bool readFrame) override;
    VideoFrame* GetNextFrame(int timestampMS, int gracetime) override;
    bool IsValid() const override;
    int GetWidth() const override;
    int GetHeight() const override;
    bool AtEnd() const override;
    int GetPos() override;
    std::string GetFilename() const override;
    int GetPixelChannels() const override;

    static long GetVideoLengthStatic(const std::string& filename);

private:
    AVFoundationVideoReaderImpl* _impl;
};

#endif // __APPLE__
