#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

#include "effects/EffectManager.h"
#include "effects/RenderableEffect.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "utils/UtilClasses.h"

namespace {

// The base RenderableEffect migration walks Effect -> EffectLayer -> Element ->
// SequenceElements -> RenderContext, so the effects under test are attached to a
// real (empty) headless show.
struct AdjustFixture {
    HeadlessRenderContext ctx;
    Element* element = nullptr;
    EffectLayer* layer = nullptr;
    int nextStart = 0;

    explicit AdjustFixture(const std::string& elementName = "Fixture") {
        element = ctx._sequenceElements.AddElement(elementName, "model", true, false, true, false, false);
        layer = element->AddEffectLayer();
    }

    Effect* Make(const std::string& effectName, const std::string& settings) {
        Effect* e = layer->AddEffect(0, effectName, settings, "", nextStart, nextStart + 100, 0, false);
        nextStart += 200;
        return e;
    }

    // Value curves are stored after construction: parsing an active curve runs
    // UpgradeValueCurve, which needs the effect's JSON metadata.
    Effect* MigrateWithCurve(const std::string& effectName, const std::string& version, const std::string& settings,
                             const std::string& curveKey, const std::string& curve) {
        Effect* e = Make(effectName, settings);
        REQUIRE(e != nullptr);
        e->GetSettings()[curveKey] = curve;
        RenderableEffect* fx = Fx(effectName);
        REQUIRE(fx != nullptr);
        fx->adjustSettings(version, e);
        return e;
    }

    RenderableEffect* Fx(const std::string& effectName) { return ctx.effectManager.GetEffect(effectName); }

    // Runs the migration the way SequenceFile does and returns the resulting effect.
    Effect* Migrate(const std::string& effectName, const std::string& version, const std::string& settings) {
        Effect* e = Make(effectName, settings);
        REQUIRE(e != nullptr);
        RenderableEffect* fx = Fx(effectName);
        REQUIRE(fx != nullptr);
        fx->adjustSettings(version, e);
        return e;
    }
};

} // namespace

TEST_CASE("Circles migration promotes Collide to Bounce", "[effects][adjust]") {
    AdjustFixture f;
    Effect* e = f.Migrate("Circles", "2026.01", "E_CHECKBOX_Circles_Collide=1");
    CHECK(e->GetSettings().GetBool("E_CHECKBOX_Circles_Bounce", false));
    CHECK_FALSE(e->GetSettings().Contains("E_CHECKBOX_Circles_Collide"));
}

TEST_CASE("Circles migration drops an unchecked Collide without enabling Bounce", "[effects][adjust]") {
    AdjustFixture f;
    Effect* e = f.Migrate("Circles", "2026.01", "E_CHECKBOX_Circles_Collide=0");
    CHECK_FALSE(e->GetSettings().Contains("E_CHECKBOX_Circles_Bounce"));
    CHECK_FALSE(e->GetSettings().Contains("E_CHECKBOX_Circles_Collide"));
}

TEST_CASE("Circles migration keeps an existing Bounce value when Collide is off", "[effects][adjust]") {
    AdjustFixture f;
    Effect* e = f.Migrate("Circles", "2026.01", "E_CHECKBOX_Circles_Collide=0,E_CHECKBOX_Circles_Bounce=1");
    CHECK(e->GetSettings().GetBool("E_CHECKBOX_Circles_Bounce", false));
}

TEST_CASE("Circles migration leaves current sequences alone", "[effects][adjust]") {
    AdjustFixture f;
    Effect* e = f.Migrate("Circles", "2026.06", "E_CHECKBOX_Circles_Collide=1");
    CHECK(e->GetSettings().GetBool("E_CHECKBOX_Circles_Collide", false));
    CHECK_FALSE(e->GetSettings().Contains("E_CHECKBOX_Circles_Bounce"));
}

TEST_CASE("Fan Galaxy and Shockwave pin Scale off for pre 2025.04 sequences", "[effects][adjust]") {
    AdjustFixture f;
    CHECK(f.Migrate("Fan", "2024.20", "")->GetSettings().Get("E_CHECKBOX_Fan_Scale", "?") == "0");
    CHECK(f.Migrate("Galaxy", "2024.20", "")->GetSettings().Get("E_CHECKBOX_Galaxy_Scale", "?") == "0");
    CHECK(f.Migrate("Shockwave", "2024.20", "")->GetSettings().Get("E_CHECKBOX_Shockwave_Scale", "?") == "0");
}

TEST_CASE("Fan Galaxy and Shockwave overwrite a stored Scale for old sequences", "[effects][adjust]") {
    AdjustFixture f;
    CHECK(f.Migrate("Fan", "2025.01", "E_CHECKBOX_Fan_Scale=1")->GetSettings().Get("E_CHECKBOX_Fan_Scale", "?") == "0");
    CHECK(f.Migrate("Galaxy", "2025.01", "E_CHECKBOX_Galaxy_Scale=1")->GetSettings().Get("E_CHECKBOX_Galaxy_Scale", "?") == "0");
}

