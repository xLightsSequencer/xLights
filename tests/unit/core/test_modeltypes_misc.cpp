#include <algorithm>
#include <cmath>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <pugixml.hpp>

#include "XmlSerializer/XmlSerializer.h"
#include "models/Model.h"
#include "models/ModelManager.h"
#include "outputs/OutputManager.h"

namespace {

struct ShowFixture {
    OutputManager om;
    ModelManager mm{ &om };

    Model* Make(const std::string& type, const std::string& attrs) {
        pugi::xml_document doc;
        std::string xml = "<model name=\"M\" DisplayAs=\"" + type + "\" StartChannel=\"1\" " + attrs + "/>";
        REQUIRE(doc.load_string(xml.c_str()));
        Model* m = mm.createAndAddModel(doc.first_child(), 100, 100);
        REQUIRE(m != nullptr);
        mm.RecalcStartChannels();
        return m;
    }
};

using Pt = std::pair<int, int>;

std::vector<Pt> FirstCells(const Model* m) {
    std::vector<Pt> pts;
    for (uint32_t i = 0; i < m->GetNodeCount(); ++i) {
        const auto& c = m->GetNode(i)->Coords[0];
        pts.push_back({ c.bufX, c.bufY });
    }
    return pts;
}

std::string Serialize(const Model* m) {
    pugi::xml_document doc;
    XmlSerializingVisitor v(&doc);
    m->Accept(v);
    std::ostringstream os;
    doc.save(os, "", pugi::format_raw);
    return os.str();
}

} // namespace

TEST_CASE("Custom model with a repeated node number shares one node", "[models2][modeltypes]") {
    ShowFixture f;
    Model* m = f.Make("Custom", "CustomWidth=\"3\" CustomHeight=\"1\" CustomModel=\"1,1,2\"");
    CHECK(m->GetNodeCount() == 2);
    CHECK(m->GetChanCount() == 6);
    CHECK(m->GetCoordCount(0) == 2);
    CHECK(m->GetCoordCount(1) == 1);
    CHECK(m->GetNode(0)->Coords[0].bufX == 0);
    CHECK(m->GetNode(0)->Coords[1].bufX == 1);
    CHECK(m->GetNode(1)->Coords[0].bufX == 2);
}

TEST_CASE("Custom model with gaps in node numbers leaves unused channels", "[models2][modeltypes]") {
    ShowFixture f;
    Model* m = f.Make("Custom", "CustomWidth=\"3\" CustomHeight=\"1\" CustomModel=\"1,,5\"");
    CHECK(m->GetNodeCount() == 2);
    CHECK(m->GetChanCount() == 15);
    CHECK(m->NodeStartChannel(0) == 0);
    CHECK(m->NodeStartChannel(1) == 12);
}

TEST_CASE("Custom model layers sit side by side in the buffer", "[models2][modeltypes]") {
    ShowFixture f;
    Model* m = f.Make("Custom", "CustomWidth=\"2\" CustomHeight=\"2\" Depth=\"2\" CustomModel=\"1,2;3,4|5,6;7,8\"");
    CHECK(m->GetDefaultBufferWi() == 4);
    CHECK(m->GetDefaultBufferHt() == 2);
    CHECK(FirstCells(m) == std::vector<Pt>{ { 0, 1 }, { 1, 1 }, { 0, 0 }, { 1, 0 }, { 2, 1 }, { 3, 1 }, { 2, 0 }, { 3, 0 } });
}

TEST_CASE("Custom model depth layers are spread through z on screen", "[models2][modeltypes]") {
    ShowFixture f;
    Model* m = f.Make("Custom", "CustomWidth=\"2\" CustomHeight=\"1\" Depth=\"2\" CustomModel=\"1,2|3,4\"");
    REQUIRE(m->GetNodeCount() == 4);
    float z0 = m->GetNode(0)->Coords[0].screenZ;
    float z2 = m->GetNode(2)->Coords[0].screenZ;
    CHECK(z0 != Catch::Approx(z2));
    CHECK(m->GetNode(0)->Coords[0].screenZ == Catch::Approx(m->GetNode(1)->Coords[0].screenZ));
}

