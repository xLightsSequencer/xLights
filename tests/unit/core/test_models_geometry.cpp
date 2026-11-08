#include <functional>
#include <list>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <pugixml.hpp>

#include "XmlSerializer/XmlSerializer.h"
#include "models/CustomModel.h"
#include "models/Model.h"
#include "models/ModelManager.h"
#include "models/SubModel.h"
#include "models/TwoPointScreenLocation.h"
#include "outputs/OutputManager.h"

namespace {

struct ShowFixture {
    OutputManager om;
    ModelManager mm{ &om };

    Model* Add(const std::string& xml) {
        pugi::xml_document doc;
        REQUIRE(doc.load_string(xml.c_str()));
        Model* m = mm.createAndAddModel(doc.first_child(), 100, 100);
        mm.RecalcStartChannels();
        return m;
    }

    Model* Default(const std::string& type) {
        Model* m = mm.CreateDefaultModel(type, "1");
        REQUIRE(m != nullptr);
        mm.AddModel(m);
        mm.RecalcStartChannels();
        return m;
    }
};

using Pt = std::pair<int, int>;

std::vector<Pt> BufferPoints(const Model* m) {
    std::vector<Pt> pts;
    for (uint32_t i = 0; i < m->GetNodeCount(); ++i) {
        const auto* n = m->GetNode(i);
        pts.push_back({ n->Coords[0].bufX, n->Coords[0].bufY });
    }
    return pts;
}

std::vector<int> StringNumbers(const Model* m) {
    std::vector<int> r;
    for (uint32_t i = 0; i < m->GetNodeCount(); ++i) {
        r.push_back(m->GetNodeStringNumber(i));
    }
    return r;
}

std::string ModelXml(const std::string& name, const std::string& displayAs, const std::string& attrs) {
    return "<model name=\"" + name + "\" DisplayAs=\"" + displayAs + "\" StartChannel=\"1\" " + attrs + "/>";
}

std::string HostileXml(const std::string& name, const std::string& type) {
    return "<model name=\"" + name + "\" DisplayAs=\"" + type + "\" StartChannel=\"1\" NumStrings=\"2\" NodesPerString=\"4\" StrandsPerString=\"1\" NumArches=\"2\" NodesPerArch=\"3\" NumCanes=\"2\" NodesPerCane=\"3\" LightsPerNode=\"1\" StarPoints=\"5\" NodesPerArm=\"3\" ArmsPerString=\"3\" TopNodes=\"3\" SideNodes=\"3\" BottomNodes=\"3\" CubeWidth=\"2\" CubeHeight=\"2\" CubeDepth=\"2\" CustomWidth=\"2\" CustomHeight=\"2\" NumChannels=\"3\"/>";
}

} // namespace

TEST_CASE("Tree counts nodes strings and strands", "[models][geometry]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("T", "Tree 360", "NumStrings=\"3\" NodesPerString=\"4\" StrandsPerString=\"1\""));
    CHECK(m->GetNodeCount() == 12);
    CHECK(m->GetChanCount() == 36);
    CHECK(m->GetNumStrings() == 3);
    CHECK(m->GetNumStrands() == 3);
    CHECK(m->GetChanCountPerNode() == 3);
    CHECK(m->GetDefaultBufferWi() == 3);
    CHECK(m->GetDefaultBufferHt() == 4);
    CHECK(StringNumbers(m) == std::vector<int>{ 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2 });
}

TEST_CASE("Tree buffer coordinates run up each string", "[models][geometry]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("T", "Tree 360", "NumStrings=\"3\" NodesPerString=\"4\" StrandsPerString=\"1\""));
    std::vector<Pt> expect;
    for (int s = 0; s < 3; ++s) {
        for (int n = 0; n < 4; ++n) {
            expect.push_back({ s, n });
        }
    }
    CHECK(BufferPoints(m) == expect);
}

TEST_CASE("Tree node channels are consecutive", "[models][geometry]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("T", "Tree 360", "NumStrings=\"2\" NodesPerString=\"5\" StrandsPerString=\"1\""));
    for (uint32_t i = 0; i < m->GetNodeCount(); ++i) {
        CHECK(m->NodeStartChannel(i) == (int32_t)(i * 3));
        CHECK(m->NodeEndChannel(i) == (int32_t)(i * 3 + 2));
    }
}

TEST_CASE("Horizontal matrix zig zags between rows", "[models][geometry]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("M", "Matrix", "NumStrings=\"1\" NodesPerString=\"12\" StrandsPerString=\"3\""));
    CHECK(m->GetNodeCount() == 12);
    CHECK(m->GetDefaultBufferWi() == 4);
    CHECK(m->GetDefaultBufferHt() == 3);
    CHECK(m->GetNumStrands() == 3);
    CHECK(BufferPoints(m) == std::vector<Pt>{ { 0, 0 }, { 1, 0 }, { 2, 0 }, { 3, 0 }, { 3, 1 }, { 2, 1 }, { 1, 1 }, { 0, 1 }, { 0, 2 }, { 1, 2 }, { 2, 2 }, { 3, 2 } });
}

TEST_CASE("Matrix with NoZig always runs the same direction", "[models][geometry]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("M", "Matrix", "NumStrings=\"1\" NodesPerString=\"12\" StrandsPerString=\"3\" NoZig=\"true\""));
    CHECK(BufferPoints(m) == std::vector<Pt>{ { 0, 0 }, { 1, 0 }, { 2, 0 }, { 3, 0 }, { 0, 1 }, { 1, 1 }, { 2, 1 }, { 3, 1 }, { 0, 2 }, { 1, 2 }, { 2, 2 }, { 3, 2 } });
}

