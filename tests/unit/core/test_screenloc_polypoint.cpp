#include <cmath>
#include <limits>
#include <memory>
#include <string>
#include <vector>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "models/Node.h"
#include "models/PolyPointScreenLocation.h"
#include "test_screenloc_common.h"

using namespace sl;
using Catch::Approx;
using handles::Axis;
using handles::Modifier;
using handles::Role;
using handles::UpdateResult;
using MSLAXIS = ModelScreenLocation::MSLAXIS;
using MSLPLANE = ModelScreenLocation::MSLPLANE;

namespace {

// Poly line with vertices (0,0,0) (10,0,0) (10,10,0) unless told otherwise.
struct Poly {
    PolyPointScreenLocation p;
    explicit Poly(glm::vec3 wp = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f),
                  const std::string& pts = "0,0,0,10,0,0,10,10,0", int n = 3, bool is3d = false) {
        p.SetNumPoints(n);
        p.SetDataFromString(pts);
        p.SetRenderSize(10.0f, 1.0f, 0.0f);
        p.SetWorldPosition(wp);
        p.SetScaleMatrix(scale);
        p.Init();
        Refresh(is3d);
    }
    void Refresh(bool is3d = false) {
        p.PrepareToDraw(is3d, true);
        std::vector<NodeBaseClassPtr> none;
        p.UpdateBoundingBox(none);
    }
    // world position of vertex i
    glm::vec3 W(int i) const {
        return p.GetWorldPosition() + p.GetPoint(i) * p.GetScaleMatrix();
    }
};

handles::WorldRay Flat() {
    handles::WorldRay r;
    r.origin = glm::vec3(0, 0, 0);
    r.direction = glm::vec3(1, 0, 0);
    return r;
}

} // namespace

// ---------------------------------------------------------------------------
// Point / curve data
// ---------------------------------------------------------------------------

TEST_CASE("Poly point data string round trips", "[screenloc]") {
    Poly x;
    CHECK(x.p.GetNumPoints() == 3);
    CHECK(x.p.GetNumHandles() == 3);
    CHECK(x.p.GetPointDataAsString() == "0.000000,0.000000,0.000000,10.000000,0.000000,0.000000,10.000000,10.000000,0.000000");
    CheckVec(x.p.GetPoint(2), 10, 10, 0);

    PolyPointScreenLocation q;
    q.SetNumPoints(3);
    q.SetDataFromString(x.p.GetPointDataAsString());
    CHECK(q.GetPointDataAsString() == x.p.GetPointDataAsString());
}

TEST_CASE("Poly point short data is padded with zeros", "[screenloc]") {
    PolyPointScreenLocation q;
    q.SetNumPoints(3);
    q.SetDataFromString("1.5,2,junk");
    CheckVec(q.GetPoint(0), 1.5f, 2, 0);
    CheckVec(q.GetPoint(1), 0, 0, 0);
    CheckVec(q.GetPoint(2), 0, 0, 0);
    q.SetNumPoints(0);
    CHECK(q.GetNumPoints() == 1);
}

TEST_CASE("Poly point curves are created removed and serialised", "[screenloc]") {
    Poly x;
    auto& p = x.p;
    CHECK(p.SupportsCurves());
    CHECK(p.GetCurveDataAsString() == "");
    p.SetCurve(0);
    CHECK(p.HasCurve(0));
    CHECK(p.HasSegmentCurve(0));
    CheckVec(p.GetCurveCp0(0), 0, 0, 0);
    CheckVec(p.GetCurveCp1(0), 10, 0, 0);
    CHECK(p.GetCurveDataAsString() == "0,0.000000,0.000000,0.000000,10.000000,0.000000,0.000000,");
    // creating again resets the controls onto the end points
    p.SetCurveCp0(0, glm::vec3(3, 4, 0));
    p.SetCurve(0, true);
    CheckVec(p.GetCurveCp0(0), 0, 0, 0);

    // out of range segments are ignored
    p.SetCurve(-1);
    p.SetCurve(2);
    CHECK_FALSE(p.HasSegmentCurve(2));
    CHECK_FALSE(p.HasSegmentCurve(-1));
    CheckVec(p.GetCurveCp1(1), 0, 0, 0);

    p.SetCurve(0, false);
    CHECK_FALSE(p.HasCurve(0));
    p.SetCurve(0, false);
    CHECK(p.GetCurveDataAsString() == "");

    p.Lock();
    p.SetCurve(1);
    CHECK_FALSE(p.HasCurve(1));
}

TEST_CASE("Poly point curve data string sets the control points", "[screenloc]") {
    Poly x;
    auto& p = x.p;
    p.SetCurveDataFromString("1,1,2,3,4,5,6,");
    CHECK_FALSE(p.HasCurve(0));
    REQUIRE(p.HasCurve(1));
    CheckVec(p.GetCurveCp0(1), 1, 2, 3);
    CheckVec(p.GetCurveCp1(1), 4, 5, 6);
    CHECK(p.GetCurveDataAsString() == "1,1.000000,2.000000,3.000000,4.000000,5.000000,6.000000,");
    // loading again reuses the existing curve
    p.SetCurveDataFromString("1,7,7,7,8,8,8");
    CheckVec(p.GetCurveCp0(1), 7, 7, 7);
    // incomplete records are ignored
    p.SetCurveDataFromString("0,1,2");
    CHECK_FALSE(p.HasCurve(0));
}

// cPointData comes straight from the rgbeffects file; a segment index with no
// following vertex must be ignored rather than index past mPos.
TEST_CASE("Poly point curve data for a segment past the end is ignored", "[screenloc]") {
    Poly x;
    x.p.SetCurveDataFromString("2,1,2,3,4,5,6");
    CHECK(x.p.GetCurveDataAsString() == "");
}

// ---------------------------------------------------------------------------
// Geometry
// ---------------------------------------------------------------------------

TEST_CASE("Poly point bounds follow the scaled vertex extent", "[screenloc]") {
    Poly x(glm::vec3(100, 50, 7), glm::vec3(2, 3, 1), "0,0,0,10,0,-2,10,10,4");
    auto& p = x.p;
    CHECK(p.GetLeft() == Approx(100.0f));
    CHECK(p.GetRight() == Approx(120.0f));
    CHECK(p.GetBottom() == Approx(50.0f));
    CHECK(p.GetTop() == Approx(80.0f));
    CHECK(p.GetBack() == Approx(5.0f));
    CHECK(p.GetFront() == Approx(11.0f));
    CHECK(p.GetMWidth() == Approx(10.0f));
    CHECK(p.GetMHeight() == Approx(10.0f));
    CHECK(p.GetMDepth() == Approx(6.0f));
    CHECK(p.GetHcenterPos() == Approx(110.0f));
    CHECK(p.GetVcenterPos() == Approx(65.0f));
    CHECK(p.GetDcenterPos() == Approx(8.0f));
    CHECK(p.GetMinX() == 0.0f);
    CHECK(p.GetMaxY() == 10.0f);
    CHECK_FALSE(p.IsCenterBased());

    // the unit cube maps onto the bounding box
    glm::vec3 lo(0, 0, 0), hi(1, 1, 1);
    p.TranslateVector(lo);
    p.TranslateVector(hi);
    CheckVec(lo, 100, 50, 5);
    CheckVec(hi, 120, 80, 11);

    // 2D drops depth from the segment matrices but keeps the extent
    p.PrepareToDraw(false, false);
    CHECK(p.GetFront() == Approx(11.0f));
}

