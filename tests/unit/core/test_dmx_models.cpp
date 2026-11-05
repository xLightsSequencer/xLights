#include <algorithm>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <pugixml.hpp>

#include "XmlSerializer/XmlSerializer.h"
#include "models/DMX/DmxBeamAbility.h"
#include "models/DMX/DmxColorAbilityCMY.h"
#include "models/DMX/DmxColorAbilityRGB.h"
#include "models/DMX/DmxColorAbilityWheel.h"
#include "models/DMX/DmxDimmerAbility.h"
#include "models/DMX/DmxFloodArea.h"
#include "models/DMX/DmxFloodlight.h"
#include "models/DMX/DmxGeneral.h"
#include "models/DMX/DmxImage.h"
#include "models/DMX/DmxModel.h"
#include "models/DMX/DmxMotor.h"
#include "models/DMX/DmxMovingHead.h"
#include "models/DMX/DmxMovingHeadAdv.h"
#include "models/DMX/DmxPresetAbility.h"
#include "models/DMX/DmxServo.h"
#include "models/DMX/DmxServo3D.h"
#include "models/DMX/DmxShutterAbility.h"
#include "models/DMX/DmxSkull.h"
#include "models/DMX/Servo.h"
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

std::string Serialize(const Model* m) {
    pugi::xml_document doc;
    XmlSerializingVisitor v(&doc);
    m->Accept(v);
    std::ostringstream os;
    doc.save(os, "", pugi::format_raw);
    return os.str();
}

std::string Dmx(const std::string& name, const std::string& type, const std::string& attrs, const std::string& children = "") {
    return "<model name=\"" + name + "\" DisplayAs=\"" + type + "\" StartChannel=\"1\" StringType=\"Single Color White\" " + attrs + ">" + children + "</model>";
}

template <class T>
T* As(Model* m) {
    T* t = dynamic_cast<T*>(m);
    REQUIRE(t != nullptr);
    return t;
}

const char* const kDmxTypes[] = { "DmxMovingHead", "DmxMovingHeadAdv", "DmxFloodlight", "DmxFloodArea", "DmxSkull", "DmxServo", "DmxServo3d", "DmxGeneral" };

} // namespace

TEST_CASE("Every default DMX model has one channel per node", "[models2][dmx]") {
    for (const char* t : kDmxTypes) {
        INFO(t);
        ShowFixture f;
        Model* m = f.Default(t);
        CHECK(m->IsDMXModel());
        CHECK(m->GetChanCountPerNode() == 1);
        CHECK(m->GetNodeCount() == m->GetChanCount());
        CHECK(m->GetNumPhysicalStrings() == 1);
        CHECK(m->GetNumStrings() == (int)m->GetNodeCount());
        CHECK(m->GetDefaultBufferHt() == 1);
        CHECK(m->GetDefaultBufferWi() == (int)m->GetNodeCount());
    }
}

TEST_CASE("Default DMX models report their expected channel counts", "[models2][dmx]") {
    struct Case {
        const char* type;
        int channels;
    };
    const Case cases[] = { { "DmxMovingHead", 8 }, { "DmxMovingHeadAdv", 8 }, { "DmxFloodlight", 3 }, { "DmxFloodArea", 3 }, { "DmxSkull", 26 }, { "DmxServo", 2 }, { "DmxServo3d", 2 }, { "DmxGeneral", 8 } };
    for (const auto& c : cases) {
        INFO(c.type);
        ShowFixture f;
        Model* m = f.Default(c.type);
        CHECK((int)m->GetChanCount() == c.channels);
        CHECK(As<DmxModel>(m)->GetDmxChannelCount() == c.channels);
    }
}

TEST_CASE("DMX model buffer is a single row one node per channel", "[models2][dmx]") {
    ShowFixture f;
    Model* m = f.Add(Dmx("G", "DmxGeneral", "DmxChannelCount=\"12\""));
    int w = 0, h = 0;
    m->GetBufferSize("Default", "2D", "None", w, h, 0);
    CHECK(w == 12);
    CHECK(h == 1);
    m->GetBufferSize("Per Preview", "2D", "None", w, h, 0);
    CHECK(w == 12);
    CHECK(h == 1);
    std::vector<NodeBaseClassPtr> nodes;
    m->InitRenderBufferNodes("Default", "2D", "None", nodes, w, h, 0);
    REQUIRE(nodes.size() == 12);
    for (size_t i = 0; i < nodes.size(); ++i) {
        CHECK(nodes[i]->Coords[0].bufX == (int)i);
        CHECK(nodes[i]->Coords[0].bufY == 0);
        CHECK(nodes[i]->ActChan == (uint32_t)i);
    }
}

TEST_CASE("DMX channel count attribute sets the node count", "[models2][dmx]") {
    for (const char* t : { "DmxFloodlight", "DmxGeneral", "DmxFloodArea" }) {
        INFO(t);
        ShowFixture f;
        Model* m = f.Add(Dmx("D", t, "DmxChannelCount=\"17\""));
        CHECK(m->GetNodeCount() == 17);
        CHECK(m->GetChanCount() == 17);
        CHECK(m->GetLastChannel() - m->GetFirstChannel() == 16);
    }
}

TEST_CASE("DMX channel count falls back to parm1 and then to one", "[models2][dmx]") {
    ShowFixture f;
    Model* a = f.Add(Dmx("A", "DmxGeneral", "parm1=\"6\""));
    CHECK(a->GetNodeCount() == 6);
    Model* b = f.Add(Dmx("B", "DmxGeneral", ""));
    CHECK(b->GetNodeCount() == 1);
}

TEST_CASE("DMX nodes sit on consecutive channels from the start channel", "[models2][dmx]") {
    ShowFixture f;
    Model* m = f.Add("<model name=\"D\" DisplayAs=\"DmxGeneral\" StartChannel=\"41\" StringType=\"Single Color White\" DmxChannelCount=\"5\"/>");
    f.mm.RecalcStartChannels();
    REQUIRE(m->GetNodeCount() == 5);
    for (uint32_t i = 0; i < 5; ++i) {
        CHECK(m->NodeStartChannel(i) == (int32_t)(40 + i));
        CHECK(m->NodeEndChannel(i) == (int32_t)(40 + i));
    }
    CHECK(m->GetFirstChannel() == 40);
    CHECK(m->GetLastChannel() == 44);
}

