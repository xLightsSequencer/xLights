#include <algorithm>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <pugixml.hpp>

#include "models/Model.h"
#include "models/ModelManager.h"
#include "outputs/OutputManager.h"

namespace {

struct ShowFixture {
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

    Model* Make(const std::string& type, const std::string& attrs) {
        return Add("<model name=\"M\" DisplayAs=\"" + type + "\" StartChannel=\"1\" " + attrs + "/>");
    }
};

using Pt = std::pair<int, int>;

std::vector<Pt> Cells(const Model* m) {
    std::vector<Pt> pts;
    for (uint32_t i = 0; i < m->GetNodeCount(); ++i) {
        const auto& c = m->GetNode(i)->Coords[0];
        pts.push_back({ c.bufX, c.bufY });
    }
    return pts;
}

std::vector<int> Strings(const Model* m) {
    std::vector<int> r;
    for (uint32_t i = 0; i < m->GetNodeCount(); ++i) {
        r.push_back(m->GetNodeStringNumber(i));
    }
    return r;
}

std::vector<int> Chans(const Model* m) {
    std::vector<int> r;
    for (uint32_t i = 0; i < m->GetNodeCount(); ++i) {
        r.push_back(m->NodeStartChannel(i) / 3);
    }
    return r;
}

bool AllCellsDistinct(const Model* m) {
    auto pts = Cells(m);
    std::set<Pt> s(pts.begin(), pts.end());
    return s.size() == pts.size();
}

bool ChannelsAreAPermutation(const Model* m) {
    auto c = Chans(m);
    std::sort(c.begin(), c.end());
    for (size_t i = 0; i < c.size(); ++i) {
        if (c[i] != (int)i) {
            return false;
        }
    }
    return true;
}

} // namespace

TEST_CASE("Vertical matrix runs up and down the columns", "[models2][layouts]") {
    ShowFixture f;
    Model* m = f.Make("Matrix", "NumStrings=\"1\" NodesPerString=\"6\" StrandsPerString=\"3\" Vertical=\"true\"");
    CHECK(m->GetDefaultBufferWi() == 3);
    CHECK(m->GetDefaultBufferHt() == 2);
    CHECK(Cells(m) == std::vector<Pt>{ { 0, 0 }, { 0, 1 }, { 1, 1 }, { 1, 0 }, { 2, 0 }, { 2, 1 } });
}

TEST_CASE("Vertical matrix start side and direction pick the first node", "[models2][layouts]") {
    struct Case {
        const char* side;
        const char* dir;
        Pt first;
        Pt second;
    };
    const Case cases[] = {
        { "B", "L", { 0, 0 }, { 0, 1 } },
        { "B", "R", { 2, 0 }, { 2, 1 } },
        { "T", "L", { 0, 1 }, { 0, 0 } },
        { "T", "R", { 2, 1 }, { 2, 0 } },
    };
    for (const auto& c : cases) {
        INFO(c.side << c.dir);
        ShowFixture f;
        Model* m = f.Make("Matrix", std::string("StartSide=\"") + c.side + "\" Dir=\"" + c.dir + "\" NumStrings=\"1\" NodesPerString=\"6\" StrandsPerString=\"3\" Vertical=\"true\"");
        auto cells = Cells(m);
        CHECK(cells[0] == c.first);
        CHECK(cells[1] == c.second);
    }
}

TEST_CASE("Horizontal matrix start side and direction pick the first node", "[models2][layouts]") {
    struct Case {
        const char* side;
        const char* dir;
        Pt first;
        Pt last;
    };
    const Case cases[] = {
        { "B", "L", { 0, 0 }, { 1, 2 } },
        { "B", "R", { 1, 0 }, { 0, 2 } },
        { "T", "L", { 0, 2 }, { 1, 0 } },
        { "T", "R", { 1, 2 }, { 0, 0 } },
    };
    for (const auto& c : cases) {
        INFO(c.side << c.dir);
        ShowFixture f;
        Model* m = f.Make("Matrix", std::string("StartSide=\"") + c.side + "\" Dir=\"" + c.dir + "\" NumStrings=\"1\" NodesPerString=\"6\" StrandsPerString=\"3\"");
        auto cells = Cells(m);
        CHECK(cells.front() == c.first);
        CHECK(cells.back() == c.last);
        CHECK(AllCellsDistinct(m));
    }
}

TEST_CASE("Vertical matrix without zig zag always runs upward", "[models2][layouts]") {
    ShowFixture f;
    Model* m = f.Make("Matrix", "NumStrings=\"1\" NodesPerString=\"6\" StrandsPerString=\"3\" Vertical=\"true\" NoZig=\"true\"");
    CHECK(Cells(m) == std::vector<Pt>{ { 0, 0 }, { 0, 1 }, { 1, 0 }, { 1, 1 }, { 2, 0 }, { 2, 1 } });
}

TEST_CASE("Vertical matrix with two strings continues the columns", "[models2][layouts]") {
    ShowFixture f;
    Model* m = f.Make("Matrix", "NumStrings=\"2\" NodesPerString=\"6\" StrandsPerString=\"3\" Vertical=\"true\"");
    CHECK(m->GetDefaultBufferWi() == 6);
    CHECK(m->GetDefaultBufferHt() == 2);
    CHECK(Strings(m) == std::vector<int>{ 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 });
    auto cells = Cells(m);
    CHECK(cells[6] == Pt{ 3, 0 });
    CHECK(cells[7] == Pt{ 3, 1 });
    CHECK(cells[8] == Pt{ 4, 1 });
}

