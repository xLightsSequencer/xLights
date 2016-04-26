
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifdef __WXMAC__
#include "OpenGL/gl.h"
#else

#include "wx/wx.h"

#ifdef _MSC_VER
#include "GL/glut.h"
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/glcorearb.h>
#include <GL/gl.h>
#endif
#endif

#include <DrawGLUtils.h>
#include <stack>


class OpenGL21Cache : public DrawGLUtils::xlGLCacheInfo {
public:


    OpenGL21Cache() : matrix(nullptr) {

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
        GLuint FragmentShaderID = CompileShader(GL_FRAGMENT_SHADER,
            "#version 120\n"
            "varying vec4 fragmentColor;\n"
            "void main(){\n"
            "    gl_FragColor = fragmentColor;\n"
            "}\n");

        ProgramIDcolors = LinkProgram(VertexShaderIDc, FragmentShaderID);
        ProgramIDstaticColor = LinkProgram(VertexShaderIDsc, FragmentShaderID);

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
        GLuint FragmentShaderIDtxt = CompileShader(GL_FRAGMENT_SHADER,
            "#version 120\n"
            "varying vec2 textCoord;\n"
            "uniform sampler2D tex;\n"
            "uniform vec4 inColor;\n"
            "void main(){\n"
            "    gl_FragColor = vec4(texture2D(tex, textCoord).rgb, texture2D(tex, textCoord).a * inColor.a);\n"
            "}\n");

        ProgramIDtexture = LinkProgram(VertexShaderIDtx, FragmentShaderIDtxt);
        
        glDeleteShader(VertexShaderIDtx);
        glDeleteShader(FragmentShaderIDtxt);
        glDeleteShader(VertexShaderIDc);
        glDeleteShader(VertexShaderIDsc);
        glDeleteShader(FragmentShaderID);

        
        glUseProgram(ProgramIDcolors);
        MatrixIDc = glGetUniformLocation(ProgramIDcolors, "MVP");
        MatrixIDsc = glGetUniformLocation(ProgramIDstaticColor, "MVP");
        MatrixIDt = glGetUniformLocation(ProgramIDcolors, "MVP");
    }
    