TEST_CASE("Poly point edge and centre setters translate the whole line", "[screenloc]") {
    Poly x(glm::vec3(100, 50, 7), glm::vec3(2, 3, 1), "0,0,0,10,0,-2,10,10,4");
    auto& p = x.p;
    p.SetLeft(0.0f);
    CHECK(p.GetLeft() == Approx(0.0f));
    p.SetRight(50.0f);
    CHECK(p.GetRight() == Approx(50.0f));
    p.SetBottom(-10.0f);
    CHECK(p.GetBottom() == Approx(-10.0f));
    p.SetTop(100.0f);
    CHECK(p.GetTop() == Approx(100.0f));
    p.SetFront(0.0f);
    CHECK(p.GetFront() == Approx(0.0f));
    p.SetBack(1.0f);
    CHECK(p.GetBack() == Approx(1.0f));
    p.SetPosition(0.0f, 0.0f);
    CHECK(p.GetHcenterPos() == Approx(0.0f));
    CHECK(p.GetVcenterPos() == Approx(0.0f));
    p.SetDcenterPos(3.0f);
    CHECK(p.GetDcenterPos() == Approx(3.0f));
    CHECK(p.GetMWidth() == Approx(10.0f));

    p.Lock();
    p.SetLeft(500.0f);
    p.SetRight(500.0f);
    p.SetTop(500.0f);
    p.SetBottom(500.0f);
    p.SetFront(500.0f);
    p.SetBack(500.0f);
    p.SetPosition(500.0f, 500.0f);
    CHECK(p.GetHcenterPos() == Approx(0.0f));
    CHECK(p.GetVcenterPos() == Approx(0.0f));
    CHECK(p.GetDcenterPos() == Approx(3.0f));
}

TEST_CASE("Poly point size setters change the scale", "[screenloc]") {
    Poly x(glm::vec3(0.0f), glm::vec3(1.0f), "0,0,0,10,0,-2,10,10,4");
    auto& p = x.p;
    p.SetMWidth(40.0f);
    p.SetMHeight(5.0f);
    p.SetMDepth(12.0f);
    CheckVec(p.GetScaleMatrix(), 4, 0.5f, 2);
    p.SetMHeight(0.0f);
    CHECK(p.GetScaleMatrix().y == 1.0f);

    Poly flat(glm::vec3(0.0f), glm::vec3(3.0f), "5,5,5,5,5,5", 2);
    flat.p.SetMWidth(40.0f);
    flat.p.SetMHeight(40.0f);
    flat.p.SetMDepth(40.0f);
    CheckVec(flat.p.GetScaleMatrix(), 1, 1, 1);
}

TEST_CASE("Poly point Init repairs bad position and scale", "[screenloc]") {
    PolyPointScreenLocation p;
    p.SetWorldPosition(glm::vec3(std::numeric_limits<float>::quiet_NaN(), 1.0f, std::numeric_limits<float>::quiet_NaN()));
    p.SetScaleMatrix(glm::vec3(-1.0f, std::numeric_limits<float>::infinity(), 0.0f));
    p.Init();
    CheckVec(p.GetWorldPosition(), 0, 1, 0);
    CheckVec(p.GetScaleMatrix(), 1, 1, 1);
    CHECK(p.GetNumSelectableHandles() == 3);
}

TEST_CASE("Poly point hit test finds straight segments and the bounding area", "[screenloc]") {
    Poly x(glm::vec3(100, 100, 0), glm::vec3(1.0f), "0,0,0,10,0,0", 2);
    auto& p = x.p;
    glm::vec3 d(0, 0, -1);
    glm::vec3 nearPt(105, 103, 0), farPt(105, 106, 0), before(97, 100, 0);
    CHECK(p.HitTest(nearPt, d));
    CHECK_FALSE(p.HitTest(farPt, d));
    CHECK_FALSE(p.HitTest(before, d));

    // inside the vertex bounding rectangle counts as a hit
    Poly l;
    glm::vec3 inside(3, 8, 0), outside(-6, 8, 0);
    CHECK(l.p.HitTest(inside, d));
    CHECK_FALSE(l.p.HitTest(outside, d));

    Poly z(glm::vec3(100, 100, 0), glm::vec3(1.0f), "0,0,0,10,0,0", 2, true);
    float dist = 1e9f;
    glm::vec3 above(105, 100, 100), off(105, 107, 100);
    CHECK(z.p.HitTest3D(above, d, dist));
    CHECK(dist == Approx(95.0f).margin(1e-2));
    CHECK_FALSE(z.p.HitTest3D(off, d, dist));
}

TEST_CASE("Poly point hit test follows a curved segment", "[screenloc]") {
    Poly x(glm::vec3(0.0f), glm::vec3(1.0f), "0,0,0,10,0,0", 2);
    auto& p = x.p;
    p.SetCurve(0);
    p.SetCurveCp0(0, glm::vec3(0, 10, 0));
    p.SetCurveCp1(0, glm::vec3(10, 10, 0));
    // the curve samples feeding the extent are rebuilt during the pass, so
    // the bounding area catches up on the next one
    x.Refresh();
    x.Refresh();
    CHECK(p.GetMaxY() == Approx(7.5f).margin(0.1));
    glm::vec3 d(0, 0, -1);
    glm::vec3 nearPeak(5, 10, 0), farAbove(5, 14, 0);
    CHECK(p.HitTest(nearPeak, d));
    CHECK_FALSE(p.HitTest(farAbove, d));

    Poly y(glm::vec3(0.0f), glm::vec3(1.0f), "0,0,0,10,0,0", 2, true);
    y.p.SetCurve(0);
    y.p.SetCurveCp0(0, glm::vec3(0, 10, 0));
    y.p.SetCurveCp1(0, glm::vec3(10, 10, 0));
    y.Refresh(true);
    y.Refresh(true);
    float dist = 1e9f;
    glm::vec3 above(5, 7.5f, 100), wide(5, 20, 100);
    CHECK(y.p.HitTest3D(above, d, dist));
    CHECK_FALSE(y.p.HitTest3D(wide, d, dist));
}

TEST_CASE("Poly point containment uses the vertex bounding box", "[screenloc]") {
    Poly x(glm::vec3(100, 100, 0));
    FakePreview fp;
    CHECK(x.p.IsContained(&fp, 90, 90, 120, 120));
    CHECK_FALSE(x.p.IsContained(&fp, 101, 90, 120, 120));
    CHECK_FALSE(x.p.IsContained(&fp, 90, 90, 120, 105));
    Poly y(glm::vec3(100, 100, 0), glm::vec3(1.0f), "0,0,0,10,0,0,10,10,0", 3, true);
    CHECK(y.p.IsContained(&fp, 90, 480, 120, 510));
    CHECK_FALSE(y.p.IsContained(&fp, 90, 90, 120, 120));
}

TEST_CASE("Poly point screen offset and dimension", "[screenloc]") {
    Poly x(glm::vec3(195, 145, 0));
    FakePreview fp;
    glm::vec2 off = x.p.GetScreenOffset(&fp);
    CHECK(off.x == Approx(0.25f));
    CHECK(off.y == Approx(0.75f));
    CHECK(x.p.GetDimension() == "");
    ScopedRuler r(100.0f, 2.0f);
    CHECK(x.p.GetDimension() == "Length 0.40m");
}

// ---------------------------------------------------------------------------
// Vertex editing
// ---------------------------------------------------------------------------

TEST_CASE("Poly point AddHandle appends a vertex under the mouse", "[screenloc]") {
    Poly x(glm::vec3(100, 0, 0), glm::vec3(2, 2, 1));
    FakePreview fp;
    x.p.AddHandle(&fp, 120, 570);
    REQUIRE(x.p.GetNumPoints() == 4);
    CheckVec(x.p.GetPoint(3), 10, 15, 0);
    CHECK(x.p.GetNumSelectableHandles() == 5);

    Poly y(glm::vec3(100, 0, 0), glm::vec3(2, 2, 1), "0,0,0,10,0,0,10,10,0", 3, true);
    y.p.SetActiveAxis(MSLAXIS::X_AXIS);
    y.p.AddHandle(&fp, 120, 570);
    REQUIRE(y.p.GetNumPoints() == 4);
    CheckVec(y.p.GetPoint(3), 10, 15, 0);
}

