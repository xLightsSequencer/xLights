/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "AngleEGL.h"

#include <atomic>
#include <functional>
#include <mutex>
#include <thread>

#include <log.h>

// ===========================================================================
// ANGLE build (USE_GLES, non-Apple): real EGL implementation.  All EGL/GLES
// headers are confined to this translation unit (see AngleEGL.h for why).
// The backend is a COMPILE-TIME choice — the ANGLE binary links libGLESv2 +
// libEGL and NOT opengl32/libGL, so gl* throughout the codebase bind directly
// to ANGLE.  Apple's USE_GLES path uses its own EGL plumbing in
// GLContextManager, so this module is Win/Linux only.
// ===========================================================================
#if defined(USE_GLES) && !defined(__APPLE__)

#define EGL_EGL_PROTOTYPES 1
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglext_angle.h>

#if defined(_WIN32) && !defined(XLIGHTS_CMAKE_BUILD)
// MSBuild (non-cmake) ANGLE build: link the ANGLE import libs for the whole
// binary (one #pragma comment(lib) affects the entire link).  This is the
// ANGLE variant, so opengl32 is intentionally NOT linked.
#pragma comment(lib, "libEGL")
#pragma comment(lib, "libGLESv2")
#endif

namespace {
std::mutex g_mutex;
EGLDisplay g_display = EGL_NO_DISPLAY;
EGLConfig  g_config = nullptr;
EGLContext g_rootContext = EGL_NO_CONTEXT;
bool       g_initialized = false;

struct EGLHandle {
    EGLContext context = EGL_NO_CONTEXT;
    EGLSurface surface = EGL_NO_SURFACE;
    bool       isWindow = false;
};

EGLint angleBackendType() {
#if defined(_WIN32)
    return EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE;
#else
    // Vulkan is ANGLE's modern Linux backend; ANGLE falls back internally if
    // Vulkan is unavailable.
    return EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE;
#endif
}

// Caller must hold g_mutex.
bool initLocked(uint64_t gpuDeviceId) {
    if (g_initialized) {
        return true;
    }

    EGLAttrib displayAttribs[] = {
        EGL_PLATFORM_ANGLE_TYPE_ANGLE, angleBackendType(),
        EGL_NONE, EGL_NONE,   // slot for device id (high) — filled below
        EGL_NONE, EGL_NONE,   // slot for device id (low)
        EGL_NONE
    };
    int idx = 2;
    if (gpuDeviceId != 0) {
        displayAttribs[idx++] = EGL_PLATFORM_ANGLE_DEVICE_ID_HIGH_ANGLE;
        displayAttribs[idx++] = static_cast<EGLAttrib>(gpuDeviceId >> 32);
        displayAttribs[idx++] = EGL_PLATFORM_ANGLE_DEVICE_ID_LOW_ANGLE;
        displayAttribs[idx++] = static_cast<EGLAttrib>(gpuDeviceId & 0xFFFFFFFF);
        displayAttribs[idx] = EGL_NONE;
    }

    g_display = eglGetPlatformDisplay(EGL_PLATFORM_ANGLE_ANGLE,
                                      reinterpret_cast<void*>(EGL_DEFAULT_DISPLAY),
                                      displayAttribs);
    if (g_display == EGL_NO_DISPLAY) {
        spdlog::error("AngleEGL: eglGetPlatformDisplay failed: 0x{:X}", eglGetError());
        return false;
    }

    EGLint major = 0, minor = 0;
    if (!eglInitialize(g_display, &major, &minor)) {
        spdlog::error("AngleEGL: eglInitialize failed: 0x{:X}", eglGetError());
        g_display = EGL_NO_DISPLAY;
        return false;
    }
    spdlog::info("AngleEGL: EGL {}.{} initialized (ANGLE)", major, minor);

    // One config that supports BOTH window (on-screen canvas) and pbuffer
    // (off-screen shader pool) surfaces, with depth+stencil for 3D previews.
    const EGLint configAttribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_SURFACE_TYPE,    EGL_WINDOW_BIT | EGL_PBUFFER_BIT,
        EGL_RED_SIZE,        8,
        EGL_GREEN_SIZE,      8,
        EGL_BLUE_SIZE,       8,
        EGL_ALPHA_SIZE,      8,
        EGL_DEPTH_SIZE,      24,
        EGL_STENCIL_SIZE,    8,
        EGL_NONE
    };
    EGLint numConfigs = 0;
    if (!eglChooseConfig(g_display, configAttribs, &g_config, 1, &numConfigs) || numConfigs == 0) {
        spdlog::error("AngleEGL: eglChooseConfig failed: 0x{:X}", eglGetError());
        eglTerminate(g_display);
        g_display = EGL_NO_DISPLAY;
        return false;
    }

