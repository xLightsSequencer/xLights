#include <exception>
#include <sstream>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <pugixml.hpp>

#include "XmlSerializer/GdtfParser.h"
#include "XmlSerializer/XmlSerializer.h"
#include "models/Model.h"
#include "models/ModelManager.h"
#include "models/SubModel.h"
#include "outputs/OutputManager.h"

namespace {

struct ShowFixture {
    OutputManager om;
    ModelManager mm{ &om };

    Model* TryAdd(const std::string& xml) {
        pugi::xml_document doc;
        if (!doc.load_string(xml.c_str())) {
            return nullptr;
        }
        try {
            Model* m = mm.createAndAddModel(doc.first_child(), 100, 100);
            mm.RecalcStartChannels();
            return m;
        } catch (const std::exception&) {
            return nullptr;
        }
    }
};

void Exercise(Model* m) {
    if (m == nullptr) {
        return;
    }
    try {
        m->GetModelScreenLocation().PrepareToDraw(true, true);
        pugi::xml_document out;
        XmlSerializingVisitor v(&out);
        m->Accept(v);
        int w = 0, h = 0;
        m->GetBufferSize("Default", "2D", "None", w, h, 0);
        std::vector<NodeBaseClassPtr> nodes;
        m->InitRenderBufferNodes("Default", "2D", "None", nodes, w, h, 0);
        (void)m->GetNodeCount();
        (void)m->GetChanCount();
        (void)m->GetLastChannel();
    } catch (const std::exception&) {
    }
}

struct TypeSpec {
    const char* type;
    const char* base;
    std::vector<const char*> attrs;
};

} // namespace

TEST_CASE("Hostile single attribute values never crash model loading", "[models2][robustness]") {
    const std::vector<TypeSpec> specs = {
        { "Tree 360", "NumStrings=\"2\" NodesPerString=\"6\" StrandsPerString=\"2\"", { "LightsPerNode", "StartSide", "Dir", "StringType", "ScaleX", "RotateX", "WorldPosX", "TreeSpiralRotations", "TreeBottomTopRatio", "TreePerspective", "TreeRotation", "NoZig", "StrandDir", "parm3" } },
        { "Matrix", "NumStrings=\"2\" NodesPerString=\"6\" StrandsPerString=\"2\"", { "Vertical", "NoZig", "StartSide", "Dir", "StringType", "ScaleX", "ScaleY", "RotateZ", "PixelCount", "parm1", "parm2", "parm3" } },
        { "Arches", "NumArches=\"2\" NodesPerArch=\"5\"", { "LightsPerNode", "ArchesSkew", "ZigZag", "Hollow", "Arc", "Gap", "StringType", "Dir", "X2", "Y2" } },
        { "Candy Canes", "NumCanes=\"2\" NodesPerCane=\"4\"", { "CandyCaneHeight", "CandyCaneReverse", "CandyCaneSticks", "CandyCaneSkew", "AlternateNodes", "LightsPerNode", "StringType", "Dir" } },
        { "Icicles", "NumStrings=\"2\" NodesPerString=\"5\"", { "AlternateNodes", "LightsPerNode", "StringType", "OffBrightness" } },
        { "Single Line", "NumStrings=\"2\" NodesPerString=\"5\"", { "LightsPerNode", "DropPattern", "AlternateNodes", "StringType", "X2", "Y2", "Z2", "ModelHeight" } },
        { "Star", "NumStrings=\"1\" NodesPerString=\"10\"", { "StarPoints", "LayerSizes", "starSizes", "starRatio", "starCenterPercent", "StarStartLocation", "StringType" } },
        { "Wreath", "NumStrings=\"1\" NodesPerString=\"8\"", { "StringType", "Dir", "StartSide", "ScaleX" } },
        { "Circle", "NumStrings=\"1\" NodesPerString=\"8\"", { "LayerSizes", "circleSizes", "centerPercent", "InsideOut", "StringType", "Dir" } },
        { "Spinner", "NumStrings=\"1\" NodesPerArm=\"3\"", { "Hollow", "Arc", "StartAngle", "ZigZag", "Alternate", "StringType", "LayerSizes" } },
        { "Window Frame", "TopNodes=\"3\" SideNodes=\"2\"", { "Rotation", "StringType", "Dir", "StartSide" } },
        { "Cube", "CubeWidth=\"2\" CubeHeight=\"2\"", { "CubeDepth", "Style", "Strings", "Start", "StrandPerLine", "StrandPerLayer", "CubeHollow", "StringType" } },
        { "Sphere", "NumStrings=\"1\" NodesPerString=\"6\"", { "StrandsPerString", "Degrees", "StartLatitude", "EndLatitude", "NoZig", "AlternateNodes", "StringType" } },
        { "Poly Line", "NumStrings=\"1\" NodesPerString=\"6\" NumPoints=\"3\" PointData=\"0.0,0.0,0.0,5.0,5.0,0.0,10.0,0.0,0.0\"", { "DropPattern", "AlternateNodes", "IndivSegs", "LightsPerNode", "cPointData", "StringType" } },
        { "MultiPoint", "NumPoints=\"3\" PointData=\"0.0,0.0,0.0,5.0,5.0,0.0,10.0,0.0,0.0\"", { "NodesPerString", "NumStrings", "StringType" } },
        { "Channel Block", "NumChannels=\"4\"", { "ChannelProperties.ChannelColor1", "StringType", "NumChannels" } },
        { "Custom", "CustomWidth=\"3\" CustomHeight=\"2\" CustomModel=\"1,2,3;4,5,6\"", { "Depth", "StringType", "CustomModelCompressed", "CustomStartNodes", "Advanced" } },
        { "Label", "LabelText=\"x\"", { "LabelFontSize", "LabelTextColor", "StringType" } },
        { "Image", "Image=\"\"", { "Transparency", "OffBrightness", "WhiteAsAlpha", "ScaleX" } },
    };
    const std::vector<std::string> values = { "", "abc", "-1", "0", "nan" };
    int loads = 0;
    for (const auto& spec : specs) {
        for (const char* attr : spec.attrs) {
            if (std::string(spec.base).find(std::string(attr) + "=") != std::string::npos) {
                continue;
            }
            for (const auto& v : values) {
                std::string xml = std::string("<model name=\"M\" DisplayAs=\"") + spec.type + "\" StartChannel=\"1\" " + spec.base + " " + attr + "=\"" + v + "\"/>";
                INFO(xml);
                ShowFixture f;
                Exercise(f.TryAdd(xml));
                ++loads;
            }
        }
    }
    CHECK(loads > 500);
}

