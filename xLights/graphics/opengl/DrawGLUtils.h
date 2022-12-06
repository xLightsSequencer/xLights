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

#include <mutex>
#include <list>
#include <vector>
#include <map>
#include "wx/glcanvas.h"
#include "../../Color.h"
#include <glm/mat4x4.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat3x3.hpp>
#include <glm/fwd.hpp>
#include "../xlGraphicsContext.h"

class xlGLCanvas;

namespace DrawGLUtils
{
#define LOG_GL_ERROR() DrawGLUtils::LogGLError(__FILE__, __LINE__)
#define LOG_GL_ERRORV(a) a; DrawGLUtils::LogGLError(__FILE__, __LINE__, #a)
#define IGNORE_GL_ERRORV(a) a; glGetError()

    bool LoadGLFunctions();


    class xl3DMesh {
    protected:
        xl3DMesh() : matrix(1.0f) {}
    public:
        virtual ~xl3DMesh() {}

        void addSurface(const float vert[3][3], const float uv[3][2], const float norms[3][3], uint8_t colors[3][4], GLint imageId);
        void addLine(float v[2][3]);
        void setMatrix(glm::mat4& mat);

    protected:
        void calcProgram();

        std::vector<GLfloat> vertices;
        std::vector<GLubyte> colors;
        std::vector<GLfloat> texCoords;
        std::vector<GLfloat> normals;
        std::vector<GLfloat> wireframe;
        std::vector<GLfloat> lines;

        std::vector<GLint> images;
        glm::mat4 matrix;
        struct PType {
            int startIdx;
            int count;
            GLint image;
            bool transparent;
        };
        std::vector<PType> solidProgramTypes;
        std::vector<PType> transparentProgramTypes;
    };

    class xlVertexAccumulatorBase {
    public:
        float* vertices;
        uint32_t count;
        uint32_t _max;

    protected:
        void DoReset() { count = 0; }
        void DoPreAlloc(unsigned int i) {
            if ((count + i) > _max) {
                DoRealloc(count + i);
                _max = count + i;
            }
        };
        virtual void DoRealloc(int newMax)
        {
            vertices = (float*)realloc(vertices, sizeof(float) * newMax * 3);
        }
        xlVertexAccumulatorBase() : count(0), _max(64)
        {
            vertices = (float*)malloc(sizeof(float) * _max * 3);
        }
        xlVertexAccumulatorBase(unsigned int m) : count(0), _max(m)
        {
            vertices = (float*)malloc(sizeof(float) * _max * 3);
        }

        xlVertexAccumulatorBase(const xlVertexAccumulatorBase& mv)
        {
            count = mv.count;
            _max = mv._max;
            vertices = (float*)malloc(sizeof(float) * _max * 3);
            memcpy(vertices, mv.vertices, count * sizeof(float) * 3);
        }
        xlVertexAccumulatorBase(xlVertexAccumulatorBase&& mv) noexcept
        {
            vertices = mv.vertices;
            mv.vertices = nullptr;
            count = mv.count;
            _max = mv._max;
        }
        virtual ~xlVertexAccumulatorBase()
        {
            if (vertices != nullptr) free(vertices);
        }
    };

    class xlVertexAccumulator : public ::xlVertexAccumulator, public xlVertexAccumulatorBase {
    public:
        xlVertexAccumulator() : ::xlVertexAccumulator(), xlVertexAccumulatorBase() {}
        virtual ~xlVertexAccumulator() {}

        virtual void Reset() override { if (!finalized) xlVertexAccumulatorBase::DoReset(); }
        virtual void PreAlloc(unsigned int i) override { if (!finalized) xlVertexAccumulatorBase::DoPreAlloc(i); }
        virtual uint32_t getCount() override { return count; }


        virtual void Finalize(bool mayChange) override {
            finalized = true;
        }
        virtual void SetVertex(uint32_t vertex, float x, float y, float z) override {
            if (vertex < count) {
                int i = vertex * 3;
                vertices[i++] = x;
                vertices[i++] = y;
                vertices[i] = z;
            }
        }
        virtual void FlushRange(uint32_t start, uint32_t end) override {
        }

