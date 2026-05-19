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

// Platform-neutral OpenGL context manager for shader rendering.
// Creates and pools offscreen GL contexts so ShaderEffect can render
// without depending on wx/UI types.
//
// macOS (CGL):    Independent contexts in pool, share via shared context.
// macOS (ANGLE):  Single EGL context (ANGLE Metal serializes anyway).
// Windows (WGL):  Hidden HWND + WGL contexts, all GL ops serialized on
//                 an internal worker thread (see ExecuteOnGLThread).
// Linux:          Independent GLX or EGL pbuffer contexts.

#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mutex>

class GLContextManager {
public:
    // Opaque handle to a platform GL context.
    using ContextHandle = void*;

    static GLContextManager& Instance();

    // Platform-specific callbacks provided by the UI layer at startup.
    struct InitParams {
        // Windows: synchronously run a callable on the main (UI) thread.
        // The callable MUST have completed by the time this function returns.
        std::function<void(std::function<void()>)> mainThreadRunner;

        // Windows: returns the shared HGLRC (cast to void*) from the main GL canvas.
        // Called lazily on first AcquireContext(), so the canvas need not exist at init time.
        std::function<void*()> getSharedGLContext;

        // Apple/ANGLE: Metal device registry ID to force ANGLE onto the same
        // GPU as the Metal compute effects.  Set to MTLDevice.registryID.
        // When non-zero, passed to eglGetPlatformDisplay via
        // EGL_PLATFORM_ANGLE_DEVICE_ID_HIGH/LOW_ANGLE.
        uint64_t metalDeviceRegistryID = 0;
    };

    // Initialize the manager.  Must be called once before AcquireContext().
    // On macOS the default InitParams{} is sufficient (CGL is self-contained).
    void Initialize(const InitParams& params);

    // Windows: controls whether shader rendering is allowed on background threads.
    // Defaults to false because many drivers crash with background GL contexts.
    // Has no effect on macOS (always true) or Linux (always false).
    void SetBackgroundRenderEnabled(bool b) { _backgroundRenderEnabled = b; }
    bool IsBackgroundRenderEnabled() const { return _backgroundRenderEnabled; }

    // Acquire a GL context for the calling thread.
    // Blocks if the pool is exhausted (macOS/Windows).
    // Returns nullptr on failure.
    ContextHandle AcquireContext();

    // Make the given context current on the calling thread.
    // Returns false if the context handle is invalid or the driver rejects it.
    bool MakeCurrent(ContextHandle ctx);

    // Release the context from the calling thread (unset current).
    // Does NOT return it to the pool — call ReleaseContext() for that.
    void DoneCurrent(ContextHandle ctx);

    // Return the context to the pool for reuse by other threads.
    void ReleaseContext(ContextHandle ctx);

    // Run a GL-touching callable on the thread that owns GL operations
    // for this platform. Blocks until the callable returns.
    //
    // - macOS / iOS / Linux: invokes fn() directly on the calling thread.
    //   Each render thread can hold its own context from the pool.
    // - Windows: posts fn() to an internal GL worker thread, blocks, returns.
    //   This serializes all GL work onto a single non-UI thread, dodging
    //   the driver-stability issues that affect some Windows GL drivers
    //   when GL is called from arbitrary render-pool threads. When
    //   SetBackgroundRenderEnabled(true) is set, the dispatch is bypassed
    //   and fn() runs directly — for users whose drivers are known good.
    //
    // The callable's lifetime must outlive the call (typical use is a
    // lambda capturing locals; ExecuteOnGLThread blocks so this is safe).
    void ExecuteOnGLThread(std::function<void()> fn);

    // Destroy all pooled contexts and release resources.
    void Shutdown();

    // Returns the native display handle (EGLDisplay for ANGLE, nullptr otherwise).
    // Used by MetalShaderEffect for texture sharing.
    void* GetNativeDisplay() const;

    // Platform-specific state lives in the .cpp behind #ifdef guards.
    // Forward-declared here as opaque pointers where needed.
    // Public so platform .cpp files can define and use the type.
    struct PlatformState;

private:
    GLContextManager() = default;
    ~GLContextManager();
    GLContextManager(const GLContextManager&) = delete;
    GLContextManager& operator=(const GLContextManager&) = delete;

    bool _initialized = false;
    bool _backgroundRenderEnabled = false;
    InitParams _params;
    PlatformState* _platform = nullptr;
};
