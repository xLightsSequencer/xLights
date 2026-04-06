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

// Internal interface for VideoReader implementations.
// Not part of the public API — only included by VideoReader.cpp and concrete implementations.

#include "VideoFrame.h"
#include <string>

class VideoReaderImpl {
public:
    virtual ~VideoReaderImpl() = default;
    virtual void SetScaleAlgorithm(VideoScaleAlgorithm) {}
    virtual int GetLengthMS() const = 0;
    virtual void Seek(int timestampMS, bool readFrame) = 0;
    virtual VideoFrame* GetNextFrame(int timestampMS, int gracetime) = 0;
    virtual bool IsValid() const = 0;
    virtual int GetWidth() const = 0;
    virtual int GetHeight() const = 0;
    virtual bool AtEnd() const = 0;
    virtual int GetPos() = 0;
    virtual std::string GetFilename() const = 0;
    virtual int GetPixelChannels() const = 0;
};
