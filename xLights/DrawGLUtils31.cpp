#include <atomic>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifdef __WXMAC__
#include "OpenGL/gl3.h"
#define __gl_h_
#else

#include "wx/wx.h"

#include <GL/gl.h>
    #ifdef _MSC_VER
        #include "GL/glext.h"
        //#include "GL/glut.h"
    #else
        #include <GL/glext.h>
    #endif
#endif

#include "DrawGLUtils.h"
#include <stack>

#include <log4cpp/Category.hh>

#ifndef __WXMAC__

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
PFNGLDETACHSHADERPROC glDetachShader;
PFNGLDELETESHADERPROC glDeleteShader;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLDELETEPROGRAMPROC glDeleteProgram;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
PFNGLUNIFORM1IPROC glUniform1i;
PFNGLUNIFORM4FPROC glUniform4f;
PFNGLUNIFORM1FPROC glUniform1f;
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
    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)wglGetProcAddress("glGetProgramInfoLog");
    glDetachShader = (PFNGLDETACHSHADERPROC)wglGetProcAddress("glDetachShader");
    glDeleteShader = (PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader");
    glLinkProgram = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
    glDeleteProgram = (PFNGLDELETEPROGRAMPROC)wglGetProcAddress("glDeleteProgram");
    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation");
    glUniform1i = (PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i");
    glUniform4f = (PFNGLUNIFORM4FPROC)wglGetProcAddress("glUniform4f");
    glUniform1f = (PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f");
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
    class BufferInfo {
    public:
        size_t currentSize = 0;
        size_t currentPos = 0;
        bool valid = true;
    };

public:
    ShaderProgram() : ProgramID(0), buffers(nullptr), numBuffers(0), bufferInfo(nullptr), buffersValid(false) {}

    void Cleanup(GLuint &pid) {
        if (ProgramID != 0) {
            LOG_GL_ERRORV(glUseProgram(0));
            LOG_GL_ERRORV(glDeleteBuffers(numBuffers, buffers));
            delete [] buffers;
            LOG_GL_ERRORV(glDeleteVertexArrays(1, &VertexArrayID));
            delete [] bufferInfo;
            numBuffers = 0;
        }
        if (pid) {
            LOG_GL_ERRORV(glDeleteProgram(pid));
            pid = 0;
            ProgramID = 0;
        }
    }

    void Reset() {
        for (int x = 0; x < numBuffers; x++) {
            if (bufferInfo[x].currentPos > bufferInfo[x].currentSize || !buffersValid) {
                bufferInfo[x].currentSize = std::max(bufferInfo[x].currentPos, bufferInfo[x].currentSize);
                LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, buffers[x]));
                LOG_GL_ERRORV(glBufferData(GL_ARRAY_BUFFER, bufferInfo[x].currentSize, nullptr, GL_STREAM_DRAW));
            }
            bufferInfo[x].currentPos = 0;
        }
        buffersValid = true;
    }
    
    GLuint GetBufferID(int idx) const {
        return buffers[idx];
    }

    void UseProgram() const {
        LOG_GL_ERRORV(glUseProgram(ProgramID));
        LOG_GL_ERRORV(glBindVertexArray(VertexArrayID));
    }

    void SetMatrix(glm::mat4 &m) const {
        LOG_GL_ERRORV(glUniformMatrix4fv(MatrixID, 1, GL_FALSE, glm::value_ptr(m)));
    }

    void SetRenderType(int i) {
        if (lastRenderType != i) {
            LOG_GL_ERRORV(glUniform1i(RenderTypeID, i));
            lastRenderType = i;
        }
    }

    int BindBuffer(int idx, void *data, int sz) {
        return BindBuffer(idx, buffers[idx], data, sz);
    }

    void ReBindBuffer(int idx) {
        LOG_GL_ERRORV(glEnableVertexAttribArray(idx));
        LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, buffers[idx]));
    }

    int BindBuffer(int idx, GLuint bufId, void *data, int sz) {
        LOG_GL_ERRORV(glEnableVertexAttribArray(idx));
        LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, bufId));
        int i = bufferInfo[idx].currentPos;
        if (!buffersValid || bufId != buffers[idx] || ((bufferInfo[idx].currentPos + sz) > bufferInfo[idx].currentSize)) {
            LOG_GL_ERRORV(glBufferData(GL_ARRAY_BUFFER, sz, data, GL_STATIC_DRAW));
            buffersValid = false;
            i = 0;
        } else {
            void * tdata = nullptr;
            tdata = glMapBufferRange(GL_ARRAY_BUFFER, bufferInfo[idx].currentPos, sz,
                                     GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);
            LOG_GL_ERROR();
            if (tdata != nullptr) {
                memcpy(tdata, data, sz);
                LOG_GL_ERRORV(glFlushMappedBufferRange(GL_ARRAY_BUFFER, 0, sz));
                LOG_GL_ERRORV(glUnmapBuffer(GL_ARRAY_BUFFER));
            } else {
                LOG_GL_ERRORV(glBufferSubData(GL_ARRAY_BUFFER, bufferInfo[idx].currentPos, sz, data));
            }
        }
        bufferInfo[idx].currentPos += sz;
        return i;
    }

    void UnbindBuffer(int idx) const {
        LOG_GL_ERRORV(glDisableVertexAttribArray(idx));
        LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, 0));
    }

    void Init(GLuint &pid, const char * vs, const char * fs, int numBuf) {
        if (pid == 0) {
            GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
            GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
            CompileShader(vs, VertexShaderID);
            CompileShader(fs, FragmentShaderID);
            pid = CreateProgram(VertexShaderID, FragmentShaderID);
            glDeleteShader(VertexShaderID);
            glDeleteShader(FragmentShaderID);
        }
        ProgramID = pid;

        LOG_GL_ERRORV(glUseProgram(ProgramID));
        LOG_GL_ERRORV(MatrixID = glGetUniformLocation(ProgramID, "MVP"));
        LOG_GL_ERRORV(PointSmoothMinID = glGetUniformLocation(ProgramID, "PointSmoothMin"));
        LOG_GL_ERRORV(PointSmoothMaxID = glGetUniformLocation(ProgramID, "PointSmoothMax"));
        LOG_GL_ERRORV(RenderTypeID = glGetUniformLocation(ProgramID, "RenderType"));


        numBuffers = numBuf;
        buffers = new GLuint[numBuffers];
        bufferInfo = new BufferInfo[numBuffers];
        LOG_GL_ERRORV(glGenVertexArrays(1, &VertexArrayID));
        LOG_GL_ERRORV(glGenBuffers(numBuffers, buffers));
        LOG_GL_ERRORV(glBindVertexArray(VertexArrayID));
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
        GLuint ProgramID = glCreateProgram();
        LOG_GL_ERRORV(glAttachShader(ProgramID, vs));
        LOG_GL_ERRORV(glAttachShader(ProgramID, fs));
        LOG_GL_ERRORV(glLinkProgram(ProgramID));


        GLint Result = GL_FALSE;
        int InfoLogLength;

        LOG_GL_ERRORV(glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result));
        LOG_GL_ERRORV(glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength));
        if (!Result &&  InfoLogLength > 0 ){
            std::vector<char> ProgramErrorMessage(InfoLogLength+1);
            glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
            wxString l = &ProgramErrorMessage[0];
            l.Trim();
            if (l.length() > 0) {
                printf("Program Log: %s\n", &ProgramErrorMessage[0]);
                static log4cpp::Category &logger_opengl = log4cpp::Category::getInstance(std::string("log_opengl"));
                logger_opengl.error(std::string(&ProgramErrorMessage[0]));
            }
        }
        LOG_GL_ERRORV(glDetachShader(ProgramID, vs));
        LOG_GL_ERRORV(glDetachShader(ProgramID, fs));
        return ProgramID;
    }

    static void CompileShader(const char *sourcePointer, GLuint shaderID) {
        LOG_GL_ERRORV(glShaderSource(shaderID, 1, &sourcePointer , NULL));
        LOG_GL_ERRORV(glCompileShader(shaderID));

        GLint Result = GL_FALSE;
        int InfoLogLength;

        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &Result);
        glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if (!Result && InfoLogLength > 0 ) {
            std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
            glGetShaderInfoLog(shaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
            wxString l = &VertexShaderErrorMessage[0];
            l.Trim();
            if (l.length() > 0) {
                printf("Shader Log: %s\n", &VertexShaderErrorMessage[0]);
                static log4cpp::Category &logger_opengl = log4cpp::Category::getInstance(std::string("log_opengl"));
                logger_opengl.error(std::string(&VertexShaderErrorMessage[0]));
            }
        }
    }

    GLuint ProgramID;
    GLuint VertexArrayID;

    GLuint MatrixID;
    GLuint PointSmoothMinID;
    GLuint PointSmoothMaxID;
    GLuint RenderTypeID;
    int lastRenderType = -1;

    GLuint *buffers;
    BufferInfo *bufferInfo;
    size_t numBuffers;
    bool buffersValid;

};