TEST_CASE("Poly point InsertHandle splits a segment at its midpoint", "[screenloc]") {
    Poly x(glm::vec3(0.0f), glm::vec3(1.0f), "0,0,0,10,0,4,10,10,0");
    auto& p = x.p;
    p.SelectSegment(1);
    p.InsertHandle(0, 1.0f, 0);
    REQUIRE(p.GetNumPoints() == 4);
    CheckVec(p.GetPoint(1), 5, 0, 2);
    CheckVec(p.GetPoint(2), 10, 0, 4);
    CHECK(p.GetSelectedSegment() == -1);
    REQUIRE(p.GetSelectedHandleId().has_value());
    CHECK(p.GetSelectedHandleId()->role == Role::Vertex);
    p.InsertHandle(3, 1.0f, 0);
    p.InsertHandle(-1, 1.0f, 0);
    CHECK(p.GetNumPoints() == 4);
}

TEST_CASE("Poly point DeleteHandle removes a vertex and its curves", "[screenloc]") {
    Poly x(glm::vec3(0.0f), glm::vec3(1.0f), "0,0,0,10,0,0,10,10,0,20,10,0", 4);
    auto& p = x.p;
    p.SetCurve(0);
    p.SetCurve(1);
    p.SetCurve(2);
    p.SetSelectedHandle(MakeId(Role::Vertex, 1));
    p.DeleteHandle(1);
    REQUIRE(p.GetNumPoints() == 3);
    CheckVec(p.GetPoint(1), 10, 10, 0);
    CHECK_FALSE(p.HasCurve(0));
    CHECK(p.HasCurve(1));
    CHECK_FALSE(p.GetSelectedHandleId().has_value());

    p.DeleteHandle(0);
    CHECK(p.GetNumPoints() == 2);
    p.DeleteHandle(0);
    CHECK(p.GetNumPoints() == 2);
    p.DeleteHandle(9);
    CHECK(p.GetNumPoints() == 2);
}

TEST_CASE("Poly point SwapStartEnd reverses vertices and curves", "[screenloc]") {
    Poly x;
    auto& p = x.p;
    p.SetCurve(0);
    p.SetCurveCp0(0, glm::vec3(2, 5, 0));
    p.SetCurveCp1(0, glm::vec3(8, 5, 0));
    p.SwapStartEnd();
    CheckVec(p.GetPoint(0), 10, 10, 0);
    CheckVec(p.GetPoint(1), 10, 0, 0);
    CheckVec(p.GetPoint(2), 0, 0, 0);
    CHECK_FALSE(p.HasCurve(0));
    REQUIRE(p.HasCurve(1));
    CheckVec(p.GetCurveCp0(1), 8, 5, 0);
    CheckVec(p.GetCurveCp1(1), 2, 5, 0);

    p.Lock();
    p.SwapStartEnd();
    CheckVec(p.GetPoint(0), 10, 10, 0);
}

TEST_CASE("Poly point rotate about a point turns every vertex and control point", "[screenloc]") {
    Poly x(glm::vec3(10, 0, 0), glm::vec3(1.0f), "0,0,0,10,0,0", 2);
    auto& p = x.p;
    p.SetCurve(0);
    p.SetCurveCp0(0, glm::vec3(15, 5, 0));
    p.RotateAboutPoint(glm::vec3(0, 0, 0), glm::vec3(0, 0, 90));
    CheckVec(x.W(0), 0, 10, 0);
    CheckVec(x.W(1), 0, 20, 0);
    CheckVec(p.GetCurveCp0(0) + p.GetWorldPosition(), -5, 15, 0);

    Poly y(glm::vec3(0.0f), glm::vec3(1.0f), "0,0,0,10,0,0", 2);
    y.p.RotateAboutPoint(glm::vec3(0, 0, 0), glm::vec3(0, 90, 0));
    CheckVec(y.W(1), 0, 0, -10);
    y.p.RotateAboutPoint(glm::vec3(0, 0, 0), glm::vec3(90, 0, 0));
    CheckVec(y.W(1), 0, 10, 0);
    y.p.SetRotatePoint(glm::vec3(0.0f));
    CHECK(y.p.Rotate(MSLAXIS::NO_AXIS, 45.0f));
    CheckVec(y.W(1), 0, 10, 0);

    y.p.Lock();
    CHECK_FALSE(y.p.Rotate(MSLAXIS::Z_AXIS, 45.0f));
    y.p.RotateAboutPoint(glm::vec3(0, 0, 0), glm::vec3(0, 0, 90));
    CheckVec(y.W(1), 0, 10, 0);
}

// Scaling (0,0)-(10,0)-(10,10) placed at x=100 by 2 about its centre puts
// vertex 0 at (95,-5) (the world offset used to be applied twice).
TEST_CASE("Poly point Scale grows the line about its centre", "[screenloc]") {
    Poly x(glm::vec3(100, 0, 0));
    CHECK(x.p.Scale(glm::vec3(2, 2, 1)));
    CheckVec(x.W(0), 95, -5, 0);
    CheckVec(x.W(2), 115, 15, 0);
}

TEST_CASE("Poly point Scale is refused when locked", "[screenloc]") {
    Poly x;
    x.p.Lock();
    CHECK_FALSE(x.p.Scale(glm::vec3(2.0f)));
    CheckVec(x.W(2), 10, 10, 0);
}

TEST_CASE("Poly point InitializeLocation starts a two vertex line at the click", "[screenloc]") {
    PolyPointScreenLocation p;
    p.SetDataFromString("5,5,5,6,6,6");
    FakePreview fp;
    std::vector<NodeBaseClassPtr> nodes;
    int handle = -1;
    CHECK(p.InitializeLocation(handle, 30, 100, nodes, &fp) == CursorType::Sizing);
    CHECK(handle == 2);
    CheckVec(p.GetWorldPosition(), 30, 500, 0);
    CheckVec(p.GetPoint(0), 0, 0, 0);
    CheckVec(p.GetPoint(1), 0, 0, 0);
    CHECK_FALSE(p.GetActiveHandleId().has_value());
    fp.is3d = true;
    p.InitializeLocation(handle, 30, 100, nodes, &fp);
    CHECK(*p.GetActiveHandleId() == MakeId(Role::Vertex, 1));
}

// ---------------------------------------------------------------------------
// Handles, tools and selection
// ---------------------------------------------------------------------------

TEST_CASE("Poly point selection and tool rules", "[screenloc]") {
    Poly x;
    auto& p = x.p;
    CHECK(p.GetDefaultTool() == handles::Tool::XYTranslate);
    p.SetActiveHandleToDefault();
    CHECK(*p.GetActiveHandleId() == MakeId(Role::Vertex, 1));
    CHECK(*p.GetSelectedHandleId() == MakeId(Role::Vertex, 1));
    p.SetActiveHandle(MakeId(Role::AxisArrow, 1, Axis::Y));
    CHECK(*p.GetActiveHandleId() == MakeId(Role::Vertex, 1));

    // a vertex only allows translate tools
    p.SetAxisTool(handles::Tool::Scale);
    CHECK(p.GetAxisTool() == handles::Tool::Translate);
    p.SetAxisTool(handles::Tool::XYTranslate);
    CHECK(p.GetAxisTool() == handles::Tool::XYTranslate);
    p.AdvanceAxisTool();
    CHECK(p.GetAxisTool() == handles::Tool::Translate);
    p.SetAxisTool(handles::Tool::Rotate);
    p.SetActiveAxis(MSLAXIS::X_AXIS);
    CHECK(p.GetActiveAxis() == MSLAXIS::X_AXIS);

    // the centre allows every tool
    p.SetActiveHandleToCentre();
    p.SetAxisTool(handles::Tool::Scale);
    CHECK(p.GetAxisTool() == handles::Tool::Scale);
    p.AdvanceAxisTool();
    CHECK(p.GetAxisTool() == handles::Tool::Rotate);

    p.SelectSegment(-1);
    CHECK(p.GetSelectedHandleId().has_value());
    p.SelectSegment(1);
    CHECK(p.GetSelectedSegment() == 1);
    CHECK_FALSE(p.GetSelectedHandleId().has_value());
    p.SetSelectedHandle(MakeId(Role::Vertex, 2));
    p.SelectHandle();
    CHECK_FALSE(p.GetSelectedHandleId().has_value());
    p.SetSelectedSegment(0);
    CHECK(p.GetSelectedSegment() == 0);
}

