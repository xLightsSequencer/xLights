#include <cmath>
#include <string>
#include <vector>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "../support/TempDir.h"
#include "render/ColorCurve.h"
#include "render/ValueCurve.h"
#include "utils/Color.h"

namespace {

std::string Cycle(const std::string& s, int times = 1) {
    std::string cur = s;
    for (int i = 0; i < times; ++i) {
        ValueCurve vc(cur);
        cur = vc.Serialise();
    }
    return cur;
}

ValueCurve Make(const std::string& type, float min, float max, float p1, float p2, float p3 = 0, float p4 = 0, bool wrap = false, float divisor = 1.0f) {
    ValueCurve vc("ID_VALUECURVE_Test", min, max, type, p1, p2, p3, p4, wrap, divisor);
    vc.SetActive(true);
    return vc;
}

const std::vector<std::string>& Types() {
    static const std::vector<std::string> t = {
        "Flat", "Ramp", "Ramp Up/Down", "Ramp Up/Down Hold", "Saw Tooth", "Square", "Parabolic Down", "Parabolic Up",
        "Logarithmic Up", "Logarithmic Down", "Exponential Up", "Exponential Down", "Sine", "Decaying Sine", "Abs Sine"
    };
    return t;
}

struct Range {
    float min, max;
};
const std::vector<Range>& Ranges() {
    static const std::vector<Range> r = { { 0, 100 }, { 0, 1 }, { -100, 100 }, { 0, 255 }, { -50, 150 }, { 0, 360 }, { 10, 20 }, { -1000, 1000 }, { 0, 5000 } };
    return r;
}

void CheckValueCurveStable(const ValueCurve& vcIn) {
    ValueCurve vc = vcIn;
    std::string s1 = vc.Serialise();
    ValueCurve b(s1);
    std::string s2 = b.Serialise();
    CHECK(s2 == s1);
    CHECK(Cycle(s2, 5) == s2);
    CHECK(b.GetType() == vc.GetType());
    CHECK(b.IsActive() == vc.IsActive());
    CHECK(b.GetId() == vc.GetId());
}

} // namespace

#define VC_TYPE_CASE(IDX, LABEL)                                                                                   \
    TEST_CASE("Roundtrip value curve type is stable across ranges: " LABEL, "[roundtrip][curves]") {              \
        const std::string& type = Types()[IDX];                                                                    \
        for (auto& r : Ranges()) {                                                                                 \
            INFO(type << " " << r.min << ".." << r.max);                                                           \
            float span = r.max - r.min;                                                                            \
            ValueCurve vc = Make(type, r.min, r.max, r.min + span * 0.25f, r.min + span * 0.75f, r.min + span * 0.1f, r.min + span * 0.9f); \
            CheckValueCurveStable(vc);                                                                             \
            ValueCurve w = Make(type, r.min, r.max, r.min + span * 0.25f, r.min + span * 0.75f, r.min + span * 0.1f, r.min + span * 0.9f, true); \
            CheckValueCurveStable(w);                                                                              \
        }                                                                                                          \
    }                                                                                                              \
    TEST_CASE("Roundtrip value curve parameters survive: " LABEL, "[roundtrip][curves]") {                        \
        const std::string& type = Types()[IDX];                                                                    \
        for (auto& r : Ranges()) {                                                                                 \
            INFO(type << " " << r.min << ".." << r.max);                                                           \
            ValueCurve vc = Make(type, r.min, r.max, 20, 60, 3, 40);                                              \
            ValueCurve b(vc.Serialise());                                                                          \
            CHECK(b.GetMin() == Catch::Approx(vc.GetMin()).margin(0.006));                                         \
            CHECK(b.GetMax() == Catch::Approx(vc.GetMax()).margin(0.006));                                         \
            CHECK(b.GetParameter1() == Catch::Approx(vc.GetParameter1()).margin(0.006));                           \
            CHECK(b.GetParameter2() == Catch::Approx(vc.GetParameter2()).margin(0.006));                           \
            CHECK(b.GetParameter3() == Catch::Approx(vc.GetParameter3()).margin(0.006));                           \
            CHECK(b.GetParameter4() == Catch::Approx(vc.GetParameter4()).margin(0.006));                           \
            CHECK(b.GetWrap() == vc.GetWrap());                                                                    \
            for (int i = 0; i <= 10; ++i) {                                                                        \
                CHECK(b.GetValueAt(i / 10.0f, 0, 1000) == Catch::Approx(vc.GetValueAt(i / 10.0f, 0, 1000)).margin(0.01 + 0.0001 * (r.max - r.min))); \
            }                                                                                                      \
        }                                                                                                          \
    }

