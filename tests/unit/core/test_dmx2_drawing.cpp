// Drawing paths of the DMX models, run against recording graphics fakes:
// pan/tilt/servo channel values turned into beam directions and mesh motion
// matrices, slew limiting, shutter/dimmer/colour applied to the beam, and the
// red "invalid" cross drawn when a channel is out of range.

#include <cmath>
#include <sstream>
#include <string>
#include <vector>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <pugixml.hpp>

#include "models/DMX/DmxBeamAbility.h"
#include "models/DMX/DmxGeneral.h"
#include "models/DMX/DmxModel.h"
#include "models/DMX/DmxMovingHead.h"
#include "models/DMX/DmxMovingHeadAdv.h"
#include "models/DMX/DmxServo3D.h"
#include "models/DMX/DmxSkull.h"
#include "models/DMX/Mesh.h"
#include "models/DMX/Servo.h"
#include "models/Model.h"
#include "models/ModelManager.h"
#include "outputs/OutputManager.h"

#include "support/TempDir.h"
#include "test_dmx2_support.h"

using Catch::Approx;
using namespace dmx2;

namespace {


// The direction of a moving head's 2D beam: the beam fan is the first three
// transparent vertices (apex, edge 1, edge 2), so the mid direction is the
// average of the two edge directions.
float BeamDirection(const RecColorAcc& t) {
    REQUIRE(t.v.size() >= 3);
    float mx = (t.v[1].x + t.v[2].x) / 2.0f;
    float my = (t.v[1].y + t.v[2].y) / 2.0f;
    return AngleOf(mx, my);
}


// A 360 degree, 16 bit pan on channels 1/2, an 8 bit 180 degree tilt on 3,
// RGB on 4/5/6 and four spare channels.
const std::string kHeadMotors =
    "<PanMotor ChannelCoarse=\"1\" ChannelFine=\"2\" RangeOfMotion=\"360\" MinLimit=\"-360\" MaxLimit=\"360\"/>"
    "<TiltMotor ChannelCoarse=\"3\" RangeOfMotion=\"180\"/>";
const std::string kHeadAttrs = "DmxChannelCount=\"10\" DmxRedChannel=\"4\" DmxGreenChannel=\"5\" DmxBlueChannel=\"6\" DmxBeamWidth=\"30\" DmxBeamLength=\"4\"";

DmxMovingHead* AddHead(Show& s, const std::string& extra = "", const std::string& motors = kHeadMotors) {
    return As<DmxMovingHead>(s.Add(Dmx("DmxMovingHead", kHeadAttrs + " " + extra, motors)));
}

void SetPan16(Model* m, uint16_t v) {
    Chan(m, 1, v >> 8);
    Chan(m, 2, v & 0xFF);
}

} // namespace

// --- DmxModel::GetChannelValue -----------------------------------------------

namespace {
// Reaches the protected channel readers through a derived-class member pointer.
struct ChannelAccess : DmxModel {
    static int Coarse8(DmxModel* m, int ch) {
        return (m->*static_cast<int (DmxModel::*)(int, bool)>(&ChannelAccess::GetChannelValue))(ch, false);
    }
    static int Coarse16(DmxModel* m, int ch) {
        return (m->*static_cast<int (DmxModel::*)(int, bool)>(&ChannelAccess::GetChannelValue))(ch, true);
    }
    static int CoarseFine(DmxModel* m, int coarse, int fine) {
        return (m->*static_cast<int (DmxModel::*)(int, int)>(&ChannelAccess::GetChannelValue))(coarse, fine);
    }
};
} // namespace

TEST_CASE("DMX channel value reads 8 and 16 bit values from nodes", "[dmx2]") {
    Show s;
    auto* m = As<DmxModel>(s.Add(Dmx("DmxGeneral", "DmxChannelCount=\"6\"")));
    Chan(m, 1, 0x12);
    Chan(m, 2, 0x34);
    Chan(m, 4, 0xAB);
    CHECK(ChannelAccess::Coarse8(m, 0) == 0x12);
    CHECK(ChannelAccess::Coarse8(m, 3) == 0xAB);
    CHECK(ChannelAccess::Coarse16(m, 0) == 0x1234);
    // fine channel need not follow the coarse one
    CHECK(ChannelAccess::CoarseFine(m, 0, 3) == 0x12AB);
    // no fine channel (-1) leaves the low byte empty
    CHECK(ChannelAccess::CoarseFine(m, 3, -1) == 0xAB00);
    // channels past the end read as zero
    CHECK(ChannelAccess::Coarse8(m, 6) == 0);
    CHECK(ChannelAccess::Coarse16(m, 5) == 0);
}

TEST_CASE("DMX channel value reads the last channel as an 8 bit value", "[dmx2]") {
    Show s;
    auto* m = As<DmxModel>(s.Add(Dmx("DmxGeneral", "DmxChannelCount=\"4\"")));
    Chan(m, 4, 200);
    CHECK(ChannelAccess::Coarse8(m, 3) == 200);
}

// A coarse or fine channel on the model's last channel used to read 0 (the
// bound was the 16 bit one), so a head with tilt there never moved.
TEST_CASE("DMX coarse/fine channel value reads the model's last channel", "[dmx2]") {
    Show s;
    auto* m = As<DmxModel>(s.Add(Dmx("DmxGeneral", "DmxChannelCount=\"4\"")));
    Chan(m, 4, 200);
    Chan(m, 3, 7);
    CHECK(ChannelAccess::CoarseFine(m, 3, -1) == (200 << 8));
    CHECK(ChannelAccess::CoarseFine(m, 2, 3) == ((7 << 8) | 200));
}

// --- DmxMovingHead -----------------------------------------------------------

TEST_CASE("Moving head top view points the beam at the pan angle", "[dmx2]") {
    struct Case {
        uint16_t pan;
        float degrees;
    };
    // pan of v/65535 of the 360 degree range, drawn counter clockwise
    const Case cases[] = { { 0, 0.0f }, { 16384, 90.0f }, { 32768, 180.0f }, { 8192, 45.0f } };
    for (auto& c : cases) {
        INFO(c.pan);
        Show s;
        auto* mh = AddHead(s);
        SetPan16(mh, c.pan);
        Chan(mh, 4, 255);
        FakePreview p;
        p.frameTime = 1000;
        DrawEffect(mh, p);
        REQUIRE(p.TransAcc().v.size() == 3);
        CHECK(BeamDirection(p.TransAcc()) == Approx(std::fmod(c.degrees, 360.0f)).margin(0.5));
        // the beam starts at the head and spans the 30 degree beam width
        CHECK(p.TransAcc().v[0].x == Approx(0.0f));
        CHECK(p.TransAcc().v[0].y == Approx(0.0f));
        float a1 = AngleOf(p.TransAcc().v[1].x, p.TransAcc().v[1].y);
        float a2 = AngleOf(p.TransAcc().v[2].x, p.TransAcc().v[2].y);
        float width = std::fmod(a2 - a1 + 360.0f, 360.0f);
        CHECK(width == Approx(30.0f).margin(0.5));
        // half the beam length is shown
        CHECK(std::hypot(p.TransAcc().v[1].x, p.TransAcc().v[1].y) == Approx(2.0f).margin(0.01));
        // the apex carries the red beam colour, the far edge fades out
        CHECK(p.TransAcc().v[0].c.red == 255);
        CHECK(p.TransAcc().v[0].c.green == 0);
        CHECK(p.TransAcc().v[1].c.alpha == 0);
        // both programs draw their whole accumulator once
        REQUIRE(p.ctx.triangles.size() == 2);
        CHECK(p.ctx.pushes == p.ctx.pops);
    }
}

