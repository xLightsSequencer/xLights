/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "GLContextManager.h"

#include <algorithm>
#include <list>
#include <queue>
#include <thread>
#include <vector>

#include <log.h>

// =========================================================================
// Apple — EGL/ANGLE (USE_GLES) or CGL (legacy desktop GL)
// =========================================================================
#if defined(__APPLE__)

#ifdef USE_GLES

// ---- ANGLE/EGL implementation (OpenGL ES 3.0 on Metal) ----

#define EGL_EGL_PROTOTYPES 1
#define GL_GLES_PROTOTYPES 1
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglext_angle.h>
#include <GLES3/gl3.h>

// ANGLE's Metal backend is NOT thread-safe — only one context may be active
// at a time.  Pool size 1 ensures callers serialize naturally: the second
// thread blocks on the condition_variable until the first releases.
static constexpr int kMaxPoolSize = 1;

struct GLContextManager::PlatformState {
    EGLDisplay display = EGL_NO_DISPLAY;
    EGLConfig  config  = nullptr;
    EGLContext sharedContext = EGL_NO_CONTEXT;

    struct PoolEntry {
        EGLContext context;
        EGLSurface surface;  // 1x1 pbuffer; required for eglMakeCurrent
    };

    std::list<PoolEntry> pool;
    int contextCount = 0;
    std::mutex poolMutex;
    std::condition_variable poolNotifier;
};

using PlatformStateEGL = GLContextManager::PlatformState;

static bool initEGLDisplay(PlatformStateEGL* ps, uint64_t metalDeviceRegistryID) {
    // Request ANGLE's Metal backend, optionally targeting a specific GPU
    std::vector<EGLAttrib> displayAttribs = {
        EGL_PLATFORM_ANGLE_TYPE_ANGLE,
        EGL_PLATFORM_ANGLE_TYPE_METAL_ANGLE,
    };
    if (metalDeviceRegistryID != 0) {
        // Force ANGLE onto the same Metal device as the compute effects
        displayAttribs.push_back(EGL_PLATFORM_ANGLE_DEVICE_ID_HIGH_ANGLE);
        displayAttribs.push_back(static_cast<EGLAttrib>(metalDeviceRegistryID >> 32));
        displayAttribs.push_back(EGL_PLATFORM_ANGLE_DEVICE_ID_LOW_ANGLE);
        displayAttribs.push_back(static_cast<EGLAttrib>(metalDeviceRegistryID & 0xFFFFFFFF));
    }
    displayAttribs.push_back(EGL_NONE);

    ps->display = eglGetPlatformDisplay(
        EGL_PLATFORM_ANGLE_ANGLE,
        nullptr,
        displayAttribs.data());

    if (ps->display == EGL_NO_DISPLAY) {
        spdlog::error("GLContextManager: eglGetPlatformDisplay failed");
        return false;
    }

    EGLint major = 0, minor = 0;
    if (!eglInitialize(ps->display, &major, &minor)) {
        spdlog::error("GLContextManager: eglInitialize failed: 0x{:X}", eglGetError());
        return false;
    }
    if (metalDeviceRegistryID != 0) {
        spdlog::info("GLContextManager: EGL {}.{} (ANGLE/Metal, device 0x{:X})", major, minor, metalDeviceRegistryID);
    } else {
        spdlog::info("GLContextManager: EGL {}.{} (ANGLE/Metal, default device)", major, minor);
    }

    // Choose an RGBA8 config that supports pbuffer surfaces
    const EGLint configAttribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_SURFACE_TYPE,    EGL_PBUFFER_BIT,
        EGL_RED_SIZE,        8,
        EGL_GREEN_SIZE,      8,
        EGL_BLUE_SIZE,       8,
        EGL_ALPHA_SIZE,      8,
        EGL_NONE
    };
    EGLint numConfigs = 0;
    if (!eglChooseConfig(ps->display, configAttribs, &ps->config, 1, &numConfigs) || numConfigs == 0) {
        spdlog::error("GLContextManager: eglChooseConfig failed: 0x{:X}", eglGetError());
        return false;
    }
    return true;
}

static PlatformStateEGL::PoolEntry
createEGLContext(PlatformStateEGL* ps, EGLContext shared) {
    const EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_NONE
    };
    EGLContext ctx = eglCreateContext(ps->display, ps->config, shared, contextAttribs);
    if (ctx == EGL_NO_CONTEXT) {
        spdlog::error("GLContextManager: eglCreateContext failed: 0x{:X}", eglGetError());
        return { EGL_NO_CONTEXT, EGL_NO_SURFACE };
    }

    // Create a 1x1 pbuffer surface — needed for eglMakeCurrent but we render to FBOs
    const EGLint pbufferAttribs[] = {
        EGL_WIDTH,  1,
        EGL_HEIGHT, 1,
        EGL_NONE
    };
    EGLSurface surface = eglCreatePbufferSurface(ps->display, ps->config, pbufferAttribs);
    if (surface == EGL_NO_SURFACE) {
        spdlog::error("GLContextManager: eglCreatePbufferSurface failed: 0x{:X}", eglGetError());
        eglDestroyContext(ps->display, ctx);
        return { EGL_NO_CONTEXT, EGL_NO_SURFACE };
    }

    // Log GL info on first successful context
    eglMakeCurrent(ps->display, surface, surface, ctx);
    const char* ver  = (const char*)glGetString(GL_VERSION);
    const char* rend = (const char*)glGetString(GL_RENDERER);
    const char* vend = (const char*)glGetString(GL_VENDOR);
    spdlog::info("GLContextManager - glVer: {} ({}) ({})",
                 ver ? ver : "?", rend ? rend : "?", vend ? vend : "?");
    eglMakeCurrent(ps->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

    return { ctx, surface };
}

GLContextManager& GLContextManager::Instance() {
    static GLContextManager instance;
    return instance;
}

GLContextManager::~GLContextManager() {
    Shutdown();
}

void GLContextManager::Initialize(const InitParams& params) {
    if (_initialized) return;
    _params = params;
    _platform = new PlatformState();
    _initialized = true;
}

