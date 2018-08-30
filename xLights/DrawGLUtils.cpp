

#include "wx/wx.h"


#ifdef __WXMAC__
 #include "OpenGL/gl.h"
#else
 #include <GL/gl.h>
 #ifdef _MSC_VER
 #include "GL\glext.h"
 #else
 #include <GL/glext.h>
 #endif
#endif

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <wx/bitmap.h>
#include "DrawGLUtils.h"
#include "osxMacUtils.h"
#include <wx/graphics.h>
#include <wx/dcgraph.h>
#include "Image_Loader.h"

#include <map>
#include "Image.h"
const double PI  =3.141592653589793238463;

#include <log4cpp/Category.hh>

static DrawGLUtils::xlGLCacheInfo *currentCache;


#define DO_LOG_GL_MSG(a, ...) logger_opengl.debug(a, ##__VA_ARGS__); printf(a, ##__VA_ARGS__); printf("\n")

void DrawGLUtils::LogGLError(const char * file, int line, const char *msg) {
    static log4cpp::Category &logger_opengl = log4cpp::Category::getInstance(std::string("log_opengl"));
    static log4cpp::Category &logger_opengl_trace = log4cpp::Category::getInstance(std::string("log_opengl_trace"));
    static bool isTraceDebugEnabled = logger_opengl_trace.isDebugEnabled();
    static bool isDebugEnabled = logger_opengl.isDebugEnabled() | isTraceDebugEnabled;
    if (isDebugEnabled) {
        int er = glGetError();
        if (er || isTraceDebugEnabled) {
            const char *f2 = file + strlen(file);
            while (f2 > file && *f2 != '\\' && *f2 != '/') {
                f2--;
            }
            if (*f2 == '\\' || *f2 == '/') {
                f2++;
            }
            if (isTraceDebugEnabled) {
                if (msg) {
                    logger_opengl_trace.debug("%s/%d - %s:   %X", f2, line, msg, er);
                } else {
                    logger_opengl_trace.debug("%s/%d:   %X", f2, line, er);
                }
            }
            if (er) {
                if (msg) {
                    DO_LOG_GL_MSG("%s/%d - %s:   %X", f2, line, msg, er);
                } else {
                    DO_LOG_GL_MSG("%s/%d:   %X", f2, line, er);
                }
            }
        }
    }
}

DrawGLUtils::xlGLCacheInfo::xlGLCacheInfo() {
}
DrawGLUtils::xlGLCacheInfo::~xlGLCacheInfo() {
    if (!deleteTextures.empty()) {
        glDeleteTextures(deleteTextures.size(), &deleteTextures[0]);
        deleteTextures.clear();
    }
    for (auto it = textures.begin(); it != textures.end(); ++it) {
        glDeleteTextures(1, &it->second);
    }
}

GLuint DrawGLUtils::xlGLCacheInfo::GetTextureId(int i) {
    GLuint id = textures[i];
    if (id == 0) {
        LOG_GL_ERRORV(glGenTextures(1, &id));
        textures[i] = id;
    }
    return id;
}
bool DrawGLUtils::xlGLCacheInfo::HasTextureId(int i) {
    return textures.find(i) != textures.end();
}
void DrawGLUtils::xlGLCacheInfo::SetCurrent() {
    if (!deleteTextures.empty()) {
        LOG_GL_ERRORV(glDeleteTextures(deleteTextures.size(), &deleteTextures[0]));
        deleteTextures.clear();
    }
}

class OpenGL11Cache : public DrawGLUtils::xlGLCacheInfo {
public:
    OpenGL11Cache() {
        data.PreAlloc(128);
        LOG_GL_ERRORV(glEnable(GL_COLOR_MATERIAL));
        LOG_GL_ERRORV(glDisable(GL_TEXTURE_2D));
    };
    virtual ~OpenGL11Cache() {
    };

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
    virtual void flush(int type, int enableCapability) override {
        Draw(data, type, enableCapability);
        data.Reset();
    }

    void Draw(DrawGLUtils::xlAccumulator &va) override {
        if (va.count == 0) {
            return;
        }
        bool textsBound = false;
        LOG_GL_ERRORV(glEnableClientState(GL_VERTEX_ARRAY));
        LOG_GL_ERRORV(glEnableClientState(GL_COLOR_ARRAY));

        LOG_GL_ERRORV(glColorPointer(4, GL_UNSIGNED_BYTE, 0, &va.colors[0]));
        LOG_GL_ERRORV(glVertexPointer(va.coordsPerVertex, GL_FLOAT, 0, &va.vertices[0]));

        for (auto it = va.types.begin(); it != va.types.end(); ++it) {
            if (it->textureId != -1) {
                LOG_GL_ERRORV(glEnableClientState(GL_TEXTURE_COORD_ARRAY));
                if (!textsBound) {
                    LOG_GL_ERRORV(glTexCoordPointer(2, GL_FLOAT, 0, va.tvertices));
                    textsBound = true;
                }
                LOG_GL_ERRORV(glBindTexture(GL_TEXTURE_2D, it->textureId));
                LOG_GL_ERRORV(glEnable(GL_TEXTURE_2D));
                LOG_GL_ERRORV(glDisableClientState(GL_COLOR_ARRAY));
                float trans = it->textureAlpha;
                trans /= 255.0f;
                LOG_GL_ERRORV(glColor4f(1.0, 1.0, 1.0, trans));
            }
            if (it->type == GL_POINTS) {
                LOG_GL_ERRORV(glPointSize(it->extra));
            } else if (it->type == GL_LINES || it->type == GL_LINE_LOOP || it->type == GL_LINE_STRIP) {
                DrawGLUtils::SetLineWidth(it->extra);
            }
            if (it->enableCapability != 0) {
                LOG_GL_ERRORV(glEnable(it->enableCapability));
            }
            LOG_GL_ERRORV(glDrawArrays(it->type, it->start, it->count));
            if (it->textureId != -1) {
                LOG_GL_ERRORV(glEnableClientState(GL_COLOR_ARRAY));
                LOG_GL_ERRORV(glDisableClientState(GL_TEXTURE_COORD_ARRAY));
                LOG_GL_ERRORV(glDisable(GL_TEXTURE_2D));
                LOG_GL_ERRORV(glColor4f(1.0, 1.0, 1.0, 1.0));
            }
            if (it->enableCapability != 0) {
                LOG_GL_ERRORV(glDisable(it->enableCapability));
            }
        }

        LOG_GL_ERRORV(glDisableClientState(GL_VERTEX_ARRAY));
        LOG_GL_ERRORV(glDisableClientState(GL_COLOR_ARRAY));
    }

    void Draw(DrawGLUtils::xlVertexAccumulator &va, const xlColor & color, int type, int enableCapability) override {
        if (va.count == 0) {
            return;
        }
        if (enableCapability != 0) {
            LOG_GL_ERRORV(glEnable(enableCapability));
        }
        LOG_GL_ERRORV(glColor4ub(color.Red(), color.Green(), color.Blue(), color.Alpha()));
        LOG_GL_ERRORV(glEnableClientState(GL_VERTEX_ARRAY));
        LOG_GL_ERRORV(glVertexPointer(va.coordsPerVertex, GL_FLOAT, 0, &va.vertices[0]));
        LOG_GL_ERRORV(glDrawArrays(type, 0, va.count));
        LOG_GL_ERRORV(glDisableClientState(GL_VERTEX_ARRAY));
        if (enableCapability != 0) {
            LOG_GL_ERRORV(glDisable(enableCapability));
        }
    }

