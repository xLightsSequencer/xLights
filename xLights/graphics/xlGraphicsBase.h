#pragma once

//#define FORCE_OPENGL_BASE

#if !defined(FORCE_OPENGL_BASE) && __has_include("osxUtils/wxMetalCanvas.hpp")
    // Metal base classes are available, use the MetalCanvas
    #include "metal/xlMetalCanvas.h"
    typedef xlMetalCanvas GRAPHICS_BASE_CLASS;
#else
    #include "opengl/xlGLCanvas.h"
    typedef xlGLCanvas GRAPHICS_BASE_CLASS;
#endif