TEST_CASE("Moving head orient zero offsets the drawn pan", "[dmx2]") {
    Show s;
    auto* mh = AddHead(s, "", "<PanMotor ChannelCoarse=\"1\" ChannelFine=\"2\" RangeOfMotion=\"360\" OrientZero=\"-30\"/><TiltMotor ChannelCoarse=\"3\"/>");
    SetPan16(mh, 0);
    Chan(mh, 4, 255);
    FakePreview p;
    p.frameTime = 1000;
    DrawEffect(mh, p);
    CHECK(BeamDirection(p.TransAcc()) == Approx(30.0f).margin(0.5));
}

TEST_CASE("Moving head tilt past vertical flips the beam to the other side", "[dmx2]") {
    Show s;
    // tilt 180 degree range, 8 bit: 0xC0 << 8 is three quarters of the range = 135 degrees
    auto* mh = AddHead(s);
    SetPan16(mh, 8192); // 45 degrees
    Chan(mh, 3, 0xC0);
    Chan(mh, 4, 255);
    FakePreview p;
    p.frameTime = 1000;
    DrawEffect(mh, p);
    CHECK(BeamDirection(p.TransAcc()) == Approx(225.0f).margin(0.5));
}

TEST_CASE("Moving head pan is slew limited between frames", "[dmx2]") {
    Show s;
    auto* mh = AddHead(s, "", "<PanMotor ChannelCoarse=\"1\" ChannelFine=\"2\" RangeOfMotion=\"360\" MinLimit=\"-360\" MaxLimit=\"360\" SlewLimit=\"100\"/><TiltMotor ChannelCoarse=\"3\"/>");
    Chan(mh, 4, 255);
    FakePreview p;
    p.frameTime = 1000;
    SetPan16(mh, 0);
    DrawEffect(mh, p);
    CHECK(BeamDirection(p.TransAcc()) == Approx(0.0f).margin(0.5));

    // 100 deg/s for 100ms lets it move 10 of the requested 90 degrees
    FakePreview p2;
    p2.frameTime = 1100;
    SetPan16(mh, 16384);
    DrawEffect(mh, p2);
    CHECK(BeamDirection(p2.TransAcc()) == Approx(10.0f).margin(0.5));

    // the same frame time again keeps the last drawn position
    FakePreview p3;
    p3.frameTime = 1100;
    DrawEffect(mh, p3);
    CHECK(BeamDirection(p3.TransAcc()) == Approx(10.0f).margin(0.5));

    // moving back is limited the same way
    FakePreview p4;
    p4.frameTime = 1150;
    SetPan16(mh, 0);
    DrawEffect(mh, p4);
    CHECK(BeamDirection(p4.TransAcc()) == Approx(5.0f).margin(0.5));

    // a jump of more than half a second is not slew limited
    FakePreview p5;
    p5.frameTime = 5000;
    SetPan16(mh, 16384);
    DrawEffect(mh, p5);
    CHECK(BeamDirection(p5.TransAcc()) == Approx(90.0f).margin(0.5));
}

TEST_CASE("Moving head slew state is kept per preview", "[dmx2]") {
    Show s;
    auto* mh = AddHead(s, "", "<PanMotor ChannelCoarse=\"1\" ChannelFine=\"2\" RangeOfMotion=\"360\" SlewLimit=\"100\"/><TiltMotor ChannelCoarse=\"3\"/>");
    Chan(mh, 4, 255);
    FakePreview a;
    a.frameTime = 1000;
    SetPan16(mh, 0);
    DrawEffect(mh, a);
    FakePreview b;
    b.name = "Other";
    b.frameTime = 1100;
    SetPan16(mh, 16384);
    DrawEffect(mh, b);
    // the other preview has no history at 1100ms (a jump), so it shows the target
    CHECK(BeamDirection(b.TransAcc()) == Approx(90.0f).margin(0.5));
}

TEST_CASE("Moving head speed channel scales the slew rate", "[dmx2]") {
    struct Case {
        uint8_t speed;
        float expected;
    };
    // slew 100 deg/s, 100ms frame: full speed moves 10 degrees, half 5, zero holds still
    const Case cases[] = { { 255, 10.0f }, { 0, 0.0f } };
    for (auto& c : cases) {
        INFO((int)c.speed);
        Show s;
        auto* mh = AddHead(s, "", "<PanMotor ChannelCoarse=\"1\" ChannelFine=\"2\" RangeOfMotion=\"360\" SlewLimit=\"100\" SpeedChannel=\"7\"/><TiltMotor ChannelCoarse=\"3\" SlewLimit=\"100\" SpeedChannel=\"8\"/>");
        Chan(mh, 4, 255);
        Chan(mh, 7, c.speed);
        Chan(mh, 8, c.speed);
        FakePreview p;
        p.frameTime = 1000;
        SetPan16(mh, 0);
        DrawEffect(mh, p);
        FakePreview p2;
        p2.frameTime = 1100;
        SetPan16(mh, 16384);
        DrawEffect(mh, p2);
        CHECK(BeamDirection(p2.TransAcc()) == Approx(c.expected).margin(0.5));
    }
}

TEST_CASE("Moving head shutter below threshold hides the beam", "[dmx2]") {
    for (int v : { 10, 200 }) {
        INFO(v);
        Show s;
        auto* mh = AddHead(s, "DmxShutterChannel=\"7\" DmxShutterOpen=\"128\" DmxShutterOnValue=\"255\"");
        Chan(mh, 4, 255);
        Chan(mh, 7, (uint8_t)v);
        FakePreview p;
        p.frameTime = 1000;
        DrawEffect(mh, p);
        CHECK(p.TransAcc().v.size() == (v >= 128 ? 3u : 0u));
    }
}

TEST_CASE("Moving head dimmer channel scales the beam colour", "[dmx2]") {
    Show s;
    auto* mh = AddHead(s, "MhDimmerChannel=\"7\"");
    Chan(mh, 4, 200);
    Chan(mh, 5, 100);
    Chan(mh, 7, 51); // 20%
    FakePreview p;
    p.frameTime = 1000;
    DrawEffect(mh, p);
    REQUIRE(p.TransAcc().v.size() == 3);
    CHECK((int)p.TransAcc().v[0].c.red == Approx(40).margin(1));
    CHECK((int)p.TransAcc().v[0].c.green == Approx(20).margin(1));
    CHECK(p.TransAcc().v[0].c.blue == 0);
}

TEST_CASE("Moving head in the layout draws a white beam at its zero position", "[dmx2]") {
    Show s;
    auto* mh = AddHead(s);
    SetPan16(mh, 16384);
    Chan(mh, 4, 255);
    FakePreview p;
    float bbox[6] = { 0 };
    DrawLayout(mh, p, nullptr, false, bbox);
    REQUIRE(p.TransAcc().v.size() == 3);
    CHECK(BeamDirection(p.TransAcc()) == Approx(0.0f).margin(0.5));
    CHECK(p.TransAcc().v[0].c == xlWHITE);
    CHECK(bbox[0] == -0.5f);
    CHECK(bbox[5] == 0.5f);
    // the head body and markers are drawn on the solid program
    CHECK(p.SolidAcc().v.size() > 48);
}