    void Draw(DrawGLUtils::xlVertexColorAccumulator &va, int type, int enableCapability) override {
        if (va.count == 0) {
            return;
        }
        if (enableCapability != 0) {
            LOG_GL_ERRORV(glEnable(enableCapability));
        }
        LOG_GL_ERRORV(glEnableClientState(GL_VERTEX_ARRAY));
        LOG_GL_ERRORV(glEnableClientState(GL_COLOR_ARRAY));

        LOG_GL_ERRORV(glColorPointer(4, GL_UNSIGNED_BYTE, 0, &va.colors[0]));
        LOG_GL_ERRORV(glVertexPointer(va.coordsPerVertex, GL_FLOAT, 0, &va.vertices[0]));
        LOG_GL_ERRORV(glDrawArrays(type, 0, va.count));

        LOG_GL_ERRORV(glDisableClientState(GL_VERTEX_ARRAY));
        LOG_GL_ERRORV(glDisableClientState(GL_COLOR_ARRAY));
        if (enableCapability != 0) {
            LOG_GL_ERRORV(glDisable(enableCapability));
        }
    }
    void Draw(DrawGLUtils::xlVertexTextureAccumulator &va, int type, int enableCapability) override {
        if (va.count == 0) {
            return;
        }
        if (enableCapability != 0) {
            LOG_GL_ERRORV(glEnable(enableCapability));
        }

        LOG_GL_ERRORV(glBindTexture(GL_TEXTURE_2D, va.id));
        LOG_GL_ERRORV(glEnable(GL_TEXTURE_2D));

        LOG_GL_ERRORV(glEnableClientState(GL_VERTEX_ARRAY));
        LOG_GL_ERRORV(glEnableClientState(GL_TEXTURE_COORD_ARRAY));

        LOG_GL_ERRORV(glTexCoordPointer(2, GL_FLOAT, 0, va.tvertices));
        LOG_GL_ERRORV(glVertexPointer(va.coordsPerVertex, GL_FLOAT, 0, va.vertices));

        if (va.forceColor) {
            int tem, crgb, srgb, orgb, ca, sa, oa;
            glGetTexEnviv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &tem);
            glGetTexEnviv(GL_TEXTURE_ENV, GL_COMBINE_RGB, &crgb);
            glGetTexEnviv(GL_TEXTURE_ENV, GL_SOURCE0_RGB, &srgb);
            glGetTexEnviv(GL_TEXTURE_ENV, GL_OPERAND0_RGB, &orgb);
            glGetTexEnviv(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, &ca);
            glGetTexEnviv(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, &sa);
            glGetTexEnviv(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, &oa);

            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
            glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_TEXTURE);
            glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);

            glAlphaFunc(GL_GREATER, 0.5f);
            LOG_GL_ERRORV(glColor4f(((float)va.color.red) / 255.0f,
                                    ((float)va.color.green) / 255.0f,
                                    ((float)va.color.blue) / 255.0f,
                                    ((float)va.color.alpha) / 255.0f));

            LOG_GL_ERRORV(glDrawArrays(type, 0, va.count));


            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, tem);
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, crgb);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, srgb);
            glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, orgb);
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, ca);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, sa);
            glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, oa);
            glAlphaFunc(GL_ALWAYS, 0.0f);
            LOG_GL_ERRORV(glColor4f(1.0, 1.0, 1.0, 1.0));

        } else if (va.alpha != 255) {
            float intensity = va.alpha;
            intensity /= 255;
            if (intensity > 1.0) {
                intensity = 1.0;
            } else if (intensity < 0) {
                intensity = 0;
            }
            LOG_GL_ERRORV(glColor4f(1.0, 1.0, 1.0, intensity));
            LOG_GL_ERRORV(glDrawArrays(type, 0, va.count));
            LOG_GL_ERRORV(glColor4f(1.0, 1.0, 1.0, 1.0));
        } else {
            LOG_GL_ERRORV(glColor4f(1.0, 1.0, 1.0, 1.0));
            LOG_GL_ERRORV(glDrawArrays(type, 0, va.count));
        }

        LOG_GL_ERRORV(glDisableClientState(GL_VERTEX_ARRAY));
        LOG_GL_ERRORV(glDisableClientState(GL_TEXTURE_COORD_ARRAY));
        if (enableCapability != 0) {
            LOG_GL_ERRORV(glDisable(enableCapability));
        }
        LOG_GL_ERRORV(glDisable(GL_TEXTURE_2D));
    }

    virtual void Ortho(int topleft_x, int topleft_y, int bottomright_x, int bottomright_y) override {
        LOG_GL_ERRORV(glMatrixMode(GL_PROJECTION));
        LOG_GL_ERRORV(glLoadIdentity());

        LOG_GL_ERRORV(glOrtho(topleft_x, bottomright_x, bottomright_y, topleft_y, -1, 1));
        LOG_GL_ERRORV(glMatrixMode(GL_MODELVIEW));
        LOG_GL_ERRORV(glLoadIdentity());
    }

    virtual void Perspective(int topleft_x, int topleft_y, int bottomright_x, int bottomright_y) override {
        LOG_GL_ERRORV(glMatrixMode(GL_PROJECTION));
        LOG_GL_ERRORV(glLoadIdentity());
        glm::mat4 m = glm::perspective(glm::radians(45.0f), (float) (bottomright_x-topleft_x) / (float)(topleft_y-bottomright_y), 1.0f, 10000.0f);
        LOG_GL_ERRORV(glLoadMatrixf(glm::value_ptr(m)));

        LOG_GL_ERRORV(glMatrixMode(GL_MODELVIEW));
        LOG_GL_ERRORV(glLoadIdentity());
    }

    virtual void SetCamera(glm::mat4& view_matrix) override {
        LOG_GL_ERRORV(glMultMatrixf(glm::value_ptr(view_matrix)));
    }

    void PushMatrix() override {
        LOG_GL_ERRORV(glPushMatrix());
    }
    void PopMatrix() override {
        LOG_GL_ERRORV(glPopMatrix());
    }
    void Translate(float x, float y, float z) override {
        LOG_GL_ERRORV(glTranslatef(x, y, z));
    }
    void Rotate(float angle, float x, float y, float z) override {
        LOG_GL_ERRORV(glRotatef(angle, x, y, z));
    }
    void Scale(float w, float h, float z) override {
        LOG_GL_ERRORV(glScalef(w, h, z));
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

protected:
    DrawGLUtils::xlVertexColorAccumulator data;
    unsigned int curCount;
};

DrawGLUtils::xlGLCacheInfo *Create11Cache() {
    return new OpenGL11Cache();
}

void DrawGLUtils::SetCurrentCache(xlGLCacheInfo *c) {
    currentCache = c;
    currentCache->SetCurrent();
}

void DrawGLUtils::DestroyCache(xlGLCacheInfo *cache) {
    if (currentCache == cache) {
        currentCache = nullptr;
    }
    delete cache;
}

bool DrawGLUtils::IsCoreProfile() {
    return currentCache->IsCoreProfile();
}

void DrawGLUtils::SetLineWidth(float i) {
    if (IsCoreProfile()) {
        //Core Profile only allows 1.0 widthlines
        return;
    }
    LOG_GL_ERRORV(glLineWidth(i));
}

void DrawGLUtils::SetViewport(xlGLCanvas &win, int topleft_x, int topleft_y, int bottomright_x, int bottomright_y) {
    int x, y, x2, y2;
    x = topleft_x;
    y = std::min(bottomright_y, topleft_y);
    x2 = bottomright_x;
    y2 = std::max(bottomright_y,topleft_y);

    xlSetRetinaCanvasViewport(win, x,y,x2,y2);
    int w = std::max(x, x2) - std::min(x, x2);
    int h = std::max(y, y2) - std::min(y, y2);
    LOG_GL_ERRORV(glViewport(x,y,w,h));
    currentCache->Ortho(topleft_x, topleft_y, bottomright_x, bottomright_y);
}

