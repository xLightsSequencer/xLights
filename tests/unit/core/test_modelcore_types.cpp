#include <algorithm>
#include <fstream>
#include <set>
#include <string>
#include <vector>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <pugixml.hpp>

#include "models/CustomModel.h"
#include "models/ImageModel.h"
#include "models/MatrixModel.h"
#include "models/Model.h"
#include "models/ModelManager.h"
#include "models/Node.h"
#include "models/SpinnerModel.h"
#include "outputs/OutputManager.h"
#include "utils/xlImage.h"

#include "support/TempDir.h"
#include "test_modelcore_fakegfx.h"

using Catch::Approx;
using fakegfx::FakePreview;

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
    template <class T>
    T* As(const std::string& xml) {
        auto* t = dynamic_cast<T*>(Add(xml));
        REQUIRE(t != nullptr);
        return t;
    }
};

bool Has(const std::string& s, const std::string& what) {
    return s.find(what) != std::string::npos;
}

bool AnyHas(const std::list<std::string>& l, const std::string& what) {
    return std::any_of(l.begin(), l.end(), [&](const std::string& s) { return Has(s, what); });
}

// writes a w x h PNG filled with c
std::string WritePng(const TempDir& d, const std::string& name, int w, int h, xlColor c) {
    xlImage img(w, h);
    for (int x = 0; x < w; ++x) {
        for (int y = 0; y < h; ++y) {
            img.SetRGB(x, y, c.red, c.green, c.blue);
            img.SetAlpha(x, y, 255);
        }
    }
    std::vector<uint8_t> png;
    REQUIRE(img.SaveAsPNG(png));
    std::string path = d.File(name);
    std::ofstream(path, std::ios::binary).write((const char*)png.data(), png.size());
    return path;
}

using Cell = std::pair<int, int>;
std::vector<Cell> Cells(const Model* m) {
    std::vector<Cell> r;
    for (uint32_t i = 0; i < m->GetNodeCount(); ++i) {
        r.push_back({ m->GetNode(i)->Coords[0].bufX, m->GetNode(i)->Coords[0].bufY });
    }
    return r;
}

} // namespace

// ---------------------------------------------------------------------------
// MatrixModel
// ---------------------------------------------------------------------------

TEST_CASE("Matrix string count changes keep the pixel and strand totals", "[modelcore][matrix]") {
    ShowFixture f;
    auto* m = f.As<MatrixModel>(R"(<model name="M" DisplayAs="Matrix" StartChannel="1" NumStrings="2" NodesPerString="20" StrandsPerString="4"/>)");
    std::string msg;
    CHECK(m->ChangeStringCount(2, msg));
    CHECK(m->ChangeStringCount(4, msg));
    m->Setup();
    CHECK(m->GetNumStrings() == 4);
    CHECK(m->NodesPerString() == 10);
    CHECK(m->GetNodeCount() == 40);
    CHECK(m->GetNumStrands() == 8);

    CHECK_FALSE(m->ChangeStringCount(3, msg));
    CHECK(msg == "Pixel Count (40) is not divisible by 3");

    // pixels divide but strands do not
    auto* s = f.As<MatrixModel>(R"(<model name="S" DisplayAs="Matrix" StartChannel="1" NumStrings="2" NodesPerString="6" StrandsPerString="3"/>)");
    CHECK_FALSE(s->ChangeStringCount(4, msg));
    CHECK(msg == "Stand Count (6) is not divisible by 4");
}

TEST_CASE("Matrix settings check flags uneven strands", "[modelcore][matrix]") {
    ShowFixture f;
    Model* m = f.Add(R"(<model name="M" DisplayAs="Matrix" StartChannel="1" NumStrings="1" NodesPerString="7" StrandsPerString="2"/>)");
    CHECK(AnyHas(m->CheckModelSettings(), "strands are not equally sized 2 does not divide into string length 7"));
    Model* ok = f.Add(R"(<model name="O" DisplayAs="Matrix" StartChannel="1" NumStrings="1" NodesPerString="8" StrandsPerString="2"/>)");
    CHECK_FALSE(AnyHas(ok->CheckModelSettings(), "strands are not equally sized"));
}

