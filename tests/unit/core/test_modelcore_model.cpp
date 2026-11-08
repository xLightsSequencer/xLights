#include <algorithm>
#include <list>
#include <string>
#include <vector>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <pugixml.hpp>

#include "models/Model.h"
#include "models/ModelManager.h"
#include "models/Node.h"
#include "models/RulerObject.h"
#include "models/TwoPointScreenLocation.h"
#include "models/ViewObjectManager.h"
#include "outputs/OutputManager.h"
#include "XmlSerializer/StringSerializingVisitor.h"

#include "support/TempDir.h"

using Catch::Approx;

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
    Model* Line(const std::string& name, int nodes = 10, const std::string& extra = "") {
        return Add("<model name=\"" + name + "\" DisplayAs=\"Single Line\" StartChannel=\"1\" NumStrings=\"1\" NodesPerString=\"" + std::to_string(nodes) + "\" " + extra + "/>");
    }
    // three horizontal strands of two
    Model* Matrix(const std::string& name = "M", const std::string& extra = "") {
        return Add("<model name=\"" + name + "\" DisplayAs=\"Matrix\" StartChannel=\"1\" NumStrings=\"1\" NodesPerString=\"6\" StrandsPerString=\"3\" WorldPosX=\"500\" WorldPosY=\"500\" " + extra + "/>");
    }
};

bool Has(const std::string& s, const std::string& what) {
    return s.find(what) != std::string::npos;
}

} // namespace

TEST_CASE("Serial protocol speeds depend on the protocol", "[modelcore][model]") {
    ShowFixture f;
    Model* m = f.Line("L");
    struct Case {
        const char* protocol;
        int speed;
        std::vector<std::string> speeds;
        int idx;
    };
    const Case cases[] = {
        { "DMX", 250000, { "250000" }, 0 },
        { "renard", 57600, { "19200", "38400", "57600", "115200" }, 2 },
        { "LOR", 500000, { "19200", "57600", "115200", "500000", "1000000" }, 3 },
        // a LOR Enhanced model with no speed of its own shows the 1M it uploads at
        { "LOR Enhanced", 9600, { "19200", "57600", "115200", "500000", "1000000" }, 4 },
        { "Pixelnet", 250000, { "19200", "57600", "115200", "250000", "500000", "1000000" }, 3 },
    };
    for (const auto& c : cases) {
        INFO(c.protocol);
        m->SetControllerSerialProtocolSpeed(c.speed);
        std::vector<std::string> cs;
        int idx = -1;
        m->GetSerialProtocolSpeeds(c.protocol, cs, idx);
        CHECK(cs == c.speeds);
        CHECK(idx == c.idx);
    }
}

TEST_CASE("Controller protocols without a controller list every pixel type", "[modelcore][model]") {
    ShowFixture f;
    Model* m = f.Line("L");
    m->SetControllerProtocol("ws2811");
    std::vector<std::string> cp;
    int idx = -1;
    m->GetControllerProtocols(cp, idx);
    REQUIRE(idx >= 0);
    CHECK(cp[idx] == "ws2811");
    CHECK(cp.size() > 10);

    // an unknown protocol is blanked: there is nothing compatible to pick
    m->SetControllerProtocol("bogus");
    cp.clear();
    idx = -1;
    m->GetControllerProtocols(cp, idx);
    CHECK(m->GetControllerProtocol().empty());
}

TEST_CASE("Smart remote names", "[modelcore][model]") {
    ShowFixture f;
    Model* m = f.Line("L");
    CHECK(m->GetSmartRemoteValues(3) == std::vector<std::string>{ "A", "B", "C" });
    CHECK(m->DecodeSmartRemote(0) == "None");
    CHECK(m->DecodeSmartRemote(3) == "C");

    Model* four = f.Add(R"(<model name="Q" DisplayAs="Single Line" StartChannel="1" NumStrings="4" NodesPerString="5"/>)");
    CHECK(four->GetNumPhysicalStrings() == 4);
    four->SetSmartRemoteTs(2);
    CHECK(four->GetNumPhysicalStrings() == 2);
    four->SetSmartRemoteTs(8);
    CHECK(four->GetNumPhysicalStrings() == 1);
}

TEST_CASE("Pixel style descriptions", "[modelcore][model]") {
    CHECK(Model::GetPixelStyleDescription(Model::PIXEL_STYLE::PIXEL_STYLE_SQUARE) == "Square");
    CHECK(Model::GetPixelStyleDescription(Model::PIXEL_STYLE::PIXEL_STYLE_BLENDED_CIRCLE) == "Blended Circle");
    CHECK(Model::GetPixelStyleDescription((Model::PIXEL_STYLE)99).empty());
}

