#include <functional>
#include <list>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <pugixml.hpp>

#include "XmlSerializer/StringSerializingVisitor.h"
#include "XmlSerializer/XmlSerializer.h"
#include "models/CustomModel.h"
#include "models/Model.h"
#include "models/ModelGroup.h"
#include "models/ModelManager.h"
#include "models/SubModel.h"
#include "outputs/OutputManager.h"

namespace {

struct ShowFixture {
    OutputManager om;
    ModelManager mm{ &om };

    Model* Add(const std::string& xml, bool recalc = true) {
        pugi::xml_document doc;
        REQUIRE(doc.load_string(xml.c_str()));
        Model* m = mm.createAndAddModel(doc.first_child(), 100, 100);
        if (recalc) {
            mm.RecalcStartChannels();
        }
        return m;
    }
};

std::string SerializeModel(const Model* m, bool exporting = false) {
    pugi::xml_document doc;
    XmlSerializingVisitor v(&doc, exporting);
    m->Accept(v);
    std::ostringstream os;
    doc.save(os, "", pugi::format_raw);
    return os.str();
}

bool SerializeToDoc(const Model* m, pugi::xml_document& doc) {
    XmlSerializingVisitor v(&doc);
    m->Accept(v);
    return static_cast<bool>(doc.first_child());
}

std::string ModelXml(const std::string& name, const std::string& displayAs, const std::string& attrs) {
    return "<model name=\"" + name + "\" DisplayAs=\"" + displayAs + "\" StartChannel=\"1\" " + attrs + "/>";
}

std::string RoundTrip(const std::string& xml, std::string* before = nullptr) {
    ShowFixture a;
    Model* m = a.Add(xml);
    REQUIRE(m != nullptr);
    std::string first = SerializeModel(m);
    if (before != nullptr) {
        *before = first;
    }
    pugi::xml_document doc;
    REQUIRE(SerializeToDoc(m, doc));
    ShowFixture b;
    Model* m2 = b.mm.createAndAddModel(doc.first_child(), 100, 100);
    REQUIRE(m2 != nullptr);
    b.mm.RecalcStartChannels();
    return SerializeModel(m2);
}

} // namespace

TEST_CASE("Serialize then deserialize reproduces every model type", "[models][serialization]") {
    const std::vector<std::string> models = {
        ModelXml("m", "Tree 360", "NumStrings=\"3\" NodesPerString=\"10\" StrandsPerString=\"2\" TreeRotation=\"4.5\" TreePerspective=\"0.5\" TreeBottomTopRatio=\"3\""),
        ModelXml("m", "Matrix", "NumStrings=\"2\" NodesPerString=\"12\" StrandsPerString=\"3\" NoZig=\"true\" Vertical=\"true\" AlternateNodes=\"true\""),
        ModelXml("m", "Arches", "NumArches=\"3\" NodesPerArch=\"20\" LightsPerNode=\"2\" Hollow=\"50\" Gap=\"3\" Arc=\"90\" ZigZag=\"true\" LayerSizes=\"5,10\""),
        ModelXml("m", "Single Line", "NumStrings=\"2\" NodesPerString=\"7\" LightsPerNode=\"1\" X2=\"50\" Y2=\"10\""),
        ModelXml("m", "Star", "NumStrings=\"1\" NodesPerString=\"20\" StarPoints=\"6\" starRatio=\"2.5\" LayerSizes=\"5,15\""),
        ModelXml("m", "Circle", "NumStrings=\"2\" NodesPerString=\"16\" InsideOut=\"1\" centerPercent=\"10\" LayerSizes=\"8,8\""),
        ModelXml("m", "Icicles", "NumStrings=\"2\" NodesPerString=\"12\" DropPattern=\"3,5,2\" AlternateNodes=\"true\""),
        ModelXml("m", "Candy Canes", "NumCanes=\"4\" NodesPerCane=\"12\" LightsPerNode=\"1\" CandyCaneSticks=\"true\" CandyCaneReverse=\"true\" CandyCaneHeight=\"1.5\""),
        ModelXml("m", "Spinner", "NumStrings=\"2\" NodesPerArm=\"5\" ArmsPerString=\"4\" Hollow=\"30\" StartAngle=\"45\" Arc=\"270\" Alternate=\"true\""),
        ModelXml("m", "Wreath", "NumStrings=\"1\" NodesPerString=\"30\""),
        ModelXml("m", "Window Frame", "TopNodes=\"8\" SideNodes=\"12\" BottomNodes=\"8\" Rotation=\"CCW\""),
        ModelXml("m", "Sphere", "NumStrings=\"1\" NodesPerString=\"20\" StrandsPerString=\"4\" StartLatitude=\"-60\" EndLatitude=\"60\" Degrees=\"270\" versionNumber=\"8\""),
        ModelXml("m", "Cube", "CubeWidth=\"3\" CubeHeight=\"4\" CubeDepth=\"5\" Strings=\"2\" Style=\"Horizontal Left/Right\""),
        ModelXml("m", "Custom", "CustomWidth=\"4\" CustomHeight=\"3\" CustomModel=\"1,,2,;,3,,4;5,,,6\" CustomStrings=\"1\""),
        ModelXml("m", "Channel Block", "NumChannels=\"5\" StringType=\"Single Color Custom\""),
        ModelXml("m", "Label", "LabelText=\"Hello\" LabelFontSize=\"20\" LabelTextColor=\"#00FF00\""),
        ModelXml("m", "Image", "Image=\"picture.png\" WhiteAsAlpha=\"True\" OffBrightness=\"60\""),
        ModelXml("m", "DmxGeneral", "DmxChannelCount=\"10\" StringType=\"Single Color White\""),
        ModelXml("m", "DmxFloodlight", "DmxChannelCount=\"3\" StringType=\"Single Color White\""),
        ModelXml("m", "DmxServo", "DmxChannelCount=\"2\" StringType=\"Single Color White\""),
        ModelXml("m", "DmxSkull", "DmxChannelCount=\"26\" StringType=\"Single Color White\""),
        ModelXml("m", "Poly Line", "NodesPerString=\"20\" NumPoints=\"3\" PointData=\"0,0,0,10,10,0,20,0,0\" LightsPerNode=\"1\""),
    };
    for (const auto& xml : models) {
        INFO(xml);
        std::string first;
        std::string second = RoundTrip(xml, &first);
        CHECK(first == second);
        CHECK_FALSE(first.empty());
    }
}