TEST_CASE("Vertical matrix swaps rows and columns", "[models][geometry]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("M", "Matrix", "NumStrings=\"1\" NodesPerString=\"12\" StrandsPerString=\"3\" Vertical=\"true\""));
    CHECK(m->GetDefaultBufferWi() == 3);
    CHECK(m->GetDefaultBufferHt() == 4);
    CHECK(BufferPoints(m) == std::vector<Pt>{ { 0, 0 }, { 0, 1 }, { 0, 2 }, { 0, 3 }, { 1, 3 }, { 1, 2 }, { 1, 1 }, { 1, 0 }, { 2, 0 }, { 2, 1 }, { 2, 2 }, { 2, 3 } });
}

TEST_CASE("Legacy Vert Matrix display type is a vertical matrix", "[models][geometry]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("M", "Vert Matrix", "parm1=\"1\" parm2=\"12\" parm3=\"3\""));
    REQUIRE(m != nullptr);
    CHECK(m->GetDisplayAs() == DisplayAsType::Matrix);
    CHECK(m->GetNodeCount() == 12);
    CHECK(m->GetDefaultBufferWi() == 3);
    CHECK(m->GetDefaultBufferHt() == 4);
}

TEST_CASE("Matrix start side and direction change where node zero is", "[models][geometry]") {
    ShowFixture f;
    Model* top = f.Add(ModelXml("Top", "Matrix", "StartSide=\"T\" NumStrings=\"1\" NodesPerString=\"6\" StrandsPerString=\"2\""));
    CHECK(BufferPoints(top) == std::vector<Pt>{ { 0, 1 }, { 1, 1 }, { 2, 1 }, { 2, 0 }, { 1, 0 }, { 0, 0 } });
    Model* right = f.Add(ModelXml("Right", "Matrix", "Dir=\"R\" NumStrings=\"1\" NodesPerString=\"6\" StrandsPerString=\"2\""));
    CHECK(BufferPoints(right) == std::vector<Pt>{ { 2, 0 }, { 1, 0 }, { 0, 0 }, { 0, 1 }, { 1, 1 }, { 2, 1 } });
}

TEST_CASE("New matrix keeps its start location through copy and paste", "[models][geometry]") {
    ShowFixture f;
    Model* m = f.Default("Matrix");
    CHECK(m->GetStartSide() == "T");
    CHECK_FALSE(m->GetIsBtoT());

    pugi::xml_document doc;
    XmlSerializingVisitor v(&doc);
    m->Accept(v);
    pugi::xml_node nd = doc.document_element();
    nd.attribute("name").set_value("Pasted");
    Model* pasted = f.mm.createAndAddModel(nd, 100, 100);
    f.mm.RecalcStartChannels();
    REQUIRE(pasted != nullptr);
    CHECK(pasted->GetIsBtoT() == m->GetIsBtoT());
    CHECK(pasted->GetIsLtoR() == m->GetIsLtoR());
    CHECK(BufferPoints(pasted) == BufferPoints(m));
}

TEST_CASE("Setting start side and direction also sets the wiring the model renders with", "[models][geometry]") {
    ShowFixture f;
    for (const char* type : { "Matrix", "Tree 360", "Sphere", "Window Frame", "Wreath", "Circle", "Arches", "Single Line" }) {
        INFO(type);
        Model* m = f.Default(type);
        m->SetStartSide("B");
        m->SetDirection("R");
        CHECK(m->GetIsBtoT());
        CHECK_FALSE(m->GetIsLtoR());
        m->SetStartSide("T");
        m->SetDirection("L");
        CHECK_FALSE(m->GetIsBtoT());
        CHECK(m->GetIsLtoR());
    }
}

TEST_CASE("Matrix given a start corner in code wires the same after save and reload", "[models][geometry]") {
    ShowFixture f;
    Model* m = f.Default("Matrix");
    m->SetStartSide("B");
    m->SetDirection("R");
    m->Setup();

    pugi::xml_document doc;
    XmlSerializingVisitor v(&doc);
    m->Accept(v);
    pugi::xml_node nd = doc.document_element();
    nd.attribute("name").set_value("Reloaded");
    Model* reloaded = f.mm.createAndAddModel(nd, 100, 100);
    f.mm.RecalcStartChannels();
    REQUIRE(reloaded != nullptr);
    CHECK(reloaded->GetIsBtoT() == m->GetIsBtoT());
    CHECK(reloaded->GetIsLtoR() == m->GetIsLtoR());
    CHECK(BufferPoints(reloaded) == BufferPoints(m));
}

TEST_CASE("Circle saved without a start side keeps its start through save and reload", "[models][geometry]") {
    ShowFixture f;
    Model* c = f.Add(ModelXml("Ci", "Circle", "NumStrings=\"1\" NodesPerString=\"8\""));
    CHECK_FALSE(c->GetIsBtoT());
    CHECK(c->GetStartSide() == "T");

    pugi::xml_document doc;
    XmlSerializingVisitor v(&doc);
    c->Accept(v);
    pugi::xml_node nd = doc.document_element();
    nd.attribute("name").set_value("Reloaded");
    Model* reloaded = f.mm.createAndAddModel(nd, 100, 100);
    f.mm.RecalcStartChannels();
    REQUIRE(reloaded != nullptr);
    CHECK_FALSE(reloaded->GetIsBtoT());
    CHECK(BufferPoints(reloaded) == BufferPoints(c));
}