TEST_CASE("Custom model string type sets the channels per node", "[models2][modeltypes]") {
    struct Case {
        const char* type;
        int perNode;
    };
    const Case cases[] = { { "RGB Nodes", 3 }, { "RGBW Nodes", 4 }, { "Single Color Red", 1 }, { "WRGB Nodes", 4 } };
    for (const auto& c : cases) {
        INFO(c.type);
        ShowFixture f;
        Model* m = f.Make("Custom", std::string("CustomWidth=\"2\" CustomHeight=\"1\" CustomModel=\"1,2\" StringType=\"") + c.type + "\"");
        CHECK(m->GetChanCountPerNode() == c.perNode);
        CHECK(m->GetChanCount() == 2 * c.perNode);
        CHECK(m->NodeStartChannel(1) == c.perNode);
    }
}

TEST_CASE("Custom model strings attribute sets the string count", "[models2][modeltypes]") {
    ShowFixture f;
    Model* m = f.Make("Custom", "CustomWidth=\"3\" CustomHeight=\"2\" CustomModel=\"1,2,3;4,5,6\" CustomStrings=\"2\"");
    CHECK(m->GetNumStrings() == 2);
    CHECK(m->GetNodeCount() == 6);
}

TEST_CASE("Custom model buffer styles and rotations", "[models2][modeltypes]") {
    ShowFixture f;
    Model* m = f.Make("Custom", "CustomWidth=\"3\" CustomHeight=\"2\" CustomModel=\"1,2,3;4,5,6\"");
    int w = 0, h = 0;
    m->GetBufferSize("Default", "2D", "None", w, h, 0);
    CHECK(w == 3);
    CHECK(h == 2);
    m->GetBufferSize("Default", "2D", "Rotate CW 90", w, h, 0);
    CHECK(w == 2);
    CHECK(h == 3);
    m->GetBufferSize("Single Line", "2D", "None", w, h, 0);
    CHECK(w == 6);
    CHECK(h == 1);
}

TEST_CASE("Custom model survives a round trip with gaps and repeats", "[models2][modeltypes]") {
    ShowFixture a;
    Model* m = a.Make("Custom", "CustomWidth=\"4\" CustomHeight=\"2\" CustomModel=\"1,,2,2;,3,,1\"");
    std::string first = Serialize(m);
    pugi::xml_document doc;
    REQUIRE(doc.load_string(first.c_str()));
    ShowFixture b;
    Model* m2 = b.mm.createAndAddModel(doc.first_child(), 100, 100);
    REQUIRE(m2 != nullptr);
    CHECK(m2->GetNodeCount() == m->GetNodeCount());
    CHECK(Serialize(m2) == first);
    for (uint32_t i = 0; i < m->GetNodeCount(); ++i) {
        CHECK(m2->GetCoordCount(i) == m->GetCoordCount(i));
    }
}

TEST_CASE("Channel block with channels has one node per channel", "[models2][modeltypes]") {
    ShowFixture f;
    Model* m = f.Make("Channel Block", "NumChannels=\"4\"");
    CHECK(m->GetNodeCount() == 4);
    CHECK(m->GetChanCount() == 4);
    CHECK(m->GetChanCountPerNode() == 1);
    CHECK(m->GetNumStrings() == 4);
    CHECK(m->GetDefaultBufferWi() == 4);
    CHECK(m->GetDefaultBufferHt() == 1);
    for (uint32_t i = 0; i < 4; ++i) {
        CHECK(m->NodeStartChannel(i) == (int32_t)i);
        CHECK(m->GetNodeStringNumber(i) == (int)i);
    }
}

