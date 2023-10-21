/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/


#include "xlOGL3GraphicsContext.h"

#include <log4cpp/Category.hh>

#include "DrawGLUtils.h"
#include "../xlMesh.h"

#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifndef GL_POINT_SMOOTH
#define GL_POINT_SMOOTH                0x0B10
#endif


#ifndef __WXMAC__
#include <GL/gl.h>
#ifdef _MSC_VER
#include "GL\glext.h"
#else
#include <GL/glx.h>
#include <GL/glext.h>
#endif

PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLCREATESHADERPROC glCreateShader;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
PFNGLGETACTIVEUNIFORMPROC glGetActiveUniform;
PFNGLDETACHSHADERPROC glDetachShader;
PFNGLDELETESHADERPROC glDeleteShader;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLDELETEPROGRAMPROC glDeleteProgram;
PFNGLISPROGRAMPROC glIsProgram;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
PFNGLUNIFORM1IPROC glUniform1i;
PFNGLUNIFORM4FPROC glUniform4f;
PFNGLUNIFORM1FPROC glUniform1f;
PFNGLUNIFORM2FPROC glUniform2f;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLDELETEBUFFERSPROC glDeleteBuffers;
PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLBUFFERSUBDATAPROC glBufferSubData;
PFNGLFLUSHMAPPEDBUFFERRANGEPROC glFlushMappedBufferRange;
PFNGLMAPBUFFERRANGEPROC glMapBufferRange;
PFNGLUNMAPBUFFERPROC glUnmapBuffer;
#ifndef LINUX
PFNGLACTIVETEXTUREPROC glActiveTexture;
#endif
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
PFNGLPOINTPARAMETERFPROC glPointParameterf;

PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
PFNGLISFRAMEBUFFERPROC glIsFramebuffer;
PFNGLFRAMEBUFFERPARAMETERIPROC glFramebufferParameteri;
PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers;
PFNGLISRENDERBUFFERPROC glIsRenderbuffer;
PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC glRenderbufferStorageMultisample;
PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;

#ifdef LINUX // conversion function from glx to wgl calls
__GLXextFuncPtr wglGetProcAddress(const char* a) {
    return glXGetProcAddress((const GLubyte*)a);
}
#endif


bool DrawGLUtils::LoadGLFunctions() {
    glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
    glCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader");
    glShaderSource = (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");
    glCompileShader = (PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader");
    glGetShaderiv = (PFNGLGETSHADERIVPROC)wglGetProcAddress("glGetShaderiv");
    glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)wglGetProcAddress("glGetShaderInfoLog");
    glCreateProgram = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");
    glAttachShader = (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");
    glGetProgramiv = (PFNGLGETPROGRAMIVPROC)wglGetProcAddress("glGetProgramiv");
    glGetActiveUniform = (PFNGLGETACTIVEUNIFORMPROC)wglGetProcAddress("glGetActiveUniform");
    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)wglGetProcAddress("glGetProgramInfoLog");
    glDetachShader = (PFNGLDETACHSHADERPROC)wglGetProcAddress("glDetachShader");
    glDeleteShader = (PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader");
    glLinkProgram = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
    glDeleteProgram = (PFNGLDELETEPROGRAMPROC)wglGetProcAddress("glDeleteProgram");
    glIsProgram = (PFNGLISPROGRAMPROC)wglGetProcAddress("glIsProgram");
    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation");
    glUniform1i = (PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i");
    glUniform4f = (PFNGLUNIFORM4FPROC)wglGetProcAddress("glUniform4f");
    glUniform1f = (PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f");
    glUniform2f = (PFNGLUNIFORM2FPROC)wglGetProcAddress("glUniform2f");
    glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)wglGetProcAddress("glUniformMatrix4fv");
    glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
    glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");
    glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
    glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
    glBufferSubData = (PFNGLBUFFERSUBDATAPROC)wglGetProcAddress("glBufferSubData");
    glFlushMappedBufferRange = (PFNGLFLUSHMAPPEDBUFFERRANGEPROC)wglGetProcAddress("glFlushMappedBufferRange");
    glMapBufferRange = (PFNGLMAPBUFFERRANGEPROC)wglGetProcAddress("glMapBufferRange");
    glUnmapBuffer = (PFNGLUNMAPBUFFERPROC)wglGetProcAddress("glUnmapBuffer");
#ifndef LINUX
    glActiveTexture = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");
#endif
    glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)wglGetProcAddress("glGenVertexArrays");
    glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)wglGetProcAddress("glBindVertexArray");
    glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)wglGetProcAddress("glDeleteVertexArrays");
    glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glEnableVertexAttribArray");
    glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)wglGetProcAddress("glVertexAttribPointer");
    glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glDisableVertexAttribArray");
    glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)wglGetProcAddress("glGetAttribLocation");
    glPointParameterf = (PFNGLPOINTPARAMETERFPROC)wglGetProcAddress("glPointParameterf");

    auto ptr = wglGetProcAddress("glGenFramebuffers");
    if ( ptr != nullptr )
      glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)ptr;
    else
       glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)wglGetProcAddress("glGenFramebuffersEXT");
     glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)wglGetProcAddress("glBindFramebuffer");
     glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)wglGetProcAddress("glDeleteFramebuffers");
     glIsFramebuffer = (PFNGLISFRAMEBUFFERPROC)wglGetProcAddress("glIsFramebuffer");
     glFramebufferParameteri = (PFNGLFRAMEBUFFERPARAMETERIPROC)wglGetProcAddress("glFramebufferParameteri");
     glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)wglGetProcAddress("glGenRenderbuffers");
     glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)wglGetProcAddress("glDeleteRenderbuffers");
     glIsRenderbuffer = (PFNGLISRENDERBUFFERPROC)wglGetProcAddress("glIsRenderbuffer");
     glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)wglGetProcAddress("glBindRenderbuffer");
     glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)wglGetProcAddress("glRenderbufferStorage");
     glRenderbufferStorageMultisample = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC)wglGetProcAddress("glRenderbufferStorageMultisample");
     glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)wglGetProcAddress("glFramebufferRenderbuffer");


    return (glUseProgram != nullptr);
}
#else
bool DrawGLUtils::LoadGLFunctions() {
    return true;
}
#endif



class ShaderProgram {
public:
    ShaderProgram() {}

    void Cleanup() {
        if (ProgramID != 0) {
            LOG_GL_ERRORV(glUseProgram(0));
            LOG_GL_ERRORV(glDeleteProgram(ProgramID));
            ProgramID = 0;
        }
    }

    void UseProgram() const {
        LOG_GL_ERRORV(glUseProgram(ProgramID));
    }

    void SetMatrix(glm::mat4 &m) const {
        LOG_GL_ERRORV(glUniformMatrix4fv(MatrixID, 1, GL_FALSE, glm::value_ptr(m)));
    }

    void SetRenderType(int i) {
        LOG_GL_ERRORV(glUniform1i(RenderTypeID, i));
    }

    void UnbindBuffer(int idx) const {
        LOG_GL_ERRORV(glDisableVertexAttribArray(idx));
        LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, 0));
    }

    bool Init(const char * vs, const char * fs) {
        GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
        if (VertexShaderID != 0) {
            GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
            if (FragmentShaderID != 0) {
                valid = CompileShader(vs, VertexShaderID);
                valid &= CompileShader(fs, FragmentShaderID);
                ProgramID = CreateProgram(VertexShaderID, FragmentShaderID);
                valid &= ProgramID != 0;

                if (ProgramID == 0)
                {
                    DrawGLUtils::DoLogGLError(__FILE__, __LINE__, "Failed to create program from vertex and fragment shader");
                }

                glDeleteShader(FragmentShaderID);
            }
            else
            {
                DrawGLUtils::DoLogGLError(__FILE__, __LINE__, "Failed to create fragment shader");
            }
            glDeleteShader(VertexShaderID);
        }
        else
        {
            DrawGLUtils::DoLogGLError(__FILE__, __LINE__, "Failed to create vertex shader");
        }

        if (valid) {
            LOG_GL_ERRORV(glUseProgram(ProgramID));
            LOG_GL_ERRORV(MatrixID = glGetUniformLocation(ProgramID, "MVP"));
            LOG_GL_ERRORV(PointSmoothMinID = glGetUniformLocation(ProgramID, "PointSmoothMin"));
            LOG_GL_ERRORV(PointSmoothMaxID = glGetUniformLocation(ProgramID, "PointSmoothMax"));
            LOG_GL_ERRORV(RenderTypeID = glGetUniformLocation(ProgramID, "RenderType"));
        }

        return valid;
    }

    void CalcSmoothPointParams(float ps) {
        LOG_GL_ERRORV(glPointSize(ps+1));
        float delta = 1.0 / (ps+1);
        float mid = 0.35 + 0.15 * ((ps - 1.0f)/25.0f);
        if (mid > 0.5) {
            mid = 0.5;
        }
        float min = std::max(0.0f, mid - delta);
        float max = std::min(1.0f, mid + delta);
        LOG_GL_ERRORV(glUniform1f(PointSmoothMinID, min));
        LOG_GL_ERRORV(glUniform1f(PointSmoothMaxID, max));
    }

    float CalcSmoothPointParams() {
        float ps;
        LOG_GL_ERRORV(glGetFloatv(GL_POINT_SIZE, &ps));
        CalcSmoothPointParams(ps);
        return ps;
    }

    static GLuint CreateProgram(GLuint vs, GLuint fs) {
        static log4cpp::Category& logger_opengl = log4cpp::Category::getInstance(std::string("log_opengl"));

        GLuint ProgramID = glCreateProgram();
        if (ProgramID != 0) {
            LOG_GL_ERRORV(glAttachShader(ProgramID, vs));
            LOG_GL_ERRORV(glAttachShader(ProgramID, fs));
            LOG_GL_ERRORV(glLinkProgram(ProgramID));

            GLint Result = GL_FALSE;
            int InfoLogLength;

            LOG_GL_ERRORV(glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result));
            LOG_GL_ERRORV(glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength));
            if (!Result) {
                logger_opengl.error("ShaderProgram::CreateProgram failed.");
                if (InfoLogLength > 0) {
                    std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
                    glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
                    wxString l = &ProgramErrorMessage[0];
                    l.Trim();
                    if (l.length() > 0) {
                        printf("Program Log: %s\n", &ProgramErrorMessage[0]);
                        logger_opengl.error(std::string(&ProgramErrorMessage[0]));
                    }
                }
            }
            LOG_GL_ERRORV(glDetachShader(ProgramID, vs));
            LOG_GL_ERRORV(glDetachShader(ProgramID, fs));
        }
        else
        {
            DrawGLUtils::DoLogGLError(__FILE__, __LINE__, "glCreateProgram failed.");
        }
        return ProgramID;
    }

    static bool CompileShader(const char *sourcePointer, GLuint shaderID) {
        static log4cpp::Category& logger_opengl = log4cpp::Category::getInstance(std::string("log_opengl"));

        LOG_GL_ERRORV(glShaderSource(shaderID, 1, &sourcePointer , NULL));
        LOG_GL_ERRORV(glCompileShader(shaderID));

        GLint Result = GL_FALSE;
        int InfoLogLength;

        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &Result);
        glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if (!Result) {
            logger_opengl.error("ShaderProgram::Compile failed.");
            if (InfoLogLength > 0) {
                std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
                glGetShaderInfoLog(shaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
                wxString l = &VertexShaderErrorMessage[0];
                l.Trim();
                if (l.length() > 0) {
                    printf("Shader Log: %s\n", &VertexShaderErrorMessage[0]);
                    logger_opengl.error(std::string(&VertexShaderErrorMessage[0]));
                }
            }
            return false;
        }
        return true;
    }

    GLuint ProgramID = 0;

    GLuint MatrixID = 0;
    GLuint PointSmoothMinID = 0;
    GLuint PointSmoothMaxID = 0;
    GLuint RenderTypeID = 0;

    bool valid = true;
};


