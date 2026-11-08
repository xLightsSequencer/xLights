#include <string>
#include <vector>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "models/TerrainScreenLocation.h"
#include "test_screenloc_common.h"

using namespace sl;
using Catch::Approx;
using handles::Axis;
using handles::Modifier;
using handles::Role;
using handles::UpdateResult;
using MSLAXIS = ModelScreenLocation::MSLAXIS;

namespace {

// 3 wide x 2 deep grid (spacing 50) centred at (100, 10, 0).
struct Grid {
    TerrainScreenLocation t;
    Grid() {
        t.UpdateSize(3, 2, 6);
        t.SetRenderSize(100.0f, 50.0f, 0.0f);
        t.SetWorldPosition(glm::vec3(100, 10, 0));
        t.PrepareToDraw(true, true);
        t.UpdateBoundingBox(100.0f, 50.0f, 50.0f);
    }
    std::vector<float> Heights() const {
        std::vector<float> h;
        for (int i = 0; i < t.GetNumPoints(); ++i) h.push_back(t.GetGridHeight(i));
        return h;
    }
};

} // namespace

TEST_CASE("Terrain grid size is clamped to something sane", "[screenloc]") {
    TerrainScreenLocation t;
    t.UpdateSize(3, 2, 6);
    CHECK(t.GetNumPoints() == 6);
    CHECK(t.GetNumPointsWide() == 3);
    CHECK(t.GetNumPointsDeep() == 2);
    CHECK(t.GetNumSelectableHandles() == 7);
    t.UpdateSize(0, 0, 0);
    CHECK(t.GetNumPoints() == 41 * 21);
    CHECK(t.GetNumPointsWide() == 41);
    t.UpdateSize(2000, 2000, 4000000);
    CHECK(t.GetNumPoints() == 41 * 21);
    CHECK(t.GetGridHeight(860) == 0.0f);
}

TEST_CASE("Terrain heights round trip through the run length data string", "[screenloc]") {
    Grid g;
    g.t.SetGridHeight(2, 5.0f);
    CHECK(g.t.GetDataAsString() == "3.000000,2.000000,0.000000,2.000000,5.000000,0.000000,3.000000");
    Grid h;
    h.t.SetDataFromString(g.t.GetDataAsString());
    CHECK(h.Heights() == std::vector<float>{ 0, 0, 5, 0, 0, 0 });

    Grid e;
    e.t.SetGridHeight(0, 1.0f);
    e.t.SetGridHeight(5, 2.0f);
    CHECK(e.t.GetDataAsString() == "3.000000,2.000000,1.000000,0.000000,4.000000,2.000000");
    Grid f;
    f.t.SetDataFromString(e.t.GetDataAsString());
    CHECK(f.Heights() == std::vector<float>{ 1, 0, 0, 0, 0, 2 });
}

TEST_CASE("Terrain data string is padded and clipped to the grid", "[screenloc]") {
    Grid g;
    g.t.SetGridHeight(4, 9.0f);
    g.t.SetDataFromString("3,2,1.5,2.5");
    CHECK(g.Heights() == std::vector<float>{ 1.5f, 2.5f, 0, 0, 0, 0 });
    g.t.SetDataFromString("3,2,0,100,7");
    CHECK(g.Heights() == std::vector<float>{ 0, 0, 0, 0, 0, 0 });
    g.t.SetDataFromString("3,2,1,2,3,4,5,6,7,8");
    CHECK(g.Heights() == std::vector<float>{ 1, 2, 3, 4, 5, 6 });
}

TEST_CASE("Terrain data string ending in a zero with no run length", "[screenloc]") {
    Grid g;
    g.t.SetGridHeight(4, 9.0f);
    g.t.SetDataFromString("3,2,7,0");
    CHECK(g.Heights() == std::vector<float>{ 7, 0, 0, 0, 0, 0 });
}

