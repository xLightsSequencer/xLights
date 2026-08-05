/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "WindowsHardwareVideoReader.h"

#ifdef _WIN32

//#define DETAILED_LOGGING

#include <winsock2.h>
#include <mferror.h>
#include <propvarutil.h>
#include <wincodec.h>
#include <Wmcodecdsp.h>
#include <objbase.h>
#include <cassert>
#include "Parallel.h"
#include <winerror.h>
#include <d3d11_4.h>

#include <log.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <climits>
#include <map>
#include <string>
#include <thread>

// How long a single ReadSample may take before the reader is written off.
// Generous on purpose: a first decode of a large frame on a box already running
// a dozen decoders is slow, but it is not minutes. Overridable for testing.
static DWORD ReadSampleTimeoutMS()
{
    static const DWORD ms = []() -> DWORD {
        const char* e = getenv("XL_MF_READ_TIMEOUT_MS");
        if (e != nullptr) {
            long v = strtol(e, nullptr, 10);
            if (v > 0)
                return (DWORD)v;
        }
        return 10000;
    }();
    return ms;
}

// Media Foundation stops responding when too many decoders run at once, and it
// recovers when the load goes away. So a missed deadline stands hardware decode
// down for a while rather than for good: switching it off permanently would
// cost a batch render its remaining sequences, and a long-running session the
// rest of its day, over one busy moment. Each further timeout doubles the wait,
// so a box where this keeps happening quickly stops paying the deadline while
// one bad patch costs only the first window.
// Guards the cooldown window and the learned decoder limit.
static std::mutex __mfStateMutex;
static std::chrono::steady_clock::time_point __mfCooldownUntil{};
static int __mfTimeouts = 0;

static constexpr int MF_COOLDOWN_BASE_MS = 30000;
static constexpr int MF_COOLDOWN_MAX_MS = 600000;

// How many hardware decoders may be open at once.
//
// Nothing reports this number: D3D11 and Media Foundation describe the formats
// and profiles a decoder supports, never how many sessions the driver will
// actually service, so it can only be learned by exceeding it. Start unlimited,
// and each time decode fails with several readers open, cap it below the number
// that were live at the time and step down again if that still fails. Readers
// beyond the cap decode in software, which is slower but always works.
static std::atomic<int> __mfActiveReaders{ 0 };
static int __mfMaxReaders = INT_MAX;

// Below this, a failure is about the file rather than the load, and capping
// concurrency would be treating the wrong cause.
static constexpr int MF_MIN_CONCURRENCY_TO_BLAME = 3;
static constexpr int MF_CAP_STEP = 2;
// One shortage produces a burst of failures; only the first should count.
static constexpr int MF_CAP_SETTLE_MS = 2000;
static std::chrono::steady_clock::time_point __mfLastCapChange{};
static std::chrono::steady_clock::time_point __mfLastFailure{};

// After a long clean spell, try one more decoder than the learned limit. What
// forced the limit down is usually a busy moment rather than a hard ceiling, and
// without this a single bad patch early on would hold a long session or a batch
// render below capacity for ever. Down by two and back up by one, so a limit
// that keeps failing settles just under whatever the driver really allows
// instead of oscillating across it.
static constexpr int MF_CAP_RECOVER_MS = 120000;
static constexpr int MF_CAP_RECOVER_STEP = 1;

// Give a lowered limit back one decoder once nothing has gone wrong for a while.
// Called on the open path, which is where a raised limit can be put to use.
static void MaybeRecoverCap()
{
    std::lock_guard<std::mutex> lock(__mfStateMutex);
    if (__mfMaxReaders == INT_MAX) {
        return; // never lowered, nothing to give back
    }
    const auto now = std::chrono::steady_clock::now();
    if (now - __mfLastFailure < std::chrono::milliseconds(MF_CAP_RECOVER_MS) ||
        now - __mfLastCapChange < std::chrono::milliseconds(MF_CAP_RECOVER_MS)) {
        return;
    }
    __mfMaxReaders += MF_CAP_RECOVER_STEP;
    __mfLastCapChange = now;
    spdlog::info("WHVD: no decode trouble for {}s - trying {} hardware decoders at a time",
                 MF_CAP_RECOVER_MS / 1000, __mfMaxReaders);
}

bool WindowsHardwareVideoReader::TryReserveDecoder()
{
    MaybeRecoverCap();

    int active = __mfActiveReaders.load(std::memory_order_acquire);
    for (;;) {
        int cap;
        {
            std::lock_guard<std::mutex> lock(__mfStateMutex);
            cap = __mfMaxReaders;
        }
        if (active >= cap) {
            return false;
        }
        if (__mfActiveReaders.compare_exchange_weak(active, active + 1,
                                                    std::memory_order_acq_rel, std::memory_order_acquire)) {
            return true;
        }
    }
}

void WindowsHardwareVideoReader::ReleaseDecoder()
{
    if (__mfActiveReaders.fetch_sub(1, std::memory_order_acq_rel) == 1) {
        // Last reader out. Nothing can be contending for a decoder any more, so
        // whatever the cooldown was waiting to subside has subsided - end it now
        // rather than make the next sequence sit out the remainder. A wedged
        // reader never gets here, because its slot is only given back once the
        // release that is stuck actually returns, so a real wedge still holds
        // the cooldown open. The learned cap is deliberately kept.
        std::lock_guard<std::mutex> lock(__mfStateMutex);
        __mfCooldownUntil = std::chrono::steady_clock::time_point{};
    }
}

// Hardware decode just failed with `observed` readers open. If that is enough
// readers for contention to be the explanation, learn from it.
//
// Readers run out of decoders in groups, so one shortage arrives as a burst of
// failures milliseconds apart. Each one is the same piece of evidence, and
// letting them all step the limit would take it from unlimited to 1 before the
// first reduction had been tried: measured, four failures in 53ms did exactly
// that. One reduction per settling window, then see whether it was enough.
static void NoteHardwareFailure(int observed, const std::string& filename)
{
    if (observed < MF_MIN_CONCURRENCY_TO_BLAME) {
        return;
    }

    std::lock_guard<std::mutex> lock(__mfStateMutex);

    const auto now = std::chrono::steady_clock::now();
    // Recorded even when the limit does not move, so a steady trickle of
    // failures keeps postponing recovery.
    __mfLastFailure = now;

    if (now - __mfLastCapChange < std::chrono::milliseconds(MF_CAP_SETTLE_MS)) {
        return;
    }

    int newCap = std::max(std::min(__mfMaxReaders, observed) - MF_CAP_STEP, 1);
    if (newCap < __mfMaxReaders) {
        spdlog::warn("WHVD: hardware decode failed with {} readers open ({}) - limiting hardware decode to {} at a time",
                     observed, filename, newCap);
        __mfMaxReaders = newCap;
        __mfLastCapChange = now;
    }
}

