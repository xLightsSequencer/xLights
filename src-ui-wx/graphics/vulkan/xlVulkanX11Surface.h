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

#ifdef HAVE_VULKAN

#include <volk.h>

// Create a Vulkan surface for an X11 window.  Kept in its own translation unit
// (xlVulkanX11Surface.cpp) so X11/Xlib.h's macro soup (None/Bool/Status/Success
// /…) never reaches the wx-heavy xlVulkanCanvas TU.  dpy is the Xlib Display*
// passed as void* to keep Xlib types out of the caller; xwindow is the X11
// Window (XID).  Returns VK_SUCCESS and fills *out on success; a non-success
// result (or a null loader entry point) means the caller should fall back to
// OpenGL.  Defined only on Linux — every other platform creates its surface
// inline in xlVulkanCanvas.cpp.
VkResult xlVulkanCreateX11Surface(VkInstance instance, void* dpy, unsigned long xwindow, VkSurfaceKHR* out);

#endif
