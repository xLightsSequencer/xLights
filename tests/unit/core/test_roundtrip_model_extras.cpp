#include <set>

#include <catch2/catch_approx.hpp>

#include "test_roundtrip_common.h"

using namespace rt;

namespace {

struct Cycle {
    std::string s1, s2, s3;
    bool ok = false;
};

Cycle RunCycle(const std::string& xml) {
    Cycle c;
    ShowFixture a;
    Model* m = a.Add(xml);
    if (m == nullptr) return c;
    c.s1 = Ser(m);
    ShowFixture b;
    Model* m2 = b.Add(c.s1);
    if (m2 == nullptr) return c;
    c.s2 = Ser(m2);
    ShowFixture d;
    Model* m3 = d.Add(c.s2);
    if (m3 == nullptr) return c;
    c.s3 = Ser(m3);
    c.ok = true;
    return c;
}

void CheckStableCycle(const std::string& xml) {
    Cycle c = RunCycle(xml);
    REQUIRE(c.ok);
    INFO("save1 vs save2:" << DiffAttrs(c.s1, c.s2));
    CHECK(c.s2 == c.s1);
    CHECK(c.s3 == c.s2);
}

std::string Line(const Attrs& extra, const std::string& children = "", const std::string& name = "M") {
    Attrs a = { { "NumStrings", "1" }, { "NodesPerString", "12" } };
    a.insert(a.end(), extra.begin(), extra.end());
    return ModelXml(name, "Single Line", a, children);
}

// Combinations that crash/hang (see the [.][known-crash] cases) or are known not to settle (see the [!mayfail] cases).
bool KnownBad(const std::string& type, const std::string& attr, const std::string& v) {
    bool huge = v == "2147483647" || v == "99999999999";
    if (type == "Single Line" && attr == "LightsPerNode" && huge) return true;
    if (type == "Cube" && attr == "Strings" && huge) return true;
    if (type == "DmxGeneral" && (attr == "DmxRedChannel" || attr == "DmxGreenChannel" || attr == "DmxBlueChannel") && v.find('-') == 0) return true;
    return false;
}

pugi::xml_node FirstChild(pugi::xml_document& d, const std::string& ser, const char* name) {
    REQUIRE(d.load_string(ser.c_str()));
    return d.first_child().child(name);
}

} // namespace

// ---- special characters in every free-text field ----------------------------------------------

#define SPECIAL_CASE(ID, TEXT)                                                                      \
    TEST_CASE("Roundtrip model text survives: " ID, "[roundtrip][models][text]") {                  \
        const std::string t = TEXT;                                                                 \
        ShowFixture a;                                                                              \
        Model* m = a.Add(Line({ { "Description", t }, { "LayoutGroup", t } }, "", "Name " + t));    \
        REQUIRE(m != nullptr);                                                                      \
        CHECK(m->GetDescription() == t);                                                            \
        std::string s1 = Ser(m);                                                                    \
        ShowFixture b;                                                                              \
        Model* m2 = b.Add(s1);                                                                      \
        REQUIRE(m2 != nullptr);                                                                     \
        CHECK(m2->GetDescription() == t);                                                           \
        CHECK(m2->GetName() == m->GetName());                                                       \
        CHECK(m2->GetLayoutGroup() == m->GetLayoutGroup());                                         \
        CHECK(Ser(m2) == s1);                                                                       \
    }

SPECIAL_CASE("ampersand", "a & b")
SPECIAL_CASE("angle brackets", "<tag>x</tag>")
SPECIAL_CASE("quotes", "he said \"hi\" and 'bye'")
SPECIAL_CASE("newline", "line1\nline2")
SPECIAL_CASE("tab", "col1\tcol2")
SPECIAL_CASE("carriage return", "a\r\nb")
SPECIAL_CASE("unicode snowman", "\xE2\x98\x83 snow")
SPECIAL_CASE("unicode accents", "caf\xC3\xA9 \xC3\xB1")
SPECIAL_CASE("unicode emoji", "\xF0\x9F\x8E\x84 tree")
SPECIAL_CASE("entities lookalike", "&amp; &lt; &#65;")
SPECIAL_CASE("cdata lookalike", "<![CDATA[x]]>")
SPECIAL_CASE("comment lookalike", "<!-- c -->")
SPECIAL_CASE("percent and backslash", "100% C:\\dir\\file")
SPECIAL_CASE("comma and semicolon", "a,b;c")
SPECIAL_CASE("leading trailing space", "  padded  ")
SPECIAL_CASE("long text", std::string(5000, 'x'))

TEST_CASE("Roundtrip model empty description and group are stable", "[roundtrip][models][text]") {
    CheckStableCycle(Line({ { "Description", "" }, { "LayoutGroup", "" } }));
}
TEST_CASE("Roundtrip model default layout group name is stable", "[roundtrip][models][text]") {
    CheckStableCycle(Line({ { "LayoutGroup", "Default" } }));
}
TEST_CASE("Roundtrip model name with leading and trailing space", "[roundtrip][models][text]") {
    ShowFixture a;
    Model* m = a.Add(Line({}, "", "  spaced  "));
    REQUIRE(m != nullptr);
    std::string s1 = Ser(m);
    ShowFixture b;
    Model* m2 = b.Add(s1);
    REQUIRE(m2 != nullptr);
    CHECK(m2->GetName() == m->GetName());
    CHECK(Ser(m2) == s1);
}

// ---- extreme numeric values: one cycle may normalise, the next must be stable ----------------------------

#define EXTREME_CASE(ID, XML_ATTRS_TYPE, BASE, ATTRS)                                                         \
    TEST_CASE("Roundtrip extreme values settle after one cycle: " ID, "[roundtrip][models][extreme]") {      \
        const std::vector<std::string> vals = { "0", "-1", "-2147483648", "2147483647", "99999999999", "1e30", "-1e30", "0.0000001", "abc", "", "nan", "inf" }; \
        const std::vector<std::string> attrs = ATTRS;                                                         \
        for (auto& attr : attrs) {                                                                            \
            for (auto& v : vals) {                                                                            \
                INFO(attr << " = " << v);                                                                     \
                if (KnownBad(XML_ATTRS_TYPE, attr, v)) continue;                                              \
                Attrs at = BASE;                                                                              \
                at.emplace_back(attr, v);                                                                     \
                ShowFixture a;                                                                                \
                pugi::xml_document doc;                                                                       \
                std::string x = ModelXml("E", XML_ATTRS_TYPE, at);                                            \
                REQUIRE(doc.load_string(x.c_str()));                                                          \
                Model* m = nullptr;                                                                           \
                try { m = a.mm.createAndAddModel(doc.first_child(), 100, 100); } catch (...) { m = nullptr; } \
                if (m == nullptr) continue;                                                                   \
                std::string s1 = Ser(m);                                                                      \
                ShowFixture b;                                                                                \
                Model* m2 = b.Add(s1);                                                                        \
                REQUIRE(m2 != nullptr);                                                                       \
                std::string s2 = Ser(m2);                                                                     \
                ShowFixture c;                                                                                \
                Model* m3 = c.Add(s2);                                                                        \
                REQUIRE(m3 != nullptr);                                                                       \
                INFO("save1 vs save2:" << DiffAttrs(s1, s2));                                                 \
                CHECK(s2 == s1);                                                                              \
                CHECK(Ser(m3) == s2);                                                                         \
            }                                                                                                 \
        }                                                                                                     \
    }

#define LOC_ATTRS "WorldPosX", "WorldPosY", "WorldPosZ", "ScaleX", "ScaleY", "ScaleZ", "RotateX", "RotateY", "RotateZ", "Transparency", "BlackTransparency", "PixelSize", "Antialias"