bool WindowsHardwareVideoReader::MediaFoundationInCooldown()
{
    std::lock_guard<std::mutex> lock(__mfStateMutex);
    return std::chrono::steady_clock::now() < __mfCooldownUntil;
}

// Returns the length of the cooldown just started, for the log.
static int StartMediaFoundationCooldown()
{
    std::lock_guard<std::mutex> lock(__mfStateMutex);
    const auto now = std::chrono::steady_clock::now();
    // Readers wedge in groups; the first one to notice sets the window and the
    // rest of that group should not push it out again.
    if (now < __mfCooldownUntil) {
        return 0;
    }
    int ms = MF_COOLDOWN_BASE_MS;
    for (int i = 0; i < __mfTimeouts && ms < MF_COOLDOWN_MAX_MS; ++i) {
        ms *= 2;
    }
    ms = std::min(ms, MF_COOLDOWN_MAX_MS);
    ++__mfTimeouts;
    __mfCooldownUntil = now + std::chrono::milliseconds(ms);
    return ms;
}

// Receives the result of an asynchronous ReadSample.
//
// Lifetime is the whole point of this class. When a read misses its deadline
// the caller walks away, but the decoder may still deliver - minutes later, or
// never. The callback therefore outlives the reader that created it: the reader
// drops its reference and Media Foundation drops the last one whenever it is
// finally done. Orphan() makes any late delivery a no-op instead of a write
// through a freed reader.
class MFReadSampleCallback : public IMFSourceReaderCallback
{
    std::mutex _mutex;
    HANDLE _ready = nullptr;
    std::atomic<long> _refCount{ 1 };
    bool _orphaned = false;

    HRESULT _status = S_OK;
    DWORD _flags = 0;
    LONGLONG _timestamp = 0;
    IMFSample* _sample = nullptr;

    ~MFReadSampleCallback()
    {
        if (_sample != nullptr)
            _sample->Release();
        if (_ready != nullptr)
            ::CloseHandle(_ready);
    }

public:
    MFReadSampleCallback()
    {
        // Auto-reset: exactly one waiter is released per delivered sample.
        _ready = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
    }

    bool IsOk() const { return _ready != nullptr; }

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override
    {
        if (ppv == nullptr)
            return E_POINTER;
        if (riid == __uuidof(IMFSourceReaderCallback) || riid == __uuidof(IUnknown)) {
            *ppv = static_cast<IMFSourceReaderCallback*>(this);
            AddRef();
            return S_OK;
        }
        *ppv = nullptr;
        return E_NOINTERFACE;
    }
    STDMETHODIMP_(ULONG) AddRef() override
    {
        return (ULONG)_refCount.fetch_add(1, std::memory_order_relaxed) + 1;
    }
    STDMETHODIMP_(ULONG) Release() override
    {
        long c = _refCount.fetch_sub(1, std::memory_order_acq_rel) - 1;
        if (c == 0)
            delete this;
        return (ULONG)c;
    }

    // IMFSourceReaderCallback
    STDMETHODIMP OnReadSample(HRESULT hrStatus, DWORD, DWORD dwStreamFlags, LONGLONG llTimestamp, IMFSample* pSample) override
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_orphaned)
            return S_OK; // nobody is waiting any more
        _status = hrStatus;
        _flags = dwStreamFlags;
        _timestamp = llTimestamp;
        if (_sample != nullptr)
            _sample->Release();
        _sample = pSample;
        if (_sample != nullptr)
            _sample->AddRef();
        ::SetEvent(_ready);
        return S_OK;
    }
    STDMETHODIMP OnFlush(DWORD) override { return S_OK; }
    STDMETHODIMP OnEvent(DWORD, IMFMediaEvent*) override { return S_OK; }

    // Blocks until a sample arrives or the deadline passes. On success the
    // sample reference is transferred to the caller.
    bool WaitForSample(DWORD timeoutMS, HRESULT& status, DWORD& flags, LONGLONG& timestamp, IMFSample** sample)
    {
        if (::WaitForSingleObject(_ready, timeoutMS) != WAIT_OBJECT_0)
            return false;
        std::lock_guard<std::mutex> lock(_mutex);
        status = _status;
        flags = _flags;
        timestamp = _timestamp;
        *sample = _sample;
        _sample = nullptr;
        return true;
    }

    // Abandon any future delivery. Called by the reader before it lets go.
    void Orphan()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _orphaned = true;
        if (_sample != nullptr) {
            _sample->Release();
            _sample = nullptr;
        }
    }
};

// Release a wedged reader's Media Foundation and D3D objects off the render
// thread. IMFSourceReader::Release tears the decoder down, which is exactly the
// thing that has stopped responding, so doing it inline would trade a hung
// render for a hung render. The thread is detached and may never finish; that
// leak is bounded by the one-shot latch above and is the point of the exercise.
static void AbandonReaderObjects(IMFSourceReader* reader, MFReadSampleCallback* callback,
                                 IMFDXGIDeviceManager* deviceManager, ID3D11Device* device,
                                 bool releaseSlot)
{
    // Stop delivery first, on this thread: if the thread below cannot start we
    // still must not be left with a callback that can fire into a dead reader.
    if (callback != nullptr)
        callback->Orphan();
    try {
        std::thread([reader, callback, deviceManager, device, releaseSlot]() {
            if (reader != nullptr)
                reader->Release();
            if (callback != nullptr)
                callback->Release();
            if (deviceManager != nullptr)
                deviceManager->Release();
            if (device != nullptr)
                device->Release();
            // Only now is the decoder session really gone. Giving the slot back
            // any earlier would let a replacement open against a session the
            // driver has not actually let go of.
            if (releaseSlot)
                WindowsHardwareVideoReader::ReleaseDecoder();
        }).detach();
    } catch (...) {
        // Out of threads. Leaking these is the only safe option left - the
        // decoder still owns them and releasing here could block the render.
        // The slot stays taken, which is honest: the session is still out there.
        spdlog::warn("WHVD: could not start cleanup thread; abandoning decoder objects");
    }
}

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
#if !defined(XLIGHTS_CMAKE_BUILD)
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "propsys.lib")
#pragma comment(lib, "wmcodecdspuuid.lib")
#endif
//#pragma comment(lib, "d3d11.lib") // d3d11.dll - direct 3d v11
    // D3D11CreateDevice
    typedef DWORD (*D3D11CreateDevice_ptr)(IDXGIAdapter*, D3D_DRIVER_TYPE, HMODULE Software, UINT Flags, const D3D_FEATURE_LEVEL*, UINT, UINT, ID3D11Device**, D3D_FEATURE_LEVEL*, ID3D11DeviceContext**);


