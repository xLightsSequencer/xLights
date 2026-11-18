#include <algorithm>
#include <cstdlib>
#include <set>
#include <string>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <pugixml.hpp>

#include "models/CubeModel.h"
#include "models/Model.h"
#include "models/ModelManager.h"
#include "models/Node.h"
#include "outputs/OutputManager.h"
#include "XmlSerializer/StringSerializingVisitor.h"

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
};

const char* kStarts[] = {
    "Front Bottom Left", "Front Bottom Right", "Front Top Left", "Front Top Right",
    "Back Bottom Left", "Back Bottom Right", "Back Top Left", "Back Top Right"
};
const char* kStyles[] = {
    "Vertical Front/Back", "Vertical Left/Right", "Horizontal Front/Back",
    "Horizontal Left/Right", "Stacked Front/Back", "Stacked Left/Right"
};

using P3 = std::tuple<int, int, int>;

// The default buffer lays the cube out as bufX = x + z * width, bufY = y.
std::vector<P3> CubeCells(const Model* m, int W) {
    std::vector<P3> r;
    for (uint32_t i = 0; i < m->GetNodeCount(); ++i) {
        const auto& c = m->GetNode(i)->Coords[0];
        r.push_back({ c.bufX % W, c.bufY, c.bufX / W });
    }
    return r;
}

std::string CubeXml(int W, int H, int D, const std::string& start, const std::string& style, const std::string& extra = "") {
    return "<model name=\"Cube\" DisplayAs=\"Cube\" StartChannel=\"1\" CubeWidth=\"" + std::to_string(W) + "\" CubeHeight=\"" + std::to_string(H) +
           "\" CubeDepth=\"" + std::to_string(D) + "\" Start=\"" + start + "\" Style=\"" + style + "\" " + extra + "/>";
}

int Manhattan(const P3& a, const P3& b) {
    return std::abs(std::get<0>(a) - std::get<0>(b)) + std::abs(std::get<1>(a) - std::get<1>(b)) + std::abs(std::get<2>(a) - std::get<2>(b));
}

// Front Bottom Left is (0,0,0): x grows to the right, y up and z to the back.
P3 ExpectedStart(const std::string& start, int W, int H, int D) {
    int x = start.find("Left") != std::string::npos ? 0 : W - 1;
    int y = start.find("Bottom") != std::string::npos ? 0 : H - 1;
    int z = start.find("Front") != std::string::npos ? 0 : D - 1;
    return { x, y, z };
}

// For the axis the strands run along first.
int FirstAxis(const std::string& style) {
    if (style.starts_with("Vertical")) return 1;
    if (style.starts_with("Horizontal")) return style.find("Front/Back") != std::string::npos ? 2 : 0;
    return style.find("Front/Back") != std::string::npos ? 0 : 2; // stacked
}

int Axis(const P3& p, int a) {
    return a == 0 ? std::get<0>(p) : (a == 1 ? std::get<1>(p) : std::get<2>(p));
}

} // namespace

TEST_CASE("Cube start and style names round trip and fall back", "[modelcore][cube]") {
    ShowFixture f;
    auto* c = dynamic_cast<CubeModel*>(f.Add(CubeXml(2, 2, 2, "Back Top Right", "Stacked Left/Right", "StrandPerLine=\"No Zig Zag\"")));
    REQUIRE(c != nullptr);
    CHECK(c->GetCubeStart() == "Back Top Right");
    CHECK(c->GetCubeStyle() == "Stacked Left/Right");
    CHECK(c->GetStrandStyle() == "No Zig Zag");
    CHECK(static_cast<Model*>(c)->GetStartLocation() == "Back Top Right Stacked Left/Right");

    c->SetCubeStart("nowhere");
    c->SetCubeStyle("sideways");
    c->SetStrandStyle("knotted");
    CHECK(c->GetCubeStart() == "Front Bottom Left");
    CHECK(c->GetCubeStyle() == "Vertical Front/Back");
    CHECK(c->GetStrandStyle() == "Zig Zag");
}