void DrawGLUtils::SetViewport3D(xlGLCanvas &win, int topleft_x, int topleft_y, int bottomright_x, int bottomright_y) {
    int x, y, x2, y2;
    x = topleft_x;
    y = bottomright_y;
    x2 = bottomright_x;
    y2 = topleft_y;

    xlSetRetinaCanvasViewport(win, x,y,x2,y2);
    LOG_GL_ERRORV(glViewport(x,y,x2-x,y2-y));
	LOG_GL_ERRORV(glScissor(0, 0, x2 - x, y2 - y));
    currentCache->Perspective(topleft_x, topleft_y, bottomright_x, bottomright_y);
	LOG_GL_ERRORV(glClearColor(0,0,0,0));   // background color
	LOG_GL_ERRORV(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
}

void DrawGLUtils::SetCamera(glm::mat4& view_matrix) {
	currentCache->SetCamera(view_matrix);
}
void DrawGLUtils::PushMatrix() {
    currentCache->PushMatrix();
}

void DrawGLUtils::PopMatrix() {
    currentCache->PopMatrix();
}

void DrawGLUtils::Translate(float x, float y, float z) {
    currentCache->Translate(x, y, z);
}

void DrawGLUtils::Rotate(float angle, float x, float y, float z) {
    currentCache->Rotate(angle, x, y, z);
}

void DrawGLUtils::Scale(float w, float h, float z) {
    currentCache->Scale(w, h, z);
}

void DrawGLUtils::Draw(xlAccumulator &va) {
    currentCache->Draw(va);
}

void DrawGLUtils::Draw(xl3Accumulator &va) {
    currentCache->Draw(va);
}

void DrawGLUtils::Draw(xlVertexAccumulator &va, const xlColor & color, int type, int enableCapability) {
    currentCache->Draw(va, color, type, enableCapability);
}

void DrawGLUtils::Draw(xlVertexColorAccumulator &va, int type, int enableCapability) {
    currentCache->Draw(va, type, enableCapability);
}

void DrawGLUtils::Draw(xlVertexTextureAccumulator &va, int type, int enableCapability) {
    currentCache->Draw(va, type, enableCapability);
}

void DrawGLUtils::Draw(xlVertex3Accumulator &va, const xlColor & color, int type, int enableCapability) {
	currentCache->Draw(va, color, type, enableCapability);
}

void DrawGLUtils::PreAlloc(int verts) {
    currentCache->ensureSize(verts);
}

int DrawGLUtils::VertexCount() {
    return currentCache->vertexCount();
}

void DrawGLUtils::AddVertex(double x, double y, const xlColor &c, int transparency) {
    xlColor color(c);
    if (transparency) {
        double t = 100.0 - transparency;
        t *= 2.55;
        transparency = t;
        color.alpha = transparency > 255 ? 255 : (transparency < 0 ? 0 : transparency);
    }
    currentCache->addVertex(x, y, color);
}

void DrawGLUtils::AddRectAsTriangles(double x1, double y1,
                                     double x2, double y2,
                                     const xlColor &c, int transparency) {
    xlColor color(c);
    if (transparency) {
        double t = 100.0 - transparency;
        t *= 2.55;
        transparency = t;
        color.alpha = transparency > 255 ? 255 : (transparency < 0 ? 0 : transparency);
    }
    PreAlloc(6);
    currentCache->addVertex(x1, y1, color);
    currentCache->addVertex(x1, y2, color);
    currentCache->addVertex(x2, y2, color);

    currentCache->addVertex(x2, y2, color);
    currentCache->addVertex(x2, y1, color);
    currentCache->addVertex(x1, y1, color);
}

void DrawGLUtils::AddRect(double x1, double y1,
                          double x2, double y2,
                          const xlColor &c, int transparency) {
    xlColor color(c);
    if (transparency) {
        double t = 100.0 - transparency;
        t *= 2.55;
        transparency = t;
        color.alpha = transparency > 255 ? 255 : (transparency < 0 ? 0 : transparency);
    }
    currentCache->ensureSize(4);
    currentCache->addVertex(x1, y1, color);
    currentCache->addVertex(x1, y2, color);
    currentCache->addVertex(x2, y2, color);
    currentCache->addVertex(x2, y1, color);
}

void DrawGLUtils::End(int type, int enableCapability) {
    currentCache->flush(type, enableCapability);
}

void DrawGLUtils::DrawCircle(const xlColor &color, double cx, double cy, double r, int ctransparency, int etransparency)
{
    xlColor c(color);

    if (ctransparency) {
        double t = 100.0 - ctransparency;
        t *= 2.56;
        ctransparency = t;
        c.alpha = ctransparency;
    }

    int num_segments = r / 2;
    if (num_segments < 16) {
        num_segments = 16;
    }
    currentCache->ensureSize(num_segments * 2 + 4);
    float theta = 2 * 3.1415926 / float(num_segments);
    float tangetial_factor = tanf(theta);//calculate the tangential factor

    float radial_factor = cosf(theta);//calculate the radial factor

    float x = r;//we start at angle = 0

    float y = 0;

    currentCache->addVertex(cx, cy, c);

    if (etransparency) {
        double t = 100.0 - etransparency;
        t *= 2.56;
        etransparency = t;
        c.alpha = etransparency;
    } else {
        c.alpha = 255;
    }

    for(int ii = 0; ii < num_segments; ii++)
    {
        currentCache->addVertex(x + cx, y + cy, c);

        //calculate the tangential vector
        //remember, the radial vector is (x, y)
        //to get the tangential vector we flip those coordinates and negate one of them

        float tx = -y;
        float ty = x;

        //add the tangential vector

        x += tx * tangetial_factor;
        y += ty * tangetial_factor;

        //correct using the radial factor

        x *= radial_factor;
        y *= radial_factor;
    }
    currentCache->addVertex(x + cx, y + cy, c);
    currentCache->flush(GL_TRIANGLE_FAN);
}

void DrawGLUtils::DrawCircleUnfilled(const xlColor &color, double cx, double cy, double r, float width )
{
    static const double inc = PI / 12;
    static const double max = 2 * PI;
    SetLineWidth(width);
    for(double d = 0; d < max; d += inc) {
        currentCache->addVertex(cos(d) * r + cx, sin(d) * r + cy, color);
    }
    currentCache->flush(GL_LINE_LOOP);
}

void DrawGLUtils::DrawLine(const xlColor &color, wxByte alpha,int x1, int y1, int x2, int y2, float width)
{
    SetLineWidth(width);
    currentCache->ensureSize(4);
    currentCache->addVertex(x1, y1, color);
    currentCache->addVertex(x2, y2, color);
    currentCache->flush(GL_LINES);
}

void DrawGLUtils::DrawRectangle(const xlColor &color, bool dashed, int x1, int y1, int x2, int y2)
{
    if (!dashed)
    {
        currentCache->ensureSize(16);
        currentCache->addVertex(x1, y1, color);
        currentCache->addVertex(x2, y1, color);

        currentCache->addVertex(x2, y1, color);
        currentCache->addVertex(x2, y2 - 0.4f, color);

        currentCache->addVertex(x2, y2 - 0.4f, color);
        currentCache->addVertex(x1, y2 - 0.4f, color);

        currentCache->addVertex(x1, y1, color);
        currentCache->addVertex(x1, y2, color);
        currentCache->flush(GL_LINES);
    }
    else
    {
        // Line 1
        int xs = x1<x2?x1:x2;
        int xf = x1>x2?x1:x2;
        for(int x=xs;x<=xf; x += 8)
        {
            currentCache->addVertex(x, y1, color);
            currentCache->addVertex(x + 4 < xf ? x + 4 : xf, y1, color);
            currentCache->addVertex(x, y2, color);
            currentCache->addVertex(x + 4 < xf ? x + 4 : xf, y2, color);
        }
        // Line 2
        int ys = y1<y2?y1:y2;
        int yf = y1>y2?y1:y2;
        for(int y=ys;y<=yf;y+=8)
        {
            currentCache->addVertex(x1, y, color);
            currentCache->addVertex(x1, y + 4 < yf ? y + 4 : yf, color);
            currentCache->addVertex(x2, y, color);
            currentCache->addVertex(x2, y + 4 < yf ? y + 4 : yf, color);
        }
        currentCache->flush(GL_LINES);
    }
}

void DrawGLUtils::DrawFillRectangle(const xlColor &color, wxByte alpha, int x, int y,int width, int height)
{
    xlColor c(color);
    c.alpha = alpha;
    AddRectAsTriangles(x, y, x + width, y + height, c);
    currentCache->flush(GL_TRIANGLES);
}


void DrawGLUtils::xlAccumulator::Finish(int type, int enableCapability, float extra) {
    if (!types.empty()
        && types.back().textureId == -1
        && types.back().type == type
        && types.back().enableCapability == enableCapability && types.back().extra == extra) {
        //same params, just extent the previous
        types.back().count += count - start;
        start = count;
        return;
    }
    if (count != start) {
        types.push_back(BufferRangeType(start, count - start, type, enableCapability, extra));
        start = count;
    }
}

void DrawGLUtils::xlAccumulator::Load(const DrawGLUtils::xlVertexColorAccumulator& va) {
    PreAlloc(va.count);
    if (va.coordsPerVertex == coordsPerVertex) {
        memcpy(&vertices[count*coordsPerVertex], va.vertices, sizeof(float)*va.count*coordsPerVertex);
    } else {
        for (int x = 0; x < va.count; x++) {
            int cur = (count + x) * coordsPerVertex;
            int curva = x * va.coordsPerVertex;
            vertices[cur++] = va.vertices[curva++];
            vertices[cur++] = va.vertices[curva++];
            if (coordsPerVertex == 3) {
                vertices[cur] = 0.0f;
            }
        }
    }
    memcpy(&colors[count*4], va.colors, va.count*4);
    count += va.count;
}


void DrawGLUtils::xlAccumulator::Load(const DrawGLUtils::xlVertexAccumulator& va, const xlColor &c) {
    PreAlloc(va.count);
    if (va.coordsPerVertex == coordsPerVertex) {
        memcpy(&vertices[count*coordsPerVertex], va.vertices, sizeof(float)*va.count*coordsPerVertex);
    } else  {
        for (int x = 0; x < va.count; x++) {
            int cur = (count + x) * coordsPerVertex;
            int curva = x * va.coordsPerVertex;
            vertices[cur++] = va.vertices[curva++];
            vertices[cur++] = va.vertices[curva++];
            if (coordsPerVertex == 3) {
                vertices[cur] = 0.0f;
            }
        }
    }
    int i = count * 4;
    for (int x = 0; x < va.count; x++) {
        colors[i++] = c.Red();
        colors[i++] = c.Green();
        colors[i++] = c.Blue();
        colors[i++] = c.Alpha();
    }
    count += va.count;
}


void DrawGLUtils::xlAccumulator::Load(const xlVertexTextureAccumulator &va, int type, int enableCapability) {
    PreAllocTexture(va.count);
    if (va.coordsPerVertex == coordsPerVertex) {
        memcpy(&vertices[count*coordsPerVertex], va.vertices, sizeof(float)*va.count*coordsPerVertex);
    } else {
        for (int x = 0; x < va.count; x++) {
            int cur = (count + x) * coordsPerVertex;
            int curva = x * va.coordsPerVertex;
            vertices[cur++] = va.vertices[curva++];
            vertices[cur++] = va.vertices[curva++];
            if (coordsPerVertex == 3) {
                vertices[cur] = 0.0f;
            }
        }
    }
    memcpy(&tvertices[count*coordsPerVertex], va.tvertices, sizeof(float)*va.count*2);
    count += va.count;
    if (va.forceColor) {
        FinishTextures(type, va.id, va.color, enableCapability);
    } else {
        FinishTextures(type, va.id, va.alpha, enableCapability);
    }
}

void DrawGLUtils::xlAccumulator::DoRealloc(int newMax) {
    DrawGLUtils::xlVertexColorAccumulator::DoRealloc(newMax);
    if (tvertices != nullptr) {
        tvertices = (float*)realloc(tvertices, sizeof(float)*newMax*2);
    }
}

void DrawGLUtils::xlAccumulator::PreAllocTexture(int i) {
    PreAlloc(i);
    if (tvertices == nullptr) {
        tvertices = (float*)malloc(sizeof(float)*max * 2);
    }
}

void DrawGLUtils::xlAccumulator::AddTextureVertex(float x, float y, float z, float tx, float ty) {
    PreAllocTexture(1);

    int i = count * coordsPerVertex;
    vertices[i] = x;
    i++;
    vertices[i] = y;
    if (coordsPerVertex == 3) {
        i++;
        vertices[i] = z;
    }
    i = count * 2;
    tvertices[i] = tx;
    i++;
    tvertices[i] = ty;
    count++;
}
void DrawGLUtils::xlAccumulator::FinishTextures(int type, GLuint textureId, const xlColor &color, int enableCapability) {
    types.push_back(BufferRangeType(start, count - start, type, enableCapability, textureId, color));
    start = count;
}

void DrawGLUtils::xlAccumulator::FinishTextures(int type, GLuint textureId, uint8_t alpha, int enableCapability) {
    types.push_back(BufferRangeType(start, count - start, type, enableCapability, textureId, alpha));
    start = count;
}

void DrawGLUtils::xlVertexColorAccumulator::AddDottedLinesRect(float x1, float y1, float x2, float y2, const xlColor &color) {
    // Line 1
    float xs = x1<x2?x1:x2;
    float xf = x1>x2?x1:x2;
    for(float x = xs; x <= xf; x += 8)
    {
        AddVertex(x, y1, color);
        AddVertex(x + 4 < xf ? x + 4 : xf, y1, color);
        AddVertex(x, y2, color);
        AddVertex(x + 4 < xf ? x + 4 : xf, y2, color);
    }
    // Line 2
    int ys = y1<y2?y1:y2;
    int yf = y1>y2?y1:y2;
    for(int y=ys;y<=yf;y+=8)
    {
        AddVertex(x1, y, color);
        AddVertex(x1, y + 4 < yf ? y + 4 : yf, color);
        AddVertex(x2, y, color);
        AddVertex(x2, y + 4 < yf ? y + 4 : yf, color);
    }
}

void DrawGLUtils::xlVertexColorAccumulator::AddDottedLinesRect(float x1, float y1, float z1, float x2, float y2, float z2, const xlColor &color) {
    // Line 1
    float xs = x1<x2?x1:x2;
    float xf = x1>x2?x1:x2;
    for(float x = xs; x <= xf; x += 8)
    {
        AddVertex(x, y1, z1, color);
        AddVertex(x + 4 < xf ? x + 4 : xf, y1, z1, color);
        AddVertex(x, y2, z2, color);
        AddVertex(x + 4 < xf ? x + 4 : xf, y2, z2, color);
    }
    // Line 2
    int ys = y1<y2?y1:y2;
    int yf = y1>y2?y1:y2;
    for(int y=ys;y<=yf;y+=8)
    {
        AddVertex(x1, y, z1, color);
        AddVertex(x1, y + 4 < yf ? y + 4 : yf, z1, color);
        AddVertex(x2, y, z2, color);
        AddVertex(x2, y + 4 < yf ? y + 4 : yf, z2, color);
    }
}

void DrawGLUtils::xlVertexColorAccumulator::AddHBlendedRectangle(const xlColorVector &colors, float x1, float y1,float x2, float y2, xlColor* colorMask, int offset) {
    xlColor start;
    xlColor end;
    int cnt = colors.size();
    if (cnt == 0) {
        return;
    }
    start = colors[0 + offset];
    start.ApplyMask(colorMask);
    if (cnt == 1) {
        AddHBlendedRectangle(start, start, x1, y1, x2, y2);
        return;
    }
    int xl = x1;
    start = colors[0+offset];
    start.ApplyMask(colorMask);
    for (int x = 1+offset; x < cnt; x++) {
        end =  colors[x];
        end.ApplyMask(colorMask);
        int xr = x1 + (x2 - x1) * x / (cnt  - 1);
        if (x == (cnt - 1)) {
            xr = x2;
        }
        AddHBlendedRectangle(start, end, xl, y1, xr, y2);
        start = end;
        xl = xr;
    }
}

void DrawGLUtils::xlVertexColorAccumulator::AddHBlendedRectangle(const xlColor &left, const xlColor &right, float x1, float y1, float x2, float y2) {
    AddVertex(x1, y1, left);
    AddVertex(x1, y2, left);
    AddVertex(x2, y2, right);
    AddVertex(x2, y2, right);
    AddVertex(x2, y1, right);
    AddVertex(x1, y1, left);
}

void DrawGLUtils::xlVertexColorAccumulator::AddTrianglesCircle(float x, float y, float radius, const xlColor &color) {
    AddTrianglesCircle(x, y, radius, color, color);
}

void DrawGLUtils::xlVertexColorAccumulator::AddTrianglesCircle(float x, float y, float z, float radius, const xlColor &color) {
    AddTrianglesCircle(x, y, z, radius, color, color);
}

void DrawGLUtils::xlVertexColorAccumulator::AddTrianglesCircle(float cx, float cy, float radius, const xlColor &center, const xlColor &edge) {
    int num_segments = radius;
    if (num_segments < 16) {
        num_segments = 16;
    }
    PreAlloc(num_segments * 3);
    float theta = 2 * 3.1415926 / float(num_segments);
    float tangetial_factor = tanf(theta);//calculate the tangential factor
    float radial_factor = cosf(theta);//calculate the radial factor

    float x = radius;//we start at angle = 0
    float y = 0;

    for(int ii = 0; ii < num_segments; ii++) {
        AddVertex(x + cx, y + cy, edge);
        //calculate the tangential vector
        //remember, the radial vector is (x, y)
        //to get the tangential vector we flip those coordinates and negate one of them
        float tx = -y;
        float ty = x;

        //add the tangential vector
        x += tx * tangetial_factor;
        y += ty * tangetial_factor;
        x *= radial_factor;
        y *= radial_factor;
        AddVertex(x + cx, y + cy, edge);
        AddVertex(cx, cy, center);
    }
}

void DrawGLUtils::xlVertexColorAccumulator::AddTrianglesCircle(float cx, float cy, float cz, float radius, const xlColor &center, const xlColor &edge) {
    int num_segments = radius;
    if (num_segments < 16) {
        num_segments = 16;
    }
    PreAlloc(num_segments * 4);
    float theta = 2 * 3.1415926 / float(num_segments);
    float tangetial_factor = tanf(theta);//calculate the tangential factor
    float radial_factor = cosf(theta);//calculate the radial factor

    float x = radius;//we start at angle = 0
    float y = 0;

    for(int ii = 0; ii < num_segments; ii++) {
        AddVertex(x + cx, y + cy, cz, edge);
        //calculate the tangential vector
        //remember, the radial vector is (x, y)
        //to get the tangential vector we flip those coordinates and negate one of them
        float tx = -y;
        float ty = x;

        //add the tangential vector
        x += tx * tangetial_factor;
        y += ty * tangetial_factor;
        x *= radial_factor;
        y *= radial_factor;
        AddVertex(x + cx, y + cy, cz, edge);
        AddVertex(cx, cy, cz, center);
    }
}
void DrawGLUtils::xlVertexColorAccumulator::AddTrianglesCircle(float ocx, float ocy, float ocz, float radius,
                                                               const xlColor &center, const xlColor &edge,
                                                               std::function<void(float &x, float &y, float &z)> &&translateFunction) {
    int num_segments = radius;
    if (num_segments < 16) {
        num_segments = 16;
    }
    PreAlloc(num_segments * 4);
    float theta = 2 * 3.1415926 / float(num_segments);
    float tangetial_factor = tanf(theta);//calculate the tangential factor
    float radial_factor = cosf(theta);//calculate the radial factor

    float cx = ocx;
    float cy = ocy;
    float cz = ocz;
    translateFunction(cx, cy, cz);

    //radius is supposed to be in "pixels" to match the size of the smooth points and such, however
    //the transformation may apply scales and such.  Thus, we'll try and figure out a
    //new radius by transforming some stuff and calculating a vector
    float tx = ocx + 1;
    float ty = ocy + 1;
    float tz = ocz + 1;
    translateFunction(tx, ty, tz);
    float vlen = std::sqrt((cx-tx)*(cx-tx) + (cy-ty)*(cy-ty) + (cz-tz)*(cz-tz));
    
    radius = radius/vlen;
    
    float x = radius;//we start at angle = 0
    float y = 0;
    
    
    for(int ii = 0; ii < num_segments; ii++) {
        tx = x + ocx;
        ty = y + ocy;
        tz = ocz;
        translateFunction(tx, ty, tz);
        AddVertex(tx, ty, tz, edge);
        //calculate the tangential vector
        //remember, the radial vector is (x, y)
        //to get the tangential vector we flip those coordinates and negate one of them
        tx = -y;
        ty = x;
        
        //add the tangential vector
        x += tx * tangetial_factor;
        y += ty * tangetial_factor;
        x *= radial_factor;
        y *= radial_factor;
        tx = x + ocx;
        ty = y + ocy;
        tz = ocz;
        translateFunction(tx, ty, tz);
        AddVertex(tx, ty, tz, edge);
        AddVertex(cx, cy, cz, center);
    }
}
void DrawGLUtils::xlVertexColorAccumulator::AddTrianglesRotatedCircle(float cx, float cy, float cz, glm::vec3 rotation, float radius, const xlColor &center, const xlColor &edge)
{
    int num_segments = radius;
    if (num_segments < 16) {
        num_segments = 16;
    }
    PreAlloc(num_segments * 4);
    float theta = 2 * 3.1415926 / float(num_segments);
    float tangetial_factor = tanf(theta);//calculate the tangential factor
    float radial_factor = cosf(theta);//calculate the radial factor

    float x = radius;//we start at angle = 0
    float y = 0;

    glm::mat4 rotationY = glm::rotate(glm::mat4(1.0f), glm::radians(-rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 rotationX = glm::rotate(glm::mat4(1.0f), glm::radians(-rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), glm::vec3(-cx, -cy, -cz));
    glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), glm::vec3(cx, cy, cz));

    for (int ii = 0; ii < num_segments; ii++) {
        glm::vec4 position = glm::vec4(x + cx, y + cy, cz, 1.0f);
        position = translateBack * rotationY * rotationX * translateToOrigin * position;
        AddVertex(position.x, position.y, position.z, edge);
        //calculate the tangential vector
        //remember, the radial vector is (x, y)
        //to get the tangential vector we flip those coordinates and negate one of them
        float tx = -y;
        float ty = x;

        //add the tangential vector
        x += tx * tangetial_factor;
        y += ty * tangetial_factor;
        x *= radial_factor;
        y *= radial_factor;
        position = glm::vec4(x + cx, y + cy, cz, 1.0f);
        position = translateBack * rotationY * rotationX * translateToOrigin * position;
        AddVertex(position.x, position.y, position.z, edge);
        AddVertex(cx, cy, cz, center);
    }
}


void DrawGLUtils::DrawDisplayList(float xOffset, float yOffset,
                                  float width, float height,
                                  const DrawGLUtils::xlDisplayList & dl,
                                  xlVertexColorAccumulator &bg) {
    std::lock_guard<std::recursive_mutex> lock(dl.lock);
    if (dl.empty()) {
        return;
    }
    bg.PreAlloc(dl.size());
    for (int idx = 0; idx < dl.size(); idx++) {
        const DisplayListItem &item = dl[idx];
        bg.AddVertex(xOffset + item.x * width, yOffset + item.y * height, item.color);
    }
}

static void addMipMap(const wxImage &l_Image, int &level) {
    if (l_Image.IsOk() == true)
    {
        LOG_GL_ERRORV(glTexImage2D(GL_TEXTURE_2D, level, GL_RGB, (GLsizei)l_Image.GetWidth(), (GLsizei)l_Image.GetHeight(),
                                   0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)l_Image.GetData()));
        int err = glGetError();
        if (err == GL_NO_ERROR) {
            level++;
        }
        else
        {
            static log4cpp::Category &logger_opengl = log4cpp::Category::getInstance(std::string("log_opengl"));
            logger_opengl.error("Error glTexImage2D: %d", err);
        }
    }
}