TEST_CASE("Fan Galaxy and Shockwave leave 2025.04 and later sequences alone", "[effects][adjust]") {
    AdjustFixture f;
    CHECK_FALSE(f.Migrate("Fan", "2025.04", "")->GetSettings().Contains("E_CHECKBOX_Fan_Scale"));
    CHECK_FALSE(f.Migrate("Galaxy", "2025.10", "")->GetSettings().Contains("E_CHECKBOX_Galaxy_Scale"));
    CHECK(f.Migrate("Shockwave", "2026.01", "E_CHECKBOX_Shockwave_Scale=1")->GetSettings().Get("E_CHECKBOX_Shockwave_Scale", "?") == "1");
}

TEST_CASE("Fill migration adds Color_Time for pre 2016.41 sequences", "[effects][adjust]") {
    AdjustFixture f;
    Effect* e = f.Migrate("Fill", "2016.40", "");
    CHECK(e->GetSettings().Get("E_CHECKBOX_Fill_Color_Time", "?") == "1");
    CHECK(e->GetSettings().Get("E_CHECKBOX_Fill_Wrap", "?") == "1");
}

TEST_CASE("Fill migration adds only Wrap between 2016.41 and 2016.53", "[effects][adjust]") {
    AdjustFixture f;
    Effect* e = f.Migrate("Fill", "2016.45", "");
    CHECK_FALSE(e->GetSettings().Contains("E_CHECKBOX_Fill_Color_Time"));
    CHECK(e->GetSettings().Get("E_CHECKBOX_Fill_Wrap", "?") == "1");
}

TEST_CASE("Fill migration does nothing for newer sequences", "[effects][adjust]") {
    AdjustFixture f;
    Effect* e = f.Migrate("Fill", "2025.01", "E_CHECKBOX_Fill_Wrap=0");
    CHECK(e->GetSettings().Get("E_CHECKBOX_Fill_Wrap", "?") == "0");
    CHECK_FALSE(e->GetSettings().Contains("E_CHECKBOX_Fill_Color_Time"));
}

TEST_CASE("Fire migration pins the classic style on old sequences", "[effects][adjust]") {
    AdjustFixture f;
    CHECK(f.Migrate("Fire", "2026.10", "")->GetSettings().Get("E_CHOICE_Fire_Style", "?") == "Old Render Method");
}

TEST_CASE("Fire migration keeps an explicitly stored style", "[effects][adjust]") {
    AdjustFixture f;
    CHECK(f.Migrate("Fire", "2026.10", "E_CHOICE_Fire_Style=New Render Method")->GetSettings().Get("E_CHOICE_Fire_Style", "?") == "New Render Method");
}

TEST_CASE("Fire migration leaves new sequences without a style", "[effects][adjust]") {
    AdjustFixture f;
    CHECK_FALSE(f.Migrate("Fire", "2026.15", "")->GetSettings().Contains("E_CHOICE_Fire_Style"));
}

TEST_CASE("Pinwheel migration pins the old render method", "[effects][adjust]") {
    AdjustFixture f;
    CHECK(f.Migrate("Pinwheel", "2026.05", "")->GetSettings().Get("E_CHOICE_Pinwheel_Style", "?") == "Old Render Method");
    CHECK(f.Migrate("Pinwheel", "2026.05", "E_CHOICE_Pinwheel_Style=New Render Method")->GetSettings().Get("E_CHOICE_Pinwheel_Style", "?") == "New Render Method");
    CHECK_FALSE(f.Migrate("Pinwheel", "2026.06", "")->GetSettings().Contains("E_CHOICE_Pinwheel_Style"));
}

TEST_CASE("Twinkle migration forces the old style for pre 2020.57 sequences", "[effects][adjust]") {
    AdjustFixture f;
    CHECK(f.Migrate("Twinkle", "2020.10", "E_CHOICE_Twinkle_Style=New Render Method")->GetSettings().Get("E_CHOICE_Twinkle_Style", "?") == "Old Render Method");
}

TEST_CASE("Twinkle migration pins the old style only when none is stored", "[effects][adjust]") {
    AdjustFixture f;
    CHECK(f.Migrate("Twinkle", "2025.01", "")->GetSettings().Get("E_CHOICE_Twinkle_Style", "?") == "Old Render Method");
    CHECK(f.Migrate("Twinkle", "2025.01", "E_CHOICE_Twinkle_Style=New Render Method")->GetSettings().Get("E_CHOICE_Twinkle_Style", "?") == "New Render Method");
    CHECK_FALSE(f.Migrate("Twinkle", "2026.06", "")->GetSettings().Contains("E_CHOICE_Twinkle_Style"));
}

TEST_CASE("Twinkle migration keeps the Steps value curve data", "[effects][adjust]") {
    AdjustFixture f;
    const std::string vc = "Active=TRUE|Id=ID_VALUECURVE_Twinkle_Steps|Type=Flat|Min=2.00|Max=100.00|P1=50.00|RV=TRUE|";
    Effect* e = f.MigrateWithCurve("Twinkle", "2026.01", "", "E_VALUECURVE_Twinkle_Steps", vc);
    const std::string out = e->GetSettings().Get("E_VALUECURVE_Twinkle_Steps", "");
    CHECK(out.find("Active=TRUE") != std::string::npos);
    CHECK(out.find("Min=2.00") != std::string::npos);
    CHECK(out.find("P1=50.00") != std::string::npos);
}

