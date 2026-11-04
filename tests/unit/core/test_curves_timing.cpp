#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceElements.h"
#include "render/ValueCurve.h"

#include <string>

using Catch::Matchers::WithinAbs;

namespace {
struct TimingFixture {
    HeadlessRenderContext ctx;
    SequenceElements& se;
    TimingElement* track = nullptr;
    EffectLayer* layer = nullptr;

    TimingFixture() : se(ctx.GetSequenceElements()) {
        se.SetViewsManager(&ctx._sequenceViewManager);
        track = dynamic_cast<TimingElement*>(se.AddElement("Beats", "timing", true, false, true, false, false));
        layer = track->AddEffectLayer();
        ValueCurve::SetSequenceElements(&se);
    }
    ~TimingFixture() {
        ValueCurve::SetSequenceElements(nullptr);
    }
    void Marks(int count, int length, const std::string& label = "") {
        for (int i = 0; i < count; ++i) {
            layer->AddEffect(0, label, "", "", i * length, (i + 1) * length, 0, false);
        }
    }
};

ValueCurve Curve(const std::string& type, float p1, float p2, float p3 = 0) {
    ValueCurve vc("ID_TT", 0, 100, type, p1, p2, p3, 0, false, 1.0f, "Beats");
    vc.SetActive(true);
    return vc;
}
}

TEST_CASE("ValueCurve timing mark offsets cover the whole range with marks in between", "[curves][timing]") {
    TimingFixture f;
    f.Marks(4, 500);
    auto vc = Curve("Timing Track Toggle", 10, 90);
    auto o = vc.GetTimingMarkOffsets(0, 2000);
    REQUIRE(o.size() == 5);
    CHECK_THAT(o[0], WithinAbs(0.0, 1e-9));
    CHECK_THAT(o[1], WithinAbs(0.25, 1e-9));
    CHECK_THAT(o[2], WithinAbs(0.5, 1e-9));
    CHECK_THAT(o[3], WithinAbs(0.75, 1e-9));
    CHECK_THAT(o[4], WithinAbs(1.0, 1e-9));
}

TEST_CASE("ValueCurve timing mark offsets are relative to the effect window", "[curves][timing]") {
    TimingFixture f;
    f.Marks(4, 500);
    auto vc = Curve("Timing Track Toggle", 10, 90);
    auto o = vc.GetTimingMarkOffsets(250, 1250);
    REQUIRE(o.size() == 4);
    CHECK_THAT(o[0], WithinAbs(0.0, 1e-9));
    CHECK_THAT(o[1], WithinAbs(0.25, 1e-9));
    CHECK_THAT(o[2], WithinAbs(0.75, 1e-9));
    CHECK_THAT(o[3], WithinAbs(1.0, 1e-9));
}

TEST_CASE("ValueCurve timing mark offsets are empty without a usable track", "[curves][timing]") {
    TimingFixture f;
    f.Marks(2, 500);
    ValueCurve none("ID", 0, 100, "Timing Track Toggle", 10, 90);
    CHECK(none.GetTimingMarkOffsets(0, 1000).empty());
    ValueCurve missing("ID", 0, 100, "Timing Track Toggle", 10, 90, 0, 0, false, 1.0f, "Nope");
    CHECK(missing.GetTimingMarkOffsets(0, 1000).empty());
    ValueCurve::SetSequenceElements(nullptr);
    auto vc = Curve("Timing Track Toggle", 10, 90);
    CHECK(vc.GetTimingMarkOffsets(0, 1000).empty());
}

TEST_CASE("ValueCurve cached timing offsets follow the requested window", "[curves][timing]") {
    TimingFixture f;
    f.Marks(4, 500);
    auto vc = Curve("Timing Track Toggle", 10, 90);
    auto a = vc.GetTimingMarkOffsetsCached(0, 2000);
    auto b = vc.GetTimingMarkOffsetsCached(0, 2000);
    CHECK(a == b);
    auto c = vc.GetTimingMarkOffsetsCached(0, 1000);
    CHECK(c != a);
    vc.SetTimingTrack("Beats");
    CHECK(vc.GetTimingMarkOffsetsCached(0, 2000) == a);
}

TEST_CASE("ValueCurve Timing Track Toggle alternates on each mark", "[curves][timing]") {
    TimingFixture f;
    f.Marks(4, 500);
    auto vc = Curve("Timing Track Toggle", 10, 90);
    CHECK_THAT(vc.GetValueAt(0.05f, 0, 2000), WithinAbs(0.9, 1e-4));
    CHECK_THAT(vc.GetValueAt(0.125f, 0, 2000), WithinAbs(0.9, 1e-4));
    CHECK_THAT(vc.GetValueAt(0.3f, 0, 2000), WithinAbs(0.1, 1e-4));
    CHECK_THAT(vc.GetValueAt(0.55f, 0, 2000), WithinAbs(0.9, 1e-4));
    CHECK_THAT(vc.GetValueAt(0.8f, 0, 2000), WithinAbs(0.1, 1e-4));
}

