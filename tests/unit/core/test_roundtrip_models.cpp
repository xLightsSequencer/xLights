#include <set>

#include <catch2/catch_approx.hpp>

#include "test_roundtrip_common.h"

using namespace rt;

namespace {

const Attrs& Common() {
    static const Attrs c = {
        { "WorldPosX", "123.5" }, { "WorldPosY", "-45.25" }, { "WorldPosZ", "7.75" },
        { "ScaleX", "2.5" }, { "ScaleY", "1.75" }, { "ScaleZ", "3.25" },
        { "RotateX", "12" }, { "RotateY", "34" }, { "RotateZ", "56" },
        { "Description", "desc & <b> \"q\" 'a'" }, { "LayoutGroup", "Back Yard" },
        { "PixelSize", "7" }, { "Transparency", "15" }, { "BlackTransparency", "25" },
    };
    return c;
}

struct Spec {
    const char* type;
    Attrs attrs;
    std::string children;
    bool common = true;
};

const std::string kExtras =
    "<subModel name=\"top\" layout=\"horizontal\" type=\"ranges\" line0=\"1-3\"/>"
    "<faceInfo Name=\"F\" Type=\"NodeRange\" CustomColors=\"0\" Eyes-Open=\"1-2\"/>"
    "<stateInfo Name=\"S\" Type=\"NodeRange\" CustomColors=\"0\" s1=\"1-4\" s1-Name=\"Half\" s1-Color=\"#FF0000\"/>"
    "<Aliases><alias name=\"Old\"/></Aliases>";

const std::vector<Spec>& Specs() {
    static const std::vector<Spec> s = {
        /*0*/ { "Tree 360", { { "NumStrings", "3" }, { "NodesPerString", "7" }, { "StrandsPerString", "1" }, { "TreeSpiralRotations", "2.5" }, { "TreeBottomTopRatio", "4.5" }, { "TreePerspective", "0.3" }, { "TreeRotation", "17" }, { "StartSide", "B" }, { "Dir", "R" }, { "StringType", "RGB Nodes" } }, kExtras },
        /*1*/ { "Tree Flat", { { "NumStrings", "2" }, { "NodesPerString", "5" }, { "StrandsPerString", "1" }, { "TreeSpiralRotations", "-1.5" }, { "TreeBottomTopRatio", "0.5" } }, "" },
        /*2*/ { "Tree Ribbon", { { "NumStrings", "2" }, { "NodesPerString", "6" }, { "StrandsPerString", "2" }, { "TreeSpiralRotations", "1.5" } }, "" },
        /*3*/ { "Matrix", { { "NumStrings", "2" }, { "NodesPerString", "12" }, { "StrandsPerString", "3" }, { "StartSide", "T" }, { "Dir", "R" }, { "Vertical", "true" }, { "NoZig", "true" }, { "StringType", "RGBW Nodes" } }, kExtras },
        /*4*/ { "Matrix", { { "NumStrings", "4" }, { "NodesPerString", "8" }, { "StrandsPerString", "2" }, { "StartSide", "B" }, { "Dir", "L" }, { "Vertical", "false" }, { "NoZig", "false" } }, "" },
        /*5*/ { "Arches", { { "NumArches", "3" }, { "NodesPerArch", "9" }, { "ArchesSkew", "20" }, { "ZigZag", "true" }, { "Hollow", "35" }, { "Arc", "270" }, { "Gap", "4" }, { "LightsPerNode", "2" }, { "Dir", "R" } }, kExtras },
        /*6*/ { "Candy Canes", { { "NumCanes", "3" }, { "NodesPerCane", "6" }, { "CandyCaneHeight", "2.5" }, { "CandyCaneReverse", "true" }, { "AlternateNodes", "true" }, { "CandyCaneSticks", "true" }, { "CandyCaneSkew", "15" }, { "LightsPerNode", "2" } }, "" },
        /*7*/ { "Icicles", { { "NumStrings", "2" }, { "NodesPerString", "10" }, { "DropPattern", "3,1,2" }, { "AlternateNodes", "true" }, { "LightsPerNode", "1" } }, "" },
        /*8*/ { "Star", { { "NumStrings", "2" }, { "NodesPerString", "10" }, { "StarPoints", "6" }, { "StarStartLocation", "Bottom Left" }, { "starRatio", "2.5" }, { "starCenterPercent", "40" }, { "LayerSizes", "5,5" } }, "" },
        /*9*/ { "Wreath", { { "NumStrings", "2" }, { "NodesPerString", "12" }, { "Dir", "R" } }, "" },
        /*10*/ { "Circle", { { "NumStrings", "2" }, { "NodesPerString", "12" }, { "LayerSizes", "8,4" }, { "InsideOut", "1" }, { "centerPercent", "30" } }, "" },
        /*11*/ { "Spinner", { { "NumStrings", "2" }, { "NodesPerArm", "9" }, { "ArmsPerString", "3" }, { "Hollow", "20" }, { "StartAngle", "15" }, { "Arc", "270" }, { "ZigZag", "false" }, { "Alternate", "true" } }, "" },
        /*12*/ { "Window Frame", { { "TopNodes", "5" }, { "SideNodes", "4" }, { "BottomNodes", "6" }, { "Rotation", "Counter Clockwise" } }, "" },
        /*13*/ { "Cube", { { "CubeWidth", "3" }, { "CubeHeight", "2" }, { "CubeDepth", "4" }, { "Style", "Horizontal Left/Right" }, { "Strings", "2" }, { "Start", "Back Top Right" }, { "StrandPerLine", "No Zig Zag" }, { "StrandPerLayer", "TRUE" }, { "CubeHollow", "40" }, { "CubeShape", "1" }, { "CubeRowOffset", "2" } }, "" },
        /*14*/ { "Sphere", { { "NumStrings", "2" }, { "NodesPerString", "10" }, { "StrandsPerString", "2" }, { "StartLatitude", "-70" }, { "EndLatitude", "70" }, { "Degrees", "300" } }, "" },
        /*15*/ { "Single Line", { { "NumStrings", "3" }, { "NodesPerString", "11" }, { "LightsPerNode", "2" }, { "StringType", "RGBW Nodes" }, { "X2", "10.5" }, { "Y2", "-3.5" }, { "Z2", "2.25" } }, "" },
        /*16*/ { "Poly Line", { { "NumStrings", "1" }, { "NodesPerString", "20" }, { "NumPoints", "3" }, { "PointData", "0.0,0.0,0.0,5.0,5.0,0.0,10.0,0.0,0.0" }, { "cPointData", "" }, { "DropPattern", "1" }, { "AlternateNodes", "true" } }, "" },
        /*17*/ { "Channel Block", { { "NumChannels", "4" }, { "ChannelProperties.ChannelColor1", "#FF0000" }, { "ChannelProperties.ChannelColor2", "#00FF00" }, { "ChannelProperties.ChannelColor3", "#0000FF" }, { "ChannelProperties.ChannelColor4", "#FFFFFF" } }, "" },
        /*18*/ { "Custom", { { "CustomWidth", "3" }, { "CustomHeight", "2" }, { "CustomModel", "1,2,3;4,5,6" }, { "StringType", "RGB Nodes" }, { "Depth", "1" } }, kExtras },
        /*19*/ { "Label", { { "LabelText", "Hello & goodbye <1>" }, { "LabelFontSize", "22" }, { "LabelTextColor", "#00FFAA" }, { "StringType", "Single Color White" } }, "" },
        /*20*/ { "Image", { { "Image", "" }, { "OffBrightness", "35" }, { "WhiteAsAlpha", "True" }, { "StringType", "Single Color White" } }, "" },
        /*21*/ { "DmxMovingHead", { { "DmxChannelCount", "16" }, { "StringType", "Single Color White" }, { "DmxFixture", "MH2" } }, "<PanMotor ChannelCoarse=\"3\" ChannelFine=\"4\" RangeOfMotion=\"360\" MinLimit=\"-90\" MaxLimit=\"180\" OrientZero=\"10\" OrientHome=\"20\" SlewLimit=\"4.5\" UpsideDown=\"1\"/><TiltMotor ChannelCoarse=\"5\" ChannelFine=\"6\"/>" },
        /*22*/ { "DmxMovingHeadAdv", { { "DmxChannelCount", "20" }, { "StringType", "Single Color White" }, { "DmxColorType", "2" }, { "DmxBeamYOffset", "6.5" }, { "DmxBeamLength", "9.5" }, { "DmxBeamWidth", "3.5" } }, "" },
        /*23*/ { "DmxFloodlight", { { "DmxChannelCount", "4" }, { "StringType", "Single Color White" }, { "DmxRedChannel", "4" }, { "DmxGreenChannel", "3" }, { "DmxBlueChannel", "2" }, { "DmxWhiteChannel", "1" }, { "DmxBeamLength", "9.5" } }, "" },
        /*24*/ { "DmxFloodArea", { { "DmxChannelCount", "3" }, { "StringType", "Single Color White" } }, "" },
        /*25*/ { "DmxSkull", { { "DmxChannelCount", "26" }, { "StringType", "Single Color White" } }, "" },
        /*26*/ { "DmxServo", { { "NumServos", "2" }, { "DmxChannelCount", "4" }, { "StringType", "Single Color White" } }, "" },
        /*27*/ { "DmxServo3d", { { "NumServos", "2" }, { "NumStatic", "2" }, { "NumMotion", "2" }, { "DmxChannelCount", "4" }, { "StringType", "Single Color White" } }, "" },
        /*28*/ { "DmxGeneral", { { "DmxChannelCount", "6" }, { "StringType", "Single Color White" } }, "" },
        /*29*/ { "MultiPoint", { { "NumPoints", "3" }, { "PointData", "0.0,0.0,0.0,5.0,5.0,0.0,10.0,0.0,0.0" }, { "MultiStrings", "2" } }, "" },
    };
    return s;
}

// Common attributes a model's screen location type does not store; they are legitimately not written.
const std::set<std::string>& NotApplicable(size_t i) {
    static const std::set<std::string> boxed;
    static const std::set<std::string> twoPoint = { "ScaleX", "ScaleY", "ScaleZ", "RotateY", "RotateZ" };
    static const std::set<std::string> line = { "ScaleX", "ScaleY", "ScaleZ", "RotateX", "RotateY", "RotateZ" };
    static const std::set<std::string> poly = { "RotateX", "RotateY", "RotateZ", "NumStrings" };
    static const std::set<std::string> multi = { "RotateX", "RotateY", "RotateZ", "ScaleX", "ScaleY", "ScaleZ" };
    static const std::set<std::string> sphere = { "ScaleX", "ScaleZ" };
    static const std::set<std::string> arches = { "ScaleX", "ScaleY", "ScaleZ", "RotateY", "RotateZ", "ArchesSkew" };
    static const std::set<std::string> canes = { "ScaleX", "ScaleY", "ScaleZ", "RotateY", "RotateZ", "CandyCaneSkew" };
    static const std::set<std::string> icicles = { "ScaleX", "ScaleY", "ScaleZ", "RotateY", "RotateZ", "LightsPerNode" };
    static const std::set<std::string> chan = { "ScaleX", "ScaleY", "ScaleZ", "RotateX", "RotateY", "RotateZ", "StringType" };
    switch (i) {
    case 5: return arches;
    case 6: return canes;
    case 7: return icicles;
    case 15: return line;
    case 16: return poly;
    case 29: return multi;
    case 14: return sphere;
    case 17: return chan;
    default: return boxed;
    }
}

std::string Name(size_t i) { return "Model " + std::to_string(i); }

Model* Load(ShowFixture& f, size_t i) {
    const Spec& s = Specs()[i];
    Attrs a = s.attrs;
    if (s.common) {
        a.insert(a.end(), Common().begin(), Common().end());
    }
    return f.Add(ModelXml(Name(i), s.type, a, s.children));
}

Model* Reload(ShowFixture& f, const std::string& ser) {
    return f.Add(ser);
}

void CheckStable(size_t i) {
    ShowFixture a;
    Model* m = Load(a, i);
    REQUIRE(m != nullptr);
    std::string s1 = Ser(m);
    ShowFixture b;
    Model* m2 = Reload(b, s1);
    REQUIRE(m2 != nullptr);
    std::string s2 = Ser(m2);
    CHECK(s2 == s1);
    ShowFixture c;
    Model* m3 = Reload(c, s2);
    REQUIRE(m3 != nullptr);
    CHECK(Ser(m3) == s2);
    CHECK(m2->GetNodeCount() == m->GetNodeCount());
    CHECK(m2->GetChanCount() == m->GetChanCount());
    CHECK(m3->GetNodeCount() == m->GetNodeCount());
}

void CheckAttrs(size_t i) {
    const Spec& s = Specs()[i];
    Attrs expect = s.attrs;
    if (s.common) {
        expect.insert(expect.end(), Common().begin(), Common().end());
    }
    ShowFixture a;
    Model* m = Load(a, i);
    REQUIRE(m != nullptr);
    pugi::xml_document d1;
    REQUIRE(d1.load_string(Ser(m).c_str()));
    ShowFixture b;
    Model* m2 = Reload(b, Ser(m));
    REQUIRE(m2 != nullptr);
    pugi::xml_document d2;
    REQUIRE(d2.load_string(Ser(m2).c_str()));
    auto am1 = AttrMap(d1.first_child());
    auto am2 = AttrMap(d2.first_child());
    for (auto& e : expect) {
        INFO(e.first << " = " << e.second);
        if (NotApplicable(i).count(e.first)) continue;
        auto it = am1.find(e.first);
        if (it == am1.end()) {
            // dropped on first save/load
            CHECK(it != am1.end());
            continue;
        }
        CHECK(Same(it->second, e.second));
        CHECK(am2[e.first] == it->second);
    }
    CHECK(am1 == am2);
}

std::string NoEmptyTag(std::string s) {
    const std::string t = " TagColour=\"\"";
    for (auto p = s.find(t); p != std::string::npos; p = s.find(t)) s.erase(p, t.size());
    return s;
}

void CheckDefault(const char* type) {
    ShowFixture a;
    Model* m = a.mm.CreateDefaultModel(type, "1");
    REQUIRE(m != nullptr);
    std::string s1 = NoEmptyTag(Ser(m));
    ShowFixture b;
    Model* m2 = Reload(b, s1);
    REQUIRE(m2 != nullptr);
    std::string s2 = NoEmptyTag(Ser(m2));
    INFO("save1 vs save2:" << DiffAttrs(s1, s2));
    CHECK(s2 == s1);
    ShowFixture c;
    Model* m3 = Reload(c, s2);
    REQUIRE(m3 != nullptr);
    INFO("save2 vs save3:" << DiffAttrs(s2, NoEmptyTag(Ser(m3))));
    CHECK(NoEmptyTag(Ser(m3)) == s2);
    CHECK(m2->GetNodeCount() == m->GetNodeCount());
    CHECK(m2->GetChanCount() == m->GetChanCount());
    delete m;
}

void CheckExport(size_t i) {
    ShowFixture a;
    Model* m = Load(a, i);
    REQUIRE(m != nullptr);
    std::string e1 = Ser(m, true);
    ShowFixture b;
    Model* m2 = Reload(b, e1);
    REQUIRE(m2 != nullptr);
    CHECK(Ser(m2, true) == e1);
}

void CheckChildren(size_t i) {
    ShowFixture a;
    Model* m = Load(a, i);
    REQUIRE(m != nullptr);
    pugi::xml_document d;
    REQUIRE(d.load_string(Ser(m).c_str()));
    std::vector<std::string> kids;
    for (auto c : d.first_child().children()) {
        kids.push_back(c.name());
    }
    ShowFixture b;
    Model* m2 = Reload(b, Ser(m));
    pugi::xml_document d2;
    REQUIRE(d2.load_string(Ser(m2).c_str()));
    std::vector<std::string> kids2;
    for (auto c : d2.first_child().children()) {
        kids2.push_back(c.name());
    }
    CHECK(kids == kids2);
    // each input child element must survive
    pugi::xml_document in;
    REQUIRE(in.load_string(("<r>" + Specs()[i].children + "</r>").c_str()));
    for (auto c : in.first_child().children()) {
        INFO(c.name());
        CHECK(std::find(kids.begin(), kids.end(), std::string(c.name())) != kids.end());
    }
}

} // namespace