TEST_CASE("Round trip preserves node and channel counts", "[models][serialization]") {
    const std::vector<std::pair<std::string, uint32_t>> models = {
        { ModelXml("m", "Tree 360", "NumStrings=\"3\" NodesPerString=\"10\" StrandsPerString=\"1\""), 30 },
        { ModelXml("m", "Matrix", "NumStrings=\"2\" NodesPerString=\"12\" StrandsPerString=\"3\""), 24 },
        { ModelXml("m", "Arches", "NumArches=\"3\" NodesPerArch=\"20\""), 60 },
        { ModelXml("m", "Candy Canes", "NumCanes=\"4\" NodesPerCane=\"12\""), 48 },
        { ModelXml("m", "Custom", "CustomWidth=\"4\" CustomHeight=\"3\" CustomModel=\"1,,2,;,3,,4;5,,,6\""), 6 },
    };
    for (const auto& [xml, nodes] : models) {
        INFO(xml);
        ShowFixture a;
        Model* m = a.Add(xml);
        REQUIRE(m->GetNodeCount() == nodes);
        pugi::xml_document doc;
        REQUIRE(SerializeToDoc(m, doc));
        ShowFixture b;
        Model* m2 = b.mm.createAndAddModel(doc.first_child(), 100, 100);
        REQUIRE(m2 != nullptr);
        CHECK(m2->GetNodeCount() == nodes);
        CHECK(m2->GetChanCount() == m->GetChanCount());
        CHECK(m2->GetDefaultBufferWi() == m->GetDefaultBufferWi());
        CHECK(m2->GetDefaultBufferHt() == m->GetDefaultBufferHt());
    }
}

TEST_CASE("Serialized model carries its identity and start channel", "[models][serialization]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("Roof", "Single Line", "NumStrings=\"1\" NodesPerString=\"5\" LayoutGroup=\"Front\""));
    pugi::xml_document doc;
    REQUIRE(SerializeToDoc(m, doc));
    pugi::xml_node n = doc.first_child();
    CHECK(std::string(n.name()) == "model");
    CHECK(std::string(n.attribute("name").as_string()) == "Roof");
    CHECK(std::string(n.attribute("DisplayAs").as_string()) == "Single Line");
    CHECK(std::string(n.attribute("LayoutGroup").as_string()) == "Front");
    CHECK(std::string(n.attribute("StartChannel").as_string()) == "1");
    CHECK(n.attribute("NumStrings").as_int() == 1);
    CHECK(n.attribute("NodesPerString").as_int() == 5);
    CHECK(std::string(n.attribute("StringType").as_string()) == "RGB Nodes");
}

TEST_CASE("Legacy tree display types are normalised on save", "[models][serialization]") {
    ShowFixture f;
    Model* t360 = f.Add(ModelXml("A", "Tree 360", "NumStrings=\"1\" NodesPerString=\"5\""));
    Model* flat = f.Add(ModelXml("B", "Tree Flat", "NumStrings=\"1\" NodesPerString=\"5\""));
    Model* ribbon = f.Add(ModelXml("C", "Tree Ribbon", "NumStrings=\"1\" NodesPerString=\"5\""));
    for (Model* m : { t360, flat, ribbon }) {
        pugi::xml_document doc;
        REQUIRE(SerializeToDoc(m, doc));
        CHECK(std::string(doc.first_child().attribute("DisplayAs").as_string()) == "Tree");
    }
    pugi::xml_document d1, d2, d3;
    SerializeToDoc(t360, d1);
    SerializeToDoc(flat, d2);
    SerializeToDoc(ribbon, d3);
    CHECK(d1.first_child().attribute("TreeType").as_int() == 0);
    CHECK(d1.first_child().attribute("TreeDegrees").as_int() == 360);
    CHECK(d2.first_child().attribute("TreeType").as_int() == 1);
    CHECK(d3.first_child().attribute("TreeType").as_int() == 2);
}

TEST_CASE("Legacy tree degrees are read from the display type", "[models][serialization]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("A", "Tree 180", "NumStrings=\"1\" NodesPerString=\"5\""));
    pugi::xml_document doc;
    REQUIRE(SerializeToDoc(m, doc));
    CHECK(doc.first_child().attribute("TreeDegrees").as_int() == 180);
}

TEST_CASE("Legacy Vert Matrix is saved as a vertical Matrix", "[models][serialization]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("A", "Vert Matrix", "NumStrings=\"1\" NodesPerString=\"6\" StrandsPerString=\"2\""));
    pugi::xml_document doc;
    REQUIRE(SerializeToDoc(m, doc));
    CHECK(std::string(doc.first_child().attribute("DisplayAs").as_string()) == "Matrix");
    CHECK(std::string(doc.first_child().attribute("Vertical").as_string()) == "true");
}

TEST_CASE("Legacy parm attributes are read but not written", "[models][serialization]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("A", "Matrix", "parm1=\"2\" parm2=\"6\" parm3=\"2\""));
    pugi::xml_document doc;
    REQUIRE(SerializeToDoc(m, doc));
    pugi::xml_node n = doc.first_child();
    CHECK(n.attribute("NumStrings").as_int() == 2);
    CHECK(n.attribute("NodesPerString").as_int() == 6);
    CHECK(n.attribute("StrandsPerString").as_int() == 2);
    CHECK(n.attribute("parm1").empty());
    CHECK(n.attribute("parm2").empty());
    CHECK(n.attribute("parm3").empty());
}

