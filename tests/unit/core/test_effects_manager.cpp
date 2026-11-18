#include <catch2/catch_test_macros.hpp>

#include <set>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "../support/TempDir.h"
#include "effects/EffectManager.h"
#include "effects/RenderableEffect.h"
#include "render/Effect.h"
#include "utils/UtilClasses.h"

namespace {

// Pointing the manager at an (empty) scratch directory keeps the result independent
// of whatever resources the host process can locate.
struct ManagerFixture {
    TempDir dir;
    EffectManager mgr;
    ManagerFixture() : mgr(dir.path.string()) {}
};

class ProbeEffect : public RenderableEffect {
public:
    ProbeEffect(int id, const std::string& n) : RenderableEffect(id, n, nullptr, nullptr, nullptr, nullptr, nullptr) {}
    void Render(Effect*, const SettingsMap&, RenderBuffer&) override {}
};

class PureProbe : public ProbeEffect {
public:
    PureProbe() : ProbeEffect(0, "PureProbe") {}
    FrameParallelism GetFrameParallelism(const SettingsMap&) const override { return FrameParallelism::Pure; }
};

} // namespace

TEST_CASE("EffectManager registers every enum effect with no gaps", "[effects][manager]") {
    ManagerFixture f;
    REQUIRE(f.mgr.size() == (size_t)EffectManager::eff_LASTEFFECT);
    CHECK(f.mgr.GetLastEffectId() == (int)EffectManager::eff_LASTEFFECT - 1);
    for (size_t i = 0; i < f.mgr.size(); ++i) {
        INFO("index " << i);
        CHECK(f.mgr.GetEffect((int)i) != nullptr);
    }
}

TEST_CASE("EffectManager effect ids equal their index", "[effects][manager]") {
    ManagerFixture f;
    for (size_t i = 0; i < f.mgr.size(); ++i) {
        REQUIRE(f.mgr.GetEffect((int)i) != nullptr);
        CHECK(f.mgr.GetEffect((int)i)->GetId() == (int)i);
    }
}

TEST_CASE("EffectManager effect names are non-empty and unique", "[effects][manager]") {
    ManagerFixture f;
    std::set<std::string> names;
    for (auto* e : f.mgr) {
        REQUIRE(e != nullptr);
        INFO("effect id " << e->GetId());
        CHECK_FALSE(e->Name().empty());
        CHECK(names.insert(e->Name()).second);
    }
    CHECK(names.size() == f.mgr.size());
}

TEST_CASE("EffectManager effect tooltips are non-empty and unique", "[effects][manager]") {
    ManagerFixture f;
    std::set<std::string> tips;
    for (auto* e : f.mgr) {
        INFO("effect " << e->Name());
        CHECK_FALSE(e->ToolTip().empty());
        CHECK(tips.insert(e->ToolTip()).second);
    }
}

TEST_CASE("EffectManager name to index round trips", "[effects][manager]") {
    ManagerFixture f;
    for (size_t i = 0; i < f.mgr.size(); ++i) {
        const std::string& name = f.mgr.GetEffectName((int)i);
        INFO("name " << name);
        CHECK(f.mgr.GetEffectIndex(name) == (int)i);
    }
}

TEST_CASE("EffectManager GetEffect by name returns the effect at that index", "[effects][manager]") {
    ManagerFixture f;
    for (size_t i = 0; i < f.mgr.size(); ++i) {
        RenderableEffect* byIndex = f.mgr.GetEffect((int)i);
        RenderableEffect* byName = f.mgr.GetEffect(byIndex->Name());
        CHECK(byName == byIndex);
        CHECK(f.mgr[(int)i] == byIndex);
    }
}

TEST_CASE("EffectManager resolves effects by tooltip as well as name", "[effects][manager]") {
    ManagerFixture f;
    for (auto* e : f.mgr) {
        INFO("tooltip " << e->ToolTip());
        CHECK(f.mgr.GetEffect(e->ToolTip()) == e);
    }
    CHECK(f.mgr.GetEffectIndex("Single Strand") == (int)EffectManager::eff_SINGLESTRAND);
    CHECK(f.mgr.GetEffectIndex("SingleStrand") == (int)EffectManager::eff_SINGLESTRAND);
}

