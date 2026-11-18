#include <algorithm>
#include <cmath>
#include <set>
#include <string>
#include <vector>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <pugixml.hpp>

#include "models/Model.h"
#include "models/ModelManager.h"
#include "models/Node.h"
#include "models/PolyLineModel.h"
#include "outputs/OutputManager.h"

using Catch::Approx;

namespace {

struct ShowFixture {
    OutputManager om;
    ModelManager mm{ &om };

    PolyLineModel* Add(const std::string& attrs) {
        std::string xml = "<model name=\"P\" DisplayAs=\"Poly Line\" StartChannel=\"1\" " + attrs + "/>";
        pugi::xml_document doc;
        REQUIRE(doc.load_string(xml.c_str()));
        Model* m = mm.createAndAddModel(doc.first_child(), 100, 100);
        REQUIRE(m != nullptr);
        mm.RecalcStartChannels();
        auto* p = dynamic_cast<PolyLineModel*>(m);
        REQUIRE(p != nullptr);
        return p;
    }
};

// An L: along x from (0,0) to (10,0), then up to (10,10). Screen coordinates
// are normalised to the unit square.
const std::string kL = "NumPoints=\"3\" PointData=\"0.0,0.0,0.0,10.0,0.0,0.0,10.0,10.0,0.0\"";

struct XY {
    float x, y;
};
XY Screen(const Model* m, int n, int c = 0) {
    const auto& co = m->GetNode(n)->Coords[c];
    return { co.screenX, co.screenY };
}

} // namespace

TEST_CASE("Poly line individual segments place each segment's lights on it", "[modelcore][polyline]") {
    ShowFixture f;
    auto* p = f.Add(kL + " Seg1=\"3\" Seg2=\"5\"");
    CHECK_FALSE(p->GetAutoDistribute());
    REQUIRE(p->GetNodeCount() == 8);
    CHECK(p->GetNumSegments() == 2);
    CHECK(p->GetNumStrands() == 2);
    CHECK(p->GetPolyLineSize(0) == 3);
    CHECK(p->GetPolyLineSize(1) == 5);
    CHECK(p->GetPolyLineSize(2) == 0);
    CHECK(p->GetStrandLength(1) == 5);
    CHECK(p->MapToNodeIndex(1, 2) == 5);
    CHECK(p->GetDropPoints() == 8);

    // lights sit at the centres of equal slots along each segment
    const float seg0[] = { 1.0f / 6, 0.5f, 5.0f / 6 };
    for (int i = 0; i < 3; ++i) {
        INFO(i);
        CHECK(Screen(p, i).x == Approx(seg0[i]).margin(1e-4));
        CHECK(Screen(p, i).y == Approx(0.0f).margin(1e-4));
    }
    for (int i = 0; i < 5; ++i) {
        INFO(i);
        CHECK(Screen(p, 3 + i).x == Approx(1.0f).margin(1e-4));
        CHECK(Screen(p, 3 + i).y == Approx(0.1f + 0.2f * i).margin(1e-4));
    }
    // and in wiring order in the buffer
    for (uint32_t i = 0; i < p->GetNodeCount(); ++i) {
        CHECK(p->GetNode(i)->Coords[0].bufX == (int)i);
        CHECK(p->NodeStartChannel(i) == (int32_t)(3 * i));
    }
    CHECK(p->IsNodeFirst(0));
    CHECK_FALSE(p->IsNodeFirst(7));
}

TEST_CASE("Poly line corners move the end lights onto the corner", "[modelcore][polyline]") {
    ShowFixture f;
    // a leading start corner pushes the first light a whole gap in; a trailing
    // middle corner gives the corner light to the second segment
    auto* p = f.Add(kL + " Seg1=\"3\" Seg2=\"5\" Corner1=\"Leading Segment\" Corner2=\"Trailing Segment\"");
    REQUIRE(p->GetNodeCount() == 8);
    auto corners = p->GetCorners();
    REQUIRE(corners.size() == 3);
    CHECK(corners[0] == "Leading Segment");
    CHECK(corners[1] == "Trailing Segment");
    CHECK(corners[2] == "Neither");
    // segment 0: lead 1.0, trail 1.0 -> 4 gaps of 3/4
    CHECK(Screen(p, 0).x == Approx(0.25f).margin(1e-4));
    CHECK(Screen(p, 2).x == Approx(0.75f).margin(1e-4));
    // segment 1: lead 0.0, trail 0.5 -> starts exactly at the corner
    CHECK(Screen(p, 3).x == Approx(1.0f).margin(1e-4));
    CHECK(Screen(p, 3).y == Approx(0.0f).margin(1e-4));
    CHECK(Screen(p, 4).y == Approx(1.0f / 4.5f).margin(1e-4));
}

