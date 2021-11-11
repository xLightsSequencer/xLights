#pragma once

#include <stdint.h>
#include <vector>
#include <mutex>
#include "../Color.h"

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
    virtual xlVertexAccumulator* Flush() { FlushRange(0, getCount()); return this; }


    virtual void AddVertex(float x, float y) {
        AddVertex(x, y, 0);
    }

    //various utilities for adding various shapes
    void AddRectAsLines(float x1, float y1, float x2, float y2);
    void AddRectAsDashedLines(float x1, float y1, float x2, float y2, float dashSize);

    void AddRectAsTriangles(float x1, float y1, float x2, float y2);
    void AddCircleAsLines(float cx, float cy, float r);
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
    virtual xlVertexColorAccumulator* Flush() { FlushRange(0, getCount()); return this; }


    //various utilities for adding various shapes
    void AddRectAsTriangles(float x1, float y1, float x2, float y2, const xlColor &color);
    void AddRectAsTriangles(float x1, float y1, float x2, float y2, float z, const xlColor& c);
    void AddRectAsLines(float x1, float y1, float x2, float y2, const xlColor &color);
    void AddRectAsLines(float x1, float y1, float x2, float y2, float z, const xlColor &color);
    void AddRectAsDashedLines(float x1, float y1, float x2, float y2, float dashSize, const xlColor &color);
    void AddRectAsDashedLines(float x1, float y1, float x2, float y2, float z, float dashSize, const xlColor& c);

    void AddHBlendedRectangleAsTriangles(float x1, float y1, float x2, float y2, xlColor* colorMask, int offset, const xlColorVector& colors);
    void AddHBlendedRectangleAsTriangles(float x1, float y1, float x2, float y2, const xlColor& left, const xlColor& right);

    void AddCircleAsLines(float cx, float cy, float radius, const xlColor &color);
    void AddCircleAsLines(float cx, float cy, float cz, float radius, const xlColor &color);
    void AddCircleAsTriangles(float cx, float cy, float radius, const xlColor& color);
    void AddCircleAsTriangles(float cx, float cy, float radius, const xlColor& center, const xlColor& edge);
    void AddCircleAsTriangles(float cx, float cy, float cz, float radius, const xlColor& color);
    void AddCircleAsTriangles(float cx, float cy, float cz, float radius, const xlColor& center, const xlColor& edge);
};

class xlVertexTextureAccumulator {
public:
    xlVertexTextureAccumulator() {}
    virtual ~xlVertexTextureAccumulator() {}

    virtual void Reset() {}
    virtual void PreAlloc(unsigned int i) {};
    virtual void AddVertex(float x, float y, float z, float tx, float ty) {};
    virtual uint32_t getCount() { return 0; }


    // mark this as ready to be copied to graphics card, after finalize,
    // vertices cannot be added, but if mayChangeV is set, the vertex
    // data can change via SetVertex and then flushed to push the
    // new data to the graphics card.  mayChangeT allows changing the texture vertices
    virtual void Finalize(bool mayChangeV, bool mayChangeT) {}
    virtual void SetVertex(uint32_t vertex, float x, float y, float z, float tx, float ty) {};
    virtual void FlushRange(uint32_t start, uint32_t len) {}
    virtual xlVertexTextureAccumulator* Flush() { FlushRange(0, getCount()); return this; }


    virtual void AddVertex(float x, float y, float tx, float ty) {
        AddVertex(x, y, 0, tx, ty);
    }
    virtual void AddFullTexture(float x, float y, float x2, float y2) {
        AddTexture(x, y, x2, y2, 0, 0, 1, 1);
    }

    virtual void AddTexture(float x, float y, float x2, float y2, float tx, float ty, float tx2, float ty2) {
        PreAlloc(6);
        AddVertex(x, y, 0, tx, ty);
        AddVertex(x, y2, 0, tx, ty2);
        AddVertex(x2, y2, 0, tx2, ty2);
        AddVertex(x2, y2, 0, tx2, ty2);
        AddVertex(x2, y, 0, tx2, ty);
        AddVertex(x, y, 0, tx, ty);
    }
};


class xlTexture {
public:
    xlTexture() {}
    virtual ~xlTexture() {}

    // mark the texture as immutable
    virtual void Finalize() {}

    virtual void UpdatePixel(int x, int y, const xlColor &c, bool copyAlpha) = 0;
};


class xlDisplayListItem {
public:
    xlDisplayListItem() : x(0.0), y(0.0) {};
    xlColor color;
    float x, y;
};
class xlDisplayList : public std::vector<xlDisplayListItem> {
public:
    xlDisplayList() : iconSize(2) {};
    int iconSize;
    mutable std::recursive_mutex lock;

    void LockedClear() {
        std::unique_lock<std::recursive_mutex> locker(lock);
        clear();
    }

    void addToAccumulator(float xOffset, float yOffset,
                          float width, float height,
                          xlVertexColorAccumulator &bg) const;
};
