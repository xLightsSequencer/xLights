/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#ifndef __APPLE__
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glext.h>

#include <memory>
#include <iostream>

extern PFNGLCREATESHADERPROC      glCreateShader;
extern PFNGLSHADERSOURCEPROC      glShaderSource;
extern PFNGLCOMPILESHADERPROC     glCompileShader;
extern PFNGLGETSHADERIVPROC       glGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC  glGetShaderInfoLog;
extern PFNGLCREATEPROGRAMPROC     glCreateProgram;
extern PFNGLATTACHSHADERPROC      glAttachShader;
extern PFNGLLINKPROGRAMPROC       glLinkProgram;
extern PFNGLDETACHSHADERPROC      glDetachShader;
extern PFNGLDELETESHADERPROC      glDeleteShader;
extern PFNGLDELETEPROGRAMPROC     glDeleteProgram;
extern PFNGLGETPROGRAMIVPROC      glGetProgramiv;
extern PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;

extern PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
extern PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
extern PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
extern PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
extern PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers;
extern PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
extern PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage;
extern PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;

static bool canUseShaders()
{
   return glCreateShader != nullptr
      && glShaderSource != nullptr
      && glCompileShader != nullptr
      && glGetShaderiv != nullptr
      && glGetShaderInfoLog != nullptr
      && glCreateProgram != nullptr
      && glAttachShader != nullptr
      && glLinkProgram != nullptr
      && glDetachShader != nullptr
      && glDeleteShader != nullptr
      && glDeleteProgram != nullptr
      && glGetProgramiv != nullptr
      && glGetProgramInfoLog != nullptr;
}
static bool canUseFramebufferObjects()
{
	return glGenFramebuffers != nullptr
		&& glBindFramebuffer != nullptr
		&& glDeleteFramebuffers != nullptr
		&& glGenRenderbuffers != nullptr
		&& glDeleteRenderbuffers != nullptr
		&& glBindRenderbuffer != nullptr
		&& glRenderbufferStorage != nullptr
		&& glFramebufferRenderbuffer != nullptr;
}
#else
    #ifdef USE_GLES
        // ANGLE provides OpenGL ES 3.0 on top of Metal
        #define GL_GLES_PROTOTYPES 1
        #include <GLES3/gl3.h>
    #else
        // OpenGL is marked deprecated in OSX so we'll turn off the deprecation warnings for this file
        #pragma clang diagnostic push
        #pragma clang diagnostic ignored "-Wdeprecated-declarations"

        #include "OpenGL/gl.h"
    #endif

static bool canUseShaders()
{
   return true;
}
static bool canUseFramebufferObjects()
{
   return true;
}
#endif

#include <cstdio>
#include <cstring>
#include <sstream>
#include <string>

#include "UtilFunctions.h"
#include "OpenGLShaders.h"
#include <log.h>

#include "utils/TraceLog.h"
using namespace TraceLog;

static bool isDebugEnabled = false;
static bool isTraceDebugEnabled = false;
static std::shared_ptr<spdlog::logger> s_glLogger{ nullptr };

void OpenGLShaders::SetupDebugLogging() {
    s_glLogger = spdlog::get("opengl");
    if (!s_glLogger) {
        s_glLogger = spdlog::default_logger();
    }
}

void OpenGLShaders::DoLogGLError(const char* file, int line, const char* msg)
{
    const char* f2 = file + strlen(file);
    while (f2 > file && *f2 != '\\' && *f2 != '/') {
        f2--;
    }
    if (*f2 == '\\' || *f2 == '/') {
        f2++;
    }
    if (!s_glLogger) {
        SetupDebugLogging();
    }
    s_glLogger->error("{}/{} - {}", f2, line, msg);
}

void OpenGLShaders::LogGLError(const char* file, int line, const char* msg) {
    if (isDebugEnabled) {
        int er = glGetError();
        if (er || isTraceDebugEnabled) {
            const char* f2 = file + strlen(file);
            while (f2 > file && *f2 != '\\' && *f2 != '/') {
                f2--;
            }
            if (*f2 == '\\' || *f2 == '/') {
                f2++;
            }
            if (!s_glLogger) {
                SetupDebugLogging();
            }
            if (isTraceDebugEnabled) {
                if (msg) {
                    s_glLogger->debug("{}/{} - {}:   {:X}", f2, line, msg, er);
                } else {
                    s_glLogger->debug("{}/{}:   {:X}", f2, line, er);
                }
            }
            if (er) {
                if (msg) {
                    s_glLogger->error("{}/{} - {}:   {:X}", f2, line, msg, er);
                } else {
                    s_glLogger->error("{}/{}:   {:X}", f2, line, er);
                }
            }
        }
    }
}


