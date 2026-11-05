#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "render/ValueCurve.h"

#include <string>
#include <vector>

using Catch::Matchers::WithinAbs;

namespace {
ValueCurve MakeActive(const std::string& type, float min, float max, float p1 = 0, float p2 = 0, float p3 = 0, float p4 = 0, bool wrap = false) {
    ValueCurve vc("ID_VC_TEST", min, max, type, p1, p2, p3, p4, wrap);
    vc.SetActive(true);
    return vc;
}

float At(ValueCurve& vc, float offset) {
    return vc.GetValueAt(offset, 0, 1000);
}
}

TEST_CASE("ValueCurve default constructed is not ok and not active", "[curves][valuecurve]") {
    ValueCurve vc;
    CHECK_FALSE(vc.IsOk());
    CHECK_FALSE(vc.IsActive());
    CHECK(vc.GetType() == "Flat");
}

TEST_CASE("ValueCurve empty string deserialises to inactive default", "[curves][valuecurve]") {
    ValueCurve vc("");
    CHECK_FALSE(vc.IsActive());
    CHECK_FALSE(vc.IsOk());
    CHECK(vc.Serialise() == "Active=FALSE|");
}

TEST_CASE("ValueCurve string without separators is treated as an id only", "[curves][valuecurve]") {
    ValueCurve vc("ID_VALUECURVE_Brightness");
    CHECK(vc.GetId() == "ID_VALUECURVE_Brightness");
    CHECK(vc.IsOk());
    CHECK_FALSE(vc.IsActive());
    CHECK(vc.Serialise() == "Active=FALSE|");
}

TEST_CASE("ValueCurve inactive curve serialises as Active=FALSE", "[curves][valuecurve]") {
    ValueCurve vc("ID_X", 0, 100, "Ramp", 10, 20);
    CHECK(vc.Serialise() == "Active=FALSE|");
    vc.SetActive(true);
    CHECK(vc.Serialise() != "Active=FALSE|");
    vc.ToggleActive();
    CHECK(vc.Serialise() == "Active=FALSE|");
}

TEST_CASE("ValueCurve Flat serialises without Type and with parameters", "[curves][valuecurve]") {
    auto vc = MakeActive("Flat", 0, 100, 40);
    CHECK(vc.Serialise() == "Active=TRUE|Id=ID_VC_TEST|Min=0.00|Max=100.00|P1=40.00|RV=TRUE|");
}

TEST_CASE("ValueCurve Ramp serialises type and both parameters", "[curves][valuecurve]") {
    auto vc = MakeActive("Ramp", 0, 100, 10, 90);
    CHECK(vc.Serialise() == "Active=TRUE|Id=ID_VC_TEST|Type=Ramp|Min=0.00|Max=100.00|P1=10.00|P2=90.00|RV=TRUE|");
}

TEST_CASE("ValueCurve Flat value at any time is the parameter", "[curves][valuecurve]") {
    auto vc = MakeActive("Flat", 0, 100, 40);
    for (float o : { 0.0f, 0.25f, 0.5f, 1.0f }) {
        CHECK_THAT(At(vc, o), WithinAbs(0.4, 1e-4));
    }
    CHECK_THAT(vc.GetOutputValueAt(0.3f, 0, 1000), WithinAbs(40.0, 1e-3));
}

TEST_CASE("ValueCurve Flat with negative range normalises", "[curves][valuecurve]") {
    auto vc = MakeActive("Flat", -100, 100, 0);
    CHECK_THAT(At(vc, 0.5f), WithinAbs(0.5, 1e-4));
    CHECK_THAT(vc.GetOutputValueAt(0.5f, 0, 1000), WithinAbs(0.0, 1e-3));
}

TEST_CASE("ValueCurve inactive curve evaluates to 1", "[curves][valuecurve]") {
    ValueCurve vc("ID_X", 0, 100, "Flat", 20);
    CHECK_THAT(At(vc, 0.5f), WithinAbs(1.0, 1e-6));
}

TEST_CASE("ValueCurve Ramp interpolates linearly", "[curves][valuecurve]") {
    auto vc = MakeActive("Ramp", 0, 100, 0, 100);
    CHECK_THAT(At(vc, 0.0f), WithinAbs(0.0, 1e-4));
    CHECK_THAT(At(vc, 0.25f), WithinAbs(0.25, 1e-4));
    CHECK_THAT(At(vc, 0.5f), WithinAbs(0.5, 1e-4));
    CHECK_THAT(At(vc, 1.0f), WithinAbs(1.0, 1e-4));
}