TEST_CASE("Terrain grid height accessors ignore out of range points", "[screenloc]") {
    Grid g;
    g.t.SetGridHeight(-1, 3.0f);
    g.t.SetGridHeight(6, 3.0f);
    CHECK(g.t.GetGridHeight(-1) == 0.0f);
    CHECK(g.t.GetGridHeight(6) == 0.0f);
    g.t.SetGridHeight(1, 3.0f);
    const auto* raw = static_cast<std::vector<float>*>(g.t.GetRawData());
    REQUIRE(raw->size() == 6);
    CHECK((*raw)[1] == 3.0f);
}

TEST_CASE("Terrain vertex world position follows the grid spacing", "[screenloc]") {
    Grid g;
    g.t.SetGridHeight(4, 7.0f);
    CheckVec(g.t.GetVertexWorldPosition(0), 50, 10, -25);
    CheckVec(g.t.GetVertexWorldPosition(4), 100, 17, 25);
    CheckVec(g.t.GetVertexWorldPosition(99), 0 * 50 - 50 + 100, 10, 33 * 50 - 25);
    g.t.SetScaleMatrix(glm::vec3(2, 2, 2));
    CheckVec(g.t.GetVertexWorldPosition(4), 100, 24, 50);
}

TEST_CASE("Terrain tool and axis rules switch to elevation on a grid point", "[screenloc]") {
    Grid g;
    auto& t = g.t;
    CHECK_FALSE(t.CanEdit());
    t.SetEdit(true);
    CHECK(t.CanEdit());
    CHECK_FALSE(t.IsElevationHandle());

    t.SetAxisTool(handles::Tool::Scale);
    CHECK(t.GetAxisTool() == handles::Tool::Scale);
    t.AdvanceAxisTool();
    CHECK(t.GetAxisTool() == handles::Tool::Rotate);
    t.SetActiveAxis(MSLAXIS::X_AXIS);
    CHECK(t.GetActiveAxis() == MSLAXIS::Y_AXIS);

    t.SetActiveHandle(MakeId(Role::Vertex, 4));
    CHECK(t.IsElevationHandle());
    CHECK(t.GetAxisTool() == handles::Tool::Elevate);
    t.AdvanceAxisTool();
    CHECK(t.GetAxisTool() == handles::Tool::Elevate);
    t.SetActiveAxis(MSLAXIS::Z_AXIS);
    CHECK(t.GetActiveAxis() == MSLAXIS::Y_AXIS);
    t.SetActiveAxis(MSLAXIS::NO_AXIS);
    CHECK(t.GetActiveAxis() == MSLAXIS::NO_AXIS);

    // an explicit tool change away from a grid point drops elevate
    t.SetActiveHandle(std::nullopt);
    t.SetAxisTool(handles::Tool::Rotate);
    CHECK(t.GetAxisTool() == handles::Tool::Rotate);
}

// SetActiveHandle passes the current tool back into SetAxisTool, so Elevate
// must not survive the move from a grid point to the centre.
TEST_CASE("Terrain selecting the centre after a grid point leaves elevate", "[screenloc]") {
    TerrainScreenLocation t;
    t.UpdateSize(3, 2, 6);
    t.SetActiveHandle(MakeId(Role::Vertex, 4));
    REQUIRE(t.GetAxisTool() == handles::Tool::Elevate);
    t.SetActiveHandleToCentre();
    CHECK(t.GetAxisTool() != handles::Tool::Elevate);
}

TEST_CASE("Terrain handles are the boxed ones unless editing in 3D", "[screenloc]") {
    Grid g;
    auto& t = g.t;
    CHECK(t.GetHandles(handles::ViewMode::TwoD, handles::Tool::Translate).size() == 5);
    CHECK(t.GetHandles(handles::ViewMode::ThreeD, handles::Tool::Translate).size() == 4);
    t.SetEdit(true);
    CHECK(t.GetHandles(handles::ViewMode::TwoD, handles::Tool::Translate).size() == 5);
    auto hs = t.GetHandles(handles::ViewMode::ThreeD, handles::Tool::Translate);
    REQUIRE(hs.size() == 6);
    CHECK(hs[0].selectionOnly);
    CheckVec(Find(hs, MakeId(Role::Vertex, 4))->worldPos, 100, 10, 25);
    t.SetActiveHandle(MakeId(Role::Vertex, 4));
    hs = t.GetHandles(handles::ViewMode::ThreeD, handles::Tool::Elevate);
    REQUIRE(hs.size() == 7);
    const auto* arrow = Find(hs, MakeId(Role::AxisArrow, 4, Axis::Y));
    REQUIRE(arrow);
    CheckVec(arrow->worldPos, 100, 64, 25);
    t.Lock();
    hs = t.GetHandles(handles::ViewMode::ThreeD, handles::Tool::Elevate);
    CHECK_FALSE(hs[0].editable);
}

