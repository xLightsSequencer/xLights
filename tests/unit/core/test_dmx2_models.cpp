// DMX model behaviour outside the 3D drawing: floodlight and 2D servo
// previews, colour abilities reading channel values, position zones,
// settings checks, node naming, fixed channels and PWM outputs.

#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <glm/glm.hpp>

#include <pugixml.hpp>

#include "XmlSerializer/XmlSerializer.h"
#include "models/DMX/DmxBeamAbility.h"
#include "models/DMX/DmxColorAbilityCMY.h"
#include "models/DMX/DmxColorAbilityRGB.h"
#include "models/DMX/DmxColorAbilityWheel.h"
#include "models/DMX/DmxFloodArea.h"
#include "models/DMX/DmxFloodlight.h"
#include "models/DMX/DmxGeneral.h"
#include "models/DMX/DmxImage.h"
#include "models/DMX/DmxMotor.h"
#include "models/DMX/DmxMovingHead.h"
#include "models/DMX/DmxMovingHeadAdv.h"
#include "models/DMX/DmxPresetAbility.h"
#include "models/DMX/DmxServo.h"
#include "models/DMX/DmxServo3D.h"
#include "models/DMX/DmxShutterAbility.h"
#include "models/DMX/DmxSkull.h"
#include "models/DMX/Servo.h"
#include "models/PWMOutput.h"
#include "utils/xlImage.h"

#include "support/TempDir.h"
#include "test_dmx2_support.h"

using Catch::Approx;
using namespace dmx2;