class GL3Mesh : public DrawGLUtils::xl3DMesh {
    static const int NUM_BUFFERS = 6;

    public:
    GL3Mesh() : DrawGLUtils::xl3DMesh() {
        for (int x = 0; x < NUM_BUFFERS; x++) {
            buffers[x] = 0;
        }
    }
    virtual ~GL3Mesh() {
        if (buffers[0]) {
            LOG_GL_ERRORV(glDeleteBuffers(NUM_BUFFERS, buffers));
        }
    }
    
    void Draw(bool wf, float brightness, glm::mat4 &curMatrix,
              ShaderProgram &singleColorProgram, ShaderProgram &meshProgram,
              DrawGLUtils::xlGLCacheInfo::DrawType dt) {
        if (buffers[0] == 0) {
            LOG_GL_ERRORV(glGenBuffers(NUM_BUFFERS, buffers));

            LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, buffers[0]));
            LOG_GL_ERRORV(glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertices.size(), &vertices[0], GL_STATIC_DRAW));

            LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, buffers[1]));
            LOG_GL_ERRORV(glBufferData(GL_ARRAY_BUFFER, sizeof(GLubyte) * colors.size(), &colors[0], GL_STATIC_DRAW));

            LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, buffers[2]));
            LOG_GL_ERRORV(glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * texCoords.size(), &texCoords[0], GL_STATIC_DRAW));

            LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, buffers[3]));
            LOG_GL_ERRORV(glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * normals.size(), &normals[0], GL_STATIC_DRAW));

            LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, buffers[4]));
            LOG_GL_ERRORV(glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * wireframe.size(), &wireframe[0], GL_STATIC_DRAW));

            LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, buffers[5]));
            if (lines.size() > 0)
            {
                LOG_GL_ERRORV(glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * lines.size(), &lines[0], GL_STATIC_DRAW));
            }
            else
            {
                LOG_GL_ERRORV(glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW));
            }

            calcProgram();
        }
        glm::mat4 mat = curMatrix * matrix;
        bool ttype = dt == DrawGLUtils::xlGLCacheInfo::DrawType::TRANSPARENTS;

        if (wf) {
            if (!ttype) {
                singleColorProgram.UseProgram();
                singleColorProgram.SetRenderType(0);
                singleColorProgram.SetMatrix(mat);
                GLuint cid = glGetUniformLocation(singleColorProgram.ProgramID, "inColor");
                LOG_GL_ERRORV(glUniform4f(cid, 0.0f, 1.0f, 0.0f, 1.0f));
                LOG_GL_ERRORV(glBindVertexArray(meshProgram.VertexArrayID));
                LOG_GL_ERRORV(glEnableVertexAttribArray(0));
                LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, buffers[4]));
                LOG_GL_ERRORV(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0));
                glEnable(GL_LINE_SMOOTH);
                LOG_GL_ERRORV(glDrawArrays(GL_LINES, 0, wireframe.size() / 2));
                glDisable(GL_LINE_SMOOTH);
                LOG_GL_ERRORV(glDisableVertexAttribArray(0));
                singleColorProgram.UseProgram();
                singleColorProgram.SetMatrix(curMatrix);
                singleColorProgram.ReBindBuffer(0);
            }
        } else {
            meshProgram.UseProgram();
            meshProgram.SetRenderType(0);
            meshProgram.SetMatrix(mat);
            
            GLuint brightnessId = glGetUniformLocation(meshProgram.ProgramID, "brightness");
            float br = brightness / 100.0f;
            LOG_GL_ERRORV(glUniform1f(brightnessId, br));

            LOG_GL_ERRORV(glBindVertexArray(meshProgram.VertexArrayID));
            LOG_GL_ERRORV(glEnableVertexAttribArray(0));
            LOG_GL_ERRORV(glEnableVertexAttribArray(1));
            LOG_GL_ERRORV(glEnableVertexAttribArray(2));
            LOG_GL_ERRORV(glEnableVertexAttribArray(3));
            LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, buffers[0]));
            LOG_GL_ERRORV(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0));
            LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, buffers[1]));
            LOG_GL_ERRORV(glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (void*)0));
            LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, buffers[2]));
            LOG_GL_ERRORV(glVertexAttribPointer(2, 2, GL_FLOAT, GL_TRUE, 0, (void*)0));
            LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, buffers[3]));
            LOG_GL_ERRORV(glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*)0));
            
            for (auto & pt : programTypes) {
                if (pt.transparent == ttype) {
                    if (pt.image == -1) {
                        meshProgram.SetRenderType(1);
                    } else {
                        meshProgram.SetRenderType(0);
                        LOG_GL_ERRORV(glActiveTexture(GL_TEXTURE0));
                        LOG_GL_ERRORV(glBindTexture(GL_TEXTURE_2D, pt.image));
                        LOG_GL_ERRORV(glUniform1i(glGetUniformLocation(meshProgram.ProgramID, "tex"), 0));
                        LOG_GL_ERRORV(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
                        LOG_GL_ERRORV(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
                    }
                    LOG_GL_ERRORV(glDrawArrays(GL_TRIANGLES, pt.startIdx, pt.count));
                    if (pt.image != -1) {
                        LOG_GL_ERRORV(glBindTexture(GL_TEXTURE_2D, 0));
                    }
                }
            }
            LOG_GL_ERRORV(glDisableVertexAttribArray(0));
            LOG_GL_ERRORV(glDisableVertexAttribArray(1));
            LOG_GL_ERRORV(glDisableVertexAttribArray(2));
            LOG_GL_ERRORV(glDisableVertexAttribArray(3));
            if (!lines.empty() && !ttype) {
                singleColorProgram.UseProgram();
                singleColorProgram.SetRenderType(0);
                singleColorProgram.SetMatrix(mat);
                GLuint cid = glGetUniformLocation(singleColorProgram.ProgramID, "inColor");
                LOG_GL_ERRORV(glUniform4f(cid, 0.0f, 0.0f, 0.0f, 1.0f));
                LOG_GL_ERRORV(glBindVertexArray(meshProgram.VertexArrayID));
                LOG_GL_ERRORV(glEnableVertexAttribArray(0));
                LOG_GL_ERRORV(glBindBuffer(GL_ARRAY_BUFFER, buffers[5]));
                LOG_GL_ERRORV(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0));

                glEnable(GL_LINE_SMOOTH);
                LOG_GL_ERRORV(glDrawArrays(GL_LINES, 0, lines.size() / 2));
                glDisable(GL_LINE_SMOOTH);
                LOG_GL_ERRORV(glDisableVertexAttribArray(0));
                singleColorProgram.UseProgram();
                singleColorProgram.SetMatrix(curMatrix);
                singleColorProgram.ReBindBuffer(0);
            }
        }
        LOG_GL_ERRORV(glBindVertexArray(0));
    }
    
    private:
    GLuint buffers[NUM_BUFFERS];
};