    GLuint CompileShader(GLenum type, const char *text) {
        GLuint shad = glCreateShader(type);
        GLint Result = GL_FALSE;
        int InfoLogLength;

        glShaderSource(shad, 1, &text , NULL);
        glCompileShader(shad);
        
        glGetShaderiv(shad, GL_COMPILE_STATUS, &Result);
        glGetShaderiv(shad, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if ( InfoLogLength > 0 ) {
            std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
            glGetShaderInfoLog(shad, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
            printf("%s\n", &VertexShaderErrorMessage[0]);
        }
        return shad;
    }
    GLuint LinkProgram(GLuint VertexShaderID, GLuint FragmentShaderID) {
        GLint Result = GL_FALSE;
        int InfoLogLength;
        
        GLuint ProgramID = glCreateProgram();
        
        glAttachShader(ProgramID, VertexShaderID);
        glAttachShader(ProgramID, FragmentShaderID);
        
        glLinkProgram(ProgramID);
        glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
        glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if ( InfoLogLength > 0 ){
            std::vector<char> ProgramErrorMessage(InfoLogLength+1);
            glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
            printf("%s\n", &ProgramErrorMessage[0]);
        }
        glDetachShader(ProgramID, VertexShaderID);
        glDetachShader(ProgramID, FragmentShaderID);
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
            glDeleteProgram(ProgramIDcolors);
        }
        if (ProgramIDstaticColor != 0) {
            glDeleteProgram(ProgramIDstaticColor);
        }
        if (ProgramIDtexture != 0) {
            glDeleteProgram(ProgramIDtexture);
        }
    }
    
    void Draw(DrawGLUtils::xlVertexAccumulator &va, const xlColor & color, int type, int enableCapability) override {
        glUseProgram(ProgramIDstaticColor);
        SetMVP(ProgramIDstaticColor);

        GLuint vattrib = glGetAttribLocation( ProgramIDstaticColor, "vertexPosition_modelspace" );
        glEnableVertexAttribArray(vattrib);
        glVertexAttribPointer(vattrib, 2, GL_FLOAT, false, 0, &va.vertices[0]);
        GLuint cid = glGetUniformLocation(ProgramIDstaticColor, "color");
        glUniform4f(cid,
                    ((float)color.Red())/255.0,
                    ((float)color.Green())/255.0,
                    ((float)color.Blue())/255.0,
                    ((float)color.Alpha())/255.0
                    );
        
        if (enableCapability != 0) {
            glEnable(enableCapability);
        }
        glDrawArrays(type, 0, va.count);
        if (enableCapability > 0) {
            glDisable(enableCapability);
        }
        glDisableVertexAttribArray(vattrib);
    }
    void Draw(DrawGLUtils::xlVertexColorAccumulator &va, int type, int enableCapability) override {
        glUseProgram(ProgramIDcolors);
        SetMVP(ProgramIDcolors);
        
        GLuint vattrib = glGetAttribLocation( ProgramIDcolors, "vertexPosition_modelspace" );
        glEnableVertexAttribArray(vattrib);
        glVertexAttribPointer(vattrib, 2, GL_FLOAT, false, 0, &va.vertices[0]);

        GLuint cattrib = glGetAttribLocation( ProgramIDcolors, "vertexColor" );
        glEnableVertexAttribArray(cattrib);
        glVertexAttribPointer(cattrib, 4, GL_UNSIGNED_BYTE, true, 0, &va.colors[0]);
        
        if (enableCapability != 0) {
            glEnable(enableCapability);
        }
        glDrawArrays(type, 0, va.count);
        if (enableCapability > 0) {
            glDisable(enableCapability);
        }
        glDisableVertexAttribArray(cattrib);
        glDisableVertexAttribArray(vattrib);
    }
    void Draw(DrawGLUtils::xlVertexTextureAccumulator &va, int type, int enableCapability) override {
        glUseProgram(ProgramIDtexture);
        SetMVP(ProgramIDtexture);
        
        GLuint vattrib = glGetAttribLocation( ProgramIDtexture, "vertexPosition_modelspace" );
        glEnableVertexAttribArray(vattrib);
        glVertexAttribPointer(vattrib, 2, GL_FLOAT, false, 0, &va.vertices[0]);
        
        GLuint cattrib = glGetAttribLocation( ProgramIDtexture, "texturePos" );
        glEnableVertexAttribArray(cattrib);
        glVertexAttribPointer(cattrib, 2, GL_FLOAT, true, 0, &va.tvertices[0]);
        
        glUniform1i(glGetUniformLocation(ProgramIDtexture, "tex"), 0);

        GLuint cid = glGetUniformLocation(ProgramIDtexture, "inColor");
        glUniform4f(cid, 1.0, 1.0, 1.0, ((float)va.alpha)/255.0);

        
        glActiveTexture(GL_TEXTURE0); //switch to texture image unit 0
        glBindTexture(GL_TEXTURE_2D, va.id);
        
        if (enableCapability != 0) {
            glEnable(enableCapability);
        }
        glDrawArrays(type, 0, va.count);
        if (enableCapability > 0) {
            glDisable(enableCapability);
        }
        glDisableVertexAttribArray(cattrib);
        glDisableVertexAttribArray(vattrib);
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

    void DrawTexture(GLuint* texture, float x, float y, float x2, float y2,
                     float tx, float ty, float tx2, float ty2) override {
        
        DrawGLUtils::xlVertexTextureAccumulator va(*texture);
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
        glUniformMatrix4fv(MatrixIDc, 1, GL_FALSE, glm::value_ptr(*matrix));
    }
    void PushMatrix() override {
        matrixStack.push(matrix);
        matrix = new glm::mat4(*matrix);
    }
    void PopMatrix() override {
        if (!matrixStack.empty()) {
            if (matrix != nullptr) {
                delete matrix;
            }
            matrix = matrixStack.top();
            matrixStack.pop();
            glUniformMatrix4fv(MatrixIDc, 1, GL_FALSE, glm::value_ptr(*matrix));
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
        glUniformMatrix4fv(MatrixIDc, 1, GL_FALSE, glm::value_ptr(*matrix));
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
        glUniformMatrix4fv(MatrixIDc, 1, GL_FALSE, glm::value_ptr(*matrix));
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
        glUniformMatrix4fv(MatrixIDc, 1, GL_FALSE, glm::value_ptr(*matrix));
    }


protected:
    DrawGLUtils::xlVertexColorAccumulator data;

    GLuint ProgramIDcolors;
    GLuint ProgramIDstaticColor;
    GLuint ProgramIDtexture;
    
    GLuint MatrixIDc;
    GLuint MatrixIDsc;
    GLuint MatrixIDt;

    std::stack<glm::mat4*> matrixStack;

    glm::mat4 *matrix;
};



DrawGLUtils::xlGLCacheInfo *Create21Cache() {
    return new OpenGL21Cache();
}