ShaderProgram texture3Program;
ShaderProgram singleColor3Program;
ShaderProgram normal3Program;

ShaderProgram meshSolidProgram;
ShaderProgram meshTextureProgram;

bool xlOGL3GraphicsContext::InitializeSharedContext() {

    bool valid = true;

    LOG_GL_ERRORV(DrawGLUtils::LoadGLFunctions());
    
    const GLubyte* str = glGetString(GL_VERSION);
    bool cp = str[0] > '3' || (str[0] == '3' && str[2] >= '3');
    
    if (cp) {
        valid = valid && singleColor3Program.Init(
                                 "#version 330 core\n"
                                 "layout(location = 0) in vec3 vertexPosition_modelspace;\n"
                                 "out vec4 fragmentColor;\n"
                                 "uniform mat4 MVP;\n"
                                 "uniform vec4 inColor;\n"
                                 "void main(){\n"
                                 "    gl_Position = MVP * vec4(vertexPosition_modelspace,1);\n"
                                 "    fragmentColor = inColor;\n"
                                 "}\n",
                                 "#version 330 core\n"
                                 "in vec4 fragmentColor;\n"
                                 "out vec4 color;\n"
                                 "uniform int RenderType = 0;\n"
                                 "uniform float PointSmoothMin = 0.4;\n"
                                 "uniform float PointSmoothMax = 0.5;\n"
                                 "void main(){\n"
                                 "    if (RenderType == 0) {\n"
                                 "        color = fragmentColor;\n"
                                 "    } else {\n"
                                 "        float dist = distance(gl_PointCoord, vec2(0.5));\n"
                                 "        float alpha = 1.0 - smoothstep(PointSmoothMin, PointSmoothMax, dist);\n"
                                 "        if (alpha == 0.0) discard;\n"
                                 "        alpha = alpha * fragmentColor.a;\n"
                                 "        color = vec4(fragmentColor.rgb, alpha);\n"
                                 "    }\n"
                                 "}\n");
        valid = valid && texture3Program.Init(
                             "#version 330 core\n"
                             "layout(location = 0) in vec3 vertexPosition_modelspace;\n"
                             "layout(location = 1) in vec2 vertexUV;\n"
                             "out vec4 fragmentColor;\n"
                             "out vec2 UV;\n"
                             "uniform mat4 MVP;\n"
                             "uniform vec4 inColor;\n"
                             "void main(){\n"
                             "    gl_Position = MVP * vec4(vertexPosition_modelspace,1);\n"
                             "    fragmentColor = inColor;\n"
                             "    UV = vertexUV;\n"
                             "}\n",
                             
                             "#version 330 core\n"
                             "in vec4 fragmentColor;\n"
                             "in vec2 UV;\n"
                             "out vec4 color;\n"
                             "uniform sampler2D tex;\n"
                             "uniform int RenderType = 0;\n"
                             "void main(){\n"
                             "    vec4 c = texture(tex, UV);\n"
                             "    if (RenderType == 0) {\n"
                             "        color = vec4(c.r*fragmentColor.r, c.g*fragmentColor.g, c.b*fragmentColor.b, c.a*fragmentColor.a);\n"
                             "    } else {\n"
                             "        color = vec4(fragmentColor.rgb, c.a * fragmentColor.a);\n"
                             "    }\n"
                             "}\n");
        valid = valid && normal3Program.Init(
                            "#version 330 core\n"
                            "layout(location = 0) in vec3 vertexPosition_modelspace;\n"
                            "layout(location = 1) in vec4 vertexColor;\n"
                            "out vec4 fragmentColor;\n"
                            "uniform int RenderType;\n"
                            "uniform mat4 MVP;\n"
                            "uniform vec4 inColor;\n"
                            "void main(){\n"
                            "    gl_Position = MVP * vec4(vertexPosition_modelspace,1);\n"
                            "    if (RenderType == -2) {\n"
                            "        fragmentColor = inColor;\n"
                            "    } else if (RenderType == -1) {\n"
                            "        fragmentColor = inColor;\n"
                            "    } else {\n"
                            "        fragmentColor = vertexColor;\n"
                            "    }\n"
                            "}\n",
                            "#version 330 core\n"
                            "in vec4 fragmentColor;\n"
                            "out vec4 color;\n"
                            "uniform int RenderType;\n"
                            "uniform float PointSmoothMin = 0.4;\n"
                            "uniform float PointSmoothMax = 0.5;\n"
                            "void main(){\n"
                            "    if (RenderType == 0 || RenderType == -2) {\n"
                            "        color = fragmentColor;\n"
                            "    } else {\n"
                            "        float dist = distance(gl_PointCoord, vec2(0.5));\n"
                            "        float alpha = 1.0 - smoothstep(PointSmoothMin, PointSmoothMax, dist);\n"
                            "        if (alpha == 0.0) discard;\n"
                            "        alpha = alpha * fragmentColor.a;\n"
                            "        color = vec4(fragmentColor.rgb, alpha);\n"
                            "    }\n"
                            "}\n");
        
        
        valid = valid && meshTextureProgram.Init(
                                "#version 330 core\n"
                                "layout(location = 0) in vec3 vertexPosition_modelspace;\n"
                                "layout(location = 1) in vec3 vertexNormal_modelspace;\n"
                                "layout(location = 2) in vec2 vertexUV;\n"
                                "out vec4 fragmentColor;\n"
                                "out vec2 UV;\n"
                                "out float cosTheta;\n"
                                "uniform vec4 inColor;\n"
                                "uniform mat4 MVP;\n"
                                "uniform mat4 NM;\n"
                                "void main(){\n"
                                "    gl_Position = MVP * vec4(vertexPosition_modelspace,1);\n"
                                "    UV = vertexUV;\n"
                                "    if (inColor.a == 1.0) {\n"
                                // Normal of the the vertex, in world space
                                "        vec4 normal_cameraspace = NM * vec4(vertexNormal_modelspace, 0.0);\n"
                                "        vec3 n = normalize(normal_cameraspace.xyz);\n"
                                // Direction of the light (from the fragment to the light)
                                "        vec3 l = normalize(vec3(0.1, 0.1, 1));\n"
                                "        cosTheta = abs(clamp(dot( n, l), -1.0, 1.0));\n"
                                "        vec4 color = vec4(cosTheta, cosTheta, cosTheta, 1.0);\n"
                                "        fragmentColor = (inColor * color) * 0.75 + inColor * 0.75;\n"
                                "    } else {\n"
                                "        cosTheta = 1.0f;\n"
                                "        fragmentColor = inColor;\n"
                                "    }\n"
                                "}\n",
                                
                                "#version 330 core\n"
                                "in vec4 fragmentColor;\n"
                                "in vec2 UV;\n"
                                "in float cosTheta;\n"
                                "out vec4 color;\n"
                                "uniform float brightness;\n"
                                "uniform sampler2D tex;\n"
                                "void main(){\n"
                                "    vec4 c = texture(tex, UV);\n"
                                "    if (cosTheta != 1.0) {\n"
                                "        vec3 c3 = vec3(cosTheta * c.rgb)*0.75 + vec3(c.rgb * 0.25);\n"
                                "        c = vec4(c3, c.a);\n"
                                "    }\n"
                                "    color = vec4(c.r*brightness, c.g*brightness, c.b*brightness, c.a);\n"
                                "}\n");
        valid = valid && meshSolidProgram.Init(
                              "#version 330 core\n"
                              "layout(location = 0) in vec3 vertexPosition_modelspace;\n"
                              "layout(location = 1) in vec3 vertexNormal_modelspace;\n"
                              "out vec4 fragmentColor;\n"
                              "uniform vec4 inColor;\n"
                              "uniform mat4 MVP;\n"
                              "uniform mat4 NM;\n"
                              "void main(){\n"
                              "    gl_Position = MVP * vec4(vertexPosition_modelspace,1);\n"
                              "        vec3 normal_cameraspace = (NM * vec4(vertexNormal_modelspace, 0)).xyz;\n"
                              "        vec3 n = normalize(normal_cameraspace);\n"
                              // Direction of the light (from the fragment to the light)
                              "        vec3 l = normalize(vec3(0.1, 0.1, 1));\n"
                              "        float cosTheta = abs(clamp(dot( n, l), -1.0, 1.0));\n"
                              "        fragmentColor = vec4(inColor.rgb * 0.75 * cosTheta + inColor.rgb * 0.25, inColor.a);\n"
                              "    //fragmentColor = inColor;\n"
                              "}\n",
                              
                              "#version 330 core\n"
                              "in vec4 fragmentColor;\n"
                              "out vec4 color;\n"
                              "void main(){\n"
                              "    color = fragmentColor;\n"
                              "}\n");
    } else {
        valid = valid && singleColor3Program.Init("#version 120\n"
                                 "attribute vec3 vertexPosition_modelspace;\n"
                                 "varying vec4 fragmentColor;\n"
                                 "uniform vec4 inColor;\n"
                                 "uniform mat4 MVP;\n"
                                 "void main(){\n"
                                 "    gl_Position = MVP * vec4(vertexPosition_modelspace,1);\n"
                                 "    fragmentColor = inColor;\n"
                                 "}\n",
        
                                 "#version 120\n"
                                 "varying vec4 fragmentColor;\n"
                                 "uniform int RenderType = 0;\n"
                                 "uniform float PointSmoothMin = 0.5;\n"
                                 "uniform float PointSmoothMax = 0.75;\n"
                                 "void main(){\n"
                                 "    if (RenderType == 0) {\n"
                                 "        gl_FragColor = fragmentColor;\n"
                                 "    } else {\n"
                                 "        float dist = distance(gl_PointCoord, vec2(0.5));\n"
                                 "        float alpha = 1.0 - smoothstep(PointSmoothMin, PointSmoothMax, dist);\n"
                                 "        if (alpha == 0.0) discard;\n"
                                 "        alpha = alpha * fragmentColor.a;\n"
                                 "        gl_FragColor = vec4(fragmentColor.rgb, alpha);\n"
                                 "    }\n"
                                 "}\n");
        valid = valid && normal3Program.Init(
                            "#version 120\n"
                            "attribute vec3 vertexPosition_modelspace;\n"
                            "attribute vec4 vertexColor;\n"
                            "varying vec4 fragmentColor;\n"
                            "uniform mat4 MVP;\n"
                            "uniform int RenderType;\n"
                            "uniform vec4 inColor;\n"
                            "void main(){\n"
                            "    gl_Position = MVP * vec4(vertexPosition_modelspace,1);\n"
                            "    if (RenderType == -2) {\n"
                            "        fragmentColor = inColor;\n"
                            "    } else if (RenderType == -1) {\n"
                            "        fragmentColor = inColor;\n"
                            "    } else {\n"
                            "        fragmentColor = vertexColor;\n"
                            "    }\n"
                            "}\n",
                            "#version 120\n"
                            "varying vec4 fragmentColor;\n"
                            "uniform int RenderType = 0;\n"
                            "uniform float PointSmoothMin = 0.5;\n"
                            "uniform float PointSmoothMax = 0.75;\n"
                            "void main(){\n"
                            "    if (RenderType == 0) {\n"
                            "        gl_FragColor = fragmentColor;\n"
                            "    } else {\n"
                            "        float dist = distance(gl_PointCoord, vec2(0.5));\n"
                            "        float alpha = 1.0 - smoothstep(PointSmoothMin, PointSmoothMax, dist);\n"
                            "        if (alpha == 0.0) discard;\n"
                            "        alpha = alpha * fragmentColor.a;\n"
                            "        gl_FragColor = vec4(fragmentColor.rgb, alpha);\n"
                            "    }\n"
                            "}\n");
        
        valid = valid && texture3Program.Init("#version 120\n"
                             "attribute vec3 vertexPosition_modelspace;\n"
                             "attribute vec2 vertexUV;\n"
                             "varying vec2 textCoord;\n"
                             "varying vec4 fragmentColor;\n"
                             "uniform mat4 MVP;\n"
                             "uniform vec4 inColor;\n"
                             "void main(){\n"
                             "    gl_Position = MVP * vec4(vertexPosition_modelspace,1);\n"
                             "    textCoord = vertexUV;\n"
                             "    fragmentColor = inColor;\n"
                             "}\n",
                             "#version 120\n"
                             "varying vec2 textCoord;\n"
                             "uniform sampler2D tex;\n"
                             "uniform vec4 inColor;\n"
                             "uniform int RenderType = 0;\n"
                             "void main(){\n"
                             "    vec4 col = texture2D(tex, textCoord);\n"
                             "    if (RenderType == 0) {\n"
                             "        gl_FragColor = vec4(col.r*inColor.r, col.g*inColor.g, col.b*inColor.b, col.a * inColor.a);\n"
                             "    } else {\n"
                             "        gl_FragColor = vec4(inColor.rgb, col.a * inColor.a);\n"
                             "    }\n"
                             "}\n");
        
        valid = valid && meshTextureProgram.Init(
                                "#version 120\n"
                                "attribute vec3 vertexPosition_modelspace;\n"
                                "attribute vec3 vertexNormal_modelspace;\n"
                                "attribute vec2 vertexUV;\n"
                                "varying vec4 fragmentColor;\n"
                                "varying vec2 UV;\n"
                                "varying float cosTheta;\n"
                                "uniform vec4 inColor;\n"
                                "uniform mat4 MVP;\n"
                                "uniform mat4 NM;\n"
                                "void main(){\n"
                                "    gl_Position = MVP * vec4(vertexPosition_modelspace,1);\n"
                                "    UV = vertexUV;\n"
                                "    if (inColor.a == 1.0) {\n"
                                // Normal of the the vertex, in world space
                                "        vec4 normal_cameraspace = NM * vec4(vertexNormal_modelspace, 0.0);\n"
                                "        vec3 n = normalize(normal_cameraspace.xyz);\n"
                                // Direction of the light (from the fragment to the light)
                                "        vec3 l = normalize(vec3(0.1, 0.1, 1));\n"
                                "        cosTheta = abs(clamp(dot( n, l), -1.0, 1.0));\n"
                                "        vec4 color = vec4(cosTheta, cosTheta, cosTheta, 1.0);\n"
                                "        fragmentColor = (inColor * color) * 0.75 + inColor * 0.75;\n"
                                "    } else {\n"
                                "        cosTheta = 1.0f;\n"
                                "        fragmentColor = inColor;\n"
                                "    }\n"
                                "}\n",
                                
                                "#version 120\n"
                                "varying vec4 fragmentColor;\n"
                                "varying vec2 UV;\n"
                                "varying float cosTheta;\n"
                                "uniform float brightness;\n"
                                "uniform sampler2D tex;\n"
                                "void main(){\n"
                                "    vec4 c = texture2D(tex, UV);\n"
                                "    //c = vec4( c.r * brightness, c.g * brightness, c.b * brightness, c.a);\n"
                                "    if (cosTheta != 1.0) {\n"
                                "        vec3 c3 = vec3(cosTheta * c.rgb)*0.75 + vec3(c.rgb * 0.25);\n"
                                "        c = vec4(c3, c.a);\n"
                                "    }\n"
                                "    gl_FragColor = vec4(c.r*brightness, c.g*brightness, c.b*brightness, c.a);\n"
                                "}\n");
         
        valid = valid && meshSolidProgram.Init(
                              "#version 120\n"
                              "attribute vec3 vertexPosition_modelspace;\n"
                              "attribute vec3 vertexNormal_modelspace;\n"
                              "varying vec4 fragmentColor;\n"
                              "uniform vec4 inColor;\n"
                              "uniform mat4 MVP;\n"
                              "uniform mat4 NM;\n"
                              "void main(){\n"
                              "    gl_Position = MVP * vec4(vertexPosition_modelspace,1);\n"
                              "        vec3 normal_cameraspace = (NM * vec4(vertexNormal_modelspace, 0)).xyz;\n"
                              "        vec3 n = normalize(normal_cameraspace);\n"
                              // Direction of the light (from the fragment to the light)
                              "        vec3 l = normalize(vec3(0.1, 0.1, 1));\n"
                              "        float cosTheta = abs(clamp(dot( n, l), -1.0, 1.0));\n"
                              "        fragmentColor = vec4(inColor.rgb * 0.75 * cosTheta + inColor.rgb * 0.25, inColor.a);\n"
                              "}\n",
                              "#version 120\n"
                              "varying vec4 fragmentColor;\n"
                              "void main(){\n"
                              "    gl_FragColor = fragmentColor;\n"
                              "}\n"
                              );
    }

    return valid;
}

