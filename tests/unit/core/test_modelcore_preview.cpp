#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <pugixml.hpp>

#include "models/Model.h"
#include "models/ModelGroup.h"
#include "models/ModelManager.h"
#include "models/Node.h"
#include "outputs/OutputManager.h"

#include "test_modelcore_fakegfx.h"

using Catch::Approx;
using fakegfx::FakePreview;
using fakegfx::RecIVA;

namespace {

struct ShowFixture {
    OutputManager om;
    ModelManager mm{ &om };

    Model* Add(const std::string& xml) {
        pugi::xml_document doc;
        REQUIRE(doc.load_string(xml.c_str()));
        Model* m = mm.createAndAddModel(doc.first_child(), 1000, 1000);
        REQUIRE(m != nullptr);
        mm.RecalcStartChannels();
        return m;
    }
    // three horizontal strands of two nodes
    Model* Matrix(const std::string& name = "M", const std::string& extra = "") {
        return Add("<model name=\"" + name + "\" DisplayAs=\"Matrix\" StartChannel=\"1\" NumStrings=\"1\" NodesPerString=\"6\" StrandsPerString=\"3\" WorldPosX=\"500\" WorldPosY=\"500\" " + extra + "/>");
    }
};

RecIVA* Draw(Model* m, FakePreview& p, bool is3d = false, const xlColor* c = nullptr, bool allowSelected = false, bool wiring = false, bool highlightFirst = false, int highlightPixel = 0, float* bb = nullptr) {
    size_t before = p.ctx.ivas.size();
    m->DisplayModelOnWindow(&p, &p.ctx, p.solid.get(), p.transparent.get(), is3d, c, allowSelected, wiring, highlightFirst, highlightPixel, bb);
    return p.ctx.ivas.size() > before ? p.ctx.ivas.back() : nullptr;
}

void Paint(Model* m) {
    for (uint32_t i = 0; i < m->GetNodeCount(); ++i) {
        m->SetNodeColor(i, xlColor(10 * (i + 1), 0, 0));
    }
}

} // namespace

TEST_CASE("Model preview draws one point per node at its screen position", "[modelcore][preview]") {
    ShowFixture f;
    Model* m = f.Matrix();
    Paint(m);
    FakePreview p(1000, 1000, false);
    float bb[6] = {};
    RecIVA* v = Draw(m, p, false, nullptr, false, false, false, 0, bb);
    REQUIRE(v != nullptr);
    REQUIRE(v->v.size() == 6);
    std::set<uint32_t> seen;
    float minX = 1e9f, maxX = -1e9f, minY = 1e9f, maxY = -1e9f;
    for (auto& e : v->v) {
        const auto& c = m->GetNode(e.idx)->Coords[0];
        CHECK(e.x == c.screenX);
        CHECK(e.y == c.screenY);
        seen.insert(e.idx);
        minX = std::min(minX, e.x);
        maxX = std::max(maxX, e.x);
        minY = std::min(minY, e.y);
        maxY = std::max(maxY, e.y);
    }
    CHECK(seen.size() == 6);
    CHECK(bb[0] == minX);
    CHECK(bb[1] == minY);
    CHECK(bb[3] == maxX);
    CHECK(bb[4] == maxY);
    // node colours, fully opaque
    REQUIRE(v->colors.size() == 6);
    for (int i = 0; i < 6; ++i) {
        CHECK(v->colors[i] == xlColor(10 * (i + 1), 0, 0));
        CHECK(v->colors[i].alpha == 255);
    }
    CHECK(v->finalized == 1);
    // the solid program runs the cached draw
    p.Run();
    CHECK(p.ctx.points == 1);
    CHECK(p.ctx.vertsDrawn == 6);
    CHECK(p.ctx.matrixDepth == 0);
}

