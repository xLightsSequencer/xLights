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

#include "VideoReaderImpl.h"

#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#ifdef _WIN32
class WindowsHardwareVideoReader;
#endif

enum class WINHARDWARERENDERTYPE : int {
    DIRECX11_API = 0,
    FFMPEG_AUTO,
    FFMPEG_CUDA,
    FFMPEG_QSV,
    FFMPEG_VULKAN,
    FFMPEG_AMF,
    FFMPEG_D3D11VA
};

class FFmpegVideoReader : public VideoReaderImpl {
public:
    FFmpegVideoReader(const std::string& filename, int maxwidth, int maxheight, bool keepaspectratio,
                      bool usenativeresolution, bool wantAlpha, bool bgr, bool wantsHWType);
    ~FFmpegVideoReader() override;

    void SetScaleAlgorithm(VideoScaleAlgorithm algorithm) override;
    int GetLengthMS() const override { return (int)_lengthMS; }
    void Seek(int timestampMS, bool readFrame) override;
    VideoFrame* GetNextFrame(int timestampMS, int gracetime) override;
    bool IsValid() const override { return _valid; }
    int GetWidth() const override { return _width; }
    int GetHeight() const override { return _height; }
    bool AtEnd() const override { return _atEnd; }
    int GetPos() override { return _curPos; }
    std::string GetFilename() const override { return _filename; }
    int GetPixelChannels() const override { return _wantAlpha ? 4 : 3; }

    // Static helpers used by VideoReader
    static long GetVideoLengthStatic(const std::string& filename);
    static void SetHardwareAcceleratedVideo(bool accel);
    static void SetHardwareRenderType(int type);
    static bool IsHardwareAcceleratedVideo() { return HW_ACCELERATION_ENABLED; }
    static int GetHardwareRenderType() { return static_cast<std::underlying_type_t<WINHARDWARERENDERTYPE>>(HW_ACCELERATION_TYPE); }
    static void InitHWAcceleration();

private:
    static bool HW_ACCELERATION_ENABLED;
    static WINHARDWARERENDERTYPE HW_ACCELERATION_TYPE;

    bool readFrame(int timestampMS);
    void reopenContext(bool allowHWDecoder = true);

    bool _wantsHWType = false;
    int _maxwidth = 0;
    int _maxheight = 0;
    bool _valid = false;
    double _lengthMS = 0;
    double _dtspersec = 0;
    long _frames = 0;
    int _frameMS = 50;
    int _keyFrameCount = 20;
    bool _wantAlpha = false;
    AVFormatContext* _formatContext = nullptr;
    AVCodecContext* _codecContext = nullptr;
    AVStream* _videoStream = nullptr;
    const AVCodec* _decoder = nullptr;
    AVBufferRef* _hw_device_ctx = nullptr;
    void* hwDecoderCache = nullptr;
    int _streamIndex = 0;
    int _width = 0;
    int _height = 0;
    AVFrame* _dstFrame = nullptr;
    AVFrame* _dstFrame2 = nullptr;
    AVFrame* _srcFrame = nullptr;
    AVFrame* _srcFrame2 = nullptr;
    int _curPos = 0;
    int _firstFramePos = -1;
    SwsContext* _swsCtx = nullptr;
    AVPacket* _packet = nullptr;
    AVPixelFormat _pixelFmt;
    bool _atEnd = false;
    std::string _filename;
    bool _abort = false;
    bool _videoToolboxAccelerated;
    VideoScaleAlgorithm _scaleAlgorithm = VideoScaleAlgorithm::Default;
    bool _abandonHardwareDecode = false;
    #ifdef _WIN32
    WindowsHardwareVideoReader* _windowsHardwareVideoReader = nullptr;
    #endif

    // The VideoFrame returned to callers, populated from _dstFrame
    VideoFrame _videoFrame;
};
