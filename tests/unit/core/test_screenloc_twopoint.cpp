#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "models/Node.h"
#include "models/ThreePointScreenLocation.h"
#include "models/TwoPointScreenLocation.h"
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

void Place(TwoPointScreenLocation& t, glm::vec3 start, glm::vec3 offset, float renderWi = 10.0f) {
    t.SetRenderSize(renderWi, 1.0f, 0.0f);
    t.SetWorldPosition(start);
    t.SetX2(offset.x);
    t.SetY2(offset.y);
    t.SetZ2(offset.z);
}

struct Line {
    TwoPointScreenLocation t;
    Line(glm::vec3 start, glm::vec3 offset, bool is3d = false) {
        Place(t, start, offset);
        t.PrepareToDraw(is3d, true);
    }
    glm::vec3 Start() const { return t.GetWorldPosition(); }
    glm::vec3 End() const { return t.GetWorldPosition() + glm::vec3(t.GetX2(), t.GetY2(), t.GetZ2()); }
};

// Arch-like three point location: base from (0,0) to (10,0), render 10 x 5.
struct Arch {
    ThreePointScreenLocation a;
    explicit Arch(glm::vec3 start = glm::vec3(0.0f), glm::vec3 offset = glm::vec3(10, 0, 0), bool is3d = false) {
        Place(a, start, offset);
        a.SetRenderSize(10.0f, 5.0f, 0.0f);
        a.PrepareToDraw(is3d, true);
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
// TwoPoint: geometry
// ---------------------------------------------------------------------------

TEST_CASE("Two point centre setters keep the offset", "[screenloc]") {
    Line l(glm::vec3(0, 0, 0), glm::vec3(10, 4, 2));
    l.t.SetHcenterPos(20.0f);
    l.t.SetVcenterPos(-8.0f);
    l.t.SetDcenterPos(1.0f);
    CheckVec(l.Start(), 15, -10, 0);
    CheckVec(l.End(), 25, -6, 2);
    CHECK(l.t.GetDcenterPos() == Approx(1.0f));
}

TEST_CASE("Two point edge setters move the line so the edge lands where asked", "[screenloc]") {
    for (float sign : { 1.0f, -1.0f }) {
        INFO("sign " << sign);
        Line l(glm::vec3(0, 0, 0), glm::vec3(10 * sign, -4 * sign, 6 * sign));
        auto& t = l.t;
        t.SetTop(20.0f);
        CHECK(t.GetTop() == Approx(20.0f));
        t.SetBottom(0.0f);
        CHECK(t.GetBottom() == Approx(0.0f));
        t.SetLeft(5.0f);
        CHECK(t.GetLeft() == Approx(5.0f));
        t.SetRight(30.0f);
        CHECK(t.GetRight() == Approx(30.0f));
        t.SetFront(50.0f);
        CHECK(t.GetFront() == Approx(50.0f));
        t.SetBack(1.0f);
        CHECK(t.GetBack() == Approx(1.0f));
        // the line keeps its length and direction
        CHECK(t.GetX2() == Approx(10 * sign));
        CHECK(t.GetZ2() == Approx(6 * sign));
    }
}

TEST_CASE("Two point size setters write the end offset", "[screenloc]") {
    TwoPointScreenLocation t;
    t.SetMWidth(7.0f);
    t.SetMHeight(-3.0f);
    t.SetMDepth(99.0f);
    CHECK(t.GetX2() == 7.0f);
    CHECK(t.GetY2() == -3.0f);
    CHECK(t.GetZ2() == 0.0f);
    CHECK(t.GetMDepth() == 1.0f);
    CHECK(t.GetYShear() == 0.0f);
    CHECK(t.hasX2());
}

TEST_CASE("Two point rotate about a point on Y swings the line round", "[screenloc]") {
    Line l(glm::vec3(0, 0, 0), glm::vec3(10, 0, 0));
    l.t.RotateAboutPoint(glm::vec3(0, 0, 0), glm::vec3(0, 90, 0));
    CheckVec(l.Start(), 0, 0, 0);
    CheckVec(l.End(), 0, 0, -10);
}

TEST_CASE("Two point rotate about a point on X lifts the line", "[screenloc]") {
    Line l(glm::vec3(0, 10, 0), glm::vec3(10, 0, 0));
    l.t.RotateAboutPoint(glm::vec3(0, 0, 0), glm::vec3(90, 0, 0));
    CheckVec(l.Start(), 0, 0, 10);
    CheckVec(l.End(), 10, 0, 10);
    l.t.Lock();
    l.t.RotateAboutPoint(glm::vec3(0, 0, 0), glm::vec3(0, 90, 0));
    CheckVec(l.Start(), 0, 0, 10);
}

TEST_CASE("Two point rotate about X and Y axes keeps the midpoint", "[screenloc]") {
    Line l(glm::vec3(0, 0, 0), glm::vec3(0, 10, 0));
    CHECK(l.t.Rotate(MSLAXIS::X_AXIS, 90.0f));
    CheckVec(l.Start(), 0, 5, -5);
    CheckVec(l.End(), 0, 5, 5);
    Line m(glm::vec3(0, 0, 0), glm::vec3(10, 0, 0));
    CHECK(m.t.Rotate(MSLAXIS::Y_AXIS, 90.0f));
    CheckVec(m.Start(), 5, 0, -5);
    CheckVec(m.End(), 5, 0, 5);
    CHECK(m.t.Rotate(MSLAXIS::NO_AXIS, 90.0f));
    CheckVec(m.End(), 5, 0, 5);
}

TEST_CASE("Two point hit test uses a slab around the line", "[screenloc]") {
    Line l(glm::vec3(0, 0, 0), glm::vec3(10, 10, 0));
    std::vector<NodeBaseClassPtr> none;
    l.t.UpdateBoundingBox(none);
    glm::vec3 d(0, 0, -1);
    glm::vec3 mid(5, 5, 0), offLine(5, -5, 0), nearLine(6, 4, 0), past(12, 12, 0);
    CHECK(l.t.HitTest(mid, d));
    CHECK(l.t.HitTest(nearLine, d));
    CHECK_FALSE(l.t.HitTest(offLine, d));
    CHECK_FALSE(l.t.HitTest(past, d));
}

TEST_CASE("Two point padded bounding box reaches past the end points", "[screenloc]") {
    Line l(glm::vec3(10, 10, 10), glm::vec3(10, 0, 0));
    l.t.UpdateBoundingBox();
    glm::vec3 d(0, 0, -1);
    glm::vec3 before(6, 10, 0), wayBefore(4, 10, 0), after(24, 10, 0);
    CHECK(l.t.HitTest(before, d));
    CHECK(l.t.HitTest(after, d));
    CHECK_FALSE(l.t.HitTest(wayBefore, d));
}

TEST_CASE("Two point containment uses the line bounding box", "[screenloc]") {
    Line l(glm::vec3(100, 100, 0), glm::vec3(10, 0, 0));
    std::vector<NodeBaseClassPtr> none;
    l.t.UpdateBoundingBox(none);
    FakePreview p;
    CHECK(l.t.IsContained(&p, 90, 90, 120, 110));
    CHECK_FALSE(l.t.IsContained(&p, 101, 90, 120, 110));

    Line l3(glm::vec3(100, 100, 0), glm::vec3(10, 0, 0), true);
    l3.t.UpdateBoundingBox(none);
    CHECK(l3.t.IsContained(&p, 90, 490, 120, 510));
    CHECK_FALSE(l3.t.IsContained(&p, 90, 90, 120, 110));
}

TEST_CASE("Two point screen offset and dimension come from the end points", "[screenloc]") {
    Line l(glm::vec3(100, 150, 0), glm::vec3(200, 0, 0));
    FakePreview p;
    glm::vec2 off = l.t.GetScreenOffset(&p);
    CHECK(off.x == Approx(0.25f));
    CHECK(off.y == Approx(0.75f));
    CHECK(l.t.GetDimension() == "");
    ScopedRuler r(100.0f, 2.0f);
    CHECK(l.t.GetDimension() == "Length 4.00m");
}

TEST_CASE("Two point InitializeLocation collapses the line onto the click", "[screenloc]") {
    TwoPointScreenLocation t;
    Place(t, glm::vec3(0, 0, 0), glm::vec3(5, 5, 5));
    FakePreview p;
    std::vector<NodeBaseClassPtr> nodes;
    int handle = -1;
    CHECK(t.InitializeLocation(handle, 30, 100, nodes, &p) == CursorType::Sizing);
    CHECK(handle == END_HANDLE);
    CheckVec(t.GetWorldPosition(), 30, 500, 0);
    CHECK(t.GetX2() == 0.0f);
    CHECK_FALSE(t.GetActiveHandleId().has_value());
    p.is3d = true;
    t.InitializeLocation(handle, 30, 100, nodes, &p);
    REQUIRE(t.GetActiveHandleId().has_value());
    CHECK(*t.GetActiveHandleId() == MakeId(Role::Endpoint, END_HANDLE));
}

// ---------------------------------------------------------------------------
// TwoPoint: handles and tools
// ---------------------------------------------------------------------------

TEST_CASE("Two point tools skip scale", "[screenloc]") {
    TwoPointScreenLocation t;
    t.SetAxisTool(handles::Tool::Scale);
    CHECK(t.GetAxisTool() == handles::Tool::Translate);
    t.SetAxisTool(handles::Tool::XYTranslate);
    CHECK(t.GetAxisTool() == handles::Tool::Translate);
    t.SetAxisTool(handles::Tool::Rotate);
    CHECK(t.GetAxisTool() == handles::Tool::Rotate);
    t.AdvanceAxisTool();
    CHECK(t.GetAxisTool() == handles::Tool::Translate);
    t.AdvanceAxisTool();
    CHECK(t.GetAxisTool() == handles::Tool::Rotate);

    t.SetActiveHandleToCentre();
    CHECK(*t.GetActiveHandleId() == MakeId(Role::Endpoint, CENTER_HANDLE));
    t.SetActiveHandleToDefault();
    CHECK(*t.GetActiveHandleId() == MakeId(Role::Endpoint, END_HANDLE));
}

TEST_CASE("Two point 2D handles are the two end points", "[screenloc]") {
    Line l(glm::vec3(1, 2, 3), glm::vec3(10, 0, 0));
    auto hs = l.t.GetHandles(handles::ViewMode::TwoD, handles::Tool::Translate);
    REQUIRE(hs.size() == 2);
    CheckVec(Find(hs, MakeId(Role::Endpoint, START_HANDLE))->worldPos, 1, 2, 3);
    CheckVec(Find(hs, MakeId(Role::Endpoint, END_HANDLE))->worldPos, 11, 2, 3);
    CHECK(hs[0].editable);
    l.t.Lock();
    hs = l.t.GetHandles(handles::ViewMode::TwoD, handles::Tool::Translate);
    CHECK_FALSE(hs[0].editable);
}

TEST_CASE("Two point 3D handles add an axis gizmo at the active end", "[screenloc]") {
    Line l(glm::vec3(0, 0, 0), glm::vec3(10, 0, 0), true);
    auto& t = l.t;
    auto hs = t.GetHandles(handles::ViewMode::ThreeD, handles::Tool::Translate);
    REQUIRE(hs.size() == 3);
    for (const auto& d : hs) CHECK(d.selectionOnly);
    CheckVec(Find(hs, MakeId(Role::Endpoint, CENTER_HANDLE))->worldPos, 5, 0, 0);

    t.SetActiveHandleToDefault();
    hs = t.GetHandles(handles::ViewMode::ThreeD, handles::Tool::Translate);
    REQUIRE(hs.size() == 6);
    const auto* ax = Find(hs, MakeId(Role::AxisArrow, END_HANDLE, Axis::X));
    REQUIRE(ax);
    CheckVec(ax->worldPos, 64, 0, 0);

    hs = t.GetHandles(handles::ViewMode::ThreeD, handles::Tool::Rotate);
    CHECK(CountRole(hs, Role::AxisRing) == 3);
    hs = t.GetHandles(handles::ViewMode::ThreeD, handles::Tool::Scale);
    CHECK(hs.size() == 3);

    t.SetActiveHandle(MakeId(Role::Endpoint, START_HANDLE));
    hs = t.GetHandles(handles::ViewMode::ThreeD, handles::Tool::Translate);
    CheckVec(Find(hs, MakeId(Role::AxisArrow, START_HANDLE, Axis::Y))->worldPos, 0, 54, 0);
    t.SetActiveHandleToCentre();
    hs = t.GetHandles(handles::ViewMode::ThreeD, handles::Tool::Translate);
    CheckVec(Find(hs, MakeId(Role::AxisArrow, CENTER_HANDLE, Axis::Z))->worldPos, 5, 0, 54);
    t.SetActiveHandle(MakeId(Role::Endpoint, 9));
    CHECK(t.GetHandles(handles::ViewMode::ThreeD, handles::Tool::Translate).size() == 3);
    t.SetActiveHandle(MakeId(Role::Vertex, 1));
    CHECK(t.GetHandles(handles::ViewMode::ThreeD, handles::Tool::Translate).size() == 3);
}

TEST_CASE("Two point drag sessions are refused for unknown handles", "[screenloc]") {
    Line l(glm::vec3(0, 0, 0), glm::vec3(10, 0, 0), true);
    auto& t = l.t;
    CHECK(t.CreateDragSession("l", MakeId(Role::Endpoint, CENTER_HANDLE), DownZ(0, 0)) == nullptr);
    CHECK(t.CreateDragSession("l", MakeId(Role::AxisArrow, 7), DownZ(0, 0)) == nullptr);
    CHECK(t.CreateDragSession("l", MakeId(Role::AxisRing, -1), DownZ(0, 0)) == nullptr);
    CHECK(t.CreateDragSession("l", MakeId(Role::Vertex, 0), DownZ(0, 0)) == nullptr);
    t.Lock();
    CHECK(t.CreateDragSession("l", MakeId(Role::Endpoint, END_HANDLE), DownZ(0, 0)) == nullptr);
    CHECK(t.BeginCreate("l", DownZ(0, 0), handles::ViewMode::TwoD) == nullptr);
}

// ---------------------------------------------------------------------------
// TwoPoint: drag sessions
// ---------------------------------------------------------------------------

TEST_CASE("Two point 2D end point drags", "[screenloc]") {
    Line l(glm::vec3(0, 0, 0), glm::vec3(10, 0, 0));
    auto& t = l.t;
    auto e = t.CreateDragSession("line", MakeId(Role::Endpoint, END_HANDLE), DownZ(10, 0));
    REQUIRE(e);
    CHECK(e->GetHandleId().index == END_HANDLE);
    CHECK(e->Update(DownZ(20, 5), Modifier::None) == UpdateResult::Updated);
    CheckVec(l.End(), 20, 5, 0);
    CheckVec(l.Start(), 0, 0, 0);
    auto c = e->Commit();
    CHECK(c.modelName == "line");
    CHECK(c.dirty == handles::DirtyField::Endpoint);
    // shift snaps the end vertical / horizontal with the start
    e->Update(DownZ(2, 30), Modifier::Shift);
    CheckVec(l.End(), 0, 30, 0);
    e->Update(DownZ(30, 3), Modifier::Shift);
    CheckVec(l.End(), 30, 0, 0);
    e->Revert();
    CheckVec(l.End(), 10, 0, 0);
    CHECK(e->Commit().dirty == handles::DirtyField::None);

    auto s = t.CreateDragSession("line", MakeId(Role::Endpoint, START_HANDLE), DownZ(0, 0));
    s->Update(DownZ(2, 3), Modifier::None);
    CheckVec(l.Start(), 2, 3, 0);
    CheckVec(l.End(), 10, 0, 0);
    // shift snaps the start onto the end's x and y when close
    s->Update(DownZ(12, 2), Modifier::Shift);
    CheckVec(l.Start(), 10, 0, 0);
    s->Revert();
    CheckVec(l.Start(), 0, 0, 0);
}

TEST_CASE("Two point axis arrows move the chosen end along one axis", "[screenloc]") {
    Line l(glm::vec3(0, 0, 0), glm::vec3(10, 0, 0), true);
    auto& t = l.t;

    auto c = t.CreateDragSession("line", MakeId(Role::AxisArrow, CENTER_HANDLE, Axis::X), DownZ(5, 0));
    REQUIRE(c);
    CHECK(c->Update(DownZ(8, 50), Modifier::None) == UpdateResult::Updated);
    CheckVec(l.Start(), 3, 0, 0);
    CheckVec(l.End(), 13, 0, 0);
    CHECK(c->Commit().dirty == handles::DirtyField::Position);
    c->Revert();
    CheckVec(l.Start(), 0, 0, 0);
    CHECK(c->Commit().dirty == handles::DirtyField::None);
    handles::WorldRay flat;
    flat.origin = glm::vec3(0, 0, 0);
    flat.direction = glm::vec3(1, 0, 0);
    CHECK(c->Update(flat, Modifier::None) == UpdateResult::Unchanged);

    auto s = t.CreateDragSession("line", MakeId(Role::AxisArrow, START_HANDLE, Axis::Y), DownZ(0, 0));
    s->Update(DownZ(9, 4), Modifier::None);
    CheckVec(l.Start(), 0, 4, 0);
    CheckVec(l.End(), 10, 0, 0);
    s->Revert();

    auto sx = t.CreateDragSession("line", MakeId(Role::AxisArrow, START_HANDLE, Axis::X), DownZ(0, 0));
    sx->Update(DownZ(3, 0), Modifier::None);
    CheckVec(l.Start(), 3, 0, 0);
    CheckVec(l.End(), 10, 0, 0);
    sx->Revert();

    auto e = t.CreateDragSession("line", MakeId(Role::AxisArrow, END_HANDLE, Axis::Z), DownY(10, 0));
    e->Update(DownY(10, 6), Modifier::None);
    CheckVec(l.Start(), 0, 0, 0);
    CheckVec(l.End(), 10, 0, 6);
    e->Revert();

    // a start ray that misses the plane falls back to the handle position
    auto f = t.CreateDragSession("line", MakeId(Role::AxisArrow, END_HANDLE, Axis::X), flat);
    f->Update(DownZ(12, 0), Modifier::None);
    CheckVec(l.End(), 12, 0, 0);
}

TEST_CASE("Two point rotate rings pivot about the chosen point", "[screenloc]") {
    Line l(glm::vec3(0, 0, 0), glm::vec3(10, 0, 0), true);
    auto& t = l.t;

    // the Y ring rotates about Z; centre pivot is the midpoint
    auto c = t.CreateDragSession("line", MakeId(Role::AxisRing, CENTER_HANDLE, Axis::Y), DownZ(15, 0));
    REQUIRE(c);
    CHECK(c->Update(DownZ(5, 10), Modifier::None) == UpdateResult::Updated);
    CheckVec(l.Start(), 5, -5, 0);
    CheckVec(l.End(), 5, 5, 0);
    auto info = c->GetRotationInfo();
    REQUIRE(info);
    CHECK(info->rotationAxis == Axis::Z);
    CHECK(info->accumulated == Approx(90.0f));
    CheckVec(info->pivot, 5, 0, 0);
    CHECK(c->Update(DownZ(5, 10), Modifier::None) == UpdateResult::Unchanged);
    CHECK(c->Commit().dirty == handles::DirtyField::Rotation);
    c->Revert();
    CheckVec(l.Start(), 0, 0, 0);
    CheckVec(l.End(), 10, 0, 0);
    CHECK(c->Commit().dirty == handles::DirtyField::None);

    auto s = t.CreateDragSession("line", MakeId(Role::AxisRing, START_HANDLE, Axis::Y), DownZ(10, 0));
    s->Update(DownZ(0, 10), Modifier::None);
    CheckVec(l.Start(), 0, 0, 0);
    CheckVec(l.End(), 0, 10, 0);
    s->Revert();

    auto e = t.CreateDragSession("line", MakeId(Role::AxisRing, END_HANDLE, Axis::Y), DownZ(20, 0));
    e->Update(DownZ(10, 10), Modifier::None);
    CheckVec(l.Start(), 10, -10, 0);
    CheckVec(l.End(), 10, 0, 0);
    e->Revert();

    // the Z ring rotates about X; the X ring about Y
    auto rx = t.CreateDragSession("line", MakeId(Role::AxisRing, START_HANDLE, Axis::Z), DownX(0, 10));
    rx->Update(DownX(10, 0), Modifier::None);
    CHECK(rx->GetRotationInfo()->rotationAxis == Axis::X);
    CheckVec(l.End(), 10, 0, 0);
    rx->Revert();
    auto ry = t.CreateDragSession("line", MakeId(Role::AxisRing, START_HANDLE, Axis::X), DownY(10, 0));
    ry->Update(DownY(0, 10), Modifier::None);
    CHECK(ry->GetRotationInfo()->rotationAxis == Axis::Y);
    CHECK(glm::length(l.End() - l.Start()) == Approx(10.0f));
    CHECK(l.End().z == Approx(10.0f).margin(1e-3));
    ry->Revert();

    handles::WorldRay flat;
    flat.origin = glm::vec3(0, 0, 0);
    flat.direction = glm::vec3(1, 0, 0);
    auto f = t.CreateDragSession("line", MakeId(Role::AxisRing, CENTER_HANDLE, Axis::Y), flat);
    CHECK(f->Update(flat, Modifier::None) == UpdateResult::Unchanged);
}

TEST_CASE("Two point 2D creation drags the end point", "[screenloc]") {
    TwoPointScreenLocation t;
    Place(t, glm::vec3(5, 5, 0), glm::vec3(0, 0, 0));
    auto s = t.BeginCreate("new", DownZ(5, 5), handles::ViewMode::TwoD);
    REQUIRE(s);
    CHECK(s->GetHandleId() == MakeId(Role::Endpoint, END_HANDLE));
    s->Update(DownZ(25, 15), Modifier::None);
    CHECK(t.GetX2() == Approx(20.0f));
    CHECK(t.GetY2() == Approx(10.0f));
    auto c = s->Commit();
    CHECK(c.modelName == "new");
    CHECK(c.dirty == handles::DirtyField::Endpoint);
    s->Revert();
    CHECK(t.GetX2() == 0.0f);
}

TEST_CASE("Two point 3D creation drags the end point freely in the chosen plane", "[screenloc]") {
    TwoPointScreenLocation t;
    Place(t, glm::vec3(1, 2, 3), glm::vec3(0, 0, 0));

    t.SetActivePlane(MSLPLANE::XY_PLANE);
    auto xy = t.BeginCreate("new", DownZ(1, 2), handles::ViewMode::ThreeD);
    CHECK(xy->Update(DownZ(7, 8), Modifier::None) == UpdateResult::Updated);
    CheckVec(glm::vec3(t.GetX2(), t.GetY2(), t.GetZ2()), 6, 6, 0);
    CHECK(xy->Update(DownY(7, 8), Modifier::None) == UpdateResult::Unchanged);
    // a plane behind the ray origin is ignored
    CHECK(xy->Update(DownZ(7, 8, -1000.0f), Modifier::None) == UpdateResult::Unchanged);
    CHECK(xy->Commit().dirty == handles::DirtyField::Endpoint);
    xy->Revert();
    CHECK(t.GetX2() == 0.0f);
    CHECK(xy->Commit().dirty == handles::DirtyField::None);

    t.SetActivePlane(MSLPLANE::XZ_PLANE);
    auto xz = t.BeginCreate("new", DownY(1, 3), handles::ViewMode::ThreeD);
    xz->Update(DownY(5, 9), Modifier::None);
    CheckVec(glm::vec3(t.GetX2(), t.GetY2(), t.GetZ2()), 4, 0, 6);
    xz->Revert();

    t.SetActivePlane(MSLPLANE::YZ_PLANE);
    auto yz = t.BeginCreate("new", DownX(2, 3), handles::ViewMode::ThreeD);
    yz->Update(DownX(5, 12), Modifier::None);
    CheckVec(glm::vec3(t.GetX2(), t.GetY2(), t.GetZ2()), 0, 3, 9);
}

TEST_CASE("Two point space mouse moves the whole line or one end", "[screenloc]") {
    Line l(glm::vec3(0, 0, 0), glm::vec3(10, 0, 0));
    auto& t = l.t;
    auto whole = t.BeginSpaceMouseSession(std::nullopt);
    CHECK(whole->Apply(1.0f, glm::vec3(0.0f), glm::vec3(1, 2, 3)) == handles::SpaceMouseResult::Dirty);
    CheckVec(l.Start(), 1, -3, 2);
    CheckVec(l.End(), 11, -3, 2);
    CHECK_FALSE(whole->GetHandleId().has_value());

    t.SetWorldPosition(glm::vec3(0, 0, 0));
    auto start = t.BeginSpaceMouseSession(MakeId(Role::Endpoint, START_HANDLE));
    start->Apply(1.0f, glm::vec3(0.0f), glm::vec3(1, 0, 0));
    CheckVec(l.Start(), 1, 0, 0);
    CheckVec(l.End(), 10, 0, 0);

    auto end = t.BeginSpaceMouseSession(MakeId(Role::Endpoint, END_HANDLE));
    end->Apply(1.0f, glm::vec3(0.0f), glm::vec3(0, 0, -2));
    CheckVec(l.Start(), 1, 0, 0);
    CheckVec(l.End(), 10, 2, 0);
    CHECK(end->GetHandleId()->index == END_HANDLE);
}

TEST_CASE("Two point space mouse end rotation pivots about the end at the origin", "[screenloc]") {
    // With the line starting at the origin the end offset equals the end
    // point, so the end-handle rotation keeps the end fixed.
    Line l(glm::vec3(0, 0, 0), glm::vec3(10, 0, 0));
    auto end = l.t.BeginSpaceMouseSession(MakeId(Role::Endpoint, END_HANDLE));
    end->Apply(1.0f, glm::vec3(0, 9, 0), glm::vec3(0.0f));
    CheckVec(l.End(), 10, 0, 0);
    CheckVec(l.Start(), 10, -10, 0);
}

// x2/y2/z2 are relative to the start, so space-mouse rotation about either
// end must keep the line length away from the origin too.
TEST_CASE("Two point space mouse rotation away from the origin keeps the line length", "[screenloc]") {
    Line l(glm::vec3(100, 0, 0), glm::vec3(10, 0, 0));
    auto start = l.t.BeginSpaceMouseSession(MakeId(Role::Endpoint, START_HANDLE));
    start->Apply(1.0f, glm::vec3(0, 9, 0), glm::vec3(0.0f));
    CheckVec(l.Start(), 100, 0, 0);
    CHECK(glm::length(l.End() - l.Start()) == Approx(10.0f));

    Line m(glm::vec3(100, 0, 0), glm::vec3(10, 0, 0));
    auto end = m.t.BeginSpaceMouseSession(MakeId(Role::Endpoint, END_HANDLE));
    end->Apply(1.0f, glm::vec3(0, 9, 0), glm::vec3(0.0f));
    CheckVec(m.End(), 110, 0, 0);
    CHECK(glm::length(m.End() - m.Start()) == Approx(10.0f));
}

// ---------------------------------------------------------------------------
// TwoPoint: drawing
// ---------------------------------------------------------------------------

TEST_CASE("Two point 2D handles draw squares and a guide line when axis aligned", "[screenloc]") {
    Line h(glm::vec3(0, 0, 0), glm::vec3(10, 0, 0));
    Program p;
    CHECK(h.t.DrawHandles(&p, 1.0f, 0, false));
    REQUIRE(p.size() == 2 + 12);
    CheckVec(p.at(0).p, 0, 0, 0);
    CheckVec(p.at(1).p, 10, 0, 0);
    CHECK(p.at(0).c == xlRED);
    CHECK(p.at(2).c == xlGREENTRANSLUCENT);
    CHECK(p.at(8).c == xlBLUETRANSLUCENT);

    Line v(glm::vec3(0, 0, 0), glm::vec3(0, 10, 0));
    Program pv;
    v.t.DrawHandles(&pv, 1.0f, 0, true);
    REQUIRE(pv.size() == 14);
    CHECK(pv.at(0).c == FROM_BASE_HANDLES_COLOUR);

    Line d(glm::vec3(0, 0, 0), glm::vec3(10, 10, 0));
    d.t.Lock();
    Program pd;
    d.t.DrawHandles(&pd, 1.0f, 0, false);
    REQUIRE(pd.size() == 12);
    CHECK(pd.at(6).c == LOCKED_HANDLES_COLOUR);
}

TEST_CASE("Two point 3D handles draw spheres and the gizmo for the active end", "[screenloc]") {
    Line l(glm::vec3(0, 0, 0), glm::vec3(10, 0, 0), true);
    auto& t = l.t;
    const size_t cube = CubeVertexCount();

    Program none;
    t.DrawHandles(&none, 1.0f, 0, false, false);
    CHECK(none.size() == 0);
    Program bbox;
    t.DrawHandles(&bbox, 1.0f, 0, true, false);
    CHECK(bbox.size() == 24);
    CHECK(bbox.at(0).c == xlWHITETRANSLUCENT);
    Program bboxBase;
    t.DrawHandles(&bboxBase, 1.0f, 0, true, true);
    CHECK(bboxBase.at(0).c == FROM_BASE_HANDLES_COLOUR);

    t.SetActiveHandleToDefault();
    t.SetActiveAxis(MSLAXIS::X_AXIS);
    t.MouseOverHandle(MakeId(Role::Endpoint, START_HANDLE));
    Program a;
    t.DrawHandles(&a, 1.0f, 0, true, false);
    CHECK(a.size() == 3 * cube + 3 * 18 * 3 + 6 + 2);
    // START first (highlighted), then END, then CENTER
    CHECK(a.at(0).c == xlYELLOWTRANSLUCENT);
    CHECK(a.at(cube).c == xlBLUETRANSLUCENT);
    CHECK(a.at(2 * cube).c == xlORANGETRANSLUCENT);
    // gizmo sits on the active END point
    CheckVec(a.back(7).p, 16, 0, 0);
    CheckVec(a.back(0).p, 1000000.0f, 0, 0);

    Program fb;
    t.DrawHandles(&fb, 1.0f, 0, true, true);
    CHECK(fb.at(0).c == FROM_BASE_HANDLES_COLOUR);

    t.Lock();
    Program lk;
    t.DrawHandles(&lk, 1.0f, 0, true, false);
    CHECK(lk.size() == 3 * cube);
    CHECK(lk.at(0).c == LOCKED_HANDLES_COLOUR);
    Program lkb;
    t.SetActiveHandle(std::nullopt);
    t.DrawHandles(&lkb, 1.0f, 0, true, false);
    CHECK(lkb.at(0).c == LOCKED_HANDLES_COLOUR);
}

// ---------------------------------------------------------------------------
// ThreePoint
// ---------------------------------------------------------------------------

TEST_CASE("Three point handles add the shear handle above the middle", "[screenloc]") {
    Arch x;
    auto& a = x.a;
    CheckVec(a.GetShearHandleWorldPosition(), 5, 5, 0);
    auto hs = a.GetHandles(handles::ViewMode::TwoD, handles::Tool::Translate);
    REQUIRE(hs.size() == 3);
    CheckVec(Find(hs, MakeId(Role::Shear, SHEAR_HANDLE))->worldPos, 5, 5, 0);

    // taller arch: the handle height follows the scaled height
    a.SetHeight(2.0f);
    a.PrepareToDraw(false, true);
    CheckVec(a.GetShearHandleWorldPosition(), 5, 10, 0);
    // model-handle-height models ignore the height in the matrix
    a.SetModelHandleHeight(true);
    a.PrepareToDraw(false, true);
    CheckVec(a.GetShearHandleWorldPosition(), 5, 5, 0);
    CHECK(a.GetVScaleFactor() == 1.0f);
    a.SetModelHandleHeight(false);
    CHECK(a.GetVScaleFactor() == 2.0f);

    // angled models swing the handle round the middle
    a.SetHeight(1.0f);
    a.SetSupportsAngle(true);
    a.SetAngle(90);
    CHECK(a.GetAngle() == 90);
    a.PrepareToDraw(false, true);
    CheckVec(a.GetShearHandleWorldPosition(), 0, 0, 0, 1e-3f);

    a.Lock();
    CHECK(a.GetHandles(handles::ViewMode::TwoD, handles::Tool::Translate).size() == 2);
    CHECK(a.GetHandles(handles::ViewMode::ThreeD, handles::Tool::Translate).size() == 3);
}

TEST_CASE("Three point 3D shear handle becomes draggable once active in XY mode", "[screenloc]") {
    Arch x(glm::vec3(0.0f), glm::vec3(10, 0, 0), true);
    auto& a = x.a;
    auto hs = a.GetHandles(handles::ViewMode::ThreeD, handles::Tool::Translate);
    REQUIRE(hs.size() == 4);
    CHECK(Find(hs, MakeId(Role::Shear, SHEAR_HANDLE))->selectionOnly);
    a.SetActiveHandle(MakeId(Role::Shear, SHEAR_HANDLE));
    CHECK(a.GetAxisTool() == handles::Tool::XYTranslate);
    CHECK(a.IsXYTransHandle());
    hs = a.GetHandles(handles::ViewMode::ThreeD, handles::Tool::XYTranslate);
    CHECK_FALSE(Find(hs, MakeId(Role::Shear, SHEAR_HANDLE))->selectionOnly);
}

TEST_CASE("Three point tool and axis rules for the shear handle", "[screenloc]") {
    Arch x;
    auto& a = x.a;
    a.SetAxisTool(handles::Tool::Scale);
    CHECK(a.GetAxisTool() == handles::Tool::Translate);
    a.AdvanceAxisTool();
    CHECK(a.GetAxisTool() == handles::Tool::Rotate);
    a.SetActiveAxis(MSLAXIS::Y_AXIS);
    CHECK(a.GetActiveAxis() == MSLAXIS::Z_AXIS);
    CHECK_FALSE(a.IsXYTransHandle());

    a.SetActiveHandle(MakeId(Role::Shear, SHEAR_HANDLE));
    a.AdvanceAxisTool();
    CHECK(a.GetAxisTool() == handles::Tool::XYTranslate);
    a.SetAxisTool(handles::Tool::Rotate);
    CHECK(a.GetAxisTool() == handles::Tool::XYTranslate);
    a.SetActiveAxis(MSLAXIS::Z_AXIS);
    CHECK(a.GetActiveAxis() == MSLAXIS::X_AXIS);
    a.SetActiveAxis(MSLAXIS::NO_AXIS);
    CHECK(a.GetActiveAxis() == MSLAXIS::NO_AXIS);
}

TEST_CASE("Three point size accessors", "[screenloc]") {
    ThreePointScreenLocation a;
    a.SetMWidth(12.0f);
    CHECK(a.GetMWidth() == 12.0f);
    CHECK(a.GetX2() == 12.0f);
    a.SetMHeight(3.0f);
    CHECK(a.GetHeight() == 3.0f);
    a.SetShear(0.4f);
    CHECK(a.GetShear() == 0.4f);
    CHECK(a.GetYShear() == 0.0f);
    CHECK_FALSE(a.GetSupportsShear());
    CHECK_FALSE(a.GetSupportsAngle());
}

TEST_CASE("Three point 2D shear drag scales the height", "[screenloc]") {
    Arch x;
    auto& a = x.a;
    auto s = a.CreateDragSession("arch", MakeId(Role::Shear, SHEAR_HANDLE), DownZ(5, 5));
    REQUIRE(s);
    CHECK(s->Update(DownZ(5, 10), Modifier::None) == UpdateResult::NeedsInit);
    CHECK(a.GetHeight() == Approx(2.0f));
    CHECK(s->Commit().dirty == handles::DirtyField::Shear);
    s->Update(DownZ(5, 0.001f), Modifier::None);
    CHECK(a.GetHeight() == Approx(0.01f));
    s->Update(DownZ(5, -0.001f), Modifier::None);
    CHECK(a.GetHeight() == Approx(-0.01f));
    s->Revert();
    CHECK(a.GetHeight() == Approx(1.0f));
    CHECK(s->Commit().dirty == handles::DirtyField::None);
    CHECK(s->GetHandleId().role == Role::Shear);

    CHECK(a.CreateDragSession("arch", MakeId(Role::Shear, 1), DownZ(5, 5)) == nullptr);
    // end points still go to the two point sessions
    CHECK(a.CreateDragSession("arch", MakeId(Role::Endpoint, END_HANDLE), DownZ(5, 5)) != nullptr);
    a.Lock();
    CHECK(a.CreateDragSession("arch", MakeId(Role::Shear, SHEAR_HANDLE), DownZ(5, 5)) == nullptr);
}

TEST_CASE("Three point 2D shear drag on an angled model sets angle and height", "[screenloc]") {
    Arch x;
    auto& a = x.a;
    a.SetSupportsAngle(true);
    auto s = a.CreateDragSession("arch", MakeId(Role::Shear, SHEAR_HANDLE), DownZ(5, 5));
    s->Update(DownZ(0, 5), Modifier::None);
    CHECK(a.GetAngle() == 45);
    CHECK(a.GetHeight() == Approx(std::sqrt(50.0f) / 5.0f));
    s->Update(DownZ(0, -5), Modifier::None);
    CHECK(a.GetAngle() == 135);
    s->Update(DownZ(10, -5), Modifier::None);
    CHECK(a.GetAngle() == -135);
    s->Update(DownZ(8, 10), Modifier::Shift);
    CHECK(a.GetAngle() == -15);
    CHECK(s->Update(DownZ(9, 0), Modifier::None) == UpdateResult::Unchanged);
    s->Revert();
    CHECK(a.GetAngle() == 0);

    // a right-to-left arch mirrors the angle
    Arch r(glm::vec3(10, 0, 0), glm::vec3(-10, 0, 0));
    r.a.SetSupportsAngle(true);
    auto rs = r.a.CreateDragSession("arch", MakeId(Role::Shear, SHEAR_HANDLE), DownZ(5, 5));
    rs->Update(DownZ(0, 5), Modifier::None);
    CHECK(r.a.GetAngle() == -45);
}

TEST_CASE("Three point 2D shear drag on a shearable model slants it", "[screenloc]") {
    Arch x;
    auto& a = x.a;
    a.SetSupportsShear(true);
    auto s = a.CreateDragSession("arch", MakeId(Role::Shear, SHEAR_HANDLE), DownZ(5, 5));
    s->Update(DownZ(8, 10), Modifier::None);
    CHECK(a.GetShear() == Approx(0.3f));
    CHECK(a.GetHeight() == Approx(2.0f));
    s->Revert();
    s->Update(DownZ(100, 10), Modifier::None);
    CHECK(a.GetShear() == Approx(3.0f));
    s->Revert();
    s->Update(DownZ(-100, 10), Modifier::None);
    CHECK(a.GetShear() == Approx(-3.0f));
    s->Revert();
    s->Update(DownZ(5, 0), Modifier::None);
    CHECK(a.GetHeight() == Approx(0.01f));
    s->Revert();
    // a flipped (negative height) model shears the other way
    a.SetHeight(-1.0f);
    s->Update(DownZ(8, -10), Modifier::None);
    CHECK(a.GetShear() == Approx(-0.3f));
    CHECK(a.GetHeight() == Approx(-2.0f));
}

TEST_CASE("Three point 3D shear drag works in the model plane", "[screenloc]") {
    Arch x(glm::vec3(0.0f), glm::vec3(10, 0, 0), true);
    auto& a = x.a;
    a.SetActiveHandle(MakeId(Role::Shear, SHEAR_HANDLE));
    auto s = a.CreateDragSession("arch", MakeId(Role::Shear, SHEAR_HANDLE), DownZ(5, 5));
    REQUIRE(s);
    CHECK(s->Update(DownZ(5, 10), Modifier::None) == UpdateResult::NeedsInit);
    CHECK(a.GetHeight() == Approx(2.0f));
    CHECK(s->Commit().dirty == handles::DirtyField::Shear);
    s->Update(DownZ(5, 0.0f + 5.001f - 5.0f), Modifier::None);
    CHECK(std::fabs(a.GetHeight()) == Approx(0.01f));
    handles::WorldRay flat;
    flat.origin = glm::vec3(0, 0, 0);
    flat.direction = glm::vec3(1, 0, 0);
    CHECK(s->Update(flat, Modifier::None) == UpdateResult::Unchanged);
    s->Revert();
    CHECK(a.GetHeight() == Approx(1.0f));
    CHECK(s->Commit().dirty == handles::DirtyField::None);
    CHECK(s->GetHandleId().role == Role::Shear);
}

TEST_CASE("Three point 3D shear drag on angled and shearable models", "[screenloc]") {
    {
        Arch x(glm::vec3(0.0f), glm::vec3(10, 0, 0), true);
        auto& a = x.a;
        a.SetSupportsAngle(true);
        a.SetActiveHandle(MakeId(Role::Shear, SHEAR_HANDLE));
        auto s = a.CreateDragSession("arch", MakeId(Role::Shear, SHEAR_HANDLE), DownZ(5, 5));
        s->Update(DownZ(0, 10), Modifier::None);
        CHECK(a.GetAngle() == 26);
        CHECK(a.GetHeight() == Approx(std::sqrt(125.0f) / 5.0f));
        s->Update(DownZ(0, 10), Modifier::Shift);
        CHECK(a.GetAngle() == 25);
        s->Update(DownZ(0, -10), Modifier::None);
        CHECK(a.GetAngle() == 154);
        s->Update(DownZ(10, -10), Modifier::None);
        CHECK(a.GetAngle() == -154);
        CHECK(s->Update(DownZ(9, 0), Modifier::None) == UpdateResult::Unchanged);
        s->Update(DownZ(5, 0.001f), Modifier::None);
        CHECK(a.GetHeight() == Approx(0.01f));
    }
    {
        Arch x(glm::vec3(0.0f), glm::vec3(10, 0, 0), true);
        auto& a = x.a;
        a.SetSupportsShear(true);
        a.SetActiveHandle(MakeId(Role::Shear, SHEAR_HANDLE));
        auto s = a.CreateDragSession("arch", MakeId(Role::Shear, SHEAR_HANDLE), DownZ(5, 5));
        s->Update(DownZ(8, 10), Modifier::None);
        CHECK(a.GetShear() == Approx(0.3f));
        CHECK(a.GetHeight() == Approx(2.0f));
        s->Revert();
        s->Update(DownZ(100, 10), Modifier::None);
        CHECK(a.GetShear() == Approx(3.0f));
        s->Revert();
        s->Update(DownZ(-100, 10), Modifier::None);
        CHECK(a.GetShear() == Approx(-3.0f));
        s->Revert();
        s->Update(DownZ(5, 0.001f), Modifier::None);
        CHECK(a.GetHeight() == Approx(0.01f));
        s->Revert();
        a.SetHeight(-1.0f);
        s->Update(DownZ(8, 10), Modifier::None);
        CHECK(a.GetShear() == Approx(-0.3f));
    }
}

TEST_CASE("Three point space mouse on the shear handle edits angle shear or height", "[screenloc]") {
    Arch x;
    auto& a = x.a;
    auto sh = a.BeginSpaceMouseSession(MakeId(Role::Shear, SHEAR_HANDLE));
    CHECK(sh->Apply(1.0f, glm::vec3(0.0f), glm::vec3(0, 0, -2)) == handles::SpaceMouseResult::NeedsInit);
    CHECK(a.GetHeight() == Approx(3.0f));
    sh->Apply(1.0f, glm::vec3(0.0f), glm::vec3(0, 0, 3));
    CHECK(a.GetHeight() == Approx(0.01f));
    CHECK(sh->GetHandleId()->role == Role::Shear);

    a.SetSupportsShear(true);
    sh->Apply(1.0f, glm::vec3(0.0f), glm::vec3(0.1f, 0, 0));
    CHECK(a.GetShear() == Approx(-1.0f));
    a.SetSupportsAngle(true);
    sh->Apply(1.0f, glm::vec3(0.0f), glm::vec3(1, 0, 0));
    CHECK(a.GetAngle() == -10);

    a.SetHeight(0.005f);
    a.SetSupportsAngle(false);
    a.SetSupportsShear(false);
    sh->Apply(1.0f, glm::vec3(0.0f), glm::vec3(0, 0, 0.01f));
    CHECK(a.GetHeight() == Approx(-0.01f));

    // other handles fall through to the two point behaviour
    auto end = a.BeginSpaceMouseSession(MakeId(Role::Endpoint, END_HANDLE));
    CHECK(end->Apply(1.0f, glm::vec3(0.0f), glm::vec3(1, 0, 0)) == handles::SpaceMouseResult::Dirty);
    CHECK(a.GetX2() == Approx(11.0f));
}

TEST_CASE("Three point dimensions through the ruler", "[screenloc]") {
    Arch x;
    auto& a = x.a;
    CHECK(a.GetDimension() == "");
    ScopedRuler r(100.0f, 2.0f);
    CHECK(a.GetRealWidth() == Approx(0.2f));
    CHECK(a.GetRealHeight() == Approx(0.1f));
    CHECK(a.GetDimension() == "Length 0.20m Height 0.10m");
    CHECK(a.GetDimension(2.0f) == "Length 0.20m Height 0.20m");
}

TEST_CASE("Three point node bounding box in 2D and 3D", "[screenloc]") {
    Arch x(glm::vec3(100, 100, 0));
    auto& a = x.a;
    auto nodes = NodesAt({ { 0, 0, 0 }, { 10, 1, 0 } });
    nodes.push_back(nullptr);
    a.UpdateBoundingBox(nodes);
    glm::vec3 d(0, 0, -1);
    glm::vec3 in(105, 103, 0), right(111, 100, 0), above(105, 107, 0);
    CHECK(a.HitTest(in, d));
    CHECK_FALSE(a.HitTest(right, d));
    CHECK_FALSE(a.HitTest(above, d));
    FakePreview p;
    CHECK(a.IsContained(&p, 90, 90, 120, 120));
    CHECK_FALSE(a.IsContained(&p, 101, 90, 120, 120));

    // 3D: the height scales the box before it is placed
    Arch y(glm::vec3(100, 100, 0), glm::vec3(10, 0, 0), true);
    y.a.SetHeight(2.0f);
    y.a.PrepareToDraw(true, true);
    auto tall = NodesAt({ { 0, 0, -3 }, { 10, 4, 3 } });
    y.a.UpdateBoundingBox(tall);
    float dist = 1e9f;
    glm::vec3 top(105, 107, 100), over(105, 109, 100);
    CHECK(y.a.HitTest3D(top, d, dist));
    CHECK_FALSE(y.a.HitTest3D(over, d, dist));
    CHECK(y.a.IsContained(&p, 90, 480, 120, 510));

    std::vector<NodeBaseClassPtr> none;
    y.a.UpdateBoundingBox(none);
    CHECK(y.a.HitTest3D(top, d, dist));
}

TEST_CASE("Three point InitializeLocation selects the end point in 3D", "[screenloc]") {
    ThreePointScreenLocation a;
    a.SetRenderSize(10, 5, 0);
    FakePreview p;
    std::vector<NodeBaseClassPtr> nodes;
    int handle = -1;
    a.InitializeLocation(handle, 30, 100, nodes, &p);
    CHECK(handle == END_HANDLE);
    CHECK_FALSE(a.GetActiveHandleId().has_value());
    p.is3d = true;
    a.InitializeLocation(handle, 30, 100, nodes, &p);
    CHECK(*a.GetActiveHandleId() == MakeId(Role::Endpoint, END_HANDLE));
    CheckVec(a.GetWorldPosition(), 30, 500, 0);
}

TEST_CASE("Three point 2D handles draw the shear spoke then the end points", "[screenloc]") {
    Arch x;
    Program p;
    CHECK(x.a.DrawHandles(&p, 1.0f, 0, false));
    REQUIRE(p.size() == 2 + 6 + 2 + 12);
    CheckVec(p.at(0).p, 5, 0, 0);
    CheckVec(p.at(1).p, 5, 5, 0);
    CHECK(p.at(2).c == xlBLUETRANSLUCENT);
    Program fb;
    x.a.DrawHandles(&fb, 1.0f, 0, true);
    CHECK(fb.at(2).c == FROM_BASE_HANDLES_COLOUR);
    x.a.Lock();
    Program lk;
    x.a.DrawHandles(&lk, 1.0f, 0, false);
    CHECK(lk.at(2).c == LOCKED_HANDLES_COLOUR);
}

TEST_CASE("Three point 3D handles draw the shear sphere and its XY cross", "[screenloc]") {
    Arch x(glm::vec3(0.0f), glm::vec3(10, 0, 0), true);
    auto& a = x.a;
    const size_t cube = CubeVertexCount();
    std::vector<NodeBaseClassPtr> none;
    a.UpdateBoundingBox(none);

    Program bbox;
    a.DrawHandles(&bbox, 1.0f, 0, true, false);
    CHECK(bbox.size() == 24);

    a.SetActiveHandle(MakeId(Role::Shear, SHEAR_HANDLE));
    a.SetActiveAxis(MSLAXIS::Y_AXIS);
    a.MouseOverHandle(MakeId(Role::Endpoint, END_HANDLE));
    Program s;
    a.DrawHandles(&s, 1.0f, 0, true, false);
    // spoke + shear sphere, then 3 end point spheres, the XY gizmo and the cross
    CHECK(s.size() == 2 + cube + 3 * cube + 8 + 4 * 18 * 3 + 4);
    CheckVec(s.at(1).p, 5, 5, 0);
    CHECK(s.at(2).c == xlBLUETRANSLUCENT);
    CheckVec(s.back(3).p, -1000000.0f, 5, 0);
    CheckVec(s.back(0).p, 5, 1000000.0f, 0);

    // the active handle is never highlighted; hover the shear from an end point
    a.SetActiveHandle(MakeId(Role::Endpoint, END_HANDLE));
    a.MouseOverHandle(MakeId(Role::Shear, SHEAR_HANDLE));
    Program hl;
    a.DrawHandles(&hl, 1.0f, 0, true, false);
    CHECK(hl.at(2).c == xlYELLOWTRANSLUCENT);
    Program fb;
    a.DrawHandles(&fb, 1.0f, 0, true, true);
    CHECK(fb.at(2).c == FROM_BASE_HANDLES_COLOUR);
    a.Lock();
    Program lk;
    a.DrawHandles(&lk, 1.0f, 0, true, false);
    CHECK(lk.at(2).c == LOCKED_HANDLES_COLOUR);
}