TEST_CASE("Model preview reuses its cache and refreshes only colours", "[modelcore][preview]") {
    ShowFixture f;
    Model* m = f.Matrix();
    FakePreview p(1000, 1000, false);
    RecIVA* v = Draw(m, p);
    REQUIRE(v != nullptr);
    m->SetNodeColor(2, xlGREEN);
    CHECK(Draw(m, p) == nullptr); // no new accumulator
    CHECK(v->colors[2] == xlGREEN);
    CHECK(v->colorFlushes == 1);

    // a change to the model rebuilds it
    m->SetPixelSize(5);
    RecIVA* v2 = Draw(m, p);
    CHECK(v2 != nullptr);
}

TEST_CASE("Model preview colour overrides and highlights", "[modelcore][preview]") {
    ShowFixture f;
    Model* m = f.Matrix();
    Paint(m);
    FakePreview p(1000, 1000, false);
    xlColor blue = xlBLUE;
    RecIVA* v = Draw(m, p, false, &blue);
    REQUIRE(v != nullptr);
    for (auto& c : v->colors) CHECK(c == xlBLUE);

    // highlight the first node in cyan, the second pixel in magenta
    FakePreview p2(1000, 1000, false);
    m->ClearRenderCaches();
    RecIVA* h = Draw(m, p2, false, nullptr, false, false, true, 2);
    REQUIRE(h != nullptr);
    CHECK(h->colors[0] == xlCYAN);
    CHECK(h->colors[1] == xlMAGENTA);
    // the rest keep their own colours
    CHECK(h->colors[2] == xlColor(30, 0, 0));

    FakePreview p3(1000, 1000, false);
    m->ClearRenderCaches();
    RecIVA* h2 = Draw(m, p3, false, &blue, false, false, true, 0);
    REQUIRE(h2 != nullptr);
    CHECK(h2->colors[0] == xlCYAN);
    CHECK(h2->colors[1] == xlBLUE);
}

TEST_CASE("Model preview applies transparency on the transparent program", "[modelcore][preview]") {
    ShowFixture f;
    Model* m = f.Matrix();
    Paint(m);
    m->SetTransparency(50);
    FakePreview p(1000, 1000, false);
    RecIVA* v = Draw(m, p);
    REQUIRE(v != nullptr);
    CHECK(v->colors[5].alpha == 127);
    // the draw step went to the transparent program
    p.transparent->runSteps(&p.ctx);
    CHECK(p.ctx.points == 1);
}

TEST_CASE("Model transparency maps to alpha", "[modelcore][preview]") {
    xlColor c(200, 100, 0);
    Model::ApplyTransparency(c, 0, 0);
    CHECK(c.alpha == 255);
    c = xlColor(200, 100, 0);
    Model::ApplyTransparency(c, 100, 0);
    CHECK(c.alpha == 0);
    c = xlColor(200, 100, 0);
    Model::ApplyTransparency(c, 20, 0);
    CHECK(c.alpha == 204);
    // black uses the black transparency only
    xlColor k = xlBLACK;
    Model::ApplyTransparency(k, 50, 0);
    CHECK(k.alpha == 255);
    k = xlBLACK;
    Model::ApplyTransparency(k, 0, 60);
    CHECK(k.alpha == 102);
    // dark colours blend towards the black transparency
    xlColor d(32, 0, 0);
    Model::ApplyTransparency(d, 0, 100);
    CHECK(d.alpha == 127);
    xlColor bright(255, 0, 0);
    Model::ApplyTransparency(bright, 0, 100);
    CHECK(bright.alpha == 255);
}