GLContextManager::ContextHandle GLContextManager::AcquireContext() {
    if (!_platform) return nullptr;

    std::unique_lock<std::mutex> lock(_platform->poolMutex);

    // Lazy-init the EGL display and shared context
    if (_platform->display == EGL_NO_DISPLAY) {
        lock.unlock();
        if (!initEGLDisplay(_platform, _params.metalDeviceRegistryID)) return nullptr;
        auto shared = createEGLContext(_platform, EGL_NO_CONTEXT);
        lock.lock();
        if (shared.context == EGL_NO_CONTEXT) return nullptr;
        _platform->sharedContext = shared.context;
        // The shared context's surface is not pooled; it stays with the shared ctx
    }

    // Grow pool if below max
    if (_platform->pool.empty() && _platform->contextCount < kMaxPoolSize) {
        lock.unlock();
        auto entry = createEGLContext(_platform, _platform->sharedContext);
        lock.lock();
        if (entry.context != EGL_NO_CONTEXT) {
            _platform->pool.push_front(entry);
            ++_platform->contextCount;
        } else if (_platform->contextCount == 0) {
            // First-ever creation failed and there's nothing in flight to
            // wait for — fail fast instead of deadlocking on poolNotifier.
            return nullptr;
        }
    }

    // Wait for a context to become available
    while (_platform->pool.empty()) {
        _platform->poolNotifier.wait(lock);
    }

    auto entry = _platform->pool.front();
    _platform->pool.pop_front();

    // Pack both context and surface into a single opaque handle
    auto* handle = new PlatformState::PoolEntry(entry);
    return (ContextHandle)handle;
}

bool GLContextManager::MakeCurrent(ContextHandle ctx) {
    auto* entry = (PlatformState::PoolEntry*)ctx;
    if (entry && _platform) {
        return eglMakeCurrent(_platform->display, entry->surface, entry->surface, entry->context) == EGL_TRUE;
    }
    return false;
}

void GLContextManager::DoneCurrent(ContextHandle /*ctx*/) {
    if (_platform && _platform->display != EGL_NO_DISPLAY) {
        eglMakeCurrent(_platform->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    }
}

void GLContextManager::ReleaseContext(ContextHandle ctx) {
    if (!_platform || !ctx) return;

    auto* entry = (PlatformState::PoolEntry*)ctx;
    DoneCurrent(ctx);

    std::unique_lock<std::mutex> lock(_platform->poolMutex);
    _platform->pool.push_front(*entry);
    delete entry;
    lock.unlock();
    _platform->poolNotifier.notify_all();
}

void GLContextManager::ExecuteOnGLThread(std::function<void()> fn) {
    if (fn) fn();
}

void GLContextManager::Shutdown() {
    if (!_platform) return;

    std::unique_lock<std::mutex> lock(_platform->poolMutex);
    for (auto& e : _platform->pool) {
        eglDestroySurface(_platform->display, e.surface);
        eglDestroyContext(_platform->display, e.context);
    }
    _platform->pool.clear();
    if (_platform->sharedContext != EGL_NO_CONTEXT) {
        eglDestroyContext(_platform->display, _platform->sharedContext);
        _platform->sharedContext = EGL_NO_CONTEXT;
    }
    if (_platform->display != EGL_NO_DISPLAY) {
        eglTerminate(_platform->display);
        _platform->display = EGL_NO_DISPLAY;
    }
    _platform->contextCount = 0;
    lock.unlock();

    delete _platform;
    _platform = nullptr;
    _initialized = false;
}

void* GLContextManager::GetNativeDisplay() const {
    return _platform ? (void*)_platform->display : nullptr;
}

#else // !USE_GLES — legacy CGL implementation

#include <OpenGL/OpenGL.h>
#include <OpenGL/gl3.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

static constexpr int kMaxPoolSize = 24;

struct GLContextManager::PlatformState {
    CGLContextObj sharedContext = nullptr;       // Master context for resource sharing
    std::list<CGLContextObj> pool;               // Available contexts
    int contextCount = 0;                        // Total created
    std::mutex poolMutex;
    std::condition_variable poolNotifier;
};

// Select the best GPU (prefer eGPU, then most VRAM) and log GL info.
static void selectBestGPU(CGLContextObj ctx) {
    CGLSetCurrentContext(ctx);

    CGLPixelFormatObj pixelFormat = CGLGetPixelFormat(ctx);

    CGLRendererInfoObj rendInfo = nullptr;
    GLint nRenderers = 0;
    CGLQueryRendererInfo(0xffffffff, &rendInfo, &nRenderers);
    if (!rendInfo) return;

    struct GPUInfo {
        GLint rendererID = 0;
        GLint videoMemoryMB = 0;
        GLint eGpu = 0;
        GLint virtualScreen = -1;
    };

    GLint nVirtualScreens = 0;
    CGLDescribePixelFormat(pixelFormat, 0, kCGLPFAVirtualScreenCount, &nVirtualScreens);

    std::vector<GPUInfo> gpus(nRenderers);
    for (GLint i = 0; i < nRenderers; ++i) {
        CGLDescribeRenderer(rendInfo, i, kCGLRPRendererID, &gpus[i].rendererID);
        CGLDescribeRenderer(rendInfo, i, kCGLRPVideoMemoryMegabytes, &gpus[i].videoMemoryMB);
        CGLDescribeRenderer(rendInfo, i, (CGLRendererProperty)142 /*kCGLRPIsExternalGPU*/, &gpus[i].eGpu);
    }
    CGLDestroyRendererInfo(rendInfo);

    // Map virtual screens to renderer IDs
    for (GLint i = 0; i < nVirtualScreens; ++i) {
        CGLSetVirtualScreen(ctx, i);
        GLint rid = 0;
        CGLGetParameter(ctx, kCGLCPCurrentRendererID, &rid);
        for (GLint j = 0; j < nRenderers; ++j) {
            if (gpus[j].rendererID == rid) {
                gpus[j].virtualScreen = i;
            }
        }
    }

    // Prefer eGPU, else most VRAM
    bool found = false;
    int maxMem = 0;
    for (GLint i = 0; i < nRenderers; ++i) {
        if (gpus[i].eGpu) {
            CGLSetVirtualScreen(ctx, gpus[i].virtualScreen);
            found = true;
        } else {
            maxMem = std::max(maxMem, gpus[i].videoMemoryMB);
        }
    }
    if (!found) {
        for (GLint i = 0; i < nRenderers; ++i) {
            if (gpus[i].videoMemoryMB == maxMem && gpus[i].virtualScreen >= 0) {
                CGLSetVirtualScreen(ctx, gpus[i].virtualScreen);
                break;
            }
        }
    }

    const char* ver = (const char*)glGetString(GL_VERSION);
    const char* rend = (const char*)glGetString(GL_RENDERER);
    const char* vend = (const char*)glGetString(GL_VENDOR);
    spdlog::info("GLContextManager - glVer: {} ({}) ({})", ver ? ver : "?", rend ? rend : "?", vend ? vend : "?");

    CGLSetCurrentContext(nullptr);
}

static CGLContextObj createCGLContext(CGLContextObj shared) {
    // Pixel format attributes — matching the original ShaderEffect createContext()
    CGLPixelFormatAttribute attribs[] = {
        kCGLPFAMinimumPolicy,
        kCGLPFAAcceleratedCompute,
        kCGLPFAAllowOfflineRenderers,
        kCGLPFAColorSize, (CGLPixelFormatAttribute)32,
        kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute)kCGLOGLPVersion_3_2_Core,
        (CGLPixelFormatAttribute)0
    };

    CGLPixelFormatObj pixelFormat = nullptr;
    GLint numFormats = 0;
    CGLError err = CGLChoosePixelFormat(attribs, &pixelFormat, &numFormats);
    if (err != kCGLNoError || !pixelFormat) {
        // Fallback without AcceleratedCompute
        CGLPixelFormatAttribute fallback[] = {
            kCGLPFAMinimumPolicy,
            kCGLPFAColorSize, (CGLPixelFormatAttribute)32,
            kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute)kCGLOGLPVersion_3_2_Core,
            (CGLPixelFormatAttribute)0
        };
        err = CGLChoosePixelFormat(fallback, &pixelFormat, &numFormats);
        if (err != kCGLNoError || !pixelFormat) {
            spdlog::error("GLContextManager: CGLChoosePixelFormat failed: {}", (int)err);
            return nullptr;
        }
    }

    CGLContextObj ctx = nullptr;
    err = CGLCreateContext(pixelFormat, shared, &ctx);
    CGLReleasePixelFormat(pixelFormat);

    if (err != kCGLNoError || !ctx) {
        spdlog::error("GLContextManager: CGLCreateContext failed: {}", (int)err);
        return nullptr;
    }

    selectBestGPU(ctx);
    return ctx;
}

