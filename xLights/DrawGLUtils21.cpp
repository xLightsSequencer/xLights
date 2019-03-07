
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifdef __WXMAC__
#include "OpenGL/gl.h"
#else

#include "wx/wx.h"

#include <GL/gl.h>
#ifdef _MSC_VER
//#include "GL/glut.h"
#include "GL/glext.h"
#else
#include <GL/glext.h>
#endif
#endif

#include "DrawGLUtils.h"

#include <log4cpp/Category.hh>

#include <stack>

#ifndef __WXMAC__
extern PFNGLUSEPROGRAMPROC glUseProgram;
extern PFNGLCREATESHADERPROC glCreateShader;
extern PFNGLSHADERSOURCEPROC glShaderSource;
extern PFNGLCOMPILESHADERPROC glCompileShader;
extern PFNGLGETSHADERIVPROC glGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
extern PFNGLCREATEPROGRAMPROC glCreateProgram;
extern PFNGLATTACHSHADERPROC glAttachShader;
extern PFNGLGETPROGRAMIVPROC glGetProgramiv;
extern PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
extern PFNGLDETACHSHADERPROC glDetachShader;
extern PFNGLDELETESHADERPROC glDeleteShader;
extern PFNGLLINKPROGRAMPROC glLinkProgram;
extern PFNGLDELETEPROGRAMPROC glDeleteProgram;
extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
extern PFNGLUNIFORM1IPROC glUniform1i;
extern PFNGLUNIFORM4FPROC glUniform4f;
extern PFNGLUNIFORM1FPROC glUniform1f;
extern PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
extern PFNGLBINDBUFFERPROC glBindBuffer;
extern PFNGLDELETEBUFFERSPROC glDeleteBuffers;
extern PFNGLGENBUFFERSPROC glGenBuffers;
extern PFNGLBUFFERDATAPROC glBufferData;
extern PFNGLFLUSHMAPPEDBUFFERRANGEPROC glFlushMappedBufferRange;
extern PFNGLMAPBUFFERRANGEPROC glMapBufferRange;
extern PFNGLUNMAPBUFFERPROC glUnmapBuffer;
#ifndef LINUX
extern PFNGLACTIVETEXTUREPROC glActiveTexture;
#endif
extern PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
extern PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
extern PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
extern PFNGLPOINTPARAMETERFPROC glPointParameterf;
#endif

extern void LoadGLFunctions();

class OpenGL21Cache : public DrawGLUtils::xlGLCacheInfo {
public:


    OpenGL21Cache() : matrix(nullptr) {
        LOG_GL_ERRORV(glEnable(GL_COLOR_MATERIAL));
        LOG_GL_ERRORV(glDisable(GL_TEXTURE_2D));
        GLuint VertexShaderIDc = CompileShader(GL_VERTEX_SHADER,
            "#version 120\n"
            "attribute vec2 vertexPosition_modelspace;\n"
            "attribute vec4 vertexColor;\n"
            "varying vec4 fragmentColor;\n"
            "uniform mat4 MVP;\n"
            "void main(){\n"
            "    gl_Position = MVP * vec4(vertexPosition_modelspace,0,1);\n"
            "    fragmentColor = vertexColor;\n"
            "}\n");
        GLuint VertexShader3DIDc = CompileShader(GL_VERTEX_SHADER,
            "#version 120\n"
            "attribute vec3 vertexPosition_modelspace;\n"
            "attribute vec4 vertexColor;\n"
            "varying vec4 fragmentColor;\n"
            "uniform mat4 MVP;\n"
            "void main(){\n"
            "    gl_Position = MVP * vec4(vertexPosition_modelspace,1);\n"
            "    fragmentColor = vertexColor;\n"
            "}\n");
        GLuint VertexShaderIDsc = CompileShader(GL_VERTEX_SHADER,
           "#version 120\n"
           "attribute vec2 vertexPosition_modelspace;\n"
           "varying vec4 fragmentColor;\n"
           "uniform vec4 color;\n"
           "uniform mat4 MVP;\n"
           "void main(){\n"
           "    gl_Position = MVP * vec4(vertexPosition_modelspace,0,1);\n"
           "    fragmentColor = color;\n"
           "}\n");
        GLuint VertexShader3DIDsc = CompileShader(GL_VERTEX_SHADER,
           "#version 120\n"
           "attribute vec3 vertexPosition_modelspace;\n"
           "varying vec4 fragmentColor;\n"
           "uniform vec4 color;\n"
           "uniform mat4 MVP;\n"
           "void main(){\n"
           "    gl_Position = MVP * vec4(vertexPosition_modelspace,1);\n"
           "    fragmentColor = color;\n"
           "}\n");
        GLuint FragmentShaderID = CompileShader(GL_FRAGMENT_SHADER,
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

        ProgramIDcolors = LinkProgram(VertexShaderIDc, FragmentShaderID);
        ProgramIDstaticColor = LinkProgram(VertexShaderIDsc, FragmentShaderID);
        ProgramID3Dcolors = LinkProgram(VertexShader3DIDc, FragmentShaderID);
        ProgramID3DstaticColor = LinkProgram(VertexShader3DIDsc, FragmentShaderID);

        GLuint VertexShaderIDtx = CompileShader(GL_VERTEX_SHADER,
            "#version 120\n"
            "attribute vec2 vertexPosition_modelspace;\n"
            "attribute vec2 texturePos;\n"
            "varying vec2 textCoord;\n"
            "uniform mat4 MVP;\n"
            "void main(){\n"
            "    gl_Position = MVP * vec4(vertexPosition_modelspace,0,1);\n"
            "    textCoord = texturePos;\n"
            "}\n");
        GLuint VertexShader3DIDtx = CompileShader(GL_VERTEX_SHADER,
            "#version 120\n"
            "attribute vec3 vertexPosition_modelspace;\n"
            "attribute vec2 texturePos;\n"
            "varying vec2 textCoord;\n"
            "uniform mat4 MVP;\n"
            "void main(){\n"
            "    gl_Position = MVP * vec4(vertexPosition_modelspace,1);\n"
            "    textCoord = texturePos;\n"
            "}\n");
        GLuint FragmentShaderIDtxt = CompileShader(GL_FRAGMENT_SHADER,
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

        ProgramIDtexture = LinkProgram(VertexShaderIDtx, FragmentShaderIDtxt);
        ProgramID3Dtexture = LinkProgram(VertexShader3DIDtx, FragmentShaderIDtxt);

        LOG_GL_ERRORV(glDeleteShader(VertexShaderIDtx));
        LOG_GL_ERRORV(glDeleteShader(VertexShader3DIDtx));
        LOG_GL_ERRORV(glDeleteShader(FragmentShaderIDtxt));
        LOG_GL_ERRORV(glDeleteShader(VertexShaderIDc));
        LOG_GL_ERRORV(glDeleteShader(VertexShaderIDsc));
        LOG_GL_ERRORV(glDeleteShader(VertexShader3DIDc));
        LOG_GL_ERRORV(glDeleteShader(VertexShader3DIDsc));
        LOG_GL_ERRORV(glDeleteShader(FragmentShaderID));


        LOG_GL_ERRORV(glUseProgram(ProgramIDcolors));

        isIntel = wxString(glGetString(GL_VENDOR)).Contains("Intel");
        //isIntel = true;
    }

    GLuint CompileShader(GLenum type, const char *text) {
        GLuint shad = glCreateShader(type);
        GLint Result = GL_FALSE;
        int InfoLogLength;

        LOG_GL_ERRORV(glShaderSource(shad, 1, &text , nullptr));
        LOG_GL_ERRORV(glCompileShader(shad));

        glGetShaderiv(shad, GL_COMPILE_STATUS, &Result);
        glGetShaderiv(shad, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if (!Result && InfoLogLength > 0 ) {
            std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
            glGetShaderInfoLog(shad, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
            wxString l = &VertexShaderErrorMessage[0];
            l.Trim();
            if (l.length() > 0) {
                printf("Shader Log(2.1): %s\n", &VertexShaderErrorMessage[0]);
                static log4cpp::Category &logger_opengl = log4cpp::Category::getInstance(std::string("log_opengl"));
                logger_opengl.error(std::string(&VertexShaderErrorMessage[0]));
            }
        }
        return shad;
    }
    GLuint LinkProgram(GLuint VertexShaderID, GLuint FragmentShaderID) {
        GLint Result = GL_FALSE;
        int InfoLogLength;

        GLuint ProgramID = glCreateProgram();

        LOG_GL_ERRORV(glAttachShader(ProgramID, VertexShaderID));
        LOG_GL_ERRORV(glAttachShader(ProgramID, FragmentShaderID));

        LOG_GL_ERRORV(glLinkProgram(ProgramID));
        glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
        glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if (!Result && InfoLogLength > 0 ){
            std::vector<char> ProgramErrorMessage(InfoLogLength+1);
            glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
            wxString l = &ProgramErrorMessage[0];
            l.Trim();
            if (l.length() > 0) {
                printf("Program Log(2.1): %s\n", &ProgramErrorMessage[0]);
                static log4cpp::Category &logger_opengl = log4cpp::Category::getInstance(std::string("log_opengl"));
                logger_opengl.error(std::string(&ProgramErrorMessage[0]));
            }
        }
        LOG_GL_ERRORV(glDetachShader(ProgramID, VertexShaderID));
        LOG_GL_ERRORV(glDetachShader(ProgramID, FragmentShaderID));
        return ProgramID;
    }

    ~OpenGL21Cache() {
        if (matrix) {
            delete matrix;
        }
        while (!matrixStack.empty()) {
            delete matrixStack.top();
            matrixStack.pop();
        }
        if (ProgramIDcolors != 0) {
            LOG_GL_ERRORV(glDeleteProgram(ProgramIDcolors));
        }
        if (ProgramIDstaticColor != 0) {
            LOG_GL_ERRORV(glDeleteProgram(ProgramIDstaticColor));
        }
        if (ProgramID3Dcolors != 0) {
            LOG_GL_ERRORV(glDeleteProgram(ProgramID3Dcolors));
        }
        if (ProgramID3DstaticColor != 0) {
            LOG_GL_ERRORV(glDeleteProgram(ProgramID3DstaticColor));
        }
        if (ProgramIDtexture != 0) {
            LOG_GL_ERRORV(glDeleteProgram(ProgramIDtexture));
        }
        if (ProgramID3Dtexture != 0) {
            LOG_GL_ERRORV(glDeleteProgram(ProgramID3Dtexture));
        }
    }

	void Draw(DrawGLUtils::xlVertexAccumulator &va, const xlColor & color, int type, int enableCapability) override {
		if (va.count == 0) {
			return;
		}
        
        GLuint program = ProgramIDstaticColor;
        if (va.coordsPerVertex == 3) {
            program = ProgramID3DstaticColor;
        }
        
		LOG_GL_ERRORV(glUseProgram(program));
		SetMVP(program);

		GLuint vattrib = glGetAttribLocation(program, "vertexPosition_modelspace");
		LOG_GL_ERRORV(glEnableVertexAttribArray(vattrib));
		LOG_GL_ERRORV(glVertexAttribPointer(vattrib, va.coordsPerVertex, GL_FLOAT, false, 0, &va.vertices[0]));
		GLuint cid = glGetUniformLocation(program, "color");
		glUniform4f(cid,
			((float)color.Red()) / 255.0,
			((float)color.Green()) / 255.0,
			((float)color.Blue()) / 255.0,
			((float)color.Alpha()) / 255.0
		);

		if (enableCapability != 0) {
			glEnable(enableCapability);
		}
		LOG_GL_ERRORV(glDrawArrays(type, 0, va.count));
		if (enableCapability > 0) {
			glDisable(enableCapability);
		}
		LOG_GL_ERRORV(glDisableVertexAttribArray(vattrib));
	}
	void CalcSmoothPointParams(GLuint pid, float ps) {
        glPointSize(ps+1);
        float delta = 1.0 / (ps+1);
        float mid = 0.35 + 0.15 * ((ps - 1.0f)/25.0f);
        if (mid > 0.5) {
            mid = 0.5;
        }

        float min = std::max(0.0f, mid - delta);
        float max = std::min(1.0f, mid + delta);
        glUniform1f(glGetUniformLocation(pid, "PointSmoothMin"), min);
        glUniform1f(glGetUniformLocation(pid, "PointSmoothMax"), max);
    }
    float CalcSmoothPointParams(GLuint pid) {
        float ps;
        glGetFloatv(GL_POINT_SIZE, &ps);
        CalcSmoothPointParams(pid, ps);
        return ps;
    }


    void Draw(DrawGLUtils::xlAccumulator &va, DrawType dt) override {
        if (va.count == 0 || dt == DrawType::SOLIDS) {
            //we cannot determine transparent vs solid right now so draw everything when transparents are asked
            return;
        }
        bool intelMapped = false;
        GLuint programId = ProgramIDcolors;
        GLuint textProgramId = ProgramIDtexture;
        if (va.coordsPerVertex == 3) {
            programId = ProgramID3Dcolors;
            textProgramId = ProgramID3Dtexture;
        }
        
        LOG_GL_ERRORV(glUseProgram(programId));
        SetMVP(programId);

        GLuint vattrib = glGetAttribLocation( programId, "vertexPosition_modelspace" );
        LOG_GL_ERRORV(glEnableVertexAttribArray(vattrib));
        LOG_GL_ERRORV(glVertexAttribPointer(vattrib, va.coordsPerVertex, GL_FLOAT, false, 0, &va.vertices[0]));

        GLuint cattrib = glGetAttribLocation( programId, "vertexColor" );
        LOG_GL_ERRORV(glEnableVertexAttribArray(cattrib));
        LOG_GL_ERRORV(glVertexAttribPointer(cattrib, 4, GL_UNSIGNED_BYTE, true, 0, &va.colors[0]));

        bool textBound = false;
        GLuint tattrib = 0;
        GLuint tvattrib = 0;

        for (auto it = va.types.begin(); it != va.types.end(); ++it) {
            if (it->type == GL_POINTS) {
                LOG_GL_ERRORV(glPointSize(it->extra));
            } else if (it->type == GL_LINES || it->type == GL_LINE_LOOP || it->type == GL_LINE_STRIP) {
                DrawGLUtils::SetLineWidth(it->extra);
            }
            if (isIntel && it->enableCapability == GL_POINT_SMOOTH) {
                if (it->enableCapability != 0) {
                    glEnable(it->enableCapability);
                }
                LOG_GL_ERRORV(glDisableVertexAttribArray(cattrib));
                LOG_GL_ERRORV(glDisableVertexAttribArray(vattrib));

                LOG_GL_ERRORV(glEnableClientState(GL_VERTEX_ARRAY));
                LOG_GL_ERRORV(glEnableClientState(GL_COLOR_ARRAY));
                if (!intelMapped) {
                    intelMapped = true;
                    LOG_GL_ERRORV(glColorPointer(4, GL_UNSIGNED_BYTE, 0, &va.colors[0]));
                    LOG_GL_ERRORV(glVertexPointer(va.coordsPerVertex, GL_FLOAT, 0, &va.vertices[0]));
                }
                LOG_GL_ERRORV(glUseProgram(0));
                LOG_GL_ERRORV(glPushMatrix());
                LOG_GL_ERRORV(glLoadMatrixf(glm::value_ptr(*matrix)));
                LOG_GL_ERRORV(glDrawArrays(it->type, it->start, it->count));
                LOG_GL_ERRORV(glPopMatrix());
                LOG_GL_ERRORV(glUseProgram(programId));
                LOG_GL_ERRORV(glDisableClientState(GL_VERTEX_ARRAY));
                LOG_GL_ERRORV(glDisableClientState(GL_COLOR_ARRAY));

                if (it->enableCapability != 0) {
                    glDisable(it->enableCapability);
                }
                LOG_GL_ERRORV(glEnableVertexAttribArray(cattrib));
                LOG_GL_ERRORV(glEnableVertexAttribArray(vattrib));
            } else {
                int enableCapability = it->enableCapability;

                float ps = 2.0;
                if (it->textureId != -1) {
                    LOG_GL_ERRORV(glDisableVertexAttribArray(vattrib));
                    LOG_GL_ERRORV(glDisableVertexAttribArray(cattrib));
                    LOG_GL_ERRORV(glUseProgram(textProgramId));
                    if (!textBound) {
                        SetMVP(textProgramId);
                        tattrib = glGetAttribLocation( textProgramId, "texturePos" );
                        tvattrib = glGetAttribLocation( textProgramId, "vertexPosition_modelspace" );
                        LOG_GL_ERRORV(glUniform1i(glGetUniformLocation(textProgramId, "tex"), 0));
                    }
                    LOG_GL_ERRORV(glEnableVertexAttribArray(tvattrib));
                    if (vattrib != tvattrib) {
                        LOG_GL_ERRORV(glVertexAttribPointer(tvattrib, va.coordsPerVertex, GL_FLOAT, false, 0, va.vertices));
                    }
                    LOG_GL_ERRORV(glEnableVertexAttribArray(tattrib));
                    if (tattrib == cattrib || tattrib == vattrib) {
                        LOG_GL_ERRORV(glVertexAttribPointer(tattrib, 2, GL_FLOAT, true, 0, va.tvertices));
                    }

                    if (it->useTexturePixelColor) {
                        GLuint cid = glGetUniformLocation(textProgramId, "RenderType");
                        LOG_GL_ERRORV(glUniform1i(cid, 1));
                        cid = glGetUniformLocation(textProgramId, "inColor");
                        LOG_GL_ERRORV(glUniform4f(cid, ((float)it->texturePixelColor.red) / 255.0f,
                                                  ((float)it->texturePixelColor.green) / 255.0f,
                                                  ((float)it->texturePixelColor.blue) / 255.0f,
                                                  ((float)it->texturePixelColor.alpha) / 255.0f));
                    } else {
                        GLuint cid = glGetUniformLocation(textProgramId, "RenderType");
                        LOG_GL_ERRORV(glUniform1i(cid, 0));
                        cid = glGetUniformLocation(textProgramId, "inColor");
                        float trans = it->textureAlpha;
                        trans /= 255.0f;
                        float bri = it->textureBrightness;
                        bri /= 100.0f;
                        LOG_GL_ERRORV(glUniform4f(cid, bri, bri, bri, trans));
                    }
                    LOG_GL_ERRORV(glActiveTexture(GL_TEXTURE0)); //switch to texture image unit 0
                    LOG_GL_ERRORV(glBindTexture(GL_TEXTURE_2D, it->textureId));
                    
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                }
                if (enableCapability != 0) {
                    if (enableCapability == GL_POINT_SMOOTH) {
                        //LOG_GL_ERRORV(glEnable(enableCapability));
                        GLuint cid = glGetUniformLocation(programId, "RenderType");
                        glUniform1i(cid, 1);
                        ps = CalcSmoothPointParams(programId);
                        LOG_GL_ERRORV(glEnable(GL_VERTEX_PROGRAM_POINT_SIZE));
                        LOG_GL_ERRORV(glEnable(GL_POINT_SPRITE));
                        LOG_GL_ERRORV(glTexEnvi(GL_POINT_SPRITE_ARB,GL_COORD_REPLACE_ARB ,GL_FALSE));
                    } else {
                        LOG_GL_ERRORV(glEnable(enableCapability));
                    }
                }
                LOG_GL_ERRORV(glDrawArrays(it->type, it->start, it->count));
                if (it->textureId != -1) {
                    LOG_GL_ERRORV(glBindTexture(GL_TEXTURE_2D, 0));
                    LOG_GL_ERRORV(glDisableVertexAttribArray(tattrib));
                    LOG_GL_ERRORV(glDisableVertexAttribArray(tvattrib));
                    LOG_GL_ERRORV(glUseProgram(programId));
                    LOG_GL_ERRORV(glEnableVertexAttribArray(vattrib));
                    if (tvattrib != vattrib) {
                        LOG_GL_ERRORV(glVertexAttribPointer(vattrib, va.coordsPerVertex, GL_FLOAT, false, 0, &va.vertices[0]));
                    }
                    LOG_GL_ERRORV(glEnableVertexAttribArray(cattrib));
                    if (tattrib == cattrib || tattrib == vattrib) {
                        LOG_GL_ERRORV(glVertexAttribPointer(cattrib, 4, GL_UNSIGNED_BYTE, true, 0, &va.colors[0]));
                    }
                }
                if (enableCapability > 0) {
                    if (enableCapability == GL_POINT_SMOOTH || enableCapability == GL_POINT_SPRITE) {
                        GLuint cid = glGetUniformLocation(programId, "RenderType");
                        glUniform1i(cid, 0);
                        LOG_GL_ERRORV(glPointSize(ps));
                        LOG_GL_ERRORV(glDisable(GL_POINT_SPRITE));
                        LOG_GL_ERRORV(glDisable(GL_VERTEX_PROGRAM_POINT_SIZE));
                    } else {
                        glDisable(enableCapability);
                    }
                }
            }
        }

        LOG_GL_ERRORV(glDisableVertexAttribArray(cattrib));
        LOG_GL_ERRORV(glDisableVertexAttribArray(vattrib));
    }

    void Draw(DrawGLUtils::xlVertexColorAccumulator &va, int type, int enableCapability) override {
        if (va.count == 0) {
            return;
        }
        if (isIntel && enableCapability == GL_POINT_SMOOTH) {
            LOG_GL_ERRORV(glUseProgram(0));
            LOG_GL_ERRORV(glPushMatrix());
            LOG_GL_ERRORV(glLoadMatrixf(glm::value_ptr(*matrix)));
            LOG_GL_ERRORV(glEnable(enableCapability));
            LOG_GL_ERRORV(glEnableClientState(GL_VERTEX_ARRAY));
            LOG_GL_ERRORV(glEnableClientState(GL_COLOR_ARRAY));

            LOG_GL_ERRORV(glColorPointer(4, GL_UNSIGNED_BYTE, 0, &va.colors[0]));
            LOG_GL_ERRORV(glVertexPointer(va.coordsPerVertex, GL_FLOAT, 0, &va.vertices[0]));
            LOG_GL_ERRORV(glDrawArrays(type, 0, va.count));

            LOG_GL_ERRORV(glDisableClientState(GL_VERTEX_ARRAY));
            LOG_GL_ERRORV(glDisableClientState(GL_COLOR_ARRAY));
            LOG_GL_ERRORV(glPopMatrix());
            LOG_GL_ERRORV(glDisable(enableCapability));
            return;
        }
        
        GLuint programId = ProgramIDcolors;
        if (va.coordsPerVertex == 3) {
            programId = ProgramID3Dcolors;
        }
        LOG_GL_ERRORV(glUseProgram(programId));
        SetMVP(programId);

        GLuint vattrib = glGetAttribLocation( ProgramIDcolors, "vertexPosition_modelspace" );
        LOG_GL_ERRORV(glEnableVertexAttribArray(vattrib));
        LOG_GL_ERRORV(glVertexAttribPointer(vattrib, va.coordsPerVertex, GL_FLOAT, false, 0, &va.vertices[0]));

        GLuint cattrib = glGetAttribLocation( ProgramIDcolors, "vertexColor" );
        LOG_GL_ERRORV(glEnableVertexAttribArray(cattrib));
        LOG_GL_ERRORV(glVertexAttribPointer(cattrib, 4, GL_UNSIGNED_BYTE, true, 0, &va.colors[0]));

        float ps = 2.0;
        if (enableCapability != 0) {
            if (enableCapability == GL_POINT_SMOOTH) {
                //LOG_GL_ERRORV(glEnable(enableCapability));
                GLuint cid = glGetUniformLocation(programId, "RenderType");
                glUniform1i(cid, 1);
                ps = CalcSmoothPointParams(programId);
                LOG_GL_ERRORV(glEnable(GL_VERTEX_PROGRAM_POINT_SIZE));
                LOG_GL_ERRORV(glEnable(GL_POINT_SPRITE));
                LOG_GL_ERRORV(glTexEnvi(GL_POINT_SPRITE_ARB,GL_COORD_REPLACE_ARB ,GL_FALSE));
            } else {
                LOG_GL_ERRORV(glEnable(enableCapability));
            }
        }
        LOG_GL_ERRORV(glDrawArrays(type, 0, va.count));
        if (enableCapability > 0) {
            if (enableCapability == GL_POINT_SMOOTH || enableCapability == GL_POINT_SPRITE) {
                GLuint cid = glGetUniformLocation(programId, "RenderType");
                glUniform1i(cid, 0);
                LOG_GL_ERRORV(glPointSize(ps));
                LOG_GL_ERRORV(glDisable(GL_POINT_SPRITE));
                LOG_GL_ERRORV(glDisable(GL_VERTEX_PROGRAM_POINT_SIZE));
            } else {
                glDisable(enableCapability);
            }
        }
        LOG_GL_ERRORV(glDisableVertexAttribArray(cattrib));
        LOG_GL_ERRORV(glDisableVertexAttribArray(vattrib));
    }
    void Draw(DrawGLUtils::xlVertexTextureAccumulator &va, int type, int enableCapability) override {
        if (va.count == 0) {
            return;
        }
        
        GLuint program = ProgramIDtexture;
        if (va.coordsPerVertex == 3) {
            program = ProgramID3Dtexture;
        }
        LOG_GL_ERRORV(glUseProgram(program));
        SetMVP(program);

        GLuint vattrib = glGetAttribLocation(program, "vertexPosition_modelspace" );
        LOG_GL_ERRORV(glEnableVertexAttribArray(vattrib));
        LOG_GL_ERRORV(glVertexAttribPointer(vattrib, va.coordsPerVertex, GL_FLOAT, false, 0, va.vertices));

        GLuint cattrib = glGetAttribLocation(program, "texturePos" );
        LOG_GL_ERRORV(glEnableVertexAttribArray(cattrib));
        LOG_GL_ERRORV(glVertexAttribPointer(cattrib, 2, GL_FLOAT, true, 0, va.tvertices));

        LOG_GL_ERRORV(glUniform1i(glGetUniformLocation(program, "tex"), 0));

        if (va.forceColor) {
            GLuint cid = glGetUniformLocation(program, "RenderType");
            LOG_GL_ERRORV(glUniform1i(cid, 1));
            cid = glGetUniformLocation(program, "inColor");
            LOG_GL_ERRORV(glUniform4f(cid, ((float)va.color.red) / 255.0f,
                                  ((float)va.color.green) / 255.0f,
                                  ((float)va.color.blue) / 255.0f,
                                  ((float)va.color.alpha) / 255.0f));
        } else {
            GLuint cid = glGetUniformLocation(program, "RenderType");
            LOG_GL_ERRORV(glUniform1i(cid, 0));
            cid = glGetUniformLocation(program, "inColor");
            glUniform4f(cid, va.brightness / 100.f, va.brightness / 100.f, va.brightness / 100.f, ((float)va.alpha)/255.0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        }

        LOG_GL_ERRORV(glActiveTexture(GL_TEXTURE0)); //switch to texture image unit 0
        LOG_GL_ERRORV(glBindTexture(GL_TEXTURE_2D, va.id));

        if (enableCapability != 0) {
            glEnable(enableCapability);
        }
        LOG_GL_ERRORV(glDrawArrays(type, 0, va.count));
        if (enableCapability > 0) {
            glDisable(enableCapability);
        }
        LOG_GL_ERRORV(glBindTexture(GL_TEXTURE_2D, 0));
        LOG_GL_ERRORV(glDisableVertexAttribArray(cattrib));
        LOG_GL_ERRORV(glDisableVertexAttribArray(vattrib));
    }
    virtual DrawGLUtils::xl3DMesh *createMesh() override {
        return nullptr;
    }
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

    void DrawTexture(GLuint texture, float x, float y, float x2, float y2,
                     float tx, float ty, float tx2, float ty2) override {

        DrawGLUtils::xlVertexTextureAccumulator va(texture);
        va.PreAlloc(6);

        va.AddVertex(x - 0.4, y, tx, ty);
        va.AddVertex(x - 0.4, y2, tx, ty2);
        va.AddVertex(x2 - 0.4, y2, tx2, ty2);
        va.AddVertex(x2 - 0.4, y2, tx2, ty2);
        va.AddVertex(x2 - 0.4, y, tx2, ty);
        va.AddVertex(x - 0.4, y, tx, ty);

        Draw(va, GL_TRIANGLES, 0);
    }

    void SetMVP(GLuint programId) {
        glUniformMatrix4fv(glGetUniformLocation(programId, "MVP"), 1, GL_FALSE, glm::value_ptr(*matrix));
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
            logger_opengl.error("OpenGL21Cache PopMatrix called but no matrixes in the stack.");
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
    DrawGLUtils::xlVertexColorAccumulator data;

    GLuint ProgramIDcolors;
    GLuint ProgramIDstaticColor;
    GLuint ProgramIDtexture;
    GLuint ProgramID3Dcolors;
    GLuint ProgramID3DstaticColor;
    GLuint ProgramID3Dtexture;

    std::stack<glm::mat4*> matrixStack;

    bool isIntel;

    glm::mat4 *matrix;
};



DrawGLUtils::xlGLCacheInfo *Create21Cache() {
    return new OpenGL21Cache();
}