TEST_CASE("Model preview circle pixel styles draw triangles", "[modelcore][preview]") {
    ShowFixture f;
    Model* m = f.Matrix();
    Paint(m);
    m->SetPixelStyle(Model::PIXEL_STYLE::PIXEL_STYLE_SOLID_CIRCLE);
    FakePreview p(1000, 1000, false);
    RecIVA* v = Draw(m, p);
    REQUIRE(v != nullptr);
    REQUIRE(v->v.size() > 6);
    CHECK(v->v.size() % 6 == 0);
    for (auto& e : v->v) CHECK(e.idx < 6);
    p.Run();
    CHECK(p.ctx.triangles == 1);
    CHECK(p.ctx.points == 0);

    // blended circles fade to a transparent edge colour
    m->SetPixelStyle(Model::PIXEL_STYLE::PIXEL_STYLE_BLENDED_CIRCLE);
    FakePreview p2(1000, 1000, false);
    RecIVA* b = Draw(m, p2);
    REQUIRE(b != nullptr);
    REQUIRE(b->colors.size() == 12);
    std::set<uint32_t> idx;
    for (auto& e : b->v) idx.insert(e.idx);
    CHECK(*idx.rbegin() >= 6);
    for (int i = 0; i < 6; ++i) {
        CHECK(b->colors[i].alpha == 255);
        CHECK(b->colors[6 + i].alpha == 0);
        CHECK(b->colors[6 + i].red == b->colors[i].red);
    }
    // blended circles always draw on the transparent program
    p2.transparent->runSteps(&p2.ctx);
    CHECK(p2.ctx.triangles == 1);

    // a different backing scale rebuilds circle geometry
    p2.backingScale = 2.0;
    CHECK(Draw(m, p2) != nullptr);
}

TEST_CASE("Model 3D preview sorts nodes back to front", "[modelcore][preview]") {
    ShowFixture f;
    Model* m = f.Add(R"(<model name="C" DisplayAs="Cube" StartChannel="1" CubeWidth="2" CubeHeight="2" CubeDepth="3" Style="Horizontal Left/Right" Start="Front Bottom Left" WorldPosX="500" WorldPosY="500"/>)");
    m->SetPixelStyle(Model::PIXEL_STYLE::PIXEL_STYLE_SMOOTH);
    FakePreview p(1000, 1000, true);
    RecIVA* v = Draw(m, p, true);
    REQUIRE(v != nullptr);
    REQUIRE(v->v.size() == 12);
    // with the camera on +z, the view space z grows with screenZ: farthest first
    for (size_t i = 1; i < v->v.size(); ++i) {
        CHECK(v->v[i - 1].z <= v->v[i].z);
    }
    // square pixels in 3D keep wiring order (no depth sort)
    m->SetPixelStyle(Model::PIXEL_STYLE::PIXEL_STYLE_SQUARE);
    FakePreview p2(1000, 1000, true);
    RecIVA* s = Draw(m, p2, true);
    REQUIRE(s != nullptr);
    std::vector<uint32_t> order;
    for (auto& e : s->v) order.push_back(e.idx);
    std::vector<uint32_t> expect(12);
    for (uint32_t i = 0; i < 12; ++i) expect[i] = i;
    // NodeRenderOrder 1 (cube) walks in from both ends; every node appears once
    std::sort(order.begin(), order.end());
    CHECK(order == expect);
}

TEST_CASE("Model preview wiring view draws the node chain", "[modelcore][preview]") {
    ShowFixture f;
    Model* m = f.Matrix();
    FakePreview p(1000, 1000, false);
    Draw(m, p, false, nullptr, false, true);
    REQUIRE(p.ctx.vas.size() == 1);
    auto* va = p.ctx.vas.back();
    REQUIRE(va->v.size() == 6);
    for (int i = 0; i < 6; ++i) {
        CHECK(va->v[i].x == m->GetNode(i)->Coords[0].screenX);
        CHECK(va->v[i].y == m->GetNode(i)->Coords[0].screenY);
    }
    p.Run();
    CHECK(p.ctx.lineStrips == 1);
}

TEST_CASE("Selected model preview draws handles", "[modelcore][preview]") {
    ShowFixture f;
    Model* m = f.Matrix();
    m->Selected(true);
    xlColor c = xlWHITE;
    FakePreview p(1000, 1000, false);
    Draw(m, p, false, &c, true);
    CHECK(p.TransparentVerts().v.size() > 0);
    FakePreview p3(1000, 1000, true);
    Draw(m, p3, true, &c, true);
    CHECK(p3.TransparentVerts().v.size() > 0);

    // not selected: nothing but the model itself
    m->Selected(false);
    FakePreview p2(1000, 1000, false);
    Draw(m, p2, false, &c, true);
    CHECK(p2.TransparentVerts().v.empty());
}