TEST_CASE("New attribute names win over legacy parm attributes", "[models][serialization]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("A", "Matrix", "NumStrings=\"3\" NodesPerString=\"4\" StrandsPerString=\"1\" parm1=\"9\" parm2=\"9\" parm3=\"9\""));
    CHECK(m->GetNodeCount() == 12);
}

TEST_CASE("Legacy element names are accepted for import", "[models][serialization]") {
    ShowFixture f;
    pugi::xml_document doc;
    REQUIRE(doc.load_string(R"(<archesmodel name="Old" StartChannel="1" parm1="2" parm2="5" parm3="1"/>)"));
    Model* m = f.mm.CreateModel(doc.first_child(), 100, 100);
    REQUIRE(m != nullptr);
    CHECK(m->GetDisplayAs() == DisplayAsType::Arches);
    CHECK(m->GetNodeCount() == 10);
    delete m;
}

TEST_CASE("Missing DisplayAs and unknown model types are rejected", "[models][serialization]") {
    ShowFixture f;
    pugi::xml_document doc;
    REQUIRE(doc.load_string(R"(<model name="A" StartChannel="1"/>)"));
    CHECK_THROWS_AS(f.mm.CreateModel(doc.first_child(), 0, 0), std::runtime_error);
    REQUIRE(doc.load_string(R"(<model name="A" DisplayAs="" StartChannel="1"/>)"));
    CHECK_THROWS_AS(f.mm.CreateModel(doc.first_child(), 0, 0), std::runtime_error);
    REQUIRE(doc.load_string(R"(<model name="A" DisplayAs="Teapot" StartChannel="1"/>)"));
    CHECK_THROWS_AS(f.mm.CreateModel(doc.first_child(), 0, 0), std::runtime_error);
    CHECK(f.mm.size() == 0);
}

TEST_CASE("Empty and root only documents produce no model", "[models][serialization]") {
    ShowFixture f;
    pugi::xml_document empty;
    XmlSerializer s;
    CHECK(s.DeserializeModel(empty, f.mm, false) == nullptr);
    pugi::xml_document rootOnly;
    REQUIRE(rootOnly.load_string("<xrgb/>"));
    CHECK(s.DeserializeModel(rootOnly, f.mm, false) == nullptr);
}

TEST_CASE("Deserializing a model document uses its first child", "[models][serialization]") {
    ShowFixture f;
    pugi::xml_document doc;
    REQUIRE(doc.load_string(R"(<xrgb><model name="A" DisplayAs="Single Line" StartChannel="1" NumStrings="1" NodesPerString="5"/></xrgb>)"));
    XmlSerializer s;
    Model* m = s.DeserializeModel(doc, f.mm, false);
    REQUIRE(m != nullptr);
    CHECK(m->GetName() == "A");
    CHECK(m->GetNodeCount() == 5);
    delete m;
}

TEST_CASE("Model names are trimmed on load", "[models][serialization]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("  Padded  ", "Single Line", "NumStrings=\"1\" NodesPerString=\"5\""));
    CHECK(m->GetName() == "Padded");
    CHECK(f.mm["Padded"] == m);
}

TEST_CASE("Names with markup characters survive a round trip", "[models][serialization]") {
    ShowFixture a;
    Model* m = a.Add(ModelXml("Tom &amp; Jerry &lt;1&gt;", "Single Line", "NumStrings=\"1\" NodesPerString=\"5\" Description=\"a &quot;quoted&quot; &amp; &lt;odd&gt; value\""));
    REQUIRE(m != nullptr);
    CHECK(m->GetName() == "Tom & Jerry <1>");
    pugi::xml_document doc;
    REQUIRE(SerializeToDoc(m, doc));
    std::ostringstream os;
    doc.save(os, "", pugi::format_raw);
    pugi::xml_document reparsed;
    REQUIRE(reparsed.load_string(os.str().c_str()));
    CHECK(std::string(reparsed.first_child().attribute("name").as_string()) == "Tom & Jerry <1>");
    CHECK(std::string(reparsed.first_child().attribute("Description").as_string()) == "a \"quoted\" & <odd> value");
}

TEST_CASE("Common model attributes round trip", "[models][serialization]") {
    ShowFixture a;
    Model* m = a.Add(ModelXml("M", "Single Line",
                              "NumStrings=\"1\" NodesPerString=\"5\" Description=\"hello\" TagColour=\"#FF0000\" NodeNames=\"a,b,c\" StrandNames=\"s1\" "
                              "PixelSize=\"5\" Antialias=\"2\" Transparency=\"20\" BlackTransparency=\"10\" StartSide=\"T\" Dir=\"R\" StringType=\"RGBW Nodes\" "
                              "Active=\"1\" LayoutGroup=\"Back\" CustomColor=\"#112233\""));
    pugi::xml_document doc;
    REQUIRE(SerializeToDoc(m, doc));
    pugi::xml_node n = doc.first_child();
    CHECK(std::string(n.attribute("Description").as_string()) == "hello");
    CHECK(std::string(n.attribute("TagColour").as_string()) == "#ff0000");
    CHECK(std::string(n.attribute("NodeNames").as_string()) == "a,b,c");
    CHECK(std::string(n.attribute("StrandNames").as_string()) == "s1");
    CHECK(n.attribute("PixelSize").as_int() == 5);
    CHECK(n.attribute("Antialias").as_int() == 2);
    CHECK(n.attribute("Transparency").as_int() == 20);
    CHECK(n.attribute("BlackTransparency").as_int() == 10);
    CHECK(std::string(n.attribute("StartSide").as_string()) == "T");
    CHECK(std::string(n.attribute("Dir").as_string()) == "R");
    CHECK(std::string(n.attribute("StringType").as_string()) == "RGBW Nodes");
    CHECK(std::string(n.attribute("LayoutGroup").as_string()) == "Back");
}

