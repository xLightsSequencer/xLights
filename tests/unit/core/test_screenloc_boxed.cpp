#include <cmath>
#include <memory>
#include <numbers>
#include <string>
#include <vector>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "models/BoxedScreenLocation.h"
#include "models/Node.h"
#include "utils/FloatChecks.h"
#include "test_screenloc_common.h"

using namespace sl;
using Catch::Approx;
using handles::Axis;
using handles::Modifier;
using handles::Role;
using handles::UpdateResult;
using MSLAXIS = ModelScreenLocation::MSLAXIS;
using MSLPLANE = ModelScreenLocation::MSLPLANE;

// Free helpers defined in ModelScreenLocation.cpp (shared by the subclasses).
extern CursorType GetResizeCursor(int cornerIndex, int PreviewRotation);
extern void rotate_point(float cx, float cy, float angle, float& x, float& y);

namespace {

// 10x10 box centred at `pos`, unit scale.
struct Box {
    BoxedScreenLocation b;
    explicit Box(glm::vec3 pos = glm::vec3(0.0f), float wi = 10.0f, float ht = 10.0f) {
        b.SetRenderSize(wi, ht, 0.0f);
        b.SetWorldPosition(pos);
        b.Init();
    }
};

std::vector<NodeBaseClassPtr> NodesAt(const std::vector<glm::vec3>& pts) {
    std::vector<NodeBaseClassPtr> nodes;
    for (const auto& p : pts) {
        auto n = std::make_unique<NodeBaseClass>(0, 1);
        n->Coords[0].screenX = p.x;
        n->Coords[0].screenY = p.y;
        n->Coords[0].screenZ = p.z;
        nodes.push_back(std::move(n));
    }
    return nodes;
}

} // namespace

// ---------------------------------------------------------------------------
// ModelScreenLocation shared helpers
// ---------------------------------------------------------------------------

TEST_CASE("Gizmo sizes grow with zoom and handle scale but never shrink below the minimum", "[screenloc]") {
    Box x;
    const auto& l = x.b;
    CHECK(l.GetAxisArrowLength(1.0f, 0) == Approx(60.0f));
    CHECK(l.GetAxisArrowLength(2.0f, 2) == Approx(240.0f));
    CHECK(l.GetAxisArrowLength(0.1f, 0) == Approx(60.0f));
    CHECK(l.GetAxisHeadLength(2.0f, 2) == Approx(48.0f));
    CHECK(l.GetAxisHeadLength(0.5f, 0) == Approx(12.0f));
    CHECK(l.GetAxisRadius(2.0f, 2) == Approx(16.0f));
    CHECK(l.GetAxisRadius(0.5f, 0) == Approx(4.0f));
    // scale 3 is "small": half of the normal (scale 0) width
    CHECK(l.GetRectHandleWidth(1.0f, 0) == Approx(6.0f));
    CHECK(l.GetRectHandleWidth(1.0f, 3) == Approx(3.0f));
    CHECK(l.GetRectHandleWidth(1.0f, 2) == Approx(12.0f));
    CHECK(l.GetRectHandleWidth(0.01f, 0) == Approx(0.5f));
}

TEST_CASE("Resize cursor follows the corner and the preview rotation", "[screenloc]") {
    CHECK(GetResizeCursor(L_TOP_HANDLE, 0) == CursorType::SizeNWSE);
    CHECK(GetResizeCursor(R_TOP_HANDLE, 0) == CursorType::SizeNESW);
    CHECK(GetResizeCursor(R_BOT_HANDLE, 0) == CursorType::SizeNWSE);
    CHECK(GetResizeCursor(L_BOT_HANDLE, 0) == CursorType::SizeNESW);
    // 22.5 degree steps walk through the 16 states
    CHECK(GetResizeCursor(L_TOP_HANDLE, 23) == CursorType::SizeWE);
    CHECK(GetResizeCursor(L_TOP_HANDLE, 68) == CursorType::SizeNESW);
    CHECK(GetResizeCursor(L_TOP_HANDLE, 113) == CursorType::SizeNS);
    CHECK(GetResizeCursor(L_TOP_HANDLE, 158) == CursorType::SizeNWSE);
    CHECK(GetResizeCursor(L_TOP_HANDLE, 203) == CursorType::SizeWE);
    CHECK(GetResizeCursor(L_TOP_HANDLE, 248) == CursorType::SizeNESW);
    CHECK(GetResizeCursor(L_TOP_HANDLE, 293) == CursorType::SizeNS);
    CHECK(GetResizeCursor(L_TOP_HANDLE, 338) == CursorType::SizeNWSE);
    // a corner a quarter turn on is the same as the next corner unrotated
    CHECK(GetResizeCursor(R_TOP_HANDLE, 90) == GetResizeCursor(R_BOT_HANDLE, 0));
}

TEST_CASE("rotate_point turns a point about a centre", "[screenloc]") {
    float x = 2.0f, y = 1.0f;
    rotate_point(1.0f, 1.0f, std::numbers::pi_v<float> / 2.0f, x, y);
    CHECK(x == Approx(1.0f).margin(1e-5));
    CHECK(y == Approx(2.0f).margin(1e-5));
}

TEST_CASE("Axis helpers cycle and the rotate tool shifts the active axis", "[screenloc]") {
    Box x;
    auto& l = x.b;
    CHECK(l.NextAxis(MSLAXIS::X_AXIS) == MSLAXIS::Y_AXIS);
    CHECK(l.NextAxis(MSLAXIS::Y_AXIS) == MSLAXIS::Z_AXIS);
    CHECK(l.NextAxis(MSLAXIS::Z_AXIS) == MSLAXIS::X_AXIS);
    CHECK(l.NextAxis(MSLAXIS::NO_AXIS) == MSLAXIS::X_AXIS);

    l.SetAxisTool(handles::Tool::Translate);
    l.SetActiveAxis(MSLAXIS::X_AXIS);
    CHECK(l.GetActiveAxis() == MSLAXIS::X_AXIS);
    l.SetAxisTool(handles::Tool::Rotate);
    l.SetActiveAxis(MSLAXIS::X_AXIS);
    CHECK(l.GetActiveAxis() == MSLAXIS::Y_AXIS);
    l.SetActiveAxis(MSLAXIS::NO_AXIS);
    CHECK(l.GetActiveAxis() == MSLAXIS::NO_AXIS);
}

TEST_CASE("Advancing the axis tool cycles translate scale rotate", "[screenloc]") {
    Box x;
    auto& l = x.b;
    CHECK(l.GetDefaultTool() == handles::Tool::Scale);
    l.SetAxisTool(handles::Tool::Translate);
    l.AdvanceAxisTool();
    CHECK(l.GetAxisTool() == handles::Tool::Scale);
    l.AdvanceAxisTool();
    CHECK(l.GetAxisTool() == handles::Tool::Rotate);
    l.AdvanceAxisTool();
    CHECK(l.GetAxisTool() == handles::Tool::Translate);
    l.SetAxisTool(handles::Tool::Elevate);
    l.AdvanceAxisTool();
    CHECK(l.GetAxisTool() == handles::Tool::Translate);
}

