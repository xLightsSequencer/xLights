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
#include <string>

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
class MFReadSampleCallback;

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
    std::string _filename;

    IMFSourceReader* _reader = nullptr;
    IMFDXGIDeviceManager* _deviceManager = nullptr;
    ID3D11Device* _device = nullptr;

    // Reads are issued asynchronously and waited on with a deadline. The
    // synchronous form of ReadSample has no timeout, and a decoder that stops
    // producing samples parks the calling render thread in
    // WaitForSingleObject forever - with enough concurrent readers that is
    // every render thread, and the render never finishes.
    MFReadSampleCallback* _callback = nullptr;
    // Latched when hardware decode gives up on this file: a read past its
    // deadline, or a seek that fails or stops advancing. Under load Media
    // Foundation does both, and a reader that cannot seek would otherwise hand
    // the effect no video at all rather than saying so.
    bool _hardwareFailed = false;
    // Holds one of the limited hardware decoder slots. Handed to the cleanup
    // thread if this reader is abandoned, so the slot comes back only when the
    // decoder session actually does.
    bool _reservedDecoder = false;

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
    void HandleReadTimeout(uint32_t timestampMS);
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

    // True once hardware decode has given up on this file. The caller must stop
    // using this reader and decode the file some other way.
    bool HasFailed() const { return _hardwareFailed; }

    // True while hardware decode is stood down after a read passed its deadline
    // - not merely failed. Opens skip Media Foundation for the duration rather
    // than pay the deadline again per file. The window expires, and lengthens
    // each time it is re-triggered, because this is load-driven and recovers:
    // standing it down permanently would cost a batch render every sequence
    // after the first bad one.
    static bool MediaFoundationInCooldown();

    // Hardware decoder slots. How many the driver will actually service is not
    // reported by any API - D3D11 and Media Foundation describe formats and
    // profiles, never session counts - so the limit starts unlimited and is
    // lowered each time decode fails with several readers open.
    static bool TryReserveDecoder();
    static void ReleaseDecoder();
};
#endif