class OpenGL33Cache : public DrawGLUtils::xlGLCacheInfo {

    ShaderProgram textureProgram;
    ShaderProgram texture3Program;
    ShaderProgram meshProgram;
    ShaderProgram singleColorProgram;
    ShaderProgram singleColor3Program;
    ShaderProgram normalProgram;
    ShaderProgram normal3Program;
    ShaderProgram vbNormalProgram;
    
    static std::atomic_int cacheCount;
    static GLuint textureProgramId;
    static GLuint texture3ProgramId;
    static GLuint meshProgramId;
    static GLuint singleColorProgramId;
    static GLuint singleColor3ProgramId;
    static GLuint normalProgramId;
    static GLuint normal3ProgramId;
    static GLuint vbNormalProgramId;

    void Load33Shaders(bool UsesVertexTextureAccumulator,
                       bool UsesVertexColorAccumulator,
                       bool UsesVertexAccumulator,
                       bool UsesAddVertex,
					   bool UsesVertex3Accumulator,
                       bool UsesVertex3TextureAccumulator,
					   bool UsesVertex3ColorAccumulator) {
        cacheCount++;
        if (UsesVertexTextureAccumulator) {
            textureProgram.Init(textureProgramId,
                                "#version 330 core\n"
                                "layout(location = 0) in vec2 vertexPosition_modelspace;\n"
                                "layout(location = 2) in vec2 vertexUV;\n"
                                "out vec4 fragmentColor;\n"
                                "out vec2 UV;\n"
                                "uniform mat4 MVP;\n"
                                "uniform vec4 inColor;\n"
                                "void main(){\n"
                                "    gl_Position = MVP * vec4(vertexPosition_modelspace,0,1);\n"
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
                                "}\n", 3);
        }
        if (UsesVertex3TextureAccumulator) {
            texture3Program.Init(texture3ProgramId,
                "#version 330 core\n"
                "layout(location = 0) in vec3 vertexPosition_modelspace;\n"
                "layout(location = 2) in vec2 vertexUV;\n"
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
                "uniform int RenderType;\n"
                "void main(){\n"
                "    vec4 c = texture(tex, UV);\n"
                "    color = vec4(c.r*fragmentColor.r, c.g*fragmentColor.g, c.b*fragmentColor.b, c.a*fragmentColor.a);\n"
                "}\n", 3);
            
            meshProgram.Init(meshProgramId,
                "#version 330 core\n"
                "layout(location = 0) in vec3 vertexPosition_modelspace;\n"
                "layout(location = 1) in vec4 vertexColor;\n"
                "layout(location = 2) in vec2 vertexUV;\n"
                "layout(location = 4) in vec3 vertexNormal_modelspace;\n"
                "out vec4 fragmentColor;\n"
                "out vec2 UV;\n"
                "uniform mat4 MVP;\n"
                "uniform mat4 V;\n"
                "uniform mat4 M;\n"
                "uniform mat4 S;\n"
                "void main(){\n"
                "    gl_Position = MVP * vec4(vertexPosition_modelspace,1);\n"
                "    fragmentColor = vertexColor;\n"
                "    UV = vertexUV;\n"
                "}\n",

                "#version 330 core\n"
                "in vec4 fragmentColor;\n"
                "in vec2 UV;\n"
                "out vec4 color;\n"
                "uniform float brightness;\n"
                "uniform sampler2D tex;\n"
                "uniform int RenderType;\n"
                "void main(){\n"
                "    vec4 c;\n"
                "    if (RenderType == 0) {\n"
                "        c = texture(tex, UV);\n"
                "    } else {\n"
                "        c = fragmentColor;\n"
                "    }\n"
                "    color = vec4(c.r*brightness, c.g*brightness, c.b*brightness, c.a);\n"
                "}\n", 0);
            
        }
        if (UsesVertexAccumulator) {
			singleColorProgram.Init(singleColorProgramId,
				"#version 330 core\n"
				"layout(location = 0) in vec2 vertexPosition_modelspace;\n"
				"out vec4 fragmentColor;\n"
				"uniform mat4 MVP;\n"
				"uniform vec4 inColor;\n"
				"void main(){\n"
				"    gl_Position = MVP * vec4(vertexPosition_modelspace,0,1);"
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
				"}\n", 1);
		}
		if (UsesVertex3Accumulator) {
			singleColor3Program.Init(singleColor3ProgramId,
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
				"}\n", 1);
		}
		const char *npVS = "#version 330 core\n"
                            "layout(location = 0) in vec2 vertexPosition_modelspace;\n"
                            "layout(location = 1) in vec4 vertexColor;\n"
                            "out vec4 fragmentColor;\n"
                            "uniform int RenderType;\n"
                            "uniform mat4 MVP;\n"
                            "uniform vec4 inColor;\n"
                            "void main(){\n"
                            "    gl_Position = MVP * vec4(vertexPosition_modelspace,0,1);\n"
                            "    if (RenderType == -2) {\n"
                            "        fragmentColor = inColor;\n"
                            "    } else if (RenderType == -1) {\n"
                            "        fragmentColor = inColor;\n"
                            "    } else {\n"
                            "        fragmentColor = vertexColor;\n"
                            "    }\n"
                            "}\n";
        const char *npFS = "#version 330 core\n"
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
                            "}\n";
		const char *np3VS = "#version 330 core\n"
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
							"}\n";
		const char *np3FS = "#version 330 core\n"
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
							"}\n";
		if (UsesVertexColorAccumulator) {
            normalProgram.Init(normalProgramId, npVS, npFS, 2);
        }
        if (UsesVertex3ColorAccumulator) {
            normal3Program.Init(normal3ProgramId, np3VS, np3FS, 2);
        }
        if (UsesAddVertex) {
            vbNormalProgram.Init(vbNormalProgramId, npVS, npFS, 2);
        }
    }
    void Release33Shaders() {
        --cacheCount;
        bool del = cacheCount == 0;
        GLuint zero = 0;
        singleColorProgram.Cleanup(del ? singleColorProgramId : zero);
        singleColor3Program.Cleanup(del ? singleColor3ProgramId : zero);
        textureProgram.Cleanup(del ? textureProgramId : zero);
        texture3Program.Cleanup(del ? texture3ProgramId : zero);
        meshProgram.Cleanup(del ? meshProgramId : zero);
        normalProgram.Cleanup(del ? normalProgramId : zero);
        normal3Program.Cleanup(del ? normal3ProgramId : zero);
        vbNormalProgram.Cleanup(del ? vbNormalProgramId : zero);
    }


public:
    OpenGL33Cache(bool UsesVertexTextureAccumulator,
                  bool UsesVertexColorAccumulator,
                  bool UsesVertexAccumulator,
                  bool UsesAddVertex,
                  bool UsesVertex3Accumulator,
                  bool UsesVertex3TextureAccumulator,
		          bool UsesVertex3ColorAccumulator) : matrix(nullptr)
    {
        UsesVertexColorAccumulator |= UsesAddVertex;
        Load33Shaders(UsesVertexTextureAccumulator, UsesVertexColorAccumulator, UsesVertexAccumulator, UsesAddVertex, UsesVertex3Accumulator, UsesVertex3TextureAccumulator, UsesVertex3ColorAccumulator);
    }
    ~OpenGL33Cache() {
        if (matrix) {
            delete matrix;
        }
        while (!matrixStack.empty()) {
            delete matrixStack.top();
            matrixStack.pop();
        }
        Release33Shaders();
    }