TEST_CASE("Active handle ignores axis gizmo ids and highlight skips the active handle", "[screenloc]") {
    Box x;
    auto& l = x.b;
    CHECK_FALSE(l.GetActiveHandleId().has_value());
    l.SetActiveHandleToCentre();
    REQUIRE(l.GetActiveHandleId().has_value());
    CHECK(l.GetActiveHandleId()->role == Role::CentreCycle);

    l.SetActiveHandle(MakeId(Role::AxisArrow, 0, Axis::Y));
    CHECK(l.GetActiveHandleId()->role == Role::CentreCycle);
    l.SetActiveHandle(MakeId(Role::AxisRing));
    l.SetActiveHandle(MakeId(Role::AxisCube));
    CHECK(l.GetActiveHandleId()->role == Role::CentreCycle);

    l.MouseOverHandle(MakeId(Role::CentreCycle));
    CHECK_FALSE(l.GetHighlightedHandleId().has_value());
    l.MouseOverHandle(MakeId(Role::ResizeCorner, L_TOP_HANDLE));
    REQUIRE(l.GetHighlightedHandleId().has_value());
    CHECK(l.GetHighlightedHandleId()->index == L_TOP_HANDLE);
    l.SetActiveHandleToDefault();
    CHECK_FALSE(l.GetHighlightedHandleId().has_value());
    l.SetActiveHandle(std::nullopt);
    CHECK_FALSE(l.GetActiveHandleId().has_value());
}

TEST_CASE("AdjustRenderSize keeps loaded scale on first sizing but resets it on a real resize", "[screenloc]") {
    BoxedScreenLocation b;
    b.SetScaleMatrix(glm::vec3(2.0f, 3.0f, 4.0f));
    b.AdjustRenderSize(10.0f, 20.0f, 30.0f);
    CheckVec(b.GetScaleMatrix(), 2, 3, 4);
    CHECK(b.GetRenderWi() == 10.0f);
    CHECK(b.GetRenderDp() == 30.0f);

    b.AdjustRenderSize(10.0f, 20.0f, 30.0f);
    CheckVec(b.GetScaleMatrix(), 2, 3, 4);

    b.AdjustRenderSize(1.0f, 5.0f, 5.0f);
    CheckVec(b.GetScaleMatrix(), 2, 3, 4);
    CHECK(b.GetRenderHt() == 5.0f);

    b.AdjustRenderSize(12.0f, 5.0f, 5.0f);
    CheckVec(b.GetScaleMatrix(), 1, 1, 1);
    CHECK(b.GetRenderWi() == 12.0f);
}

TEST_CASE("Camera angle picks the drag plane", "[screenloc]") {
    Box x;
    auto& l = x.b;
    FakePreview p;
    p.is3d = true;

    l.RefreshActivePlaneFromCamera(nullptr);
    CHECK(l.GetActivePlane() == MSLPLANE::NO_PLANE);

    l.SetPreferredSelectionPlane(MSLPLANE::XY_PLANE);
    l.RefreshActivePlaneFromCamera(&p);
    CHECK(l.GetActivePlane() == MSLPLANE::XY_PLANE);

    // looking straight on, the ground plane is edge-on so XY wins
    l.SetPreferredSelectionPlane(MSLPLANE::GROUND);
    CHECK(l.GetPreferredSelectionPlane() == MSLPLANE::XZ_PLANE);
    l.RefreshActivePlaneFromCamera(&p);
    CHECK(l.GetActivePlane() == MSLPLANE::XY_PLANE);

    // tilted 45 degrees both are usable: the preference wins
    p.camRotX = 45.0f;
    l.RefreshActivePlaneFromCamera(&p);
    CHECK(l.GetActivePlane() == MSLPLANE::XZ_PLANE);
    l.SetPreferredSelectionPlane(MSLPLANE::XY_PLANE);
    l.RefreshActivePlaneFromCamera(&p);
    CHECK(l.GetActivePlane() == MSLPLANE::XY_PLANE);

    // looking straight down the XY plane is edge-on
    p.camRotX = 90.0f;
    l.RefreshActivePlaneFromCamera(&p);
    CHECK(l.GetActivePlane() == MSLPLANE::XZ_PLANE);

    // camera swung round to the side: XY becomes YZ
    p.camRotX = 0.0f;
    p.camRotY = 90.0f;
    l.RefreshActivePlaneFromCamera(&p);
    CHECK(l.GetActivePlane() == MSLPLANE::YZ_PLANE);
    p.camRotY = 270.0f;
    l.SetPreferredSelectionPlane(MSLPLANE::GROUND);
    p.camRotX = 45.0f;
    l.RefreshActivePlaneFromCamera(&p);
    CHECK(l.GetActivePlane() == MSLPLANE::XZ_PLANE);

    // a camera tilted below the horizon has neither plane: fall back to YZ
    p.camRotX = -45.0f;
    p.camRotY = 0.0f;
    l.SetPreferredSelectionPlane(MSLPLANE::XY_PLANE);
    l.RefreshActivePlaneFromCamera(&p);
    CHECK(l.GetActivePlane() == MSLPLANE::YZ_PLANE);
}

TEST_CASE("Screen position helpers pass the point through without a camera", "[screenloc]") {
    Box x;
    FakePreview p;
    float sx = 3.0f, sy = 4.0f, sz = 5.0f;
    glm::vec2 r = x.b.GetScreenPosition(800, 600, &p, nullptr, sx, sy, sz);
    CHECK(r.x == 3.0f);
    CHECK(r.y == 4.0f);
    r = x.b.GetScreenPosition(800, 600, nullptr, nullptr, sx, sy, sz);
    CHECK(r.x == 3.0f);
    r = x.b.GetScreenPositionForAspect(800, 600, 0, 600, nullptr, sx, sy, sz);
    CHECK(r.y == 4.0f);
}

TEST_CASE("TranslateVector matches TranslatePoint", "[screenloc]") {
    Box x(glm::vec3(1, 2, 3));
    x.b.SetScaleMatrix(glm::vec3(2, 2, 2));
    x.b.SetRotateZ(90.0f);
    x.b.PrepareToDraw(true, true);
    glm::vec3 v(1.0f, 0.0f, 0.0f);
    x.b.TranslateVector(v);
    CheckVec(v, 1, 4, 3);
}

TEST_CASE("Default matrices only translate", "[screenloc]") {
    Box x(glm::vec3(4, 5, 6));
    x.b.SetDefaultMatrices();
    glm::mat4 m = x.b.GetModelMatrix();
    CheckVec(glm::vec3(m * glm::vec4(1, 1, 1, 1)), 5, 6, 7);
}