TEST_CASE("EffectManager enum ids map to the expected effect names", "[effects][manager]") {
    ManagerFixture f;
    CHECK(f.mgr.GetEffectName(EffectManager::eff_OFF) == "Off");
    CHECK(f.mgr.GetEffectName(EffectManager::eff_ON) == "On");
    CHECK(f.mgr.GetEffectName(EffectManager::eff_BARS) == "Bars");
    CHECK(f.mgr.GetEffectName(EffectManager::eff_COLORWASH) == "Color Wash");
    CHECK(f.mgr.GetEffectName(EffectManager::eff_FILL) == "Fill");
    CHECK(f.mgr.GetEffectName(EffectManager::eff_MARQUEE) == "Marquee");
    CHECK(f.mgr.GetEffectName(EffectManager::eff_MOVINGHEAD) == "Moving Head");
    CHECK(f.mgr.GetEffectName(EffectManager::eff_SINGLESTRAND) == "SingleStrand");
    CHECK(f.mgr.GetEffectName(EffectManager::eff_STROBE) == "Strobe");
    CHECK(f.mgr.GetEffectName(EffectManager::eff_TWINKLE) == "Twinkle");
    CHECK(f.mgr.GetEffectName(EffectManager::eff_VUMETER) == "VU Meter");
    CHECK(f.mgr.GetEffectName(EffectManager::eff_WAVE) == "Wave");
}

TEST_CASE("EffectManager unknown names are not found", "[effects][manager]") {
    ManagerFixture f;
    CHECK(f.mgr.GetEffect(std::string("No Such Effect")) == nullptr);
    CHECK(f.mgr.GetEffectIndex("No Such Effect") == -1);
    CHECK(f.mgr.GetEffect(std::string()) == nullptr);
    CHECK(f.mgr.GetEffectIndex("") == -1);
    CHECK(f.mgr.size() == (size_t)EffectManager::eff_LASTEFFECT);
}

TEST_CASE("EffectManager name lookup is case sensitive", "[effects][manager]") {
    ManagerFixture f;
    CHECK(f.mgr.GetEffectIndex("Color Wash") == (int)EffectManager::eff_COLORWASH);
    CHECK(f.mgr.GetEffectIndex("color wash") == -1);
    CHECK(f.mgr.GetEffectIndex("COLOR WASH") == -1);
    CHECK(f.mgr.GetEffectIndex("ColorWash") == -1);
}

TEST_CASE("EffectManager legacy CoroFaces name maps to Faces", "[effects][manager]") {
    ManagerFixture f;
    REQUIRE(f.mgr.GetEffect("Faces") != nullptr);
    CHECK(f.mgr.GetEffect("CoroFaces") == f.mgr.GetEffect("Faces"));
    CHECK(f.mgr.GetEffectIndex("CoroFaces") == (int)EffectManager::eff_FACES);
}

TEST_CASE("EffectManager out of range index returns null effect", "[effects][manager]") {
    ManagerFixture f;
    CHECK(f.mgr.GetEffect(-1) == nullptr);
    CHECK(f.mgr.GetEffect((int)f.mgr.size()) == nullptr);
    CHECK(f.mgr.GetEffect(100000) == nullptr);
    CHECK(f.mgr[-5] == nullptr);
}

TEST_CASE("EffectManager GetEffectName falls back to the first effect for a bad index", "[effects][manager]") {
    ManagerFixture f;
    const std::string& first = f.mgr.GetEffectName(0);
    CHECK(f.mgr.GetEffectName(-1) == first);
    CHECK(f.mgr.GetEffectName(9999) == first);
}

TEST_CASE("EffectManager iterators cover every effect once", "[effects][manager]") {
    ManagerFixture f;
    size_t n = 0;
    std::set<RenderableEffect*> seen;
    for (auto it = f.mgr.begin(); it != f.mgr.end(); ++it) {
        ++n;
        seen.insert(*it);
    }
    CHECK(n == f.mgr.size());
    CHECK(seen.size() == f.mgr.size());
}

TEST_CASE("EffectManager without metadata exposes none", "[effects][manager][metadata]") {
    ManagerFixture f;
    for (auto* e : f.mgr) {
        INFO("effect " << e->Name());
        CHECK_FALSE(e->HasMetadata());
        CHECK(e->GetPropertyMetadata("anything") == nullptr);
    }
}