TEST_CASE("Poly point active sub handle position", "[screenloc]") {
    Poly x(glm::vec3(100, 50, 0), glm::vec3(2, 2, 1));
    auto& p = x.p;
    CheckVec(p.GetActiveSubHandleWorldPos(), 0, 0, 0);
    p.SetActiveHandle(MakeId(Role::Vertex, 2));
    CheckVec(p.GetActiveSubHandleWorldPos(), 120, 70, 0);
    p.SetActiveHandle(MakeId(Role::Vertex, 7));
    CheckVec(p.GetActiveSubHandleWorldPos(), 0, 0, 0);
    p.SetActiveHandleToCentre();
    CheckVec(p.GetActiveSubHandleWorldPos(), 110, 60, 0);
    p.SetActiveHandle(MakeId(Role::CurveControl, 1, Axis::X, 0));
    CheckVec(p.GetActiveSubHandleWorldPos(), 0, 0, 0);
    p.SetCurve(0);
    CheckVec(p.GetActiveSubHandleWorldPos(), 120, 50, 0);
    p.SetActiveHandle(MakeId(Role::CurveControl, 0, Axis::X, 0));
    CheckVec(p.GetActiveSubHandleWorldPos(), 100, 50, 0);
    p.SetActiveHandle(MakeId(Role::Segment, 0));
    CheckVec(p.GetActiveSubHandleWorldPos(), 0, 0, 0);
}

TEST_CASE("Poly point 2D handles are vertices segments controls and resize corners", "[screenloc]") {
    Poly x(glm::vec3(100, 50, 0));
    auto& p = x.p;
    auto hs = p.GetHandles(handles::ViewMode::TwoD, handles::Tool::Translate);
    CHECK(CountRole(hs, Role::Vertex) == 3);
    CHECK(CountRole(hs, Role::Segment) == 2);
    CHECK(CountRole(hs, Role::ResizeCorner) == 4);
    CHECK(hs.size() == 9);
    CheckVec(Find(hs, MakeId(Role::Vertex, 1))->worldPos, 110, 50, 0);
    const auto* seg = Find(hs, MakeId(Role::Segment, 1));
    REQUIRE(seg);
    CheckVec(seg->worldPos, 110, 50, 0);
    CheckVec(seg->endPos, 110, 60, 0);
    CHECK(seg->selectionOnly);
    CheckVec(Find(hs, MakeId(Role::ResizeCorner, 0))->worldPos, 87.5f, 37.5f, 0);
    CheckVec(Find(hs, MakeId(Role::ResizeCorner, 1))->worldPos, 87.5f, 72.5f, 0);
    CheckVec(Find(hs, MakeId(Role::ResizeCorner, 2))->worldPos, 122.5f, 37.5f, 0);
    CheckVec(Find(hs, MakeId(Role::ResizeCorner, 3))->worldPos, 122.5f, 72.5f, 0);

    p.SetCurve(0);
    p.SetCurveCp0(0, glm::vec3(103, 55, 0));
    x.Refresh();
    hs = p.GetHandles(handles::ViewMode::TwoD, handles::Tool::Translate);
    CHECK(CountRole(hs, Role::CurveControl) == 2);
    CHECK(CountRole(hs, Role::Segment) > 2);
    CheckVec(Find(hs, MakeId(Role::CurveControl, 0, Axis::X, 0))->worldPos, 103, 55, 0);
    CheckVec(Find(hs, MakeId(Role::CurveControl, 1, Axis::X, 0))->worldPos, 110, 50, 0);

    p.Lock();
    hs = p.GetHandles(handles::ViewMode::TwoD, handles::Tool::Translate);
    for (const auto& d : hs) CHECK_FALSE(d.editable);
}

TEST_CASE("Poly point 3D handles and per handle gizmos", "[screenloc]") {
    Poly x(glm::vec3(100, 50, 0), glm::vec3(1.0f), "0,0,0,10,0,0,10,10,0", 3, true);
    auto& p = x.p;
    auto hs = p.GetHandles(handles::ViewMode::ThreeD, handles::Tool::Translate);
    CHECK(hs.size() == 1 + 3 + 2);
    const auto* c = Find(hs, MakeId(Role::CentreCycle, CENTER_HANDLE));
    REQUIRE(c);
    CheckVec(c->worldPos, 105, 55, 0);
    CHECK(Find(hs, MakeId(Role::Vertex, 0))->selectionOnly);
    hs = p.GetHandles(handles::ViewMode::ThreeD, handles::Tool::XYTranslate);
    CHECK_FALSE(Find(hs, MakeId(Role::Vertex, 0))->selectionOnly);

    p.SetActiveHandleToCentre();
    hs = p.GetHandles(handles::ViewMode::ThreeD, handles::Tool::Translate);
    CheckVec(Find(hs, MakeId(Role::AxisArrow, -1, Axis::X))->worldPos, 159, 55, 0);
    hs = p.GetHandles(handles::ViewMode::ThreeD, handles::Tool::Scale);
    CHECK(CountRole(hs, Role::AxisCube) == 3);
    hs = p.GetHandles(handles::ViewMode::ThreeD, handles::Tool::Rotate);
    CHECK(CountRole(hs, Role::AxisRing) == 3);
    hs = p.GetHandles(handles::ViewMode::ThreeD, handles::Tool::XYTranslate);
    CHECK(CountRole(hs, Role::AxisArrow) == 0);

    p.SetActiveHandle(MakeId(Role::Vertex, 2));
    hs = p.GetHandles(handles::ViewMode::ThreeD, handles::Tool::Translate);
    CheckVec(Find(hs, MakeId(Role::AxisArrow, 2, Axis::Y))->worldPos, 110, 114, 0);
    hs = p.GetHandles(handles::ViewMode::ThreeD, handles::Tool::Scale);
    CHECK(CountRole(hs, Role::AxisCube) == 0);
    p.SetActiveHandle(MakeId(Role::Vertex, 7));
    hs = p.GetHandles(handles::ViewMode::ThreeD, handles::Tool::Translate);
    CHECK(CountRole(hs, Role::AxisArrow) == 0);
    p.SetActiveHandle(MakeId(Role::Segment, 0));
    hs = p.GetHandles(handles::ViewMode::ThreeD, handles::Tool::Translate);
    CHECK(CountRole(hs, Role::AxisArrow) == 0);

    p.SetCurve(1);
    x.Refresh(true);
    p.SetActiveHandle(MakeId(Role::CurveControl, 0, Axis::X, 1));
    hs = p.GetHandles(handles::ViewMode::ThreeD, handles::Tool::Translate);
    CHECK(CountRole(hs, Role::CurveControl) == 2);
    CHECK(Find(hs, MakeId(Role::AxisArrow, 0, Axis::Z, 1)) != nullptr);
    p.SetActiveHandle(MakeId(Role::CurveControl, 1, Axis::X, 1));
    hs = p.GetHandles(handles::ViewMode::ThreeD, handles::Tool::Translate);
    CheckVec(Find(hs, MakeId(Role::AxisArrow, 1, Axis::X, 1))->worldPos, 164, 60, 0);
    p.SetActiveHandle(std::nullopt);
    CHECK(CountRole(p.GetHandles(handles::ViewMode::ThreeD, handles::Tool::Translate), Role::AxisArrow) == 0);
}