        virtual void AddVertex(float x, float y) override {
            AddVertex(x, y, 0);
        }
        virtual void AddVertex(float x, float y, float z)  override {
            if (!finalized) {
                PreAlloc(1);
                int i = count * 3;
                vertices[i++] = x;
                vertices[i++] = y;
                vertices[i] = z;
                count++;
            }
        }
        bool finalized = false;
    };
    class xlVertexColorAccumulator : public ::xlVertexColorAccumulator, public xlVertexAccumulatorBase {
    public:
        xlVertexColorAccumulator() : ::xlVertexColorAccumulator(), xlVertexAccumulatorBase()
        {
            colors = (uint8_t*)malloc(_max * 4);
        }
        xlVertexColorAccumulator(unsigned int m) : xlVertexAccumulatorBase(m)
        {
            colors = (uint8_t*)malloc(_max * 4);
        }
        xlVertexColorAccumulator(xlVertexColorAccumulator&& mv) noexcept : xlVertexAccumulatorBase(mv)
        {
            colors = mv.colors;
            mv.colors = nullptr;
        }
        xlVertexColorAccumulator(const xlVertexColorAccumulator& mv) : xlVertexAccumulatorBase(mv)
        {
            colors = (uint8_t*)malloc(_max * 4);
            memcpy(colors, mv.colors, count * 4);
        }

        virtual ~xlVertexColorAccumulator()
        {
            free(colors);
        }

        virtual void Reset() { if (!finalized) xlVertexAccumulatorBase::DoReset(); }
        virtual void PreAlloc(unsigned int i) { if (!finalized) xlVertexAccumulatorBase::DoPreAlloc(i); }
        virtual uint32_t getCount() { return count; }

        virtual void Finalize(bool mayChangeVertices, bool mayChangeColors) {
            finalized = true;
        }
        virtual void SetVertex(uint32_t vertex, float x, float y, float z, const xlColor &c) {
            if (vertex < count) {
                int i = vertex * 3;
                vertices[i++] = x;
                vertices[i++] = y;
                vertices[i] = z;

                i = vertex * 4;
                colors[i++] = c.Red();
                colors[i++] = c.Green();
                colors[i++] = c.Blue();
                colors[i] = c.Alpha();
            }
        };
        virtual void SetVertex(uint32_t vertex, float x, float y, float z) {
            if (vertex < count) {
                int i = vertex * 3;
                vertices[i++] = x;
                vertices[i++] = y;
                vertices[i] = z;
            }
        };
        virtual void SetVertex(uint32_t vertex, const xlColor &c) {
            if (vertex < count) {
                int i = vertex * 4;
                colors[i++] = c.Red();
                colors[i++] = c.Green();
                colors[i++] = c.Blue();
                colors[i] = c.Alpha();
            }
        };
        virtual void FlushRange(uint32_t start, uint32_t len) {}

        virtual void AddVertex(float x, float y, const xlColor& c) {
            AddVertex(x, y, 0, c, false);
        }
        virtual void AddVertex(float x, float y, float z, const xlColor& c) {
            AddVertex(x, y, z, c, false);
        }
        void AddVertex(float x, float y, float z, const xlColor& c, bool replace) {
            if (replace) {
                for (unsigned int i = 0; i < count; i++) {
                    int base = i * 3;
                    if (vertices[base] == x && vertices[base + 1] == y && vertices[base + 2] == z) {
                        base = i * 4;
                        colors[base] = c.Red();
                        colors[base + 1] = c.Green();
                        colors[base + 2] = c.Blue();
                        colors[base + 3] = c.Alpha();
                        return;
                    }
                }
            }

            PreAlloc(1);
            int i = count * 3;
            vertices[i++] = x;
            vertices[i++] = y;
            vertices[i] = z;

            i = count * 4;
            colors[i++] = c.Red();
            colors[i++] = c.Green();
            colors[i++] = c.Blue();
            colors[i] = c.Alpha();
            count++;
        }



        void AddRotatedCircleAsTriangles(float x, float y, float z, glm::quat rotation, float radius, const xlColor& center, const xlColor& edge, float depth = 0);
        void AddTranslatedCircleAsTriangles(float ox, float oy, float oz, float radius,
            const xlColor& center, const xlColor& edge,
            std::function<void(float& x, float& y, float& z)>&& translateFunction, bool replace = false);


        uint8_t* colors;
    protected:
        bool finalized = false;
        virtual void DoRealloc(int newMax)
        {
            xlVertexAccumulatorBase::DoRealloc(newMax);
            colors = (uint8_t*)realloc(colors, newMax * 4);
        }
    };