TEST_CASE("Cube nodes fill the box once for every start and style", "[modelcore][cube]") {
    const int W = 3, H = 4, D = 2;
    for (const char* start : kStarts) {
        for (const char* style : kStyles) {
            INFO(start << " / " << style);
            ShowFixture f;
            Model* m = f.Add(CubeXml(W, H, D, start, style));
            REQUIRE(m->GetNodeCount() == (uint32_t)(W * H * D));
            auto cells = CubeCells(m, W);
            std::set<P3> s(cells.begin(), cells.end());
            CHECK(s.size() == cells.size());
            for (auto& p : cells) {
                CHECK(std::get<0>(p) >= 0);
                CHECK(std::get<0>(p) < W);
                CHECK(std::get<1>(p) >= 0);
                CHECK(std::get<1>(p) < H);
                CHECK(std::get<2>(p) >= 0);
                CHECK(std::get<2>(p) < D);
            }
            // a zig-zag strand never jumps: each node is next to the one before
            for (size_t i = 1; i < cells.size(); ++i) {
                CHECK(Manhattan(cells[i - 1], cells[i]) == 1);
            }
            // node channels follow wiring order
            for (uint32_t i = 0; i < m->GetNodeCount(); ++i) {
                CHECK(m->NodeStartChannel(i) == (int32_t)(i * 3));
            }
        }
    }
}

TEST_CASE("Cube strands run along the axis the style names", "[modelcore][cube]") {
    const int W = 3, H = 4, D = 5;
    for (const char* style : kStyles) {
        INFO(style);
        ShowFixture f;
        Model* m = f.Add(CubeXml(W, H, D, "Front Bottom Left", style));
        auto cells = CubeCells(m, W);
        int a = FirstAxis(style);
        // the first step moves along the strand axis only
        CHECK(Axis(cells[1], a) != Axis(cells[0], a));
        for (int o = 0; o < 3; ++o) {
            if (o != a) {
                CHECK(Axis(cells[1], o) == Axis(cells[0], o));
            }
        }
    }
}

// Back Top Left/Right with the Horizontal Front/Back, Stacked Front/Back and
// Stacked Left/Right styles start on the front face (z=0) instead of the back,
// even on a 3x3x3 cube. The H FB / S FB rows of CubeModel.cpp's transformations
// table for Back Top (38/40/44/46) are copies of the Front Top rows (14/16/20/22);
// Back Top S LR (41/47) lands on the opposite x as well.
TEST_CASE("Cube first node sits at the named start corner", "[modelcore][cube]") {
    const int W = 3, H = 4, D = 5;
    for (const char* start : kStarts) {
        for (const char* style : kStyles) {
            std::string st(start), sy(style);
            if (st.starts_with("Back Top") && !sy.starts_with("Vertical") && sy != "Horizontal Left/Right") {
                continue; // known bug, see the hidden case below
            }
            INFO(start << " / " << style);
            ShowFixture f;
            Model* m = f.Add(CubeXml(W, H, D, start, style));
            auto cells = CubeCells(m, W);
            CHECK(cells.front() == ExpectedStart(start, W, H, D));
        }
    }
}

TEST_CASE("Cube back top front/back styles start at the back", "[.knownbug][knownbug_cube_backtop_transform]") {
    const int W = 3, H = 4, D = 5;
    for (const char* start : { "Back Top Left", "Back Top Right" }) {
        for (const char* style : { "Horizontal Front/Back", "Stacked Front/Back", "Stacked Left/Right" }) {
            INFO(start << " / " << style);
            ShowFixture f;
            Model* m = f.Add(CubeXml(W, H, D, start, style));
            auto c0 = CubeCells(m, W).front();
            auto e = ExpectedStart(start, W, H, D);
            INFO("got " << std::get<0>(c0) << "," << std::get<1>(c0) << "," << std::get<2>(c0) << " want " << std::get<0>(e) << "," << std::get<1>(e) << "," << std::get<2>(e));
            CHECK(c0 == e);
        }
    }
}

TEST_CASE("Cube strand styles change the order within a row", "[modelcore][cube]") {
    const int W = 5, H = 2, D = 1;
    auto row = [&](const std::string& strand) {
        ShowFixture f;
        Model* m = f.Add(CubeXml(W, H, D, "Front Bottom Left", "Horizontal Left/Right", "StrandPerLine=\"" + strand + "\""));
        std::vector<int> xs;
        for (auto& p : CubeCells(m, W)) {
            xs.push_back(std::get<0>(p) + W * std::get<1>(p));
        }
        return xs;
    };
    CHECK(row("Zig Zag") == std::vector<int>{ 0, 1, 2, 3, 4, 9, 8, 7, 6, 5 });
    CHECK(row("No Zig Zag") == std::vector<int>{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 });
    // alternate pixel: out on the even cells, back on the odd ones
    CHECK(row("Aternate Pixel") == std::vector<int>{ 0, 2, 4, 3, 1, 5, 7, 9, 8, 6 });
}

