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

// Create a native Wayland Vulkan surface (VK_KHR_wayland_surface).  Kept in its
// own translation unit (xlVulkanWaylandSurface.cpp) so the GDK/GTK Wayland
// headers stay out of the wx-heavy xlVulkanCanvas TU.  gtkWidget is the canvas's
// native GtkWidget* (wxWindow::GetHandle()); the helper realizes it and pulls
// the wl_display / wl_surface out of GDK.  Returns VK_SUCCESS and fills *out on
// success; anything else means the caller should fall back to OpenGL.  This is
// the path that lets xLights present without XWayland — defined only on Linux.
VkResult xlVulkanCreateWaylandSurface(VkInstance instance, void* gtkWidget, VkSurfaceKHR* out);

#endif