// ---------------------------------------------------------------------------
// Drag sessions
// ---------------------------------------------------------------------------

TEST_CASE("Poly point drag sessions reject bad handles", "[screenloc]") {
    Poly x;
    auto& p = x.p;
    CHECK(p.CreateDragSession("p", MakeId(Role::Vertex, 3), DownZ(0, 0)) == nullptr);
    CHECK(p.CreateDragSession("p", MakeId(Role::Vertex, -1), DownZ(0, 0)) == nullptr);
    CHECK(p.CreateDragSession("p", MakeId(Role::CurveControl, 0, Axis::X, 0), DownZ(0, 0)) == nullptr);
    CHECK(p.CreateDragSession("p", MakeId(Role::CurveControl, 0, Axis::X, 5), DownZ(0, 0)) == nullptr);
    CHECK(p.CreateDragSession("p", MakeId(Role::AxisArrow, 0, Axis::X, 5), DownZ(0, 0)) == nullptr);
    CHECK(p.CreateDragSession("p", MakeId(Role::AxisArrow, 0, Axis::X, 0), DownZ(0, 0)) == nullptr);
    p.SetCurve(0);
    CHECK(p.CreateDragSession("p", MakeId(Role::AxisArrow, 2, Axis::X, 0), DownZ(0, 0)) == nullptr);
    CHECK(p.CreateDragSession("p", MakeId(Role::AxisArrow, 7, Axis::X), DownZ(0, 0)) == nullptr);
    CHECK(p.CreateDragSession("p", MakeId(Role::AxisCube, 0), DownZ(0, 0)) == nullptr);
    CHECK(p.CreateDragSession("p", MakeId(Role::AxisRing, 0), DownZ(0, 0)) == nullptr);
    CHECK(p.CreateDragSession("p", MakeId(Role::ResizeCorner, 4), DownZ(0, 0)) == nullptr);
    CHECK(p.CreateDragSession("p", MakeId(Role::Segment, 0), DownZ(0, 0)) == nullptr);
    CHECK(p.BeginExtend("p", DownZ(0, 0), handles::ViewMode::TwoD, 3) == nullptr);
    CHECK(p.BeginExtend("p", DownZ(0, 0), handles::ViewMode::TwoD, -1) == nullptr);
    p.Lock();
    CHECK(p.CreateDragSession("p", MakeId(Role::Vertex, 0), DownZ(0, 0)) == nullptr);
    CHECK(p.BeginCreate("p", DownZ(0, 0), handles::ViewMode::TwoD) == nullptr);
    CHECK(p.BeginExtend("p", DownZ(0, 0), handles::ViewMode::TwoD, 1) == nullptr);
}

TEST_CASE("Poly point 2D vertex and control point drags", "[screenloc]") {
    Poly x(glm::vec3(100, 50, 0), glm::vec3(2, 2, 1));
    auto& p = x.p;
    auto v = p.CreateDragSession("poly", MakeId(Role::Vertex, 1), DownZ(120, 50));
    REQUIRE(v);
    CHECK(v->GetHandleId() == MakeId(Role::Vertex, 1));
    CHECK(v->Update(DownZ(120, 70), Modifier::None) == UpdateResult::NeedsInit);
    CheckVec(p.GetPoint(1), 10, 10, 0);
    auto c = v->Commit();
    CHECK(c.modelName == "poly");
    CHECK(c.dirty == handles::DirtyField::Vertex);
    v->Revert();
    CheckVec(p.GetPoint(1), 10, 0, 0);
    CHECK(v->Commit().dirty == handles::DirtyField::None);

    p.SetCurve(0);
    auto cp0 = p.CreateDragSession("poly", MakeId(Role::CurveControl, 0, Axis::X, 0), DownZ(0, 0));
    REQUIRE(cp0);
    // 2D rays carry the cursor in origin.xy; keep z on the model plane
    cp0->Update(DownZ(110, 60, 0.0f), Modifier::None);
    CheckVec(p.GetCurveCp0(0), 5, 5, 0);
    CHECK(cp0->Commit().dirty == handles::DirtyField::Curve);
    cp0->Revert();
    CheckVec(p.GetCurveCp0(0), 0, 0, 0);

    auto cp1 = p.CreateDragSession("poly", MakeId(Role::CurveControl, 1, Axis::X, 0), DownZ(0, 0));
    cp1->Update(DownZ(104, 58, 0.0f), Modifier::None);
    CheckVec(p.GetCurveCp1(0), 2, 4, 0);
    cp1->Revert();
    CheckVec(p.GetCurveCp1(0), 10, 0, 0);

    // moving a vertex keeps the attached curve's end on it
    auto v0 = p.CreateDragSession("poly", MakeId(Role::Vertex, 0), DownZ(0, 0));
    v0->Update(DownZ(90, 40), Modifier::None);
    CheckVec(p.GetPoint(0), -5, -5, 0);
    x.Refresh();
    CheckVec(Find(p.GetHandles(handles::ViewMode::TwoD, handles::Tool::Translate), MakeId(Role::Vertex, 0))->worldPos, 90, 40, 0);
}

// Like the 2D vertex drag, a 2D control-point drag keeps the point on the
// model plane instead of taking the ray's near-plane depth.
TEST_CASE("Poly point 2D control point drag keeps the control point on the model plane", "[screenloc]") {
    Poly x;
    x.p.SetCurve(0);
    auto cp0 = x.p.CreateDragSession("poly", MakeId(Role::CurveControl, 0, Axis::X, 0), DownZ(0, 0));
    cp0->Update(DownZ(5, 5, 1000.0f), Modifier::None);
    CheckVec(x.p.GetCurveCp0(0), 5, 5, 0);
}

TEST_CASE("Poly point vertex drags survive a zero scale", "[screenloc]") {
    Poly x(glm::vec3(0.0f), glm::vec3(1.0f));
    x.p.SetScaleMatrix(glm::vec3(0, 0, 0));
    auto v = x.p.CreateDragSession("poly", MakeId(Role::Vertex, 1), DownZ(0, 0));
    v->Update(DownZ(1, 1), Modifier::None);
    CheckVec(x.p.GetPoint(1), 1000, 1000, 0, 0.5f);
}

TEST_CASE("Poly point 3D vertex drag follows the active plane", "[screenloc]") {
    Poly x(glm::vec3(0, 0, 5), glm::vec3(1.0f), "0,0,0,10,0,0,10,10,0", 3, true);
    auto& p = x.p;

    auto xy = p.CreateDragSession("poly", MakeId(Role::Vertex, 1), DownZ(10, 0));
    REQUIRE(xy);
    CHECK(xy->Update(DownZ(12, 3), Modifier::None) == UpdateResult::NeedsInit);
    CheckVec(p.GetPoint(1), 12, 3, 0);
    CHECK(xy->Update(Flat(), Modifier::None) == UpdateResult::Unchanged);
    CHECK(xy->Commit().dirty == handles::DirtyField::Vertex);
    xy->Revert();
    CheckVec(p.GetPoint(1), 10, 0, 0);

    p.SetActivePlane(MSLPLANE::XZ_PLANE);
    auto xz = p.CreateDragSession("poly", MakeId(Role::Vertex, 1), DownY(10, 5));
    xz->Update(DownY(14, 9), Modifier::None);
    CheckVec(p.GetPoint(1), 14, 0, 4);
    xz->Revert();

    p.SetActivePlane(MSLPLANE::YZ_PLANE);
    auto yz = p.CreateDragSession("poly", MakeId(Role::Vertex, 1), DownX(0, 5));
    yz->Update(DownX(6, 8), Modifier::None);
    CheckVec(p.GetPoint(1), 10, 6, 3);
    yz->Revert();

    // a start ray that misses the plane anchors on the vertex itself
    p.SetActivePlane(MSLPLANE::XY_PLANE);
    auto miss = p.CreateDragSession("poly", MakeId(Role::Vertex, 1), Flat());
    miss->Update(DownZ(11, 1), Modifier::None);
    CheckVec(p.GetPoint(1), 11, 1, 0);
}

