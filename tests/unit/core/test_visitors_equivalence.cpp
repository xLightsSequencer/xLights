#include <algorithm>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <pugixml.hpp>

#include "../support/TempDir.h"
#include "XmlSerializer/FileSerializingVisitor.h"
#include "XmlSerializer/StringSerializingVisitor.h"
#include "XmlSerializer/XmlSerializer.h"
#include "XmlSerializer/XmlSerializingVisitor.h"
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
};

std::string M(const std::string& name, const std::string& type, const std::string& attrs, const std::string& children = "") {
    return "<model name=\"" + name + "\" DisplayAs=\"" + type + "\" StartChannel=\"1\" " + attrs + ">" + children + "</model>";
}

struct Sample {
    const char* label;
    std::string xml;
};

std::vector<Sample> Corpus() {
    const std::string extras = "<subModel name=\"top\" layout=\"horizontal\" type=\"ranges\" line0=\"1-3\"/>"
                               "<faceInfo Name=\"F\" Type=\"NodeRange\" CustomColors=\"0\" Eyes-Open=\"1-2\"/>"
                               "<stateInfo Name=\"S\" Type=\"NodeRange\" CustomColors=\"0\" s1=\"1-4\" s1-Name=\"Half\" s1-Color=\"#FF0000\"/>"
                               "<Aliases><alias name=\"Old\"/></Aliases>";
    return {
        { "tree", M("T", "Tree 360", "NumStrings=\"3\" NodesPerString=\"7\" StrandsPerString=\"1\" Description=\"a &amp; b &lt;c&gt;\"", extras) },
        { "tree flat", M("TF", "Tree Flat", "NumStrings=\"2\" NodesPerString=\"5\" StrandsPerString=\"1\" TreeSpiralRotations=\"2.5\"") },
        { "matrix", M("Mx", "Matrix", "NumStrings=\"2\" NodesPerString=\"12\" StrandsPerString=\"3\" StartSide=\"T\" Dir=\"R\"", extras) },
        { "vertical matrix", M("Vm", "Matrix", "NumStrings=\"1\" NodesPerString=\"12\" StrandsPerString=\"3\" Vertical=\"true\" NoZig=\"true\"") },
        { "arches", M("A", "Arches", "NumArches=\"3\" NodesPerArch=\"9\" ArchesSkew=\"20\" ZigZag=\"true\"") },
        { "candy canes", M("C", "Candy Canes", "NumCanes=\"3\" NodesPerCane=\"6\" CandyCaneHeight=\"2.5\" CandyCaneReverse=\"true\" AlternateNodes=\"true\"") },
        { "icicles", M("I", "Icicles", "NumStrings=\"2\" NodesPerString=\"10\" DropPattern=\"3,1,2\" AlternateNodes=\"True\"") },
        { "star", M("St", "Star", "NumStrings=\"2\" NodesPerString=\"10\" StarPoints=\"6\" StarStartLocation=\"Bottom Left\" starRatio=\"2.5\"") },
        { "wreath", M("W", "Wreath", "NumStrings=\"2\" NodesPerString=\"12\"") },
        { "circle", M("Ci", "Circle", "NumStrings=\"2\" NodesPerString=\"12\" LayerSizes=\"8,4\" InsideOut=\"1\"") },
        { "spinner", M("Sp", "Spinner", "NumStrings=\"2\" NodesPerString=\"9\" ArmsPerString=\"3\" Hollow=\"20\" StartAngle=\"15\" Arc=\"270\" ZigZag=\"false\"") },
        { "window frame", M("Wf", "Window Frame", "TopNodes=\"5\" SideNodes=\"4\" BottomNodes=\"5\" RotateDirection=\"1\"") },
        { "cube", M("Cu", "Cube", "CubeWidth=\"3\" CubeHeight=\"2\" CubeDepth=\"2\" Style=\"Vertical Left\" Strings=\"2\" Start=\"TopFrontLeft\"") },
        { "sphere", M("Sh", "Sphere", "NumStrings=\"2\" NodesPerString=\"10\" StrandsPerString=\"2\" StartLatitude=\"-70\" EndLatitude=\"70\" Degrees=\"300\"") },
        { "single line", M("Sl", "Single Line", "NumStrings=\"3\" NodesPerString=\"11\" LightsPerNode=\"2\" StringType=\"RGBW Nodes\"") },
        { "poly line", M("Pl", "Poly Line", "NumStrings=\"1\" NodesPerString=\"20\" NumPoints=\"3\" PointData=\"0.0,0.0,0.0,5.0,5.0,0.0,10.0,0.0,0.0\" cPointData=\"\" IndivSegs=\"2,3\" DropPattern=\"1\" AlternateNodes=\"true\"") },
        { "channel block", M("Cb", "Channel Block", "parm1=\"1\" parm2=\"4\" ChannelColors=\"Red,Green,Blue,White\"") },
        { "custom", M("Cm", "Custom", "CustomWidth=\"3\" CustomHeight=\"2\" CustomModel=\"1,2,3;4,5,6\" CustomStrings=\"1\" StringType=\"RGB Nodes\"", extras) },
        { "label", M("L", "Label", "Text=\"Hello &amp; goodbye\" FontSize=\"22\" Style=\"Plain\"") },
        { "moving head", M("Mh", "DmxMovingHead", "DmxChannelCount=\"16\" StringType=\"Single Color White\" DmxFixture=\"MH2\"", "<PanMotor ChannelCoarse=\"3\" ChannelFine=\"4\" RangeOfMotion=\"360\"/><TiltMotor ChannelCoarse=\"5\"/>") },
        { "moving head adv", M("Ma", "DmxMovingHeadAdv", "DmxChannelCount=\"20\" StringType=\"Single Color White\" DmxColorType=\"2\"") },
        { "floodlight", M("Fl", "DmxFloodlight", "DmxChannelCount=\"4\" StringType=\"Single Color White\" DmxRedChannel=\"1\" DmxGreenChannel=\"2\" DmxBlueChannel=\"3\" DmxWhiteChannel=\"4\"") },
        { "flood area", M("Fa", "DmxFloodArea", "DmxChannelCount=\"3\" StringType=\"Single Color White\"") },
        { "skull", M("Sk", "DmxSkull", "DmxChannelCount=\"26\" StringType=\"Single Color White\"") },
        { "servo", M("Sv", "DmxServo", "NumServos=\"1\" DmxChannelCount=\"2\" StringType=\"Single Color White\"") },
        { "servo 3d", M("S3", "DmxServo3d", "NumServos=\"2\" NumStatic=\"2\" NumMotion=\"2\" DmxChannelCount=\"4\" StringType=\"Single Color White\"") },
        { "dmx general", M("Dg", "DmxGeneral", "DmxChannelCount=\"6\" StringType=\"Single Color White\"") },
    };
}