TEST_CASE("DMX UpdateChannelCount changes the stored count", "[models2][dmx]") {
    ShowFixture f;
    DmxModel* m = As<DmxModel>(f.Add(Dmx("D", "DmxGeneral", "DmxChannelCount=\"4\"")));
    m->UpdateChannelCount(9, false);
    CHECK(m->GetDmxChannelCount() == 9);
    m->SetDmxChannelCount(3);
    CHECK(m->GetDmxChannelCount() == 3);
    CHECK(m->GetNumStrings() == 3);
}

TEST_CASE("Moving head loads pan and tilt motors from child elements", "[models2][dmx]") {
    ShowFixture f;
    auto* mh = As<DmxMovingHead>(f.Add(Dmx("MH", "DmxMovingHead", "DmxChannelCount=\"16\"",
        "<PanMotor ChannelCoarse=\"5\" ChannelFine=\"6\" MinLimit=\"-100\" MaxLimit=\"200\" RangeOfMotion=\"360\" OrientZero=\"10\" OrientHome=\"20\" SlewLimit=\"90\" SpeedChannel=\"7\" Reverse=\"1\" UpsideDown=\"1\"/>"
        "<TiltMotor ChannelCoarse=\"8\" RangeOfMotion=\"270\"/>")));
    DmxMotor* pan = mh->GetPanMotor();
    CHECK(pan->GetChannelCoarse() == 5);
    CHECK(pan->GetChannelFine() == 6);
    CHECK(pan->Is16Bit());
    CHECK(pan->GetMinLimit() == -100);
    CHECK(pan->GetMaxLimit() == 200);
    CHECK(pan->GetRangeOfMotion() == Catch::Approx(360.0f));
    CHECK(pan->GetOrientZero() == 10);
    CHECK(pan->GetOrientHome() == 20);
    CHECK(pan->GetSlewLimit() == Catch::Approx(90.0f));
    CHECK(pan->GetSpeedChannel() == 7);
    CHECK(pan->GetReverse());
    CHECK(pan->GetUpsideDown());
    DmxMotor* tilt = mh->GetTiltMotor();
    CHECK(tilt->GetChannelCoarse() == 8);
    CHECK(tilt->GetChannelFine() == 0);
    CHECK_FALSE(tilt->Is16Bit());
    CHECK(tilt->GetRangeOfMotion() == Catch::Approx(270.0f));
    CHECK(tilt->GetMinLimit() == -180);
    CHECK(tilt->GetMaxLimit() == 180);
    CHECK_FALSE(tilt->GetReverse());
}

TEST_CASE("Moving head motors round trip through serialization", "[models2][dmx]") {
    ShowFixture a;
    Model* m = a.Add(Dmx("MH", "DmxMovingHead", "DmxChannelCount=\"16\" DmxFixture=\"MH3\" DmxStyle=\"Moving Head Side\" HideBody=\"True\"",
        "<PanMotor ChannelCoarse=\"5\" ChannelFine=\"6\" RangeOfMotion=\"360\" Reverse=\"1\"/><TiltMotor ChannelCoarse=\"8\" SpeedChannel=\"9\"/>"));
    std::string s1 = Serialize(m);
    ShowFixture b;
    pugi::xml_document doc;
    REQUIRE(doc.load_string(s1.c_str()));
    Model* m2 = b.mm.createAndAddModel(doc.first_child(), 100, 100);
    REQUIRE(m2 != nullptr);
    CHECK(Serialize(m2) == s1);
    auto* mh = As<DmxMovingHead>(m2);
    CHECK(mh->GetFixture() == "MH3");
    CHECK(mh->GetFixtureVal() == 3);
    CHECK(mh->GetDmxStyle() == "Moving Head Side");
    CHECK(mh->GetHideBody());
    CHECK(mh->GetPanMotor()->GetReverse());
    CHECK(mh->GetTiltMotor()->GetSpeedChannel() == 9);
}

TEST_CASE("Moving head style names map to style values", "[models2][dmx]") {
    struct Case {
        const char* style;
        int value;
    };
    const Case cases[] = { { "Moving Head Top", 0 }, { "Moving Head Side", 1 }, { "Moving Head Bars", 2 }, { "Moving Head TopBars", 3 }, { "Moving Head SideBars", 4 }, { "Moving Head 3D", 5 }, { "Whatever", 0 } };
    for (const auto& c : cases) {
        INFO(c.style);
        ShowFixture f;
        auto* mh = As<DmxMovingHead>(f.Add(Dmx("MH", "DmxMovingHead", std::string("DmxStyle=\"") + c.style + "\" DmxChannelCount=\"8\"")));
        CHECK(mh->GetDmxStyleVal() == c.value);
    }
}

TEST_CASE("Moving head fixture id follows the DmxFixture attribute", "[models2][dmx]") {
    for (int i = 1; i <= 8; ++i) {
        ShowFixture f;
        auto* mh = As<DmxMovingHead>(f.Add(Dmx("MH", "DmxMovingHead", "DmxChannelCount=\"8\" DmxFixture=\"MH" + std::to_string(i) + "\"")));
        CHECK(mh->GetFixtureVal() == i);
        CHECK(mh->GetFixture() == "MH" + std::to_string(i));
    }
    ShowFixture f;
    auto* mh = As<DmxMovingHead>(f.Add(Dmx("MH", "DmxMovingHead", "DmxChannelCount=\"8\" DmxFixture=\"nonsense\"")));
    CHECK(mh->GetFixtureVal() == 1);
}

