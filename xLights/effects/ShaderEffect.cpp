#include "../../include/shader_64.xpm"
#include "../../include/shader_48.xpm"
#include "../../include/shader_32.xpm"
#include "../../include/shader_24.xpm"
#include "../../include/shader_16.xpm"

#include "ShaderEffect.h"
#include "ShaderPanel.h"
#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../xLightsMain.h"
#include "../xLightsApp.h"
#include "../TimingPanel.h"
#include "OpenGLShaders.h"
#include "UtilFunctions.h"

// Ack... forgot the old warp effect needed all of this!!
#ifndef __WXMAC__
#include <GL/gl.h>
#ifdef _MSC_VER
#include "GL\glext.h"
#else
#include <GL/glext.h>
#endif

extern PFNGLGENBUFFERSPROC glGenBuffers;
extern PFNGLBINDBUFFERPROC glBindBuffer;
extern PFNGLBUFFERDATAPROC glBufferData;
extern PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
extern PFNGLDELETEBUFFERSPROC glDeleteBuffers;
extern PFNGLACTIVETEXTUREPROC glActiveTexture;
extern PFNGLDELETEPROGRAMPROC glDeleteProgram;
extern PFNGLUSEPROGRAMPROC glUseProgram;
extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
extern PFNGLUNIFORMMATRIX2FVPROC glUniformMatrix4fv;
extern PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
extern PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
extern PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
extern PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
extern PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
extern PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage;
extern PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;
extern PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers;
extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
extern PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
extern PFNGLUNIFORM1IPROC glUniform1i;
extern PFNGLUNIFORM1FPROC glUniform1f;
extern PFNGLUNIFORM4FPROC glUniform4f;
#else
#include "OpenGL/gl.h"
#endif

namespace
{
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

   GLuint RenderBufferTexture( int w, int h )
   {
      GLuint texId = 0;

      glGenTextures( 1, &texId );
      glBindTexture( GL_TEXTURE_2D, texId );

      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr );

      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

      glBindTexture( GL_TEXTURE_2D, 0 );

      return texId;
   }

   bool createOpenGLRenderBuffer( int width, int height, GLuint *rbID, GLuint *fbID )
   {
      glGenRenderbuffers(1, rbID);
      glBindRenderbuffer(GL_RENDERBUFFER, *rbID);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, width, height );

      glGenFramebuffers(1, fbID );
      glBindFramebuffer(GL_FRAMEBUFFER, *fbID);
      glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, *rbID);
      glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, *rbID);

      glBindFramebuffer( GL_FRAMEBUFFER, 0 );

      return *rbID != 0 && *fbID != 0;
   }

   const char *vsSrc =
      "#version 330 core\n"
      "in vec2 vpos;\n"
      "in vec2 tpos;\n"
      "out vec2 texCoord;\n"
      "void main(){\n"
      "    gl_Position = vec4(vpos,0,1);\n"
      "    texCoord = tpos;\n"
      "}\n";

   const char *psSrc =
      "#version 330\n"
      "out vec4 color;\n"
      "void main() {\n"
      "    color = vec4( 1, 0, 0, 1);\n"
      "}\n";
}

bool     ShaderEffect::s_shadersInit;
unsigned ShaderEffect::s_vertexArrayId = 0;
unsigned ShaderEffect::s_vertexBufferId = 0;
unsigned ShaderEffect::s_fbId = 0;
unsigned ShaderEffect::s_rbId = 0;
unsigned ShaderEffect::s_programId = 0;
unsigned ShaderEffect::s_rbTex = 0;
int      ShaderEffect::s_rbWidth = 0;
int      ShaderEffect::s_rbHeight = 0;

ShaderEffect::ShaderEffect(int i) : RenderableEffect(i, "Shader", shader_16_xpm, shader_24_xpm, shader_32_xpm, shader_48_xpm, shader_64_xpm)
{

}

ShaderEffect::~ShaderEffect()
{

}

wxPanel *ShaderEffect::CreatePanel(wxWindow *parent)
{
    return new ShaderPanel(parent);
}

bool ShaderEffect::needToAdjustSettings(const std::string &version)
{
    return false;
}

