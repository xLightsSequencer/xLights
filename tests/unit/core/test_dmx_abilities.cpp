#include <cmath>
#include <map>
#include <string>
#include <vector>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "models/DMX/DmxBeamAbility.h"
#include "models/DMX/DmxColorAbilityCMY.h"
#include "models/DMX/DmxColorAbilityRGB.h"
#include "models/DMX/DmxColorAbilityWheel.h"
#include "models/DMX/DmxDimmerAbility.h"
#include "models/DMX/DmxMotor.h"
#include "models/DMX/DmxPresetAbility.h"
#include "models/DMX/DmxShutterAbility.h"
#include "models/DMX/DmxMovingHeadComm.h"
#include "utils/Color.h"

namespace {

xlColorVector Blank(size_t n) {
    return xlColorVector(n, xlBLACK);
}

std::vector<std::string> Names(size_t n) {
    return std::vector<std::string>(n, std::string());
}

} // namespace

TEST_CASE("RGBW ability defaults to no channels", "[models2][dmx]") {
    DmxColorAbilityRGB a;
    CHECK(a.GetColorType() == DmxColorAbility::DMX_COLOR_TYPE::DMX_COLOR_RGBW);
    CHECK(a.GetTypeName() == "RGBW");
    CHECK(a.GetNumChannels() == 0);
    CHECK(a.GetRedBrightness() == 100);
    CHECK(a.GetWhiteBrightness() == 100);
    CHECK(a.GetRedGamma() == Catch::Approx(1.0f));
    CHECK_FALSE(a.IsColorChannel(1));
}

TEST_CASE("RGBW ability counts only the assigned channels", "[models2][dmx]") {
    DmxColorAbilityRGB a;
    a.SetRedChannel(1);
    CHECK(a.GetNumChannels() == 1);
    a.SetBlueChannel(3);
    CHECK(a.GetNumChannels() == 2);
    a.SetGreenChannel(2);
    a.SetWhiteChannel(4);
    CHECK(a.GetNumChannels() == 4);
    for (uint32_t c = 1; c <= 4; ++c) {
        CHECK(a.IsColorChannel(c));
    }
    CHECK_FALSE(a.IsColorChannel(5));
}

TEST_CASE("RGBW SetColorPixels writes one channel per component", "[models2][dmx]") {
    DmxColorAbilityRGB a;
    a.SetRedChannel(2);
    a.SetGreenChannel(4);
    a.SetBlueChannel(6);
    auto px = Blank(6);
    a.SetColorPixels(xlColor(10, 20, 30), px);
    CHECK(px[1].red == 10);
    CHECK(px[3].red == 20);
    CHECK(px[5].red == 30);
    CHECK(px[3].green == 20);
    CHECK(px[3].blue == 20);
    CHECK(px[0] == xlBLACK);
    CHECK(px[2] == xlBLACK);
    CHECK(px[4] == xlBLACK);
}

TEST_CASE("RGBW grey colours go to the white channel when there is one", "[models2][dmx]") {
    DmxColorAbilityRGB a;
    a.SetRedChannel(1);
    a.SetGreenChannel(2);
    a.SetBlueChannel(3);
    a.SetWhiteChannel(4);
    auto px = Blank(4);
    a.SetColorPixels(xlColor(90, 90, 90), px);
    CHECK(px[3].red == 90);
    CHECK(px[0] == xlBLACK);
    CHECK(px[1] == xlBLACK);
    CHECK(px[2] == xlBLACK);

    a.SetColorPixels(xlColor(90, 80, 90), px);
    CHECK(px[0].red == 90);
    CHECK(px[1].red == 80);
    CHECK(px[2].red == 90);
}