VC_TYPE_CASE(0, "Flat")
VC_TYPE_CASE(1, "Ramp")
VC_TYPE_CASE(2, "Ramp Up/Down")
VC_TYPE_CASE(3, "Ramp Up/Down Hold")
VC_TYPE_CASE(4, "Saw Tooth")
VC_TYPE_CASE(5, "Square")
VC_TYPE_CASE(6, "Parabolic Down")
VC_TYPE_CASE(7, "Parabolic Up")
VC_TYPE_CASE(8, "Logarithmic Up")
VC_TYPE_CASE(9, "Logarithmic Down")
VC_TYPE_CASE(10, "Exponential Up")
VC_TYPE_CASE(11, "Exponential Down")
VC_TYPE_CASE(12, "Sine")
VC_TYPE_CASE(13, "Decaying Sine")
VC_TYPE_CASE(14, "Abs Sine")

TEST_CASE("Roundtrip value curve with divisor settles", "[roundtrip][curves]") {
    for (float div : { 1.0f, 10.0f, 100.0f }) {
        INFO(div);
        ValueCurve vc = Make("Ramp", 0, 100, 10, 90, 0, 0, false, div);
        std::string s1 = vc.Serialise();
        ValueCurve b(s1);
        CHECK(b.Serialise() == s1);
    }
}
TEST_CASE("Roundtrip value curve start end level", "[roundtrip][curves]") {
    ValueCurve vc = Make("Sine", 0, 100, 10, 50, 20, 80);
    vc.SetStartEndLevelActive(true);
    std::string s1 = vc.Serialise();
    CHECK(s1.find("SE=Y|") != std::string::npos);
    ValueCurve b(s1);
    CHECK(b.IsStartEndLevelActive());
    CHECK(b.GetParameter3() == Catch::Approx(20.0f).margin(0.01));
    CHECK(b.GetParameter4() == Catch::Approx(80.0f).margin(0.01));
    CHECK(b.Serialise() == s1);
}
TEST_CASE("Roundtrip value curve zero parameters are not written but survive", "[roundtrip][curves]") {
    ValueCurve vc = Make("Ramp", 0, 100, 0, 0);
    std::string s1 = vc.Serialise();
    ValueCurve b(s1);
    CHECK(b.GetParameter1() == 0.0f);
    CHECK(b.GetParameter2() == 0.0f);
    CHECK(b.Serialise() == s1);
}
TEST_CASE("Roundtrip value curve time offset and audio track", "[roundtrip][curves]") {
    for (int to : { 0, 1, -1, 50, -50, 100, 1000 }) {
        INFO(to);
        ValueCurve vc = Make("Sine", 0, 100, 10, 50, 0, 0);
        vc.SetTimeOffset(to);
        vc.SetAudioTrack("Drums");
        std::string s1 = vc.Serialise();
        ValueCurve b(s1);
        CHECK(b.GetTimeOffset() == to);
        CHECK(b.GetAudioTrack() == "Drums");
        CHECK(b.Serialise() == s1);
    }
}
TEST_CASE("Roundtrip value curve timing track names", "[roundtrip][curves]") {
    for (const char* name : { "Beats", "Beat Track 2", "uni\xC3\xA9", "a,b", "a;b", "a:b", "x y z", "100%", "<tag>", "a&b", "quote\"s", "new\nline", "tab\t" }) {
        INFO(name);
        ValueCurve vc = Make("Ramp", 0, 100, 10, 90);
        vc.SetTimingTrack(name);
        vc.SetFilterLabelText(name);
        std::string s1 = vc.Serialise();
        ValueCurve b(s1);
        CHECK(b.GetTimingTrack() == name);
        CHECK(b.GetFilterLabelText() == name);
        CHECK(b.Serialise() == s1);
    }
}
TEST_CASE("Roundtrip value curve timing track name containing a pipe", "[roundtrip][curves]") {
    ValueCurve vc = Make("Ramp", 0, 100, 10, 90);
    vc.SetTimingTrack("Beats|1");
    ValueCurve b(vc.Serialise());
    CHECK(b.GetTimingTrack() == "Beats|1");
    CHECK(b.GetParameter1() == Catch::Approx(10.0f).margin(0.01));
}
TEST_CASE("Roundtrip value curve filter text containing an equals sign", "[roundtrip][curves]") {
    ValueCurve vc = Make("Ramp", 0, 100, 10, 90);
    vc.SetFilterLabelText("a=b");
    ValueCurve b(vc.Serialise());
    CHECK(b.GetFilterLabelText() == "a=b");
}
TEST_CASE("Roundtrip value curve audio track containing a pipe", "[roundtrip][curves]") {
    ValueCurve vc = Make("Ramp", 0, 100, 10, 90);
    vc.SetAudioTrack("Drums|Kick");
    ValueCurve b(vc.Serialise());
    CHECK(b.GetAudioTrack() == "Drums|Kick");
}
TEST_CASE("Roundtrip value curve regex flag with timing track", "[roundtrip][curves]") {
    ValueCurve vc = Make("Ramp", 0, 100, 10, 90);
    vc.SetTimingTrack("Words");
    vc.SetFilterLabelText("^a.*b$");
    vc.SetFilterLabelRegex(true);
    ValueCurve b(vc.Serialise());
    CHECK(b.IsFilterLabelRegex());
    CHECK(b.GetFilterLabelText() == "^a.*b$");
}
TEST_CASE("Roundtrip value curve inactive stays inactive", "[roundtrip][curves]") {
    ValueCurve vc = Make("Ramp", 0, 100, 10, 90);
    vc.SetActive(false);
    std::string s1 = vc.Serialise();
    CHECK(s1 == "Active=FALSE|");
    ValueCurve b(s1);
    CHECK_FALSE(b.IsActive());
    CHECK(b.Serialise() == s1);
}
TEST_CASE("Roundtrip value curve custom points", "[roundtrip][curves]") {
    for (int n : { 2, 3, 5, 10, 25, 100 }) {
        INFO(n);
        ValueCurve vc("ID_VALUECURVE_Custom", 0, 100, "Custom");
        vc.SetActive(true);
        for (int i = 0; i < n; ++i) {
            vc.SetValueAt(i / (float)(n - 1), (float)((i * 37) % 100) / 100.0f, true);
        }
        std::string s1 = vc.Serialise();
        ValueCurve b(s1);
        CHECK(b.GetType() == "Custom");
        CHECK(b.Serialise() == s1);
        CHECK(Cycle(s1, 4) == s1);
        CHECK(b.GetPointCount() == vc.GetPointCount());
        auto p1 = vc.GetPoints();
        auto p2 = b.GetPoints();
        REQUIRE(p1.size() == p2.size());
        auto i1 = p1.begin();
        auto i2 = p2.begin();
        for (; i1 != p1.end(); ++i1, ++i2) {
            CHECK(i1->x == Catch::Approx(i2->x).margin(0.006));
            CHECK(i1->y == Catch::Approx(i2->y).margin(0.006));
        }
    }
}
TEST_CASE("Roundtrip value curve custom with real range", "[roundtrip][curves]") {
    for (auto& r : Ranges()) {
        INFO(r.min << ".." << r.max);
        ValueCurve vc("ID_VALUECURVE_Custom", r.min, r.max, "Custom");
        vc.SetActive(true);
        vc.SetValueAt(0.0f, 0.1f, true);
        vc.SetValueAt(0.5f, 0.9f, true);
        vc.SetValueAt(1.0f, 0.4f, true);
        std::string s1 = vc.Serialise();
        ValueCurve b(s1);
        CHECK(b.Serialise() == s1);
        CHECK(b.GetMin() == Catch::Approx(vc.GetMin()).margin(0.006));
        CHECK(b.GetMax() == Catch::Approx(vc.GetMax()).margin(0.006));
        for (int i = 0; i <= 10; ++i) {
            CHECK(b.GetValueAt(i / 10.0f, 0, 1000) == Catch::Approx(vc.GetValueAt(i / 10.0f, 0, 1000)).margin(0.01 + 0.0001 * (r.max - r.min)));
        }
    }
}
TEST_CASE("Roundtrip value curve random keeps its points", "[roundtrip][curves]") {
    ValueCurve vc("ID_VALUECURVE_Random", 0, 100, "Random", 10, 90, 5);
    vc.SetActive(true);
    std::string s1 = vc.Serialise();
    ValueCurve b(s1);
    CHECK(b.Serialise() == s1);
    CHECK(Cycle(s1, 4) == s1);
}
TEST_CASE("Roundtrip value curve Min Max precision", "[roundtrip][curves]") {
    for (float v : { 0.01f, 0.05f, 0.125f, 0.5f, 1.5f, 33.33f, 99.99f }) {
        INFO(v);
        ValueCurve vc = Make("Ramp", 0, v, 0, v);
        std::string s1 = vc.Serialise();
        ValueCurve b(s1);
        CHECK(b.GetMax() == Catch::Approx(v).margin(0.006));
        CHECK(b.Serialise() == s1);
    }
}
TEST_CASE("Roundtrip value curve tiny ranges keep their limits", "[roundtrip][curves]") {
    ValueCurve vc = Make("Ramp", 0, 0.004f, 0, 0.004f);
    ValueCurve b(vc.Serialise());
    CHECK(b.GetMax() == Catch::Approx(0.004f).margin(1e-6));
}
TEST_CASE("Roundtrip value curve extreme parameter values settle", "[roundtrip][curves]") {
    for (float v : { 0.0f, -1.0f, 1e3f, 1e6f, -1e6f, 123456.789f }) {
        INFO(v);
        ValueCurve vc("ID_VALUECURVE_X", -1e6f, 1e6f, "Sine", v, v, v, v);
        vc.SetActive(true);
        std::string s1 = vc.Serialise();
        ValueCurve b(s1);
        std::string s2 = b.Serialise();
        CHECK(s2 == s1);
        CHECK(Cycle(s2, 5) == s2);
    }
}
TEST_CASE("Roundtrip value curve malformed strings settle after one cycle", "[roundtrip][curves]") {
    for (const char* s : { "Active=TRUE|", "Active=TRUE|Id=|", "Active=TRUE|Id=X|Min=abc|Max=def|", "Active=TRUE|Id=X|Type=Bogus|Min=0|Max=100|P1=zzz|", "Id=X|Min=0|Max=100|", "|||", "=|=|", "Active=TRUE|Id=X|Min=0|Max=100|Values=1:2;3|", "Active=TRUE|Id=X|Type=Custom|Min=0|Max=100|Values=|", "Active=TRUE|Id=X|Type=Custom|Min=0|Max=100|Values=0.00:0.50;1.00:abc|", "Active=TRUE|Id=X|Min=0|Max=100|P1=1e999|", "Active=TRUE|Id=X|Min=nan|Max=inf|" }) {
        INFO(s);
        ValueCurve a(s);
        std::string s1 = a.Serialise();
        ValueCurve b(s1);
        std::string s2 = b.Serialise();
        CHECK(Cycle(s2, 3) == s2);
    }
}
TEST_CASE("Roundtrip value curve id only strings stay ids", "[roundtrip][curves]") {
    for (const char* id : { "ID_VALUECURVE_Brightness", "x", "with space", "uni\xC3\xA9" }) {
        INFO(id);
        ValueCurve a(id);
        CHECK(a.GetId() == id);
        CHECK(a.Serialise() == "Active=FALSE|");
    }
}
TEST_CASE("Roundtrip value curve reverse and flip twice restores serialisation", "[roundtrip][curves]") {
    for (const auto& type : Types()) {
        if (type == "Sine" || type == "Abs Sine") continue;
        INFO(type);
        ValueCurve vc = Make(type, 0, 100, 20, 60, 3, 40);
        std::string s1 = vc.Serialise();
        vc.Reverse();
        vc.Reverse();
        CHECK(vc.Serialise() == s1);
        vc.Flip();
        vc.Flip();
        CHECK(vc.Serialise() == s1);
    }
}
TEST_CASE("Roundtrip value curve reverse twice on sine restores serialisation", "[roundtrip][curves]") {
    for (const char* type : { "Sine", "Abs Sine" }) {
        INFO(type);
        ValueCurve vc = Make(type, 0, 100, 20, 60, 3, 40);
        std::string s1 = vc.Serialise();
        vc.Reverse();
        vc.Reverse();
        CHECK(vc.Serialise() == s1);
    }
}
TEST_CASE("Roundtrip value curve custom reverse twice restores points", "[roundtrip][curves]") {
    ValueCurve vc("ID_VALUECURVE_Custom", 0, 100, "Custom");
    vc.SetActive(true);
    vc.SetValueAt(0.0f, 0.1f, true);
    vc.SetValueAt(0.25f, 0.7f, true);
    vc.SetValueAt(1.0f, 0.4f, true);
    std::string s1 = vc.Serialise();
    vc.Reverse();
    vc.Reverse();
    CHECK(vc.Serialise() == s1);
    vc.Flip();
    vc.Flip();
    CHECK(vc.Serialise() == s1);
}
TEST_CASE("Roundtrip value curve divider conversion round trip", "[roundtrip][curves]") {
    ValueCurve vc = Make("Ramp", 0, 100, 25, 75);
    float p1 = vc.GetParameter1();
    float p2 = vc.GetParameter2();
    for (int i = 0; i < 5; ++i) {
        vc.ConvertDivider(1, 10);
        vc.ConvertDivider(10, 1);
    }
    CHECK(vc.GetParameter1() == Catch::Approx(p1).margin(0.01));
    CHECK(vc.GetParameter2() == Catch::Approx(p2).margin(0.01));
}
TEST_CASE("Roundtrip value curve deserialise into existing curve resets state", "[roundtrip][curves]") {
    ValueCurve a = Make("Sine", 0, 100, 10, 50, 0, 0, true);
    a.SetTimeOffset(30);
    a.SetTimingTrack("T");
    ValueCurve b("ID_VALUECURVE_B", 0, 100, "Ramp", 1, 2);
    b.SetActive(true);
    ValueCurve plain = Make("Ramp", 0, 100, 5, 95);
    b.Deserialise(plain.Serialise());
    CHECK_FALSE(b.GetWrap());
    CHECK(b.GetTimeOffset() == 0);
    CHECK(b.GetTimingTrack().empty());
    CHECK(b.Serialise() == plain.Serialise());
}