class xlGLTexture : public xlTexture {
public:
    xlGLTexture(bool cp) : xlTexture(), coreProfile(cp) {}
    xlGLTexture(const wxImage &i, bool cp) : xlTexture(), coreProfile(cp)  {
        LoadImage(i);
    }

    xlGLTexture(int w, int h, bool bgr, bool alpha, bool cp) : xlTexture(), coreProfile(cp) {
        this->alpha = alpha;
        LOG_GL_ERRORV( glGenTextures( 1, &_texId ) );
        LOG_GL_ERRORV( glBindTexture( GL_TEXTURE_2D, _texId ) );

        GLuint tp = bgr ? GL_BGRA : GL_RGBA;
        LOG_GL_ERRORV( glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, tp, GL_UNSIGNED_BYTE, nullptr ) );
        LOG_GL_ERRORV( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR ) );
        LOG_GL_ERRORV( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ) );
        LOG_GL_ERRORV( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE ) );
        LOG_GL_ERRORV( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE ) );

        LOG_GL_ERRORV( glBindTexture( GL_TEXTURE_2D, 0 ) );
        width = w;
        height = h;
    }
    virtual ~xlGLTexture() {
        if (_texId != 0) {
            glDeleteTextures(1, &_texId);
        }
    }
    virtual void UpdatePixel(int x, int y, const xlColor &c, bool copyAlpha) override {
        int bytesPerPixel = copyAlpha ?  4 : 3;
        GLubyte *imageData = new GLubyte[bytesPerPixel];
        imageData[0] = c.red;
        imageData[1] = c.green;
        imageData[2] = c.blue;
        if (copyAlpha) {
            imageData[3] = c.alpha;
        }
        if (!coreProfile) {
            LOG_GL_ERRORV(glEnable(GL_TEXTURE_2D));
        }
        LOG_GL_ERRORV(glBindTexture(GL_TEXTURE_2D, _texId));
        LOG_GL_ERRORV(glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, 1, 1, copyAlpha ?  GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, imageData));
        delete [] imageData;
        if (!coreProfile) {
            LOG_GL_ERRORV(glDisable(GL_TEXTURE_2D));
        }
    }
    virtual void UpdateData(uint8_t *data, bool bgr, bool alpha) override {
        LOG_GL_ERRORV( glBindTexture( GL_TEXTURE_2D, _texId ) );
        if (bgr && alpha) {
            LOG_GL_ERRORV( glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, width, height, GL_BGRA, GL_UNSIGNED_BYTE, data ) );
        } else if (bgr && !alpha) {
            LOG_GL_ERRORV( glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, width, height, GL_BGR, GL_UNSIGNED_BYTE, data ) );
        } else if (!bgr && alpha) {
            LOG_GL_ERRORV( glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data ) );
        } else if (!bgr && !alpha) {
            LOG_GL_ERRORV( glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data ) );
        }
        LOG_GL_ERRORV( glBindTexture( GL_TEXTURE_2D, 0 ) );
    }
    void LoadImage(wxImage image) {
        if (!coreProfile) {
            LOG_GL_ERRORV(glEnable(GL_TEXTURE_2D));
        }
        int maxSize = 0;
        LOG_GL_ERRORV(glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize));
        LOG_GL_ERRORV(glGenTextures( 1, &_texId ));
        LOG_GL_ERRORV(glBindTexture( GL_TEXTURE_2D, _texId ));

        width = image.GetWidth();
        height = image.GetHeight();

        LOG_GL_ERRORV(glPixelStorei(GL_UNPACK_ALIGNMENT,  1 ));

        if (width > maxSize || height > maxSize) {
            int newWid = std::min(width, maxSize);
            int newHi = std::min(height, maxSize);

            image = image.Rescale(newWid, newHi, wxIMAGE_QUALITY_HIGH);
        }
        width = image.GetWidth();
        height = image.GetHeight();

        // note: must make a local copy before passing the data to OpenGL, as GetData() returns RGB
        // and we want the Alpha channel if it's present.
        GLubyte *bitmapData = image.GetData();
        GLubyte *alphaData = image.GetAlpha();

        alpha = image.HasAlpha();
        int bytesPerPixel = image.HasAlpha() ?  4 : 3;

        int imageSize = width * height * bytesPerPixel;
        GLubyte *imageData = new GLubyte[imageSize];

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                imageData[(x + y * width) * bytesPerPixel] = bitmapData[(x + y * width) * 3];
                imageData[(x + y * width) * bytesPerPixel + 1] = bitmapData[(x + y * width) * 3 + 1];
                imageData[(x + y * width) * bytesPerPixel + 2] = bitmapData[(x + y * width) * 3 + 2];
                if (bytesPerPixel==4) {
                    imageData[(x + y * width) * bytesPerPixel + 3] = alphaData[x + y * width];
                }
            }
        }

        // if yes, everything is fine
        LOG_GL_ERRORV(glTexImage2D(GL_TEXTURE_2D,
                     0,
                     alpha ? GL_RGBA : GL_RGB,
                     width,
                     height,
                     0,
                     alpha ?  GL_RGBA : GL_RGB,
                     GL_UNSIGNED_BYTE,
                     imageData));

        delete [] imageData;
        // set texture parameters as you wish
        LOG_GL_ERRORV(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)); // GL_LINEAR
        LOG_GL_ERRORV(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)); // GL_LINEAR
        LOG_GL_ERRORV(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        LOG_GL_ERRORV(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        if (!coreProfile) {
            LOG_GL_ERRORV(glDisable(GL_TEXTURE_2D));
        }
    }

    GLuint _texId = 0;
    int width = 0;
    int height = 0;
    bool alpha = true;
    bool coreProfile = true;
};

xlOGL3GraphicsContext::xlOGL3GraphicsContext(xlGLCanvas *c) : xlGraphicsContext(c), canvas(c) {
}
xlOGL3GraphicsContext::~xlOGL3GraphicsContext() {}

class xlOGL3VertexAccumulator : public xlVertexAccumulator {
public:
    xlOGL3VertexAccumulator() {}
    virtual ~xlOGL3VertexAccumulator() {
        if (bufferIdx) {
            LOG_GL_ERRORV(glDeleteBuffers(1, &bufferIdx));
        }
    }