TEST_CASE("Poly line auto distribution spaces lights by length", "[modelcore][polyline]") {
    ShowFixture f;
    auto* p = f.Add(kL + " NodesPerString=\"10\"");
    CHECK(p->GetAutoDistribute());
    REQUIRE(p->GetNodeCount() == 10);
    // ten lights over a path 20 long: one every 2 units (0.2 normalised),
    // first along the bottom, then up the side
    std::vector<XY> pts;
    for (int i = 0; i < 10; ++i) pts.push_back(Screen(p, i));
    for (int i = 1; i < 10; ++i) {
        INFO(i);
        float step = std::abs(pts[i].x - pts[i - 1].x) + std::abs(pts[i].y - pts[i - 1].y);
        CHECK(step == Approx(0.2f).margin(1e-3));
    }
    for (auto& q : pts) {
        bool onBottom = std::abs(q.y) < 1e-3f && q.x >= -1e-3f && q.x <= 1.0f + 1e-3f;
        bool onSide = std::abs(q.x - 1.0f) < 1e-3f && q.y >= -1e-3f && q.y <= 1.0f + 1e-3f;
        CHECK((onBottom || onSide));
    }
    // the per segment counts are worked out from where the lights landed
    CHECK(p->GetPolyLineSize(0) + p->GetPolyLineSize(1) == 10);
    CHECK(p->GetSegmentsSizes()[0] == p->GetPolyLineSize(0));
}

TEST_CASE("Poly line auto distribution centres lights in their slots", "[modelcore]") {
    ShowFixture f;
    auto* p = f.Add(kL + " NodesPerString=\"10\"");
    REQUIRE(p->GetNodeCount() == 10);
    for (int i = 0; i < 5; ++i) {
        INFO(i);
        CHECK(Screen(p, i).x == Approx(0.1f + 0.2f * i).margin(1e-3));
        CHECK(Screen(p, 5 + i).y == Approx(0.1f + 0.2f * i).margin(1e-3));
    }
    ShowFixture g;
    auto* s = g.Add("NumPoints=\"2\" PointData=\"0.0,0.0,0.0,10.0,3.0,0.0\" NodesPerString=\"4\"");
    for (int i = 0; i < 4; ++i) {
        CHECK(Screen(s, i).x == Approx(0.125f + 0.25f * i).margin(1e-3));
    }
    // an arch's lights should then be symmetric about its middle
    ShowFixture h;
    auto* a = h.Add("NumPoints=\"2\" PointData=\"0.0,0.0,0.0,10.0,0.0,0.0\" cPointData=\"0,3.0,5.0,0.0,7.0,5.0,0.0\" NodesPerString=\"5\"");
    CHECK(Screen(a, 2).x == Approx(0.5f).margin(0.02f));
    CHECK(Screen(a, 0).y == Approx(Screen(a, 4).y).margin(0.01f));
}