TEST_CASE("Horizontal matrix with two strings stacks the strings", "[models2][layouts]") {
    ShowFixture f;
    Model* m = f.Make("Matrix", "NumStrings=\"2\" NodesPerString=\"4\" StrandsPerString=\"2\"");
    CHECK(m->GetDefaultBufferWi() == 2);
    CHECK(m->GetDefaultBufferHt() == 4);
    CHECK(Cells(m) == std::vector<Pt>{ { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 }, { 0, 2 }, { 1, 2 }, { 1, 3 }, { 0, 3 } });
    CHECK(Strings(m) == std::vector<int>{ 0, 0, 0, 0, 1, 1, 1, 1 });
}

TEST_CASE("Matrix channels are consecutive in node order for every orientation", "[models2][layouts]") {
    for (const char* side : { "B", "T" }) {
        for (const char* dir : { "L", "R" }) {
            for (const char* vert : { "true", "false" }) {
                INFO(side << dir << vert);
                ShowFixture f;
                Model* m = f.Make("Matrix", std::string("StartSide=\"") + side + "\" Dir=\"" + dir + "\" Vertical=\"" + vert + "\" NumStrings=\"2\" NodesPerString=\"6\" StrandsPerString=\"3\"");
                CHECK(Chans(m) == std::vector<int>{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 });
                CHECK(AllCellsDistinct(m));
                CHECK(m->GetNodeCount() == 12);
            }
        }
    }
}

TEST_CASE("Matrix buffer styles give known sizes", "[models2][layouts]") {
    ShowFixture f;
    Model* m = f.Make("Matrix", "NumStrings=\"2\" NodesPerString=\"12\" StrandsPerString=\"3\"");
    REQUIRE(m->GetDefaultBufferWi() == 4);
    REQUIRE(m->GetDefaultBufferHt() == 6);
    int w = 0, h = 0;
    m->GetBufferSize("Default", "2D", "None", w, h, 0);
    CHECK(w == 4);
    CHECK(h == 6);
    m->GetBufferSize("Single Line", "2D", "None", w, h, 0);
    CHECK(w == 24);
    CHECK(h == 1);
    m->GetBufferSize("Default", "2D", "Rotate CC 90", w, h, 0);
    CHECK(w == 6);
    CHECK(h == 4);
    m->GetBufferSize("Default", "2D", "Rotate 180", w, h, 0);
    CHECK(w == 4);
    CHECK(h == 6);
    m->GetBufferSize("Default", "2D", "Rotate CW 90", w, h, 0);
    CHECK(w == 6);
    CHECK(h == 4);
    m->GetBufferSize("Default", "2D", "Flip Vertical", w, h, 0);
    CHECK(w == 4);
    CHECK(h == 6);
}

TEST_CASE("Per strand buffer styles of a matrix use strands as the axis", "[models2][layouts]") {
    ShowFixture f;
    Model* m = f.Make("Matrix", "NumStrings=\"1\" NodesPerString=\"12\" StrandsPerString=\"3\"");
    int w = 0, h = 0;
    m->GetBufferSize("Horizontal Per Strand", "2D", "None", w, h, 0);
    CHECK(w == 3);
    CHECK(h == 4);
    m->GetBufferSize("Vertical Per Strand", "2D", "None", w, h, 0);
    CHECK(w == 4);
    CHECK(h == 3);
    m->GetBufferSize("Horizontal Per Model/Strand", "2D", "None", w, h, 0);
    CHECK(w == 3);
    CHECK(h == 4);
    m->GetBufferSize("As Pixel", "2D", "None", w, h, 0);
    CHECK(w == 1);
    CHECK(h == 1);
}

TEST_CASE("Per strand render buffers place every node exactly once", "[models2][layouts]") {
    ShowFixture f;
    Model* m = f.Make("Matrix", "NumStrings=\"2\" NodesPerString=\"12\" StrandsPerString=\"3\"");
    for (const char* style : { "Default", "Single Line", "Horizontal Per Strand", "Vertical Per Strand", "As Pixel", "Per Preview" }) {
        INFO(style);
        std::vector<NodeBaseClassPtr> nodes;
        int w = 0, h = 0;
        m->InitRenderBufferNodes(style, "2D", "None", nodes, w, h, 0);
        CHECK(nodes.size() == 24);
        CHECK(w >= 1);
        CHECK(h >= 1);
        for (const auto& n : nodes) {
            for (const auto& c : n->Coords) {
                CHECK(c.bufX >= 0);
                CHECK(c.bufX < w);
                CHECK(c.bufY >= 0);
                CHECK(c.bufY < h);
            }
        }
    }
}

TEST_CASE("Rotating the render buffer moves nodes to the rotated cells", "[models2][layouts]") {
    ShowFixture f;
    Model* m = f.Make("Matrix", "NumStrings=\"1\" NodesPerString=\"6\" StrandsPerString=\"3\"");
    std::vector<NodeBaseClassPtr> plain;
    int w = 0, h = 0;
    m->InitRenderBufferNodes("Default", "2D", "None", plain, w, h, 0);
    REQUIRE(w == 2);
    REQUIRE(h == 3);
    std::vector<NodeBaseClassPtr> flipped;
    int fw = 0, fh = 0;
    m->InitRenderBufferNodes("Default", "2D", "Flip Vertical", flipped, fw, fh, 0);
    REQUIRE(flipped.size() == plain.size());
    for (size_t i = 0; i < plain.size(); ++i) {
        CHECK(flipped[i]->Coords[0].bufX == plain[i]->Coords[0].bufX);
        CHECK(flipped[i]->Coords[0].bufY == h - 1 - plain[i]->Coords[0].bufY);
    }
    std::vector<NodeBaseClassPtr> mirrored;
    m->InitRenderBufferNodes("Default", "2D", "Flip Horizontal", mirrored, fw, fh, 0);
    for (size_t i = 0; i < plain.size(); ++i) {
        CHECK(mirrored[i]->Coords[0].bufX == w - 1 - plain[i]->Coords[0].bufX);
        CHECK(mirrored[i]->Coords[0].bufY == plain[i]->Coords[0].bufY);
    }
}

