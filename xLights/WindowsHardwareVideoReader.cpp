/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "WindowsHardwareVideoReader.h"

#ifdef __WXMSW__

//#define DETAILED_LOGGING

#include <mferror.h>
#include <propvarutil.h>
#include <wincodec.h>
#include <Wmcodecdsp.h>
#include <wx/msw/ole/oleutils.h>
#include "Parallel.h"
#include <winerror.h>
#include <d3d11_4.h>

#include <log4cpp/Category.hh>

// All of this allows me to dynamically load the Direct X DLLs ensuring that on older platforms it still loads but hardware decoding wont work

//#pragma comment(lib, "mfplat.lib") // mfplat.dll - media foundation platform
    // MFStartup
    typedef DWORD (*MFStartup_ptr)(ULONG, DWORD);
    // MFShutdown
    typedef DWORD (*MFShutdown_ptr)();
    // MFCreateDXGIDeviceManager
    typedef DWORD (*MFCreateDXGIDeviceManager_ptr)(UINT*, IMFDXGIDeviceManager**);
    // MFCreateAttributes
    typedef DWORD (*MFCreateAttributes_ptr)(IMFAttributes**, UINT32);
    // MFCreateMediaType
    typedef DWORD (*MFCreateMediaType_ptr)(IMFMediaType**);

//#pragma comment(lib, "mfreadwrite.lib") // mfreadwrite.dll - media foundation read and write
    // MFCreateSourceReaderFromURL
    typedef DWORD (*MFCreateSourceReaderFromURL_ptr)(LPCWSTR, IMFAttributes*, IMFSourceReader**);
#pragma comment(lib, "mfuuid.lib")
//#pragma comment(lib, "d3d11.lib") // d3d11.dll - direct 3d v11 
    // D3D11CreateDevice
    typedef DWORD (*D3D11CreateDevice_ptr)(IDXGIAdapter*, D3D_DRIVER_TYPE, HMODULE Software, UINT Flags, const D3D_FEATURE_LEVEL*, UINT, UINT, ID3D11Device**, D3D_FEATURE_LEVEL*, ID3D11DeviceContext**);
#pragma comment(lib, "propsys.lib") // propsys.dll microsoft property system
#pragma comment(lib, "wmcodecdspuuid.lib")


#define TIME_DIV 10000
#define ENABLE_HW_ACCELERATION
#define ENABLE_HW_DRIVER

#define SAFEEXEC(fn, msg)               \
    if (SUCCEEDED(hr)) {                \
        hr = fn;                        \
        if (FAILED(hr)) {               \
            logger_base.error("---------- " msg " : 0x%08x : %s", hr, (const char*)WindowsHardwareVideoReader::DecodeMFError(hr).c_str()); \
        }                               \
    }

#define COMMA ,
#define LIT(x) x
#define DYNAMICCALL(dll, function, fn, msg)                                                                                                           \
    {                                                                                                                                                 \
        LIT(function)_ptr ffn = (LIT(function)_ptr)GetFunction(dll, #function);                                                                                                       \
        if (ffn == nullptr) {                                                                                                                         \
            logger_base.error("---------- " msg " : 0x%08x : %s", hr, (const char*)WindowsHardwareVideoReader::DecodeMFError(E_NOINTERFACE).c_str()); \
            hr = E_NOINTERFACE;                                                                                                                       \
        } else {                                                                                                                                      \
            SAFEEXEC((ffn)(fn), msg);                                                                                                                        \
        }                                                                                                                                             \
    }

static std::map<std::string, HINSTANCE> __delayLoadDLLs;
static std::map<std::string, FARPROC> __delayLoadFunctions;

HINSTANCE GetDLL(const std::string& dll)
{
    if (__delayLoadDLLs.find(dll) == end(__delayLoadDLLs)) {
        HINSTANCE hinst = ::LoadLibraryA(dll.c_str());
        if (hinst == nullptr)
            return nullptr;
        __delayLoadDLLs[dll] = hinst;
    }
    return __delayLoadDLLs[dll];
}

void FreeAllDLLs()
{
    for (const auto& it : __delayLoadDLLs) {
        ::FreeLibrary(it.second);
    }
    __delayLoadDLLs.clear();
}

FARPROC GetFunction(const std::string& dll, const std::string& function)
{
    if (__delayLoadFunctions.find(function) == end(__delayLoadFunctions)) {
        HINSTANCE hinst = GetDLL(dll);
        if (hinst != nullptr) {
            FARPROC proc = ::GetProcAddress(hinst, function.c_str());
            if (proc != nullptr) {
                __delayLoadFunctions[function] = proc;
            } else {
                return nullptr;
            }
        } else {
            return nullptr;
        }
    }

    return __delayLoadFunctions[function];
}

class WVHRStatic
{
    bool _ok = false;

public:
    WVHRStatic()
    {
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

        HRESULT hr = S_OK;

//#define THREADING COINIT_MULTITHREADED
#define THREADING COINIT_APARTMENTTHREADED

        SAFEEXEC(::CoInitializeEx(nullptr, THREADING | COINIT_SPEED_OVER_MEMORY | COINIT_DISABLE_OLE1DDE), "WHVD: Failed to initialise OLE EX");

        // we test load all the dlls to make sure hardware video decoding is possible ... if any fail then we wont try to hardware decode
        // this does not protect us against internal differences but I am hoping it means we can load and run on older platforms
        if (GetDLL("mfplat.dll") == nullptr || GetDLL("mfreadwrite.dll") == nullptr || GetDLL("d3d11.dll") == nullptr) {
            logger_base.error("Failed to load mfplat.dll ... windows hardware debugging disabled.");
        } else {
            DYNAMICCALL("mfplat.dll", MFStartup, MF_VERSION COMMA MFSTARTUP_FULL, "WHVD: Failed to initialise Media Framework");
            if (SUCCEEDED(hr)) {
                _ok = true;
            }
        }
    }
    virtual ~WVHRStatic()
    {
        if (_ok) {
            static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            HRESULT hr = S_OK;
            DYNAMICCALL("mfplat.dll", MFShutdown, , "WHVD: Failed to initialise Media Framework");
        }
        FreeAllDLLs();
    }
    bool IsOk() const
    {
        return _ok;
    }
};

WVHRStatic WindowsHardwareVideoReader::_init; // this initialises and de-initialises the framework

WindowsHardwareVideoReader::WindowsHardwareVideoReader(const std::string& filename, bool wantAlpha, bool usenativeresolution, bool keepaspectratio, uint32_t maxwidth, uint32_t maxheight, AVPixelFormat pixelFormat)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

#ifdef DETAILED_LOGGING
    logger_base.debug("WHVD: Constructor: %s", (const char*)filename.c_str());
#endif

    _pixelFormat = pixelFormat;
    _wantAlpha = wantAlpha;
    _width = maxwidth;
    _height = maxheight;

    if (!_init.IsOk())
        return;

    HRESULT hr = S_OK;

    IMFAttributes* attributes;
    DYNAMICCALL("mfplat.dll", MFCreateAttributes, &attributes COMMA 4, "WHVD: Failed to create Media Framework attributes");

#if defined(ENABLE_HW_ACCELERATION)
    D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };

#if defined(ENABLE_HW_DRIVER)
    DYNAMICCALL("d3d11.dll", D3D11CreateDevice, nullptr COMMA D3D_DRIVER_TYPE_HARDWARE COMMA nullptr COMMA D3D11_CREATE_DEVICE_VIDEO_SUPPORT COMMA levels COMMA ARRAYSIZE(levels) COMMA D3D11_SDK_VERSION COMMA & _device COMMA nullptr COMMA nullptr, "WHVD: Failed to create D3D11 device");
#else
    DYNAMICCALL("d3d11.dll", D3D11CreateDevice, nullptr COMMA D3D_DRIVER_TYPE_NULL COMMA nullptr COMMA 0 COMMA levels COMMA ARRAYSIZE(levels) COMMA D3D11_SDK_VERSION COMMA & _device COMMA nullptr COMMA nullptr, "WHVD: Failed to create D3D11 device");
#endif

    // NOTE: Getting ready for multi-threaded operation
    ID3D11Multithread* multithread = nullptr;
    SAFEEXEC(_device->QueryInterface(__uuidof(ID3D11Multithread), (void**)&multithread), "WHVD: Failed to get device multithread interface");
    if (SUCCEEDED(hr)) {
        multithread->SetMultithreadProtected(true);
        SafeRelease(&multithread);
    }

    UINT token = 0;
    DYNAMICCALL("mfplat.dll", MFCreateDXGIDeviceManager, &token COMMA & _deviceManager, "WHVD: Failed to create DXGI device manager");

    SAFEEXEC(_deviceManager->ResetDevice(_device, token), "WHVD: Failed to reset device");

    SAFEEXEC(attributes->SetUnknown(MF_SOURCE_READER_D3D_MANAGER, _deviceManager), "WHVD: Failed to set attribute");
    SAFEEXEC(attributes->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE), "WHVD: Failed to set attribute");
    SAFEEXEC(attributes->SetUINT32(MF_SOURCE_READER_ENABLE_ADVANCED_VIDEO_PROCESSING, TRUE), "WHVD: Failed to set attribute");
#else
    SAFEEXEC(attributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE), "WHVD: Failed to set attribute");