TEST_CASE("ValueCurve Ramp down interpolates", "[curves][valuecurve]") {
    auto vc = MakeActive("Ramp", 0, 100, 100, 0);
    CHECK_THAT(At(vc, 0.0f), WithinAbs(1.0, 1e-4));
    CHECK_THAT(At(vc, 0.75f), WithinAbs(0.25, 1e-4));
}

TEST_CASE("ValueCurve offsets outside 0..1 are clamped", "[curves][valuecurve]") {
    auto vc = MakeActive("Ramp", 0, 100, 10, 90);
    CHECK_THAT(At(vc, -5.0f), WithinAbs(0.1, 1e-4));
    CHECK_THAT(At(vc, 5.0f), WithinAbs(0.9, 1e-4));
}

TEST_CASE("ValueCurve Ramp Up/Down peaks in the middle", "[curves][valuecurve]") {
    auto vc = MakeActive("Ramp Up/Down", 0, 100, 0, 100, 0);
    CHECK_THAT(At(vc, 0.0f), WithinAbs(0.0, 1e-4));
    CHECK_THAT(At(vc, 0.5f), WithinAbs(1.0, 1e-4));
    CHECK_THAT(At(vc, 0.25f), WithinAbs(0.5, 1e-4));
    CHECK_THAT(At(vc, 1.0f), WithinAbs(0.0, 1e-4));
}

TEST_CASE("ValueCurve Ramp Up/Down Hold holds the peak", "[curves][valuecurve]") {
    auto vc = MakeActive("Ramp Up/Down Hold", 0, 100, 0, 100, 50);
    CHECK_THAT(At(vc, 0.0f), WithinAbs(0.0, 1e-4));
    CHECK_THAT(At(vc, 0.5f), WithinAbs(1.0, 1e-4));
    CHECK_THAT(At(vc, 0.3f), WithinAbs(1.0, 1e-4));
    CHECK_THAT(At(vc, 0.7f), WithinAbs(1.0, 1e-4));
    CHECK_THAT(At(vc, 1.0f), WithinAbs(0.0, 1e-4));
}

TEST_CASE("ValueCurve Saw Tooth repeats", "[curves][valuecurve]") {
    auto vc = MakeActive("Saw Tooth", 0, 100, 0, 100, 2);
    CHECK_THAT(At(vc, 0.0f), WithinAbs(0.0, 1e-4));
    CHECK_THAT(At(vc, 0.25f), WithinAbs(1.0, 1e-4));
    CHECK_THAT(At(vc, 0.5f), WithinAbs(0.0, 1e-4));
    CHECK_THAT(At(vc, 0.75f), WithinAbs(1.0, 1e-4));
    CHECK_THAT(At(vc, 1.0f), WithinAbs(0.0, 1e-4));
}

TEST_CASE("ValueCurve Square alternates between parameters", "[curves][valuecurve]") {
    auto vc = MakeActive("Square", 0, 100, 0, 100, 2);
    CHECK_THAT(At(vc, 0.05f), WithinAbs(0.0, 1e-4));
    CHECK_THAT(At(vc, 0.3f), WithinAbs(1.0, 1e-4));
    CHECK_THAT(At(vc, 0.55f), WithinAbs(0.0, 1e-4));
    CHECK_THAT(At(vc, 0.8f), WithinAbs(1.0, 1e-4));
}

TEST_CASE("ValueCurve Sine stays within unit range", "[curves][valuecurve]") {
    auto vc = MakeActive("Sine", 0, 100, 0, 100, 10, 50);
    for (int i = 0; i <= 100; ++i) {
        float v = At(vc, i / 100.0f);
        CHECK(v >= 0.0f);
        CHECK(v <= 1.0f);
    }
}

TEST_CASE("ValueCurve Abs Sine is never negative and not constant", "[curves][valuecurve]") {
    auto vc = MakeActive("Abs Sine", 0, 100, 0, 100, 10, 50);
    float lo = 2.0f, hi = -1.0f;
    for (int i = 0; i <= 200; ++i) {
        float v = At(vc, i / 200.0f);
        lo = std::min(lo, v);
        hi = std::max(hi, v);
    }
    CHECK(lo >= 0.0f);
    CHECK(hi > 0.5f);
    CHECK(hi > lo);
}

