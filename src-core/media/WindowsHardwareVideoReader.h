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

#include <mutex>

#ifdef _WIN32

#ifdef _DEBUG
//#define D3D_DEBUG_INFO
#endif

// Must be defined before windows.h (via d3d11.h) to prevent winsock.h being
// included, which conflicts with winsock2.h included by other translation units.
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif

#include <d3d11.h>
#include <D2d1.h>
#include <D2d1helper.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

extern "C" {
#include <libavcodec/avcodec.h>
}

struct AVFrame;
class WVHRStatic;

class WindowsHardwareVideoReader
{
    static WVHRStatic _init;
    uint32_t _nativeWidth = 0;
    uint32_t _nativeHeight = 0;
    uint32_t _width = 0;
    uint32_t _height = 0;
    uint32_t _duration = 0;
    uint32_t _frameMS = 0;
    uint32_t _curPos = 0;
    uint32_t _stride = 0;
    bool _wantAlpha = false;
    AVFrame* _frame = nullptr;
    AVPixelFormat _pixelFormat = AV_PIX_FMT_RGB24;

    IMFSourceReader* _reader = nullptr;
    IMFDXGIDeviceManager* _deviceManager = nullptr;
    ID3D11Device* _device = nullptr;

    // Direct D3D11 video-processor path.
    //
    // MF_SOURCE_READER_ENABLE_ADVANCED_VIDEO_PROCESSING hands YUV->RGB and the
    // downscale to a processor we cannot configure: it ignores every colour
    // attribute offered on the media types, and scales in a colour space that
    // measured ~18% brighter than swscale. Driving ID3D11VideoProcessor
    // ourselves is the same GPU work with the colour space stated explicitly.
    // MF is then used for decode only, which is what FFmpeg's d3d11va hwaccel
    // does too.
    bool _useVideoProcessor = false;
    ID3D11DeviceContext* _immediateContext = nullptr;
    ID3D11VideoDevice* _videoDevice = nullptr;
    ID3D11VideoContext* _videoContext = nullptr;
    ID3D11VideoProcessorEnumerator* _vpEnum = nullptr;
    ID3D11VideoProcessor* _videoProcessor = nullptr;
    ID3D11Texture2D* _vpOutput = nullptr;              // BGRA, target size, render target
    ID3D11VideoProcessorOutputView* _vpOutputView = nullptr;
    ID3D11Texture2D* _staging = nullptr;               // BGRA, target size, CPU readable

    template<class T>
    void SafeRelease(T** ppT)
    {
        if (*ppT) {
            (*ppT)->Release();
            *ppT = nullptr;
        }
    }
    HRESULT SelectVideoStream(bool usenativeresolution, bool keepaspectratio);
    bool CanSeek() const;
    bool BitmapFromSample(IMFSample* sample, AVFrame* frame);
    bool InitVideoProcessor(DXGI_FORMAT inputFormat);
    bool BltFromSample(IMFSample* sample);
    void ReleaseVideoProcessor();
    uint8_t GetPixelBytes() const;
    std::string DecodeDXGIReason(HRESULT reason) const;
    std::string DecodeReadFlags(DWORD flags) const;

public:
    WindowsHardwareVideoReader(const std::string& filename, bool wantAlpha, bool usenativeresolution, bool keepaspectratio, uint32_t maxwidth, uint32_t maxheight, AVPixelFormat pixelFormat);
    virtual ~WindowsHardwareVideoReader();
    bool IsOk() const;
    uint32_t GetFrames() const
    {
        if (_frameMS == 0)
            return 0;
        return GetDuration() / _frameMS;
    }
    uint32_t GetFrameMS() const
    {
        return _frameMS;
    }
    uint32_t GetWidth() const
    {
        return _width;
    }
    uint32_t GetHeight() const
    {
        return _height;
    }
    uint32_t GetNativeWidth() const
    {
        return _nativeWidth;
    }
    uint32_t GetNativeHeight() const
    {
        return _nativeHeight;
    }
    uint32_t GetDuration() const
    {
        return _duration;
    }
    bool Seek(uint32_t pos);
    uint32_t GetPos() const
    {
        return _curPos;
    }
    AVFrame* GetNextFrame(uint32_t timestampMS, uint32_t gracetime);
    static std::string DecodeMFError(HRESULT hr);
};
#endif
