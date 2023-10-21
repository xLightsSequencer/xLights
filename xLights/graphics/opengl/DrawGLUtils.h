#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/


namespace DrawGLUtils
{
#define LOG_GL_ERROR() DrawGLUtils::LogGLError(__FILE__, __LINE__)
#define LOG_GL_ERRORV(a) a; DrawGLUtils::LogGLError(__FILE__, __LINE__, #a)
#define IGNORE_GL_ERRORV(a) a; glGetError()

    bool LoadGLFunctions();
    void DoLogGLError(const char* file, int line, const char* msg); // always logs
    void LogGLError(const char* file, int line, const char* msg = nullptr);
    void SetupDebugLogging();
}