TEST_CASE("Size-based bounding box pads thin axes and drives 3D ray hits", "[screenloc]") {
    Box x(glm::vec3(100, 50, 0));
    auto& b = x.b;
    b.SetScaleMatrix(glm::vec3(2, 1, 1));
    b.UpdateBoundingBox(10.0f, 6.0f, 2.0f);
    b.PrepareToDraw(true, true);

    float dist = 1e9f;
    glm::vec3 o(100, 50, 100), d(0, 0, -1);
    CHECK(b.HitTest3D(o, d, dist));
    // depth 2 is padded by 8 either side: front face at z = 9
    CHECK(dist == Approx(91.0f).margin(1e-2));

    glm::vec3 o2(109, 50, 100);
    CHECK(b.HitTest3D(o2, d, dist));
    glm::vec3 o3(111, 50, 100);
    CHECK_FALSE(b.HitTest3D(o3, d, dist));
    glm::vec3 o4(100, 54, 100);
    CHECK_FALSE(b.HitTest3D(o4, d, dist));
}

// ---------------------------------------------------------------------------
// Boxed: bounds, hit testing and containment
// ---------------------------------------------------------------------------

TEST_CASE("Boxed 2D hit test uses the node bounding box around the centre", "[screenloc]") {
    Box x(glm::vec3(50, 50, 0));
    auto& b = x.b;
    b.UpdateBoundingBox(10.0f, 10.0f, 0.0f);
    b.PrepareToDraw(false, true);
    glm::vec3 d(0, 0, -1);
    glm::vec3 in(53, 47, 0), out(56, 50, 0), out2(50, 44, 0);
    CHECK(b.HitTest(in, d));
    CHECK_FALSE(b.HitTest(out, d));
    CHECK_FALSE(b.HitTest(out2, d));
}

TEST_CASE("Boxed containment in 2D needs the whole box inside the rectangle", "[screenloc]") {
    Box x(glm::vec3(50, 50, 0));
    auto& b = x.b;
    b.UpdateBoundingBox(10.0f, 10.0f, 0.0f);
    b.PrepareToDraw(false, true);
    FakePreview p;
    CHECK(b.IsContained(&p, 40, 40, 60, 60));
    CHECK(b.IsContained(&p, 60, 60, 40, 40));
    CHECK_FALSE(b.IsContained(&p, 46, 40, 60, 60));
    CHECK_FALSE(b.IsContained(&p, 40, 40, 60, 54));
}

TEST_CASE("Boxed containment in 3D projects the box to screen space", "[screenloc]") {
    Box x(glm::vec3(50, 50, 0));
    auto& b = x.b;
    b.UpdateBoundingBox(10.0f, 10.0f, 10.0f);
    b.PrepareToDraw(true, true);
    FakePreview p;
    // screen Y runs down: world y 45..55 is screen 545..555
    CHECK(b.IsContained(&p, 40, 540, 60, 560));
    CHECK_FALSE(b.IsContained(&p, 40, 40, 60, 60));
}

TEST_CASE("Boxed node bounding box is scaled and padded", "[screenloc]") {
    Box x(glm::vec3(0, 0, 0));
    auto& b = x.b;
    b.SetScaleMatrix(glm::vec3(2, 1, 1));
    auto nodes = NodesAt({ { -5, -5, 0 }, { 5, 5, 0 } });
    nodes.push_back(nullptr);
    b.UpdateBoundingBox(nodes);
    b.PrepareToDraw(false, true);
    glm::vec3 d(0, 0, -1);
    glm::vec3 edge(9.5f, 0, 0), past(10.5f, 0, 0);
    CHECK(b.HitTest(edge, d));
    CHECK_FALSE(b.HitTest(past, d));

    // a flat line of nodes is padded to a selectable height
    auto line = NodesAt({ { -5, 0, 0 }, { 5, 0, 0 } });
    b.UpdateBoundingBox(line);
    glm::vec3 nearLine(0, 4.5f, 0), farLine(0, 5.5f, 0);
    CHECK(b.HitTest(nearLine, d));
    CHECK_FALSE(b.HitTest(farLine, d));

    // empty node list leaves the bounding box unchanged
    std::vector<NodeBaseClassPtr> none;
    b.UpdateBoundingBox(none);
    CHECK(b.HitTest(nearLine, d));
}

TEST_CASE("Boxed depth extent comes from the rotated node extent", "[screenloc]") {
    Box x(glm::vec3(0, 0, 100));
    auto& b = x.b;
    auto nodes = NodesAt({ { -5, -2, -1 }, { 5, 2, 1 } });
    b.UpdateBoundingBox(nodes);
    CHECK(b.GetFront() == Approx(101.0f));
    CHECK(b.GetBack() == Approx(99.0f));
    // a quarter turn about Y swings the X extent into depth
    b.SetRotateY(90.0f);
    CHECK(b.GetFront() == Approx(105.0f).margin(1e-3));
    CHECK(b.GetBack() == Approx(95.0f).margin(1e-3));
    b.SetBack(0.0f);
    CHECK(b.GetBack() == Approx(0.0f).margin(1e-3));
    CHECK(b.GetFront() == Approx(10.0f).margin(1e-3));
}

TEST_CASE("Boxed size setters handle single pixel models", "[screenloc]") {
    BoxedScreenLocation b;
    b.SetRenderSize(1.0f, 1.0f, 0.0f);
    b.SetMWidth(7.0f);
    CHECK(b.GetScaleX() == Approx(7.0f));
    CHECK(b.GetRestorableMWidth() == Approx(7.0f));
    b.SetMWidth(0.0f);
    CHECK(b.GetScaleX() == Approx(1.0f));
    b.SetMHeight(4.0f);
    CHECK(b.GetScaleY() == Approx(4.0f));
    CHECK(b.GetRestorableMHeight() == Approx(4.0f));
    b.SetMDepth(9.0f);
    CHECK(b.GetScaleZ() == Approx(1.0f));

    b.SetRenderSize(11.0f, 6.0f, 0.0f);
    b.SetMDepth(20.0f);
    CHECK(b.GetScaleZ() == Approx(2.0f));
    CHECK(b.GetRestorableMDepth() == Approx(20.0f));
    CHECK(b.GetMDepth() == Approx(22.0f));

    b.SetRenderSize(-1.0f, -1.0f, 0.0f);
    CHECK(b.GetMWidth() == 0.0f);
    CHECK(b.GetMHeight() == 0.0f);
}

