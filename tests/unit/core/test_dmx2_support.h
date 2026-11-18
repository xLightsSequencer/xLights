#pragma once

// Helpers for the [dmx2] tests. Recording fakes for the wx-free graphics
// interfaces, so DMX model drawing code can run without a GPU: accumulators
// keep every vertex, the context records draw calls and the matrix applied
// before each mesh/texture draw, and the preview hands out a fixed name, size
// and frame time. Plus a small show fixture for loading models from XML.

#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>

#include <pugixml.hpp>

#include "graphics/IModelPreview.h"
#include "graphics/xlGraphicsAccumulators.h"
#include "graphics/xlGraphicsContext.h"
#include "graphics/xlMesh.h"
#include "models/Model.h"
#include "models/ModelManager.h"
#include "outputs/OutputManager.h"

namespace dmx2 {

struct RecVertex {
    float x, y, z;
    xlColor c;
};

struct RecColorAcc : xlVertexColorAccumulator {
    std::vector<RecVertex> v;
    void Reset() override { v.clear(); }
    void AddVertex(float x, float y, float z, const xlColor& c) override { v.push_back({ x, y, z, c }); }
    uint32_t getCount() override { return (uint32_t)v.size(); }
    void SetVertex(uint32_t i, float x, float y, float z, const xlColor& c) override { v[i] = { x, y, z, c }; }
    void SetVertex(uint32_t i, float x, float y, float z) override {
        v[i].x = x;
        v[i].y = y;
        v[i].z = z;
    }
    void SetVertex(uint32_t i, const xlColor& c) override { v[i].c = c; }
};

struct RecAcc : xlVertexAccumulator {
    std::vector<glm::vec3> v;
    void Reset() override { v.clear(); }
    void AddVertex(float x, float y, float z) override { v.emplace_back(x, y, z); }
    uint32_t getCount() override { return (uint32_t)v.size(); }
};

struct RecIndexedAcc : xlVertexIndexedColorAccumulator {
    uint32_t n = 0;
    void AddVertex(float, float, float, uint32_t) override { ++n; }
    uint32_t getCount() override { return n; }
    void SetVertex(uint32_t, float, float, float, uint32_t) override {}
    void SetVertex(uint32_t, float, float, float) override {}
    void SetVertex(uint32_t, uint32_t) override {}
};

struct RecTexAcc : xlVertexTextureAccumulator {
    uint32_t n = 0;
    void AddVertex(float, float, float, float, float) override { ++n; }
    uint32_t getCount() override { return n; }
};

struct NullTexture : xlTexture {
    void UpdatePixel(int, int, const xlColor&, bool) override {}
    void UpdateData(uint8_t*, bool, bool) override {}
};

struct MeshDraw {
    const xlMesh* mesh;
    int brightness;
    glm::mat4 matrix;
    bool wireframe;
};

struct RangeDraw {
    const void* acc;
    int start;
    int count;
};

struct TextureDraw {
    const xlTexture* texture;
    int alpha;
    glm::mat4 matrix;
};

struct FakeContext : xlGraphicsContext {
    std::vector<TextureDraw> textures;
    std::vector<MeshDraw> solids;
    std::vector<MeshDraw> transparents;
    std::vector<RangeDraw> triangles;
    std::vector<RangeDraw> lines;
    std::vector<std::string> loadedMeshes;
    glm::mat4 lastApplied{ 1.0f };
    int pushes = 0;
    int pops = 0;

    xlGraphicsContext* SetViewport(int, int, int, int, bool) override { return this; }
    xlVertexAccumulator* createVertexAccumulator() override { return new RecAcc(); }
    xlVertexColorAccumulator* createVertexColorAccumulator() override { return new RecColorAcc(); }
    xlVertexTextureAccumulator* createVertexTextureAccumulator() override { return new RecTexAcc(); }
    xlVertexIndexedColorAccumulator* createVertexIndexedColorAccumulator() override { return new RecIndexedAcc(); }
    xlTexture* createTextureMipMaps(const std::vector<xlImage>&, const std::string&) override { return new NullTexture(); }
    xlTexture* createTexture(const xlImage&, const std::string&, bool) override { return new NullTexture(); }
    xlTexture* createTexture(int, int, bool, bool) override { return new NullTexture(); }
    xlTexture* createTextureForFont(const xlFontInfo&) override { return new NullTexture(); }
    xlGraphicsProgram* createGraphicsProgram() override { return new xlGraphicsProgram(new RecColorAcc()); }
    std::unique_ptr<xlMesh> loadMeshFromObjFile(const std::string& file) override {
        loadedMeshes.push_back(file);
        return std::make_unique<xlMesh>(nullptr, file);
    }