void DrawGLUtils::CreateOrUpdateTexture(const wxBitmap &bmp48,
                                        const wxBitmap &bmp32,
                                        const wxBitmap &bmp16,
                                        GLuint *texture)
{
    int level = 0;
    LOG_GL_ERRORV(glGenTextures(1,texture));
    LOG_GL_ERRORV(glBindTexture(GL_TEXTURE_2D, *texture));
    LOG_GL_ERRORV(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    LOG_GL_ERRORV(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST));
    addMipMap(bmp48.ConvertToImage().Rescale(64, 64, wxIMAGE_QUALITY_HIGH), level);
    addMipMap(bmp32.ConvertToImage(), level);
    addMipMap(bmp16.ConvertToImage(), level);
    addMipMap(bmp16.ConvertToImage().Rescale(8, 8, wxIMAGE_QUALITY_HIGH), level);
    addMipMap(bmp16.ConvertToImage().Rescale(4, 4, wxIMAGE_QUALITY_HIGH), level);
    addMipMap(bmp16.ConvertToImage().Rescale(2, 2, wxIMAGE_QUALITY_HIGH), level);
    addMipMap(bmp16.ConvertToImage().Rescale(1, 1, wxIMAGE_QUALITY_HIGH), level);
}

void DrawGLUtils::DrawTexture(GLuint texture,
                         float x, float y, float x2, float y2,
                         float tx, float ty, float tx2, float ty2)
{
    currentCache->DrawTexture(texture, x, y, x2, y2, tx, ty, tx2, ty2);
}