namespace
{
   bool shaderLinkSuceeded( GLuint programID, const std::string &filename = "")
   {
      GLint result = GL_FALSE;
      glGetProgramiv( programID, GL_LINK_STATUS, &result );
      if ( result == GL_FALSE )
      {
         int infoLogLength = 0;
         glGetProgramiv( programID, GL_INFO_LOG_LENGTH, &infoLogLength );
         if ( infoLogLength > 0 )
         {
            std::vector<char> errorMessage( infoLogLength + 1 );
            char*             messagePtr = &errorMessage[0];
            glGetProgramInfoLog( programID, infoLogLength, NULL, messagePtr );
            auto logger = spdlog::get("opengl");
            logger->error("shader-link failure {}: '{}'", filename, messagePtr);
         }
      }
      return result == GL_TRUE;
   }

   static bool shaderCompileSuceeded( GLuint shaderID, const std::string &filename = "" )
   {
      GLint result = GL_FALSE;
      glGetShaderiv( shaderID, GL_COMPILE_STATUS, &result );
      if ( result == GL_FALSE )
      {
         int infoLogLength = 0;
         glGetShaderiv( shaderID, GL_INFO_LOG_LENGTH, &infoLogLength );
         if ( infoLogLength > 0 )
         {
            std::vector<char> errorMessage( infoLogLength + 1 );
            char*             messagePtr = &errorMessage[0];
            glGetShaderInfoLog( shaderID, infoLogLength, NULL, messagePtr );

             messagePtr[infoLogLength] = 0;
             std::string m = "Shader fail message: ";
             m += messagePtr;
             AddTraceMessage(m);

            auto logger = spdlog::get("opengl");
            logger->error("shader-compile failure {}: '{}'", filename, messagePtr);
         }
      }
      return result == GL_TRUE;
   }
}

bool OpenGLShaders::HasShaderSupport()
{
   return canUseShaders();
}

bool OpenGLShaders::HasFramebufferObjects()
{
   return canUseFramebufferObjects();
}

std::string OpenGLShaders::PrepareShaderCodeForLogging(const std::string& code)
{
    std::string res = "\n";

    std::istringstream reader(code);

    int line = 1;
    char buffer[4096];
    do {
        reader.getline(buffer, sizeof(buffer));
        char linePrefix[16];
        snprintf(linePrefix, sizeof(linePrefix), "%04d: ", line++);
        res += linePrefix;
        res += buffer;
        res += '\n';
    } while (!reader.eof() && !reader.fail());

    Replace(res, "\r", "");

    return res;
}

unsigned OpenGLShaders::compile( const std::string& vertexSource, const std::string& fragmentSource, const std::string &filename )
{
    AddTraceMessage("In vshader compile");
    LOG_GL_ERRORV(GLuint vertexShader = glCreateShader( GL_VERTEX_SHADER ));
    AddTraceMessage("Setting vshader source");
    const GLchar* vertexShaders[] = { vertexSource.c_str() };
    LOG_GL_ERRORV(glShaderSource( vertexShader, 1, vertexShaders, NULL ));
    AddTraceMessage("VCompiling");
    LOG_GL_ERRORV(glCompileShader( vertexShader ));
    AddTraceMessage("VCompile Step complete");
    if (!shaderCompileSuceeded(vertexShader))
    {
        AddTraceMessage("VShader failed to compile");
        LOG_GL_ERRORV(glDeleteShader(vertexShader));
        return 0;
    }
    AddTraceMessage("Compile successful");

    LOG_GL_ERRORV(GLuint fragmentShader = glCreateShader( GL_FRAGMENT_SHADER ));
    const GLchar* fragmentShaders[] = { fragmentSource.c_str() };
    AddTraceMessage("Setting fshader source");
    LOG_GL_ERRORV(glShaderSource( fragmentShader, 1, fragmentShaders, nullptr ));
    AddTraceMessage("FCompiling");
    LOG_GL_ERRORV(glCompileShader( fragmentShader ));
    AddTraceMessage("FCompile Step complete");
    if (!shaderCompileSuceeded(fragmentShader, filename))
    {
        AddTraceMessage("FShader failed to compile");
        LOG_GL_ERRORV(glDeleteShader(vertexShader));
        LOG_GL_ERRORV(glDeleteShader(fragmentShader));

        auto logger = spdlog::get("opengl");
        logger->error(PrepareShaderCodeForLogging(fragmentSource));
        return 0;
    }
    AddTraceMessage("Compile successful");

    LOG_GL_ERRORV(GLuint program = glCreateProgram());
    LOG_GL_ERRORV(glAttachShader( program, vertexShader ));
    LOG_GL_ERRORV(glAttachShader( program, fragmentShader ));
    LOG_GL_ERRORV(glLinkProgram( program ));
    AddTraceMessage("Linking");
    bool linkSuccess = shaderLinkSuceeded( program );
    AddTraceMessage(linkSuccess ? "Linked" : "Linking failed");

    LOG_GL_ERRORV(glDetachShader( program, vertexShader ));
    LOG_GL_ERRORV(glDetachShader( program, fragmentShader ));

    LOG_GL_ERRORV(glDeleteShader( vertexShader ));
    LOG_GL_ERRORV(glDeleteShader( fragmentShader ));

    return linkSuccess ? program : 0;
}

#if defined(__APPLE__) && !defined(USE_GLES)
// OpenGL is marked deprecated in OSX so we'll turn off the deprecation warnings for this file
#pragma clang diagnostic pop
#endif