TEST_CASE("Matrix alternate nodes skip every other cell", "[modelcore][matrix]") {
    ShowFixture f;
    // horizontal: one strand of 5, left to right then right to left
    Model* h = f.Add(R"(<model name="H" DisplayAs="Matrix" StartChannel="1" NumStrings="1" NodesPerString="5" StrandsPerString="1" AlternateNodes="true"/>)");
    std::vector<int> xs;
    for (auto& c : Cells(h)) xs.push_back(c.first);
    CHECK(xs == std::vector<int>{ 0, 2, 4, 3, 1 });
    Model* hr = f.Add(R"(<model name="HR" DisplayAs="Matrix" StartChannel="1" NumStrings="1" NodesPerString="5" StrandsPerString="1" AlternateNodes="true" Dir="R"/>)");
    xs.clear();
    for (auto& c : Cells(hr)) xs.push_back(c.first);
    CHECK(xs == std::vector<int>{ 4, 2, 0, 1, 3 });

    // vertical: bottom to top, or top to bottom
    Model* v = f.Add(R"(<model name="V" DisplayAs="Matrix" Vertical="true" StartChannel="1" NumStrings="1" NodesPerString="4" StrandsPerString="1" AlternateNodes="true"/>)");
    std::vector<int> ys;
    for (auto& c : Cells(v)) ys.push_back(c.second);
    CHECK(ys == std::vector<int>{ 0, 2, 3, 1 });
    Model* vt = f.Add(R"(<model name="VT" DisplayAs="Matrix" Vertical="true" StartChannel="1" NumStrings="1" NodesPerString="4" StrandsPerString="1" AlternateNodes="true" StartSide="T"/>)");
    ys.clear();
    for (auto& c : Cells(vt)) ys.push_back(c.second);
    CHECK(ys == std::vector<int>{ 3, 1, 0, 2 });
}

TEST_CASE("Matrix no zig zag runs every strand the same way", "[modelcore][matrix]") {
    ShowFixture f;
    Model* h = f.Add(R"(<model name="H" DisplayAs="Matrix" StartChannel="1" NumStrings="1" NodesPerString="6" StrandsPerString="2" NoZig="true" Dir="R"/>)");
    CHECK(Cells(h) == std::vector<Cell>{ { 2, 0 }, { 1, 0 }, { 0, 0 }, { 2, 1 }, { 1, 1 }, { 0, 1 } });
    Model* v = f.Add(R"(<model name="V" DisplayAs="Matrix" Vertical="true" StartChannel="1" NumStrings="1" NodesPerString="6" StrandsPerString="2" NoZig="true" StartSide="T"/>)");
    CHECK(Cells(v) == std::vector<Cell>{ { 0, 2 }, { 0, 1 }, { 0, 0 }, { 1, 2 }, { 1, 1 }, { 1, 0 } });
}

TEST_CASE("Single colour matrix strings are one node per string", "[modelcore][matrix]") {
    ShowFixture f;
    for (const char* vert : { "false", "true" }) {
        INFO("vertical " << vert);
        Model* m = f.Add(std::string("<model name=\"S") + vert + "\" DisplayAs=\"Matrix\" Vertical=\"" + vert + "\" StartChannel=\"1\" NumStrings=\"3\" NodesPerString=\"4\" StrandsPerString=\"2\" StringType=\"Single Color Red\"/>");
        REQUIRE(m->GetNodeCount() == 3);
        CHECK(m->GetNumStrands() == 3);
        CHECK(m->NodesPerString() == 1);
        for (uint32_t i = 0; i < 3; ++i) {
            CHECK(m->GetNode(i)->Coords.size() == 4);
            CHECK(m->NodeStartChannel(i) == (int32_t)i);
        }
        // one buffer cell per string
        std::set<Cell> cells;
        for (uint32_t i = 0; i < 3; ++i) {
            for (auto& c : m->GetNode(i)->Coords) cells.insert({ c.bufX, c.bufY });
        }
        CHECK(cells.size() == 3);
        // the lights of a string are spread over its strands on screen
        std::set<std::pair<float, float>> screen;
        for (uint32_t i = 0; i < 3; ++i) {
            for (auto& c : m->GetNode(i)->Coords) screen.insert({ c.screenX, c.screenY });
        }
        CHECK(screen.size() == 12);
    }
}