TEST_CASE("ValueCurve Decaying Sine decays over time", "[curves][valuecurve]") {
    auto vc = MakeActive("Decaying Sine", 0, 100, 0, 50, 10, 50);
    float earlyMax = 0, lateMax = 0;
    for (int i = 0; i <= 25; ++i) {
        earlyMax = std::max(earlyMax, std::abs(At(vc, i / 100.0f) - 0.5f));
    }
    for (int i = 75; i <= 100; ++i) {
        lateMax = std::max(lateMax, std::abs(At(vc, i / 100.0f) - 0.5f));
    }
    CHECK(earlyMax >= lateMax);
}

TEST_CASE("ValueCurve Parabolic Up and Down are mirror shapes", "[curves][valuecurve]") {
    auto down = MakeActive("Parabolic Down", 0, 100, 20, 50);
    auto up = MakeActive("Parabolic Up", 0, 100, 20, 50);
    float dm = At(down, 0.5f);
    float um = At(up, 0.5f);
    CHECK(um >= dm);
    CHECK_THAT(dm, WithinAbs(0.5, 1e-3));
    CHECK_THAT(um, WithinAbs(0.5, 1e-3));
    CHECK(At(down, 0.0f) > dm);
    CHECK(At(up, 0.0f) < um);
}

TEST_CASE("ValueCurve Exponential and Logarithmic curves are monotonic", "[curves][valuecurve]") {
    for (const char* type : { "Exponential Up", "Logarithmic Up" }) {
        auto vc = MakeActive(type, 0, 100, 50, 50, 0, 100);
        float prev = -1.0f;
        for (int i = 0; i <= 40; ++i) {
            float v = At(vc, i / 40.0f);
            CHECK(v + 1e-4f >= prev);
            prev = v;
        }
    }
    for (const char* type : { "Exponential Down", "Logarithmic Down" }) {
        auto vc = MakeActive(type, 0, 100, 50, 50, 0, 100);
        float prev = 2.0f;
        for (int i = 0; i <= 40; ++i) {
            float v = At(vc, i / 40.0f);
            CHECK(v <= prev + 1e-4f);
            prev = v;
        }
    }
}

TEST_CASE("ValueCurve serialise/deserialise round trip for every parametric type", "[curves][valuecurve]") {
    const std::vector<std::string> types = {
        "Flat", "Ramp", "Ramp Up/Down", "Ramp Up/Down Hold", "Saw Tooth", "Square",
        "Parabolic Down", "Parabolic Up", "Logarithmic Up", "Logarithmic Down",
        "Exponential Up", "Exponential Down", "Sine", "Decaying Sine", "Abs Sine"
    };
    for (const auto& type : types) {
        DYNAMIC_SECTION(type) {
            auto vc = MakeActive(type, 0, 100, 20, 60, 3, 40);
            std::string s = vc.Serialise();
            ValueCurve back(s);
            CHECK(back.IsActive());
            CHECK(back.GetType() == type);
            CHECK(back.GetId() == "ID_VC_TEST");
            CHECK(back.Serialise() == s);
            for (int i = 0; i <= 20; ++i) {
                CHECK_THAT(back.GetValueAt(i / 20.0f, 0, 1000), WithinAbs(vc.GetValueAt(i / 20.0f, 0, 1000), 1e-3));
            }
        }
    }
}

TEST_CASE("ValueCurve round trip preserves limits and parameters", "[curves][valuecurve]") {
    auto vc = MakeActive("Ramp", -50, 150, 10, 120);
    ValueCurve back(vc.Serialise());
    CHECK_THAT(back.GetMin(), WithinAbs(-50.0, 1e-3));
    CHECK_THAT(back.GetMax(), WithinAbs(150.0, 1e-3));
    CHECK_THAT(back.GetParameter1(), WithinAbs(10.0, 1e-3));
    CHECK_THAT(back.GetParameter2(), WithinAbs(120.0, 1e-3));
    CHECK(back.IsRealValue());
}

TEST_CASE("ValueCurve round trip preserves time offset wrap and timing track", "[curves][valuecurve]") {
    ValueCurve vc("ID_X", 0, 100, "Sine", 10, 50, 10, 50, true);
    vc.SetTimeOffset(25);
    vc.SetTimingTrack("Beats");
    vc.SetFilterLabelText("kick");
    vc.SetFilterLabelRegex(true);
    vc.SetAudioTrack("Drums");
    vc.SetActive(true);
    std::string s = vc.Serialise();
    CHECK(s.find("TO=25|") != std::string::npos);
    CHECK(s.find("WRAP=TRUE|") != std::string::npos);
    CHECK(s.find("TT=Beats|") != std::string::npos);
    CHECK(s.find("AT=Drums|") != std::string::npos);
    CHECK(s.find("FT=kick|") != std::string::npos);
    CHECK(s.find("FLR=Y|") != std::string::npos);

    ValueCurve back(s);
    CHECK(back.GetTimeOffset() == 25);
    CHECK(back.GetWrap());
    CHECK(back.GetTimingTrack() == "Beats");
    CHECK(back.GetAudioTrack() == "Drums");
    CHECK(back.GetFilterLabelText() == "kick");
    CHECK(back.IsFilterLabelRegex());
}