TEST_CASE("Moving head hide body draws only the beam", "[dmx2]") {
    Show s;
    auto* mh = AddHead(s, "HideBody=\"True\"");
    Chan(mh, 4, 255);
    FakePreview p;
    p.frameTime = 1000;
    DrawEffect(mh, p);
    CHECK(p.TransAcc().v.size() == 3);
    CHECK(p.SolidAcc().v.empty());
}

TEST_CASE("Moving head side view draws the head at the tilt angle", "[dmx2]") {
    Show s;
    auto* mh = AddHead(s, "DmxStyle=\"Moving Head Side\"");
    SetPan16(mh, 16384);
    Chan(mh, 3, 0x20); // 0x2000/65535 of 180 degrees
    Chan(mh, 4, 255);
    FakePreview p;
    p.frameTime = 1000;
    DrawEffect(mh, p);
    float tilt = 0x2000 / 65535.0f * 180.0f;
    CHECK(BeamDirection(p.TransAcc()) == Approx(tilt).margin(0.5));
    // 12 head vertices, a 32 segment base circle, the base rect and the pan marker
    CHECK(p.SolidAcc().v.size() == 12 + 32 * 3 + 6 + 3);
    // the pan marker points along the pan angle
    auto& v = p.SolidAcc().v;
    size_t n = v.size();
    float mx = (v[n - 2].x + v[n - 1].x) / 2.0f;
    float my = (v[n - 2].y + v[n - 1].y) / 2.0f;
    CHECK(AngleOf(mx, my) == Approx(90.0f).margin(0.5));
    CHECK(v[n - 1].c.red == 255);
}

TEST_CASE("Moving head bar styles draw one bar per channel", "[dmx2]") {
    for (const char* style : { "Moving Head Bars", "Moving Head TopBars", "Moving Head SideBars" }) {
        INFO(style);
        Show s;
        auto* mh = AddHead(s, std::string("DmxStyle=\"") + style + "\"");
        ClearChans(mh);
        Chan(mh, 1, 255);
        Chan(mh, 3, 51);
        Chan(mh, 4, 255);
        Chan(mh, 9, 102);
        FakePreview p;
        p.frameTime = 1000;
        DrawEffect(mh, p);
        // plain Bars has no beam
        CHECK(p.TransAcc().v.size() == (std::string(style) == "Moving Head Bars" ? 0u : 3u));
        auto& v = p.SolidAcc().v;
        REQUIRE(v.size() >= 10 * 6);
        size_t first = v.size() - 10 * 6;
        auto barLen = [&](int ch) {
            const auto& r = v[first + (ch - 1) * 6];
            const auto& far = v[first + (ch - 1) * 6 + 2];
            REQUIRE(r.x == Approx(0.65f));
            return far.x - r.x;
        };
        CHECK(barLen(1) == Approx(0.8f));
        CHECK(barLen(2) == Approx(0.0f));
        CHECK(barLen(3) == Approx(0.16f));
        CHECK(barLen(9) == Approx(0.32f));
        // pan coarse is pink, tilt turquoise, the red channel red
        CHECK(v[first + 0].c == xlColor(255, 51, 255));
        CHECK(v[first + 2 * 6].c == xlColor(64, 224, 208));
        CHECK(v[first + 3 * 6].c.red == 255);
        CHECK(v[first + 3 * 6].c.green == 0);
    }
}

TEST_CASE("Moving head 3D style draws a 3D beam and body", "[dmx2]") {
    for (uint16_t pan : { (uint16_t)0, (uint16_t)32768 }) {
        INFO(pan);
        Show s;
        auto* mh = AddHead(s, "DmxStyle=\"Moving Head 3D\"");
        SetPan16(mh, pan);
        Chan(mh, 4, 255);
        FakePreview p;
        p.frameTime = 1000;
        DrawEffect(mh, p);
        // four beam faces
        REQUIRE(p.TransAcc().v.size() == 12);
        for (size_t i = 0; i < 12; i += 3) {
            CHECK(p.TransAcc().v[i + 2].x == Approx(0.0f).margin(1e-5));
            CHECK(p.TransAcc().v[i + 2].c.red == 255);
            CHECK(p.TransAcc().v[i].c.alpha == 0);
        }
        // the beam length shown is half the configured 4
        CHECK(std::hypot(p.TransAcc().v[0].x, p.TransAcc().v[0].y, p.TransAcc().v[0].z) > 1.9f);
        CHECK_FALSE(p.SolidAcc().v.empty());
    }
}

TEST_CASE("Moving head 3D style with a black beam draws no beam", "[dmx2]") {
    Show s;
    auto* mh = AddHead(s, "DmxStyle=\"Moving Head 3D\" HideBody=\"True\"");
    ClearChans(mh);
    FakePreview p;
    p.frameTime = 1000;
    DrawEffect(mh, p);
    CHECK(p.TransAcc().v.empty());
    CHECK(p.SolidAcc().v.empty());
}

TEST_CASE("Moving head with motors beyond its channels draws the invalid cross", "[dmx2]") {
    const char* motors[] = {
        "<PanMotor ChannelCoarse=\"20\"/><TiltMotor ChannelCoarse=\"3\"/>",
        "<PanMotor ChannelCoarse=\"1\" ChannelFine=\"20\"/><TiltMotor ChannelCoarse=\"3\"/>",
        "<PanMotor ChannelCoarse=\"1\"/><TiltMotor ChannelCoarse=\"20\"/>",
        "<PanMotor ChannelCoarse=\"1\"/><TiltMotor ChannelCoarse=\"3\" ChannelFine=\"20\"/>",
    };
    for (const char* m : motors) {
        INFO(m);
        Show s;
        auto* mh = AddHead(s, "", m);
        FakePreview p;
        p.frameTime = 1000;
        DrawEffect(mh, p);
        CHECK(IsInvalidCross(p.SolidAcc()));
        CHECK(p.TransAcc().v.empty());
        REQUIRE(p.ctx.lines.size() == 1);
        CHECK(p.ctx.lines[0].count == 12);
    }
}

TEST_CASE("Moving head with abilities beyond its channels draws the invalid cross", "[dmx2]") {
    for (const char* extra : { "DmxShutterChannel=\"30\"", "MhDimmerChannel=\"30\"", "DmxRedChannel=\"30\"" }) {
        INFO(extra);
        Show s;
        auto* mh = As<DmxMovingHead>(s.Add(Dmx("DmxMovingHead", std::string("DmxChannelCount=\"10\" ") + extra, kHeadMotors)));
        FakePreview p;
        p.frameTime = 1000;
        DrawEffect(mh, p);
        CHECK(IsInvalidCross(p.SolidAcc()));
    }
}

TEST_CASE("Effect preview starts and ends drawing when it has no context", "[dmx2]") {
    Show s;
    auto* mh = AddHead(s);
    auto* gen = As<DmxGeneral>(s.Add(Dmx("DmxGeneral", "DmxChannelCount=\"4\"")));
    for (Model* m : std::vector<Model*>{ mh, gen }) {
        FakePreview p;
        p.hasContext = false;
        p.frameTime = 1000;
        DrawEffect(m, p);
        CHECK(p.startCalls == 1);
        CHECK(p.endCalls == 1);
        CHECK_FALSE(p.SolidAcc().v.empty());
    }
}