TEST_CASE("Matrix with several strings numbers nodes by string", "[models][geometry]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("M", "Matrix", "NumStrings=\"2\" NodesPerString=\"6\" StrandsPerString=\"2\""));
    CHECK(m->GetNodeCount() == 12);
    CHECK(m->GetNumStrings() == 2);
    CHECK(m->GetNumStrands() == 4);
    CHECK(m->GetDefaultBufferWi() == 3);
    CHECK(m->GetDefaultBufferHt() == 4);
    CHECK(StringNumbers(m) == std::vector<int>{ 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 });
    CHECK(m->NodesPerString() == 6);
}

TEST_CASE("Arches have one row per arch", "[models][geometry]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("A", "Arches", "NumArches=\"2\" NodesPerArch=\"3\" LightsPerNode=\"1\""));
    CHECK(m->GetNodeCount() == 6);
    CHECK(m->GetNumStrings() == 2);
    CHECK(m->GetDefaultBufferWi() == 3);
    CHECK(m->GetDefaultBufferHt() == 2);
    CHECK(BufferPoints(m) == std::vector<Pt>{ { 0, 0 }, { 1, 0 }, { 2, 0 }, { 0, 1 }, { 1, 1 }, { 2, 1 } });
    CHECK(StringNumbers(m) == std::vector<int>{ 0, 0, 0, 1, 1, 1 });
}

TEST_CASE("Arches with several lights per node report multiple coordinates", "[models][geometry]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("A", "Arches", "NumArches=\"1\" NodesPerArch=\"4\" LightsPerNode=\"3\""));
    CHECK(m->GetNodeCount() == 4);
    CHECK(m->GetChanCount() == 12);
    CHECK(m->IsMultiCoordsPerNode());
    CHECK(m->GetCoordCount(0) == 3);
}

TEST_CASE("Single line strings are laid end to end", "[models][geometry]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("L", "Single Line", "NumStrings=\"2\" NodesPerString=\"3\""));
    CHECK(m->GetNodeCount() == 6);
    CHECK(m->GetDefaultBufferWi() == 6);
    CHECK(m->GetDefaultBufferHt() == 1);
    CHECK(BufferPoints(m) == std::vector<Pt>{ { 0, 0 }, { 1, 0 }, { 2, 0 }, { 3, 0 }, { 4, 0 }, { 5, 0 } });
    CHECK(StringNumbers(m) == std::vector<int>{ 0, 0, 0, 1, 1, 1 });
}

TEST_CASE("Candy canes are columns of nodes", "[models][geometry]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("C", "Candy Canes", "NumCanes=\"2\" NodesPerCane=\"5\""));
    CHECK(m->GetNodeCount() == 10);
    CHECK(m->GetNumStrings() == 2);
    CHECK(m->GetDefaultBufferWi() == 2);
    CHECK(m->GetDefaultBufferHt() == 5);
    CHECK(BufferPoints(m).front() == Pt{ 0, 0 });
    CHECK(BufferPoints(m)[4] == Pt{ 0, 4 });
    CHECK(BufferPoints(m)[5] == Pt{ 1, 0 });
}

TEST_CASE("Icicle drop pattern determines the buffer height", "[models][geometry]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("I", "Icicles", "NumStrings=\"2\" NodesPerString=\"10\" DropPattern=\"3,4\""));
    CHECK(m->GetNodeCount() == 20);
    CHECK(m->GetDefaultBufferHt() == 4);
    CHECK(m->GetDefaultBufferWi() == 6);
    auto pts = BufferPoints(m);
    CHECK(pts[0] == Pt{ 0, 3 });
    CHECK(pts[1] == Pt{ 0, 2 });
    CHECK(pts[2] == Pt{ 0, 1 });
    CHECK(pts[3] == Pt{ 1, 3 });
}

TEST_CASE("Circle star wreath and spinner node counts", "[models][geometry]") {
    ShowFixture f;
    Model* circle = f.Add(ModelXml("Ci", "Circle", "NumStrings=\"2\" NodesPerString=\"8\""));
    CHECK(circle->GetNodeCount() == 16);
    CHECK(circle->GetNumStrings() == 2);

    Model* star = f.Add(ModelXml("St", "Star", "NumStrings=\"1\" NodesPerString=\"10\" StarPoints=\"5\""));
    CHECK(star->GetNodeCount() == 10);
    CHECK(star->GetChanCount() == 30);

    Model* wreath = f.Add(ModelXml("Wr", "Wreath", "NumStrings=\"1\" NodesPerString=\"6\""));
    CHECK(wreath->GetNodeCount() == 6);

    Model* spinner = f.Add(ModelXml("Sp", "Spinner", "NumStrings=\"1\" NodesPerArm=\"3\" ArmsPerString=\"4\""));
    CHECK(spinner->GetNodeCount() == 12);
    CHECK(spinner->GetDefaultBufferWi() == 4);
    CHECK(spinner->GetDefaultBufferHt() == 3);
}

TEST_CASE("Window frame node count is top plus bottom plus two sides", "[models][geometry]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("W", "Window Frame", "TopNodes=\"3\" SideNodes=\"2\" BottomNodes=\"3\""));
    CHECK(m->GetNodeCount() == 10);
    CHECK(m->GetDefaultBufferWi() == 5);
    CHECK(m->GetDefaultBufferHt() == 2);
}

TEST_CASE("Cube and sphere node counts", "[models][geometry]") {
    ShowFixture f;
    Model* cube = f.Add(ModelXml("Cb", "Cube", "CubeWidth=\"2\" CubeHeight=\"3\" CubeDepth=\"4\""));
    CHECK(cube->GetNodeCount() == 24);
    CHECK(cube->GetChanCount() == 72);

    Model* sphere = f.Add(ModelXml("Sh", "Sphere", "NumStrings=\"1\" NodesPerString=\"12\" StrandsPerString=\"3\""));
    CHECK(sphere->GetNodeCount() == 12);
    CHECK(sphere->GetDefaultBufferWi() == 3);
    CHECK(sphere->GetDefaultBufferHt() == 4);
}