TEST_CASE("ValueCurve time offset shifts the curve", "[curves][valuecurve]") {
    auto vc = MakeActive("Ramp", 0, 100, 0, 100);
    vc.SetTimeOffset(50);
    CHECK_THAT(At(vc, 0.0f), WithinAbs(0.5, 1e-3));
    CHECK_THAT(At(vc, 0.25f), WithinAbs(0.75, 1e-3));
}

TEST_CASE("ValueCurve parameters are clamped to their legal range", "[curves][valuecurve]") {
    auto vc = MakeActive("Sine", 0, 100, 0, 50, 10, 50);
    vc.SetParameter1(500);
    CHECK_THAT(vc.GetParameter1(), WithinAbs(100.0, 1e-4));
    vc.SetParameter1(-20);
    CHECK_THAT(vc.GetParameter1(), WithinAbs(0.0, 1e-4));

    auto flat = MakeActive("Flat", 0, 50, 10);
    flat.SetParameter1(80);
    CHECK_THAT(flat.GetParameter1(), WithinAbs(50.0, 1e-4));
    flat.SetParameter1(-5);
    CHECK_THAT(flat.GetParameter1(), WithinAbs(0.0, 1e-4));
}

TEST_CASE("ValueCurve constructor clamps parameters", "[curves][valuecurve]") {
    ValueCurve vc("ID_X", 0, 10, "Ramp", -5, 99);
    CHECK_THAT(vc.GetParameter1(), WithinAbs(0.0, 1e-4));
    CHECK_THAT(vc.GetParameter2(), WithinAbs(10.0, 1e-4));
}

TEST_CASE("ValueCurve GetRangeParm reports per type ranges", "[curves][valuecurve]") {
    float lo = 0, hi = 0;
    ValueCurve::GetRangeParm(1, "Custom", lo, hi);
    CHECK(lo == 1);
    CHECK(hi == 10);
    ValueCurve::GetRangeParm(3, "Random", lo, hi);
    CHECK(lo == 1);
    CHECK(hi == VC_X_POINTS);
    ValueCurve::GetRangeParm(3, "Timing Track Fade Fixed", lo, hi);
    CHECK(lo == 1);
    CHECK(hi == 1000);
    ValueCurve::GetRangeParm(1, "Flat", lo, hi);
    CHECK(lo == MINVOID);
    CHECK(hi == MAXVOID);
    ValueCurve::GetRangeParm(1, "Sine", lo, hi);
    CHECK(lo == 0);
    CHECK(hi == 100);
}

TEST_CASE("ValueCurve Custom points are evaluated by interpolation", "[curves][valuecurve]") {
    ValueCurve vc("ID_X", 0, 100, "Custom", 1);
    vc.SetValueAt(0.0f, 0.0f);
    vc.SetValueAt(0.5f, 1.0f);
    vc.SetValueAt(1.0f, 0.0f);
    vc.SetActive(true);
    CHECK(vc.GetPointCount() == 3);
    CHECK_THAT(At(vc, 0.0f), WithinAbs(0.0, 1e-4));
    CHECK_THAT(At(vc, 0.25f), WithinAbs(0.5, 1e-4));
    CHECK_THAT(At(vc, 0.5f), WithinAbs(1.0, 1e-4));
    CHECK_THAT(At(vc, 0.75f), WithinAbs(0.5, 1e-4));
}

TEST_CASE("ValueCurve Custom round trip keeps points", "[curves][valuecurve]") {
    ValueCurve vc("ID_X", 0, 100, "Custom", 1);
    vc.SetValueAt(0.0f, 0.1f);
    vc.SetValueAt(0.4f, 0.9f);
    vc.SetValueAt(1.0f, 0.3f);
    vc.SetActive(true);
    std::string s = vc.Serialise();
    CHECK(s.find("Type=Custom|") != std::string::npos);
    CHECK(s.find("Values=0.00:0.10;0.40:0.90;1.00:0.30|") != std::string::npos);

    ValueCurve back(s);
    CHECK(back.GetType() == "Custom");
    CHECK(back.GetPointCount() == 3);
    CHECK_THAT(back.GetPointAt(0.4f), WithinAbs(0.9, 1e-3));
    CHECK_THAT(back.GetValueAt(0.2f, 0, 1000), WithinAbs(0.5, 1e-3));
    CHECK(back.Serialise() == s);
}