TEST_CASE("Inactive model is not drawn when there is no current model", "[modelcore][preview]") {
    ShowFixture f;
    Model* m = f.Matrix();
    m->SetActive(false);
    FakePreview p(1000, 1000, false);
    p.noCurrentModel = true;
    CHECK(Draw(m, p) == nullptr);
    m->DisplayEffectOnWindow(&p, 2.0);
    CHECK(p.ctx.ivas.empty());
}

TEST_CASE("Model effect preview fits the model to the window", "[modelcore][preview]") {
    ShowFixture f;
    Model* m = f.Matrix();
    Paint(m);
    FakePreview p(300, 200, false);
    m->DisplayEffectOnWindow(&p, 2.0);
    REQUIRE(p.ctx.ivas.size() == 1);
    RecIVA* v = p.ctx.ivas.back();
    REQUIRE(v->v.size() == 6);
    for (auto& e : v->v) {
        const auto& c = m->GetNode(e.idx)->Coords[0];
        CHECK(e.x == c.screenX);
        CHECK(e.y == c.screenY);
    }
    REQUIRE(v->colors.size() == 12);
    for (int i = 0; i < 6; ++i) CHECK(v->colors[i] == xlColor(10 * (i + 1), 0, 0));
    p.Run();
    CHECK(p.ctx.points == 1);
    CHECK(p.ctx.matrixDepth == 0);

    // unchanged model and window reuse the cache
    m->SetNodeColor(0, xlWHITE);
    m->DisplayEffectOnWindow(&p, 2.0);
    CHECK(p.ctx.ivas.size() == 1);
    CHECK(v->colors[0] == xlWHITE);
    CHECK(v->colorFlushes == 1);

    // resizing the window rebuilds it
    p.w = 400;
    m->DisplayEffectOnWindow(&p, 2.0);
    CHECK(p.ctx.ivas.size() == 2);

    float sx = 0, sy = 0;
    int w = 0, h = 0;
    float scale = m->GetPreviewDimScale(&p, w, h);
    CHECK(w == 400);
    CHECK(h == 200);
    CHECK(scale > 0.0f);
    m->GetScreenLocation(sx, sy, m->GetNode(0)->Coords[0], w, h, scale);
    CHECK(sx >= 0.0f);
    CHECK(sx <= 400.0f);
    CHECK(sy >= 0.0f);
    CHECK(sy <= 200.0f);
}

TEST_CASE("Model effect preview of a group mixes pixel styles", "[modelcore][preview]") {
    ShowFixture f;
    Model* a = f.Matrix("A");
    Model* b = f.Matrix("B", "WorldPosX=\"700\"");
    b->SetPixelStyle(Model::PIXEL_STYLE::PIXEL_STYLE_BLENDED_CIRCLE);
    b->SetPixelSize(4);
    auto* g = dynamic_cast<ModelGroup*>(f.Add(R"(<modelGroup name="G" models="A,B" layout="minimalGrid" GridSize="400"/>)"));
    REQUIRE(g != nullptr);
    g->Reinitialize();
    REQUIRE(g->GetNodeCount() == 12);
    FakePreview p(400, 400, false);
    g->DisplayEffectOnWindow(&p, 2.0);
    REQUIRE(p.ctx.ivas.size() == 1);
    p.Run();
    // the points of A and the circles of B are drawn in separate steps
    CHECK(p.ctx.points == 1);
    CHECK(p.ctx.triangles == 1);
    RecIVA* v = p.ctx.ivas.back();
    // B's blended circle edges are transparent copies
    int transparentEdges = 0;
    for (size_t i = 12; i < v->colors.size(); ++i) {
        if (v->colors[i].alpha == 0) ++transparentEdges;
    }
    CHECK(transparentEdges == 6);
    (void)a;
}

