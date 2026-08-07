/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Isolated Xlib Vulkan-surface glue.  This is the only TU that pulls in
// X11/Xlib.h (via vulkan_xlib.h), keeping its macro pollution away from the
// wx code in xlVulkanCanvas.cpp.

#ifdef HAVE_VULKAN

#if defined(__linux__) && !defined(__ANDROID__)

#ifndef VK_USE_PLATFORM_XLIB_KHR
#define VK_USE_PLATFORM_XLIB_KHR
#endif
#include <volk.h>

#include "xlVulkanX11Surface.h"

VkResult xlVulkanCreateX11Surface(VkInstance instance, void* dpy, unsigned long xwindow, VkSurfaceKHR* out) {
    if (vkCreateXlibSurfaceKHR == nullptr || dpy == nullptr || xwindow == 0) {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
    VkXlibSurfaceCreateInfoKHR sci = {};
    sci.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    sci.dpy = (Display*)dpy;
    sci.window = (Window)xwindow;
    return vkCreateXlibSurfaceKHR(instance, &sci, nullptr, out);
}

#endif // __linux__

#endif // HAVE_VULKAN
