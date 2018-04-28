#include "wx/wx.h"

#ifndef __WXMAC__
#include <GL/gl.h>
#ifdef _MSC_VER
#include "GL\glext.h"
#else
#include <GL/glext.h>
#endif

extern PFNGLCREATESHADERPROC     glCreateShader;
extern PFNGLSHADERSOURCEPROC     glShaderSource;
extern PFNGLCOMPILESHADERPROC    glCompileShader;
extern PFNGLGETSHADERIVPROC      glGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
extern PFNGLCREATEPROGRAMPROC    glCreateProgram;
extern PFNGLATTACHSHADERPROC     glAttachShader;
extern PFNGLLINKPROGRAMPROC      glLinkProgram;
extern PFNGLDETACHSHADERPROC     glDetachShader;
extern PFNGLDELETESHADERPROC     glDeleteShader;
extern PFNGLDELETEPROGRAMPROC    glDeleteProgram;

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
      && glDeleteProgram != nullptr;
}
#else
#include "OpenGL/gl.h"

static bool canUseShaders()
{
   return true;
}
#endif

#include "OpenGLShaders.h"

#include "DrawGLUtils.h"

bool OpenGLShaders::HasShaderSupport()
{
   return canUseShaders();
}

unsigned OpenGLShaders::compile( const std::string& vertexSource, const std::string& fragmentSource )
{
   return 0;
}
