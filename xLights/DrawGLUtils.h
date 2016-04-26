
#ifndef __XL_DRAWGLUTILS
#define __XL_DRAWGLUTILS

#include <mutex>
#include <vector>
#include "wx/glcanvas.h"
#include "Color.h"

class xlGLCanvas;

namespace DrawGLUtils
{
    class xlVertexAccumulator {
    public:
        xlVertexAccumulator() : count(0) {}
        ~xlVertexAccumulator() {}
        
        void PreAlloc(unsigned int i) {
            vertices.reserve(vertices.size() + i * 2);
        };
        void AddVertex(float x, float y) {
            vertices.push_back(x);
            vertices.push_back(y);
            count++;
        }
        void AddRect(float x1, float y1, float x2, float y2) {
            PreAlloc(8);
            AddVertex(x1, y1);
            AddVertex(x1, y2);
            AddVertex(x2, y1);
            AddVertex(x2, y2);
            AddVertex(x2, y2);
            AddVertex(x2, y2);
            AddVertex(x2, y1);
            AddVertex(x2, y1);
        }

        void Reset() {count = 0; vertices.clear();}
        std::vector<float> vertices;
        unsigned int count;
    };
    class xlVertexColorAccumulator {
    public:
        xlVertexColorAccumulator() : count(0) {}
        ~xlVertexColorAccumulator() {}
        
        void Reset() {count = 0; vertices.clear(); colors.clear();}
        void PreAlloc(unsigned int i) {
            vertices.reserve(vertices.size() + i*2);
            colors.reserve(colors.size() + i*4);
        };
        
        void AddVertex(float x, float y, const xlColor &c) {
            vertices.push_back(x);
            vertices.push_back(y);
            colors.push_back(c.Red());
            colors.push_back(c.Green());
            colors.push_back(c.Blue());
            colors.push_back(c.Alpha());
            count++;
        }
        void AddRect(float x1, float y1, float x2, float y2, const xlColor &c) {
            PreAlloc(6);
            AddVertex(x1, y1, c);
            AddVertex(x1, y2, c);
            AddVertex(x2, y2, c);
            AddVertex(x2, y2, c);
            AddVertex(x2, y1, c);
            AddVertex(x1, y1, c);
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
        std::vector<float> vertices;
        std::vector<uint8_t> colors;
        unsigned int count;
    };
    
    class xlVertexTextureAccumulator {
    public:
        xlVertexTextureAccumulator() : id(0), count(0), alpha(255) {}
        xlVertexTextureAccumulator(GLuint i) : id(i), count(0) {}
        ~xlVertexTextureAccumulator() {}
        
        void PreAlloc(unsigned int i) {
            vertices.reserve(vertices.size() + i*2);
            tvertices.reserve(tvertices.size() + i*2);
        };
        void Reset() {count = 0; vertices.clear(); tvertices.clear();}
        void AddVertex(float x, float y, float tx, float ty) {
            vertices.push_back(x); vertices.push_back(y);
            tvertices.push_back(tx); tvertices.push_back(ty);
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
        std::vector<float> vertices;
        std::vector<float> tvertices;
        unsigned int count;
        uint8_t alpha;
        GLuint id;
    };
    class xlVertexTextAccumulator {
    public:
        xlVertexTextAccumulator() : count(0) {}
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
        unsigned int count;
    };

    class xlGLCacheInfo {
    public:
        xlGLCacheInfo() {};
        virtual ~xlGLCacheInfo() {};
        
        virtual void Draw(xlVertexAccumulator &va, const xlColor & color, int type, int enableCapability = 0) = 0;
        virtual void Draw(xlVertexColorAccumulator &va, int type, int enableCapability = 0) = 0;
        virtual void Draw(xlVertexTextureAccumulator &va, int type, int enableCapability = 0) = 0;
        
        virtual void addVertex(float x, float y, const xlColor &c) = 0;
        virtual unsigned int vertexCount() = 0;
        virtual void flush(int type, int enableCapability = 0) = 0;
        virtual void ensureSize(unsigned int i) = 0;
        
        virtual void Ortho(int topleft_x, int topleft_y, int bottomright_x, int bottomright_y) = 0;
        virtual void PushMatrix() = 0;
        virtual void PopMatrix() = 0;
        virtual void Translate(float x, float y, float z) = 0;
        virtual void Rotate(float angle, float x, float y, float z) = 0;
        virtual void Scale(float w, float h, float z) = 0;
        virtual void DrawTexture(GLuint* texture,
                                 float x, float y, float x2, float y2,
                                 float tx = 0.0, float ty = 0.0, float tx2 = 1.0, float ty2 = 1.0) = 0;
        
    };
    
    xlGLCacheInfo *CreateCache();
    void SetCurrentCache(xlGLCacheInfo *cache);
    void DestroyCache(xlGLCacheInfo *cache);
    
    void SetViewport(xlGLCanvas &win, int x1, int y1, int x2, int y2);
    void PushMatrix();
    void PopMatrix();
    void Translate(float x, float y, float z);
    void Rotate(float angle, float x, float y, float z);
    void Scale(float w, float h, float z);
    
    
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
   
    void Draw(xlVertexAccumulator &va, const xlColor & color, int type, int enableCapability = 0);
    void Draw(xlVertexColorAccumulator &va, int type, int enableCapability = 0);
    void Draw(xlVertexTextureAccumulator &va, int type, int enableCapability = 0);
    void Draw(xlVertexTextAccumulator &va, int size, float factor, int enableCapability = 0);

    
    void DrawText(double x, double y, double size, const wxString &text, double factor = 1.0);
    int GetTextWidth(double size, const wxString &text, double factor = 1.0);
    
    
    void DrawPoint(const xlColor &color, double x, double y);

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

    void DrawHBlendedRectangle(const xlColor &lcolor, const xlColor &rcolor, int x, int y, int x2, int y2, bool flush = true);
    void DrawHBlendedRectangle(const xlColorVector &colors, int x, int y, int x2, int y2, int offset = 0);
    void CreateOrUpdateTexture(const wxBitmap &bmp48,    //will scale to 64x64 for base
                               const wxBitmap &bmp32,
                               const wxBitmap &bmp16,
                               GLuint* texture);
    void DrawTexture(GLuint* texture,
                     float x, float y, float x2, float y2,
                     float tx = 0.0, float ty = 0.0, float tx2 = 1.0, float ty2 = 1.0);
    
    void UpdateTexturePixel(GLuint* texture,double x, double y, xlColor& color, bool hasAlpha);

    void DrawRectangleArray(double y1, double y2, double x, std::vector<double> &xs, std::vector<xlColor> & colors, bool flush = true);

    void DrawDisplayList(float xOffset, float yOffset,
                         float width, float height,
                         const xlDisplayList & dl,
                         xlVertexColorAccumulator &bg);
}

#endif