EXTREME_CASE("Single Line", "Single Line", (Attrs{ { "NumStrings", "2" }, { "NodesPerString", "5" } }), (std::vector<std::string>{ LOC_ATTRS, "NumStrings", "NodesPerString", "LightsPerNode", "X2", "Y2", "Z2" }))
EXTREME_CASE("Matrix", "Matrix", (Attrs{ { "NumStrings", "2" }, { "NodesPerString", "6" }, { "StrandsPerString", "2" } }), (std::vector<std::string>{ LOC_ATTRS, "NumStrings", "NodesPerString", "StrandsPerString", "LightsPerNode" }))
EXTREME_CASE("Tree", "Tree 360", (Attrs{ { "NumStrings", "2" }, { "NodesPerString", "6" }, { "StrandsPerString", "1" } }), (std::vector<std::string>{ LOC_ATTRS, "NumStrings", "NodesPerString", "TreeSpiralRotations", "TreeBottomTopRatio", "TreePerspective", "TreeRotation", "TreeDegrees" }))
EXTREME_CASE("Arches", "Arches", (Attrs{ { "NumArches", "2" }, { "NodesPerArch", "5" } }), (std::vector<std::string>{ LOC_ATTRS, "NumArches", "NodesPerArch", "Hollow", "Arc", "Gap", "Angle", "Height", "Shear", "X2", "Y2" }))
EXTREME_CASE("Candy Canes", "Candy Canes", (Attrs{ { "NumCanes", "2" }, { "NodesPerCane", "5" } }), (std::vector<std::string>{ LOC_ATTRS, "NumCanes", "NodesPerCane", "CandyCaneHeight", "Angle", "Height" }))
EXTREME_CASE("Star", "Star", (Attrs{ { "NumStrings", "1" }, { "NodesPerString", "10" } }), (std::vector<std::string>{ LOC_ATTRS, "NumStrings", "NodesPerString", "StarPoints", "starRatio", "starCenterPercent" }))
EXTREME_CASE("Circle", "Circle", (Attrs{ { "NumStrings", "1" }, { "NodesPerString", "10" } }), (std::vector<std::string>{ LOC_ATTRS, "NumStrings", "NodesPerString", "centerPercent" }))
EXTREME_CASE("Spinner", "Spinner", (Attrs{ { "NumStrings", "1" }, { "NodesPerArm", "3" }, { "ArmsPerString", "3" } }), (std::vector<std::string>{ LOC_ATTRS, "NumStrings", "NodesPerArm", "ArmsPerString", "Hollow", "StartAngle", "Arc" }))
EXTREME_CASE("Sphere", "Sphere", (Attrs{ { "NumStrings", "2" }, { "NodesPerString", "6" }, { "StrandsPerString", "1" } }), (std::vector<std::string>{ LOC_ATTRS, "NumStrings", "NodesPerString", "StrandsPerString", "Degrees", "StartLatitude", "EndLatitude" }))
EXTREME_CASE("Window Frame", "Window Frame", (Attrs{ { "TopNodes", "3" }, { "SideNodes", "2" }, { "BottomNodes", "3" } }), (std::vector<std::string>{ LOC_ATTRS, "TopNodes", "SideNodes", "BottomNodes" }))
EXTREME_CASE("Cube", "Cube", (Attrs{ { "CubeWidth", "2" }, { "CubeHeight", "2" }, { "CubeDepth", "2" } }), (std::vector<std::string>{ LOC_ATTRS, "CubeWidth", "CubeHeight", "CubeDepth", "Strings", "CubeHollow", "CubeShape", "CubeRowOffset" }))
EXTREME_CASE("Label", "Label", (Attrs{ { "LabelText", "x" } }), (std::vector<std::string>{ LOC_ATTRS, "LabelFontSize", "LabelTextColor" }))
EXTREME_CASE("Image", "Image", (Attrs{ { "Image", "" } }), (std::vector<std::string>{ LOC_ATTRS, "OffBrightness", "WhiteAsAlpha" }))
EXTREME_CASE("Channel Block", "Channel Block", (Attrs{ { "NumChannels", "4" } }), (std::vector<std::string>{ LOC_ATTRS, "NumChannels" }))
EXTREME_CASE("DmxGeneral", "DmxGeneral", (Attrs{ { "DmxChannelCount", "6" }, { "StringType", "Single Color White" } }), (std::vector<std::string>{ LOC_ATTRS, "DmxChannelCount", "DmxRedChannel", "DmxGreenChannel", "DmxBlueChannel", "DmxColorType" }))
EXTREME_CASE("DmxMovingHeadAdv", "DmxMovingHeadAdv", (Attrs{ { "DmxChannelCount", "20" }, { "StringType", "Single Color White" } }), (std::vector<std::string>{ "DmxChannelCount", "DmxBeamLength", "DmxBeamWidth", "DmxBeamYOffset", "DmxShutterChannel", "DmxShutterOpen", "MhDimmerChannel" }))
EXTREME_CASE("DmxServo", "DmxServo", (Attrs{ { "NumServos", "2" }, { "DmxChannelCount", "4" }, { "StringType", "Single Color White" } }), (std::vector<std::string>{ "NumServos", "DmxChannelCount", "Brightness", "Bits16" }))
EXTREME_CASE("DmxSkull", "DmxSkull", (Attrs{ { "DmxChannelCount", "26" }, { "StringType", "Single Color White" } }), (std::vector<std::string>{ "DmxChannelCount", "DmxJawOrient", "DmxPanOrient", "DmxEyeBrtChannel" }))

// ---- child elements ----------------------------------------------------------------------------------