TEST_CASE("Terrain elevation drag raises a grid point and its brush", "[screenloc]") {
    Grid g;
    auto& t = g.t;
    t.SetEdit(true);
    t.SetActiveHandle(MakeId(Role::Vertex, 4));
    auto s = t.CreateDragSession("terrain", MakeId(Role::AxisArrow, 4, Axis::Y), DownZ(100, 10));
    REQUIRE(s);
    CHECK(s->GetHandleId() == MakeId(Role::AxisArrow, 4, Axis::Y));
    CHECK(s->Update(DownZ(100, 22), Modifier::None) == UpdateResult::NeedsInit);
    CHECK(g.Heights() == std::vector<float>{ 0, 0, 0, 0, 12, 0 });
    handles::WorldRay flat;
    flat.origin = glm::vec3(0, 0, 0);
    flat.direction = glm::vec3(1, 0, 0);
    CHECK(s->Update(flat, Modifier::None) == UpdateResult::Unchanged);
    auto c = s->Commit();
    CHECK(c.modelName == "terrain");
    CHECK(c.dirty == handles::DirtyField::Vertex);
    s->Revert();
    CHECK(g.Heights() == std::vector<float>{ 0, 0, 0, 0, 0, 0 });
    CHECK(s->Commit().dirty == handles::DirtyField::None);

    t.SetGridHeight(0, 3.0f);
    t.SetToolSize(2);
    CHECK(t.GetToolSize() == 2);
    auto b = t.CreateDragSession("terrain", MakeId(Role::AxisArrow, 4, Axis::Y), DownZ(100, 10));
    b->Update(DownZ(100, 15), Modifier::None);
    CHECK(g.Heights() == std::vector<float>{ 5, 5, 5, 5, 5, 5 });
    b->Revert();
    CHECK(g.Heights() == std::vector<float>{ 3, 0, 0, 0, 0, 0 });

    // a ray that misses the plane at the start anchors on the grid point
    auto m = t.CreateDragSession("terrain", MakeId(Role::AxisArrow, 5, Axis::Y), flat);
    REQUIRE(m);
    m->Update(DownZ(0, 12), Modifier::None);
    CHECK(t.GetGridHeight(5) == Approx(2.0f));
}

TEST_CASE("Terrain elevation drags are refused for bad handles", "[screenloc]") {
    Grid g;
    auto& t = g.t;
    t.SetEdit(true);
    t.SetActiveHandle(MakeId(Role::Vertex, 4));
    CHECK(t.CreateDragSession("t", MakeId(Role::AxisArrow, 6, Axis::Y), DownZ(0, 0)) == nullptr);
    CHECK(t.CreateDragSession("t", MakeId(Role::AxisArrow, -1, Axis::Y), DownZ(0, 0)) == nullptr);
    // other handles fall back to the boxed sessions
    CHECK(t.CreateDragSession("t", MakeId(Role::AxisArrow, 4, Axis::X), DownZ(0, 0)) != nullptr);
    t.SetActiveHandleToCentre();
    auto boxed = t.CreateDragSession("t", MakeId(Role::AxisArrow, 0, Axis::Y), DownZ(0, 0));
    REQUIRE(boxed);
    boxed->Update(DownZ(0, 5), Modifier::None);
    CHECK(t.GetWorldPosition().y == Approx(15.0f));
    t.Lock();
    CHECK(t.CreateDragSession("t", MakeId(Role::AxisArrow, 4, Axis::Y), DownZ(0, 0)) == nullptr);
}