TEST_CASE("Channel block has one single channel node per channel", "[models][geometry]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("C", "Channel Block", "NumChannels=\"7\" StringType=\"Single Color Custom\""));
    CHECK(m->GetNodeCount() == 7);
    CHECK(m->GetChanCount() == 7);
    CHECK(m->GetChanCountPerNode() == 1);
    CHECK(m->GetNumStrands() == 7);
    CHECK(m->GetDefaultBufferWi() == 7);
    CHECK(m->GetDefaultBufferHt() == 1);
    CHECK(m->GetLastChannel() == 6);
}

TEST_CASE("Custom model places nodes from the grid", "[models][geometry]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("C", "Custom", "CustomWidth=\"3\" CustomHeight=\"2\" CustomModel=\"1,,2;,3,\""));
    CHECK(m->GetNodeCount() == 3);
    CHECK(m->GetChanCount() == 9);
    CHECK(m->GetDefaultBufferWi() == 3);
    CHECK(m->GetDefaultBufferHt() == 2);
    CHECK(BufferPoints(m) == std::vector<Pt>{ { 0, 1 }, { 2, 1 }, { 1, 0 } });
}

TEST_CASE("Custom model node numbers follow the numbers in the grid", "[models][geometry]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("C", "Custom", "CustomWidth=\"3\" CustomHeight=\"1\" CustomModel=\"3,1,2\""));
    REQUIRE(m->GetNodeCount() == 3);
    CHECK(BufferPoints(m) == std::vector<Pt>{ { 1, 0 }, { 2, 0 }, { 0, 0 } });
    CHECK(m->NodeStartChannel(0) == 0);
    CHECK(m->NodeStartChannel(2) == 6);
}

TEST_CASE("Custom model compressed form matches the uncompressed form", "[models][geometry]") {
    ShowFixture f;
    Model* a = f.Add(ModelXml("A", "Custom", "CustomWidth=\"3\" CustomHeight=\"2\" CustomModel=\"1,,2;,3,\""));
    Model* b = f.Add(ModelXml("B", "Custom", "CustomWidth=\"3\" CustomHeight=\"2\" CustomModelCompressed=\"1,0,0;2,0,2;3,1,1\""));
    CHECK(BufferPoints(a) == BufferPoints(b));
    CHECK(a->GetChanCount() == b->GetChanCount());
}

TEST_CASE("Custom model with depth has a node for every layer cell", "[models][geometry]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("C", "Custom", "CustomWidth=\"2\" CustomHeight=\"2\" Depth=\"2\" CustomModel=\"1,2;3,4|5,6;7,8\""));
    CHECK(m->GetNodeCount() == 8);
    CHECK(m->GetChanCount() == 24);
}

TEST_CASE("Empty custom model has no nodes and survives queries", "[models][geometry]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("C", "Custom", "CustomWidth=\"2\" CustomHeight=\"2\""));
    REQUIRE(m != nullptr);
    CHECK(m->GetNodeCount() == 0);
    CHECK(m->GetChanCount() == 0);
    CHECK(m->GetNumSubModels() == 0);
}

TEST_CASE("Custom model ignores malformed grid cells", "[models][geometry]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("C", "Custom", "CustomWidth=\"3\" CustomHeight=\"1\" CustomModel=\"1,-4,junk\""));
    REQUIRE(m != nullptr);
    CHECK(m->GetNodeCount() >= 1);
}

TEST_CASE("CreateDefaultModel node and channel counts", "[models][geometry]") {
    struct Case {
        const char* type;
        uint32_t nodes;
        uint32_t chans;
    };
    const Case cases[] = {
        { "Star", 50, 150 },
        { "Arches", 50, 150 },
        { "Candy Canes", 54, 162 },
        { "Channel Block", 16, 16 },
        { "Circle", 50, 150 },
        { "Window Frame", 132, 396 },
        { "Wreath", 50, 150 },
        { "Sphere", 100, 300 },
        { "Single Line", 50, 150 },
        { "Poly Line", 50, 150 },
        { "Cube", 125, 375 },
        { "Tree", 800, 2400 },
        { "Matrix", 800, 2400 },
        { "Spinner", 50, 150 },
        { "Icicles", 80, 240 },
        { "Image", 1, 1 },
        { "Label", 1, 1 },
    };
    for (const auto& c : cases) {
        INFO(c.type);
        ShowFixture f;
        Model* m = f.Default(c.type);
        CHECK(m->GetNodeCount() == c.nodes);
        CHECK(m->GetChanCount() == c.chans);
        CHECK(m->GetFirstChannel() == 0);
        CHECK(m->GetLastChannel() == c.chans - 1);
    }
}

TEST_CASE("CreateDefaultModel DMX channel counts", "[models][geometry]") {
    struct Case {
        const char* type;
        uint32_t chans;
    };
    const Case cases[] = {
        { "DmxMovingHead", 8 },
        { "DmxMovingHeadAdv", 8 },
        { "DmxGeneral", 8 },
        { "DmxFloodlight", 3 },
        { "DmxFloodArea", 3 },
        { "DmxSkull", 26 },
        { "DmxServo", 2 },
        { "DmxServo3d", 2 },
        { "DmxServo3Axis", 6 },
    };
    for (const auto& c : cases) {
        INFO(c.type);
        ShowFixture f;
        Model* m = f.Default(c.type);
        CHECK(IsDmxDisplayType(m->GetDisplayAs()));
        CHECK(m->GetChanCount() == c.chans);
        CHECK(m->GetChanCountPerNode() == 1);
        CHECK(m->GetLastChannel() == c.chans - 1);
    }
}