    virtual void Reset() override {
        if (!finalized) {
            count = 0;
            vertices.resize(0);
        }
    }
    virtual void PreAlloc(unsigned int i) override {
        vertices.reserve(i * 3);
    }
    virtual void AddVertex(float x, float y, float z) override {
        if (!finalized) {
            vertices.emplace_back(x);
            vertices.emplace_back(y);
            vertices.emplace_back(z);
            changed = true;
            count++;
        }
    }
    virtual uint32_t getCount() override {
        return count;
    }
    virtual void Finalize(bool mc) override {
        finalized = true;
        mayChange = mc;
    }
    virtual void SetVertex(uint32_t vertex, float x, float y, float z) override {
        if (vertex < count) {
            vertices[vertex * 3] = x;
            vertices[vertex * 3 + 1] = y;
            vertices[vertex * 3 + 2] = z;
            changed = true;
        }
    }
    virtual void FlushRange(uint32_t start, uint32_t len) override {
        if (len && bufferIdx && (!finalized || mayChange) && changed) {
            LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, bufferIdx));
            if (start == 0 && len == count) {
                LOG_GL_ERRORV(glBufferData(GL_ARRAY_BUFFER, count * sizeof(float) * 3, &vertices[0], GL_DYNAMIC_DRAW));
            } else {
                uint32_t s = start * sizeof(float) * 3;
                uint32_t l = len * sizeof(float) * 3;
                LOG_GL_ERRORV(glBufferSubData(GL_ARRAY_BUFFER, s, l, &vertices[start * 3]));
            }
            changed = false;
        }
    }
    
    void SetBufferBytes(int idx) {
        if (!bufferIdx) {
            LOG_GL_ERRORV(glGenBuffers(1, &bufferIdx));
        }
        LOG_GL_ERRORV(glEnableVertexAttribArray(idx));
        LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, bufferIdx));
        if (changed) {
            LOG_GL_ERRORV(glBufferData(GL_ARRAY_BUFFER, count * sizeof(float) * 3, &vertices[0], mayChange ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW));
            changed = false;
        }
        LOG_GL_ERRORV(glVertexAttribPointer(idx, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 ));
    }
    uint32_t count = 0;
    std::vector<float> vertices;

    bool finalized = false;
    bool mayChange = false;

    GLuint bufferIdx = 0;
    bool changed = false;
};

class xlOGL3VertexColorAccumulator : public xlVertexColorAccumulator {
public:
    xlOGL3VertexColorAccumulator() {}
    virtual ~xlOGL3VertexColorAccumulator() {
        if (vbuffer) {
            LOG_GL_ERRORV(glDeleteBuffers(1, &vbuffer));
        }
        if (cbuffer) {
            LOG_GL_ERRORV(glDeleteBuffers(1, &cbuffer));
        }
    }
    virtual uint32_t getCount() override {
        return count;
    }
    virtual void Reset() override {
        if (!finalized) {
            count = 0;
            vertices.resize(0);
            colors.resize(0);
        }
    }
    virtual void PreAlloc(unsigned int i) override {
        vertices.reserve(i * 3);
        colors.reserve(i);
    }
    virtual void AddVertex(float x, float y, float z, const xlColor &c) override {
        if (!finalized) {
            vertices.emplace_back(x);
            vertices.emplace_back(y);
            vertices.emplace_back(z);
            
            colors.emplace_back(c.GetRGBA());

            vchanged = true;
            cchanged = true;
            count++;
        }
    }

    virtual void Finalize(bool mcv, bool mcc) override {
        finalized = true;
        mayChangeVertices = mcv;
        mayChangeColors = mcc;
    }
    virtual void SetVertex(uint32_t vertex, float x, float y, float z, const xlColor &c) override {
        if (vertex < count) {
            vertices[vertex * 3] = x;
            vertices[vertex * 3 + 1] = y;
            vertices[vertex * 3 + 2] = z;
            colors[vertex] = c.GetRGBA();
            cchanged = true;
            vchanged = true;
        }
    }
    virtual void SetVertex(uint32_t vertex, float x, float y, float z) override {
        if (vertex < count) {
            vertices[vertex * 3] = x;
            vertices[vertex * 3 + 1] = y;
            vertices[vertex * 3 + 2] = z;
            vchanged = true;
        }
    }
    virtual void SetVertex(uint32_t vertex, const xlColor &c) override {
        if (vertex < count) {
            colors[vertex] = c.GetRGBA();
            cchanged = true;
        }
    }
    virtual void FlushRange(uint32_t start, uint32_t len) override {
        if (len) {
            if (vbuffer && (!finalized || mayChangeVertices) && vchanged) {
                LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, vbuffer));
                if (start == 0 && len == count) {
                    LOG_GL_ERRORV(glBufferData(GL_ARRAY_BUFFER, count * sizeof(float) * 3, &vertices[0], GL_DYNAMIC_DRAW));
                } else {
                    uint32_t s = start * sizeof(float) * 3;
                    uint32_t l = len * sizeof(float) * 3;
                    LOG_GL_ERRORV(glBufferSubData(GL_ARRAY_BUFFER, s, l, &vertices[start * 3]));
                }
                vchanged = false;
            }
            if (cbuffer && (!finalized || mayChangeColors) && cchanged) {
                LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, cbuffer));
                if (start == 0 && len == count) {
                    LOG_GL_ERRORV(glBufferData(GL_ARRAY_BUFFER, count * sizeof(uint32_t), &colors[0], GL_DYNAMIC_DRAW));
                } else {
                    uint32_t s = start * sizeof(uint32_t);
                    uint32_t l = len * sizeof(uint32_t);
                        LOG_GL_ERRORV(glBufferSubData(GL_ARRAY_BUFFER, s, l, &colors[start]));
                }
                cchanged = false;
            }
        }
    }


    void SetBufferBytes(int indexV, int indexC) {
        if (!vbuffer) {
            LOG_GL_ERRORV(glGenBuffers(1, &vbuffer));
        }
        if (!cbuffer) {
            LOG_GL_ERRORV(glGenBuffers(1, &cbuffer));
        }

        LOG_GL_ERRORV(glEnableVertexAttribArray(indexV));
        LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, vbuffer));
        if (vchanged) {
            LOG_GL_ERRORV(glBufferData(GL_ARRAY_BUFFER, count * sizeof(float) * 3, &vertices[0], mayChangeVertices ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW));
            vchanged = false;
        }
        LOG_GL_ERRORV(glVertexAttribPointer(indexV, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 ));

        LOG_GL_ERRORV(glEnableVertexAttribArray(indexC));
        LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, cbuffer));
        if (cchanged) {
            LOG_GL_ERRORV(glBufferData(GL_ARRAY_BUFFER, count * sizeof(uint32_t), &colors[0], mayChangeColors ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW));
            cchanged = false;
        }
        LOG_GL_ERRORV(glVertexAttribPointer(indexC, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (void*)0 ));
    }

    uint32_t count = 0;
    std::vector<float> vertices;
    std::vector<uint32_t> colors;

    bool finalized = false;
    bool mayChangeColors = false;
    bool mayChangeVertices = false;
    
    bool vchanged = false;
    bool cchanged = false;


    GLuint vbuffer = 0;
    GLuint cbuffer = 0;
};


class xlOGL3VertexTextureAccumulator : public xlVertexTextureAccumulator {
public:
    xlOGL3VertexTextureAccumulator() {}
    virtual ~xlOGL3VertexTextureAccumulator() {
        if (vbuffer) {
            LOG_GL_ERRORV(glDeleteBuffers(1, &vbuffer));
        }
        if (tbuffer) {
            LOG_GL_ERRORV(glDeleteBuffers(1, &tbuffer));
        }
    }

    virtual void Reset() override {
        if (!finalized) {
            count = 0;
            vertices.resize(0);
            tvertices.resize(0);
        }
    }
    virtual void PreAlloc(unsigned int i) override {
        vertices.reserve(i * 3);
        tvertices.reserve(i * 2);
    }
    virtual void AddVertex(float x, float y, float z, float tx, float ty) override {
        if (!finalized) {
            vertices.emplace_back(x);
            vertices.emplace_back(y);
            vertices.emplace_back(z);
            
            tvertices.emplace_back(tx);
            tvertices.emplace_back(ty);
            vchanged = true;
            tchanged = true;
            count++;
        }
    }
    virtual uint32_t getCount() override { return count; }


    virtual void Finalize(bool mcv, bool mct) override {
        finalized = true;
        mayChangeVertices = mcv;
        mayChangeTextures = mct;
    }

    virtual void SetVertex(uint32_t vertex, float x, float y, float z, float tx, float ty) override {
        if (vertex < count) {
            vertices[vertex * 3] = x;
            vertices[vertex * 3 + 1] = y;
            vertices[vertex * 3 + 2] = z;
            vchanged = true;
            
            tvertices[vertex * 2] = tx;
            tvertices[vertex * 2 + 1] = ty;
            tchanged = true;
        }
    }

    virtual void FlushRange(uint32_t start, uint32_t len) override {
        if (vbuffer && (!finalized || mayChangeVertices) && vchanged) {
            LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, vbuffer));
            if (start == 0 && len == count) {
                LOG_GL_ERRORV(glBufferData(GL_ARRAY_BUFFER, count * sizeof(float) * 3, &vertices[0], GL_DYNAMIC_DRAW));
            } else {
                uint32_t s = start * sizeof(float) * 3;
                uint32_t l = len * sizeof(float) * 3;
                LOG_GL_ERRORV(glBufferSubData(GL_ARRAY_BUFFER, s, l, &vertices[start * 3]));
            }
            vchanged = false;
        }
        if (tbuffer && (!finalized || mayChangeTextures) && tchanged) {
            LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, tbuffer));
            if (start == 0 && len == count) {
                LOG_GL_ERRORV(glBufferData(GL_ARRAY_BUFFER, count * sizeof(float) * 2, &tvertices[0], GL_DYNAMIC_DRAW));
            } else {
                uint32_t s = start * sizeof(float) * 2;
                uint32_t l = len * sizeof(float) * 2;
                LOG_GL_ERRORV(glBufferSubData(GL_ARRAY_BUFFER, s, l, &tvertices[start * 2]));
            }
            tchanged = false;
        }
    }

    void SetBufferBytes(int indexV, int indexT) {
        if (!vbuffer) {
            LOG_GL_ERRORV(glGenBuffers(1, &vbuffer));
        }
        if (!tbuffer) {
            LOG_GL_ERRORV(glGenBuffers(1, &tbuffer));
        }

        LOG_GL_ERRORV(glEnableVertexAttribArray(indexV));
        LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, vbuffer));
        if (vchanged) {
            LOG_GL_ERRORV(glBufferData(GL_ARRAY_BUFFER, count * sizeof(float) * 3, &vertices[0], mayChangeVertices ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW));
            vchanged = false;
        }
        LOG_GL_ERRORV(glVertexAttribPointer(indexV, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 ));

        LOG_GL_ERRORV(glEnableVertexAttribArray(indexT));
        LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, tbuffer));
        if (tchanged) {
            LOG_GL_ERRORV(glBufferData(GL_ARRAY_BUFFER, count * sizeof(float) * 2, &tvertices[0], mayChangeTextures ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW));
            tchanged = false;
        }
        LOG_GL_ERRORV(glVertexAttribPointer(indexT, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 ));
    }

    uint32_t count = 0;
    std::vector<float> vertices;
    std::vector<float> tvertices;
    bool vchanged = true;
    bool tchanged = true;

    bool finalized = false;
    bool mayChangeVertices = false;
    bool mayChangeTextures = false;
    
    GLuint vbuffer = 0;
    GLuint tbuffer = 0;
};