TEST_CASE("RGBW SetColorPixels ignores channels beyond the pixel vector", "[models2][dmx]") {
    DmxColorAbilityRGB a;
    a.SetRedChannel(1);
    a.SetGreenChannel(9);
    a.SetBlueChannel(0);
    auto px = Blank(3);
    a.SetColorPixels(xlColor(7, 8, 9), px);
    CHECK(px[0].red == 7);
    CHECK(px[1] == xlBLACK);
    CHECK(px[2] == xlBLACK);
    xlColorVector empty;
    a.SetColorPixels(xlColor(7, 8, 9), empty);
    CHECK(empty.empty());
}

TEST_CASE("RGBW colour readback recombines the channels", "[models2][dmx]") {
    DmxColorAbilityRGB a;
    a.SetRedChannel(1);
    a.SetGreenChannel(2);
    a.SetBlueChannel(3);
    auto px = Blank(3);
    a.SetColorPixels(xlColor(200, 100, 50), px);
    xlColor c = a.GetColorPixels(px);
    CHECK(c.red == 200);
    CHECK(c.green == 100);
    CHECK(c.blue == 50);
}

TEST_CASE("RGBW readback prefers a lit white channel", "[models2][dmx]") {
    DmxColorAbilityRGB a;
    a.SetRedChannel(1);
    a.SetGreenChannel(2);
    a.SetBlueChannel(3);
    a.SetWhiteChannel(4);
    auto px = Blank(4);
    px[3] = xlColor(120, 120, 120);
    px[0] = xlColor(10, 10, 10);
    xlColor c = a.GetColorPixels(px);
    CHECK(c.red == 120);
    CHECK(c.green == 120);
    CHECK(c.blue == 120);
}

TEST_CASE("RGBW readback with only white uses it as grey", "[models2][dmx]") {
    DmxColorAbilityRGB a;
    a.SetWhiteChannel(2);
    auto px = Blank(2);
    px[1] = xlColor(77, 0, 0);
    xlColor c = a.GetColorPixels(px);
    CHECK(c.red == 77);
    CHECK(c.green == 77);
    CHECK(c.blue == 77);
}

TEST_CASE("RGBW readback with nothing configured is black", "[models2][dmx]") {
    DmxColorAbilityRGB a;
    auto px = Blank(4);
    px[0] = xlColor(255, 255, 255);
    CHECK(a.GetColorPixels(px) == xlBLACK);
}

TEST_CASE("RGBW node names label the channels", "[models2][dmx]") {
    DmxColorAbilityRGB a;
    a.SetRedChannel(1);
    a.SetGreenChannel(2);
    a.SetBlueChannel(3);
    a.SetWhiteChannel(4);
    auto n = Names(5);
    a.SetNodeNames(n, "Lamp-");
    CHECK(n[0] == "Lamp-Red");
    CHECK(n[1] == "Lamp-Green");
    CHECK(n[2] == "Lamp-Blue");
    CHECK(n[3] == "Lamp-White");
    CHECK(n[4].empty());
}

TEST_CASE("RGBW node names skip channels past the end", "[models2][dmx]") {
    DmxColorAbilityRGB a;
    a.SetRedChannel(1);
    a.SetGreenChannel(10);
    auto n = Names(2);
    a.SetNodeNames(n);
    CHECK(n[0] == "Red");
    CHECK(n[1].empty());
}

TEST_CASE("RGBW PWM outputs are keyed by channel", "[models2][dmx]") {
    DmxColorAbilityRGB a;
    a.SetRedChannel(3);
    a.SetBlueChannel(5);
    std::map<uint32_t, PWMOutput> out;
    a.GetPWMOutputs(out);
    CHECK(out.size() == 2);
    CHECK(out.count(3) == 1);
    CHECK(out.count(5) == 1);
    CHECK(out.count(4) == 0);
}