TEST_CASE("Roundtrip submodel range lines are preserved in order", "[roundtrip][submodels]") {
    std::string kids = "<subModel name=\"s\" layout=\"horizontal\" type=\"ranges\" bufferstyle=\"Default\" line0=\"1-3\" line1=\"5,7-9\" line2=\"12\"/>";
    ShowFixture a;
    Model* m = a.Add(Line({}, kids));
    REQUIRE(m != nullptr);
    pugi::xml_document d;
    auto sm = FirstChild(d, Ser(m), "subModel");
    REQUIRE(sm);
    CHECK(std::string(sm.attribute("line0").value()) == "1-3");
    CHECK(std::string(sm.attribute("line1").value()) == "5,7-9");
    CHECK(std::string(sm.attribute("line2").value()) == "12");
    CheckStableCycle(Line({}, kids));
}
TEST_CASE("Roundtrip submodel subbuffer is preserved", "[roundtrip][submodels]") {
    std::string kids = "<subModel name=\"sb\" layout=\"horizontal\" type=\"subbuffer\" bufferstyle=\"Default\" subBuffer=\"10x20x50x80\"/>";
    ShowFixture a;
    Model* m = a.Add(Line({}, kids));
    REQUIRE(m != nullptr);
    pugi::xml_document d;
    auto sm = FirstChild(d, Ser(m), "subModel");
    REQUIRE(sm);
    CHECK(std::string(sm.attribute("subBuffer").value()) == "10x20x50x80");
    CHECK(std::string(sm.attribute("type").value()) == "subbuffer");
    CheckStableCycle(Line({}, kids));
}
TEST_CASE("Roundtrip submodel layouts and buffer styles are preserved", "[roundtrip][submodels]") {
    for (const char* layout : { "horizontal", "vertical" }) {
        for (const char* bs : { "Default", "Keep XY", "Stacked", "Per Model Default" }) {
            INFO(layout << " / " << bs);
            std::string kids = std::string("<subModel name=\"s\" layout=\"") + layout + "\" type=\"ranges\" bufferstyle=\"" + bs + "\" line0=\"1-6\"/>";
            ShowFixture a;
            Model* m = a.Add(Line({}, kids));
            REQUIRE(m != nullptr);
            pugi::xml_document d;
            auto sm = FirstChild(d, Ser(m), "subModel");
            REQUIRE(sm);
            CHECK(std::string(sm.attribute("layout").value()) == layout);
            CHECK(std::string(sm.attribute("bufferstyle").value()) == bs);
            CheckStableCycle(Line({}, kids));
        }
    }
}
TEST_CASE("Roundtrip many submodels keep order and names", "[roundtrip][submodels]") {
    std::string kids;
    for (int i = 0; i < 25; ++i) {
        kids += "<subModel name=\"sub" + std::to_string(i) + "\" layout=\"horizontal\" type=\"ranges\" bufferstyle=\"Default\" line0=\"" + std::to_string(i % 12 + 1) + "\"/>";
    }
    ShowFixture a;
    Model* m = a.Add(Line({}, kids));
    REQUIRE(m != nullptr);
    pugi::xml_document d;
    REQUIRE(d.load_string(Ser(m).c_str()));
    int i = 0;
    for (auto c : d.first_child().children("subModel")) {
        CHECK(std::string(c.attribute("name").value()) == "sub" + std::to_string(i));
        ++i;
    }
    CHECK(i == 25);
    CheckStableCycle(Line({}, kids));
}
TEST_CASE("Roundtrip submodel names with specials", "[roundtrip][submodels]") {
    for (const char* n : { "a&b", "<x>", "q\"uote", "uni\xC3\xA9", "with space", "tab\there", "new\nline", "comma,name", "a/b", "1" }) {
        INFO(n);
        std::string kids = "<subModel name=\"" + Esc(n) + "\" layout=\"horizontal\" type=\"ranges\" bufferstyle=\"Default\" line0=\"1-3\"/>";
        ShowFixture a;
        Model* m = a.Add(Line({}, kids));
        REQUIRE(m != nullptr);
        std::string s1 = Ser(m);
        pugi::xml_document d;
        auto sm = FirstChild(d, s1, "subModel");
        REQUIRE(sm);
        std::string got = sm.attribute("name").value();
        ShowFixture b;
        Model* m2 = b.Add(s1);
        REQUIRE(m2 != nullptr);
        pugi::xml_document d2;
        auto sm2 = FirstChild(d2, Ser(m2), "subModel");
        REQUIRE(sm2);
        CHECK(std::string(sm2.attribute("name").value()) == got);
        CHECK(Ser(m2) == s1);
    }
}
TEST_CASE("Roundtrip submodel aliases survive", "[roundtrip][submodels]") {
    std::string kids = "<subModel name=\"s\" layout=\"horizontal\" type=\"ranges\" bufferstyle=\"Default\" line0=\"1-3\"><Aliases><alias name=\"first\"/><alias name=\"second\"/></Aliases></subModel>";
    ShowFixture a;
    Model* m = a.Add(Line({}, kids));
    REQUIRE(m != nullptr);
    pugi::xml_document d;
    auto sm = FirstChild(d, Ser(m), "subModel");
    REQUIRE(sm);
    int n = 0;
    for (auto al : sm.child("Aliases").children("alias")) {
        (void)al;
        ++n;
    }
    CHECK(n == 2);
    CheckStableCycle(Line({}, kids));
}
TEST_CASE("Roundtrip submodel with empty range is stable", "[roundtrip][submodels]") {
    CheckStableCycle(Line({}, "<subModel name=\"e\" layout=\"horizontal\" type=\"ranges\" bufferstyle=\"Default\" line0=\"\"/>"));
}
TEST_CASE("Roundtrip submodel out of range nodes are stable", "[roundtrip][submodels]") {
    CheckStableCycle(Line({}, "<subModel name=\"o\" layout=\"horizontal\" type=\"ranges\" bufferstyle=\"Default\" line0=\"1-9999,0,-3\"/>"));
}
TEST_CASE("Roundtrip duplicate submodel names are stable", "[roundtrip][submodels]") {
    CheckStableCycle(Line({}, "<subModel name=\"d\" type=\"ranges\" line0=\"1\"/><subModel name=\"d\" type=\"ranges\" line0=\"2\"/>"));
}

// ---- faces and states -----------------------------------------------------------------------------

TEST_CASE("Roundtrip node range face keeps all phoneme keys", "[roundtrip][faces]") {
    const char* keys[] = { "AI", "E", "FV", "L", "MBP", "O", "U", "WQ", "etc", "rest", "Eyes-Open", "Eyes-Closed", "Eyes-Open-Mouth", "Eyes-Closed-Mouth" };
    std::string f = "<faceInfo Name=\"Face\" Type=\"NodeRange\" CustomColors=\"1\"";
    int i = 1;
    for (auto k : keys) {
        f += std::string(" ") + k + "=\"" + std::to_string(i) + "-" + std::to_string(i + 1) + "\" " + k + "-Color=\"#0000FF\"";
        i += 2;
    }
    f += "/>";
    ShowFixture a;
    Model* m = a.Add(Line({}, f));
    REQUIRE(m != nullptr);
    pugi::xml_document d;
    auto fi = FirstChild(d, Ser(m), "faceInfo");
    REQUIRE(fi);
    CHECK(std::string(fi.attribute("Name").value()) == "Face");
    CHECK(std::string(fi.attribute("Type").value()) == "NodeRange");
    i = 1;
    for (auto k : keys) {
        INFO(k);
        std::string want = std::to_string(i) + "-" + std::to_string(i + 1);
        i += 2;
        CHECK(std::string(fi.attribute(k).value()) == want);
    }
    CheckStableCycle(Line({}, f));
}
TEST_CASE("Roundtrip matrix face survives", "[roundtrip][faces]") {
    std::string f = "<faceInfo Name=\"MxFace\" Type=\"Matrix\" CustomColors=\"0\" ImagePlacement=\"Centre\" AI=\"a.png\" E=\"b.png\"/>";
    CheckStableCycle(Line({}, f));
}
TEST_CASE("Roundtrip multiple faces and states keep order", "[roundtrip][faces]") {
    std::string kids;
    for (int i = 0; i < 6; ++i) {
        kids += "<faceInfo Name=\"face" + std::to_string(i) + "\" Type=\"NodeRange\" CustomColors=\"0\" AI=\"" + std::to_string(i + 1) + "\"/>";
    }
    for (int i = 0; i < 6; ++i) {
        kids += "<stateInfo Name=\"state" + std::to_string(i) + "\" Type=\"NodeRange\" CustomColors=\"0\" s1=\"" + std::to_string(i + 1) + "\" s1-Name=\"n" + std::to_string(i) + "\"/>";
    }
    ShowFixture a;
    Model* m = a.Add(Line({}, kids));
    REQUIRE(m != nullptr);
    std::string s1 = Ser(m);
    pugi::xml_document d;
    REQUIRE(d.load_string(s1.c_str()));
    std::set<std::string> names;
    for (auto c : d.first_child().children("faceInfo")) names.insert(c.attribute("Name").value());
    for (auto c : d.first_child().children("stateInfo")) names.insert(c.attribute("Name").value());
    CHECK(names.size() == 12);
    CheckStableCycle(Line({}, kids));
}
TEST_CASE("Roundtrip state with several named states and colours", "[roundtrip][faces]") {
    std::string st = "<stateInfo Name=\"St\" Type=\"NodeRange\" CustomColors=\"1\" s1=\"1-3\" s1-Name=\"One\" s1-Color=\"#FF0000\" s2=\"4-6\" s2-Name=\"Two\" s2-Color=\"#00FF00\" s3=\"7-9\" s3-Name=\"Three\" s3-Color=\"#0000FF\"/>";
    ShowFixture a;
    Model* m = a.Add(Line({}, st));
    REQUIRE(m != nullptr);
    pugi::xml_document d;
    auto si = FirstChild(d, Ser(m), "stateInfo");
    REQUIRE(si);
    CHECK(std::string(si.attribute("s001-Name").value()) == "One");
    CHECK(std::string(si.attribute("s002-Name").value()) == "Two");
    CHECK(std::string(si.attribute("s003-Name").value()) == "Three");
    CHECK(std::string(si.attribute("s002").value()) == "4-6");
    CheckStableCycle(Line({}, st));
}
TEST_CASE("Roundtrip state names with specials", "[roundtrip][faces]") {
    for (const char* n : { "A&B", "<st>", "quo\"te", "caf\xC3\xA9", "sp ace", "dash-name", "dot.name" }) {
        INFO(n);
        std::string st = "<stateInfo Name=\"" + Esc(n) + "\" Type=\"NodeRange\" CustomColors=\"0\" s1=\"1-3\" s1-Name=\"" + Esc(n) + "\"/>";
        ShowFixture a;
        Model* m = a.Add(Line({}, st));
        REQUIRE(m != nullptr);
        std::string s1 = Ser(m);
        pugi::xml_document d;
        auto si = FirstChild(d, s1, "stateInfo");
        REQUIRE(si);
        CHECK(std::string(si.attribute("Name").value()) == n);
        CHECK(std::string(si.attribute("s001-Name").value()) == n);
        CheckStableCycle(Line({}, st));
    }
}
TEST_CASE("Roundtrip legacy unpadded state keys are rewritten once", "[roundtrip][faces]") {
    std::string st = "<stateInfo Name=\"St\" Type=\"NodeRange\" CustomColors=\"0\" s1=\"1-3\" s10=\"4-6\" s2=\"7\"/>";
    Cycle c = RunCycle(Line({}, st));
    REQUIRE(c.ok);
    CHECK(c.s2 == c.s1);
    pugi::xml_document d;
    auto si = FirstChild(d, c.s1, "stateInfo");
    CHECK(std::string(si.attribute("s001").value()) == "1-3");
    CHECK(std::string(si.attribute("s002").value()) == "7");
    CHECK(std::string(si.attribute("s010").value()) == "4-6");
}
TEST_CASE("Roundtrip single node state type", "[roundtrip][faces]") {
    CheckStableCycle(Line({}, "<stateInfo Name=\"Sn\" Type=\"SingleNode\" CustomColors=\"0\" s1=\"1\" s1-Name=\"x\" s2=\"2\" s2-Name=\"y\"/>"));
}

