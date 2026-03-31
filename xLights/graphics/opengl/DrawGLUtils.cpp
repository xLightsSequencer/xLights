/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/wx.h>

#ifdef __WXMAC__
    // OpenGL is marked deprecated in OSX so we'll turn off the deprecation warnings for this file
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wdeprecated-declarations"


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
#include "utils/ExternalHooks.h"
#include "UtilFunctions.h"

#include <map>

#include <log.h>

#define DO_LOG_GL_MSG(a, ...) static_logger_opengl->error(a, ##__VA_ARGS__); printf(a, ##__VA_ARGS__); printf("\n")

static bool isDebugEnabled = false;
static bool isTraceDebugEnabled = false;
static std::shared_ptr<spdlog::logger> m_logger{ nullptr };
void DrawGLUtils::SetupDebugLogging() {
    m_logger = spdlog::get("opengl");
    if (!m_logger) {
        m_logger = spdlog::default_logger();
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
    m_logger->error("{}/{} - {}", f2, line, msg);
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
                    m_logger->debug("{}/{} - {}:   {:X}", f2, line, msg, er);
                } else {
                    m_logger->debug("{}/{}:   {:X}", f2, line, er);
                }
            }
            if (er) {
                if (msg) {
                    m_logger->error("{}/{} - {}:   {:X}", f2, line, msg, er);
                } else {
                    m_logger->error("{}/{}:   {:X}", f2, line, er);
                }
            }
        }
    }
}

