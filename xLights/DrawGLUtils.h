
#ifndef __XL_DRAWGLUTILS
#define __XL_DRAWGLUTILS

#include <mutex>
#include <list>
#include <vector>
#include <map>
#include "wx/glcanvas.h"
#include "Color.h"
#include <glm/mat4x4.hpp>
#include <glm/mat3x3.hpp>

class xlGLCanvas;

namespace DrawGLUtils
{
    #define LOG_GL_ERROR() DrawGLUtils::LogGLError(__FILE__, __LINE__)
    #define LOG_GL_ERRORV(a) a; DrawGLUtils::LogGLError(__FILE__, __LINE__, #a)
    #define IGNORE_GL_ERRORV(a) a; glGetError()

    bool LoadGLFunctions();
    
    
    class xl3DMesh {
    protected:
        xl3DMesh() : matrix(1.0f)  {}
    public:
        virtual ~xl3DMesh() {}
        
        void addSurface(const float vert[3][3], const float uv[3][2], const float norms[3][3], uint8_t colors[3][4], GLint imageId);
        void addLine(float v[2][3]);
        void setMatrix(glm::mat4& mat);

    protected:
        void calcProgram();
        
        std::vector<GLfloat> vertices;
        std::vector<GLbyte> colors;
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
        };
        std::vector<PType> programTypes;
    };

    class xlVertexAccumulatorBase {
    public:
        virtual void Reset() {count = 0;}
        void PreAlloc(unsigned int i) {
            if ((count + i) > max) {
                DoRealloc(count + i);
                max = count + i;
            }
        };

        float *vertices;
        unsigned int count;
        unsigned int max;
        unsigned int coordsPerVertex;
        
    protected:
        virtual void DoRealloc(int newMax) {
            vertices = (float*)realloc(vertices, sizeof(float)*newMax*coordsPerVertex);
        }
        xlVertexAccumulatorBase() : count(0), max(64), coordsPerVertex(2) {
            vertices = (float*)malloc(sizeof(float)*max*coordsPerVertex);
        }
        xlVertexAccumulatorBase(unsigned int m) : count(0), max(m), coordsPerVertex(2) {
            vertices = (float*)malloc(sizeof(float)*max*coordsPerVertex);
        }

        xlVertexAccumulatorBase(const xlVertexAccumulatorBase &mv) {
            coordsPerVertex = mv.coordsPerVertex;
            count = mv.count;
            max = mv.max;
            vertices = (float*)malloc(sizeof(float)*max*coordsPerVertex);
            memcpy(vertices, mv.vertices, count * sizeof(float) * coordsPerVertex);
        }
        xlVertexAccumulatorBase(xlVertexAccumulatorBase &&mv) {
            coordsPerVertex = mv.coordsPerVertex;
            vertices = mv.vertices;
            mv.vertices = nullptr;
            count = mv.count;
            max = mv.max;
        }
        virtual ~xlVertexAccumulatorBase() {
            if (vertices != nullptr) free(vertices);
        }
        
        void SetCoordsPerVertex(unsigned int c) {
            coordsPerVertex = c;
            DoRealloc(max);
        }
    };

    class xlVertexAccumulator : public xlVertexAccumulatorBase {
    public:
        xlVertexAccumulator() : xlVertexAccumulatorBase() {}

        void AddVertex(float x, float y) {
            AddVertex(x, y, 0);
        }
        void AddVertex(float x, float y, float z) {
            PreAlloc(1);
            int i = count * coordsPerVertex;
            vertices[i++] = x;
            vertices[i++] = y;
            if (coordsPerVertex == 3) {
                vertices[i] = z;
            }
            count++;
        }
        void AddLinesRect(float x1, float y1, float x2, float y2) {
            PreAlloc(8);
            AddVertex(x1, y1);
            AddVertex(x1, y2);
            AddVertex(x1, y2);
            AddVertex(x2, y2);
            AddVertex(x2, y2);
            AddVertex(x2, y1);
            AddVertex(x2, y1);
            AddVertex(x1, y1);
        }
        void AddRect(float x1, float y1, float x2, float y2) {
            PreAlloc(6);
            AddVertex(x1, y1);
            AddVertex(x1, y2);
            AddVertex(x2, y2);
            AddVertex(x2, y2);
            AddVertex(x2, y1);
            AddVertex(x1, y1);
        }
    };
    class xlVertexColorAccumulator : public xlVertexAccumulatorBase {
    public:
        xlVertexColorAccumulator() : xlVertexAccumulatorBase() {
            colors = (uint8_t*)malloc(max*4);
        }
        xlVertexColorAccumulator(unsigned int m) : xlVertexAccumulatorBase(m) {
            colors = (uint8_t*)malloc(max*4);
        }
        xlVertexColorAccumulator(xlVertexColorAccumulator &&mv) : xlVertexAccumulatorBase(mv) {
            colors = mv.colors;
            mv.colors = nullptr;
        }
        xlVertexColorAccumulator(const xlVertexColorAccumulator &mv) : xlVertexAccumulatorBase(mv) {
            colors = (uint8_t*)malloc(max*4);
            memcpy(colors, mv.colors, count * 4);
        }

        virtual ~xlVertexColorAccumulator() {
            free(colors);
        }

        void AddVertex(float x, float y, const xlColor &c) {
            AddVertex(x, y, 0, c);
        }
        void AddVertex(float x, float y, float z, const xlColor &c) {
            PreAlloc(1);
            int i = count*coordsPerVertex;
            vertices[i++] = x;
            vertices[i++] = y;
            if (coordsPerVertex == 3) {
                vertices[i] = z;
            }
            i = count*4;
            colors[i++] = c.Red();
            colors[i++] = c.Green();
            colors[i++] = c.Blue();
            colors[i] = c.Alpha();
            count++;
        }

        
        void AddRect(float x1, float y1, float x2, float y2, const xlColor &c) {
            AddRect(x1, y1, x2, y2, 0, c);
        }
        void AddLinesRect(float x1, float y1, float x2, float y2, const xlColor &c) {
            PreAlloc(8);
            AddVertex(x1, y1, c);
            AddVertex(x1, y2, c);
            AddVertex(x2, y2, c);
            AddVertex(x2, y1, c);

            AddVertex(x1, y2, c);
            AddVertex(x2, y2, c);
            AddVertex(x2, y1, c);
            AddVertex(x1, y1, c);
        }
        void AddDottedLinesRect(float x1, float y1, float x2, float y2, const xlColor &c);
        void AddHBlendedRectangle(const xlColorVector &colors, float x1, float y1,float x2, float y2, xlColor* colorMask, int offset = 0);
        void AddHBlendedRectangle(const xlColor &left, const xlColor &right, float x1, float y1, float x2, float y2);
        void AddTrianglesCircle(float x, float y, float radius, const xlColor &color);
        void AddTrianglesCircle(float x, float y, float radius, const xlColor &center, const xlColor &edge);
        
        
        void AddRect(float x1, float y1, float x2, float y2, float z1, const xlColor &c) {
            PreAlloc(6);
            AddVertex(x1, y1, z1, c);
            AddVertex(x1, y2, z1, c);
            AddVertex(x2, y2, z1, c);
            AddVertex(x2, y2, z1, c);
            AddVertex(x2, y1, z1, c);
            AddVertex(x1, y1, z1, c);
        }
        void AddDottedLinesRect(float x1, float y1, float z1, float x2, float y2, float z2, const xlColor &c);
        void AddTrianglesRotatedCircle(float x, float y, float z, glm::vec3 rotation, float radius, const xlColor &center, const xlColor &edge);
        
        void AddTrianglesCircle(float x, float y, float z, float radius, const xlColor &color);
        void AddTrianglesCircle(float x, float y, float z, float radius, const xlColor &center, const xlColor &edge);
        void AddTrianglesCircle(float ox, float oy, float oz, float radius,
                                const xlColor &center, const xlColor &edge,
                                std::function<void(float &x, float &y, float &z)> &&translateFunction);


        uint8_t *colors;
    protected:
        virtual void DoRealloc(int newMax) {
            xlVertexAccumulatorBase::DoRealloc(newMax);
            colors = (uint8_t*)realloc(colors, newMax*4);
        }
    };

    class xlVertexTextureAccumulator : public xlVertexAccumulatorBase {
    public:
        xlVertexTextureAccumulator() : xlVertexAccumulatorBase(), id(0), alpha(255), forceColor(false), brightness(100.0) {
            tvertices = (float*)malloc(sizeof(float)*max*2);
        }
        xlVertexTextureAccumulator(GLuint i) : xlVertexAccumulatorBase(), id(i), alpha(255), forceColor(false), brightness(100.0) {
            tvertices = (float*)malloc(sizeof(float)*max*2);
        }
        xlVertexTextureAccumulator(GLuint i, uint8_t a) : xlVertexAccumulatorBase(), id(i), alpha(a), forceColor(false), brightness(100.0) {
            tvertices = (float*)malloc(sizeof(float)*max*2);
        }
        xlVertexTextureAccumulator(xlVertexTextureAccumulator &&mv) : xlVertexAccumulatorBase(mv) {
            tvertices = mv.tvertices;
            mv.tvertices = nullptr;
            id = mv.id;
            alpha = mv.alpha;
            forceColor = mv.forceColor;
            color = mv.color;
            brightness = mv.brightness;
        }
        xlVertexTextureAccumulator(const xlVertexTextureAccumulator &mv) : xlVertexAccumulatorBase(mv) {
            id = mv.id;
            alpha = mv.alpha;
            forceColor = mv.forceColor;
            color = mv.color;
            tvertices = (float*)malloc(sizeof(float)*max*2);
            memcpy(tvertices, mv.tvertices, count * sizeof(float) * 2);
            brightness = mv.brightness;
        }

        virtual ~xlVertexTextureAccumulator() {
            free(tvertices);
        }

        void AddVertex(float x, float y, float tx, float ty) {
            PreAlloc(1);
            int i = count*coordsPerVertex;
            vertices[i] = x;
            tvertices[i] = tx;
            i++;
            vertices[i] = y;
            tvertices[i] = ty;
            count++;
        }
        void AddFullTexture(float x, float y, float x2, float y2) {
            PreAlloc(6);
            AddVertex(x, y, 0, 0);
            AddVertex(x, y2, 0, 1);
            AddVertex(x2, y2, 1, 1);
            AddVertex(x2, y2, 1, 1);
            AddVertex(x2, y, 1, 0);
            AddVertex(x, y, 0, 0);
        }
        GLuint id;
        uint8_t alpha;
        float brightness;
        bool forceColor;
        xlColor color;
        float *tvertices;
    protected:
        virtual void DoRealloc(int newMax) {
            xlVertexAccumulatorBase::DoRealloc(newMax);
            tvertices = (float*)realloc(tvertices, sizeof(float)*newMax*2);
        }
    };
    class xlVertexTextAccumulator {
    public:
        xlVertexTextAccumulator() : count(0), color(xlBLACK) {}
        xlVertexTextAccumulator(const xlColor &c) : count(0), color(c) {}
        ~xlVertexTextAccumulator() {}

        void PreAlloc(unsigned int i) {
            vertices.reserve(vertices.size() + i*2);
            text.reserve(text.size() + i);
        };
        void Reset() {count = 0; vertices.clear(); text.clear();}
        void AddVertex(float x, float y, const std::string &s) {
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
        xlAccumulator() : xlVertexColorAccumulator(), tvertices(nullptr) { start = 0;}
        xlAccumulator(unsigned int max) : xlVertexColorAccumulator(max), tvertices(nullptr) { start = 0;}
        virtual ~xlAccumulator() {
            if (tvertices) {
                free(tvertices);
                tvertices = nullptr;
            }
        }
        virtual void Reset() override {
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
        void AddMesh(xl3DMesh *m, bool wireframe, float textureBrightness = 100.0);


        void PreAllocTexture(int i);
        void AddTextureVertex(float x, float y, float tx, float ty) {
            AddTextureVertex(x, y , 0, tx, ty);
        }
        void AddTextureVertex(float x, float y, float z, float tx, float ty);

        void FinishTextures(int type, GLuint textureId, uint8_t alpha, float brightness, int enableCapability = 0);
        void FinishTextures(int type, GLuint textureId, const xlColor &color, int enableCapability = 0);

        void Load(const xlVertexColorAccumulator &ca);
        void Load(const xlVertexAccumulator &ca, const xlColor &c);
        void Load(const xlVertexTextureAccumulator &ca, int type, int enableCapability = 0);

        class BufferRangeType {
        public:
            BufferRangeType(int s, int c, int t, int ec, float ex) {
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
            BufferRangeType(int s, int c, int t, int ec, GLuint tid, uint8_t alpha, float brightness) {
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
            BufferRangeType(int s, int c, int t, int ec, GLuint tid, const xlColor &color) {
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
            BufferRangeType(xl3DMesh *m, bool wireframe, float brightness) : mesh(m) {
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
            uint8_t textureAlpha;
            float textureBrightness;
            bool useTexturePixelColor;
            xlColor texturePixelColor;
            xl3DMesh *mesh;
        };
        std::list<BufferRangeType> types;
        float *tvertices;
    private:
        int start;
    };


	class xlVertex3Accumulator : public xlVertexAccumulator {
	public:
        xlVertex3Accumulator() : xlVertexAccumulator() { SetCoordsPerVertex(3);}
		
	};

    class xlVertex3ColorAccumulator : public xlVertexColorAccumulator {
    public:
        xlVertex3ColorAccumulator() : xlVertexColorAccumulator() {
            SetCoordsPerVertex(3);
        }
        xlVertex3ColorAccumulator(unsigned int m) : xlVertexColorAccumulator(m) {
            SetCoordsPerVertex(3);
        }
        xlVertex3ColorAccumulator(xlVertex3ColorAccumulator &&mv) : xlVertexColorAccumulator(mv) {
        }
        xlVertex3ColorAccumulator(const xlVertex3ColorAccumulator &mv) : xlVertexColorAccumulator(mv) {
        }

        virtual ~xlVertex3ColorAccumulator() {
        }

    protected:
    };

    class xl3Accumulator : public xlAccumulator {
    public:
        xl3Accumulator() : xlAccumulator() { SetCoordsPerVertex(3);}
        xl3Accumulator(unsigned int max) : xlAccumulator(max) { SetCoordsPerVertex(3);}
        virtual ~xl3Accumulator() {}

    private:
    };

    class xlGLCacheInfo {
    public:
        xlGLCacheInfo();
        virtual ~xlGLCacheInfo();

        virtual bool IsCoreProfile() { return false;}
        virtual void SetCurrent();
		virtual void Draw(xlVertexAccumulator &va, const xlColor & color, int type, int enableCapability = 0) = 0;
		virtual void Draw(xlVertexColorAccumulator &va, int type, int enableCapability = 0) = 0;
        virtual void Draw(xlVertexTextureAccumulator &va, int type, int enableCapability = 0) = 0;
        virtual void Draw(xlAccumulator &va) = 0;
        
        virtual xl3DMesh *createMesh() = 0;

        virtual void addVertex(float x, float y, const xlColor &c) = 0;
        virtual unsigned int vertexCount() = 0;
        virtual void flush(int type, int enableCapability = 0) = 0;
        virtual void ensureSize(unsigned int i) = 0;

        virtual void Ortho(int topleft_x, int topleft_y, int bottomright_x, int bottomright_y) = 0;
        virtual void Perspective(int topleft_x, int topleft_y, int bottomright_x, int bottomright_y) = 0;
        virtual void SetCamera(glm::mat4& view_matrix) = 0;
        virtual void PushMatrix() = 0;
        virtual void PopMatrix() = 0;
        virtual void Translate(float x, float y, float z) = 0;
        virtual void Rotate(float angle, float x, float y, float z) = 0;
        virtual void Scale(float w, float h, float z) = 0;
        virtual void DrawTexture(GLuint texture,
                                 float x, float y, float x2, float y2,
                                 float tx = 0.0, float ty = 0.0, float tx2 = 1.0, float ty2 = 1.0) = 0;

        GLuint GetTextureId(int i);
        void SetTextureId(int i, GLuint id) {textures[i] = id;}
        bool HasTextureId(int i);
        void AddTextureToDelete(GLuint i) { deleteTextures.push_back(i); }
        protected:
            std::vector<GLuint> deleteTextures;
            std::map<int, GLuint> textures;
    };

    xlGLCacheInfo *CreateCache();
    void SetCurrentCache(xlGLCacheInfo *cache);
    void DestroyCache(xlGLCacheInfo *cache);

    void SetViewport(xlGLCanvas &win, int x1, int y1, int x2, int y2);
    void SetViewport3D(xlGLCanvas &win, int x1, int y1, int x2, int y2);
    void SetCamera(glm::mat4& view_matrix);
	void PushMatrix();
    void PopMatrix();
    void Translate(float x, float y, float z);
    void Rotate(float angle, float x, float y, float z);
    void Scale(float w, float h, float z);

    void LogGLError(const char *file, int line, const char *msg = nullptr);


    class DisplayListItem {
    public:
        DisplayListItem() : x(0.0), y(0.0) {};
        xlColor color;
        float x, y;
    };
    class xlDisplayList : public std::vector<DisplayListItem> {
    public:
        xlDisplayList() : iconSize(2) {};
        int iconSize;
        mutable std::recursive_mutex lock;

        void LockedClear() {
            std::unique_lock<std::recursive_mutex> locker(lock);
            clear();
        }
    };


    bool IsCoreProfile();
    int NextTextureIdx();

    void Draw(xlAccumulator &va);
    void Draw(xl3Accumulator &va);
    void Draw(xlVertexAccumulator &va, const xlColor & color, int type, int enableCapability = 0);
    void Draw(xlVertexColorAccumulator &va, int type, int enableCapability = 0);
    void Draw(xlVertexTextureAccumulator &va, int type, int enableCapability = 0);
    void Draw(xlVertexTextAccumulator &va, int size, float factor);
	void Draw(xlVertex3Accumulator &va, const xlColor & color, int type, int enableCapability = 0);

    xl3DMesh *createMesh();

    void DrawText(double x, double y, double size, const wxString &text, double factor = 1.0);
    int GetTextWidth(double size, const wxString &text, double factor = 1.0);

    void SetLineWidth(float i);

    void DrawCircle(const xlColor &color, double x, double y, double r, int ctransparency = 0, int etransparency = 0);
    void DrawCircleUnfilled(const xlColor &color, double cx, double cy, double r, float width);

    /* Methods to hold vertex informaton (x, y, color) in an array until End is called where they are all
       draw out to the context in very few calls) */
    void AddVertex(double x, double y, const xlColor &c, int transparency = 0);
    int VertexCount();
    /* Add four vertices to the cache list, all with the given color */
    void PreAlloc(int verts);
    void AddRect(double x1, double y1,
                 double x2, double y2,
                 const xlColor &c, int transparency = 0);
    void AddRectAsTriangles(double x1, double y1,
                            double x2, double y2,
                            const xlColor &c, int transparency = 0);
    void End(int type, int enableCapability = 0);

    void DrawLine(const xlColor &color, wxByte alpha,int x1, int y1,int x2, int y2,float width);
    void DrawRectangle(const xlColor &color, bool dashed, int x1, int y1,int x2, int y2);
    void DrawFillRectangle(const xlColor &color, wxByte alpha, int x, int y,int width, int height);

    void CreateOrUpdateTexture(const wxBitmap &bmp48,    //will scale to 64x64 for base
                               const wxBitmap &bmp32,
                               const wxBitmap &bmp16,
                               GLuint *texture);
    void DrawTexture(GLuint texture,
                     float x, float y, float x2, float y2,
                     float tx = 0.0, float ty = 0.0, float tx2 = 1.0, float ty2 = 1.0);

    void UpdateTexturePixel(GLuint texture,double x, double y, xlColor& color, bool hasAlpha);


    void DrawDisplayList(float xOffset, float yOffset,
                         float width, float height,
                         const xlDisplayList & dl,
                         xlVertexColorAccumulator &bg);

    void DrawCube(double x, double y, double z, double width, const xlColor &color, xl3Accumulator &va);
    void DrawSphere(double x, double y, double z, double radius, const xlColor &color, xl3Accumulator &va);
    void DrawBoundingBox(xlColor c, glm::vec3& min_pt, glm::vec3& max_pt, glm::mat4& bound_matrix, DrawGLUtils::xl3Accumulator &va);
    // 2D version is mainly useful for debugging hit testing
    void DrawBoundingBox(xlColor c, glm::vec3& min_pt, glm::vec3& max_pt, glm::mat4& bound_matrix, DrawGLUtils::xlAccumulator &va);
}

#endif
