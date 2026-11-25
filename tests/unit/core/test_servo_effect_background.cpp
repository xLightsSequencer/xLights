#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <string>

#include "effects/EffectManager.h"
#include "effects/ServoEffect.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceElements.h"
#include "test_dmx2_support.h"

using Catch::Approx;
using dmx2::RecColorAcc;

namespace {

struct ServoFixture {
    HeadlessRenderContext ctx;
    SequenceElements& se;
    EffectLayer* layer;
    ServoFixture() : se(ctx.GetSequenceElements()) {
        se.SetViewsManager(&ctx._sequenceViewManager);
        auto* el = dynamic_cast<ModelElement*>(se.AddElement("Servo Model", "model", true, false, true, false, false));
        REQUIRE(el != nullptr);
        layer = el->AddEffectLayer();
        REQUIRE(layer != nullptr);
    }
    Effect* Add(const std::string& settings, int start = 0, int end = 2000) {
        Effect* e = layer->AddEffect(0, "Servo", settings, "", start, end, 0, false);
        REQUIRE(e != nullptr);
        return e;
    }
};

// The line's centre at the left and right edges of a 100x100 cell (y grows downwards).
std::pair<float, float> LineEnds(const RecColorAcc& acc) {
    float lmin = 1e9f, lmax = -1e9f, rmin = 1e9f, rmax = -1e9f;
    for (const auto& v : acc.v) {
        if (v.x == Approx(0.0f)) {
            lmin = std::min(lmin, v.y);
            lmax = std::max(lmax, v.y);
        } else if (v.x == Approx(100.0f)) {
            rmin = std::min(rmin, v.y);
            rmax = std::max(rmax, v.y);
        }
    }
    REQUIRE(lmax >= lmin);
    REQUIRE(rmax >= rmin);
    CHECK(lmax - lmin == Approx(5.6f).margin(0.01));
    return { (lmin + lmax) / 2, (rmin + rmax) / 2 };
}

} // namespace

TEST_CASE("A Servo effect shows its position line before it has rendered", "[effects][servo]") {
    ServoFixture f;
    ServoEffect servo(EffectManager::eff_SERVO);

    SECTION("a ramp from the start value to the end value") {
        Effect* e = f.Add("E_TEXTCTRL_Servo=0.0,E_TEXTCTRL_EndValue=87.2,E_TOGGLEBUTTON_End=1");
        RecColorAcc acc;
        CHECK(servo.DrawEffectBackground(e, 0, 0, 100, 100, acc, nullptr, false) == 2);
        REQUIRE(acc.v.size() == 6);
        auto [left, right] = LineEnds(acc);
        CHECK(left == Approx(100.0f).margin(0.01));
        CHECK(right == Approx(12.8f).margin(0.01));
        CHECK(acc.v[0].c == xlYELLOW);
    }

    SECTION("a timing track driven servo waits for the render") {
        Effect* e = f.Add("E_CHECKBOX_Timing_Track=1,E_TEXTCTRL_Servo=50.0,E_TEXTCTRL_EndValue=50.0");
        RecColorAcc acc;
        servo.DrawEffectBackground(e, 0, 0, 100, 100, acc, nullptr, false);
        CHECK(acc.v.empty());
    }

    SECTION("the grid preference turns it off") {
        Effect* e = f.Add("E_TEXTCTRL_Servo=50.0,E_TEXTCTRL_EndValue=50.0");
        Effect::EnableBackgroundDisplayLists(false);
        RecColorAcc acc;
        servo.DrawEffectBackground(e, 0, 0, 100, 100, acc, nullptr, false);
        Effect::EnableBackgroundDisplayLists(true);
        CHECK(acc.v.empty());
    }

    SECTION("a rendered line is drawn instead") {
        Effect* e = f.Add("E_TEXTCTRL_Servo=0.0,E_TEXTCTRL_EndValue=0.0");
        {
            xlDisplayList& dl = e->GetBackgroundDisplayList();
            dl.resize(6);
            for (auto& item : dl) {
                item.x = 0.5f;
                item.y = 0.25f;
                item.color = xlRED;
            }
        }
        RecColorAcc acc;
        servo.DrawEffectBackground(e, 0, 0, 100, 100, acc, nullptr, false);
        REQUIRE(acc.v.size() == 6);
        CHECK(acc.v[0].y == Approx(25.0f));
        CHECK(acc.v[0].c == xlRED);
    }
}

TEST_CASE("Turning off one effect's background display list leaves the others on", "[effects][servo]") {
    ServoFixture f;
    Effect* a = f.Add("", 0, 1000);
    Effect* b = f.Add("", 1000, 2000);
    REQUIRE(Effect::AreBackgroundDisplayListsEnabled());

    // A Duplicate effect renders through a temporary copy of its source,
    // which must not record a line; the source and every other effect still do.
    Effect copy(*a);
    copy.DisableBackgroundDisplayList();
    CHECK_FALSE(copy.IsBackgroundDisplayListEnabled());
    CHECK(a->IsBackgroundDisplayListEnabled());
    CHECK(b->IsBackgroundDisplayListEnabled());
    CHECK(Effect::AreBackgroundDisplayListsEnabled());
}

TEST_CASE("A Servo effect draws its background without palette colours", "[effects][servo]") {
    // The grid hides an effect's background while no palette colour is
    // checked; the servo line doesn't use the palette, so it still shows.
    ServoEffect servo(EffectManager::eff_SERVO);
    CHECK_FALSE(servo.BackgroundNeedsPalette());
    EffectManager mgr;
    RenderableEffect* on = mgr.GetEffect("On");
    REQUIRE(on != nullptr);
    CHECK(on->BackgroundNeedsPalette());
}