    virtual bool IsCoreProfile() override { return true;}

    void Draw(DrawGLUtils::xlVertexAccumulator &va, const xlColor & color, int type, int enableCapability) override {
        if (va.count == 0) {
            return;
        }

        ShaderProgram *program = &singleColorProgram;
        if (va.coordsPerVertex == 3) {
            program = &singleColor3Program;
        }

        program->UseProgram();
        program->SetMatrix(*matrix);
        int offset0 = program->BindBuffer(0, &va.vertices[0], va.count*va.coordsPerVertex*sizeof(GLfloat)) / (va.coordsPerVertex*sizeof(GLfloat));
        LOG_GL_ERRORV(glVertexAttribPointer(0, va.coordsPerVertex, GL_FLOAT, GL_FALSE, 0, (void*)0 ));

        LOG_GL_ERRORV(GLuint cid = glGetUniformLocation(program->ProgramID, "inColor"));
        LOG_GL_ERRORV(glUniform4f(cid,
                    ((float)color.Red())/255.0,
                    ((float)color.Green())/255.0,
                    ((float)color.Blue())/255.0,
                    ((float)color.Alpha())/255.0
                    ));
        float ps = 0;
        if (type == GL_POINTS && enableCapability == 0x0B10) {
            //POINT_SMOOTH, removed in OpenGL3.x
            program->SetRenderType(1);
            ps = program->CalcSmoothPointParams();
        } else if (enableCapability > 0) {
            LOG_GL_ERRORV(glEnable(enableCapability));
        }
        LOG_GL_ERRORV(glDrawArrays(type, offset0, va.count));
        if (type == GL_POINTS && enableCapability == 0x0B10) {
            program->SetRenderType(0);
            LOG_GL_ERRORV(glPointSize(ps));
        } else if (enableCapability > 0) {
            LOG_GL_ERRORV(glDisable(enableCapability));
        }
        program->UnbindBuffer(0);
    }

