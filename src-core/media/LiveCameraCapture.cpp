/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "LiveCameraCapture.h"

#ifdef _WIN32

#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <strmif.h> // IAMCameraControl / IAMVideoProcAmp (supported by the MF capture source for UVC cameras)

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <thread>

#include <log.h>

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

namespace {

std::wstring Utf8ToWide(const std::string& s)
{
    if (s.empty()) return std::wstring();
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
    std::wstring w(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), w.data(), len);
    return w;
}

std::string WideToUtf8(const wchar_t* w)
{
    if (w == nullptr) return std::string();
    int len = WideCharToMultiByte(CP_UTF8, 0, w, -1, nullptr, 0, nullptr, nullptr);
    if (len <= 0) return std::string();
    std::string s(len - 1, 0); // exclude the null terminator
    WideCharToMultiByte(CP_UTF8, 0, w, -1, s.data(), len, nullptr, nullptr);
    return s;
}

struct ComScope {
    bool needUninit = false;
    ComScope()
    {
        HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        needUninit = SUCCEEDED(hr) && hr != S_FALSE;
    }
    ~ComScope()
    {
        if (needUninit) CoUninitialize();
    }
};

} // namespace

std::vector<LiveCameraDevice> EnumerateLiveCameras()
{
    std::vector<LiveCameraDevice> result;

    ComScope com;
    if (FAILED(MFStartup(MF_VERSION, MFSTARTUP_LITE))) return result;

    IMFAttributes* attributes = nullptr;
    if (SUCCEEDED(MFCreateAttributes(&attributes, 1))) {
        if (SUCCEEDED(attributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID))) {
            IMFActivate** devices = nullptr;
            UINT32 count = 0;
            if (SUCCEEDED(MFEnumDeviceSources(attributes, &devices, &count))) {
                for (UINT32 i = 0; i < count; ++i) {
                    WCHAR* name = nullptr;
                    UINT32 nameLen = 0;
                    WCHAR* link = nullptr;
                    UINT32 linkLen = 0;
                    bool haveName = SUCCEEDED(devices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &name, &nameLen));
                    bool haveLink = SUCCEEDED(devices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, &link, &linkLen));
                    if (haveName && haveLink) {
                        LiveCameraDevice dev;
                        dev.name = WideToUtf8(name);
                        dev.symbolicLink = WideToUtf8(link);
                        result.push_back(dev);
                    }
                    if (name != nullptr) CoTaskMemFree(name);
                    if (link != nullptr) CoTaskMemFree(link);
                    devices[i]->Release();
                }
                CoTaskMemFree(devices);
            }
        }
        attributes->Release();
    }

    MFShutdown();
    return result;
}

struct LiveCameraCapture::Impl {
    std::string symbolicLink;
    std::thread captureThread;
    std::atomic<bool> stopRequested{ false };
    std::atomic<bool> valid{ false };
    std::atomic<int> darkModeRequest{ -1 }; // -1 = nothing pending, 0 = restore camera defaults, 1 = apply dark mode
    int width = 0;
    int height = 0;

    mutable std::mutex latestMutex;
    std::vector<uint8_t> latestRgb;
    int latestWidth = 0;
    int latestHeight = 0;
    bool hasLatest = false;
    uint64_t generation = 0;
    mutable uint64_t lastReadGeneration = ~(uint64_t)0;

    void CaptureLoop()
    {
        ComScope com;

        if (FAILED(MFStartup(MF_VERSION, MFSTARTUP_LITE))) {
            spdlog::warn("LiveCameraCapture: MFStartup failed.");
            return;
        }

        IMFAttributes* srcAttr = nullptr;
        IMFMediaSource* source = nullptr;
        IMFSourceReader* reader = nullptr;

        HRESULT hr = MFCreateAttributes(&srcAttr, 2);
        if (SUCCEEDED(hr)) hr = srcAttr->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
        if (SUCCEEDED(hr)) {
            std::wstring wlink = Utf8ToWide(symbolicLink);
            hr = srcAttr->SetString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, wlink.c_str());
        }
        if (SUCCEEDED(hr)) hr = MFCreateDeviceSource(srcAttr, &source);