TEST_CASE("Tree start side and direction order the strings", "[models2][layouts]") {
    struct Case {
        const char* side;
        const char* dir;
        std::vector<Pt> cells;
    };
    const Case cases[] = {
        { "B", "L", { { 0, 0 }, { 0, 1 }, { 1, 0 }, { 1, 1 }, { 2, 0 }, { 2, 1 } } },
        { "B", "R", { { 2, 0 }, { 2, 1 }, { 1, 0 }, { 1, 1 }, { 0, 0 }, { 0, 1 } } },
        { "T", "L", { { 0, 1 }, { 0, 0 }, { 1, 1 }, { 1, 0 }, { 2, 1 }, { 2, 0 } } },
        { "T", "R", { { 2, 1 }, { 2, 0 }, { 1, 1 }, { 1, 0 }, { 0, 1 }, { 0, 0 } } },
    };
    for (const auto& c : cases) {
        INFO(c.side << c.dir);
        ShowFixture f;
        Model* m = f.Make("Tree 360", std::string("StartSide=\"") + c.side + "\" Dir=\"" + c.dir + "\" NumStrings=\"3\" NodesPerString=\"2\" StrandsPerString=\"1\"");
        CHECK(Cells(m) == c.cells);
        CHECK(Strings(m) == std::vector<int>{ 0, 0, 1, 1, 2, 2 });
        CHECK(Chans(m) == std::vector<int>{ 0, 1, 2, 3, 4, 5 });
    }
}

TEST_CASE("Tree with several strands per string zig zags unless disabled", "[models2][layouts]") {
    ShowFixture f;
    Model* zig = f.Make("Tree 360", "NumStrings=\"2\" NodesPerString=\"4\" StrandsPerString=\"2\"");
    CHECK(zig->GetDefaultBufferWi() == 4);
    CHECK(zig->GetDefaultBufferHt() == 2);
    CHECK(Cells(zig) == std::vector<Pt>{ { 0, 0 }, { 0, 1 }, { 1, 1 }, { 1, 0 }, { 2, 0 }, { 2, 1 }, { 3, 1 }, { 3, 0 } });
    ShowFixture g;
    Model* nozig = g.Make("Tree 360", "NumStrings=\"2\" NodesPerString=\"4\" StrandsPerString=\"2\" NoZig=\"true\"");
    CHECK(Cells(nozig) == std::vector<Pt>{ { 0, 0 }, { 0, 1 }, { 1, 0 }, { 1, 1 }, { 2, 0 }, { 2, 1 }, { 3, 0 }, { 3, 1 } });
}

TEST_CASE("Tree display types differ in screen shape but share the buffer", "[models2][layouts]") {
    ShowFixture f;
    Model* round = f.Make("Tree 360", "NumStrings=\"4\" NodesPerString=\"5\" StrandsPerString=\"1\"");
    ShowFixture g;
    Model* flat = g.Make("Tree Flat", "NumStrings=\"4\" NodesPerString=\"5\" StrandsPerString=\"1\"");
    CHECK(Cells(round) == Cells(flat));
    CHECK(round->GetDefaultBufferWi() == flat->GetDefaultBufferWi());
    float roundDepthSpan = 0.0f, flatDepthSpan = 0.0f;
    for (uint32_t i = 0; i < round->GetNodeCount(); ++i) {
        roundDepthSpan = std::max(roundDepthSpan, std::fabs(round->GetNode(i)->Coords[0].screenZ));
        flatDepthSpan = std::max(flatDepthSpan, std::fabs(flat->GetNode(i)->Coords[0].screenZ));
    }
    CHECK(flatDepthSpan <= roundDepthSpan);
}

TEST_CASE("Tree round layout spreads strings around the circle", "[models2][layouts]") {
    ShowFixture f;
    Model* m = f.Make("Tree 360", "NumStrings=\"4\" NodesPerString=\"3\" StrandsPerString=\"1\"");
    std::set<int> xs;
    float minX = 1e9f, maxX = -1e9f;
    for (uint32_t i = 0; i < m->GetNodeCount(); ++i) {
        float x = m->GetNode(i)->Coords[0].screenX;
        minX = std::min(minX, x);
        maxX = std::max(maxX, x);
    }
    CHECK(minX < 0.0f);
    CHECK(maxX > 0.0f);
    float bottom0 = m->GetNode(0)->Coords[0].screenX;
    float bottom2 = m->GetNode(6)->Coords[0].screenX;
    CHECK(bottom0 == Catch::Approx(-bottom2).margin(1e-2));
}