TEST_CASE("Hostile sizes for every type either load or throw", "[models2][robustness]") {
    const char* types[] = { "Tree 360", "Matrix", "Arches", "Candy Canes", "Icicles", "Single Line", "Star", "Wreath", "Circle", "Spinner", "Window Frame", "Cube", "Sphere", "Poly Line", "MultiPoint", "Channel Block", "Custom", "Label", "Image" };
    for (const char* t : types) {
        for (const char* v : { "0", "1", "abc", "" }) {
            INFO(t << " " << v);
            std::string xml = std::string("<model name=\"M\" DisplayAs=\"") + t + "\" StartChannel=\"1\" NumStrings=\"" + v + "\" NodesPerString=\"" + v + "\" NumArches=\"" + v + "\" NodesPerArch=\"" + v +
                              "\" NumCanes=\"" + v + "\" NodesPerCane=\"" + v + "\" StrandsPerString=\"" + v + "\" TopNodes=\"" + v + "\" SideNodes=\"" + v + "\" BottomNodes=\"" + v + "\" CubeWidth=\"" + v + "\" CubeHeight=\"" + v + "\" CubeDepth=\"" + v +
                              "\" NodesPerArm=\"" + v + "\" ArmsPerString=\"" + v + "\" StarPoints=\"" + v + "\" NumChannels=\"" + v + "\"/>";
            ShowFixture f;
            Exercise(f.TryAdd(xml));
        }
    }
    SUCCEED();
}