TEST_CASE("Boxed centre setters move the world position", "[screenloc]") {
    BoxedScreenLocation b;
    b.SetCentreX(1.0f);
    b.SetCentreY(2.0f);
    b.SetCentreZ(3.0f);
    CHECK(b.GetCentreX() == 1.0f);
    CHECK(b.GetCentreY() == 2.0f);
    CHECK(b.GetCentreZ() == 3.0f);
    CheckVec(b.GetWorldPosition(), 1, 2, 3);
    b.SetHcenterPos(4.0f);
    b.SetVcenterPos(5.0f);
    CheckVec(b.GetWorldPosition(), 4, 5, 3);
    b.SetScale(2.0f, 3.0f);
    b.SetScaleZ(4.0f);
    b.SetScaleX(5.0f);
    b.SetScaleY(6.0f);
    CheckVec(b.GetScaleMatrix(), 5, 6, 4);
    b.SetRotation(30);
    CHECK(b.GetRotation() == 30);
    b.PrepareToDraw(false, false);
    CHECK(b.GetCentreX() == 4.0f);
    CHECK(b.IsCenterBased());
}

TEST_CASE("Boxed screen offset is the centre as a fraction of the preview", "[screenloc]") {
    Box x(glm::vec3(200, 150, 0));
    FakePreview p;
    glm::vec2 off = x.b.GetScreenOffset(&p);
    CHECK(off.x == Approx(0.25f));
    CHECK(off.y == Approx(0.75f));
}

TEST_CASE("Boxed dimensions are reported through the ruler", "[screenloc]") {
    Box x(glm::vec3(0.0f), 11.0f, 6.0f);
    auto& b = x.b;
    CHECK(b.GetDimension() == "");
    {
        // 100 world units = 2 metres
        ScopedRuler r(100.0f, 2.0f);
        CHECK(b.GetDimension() == "Width 0.22m Height 0.12m");
        CHECK(b.GetRealWidth() == Approx(0.2f));
        CHECK(b.GetRealHeight() == Approx(0.1f));
        CHECK(b.GetRealDepth() == 0.0f);
        b.SetSupportsZScaling(true);
        CHECK(b.GetSupportsZScaling());
        CHECK(b.GetDimension() == "Width 0.22m Height 0.12m Depth 0.22m");
        CHECK(b.GetRealDepth() == Approx(0.2f));
    }
    CHECK(b.GetDimension() == "");
}

TEST_CASE("Boxed InitializeLocation in 2D starts a corner drag at the click", "[screenloc]") {
    BoxedScreenLocation b;
    b.SetRenderSize(10, 10, 0);
    FakePreview p;
    int handle = -1;
    std::vector<NodeBaseClassPtr> nodes;
    CHECK(b.InitializeLocation(handle, 120, 200, nodes, &p) == CursorType::Sizing);
    CHECK(handle == R_BOT_HANDLE);
    CheckVec(b.GetWorldPosition(), 120, 400, 0);
    CheckVec(b.GetScaleMatrix(), 0, 0, 0);
    CHECK(b.GetActivePlane() == MSLPLANE::XY_PLANE);
    CHECK(b.GetActiveAxis() == MSLAXIS::Y_AXIS);
}

TEST_CASE("Boxed InitializeLocation in 3D puts the top left corner at the click", "[screenloc]") {
    BoxedScreenLocation b;
    b.SetRenderSize(10, 10, 0);
    FakePreview p;
    p.is3d = true;
    int handle = -1;
    std::vector<NodeBaseClassPtr> nodes;
    b.InitializeLocation(handle, 120, 200, nodes, &p);
    CHECK(handle == CENTER_HANDLE);
    CHECK(b.GetLeft() == Approx(120.0f));
    CHECK(b.GetTop() == Approx(400.0f));
    CHECK(b.GetWorldPos_Z() == 0.0f);

    // a ground-preferring model still samples the click on the vertical plane
    BoxedScreenLocation g;
    g.SetRenderSize(10, 10, 0);
    g.SetPreferredSelectionPlane(MSLPLANE::GROUND);
    p.camRotX = 45.0f;
    g.InitializeLocation(handle, 120, 200, nodes, &p);
    CHECK(g.GetActivePlane() == MSLPLANE::XZ_PLANE);
    CHECK(g.GetLeft() == Approx(120.0f));
    CHECK(g.GetTop() == Approx(400.0f));

    // depth-scaling models start with the Y axis active
    BoxedScreenLocation z;
    z.SetRenderSize(10, 10, 0);
    z.SetSupportsZScaling(true);
    p.camRotX = 0.0f;
    z.InitializeLocation(handle, 10, 10, nodes, &p);
    CHECK(z.GetActiveAxis() == MSLAXIS::Y_AXIS);

    // a side view starts the model turned to face the camera
    BoxedScreenLocation s;
    s.SetRenderSize(10, 10, 0);
    p.camRotY = 90.0f;
    p.camRotX = 0.0f;
    s.InitializeLocation(handle, 10, 10, nodes, &p);
    CHECK(s.GetActivePlane() == MSLPLANE::YZ_PLANE);
    CHECK(s.GetRotateY() == Approx(90.0f));
}

// ---------------------------------------------------------------------------
// Boxed: handle descriptors
// ---------------------------------------------------------------------------

TEST_CASE("Boxed 2D handles sit just outside the scaled corners", "[screenloc]") {
    Box x(glm::vec3(100, 50, 0), 10.0f, 4.0f);
    auto& b = x.b;
    b.SetScaleMatrix(glm::vec3(2, 2, 1));
    auto hs = b.GetHandles(handles::ViewMode::TwoD, handles::Tool::Translate);
    REQUIRE(hs.size() == 5);
    CHECK(CountRole(hs, Role::ResizeCorner) == 4);
    const auto* lt = Find(hs, MakeId(Role::ResizeCorner, L_TOP_HANDLE));
    const auto* rt = Find(hs, MakeId(Role::ResizeCorner, R_TOP_HANDLE));
    const auto* rb = Find(hs, MakeId(Role::ResizeCorner, R_BOT_HANDLE));
    const auto* lb = Find(hs, MakeId(Role::ResizeCorner, L_BOT_HANDLE));
    const auto* rot = Find(hs, MakeId(Role::Rotate, ROTATE_HANDLE));
    REQUIRE(lt);
    REQUIRE(rt);
    REQUIRE(rb);
    REQUIRE(lb);
    REQUIRE(rot);
    // half width 10 + 8 offset, half height 4 + 8 offset
    CheckVec(lt->worldPos, 82, 62, 0);
    CheckVec(rt->worldPos, 118, 62, 0);
    CheckVec(rb->worldPos, 118, 38, 0);
    CheckVec(lb->worldPos, 82, 38, 0);
    CheckVec(rot->worldPos, 100, 104, 0);
    CHECK(lt->editable);

    // the handles turn with the model
    b.SetRotateZ(90.0f);
    hs = b.GetHandles(handles::ViewMode::TwoD, handles::Tool::Translate);
    CheckVec(Find(hs, MakeId(Role::Rotate, ROTATE_HANDLE))->worldPos, 46, 50, 0);

    b.Lock();
    hs = b.GetHandles(handles::ViewMode::TwoD, handles::Tool::Translate);
    for (const auto& d : hs) CHECK_FALSE(d.editable);
}