namespace {

bool Contains(const std::list<std::string>& msgs, const std::string& needle) {
    for (auto& m : msgs) {
        if (m.find(needle) != std::string::npos) {
            return true;
        }
    }
    return false;
}

// The model's nodes with their current channel values, as the colour
// abilities read them.
std::vector<NodeBaseClassPtr> NodesOf(Model* m) {
    std::vector<NodeBaseClassPtr> nodes;
    int w = 0, h = 0;
    m->InitRenderBufferNodes("Default", "2D", "None", nodes, w, h, 0);
    return nodes;
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

// --- Floodlight / flood area -------------------------------------------------

TEST_CASE("Floodlight effect preview draws a disc in the channel colour", "[dmx2]") {
    Show s;
    auto* fl = As<DmxFloodlight>(s.Add(Dmx("DmxFloodlight", "DmxChannelCount=\"3\" DmxRedChannel=\"1\" DmxGreenChannel=\"2\" DmxBlueChannel=\"3\"")));
    Chan(fl, 1, 10);
    Chan(fl, 2, 20);
    Chan(fl, 3, 30);
    FakePreview p;
    DrawEffect(fl, p);
    auto& v = p.TransAcc().v;
    // a 16 segment fan around the centre
    REQUIRE(v.size() == 16 * 3);
    CHECK(v[2].x == 0.0f);
    CHECK(v[2].c == xlColor(10, 20, 30));
    CHECK(std::hypot(v[0].x, v[0].y) == Approx(0.5f));
    // the rim fades out for the default smooth pixel style
    CHECK(v[0].c.red == 10);
    CHECK(v[0].c.alpha == 0);
    REQUIRE(p.ctx.triangles.size() == 1);
    CHECK(p.ctx.triangles[0].count == 48);
}

TEST_CASE("Floodlight solid circle style keeps the rim opaque", "[dmx2]") {
    Show s;
    auto* fl = As<DmxFloodlight>(s.Add(Dmx("DmxFloodlight", "DmxChannelCount=\"3\" Antialias=\"2\"")));
    REQUIRE(fl->GetPixelStyle() == Model::PIXEL_STYLE::PIXEL_STYLE_SOLID_CIRCLE);
    Chan(fl, 1, 200);
    FakePreview p;
    DrawEffect(fl, p);
    REQUIRE_FALSE(p.TransAcc().v.empty());
    CHECK(p.TransAcc().v[0].c.alpha == 255);
}

TEST_CASE("Floodlight with its shutter closed draws nothing in the effect preview", "[dmx2]") {
    Show s;
    auto* fl = As<DmxFloodlight>(s.Add(Dmx("DmxFloodlight", "DmxChannelCount=\"4\" DmxShutterChannel=\"4\" DmxShutterOpen=\"100\"")));
    Chan(fl, 1, 255);
    Chan(fl, 4, 99);
    FakePreview closed;
    DrawEffect(fl, closed);
    CHECK(closed.TransAcc().v.empty());
    Chan(fl, 4, 100);
    FakePreview open;
    DrawEffect(fl, open);
    CHECK(open.TransAcc().v.size() == 48);
    // the layout ignores the shutter
    Chan(fl, 4, 0);
    FakePreview layout;
    DrawLayout(fl, layout);
    CHECK(layout.TransAcc().v.size() == 48);
    CHECK(layout.TransAcc().v[2].c == xlWHITE);
}

TEST_CASE("Floodlight 3D layout stretches the beam by its length", "[dmx2]") {
    float z[2];
    int i = 0;
    for (const char* len : { "1", "2" }) {
        Show s;
        auto* fl = As<DmxFloodlight>(s.Add(Dmx("DmxFloodlight", std::string("DmxChannelCount=\"3\" DmxBeamLength=\"") + len + "\"")));
        FakePreview p;
        float bbox[6] = { 0 };
        DrawLayout(fl, p, nullptr, true, bbox);
        REQUIRE(p.TransAcc().v.size() == 48);
        CHECK(p.TransAcc().v[2].z == 0.0f);
        z[i++] = p.TransAcc().v[0].z;
        CHECK(bbox[2] == -0.5f);
    }
    CHECK(z[0] > 0.0f);
    CHECK(z[1] == Approx(2.0f * z[0]));
    // 2D layouts are flat
    Show s;
    auto* fl = As<DmxFloodlight>(s.Add(Dmx("DmxFloodlight", "DmxChannelCount=\"3\" DmxBeamLength=\"2\"")));
    FakePreview p;
    DrawLayout(fl, p);
    CHECK(p.TransAcc().v[0].z == 0.0f);
}

TEST_CASE("Floodlight with a colour channel past the end draws the invalid cross", "[dmx2]") {
    Show s;
    auto* fl = As<DmxFloodlight>(s.Add(Dmx("DmxFloodlight", "DmxChannelCount=\"3\" DmxBlueChannel=\"9\"")));
    FakePreview p;
    DrawLayout(fl, p);
    CHECK(IsInvalidCross(p.SolidAcc()));
    CHECK(p.TransAcc().v.empty());
    // the cross carries its own transform
    CHECK(p.ctx.pushes == 1);
    CHECK(p.ctx.pops == 1);
}

TEST_CASE("Selected floodlight draws handles", "[dmx2]") {
    Show s;
    auto* fl = As<DmxFloodlight>(s.Add(Dmx("DmxFloodlight", "DmxChannelCount=\"3\"")));
    for (bool is3d : { false, true }) {
        FakePreview plain;
        DrawLayout(fl, plain, &xlWHITE, is3d);
        fl->Selected(true);
        FakePreview sel;
        DrawLayout(fl, sel, &xlWHITE, is3d);
        fl->Selected(false);
        CHECK(sel.TransAcc().v.size() > plain.TransAcc().v.size());
        // the passed colour is used for the disc
        CHECK(plain.TransAcc().v[2].c == xlWHITE);
    }
}

TEST_CASE("Flood area draws a four sided pyramid", "[dmx2]") {
    Show s;
    auto* fa = As<DmxFloodArea>(s.Add(Dmx("DmxFloodArea", "DmxChannelCount=\"3\" DmxBeamLength=\"2\"")));
    Chan(fa, 1, 255);
    Chan(fa, 2, 128);
    FakePreview p;
    DrawEffect(fa, p);
    auto& v = p.TransAcc().v;
    REQUIRE(v.size() == 12);
    for (size_t i = 0; i < 12; i += 3) {
        CHECK(v[i].x == 0.0f);
        CHECK(v[i].y == 0.0f);
        CHECK(v[i].c == xlColor(255, 128, 0));
        CHECK(std::abs(v[i + 1].x) == 0.5f);
        CHECK(std::abs(v[i + 1].y) == 0.5f);
        CHECK(v[i + 1].c.alpha == 0);
        // flat in the effect preview
        CHECK(v[i + 1].z == 0.0f);
    }
    FakePreview p3;
    DrawLayout(fa, p3, nullptr, true);
    REQUIRE(p3.TransAcc().v.size() == 12);
    CHECK(p3.TransAcc().v[1].z > 0.0f);
}

TEST_CASE("Floodlight fixed channels hold the shutter open", "[dmx2]") {
    Show s;
    auto* fl = As<DmxFloodlight>(s.Add(Dmx("DmxFloodlight", "DmxChannelCount=\"5\" DmxShutterChannel=\"5\" DmxShutterOnValue=\"200\" DmxPresetChannel0=\"4\" DmxPresetValue0=\"33\"")));
    xlColorVector px(5, xlBLACK);
    fl->EnableFixedChannels(px);
    CHECK(px[4] == xlColor(200, 200, 200));
    CHECK(px[3].red == 33);
    CHECK(px[0] == xlBLACK);

    // no on value: nothing forced
    Show s2;
    auto* fl2 = As<DmxFloodlight>(s2.Add(Dmx("DmxFloodlight", "DmxChannelCount=\"5\" DmxShutterChannel=\"5\"")));
    xlColorVector px2(5, xlBLACK);
    fl2->EnableFixedChannels(px2);
    CHECK(px2[4] == xlBLACK);
}

TEST_CASE("Floodlight names and PWM outputs include the shutter", "[dmx2]") {
    Show s;
    auto* fl = As<DmxFloodlight>(s.Add(Dmx("DmxFloodlight", "DmxChannelCount=\"5\" DmxShutterChannel=\"4\" DmxRedChannel=\"1\" DmxGreenChannel=\"2\" DmxBlueChannel=\"3\"")));
    auto names = fl->GenerateNodeNames();
    REQUIRE(names.size() == 5);
    CHECK(names[3] == "Shutter");
    std::map<uint32_t, PWMOutput> out;
    fl->GetPWMOutputs(out);
    REQUIRE(out.count(4) == 1);
    CHECK(out[4].label == "Shutter");
    CHECK(out[4].type == PWMOutput::Type::LED);
    CHECK(out[4].channels == 1);
}

TEST_CASE("DMX PWM output list is placed at the model start and labelled by node name", "[dmx2]") {
    Show s;
    auto* fl = As<DmxFloodlight>(s.Add("<model name=\"F\" DisplayAs=\"DmxFloodlight\" StartChannel=\"41\" StringType=\"Single Color White\" DmxChannelCount=\"4\" "
                                       "DmxRedChannel=\"1\" DmxGreenChannel=\"2\" DmxBlueChannel=\"3\" DmxShutterChannel=\"4\" NodeNames=\"Rd,,Bl\"/>"));
    REQUIRE(fl->GetFirstChannel() == 40);
    auto outs = static_cast<const Model*>(fl)->GetPWMOutputs();
    REQUIRE(outs.size() == 4);
    // like Model::GetPWMOutputs, start channels are 0 based absolute channels
    for (size_t i = 0; i < outs.size(); ++i) {
        CHECK(outs[i].startChannel == 40 + i);
    }
    CHECK(outs[0].label == "Rd");
    CHECK_FALSE(outs[1].label.empty()); // no node name: the ability's own label
    CHECK(outs[2].label == "Bl");
    CHECK(outs[3].label == "Shutter");
}

// --- Colour abilities on live channel values -----------------------------------

TEST_CASE("CMY colour reads cyan magenta and yellow channels as RGB", "[dmx2]") {
    Show s;
    auto* mh = As<DmxMovingHead>(s.Add(Dmx("DmxMovingHead", "DmxChannelCount=\"8\" DmxColorType=\"2\" DmxCyanChannel=\"4\" DmxMagentaChannel=\"5\" DmxYellowChannel=\"6\" DmxWhiteChannel=\"7\"")));
    auto* cmy = dynamic_cast<DmxColorAbilityCMY*>(mh->GetColorAbility());
    REQUIRE(cmy != nullptr);
    Chan(mh, 4, 255);
    Chan(mh, 5, 0);
    Chan(mh, 6, 51);
    auto nodes = NodesOf(mh);
    xlColor beam = cmy->GetBeamColor(nodes);
    CHECK(beam == xlColor(0, 255, 204));
    xlColor c;
    cmy->GetColor(c, 0, 0, false, nullptr, nodes);
    CHECK(c.red == 0);
    CHECK(c.green == 255);
    CHECK(c.blue == 204);
    // a lit white channel wins
    Chan(mh, 7, 90);
    nodes = NodesOf(mh);
    CHECK(cmy->GetBeamColor(nodes) == xlColor(90, 90, 90));
    cmy->GetColor(c, 0, 0, false, nullptr, nodes);
    CHECK(c.red == 90);
    CHECK(c.blue == 90);
    // the layout shows white, a passed colour wins
    cmy->GetColor(c, 0, 0, true, nullptr, nodes);
    CHECK(c.red == 255);
    CHECK(c.green == 255);
    xlColor pass(1, 2, 3);
    cmy->GetColor(c, 0, 0, false, &pass, nodes);
    CHECK(c.red == 1);
    CHECK(c.blue == 3);
}

TEST_CASE("CMY colour with only a white channel shows it as grey", "[dmx2]") {
    Show s;
    auto* mh = As<DmxMovingHead>(s.Add(Dmx("DmxMovingHead", "DmxChannelCount=\"8\" DmxColorType=\"2\" DmxCyanChannel=\"0\" DmxMagentaChannel=\"0\" DmxYellowChannel=\"0\" DmxWhiteChannel=\"8\"")));
    auto* cmy = dynamic_cast<DmxColorAbilityCMY*>(mh->GetColorAbility());
    REQUIRE(cmy != nullptr);
    REQUIRE(cmy->GetCyanChannel() == 0);
    Chan(mh, 8, 70);
    auto nodes = NodesOf(mh);
    CHECK(cmy->GetBeamColor(nodes) == xlColor(70, 70, 70));
    xlColor c;
    cmy->GetColor(c, 0, 0, false, nullptr, nodes);
    CHECK(c.red == 70);
    CHECK(c.green == 70);
}

TEST_CASE("CMY moving head beam and bars use the CMY channels", "[dmx2]") {
    Show s;
    auto* mh = As<DmxMovingHead>(s.Add(Dmx("DmxMovingHead", "DmxChannelCount=\"8\" DmxStyle=\"Moving Head TopBars\" DmxColorType=\"2\" DmxCyanChannel=\"4\" DmxMagentaChannel=\"5\" DmxYellowChannel=\"6\" DmxWhiteChannel=\"7\"",
        "<PanMotor ChannelCoarse=\"1\"/><TiltMotor ChannelCoarse=\"2\"/>")));
    Chan(mh, 4, 0);
    Chan(mh, 5, 255);
    Chan(mh, 6, 255);
    FakePreview p;
    p.frameTime = 1000;
    DrawEffect(mh, p);
    REQUIRE(p.TransAcc().v.size() == 3);
    CHECK(p.TransAcc().v[0].c.red == 255);
    CHECK(p.TransAcc().v[0].c.green == 0);
    CHECK(p.TransAcc().v[0].c.blue == 0);
    // bars: cyan, magenta, yellow and white channels get their own colours
    auto& v = p.SolidAcc().v;
    size_t first = v.size() - 8 * 6;
    CHECK(v[first + 3 * 6].c.red == xlCYAN.red);
    CHECK(v[first + 3 * 6].c.green == xlCYAN.green);
    CHECK(v[first + 4 * 6].c.red == xlMAGENTA.red);
    CHECK(v[first + 4 * 6].c.green == xlMAGENTA.green);
    CHECK(v[first + 5 * 6].c.blue == xlYELLOW.blue);
    CHECK(v[first + 6 * 6].c.red == 255);
    CHECK(v[first + 6 * 6].c.blue == 255);
}

TEST_CASE("Colour wheel reads the wheel slot and dimmer from channel values", "[dmx2]") {
    Show s;
    auto* mh = As<DmxMovingHead>(s.Add(Dmx("DmxMovingHead", "DmxChannelCount=\"8\" DmxColorType=\"1\" DmxColorWheelChannel=\"4\" DmxDimmerChannel=\"5\" "
                                                             "DmxColorWheelDMX0=\"10\" DmxColorWheelColor0=\"#FF0000\" DmxColorWheelDMX1=\"20\" DmxColorWheelColor1=\"#00FF00\"")));
    auto* wheel = dynamic_cast<DmxColorAbilityWheel*>(mh->GetColorAbility());
    REQUIRE(wheel != nullptr);
    Chan(mh, 4, 20);
    Chan(mh, 5, 255);
    auto nodes = NodesOf(mh);
    CHECK(wheel->GetBeamColor(nodes) == xlColor(0, 255, 0));
    xlColor c;
    wheel->GetColor(c, 0, 0, false, nullptr, nodes);
    CHECK(c.green == 255);
    CHECK(c.red == 0);
    // the dimmer scales the colour
    Chan(mh, 5, 51);
    nodes = NodesOf(mh);
    CHECK((int)wheel->GetBeamColor(nodes).green == Approx(51).margin(1));
    wheel->GetColor(c, 0, 0, false, nullptr, nodes);
    CHECK((int)c.green == Approx(51).margin(1));
    // a passed colour wins; the layout (allow selected) is black
    xlColor pass(9, 8, 7);
    wheel->GetColor(c, 0, 0, false, &pass, nodes);
    CHECK(c.red == 9);
    wheel->GetColor(c, 0, 0, true, nullptr, nodes);
    CHECK(c.red == 0);
    CHECK(c.green == 0);
}

TEST_CASE("Colour wheel beam is white without a wheel channel", "[dmx2]") {
    Show s;
    auto* mh = As<DmxMovingHead>(s.Add(Dmx("DmxMovingHead", "DmxChannelCount=\"8\" DmxColorType=\"1\" DmxColorWheelChannel=\"0\"")));
    auto* wheel = dynamic_cast<DmxColorAbilityWheel*>(mh->GetColorAbility());
    REQUIRE(wheel != nullptr);
    auto nodes = NodesOf(mh);
    CHECK(wheel->GetBeamColor(nodes) == xlWHITE);
}

TEST_CASE("Colour wheel moving head bars mark the wheel and dimmer channels white", "[dmx2]") {
    Show s;
    auto* mh = As<DmxMovingHead>(s.Add(Dmx("DmxMovingHead", "DmxChannelCount=\"6\" DmxStyle=\"Moving Head Bars\" DmxColorType=\"1\" DmxColorWheelChannel=\"4\" DmxDimmerChannel=\"5\"",
        "<PanMotor ChannelCoarse=\"1\"/><TiltMotor ChannelCoarse=\"2\"/>")));
    FakePreview p;
    p.frameTime = 1000;
    DrawEffect(mh, p);
    auto& v = p.SolidAcc().v;
    size_t first = v.size() - 6 * 6;
    CHECK(v[first + 3 * 6].c == xlWHITE);
    CHECK(v[first + 4 * 6].c == xlWHITE);
}

TEST_CASE("Shutter ability reads its channel against the threshold", "[dmx2]") {
    Show s;
    auto* fl = As<DmxFloodlight>(s.Add(Dmx("DmxFloodlight", "DmxChannelCount=\"4\" DmxShutterChannel=\"4\" DmxShutterOpen=\"128\"")));
    auto* sh = fl->GetShutterAbility();
    Chan(fl, 4, 127);
    CHECK_FALSE(sh->IsShutterOpen(NodesOf(fl)));
    Chan(fl, 4, 128);
    CHECK(sh->IsShutterOpen(NodesOf(fl)));
    // a channel the model does not have counts as open
    sh->SetShutterChannel(9);
    CHECK(sh->IsShutterOpen(NodesOf(fl)));
}

// --- Presets -----------------------------------------------------------------

TEST_CASE("Preset channel past the end makes the model invalid", "[dmx2]") {
    Show s;
    auto* g = As<DmxGeneral>(s.Add(Dmx("DmxGeneral", "DmxChannelCount=\"4\" DmxPresetChannel0=\"9\" DmxPresetValue0=\"1\"")));
    CHECK_FALSE(g->GetPresetAbility()->IsValidModelSettings(g));
    FakePreview p;
    DrawEffect(g, p);
    CHECK(IsInvalidCross(p.SolidAcc()));
    CHECK(Contains(g->CheckModelSettings(), "9"));

    Show s2;
    auto* ok = As<DmxGeneral>(s2.Add(Dmx("DmxGeneral", "DmxChannelCount=\"4\" DmxPresetChannel0=\"4\" DmxPresetValue0=\"1\"")));
    CHECK(ok->GetPresetAbility()->IsValidModelSettings(ok));
}

TEST_CASE("Preset descriptions name their channels", "[dmx2]") {
    Show s;
    auto* g = As<DmxGeneral>(s.Add(Dmx("DmxGeneral", "DmxChannelCount=\"6\" DmxRedChannel=\"0\" DmxGreenChannel=\"0\" DmxBlueChannel=\"0\" "
                                                     "DmxPresetChannel0=\"2\" DmxPresetValue0=\"10\" DmxPresetDesc0=\"Gobo\" DmxPresetChannel1=\"3\" DmxPresetValue1=\"20\"")));
    auto names = g->GenerateNodeNames();
    REQUIRE(names.size() == 6);
    CHECK(names[1] == "Gobo");
    CHECK(names[2].empty()); // no description
}

// Presets, the floodlight shutter and skull servos on the model's last
// channel are named (the 1-based bound used to be off by one).
TEST_CASE("Functions on the last DMX channel are named", "[dmx2]") {
    {
        Show s;
        auto* g = As<DmxGeneral>(s.Add(Dmx("DmxGeneral", "DmxChannelCount=\"3\" DmxRedChannel=\"0\" DmxGreenChannel=\"0\" DmxBlueChannel=\"0\" DmxPresetChannel0=\"3\" DmxPresetValue0=\"10\" DmxPresetDesc0=\"Gobo\"")));
        CHECK(g->GenerateNodeNames()[2] == "Gobo");
    }
    {
        Show s;
        auto* fl = As<DmxFloodlight>(s.Add(Dmx("DmxFloodlight", "DmxChannelCount=\"4\" DmxShutterChannel=\"4\"")));
        CHECK(fl->GenerateNodeNames()[3] == "Shutter");
    }
    {
        Show s;
        auto* sk = As<DmxSkull>(s.Add(Dmx("DmxSkull", "DmxChannelCount=\"3\" Bits16=\"0\" HasPan=\"0\" HasTilt=\"0\" HasNod=\"0\" HasEyeUD=\"0\" HasEyeLR=\"0\" HasColor=\"0\" DmxEyeBrtChannel=\"0\" DmxRedChannel=\"0\" DmxGreenChannel=\"0\" DmxBlueChannel=\"0\"",
            "<JawServo Channel=\"3\"/>")));
        CHECK(sk->GenerateNodeNames()[2] == "Jaw");
    }
}

// --- DmxServo (2D, image based) ----------------------------------------------

namespace {
struct ImageFiles {
    TempDir dir;
    std::string png;
    ImageFiles() {
        xlImage img(4, 2);
        std::vector<uint8_t> bytes;
        REQUIRE(img.SaveAsPNG(bytes));
        std::string body(bytes.begin(), bytes.end());
        png = dir.Write("img.png", body);
    }
    std::string Img(const std::string& name, const std::string& extra = "") const {
        return "<" + name + " Image=\"" + png + "\" " + extra + "/>";
    }
};

glm::vec3 Apply(const glm::mat4& m, glm::vec3 p) {
    return glm::vec3(m * glm::vec4(p, 1.0f));
}
} // namespace

TEST_CASE("Servo model translates its motion image by the servo value", "[dmx2]") {
    ImageFiles img;
    Show s;
    auto* m = As<DmxServo>(s.Add(Dmx("DmxServo", "DmxChannelCount=\"1\" NumServos=\"1\" Bits16=\"0\" Transparency=\"20\"",
        "<Servo1 Channel=\"1\" MinLimit=\"0\" MaxLimit=\"255\" RangeOfMotion=\"100\" ServoStyle=\"Translate X\"/>" + img.Img("StaticImage1") + img.Img("MotionImage1"))));
    Chan(m, 1, 255);
    FakePreview p;
    DrawEffect(m, p);
    REQUIRE(p.ctx.textures.size() == 2);
    // 20% transparency
    CHECK(p.ctx.textures[0].alpha == 204);
    CHECK(p.ctx.textures[1].alpha == 204);
    // the static image stays at the origin
    glm::vec3 st = Apply(p.ctx.textures[0].matrix, glm::vec3(0, 0, 0));
    CHECK(st.x == Approx(0.0f).margin(1e-5));
    // full travel is 100 units, divided by the model's X scale
    float sx = m->GetBaseObjectScreenLocation().GetScaleMatrix().x;
    glm::vec3 mv = Apply(p.ctx.textures[1].matrix, glm::vec3(0, 0, 0));
    CHECK(mv.x == Approx(100.0f / sx).margin(1e-3));
    CHECK(mv.z == Approx(0.1f).margin(1e-5));

    // the layout shows the rest position
    FakePreview lay;
    DrawLayout(m, lay);
    REQUIRE(lay.ctx.textures.size() == 2);
    CHECK(Apply(lay.ctx.textures[1].matrix, glm::vec3(0, 0, 0)).x == Approx(0.0f).margin(1e-5));
}

TEST_CASE("Servo model rotate servo marks its pivot in the layout", "[dmx2]") {
    ImageFiles img;
    Show s;
    auto* m = As<DmxServo>(s.Add(Dmx("DmxServo", "DmxChannelCount=\"1\" NumServos=\"1\" Bits16=\"0\"",
        "<Servo1 Channel=\"1\" MinLimit=\"0\" MaxLimit=\"255\" RangeOfMotion=\"90\" ServoStyle=\"Rotate Z\" PivotOffsetX=\"0.1\"/>" + img.Img("StaticImage1") + img.Img("MotionImage1"))));
    FakePreview lay;
    DrawLayout(m, lay);
    REQUIRE(lay.ctx.textures.size() == 2);
    // two pivot circles and a cross
    CHECK(lay.TransAcc().v.size() > 4);
    CHECK(lay.ctx.lines.size() == 1);
    CHECK(lay.ctx.lines[0].count == 4);

    // the effect preview turns the image without the pivot marker
    Chan(m, 1, 255);
    FakePreview p;
    DrawEffect(m, p);
    REQUIRE(p.ctx.textures.size() == 2);
    CHECK(p.TransAcc().v.empty());
    glm::vec3 x = Apply(p.ctx.textures[1].matrix, glm::vec3(1, 0, 0)) - Apply(p.ctx.textures[1].matrix, glm::vec3(0, 0, 0));
    // -90 degrees about Z
    CHECK(x.x == Approx(0.0f).margin(1e-4));
    CHECK(x.y == Approx(-1.0f).margin(1e-4));
}

TEST_CASE("Servo model with no images draws the invalid cross", "[dmx2]") {
    Show s;
    auto* m = As<DmxServo>(s.Add(Dmx("DmxServo", "DmxChannelCount=\"1\" NumServos=\"1\" Bits16=\"0\"", "<Servo1 Channel=\"1\"/>")));
    FakePreview p;
    DrawEffect(m, p);
    CHECK(p.ctx.textures.empty());
    CHECK(p.TransAcc().v.size() == 24);
}

TEST_CASE("Servo model with a servo beyond its channels draws the invalid cross", "[dmx2]") {
    Show s;
    auto* m = As<DmxServo>(s.Add(Dmx("DmxServo", "DmxChannelCount=\"2\" NumServos=\"1\" Bits16=\"0\"", "<Servo1 Channel=\"7\"/>")));
    FakePreview p;
    DrawEffect(m, p);
    CHECK(IsInvalidCross(p.TransAcc()));
    CHECK(Contains(m->CheckModelSettings(), "servo 1 is assigned to channel 7"));
}

TEST_CASE("Servo models raise the channel count to fit their servos", "[dmx2]") {
    Show s;
    auto* m = As<DmxServo>(s.Add(Dmx("DmxServo", "DmxChannelCount=\"1\" NumServos=\"3\" Bits16=\"1\"")));
    CHECK(m->GetDmxChannelCount() == 6);
    CHECK(m->GetNodeCount() == 6);
    auto* m3 = As<DmxServo3d>(s.Add(Dmx("DmxServo3d", "DmxChannelCount=\"1\" NumServos=\"2\" Bits16=\"1\"")));
    CHECK(m3->GetDmxChannelCount() == 4);
    CHECK(m3->GetNodeCount() == 4);
    CHECK(m3->GetNodeName(0) == "Servo1");
    CHECK(m3->GetNodeName(2) == "Servo2");
}

TEST_CASE("Servo PWM outputs are servo outputs with their limits", "[dmx2]") {
    Show s;
    auto* m = As<DmxServo3d>(s.Add(Dmx("DmxServo3d", "DmxChannelCount=\"4\" NumServos=\"2\" Bits16=\"1\"",
        "<Servo1 Channel=\"1\" ControllerMin=\"900\" ControllerMax=\"2100\" ControllerReverse=\"1\" ControllerZeroBehavior=\"Center\" ControllerDataType=\"Absolute\"/><Servo2 Channel=\"3\"/>")));
    std::map<uint32_t, PWMOutput> out;
    m->GetPWMOutputs(out);
    REQUIRE(out.count(1) == 1);
    REQUIRE(out.count(3) == 1);
    CHECK(out[1].type == PWMOutput::Type::SERVO);
    CHECK(out[1].channels == 2);
    CHECK(out[1].label == "Servo1");
    CHECK(out[1].min_limit == 900);
    CHECK(out[1].max_limit == 2100);
    CHECK(out[1].reverse);
    CHECK(out[1].zeroStyle == "Center");
    CHECK(out[1].dataType == "Absolute");
    CHECK(out[3].min_limit == 1000);

    Show s2;
    auto* m2 = As<DmxServo>(s2.Add(Dmx("DmxServo", "DmxChannelCount=\"2\" NumServos=\"2\" Bits16=\"0\"")));
    std::map<uint32_t, PWMOutput> out2;
    m2->GetPWMOutputs(out2);
    REQUIRE(out2.count(1) == 1);
    REQUIRE(out2.count(2) == 1);
    CHECK(out2[2].channels == 1);
}

// --- DmxServo3d settings -----------------------------------------------------

TEST_CASE("Servo3d renumbers servos when the bit depth changes", "[dmx2]") {
    Show s;
    auto* m = As<DmxServo3d>(s.Add(Dmx("DmxServo3d", "DmxChannelCount=\"6\" NumServos=\"3\" Bits16=\"1\"")));
    CHECK(m->GetServo(1)->GetChannel() == 3);
    CHECK(m->GetServo(2)->GetChannel() == 5);
    m->SetIs16Bit(false);
    m->UpdateBits();
    m->Reinitialize();
    CHECK(m->GetServo(0)->GetChannel() == 1);
    CHECK(m->GetServo(1)->GetChannel() == 2);
    CHECK(m->GetServo(2)->GetChannel() == 3);
    CHECK_FALSE(m->GetServo(0)->Is16Bit());
}

TEST_CASE("Servo3d settings check reports servos past the channel count", "[dmx2]") {
    Show s;
    auto* m = As<DmxServo3d>(s.Add(Dmx("DmxServo3d", "DmxChannelCount=\"2\" NumServos=\"2\" Bits16=\"0\"", "<Servo2 Channel=\"5\"/>")));
    auto res = m->CheckModelSettings();
    CHECK(Contains(res, "servo 2 is assigned to channel 5 but the model only has 2 channels"));
    CHECK_FALSE(Contains(res, "servo 1"));
    // more servos than channels
    m->SetNumServos(3);
    CHECK(Contains(m->CheckModelSettings(), "requires more channels 3 than have been allocated to it 2"));
}

// --- DmxMovingHeadAdv ----------------------------------------------------------

TEST_CASE("Position zones force a channel while pan and tilt are inside them", "[dmx2]") {
    Show s;
    auto* mh = As<DmxMovingHeadAdv>(s.Add(Dmx("DmxMovingHeadAdv", "DmxChannelCount=\"10\"",
        "<PanMotor ChannelCoarse=\"1\"/><TiltMotor ChannelCoarse=\"3\"/>"
        "<PositionZone Label=\"Low\" PanMin=\"0\" PanMax=\"10\" TiltMin=\"20\" TiltMax=\"30\" Channel=\"8\" Value=\"77\"/>"
        "<PositionZone Label=\"Bad\" PanMin=\"0\" PanMax=\"255\" TiltMin=\"0\" TiltMax=\"255\" Channel=\"11\" Value=\"5\"/>")));
    REQUIRE(mh->GetPositionZones().size() == 2);
    std::vector<uint8_t> frame(20, 0);
    const uint32_t start = 5;
    auto run = [&](uint8_t pan, uint8_t tilt) {
        std::fill(frame.begin(), frame.end(), 0);
        frame[start + 0] = pan;
        frame[start + 2] = tilt;
        frame[start + 7] = 1;
        mh->ApplyPositionZones(frame.data(), start);
        return frame[start + 7];
    };
    CHECK(run(5, 25) == 77);
    CHECK(run(0, 20) == 77);
    CHECK(run(10, 30) == 77);
    CHECK(run(11, 25) == 1);
    CHECK(run(5, 31) == 1);
    // the out of range zone never writes past the model
    run(5, 25);
    CHECK(frame[start + 10] == 0);
}

TEST_CASE("Position zones need pan and tilt channels", "[dmx2]") {
    Show s;
    auto* mh = As<DmxMovingHeadAdv>(s.Add(Dmx("DmxMovingHeadAdv", "DmxChannelCount=\"10\"",
        "<PanMotor ChannelCoarse=\"0\"/><TiltMotor ChannelCoarse=\"3\"/><PositionZone Channel=\"8\" Value=\"77\"/>")));
    std::vector<uint8_t> frame(10, 0);
    mh->ApplyPositionZones(frame.data(), 0);
    CHECK(frame[7] == 0);
    // no zones: nothing to do
    mh->SetPositionZones({});
    mh->ApplyPositionZones(frame.data(), 0);
    CHECK(frame[7] == 0);
}

TEST_CASE("Position zones round trip through serialization", "[dmx2]") {
    Show s;
    auto* mh = As<DmxMovingHeadAdv>(s.Add(Dmx("DmxMovingHeadAdv", "DmxChannelCount=\"10\"",
        "<PanMotor ChannelCoarse=\"1\"/><TiltMotor ChannelCoarse=\"3\"/>"
        "<PositionZone Label=\"Low &amp; left\" PanMin=\"3\" PanMax=\"40\" TiltMin=\"5\" TiltMax=\"60\" Channel=\"8\" Value=\"200\"/>"
        "<PositionZone PanMin=\"100\" Channel=\"0\" Value=\"9\"/>")));
    std::string xml = Serialize(mh);
    Show s2;
    pugi::xml_document doc;
    REQUIRE(doc.load_string(xml.c_str()));
    doc.first_child().attribute("name").set_value("Copy");
    auto* copy = As<DmxMovingHeadAdv>(s2.mm.createAndAddModel(doc.first_child(), 100, 100));
    auto& z = copy->GetPositionZones();
    REQUIRE(z.size() == 2);
    CHECK(z[0].label == "Low & left");
    CHECK(z[0].pan_min == 3);
    CHECK(z[0].pan_max == 40);
    CHECK(z[0].tilt_min == 5);
    CHECK(z[0].tilt_max == 60);
    CHECK(z[0].channel == 8);
    CHECK(z[0].value == 200);
    CHECK(z[1].label.empty());
    CHECK(z[1].pan_min == 100);
    CHECK(z[1].pan_max == 255);
    CHECK(z[1].tilt_max == 255);
    // a zone channel below 1 is clamped to channel 1 on load
    CHECK(z[1].channel == 1);
    CHECK(z[1].value == 9);
    CHECK(Serialize(copy).find("Low &amp; left") != std::string::npos);
}

TEST_CASE("Advanced moving head moves default RGB channels off pan and tilt", "[dmx2]") {
    Show s;
    auto* mh = As<DmxMovingHeadAdv>(s.Add(Dmx("DmxMovingHeadAdv", "DmxChannelCount=\"10\"", "<PanMotor ChannelCoarse=\"1\"/><TiltMotor ChannelCoarse=\"3\"/>")));
    auto* rgb = dynamic_cast<DmxColorAbilityRGB*>(mh->GetColorAbility());
    REQUIRE(rgb != nullptr);
    CHECK(rgb->GetRedChannel() == 5);
    CHECK(rgb->GetGreenChannel() == 6);
    CHECK(rgb->GetBlueChannel() == 7);

    // RGB on 1/2/3 stays when pan is somewhere else
    Show s2;
    auto* mh2 = As<DmxMovingHeadAdv>(s2.Add(Dmx("DmxMovingHeadAdv", "DmxChannelCount=\"10\"", "<PanMotor ChannelCoarse=\"8\"/><TiltMotor ChannelCoarse=\"9\"/>")));
    auto* rgb2 = dynamic_cast<DmxColorAbilityRGB*>(mh2->GetColorAbility());
    CHECK(rgb2->GetRedChannel() == 1);
}

TEST_CASE("Advanced moving head names its channels on load", "[dmx2]") {
    Show s;
    auto* mh = As<DmxMovingHeadAdv>(s.Add(Dmx("DmxMovingHeadAdv", "DmxChannelCount=\"10\" DmxColorType=\"2\" DmxCyanChannel=\"5\" DmxMagentaChannel=\"6\" DmxYellowChannel=\"7\" DmxWhiteChannel=\"8\"",
        "<PanMotor ChannelCoarse=\"1\" ChannelFine=\"2\"/><TiltMotor ChannelCoarse=\"3\" ChannelFine=\"4\"/>")));
    const char* expect[] = { "Pan", "Pan Fine", "Tilt", "Tilt Fine", "Cyan", "Magenta", "Yellow", "White", "", "" };
    for (int i = 0; i < 10; ++i) {
        INFO(i);
        CHECK(mh->GetNodeName(i) == expect[i]);
    }

    Show s2;
    auto* rgbw = As<DmxMovingHeadAdv>(s2.Add(Dmx("DmxMovingHeadAdv", "DmxChannelCount=\"8\" DmxRedChannel=\"4\" DmxGreenChannel=\"5\" DmxBlueChannel=\"6\" DmxWhiteChannel=\"7\"",
        "<PanMotor ChannelCoarse=\"1\"/><TiltMotor ChannelCoarse=\"2\"/>")));
    CHECK(rgbw->GetNodeName(0) == "Pan");
    CHECK(rgbw->GetNodeName(1) == "Tilt");
    CHECK(rgbw->GetNodeName(6) == "White");
}

TEST_CASE("Advanced moving head generated names cover fine speed and shutter channels", "[dmx2]") {
    Show s;
    auto* mh = As<DmxMovingHeadAdv>(s.Add(Dmx("DmxMovingHeadAdv", "DmxChannelCount=\"12\" DmxShutterChannel=\"12\" DmxRedChannel=\"7\" DmxGreenChannel=\"8\" DmxBlueChannel=\"9\"",
        "<PanMotor ChannelCoarse=\"1\" ChannelFine=\"2\" SpeedChannel=\"5\"/><TiltMotor ChannelCoarse=\"3\" ChannelFine=\"4\" SpeedChannel=\"6\"/>")));
    auto n = mh->GenerateNodeNames();
    REQUIRE(n.size() == 12);
    CHECK(n[0] == "Pan");
    CHECK(n[1] == "Pan Fine");
    CHECK(n[2] == "Tilt");
    CHECK(n[3] == "Tilt Fine");
    CHECK(n[4] == "Pan Speed");
    CHECK(n[5] == "Tilt Speed");
    CHECK(n[11] == "Shutter");
}

TEST_CASE("Advanced moving head settings check reports each out of range channel", "[dmx2]") {
    Show s;
    auto* mh = As<DmxMovingHeadAdv>(s.Add(Dmx("DmxMovingHeadAdv", "DmxChannelCount=\"4\" DmxRedChannel=\"1\" DmxGreenChannel=\"1\" DmxBlueChannel=\"1\"",
        "<PanMotor ChannelCoarse=\"5\" ChannelFine=\"6\" SpeedChannel=\"9\"/><TiltMotor ChannelCoarse=\"7\" ChannelFine=\"8\" SpeedChannel=\"10\"/>")));
    auto res = mh->CheckModelSettings();
    CHECK(Contains(res, "requires more channels 7 than have been allocated to it 4"));
    CHECK(Contains(res, "pan motor coarse is assigned to channel 5"));
    CHECK(Contains(res, "pan motor fine is assigned to channel 6"));
    CHECK(Contains(res, "tilt motor coarse is assigned to channel 7"));
    CHECK(Contains(res, "tilt motor fine is assigned to channel 8"));
    CHECK(Contains(res, "pan motor speed is assigned to channel 9"));
    CHECK(Contains(res, "tilt motor speed is assigned to channel 10"));
}

// --- DmxMovingHead -------------------------------------------------------------

TEST_CASE("Moving head settings check reports fine and speed channels", "[dmx2]") {
    Show s;
    auto* mh = As<DmxMovingHead>(s.Add(Dmx("DmxMovingHead", "DmxChannelCount=\"4\" DmxRedChannel=\"1\" DmxGreenChannel=\"1\" DmxBlueChannel=\"1\"",
        "<PanMotor ChannelCoarse=\"1\" ChannelFine=\"6\" SpeedChannel=\"9\"/><TiltMotor ChannelCoarse=\"2\" ChannelFine=\"8\" SpeedChannel=\"10\"/>")));
    auto res = mh->CheckModelSettings();
    CHECK(Contains(res, "pan motor fine is assigned to channel 6"));
    CHECK(Contains(res, "tilt motor fine is assigned to channel 8"));
    CHECK(Contains(res, "pan motor speed is assigned to channel 9"));
    CHECK(Contains(res, "tilt motor speed is assigned to channel 10"));
    CHECK_FALSE(Contains(res, "coarse"));
}

TEST_CASE("Moving head fixed channels hold the shutter open and apply presets", "[dmx2]") {
    Show s;
    auto* mh = As<DmxMovingHead>(s.Add(Dmx("DmxMovingHead", "DmxChannelCount=\"8\" DmxShutterChannel=\"8\" DmxShutterOnValue=\"180\" DmxPresetChannel0=\"7\" DmxPresetValue0=\"12\"")));
    xlColorVector px(8, xlBLACK);
    mh->EnableFixedChannels(px);
    CHECK(px[7] == xlColor(180, 180, 180));
    CHECK(px[6].red == 12);
}

TEST_CASE("Moving head load fills in missing style fixture and depth", "[dmx2]") {
    Show s;
    auto* mh = As<DmxMovingHead>(s.Add(Dmx("DmxMovingHead", "DmxChannelCount=\"8\" DmxStyle=\"\" DmxFixture=\"\" ScaleZ=\"0.25\"")));
    CHECK(mh->GetDmxStyle() == "Moving Head Top");
    CHECK(mh->GetDmxStyleVal() == 0);
    CHECK(mh->GetFixture() == "MH1");
    CHECK(mh->GetBaseObjectScreenLocation().GetScaleMatrix().z == Approx(1.0f));
}

// --- DmxSkull --------------------------------------------------------------------

TEST_CASE("Skull settings check reports each servo and the eye brightness channel", "[dmx2]") {
    Show s;
    auto* sk = As<DmxSkull>(s.Add(Dmx("DmxSkull", "DmxChannelCount=\"4\" DmxEyeBrtChannel=\"30\" DmxRedChannel=\"1\" DmxGreenChannel=\"2\" DmxBlueChannel=\"3\"",
        "<JawServo Channel=\"11\"/><PanServo Channel=\"12\"/><TiltServo Channel=\"13\"/><NodServo Channel=\"14\"/><EyeUpDownServo Channel=\"15\"/><EyeLeftRightServo Channel=\"16\"/>")));
    auto res = static_cast<Model*>(sk)->CheckModelSettings();
    CHECK(Contains(res, "eye brightness channel refers to a channel (30)"));
    CHECK(Contains(res, "jaw servo channel refers to a channel (11)"));
    CHECK(Contains(res, "pan servo channel refers to a channel (12)"));
    CHECK(Contains(res, "tilt servo channel refers to a channel (13)"));
    CHECK(Contains(res, "nod servo channel refers to a channel (14)"));
    CHECK(Contains(res, "eye up/down servo channel refers to a channel (15)"));
    CHECK(Contains(res, "eye left/right servo channel refers to a channel (16)"));
}

TEST_CASE("Skull generated names follow its 16 bit servos and colour", "[dmx2]") {
    Show s;
    auto* sk = As<DmxSkull>(s.Add(Dmx("DmxSkull", "DmxChannelCount=\"18\" DmxRedChannel=\"16\" DmxGreenChannel=\"17\" DmxBlueChannel=\"18\" DmxEyeBrtChannel=\"15\"")));
    auto n = sk->GenerateNodeNames();
    REQUIRE(n.size() == 18);
    const char* expect[] = { "Jaw", "Jaw Fine", "Pan", "Pan Fine", "Tilt", "Tilt Fine", "Nod", "Nod Fine", "Eye U-D", "Eye U-D Fine", "Eye L-R", "Eye L-R Fine" };
    for (int i = 0; i < 12; ++i) {
        INFO(i);
        CHECK(n[i] == expect[i]);
    }
    CHECK(n[14] == "Eye Bright");
    CHECK(n[15].find("Red") != std::string::npos);
    CHECK(n[15].rfind("Eye ", 0) == 0);
}

TEST_CASE("Skull PWM outputs cover servos colour and eye brightness", "[dmx2]") {
    Show s;
    auto* sk = As<DmxSkull>(s.Add(Dmx("DmxSkull", "DmxChannelCount=\"18\" DmxRedChannel=\"16\" DmxGreenChannel=\"17\" DmxBlueChannel=\"18\" DmxEyeBrtChannel=\"15\"")));
    std::map<uint32_t, PWMOutput> out;
    sk->GetPWMOutputs(out);
    for (uint32_t ch : { 1u, 3u, 5u, 7u, 9u, 11u }) {
        INFO(ch);
        REQUIRE(out.count(ch) == 1);
        CHECK(out[ch].type == PWMOutput::Type::SERVO);
        CHECK(out[ch].channels == 2);
    }
    REQUIRE(out.count(15) == 1);
    CHECK(out[15].type == PWMOutput::Type::LED);
    CHECK(out[15].label == "Eye Brightness");
    CHECK(out.count(16) == 1);
    CHECK(out.count(18) == 1);

    // without colour there are no LED outputs
    Show s2;
    auto* plain = As<DmxSkull>(s2.Add(Dmx("DmxSkull", "DmxChannelCount=\"18\" HasColor=\"0\" HasJaw=\"0\"")));
    std::map<uint32_t, PWMOutput> out2;
    plain->GetPWMOutputs(out2);
    CHECK(out2.count(15) == 0);
    CHECK(out2.count(1) == 0);
    CHECK(out2.count(3) == 1);
}

TEST_CASE("Skulltronix setup moves servos to the Skulltronix layout", "[dmx2]") {
    Show s;
    auto* sk = As<DmxSkull>(s.Add(Dmx("DmxSkull", "DmxChannelCount=\"26\"")));
    sk->SetSkulltronix();
    sk->Reinitialize();
    struct Case {
        Servo* servo;
        int channel, min, max;
        float range;
    };
    const Case cases[] = {
        { sk->GetJawServo(), 9, 500, 750, -20 },
        { sk->GetPanServo(), 13, 400, 1100, 180 },
        { sk->GetTiltServo(), 19, 442, 836, -40 },
        { sk->GetNodServo(), 11, 452, 745, 58 },
        { sk->GetEyeUDServo(), 15, 575, 1000, 70 },
        { sk->GetEyeLRServo(), 17, 499, 878, -70 },
    };
    for (auto& c : cases) {
        INFO(c.channel);
        REQUIRE(c.servo != nullptr);
        CHECK(c.servo->GetChannel() == c.channel);
        CHECK(c.servo->GetMinLimit() == c.min);
        CHECK(c.servo->GetMaxLimit() == c.max);
        CHECK(c.servo->GetRangeOfMotion() == Approx(c.range));
        CHECK(c.servo->Is16Bit());
    }
    CHECK(sk->GetEyeBrightnessChannel() == 23);
    CHECK(sk->GetPanOrient() == 90);
    CHECK(sk->GetTiltOrient() == -45);
    CHECK(sk->GetNodeName(7) == "Power");
    CHECK(sk->GetNodeName(8) == "Jaw");
    CHECK(sk->GetNodeName(22) == "Eye Brightness");
    // it only happens once
    sk->GetJawServo()->SetChannel(2);
    sk->Reinitialize();
    CHECK(sk->GetJawServo()->GetChannel() == 2);
}

// --- Servo ------------------------------------------------------------------------

TEST_CASE("Servo styles map to translate or rotate", "[dmx2]") {
    struct Case {
        const char* style;
        bool translate;
    };
    const Case cases[] = { { "Translate X", true }, { "Translate Y", true }, { "Translate Z", true }, { "Rotate X", false }, { "Rotate Y", false }, { "Rotate Z", false }, { "Bogus", true } };
    for (auto& c : cases) {
        INFO(c.style);
        Servo sv("S", false);
        sv.SetStyle(c.style);
        CHECK(sv.IsTranslate() == c.translate);
        CHECK(sv.IsRotate() == !c.translate);
    }
}

TEST_CASE("Servo position clamps to its limits and holds on zero", "[dmx2]") {
    Servo sv("S", false);
    sv.SetMinLimit(100);
    sv.SetMaxLimit(200);
    sv.SetRangeOfMotion(90);
    CHECK(sv.GetPosition(100) == Approx(0.0f));
    CHECK(sv.GetPosition(150) == Approx(-45.0f));
    CHECK(sv.GetPosition(200) == Approx(-90.0f));
    CHECK(sv.GetPosition(50) == Approx(0.0f));
    CHECK(sv.GetPosition(250) == Approx(-90.0f));
    // 0 means "no value": keep the last position
    CHECK(sv.GetPosition(0) == Approx(-90.0f));
}

TEST_CASE("Servo 8 bit clamps its limits to 255", "[dmx2]") {
    Servo sv("S", false);
    sv.SetMinLimit(300);
    sv.SetMaxLimit(4000);
    sv.Set16Bit(false);
    CHECK(sv.GetMinLimit() == 255);
    CHECK(sv.GetMaxLimit() == 255);
}