void DrawGLUtils::UpdateTexturePixel(GLuint texture, double x, double y, xlColor& color, bool hasAlpha)
{
    int bytesPerPixel = hasAlpha ?  4 : 3;
    GLubyte *imageData=new GLubyte[bytesPerPixel];
    imageData[0] = color.red;
    imageData[1] = color.green;
    imageData[2] = color.blue;
    if( hasAlpha ) {
        imageData[3] = color.alpha;
    }
    if (!DrawGLUtils::IsCoreProfile()) {
        LOG_GL_ERRORV(glEnable(GL_TEXTURE_2D));
    }
    LOG_GL_ERRORV(glBindTexture(GL_TEXTURE_2D,texture));
    LOG_GL_ERRORV(glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, 1, 1, hasAlpha ?  GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, imageData));
    delete [] imageData;
    if (!DrawGLUtils::IsCoreProfile()) {
        LOG_GL_ERRORV(glDisable(GL_TEXTURE_2D));
    }
}

static int TEXTURE_IDX = 1;
int DrawGLUtils::NextTextureIdx() {
    TEXTURE_IDX++;
    return TEXTURE_IDX;
}

class FontInfoStruct {
public:
    FontInfoStruct(const char * const *d,
                   float mh, float mw, float md,
                   std::vector<float> w)
        : data(d), maxH(mh), maxW(mw), maxD(md), widths(w) {

    }