TEST_CASE("Legacy flat pan and tilt attributes become motor elements", "[models2][dmx]") {
    ShowFixture f;
    auto* mh = As<DmxMovingHead>(f.Add(Dmx("MH", "DmxMovingHead", "DmxChannelCount=\"12\" DmxPanChannel=\"4\" DmxTiltChannel=\"6\" DmxPanSlewLimit=\"45\" DmxPanDegOfRot=\"-360\" DmxTiltDegOfRot=\"90\"")));
    CHECK(mh->GetPanMotor()->GetChannelCoarse() == 4);
    CHECK(mh->GetTiltMotor()->GetChannelCoarse() == 6);
    CHECK(mh->GetPanMotor()->GetSlewLimit() == Catch::Approx(45.0f));
    CHECK(mh->GetPanMotor()->GetRangeOfMotion() == Catch::Approx(360.0f));
    CHECK(mh->GetPanMotor()->GetReverse());
    CHECK(mh->GetTiltMotor()->GetRangeOfMotion() == Catch::Approx(90.0f));
    CHECK_FALSE(mh->GetTiltMotor()->GetReverse());
    std::string s = Serialize(mh);
    CHECK(s.find("DmxPanChannel") == std::string::npos);
    CHECK(s.find("<PanMotor") != std::string::npos);
}

TEST_CASE("Legacy DMX display type is split by its style", "[models2][dmx]") {
    ShowFixture f;
    Model* flood = f.Add("<model name=\"F\" DisplayAs=\"DMX\" StartChannel=\"1\" StringType=\"Single Color White\" DmxStyle=\"Flood Light\" parm1=\"3\"/>");
    CHECK(flood->GetDisplayAsString() == "DmxFloodlight");
    Model* skull = f.Add("<model name=\"S\" DisplayAs=\"DMX\" StartChannel=\"1\" StringType=\"Single Color White\" DmxStyle=\"Skulltronix Skull\" parm1=\"26\"/>");
    CHECK(skull->GetDisplayAsString() == "DmxSkull");
    Model* mh = f.Add("<model name=\"M\" DisplayAs=\"DMX\" StartChannel=\"1\" StringType=\"Single Color White\" DmxStyle=\"Moving Head Top\" parm1=\"8\"/>");
    CHECK(mh->GetDisplayAsString() == "DmxMovingHead");
}

TEST_CASE("Legacy DmxMovingHead3D becomes an advanced moving head", "[models2][dmx]") {
    ShowFixture f;
    Model* m = f.Add(Dmx("M", "DmxMovingHead3D", "DmxChannelCount=\"8\" DmxBeamLength=\"4\" DmxStyle=\"Moving Head 3D\""));
    REQUIRE(m != nullptr);
    CHECK(m->GetDisplayAsString() == "DmxMovingHeadAdv");
    auto* adv = As<DmxMovingHeadAdv>(m);
    CHECK(adv->GetBeamAbility()->GetBeamLength() == Catch::Approx(5.1f).margin(0.01));
    CHECK(adv->GetBeamAbility()->GetBeamYOffset() == Catch::Approx(0.0f));
}

TEST_CASE("DMX colour type selects the colour ability", "[models2][dmx]") {
    struct Case {
        int type;
        DmxColorAbility::DMX_COLOR_TYPE expect;
    };
    const Case cases[] = { { 0, DmxColorAbility::DMX_COLOR_TYPE::DMX_COLOR_RGBW }, { 1, DmxColorAbility::DMX_COLOR_TYPE::DMX_COLOR_WHEEL }, { 2, DmxColorAbility::DMX_COLOR_TYPE::DMX_COLOR_CMYW } };
    for (const auto& c : cases) {
        INFO(c.type);
        ShowFixture f;
        auto* mh = As<DmxMovingHeadAdv>(f.Add(Dmx("MH", "DmxMovingHeadAdv", "DmxChannelCount=\"20\" DmxColorType=\"" + std::to_string(c.type) + "\"")));
        REQUIRE(mh->HasColorAbility());
        CHECK(mh->GetColorAbility()->GetColorType() == c.expect);
    }
}

TEST_CASE("DMX colour type 3 removes the colour ability", "[models2][dmx]") {
    ShowFixture f;
    auto* mh = As<DmxMovingHeadAdv>(f.Add(Dmx("MH", "DmxMovingHeadAdv", "DmxChannelCount=\"20\" DmxColorType=\"3\"")));
    CHECK_FALSE(mh->HasColorAbility());
    std::vector<std::string> names = mh->GenerateNodeNames();
    CHECK(std::find(names.begin(), names.end(), "Red") == names.end());
}

TEST_CASE("RGBW channels load from attributes and default to 1 2 3", "[models2][dmx]") {
    ShowFixture f;
    auto* mh = As<DmxMovingHead>(f.Add(Dmx("MH", "DmxMovingHead", "DmxChannelCount=\"12\" DmxRedChannel=\"9\" DmxGreenChannel=\"10\" DmxBlueChannel=\"11\" DmxWhiteChannel=\"12\" DmxRedBrightness=\"50\" DmxGreenGamma=\"2.2\"")));
    auto* rgb = dynamic_cast<DmxColorAbilityRGB*>(mh->GetColorAbility());
    REQUIRE(rgb != nullptr);
    CHECK(rgb->GetRedChannel() == 9);
    CHECK(rgb->GetGreenChannel() == 10);
    CHECK(rgb->GetBlueChannel() == 11);
    CHECK(rgb->GetWhiteChannel() == 12);
    CHECK(rgb->GetRedBrightness() == 50);
    CHECK(rgb->GetBlueBrightness() == 100);
    CHECK(rgb->GetGreenGamma() == Catch::Approx(2.2f));
    CHECK(rgb->GetNumChannels() == 4);

    ShowFixture g;
    auto* bare = As<DmxMovingHead>(g.Add(Dmx("MH", "DmxMovingHead", "DmxChannelCount=\"12\"")));
    auto* rgb2 = dynamic_cast<DmxColorAbilityRGB*>(bare->GetColorAbility());
    REQUIRE(rgb2 != nullptr);
    CHECK(rgb2->GetRedChannel() == 1);
    CHECK(rgb2->GetGreenChannel() == 2);
    CHECK(rgb2->GetBlueChannel() == 3);
    CHECK(rgb2->GetWhiteChannel() == 0);
}