namespace {

// Saves a curve to an .xvc and loads it back the way ValueCurveDialog::LoadXVC does.
void XvcRoundTrip(float mn, float mx, const char* type, bool& okType, ValueCurve& vc, ValueCurve& loaded) {
    TempDir dir;
    vc = Make(type, mn, mx, mn + (mx - mn) * 0.2f, mn + (mx - mn) * 0.8f);
    std::string fn = dir.File("curve.xvc");
    vc.SaveXVC(fn);
    loaded = ValueCurve("ID_VALUECURVE_Test", mn, mx);
    loaded.SetLimits(0, 100);
    loaded.SetDivisor(1);
    loaded.LoadXVC(fn);
    loaded.SetLimits(mn, mx);
    loaded.FixChangedScale(mn, mx, 1);
    loaded.SetId("ID_VALUECURVE_Test");
    okType = loaded.GetType() == vc.GetType();
}

} // namespace

TEST_CASE("Roundtrip value curve xvc file with zero based ranges", "[roundtrip][curves]") {
    for (auto& r : Ranges()) {
        if (r.min != 0) continue;
        for (const char* type : { "Ramp", "Sine", "Square", "Ramp Up/Down" }) {
            INFO(type << " " << r.min << ".." << r.max);
            ValueCurve vc, loaded;
            bool ok = false;
            XvcRoundTrip(r.min, r.max, type, ok, vc, loaded);
            CHECK(ok);
            INFO(vc.Serialise() << " vs " << loaded.Serialise());
            CHECK(loaded.GetParameter1() == Catch::Approx(vc.GetParameter1()).margin(0.01 + 0.0002 * (r.max - r.min)));
            CHECK(loaded.GetParameter2() == Catch::Approx(vc.GetParameter2()).margin(0.01 + 0.0002 * (r.max - r.min)));
        }
    }
}
TEST_CASE("Roundtrip value curve xvc file with offset ranges", "[roundtrip][curves]") {
    for (auto& r : Ranges()) {
        if (r.min == 0) continue;
        for (const char* type : { "Ramp", "Sine" }) {
            INFO(type << " " << r.min << ".." << r.max);
            ValueCurve vc, loaded;
            bool ok = false;
            XvcRoundTrip(r.min, r.max, type, ok, vc, loaded);
            CHECK(ok);
            INFO(vc.Serialise() << " vs " << loaded.Serialise());
            CHECK(loaded.GetParameter1() == Catch::Approx(vc.GetParameter1()).margin(0.01 + 0.0002 * (r.max - r.min)));
            CHECK(loaded.GetParameter2() == Catch::Approx(vc.GetParameter2()).margin(0.01 + 0.0002 * (r.max - r.min)));
        }
    }
}