// ---------------------------------------------------------------------------
// SpinnerModel
// ---------------------------------------------------------------------------

TEST_CASE("Spinner start locations encode and decode", "[modelcore][spinner]") {
    ShowFixture f;
    auto* s = f.As<SpinnerModel>(R"(<model name="S" DisplayAs="Spinner" StartChannel="1" NumStrings="1" NodesPerArm="3" ArmsPerString="4"/>)");
    for (int sl = 0; sl < 6; ++sl) {
        INFO(sl);
        s->DecodeStartLocation(sl);
        CHECK(s->EncodeStartLocation() == sl);
    }
}

TEST_CASE("Spinner arms radiate from the centre", "[modelcore][spinner]") {
    ShowFixture f;
    auto* s = f.As<SpinnerModel>(R"(<model name="S" DisplayAs="Spinner" StartChannel="1" NumStrings="1" NodesPerArm="3" ArmsPerString="4" Hollow="0"/>)");
    REQUIRE(s->GetNodeCount() == 12);
    CHECK(s->GetNumStrands() == 4);
    CHECK(s->GetStrandLength(0) == 3);
    CHECK(static_cast<Model*>(s)->MapToNodeIndex(2, 1) == 7);
    CHECK(s->NodesPerString() == 12);
    // every arm's nodes get further from the centre (or nearer) one step at a time
    for (int a = 0; a < 4; ++a) {
        std::vector<float> r;
        for (int n = 0; n < 3; ++n) {
            const auto& c = s->GetNode(a * 3 + n)->Coords[0];
            r.push_back(std::sqrt(c.screenX * c.screenX + c.screenY * c.screenY));
        }
        CHECK(std::abs(r[2] - r[0]) == Approx(2.0f).margin(1e-3));
    }

    // single line buffer lays every node in a row
    std::vector<NodeBaseClassPtr> nodes;
    int bw = 0, bh = 0;
    s->InitRenderBufferNodes("Single Line", "2D", "None", nodes, bw, bh, 0);
    CHECK(bw == 12);
    CHECK(bh == 1);
    for (int i = 0; i < 12; ++i) {
        CHECK(nodes[i]->Coords[0].bufX == i);
    }
    int gw = 0, gh = 0;
    s->GetBufferSize("Single Line", "2D", "None", gw, gh, 0);
    CHECK(gw == 12);
    CHECK(gh == 1);
    nodes.clear();
    s->InitRenderBufferNodes("Per Model Default", "2D", "None", nodes, bw, bh, 0);
    CHECK(bw == 4);
    CHECK(bh == 3);
}

TEST_CASE("Spinner hollow and arc move the nodes", "[modelcore][spinner]") {
    ShowFixture f;
    auto* full = f.As<SpinnerModel>(R"(<model name="A" DisplayAs="Spinner" StartChannel="1" NumStrings="1" NodesPerArm="2" ArmsPerString="4" Hollow="0"/>)");
    auto* hollow = f.As<SpinnerModel>(R"(<model name="B" DisplayAs="Spinner" StartChannel="1" NumStrings="1" NodesPerArm="2" ArmsPerString="4" Hollow="50"/>)");
    auto rad = [](Model* m, int n) {
        const auto& c = m->GetNode(n)->Coords[0];
        return std::sqrt(c.screenX * c.screenX + c.screenY * c.screenY);
    };
    float inner = std::min(rad(full, 0), rad(full, 1));
    float innerHollow = std::min(rad(hollow, 0), rad(hollow, 1));
    // the hollow pushes the first light out by hollow% * 2 * nodes per arm
    CHECK(innerHollow - inner == Approx(2.0f).margin(1e-3));

    // a half circle arc with the arms at both ends
    auto* half = f.As<SpinnerModel>(R"(<model name="C" DisplayAs="Spinner" StartChannel="1" NumStrings="1" NodesPerArm="1" ArmsPerString="3" Hollow="0" Arc="180"/>)");
    const auto& a0 = half->GetNode(0)->Coords[0];
    const auto& a2 = half->GetNode(2)->Coords[0];
    CHECK(a0.screenX == Approx(-a2.screenX).margin(1e-3));
    CHECK(a0.screenY == Approx(-a2.screenY).margin(1e-3));
}