TEST_CASE("EffectManager loads metadata from <Name without spaces>.json", "[effects][manager][metadata]") {
    TempDir dir;
    dir.Write("ColorWash.json", R"({"properties":[{"id":"Probe_Value","default":7}]})");
    dir.Write("VUMeter.json", R"({"properties":[{"id":"Probe_Other","default":"x"}]})");
    EffectManager mgr(dir.path.string());

    RenderableEffect* cw = mgr.GetEffect("Color Wash");
    REQUIRE(cw != nullptr);
    CHECK(cw->HasMetadata());
    CHECK(cw->GetIntDefault("Probe_Value", -1) == 7);

    RenderableEffect* vu = mgr.GetEffect("VU Meter");
    REQUIRE(vu != nullptr);
    CHECK(vu->HasMetadata());
    CHECK(vu->GetStringDefault("Probe_Other", "") == "x");

    CHECK_FALSE(mgr.GetEffect("Bars")->HasMetadata());
}

TEST_CASE("EffectManager tolerates a corrupt metadata file", "[effects][manager][metadata]") {
    TempDir dir;
    dir.Write("Bars.json", "{ this is not json");
    EffectManager mgr(dir.path.string());
    RenderableEffect* bars = mgr.GetEffect("Bars");
    REQUIRE(bars != nullptr);
    CHECK_FALSE(bars->HasMetadata());
    CHECK(mgr.size() == (size_t)EffectManager::eff_LASTEFFECT);
}

TEST_CASE("RenderableEffect metadata accessors return fallbacks when absent", "[effects][metadata]") {
    ProbeEffect e(0, "Probe");
    CHECK_FALSE(e.HasMetadata());
    CHECK(e.GetIntDefault("x", 42) == 42);
    CHECK(e.GetBoolDefault("x", true));
    CHECK(e.GetStringDefault("x", "fb") == "fb");
    CHECK(e.GetFloatDefault("x", 1.5f) == 1.5f);
    CHECK(e.GetDoubleDefault("x", 2.5) == 2.5);
    CHECK(e.GetMinFromMetadata("x", -3) == -3);
    CHECK(e.GetMaxFromMetadata("x", 99) == 99);
    CHECK(e.GetDivisorFromMetadata("x", 10) == 10);
}

TEST_CASE("RenderableEffect metadata accessors read default min max and divisor", "[effects][metadata]") {
    ProbeEffect e(0, "Probe");
    e.SetMetadata(nlohmann::json::parse(R"({"properties":[
        {"id":"A","default":5,"min":1,"max":10,"divisor":100,"vcMin":2,"vcMax":8},
        {"id":"B","default":2.5},
        {"id":"C","default":true},
        {"id":"D","default":"text"},
        {"id":"E","min":0,"max":4}
    ]})"));
    REQUIRE(e.HasMetadata());
    CHECK(e.GetIntDefault("A", 0) == 5);
    CHECK(e.GetMinFromMetadata("A", 0) == 1.0);
    CHECK(e.GetMaxFromMetadata("A", 0) == 10.0);
    CHECK(e.GetDivisorFromMetadata("A", 1) == 100);
    CHECK(e.GetVCMinFromMetadata("A", 0) == 2.0);
    CHECK(e.GetVCMaxFromMetadata("A", 0) == 8.0);
    CHECK(e.GetFloatDefault("B", 0) == 2.5f);
    CHECK(e.GetDoubleDefault("B", 0) == 2.5);
    CHECK(e.GetBoolDefault("C", false));
    CHECK(e.GetStringDefault("D", "") == "text");
    CHECK(e.GetIntDefault("E", 77) == 77);
    CHECK(e.GetMaxFromMetadata("E", 0) == 4.0);
}

TEST_CASE("RenderableEffect VC limits fall back to slider min and max", "[effects][metadata]") {
    ProbeEffect e(0, "Probe");
    e.SetMetadata(nlohmann::json::parse(R"({"properties":[{"id":"A","min":3,"max":30}]})"));
    CHECK(e.GetVCMinFromMetadata("A", -1) == 3.0);
    CHECK(e.GetVCMaxFromMetadata("A", -1) == 30.0);
    CHECK(e.GetVCMinFromMetadata("Missing", -1) == -1.0);
}

TEST_CASE("RenderableEffect metadata accessors ignore wrongly typed fields", "[effects][metadata]") {
    ProbeEffect e(0, "Probe");
    e.SetMetadata(nlohmann::json::parse(R"({"properties":[
        {"id":"A","default":"five","min":"x","max":null,"divisor":"y"},
        {"id":"B","default":3},
        {"id":"C","default":1}
    ]})"));
    CHECK(e.GetIntDefault("A", 11) == 11);
    CHECK(e.GetMinFromMetadata("A", 1) == 1.0);
    CHECK(e.GetMaxFromMetadata("A", 2) == 2.0);
    CHECK(e.GetDivisorFromMetadata("A", 3) == 3);
    CHECK(e.GetStringDefault("B", "fb") == "fb");
    CHECK(e.GetBoolDefault("C", false) == false);
}