TEST_CASE("ValueCurve Timing Track Toggle with no marks stays at the minimum", "[curves][timing]") {
    TimingFixture f;
    auto vc = Curve("Timing Track Toggle", 10, 90);
    CHECK_THAT(vc.GetValueAt(0.5f, 0, 2000), WithinAbs(0.1, 1e-4));
}

TEST_CASE("ValueCurve Timing Track Fade Fixed fades over a fixed number of frames", "[curves][timing]") {
    TimingFixture f;
    f.Marks(4, 500);
    auto vc = Curve("Timing Track Fade Fixed", 10, 90, 4);
    CHECK_THAT(vc.GetValueAt(0.0f, 0, 2000), WithinAbs(0.9, 1e-3));
    CHECK_THAT(vc.GetValueAt(0.05f, 0, 2000), WithinAbs(0.5, 1e-3));
    CHECK_THAT(vc.GetValueAt(0.125f, 0, 2000), WithinAbs(0.1, 1e-3));
    CHECK_THAT(vc.GetValueAt(0.25f, 0, 2000), WithinAbs(0.9, 1e-3));
    CHECK_THAT(vc.GetValueAt(0.4f, 0, 2000), WithinAbs(0.1, 1e-3));
}

TEST_CASE("ValueCurve Timing Track Fade Fixed before any mark is the minimum", "[curves][timing]") {
    TimingFixture f;
    f.layer->AddEffect(0, "", "", "", 1000, 1500, 0, false);
    auto vc = Curve("Timing Track Fade Fixed", 10, 90, 4);
    CHECK_THAT(vc.GetValueAt(0.1f, 0, 2000), WithinAbs(0.1, 1e-3));
}

TEST_CASE("ValueCurve Timing Track Fade Proportional fades over a share of each interval", "[curves][timing]") {
    TimingFixture f;
    f.Marks(4, 500);
    auto vc = Curve("Timing Track Fade Proportional", 0, 100, 50);
    float start = vc.GetValueAt(0.0f, 0, 2000);
    float mid = vc.GetValueAt(0.05f, 0, 2000);
    float late = vc.GetValueAt(0.2f, 0, 2000);
    CHECK_THAT(start, WithinAbs(1.0, 1e-3));
    CHECK(mid < start);
    CHECK(mid > late);
    CHECK_THAT(late, WithinAbs(0.0, 1e-3));
}

TEST_CASE("ValueCurve label filter limits which marks count", "[curves][timing]") {
    TimingFixture f;
    f.layer->AddEffect(0, "a", "", "", 0, 500, 0, false);
    f.layer->AddEffect(0, "b", "", "", 500, 1000, 0, false);
    f.layer->AddEffect(0, "a", "", "", 1000, 1500, 0, false);
    ValueCurve vc("ID_TT", 0, 100, "Timing Track Fade Fixed", 10, 90, 4, 0, false, 1.0f, "Beats", "a", false);
    vc.SetActive(true);
    CHECK_THAT(vc.GetValueAt(0.26f, 0, 2000), WithinAbs(0.1, 1e-3));
    CHECK_THAT(vc.GetValueAt(0.5f, 0, 2000), WithinAbs(0.9, 1e-3));
}

TEST_CASE("ValueCurve Custom with cycles snaps repeats to timing marks", "[curves][timing]") {
    TimingFixture f;
    f.layer->AddEffect(0, "", "", "", 0, 800, 0, false);
    f.layer->AddEffect(0, "", "", "", 800, 2000, 0, false);
    ValueCurve vc("ID_X", 0, 100, "Custom", 1);
    vc.SetValueAt(0.0f, 0.0f);
    vc.SetValueAt(1.0f, 1.0f);
    vc.SetActive(true);
    vc.SetParameter1(2);
    vc.SetTimingTrack("Beats");
    CHECK_THAT(vc.GetValueAt(0.4f, 0, 2000), WithinAbs(1.0, 0.03));
    CHECK(vc.GetValueAt(0.39f, 0, 2000) > 0.9f);
    CHECK(vc.GetValueAt(0.41f, 0, 2000) < 0.05f);
    auto r = vc.GetResampledOffsets(0, 2000, 2);
    REQUIRE(r.size() == 3);
    CHECK_THAT(r[1], WithinAbs(0.4, 1e-6));
}