TEST_CASE("RGBW channel transparency maps channel to its colour", "[models2][dmx]") {
    DmxColorAbilityRGB a;
    a.SetRedChannel(1);
    a.SetGreenChannel(2);
    a.SetBlueChannel(3);
    a.SetWhiteChannel(4);
    xlColor c;
    CHECK(a.ApplyChannelTransparency(c, 0, 1));
    CHECK(c.red == 255);
    CHECK(c.green == 0);
    CHECK(a.ApplyChannelTransparency(c, 0, 3));
    CHECK(c.blue == 255);
    CHECK(c.red == 0);
    CHECK_FALSE(a.ApplyChannelTransparency(c, 0, 9));
}

TEST_CASE("CMYW ability basics", "[models2][dmx]") {
    DmxColorAbilityCMY a;
    CHECK(a.GetColorType() == DmxColorAbility::DMX_COLOR_TYPE::DMX_COLOR_CMYW);
    CHECK(a.GetTypeName() == "CMYW");
    CHECK(a.GetNumChannels() == 0);
    a.SetCyanChannel(1);
    a.SetMagentaChannel(2);
    a.SetYellowChannel(3);
    CHECK(a.GetNumChannels() == 3);
    a.SetWhiteChannel(4);
    CHECK(a.GetNumChannels() == 4);
    CHECK(a.IsColorChannel(2));
    CHECK_FALSE(a.IsColorChannel(5));
}

TEST_CASE("CMY conversions for primary colours", "[models2][dmx]") {
    DmxColorAbilityCMY a;
    CHECK(a.GetC(xlColor(255, 0, 0)) == 0);
    CHECK(a.GetM(xlColor(255, 0, 0)) == 255);
    CHECK(a.GetY(xlColor(255, 0, 0)) == 255);
    CHECK(a.GetC(xlColor(0, 255, 0)) == 255);
    CHECK(a.GetM(xlColor(0, 255, 0)) == 0);
    CHECK(a.GetY(xlColor(0, 255, 0)) == 255);
    CHECK(a.GetC(xlColor(0, 0, 255)) == 255);
    CHECK(a.GetY(xlColor(0, 0, 255)) == 0);
}

TEST_CASE("CMY of a grey is the grey level", "[models2][dmx]") {
    DmxColorAbilityCMY a;
    xlColor g(100, 100, 100);
    CHECK(a.GetC(g) == 100);
    CHECK(a.GetM(g) == 100);
    CHECK(a.GetY(g) == 100);
    CHECK(a.GetK(xlColor(255, 255, 255)) == Catch::Approx(0.0f).margin(1e-5));
    CHECK(a.GetK(xlColor(0, 0, 0)) == Catch::Approx(1.0f).margin(1e-5));
}

TEST_CASE("CMY to RGB inverts the channel values", "[models2][dmx]") {
    DmxColorAbilityCMY a;
    xlColor c = a.GetRGB(0, 255, 255);
    CHECK(c.red == 255);
    CHECK(c.green == 0);
    CHECK(c.blue == 0);
    xlColor g = a.GetRGB(40, 40, 40);
    CHECK(g.red == 40);
    CHECK(g.green == 40);
    CHECK(g.blue == 40);
}

TEST_CASE("CMY SetColorPixels then GetColorPixels recovers the colour", "[models2][dmx]") {
    DmxColorAbilityCMY a;
    a.SetCyanChannel(1);
    a.SetMagentaChannel(2);
    a.SetYellowChannel(3);
    auto px = Blank(3);
    a.SetColorPixels(xlColor(255, 0, 0), px);
    CHECK(px[0].red == 0);
    CHECK(px[1].red == 255);
    CHECK(px[2].red == 255);
    xlColor back = a.GetColorPixels(px);
    CHECK(back.red == 255);
    CHECK(back.green == 0);
    CHECK(back.blue == 0);
}

