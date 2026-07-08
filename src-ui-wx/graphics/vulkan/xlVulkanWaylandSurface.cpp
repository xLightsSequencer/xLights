/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Isolated native-Wayland Vulkan-surface glue.  This is the only TU that pulls
// in the GDK/GTK Wayland headers, keeping them away from the wx code in
// xlVulkanCanvas.cpp.  vkCreateWaylandSurfaceKHR itself is loaded through volk,
// so the only extra link dependency this file adds is libgdk-3 (which owns the
// gdk_wayland_* accessors) — no direct libwayland-client link is needed.

#ifdef HAVE_VULKAN

#if defined(__linux__) && !defined(__ANDROID__)

#ifndef VK_USE_PLATFORM_WAYLAND_KHR
#define VK_USE_PLATFORM_WAYLAND_KHR
#endif
#include <volk.h>

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkwayland.h>

#include "xlVulkanWaylandSurface.h"

VkResult xlVulkanCreateWaylandSurface(VkInstance instance, void* gtkWidget, VkSurfaceKHR* out) {
    if (vkCreateWaylandSurfaceKHR == nullptr || gtkWidget == nullptr) {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
    GtkWidget* widget = static_cast<GtkWidget*>(gtkWidget);
    // A Vulkan swapchain needs a real wl_surface, which only exists once the
    // widget owns a native GdkWindow (GL/Vulkan canvases request one).
    gtk_widget_realize(widget);
    GdkWindow* window = gtk_widget_get_window(widget);
    if (window == nullptr || !GDK_IS_WAYLAND_WINDOW(window)) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    struct wl_display* display = gdk_wayland_display_get_wl_display(gdk_window_get_display(window));
    struct wl_surface* wlSurface = gdk_wayland_window_get_wl_surface(window);
    if (display == nullptr || wlSurface == nullptr) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    VkWaylandSurfaceCreateInfoKHR sci = {};
    sci.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    sci.display = display;
    sci.surface = wlSurface;
    return vkCreateWaylandSurfaceKHR(instance, &sci, nullptr, out);
}

#endif // __linux__

#endif // HAVE_VULKAN