GLContextManager& GLContextManager::Instance() {
    static GLContextManager instance;
    return instance;
}

GLContextManager::~GLContextManager() {
    Shutdown();
}

void GLContextManager::Initialize(const InitParams& params) {
    if (_initialized) return;
    _params = params;
    _platform = new PlatformState();

    // Create the master shared context lazily on first AcquireContext()
    _initialized = true;
}

GLContextManager::ContextHandle GLContextManager::AcquireContext() {
    if (!_platform) return nullptr;

    std::unique_lock<std::mutex> lock(_platform->poolMutex);

    // Create master shared context on first call
    if (!_platform->sharedContext) {
        _platform->sharedContext = createCGLContext(nullptr);
        if (!_platform->sharedContext) return nullptr;
    }

    // Grow pool if below max
    if (_platform->pool.empty() && _platform->contextCount < kMaxPoolSize) {
        lock.unlock();
        CGLContextObj ctx = createCGLContext(_platform->sharedContext);
        lock.lock();
        if (ctx) {
            _platform->pool.push_front(ctx);
            ++_platform->contextCount;
        } else if (_platform->contextCount == 0) {
            // First-ever creation failed and there's nothing in flight to
            // wait for — fail fast instead of deadlocking on poolNotifier.
            return nullptr;
        }
    }

    // Wait for a context to become available
    while (_platform->pool.empty()) {
        _platform->poolNotifier.wait(lock);
    }

    CGLContextObj ctx = _platform->pool.front();
    _platform->pool.pop_front();
    return (ContextHandle)ctx;
}

bool GLContextManager::MakeCurrent(ContextHandle ctx) {
    return CGLSetCurrentContext((CGLContextObj)ctx) == kCGLNoError;
}

void GLContextManager::DoneCurrent(ContextHandle /*ctx*/) {
    CGLSetCurrentContext(nullptr);
}

void GLContextManager::ReleaseContext(ContextHandle ctx) {
    if (!_platform || !ctx) return;

    CGLSetCurrentContext(nullptr);

    std::unique_lock<std::mutex> lock(_platform->poolMutex);
    _platform->pool.push_front((CGLContextObj)ctx);
    lock.unlock();
    _platform->poolNotifier.notify_all();
}

void GLContextManager::ExecuteOnGLThread(std::function<void()> fn) {
    if (fn) fn();
}

void GLContextManager::Shutdown() {
    if (!_platform) return;

    std::unique_lock<std::mutex> lock(_platform->poolMutex);
    for (auto ctx : _platform->pool) {
        CGLDestroyContext(ctx);
    }
    _platform->pool.clear();
    if (_platform->sharedContext) {
        CGLDestroyContext(_platform->sharedContext);
        _platform->sharedContext = nullptr;
    }
    _platform->contextCount = 0;
    lock.unlock();

    delete _platform;
    _platform = nullptr;
    _initialized = false;
}

void* GLContextManager::GetNativeDisplay() const {
    return nullptr; // CGL has no EGL display
}

#pragma clang diagnostic pop

#endif // USE_GLES

// =========================================================================
// Windows — Hidden HWND + WGL implementation
// =========================================================================
#elif defined(_WIN32)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>

#include <future>

#include <GL/gl.h>
#include <GL/glext.h>
//#include <GL/wglext.h>
//#include <GL\glu.h>

// Define WGL extension function pointer if not already defined
#ifndef WGL_ARB_create_context
typedef HGLRC (WINAPI * PFNWGLCREATECONTEXTATTRIBSARBPROC) (HDC hDC, HGLRC hShareContext, const int *attribList);
#define WGL_CONTEXT_MAJOR_VERSION_ARB           0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB           0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB            0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB        0x00000001
#endif

