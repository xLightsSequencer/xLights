#pragma once

//#define FORCE_OPENGL_BASE

#if !defined(FORCE_OPENGL_BASE) && __has_include("osxUtilsUI/wxMetalCanvas.hpp")
    // Metal base classes are available, use the MetalCanvas
    #include "metal/xlMetalCanvas.h"
    #define XL_DRAWING_WITH_METAL
    typedef xlMetalCanvas GRAPHICS_BASE_CLASS;
#elif !defined(FORCE_OPENGL_BASE) && defined(HAVE_VULKAN)
    // Runtime-switchable canvas: renders via Vulkan when the preference /
    // XL_GRAPHICS_BACKEND selects it and Vulkan is available, otherwise
    // behaves exactly like its xlGLCanvas base.
    #include "vulkan/xlVulkanCanvas.h"
    #define XL_DRAWING_WITH_VULKAN
    typedef xlVulkanCanvas GRAPHICS_BASE_CLASS;
#else
    #include "opengl/xlGLCanvas.h"
    #define XL_DRAWING_WITH_OPENGL
    typedef xlGLCanvas GRAPHICS_BASE_CLASS;
#endif