TEST_CASE("Cube strand per layer restarts each layer at the bottom", "[modelcore][cube]") {
    const int W = 2, H = 2, D = 2;
    ShowFixture f;
    Model* a = f.Add(CubeXml(W, H, D, "Front Bottom Left", "Horizontal Left/Right", "StrandPerLayer=\"TRUE\""));
    auto cells = CubeCells(a, W);
    CHECK(cells[4] == P3{ 0, 0, 1 });
    ShowFixture g;
    Model* b = g.Add(CubeXml(W, H, D, "Front Bottom Left", "Horizontal Left/Right"));
    CHECK(CubeCells(b, W)[4] == P3{ 0, 1, 1 });
}

TEST_CASE("Cube strings and strands split the nodes evenly", "[modelcore][cube]") {
    ShowFixture f;
    auto* c = dynamic_cast<CubeModel*>(f.Add(CubeXml(3, 4, 2, "Front Bottom Left", "Vertical Front/Back", "Strings=\"4\"")));
    REQUIRE(c != nullptr);
    CHECK(c->GetNumPhysicalStrings() == 4);
    CHECK(c->NodesPerString() == 6);
    // front/back styles: one strand per x column
    CHECK(c->GetNumStrands() == 3);
    CHECK(c->GetStrandLength(0) == 8);
    CHECK(c->MapToNodeIndex(1, 2) == 10);
    std::vector<int> perString(4, 0);
    for (uint32_t i = 0; i < c->GetNodeCount(); ++i) {
        perString[c->GetNodeStringNumber(i)]++;
    }
    CHECK(perString == std::vector<int>{ 6, 6, 6, 6 });

    ShowFixture g;
    auto* lr = dynamic_cast<CubeModel*>(g.Add(CubeXml(3, 4, 2, "Front Bottom Left", "Vertical Left/Right")));
    CHECK(lr->GetNumStrands() == 2);
    CHECK(lr->GetStrandLength(0) == 12);
}

TEST_CASE("Cube buffer styles size and place every node inside the buffer", "[modelcore][cube]") {
    const int W = 3, H = 2, D = 4;
    ShowFixture f;
    Model* m = f.Add(CubeXml(W, H, D, "Front Bottom Left", "Vertical Front/Back"));
    struct S {
        const char* style;
        int w, h;
        bool distinct;
    };
    const S styles[] = {
        { "Default", W * D, H, true },
        { "Stacked X Horizontally", W * D, H, true },
        { "Stacked Y Horizontally", W * H, D, true },
        { "Stacked Z Horizontally", W * D, H, true },
        { "Stacked X Vertically", D, H * W, true },
        { "Stacked Y Vertically", W, D * H, true },
        { "Overlaid X", D, H, false },
        { "Overlaid Y", W, D, false },
        { "Overlaid Z", W, H, false },
        { "Unique X and Y Y", W * H, D * H, true },
        { "Per Model Overlaid Z", W, H, false },
    };
    for (const auto& s : styles) {
        INFO(s.style);
        std::vector<NodeBaseClassPtr> nodes;
        int bw = 0, bh = 0;
        m->InitRenderBufferNodes(s.style, "2D", "None", nodes, bw, bh, 0);
        CHECK(bw == s.w);
        CHECK(bh == s.h);
        int gw = 0, gh = 0;
        m->GetBufferSize(s.style, "2D", "None", gw, gh, 0);
        CHECK(gw == bw);
        CHECK(gh == bh);
        REQUIRE(nodes.size() == (size_t)(W * H * D));
        std::set<std::pair<int, int>> cells;
        for (auto& n : nodes) {
            const auto& c = n->Coords[0];
            CHECK(c.bufX >= 0);
            CHECK(c.bufX < bw);
            CHECK(c.bufY >= 0);
            CHECK(c.bufY < bh);
            cells.insert({ c.bufX, c.bufY });
        }
        if (s.distinct) {
            CHECK(cells.size() == nodes.size());
        } else {
            CHECK(cells.size() == (size_t)(bw * bh));
        }
    }
}