TEST_CASE("DMX models with hostile channel counts either load or throw", "[models2][robustness][dmx]") {
    for (const char* t : { "DmxGeneral", "DmxFloodlight", "DmxFloodArea", "DmxSkull", "DmxMovingHead" }) {
        for (const char* v : { "-5", "-1", "0", "1", "abc", "" }) {
            INFO(t << " " << v);
            std::string xml = std::string("<model name=\"M\" DisplayAs=\"") + t + "\" StartChannel=\"1\" StringType=\"Single Color White\" DmxChannelCount=\"" + v + "\"/>";
            ShowFixture f;
            Model* m = f.TryAdd(xml);
            Exercise(m);
            if (m != nullptr) {
                CHECK(m->GetNodeCount() == m->GetChanCount());
            }
        }
    }
}

TEST_CASE("DMX models with hostile channel assignments load", "[models2][robustness][dmx]") {
    for (const char* t : { "DmxFloodlight", "DmxMovingHead", "DmxMovingHeadAdv", "DmxSkull" }) {
        for (const char* v : { "-1", "0", "1000", "abc" }) {
            INFO(t << " " << v);
            std::string xml = std::string("<model name=\"M\" DisplayAs=\"") + t + "\" StartChannel=\"1\" StringType=\"Single Color White\" DmxChannelCount=\"40\" NodeNames=\"a\" DmxRedChannel=\"" + v + "\" DmxGreenChannel=\"" + v + "\" DmxBlueChannel=\"" + v +
                              "\" DmxWhiteChannel=\"" + v + "\" DmxShutterChannel=\"" + v + "\" MhDimmerChannel=\"" + v + "\" DmxColorType=\"" + v + "\" DmxPresetChannel0=\"" + v + "\" DmxPresetValue0=\"" + v + "\" DmxBeamLength=\"" + v + "\" DmxBeamWidth=\"" + v +
                              "\"><PanMotor ChannelCoarse=\"" + v + "\" ChannelFine=\"" + v + "\" RangeOfMotion=\"" + v + "\" SpeedChannel=\"" + v + "\"/><TiltMotor ChannelCoarse=\"" + v + "\" MinLimit=\"" + v + "\" MaxLimit=\"" + v + "\"/></model>";
            ShowFixture f;
            Model* m = f.TryAdd(xml);
            Exercise(m);
            if (m != nullptr) {
                (void)m->CheckModelSettings();
            }
        }
    }
    SUCCEED();
}

TEST_CASE("Servo models with hostile servo counts either load or throw", "[models2][robustness][dmx]") {
    for (const char* t : { "DmxServo", "DmxServo3d" }) {
        for (const char* v : { "-1", "0", "1", "25", "abc" }) {
            INFO(t << " " << v);
            std::string xml = std::string("<model name=\"M\" DisplayAs=\"") + t + "\" StartChannel=\"1\" StringType=\"Single Color White\" DmxChannelCount=\"4\" NumServos=\"" + v + "\" NumStatic=\"" + v + "\" NumMotion=\"" + v + "\"/>";
            ShowFixture f;
            Exercise(f.TryAdd(xml));
        }
    }
    SUCCEED();
}

TEST_CASE("Servo element names with odd numbering are ignored", "[models2][robustness][dmx]") {
    ShowFixture f;
    Model* m = f.TryAdd("<model name=\"M\" DisplayAs=\"DmxServo\" StartChannel=\"1\" StringType=\"Single Color White\" NumServos=\"2\" DmxChannelCount=\"4\">"
                        "<Servo2 Channel=\"3\"/><Servo99 Channel=\"1\"/><StaticImage2 Image=\"\"/><MotionImage99 Image=\"\"/><Unrelated/></model>");
    REQUIRE(m != nullptr);
    CHECK(m->GetNodeCount() == 4);
}