#define MODEL_CASES(IDX, LABEL)                                                                  \
    TEST_CASE("Roundtrip model is stable: " LABEL, "[roundtrip][models]") { CheckStable(IDX); }  \
    TEST_CASE("Roundtrip model keeps attributes: " LABEL, "[roundtrip][models]") { CheckAttrs(IDX); } \
    TEST_CASE("Roundtrip model export mode is stable: " LABEL, "[roundtrip][models]") { CheckExport(IDX); } \
    TEST_CASE("Roundtrip model keeps child elements: " LABEL, "[roundtrip][models]") { CheckChildren(IDX); }

MODEL_CASES(0, "Tree 360")
MODEL_CASES(1, "Tree Flat")
MODEL_CASES(2, "Tree Ribbon")
MODEL_CASES(3, "Matrix vertical")
MODEL_CASES(4, "Matrix horizontal")
MODEL_CASES(5, "Arches")
MODEL_CASES(6, "Candy Canes")
MODEL_CASES(7, "Icicles")
MODEL_CASES(8, "Star")
MODEL_CASES(9, "Wreath")
MODEL_CASES(10, "Circle")
MODEL_CASES(11, "Spinner")
MODEL_CASES(12, "Window Frame")
MODEL_CASES(13, "Cube")
MODEL_CASES(14, "Sphere")
MODEL_CASES(15, "Single Line")
MODEL_CASES(16, "Poly Line")
MODEL_CASES(17, "Channel Block")
MODEL_CASES(18, "Custom")
MODEL_CASES(19, "Label")
MODEL_CASES(20, "Image")
MODEL_CASES(21, "DmxMovingHead")
MODEL_CASES(22, "DmxMovingHeadAdv")
MODEL_CASES(23, "DmxFloodlight")
MODEL_CASES(24, "DmxFloodArea")
MODEL_CASES(25, "DmxSkull")
MODEL_CASES(26, "DmxServo")
MODEL_CASES(27, "DmxServo3d")
MODEL_CASES(28, "DmxGeneral")
MODEL_CASES(29, "MultiPoint")