TEST_CASE("Twinkle migration widens an active Steps value curve maximum to 400", "[effects][adjust]") {
    AdjustFixture f;
    const std::string vc = "Active=TRUE|Id=ID_VALUECURVE_Twinkle_Steps|Type=Flat|Min=2.00|Max=100.00|P1=50.00|RV=TRUE|";
    Effect* e = f.MigrateWithCurve("Twinkle", "2026.01", "", "E_VALUECURVE_Twinkle_Steps", vc);
    const std::string out = e->GetSettings().Get("E_VALUECURVE_Twinkle_Steps", "");
    CHECK(out.find("Max=400.00") != std::string::npos);
}

TEST_CASE("Twinkle migration ignores a Steps curve that is not at the old maximum", "[effects][adjust]") {
    AdjustFixture f;
    const std::string vc = "Active=TRUE|Id=ID_VALUECURVE_Twinkle_Steps|Type=Flat|Min=2.00|Max=400.00|P1=50.00|RV=TRUE|";
    Effect* e = f.MigrateWithCurve("Twinkle", "2026.01", "", "E_VALUECURVE_Twinkle_Steps", vc);
    CHECK(e->GetSettings().Get("E_VALUECURVE_Twinkle_Steps", "") == vc);
}

TEST_CASE("Garlands migration moves the Cycles slider to a text control", "[effects][adjust]") {
    AdjustFixture f;
    Effect* e = f.Migrate("Garlands", "2026.01", "E_SLIDER_Garlands_Cycles=7");
    CHECK_FALSE(e->GetSettings().Contains("E_SLIDER_Garlands_Cycles"));
    CHECK(e->GetSettings().Get("E_TEXTCTRL_Garlands_Cycles", "?") == "7");
}

TEST_CASE("Garlands migration without a Cycles slider adds nothing", "[effects][adjust]") {
    AdjustFixture f;
    Effect* e = f.Migrate("Garlands", "2026.01", "E_SLIDER_Garlands_Spacing=5");
    CHECK_FALSE(e->GetSettings().Contains("E_TEXTCTRL_Garlands_Cycles"));
    CHECK(e->GetSettings().Get("E_SLIDER_Garlands_Spacing", "?") == "5");
}

TEST_CASE("Garlands migration skips current sequences", "[effects][adjust]") {
    AdjustFixture f;
    Effect* e = f.Migrate("Garlands", "2026.06", "E_SLIDER_Garlands_Cycles=7");
    CHECK(e->GetSettings().Contains("E_SLIDER_Garlands_Cycles"));
}

TEST_CASE("Glediator migration renames the filename control", "[effects][adjust]") {
    AdjustFixture f;
    Effect* e = f.Migrate("Glediator", "2016.01", "E_TEXTCTRL_Glediator_Filename=show.gled");
    CHECK_FALSE(e->GetSettings().Contains("E_TEXTCTRL_Glediator_Filename"));
    CHECK(e->GetSettings().Get("E_FILEPICKERCTRL_Glediator_Filename", "?") == "show.gled");
}

TEST_CASE("Glediator migration leaves an empty filename alone", "[effects][adjust]") {
    AdjustFixture f;
    Effect* e = f.Migrate("Glediator", "2016.01", "E_TEXTCTRL_Glediator_Filename=");
    CHECK_FALSE(e->GetSettings().Contains("E_FILEPICKERCTRL_Glediator_Filename"));
}

TEST_CASE("Guitar migration converts wave factor sliders to float text controls", "[effects][adjust]") {
    AdjustFixture f;
    Effect* e = f.Migrate("Guitar", "2026.01", "E_SLIDER_BaseWaveFactor=25,E_SLIDER_StringWaveFactor=5");
    CHECK_FALSE(e->GetSettings().Contains("E_SLIDER_BaseWaveFactor"));
    CHECK_FALSE(e->GetSettings().Contains("E_SLIDER_StringWaveFactor"));
    CHECK(e->GetSettings().Get("E_TEXTCTRL_BaseWaveFactor", "?") == "2.5");
    CHECK(e->GetSettings().Get("E_TEXTCTRL_StringWaveFactor", "?") == "0.5");
}

TEST_CASE("Guitar migration keeps an already migrated text control", "[effects][adjust]") {
    AdjustFixture f;
    Effect* e = f.Migrate("Guitar", "2026.01", "E_SLIDER_BaseWaveFactor=25,E_TEXTCTRL_BaseWaveFactor=4.0");
    CHECK(e->GetSettings().Get("E_TEXTCTRL_BaseWaveFactor", "?") == "4.0");
    CHECK_FALSE(e->GetSettings().Contains("E_SLIDER_BaseWaveFactor"));
}

TEST_CASE("Guitar migration survives a malformed slider value", "[effects][adjust]") {
    AdjustFixture f;
    Effect* e = f.Migrate("Guitar", "2026.01", "E_SLIDER_BaseWaveFactor=abc");
    CHECK_FALSE(e->GetSettings().Contains("E_SLIDER_BaseWaveFactor"));
    CHECK(e->GetSettings().Get("E_TEXTCTRL_BaseWaveFactor", "?") == "1.0");
}