    xlGraphicsContext* PushMatrix() override {
        ++pushes;
        return this;
    }
    xlGraphicsContext* PopMatrix() override {
        ++pops;
        return this;
    }
    xlGraphicsContext* Translate(float, float, float) override { return this; }
    xlGraphicsContext* Rotate(float, float, float, float) override { return this; }
    xlGraphicsContext* ApplyMatrix(const glm::mat4& m) override {
        lastApplied = m;
        return this;
    }
    xlGraphicsContext* Scale(float, float, float) override { return this; }
    xlGraphicsContext* SetCamera(const glm::mat4&) override { return this; }
    xlGraphicsContext* SetModelMatrix(const glm::mat4&) override { return this; }
    xlGraphicsContext* ScaleViewMatrix(float, float, float) override { return this; }
    xlGraphicsContext* TranslateViewMatrix(float, float, float) override { return this; }
    xlGraphicsContext* enableBlending(bool) override { return this; }

    xlGraphicsContext* drawLines(xlVertexAccumulator* a, const xlColor&, int s, int c) override { lines.push_back({ a, s, c }); return this; }
    xlGraphicsContext* drawLineStrip(xlVertexAccumulator*, const xlColor&, int, int) override { return this; }
    xlGraphicsContext* drawTriangles(xlVertexAccumulator* a, const xlColor&, int s, int c) override { triangles.push_back({ a, s, c }); return this; }
    xlGraphicsContext* drawTriangleStrip(xlVertexAccumulator*, const xlColor&, int, int) override { return this; }
    xlGraphicsContext* drawPoints(xlVertexAccumulator*, const xlColor&, float, bool, int, int) override { return this; }
    xlGraphicsContext* drawLines(xlVertexColorAccumulator* a, int s, int c) override { lines.push_back({ a, s, c }); return this; }
    xlGraphicsContext* drawLineStrip(xlVertexColorAccumulator*, int, int) override { return this; }
    xlGraphicsContext* drawTriangles(xlVertexColorAccumulator* a, int s, int c) override { triangles.push_back({ a, s, c }); return this; }
    xlGraphicsContext* drawTriangleStrip(xlVertexColorAccumulator*, int, int) override { return this; }
    xlGraphicsContext* drawPoints(xlVertexColorAccumulator*, float, bool, int, int) override { return this; }
    xlGraphicsContext* drawLines(xlVertexIndexedColorAccumulator*, int, int) override { return this; }
    xlGraphicsContext* drawLineStrip(xlVertexIndexedColorAccumulator*, int, int) override { return this; }
    xlGraphicsContext* drawTriangles(xlVertexIndexedColorAccumulator*, int, int) override { return this; }
    xlGraphicsContext* drawTriangleStrip(xlVertexIndexedColorAccumulator*, int, int) override { return this; }
    xlGraphicsContext* drawPoints(xlVertexIndexedColorAccumulator*, float, bool, int, int) override { return this; }
    xlGraphicsContext* drawTexture(xlTexture* t, float, float, float, float, float, float, float, float, bool, int, int alpha) override {
        textures.push_back({ t, alpha, lastApplied });
        return this;
    }
    xlGraphicsContext* drawTexture(xlVertexTextureAccumulator*, xlTexture*, const xlColor&, int, int) override { return this; }
    xlGraphicsContext* drawTexture(xlVertexTextureAccumulator*, xlTexture*, int, uint8_t, int, int) override { return this; }
    xlGraphicsContext* drawMeshSolids(xlMesh* m, int b, bool) override {
        solids.push_back({ m, b, lastApplied, false });
        return this;
    }
    xlGraphicsContext* drawMeshTransparents(xlMesh* m, int b) override {
        transparents.push_back({ m, b, lastApplied, false });
        return this;
    }
    xlGraphicsContext* drawMeshWireframe(xlMesh* m, int b) override {
        solids.push_back({ m, b, lastApplied, true });
        return this;
    }
};

struct FakePreview : IModelPreview {
    std::string name = "Preview";
    int width = 200;
    int height = 100;
    uint32_t frameTime = 0;
    bool is3d = false;
    bool zonesEnabled = true;
    bool zoneIndicator = false;
    bool noCurrentModel = false;
    bool hasContext = true;
    int startCalls = 0;
    int endCalls = 0;
    FakeContext ctx;
    xlGraphicsProgram solid{ new RecColorAcc() };
    xlGraphicsProgram transparent{ new RecColorAcc() };
    glm::mat4 m{ 1.0f };

    RecColorAcc& SolidAcc() { return *static_cast<RecColorAcc*>(solid.getAccumulator()); }
    RecColorAcc& TransAcc() { return *static_cast<RecColorAcc*>(transparent.getAccumulator()); }
    void Run() {
        solid.runSteps(&ctx);
        transparent.runSteps(&ctx);
    }