TEST_CASE("Model node picking maps window points to nodes", "[modelcore][preview]") {
    ShowFixture f;
    Model* m = f.Matrix();
    FakePreview p(300, 200, false);
    std::map<int, std::pair<float, float>> coords;
    REQUIRE(m->GetScreenLocations(&p, coords));
    REQUIRE(coords.size() == 6);
    for (auto& [n, xy] : coords) {
        INFO(n);
        CHECK(xy.first >= 0.0f);
        CHECK(xy.first <= 300.0f);
        CHECK(xy.second >= 0.0f);
        CHECK(xy.second <= 200.0f);
        // the node near its own location is itself
        CHECK(m->GetNodeNear(&p, xlPoint((int)std::lround(xy.first), (int)std::lround(xy.second)), false) == std::to_string(n));
        // with flip the y axis is measured from the top
        CHECK(m->GetNodeNear(&p, xlPoint((int)std::lround(xy.first), 200 - (int)std::lround(xy.second)), true) == std::to_string(n));
    }
    CHECK(m->GetNodeNear(&p, xlPoint(-500, -500), false).empty());

    // a box around everything finds every node; an empty corner finds none
    auto all = m->GetNodesInBoundingBox(&p, xlPoint(300, 200), xlPoint(0, 0));
    CHECK(all.size() == 6);
    CHECK(m->GetNodesInBoundingBox(&p, xlPoint(-50, -50), xlPoint(-10, -10)).empty());

    // a pencil stroke along a row picks up its nodes segment by segment
    std::vector<int> bottom;
    for (auto& [n, xy] : coords) {
        if (xy.second == coords[1].second) bottom.push_back(n);
    }
    REQUIRE(bottom.size() == 2); // a horizontal matrix: 3 strands of 2
    auto at = [&](int n) { return xlPoint((int)std::lround(coords[n].first), (int)std::lround(coords[n].second)); };
    // a dot on the second node, then a stroke back to the first
    std::vector<xlPoint> path{ at(bottom[1]), at(bottom[1]), at(bottom[0]) };
    auto hit = m->GetNodesNearPath(&p, path);
    CHECK(hit == std::vector<int>{ bottom[1], bottom[0] });
    // within one segment nodes come in node order
    auto hit2 = m->GetNodesNearPath(&p, { at(bottom[1]), at(bottom[0]) });
    CHECK(hit2 == bottom);
    // a single point picks the nodes around it
    auto one = m->GetNodesNearPath(&p, { path[0] });
    CHECK(std::find(one.begin(), one.end(), bottom.back()) != one.end());
    CHECK(m->GetNodesNearPath(&p, {}).empty());
}

TEST_CASE("Model node coordinate queries", "[modelcore][preview]") {
    ShowFixture f;
    Model* m = f.Matrix();
    CHECK_FALSE(m->IsMultiCoordsPerNode());
    std::vector<xlPoint> pts;
    m->GetNodeCoords(4, pts);
    REQUIRE(pts.size() == 1);
    CHECK(pts[0].x == m->GetNode(4)->Coords[0].bufX);
    CHECK(pts[0].y == m->GetNode(4)->Coords[0].bufY);
    std::vector<std::tuple<float, float, float>> pts3;
    m->GetNode3DScreenCoords(4, pts3);
    REQUIRE(pts3.size() == 1);
    CHECK(std::get<0>(pts3[0]) == m->GetNode(4)->Coords[0].screenX);

    Model* s = f.Add(R"(<model name="S" DisplayAs="Single Line" StartChannel="1" NumStrings="1" NodesPerString="2" LightsPerNode="3" StringType="Single Color Red"/>)");
    CHECK(s->IsMultiCoordsPerNode());
    std::map<int, std::pair<float, float>> coords;
    FakePreview p(300, 200, false);
    CHECK_FALSE(s->GetScreenLocations(&p, coords));
}