TEST_CASE("Spinner zig zag and alternate wiring", "[modelcore][spinner]") {
    ShowFixture f;
    auto rad = [](Model* m, int n) {
        const auto& c = m->GetNode(n)->Coords[0];
        return std::sqrt(c.screenX * c.screenX + c.screenY * c.screenY);
    };
    // zig zag: the second arm runs back the other way; without it both run alike
    auto* z = f.As<SpinnerModel>(R"(<model name="Z" DisplayAs="Spinner" StartChannel="1" NumStrings="1" NodesPerArm="3" ArmsPerString="2" Hollow="0" ZigZag="true"/>)");
    CHECK(rad(z, 2) - rad(z, 0) == Approx(-(rad(z, 5) - rad(z, 3))).margin(1e-3));
    CHECK(std::abs(rad(z, 2) - rad(z, 0)) == Approx(2.0f).margin(1e-3));
    auto* nz = f.As<SpinnerModel>(R"(<model name="NZ" DisplayAs="Spinner" StartChannel="1" NumStrings="1" NodesPerArm="3" ArmsPerString="2" Hollow="0"/>)");
    CHECK(rad(nz, 2) - rad(nz, 0) == Approx(rad(nz, 5) - rad(nz, 3)).margin(1e-3));
    // alternate: down one arm and back up the next, every other node
    auto* alt = f.As<SpinnerModel>(R"(<model name="A" DisplayAs="Spinner" StartChannel="1" NumStrings="1" NodesPerArm="3" ArmsPerString="2" Hollow="0" Alternate="true"/>)");
    std::set<std::pair<int, int>> cells;
    for (uint32_t i = 0; i < alt->GetNodeCount(); ++i) {
        cells.insert({ alt->GetNode(i)->Coords[0].bufX, alt->GetNode(i)->Coords[0].bufY });
    }
    CHECK(cells.size() == 6);
}

TEST_CASE("Single colour spinner strings are one node each", "[modelcore][spinner]") {
    ShowFixture f;
    auto* s = f.As<SpinnerModel>(R"(<model name="S" DisplayAs="Spinner" StartChannel="1" NumStrings="2" NodesPerArm="3" ArmsPerString="2" StringType="Single Color Red"/>)");
    REQUIRE(s->GetNodeCount() == 2);
    CHECK(s->GetNode(0)->Coords.size() == 6);
    CHECK(s->NodesPerString() == 1);
    CHECK(static_cast<Model*>(s)->MapToNodeIndex(1, 2) == 1);
    CHECK(s->NodeStartChannel(1) == 1);
    std::vector<NodeBaseClassPtr> nodes;
    int bw = 0, bh = 0;
    s->InitRenderBufferNodes("Single Line", "2D", "None", nodes, bw, bh, 0);
    CHECK(bw == 2);
    REQUIRE(nodes.size() == 2);
    CHECK(nodes[1]->Coords[0].bufX == 1);
}

// ---------------------------------------------------------------------------
// ImageModel
// ---------------------------------------------------------------------------

