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
// xLights links the platform native desktop-GL provider, so this module just
// names the compiled-in backend for logging/diagnostics.
//
// Core layer — must not include wx or any UI header.

namespace xlGLBackend {

enum class Type {
    Native,  // platform native desktop GL (WGL / GLX / CGL)
};

// The backend this binary was built with.
Type CompiledBackend();

// Human-readable name for logging.
const char* ToString(Type t);

} // namespace xlGLBackend