// ---- colour curves -----------------------------------------------------------------------------------------

namespace {

ColorCurve MakeCC(const std::string& type, std::vector<std::pair<float, xlColor>> pts) {
    ColorCurve cc("ID_COLORCURVE_Test", type, xlBLACK);
    for (auto& p : pts) cc.SetValueAt(p.first, p.second);
    cc.SetActive(true);
    return cc;
}

std::string CCCycle(const std::string& s, int times) {
    std::string cur = s;
    for (int i = 0; i < times; ++i) {
        ColorCurve c(cur);
        cur = c.Serialise();
    }
    return cur;
}

} // namespace

TEST_CASE("Roundtrip color curve types are stable", "[roundtrip][curves][colorcurve]") {
    for (const char* type : { "Gradient", "None", "Random", "Bogus" }) {
        INFO(type);
        ColorCurve cc = MakeCC(type, { { 0.0f, xlRED }, { 0.25f, xlGREEN }, { 0.5f, xlBLUE }, { 1.0f, xlWHITE } });
        std::string s1 = cc.Serialise();
        ColorCurve b(s1);
        CHECK(b.Serialise() == s1);
        CHECK(CCCycle(s1, 5) == s1);
        CHECK(b.GetType() == cc.GetType());
        CHECK(b.GetPointCount() == cc.GetPointCount());
    }
}
TEST_CASE("Roundtrip color curve points keep colour and position", "[roundtrip][curves][colorcurve]") {
    std::vector<std::pair<float, xlColor>> pts;
    for (int i = 0; i <= 20; ++i) pts.emplace_back(i / 20.0f, xlColor((i * 13) & 255, (i * 59) & 255, (i * 101) & 255));
    ColorCurve cc = MakeCC("Gradient", pts);
    ColorCurve b(cc.Serialise());
    auto p1 = cc.GetPoints();
    auto p2 = b.GetPoints();
    REQUIRE(p1.size() == p2.size());
    auto i1 = p1.begin();
    auto i2 = p2.begin();
    for (; i1 != p1.end(); ++i1, ++i2) {
        CHECK(i1->x == Catch::Approx(i2->x).margin(0.006));
        CHECK(i1->color.Red() == i2->color.Red());
        CHECK(i1->color.Green() == i2->color.Green());
        CHECK(i1->color.Blue() == i2->color.Blue());
    }
}
TEST_CASE("Roundtrip color curve every time curve mode", "[roundtrip][curves][colorcurve]") {
    for (int tc = 0; tc <= 8; ++tc) {
        INFO(tc);
        std::string s = "Active=TRUE|Id=ID_COLORCURVE_T|Timecurve=" + std::to_string(tc) + "|Values=x=0.000^c=#FF0000;x=1.000^c=#0000FF;|";
        ColorCurve a(s);
        std::string s1 = a.Serialise();
        ColorCurve b(s1);
        CHECK(b.Serialise() == s1);
        CHECK(b.GetTimeCurve() == a.GetTimeCurve());
    }
}
TEST_CASE("Roundtrip color curve colours with alpha keep rgb", "[roundtrip][curves][colorcurve]") {
    ColorCurve cc = MakeCC("Gradient", { { 0.0f, xlColor(10, 20, 30, 40) }, { 1.0f, xlColor(200, 210, 220, 230) } });
    std::string s1 = cc.Serialise();
    ColorCurve b(s1);
    CHECK(b.Serialise() == s1);
    auto pts = b.GetPoints();
    REQUIRE(pts.size() == cc.GetPointCount());
    CHECK(pts.front().color.Red() == 10);
    CHECK(pts.back().color.Blue() == 220);
}
TEST_CASE("Roundtrip color curve inactive stays inactive", "[roundtrip][curves][colorcurve]") {
    ColorCurve cc = MakeCC("Gradient", { { 0.0f, xlRED } });
    cc.SetActive(false);
    std::string s1 = cc.Serialise();
    CHECK(s1 == "Active=FALSE|");
    ColorCurve b(s1);
    CHECK(b.Serialise() == s1);
}
TEST_CASE("Roundtrip color curve single point", "[roundtrip][curves][colorcurve]") {
    ColorCurve cc = MakeCC("Gradient", { { 0.0f, xlColor(1, 2, 3) } });
    std::string s1 = cc.Serialise();
    ColorCurve b(s1);
    CHECK(b.Serialise() == s1);
}
TEST_CASE("Roundtrip color curve points that snap to the same position", "[roundtrip][curves][colorcurve]") {
    for (const char* x : { "2.0", "0.996" }) {
        INFO(x);
        ColorCurve a(std::string("Active=TRUE|Id=ID_COLORCURVE_T|Values=x=") + x + "^c=#FF0000;x=1.000^c=#0000FF;|");
        std::string s1 = a.Serialise();
        CHECK(CCCycle(s1, 3) == s1);
    }
}
TEST_CASE("Roundtrip color curve positions beyond range settle", "[roundtrip][curves][colorcurve]") {
    for (const char* x : { "-1.0", "0.004", "abc", "" }) {
        INFO(x);
        std::string s = std::string("Active=TRUE|Id=ID_COLORCURVE_T|Values=x=") + x + "^c=#FF0000;x=1.000^c=#0000FF;|";
        ColorCurve a(s);
        std::string s1 = a.Serialise();
        CHECK(CCCycle(s1, 4) == s1);
    }
}
TEST_CASE("Roundtrip color curve malformed colours settle", "[roundtrip][curves][colorcurve]") {
    for (const char* c : { "#GGGGGG", "#FFF", "", "red", "#FF00", "#FF0000FF", "1@2@3", "255@0@0@128", "-1@0@0" }) {
        INFO(c);
        std::string s = std::string("Active=TRUE|Id=ID_COLORCURVE_T|Values=x=0.000^c=") + c + ";x=1.000^c=#0000FF;|";
        ColorCurve a(s);
        std::string s1 = a.Serialise();
        CHECK(CCCycle(s1, 4) == s1);
    }
}
TEST_CASE("Roundtrip color curve flip twice restores", "[roundtrip][curves][colorcurve]") {
    ColorCurve cc = MakeCC("Gradient", { { 0.0f, xlRED }, { 0.3f, xlGREEN }, { 1.0f, xlBLUE } });
    std::string s1 = cc.Serialise();
    cc.Flip();
    cc.Flip();
    CHECK(cc.Serialise() == s1);
}
TEST_CASE("Roundtrip color curve IsColorCurve agrees with Serialise", "[roundtrip][curves][colorcurve]") {
    ColorCurve cc = MakeCC("Gradient", { { 0.0f, xlRED }, { 1.0f, xlBLUE } });
    CHECK(ColorCurve::IsColorCurve(cc.Serialise()));
    ColorCurve off = cc;
    off.SetActive(false);
    CHECK(ColorCurve::IsColorCurve(off.Serialise()));
}