TEST_CASE("CMY grey goes to white channel", "[models2][dmx]") {
    DmxColorAbilityCMY a;
    a.SetCyanChannel(1);
    a.SetMagentaChannel(2);
    a.SetYellowChannel(3);
    a.SetWhiteChannel(4);
    auto px = Blank(4);
    a.SetColorPixels(xlColor(60, 60, 60), px);
    CHECK(px[3].red == 60);
    CHECK(px[0] == xlBLACK);
    xlColor back = a.GetColorPixels(px);
    CHECK(back.red == 60);
    CHECK(back.green == 60);
    CHECK(back.blue == 60);
}

TEST_CASE("CMY node names", "[models2][dmx]") {
    DmxColorAbilityCMY a;
    a.SetCyanChannel(1);
    a.SetMagentaChannel(2);
    a.SetYellowChannel(3);
    a.SetWhiteChannel(4);
    auto n = Names(4);
    a.SetNodeNames(n, "X-");
    CHECK(n[0] == "X-Cyan");
    CHECK(n[1] == "X-Magenta");
    CHECK(n[2] == "X-Yellow");
    CHECK(n[3] == "X-White");
}

TEST_CASE("CMY transparency colours and PWM outputs", "[models2][dmx]") {
    DmxColorAbilityCMY a;
    a.SetCyanChannel(1);
    a.SetMagentaChannel(2);
    a.SetYellowChannel(3);
    xlColor c;
    CHECK(a.ApplyChannelTransparency(c, 0, 2));
    CHECK(c.red == 255);
    CHECK(c.green == 0);
    CHECK(c.blue == 255);
    CHECK_FALSE(a.ApplyChannelTransparency(c, 0, 7));
    std::map<uint32_t, PWMOutput> out;
    a.GetPWMOutputs(out);
    CHECK(out.size() == 3);
}

TEST_CASE("Colour wheel starts with four default colours", "[models2][dmx]") {
    DmxColorAbilityWheel w;
    CHECK(w.GetColorType() == DmxColorAbility::DMX_COLOR_TYPE::DMX_COLOR_WHEEL);
    CHECK(w.GetTypeName() == "ColorWheel");
    REQUIRE(w.GetColorWheelColorSize() == 4);
    CHECK(w.GetWheelColorSettings()[0].dmxValue == 10);
    CHECK(w.GetWheelColorSettings()[1].color == xlRED);
    CHECK(w.GetLastColorDMX() == 70);
    CHECK(w.GetNumChannels() == 0);
}

TEST_CASE("Colour wheel add pop and clear", "[models2][dmx]") {
    DmxColorAbilityWheel w;
    w.AddColor("#00FFFF", 90);
    REQUIRE(w.GetColorWheelColorSize() == 5);
    CHECK(w.GetLastColorDMX() == 90);
    CHECK(w.GetColors().back() == xlColor(0, 255, 255));
    w.PopColor();
    CHECK(w.GetColorWheelColorSize() == 4);
    w.ClearColors();
    CHECK(w.GetColorWheelColorSize() == 0);
    CHECK(w.GetLastColorDMX() == 0);
    w.PopColor();
    CHECK(w.GetColorWheelColorSize() == 0);
}

TEST_CASE("Colour wheel finds the DMX value for a colour by hue", "[models2][dmx]") {
    DmxColorAbilityWheel w;
    w.SetWheelChannel(1);
    auto px = Blank(2);
    w.SetColorPixels(xlGREEN, px);
    CHECK(px[0].red == 50);
    w.SetColorPixels(xlBLUE, px);
    CHECK(px[0].red == 70);
    w.SetColorPixels(xlRED, px);
    CHECK(px[0].red == 30);
    CHECK(w.GetDMXWheelIndex(xlGREEN) == 2);
    CHECK(w.GetDMXWheelIndex(xlBLUE) == 3);
    CHECK(w.GetDMXWheelIndex(xlColor(255, 255, 0)) == -1);
}

TEST_CASE("Colour wheel ignores colours not on the wheel", "[models2][dmx]") {
    DmxColorAbilityWheel w;
    w.SetWheelChannel(1);
    auto px = Blank(2);
    px[0] = xlColor(5, 5, 5);
    w.SetColorPixels(xlColor(255, 255, 0), px);
    CHECK(px[0].red == 5);
}

