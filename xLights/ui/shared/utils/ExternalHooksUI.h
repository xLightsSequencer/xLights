#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 **************************************************************/

// UI-specific platform hooks.  Depends on wxWidgets types.
// Core-layer code must use ExternalHooks.h instead.

#include "ExternalHooks.h"

#if defined(__APPLE__)
#if __has_include("ExternalHooksMacOSUI.h")
#include "osxUtils/ExternalHooksMacOSUI.h"
#endif
#elif defined(_WIN32)
#if __has_include("ExternalHooksMSWUI.h")
#include "mswUtils/ExternalHooksMSWUI.h"
#endif
#else
#if __has_include("ExternalHooksLinuxUI.h")
#include "linuxUtils/ExternalHooksLinuxUI.h"
#endif
#endif

// Fallback no-ops for platforms without platform-specific hooks.
#ifndef __XL_EXTERNAL_HOOKS__
inline double xlOSGetMainScreenContentScaleFactor() { return 1.0; }
inline bool IsMouseEventFromTouchpad() { return false; }
#define WXGLUnsetCurrentContext()
#endif