TEST_CASE("Boxed 2D handles survive a zero scale", "[screenloc]") {
    Box x(glm::vec3(0, 0, 0));
    x.b.SetScaleMatrix(glm::vec3(0, 0, 1));
    auto hs = x.b.GetHandles(handles::ViewMode::TwoD, handles::Tool::Translate);
    REQUIRE(hs.size() == 5);
    for (const auto& d : hs) {
        CHECK(xl::isfinite(d.worldPos.x));
        CHECK(xl::isfinite(d.worldPos.y));
    }
}

TEST_CASE("Boxed 3D handles depend on the tool", "[screenloc]") {
    Box x(glm::vec3(10, 20, 30));
    auto& b = x.b;
    auto t = b.GetHandles(handles::ViewMode::ThreeD, handles::Tool::Translate);
    REQUIRE(t.size() == 4);
    const auto* c = Find(t, MakeId(Role::CentreCycle));
    REQUIRE(c);
    CHECK(c->selectionOnly);
    CheckVec(c->worldPos, 10, 20, 30);
    // arrow descriptor sits mid head: 60 - 12/2 along the axis
    CheckVec(Find(t, MakeId(Role::AxisArrow, 0, Axis::X))->worldPos, 64, 20, 30);
    CheckVec(Find(t, MakeId(Role::AxisArrow, 0, Axis::Y))->worldPos, 10, 74, 30);
    CheckVec(Find(t, MakeId(Role::AxisArrow, 0, Axis::Z))->worldPos, 10, 20, 84);

    handles::ViewParams vp;
    vp.axisArrowLength = 100.0f;
    vp.axisHeadLength = 20.0f;
    vp.axisRadius = 7.0f;
    auto big = b.GetHandles(handles::ViewMode::ThreeD, handles::Tool::Translate, vp);
    const auto* bx = Find(big, MakeId(Role::AxisArrow, 0, Axis::X));
    CheckVec(bx->worldPos, 100, 20, 30);
    CHECK(bx->suggestedRadius == 7.0f);

    auto s = b.GetHandles(handles::ViewMode::ThreeD, handles::Tool::Scale);
    CHECK(CountRole(s, Role::AxisCube) == 2);
    b.SetSupportsZScaling(true);
    s = b.GetHandles(handles::ViewMode::ThreeD, handles::Tool::Scale);
    CHECK(CountRole(s, Role::AxisCube) == 3);

    auto r = b.GetHandles(handles::ViewMode::ThreeD, handles::Tool::Rotate);
    CHECK(CountRole(r, Role::AxisRing) == 3);

    auto n = b.GetHandles(handles::ViewMode::ThreeD, handles::Tool::None);
    CHECK(n.size() == 1);
}

TEST_CASE("Handle position lookup by id", "[screenloc]") {
    Box x(glm::vec3(100, 50, 0));
    auto& b = x.b;
    b.PrepareToDraw(false, false);
    CheckVec(b.GetHandlePositionById(std::nullopt), 0, 0, 0);
    CheckVec(b.GetHandlePositionById(MakeId(Role::ResizeCorner, R_TOP_HANDLE)), 113, 63, 0);
    CheckVec(b.GetHandlePositionById(MakeId(Role::Vertex, 7)), 0, 0, 0);
    b.SetActiveHandle(MakeId(Role::Rotate, ROTATE_HANDLE));
    CheckVec(b.GetActiveHandlePosition(), 100, 105, 0);
    b.PrepareToDraw(true, false);
    b.SetActiveHandleToCentre();
    CheckVec(b.GetActiveHandlePosition(), 100, 50, 0);
}

// ---------------------------------------------------------------------------
// Boxed: drag sessions
// ---------------------------------------------------------------------------

TEST_CASE("Boxed drag sessions are refused when locked or for unported handles", "[screenloc]") {
    Box x;
    auto& b = x.b;
    CHECK(b.CreateDragSession("m", MakeId(Role::CentreCycle), DownZ(0, 0)) == nullptr);
    CHECK(b.CreateDragSession("m", MakeId(Role::Move), DownZ(0, 0)) == nullptr);
    b.Lock();
    CHECK(b.CreateDragSession("m", MakeId(Role::AxisArrow), DownZ(0, 0)) == nullptr);
    CHECK(b.BeginCreate("m", DownZ(0, 0), handles::ViewMode::TwoD) == nullptr);
}

TEST_CASE("Boxed translate arrow moves only along its axis", "[screenloc]") {
    Box x(glm::vec3(10, 20, 30));
    auto& b = x.b;
    auto s = b.CreateDragSession("box", MakeId(Role::AxisArrow, 0, Axis::X), DownZ(12, 22));
    REQUIRE(s);
    CHECK(s->GetHandleId().axis == Axis::X);
    CHECK(s->GetRotationInfo() == std::nullopt);
    CHECK(s->Update(DownZ(19, 99), Modifier::None) == UpdateResult::Updated);
    CheckVec(b.GetWorldPosition(), 17, 20, 30);
    CHECK(s->Update(DownZ(19, 0), Modifier::None) == UpdateResult::Unchanged);
    // a ray parallel to the drag plane is ignored
    handles::WorldRay flat;
    flat.origin = glm::vec3(0, 0, 30);
    flat.direction = glm::vec3(1, 0, 0);
    CHECK(s->Update(flat, Modifier::None) == UpdateResult::Unchanged);
    auto c = s->Commit();
    CHECK(c.modelName == "box");
    CHECK(c.dirty == handles::DirtyField::Position);
    s->Revert();
    CheckVec(b.GetWorldPosition(), 10, 20, 30);
    CHECK(s->Commit().dirty == handles::DirtyField::None);

    auto y = b.CreateDragSession("box", MakeId(Role::AxisArrow, 0, Axis::Y), DownZ(0, 0));
    y->Update(DownZ(5, -4), Modifier::None);
    CheckVec(b.GetWorldPosition(), 10, 16, 30);

    // Z drags on the ground plane through the model
    auto z = b.CreateDragSession("box", MakeId(Role::AxisArrow, 0, Axis::Z), DownY(0, 0));
    z->Update(DownY(3, 7), Modifier::None);
    CheckVec(b.GetWorldPosition(), 10, 16, 37);
}

TEST_CASE("Boxed translate falls back to the model centre when the first ray misses", "[screenloc]") {
    Box x(glm::vec3(10, 20, 30));
    handles::WorldRay flat;
    flat.origin = glm::vec3(0, 0, 30);
    flat.direction = glm::vec3(1, 0, 0);
    auto s = x.b.CreateDragSession("box", MakeId(Role::AxisArrow, 0, Axis::X), flat);
    s->Update(DownZ(15, 0), Modifier::None);
    CheckVec(x.b.GetWorldPosition(), 15, 20, 30);
}