TEST_CASE("Arches have one row per arch with nodes along x", "[models2][layouts]") {
    ShowFixture f;
    Model* m = f.Make("Arches", "NumArches=\"3\" NodesPerArch=\"4\"");
    CHECK(m->GetDefaultBufferWi() == 4);
    CHECK(m->GetDefaultBufferHt() == 3);
    CHECK(Strings(m) == std::vector<int>{ 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2 });
    CHECK(Cells(m) == std::vector<Pt>{ { 0, 0 }, { 1, 0 }, { 2, 0 }, { 3, 0 }, { 0, 1 }, { 1, 1 }, { 2, 1 }, { 3, 1 }, { 0, 2 }, { 1, 2 }, { 2, 2 }, { 3, 2 } });
}

TEST_CASE("Arches direction right reverses the wiring order", "[models2][layouts]") {
    ShowFixture f;
    Model* l = f.Make("Arches", "NumArches=\"2\" NodesPerArch=\"3\" Dir=\"L\"");
    ShowFixture g;
    Model* r = g.Make("Arches", "NumArches=\"2\" NodesPerArch=\"3\" Dir=\"R\"");
    CHECK(Cells(l) == Cells(r));
    auto cl = Chans(l);
    auto cr = Chans(r);
    for (size_t i = 0; i < cl.size(); ++i) {
        CHECK(cr[i] == 5 - cl[i]);
    }
}

TEST_CASE("Arches buffer styles", "[models2][layouts]") {
    ShowFixture f;
    Model* m = f.Make("Arches", "NumArches=\"3\" NodesPerArch=\"4\"");
    int w = 0, h = 0;
    m->GetBufferSize("Single Line", "2D", "None", w, h, 0);
    CHECK(w == 12);
    CHECK(h == 1);
    m->GetBufferSize("Default", "2D", "None", w, h, 0);
    CHECK(w == 4);
    CHECK(h == 3);
}

TEST_CASE("Candy canes are columns running upward", "[models2][layouts]") {
    ShowFixture f;
    Model* m = f.Make("Candy Canes", "NumCanes=\"3\" NodesPerCane=\"4\"");
    CHECK(m->GetDefaultBufferWi() == 3);
    CHECK(m->GetDefaultBufferHt() == 4);
    CHECK(Strings(m) == std::vector<int>{ 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2 });
    auto cells = Cells(m);
    for (int cane = 0; cane < 3; ++cane) {
        for (int n = 0; n < 4; ++n) {
            CHECK(cells[cane * 4 + n] == Pt{ cane, n });
        }
    }
}

TEST_CASE("Candy cane direction right wires the last cane first", "[models2][layouts]") {
    ShowFixture f;
    Model* m = f.Make("Candy Canes", "NumCanes=\"2\" NodesPerCane=\"4\" Dir=\"R\"");
    CHECK(Chans(m) == std::vector<int>{ 4, 5, 6, 7, 0, 1, 2, 3 });
    CHECK(Cells(m) == std::vector<Pt>{ { 0, 0 }, { 0, 1 }, { 0, 2 }, { 0, 3 }, { 1, 0 }, { 1, 1 }, { 1, 2 }, { 1, 3 } });
}

TEST_CASE("Candy cane alternate nodes interleave the cane", "[models2][layouts]") {
    ShowFixture f;
    Model* m = f.Make("Candy Canes", "NumCanes=\"2\" NodesPerCane=\"4\" AlternateNodes=\"true\"");
    CHECK(Cells(m) == std::vector<Pt>{ { 0, 0 }, { 0, 2 }, { 0, 3 }, { 0, 1 }, { 1, 0 }, { 1, 2 }, { 1, 3 }, { 1, 1 } });
    CHECK(AllCellsDistinct(m));
}

TEST_CASE("Candy cane sticks keep the same node layout", "[models2][layouts]") {
    ShowFixture f;
    Model* m = f.Make("Candy Canes", "NumCanes=\"3\" NodesPerCane=\"2\" CandyCaneSticks=\"true\"");
    CHECK(m->GetNodeCount() == 6);
    CHECK(Cells(m) == std::vector<Pt>{ { 0, 0 }, { 0, 1 }, { 1, 0 }, { 1, 1 }, { 2, 0 }, { 2, 1 } });
}

TEST_CASE("Icicle drop pattern shapes each string", "[models2][layouts]") {
    ShowFixture f;
    Model* m = f.Make("Icicles", "NumStrings=\"2\" NodesPerString=\"4\" DropPattern=\"3,1\"");
    CHECK(m->GetDefaultBufferHt() == 3);
    CHECK(m->GetDefaultBufferWi() == 4);
    CHECK(Cells(m) == std::vector<Pt>{ { 0, 2 }, { 0, 1 }, { 0, 0 }, { 1, 2 }, { 2, 2 }, { 2, 1 }, { 2, 0 }, { 3, 2 } });
    CHECK(Strings(m) == std::vector<int>{ 0, 0, 0, 0, 1, 1, 1, 1 });
}

TEST_CASE("Icicle drop pattern repeats and truncates to the node count", "[models2][layouts]") {
    ShowFixture f;
    Model* m = f.Make("Icicles", "NumStrings=\"1\" NodesPerString=\"5\" DropPattern=\"1,2,3\"");
    CHECK(m->GetDefaultBufferWi() == 3);
    CHECK(m->GetDefaultBufferHt() == 3);
    CHECK(Cells(m) == std::vector<Pt>{ { 0, 2 }, { 1, 2 }, { 1, 1 }, { 2, 2 }, { 2, 1 } });
}

TEST_CASE("Icicle default drop pattern is a single node column", "[models2][layouts]") {
    ShowFixture f;
    Model* m = f.Make("Icicles", "NumStrings=\"1\" NodesPerString=\"6\"");
    CHECK(m->GetNodeCount() == 6);
    CHECK(AllCellsDistinct(m));
    CHECK(ChannelsAreAPermutation(m));
}