xlVertexAccumulator *xlOGL3GraphicsContext::createVertexAccumulator() {
    return new xlOGL3VertexAccumulator();
}
xlVertexColorAccumulator *xlOGL3GraphicsContext::createVertexColorAccumulator() {
    return new xlOGL3VertexColorAccumulator();
}

xlVertexTextureAccumulator *xlOGL3GraphicsContext::createVertexTextureAccumulator() {
    return new xlOGL3VertexTextureAccumulator();
}

static void addMipMap(const wxImage& l_Image, int& level) {
    if (l_Image.IsOk() == true) {
        LOG_GL_ERRORV(glTexImage2D(GL_TEXTURE_2D, level, GL_RGB, (GLsizei)l_Image.GetWidth(), (GLsizei)l_Image.GetHeight(),
            0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)l_Image.GetData()));
        int err = glGetError();
        if (err == GL_NO_ERROR) {
            level++;
        } else {
            static log4cpp::Category& logger_opengl = log4cpp::Category::getInstance(std::string("log_opengl"));
            logger_opengl.error("Error glTexImage2D: %d", err);
        }
    }
}
static void CreateOrUpdateTexture(const wxBitmap &bmp48,
                                  const wxBitmap &bmp32,
                                  const wxBitmap &bmp16,
                                  GLuint *texture) {
    int level = 0;
    LOG_GL_ERRORV(glGenTextures(1,texture));
    LOG_GL_ERRORV(glBindTexture(GL_TEXTURE_2D, *texture));
    LOG_GL_ERRORV(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    LOG_GL_ERRORV(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST));

    addMipMap(bmp48.ConvertToImage(), level);
    addMipMap(bmp32.ConvertToImage(), level);
    addMipMap(bmp16.ConvertToImage(), level);
    int w = bmp16.GetScaledWidth() / 2;
    while (w > 0) {
        addMipMap(bmp16.ConvertToImage().Rescale(w, w, wxIMAGE_QUALITY_HIGH), level);
        w = w / 2;
    }
}
xlTexture *xlOGL3GraphicsContext::createTextureMipMaps(const std::vector<wxBitmap> &bitmaps) {
    xlGLTexture *t = new xlGLTexture(canvas->IsCoreProfile());
    GLuint tid = 0;
    CreateOrUpdateTexture(bitmaps[0], bitmaps[1], bitmaps[2], &tid);
    t->_texId = tid;
    return t;
}
xlTexture *xlOGL3GraphicsContext::createTextureMipMaps(const std::vector<wxImage> &images) {
    xlGLTexture *t = new xlGLTexture(canvas->IsCoreProfile());
    GLuint tid = 0;
    CreateOrUpdateTexture(wxBitmap(images[0]), wxBitmap(images[1]), wxBitmap(images[2]), &tid);
    t->_texId = tid;
    t->width = images[0].GetWidth();
    t->height = images[0].GetHeight();
    return t;
}
xlTexture *xlOGL3GraphicsContext::createTexture(const wxImage &image) {
    return new xlGLTexture(image, canvas->IsCoreProfile());
}
xlTexture *xlOGL3GraphicsContext::createTexture(int w, int h, bool bgr, bool alpha) {
    return new xlGLTexture(w, h, bgr, alpha, canvas->IsCoreProfile());
}
xlTexture *xlOGL3GraphicsContext::createTextureForFont(const xlFontInfo &font) {
    return createTexture(font.getImage());
}
xlGraphicsProgram *xlOGL3GraphicsContext::createGraphicsProgram() {
    return new xlGraphicsProgram(createVertexColorAccumulator());
}


//drawing methods

xlGraphicsContext* xlOGL3GraphicsContext::drawLines(xlVertexAccumulator *vac, const xlColor &c, int start, int count) {
    return drawPrimitive(GL_LINES, vac, c, start, count);
}
xlGraphicsContext* xlOGL3GraphicsContext::drawLineStrip(xlVertexAccumulator *vac, const xlColor &c, int start, int count) {
    return drawPrimitive(GL_LINE_STRIP, vac, c, start, count);
}
xlGraphicsContext* xlOGL3GraphicsContext::drawTriangles(xlVertexAccumulator *vac, const xlColor &c, int start, int count) {
    return drawPrimitive(GL_TRIANGLES, vac, c, start, count);
}
xlGraphicsContext* xlOGL3GraphicsContext::drawTriangleStrip(xlVertexAccumulator *vac, const xlColor &c, int start, int count) {
    return drawPrimitive(GL_TRIANGLE_STRIP, vac, c, start, count);
}

xlGraphicsContext* xlOGL3GraphicsContext::drawPoints(xlVertexAccumulator *vac, const xlColor &c, float pointSize, bool smoothPoints, int start, int count) {
    LOG_GL_ERRORV(glPointSize(pointSize));
    drawPrimitive(GL_POINTS, vac, c, start, count);
    return this;
}
xlGraphicsContext* xlOGL3GraphicsContext::drawPrimitive(int type, xlVertexAccumulator *vac, const xlColor &color, int start, int count) {
    xlOGL3VertexAccumulator *v = dynamic_cast<xlOGL3VertexAccumulator*>(vac);
    if (v->getCount() == 0) {
        return this;
    }
    int caps = enableCapabilities;
    if (isBlending && (type == GL_LINES || type == GL_LINE_STRIP)) {
        caps = GL_LINE_SMOOTH;
    }
    int c = count;
    if (c < 0) {
        c = v->count - start;
    }
    if (c <= 0) {
        return this;
    }
    ShaderProgram *program = &singleColor3Program;
    program->UseProgram();
    program->SetMatrix(frameData.MVP);
    int bid = 0;
    if (!canvas->bindVertexArrayID(program->ProgramID)) {
        bid = glGetAttribLocation(program->ProgramID, "vertexPosition_modelspace" );
    }
    v->SetBufferBytes(bid);
    LOG_GL_ERRORV(GLuint cid = glGetUniformLocation(program->ProgramID, "inColor"));
    LOG_GL_ERRORV(glUniform4f(cid,
                ((float)color.Red())/255.0,
                ((float)color.Green())/255.0,
                ((float)color.Blue())/255.0,
                ((float)color.Alpha())/255.0
                ));
    float ps = 0;
    if (type == GL_POINTS && caps == GL_POINT_SMOOTH) {
        program->SetRenderType(1);
        ps = program->CalcSmoothPointParams();
    } else if (caps > 0) {
        LOG_GL_ERRORV(glEnable(caps));
    }
    LOG_GL_ERRORV(glDrawArrays(type, start, c));
    if (type == GL_POINTS && caps == GL_POINT_SMOOTH) {
        program->SetRenderType(0);
        LOG_GL_ERRORV(glPointSize(ps));
    } else if (caps > 0) {
        LOG_GL_ERRORV(glDisable(caps));
    }
    program->UnbindBuffer(bid);
    return this;
}

xlGraphicsContext* xlOGL3GraphicsContext::drawLines(xlVertexColorAccumulator *vac, int start, int count) {
    return drawPrimitive(GL_LINES, vac, start, count);
}
xlGraphicsContext* xlOGL3GraphicsContext::drawLineStrip(xlVertexColorAccumulator *vac, int start, int count) {
    return drawPrimitive(GL_LINE_STRIP, vac, start, count);
}
xlGraphicsContext* xlOGL3GraphicsContext::drawTriangles(xlVertexColorAccumulator *vac, int start, int count) {
    return drawPrimitive(GL_TRIANGLES, vac, start, count);
}
xlGraphicsContext* xlOGL3GraphicsContext::drawTriangleStrip(xlVertexColorAccumulator *vac, int start, int count) {
    return drawPrimitive(GL_TRIANGLE_STRIP, vac, start, count);
}
xlGraphicsContext* xlOGL3GraphicsContext::drawPoints(xlVertexColorAccumulator *vac, float pointSize, bool smoothPoints, int start, int count) {
    LOG_GL_ERRORV(glPointSize(pointSize));
    int c1 = enableCapabilities;
    if (smoothPoints && c1 != GL_POINT_SMOOTH) {
        enableCapabilities = GL_POINT_SMOOTH;
    }
    drawPrimitive(GL_POINTS, vac, start, count);
    enableCapabilities = c1;
    return this;
}

xlGraphicsContext* xlOGL3GraphicsContext::drawPrimitive(int type, xlVertexColorAccumulator *vac, int start, int count) {
    if (vac->getCount() == 0) {
        return this;
    }
    int c = count;
    if (c < 0) {
        c = vac->getCount() - start;
    }
    if (c <= 0) {
        return this;
    }
    ShaderProgram *program = &normal3Program;
    xlOGL3VertexColorAccumulator *v = dynamic_cast<xlOGL3VertexColorAccumulator*>(vac);
    program->UseProgram();
    program->SetMatrix(frameData.MVP);
    
    
    int bid = 0;
    int cid = 1;
    if (!canvas->bindVertexArrayID(program->ProgramID)) {
        bid = glGetAttribLocation(program->ProgramID, "vertexPosition_modelspace" );
        cid = glGetAttribLocation(program->ProgramID, "vertexColor" );
    }
    v->SetBufferBytes(bid, cid);

    int caps = enableCapabilities;
    if (isBlending && (type == GL_LINES || type == GL_LINE_STRIP)) {
        caps = GL_LINE_SMOOTH;
    }

    float ps = 2.0;
    if (type == GL_POINTS && caps == GL_POINT_SMOOTH) {
        program->SetRenderType(1);
        ps = program->CalcSmoothPointParams();
    } else {
        if (caps > 0) {
            LOG_GL_ERRORV(glEnable(caps));
        } else if (caps != 0) {
            program->SetRenderType(caps);
        }
    }
    LOG_GL_ERRORV(glDrawArrays(type, start, c));
    if (type == GL_POINTS && caps == 0x0B10) {
        program->SetRenderType(0);
        LOG_GL_ERRORV(glPointSize(ps));
    } else if (caps > 0) {
        LOG_GL_ERRORV(glDisable(caps));
    } else if (caps != 0) {
        program->SetRenderType(0);
    }

    program->UnbindBuffer(bid);
    program->UnbindBuffer(cid);
    
    return this;
}

class glVertexIndexedColorAccumulator : public xlVertexIndexedColorAccumulator {
public:
    virtual void Reset() override {
        vac.Reset();
    }
    virtual void PreAlloc(unsigned int i) override {
        vac.PreAlloc(i);
        colorIndexes.reserve(i);
    };
    virtual void AddVertex(float x, float y, float z, uint32_t cIdx) override {
        vac.AddVertex(x, y, z, xlBLACK);
        colorIndexes.push_back(cIdx);
    }
    virtual uint32_t getCount() override { return vac.getCount(); }

    virtual void SetColorCount(int c) override {
        colors.resize(c);
    }
    virtual uint32_t GetColorCount() override { return colors.size(); }
    virtual void SetColor(uint32_t idx, const xlColor &c) override {
        colors[idx] = c;
    }
    
    // mark this as ready to be copied to graphics card, after finalize,
    // vertices cannot be added, but if mayChange is set, the vertex/color
    // data can change via SetVertex and then flushed to push the
    // new data to the graphics card
    virtual void Finalize(bool mayChangeVertices, bool mayChangeColors) override {
        for (size_t x = 0; x < colorIndexes.size(); ++x) {
            vac.SetVertex(x, colors[colorIndexes[x]]);
        }
        vac.Finalize(mayChangeVertices, mayChangeColors);
    }
    