// Upper bound on pool size.  In the default single-worker-thread mode the
// pool stays at 1; the cap only matters when SetBackgroundRenderEnabled(true)
// lets concurrent render threads acquire contexts directly.  Each HGLRC pins
// driver state (NVIDIA contexts are MB-scale) plus a Win32 dummy window —
// 24 matches the macOS cap and exceeds typical render thread counts.
static constexpr int kMaxPoolSize = 24;

struct GLContextManager::PlatformState {
    HGLRC sharedContext = nullptr;        // wx UI canvas HGLRC (not used for sharing on Windows)
    // Persistent root that every pool context shares with, so pool contexts
    // share GL objects with each other (programs/buffers/textures survive a
    // ShaderRenderCache swapping contexts each frame).  Not shared with the
    // wx canvas HGLRC: NVIDIA's one-current-per-share-group restriction
    // surfaces as wglMakeCurrent error 2004 when the UI thread holds the
    // canvas current.
    HWND  shaderShareRootHwnd = nullptr;
    HDC   shaderShareRootHdc  = nullptr;
    HGLRC shaderShareRoot     = nullptr;
    std::queue<void*> pool;  // queue of WinGLContextInfo*
    int contextCount = 0;
    std::mutex poolMutex;
    std::condition_variable poolNotifier;
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;
    std::once_flag bootstrapFlag;

    // Internal GL worker thread.  All GL operations on Windows run here
    // (HWND/HDC creation, wgl*, glGen*, glDelete*, draw, glReadPixels, etc.)
    // so that drivers which misbehave when GL is touched from arbitrary
    // render-pool threads see a stable single-thread caller.  The worker
    // is also the thread that owns the dummy HWNDs — Windows requires
    // window destruction on the creating thread.
    std::thread worker;
    std::queue<std::function<void()>> taskQueue;
    std::mutex taskMutex;
    std::condition_variable taskCv;
    std::once_flag workerStartFlag;
    bool workerStop = false;

    struct WinGLContextInfo {
        HGLRC context;
        HDC hdc;
        HWND hwnd;
    };
};

using PlatformState = GLContextManager::PlatformState;

static HWND createDummyWindow() {
    static bool registered = false;
    if (!registered) {
        WNDCLASSA wc = {};
        wc.lpfnWndProc = DefWindowProcA;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = "XLightsGLContextDummy";
        RegisterClassA(&wc);
        registered = true;
    }
    return CreateWindowExA(0, "XLightsGLContextDummy", "", 0, 0, 0, 1, 1,
                           nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
}

GLContextManager& GLContextManager::Instance() {
    static GLContextManager instance;
    return instance;
}

GLContextManager::~GLContextManager() {
    Shutdown();
}

void GLContextManager::Initialize(const InitParams& params) {
    if (_initialized) return;
    _params = params;
    _platform = new PlatformState();
    // Shared context and WGL bootstrap deferred to first AcquireContext()
    _initialized = true;
}

// Creates a 3.3-core (or 3.1-core fallback) HGLRC on a fresh dummy HWND.
// On success returns the HGLRC and writes the owning HWND/HDC to outHwnd/outHdc.
// On failure returns nullptr and releases the HWND/HDC.
static HGLRC createCoreContext(PlatformState* ps, HGLRC share,
                               HWND& outHwnd, HDC& outHdc) {
    HWND hwnd = createDummyWindow();
    HDC hdc = GetDC(hwnd);

    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    int pf = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, pf, &pfd);

    int attribs33[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };
    HGLRC ctx = ps->wglCreateContextAttribsARB(hdc, share, attribs33);
    if (!ctx) {
        int attribs31[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
            WGL_CONTEXT_MINOR_VERSION_ARB, 1,
            WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            0
        };
        ctx = ps->wglCreateContextAttribsARB(hdc, share, attribs31);
    }

    if (ctx) {
        outHwnd = hwnd;
        outHdc = hdc;
    } else {
        ReleaseDC(hwnd, hdc);
        DestroyWindow(hwnd);
    }
    return ctx;
}