TEST_CASE("Hostile sub model definitions never crash", "[models2][robustness][submodel]") {
    const std::vector<std::string> subs = {
        "<subModel/>",
        "<subModel name=\"\"/>",
        "<subModel name=\"s\"/>",
        "<subModel name=\"s\" type=\"weird\" layout=\"sideways\" bufferstyle=\"nothing\"/>",
        "<subModel name=\"s\" type=\"ranges\" line0=\"\"/>",
        "<subModel name=\"s\" type=\"ranges\" line0=\"-1\"/>",
        "<subModel name=\"s\" type=\"ranges\" line0=\"1-\"/>",
        "<subModel name=\"s\" type=\"ranges\" line0=\"-5\"/>",
        "<subModel name=\"s\" type=\"ranges\" line0=\"a-b\"/>",
        "<subModel name=\"s\" type=\"ranges\" line0=\"5-1\"/>",
        "<subModel name=\"s\" type=\"ranges\" line0=\"1-5-9\"/>",
        "<subModel name=\"s\" type=\"ranges\" line0=\",,,\"/>",
        "<subModel name=\"s\" type=\"ranges\" line0=\"99999999999999999999\"/>",
        "<subModel name=\"s\" type=\"ranges\" line0=\"1-100000\"/>",
        "<subModel name=\"s\" type=\"ranges\" line0=\"1\" line5=\"2\" line99=\"3\"/>",
        "<subModel name=\"s\" type=\"ranges\" line0=\"1-3\"/><subModel name=\"s\" type=\"ranges\" line0=\"4-6\"/>",
        "<subModel name=\"s/t\" type=\"ranges\" line0=\"1-3\"/>",
        "<subModel name=\"s\" type=\"subbuffer\" subBuffer=\"\"/>",
        "<subModel name=\"s\" type=\"subbuffer\" subBuffer=\"0x0x0x0\"/>",
        "<subModel name=\"s\" type=\"subbuffer\" subBuffer=\"100x100x0x0\"/>",
        "<subModel name=\"s\" type=\"subbuffer\" subBuffer=\"-10x-10x500x500\"/>",
        "<subModel name=\"s\" type=\"subbuffer\" subBuffer=\"1x1\"/>",
        "<subModel name=\"s\" type=\"subbuffer\" subBuffer=\"axbxcxd\"/>",
        "<subModel name=\"s\" type=\"subbuffer\" subBuffer=\"0x0x100x100x5\"/>",
    };
    for (const auto& sub : subs) {
        INFO(sub);
        ShowFixture f;
        Model* m = f.TryAdd("<model name=\"M\" DisplayAs=\"Matrix\" StartChannel=\"1\" NumStrings=\"1\" NodesPerString=\"24\" StrandsPerString=\"4\">" + sub + "</model>");
        REQUIRE(m != nullptr);
        for (auto* s : m->GetSubModels()) {
            CHECK(s->GetNodeCount() <= m->GetNodeCount());
            Exercise(s);
            int w = 0, h = 0;
            s->GetBufferSize("Default", "2D", "None", w, h, 0);
            CHECK(w >= 0);
            CHECK(h >= 0);
            for (const char* style : { "Single Line", "Horizontal Per Strand", "Vertical Per Strand", "Keep XY", "** Single Line Legacy" }) {
                std::vector<NodeBaseClassPtr> nodes;
                int rw = 0, rh = 0;
                s->InitRenderBufferNodes(style, "2D", "None", nodes, rw, rh, 0);
                CHECK(rw >= 0);
                CHECK(rh >= 0);
            }
        }
        pugi::xml_document out;
        XmlSerializingVisitor v(&out);
        m->Accept(v);
    }
}

TEST_CASE("Sub model ranges reaching far past the model stay bounded", "[models2][robustness][submodel]") {
    ShowFixture f;
    Model* m = f.TryAdd("<model name=\"M\" DisplayAs=\"Matrix\" StartChannel=\"1\" NumStrings=\"1\" NodesPerString=\"24\" StrandsPerString=\"4\">"
                        "<subModel name=\"s\" type=\"ranges\" layout=\"horizontal\" bufferstyle=\"Default\" line0=\"1-2000000\"/></model>");
    REQUIRE(m != nullptr);
    Model* s = m->GetSubModel("s");
    REQUIRE(s != nullptr);
    CHECK(s->GetNodeCount() == 24);
}