TEST_CASE("Guitar migration adds nothing when no wave factor is stored", "[effects][adjust]") {
    AdjustFixture f;
    Effect* e = f.Migrate("Guitar", "2026.01", "");
    CHECK_FALSE(e->GetSettings().Contains("E_TEXTCTRL_BaseWaveFactor"));
    CHECK_FALSE(e->GetSettings().Contains("E_TEXTCTRL_StringWaveFactor"));
}

TEST_CASE("Lightning migration removes the retired BOTY slider", "[effects][adjust]") {
    AdjustFixture f;
    Effect* e = f.Migrate("Lightning", "2023.01", "E_SLIDER_Lightning_BOTY=50,E_SLIDER_Number_Segments=5");
    CHECK_FALSE(e->GetSettings().Contains("E_SLIDER_Lightning_BOTY"));
    CHECK(e->GetSettings().Get("E_SLIDER_Number_Segments", "?") == "5");
}

TEST_CASE("Lines migration renames the speed slider to a text control", "[effects][adjust]") {
    AdjustFixture f;
    Effect* e = f.Migrate("Lines", "2026.05", "E_SLIDER_Lines_Speed=4");
    CHECK_FALSE(e->GetSettings().Contains("E_SLIDER_Lines_Speed"));
    CHECK(e->GetSettings().Get("E_TEXTCTRL_Lines_Speed", "?") == "4");
}

TEST_CASE("Lines migration does not overwrite an existing text control", "[effects][adjust]") {
    AdjustFixture f;
    Effect* e = f.Migrate("Lines", "2026.05", "E_SLIDER_Lines_Speed=4,E_TEXTCTRL_Lines_Speed=2.5");
    CHECK(e->GetSettings().Get("E_TEXTCTRL_Lines_Speed", "?") == "2.5");
    CHECK(e->GetSettings().Contains("E_SLIDER_Lines_Speed"));
}

TEST_CASE("Liquid migration scales velocity and converts warm up frames", "[effects][adjust]") {
    AdjustFixture f;
    REQUIRE(f.ctx._sequenceElements.GetFrameMS() == 50);
    Effect* e = f.Migrate("Liquid", "2026.01", "E_TEXTCTRL_Velocity1=10,E_TEXTCTRL_WarmUpFrames=10");
    CHECK(e->GetSettings().Get("E_TEXTCTRL_Velocity1", "?") == "1000");
    CHECK_FALSE(e->GetSettings().Contains("E_TEXTCTRL_WarmUpFrames"));
    CHECK(e->GetSettings().Get("E_TEXTCTRL_WarmUpTime", "?") == "50");
}

TEST_CASE("Liquid migration converts per frame flow to per second", "[effects][adjust]") {
    AdjustFixture f;
    Effect* e = f.Migrate("Liquid", "2026.01", "E_TEXTCTRL_Flow1=10,E_TEXTCTRL_Flow2=0");
    // 10 particles per 50ms frame is 200 per second; the slider maps 4 particles per second per unit
    CHECK(e->GetSettings().Get("E_TEXTCTRL_Flow1", "?") == "50");
    CHECK(e->GetSettings().Get("E_TEXTCTRL_Flow2", "?") == "0");
}

TEST_CASE("Liquid migration clamps scaled velocity", "[effects][adjust]") {
    AdjustFixture f;
    Effect* small = f.Migrate("Liquid", "2026.01", "E_TEXTCTRL_Velocity2=1");
    Effect* big = f.Migrate("Liquid", "2026.01", "E_TEXTCTRL_Velocity2=100000");
    CHECK(small->GetSettings().GetInt("E_TEXTCTRL_Velocity2", -1) == 100);
    CHECK(big->GetSettings().GetInt("E_TEXTCTRL_Velocity2", -1) < 100000 * 100);
    CHECK(big->GetSettings().GetInt("E_TEXTCTRL_Velocity2", -1) >= 100);
}

TEST_CASE("Liquid migration leaves current sequences untouched", "[effects][adjust]") {
    AdjustFixture f;
    Effect* e = f.Migrate("Liquid", "2026.08", "E_TEXTCTRL_Velocity1=10,E_TEXTCTRL_WarmUpFrames=10,E_TEXTCTRL_Flow1=10");
    CHECK(e->GetSettings().Get("E_TEXTCTRL_Velocity1", "?") == "10");
    CHECK(e->GetSettings().Get("E_TEXTCTRL_WarmUpFrames", "?") == "10");
    CHECK(e->GetSettings().Get("E_TEXTCTRL_Flow1", "?") == "10");
}

TEST_CASE("Liquid migration survives non numeric values", "[effects][adjust]") {
    AdjustFixture f;
    Effect* e = f.Migrate("Liquid", "2026.01", "E_TEXTCTRL_Velocity1=abc,E_TEXTCTRL_Flow1=xyz,E_TEXTCTRL_WarmUpFrames=q");
    CHECK(e->GetSettings().Get("E_TEXTCTRL_Velocity1", "?") == "0");
    CHECK(e->GetSettings().Get("E_TEXTCTRL_Flow1", "?") == "0");
    CHECK(e->GetSettings().Get("E_TEXTCTRL_WarmUpTime", "?") == "0");
}

