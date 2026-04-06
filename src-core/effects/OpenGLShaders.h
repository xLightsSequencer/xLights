#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <string>

class OpenGLShaders
{
public:
    static bool HasShaderSupport();
    static bool HasFramebufferObjects();

    static unsigned compile( const std::string& vertexSource, const std::string& fragmentSource, const std::string &filename );
    static std::string PrepareShaderCodeForLogging(const std::string& code);

    static void LogGLError(const char* file, int line, const char* msg = nullptr);
    static void DoLogGLError(const char* file, int line, const char* msg); // always logs
    static void SetupDebugLogging();
};

#define LOG_GL_ERROR() OpenGLShaders::LogGLError(__FILE__, __LINE__)
#define LOG_GL_ERRORV(a) a; OpenGLShaders::LogGLError(__FILE__, __LINE__, #a)
#define IGNORE_GL_ERRORV(a) a; glGetError()