// ---- aliases, dimming curve, controller connection ---------------------------------------------------------

TEST_CASE("Roundtrip model aliases are lowered once and keep order", "[roundtrip][models]") {
    std::string kids = "<Aliases><alias name=\"Zed\"/><alias name=\"Alpha\"/><alias name=\"mid dle\"/><alias name=\"x&amp;y\"/></Aliases>";
    ShowFixture a;
    Model* m = a.Add(Line({}, kids));
    REQUIRE(m != nullptr);
    pugi::xml_document d;
    auto al = FirstChild(d, Ser(m), "Aliases");
    REQUIRE(al);
    std::vector<std::string> got;
    for (auto c : al.children("alias")) got.push_back(c.attribute("name").value());
    CHECK(got == std::vector<std::string>{ "zed", "alpha", "mid dle", "x&y" });
    CheckStableCycle(Line({}, kids));
}
TEST_CASE("Roundtrip model duplicate aliases are collapsed and stable", "[roundtrip][models]") {
    CheckStableCycle(Line({}, "<Aliases><alias name=\"dup\"/><alias name=\"DUP\"/></Aliases>"));
}
TEST_CASE("Roundtrip dimming curve all channels", "[roundtrip][models]") {
    std::string kids = "<dimmingCurve><all gamma=\"2.2\" brightness=\"-20\"/></dimmingCurve>";
    ShowFixture a;
    Model* m = a.Add(Line({}, kids));
    REQUIRE(m != nullptr);
    pugi::xml_document d;
    auto dc = FirstChild(d, Ser(m), "dimmingCurve");
    REQUIRE(dc);
    CHECK(dc.child("all").attribute("gamma").as_float() == Catch::Approx(2.2f));
    CHECK(dc.child("all").attribute("brightness").as_int() == -20);
    CheckStableCycle(Line({}, kids));
}
TEST_CASE("Roundtrip dimming curve per channel", "[roundtrip][models]") {
    std::string kids = "<dimmingCurve><red gamma=\"1.5\" brightness=\"5\"/><green gamma=\"2.5\" brightness=\"-5\"/><blue gamma=\"3.5\" brightness=\"15\"/></dimmingCurve>";
    ShowFixture a;
    Model* m = a.Add(Line({}, kids));
    REQUIRE(m != nullptr);
    pugi::xml_document d;
    auto dc = FirstChild(d, Ser(m), "dimmingCurve");
    REQUIRE(dc);
    CHECK(dc.child("red").attribute("gamma").as_float() == Catch::Approx(1.5f));
    CHECK(dc.child("green").attribute("brightness").as_int() == -5);
    CHECK(dc.child("blue").attribute("gamma").as_float() == Catch::Approx(3.5f));
    CheckStableCycle(Line({}, kids));
}
TEST_CASE("Roundtrip legacy ModelBrightness converts to dimming curve once", "[roundtrip][models]") {
    Cycle c = RunCycle(Line({ { "ModelBrightness", "-25" } }));
    REQUIRE(c.ok);
    CHECK(c.s2 == c.s1);
    pugi::xml_document d;
    auto dc = FirstChild(d, c.s1, "dimmingCurve");
    REQUIRE(dc);
    CHECK(dc.child("all").attribute("brightness").as_int() == -25);
    CHECK(d.first_child().attribute("ModelBrightness").empty());
}
TEST_CASE("Roundtrip dimming curve extreme values settle", "[roundtrip][models]") {
    for (const char* v : { "0", "-100", "100", "1000", "-1000", "abc", "", "0.001", "99999" }) {
        INFO(v);
        std::string kids = std::string("<dimmingCurve><all gamma=\"") + v + "\" brightness=\"" + v + "\"/></dimmingCurve>";
        Cycle c = RunCycle(Line({}, kids));
        REQUIRE(c.ok);
        CHECK(c.s3 == c.s2);
    }
}
TEST_CASE("Roundtrip controller connection port and protocol", "[roundtrip][models]") {
    std::string kids = "<ControllerConnection Port=\"7\" Protocol=\"ws2811\"/>";
    ShowFixture a;
    Model* m = a.Add(Line({ { "Controller", "Ctl 1" } }, kids));
    REQUIRE(m != nullptr);
    pugi::xml_document d;
    auto cc = FirstChild(d, Ser(m), "ControllerConnection");
    REQUIRE(cc);
    CHECK(cc.attribute("Port").as_int() == 7);
    CHECK(std::string(cc.attribute("Protocol").value()) == "ws2811");
    CHECK(std::string(d.first_child().attribute("Controller").value()) == "Ctl 1");
    CheckStableCycle(Line({ { "Controller", "Ctl 1" } }, kids));
}
TEST_CASE("Roundtrip controller connection protocols", "[roundtrip][models]") {
    for (const char* p : { "ws2811", "ws2812", "apa102", "tm1814", "gece", "dmx", "pixelnet", "renard", "lor", "dmx-open", "WS2811", "unknownproto" }) {
        INFO(p);
        std::string kids = std::string("<ControllerConnection Port=\"2\" Protocol=\"") + p + "\"/>";
        Cycle c = RunCycle(Line({ { "Controller", "C" } }, kids));
        REQUIRE(c.ok);
        CHECK(c.s2 == c.s1);
        CHECK(c.s3 == c.s2);
    }
}
TEST_CASE("Roundtrip controller connection extra properties", "[roundtrip][models]") {
    std::string kids = "<ControllerConnection Port=\"3\" Protocol=\"ws2811\" nullNodes=\"2\" endNullNodes=\"3\" brightness=\"80\" gamma=\"2.2\" colorOrder=\"GRB\" reverse=\"1\" groupCount=\"4\" zigZag=\"6\"/>";
    Cycle c = RunCycle(Line({ { "Controller", "C" } }, kids));
    REQUIRE(c.ok);
    INFO("save1 vs save2:" << DiffAttrs(c.s1, c.s2));
    CHECK(c.s2 == c.s1);
}
TEST_CASE("Roundtrip controller connection serial channel and speed", "[roundtrip][models]") {
    std::string kids = "<ControllerConnection Port=\"1\" Protocol=\"dmx\" channel=\"17\" Speed=\"250000\"/>";
    Cycle c = RunCycle(Line({ { "Controller", "C" } }, kids));
    REQUIRE(c.ok);
    CHECK(c.s2 == c.s1);
}
TEST_CASE("Roundtrip controller connection port zero is dropped consistently", "[roundtrip][models]") {
    Cycle c = RunCycle(Line({ { "Controller", "C" } }, "<ControllerConnection Port=\"0\" Protocol=\"ws2811\"/>"));
    REQUIRE(c.ok);
    CHECK(c.s2 == c.s1);
}
TEST_CASE("Roundtrip controller connection extreme ports", "[roundtrip][models]") {
    for (const char* p : { "-1", "99999", "abc", "" }) {
        INFO(p);
        Cycle c = RunCycle(Line({ { "Controller", "C" } }, std::string("<ControllerConnection Port=\"") + p + "\" Protocol=\"ws2811\"/>"));
        REQUIRE(c.ok);
        CHECK(c.s3 == c.s2);
    }
}

