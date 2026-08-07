/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Isolated OpenGL-renderer probe.  This is the only TU pulling in <GL/glx.h>
// (and thus X11/Xlib.h), keeping its macros away from the wx code.

#if defined(__linux__) && !defined(__ANDROID__)

#include <GL/glx.h>
#include <cstring>

#include "xlSoftwareGLProbe.h"

static int classifyRenderer(const char* rend) {
    if (rend == nullptr) {
        return -1;
    }
    // Mesa's software rasterizers + Intel OpenSWR — the renderer string is a
    // stable, driver-level signal (independent of the FBConfig chosen here).
    if (strstr(rend, "llvmpipe") != nullptr || strstr(rend, "softpipe") != nullptr ||
        strstr(rend, "swrast") != nullptr || strstr(rend, "OpenSWR") != nullptr) {
        return 1;
    }
    return 0;
}

int xlProbeSoftwareGL(void* xdisplay) {
    Display* dpy = static_cast<Display*>(xdisplay);
    if (dpy == nullptr) {
        return -1;
    }
    // Probe through a WINDOW drawable (like the real canvas), not a pbuffer:
    // some drivers (notably virgl) fall back to llvmpipe for offscreen pbuffers
    // while rendering windows on the GPU, which would give a false "software".
    static const int fbAttribs[] = {
        GLX_RENDER_TYPE, GLX_RGBA_BIT,
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_RED_SIZE, 1, GLX_GREEN_SIZE, 1, GLX_BLUE_SIZE, 1,
        None
    };
    int nfb = 0;
    GLXFBConfig* fbc = glXChooseFBConfig(dpy, DefaultScreen(dpy), fbAttribs, &nfb);
    if (fbc == nullptr || nfb == 0) {
        return -1;
    }
    XVisualInfo* vi = glXGetVisualFromFBConfig(dpy, fbc[0]);
    if (vi == nullptr) {
        XFree(fbc);
        return -1;
    }

    Window root = RootWindow(dpy, vi->screen);
    XSetWindowAttributes swa = {};
    swa.colormap = XCreateColormap(dpy, root, vi->visual, AllocNone);
    swa.border_pixel = 0;
    // A 1x1 window, never mapped (invisible) — a valid GLX window drawable.
    Window win = XCreateWindow(dpy, root, 0, 0, 1, 1, 0, vi->depth, InputOutput,
                               vi->visual, CWBorderPixel | CWColormap, &swa);

    // Leave the caller's GL state untouched: restore whatever context (if any)
    // was current after the one glGetString.
    GLXContext prevCtx = glXGetCurrentContext();
    GLXDrawable prevDraw = glXGetCurrentDrawable();
    GLXDrawable prevRead = glXGetCurrentReadDrawable();

    int result = -1;
    GLXContext ctx = glXCreateNewContext(dpy, fbc[0], GLX_RGBA_TYPE, nullptr, True);
    if (ctx != nullptr && win != 0) {
        if (glXMakeContextCurrent(dpy, win, win, ctx)) {
            result = classifyRenderer(reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
        }
        if (prevCtx != nullptr) {
            glXMakeContextCurrent(dpy, prevDraw, prevRead, prevCtx);
        } else {
            glXMakeContextCurrent(dpy, None, None, nullptr);
        }
    }
    if (ctx != nullptr) {
        glXDestroyContext(dpy, ctx);
    }
    if (win != 0) {
        XDestroyWindow(dpy, win);
    }
    if (swa.colormap != 0) {
        XFreeColormap(dpy, swa.colormap);
    }
    XFree(vi);
    XFree(fbc);
    return result;
}

#else

#include "xlSoftwareGLProbe.h"
int xlProbeSoftwareGL(void*) {
    return -1;
}

#endif