TEST_CASE("Common model attributes default when missing", "[models][serialization]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("M", "Single Line", "NumStrings=\"1\" NodesPerString=\"5\""));
    CHECK(m->GetPixelSize() == 2);
    CHECK(m->GetPixelStyle() == Model::PIXEL_STYLE::PIXEL_STYLE_SMOOTH);
    CHECK(m->GetTransparency() == 0);
    CHECK(m->GetBlackTransparency() == 0);
    CHECK(m->GetDescription().empty());
    CHECK(m->GetStartSide() == "B");
    CHECK(m->GetDirection() == "L");
    CHECK(m->GetStringType() == "RGB Nodes");
    CHECK(m->GetLayoutGroup() == "Unassigned");
    CHECK(m->IsActive());
    CHECK_FALSE(m->IsFromBase());
    CHECK(m->GetNodeNames().empty());
    CHECK(m->GetStrandNames().empty());
    CHECK(m->GetControllerName().empty());
}

TEST_CASE("Inactive and from-base flags are saved", "[models][serialization]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("M", "Single Line", "NumStrings=\"1\" NodesPerString=\"5\" Active=\"0\" FromBase=\"1\""));
    CHECK_FALSE(m->IsActive());
    CHECK(m->IsFromBase());
    pugi::xml_document doc;
    REQUIRE(SerializeToDoc(m, doc));
    CHECK(doc.first_child().attribute("Active").as_int(1) == 0);
    CHECK(doc.first_child().attribute("FromBase").as_int() == 1);
}

TEST_CASE("Active and from-base attributes are omitted when default", "[models][serialization]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("M", "Single Line", "NumStrings=\"1\" NodesPerString=\"5\""));
    pugi::xml_document doc;
    REQUIRE(SerializeToDoc(m, doc));
    CHECK(doc.first_child().attribute("Active").empty());
    CHECK(doc.first_child().attribute("FromBase").empty());
}

TEST_CASE("Controller connection round trips", "[models][serialization]") {
    ShowFixture a;
    Model* m = a.Add(R"(<model name="M" DisplayAs="Single Line" StartChannel="1" NumStrings="1" NodesPerString="5" Controller="">
        <ControllerConnection Port="3" Protocol="ws2811" Brightness="80" nullNodes="2" endNullNodes="1" colorOrder="GRB" gamma="2.2" groupCount="2"/>
    </model>)");
    CHECK(m->GetControllerProtocol() == "ws2811");
    CHECK(m->GetControllerPort() == 3);
    CHECK(m->GetCtrlConn().GetBrightness() == 80);
    CHECK(m->GetCtrlConn().GetStartNulls() == 2);
    CHECK(m->GetCtrlConn().GetEndNulls() == 1);
    CHECK(m->GetCtrlConn().GetColorOrder() == "GRB");
    CHECK(m->GetCtrlConn().GetGroupCount() == 2);
    CHECK(m->GetCtrlConn().GetGamma() == 2.2f);

    pugi::xml_document doc;
    REQUIRE(SerializeToDoc(m, doc));
    pugi::xml_node cc = doc.first_child().child("ControllerConnection");
    REQUIRE(cc);
    CHECK(cc.attribute("Port").as_int() == 3);
    CHECK(std::string(cc.attribute("Protocol").as_string()) == "ws2811");

    ShowFixture b;
    Model* m2 = b.mm.createAndAddModel(doc.first_child(), 0, 0);
    REQUIRE(m2 != nullptr);
    CHECK(m2->GetControllerPort() == 3);
    CHECK(m2->GetCtrlConn().GetBrightness() == 80);
    CHECK(m2->GetCtrlConn().GetColorOrder() == "GRB");
    CHECK(m2->GetCtrlConn().GetGroupCount() == 2);
}

TEST_CASE("Controller connection defaults when attributes are missing", "[models][serialization]") {
    ShowFixture f;
    Model* m = f.Add(R"(<model name="M" DisplayAs="Single Line" StartChannel="1" NumStrings="1" NodesPerString="5">
        <ControllerConnection Protocol="ws2811"/>
    </model>)");
    CHECK(m->GetControllerPort() == 0);
    CHECK(m->GetCtrlConn().GetBrightness() == 100);
    CHECK(m->GetCtrlConn().GetStartNulls() == 0);
    CHECK(m->GetCtrlConn().GetEndNulls() == 0);
    CHECK(m->GetCtrlConn().GetColorOrder() == "RGB");
    CHECK(m->GetCtrlConn().GetGamma() == 1.0f);
    CHECK(m->GetCtrlConn().GetGroupCount() == 0);
    CHECK(m->GetCtrlConn().GetProtocolSpeed() == 250000);
}

TEST_CASE("Controller connection with junk values does not crash", "[models][serialization]") {
    ShowFixture f;
    Model* m = f.Add(R"(<model name="M" DisplayAs="Single Line" StartChannel="1" NumStrings="1" NodesPerString="5">
        <ControllerConnection Port="abc" Protocol="" Brightness="-50" nullNodes="zzz" gamma="notanumber" groupCount="99999999999"/>
    </model>)");
    REQUIRE(m != nullptr);
    (void)m->GetControllerPort();
}

TEST_CASE("Submodels round trip", "[models][serialization][submodel]") {
    const std::string xml = R"(<model name="T" DisplayAs="Tree 360" NumStrings="2" NodesPerString="10" StrandsPerString="1" StartChannel="1">
        <subModel name="h" layout="horizontal" type="ranges" bufferstyle="Default" line0="1-5" line1="6-10"/>
        <subModel name="v" layout="vertical" type="ranges" bufferstyle="Stacked - Horizontal" line0="1-2" line1="3-4"/>
        <subModel name="sb" layout="horizontal" type="subbuffer" subBuffer="0x0x100x100"/>
    </model>)";
    std::string first;
    std::string second = RoundTrip(xml, &first);
    CHECK(first == second);

    ShowFixture f;
    Model* m = f.Add(xml);
    pugi::xml_document doc;
    REQUIRE(SerializeToDoc(m, doc));
    int count = 0;
    for (pugi::xml_node c : doc.first_child().children("subModel")) {
        ++count;
        if (std::string(c.attribute("name").as_string()) == "h") {
            CHECK(std::string(c.attribute("line0").as_string()) == "1-5");
            CHECK(std::string(c.attribute("line1").as_string()) == "6-10");
            CHECK(std::string(c.attribute("layout").as_string()) == "horizontal");
        }
    }
    CHECK(count == 3);
}