// Bootstrap WGL: create a temp legacy context to load wglCreateContextAttribsARB,
// then obtain the shared HGLRC from the UI layer and create the shader share-root.
//
// Runs on the GL worker thread.  Windows requires DestroyWindow on the
// creating thread, so the dummy HWNDs are created and destroyed here.  The
// one piece that needs main-thread dispatch is getSharedGLContext — wx
// canvas access is main-thread-only.
static void bootstrapWGL(PlatformState* ps,
                  const GLContextManager::InitParams& params) {
    HWND tmpHwnd = createDummyWindow();
    HDC tmpDC = GetDC(tmpHwnd);

    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    int pf = ChoosePixelFormat(tmpDC, &pfd);
    SetPixelFormat(tmpDC, pf, &pfd);

    HGLRC tmpCtx = wglCreateContext(tmpDC);
    wglMakeCurrent(tmpDC, tmpCtx);

    ps->wglCreateContextAttribsARB =
        (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");

    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(tmpCtx);
    ReleaseDC(tmpHwnd, tmpDC);
    DestroyWindow(tmpHwnd);

    // Obtain the shared HGLRC from the UI layer.  This callback talks to wx
    // and must run on the main thread.
    if (params.getSharedGLContext) {
        if (params.mainThreadRunner) {
            params.mainThreadRunner([&]() {
                ps->sharedContext = (HGLRC)params.getSharedGLContext();
            });
        } else {
            ps->sharedContext = (HGLRC)params.getSharedGLContext();
        }
    }

    if (ps->wglCreateContextAttribsARB && !ps->shaderShareRoot) {
        ps->shaderShareRoot = createCoreContext(ps, nullptr,
                                                ps->shaderShareRootHwnd,
                                                ps->shaderShareRootHdc);
        if (ps->shaderShareRoot) {
            // Prime the share-root by making it current once.  Several drivers
            // (observed on Windows in GL-worker thread mode) refuse to make a
            // sharing child context current until the share parent itself has
            // been made current at least once — wglMakeCurrent on the child
            // fails with ERROR_INVALID_HANDLE, the version-logging code below
            // produces "? (?) (?)" output, and rendering silently disables
            // every shader effect ("Could not create/set OpenGL Context for
            // ShaderEffect" in the log).  macOS does the equivalent under
            // selectBestGPU().  If priming itself fails, tear the share-root
            // down so pool contexts fall back to non-sharing (functionally
            // correct, programs just won't be reused across pool contexts).
            if (wglMakeCurrent(ps->shaderShareRootHdc, ps->shaderShareRoot)) {
                const char* ver = (const char*)glGetString(GL_VERSION);
                const char* rend = (const char*)glGetString(GL_RENDERER);
                const char* vend = (const char*)glGetString(GL_VENDOR);
                spdlog::info("GLContextManager (share-root) - glVer: {} ({}) ({})",
                             ver ? ver : "?", rend ? rend : "?", vend ? vend : "?");
                wglMakeCurrent(ps->shaderShareRootHdc, nullptr);
            } else {
                spdlog::warn("GLContextManager: share-root wglMakeCurrent failed (GLE={}); "
                             "pool contexts will not share programs/buffers",
                             (unsigned)GetLastError());
                wglDeleteContext(ps->shaderShareRoot);
                if (ps->shaderShareRootHdc) {
                    ReleaseDC(ps->shaderShareRootHwnd, ps->shaderShareRootHdc);
                    ps->shaderShareRootHdc = nullptr;
                }
                if (ps->shaderShareRootHwnd) {
                    DestroyWindow(ps->shaderShareRootHwnd);
                    ps->shaderShareRootHwnd = nullptr;
                }
                ps->shaderShareRoot = nullptr;
            }
        } else {
            spdlog::warn("GLContextManager: shader share-root creation failed; "
                         "pool contexts will be isolated (program cache won't survive context shuffle)");
        }
    }
}

GLContextManager::ContextHandle GLContextManager::AcquireContext() {
    if (!_platform) return nullptr;

    // Callers reach AcquireContext from inside ExecuteOnGLThread, so we are
    // already on the GL worker thread (or the calling render thread when
    // background rendering is user-enabled).  Either way, no further
    // dispatch is needed — bootstrap, dummy HWND creation and wglCreateContext
    // all run on whichever thread we're on.
    std::call_once(_platform->bootstrapFlag, [this]() {
        bootstrapWGL(_platform, _params);
    });
    if (!_platform->wglCreateContextAttribsARB) return nullptr;

    std::unique_lock<std::mutex> lock(_platform->poolMutex);

    // Grow the pool if it's empty and we're under the cap.  Drop the lock
    // around the actual context creation — CreateWindowEx + WGL setup is
    // slow and other threads should be able to release into the pool while
    // we wait on the driver.  Share with the shader-internal share-root
    // (may be null if share-root creation failed — pool contexts then end
    // up isolated, which still works, just without cross-context resource
    // sharing).
    if (_platform->pool.empty() && _platform->contextCount < kMaxPoolSize) {
        lock.unlock();
        HWND hwnd = nullptr;
        HDC hdc = nullptr;
        HGLRC ctx = createCoreContext(_platform, _platform->shaderShareRoot, hwnd, hdc);
        PlatformState::WinGLContextInfo* info = nullptr;
        if (ctx) {
            info = new PlatformState::WinGLContextInfo{ctx, hdc, hwnd};
            wglMakeCurrent(hdc, ctx);
            const char* ver = (const char*)glGetString(GL_VERSION);
            const char* rend = (const char*)glGetString(GL_RENDERER);
            const char* vend = (const char*)glGetString(GL_VENDOR);
            spdlog::info("GLContextManager - glVer: {} ({}) ({})",
                         ver ? ver : "?", rend ? rend : "?", vend ? vend : "?");
            wglMakeCurrent(hdc, nullptr);
        } else {
            spdlog::error("GLContextManager: wglCreateContextAttribsARB failed");
        }
        lock.lock();
        if (info) {
            _platform->pool.push(info);
            ++_platform->contextCount;
        } else if (_platform->contextCount == 0) {
            // First-ever creation failed and there's nothing in flight to
            // wait for — fail fast instead of deadlocking on poolNotifier.
            return nullptr;
        }
        // Otherwise: creation failed but other contexts exist in flight
        // (held by other threads).  Fall through to the wait loop and pick
        // one up when it's returned.
    }

    // Wait for a context to become available (pool capped at kMaxPoolSize).
    while (_platform->pool.empty()) {
        _platform->poolNotifier.wait(lock);
    }

    auto* info = (PlatformState::WinGLContextInfo*)_platform->pool.front();
    _platform->pool.pop();
    return (ContextHandle)info;
}

bool GLContextManager::MakeCurrent(ContextHandle ctx) {
    auto* info = (PlatformState::WinGLContextInfo*)ctx;
    if (!info) return false;
    for (int x = 0; x < 10; ++x) {
        if (wglMakeCurrent(info->hdc, info->context)) return true;
        DWORD gle = GetLastError();
        if (gle == ERROR_INVALID_HANDLE) {
            // Bumped from debug to warn so a recurrence is visible in user
            // logs without needing a debug-level config.  Common causes:
            // share-root not primed (see bootstrapWGL), HDC/HWND destroyed
            // out from under us, or the HGLRC being current on another
            // thread.
            spdlog::warn("GLContextManager: wglMakeCurrent invalid handle - "
                         "hwnd_valid={} dc_type={} hglrc={:p}",
                         (int)IsWindow(info->hwnd),
                         (int)GetObjectType(info->hdc),
                         (void*)info->context);
            return false;
        }
        if (gle == 2004) {
            // NVIDIA driver-specific: GPU busy (typically CUDA/NVDEC contention).
            // No point retrying in a tight loop — caller will skip this frame
            // and return the context to the pool for the next frame.
            spdlog::warn("GLContextManager: wglMakeCurrent GPU busy (GLE=2004), skipping frame");
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    spdlog::error("GLContextManager: wglMakeCurrent failed after retries (GLE={})", GetLastError());
    return false;
}

void GLContextManager::DoneCurrent(ContextHandle ctx) {
    auto* info = (PlatformState::WinGLContextInfo*)ctx;
    if (info) {
        wglMakeCurrent(info->hdc, nullptr);
    }
}

void GLContextManager::ReleaseContext(ContextHandle ctx) {
    if (!_platform || !ctx) return;
    DoneCurrent(ctx);
    {
        std::unique_lock<std::mutex> lock(_platform->poolMutex);
        _platform->pool.push(ctx);
    }
    _platform->poolNotifier.notify_one();
}

void GLContextManager::ExecuteOnGLThread(std::function<void()> fn) {
    if (!fn || !_platform) return;

    if (_backgroundRenderEnabled) {
        // User opted in to direct rendering — driver expected to handle MT GL.
        fn();
        return;
    }

    // Lazy-start the GL worker thread on first dispatch.
    std::call_once(_platform->workerStartFlag, [this]() {
        _platform->worker = std::thread([this]() {
            for (;;) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(_platform->taskMutex);
                    _platform->taskCv.wait(lock, [this]() {
                        return _platform->workerStop || !_platform->taskQueue.empty();
                    });
                    if (_platform->taskQueue.empty()) {
                        // Stop signal with no remaining work.
                        return;
                    }
                    task = std::move(_platform->taskQueue.front());
                    _platform->taskQueue.pop();
                }
                task();
                // Pump any pending messages for dummy windows owned by this
                // thread.  WGL drivers sometimes post internal messages; if
                // the queue fills up wglMakeCurrent can fail on NVIDIA.
                MSG msg;
                while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE | PM_NOYIELD))
                    DispatchMessageA(&msg);
            }
        });
    });

    auto promise = std::make_shared<std::promise<void>>();
    auto future = promise->get_future();

    {
        std::unique_lock<std::mutex> lock(_platform->taskMutex);
        _platform->taskQueue.push([fn = std::move(fn), promise]() {
            try {
                fn();
                promise->set_value();
            } catch (...) {
                promise->set_exception(std::current_exception());
            }
        });
    }
    _platform->taskCv.notify_one();
    future.get();  // re-throws any exception from fn
}