    virtual void SetVertex(uint32_t vertex, float x, float y, float z, uint32_t cIdx) override  {
        vac.SetVertex(vertex, x, y, z);
        colorIndexes[vertex] = cIdx;
    }
    virtual void SetVertex(uint32_t vertex, float x, float y, float z) override {
        vac.SetVertex(vertex, x, y, z);
    }
    virtual void SetVertex(uint32_t vertex, uint32_t cIdx) override {
        colorIndexes[vertex] = cIdx;
    }
    virtual void FlushRange(uint32_t start, uint32_t len) override {
        vac.FlushRange(start, len);
    }
    virtual void FlushColors(uint32_t start, uint32_t len) override {
        for (size_t x = 0; x < getCount(); ++x) {
            uint32_t idx = colorIndexes[x];
            if (idx >= start && (idx < (start + len))) {
                vac.SetVertex(x, colors[idx]);
            }
        }
        vac.FlushRange(0, getCount());
    }
    
    xlOGL3VertexColorAccumulator vac;
    std::vector<uint32_t> colorIndexes;
    std::vector<xlColor> colors;
};
xlVertexIndexedColorAccumulator *xlOGL3GraphicsContext::createVertexIndexedColorAccumulator() {
    return new glVertexIndexedColorAccumulator();
}
xlGraphicsContext* xlOGL3GraphicsContext::drawLines(xlVertexIndexedColorAccumulator *vac, int start, int count) {
    glVertexIndexedColorAccumulator *va = (glVertexIndexedColorAccumulator*)vac;
    drawLines(&va->vac, start, count);
    return this;
}
xlGraphicsContext* xlOGL3GraphicsContext::drawLineStrip(xlVertexIndexedColorAccumulator *vac, int start, int count) {
    glVertexIndexedColorAccumulator *va = (glVertexIndexedColorAccumulator*)vac;
    drawLineStrip(&va->vac, start, count);
    return this;
}
xlGraphicsContext* xlOGL3GraphicsContext::drawTriangles(xlVertexIndexedColorAccumulator *vac, int start, int count) {
    glVertexIndexedColorAccumulator *va = (glVertexIndexedColorAccumulator*)vac;
    drawTriangles(&va->vac, start, count);
    return this;
}
xlGraphicsContext* xlOGL3GraphicsContext::drawTriangleStrip(xlVertexIndexedColorAccumulator *vac, int start, int count) {
    glVertexIndexedColorAccumulator *va = (glVertexIndexedColorAccumulator*)vac;
    drawTriangleStrip(&va->vac, start, count);
    return this;
}
xlGraphicsContext* xlOGL3GraphicsContext::drawPoints(xlVertexIndexedColorAccumulator *vac, float pointSize, bool smoothPoints, int start, int count) {
    glVertexIndexedColorAccumulator *va = (glVertexIndexedColorAccumulator*)vac;
    drawPoints(&va->vac, pointSize, smoothPoints, start, count);
    return this;
}


xlGraphicsContext* xlOGL3GraphicsContext::drawTexture(xlTexture *texture,
                         float x, float y, float x2, float y2,
                         float tx, float ty, float tx2, float ty2,
                         bool nearest,
                         int brightness, int alpha) {
    xlOGL3VertexTextureAccumulator va;
    va.PreAlloc(6);

    va.AddVertex(x, y, 0, tx, ty);
    va.AddVertex(x, y2, 0, tx, ty2);
    va.AddVertex(x2, y2, 0, tx2, ty2);
    va.AddVertex(x, y, 0, tx, ty);
    va.AddVertex(x2, y2, 0, tx2, ty2);
    va.AddVertex(x2, y, 0, tx2, ty);
    return drawTexture(&va, texture, brightness, alpha, 0, 6);
}
xlGraphicsContext* xlOGL3GraphicsContext::drawTexture(xlVertexTextureAccumulator *vac, xlTexture *texture, int brightness, uint8_t alpha, int start, int count) {
    xlOGL3VertexTextureAccumulator *va = dynamic_cast<xlOGL3VertexTextureAccumulator*>(vac);
    xlGLTexture *t = (xlGLTexture*)texture;

    if (va->count == 0) {
        return this;
    }
    int c = count;
    if (c < 0) {
        c = va->count - start;
    }
    if (c <= 0) {
        return this;
    }
    texture3Program.UseProgram();
    texture3Program.SetMatrix(frameData.MVP);
    
    int bid = 0;
    int vid = 1;
    if (!canvas->bindVertexArrayID(texture3Program.ProgramID)) {
        bid = glGetAttribLocation(texture3Program.ProgramID, "vertexPosition_modelspace" );
        vid = glGetAttribLocation(texture3Program.ProgramID, "vertexUV" );
    }
    va->SetBufferBytes(bid, vid);

    LOG_GL_ERRORV(glActiveTexture(GL_TEXTURE0)); //switch to texture image unit 0
    LOG_GL_ERRORV(glBindTexture(GL_TEXTURE_2D, t->_texId));
    LOG_GL_ERRORV(glUniform1i(glGetUniformLocation(texture3Program.ProgramID, "tex"), 0));

    texture3Program.SetRenderType(0);
    GLuint cid = glGetUniformLocation(texture3Program.ProgramID, "inColor");
    float b = brightness / 100.0f;
    LOG_GL_ERRORV(glUniform4f(cid, b, b, b, ((float)alpha)/255.0));

    
    if (enableCapabilities > 0) {
        LOG_GL_ERRORV(glEnable(enableCapabilities));
    }
    LOG_GL_ERRORV(glDrawArrays(GL_TRIANGLES, start, c));
    if (enableCapabilities > 0) {
        LOG_GL_ERRORV(glDisable(enableCapabilities));
    }
    LOG_GL_ERRORV(glBindTexture(GL_TEXTURE_2D, 0));
    LOG_GL_ERRORV(glActiveTexture(GL_TEXTURE0)); //switch to texture image unit 0

    texture3Program.UnbindBuffer(bid);
    texture3Program.UnbindBuffer(vid);
    return this;
}
xlGraphicsContext* xlOGL3GraphicsContext::drawTexture(xlVertexTextureAccumulator *vac, xlTexture *texture, const xlColor &color, int start, int count) {
    xlOGL3VertexTextureAccumulator *va = dynamic_cast<xlOGL3VertexTextureAccumulator*>(vac);
    xlGLTexture *t = (xlGLTexture*)texture;

    if (va->count == 0) {
        return this;
    }
    int c = count;
    if (c < 0) {
        c = va->count - start;
    }
    if (c <= 0) {
        return this;
    }
    ShaderProgram *program = &texture3Program;

    program->UseProgram();
    program->SetMatrix(frameData.MVP);
    
    int bid = 0;
    int vid = 1;
    if (!canvas->bindVertexArrayID(program->ProgramID)) {
        bid = glGetAttribLocation(program->ProgramID, "vertexPosition_modelspace" );
        vid = glGetAttribLocation(program->ProgramID, "vertexUV" );
    }
    va->SetBufferBytes(bid, vid);


    LOG_GL_ERRORV(glActiveTexture(GL_TEXTURE0)); //switch to texture image unit 0
    LOG_GL_ERRORV(glBindTexture(GL_TEXTURE_2D, t->_texId));
    LOG_GL_ERRORV(glUniform1i(glGetUniformLocation(program->ProgramID, "tex"), 0));

    
    program->SetRenderType(1);
    GLuint cid = glGetUniformLocation(program->ProgramID, "inColor");
    LOG_GL_ERRORV(glUniform4f(cid, ((float)color.red) / 255.0f,
                              ((float)color.green) / 255.0f,
                              ((float)color.blue) / 255.0f,
                              ((float)color.alpha) / 255.0f));

    if (enableCapabilities > 0) {
        LOG_GL_ERRORV(glEnable(enableCapabilities));
    }
    LOG_GL_ERRORV(glDrawArrays(GL_TRIANGLES, start, c));
    if (enableCapabilities > 0) {
        LOG_GL_ERRORV(glDisable(enableCapabilities));
    }
    LOG_GL_ERRORV(glBindTexture(GL_TEXTURE_2D, 0));
    LOG_GL_ERRORV(glActiveTexture(GL_TEXTURE0)); //switch to texture image unit 0

    program->UnbindBuffer(bid);
    program->UnbindBuffer(vid);
    return this;
}

class xlGLMesh : public xlMesh {
public:
    struct MeshVertexInput {
        float positionX;
        float positionY;
        float positionZ;
        float normalX;
        float normalY;
        float normalZ;
        float texcoordX;
        float texcoordY;
    };

    class xlOGLSubMesh {
    public:
        xlOGLSubMesh() {
            
        }
        ~xlOGLSubMesh() {
        }
        
        std::string name;
        int startIndex;
        int count;
        GLuint type;
        int material = 0;
    };
    
    
    
    xlGLMesh(const std::string &file, xlOGL3GraphicsContext *ctx) : xlMesh(ctx, file) {
    }
    virtual ~xlGLMesh() {
        for (auto a: subMeshes) {
            delete a;
        }
        if (vbuffer) {
            LOG_GL_ERRORV(glDeleteBuffers(1, &vbuffer));
        }
        if (tbuffer) {
            LOG_GL_ERRORV(glDeleteBuffers(1, &tbuffer));
        }
        if (nbuffer) {
            LOG_GL_ERRORV(glDeleteBuffers(1, &nbuffer));
        }
        if (wfIndexes) {
            LOG_GL_ERRORV(glDeleteBuffers(1, &wfIndexes));
        }
        if (lineIndexes) {
            LOG_GL_ERRORV(glDeleteBuffers(1, &lineIndexes));
        }
        if (indexBuffer) {
            LOG_GL_ERRORV(glDeleteBuffers(1, &indexBuffer));
        }
    }
    