void ShaderEffect::adjustSettings(const std::string &version, Effect *effect, bool removeDefaults)
{
    //SettingsMap &settings = effect->GetSettings();

    // also give the base class a chance to adjust any settings
    if (RenderableEffect::needToAdjustSettings(version))
    {
        RenderableEffect::adjustSettings(version, effect, removeDefaults);
    }
}

std::list<std::string> ShaderEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff)
{
    std::list<std::string> res;

    return res;
}

void ShaderEffect::SetDefaultParameters()
{

}

void ShaderEffect::RemoveDefaults(const std::string &version, Effect *effect)
{
    RenderableEffect::RemoveDefaults(version, effect);
}

void ShaderEffect::Render(Effect *eff, SettingsMap &SettingsMap, RenderBuffer &buffer)
{
   ShaderPanel *p = (ShaderPanel *)panel;
   p->_preview->SetCurrentGLContext();

   if ( OpenGLShaders::HasFramebufferObjects() && OpenGLShaders::HasShaderSupport() )
   {
      sizeForRenderBuffer( buffer );

      glBindFramebuffer( GL_FRAMEBUFFER, s_fbId );
      glViewport( 0, 0, buffer.BufferWi, buffer.BufferHt );

      glClearColor( 0.f, 0.f, 0.f, 0.f );
      glClear( GL_COLOR_BUFFER_BIT );

      glBindVertexArray( s_vertexArrayId );
      glBindBuffer( GL_ARRAY_BUFFER, s_vertexBufferId );

      GLuint programId = s_programId;
      glUseProgram( programId );

      GLuint vattrib = glGetAttribLocation( programId, "vpos" );
      glVertexAttribPointer( vattrib, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTex), reinterpret_cast<void *>( offsetof(VertexTex, v) ) );
      glEnableVertexAttribArray( vattrib );

      GLuint tattrib = glGetAttribLocation( programId, "tpos" );
      glVertexAttribPointer( tattrib, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTex), reinterpret_cast<void *>( offsetof(VertexTex, t ) ) );
      glEnableVertexAttribArray( tattrib );

      glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

      glDisableVertexAttribArray( vattrib );
      glDisableVertexAttribArray( tattrib );

      glBindVertexArray( 0 );
      glBindBuffer( GL_ARRAY_BUFFER, 0 );

      xlColorVector& cv( buffer.pixels );
      glReadPixels( 0, 0, buffer.BufferWi, buffer.BufferHt, GL_RGBA, GL_UNSIGNED_BYTE, &cv[0] );
   }
}

void ShaderEffect::sizeForRenderBuffer(const RenderBuffer& rb)
{
    if (!s_shadersInit)
    {
        VertexTex vt[4] =
        {
           { {  1.f, -1.f }, { 1.f, 0.f } },
           { { -1.f, -1.f }, { 0.f, 0.f } },
           { {  1.f,  1.f }, { 1.f, 1.f } },
           { { -1.f,  1.f }, { 0.f, 1.f } }
        };
        glGenVertexArrays(1, &s_vertexArrayId);
        glGenBuffers(1, &s_vertexBufferId);

        glBindVertexArray(s_vertexArrayId);
        glBindBuffer(GL_ARRAY_BUFFER, s_vertexBufferId);
        glBufferData(GL_ARRAY_BUFFER, sizeof(VertexTex[4]), vt, GL_STATIC_DRAW);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        createOpenGLRenderBuffer(rb.BufferWi, rb.BufferHt, &s_rbId, &s_fbId);

        s_rbTex = RenderBufferTexture(rb.BufferWi, rb.BufferHt);

        s_programId = OpenGLShaders::compile( vsSrc, psSrc );

        s_rbWidth = rb.BufferWi;
        s_rbHeight = rb.BufferHt;
        s_shadersInit = true;
    }
    else if (rb.BufferWi > s_rbWidth || rb.BufferHt > s_rbHeight)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        if (s_fbId)
            glDeleteFramebuffers(1, &s_fbId);
        if (s_rbId)
        {
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
            glDeleteRenderbuffers(1, &s_rbId);
        }
        if (s_rbTex)
            glDeleteTextures(1, &s_rbTex);
        createOpenGLRenderBuffer(rb.BufferWi, rb.BufferHt, &s_rbId, &s_fbId);
        s_rbTex = RenderBufferTexture(rb.BufferWi, rb.BufferHt);

        s_rbWidth = rb.BufferWi;
        s_rbHeight = rb.BufferHt;
    }
}