TEST_CASE("Single line strings are consecutive and honour direction", "[models2][layouts]") {
    ShowFixture f;
    Model* l = f.Make("Single Line", "NumStrings=\"2\" NodesPerString=\"3\"");
    CHECK(Cells(l) == std::vector<Pt>{ { 0, 0 }, { 1, 0 }, { 2, 0 }, { 3, 0 }, { 4, 0 }, { 5, 0 } });
    CHECK(Chans(l) == std::vector<int>{ 0, 1, 2, 3, 4, 5 });
    CHECK(Strings(l) == std::vector<int>{ 0, 0, 0, 1, 1, 1 });
    ShowFixture g;
    Model* r = g.Make("Single Line", "NumStrings=\"2\" NodesPerString=\"3\" Dir=\"R\"");
    CHECK(Chans(r) == std::vector<int>{ 2, 1, 0, 5, 4, 3 });
}

TEST_CASE("Single line buffer is always one row", "[models2][layouts]") {
    ShowFixture f;
    Model* m = f.Make("Single Line", "NumStrings=\"3\" NodesPerString=\"7\"");
    CHECK(m->GetDefaultBufferWi() == 21);
    CHECK(m->GetDefaultBufferHt() == 1);
    int w = 0, h = 0;
    m->GetBufferSize("Default", "2D", "Rotate CW 90", w, h, 0);
    CHECK(w == 1);
    CHECK(h == 21);
}

TEST_CASE("Star starting at the bottom places the first node at the bottom centre", "[models2][layouts]") {
    ShowFixture f;
    Model* m = f.Make("Star", "NumStrings=\"1\" NodesPerString=\"10\" StarPoints=\"5\" StarStartLocation=\"Bottom Ctr-CW\"");
    CHECK(m->GetDefaultBufferWi() == 11);
    CHECK(m->GetDefaultBufferHt() == 11);
    auto cells = Cells(m);
    CHECK(cells[0].first == 5);
    CHECK(cells[0].second <= 1);
    CHECK(AllCellsDistinct(m));
}

TEST_CASE("Star top start mirrors the bottom start vertically", "[models2][layouts]") {
    ShowFixture f;
    Model* bottom = f.Make("Star", "NumStrings=\"1\" NodesPerString=\"10\" StarPoints=\"5\" StarStartLocation=\"Bottom Ctr-CW\"");
    ShowFixture g;
    Model* top = g.Make("Star", "NumStrings=\"1\" NodesPerString=\"10\" StarPoints=\"5\" StarStartLocation=\"Top Ctr-CCW\"");
    auto b = Cells(bottom);
    auto t = Cells(top);
    REQUIRE(b.size() == t.size());
    CHECK(t[0].first == 5);
    CHECK(t[0].second >= 9);
    CHECK(AllCellsDistinct(top));
    CHECK(ChannelsAreAPermutation(top));
}

TEST_CASE("Star clockwise and counter clockwise starts visit nodes in opposite directions", "[models2][layouts]") {
    ShowFixture f;
    Model* cw = f.Make("Star", "NumStrings=\"1\" NodesPerString=\"10\" StarPoints=\"5\" StarStartLocation=\"Top Ctr-CW\"");
    ShowFixture g;
    Model* ccw = g.Make("Star", "NumStrings=\"1\" NodesPerString=\"10\" StarPoints=\"5\" StarStartLocation=\"Top Ctr-CCW\"");
    auto a = Cells(cw);
    auto b = Cells(ccw);
    CHECK(a[0] == b[0]);
    CHECK(a[1] != b[1]);
    CHECK(a[1] == b[9]);
}

TEST_CASE("Star layer star buffer has one row per string", "[models2][layouts]") {
    ShowFixture f;
    Model* m = f.Make("Star", "NumStrings=\"2\" NodesPerString=\"5\" StarPoints=\"5\" LayerSizes=\"5,5\" StarStartLocation=\"Bottom Ctr-CW\"");
    int w = 0, h = 0;
    m->GetBufferSize("Layer Star", "2D", "None", w, h, 0);
    CHECK(h == m->GetNumStrands());
    CHECK(w >= 1);
}

TEST_CASE("Star offers an extra layer buffer style", "[models2][layouts]") {
    ShowFixture f;
    Model* m = f.Make("Star", "NumStrings=\"1\" NodesPerString=\"10\" StarPoints=\"5\"");
    const auto& styles = m->GetBufferStyles();
    CHECK(std::find(styles.begin(), styles.end(), "Layer Star") != styles.end());
    CHECK(std::find(styles.begin(), styles.end(), "Default") != styles.end());
}

TEST_CASE("Wreath places nodes evenly around a circle", "[models2][layouts]") {
    ShowFixture f;
    Model* m = f.Make("Wreath", "NumStrings=\"2\" NodesPerString=\"4\"");
    CHECK(m->GetNodeCount() == 8);
    CHECK(m->GetDefaultBufferWi() == 9);
    CHECK(m->GetDefaultBufferHt() == 9);
    CHECK(Cells(m) == std::vector<Pt>{ { 4, 0 }, { 1, 1 }, { 0, 4 }, { 1, 7 }, { 4, 8 }, { 7, 7 }, { 8, 4 }, { 7, 1 } });
}

