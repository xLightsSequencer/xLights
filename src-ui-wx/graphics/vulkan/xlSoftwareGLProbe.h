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

// Probe the platform OpenGL renderer to tell whether GL will run on a software
// rasterizer (Mesa llvmpipe/softpipe/swrast, OpenSWR, …).  Returns 1 = software,
// 0 = hardware, -1 = could not determine.  Used by the "Auto" graphics-backend
// heuristic to catch a silent Mesa llvmpipe fallback that sets no env flag.
//
// xdisplay is the Xlib Display* (passed as void*).  Linux/X11 only — returns -1
// on other platforms/backends.  Kept in its own translation unit so
// <GL/glx.h>'s X11/Xlib.h macro soup (None/Bool/Status/…) never reaches the wx
// code that consumes the result.
int xlProbeSoftwareGL(void* xdisplay);
