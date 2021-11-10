#pragma once

#include <stack>


#include "../xlGraphicsContext.h"
#include "xlMetalCanvas.h"

#include "Shaders/MetalShaderTypes.h"

class xlMetalGraphicsContext : public xlGraphicsContext {
public:
    xlMetalGraphicsContext(xlMetalCanvas *c);
    virtual ~xlMetalGraphicsContext();


    virtual xlVertexAccumulator *createVertexAccumulator() override;
    virtual xlVertexColorAccumulator *createVertexColorAccumulator() override;
    virtual xlVertexTextureAccumulator *createVertexTextureAccumulator() override;
    virtual xlTexture *createTextureMipMaps(const std::vector<wxBitmap> &bitmaps) override;
    virtual xlTexture *createTextureMipMaps(const std::vector<wxImage> &images) override;
    virtual xlTexture *createTexture(const wxImage &image) override;
    virtual xlTexture *createTextureForFont(const xlFontInfo &font) override;

    // Setup the Viewport
    virtual void SetViewport(int x1, int y1, int x2, int y2, bool is3D = false) override;

    //setters for various states
    virtual void enableBlending(bool e = true) override {
        blending = e;
    }

    //drawing methods
    //SingleColor Versions, only vertices need to be transfered, color is in uniform
    virtual void drawLines(xlVertexAccumulator *vac, const xlColor &c) override;
    virtual void drawLineStrip(xlVertexAccumulator *vac, const xlColor &c) override;
    virtual void drawTriangles(xlVertexAccumulator *vac, const xlColor &c) override;
    virtual void drawTriangleStrip(xlVertexAccumulator *vac, const xlColor &c) override;

    virtual void drawLines(xlVertexColorAccumulator *vac) override;
    virtual void drawLineStrip(xlVertexColorAccumulator *vac) override;
    virtual void drawTriangles(xlVertexColorAccumulator *vac) override;
    virtual void drawTriangleStrip(xlVertexColorAccumulator *vac) override;

    virtual void drawTexture(xlTexture *texture,
                             float x, float y, float x2, float y2,
                             float tx = 0.0, float ty = 0.0, float tx2 = 1.0, float ty2 = 1.0,
                             bool linearScale = true) override;
    virtual void drawTexture(xlVertexTextureAccumulator *vac, xlTexture *texture) override;
    virtual void drawTexture(xlVertexTextureAccumulator *vac, xlTexture *texture, const xlColor &c) override;


    //manipulating the matrices
    virtual void PushMatrix() override;
    virtual void PopMatrix() override;
    virtual void Translate(float x, float y, float z) override;
    virtual void Rotate(float angle, float x, float y, float z) override;
    virtual void Scale(float w, float h, float z) override;


    bool hasDrawable();
protected:
    xlMetalCanvas *canvas;
    id<MTLCommandBuffer> buffer;
    id<MTLRenderCommandEncoder> encoder;
    id<CAMetalDrawable> drawable;


    void drawPrimitive(MTLPrimitiveType type, xlVertexAccumulator *vac, const xlColor &c);
    void drawPrimitive(MTLPrimitiveType type, xlVertexColorAccumulator *vac);
    xlTexture *createTexture(const wxImage &image, bool pvt);

    bool setPipelineState(const std::string &name, const char *vShader, const char *fShader);

    bool blending = false;
    std::string lastPipeline;
    bool lastPipelineBlend = false;
    FrameData frameData;
    bool frameDataChanged = true;
    std::stack<simd::float4x4> matrixStack;
};
