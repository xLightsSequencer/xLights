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

// FFmpeg (libav*) implementation of VideoWriterImpl. This is the
// universal fallback backend — the only one on Linux/Windows, and the
// macOS fallback for export formats AVFoundation can't encode (MPEG-4,
// AVI, rawvideo, ...). Carries the H.264/H.265/MPEG-4 codec selection,
// the Apple VideoToolbox-encoder fast path, the Windows NVENC/AMF/QSV
// preferences, sws full-range color conversion, and AAC audio muxing.

#include "VideoWriterImpl.h"

#include <cstdarg>

extern "C" {
struct AVCodec;
struct AVCodecContext;
struct AVFormatContext;
struct AVFrame;
struct AVPacket;
struct SwsContext;
struct AVBufferRef;
}

#define MAX_EXPORT_BUFFER_FRAMES 20

// Uncomment this to turn on avlib debug logging
//#define VIDEOWRITE_DEBUG

// Log messages from libav*. Kept here (rather than private) because the
// direct-FFmpeg FSEQ->video pipeline in TabConvert.cpp installs it too.
void my_av_log_callback(void* ptr, int level, const char* fmt, va_list vargs);

class FFmpegVideoWriter : public VideoWriterImpl {
public:
    FFmpegVideoWriter(const std::string& outPath, const VideoWriterParams& params, bool videoOnly);
    ~FFmpegVideoWriter() override;

    void initialize() override;
    void exportFrames(int videoFrameCount) override;
    void completeExport() override;

protected:
    bool initializeVideo(const AVCodec* codec);
    void initializeAudio(const AVCodec* codec);
    void initializeFrames();
    void initializePackets();

    int pushVideoUntilPacketFilled(int startFrameIndex);
    void pushAudioUntilPacketFilled();

    void cleanup();

    int _inPfmt = 0;   // AVPixelFormat — renderer-supplied RGB24
    int _outPfmt = 0;  // AVPixelFormat — encoder input (YUV420P)
    int64_t _ptsIncrement = 0LL;
    SwsContext* _swsContext = nullptr;
    AVFormatContext* _formatContext = nullptr;
    AVCodecContext* _videoCodecContext = nullptr;
    AVCodecContext* _audioCodecContext = nullptr;
    AVFrame* _colorConversionFrame = nullptr;
    AVFrame* _videoFrames[MAX_EXPORT_BUFFER_FRAMES];
    AVFrame* _audioFrame = nullptr;
    AVPacket* _videoPacket = nullptr;
    AVPacket* _audioPacket = nullptr;
    uint32_t _curVideoFrame = 0;
    int64_t _curPts = 0LL;

    // Optional hardware acceleration context (used for VideoToolbox on Apple).
    // When populated, _videoCodecContext->pix_fmt is AV_PIX_FMT_VIDEOTOOLBOX and
    // each AVFrame's data[3] is a pool-allocated CVPixelBufferRef, which skips
    // the CPU-side sws_scale conversion entirely.
    AVBufferRef* _hwDeviceCtx = nullptr;
    AVBufferRef* _hwFramesCtx = nullptr;
};