TEST_CASE("Music migration converts ScaleNotes to the scaling choice", "[effects][adjust]") {
    AdjustFixture f;
    Effect* on = f.Migrate("Music Effect", "2021.01", "E_CHECKBOX_Music_ScaleNotes=1");
    CHECK(on->GetSettings().Get("E_CHOICE_Music_Scaling", "?") == "Individual Notes");
    CHECK_FALSE(on->GetSettings().Contains("E_CHECKBOX_Music_ScaleNotes"));
    Effect* off = f.Migrate("Music Effect", "2021.01", "E_CHECKBOX_Music_ScaleNotes=0");
    CHECK(off->GetSettings().Get("E_CHOICE_Music_Scaling", "?") == "All Notes");
    CHECK_FALSE(off->GetSettings().Contains("E_CHECKBOX_Music_ScaleNotes"));
}

TEST_CASE("Music migration fixes the Seperate typo", "[effects][adjust]") {
    AdjustFixture f;
    CHECK(f.Migrate("Music Effect", "2021.01", "E_CHOICE_Music_Type=Seperate")->GetSettings().Get("E_CHOICE_Music_Type", "?") == "Separate");
    CHECK(f.Migrate("Music Effect", "2021.01", "E_CHOICE_Music_Type=Combined")->GetSettings().Get("E_CHOICE_Music_Type", "?") == "Combined");
}

TEST_CASE("Off migration converts the transparent checkbox into a style", "[effects][adjust]") {
    AdjustFixture f;
    Effect* t = f.Migrate("Off", "2024.10", "E_CHECKBOX_Off_Transparent=1");
    CHECK(t->GetSettings().Get("E_CHOICE_Off_Style", "?") == "Transparent");
    CHECK_FALSE(t->GetSettings().Contains("E_CHECKBOX_Off_Transparent"));

    Effect* c = f.Migrate("Off", "2024.10", "E_CHECKBOX_Off_Transparent=1,T_CHECKBOX_Canvas=1");
    CHECK(c->GetSettings().Get("E_CHOICE_Off_Style", "?") == "Transparent -> Black");

    Effect* n = f.Migrate("Off", "2024.10", "E_CHECKBOX_Off_Transparent=0");
    CHECK_FALSE(n->GetSettings().Contains("E_CHOICE_Off_Style"));
    CHECK_FALSE(n->GetSettings().Contains("E_CHECKBOX_Off_Transparent"));
}

TEST_CASE("Pictures migration converts ScaleToFit and drops ForceGIFOverlay", "[effects][adjust]") {
    AdjustFixture f;
    Effect* a = f.Migrate("Pictures", "2016.01", "E_CHECKBOX_Pictures_ScaleToFit=1,E_CHECKBOX_Pictures_ForceGIFOverlay=1");
    CHECK(a->GetSettings().Get("E_CHOICE_Scaling", "?") == "Scale To Fit");
    CHECK_FALSE(a->GetSettings().Contains("E_CHECKBOX_Pictures_ScaleToFit"));
    CHECK_FALSE(a->GetSettings().Contains("E_CHECKBOX_Pictures_ForceGIFOverlay"));
    Effect* b = f.Migrate("Pictures", "2016.01", "E_CHECKBOX_Pictures_ScaleToFit=0");
    CHECK(b->GetSettings().Get("E_CHOICE_Scaling", "?") == "No Scaling");
}

TEST_CASE("Ripple migration copies the generic SVG setting to the Ripple one", "[effects][adjust]") {
    AdjustFixture f;
    Effect* e = f.Migrate("Ripple", "2023.01", "E_FILEPICKERCTRL_SVG=shape.svg");
    CHECK(e->GetSettings().Get("E_FILEPICKERCTRL_Ripple_SVG", "?") == "shape.svg");
    Effect* keep = f.Migrate("Ripple", "2023.01", "E_FILEPICKERCTRL_SVG=a.svg,E_FILEPICKERCTRL_Ripple_SVG=b.svg");
    CHECK(keep->GetSettings().Get("E_FILEPICKERCTRL_Ripple_SVG", "?") == "b.svg");
}

TEST_CASE("Servo migration converts a Flat value curve to start and end values", "[effects][adjust]") {
    AdjustFixture f;
    Effect* e = f.MigrateWithCurve("Servo", "2024.01", "", "E_VALUECURVE_Servo", "Active=TRUE|Id=ID_VALUECURVE_Servo|Type=Flat|Min=0.00|Max=100.00|P1=50.00|RV=TRUE|");
    CHECK_FALSE(e->GetSettings().Contains("E_VALUECURVE_Servo"));
    CHECK(e->GetSettings().Get("E_TEXTCTRL_Servo", "?") == "5.0");
    CHECK(e->GetSettings().Get("E_TEXTCTRL_EndValue", "?") == "5.0");
}

TEST_CASE("Servo migration converts a Ramp value curve to start and end values", "[effects][adjust]") {
    AdjustFixture f;
    Effect* e = f.MigrateWithCurve("Servo", "2024.01", "", "E_VALUECURVE_Servo", "Active=TRUE|Id=ID_VALUECURVE_Servo|Type=Ramp|Min=0.00|Max=100.00|P1=20.00|P2=80.00|RV=TRUE|");
    CHECK_FALSE(e->GetSettings().Contains("E_VALUECURVE_Servo"));
    CHECK(e->GetSettings().Get("E_TEXTCTRL_Servo", "?") == "2.0");
    CHECK(e->GetSettings().Get("E_TEXTCTRL_EndValue", "?") == "8.0");
}