    const char * const *data;
    float maxH;
    float maxW;
    float maxD;
    std::vector<float> widths;
};
#include "fonts/fonts.h"

#ifndef LINUX
#define USEAA true
#else
#define USEAA false
#endif

#define NUMLINES 6
#define NUMCHARS 16

#define CASEFONT(x) case x: Load(font##x); return true;

class FontTexture {
public:
    FontTexture() { fontIdx = DrawGLUtils::NextTextureIdx();};
    ~FontTexture() {};

    bool Valid() { return currentCache->HasTextureId(fontIdx);}

    bool Create(int size) {
        switch (size) {
            CASEFONT(10);
            CASEFONT(12);
            CASEFONT(14);
            CASEFONT(16);
            CASEFONT(18);
            CASEFONT(20);
            CASEFONT(24);
            CASEFONT(28);
            CASEFONT(40);
            CASEFONT(44);
            CASEFONT(56);
            CASEFONT(88);
            default:
                return false;
            /*default:
                printf("No FONT!!!! %d\n", size);
                ForceCreate(size);
                break;
             */
        }
    }
    void Load(const FontInfoStruct &fi) {
        maxW = fi.maxW;
        maxH = fi.maxH;
        maxD = fi.maxD;
        widths = fi.widths;
        wxImage cimg(fi.data);
        cimg.InitAlpha();
        InitializeImage(cimg);
    }

    void InitializeImage(wxImage &cimg) {
        for (int x = 0; x < cimg.GetWidth(); x++) {
            for (int y = 0; y < cimg.GetHeight(); y++) {
                int alpha = cimg.GetRed(x, y);
                if (alpha) {
                    cimg.SetRGB(x, y, 0, 0, 0);
                    cimg.SetAlpha(x, y, alpha);
                } else {
                    cimg.SetRGB(x, y, 0, 0, 0);
                    cimg.SetAlpha(x, y, 0);
                }
            }
        }

        bool scaledW, scaledH, mAlpha;
        int width, height;
        GLuint id = loadImage(&cimg, width, height, textureWidth, textureHeight, scaledW, scaledH, mAlpha);
        currentCache->SetTextureId(fontIdx, id);
    }
    void ForceCreate(int size) {
        wxString faceName = "Gil Sans";

        bool useAA = USEAA;
        if (size <= 12)  {
            useAA = false;
        }

        wxGraphicsContext *ctx = wxGraphicsContext::Create();
        ctx->SetInterpolationQuality(wxInterpolationQuality::wxINTERPOLATION_BEST);
        ctx->SetCompositionMode(wxCompositionMode::wxCOMPOSITION_OVER);
        if (useAA) {
            ctx->SetAntialiasMode(wxANTIALIAS_DEFAULT);
            ctx->SetFont(ctx->CreateFont(size, faceName, wxFONTFLAG_ANTIALIASED, *wxWHITE));
        } else {
            ctx->SetAntialiasMode(wxANTIALIAS_NONE);
            ctx->SetFont(ctx->CreateFont(size, faceName, wxFONTFLAG_NOT_ANTIALIASED, *wxWHITE));
        }
        maxW = 0;
        maxH = 0;
        maxD = 0;
        for (char c = ' '; c <= '~'; c++) {
            wxString s = c;
            double width, height, desc, el;
            ctx->GetTextExtent(s, &width, &height, &desc, &el);
            maxW = std::max(maxW, (float)width);
            maxH = std::max(maxH, (float)height);
            maxD = std::max(maxD, (float)desc);
        }
        delete ctx;

        maxW += 1.0; // allow room to possibly have to handle some kerning


        int imgh = (maxH + 5) * 6;
        int imgw = (maxW + 5) * 16;
        float power_of_two_that_gives_correct_width=std::log((float)imgw)/std::log(2.0);
        float power_of_two_that_gives_correct_height=std::log((float)imgh)/std::log(2.0);
        imgw=(int)std::pow( 2.0, (int)(std::ceil(power_of_two_that_gives_correct_width)) );
        imgh=(int)std::pow( 2.0, (int)(std::ceil(power_of_two_that_gives_correct_height)) );


        wxImage cimg(imgw, imgh);
        cimg.InitAlpha();

        /*
        ctx = wxGraphicsContext::Create(cimg);
        ctx->SetAntialiasMode(wxANTIALIAS_NONE);
        ctx->SetPen( wxPen(wxColor(92, 92, 92), 0) );
        for (int l = 0; l < NUMLINES; l++) {
            for (int x = 0; x < (maxH + 5); x += 2) {
                wxPoint2DDouble beginPoints(0, x + l * (maxH+5));
                wxPoint2DDouble endPoints((maxW + 5)*NUMCHARS, x + l * (maxH+5));
                ctx->StrokeLines(1, &beginPoints, &endPoints);
            }
        }
        for (int l = 0; l < NUMCHARS; l++) {
            wxPoint2DDouble beginPoints(l * (maxW+5), 0);
            wxPoint2DDouble endPoints(l * (maxW + 5), (NUMLINES + 1) * (maxH+5));
            ctx->StrokeLines(1, &beginPoints, &endPoints);
        }
        delete ctx;
         */


        ctx = wxGraphicsContext::Create(cimg);
        ctx->SetPen( *wxWHITE_PEN );
        ctx->SetInterpolationQuality(wxInterpolationQuality::wxINTERPOLATION_BEST);
        ctx->SetCompositionMode(wxCompositionMode::wxCOMPOSITION_OVER);

        if (useAA) {
            ctx->SetAntialiasMode(wxANTIALIAS_DEFAULT);
            ctx->SetFont(ctx->CreateFont(size, faceName, wxFONTFLAG_ANTIALIASED, *wxWHITE));
        } else {
            ctx->SetAntialiasMode(wxANTIALIAS_NONE);
            ctx->SetFont(ctx->CreateFont(size, faceName, wxFONTFLAG_NOT_ANTIALIASED, *wxWHITE));
        }

        int count = 0;
        int line = 0;
        widths.clear();
        widths.resize('~' - ' ' + 1);
        for (char c = ' '; c <= '~'; c++) {
            wxString s = c;
            ctx->DrawText(s, 2 + (count * (maxW + 5)), 2 + line * (maxH + 5));

            double width, height, desc, el;
            ctx->GetTextExtent(s, &width, &height, &desc, &el);
            widths[c - ' '] = width;
            count++;
            if (count == 16) {
                count = 0;
                line++;
            }
        }
        delete ctx;


        for (int l = 0; l < NUMLINES; l++) {
            for (int c = 0; c < NUMCHARS; c++) {
                bool kerned = false;
                char ch = (l * NUMCHARS + c);
                for (int y = 0; y < (maxH + 3) && !kerned; y++) {
                    int notblack = cimg.GetRed(2 + c * (maxW + 5), y + 1 + l * (maxH + 5));
                    if (notblack) {
                        //the antialiasing has caused part of the pixel to bleed to the left
                        //we need to move the entire thing to the right
                        kerned = true;
                        //printf("Kerned: %c\n", (ch + ' '));
                    }
                }
                if (kerned) {
                    for (int x = (maxW-1); x > 1; x--) {
                        int xpos = c * (maxW + 5) + x;
                        for (int y = 1; y <= (maxH + 4); y++) {
                            int ypos = y  + l * (maxH + 5);
                            int clr = cimg.GetRed(xpos - 1, ypos);
                            cimg.SetRGB(xpos, ypos, clr, clr, clr);
                        }
                    }
                    for (int y = 1; y <= (maxH + 4); y++) {
                        int ypos = y  + l * (maxH + 5);
                        cimg.SetRGB(c * (maxW + 5) + 1, ypos, 0, 0, 0);
                    }
                    widths[ch] += 1.0;
                }
                /*
                if ((ch + ' ') == 'j') {
                    printf("%f  %f\n", c * (maxW + 5) + 1, widths[ch]);
                }
                 */
            }
        }


         //used to output data that can be used to static generation above
        /*
        wxString fn = wxString::Format("/tmp/font_%d.png", size);
        cimg.SaveFile(fn, wxBITMAP_TYPE_PNG);
        printf("#include \"font_%d.xpm\"\n", size);
        if (size == 10) {
            widths['b' - ' '] += 0.5;
            widths['o' - ' '] += 0.5;
            widths['p' - ' '] += 0.5;
            widths['w' - ' '] += 0.5;
            widths['x' - ' '] += 0.75;
            widths['y' - ' '] += 0.75;
            widths['m' - ' '] -= 0.75;
            widths['A' - ' '] += 0.75;
            widths['C' - ' '] -= 0.5;
            widths['W' - ' '] += 0.5;
            widths['P' - ' '] += 0.5;
            widths['|' - ' '] -= 0.5;
        }
        if (size == 12) {
            widths['p' - ' '] += 0.5;
            widths['w' - ' '] += 0.75;
            widths['m' - ' '] -= 0.5;
            widths['e' - ' '] -= 0.5;
            widths['t' - ' '] += 0.5;
        }
        printf("static FontInfoStruct font%d(font_%d_xpm, %ff, %ff, %ff, {\n", size, size, maxH, maxW, maxD);
        printf("%ff", widths[0]);
        for (int x = 1; x < widths.size(); x++) {
            printf(", %ff", widths[x]);
        }
        printf("});\n");
        */


        InitializeImage(cimg);
    }