TEST_CASE("Roundtrip individual start channels", "[roundtrip][models]") {
    Cycle c = RunCycle(ModelXml("Ind", "Single Line", { { "NumStrings", "3" }, { "NodesPerString", "5" }, { "Advanced", "1" }, { "String1", "10" }, { "String2", "#Ctl:25" }, { "String3", "500" } }));
    REQUIRE(c.ok);
    CHECK(c.s2 == c.s1);
    CHECK(c.s3 == c.s2);
    pugi::xml_document d;
    REQUIRE(d.load_string(c.s1.c_str()));
    CHECK(std::string(d.first_child().attribute("String1").value()) == "10");
    CHECK(std::string(d.first_child().attribute("String2").value()) == "#Ctl:25");
    CHECK(std::string(d.first_child().attribute("String3").value()) == "500");
    CHECK(d.first_child().attribute("Advanced").as_int() == 1);
}
TEST_CASE("Roundtrip start channel formats", "[roundtrip][models]") {
    for (const char* sc : { "1", "513", "#1:1", "#Ctl:25", "!Other:5", "@Other:5", ">Other:5:2", "1:1", "2:100", "" }) {
        INFO(sc);
        pugi::xml_document doc;
        std::string x = "<model name=\"S\" DisplayAs=\"Single Line\" NumStrings=\"1\" NodesPerString=\"5\" StartChannel=\"" + Esc(sc) + "\"/>";
        REQUIRE(doc.load_string(x.c_str()));
        ShowFixture a;
        Model* m = a.mm.createAndAddModel(doc.first_child(), 100, 100);
        REQUIRE(m != nullptr);
        std::string s1 = Ser(m);
        pugi::xml_document d1;
        REQUIRE(d1.load_string(s1.c_str()));
        CHECK(std::string(d1.first_child().attribute("StartChannel").value()) == std::string(sc));
        ShowFixture b;
        Model* m2 = b.Add(s1);
        REQUIRE(m2 != nullptr);
        CHECK(Ser(m2) == s1);
    }
}
TEST_CASE("Roundtrip string types", "[roundtrip][models]") {
    for (const char* t : { "RGB Nodes", "RBG Nodes", "GBR Nodes", "GRB Nodes", "BRG Nodes", "BGR Nodes", "RGBW Nodes", "WRGB Nodes", "Single Color White", "Single Color Red", "Single Color Green", "Single Color Blue", "Single Color Custom", "3 Channel RGB", "4 Channel RGBW", "Strobes", "Node Single Color", "Superstring" }) {
        INFO(t);
        Cycle c = RunCycle(Line({ { "StringType", t } }));
        REQUIRE(c.ok);
        CHECK(c.s2 == c.s1);
        CHECK(c.s3 == c.s2);
        pugi::xml_document d;
        REQUIRE(d.load_string(c.s1.c_str()));
        INFO("saved as " << d.first_child().attribute("StringType").value());
    }
}
TEST_CASE("Roundtrip string types keep their value", "[roundtrip][models]") {
    for (const char* t : { "RGB Nodes", "RBG Nodes", "GBR Nodes", "GRB Nodes", "BRG Nodes", "BGR Nodes", "RGBW Nodes", "Single Color White", "3 Channel RGB", "4 Channel RGBW", "Strobes", "Node Single Color" }) {
        INFO(t);
        Cycle c = RunCycle(Line({ { "StringType", t } }));
        REQUIRE(c.ok);
        pugi::xml_document d;
        REQUIRE(d.load_string(c.s1.c_str()));
        CHECK(std::string(d.first_child().attribute("StringType").value()) == t);
    }
}
TEST_CASE("Roundtrip RGBW handling modes", "[roundtrip][models]") {
    for (const char* t : { "R=G=B -> W", "RGB Only", "Advanced", "White Only" }) {
        INFO(t);
        Cycle c = RunCycle(Line({ { "StringType", "RGBW Nodes" }, { "RGBWHandling", t } }));
        REQUIRE(c.ok);
        CHECK(c.s2 == c.s1);
        pugi::xml_document d;
        REQUIRE(d.load_string(c.s1.c_str()));
        CHECK(std::string(d.first_child().attribute("RGBWHandling").value()) == t);
    }
}
TEST_CASE("Roundtrip start side and direction values", "[roundtrip][models]") {
    for (const char* side : { "B", "T", "L", "R" }) {
        for (const char* dir : { "L", "R" }) {
            INFO(side << dir);
            Cycle c = RunCycle(ModelXml("M", "Matrix", { { "NumStrings", "2" }, { "NodesPerString", "6" }, { "StrandsPerString", "2" }, { "StartSide", side }, { "Dir", dir } }));
            REQUIRE(c.ok);
            CHECK(c.s2 == c.s1);
            pugi::xml_document d;
            REQUIRE(d.load_string(c.s1.c_str()));
            CHECK(std::string(d.first_child().attribute("StartSide").value()) == side);
            CHECK(std::string(d.first_child().attribute("Dir").value()) == dir);
        }
    }
}
TEST_CASE("Roundtrip shadow model and chain attributes", "[roundtrip][models]") {
    Cycle c = RunCycle(Line({ { "ShadowModelFor", "Other" }, { "ModelChain", ">Prev" } }));
    REQUIRE(c.ok);
    CHECK(c.s2 == c.s1);
    pugi::xml_document d;
    REQUIRE(d.load_string(c.s1.c_str()));
    CHECK(std::string(d.first_child().attribute("ShadowModelFor").value()) == "Other");
    CHECK(std::string(d.first_child().attribute("ModelChain").value()) == ">Prev");
}
TEST_CASE("Roundtrip active flag and from-base flag", "[roundtrip][models]") {
    Cycle c = RunCycle(Line({ { "Active", "0" }, { "FromBase", "1" } }));
    REQUIRE(c.ok);
    CHECK(c.s2 == c.s1);
    pugi::xml_document d;
    REQUIRE(d.load_string(c.s1.c_str()));
    CHECK(d.first_child().attribute("Active").as_int(1) == 0);
    CHECK(d.first_child().attribute("FromBase").as_int(0) == 1);
}
TEST_CASE("Roundtrip locked flag", "[roundtrip][models]") {
    Cycle c = RunCycle(Line({ { "Locked", "1" } }));
    REQUIRE(c.ok);
    CHECK(c.s2 == c.s1);
    pugi::xml_document d;
    REQUIRE(d.load_string(c.s1.c_str()));
    CHECK(d.first_child().attribute("Locked").as_int(0) == 1);
}
TEST_CASE("Roundtrip custom model data forms", "[roundtrip][models]") {
    std::string big;
    for (int r = 0; r < 20; ++r) {
        for (int c = 0; c < 30; ++c) {
            big += (c ? "," : "") + std::string(((r + c) % 3 == 0) ? std::to_string(r * 30 + c + 1) : "");
        }
        big += ";";
    }
    big.pop_back();
    for (const std::string& data : { std::string("1"), std::string("1,2,3;4,5,6;7,8,9"), std::string(",,1;,2,;3,,"), big }) {
        INFO(data.substr(0, 40));
        Cycle c = RunCycle(ModelXml("C", "Custom", { { "CustomWidth", "30" }, { "CustomHeight", "20" }, { "CustomModel", data } }));
        REQUIRE(c.ok);
        CHECK(c.s2 == c.s1);
        CHECK(c.s3 == c.s2);
    }
}
TEST_CASE("Roundtrip custom model layers", "[roundtrip][models]") {
    std::string data = "1,2;3,4|5,6;7,8|9,10;11,12";
    Cycle c = RunCycle(ModelXml("C", "Custom", { { "CustomWidth", "2" }, { "CustomHeight", "2" }, { "Depth", "3" }, { "CustomModel", data } }));
    REQUIRE(c.ok);
    CHECK(c.s2 == c.s1);
    pugi::xml_document d;
    REQUIRE(d.load_string(c.s1.c_str()));
    CHECK(d.first_child().attribute("Depth").as_int() == 3);
    CHECK(std::string(d.first_child().attribute("CustomModel").value()) == data);
}
TEST_CASE("Roundtrip custom model with mismatched dimensions settles", "[roundtrip][models]") {
    Cycle c = RunCycle(ModelXml("C", "Custom", { { "CustomWidth", "2" }, { "CustomHeight", "2" }, { "CustomModel", "1,2,3,4,5;6,7,8,9,10;11" } }));
    REQUIRE(c.ok);
    CHECK(c.s3 == c.s2);
}
TEST_CASE("Roundtrip custom model compressed data stays compressed", "[roundtrip][models]") {
    ShowFixture a;
    Model* m = a.Add(ModelXml("C", "Custom", { { "CustomWidth", "3" }, { "CustomHeight", "2" }, { "CustomModel", "1,2,3;4,5,6" } }));
    REQUIRE(m != nullptr);
    std::string s1 = Ser(m);
    ShowFixture b;
    Model* m2 = b.Add(s1);
    REQUIRE(m2 != nullptr);
    CHECK(m2->GetNodeCount() == 6);
    CHECK(Ser(m2) == s1);
}
TEST_CASE("Roundtrip poly line points", "[roundtrip][models]") {
    for (const char* pts : { "0.0,0.0,0.0,5.0,5.0,0.0", "0.0,0.0,0.0,5.0,5.0,0.0,10.0,0.0,0.0,15.0,-5.0,2.5", "-100.5,-100.5,-100.5,100.5,100.5,100.5" }) {
        int n = 2;
        for (const char* p = pts; *p; ++p) if (*p == ',') ++n;
        n = (n - 1) / 3 + (((n - 1) % 3) ? 1 : 0);
        INFO(pts);
        Cycle c = RunCycle(ModelXml("P", "Poly Line", { { "NumStrings", "1" }, { "NodesPerString", "20" }, { "NumPoints", std::to_string(n) }, { "PointData", pts } }));
        REQUIRE(c.ok);
        CHECK(c.s2 == c.s1);
        CHECK(c.s3 == c.s2);
    }
}
TEST_CASE("Roundtrip poly line curve data", "[roundtrip][models]") {
    Cycle c = RunCycle(ModelXml("P", "Poly Line", { { "NumStrings", "1" }, { "NodesPerString", "20" }, { "NumPoints", "3" }, { "PointData", "0,0,0,5,5,0,10,0,0" }, { "cPointData", "2,1.5,2.5,0.5,0.5,0.5" } }));
    REQUIRE(c.ok);
    INFO("save1 vs save2:" << DiffAttrs(c.s1, c.s2));
    CHECK(c.s2 == c.s1);
}
TEST_CASE("Roundtrip poly line corners and segments", "[roundtrip][models]") {
    Cycle c = RunCycle(ModelXml("P", "Poly Line", { { "NumStrings", "1" }, { "NodesPerString", "20" }, { "NumPoints", "4" }, { "PointData", "0,0,0,5,5,0,10,0,0,15,5,0" }, { "Corner1", "Leading Segment" }, { "Corner2", "Trailing Segment" }, { "Corner3", "Neither" }, { "IndivSegs", "5,5,5" }, { "SegsExpanded", "FALSE" } }));
    REQUIRE(c.ok);
    INFO("save1 vs save2:" << DiffAttrs(c.s1, c.s2));
    CHECK(c.s2 == c.s1);
}
TEST_CASE("Roundtrip channel block colours", "[roundtrip][models]") {
    for (int n : { 1, 2, 8, 16, 32 }) {
        Attrs at = { { "NumChannels", std::to_string(n) } };
        for (int i = 1; i <= n; ++i) {
            char col[16];
            snprintf(col, sizeof(col), "#%02X%02X%02X", (i * 37) & 255, (i * 91) & 255, (i * 13) & 255);
            at.emplace_back("ChannelProperties.ChannelColor" + std::to_string(i), col);
        }
        INFO(n);
        Cycle c = RunCycle(ModelXml("B", "Channel Block", at));
        REQUIRE(c.ok);
        CHECK(c.s2 == c.s1);
        pugi::xml_document d;
        REQUIRE(d.load_string(c.s1.c_str()));
        for (int i = 1; i <= n; ++i) {
            auto got = std::string(d.first_child().attribute(("ChannelProperties.ChannelColor" + std::to_string(i)).c_str()).value());
            CHECK(Same(got, at[i].second));
        }
    }
}
TEST_CASE("Roundtrip icicle drop patterns", "[roundtrip][models]") {
    for (const char* dp : { "1", "3,1,2", "5,4,3,2,1", "10", "0", "-1,2", "a,b", "" }) {
        INFO(dp);
        Cycle c = RunCycle(ModelXml("I", "Icicles", { { "NumStrings", "2" }, { "NodesPerString", "20" }, { "DropPattern", dp } }));
        REQUIRE(c.ok);
        CHECK(c.s3 == c.s2);
    }
}
TEST_CASE("Roundtrip icicle valid drop patterns survive", "[roundtrip][models]") {
    for (const char* dp : { "1", "3,1,2", "5,4,3,2,1", "10" }) {
        INFO(dp);
        Cycle c = RunCycle(ModelXml("I", "Icicles", { { "NumStrings", "2" }, { "NodesPerString", "20" }, { "DropPattern", dp } }));
        REQUIRE(c.ok);
        pugi::xml_document d;
        REQUIRE(d.load_string(c.s1.c_str()));
        CHECK(std::string(d.first_child().attribute("DropPattern").value()) == dp);
    }
}
TEST_CASE("Roundtrip DMX presets keep every entry", "[roundtrip][dmx]") {
    Attrs at = { { "DmxChannelCount", "20" }, { "StringType", "Single Color White" } };
    for (int i = 0; i < 5; ++i) {
        at.emplace_back("DmxPresetChannel" + std::to_string(i), std::to_string(i + 3));
        at.emplace_back("DmxPresetValue" + std::to_string(i), std::to_string(10 * i + 7));
        at.emplace_back("DmxPresetDesc" + std::to_string(i), "preset & " + std::to_string(i));
    }
    for (const char* type : { "DmxMovingHeadAdv", "DmxMovingHead", "DmxGeneral" }) {
        INFO(type);
        Cycle c = RunCycle(ModelXml("D", type, at));
        REQUIRE(c.ok);
        CHECK(c.s2 == c.s1);
        pugi::xml_document d;
        REQUIRE(d.load_string(c.s1.c_str()));
        auto am = AttrMap(d.first_child());
        if (am.count("DmxPresetChannel0")) {
            for (int i = 0; i < 5; ++i) {
                CHECK(am["DmxPresetChannel" + std::to_string(i)] == std::to_string(i + 3));
                CHECK(am["DmxPresetValue" + std::to_string(i)] == std::to_string(10 * i + 7));
                CHECK(am["DmxPresetDesc" + std::to_string(i)] == "preset & " + std::to_string(i));
            }
        }
    }
}
TEST_CASE("Roundtrip DMX colour abilities", "[roundtrip][dmx]") {
    struct C { const char* type; int colorType; Attrs a; };
    std::vector<C> cs = {
        { "DmxGeneral", 0, { { "DmxRedChannel", "3" }, { "DmxGreenChannel", "2" }, { "DmxBlueChannel", "1" }, { "DmxWhiteChannel", "4" } } },
        { "DmxGeneral", 1, { { "DmxColorWheelChannel", "3" }, { "DmxDimmerChannel", "4" }, { "DmxColorWheelDelay", "250" } } },
        { "DmxGeneral", 2, { { "DmxCyanChannel", "3" }, { "DmxMagentaChannel", "4" }, { "DmxYellowChannel", "5" } } },
        { "DmxFloodlight", 0, { { "DmxRedChannel", "3" }, { "DmxGreenChannel", "2" }, { "DmxBlueChannel", "1" } } },
        { "DmxFloodArea", 0, { { "DmxRedChannel", "3" }, { "DmxGreenChannel", "2" }, { "DmxBlueChannel", "1" } } },
    };
    for (auto& c : cs) {
        INFO(c.type << " colorType " << c.colorType);
        Attrs at = { { "DmxChannelCount", "12" }, { "StringType", "Single Color White" }, { "DmxColorType", std::to_string(c.colorType) } };
        at.insert(at.end(), c.a.begin(), c.a.end());
        Cycle cy = RunCycle(ModelXml("D", c.type, at));
        REQUIRE(cy.ok);
        CHECK(cy.s2 == cy.s1);
        pugi::xml_document d;
        REQUIRE(d.load_string(cy.s1.c_str()));
        auto am = AttrMap(d.first_child());
        for (auto& e : c.a) {
            INFO(e.first);
            if (am.count(e.first)) CHECK(Same(am[e.first], e.second));
        }
    }
}
TEST_CASE("Roundtrip DMX skull with all parts disabled and enabled", "[roundtrip][dmx]") {
    for (const char* flag : { "0", "1" }) {
        INFO(flag);
        Attrs at = { { "DmxChannelCount", "26" }, { "StringType", "Single Color White" } };
        for (const char* f : { "HasJaw", "HasPan", "HasTilt", "HasNod", "HasEyeUD", "HasEyeLR", "HasColor", "Bits16" }) at.emplace_back(f, flag);
        Cycle c = RunCycle(ModelXml("S", "DmxSkull", at));
        REQUIRE(c.ok);
        CHECK(c.s2 == c.s1);
        pugi::xml_document d;
        REQUIRE(d.load_string(c.s1.c_str()));
        for (const char* f : { "HasJaw", "HasPan", "HasTilt", "HasNod", "HasEyeUD", "HasEyeLR", "HasColor", "Bits16" }) {
            INFO(f);
            CHECK(std::string(d.first_child().attribute(f).value()) == flag);
        }
    }
}
TEST_CASE("Roundtrip DMX moving head motors with extreme limits settle", "[roundtrip][dmx]") {
    for (const char* v : { "0", "-1000", "1000", "99999", "-0.5", "abc" }) {
        INFO(v);
        std::string kids = std::string("<PanMotor ChannelCoarse=\"") + v + "\" ChannelFine=\"" + v + "\" MinLimit=\"" + v + "\" MaxLimit=\"" + v + "\" RangeOfMotion=\"" + v + "\" OrientZero=\"" + v + "\" OrientHome=\"" + v + "\" SlewLimit=\"" + v + "\"/>";
        Cycle c = RunCycle(ModelXml("MH", "DmxMovingHead", { { "DmxChannelCount", "16" }, { "StringType", "Single Color White" } }, kids));
        REQUIRE(c.ok);
        CHECK(c.s3 == c.s2);
    }
}
TEST_CASE("Roundtrip DMX moving head motor values survive", "[roundtrip][dmx]") {
    std::string kids = "<PanMotor ChannelCoarse=\"3\" ChannelFine=\"4\" MinLimit=\"-120\" MaxLimit=\"150\" RangeOfMotion=\"300\" OrientZero=\"15\" OrientHome=\"25\" SlewLimit=\"6.5\" SpeedChannel=\"9\" Reverse=\"1\" UpsideDown=\"1\"/>"
                       "<TiltMotor ChannelCoarse=\"5\" ChannelFine=\"6\" MinLimit=\"-60\" MaxLimit=\"70\" RangeOfMotion=\"130\" OrientZero=\"5\" OrientHome=\"35\" SlewLimit=\"7.5\" SpeedChannel=\"10\" Reverse=\"1\" UpsideDown=\"0\"/>";
    for (const char* type : { "DmxMovingHead", "DmxMovingHeadAdv" }) {
        INFO(type);
        Cycle c = RunCycle(ModelXml("MH", type, { { "DmxChannelCount", "20" }, { "StringType", "Single Color White" } }, kids));
        REQUIRE(c.ok);
        CHECK(c.s2 == c.s1);
        pugi::xml_document d;
        REQUIRE(d.load_string(c.s1.c_str()));
        auto pan = d.first_child().child("PanMotor");
        REQUIRE(pan);
        CHECK(pan.attribute("ChannelCoarse").as_int() == 3);
        CHECK(pan.attribute("MinLimit").as_int() == -120);
        CHECK(pan.attribute("MaxLimit").as_int() == 150);
        CHECK(pan.attribute("RangeOfMotion").as_float() == Catch::Approx(300.0f));
        CHECK(pan.attribute("OrientZero").as_int() == 15);
        CHECK(pan.attribute("SlewLimit").as_float() == Catch::Approx(6.5f));
        CHECK(pan.attribute("Reverse").as_int() == 1);
        CHECK(pan.attribute("UpsideDown").as_int() == 1);
        auto tilt = d.first_child().child("TiltMotor");
        REQUIRE(tilt);
        CHECK(tilt.attribute("ChannelCoarse").as_int() == 5);
        CHECK(tilt.attribute("MaxLimit").as_int() == 70);
        CHECK(tilt.attribute("SlewLimit").as_float() == Catch::Approx(7.5f));
    }
}
TEST_CASE("Roundtrip DMX servo values survive", "[roundtrip][dmx]") {
    std::string kids = "<Servo1 Channel=\"2\" MinLimit=\"100\" MaxLimit=\"900\" RangeOfMotion=\"120\" ServoStyle=\"Rotate X\" ControllerMin=\"1200\" ControllerMax=\"1800\" ControllerReverse=\"1\" ControllerZeroBehavior=\"Hold\" ControllerDataType=\"Scaled\"/>";
    Cycle c = RunCycle(ModelXml("SV", "DmxServo", { { "NumServos", "1" }, { "DmxChannelCount", "2" }, { "StringType", "Single Color White" }, { "Bits16", "1" } }, kids));
    REQUIRE(c.ok);
    CHECK(c.s2 == c.s1);
    pugi::xml_document d;
    REQUIRE(d.load_string(c.s1.c_str()));
    auto sv = d.first_child().child("Servo1");
    REQUIRE(sv);
    CHECK(sv.attribute("Channel").as_int() == 2);
    CHECK(sv.attribute("MinLimit").as_int() == 100);
    CHECK(sv.attribute("MaxLimit").as_int() == 900);
    CHECK(sv.attribute("RangeOfMotion").as_int() == 120);
    CHECK(std::string(sv.attribute("ServoStyle").value()) == "Rotate X");
    CHECK(sv.attribute("ControllerMin").as_int() == 1200);
    CHECK(sv.attribute("ControllerMax").as_int() == 1800);
    CHECK(sv.attribute("ControllerReverse").as_int() == 1);
}
TEST_CASE("Roundtrip DMX servo image offsets and scales", "[roundtrip][dmx]") {
    std::string kids = "<StaticImage1 Image=\"\" ScaleX=\"1.5\" ScaleY=\"2.5\" ScaleZ=\"3.5\" RotateX=\"10\" RotateY=\"20\" RotateZ=\"30\" OffsetX=\"4.5\" OffsetY=\"5.5\" OffsetZ=\"6.5\"/>"
                       "<MotionImage1 Image=\"\" ScaleX=\"1.25\" ScaleY=\"2.25\" ScaleZ=\"3.25\" RotateX=\"11\" RotateY=\"21\" RotateZ=\"31\" OffsetX=\"7.5\" OffsetY=\"8.5\" OffsetZ=\"9.5\"/>";
    Cycle c = RunCycle(ModelXml("SV", "DmxServo", { { "NumServos", "1" }, { "DmxChannelCount", "2" }, { "StringType", "Single Color White" } }, kids));
    REQUIRE(c.ok);
    CHECK(c.s2 == c.s1);
    CHECK(c.s3 == c.s2);
    pugi::xml_document d;
    REQUIRE(d.load_string(c.s1.c_str()));
    for (const char* el : { "StaticImage1", "MotionImage1" }) {
        auto n = d.first_child().child(el);
        REQUIRE(n);
        INFO(el);
    }
    auto si = d.first_child().child("StaticImage1");
    CHECK(si.attribute("ScaleX").as_float() == Catch::Approx(1.5f));
    CHECK(si.attribute("ScaleZ").as_float() == Catch::Approx(3.5f));
    CHECK(si.attribute("RotateY").as_float() == Catch::Approx(20.0f));
    CHECK(si.attribute("OffsetX").as_float() == Catch::Approx(4.5f));
    CHECK(si.attribute("OffsetZ").as_float() == Catch::Approx(6.5f));
    auto mi = d.first_child().child("MotionImage1");
    CHECK(mi.attribute("OffsetY").as_float() == Catch::Approx(8.5f));
}
TEST_CASE("Roundtrip DMX servo3d mesh values survive", "[roundtrip][dmx]") {
    std::string kids = "<StaticMesh1 ObjFile=\"\" MeshOnly=\"1\" Brightness=\"60\" ScaleX=\"1.5\" ScaleY=\"2.5\" ScaleZ=\"3.5\" RotateX=\"10\" RotateY=\"20\" RotateZ=\"30\" OffsetX=\"4.5\" OffsetY=\"5.5\" OffsetZ=\"6.5\"/>";
    Cycle c = RunCycle(ModelXml("SV", "DmxServo3d", { { "NumServos", "1" }, { "NumStatic", "1" }, { "NumMotion", "1" }, { "DmxChannelCount", "2" }, { "StringType", "Single Color White" } }, kids));
    REQUIRE(c.ok);
    CHECK(c.s2 == c.s1);
    pugi::xml_document d;
    REQUIRE(d.load_string(c.s1.c_str()));
    auto sm = d.first_child().child("StaticMesh1");
    REQUIRE(sm);
    CHECK(sm.attribute("MeshOnly").as_int() == 1);
    CHECK(sm.attribute("Brightness").as_float() == Catch::Approx(60.0f));
    CHECK(sm.attribute("ScaleY").as_float() == Catch::Approx(2.5f));
    CHECK(sm.attribute("RotateZ").as_float() == Catch::Approx(30.0f));
    CHECK(sm.attribute("OffsetX").as_float() == Catch::Approx(4.5f));
}
TEST_CASE("Roundtrip model unknown extra attributes and children settle", "[roundtrip][models]") {
    Cycle c = RunCycle(Line({ { "BogusAttr", "x" } }, "<bogusChild a=\"1\"/>"));
    REQUIRE(c.ok);
    CHECK(c.s3 == c.s2);
}

