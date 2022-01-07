#pragma once

#include <stdint.h>
#include <list>
#include <vector>
#include <mutex>
#include <functional>
#include "../Color.h"

class xlGraphicsContext;

class xlVertexAccumulator {
public:
    xlVertexAccumulator() {}
    virtual ~xlVertexAccumulator() {}
    
    xlVertexAccumulator *SetName(const std::string &n) { name = n; return this; }
    const std::string &GetName() const { return name; }

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
    
protected:
    std::string name;
};

class xlVertexColorAccumulator {
public:
    xlVertexColorAccumulator() {}
    virtual ~xlVertexColorAccumulator() {}
    
    xlVertexColorAccumulator *SetName(const std::string &n) { name = n; return this; }
    const std::string &GetName() const { return name; }

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
    void AddCircleAsTriangles(float cx, float cy, float cz, float radius, const xlColor& center, const xlColor& edge, float depthRatio, int numSegments = -1);

    void AddCubeAsTriangles(float x, float y, float z, float width, const xlColor &color);
    void AddSphereAsTriangles(float x, float y, float z, float radius, const xlColor &color);
    
protected:
    std::string name;
};
class xlVertexIndexedColorAccumulator {
public:
    xlVertexIndexedColorAccumulator() {}
    virtual ~xlVertexIndexedColorAccumulator() {}
    
    xlVertexIndexedColorAccumulator *SetName(const std::string &n) { name = n;  return this; }
    const std::string &GetName() const { return name; }

    virtual void Reset() {}
    virtual void PreAlloc(unsigned int i) {};
    virtual void AddVertex(float x, float y, float z, uint32_t cIdx) {};
    virtual void AddVertex(float x, float y, uint32_t cIdx) { AddVertex(x, y, 0.0f, cIdx);};
    virtual uint32_t getCount() { return 0; }

    virtual void SetColorCount(int c) {}
    virtual uint32_t GetColorCount() { return 0; }
    virtual void SetColor(uint32_t idx, const xlColor &c) {}
    
    // mark this as ready to be copied to graphics card, after finalize,
    // vertices cannot be added, but if mayChange is set, the vertex/color
    // data can change via SetVertex and then flushed to push the
    // new data to the graphics card
    virtual void Finalize(bool mayChangeVertices, bool mayChangeColors) {}
    virtual void SetVertex(uint32_t vertex, float x, float y, float z, uint32_t cIdx)  = 0;
    virtual void SetVertex(uint32_t vertex, float x, float y, float z) = 0;
    virtual void SetVertex(uint32_t vertex, uint32_t cIdx) = 0;
    virtual void FlushRange(uint32_t start, uint32_t len) {}
    virtual void FlushColors(uint32_t start, uint32_t len) {}
    virtual xlVertexIndexedColorAccumulator* Flush() { FlushRange(0, getCount()); FlushColors(0, GetColorCount()); return this; }
    
    
    void AddCircleAsTriangles(float cx, float cy, float cz, float radius, uint32_t cIdx) {
        AddCircleAsTriangles(cx, cy, cz, radius, cIdx, cIdx, -1);
    }
    void AddCircleAsTriangles(float cx, float cy, float cz, float radius, uint32_t cIdx, uint32_t eIdx, int numSegments = -1);

    
protected:
    std::string name;
};


class xlVertexTextureAccumulator {
public:
    xlVertexTextureAccumulator() {}
    virtual ~xlVertexTextureAccumulator() {}
    
    xlVertexTextureAccumulator *SetName(const std::string &n) { name = n; return this; }
    const std::string &GetName() const { return name; }

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
    
protected:
    std::string name;
};


class xlTexture {
public:
    xlTexture() {}
    virtual ~xlTexture() {}
    
    xlTexture *SetName(const std::string &n) { name = n; return this; }
    const std::string &GetName() const { return name; }

    // mark the texture as immutable
    virtual void Finalize() {}

    virtual void UpdatePixel(int x, int y, const xlColor &c, bool copyAlpha) = 0;
    virtual void UpdateData(uint8_t *data, bool bgr, bool alpha) = 0;
    
    //platform specific data, possibly something like VideoToolbox or similar
    virtual void UpdateData(xlGraphicsContext *ctx, void *data, const std::string &type) {}
    
protected:
    std::string name;
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


class xlGraphicsProgram {
public:
    xlGraphicsProgram(xlVertexColorAccumulator *a);
    virtual ~xlGraphicsProgram();
    
    void runSteps(xlGraphicsContext *ctx);
    
    void addStep(std::function<void(xlGraphicsContext *ctx)> && f) {
        steps.push_back(f);
    }

    xlVertexColorAccumulator *getAccumulator();
private:
    xlVertexColorAccumulator *accumulator;
    
    std::list<std::function<void(xlGraphicsContext *ctx)>> steps;
};