TEST_CASE("Colour wheel dimmer channel carries the brightness", "[models2][dmx]") {
    DmxColorAbilityWheel w;
    w.SetWheelChannel(1);
    w.SetDimmerChannel(2);
    auto px = Blank(2);
    w.SetColorPixels(xlColor(0, 255, 0), px);
    CHECK(px[0].red == 50);
    CHECK(px[1].red == 255);
    w.SetColorPixels(xlColor(0, 128, 0), px);
    CHECK(px[1].red == Catch::Approx(128).margin(1));
}

TEST_CASE("Colour wheel readback chooses the nearest wheel slot", "[models2][dmx]") {
    DmxColorAbilityWheel w;
    w.SetWheelChannel(1);
    auto px = Blank(1);
    px[0] = xlColor(10, 10, 10);
    CHECK(w.GetColorPixels(px) == xlWHITE);
    px[0] = xlColor(28, 28, 28);
    CHECK(w.GetColorPixels(px) == xlRED);
    px[0] = xlColor(52, 52, 52);
    CHECK(w.GetColorPixels(px) == xlGREEN);
    px[0] = xlColor(200, 200, 200);
    CHECK(w.GetColorPixels(px) == xlBLUE);
    px[0] = xlColor(0, 0, 0);
    CHECK(w.GetColorPixels(px) == xlWHITE);
}

TEST_CASE("Colour wheel with a zero slot takes the colour at or below the value", "[models2][dmx]") {
    DmxColorAbilityWheel w;
    w.ClearColors();
    w.AddWheelColor(xlWHITE, 0);
    w.AddWheelColor(xlRED, 50);
    w.AddWheelColor(xlBLUE, 100);
    w.SetWheelChannel(1);
    auto px = Blank(1);
    px[0] = xlColor(49, 0, 0);
    CHECK(w.GetColorPixels(px) == xlWHITE);
    px[0] = xlColor(50, 0, 0);
    CHECK(w.GetColorPixels(px) == xlRED);
    px[0] = xlColor(250, 0, 0);
    CHECK(w.GetColorPixels(px) == xlBLUE);
}

TEST_CASE("Colour wheel with a 255 slot takes the colour at or above the value", "[models2][dmx]") {
    DmxColorAbilityWheel w;
    w.ClearColors();
    w.AddWheelColor(xlRED, 100);
    w.AddWheelColor(xlBLUE, 255);
    w.SetWheelChannel(1);
    auto px = Blank(1);
    px[0] = xlColor(101, 0, 0);
    CHECK(w.GetColorPixels(px) == xlBLUE);
    px[0] = xlColor(100, 0, 0);
    CHECK(w.GetColorPixels(px) == xlRED);
}

TEST_CASE("Colour wheel with no colours reads back black", "[models2][dmx]") {
    DmxColorAbilityWheel w;
    w.ClearColors();
    w.SetWheelChannel(1);
    auto px = Blank(1);
    CHECK(w.GetColorPixels(px) == xlBLACK);
    xlColor c;
    w.SetColorPixels(xlRED, px);
    CHECK(px[0] == xlBLACK);
}

TEST_CASE("Colour wheel node names and PWM", "[models2][dmx]") {
    DmxColorAbilityWheel w;
    w.SetWheelChannel(2);
    w.SetDimmerChannel(3);
    CHECK(w.GetNumChannels() == 2);
    CHECK(w.IsColorChannel(2));
    CHECK(w.IsColorChannel(3));
    CHECK_FALSE(w.IsColorChannel(1));
    auto n = Names(3);
    w.SetNodeNames(n);
    CHECK(n[1] == "Color Wheel");
    CHECK(n[2] == "Dimmer");
    std::map<uint32_t, PWMOutput> out;
    w.GetPWMOutputs(out);
    CHECK(out.size() == 2);
}