TEST_CASE("CreateDefaultModel rejects unknown types", "[models][geometry]") {
    ShowFixture f;
    CHECK(f.mm.CreateDefaultModel("Not A Model", "1") == nullptr);
    CHECK(f.mm.CreateDefaultModel("", "1") == nullptr);
}

TEST_CASE("CreateDefaultModel generates unique names", "[models][geometry]") {
    ShowFixture f;
    Model* a = f.Default("Single Line");
    Model* b = f.Default("Single Line");
    CHECK(a->GetName() != b->GetName());
    CHECK(f.mm.size() == 2);
}

TEST_CASE("String types set the number of channels per node", "[models][geometry]") {
    struct Case {
        const char* type;
        int perNode;
        uint32_t chans;
    };
    const Case cases[] = {
        { "RGB Nodes", 3, 12 },
        { "RBG Nodes", 3, 12 },
        { "GRB Nodes", 3, 12 },
        { "RGBW Nodes", 4, 16 },
        { "WRGB Nodes", 4, 16 },
        { "4 Channel RGBW", 4, 4 },
        { "RGBWW Nodes", 5, 20 },
        { "Node Single Color", 1, 4 },
    };
    for (const auto& c : cases) {
        INFO(c.type);
        ShowFixture f;
        Model* m = f.Add(ModelXml("M", "Single Line", std::string("NumStrings=\"1\" NodesPerString=\"4\" StringType=\"") + c.type + "\""));
        CHECK(m->GetChanCountPerNode() == c.perNode);
        CHECK(m->GetChanCount() == c.chans);
    }
}

TEST_CASE("Single color string types collapse a string to one channel", "[models][geometry]") {
    ShowFixture f;
    for (const char* type : { "Single Color Red", "Single Color White", "Strobes" }) {
        INFO(type);
        Model* m = f.Add(ModelXml(std::string("M") + type, "Single Line", std::string("NumStrings=\"1\" NodesPerString=\"4\" StringType=\"") + type + "\""));
        CHECK(m->GetNodeCount() == 1);
        CHECK(m->GetChanCount() == 1);
    }
}

TEST_CASE("Missing StringType defaults to RGB nodes", "[models][geometry]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("M", "Single Line", "NumStrings=\"1\" NodesPerString=\"4\""));
    CHECK(m->GetStringType() == "RGB Nodes");
    CHECK(m->GetChanCountPerNode() == 3);
}

TEST_CASE("Node and strand names resolve with defaults", "[models][geometry]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("M", "Matrix", "NumStrings=\"1\" NodesPerString=\"4\" StrandsPerString=\"2\" NodeNames=\"a,b,,d\" StrandNames=\"top,bottom\""));
    CHECK(m->GetNodeNames() == "a,b,,d");
    CHECK(m->GetStrandNames() == "top,bottom");
    CHECK(m->GetNodeName(0) == "a");
    CHECK(m->GetNodeName(1) == "b");
    CHECK(m->GetNodeName(3) == "d");
    CHECK(m->GetStrandName(0) == "top");
    CHECK(m->GetStrandName(1) == "bottom");
    CHECK(m->GetStrandName(2).empty());
    CHECK(m->GetStrandName(2, true) == "Strand 3");
    CHECK(m->GetNodeName(2).empty());
    CHECK(m->GetNodeName(4, true) == "Node 5");
}

TEST_CASE("Strand length is nodes divided by strands", "[models][geometry]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("M", "Matrix", "NumStrings=\"2\" NodesPerString=\"6\" StrandsPerString=\"2\""));
    CHECK(m->GetStrandLength(0) == 3);
    CHECK(m->GetStrandLength(3) == 3);
}

TEST_CASE("Buffer sizes for the standard buffer styles", "[models][geometry]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("M", "Matrix", "NumStrings=\"1\" NodesPerString=\"12\" StrandsPerString=\"3\""));
    int w = 0, h = 0;
    m->GetBufferSize("Default", "2D", "None", w, h, 0);
    CHECK(w == 4);
    CHECK(h == 3);
    m->GetBufferSize("Single Line", "2D", "None", w, h, 0);
    CHECK(w == 12);
    CHECK(h == 1);
    m->GetBufferSize("Default", "2D", "Rotate CW 90", w, h, 0);
    CHECK(w == 3);
    CHECK(h == 4);
}

TEST_CASE("Rendered nodes for a buffer style cover every model node", "[models][geometry]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("M", "Matrix", "NumStrings=\"1\" NodesPerString=\"12\" StrandsPerString=\"3\""));
    std::vector<NodeBaseClassPtr> nodes;
    int w = 0, h = 0;
    m->InitRenderBufferNodes("Default", "2D", "None", nodes, w, h, 0);
    CHECK(nodes.size() == 12);
    CHECK(w == 4);
    CHECK(h == 3);

    nodes.clear();
    m->InitRenderBufferNodes("Single Line", "2D", "None", nodes, w, h, 0);
    CHECK(nodes.size() == 12);
    CHECK(w == 12);
    CHECK(h == 1);
}

TEST_CASE("Legacy parm attributes still define model sizes", "[models][geometry]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("T", "Tree 360", "parm1=\"2\" parm2=\"5\" parm3=\"1\""));
    CHECK(m->GetNodeCount() == 10);
    CHECK(m->GetNumStrings() == 2);
}