    void Populate(float x, float yBase, const std::string &text, float factor, DrawGLUtils::xlVertexTextureAccumulator &va) {
        va.PreAlloc(6 * text.size());
        va.id = currentCache->GetTextureId(fontIdx);
        //DrawGLUtils::DrawLine(xlBLUE, 255, x, yBase, x+3, yBase, 1);
        //DrawGLUtils::DrawLine(xlBLUE, 255, x, yBase - (float(maxH) + 2) / factor, x+3, yBase - (float(maxH) + 2) / factor, 1);
        for (int idx = 0; idx < text.size(); idx++) {
            char ch = text[idx];
            if (ch < ' ' && ch > '~') {
                ch = '?';
            }
            if (ch == ' ') {
                x += widths[0] / factor;
                continue;
            }
            int linei = ch;
            linei -= ' ';
            linei /= 16.0;
            float line = linei;

            float pos = ch - ' ';
            pos -= linei * 16.0;

            float tx = 1 + (pos * (maxW + 5));
            float tx2 = tx + widths[ch - ' '];

            float x2 = x + float(widths[ch - ' ']) / factor;

            float ty2 = textureHeight - 3 - (line * (maxH + 5));
            float ty = ty2 - maxH;

            float y = yBase;
            float y2 = yBase - float(maxH) / factor;
            /*
            if (ch == 'p') {
                printf("%c   %f %f    %f %f     %f %f\n", ch, x, x2, tx, tx2,  (x2-x), (tx2-tx));
            }
             */

            tx /= textureWidth;
            tx2 /= textureWidth;

            ty2 /= textureHeight;
            ty /= textureHeight;

            //samples need to be from the MIDDLE of the pixel
            ty -= 0.5 / textureHeight;
            ty2 += 0.75 / textureHeight;

            tx += 0.5 / textureWidth;
            tx2 += 1.0 / textureWidth;

            y += 0.25/factor;
            y2 -= 0.75/factor;
            x -= 0.25/factor;
            x2 += 0.25/factor;

            /*
            if (ch == '1') {
                DrawGLUtils::DrawFillRectangle(xlWHITE, 255,x,y, image.textureWidth / factor, image.textureHeight / factor);

                ty = 0.0;
                ty2 = 1.0 - ty;
                tx = 0.0;
                tx2 = 1.0 - tx;
                y = yBase + (float)(image.textureHeight)/factor;
                x2 = x + (float)(image.textureWidth)/ factor;
                y2 = yBase;
            }
            */
            //printf("%c   %f %f    %f %f\n", ch, tx, tx2, ty, ty2);

            va.AddVertex(x, y, tx, ty);
            va.AddVertex(x, y2, tx, ty2);
            va.AddVertex(x2, y2, tx2, ty2);
            va.AddVertex(x2, y2, tx2, ty2);
            va.AddVertex(x2, y, tx2, ty);
            va.AddVertex(x, y, tx, ty);

            x += widths[ch - ' '] / factor;
        }

    }