#define TIME_DIV 10000
#define ENABLE_HW_ACCELERATION
#define ENABLE_HW_DRIVER

#define SAFEEXEC(fn, msg)               \
    if (SUCCEEDED(hr)) {                \
        hr = fn;                        \
        if (FAILED(hr)) {               \
            spdlog::error("---------- " msg " : {:#08x}x : {}", hr, WindowsHardwareVideoReader::DecodeMFError(hr)); \
        }                               \
    }

#define COMMA ,
#define LIT(x) x
#define DYNAMICCALL(dll, function, fn, msg)                                                                                                           \
    {                                                                                                                                                 \
        function##_ptr ffn = (function##_ptr)GetFunction(dll, #function);                                                                              \
        if (ffn == nullptr) {                                                                                                                         \
            spdlog::error("---------- " msg " : {:#08x}x : {}", hr, WindowsHardwareVideoReader::DecodeMFError(E_NOINTERFACE)); \
            hr = E_NOINTERFACE;                                                                                                                       \
        } else {                                                                                                                                      \
            SAFEEXEC(ffn(fn), msg);                                                                                                                   \
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


        HRESULT hr = S_OK;

//#define THREADING COINIT_MULTITHREADED
#define THREADING COINIT_APARTMENTTHREADED

        spdlog::trace("WHVD: WVHRStatic init on thread {:#x} (APARTMENTTHREADED)", (uintptr_t)::GetCurrentThreadId());
        HRESULT cohr = ::CoInitializeEx(nullptr, THREADING | COINIT_SPEED_OVER_MEMORY | COINIT_DISABLE_OLE1DDE);
        if (FAILED(cohr) && cohr != RPC_E_CHANGED_MODE) {
            spdlog::error("WHVD: CoInitializeEx failed on thread {:#x}: {:#08x}", (uintptr_t)::GetCurrentThreadId(), (unsigned)cohr);
            hr = cohr;
        } else {
            if (cohr == S_FALSE) {
                spdlog::trace("WHVD: COM already initialised on thread {:#x} (S_FALSE — reuse)", (uintptr_t)::GetCurrentThreadId());
            } else if (cohr == RPC_E_CHANGED_MODE) {
                spdlog::warn("WHVD: CoInitializeEx RPC_E_CHANGED_MODE on thread {:#x} — thread already has a different apartment model; MF calls may marshal or deadlock", (uintptr_t)::GetCurrentThreadId());
            }
        }

        // we test load all the dlls to make sure hardware video decoding is possible ... if any fail then we wont try to hardware decode
        // this does not protect us against internal differences but I am hoping it means we can load and run on older platforms
        if (GetDLL("mfplat.dll") == nullptr || GetDLL("mfreadwrite.dll") == nullptr || GetDLL("d3d11.dll") == nullptr) {
            spdlog::error("Failed to load mfplat.dll ... windows hardware debugging disabled.");
        } else {
            DYNAMICCALL("mfplat.dll", MFStartup, MF_VERSION COMMA MFSTARTUP_FULL, "WHVD: Failed to initialise Media Framework");
            if (SUCCEEDED(hr)) {
                _ok = true;
                spdlog::trace("WHVD: Media Foundation started OK on thread {:#x}", (uintptr_t)::GetCurrentThreadId());
            }
        }
    }
    virtual ~WVHRStatic()
    {
        if (_ok) {
            
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
    

#ifdef DETAILED_LOGGING
    spdlog::debug("WHVD: Constructor: {}", (const char*)filename.c_str());
#endif

    _pixelFormat = pixelFormat;
    _wantAlpha = wantAlpha;
    _width = maxwidth;
    _height = maxheight;
    _filename = filename;
    // The video processor IS the DirectX11 path now: it is faster than letting
    // the source reader convert and scale (~17% on a video-heavy sequence) and it
    // is the only way to state the colour space. XL_MF_NO_D3DVP falls back to
    // the source reader's own processing for debugging. Native-resolution
    // requests skip it - there is nothing to scale.
    static const bool sNoVP = (getenv("XL_MF_NO_D3DVP") != nullptr);
    _useVideoProcessor = !sNoVP && !usenativeresolution;

    if (!_init.IsOk())
        return;

    // Take a decoder slot before anything is created. Over the learned limit the
    // reader simply never opens, which the caller already handles as "Media
    // Foundation cannot take this file" and decodes in software.
    if (!TryReserveDecoder()) {
        spdlog::debug("WHVD: at the hardware decoder limit; {} will decode in software", filename);
        return;
    }
    _reservedDecoder = true;

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
    if (!_useVideoProcessor) {
        // Advanced video processing is what we are replacing: it converts and
        // scales with a colour space we cannot set. With the direct video
        // processor path we want the decoder's untouched NV12 surface instead.
        SAFEEXEC(attributes->SetUINT32(MF_SOURCE_READER_ENABLE_ADVANCED_VIDEO_PROCESSING, TRUE), "WHVD: Failed to set attribute");
    }
#else
    SAFEEXEC(attributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE), "WHVD: Failed to set attribute");
#endif

    // Asynchronous reads. Media Foundation delivers on one of its own work
    // queue threads, so the wait below is a plain event wait with a deadline
    // rather than an open-ended call into the decoder.
    _callback = new MFReadSampleCallback();
    if (!_callback->IsOk()) {
        spdlog::error("WHVD: Failed to create read completion event");
        hr = E_FAIL;
    }
    SAFEEXEC(attributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, _callback), "WHVD: Failed to set async callback attribute");

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

        if (_useVideoProcessor) {
            if (!InitVideoProcessor(DXGI_FORMAT_NV12)) {
                spdlog::warn("WHVD: direct video-processor setup failed for {}; abandoning MF for this file", filename);
                ReleaseVideoProcessor();
                _useVideoProcessor = false;
                SafeRelease(&_reader);
                SafeRelease(&_deviceManager);
                return;
            }
        }

        assert(_reader != nullptr);
        assert(_deviceManager != nullptr);

        spdlog::debug("WHVD: Hardware Video Decoder Initialised OK for video: {}", (const char*)filename.c_str());
    } else {
        SafeRelease(&_reader);
        SafeRelease(&_deviceManager);
        assert(_reader == nullptr);
        assert(_deviceManager == nullptr);
    }

    assert(attributes == nullptr);