TEST_CASE("Circle layers are rows with the first layer outermost", "[models2][layouts]") {
    ShowFixture f;
    Model* m = f.Make("Circle", "NumStrings=\"1\" NodesPerString=\"8\" LayerSizes=\"5,3\"");
    CHECK(m->GetDefaultBufferWi() == 5);
    CHECK(m->GetDefaultBufferHt() == 2);
    CHECK(Cells(m) == std::vector<Pt>{ { 0, 0 }, { 2, 0 }, { 4, 0 }, { 0, 1 }, { 1, 1 }, { 2, 1 }, { 3, 1 }, { 4, 1 } });
}

TEST_CASE("Circle inside out reverses the layer order", "[models2][layouts]") {
    ShowFixture f;
    Model* m = f.Make("Circle", "NumStrings=\"1\" NodesPerString=\"8\" LayerSizes=\"5,3\" InsideOut=\"1\"");
    CHECK(Cells(m) == std::vector<Pt>{ { 0, 1 }, { 1, 1 }, { 2, 1 }, { 3, 1 }, { 4, 1 }, { 0, 0 }, { 2, 0 }, { 4, 0 } });
}

TEST_CASE("Circle with legacy circle sizes lists layers inner first", "[models2][layouts]") {
    ShowFixture f;
    Model* m = f.Make("Circle", "NumStrings=\"1\" NodesPerString=\"8\" circleSizes=\"5,3\"");
    CHECK(Cells(m) == std::vector<Pt>{ { 0, 0 }, { 1, 0 }, { 2, 0 }, { 3, 0 }, { 4, 0 }, { 0, 1 }, { 2, 1 }, { 4, 1 } });
}

TEST_CASE("Spinner arms are columns of nodes", "[models2][layouts]") {
    ShowFixture f;
    Model* m = f.Make("Spinner", "NumStrings=\"1\" NodesPerArm=\"3\" ArmsPerString=\"2\"");
    CHECK(m->GetNodeCount() == 6);
    CHECK(m->GetDefaultBufferWi() == 2);
    CHECK(m->GetDefaultBufferHt() == 3);
    CHECK(Cells(m) == std::vector<Pt>{ { 0, 0 }, { 0, 1 }, { 0, 2 }, { 1, 0 }, { 1, 1 }, { 1, 2 } });
}

TEST_CASE("Spinner zig zag reverses every other arm", "[models2][layouts]") {
    ShowFixture f;
    Model* m = f.Make("Spinner", "NumStrings=\"1\" NodesPerArm=\"3\" ArmsPerString=\"2\" ZigZag=\"true\"");
    CHECK(Cells(m) == std::vector<Pt>{ { 0, 0 }, { 0, 1 }, { 0, 2 }, { 1, 2 }, { 1, 1 }, { 1, 0 } });
}

TEST_CASE("Spinner with two strings continues the arm numbering", "[models2][layouts]") {
    ShowFixture f;
    Model* m = f.Make("Spinner", "NumStrings=\"2\" NodesPerArm=\"3\" ArmsPerString=\"2\"");
    CHECK(m->GetNodeCount() == 12);
    CHECK(m->GetDefaultBufferWi() == 4);
    CHECK(Strings(m) == std::vector<int>{ 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 });
    CHECK(AllCellsDistinct(m));
}

TEST_CASE("Window frame goes up the left across the top and down the right", "[models2][layouts]") {
    ShowFixture f;
    Model* m = f.Make("Window Frame", "TopNodes=\"3\" SideNodes=\"2\" BottomNodes=\"3\"");
    CHECK(m->GetNodeCount() == 10);
    CHECK(m->GetDefaultBufferWi() == 5);
    CHECK(m->GetDefaultBufferHt() == 2);
    CHECK(Cells(m) == std::vector<Pt>{ { 0, 0 }, { 0, 1 }, { 1, 1 }, { 2, 1 }, { 3, 1 }, { 4, 1 }, { 4, 0 }, { 3, 0 }, { 2, 0 }, { 1, 0 } });
}

TEST_CASE("Window frame counter clockwise reverses the circuit", "[models2][layouts]") {
    ShowFixture f;
    Model* m = f.Make("Window Frame", "TopNodes=\"3\" SideNodes=\"2\" BottomNodes=\"3\" Rotation=\"CCW\"");
    CHECK(Cells(m) == std::vector<Pt>{ { 0, 0 }, { 1, 0 }, { 2, 0 }, { 3, 0 }, { 4, 0 }, { 4, 1 }, { 3, 1 }, { 2, 1 }, { 1, 1 }, { 0, 1 } });
}

TEST_CASE("Window frame right and top start side flip the circuit", "[models2][layouts]") {
    ShowFixture f;
    Model* m = f.Make("Window Frame", "TopNodes=\"3\" SideNodes=\"2\" BottomNodes=\"3\" Rotation=\"CCW\" Dir=\"R\" StartSide=\"T\"");
    auto cells = Cells(m);
    CHECK(cells.front() == Pt{ 4, 1 });
    CHECK(cells[4] == Pt{ 0, 1 });
    CHECK(AllCellsDistinct(m));
}

TEST_CASE("Window frame with different top and bottom counts uses the wider width", "[models2][layouts]") {
    ShowFixture f;
    Model* m = f.Make("Window Frame", "TopNodes=\"6\" SideNodes=\"3\" BottomNodes=\"4\"");
    CHECK(m->GetNodeCount() == 6 + 3 * 2 + 4);
    CHECK(m->GetDefaultBufferWi() == 8);
    CHECK(m->GetDefaultBufferHt() == 3);
    CHECK(AllCellsDistinct(m));
}