    // Root share context — never rendered to; it is the share parent so every
    // window/pbuffer context created later shares one GL-object namespace.
    const EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
    g_rootContext = eglCreateContext(g_display, g_config, EGL_NO_CONTEXT, contextAttribs);
    if (g_rootContext == EGL_NO_CONTEXT) {
        spdlog::error("AngleEGL: root eglCreateContext failed: 0x{:X}", eglGetError());
        eglTerminate(g_display);
        g_display = EGL_NO_DISPLAY;
        return false;
    }

    g_initialized = true;
    return true;
}

EGLContext createSharedContext() {
    const EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
    EGLContext ctx = eglCreateContext(g_display, g_config, g_rootContext, contextAttribs);
    if (ctx == EGL_NO_CONTEXT) {
        spdlog::error("AngleEGL: eglCreateContext failed: 0x{:X}", eglGetError());
    }
    return ctx;
}
} // namespace

namespace xlAngleEGL {

bool IsCompiledIn() { return true; }

bool Initialize(uint64_t gpuDeviceId) {
    std::lock_guard<std::mutex> lock(g_mutex);
    return initLocked(gpuDeviceId);
}

bool IsInitialized() {
    std::lock_guard<std::mutex> lock(g_mutex);
    return g_initialized;
}

Handle CreateWindowContext(void* nativeWindow, int width, int height) {
    std::lock_guard<std::mutex> lock(g_mutex);
    if (!g_initialized || !nativeWindow) {
        return nullptr;
    }
    EGLContext ctx = createSharedContext();
    if (ctx == EGL_NO_CONTEXT) {
        return nullptr;
    }
    // ANGLE tracks the HWND client rect itself; width/height are advisory.
    EGLSurface surf = eglCreateWindowSurface(
        g_display, g_config,
        reinterpret_cast<EGLNativeWindowType>(nativeWindow), nullptr);
    if (surf == EGL_NO_SURFACE) {
        spdlog::error("AngleEGL: eglCreateWindowSurface failed: 0x{:X}", eglGetError());
        eglDestroyContext(g_display, ctx);
        return nullptr;
    }
    return new EGLHandle{ ctx, surf, true };
}

Handle CreatePbufferContext() {
    std::lock_guard<std::mutex> lock(g_mutex);
    if (!g_initialized) {
        return nullptr;
    }
    EGLContext ctx = createSharedContext();
    if (ctx == EGL_NO_CONTEXT) {
        return nullptr;
    }
    const EGLint pbAttribs[] = { EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE };
    EGLSurface surf = eglCreatePbufferSurface(g_display, g_config, pbAttribs);
    if (surf == EGL_NO_SURFACE) {
        spdlog::error("AngleEGL: eglCreatePbufferSurface failed: 0x{:X}", eglGetError());
        eglDestroyContext(g_display, ctx);
        return nullptr;
    }
    return new EGLHandle{ ctx, surf, false };
}

bool MakeCurrent(Handle h) {
    auto* eh = static_cast<EGLHandle*>(h);
    if (!eh || g_display == EGL_NO_DISPLAY) {
        return false;
    }
    return eglMakeCurrent(g_display, eh->surface, eh->surface, eh->context) == EGL_TRUE;
}

void ClearCurrent() {
    if (g_display != EGL_NO_DISPLAY) {
        eglMakeCurrent(g_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    }
}

bool SwapBuffers(Handle h) {
    auto* eh = static_cast<EGLHandle*>(h);
    if (!eh || !eh->isWindow || g_display == EGL_NO_DISPLAY) {
        return false;
    }
    return eglSwapBuffers(g_display, eh->surface) == EGL_TRUE;
}

void NotifyResize(Handle /*h*/, int /*width*/, int /*height*/) {
    // ANGLE window surfaces auto-resize to the HWND client rect on the next
    // eglSwapBuffers; nothing to do today.  Kept as a seam for backends that
    // need an explicit swapchain resize.
}

void DestroyContext(Handle h) {
    std::lock_guard<std::mutex> lock(g_mutex);
    auto* eh = static_cast<EGLHandle*>(h);
    if (!eh) {
        return;
    }
    if (g_display != EGL_NO_DISPLAY) {
        if (eh->surface != EGL_NO_SURFACE) {
            eglDestroySurface(g_display, eh->surface);
        }
        if (eh->context != EGL_NO_CONTEXT) {
            eglDestroyContext(g_display, eh->context);
        }
    }
    delete eh;
}

void* GetProcAddress(const char* name) {
    if (g_display == EGL_NO_DISPLAY) {
        return nullptr;
    }
    return reinterpret_cast<void*>(eglGetProcAddress(name));
}

void Shutdown() {
    std::lock_guard<std::mutex> lock(g_mutex);
    if (g_display != EGL_NO_DISPLAY) {
        eglMakeCurrent(g_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (g_rootContext != EGL_NO_CONTEXT) {
            eglDestroyContext(g_display, g_rootContext);
            g_rootContext = EGL_NO_CONTEXT;
        }
        eglTerminate(g_display);
        g_display = EGL_NO_DISPLAY;
    }
    g_config = nullptr;
    g_initialized = false;
}

} // namespace xlAngleEGL

// ===========================================================================
// Native build (no USE_GLES, or Apple): stubs so callers need no compile
// guards.  Initialize() fails and every creator returns nullptr.
// ===========================================================================
#else // native (non-ANGLE) build

namespace xlAngleEGL {
bool   IsCompiledIn() { return false; }
bool   Initialize(uint64_t) { return false; }
bool   IsInitialized() { return false; }
Handle CreateWindowContext(void*, int, int) { return nullptr; }
Handle CreatePbufferContext() { return nullptr; }
bool   MakeCurrent(Handle) { return false; }
void   ClearCurrent() {}
bool   SwapBuffers(Handle) { return false; }
void   NotifyResize(Handle, int, int) {}
void   DestroyContext(Handle) {}
void*  GetProcAddress(const char*) { return nullptr; }
void   Shutdown() {}
} // namespace xlAngleEGL

#endif // USE_GLES && !__APPLE__

// ===========================================================================
// Render serialization — defined for all builds (a dedicated mutex, separate
// from the init mutex above).  Only the ANGLE on-screen canvas and off-screen
// pool actually take it; on native builds it is never contended.
// ===========================================================================
namespace xlAngleEGL {
// Recursive so a single thread that nests on-screen renders (canvas-in-canvas)
// doesn't self-deadlock; cross-thread it still blocks, which is the point.
static std::recursive_mutex g_renderMutex;
static std::atomic<size_t> g_renderOwner{ 0 };
static thread_local int g_renderDepth = 0;
static size_t currentThreadToken() {
    return std::hash<std::thread::id>{}(std::this_thread::get_id());
}
void RenderLock() {
    g_renderMutex.lock();
    if (g_renderDepth++ == 0) {
        g_renderOwner.store(currentThreadToken(), std::memory_order_relaxed);
    }
}
void RenderUnlock() {
    if (--g_renderDepth == 0) {
        g_renderOwner.store(0, std::memory_order_relaxed);
    }
    g_renderMutex.unlock();
}
bool RenderLockHeldByCurrentThread() {
    return g_renderOwner.load(std::memory_order_relaxed) == currentThreadToken();
}
} // namespace xlAngleEGL