#ifdef DETAILED_LOGGING
    spdlog::debug("WHVD: Constructor Done: {}", (const char*)filename.c_str());
#endif
}

uint8_t WindowsHardwareVideoReader::GetPixelBytes() const
{
    return ((_pixelFormat == AV_PIX_FMT_RGB24 || _pixelFormat == AV_PIX_FMT_BGR24) ? 3 : 4);
}

WindowsHardwareVideoReader::~WindowsHardwareVideoReader()
{
    
#ifdef DETAILED_LOGGING
    spdlog::debug("WHVD: Destructor.");
#endif
    // Release the reader before our own reference to the callback: the reader
    // holds one too, and dropping ours first would leave it calling into an
    // object that is already gone.
    SafeRelease(&_reader);
    if (_callback != nullptr) {
        _callback->Orphan();
        _callback->Release();
        _callback = nullptr;
    }
    SafeRelease(&_deviceManager);
    ReleaseVideoProcessor();
    SafeRelease(&_device);

    if (_reservedDecoder) {
        _reservedDecoder = false;
        ReleaseDecoder();
    }

    if (_frame != nullptr) {
        if (_frame->data[0] != nullptr) {
            av_free(_frame->data[0]);
        }
        av_frame_free(&_frame);
    }

#ifdef DETAILED_LOGGING
    spdlog::debug("WHVD: Destructor DONE.");
#endif
}

// A read missed its deadline. The decoder still owns the request and may
// complete it at any time or not at all, so this reader is finished: its Media
// Foundation objects are handed to a detached thread and never touched again.
// The caller falls back to software decode for this file, and hardware decode
// stands down for a cooldown so the files after it do not each pay the deadline.
void WindowsHardwareVideoReader::HandleReadTimeout(uint32_t timestampMS)
{
    _hardwareFailed = true;

    NoteHardwareFailure(__mfActiveReaders.load(std::memory_order_acquire), _filename);

    const int cooldownMS = StartMediaFoundationCooldown();
    if (cooldownMS > 0) {
        spdlog::error("WHVD: no frame after {}ms seeking {}ms in {} - Media Foundation decode has stopped responding. "
                      "Using software decode for the next {}s.",
                      ReadSampleTimeoutMS(), timestampMS, _filename, cooldownMS / 1000);
    } else {
        spdlog::warn("WHVD: no frame after {}ms seeking {}ms in {} - falling back to software decode",
                     ReadSampleTimeoutMS(), timestampMS, _filename);
    }

    AbandonReaderObjects(_reader, _callback, _deviceManager, _device, _reservedDecoder);
    _reservedDecoder = false; // the cleanup thread owns the slot now
    _reader = nullptr;
    _callback = nullptr;
    _deviceManager = nullptr;
    _device = nullptr;
}

bool WindowsHardwareVideoReader::CanSeek() const
{
    
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
                spdlog::debug("WHVD: Able to seek.");
            }
        }
        PropVariantClear(&var);
    }
    return res;
}