TEST_CASE("Model aliases are case insensitive and never the model's own name", "[modelcore][model]") {
    ShowFixture f;
    Model* m = f.Line("Arch");
    CHECK(m->AddAlias("Front Arch"));
    CHECK_FALSE(m->AddAlias("front arch"));
    CHECK_FALSE(m->AddAlias("ARCH"));
    CHECK(m->IsAlias("FRONT ARCH"));
    CHECK(m->GetAliases() == std::list<std::string>{ "front arch" });

    CHECK(m->AddAlias("oldname:Left Arch"));
    CHECK(m->IsAlias("left arch"));
    CHECK(m->IsAlias("Left Arch", true));
    CHECK_FALSE(m->IsAlias("Front Arch", true));

    m->DeleteAlias("Front Arch");
    CHECK_FALSE(m->IsAlias("front arch"));
    m->DeleteAlias("oldname:Left Arch");
    CHECK(m->GetAliases().empty());
    CHECK_FALSE(m->DeleteAllAliases());

    m->SetAliases({ "One", "TWO" });
    CHECK(m->GetAliases() == std::list<std::string>{ "one", "two" });
    CHECK(m->DeleteAllAliases());
    CHECK(m->GetAliases().empty());
}

TEST_CASE("DeleteAlias removes an alias and its old name form", "[modelcore]") {
    ShowFixture f;
    Model* m = f.Line("Arch");
    m->AddAlias("oldname:Left Arch");
    m->AddAlias("Right");
    REQUIRE(m->IsAlias("Left Arch"));
    m->DeleteAlias("Left Arch");
    CHECK_FALSE(m->IsAlias("Left Arch"));
    CHECK(m->IsAlias("Right"));
    m->DeleteAlias("RIGHT");
    CHECK(m->GetAliases().empty());
}

TEST_CASE("Face node ranges are parsed into zero based nodes", "[modelcore][model][faces]") {
    ShowFixture f;
    Model* m = f.Line("L", 20);
    m->AddFace({ { "Name", "Singing" }, { "Type", "NodeRange" }, { "Mouth-AI", "1-3,5" }, { "Eyes-Open", "9-7" }, { "Mouth-AI-Color", "#ff0000" }, { "Mouth-O", "" } });
    const auto& nodes = m->GetFaceInfoNodes();
    REQUIRE(nodes.count("Singing") == 1);
    const auto& face = nodes.at("Singing");
    CHECK(face.at("Mouth-AI") == std::list<int>{ 0, 1, 2, 4 });
    CHECK(face.at("Eyes-Open") == std::list<int>{ 6, 7, 8 });
    CHECK(face.count("Mouth-AI-Color") == 0);
    CHECK(face.count("Mouth-O") == 0);

    // matrix faces have no node ranges
    m->AddFace({ { "Name", "Pictures" }, { "Type", "Matrix" }, { "Mouth-AI", "a.png" } });
    CHECK(m->GetFaceInfoNodes().count("Pictures") == 0);

    std::string s = m->SerialiseFace();
    CHECK(Has(s, "<faceInfo Name=\"Singing\" "));
    CHECK(Has(s, "Mouth-AI=\"1-3,5\""));
    CHECK(Has(s, "<faceInfo Name=\"Pictures\" "));

    pugi::xml_document d;
    auto root = d.append_child("model");
    Model::WriteFaceInfo(root, m->GetFaceInfo());
    int n = 0;
    for (auto fi : root.children("faceInfo")) {
        ++n;
        CHECK(std::string(fi.attribute("Name").as_string()).size() > 0);
    }
    CHECK(n == 2);
}

TEST_CASE("State node ranges and serialisation", "[modelcore][model][faces]") {
    ShowFixture f;
    Model* m = f.Line("L", 20);
    m->AddState({ { "Name", "happy" }, { "Type", "NodeRange" }, { "s1", "2-4" }, { "s1-Name", "on" }, { "s2", "10,12" }, { "s1-Color", "#00ff00" }, { " ", "ignored" } });
    const auto& nodes = m->GetStateInfoNodes();
    REQUIRE(nodes.count("happy") == 1);
    CHECK(nodes.at("happy").at("s1") == std::list<int>{ 1, 2, 3 });
    CHECK(nodes.at("happy").at("s2") == std::list<int>{ 9, 11 });
    CHECK(m->HasState("happy"));
    CHECK(m->HasState("HAPPY"));
    CHECK_FALSE(m->HasState("sad"));

    std::string s = m->SerialiseState();
    CHECK(Has(s, "<stateInfo Name=\"happy\" "));
    CHECK(Has(s, "s1=\"2-4\""));
    CHECK_FALSE(Has(s, "ignored"));

    pugi::xml_document d;
    auto root = d.append_child("model");
    FaceStateData data = m->GetStateInfo();
    data["   "] = { { "x", "1" } }; // blank names are skipped
    Model::WriteStateInfo(root, data, true);
    auto si = root.child("stateInfo");
    REQUIRE(si);
    CHECK(std::string(si.attribute("CustomColors").as_string()) == "1");
    CHECK(std::distance(root.children("stateInfo").begin(), root.children("stateInfo").end()) == 1);

    // a state with an ampersand is escaped
    m->AddState({ { "Name", "a&b" }, { "Type", "NodeRange" }, { "s1", "1" } });
    CHECK(Has(m->SerialiseState(), "a&amp;b"));
}

