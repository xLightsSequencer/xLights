#pragma once

//#define FORCE_OPENGL_BASE
//#define FORCE_VULKAN_BASE

#if !defined(FORCE_OPENGL_BASE) && __has_include("osxUtilsUI/wxMetalCanvas.hpp")
    // Metal base classes are available, use the MetalCanvas
    #include "metal/xlMetalCanvas.h"
    #define XL_DRAWING_WITH_METAL
    typedef xlMetalCanvas GRAPHICS_BASE_CLASS;
#elif defined(FORCE_VULKAN_BASE) && __has_include("vulkan/xlVulkanCanvas.h")
    #include "vulkan/xlVulkanCanvas.h"
    #define XL_DRAWING_WITH_VULKAN
    typedef xlVulkanCanvas GRAPHICS_BASE_CLASS;
#else
    #include "opengl/xlGLCanvas.h"
    #define XL_DRAWING_WITH_OPENGL
    typedef xlGLCanvas GRAPHICS_BASE_CLASS;
#endif