TEST_CASE("Image model is a single node one cell buffer", "[modelcore][image]") {
    ShowFixture f;
    auto* im = f.As<ImageModel>(R"(<model name="I" DisplayAs="Image" StartChannel="5" Image=""/>)");
    REQUIRE(im->GetNodeCount() == 1);
    CHECK(im->NodeStartChannel(0) == 4);
    int bw = 0, bh = 0;
    im->GetBufferSize("Default", "2D", "None", bw, bh, 0);
    CHECK(bw == 1);
    CHECK(bh == 1);
    std::vector<NodeBaseClassPtr> nodes;
    im->InitRenderBufferNodes("Default", "2D", "None", nodes, bw, bh, 0);
    REQUIRE(nodes.size() == 1);
    CHECK(nodes[0]->Coords.size() == 1);
    CHECK(nodes[0]->Coords[0].bufX == 0);

}

TEST_CASE("Image model checks its image file", "[modelcore][image]") {
    TempDir d;
    ShowFixture f;
    auto* missing = f.As<ImageModel>(R"(<model name="I" DisplayAs="Image" StartChannel="1" Image=""/>)");
    CHECK(AnyHas(missing->CheckModelSettings(), "ERR: Image model 'I' cant find image file"));
    CHECK(missing->GetFileReferences().empty());

    std::string junk = d.Write("junk.png", "not an image");
    auto* bad = f.As<ImageModel>("<model name=\"J\" DisplayAs=\"Image\" StartChannel=\"1\" Image=\"" + junk + "\"/>");
    CHECK(AnyHas(bad->CheckModelSettings(), "cant load image file"));

    std::string png = WritePng(d, "pic.png", 4, 2, xlRED);
    auto* good = f.As<ImageModel>("<model name=\"G\" DisplayAs=\"Image\" StartChannel=\"1\" Image=\"" + png + "\"/>");
    auto res = good->CheckModelSettings();
    CHECK_FALSE(AnyHas(res, "ERR"));
    CHECK(good->GetFileReferences() == std::list<std::string>{ png });
}

TEST_CASE("Image model draws its picture in the layout", "[modelcore][image]") {
    TempDir d;
    std::string png = WritePng(d, "pic.png", 4, 2, xlColor(200, 100, 50));
    ShowFixture f;
    auto* im = f.As<ImageModel>("<model name=\"I\" DisplayAs=\"Image\" StartChannel=\"1\" Image=\"" + png + "\" WhiteAsAlpha=\"True\"/>");
    im->SetNodeColor(0, xlWHITE);
    FakePreview p(1000, 1000, false);
    xlColor c = xlWHITE;
    im->Selected(true);
    im->DisplayModelOnWindow(&p, &p.ctx, p.solid.get(), p.transparent.get(), false, &c, true);
    p.Run();
    // picture only (selected layout view does not colour it), plus handles
    CHECK(p.ctx.textures == 1);
    CHECK(p.TransparentVerts().v.size() > 0);

    // the house preview colours the image with the node colour
    FakePreview h(1000, 1000, true);
    im->DisplayModelOnWindow(&h, &h.ctx, h.solid.get(), h.transparent.get(), true);
    h.Run();
    CHECK(h.ctx.textures == 2);

    // without an image a red cross marks the spot
    auto* none = f.As<ImageModel>(R"(<model name="N" DisplayAs="Image" StartChannel="1" Image=""/>)");
    FakePreview q(1000, 1000, false);
    none->DisplayModelOnWindow(&q, &q.ctx, q.solid.get(), q.transparent.get(), false);
    CHECK(q.SolidVerts().v.size() == 12);
    CHECK(q.SolidVerts().v[0].c == xlRED);
    q.Run();
    CHECK(q.ctx.lines == 1);
}

TEST_CASE("Image model effect preview fits the picture to the window", "[modelcore][image]") {
    TempDir d;
    std::string png = WritePng(d, "pic.png", 4, 2, xlColor(200, 100, 50));
    ShowFixture f;
    auto* im = f.As<ImageModel>("<model name=\"I\" DisplayAs=\"Image\" StartChannel=\"1\" Image=\"" + png + "\"/>");
    FakePreview p(400, 400, false);
    im->DisplayEffectOnWindow(&p, 2.0);
    p.Run();
    // image plus its colour overlay
    CHECK(p.ctx.textures == 2);

    auto* none = f.As<ImageModel>(R"(<model name="N" DisplayAs="Image" StartChannel="1" Image=""/>)");
    FakePreview q(400, 400, false);
    none->DisplayEffectOnWindow(&q, 2.0);
    CHECK(q.SolidVerts().v.size() == 12);
    q.Run();
    CHECK(q.ctx.lines == 1);
}