TEST_CASE("Submodel nodes without a name or type use defaults", "[models][serialization][submodel]") {
    ShowFixture f;
    Model* m = f.Add(R"(<model name="T" DisplayAs="Tree 360" NumStrings="1" NodesPerString="10" StrandsPerString="1" StartChannel="1">
        <subModel line0="1-3"/>
    </model>)");
    REQUIRE(m != nullptr);
    CHECK(m->GetNumSubModels() == 1);
}

TEST_CASE("ParseSubModelNode rejects other elements and reads lines", "[models][serialization][submodel]") {
    pugi::xml_document doc;
    REQUIRE(doc.load_string(R"(<subModel name=" Sub " layout="vertical" type="ranges" bufferstyle="Keep XY" line0="1-3" line1="5" line3="9"/>)"));
    auto sm = XmlSerialize::ParseSubModelNode(doc.first_child());
    REQUIRE(sm.has_value());
    CHECK(sm->name == "Sub");
    CHECK(sm->isRanges);
    CHECK(sm->vertical);
    CHECK(sm->bufferStyle == "Keep XY");
    REQUIRE(sm->strands.size() == 2);
    CHECK(sm->strands[0] == "1-3");
    CHECK(sm->strands[1] == "5");

    REQUIRE(doc.load_string(R"(<other name="x"/>)"));
    CHECK_FALSE(XmlSerialize::ParseSubModelNode(doc.first_child()).has_value());
    CHECK_FALSE(XmlSerialize::ParseSubModelNode(pugi::xml_node()).has_value());
}

TEST_CASE("ParseSubModelNode defaults", "[models][serialization][submodel]") {
    pugi::xml_document doc;
    REQUIRE(doc.load_string(R"(<subModel name="a"/>)"));
    auto sm = XmlSerialize::ParseSubModelNode(doc.first_child());
    REQUIRE(sm.has_value());
    CHECK(sm->isRanges);
    CHECK(sm->vertical);
    CHECK(sm->bufferStyle == "Default");
    CHECK(sm->strands.empty());
}

TEST_CASE("Aliases round trip", "[models][serialization]") {
    const std::string xml = R"(<model name="M" DisplayAs="Single Line" StartChannel="1" NumStrings="1" NodesPerString="5">
        <Aliases><alias name="Old One"/><alias name="Older"/><alias/></Aliases>
    </model>)";
    ShowFixture a;
    Model* m = a.Add(xml);
    REQUIRE(m->GetAliases().size() == 2);
    CHECK(m->IsAlias("Old One"));
    CHECK(m->IsAlias("Older"));
    CHECK_FALSE(m->IsAlias("Newer"));

    pugi::xml_document doc;
    REQUIRE(SerializeToDoc(m, doc));
    ShowFixture b;
    Model* m2 = b.mm.createAndAddModel(doc.first_child(), 0, 0);
    REQUIRE(m2 != nullptr);
    CHECK(m2->GetAliases() == m->GetAliases());
}

TEST_CASE("Face and state definitions round trip", "[models][serialization]") {
    const std::string xml = R"(<model name="M" DisplayAs="Single Line" StartChannel="1" NumStrings="1" NodesPerString="20">
        <faceInfo Name="Face1" Type="NodeRange" CustomColors="0" Eyes-Open="1-2" Mouth-AI="3-5"/>
        <stateInfo Name="Lit" Type="NodeRange" CustomColors="0" s1="1-10" s1-Name="Half" s1-Color="#FF0000"/>
    </model>)";
    ShowFixture a;
    Model* m = a.Add(xml);
    REQUIRE(m->GetFaceInfo().count("Face1") == 1);
    CHECK(m->GetFaceInfo().at("Face1").at("Type") == "NodeRange");
    CHECK(m->GetFaceInfo().at("Face1").at("Eyes-Open") == "1-2");
    REQUIRE(m->GetStateInfo().count("Lit") == 1);
    CHECK(m->GetStateInfo().at("Lit").at("s001") == "1-10");
    CHECK(m->GetStateInfo().at("Lit").at("s001-Name") == "Half");

    std::string first;
    std::string second = RoundTrip(xml, &first);
    CHECK(first == second);
}

TEST_CASE("Individual start channels are saved with the model", "[models][serialization]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("T", "Tree 360", "NumStrings=\"3\" NodesPerString=\"10\" StrandsPerString=\"1\" Advanced=\"1\" String1=\"1\" String2=\"500\" String3=\"900\""));
    pugi::xml_document doc;
    REQUIRE(SerializeToDoc(m, doc));
    pugi::xml_node n = doc.first_child();
    CHECK(n.attribute("Advanced").as_int() == 1);
    CHECK(std::string(n.attribute("String1").as_string()) == "1");
    CHECK(std::string(n.attribute("String2").as_string()) == "500");
    CHECK(std::string(n.attribute("String3").as_string()) == "900");
}

TEST_CASE("Models without individual start channels do not write them", "[models][serialization]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("T", "Tree 360", "NumStrings=\"3\" NodesPerString=\"10\" StrandsPerString=\"1\""));
    pugi::xml_document doc;
    REQUIRE(SerializeToDoc(m, doc));
    CHECK(doc.first_child().attribute("Advanced").empty());
    CHECK(doc.first_child().attribute("String1").empty());
}