// KNOWN BUG: a huge LightsPerNode makes loading a Single Line model segfault (no clamp on the per-node channel multiplier).
TEST_CASE("Roundtrip huge LightsPerNode loads safely", "[.][known-crash][roundtrip_crash]") {
    ShowFixture a;
    Model* m = a.Add(ModelXml("E", "Single Line", { { "NumStrings", "2" }, { "NodesPerString", "5" }, { "LightsPerNode", "2147483647" } }));
    if (m != nullptr) {
        ShowFixture b;
        CHECK(b.Add(Ser(m)) != nullptr);
    }
}
// KNOWN BUG: a huge Cube Strings value hangs the loader (no upper clamp on the string count).
TEST_CASE("Roundtrip huge cube string count loads safely", "[.][known-crash][roundtrip_crash]") {
    ShowFixture a;
    Model* m = a.Add(ModelXml("E", "Cube", { { "CubeWidth", "2" }, { "CubeHeight", "2" }, { "CubeDepth", "2" }, { "Strings", "2147483647" } }));
    if (m != nullptr) {
        ShowFixture b;
        CHECK(b.Add(Ser(m)) != nullptr);
    }
}
// KNOWN BUG: a negative DMX colour channel is stored unsigned (4294967295), written as such, and then clamped to INT_MAX on the next load.
TEST_CASE("Roundtrip negative DMX colour channel settles", "[roundtrip][dmx][!mayfail]") {
    for (const char* attr : { "DmxRedChannel", "DmxGreenChannel", "DmxBlueChannel" }) {
        INFO(attr);
        Cycle c = RunCycle(ModelXml("E", "DmxGeneral", { { "DmxChannelCount", "6" }, { "StringType", "Single Color White" }, { attr, "-1" } }));
        REQUIRE(c.ok);
        CHECK(c.s2 == c.s1);
    }
}
