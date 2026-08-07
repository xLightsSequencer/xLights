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
#include <cstdint>
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

    // Change the output size without recreating the underlying file/decoder.
    // Returns true if the impl handled the resize; false means the caller should
    // delete and reconstruct. Default: not supported.
    virtual bool Resize(int /*width*/, int /*height*/) { return false; }

    // Opaque corridor identity for decoder chain affinity: readers serving
    // the same row/effect stream (e.g. frame-parallel clones) share a group,
    // letting the decoder keep one forward decode chain per corridor instead
    // of stealing chains across corridors. 0 = ungrouped. Default: ignored.
    virtual void SetStreamGroup(uint64_t /*group*/) {}

    // True when GetNextFrame(t) serves a frame that is a pure function of t and
    // the file — independent of this reader's current decode position, of the
    // order requests arrive in, and of other readers on the same file (the
    // AVFoundation bridge: shared per-file decoder + pts-indexed frame cache).
    // False for forward-decoding readers (FFmpeg), whose served frame can
    // depend on where the decoder currently sits. Callers use this to decide
    // whether frames may be requested out of order / from concurrent clones.
    virtual bool SupportsFrameIndependentAccess() const { return false; }
};
