#pragma once

// A recording, GPU-free xlGraphicsContext + IModelPreview, so the preview
// drawing code in models/ can be driven from unit tests and its geometry
// checked.

#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "graphics/IModelPreview.h"
#include "graphics/xlGraphicsAccumulators.h"
#include "graphics/xlGraphicsContext.h"
#include "graphics/xlMesh.h"

namespace fakegfx {

struct V {
    float x, y, z;
    xlColor c;
};

struct RecVA : xlVertexAccumulator {
    std::vector<glm::vec3> v;
    void AddVertex(float x, float y, float z) override { v.push_back({ x, y, z }); }
    uint32_t getCount() override { return (uint32_t)v.size(); }
    void SetVertex(uint32_t i, float x, float y, float z) override {
        if (i < v.size()) v[i] = { x, y, z };
    }
    void Reset() override { v.clear(); }
};

struct RecCVA : xlVertexColorAccumulator {
    std::vector<V> v;
    int finalized = 0;
    void PreAlloc(unsigned int) override {}
    void AddVertex(float x, float y, float z, const xlColor& c) override { v.push_back({ x, y, z, c }); }
    uint32_t getCount() override { return (uint32_t)v.size(); }
    void Finalize(bool, bool) override { ++finalized; }
    void SetVertex(uint32_t i, float x, float y, float z, const xlColor& c) override {
        if (i < v.size()) v[i] = { x, y, z, c };
    }
    void SetVertex(uint32_t i, float x, float y, float z) override {
        if (i < v.size()) {
            v[i].x = x;
            v[i].y = y;
            v[i].z = z;
        }
    }
    void SetVertex(uint32_t i, const xlColor& c) override {
        if (i < v.size()) v[i].c = c;
    }
    void Reset() override { v.clear(); }
};

struct RecIVA : xlVertexIndexedColorAccumulator {
    struct E {
        float x, y, z;
        uint32_t idx;
    };
    std::vector<E> v;
    std::vector<xlColor> colors;
    int finalized = 0, colorFlushes = 0;
    void AddVertex(float x, float y, float z, uint32_t i) override { v.push_back({ x, y, z, i }); }
    void SetColorCount(int c) override { colors.resize(c); }
    uint32_t GetColorCount() override { return (uint32_t)colors.size(); }
    void SetColor(uint32_t idx, const xlColor& c) override {
        if (idx < colors.size()) colors[idx] = c;
    }
    void Finalize(bool, bool) override { ++finalized; }
    void FlushColors(uint32_t, uint32_t) override { ++colorFlushes; }
    uint32_t getCount() override { return (uint32_t)v.size(); }
    void SetVertex(uint32_t i, float x, float y, float z, uint32_t c) override {
        if (i < v.size()) v[i] = { x, y, z, c };
    }
    void SetVertex(uint32_t i, float x, float y, float z) override {
        if (i < v.size()) {
            v[i].x = x;
            v[i].y = y;
            v[i].z = z;
        }
    }
    void SetVertex(uint32_t i, uint32_t c) override {
        if (i < v.size()) v[i].idx = c;
    }
    void Reset() override { v.clear(); }
};

struct RecTVA : xlVertexTextureAccumulator {
    int count = 0;
    void AddVertex(float, float, float, float, float) override { ++count; }
    uint32_t getCount() override { return (uint32_t)count; }
};

struct FakeTexture : xlTexture {
    void UpdatePixel(int, int, const xlColor&, bool) override {}
    void UpdateData(uint8_t*, bool, bool) override {}
};

struct FakeContext : xlGraphicsContext {
    // number of draw calls and total vertices drawn, by primitive
    int lines = 0, lineStrips = 0, triangles = 0, triangleStrips = 0, points = 0, textures = 0, meshes = 0;
    int vertsDrawn = 0;
    int matrixDepth = 0;
    bool provideMeshes = false;

    void drew(int& kind, uint32_t avail, int start, int count) {
        ++kind;
        vertsDrawn += count < 0 ? (int)avail - start : count;
    }