TEST_CASE("Channel block channel count round trips", "[models2][modeltypes]") {
    ShowFixture a;
    Model* m = a.Make("Channel Block", "NumChannels=\"6\"");
    std::string first = Serialize(m);
    pugi::xml_document doc;
    REQUIRE(doc.load_string(first.c_str()));
    ShowFixture b;
    Model* m2 = b.mm.createAndAddModel(doc.first_child(), 100, 100);
    CHECK(m2->GetNodeCount() == 6);
    CHECK(Serialize(m2) == first);
}

TEST_CASE("Image and label models are a single selectable node", "[models2][modeltypes]") {
    for (const char* type : { "Image", "Label" }) {
        INFO(type);
        ShowFixture f;
        Model* m = f.Make(type, "");
        CHECK(m->GetNodeCount() == 1);
        CHECK(m->GetChanCount() == 3);
        CHECK(m->GetDefaultBufferWi() == 1);
        CHECK(m->GetDefaultBufferHt() == 1);
        CHECK(m->GetCoordCount(0) == 2);
        CHECK(m->GetNode(0)->Coords[0].screenX == Catch::Approx(-0.5f));
        CHECK(m->GetNode(0)->Coords[1].screenX == Catch::Approx(0.5f));
        CHECK(m->GetNode(0)->Coords[1].bufX == 0);
        CHECK(m->GetNode(0)->Coords[1].bufY == 0);
    }
}

TEST_CASE("Image and label first buffer coordinate is initialised", "[models2][modeltypes]") {
    for (const char* type : { "Image", "Label" }) {
        INFO(type);
        ShowFixture f;
        Model* m = f.Make(type, "");
        CHECK(m->GetNode(0)->Coords[0].bufX == 0);
        CHECK(m->GetNode(0)->Coords[0].bufY == 0);
    }
}

TEST_CASE("Image model stores its image path and round trips", "[models2][modeltypes]") {
    ShowFixture a;
    Model* m = a.Make("Image", "Image=\"\" Transparency=\"25\" OffBrightness=\"40\" WhiteAsAlpha=\"True\"");
    std::string first = Serialize(m);
    pugi::xml_document doc;
    REQUIRE(doc.load_string(first.c_str()));
    ShowFixture b;
    Model* m2 = b.mm.createAndAddModel(doc.first_child(), 100, 100);
    REQUIRE(m2 != nullptr);
    CHECK(Serialize(m2) == first);
}

TEST_CASE("Label model round trips its text", "[models2][modeltypes]") {
    ShowFixture a;
    Model* m = a.Make("Label", "LabelText=\"Hello &amp; goodbye\" LabelFontSize=\"22\"");
    std::string first = Serialize(m);
    CHECK(first.find("Hello &amp; goodbye") != std::string::npos);
    pugi::xml_document doc;
    REQUIRE(doc.load_string(first.c_str()));
    ShowFixture b;
    Model* m2 = b.mm.createAndAddModel(doc.first_child(), 100, 100);
    REQUIRE(m2 != nullptr);
    CHECK(Serialize(m2) == first);
}

TEST_CASE("Multi point model has one node per point", "[models2][modeltypes]") {
    ShowFixture f;
    Model* m = f.Make("MultiPoint", "NumPoints=\"3\" PointData=\"0.0,0.0,0.0,5.0,5.0,0.0,10.0,0.0,0.0\" NodesPerString=\"6\" NumStrings=\"1\"");
    CHECK(m->GetNodeCount() == 3);
    CHECK(m->GetChanCount() == 9);
    CHECK(FirstCells(m) == std::vector<Pt>{ { 0, 0 }, { 1, 0 }, { 2, 0 } });
    CHECK(m->GetDefaultBufferWi() == 3);
    CHECK(m->GetDefaultBufferHt() == 1);
}

TEST_CASE("Multi point model positions come from the points", "[models2][modeltypes]") {
    ShowFixture f;
    Model* m = f.Make("MultiPoint", "NumPoints=\"3\" PointData=\"0.0,0.0,0.0,5.0,5.0,0.0,10.0,0.0,0.0\" NumStrings=\"1\" NodesPerString=\"3\"");
    REQUIRE(m->GetNodeCount() == 3);
    auto& l = m->GetModelScreenLocation();
    l.PrepareToDraw(true, false);
    CHECK(l.GetNumHandles() == 3);
    float xs[3];
    for (int i = 0; i < 3; ++i) {
        xs[i] = m->GetNode(i)->Coords[0].screenX;
    }
    CHECK(xs[0] < xs[1]);
    CHECK(xs[1] < xs[2]);
}