TEST_CASE("Boxed scale cube scales by the drag ratio", "[screenloc]") {
    Box x(glm::vec3(0, 0, 0));
    auto& b = x.b;
    auto s = b.CreateDragSession("box", MakeId(Role::AxisCube, 0, Axis::X), DownZ(0, 0));
    REQUIRE(s);
    CHECK(s->Update(DownZ(5, 3), Modifier::None) == UpdateResult::Updated);
    CheckVec(b.GetScaleMatrix(), 1.5f, 1, 1);
    CHECK(s->Commit().dirty == handles::DirtyField::Dimensions);
    CHECK(s->Update(DownZ(5, 3), Modifier::None) == UpdateResult::Updated);
    s->Revert();
    CheckVec(b.GetScaleMatrix(), 1, 1, 1);
    CHECK(s->Update(DownZ(0, 0), Modifier::None) == UpdateResult::Unchanged);

    // shift on X keeps the aspect: height follows (depth when it scales)
    s->Update(DownZ(5, 0), Modifier::Shift);
    CheckVec(b.GetScaleMatrix(), 1.5f, 1.5f, 1);
    s->Revert();
    b.SetSupportsZScaling(true);
    auto sz = b.CreateDragSession("box", MakeId(Role::AxisCube, 0, Axis::X), DownZ(0, 0));
    sz->Update(DownZ(5, 0), Modifier::Shift);
    CheckVec(b.GetScaleMatrix(), 1.5f, 1, 1.5f);
    sz->Revert();

    // ctrl scales everything uniformly from the dragged axis
    auto cx = b.CreateDragSession("box", MakeId(Role::AxisCube, 0, Axis::X), DownZ(0, 0));
    cx->Update(DownZ(10, 0), Modifier::Control);
    CheckVec(b.GetScaleMatrix(), 2, 2, 2);
    cx->Revert();
    auto cy = b.CreateDragSession("box", MakeId(Role::AxisCube, 0, Axis::Y), DownZ(0, 0));
    cy->Update(DownZ(0, 10), Modifier::Control);
    CheckVec(b.GetScaleMatrix(), 2, 2, 2);
    cy->Revert();
    auto cz = b.CreateDragSession("box", MakeId(Role::AxisCube, 0, Axis::Z), DownY(0, 0));
    cz->Update(DownY(0, 10), Modifier::Control);
    CheckVec(b.GetScaleMatrix(), 2, 2, 2);
    cz->Revert();

    // ctrl+shift keeps the bottom edge where it was
    auto cs = b.CreateDragSession("box", MakeId(Role::AxisCube, 0, Axis::Y), DownZ(0, 0));
    cs->Update(DownZ(0, 10), Modifier::Control | Modifier::Shift);
    CHECK(b.GetBottom() == Approx(-5.0f));
    cs->Revert();

    // shift on Y also anchors the bottom
    auto ys = b.CreateDragSession("box", MakeId(Role::AxisCube, 0, Axis::Y), DownZ(0, 0));
    ys->Update(DownZ(0, 10), Modifier::Shift);
    CheckVec(b.GetScaleMatrix(), 1, 2, 1);
    CHECK(b.GetBottom() == Approx(-5.0f));
    CHECK(b.GetVcenterPos() == Approx(5.0f));
    ys->Revert();

    // Z: depth only, and with shift width follows
    auto z = b.CreateDragSession("box", MakeId(Role::AxisCube, 0, Axis::Z), DownY(0, 0));
    z->Update(DownY(0, 5), Modifier::Shift);
    CheckVec(b.GetScaleMatrix(), 1.5f, 1, 1.5f);
    z->Revert();
}

TEST_CASE("Boxed Z scale is ignored for models without depth scaling", "[screenloc]") {
    Box x;
    auto& b = x.b;
    auto z = b.CreateDragSession("box", MakeId(Role::AxisCube, 0, Axis::Z), DownY(0, 0));
    CHECK(z->Update(DownY(0, 5), Modifier::None) == UpdateResult::Unchanged);
    CheckVec(b.GetScaleMatrix(), 1, 1, 1);
}

TEST_CASE("Boxed scale does nothing for a zero sized model", "[screenloc]") {
    BoxedScreenLocation b;
    auto s = b.CreateDragSession("box", MakeId(Role::AxisCube, 0, Axis::X), DownZ(0, 0));
    CHECK(s->Update(DownZ(5, 0), Modifier::None) == UpdateResult::Unchanged);
}

TEST_CASE("Boxed rotate ring turns the model by the swept angle", "[screenloc]") {
    Box x(glm::vec3(0, 0, 0));
    auto& b = x.b;
    // the Y ring rotates about Z (next axis)
    auto s = b.CreateDragSession("box", MakeId(Role::AxisRing, 0, Axis::Y), DownZ(10, 0));
    REQUIRE(s);
    CHECK(s->Update(DownZ(0, 10), Modifier::None) == UpdateResult::Updated);
    CHECK(b.GetRotateZ() == Approx(90.0f).margin(1e-3));
    auto info = s->GetRotationInfo();
    REQUIRE(info.has_value());
    CHECK(info->rotationAxis == Axis::Z);
    CHECK(info->accumulated == Approx(90.0f));
    CheckVec(info->pivot, 0, 0, 0);
    CHECK(s->Update(DownZ(0, 10), Modifier::None) == UpdateResult::Unchanged);
    CHECK(s->Commit().dirty == handles::DirtyField::Rotation);
    s->Revert();
    CHECK(b.GetRotateZ() == Approx(0.0f));
    CHECK(s->Commit().dirty == handles::DirtyField::None);

    // the Z ring rotates about X
    auto rx = b.CreateDragSession("box", MakeId(Role::AxisRing, 0, Axis::Z), DownX(0, 10));
    rx->Update(DownX(10, 0), Modifier::None);
    CHECK(std::fabs(b.GetRotateX()) == Approx(90.0f).margin(1e-3));
    rx->Revert();
    CHECK(b.GetRotateX() == Approx(0.0f));

    // the X ring rotates about Y
    auto ry = b.CreateDragSession("box", MakeId(Role::AxisRing, 0, Axis::X), DownY(0, 10));
    ry->Update(DownY(10, 0), Modifier::None);
    CHECK(std::fabs(b.GetRotateY()) == Approx(90.0f).margin(0.05));
    ry->Revert();
    CHECK(b.GetRotateY() == Approx(0.0f));
}

