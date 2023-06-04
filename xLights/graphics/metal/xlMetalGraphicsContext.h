#pragma once

#include <stack>


#include "../xlGraphicsContext.h"
#include "xlMetalCanvas.h"

#include "Shaders/MetalShaderTypes.h"

class xlMetalGraphicsContext : public xlGraphicsContext {
public:
    xlMetalGraphicsContext(xlMetalCanvas *c, id<MTLTexture> target, bool enqueImmediate);
    virtual ~xlMetalGraphicsContext();
    void Commit(bool displayOnScreen, id<MTLBuffer> capture);


    virtual xlVertexAccumulator *createVertexAccumulator() override;
    virtual xlVertexColorAccumulator *createVertexColorAccumulator() override;
    virtual xlVertexTextureAccumulator *createVertexTextureAccumulator() override;
    virtual xlVertexIndexedColorAccumulator *createVertexIndexedColorAccumulator() override;
    virtual xlTexture *createTextureMipMaps(const std::vector<wxBitmap> &bitmaps) override;
    virtual xlTexture *createTextureMipMaps(const std::vector<wxImage> &images) override;
    virtual xlTexture *createTexture(const wxImage &image) override;
    virtual xlTexture *createTexture(int w, int h, bool bgr, bool alpha) override;
    virtual xlTexture *createTextureForFont(const xlFontInfo &font) override;
    virtual xlGraphicsProgram *createGraphicsProgram() override;
    virtual xlMesh *loadMeshFromObjFile(const std::string &file) override;

    // Setup the Viewport
    virtual xlGraphicsContext* SetViewport(int x1, int y1, int x2, int y2, bool is3D = false) override;

    //setters for various states
    virtual xlGraphicsContext* enableBlending(bool e = true) override {
        blending = e;
        return this;
    }

    //drawing methods
    //SingleColor Versions, only vertices need to be transfered, color is in uniform
    virtual xlGraphicsContext* drawLines(xlVertexAccumulator *vac, const xlColor &c, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawLineStrip(xlVertexAccumulator *vac, const xlColor &c, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawTriangles(xlVertexAccumulator *vac, const xlColor &c, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawTriangleStrip(xlVertexAccumulator *vac, const xlColor &c, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawPoints(xlVertexAccumulator *vac, const xlColor &c, float pointSize, bool smoothPoints, int start = 0, int count = -1) override;

    virtual xlGraphicsContext* drawLines(xlVertexColorAccumulator *vac, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawLineStrip(xlVertexColorAccumulator *vac, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawTriangles(xlVertexColorAccumulator *vac, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawTriangleStrip(xlVertexColorAccumulator *vac, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawPoints(xlVertexColorAccumulator *vac, float pointSize, bool smoothPoints, int start = 0, int count = -1) override;

    virtual xlGraphicsContext* drawLines(xlVertexIndexedColorAccumulator *vac, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawLineStrip(xlVertexIndexedColorAccumulator *vac, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawTriangles(xlVertexIndexedColorAccumulator *vac, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawTriangleStrip(xlVertexIndexedColorAccumulator *vac, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawPoints(xlVertexIndexedColorAccumulator *vac, float pointSize, bool smoothPoints, int start = 0, int count = -1) override;


    virtual xlGraphicsContext* drawTexture(xlTexture *texture,
                                           float x, float y,
                                           float x2, float y2,
                                           float tx = 0.0, float ty = 0.0, float tx2 = 1.0, float ty2 = 1.0,
                                           bool smoothScale = true,
                                           int brightness = 100, int alpha = 255) override;
    virtual xlGraphicsContext* drawTexture(xlVertexTextureAccumulator *vac, xlTexture *texture, const xlColor &c, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawTexture(xlVertexTextureAccumulator *vac, xlTexture *texture, int brightness, uint8_t alpha, int start, int count) override;
    virtual xlGraphicsContext* drawMeshSolids(xlMesh *mesh, int brightness, bool useViewMatrix) override;
    virtual xlGraphicsContext* drawMeshTransparents(xlMesh *mesh, int brightness) override;
    virtual xlGraphicsContext* drawMeshWireframe(xlMesh *mesh, int brightness) override;


    //manipulating the matrices
    virtual xlGraphicsContext* PushMatrix() override;
    virtual xlGraphicsContext* PopMatrix() override;
    virtual xlGraphicsContext* Translate(float x, float y, float z) override;
    virtual xlGraphicsContext* Rotate(float angle, float x, float y, float z) override;
    virtual xlGraphicsContext* Scale(float w, float h, float z) override;
    virtual xlGraphicsContext* ApplyMatrix(const glm::mat4 &m) override;
    virtual xlGraphicsContext* SetCamera(const glm::mat4 &m) override;
    virtual xlGraphicsContext* SetModelMatrix(const glm::mat4 &m) override;
    virtual xlGraphicsContext* ScaleViewMatrix(float w, float h, float z) override;
    virtual xlGraphicsContext* TranslateViewMatrix(float x, float y, float z) override;

    virtual xlGraphicsContext* pushDebugContext(const std::string &label) override;
    virtual xlGraphicsContext* popDebugContext() override;


    bool isValid();
    id<CAMetalDrawable> getDrawable() { return drawable; }
    id<MTLCommandBuffer> getCommandBuffer() { return buffer; }
protected:
    xlMetalCanvas *canvas;
    id<MTLCommandBuffer> buffer;
    id<MTLRenderCommandEncoder> encoder;
    id<CAMetalDrawable> drawable;
    id<MTLTexture> target;

    xlGraphicsContext* drawPrimitive(MTLPrimitiveType type, xlVertexAccumulator *vac, const xlColor &c, int start, int count);
    xlGraphicsContext* drawPrimitive(MTLPrimitiveType type, xlVertexColorAccumulator *vac, int start, int count);
    xlGraphicsContext* drawPrimitive(MTLPrimitiveType type, xlVertexIndexedColorAccumulator *vac, int start, int count);
    
    xlTexture *createTexture(const wxImage &image, bool pvt, const std::string &n);
    void setPointSize(float ps, bool smoothPoints);

    bool setPipelineState(const std::string &name, const char *vShader, const char *fShader);

    bool blending = false;
    std::string lastPipeline;
    bool lastPipelineBlend = false;
    MTLFrameData frameData;
    bool frameDataChanged = true;
    std::stack<simd::float4x4> matrixStack;
    
    void *lastAccumulator = nullptr;
};