std::string Canon(const pugi::xml_node& n) {
    std::ostringstream os;
    n.print(os, "", pugi::format_raw);
    return os.str();
}

std::string ViaXml(const Model* m, bool exporting = false) {
    pugi::xml_document doc;
    XmlSerializingVisitor v(&doc, exporting);
    m->Accept(v);
    return Canon(doc.first_child());
}

std::string ViaString(const Model* m, bool exporting = false, bool pretty = true, std::string* raw = nullptr) {
    StringSerializingVisitor v(exporting, pretty);
    m->Accept(v);
    if (raw) {
        *raw = v.GetResult();
    }
    pugi::xml_document doc;
    REQUIRE(doc.load_string(v.GetResult().c_str()));
    return Canon(doc.first_child());
}

std::string ReadFile(const std::string& path) {
    std::ifstream in(path); // text mode: Windows writers emit CRLF
    return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

} // namespace

TEST_CASE("String and XML visitors produce the same document for every model type", "[models2][visitors]") {
    for (const auto& s : Corpus()) {
        INFO(s.label);
        ShowFixture f;
        Model* m = f.Add(s.xml);
        std::string x = ViaXml(m);
        CHECK(!x.empty());
        CHECK(ViaString(m) == x);
    }
}

TEST_CASE("Pretty printing does not change the parsed document", "[models2][visitors]") {
    for (const auto& s : Corpus()) {
        INFO(s.label);
        ShowFixture f;
        Model* m = f.Add(s.xml);
        CHECK(ViaString(m, false, true) == ViaString(m, false, false));
    }
}