TEST_CASE("RenderableEffect metadata without a properties array yields nothing", "[effects][metadata]") {
    ProbeEffect e(0, "Probe");
    e.SetMetadata(nlohmann::json::parse(R"({"name":"x","properties":5})"));
    CHECK(e.GetPropertyMetadata("A") == nullptr);
    CHECK(e.GetIntDefault("A", 9) == 9);
    e.SetMetadata(nlohmann::json::object());
    CHECK_FALSE(e.HasMetadata());
}

TEST_CASE("RenderableEffect base class defaults", "[effects][manager]") {
    ProbeEffect e(3, "Probe");
    SettingsMap sm;
    CHECK(e.GetId() == 3);
    CHECK(e.Name() == "Probe");
    CHECK(e.ToolTip() == "Probe");
    CHECK(e.GetColorSupportedCount() == -1);
    CHECK(e.CanBeRandom());
    CHECK(e.AppropriateOnNodes());
    CHECK_FALSE(e.needsLoadFiles());
    CHECK_FALSE(e.CanRenderPartialTimeInterval());
    CHECK_FALSE(e.SupportsLinearColorCurves(sm));
    CHECK_FALSE(e.SupportsRadialColorCurves(sm));
    CHECK(e.GetFrameParallelism(sm) == RenderableEffect::FrameParallelism::Stateful);
    CHECK(e.GetEffectiveFrameParallelism(sm) == RenderableEffect::FrameParallelism::Stateful);
    CHECK(e.GetSettingOptions("anything").empty());
}

TEST_CASE("RenderableEffect needToAdjustSettings reflects the 2024.05 base migration", "[effects][manager]") {
    ProbeEffect e(0, "Probe");
    CHECK(e.needToAdjustSettings("2020.01"));
    CHECK(e.needToAdjustSettings("2024.04"));
    CHECK_FALSE(e.needToAdjustSettings("2024.05"));
    CHECK_FALSE(e.needToAdjustSettings("2026.10"));
}

TEST_CASE("Every effect wants migration for very old sequences", "[effects][manager][adjust]") {
    ManagerFixture f;
    for (auto* e : f.mgr) {
        INFO("effect " << e->Name());
        CHECK(e->needToAdjustSettings("2000.01"));
    }
}

TEST_CASE("No effect except Fill wants migration for far-future sequences", "[effects][manager][adjust]") {
    ManagerFixture f;
    for (auto* e : f.mgr) {
        INFO("effect " << e->Name());
        if (e->GetId() == EffectManager::eff_FILL) {
            // FillEffect::needToAdjustSettings unconditionally returns true
            CHECK(e->needToAdjustSettings("2999.01"));
        } else {
            CHECK_FALSE(e->needToAdjustSettings("2999.01"));
        }
    }
}

TEST_CASE("Persistent canvas and freeze settings force stateful rendering", "[effects][manager]") {
    PureProbe pure;
    SettingsMap sm;
    CHECK(pure.GetEffectiveFrameParallelism(sm) == RenderableEffect::FrameParallelism::Pure);

    SettingsMap overlay;
    overlay["CHECKBOX_OverlayBkg"] = "1";
    CHECK(pure.GetEffectiveFrameParallelism(overlay) == RenderableEffect::FrameParallelism::Stateful);

    SettingsMap canvas;
    canvas["CHECKBOX_Canvas"] = "1";
    CHECK(pure.GetEffectiveFrameParallelism(canvas) == RenderableEffect::FrameParallelism::Stateful);

    SettingsMap freeze;
    freeze["SPINCTRL_FreezeEffectAtFrame"] = "10";
    CHECK(pure.GetEffectiveFrameParallelism(freeze) == RenderableEffect::FrameParallelism::Stateful);

    SettingsMap suppress;
    suppress["SPINCTRL_SuppressEffectUntil"] = "3";
    CHECK(pure.GetEffectiveFrameParallelism(suppress) == RenderableEffect::FrameParallelism::Stateful);

    SettingsMap notPersistent;
    notPersistent["CHECKBOX_OverlayBkg"] = "0";
    notPersistent["SPINCTRL_SuppressEffectUntil"] = "0";
    CHECK(pure.GetEffectiveFrameParallelism(notPersistent) == RenderableEffect::FrameParallelism::Pure);
}