TEST_CASE("Matrix face files are listed when they exist", "[modelcore][model][faces]") {
    TempDir d;
    std::string have = d.Write("open.png", "x");
    std::string missing = d.File("closed.png");
    ShowFixture f;
    Model* m = f.Line("L");
    // as loaded from a file: the face name is the key, not an attribute
    FaceStateData faces;
    faces["Pics"] = { { "Type", "Matrix" }, { "Mouth-AI", have }, { "Mouth-O", missing }, { "ImagePlacement", "Centered" }, { "CustomColors", "1" } };
    faces["Other"] = { { "Type", "Matrix" }, { "Mouth-AI", have } };
    faces["Ranges"] = { { "Type", "NodeRange" }, { "Mouth-AI", "1-2" } };
    m->SetFaceInfo(faces);

    auto used = m->GetFaceFiles({ "Pics" });
    CHECK(used == std::list<std::string>{ have });
    auto all = m->GetFaceFiles({}, true);
    CHECK(all.size() == 3);
    auto named = m->GetFaceFiles({ "Pics" }, false, true);
    CHECK(named == std::list<std::string>{ "Pics|" + have });
    CHECK(m->GetFaceFiles({ "Nope" }).empty());
}

TEST_CASE("Node XY descriptions parse back to the same cell", "[modelcore][model]") {
    ShowFixture f;
    // a custom model wide enough to need two letter columns (one node: see the
    // knownbug_custom_node_number case for why not more)
    std::string row0 = std::string(28, ',') + "1,";
    std::string row1 = std::string(29, ',');
    Model* c = f.Add("<model name=\"C\" DisplayAs=\"Custom\" StartChannel=\"1\" CustomWidth=\"30\" CustomHeight=\"2\" CustomModel=\"" + row0 + ";" + row1 + "\"/>");
    Model* m = f.Matrix();
    for (Model* model : { c, m }) {
        INFO(model->GetName());
        std::vector<std::string> choices;
        CHECK(model->GetChannelCoords(choices) == model->GetNodeCount());
        for (uint32_t i = 0; i < model->GetNodeCount(); ++i) {
            std::string xy = model->GetNodeXY((int)i);
            INFO(xy);
            CHECK(choices[i] == xy);
            CHECK(xy.starts_with(std::to_string(model->GetNodeNumber(i)) + ": @"));
            std::vector<xlPoint> pts;
            REQUIRE(Model::ParseFaceElement(xy, pts));
            REQUIRE(pts.size() == 1);
            const auto& co = model->GetNode(i)->Coords[0];
            CHECK(pts[0].x == co.bufX + 1);
            CHECK(pts[0].y == model->GetDefaultBufferHt() - co.bufY);
            // lookup by node number gives the same text
            CHECK(model->GetNodeXY(std::to_string(model->GetNodeNumber(i))) == xy);
        }
        CHECK(model->GetNodeXY(-1).empty());
        CHECK(model->GetNodeXY((int)model->GetNodeCount()).empty());
        CHECK(model->GetNodeXY(std::string("999")) == "999");
    }
    // rows count down from the top, as in the custom model grid
    CHECK(c->GetNodeXY(0) == "1: @AC1");
}

TEST_CASE("Custom model node numbers follow the custom grid", "[modelcore]") {
    ShowFixture f;
    Model* c = f.Add(R"(<model name="C" DisplayAs="Custom" StartChannel="1" CustomWidth="3" CustomHeight="1" CustomModel="1,2,3"/>)");
    REQUIRE(c->GetNodeCount() == 3);
    for (uint32_t i = 0; i < 3; ++i) {
        CHECK(c->GetNodeNumber(i) == i + 1);
    }
}

TEST_CASE("Face and state element parsing", "[modelcore][model][faces]") {
    std::vector<xlPoint> pts;
    CHECK(Model::ParseFaceElement("1: @A1+2: @(3,4)+junk+5: @B+6: @C2x+7: @(5+8: @(6,7", pts));
    REQUIRE(pts.size() == 2);
    CHECK(pts[0].x == 1);
    CHECK(pts[0].y == 1);
    CHECK(pts[1].x == 3);
    CHECK(pts[1].y == 4);
    std::vector<xlPoint> none;
    CHECK_FALSE(Model::ParseFaceElement("", none));
    CHECK_FALSE(Model::ParseFaceElement("1: @", none));

    std::vector<xlPoint> st;
    CHECK(Model::ParseStateElement("3: @AB12-AC13+4: @(10,20)=(11,21)", st));
    REQUIRE(st.size() == 2);
    CHECK(st[0].x == 28);
    CHECK(st[0].y == 12);
    CHECK(st[1].x == 10);
    CHECK(st[1].y == 20);
    std::vector<xlPoint> bad;
    CHECK_FALSE(Model::ParseStateElement("x+@+@Q+@(1+@(1,2", bad));
}