        // Most webcams natively deliver MJPEG or YUY2/NV12, not RGB. Without
        // MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING the source reader will only wire
        // up a plain decoder (compressed -> raw) and refuses to also colour-convert,
        // so SetCurrentMediaType(..., RGB32) below fails with MF_E_INVALIDMEDIATYPE.
        // This attribute lets it insert the Video Processor MFT to do that
        // conversion for us.
        if (SUCCEEDED(hr)) {
            IMFAttributes* readerAttr = nullptr;
            if (SUCCEEDED(MFCreateAttributes(&readerAttr, 1))) {
                readerAttr->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);
                hr = MFCreateSourceReaderFromMediaSource(source, readerAttr, &reader);
                readerAttr->Release();
            } else {
                hr = MFCreateSourceReaderFromMediaSource(source, nullptr, &reader);
            }
        }

        LONG defaultStride = 0;

        if (SUCCEEDED(hr)) {
            reader->SetStreamSelection((DWORD)MF_SOURCE_READER_ALL_STREAMS, FALSE);
            reader->SetStreamSelection((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, TRUE);

            // Ask for RGB32, not RGB24: the built-in Video Processor MFT that
            // MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING enables reliably supports
            // RGB32 as a conversion target from a webcam's native MJPEG/YUY2/NV12,
            // but frequently rejects RGB24 (MF_E_INVALIDMEDIATYPE) since it isn't
            // one of its supported output formats. We drop the padding byte per
            // pixel ourselves when copying frames out (see rgb32 handling below).
            IMFMediaType* outType = nullptr;
            if (SUCCEEDED(MFCreateMediaType(&outType))) {
                outType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
                outType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
                hr = reader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, outType);
                outType->Release();
            } else {
                hr = E_FAIL;
            }
        }

        if (SUCCEEDED(hr)) {
            IMFMediaType* actual = nullptr;
            if (SUCCEEDED(reader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, &actual))) {
                UINT32 w = 0, h = 0;
                MFGetAttributeSize(actual, MF_MT_FRAME_SIZE, &w, &h);
                width = (int)w;
                height = (int)h;

                UINT32 strideU = 0;
                if (FAILED(actual->GetUINT32(MF_MT_DEFAULT_STRIDE, &strideU))) {
                    defaultStride = -(LONG)(width * 4); // RGB32 default convention: bottom-up
                } else {
                    defaultStride = (LONG)strideU;
                }
                actual->Release();
            }
            valid = (width > 0 && height > 0);
        } else {
            spdlog::warn("LiveCameraCapture: failed to open/configure device {} (hr=0x{:08x}).", symbolicLink, (unsigned long)hr);
        }

        // Camera property control for dark mode. The Media Foundation capture
        // source for UVC cameras implements the classic DirectShow control
        // interfaces via QueryInterface; either may be absent on exotic devices.
        IAMCameraControl* camCtrl = nullptr;
        IAMVideoProcAmp* procAmp = nullptr;
        struct SavedProp {
            long value = 0;
            long flags = 0;
            bool saved = false;
        };
        SavedProp savedExposure, savedGain, savedBacklight;
        bool darkApplied = false;

        auto applyDarkMode = [&]() {
            long mn = 0, mx = 0, step = 0, def = 0, caps = 0;
            if (camCtrl != nullptr &&
                SUCCEEDED(camCtrl->GetRange(CameraControl_Exposure, &mn, &mx, &step, &def, &caps)) &&
                (caps & CameraControl_Flags_Manual)) {
                if (!savedExposure.saved) {
                    savedExposure.saved = SUCCEEDED(camCtrl->Get(CameraControl_Exposure, &savedExposure.value, &savedExposure.flags));
                }
                // exposure is log2(seconds); mn is the shortest/darkest. A quarter
                // of the way up from the floor is dark enough to kill ambient
                // light while lit pixels stay clearly visible.
                long v = mn + (mx - mn) / 4;
                if (step > 0) v = mn + ((v - mn) / step) * step;
                camCtrl->Set(CameraControl_Exposure, v, CameraControl_Flags_Manual);
            } else {
                spdlog::warn("LiveCameraCapture: camera does not support manual exposure; dark mode will rely on gain only.");
            }
            if (procAmp != nullptr) {
                if (SUCCEEDED(procAmp->GetRange(VideoProcAmp_Gain, &mn, &mx, &step, &def, &caps)) &&
                    (caps & VideoProcAmp_Flags_Manual)) {
                    if (!savedGain.saved) {
                        savedGain.saved = SUCCEEDED(procAmp->Get(VideoProcAmp_Gain, &savedGain.value, &savedGain.flags));
                    }
                    procAmp->Set(VideoProcAmp_Gain, mn, VideoProcAmp_Flags_Manual);
                }
                if (SUCCEEDED(procAmp->GetRange(VideoProcAmp_BacklightCompensation, &mn, &mx, &step, &def, &caps)) &&
                    (caps & VideoProcAmp_Flags_Manual)) {
                    if (!savedBacklight.saved) {
                        savedBacklight.saved = SUCCEEDED(procAmp->Get(VideoProcAmp_BacklightCompensation, &savedBacklight.value, &savedBacklight.flags));
                    }
                    procAmp->Set(VideoProcAmp_BacklightCompensation, mn, VideoProcAmp_Flags_Manual);
                }
            }
            darkApplied = true;
        };
        auto restoreDarkMode = [&]() {
            if (!darkApplied) return;
            if (camCtrl != nullptr && savedExposure.saved) {
                camCtrl->Set(CameraControl_Exposure, savedExposure.value, savedExposure.flags);
            }
            if (procAmp != nullptr && savedGain.saved) {
                procAmp->Set(VideoProcAmp_Gain, savedGain.value, savedGain.flags);
            }
            if (procAmp != nullptr && savedBacklight.saved) {
                procAmp->Set(VideoProcAmp_BacklightCompensation, savedBacklight.value, savedBacklight.flags);
            }
            darkApplied = false;
        };

        if (valid) {
            source->QueryInterface(IID_PPV_ARGS(&camCtrl));
            source->QueryInterface(IID_PPV_ARGS(&procAmp));

            std::vector<uint8_t> rgb;
            while (!stopRequested.load()) {
                int req = darkModeRequest.exchange(-1);
                if (req == 1) {
                    applyDarkMode();
                } else if (req == 0) {
                    restoreDarkMode();
                }

                DWORD streamIndex = 0, flags = 0;
                LONGLONG ts = 0;
                IMFSample* sample = nullptr;
                HRESULT rhr = reader->ReadSample((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &streamIndex, &flags, &ts, &sample);

                if (FAILED(rhr) || (flags & MF_SOURCE_READERF_ENDOFSTREAM)) {
                    if (sample != nullptr) sample->Release();
                    break;
                }

                if (sample != nullptr) {
                    IMFMediaBuffer* buffer = nullptr;
                    if (SUCCEEDED(sample->ConvertToContiguousBuffer(&buffer))) {
                        BYTE* data = nullptr;
                        DWORD maxLen = 0, curLen = 0;
                        if (SUCCEEDED(buffer->Lock(&data, &maxLen, &curLen))) {
                            // A driver can renegotiate resolution/stride mid-stream without
                            // us re-querying GetCurrentMediaType, so the buffer it hands back
                            // may be smaller than width/height/defaultStride expect. Bail out
                            // of this sample rather than reading past the mapped region.
                            size_t requiredBytes = (size_t)height * (size_t)std::abs((long)defaultStride);
                            if (curLen >= requiredBytes) {
                                rgb.resize((size_t)width * height * 3);

                                // Captured samples are RGB32 (4 bytes/pixel, Windows DIB byte
                                // order B,G,R,X - NOT the R,G,B order the name suggests). Unpack
                                // to tightly-packed top-down R,G,B triples, dropping the padding
                                // byte and swapping B/R, to match what wxImage expects.
                                int dstRowBytes = width * 3;
                                for (int row = 0; row < height; ++row) {
                                    int srcRow = (defaultStride < 0) ? (height - 1 - row) : row;
                                    const BYTE* srcRowPtr = data + (size_t)srcRow * (size_t)std::abs((long)defaultStride);
                                    BYTE* dstRowPtr = rgb.data() + (size_t)row * dstRowBytes;
                                    for (int col = 0; col < width; ++col) {
                                        const BYTE* px = srcRowPtr + (size_t)col * 4;
                                        BYTE* out = dstRowPtr + (size_t)col * 3;
                                        out[0] = px[2]; // R
                                        out[1] = px[1]; // G
                                        out[2] = px[0]; // B
                                    }
                                }

                                {
                                    std::lock_guard<std::mutex> lock(latestMutex);
                                    latestRgb.swap(rgb);
                                    latestWidth = width;
                                    latestHeight = height;
                                    hasLatest = true;
                                    ++generation;
                                }
                            }

                            buffer->Unlock();
                        }
                        buffer->Release();
                    }
                    sample->Release();
                }
            }
        }

        // leave the camera the way we found it - manual exposure would otherwise
        // persist in the driver and darken the user's next video call
        restoreDarkMode();
        if (camCtrl != nullptr) camCtrl->Release();
        if (procAmp != nullptr) procAmp->Release();

        if (reader != nullptr) reader->Release();
        if (source != nullptr) {
            source->Shutdown();
            source->Release();
        }
        if (srcAttr != nullptr) srcAttr->Release();

        MFShutdown();
    }
};