TEST_CASE("ValueCurve SetValueAt replaces a point at the same x unless forced", "[curves][valuecurve]") {
    ValueCurve vc("ID_X", 0, 100, "Custom", 1);
    vc.SetValueAt(0.0f, 0.0f);
    vc.SetValueAt(1.0f, 1.0f);
    vc.SetValueAt(0.5f, 0.2f);
    vc.SetValueAt(0.5f, 0.7f);
    CHECK(vc.GetPointCount() == 3);
    CHECK_THAT(vc.GetPointAt(0.5f), WithinAbs(0.7, 1e-6));
    CHECK(vc.IsSetPoint(0.5f));
    CHECK_FALSE(vc.IsSetPoint(0.25f));
}

TEST_CASE("ValueCurve DeletePoint never goes below two points", "[curves][valuecurve]") {
    ValueCurve vc("ID_X", 0, 100, "Custom", 1);
    vc.SetValueAt(0.0f, 0.0f);
    vc.SetValueAt(0.5f, 0.5f);
    vc.SetValueAt(1.0f, 1.0f);
    vc.DeletePoint(0.5f);
    CHECK(vc.GetPointCount() == 2);
    vc.DeletePoint(0.0f);
    CHECK(vc.GetPointCount() == 2);
}

TEST_CASE("ValueCurve points snap to the 1/200 grid", "[curves][valuecurve]") {
    vcSortablePoint p(0.3333f, 0.5f, false);
    CHECK_THAT(p.x, WithinAbs(0.335, 1e-6));
    CHECK(vcSortablePoint::Normalise(0.0f) == 0.0f);
    CHECK(vcSortablePoint::Normalise(1.0f) == 1.0f);
    CHECK_THAT(vcSortablePoint::perPoint(), WithinAbs(0.005, 1e-7));
}

TEST_CASE("ValueCurve RemoveExcessCustomPoints drops redundant flat middles", "[curves][valuecurve]") {
    ValueCurve vc("ID_X", 0, 100, "Custom", 1);
    vc.SetValueAt(0.0f, 0.5f);
    vc.SetValueAt(0.25f, 0.5f);
    vc.SetValueAt(0.5f, 0.5f);
    vc.SetValueAt(1.0f, 0.8f);
    vc.RemoveExcessCustomPoints();
    CHECK(vc.GetPointCount() == 3);
    CHECK_FALSE(vc.IsSetPoint(0.25f));
}

TEST_CASE("ValueCurve FindMinPointLessThan and FindMaxPointGreaterThan bound edits", "[curves][valuecurve]") {
    ValueCurve vc("ID_X", 0, 100, "Custom", 1);
    vc.SetValueAt(0.0f, 0.0f);
    vc.SetValueAt(0.2f, 0.5f);
    vc.SetValueAt(0.8f, 0.5f);
    vc.SetValueAt(1.0f, 1.0f);
    CHECK_THAT(vc.FindMinPointLessThan(0.5f), WithinAbs(0.205, 1e-4));
    CHECK_THAT(vc.FindMaxPointGreaterThan(0.5f), WithinAbs(0.795, 1e-4));
    CHECK(vc.NearCustomPoint(0.2f, 0.5f));
    CHECK_FALSE(vc.NearCustomPoint(0.2f, 0.9f));
}

TEST_CASE("ValueCurve Reverse mirrors a ramp", "[curves][valuecurve]") {
    auto vc = MakeActive("Ramp", 0, 100, 10, 90);
    vc.Reverse();
    CHECK_THAT(vc.GetParameter1(), WithinAbs(90.0, 1e-4));
    CHECK_THAT(vc.GetParameter2(), WithinAbs(10.0, 1e-4));
    CHECK_THAT(At(vc, 0.0f), WithinAbs(0.9, 1e-4));
}

TEST_CASE("ValueCurve Reverse on Custom mirrors x", "[curves][valuecurve]") {
    ValueCurve vc("ID_X", 0, 100, "Custom", 1);
    vc.SetValueAt(0.0f, 0.0f);
    vc.SetValueAt(1.0f, 1.0f);
    vc.SetActive(true);
    vc.Reverse();
    CHECK_THAT(At(vc, 0.0f), WithinAbs(1.0, 1e-4));
    CHECK_THAT(At(vc, 1.0f), WithinAbs(0.0, 1e-4));
}