TEST_CASE("Model exports as a custom model that reimports to the same layout", "[modelcore][model][export]") {
    ShowFixture f;
    Model* m = f.Matrix();
    m->SetPixelCount("6");
    m->SetPixelType("WS2811");
    m->SetPixelSpacing("3in");
    StringSerializingVisitor v(false, false);
    m->ExportAsCustomXModel(v);
    pugi::xml_document d;
    REQUIRE(d.load_string(v.GetResult().c_str()));
    auto cm = d.child("custommodel");
    REQUIRE(cm);
    CHECK(std::string(cm.attribute("PixelCount").as_string()) == "6");
    CHECK(std::string(cm.attribute("PixelSpacing").as_string()) == "3in");
    int w = cm.attribute("CustomWidth").as_int();
    int h = cm.attribute("CustomHeight").as_int();
    CHECK(w * h >= 6);
    // the default render buffer travels as a submodel, one line per row
    auto sm = cm.child("subModel");
    REQUIRE(sm);
    CHECK(std::string(sm.attribute("name").as_string()) == "DefaultRenderBuffer");
    int lines = 0;
    for (int y = 0; y < m->GetDefaultBufferHt(); ++y) {
        if (sm.attribute(("line" + std::to_string(y)).c_str())) ++lines;
    }
    CHECK(lines == m->GetDefaultBufferHt());

    // re-import through the saved model path
    bool cancelled = false;
    Model* base = f.mm.CreateDefaultModel("Custom", "1");
    Model* imported = m->CreateDefaultModelFromSavedModelNode(base, cm, f.mm, cancelled);
    REQUIRE(imported != nullptr);
    CHECK_FALSE(cancelled);
    REQUIRE(imported->GetNodeCount() == 6);
    // relative positions of the nodes are kept
    for (uint32_t i = 1; i < 6; ++i) {
        const auto& a = m->GetNode(i)->Coords[0];
        const auto& a0 = m->GetNode(0)->Coords[0];
        const auto& b = imported->GetNode(i)->Coords[0];
        const auto& b0 = imported->GetNode(0)->Coords[0];
        CHECK((a.screenX > a0.screenX) == (b.bufX > b0.bufX));
        CHECK((a.screenY > a0.screenY) == (b.bufY > b0.bufY));
    }
    delete imported;
}

TEST_CASE("Default render buffer as a submodel lists each row", "[modelcore][model][export]") {
    ShowFixture f;
    Model* m = f.Matrix();
    std::string xml = m->CreateBufferAsSubmodel();
    pugi::xml_document d;
    REQUIRE(d.load_string(xml.c_str()));
    auto sm = d.child("subModel");
    REQUIRE(sm);
    CHECK(std::string(sm.attribute("type").as_string()) == "ranges");
    std::vector<int> seen;
    for (int y = 0; y < m->GetDefaultBufferHt(); ++y) {
        std::string line = sm.attribute(("line" + std::to_string(y)).c_str()).as_string();
        CHECK_FALSE(line.empty());
        for (uint32_t i = 0; i < m->GetNodeCount(); ++i) {
            if (m->GetNode(i)->Coords[0].bufY == y) seen.push_back(i);
        }
    }
    CHECK(seen.size() == 6);
}

TEST_CASE("Submodels serialise with their ranges and aliases", "[modelcore][model]") {
    ShowFixture f;
    Model* m = f.Add(R"(<model name="L" DisplayAs="Single Line" StartChannel="1" NumStrings="1" NodesPerString="10">
        <subModel name="Left" layout="horizontal" type="ranges" line0="1-5"/>
        <subModel name="Buf" layout="horizontal" type="subbuffer" subBuffer="0x0x50x100"/>
    </model>)");
    REQUIRE(m->GetNumSubModels() == 2);
    m->GetSubModel("Left")->AddAlias("west");
    std::string s = m->SerialiseSubmodel();
    INFO(s);
    CHECK(Has(s, "<subModel name=\"Left\" layout=\"horizontal\" type=\"ranges\" bufferstyle=\"Default\" line0=\"1-5\">"));
    CHECK(Has(s, "<alias name=\"west\"/>"));
    CHECK(Has(s, "<subModel name=\"Buf\""));
    CHECK(Has(s, "subBuffer=\"0x0x50x100\" />"));

    CHECK(m->GenerateUniqueSubmodelName("New") == "New");
    CHECK(m->GenerateUniqueSubmodelName("Left") == "Left_2");
    m->RemoveSubModel("Nope");
    CHECK(m->GetNumSubModels() == 2);
    m->RemoveSubModel("Left");
    CHECK(m->GetNumSubModels() == 1);
    CHECK(m->GetSubModel("Left") == nullptr);
    CHECK(m->GetSubModel(0) == m->GetSubModel("Buf"));
    CHECK(m->GetSubModel(5) == nullptr);
    m->RemoveAllSubModels();
    CHECK(m->GetNumSubModels() == 0);
}

TEST_CASE("Channel colour letters", "[modelcore][model]") {
    CHECK(Model::EncodeColour(xlColor(9, 0, 0)) == 'R');
    CHECK(Model::EncodeColour(xlColor(0, 9, 0)) == 'G');
    CHECK(Model::EncodeColour(xlColor(0, 0, 9)) == 'B');
    CHECK(Model::EncodeColour(xlColor(7, 7, 7)) == 'W');
    CHECK(Model::EncodeColour(xlColor(7, 7, 0)) == 'X');
    CHECK(Model::EncodeColour(xlBLACK) == 'X');

    ShowFixture f;
    Model* m = f.Add(R"(<model name="L" DisplayAs="Single Line" StartChannel="11" NumStrings="1" NodesPerString="2" StringType="GRB Nodes"/>)");
    CHECK(m->GetChannelColorLetter(0) == 'G');
    CHECK(m->GetAbsoluteChannelColorLetter(10) == ' ');
    CHECK(m->GetAbsoluteChannelColorLetter(11) == 'G');
    CHECK(m->GetAbsoluteChannelColorLetter(12) == 'R');
    CHECK(m->GetAbsoluteChannelColorLetter(13) == 'B');
    CHECK(m->GetAbsoluteChannelColorLetter(14) == 'G');
    CHECK(m->GetAbsoluteChannelColorLetter(17) == ' ');

    Model* s = f.Add(R"(<model name="S" DisplayAs="Single Line" StartChannel="1" NumStrings="1" NodesPerString="3" StringType="Single Color Blue"/>)");
    CHECK(s->GetAbsoluteChannelColorLetter(1) == 'B');
}