TEST_CASE("Inactive DMX models draw nothing", "[dmx2]") {
    Show s;
    auto* mh = AddHead(s, "Active=\"0\"");
    REQUIRE_FALSE(mh->IsActive());
    FakePreview p;
    DrawLayout(mh, p);
    CHECK(p.SolidAcc().v.empty());
    FakePreview p2;
    p2.noCurrentModel = true;
    DrawEffect(mh, p2);
    CHECK(p2.SolidAcc().v.empty());
    CHECK(p2.TransAcc().v.empty());
}

TEST_CASE("Selected DMX models draw their handles in the layout", "[dmx2]") {
    Show s;
    std::vector<Model*> models = {
        AddHead(s),
        s.Add(Dmx("DmxGeneral", "DmxChannelCount=\"4\"")),
        s.Add(Dmx("DmxMovingHeadAdv", "DmxChannelCount=\"10\"")),
        s.Add(Dmx("DmxServo3d", "DmxChannelCount=\"4\" NumServos=\"1\"")),
        s.Add(Dmx("DmxSkull", "DmxChannelCount=\"26\"")),
    };
    for (Model* m : models) {
        INFO(m->GetDisplayAsString());
        for (bool is3d : { false, true }) {
            FakePreview plain;
            DrawLayout(m, plain, &xlWHITE, is3d);
            m->Selected(true);
            FakePreview sel;
            DrawLayout(m, sel, &xlWHITE, is3d);
            m->Selected(false);
            CHECK(sel.TransAcc().v.size() > plain.TransAcc().v.size());
        }
    }
}

// --- DmxGeneral --------------------------------------------------------------

TEST_CASE("General DMX draws a bar per channel sized by its value", "[dmx2]") {
    Show s;
    auto* g = As<DmxGeneral>(s.Add(Dmx("DmxGeneral", "DmxChannelCount=\"5\" DmxRedChannel=\"1\" DmxGreenChannel=\"2\" DmxBlueChannel=\"3\" DmxWhiteChannel=\"4\"")));
    const uint8_t vals[5] = { 255, 0, 51, 102, 204 };
    for (int i = 0; i < 5; ++i) {
        Chan(g, i + 1, vals[i]);
    }
    FakePreview p;
    DrawEffect(g, p);
    auto& v = p.SolidAcc().v;
    // four frame sides and five bars
    REQUIRE(v.size() == 9 * 6);
    float lineSize = 1.7f / 5.0f;
    for (int i = 0; i < 5; ++i) {
        INFO(i);
        const auto& a = v[(4 + i) * 6];
        const auto& far = v[(4 + i) * 6 + 2];
        CHECK(a.x == Approx(-0.4f));
        CHECK(far.x - a.x == Approx(vals[i] / 255.0f * 1.8f));
        CHECK(a.y == Approx(0.825f - i * lineSize));
        CHECK(far.y == Approx(0.825f - i * lineSize - lineSize * 0.8f));
    }
    CHECK(v[4 * 6].c.red == 255);
    CHECK(v[4 * 6].c.green == 0);
    CHECK(v[5 * 6].c.green == 255);
    CHECK(v[5 * 6].c.red == 0);
    CHECK(v[6 * 6].c.blue == 255);
    CHECK(v[6 * 6].c.red == 0);
    CHECK(v[7 * 6].c == xlWHITE);
    CHECK(p.TransAcc().v.empty());
}

TEST_CASE("General DMX with a colour channel past the end draws the invalid cross", "[dmx2]") {
    Show s;
    auto* g = As<DmxGeneral>(s.Add(Dmx("DmxGeneral", "DmxChannelCount=\"3\" DmxRedChannel=\"9\" DmxGreenChannel=\"2\" DmxBlueChannel=\"3\"")));
    FakePreview p;
    DrawLayout(g, p);
    CHECK(IsInvalidCross(p.SolidAcc()));
}

TEST_CASE("General DMX layout draw sets the bounding box", "[dmx2]") {
    Show s;
    auto* g = As<DmxGeneral>(s.Add(Dmx("DmxGeneral", "DmxChannelCount=\"3\"")));
    FakePreview p;
    float bbox[6] = { 0 };
    DrawLayout(g, p, nullptr, true, bbox);
    CHECK(bbox[0] == -0.5f);
    CHECK(bbox[3] == 0.5f);
    CHECK(p.SolidAcc().v.size() == 7 * 6);
    CHECK(p.ctx.pushes == p.ctx.pops);
}

// --- Meshes ------------------------------------------------------------------

namespace {
struct CubeFiles {
    TempDir dir;
    std::string obj;
    CubeFiles() {
        dir.Write("cube.mtl", kCubeMtl);
        obj = dir.Write("cube.obj", kCubeObj);
    }
    std::string Mesh(const std::string& name, const std::string& extra = "") const {
        return "<" + name + " ObjFile=\"" + obj + "\" " + extra + "/>";
    }
};

glm::vec3 Apply(const glm::mat4& m, glm::vec3 p) {
    glm::vec4 r = m * glm::vec4(p, 1.0f);
    return glm::vec3(r);
}
} // namespace

// --- DmxMovingHeadAdv --------------------------------------------------------

namespace {
DmxMovingHeadAdv* AddAdv(Show& s, const CubeFiles& cube, const std::string& extra = "", const std::string& more = "") {
    return As<DmxMovingHeadAdv>(s.Add(Dmx("DmxMovingHeadAdv",
        "DmxChannelCount=\"12\" DmxRedChannel=\"5\" DmxGreenChannel=\"6\" DmxBlueChannel=\"7\" DmxBeamWidth=\"4\" DmxBeamLength=\"4\" DmxBeamYOffset=\"0\" " + extra,
        "<PanMotor ChannelCoarse=\"1\" ChannelFine=\"2\" RangeOfMotion=\"360\" MinLimit=\"-360\" MaxLimit=\"360\"/>"
        "<TiltMotor ChannelCoarse=\"3\" RangeOfMotion=\"180\"/>" +
            cube.Mesh("BaseMesh", "Brightness=\"40\"") + cube.Mesh("YokeMesh", "Brightness=\"50\"") + cube.Mesh("HeadMesh", "Brightness=\"80\"") + more)));
}
} // namespace

TEST_CASE("Advanced moving head draws its three meshes", "[dmx2]") {
    CubeFiles cube;
    Show s;
    auto* mh = AddAdv(s, cube);
    Chan(mh, 5, 255);
    FakePreview p;
    p.frameTime = 1000;
    DrawEffect(mh, p);
    CHECK(p.ctx.loadedMeshes.size() == 3);
    REQUIRE(p.ctx.solids.size() == 3);
    CHECK(p.ctx.transparents.size() == 3);
    CHECK(p.ctx.solids[0].brightness == 40);
    CHECK(p.ctx.solids[1].brightness == 50);
    CHECK(p.ctx.solids[2].brightness == 80);
    // the beam is four triangles in the beam colour
    REQUIRE(p.TransAcc().v.size() == 12);
    CHECK(p.TransAcc().v[2].c.red == 255);
    CHECK(p.TransAcc().v[2].c.green == 0);
}