TEST_CASE("Shutter is open when no channel is set", "[models2][dmx]") {
    DmxShutterAbility s;
    std::vector<NodeBaseClassPtr> nodes;
    CHECK(s.IsShutterOpen(nodes));
    CHECK(s.GetShutterChannel() == 0);
    CHECK(s.GetShutterThreshold() == 1);
    CHECK(s.GetShutterOnValue() == 0);
}

TEST_CASE("Shutter channel beyond the node count counts as open", "[models2][dmx]") {
    DmxShutterAbility s;
    s.SetShutterChannel(5);
    std::vector<NodeBaseClassPtr> nodes;
    CHECK(s.IsShutterOpen(nodes));
}

TEST_CASE("Preset ability stores and applies presets", "[models2][dmx]") {
    DmxPresetAbility p;
    CHECK(p.GetPresetsCount() == 0);
    p.AddPreset(2, 100, "Gobo");
    p.AddPreset(4, 0, "Zero value");
    p.AddPreset(0, 5, "Zero channel");
    p.AddPreset(9, 5, "Far");
    CHECK(p.GetPresetsCount() == 4);
    auto px = Blank(5);
    p.SetPresetValues(px);
    CHECK(px[1].red == 100);
    CHECK(px[1].blue == 100);
    CHECK(px[3] == xlBLACK);
    CHECK(px[0] == xlBLACK);
}

TEST_CASE("Preset ability edits and pops", "[models2][dmx]") {
    DmxPresetAbility p;
    p.AddPreset(1, 10, "a");
    p.AddPreset(2, 20, "b");
    p.SetPresetChannel(1, 3);
    p.SetPresetValue(1, 33);
    p.SetPresetDescription(1, "bb");
    REQUIRE(p.GetPresetSettings().size() == 2);
    CHECK(p.GetPresetSettings()[1].DMXChannel == 3);
    CHECK(p.GetPresetSettings()[1].DMXValue == 33);
    CHECK(p.GetPresetSettings()[1].Description == "bb");
    p.PopPreset();
    CHECK(p.GetPresetsCount() == 1);
    p.PopPreset();
    p.PopPreset();
    CHECK(p.GetPresetsCount() == 0);
}

TEST_CASE("Beam ability defaults and setters", "[models2][dmx]") {
    DmxBeamAbility b;
    CHECK(b.GetBeamLength() == Catch::Approx(1.0f));
    CHECK(b.GetBeamWidth() == Catch::Approx(1.0f));
    CHECK(b.GetBeamOrient() == 0);
    CHECK_FALSE(b.SupportsOrient());
    CHECK_FALSE(b.SupportsYOffset());
    b.SetBeamLength(6.5f);
    b.SetBeamYOffset(-2.0f);
    b.SetSupportsOrient(true);
    CHECK(b.GetBeamLength() == Catch::Approx(6.5f));
    CHECK(b.GetBeamYOffset() == Catch::Approx(-2.0f));
    CHECK(b.GetDefaultBeamLength() == Catch::Approx(1.0f));
    CHECK(b.SupportsOrient());
}

TEST_CASE("Dimmer ability stores its channel", "[models2][dmx]") {
    DmxDimmerAbility d;
    CHECK(d.GetDimmerChannel() == 0);
    d.SetDimmerChannel(7);
    CHECK(d.GetDimmerChannel() == 7);
}

TEST_CASE("Motor position to command at the limits and centre", "[models2][dmx]") {
    DmxMotor m("Pan");
    m.SetRangeOfMOtion(540.0f);
    m.SetMinLimit(-270);
    m.SetMaxLimit(270);
    m.Init();
    CHECK(m.ConvertPostoCmd(0.0f) == 0);
    CHECK(m.ConvertPostoCmd(270.0f) == Catch::Approx(65535.0f * 270.0f / 540.0f).margin(1));
    CHECK(m.ConvertPostoCmd(1000.0f) == m.ConvertPostoCmd(270.0f));
    int home = 0;
    m.SetOrientHome(270);
    home = m.ConvertPostoCmd(0.0f);
    CHECK(home == Catch::Approx(32767.5f).margin(1));
}