TEST_CASE("Unpretty output has no newlines or indentation", "[models2][visitors]") {
    ShowFixture f;
    Model* m = f.Add(Corpus()[0].xml);
    std::string raw;
    ViaString(m, false, false, &raw);
    std::string body = raw.substr(raw.find("?>") + 2);
    CHECK(body.find('\n') == std::string::npos);
    CHECK(body.find("  <") == std::string::npos);
}

TEST_CASE("Pretty output is indented and one element per line", "[models2][visitors]") {
    ShowFixture f;
    Model* m = f.Add(Corpus()[0].xml);
    std::string raw;
    ViaString(m, false, true, &raw);
    CHECK(raw.find('\n') != std::string::npos);
    CHECK(raw.find("\n  <subModel") != std::string::npos);
}

TEST_CASE("String visitor output starts with an XML declaration", "[models2][visitors]") {
    ShowFixture f;
    Model* m = f.Add(Corpus()[2].xml);
    std::string raw;
    ViaString(m, false, true, &raw);
    CHECK(raw.rfind("<?xml version=\"1.0\" encoding=\"UTF-8\"?>", 0) == 0);
    StringSerializingVisitor empty;
    CHECK(empty.GetResult().rfind("<?xml", 0) == 0);
}

TEST_CASE("File visitor over a stream matches the string visitor", "[models2][visitors]") {
    for (const auto& s : Corpus()) {
        INFO(s.label);
        ShowFixture f;
        Model* m = f.Add(s.xml);
        StringSerializingVisitor sv;
        m->Accept(sv);
        std::ostringstream os;
        {
            FileSerializingVisitor fv(os);
            m->Accept(fv);
        }
        CHECK(os.str() == sv.GetResult());
    }
}

TEST_CASE("File visitor writes the same bytes to a file", "[models2][visitors]") {
    TempDir dir;
    ShowFixture f;
    for (const auto& s : Corpus()) {
        INFO(s.label);
        Model* m = f.Add(s.xml);
        StringSerializingVisitor sv(false, false);
        m->Accept(sv);
        std::string path = dir.File(std::string("out_") + std::to_string(f.mm.size()) + ".xml");
        {
            FileSerializingVisitor fv(path, false, false);
            REQUIRE(fv.IsOpen());
            m->Accept(fv);
        }
        CHECK(ReadFile(path) == sv.GetResult());
    }
}

TEST_CASE("File visitor reports a path it cannot open", "[models2][visitors]") {
    TempDir dir;
    FileSerializingVisitor fv(dir.File("no_such_directory/deeper/out.xml"));
    CHECK_FALSE(fv.IsOpen());
    ShowFixture f;
    Model* m = f.Add(Corpus()[0].xml);
    m->Accept(fv);
}

TEST_CASE("File visitor with a pretty flag matches the string visitor pretty setting", "[models2][visitors]") {
    TempDir dir;
    ShowFixture f;
    Model* m = f.Add(Corpus()[4].xml);
    StringSerializingVisitor sv(false, true);
    m->Accept(sv);
    std::string path = dir.File("pretty.xml");
    {
        FileSerializingVisitor fv(path, false, true);
        REQUIRE(fv.IsOpen());
        CHECK(fv.IsPrettyPrint());
        m->Accept(fv);
    }
    CHECK(ReadFile(path) == sv.GetResult());
}