namespace {
struct CountingContext : fakegfx::FakeContext {
    int created = 0;
    xlTexture* createTexture(const xlImage& i, const std::string& n, bool f) override {
        ++created;
        return FakeContext::createTexture(i, n, f);
    }
};
} // namespace

TEST_CASE("Image model effect preview reuses its textures", "[modelcore]") {
    TempDir d;
    std::string png = WritePng(d, "pic.png", 4, 2, xlColor(200, 100, 50));
    ShowFixture f;
    auto* im = f.As<ImageModel>("<model name=\"I\" DisplayAs=\"Image\" StartChannel=\"1\" Image=\"" + png + "\"/>");
    CountingContext ctx;
    struct P : FakePreview {
        xlGraphicsContext* c;
        xlGraphicsContext* getCurrentGraphicsContext() override { return c; }
    } pv;
    pv.c = &ctx;
    im->DisplayEffectOnWindow(&pv, 2.0);
    int first = ctx.created;
    im->DisplayEffectOnWindow(&pv, 2.0);
    CHECK(first == 2);
    CHECK(ctx.created == first);
}

// ---------------------------------------------------------------------------
// CustomModel
// ---------------------------------------------------------------------------

TEST_CASE("Custom model compressed form round trips sparse models", "[modelcore][custom]") {
    std::string sparse = "1,,,;,,,2;,3,,";
    std::string c = CustomModel::CustomModelToCompressed(sparse);
    CHECK(c == "1,0,0;2,1,3;3,2,1");
    CHECK(CustomModel::CompressedToCustomModel(c) == sparse);
    // 3D models carry the layer
    std::string layered = "1,,;,,|,,;,2,";
    std::string lc = CustomModel::CustomModelToCompressed(layered);
    CHECK(lc == "1,0,0,0;2,1,1,1");
    // the compressed form has no size, so trailing empty columns are lost
    CHECK(CustomModel::CompressedToCustomModel(lc) == "1,;,|,;,2");
    // dense models are not worth compressing
    CHECK(CustomModel::CustomModelToCompressed("1,2;3,4").empty());
}

TEST_CASE("Custom model settings check finds missing and duplicate nodes", "[modelcore][custom]") {
    ShowFixture f;
    Model* gaps = f.Add(R"(<model name="G" DisplayAs="Custom" StartChannel="1" CustomWidth="7" CustomHeight="1" CustomModel="1,,3,,,6,7"/>)");
    auto res = gaps->CheckModelSettings();
    CHECK(AnyHas(res, "WARN: Custom model 'G' missing node 2."));
    CHECK(AnyHas(res, "WARN: Custom model 'G' missing nodes 4-5."));

    // one node in a model of many appearing twice looks like a mistake
    std::string row;
    for (int i = 1; i <= 12; ++i) row += std::to_string(i) + ",";
    row += "5";
    Model* dup = f.Add("<model name=\"D\" DisplayAs=\"Custom\" StartChannel=\"1\" CustomWidth=\"13\" CustomHeight=\"1\" CustomModel=\"" + row + "\"/>");
    CHECK(AnyHas(dup->CheckModelSettings(), "5th node has 2 instances"));
    CHECK_FALSE(dynamic_cast<CustomModel*>(dup)->IsAllNodesUnique());
    CHECK(dynamic_cast<CustomModel*>(gaps)->IsAllNodesUnique());

    Model* big = f.Add(R"(<model name="B" DisplayAs="Custom" StartChannel="1" CustomWidth="400" CustomHeight="2" CustomModelCompressed="1,0,0;2,1,399"/>)");
    CHECK(AnyHas(big->CheckModelSettings(), "dimensions are really large (400 x 2 x 1"));

    Model* empty = f.Add(R"(<model name="E" DisplayAs="Custom" StartChannel="1" CustomWidth="2" CustomHeight="1" CustomModel=","/>)");
    CHECK(AnyHas(empty->CheckModelSettings(), "has no nodes defined"));
    CHECK_FALSE(dynamic_cast<CustomModel*>(empty)->IsAllNodesUnique());
}