TEST_CASE("Controller connection descriptions", "[modelcore][model]") {
    ShowFixture f;
    Model* m = f.Add(R"(<model name="L" DisplayAs="Single Line" StartChannel="1" NumStrings="2" NodesPerString="5"/>)");
    CHECK(m->GetControllerConnectionString().empty());
    CHECK(m->GetControllerConnectionRangeString().empty());
    m->SetControllerProtocol("ws2811");
    m->SetControllerPort(3);
    CHECK(m->GetControllerConnectionString().starts_with("ws2811:3"));
    CHECK(m->GetControllerConnectionRangeString().starts_with("ws2811:3-4"));
    CHECK(m->GetControllerConnectionPortRangeString() == "3-4");

    m->SetControllerBrightness(50);
    m->SetControllerProperty(ControllerConnection::CTRL_PROPS::BRIGHTNESS_ACTIVE);
    std::string attrs = m->GetControllerConnectionAttributeString();
    INFO(attrs);
    CHECK(Has(attrs, ":Brightness=50"));
    CHECK_FALSE(Has(attrs, "Protocol="));
    CHECK(Has(m->GetControllerConnectionString(), ":Brightness=50"));

    m->SetControllerPort(0);
    CHECK(m->GetControllerConnectionRangeString().starts_with("ws2811:"));
    CHECK_FALSE(m->GetControllerConnectionRangeString().starts_with("ws2811:0"));
    CHECK(m->GetControllerConnectionPortRangeString() == "0");

    // with no controller the sort key puts the model last
    std::string key = m->GetControllerPortSortString();
    CHECK(key.starts_with(std::string(140, 'Z') + ":00000000:"));
    m->SetControllerName("Ctl", true);
    m->SetControllerPort(7);
    CHECK(m->GetControllerPortSortString() == "Ctl:00000007:00000000");
}

TEST_CASE("Model chain names are normalised", "[modelcore][model]") {
    ShowFixture f;
    Model* m = f.Line("L");
    m->SetModelChain("Other");
    CHECK(m->GetModelChain() == ">Other");
    m->SetModelChain(">Other");
    CHECK(m->GetModelChain() == ">Other");
    m->SetModelChain("Beginning");
    CHECK(m->GetModelChain().empty());
    m->SetModelChain(">");
    CHECK(m->GetModelChain().empty());
}

TEST_CASE("Super string colours fill in the primaries", "[modelcore][model]") {
    ShowFixture f;
    Model* m = f.Line("L");
    CHECK(m->ExportSuperStringColors().empty());
    m->SetSuperStringColours(7);
    REQUIRE(m->GetNumSuperStringColours() == 7);
    const auto& c = m->GetSuperStringColours();
    CHECK(c[0] == xlRED);
    CHECK(c[1] == xlGREEN);
    CHECK(c[2] == xlBLUE);
    CHECK(c[3] == xlWHITE);
    CHECK(c[4] == xlYELLOW);
    CHECK(c[5] == xlRED);
    m->SetSuperStringColours(2);
    CHECK(m->GetNumSuperStringColours() == 2);
    m->SetSuperStringColour(1, xlColor(1, 2, 3));
    m->AddSuperStringColour(xlCYAN);
    std::string ex = m->ExportSuperStringColors();
    CHECK(Has(ex, "SuperStringColour0=\"#ff0000\""));
    CHECK(Has(ex, "SuperStringColour1=\"#010203\""));
    CHECK(Has(ex, "SuperStringColour2=\"#00ffff\""));

    // the exported attributes load back onto a model
    Model* n = f.Line("N", 10, "StringType=\"Superstring\" " + ex);
    REQUIRE(n->GetNumSuperStringColours() == 3);
    CHECK(n->GetSuperStringColours()[1] == xlColor(1, 2, 3));
    n->InitSuperStringColours();
    CHECK(n->GetNumSuperStringColours() == 3);
}

TEST_CASE("Model tag colour", "[modelcore][model]") {
    ShowFixture f;
    Model* m = f.Line("L");
    CHECK(m->GetTagColour() == xlBLACK);
    CHECK(m->GetTagColourAsString() == "#000000");
    m->SetTagColourAsString("#112233");
    CHECK(m->GetTagColour() == xlColor(0x11, 0x22, 0x33));
    m->SetTagColour(xlRED);
    CHECK(m->GetTagColourAsString() == "#ff0000");
    m->SetModelTagColour(xlBLUE);
    CHECK(m->GetTagColour() == xlBLUE);
}