TEST_CASE("CMYW channels load from attributes", "[models2][dmx]") {
    ShowFixture f;
    auto* mh = As<DmxMovingHeadAdv>(f.Add(Dmx("MH", "DmxMovingHeadAdv", "DmxChannelCount=\"20\" DmxColorType=\"2\" DmxCyanChannel=\"10\" DmxMagentaChannel=\"11\" DmxYellowChannel=\"12\" DmxWhiteChannel=\"13\"")));
    auto* cmy = dynamic_cast<DmxColorAbilityCMY*>(mh->GetColorAbility());
    REQUIRE(cmy != nullptr);
    CHECK(cmy->GetCyanChannel() == 10);
    CHECK(cmy->GetMagentaChannel() == 11);
    CHECK(cmy->GetYellowChannel() == 12);
    CHECK(cmy->GetWhiteChannel() == 13);
}

TEST_CASE("Colour wheel settings load and round trip", "[models2][dmx]") {
    ShowFixture a;
    Model* m = a.Add(Dmx("MH", "DmxMovingHeadAdv", "DmxChannelCount=\"20\" DmxColorType=\"1\" DmxColorWheelChannel=\"9\" DmxDimmerChannel=\"10\" DmxColorWheelDelay=\"250\" DmxColorWheelDMX0=\"5\" DmxColorWheelColor0=\"#FF0000\" DmxColorWheelDMX1=\"15\" DmxColorWheelColor1=\"#00FF00\" DmxColorWheelDMX2=\"25\" DmxColorWheelColor2=\"#0000FF\""));
    auto* wheel = dynamic_cast<DmxColorAbilityWheel*>(As<DmxMovingHeadAdv>(m)->GetColorAbility());
    REQUIRE(wheel != nullptr);
    CHECK(wheel->GetWheelChannel() == 9);
    CHECK(wheel->GetDimmerChannel() == 10);
    CHECK(wheel->GetWheelDelay() == 250);
    REQUIRE(wheel->GetColorWheelColorSize() == 3);
    CHECK(wheel->GetWheelColorSettings()[1].dmxValue == 15);
    CHECK(wheel->GetWheelColorSettings()[1].color == xlGREEN);
    std::string s1 = Serialize(m);
    ShowFixture b;
    pugi::xml_document doc;
    REQUIRE(doc.load_string(s1.c_str()));
    Model* m2 = b.mm.createAndAddModel(doc.first_child(), 100, 100);
    REQUIRE(m2 != nullptr);
    CHECK(Serialize(m2) == s1);
}

TEST_CASE("Colour wheel loading stops at the first missing entry", "[models2][dmx]") {
    ShowFixture f;
    auto* mh = As<DmxMovingHeadAdv>(f.Add(Dmx("MH", "DmxMovingHeadAdv", "DmxChannelCount=\"20\" DmxColorType=\"1\" DmxColorWheelDMX0=\"5\" DmxColorWheelColor0=\"#FF0000\" DmxColorWheelDMX2=\"25\" DmxColorWheelColor2=\"#0000FF\"")));
    auto* wheel = dynamic_cast<DmxColorAbilityWheel*>(mh->GetColorAbility());
    REQUIRE(wheel != nullptr);
    CHECK(wheel->GetColorWheelColorSize() == 1);
}

TEST_CASE("Shutter and dimmer settings load", "[models2][dmx]") {
    ShowFixture f;
    auto* mh = As<DmxMovingHead>(f.Add(Dmx("MH", "DmxMovingHead", "DmxChannelCount=\"16\" DmxShutterChannel=\"12\" DmxShutterOpen=\"20\" DmxShutterOnValue=\"255\" MhDimmerChannel=\"13\"")));
    REQUIRE(mh->HasShutterAbility());
    CHECK(mh->GetShutterAbility()->GetShutterChannel() == 12);
    CHECK(mh->GetShutterAbility()->GetShutterThreshold() == 20);
    CHECK(mh->GetShutterAbility()->GetShutterOnValue() == 255);
    REQUIRE(mh->HasDimmerAbility());
    CHECK(mh->GetDimmerAbility()->GetDimmerChannel() == 13);
}

TEST_CASE("Beam settings load with per model defaults", "[models2][dmx]") {
    ShowFixture f;
    auto* mh = As<DmxMovingHead>(f.Add(Dmx("MH", "DmxMovingHead", "DmxChannelCount=\"8\"")));
    REQUIRE(mh->HasBeamAbility());
    CHECK(mh->GetBeamAbility()->GetBeamLength() == Catch::Approx(4.0f));
    CHECK(mh->GetBeamAbility()->GetBeamWidth() == Catch::Approx(30.0f));
    ShowFixture g;
    auto* mh2 = As<DmxMovingHead>(g.Add(Dmx("MH", "DmxMovingHead", "DmxChannelCount=\"8\" DmxBeamLength=\"9.5\" DmxBeamWidth=\"12\"")));
    CHECK(mh2->GetBeamAbility()->GetBeamLength() == Catch::Approx(9.5f));
    CHECK(mh2->GetBeamAbility()->GetBeamWidth() == Catch::Approx(12.0f));
}

TEST_CASE("Preset channels load from numbered attributes", "[models2][dmx]") {
    ShowFixture f;
    auto* mh = As<DmxMovingHeadAdv>(f.Add(Dmx("MH", "DmxMovingHeadAdv", "DmxChannelCount=\"20\" DmxPresetChannel0=\"14\" DmxPresetValue0=\"128\" DmxPresetDesc0=\"Gobo\" DmxPresetChannel1=\"15\" DmxPresetValue1=\"64\" DmxPresetDesc1=\"Prism\"")));
    REQUIRE(mh->HasPresetAbility());
    auto* p = mh->GetPresetAbility();
    REQUIRE(p->GetPresetsCount() == 2);
    CHECK(p->GetPresetSettings()[0].DMXChannel == 14);
    CHECK(p->GetPresetSettings()[0].DMXValue == 128);
    CHECK(p->GetPresetSettings()[1].Description == "Prism");
    xlColorVector px(20, xlBLACK);
    mh->EnableFixedChannels(px);
    CHECK(px[13].red == 128);
    CHECK(px[14].red == 64);
}