TEST_CASE("Poly point creation and extension drag a vertex", "[screenloc]") {
    Poly x(glm::vec3(100, 50, 0), glm::vec3(2, 2, 1));
    auto& p = x.p;
    auto c = p.BeginCreate("poly", DownZ(100, 50), handles::ViewMode::TwoD);
    REQUIRE(c);
    CHECK(c->GetHandleId() == MakeId(Role::Vertex, 1));
    c->Update(DownZ(140, 30), Modifier::None);
    CheckVec(p.GetPoint(1), 20, -10, 0);
    c->Revert();
    CheckVec(p.GetPoint(1), 10, 0, 0);

    auto e = p.BeginExtend("poly", DownZ(0, 0), handles::ViewMode::ThreeD, 2);
    REQUIRE(e);
    e->Update(DownZ(104, 56), Modifier::None);
    CheckVec(p.GetPoint(2), 2, 3, 0);
    // the vertex disappeared under the session
    p.DeleteHandle(2);
    CHECK(e->Update(DownZ(104, 56), Modifier::None) == UpdateResult::Unchanged);
}

TEST_CASE("Poly point 3D axis arrows move one vertex or control point", "[screenloc]") {
    Poly x(glm::vec3(100, 50, 0), glm::vec3(2, 2, 1), "0,0,0,10,0,0,10,10,0", 3, true);
    auto& p = x.p;
    p.SetActiveHandle(MakeId(Role::Vertex, 1));
    auto v = p.CreateDragSession("poly", MakeId(Role::AxisArrow, 1, Axis::Y), DownZ(120, 50));
    REQUIRE(v);
    CHECK(v->Update(DownZ(150, 60), Modifier::None) == UpdateResult::NeedsInit);
    CheckVec(p.GetPoint(1), 10, 5, 0);
    CHECK(v->Update(Flat(), Modifier::None) == UpdateResult::Unchanged);
    CHECK(v->Commit().dirty == handles::DirtyField::Vertex);
    v->Revert();
    CheckVec(p.GetPoint(1), 10, 0, 0);
    CHECK(v->Commit().dirty == handles::DirtyField::None);
    auto vx = p.CreateDragSession("poly", MakeId(Role::AxisArrow, 1, Axis::X), DownZ(120, 50));
    vx->Update(DownZ(130, 0), Modifier::None);
    CheckVec(p.GetPoint(1), 15, 0, 0);
    vx->Revert();
    auto vz = p.CreateDragSession("poly", MakeId(Role::AxisArrow, 1, Axis::Z), DownY(120, 0));
    vz->Update(DownY(0, 8), Modifier::None);
    CheckVec(p.GetPoint(1), 10, 0, 8);
    vz->Revert();

    p.SetCurve(0);
    p.SetActiveHandle(MakeId(Role::CurveControl, 1, Axis::X, 0));
    auto cp = p.CreateDragSession("poly", MakeId(Role::AxisArrow, 1, Axis::X, 0), DownZ(120, 50));
    REQUIRE(cp);
    CHECK(cp->Update(DownZ(110, 0), Modifier::None) == UpdateResult::NeedsInit);
    CheckVec(p.GetCurveCp1(0), 5, 0, 0);
    CHECK(cp->Update(Flat(), Modifier::None) == UpdateResult::Unchanged);
    CHECK(cp->Commit().dirty == handles::DirtyField::Curve);
    cp->Revert();
    CheckVec(p.GetCurveCp1(0), 10, 0, 0);
    CHECK(cp->Commit().dirty == handles::DirtyField::None);

    p.SetActiveHandle(MakeId(Role::CurveControl, 0, Axis::X, 0));
    auto cy = p.CreateDragSession("poly", MakeId(Role::AxisArrow, 0, Axis::Y, 0), DownZ(100, 50));
    cy->Update(DownZ(100, 56), Modifier::None);
    CheckVec(p.GetCurveCp0(0), 0, 3, 0);
    cy->Revert();
    auto cz = p.CreateDragSession("poly", MakeId(Role::AxisArrow, 0, Axis::Z, 0), DownY(100, 0));
    cz->Update(DownY(100, 4), Modifier::None);
    CheckVec(p.GetCurveCp0(0), 0, 0, 4);
    cz->Revert();
}

TEST_CASE("Poly point centre arrow translates the whole line", "[screenloc]") {
    Poly x(glm::vec3(0.0f), glm::vec3(1.0f), "0,0,0,10,0,0,10,10,0", 3, true);
    auto& p = x.p;
    p.SetActiveHandleToCentre();
    auto s = p.CreateDragSession("poly", MakeId(Role::AxisArrow, -1, Axis::X), DownZ(5, 5));
    REQUIRE(s);
    CHECK(s->Update(DownZ(12, 9), Modifier::None) == UpdateResult::Updated);
    CheckVec(p.GetWorldPosition(), 7, 0, 0);
    CHECK(s->Update(DownZ(12, 0), Modifier::None) == UpdateResult::Unchanged);
    CHECK(s->Update(Flat(), Modifier::None) == UpdateResult::Unchanged);
    CHECK(s->Commit().dirty == handles::DirtyField::Position);
    s->Revert();
    CheckVec(p.GetWorldPosition(), 0, 0, 0);
    CHECK(s->Commit().dirty == handles::DirtyField::None);

    auto y = p.CreateDragSession("poly", MakeId(Role::AxisArrow, -1, Axis::Y), DownZ(5, 5));
    y->Update(DownZ(5, 1), Modifier::None);
    CheckVec(p.GetWorldPosition(), 0, -4, 0);
    y->Revert();
    auto z = p.CreateDragSession("poly", MakeId(Role::AxisArrow, -1, Axis::Z), DownY(5, 0));
    z->Update(DownY(5, 3), Modifier::None);
    CheckVec(p.GetWorldPosition(), 0, 0, 3);
    z->Revert();
    auto m = p.CreateDragSession("poly", MakeId(Role::AxisArrow, -1, Axis::X), Flat());
    m->Update(DownZ(1, 0), Modifier::None);
    CHECK(p.GetWorldPosition().x == Approx(1.0f));
}