TEST_CASE("Missing size attributes give a minimal one node model", "[models][geometry]") {
    ShowFixture f;
    Model* tree = f.Add(ModelXml("T", "Tree 360", ""));
    Model* arch = f.Add(ModelXml("A", "Arches", ""));
    Model* line = f.Add(ModelXml("L", "Single Line", ""));
    CHECK(tree->GetNodeCount() == 1);
    CHECK(arch->GetNodeCount() == 1);
    CHECK(line->GetNodeCount() == 50);
}

TEST_CASE("Zero blank and non numeric sizes do not crash model construction", "[models][geometry]") {
    struct Case {
        const char* type;
        std::vector<const char*> attrs;
    };
    const std::vector<Case> cases = {
        { "Tree 360", { "NumStrings", "StrandsPerString" } },
        { "Matrix", { "NumStrings", "StrandsPerString" } },
        { "Arches", { "NumArches", "LightsPerNode" } },
        { "Single Line", { "NumStrings", "NodesPerString", "LightsPerNode" } },
        { "Star", { "NumStrings", "NodesPerString", "StarPoints" } },
        { "Circle", { "NumStrings", "NodesPerString" } },
        { "Icicles", { "NumStrings", "NodesPerString" } },
        { "Candy Canes", { "NumCanes", "NodesPerCane", "LightsPerNode" } },
        { "Spinner", { "NumStrings", "NodesPerArm", "ArmsPerString" } },
        { "Wreath", { "NumStrings", "NodesPerString" } },
        { "Window Frame", { "TopNodes", "SideNodes", "BottomNodes" } },
        { "Sphere", { "NumStrings", "StrandsPerString" } },
        { "Custom", { "CustomWidth", "CustomHeight" } },
        { "Channel Block", { "NumChannels" } },
    };
    ShowFixture f;
    int i = 0;
    for (const auto& c : cases) {
        for (const char* attr : c.attrs) {
            for (const char* v : { "0", "abc", "" }) {
                INFO(c.type << " " << attr << "='" << v << "'");
                pugi::xml_document doc;
                REQUIRE(doc.load_string(HostileXml("H" + std::to_string(i++), c.type).c_str()));
                doc.first_child().attribute(attr).set_value(v);
                Model* m = f.mm.createAndAddModel(doc.first_child(), 100, 100);
                REQUIRE(m != nullptr);
                (void)m->GetNodeCount();
                (void)m->GetChanCount();
                (void)m->GetFirstChannel();
            }
        }
    }
}

TEST_CASE("Negative string counts either load or throw", "[models][geometry]") {
    struct Case {
        const char* type;
        const char* attr;
    };
    const Case cases[] = {
        { "Tree 360", "NumStrings" }, { "Matrix", "NumStrings" }, { "Arches", "NumArches" }, { "Single Line", "NumStrings" },
        { "Star", "NumStrings" }, { "Circle", "NumStrings" }, { "Candy Canes", "NumCanes" }, { "Wreath", "NumStrings" },
        { "Channel Block", "NumChannels" }, { "Sphere", "NumStrings" },
    };
    ShowFixture f;
    int i = 0;
    for (const auto& c : cases) {
        INFO(c.type << " " << c.attr);
        pugi::xml_document doc;
        REQUIRE(doc.load_string(HostileXml("H" + std::to_string(i++), c.type).c_str()));
        doc.first_child().attribute(c.attr).set_value("-5");
        try {
            Model* m = f.mm.createAndAddModel(doc.first_child(), 100, 100);
            if (m != nullptr) {
                (void)m->GetNodeCount();
            }
        } catch (const std::exception&) {
        }
    }
    SUCCEED();
}

// Zero or negative counts and dimensions in a corrupt file are clamped to 1 when the
// model is deserialised, so construction yields a small sane model instead of crashing.
TEST_CASE("Zero nodes per string does not crash a tree", "[models][geometry]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("T", "Tree 360", "NumStrings=\"2\" NodesPerString=\"0\" StrandsPerString=\"1\""));
    REQUIRE(m != nullptr);
    CHECK(m->GetNodeCount() == 2);
}

TEST_CASE("Zero nodes per string does not crash a matrix or sphere", "[models][geometry]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("M", "Matrix", "NumStrings=\"2\" NodesPerString=\"0\" StrandsPerString=\"1\""));
    REQUIRE(m != nullptr);
    CHECK(m->GetNodeCount() == 2);
    Model* s = f.Add(ModelXml("S", "Sphere", "NumStrings=\"1\" NodesPerString=\"0\" StrandsPerString=\"1\""));
    REQUIRE(s != nullptr);
    CHECK(s->GetNodeCount() == 1);
}

TEST_CASE("Zero nodes per arch does not crash arches", "[models][geometry]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("A", "Arches", "NumArches=\"2\" NodesPerArch=\"0\""));
    REQUIRE(m != nullptr);
    CHECK(m->GetNodeCount() == 2);
}

TEST_CASE("Zero sized single line does not crash", "[models][geometry]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("L", "Single Line", "NumStrings=\"0\" NodesPerString=\"0\""));
    REQUIRE(m != nullptr);
    CHECK(m->GetNodeCount() == 1);
}

TEST_CASE("Negative lights per node does not crash a single line", "[models][geometry]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("L", "Single Line", "NumStrings=\"1\" NodesPerString=\"4\" LightsPerNode=\"-5\""));
    REQUIRE(m != nullptr);
    CHECK(m->GetNodeCount() == 4);
}

TEST_CASE("Zero lights per node does not crash a poly line", "[models][geometry]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("P", "Poly Line", "NodesPerString=\"10\" LightsPerNode=\"0\""));
    CHECK(m != nullptr);
}