TEST_CASE("Advanced moving head rotates the yoke by pan and the head by tilt", "[dmx2]") {
    CubeFiles cube;
    Show s;
    auto* mh = AddAdv(s, cube);
    SetPan16(mh, 16384); // 90 degrees
    Chan(mh, 3, 0x40);   // 0x4000/65535 of 180 = 45 degrees
    Chan(mh, 5, 255);
    FakePreview p;
    p.frameTime = 1000;
    DrawEffect(mh, p);
    REQUIRE(p.ctx.solids.size() == 3);
    float pan = -16384 / 65535.0f * 360.0f;
    float tilt = -0x4000 / 65535.0f * 180.0f;
    // base mesh is not moved
    glm::vec3 b = Apply(p.ctx.solids[0].matrix, glm::vec3(1, 0, 0));
    CHECK(b.x == Approx(1.0f));
    CHECK(b.z == Approx(0.0f).margin(1e-5));
    // the yoke turns about Y by the pan angle
    glm::vec3 y = Apply(p.ctx.solids[1].matrix, glm::vec3(1, 0, 0));
    glm::vec3 ey = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(pan), glm::vec3(0, 1, 0)) * glm::vec4(1, 0, 0, 1));
    CHECK(y.x == Approx(ey.x).margin(1e-4));
    CHECK(y.z == Approx(ey.z).margin(1e-4));
    // the head also tilts about X
    glm::mat4 eh = glm::rotate(glm::mat4(1.0f), glm::radians(pan), glm::vec3(0, 1, 0)) * glm::rotate(glm::mat4(1.0f), glm::radians(tilt), glm::vec3(1, 0, 0));
    glm::vec3 h = Apply(p.ctx.solids[2].matrix, glm::vec3(0, 1, 0));
    glm::vec3 eh1 = glm::vec3(eh * glm::vec4(0, 1, 0, 1));
    CHECK(h.x == Approx(eh1.x).margin(1e-4));
    CHECK(h.y == Approx(eh1.y).margin(1e-4));
    CHECK(h.z == Approx(eh1.z).margin(1e-4));
}

TEST_CASE("Advanced moving head mesh-only draws wireframes", "[dmx2]") {
    CubeFiles cube;
    Show s;
    auto* mh = As<DmxMovingHeadAdv>(s.Add(Dmx("DmxMovingHeadAdv", "DmxChannelCount=\"12\"",
        "<PanMotor ChannelCoarse=\"1\"/><TiltMotor ChannelCoarse=\"3\"/>" + cube.Mesh("BaseMesh", "MeshOnly=\"1\"") + cube.Mesh("YokeMesh") + cube.Mesh("HeadMesh"))));
    FakePreview p;
    p.frameTime = 1000;
    DrawEffect(mh, p);
    REQUIRE(p.ctx.solids.size() == 3);
    CHECK(p.ctx.solids[0].wireframe);
    CHECK_FALSE(p.ctx.solids[1].wireframe);
    // a wireframe mesh has no transparent pass
    CHECK(p.ctx.transparents.size() == 2);
}

TEST_CASE("Advanced moving head head mesh sets the model size", "[dmx2]") {
    CubeFiles cube;
    Show s;
    auto* mh = As<DmxMovingHeadAdv>(s.Add(Dmx("DmxMovingHeadAdv", "DmxChannelCount=\"12\"",
        "<PanMotor ChannelCoarse=\"1\"/><TiltMotor ChannelCoarse=\"3\"/>" + cube.Mesh("HeadMesh", "ScaleX=\"2\" ScaleY=\"3\" ScaleZ=\"4\""))));
    FakePreview p;
    p.frameTime = 1000;
    DrawEffect(mh, p);
    auto& sl = mh->GetModelScreenLocation();
    CHECK(sl.GetRenderWi() == Approx(2.0f));
    CHECK(sl.GetRenderHt() == Approx(3.0f));
    CHECK(sl.GetRenderDp() == Approx(4.0f));
}

TEST_CASE("Advanced moving head shutter and dimmer shape the beam", "[dmx2]") {
    CubeFiles cube;
    {
        Show s;
        auto* mh = AddAdv(s, cube, "DmxShutterChannel=\"8\" DmxShutterOpen=\"100\" DmxShutterOnValue=\"255\"");
        Chan(mh, 5, 255);
        Chan(mh, 8, 50);
        FakePreview p;
        p.frameTime = 1000;
        DrawEffect(mh, p);
        CHECK(p.TransAcc().v.empty());
    }
    {
        Show s;
        auto* mh = AddAdv(s, cube, "MhDimmerChannel=\"9\"");
        Chan(mh, 5, 200);
        Chan(mh, 9, 128);
        FakePreview p;
        p.frameTime = 1000;
        DrawEffect(mh, p);
        REQUIRE(p.TransAcc().v.size() == 12);
        CHECK((int)p.TransAcc().v[2].c.red == Approx(100).margin(1));
    }
    {
        // in the layout the beam is white whatever the channels say
        Show s;
        auto* mh = AddAdv(s, cube);
        FakePreview p;
        DrawLayout(mh, p);
        REQUIRE(p.TransAcc().v.size() == 12);
        CHECK(p.TransAcc().v[2].c == xlWHITE);
    }
}

TEST_CASE("Advanced moving head beam y offset lifts the beam origin", "[dmx2]") {
    CubeFiles cube;
    Show s;
    auto* mh = AddAdv(s, cube, "", "");
    mh->GetBeamAbility()->SetBeamYOffset(2.5f);
    Chan(mh, 5, 255);
    FakePreview p;
    p.frameTime = 1000;
    DrawEffect(mh, p);
    REQUIRE(p.TransAcc().v.size() == 12);
    CHECK(p.TransAcc().v[2].y == Approx(2.5f));
}

TEST_CASE("Advanced moving head position zone overrides the beam channels", "[dmx2]") {
    CubeFiles cube;
    Show s;
    // inside pan 0..10 / tilt 0..10 the zone forces the red channel to 0
    auto* mh = AddAdv(s, cube, "", "<PositionZone Label=\"Down\" PanMin=\"0\" PanMax=\"10\" TiltMin=\"0\" TiltMax=\"10\" Channel=\"5\" Value=\"0\"/>");
    Chan(mh, 5, 255);
    Chan(mh, 6, 255);
    FakePreview p;
    p.frameTime = 1000;
    p.zoneIndicator = true;
    DrawEffect(mh, p);
    REQUIRE(p.TransAcc().v.size() == 12);
    CHECK(p.TransAcc().v[2].c.red == 0);
    CHECK(p.TransAcc().v[2].c.green == 255);
    // the node itself is restored after the draw
    xlColor c;
    mh->GetNode(4)->GetColor(c);
    CHECK(c.red == 255);
    // and an orange ring of 48 segments marks the active zone
    size_t orange = 0;
    for (auto& v : p.SolidAcc().v) {
        if (v.c == xlColor(255, 140, 0, 220)) {
            ++orange;
        }
    }
    CHECK(orange == 48 * 6);

    // outside the zone nothing changes and no ring is drawn
    Chan(mh, 1, 200);
    FakePreview p2;
    p2.frameTime = 1000;
    p2.zoneIndicator = true;
    DrawEffect(mh, p2);
    CHECK(p2.TransAcc().v[2].c.red == 255);
    CHECK(p2.SolidAcc().v.empty());

    // previews can turn zones off
    Chan(mh, 1, 0);
    FakePreview p3;
    p3.frameTime = 1000;
    p3.zonesEnabled = false;
    DrawEffect(mh, p3);
    CHECK(p3.TransAcc().v[2].c.red == 255);
}

