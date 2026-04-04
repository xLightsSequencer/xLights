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
// on background threads without depending on wx/UI types.
//
// macOS:   Pure CGL — contexts created and pooled natively.
// Windows: Hidden HWND + WGL — context creation dispatched to main thread.
// Linux:   Callback-based — main thread context activation via UICallbacks.

#include <condition_variable>
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

        // Linux: activate/deactivate the panel's GL context on the main thread.
        std::function<void()> activateMainContext;
        std::function<void()> deactivateMainContext;
    };

    // Initialize the manager.  Must be called once before AcquireContext().
    // On macOS the default InitParams{} is sufficient (CGL is self-contained).
    void Initialize(const InitParams& params = {});

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
    void MakeCurrent(ContextHandle ctx);

    // Release the context from the calling thread (unset current).
    // Does NOT return it to the pool — call ReleaseContext() for that.
    void DoneCurrent(ContextHandle ctx);

    // Return the context to the pool for reuse by other threads.
    void ReleaseContext(ContextHandle ctx);

    // Whether this platform supports shader rendering on background threads.
    bool CanRenderOnBackgroundThread() const;

    // Destroy all pooled contexts and release resources.
    void Shutdown();

private:
    GLContextManager() = default;
    ~GLContextManager();
    GLContextManager(const GLContextManager&) = delete;
    GLContextManager& operator=(const GLContextManager&) = delete;

    bool _initialized = false;
    bool _backgroundRenderEnabled = false;
    InitParams _params;

    // Platform-specific state lives in the .cpp behind #ifdef guards.
    // Forward-declared here as opaque pointers where needed.
    struct PlatformState;
    PlatformState* _platform = nullptr;
};