    void Draw(float x, float yBase, const std::string &text, float factor) {
        LOG_GL_ERRORV(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        DrawGLUtils::xlVertexTextureAccumulator va(currentCache->GetTextureId(fontIdx));
        Populate(x, yBase, text, factor, va);
        DrawGLUtils::Draw(va, GL_TRIANGLES, GL_BLEND);
    }

    float TextWidth(const std::string &text, float factor) {
        float w = 0;
        for (int idx = 0; idx < text.size(); idx++) {
            char ch = text[idx];
            if (ch < ' ' && ch > '~') {
                ch = '?';
            }
            w += widths[ch - ' '];
        }
        return w / factor;
    }

    float maxD, maxW, maxH;
    std::vector<float> widths;
    int fontIdx;

    int textureWidth, textureHeight;
};

static std::map<unsigned int, FontTexture> FONTS;

static int FindFont(double size, double factor) {
    int tsize = size * factor;
    while (!FONTS[tsize].Valid() && tsize > 0) {
        if (FONTS[tsize].Create(tsize)) {
            return tsize;
        }
        tsize--;
    }
    tsize = size * factor;
    while (!FONTS[tsize].Valid() && tsize <= 88) {
        if (FONTS[tsize].Create(tsize)) {
            return tsize;
        }
        tsize++;
    }

    if (!FONTS[tsize].Valid())
    {
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.warn("DrawGLUtilities had trouble creating font size %d scale %f ... this could end badly.", size, factor);
    }

    return tsize;
}

void DrawGLUtils::DrawText(double x, double y, double size, const wxString &text, double factor) {
    int tsize = FindFont(size, factor);
    if (!FONTS[tsize].Valid()) {
        FONTS[tsize].Create(tsize);
    }
    FONTS[tsize].Draw(x, y, text.ToStdString(), factor);
}
int DrawGLUtils::GetTextWidth(double size, const wxString &text, double factor) {
    int tsize = FindFont(size, factor);
    if (!FONTS[tsize].Valid()) {
        FONTS[tsize].Create(tsize);
    }
    return FONTS[tsize].TextWidth(text.ToStdString(), factor);
}

void DrawGLUtils::Draw(DrawGLUtils::xlVertexTextAccumulator &va, int size, float factor) {
    int tsize = FindFont(size, factor);
    if (!FONTS[tsize].Valid()) {
        FONTS[tsize].Create(tsize);
    }
    DrawGLUtils::xlVertexTextureAccumulator vat(currentCache->GetTextureId(FONTS[tsize].fontIdx));
    if (va.color != xlBLACK) {
        vat.forceColor = true;
        vat.color = va.color;
    }
    for (int x = 0; x < va.count; x++) {
        FONTS[tsize].Populate(va.vertices[x*2], va.vertices[x*2 + 1], va.text[x], factor, vat);
    }
    LOG_GL_ERRORV(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    Draw(vat, GL_TRIANGLES, GL_BLEND);
}

void DrawGLUtils::DrawSphere(double x, double y, double z, double radius, const xlColor &color, xl3Accumulator &va)
{
    // FIXME:  draw a square until I get a good sphere routine
    DrawCube(x, y, z, radius*2, color, va);
}

void DrawGLUtils::DrawCube(double x, double y, double z, double width, const xlColor &color, xl3Accumulator &va)
{
    double halfwidth = width / 2.0;

    // front
    va.AddVertex(x - halfwidth, y + halfwidth, z + halfwidth, color);
    va.AddVertex(x + halfwidth, y + halfwidth, z + halfwidth, color);
    va.AddVertex(x + halfwidth, y - halfwidth, z + halfwidth, color);

    va.AddVertex(x - halfwidth, y + halfwidth, z + halfwidth, color);
    va.AddVertex(x - halfwidth, y - halfwidth, z + halfwidth, color);
    va.AddVertex(x + halfwidth, y - halfwidth, z + halfwidth, color);

    // back
    va.AddVertex(x - halfwidth, y + halfwidth, z - halfwidth, color);
    va.AddVertex(x + halfwidth, y + halfwidth, z - halfwidth, color);
    va.AddVertex(x + halfwidth, y - halfwidth, z - halfwidth, color);

    va.AddVertex(x - halfwidth, y + halfwidth, z - halfwidth, color);
    va.AddVertex(x - halfwidth, y - halfwidth, z - halfwidth, color);
    va.AddVertex(x + halfwidth, y - halfwidth, z - halfwidth, color);

    // left side
    va.AddVertex(x - halfwidth, y + halfwidth, z + halfwidth, color);
    va.AddVertex(x - halfwidth, y - halfwidth, z + halfwidth, color);
    va.AddVertex(x - halfwidth, y - halfwidth, z - halfwidth, color);

    va.AddVertex(x - halfwidth, y + halfwidth, z + halfwidth, color);
    va.AddVertex(x - halfwidth, y + halfwidth, z - halfwidth, color);
    va.AddVertex(x - halfwidth, y - halfwidth, z - halfwidth, color);

    // right side
    va.AddVertex(x + halfwidth, y + halfwidth, z + halfwidth, color);
    va.AddVertex(x + halfwidth, y - halfwidth, z + halfwidth, color);
    va.AddVertex(x + halfwidth, y - halfwidth, z - halfwidth, color);

    va.AddVertex(x + halfwidth, y + halfwidth, z + halfwidth, color);
    va.AddVertex(x + halfwidth, y + halfwidth, z - halfwidth, color);
    va.AddVertex(x + halfwidth, y - halfwidth, z - halfwidth, color);

    // top side
    va.AddVertex(x - halfwidth, y + halfwidth, z + halfwidth, color);
    va.AddVertex(x + halfwidth, y + halfwidth, z + halfwidth, color);
    va.AddVertex(x - halfwidth, y + halfwidth, z - halfwidth, color);

    va.AddVertex(x - halfwidth, y + halfwidth, z + halfwidth, color);
    va.AddVertex(x + halfwidth, y + halfwidth, z + halfwidth, color);
    va.AddVertex(x + halfwidth, y + halfwidth, z - halfwidth, color);

    // bottom side
    va.AddVertex(x - halfwidth, y - halfwidth, z + halfwidth, color);
    va.AddVertex(x + halfwidth, y - halfwidth, z + halfwidth, color);
    va.AddVertex(x - halfwidth, y - halfwidth, z - halfwidth, color);

    va.AddVertex(x - halfwidth, y - halfwidth, z + halfwidth, color);
    va.AddVertex(x + halfwidth, y - halfwidth, z + halfwidth, color);
    va.AddVertex(x + halfwidth, y - halfwidth, z - halfwidth, color);

    va.Finish(GL_TRIANGLES);
}

void DrawGLUtils::DrawBoundingBox(glm::vec3& min_pt, glm::vec3& max_pt, glm::mat4& bound_matrix, DrawGLUtils::xl3Accumulator &va)
{
    glm::vec4 c1(min_pt.x, max_pt.y, min_pt.z, 1.0f);
    glm::vec4 c2(max_pt.x, max_pt.y, min_pt.z, 1.0f);
    glm::vec4 c3(max_pt.x, min_pt.y, min_pt.z, 1.0f);
    glm::vec4 c4(min_pt.x, min_pt.y, min_pt.z, 1.0f);
    glm::vec4 c5(min_pt.x, max_pt.y, max_pt.z, 1.0f);
    glm::vec4 c6(max_pt.x, max_pt.y, max_pt.z, 1.0f);
    glm::vec4 c7(max_pt.x, min_pt.y, max_pt.z, 1.0f);
    glm::vec4 c8(min_pt.x, min_pt.y, max_pt.z, 1.0f);

    c1 = bound_matrix * c1;
    c2 = bound_matrix * c2;
    c3 = bound_matrix * c3;
    c4 = bound_matrix * c4;
    c5 = bound_matrix * c5;
    c6 = bound_matrix * c6;
    c7 = bound_matrix * c7;
    c8 = bound_matrix * c8;

    LOG_GL_ERRORV(glHint(GL_LINE_SMOOTH_HINT, GL_NICEST));
    va.AddVertex(c1.x, c1.y, c1.z, xlWHITE);
    va.AddVertex(c2.x, c2.y, c2.z, xlWHITE);
    va.AddVertex(c2.x, c2.y, c2.z, xlWHITE);
    va.AddVertex(c3.x, c3.y, c3.z, xlWHITE);
    va.AddVertex(c3.x, c3.y, c3.z, xlWHITE);
    va.AddVertex(c4.x, c4.y, c4.z, xlWHITE);
    va.AddVertex(c4.x, c4.y, c4.z, xlWHITE);
    va.AddVertex(c1.x, c1.y, c1.z, xlWHITE);

    va.AddVertex(c5.x, c5.y, c5.z, xlWHITE);
    va.AddVertex(c6.x, c6.y, c6.z, xlWHITE);
    va.AddVertex(c6.x, c6.y, c6.z, xlWHITE);
    va.AddVertex(c7.x, c7.y, c7.z, xlWHITE);
    va.AddVertex(c7.x, c7.y, c7.z, xlWHITE);
    va.AddVertex(c8.x, c8.y, c8.z, xlWHITE);
    va.AddVertex(c8.x, c8.y, c8.z, xlWHITE);
    va.AddVertex(c5.x, c5.y, c5.z, xlWHITE);

    va.AddVertex(c1.x, c1.y, c1.z, xlWHITE);
    va.AddVertex(c5.x, c5.y, c5.z, xlWHITE);
    va.AddVertex(c2.x, c2.y, c2.z, xlWHITE);
    va.AddVertex(c6.x, c6.y, c6.z, xlWHITE);
    va.AddVertex(c3.x, c3.y, c3.z, xlWHITE);
    va.AddVertex(c7.x, c7.y, c7.z, xlWHITE);
    va.AddVertex(c4.x, c4.y, c4.z, xlWHITE);
    va.AddVertex(c8.x, c8.y, c8.z, xlWHITE);
    va.Finish(GL_LINES, GL_LINE_SMOOTH, 1.7f);
}

void DrawGLUtils::DrawBoundingBox(glm::vec3& min_pt, glm::vec3& max_pt, glm::mat4& bound_matrix, DrawGLUtils::xlAccumulator &va)
{
    glm::vec4 c1(min_pt.x, max_pt.y, 1.0f, 1.0f);
    glm::vec4 c2(max_pt.x, max_pt.y, 1.0f, 1.0f);
    glm::vec4 c3(max_pt.x, min_pt.y, 1.0f, 1.0f);
    glm::vec4 c4(min_pt.x, min_pt.y, 1.0f, 1.0f);

    c1 = bound_matrix * c1;
    c2 = bound_matrix * c2;
    c3 = bound_matrix * c3;
    c4 = bound_matrix * c4;

    LOG_GL_ERRORV(glHint(GL_LINE_SMOOTH_HINT, GL_NICEST));
    va.AddVertex(c1.x, c1.y, xlWHITE);
    va.AddVertex(c2.x, c2.y, xlWHITE);
    va.AddVertex(c2.x, c2.y, xlWHITE);
    va.AddVertex(c3.x, c3.y, xlWHITE);
    va.AddVertex(c3.x, c3.y, xlWHITE);
    va.AddVertex(c4.x, c4.y, xlWHITE);
    va.AddVertex(c4.x, c4.y, xlWHITE);
    va.AddVertex(c1.x, c1.y, xlWHITE);

    va.Finish(GL_LINES, GL_LINE_SMOOTH, 1.7f);
}