TEST_CASE("Poly point centre cube scales about the centre", "[screenloc]") {
    Poly x(glm::vec3(0.0f), glm::vec3(1.0f), "0,0,0,10,0,0,10,10,0", 3, true);
    auto& p = x.p;
    auto s = p.CreateDragSession("poly", MakeId(Role::AxisCube, -1, Axis::X), DownZ(5, 5));
    REQUIRE(s);
    CHECK(s->Update(DownZ(10, 5), Modifier::None) == UpdateResult::NeedsInit);
    CheckVec(p.GetScaleMatrix(), 2, 1, 1);
    CHECK(p.GetWorldPosition().x == Approx(-5.0f));
    // the centre stays put
    CHECK(p.GetHcenterPos() == Approx(5.0f));
    CHECK(s->Update(DownZ(10, 5), Modifier::None) == UpdateResult::Unchanged);
    CHECK(s->Update(DownZ(-10, 5), Modifier::None) == UpdateResult::Unchanged);
    CHECK(s->Update(Flat(), Modifier::None) == UpdateResult::Unchanged);
    CHECK(s->Commit().dirty == handles::DirtyField::Dimensions);
    // ctrl scales about the world position instead
    s->Update(DownZ(10, 5), Modifier::Control);
    CHECK(p.GetWorldPosition().x == Approx(0.0f));
    s->Revert();
    CheckVec(p.GetScaleMatrix(), 1, 1, 1);
    CHECK(s->Commit().dirty == handles::DirtyField::None);

    auto y = p.CreateDragSession("poly", MakeId(Role::AxisCube, -1, Axis::Y), DownZ(5, 5));
    y->Update(DownZ(5, 15), Modifier::None);
    CheckVec(p.GetScaleMatrix(), 1, 3, 1);
    CHECK(p.GetVcenterPos() == Approx(5.0f));
    CHECK(y->Update(DownZ(5, -10), Modifier::None) == UpdateResult::Unchanged);
    y->Revert();

    auto z = p.CreateDragSession("poly", MakeId(Role::AxisCube, -1, Axis::Z), DownY(5, 0));
    z->Update(DownY(5, 0.001f), Modifier::None);
    CHECK(p.GetScaleMatrix().z == Approx(2.0f).margin(0.01));
    CHECK(z->Update(DownY(5, -1), Modifier::None) == UpdateResult::Unchanged);
    z->Revert();
    CheckVec(p.GetScaleMatrix(), 1, 1, 1);

    auto m = p.CreateDragSession("poly", MakeId(Role::AxisCube, -1, Axis::X), Flat());
    m->Update(DownZ(10, 5), Modifier::None);
    CHECK(p.GetScaleMatrix().x == Approx(2.0f));
}

TEST_CASE("Poly point centre ring rotates about the centre and reverts", "[screenloc]") {
    Poly x(glm::vec3(0.0f), glm::vec3(1.0f), "0,0,0,10,0,0,10,10,0", 3, true);
    auto& p = x.p;
    // the Y ring rotates about Z
    auto s = p.CreateDragSession("poly", MakeId(Role::AxisRing, -1, Axis::Y), DownZ(15, 5));
    REQUIRE(s);
    CHECK(s->Update(DownZ(5, 15), Modifier::None) == UpdateResult::NeedsInit);
    CheckVec(x.W(0), 10, 0, 0);
    CheckVec(x.W(1), 10, 10, 0);
    CheckVec(x.W(2), 0, 10, 0);
    auto info = s->GetRotationInfo();
    REQUIRE(info);
    CHECK(info->rotationAxis == Axis::Z);
    CHECK(info->accumulated == Approx(90.0f));
    CheckVec(info->pivot, 5, 5, 0);
    CHECK(s->Update(DownZ(5, 15), Modifier::None) == UpdateResult::Unchanged);
    CHECK(s->Update(Flat(), Modifier::None) == UpdateResult::Unchanged);
    CHECK(s->Commit().dirty == handles::DirtyField::Rotation);
    s->Revert();
    CheckVec(x.W(0), 0, 0, 0);
    CheckVec(x.W(2), 10, 10, 0);
    CHECK(s->Commit().dirty == handles::DirtyField::None);
    s->Revert();

    // the Z ring rotates about X, the X ring about Y; both revert cleanly
    auto rx = p.CreateDragSession("poly", MakeId(Role::AxisRing, -1, Axis::Z), DownX(5, 10));
    rx->Update(DownX(15, 0), Modifier::None);
    CHECK(rx->GetRotationInfo()->rotationAxis == Axis::X);
    CHECK(x.W(2).z != Approx(0.0f).margin(1.0));
    rx->Revert();
    CheckVec(x.W(2), 10, 10, 0);
    auto ry = p.CreateDragSession("poly", MakeId(Role::AxisRing, -1, Axis::X), DownY(5, 10));
    ry->Update(DownY(15, 0), Modifier::None);
    CHECK(ry->GetRotationInfo()->rotationAxis == Axis::Y);
    CHECK(x.W(0).z != Approx(0.0f).margin(1.0));
    ry->Revert();
    CheckVec(x.W(0), 0, 0, 0);

    // a start ray that misses the plane measures from the centre's +X
    auto m = p.CreateDragSession("poly", MakeId(Role::AxisRing, -1, Axis::Y), Flat());
    CHECK(m->Update(DownZ(5, 15), Modifier::None) == UpdateResult::NeedsInit);
    CHECK(m->GetRotationInfo()->accumulated == Approx(90.0f));
}

TEST_CASE("Poly point boundary corners resize the vertex box", "[screenloc]") {
    Poly x;
    auto& p = x.p;
    p.SetCurve(0);
    p.SetCurveCp0(0, glm::vec3(2, 4, 0));
    x.Refresh();

    auto tr = p.CreateDragSession("poly", MakeId(Role::ResizeCorner, 3), DownZ(0, 0));
    REQUIRE(tr);
    CHECK(tr->Update(DownZ(28, 28), Modifier::None) == UpdateResult::NeedsInit);
    CheckVec(p.GetPoint(1), 20, 0, 0);
    CheckVec(p.GetPoint(2), 20, 20, 0);
    CheckVec(p.GetCurveCp0(0), 4, 8, 0);
    CHECK(tr->Commit().dirty == handles::DirtyField::Dimensions);
    tr->Revert();
    CheckVec(p.GetPoint(2), 10, 10, 0);
    CheckVec(p.GetCurveCp0(0), 2, 4, 0);
    CHECK(tr->Commit().dirty == handles::DirtyField::None);
    CHECK(tr->Update(DownZ(8, 28), Modifier::None) == UpdateResult::Unchanged);
    CHECK(tr->Update(DownZ(28, 8), Modifier::None) == UpdateResult::Unchanged);

    auto bl = p.CreateDragSession("poly", MakeId(Role::ResizeCorner, 0), DownZ(0, 0));
    bl->Update(DownZ(-18, -18), Modifier::None);
    CheckVec(p.GetPoint(0), -10, -10, 0);
    CheckVec(p.GetPoint(2), 10, 10, 0);
    bl->Revert();
    CHECK(bl->Update(DownZ(2, -18), Modifier::None) == UpdateResult::Unchanged);

    auto tl = p.CreateDragSession("poly", MakeId(Role::ResizeCorner, 1), DownZ(0, 0));
    tl->Update(DownZ(-18, 28), Modifier::None);
    CheckVec(p.GetPoint(0), -10, 0, 0);
    CheckVec(p.GetPoint(2), 10, 20, 0);
    tl->Revert();
    CHECK(tl->Update(DownZ(-18, 8), Modifier::None) == UpdateResult::Unchanged);

    auto br = p.CreateDragSession("poly", MakeId(Role::ResizeCorner, 2), DownZ(0, 0));
    br->Update(DownZ(28, -18), Modifier::None);
    CheckVec(p.GetPoint(0), 0, -10, 0);
    CheckVec(p.GetPoint(2), 20, 10, 0);
    br->Revert();
    CHECK(br->Update(DownZ(8, -18), Modifier::None) == UpdateResult::Unchanged);
    CHECK(br->GetHandleId() == MakeId(Role::ResizeCorner, 2));
}

TEST_CASE("Poly point boundary resize keeps a flat line flat", "[screenloc]") {
    Poly x(glm::vec3(0.0f), glm::vec3(1.0f), "0,0,0,10,0,0", 2);
    auto& p = x.p;
    auto tr = p.CreateDragSession("poly", MakeId(Role::ResizeCorner, 3), DownZ(0, 0));
    tr->Update(DownZ(28, 28), Modifier::None);
    CheckVec(p.GetPoint(1), 20, 0, 0);
    tr->Revert();
    auto bl = p.CreateDragSession("poly", MakeId(Role::ResizeCorner, 0), DownZ(0, 0));
    bl->Update(DownZ(-18, -18), Modifier::None);
    CheckVec(p.GetPoint(0), -10, 0, 0);
}