#endif

    // Create the source reader from the URL.
    std::wstring fn(filename.begin(), filename.end());
    DYNAMICCALL("mfreadwrite.dll", MFCreateSourceReaderFromURL, fn.c_str() COMMA attributes COMMA & _reader, "WHVD: Failed to create video source reader");

    SafeRelease(&attributes);

    // Attempt to find a video stream.
    SAFEEXEC(SelectVideoStream(usenativeresolution, keepaspectratio), "WHVD: Failed to find video stream");

    if (SUCCEEDED(hr)) {

        Seek(0);

        _frame = av_frame_alloc();
        _frame->width = _width;
        _frame->height = _height;
        _frame->linesize[0] = _width * GetPixelBytes();
        _frame->data[0] = (uint8_t*)av_malloc((size_t)_height * _frame->linesize[0]);
        memset(_frame->data[0], 0x00, (size_t)_height * _frame->linesize[0]);
        _frame->format = _pixelFormat;
        wxASSERT(_reader != nullptr);
        wxASSERT(_deviceManager != nullptr);

        logger_base.debug("WHVD: Hardware Video Decoder Initialised OK for video: %s", (const char*)filename.c_str());
    } else {
        SafeRelease(&_reader);
        SafeRelease(&_deviceManager);
        wxASSERT(_reader == nullptr);
        wxASSERT(_deviceManager == nullptr);
    }

    wxASSERT(attributes == nullptr);

#ifdef DETAILED_LOGGING
    logger_base.debug("WHVD: Constructor Done: %s", (const char*)filename.c_str());
#endif
}

uint8_t WindowsHardwareVideoReader::GetPixelBytes() const
{
    return ((_pixelFormat == AV_PIX_FMT_RGB24 || _pixelFormat == AV_PIX_FMT_BGR24) ? 3 : 4);
}

WindowsHardwareVideoReader::~WindowsHardwareVideoReader()
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
#ifdef DETAILED_LOGGING
    logger_base.debug("WHVD: Destructor.");
#endif
    SafeRelease(&_reader);
    SafeRelease(&_deviceManager);
    SafeRelease(&_device);

    if (_frame != nullptr) {
        if (_frame->data[0] != nullptr) {
            av_free(_frame->data[0]);
        }
        av_frame_free(&_frame);
    }

#ifdef DETAILED_LOGGING
    logger_base.debug("WHVD: Destructor DONE.");
#endif
}

bool WindowsHardwareVideoReader::CanSeek() const
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    bool res = false;
    HRESULT hr = S_OK;

    if (_reader != nullptr) {
        PROPVARIANT var;
        PropVariantInit(&var);
        SAFEEXEC(_reader->GetPresentationAttribute((DWORD)MF_SOURCE_READER_MEDIASOURCE, MF_SOURCE_READER_MEDIASOURCE_CHARACTERISTICS, &var), "WHVD: Failed to check seekability");

        ULONG flags = 0;
        SAFEEXEC(PropVariantToUInt32(var, &flags), "WHVD: Failed to decode flags variant");

        if (SUCCEEDED(hr)) {
            // If the source has slow seeking, we will treat it as
            // not supporting seeking.

            if ((flags & MFMEDIASOURCE_CAN_SEEK) &&
                !(flags & MFMEDIASOURCE_HAS_SLOW_SEEK)) {
                res = TRUE;
                logger_base.debug("WHVD: Able to seek.");
            }
        }
    }
    return res;
}

HRESULT WindowsHardwareVideoReader::SelectVideoStream(bool usenativeresolution, bool keepaspectratio)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    HRESULT hr = S_OK;

    // I need to get the native size of the video first
    IMFMediaType* pType = nullptr;
    DYNAMICCALL("mfplat.dll", MFCreateMediaType, &pType, "WHVD: Failed to create media type");
    if (SUCCEEDED(hr) && pType != nullptr) {
        SAFEEXEC(pType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video), "WHVD: Failed to set major type");

        SAFEEXEC(pType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32), "WHVD: Failed to set sub type");

        SAFEEXEC(_reader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, pType), "WHVD: Failed to set media type");

        SAFEEXEC(_reader->SetStreamSelection((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, TRUE), "WHVD: Failed to set stream");

        SafeRelease(&pType);
        pType = nullptr;
    }

    SAFEEXEC(_reader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, &pType), "WHVD: Failed to get media type");
    if (SUCCEEDED(hr) && pType != nullptr) {
        SAFEEXEC(MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &_nativeWidth, &_nativeHeight), "WHVD: Failed to get native size");

        if (SUCCEEDED(hr)) {
            PROPVARIANT var;
            PropVariantInit(&var);

            SAFEEXEC(_reader->GetPresentationAttribute((DWORD)MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &var), "WHVD: Failed to get duration");

            if (SUCCEEDED(hr)) {
                _duration = (uint32_t)(var.hVal.QuadPart / TIME_DIV);
            }

            PropVariantClear(&var);
        }

        if (SUCCEEDED(hr)) {
            uint32_t numerator = 0;
            uint32_t denominator = 0;
            SAFEEXEC(MFGetAttributeRatio(pType, MF_MT_FRAME_RATE, &numerator, &denominator), "WHVD: Failed to get frame rate");

            if (SUCCEEDED(hr) && denominator != 0) {
                _frameMS = (uint32_t)(1000.0f / ((float)numerator / (float)denominator));
            }

            if (denominator == 0) {
                logger_base.error("WHVD: Failed to get frame rate ");
            }
        }

        SafeRelease(&pType);
        pType = nullptr;
    }

    // Configure the source reader to give us progressive RGB32 frames.
    // The source reader will load the decoder if needed.

    DYNAMICCALL("mfplat.dll", MFCreateMediaType, &pType, "WHVD: Failed to create media type");
    if (SUCCEEDED(hr) && pType != nullptr) {
        SAFEEXEC(pType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video), "WHVD: Failed to set major type");

        SAFEEXEC(pType->SetGUID(MF_MT_SUBTYPE, _wantAlpha ? MFVideoFormat_ARGB32 : MFVideoFormat_RGB32), "WHVD: Failed to set sub type");

        if (SUCCEEDED(hr)) {
            if (usenativeresolution) {
            } else {
                SAFEEXEC(MFSetAttributeSize(pType, MF_MT_FRAME_SIZE, _width, _height), "WHVD: Failed to set target size");

                if (!keepaspectratio) {
                    // we need to stretch pixels
                    logger_base.info("Stretching pixels by %u/%u", _height * _nativeWidth, _width * _nativeHeight);
                    SAFEEXEC(MFSetAttributeRatio(pType, MF_MT_PIXEL_ASPECT_RATIO, _height * _nativeWidth, _width * _nativeHeight), "WHVD: Failed to set target ratio");
                }
            }
        }

        SAFEEXEC(_reader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, pType), "WHVD: Failed to set media type");

        SAFEEXEC(_reader->SetStreamSelection((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, TRUE), "WHVD: Failed to set stream");

        SafeRelease(&pType);
        pType = nullptr;
    }

    SAFEEXEC(_reader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, &pType), "WHVD: Failed to get media type");
    if (SUCCEEDED(hr) && pType != nullptr) {
        _stride = (LONG)MFGetAttributeUINT32(pType, MF_MT_DEFAULT_STRIDE, 1);
        wxASSERT(_stride = _width * GetPixelBytes());

        GUID subtype = { 0 };
        SAFEEXEC(pType->GetGUID(MF_MT_SUBTYPE, &subtype), "WHVD: Failed to get media subtype");

        if (SUCCEEDED(hr)) {
            if (!IsEqualGUID(subtype, MFVideoFormat_RGB32) && !IsEqualGUID(subtype, MFVideoFormat_ARGB32)) {
                logger_base.error("WHVD: Invalid media subtype");
                hr = E_UNEXPECTED;
            }
        }

        SafeRelease(&pType);
        pType = nullptr;
    }

    return hr;
}

bool WindowsHardwareVideoReader::IsOk() const
{
    return _reader != nullptr && _nativeWidth != 0 && _nativeHeight != 0 && _frameMS != 0 && _duration != 0;
}

bool WindowsHardwareVideoReader::Seek(uint32_t pos)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (_reader == nullptr || pos > GetDuration())
        return false;

    HRESULT hr = S_OK;
    PROPVARIANT var;
    PropVariantInit(&var);

    var.vt = VT_I8;
    if (pos < _frameMS - 1) {
        var.hVal.QuadPart = (LONGLONG)(pos)*TIME_DIV;
    } else {
        var.hVal.QuadPart = (LONGLONG)(pos - _frameMS - 1) * TIME_DIV;
    }

#ifdef DETAILED_LOGGING
    logger_base.debug("WHVD: Seeking to %u", pos);
#endif
    SAFEEXEC(_reader->SetCurrentPosition(GUID_NULL, var), "WHVD: Failed to seek");

    if (FAILED(hr)) {
        return false;
    }

    if (pos > 0) {
        bool first = true;
        do {
            uint32_t lastPos = _curPos;
            GetNextFrame(0xFFFFFFFF, 0xFFFFFFFF);
            if (!first && lastPos == _curPos) {
                logger_base.error("WHVD: Seek failed.");
                return false;
            }
            first = false;
        } while (((LONGLONG)_curPos) * TIME_DIV < var.hVal.QuadPart);
    } else {
        _curPos = 0;
    }
    return true;
}

