/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/wx.h>

#ifdef __WXMAC__
    // we NEED OpenGL 1.1 for this file
    #undef __gl_h_
    #define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
    #include "OpenGL/gl.h"
#else
 #include <GL/gl.h>
 #ifdef _MSC_VER
 #include "GL\glext.h"
 #else
 #include <GL/glext.h>
 #endif
#endif


#include <wx/bitmap.h>
#include "DrawGLUtils.h"
#include <wx/graphics.h>
#include <wx/dcgraph.h>
#include "xlGLCanvas.h"
#include "../../ExternalHooks.h"
#include "../../UtilFunctions.h"

#include <map>

#include <log4cpp/Category.hh>

#define DO_LOG_GL_MSG(a, ...) static_logger_opengl->error(a, ##__VA_ARGS__); printf(a, ##__VA_ARGS__); printf("\n")

static bool isDebugEnabled = false;
static bool isTraceDebugEnabled = false;
static log4cpp::Category *static_logger_opengl = nullptr;
static log4cpp::Category *static_logger_opengl_trace = nullptr;
void DrawGLUtils::SetupDebugLogging() {
    if (!static_logger_opengl) {
        static_logger_opengl = &log4cpp::Category::getInstance(std::string("log_opengl"));
        static_logger_opengl_trace = &log4cpp::Category::getInstance(std::string("log_opengl_trace"));
        isTraceDebugEnabled = static_logger_opengl_trace->isDebugEnabled();
        isDebugEnabled = static_logger_opengl->isDebugEnabled() | isTraceDebugEnabled;
    }
}

void DrawGLUtils::DoLogGLError(const char* file, int line, const char* msg)
{
    const char* f2 = file + strlen(file);
    while (f2 > file && *f2 != '\\' && *f2 != '/') {
        f2--;
    }
    if (*f2 == '\\' || *f2 == '/') {
        f2++;
    }
    static_logger_opengl_trace->debug("%s/%d - %s", f2, line, msg);
}

void DrawGLUtils::LogGLError(const char * file, int line, const char *msg) {
    if (isDebugEnabled) {
        int er = glGetError();
        if (er || isTraceDebugEnabled) {
            const char *f2 = file + strlen(file);
            while (f2 > file && *f2 != '\\' && *f2 != '/') {
                f2--;
            }
            if (*f2 == '\\' || *f2 == '/') {
                f2++;
            }
            if (isTraceDebugEnabled) {
                if (msg) {
                    static_logger_opengl_trace->debug("%s/%d - %s:   %X", f2, line, msg, er);
                } else {
                    static_logger_opengl_trace->debug("%s/%d:   %X", f2, line, er);
                }
            }
            if (er) {
                if (msg) {
                    DO_LOG_GL_MSG("%s/%d - %s:   %X", f2, line, msg, er);
                } else {
                    DO_LOG_GL_MSG("%s/%d:   %X", f2, line, er);
                }
            }
        }
    }
}