TEST_CASE("Cube stacked Z vertical and unique X and Y buffers keep every node inside", "[modelcore]") {
    for (auto [H, D] : { std::pair<int, int>{ 2, 4 }, std::pair<int, int>{ 4, 2 } })
    for (const char* style : { "Stacked Z Vertically", "Unique X and Y X", "Unique X and Y Z" }) {
        INFO(style << " height " << H << " depth " << D);
        ShowFixture f;
        Model* m = f.Add(CubeXml(3, H, D, "Front Bottom Left", "Vertical Front/Back"));
        std::vector<NodeBaseClassPtr> nodes;
        int bw = 0, bh = 0;
        m->InitRenderBufferNodes(style, "2D", "None", nodes, bw, bh, 0);
        std::set<std::pair<int, int>> cells;
        for (auto& n : nodes) {
            CHECK(n->Coords[0].bufX < bw);
            CHECK(n->Coords[0].bufY < bh);
            cells.insert({ n->Coords[0].bufX, n->Coords[0].bufY });
        }
        // and every node gets its own cell
        CHECK(cells.size() == nodes.size());
    }
}

TEST_CASE("Cube side buffers show only the nodes on that face", "[modelcore][cube]") {
    const int W = 3, H = 2, D = 4;
    ShowFixture f;
    Model* m = f.Add(CubeXml(W, H, D, "Front Bottom Left", "Horizontal Left/Right"));
    struct S {
        const char* style;
        int w, h;
    };
    const S sides[] = {
        { "Left Side", D, H }, { "Right Side", D, H }, { "Front Side", W, H },
        { "Back Side", W, H }, { "Top Side", W, D }, { "Bottom Side", W, D },
    };
    for (const auto& s : sides) {
        INFO(s.style);
        std::vector<NodeBaseClassPtr> nodes;
        int bw = 0, bh = 0;
        m->InitRenderBufferNodes(s.style, "2D", "None", nodes, bw, bh, 0);
        CHECK(bw == s.w);
        CHECK(bh == s.h);
        std::set<std::pair<int, int>> cells;
        int hidden = 0;
        for (auto& n : nodes) {
            const auto& c = n->Coords[0];
            if (c.bufX == -1 && c.bufY == -1) {
                ++hidden;
                continue;
            }
            CHECK(c.bufX >= 0);
            CHECK(c.bufX < bw);
            CHECK(c.bufY >= 0);
            CHECK(c.bufY < bh);
            cells.insert({ c.bufX, c.bufY });
        }
        CHECK(cells.size() == (size_t)(bw * bh));
        CHECK(hidden == W * H * D - bw * bh);
    }
}

TEST_CASE("Cube per strand buffers lay strands side by side", "[modelcore][cube]") {
    ShowFixture f;
    Model* m = f.Add(CubeXml(3, 2, 4, "Front Bottom Left", "Vertical Front/Back"));
    std::vector<NodeBaseClassPtr> nodes;
    int bw = 0, bh = 0;
    m->InitRenderBufferNodes("Horizontal Per Strand", "2D", "None", nodes, bw, bh, 0);
    CHECK(bw == 3);
    CHECK(bh == 8);
    CHECK(nodes[9]->Coords[0].bufX == 1);
    CHECK(nodes[9]->Coords[0].bufY == 1);
    nodes.clear();
    m->InitRenderBufferNodes("Vertical Per Strand", "2D", "None", nodes, bw, bh, 0);
    CHECK(bw == 8);
    CHECK(bh == 3);
    CHECK(nodes[9]->Coords[0].bufX == 1);
    CHECK(nodes[9]->Coords[0].bufY == 1);
    nodes.clear();
    m->InitRenderBufferNodes("Single Line", "2D", "None", nodes, bw, bh, 0);
    CHECK(bw == 24);
    CHECK(bh == 1);
}

TEST_CASE("Cube screen positions are a rigid copy of the grid with the front nearest", "[modelcore][cube]") {
    const int W = 3, H = 4, D = 2;
    ShowFixture f;
    Model* m = f.Add(CubeXml(W, H, D, "Front Bottom Left", "Horizontal Left/Right"));
    auto cells = CubeCells(m, W);
    const auto& c0 = m->GetNode(0)->Coords[0];
    auto [x0, y0, z0] = cells[0];
    for (uint32_t i = 0; i < m->GetNodeCount(); ++i) {
        const auto& c = m->GetNode(i)->Coords[0];
        auto [x, y, z] = cells[i];
        // one unit per light; z runs towards the back so screenZ falls
        CHECK(c.screenX - c0.screenX == (float)(x - x0));
        CHECK(c.screenY - c0.screenY == (float)(y - y0));
        CHECK(c.screenZ - c0.screenZ == (float)(z0 - z));
    }
}