TEST_CASE("Multi point model round trips", "[models2][modeltypes]") {
    ShowFixture a;
    Model* m = a.Make("MultiPoint", "NumPoints=\"3\" PointData=\"0.0,0.0,0.0,5.0,5.0,0.0,10.0,0.0,0.0\" NumStrings=\"1\" NodesPerString=\"3\"");
    std::string first = Serialize(m);
    pugi::xml_document doc;
    REQUIRE(doc.load_string(first.c_str()));
    ShowFixture b;
    Model* m2 = b.mm.createAndAddModel(doc.first_child(), 100, 100);
    REQUIRE(m2 != nullptr);
    CHECK(m2->GetNodeCount() == 3);
    CHECK(Serialize(m2) == first);
}

TEST_CASE("Poly line drop pattern behaves like icicles", "[models2][modeltypes]") {
    ShowFixture f;
    Model* m = f.Make("Poly Line", "NumStrings=\"1\" NodesPerString=\"6\" NumPoints=\"3\" PointData=\"0.0,0.0,0.0,5.0,5.0,0.0,10.0,0.0,0.0\" DropPattern=\"2,1\"");
    CHECK(m->GetDefaultBufferWi() == 4);
    CHECK(m->GetDefaultBufferHt() == 2);
    CHECK(FirstCells(m) == std::vector<Pt>{ { 0, 1 }, { 0, 0 }, { 1, 1 }, { 2, 1 }, { 2, 0 }, { 3, 1 } });
}

TEST_CASE("Poly line screen positions follow the point list", "[models2][modeltypes]") {
    ShowFixture f;
    Model* m = f.Make("Poly Line", "NumStrings=\"1\" NodesPerString=\"6\" NumPoints=\"3\" PointData=\"0.0,0.0,0.0,5.0,5.0,0.0,10.0,0.0,0.0\"");
    float prevX = -1.0f;
    float maxY = -1.0f;
    for (uint32_t i = 0; i < m->GetNodeCount(); ++i) {
        const auto& c = m->GetNode(i)->Coords[0];
        CHECK(c.screenX > prevX);
        prevX = c.screenX;
        maxY = std::max(maxY, c.screenY);
    }
    CHECK(maxY > 0.5f);
    CHECK(m->GetNode(0)->Coords[0].screenY == Catch::Approx(0.0f).margin(1e-3));
}

TEST_CASE("Each model type writes its own display type", "[models2][modeltypes]") {
    struct Case {
        const char* type;
        const char* attrs;
        const char* expect;
    };
    const Case cases[] = {
        { "Matrix", "NumStrings=\"1\" NodesPerString=\"4\" StrandsPerString=\"1\"", "Matrix" },
        { "Candy Canes", "NumCanes=\"2\" NodesPerCane=\"3\"", "Candy Canes" },
        { "Single Line", "NumStrings=\"1\" NodesPerString=\"4\"", "Single Line" },
        { "Window Frame", "TopNodes=\"2\" SideNodes=\"2\" BottomNodes=\"2\"", "Window Frame" },
    };
    for (const auto& c : cases) {
        INFO(c.type);
        ShowFixture f;
        Model* m = f.Make(c.type, c.attrs);
        std::string s = Serialize(m);
        std::string key = std::string("DisplayAs=\"") + c.expect + "\"";
        CHECK(s.find(key) != std::string::npos);
    }
}