TEST_CASE("Model RGBW handling", "[modelcore][model]") {
    ShowFixture f;
    Model* m = f.Line("L", 2, "StringType=\"RGBW Nodes\"");
    CHECK(m->FourChannelNodes());
    CHECK_FALSE(m->FiveChannelNodes());
    std::string first = m->GetRGBWHandling();
    m->SetRGBWHandling("not a mode");
    CHECK(m->GetRGBWHandling() == first);
    m->SetRGBWHandling("Advanced");
    CHECK(m->GetRGBWHandling() == "Advanced");
}

TEST_CASE("Ruler dimensions size a model in real units", "[modelcore][model][ruler]") {
    ShowFixture f;
    Model* m = f.Matrix();
    CHECK(m->GetRulerDim().empty());
    {
        ViewObjectManager vom(nullptr);
        auto* r = dynamic_cast<RulerObject*>(vom.CreateAndAddObject("Ruler"));
        REQUIRE(r != nullptr);
        auto& loc = static_cast<TwoPointScreenLocation&>(r->GetObjectScreenLocation());
        loc.SetX2(100.0f); // 100 units = 2 m
        r->SetLength(2.0f);
        r->SetUnits(RULER_UNITS_CM);
        CHECK(m->GetRulerDim() == "cm");
        for (auto [u, s] : { std::pair{ RULER_UNITS_INCHES, "i" }, std::pair{ RULER_UNITS_FEET, "f" }, std::pair{ RULER_UNITS_YARDS, "y" }, std::pair{ RULER_UNITS_MM, "mm" }, std::pair{ RULER_UNITS_M, "m" } }) {
            r->SetUnits(u);
            CHECK(m->GetRulerDim() == s);
        }
        r->SetUnits(RULER_UNITS_M);
        // 1 m wide = 50 units, 0.5 m high = 25 units
        m->ApplyDimensions("m", 1.0f, 0.5f, 0.0f);
        CHECK(m->GetModelScreenLocation().GetRestorableMWidth() == Approx(50.0f).epsilon(0.01));
        CHECK(m->GetModelScreenLocation().GetRestorableMHeight() == Approx(25.0f).epsilon(0.01));
        // centimetres are converted to the ruler's units first
        m->ApplyDimensions("cm", 200.0f, 100.0f, 0.0f);
        CHECK(m->GetModelScreenLocation().GetRestorableMWidth() == Approx(100.0f).epsilon(0.01));
        // a zero width leaves it alone
        m->ApplyDimensions("cm", 0.0f, 100.0f, 0.0f);
        CHECK(m->GetModelScreenLocation().GetRestorableMWidth() == Approx(100.0f).epsilon(0.01));
    }
    CHECK(m->GetRulerDim().empty());
}

TEST_CASE("Display dimensions can be saved and restored", "[modelcore][model]") {
    ShowFixture f;
    Model* m = f.Matrix();
    m->SetWidth(40.0f, true);
    m->SetHeight(30.0f, true);
    float w = m->GetWidth();
    float h = m->GetHeight();
    m->SaveDisplayDimensions();
    m->SetWidth(200.0f, true);
    m->SetHeight(10.0f, true);
    m->RestoreDisplayDimensions();
    CHECK(m->GetWidth() == Approx(w).epsilon(0.01));
    CHECK(m->GetHeight() == Approx(h).epsilon(0.01));
}

TEST_CASE("Copying geometry takes size, position and rotation", "[modelcore][model]") {
    ShowFixture f;
    Model* a = f.Matrix("A", "ScaleX=\"3\" ScaleY=\"2\" RotateZ=\"30\"");
    Model* b = f.Matrix("B", "WorldPosX=\"100\" WorldPosY=\"100\"");
    b->CopyGeometryFrom(*a);
    CHECK(b->GetHcenterPos() == Approx(a->GetHcenterPos()));
    CHECK(b->GetVcenterPos() == Approx(a->GetVcenterPos()));
    CHECK(b->GetModelScreenLocation().GetRotation().z == Approx(30.0f));
    CHECK(b->GetWidth() == Approx(a->GetWidth()));

    Model* l1 = f.Add(R"(<model name="L1" DisplayAs="Single Line" StartChannel="1" NumStrings="1" NodesPerString="5" WorldPosX="10" WorldPosY="20" X2="80" Y2="40"/>)");
    Model* l2 = f.Line("L2", 5);
    l2->CopyGeometryFrom(*l1);
    CHECK(l2->GetHcenterPos() == Approx(l1->GetHcenterPos()).margin(0.01));
    CHECK(l2->GetVcenterPos() == Approx(l1->GetVcenterPos()).margin(0.01));
    CHECK(l2->GetWidth() == Approx(l1->GetWidth()).margin(0.01));
}

TEST_CASE("Renaming a model updates shadows and chains that name it", "[modelcore][model]") {
    ShowFixture f;
    f.Line("Old");
    Model* m = f.Line("M");
    m->SetShadowModelFor("Old", false);
    m->SetModelChain(">Old");
    CHECK(m->IsShadowModel());
    CHECK(m->ModelRenamed("Old", "New"));
    CHECK(m->GetShadowModelFor() == "New");
    CHECK(m->GetModelChain() == ">New");
    CHECK_FALSE(m->ModelRenamed("Nobody", "Else"));
    // a model is never its own shadow
    m->SetShadowModelFor("M");
    CHECK(m->GetShadowModelFor() == "New");
}