TEST_CASE("Servo migration defaults EndValue to the start value or zero", "[effects][adjust]") {
    AdjustFixture f;
    CHECK(f.Migrate("Servo", "2024.01", "E_TEXTCTRL_Servo=5.5")->GetSettings().Get("E_TEXTCTRL_EndValue", "?") == "5.5");
    CHECK(f.Migrate("Servo", "2024.01", "")->GetSettings().Get("E_TEXTCTRL_EndValue", "?") == "0");
    CHECK(f.Migrate("Servo", "2024.01", "E_TEXTCTRL_Servo=5.5,E_TEXTCTRL_EndValue=9")->GetSettings().Get("E_TEXTCTRL_EndValue", "?") == "9");
}

TEST_CASE("Shape migration flips CentreY for emoji", "[effects][adjust]") {
    AdjustFixture f;
    Effect* emoji = f.Migrate("Shape", "2024.01", "E_CHOICE_Shape_ObjectToDraw=Emoji,E_SLIDER_Shape_CentreY=30");
    CHECK(emoji->GetSettings().Get("E_SLIDER_Shape_CentreY", "?") == "70");
    Effect* circle = f.Migrate("Shape", "2024.01", "E_CHOICE_Shape_ObjectToDraw=Circle,E_SLIDER_Shape_CentreY=30");
    CHECK(circle->GetSettings().Get("E_SLIDER_Shape_CentreY", "?") == "30");
    Effect* nocentre = f.Migrate("Shape", "2024.01", "E_CHOICE_Shape_ObjectToDraw=Emoji");
    CHECK_FALSE(nocentre->GetSettings().Contains("E_SLIDER_Shape_CentreY"));
}

TEST_CASE("SingleStrand migration turns the 3D fade checkbox into a fade type", "[effects][adjust]") {
    AdjustFixture f;
    Effect* checked = f.Migrate("SingleStrand", "2024.10", "E_CHECKBOX_Chase_3dFade1=1");
    CHECK(checked->GetSettings().Get("E_CHOICE_Fade_Type", "?") == "From Head");
    CHECK_FALSE(checked->GetSettings().Contains("E_CHECKBOX_Chase_3dFade1"));
    Effect* unchecked = f.Migrate("SingleStrand", "2024.10", "E_CHECKBOX_Chase_3dFade1=0");
    CHECK(unchecked->GetSettings().Get("E_CHOICE_Fade_Type", "?") == "None");
    CHECK_FALSE(unchecked->GetSettings().Contains("E_CHECKBOX_Chase_3dFade1"));
    Effect* absent = f.Migrate("SingleStrand", "2024.10", "");
    CHECK_FALSE(absent->GetSettings().Contains("E_CHOICE_Fade_Type"));
}

TEST_CASE("SingleStrand migration renames Dual Bounce", "[effects][adjust]") {
    AdjustFixture f;
    CHECK(f.Migrate("SingleStrand", "2020.10", "E_CHOICE_Chase_Type1=Dual Bounce")->GetSettings().Get("E_CHOICE_Chase_Type1", "?") == "Dual Chase");
    CHECK(f.Migrate("SingleStrand", "2020.10", "E_CHOICE_Chase_Type1=Bounce from Left")->GetSettings().Get("E_CHOICE_Chase_Type1", "?") == "Bounce from Left");
    CHECK(f.Migrate("SingleStrand", "2024.20", "E_CHOICE_Chase_Type1=Dual Bounce")->GetSettings().Get("E_CHOICE_Chase_Type1", "?") == "Dual Bounce");
}

TEST_CASE("SingleStrand migration rewrites single line buffers on submodels", "[effects][adjust]") {
    AdjustFixture f("Parent/Sub");
    Effect* e = f.Migrate("SingleStrand", "2024.01", "B_CHOICE_BufferStyle=Single Line,B_CHOICE_BufferTransform=");
    CHECK(e->GetSettings().Get("B_CHOICE_BufferStyle", "?") == "Horizontal Per Strand");
    CHECK(e->GetSettings().Get("E_CHECKBOX_Chase_Group_All", "?") == "1");
    CHECK(e->GetSettings().Get("B_CHOICE_BufferTransform", "?") == "Flip Vertical");
}

TEST_CASE("SingleStrand migration leaves single line buffers on plain models", "[effects][adjust]") {
    AdjustFixture f;
    Effect* e = f.Migrate("SingleStrand", "2024.01", "B_CHOICE_BufferStyle=Single Line");
    CHECK(e->GetSettings().Get("B_CHOICE_BufferStyle", "?") == "Single Line");
}

TEST_CASE("VUMeter migration sets Sensitivity for the Timing Event Color type", "[effects][adjust]") {
    AdjustFixture f;
    Effect* e = f.Migrate("VU Meter", "2022.01", "E_CHOICE_VUMeter_Type=Timing Event Color");
    CHECK(e->GetSettings().Get("E_SLIDER_VUMeter_Sensitivity", "?") == "100");
}

TEST_CASE("VUMeter migration leaves other types alone", "[effects][adjust]") {
    AdjustFixture f;
    Effect* e = f.Migrate("VU Meter", "2022.01", "E_CHOICE_VUMeter_Type=Spectrogram");
    CHECK_FALSE(e->GetSettings().Contains("E_SLIDER_VUMeter_Sensitivity"));
}

