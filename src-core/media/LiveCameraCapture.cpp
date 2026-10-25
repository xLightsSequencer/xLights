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

#elif defined(__linux__)

// Linux: raw V4L2 ioctl (video4linux2). No extra library - linux/videodev2.h
// ships with the kernel headers already present on any Linux dev box, unlike
// libv4l2 which would add a new link dependency for a conversion convenience
// we don't need: webcams that don't natively deliver RGB24 almost always
// deliver YUYV, which we convert ourselves below.

#include <cerrno>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>
#include <vector>

#include <dirent.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <linux/videodev2.h>

#include <spdlog/spdlog.h>

namespace {

inline uint8_t ClampByte(int v) { return (uint8_t)std::clamp(v, 0, 255); }

// BT.601 YUV -> RGB, the standard UVC YUYV conversion.
void YuvToRgb(int y, int u, int v, uint8_t* out)
{
    int c = y - 16, d = u - 128, e = v - 128;
    out[0] = ClampByte((298 * c + 409 * e + 128) >> 8);
    out[1] = ClampByte((298 * c - 100 * d - 208 * e + 128) >> 8);
    out[2] = ClampByte((298 * c + 516 * d + 128) >> 8);
}

} // namespace

std::vector<LiveCameraDevice> EnumerateLiveCameras()
{
    std::vector<LiveCameraDevice> result;

    DIR* dir = opendir("/dev");
    if (dir == nullptr) return result;
    std::vector<std::string> candidates;
    while (struct dirent* entry = readdir(dir)) {
        std::string name(entry->d_name);
        if (name.rfind("video", 0) == 0) candidates.push_back("/dev/" + name);
    }
    closedir(dir);
    std::sort(candidates.begin(), candidates.end());

    for (const auto& path : candidates) {
        int fd = open(path.c_str(), O_RDWR | O_NONBLOCK);
        if (fd < 0) continue;

        struct v4l2_capability cap;
        std::memset(&cap, 0, sizeof(cap));
        if (ioctl(fd, VIDIOC_QUERYCAP, &cap) == 0) {
            uint32_t caps = (cap.capabilities & V4L2_CAP_DEVICE_CAPS) ? cap.device_caps : cap.capabilities;
            // A UVC webcam typically exposes a metadata-only node alongside its
            // real capture node; V4L2_CAP_STREAMING (mmap I/O) filters those out.
            if ((caps & V4L2_CAP_VIDEO_CAPTURE) && (caps & V4L2_CAP_STREAMING)) {
                LiveCameraDevice dev;
                dev.name = std::string(reinterpret_cast<const char*>(cap.card));
                dev.symbolicLink = path;
                result.push_back(std::move(dev));
            }
        }
        close(fd);
    }
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

    struct MappedBuffer {
        void* start = nullptr;
        size_t length = 0;
    };

    void CaptureLoop()
    {
        int fd = open(symbolicLink.c_str(), O_RDWR);
        if (fd < 0) {
            spdlog::warn("LiveCameraCapture: failed to open {} ({}).", symbolicLink, strerror(errno));
            return;
        }

        struct v4l2_format fmt;
        std::memset(&fmt, 0, sizeof(fmt));
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (ioctl(fd, VIDIOC_G_FMT, &fmt) != 0) {
            spdlog::warn("LiveCameraCapture: VIDIOC_G_FMT failed on {} ({}).", symbolicLink, strerror(errno));
            close(fd);
            return;
        }
        uint32_t desiredWidth = fmt.fmt.pix.width > 0 ? fmt.fmt.pix.width : 640;
        uint32_t desiredHeight = fmt.fmt.pix.height > 0 ? fmt.fmt.pix.height : 480;

        bool useRgb24 = false;
        auto tryFormat = [&](uint32_t pixfmt) -> bool {
            struct v4l2_format f;
            std::memset(&f, 0, sizeof(f));
            f.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            f.fmt.pix.width = desiredWidth;
            f.fmt.pix.height = desiredHeight;
            f.fmt.pix.pixelformat = pixfmt;
            f.fmt.pix.field = V4L2_FIELD_ANY;
            if (ioctl(fd, VIDIOC_S_FMT, &f) != 0) return false;
            if (f.fmt.pix.pixelformat != pixfmt) return false;
            fmt = f;
            return true;
        };

        // Most UVC webcams only deliver YUYV or MJPEG natively; ask for RGB24
        // first (a few uvcvideo-backed devices do support it) and fall back to
        // YUYV, which we convert ourselves. MJPEG decode would need a JPEG
        // library we don't otherwise link on this path, so it's not offered -
        // a device that only speaks MJPEG at the driver level is skipped.
        if (tryFormat(V4L2_PIX_FMT_RGB24)) {
            useRgb24 = true;
        } else if (!tryFormat(V4L2_PIX_FMT_YUYV)) {
            spdlog::warn("LiveCameraCapture: {} offers neither RGB24 nor YUYV capture.", symbolicLink);
            close(fd);
            return;
        }

        width = (int)fmt.fmt.pix.width;
        height = (int)fmt.fmt.pix.height;

        struct v4l2_requestbuffers req;
        std::memset(&req, 0, sizeof(req));
        req.count = 4;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_MMAP;
        if (ioctl(fd, VIDIOC_REQBUFS, &req) != 0 || req.count < 1) {
            spdlog::warn("LiveCameraCapture: VIDIOC_REQBUFS failed on {} ({}).", symbolicLink, strerror(errno));
            close(fd);
            return;
        }

        std::vector<MappedBuffer> buffers(req.count);
        bool mapFailed = false;
        for (uint32_t i = 0; i < req.count && !mapFailed; ++i) {
            struct v4l2_buffer buf;
            std::memset(&buf, 0, sizeof(buf));
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index = i;
            if (ioctl(fd, VIDIOC_QUERYBUF, &buf) != 0) { mapFailed = true; break; }
            buffers[i].length = buf.length;
            buffers[i].start = mmap(nullptr, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
            if (buffers[i].start == MAP_FAILED) { buffers[i].start = nullptr; mapFailed = true; break; }
            if (ioctl(fd, VIDIOC_QBUF, &buf) != 0) { mapFailed = true; break; }
        }

        bool streaming = false;
        if (!mapFailed) {
            int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            streaming = ioctl(fd, VIDIOC_STREAMON, &type) == 0;
        }
        valid = streaming;

        // Camera property control for dark mode (same intent as the Windows
        // IAMCameraControl/IAMVideoProcAmp path: pin a short exposure, floor the
        // gain, and disable backlight compensation so ambient light drops out and
        // lit pixels stand out). Controls absent on a given device are skipped.
        auto queryCtrl = [&](uint32_t id, struct v4l2_queryctrl& qc) -> bool {
            std::memset(&qc, 0, sizeof(qc));
            qc.id = id;
            return ioctl(fd, VIDIOC_QUERYCTRL, &qc) == 0 && !(qc.flags & V4L2_CTRL_FLAG_DISABLED);
        };
        auto getCtrl = [&](uint32_t id, int32_t& value) -> bool {
            struct v4l2_control c;
            std::memset(&c, 0, sizeof(c));
            c.id = id;
            if (ioctl(fd, VIDIOC_G_CTRL, &c) != 0) return false;
            value = c.value;
            return true;
        };
        auto setCtrl = [&](uint32_t id, int32_t value) {
            struct v4l2_control c;
            std::memset(&c, 0, sizeof(c));
            c.id = id;
            c.value = value;
            ioctl(fd, VIDIOC_S_CTRL, &c);
        };
        struct SavedCtrl { int32_t value = 0; bool saved = false; };
        SavedCtrl savedExposureAuto, savedExposure, savedGain, savedBacklight;
        bool darkApplied = false;

        auto applyDarkMode = [&]() {
            struct v4l2_queryctrl qc;
            if (queryCtrl(V4L2_CID_EXPOSURE_AUTO, qc)) {
                if (!savedExposureAuto.saved) savedExposureAuto.saved = getCtrl(V4L2_CID_EXPOSURE_AUTO, savedExposureAuto.value);
                setCtrl(V4L2_CID_EXPOSURE_AUTO, V4L2_EXPOSURE_MANUAL);
            }
            if (queryCtrl(V4L2_CID_EXPOSURE_ABSOLUTE, qc)) {
                if (!savedExposure.saved) savedExposure.saved = getCtrl(V4L2_CID_EXPOSURE_ABSOLUTE, savedExposure.value);
                // A quarter of the way up from the floor is dark enough to kill
                // ambient light while lit pixels stay clearly visible.
                int32_t v = qc.minimum + (qc.maximum - qc.minimum) / 4;
                if (qc.step > 0) v = qc.minimum + ((v - qc.minimum) / qc.step) * qc.step;
                setCtrl(V4L2_CID_EXPOSURE_ABSOLUTE, v);
            } else {
                spdlog::warn("LiveCameraCapture: camera does not support manual exposure; dark mode will rely on gain only.");
            }
            if (queryCtrl(V4L2_CID_GAIN, qc)) {
                if (!savedGain.saved) savedGain.saved = getCtrl(V4L2_CID_GAIN, savedGain.value);
                setCtrl(V4L2_CID_GAIN, qc.minimum);
            }
            if (queryCtrl(V4L2_CID_BACKLIGHT_COMPENSATION, qc)) {
                if (!savedBacklight.saved) savedBacklight.saved = getCtrl(V4L2_CID_BACKLIGHT_COMPENSATION, savedBacklight.value);
                setCtrl(V4L2_CID_BACKLIGHT_COMPENSATION, qc.minimum);
            }
            darkApplied = true;
        };
        auto restoreDarkMode = [&]() {
            if (!darkApplied) return;
            if (savedExposure.saved) setCtrl(V4L2_CID_EXPOSURE_ABSOLUTE, savedExposure.value);
            if (savedExposureAuto.saved) setCtrl(V4L2_CID_EXPOSURE_AUTO, savedExposureAuto.value);
            if (savedGain.saved) setCtrl(V4L2_CID_GAIN, savedGain.value);
            if (savedBacklight.saved) setCtrl(V4L2_CID_BACKLIGHT_COMPENSATION, savedBacklight.value);
            darkApplied = false;
        };

        if (streaming) {
            uint32_t stride = fmt.fmt.pix.bytesperline > 0
                ? fmt.fmt.pix.bytesperline
                : (useRgb24 ? (uint32_t)width * 3 : (uint32_t)width * 2);

            std::vector<uint8_t> rgb;
            while (!stopRequested.load()) {
                int req2 = darkModeRequest.exchange(-1);
                if (req2 == 1) applyDarkMode();
                else if (req2 == 0) restoreDarkMode();

                // Short timeout so stopRequested is re-checked promptly even with
                // no frames arriving (e.g. the device was unplugged).
                struct pollfd pfd{ fd, POLLIN, 0 };
                int pr = poll(&pfd, 1, 200);
                if (pr <= 0 || !(pfd.revents & POLLIN)) continue;

                struct v4l2_buffer buf;
                std::memset(&buf, 0, sizeof(buf));
                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;
                if (ioctl(fd, VIDIOC_DQBUF, &buf) != 0) {
                    if (errno == EAGAIN) continue;
                    break;
                }

                const uint8_t* src = static_cast<const uint8_t*>(buffers[buf.index].start);
                rgb.resize((size_t)width * height * 3);
                if (useRgb24) {
                    for (int row = 0; row < height; ++row) {
                        std::memcpy(rgb.data() + (size_t)row * width * 3, src + (size_t)row * stride, (size_t)width * 3);
                    }
                } else {
                    for (int row = 0; row < height; ++row) {
                        const uint8_t* srow = src + (size_t)row * stride;
                        uint8_t* drow = rgb.data() + (size_t)row * width * 3;
                        for (int col = 0; col + 1 < width; col += 2) {
                            int y0 = srow[col * 2 + 0], u = srow[col * 2 + 1];
                            int y1 = srow[col * 2 + 2], v = srow[col * 2 + 3];
                            YuvToRgb(y0, u, v, drow + col * 3);
                            YuvToRgb(y1, u, v, drow + (col + 1) * 3);
                        }
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

                if (ioctl(fd, VIDIOC_QBUF, &buf) != 0) break;
            }
        }

        // leave the camera the way we found it - manual exposure would otherwise
        // persist in the driver and darken the user's next video call
        restoreDarkMode();

        if (streaming) {
            int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            ioctl(fd, VIDIOC_STREAMOFF, &type);
        }
        for (auto& b : buffers) {
            if (b.start != nullptr) munmap(b.start, b.length);
        }
        close(fd);
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

#elif defined(XLIGHTS_HAVE_LIVE_CAMERA)   // Apple desktop

// Apple: a thin shell over AppleLiveCameraBridge, which owns every
// AVFoundation object. Keeping the ObjC on that side of the boundary is
// what lets src-core stay wx-free AND ObjC-free (see AVFoundationVideoReader
// for the same split on the decode path).

#include "media/LiveCameraBridge.h"

std::vector<LiveCameraDevice> EnumerateLiveCameras()
{
    std::vector<LiveCameraDevice> out;
    for (const auto& d : AppleLiveCameraBridge::EnumerateDevices()) {
        LiveCameraDevice dev;
        dev.name = d.name;
        dev.symbolicLink = d.uniqueID;
        out.push_back(std::move(dev));
    }
    return out;
}

struct LiveCameraCapture::Impl {
    AppleLiveCameraBridge::CaptureHandle* handle = nullptr;
    // Mirrors the Windows backend's read-generation trick: the poller ticks
    // faster than frames arrive, so re-copying an unchanged frame would be
    // most of the cost of asking.
    mutable uint64_t generation = 0;
};

LiveCameraCapture::LiveCameraCapture(const std::string& symbolicLink)
    : _impl(new Impl())
{
    _impl->handle = AppleLiveCameraBridge::Open(symbolicLink);
}

LiveCameraCapture::~LiveCameraCapture()
{
    if (_impl->handle != nullptr) AppleLiveCameraBridge::Destroy(_impl->handle);
    delete _impl;
}

bool LiveCameraCapture::IsValid() const
{
    return AppleLiveCameraBridge::IsValid(_impl->handle);
}

void LiveCameraCapture::Stop()
{
    AppleLiveCameraBridge::Stop(_impl->handle);
}

void LiveCameraCapture::SetDarkMode(bool enabled)
{
    AppleLiveCameraBridge::SetDarkMode(_impl->handle, enabled);
}

bool LiveCameraCapture::TryGetLatestFrame(std::vector<uint8_t>& outRgb, int& outWidth, int& outHeight) const
{
    return AppleLiveCameraBridge::TryGetLatestFrame(_impl->handle, outRgb, outWidth,
                                                    outHeight, _impl->generation);
}

#endif // _WIN32 / __linux__ / __APPLE__