TEST_CASE("ValueCurve Flip inverts values", "[curves][valuecurve]") {
    auto flat = MakeActive("Flat", 0, 100, 30);
    flat.Flip();
    CHECK_THAT(flat.GetParameter1(), WithinAbs(70.0, 1e-4));

    auto ramp = MakeActive("Ramp", 0, 100, 0, 100);
    ramp.Flip();
    CHECK_THAT(ramp.GetParameter1(), WithinAbs(100.0, 1e-4));
    CHECK_THAT(ramp.GetParameter2(), WithinAbs(0.0, 1e-4));

    ValueCurve custom("ID_X", 0, 100, "Custom", 1);
    custom.SetValueAt(0.0f, 0.2f);
    custom.SetValueAt(1.0f, 0.9f);
    custom.Flip();
    CHECK_THAT(custom.GetPointAt(0.0f), WithinAbs(0.8, 1e-5));
    CHECK_THAT(custom.GetPointAt(1.0f), WithinAbs(0.1, 1e-5));
}

TEST_CASE("ValueCurve Flip Parabolic swaps its type", "[curves][valuecurve]") {
    auto vc = MakeActive("Parabolic Down", 0, 100, 20, 30);
    vc.Flip();
    CHECK(vc.GetType() == "Parabolic Up");
    CHECK_THAT(vc.GetParameter2(), WithinAbs(70.0, 1e-4));
    vc.Flip();
    CHECK(vc.GetType() == "Parabolic Down");
}

TEST_CASE("ValueCurve Custom with multiple cycles repeats the shape", "[curves][valuecurve]") {
    ValueCurve vc("ID_X", 0, 100, "Custom", 1);
    vc.SetValueAt(0.0f, 0.0f);
    vc.SetValueAt(1.0f, 1.0f);
    vc.SetActive(true);
    vc.SetParameter1(2);
    CHECK_THAT(At(vc, 0.25f), WithinAbs(0.5, 2e-2));
    CHECK_THAT(At(vc, 0.75f), WithinAbs(0.5, 2e-2));
    CHECK_THAT(At(vc, 0.45f), WithinAbs(0.9, 3e-2));
    CHECK_THAT(At(vc, 0.55f), WithinAbs(0.1, 3e-2));
}

TEST_CASE("ValueCurve Custom cycle count is clamped to 1..10", "[curves][valuecurve]") {
    ValueCurve vc("ID_X", 0, 100, "Custom", 1);
    vc.SetValueAt(0.0f, 0.0f);
    vc.SetValueAt(1.0f, 1.0f);
    vc.SetParameter1(50);
    CHECK_THAT(vc.GetParameter1(), WithinAbs(10.0, 1e-4));
    vc.SetParameter1(0);
    CHECK_THAT(vc.GetParameter1(), WithinAbs(1.0, 1e-4));
}

TEST_CASE("ValueCurve Random keeps its generated points through serialisation", "[curves][valuecurve]") {
    ValueCurve vc("ID_RND", 0, 100, "Random", 0, 100, 10);
    vc.SetActive(true);
    int pts = vc.GetPointCount();
    CHECK(pts == 10);
    std::string s = vc.Serialise();
    ValueCurve back(s);
    CHECK(back.GetPointCount() == pts);
    CHECK(back.Serialise() == s);
    auto a = vc.GetPoints();
    auto b = back.GetPoints();
    REQUIRE(a.size() == b.size());
    auto ia = a.begin();
    auto ib = b.begin();
    for (; ia != a.end(); ++ia, ++ib) {
        CHECK_THAT(ib->x, WithinAbs(ia->x, 0.0051));
        CHECK_THAT(ib->y, WithinAbs(ia->y, 0.0051));
    }
}

TEST_CASE("ValueCurve Random is deterministic for the same identity", "[curves][valuecurve]") {
    ValueCurve a("ID_RND", 0, 100, "Random", 0, 100, 8);
    ValueCurve b("ID_RND", 0, 100, "Random", 0, 100, 8);
    a.SetActive(true);
    b.SetActive(true);
    CHECK(a.Serialise() == b.Serialise());
}

TEST_CASE("ValueCurve Random values stay within min/max parameters", "[curves][valuecurve]") {
    ValueCurve vc("ID_RND", 0, 100, "Random", 20, 60, 20);
    vc.SetActive(true);
    for (const auto& p : vc.GetPoints()) {
        CHECK(p.y >= 0.2f - 1e-4f);
        CHECK(p.y <= 0.6f + 1e-4f);
    }
}