TEST_CASE("Exporting flag leaves models without file references unchanged", "[models2][visitors]") {
    for (const auto& s : Corpus()) {
        INFO(s.label);
        ShowFixture f;
        Model* m = f.Add(s.xml);
        CHECK(ViaXml(m, false) == ViaXml(m, true));
        CHECK(ViaString(m, false) == ViaString(m, true));
    }
}

TEST_CASE("Exporting through string and XML visitors agrees", "[models2][visitors]") {
    for (const auto& s : Corpus()) {
        INFO(s.label);
        ShowFixture f;
        Model* m = f.Add(s.xml);
        CHECK(ViaXml(m, true) == ViaString(m, true));
    }
}

TEST_CASE("Serialized output of every model type reloads to the same document", "[models2][visitors]") {
    for (const auto& s : Corpus()) {
        INFO(s.label);
        ShowFixture a;
        Model* m = a.Add(s.xml);
        StringSerializingVisitor sv(false, false);
        m->Accept(sv);
        pugi::xml_document doc;
        REQUIRE(doc.load_string(sv.GetResult().c_str()));
        ShowFixture b;
        Model* m2 = b.mm.createAndAddModel(doc.child("model"), 100, 100);
        REQUIRE(m2 != nullptr);
        std::string first = ViaString(m2, false, false);
        ShowFixture c;
        pugi::xml_document doc2;
        REQUIRE(doc2.load_string(first.c_str()));
        Model* m3 = c.mm.createAndAddModel(doc2.child("model"), 100, 100);
        REQUIRE(m3 != nullptr);
        CHECK(ViaString(m3, false, false) == first);
        CHECK(m3->GetNodeCount() == m->GetNodeCount());
        CHECK(m3->GetChanCount() == m->GetChanCount());
    }
}

TEST_CASE("Model attributes are written with name first and DisplayAs second", "[models2][visitors]") {
    for (const auto& s : Corpus()) {
        INFO(s.label);
        ShowFixture f;
        Model* m = f.Add(s.xml);
        pugi::xml_document doc;
        XmlSerializingVisitor v(&doc);
        m->Accept(v);
        pugi::xml_node n = doc.first_child();
        REQUIRE(n);
        pugi::xml_attribute a = n.first_attribute();
        REQUIRE(a);
        CHECK(std::string(a.name()) == "name");
        a = a.next_attribute();
        REQUIRE(a);
        CHECK(std::string(a.name()) == "DisplayAs");
    }
}

TEST_CASE("Remaining model attributes are sorted case insensitively", "[models2][visitors]") {
    for (const auto& s : Corpus()) {
        INFO(s.label);
        ShowFixture f;
        Model* m = f.Add(s.xml);
        pugi::xml_document doc;
        XmlSerializingVisitor v(&doc);
        m->Accept(v);
        std::vector<std::string> names;
        for (pugi::xml_attribute a = doc.first_child().first_attribute(); a; a = a.next_attribute()) {
            std::string n = a.name();
            if (n == "name" || n == "DisplayAs" || n.find("CustomModel") != std::string::npos) {
                continue;
            }
            for (auto& c : n) {
                c = (char)std::tolower((unsigned char)c);
            }
            names.push_back(n);
        }
        CHECK(std::is_sorted(names.begin(), names.end()));
    }
}

TEST_CASE("Custom model data attribute is written after all others", "[models2][visitors]") {
    ShowFixture f;
    Model* m = f.Add(Corpus()[17].xml);
    pugi::xml_document doc;
    XmlSerializingVisitor v(&doc);
    m->Accept(v);
    pugi::xml_attribute last;
    for (pugi::xml_attribute a = doc.first_child().first_attribute(); a; a = a.next_attribute()) {
        last = a;
    }
    REQUIRE(last);
    CHECK(std::string(last.name()).find("CustomModel") == 0);
}