TEST_CASE("Preset settings round trip", "[models2][dmx]") {
    ShowFixture a;
    Model* m = a.Add(Dmx("MH", "DmxMovingHeadAdv", "DmxChannelCount=\"20\" DmxPresetChannel0=\"14\" DmxPresetValue0=\"128\" DmxPresetDesc0=\"Gobo\""));
    std::string s1 = Serialize(m);
    CHECK(s1.find("DmxPresetDesc0=\"Gobo\"") != std::string::npos);
    ShowFixture b;
    pugi::xml_document doc;
    REQUIRE(doc.load_string(s1.c_str()));
    Model* m2 = b.mm.createAndAddModel(doc.first_child(), 100, 100);
    CHECK(Serialize(m2) == s1);
}

TEST_CASE("Moving head advanced names its channels", "[models2][dmx]") {
    ShowFixture f;
    auto* mh = As<DmxMovingHeadAdv>(f.Add(Dmx("MH", "DmxMovingHeadAdv", "DmxChannelCount=\"20\" DmxShutterChannel=\"11\" DmxShutterOnValue=\"255\" DmxRedChannel=\"5\" DmxGreenChannel=\"6\" DmxBlueChannel=\"7\"",
        "<PanMotor ChannelCoarse=\"1\" ChannelFine=\"2\"/><TiltMotor ChannelCoarse=\"3\" ChannelFine=\"4\"/>")));
    auto names = mh->GenerateNodeNames();
    REQUIRE(names.size() == 20);
    CHECK(names[0] == "Pan");
    CHECK(names[1] == "Pan Fine");
    CHECK(names[2] == "Tilt");
    CHECK(names[3] == "Tilt Fine");
    CHECK(names[4] == "Red");
    CHECK(names[5] == "Green");
    CHECK(names[6] == "Blue");
    CHECK(names[10] == "Shutter");
    CHECK(names[19].empty());
}

TEST_CASE("Basic moving head names pan tilt and speed channels", "[models2][dmx]") {
    ShowFixture f;
    auto* mh = As<DmxMovingHead>(f.Add(Dmx("MH", "DmxMovingHead", "DmxChannelCount=\"12\" DmxRedChannel=\"9\" DmxGreenChannel=\"10\" DmxBlueChannel=\"11\"",
        "<PanMotor ChannelCoarse=\"1\" ChannelFine=\"2\" SpeedChannel=\"5\"/><TiltMotor ChannelCoarse=\"3\" ChannelFine=\"4\" SpeedChannel=\"6\"/>")));
    auto names = mh->GenerateNodeNames();
    REQUIRE(names.size() == 12);
    CHECK(names[0] == "Pan");
    CHECK(names[1] == "Pan Fine");
    CHECK(names[2] == "Tilt");
    CHECK(names[3] == "Tilt Fine");
    CHECK(names[4] == "Pan Speed");
    CHECK(names[5] == "Tilt Speed");
    CHECK(names[8] == "Red");
    CHECK(names[10] == "Blue");
}

TEST_CASE("Floodlight names its colour channels", "[models2][dmx]") {
    ShowFixture f;
    auto* fl = As<DmxFloodlight>(f.Add(Dmx("F", "DmxFloodlight", "DmxChannelCount=\"4\" DmxRedChannel=\"1\" DmxGreenChannel=\"2\" DmxBlueChannel=\"3\" DmxWhiteChannel=\"4\"")));
    auto names = fl->GenerateNodeNames();
    REQUIRE(names.size() == 4);
    CHECK(names[0] == "Red");
    CHECK(names[1] == "Green");
    CHECK(names[2] == "Blue");
    CHECK(names[3] == "White");
}

TEST_CASE("Flood area is a floodlight with its own type", "[models2][dmx]") {
    ShowFixture f;
    Model* m = f.Default("DmxFloodArea");
    CHECK(m->GetDisplayAsString() == "DmxFloodArea");
    CHECK(dynamic_cast<DmxFloodlight*>(m) != nullptr);
    CHECK(dynamic_cast<DmxFloodArea*>(m) != nullptr);
    CHECK(m->GetDisplayAsString() != f.Default("DmxFloodlight")->GetDisplayAsString());
}

TEST_CASE("DMX model warns about colour channels beyond the channel count", "[models2][dmx]") {
    ShowFixture f;
    Model* m = f.Add(Dmx("F", "DmxFloodlight", "DmxChannelCount=\"3\" DmxRedChannel=\"1\" DmxGreenChannel=\"2\" DmxBlueChannel=\"9\""));
    auto res = m->CheckModelSettings();
    bool found = false;
    for (const auto& s : res) {
        if (s.find("blue channel") != std::string::npos) {
            found = true;
        }
    }
    CHECK(found);
}

TEST_CASE("DMX model with in range channels reports no colour errors", "[models2][dmx]") {
    ShowFixture f;
    Model* m = f.Add(Dmx("F", "DmxFloodlight", "DmxChannelCount=\"3\" DmxRedChannel=\"1\" DmxGreenChannel=\"2\" DmxBlueChannel=\"3\""));
    for (const auto& s : m->CheckModelSettings()) {
        CHECK(s.find("channel refers to") == std::string::npos);
    }
}

TEST_CASE("Shutter channel with zero on value draws a warning", "[models2][dmx]") {
    ShowFixture f;
    Model* m = f.Add(Dmx("MH", "DmxMovingHead", "DmxChannelCount=\"12\" DmxShutterChannel=\"10\" DmxShutterOnValue=\"0\""));
    bool warned = false;
    for (const auto& s : m->CheckModelSettings()) {
        if (s.find("Shutter On Value") != std::string::npos) {
            warned = true;
        }
    }
    CHECK(warned);
    ShowFixture g;
    Model* ok = g.Add(Dmx("MH", "DmxMovingHead", "DmxChannelCount=\"12\" DmxShutterChannel=\"10\" DmxShutterOnValue=\"255\""));
    for (const auto& s : ok->CheckModelSettings()) {
        CHECK(s.find("Shutter On Value") == std::string::npos);
    }
}