    void Draw(DrawGLUtils::xlAccumulator &va, DrawType dt) override {
        if (va.count == 0) {
            bool hasMesh = false;
            for (auto &a : va.types) {
                hasMesh |= a.mesh != nullptr;
            }
            if (!hasMesh) {
                return;
            }
        }
        ShaderProgram *program = &normalProgram;
        ShaderProgram *texturep = &textureProgram;
        if (va.coordsPerVertex == 3) {
            program = &normal3Program;
            texturep = &texture3Program;
        }

        
        bool hasTexture = false;
        for (auto &brt : va.types) {
            if (brt.textureId != -1) {
                hasTexture = true;
            }
        }
        int toffset0 = 0;
        if (hasTexture) {
            // there are textures, bind the texture vertices
            texturep->UseProgram();
            texturep->SetMatrix(*matrix);
            if (va.tvertices) {
                toffset0 = texturep->BindBuffer(0, &va.vertices[0], va.count*va.coordsPerVertex*sizeof(GLfloat))/ (va.coordsPerVertex*sizeof(GLfloat));
                LOG_GL_ERRORV(glVertexAttribPointer(0, va.coordsPerVertex, GL_FLOAT, GL_FALSE, 0, (void*)0 ));
                texturep->BindBuffer(2, va.tvertices, va.count * 2 * sizeof(GLfloat));
                LOG_GL_ERRORV(glVertexAttribPointer(2, 2, GL_FLOAT, GL_TRUE, 0, (void*)0 ));
            }
        }
        
        program->UseProgram();
        program->SetMatrix(*matrix);
        program->SetRenderType(0);
        
        int roffset0 = program->BindBuffer(0, &va.vertices[0], va.count*va.coordsPerVertex*sizeof(GLfloat))/ (va.coordsPerVertex*sizeof(GLfloat));
        LOG_GL_ERRORV(glVertexAttribPointer(0, va.coordsPerVertex, GL_FLOAT, GL_FALSE, 0, (void*)0 ));
        int offset0 = roffset0;
        program->BindBuffer(1, &va.colors[0], va.count*4*sizeof(GLubyte));
        LOG_GL_ERRORV(glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (void*)0 ));
        