TEST_CASE("Unusual model names are stored and found", "[models2][robustness]") {
    const std::vector<std::string> names = { "A B", "tab\there", "quote\"d", "amp&amp;ersand", "unicode \xE2\x98\x83", "dash-dash", "under_score", "dots.dots", std::string(2000, 'x') };
    for (const auto& n : names) {
        ShowFixture f;
        pugi::xml_document doc;
        pugi::xml_node node = doc.append_child("model");
        node.append_attribute("name") = n.c_str();
        node.append_attribute("DisplayAs") = "Single Line";
        node.append_attribute("StartChannel") = "1";
        node.append_attribute("NumStrings") = "1";
        node.append_attribute("NodesPerString") = "3";
        Model* m = f.mm.createAndAddModel(node, 100, 100);
        REQUIRE(m != nullptr);
        INFO(n.substr(0, 40));
        CHECK(!m->GetName().empty());
        CHECK(f.mm.GetModel(m->GetName()) == m);
        pugi::xml_document out;
        XmlSerializingVisitor v(&out);
        m->Accept(v);
        std::ostringstream os;
        out.save(os, "", pugi::format_raw);
        pugi::xml_document back;
        REQUIRE(back.load_string(os.str().c_str()));
        ShowFixture g;
        Model* m2 = g.mm.createAndAddModel(back.first_child(), 100, 100);
        REQUIRE(m2 != nullptr);
        CHECK(m2->GetName() == m->GetName());
    }
}

TEST_CASE("A model named like a sub model path is still looked up as a model", "[models2][robustness]") {
    ShowFixture f;
    f.TryAdd("<model name=\"T\" DisplayAs=\"Tree 360\" StartChannel=\"1\" NumStrings=\"1\" NodesPerString=\"10\" StrandsPerString=\"1\">"
             "<subModel name=\"h\" type=\"ranges\" layout=\"horizontal\" bufferstyle=\"Default\" line0=\"1-5\"/></model>");
    Model* other = f.TryAdd("<model name=\"T/h\" DisplayAs=\"Single Line\" StartChannel=\"100\" NumStrings=\"1\" NodesPerString=\"3\"/>");
    REQUIRE(other != nullptr);
    Model* found = f.mm.GetModel("T/h");
    REQUIRE(found != nullptr);
    CHECK((found == other || found->GetDisplayAs() == DisplayAsType::SubModel));
}

TEST_CASE("Hostile custom model grids load", "[models2][robustness]") {
    const std::vector<std::string> grids = {
        "", ";", ";;;", ",", ",,,;,,,", "1,2,3,4,5;6", "1;2;3;4;5", "0", "0,0;0,0", "-1,-2;-3,-4", "a,b;c,d", "1,2|3,4", "|", "||", "1,2;3,4|5,6;7,8|9,10;11,12",
        "99999,1;1,99999", "1.5,2.5;3.5,4.5", " 1 , 2 ; 3 , 4 ", "1,2;3,4;",
    };
    for (const auto& g : grids) {
        INFO(g);
        ShowFixture f;
        Model* m = f.TryAdd("<model name=\"C\" DisplayAs=\"Custom\" StartChannel=\"1\" CustomWidth=\"2\" CustomHeight=\"2\" Depth=\"2\" CustomModel=\"" + g + "\"/>");
        Exercise(m);
        if (m != nullptr) {
            for (uint32_t i = 0; i < m->GetNodeCount(); ++i) {
                CHECK(m->GetCoordCount(i) >= 1);
            }
        }
    }
}

TEST_CASE("Hostile compressed custom grids load", "[models2][robustness]") {
    const std::vector<std::string> grids = { "", ";", "1,0,0", "1,0,0;2,1,1", "1,5,5", "x,y,z", "1,0", "1,0,0,0", "1,0,0;;;", "99999,0,0" };
    for (const auto& g : grids) {
        INFO(g);
        ShowFixture f;
        Model* m = f.TryAdd("<model name=\"C\" DisplayAs=\"Custom\" StartChannel=\"1\" CustomWidth=\"2\" CustomHeight=\"2\" CustomModelCompressed=\"" + g + "\"/>");
        Exercise(m);
    }
    SUCCEED();
}

