#pragma once

#include <vector>
#include <wx/bitmap.h>

#include "../Color.h"

class xlGraphicsContext {
public:

    class xlVertexAccumulator {
    public:
        xlVertexAccumulator() {}
        virtual ~xlVertexAccumulator() {}

        virtual void Reset() {}
        virtual void PreAlloc(unsigned int i) {};
        virtual void AddVertex(float x, float y, float z) {};
        virtual uint32_t getCount() { return 0; }


        // mark this as ready to be copied to graphics card, after finalize,
        // vertices cannot be added, but if mayChange is set, the vertex
        // data can change via SetVertex and then flushed to push the
        // new data to the graphics card
        virtual void Finalize(bool mayChange) {}
        virtual void SetVertex(uint32_t vertex, float x, float y, float z) {};
        virtual void FlushRange(uint32_t start, uint32_t len) {}


        virtual void AddVertex(float x, float y) {
            AddVertex(x, y, 0);
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
        void AddDashedRect(float x1, float y1, float x2, float y2, float dashSize = 8.0f) {
            float xs = std::min(x1, x2);
            float xf = std::max(x1, x2);
            float halfDash = dashSize / 2.0;
            for(float x = xs; x <= xf; x += dashSize) {
                AddVertex(x, y1);
                AddVertex(x + halfDash < xf ? x + halfDash : xf, y1);
                AddVertex(x, y2);
                AddVertex(x + halfDash < xf ? x + halfDash : xf, y2);
            }
            // Line 2
            float ys = std::min(y1, y2);
            float yf = std::max(y1, y2);
            for(int y = ys; y <= yf; y += dashSize) {
                AddVertex(x1, y);
                AddVertex(x1, y + halfDash < yf ? y + halfDash : yf);
                AddVertex(x2, y);
                AddVertex(x2, y + halfDash < yf ? y + halfDash : yf);
            }
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
        void AddCircleAsLines(double cx, double cy, double r) {
            static const int steps = 24;
            static const double inc = 2.0 * 3.14159 / float(steps);
            double d = 0;
            for (int x = 0; x <= steps; x++, d += inc) {
                AddVertex(cos(d) * r + cx, std::sin(d) * r + cy);
            }
        }
    };

    class xlVertexColorAccumulator {
    public:
        xlVertexColorAccumulator() {}
        virtual ~xlVertexColorAccumulator() {}

        virtual void Reset() {}
        virtual void PreAlloc(unsigned int i) {};
        virtual void AddVertex(float x, float y, float z, const xlColor &c) {};
        virtual void AddVertex(float x, float y, const xlColor &c) { AddVertex(x, y, 0.0f, c);};
        virtual uint32_t getCount() { return 0; }


        // mark this as ready to be copied to graphics card, after finalize,
        // vertices cannot be added, but if mayChange is set, the vertex/color
        // data can change via SetVertex and then flushed to push the
        // new data to the graphics card
        virtual void Finalize(bool mayChangeVertices, bool mayChangeColors) {}
        virtual void SetVertex(uint32_t vertex, float x, float y, float z, const xlColor &c)  = 0;
        virtual void SetVertex(uint32_t vertex, float x, float y, float z) = 0;
        virtual void SetVertex(uint32_t vertex, const xlColor &c) = 0;
        virtual void FlushRange(uint32_t start, uint32_t len) {}


        void AddRectAsTriangles(double x1, double y1,
                                double x2, double y2,
                                const xlColor &color) {
            PreAlloc(6);
            AddVertex(x1, y1, color);
            AddVertex(x1, y2, color);
            AddVertex(x2, y2, color);

            AddVertex(x2, y2, color);
            AddVertex(x2, y1, color);
            AddVertex(x1, y1, color);
        }

        void AddCircleAsLines(const xlColor &color, double cx, double cy, double r) {
            static const int steps = 24;
            static const double inc = 2.0 * 3.14159 / float(steps);
            double d = 0;
            for (int x = 0; x <= steps; x++, d += inc) {
                AddVertex(cos(d) * r + cx, std::sin(d) * r + cy, color);
            }
        }
    };

    class xlTexture {
    public:
        xlTexture() {}
        virtual ~xlTexture() {}
    };

    xlGraphicsContext() {}
    virtual ~xlGraphicsContext() {}

    // Setup the Viewport
    virtual void SetViewport(int x1, int y1, int x2, int y2, bool is3D = false) = 0;


    //create various accumulators/buffers/textures
    virtual xlVertexAccumulator *createVertexAccumulator() = 0;
    virtual xlVertexColorAccumulator *createVertexColorAccumulator() = 0;
    virtual xlTexture *createTextureMipMaps(const std::vector<wxBitmap> &bitmaps) = 0;
    virtual xlTexture *createTextureMipMaps(const std::vector<wxImage> &images) = 0;
    virtual xlTexture *createTexture(const wxImage &image) = 0;


    //manipulating the matrices
    virtual void PushMatrix() = 0;
    virtual void PopMatrix() = 0;
    virtual void Translate(float x, float y, float z) = 0;
    virtual void Rotate(float angle, float x, float y, float z) = 0;
    virtual void Scale(float w, float h, float z) = 0;

    //setters for various states
    virtual void enableBlending(bool e = true) = 0;
    virtual void disableBlending() { enableBlending(false); }

    //drawing methods
    virtual void drawLines(xlVertexAccumulator *vac, const xlColor &c) = 0;
    virtual void drawLineStrip(xlVertexAccumulator *vac, const xlColor &c) = 0;
    virtual void drawTriangles(xlVertexAccumulator *vac, const xlColor &c) = 0;
    virtual void drawTriangleStrip(xlVertexAccumulator *vac, const xlColor &c) = 0;

    virtual void drawLines(xlVertexColorAccumulator *vac) = 0;
    virtual void drawLineStrip(xlVertexColorAccumulator *vac) = 0;
    virtual void drawTriangles(xlVertexColorAccumulator *vac) = 0;
    virtual void drawTriangleStrip(xlVertexColorAccumulator *vac) = 0;

    virtual void drawTexture(xlTexture *texture,
                             float x, float y, float x2, float y2,
                             float tx = 0.0, float ty = 0.0, float tx2 = 1.0, float ty2 = 1.0,
                             bool smoothScale = true) = 0;

};