        bool ttype = dt == DrawGLUtils::xlGLCacheInfo::DrawType::TRANSPARENTS;

        int lastTextureId = -1;
        for (auto &brt : va.types) {
            int type = brt.type;
            int enableCapability = brt.enableCapability;
            if (brt.mesh != nullptr) {
                GL3Mesh *mesh = (GL3Mesh*)brt.mesh;
                mesh->Draw(brt.extra == 1, brt.textureBrightness, *matrix, singleColor3Program, meshProgram, dt);
                program->UseProgram();
                program->ReBindBuffer(0);
                program->ReBindBuffer(1);
                continue;
            } else if (brt.textureId != lastTextureId) {
                if (brt.textureId == -1) {
                    //back to non-texture
                    offset0 = roffset0;
                    LOG_GL_ERRORV(glBindTexture(GL_TEXTURE_2D, 0));
                    LOG_GL_ERRORV(glDisableVertexAttribArray(2));
                    program->UseProgram();
                    program->ReBindBuffer(0);
                    LOG_GL_ERRORV(glEnableVertexAttribArray(0));
                    LOG_GL_ERRORV(glVertexAttribPointer(0, va.coordsPerVertex, GL_FLOAT, GL_FALSE, 0, (void*)0 ));
                    program->ReBindBuffer(1);
                    LOG_GL_ERRORV(glEnableVertexAttribArray(1));
                    LOG_GL_ERRORV(glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (void*)0 ));
                } else if (lastTextureId == -1) {
                    //from non-texture to texture
                    texturep->UseProgram();
                    texturep->ReBindBuffer(0);
                    LOG_GL_ERRORV(glVertexAttribPointer(0, va.coordsPerVertex, GL_FLOAT, GL_FALSE, 0, (void*)0 ));
                    texturep->ReBindBuffer(2);
                    LOG_GL_ERRORV(glVertexAttribPointer(2, 2, GL_FLOAT, GL_TRUE, 0, (void*)0 ));
                    offset0 = toffset0;

                    LOG_GL_ERRORV(glActiveTexture(GL_TEXTURE0)); //switch to texture image unit 0
                    LOG_GL_ERRORV(glBindTexture(GL_TEXTURE_2D, brt.textureId));
                    LOG_GL_ERRORV(glUniform1i(glGetUniformLocation(texturep->ProgramID, "tex"), 0));
                    
                    LOG_GL_ERRORV(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
                    LOG_GL_ERRORV(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
                } else {
                    // switching texture
                    LOG_GL_ERRORV(glActiveTexture(GL_TEXTURE0)); //switch to texture image unit 0
                    LOG_GL_ERRORV(glBindTexture(GL_TEXTURE_2D, brt.textureId));
                    LOG_GL_ERRORV(glUniform1i(glGetUniformLocation(texturep->ProgramID, "tex"), 0));
                }
                lastTextureId = brt.textureId;
            }
            if (brt.mesh == nullptr) {
                bool doDraw = !ttype;
                if (brt.textureId != -1) {
                    GLuint cid = glGetUniformLocation(texturep->ProgramID, "inColor");
                    if (brt.useTexturePixelColor) {
                        LOG_GL_ERRORV(glUniform4f(cid, ((float)brt.texturePixelColor.red) / 255.0f,
                                                  ((float)brt.texturePixelColor.green) / 255.0f,
                                                  ((float)brt.texturePixelColor.blue) / 255.0f,
                                                  ((float)brt.texturePixelColor.alpha) / 255.0f));
                        texturep->SetRenderType(1);
                    } else {
                        if (brt.textureAlpha != 255) {
                            doDraw = ttype;
                        }
                        float alpha = ((float)brt.textureAlpha)/255.0;
                        float brightness = brt.textureBrightness / 100.0f;
                        LOG_GL_ERRORV(glUniform4f(cid, brightness, brightness, brightness, alpha));
                        texturep->SetRenderType(0);
                    }
                } else if (type == GL_POINTS && enableCapability == 0x0B10) {
                    //POINT_SMOOTH, removed in OpenGL3.x
                    program->SetRenderType(1);
                    program->CalcSmoothPointParams(brt.extra);
                } else {
                    if (brt.type == GL_POINTS) {
                        LOG_GL_ERRORV(glPointSize(brt.extra));
                    }
                    if (enableCapability > 0) {
                        program->SetRenderType(0);
                        LOG_GL_ERRORV(glEnable(enableCapability));
                    } else if (enableCapability != 0) {
                        program->SetRenderType(enableCapability);
                    } else {
                        program->SetRenderType(0);
                    }
                }
                if (doDraw) {
                    LOG_GL_ERRORV(glDrawArrays(type, offset0 + brt.start, brt.count));
                }
                if (enableCapability > 0 && type != GL_POINTS && enableCapability != 0x0B10) {
                    LOG_GL_ERRORV(glDisable(enableCapability));
                }
            }
        }
        program->SetRenderType(0);
        program->UnbindBuffer(0);
        program->UnbindBuffer(1);
        if (hasTexture) {
            texturep->UnbindBuffer(0);
            texturep->UnbindBuffer(2);
        }
    }
    void Draw(DrawGLUtils::xlVertexColorAccumulator &va, int type, int enableCapability) override {
        if (va.count == 0) {
            return;
        }

        ShaderProgram *program = &normalProgram;
        if (va.coordsPerVertex == 3) {
            program = &normal3Program;
        }

        program->UseProgram();
        program->SetMatrix(*matrix);

        int offset0 = program->BindBuffer(0, &va.vertices[0], va.count*va.coordsPerVertex*sizeof(GLfloat))/ (va.coordsPerVertex*sizeof(GLfloat));
        LOG_GL_ERRORV(glVertexAttribPointer(0, va.coordsPerVertex, GL_FLOAT, GL_FALSE, 0, (void*)0 ));

        program->BindBuffer(1, &va.colors[0], va.count*4*sizeof(GLubyte));
        LOG_GL_ERRORV(glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (void*)0 ));