TEST_CASE("Advanced moving head reports motors beyond the channel count", "[models2][dmx]") {
    ShowFixture f;
    Model* m = f.Add(Dmx("MH", "DmxMovingHeadAdv", "DmxChannelCount=\"8\" NodeNames=\"a,b,c,d,e,f,g,h\"", "<PanMotor ChannelCoarse=\"1\"/><TiltMotor ChannelCoarse=\"30\"/>"));
    auto res = m->CheckModelSettings();
    bool found = false;
    for (const auto& s : res) {
        if (s.find("tilt motor coarse") != std::string::npos) {
            found = true;
        }
    }
    CHECK(found);
}

TEST_CASE("Moving head PWM outputs cover pan tilt and colour channels", "[models2][dmx]") {
    ShowFixture f;
    auto* mh = As<DmxMovingHead>(f.Add(Dmx("MH", "DmxMovingHead", "DmxChannelCount=\"8\"")));
    std::map<uint32_t, PWMOutput> out;
    mh->GetPWMOutputs(out);
    CHECK(out.count(1) == 1);
    CHECK(out.count(3) == 1);
    CHECK(out.count(2) == 1);
    CHECK(out.size() >= 3);
    for (const auto& kv : out) {
        CHECK(kv.first >= 1);
        CHECK(kv.first <= 8);
    }
}

TEST_CASE("Floodlight PWM outputs follow the colour channels", "[models2][dmx]") {
    ShowFixture f;
    auto* fl = As<DmxFloodlight>(f.Add(Dmx("F", "DmxFloodlight", "DmxChannelCount=\"4\" DmxRedChannel=\"2\" DmxGreenChannel=\"3\" DmxBlueChannel=\"4\"")));
    std::map<uint32_t, PWMOutput> out;
    fl->GetPWMOutputs(out);
    CHECK(out.count(2) == 1);
    CHECK(out.count(3) == 1);
    CHECK(out.count(4) == 1);
    CHECK(out.count(1) == 0);
}

TEST_CASE("Skull flags and servos load from attributes", "[models2][dmx]") {
    ShowFixture f;
    auto* sk = As<DmxSkull>(f.Add(Dmx("S", "DmxSkull", "DmxChannelCount=\"26\" Bits16=\"0\" HasJaw=\"1\" HasPan=\"0\" HasTilt=\"1\" HasNod=\"0\" HasEyeUD=\"1\" HasEyeLR=\"0\" HasColor=\"0\" DmxJawOrient=\"11\" DmxPanOrient=\"22\" DmxEyeBrtChannel=\"20\"",
        "<JawServo Channel=\"7\" MinLimit=\"100\" MaxLimit=\"900\"/><TiltServo Channel=\"9\"/>")));
    CHECK_FALSE(sk->Is16Bit());
    CHECK(sk->HasJaw());
    CHECK_FALSE(sk->HasPan());
    CHECK(sk->HasTilt());
    CHECK_FALSE(sk->HasNod());
    CHECK(sk->HasEyeUD());
    CHECK_FALSE(sk->HasEyeLR());
    CHECK_FALSE(sk->HasColor());
    CHECK(sk->GetJawOrient() == 11);
    CHECK(sk->GetPanOrient() == 22);
    CHECK(sk->GetEyeBrightnessChannel() == 20);
    CHECK(sk->GetJawChannel() == 7);
    CHECK(sk->GetJawMinLimit() == 100);
    CHECK(sk->GetJawMaxLimit() == 900);
    CHECK(sk->GetTiltChannel() == 9);
    CHECK(sk->GetPanChannel() == 0);
    CHECK(sk->GetNodChannel() == 0);
}

TEST_CASE("Skull defaults when flags are absent", "[models2][dmx]") {
    ShowFixture f;
    auto* sk = As<DmxSkull>(f.Add(Dmx("S", "DmxSkull", "DmxChannelCount=\"26\"")));
    CHECK(sk->Is16Bit());
    CHECK(sk->HasJaw());
    CHECK(sk->HasPan());
    CHECK(sk->HasTilt());
    CHECK(sk->HasNod());
    CHECK(sk->HasEyeUD());
    CHECK(sk->HasEyeLR());
    CHECK(sk->HasColor());
    CHECK_FALSE(sk->IsMeshOnly());
    CHECK(sk->GetJawOrient() == sk->GetDefaultOrient(DmxSkull::JAW));
    CHECK(sk->GetPanOrient() == sk->GetDefaultOrient(DmxSkull::PAN));
    CHECK(sk->GetEyeLROrient() == sk->GetDefaultOrient(DmxSkull::EYE_LR));
    CHECK(sk->GetEyeBrightnessChannel() == 15);
}

TEST_CASE("Skull default servo channels are two apart for 16 bit", "[models2][dmx]") {
    ShowFixture f;
    auto* sk = As<DmxSkull>(f.Default("DmxSkull"));
    CHECK(sk->GetJawChannel() == 1);
    CHECK(sk->GetPanChannel() == 3);
    CHECK(sk->GetTiltChannel() == 5);
    CHECK(sk->GetNodChannel() == 7);
    CHECK(sk->GetEyeUDChannel() == 9);
    CHECK(sk->GetEyeLRChannel() == 11);
}

TEST_CASE("Skull round trip keeps servos and meshes", "[models2][dmx]") {
    ShowFixture a;
    Model* m = a.Add(Dmx("S", "DmxSkull", "DmxChannelCount=\"26\" HasNod=\"0\" DmxPanOrient=\"45\"", "<JawServo Channel=\"1\"/><PanServo Channel=\"13\" MinLimit=\"10\" MaxLimit=\"20\" RangeOfMotion=\"90\"/><TiltServo Channel=\"5\"/><NodServo Channel=\"7\"/><EyeUpDownServo Channel=\"9\"/><EyeLeftRightServo Channel=\"11\"/>"));
    std::string s1 = Serialize(m);
    ShowFixture b;
    pugi::xml_document doc;
    REQUIRE(doc.load_string(s1.c_str()));
    Model* m2 = b.mm.createAndAddModel(doc.first_child(), 100, 100);
    REQUIRE(m2 != nullptr);
    CHECK(Serialize(m2) == s1);
    auto* sk = As<DmxSkull>(m2);
    CHECK(sk->GetPanChannel() == 13);
    CHECK(sk->GetPanMinLimit() == 10);
    CHECK_FALSE(sk->HasNod());
}