TEST_CASE("Poly line line segments buffer gives each segment a row", "[modelcore][polyline]") {
    ShowFixture f;
    auto* p = f.Add(kL + " Seg1=\"3\" Seg2=\"5\"");
    const auto& styles = p->GetBufferStyles();
    CHECK(std::find(styles.begin(), styles.end(), "Line Segments") != styles.end());
    std::vector<NodeBaseClassPtr> nodes;
    int bw = 0, bh = 0;
    p->InitRenderBufferNodes("Line Segments", "2D", "None", nodes, bw, bh, 0);
    CHECK(bw == 5);
    CHECK(bh == 2);
    REQUIRE(nodes.size() == 8);
    // the three lights of the short segment are stretched across the row
    CHECK(nodes[0]->Coords[0].bufX == 0);
    CHECK(nodes[1]->Coords[0].bufX == 2);
    CHECK(nodes[2]->Coords[0].bufX == 4);
    for (int i = 0; i < 3; ++i) CHECK(nodes[i]->Coords[0].bufY == 0);
    for (int i = 0; i < 5; ++i) {
        CHECK(nodes[3 + i]->Coords[0].bufX == i);
        CHECK(nodes[3 + i]->Coords[0].bufY == 1);
    }

    nodes.clear();
    p->InitRenderBufferNodes("Per Model Default", "2D", "None", nodes, bw, bh, 0);
    CHECK(bw == 8);
    CHECK(bh == 1);
}

TEST_CASE("Poly line curved segment bends the lights onto the curve", "[modelcore][polyline]") {
    ShowFixture f;
    // a symmetric arch: control points above the straight line between the ends
    auto* p = f.Add("NumPoints=\"2\" PointData=\"0.0,0.0,0.0,10.0,0.0,0.0\" cPointData=\"0,3.0,5.0,0.0,7.0,5.0,0.0\" NodesPerString=\"5\"");
    REQUIRE(p->GetNodeCount() == 5);
    // the lights climb the arch and come down the other side, never below the
    // chord and never above the apex (which normalises to y=1)
    CHECK(Screen(p, 0).y < Screen(p, 1).y);
    CHECK(Screen(p, 1).y < Screen(p, 2).y);
    CHECK(Screen(p, 4).y < Screen(p, 3).y);
    float topY = 0.0f;
    for (int i = 0; i < 5; ++i) {
        CHECK(Screen(p, i).y >= 0.0f);
        CHECK(Screen(p, i).y <= 1.0f + 1e-3f);
        topY = std::max(topY, Screen(p, i).y);
        if (i > 0) CHECK(Screen(p, i).x > Screen(p, i - 1).x);
    }
    CHECK(topY > 0.9f);

    // the same curve with an explicit segment size
    ShowFixture g;
    auto* q = g.Add("NumPoints=\"2\" PointData=\"0.0,0.0,0.0,10.0,0.0,0.0\" cPointData=\"0,3.0,5.0,0.0,7.0,5.0,0.0\" Seg1=\"5\"");
    REQUIRE(q->GetNodeCount() == 5);
    CHECK(Screen(q, 2).x == Approx(0.5f).margin(0.02f));
    CHECK(Screen(q, 2).y == Approx(1.0f).margin(0.02f));
    CHECK(Screen(q, 0).y < Screen(q, 2).y);
}

TEST_CASE("Poly line straight two point line follows its slope", "[modelcore][polyline]") {
    ShowFixture f;
    auto* p = f.Add("NumPoints=\"2\" PointData=\"0.0,0.0,0.0,10.0,3.0,0.0\" NodesPerString=\"4\"");
    REQUIRE(p->GetNodeCount() == 4);
    // both axes normalise to 0..1, so the lights run up the diagonal a quarter apart
    for (int i = 0; i < 4; ++i) {
        CHECK(Screen(p, i).y == Approx(Screen(p, i).x).margin(1e-4));
        if (i > 0) CHECK(Screen(p, i).x - Screen(p, i - 1).x == Approx(0.25f).margin(1e-3));
    }
}

TEST_CASE("Poly line upward drops grow from the bottom", "[modelcore][polyline]") {
    ShowFixture f;
    auto* p = f.Add("NumPoints=\"2\" PointData=\"0.0,0.0,0.0,10.0,0.0,0.0\" NodesPerString=\"5\" DropPattern=\"-3,2\"");
    REQUIRE(p->GetNodeCount() == 5);
    CHECK(p->GetDefaultBufferHt() == 3);
    CHECK(p->GetDefaultBufferWi() == 2);
    // first drop goes up from the bottom row, second hangs down from the top
    std::vector<std::pair<int, int>> cells;
    for (uint32_t i = 0; i < p->GetNodeCount(); ++i) {
        cells.push_back({ p->GetNode(i)->Coords[0].bufX, p->GetNode(i)->Coords[0].bufY });
    }
    CHECK(cells == std::vector<std::pair<int, int>>{ { 0, 0 }, { 0, 1 }, { 0, 2 }, { 1, 2 }, { 1, 1 } });
    // an upward drop's lights are above its anchor, a downward drop's below
    CHECK(Screen(p, 1).y > Screen(p, 0).y);
    CHECK(Screen(p, 4).y < Screen(p, 3).y);
}