    class xlVertexTextureAccumulator : public xlVertexAccumulatorBase, public :: xlVertexTextureAccumulator {
    public:
        xlVertexTextureAccumulator() : xlVertexAccumulatorBase(), id(0), alpha(255), forceColor(false), brightness(100.0)
        {
            tvertices = (float*)malloc(sizeof(float) * _max * 2);
        }
        xlVertexTextureAccumulator(GLuint i) : xlVertexAccumulatorBase(), id(i), alpha(255), forceColor(false), brightness(100.0)
        {
            tvertices = (float*)malloc(sizeof(float) * _max * 2);
        }
        xlVertexTextureAccumulator(GLuint i, uint8_t a) : xlVertexAccumulatorBase(), id(i), alpha(a), forceColor(false), brightness(100.0)
        {
            tvertices = (float*)malloc(sizeof(float) * _max * 2);
        }
        xlVertexTextureAccumulator(xlVertexTextureAccumulator&& mv) noexcept : xlVertexAccumulatorBase(mv)
        {
            tvertices = mv.tvertices;
            mv.tvertices = nullptr;
            id = mv.id;
            alpha = mv.alpha;
            forceColor = mv.forceColor;
            color = mv.color;
            brightness = mv.brightness;
        }
        xlVertexTextureAccumulator(const xlVertexTextureAccumulator& mv) : xlVertexAccumulatorBase(mv)
        {
            id = mv.id;
            alpha = mv.alpha;
            forceColor = mv.forceColor;
            color = mv.color;
            tvertices = (float*)malloc(sizeof(float) * _max * 2);
            memcpy(tvertices, mv.tvertices, count * sizeof(float) * 2);
            brightness = mv.brightness;
        }

        virtual ~xlVertexTextureAccumulator()
        {
            free(tvertices);
        }

        virtual void Reset() override { xlVertexAccumulatorBase::DoReset(); }
        virtual void PreAlloc(unsigned int i) override { xlVertexAccumulatorBase::DoPreAlloc(i); }
        virtual uint32_t getCount() override { return count; }

        virtual void AddVertex(float x, float y, float z, float tx, float ty) override {
            PreAlloc(1);
            int i = count * 3;
            int ti = count * 2;
            vertices[i] = x;
            tvertices[ti] = tx;
            i++;
            ti++;
            vertices[i] = y;
            tvertices[ti] = ty;
            vertices[i + 1] = z;
            count++;
        }
        virtual void AddVertex(float x, float y, float tx, float ty) override {
            AddVertex(x, y, 0, tx, ty);
        }
        GLuint id;
        uint8_t alpha;
        float brightness;
        bool forceColor;
        xlColor color;
        float* tvertices;
    protected:
        virtual void DoRealloc(int newMax) override {
            xlVertexAccumulatorBase::DoRealloc(newMax);
            tvertices = (float*)realloc(tvertices, sizeof(float) * newMax * 2);
        }
    };
    class xlVertexTextAccumulator {
    public:
        xlVertexTextAccumulator() : count(0), color(xlBLACK) {}
        xlVertexTextAccumulator(const xlColor& c) : count(0), color(c) {}
        virtual ~xlVertexTextAccumulator() {}

        void PreAlloc(unsigned int i)
        {
            vertices.reserve(vertices.size() + i * 2);
            text.reserve(text.size() + i);
        };
        void Reset() { count = 0; vertices.clear(); text.clear(); }
        virtual int getCount() { return count; }
        void AddVertex(float x, float y, const std::string& s)
        {
            vertices.push_back(x); vertices.push_back(y);
            text.push_back(s);
            count++;
        }
        std::vector<float> vertices;
        std::vector<std::string> text;
        xlColor color;
        unsigned int count;
    };

    class xlAccumulator : public xlVertexColorAccumulator {
    public:
        xlAccumulator() : xlVertexColorAccumulator(), tvertices(nullptr) { start = 0; }
        xlAccumulator(unsigned int max) : xlVertexColorAccumulator(max), tvertices(nullptr) { start = 0; }
        virtual ~xlAccumulator()
        {
            if (tvertices) {
                free(tvertices);
                tvertices = nullptr;
            }
        }
        virtual void Reset() override
        {
            xlVertexColorAccumulator::Reset();
            start = 0;
            types.clear();
            if (tvertices) {
                free(tvertices);
                tvertices = nullptr;
            }
        }

        virtual void DoRealloc(int newMax) override;