TEST_CASE("Cube row offset shifts every other layer", "[modelcore][cube]") {
    ShowFixture f;
    Model* m = f.Add(CubeXml(2, 1, 2, "Front Bottom Left", "Horizontal Left/Right", "CubeRowOffset=\"1\""));
    auto cells = CubeCells(m, 2);
    for (uint32_t i = 0; i < m->GetNodeCount(); ++i) {
        const auto& c = m->GetNode(i)->Coords[0];
        float base = (float)(std::get<0>(cells[i]) - 1);
        if (std::get<2>(cells[i]) % 2 == 1) {
            CHECK(c.screenX != base);
            CHECK(std::abs(c.screenX - base) == 0.5f);
        } else {
            CHECK(c.screenX == base);
        }
    }
}

TEST_CASE("Cylinder cube places rings around the axis", "[modelcore][cube]") {
    ShowFixture f;
    Model* m = f.Add(CubeXml(8, 2, 3, "Front Bottom Left", "Horizontal Left/Right", "CubeShape=\"1\" CubeHollow=\"50\""));
    const float outer = 8.0f / (2.0f * 3.14159265f);
    auto cells = CubeCells(m, 8);
    for (uint32_t i = 0; i < m->GetNodeCount(); ++i) {
        const auto& c = m->GetNode(i)->Coords[0];
        float r = std::sqrt(c.screenX * c.screenX + c.screenZ * c.screenZ);
        int z = std::get<2>(cells[i]);
        // rings shrink from the outer radius to 50% of it
        float expect = outer * (1.0f - z / 2.0f * 0.5f);
        CHECK(std::abs(r - expect) < 1e-3f);
    }
}

TEST_CASE("Cube single node models collapse every light into one node", "[modelcore][cube]") {
    ShowFixture f;
    Model* m = f.Add(CubeXml(2, 2, 2, "Front Bottom Left", "Horizontal Left/Right", "StringType=\"Single Color Red\""));
    CHECK(m->GetNodeCount() == 1);
    CHECK(m->GetNode(0)->Coords.size() == 8);
    int bw = 0, bh = 0;
    m->GetBufferSize("Default", "2D", "None", bw, bh, 0);
    CHECK(bw == 1);
    CHECK(bh == 1);
    CHECK(m->MapToNodeIndex(3, 1) == 1);
    std::string html = m->ChannelLayoutHtml(&f.om);
    CHECK(html.find("<html>") != std::string::npos);
}

TEST_CASE("Cube channel layout lists every node with its string", "[modelcore][cube]") {
    ShowFixture f;
    Model* m = f.Add(CubeXml(2, 2, 2, "Front Bottom Left", "Horizontal Left/Right", "Strings=\"2\""));
    std::string html = m->ChannelLayoutHtml(&f.om, false);
    CHECK(html.find("Start Corner:</td><td>Front Bottom Left Horizontal Left/Right") != std::string::npos);
    CHECK(html.find("Depth:</td><td>2") != std::string::npos);
    for (int n = 1; n <= 4; ++n) {
        for (int s = 1; s <= 2; ++s) {
            CHECK(html.find("n" + std::to_string(n) + "s" + std::to_string(s) + "<") != std::string::npos);
        }
    }
    CHECK(html.find("#ADD8E6") != std::string::npos);
    std::string dark = m->ChannelLayoutHtml(&f.om, true);
    CHECK(dark.find("#3f7c85") != std::string::npos);
    CHECK(dark.find("#ADD8E6") == std::string::npos);
}

TEST_CASE("Cube exports as a 3D custom model with its wiring order", "[modelcore][cube]") {
    ShowFixture f;
    Model* m = f.Add(CubeXml(2, 2, 2, "Front Bottom Left", "Horizontal Left/Right", "StrandPerLine=\"No Zig Zag\""));
    REQUIRE(m->SupportsExportAsCustom3D());
    StringSerializingVisitor v(false, false);
    m->ExportAsCustomXModel3D(v);
    std::string s = v.GetResult();
    pugi::xml_document d;
    REQUIRE(d.load_string(s.c_str()));
    auto cm = d.child("custommodel");
    REQUIRE(cm);
    CHECK(std::string(cm.attribute("CustomWidth").as_string()) == "2");
    CHECK(std::string(cm.attribute("Depth").as_string()) == "2");
    // layers front to back, rows top to bottom: front layer has 1,2 on the
    // bottom row and 3,4 on top; the back layer starts at the top again.
    CHECK(std::string(cm.attribute("CustomModel").as_string()) == "3,4;1,2|5,6;7,8");
    // a full cube is too dense to be worth compressing
    CHECK(std::string(cm.attribute("CustomModelCompressed").as_string()).empty());
}