TEST_CASE("Poly line alternate nodes interleave a drop", "[modelcore][polyline]") {
    ShowFixture f;
    auto* p = f.Add("NumPoints=\"2\" PointData=\"0.0,0.0,0.0,10.0,0.0,0.0\" NodesPerString=\"8\" DropPattern=\"4\" AlternateNodes=\"true\"");
    REQUIRE(p->HasAlternateNodes());
    REQUIRE(p->GetNodeCount() == 8);
    std::vector<int> ys;
    for (int i = 0; i < 4; ++i) ys.push_back(p->GetNode(i)->Coords[0].bufY);
    // down every other cell, then back up the gaps
    CHECK(ys == std::vector<int>{ 3, 1, 0, 2 });
    std::vector<int> up;
    ShowFixture g;
    auto* q = g.Add("NumPoints=\"2\" PointData=\"0.0,0.0,0.0,10.0,0.0,0.0\" NodesPerString=\"4\" DropPattern=\"-4\" AlternateNodes=\"true\"");
    for (int i = 0; i < 4; ++i) up.push_back(q->GetNode(i)->Coords[0].bufY);
    CHECK(up == std::vector<int>{ 0, 2, 3, 1 });
}

TEST_CASE("Poly line with several lights per node gives each node all its coords", "[modelcore][polyline]") {
    ShowFixture f;
    auto* p = f.Add("NumPoints=\"2\" PointData=\"0.0,0.0,0.0,10.0,0.0,0.0\" NodesPerString=\"3\" LightsPerNode=\"2\"");
    REQUIRE(p->GetNodeCount() == 3);
    CHECK(p->GetLightsPerNode() == 2);
    std::set<float> xs;
    for (uint32_t i = 0; i < 3; ++i) {
        REQUIRE(p->GetNode(i)->Coords.size() == 2);
        xs.insert(p->GetNode(i)->Coords[0].screenX);
        xs.insert(p->GetNode(i)->Coords[1].screenX);
    }
    // six distinct lights along the line
    CHECK(xs.size() == 6);
    CHECK(*xs.begin() >= 0.0f);
    CHECK(*xs.rbegin() <= 1.0f);
}

TEST_CASE("Poly line single colour string is one node with every light", "[modelcore][polyline]") {
    ShowFixture f;
    auto* p = f.Add(kL + " NodesPerString=\"6\" StringType=\"Single Color Red\"");
    REQUIRE(p->GetNodeCount() == 1);
    CHECK(p->GetNode(0)->Coords.size() == 6);
    CHECK(p->GetNumStrands() == 1);
    CHECK(p->GetStrandLength(0) == 1);
    CHECK(p->GetDefaultBufferWi() == 1);
    // lights still follow the L
    CHECK(p->GetNode(0)->Coords[0].screenY == Approx(0.0f).margin(1e-3));
    CHECK(p->GetNode(0)->Coords[5].screenX == Approx(1.0f).margin(1e-3));

    ShowFixture g;
    auto* q = g.Add(kL + " Seg1=\"2\" Seg2=\"2\" StringType=\"Single Color Red\"");
    REQUIRE(q->GetNodeCount() == 1);
    REQUIRE(q->GetNode(0)->Coords.size() == 4);
    CHECK(q->GetNode(0)->Coords[3].screenX == Approx(1.0f).margin(1e-3));
    CHECK(q->GetNode(0)->Coords[3].screenY == Approx(0.75f).margin(1e-3));
}