TEST_CASE("Boxed 2D corner drag resizes keeping the opposite corner fixed", "[screenloc]") {
    Box x(glm::vec3(0, 0, 0));
    auto& b = x.b;
    auto s = b.CreateDragSession("box", MakeId(Role::ResizeCorner, R_BOT_HANDLE), DownZ(0, 0));
    REQUIRE(s);
    CHECK(s->Update(DownZ(20, -20), Modifier::None) == UpdateResult::Updated);
    // 20 + 5 - 8 = 17 wide/high
    CheckVec(b.GetScaleMatrix(), 1.7f, 1.7f, 1);
    CHECK(b.GetLeft() == Approx(-5.0f));
    CHECK(b.GetTop() == Approx(5.0f));
    CHECK(s->Commit().dirty == handles::DirtyField::Dimensions);
    s->Revert();
    CheckVec(b.GetScaleMatrix(), 1, 1, 1);
    CheckVec(b.GetWorldPosition(), 0, 0, 0);

    // dragging past the opposite edge is refused
    CHECK(s->Update(DownZ(-6, -20), Modifier::None) == UpdateResult::Unchanged);
    CHECK(s->Update(DownZ(20, 6), Modifier::None) == UpdateResult::Unchanged);

    auto lt = b.CreateDragSession("box", MakeId(Role::ResizeCorner, L_TOP_HANDLE), DownZ(0, 0));
    lt->Update(DownZ(-20, 20), Modifier::None);
    CHECK(b.GetRight() == Approx(5.0f));
    CHECK(b.GetBottom() == Approx(-5.0f));
    // the guard uses the current (grown) size: right edge is now at 8.5
    CHECK(lt->Update(DownZ(9, 20), Modifier::None) == UpdateResult::Unchanged);
    CHECK(lt->Update(DownZ(-20, -9), Modifier::None) == UpdateResult::Unchanged);
    lt->Revert();
}

TEST_CASE("Boxed 2D corner drag with shift keeps the aspect ratio", "[screenloc]") {
    Box x(glm::vec3(0, 0, 0), 10.0f, 5.0f);
    auto& b = x.b;
    auto s = b.CreateDragSession("box", MakeId(Role::ResizeCorner, R_TOP_HANDLE), DownZ(0, 0));
    // width wins: 17 wide, so 8.5 high
    s->Update(DownZ(20, 10), Modifier::Shift);
    CHECK(b.GetScaleMatrix().x == Approx(1.7f));
    CHECK(b.GetScaleMatrix().y == Approx(1.7f));
    s->Revert();
    // height wins
    s->Update(DownZ(10, 30), Modifier::Shift);
    CHECK(b.GetScaleMatrix().y == Approx(b.GetScaleMatrix().x));
    CHECK(b.GetScaleMatrix().y == Approx((30.0f + 2.5f - 8.0f) / 5.0f));
    s->Revert();

    b.SetSupportsZScaling(true);
    auto z = b.CreateDragSession("box", MakeId(Role::ResizeCorner, L_BOT_HANDLE), DownZ(0, 0));
    z->Update(DownZ(-20, -10), Modifier::None);
    CHECK(b.GetScaleMatrix().z == Approx(b.GetScaleMatrix().x));
}

TEST_CASE("Boxed 2D rotate handle follows the cursor angle", "[screenloc]") {
    Box x(glm::vec3(0, 0, 0));
    auto& b = x.b;
    auto s = b.CreateDragSession("box", MakeId(Role::Rotate, ROTATE_HANDLE), DownZ(0, 10));
    REQUIRE(s);
    CHECK(s->Update(DownZ(0, 10), Modifier::None) == UpdateResult::Unchanged);
    CHECK(s->Update(DownZ(10, 0), Modifier::None) == UpdateResult::Unchanged);
    CHECK(s->Update(DownZ(10, 10), Modifier::None) == UpdateResult::Updated);
    CHECK(b.GetRotateZ() == Approx(-45.0f));
    auto info = s->GetRotationInfo();
    REQUIRE(info);
    CHECK(info->accumulated == Approx(-45.0f));
    CHECK(info->rotationAxis == Axis::Z);
    s->Update(DownZ(-10, -10), Modifier::None);
    CHECK(b.GetRotateZ() == Approx(135.0f));
    s->Update(DownZ(10, -10), Modifier::None);
    CHECK(b.GetRotateZ() == Approx(-135.0f));
    // shift snaps to 5 degrees
    s->Update(DownZ(10, 30), Modifier::Shift);
    CHECK(b.GetRotateZ() == Approx(-15.0f));
    CHECK(s->Commit().dirty == handles::DirtyField::Rotation);
    s->Revert();
    CHECK(b.GetRotateZ() == Approx(0.0f));
}

TEST_CASE("Boxed 2D creation drags the bottom right corner", "[screenloc]") {
    BoxedScreenLocation b;
    b.SetRenderSize(10, 10, 0);
    b.SetWorldPosition(glm::vec3(0, 0, 0));
    auto s = b.BeginCreate("new", DownZ(0, 0), handles::ViewMode::TwoD);
    REQUIRE(s);
    CHECK(s->GetHandleId().role == Role::ResizeCorner);
    CHECK(s->GetHandleId().index == R_BOT_HANDLE);
    CHECK(s->Update(DownZ(20, -20), Modifier::None) == UpdateResult::Updated);
    CHECK(b.GetLeft() == Approx(-5.0f));
    auto c = s->Commit();
    CHECK(c.modelName == "new");
    CHECK(c.dirty == handles::DirtyField::Dimensions);
    s->Revert();
    CheckVec(b.GetScaleMatrix(), 1, 1, 1);
}

TEST_CASE("Boxed 3D creation grows right and down from the top left corner", "[screenloc]") {
    BoxedScreenLocation b;
    b.SetRenderSize(10, 10, 0);
    b.SetWorldPosition(glm::vec3(5, -5, 0));
    auto s = b.BeginCreate("new", DownZ(0, 0), handles::ViewMode::ThreeD);
    REQUIRE(s);
    CHECK(s->GetHandleId().role == Role::AxisCube);
    CHECK(s->Update(DownZ(10, -10), Modifier::None) == UpdateResult::Updated);
    CheckVec(b.GetScaleMatrix(), 2, 2, 1);
    CHECK(b.GetLeft() == Approx(0.0f));
    CHECK(b.GetTop() == Approx(0.0f));
    CHECK(s->Update(DownZ(10, -10), Modifier::None) == UpdateResult::Updated);
    s->Revert();
    CHECK(s->Update(DownZ(0, 0), Modifier::None) == UpdateResult::Unchanged);

    // shrinking past zero clamps to a sliver instead of flipping
    s->Update(DownZ(-100, 100), Modifier::None);
    CHECK(b.GetScaleMatrix().x == Approx(0.05f));
    CHECK(b.GetScaleMatrix().y == Approx(0.05f));
    s->Revert();
}

TEST_CASE("Boxed 3D creation keeps ground models above the ground", "[screenloc]") {
    BoxedScreenLocation b;
    b.SetRenderSize(10, 10, 0);
    b.SetWorldPosition(glm::vec3(5, 5, 0));
    b.SetPreferredSelectionPlane(MSLPLANE::GROUND);
    b.SetSupportsZScaling(true);
    auto s = b.BeginCreate("tree", DownZ(0, 0), handles::ViewMode::ThreeD);
    s->Update(DownZ(10, -30), Modifier::None);
    CheckVec(b.GetScaleMatrix(), 2, 4, 2);
    CHECK(b.GetBottom() == Approx(0.0f));
    // depth grows back from the front face
    CHECK(b.GetWorldPos_Z() == Approx(-5.0f));
}