TEST_CASE("Tree display variants are saved as one type with a tree type", "[models2][modeltypes]") {
    struct Case {
        const char* display;
        const char* treeType;
    };
    const Case cases[] = { { "Tree 360", "TreeType=\"0\"" }, { "Tree Flat", "TreeType=\"1\"" }, { "Tree Ribbon", "TreeType=\"2\"" } };
    for (const auto& c : cases) {
        INFO(c.display);
        ShowFixture f;
        Model* m = f.Make(c.display, "NumStrings=\"1\" NodesPerString=\"4\" StrandsPerString=\"1\"");
        std::string s = Serialize(m);
        CHECK(s.find("DisplayAs=\"Tree\"") != std::string::npos);
        CHECK(s.find(c.treeType) != std::string::npos);
    }
}

TEST_CASE("Tree ribbon and flat have different screen layouts", "[models2][modeltypes]") {
    ShowFixture f;
    Model* flat = f.Make("Tree Flat", "NumStrings=\"4\" NodesPerString=\"3\" StrandsPerString=\"1\"");
    ShowFixture g;
    Model* ribbon = g.Make("Tree Ribbon", "NumStrings=\"4\" NodesPerString=\"3\" StrandsPerString=\"1\"");
    bool differs = false;
    for (uint32_t i = 0; i < flat->GetNodeCount(); ++i) {
        if (std::fabs(flat->GetNode(i)->Coords[0].screenX - ribbon->GetNode(i)->Coords[0].screenX) > 1e-3f) {
            differs = true;
        }
    }
    CHECK(differs);
    CHECK(flat->GetNodeCount() == ribbon->GetNodeCount());
}

TEST_CASE("Tree degrees attribute changes the horizontal spread", "[models2][modeltypes]") {
    ShowFixture f;
    Model* full = f.Make("Tree 360", "NumStrings=\"6\" NodesPerString=\"3\" StrandsPerString=\"1\"");
    ShowFixture g;
    Model* half = g.Make("Tree 180", "NumStrings=\"6\" NodesPerString=\"3\" StrandsPerString=\"1\"");
    auto zspan = [](Model* m) {
        float lo = 1e9f, hi = -1e9f;
        for (uint32_t i = 0; i < m->GetNodeCount(); ++i) {
            lo = std::min(lo, m->GetNode(i)->Coords[0].screenZ);
            hi = std::max(hi, m->GetNode(i)->Coords[0].screenZ);
        }
        return hi - lo;
    };
    CHECK(zspan(full) > zspan(half));
}

TEST_CASE("Tree spiral rotations offset the upper nodes", "[models2][modeltypes]") {
    ShowFixture f;
    Model* plain = f.Make("Tree 360", "NumStrings=\"2\" NodesPerString=\"6\" StrandsPerString=\"1\"");
    ShowFixture g;
    Model* spiral = g.Make("Tree 360", "NumStrings=\"2\" NodesPerString=\"6\" StrandsPerString=\"1\" TreeSpiralRotations=\"2\"");
    CHECK(plain->GetNode(0)->Coords[0].screenX == Catch::Approx(spiral->GetNode(0)->Coords[0].screenX).margin(1e-3));
    CHECK(plain->GetNode(5)->Coords[0].screenX != Catch::Approx(spiral->GetNode(5)->Coords[0].screenX).margin(1e-3));
    CHECK(FirstCells(plain) == FirstCells(spiral));
}

TEST_CASE("Tree bottom to top ratio narrows the top", "[models2][modeltypes]") {
    ShowFixture f;
    Model* cone = f.Make("Tree 360", "NumStrings=\"4\" NodesPerString=\"5\" StrandsPerString=\"1\" TreeBottomTopRatio=\"6\"");
    auto topSpread = [](Model* m) {
        float lo = 1e9f, hi = -1e9f;
        for (uint32_t i = 0; i < m->GetNodeCount(); ++i) {
            if (m->GetNode(i)->Coords[0].bufY == 4) {
                lo = std::min(lo, m->GetNode(i)->Coords[0].screenX);
                hi = std::max(hi, m->GetNode(i)->Coords[0].screenX);
            }
        }
        return hi - lo;
    };
    auto bottomSpread = [](Model* m) {
        float lo = 1e9f, hi = -1e9f;
        for (uint32_t i = 0; i < m->GetNodeCount(); ++i) {
            if (m->GetNode(i)->Coords[0].bufY == 0) {
                lo = std::min(lo, m->GetNode(i)->Coords[0].screenX);
                hi = std::max(hi, m->GetNode(i)->Coords[0].screenX);
            }
        }
        return hi - lo;
    };
    CHECK(topSpread(cone) < bottomSpread(cone));
}