    int getWidth() const override { return width; }
    int getHeight() const override { return height; }
    std::string getName() const override { return name; }
    int GetVirtualCanvasWidth() const override { return width; }
    int GetVirtualCanvasHeight() const override { return height; }
    void GetVirtualCanvasSize(int& w, int& h) const override {
        w = width;
        h = height;
    }
    float GetCameraZoomForHandles() const override { return 1.0f; }
    int GetHandleScale() const override { return 1; }
    float GetCameraRotationX() const override { return 0.0f; }
    float GetCameraRotationY() const override { return 0.0f; }
    glm::mat4& GetProjViewMatrix() override { return m; }
    glm::mat4& GetProjMatrix() override { return m; }
    glm::mat4& GetViewMatrix() override { return m; }
    xlGraphicsContext* getCurrentGraphicsContext() override { return hasContext ? &ctx : nullptr; }
    xlGraphicsProgram* getCurrentSolidProgram() override { return &solid; }
    xlGraphicsProgram* getCurrentTransparentProgram() override { return &transparent; }
    bool StartDrawing(double, bool) override {
        ++startCalls;
        hasContext = true;
        return true;
    }
    void EndDrawing(bool) override { ++endCalls; }
    double calcPixelSize(double i) override { return i; }
    uint32_t getCurrentFrameTime() const override { return frameTime; }
    bool Is3D() const override { return is3d; }
    bool IsNoCurrentModel() override { return noCurrentModel; }
    bool GetShowZoneIndicator() const override { return zoneIndicator; }
    bool GetEnablePositionZones() const override { return zonesEnabled; }
};

// A unit cube from (0,0,0) to (1,1,1) with one material per face group, so
// mesh colour overrides can be read back by material name.
inline const char* kCubeObj = R"(mtllib cube.mtl
o Cube
v 0 0 0
v 1 0 0
v 1 1 0
v 0 1 0
v 0 0 1
v 1 0 1
v 1 1 1
v 0 1 1
usemtl EyeColor
f 1 2 3 4
f 5 8 7 6
usemtl Body
f 1 5 6 2
f 2 6 7 3
f 3 7 8 4
f 5 1 4 8
)";

inline const char* kCubeMtl = R"(newmtl EyeColor
Kd 1 1 1
newmtl Body
Kd 0.5 0.5 0.5
)";

struct Show {
    OutputManager om;
    ModelManager mm{ &om };

    Model* Add(const std::string& xml) {
        pugi::xml_document doc;
        REQUIRE(doc.load_string(xml.c_str()));
        Model* m = mm.createAndAddModel(doc.first_child(), 100, 100);
        REQUIRE(m != nullptr);
        mm.RecalcStartChannels();
        return m;
    }
};

// A model element with a unique name (models with the same name replace each other).
inline std::string Dmx(const std::string& type, const std::string& attrs, const std::string& children = "") {
    static int counter = 0;
    return "<model name=\"M" + std::to_string(++counter) + "\" DisplayAs=\"" + type + "\" StartChannel=\"1\" StringType=\"Single Color White\" " + attrs + ">" + children + "</model>";
}

template <class T>
T* As(Model* m) {
    T* t = dynamic_cast<T*>(m);
    REQUIRE(t != nullptr);
    return t;
}

// Sets 1-based channel `channel` of the model to `v`, as rendered data would.
inline void Chan(Model* m, int channel, uint8_t v) {
    m->SetNodeChannelValues(channel - 1, &v);
}

inline void ClearChans(Model* m) {
    for (uint32_t i = 0; i < m->GetNodeCount(); ++i) {
        Chan(m, i + 1, 0);
    }
}

// Draws the model the way the sequencer's effect preview does (active,
// reading channel values) and runs the recorded steps.
inline void DrawEffect(Model* m, FakePreview& p) {
    m->DisplayEffectOnWindow(&p, 2.0);
    p.Run();
}

// Draws the model the way the layout does (inactive: no channel values).
inline void DrawLayout(Model* m, FakePreview& p, const xlColor* c = nullptr, bool is3d = false, float* bbox = nullptr) {
    m->DisplayModelOnWindow(&p, &p.ctx, &p.solid, &p.transparent, is3d, c, true, false, false, 0, bbox);
    p.Run();
}

// The angle in degrees [0, 360) of the vector from (0,0) to (x,y).
inline float AngleOf(float x, float y) {
    float a = std::atan2(y, x) * 180.0f / 3.14159265f;
    if (a < 0) {
        a += 360.0f;
    }
    return a;
}

// True when the accumulator holds exactly DmxModel::DrawInvalid's red cross.
inline bool IsInvalidCross(const RecColorAcc& s) {
    if (s.v.size() != 12) {
        return false;
    }
    for (auto& v : s.v) {
        if (v.c.red != 255 || v.c.green != 0 || v.c.blue != 0) {
            return false;
        }
        if (std::abs(std::abs(v.x) - 0.5f) > 1e-6 || std::abs(std::abs(v.y) - 0.5f) > 1e-6) {
            return false;
        }
    }
    return true;
}

} // namespace dmx2