TEST_CASE("Advanced moving head with too few channels draws the invalid cross", "[dmx2]") {
    CubeFiles cube;
    struct Case {
        const char* attrs;
        const char* motors;
    };
    const Case cases[] = {
        { "DmxChannelCount=\"3\"", "<PanMotor ChannelCoarse=\"1\"/><TiltMotor ChannelCoarse=\"2\"/>" },
        { "DmxChannelCount=\"8\"", "<PanMotor ChannelCoarse=\"9\"/><TiltMotor ChannelCoarse=\"2\"/>" },
        { "DmxChannelCount=\"8\"", "<PanMotor ChannelCoarse=\"1\" ChannelFine=\"9\"/><TiltMotor ChannelCoarse=\"3\"/>" },
        { "DmxChannelCount=\"8\"", "<PanMotor ChannelCoarse=\"1\"/><TiltMotor ChannelCoarse=\"9\"/>" },
        { "DmxChannelCount=\"8\"", "<PanMotor ChannelCoarse=\"1\"/><TiltMotor ChannelCoarse=\"3\" ChannelFine=\"9\"/>" },
        { "DmxChannelCount=\"8\" MhDimmerChannel=\"9\"", "<PanMotor ChannelCoarse=\"1\"/><TiltMotor ChannelCoarse=\"3\"/>" },
    };
    for (auto& c : cases) {
        INFO(c.attrs << " " << c.motors);
        Show s;
        auto* mh = As<DmxMovingHeadAdv>(s.Add(Dmx("DmxMovingHeadAdv", c.attrs, c.motors)));
        FakePreview p;
        p.frameTime = 1000;
        DrawEffect(mh, p);
        CHECK(IsInvalidCross(p.SolidAcc()));
        CHECK(p.ctx.solids.empty());
    }
}

TEST_CASE("Advanced moving head pan is slew limited", "[dmx2]") {
    CubeFiles cube;
    Show s;
    auto* mh = As<DmxMovingHeadAdv>(s.Add(Dmx("DmxMovingHeadAdv", "DmxChannelCount=\"12\" DmxRedChannel=\"5\" DmxGreenChannel=\"6\" DmxBlueChannel=\"7\"",
        "<PanMotor ChannelCoarse=\"1\" ChannelFine=\"2\" RangeOfMotion=\"360\" MinLimit=\"-360\" MaxLimit=\"360\" SlewLimit=\"100\" SpeedChannel=\"9\"/>"
        "<TiltMotor ChannelCoarse=\"3\" RangeOfMotion=\"180\" SlewLimit=\"100\" SpeedChannel=\"9\"/>" +
            cube.Mesh("YokeMesh") + cube.Mesh("HeadMesh"))));
    Chan(mh, 5, 255);
    Chan(mh, 9, 255);
    FakePreview p;
    p.frameTime = 1000;
    DrawEffect(mh, p);
    FakePreview p2;
    p2.frameTime = 1100;
    SetPan16(mh, 16384);
    Chan(mh, 3, 0x40);
    DrawEffect(mh, p2);
    REQUIRE(p2.ctx.solids.size() == 2);
    // the yoke turned 10 of the 90 degrees requested (rotation is negative pan)
    glm::vec3 y = Apply(p2.ctx.solids[0].matrix, glm::vec3(1, 0, 0));
    CHECK(AngleOf(y.x, -y.z) == Approx(350.0f).margin(0.5));

    // the same frame time holds the last position
    FakePreview p3;
    p3.frameTime = 1100;
    DrawEffect(mh, p3);
    glm::vec3 y3 = Apply(p3.ctx.solids[0].matrix, glm::vec3(1, 0, 0));
    CHECK(AngleOf(y3.x, -y3.z) == Approx(350.0f).margin(0.5));
}

// --- DmxServo3d --------------------------------------------------------------

TEST_CASE("Servo3d translate servo moves its motion mesh", "[dmx2]") {
    CubeFiles cube;
    Show s;
    // 16 bit servo on channels 1/2, 0..65535 maps to 0..-100 units of travel
    auto* m = As<DmxServo3d>(s.Add(Dmx("DmxServo3d", "DmxChannelCount=\"2\" NumServos=\"1\" NumStatic=\"1\" NumMotion=\"1\" Bits16=\"1\"",
        "<Servo1 Channel=\"1\" MinLimit=\"0\" MaxLimit=\"65535\" RangeOfMotion=\"100\" ServoStyle=\"Translate Y\"/>" + cube.Mesh("StaticMesh1") + cube.Mesh("MotionMesh1"))));
    Chan(m, 1, 0x80);
    Chan(m, 2, 0x00);
    FakePreview p;
    DrawEffect(m, p);
    REQUIRE(p.ctx.solids.size() == 2);
    // static mesh does not move
    glm::vec3 st = Apply(p.ctx.solids[0].matrix, glm::vec3(0, 0, 0));
    CHECK(st.y == Approx(0.0f).margin(1e-5));
    // the servo position is -(0x8000/65535)*100, the mesh moves the opposite way
    float pos = -(0x8000 / 65535.0f) * 100.0f;
    glm::vec3 mv = Apply(p.ctx.solids[1].matrix, glm::vec3(0, 0, 0));
    CHECK(mv.x == Approx(0.0f).margin(1e-4));
    CHECK(mv.y == Approx(-pos).margin(1e-2));
}

TEST_CASE("Servo3d rotate servo turns its motion mesh", "[dmx2]") {
    CubeFiles cube;
    for (const char* style : { "Rotate X", "Rotate Y", "Rotate Z", "Translate X", "Translate Z" }) {
        INFO(style);
        Show s;
        auto* m = As<DmxServo3d>(s.Add(Dmx("DmxServo3d", "DmxChannelCount=\"1\" NumServos=\"1\" NumStatic=\"1\" NumMotion=\"1\" Bits16=\"0\"",
            std::string("<Servo1 Channel=\"1\" MinLimit=\"0\" MaxLimit=\"255\" RangeOfMotion=\"90\" ServoStyle=\"") + style + "\"/>" + cube.Mesh("StaticMesh1") + cube.Mesh("MotionMesh1"))));
        Chan(m, 1, 255);
        FakePreview p;
        DrawEffect(m, p);
        REQUIRE(p.ctx.solids.size() == 2);
        // 255 is the end of travel: -90 degrees / units
        Servo* sv = m->GetServo(0);
        glm::mat4 expect(1.0f);
        sv->FillMotionMatrix(-90.0f, expect);
        glm::vec3 got = Apply(p.ctx.solids[1].matrix, glm::vec3(1, 2, 3));
        glm::vec3 want = Apply(expect, glm::vec3(1, 2, 3));
        CHECK(got.x == Approx(want.x).margin(1e-3));
        CHECK(got.y == Approx(want.y).margin(1e-3));
        CHECK(got.z == Approx(want.z).margin(1e-3));
        CHECK(glm::length(got - glm::vec3(1, 2, 3)) > 1.0f);
    }
}