TEST_CASE("Motor reverse mirrors the command around home", "[models2][dmx]") {
    DmxMotor fwd("Tilt");
    fwd.SetRangeOfMOtion(180.0f);
    fwd.SetOrientHome(90);
    fwd.Init();
    DmxMotor rev("Tilt");
    rev.SetRangeOfMOtion(180.0f);
    rev.SetOrientHome(90);
    rev.SetReverse(true);
    rev.Init();
    int home = fwd.ConvertPostoCmd(0.0f);
    CHECK(home == rev.ConvertPostoCmd(0.0f));
    CHECK(fwd.ConvertPostoCmd(45.0f) - home == Catch::Approx(-(rev.ConvertPostoCmd(45.0f) - home)).margin(2));
}

TEST_CASE("Motor upside down inverts the position", "[models2][dmx]") {
    DmxMotor a("Pan");
    a.SetRangeOfMOtion(360.0f);
    a.SetOrientHome(180);
    a.Init();
    DmxMotor b("Pan");
    b.SetRangeOfMOtion(360.0f);
    b.SetOrientHome(180);
    b.SetUpsideDown(true);
    b.Init();
    CHECK(a.ConvertPostoCmd(90.0f) == b.ConvertPostoCmd(-90.0f));
}

TEST_CASE("Motor command stays within the 16 bit range", "[models2][dmx]") {
    DmxMotor m("Pan");
    m.SetRangeOfMOtion(180.0f);
    m.SetOrientHome(90);
    m.Init();
    for (float p = -180.0f; p <= 180.0f; p += 7.0f) {
        int cmd = m.ConvertPostoCmd(p);
        CHECK(cmd >= 0);
        CHECK(cmd <= 65535);
    }
}

TEST_CASE("Motor position from channel value spans the range of motion", "[models2][dmx]") {
    DmxMotor m("Pan");
    m.SetRangeOfMOtion(540.0f);
    m.Init();
    CHECK(m.GetPosition(0) == Catch::Approx(0.0f).margin(1e-3));
    CHECK(m.GetPosition(65535) == Catch::Approx(-540.0f).margin(1e-2));
    DmxMotor r("Pan");
    r.SetRangeOfMOtion(540.0f);
    r.SetReverse(true);
    r.Init();
    CHECK(r.GetPosition(65535) == Catch::Approx(540.0f).margin(1e-2));
}

TEST_CASE("Motor bit width follows the fine channel", "[models2][dmx]") {
    DmxMotor m("Pan");
    CHECK_FALSE(m.Is16Bit());
    m.SetChannelCoarse(1);
    CHECK_FALSE(m.Is16Bit());
    m.SetChannelFine(2);
    CHECK(m.Is16Bit());
    CHECK(m.GetName() == "Pan");
    CHECK(m.GetMinValue() == 0);
    CHECK(m.GetMaxValue() == 65535);
}

TEST_CASE("Moving head fixture ids map to names and back", "[models2][dmx]") {
    for (int i = 0; i < 8; ++i) {
        std::string name = DmxMovingHeadComm::FixtureIDtoString(i);
        CHECK(name == "MH" + std::to_string(i + 1));
        CHECK(DmxMovingHeadComm::FixtureStringtoID(name) == i);
    }
    CHECK(DmxMovingHeadComm::FixtureIDtoString(99) == "MH1");
    CHECK(DmxMovingHeadComm::FixtureStringtoID("bogus") == 0);
    CHECK(DmxMovingHeadComm::FixtureStringtoID("") == 0);
}
