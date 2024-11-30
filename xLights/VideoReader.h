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

#include <wx/wx.h>
#include <string>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#ifdef __WXMSW__
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

class VideoReader
{
public:
    static bool IsVideoFile(const std::string &filename);
    static long GetVideoLength(const std::string& filename);
	VideoReader(const std::string& filename, int width, int height, bool keepaspectratio, bool usenativeresolution = false,
                bool wantAlpha = false, bool bgr = false, bool wantsHardwareDecoderType = false);
	~VideoReader();
	int GetLengthMS() const { return (int)_lengthMS; };
	void Seek(int timestampMS, bool readFrame = true);
	AVFrame* GetNextFrame(int timestampMS, int gracetime = 0); // grace time is the minimum the video must be ahead before we bother to seek back to a frame
	bool IsValid() const { return _valid; };
	int GetWidth() const { return _width; };
	int GetHeight() const { return _height; };
	bool AtEnd() const { return _atEnd; };
    int GetPos();
    std::string GetFilename() const { return _filename; }
    int GetPixelChannels() const { return _wantAlpha ? 4 : 3; }
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
    void *hwDecoderCache = nullptr;
	int _streamIndex = 0;
	int _width = 0;
	int _height = 0;
	AVFrame* _dstFrame = nullptr; // the last frame
    AVFrame* _dstFrame2 = nullptr; // the second to the last frame
    AVFrame* _srcFrame = nullptr; // the src frame
    AVFrame* _srcFrame2 = nullptr; // the src frame
    int _curPos = 0;
    int _firstFramePos = -1;
    SwsContext *_swsCtx = nullptr;
    AVPacket* _packet = nullptr;
	AVPixelFormat _pixelFmt;
	bool _atEnd = false;
    std::string _filename;
    bool _abort = false;
    bool _videoToolboxAccelerated; 
    bool _abandonHardwareDecode = false;
    #ifdef __WXMSW__
    WindowsHardwareVideoReader* _windowsHardwareVideoReader = nullptr;
    #endif
};