TEST_CASE("Servo model creates one servo per NumServos", "[models2][dmx]") {
    ShowFixture f;
    auto* sv = As<DmxServo>(f.Add(Dmx("S", "DmxServo", "NumServos=\"3\" DmxChannelCount=\"6\" Bits16=\"1\"")));
    CHECK(sv->GetNumServos() == 3);
    CHECK(sv->Is16Bit());
    REQUIRE(sv->GetServo(0) != nullptr);
    REQUIRE(sv->GetServo(2) != nullptr);
    CHECK(sv->GetServo(0)->GetChannel() == 1);
    CHECK(sv->GetServo(1)->GetChannel() == 3);
    CHECK(sv->GetServo(2)->GetChannel() == 5);
    REQUIRE(sv->GetStaticImage(2) != nullptr);
    REQUIRE(sv->GetMotionImage(2) != nullptr);
}

TEST_CASE("Servo model 8 bit servos are one channel apart", "[models2][dmx]") {
    ShowFixture f;
    auto* sv = As<DmxServo>(f.Add(Dmx("S", "DmxServo", "NumServos=\"4\" DmxChannelCount=\"4\" Bits16=\"0\"")));
    CHECK_FALSE(sv->Is16Bit());
    CHECK(sv->GetServo(0)->GetChannel() == 1);
    CHECK(sv->GetServo(3)->GetChannel() == 4);
    CHECK(sv->GetNodeCount() == 4);
}

TEST_CASE("Servo model raises a too small channel count", "[models2][dmx]") {
    ShowFixture f;
    auto* sv = As<DmxServo>(f.Add(Dmx("S", "DmxServo", "NumServos=\"3\" DmxChannelCount=\"2\" Bits16=\"1\"")));
    CHECK(sv->GetDmxChannelCount() == 6);
    CHECK(sv->GetNodeCount() == 6);
}

TEST_CASE("Servo model round trips servos and images", "[models2][dmx]") {
    ShowFixture a;
    Model* m = a.Add(Dmx("S", "DmxServo", "NumServos=\"1\" DmxChannelCount=\"2\" Bits16=\"1\" Brightness=\"60\" Transparency=\"20\"",
        "<Servo1 Channel=\"1\" MinLimit=\"5\" MaxLimit=\"500\" RangeOfMotion=\"90\" ServoStyle=\"Rotate Z\" ControllerMin=\"900\" ControllerMax=\"2100\" ControllerReverse=\"1\" ControllerZeroBehavior=\"Hold\"/>"
        "<StaticImage1 Image=\"\" ScaleX=\"2\" RotateZ=\"30\"/>"));
    std::string s1 = Serialize(m);
    ShowFixture b;
    pugi::xml_document doc;
    REQUIRE(doc.load_string(s1.c_str()));
    Model* m2 = b.mm.createAndAddModel(doc.first_child(), 100, 100);
    REQUIRE(m2 != nullptr);
    CHECK(Serialize(m2) == s1);
    auto* sv = As<DmxServo>(m2);
    CHECK(sv->GetServo(0)->GetMinLimit() == 5);
    CHECK(sv->GetServo(0)->GetMaxLimit() == 500);
    CHECK(sv->GetServo(0)->GetControllerMin() == 900);
    CHECK(sv->GetServo(0)->GetControllerReverse());
    CHECK(sv->GetBrightness() == Catch::Approx(60.0f));
    CHECK(sv->GetTransparency() == 20);
}

TEST_CASE("Servo model ignores servo elements beyond NumServos", "[models2][dmx]") {
    ShowFixture f;
    auto* sv = As<DmxServo>(f.Add(Dmx("S", "DmxServo", "NumServos=\"1\" DmxChannelCount=\"2\"", "<Servo1 Channel=\"2\"/><Servo2 Channel=\"9\"/><Servo7 Channel=\"11\"/>")));
    CHECK(sv->GetNumServos() == 1);
    CHECK(sv->GetServo(0)->GetChannel() == 2);
    CHECK(sv->GetServo(5)->GetChannel() == 2);
}

TEST_CASE("Servo3d tracks servos static and motion meshes", "[models2][dmx]") {
    ShowFixture f;
    auto* sv = As<DmxServo3d>(f.Add(Dmx("S", "DmxServo3d", "NumServos=\"2\" NumStatic=\"3\" NumMotion=\"2\" DmxChannelCount=\"4\" Bits16=\"1\" Servo1Linkage=\"Mesh 2\" Servo2Linkage=\"Mesh 1\" Mesh1Linkage=\"Mesh 1\" Mesh2Linkage=\"Mesh 1\"")));
    CHECK(sv->GetNumServos() == 2);
    CHECK(sv->GetNumStatic() == 3);
    CHECK(sv->GetNumMotion() == 2);
    CHECK(sv->GetServoLink(0) == 1);
    CHECK(sv->GetServoLink(1) == 0);
    CHECK(sv->GetMeshLink(0) == -1);
    CHECK(sv->GetMeshLink(1) == 0);
    REQUIRE(sv->GetStaticMesh(2) != nullptr);
    REQUIRE(sv->GetMotionMesh(1) != nullptr);
}

TEST_CASE("Servo3d legacy 3 axis type maps to Servo3d", "[models2][dmx]") {
    ShowFixture f;
    Model* m = f.Add(Dmx("S", "DmxServo3Axis", "DmxChannelCount=\"4\""));
    REQUIRE(m != nullptr);
    CHECK(m->GetDisplayAsString() == "DmxServo3d");
    CHECK(dynamic_cast<DmxServo3d*>(m) != nullptr);
}

TEST_CASE("Servo3d round trips", "[models2][dmx]") {
    ShowFixture a;
    Model* m = a.Add(Dmx("S", "DmxServo3d", "NumServos=\"2\" NumStatic=\"2\" NumMotion=\"1\" DmxChannelCount=\"4\" Bits16=\"1\" Servo1Linkage=\"Mesh 2\" Servo2Linkage=\"Mesh 1\" Mesh1Linkage=\"Mesh 2\" Mesh2Linkage=\"Mesh 1\""));
    std::string s1 = Serialize(m);
    ShowFixture b;
    pugi::xml_document doc;
    REQUIRE(doc.load_string(s1.c_str()));
    Model* m2 = b.mm.createAndAddModel(doc.first_child(), 100, 100);
    REQUIRE(m2 != nullptr);
    CHECK(Serialize(m2) == s1);
}