void GLContextManager::Shutdown() {
    if (!_platform) return;

    auto destroyPool = [this]() {
        while (!_platform->pool.empty()) {
            auto* info = (PlatformState::WinGLContextInfo*)_platform->pool.front();
            _platform->pool.pop();
            wglDeleteContext(info->context);
            ReleaseDC(info->hwnd, info->hdc);
            DestroyWindow(info->hwnd);
            delete info;
        }
        _platform->contextCount = 0;
        // Share-root must be torn down after its dependent pool contexts.
        if (_platform->shaderShareRoot) {
            wglDeleteContext(_platform->shaderShareRoot);
            _platform->shaderShareRoot = nullptr;
        }
        if (_platform->shaderShareRootHwnd) {
            if (_platform->shaderShareRootHdc) {
                ReleaseDC(_platform->shaderShareRootHwnd, _platform->shaderShareRootHdc);
                _platform->shaderShareRootHdc = nullptr;
            }
            DestroyWindow(_platform->shaderShareRootHwnd);
            _platform->shaderShareRootHwnd = nullptr;
        }
    };

    // If the worker thread is running, post pool teardown to it (so
    // wglDeleteContext + DestroyWindow run on the thread that created the
    // contexts and windows — Windows requires window destruction on the
    // creating thread), then signal stop and join.
    if (_platform->worker.joinable()) {
        std::promise<void> drained;
        auto drainedFuture = drained.get_future();
        {
            std::unique_lock<std::mutex> lock(_platform->taskMutex);
            _platform->taskQueue.push([&]() {
                destroyPool();
                drained.set_value();
            });
        }
        _platform->taskCv.notify_one();
        drainedFuture.get();

        {
            std::unique_lock<std::mutex> lock(_platform->taskMutex);
            _platform->workerStop = true;
        }
        _platform->taskCv.notify_one();
        _platform->worker.join();
    } else {
        // Worker never started; tear down inline.  Safe because no contexts
        // can have been created without going through the worker.
        destroyPool();
    }

    delete _platform;
    _platform = nullptr;
    _initialized = false;
}

void* GLContextManager::GetNativeDisplay() const {
    return nullptr; // WGL has no EGL display
}

// =========================================================================
// Linux — GLX + Pbuffer (X11/XWayland) with EGL + Pbuffer fallback (Wayland)
//
// Creates GL contexts entirely independent of the wx canvas hierarchy so
// shader rendering never pollutes the state of any visible canvas.
//
// Strategy:
//   1. Try GLX with a pbuffer (works on X11 and XWayland).
//   2. If no X11 display is available (pure Wayland), fall back to EGL with
//      a pbuffer surface using EGL_DEFAULT_DISPLAY.
// =========================================================================
#else

#include <GL/glx.h>
#include <GL/glxext.h>
#include <X11/Xlib.h>
#include <EGL/egl.h>
#include <GL/gl.h>

// One context per background render thread, plus one for the main thread.
// Mirrors the macOS CGL pool size.
static constexpr int kMaxPoolSize = 24;

struct GLContextManager::PlatformState {
    // GLX path (X11 / XWayland)
    Display*     xDisplay  = nullptr;
    GLXFBConfig  fbConfig  = nullptr;

    // EGL path (native Wayland fallback)
    EGLDisplay   eglDisplay = EGL_NO_DISPLAY;
    EGLConfig    eglConfig  = nullptr;
    bool         useEGL     = false;

    struct PoolEntry {
        // GLX
        GLXContext  glxContext = nullptr;
        GLXPbuffer  glxPbuffer = 0;
        // EGL
        EGLContext  eglContext = EGL_NO_CONTEXT;
        EGLSurface  eglSurface = EGL_NO_SURFACE;
    };

    std::list<PoolEntry>     pool;
    int                      contextCount = 0;
    std::mutex               poolMutex;
    std::condition_variable  poolNotifier;
    std::once_flag           initFlag;
    bool                     initOk = false;
};

using PlatformStateLinux = GLContextManager::PlatformState;

// ---- GLX path ----