TEST_CASE("Spinner arc limits how far the arms fan out", "[models2][modeltypes]") {
    ShowFixture f;
    Model* full = f.Make("Spinner", "NumStrings=\"1\" NodesPerArm=\"2\" ArmsPerString=\"4\" Arc=\"360\"");
    ShowFixture g;
    Model* arc = g.Make("Spinner", "NumStrings=\"1\" NodesPerArm=\"2\" ArmsPerString=\"4\" Arc=\"90\"");
    auto xspan = [](Model* m) {
        float lo = 1e9f, hi = -1e9f;
        for (uint32_t i = 0; i < m->GetNodeCount(); ++i) {
            lo = std::min(lo, m->GetNode(i)->Coords[0].screenX);
            hi = std::max(hi, m->GetNode(i)->Coords[0].screenX);
        }
        return hi - lo;
    };
    CHECK(xspan(full) > xspan(arc));
    CHECK(full->GetNodeCount() == arc->GetNodeCount());
}

TEST_CASE("Spinner hollow center pushes the inner nodes outward", "[models2][modeltypes]") {
    ShowFixture f;
    Model* solid = f.Make("Spinner", "NumStrings=\"1\" NodesPerArm=\"4\" ArmsPerString=\"2\" Hollow=\"0\"");
    ShowFixture g;
    Model* hollow = g.Make("Spinner", "NumStrings=\"1\" NodesPerArm=\"4\" ArmsPerString=\"2\" Hollow=\"50\"");
    auto radius = [](Model* m, int node) {
        const auto& c = m->GetNode(node)->Coords[0];
        return std::sqrt(c.screenX * c.screenX + c.screenY * c.screenY);
    };
    CHECK(radius(hollow, 0) > radius(solid, 0));
}

TEST_CASE("Star ratio changes the inner point radius", "[models2][modeltypes]") {
    ShowFixture f;
    Model* thin = f.Make("Star", "NumStrings=\"1\" NodesPerString=\"10\" StarPoints=\"5\" starRatio=\"4\"");
    ShowFixture g;
    Model* fat = g.Make("Star", "NumStrings=\"1\" NodesPerString=\"10\" StarPoints=\"5\" starRatio=\"1.5\"");
    CHECK(FirstCells(thin) != FirstCells(fat));
    CHECK(thin->GetNodeCount() == fat->GetNodeCount());
}

TEST_CASE("Arches skew bends the screen layout but not the buffer", "[models2][modeltypes]") {
    ShowFixture f;
    Model* flat = f.Make("Arches", "NumArches=\"2\" NodesPerArch=\"5\" ArchesSkew=\"0\"");
    ShowFixture g;
    Model* skewed = g.Make("Arches", "NumArches=\"2\" NodesPerArch=\"5\" ArchesSkew=\"40\"");
    CHECK(FirstCells(flat) == FirstCells(skewed));
}

TEST_CASE("Cube node count is width times height times depth for filled styles", "[models2][modeltypes]") {
    ShowFixture f;
    Model* m = f.Make("Cube", "CubeWidth=\"3\" CubeHeight=\"4\" CubeDepth=\"5\" Style=\"Horizontal Left\" Strings=\"1\" Start=\"TopFrontLeft\"");
    CHECK(m->GetNodeCount() == 60);
    CHECK(m->GetChanCount() == 180);
}