std::string WindowsHardwareVideoReader::DecodeDXGIReason(HRESULT reason) const
{
    switch (reason) {
    case DXGI_ERROR_ACCESS_DENIED:
        return "Access denied.";
    case DXGI_ERROR_ACCESS_LOST:
        return "Access lost.";
    case DXGI_ERROR_ALREADY_EXISTS:
        return "Element already exists.";
    case DXGI_ERROR_CANNOT_PROTECT_CONTENT:
        return "Cannot protect content.";
    case DXGI_ERROR_DEVICE_HUNG:
        return "Device hung.";
    case DXGI_ERROR_DEVICE_REMOVED:
        return "Device removed.";
    case DXGI_ERROR_DEVICE_RESET:
        return "Device reset.";
    case DXGI_ERROR_DRIVER_INTERNAL_ERROR:
        return "Driver internal error.";
    case DXGI_ERROR_FRAME_STATISTICS_DISJOINT:
        return "Frame statistics disjoint.";
    case DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE:
        return "Graphics VIDPN source in use.";
    case DXGI_ERROR_INVALID_CALL:
        return "Invalid call.";
    case DXGI_ERROR_MORE_DATA:
        return "Buffer too small.";
    case DXGI_ERROR_NAME_ALREADY_EXISTS:
        return "Name already exists.";
    case DXGI_ERROR_NONEXCLUSIVE:
        return "Non exclusive access.";
    case DXGI_ERROR_NOT_CURRENTLY_AVAILABLE:
        return "Not currently available.";
    case DXGI_ERROR_NOT_FOUND:
        return "Not found.";
    case DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED:
        return "Remote client disconnected.";
    case DXGI_ERROR_REMOTE_OUTOFMEMORY:
        return "Remote out of memory.";
    case DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE:
        return "Restrict to output stale.";
    case DXGI_ERROR_SDK_COMPONENT_MISSING:
        return "SDK component missing.";
    case DXGI_ERROR_SESSION_DISCONNECTED:
        return "Session disconnected.";
    case DXGI_ERROR_UNSUPPORTED:
        return "Unsupported.";
    case DXGI_ERROR_WAIT_TIMEOUT:
        return "Timeout.";
    case DXGI_ERROR_WAS_STILL_DRAWING:
        return "Was still drawing.";
    case S_OK:
        return "No error.";
    default:
        break;
    }

    return "Unknown code.";
}

bool WindowsHardwareVideoReader::BitmapFromSample(IMFSample* sample, AVFrame* frame)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    bool res = false;
    HRESULT hr = S_OK;

#ifdef DETAILED_LOGGING
    logger_base.debug("WHVD: Get sample buffer");
#endif
    IMFMediaBuffer* pBuffer = nullptr;
    SAFEEXEC(sample->ConvertToContiguousBuffer(&pBuffer), "WHVD: Failed to get sample buffer");

#ifdef DETAILED_LOGGING
    logger_base.debug("WHVD: lock buffer");
#endif
    BYTE* pBitmapData = nullptr; // Bitmap data
    DWORD cbBitmapData = 0;      // Size of data, in bytes
    SAFEEXEC(pBuffer->Lock(&pBitmapData, nullptr, &cbBitmapData), "WHVD: Failed to lock buffer");

    if (FAILED(hr)) {
        logger_base.error("Was reading video at %dms", _curPos);
        HRESULT reason = _device->GetDeviceRemovedReason();
        logger_base.error("Device removed reason 0x%08x : %s", reason, (const char*)DecodeDXGIReason(reason).c_str());
    }

    if (SUCCEEDED(hr)) {
        wxASSERT(pBitmapData != nullptr);
        wxASSERT(cbBitmapData > 0);
        if (_pixelFormat == AVPixelFormat::AV_PIX_FMT_BGRA || _pixelFormat == AVPixelFormat::AV_PIX_FMT_BGR24) {
            // I am not sure this is correct
            memcpy(_frame->data[0], pBitmapData, std::min((uint32_t)cbBitmapData, (uint32_t)_frame->linesize[0] * _height));
        } else {
            uint8_t pb = GetPixelBytes();
            parallel_for(0, std::min((uint32_t)cbBitmapData / 4, ((uint32_t)_frame->linesize[0] * _height) / pb), [this, pb, pBitmapData](int i) {
                *(this->_frame->data[0] + i * pb + 0) = *(pBitmapData + i * 4 + 2);
                *(this->_frame->data[0] + i * pb + 1) = *(pBitmapData + i * 4 + 1);
                *(this->_frame->data[0] + i * pb + 2) = *(pBitmapData + i * 4 + 0);
                if (pb == 4)
                    *(this->_frame->data[0] + i * pb + 3) = *(pBitmapData + i * 4 + 3);
            });
        }
        res = true;
#ifdef DETAILED_LOGGING
        logger_base.debug("WHVD: Unlock buffer");
#endif
        SAFEEXEC(pBuffer->Unlock(), "WHVD: Failed to unlock buffer");
    }

#ifdef DETAILED_LOGGING
    logger_base.debug("WHVD: Release buffer");
#endif
    SafeRelease(&pBuffer);

    return res;
}

std::string WindowsHardwareVideoReader::DecodeReadFlags(DWORD flags) const
{
    std::string res;

    if (flags & MF_SOURCE_READERF_ERROR) {
        res += " Reader Error,";
    }
    if (flags & MF_SOURCE_READERF_ENDOFSTREAM) {
        res += " End of Stream,";
    }
    if (flags & MF_SOURCE_READERF_NEWSTREAM) {
        res += " New Stream,";
    }
    if (flags & MF_SOURCE_READERF_NATIVEMEDIATYPECHANGED) {
        res += " Native Media Type Changed,";
    }
    if (flags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED) {
        res += " Current Media Type Changed,";
    }
    if (flags & MF_SOURCE_READERF_STREAMTICK) {
        res += " Stream Tick,";
    }
    if (flags & MF_SOURCE_READERF_ALLEFFECTSREMOVED) {
        res += " All Effects Removed,";
    }

    if (res == "") {
        res = "No Flags Set";
    }

    return res;
}

AVFrame* WindowsHardwareVideoReader::GetNextFrame(uint32_t timestampMS, uint32_t gracetime)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    HRESULT hr = S_OK;

#ifdef DETAILED_LOGGING
    logger_base.debug("WHVD: GetNextFrame %u.", timestampMS);
#endif

    if (_reader == nullptr || (timestampMS != 0xFFFFFFFF && timestampMS > GetDuration()))
        return nullptr;

    if (timestampMS != 0xFFFFFFFF && timestampMS != 0 && _curPos > timestampMS && _curPos < timestampMS + GetFrameMS()) {
        // the last frame should be ok ... so just return it again
#ifdef DETAILED_LOGGING
        logger_base.debug("WHVD: Just returning last frame at %u.", _curPos);
#endif
        return _frame;
    }

    // we only seek if the timestamp is valid and we are past the time or more than a second from it.
    if (timestampMS != 0xFFFFFFFF && (_curPos > timestampMS || (LONGLONG)_curPos < (LONGLONG)timestampMS - 1000)) {
        if (!Seek(timestampMS)) {
            return nullptr;
        }
    }

    IMFSample* sample = nullptr;
    do {
        wxASSERT(sample == nullptr);

#ifdef DETAILED_LOGGING
        logger_base.debug("WHVD: Reading sample");
#endif
        DWORD dwFlags = 0;
        LONGLONG currentTime;
        SAFEEXEC(_reader->ReadSample((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, nullptr, &dwFlags, &currentTime, &sample), "WHVD: Failed to read frame");

#ifdef DETAILED_LOGGING
        logger_base.info("Read flags: 0x%08x : %s", dwFlags, (const char *)DecodeReadFlags(dwFlags).c_str());
#endif

        if (FAILED(hr)) {
            return nullptr;
        }

        if (dwFlags & MF_SOURCE_READERF_ENDOFSTREAM) {
#ifdef DETAILED_LOGGING
            logger_base.debug("WHVD: Release sample");
#endif
            SafeRelease(&sample);
            logger_base.info("WHVD: Reached end of video");
            return nullptr;
        }

        if (SUCCEEDED(hr)) {
            _curPos = currentTime / TIME_DIV;
#ifdef DETAILED_LOGGING
            logger_base.debug("Read video at %ums", _curPos);
#endif
        }

        // we are not going to use this frame so we can let it go
        if (_curPos < timestampMS) {
#ifdef DETAILED_LOGGING
            logger_base.debug("WHVD: Release sample");
#endif
            SafeRelease(&sample);
        }

    } while (_curPos < timestampMS && timestampMS != 0xFFFFFFFF);

    if (timestampMS != 0xFFFFFFFF && sample != nullptr) {
        if (!BitmapFromSample(sample, _frame)) {
            logger_base.error("WHVD: Failed to extract the frame bitmap ... Media Foundations may be in a corrupt state.");
        }
#ifdef DETAILED_LOGGING
        logger_base.debug("WHVD: Release sample");
#endif
        SafeRelease(&sample);
#ifdef DETAILED_LOGGING
        logger_base.debug("WHVD: GetNextFrame %u DONE.", timestampMS);
#endif
        return _frame;
    }

    SafeRelease(&sample);

#ifdef DETAILED_LOGGING
    logger_base.debug("WHVD: GetNextFrame %u DONE.", timestampMS);
#endif
    return nullptr;
}