TEST_CASE("Terrain space mouse elevates grid points or moves the model", "[screenloc]") {
    Grid g;
    auto& t = g.t;
    auto v = t.BeginSpaceMouseSession(MakeId(Role::Vertex, 4));
    // not in elevate mode yet: nothing changes
    CHECK(v->Apply(1.0f, glm::vec3(0.0f), glm::vec3(0, 0, -3)) == handles::SpaceMouseResult::Dirty);
    CHECK(t.GetGridHeight(4) == 0.0f);
    t.SetActiveHandle(MakeId(Role::Vertex, 4));
    v->Apply(2.0f, glm::vec3(0.0f), glm::vec3(0, 0, -3));
    CHECK(g.Heights() == std::vector<float>{ 0, 0, 0, 0, 6, 0 });
    t.SetToolSize(2);
    v->Apply(1.0f, glm::vec3(0.0f), glm::vec3(0, 0, 1));
    CHECK(g.Heights() == std::vector<float>{ 5, 5, 5, 5, 5, 5 });
    CHECK(v->GetHandleId()->index == 4);
    auto bad = t.BeginSpaceMouseSession(MakeId(Role::Vertex, 60));
    bad->Apply(1.0f, glm::vec3(0.0f), glm::vec3(0, 0, 1));

    auto whole = t.BeginSpaceMouseSession(MakeId(Role::CentreCycle));
    CHECK(whole->Apply(1.0f, glm::vec3(0.0f), glm::vec3(1, 0, 0)) == handles::SpaceMouseResult::Dirty);
    CHECK(t.GetWorldPosition().x == Approx(101.0f));
    t.Lock();
    CHECK(whole->Apply(1.0f, glm::vec3(0.0f), glm::vec3(1, 0, 0)) == handles::SpaceMouseResult::Unchanged);
}

TEST_CASE("Terrain draws its bounding box and the editing handles", "[screenloc]") {
    Grid g;
    auto& t = g.t;
    const size_t cube = CubeVertexCount();

    Program bare;
    CHECK(t.DrawHandles(&bare, 1.0f, 0, true, false));
    REQUIRE(bare.size() == 24);
    CheckVec(bare.at(0).p, 50, 35, -25);
    CHECK(bare.at(0).c == xlWHITE);

    t.SetActiveHandleToCentre();
    t.SetAxisTool(handles::Tool::Translate);
    Program c;
    t.DrawHandles(&c, 1.0f, 0, true, false);
    CHECK(c.size() == cube + 3 * 18 * 3 + 6 + 24);
    CHECK(c.at(0).c == xlORANGETRANSLUCENT);

    t.SetEdit(true);
    t.SetActiveHandle(MakeId(Role::Vertex, 4));
    t.MouseOverHandle(MakeId(Role::Vertex, 3));
    t.SetActiveAxis(MSLAXIS::X_AXIS);
    t.SetGridHeight(0, -20.0f);
    t.SetGridHeight(1, 20.0f);
    Program e;
    t.DrawHandles(&e, 1.0f, 0, true, false);
    CHECK(e.size() == 2 * cube + 18 * 3 + 2 + 2 + 24);
    CHECK(e.at(0).c == xlBLUETRANSLUCENT);
    CHECK(e.at(cube).c == xlYELLOWTRANSLUCENT);
    CheckVec(e.at(2 * cube + 18 * 3 + 2).p, 100, -1000000.0f, 25);

    Program fb;
    t.DrawHandles(&fb, 1.0f, 0, true, true);
    CHECK(fb.back(0).c == FROM_BASE_HANDLES_COLOUR);
    t.Lock();
    Program lk;
    t.DrawHandles(&lk, 1.0f, 0, true, false);
    CHECK(lk.back(0).c == LOCKED_HANDLES_COLOUR);
    CHECK(lk.at(0).c == LOCKED_HANDLES_COLOUR);
}
