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
