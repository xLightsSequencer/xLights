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

// Isolated ANGLE/EGL backbone for the runtime-selectable OpenGL ES backend.
//
// WHY THIS IS A SEPARATE MODULE WITH AN OPAQUE API:
// ANGLE's libGLESv2 exports the same `gl*` symbols as the desktop GL the rest
// of xLights links (opengl32 / libGL).  Pulling <EGL/egl.h> + <GLES3/gl3.h>
// into a translation unit that also includes <GL/gl.h> produces a storm of
// type/enum redefinitions.  So every EGL/GLES include is confined to
// AngleEGL.cpp; this header exposes only opaque `void*` handles and plain
// types, and can be included safely from desktop-GL TUs
// (GLContextManager.cpp) and from wx UI TUs (the on-screen xlEGLCanvas).
//
// One shared EGLDisplay + one share-group serve the whole process:
//   - the on-screen canvas creates a WINDOW surface from the native handle;
//   - the off-screen ShaderEffect pool creates PBUFFER contexts;
//   - both share GL objects (programs/textures) via a common root context.
//
// When HAVE_ANGLE is not defined the whole module degrades to stubs:
// Initialize() returns false and every creator returns nullptr, so callers
// can probe availability without compile guards at the call site.
//
// Core layer — must not include wx or any UI header.

#include <cstdint>

namespace xlAngleEGL {

// Opaque handle bundling an EGLContext + its surface (window or pbuffer).
using Handle = void*;

// True only in builds compiled with HAVE_ANGLE (i.e. ANGLE libs available).
bool IsCompiledIn();

// Bring up the shared EGLDisplay (ANGLE D3D11 on Windows, Vulkan on Linux),
// choose a config, and create the root share context.  Idempotent; returns
// true once the display is usable.  `gpuDeviceId` optionally pins ANGLE to a
// specific adapter (0 = default).  Thread-safe.
bool Initialize(uint64_t gpuDeviceId = 0);
bool IsInitialized();

// On-screen: create a context + window surface bound to a native window
// (HWND on Windows; X11 Window / wl_surface handle on Linux), sharing the
// root share-group.  Returns nullptr on failure.
Handle CreateWindowContext(void* nativeWindow, int width, int height);

// Off-screen: create a context backed by a 1x1 pbuffer (the pool renders to
// FBOs, the pbuffer just satisfies eglMakeCurrent).  Shares the root group.
Handle CreatePbufferContext();

// Make the handle's context+surface current on the calling thread.
bool MakeCurrent(Handle h);
// Release whatever context is current on this thread.
void ClearCurrent();
// Present a window surface (no-op for pbuffer handles).
bool SwapBuffers(Handle h);
// Resize bookkeeping for a window surface (ANGLE window surfaces track the
// HWND client size; call on canvas resize so the next frame uses new extents).
void NotifyResize(Handle h, int width, int height);

// Destroy a context+surface created above.
void DestroyContext(Handle h);

// eglGetProcAddress passthrough — used to populate the global gl* pointer
// table from ANGLE when the active backend is ANGLE.  Returns nullptr if not
// initialized or the entry point is unknown.
void* GetProcAddress(const char* name);

// Process-wide render serialization.  ANGLE's D3D11 backend is not safe when
// the on-screen canvas (main thread) and the off-screen ShaderEffect pool
// (worker thread) submit to the shared device concurrently, so BOTH bracket
// their rendering with this lock — only one ANGLE context renders at a time.
// RenderGuard is the RAII helper; RenderLock/Unlock are for callers that span
// multiple functions (e.g. xlGLCanvas Prepare…/FinishDrawing).
void RenderLock();
void RenderUnlock();
// TEMP ANGLE crash diagnosis: true iff the calling thread currently holds the
// render lock.  Lets the draw path assert it is serialized before issuing GL.
bool RenderLockHeldByCurrentThread();
struct RenderGuard {
    RenderGuard() { RenderLock(); }
    ~RenderGuard() { RenderUnlock(); }
    RenderGuard(const RenderGuard&) = delete;
    RenderGuard& operator=(const RenderGuard&) = delete;
};

// Tear down all surfaces/contexts and the display.
void Shutdown();

} // namespace xlAngleEGL