TEST_CASE("Hostile GDTF documents never crash the parser", "[models2][robustness][gdtf]") {
    const std::vector<std::string> docs = {
        "<GDTF/>",
        "<GDTF><FixtureType/></GDTF>",
        "<GDTF><FixtureType><DMXModes><DMXMode/></DMXModes></FixtureType></GDTF>",
        "<GDTF><FixtureType><DMXModes><DMXMode Name=\"\"><DMXChannels><DMXChannel/></DMXChannels></DMXMode></DMXModes></FixtureType></GDTF>",
        "<GDTF><FixtureType><DMXModes><DMXMode Name=\"m\"><DMXChannels><DMXChannel Offset=\"-5\"><LogicalChannel Attribute=\"Pan\"/></DMXChannel></DMXChannels></DMXMode></DMXModes></FixtureType></GDTF>",
        "<GDTF><FixtureType><DMXModes><DMXMode Name=\"m\"><DMXChannels><DMXChannel Offset=\"a,b,c\"><LogicalChannel Attribute=\"Pan\"/></DMXChannel></DMXChannels></DMXMode></DMXModes></FixtureType></GDTF>",
        "<GDTF><FixtureType><DMXModes><DMXMode Name=\"m\"><DMXChannels><DMXChannel Offset=\",,,,\"><LogicalChannel Attribute=\"Pan\"/></DMXChannel></DMXChannels></DMXMode></DMXModes></FixtureType></GDTF>",
        "<GDTF><FixtureType><DMXModes><DMXMode Name=\"m\"><DMXChannels><DMXChannel Offset=\"1\"><LogicalChannel><ChannelFunction><ChannelSet/></ChannelFunction></LogicalChannel></DMXChannel></DMXChannels></DMXMode></DMXModes></FixtureType></GDTF>",
        "<GDTF><FixtureType><DMXModes><DMXMode Name=\"m\"><DMXChannels><DMXChannel Offset=\"1\"><LogicalChannel Attribute=\"x\"><ChannelFunction><ChannelSet Name=\"n\" DMXFrom=\"/\"/><ChannelSet Name=\"o\" DMXFrom=\"//\"/><ChannelSet Name=\"p\" DMXFrom=\"1/\"/><ChannelSet Name=\"q\" DMXFrom=\"/1\"/></ChannelFunction></LogicalChannel></DMXChannel></DMXChannels></DMXMode></DMXModes></FixtureType></GDTF>",
        "<GDTF><FixtureType><DMXModes><DMXMode Name=\"m\"><DMXChannels><DMXChannel Offset=\"99999999999999999999\"><LogicalChannel Attribute=\"x\"/></DMXChannel></DMXChannels></DMXMode></DMXModes></FixtureType></GDTF>",
        "<GDTF><FixtureType><DMXModes><DMXMode Name=\"m\"/><DMXMode Name=\"m\"/><DMXMode Name=\"n\"/></DMXModes></FixtureType></GDTF>",
        "<FixtureType><DMXModes><DMXMode Name=\"m\"/></DMXModes></FixtureType>",
        "<A><B><C><D><E/></D></C></B></A>",
    };
    for (const auto& d : docs) {
        INFO(d);
        OutputManager om;
        ModelManager mm{ &om };
        pugi::xml_document doc;
        if (!doc.load_string(d.c_str())) {
            continue;
        }
        XmlSerialize::GdtfModelData data;
        bool cancelled = false;
        bool ok = XmlSerialize::ParseGdtfDescriptionXml(doc, mm, nullptr, cancelled, data);
        CHECK_FALSE(cancelled);
        if (ok) {
            CHECK(data.totalChannels >= 0);
        }
    }
}

TEST_CASE("Serializing models with no nodes does not crash", "[models2][robustness]") {
    const char* types[] = { "DmxGeneral", "DmxFloodlight", "DmxMovingHead", "DmxSkull" };
    for (const char* t : types) {
        INFO(t);
        ShowFixture f;
        Model* m = f.TryAdd(std::string("<model name=\"M\" DisplayAs=\"") + t + "\" StartChannel=\"1\" StringType=\"Single Color White\" DmxChannelCount=\"0\"/>");
        REQUIRE(m != nullptr);
        CHECK(m->GetNodeCount() == 0);
        Exercise(m);
    }
}