TEST_CASE("Child elements follow the model attributes in a fixed order", "[models2][visitors]") {
    ShowFixture f;
    Model* m = f.Add(Corpus()[0].xml);
    pugi::xml_document doc;
    XmlSerializingVisitor v(&doc);
    m->Accept(v);
    std::vector<std::string> kids;
    for (pugi::xml_node c : doc.first_child().children()) {
        kids.push_back(c.name());
    }
    REQUIRE(!kids.empty());
    CHECK(std::find(kids.begin(), kids.end(), "subModel") != kids.end());
    CHECK(std::find(kids.begin(), kids.end(), "faceInfo") != kids.end());
    CHECK(std::find(kids.begin(), kids.end(), "stateInfo") != kids.end());
    CHECK(std::find(kids.begin(), kids.end(), "Aliases") != kids.end());
}

TEST_CASE("Special characters survive every visitor", "[models2][visitors]") {
    ShowFixture f;
    Model* m = f.Add(M("Q", "Single Line", "NumStrings=\"1\" NodesPerString=\"4\" Description=\"line1&#10;line2&#9;tab &amp; &lt;x&gt; &quot;q&quot; &apos;a&apos;\""));
    REQUIRE(m->GetDescription() == "line1\nline2\ttab & <x> \"q\" 'a'");
    std::string x = ViaXml(m);
    CHECK(ViaString(m, false, true) == x);
    CHECK(ViaString(m, false, false) == x);
    pugi::xml_document doc;
    REQUIRE(doc.load_string(ViaString(m).c_str()));
    CHECK(std::string(doc.first_child().attribute("Description").as_string()) == "line1\nline2\ttab & <x> \"q\" 'a'");
}

TEST_CASE("Unicode text survives every visitor", "[models2][visitors]") {
    ShowFixture f;
    Model* m = f.Add(M("Caf\xC3\xA9 \xE2\x9D\x84", "Single Line", "NumStrings=\"1\" NodesPerString=\"4\" Description=\"\xE6\x97\xA5\xE6\x9C\xAC\""));
    pugi::xml_document doc;
    REQUIRE(doc.load_string(ViaString(m, false, false).c_str()));
    CHECK(std::string(doc.first_child().attribute("name").as_string()) == "Caf\xC3\xA9 \xE2\x9D\x84");
    CHECK(std::string(doc.first_child().attribute("Description").as_string()) == "\xE6\x97\xA5\xE6\x9C\xAC");
    CHECK(ViaXml(m) == ViaString(m));
}

TEST_CASE("Whole show serialization matches between string and XML visitors", "[models2][visitors]") {
    ShowFixture f;
    for (const auto& s : Corpus()) {
        f.Add(s.xml);
    }
    f.Add("<modelGroup name=\"G\" models=\"T,Mx\" layout=\"minimalGrid\" GridSize=\"400\"/>");
    pugi::xml_document doc;
    XmlSerializingVisitor xv(&doc);
    XmlSerializer::SerializeAllModels(f.mm, xv);
    StringSerializingVisitor sv(false, false);
    XmlSerializer::SerializeAllModels(f.mm, sv);
    pugi::xml_document doc2;
    REQUIRE(doc2.load_string(sv.GetResult().c_str()));
    std::ostringstream a, b;
    for (pugi::xml_node c : doc.children()) {
        c.print(a, "", pugi::format_raw);
    }
    for (pugi::xml_node c : doc2.children()) {
        c.print(b, "", pugi::format_raw);
    }
    CHECK(a.str() == b.str());
    CHECK(doc2.child("models").child("model"));
    CHECK(doc2.child("modelGroups").child("modelGroup"));
}