    struct Index3 {
        int x, y, z;
    };
    struct CompareIndex3 {
        bool operator()(const Index3& lhs, const Index3& rhs) const {
            if (lhs.x != rhs.x) {
                return lhs.x < rhs.x;
            }
            if (lhs.y != rhs.y) {
                return lhs.y < rhs.y;
            }
            if (lhs.z != rhs.z) {
                return lhs.z < rhs.z;
            }
            return false;
        }
    };
    uint32_t getOrAddIndex(std::map<Index3, uint32_t, CompareIndex3> &indexMap, std::vector<MeshVertexInput> &input, const tinyobj::index_t &idx) {
        Index3 key;
        key.x = idx.vertex_index;
        key.y = idx.normal_index;
        key.z = idx.texcoord_index;
        int ret = indexMap[key];
        
        if (ret == 0) {
            ret = input.size();
            MeshVertexInput mvi;
            mvi.positionX = objects.GetAttrib().vertices[idx.vertex_index * 3];
            mvi.positionY = objects.GetAttrib().vertices[idx.vertex_index * 3 + 1];
            mvi.positionZ = objects.GetAttrib().vertices[idx.vertex_index * 3 + 2];
            if (idx.normal_index == -1) {
                mvi.normalX = 0;
                mvi.normalY = 0;
                mvi.normalZ = 0;
            } else {
                mvi.normalX = objects.GetAttrib().normals[idx.normal_index * 3];
                mvi.normalY = objects.GetAttrib().normals[idx.normal_index * 3 + 1];
                mvi.normalZ = objects.GetAttrib().normals[idx.normal_index * 3 + 2];
            }
            if (idx.texcoord_index == -1) {
                mvi.texcoordX = 0;
                mvi.texcoordY = 0;
            } else {
                mvi.texcoordX = objects.GetAttrib().texcoords[idx.texcoord_index * 2];
                mvi.texcoordY = objects.GetAttrib().texcoords[idx.texcoord_index * 2 + 1];
            }
            indexMap[key] = ret;
            input.emplace_back(mvi);
        }
        return ret;
    }
    void LoadBuffers() {
        std::map<Index3, uint32_t, CompareIndex3> indexMap;
        
        std::vector<MeshVertexInput> input;
        input.reserve(objects.GetAttrib().vertices.size());
        input.resize(1); // 0 position is ignored, indexMap[key] == 0 means not found yet
        
        indexes.resize(0);
        lines.resize(0);
        wireFrame.resize(0);
        indexes.reserve(objects.GetAttrib().vertices.size());
        lines.reserve(objects.GetAttrib().vertices.size() * 2);
        wireFrame.reserve(objects.GetAttrib().vertices.size() * 2);

        xlOGLSubMesh *lastMesh = nullptr;
        for (auto &s : objects.GetShapes()) {
            if (!s.mesh.indices.empty()) {
                xlOGLSubMesh *sm;
                if (lastMesh && (s.mesh.material_ids[0] == lastMesh->material)) {
                    sm = lastMesh;
                } else {
                    sm = new xlOGLSubMesh();
                    sm->name = s.name;
                    sm->type = GL_TRIANGLES;
                    sm->startIndex = indexes.size();
                    sm->material = s.mesh.material_ids[0];
                    subMeshes.push_back(sm);
                    lastMesh = sm;
                }
                for (size_t idx = 0; idx < s.mesh.material_ids.size(); idx++) {
                    if (sm->material != s.mesh.material_ids[idx]) {
                        sm->count = indexes.size() - sm->startIndex;

                        sm = new xlOGLSubMesh();
                        sm->name = s.name;
                        sm->type = GL_TRIANGLES;
                        sm->startIndex = indexes.size();
                        sm->material = s.mesh.material_ids[idx];
                        lastMesh = sm;
                        subMeshes.push_back(sm);
                    }
                    uint32_t vidx1 = getOrAddIndex(indexMap, input, s.mesh.indices[idx*3]);
                    uint32_t vidx2 = getOrAddIndex(indexMap, input, s.mesh.indices[idx*3 + 1]);
                    uint32_t vidx3 = getOrAddIndex(indexMap, input, s.mesh.indices[idx*3 + 2]);
                    indexes.push_back(vidx1);
                    indexes.push_back(vidx2);
                    indexes.push_back(vidx3);

                    wireFrame.push_back(vidx1);
                    wireFrame.push_back(vidx2);
                    wireFrame.push_back(vidx2);
                    wireFrame.push_back(vidx3);
                    wireFrame.push_back(vidx3);
                    wireFrame.push_back(vidx1);
                }
                sm->count = indexes.size() - sm->startIndex;
            }
            if (!s.lines.indices.empty()) {
                for (auto &idx : s.lines.indices) {
                    uint32_t vindex = getOrAddIndex(indexMap, input, idx);
                    lines.push_back(vindex);
                }
            }
            if (!s.points.indices.empty()) {
                xlOGLSubMesh *sm = new xlOGLSubMesh();
                sm->name = s.name;
                sm->type = GL_POINTS;
                sm->startIndex = indexes.size();
                for (auto &idx : s.points.indices) {
                    indexes.push_back(getOrAddIndex(indexMap, input, idx));
                }
                sm->count = indexes.size() - sm->startIndex;
                subMeshes.push_back(sm);
            }
        }
        
        std::vector<float> vertices;
        std::vector<float> tvertices;
        std::vector<float> normals;
        
        for (auto &sm : input) {
            vertices.push_back(sm.positionX);
            vertices.push_back(sm.positionY);
            vertices.push_back(sm.positionZ);
            
            tvertices.push_back(sm.texcoordX);
            tvertices.push_back(sm.texcoordY);
            
            normals.push_back(sm.normalX);
            normals.push_back(sm.normalY);
            normals.push_back(sm.normalZ);
        }
        LOG_GL_ERRORV(glGenBuffers(1, &vbuffer));
        LOG_GL_ERRORV(glGenBuffers(1, &tbuffer));
        LOG_GL_ERRORV(glGenBuffers(1, &nbuffer));
        LOG_GL_ERRORV(glGenBuffers(1, &wfIndexes));
        LOG_GL_ERRORV(glGenBuffers(1, &lineIndexes));
        LOG_GL_ERRORV(glGenBuffers(1, &indexBuffer));


        if (vertices.size() > 0)
        {
            LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, vbuffer));
            LOG_GL_ERRORV(glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW));

            LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, tbuffer));
            LOG_GL_ERRORV(glBufferData(GL_ARRAY_BUFFER, tvertices.size() * sizeof(float), &tvertices[0], GL_STATIC_DRAW));
        }

        if (normals.size() > 0)
        {
            LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, nbuffer));
            LOG_GL_ERRORV(glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), &normals[0], GL_STATIC_DRAW));
        }
        
        if (wireFrame.size() > 0)
        {
            LOG_GL_ERRORV(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, wfIndexes));
            LOG_GL_ERRORV(glBufferData(GL_ELEMENT_ARRAY_BUFFER, wireFrame.size() * sizeof(uint32_t), &wireFrame[0], GL_STATIC_DRAW));
            LOG_GL_ERRORV(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
        }

        if (lines.size() > 0)
        {
            LOG_GL_ERRORV(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lineIndexes));
            LOG_GL_ERRORV(glBufferData(GL_ELEMENT_ARRAY_BUFFER, lines.size() * sizeof(uint32_t), &lines[0], GL_STATIC_DRAW));
            LOG_GL_ERRORV(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
        }

        if (indexes.size() > 0)
        {
            LOG_GL_ERRORV(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer));
            LOG_GL_ERRORV(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexes.size() * sizeof(uint32_t), &indexes[0], GL_STATIC_DRAW));
            LOG_GL_ERRORV(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
        }
    }

    GLuint vbuffer = 0;
    GLuint tbuffer = 0;
    GLuint nbuffer = 0;

    GLuint wfIndexes = 0;
    GLuint lineIndexes = 0;
    GLuint indexBuffer = 0;

    std::vector<xlOGLSubMesh*> subMeshes;
    std::vector<uint32_t> indexes;
    std::vector<uint32_t> lines;
    std::vector<uint32_t> wireFrame;
};


xlMesh *xlOGL3GraphicsContext::loadMeshFromObjFile(const std::string &file) {
    return new xlGLMesh(file, this);
}
xlGraphicsContext* xlOGL3GraphicsContext::drawMeshSolids(xlMesh *mesh, int brightness, bool useViewMatrix) {
    drawMesh(mesh, brightness, useViewMatrix, false);
    return this;
}
xlGraphicsContext* xlOGL3GraphicsContext::drawMeshTransparents(xlMesh *mesh, int brightness) {
    drawMesh(mesh, brightness, false, true);
    return this;
}
void xlOGL3GraphicsContext::drawMesh(xlMesh *mesh, int brightness, bool useViewMatrix, bool transparents) {
    xlGLMesh *glm = (xlGLMesh*)mesh;
    if (glm->vbuffer == 0) {
        glm->LoadBuffers();
    }
    if (glm->indexes.size()) {
        LOG_GL_ERRORV(glDepthFunc(GL_LESS));
        float b = brightness;
        b /= 100.0f;
        
        glm::mat4 vm = useViewMatrix ? frameData.viewMatrix * frameData.modelMatrix : frameData.modelMatrix;

        meshTextureProgram.UseProgram();
        int bid = 0;
        int vnid = 1;
        int vid = 2;
        bool setVAS = false;
        if (!canvas->bindVertexArrayID(meshTextureProgram.ProgramID)) {
            bid = glGetAttribLocation(meshTextureProgram.ProgramID, "vertexPosition_modelspace" );
            vid = glGetAttribLocation(meshTextureProgram.ProgramID, "vertexUV" );
            vnid = glGetAttribLocation(meshTextureProgram.ProgramID, "vertexNormal_modelspace" );
            setVAS = true;
        }
        meshTextureProgram.SetMatrix(frameData.MVP);
        LOG_GL_ERRORV(glEnableVertexAttribArray(bid));
        LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, glm->vbuffer));
        LOG_GL_ERRORV(glVertexAttribPointer(bid, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 ));
        LOG_GL_ERRORV(glEnableVertexAttribArray(vid));
        LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, glm->tbuffer));
        LOG_GL_ERRORV(glVertexAttribPointer(vid, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 ));
        LOG_GL_ERRORV(glEnableVertexAttribArray(vnid));
        LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, glm->nbuffer));
        LOG_GL_ERRORV(glVertexAttribPointer(vnid, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 ));
        LOG_GL_ERRORV(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glm->indexBuffer));
        LOG_GL_ERRORV(GLuint textureBrightness = glGetUniformLocation(meshTextureProgram.ProgramID, "brightness"));
        LOG_GL_ERRORV(glUniform1f(textureBrightness, b));
        LOG_GL_ERRORV(GLuint textureID = glGetUniformLocation(meshTextureProgram.ProgramID, "tex"));
        LOG_GL_ERRORV(GLuint textureCid = glGetUniformLocation(meshTextureProgram.ProgramID, "inColor"));
        LOG_GL_ERRORV(GLuint MatrixID = glGetUniformLocation(meshTextureProgram.ProgramID, "NM"));
        LOG_GL_ERRORV(glUniformMatrix4fv(MatrixID, 1, GL_FALSE, glm::value_ptr(vm)));
        LOG_GL_ERRORV(glUniform4f(textureCid, 0, 0, 0, 1.0f));
        
        meshSolidProgram.UseProgram();
        int bids = 0;
        int vnids = 1;
        if (!canvas->bindVertexArrayID(meshSolidProgram.ProgramID)) {
            bids = glGetAttribLocation(meshSolidProgram.ProgramID, "vertexPosition_modelspace" );
            vnids = glGetAttribLocation(meshSolidProgram.ProgramID, "vertexNormal_modelspace" );
            setVAS = true;
        }
        meshSolidProgram.SetMatrix(frameData.MVP);
        LOG_GL_ERRORV(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glm->indexBuffer));
        LOG_GL_ERRORV(GLuint solidCid = glGetUniformLocation(meshSolidProgram.ProgramID, "inColor"));
        LOG_GL_ERRORV(GLuint MatrixID2 = glGetUniformLocation(meshSolidProgram.ProgramID, "NM"));
        LOG_GL_ERRORV(glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, glm::value_ptr(vm)));

        LOG_GL_ERRORV(glEnableVertexAttribArray(bids));
        LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, glm->vbuffer));
        LOG_GL_ERRORV(glVertexAttribPointer(bids, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 ));
        
        LOG_GL_ERRORV(glEnableVertexAttribArray(vnids));
        LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, glm->nbuffer));
        LOG_GL_ERRORV(glVertexAttribPointer(vnids, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 ));

        LOG_GL_ERRORV(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glm->indexBuffer));


        xlTexture *lastTexture = nullptr;
        bool lastIsSolid = true;
        for (auto sm : glm->subMeshes) {
            int mid = sm->material;
            
            bool output = mid < 0 || glm->GetMaterial(mid).color.alpha == 255;
            if (transparents) {
                output = mid > 0 && glm->GetMaterial(mid).color.alpha != 255;
            }
            
            if (output) {
                if (mid < 0 || !glm->GetMaterial(mid).texture || glm->GetMaterial(mid).forceColor) {
                    if (!lastIsSolid) {
                        lastIsSolid = true;
                        meshSolidProgram.UseProgram();
                        if (!canvas->bindVertexArrayID(meshSolidProgram.ProgramID)) {
                            LOG_GL_ERRORV(glEnableVertexAttribArray(bids));
                            LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, glm->vbuffer));
                            LOG_GL_ERRORV(glVertexAttribPointer(bids, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 ));
                            
                            LOG_GL_ERRORV(glEnableVertexAttribArray(vnids));
                            LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, glm->nbuffer));
                            LOG_GL_ERRORV(glVertexAttribPointer(vnids, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 ));
                        }
                    }
                    xlColor color = mid < 0 ? xlWHITE : glm->GetMaterial(mid).color;
                    if (sm->type == GL_LINES) {
                        color = xlBLACK;
                    }
                    LOG_GL_ERRORV(glUniform4f(solidCid,
                                ((float)color.Red())/255.0 * b,
                                ((float)color.Green())/255.0 * b,
                                ((float)color.Blue())/255.0 * b,
                                ((float)color.Alpha())/255.0
                                ));
                } else {
                    xlGLTexture *t = (xlGLTexture*)glm->GetMaterial(mid).texture;
                    if (lastIsSolid) {
                        lastIsSolid = false;
                        meshTextureProgram.UseProgram();
                        if (!canvas->bindVertexArrayID(meshTextureProgram.ProgramID)) {
                            LOG_GL_ERRORV(glEnableVertexAttribArray(bid));
                            LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, glm->vbuffer));
                            LOG_GL_ERRORV(glVertexAttribPointer(bid, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 ));
                            LOG_GL_ERRORV(glEnableVertexAttribArray(vid));
                            LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, glm->tbuffer));
                            LOG_GL_ERRORV(glVertexAttribPointer(vid, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 ));
                            LOG_GL_ERRORV(glEnableVertexAttribArray(vnid));
                            LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, glm->nbuffer));
                            LOG_GL_ERRORV(glVertexAttribPointer(vnid, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 ));
                            LOG_GL_ERRORV(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glm->indexBuffer));
                        }

                        LOG_GL_ERRORV(glActiveTexture(GL_TEXTURE0)); //switch to texture image unit 0
                        LOG_GL_ERRORV(glBindTexture(GL_TEXTURE_2D, t->_texId));
                        LOG_GL_ERRORV(glUniform1i(textureID, 0));
                        LOG_GL_ERRORV(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
                        LOG_GL_ERRORV(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
                        lastTexture = t;
                    } else if (t != lastTexture) {
                        LOG_GL_ERRORV(glActiveTexture(GL_TEXTURE0)); //switch to texture image unit 0
                        LOG_GL_ERRORV(glBindTexture(GL_TEXTURE_2D, t->_texId));
                        LOG_GL_ERRORV(glUniform1i(textureID, 0));
                        LOG_GL_ERRORV(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
                        LOG_GL_ERRORV(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
                        lastTexture = t;
                    }
                }
                // Draw the submesh.
                LOG_GL_ERRORV(glDrawElements(sm->type, sm->count, GL_UNSIGNED_INT, (void*)(sm->startIndex * sizeof(uint32_t))));
            }
        }
        if (!lastIsSolid) {
            //lastIsSolid = true;
            meshSolidProgram.UseProgram();
        }
        if (!transparents && glm->lines.size()) {
            LOG_GL_ERRORV(glEnable(GL_LINE_SMOOTH));
            LOG_GL_ERRORV(glUniform4f(solidCid, 0.0, 0, 0.0, 1.0));
            LOG_GL_ERRORV(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glm->lineIndexes));
            LOG_GL_ERRORV(glDrawElements(GL_LINES, glm->lines.size(), GL_UNSIGNED_INT, 0));
            LOG_GL_ERRORV(glDisable(GL_LINE_SMOOTH));
        }
        
        LOG_GL_ERRORV(glDisableVertexAttribArray(0));
        LOG_GL_ERRORV(glDisableVertexAttribArray(1));
        LOG_GL_ERRORV(glDisableVertexAttribArray(2));
        LOG_GL_ERRORV(glDisableVertexAttribArray(3));
        LOG_GL_ERRORV(glDisableVertexAttribArray(4));
        LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, 0));
        LOG_GL_ERRORV(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

        LOG_GL_ERRORV(glDepthFunc(GL_LEQUAL));
    }
}

