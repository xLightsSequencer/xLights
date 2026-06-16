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

// Reports which OpenGL backend this binary was compiled against.
//
// The backend is a COMPILE-TIME choice (see plans/angle-support.md): xLights
// is built either as a native desktop-GL binary or as an ANGLE (OpenGL ES via
// libEGL/libGLESv2) binary, selected by the USE_GLES macro.  A single binary
// links exactly one gl* provider, so there is nothing to switch at runtime —
// this module just names the compiled-in backend for logging/diagnostics.
//
// Core layer — must not include wx or any UI header.

namespace xlGLBackend {

enum class Type {
    Native,  // platform native desktop GL (WGL / GLX / CGL)
    ANGLE,   // ANGLE: OpenGL ES 3.0 over D3D11 / Vulkan / Metal
};

// The backend this binary was built with (ANGLE iff USE_GLES is defined).
Type CompiledBackend();

// Human-readable name for logging.
const char* ToString(Type t);

} // namespace xlGLBackend