        float ps = 2.0;
        if (type == GL_POINTS && enableCapability == 0x0B10) {
            //POINT_SMOOTH, removed in OpenGL3.x
            program->SetRenderType(1);
            ps = program->CalcSmoothPointParams();
        } else {
            if (enableCapability > 0) {
                LOG_GL_ERRORV(glEnable(enableCapability));
            } else if (enableCapability != 0) {
                program->SetRenderType(enableCapability);
            }
        }
        LOG_GL_ERRORV(glDrawArrays(type, offset0, va.count));
        if (type == GL_POINTS && enableCapability == 0x0B10) {
            program->SetRenderType(0);
            LOG_GL_ERRORV(glPointSize(ps));
        } else if (enableCapability > 0) {
            LOG_GL_ERRORV(glDisable(enableCapability));
        } else if (enableCapability != 0) {
            program->SetRenderType(0);
        }

        program->UnbindBuffer(0);
        program->UnbindBuffer(1);
    }
    void Draw(DrawGLUtils::xlVertexTextureAccumulator &va, int type, int enableCapability) override {
        if (va.count == 0) {
            return;
        }
        ShaderProgram *program = &textureProgram;
        if (va.coordsPerVertex == 3) {
            program = &texture3Program;
        }

        program->UseProgram();
        program->SetMatrix(*matrix);

        int offset0 = textureProgram.BindBuffer(0, &va.vertices[0], va.count*va.coordsPerVertex*sizeof(GLfloat)) / (va.coordsPerVertex*sizeof(GLfloat));
        LOG_GL_ERRORV(glVertexAttribPointer(0, va.coordsPerVertex, GL_FLOAT, GL_FALSE, 0, (void*)0 ));

        textureProgram.BindBuffer(2, va.tvertices, va.count*2*sizeof(GLfloat));
        LOG_GL_ERRORV(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 ));

        LOG_GL_ERRORV(glActiveTexture(GL_TEXTURE0)); //switch to texture image unit 0
        LOG_GL_ERRORV(glBindTexture(GL_TEXTURE_2D, va.id));
        LOG_GL_ERRORV(glUniform1i(glGetUniformLocation(program->ProgramID, "tex"), 0));

        GLuint cid = glGetUniformLocation(program->ProgramID, "inColor");
        if (va.forceColor) {
            program->SetRenderType(1);
            LOG_GL_ERRORV(glUniform4f(cid, ((float)va.color.red) / 255.0f,
                                      ((float)va.color.green) / 255.0f,
                                      ((float)va.color.blue) / 255.0f,
                                      ((float)va.color.alpha) / 255.0f));
        } else {
            program->SetRenderType(0);
            LOG_GL_ERRORV(glUniform4f(cid, 1.0, 1.0, 1.0, ((float)va.alpha)/255.0));
        }

        if (enableCapability > 0) {
            LOG_GL_ERRORV(glEnable(enableCapability));
        }
        LOG_GL_ERRORV(glDrawArrays(type, offset0, va.count));
        if (enableCapability > 0) {
            LOG_GL_ERRORV(glDisable(enableCapability));
        }
        LOG_GL_ERRORV(glBindTexture(GL_TEXTURE_2D, 0));
        LOG_GL_ERRORV(glActiveTexture(GL_TEXTURE0)); //switch to texture image unit 0

        program->UnbindBuffer(0);
        program->UnbindBuffer(1);
    }
    
    virtual DrawGLUtils::xl3DMesh *createMesh() override {
        return new GL3Mesh();
    }
    virtual void SetCurrent() override {
        DrawGLUtils::xlGLCacheInfo::SetCurrent();
        data.Reset();

        textureProgram.Reset();
        texture3Program.Reset();
        singleColorProgram.Reset();
        singleColor3Program.Reset();
        normalProgram.Reset();
        normal3Program.Reset();
        vbNormalProgram.Reset();
    }

    DrawGLUtils::xlVertexColorAccumulator data;
    virtual void addVertex(float x, float y, const xlColor &c) override {
        data.PreAlloc(1);
        data.AddVertex(x, y, c);
    }
    virtual void ensureSize(unsigned int s) override {
        data.PreAlloc(s);
    }
    virtual unsigned int vertexCount() override {
        return data.count;
    }

    void flush(int type, int enableCapability) override {
        Draw(data, type, enableCapability);
        data.Reset();
    }

    void DrawTexture(GLuint texture,
                     float x, float y, float x2, float y2,
                     float tx, float ty, float tx2, float ty2) override {
        DrawGLUtils::xlVertexTextureAccumulator va(texture);
        va.PreAlloc(12);
        va.AddVertex(x, y, tx, ty);
        va.AddVertex(x2, y, tx2, ty);
        va.AddVertex(x2, y2, tx2, ty2);
        va.AddVertex(x2, y2, tx2, ty2);
        va.AddVertex(x, y2, tx, ty2);
        va.AddVertex(x, y, tx, ty);
        Draw(va, GL_TRIANGLES, 0);
    }

    void Ortho(int topleft_x, int topleft_y, int bottomright_x, int bottomright_y) override {
        if (matrix) {
            delete matrix;
        }
        glm::mat4 m = glm::ortho((float)topleft_x, (float)bottomright_x, (float)bottomright_y, (float)topleft_y);
        matrix = new glm::mat4(m);
    }

    void Perspective(int topleft_x, int topleft_y, int bottomright_x, int bottomright_y) override {
        if (matrix) {
            delete matrix;
        }
        glm::mat4 m = glm::perspective(glm::radians(45.0f), (float) (bottomright_x-topleft_x) / (float)(topleft_y-bottomright_y), 1.0f, 10000.0f);
        matrix = new glm::mat4(m);
    }

    virtual void SetCamera(glm::mat4& view_matrix) override {
        *matrix = *matrix * view_matrix;
    }

    void PushMatrix() override {
        matrixStack.push(matrix);
        matrix = new glm::mat4(*matrix);
    }
    void PopMatrix() override {
        static log4cpp::Category &logger_opengl = log4cpp::Category::getInstance(std::string("log_opengl"));
        if (!matrixStack.empty()) {
            if (matrix != nullptr) {
                delete matrix;
            }
            matrix = matrixStack.top();
            matrixStack.pop();
        }
        else
        {
            logger_opengl.error("OpenGL33Cache PopMatrix called but no matrixes in the stack.");
        }
    }
    void Translate(float x, float y, float z) override {
        if (matrix) {
            glm::mat4 tm = glm::translate(*matrix, glm::vec3(x, y, z));
            delete matrix;
            matrix = new glm::mat4(tm);
        } else {
            glm::mat4 tm = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));
            matrix = new glm::mat4(tm);
        }
    }
    void Rotate(float angle, float x, float y, float z) override {
        if (matrix) {
            angle = wxDegToRad(angle);
            glm::mat4 tm = glm::rotate(*matrix, angle, glm::vec3(x, y, z));
            delete matrix;
            matrix = new glm::mat4(tm);
        } else {
            angle = wxDegToRad(angle);
            glm::mat4 tm = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(x, y, z));
            matrix = new glm::mat4(tm);
        }
    }
    void Scale(float w, float h, float z) override {
        if (matrix) {
            glm::mat4 tm = glm::scale(*matrix, glm::vec3(w, h, z));
            delete matrix;
            matrix = new glm::mat4(tm);
        } else {
            glm::mat4 tm = glm::scale(glm::mat4(1.0f), glm::vec3(w, h, z));
            matrix = new glm::mat4(tm);
        }
    }