    xlGraphicsContext* SetViewport(int, int, int, int, bool) override { return this; }
    xlVertexAccumulator* createVertexAccumulator() override {
        vas.push_back(new RecVA());
        return vas.back();
    }
    xlVertexColorAccumulator* createVertexColorAccumulator() override { return new RecCVA(); }
    xlVertexTextureAccumulator* createVertexTextureAccumulator() override { return new RecTVA(); }
    // not owned: whoever asked for them deletes them
    std::vector<RecIVA*> ivas;
    std::vector<RecVA*> vas;
    xlVertexIndexedColorAccumulator* createVertexIndexedColorAccumulator() override {
        ivas.push_back(new RecIVA());
        return ivas.back();
    }
    xlTexture* createTextureMipMaps(const std::vector<xlImage>&, const std::string&) override { return new FakeTexture(); }
    xlTexture* createTexture(const xlImage&, const std::string&, bool) override { return new FakeTexture(); }
    xlTexture* createTexture(int, int, bool, bool) override { return new FakeTexture(); }
    xlTexture* createTextureForFont(const xlFontInfo&) override { return new FakeTexture(); }
    xlGraphicsProgram* createGraphicsProgram() override { return new xlGraphicsProgram(new RecCVA()); }
    std::unique_ptr<xlMesh> loadMeshFromObjFile(const std::string& file) override {
        if (!provideMeshes) return nullptr;
        return std::make_unique<xlMesh>(this, file);
    }

    xlGraphicsContext* PushMatrix() override { ++matrixDepth; return this; }
    xlGraphicsContext* PopMatrix() override { --matrixDepth; return this; }
    xlGraphicsContext* Translate(float, float, float) override { return this; }
    xlGraphicsContext* Rotate(float, float, float, float) override { return this; }
    xlGraphicsContext* ApplyMatrix(const glm::mat4&) override { return this; }
    xlGraphicsContext* Scale(float, float, float) override { return this; }
    xlGraphicsContext* SetCamera(const glm::mat4&) override { return this; }
    xlGraphicsContext* SetModelMatrix(const glm::mat4&) override { return this; }
    xlGraphicsContext* ScaleViewMatrix(float, float, float) override { return this; }
    xlGraphicsContext* TranslateViewMatrix(float, float, float) override { return this; }
    xlGraphicsContext* enableBlending(bool) override { return this; }

    xlGraphicsContext* drawLines(xlVertexAccumulator* v, const xlColor&, int s, int c) override { drew(lines, v->getCount(), s, c); return this; }
    xlGraphicsContext* drawLineStrip(xlVertexAccumulator* v, const xlColor&, int s, int c) override { drew(lineStrips, v->getCount(), s, c); return this; }
    xlGraphicsContext* drawTriangles(xlVertexAccumulator* v, const xlColor&, int s, int c) override { drew(triangles, v->getCount(), s, c); return this; }
    xlGraphicsContext* drawTriangleStrip(xlVertexAccumulator* v, const xlColor&, int s, int c) override { drew(triangleStrips, v->getCount(), s, c); return this; }
    xlGraphicsContext* drawPoints(xlVertexAccumulator* v, const xlColor&, float, bool, int s, int c) override { drew(points, v->getCount(), s, c); return this; }

    xlGraphicsContext* drawLines(xlVertexColorAccumulator* v, int s, int c) override { drew(lines, v->getCount(), s, c); return this; }
    xlGraphicsContext* drawLineStrip(xlVertexColorAccumulator* v, int s, int c) override { drew(lineStrips, v->getCount(), s, c); return this; }
    xlGraphicsContext* drawTriangles(xlVertexColorAccumulator* v, int s, int c) override { drew(triangles, v->getCount(), s, c); return this; }
    xlGraphicsContext* drawTriangleStrip(xlVertexColorAccumulator* v, int s, int c) override { drew(triangleStrips, v->getCount(), s, c); return this; }
    xlGraphicsContext* drawPoints(xlVertexColorAccumulator* v, float, bool, int s, int c) override { drew(points, v->getCount(), s, c); return this; }

    xlGraphicsContext* drawLines(xlVertexIndexedColorAccumulator* v, int s, int c) override { drew(lines, v->getCount(), s, c); return this; }
    xlGraphicsContext* drawLineStrip(xlVertexIndexedColorAccumulator* v, int s, int c) override { drew(lineStrips, v->getCount(), s, c); return this; }
    xlGraphicsContext* drawTriangles(xlVertexIndexedColorAccumulator* v, int s, int c) override { drew(triangles, v->getCount(), s, c); return this; }
    xlGraphicsContext* drawTriangleStrip(xlVertexIndexedColorAccumulator* v, int s, int c) override { drew(triangleStrips, v->getCount(), s, c); return this; }
    xlGraphicsContext* drawPoints(xlVertexIndexedColorAccumulator* v, float, bool, int s, int c) override { drew(points, v->getCount(), s, c); return this; }