TEST_CASE("Video migration turns the Loop checkbox into a duration treatment", "[effects][adjust]") {
    AdjustFixture f;
    Effect* e = f.Migrate("Video", "2026.01", "E_CHECKBOX_Video_Loop=1,E_SLIDER_Video_Starttime=500");
    CHECK(e->GetSettings().Get("E_CHOICE_Video_DurationTreatment", "?") == "Loop");
    CHECK_FALSE(e->GetSettings().Contains("E_CHECKBOX_Video_Loop"));
    CHECK_FALSE(e->GetSettings().Contains("E_SLIDER_Video_Starttime"));
}

TEST_CASE("Video migration without Loop keeps the duration treatment", "[effects][adjust]") {
    AdjustFixture f;
    Effect* e = f.Migrate("Video", "2026.01", "E_CHECKBOX_Video_Loop=0,E_CHOICE_Video_DurationTreatment=Slow/Accelerate");
    CHECK(e->GetSettings().Get("E_CHOICE_Video_DurationTreatment", "?") == "Slow/Accelerate");
}

TEST_CASE("Wave migration converts the speed slider and number of waves", "[effects][adjust]") {
    AdjustFixture f;
    Effect* e = f.Migrate("Wave", "2022.01", "E_SLIDER_Wave_Speed=10,E_SLIDER_Number_Waves=900");
    CHECK_FALSE(e->GetSettings().Contains("E_SLIDER_Wave_Speed"));
    CHECK(e->GetSettings().Get("E_TEXTCTRL_Wave_Speed", "?") == "10");
    CHECK_FALSE(e->GetSettings().Contains("E_SLIDER_Number_Waves"));
    CHECK(e->GetSettings().Get("E_TEXTCTRL_Number_Waves", "?") == "2.50");
}

TEST_CASE("Wave migration of waves only applies before 2026.05.2", "[effects][adjust]") {
    AdjustFixture f;
    Effect* e = f.Migrate("Wave", "2025.01", "E_SLIDER_Wave_Speed=10,E_SLIDER_Number_Waves=360");
    CHECK(e->GetSettings().Get("E_TEXTCTRL_Number_Waves", "?") == "1.00");
    CHECK(e->GetSettings().Get("E_SLIDER_Wave_Speed", "?") == "10");
    Effect* cur = f.Migrate("Wave", "2026.06", "E_SLIDER_Number_Waves=360");
    CHECK(cur->GetSettings().Get("E_SLIDER_Number_Waves", "?") == "360");
}

TEST_CASE("Shader migration namespaces effect settings except the well known ones", "[effects][adjust]") {
    AdjustFixture f;
    Effect* e = f.Migrate("Shader", "2020.01", "E_SLIDER_Foo=1,E_TEXTCTRL_Shader_Zoom=3,B_CHOICE_BufferStyle=Default");
    CHECK_FALSE(e->GetSettings().Contains("E_SLIDER_Foo"));
    CHECK(e->GetSettings().Get("E_SLIDER_SHADERXYZZY_Foo", "?") == "1");
    CHECK(e->GetSettings().Get("E_TEXTCTRL_Shader_Zoom", "?") == "3");
    CHECK(e->GetSettings().Get("B_CHOICE_BufferStyle", "?") == "Default");
}

TEST_CASE("Shader migration is idempotent", "[effects][adjust]") {
    AdjustFixture f;
    Effect* e = f.Migrate("Shader", "2020.01", "E_SLIDER_Foo=1,E_CHOICE_Bar_Baz=x");
    const std::string once = e->GetSettingsAsString();
    f.Fx("Shader")->adjustSettings("2020.01", e);
    CHECK(e->GetSettingsAsString() == once);
}

TEST_CASE("Effects with no migration leave their settings unchanged", "[effects][adjust]") {
    AdjustFixture f;
    for (const char* name : { "Text", "Sketch", "Bars", "Color Wash", "On", "Marquee", "Strobe", "Plasma" }) {
        INFO("effect " << name);
        const std::string settings = "E_SLIDER_Example=3,E_CHOICE_Example=Left,B_CHOICE_BufferStyle=Default";
        Effect* e = f.Migrate(name, "2000.01", settings);
        CHECK(e->GetSettingsAsString() == "B_CHOICE_BufferStyle=Default,E_CHOICE_Example=Left,E_SLIDER_Example=3");
    }
}

TEST_CASE("Base migration is a no-op when the element is not a submodel", "[effects][adjust]") {
    AdjustFixture f;
    Effect* e = f.Migrate("Bars", "2020.01", "B_CHOICE_BufferStyle=Single Line");
    CHECK(e->GetSettings().Get("B_CHOICE_BufferStyle", "?") == "Single Line");
}