TEST_CASE("ValueCurve deserialise tolerates malformed input", "[curves][valuecurve]") {
    SECTION("garbage with a pipe") {
        ValueCurve vc("foo|bar|baz");
        CHECK(vc.GetType() == "Flat");
    }
    SECTION("keys without values") {
        ValueCurve vc("Active=TRUE|Id=|Type=|Min=|Max=|");
        CHECK(vc.GetType().empty());
    }
    SECTION("non numeric parameters read as zero") {
        ValueCurve vc("Active=TRUE|Id=ID_A|Type=Ramp|Min=0.00|Max=100.00|P1=abc|P2=xyz|RV=TRUE|");
        CHECK_THAT(vc.GetParameter1(), WithinAbs(0.0, 1e-6));
        CHECK_THAT(vc.GetParameter2(), WithinAbs(0.0, 1e-6));
    }
    SECTION("custom values with missing separators are skipped") {
        ValueCurve vc("Active=TRUE|Id=ID_A|Type=Custom|Min=0.00|Max=100.00|RV=TRUE|Values=0.00:0.50;junk;1.00:0.25|");
        CHECK(vc.GetPointCount() == 2);
    }
    SECTION("unknown keys are ignored") {
        ValueCurve vc("Active=TRUE|Id=ID_A|Type=Flat|Min=0.00|Max=100.00|P1=10.00|Bogus=1|RV=TRUE|");
        CHECK_THAT(vc.GetParameter1(), WithinAbs(10.0, 1e-4));
    }
}

TEST_CASE("ValueCurve Active=FALSE stored curve remains inactive", "[curves][valuecurve]") {
    ValueCurve vc("Active=FALSE|Id=ID_Z|");
    CHECK_FALSE(vc.IsActive());
    CHECK(vc.GetId() == "ID_Z");
}

TEST_CASE("ValueCurve rescales unscaled curve to a new real range", "[curves][valuecurve]") {
    ValueCurve vc("Active=TRUE|Id=ID_A|Type=Ramp|Min=0.00|Max=200.00|P1=0.00|P2=100.00|");
    CHECK_THAT(vc.GetMax(), WithinAbs(200.0, 1e-3));
    CHECK_THAT(At(vc, 0.0f), WithinAbs(0.0, 1e-3));
}

TEST_CASE("ValueCurve FixChangedScale converts 0-100 parameters to a real range", "[curves][valuecurve]") {
    ValueCurve vc("ID_X", 0, 100, "Ramp", 0, 100);
    vc.FixChangedScale(0, 200, 1);
    CHECK_THAT(vc.GetParameter2(), WithinAbs(200.0, 1e-3));
    vc.UnFixChangedScale(0, 200);
    CHECK_THAT(vc.GetParameter2(), WithinAbs(100.0, 1e-3));
}

TEST_CASE("ValueCurve ConvertChangedScale rescales around a non-zero minimum", "[curves][valuecurve]") {
    ValueCurve vc("ID_X", 10, 20, "Ramp", 10, 20);
    vc.ConvertChangedScale(0, 100);
    CHECK_THAT(vc.GetParameter1(), WithinAbs(0.0, 1e-3));
    CHECK_THAT(vc.GetParameter2(), WithinAbs(100.0, 1e-3));
}

TEST_CASE("ValueCurve ConvertDivider scales parameters and limits", "[curves][valuecurve]") {
    ValueCurve vc("ID_X", 0, 100, "Ramp", 10, 50);
    vc.ConvertDivider(1, 10);
    CHECK_THAT(vc.GetParameter1(), WithinAbs(100.0, 1e-3));
    CHECK_THAT(vc.GetParameter2(), WithinAbs(500.0, 1e-3));
    CHECK_THAT(vc.GetMax(), WithinAbs(1000.0, 1e-3));
}

TEST_CASE("ValueCurve divisor is applied to divided outputs", "[curves][valuecurve]") {
    ValueCurve vc("ID_X", 0, 100, "Flat", 50, 0, 0, 0, false, 10.0f);
    vc.SetActive(true);
    CHECK(vc.GetDivisor() == 10);
    CHECK_THAT(vc.GetOutputValueAt(0.5f, 0, 1000), WithinAbs(50.0, 1e-3));
    CHECK_THAT(vc.GetOutputValueAtDivided(0.5f, 0, 1000), WithinAbs(5.0, 1e-3));
    CHECK_THAT(vc.GetScaledValue(1.0f), WithinAbs(10.0, 1e-4));
    CHECK_THAT(vc.GetMaxValueDivided(), WithinAbs(5.0, 1e-3));
}