TEST_CASE("Picking a shadow target moves the target onto this model", "[modelcore][model]") {
    ShowFixture f;
    Model* target = f.Line("T");
    Model* m = f.Line("M");
    m->SetShadowModelFor("T");
    CHECK(target->GetModelStartChannel() == "@M:1");
    CHECK(target->GetControllerName().empty());
}

TEST_CASE("Renaming a model rewrites individual string start channels", "[modelcore]") {
    ShowFixture f;
    f.Line("Old", 20);
    Model* m = f.Add(R"(<model name="M" DisplayAs="Single Line" StartChannel="@Old:1" NumStrings="2" NodesPerString="5" Advanced="1" String1="@Old:1" String2="@Old:31"/>)");
    REQUIRE(m->HasIndividualStartChannels());
    m->ModelRenamed("Old", "New");
    CHECK(m->GetModelStartChannel() == "@New:1");
    CHECK(m->GetIndividualStartChannel(0) == "@New:1");
    CHECK(m->GetIndividualStartChannel(1) == "@New:31");
}

TEST_CASE("Base channel layout lists node and string per cell", "[modelcore][model]") {
    ShowFixture f;
    Model* m = f.Add(R"(<model name="L" DisplayAs="Single Line" StartChannel="1" NumStrings="2" NodesPerString="3"/>)");
    std::string html = m->ChannelLayoutHtml(&f.om);
    CHECK(Has(html, "Name:</td><td>L<"));
    CHECK(Has(html, "Total nodes:</td><td>6<"));
    for (int n = 1; n <= 3; ++n) {
        CHECK(Has(html, "n" + std::to_string(n) + "s1<"));
        CHECK(Has(html, "n" + std::to_string(n) + "s2<"));
    }
    CHECK(Has(html, "#ADD8E6"));
    CHECK(Has(html, "#90EE90"));
    m->SetControllerProtocol("ws2811");
    m->SetControllerPort(2);
    std::string h2 = m->ChannelLayoutHtml(&f.om, true);
    CHECK(Has(h2, "Pixel protocol:</td><td>ws2811"));
    CHECK(Has(h2, "Controller Connections:</td><td>2-3"));
    CHECK(Has(h2, "#3F7C85"));

    Model* one = f.Line("One", 2);
    one->SetControllerProtocol("ws2811");
    one->SetControllerPort(5);
    CHECK(Has(one->ChannelLayoutHtml(&f.om), "Controller Connection:</td><td>5"));
    CHECK(Has(one->ChannelLayoutHtml(&f.om), "Start Corner:</td><td>"));
}

TEST_CASE("Start location follows direction and start side", "[modelcore][model]") {
    ShowFixture f;
    Model* m = f.Line("L");
    m->SetIsLtoR(true);
    m->SetIsBtoT(true);
    CHECK(m->GetStartLocation() == "Bottom Left");
    m->SetIsBtoT(false);
    CHECK(m->GetStartLocation() == "Top Left");
    m->SetIsLtoR(false);
    CHECK(m->GetStartLocation() == "Top Right");
    m->SetIsBtoT(true);
    CHECK(m->GetStartLocation() == "Bottom Right");
}

TEST_CASE("Node buffer range checks", "[modelcore][model]") {
    ShowFixture f;
    Model* m = f.Matrix();
    const auto& c = m->GetNode(3)->Coords[0];
    CHECK(m->IsNodeInBufferRange(3, c.bufX, c.bufY, c.bufX, c.bufY));
    CHECK_FALSE(m->IsNodeInBufferRange(3, c.bufX + 1, c.bufY, c.bufX + 5, c.bufY + 5));
    CHECK_FALSE(m->IsNodeInBufferRange(99, 0, 0, 100, 100));
}

TEST_CASE("LOR Enhanced runs carry a string across unit IDs", "[modelcore][model][lor]") {
    ShowFixture f;
    Model* m = f.Line("L", 200);
    m->SetControllerProtocol("LOR Enhanced");
    auto runs = m->GetLOREnhancedRuns(1);
    REQUIRE(runs.size() == 2);
    CHECK(runs[0].unit == 1);
    CHECK(runs[0].startPixel == 1);
    CHECK(runs[0].pixels == 170);
    CHECK(runs[0].modelPixel == 0);
    CHECK(runs[1].unit == 2);
    CHECK(runs[1].startPixel == 1);
    CHECK(runs[1].pixels == 30);
    CHECK(runs[1].modelPixel == 170);
    CHECK(m->GetSerialChannelSpan(1) == 600);

    // starting part way into a unit
    auto mid = m->GetLOREnhancedRuns(301);
    REQUIRE(mid.size() == 2);
    CHECK(mid[0].unit == 1);
    CHECK(mid[0].startPixel == 101);
    CHECK(mid[0].pixels == 70);
    CHECK(mid[1].pixels == 130);
    CHECK(m->GetSerialChannelSpan(301) == 600);

    // other protocols just take their channels
    m->SetControllerProtocol("DMX");
    CHECK(m->GetSerialChannelSpan(1) == 600);
}