TEST_CASE("Boxed space mouse rotates and translates the whole model", "[screenloc]") {
    Box x(glm::vec3(1, 2, 3));
    auto& b = x.b;
    auto sm = b.BeginSpaceMouseSession(MakeId(Role::CentreCycle));
    REQUIRE(sm);
    REQUIRE(sm->GetHandleId().has_value());
    CHECK(sm->Apply(2.0f, glm::vec3(0.0f), glm::vec3(1, 2, 3)) == handles::SpaceMouseResult::Dirty);
    CheckVec(b.GetWorldPosition(), 3, -4, 7);
    sm->Apply(1.0f, glm::vec3(0, 9, 0), glm::vec3(0.0f));
    CHECK(b.GetRotateZ() == Approx(90.0f).margin(1e-3));
    auto none = b.BeginSpaceMouseSession(std::nullopt);
    CHECK_FALSE(none->GetHandleId().has_value());
}

// ---------------------------------------------------------------------------
// Boxed: drawing
// ---------------------------------------------------------------------------

TEST_CASE("Boxed 2D handles draw a square at each descriptor and a rotate spoke", "[screenloc]") {
    Box x(glm::vec3(100, 50, 0));
    auto& b = x.b;
    Program p;
    CHECK(b.DrawHandles(&p, 1.0f, 0, false));
    // 5 squares of two triangles + one spoke line
    REQUIRE(p.size() == 5 * 6 + 2);
    const auto hs = b.GetHandles(handles::ViewMode::TwoD, handles::Tool::Translate);
    // every square is centred on its descriptor, 6 wide at zoom 1
    for (size_t i = 0; i < 5; ++i) {
        float minx = 1e9f, maxx = -1e9f, miny = 1e9f, maxy = -1e9f;
        for (size_t k = 0; k < 6; ++k) {
            const auto& v = p.at(i * 6 + k).p;
            minx = std::min(minx, v.x);
            maxx = std::max(maxx, v.x);
            miny = std::min(miny, v.y);
            maxy = std::max(maxy, v.y);
        }
        CHECK((minx + maxx) / 2 == Approx(hs[i].worldPos.x));
        CHECK((miny + maxy) / 2 == Approx(hs[i].worldPos.y));
        CHECK(maxx - minx == Approx(6.0f));
    }
    CheckVec(p.back(1).p, 100, 50, 0);
    CheckVec(p.back(0).p, 100, 105, 0);
    CHECK(p.at(0).c == xlBLUETRANSLUCENT);

    Program locked;
    b.Lock();
    b.DrawHandles(&locked, 1.0f, 0, false);
    CHECK(locked.at(0).c == FROM_BASE_HANDLES_COLOUR);
}

TEST_CASE("Boxed 3D handles draw the bounding cube and the active gizmo", "[screenloc]") {
    Box x(glm::vec3(0, 0, 0));
    auto& b = x.b;
    b.SetRenderSize(10, 10, 4);
    b.PrepareToDraw(true, true);
    Program bare;
    CHECK(b.DrawHandles(&bare, 1.0f, 0, true, false));
    // 12 cube edges
    REQUIRE(bare.size() == 24);
    // front top-left corner: half size + 8 offset, front face at +Dp/2
    CheckVec(bare.at(0).p, -13, 13, 2);
    CheckVec(bare.at(1).p, 13, 13, 2);
    CHECK(bare.at(0).c == xlWHITE);

    Program fromBase;
    b.DrawHandles(&fromBase, 1.0f, 0, true, true);
    CHECK(fromBase.at(0).c == FROM_BASE_HANDLES_COLOUR);
    b.Lock();
    Program locked;
    b.DrawHandles(&locked, 1.0f, 0, true, false);
    CHECK(locked.at(0).c == LOCKED_HANDLES_COLOUR);
    b.Lock(false);

    const size_t cube = CubeVertexCount();
    b.SetActiveHandleToCentre();
    b.SetAxisTool(handles::Tool::Translate);
    b.SetActiveAxis(MSLAXIS::X_AXIS);
    Program t;
    b.DrawHandles(&t, 1.0f, 0, true, false);
    // cube edges + centre sphere + 3 cones of 18 triangles + 3 shafts + axis line
    CHECK(t.size() == 24 + cube + 3 * 18 * 3 + 6 + 2);
    CheckVec(t.back(1).p, -1000000.0f, 0, 0);
    CheckVec(t.back(0).p, 1000000.0f, 0, 0);
    // X shaft runs from the handle edge to just short of the arrow tip
    CheckVec(t.back(7).p, 6, 0, 0);
    CheckVec(t.back(6).p, 56, 0, 0);

    b.SetAxisTool(handles::Tool::Scale);
    b.SetActiveAxis(MSLAXIS::Y_AXIS);
    Program s;
    b.DrawHandles(&s, 1.0f, 0, true, false);
    CHECK(s.size() == 24 + cube + 3 * cube + 6 + 2);
    CheckVec(s.back(0).p, 0, 1000000.0f, 0);

    b.SetAxisTool(handles::Tool::Rotate);
    b.SetActiveAxis(MSLAXIS::Y_AXIS); // rotate shifts to Z
    Program r;
    b.DrawHandles(&r, 1.0f, 0, true, false);
    CHECK(r.size() == 24 + cube + 3 * 25 * 2 + 3 * cube + 2);
    CheckVec(r.back(0).p, 0, 0, 1000000.0f);

    b.SetAxisTool(handles::Tool::XYTranslate);
    b.SetActiveAxis(MSLAXIS::NO_AXIS);
    Program xy;
    b.DrawHandles(&xy, 1.0f, 0, true, false);
    CHECK(xy.size() == 24 + cube + 8 + 4 * 18 * 3);

    b.SetAxisTool(handles::Tool::Elevate);
    b.MouseOverHandle(MakeId(Role::AxisArrow, 0, Axis::X));
    Program e;
    b.DrawHandles(&e, 1.0f, 0, true, false);
    CHECK(e.size() == 24 + cube + 18 * 3 + 2);
    CHECK(e.back(2).c == xlYELLOW);
}

TEST_CASE("Gizmo highlight colours the hovered axis", "[screenloc]") {
    Box x(glm::vec3(0, 0, 0));
    auto& b = x.b;
    b.PrepareToDraw(true, true);
    b.SetActiveHandleToCentre();
    b.SetAxisTool(handles::Tool::Translate);
    b.MouseOverHandle(MakeId(Role::AxisArrow, 0, Axis::Y));
    Program t;
    b.DrawHandles(&t, 1.0f, 0, true, false);
    const size_t cube = CubeVertexCount();
    const size_t firstCone = 24 + cube;
    CHECK(t.at(firstCone).c == xlRED);
    CHECK(t.at(firstCone + 18 * 3).c == xlYELLOW);
    CHECK(t.at(firstCone + 2 * 18 * 3).c == xlBLUE);
}