TEST_CASE("ValueCurve ScaleAndOffsetValues scales flat and clamps", "[curves][valuecurve]") {
    auto vc = MakeActive("Flat", 0, 100, 40);
    vc.ScaleAndOffsetValues(2.0f, 0);
    CHECK_THAT(vc.GetParameter1(), WithinAbs(80.0, 1e-3));
    vc.ScaleAndOffsetValues(2.0f, 0);
    CHECK_THAT(vc.GetParameter1(), WithinAbs(100.0, 1e-3));
    vc.ScaleAndOffsetValues(1.0f, -30);
    CHECK_THAT(vc.GetParameter1(), WithinAbs(70.0, 1e-3));
}

TEST_CASE("ValueCurve ScaleAndOffsetValues scales custom points", "[curves][valuecurve]") {
    ValueCurve vc("ID_X", 0, 100, "Custom", 1);
    vc.SetValueAt(0.0f, 0.2f);
    vc.SetValueAt(1.0f, 0.4f);
    vc.ScaleAndOffsetValues(2.0f, 0);
    CHECK_THAT(vc.GetPointAt(0.0f), WithinAbs(0.4, 1e-4));
    CHECK_THAT(vc.GetPointAt(1.0f), WithinAbs(0.8, 1e-4));
}

TEST_CASE("ValueCurve SetType re-renders the shape", "[curves][valuecurve]") {
    auto vc = MakeActive("Flat", 0, 100, 25, 75);
    CHECK(vc.GetPointCount() == 2);
    vc.SetType("Ramp Up/Down");
    CHECK(vc.GetPointCount() == 3);
    CHECK(vc.GetType() == "Ramp Up/Down");
}

TEST_CASE("ValueCurve Timing Track types without a sequence fall back to min", "[curves][valuecurve]") {
    ValueCurve::SetSequenceElements(nullptr);
    ValueCurve vc("ID_X", 0, 100, "Timing Track Toggle", 10, 90, 0, 0, false, 1.0f, "Missing");
    vc.SetActive(true);
    CHECK(vc.GetTimingMarkOffsets(0, 1000).empty());
}

TEST_CASE("ValueCurve Music types without audio evaluate to the minimum", "[curves][valuecurve]") {
    ValueCurve::SetAudio(nullptr);
    ValueCurve vc("ID_X", 0, 100, "Music", 10, 90);
    vc.SetActive(true);
    CHECK_THAT(At(vc, 0.5f), WithinAbs(0.0, 1e-4));
}

TEST_CASE("ValueCurve resampled offsets without timing marks are evenly spaced", "[curves][valuecurve]") {
    ValueCurve::SetSequenceElements(nullptr);
    ValueCurve vc("ID_X", 0, 100, "Custom", 4);
    auto o = vc.GetResampledOffsets(0, 1000, 4);
    REQUIRE(o.size() == 5);
    for (int i = 0; i <= 4; ++i) {
        CHECK_THAT(o[i], WithinAbs(i / 4.0, 1e-9));
    }
    CHECK(vc.GetResampledOffsets(0, 1000, 0).size() == 2);
    CHECK(vc.GetResampledOffsets(0, 1000, 99).size() == 11);
}

// A wrapped wobble around 0: y = 0.05*sin(2*pi*(x + 0.01)), so the negative
// half-cycle wraps up to 0.95..1.0. Samples are every 0.025; the curve enters
// the wrapped region between 0.475 and 0.5 and leaves it between 0.975 and 1.0.
TEST_CASE("ValueCurve wrapped Sine interpolates inside the wrapped region", "[curves][valuecurve]") {
    auto vc = MakeActive("Sine", 0, 100, 1, 10, 10, 25, true);
    float a = At(vc, 0.600f);
    float b = At(vc, 0.625f);
    float mid = At(vc, 0.610f);
    REQUIRE(a > 0.9f);
    REQUIRE(b > 0.9f);
    REQUIRE(a > b);
    CHECK(mid < a);
    CHECK(mid > b);
}

TEST_CASE("ValueCurve wrapped Sine jumps across the wrap instead of sweeping", "[curves][valuecurve]") {
    auto vc = MakeActive("Sine", 0, 100, 1, 10, 10, 25, true);
    for (int i = 0; i <= 200; ++i) {
        float v = At(vc, i / 200.0f);
        INFO("offset " << i / 200.0f << " value " << v);
        CHECK(std::min(v, 1.0f - v) <= 0.051f);
    }
}