TEST_CASE("Servo3d linked servo drives another mesh", "[dmx2]") {
    CubeFiles cube;
    Show s;
    // servo 2 is redirected to mesh 1, so mesh 1 moves by both and mesh 2 stays put
    auto* m = As<DmxServo3d>(s.Add(Dmx("DmxServo3d", "DmxChannelCount=\"2\" NumServos=\"2\" NumStatic=\"2\" NumMotion=\"2\" Bits16=\"0\" Servo2Linkage=\"Mesh 1\"",
        "<Servo1 Channel=\"1\" MinLimit=\"0\" MaxLimit=\"255\" RangeOfMotion=\"10\" ServoStyle=\"Translate X\"/>"
        "<Servo2 Channel=\"2\" MinLimit=\"0\" MaxLimit=\"255\" RangeOfMotion=\"20\" ServoStyle=\"Translate Y\"/>" +
            cube.Mesh("StaticMesh1") + cube.Mesh("StaticMesh2") + cube.Mesh("MotionMesh1") + cube.Mesh("MotionMesh2"))));
    REQUIRE(m->GetServoLink(1) == 0);
    Chan(m, 1, 255);
    Chan(m, 2, 255);
    FakePreview p;
    DrawEffect(m, p);
    REQUIRE(p.ctx.solids.size() == 4);
    glm::vec3 m1 = Apply(p.ctx.solids[2].matrix, glm::vec3(0, 0, 0));
    glm::vec3 m2 = Apply(p.ctx.solids[3].matrix, glm::vec3(0, 0, 0));
    CHECK(m1.x == Approx(10.0f).margin(1e-3));
    CHECK(m1.y == Approx(20.0f).margin(1e-3));
    CHECK(m2.x == Approx(0.0f).margin(1e-4));
    CHECK(m2.y == Approx(0.0f).margin(1e-4));
}

TEST_CASE("Servo3d mesh linkage chains motion onto a child mesh", "[dmx2]") {
    CubeFiles cube;
    Show s;
    // mesh 2 is attached to mesh 1, so it moves with servo 1 and with its own servo 2
    auto* m = As<DmxServo3d>(s.Add(Dmx("DmxServo3d", "DmxChannelCount=\"2\" NumServos=\"2\" NumStatic=\"1\" NumMotion=\"2\" Bits16=\"0\" Mesh2Linkage=\"Mesh 1\"",
        "<Servo1 Channel=\"1\" MinLimit=\"0\" MaxLimit=\"255\" RangeOfMotion=\"10\" ServoStyle=\"Translate X\"/>"
        "<Servo2 Channel=\"2\" MinLimit=\"0\" MaxLimit=\"255\" RangeOfMotion=\"20\" ServoStyle=\"Translate Y\"/>" +
            cube.Mesh("StaticMesh1") + cube.Mesh("MotionMesh1") + cube.Mesh("MotionMesh2"))));
    REQUIRE(m->GetMeshLink(1) == 0);
    Chan(m, 1, 255);
    Chan(m, 2, 255);
    FakePreview p;
    DrawEffect(m, p);
    REQUIRE(p.ctx.solids.size() == 3);
    glm::vec3 m1 = Apply(p.ctx.solids[1].matrix, glm::vec3(0, 0, 0));
    glm::vec3 m2 = Apply(p.ctx.solids[2].matrix, glm::vec3(0, 0, 0));
    CHECK(m1.x == Approx(10.0f).margin(1e-3));
    CHECK(m1.y == Approx(0.0f).margin(1e-3));
    CHECK(m2.x == Approx(10.0f).margin(1e-3));
    CHECK(m2.y == Approx(20.0f).margin(1e-3));
}

TEST_CASE("Servo3d in the layout draws meshes at rest", "[dmx2]") {
    CubeFiles cube;
    Show s;
    auto* m = As<DmxServo3d>(s.Add(Dmx("DmxServo3d", "DmxChannelCount=\"1\" NumServos=\"1\" NumStatic=\"1\" NumMotion=\"1\" Bits16=\"0\"",
        "<Servo1 Channel=\"1\" MinLimit=\"0\" MaxLimit=\"255\" RangeOfMotion=\"10\" ServoStyle=\"Translate X\"/>" + cube.Mesh("StaticMesh1") + cube.Mesh("MotionMesh1"))));
    Chan(m, 1, 255);
    FakePreview p;
    float bbox[6] = { 0 };
    DrawLayout(m, p, nullptr, true, bbox);
    REQUIRE(p.ctx.solids.size() == 2);
    glm::vec3 mv = Apply(p.ctx.solids[1].matrix, glm::vec3(0, 0, 0));
    CHECK(mv.x == Approx(0.0f).margin(1e-5));
    CHECK(bbox[0] == -0.5f);
}

TEST_CASE("Servo3d with missing mesh files draws invalid crosses in their place", "[dmx2]") {
    CubeFiles cube;
    std::string missing = cube.dir.File("missing.obj");
    {
        // one of the pair is loaded: it is drawn and nothing is flagged
        Show s;
        auto* m = As<DmxServo3d>(s.Add(Dmx("DmxServo3d", "DmxChannelCount=\"1\" NumServos=\"1\" NumStatic=\"1\" NumMotion=\"1\" Bits16=\"0\"",
            "<Servo1 Channel=\"1\"/><StaticMesh1 ObjFile=\"" + missing + "\"/>" + cube.Mesh("MotionMesh1"))));
        FakePreview p;
        DrawEffect(m, p);
        CHECK(p.ctx.solids.size() == 1);
        CHECK(p.SolidAcc().v.empty());
    }
    {
        // neither loads: both slots show the red cross
        Show s;
        auto* m = As<DmxServo3d>(s.Add(Dmx("DmxServo3d", "DmxChannelCount=\"1\" NumServos=\"1\" NumStatic=\"1\" NumMotion=\"1\" Bits16=\"0\"",
            "<Servo1 Channel=\"1\"/><StaticMesh1 ObjFile=\"" + missing + "\"/><MotionMesh1 ObjFile=\"" + missing + "\"/>")));
        FakePreview p;
        DrawEffect(m, p);
        CHECK(p.ctx.solids.empty());
        CHECK(p.SolidAcc().v.size() == 24);
        CHECK(p.ctx.lines.size() == 2);
    }
}

TEST_CASE("Servo3d with a servo beyond its channels draws the invalid cross", "[dmx2]") {
    Show s;
    auto* m = As<DmxServo3d>(s.Add(Dmx("DmxServo3d", "DmxChannelCount=\"2\" NumServos=\"1\" Bits16=\"0\"", "<Servo1 Channel=\"5\"/>")));
    FakePreview p;
    DrawEffect(m, p);
    CHECK(IsInvalidCross(p.SolidAcc()));
}

// --- DmxSkull ----------------------------------------------------------------

namespace {
std::string SkullMeshes(const CubeFiles& cube) {
    return cube.Mesh("HeadMesh") + cube.Mesh("JawMesh") + cube.Mesh("EyeMeshL") + cube.Mesh("EyeMeshR");
}
} // namespace

TEST_CASE("Skull draws head jaw and both eyes", "[dmx2]") {
    CubeFiles cube;
    Show s;
    auto* sk = As<DmxSkull>(s.Add(Dmx("DmxSkull", "DmxChannelCount=\"26\"", SkullMeshes(cube))));
    FakePreview p;
    DrawEffect(sk, p);
    CHECK(p.ctx.loadedMeshes.size() == 4);
    CHECK(p.ctx.solids.size() == 4);
    CHECK(p.ctx.transparents.size() == 4);
}