        bool HasMoreVertices() { return count != start; }
        void Finish(int type, int enableCapability = 0, float extra = 1);
        void AddMesh(xl3DMesh* m, bool wireframe, float textureBrightness, bool doTransparents);


        void PreAllocTexture(int i);
        void AddTextureVertex(float x, float y, float tx, float ty)
        {
            AddTextureVertex(x, y, 0, tx, ty);
        }
        void AddTextureVertex(float x, float y, float z, float tx, float ty);

        void FinishTextures(int type, GLuint textureId, uint8_t alpha, float brightness, int enableCapability = 0);
        void FinishTextures(int type, GLuint textureId, const xlColor& color, int enableCapability = 0);

        void Load(const xlVertexColorAccumulator& ca);
        void Load(const xlVertexAccumulator& ca, const xlColor& c);
        void Load(const xlVertexTextureAccumulator& ca, int type, int enableCapability = 0);

        class BufferRangeType {
        public:
            BufferRangeType(int s, int c, int t, int ec, float ex)
            {
                start = s;
                count = c;
                type = t;
                enableCapability = ec;
                extra = ex;
                textureId = -1;
                textureBrightness = 1.0f;
                useTexturePixelColor = false;
                mesh = nullptr;
            }
            BufferRangeType(int s, int c, int t, int ec, GLuint tid, uint8_t alpha, float brightness)
            {
                start = s;
                count = c;
                type = t;
                enableCapability = ec;
                extra = 0.0f;
                textureId = tid;
                textureAlpha = alpha;
                textureBrightness = brightness;
                useTexturePixelColor = false;
                mesh = nullptr;
            }
            BufferRangeType(int s, int c, int t, int ec, GLuint tid, const xlColor& color)
            {
                start = s;
                count = c;
                type = t;
                enableCapability = ec;
                extra = 0.0f;
                textureId = tid;
                textureAlpha = 255;
                textureBrightness = 1.0f;
                useTexturePixelColor = true;
                texturePixelColor = color;
                mesh = nullptr;
            }
            BufferRangeType(xl3DMesh* m, bool wireframe, float brightness, bool doTransparents)
                : mesh(m), meshTransparents(doTransparents)
            {
                start = 0;
                count = 0;
                type = 0;
                enableCapability = 0;
                extra = wireframe ? 1 : 0;
                textureId = -1;
                textureBrightness = brightness;
                textureAlpha = 255;
                useTexturePixelColor = false;
            }
            int start;
            int count;
            int type;
            int enableCapability;
            float extra;
            GLuint textureId;
            uint8_t textureAlpha = 1;
            float textureBrightness = 1.0f;
            bool useTexturePixelColor = false;
            xlColor texturePixelColor;
            xl3DMesh* mesh = nullptr;
            bool meshTransparents = false;
        };
        std::list<BufferRangeType> types;
        float* tvertices;
    private:
        int start;
    };



    class xlGLCacheInfo {
    public:
        xlGLCacheInfo();
        virtual ~xlGLCacheInfo();

        virtual bool IsCoreProfile() { return false; }
        virtual void SetCurrent();
        virtual void Draw(xlVertexAccumulator& va, const xlColor& color, int type, int enableCapability = 0, int start = 0, int count = -1) = 0;
        virtual void Draw(xlVertexColorAccumulator& va, int type, int enableCapability = 0, int start = 0, int count = -1) = 0;
        virtual void Draw(xlVertexTextureAccumulator& va, int type, int enableCapability = 0, int start = 0, int count = -1) = 0;

        virtual void Draw(xlAccumulator& va) = 0;

        virtual xl3DMesh* createMesh() = 0;

        virtual void addVertex(float x, float y, const xlColor& c) = 0;
        virtual unsigned int vertexCount() = 0;
        virtual void flush(int type, int enableCapability = 0) = 0;
        virtual void ensureSize(unsigned int i) = 0;

        virtual void Ortho(int topleft_x, int topleft_y, int bottomright_x, int bottomright_y) = 0;
        virtual void Perspective(int topleft_x, int topleft_y, int bottomright_x, int bottomright_y, int zDepth) = 0;
        virtual void SetCamera(const glm::mat4& view_matrix) = 0;
        virtual void SetModelMatrix(const glm::mat4& model_matrix) = 0;
        virtual void ApplyMatrix(const glm::mat4& model_matrix) = 0;
        virtual void PushMatrix() = 0;
        virtual void PopMatrix() = 0;
        virtual void Translate(float x, float y, float z) = 0;
        virtual void Rotate(float angle, float x, float y, float z) = 0;
        virtual void Scale(float w, float h, float z) = 0;
        virtual void DrawTexture(GLuint texture,
            float x, float y, float x2, float y2,
            float tx = 0.0, float ty = 0.0, float tx2 = 1.0, float ty2 = 1.0) = 0;

