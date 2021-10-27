#pragma once

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


    xlGraphicsContext() {}
    virtual ~xlGraphicsContext() {}

    // Setup the Viewport
    virtual void SetViewport(int x1, int y1, int x2, int y2, bool is3D = false) = 0;


    //create various accumulators/buffers
    virtual xlVertexAccumulator *createVertexAccumulator() = 0;



    //manipulating the matrices
    virtual void PushMatrix() = 0;
    virtual void PopMatrix() = 0;
    virtual void Translate(float x, float y, float z) = 0;
    virtual void Rotate(float angle, float x, float y, float z) = 0;
    virtual void Scale(float w, float h, float z) = 0;

    //setters for various states


    //drawing methods
    virtual void drawLines(xlVertexAccumulator *vac, const xlColor &c) = 0;
    virtual void drawLineLoop(xlVertexAccumulator *vac, const xlColor &c) = 0;
    virtual void drawLineStrip(xlVertexAccumulator *vac, const xlColor &c) = 0;

};
