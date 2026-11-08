#pragma once

// Shared fakes for the screen-location tests: an orthographic preview whose
// world X/Y equal its pixel X/Y (with the usual top-left mouse origin), a
// recording colour accumulator for DrawHandles, and ray helpers for the drag
// sessions.

#include <memory>
#include <string>
#include <vector>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "graphics/IModelPreview.h"
#include "graphics/xlGraphicsAccumulators.h"
#include "models/ModelScreenLocation.h"
#include "models/RulerObject.h"
#include "models/ViewObjectManager.h"

namespace sl {

// Orthographic preview. A mouse click at (mx, my) unprojects to the world
// point (mx, height - my); rays point down -Z starting at z = +1000.
struct FakePreview : IModelPreview {
    int w = 800;
    int h = 600;
    bool is3d = false;
    float camRotX = 0.0f;
    float camRotY = 0.0f;
    float zoom = 1.0f;
    int handleScale = 0;
    glm::mat4 proj;
    glm::mat4 view{ 1.0f };
    glm::mat4 projView;

    FakePreview() {
        proj = glm::ortho(0.0f, (float)w, 0.0f, (float)h, -1000.0f, 1000.0f);
        projView = proj * view;
    }

    int getWidth() const override { return w; }
    int getHeight() const override { return h; }
    std::string getName() const override { return "fake"; }
    int GetVirtualCanvasWidth() const override { return w; }
    int GetVirtualCanvasHeight() const override { return h; }
    void GetVirtualCanvasSize(int& ww, int& hh) const override {
        ww = w;
        hh = h;
    }
    float GetCameraZoomForHandles() const override { return zoom; }
    int GetHandleScale() const override { return handleScale; }
    float GetCameraRotationX() const override { return camRotX; }
    float GetCameraRotationY() const override { return camRotY; }
    glm::mat4& GetProjViewMatrix() override { return projView; }
    glm::mat4& GetProjMatrix() override { return proj; }
    glm::mat4& GetViewMatrix() override { return view; }
    xlGraphicsContext* getCurrentGraphicsContext() override { return nullptr; }
    xlGraphicsProgram* getCurrentSolidProgram() override { return nullptr; }
    xlGraphicsProgram* getCurrentTransparentProgram() override { return nullptr; }
    bool StartDrawing(double, bool) override { return false; }
    void EndDrawing(bool) override {}
    double calcPixelSize(double i) override { return i; }
    uint32_t getCurrentFrameTime() const override { return 0; }
    bool Is3D() const override { return is3d; }
    bool IsNoCurrentModel() override { return false; }
};

struct CV {
    glm::vec3 p;
    xlColor c;
};

struct RecordingCVA : xlVertexColorAccumulator {
    std::vector<CV> v;
    void AddVertex(float x, float y, float z, const xlColor& c) override { v.push_back({ { x, y, z }, c }); }
    uint32_t getCount() override { return (uint32_t)v.size(); }
    void SetVertex(uint32_t i, float x, float y, float z, const xlColor& c) override { v[i] = { { x, y, z }, c }; }
    void SetVertex(uint32_t i, float x, float y, float z) override { v[i].p = { x, y, z }; }
    void SetVertex(uint32_t i, const xlColor& c) override { v[i].c = c; }
};

// xlGraphicsProgram owns (and deletes) its accumulator.
struct Program {
    RecordingCVA* va = new RecordingCVA();
    xlGraphicsProgram prog{ va };
    xlGraphicsProgram* operator&() { return &prog; }
    size_t size() const { return va->v.size(); }
    const CV& at(size_t i) const { return va->v.at(i); }
    const CV& back(size_t fromEnd = 0) const { return va->v.at(va->v.size() - 1 - fromEnd); }
};

inline size_t CubeVertexCount() {
    RecordingCVA r;
    r.AddCubeAsTriangles(0, 0, 0, 1, xlWHITE);
    return r.v.size();
}

// Ray straight down the -Z axis through (x, y) — what a 2D/front camera sends.
inline handles::WorldRay DownZ(float x, float y, float z = 1000.0f) {
    handles::WorldRay r;
    r.origin = glm::vec3(x, y, z);
    r.direction = glm::vec3(0.0f, 0.0f, -1.0f);
    return r;
}

// Ray along -Y through (x, z) — a top-down camera; hits XZ planes.
inline handles::WorldRay DownY(float x, float z, float y = 1000.0f) {
    handles::WorldRay r;
    r.origin = glm::vec3(x, y, z);
    r.direction = glm::vec3(0.0f, -1.0f, 0.0f);
    return r;
}

// Ray along -X through (y, z) — a side camera; hits YZ planes.
inline handles::WorldRay DownX(float y, float z, float x = 1000.0f) {
    handles::WorldRay r;
    r.origin = glm::vec3(x, y, z);
    r.direction = glm::vec3(-1.0f, 0.0f, 0.0f);
    return r;
}

inline handles::Id MakeId(handles::Role role, int index = 0, handles::Axis axis = handles::Axis::X, int segment = -1) {
    handles::Id id;
    id.role = role;
    id.index = index;
    id.axis = axis;
    id.segment = segment;
    return id;
}

inline const handles::Descriptor* Find(const std::vector<handles::Descriptor>& ds, const handles::Id& id) {
    for (const auto& d : ds) {
        if (d.id == id) return &d;
    }
    return nullptr;
}

inline size_t CountRole(const std::vector<handles::Descriptor>& ds, handles::Role role) {
    size_t n = 0;
    for (const auto& d : ds) {
        if (d.id.role == role) ++n;
    }
    return n;
}

inline void CheckVec(const glm::vec3& v, float x, float y, float z, float margin = 1e-3f) {
    CHECK(v.x == Catch::Approx(x).margin(margin));
    CHECK(v.y == Catch::Approx(y).margin(margin));
    CHECK(v.z == Catch::Approx(z).margin(margin));
}

// A ruler whose X2 run of `units` world units measures `length` metres.
// Registers itself as the global ruler for its lifetime.
struct ScopedRuler {
    ViewObjectManager vom{ nullptr };
    std::unique_ptr<RulerObject> ruler;
    ScopedRuler(float units, float length) {
        ruler = std::make_unique<RulerObject>(vom);
        ruler->GetObjectScreenLocation().SetWorldPos(0, 0, 0);
        static_cast<TwoPointScreenLocation&>(ruler->GetObjectScreenLocation()).SetX2(units);
        ruler->SetLength(length);
        ruler->SetUnits(RULER_UNITS_M);
    }
};

} // namespace sl