TEST_CASE("LOR Enhanced strings start on their own port's unit ID", "[modelcore][model][lor]") {
    ShowFixture f;
    Model* m = f.Add(R"(<model name="L" DisplayAs="Single Line" StartChannel="1" NumStrings="2" NodesPerString="100"/>)");
    m->SetControllerProtocol("LOR Enhanced");
    m->SetControllerLORPortPixels(170);
    auto runs = m->GetLOREnhancedRuns(1);
    REQUIRE(runs.size() == 2);
    CHECK(runs[1].unit == 2);
    CHECK(runs[1].string == 1);
    CHECK(runs[1].modelPixel == 100);
    // ports of 340 pixels have two unit IDs each
    m->SetControllerLORPortPixels(340);
    auto runs2 = m->GetLOREnhancedRuns(1);
    REQUIRE(runs2.size() == 2);
    CHECK(runs2[1].unit == 3);
    // the span runs to the last pixel of the last unit used
    CHECK(m->GetSerialChannelSpan(1) == 2 * 510 + 300);
}

TEST_CASE("Model class for import mapping", "[modelcore][model]") {
    CHECK(Model::DetermineClass("Custom", true, false, false, "") == "SingingFace");
    CHECK(Model::DetermineClass("Custom", false, false, false, "") == "");
    CHECK(Model::DetermineClass("Candy Canes", false, false, true, "") == "Sticks");
    CHECK(Model::DetermineClass("Candy Canes", false, false, false, "") == "Line");
    CHECK(Model::DetermineClass("Poly Line", false, false, false, "-3,2") == "Sticks");
    CHECK(Model::DetermineClass("Poly Line", false, false, false, "0,4") == "Icicles");
    CHECK(Model::DetermineClass("Poly Line", false, false, false, "") == "Line");
    CHECK(Model::DetermineClass("Icicles", false, false, false, "") == "Icicles");
    CHECK(Model::DetermineClass("Tree 360", false, true, false, "") == "Line");
    CHECK(Model::DetermineClass("Tree 360", false, false, false, "") == "Matrix");
    CHECK(Model::DetermineClass("Horiz Matrix", false, false, false, "") == "Matrix");
    CHECK(Model::DetermineClass("Window Frame", false, false, false, "") == "Line");
    CHECK(Model::DetermineClass("Image", false, false, false, "") == "Pixel");
    CHECK(Model::DetermineClass("Channel Block", false, false, false, "") == "Pixel");
    CHECK(Model::DetermineClass("DmxMovingHeadAdv", false, false, false, "") == "Moving Head");
    CHECK(Model::DetermineClass("DmxFloodArea", false, false, false, "") == "Floodlight");
    CHECK(Model::DetermineClass("DmxSkull", false, false, false, "") == "DMX Special Purpose");
    CHECK(Model::DetermineClass("Star", false, false, false, "") == "");
}

TEST_CASE("A poly line without drops is a line", "[modelcore]") {
    CHECK(Model::DetermineClass("Poly Line", false, false, false, "1") == "Line");
    CHECK(Model::DetermineClass("Poly Line", false, false, false, "1,-1,1") == "Line");
    CHECK(Model::DetermineClass("Poly Line", false, false, false, "1,2") == "Icicles");
}

TEST_CASE("Model settings check flags a blanking dimming curve", "[modelcore][model]") {
    ShowFixture f;
    Model* m = f.Line("L");
    m->SetDimmingInfo({ { "all", { { "gamma", "0.0" }, { "brightness", "-100" } } } });
    auto errs = m->CheckModelSettings();
    bool gamma = false, bright = false;
    for (auto& e : errs) {
        gamma |= Has(e, "dimming curve gamma of 0.0");
        bright |= Has(e, "dimming curve brightness of -100");
    }
    CHECK(gamma);
    CHECK(bright);

    m->SetDimmingInfo({ { "all", { { "gamma", "1.0" }, { "brightness", "0" } } } });
    for (auto& e : m->CheckModelSettings()) {
        CHECK_FALSE(Has(e, "dimming curve"));
    }
    CHECK(m->GetDimmingInfo().size() == 1);
}

TEST_CASE("Model attributes as JSON", "[modelcore][model]") {
    ShowFixture f;
    Model* m = f.Line("Front Line", 4);
    std::string j = m->GetAttributesAsJSON();
    CHECK(j.starts_with("{"));
    CHECK(Has(j, "Front Line"));
    CHECK(Has(j, "Single Line"));
}

TEST_CASE("Node and strand names", "[modelcore][model]") {
    ShowFixture f;
    Model* m = f.Line("L", 4);
    m->SetNodeNames("a,,c");
    CHECK(m->GetNodeNames() == "a,,c");
    CHECK(m->GetNodeName(0) == "a");
    CHECK(m->GetNodeName(1).empty());
    CHECK(m->GetNodeName(2) == "c");
    CHECK(m->GetNodeName(3).empty());
    CHECK(m->GetNodeName(3, true) == "Node 4");
    m->SetStrandNames(",second");
    CHECK(m->GetStrandNames() == ",second");
    CHECK(m->GetStrandName(0).empty());
    CHECK(m->GetStrandName(1) == "second");
    CHECK(m->GetStrandName(5, true) == "Strand 6");
}