    xlGraphicsContext* drawTexture(xlTexture*, float, float, float, float, float, float, float, float, bool, int, int) override { ++textures; return this; }
    xlGraphicsContext* drawTexture(xlVertexTextureAccumulator*, xlTexture*, const xlColor&, int, int) override { ++textures; return this; }
    xlGraphicsContext* drawTexture(xlVertexTextureAccumulator*, xlTexture*, int, uint8_t, int, int) override { ++textures; return this; }

    xlGraphicsContext* drawMeshSolids(xlMesh*, int, bool) override { ++meshes; return this; }
    xlGraphicsContext* drawMeshTransparents(xlMesh*, int) override { ++meshes; return this; }
    xlGraphicsContext* drawMeshWireframe(xlMesh*, int) override { ++meshes; return this; }
};

// A preview window of the given pixel size whose virtual canvas matches it,
// with an orthographic 2D projection (or a simple perspective one in 3D).
struct FakePreview : IModelPreview {
    int w, h;
    bool is3d;
    std::string name = "FakePreview";
    FakeContext ctx;
    std::unique_ptr<xlGraphicsProgram> solid{ new xlGraphicsProgram(new RecCVA()) };
    std::unique_ptr<xlGraphicsProgram> transparent{ new xlGraphicsProgram(new RecCVA()) };
    glm::mat4 projView;
    glm::mat4 proj;
    glm::mat4 view{ 1.0f };
    bool noCurrentModel = false;
    double backingScale = 1.0;
    std::string highlightedController;

    FakePreview(int w_ = 1000, int h_ = 1000, bool is3d_ = false) : w(w_), h(h_), is3d(is3d_) {
        if (is3d) {
            proj = glm::perspective(glm::radians(45.0f), (float)w / (float)h, 1.0f, 100000.0f);
            view = glm::lookAt(glm::vec3(w / 2.0f, h / 2.0f, 2000.0f), glm::vec3(w / 2.0f, h / 2.0f, 0.0f), glm::vec3(0, 1, 0));
        } else {
            proj = glm::ortho(0.0f, (float)w, 0.0f, (float)h);
        }
        projView = proj * view;
        ctx.setContextualValue("modelPreview", static_cast<IModelPreview*>(this));
    }

    RecCVA& SolidVerts() { return *static_cast<RecCVA*>(solid->getAccumulator()); }
    RecCVA& TransparentVerts() { return *static_cast<RecCVA*>(transparent->getAccumulator()); }
    void Run() {
        solid->runSteps(&ctx);
        transparent->runSteps(&ctx);
    }

    int getWidth() const override { return w; }
    int getHeight() const override { return h; }
    std::string getName() const override { return name; }
    int GetVirtualCanvasWidth() const override { return w; }
    int GetVirtualCanvasHeight() const override { return h; }
    void GetVirtualCanvasSize(int& ww, int& hh) const override {
        ww = w;
        hh = h;
    }
    float GetCameraZoomForHandles() const override { return 1.0f; }
    int GetHandleScale() const override { return 1; }
    float GetCameraRotationX() const override { return 0.0f; }
    float GetCameraRotationY() const override { return 0.0f; }
    glm::mat4& GetProjViewMatrix() override { return projView; }
    glm::mat4& GetProjMatrix() override { return proj; }
    glm::mat4& GetViewMatrix() override { return view; }
    xlGraphicsContext* getCurrentGraphicsContext() override { return &ctx; }
    xlGraphicsProgram* getCurrentSolidProgram() override { return solid.get(); }
    xlGraphicsProgram* getCurrentTransparentProgram() override { return transparent.get(); }
    bool StartDrawing(double, bool) override { return true; }
    void EndDrawing(bool) override {}
    double calcPixelSize(double i) override { return i; }
    uint32_t getCurrentFrameTime() const override { return 0; }
    bool Is3D() const override { return is3d; }
    bool IsNoCurrentModel() override { return noCurrentModel; }
    double getBackingScaleFactor() const override { return backingScale; }
    bool IsControllerHighlighted(const std::string& n) const override { return !n.empty() && n == highlightedController; }
};

} // namespace fakegfx
