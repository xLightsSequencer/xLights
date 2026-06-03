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

// Backend-abstracted video encoder. Mirrors the VideoReader split:
// VideoWriter -> VideoWriterImpl -> { FFmpegVideoWriter,
// AVFoundationVideoWriter }. The public header is intentionally wx-free
// AND ffmpeg-free so the iPad app (which has no FFmpeg) and any future
// non-wx host can encode video through it unchanged.
//
// The renderer supplies each frame through a GetVideoFrameCb that fills a
// VideoWriterFrame — either a CPU RGB24 buffer, or, on Apple backends, a
// pool-allocated CVPixelBuffer for a zero-copy GPU path. No FFmpeg
// AVFrame ever crosses this boundary.

#include <cstdint>
#include <functional>
#include <string>

// A frame the renderer is asked to fill. Exactly one of the two targets
// is active per call:
//   - nativeSurface == nullptr : write width*height*inputChannels bytes
//                                into rgbBuffer (CPU path; all non-Apple
//                                backends and the Apple software path).
//                                inputChannels is 3 (RGB24) or 4 (RGBA).
//   - nativeSurface != nullptr : a CVPixelBufferRef (as void*) the
//                                renderer fills directly, e.g. by
//                                rendering its Metal capture texture into
//                                it via CoreImage (Apple zero-copy path).
struct VideoWriterFrame {
    int width = 0;
    int height = 0;
    uint8_t* rgbBuffer = nullptr;
    int rgbBufferSize = 0;
    void* nativeSurface = nullptr;
};

struct VideoWriterParams {
    int width = 0;
    int height = 0;
    int fps = 0;                   // constant-rate input/output only
    int audioSampleRate = 0;       // 0 == no audio; assumes stereo otherwise
    std::string videoCodec;        // "H.264", "H.265", "MPEG-4", "ProRes", "rawvideo", "auto"
    int videoBitrate = 0;          // kbps; 0 lets the encoder choose / use constant quality
    bool lossless = false;         // request lossless / near-lossless encoding where the codec supports it
    int inputChannels = 3;         // bytes per pixel the GetVideoFrameCb fills into rgbBuffer (3=RGB24, 4=RGBA)
    bool cpuFrames = false;        // true: GetVideoFrameCb fills the CPU rgbBuffer (no GPU nativeSurface)
};

// Fill `frame` for the given output frame index. Return true if the CPU
// rgbBuffer was filled; return false if the native surface was filled
// directly (zero-copy) or the frame should be left as-is.
typedef std::function<bool(VideoWriterFrame& frame, unsigned frameIndex)> GetVideoFrameCb;

// Provide one encoder frame of stereo audio (planar float L/R).
typedef std::function<bool(float* leftCh, float* rightCh, int frameSize)> GetAudioFrameCb;

// Return true to abort the export.
typedef std::function<bool()> QueryForCancelCb;

// Report progress on a 0-100 scale.
typedef std::function<void(int)> ProgressReportCb;

class VideoWriterImpl;

class VideoWriter {
public:
    VideoWriter(const std::string& outPath, const VideoWriterParams& params, bool videoOnly = false);
    ~VideoWriter();

    void setGetVideoCallback(GetVideoFrameCb fn);
    void setGetAudioCallback(GetAudioFrameCb fn);
    void setQueryForCancelCallback(QueryForCancelCb fn);
    void setProgressReportCallback(ProgressReportCb fn);

    // Each may throw std::runtime_error on a fatal encoder/muxer error.
    void initialize();
    void exportFrames(int videoFrameCount);
    void completeExport();

    const VideoWriterParams& inputParams() const;
    const VideoWriterParams& outputParams() const;

    // True when the selected (active) backend is AVFoundation rather than
    // FFmpeg. Useful for logging / diagnostics.
    bool usingAVFoundation() const;

private:
    // Re-apply the stored callbacks to the current _impl (used after an
    // AVFoundation -> FFmpeg fallback swap).
    void applyCallbacks();

    std::string _outPath;
    VideoWriterParams _params;
    bool _videoOnly = false;

    GetVideoFrameCb _getVideo = nullptr;
    GetAudioFrameCb _getAudio = nullptr;
    QueryForCancelCb _queryForCancel = nullptr;
    ProgressReportCb _progressReporter = nullptr;

    VideoWriterImpl* _impl = nullptr;
};