TEST_CASE("Custom model is saved in compressed form", "[models][serialization]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("C", "Custom", "CustomWidth=\"3\" CustomHeight=\"2\" CustomModel=\"1,,2;,3,\""));
    pugi::xml_document doc;
    REQUIRE(SerializeToDoc(m, doc));
    pugi::xml_node n = doc.first_child();
    CHECK(std::string(n.attribute("CustomModelCompressed").as_string()) == "1,0,0;2,0,2;3,1,1");
    CHECK(n.attribute("CustomModel").empty());
    CHECK(n.attribute("CustomWidth").as_int() == 3);
    CHECK(n.attribute("CustomHeight").as_int() == 2);
    auto grid = XmlSerialize::ParseCustomModelDataFromXml(n);
    REQUIRE(grid.size() == 1);
    CHECK(grid[0][0] == std::vector<int>{ 1, -1, 2 });
    CHECK(grid[0][1] == std::vector<int>{ -1, 3, -1 });
}

TEST_CASE("Custom model strings and start nodes round trip", "[models][serialization]") {
    const std::string xml = ModelXml("C", "Custom", "CustomWidth=\"4\" CustomHeight=\"1\" CustomModel=\"1,2,3,4\" CustomStrings=\"2\" Advanced=\"1\" String1=\"1\" String2=\"200\" NodesPerString=\"2\"");
    ShowFixture a;
    Model* m = a.Add(xml);
    REQUIRE(m != nullptr);
    CHECK(m->GetNumStrings() == 2);
    std::string first;
    std::string second = RoundTrip(xml, &first);
    CHECK(first == second);
}

TEST_CASE("Zig zag default depends on the serializer version", "[models][serialization]") {
    ShowFixture f;
    Model* old = f.Add(ModelXml("Old", "Arches", "NumArches=\"1\" NodesPerArch=\"5\""));
    Model* current = f.Add(ModelXml("New", "Arches", "NumArches=\"1\" NodesPerArch=\"5\" SourceVersion=\"2026.04\""));
    pugi::xml_document d1, d2;
    REQUIRE(SerializeToDoc(old, d1));
    REQUIRE(SerializeToDoc(current, d2));
    CHECK(std::string(d1.first_child().attribute("ZigZag").as_string()) == "true");
    CHECK(std::string(d2.first_child().attribute("ZigZag").as_string("absent")) != "true");
}

TEST_CASE("Group node deserialises with defaults", "[models][serialization][group]") {
    ShowFixture f;
    f.Add(ModelXml("A", "Single Line", "NumStrings=\"1\" NodesPerString=\"5\""));
    Model* g = f.Add(R"(<modelGroup name="G" models="A"/>)");
    auto* group = dynamic_cast<ModelGroup*>(g);
    REQUIRE(group != nullptr);
    CHECK(group->GetGridSize() == 400);
    CHECK(group->GetLayout() == "minimalGrid");
    CHECK(group->GetDefaultCamera() == "2D");
    CHECK(group->ModelNames() == std::vector<std::string>{ "A" });
    CHECK(group->GetLayoutGroup() == "Unassigned");
    CHECK(group->IsActive());
}

TEST_CASE("Group attributes round trip", "[models][serialization][group]") {
    const std::string xml = R"(<modelGroup name="G" models="A,B" GridSize="200" layout="horizontal" DefaultCamera="3D" LayoutGroup="Front" centreDefined="1" centrex="0.25" centrey="0.75" TagColour="#00FF00"/>)";
    ShowFixture a;
    a.Add(ModelXml("A", "Single Line", "NumStrings=\"1\" NodesPerString=\"5\""));
    a.Add(ModelXml("B", "Single Line", "NumStrings=\"1\" NodesPerString=\"5\""));
    Model* g = a.Add(xml);
    pugi::xml_document doc;
    REQUIRE(SerializeToDoc(g, doc));
    pugi::xml_node n = doc.first_child();
    CHECK(std::string(n.name()) == "modelGroup");
    CHECK(std::string(n.attribute("models").as_string()) == "A,B");
    CHECK(n.attribute("GridSize").as_int() == 200);
    CHECK(std::string(n.attribute("layout").as_string()) == "horizontal");
    CHECK(std::string(n.attribute("DefaultCamera").as_string()) == "3D");
    CHECK(std::string(n.attribute("LayoutGroup").as_string()) == "Front");

    ShowFixture b;
    b.Add(ModelXml("A", "Single Line", "NumStrings=\"1\" NodesPerString=\"5\""));
    b.Add(ModelXml("B", "Single Line", "NumStrings=\"1\" NodesPerString=\"5\""));
    Model* g2 = b.mm.createAndAddModel(doc.first_child(), 0, 0);
    REQUIRE(g2 != nullptr);
    CHECK(SerializeModel(g2) == SerializeModel(g));
}

TEST_CASE("Whole show serialises models and groups separately", "[models][serialization][group]") {
    ShowFixture f;
    f.Add(ModelXml("A", "Single Line", "NumStrings=\"1\" NodesPerString=\"5\""));
    f.Add(ModelXml("B", "Arches", "NumArches=\"1\" NodesPerArch=\"5\""));
    f.Add(R"(<modelGroup name="G" models="A,B"/>)");
    pugi::xml_document doc;
    XmlSerializingVisitor v(&doc);
    XmlSerializer::SerializeAllModels(f.mm, v);
    pugi::xml_node models = doc.child("models");
    pugi::xml_node groups = doc.child("modelGroups");
    REQUIRE(models);
    REQUIRE(groups);
    int modelCount = 0;
    for (pugi::xml_node c : models.children("model")) {
        (void)c;
        ++modelCount;
    }
    int groupCount = 0;
    for (pugi::xml_node c : groups.children("modelGroup")) {
        (void)c;
        ++groupCount;
    }
    CHECK(modelCount == 2);
    CHECK(groupCount == 1);
}