#define DEFAULT_CASE(TYPE) \
    TEST_CASE("Roundtrip default model is stable: " TYPE, "[roundtrip][models]") { CheckDefault(TYPE); }

DEFAULT_CASE("Star")
DEFAULT_CASE("Arches")
DEFAULT_CASE("Candy Canes")
DEFAULT_CASE("Channel Block")
DEFAULT_CASE("Circle")
DEFAULT_CASE("DmxMovingHead")
DEFAULT_CASE("DmxGeneral")
DEFAULT_CASE("DmxMovingHeadAdv")
DEFAULT_CASE("DmxFloodlight")
DEFAULT_CASE("DmxFloodArea")
DEFAULT_CASE("DmxSkull")
DEFAULT_CASE("DmxServo")
DEFAULT_CASE("DmxServo3d")
DEFAULT_CASE("DmxServo3Axis")
DEFAULT_CASE("Image")
DEFAULT_CASE("Label")
DEFAULT_CASE("Window Frame")
DEFAULT_CASE("Wreath")
DEFAULT_CASE("Sphere")
DEFAULT_CASE("Single Line")
DEFAULT_CASE("Poly Line")
DEFAULT_CASE("MultiPoint")
DEFAULT_CASE("Cube")
DEFAULT_CASE("Custom")
DEFAULT_CASE("Tree 360")
DEFAULT_CASE("Tree Flat")
DEFAULT_CASE("Matrix")
DEFAULT_CASE("Spinner")
DEFAULT_CASE("Icicles")