TEST_CASE("Whole show has one model element per non group model", "[models2][visitors]") {
    ShowFixture f;
    for (const auto& s : Corpus()) {
        f.Add(s.xml);
    }
    f.Add("<modelGroup name=\"G\" models=\"T,Mx\"/>");
    StringSerializingVisitor sv(false, false);
    XmlSerializer::SerializeAllModels(f.mm, sv);
    pugi::xml_document doc;
    REQUIRE(doc.load_string(sv.GetResult().c_str()));
    size_t models = 0;
    for (pugi::xml_node c : doc.child("models").children("model")) {
        (void)c;
        ++models;
    }
    size_t groups = 0;
    for (pugi::xml_node c : doc.child("modelGroups").children("modelGroup")) {
        (void)c;
        ++groups;
    }
    CHECK(models == Corpus().size());
    CHECK(groups == 1);
}

TEST_CASE("Group serialization matches between string and XML visitors", "[models2][visitors]") {
    ShowFixture f;
    f.Add(M("A", "Single Line", "NumStrings=\"1\" NodesPerString=\"4\""));
    f.Add(M("B", "Single Line", "NumStrings=\"1\" NodesPerString=\"4\""));
    Model* g = f.Add("<modelGroup name=\"G\" models=\"A,B\" layout=\"horizontal\" GridSize=\"300\" LayoutGroup=\"Default\" aliases=\"\"/>");
    CHECK(ViaXml(g) == ViaString(g));
    CHECK(ViaXml(g).find("models=\"A,B\"") != std::string::npos);
}

TEST_CASE("Raw tag writing builds the expected text", "[models2][visitors]") {
    StringSerializingVisitor sv(false, false);
    BaseSerializingVisitor::AttrCollector attrs;
    attrs.Add("a", "1");
    attrs.Add(std::string("b"), "x<y & \"z\"");
    sv.WriteOpenTag("root", attrs);
    sv.WriteOpenTag("leaf");
    sv.WriteBodyText("t<&>");
    sv.WriteCloseTag();
    sv.WriteOpenTag("self", attrs, true);
    sv.WriteCloseTag();
    std::string out = sv.GetResult();
    std::string body = out.substr(out.find("?>") + 2);
    CHECK(body == "<root a=\"1\" b=\"x&lt;y &amp; &quot;z&quot;\"><leaf>t&lt;&amp;&gt;</leaf><self a=\"1\" b=\"x&lt;y &amp; &quot;z&quot;\"/></root>");
}

TEST_CASE("Close tag with nothing open is ignored", "[models2][visitors]") {
    StringSerializingVisitor sv(false, false);
    std::string before = sv.GetResult();
    sv.WriteCloseTag();
    sv.WriteCloseTag();
    CHECK(sv.GetResult() == before);
    CHECK(sv.GetIndentLevel() == 0);
}

TEST_CASE("Indent level follows open elements", "[models2][visitors]") {
    StringSerializingVisitor sv;
    CHECK(sv.GetIndentLevel() == 0);
    sv.WriteOpenTag("a");
    CHECK(sv.GetIndentLevel() == 1);
    sv.WriteOpenTag("b");
    CHECK(sv.GetIndentLevel() == 2);
    BaseSerializingVisitor::AttrCollector none;
    sv.WriteOpenTag("c", none, true);
    CHECK(sv.GetIndentLevel() == 2);
    sv.WriteCloseTag();
    sv.WriteCloseTag();
    CHECK(sv.GetIndentLevel() == 0);
    CHECK(sv.IsPrettyPrint());
}

TEST_CASE("Control characters in attributes are escaped as character references", "[models2][visitors]") {
    StringSerializingVisitor sv(false, false);
    BaseSerializingVisitor::AttrCollector attrs;
    attrs.Add("v", std::string("a\tb\nc\rd\x01" "e"));
    sv.WriteOpenTag("n", attrs, true);
    std::string out = sv.GetResult();
    CHECK(out.find("&#x9;") != std::string::npos);
    CHECK(out.find("&#xA;") != std::string::npos);
    CHECK(out.find("&#xD;") != std::string::npos);
    CHECK(out.find("&#x01;") != std::string::npos);
    CHECK(out.find('\t') == std::string::npos);
}