xlGraphicsContext* xlOGL3GraphicsContext::drawMeshWireframe(xlMesh *mesh, int brightness) {
    xlGLMesh *glm = (xlGLMesh*)mesh;
    if (glm->vbuffer == 0) {
        glm->LoadBuffers();
    }
    if (glm->wireFrame.size()) {
        LOG_GL_ERRORV(glDepthFunc(GL_LESS));
        
        ShaderProgram *program = &singleColor3Program;
        program->UseProgram();
        program->SetMatrix(frameData.MVP);
        
        LOG_GL_ERRORV(GLuint cid = glGetUniformLocation(program->ProgramID, "inColor"));
        float b = brightness;
        b /= 100.0f;
        LOG_GL_ERRORV(glUniform4f(cid, 0.0f, 1.0f * b, 0.0f, 255.0f));
        
        LOG_GL_ERRORV(glEnableVertexAttribArray(0));
        LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, glm->vbuffer));
        LOG_GL_ERRORV(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 ));
        int caps = enableCapabilities;
        if (isBlending) {
            caps = GL_LINE_SMOOTH;
        }
        LOG_GL_ERRORV(glEnable(caps));
        program->SetRenderType(0);
        LOG_GL_ERRORV(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glm->wfIndexes));
        LOG_GL_ERRORV(glDrawElements(GL_LINES, glm->wireFrame.size(), GL_UNSIGNED_INT, 0));
        
        LOG_GL_ERRORV(glDisableVertexAttribArray(0));
        LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, 0));
        LOG_GL_ERRORV(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

        LOG_GL_ERRORV(glDepthFunc(GL_LEQUAL));
        return this;
    }
    return this;
}

xlGraphicsContext* xlOGL3GraphicsContext::enableBlending(bool e) {
    if (e) {
        LOG_GL_ERRORV(glEnable(GL_BLEND));
        LOG_GL_ERRORV(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        LOG_GL_ERRORV(glHint(GL_LINE_SMOOTH_HINT, GL_NICEST));
    } else {
        LOG_GL_ERRORV(glDisable(GL_BLEND));
    }
    isBlending = e;
    return this;
}

// Setup the Viewport
xlGraphicsContext* xlOGL3GraphicsContext::SetViewport(int topleft_x, int topleft_y, int bottomright_x, int bottomright_y, bool is3D) {
    frameData.modelMatrix = glm::mat4(1.0);
    frameData.viewMatrix = glm::mat4(1.0);
    if (is3D) {
        float x, y, x2, y2;
        x = topleft_x;
        y = bottomright_y;
        x2 = bottomright_x;
        y2 = topleft_y;

        int depth = canvas->GetZDepth();
        
        double sf = canvas->GetContentScaleFactor();
        x = sf * x;
        y = sf * y;
        x2 = sf * x2;
        y2 = sf * y2;
        LOG_GL_ERRORV(glViewport(x, y, x2 - x, y2 - y));
        LOG_GL_ERRORV(glScissor(0, 0, x2 - x, y2 - y));
        
        float min = 1.0f;
        if (depth < 24) {
            min = 50.0f;
        }
        glm::mat4 m = glm::perspective(glm::radians(45.0f), (float) (bottomright_x-topleft_x) / (float)(topleft_y-bottomright_y), min, 200000.0f); // bumped from 20,000 to 200,000 to allow bigger models without clipping
        frameData.MVP = m;
        frameData.perspectiveMatrix = frameData.MVP;

        LOG_GL_ERRORV(glClearColor(0,0,0,0));   // background color
        LOG_GL_ERRORV(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
    } else {
        int x, y, x2, y2;
        x = topleft_x;
        y = std::min(bottomright_y, topleft_y);
        x2 = bottomright_x;
        y2 = std::max(bottomright_y,topleft_y);

        double sf = canvas->GetContentScaleFactor();
        x = sf * x;
        y = sf * y;
        x2 = sf * x2;
        y2 = sf * y2;

        int w = std::max(x, x2) - std::min(x, x2);
        int h = std::max(y, y2) - std::min(y, y2);
        LOG_GL_ERRORV(glViewport(x,y,w,h));
        glm::mat4 m = glm::ortho((float)topleft_x, (float)bottomright_x, (float)bottomright_y, (float)topleft_y);
        frameData.MVP = m;
        frameData.perspectiveMatrix = frameData.MVP;

        if (canvas->RequiresDepthBuffer()) {
            LOG_GL_ERRORV(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
        }
    }
    frameDataChanged = true;
    return this;
}

//manipulating the matrices
xlGraphicsContext* xlOGL3GraphicsContext::PushMatrix() {
    matrixStack.push(frameData.MVP);
    matrixStack.push(frameData.viewMatrix);
    matrixStack.push(frameData.modelMatrix);
    return this;
}
xlGraphicsContext* xlOGL3GraphicsContext::PopMatrix() {
    if (!matrixStack.empty()) {
        frameData.modelMatrix = matrixStack.top();
        matrixStack.pop();
        frameData.viewMatrix = matrixStack.top();
        matrixStack.pop();
        frameData.MVP = matrixStack.top();
        matrixStack.pop();
        frameDataChanged = true;
    }
    return this;
}
xlGraphicsContext* xlOGL3GraphicsContext::Translate(float x, float y, float z) {
    frameData.MVP = glm::translate(frameData.MVP, glm::vec3(x, y, z));
    frameData.modelMatrix = glm::translate(frameData.modelMatrix, glm::vec3(x, y, z));
    frameDataChanged = true;
    return this;
}
xlGraphicsContext* xlOGL3GraphicsContext::Rotate(float angle, float x, float y, float z) {
    angle = angle * 3.14159f/180.0f;
    frameData.MVP = glm::rotate(frameData.MVP, angle, glm::vec3(x, y, z));
    frameData.modelMatrix = glm::rotate(frameData.modelMatrix, angle, glm::vec3(x, y, z));
    frameDataChanged = true;
    return this;
}
xlGraphicsContext* xlOGL3GraphicsContext::Scale(float w, float h, float z) {
    frameData.MVP = glm::scale(frameData.MVP, glm::vec3(w, h, z));
    frameData.modelMatrix = glm::scale(frameData.modelMatrix, glm::vec3(w, h, z));
    frameDataChanged = true;
    return this;
}
xlGraphicsContext* xlOGL3GraphicsContext::ScaleViewMatrix(float w, float h, float z) {
    frameData.MVP = glm::scale(frameData.MVP, glm::vec3(w, h, z));
    frameData.viewMatrix = glm::scale(frameData.viewMatrix, glm::vec3(w, h, z));
    frameDataChanged = true;
    return this;
}
xlGraphicsContext* xlOGL3GraphicsContext::TranslateViewMatrix(float x, float y, float z) {
    frameData.MVP = glm::translate(frameData.MVP, glm::vec3(x, y, z));
    frameData.viewMatrix = glm::translate(frameData.viewMatrix, glm::vec3(x, y, z));
    frameDataChanged = true;
    return this;
}

xlGraphicsContext* xlOGL3GraphicsContext::SetCamera(const glm::mat4 &m) {
    frameData.MVP = frameData.MVP * m;
    frameData.viewMatrix = frameData.viewMatrix * m;
    frameDataChanged = true;
    return this;
}
xlGraphicsContext* xlOGL3GraphicsContext::SetModelMatrix(const glm::mat4 &m) {
    frameData.MVP = frameData.MVP * m;
    frameData.modelMatrix = m;
    frameDataChanged = true;
    return this;
}
xlGraphicsContext* xlOGL3GraphicsContext::ApplyMatrix(const glm::mat4 &m) {
    frameData.MVP = frameData.MVP * m;
    frameData.modelMatrix = frameData.modelMatrix * m;
    frameDataChanged = true;
    return this;
}
