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

static bool initEGLDisplay(PlatformStateEGL* ps) {
    // Request ANGLE's Metal backend
    const EGLAttrib displayAttribs[] = {
        EGL_PLATFORM_ANGLE_TYPE_ANGLE,
        EGL_PLATFORM_ANGLE_TYPE_METAL_ANGLE,
        EGL_NONE
    };

    ps->display = eglGetPlatformDisplay(
        EGL_PLATFORM_ANGLE_ANGLE,
        nullptr,
        displayAttribs);

    if (ps->display == EGL_NO_DISPLAY) {
        spdlog::error("GLContextManager: eglGetPlatformDisplay failed");
        return false;
    }

    EGLint major = 0, minor = 0;
    if (!eglInitialize(ps->display, &major, &minor)) {
        spdlog::error("GLContextManager: eglInitialize failed: 0x{:X}", eglGetError());
        return false;
    }
    spdlog::info("GLContextManager: EGL {}.{} (ANGLE/Metal)", major, minor);

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
        if (!initEGLDisplay(_platform)) return nullptr;
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

void GLContextManager::MakeCurrent(ContextHandle ctx) {
    auto* entry = (PlatformState::PoolEntry*)ctx;
    if (entry && _platform) {
        eglMakeCurrent(_platform->display, entry->surface, entry->surface, entry->context);
    }
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

bool GLContextManager::CanRenderOnBackgroundThread() const {
    return true; // EGL contexts are thread-safe with separate contexts
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

void GLContextManager::MakeCurrent(ContextHandle ctx) {
    CGLSetCurrentContext((CGLContextObj)ctx);
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

bool GLContextManager::CanRenderOnBackgroundThread() const {
    return true; // macOS CGL is thread-safe with separate contexts
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

struct GLContextManager::PlatformState {
    HGLRC sharedContext = nullptr;
    std::queue<void*> pool;  // queue of WinGLContextInfo*
    std::mutex poolMutex;
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;
    std::once_flag bootstrapFlag;

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

// Bootstrap WGL: create a temp legacy context to load wglCreateContextAttribsARB,
// then obtain the shared HGLRC from the UI layer.  Must run on the main thread.
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

    // Obtain the shared HGLRC from the UI layer
    if (params.getSharedGLContext) {
        ps->sharedContext = (HGLRC)params.getSharedGLContext();
    }
}

GLContextManager::ContextHandle GLContextManager::AcquireContext() {
    if (!_platform) return nullptr;

    // Lazy bootstrap: load WGL functions and shared context on first call (thread-safe)
    std::call_once(_platform->bootstrapFlag, [this]() {
        if (_params.mainThreadRunner) {
            _params.mainThreadRunner([this]() {
                bootstrapWGL(_platform, _params);
            });
        } else {
            bootstrapWGL(_platform, _params);
        }
    });
    if (!_platform->wglCreateContextAttribsARB) return nullptr;

    {
        std::unique_lock<std::mutex> lock(_platform->poolMutex);
        if (!_platform->pool.empty()) {
            auto* info = (PlatformState::WinGLContextInfo*)_platform->pool.front();
            _platform->pool.pop();
            return (ContextHandle)info;
        }
    }

    // Create a new context — must happen on main thread
    PlatformState::WinGLContextInfo* info = nullptr;
    auto createFn = [this, &info]() {
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
        HGLRC ctx = _platform->wglCreateContextAttribsARB(hdc, _platform->sharedContext, attribs33);
        if (!ctx) {
            // Fallback to 3.1
            int attribs31[] = {
                WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
                WGL_CONTEXT_MINOR_VERSION_ARB, 1,
                WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                0
            };
            ctx = _platform->wglCreateContextAttribsARB(hdc, _platform->sharedContext, attribs31);
        }

        if (ctx) {
            info = new PlatformState::WinGLContextInfo{ctx, hdc, hwnd};
            // Log GL version info for diagnostics
            wglMakeCurrent(hdc, ctx);
            const char* ver = (const char*)glGetString(GL_VERSION);
            const char* rend = (const char*)glGetString(GL_RENDERER);
            const char* vend = (const char*)glGetString(GL_VENDOR);
            spdlog::info("GLContextManager - glVer: {} ({}) ({})",
                         ver ? ver : "?", rend ? rend : "?", vend ? vend : "?");
            wglMakeCurrent(hdc, nullptr);
        } else {
            ReleaseDC(hwnd, hdc);
            DestroyWindow(hwnd);
            spdlog::error("GLContextManager: wglCreateContextAttribsARB failed");
        }
    };

    if (_params.mainThreadRunner) {
        _params.mainThreadRunner(createFn);
    } else {
        createFn();
    }

    return (ContextHandle)info;
}

void GLContextManager::MakeCurrent(ContextHandle ctx) {
    auto* info = (PlatformState::WinGLContextInfo*)ctx;
    if (info) {
        for (int x = 0; x < 10; ++x) {
            if (wglMakeCurrent(info->hdc, info->context)) return;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        spdlog::error("GLContextManager: wglMakeCurrent failed after retries");
    }
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
    std::unique_lock<std::mutex> lock(_platform->poolMutex);
    _platform->pool.push(ctx);
}

bool GLContextManager::CanRenderOnBackgroundThread() const {
    return _backgroundRenderEnabled; // User-configurable; defaults to false due to driver crashes
}

void GLContextManager::Shutdown() {
    if (!_platform) return;

    while (!_platform->pool.empty()) {
        auto* info = (PlatformState::WinGLContextInfo*)_platform->pool.front();
        _platform->pool.pop();
        wglDeleteContext(info->context);
        ReleaseDC(info->hwnd, info->hdc);
        DestroyWindow(info->hwnd);
        delete info;
    }

    delete _platform;
    _platform = nullptr;
    _initialized = false;
}

// =========================================================================
// Linux — Callback-based (main thread only)
// =========================================================================
#else

struct GLContextManager::PlatformState {
    // No pooling needed — Linux uses main-thread-only rendering
};

static constexpr intptr_t kLinuxSentinelHandle = 0x1;

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
    // Call the UI-provided callback to activate the GL context
    if (_params.activateMainContext) {
        _params.activateMainContext();
    }
    return (ContextHandle)kLinuxSentinelHandle;
}

void GLContextManager::MakeCurrent(ContextHandle /*ctx*/) {
    // Already made current in AcquireContext
}

void GLContextManager::DoneCurrent(ContextHandle /*ctx*/) {
    // No-op; deactivation happens in ReleaseContext
}

void GLContextManager::ReleaseContext(ContextHandle /*ctx*/) {
    if (_params.deactivateMainContext) {
        _params.deactivateMainContext();
    }
}

bool GLContextManager::CanRenderOnBackgroundThread() const {
    return false; // Linux does not support background shader rendering
}

void GLContextManager::Shutdown() {
    delete _platform;
    _platform = nullptr;
    _initialized = false;
}

#endif