static bool initGLX(PlatformStateLinux* ps) {
    ps->xDisplay = XOpenDisplay(nullptr);
    if (!ps->xDisplay) {
        spdlog::info("GLContextManager: no X11 display (likely pure Wayland), trying EGL");
        return false;
    }

    int glxMaj = 0, glxMin = 0;
    if (!glXQueryVersion(ps->xDisplay, &glxMaj, &glxMin) ||
        glxMaj < 1 || (glxMaj == 1 && glxMin < 3)) {
        spdlog::warn("GLContextManager: GLX 1.3+ required (got {}.{}), trying EGL", glxMaj, glxMin);
        XCloseDisplay(ps->xDisplay);
        ps->xDisplay = nullptr;
        return false;
    }

    static const int fbAttribs[] = {
        GLX_RENDER_TYPE,   GLX_RGBA_BIT,
        GLX_DRAWABLE_TYPE, GLX_PBUFFER_BIT,
        GLX_RED_SIZE,      8,
        GLX_GREEN_SIZE,    8,
        GLX_BLUE_SIZE,     8,
        GLX_ALPHA_SIZE,    8,
        None
    };
    int nConfigs = 0;
    GLXFBConfig* configs = glXChooseFBConfig(
        ps->xDisplay, DefaultScreen(ps->xDisplay), fbAttribs, &nConfigs);
    if (!configs || nConfigs == 0) {
        spdlog::warn("GLContextManager: glXChooseFBConfig failed, trying EGL");
        XCloseDisplay(ps->xDisplay);
        ps->xDisplay = nullptr;
        return false;
    }
    ps->fbConfig = configs[0];
    XFree(configs);
    return true;
}

static PlatformStateLinux::PoolEntry createGLXPoolEntry(PlatformStateLinux* ps) {
    PlatformStateLinux::PoolEntry entry;

    auto createCtxARB =
        (GLXContext(*)(Display*, GLXFBConfig, GLXContext, Bool, const int*))
        glXGetProcAddressARB((const GLubyte*)"glXCreateContextAttribsARB");

    if (createCtxARB) {
        static const int ctx33[] = {
            GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
            GLX_CONTEXT_MINOR_VERSION_ARB, 3,
            GLX_CONTEXT_PROFILE_MASK_ARB,  GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
            None
        };
        // glXCreateContextAttribsARB triggers an X11 protocol error (caught by
        // the default handler, which calls exit()) if the driver rejects the
        // requested profile.  Suppress it with a no-op error handler so the
        // nullptr return value can be tested and the legacy fallback used.
        auto prevHandler = XSetErrorHandler([](Display*, XErrorEvent*) { return 0; });
        entry.glxContext = createCtxARB(ps->xDisplay, ps->fbConfig, nullptr, True, ctx33);
        XSync(ps->xDisplay, False);  // flush any pending X11 error
        XSetErrorHandler(prevHandler);
    }
    if (!entry.glxContext) {
        entry.glxContext = glXCreateNewContext(
            ps->xDisplay, ps->fbConfig, GLX_RGBA_TYPE, nullptr, True);
    }
    if (!entry.glxContext) {
        spdlog::error("GLContextManager: GLX context creation failed");
        return {};
    }

    static const int pbAttribs[] = { GLX_PBUFFER_WIDTH, 1, GLX_PBUFFER_HEIGHT, 1, None };
    entry.glxPbuffer = glXCreatePbuffer(ps->xDisplay, ps->fbConfig, pbAttribs);
    if (!entry.glxPbuffer) {
        spdlog::error("GLContextManager: glXCreatePbuffer failed");
        glXDestroyContext(ps->xDisplay, entry.glxContext);
        return {};
    }

    glXMakeCurrent(ps->xDisplay, entry.glxPbuffer, entry.glxContext);
    const char* ver  = (const char*)glGetString(GL_VERSION);
    const char* rend = (const char*)glGetString(GL_RENDERER);
    const char* vend = (const char*)glGetString(GL_VENDOR);
    spdlog::info("GLContextManager (GLX) - glVer: {} ({}) ({})",
                 ver ? ver : "?", rend ? rend : "?", vend ? vend : "?");
    glXMakeCurrent(ps->xDisplay, None, nullptr);
    return entry;
}

// ---- EGL path (Wayland fallback) ----

static bool initEGL(PlatformStateLinux* ps) {
    ps->eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (ps->eglDisplay == EGL_NO_DISPLAY) {
        spdlog::error("GLContextManager: eglGetDisplay failed");
        return false;
    }

    EGLint major = 0, minor = 0;
    if (!eglInitialize(ps->eglDisplay, &major, &minor)) {
        spdlog::error("GLContextManager: eglInitialize failed: 0x{:X}", eglGetError());
        ps->eglDisplay = EGL_NO_DISPLAY;
        return false;
    }
    spdlog::info("GLContextManager: EGL {}.{} (Wayland path)", major, minor);

    eglBindAPI(EGL_OPENGL_API);

    static const EGLint configAttribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_SURFACE_TYPE,    EGL_PBUFFER_BIT,
        EGL_RED_SIZE,        8,
        EGL_GREEN_SIZE,      8,
        EGL_BLUE_SIZE,       8,
        EGL_ALPHA_SIZE,      8,
        EGL_NONE
    };
    EGLint numConfigs = 0;
    if (!eglChooseConfig(ps->eglDisplay, configAttribs, &ps->eglConfig, 1, &numConfigs)
        || numConfigs == 0) {
        spdlog::error("GLContextManager: eglChooseConfig failed: 0x{:X}", eglGetError());
        eglTerminate(ps->eglDisplay);
        ps->eglDisplay = EGL_NO_DISPLAY;
        return false;
    }
    return true;
}