TEST_CASE("Cube node cells are all distinct for common styles", "[models2][layouts]") {
    for (const char* style : { "Horizontal Left", "Horizontal Right", "Vertical Left", "Vertical Right", "Horizontal Back", "Vertical Back" }) {
        INFO(style);
        ShowFixture f;
        Model* m = f.Make("Cube", std::string("CubeWidth=\"3\" CubeHeight=\"2\" CubeDepth=\"2\" Style=\"") + style + "\" Strings=\"1\" Start=\"TopFrontLeft\"");
        CHECK(m->GetNodeCount() == 12);
        CHECK(ChannelsAreAPermutation(m));
    }
}

TEST_CASE("Cube buffer covers width times depth by height", "[models2][layouts]") {
    ShowFixture f;
    Model* m = f.Make("Cube", "CubeWidth=\"3\" CubeHeight=\"2\" CubeDepth=\"2\" Style=\"Horizontal Left\" Strings=\"1\" Start=\"TopFrontLeft\"");
    CHECK(m->GetDefaultBufferHt() == 2);
    CHECK(m->GetDefaultBufferWi() == 6);
    CHECK(AllCellsDistinct(m));
}

TEST_CASE("Cube with several strings numbers nodes by string", "[models2][layouts]") {
    ShowFixture f;
    Model* m = f.Make("Cube", "CubeWidth=\"2\" CubeHeight=\"2\" CubeDepth=\"1\" Style=\"Vertical Left\" Strings=\"2\" Start=\"TopFrontLeft\"");
    CHECK(m->GetNodeCount() == 4);
    auto s = Strings(m);
    CHECK(std::count(s.begin(), s.end(), 0) == 2);
    CHECK(std::count(s.begin(), s.end(), 1) == 2);
}

TEST_CASE("Sphere strands alternate direction", "[models2][layouts]") {
    ShowFixture f;
    Model* m = f.Make("Sphere", "NumStrings=\"1\" NodesPerString=\"6\" StrandsPerString=\"3\"");
    CHECK(m->GetDefaultBufferWi() == 3);
    CHECK(m->GetDefaultBufferHt() == 2);
    CHECK(Cells(m) == std::vector<Pt>{ { 0, 0 }, { 0, 1 }, { 1, 1 }, { 1, 0 }, { 2, 0 }, { 2, 1 } });
}

TEST_CASE("Sphere without zig zag keeps every strand the same way", "[models2][layouts]") {
    ShowFixture f;
    Model* m = f.Make("Sphere", "NumStrings=\"1\" NodesPerString=\"6\" StrandsPerString=\"3\" NoZig=\"true\"");
    CHECK(Cells(m) == std::vector<Pt>{ { 0, 0 }, { 0, 1 }, { 1, 0 }, { 1, 1 }, { 2, 0 }, { 2, 1 } });
}

TEST_CASE("Sphere screen positions stay within the latitude range", "[models2][layouts]") {
    ShowFixture f;
    Model* m = f.Make("Sphere", "NumStrings=\"1\" NodesPerString=\"20\" StrandsPerString=\"4\" StartLatitude=\"-90\" EndLatitude=\"90\" Degrees=\"360\"");
    float minY = 1e9f, maxY = -1e9f;
    for (uint32_t i = 0; i < m->GetNodeCount(); ++i) {
        float y = m->GetNode(i)->Coords[0].screenY;
        minY = std::min(minY, y);
        maxY = std::max(maxY, y);
    }
    CHECK(minY < 0.0f);
    CHECK(maxY > 0.0f);
    ShowFixture g;
    Model* narrow = g.Make("Sphere", "NumStrings=\"1\" NodesPerString=\"20\" StrandsPerString=\"4\" StartLatitude=\"-10\" EndLatitude=\"10\" Degrees=\"360\"");
    float nMin = 1e9f, nMax = -1e9f;
    for (uint32_t i = 0; i < narrow->GetNodeCount(); ++i) {
        float y = narrow->GetNode(i)->Coords[0].screenY;
        nMin = std::min(nMin, y);
        nMax = std::max(nMax, y);
    }
    CHECK((nMax - nMin) < (maxY - minY));
}

TEST_CASE("Channel block is a single node with the requested channels", "[models2][layouts]") {
    ShowFixture f;
    Model* m = f.Make("Channel Block", "parm1=\"1\" parm2=\"3\"");
    CHECK(m->GetDefaultBufferWi() == 1);
    CHECK(m->GetDefaultBufferHt() == 1);
    CHECK(m->GetNodeCount() == 1);
}

TEST_CASE("Poly line without alternate nodes lays nodes along one row", "[models2][layouts]") {
    ShowFixture f;
    Model* m = f.Make("Poly Line", "NumStrings=\"1\" NodesPerString=\"6\" NumPoints=\"3\" PointData=\"0.0,0.0,0.0,5.0,5.0,0.0,10.0,0.0,0.0\"");
    CHECK(m->GetDefaultBufferWi() == 6);
    CHECK(m->GetDefaultBufferHt() == 1);
    CHECK(Cells(m) == std::vector<Pt>{ { 0, 0 }, { 1, 0 }, { 2, 0 }, { 3, 0 }, { 4, 0 }, { 5, 0 } });
}