TEST_CASE("Every DMX model type reaches a stable serialized form", "[models2][dmx]") {
    for (const char* t : kDmxTypes) {
        INFO(t);
        ShowFixture a;
        Model* m = a.Default(t);
        std::string s0 = Serialize(m);
        ShowFixture b;
        pugi::xml_document doc;
        REQUIRE(doc.load_string(s0.c_str()));
        Model* m1 = b.mm.createAndAddModel(doc.first_child(), 100, 100);
        REQUIRE(m1 != nullptr);
        CHECK(m1->GetDisplayAsString() == m->GetDisplayAsString());
        CHECK(m1->GetNodeCount() == m->GetNodeCount());
        CHECK(m1->GetChanCount() == m->GetChanCount());
        std::string s1 = Serialize(m1);
        ShowFixture c;
        pugi::xml_document doc2;
        REQUIRE(doc2.load_string(s1.c_str()));
        Model* m2 = c.mm.createAndAddModel(doc2.first_child(), 100, 100);
        REQUIRE(m2 != nullptr);
        CHECK(Serialize(m2) == s1);
    }
}

TEST_CASE("DMX models keep their screen location through a round trip", "[models2][dmx]") {
    ShowFixture a;
    Model* m = a.Add(Dmx("F", "DmxFloodlight", "DmxChannelCount=\"3\" WorldPosX=\"10\" WorldPosY=\"20\" WorldPosZ=\"30\" ScaleX=\"2\" ScaleY=\"3\" ScaleZ=\"4\" RotateX=\"5\" RotateY=\"6\" RotateZ=\"7\""));
    CHECK(m->GetHcenterPos() == Catch::Approx(10.0f));
    CHECK(m->GetVcenterPos() == Catch::Approx(20.0f));
    CHECK(m->GetDcenterPos() == Catch::Approx(30.0f));
    std::string s1 = Serialize(m);
    CHECK(s1.find("RotateZ=\"7.000000\"") != std::string::npos);
    CHECK(s1.find("ScaleY=\"3.000000\"") != std::string::npos);
}

TEST_CASE("DMX model with a missing StringType keeps the DMX channel count", "[models2][dmx]") {
    ShowFixture f;
    Model* m = f.Add("<model name=\"D\" DisplayAs=\"DmxFloodlight\" StartChannel=\"1\" DmxChannelCount=\"10\"/>");
    CHECK(m->GetNodeCount() == 10);
    CHECK(m->GetChanCount() == 10);
}

TEST_CASE("Legacy servo MotionImage element stays separate from StaticImage", "[models2][dmx]") {
    ShowFixture f;
    auto* sv = As<DmxServo>(f.Add(Dmx("S", "DmxServo", "NumServos=\"1\" DmxChannelCount=\"2\"", "<StaticImage Image=\"a.png\"/><MotionImage Image=\"b.png\"/>")));
    REQUIRE(sv->GetStaticImage(0) != nullptr);
    REQUIRE(sv->GetMotionImage(0) != nullptr);
    CHECK(sv->GetStaticImage(0)->GetImageFile().find("a.png") != std::string::npos);
    CHECK(sv->GetMotionImage(0)->GetImageFile().find("b.png") != std::string::npos);
}

TEST_CASE("Skull node names with disabled servos do not crash", "[models2][dmx]") {
    ShowFixture f;
    auto* sk = As<DmxSkull>(f.Add(Dmx("S", "DmxSkull", "DmxChannelCount=\"26\" HasJaw=\"0\"")));
    auto names = sk->GenerateNodeNames();
    CHECK(names.size() == 26);
}

TEST_CASE("Advanced moving head with fewer channels than its motors does not crash", "[models2][dmx]") {
    ShowFixture f;
    Model* m = f.Add(Dmx("MH", "DmxMovingHeadAdv", "DmxChannelCount=\"2\""));
    CHECK(m != nullptr);
}

TEST_CASE("Serializing a skull with a disabled servo that has no element does not crash", "[models2][dmx]") {
    ShowFixture f;
    Model* m = f.Add(Dmx("S", "DmxSkull", "DmxChannelCount=\"26\" HasNod=\"0\""));
    CHECK(!Serialize(m).empty());
}

TEST_CASE("Servo image offsets survive a save and load unchanged", "[models2][dmx]") {
    ShowFixture a;
    Model* m = a.Add(Dmx("S", "DmxServo", "NumServos=\"1\" DmxChannelCount=\"2\"", "<StaticImage1 Image=\"\" OffsetX=\"50\" OffsetY=\"20\" OffsetZ=\"10\"/>"));
    pugi::xml_document doc;
    REQUIRE(doc.load_string(Serialize(m).c_str()));
    pugi::xml_node img = doc.first_child().child("StaticImage1");
    REQUIRE(img);
    CHECK(img.attribute("OffsetX").as_float() == Catch::Approx(50.0f));
    CHECK(img.attribute("OffsetY").as_float() == Catch::Approx(20.0f));
}

TEST_CASE("Unlinked servos stay unlinked after a save and load", "[models2][dmx]") {
    ShowFixture a;
    Model* m = a.Add(Dmx("S", "DmxServo3d", "NumServos=\"2\" NumStatic=\"2\" NumMotion=\"2\" DmxChannelCount=\"4\" Bits16=\"1\""));
    auto* sv = As<DmxServo3d>(m);
    REQUIRE(sv->GetServoLink(1) == -1);
    ShowFixture b;
    pugi::xml_document doc;
    REQUIRE(doc.load_string(Serialize(m).c_str()));
    auto* sv2 = As<DmxServo3d>(b.mm.createAndAddModel(doc.first_child(), 100, 100));
    CHECK(sv2->GetServoLink(1) == -1);
    CHECK(sv2->GetMeshLink(1) == -1);
}