TEST_CASE("Poly line strings split the lights", "[modelcore][polyline]") {
    ShowFixture f;
    auto* p = f.Add(kL + " NodesPerString=\"6\" PolyStrings=\"2\"");
    CHECK(p->GetNumStrings() == 2);
    CHECK(p->GetNumPhysicalStrings() == 2);
    CHECK(p->NodesPerString() == 3);
    CHECK(p->NodesPerString(1) == 3);
    for (uint32_t i = 0; i < 3; ++i) {
        CHECK(p->NodeStartChannel(i) == (int32_t)(3 * i));
    }

    ShowFixture g;
    auto* q = g.Add(kL + " Seg1=\"3\" Seg2=\"3\" PolyStrings=\"2\" PolyNode1=\"1\" PolyNode2=\"5\" Advanced=\"1\" String1=\"1\" String2=\"101\"");
    CHECK(q->NodesPerString(0) == 4);
    CHECK(q->NodesPerString(1) == 2);
    CHECK(q->NodeStartChannel(0) == 0);
    CHECK(q->NodeStartChannel(3) == 9);
    // the second string starts at node 5 on its own start channel
    CHECK(q->NodeStartChannel(4) == 100);
    CHECK(q->NodeStartChannel(5) == 103);
}

TEST_CASE("Poly line strings get consecutive channel ranges", "[modelcore]") {
    ShowFixture f;
    auto* p = f.Add(kL + " NodesPerString=\"6\" PolyStrings=\"2\"");
    for (uint32_t i = 0; i < 6; ++i) {
        CHECK(p->NodeStartChannel(i) == (int32_t)(3 * i));
    }
}

TEST_CASE("Poly line swap start and end reverses segments and corners", "[modelcore][polyline]") {
    ShowFixture f;
    auto* p = f.Add(kL + " Seg1=\"3\" Seg2=\"5\" Corner1=\"Leading Segment\" Corner2=\"Trailing Segment\"");
    REQUIRE(p->SupportsSwapStartEnd());
    p->SwapStartEnd();
    CHECK(p->GetSegmentsSizes() == std::vector<int>{ 5, 3 });
    CHECK(p->GetCorners() == std::vector<std::string>{ "Neither", "Leading Segment", "Trailing Segment" });
    REQUIRE(p->GetNodeCount() == 8);
    // the first light is now at the top of the old last segment
    CHECK(Screen(p, 0).x == Approx(1.0f).margin(1e-3));
    // new segment 0 has lead 0.5 (old trail) and trail 0.0 (old lead): 4.5 gaps
    CHECK(Screen(p, 0).y == Approx(1.0f - 1.0f / 9.0f).margin(1e-3));
    CHECK(Screen(p, 7).y == Approx(0.0f).margin(1e-3));

    // a locked model does not swap
    p->GetModelScreenLocation().Lock(true);
    p->SwapStartEnd();
    CHECK(p->GetSegmentsSizes() == std::vector<int>{ 5, 3 });
}

TEST_CASE("Poly line inserting and deleting handles splits and joins segments", "[modelcore][polyline]") {
    ShowFixture f;
    auto* p = f.Add(kL + " Seg1=\"4\" Seg2=\"5\"");
    p->InsertHandle(0, 1.0f, 1);
    CHECK(p->GetSegmentsSizes() == std::vector<int>{ 2, 2, 5 });
    p->Reinitialize();
    CHECK(p->GetNumSegments() == 3);
    CHECK(p->GetNodeCount() == 9);

    p->DeleteHandle(1);
    CHECK(p->GetSegmentsSizes() == std::vector<int>{ 2, 5 });
    CHECK(p->GetNumSegments() == 2);
    CHECK(p->GetNodeCount() == 7);

    p->AddHandle();
    CHECK(p->GetSegmentsSizes().back() == 50);
    CHECK(p->GetAutoDistribute());
}

TEST_CASE("Poly line drop pattern parsing", "[modelcore][polyline]") {
    ShowFixture f;
    auto* p = f.Add("NumPoints=\"2\" PointData=\"0.0,0.0,0.0,10.0,0.0,0.0\" NodesPerString=\"6\" DropPattern=\"0,-2\"");
    CHECK(p->GetDropPattern() == "0,-2");
    // a zero drop counts as one light; height is the longest drop
    CHECK(p->GetDefaultBufferHt() == 2);
    CHECK(p->GetNodeCount() == 6);
    CHECK(p->GetDefaultBufferWi() == 4);
}