TEST_CASE("A whole show survives serialize and load", "[models][serialization]") {
    ShowFixture a;
    a.Add(ModelXml("A", "Single Line", "NumStrings=\"1\" NodesPerString=\"5\""));
    a.Add("<model name=\"B\" DisplayAs=\"Matrix\" NumStrings=\"1\" NodesPerString=\"12\" StrandsPerString=\"3\" StartChannel=\"&gt;A:1\"/>");
    pugi::xml_document doc;
    XmlSerializingVisitor v(&doc);
    XmlSerializer::SerializeAllModels(a.mm, v);

    ShowFixture b;
    for (pugi::xml_node c : doc.child("models").children("model")) {
        b.mm.createAndAddModel(c, 100, 100);
    }
    b.mm.RecalcStartChannels();
    REQUIRE(b.mm.size() == 2);
    CHECK(b.mm["A"]->GetNodeCount() == 5);
    CHECK(b.mm["B"]->GetNodeCount() == 12);
    CHECK(b.mm["B"]->GetFirstChannel() == b.mm["A"]->GetLastChannel() + 1);
}

TEST_CASE("StringSerializingVisitor writes a parseable model", "[models][serialization]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("A", "Arches", "NumArches=\"2\" NodesPerArch=\"5\""));
    StringSerializingVisitor sv;
    m->Accept(sv);
    std::string text = sv.GetResult();
    CHECK(text.find("<model") != std::string::npos);
    CHECK(text.find("name=\"A\"") != std::string::npos);
    pugi::xml_document doc;
    REQUIRE(doc.load_string(text.c_str()));
    CHECK(std::string(doc.child("model").attribute("DisplayAs").as_string()) == "Arches");
    CHECK(doc.child("model").attribute("NumArches").as_int() == 2);
}

TEST_CASE("StringSerializingVisitor output deserialises to an equal model", "[models][serialization]") {
    ShowFixture a;
    Model* m = a.Add(ModelXml("A", "Matrix", "NumStrings=\"2\" NodesPerString=\"6\" StrandsPerString=\"2\""));
    StringSerializingVisitor sv(false, false);
    m->Accept(sv);
    pugi::xml_document doc;
    REQUIRE(doc.load_string(sv.GetResult().c_str()));
    ShowFixture b;
    Model* m2 = b.mm.createAndAddModel(doc.child("model"), 0, 0);
    REQUIRE(m2 != nullptr);
    CHECK(SerializeModel(m2) == SerializeModel(m));
}

TEST_CASE("Importing a model with a clashing name gets a new name", "[models][serialization]") {
    ShowFixture f;
    f.Add(ModelXml("Tree", "Single Line", "NumStrings=\"1\" NodesPerString=\"5\""));
    pugi::xml_document doc;
    REQUIRE(doc.load_string(ModelXml("Tree", "Single Line", "NumStrings=\"1\" NodesPerString=\"5\" LayoutGroup=\"Front\"").c_str()));
    XmlSerializer s;
    Model* imported = s.DeserializeModel(doc.first_child(), f.mm, true);
    REQUIRE(imported != nullptr);
    CHECK(imported->GetName() != "Tree");
    CHECK(imported->GetName().find("Tree") == 0);
    CHECK(imported->GetLayoutGroup() == "Unassigned");
    delete imported;
}

TEST_CASE("Importing keeps the start channel but drops the controller", "[models][serialization]") {
    ShowFixture f;
    pugi::xml_document doc;
    REQUIRE(doc.load_string(R"(<model name="Imp" DisplayAs="Single Line" StartChannel="77" Controller="Somewhere" NumStrings="1" NodesPerString="5"/>)"));
    XmlSerializer s;
    Model* imported = s.DeserializeModel(doc.first_child(), f.mm, true);
    REQUIRE(imported != nullptr);
    CHECK(imported->GetModelStartChannel() == "77");
    CHECK(imported->GetControllerName() != "Somewhere");
    delete imported;
}

TEST_CASE("Models with duplicate names replace the earlier model", "[models][serialization]") {
    ShowFixture f;
    f.Add(ModelXml("Dup", "Single Line", "NumStrings=\"1\" NodesPerString=\"5\""));
    Model* second = f.Add(ModelXml("Dup", "Single Line", "NumStrings=\"1\" NodesPerString=\"9\""));
    CHECK(f.mm.size() == 1);
    CHECK(f.mm["Dup"] == second);
    CHECK(f.mm["Dup"]->GetNodeCount() == 9);
}

TEST_CASE("Models without a name are created with an empty name", "[models][serialization]") {
    ShowFixture f;
    pugi::xml_document doc;
    REQUIRE(doc.load_string(R"(<model DisplayAs="Single Line" StartChannel="1" NumStrings="1" NodesPerString="5"/>)"));
    Model* m = f.mm.CreateModel(doc.first_child(), 0, 0);
    REQUIRE(m != nullptr);
    CHECK(m->GetName().empty());
    delete m;
}

TEST_CASE("Extremely long and unusual attribute values do not crash deserialisation", "[models][serialization]") {
    ShowFixture f;
    const std::string longText(100000, 'x');
    Model* m = f.Add(ModelXml("Long", "Single Line",
                              "NumStrings=\"1\" NodesPerString=\"5\" Description=\"" + longText + "\" NodeNames=\"" + longText + "\" StrandNames=\",,,,,\" TagColour=\"notacolor\" CustomColor=\"\" StartSide=\"\" Dir=\"\" PixelSize=\"-1\" Antialias=\"99\" Transparency=\"abc\""));
    REQUIRE(m != nullptr);
    CHECK(m->GetDescription().size() == longText.size());
    (void)SerializeModel(m);
}