TEST_CASE("Zero cube dimension does not crash", "[models][geometry]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("C", "Cube", "CubeWidth=\"0\" CubeHeight=\"2\" CubeDepth=\"2\""));
    REQUIRE(m != nullptr);
    CHECK(m->GetNodeCount() > 0);
}

TEST_CASE("Negative node counts do not hang or crash", "[models][geometry]") {
    const char* types[] = { "Tree 360", "Matrix", "Sphere", "Single Line", "Star", "Circle", "Wreath", "Icicles", "Candy Canes", "Arches" };
    ShowFixture f;
    int i = 0;
    for (const char* t : types) {
        INFO(t);
        pugi::xml_document doc;
        REQUIRE(doc.load_string(HostileXml("N" + std::to_string(i++), t).c_str()));
        for (const char* a : { "NodesPerString", "NodesPerArch", "NodesPerCane", "LightsPerNode" }) {
            if (doc.first_child().attribute(a)) {
                doc.first_child().attribute(a).set_value("-5");
            }
        }
        Model* m = f.mm.createAndAddModel(doc.first_child(), 100, 100);
        if (m != nullptr) {
            CHECK(m->GetNodeCount() >= 0);
        }
    }
}

TEST_CASE("Model screen location is read from the XML", "[models][geometry]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("M", "Matrix", "NumStrings=\"1\" NodesPerString=\"4\" StrandsPerString=\"1\" WorldPosX=\"10.5\" WorldPosY=\"-20\" WorldPosZ=\"3\" ScaleX=\"2\" ScaleY=\"3\" ScaleZ=\"4\" RotateX=\"10\" RotateY=\"20\" RotateZ=\"30\" Locked=\"1\""));
    auto& loc = m->GetModelScreenLocation();
    CHECK(loc.GetWorldPos_X() == 10.5f);
    CHECK(loc.GetWorldPos_Y() == -20.0f);
    CHECK(loc.GetWorldPos_Z() == 3.0f);
    CHECK(loc.GetRotateX() == 10.0f);
    CHECK(loc.GetRotateY() == 20.0f);
    CHECK(loc.GetRotateZ() == 30.0f);
    CHECK(loc.IsLocked());
    auto scale = loc.GetScaleMatrix();
    CHECK(scale.x == 2.0f);
    CHECK(scale.y == 3.0f);
    CHECK(scale.z == 4.0f);
}

TEST_CASE("Model screen location defaults when attributes are missing", "[models][geometry]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("M", "Matrix", "NumStrings=\"1\" NodesPerString=\"4\""));
    auto& loc = m->GetModelScreenLocation();
    CHECK(loc.GetWorldPos_X() == 0.0f);
    CHECK(loc.GetRotateZ() == 0.0f);
    CHECK_FALSE(loc.IsLocked());
    auto scale = loc.GetScaleMatrix();
    CHECK(scale.x == 1.0f);
    CHECK(scale.y == 1.0f);
}

TEST_CASE("Two point models read their second point", "[models][geometry]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("L", "Single Line", "NumStrings=\"1\" NodesPerString=\"4\" WorldPosX=\"1\" WorldPosY=\"2\" X2=\"30\" Y2=\"40\" Z2=\"5\""));
    auto* loc = dynamic_cast<TwoPointScreenLocation*>(&m->GetModelScreenLocation());
    REQUIRE(loc != nullptr);
    CHECK(loc->GetX2() == 30.0f);
    CHECK(loc->GetY2() == 40.0f);
    CHECK(loc->GetZ2() == 5.0f);
}

TEST_CASE("Setting the position moves the screen location", "[models][geometry]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("M", "Matrix", "NumStrings=\"1\" NodesPerString=\"4\" WorldPosX=\"1\" WorldPosY=\"2\""));
    m->GetModelScreenLocation().SetWorldPos(7.0f, 8.0f, 9.0f);
    CHECK(m->GetModelScreenLocation().GetWorldPos_X() == 7.0f);
    CHECK(m->GetModelScreenLocation().GetWorldPos_Y() == 8.0f);
    CHECK(m->GetModelScreenLocation().GetWorldPos_Z() == 9.0f);
}

TEST_CASE("Submodel ranges select nodes of the parent", "[models][geometry][submodel]") {
    ShowFixture f;
    Model* t = f.Add(R"(<model name="T" DisplayAs="Tree 360" NumStrings="2" NodesPerString="10" StrandsPerString="1" StartChannel="1">
        <subModel name="h" layout="horizontal" type="ranges" bufferstyle="Default" line0="1-5" line1="6-10"/>
        <subModel name="v" layout="vertical" type="ranges" bufferstyle="Default" line0="1-5" line1="6-10"/>
        <subModel name="mix" layout="horizontal" type="ranges" bufferstyle="Default" line0="1,3,5-7"/>
    </model>)");
    REQUIRE(t->GetNumSubModels() == 3);

    Model* h = t->GetSubModel("h");
    REQUIRE(h != nullptr);
    CHECK(h->GetNodeCount() == 10);
    CHECK(h->GetDefaultBufferWi() == 5);
    CHECK(h->GetDefaultBufferHt() == 2);
    CHECK(BufferPoints(h)[5] == Pt{ 0, 1 });

    Model* v = t->GetSubModel("v");
    REQUIRE(v != nullptr);
    CHECK(v->GetDefaultBufferWi() == 2);
    CHECK(v->GetDefaultBufferHt() == 5);

    Model* mix = t->GetSubModel("mix");
    REQUIRE(mix != nullptr);
    CHECK(mix->GetNodeCount() == 5);
    CHECK(mix->NodeStartChannel(0) == 0);
    CHECK(mix->NodeStartChannel(1) == 6);
    CHECK(mix->NodeStartChannel(2) == 12);
    CHECK(mix->NodeStartChannel(4) == 18);
}