TEST_CASE("Skull jaw servo opens the jaw", "[dmx2]") {
    CubeFiles cube;
    Show s;
    // only a jaw, 8 bit, full travel of 20 degrees
    auto* sk = As<DmxSkull>(s.Add(Dmx("DmxSkull", "DmxChannelCount=\"26\" Bits16=\"0\" HasPan=\"0\" HasTilt=\"0\" HasNod=\"0\" HasEyeUD=\"0\" HasEyeLR=\"0\" HasColor=\"0\" DmxJawOrient=\"0\"",
        "<JawServo Channel=\"1\" MinLimit=\"0\" MaxLimit=\"255\" RangeOfMotion=\"20\" ServoStyle=\"Rotate X\"/>" + SkullMeshes(cube))));
    FakePreview closed;
    Chan(sk, 1, 0);
    DrawEffect(sk, closed);
    FakePreview open;
    Chan(sk, 1, 255);
    DrawEffect(sk, open);
    REQUIRE(closed.ctx.solids.size() == 4);
    REQUIRE(open.ctx.solids.size() == 4);
    // the head does not move with the jaw
    CHECK(closed.ctx.solids[0].matrix == open.ctx.solids[0].matrix);
    // the jaw does
    CHECK_FALSE(closed.ctx.solids[1].matrix == open.ctx.solids[1].matrix);
    // channel value 0 means "hold": it keeps the last position
    FakePreview again;
    Chan(sk, 1, 0);
    DrawEffect(sk, again);
    CHECK(again.ctx.solids[1].matrix == open.ctx.solids[1].matrix);
}

TEST_CASE("Skull pan servo turns the head and everything on it", "[dmx2]") {
    CubeFiles cube;
    Show s;
    auto* sk = As<DmxSkull>(s.Add(Dmx("DmxSkull", "DmxChannelCount=\"26\" Bits16=\"0\" HasJaw=\"0\" HasTilt=\"0\" HasNod=\"0\" HasEyeUD=\"0\" HasEyeLR=\"0\" HasColor=\"0\" DmxPanOrient=\"0\"",
        "<PanServo Channel=\"3\" MinLimit=\"1\" MaxLimit=\"255\" RangeOfMotion=\"180\" ServoStyle=\"Rotate Y\"/>" + SkullMeshes(cube))));
    Chan(sk, 3, 128);
    FakePreview p;
    DrawEffect(sk, p);
    REQUIRE(p.ctx.solids.size() == 4);
    // the servo position for 128 of 1..255
    float pos = (1.0f - (127.0f / 254.0f)) * 180.0f - 180.0f;
    glm::mat4 expect = glm::rotate(glm::mat4(1.0f), glm::radians(pos), glm::vec3(0, 1, 0));
    // the head mesh matrix carries the pan rotation (the cube is 1 wide so the render scale is 1)
    glm::vec3 got = Apply(p.ctx.solids[0].matrix, glm::vec3(1, 0, 0));
    glm::vec3 want = Apply(expect, glm::vec3(1, 0, 0));
    CHECK(got.x == Approx(want.x).margin(1e-3));
    CHECK(got.z == Approx(want.z).margin(1e-3));
    // the eyes ride along with the head
    glm::vec3 eye = Apply(p.ctx.solids[2].matrix, glm::vec3(0, 0, 0));
    glm::vec3 eyeWant = Apply(expect, glm::vec3(-1.0f, 4.5f, 3.2f));
    CHECK(eye.x == Approx(eyeWant.x).margin(1e-3));
    CHECK(eye.y == Approx(eyeWant.y).margin(1e-3));
    CHECK(eye.z == Approx(eyeWant.z).margin(1e-3));
}

TEST_CASE("Skull eye colour follows its colour channels", "[dmx2]") {
    CubeFiles cube;
    Show s;
    auto* sk = As<DmxSkull>(s.Add(Dmx("DmxSkull", "DmxChannelCount=\"26\" HasColor=\"1\" DmxRedChannel=\"24\" DmxGreenChannel=\"25\" DmxBlueChannel=\"26\" DmxEyeBrtChannel=\"23\"", SkullMeshes(cube))));
    Chan(sk, 24, 255);
    Chan(sk, 25, 0);
    Chan(sk, 26, 128);
    FakePreview p;
    DrawEffect(sk, p);
    REQUIRE(p.ctx.solids.size() == 4);
    const xlMesh* eye = p.ctx.solids[2].mesh;
    REQUIRE(eye != nullptr);
    bool found = false;
    for (int i = 0; i < 2; ++i) {
        auto& mat = eye->GetMaterial(i);
        if (mat.name == "EyeColor") {
            found = true;
            CHECK(mat.forceColor);
            CHECK(mat.color.red == 255);
            CHECK(mat.color.green == 0);
            CHECK(mat.color.blue == 128);
        }
    }
    CHECK(found);

    // a later black is applied
    ClearChans(sk);
    FakePreview p2;
    DrawEffect(sk, p2);
    for (int i = 0; i < 2; ++i) {
        auto& mat = eye->GetMaterial(i);
        if (mat.name == "EyeColor") {
            CHECK(mat.color.red == 0);
            CHECK(mat.color.blue == 0);
        }
    }
}

// Mesh::Draw skips unchanged colours; a first colour of black must still
// replace the .mtl colour.
TEST_CASE("Skull without colour paints its eyes black", "[dmx2]") {
    CubeFiles cube;
    Show s;
    auto* sk = As<DmxSkull>(s.Add(Dmx("DmxSkull", "DmxChannelCount=\"26\" HasColor=\"0\"", SkullMeshes(cube))));
    FakePreview p;
    DrawEffect(sk, p);
    REQUIRE(p.ctx.solids.size() == 4);
    const xlMesh* eye = p.ctx.solids[3].mesh;
    for (int i = 0; i < 2; ++i) {
        auto& mat = eye->GetMaterial(i);
        if (mat.name == "EyeColor") {
            CHECK(mat.color.red == 0);
            CHECK(mat.color.green == 0);
            CHECK(mat.color.blue == 0);
        }
    }
}

TEST_CASE("Skull with servos beyond its channels draws the invalid cross", "[dmx2]") {
    const char* servos[] = { "JawServo", "PanServo", "TiltServo", "NodServo", "EyeUpDownServo", "EyeLeftRightServo" };
    for (const char* sv : servos) {
        INFO(sv);
        Show s;
        auto* sk = As<DmxSkull>(s.Add(Dmx("DmxSkull", "DmxChannelCount=\"26\" HasColor=\"0\"", std::string("<") + sv + " Channel=\"40\"/>")));
        FakePreview p;
        DrawEffect(sk, p);
        CHECK(IsInvalidCross(p.SolidAcc()));
        CHECK(p.ctx.solids.empty());
    }
    {
        Show s;
        auto* sk = As<DmxSkull>(s.Add(Dmx("DmxSkull", "DmxChannelCount=\"26\" HasColor=\"1\" DmxEyeBrtChannel=\"40\"")));
        FakePreview p;
        DrawEffect(sk, p);
        CHECK(IsInvalidCross(p.SolidAcc()));
    }
}

TEST_CASE("Skull mesh-only draws wireframes", "[dmx2]") {
    CubeFiles cube;
    Show s;
    auto* sk = As<DmxSkull>(s.Add(Dmx("DmxSkull", "DmxChannelCount=\"26\" MeshOnly=\"1\"", SkullMeshes(cube))));
    FakePreview p;
    DrawLayout(sk, p, nullptr, true);
    REQUIRE(p.ctx.solids.size() == 4);
    for (auto& d : p.ctx.solids) {
        CHECK(d.wireframe);
    }
    CHECK(p.ctx.transparents.empty());
}