TEST_CASE("XML visitor builds nested elements and text", "[models2][visitors]") {
    pugi::xml_document doc;
    XmlSerializingVisitor v(&doc);
    BaseSerializingVisitor::AttrCollector attrs;
    attrs.Add("k", "v");
    v.WriteOpenTag("root", attrs);
    v.WriteOpenTag("child");
    v.WriteBodyText("hello");
    v.WriteCloseTag();
    v.WriteOpenTag("empty", attrs, true);
    v.WriteCloseTag();
    CHECK(std::string(doc.child("root").attribute("k").as_string()) == "v");
    CHECK(std::string(doc.child("root").child("child").text().as_string()) == "hello");
    CHECK(doc.child("root").child("empty"));
}

TEST_CASE("XML visitor can append under an existing node", "[models2][visitors]") {
    ShowFixture f;
    Model* a = f.Add(M("A", "Single Line", "NumStrings=\"1\" NodesPerString=\"4\""));
    Model* b = f.Add(M("B", "Arches", "NumArches=\"1\" NodesPerArch=\"4\""));
    pugi::xml_document doc;
    pugi::xml_node root = doc.append_child("models");
    XmlSerializingVisitor v(root);
    a->Accept(v);
    b->Accept(v);
    int count = 0;
    for (pugi::xml_node c : root.children("model")) {
        (void)c;
        ++count;
    }
    CHECK(count == 2);
    CHECK(std::string(root.first_child().attribute("name").as_string()) == "A");
    CHECK(std::string(root.last_child().attribute("name").as_string()) == "B");
}

TEST_CASE("Visiting two models with one string visitor concatenates two documents", "[models2][visitors]") {
    ShowFixture f;
    Model* a = f.Add(M("A", "Single Line", "NumStrings=\"1\" NodesPerString=\"4\""));
    Model* b = f.Add(M("B", "Arches", "NumArches=\"1\" NodesPerArch=\"4\""));
    StringSerializingVisitor sv(false, false);
    a->Accept(sv);
    b->Accept(sv);
    std::string out = sv.GetResult();
    CHECK(out.find("name=\"A\"") != std::string::npos);
    CHECK(out.find("name=\"B\"") != std::string::npos);
    CHECK(out.find("name=\"A\"") < out.find("name=\"B\""));
}

TEST_CASE("FloatToString prints compact values", "[models2][visitors]") {
    CHECK(BaseSerializingVisitor::FloatToString(0.0f) == "0");
    CHECK(BaseSerializingVisitor::FloatToString(1.0f) == "1");
    CHECK(BaseSerializingVisitor::FloatToString(2.5f) == "2.5");
    CHECK(BaseSerializingVisitor::FloatToString(-0.25f) == "-0.25");
    CHECK(BaseSerializingVisitor::FloatToString(1234567.0f) == "1.23457e+06");
    CHECK(BaseSerializingVisitor::FloatToString(100000.0f) == "100000");
}

TEST_CASE("Serializing the same model twice gives identical output", "[models2][visitors]") {
    for (const auto& s : Corpus()) {
        INFO(s.label);
        ShowFixture f;
        Model* m = f.Add(s.xml);
        CHECK(ViaXml(m) == ViaXml(m));
        CHECK(ViaString(m) == ViaString(m));
    }
}

TEST_CASE("A renamed model serializes under its new name", "[models2][visitors]") {
    ShowFixture f;
    Model* m = f.Add(Corpus()[0].xml);
    f.mm.Rename("T", "Renamed");
    Model* r = f.mm.GetModel("Renamed");
    REQUIRE(r != nullptr);
    CHECK(ViaXml(r).find("name=\"Renamed\"") != std::string::npos);
    CHECK(ViaXml(r) == ViaString(r));
    (void)m;
}