TEST_CASE("Submodels are reachable through the model manager by full name", "[models][geometry][submodel]") {
    ShowFixture f;
    f.Add(R"(<model name="T" DisplayAs="Tree 360" NumStrings="2" NodesPerString="10" StrandsPerString="1" StartChannel="1">
        <subModel name="h" layout="horizontal" type="ranges" bufferstyle="Default" line0="1-5"/>
    </model>)");
    Model* sm = f.mm["T/h"];
    REQUIRE(sm != nullptr);
    CHECK(sm->GetFullName() == "T/h");
    CHECK(sm->GetDisplayAs() == DisplayAsType::SubModel);
    CHECK(f.mm["T/missing"] == nullptr);
    CHECK(f.mm["Missing/h"] == nullptr);
}

TEST_CASE("Submodel with reversed range reverses node order", "[models][geometry][submodel]") {
    ShowFixture f;
    Model* t = f.Add(R"(<model name="T" DisplayAs="Tree 360" NumStrings="1" NodesPerString="10" StrandsPerString="1" StartChannel="1">
        <subModel name="rev" layout="horizontal" type="ranges" bufferstyle="Default" line0="5-1"/>
    </model>)");
    Model* sm = t->GetSubModel("rev");
    REQUIRE(sm != nullptr);
    REQUIRE(sm->GetNodeCount() == 5);
    CHECK(sm->NodeStartChannel(0) == 12);
    CHECK(sm->NodeStartChannel(4) == 0);
}

TEST_CASE("Submodel ranges with invalid entries keep only the valid nodes", "[models][geometry][submodel]") {
    ShowFixture f;
    Model* t = f.Add(R"(<model name="T" DisplayAs="Tree 360" NumStrings="1" NodesPerString="10" StrandsPerString="1" StartChannel="1">
        <subModel name="bad" layout="horizontal" type="ranges" bufferstyle="Default" line0="0,99,abc,3"/>
        <subModel name="empty" layout="horizontal" type="ranges" bufferstyle="Default"/>
    </model>)");
    Model* bad = t->GetSubModel("bad");
    REQUIRE(bad != nullptr);
    CHECK(bad->GetNodeCount() == 1);
    CHECK(bad->NodeStartChannel(0) == 6);
    Model* empty = t->GetSubModel("empty");
    REQUIRE(empty != nullptr);
    CHECK(empty->GetNodeCount() == 0);
}

TEST_CASE("Stacked submodel buffer style stacks lines", "[models][geometry][submodel]") {
    ShowFixture f;
    Model* t = f.Add(R"(<model name="T" DisplayAs="Tree 360" NumStrings="1" NodesPerString="10" StrandsPerString="1" StartChannel="1">
        <subModel name="stk" layout="horizontal" type="ranges" bufferstyle="Stacked - Horizontal" line0="1-2" line1="3-4"/>
    </model>)");
    Model* sm = t->GetSubModel("stk");
    REQUIRE(sm != nullptr);
    CHECK(sm->GetNodeCount() == 4);
    CHECK(sm->GetDefaultBufferWi() == 2);
    CHECK(sm->GetDefaultBufferHt() == 2);
}

TEST_CASE("Subbuffer submodel covering the whole model has every node", "[models][geometry][submodel]") {
    ShowFixture f;
    Model* t = f.Add(R"(<model name="T" DisplayAs="Matrix" NumStrings="1" NodesPerString="12" StrandsPerString="3" StartChannel="1">
        <subModel name="all" layout="horizontal" type="subbuffer" subBuffer="0x0x100x100"/>
    </model>)");
    Model* sm = t->GetSubModel("all");
    REQUIRE(sm != nullptr);
    CHECK(sm->GetNodeCount() == 12);
}

TEST_CASE("Submodel nodes keep the parent channels after the parent moves", "[models][geometry][submodel]") {
    ShowFixture f;
    Model* t = f.Add(R"(<model name="T" DisplayAs="Tree 360" NumStrings="1" NodesPerString="10" StrandsPerString="1" StartChannel="1">
        <subModel name="s" layout="horizontal" type="ranges" bufferstyle="Default" line0="2-3"/>
    </model>)");
    Model* sm = t->GetSubModel("s");
    REQUIRE(sm != nullptr);
    CHECK(sm->NodeStartChannel(0) == 3);
    t->SetStartChannel("101");
    f.mm.RecalcStartChannels();
    sm = t->GetSubModel("s");
    REQUIRE(sm != nullptr);
    CHECK(sm->NodeStartChannel(0) == 103);
}

TEST_CASE("Submodel full name includes the parent name", "[models][geometry][submodel]") {
    ShowFixture f;
    Model* t = f.Add(R"(<model name="Parent" DisplayAs="Tree 360" NumStrings="1" NodesPerString="10" StrandsPerString="1" StartChannel="1">
        <subModel name="Child" layout="horizontal" type="ranges" bufferstyle="Default" line0="1"/>
    </model>)");
    Model* sm = t->GetSubModel("Child");
    REQUIRE(sm != nullptr);
    CHECK(sm->GetFullName() == "Parent/Child");
    auto* concrete = dynamic_cast<SubModel*>(sm);
    REQUIRE(concrete != nullptr);
    CHECK(concrete->GetParent() == t);
    CHECK(concrete->IsRanges());
}