TEST_CASE("Strand and node counts follow each type's parameters", "[models2][layouts]") {
    struct Case {
        const char* type;
        const char* attrs;
        int nodes;
        int strings;
        int strands;
    };
    const Case cases[] = {
        { "Tree 360", "NumStrings=\"3\" NodesPerString=\"5\" StrandsPerString=\"1\"", 15, 3, 3 },
        { "Matrix", "NumStrings=\"2\" NodesPerString=\"12\" StrandsPerString=\"3\"", 24, 2, 6 },
        { "Arches", "NumArches=\"4\" NodesPerArch=\"5\"", 20, 4, 4 },
        { "Candy Canes", "NumCanes=\"5\" NodesPerCane=\"3\"", 15, 5, 5 },
        { "Single Line", "NumStrings=\"3\" NodesPerString=\"7\"", 21, 3, 3 },
        { "Icicles", "NumStrings=\"3\" NodesPerString=\"7\"", 21, 3, 3 },
        { "Wreath", "NumStrings=\"2\" NodesPerString=\"9\"", 18, 2, 2 },
        { "Window Frame", "TopNodes=\"4\" SideNodes=\"3\" BottomNodes=\"4\"", 14, 1, 1 },
        { "Cube", "CubeWidth=\"3\" CubeHeight=\"3\" CubeDepth=\"3\" Style=\"Horizontal Left\" Strings=\"1\" Start=\"TopFrontLeft\"", 27, 1, 1 },
    };
    for (const auto& c : cases) {
        INFO(c.type);
        ShowFixture f;
        Model* m = f.Make(c.type, c.attrs);
        CHECK((int)m->GetNodeCount() == c.nodes);
        CHECK(m->GetNumStrings() == c.strings);
        CHECK(m->GetNumStrands() >= 1);
        CHECK(ChannelsAreAPermutation(m));
    }
}

TEST_CASE("Lights per node produce multiple coordinates for sticks of lights", "[models2][layouts]") {
    ShowFixture f;
    Model* m = f.Make("Single Line", "NumStrings=\"1\" NodesPerString=\"3\" LightsPerNode=\"4\"");
    CHECK(m->GetNodeCount() == 3);
    CHECK(m->GetCoordCount(0) == 4);
    CHECK(m->GetCoordCount(2) == 4);
}

TEST_CASE("RGBW strings use four channels per node in the node layout", "[models2][layouts]") {
    ShowFixture f;
    Model* m = f.Make("Matrix", "NumStrings=\"1\" NodesPerString=\"6\" StrandsPerString=\"3\" StringType=\"RGBW Nodes\"");
    CHECK(m->GetChanCountPerNode() == 4);
    CHECK(m->GetChanCount() == 24);
    CHECK(m->NodeStartChannel(1) == 4);
    CHECK(m->NodeEndChannel(1) == 7);
}

TEST_CASE("RGB order strings keep the node positions", "[models2][layouts]") {
    ShowFixture f;
    Model* rgb = f.Make("Matrix", "NumStrings=\"1\" NodesPerString=\"6\" StrandsPerString=\"3\" StringType=\"RGB Nodes\"");
    ShowFixture g;
    Model* grb = g.Make("Matrix", "NumStrings=\"1\" NodesPerString=\"6\" StrandsPerString=\"3\" StringType=\"GRB Nodes\"");
    CHECK(Cells(rgb) == Cells(grb));
    CHECK(Chans(rgb) == Chans(grb));
}

TEST_CASE("Every model type keeps all nodes inside its default buffer", "[models2][layouts]") {
    struct Case {
        const char* type;
        const char* attrs;
    };
    const Case cases[] = {
        { "Tree 360", "NumStrings=\"3\" NodesPerString=\"5\" StrandsPerString=\"1\"" },
        { "Matrix", "NumStrings=\"2\" NodesPerString=\"12\" StrandsPerString=\"3\"" },
        { "Arches", "NumArches=\"4\" NodesPerArch=\"5\"" },
        { "Candy Canes", "NumCanes=\"5\" NodesPerCane=\"3\" AlternateNodes=\"true\"" },
        { "Single Line", "NumStrings=\"3\" NodesPerString=\"7\"" },
        { "Icicles", "NumStrings=\"3\" NodesPerString=\"7\" DropPattern=\"2,3,1\"" },
        { "Star", "NumStrings=\"1\" NodesPerString=\"15\" StarPoints=\"5\"" },
        { "Wreath", "NumStrings=\"2\" NodesPerString=\"9\"" },
        { "Circle", "NumStrings=\"1\" NodesPerString=\"12\" LayerSizes=\"8,4\"" },
        { "Spinner", "NumStrings=\"2\" NodesPerArm=\"4\" ArmsPerString=\"3\"" },
        { "Window Frame", "TopNodes=\"4\" SideNodes=\"3\" BottomNodes=\"4\"" },
        { "Sphere", "NumStrings=\"2\" NodesPerString=\"8\" StrandsPerString=\"2\"" },
        { "Poly Line", "NumStrings=\"1\" NodesPerString=\"7\" NumPoints=\"3\" PointData=\"0.0,0.0,0.0,5.0,5.0,0.0,10.0,0.0,0.0\"" },
    };
    for (const auto& c : cases) {
        INFO(c.type);
        ShowFixture f;
        Model* m = f.Make(c.type, c.attrs);
        int w = m->GetDefaultBufferWi();
        int h = m->GetDefaultBufferHt();
        for (uint32_t i = 0; i < m->GetNodeCount(); ++i) {
            for (const auto& co : m->GetNode(i)->Coords) {
                CHECK(co.bufX >= 0);
                CHECK(co.bufX < w);
                CHECK(co.bufY >= 0);
                CHECK(co.bufY < h);
            }
        }
    }
}