std::string WindowsHardwareVideoReader::DecodeMFError(HRESULT hr)
{
#define HANDLE_MF_ERROR(err) \
    if (hr == err)           \
        return std::string(#err);

    if (hr == S_OK)
        return "No Error";

    HANDLE_MF_ERROR(MF_E_PLATFORM_NOT_INITIALIZED);
    HANDLE_MF_ERROR(MF_E_BUFFERTOOSMALL);
    HANDLE_MF_ERROR(MF_E_INVALIDREQUEST);
    HANDLE_MF_ERROR(MF_E_INVALIDSTREAMNUMBER);
    HANDLE_MF_ERROR(MF_E_INVALIDMEDIATYPE);
    HANDLE_MF_ERROR(MF_E_NOTACCEPTING);
    HANDLE_MF_ERROR(MF_E_NOT_INITIALIZED);
    HANDLE_MF_ERROR(MF_E_UNSUPPORTED_REPRESENTATION);
    HANDLE_MF_ERROR(MF_E_NO_MORE_TYPES);
    HANDLE_MF_ERROR(MF_E_UNSUPPORTED_SERVICE);
    HANDLE_MF_ERROR(MF_E_UNEXPECTED);
    HANDLE_MF_ERROR(MF_E_INVALIDNAME);
    HANDLE_MF_ERROR(MF_E_INVALIDTYPE);
    HANDLE_MF_ERROR(MF_E_INVALID_FILE_FORMAT);
    HANDLE_MF_ERROR(MF_E_INVALIDINDEX);
    HANDLE_MF_ERROR(MF_E_INVALID_TIMESTAMP);
    HANDLE_MF_ERROR(MF_E_UNSUPPORTED_SCHEME);
    HANDLE_MF_ERROR(MF_E_UNSUPPORTED_BYTESTREAM_TYPE);
    HANDLE_MF_ERROR(MF_E_UNSUPPORTED_TIME_FORMAT);
    HANDLE_MF_ERROR(MF_E_NO_SAMPLE_TIMESTAMP);
    HANDLE_MF_ERROR(MF_E_NO_SAMPLE_DURATION);
    HANDLE_MF_ERROR(MF_E_INVALID_STREAM_DATA);
    HANDLE_MF_ERROR(MF_E_RT_UNAVAILABLE);
    HANDLE_MF_ERROR(MF_E_UNSUPPORTED_RATE);
    HANDLE_MF_ERROR(MF_E_THINNING_UNSUPPORTED);
    HANDLE_MF_ERROR(MF_E_REVERSE_UNSUPPORTED);
    HANDLE_MF_ERROR(MF_E_UNSUPPORTED_RATE_TRANSITION);
    HANDLE_MF_ERROR(MF_E_RATE_CHANGE_PREEMPTED);
    HANDLE_MF_ERROR(MF_E_NOT_FOUND);
    HANDLE_MF_ERROR(MF_E_NOT_AVAILABLE);
    HANDLE_MF_ERROR(MF_E_NO_CLOCK);
    HANDLE_MF_ERROR(MF_S_MULTIPLE_BEGIN);
    HANDLE_MF_ERROR(MF_E_MULTIPLE_BEGIN);
    HANDLE_MF_ERROR(MF_E_MULTIPLE_SUBSCRIBERS);
    HANDLE_MF_ERROR(MF_E_TIMER_ORPHANED);
    HANDLE_MF_ERROR(MF_E_STATE_TRANSITION_PENDING);
    HANDLE_MF_ERROR(MF_E_UNSUPPORTED_STATE_TRANSITION);
    HANDLE_MF_ERROR(MF_E_UNRECOVERABLE_ERROR_OCCURRED);
    HANDLE_MF_ERROR(MF_E_SAMPLE_HAS_TOO_MANY_BUFFERS);
    HANDLE_MF_ERROR(MF_E_SAMPLE_NOT_WRITABLE);
    HANDLE_MF_ERROR(MF_E_INVALID_KEY);
    HANDLE_MF_ERROR(MF_E_BAD_STARTUP_VERSION);
    HANDLE_MF_ERROR(MF_E_UNSUPPORTED_CAPTION);
    HANDLE_MF_ERROR(MF_E_INVALID_POSITION);
    HANDLE_MF_ERROR(MF_E_ATTRIBUTENOTFOUND);
    HANDLE_MF_ERROR(MF_E_PROPERTY_TYPE_NOT_ALLOWED);
    HANDLE_MF_ERROR(MF_E_PROPERTY_TYPE_NOT_SUPPORTED);
    HANDLE_MF_ERROR(MF_E_PROPERTY_EMPTY);
    HANDLE_MF_ERROR(MF_E_PROPERTY_NOT_EMPTY);
    HANDLE_MF_ERROR(MF_E_PROPERTY_VECTOR_NOT_ALLOWED);
    HANDLE_MF_ERROR(MF_E_PROPERTY_VECTOR_REQUIRED);
    HANDLE_MF_ERROR(MF_E_OPERATION_CANCELLED);
    HANDLE_MF_ERROR(MF_E_BYTESTREAM_NOT_SEEKABLE);
    HANDLE_MF_ERROR(MF_E_DISABLED_IN_SAFEMODE);
    HANDLE_MF_ERROR(MF_E_CANNOT_PARSE_BYTESTREAM);
    HANDLE_MF_ERROR(MF_E_SOURCERESOLVER_MUTUALLY_EXCLUSIVE_FLAGS);
    HANDLE_MF_ERROR(MF_E_MEDIAPROC_WRONGSTATE);
    HANDLE_MF_ERROR(MF_E_RT_THROUGHPUT_NOT_AVAILABLE);
    HANDLE_MF_ERROR(MF_E_RT_TOO_MANY_CLASSES);
    HANDLE_MF_ERROR(MF_E_RT_WOULDBLOCK);
    HANDLE_MF_ERROR(MF_E_NO_BITPUMP);
    HANDLE_MF_ERROR(MF_E_RT_OUTOFMEMORY);
    HANDLE_MF_ERROR(MF_E_RT_WORKQUEUE_CLASS_NOT_SPECIFIED);
    HANDLE_MF_ERROR(MF_E_INSUFFICIENT_BUFFER);
    HANDLE_MF_ERROR(MF_E_CANNOT_CREATE_SINK);
    HANDLE_MF_ERROR(MF_E_BYTESTREAM_UNKNOWN_LENGTH);
    HANDLE_MF_ERROR(MF_E_SESSION_PAUSEWHILESTOPPED);
    HANDLE_MF_ERROR(MF_S_ACTIVATE_REPLACED);
    HANDLE_MF_ERROR(MF_E_FORMAT_CHANGE_NOT_SUPPORTED);
    HANDLE_MF_ERROR(MF_E_INVALID_WORKQUEUE);
    HANDLE_MF_ERROR(MF_E_DRM_UNSUPPORTED);
    HANDLE_MF_ERROR(MF_E_UNAUTHORIZED);
    HANDLE_MF_ERROR(MF_E_OUT_OF_RANGE);
    HANDLE_MF_ERROR(MF_E_INVALID_CODEC_MERIT);
    HANDLE_MF_ERROR(MF_E_HW_MFT_FAILED_START_STREAMING);
    HANDLE_MF_ERROR(MF_E_OPERATION_IN_PROGRESS);
    HANDLE_MF_ERROR(MF_E_HARDWARE_DRM_UNSUPPORTED);
    HANDLE_MF_ERROR(MF_E_DURATION_TOO_LONG);
    HANDLE_MF_ERROR(MF_E_OPERATION_UNSUPPORTED_AT_D3D_FEATURE_LEVEL);
    HANDLE_MF_ERROR(MF_E_UNSUPPORTED_MEDIATYPE_AT_D3D_FEATURE_LEVEL);
    HANDLE_MF_ERROR(MF_S_ASF_PARSEINPROGRESS);
    HANDLE_MF_ERROR(MF_E_ASF_PARSINGINCOMPLETE);
    HANDLE_MF_ERROR(MF_E_ASF_MISSINGDATA);
    HANDLE_MF_ERROR(MF_E_ASF_INVALIDDATA);
    HANDLE_MF_ERROR(MF_E_ASF_OPAQUEPACKET);
    HANDLE_MF_ERROR(MF_E_ASF_NOINDEX);
    HANDLE_MF_ERROR(MF_E_ASF_OUTOFRANGE);
    HANDLE_MF_ERROR(MF_E_ASF_INDEXNOTLOADED);
    HANDLE_MF_ERROR(MF_E_ASF_TOO_MANY_PAYLOADS);
    HANDLE_MF_ERROR(MF_E_ASF_UNSUPPORTED_STREAM_TYPE);
    HANDLE_MF_ERROR(MF_E_ASF_DROPPED_PACKET);
    HANDLE_MF_ERROR(MF_E_NO_EVENTS_AVAILABLE);
    HANDLE_MF_ERROR(MF_E_INVALID_STATE_TRANSITION);
    HANDLE_MF_ERROR(MF_E_END_OF_STREAM);
    HANDLE_MF_ERROR(MF_E_SHUTDOWN);
    HANDLE_MF_ERROR(MF_E_MP3_NOTFOUND);
    HANDLE_MF_ERROR(MF_E_MP3_OUTOFDATA);
    HANDLE_MF_ERROR(MF_E_MP3_NOTMP3);
    HANDLE_MF_ERROR(MF_E_MP3_NOTSUPPORTED);
    HANDLE_MF_ERROR(MF_E_NO_DURATION);
    HANDLE_MF_ERROR(MF_E_INVALID_FORMAT);
    HANDLE_MF_ERROR(MF_E_PROPERTY_NOT_FOUND);
    HANDLE_MF_ERROR(MF_E_PROPERTY_READ_ONLY);
    HANDLE_MF_ERROR(MF_E_PROPERTY_NOT_ALLOWED);
    HANDLE_MF_ERROR(MF_E_MEDIA_SOURCE_NOT_STARTED);
    HANDLE_MF_ERROR(MF_E_UNSUPPORTED_FORMAT);
    HANDLE_MF_ERROR(MF_E_MP3_BAD_CRC);
    HANDLE_MF_ERROR(MF_E_NOT_PROTECTED);
    HANDLE_MF_ERROR(MF_E_MEDIA_SOURCE_WRONGSTATE);
    HANDLE_MF_ERROR(MF_E_MEDIA_SOURCE_NO_STREAMS_SELECTED);
    HANDLE_MF_ERROR(MF_E_CANNOT_FIND_KEYFRAME_SAMPLE);
    HANDLE_MF_ERROR(MF_E_UNSUPPORTED_CHARACTERISTICS);
    HANDLE_MF_ERROR(MF_E_NO_AUDIO_RECORDING_DEVICE);
    HANDLE_MF_ERROR(MF_E_AUDIO_RECORDING_DEVICE_IN_USE);
    HANDLE_MF_ERROR(MF_E_AUDIO_RECORDING_DEVICE_INVALIDATED);
    HANDLE_MF_ERROR(MF_E_VIDEO_RECORDING_DEVICE_INVALIDATED);
    HANDLE_MF_ERROR(MF_E_VIDEO_RECORDING_DEVICE_PREEMPTED);
    HANDLE_MF_ERROR(MF_E_NETWORK_RESOURCE_FAILURE);
    HANDLE_MF_ERROR(MF_E_NET_WRITE);
    HANDLE_MF_ERROR(MF_E_NET_READ);
    HANDLE_MF_ERROR(MF_E_NET_REQUIRE_NETWORK);
    HANDLE_MF_ERROR(MF_E_NET_REQUIRE_ASYNC);
    HANDLE_MF_ERROR(MF_E_NET_BWLEVEL_NOT_SUPPORTED);
    HANDLE_MF_ERROR(MF_E_NET_STREAMGROUPS_NOT_SUPPORTED);
    HANDLE_MF_ERROR(MF_E_NET_MANUALSS_NOT_SUPPORTED);
    HANDLE_MF_ERROR(MF_E_NET_INVALID_PRESENTATION_DESCRIPTOR);
    HANDLE_MF_ERROR(MF_E_NET_CACHESTREAM_NOT_FOUND);
    HANDLE_MF_ERROR(MF_I_MANUAL_PROXY);
    HANDLE_MF_ERROR(MF_E_NET_REQUIRE_INPUT);
    HANDLE_MF_ERROR(MF_E_NET_REDIRECT);
    HANDLE_MF_ERROR(MF_E_NET_REDIRECT_TO_PROXY);
    HANDLE_MF_ERROR(MF_E_NET_TOO_MANY_REDIRECTS);
    HANDLE_MF_ERROR(MF_E_NET_TIMEOUT);
    HANDLE_MF_ERROR(MF_E_NET_CLIENT_CLOSE);
    HANDLE_MF_ERROR(MF_E_NET_BAD_CONTROL_DATA);
    HANDLE_MF_ERROR(MF_E_NET_INCOMPATIBLE_SERVER);
    HANDLE_MF_ERROR(MF_E_NET_UNSAFE_URL);
    HANDLE_MF_ERROR(MF_E_NET_CACHE_NO_DATA);
    HANDLE_MF_ERROR(MF_E_NET_EOL);
    HANDLE_MF_ERROR(MF_E_NET_BAD_REQUEST);
    HANDLE_MF_ERROR(MF_E_NET_INTERNAL_SERVER_ERROR);
    HANDLE_MF_ERROR(MF_E_NET_SESSION_NOT_FOUND);
    HANDLE_MF_ERROR(MF_E_NET_NOCONNECTION);
    HANDLE_MF_ERROR(MF_E_NET_CONNECTION_FAILURE);
    HANDLE_MF_ERROR(MF_E_NET_INCOMPATIBLE_PUSHSERVER);
    HANDLE_MF_ERROR(MF_E_NET_SERVER_ACCESSDENIED);
    HANDLE_MF_ERROR(MF_E_NET_PROXY_ACCESSDENIED);
    HANDLE_MF_ERROR(MF_E_NET_CANNOTCONNECT);
    HANDLE_MF_ERROR(MF_E_NET_INVALID_PUSH_TEMPLATE);
    HANDLE_MF_ERROR(MF_E_NET_INVALID_PUSH_PUBLISHING_POINT);
    HANDLE_MF_ERROR(MF_E_NET_BUSY);
    HANDLE_MF_ERROR(MF_E_NET_RESOURCE_GONE);
    HANDLE_MF_ERROR(MF_E_NET_ERROR_FROM_PROXY);
    HANDLE_MF_ERROR(MF_E_NET_PROXY_TIMEOUT);
    HANDLE_MF_ERROR(MF_E_NET_SERVER_UNAVAILABLE);
    HANDLE_MF_ERROR(MF_E_NET_TOO_MUCH_DATA);
    HANDLE_MF_ERROR(MF_E_NET_SESSION_INVALID);
    HANDLE_MF_ERROR(MF_E_OFFLINE_MODE);
    HANDLE_MF_ERROR(MF_E_NET_UDP_BLOCKED);
    HANDLE_MF_ERROR(MF_E_NET_UNSUPPORTED_CONFIGURATION);
    HANDLE_MF_ERROR(MF_E_NET_PROTOCOL_DISABLED);
    HANDLE_MF_ERROR(MF_E_NET_COMPANION_DRIVER_DISCONNECT);
    HANDLE_MF_ERROR(MF_E_ALREADY_INITIALIZED);
    HANDLE_MF_ERROR(MF_E_BANDWIDTH_OVERRUN);
    HANDLE_MF_ERROR(MF_E_LATE_SAMPLE);
    HANDLE_MF_ERROR(MF_E_FLUSH_NEEDED);
    HANDLE_MF_ERROR(MF_E_INVALID_PROFILE);
    HANDLE_MF_ERROR(MF_E_INDEX_NOT_COMMITTED);
    HANDLE_MF_ERROR(MF_E_NO_INDEX);
    HANDLE_MF_ERROR(MF_E_CANNOT_INDEX_IN_PLACE);
    HANDLE_MF_ERROR(MF_E_MISSING_ASF_LEAKYBUCKET);
    HANDLE_MF_ERROR(MF_E_INVALID_ASF_STREAMID);
    HANDLE_MF_ERROR(MF_E_STREAMSINK_REMOVED);
    HANDLE_MF_ERROR(MF_E_STREAMSINKS_OUT_OF_SYNC);
    HANDLE_MF_ERROR(MF_E_STREAMSINKS_FIXED);
    HANDLE_MF_ERROR(MF_E_STREAMSINK_EXISTS);
    HANDLE_MF_ERROR(MF_E_SAMPLEALLOCATOR_CANCELED);
    HANDLE_MF_ERROR(MF_E_SAMPLEALLOCATOR_EMPTY);
    HANDLE_MF_ERROR(MF_E_SINK_ALREADYSTOPPED);
    HANDLE_MF_ERROR(MF_E_ASF_FILESINK_BITRATE_UNKNOWN);
    HANDLE_MF_ERROR(MF_E_SINK_NO_STREAMS);
    HANDLE_MF_ERROR(MF_S_SINK_NOT_FINALIZED);
    HANDLE_MF_ERROR(MF_E_METADATA_TOO_LONG);
    HANDLE_MF_ERROR(MF_E_SINK_NO_SAMPLES_PROCESSED);
    HANDLE_MF_ERROR(MF_E_SINK_HEADERS_NOT_FOUND);
    HANDLE_MF_ERROR(MF_E_VIDEO_REN_NO_PROCAMP_HW);
    HANDLE_MF_ERROR(MF_E_VIDEO_REN_NO_DEINTERLACE_HW);
    HANDLE_MF_ERROR(MF_E_VIDEO_REN_COPYPROT_FAILED);
    HANDLE_MF_ERROR(MF_E_VIDEO_REN_SURFACE_NOT_SHARED);
    HANDLE_MF_ERROR(MF_E_VIDEO_DEVICE_LOCKED);
    HANDLE_MF_ERROR(MF_E_NEW_VIDEO_DEVICE);
    HANDLE_MF_ERROR(MF_E_NO_VIDEO_SAMPLE_AVAILABLE);
    HANDLE_MF_ERROR(MF_E_NO_AUDIO_PLAYBACK_DEVICE);
    HANDLE_MF_ERROR(MF_E_AUDIO_PLAYBACK_DEVICE_IN_USE);
    HANDLE_MF_ERROR(MF_E_AUDIO_PLAYBACK_DEVICE_INVALIDATED);
    HANDLE_MF_ERROR(MF_E_AUDIO_SERVICE_NOT_RUNNING);
    HANDLE_MF_ERROR(MF_E_AUDIO_BUFFER_SIZE_ERROR);
    HANDLE_MF_ERROR(MF_E_AUDIO_CLIENT_WRAPPER_SPOOF_ERROR);
    HANDLE_MF_ERROR(MF_E_TOPO_INVALID_OPTIONAL_NODE);
    HANDLE_MF_ERROR(MF_E_TOPO_CANNOT_FIND_DECRYPTOR);
    HANDLE_MF_ERROR(MF_E_TOPO_CODEC_NOT_FOUND);
    HANDLE_MF_ERROR(MF_E_TOPO_CANNOT_CONNECT);
    HANDLE_MF_ERROR(MF_E_TOPO_UNSUPPORTED);
    HANDLE_MF_ERROR(MF_E_TOPO_INVALID_TIME_ATTRIBUTES);
    HANDLE_MF_ERROR(MF_E_TOPO_LOOPS_IN_TOPOLOGY);
    HANDLE_MF_ERROR(MF_E_TOPO_MISSING_PRESENTATION_DESCRIPTOR);
    HANDLE_MF_ERROR(MF_E_TOPO_MISSING_STREAM_DESCRIPTOR);
    HANDLE_MF_ERROR(MF_E_TOPO_STREAM_DESCRIPTOR_NOT_SELECTED);
    HANDLE_MF_ERROR(MF_E_TOPO_MISSING_SOURCE);
    HANDLE_MF_ERROR(MF_E_TOPO_SINK_ACTIVATES_UNSUPPORTED);
    HANDLE_MF_ERROR(MF_E_SEQUENCER_UNKNOWN_SEGMENT_ID);
    HANDLE_MF_ERROR(MF_S_SEQUENCER_CONTEXT_CANCELED);
    HANDLE_MF_ERROR(MF_E_NO_SOURCE_IN_CACHE);
    HANDLE_MF_ERROR(MF_S_SEQUENCER_SEGMENT_AT_END_OF_STREAM);
    HANDLE_MF_ERROR(MF_E_TRANSFORM_TYPE_NOT_SET);
    HANDLE_MF_ERROR(MF_E_TRANSFORM_STREAM_CHANGE);
    HANDLE_MF_ERROR(MF_E_TRANSFORM_INPUT_REMAINING);
    HANDLE_MF_ERROR(MF_E_TRANSFORM_PROFILE_MISSING);
    HANDLE_MF_ERROR(MF_E_TRANSFORM_PROFILE_INVALID_OR_CORRUPT);
    HANDLE_MF_ERROR(MF_E_TRANSFORM_PROFILE_TRUNCATED);
    HANDLE_MF_ERROR(MF_E_TRANSFORM_PROPERTY_PID_NOT_RECOGNIZED);
    HANDLE_MF_ERROR(MF_E_TRANSFORM_PROPERTY_VARIANT_TYPE_WRONG);
    HANDLE_MF_ERROR(MF_E_TRANSFORM_PROPERTY_NOT_WRITEABLE);
    HANDLE_MF_ERROR(MF_E_TRANSFORM_PROPERTY_ARRAY_VALUE_WRONG_NUM_DIM);
    HANDLE_MF_ERROR(MF_E_TRANSFORM_PROPERTY_VALUE_SIZE_WRONG);
    HANDLE_MF_ERROR(MF_E_TRANSFORM_PROPERTY_VALUE_OUT_OF_RANGE);
    HANDLE_MF_ERROR(MF_E_TRANSFORM_PROPERTY_VALUE_INCOMPATIBLE);
    HANDLE_MF_ERROR(MF_E_TRANSFORM_NOT_POSSIBLE_FOR_CURRENT_OUTPUT_MEDIATYPE);
    HANDLE_MF_ERROR(MF_E_TRANSFORM_NOT_POSSIBLE_FOR_CURRENT_INPUT_MEDIATYPE);
    HANDLE_MF_ERROR(MF_E_TRANSFORM_NOT_POSSIBLE_FOR_CURRENT_MEDIATYPE_COMBINATION);
    HANDLE_MF_ERROR(MF_E_TRANSFORM_CONFLICTS_WITH_OTHER_CURRENTLY_ENABLED_FEATURES);
    HANDLE_MF_ERROR(MF_E_TRANSFORM_NEED_MORE_INPUT);
    HANDLE_MF_ERROR(MF_E_TRANSFORM_NOT_POSSIBLE_FOR_CURRENT_SPKR_CONFIG);
    HANDLE_MF_ERROR(MF_E_TRANSFORM_CANNOT_CHANGE_MEDIATYPE_WHILE_PROCESSING);
    HANDLE_MF_ERROR(MF_S_TRANSFORM_DO_NOT_PROPAGATE_EVENT);
    HANDLE_MF_ERROR(MF_E_UNSUPPORTED_D3D_TYPE);
    HANDLE_MF_ERROR(MF_E_TRANSFORM_ASYNC_LOCKED);
    HANDLE_MF_ERROR(MF_E_TRANSFORM_CANNOT_INITIALIZE_ACM_DRIVER);
    HANDLE_MF_ERROR(MF_E_TRANSFORM_STREAM_INVALID_RESOLUTION);
    HANDLE_MF_ERROR(MF_E_TRANSFORM_ASYNC_MFT_NOT_SUPPORTED);
    HANDLE_MF_ERROR(MF_E_TRANSFORM_EXATTRIBUTE_NOT_SUPPORTED);
    HANDLE_MF_ERROR(MF_E_LICENSE_INCORRECT_RIGHTS);
    HANDLE_MF_ERROR(MF_E_LICENSE_OUTOFDATE);
    HANDLE_MF_ERROR(MF_E_LICENSE_REQUIRED);
    HANDLE_MF_ERROR(MF_E_DRM_HARDWARE_INCONSISTENT);
    HANDLE_MF_ERROR(MF_E_NO_CONTENT_PROTECTION_MANAGER);
    HANDLE_MF_ERROR(MF_E_LICENSE_RESTORE_NO_RIGHTS);
    HANDLE_MF_ERROR(MF_E_BACKUP_RESTRICTED_LICENSE);
    HANDLE_MF_ERROR(MF_E_LICENSE_RESTORE_NEEDS_INDIVIDUALIZATION);
    HANDLE_MF_ERROR(MF_S_PROTECTION_NOT_REQUIRED);
    HANDLE_MF_ERROR(MF_E_COMPONENT_REVOKED);
    HANDLE_MF_ERROR(MF_E_TRUST_DISABLED);
    HANDLE_MF_ERROR(MF_E_WMDRMOTA_NO_ACTION);
    HANDLE_MF_ERROR(MF_E_WMDRMOTA_ACTION_ALREADY_SET);
    HANDLE_MF_ERROR(MF_E_WMDRMOTA_DRM_HEADER_NOT_AVAILABLE);
    HANDLE_MF_ERROR(MF_E_WMDRMOTA_DRM_ENCRYPTION_SCHEME_NOT_SUPPORTED);
    HANDLE_MF_ERROR(MF_E_WMDRMOTA_ACTION_MISMATCH);
    HANDLE_MF_ERROR(MF_E_WMDRMOTA_INVALID_POLICY);
    HANDLE_MF_ERROR(MF_E_POLICY_UNSUPPORTED);
    HANDLE_MF_ERROR(MF_E_OPL_NOT_SUPPORTED);
    HANDLE_MF_ERROR(MF_E_TOPOLOGY_VERIFICATION_FAILED);
    HANDLE_MF_ERROR(MF_E_SIGNATURE_VERIFICATION_FAILED);
    HANDLE_MF_ERROR(MF_E_DEBUGGING_NOT_ALLOWED);
    HANDLE_MF_ERROR(MF_E_CODE_EXPIRED);
    HANDLE_MF_ERROR(MF_E_GRL_VERSION_TOO_LOW);
    HANDLE_MF_ERROR(MF_E_GRL_RENEWAL_NOT_FOUND);
    HANDLE_MF_ERROR(MF_E_GRL_EXTENSIBLE_ENTRY_NOT_FOUND);
    HANDLE_MF_ERROR(MF_E_KERNEL_UNTRUSTED);
    HANDLE_MF_ERROR(MF_E_PEAUTH_UNTRUSTED);
    HANDLE_MF_ERROR(MF_E_NON_PE_PROCESS);
    HANDLE_MF_ERROR(MF_E_REBOOT_REQUIRED);
    HANDLE_MF_ERROR(MF_S_WAIT_FOR_POLICY_SET);
    HANDLE_MF_ERROR(MF_S_VIDEO_DISABLED_WITH_UNKNOWN_SOFTWARE_OUTPUT);
    HANDLE_MF_ERROR(MF_E_GRL_INVALID_FORMAT);
    HANDLE_MF_ERROR(MF_E_GRL_UNRECOGNIZED_FORMAT);
    HANDLE_MF_ERROR(MF_E_ALL_PROCESS_RESTART_REQUIRED);
    HANDLE_MF_ERROR(MF_E_PROCESS_RESTART_REQUIRED);
    HANDLE_MF_ERROR(MF_E_USERMODE_UNTRUSTED);
    HANDLE_MF_ERROR(MF_E_PEAUTH_SESSION_NOT_STARTED);
    HANDLE_MF_ERROR(MF_E_PEAUTH_PUBLICKEY_REVOKED);
    HANDLE_MF_ERROR(MF_E_GRL_ABSENT);
    HANDLE_MF_ERROR(MF_S_PE_TRUSTED);
    HANDLE_MF_ERROR(MF_E_PE_UNTRUSTED);
    HANDLE_MF_ERROR(MF_E_PEAUTH_NOT_STARTED);
    HANDLE_MF_ERROR(MF_E_INCOMPATIBLE_SAMPLE_PROTECTION);
    HANDLE_MF_ERROR(MF_E_PE_SESSIONS_MAXED);
    HANDLE_MF_ERROR(MF_E_HIGH_SECURITY_LEVEL_CONTENT_NOT_ALLOWED);
    HANDLE_MF_ERROR(MF_E_TEST_SIGNED_COMPONENTS_NOT_ALLOWED);
    HANDLE_MF_ERROR(MF_E_ITA_UNSUPPORTED_ACTION);
    HANDLE_MF_ERROR(MF_E_ITA_ERROR_PARSING_SAP_PARAMETERS);
    HANDLE_MF_ERROR(MF_E_POLICY_MGR_ACTION_OUTOFBOUNDS);
    HANDLE_MF_ERROR(MF_E_BAD_OPL_STRUCTURE_FORMAT);
    HANDLE_MF_ERROR(MF_E_ITA_UNRECOGNIZED_ANALOG_VIDEO_PROTECTION_GUID);
    HANDLE_MF_ERROR(MF_E_NO_PMP_HOST);
    HANDLE_MF_ERROR(MF_E_ITA_OPL_DATA_NOT_INITIALIZED);
    HANDLE_MF_ERROR(MF_E_ITA_UNRECOGNIZED_ANALOG_VIDEO_OUTPUT);
    HANDLE_MF_ERROR(MF_E_ITA_UNRECOGNIZED_DIGITAL_VIDEO_OUTPUT);
    HANDLE_MF_ERROR(MF_E_RESOLUTION_REQUIRES_PMP_CREATION_CALLBACK);
    HANDLE_MF_ERROR(MF_E_INVALID_AKE_CHANNEL_PARAMETERS);
    HANDLE_MF_ERROR(MF_E_CONTENT_PROTECTION_SYSTEM_NOT_ENABLED);
    HANDLE_MF_ERROR(MF_E_UNSUPPORTED_CONTENT_PROTECTION_SYSTEM);
    HANDLE_MF_ERROR(MF_E_DRM_MIGRATION_NOT_SUPPORTED);
    HANDLE_MF_ERROR(MF_E_HDCP_AUTHENTICATION_FAILURE);
    HANDLE_MF_ERROR(MF_E_HDCP_LINK_FAILURE);
    HANDLE_MF_ERROR(MF_E_CLOCK_INVALID_CONTINUITY_KEY);
    HANDLE_MF_ERROR(MF_E_CLOCK_NO_TIME_SOURCE);
    HANDLE_MF_ERROR(MF_E_CLOCK_STATE_ALREADY_SET);
    HANDLE_MF_ERROR(MF_E_CLOCK_NOT_SIMPLE);
    HANDLE_MF_ERROR(MF_S_CLOCK_STOPPED);
    HANDLE_MF_ERROR(MF_E_CLOCK_AUDIO_DEVICE_POSITION_UNEXPECTED);
    HANDLE_MF_ERROR(MF_E_CLOCK_AUDIO_RENDER_POSITION_UNEXPECTED);
    HANDLE_MF_ERROR(MF_E_CLOCK_AUDIO_RENDER_TIME_UNEXPECTED);
    HANDLE_MF_ERROR(MF_E_NO_MORE_DROP_MODES);
    HANDLE_MF_ERROR(MF_E_NO_MORE_QUALITY_LEVELS);
    HANDLE_MF_ERROR(MF_E_DROPTIME_NOT_SUPPORTED);
    HANDLE_MF_ERROR(MF_E_QUALITYKNOB_WAIT_LONGER);
    HANDLE_MF_ERROR(MF_E_QM_INVALIDSTATE);
    HANDLE_MF_ERROR(MF_E_TRANSCODE_NO_CONTAINERTYPE);
    HANDLE_MF_ERROR(MF_E_TRANSCODE_PROFILE_NO_MATCHING_STREAMS);
    HANDLE_MF_ERROR(MF_E_TRANSCODE_NO_MATCHING_ENCODER);
    HANDLE_MF_ERROR(MF_E_TRANSCODE_INVALID_PROFILE);
    HANDLE_MF_ERROR(MF_E_ALLOCATOR_NOT_INITIALIZED);
    HANDLE_MF_ERROR(MF_E_ALLOCATOR_NOT_COMMITED);
    HANDLE_MF_ERROR(MF_E_ALLOCATOR_ALREADY_COMMITED);
    HANDLE_MF_ERROR(MF_E_STREAM_ERROR);
    HANDLE_MF_ERROR(MF_E_INVALID_STREAM_STATE);
    HANDLE_MF_ERROR(MF_E_HW_STREAM_NOT_CONNECTED);
    HANDLE_MF_ERROR(MF_E_NO_CAPTURE_DEVICES_AVAILABLE);
    HANDLE_MF_ERROR(MF_E_CAPTURE_SINK_OUTPUT_NOT_SET);
    HANDLE_MF_ERROR(MF_E_CAPTURE_SINK_MIRROR_ERROR);
    HANDLE_MF_ERROR(MF_E_CAPTURE_SINK_ROTATE_ERROR);
    HANDLE_MF_ERROR(MF_E_CAPTURE_ENGINE_INVALID_OP);
    HANDLE_MF_ERROR(MF_E_CAPTURE_ENGINE_ALL_EFFECTS_REMOVED);
    HANDLE_MF_ERROR(MF_E_CAPTURE_SOURCE_NO_INDEPENDENT_PHOTO_STREAM_PRESENT);
    HANDLE_MF_ERROR(MF_E_CAPTURE_SOURCE_NO_VIDEO_STREAM_PRESENT);
    HANDLE_MF_ERROR(MF_E_CAPTURE_SOURCE_NO_AUDIO_STREAM_PRESENT);
    HANDLE_MF_ERROR(MF_E_CAPTURE_SOURCE_DEVICE_EXTENDEDPROP_OP_IN_PROGRESS);
    HANDLE_MF_ERROR(MF_E_CAPTURE_PROPERTY_SET_DURING_PHOTO);
    HANDLE_MF_ERROR(MF_E_CAPTURE_NO_SAMPLES_IN_QUEUE);
    HANDLE_MF_ERROR(MF_E_HW_ACCELERATED_THUMBNAIL_NOT_SUPPORTED);
    HANDLE_MF_ERROR(MF_E_UNSUPPORTED_CAPTURE_DEVICE_PRESENT);
    HANDLE_MF_ERROR(MF_E_TIMELINECONTROLLER_UNSUPPORTED_SOURCE_TYPE);
    HANDLE_MF_ERROR(MF_E_TIMELINECONTROLLER_NOT_ALLOWED);
    HANDLE_MF_ERROR(MF_E_TIMELINECONTROLLER_CANNOT_ATTACH);
    HANDLE_MF_ERROR(MF_E_MEDIA_EXTENSION_APPSERVICE_CONNECTION_FAILED);
    HANDLE_MF_ERROR(MF_E_MEDIA_EXTENSION_APPSERVICE_REQUEST_FAILED);
    HANDLE_MF_ERROR(MF_E_MEDIA_EXTENSION_PACKAGE_INTEGRITY_CHECK_FAILED);
    HANDLE_MF_ERROR(MF_E_MEDIA_EXTENSION_PACKAGE_LICENSE_INVALID);
    HANDLE_MF_ERROR(MF_INDEX_SIZE_ERR);
    HANDLE_MF_ERROR(MF_NOT_FOUND_ERR);
    HANDLE_MF_ERROR(MF_NOT_SUPPORTED_ERR);
    HANDLE_MF_ERROR(MF_INVALID_STATE_ERR);
    HANDLE_MF_ERROR(MF_SYNTAX_ERR);
    HANDLE_MF_ERROR(MF_INVALID_ACCESS_ERR);
    HANDLE_MF_ERROR(MF_QUOTA_EXCEEDED_ERR);
    HANDLE_MF_ERROR(MF_PARSE_ERR);
    HANDLE_MF_ERROR(MF_TYPE_ERR);
    HANDLE_MF_ERROR(DXGI_STATUS_OCCLUDED);
    HANDLE_MF_ERROR(DXGI_STATUS_CLIPPED);
    HANDLE_MF_ERROR(DXGI_STATUS_NO_REDIRECTION);
    HANDLE_MF_ERROR(DXGI_STATUS_NO_DESKTOP_ACCESS);
    HANDLE_MF_ERROR(DXGI_STATUS_GRAPHICS_VIDPN_SOURCE_IN_USE);
    HANDLE_MF_ERROR(DXGI_STATUS_MODE_CHANGED);
    HANDLE_MF_ERROR(DXGI_STATUS_MODE_CHANGE_IN_PROGRESS);
    HANDLE_MF_ERROR(DXGI_ERROR_INVALID_CALL);
    HANDLE_MF_ERROR(DXGI_ERROR_NOT_FOUND);
    HANDLE_MF_ERROR(DXGI_ERROR_MORE_DATA);
    HANDLE_MF_ERROR(DXGI_ERROR_UNSUPPORTED);
    HANDLE_MF_ERROR(DXGI_ERROR_DEVICE_REMOVED);
    HANDLE_MF_ERROR(DXGI_ERROR_DEVICE_HUNG);
    HANDLE_MF_ERROR(DXGI_ERROR_DEVICE_RESET);
    HANDLE_MF_ERROR(DXGI_ERROR_WAS_STILL_DRAWING);
    HANDLE_MF_ERROR(DXGI_ERROR_FRAME_STATISTICS_DISJOINT);
    HANDLE_MF_ERROR(DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE);
    HANDLE_MF_ERROR(DXGI_ERROR_DRIVER_INTERNAL_ERROR);
    HANDLE_MF_ERROR(DXGI_ERROR_NONEXCLUSIVE);
    HANDLE_MF_ERROR(DXGI_ERROR_NOT_CURRENTLY_AVAILABLE);
    HANDLE_MF_ERROR(DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED);
    HANDLE_MF_ERROR(DXGI_ERROR_REMOTE_OUTOFMEMORY);
    HANDLE_MF_ERROR(DXGI_ERROR_ACCESS_LOST);
    HANDLE_MF_ERROR(DXGI_ERROR_WAIT_TIMEOUT);
    HANDLE_MF_ERROR(DXGI_ERROR_SESSION_DISCONNECTED);
    HANDLE_MF_ERROR(DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE);
    HANDLE_MF_ERROR(DXGI_ERROR_CANNOT_PROTECT_CONTENT);
    HANDLE_MF_ERROR(DXGI_ERROR_ACCESS_DENIED);
    HANDLE_MF_ERROR(DXGI_ERROR_NAME_ALREADY_EXISTS);
    HANDLE_MF_ERROR(DXGI_ERROR_SDK_COMPONENT_MISSING);
    HANDLE_MF_ERROR(DXGI_ERROR_NOT_CURRENT);
    HANDLE_MF_ERROR(DXGI_ERROR_HW_PROTECTION_OUTOFMEMORY);
    HANDLE_MF_ERROR(DXGI_ERROR_DYNAMIC_CODE_POLICY_VIOLATION);
    HANDLE_MF_ERROR(DXGI_ERROR_NON_COMPOSITED_UI);
    HANDLE_MF_ERROR(DXCORE_ERROR_EVENT_NOT_UNREGISTERED);
#ifdef PRESENTATION_ERROR_LOST
    HANDLE_MF_ERROR(PRESENTATION_ERROR_LOST);
#endif
    HANDLE_MF_ERROR(DXGI_STATUS_UNOCCLUDED);
    HANDLE_MF_ERROR(DXGI_STATUS_DDA_WAS_STILL_DRAWING);
    HANDLE_MF_ERROR(DXGI_ERROR_MODE_CHANGE_IN_PROGRESS);
    HANDLE_MF_ERROR(DXGI_STATUS_PRESENT_REQUIRED);
    HANDLE_MF_ERROR(DXGI_ERROR_CACHE_CORRUPT);
    HANDLE_MF_ERROR(DXGI_ERROR_CACHE_FULL);
    HANDLE_MF_ERROR(DXGI_ERROR_CACHE_HASH_COLLISION);
    HANDLE_MF_ERROR(DXGI_ERROR_ALREADY_EXISTS);
    HANDLE_MF_ERROR(DXGI_DDI_ERR_WASSTILLDRAWING);
    HANDLE_MF_ERROR(DXGI_DDI_ERR_UNSUPPORTED);
    HANDLE_MF_ERROR(DXGI_DDI_ERR_NONEXCLUSIVE);
    HANDLE_MF_ERROR(D3D10_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS);
    HANDLE_MF_ERROR(D3D10_ERROR_FILE_NOT_FOUND);
    HANDLE_MF_ERROR(D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS);
    HANDLE_MF_ERROR(D3D11_ERROR_FILE_NOT_FOUND);
    HANDLE_MF_ERROR(D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS);
    HANDLE_MF_ERROR(D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD);
    HANDLE_MF_ERROR(D3D12_ERROR_ADAPTER_NOT_FOUND);
    HANDLE_MF_ERROR(D3D12_ERROR_DRIVER_VERSION_MISMATCH);
#ifdef D3D12_ERROR_INVALID_REDIST
    HANDLE_MF_ERROR(D3D12_ERROR_INVALID_REDIST);
#endif
    HANDLE_MF_ERROR(D2DERR_WRONG_STATE);
    HANDLE_MF_ERROR(D2DERR_NOT_INITIALIZED);
    HANDLE_MF_ERROR(D2DERR_UNSUPPORTED_OPERATION);
    HANDLE_MF_ERROR(D2DERR_SCANNER_FAILED);
    HANDLE_MF_ERROR(D2DERR_SCREEN_ACCESS_DENIED);
    HANDLE_MF_ERROR(D2DERR_DISPLAY_STATE_INVALID);
    HANDLE_MF_ERROR(D2DERR_ZERO_VECTOR);
    HANDLE_MF_ERROR(D2DERR_INTERNAL_ERROR);
    HANDLE_MF_ERROR(D2DERR_DISPLAY_FORMAT_NOT_SUPPORTED);
    HANDLE_MF_ERROR(D2DERR_INVALID_CALL);
    HANDLE_MF_ERROR(D2DERR_NO_HARDWARE_DEVICE);
    HANDLE_MF_ERROR(D2DERR_RECREATE_TARGET);
    HANDLE_MF_ERROR(D2DERR_TOO_MANY_SHADER_ELEMENTS);
    HANDLE_MF_ERROR(D2DERR_SHADER_COMPILE_FAILED);
    HANDLE_MF_ERROR(D2DERR_MAX_TEXTURE_SIZE_EXCEEDED);
    HANDLE_MF_ERROR(D2DERR_UNSUPPORTED_VERSION);
    HANDLE_MF_ERROR(D2DERR_BAD_NUMBER);
    HANDLE_MF_ERROR(D2DERR_WRONG_FACTORY);
    HANDLE_MF_ERROR(D2DERR_LAYER_ALREADY_IN_USE);
    HANDLE_MF_ERROR(D2DERR_POP_CALL_DID_NOT_MATCH_PUSH);
    HANDLE_MF_ERROR(D2DERR_WRONG_RESOURCE_DOMAIN);
    HANDLE_MF_ERROR(D2DERR_PUSH_POP_UNBALANCED);
    HANDLE_MF_ERROR(D2DERR_RENDER_TARGET_HAS_LAYER_OR_CLIPRECT);
    HANDLE_MF_ERROR(D2DERR_INCOMPATIBLE_BRUSH_TYPES);
    HANDLE_MF_ERROR(D2DERR_WIN32_ERROR);
    HANDLE_MF_ERROR(D2DERR_TARGET_NOT_GDI_COMPATIBLE);
    HANDLE_MF_ERROR(D2DERR_TEXT_EFFECT_IS_WRONG_TYPE);
    HANDLE_MF_ERROR(D2DERR_TEXT_RENDERER_NOT_RELEASED);
    HANDLE_MF_ERROR(D2DERR_EXCEEDS_MAX_BITMAP_SIZE);
    HANDLE_MF_ERROR(D2DERR_INVALID_GRAPH_CONFIGURATION);
    HANDLE_MF_ERROR(D2DERR_INVALID_INTERNAL_GRAPH_CONFIGURATION);
    HANDLE_MF_ERROR(D2DERR_CYCLIC_GRAPH);
    HANDLE_MF_ERROR(D2DERR_BITMAP_CANNOT_DRAW);
    HANDLE_MF_ERROR(D2DERR_OUTSTANDING_BITMAP_REFERENCES);
    HANDLE_MF_ERROR(D2DERR_ORIGINAL_TARGET_NOT_BOUND);
    HANDLE_MF_ERROR(D2DERR_INVALID_TARGET);
    HANDLE_MF_ERROR(D2DERR_BITMAP_BOUND_AS_TARGET);
    HANDLE_MF_ERROR(D2DERR_INSUFFICIENT_DEVICE_CAPABILITIES);
    HANDLE_MF_ERROR(D2DERR_INTERMEDIATE_TOO_LARGE);
    HANDLE_MF_ERROR(D2DERR_EFFECT_IS_NOT_REGISTERED);
    HANDLE_MF_ERROR(D2DERR_INVALID_PROPERTY);
    HANDLE_MF_ERROR(D2DERR_NO_SUBPROPERTIES);
    HANDLE_MF_ERROR(D2DERR_PRINT_JOB_CLOSED);
    HANDLE_MF_ERROR(D2DERR_PRINT_FORMAT_NOT_SUPPORTED);
    HANDLE_MF_ERROR(D2DERR_TOO_MANY_TRANSFORM_INPUTS);
    HANDLE_MF_ERROR(D2DERR_INVALID_GLYPH_IMAGE);
    HANDLE_MF_ERROR(E_INVALIDARG);
    HANDLE_MF_ERROR(E_UNEXPECTED);
    HANDLE_MF_ERROR(E_NOTIMPL);
    HANDLE_MF_ERROR(E_OUTOFMEMORY);
    HANDLE_MF_ERROR(E_NOINTERFACE);
    HANDLE_MF_ERROR(E_POINTER);
    HANDLE_MF_ERROR(E_HANDLE);
    HANDLE_MF_ERROR(E_ABORT);
    HANDLE_MF_ERROR(E_FAIL);
    HANDLE_MF_ERROR(E_ACCESSDENIED);
    return "Unknown error code.";
}

#endif
