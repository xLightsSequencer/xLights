// OpenGL extension function pointer definitions for the Qt build.
// This file is compiled WITHOUT Qt headers in the include path (separate
// OBJECT target in CMake) so that <GL/gl.h>/<GL/glext.h> don't conflict
// with Qt's own GL abstractions.
//
// All pointers default to nullptr.  OpenGLShaders::HasShaderSupport()
// checks for nullptr before any GL call, so ShaderEffect exits harmlessly.

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <GL/gl.h>
#include <GL/glext.h>

PFNGLUSEPROGRAMPROC                      glUseProgram                     = nullptr;
PFNGLCREATESHADERPROC                    glCreateShader                   = nullptr;
PFNGLSHADERSOURCEPROC                    glShaderSource                   = nullptr;
PFNGLCOMPILESHADERPROC                   glCompileShader                  = nullptr;
PFNGLGETSHADERIVPROC                     glGetShaderiv                    = nullptr;
PFNGLGETSHADERINFOLOGPROC                glGetShaderInfoLog               = nullptr;
PFNGLCREATEPROGRAMPROC                   glCreateProgram                  = nullptr;
PFNGLATTACHSHADERPROC                    glAttachShader                   = nullptr;
PFNGLGETPROGRAMIVPROC                    glGetProgramiv                   = nullptr;
PFNGLGETPROGRAMINFOLOGPROC               glGetProgramInfoLog              = nullptr;
PFNGLGETACTIVEUNIFORMPROC                glGetActiveUniform               = nullptr;
PFNGLDETACHSHADERPROC                    glDetachShader                   = nullptr;
PFNGLDELETESHADERPROC                    glDeleteShader                   = nullptr;
PFNGLLINKPROGRAMPROC                     glLinkProgram                    = nullptr;
PFNGLDELETEPROGRAMPROC                   glDeleteProgram                  = nullptr;
PFNGLISPROGRAMPROC                       glIsProgram                      = nullptr;
PFNGLGETUNIFORMLOCATIONPROC              glGetUniformLocation             = nullptr;
PFNGLUNIFORM1IPROC                       glUniform1i                      = nullptr;
PFNGLUNIFORM4FPROC                       glUniform4f                      = nullptr;
PFNGLUNIFORM1FPROC                       glUniform1f                      = nullptr;
PFNGLUNIFORM2FPROC                       glUniform2f                      = nullptr;
PFNGLUNIFORMMATRIX4FVPROC                glUniformMatrix4fv               = nullptr;
PFNGLBINDBUFFERPROC                      glBindBuffer                     = nullptr;
PFNGLDELETEBUFFERSPROC                   glDeleteBuffers                  = nullptr;
PFNGLGENBUFFERSPROC                      glGenBuffers                     = nullptr;
PFNGLBUFFERDATAPROC                      glBufferData                     = nullptr;
PFNGLBUFFERSUBDATAPROC                   glBufferSubData                  = nullptr;
PFNGLFLUSHMAPPEDBUFFERRANGEPROC          glFlushMappedBufferRange         = nullptr;
PFNGLMAPBUFFERRANGEPROC                  glMapBufferRange                 = nullptr;
PFNGLUNMAPBUFFERPROC                     glUnmapBuffer                    = nullptr;
PFNGLACTIVETEXTUREPROC                   glActiveTexture                  = nullptr;
PFNGLGENVERTEXARRAYSPROC                 glGenVertexArrays                = nullptr;
PFNGLBINDVERTEXARRAYPROC                 glBindVertexArray                = nullptr;
PFNGLDELETEVERTEXARRAYSPROC              glDeleteVertexArrays             = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC         glEnableVertexAttribArray        = nullptr;
PFNGLVERTEXATTRIBPOINTERPROC             glVertexAttribPointer            = nullptr;
PFNGLDISABLEVERTEXATTRIBARRAYPROC        glDisableVertexAttribArray       = nullptr;
PFNGLGETATTRIBLOCATIONPROC               glGetAttribLocation              = nullptr;
PFNGLPOINTPARAMETERFPROC                 glPointParameterf                = nullptr;
PFNGLGENFRAMEBUFFERSPROC                 glGenFramebuffers                = nullptr;
PFNGLBINDFRAMEBUFFERPROC                 glBindFramebuffer                = nullptr;
PFNGLDELETEFRAMEBUFFERSPROC              glDeleteFramebuffers             = nullptr;
PFNGLISFRAMEBUFFERPROC                   glIsFramebuffer                  = nullptr;
PFNGLFRAMEBUFFERPARAMETERIPROC           glFramebufferParameteri          = nullptr;
PFNGLGENRENDERBUFFERSPROC                glGenRenderbuffers               = nullptr;
PFNGLDELETERENDERBUFFERSPROC             glDeleteRenderbuffers            = nullptr;
PFNGLISRENDERBUFFERPROC                  glIsRenderbuffer                 = nullptr;
PFNGLBINDRENDERBUFFERPROC                glBindRenderbuffer               = nullptr;
PFNGLRENDERBUFFERSTORAGEPROC             glRenderbufferStorage            = nullptr;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC  glRenderbufferStorageMultisample = nullptr;
PFNGLFRAMEBUFFERRENDERBUFFERPROC         glFramebufferRenderbuffer        = nullptr;
PFNGLCHECKFRAMEBUFFERSTATUSPROC          glCheckFramebufferStatus         = nullptr;
PFNGLFRAMEBUFFERTEXTURE2DPROC            glFramebufferTexture2D           = nullptr;
PFNGLBLITFRAMEBUFFERPROC                 glBlitFramebuffer                = nullptr;