TEST_CASE("Roundtrip default model TagColour is written consistently", "[roundtrip][models]") {
    ShowFixture a;
    Model* m = a.mm.CreateDefaultModel("Star", "1");
    REQUIRE(m != nullptr);
    std::string s1 = Ser(m);
    ShowFixture b;
    CHECK(Ser(Reload(b, s1)) == s1);
    delete m;
}

TEST_CASE("Roundtrip sphere legacy scale migration happens only once", "[roundtrip][models]") {
    ShowFixture a;
    Model* m = a.Add(ModelXml("Sp", "Sphere", { { "NumStrings", "2" }, { "NodesPerString", "10" }, { "StrandsPerString", "2" }, { "ScaleX", "2.5" }, { "ScaleZ", "3.25" } }));
    REQUIRE(m != nullptr);
    std::string s1 = Ser(m);
    pugi::xml_document d1;
    REQUIRE(d1.load_string(s1.c_str()));
    float sx1 = d1.first_child().attribute("ScaleX").as_float();
    ShowFixture b;
    std::string s2 = Ser(b.Add(s1));
    ShowFixture c;
    std::string s3 = Ser(c.Add(s2));
    pugi::xml_document d3;
    REQUIRE(d3.load_string(s3.c_str()));
    CHECK(d3.first_child().attribute("ScaleX").as_float() == Catch::Approx(sx1));
    CHECK(s1 == s2);
    CHECK(s2 == s3);
}
