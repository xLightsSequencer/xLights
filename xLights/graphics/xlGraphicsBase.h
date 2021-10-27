#pragma once

#if __has_include("osxUtils/wxMetalCanvas.hpp")
    // Metal base classes are available, use the MetalCanvas
    #include "graphics/metal/xlMetalCanvas.h"
    typedef xlMetalCanvas GRAPHICS_BASE_CLASS;
#else
    #include "graphics/opengl/xlGLCanvas.h"
    typedef xlGLCanvas GRAPHICS_BASE_CLASS;
#endif
