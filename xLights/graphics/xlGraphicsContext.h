#pragma once

#include <vector>
#include <wx/bitmap.h>

#include "../Color.h"
#include "xlGraphicsAccumulators.h"
#include "xlFontInfo.h"

class xlGraphicsContext {
public:

    xlGraphicsContext() {}
    virtual ~xlGraphicsContext() {}

    // Setup the Viewport
    virtual void SetViewport(int x1, int y1, int x2, int y2, bool is3D = false) = 0;


    //create various accumulators/buffers/textures
    virtual xlVertexAccumulator *createVertexAccumulator() = 0;
    virtual xlVertexColorAccumulator *createVertexColorAccumulator() = 0;
    virtual xlVertexTextureAccumulator *createVertexTextureAccumulator() = 0;
    virtual xlTexture *createTextureMipMaps(const std::vector<wxBitmap> &bitmaps) = 0;
    virtual xlTexture *createTextureMipMaps(const std::vector<wxImage> &images) = 0;
    virtual xlTexture *createTexture(const wxImage &image) = 0;
    virtual xlTexture *createTextureForFont(const xlFontInfo &font) = 0;


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
    virtual void drawTexture(xlVertexTextureAccumulator *vac, xlTexture *texture) = 0;
    virtual void drawTexture(xlVertexTextureAccumulator *vac, xlTexture *texture, const xlColor &c) = 0;

};