        GLuint GetTextureId(int i);
        void SetTextureId(int i, GLuint id) { textures[i] = id; }
        bool HasTextureId(int i);
        void AddTextureToDelete(GLuint i) { deleteTextures.push_back(i); }
    protected:
        std::vector<GLuint> deleteTextures;
        std::map<int, GLuint> textures;
    };

    xlGLCacheInfo* CreateCache();
    void SetCurrentCache(xlGLCacheInfo* cache);
    void DestroyCache(xlGLCacheInfo* cache);

    void SetViewport(xlGLCanvas& win, int x1, int y1, int x2, int y2);
    void SetViewport3D(xlGLCanvas& win, int x1, int y1, int x2, int y2);
    void SetCamera(const glm::mat4& view_matrix);
    void SetModelMatrix(const glm::mat4& model_matrix);
    void ApplyMatrix(const glm::mat4& model_matrix);
    void PushMatrix();
    void PopMatrix();
    void Translate(float x, float y, float z);
    void Rotate(float angle, float x, float y, float z);
    void Scale(float w, float h, float z);

    void LogGLError(const char* file, int line, const char* msg = nullptr);
    void SetupDebugLogging();


    bool IsCoreProfile();
    int NextTextureIdx();

    void Draw(xlAccumulator& va);
    void Draw(xlVertexAccumulator& va, const xlColor& color, int type, int enableCapability = 0, int start = 0, int count = -1);
    void Draw(xlVertexColorAccumulator& va, int type, int enableCapability = 0, int start = 0, int count = -1);
    void Draw(xlVertexTextureAccumulator& va, int type, int enableCapability = 0, int start = 0, int count = -1);
    void Draw(xlVertexTextAccumulator& va, int size, float factor);

    xl3DMesh* createMesh();

    void DrawText(double x, double y, double size, const wxString& text, double factor = 1.0);
    int GetTextWidth(double size, const wxString& text, double factor = 1.0);

    void SetLineWidth(float i);

    void DrawCircle(const xlColor& color, double x, double y, double r, int ctransparency = 0, int etransparency = 0);
    void DrawCircleUnfilled(const xlColor& color, double cx, double cy, double r, float width);

    /* Methods to hold vertex informaton (x, y, color) in an array until End is called where they are all
       draw out to the context in very few calls) */
    void AddVertex(double x, double y, const xlColor& c, int transparency = 0);
    int VertexCount();
    /* Add four vertices to the cache list, all with the given color */
    void PreAlloc(int verts);
    void AddRect(double x1, double y1,
        double x2, double y2,
        const xlColor& c, int transparency = 0);
    void AddRectAsTriangles(double x1, double y1,
        double x2, double y2,
        const xlColor& c, int transparency = 0);
    void End(int type, int enableCapability = 0);

    void DrawLine(const xlColor& color, wxByte alpha, int x1, int y1, int x2, int y2, float width);
    void DrawRectangle(const xlColor& color, bool dashed, int x1, int y1, int x2, int y2);
    void DrawFillRectangle(const xlColor& color, wxByte alpha, int x, int y, int width, int height);

    void CreateOrUpdateTexture(const wxBitmap& bmp48,    //will scale to 64x64 for base
        const wxBitmap& bmp32,
        const wxBitmap& bmp16,
        GLuint* texture);
    void DrawTexture(GLuint texture,
        float x, float y, float x2, float y2,
        float tx = 0.0, float ty = 0.0, float tx2 = 1.0, float ty2 = 1.0);

    void UpdateTexturePixel(GLuint texture, double x, double y, const xlColor& color, bool hasAlpha);

    void DrawCube(double x, double y, double z, double width, const xlColor& color, xlAccumulator& va);
    void DrawSphere(double x, double y, double z, double radius, const xlColor& color, xlAccumulator& va);
    void DrawBoundingBox(xlColor c, glm::vec3& min_pt, glm::vec3& max_pt, glm::mat4& bound_matrix, DrawGLUtils::xlAccumulator& va);
}