LiveCameraCapture::LiveCameraCapture(const std::string& symbolicLink)
{
    _impl = new Impl();
    _impl->symbolicLink = symbolicLink;
    _impl->captureThread = std::thread(&Impl::CaptureLoop, _impl);

    // give the capture thread a short window to open the device and report
    // whether it succeeded, so the caller can bail out immediately on failure
    // rather than discovering it only once a scan is underway.
    for (int i = 0; i < 100 && !_impl->valid.load() && _impl->captureThread.joinable(); ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}

LiveCameraCapture::~LiveCameraCapture()
{
    Stop();
    delete _impl;
}

bool LiveCameraCapture::IsValid() const
{
    return _impl->valid.load();
}

void LiveCameraCapture::Stop()
{
    if (_impl->captureThread.joinable()) {
        _impl->stopRequested = true;
        _impl->captureThread.join();
    }
}

void LiveCameraCapture::SetDarkMode(bool enabled)
{
    _impl->darkModeRequest = enabled ? 1 : 0;
}

bool LiveCameraCapture::TryGetLatestFrame(std::vector<uint8_t>& outRgb, int& outWidth, int& outHeight) const
{
    std::lock_guard<std::mutex> lock(_impl->latestMutex);
    if (!_impl->hasLatest) return false;
    if (_impl->generation != _impl->lastReadGeneration) {
        outRgb = _impl->latestRgb;
        outWidth = _impl->latestWidth;
        outHeight = _impl->latestHeight;
        _impl->lastReadGeneration = _impl->generation;
    }
    return true;
}

#endif // _WIN32