TEST_CASE("Custom model string start nodes are checked", "[modelcore][custom]") {
    ShowFixture f;
    Model* m = f.Add(R"(<model name="C" DisplayAs="Custom" StartChannel="1" CustomWidth="4" CustomHeight="1" CustomModel="1,2,3,4" CustomStrings="3" NodeStart1="2" NodeStart2="2" NodeStart3="9"/>)");
    auto res = m->CheckModelSettings();
    CHECK(AnyHas(res, "String 1 starts at a node 2 which has already been used by another string."));
    CHECK(AnyHas(res, "String 3 starts at a node 9 outside the node count 4 in the model."));
    CHECK(AnyHas(res, "Multiple strings but none starting at node 1."));

    Model* ok = f.Add(R"(<model name="O" DisplayAs="Custom" StartChannel="1" CustomWidth="4" CustomHeight="1" CustomModel="1,2,3,4" CustomStrings="2" NodeStart1="1" NodeStart2="4"/>)");
    CHECK_FALSE(AnyHas(ok->CheckModelSettings(), "ERR"));
    // strings run to the next string's start
    CHECK(ok->NodesPerString(0) == 3);
    CHECK(ok->NodesPerString(1) == 1);
    CHECK(ok->GetNumPhysicalStrings() == 2);
    // the old String<n> attribute names are read as start nodes too
    Model* old = f.Add(R"(<model name="L" DisplayAs="Custom" StartChannel="1" CustomWidth="4" CustomHeight="1" CustomModel="1,2,3,4" CustomStrings="2" String1="1" String2="3"/>)");
    CHECK(old->NodesPerString(0) == 2);
    CHECK(old->NodeStartChannel(2) == 6);
}

TEST_CASE("Custom model string count changes recompute start nodes", "[modelcore][custom]") {
    ShowFixture f;
    auto* c = f.As<CustomModel>(R"(<model name="C" DisplayAs="Custom" StartChannel="1" CustomWidth="6" CustomHeight="1" CustomModel="1,2,3,4,5,6"/>)");
    std::string msg;
    CHECK(c->ChangeStringCount(1, msg));
    CHECK(c->ChangeStringCount(3, msg));
    c->Setup();
    CHECK(c->GetNumStrings() == 3);
    CHECK(c->HasIndivStartNodes());
    CHECK(c->GetNodeSizes() == std::vector<int>{ 1, 3, 5 });
    CHECK(c->NodesPerString(1) == 2);
}

TEST_CASE("Custom model channel layout", "[modelcore][custom]") {
    ShowFixture f;
    Model* one = f.Add(R"(<model name="C" DisplayAs="Custom" StartChannel="1" CustomWidth="3" CustomHeight="2" CustomModel="1,,2;,3,"/>)");
    std::string html = one->ChannelLayoutHtml(&f.om, false);
    CHECK(Has(html, "Width:</td><td>3<"));
    CHECK(Has(html, "<td bgcolor='#ADD8E6'>n1</td>"));
    CHECK(Has(html, "<td bgcolor='#ADD8E6'>n3</td>"));
    CHECK(Has(one->ChannelLayoutHtml(&f.om, true), "<td bgcolor='#962B09'>n2</td>"));
    one->SetControllerProtocol("ws2811");
    one->SetControllerPort(4);
    CHECK(Has(one->ChannelLayoutHtml(&f.om), "Controller Connection:</td><td>4<"));

    Model* two = f.Add(R"(<model name="T" DisplayAs="Custom" StartChannel="1" CustomWidth="4" CustomHeight="1" CustomModel="1,2,3,4" CustomStrings="2" NodeStart1="1" NodeStart2="3"/>)");
    two->SetControllerProtocol("ws2811");
    two->SetControllerPort(1);
    std::string h2 = two->ChannelLayoutHtml(&f.om);
    CHECK(Has(h2, ">n1s1<"));
    CHECK(Has(h2, ">n3s2<"));
    CHECK(Has(h2, "Controller Connection:</td><td>1-2<"));
    CHECK(Has(two->ChannelLayoutHtml(&f.om, true), "#3f7c85"));

    // a 3D model lays its layers side by side
    Model* deep = f.Add(R"(<model name="D" DisplayAs="Custom" StartChannel="1" CustomWidth="2" CustomHeight="1" Depth="2" CustomModel="1,|,2"/>)");
    std::string h3 = deep->ChannelLayoutHtml(&f.om);
    CHECK(h3.find(">n1<") < h3.find(">n2<"));
}