protected:


    std::stack<glm::mat4*> matrixStack;
    glm::mat4 *matrix;
};
std::atomic_int OpenGL33Cache::cacheCount(0);
GLuint OpenGL33Cache::textureProgramId(0);
GLuint OpenGL33Cache::texture3ProgramId(0);
GLuint OpenGL33Cache::meshProgramId(0);
GLuint OpenGL33Cache::singleColorProgramId(0);
GLuint OpenGL33Cache::singleColor3ProgramId(0);
GLuint OpenGL33Cache::normalProgramId(0);
GLuint OpenGL33Cache::normal3ProgramId(0);
GLuint OpenGL33Cache::vbNormalProgramId(0);



DrawGLUtils::xlGLCacheInfo *Create33Cache(bool UsesVertexTextureAccumulator,
                                          bool UsesVertexColorAccumulator,
                                          bool UsesVertexAccumulator,
                                          bool UsesAddVertex,
                                          bool UsesVertex3Accumulator,
                                          bool UsesVertex3TextureAccumulator,
										  bool UsesVertex3ColorAccumulator) {
    return new OpenGL33Cache(UsesVertexTextureAccumulator, UsesVertexColorAccumulator, UsesVertexAccumulator, UsesAddVertex, UsesVertex3Accumulator, UsesVertexTextureAccumulator, UsesVertex3ColorAccumulator);
}