TEST_CASE("Migrating a current sequence leaves every effect's settings unchanged", "[effects][adjust]") {
    AdjustFixture f;
    const std::string settings = "B_CHOICE_BufferStyle=Default,E_CHOICE_Example=Left,E_SLIDER_Example=3,T_CHECKBOX_Canvas=0";
    for (auto* fx : f.ctx.effectManager) {
        INFO("effect " << fx->Name());
        if (fx->GetId() == EffectManager::eff_SERVO || fx->GetId() == EffectManager::eff_SHADER) {
            // Servo always ensures an EndValue and Shader always namespaces unknown E_ settings
            continue;
        }
        Effect* e = f.Make(fx->Name(), settings);
        REQUIRE(e != nullptr);
        fx->adjustSettings("2999.01", e);
        CHECK(e->GetSettingsAsString() == settings);
    }
}

TEST_CASE("Servo migration of a current sequence only adds EndValue", "[effects][adjust]") {
    AdjustFixture f;
    Effect* e = f.Migrate("Servo", "2999.01", "E_SLIDER_Example=3");
    CHECK(e->GetSettings().Get("E_SLIDER_Example", "?") == "3");
    CHECK(e->GetSettings().Get("E_TEXTCTRL_EndValue", "?") == "0");
}

TEST_CASE("Migration is idempotent for effects whose migrations are rewrites", "[effects][adjust]") {
    AdjustFixture f;
    struct Case {
        const char* effect;
        const char* version;
        const char* settings;
    };
    const std::vector<Case> cases = {
        { "Circles", "2026.01", "E_CHECKBOX_Circles_Collide=1" },
        { "Fan", "2024.01", "E_CHECKBOX_Fan_Scale=1" },
        { "Fill", "2016.01", "E_SLIDER_X=1" },
        { "Fire", "2026.01", "" },
        { "Garlands", "2026.01", "E_SLIDER_Garlands_Cycles=7" },
        { "Glediator", "2016.01", "E_TEXTCTRL_Glediator_Filename=a.gled" },
        { "Guitar", "2026.01", "E_SLIDER_BaseWaveFactor=25" },
        { "Lightning", "2023.01", "E_SLIDER_Lightning_BOTY=5" },
        { "Lines", "2026.01", "E_SLIDER_Lines_Speed=3" },
        { "Music Effect", "2021.01", "E_CHECKBOX_Music_ScaleNotes=1,E_CHOICE_Music_Type=Seperate" },
        { "Off", "2024.01", "E_CHECKBOX_Off_Transparent=1" },
        { "Pictures", "2016.01", "E_CHECKBOX_Pictures_ScaleToFit=1" },
        { "Pinwheel", "2026.01", "" },
        { "Ripple", "2023.01", "E_FILEPICKERCTRL_SVG=a.svg" },
        { "Servo", "2024.01", "E_TEXTCTRL_Servo=3" },
        { "Shape", "2024.01", "E_CHOICE_Shape_ObjectToDraw=Circle" },
        { "SingleStrand", "2024.10", "E_CHECKBOX_Chase_3dFade1=1" },
        { "Twinkle", "2026.01", "" },
        { "Video", "2026.01", "E_CHECKBOX_Video_Loop=1" },
        { "Wave", "2022.01", "E_SLIDER_Wave_Speed=10,E_SLIDER_Number_Waves=720" },
    };
    for (const auto& c : cases) {
        INFO("effect " << c.effect);
        Effect* e = f.Migrate(c.effect, c.version, c.settings);
        const std::string once = e->GetSettingsAsString();
        f.Fx(c.effect)->adjustSettings(c.version, e);
        CHECK(e->GetSettingsAsString() == once);
    }
}

TEST_CASE("Migration does not crash on empty or malformed values for any effect", "[effects][adjust]") {
    AdjustFixture f;
    const std::string nasty =
        "E_SLIDER_A=,E_TEXTCTRL_B=not a number,E_CHECKBOX_C=,E_CHOICE_D=,E_VALUECURVE_E=garbage,"
        "E_VALUECURVE_Servo=Active=FALSE|,E_VALUECURVE_Twinkle_Steps=VALUECURVE Max=100.00,"
        "E_SLIDER_Garlands_Cycles=,E_SLIDER_Number_Waves=x,E_SLIDER_Wave_Speed=y,"
        "E_TEXTCTRL_Velocity1=,E_TEXTCTRL_Flow1=-,E_TEXTCTRL_WarmUpFrames=9999999999999,"
        "E_SLIDER_BaseWaveFactor=,E_CHECKBOX_Music_ScaleNotes=,E_CHOICE_Shape_ObjectToDraw=Emoji,E_SLIDER_Shape_CentreY=zz,"
        "E_VALUECURVE_Chase_Rotations=VALUECURVE";
    for (auto* fx : f.ctx.effectManager) {
        for (const char* version : { "2000.01", "2018.30", "2024.04", "2026.07.1", "2999.01" }) {
            INFO("effect " << fx->Name() << " version " << version);
            Effect* e = f.Make(fx->Name(), nasty);
            REQUIRE(e != nullptr);
            fx->adjustSettings(version, e);
            SUCCEED();
        }
    }
}

TEST_CASE("Migration of an empty settings map succeeds for every effect", "[effects][adjust]") {
    AdjustFixture f;
    for (auto* fx : f.ctx.effectManager) {
        INFO("effect " << fx->Name());
        Effect* e = f.Make(fx->Name(), "");
        REQUIRE(e != nullptr);
        fx->adjustSettings("2000.01", e);
        fx->adjustSettings("2999.01", e);
        SUCCEED();
    }
}