TEST_CASE("Unknown attributes and child elements are ignored", "[models][serialization]") {
    ShowFixture f;
    Model* m = f.Add(R"(<model name="M" DisplayAs="Single Line" StartChannel="1" NumStrings="1" NodesPerString="5" Bogus="1" AnotherBogus="x">
        <bogusChild a="1"><nested/></bogusChild>
        text node
    </model>)");
    REQUIRE(m != nullptr);
    CHECK(m->GetNodeCount() == 5);
}

TEST_CASE("Dimming curve round trips", "[models][serialization]") {
    const std::string xml = R"(<model name="M" DisplayAs="Single Line" StartChannel="1" NumStrings="1" NodesPerString="5">
        <dimmingCurve><all gamma="2.2" brightness="80"/></dimmingCurve>
    </model>)";
    ShowFixture a;
    Model* m = a.Add(xml);
    REQUIRE(m->GetDimmingCurve() != nullptr);
    pugi::xml_document doc;
    REQUIRE(SerializeToDoc(m, doc));
    pugi::xml_node dc = doc.first_child().child("dimmingCurve");
    REQUIRE(dc);
    CHECK(std::string(dc.child("all").attribute("gamma").as_string()) == "2.2");
    CHECK(std::string(dc.child("all").attribute("brightness").as_string()) == "80");
}

TEST_CASE("Legacy ModelBrightness becomes a dimming curve", "[models][serialization]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("M", "Single Line", "NumStrings=\"1\" NodesPerString=\"5\" ModelBrightness=\"25\""));
    REQUIRE(m->GetDimmingCurve() != nullptr);
    Model* none = f.Add(ModelXml("N", "Single Line", "NumStrings=\"1\" NodesPerString=\"5\" ModelBrightness=\"0\""));
    CHECK(none->GetDimmingCurve() == nullptr);
}

TEST_CASE("Legacy DMX display types migrate to current types", "[models][serialization]") {
    ShowFixture f;
    pugi::xml_document doc;
    REQUIRE(doc.load_string(R"(<model name="Old" DisplayAs="DMX" DmxStyle="Flood Light" StartChannel="1" parm1="3"/>)"));
    XmlDeserializingModelFactory::MigrateLegacyModelNode(doc.first_child());
    CHECK(std::string(doc.first_child().attribute("DisplayAs").as_string()) == "DmxFloodlight");

    REQUIRE(doc.load_string(R"(<model name="Old" DisplayAs="DMX" DmxStyle="Skulltronix Skull" StartChannel="1"/>)"));
    XmlDeserializingModelFactory::MigrateLegacyModelNode(doc.first_child());
    CHECK(std::string(doc.first_child().attribute("DisplayAs").as_string()) == "DmxSkull");

    REQUIRE(doc.load_string(R"(<model name="Old" DisplayAs="DMX" DmxStyle="Moving Head Top" StartChannel="1"/>)"));
    XmlDeserializingModelFactory::MigrateLegacyModelNode(doc.first_child());
    CHECK(std::string(doc.first_child().attribute("DisplayAs").as_string()) == "DmxMovingHead");
    CHECK(doc.first_child().child("PanMotor"));
    CHECK(doc.first_child().child("TiltMotor"));
}

TEST_CASE("Legacy node migration leaves current models untouched", "[models][serialization]") {
    pugi::xml_document doc;
    REQUIRE(doc.load_string(R"(<model name="A" DisplayAs="Arches" StartChannel="1" NumArches="1"/>)"));
    XmlDeserializingModelFactory::MigrateLegacyModelNode(doc.first_child());
    CHECK(std::string(doc.first_child().attribute("DisplayAs").as_string()) == "Arches");
    CHECK(doc.first_child().first_child().empty());
    XmlDeserializingModelFactory::MigrateLegacyModelNode(pugi::xml_node());
}

TEST_CASE("Serialising the same model twice is stable", "[models][serialization]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("M", "Tree 360", "NumStrings=\"2\" NodesPerString=\"10\" StrandsPerString=\"1\""));
    CHECK(SerializeModel(m) == SerializeModel(m));
}

TEST_CASE("Serialised attributes are sorted with name and DisplayAs first", "[models][serialization]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("M", "Arches", "NumArches=\"1\" NodesPerArch=\"5\""));
    pugi::xml_document doc;
    REQUIRE(SerializeToDoc(m, doc));
    auto attr = doc.first_child().first_attribute();
    REQUIRE(attr);
    CHECK(std::string(attr.name()) == "name");
    CHECK(std::string(attr.next_attribute().name()) == "DisplayAs");
}

TEST_CASE("Floating point values are written without locale or precision noise", "[models][serialization]") {
    ShowFixture f;
    Model* m = f.Add(ModelXml("M", "Single Line", "NumStrings=\"1\" NodesPerString=\"5\" WorldPosX=\"1.5\" WorldPosY=\"-2.25\" X2=\"10.125\""));
    pugi::xml_document doc;
    REQUIRE(SerializeToDoc(m, doc));
    pugi::xml_node n = doc.first_child();
    CHECK(n.attribute("WorldPosX").as_float() == 1.5f);
    CHECK(n.attribute("WorldPosY").as_float() == -2.25f);
    CHECK(n.attribute("X2").as_float() == 10.125f);
}

TEST_CASE("DeserializeStateInfo normalises state keys", "[models][serialization]") {
    pugi::xml_document doc;
    REQUIRE(doc.load_string(R"(<stateInfo Name="Lit" Type="NodeRange" CustomColors="1" s1="1-5" s12-Name="Twelve" s3-Color="#00FF00" Empty=""/>)"));
    FaceStateData info;
    XmlSerialize::DeserializeStateInfo(doc.first_child(), info);
    REQUIRE(info.count("Lit") == 1);
    const auto& st = info.at("Lit");
    CHECK(st.at("s001") == "1-5");
    CHECK(st.at("s012-Name") == "Twelve");
    CHECK(st.at("s003-Color") == "#00FF00");
    CHECK(st.at("Type") == "NodeRange");
    CHECK(st.count("Empty") == 0);
    CHECK(st.count("Name") == 0);
}