TEST_CASE("DMX servo with a zero channel count does not crash", "[models2-crash][robustness][known-crash][.]") {
    ShowFixture f;
    Model* m = f.TryAdd("<model name=\"M\" DisplayAs=\"DmxServo\" StartChannel=\"1\" StringType=\"Single Color White\" DmxChannelCount=\"0\"/>");
    CHECK(m != nullptr);
}

TEST_CASE("DMX servo 3d with a zero channel count does not crash", "[models2-crash][robustness][known-crash][.]") {
    ShowFixture f;
    Model* m = f.TryAdd("<model name=\"M\" DisplayAs=\"DmxServo3d\" StartChannel=\"1\" StringType=\"Single Color White\" DmxChannelCount=\"0\"/>");
    CHECK(m != nullptr);
}

TEST_CASE("Icicles with an empty or zero drop pattern load", "[models2-crash][robustness][known-crash][.]") {
    for (const char* v : { "", "abc", "0" }) {
        INFO(v);
        ShowFixture f;
        f.TryAdd(std::string("<model name=\"M\" DisplayAs=\"Icicles\" StartChannel=\"1\" NumStrings=\"2\" NodesPerString=\"5\" DropPattern=\"") + v + "\"/>");
    }
}

TEST_CASE("Spinner with negative arms per string loads", "[models2-crash][robustness][known-crash][.]") {
    ShowFixture f;
    f.TryAdd("<model name=\"M\" DisplayAs=\"Spinner\" StartChannel=\"1\" NumStrings=\"1\" NodesPerArm=\"3\" ArmsPerString=\"-1\"/>");
}

TEST_CASE("Window frame with negative bottom nodes loads", "[models2-crash][robustness][known-crash][.]") {
    ShowFixture f;
    f.TryAdd("<model name=\"M\" DisplayAs=\"Window Frame\" StartChannel=\"1\" TopNodes=\"3\" SideNodes=\"2\" BottomNodes=\"-1\"/>");
}

TEST_CASE("Poly line with zero or invalid poly strings loads", "[models2-crash][robustness][known-crash][.]") {
    for (const char* v : { "", "abc", "0" }) {
        INFO(v);
        ShowFixture f;
        f.TryAdd(std::string("<model name=\"M\" DisplayAs=\"Poly Line\" StartChannel=\"1\" NumStrings=\"1\" NodesPerString=\"6\" NumPoints=\"3\" PointData=\"0.0,0.0,0.0,5.0,5.0,0.0,10.0,0.0,0.0\" PolyStrings=\"") + v + "\"/>");
    }
}

TEST_CASE("Custom model with zero or invalid strings loads", "[models2-crash][robustness][known-crash][.]") {
    for (const char* v : { "", "abc", "0" }) {
        INFO(v);
        ShowFixture f;
        f.TryAdd(std::string("<model name=\"M\" DisplayAs=\"Custom\" StartChannel=\"1\" CustomWidth=\"3\" CustomHeight=\"2\" CustomModel=\"1,2,3;4,5,6\" CustomStrings=\"") + v + "\"/>");
    }
}

TEST_CASE("Servo model with servo element names lacking a valid number loads", "[models2-crash][robustness][known-crash][.]") {
    for (const char* e : { "<ServoX Channel=\"1\"/>", "<Servo0 Channel=\"1\"/>", "<Servo Channel=\"1\"/>", "<StaticImage0 Image=\"\"/>", "<StaticImageX Image=\"\"/>" }) {
        INFO(e);
        ShowFixture f;
        f.TryAdd(std::string("<model name=\"M\" DisplayAs=\"DmxServo\" StartChannel=\"1\" StringType=\"Single Color White\" NumServos=\"2\" DmxChannelCount=\"4\">") + e + "</model>");
    }
}

TEST_CASE("Custom model with negative compressed coordinates loads", "[models2-crash][robustness][known-crash][.]") {
    ShowFixture f;
    f.TryAdd("<model name=\"C\" DisplayAs=\"Custom\" StartChannel=\"1\" CustomWidth=\"2\" CustomHeight=\"2\" CustomModelCompressed=\"1,-1,-1\"/>");
}