static PlatformStateLinux::PoolEntry createEGLPoolEntry(PlatformStateLinux* ps) {
    PlatformStateLinux::PoolEntry entry;

    static const EGLint ctxAttribs[] = {
        EGL_CONTEXT_MAJOR_VERSION, 3,
        EGL_CONTEXT_MINOR_VERSION, 3,
        EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
        EGL_NONE
    };
    entry.eglContext = eglCreateContext(
        ps->eglDisplay, ps->eglConfig, EGL_NO_CONTEXT, ctxAttribs);
    if (entry.eglContext == EGL_NO_CONTEXT) {
        // Fallback: no version constraints
        static const EGLint ctxAttribsFallback[] = { EGL_NONE };
        entry.eglContext = eglCreateContext(
            ps->eglDisplay, ps->eglConfig, EGL_NO_CONTEXT, ctxAttribsFallback);
    }
    if (entry.eglContext == EGL_NO_CONTEXT) {
        spdlog::error("GLContextManager: eglCreateContext failed: 0x{:X}", eglGetError());
        return {};
    }

    static const EGLint pbAttribs[] = { EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE };
    entry.eglSurface = eglCreatePbufferSurface(ps->eglDisplay, ps->eglConfig, pbAttribs);
    if (entry.eglSurface == EGL_NO_SURFACE) {
        spdlog::error("GLContextManager: eglCreatePbufferSurface failed: 0x{:X}", eglGetError());
        eglDestroyContext(ps->eglDisplay, entry.eglContext);
        entry.eglContext = EGL_NO_CONTEXT;
        return {};
    }

    eglMakeCurrent(ps->eglDisplay, entry.eglSurface, entry.eglSurface, entry.eglContext);
    const char* ver  = (const char*)glGetString(GL_VERSION);
    const char* rend = (const char*)glGetString(GL_RENDERER);
    const char* vend = (const char*)glGetString(GL_VENDOR);
    spdlog::info("GLContextManager (EGL) - glVer: {} ({}) ({})",
                 ver ? ver : "?", rend ? rend : "?", vend ? vend : "?");
    eglMakeCurrent(ps->eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    return entry;
}

// ---- Common GLContextManager methods ----

GLContextManager& GLContextManager::Instance() {
    static GLContextManager instance;
    return instance;
}

GLContextManager::~GLContextManager() {
    Shutdown();
}

void GLContextManager::Initialize(const InitParams& params) {
    if (_initialized) return;
    _params = params;
    _platform = new PlatformState();
    _initialized = true;
}

GLContextManager::ContextHandle GLContextManager::AcquireContext() {
    if (!_platform) return nullptr;

    std::call_once(_platform->initFlag, [this]() {
        if (initGLX(_platform)) {
            _platform->useEGL = false;
            _platform->initOk = true;
        } else if (initEGL(_platform)) {
            _platform->useEGL = true;
            _platform->initOk = true;
        }
    });
    if (!_platform->initOk) return nullptr;

    std::unique_lock<std::mutex> lock(_platform->poolMutex);

    if (_platform->pool.empty() && _platform->contextCount < kMaxPoolSize) {
        lock.unlock();
        PlatformState::PoolEntry entry = _platform->useEGL
            ? createEGLPoolEntry(_platform)
            : createGLXPoolEntry(_platform);
        lock.lock();
        bool ok = _platform->useEGL
            ? (entry.eglContext != EGL_NO_CONTEXT)
            : (entry.glxContext != nullptr);
        if (ok) {
            _platform->pool.push_front(entry);
            ++_platform->contextCount;
        }
    }

    while (_platform->pool.empty()) {
        _platform->poolNotifier.wait(lock);
    }

    auto entry = _platform->pool.front();
    _platform->pool.pop_front();
    return (ContextHandle)(new PlatformState::PoolEntry(entry));
}

bool GLContextManager::MakeCurrent(ContextHandle ctx) {
    auto* entry = (PlatformState::PoolEntry*)ctx;
    if (!entry || !_platform) return false;
    if (_platform->useEGL) {
        return eglMakeCurrent(_platform->eglDisplay, entry->eglSurface, entry->eglSurface, entry->eglContext) == EGL_TRUE;
    } else {
        return glXMakeCurrent(_platform->xDisplay, entry->glxPbuffer, entry->glxContext) == True;
    }
}

void GLContextManager::DoneCurrent(ContextHandle /*ctx*/) {
    // Release the context so wx canvases can bind their own contexts freely.
    if (!_platform) return;
    if (_platform->useEGL) {
        eglMakeCurrent(_platform->eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    } else if (_platform->xDisplay) {
        glXMakeCurrent(_platform->xDisplay, None, nullptr);
    }
}

void GLContextManager::ReleaseContext(ContextHandle ctx) {
    if (!_platform || !ctx) return;
    // Callers (ShaderEffect::UnsetGLContext) must have already called DoneCurrent
    // before ReleaseContext.  Do not call it again here to avoid a redundant
    // glX/eglMakeCurrent(None) that could unset a context another caller set.
    auto* entry = (PlatformState::PoolEntry*)ctx;
    std::unique_lock<std::mutex> lock(_platform->poolMutex);
    _platform->pool.push_front(*entry);
    delete entry;
    lock.unlock();
    _platform->poolNotifier.notify_all();
}

void GLContextManager::ExecuteOnGLThread(std::function<void()> fn) {
    if (fn) fn();
}

void GLContextManager::Shutdown() {
    if (!_platform) return;

    {
        std::unique_lock<std::mutex> lock(_platform->poolMutex);
        for (auto& e : _platform->pool) {
            if (_platform->useEGL) {
                if (_platform->eglDisplay != EGL_NO_DISPLAY) {
                    eglDestroySurface(_platform->eglDisplay, e.eglSurface);
                    eglDestroyContext(_platform->eglDisplay, e.eglContext);
                }
            } else if (_platform->xDisplay) {
                glXDestroyPbuffer(_platform->xDisplay, e.glxPbuffer);
                glXDestroyContext(_platform->xDisplay, e.glxContext);
            }
        }
        _platform->pool.clear();
    }

    if (_platform->useEGL && _platform->eglDisplay != EGL_NO_DISPLAY) {
        eglTerminate(_platform->eglDisplay);
        _platform->eglDisplay = EGL_NO_DISPLAY;
    } else if (_platform->xDisplay) {
        XCloseDisplay(_platform->xDisplay);
        _platform->xDisplay = nullptr;
    }

    delete _platform;
    _platform = nullptr;
    _initialized = false;
}

void* GLContextManager::GetNativeDisplay() const {
    // Returns nullptr on Linux: the EGL display is internal to this class and
    // not intended for external texture-sharing.  Callers requiring the
    // EGLDisplay for interop must use platform-specific APIs directly.
    return nullptr;
}

#endif