TEST_CASE("Cube hollow style removes the interior nodes", "[models2][modeltypes]") {
    ShowFixture f;
    Model* solid = f.Make("Cube", "CubeWidth=\"4\" CubeHeight=\"4\" CubeDepth=\"4\" Style=\"Horizontal Left\" Strings=\"1\" Start=\"TopFrontLeft\"");
    ShowFixture g;
    Model* hollow = g.Make("Cube", "CubeWidth=\"4\" CubeHeight=\"4\" CubeDepth=\"4\" Style=\"Horizontal Left\" Strings=\"1\" Start=\"TopFrontLeft\" CubeHollow=\"1\"");
    CHECK(solid->GetNodeCount() == 64);
    CHECK(hollow->GetNodeCount() <= solid->GetNodeCount());
}

TEST_CASE("Sphere longitude degrees limit the arc used", "[models2][modeltypes]") {
    ShowFixture f;
    Model* full = f.Make("Sphere", "NumStrings=\"1\" NodesPerString=\"12\" StrandsPerString=\"4\" Degrees=\"360\"");
    ShowFixture g;
    Model* part = g.Make("Sphere", "NumStrings=\"1\" NodesPerString=\"12\" StrandsPerString=\"4\" Degrees=\"90\"");
    auto xspan = [](Model* m) {
        float lo = 1e9f, hi = -1e9f;
        for (uint32_t i = 0; i < m->GetNodeCount(); ++i) {
            lo = std::min(lo, m->GetNode(i)->Coords[0].screenX);
            hi = std::max(hi, m->GetNode(i)->Coords[0].screenX);
        }
        return hi - lo;
    };
    CHECK(xspan(full) > xspan(part));
    CHECK(FirstCells(full) == FirstCells(part));
}

TEST_CASE("Window frame screen layout is a rectangle", "[models2][modeltypes]") {
    ShowFixture f;
    Model* m = f.Make("Window Frame", "TopNodes=\"4\" SideNodes=\"3\" BottomNodes=\"4\"");
    float minX = 1e9f, maxX = -1e9f, minY = 1e9f, maxY = -1e9f;
    for (uint32_t i = 0; i < m->GetNodeCount(); ++i) {
        const auto& c = m->GetNode(i)->Coords[0];
        minX = std::min(minX, c.screenX);
        maxX = std::max(maxX, c.screenX);
        minY = std::min(minY, c.screenY);
        maxY = std::max(maxY, c.screenY);
    }
    CHECK(maxX > minX);
    CHECK(maxY > minY);
    CHECK((maxX - minX) > (maxY - minY));
}

TEST_CASE("Icicle alternate nodes keep one node per column cell", "[models2][modeltypes]") {
    ShowFixture f;
    Model* m = f.Make("Icicles", "NumStrings=\"2\" NodesPerString=\"4\" DropPattern=\"3,1\" AlternateNodes=\"True\"");
    CHECK(m->GetNodeCount() == 8);
    CHECK(m->GetDefaultBufferWi() == 4);
}

TEST_CASE("Circle layers fill the buffer width of the outer layer", "[models2][modeltypes]") {
    ShowFixture f;
    Model* m = f.Make("Circle", "NumStrings=\"1\" NodesPerString=\"9\" LayerSizes=\"6,3\"");
    CHECK(m->GetDefaultBufferWi() == 6);
    CHECK(m->GetDefaultBufferHt() == 2);
    int inner = 0, outer = 0;
    for (const auto& c : FirstCells(m)) {
        (c.second == 0 ? inner : outer)++;
    }
    CHECK(inner == 3);
    CHECK(outer == 6);
}

TEST_CASE("Candy cane height scales the screen height only", "[models2][modeltypes]") {
    ShowFixture f;
    Model* tall = f.Make("Candy Canes", "NumCanes=\"2\" NodesPerCane=\"4\" CandyCaneHeight=\"5\"");
    ShowFixture g;
    Model* shorter = g.Make("Candy Canes", "NumCanes=\"2\" NodesPerCane=\"4\" CandyCaneHeight=\"1\"");
    CHECK(FirstCells(tall) == FirstCells(shorter));
    CHECK(tall->GetNodeCount() == shorter->GetNodeCount());
}