TEST_CASE("Custom model imports a LOR model file", "[modelcore][custom]") {
    TempDir d;
    std::string lor = d.Write("Arch Lights.lcb", R"(<?xml version="1.0"?>
<Model>
  <DrawObjects>
    <DrawObject><DrawPoints><DrawPoint X="0" Y="0"/></DrawPoints></DrawObject>
    <DrawObject><DrawPoints><DrawPoint X="50" Y="0"/></DrawPoints></DrawObject>
    <DrawObject><DrawPoints><DrawPoint X="100" Y="0"/><DrawPoint X="100" Y="50"/></DrawPoints></DrawObject>
  </DrawObjects>
</Model>)");
    ShowFixture f;
    auto* c = dynamic_cast<CustomModel*>(f.mm.CreateDefaultModel("Custom", "1"));
    REQUIRE(c != nullptr);
    float minx, maxx, miny, maxy;
    REQUIRE(c->ImportLORModel(lor, minx, maxx, miny, maxy));
    CHECK(c->GetName() == "Arch Lights");
    CHECK(c->GetCustomWidth() == 3);
    CHECK(c->GetCustomHeight() == 2);
    // each draw point list is one channel; the last one lights two cells
    CHECK(c->GetCustomData() == "1,2,3;,,3");

    TempDir e;
    std::string empty = e.Write("empty.lcb", "<Model><DrawObjects/></Model>");
    CHECK_FALSE(c->ImportLORModel(empty, minx, maxx, miny, maxy));
    CHECK_FALSE(c->ImportLORModel(e.File("missing.lcb"), minx, maxx, miny, maxy));
    delete c;
}

TEST_CASE("Custom model background image is drawn behind the nodes", "[modelcore][custom]") {
    TempDir d;
    std::string png = WritePng(d, "bkg.png", 8, 8, xlColor(50, 60, 70));
    ShowFixture f;
    auto* c = f.As<CustomModel>("<model name=\"C\" DisplayAs=\"Custom\" StartChannel=\"1\" CustomWidth=\"3\" CustomHeight=\"2\" CustomModel=\"1,,2;,3,\" CustomBkgImage=\"" + png + "\" CustomBkgTransparency=\"20\"/>");
    CHECK(c->GetFileReferences() == std::list<std::string>{ png });
    FakePreview p(1000, 1000, false);
    c->DisplayModelOnWindow(&p, &p.ctx, p.solid.get(), p.transparent.get(), false);
    p.Run();
    CHECK(p.ctx.textures == 1);
    CHECK(p.ctx.points == 1);

    FakePreview e(400, 300, false);
    c->DisplayEffectOnWindow(&e, 2.0);
    e.Run();
    CHECK(e.ctx.textures == 1);
    CHECK(e.ctx.points == 1);

    // a new background drops the cached texture
    c->SetCustomBackground("");
    CHECK(c->GetFileReferences().empty());
    FakePreview p2(1000, 1000, false);
    c->DisplayModelOnWindow(&p2, &p2.ctx, p2.solid.get(), p2.transparent.get(), false);
    p2.Run();
    CHECK(p2.ctx.textures == 0);
}
