/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Single translation unit hosting the volk and VulkanMemoryAllocator
// implementations.  Everything else includes <volk.h>/<vk_mem_alloc.h>
// declarations only (VK_NO_PROTOTYPES is defined project-wide; volk
// dlopens the Vulkan loader at runtime, so there is no link-time
// dependency on libvulkan).
#ifdef HAVE_VULKAN

// Platform surface (WSI) entry points for the on-screen graphics backend —
// volk only defines/loads them when the platform macro is set in the
// implementation TU.  On Linux we load both the Xlib and Wayland entry points:
// xLights still forces the X11 GDK backend today (for GLX), but Vulkan can
// present natively on either, which is the path toward dropping that force and
// running pure Wayland.
#if defined(_WIN32) && !defined(VK_USE_PLATFORM_WIN32_KHR)
#define VK_USE_PLATFORM_WIN32_KHR
#elif defined(__linux__) && !defined(__ANDROID__)
#ifndef VK_USE_PLATFORM_XLIB_KHR
#define VK_USE_PLATFORM_XLIB_KHR
#endif
#ifndef VK_USE_PLATFORM_WAYLAND_KHR
#define VK_USE_PLATFORM_WAYLAND_KHR
#endif
#endif

#define VOLK_IMPLEMENTATION
#include <volk.h>

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include <vk_mem_alloc.h>

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

#endif