TEST_CASE("Poly point space mouse moves vertices control points or the whole line", "[screenloc]") {
    Poly x;
    auto& p = x.p;
    auto v = p.BeginSpaceMouseSession(MakeId(Role::Vertex, 1));
    CHECK(v->Apply(2.0f, glm::vec3(0.0f), glm::vec3(1, 3, -2)) == handles::SpaceMouseResult::NeedsInit);
    CheckVec(p.GetPoint(1), 12, 4, 6);
    CHECK(*v->GetHandleId() == MakeId(Role::Vertex, 1));
    auto bad = p.BeginSpaceMouseSession(MakeId(Role::Vertex, 9));
    bad->Apply(1.0f, glm::vec3(0.0f), glm::vec3(1, 1, 1));

    // the control point nudges start from the cache the draw pass refreshes
    p.SetCurve(1);
    p.SetCurveCp0(1, glm::vec3(3, 3, 3));
    p.SelectSegment(1);
    Program prog;
    p.DrawHandles(&prog, 1.0f, 0, false);
    auto c0 = p.BeginSpaceMouseSession(MakeId(Role::CurveControl, 0, Axis::X, 1));
    CHECK(c0->Apply(1.0f, glm::vec3(0.0f), glm::vec3(1, 1, 1)) == handles::SpaceMouseResult::NeedsInit);
    CheckVec(p.GetCurveCp0(1), 4, 2, 4);
    auto c1 = p.BeginSpaceMouseSession(MakeId(Role::CurveControl, 1, Axis::X, 1));
    c1->Apply(1.0f, glm::vec3(0.0f), glm::vec3(1, 0, 0));
    CheckVec(p.GetCurveCp1(1), 11, 10, 0);
    auto cbad = p.BeginSpaceMouseSession(MakeId(Role::CurveControl, 0, Axis::X, 9));
    cbad->Apply(1.0f, glm::vec3(0.0f), glm::vec3(1, 0, 0));

    p.SetRotatePoint(glm::vec3(0.0f));
    auto whole = p.BeginSpaceMouseSession(std::nullopt);
    CHECK(whole->Apply(1.0f, glm::vec3(0.0f), glm::vec3(1, 2, 3)) == handles::SpaceMouseResult::Dirty);
    CheckVec(p.GetWorldPosition(), 1, -3, 2);
}

// ---------------------------------------------------------------------------
// Drawing
// ---------------------------------------------------------------------------

TEST_CASE("Poly point 2D handles draw corner boxes vertices and the selected segment", "[screenloc]") {
    Poly x(glm::vec3(100, 50, 0));
    auto& p = x.p;
    Program a;
    CHECK(p.DrawHandles(&a, 1.0f, 0, false));
    REQUIRE(a.size() == 4 * 6 + 3 * 6);
    // first corner box sits outside the bottom-left: 6 wide, 12 out
    CheckVec(a.at(0).p, 100 - 3 - 12, 50 - 3 - 12, 0);
    CHECK(a.at(24).c == xlGREENTRANSLUCENT);
    CHECK(a.at(30).c == xlBLUETRANSLUCENT);

    p.SetSelectedHandle(MakeId(Role::Vertex, 1));
    p.SelectSegment(-1);
    Program sel;
    p.DrawHandles(&sel, 1.0f, 0, false);
    CHECK(sel.at(30).c == xlMAGENTATRANSLUCENT);

    p.SelectSegment(0);
    Program seg;
    p.DrawHandles(&seg, 1.0f, 0, false);
    REQUIRE(seg.size() == 24 + 2 + 18);
    CheckVec(seg.at(24).p, 100, 50, 0);
    CheckVec(seg.at(25).p, 110, 50, 0);
    CHECK(seg.at(24).c == xlMAGENTA);

    p.SetCurve(0);
    x.Refresh();
    Program curve;
    p.DrawHandles(&curve, 1.0f, 0, false);
    const size_t curveSegs = CountRole(p.GetHandles(handles::ViewMode::TwoD, handles::Tool::Translate), Role::Segment) - 1;
    CHECK(curve.size() == 24 + curveSegs * 2 + 4 + 18 + 12);

    Program fb;
    p.DrawHandles(&fb, 1.0f, 0, true);
    CHECK(fb.at(0).c == FROM_BASE_HANDLES_COLOUR);
    p.Lock();
    Program lk;
    p.DrawHandles(&lk, 1.0f, 0, false);
    CHECK(lk.at(0).c == LOCKED_HANDLES_COLOUR);
}

TEST_CASE("Poly point 3D handles draw bounding boxes or the active gizmo", "[screenloc]") {
    Poly x(glm::vec3(0.0f), glm::vec3(1.0f), "0,0,0,10,0,0,10,10,0", 3, true);
    auto& p = x.p;
    const size_t cube = CubeVertexCount();

    Program none;
    p.DrawHandles(&none, 1.0f, 0, false, false);
    CHECK(none.size() == 0);
    Program bb;
    p.DrawHandles(&bb, 1.0f, 0, true, false);
    CHECK(bb.size() == 2 * 24);
    CHECK(bb.at(0).c == xlWHITETRANSLUCENT);
    p.SetCurve(0);
    x.Refresh(true);
    Program bbc;
    p.DrawHandles(&bbc, 1.0f, 0, true, true);
    CHECK(bbc.size() > 2 * 24);
    CHECK(bbc.at(0).c == FROM_BASE_HANDLES_COLOUR);
    p.SetCurve(0, false);
    x.Refresh(true);

    p.SetActiveHandle(MakeId(Role::Vertex, 1));
    p.SetAxisTool(handles::Tool::Translate);
    p.SetActiveAxis(MSLAXIS::Y_AXIS);
    Program t;
    p.DrawHandles(&t, 1.0f, 0, false, false);
    // centre sphere, 3 vertex spheres, translate gizmo and the axis line
    CHECK(t.size() == cube + 3 * cube + 3 * 18 * 3 + 6 + 2);
    CHECK(t.at(cube).c == xlGREENTRANSLUCENT);
    CheckVec(t.back(0).p, 10, 1000000.0f, 0);

    p.SetAxisTool(handles::Tool::XYTranslate);
    Program xy;
    p.DrawHandles(&xy, 1.0f, 0, false, false);
    CHECK(xy.size() == cube + 3 * cube + 8 + 4 * 18 * 3 + 4);
    CheckVec(xy.back(3).p, -1000000.0f, 0, 0);

    // a selected curved segment adds its samples and control spheres
    p.SetCurve(1);
    x.Refresh(true);
    p.SelectSegment(1);
    p.MouseOverHandle(MakeId(Role::CurveControl, 0, Axis::X, 1));
    Program cs;
    p.DrawHandles(&cs, 1.0f, 0, false, false);
    CHECK(cs.size() > xy.size() + 2 * cube);

    p.SelectSegment(0);
    p.MouseOverHandle(MakeId(Role::Vertex, 0));
    Program ss;
    p.DrawHandles(&ss, 1.0f, 0, false, false);
    CHECK(ss.at(cube + 2).c == xlYELLOWTRANSLUCENT);

    Program fb;
    p.DrawHandles(&fb, 1.0f, 0, false, true);
    CHECK(fb.at(0).c == FROM_BASE_HANDLES_COLOUR);
    p.Lock();
    Program lk;
    p.DrawHandles(&lk, 1.0f, 0, false, false);
    CHECK(lk.at(0).c == LOCKED_HANDLES_COLOUR);
    p.Lock(false);

    p.SetActiveHandle(std::nullopt);
    p.MouseDown(true);
    Program md;
    p.DrawHandles(&md, 1.0f, 0, false, false);
    CHECK(md.size() > 0);
}