HRESULT WindowsHardwareVideoReader::SelectVideoStream(bool usenativeresolution, bool keepaspectratio)
{
    
    HRESULT hr = S_OK;

    // I need to get the native size of the video first
    IMFMediaType* pType = nullptr;
    DYNAMICCALL("mfplat.dll", MFCreateMediaType, &pType, "WHVD: Failed to create media type");
    if (SUCCEEDED(hr) && pType != nullptr) {
        SAFEEXEC(pType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video), "WHVD: Failed to set major type");

        SAFEEXEC(pType->SetGUID(MF_MT_SUBTYPE, _useVideoProcessor ? MFVideoFormat_NV12 : MFVideoFormat_RGB32), "WHVD: Failed to set sub type");

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
                spdlog::error("WHVD: Failed to get frame rate ");
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

        SAFEEXEC(pType->SetGUID(MF_MT_SUBTYPE,
                                _useVideoProcessor ? MFVideoFormat_NV12
                                                   : (_wantAlpha ? MFVideoFormat_ARGB32 : MFVideoFormat_RGB32)),
                 "WHVD: Failed to set sub type");


        if (SUCCEEDED(hr)) {
            if (usenativeresolution || _useVideoProcessor) {
            } else {
                SAFEEXEC(MFSetAttributeSize(pType, MF_MT_FRAME_SIZE, _width, _height), "WHVD: Failed to set target size");

                if (!keepaspectratio) {
                    // we need to stretch pixels
                    spdlog::info("Stretching pixels by {}/{}", _height * _nativeWidth, _width * _nativeHeight);
                    SAFEEXEC(MFSetAttributeRatio(pType, MF_MT_PIXEL_ASPECT_RATIO, _height * _nativeWidth, _width * _nativeHeight), "WHVD: Failed to set target ratio");
                }
            }
        }

        SAFEEXEC(_reader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, pType), "WHVD: Failed to set media type");

        SAFEEXEC(_reader->SetStreamSelection((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, TRUE), "WHVD: Failed to set stream");

        SafeRelease(&pType);
        pType = nullptr;
    }

    if (getenv("XL_MF_COLOR_DEBUG") != nullptr) {
        auto dumpColor = [](const char* what, IMFMediaType* mt) {
            if (mt == nullptr) {
                return;
            }
            auto attr = [mt](const GUID& g) -> long {
                UINT32 v = 0;
                return SUCCEEDED(mt->GetUINT32(g, &v)) ? (long)v : -1;
            };
            UINT32 w = 0, h = 0;
            MFGetAttributeSize(mt, MF_MT_FRAME_SIZE, &w, &h);
            spdlog::warn("WHVD COLOR {}: size={}x{} nominalRange={} yuvMatrix={} transferFn={} primaries={} lighting={}  (-1 = not set)",
                         what, w, h, attr(MF_MT_VIDEO_NOMINAL_RANGE), attr(MF_MT_YUV_MATRIX),
                         attr(MF_MT_TRANSFER_FUNCTION), attr(MF_MT_VIDEO_PRIMARIES), attr(MF_MT_VIDEO_LIGHTING));
        };
        IMFMediaType* pNative = nullptr;
        if (SUCCEEDED(_reader->GetNativeMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &pNative))) {
            dumpColor("source", pNative);
            SafeRelease(&pNative);
        }
        IMFMediaType* pCur = nullptr;
        if (SUCCEEDED(_reader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, &pCur))) {
            dumpColor("output", pCur);
            SafeRelease(&pCur);
        }
    }

    SAFEEXEC(_reader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, &pType), "WHVD: Failed to get media type");
    if (SUCCEEDED(hr) && pType != nullptr) {
        _stride = (LONG)MFGetAttributeUINT32(pType, MF_MT_DEFAULT_STRIDE, 1);
        assert(_stride = _width * GetPixelBytes());

        GUID subtype = { 0 };
        SAFEEXEC(pType->GetGUID(MF_MT_SUBTYPE, &subtype), "WHVD: Failed to get media subtype");

        if (SUCCEEDED(hr)) {
            const bool wantNV12 = _useVideoProcessor;
            if (wantNV12 ? !IsEqualGUID(subtype, MFVideoFormat_NV12)
                         : (!IsEqualGUID(subtype, MFVideoFormat_RGB32) && !IsEqualGUID(subtype, MFVideoFormat_ARGB32))) {
                spdlog::error("WHVD: Invalid media subtype");
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
    spdlog::debug("WHVD: Seeking to {}", pos);
#endif
    SAFEEXEC(_reader->SetCurrentPosition(GUID_NULL, var), "WHVD: Failed to seek");

    if (FAILED(hr)) {
        // A reader that cannot reposition is finished - every later frame
        // request would seek again and fail again, silently handing the effect
        // no video at all. Hand the file to the software decoder instead.
        if (!_hardwareFailed) {
            spdlog::error("WHVD: seek to {}ms failed in {} ({}) - falling back to software decode",
                          pos, _filename, DecodeMFError(hr));
            // Under load this is what running out of decoders looks like: the
            // reader answers, but can no longer reposition. Same lesson as a
            // missed deadline, so it feeds the same limit.
            NoteHardwareFailure(__mfActiveReaders.load(std::memory_order_acquire), _filename);
        }
        _hardwareFailed = true;
        PropVariantClear(&var);
        return false;
    }

    if (pos > 0) {
        bool first = true;
        do {
            uint32_t lastPos = _curPos;
            AVFrame* frame = GetNextFrame(0xFFFFFFFF, 0xFFFFFFFF);
            if (frame == nullptr) {
                spdlog::error("WHVD: GetNextFrame failed");
                PropVariantClear(&var);
                return false;
            }
            if (!first && lastPos == _curPos) {
                spdlog::error("WHVD: seek to {}ms stopped advancing at {}ms in {} - falling back to software decode",
                              pos, _curPos, _filename);
                _hardwareFailed = true;
                PropVariantClear(&var);
                return false;
            }
            first = false;
        } while (((LONGLONG)_curPos) * TIME_DIV < var.hVal.QuadPart);
    } else {
        _curPos = 0;
    }
    PropVariantClear(&var);
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

bool WindowsHardwareVideoReader::InitVideoProcessor(DXGI_FORMAT inputFormat)
{
    if (_device == nullptr) {
        return false;
    }

    _device->GetImmediateContext(&_immediateContext);
    if (_immediateContext == nullptr) {
        spdlog::warn("WHVD VP: no immediate context");
        return false;
    }
    if (FAILED(_device->QueryInterface(__uuidof(ID3D11VideoDevice), (void**)&_videoDevice)) ||
        FAILED(_immediateContext->QueryInterface(__uuidof(ID3D11VideoContext), (void**)&_videoContext))) {
        spdlog::warn("WHVD VP: ID3D11VideoDevice/VideoContext unavailable");
        return false;
    }

    D3D11_VIDEO_PROCESSOR_CONTENT_DESC desc = {};
    desc.InputFrameFormat = D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE;
    desc.InputWidth = _nativeWidth;
    desc.InputHeight = _nativeHeight;
    desc.OutputWidth = _width;
    desc.OutputHeight = _height;
    desc.Usage = D3D11_VIDEO_USAGE_PLAYBACK_NORMAL;

    if (FAILED(_videoDevice->CreateVideoProcessorEnumerator(&desc, &_vpEnum)) || _vpEnum == nullptr) {
        spdlog::warn("WHVD VP: CreateVideoProcessorEnumerator failed");
        return false;
    }
    UINT fmtFlags = 0;
    if (FAILED(_vpEnum->CheckVideoProcessorFormat(inputFormat, &fmtFlags)) ||
        (fmtFlags & D3D11_VIDEO_PROCESSOR_FORMAT_SUPPORT_INPUT) == 0) {
        spdlog::warn("WHVD VP: input format {} not supported by the processor", (int)inputFormat);
        return false;
    }
    if (FAILED(_videoDevice->CreateVideoProcessor(_vpEnum, 0, &_videoProcessor)) || _videoProcessor == nullptr) {
        spdlog::warn("WHVD VP: CreateVideoProcessor failed");
        return false;
    }

    // State the colour space instead of letting the processor guess - but take
    // it from the decoder rather than assuming. A fixed studio/BT.709 guess was
    // measured wrong per file: brightness ratios against the FFmpeg path ranged
    // 0.84 to 1.31 across models, because different files really do differ.
    // Only fall back to the resolution heuristic when the decoder says nothing.
    UINT32 srcRange = 0;
    UINT32 srcMatrix = 0;
    bool haveRange = false;
    bool haveMatrix = false;
    {
        IMFMediaType* cur = nullptr;
        if (SUCCEEDED(_reader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, &cur)) && cur != nullptr) {
            haveRange = SUCCEEDED(cur->GetUINT32(MF_MT_VIDEO_NOMINAL_RANGE, &srcRange));
            haveMatrix = SUCCEEDED(cur->GetUINT32(MF_MT_YUV_MATRIX, &srcMatrix));
            SafeRelease(&cur);
        }
    }

    D3D11_VIDEO_PROCESSOR_COLOR_SPACE inCS = {};
    inCS.Usage = 0;         // playback, not video processing
    inCS.RGB_Range = 0;     // full (not meaningful for a YUV input)
    // 1 = BT.709, 0 = BT.601. When nothing is declared - which is every file
    // measured here - match swscale rather than guess by resolution: FFmpeg
    // falls back to BT.601 coefficients for an unspecified colorspace, and the
    // FFmpeg reader is the path this has to agree with.
    inCS.YCbCr_Matrix = haveMatrix ? (srcMatrix == MFVideoTransferMatrix_BT601 ? 0 : 1) : 0;
    inCS.YCbCr_xvYCC = 0;
    inCS.Nominal_Range = (haveRange && srcRange == MFNominalRange_0_255)
                             ? D3D11_VIDEO_PROCESSOR_NOMINAL_RANGE_0_255
                             : D3D11_VIDEO_PROCESSOR_NOMINAL_RANGE_16_235;
    _videoContext->VideoProcessorSetStreamColorSpace(_videoProcessor, 0, &inCS);
    spdlog::info("WHVD VP: source colour range={} matrix={} (declared: range {} matrix {})",
                 (int)inCS.Nominal_Range, (int)inCS.YCbCr_Matrix,
                 haveRange ? (int)srcRange : -1, haveMatrix ? (int)srcMatrix : -1);

    D3D11_VIDEO_PROCESSOR_COLOR_SPACE outCS = {};
    outCS.Usage = 0;
    outCS.RGB_Range = 0;    // 0 = full 0-255, which is what the render buffer wants
    outCS.YCbCr_Matrix = inCS.YCbCr_Matrix;
    outCS.YCbCr_xvYCC = 0;
    outCS.Nominal_Range = D3D11_VIDEO_PROCESSOR_NOMINAL_RANGE_0_255;
    _videoContext->VideoProcessorSetOutputColorSpace(_videoProcessor, &outCS);

    // Drivers otherwise apply denoise/sharpening/skin-tone "enhancements" that
    // would alter pixels in ways no other decode path in xLights does.
    _videoContext->VideoProcessorSetStreamAutoProcessingMode(_videoProcessor, 0, FALSE);

    // Explicitly neutralise every filter. Turning auto-processing off is not
    // enough: a driver may still apply a non-neutral default, and this one did
    // - raw frames came out a near-constant +2.5 brighter than swscale at every
    // intensity, which is an additive offset (a brightness filter), not the
    // multiplicative error a colour-space mistake produces. Each filter is
    // pinned to the range's stated default and disabled.
    static const D3D11_VIDEO_PROCESSOR_FILTER kFilters[] = {
        D3D11_VIDEO_PROCESSOR_FILTER_BRIGHTNESS,
        D3D11_VIDEO_PROCESSOR_FILTER_CONTRAST,
        D3D11_VIDEO_PROCESSOR_FILTER_HUE,
        D3D11_VIDEO_PROCESSOR_FILTER_SATURATION,
        D3D11_VIDEO_PROCESSOR_FILTER_NOISE_REDUCTION,
        D3D11_VIDEO_PROCESSOR_FILTER_EDGE_ENHANCEMENT,
        D3D11_VIDEO_PROCESSOR_FILTER_ANAMORPHIC_SCALING,
        D3D11_VIDEO_PROCESSOR_FILTER_STEREO_ADJUSTMENT,
    };
    for (auto filter : kFilters) {
        D3D11_VIDEO_PROCESSOR_FILTER_RANGE range = {};
        if (SUCCEEDED(_vpEnum->GetVideoProcessorFilterRange(filter, &range))) {
            _videoContext->VideoProcessorSetStreamFilter(_videoProcessor, 0, filter, FALSE, range.Default);
        }
    }

    D3D11_TEXTURE2D_DESC td = {};
    td.Width = _width;
    td.Height = _height;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_RENDER_TARGET;
    if (FAILED(_device->CreateTexture2D(&td, nullptr, &_vpOutput))) {
        spdlog::warn("WHVD VP: output texture creation failed");
        return false;
    }

    D3D11_VIDEO_PROCESSOR_OUTPUT_VIEW_DESC ovd = {};
    ovd.ViewDimension = D3D11_VPOV_DIMENSION_TEXTURE2D;
    ovd.Texture2D.MipSlice = 0;
    if (FAILED(_videoDevice->CreateVideoProcessorOutputView(_vpOutput, _vpEnum, &ovd, &_vpOutputView))) {
        spdlog::warn("WHVD VP: output view creation failed");
        return false;
    }

    td.Usage = D3D11_USAGE_STAGING;
    td.BindFlags = 0;
    td.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    if (FAILED(_device->CreateTexture2D(&td, nullptr, &_staging))) {
        spdlog::warn("WHVD VP: staging texture creation failed");
        return false;
    }

    spdlog::info("WHVD VP: ready {}x{} -> {}x{}, matrix BT.{}",
                  _nativeWidth, _nativeHeight, _width, _height, inCS.YCbCr_Matrix ? 709 : 601);
    return true;
}

// Convert + scale the decoder's GPU surface with the video processor and read
// back the (small) result. Only the final target-size frame crosses the bus.
bool WindowsHardwareVideoReader::BltFromSample(IMFSample* sample)
{
    IMFMediaBuffer* buf = nullptr;
    if (FAILED(sample->GetBufferByIndex(0, &buf)) || buf == nullptr) {
        return false;
    }

    IMFDXGIBuffer* dxgi = nullptr;
    if (FAILED(buf->QueryInterface(__uuidof(IMFDXGIBuffer), (void**)&dxgi)) || dxgi == nullptr) {
        // Software-decoded sample - there is no GPU surface to process.
        SafeRelease(&buf);
        return false;
    }

    ID3D11Texture2D* tex = nullptr;
    UINT subIdx = 0;
    HRESULT hr = dxgi->GetResource(__uuidof(ID3D11Texture2D), (void**)&tex);
    dxgi->GetSubresourceIndex(&subIdx);

    bool ok = false;
    if (SUCCEEDED(hr) && tex != nullptr) {
        D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC ivd = {};
        ivd.FourCC = 0;
        ivd.ViewDimension = D3D11_VPIV_DIMENSION_TEXTURE2D;
        ivd.Texture2D.MipSlice = 0;
        ivd.Texture2D.ArraySlice = subIdx;

        ID3D11VideoProcessorInputView* iv = nullptr;
        if (SUCCEEDED(_videoDevice->CreateVideoProcessorInputView(tex, _vpEnum, &ivd, &iv)) && iv != nullptr) {
            D3D11_VIDEO_PROCESSOR_STREAM stream = {};
            stream.Enable = TRUE;
            stream.pInputSurface = iv;

            if (SUCCEEDED(_videoContext->VideoProcessorBlt(_videoProcessor, _vpOutputView, 0, 1, &stream))) {
                _immediateContext->CopyResource(_staging, _vpOutput);

                D3D11_MAPPED_SUBRESOURCE map = {};
                if (SUCCEEDED(_immediateContext->Map(_staging, 0, D3D11_MAP_READ, 0, &map))) {
                    const uint8_t pb = GetPixelBytes();
                    const bool bgrOut = (_pixelFormat == AV_PIX_FMT_BGRA || _pixelFormat == AV_PIX_FMT_BGR24);
                    const uint8_t* base = (const uint8_t*)map.pData;
                    const uint32_t pitch = map.RowPitch;
                    const uint32_t w = _width;
                    AVFrame* frame = _frame;
                    parallel_for(0, (int)_height, [base, pitch, w, pb, bgrOut, frame](int y) {
                        const uint8_t* src = base + (size_t)y * pitch;
                        uint8_t* dst = frame->data[0] + (size_t)y * frame->linesize[0];
                        for (uint32_t x = 0; x < w; ++x) {
                            uint8_t b = src[x * 4 + 0];
                            uint8_t g = src[x * 4 + 1];
                            uint8_t r = src[x * 4 + 2];
                            // See BitmapFromSample: near-black must land on exact
                            // zero or TransparentBlack treats it as opaque.
                            if (r <= 4 && g <= 4 && b <= 4) {
                                r = 0;
                                g = 0;
                                b = 0;
                            }
                            if (bgrOut) {
                                dst[x * pb + 0] = b;
                                dst[x * pb + 1] = g;
                                dst[x * pb + 2] = r;
                            } else {
                                dst[x * pb + 0] = r;
                                dst[x * pb + 1] = g;
                                dst[x * pb + 2] = b;
                            }
                            if (pb == 4) {
                                dst[x * pb + 3] = src[x * 4 + 3];
                            }
                        }
                    });
                    _immediateContext->Unmap(_staging, 0);
                    ok = true;
                }
            }
            SafeRelease(&iv);
        }
        SafeRelease(&tex);
    }

    SafeRelease(&dxgi);
    SafeRelease(&buf);
    return ok;
}

void WindowsHardwareVideoReader::ReleaseVideoProcessor()
{
    SafeRelease(&_staging);
    SafeRelease(&_vpOutputView);
    SafeRelease(&_vpOutput);
    SafeRelease(&_videoProcessor);
    SafeRelease(&_vpEnum);
    SafeRelease(&_videoContext);
    SafeRelease(&_videoDevice);
    SafeRelease(&_immediateContext);
}

bool WindowsHardwareVideoReader::BitmapFromSample(IMFSample* sample, AVFrame* frame)
{
    
    bool res = false;
    HRESULT hr = S_OK;

#ifdef DETAILED_LOGGING
    spdlog::debug("WHVD: Get sample buffer");
#endif
    IMFMediaBuffer* pBuffer = nullptr;
    SAFEEXEC(sample->ConvertToContiguousBuffer(&pBuffer), "WHVD: Failed to get sample buffer");

#ifdef DETAILED_LOGGING
    spdlog::debug("WHVD: lock buffer");
#endif
    BYTE* pBitmapData = nullptr; // Bitmap data
    DWORD cbBitmapData = 0;      // Size of data, in bytes
    SAFEEXEC(pBuffer->Lock(&pBitmapData, nullptr, &cbBitmapData), "WHVD: Failed to lock buffer");

    if (FAILED(hr)) {
        spdlog::error("WHVD: BitmapFromSample Lock failed at {}ms thread={:#x} hr={:#08x}", _curPos, (uintptr_t)::GetCurrentThreadId(), (unsigned)hr);
        HRESULT reason = _device->GetDeviceRemovedReason();
        spdlog::error("WHVD: Device removed reason {:#08x} : {}", (unsigned)reason, (const char*)DecodeDXGIReason(reason).c_str());
    }

    if (SUCCEEDED(hr)) {
        assert(pBitmapData != nullptr);
        assert(cbBitmapData > 0);
        // Snap uniformly-near-black pixels to exact (0,0,0). H.264 artifacts in
        // dark regions decode to values like (2,0,2) - sum 4, one over a typical
        // TransparentBlack threshold of 3 - which render as faint blotches where
        // the user expects clean transparency. FFmpeg's swscale bicubic happens
        // to clip those during scaling; the MF video processor preserves them.
        // Requires ALL THREE channels <= 4 so deliberately dark single-channel
        // content (e.g. RGB(0,0,10)) is left alone. Mirrors the macOS bridge's
        // copyPixelBufferToFrame.
        auto snap = [](uint8_t& r, uint8_t& g, uint8_t& b) {
            if (r <= 4 && g <= 4 && b <= 4) {
                r = 0;
                g = 0;
                b = 0;
            }
        };
        uint8_t pb = GetPixelBytes();
        AVFrame* dstFrame = _frame;
        uint32_t dstRows = _height;
        if (_pixelFormat == AVPixelFormat::AV_PIX_FMT_BGRA || _pixelFormat == AVPixelFormat::AV_PIX_FMT_BGR24) {
            // Source layout already matches the destination; copy then snap.
            memcpy(dstFrame->data[0], pBitmapData, std::min((uint32_t)cbBitmapData, (uint32_t)dstFrame->linesize[0] * dstRows));
            parallel_for(0, std::min((uint32_t)cbBitmapData / 4, ((uint32_t)dstFrame->linesize[0] * dstRows) / pb), [dstFrame, pb, snap](int i) {
                uint8_t* p = dstFrame->data[0] + i * pb;
                snap(p[0], p[1], p[2]);
            });
        } else {
            parallel_for(0, std::min((uint32_t)cbBitmapData / 4, ((uint32_t)dstFrame->linesize[0] * dstRows) / pb), [dstFrame, pb, pBitmapData, snap](int i) {
                uint8_t* p = dstFrame->data[0] + i * pb;
                p[0] = *(pBitmapData + i * 4 + 2);
                p[1] = *(pBitmapData + i * 4 + 1);
                p[2] = *(pBitmapData + i * 4 + 0);
                snap(p[0], p[1], p[2]);
                if (pb == 4)
                    p[3] = *(pBitmapData + i * 4 + 3);
            });
        }
        res = true;
#ifdef DETAILED_LOGGING
        spdlog::debug("WHVD: Unlock buffer");
#endif
        SAFEEXEC(pBuffer->Unlock(), "WHVD: Failed to unlock buffer");
    }

#ifdef DETAILED_LOGGING
    spdlog::debug("WHVD: Release buffer");
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

    HRESULT hr = S_OK;

    spdlog::trace("WHVD: GetNextFrame {} curPos={} duration={} thread={:#x}", timestampMS, _curPos, GetDuration(), (uintptr_t)::GetCurrentThreadId());
#ifdef DETAILED_LOGGING
    spdlog::debug("WHVD: GetNextFrame {}.", timestampMS);
#endif

    if (_reader == nullptr || (timestampMS != 0xFFFFFFFF && timestampMS > GetDuration()))
        return nullptr;

    // Reuse the frame already decoded when it is the one NEAREST the request.
    // The old test was `_curPos > timestampMS`, strictly greater, so a request
    // landing exactly on the cached frame (curPos == timestampMS) fell through
    // to the read loop - and that loop always reads at least one new sample, so
    // it overshot by a whole frame. Measured against AVFoundation on a 30fps
    // clip that was a constant +16.6ms (half a frame) on every such request.
    // Allowing half a frame of lead-in makes the reuse test the same
    // nearest-frame rule the read loop uses.
    if (timestampMS != 0xFFFFFFFF && timestampMS != 0 &&
        (int64_t)_curPos + (int64_t)(_frameMS / 2) >= (int64_t)timestampMS &&
        _curPos < timestampMS + GetFrameMS()) {
        // the last frame should be ok ... so just return it again
#ifdef DETAILED_LOGGING
        spdlog::debug("WHVD: Just returning last frame at {}.", _curPos);
#endif
        return _frame;
    }

    // we only seek if the timestamp is valid and we are past the time or more than 5 seconds from it.
    if (timestampMS != 0xFFFFFFFF && (_curPos > timestampMS || (LONGLONG)_curPos < (LONGLONG)timestampMS - 5000)) {
        if (!Seek(timestampMS)) {
            return nullptr;
        }
    }

    IMFSample* sample = nullptr;
    bool wantMore = false;   // set at the bottom of the loop; drives the do-while
    uint32_t lastPos = _curPos;
    int stalled = 0;
    do {
        assert(sample == nullptr);

#ifdef DETAILED_LOGGING
        spdlog::debug("WHVD: Reading sample");
#endif
        DWORD dwFlags = 0;
        LONGLONG currentTime = 0;
        spdlog::trace("WHVD: ReadSample enter pos={}ms target={}ms thread={:#x}", _curPos, timestampMS, (uintptr_t)::GetCurrentThreadId());
        {
            auto _rs_t0 = std::chrono::steady_clock::now();
            // Asynchronous form: every out-parameter must be null, the result
            // arrives on the callback. Only one read may be outstanding per
            // stream, which holds because this is the sole caller and it waits.
            SAFEEXEC(_reader->ReadSample((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, nullptr, nullptr, nullptr, nullptr), "WHVD: Failed to request frame");
            if (SUCCEEDED(hr)) {
                HRESULT readStatus = S_OK;
                if (!_callback->WaitForSample(ReadSampleTimeoutMS(), readStatus, dwFlags, currentTime, &sample)) {
                    HandleReadTimeout(timestampMS);
                    return nullptr;
                }
                hr = readStatus;
            }
            auto _rs_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - _rs_t0).count();
            if (_rs_ms > 500)
                spdlog::warn("WHVD: ReadSample took {}ms (pos={}ms target={}ms thread={:#x}) — possible GPU stall or TDR", _rs_ms, _curPos, timestampMS, (uintptr_t)::GetCurrentThreadId());
            else
                spdlog::trace("WHVD: ReadSample exit {}ms hr={:#08x} flags={:#x}({})", _rs_ms, (unsigned)hr, dwFlags, DecodeReadFlags(dwFlags));
        }

#ifdef DETAILED_LOGGING
        spdlog::info("Read flags: {:#08x}x : {}", dwFlags, DecodeReadFlags(dwFlags));
#endif

        if (FAILED(hr)) {
            if (_device != nullptr) {
                HRESULT removed = _device->GetDeviceRemovedReason();
                if (FAILED(removed))
                    spdlog::error("WHVD: D3D11 device removed: {:#08x} ({})", (unsigned)removed, DecodeMFError(removed));
                else
                    spdlog::warn("WHVD: ReadSample failed but D3D11 device still alive (removed=S_OK)");
            }
            return nullptr;
        }

        if (dwFlags & MF_SOURCE_READERF_ENDOFSTREAM) {
#ifdef DETAILED_LOGGING
            spdlog::debug("WHVD: Release sample");
#endif
            SafeRelease(&sample);
            spdlog::info("WHVD: Reached end of video");
            return nullptr;
        }

        if (SUCCEEDED(hr)) {
            _curPos = currentTime / TIME_DIV;
#ifdef DETAILED_LOGGING
            spdlog::debug("Read video at {}ms", _curPos);
#endif
        }

        // Stop on the frame NEAREST the requested time, not the first frame at
        // or after it. `_curPos < timestampMS` overshoots by a whole frame
        // whenever the request falls in the first half of a frame interval:
        // measured against AVFoundation on a 29.97fps clip, MF served the
        // following frame on roughly half of all requests (+9.3ms mean, vs
        // +0.9ms for FFmpeg, which agrees with AVFoundation). Half a frame of
        // lead-in makes the three paths pick the same frame.
        wantMore = (timestampMS != 0xFFFFFFFF) &&
                   ((int64_t)_curPos + (int64_t)(_frameMS / 2) < (int64_t)timestampMS);

        // A decoder that keeps succeeding without advancing would spin here for
        // ever. Seek has always guarded against that; this loop had not.
        if (wantMore && _curPos == lastPos) {
            if (++stalled >= 100) {
                // This file, not the platform: the decoder is answering, it just
                // will not move on. Fall back for this reader only - the
                // process-wide latch is for a decoder that has stopped
                // responding altogether.
                spdlog::error("WHVD: decoder stopped advancing at {}ms seeking {}ms in {} - falling back to software decode",
                              _curPos, timestampMS, _filename);
                SafeRelease(&sample);
                _hardwareFailed = true;
                return nullptr;
            }
        } else {
            stalled = 0;
        }
        lastPos = _curPos;

        // we are not going to use this frame so we can let it go
        if (wantMore) {
#ifdef DETAILED_LOGGING
            spdlog::debug("WHVD: Release sample");
#endif
            SafeRelease(&sample);
        }

    } while (wantMore);

    if (timestampMS != 0xFFFFFFFF && sample != nullptr) {
        if (_useVideoProcessor ? !BltFromSample(sample) : !BitmapFromSample(sample, _frame)) {
            spdlog::error("WHVD: Failed to extract the frame bitmap ... Media Foundations may be in a corrupt state.");
        }
#ifdef DETAILED_LOGGING
        spdlog::debug("WHVD: Release sample");
#endif
        SafeRelease(&sample);
#ifdef DETAILED_LOGGING
        spdlog::debug("WHVD: GetNextFrame {} DONE.", timestampMS);
#endif
        return _frame;
    }

    SafeRelease(&sample);

#ifdef DETAILED_LOGGING
    spdlog::debug("WHVD: GetNextFrame {} DONE.", timestampMS);
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
