#pragma once

#include "VulkanBackend.h"
#include "../xlGraphicsContext.h"
#include <memory>
#include <stack>
#include <glm/glm.hpp>

class xlVulkanGraphicsContext : public xlGraphicsContext {
public:
    xlVulkanGraphicsContext(wxWindow* w, gfx::VulkanBackend* backend);
    virtual ~xlVulkanGraphicsContext();

    // Setup the Viewport
    virtual xlGraphicsContext* SetViewport(int x1, int y1, int x2, int y2, bool is3D = false) override;

    //create various accumulators/buffers/textures
    virtual xlVertexAccumulator *createVertexAccumulator() override;
    virtual xlVertexColorAccumulator *createVertexColorAccumulator() override;
    virtual xlVertexTextureAccumulator *createVertexTextureAccumulator() override;
    virtual xlVertexIndexedColorAccumulator *createVertexIndexedColorAccumulator() override;
    virtual xlTexture *createTextureMipMaps(const std::vector<wxBitmap> &bitmaps, const std::string &name) override;
    virtual xlTexture *createTextureMipMaps(const std::vector<wxImage> &images, const std::string &name) override;
    virtual xlTexture *createTexture(const wxImage &image, const std::string &name, bool finalize = false) override;
    virtual xlTexture *createTextureMipMaps(const std::vector<xlImage> &images, const std::string &name) override;
    virtual xlTexture *createTexture(const xlImage &image, const std::string &name, bool finalize = false) override;
    virtual xlTexture *createTexture(int w, int h, bool bgr, bool alpha) override;
    virtual xlTexture *createTextureForFont(const xlFontInfo &font) override;
    virtual xlGraphicsProgram *createGraphicsProgram() override;
    virtual std::unique_ptr<xlMesh> loadMeshFromObjFile(const std::string &file) override;

    //matrix ops
    virtual xlGraphicsContext* PushMatrix() override;
    virtual xlGraphicsContext* PopMatrix() override;
    virtual xlGraphicsContext* Translate(float x, float y, float z) override;
    virtual xlGraphicsContext* Rotate(float angle, float x, float y, float z) override;
    virtual xlGraphicsContext* ApplyMatrix(const glm::mat4 &m) override;
    virtual xlGraphicsContext* Scale(float w, float h, float z) override;
    virtual xlGraphicsContext* SetCamera(const glm::mat4 &m) override;
    virtual xlGraphicsContext* SetModelMatrix(const glm::mat4 &m) override;
    virtual xlGraphicsContext* ScaleViewMatrix(float w, float h, float z) override;
    virtual xlGraphicsContext* TranslateViewMatrix(float x, float y, float z) override;

    virtual xlGraphicsContext* enableBlending(bool e = true) override;

    //drawing methods
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
                                           float x, float y, float x2, float y2,
                                           float tx = 0.0, float ty = 0.0, float tx2 = 1.0, float ty2 = 1.0,
                                           bool smoothScale = true,
                                           int brightness = 100, int alpha = 255) override;
    virtual xlGraphicsContext* drawTexture(xlVertexTextureAccumulator *vac, xlTexture *texture, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawTexture(xlVertexTextureAccumulator *vac, xlTexture *texture, const xlColor &c, int start = 0, int count = -1) override;
    virtual xlGraphicsContext* drawTexture(xlVertexTextureAccumulator *vac, xlTexture *texture, int brightness, uint8_t alpha, int start, int count) override;

    virtual xlGraphicsContext* drawMeshSolids(xlMesh *mesh, int brightness, bool useViewMatrix) override;
    virtual xlGraphicsContext* drawMeshTransparents(xlMesh *mesh, int brightness) override;
    virtual xlGraphicsContext* drawMeshWireframe(xlMesh *mesh, int brightness) override;

private:
    // Use the canvas-owned backend instance; this is a non-owning pointer
    gfx::VulkanBackend* backend_ = nullptr;
    std::stack<glm::mat4> matrixStack_;
    glm::mat4 projection_;
    glm::mat4 view_;
    glm::mat4 model_;

    bool blendingEnabled_ = false;
};
