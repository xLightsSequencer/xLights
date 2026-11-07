#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/catch_approx.hpp>
using Catch::Approx;

#include "import_export/LOREdit.h"
#include "test_impexp2_lor_common.h"

#include <string>

using namespace lortest;

namespace {
bool Has(const std::string& s, const std::string& sub) { return s.find(sub) != std::string::npos; }
}

#define HAS(c, sub)                                    \
    do {                                               \
        INFO(c.settings);                              \
        CHECK(c.settings.find(sub) != std::string::npos); \
    } while (0)
#define HASNOT(c, sub)                                 \
    do {                                               \
        INFO(c.settings);                              \
        CHECK(c.settings.find(sub) == std::string::npos); \
    } while (0)

// ---------- GetxLightsEffect name mapping ----------

TEST_CASE("LOR effect type names map to xLights effects", "[impexp2][lor][effects]") {
    struct Row { const char* lor; const char* xl; };
    auto r = GENERATE(Row{ "INTENSITY", "On" }, Row{ "DMX_INTENSITY", "DMX" }, Row{ "SHIMMER", "On" },
                      Row{ "colorwash", "Color Wash" }, Row{ "picture", "Pictures" }, Row{ "picturexy", "Pictures" },
                      Row{ "lineshorizontal", "Lines" }, Row{ "linesvertical", "Lines" }, Row{ "straightlines", "Lines" },
                      Row{ "garland", "Garlands" }, Row{ "spinner", "Pinwheel" }, Row{ "spinfade", "Pinwheel" },
                      Row{ "blendedbars", "Bars" }, Row{ "singleblock", "Morph" }, Row{ "countdown", "Text" },
                      Row{ "starfield", "Shape" }, Row{ "movingshapes", "Shape" }, Row{ "simpleshape", "Shape" },
                      Row{ "bars", "Bars" }, Row{ "curtain", "Curtain" }, Row{ "fire", "Fire" }, Row{ "ripple", "Ripple" },
                      Row{ "spirals", "Spirals" }, Row{ "wave", "Wave" }, Row{ "plasma", "Plasma" }, Row{ "foo bar", "Foo Bar" },
                      Row{ "", "" });
    LOREditEffect e;
    e.effectType = r.lor;
    CHECK(e.GetxLightsEffect() == r.xl);
}

TEST_CASE("LOR movie effect maps to the xLights Video effect", "[impexp2][lor][effects][!mayfail]") {
    // BUG: GetxLightsEffect falls through to Capitalise("movie") = "Movie", which is not an xLights effect (it is "Video")
    LOREditEffect e;
    e.effectType = "movie";
    CHECK(e.GetxLightsEffect() == "Video");
}

// ---------- Palette ----------

TEST_CASE("LOR track palette parsing", "[impexp2][lor][palette]") {
    struct Row { const char* pal; const char* expect; const char* notExpect; };
    auto r = GENERATE(Row{ "FFFF0000,1", "C_BUTTON_Palette1=#FF0000,C_CHECKBOX_Palette1=1", "Palette2" },
                      Row{ "FFFF0000,0", "C_BUTTON_Palette1=#FF0000", "C_CHECKBOX_Palette1" },
                      Row{ "FFFF0000,1;FF00FF00,1;FF0000FF,0", "C_BUTTON_Palette3=#0000FF", "C_CHECKBOX_Palette3" },
                      Row{ "FFFF0000,FF00FF00,1", "C_BUTTON_Palette1=Active=TRUE|Id=ID_BUTTON_Palette1|Values=x=0.000^c=#FF0000;x=1.000^c=#00FF00|", "" },
                      Row{ "FFFF0000,FFFF0000,1", "C_BUTTON_Palette1=#FF0000,C_CHECKBOX_Palette1=1", "Values=" },
                      Row{ "FFFF0000,1;", "C_BUTTON_Palette1=#FF0000", "Palette2" },
                      Row{ "", "", "C_BUTTON" },
                      Row{ "garbage", "", "C_BUTTON" },
                      Row{ "FFFF0000,1;x;FF00FF00,1", "C_BUTTON_Palette1=#FF0000", "Palette2" });
    auto c = Convert(Settings(Layer("colorwash", "full,full,none", r.pal)));
    REQUIRE(c.ok);
    INFO(c.palette);
    if (*r.expect) CHECK(Has(c.palette, r.expect));
    if (*r.notExpect) CHECK_FALSE(Has(c.palette, r.notExpect));
}

TEST_CASE("LOR palette gradient entry keeps its comma separator", "[impexp2][lor][palette]") {
    auto c = Convert(Settings(Layer("colorwash", "full,full,none", "FF000000,1;FFFF0000,FF00FF00,1")));
    REQUIRE(c.ok);
    CHECK(Has(c.palette, ",C_BUTTON_Palette2=Active=TRUE"));
}

TEST_CASE("LOR palette entries too short to hold a colour do not throw", "[impexp2][lor][palette][!mayfail]") {
    // BUG: GetPalette does cc[0].substr(2) with no size check; "F,1" or ",1" throws std::out_of_range
    auto doc = Parse(OneEffectDoc(Settings(Layer("colorwash", "full,full,none", "F,1"))));
    LOREdit l(doc, 20);
    auto v = l.GetTrackEffects("P", 0, 0);
    REQUIRE(v.size() == 1);
    CHECK_NOTHROW(v[0].GetPalette());
}

TEST_CASE("LOR palette empty colour field does not throw", "[impexp2][lor][palette][!mayfail]") {
    // BUG: same substr(2) on an empty first field (",1")
    auto doc = Parse(OneEffectDoc(Settings(Layer("colorwash", "full,full,none", ",1"))));
    LOREdit l(doc, 20);
    auto v = l.GetTrackEffects("P", 0, 0);
    REQUIRE(v.size() == 1);
    CHECK_NOTHROW(v[0].GetPalette());
}

TEST_CASE("LOR palette sparkle and brightness", "[impexp2][lor][palette]") {
    SECTION("sparkle from field 2") {
        auto c = Convert(Settings(Layer("colorwash", "full,full,none"), kNone, "Mix_Average", "0", "35"));
        CHECK(Has(c.palette, "C_SLIDER_SparkleFrequency=35"));
    }
    SECTION("zero sparkle") {
        auto c = Convert(Settings(Layer("colorwash", "full,full,none"), kNone, "Mix_Average", "0", "0"));
        CHECK_FALSE(Has(c.palette, "Sparkle"));
    }
    SECTION("negative sparkle ignored") {
        auto c = Convert(Settings(Layer("colorwash", "full,full,none"), kNone, "Mix_Average", "0", "-5"));
        CHECK_FALSE(Has(c.palette, "Sparkle"));
    }
    SECTION("non numeric sparkle ignored") {
        auto c = Convert(Settings(Layer("colorwash", "full,full,none"), kNone, "Mix_Average", "0", "abc"));
        CHECK_FALSE(Has(c.palette, "Sparkle"));
    }
    SECTION("full brightness untouched") {
        auto c = Convert(Settings(Layer("colorwash", "full,full,none")), 0, 100, " intensity=\"100\"");
        CHECK_FALSE(Has(c.palette, "Brightness"));
    }
    SECTION("fixed brightness") {
        auto c = Convert(Settings(Layer("colorwash", "full,full,none")), 0, 100, " intensity=\"40\"");
        CHECK(Has(c.palette, "C_SLIDER_Brightness=40"));
    }
    SECTION("ramp brightness") {
        auto c = Convert(Settings(Layer("colorwash", "full,full,none")), 0, 100, " startIntensity=\"10\" endIntensity=\"90\"");
        CHECK(Has(c.palette, "C_VALUECURVE_Brightness=Active=TRUE|Id=ID_VALUECURVE_Brightness|Type=Ramp|Min=0.00|Max=400.00|P1=10|P2=90|RV=TRUE|"));
    }
    SECTION("missing intensity defaults to 100") {
        auto c = Convert(Settings(Layer("colorwash", "full,full,none")), 0, 100, "");
        CHECK_FALSE(Has(c.palette, "Brightness"));
    }
    SECTION("zero intensity") {
        auto c = Convert(Settings(Layer("colorwash", "full,full,none")), 0, 100, " intensity=\"0\"");
        CHECK(Has(c.palette, "C_SLIDER_Brightness=0"));
    }
}

TEST_CASE("LOR channel palette is a plain colour or a ramp", "[impexp2][lor][palette]") {
    LOREditEffect e;
    e.type = loreditType::CHANNELS;
    e.startColour = xlColor(255, 0, 0);
    e.endColour = xlColor(255, 0, 0);
    CHECK(e.GetPalette() == "C_BUTTON_Palette1=#ff0000,C_CHECKBOX_Palette1=1,C_BUTTON_Palette2=#000000,C_CHECKBOX_Palette2=0");
    e.endColour = xlColor(0, 0, 255);
    auto p = e.GetPalette();
    CHECK(Has(p, "Values=x=0.000^c=#ff0000;x=1.000^c=#0000ff|"));
    CHECK(Has(p, "C_CHECKBOX_Palette1=1"));
}

TEST_CASE("LOR palette with no effect settings is empty", "[impexp2][lor][palette]") {
    LOREditEffect e;
    e.type = loreditType::TRACKS;
    CHECK(e.GetPalette().empty());
}

// ---------- Per effect settings ----------

#define LOR_CASE(NAME, TYPE, PARAMS, ...)                                              \
    TEST_CASE("LOR convert " NAME, "[impexp2][lor][effects]") {                        \
        auto c = Convert(Settings(Layer(TYPE, PARAMS)));                               \
        REQUIRE(c.ok);                                                                 \
        __VA_ARGS__                                                                    \
    }

LOR_CASE("butterfly linear", "butterfly", "linear,5,3,forward,0,10,rainbow",
         CHECK(c.name == "Butterfly"); HAS(c, ",E_SLIDER_Butterfly_Style=1"); HAS(c, "E_CHOICE_Butterfly_Colors=Rainbow");
         HAS(c, "E_CHOICE_Butterfly_Direction=Forward"); HAS(c, "E_SLIDER_Butterfly_Chunks=5"); HAS(c, "E_SLIDER_Butterfly_Skip=3");
         HAS(c, "E_SLIDER_Butterfly_Speed=10"); HAS(c, "T_CHOICE_LayerMethod=Normal");)
LOR_CASE("butterfly radial hue", "butterfly", "radial,5,3,forward,359,10,palette",
         HAS(c, "Butterfly_Style=2"); CHECK(Has(c.palette, "C_SLIDER_Color_HueAdjust=100"));)
LOR_CASE("butterfly blocks", "butterfly", "blocks,1,2,backward,0,0,palette", HAS(c, "Butterfly_Style=3"); HAS(c, "Direction=Backward");)
LOR_CASE("butterfly corner", "butterfly", "corner,1,2,backward,0,0,palette", HAS(c, "Butterfly_Style=5");)
LOR_CASE("butterfly unknown style", "butterfly", "weird,1,2,backward,0,0,palette", HASNOT(c, "Butterfly_Style");)
LOR_CASE("butterfly ramped chunks", "butterfly", "linear,R2R8,3,forward,0,10,rainbow",
         HAS(c, "E_VALUECURVE_Butterfly_Chunks=Active=TRUE|Id=ID_VALUECURVE_Butterfly_Chunks|Type=Ramp");)
LOR_CASE("butterfly out of range chunks clamp", "butterfly", "linear,99,99,forward,0,999,rainbow",
         HAS(c, "E_SLIDER_Butterfly_Chunks=10"); HAS(c, "E_SLIDER_Butterfly_Skip=10"); HAS(c, "E_SLIDER_Butterfly_Speed=50");)
LOR_CASE("butterfly negative values clamp", "butterfly", "linear,-4,-4,forward,0,-9,rainbow",
         HAS(c, "E_SLIDER_Butterfly_Chunks=1"); HAS(c, "E_SLIDER_Butterfly_Skip=2"); HAS(c, "E_SLIDER_Butterfly_Speed=0");)
LOR_CASE("butterfly too few params", "butterfly", "linear", HAS(c, "E_SLIDER_Butterfly_Style=1"); HAS(c, "E_SLIDER_Butterfly_Chunks=1");)
LOR_CASE("butterfly empty params", "butterfly", "", CHECK(c.name == "Butterfly"); HAS(c, "T_CHOICE_LayerMethod");)

LOR_CASE("colorwash full", "colorwash", "full,full,none", CHECK(c.name == "Color Wash"); HASNOT(c, "HFade"); HASNOT(c, "VFade"); HASNOT(c, "Shimmer");)
LOR_CASE("colorwash horizontal", "colorwash", "left_to_right,full,none", HAS(c, "E_CHECKBOX_ColorWash_HFade=1"); HASNOT(c, "VFade");)
LOR_CASE("colorwash horizontal r2l", "colorwash", "right_to_left,full,none", HAS(c, "E_CHECKBOX_ColorWash_HFade=1");)
LOR_CASE("colorwash horizontal centre on", "colorwash", "center_on,full,none", HAS(c, "E_CHECKBOX_ColorWash_HFade=1");)
LOR_CASE("colorwash horizontal centre off", "colorwash", "center_off,full,none", HAS(c, "E_CHECKBOX_ColorWash_HFade=1");)
LOR_CASE("colorwash vertical", "colorwash", "full,top_to_bottom,none", HAS(c, "E_CHECKBOX_ColorWash_VFade=1"); HASNOT(c, "HFade");)
LOR_CASE("colorwash vertical b2t", "colorwash", "full,bottom_to_top,none", HAS(c, "E_CHECKBOX_ColorWash_VFade=1");)
LOR_CASE("colorwash vertical centre", "colorwash", "full,center_off,none", HAS(c, "E_CHECKBOX_ColorWash_VFade=1");)
LOR_CASE("colorwash unknown fades", "colorwash", "sideways,diagonal,none", HASNOT(c, "Fade");)
LOR_CASE("colorwash empty", "colorwash", "", HASNOT(c, "Fade"); CHECK(c.name == "Color Wash");)

TEST_CASE("LOR colorwash blink in unison turns on shimmer", "[impexp2][lor][effects]") {
    auto c = Convert(Settings(Layer("colorwash", "full,full,none"), kNone, "Mix_Average", "0", "0", "blink_in_unison"));
    REQUIRE(c.ok);
    HAS(c, "E_CHECKBOX_ColorWash_Shimmer=1");
    auto d = Convert(Settings(Layer("colorwash", "full,full,none"), kNone, "Mix_Average", "0", "0", "full"));
    HASNOT(d, "Shimmer");
}

TEST_CASE("LOR colorwash single colour keeps only the first checked palette entry", "[impexp2][lor][effects]") {
    auto c = Convert(Settings(Layer("colorwash", "full,full,single_color", "FFFF0000,1;FF00FF00,1;FF0000FF,1")));
    REQUIRE(c.ok);
    INFO(c.palette);
    CHECK(Has(c.palette, "C_CHECKBOX_Palette1=1"));
    CHECK_FALSE(Has(c.palette, "C_CHECKBOX_Palette2=1"));
    CHECK_FALSE(Has(c.palette, "C_CHECKBOX_Palette3=1"));
    CHECK(Has(c.palette, "C_BUTTON_Palette3=#0000FF"));
}

TEST_CASE("LOR colorwash single colour picks the first checked entry even when it is not palette 1", "[impexp2][lor][effects]") {
    auto c = Convert(Settings(Layer("colorwash", "full,full,single_color", "FFFF0000,0;FF00FF00,1;FF0000FF,1")));
    REQUIRE(c.ok);
    INFO(c.palette);
    CHECK_FALSE(Has(c.palette, "C_CHECKBOX_Palette1"));
    CHECK(Has(c.palette, "C_CHECKBOX_Palette2=1"));
    CHECK_FALSE(Has(c.palette, "C_CHECKBOX_Palette3=1"));
}

TEST_CASE("LOR colorwash single colour with no checked entries", "[impexp2][lor][effects]") {
    auto c = Convert(Settings(Layer("colorwash", "full,full,single_color", "FFFF0000,0;FF00FF00,0")));
    REQUIRE(c.ok);
    CHECK_FALSE(Has(c.palette, "C_CHECKBOX_Palette"));
}

TEST_CASE("LOR colorwash single colour with ten palette entries only matches single digit ids", "[impexp2][lor][effects][!mayfail]") {
    // BUG: the checkbox regex uses (\d) so Palette10 / Palette11 are matched as Palette1 followed by "0"/"1"
    std::string pal;
    for (int i = 0; i < 11; ++i) pal += std::string(i ? ";" : "") + "FFFF0000,1";
    auto c = Convert(Settings(Layer("colorwash", "full,full,single_color", pal)));
    REQUIRE(c.ok);
    CHECK_FALSE(Has(c.palette, "C_CHECKBOX_Palette10=1"));
}

LOR_CASE("spirals basic", "spirals", "1,left_to_right,20,50,0,False,trail_left,50",
         CHECK(c.name == "Spirals"); HAS(c, "E_SLIDER_Spirals_Count=1"); HAS(c, "E_TEXTCTRL_Spirals_Movement=2.5");
         HAS(c, "E_SLIDER_Spirals_Thickness=50"); HAS(c, "E_CHECKBOX_Spirals_3D=1"); HASNOT(c, "Spirals_Blend"); HAS(c, "E_SLIDER_Spirals_Rotation=");)
LOR_CASE("spirals right to left", "spirals", "3,right_to_left,20,50,0,True,trail_right,50",
         HAS(c, "E_TEXTCTRL_Spirals_Movement=-2.5"); HAS(c, "E_CHECKBOX_Spirals_Blend=1"); HAS(c, "E_CHECKBOX_Spirals_3D=1");)
LOR_CASE("spirals no 3d", "spirals", "2,left_to_right,20,50,0,False,none,50", HASNOT(c, "Spirals_3D");)
LOR_CASE("spirals thickness ramp", "spirals", "2,left_to_right,20,R10R90,0,False,none,50",
         HAS(c, "E_VALUECURVE_Spirals_Thickness=Active=TRUE");)
LOR_CASE("spirals count out of range", "spirals", "40,left_to_right,20,500,0,False,none,50",
         HAS(c, "E_SLIDER_Spirals_Count=5"); HAS(c, "E_SLIDER_Spirals_Thickness=100");)
LOR_CASE("spirals short params", "spirals", "2", HAS(c, "E_SLIDER_Spirals_Count=2");)

LOR_CASE("bars basic", "bars", "right,1,none,True,19,0",
         CHECK(c.name == "Bars"); HAS(c, "E_CHOICE_Bars_Direction=Right"); HAS(c, "E_SLIDER_Bars_BarCount=1"); HAS(c, "E_CHECKBOX_Bars_3D=1");
         HASNOT(c, "Highlight"); HAS(c, "E_TEXTCTRL_Bars_Cycles="); HAS(c, "E_TEXTCTRL_Bars_Center=0");)
LOR_CASE("bars highlight true", "bars", "left,2,True,False,8,0", HAS(c, "E_CHECKBOX_Bars_Highlight=1"); HASNOT(c, "Bars_3D"); HAS(c, "Direction=Left");)
LOR_CASE("bars highlight white", "bars", "up,2,white,False,8,0", HAS(c, "E_CHECKBOX_Bars_Highlight=1"); HAS(c, "Direction=up");)
LOR_CASE("bars highlight last", "bars", "down,2,last_color,False,8,0", HAS(c, "E_CHECKBOX_Bars_Highlight=1");)
LOR_CASE("bars highlight none", "bars", "down,2,none,False,8,0", HASNOT(c, "Highlight");)
LOR_CASE("bars diagonal down right", "bars", "down_right,2,none,False,8,0", HAS(c, "E_CHOICE_Bars_Direction=Custom,E_SLIDER_Bars_Angle=-45");)
LOR_CASE("bars diagonal down left", "bars", "down_left,2,none,False,8,0", HAS(c, "Bars_Angle=-135");)
LOR_CASE("bars diagonal up right", "bars", "up_right,2,none,False,8,0", HAS(c, "Bars_Angle=45");)
LOR_CASE("bars diagonal up left", "bars", "up_left,2,none,False,8,0", HAS(c, "Bars_Angle=135");)
LOR_CASE("bars v expand", "bars", "V_expand,2,none,False,8,0", HAS(c, "Bars_Direction=expand");)
LOR_CASE("bars v compress", "bars", "V_compress,2,none,False,8,0", HAS(c, "Bars_Direction=compress");)
LOR_CASE("bars h expand", "bars", "H_expand,2,none,False,8,0", HAS(c, "Bars_Direction=H-expand");)
LOR_CASE("bars h compress", "bars", "H_compress,2,none,False,8,0", HAS(c, "Bars_Direction=H-compress");)
LOR_CASE("bars block up", "bars", "block_up,2,none,False,8,0", HAS(c, "Bars_Direction=Alternate Up");)
LOR_CASE("bars block down", "bars", "block_down,2,none,False,8,0", HAS(c, "Bars_Direction=Alternate Down");)
LOR_CASE("bars block left", "bars", "block_left,2,none,False,8,0", HAS(c, "Bars_Direction=Alternate Left");)
LOR_CASE("bars block right", "bars", "block_right,2,none,False,8,0", HAS(c, "Bars_Direction=Alternate Right");)
LOR_CASE("bars centre extremes", "bars", "up,2,none,False,8,50", HAS(c, "E_TEXTCTRL_Bars_Center=100");)
LOR_CASE("bars centre negative", "bars", "up,2,none,False,8,-50", HAS(c, "E_TEXTCTRL_Bars_Center=-100");)
LOR_CASE("bars count clamps", "bars", "up,9,none,False,8,0", HAS(c, "E_SLIDER_Bars_BarCount=5");)
LOR_CASE("bars speed ramp does not crash", "bars", "up,2,none,False,R0R10,0", HAS(c, "E_TEXTCTRL_Bars_Cycles=");)
LOR_CASE("bars empty", "bars", "", HAS(c, "Bars_Direction=");)

LOR_CASE("countdown", "countdown", "5,Arial,75,7",
         CHECK(c.name == "Text"); HAS(c, "E_TEXTCTRL_Text=5"); HAS(c, "E_CHOICE_Text_Count=seconds"); HAS(c, "E_FONTPICKER_Text_Font='Arial' 75");
         HAS(c, "E_SLIDER_Text_XStart=28");)
LOR_CASE("countdown font size clamps", "countdown", "5,Arial,500,-90", HAS(c, "Text_Font='Arial' 100"); HAS(c, "E_SLIDER_Text_XStart=-200");)

LOR_CASE("lines horizontal has no xLights equivalent but converts", "lineshorizontal", "Bottom_to_Top,8,38",
         CHECK(c.name == "Lines"); HAS(c, "T_CHOICE_LayerMethod=Normal");)
LOR_CASE("lines vertical", "linesvertical", "Left_to_Right,4,32", CHECK(c.name == "Lines");)
LOR_CASE("straight lines", "straightlines", "1,2,3", CHECK(c.name == "Lines"); CHECK(c.settings == ",T_CHOICE_LayerMethod=Normal");)

LOR_CASE("curtain basic", "curtain", "center,open,0,once_at_speed,12,",
         CHECK(c.name == "Curtain"); HAS(c, "E_CHOICE_Curtain_Edge=center"); HAS(c, "E_CHOICE_Curtain_Effect=open"); HAS(c, "E_CHECKBOX_Curtain_Repeat=0");
         HAS(c, "E_TEXTCTRL_Curtain_Speed=2.4");)
LOR_CASE("curtain open close", "curtain", "left,open_then_close,2,repeat_at_speed_rotate_colors,12,", HAS(c, "E_CHOICE_Curtain_Effect=open then close"); HAS(c, "E_CHECKBOX_Curtain_Repeat=1");)
LOR_CASE("curtain chase", "curtain", "left,chase,2,repeat_at_speed_blend_colors,12,", HAS(c, "E_CHOICE_Curtain_Effect=open then close"); HAS(c, "E_CHECKBOX_Curtain_Repeat=1");)
LOR_CASE("curtain fit to duration", "curtain", "center,open,0,once_fit_to_duration,12,R0R100", HAS(c, "E_TEXTCTRL_Curtain_Speed=1.00"); HAS(c, "Repeat=0"); HASNOT(c, "VALUECURVE_Curtain_Speed");)
LOR_CASE("curtain fit to duration half", "curtain", "center,open,0,once_fit_to_duration,12,R0R50", HAS(c, "E_TEXTCTRL_Curtain_Speed=0.50");)
LOR_CASE("curtain fit to duration no ramp", "curtain", "center,open,0,once_fit_to_duration,12,", HAS(c, "E_TEXTCTRL_Curtain_Speed=1.00");)
LOR_CASE("curtain fit to duration offset start", "curtain", "center,open,0,once_fit_to_duration,12,R20R100", HAS(c, "E_TEXTCTRL_Curtain_Speed=1.00");)
LOR_CASE("curtain fit to duration reversed ramp", "curtain", "center,open,0,once_fit_to_duration,12,R100R0", HAS(c, "E_TEXTCTRL_Curtain_Speed=1.00");)
LOR_CASE("curtain unknown repeat", "curtain", "center,open,0,sometimes,12,", HASNOT(c, "Curtain_Repeat");)
LOR_CASE("curtain swag ramp", "curtain", "center,open,R0R10,once_at_speed,12,", HAS(c, "E_VALUECURVE_Curtain_Swag=Active=TRUE");)
LOR_CASE("curtain speed ramp", "curtain", "center,open,0,once_at_speed,R0R50,", HAS(c, "E_VALUECURVE_Curtain_Speed=Active=TRUE"); HAS(c, "Max=10.00");)

LOR_CASE("fire", "fire", "50,0", CHECK(c.name == "Fire"); HAS(c, "E_SLIDER_Fire_Height=44"); HAS(c, "E_SLIDER_Fire_HueShift=0");)
LOR_CASE("fire extremes", "fire", "500,500", HAS(c, "E_SLIDER_Fire_Height=100"); HAS(c, "E_SLIDER_Fire_HueShift=100");)
LOR_CASE("fire low extremes", "fire", "-5,-5", HAS(c, "E_SLIDER_Fire_Height=0"); HAS(c, "E_SLIDER_Fire_HueShift=0");)
LOR_CASE("fire non numeric", "fire", "abc,def", HAS(c, "E_SLIDER_Fire_Height=0");)

LOR_CASE("fireworks", "fireworks", "10,50,2,30,normal,continuous",
         CHECK(c.name == "Fireworks"); HAS(c, "E_SLIDER_Fireworks_Explosions=5"); HAS(c, "E_SLIDER_Fireworks_Count=50");
         HAS(c, "E_SLIDER_Fireworks_Velocity=2"); HAS(c, "E_SLIDER_Fireworks_Fade=30");)
LOR_CASE("fireworks extremes", "fireworks", "999,999,999,999", HAS(c, "Explosions=50"); HAS(c, "Count=100"); HAS(c, "Velocity=10"); HAS(c, "Fade=100");)

LOR_CASE("garland repeat", "garland", "3,34,repeat_at_speed,12,left_to_right",
         CHECK(c.name == "Garlands"); HAS(c, "E_SLIDER_Garlands_Type=3"); HAS(c, "E_CHOICE_Garlands_Direction=Right"); HAS(c, "E_SLIDER_Garlands_Spacing=34");
         HAS(c, "E_TEXTCTRL_Garlands_Cycles=4.8");)
LOR_CASE("garland once", "garland", "3,34,once_at_speed,12,bottom_to_top", HAS(c, "Garlands_Direction=Up"); HAS(c, "E_TEXTCTRL_Garlands_Cycles=1.0");)
LOR_CASE("garland fit", "garland", "3,34,once_fit_to_duration,12,top_to_bottom", HAS(c, "Garlands_Direction=Down"); HAS(c, "E_TEXTCTRL_Garlands_Cycles=1.0");)
LOR_CASE("garland r2l", "garland", "3,34,wibble,12,right_to_left", HAS(c, "Garlands_Direction=Left"); HASNOT(c, "Garlands_Cycles");)
LOR_CASE("garland no fill", "garland", "9,500,repeat_at_speed,12,", HAS(c, "Garlands_Type=4"); HAS(c, "Spacing=100"); HASNOT(c, "Garlands_Direction");)
LOR_CASE("garland spacing ramp", "garland", "3,R10R90,repeat_at_speed,12,bottom_to_top", HAS(c, "E_VALUECURVE_Garlands_Spacing=Active=TRUE");)
LOR_CASE("garland speed ramp", "garland", "3,34,repeat_at_speed,R0R50,bottom_to_top", HAS(c, "E_VALUECURVE_Garlands_Cycles=Active=TRUE");)

LOR_CASE("marquee", "marquee", "4,1,1,0,12",
         CHECK(c.name == "Marquee"); HAS(c, "E_SLIDER_Marquee_Skip_Size=4"); HAS(c, "E_SLIDER_Marquee_Band_Size=1"); HAS(c, "E_SLIDER_Marquee_Thickness=1");
         HAS(c, "E_SLIDER_Marquee_Speed=12");)
LOR_CASE("marquee four params", "marquee", "4,1,1,12", CHECK(c.name == "Marquee"); HAS(c, "E_SLIDER_Marquee_Skip_Size=4");)

TEST_CASE("LOR marquee four parameter form takes its speed from the fourth field", "[impexp2][lor][effects][!mayfail]") {
    // SUSPECT: the source comment documents "spacing,filled space,size,speed" but the code reads parms[4] for speed, so the
    // fourth field is ignored. Needs a real marquee sample to confirm which is right.
    auto c = Convert(Settings(Layer("marquee", "4,1,1,12")));
    REQUIRE(c.ok);
    HAS(c, "E_SLIDER_Marquee_Speed=12");
}

LOR_CASE("meteors", "meteors", "rainbow,10,25,down,0,12",
         CHECK(c.name == "Meteors"); HAS(c, "E_CHOICE_Meteors_Type=Rainbow"); HAS(c, "E_SLIDER_Meteors_Count=10"); HAS(c, "E_SLIDER_Meteors_Length=25");
         HAS(c, "E_CHOICE_Meteors_Effect=Down"); HAS(c, "E_SLIDER_Meteors_Swirl_Intensity=0"); HAS(c, "E_SLIDER_Meteors_Speed=12");)
LOR_CASE("meteors upper scheme", "meteors", "PALETTE,200,200,UP,99,99", HAS(c, "Meteors_Type=Palette"); HAS(c, "Count=100"); HAS(c, "Swirl_Intensity=20"); HAS(c, "Speed=50");)
LOR_CASE("meteors ramps", "meteors", "rainbow,R1R50,R1R50,down,R0R10,R1R20", HAS(c, "E_VALUECURVE_Meteors_Count"); HAS(c, "E_VALUECURVE_Meteors_Length"); HAS(c, "E_VALUECURVE_Meteors_Swirl_Intensity"); HAS(c, "E_VALUECURVE_Meteors_Speed");)

LOR_CASE("movie scaled", "movie", "xxx.avi,True,False", HAS(c, "E_FILEPICKERCTRL_Video_Filename=xxx.avi"); HAS(c, "Video_AspectRatio=0"); HAS(c, "DurationTreatment=Normal");)
LOR_CASE("movie full length", "movie", "xxx.avi,False,True", HAS(c, "Video_AspectRatio=1"); HAS(c, "DurationTreatment=Slow/Accelerate");)

LOR_CASE("picture", "picture", "file.jpg,True,none,0,10,19,12",
         CHECK(c.name == "Pictures"); HAS(c, "E_TEXTCTRL_Pictures_Filename=file.jpg"); HAS(c, "E_CHOICE_Scaling=Scale To Fit"); HAS(c, "E_CHOICE_Pictures_Direction=none");
         HAS(c, "E_SLIDER_PicturesXC=0"); HAS(c, "E_TEXTCTRL_Pictures_Speed=4.8"); HASNOT(c, "LoopGIF");)
LOR_CASE("picture gif loops", "picture", "anim.GIF,False,none,0,10,19,12", HAS(c, "E_CHECKBOX_LoopGIF=1"); HAS(c, "E_CHOICE_Scaling=No Scaling");)
LOR_CASE("picture fit both", "picture", "a.png,fit_both,none,0,10,,12", HAS(c, "Scale To Fit"); HAS(c, "Pictures_Speed=4.8");)
LOR_CASE("picture fit width", "picture", "a.png,fit_width,none,0,10,,12", HAS(c, "Scale Keep Aspect Ratio");)
LOR_CASE("picture unknown fit", "picture", "a.png,stretchy,none,0,10,,12", HAS(c, "No Scaling");)
LOR_CASE("picture embedded", "picture", "*guitar*f526c123-50dc-48ea-8238-5c2a4855f32e*.gif,fit_both,none,0,10,,12",
         HAS(c, "E_TEXTCTRL_Pictures_Filename=guitar-f526c123.gif"); HAS(c, "LoopGIF=1");)
LOR_CASE("picture peekaboo bottom", "picture", "a.png,True,peekaboo_bottom,0,0,0,0", HAS(c, "Pictures_Direction=peekaboo,");)
LOR_CASE("picture peekaboo top", "picture", "a.png,True,peekaboo_top,0,0,0,0", HAS(c, "Pictures_Direction=peekaboo 180");)
LOR_CASE("picture peekaboo left", "picture", "a.png,True,peekaboo_left,0,0,0,0", HAS(c, "Pictures_Direction=peekaboo 90");)
LOR_CASE("picture peekaboo right", "picture", "a.png,True,peekaboo_right,0,0,0,0", HAS(c, "Pictures_Direction=peekaboo 270");)
LOR_CASE("picture scroll left", "picture", "a.png,True,scroll_left,0,0,0,0", HAS(c, "Pictures_Direction=scroll-left");)
LOR_CASE("picture x extremes", "picture", "a.png,True,none,50,0,0,0", HAS(c, "E_SLIDER_PicturesXC=100");)
LOR_CASE("picture x negative extremes", "picture", "a.png,True,none,-500,0,0,0", HAS(c, "E_SLIDER_PicturesXC=-100");)
LOR_CASE("picture percent encoded name", "picture", "my%20pic.png,True,none,0,0,0,0", HAS(c, "Pictures_Filename=my pic.png");)
LOR_CASE("picture no params", "picture", "", CHECK(c.name == "Pictures"); HAS(c, "Pictures_Filename=,");)

TEST_CASE("LOR picture name percent decoding is a single pass", "[impexp2][lor][effects][!mayfail]") {
    // BUG: PercentDecode loops on find('%') so "%2541" decodes to "%41" and is then decoded again to "A"
    auto c = Convert(Settings(Layer("picture", "a%2541.png,True,none,0,0,0,0")));
    REQUIRE(c.ok);
    HAS(c, "Pictures_Filename=a%41.png");
}

TEST_CASE("LOR picture name with invalid percent escape stays readable", "[impexp2][lor][effects][!mayfail]") {
    // BUG: PercentDecode does not check isHexChar, so "%zz" is replaced by an arbitrary control character
    auto c = Convert(Settings(Layer("picture", "a%zzb.png,True,none,0,0,0,0")));
    REQUIRE(c.ok);
    HAS(c, "Pictures_Filename=a%zzb.png");
}

TEST_CASE("LOR picture name ending in a lone percent sign does not hang", "[impexp2][lor][effects]") {
    auto c = Convert(Settings(Layer("picture", "100%,True,none,0,0,0,0")));
    REQUIRE(c.ok);
    HAS(c, "Pictures_Filename=100%");
}

LOR_CASE("picturexy full prop", "picturexy", "file.jpg,100,100,0,0,100,100,100,100,FFFF,0",
         CHECK(c.name == "Pictures"); HAS(c, "E_CHOICE_Scaling=Scale To Fit"); HAS(c, "E_CHOICE_Pictures_Direction=none");)
LOR_CASE("picturexy moves", "picturexy", "file.jpg,71,104,0,R32R0R1.00,100,100,100,100,FFFF",
         HAS(c, "E_CHOICE_Scaling=No Scaling"); HAS(c, "E_CHOICE_Pictures_Direction=vector"); HAS(c, "E_SLIDER_PicturesXC=0,E_SLIDER_PicturesEndXC=0");
         HAS(c, "E_SLIDER_PicturesYC=32"); HAS(c, "E_SLIDER_PicturesEndYC=0"); HAS(c, "E_SLIDER_Pictures_StartScale=71"); HAS(c, "E_SLIDER_Pictures_EndScale=71");
         HAS(c, "E_TEXTCTRL_Pictures_Speed=1.0");)
LOR_CASE("picturexy left ramp", "picturexy", "file.jpg,50,50,R-10R10R1.00,0,0,0,0,0,FFFF", HAS(c, "E_SLIDER_PicturesXC=-10"); HAS(c, "E_SLIDER_PicturesEndXC=10");)
LOR_CASE("picturexy scale ramp", "picturexy", "file.jpg,R10R90R1.00,50,0,0,0,0,0,0,FFFF", HAS(c, "Pictures_StartScale=10"); HAS(c, "Pictures_EndScale=90");)
LOR_CASE("picturexy full prop but moving", "picturexy", "file.jpg,100,100,5,0", HAS(c, "Scaling=No Scaling"); HAS(c, "XC=5");)
LOR_CASE("picturexy non numeric movement", "picturexy", "file.jpg,50,50,abc,xyz", HASNOT(c, "PicturesXC"); HASNOT(c, "PicturesYC");)
LOR_CASE("picturexy empty movement is full prop", "picturexy", "file.jpg,100,100,,", HAS(c, "Scale To Fit");)
LOR_CASE("picturexy gif", "picturexy", "a.gif,50,50,0,0", HAS(c, "LoopGIF=1");)
LOR_CASE("picturexy embedded", "picturexy", "*img*abcdef12-0000*.png,50,50,0,0", HAS(c, "Pictures_Filename=img-abcdef12.png");)

TEST_CASE("LOR picturexy ramp with too few fields", "[impexp2][lor][effects][.][known-crash]") {
    // BUG: Split(movementLeft,'R') then rr[1]/rr[2] with no size check; "R32" yields only two tokens -> out of range read
    auto c = Convert(Settings(Layer("picturexy", "file.jpg,50,50,R32,0")));
    CHECK(c.ok);
}

TEST_CASE("LOR picturexy lone R", "[impexp2][lor][effects][.][known-crash]") {
    // BUG: same rr[2] out of range for "R"
    auto c = Convert(Settings(Layer("picturexy", "file.jpg,R,R,R,R")));
    CHECK(c.ok);
}

LOR_CASE("spinner", "spinner", "pinwheel_1,color_per_arm,10,15,0,0,5,15,200,200,-24,0",
         CHECK(c.name == "Pinwheel"); HAS(c, "E_SLIDER_Pinwheel_Arms=10"); HAS(c, "E_SLIDER_Pinwheel_Thickness=15"); HAS(c, "E_SLIDER_Pinwheel_Twist=-360");
         HAS(c, "E_CHECKBOX_Pinwheel_Rotation=0"); HAS(c, "E_SLIDER_Pinwheel_Speed=23"); HAS(c, "E_SLIDER_Pinwheel_ArmSize=400");
         HAS(c, "E_SLIDER_PinwheelXC=-48"); HAS(c, "E_SLIDER_PinwheelYC=0"); HAS(c, "E_CHOICE_Pinwheel_Style=New Render Method");)
LOR_CASE("spinner counter clockwise", "spinner", "pinwheel_1,color_per_arm,3,15,0,0,5,-10,100,100,0,0",
         HAS(c, "E_CHECKBOX_Pinwheel_Rotation=1"); HAS(c, "E_SLIDER_Pinwheel_Speed=16");)
LOR_CASE("spinner fastest counter clockwise", "spinner", "p,c,3,15,0,0,5,-32,100,100,0,0", HAS(c, "Pinwheel_Rotation=1"); HAS(c, "E_SLIDER_Pinwheel_Speed=50");)
LOR_CASE("spinner slowest counter clockwise", "spinner", "p,c,3,15,0,0,5,-1,100,100,0,0", HAS(c, "Pinwheel_Rotation=1");)
LOR_CASE("spinner zero speed", "spinner", "p,c,3,15,0,0,5,0,100,100,0,0", HAS(c, "Pinwheel_Rotation=0"); HAS(c, "E_SLIDER_Pinwheel_Speed=0");)
LOR_CASE("spinner extreme speed", "spinner", "p,c,3,15,0,0,5,500,100,100,0,0", HAS(c, "E_SLIDER_Pinwheel_Speed=50");)
LOR_CASE("spinner extreme position", "spinner", "p,c,30,150,0,99,5,5,900,100,900,-900", HAS(c, "Pinwheel_Arms=10"); HAS(c, "PinwheelXC=100"); HAS(c, "PinwheelYC=-100"); HAS(c, "Thickness=100");)
LOR_CASE("spinner speed ramp", "spinner", "p,c,3,15,0,0,5,R-10R10,100,100,0,0", HAS(c, "E_VALUECURVE_Pinwheel_Speed=Active=TRUE");)
LOR_CASE("spinner short", "spinner", "p,c,4", HAS(c, "Pinwheel_Arms=4");)

LOR_CASE("pinwheel", "pinwheel", "3,1,6,color_per_arm,True,12,100,10,-23",
         CHECK(c.name == "Pinwheel"); HAS(c, "E_SLIDER_Pinwheel_Arms=3"); HAS(c, "E_SLIDER_Pinwheel_Thickness=0"); HAS(c, "E_SLIDER_Pinwheel_Twist=216");
         HAS(c, "E_CHECKBOX_Pinwheel_Rotation=1"); HAS(c, "E_SLIDER_Pinwheel_Speed=12"); HAS(c, "E_SLIDER_Pinwheel_ArmSize=400");
         HAS(c, "E_SLIDER_PinwheelXC=20"); HAS(c, "E_SLIDER_PinwheelYC=-4");)
LOR_CASE("pinwheel clockwise", "pinwheel", "3,10,0,color_per_arm,False,12,1,0,0", HAS(c, "Pinwheel_Rotation=0"); HAS(c, "Pinwheel_Thickness=100"); HAS(c, "Pinwheel_ArmSize=0");)

LOR_CASE("snowflakes falling", "snowflakes", "5,1,0,12,0",
         CHECK(c.name == "Snowflakes"); HAS(c, "E_SLIDER_Snowflakes_Count=5"); HAS(c, "E_SLIDER_Snowflakes_Type=1"); HAS(c, "E_SLIDER_Snowflakes_Speed=12");
         HAS(c, "E_CHOICE_Falling=Falling"); HASNOT(c, "Accumulating");)
LOR_CASE("snowflakes accumulating", "snowflakes", "5,1,0,12,60", HAS(c, "E_CHOICE_Falling=Falling & Accumulating");)
LOR_CASE("snowflakes ramps", "snowflakes", "R1R10,1,0,R0R10,0", HAS(c, "E_VALUECURVE_Snowflakes_Count"); HAS(c, "E_VALUECURVE_Snowflakes_Speed");)
LOR_CASE("snowflakes clamps", "snowflakes", "99,99,0,99,0", HAS(c, "Snowflakes_Count=20"); HAS(c, "Snowflakes_Type=5"); HAS(c, "Snowflakes_Speed=50");)

LOR_CASE("text", "text", "Hello,50,left,0,10,0,4,True",
         CHECK(c.name == "Text"); HAS(c, "E_TEXTCTRL_Text=Hello"); HAS(c, "E_CHOICE_Text_Font=Use OS Fonts"); HAS(c, "E_FONTPICKER_Text_Font='segoe ui' 50");
         HAS(c, "E_CHOICE_Text_Dir=left"); HAS(c, "E_SLIDER_Text_XStart=2"); HAS(c, "E_TEXTCTRL_Text_Speed=4");
         CHECK(Has(c.palette, "C_BUTTON_Palette1=#FFFFFF,C_CHECKBOX_Palette1=1"));)
LOR_CASE("text encoded", "text", "Hello%26nbsp%3B%20Keith,50,left,0,10,0,4,True", HAS(c, "E_TEXTCTRL_Text=Hello  Keith");)
LOR_CASE("text html colour", "text", "%3Cp%3E%3Cfont%20color%3D%22%23FF0000%22%3EHi%3C%2Ffont%3E%3C%2Fp%3E,50,left,0,10,0,4,True",
         HAS(c, "E_TEXTCTRL_Text=Hi"); CHECK(Has(c.palette, "C_BUTTON_Palette1=#FF0000"));)
LOR_CASE("text named colour", "text", "%3Cspan%20style%3D%22color%3Ared%22%3EHi%3C%2Fspan%3E,50,left,0,10,0,4",
         CHECK(Has(c.palette, "C_BUTTON_Palette1=#FF0000"));)
LOR_CASE("text unknown colour name is white", "text", "%3Cspan%20style%3D%22color%3Achartreuse%22%3EHi%3C%2Fspan%3E,50,left,0,10,0,4",
         CHECK(Has(c.palette, "C_BUTTON_Palette1=#FFFFFF"));)
LOR_CASE("text two lines", "text", "%3Cp%3Eone%3C%2Fp%3E%3Cp%3Etwo%3C%2Fp%3E,50,left,0,10,0,4", HAS(c, "E_TEXTCTRL_Text=one\\ntwo");)
LOR_CASE("text with comma", "text", "a%2Cb,50,left,0,10,0,4", HAS(c, "E_TEXTCTRL_Text=a&comma;b");)
LOR_CASE("text br tag", "text", "a%3Cbr%2F%3Eb,50,left,0,10,0,4", HAS(c, "E_TEXTCTRL_Text=a\\nb");)
LOR_CASE("text unterminated tag", "text", "abc%3Cfont,50,left,0,10,0,4", HAS(c, "E_TEXTCTRL_Text=abc");)
LOR_CASE("text entities", "text", "a%26lt%3Bb%26gt%3Bc%26amp%3Bd,50,left,0,10,0,4", HAS(c, "E_TEXTCTRL_Text=a<b>c&d");)
LOR_CASE("text peekaboo bottom", "text", "x,50,peekaboo_bottom,0,10,0,4", HAS(c, "E_CHECKBOX_TextToCenter=1"); HAS(c, "E_CHOICE_Text_Dir=up");)
LOR_CASE("text peekaboo top", "text", "x,50,peekaboo_top,0,10,0,4", HAS(c, "E_CHOICE_Text_Dir=down");)
LOR_CASE("text peekaboo left", "text", "x,50,peekaboo_left,0,10,0,4", HAS(c, "E_CHOICE_Text_Dir=left");)
LOR_CASE("text peekaboo right", "text", "x,50,peekaboo_right,0,10,0,4", HAS(c, "E_CHOICE_Text_Dir=right");)
LOR_CASE("text empty", "text", "", CHECK(c.name == "Text"); HAS(c, "E_TEXTCTRL_Text=,");)
LOR_CASE("text lone percent", "text", "100%,50,left,0,10,0,4", HAS(c, "E_TEXTCTRL_Text=100%");)
LOR_CASE("text escape at end", "text", "abc%4,50,left,0,10,0,4", HAS(c, "E_TEXTCTRL_Text=abc%4");)
LOR_CASE("text hex colour with six digits only", "text", "%3Cfont%20color%3D%22%23FF00FF%22%3Ea%3C%2Ffont%3E,50,left,0,10,0,4", CHECK(Has(c.palette, "#FF00FF"));)
LOR_CASE("text font size clamp", "text", "x,999,left,999,10,0,999", HAS(c, "' 149"); HAS(c, "XStart=200"); HAS(c, "Text_Speed=50");)

TEST_CASE("LOR text keeps sparkle and brightness while replacing the palette", "[impexp2][lor][effects]") {
    auto c = Convert(Settings(Layer("text", "x,50,left,0,10,0,4"), kNone, "Mix_Average", "0", "20"), 0, 100, " intensity=\"50\"");
    REQUIRE(c.ok);
    CHECK(Has(c.palette, "C_SLIDER_SparkleFrequency=20"));
    CHECK(Has(c.palette, "C_SLIDER_Brightness=50"));
    CHECK_FALSE(Has(c.palette, "Palette2"));
}

LOR_CASE("twinkle", "twinkle", "50,25,twinkle,random",
         CHECK(c.name == "Twinkle"); HAS(c, "E_SLIDER_Twinkle_Count=25"); HAS(c, "E_SLIDER_Twinkle_Steps=50"); HAS(c, "E_CHECKBOX_Twinkle_ReRandom=1");
         HAS(c, "E_CHECKBOX_Twinkle_Strobe=0");)
LOR_CASE("twinkle pulse interval", "twinkle", "50,25,pulse,interval", HAS(c, "Twinkle_ReRandom=0"); HAS(c, "Twinkle_Strobe=1");)
LOR_CASE("twinkle unknown layout", "twinkle", "500,500,flash,other", HASNOT(c, "ReRandom"); HAS(c, "Twinkle_Strobe=1"); HAS(c, "Twinkle_Count=100");)

LOR_CASE("blendedbars", "blendedbars", "right,10,36,0",
         CHECK(c.name == "Bars"); HAS(c, "E_CHOICE_Bars_Direction=Right"); HAS(c, "E_SLIDER_Bars_BarCount=5"); HAS(c, "E_TEXTCTRL_Bars_Cycles=1.80");
         HAS(c, "E_CHECKBOX_Bars_Gradient=1");)
LOR_CASE("blendedbars min count", "blendedbars", "down_left,0,0,0", HAS(c, "Bars_BarCount=1"); HAS(c, "Bars_Angle=-135"); HAS(c, "Bars_Cycles=0.00");)
LOR_CASE("blendedbars negative count", "blendedbars", "up,-8,0,0", HAS(c, "Bars_BarCount=1");)

LOR_CASE("singleblock down", "singleblock", "down,0,0,50,50,100,50,single_color",
         CHECK(c.name == "Morph"); HAS(c, "E_SLIDER_Morph_Start_X1=0,E_SLIDER_Morph_Start_Y1=100,E_SLIDER_Morph_Start_X2=100,E_SLIDER_Morph_Start_Y2=100");
         HAS(c, "E_SLIDER_Morph_End_X1=0,E_SLIDER_Morph_End_Y1=0,E_SLIDER_Morph_End_X2=100,E_SLIDER_Morph_End_Y2=0");
         HAS(c, "E_SLIDER_MorphStartLength=1,E_SLIDER_MorphEndLength=1"); HAS(c, "E_SLIDER_MorphDuration=67");)
LOR_CASE("singleblock up", "singleblock", "up,10,20,0,50,100,50,single_color",
         HAS(c, "Morph_Start_X1=0,E_SLIDER_Morph_Start_Y1=0"); HAS(c, "Morph_End_X1=0,E_SLIDER_Morph_End_Y1=100"); HAS(c, "MorphStartLength=30"); HAS(c, "MorphDuration=100");)
LOR_CASE("singleblock left", "singleblock", "left,0,0,100,50,50,25,single_color", HAS(c, "Morph_Start_X1=100,E_SLIDER_Morph_Start_Y1=0,E_SLIDER_Morph_Start_X2=100,E_SLIDER_Morph_Start_Y2=50");
         HAS(c, "Morph_End_X1=0"); HAS(c, "MorphDuration=50");)
LOR_CASE("singleblock right", "singleblock", "right,0,0,0,50,50,75,single_color", HAS(c, "Morph_Start_X1=0,E_SLIDER_Morph_Start_Y1=50,E_SLIDER_Morph_Start_X2=0,E_SLIDER_Morph_Start_Y2=100");
         HAS(c, "Morph_End_X1=100");)
LOR_CASE("singleblock empty size and offset", "singleblock", "down,0,0,0,,,,single_color", HAS(c, "Morph_Start_X1=0,E_SLIDER_Morph_Start_Y1=100,E_SLIDER_Morph_Start_X2=100");)
LOR_CASE("singleblock huge head", "singleblock", "down,500,500,500,0,500,500,x", HAS(c, "MorphStartLength=100"); HAS(c, "MorphDuration=17");)
LOR_CASE("singleblock negative values", "singleblock", "down,-5,-5,-50,0,-10,-10,x", HAS(c, "MorphStartLength=1"); HAS(c, "Morph_Start_X1=0,E_SLIDER_Morph_Start_Y1=100,E_SLIDER_Morph_Start_X2=0");)
LOR_CASE("singleblock tail gives zero duration", "singleblock", "down,0,0,-100000,50,100,50,x", HAS(c, "MorphDuration=1,");)
LOR_CASE("singleblock unknown direction acts as down", "singleblock", "sideways,0,0,0,50,100,50,x", HAS(c, "Morph_Start_Y1=100");)

LOR_CASE("ripple", "ripple", "circle,21,47,46,50,0,0,37,False,50",
         CHECK(c.name == "Ripple"); HAS(c, "E_CHOICE_Ripple_Object_To_Draw=Circle"); HAS(c, "E_CHOICE_Ripple_Movement=Explode"); HAS(c, "E_SLIDER_Ripple_Thickness=47");
         HAS(c, "E_TEXTCTRL_Ripple_Cycles=");)
LOR_CASE("ripple implode", "ripple", "square,1,5,1,1,-50,50,0,True,0", HAS(c, "Ripple_Object_To_Draw=Square"); HAS(c, "Ripple_Movement=Implode"); HAS(c, "Ripple_Cycles=1");)
LOR_CASE("ripple short params", "ripple", "circle", HAS(c, "Ripple_Thickness=0"); HAS(c, "Ripple_Movement=Explode");)
LOR_CASE("ripple two word object", "ripple", "star shape,1,1,1,1,0,0,0,False,0", HAS(c, "Ripple_Object_To_Draw=Star Shape");)

LOR_CASE("wave", "wave", "left,along_wave_scrolling,rainbow,triple,50,23,88,A3A50A1.00,A-14A50A1.00,sine",
         CHECK(c.name == "Wave"); HAS(c, "E_CHOICE_Wave_Direction=Right to Left"); HAS(c, "E_CHOICE_Fill_Colors=Rainbow"); HAS(c, "E_TEXTCTRL_Number_Waves=3.00");
         HAS(c, "E_CHOICE_Wave_Type=Sine");)
LOR_CASE("wave other", "wave", "up,across_wave_scrolling,palette,double,25,10,50,12,0,Triangle,repeat_at_speed,0",
         HAS(c, "Wave_Direction=Left to Right"); HAS(c, "Fill_Colors=Palette"); HAS(c, "Number_Waves=2.00"); HAS(c, "Wave_Type=Triangle");)
LOR_CASE("wave none single", "wave", "up,a,none,single,0,0,0,0,0,SQUARE", HAS(c, "Fill_Colors=None"); HAS(c, "Number_Waves=1.00"); HAS(c, "Wave_Type=Square");)
LOR_CASE("wave unknown", "wave", "up,a,b,quad,0,0,0,0,0,saw", HASNOT(c, "Number_Waves"); HASNOT(c, "Wave_Type");)

LOR_CASE("plasma", "plasma", "1,3,12,blended",
         CHECK(c.name == "Plasma"); HAS(c, "E_SLIDER_Plasma_Style=1"); HAS(c, "E_SLIDER_Plasma_Line_Density=3"); HAS(c, "E_SLIDER_Plasma_Speed=10");)
LOR_CASE("plasma clamps high", "plasma", "99,99,9999,x", HAS(c, "Plasma_Style=10"); HAS(c, "Line_Density=10"); HAS(c, "Plasma_Speed=100");)
LOR_CASE("plasma clamps low", "plasma", "0,-4,-12,x", HAS(c, "Plasma_Style=1"); HAS(c, "Line_Density=1"); HAS(c, "Plasma_Speed=0");)
LOR_CASE("plasma huge speed does not overflow", "plasma", "1,1,2147483647,x", HAS(c, "Plasma_Speed=");)

TEST_CASE("LOR plasma five part form keeps the extra parts out of the settings", "[impexp2][lor][effects]") {
    auto c = Convert(Settings("lightorama_plasma:FFFF0000,1:1,3,12,blended:extra1:extra2"));
    REQUIRE(c.ok);
    CHECK(c.eff.effectSettings.size() == 4);
    HAS(c, "E_SLIDER_Plasma_Style=1");
}

LOR_CASE("starfield", "starfield", "tree,palette,solid,50,50,50,1,3,0,25,random",
         CHECK(c.name == "Shape"); HAS(c, "E_CHOICE_Shape_ObjectToDraw=Tree"); HAS(c, "E_SLIDER_Shape_Count=10"); HAS(c, "E_SLIDER_Shape_Growth=25");
         HAS(c, "E_SLIDER_Shape_Lifetime=35"); HAS(c, "E_CHECKBOX_Shape_RandomLocation=1");)
LOR_CASE("starfield unknown shape", "starfield", "blob,palette,solid,0,0,0,1,3,0,25,random", HAS(c, "Shape_ObjectToDraw=Star"); HAS(c, "Shape_Count=1,"); HAS(c, "Shape_Growth=1,"); HAS(c, "Shape_Lifetime=60");)
LOR_CASE("starfield known shapes", "starfield", "HEART,palette,solid,5000,5000,5000", HAS(c, "Shape_ObjectToDraw=Heart"); HAS(c, "Shape_Count=100"); HAS(c, "Shape_Growth=100"); HAS(c, "Shape_Lifetime=5");)
LOR_CASE("starfield snowflake", "starfield", "snowflake", HAS(c, "Shape_ObjectToDraw=Snowflake");)
LOR_CASE("starfield circle", "starfield", "circle", HAS(c, "Shape_ObjectToDraw=Circle");)
LOR_CASE("starfield empty", "starfield", "", HAS(c, "Shape_ObjectToDraw=Star");)

LOR_CASE("movingshapes", "movingshapes", "star5,8,73,10,palette,solid,random_wrap,90,continuous_rotation,20,12,False,0,0,1",
         CHECK(c.name == "Shape"); HAS(c, "E_CHOICE_Shape_ObjectToDraw=Star"); HAS(c, "E_SLIDER_Shape_Points=5"); HAS(c, "E_SLIDER_Shape_Count=8");
         HAS(c, "E_SLIDER_Shape_StartSize=73"); HAS(c, "E_SLIDER_Shape_Thickness=50"); HAS(c, "E_SLIDER_Shapes_Velocity=10");
         HAS(c, "E_SLIDER_Shapes_Direction=90"); HAS(c, "E_CHECKBOX_Shapes_RandomMovement=1");)
LOR_CASE("movingshapes star points capped", "movingshapes", "star12,1,1,1,x,hollow,bounce,400", HAS(c, "Shape_Points=9"); HAS(c, "Shape_Thickness=3"); HASNOT(c, "RandomMovement"); HAS(c, "Shapes_Direction=359");)
LOR_CASE("movingshapes star bare", "movingshapes", "star,1,1,1,x,solid,x,0", HAS(c, "Shape_ObjectToDraw=Star"); HAS(c, "Shape_Points=5");)
LOR_CASE("movingshapes star one point", "movingshapes", "star1,1,1,1,x,solid,x,0", HAS(c, "Shape_Points=5");)
LOR_CASE("movingshapes heart", "movingshapes", "HEART,0,0,-5,x,solid,x,-9", HAS(c, "Shape_ObjectToDraw=Heart"); HAS(c, "Shape_Count=1"); HAS(c, "Shape_StartSize=1"); HAS(c, "Shapes_Velocity=0"); HAS(c, "Shapes_Direction=0");)
LOR_CASE("movingshapes tree", "movingshapes", "tree,1,1,99,x,solid,x,0", HAS(c, "Shape_ObjectToDraw=Tree"); HAS(c, "Shapes_Velocity=20");)
LOR_CASE("movingshapes snowflake", "movingshapes", "snowflake,1,1,1,x,solid,x,0", HAS(c, "Shape_ObjectToDraw=Snowflake");)
LOR_CASE("movingshapes square", "movingshapes", "square,1,1,1,x,solid,x,0", HAS(c, "Shape_ObjectToDraw=Square");)
LOR_CASE("movingshapes triangle", "movingshapes", "triangle,1,1,1,x,solid,x,0", HAS(c, "Shape_ObjectToDraw=Triangle");)
LOR_CASE("movingshapes unknown is circle", "movingshapes", "blob,1,1,1,x,solid,x,0", HAS(c, "Shape_ObjectToDraw=Circle");)
LOR_CASE("movingshapes star huge point count", "movingshapes", "star99999999999999,1,1,1,x,solid,x,0", HAS(c, "Shape_Points=9");)
LOR_CASE("movingshapes star negative", "movingshapes", "star-3,1,1,1,x,solid,x,0", HAS(c, "Shape_Points=5");)

LOR_CASE("simpleshape", "simpleshape", "circle,O100O174O1.00O2.00O0.00,O100O174",
         CHECK(c.name == "Shape"); HAS(c, "E_CHOICE_Shape_ObjectToDraw=Circle"); HAS(c, "E_SLIDER_Shape_Count=1"); HAS(c, "E_CHECKBOX_Shape_RandomLocation=0");)
LOR_CASE("simpleshape empty", "simpleshape", "", HAS(c, "Shape_ObjectToDraw=Circle");)
LOR_CASE("simpleshape square", "simpleshape", "SQUARE", HAS(c, "Shape_ObjectToDraw=Square");)

LOR_CASE("spinfade", "spinfade", "arc,4,50,0,12,100,100,0,0,False",
         CHECK(c.name == "Pinwheel"); HAS(c, "E_SLIDER_Pinwheel_Arms=4"); HAS(c, "E_SLIDER_Pinwheel_Thickness=50"); HAS(c, "E_SLIDER_Pinwheel_Speed=10");
         HAS(c, "E_CHOICE_Pinwheel_3D=Sweep");)
LOR_CASE("spinfade clamps", "spinfade", "arc,99,500,0,9999", HAS(c, "Pinwheel_Arms=20"); HAS(c, "Pinwheel_Thickness=100"); HAS(c, "Pinwheel_Speed=50");)
LOR_CASE("spinfade negatives", "spinfade", "arc,-1,-5,0,-12", HAS(c, "Pinwheel_Arms=1"); HAS(c, "Pinwheel_Thickness=0"); HAS(c, "Pinwheel_Speed=0");)

LOR_CASE("unknown effect type", "hologram", "1,2,3", CHECK(c.name == "Hologram"); CHECK(c.settings == ",T_CHOICE_LayerMethod=Normal");)
LOR_CASE("lightorama prefix inside the effect type", "lightorama_fire", "50,0", CHECK(c.eff.effectType == "lightorama_fire");)

TEST_CASE("LOR effect type with the lightorama prefix inside effectType converts as the base type", "[impexp2][lor][effects]") {
    LOREditEffect e;
    e.type = loreditType::TRACKS;
    e.left = false;
    e.startMS = 0;
    e.endMS = 1000;
    e.effectType = "lightorama_fire";
    e.effectSettings = { "FFFF0000,1", "50,0" };
    e.otherSettings = { "Mix_Average", "0", "0", "full", "10", "" };
    std::string pal;
    auto s = e.GetSettings(pal);
    CHECK(Has(s, "E_SLIDER_Fire_Height=44"));
}

// ---------- Channel style effect types ----------

TEST_CASE("LOR channel style effect settings", "[impexp2][lor][effects]") {
    LOREditEffect e;
    e.startIntensity = 10;
    e.endIntensity = 80;
    std::string pal;
    SECTION("intensity") {
        e.effectType = "INTENSITY";
        CHECK(e.GetSettings(pal) == "E_TEXTCTRL_Eff_On_End=80,E_TEXTCTRL_Eff_On_Start=10");
    }
    SECTION("shimmer") {
        e.effectType = "SHIMMER";
        CHECK(e.GetSettings(pal) == "E_CHECKBOX_On_Shimmer=1,E_TEXTCTRL_Eff_On_End=80,E_TEXTCTRL_Eff_On_Start=10");
    }
    SECTION("dmx") {
        e.effectType = "DMX_INTENSITY";
        CHECK(e.GetSettings(pal) == "E_VALUECURVE_DMX1=Active=TRUE|Id=ID_VALUECURVE_DMX1|Type=Ramp|Min=0.00|Max=255.00|P1=10|P2=80|RV=TRUE|");
    }
    SECTION("twinkle") {
        e.effectType = "TWINKLE";
        CHECK(e.GetSettings(pal) == "E_SLIDER_Twinkle_Count=48");
    }
    SECTION("empty settings") {
        e.effectType = "colorwash";
        CHECK(e.GetSettings(pal).empty());
    }
}

// ---------- Blend / layer settings ----------

TEST_CASE("LOR mix blend names", "[impexp2][lor][blend]") {
    struct Row { const char* mix; const char* blend; };
    auto r = GENERATE(Row{ "Mix_Average", "Average" }, Row{ "Mix_Overlay", "Layered" }, Row{ "Mix_Maximum", "Max" }, Row{ "Mix_Bottom_Top", "Bottom-Top" },
                      Row{ "Mix_Left-Right", "Left-Right" }, Row{ "Mix_Rt_Hides_Lt", "1 is Mask" }, Row{ "Mix_Rt_Reveals_Lt", "2 reveals 1" },
                      Row{ "Mix_Bogus", "Normal" }, Row{ "", "Normal" });
    auto c = Convert(Settings(Layer("colorwash", "full,full,none"), Layer("fire", "50,0"), r.mix));
    REQUIRE(c.ok);
    REQUIRE(c.eff.left);
    CHECK(c.eff.otherSidePresent);
    CHECK(c.eff.GetBlend() == r.blend);
    CHECK(c.eff.GetLayerSettings() == std::string(",T_CHOICE_LayerMethod=") + r.blend);
}

TEST_CASE("LOR right side effect always blends normally", "[impexp2][lor][blend]") {
    auto doc = Parse(OneEffectDoc(Settings(Layer("colorwash", "full,full,none"), Layer("fire", "50,0"), "Mix_Maximum")));
    LOREdit l(doc, 20);
    REQUIRE(l.GetModelLayers("P") == 2);
    auto left = l.GetTrackEffects("P", 0, 0);
    auto right = l.GetTrackEffects("P", 1, 0);
    REQUIRE(left.size() == 1);
    REQUIRE(right.size() == 1);
    CHECK(left[0].left);
    CHECK_FALSE(right[0].left);
    CHECK(left[0].effectType == "colorwash");
    CHECK(right[0].effectType == "fire");
    CHECK(right[0].GetBlend() == "Normal");
    CHECK(right[0].GetLayerSettings() == ",T_CHOICE_LayerMethod=Normal");
    CHECK(left[0].GetBlend() == "Max");
}

TEST_CASE("LOR a lone side ignores the mix", "[impexp2][lor][blend]") {
    auto c = Convert(Settings(Layer("colorwash", "full,full,none"), kNone, "Mix_Maximum"));
    REQUIRE(c.ok);
    CHECK_FALSE(c.eff.otherSidePresent);
    CHECK(c.eff.GetLayerSettings() == ",T_CHOICE_LayerMethod=Normal");
    CHECK(c.eff.GetBlend() == "Max");
}

TEST_CASE("LOR dissolve mix becomes a transition", "[impexp2][lor][blend]") {
    SECTION("forward") {
        auto c = Convert(Settings(Layer("colorwash", "full,full,none"), Layer("fire", "50,0"), "Dissolve_Pixel", "R0R100"), 0, 250);
        REQUIRE(c.ok);
        CHECK(c.eff.GetLayerSettings() == ",T_CHOICE_LayerMethod=Normal,T_CHOICE_Out_Transition_Type=Dissolve,T_TEXTCTRL_Fadeout=2.50");
    }
    SECTION("reverse") {
        auto c = Convert(Settings(Layer("colorwash", "full,full,none"), Layer("fire", "50,0"), "Dissolve_Pixel", "R100R0"), 0, 250);
        REQUIRE(c.ok);
        CHECK(c.eff.GetLayerSettings() == ",T_CHOICE_LayerMethod=Normal,T_CHOICE_In_Transition_Type=Dissolve,T_TEXTCTRL_Fadein=2.50");
    }
    SECTION("ramp missing") {
        auto c = Convert(Settings(Layer("colorwash", "full,full,none"), Layer("fire", "50,0"), "Dissolve_Pixel", "5"), 0, 100);
        REQUIRE(c.ok);
        CHECK(Has(c.eff.GetLayerSettings(), "Out_Transition_Type=Dissolve"));
    }
    SECTION("dissolve with a lone side is normal") {
        auto c = Convert(Settings(Layer("colorwash", "full,full,none"), kNone, "Dissolve_Pixel", "R0R100"));
        REQUIRE(c.ok);
        CHECK(c.eff.GetLayerSettings() == ",T_CHOICE_LayerMethod=Normal");
    }
}

TEST_CASE("LOR dissolve of a backwards effect does not produce a huge fade", "[impexp2][lor][blend][!mayfail]") {
    // BUG: (endMS - startMS) is unsigned so an effect whose end precedes its start gives a ~4.29 million second fade
    LOREditEffect e;
    e.left = true;
    e.otherSidePresent = true;
    e.startMS = 5000;
    e.endMS = 1000;
    e.otherSettings = { "Dissolve_Pixel", "R0R100", "0", "full", "10", "" };
    auto s = e.GetLayerSettings();
    CHECK(s.find("429496") == std::string::npos);
}

TEST_CASE("LOR layer settings with too few other settings", "[impexp2][lor][blend]") {
    LOREditEffect e;
    e.left = true;
    e.otherSidePresent = true;
    e.otherSettings = { "Mix_Maximum" };
    CHECK(e.GetLayerSettings() == ",T_CHOICE_LayerMethod=Normal");
    e.otherSettings.clear();
    CHECK(e.GetBlend() == "Normal");
}

// ---------- Sub buffer ----------

TEST_CASE("LOR sub buffer from the motion row", "[impexp2][lor][subbuffer]") {
    LOREditEffect e;
    SECTION("none") {
        CHECK(e.GetSubBuffer().empty());
    }
    SECTION("custom") {
        e.trackType = "custom";
        CHECK(e.GetSubBuffer().empty());
    }
    SECTION("single line") {
        e.trackType = "custom_horizontal_buffer";
        CHECK(e.GetSubBuffer() == ",B_CHOICE_BufferStyle=Single Line");
    }
    SECTION("full rectangle is no sub buffer") {
        e.trackType = "rectangle";
        CHECK(e.GetSubBuffer().empty());
    }
    SECTION("centre") {
        e.trackType = "rectangle";
        e.subx = 0.25f; e.suby = 0.25f; e.subw = 0.5f; e.subh = 0.5f;
        CHECK(e.GetSubBuffer() == ",B_CUSTOM_SubBuffer=25.00x25.00x75.00x75.00");
    }
    SECTION("y is measured from the top") {
        e.trackType = "rectangle";
        e.subx = 0.0f; e.suby = 0.0f; e.subw = 1.0f; e.subh = 0.5f;
        CHECK(e.GetSubBuffer() == ",B_CUSTOM_SubBuffer=0.00x50.00x100.00x100.00");
    }
    SECTION("bottom half") {
        e.trackType = "rectangle";
        e.suby = 0.5f; e.subh = 0.5f;
        CHECK(e.GetSubBuffer() == ",B_CUSTOM_SubBuffer=0.00x0.00x100.00x50.00");
    }
    SECTION("overflow clamps to full") {
        e.trackType = "rectangle";
        e.subx = -2.0f; e.suby = -2.0f; e.subw = 9.0f; e.subh = 9.0f;
        CHECK(e.GetSubBuffer().empty());
    }
    SECTION("negative sizes clamp") {
        e.trackType = "rectangle";
        e.subx = 0.5f; e.subw = -0.2f;
        auto s = e.GetSubBuffer();
        CHECK(Has(s, "B_CUSTOM_SubBuffer="));
    }
    SECTION("zero sized") {
        e.trackType = "rectangle";
        e.subw = 0.0f; e.subh = 0.0f;
        CHECK(Has(e.GetSubBuffer(), "B_CUSTOM_SubBuffer=0.00x100.00x0.00x100.00"));
    }
    SECTION("unknown type") {
        e.trackType = "polygon";
        CHECK(e.GetSubBuffer().empty());
    }
}

TEST_CASE("LOR sub buffer is added to converted settings", "[impexp2][lor][subbuffer]") {
    auto doc = Parse(OneEffectDoc(Settings(Layer("fire", "50,0")), 0, 100, " intensity=\"100\"",
                                  " name=\"T\" type=\"rectangle\" subx=\"0.1\" suby=\"0.2\" subw=\"0.5\" subh=\"0.25\""));
    LOREdit l(doc, 20);
    auto v = l.GetTrackEffects("P", 0, 0);
    REQUIRE(v.size() == 1);
    std::string pal = v[0].GetPalette();
    auto s = v[0].GetSettings(pal);
    CHECK(Has(s, ",B_CUSTOM_SubBuffer=10.00x55.00x60.00x80.00"));
    CHECK(v[0].trackType == "rectangle");
}

TEST_CASE("LOR picturexy full-prop shortcut still gets layer and sub buffer settings", "[impexp2][lor][subbuffer]") {
    auto doc = Parse(OneEffectDoc(Settings(Layer("picturexy", "f.png,100,100,0,0")), 0, 100, " intensity=\"100\"",
                                  " name=\"T\" type=\"custom_horizontal_buffer\""));
    LOREdit l(doc, 20);
    auto v = l.GetTrackEffects("P/T", 0, 0);
    REQUIRE(v.size() == 1);
    std::string pal = v[0].GetPalette();
    auto s = v[0].GetSettings(pal);
    CHECK(Has(s, "Scale To Fit"));
    CHECK(Has(s, "T_CHOICE_LayerMethod=Normal"));
    CHECK(Has(s, "B_CHOICE_BufferStyle=Single Line"));
}

// ---------- Static helpers ----------

TEST_CASE("LOR Rescale clamps and maps", "[impexp2][lor][rescale]") {
    struct Row { float v, smin, smax, tmin, tmax, expect; };
    auto r = GENERATE(Row{ 5, 0, 10, 0, 100, 50 }, Row{ -5, 0, 10, 0, 100, 0 }, Row{ 50, 0, 10, 0, 100, 100 }, Row{ 0, -50, 50, -100, 100, 0 },
                      Row{ 5, 0, 10, 100, 0, 50 }, Row{ 10, 0, 10, 0, -50, -50 }, Row{ 3, 3, 3.5f, 7, 9, 7 });
    CHECK(LOREditEffect::Rescale(r.v, r.smin, r.smax, r.tmin, r.tmax) == Approx(r.expect));
}

TEST_CASE("LOR Rescale with zero width source does not crash", "[impexp2][lor][rescale]") {
    float v = LOREditEffect::Rescale(5, 5, 5, 0, 10);
    (void)v;
    SUCCEED();
}

TEST_CASE("LOR RescaleWithRangeI and F", "[impexp2][lor][rescale]") {
    std::string vc;
    SECTION("int plain") {
        CHECK(LOREditEffect::RescaleWithRangeI("5", "E_VALUECURVE_X", 0, 10, 0, 100, vc, 0, 100) == "50");
        CHECK(vc.empty());
    }
    SECTION("int ramp") {
        CHECK(LOREditEffect::RescaleWithRangeI("R0R10", "E_VALUECURVE_X", 0, 10, 0, 100, vc, 1, 99) == "0");
        CHECK(Has(vc, ",E_VALUECURVE_X=Active=TRUE|Id=ID_VALUECURVE_X|Type=Ramp|Min=1.00|Max=99.00|P1="));
    }
    SECTION("int ramp with decimals after") {
        LOREditEffect::RescaleWithRangeI("R5R10R1.00", "E_VALUECURVE_X", 0, 10, 0, 100, vc, 0, 100);
        CHECK(Has(vc, "Type=Ramp"));
    }
    SECTION("int garbage") {
        CHECK(LOREditEffect::RescaleWithRangeI("abc", "E_VALUECURVE_X", 0, 10, 0, 100, vc, 0, 100) == "0");
    }
    SECTION("int empty") {
        CHECK(LOREditEffect::RescaleWithRangeI("", "E_VALUECURVE_X", 1, 10, 1, 10, vc, 0, 100) == "1");
    }
    SECTION("float plain") {
        CHECK(LOREditEffect::RescaleWithRangeF("2.5", "E_VALUECURVE_X", 0, 5, 0, 10, vc, 0, 10) == "5.0");
        CHECK(vc.empty());
    }
    SECTION("float ramp") {
        CHECK(LOREditEffect::RescaleWithRangeF("R0R5", "E_VALUECURVE_Y", 0, 5, 0, 10, vc, 0, 10) == "0.0");
        CHECK(Has(vc, "Type=Ramp"));
        CHECK(Has(vc, "ID_VALUECURVE_Y"));
    }
}

TEST_CASE("LOR ramp values are taken from the numbers after each R", "[impexp2][lor][rescale][!mayfail]") {
    // BUG: "R0R10" splits on 'R' into {"", "0", "10"} but RescaleWithRange* read rr[0] (empty) and rr[1] (the start), so the
    // ramp becomes start=0 .. end=<start> and the real end value is dropped. (picturexy correctly uses rr[1] and rr[2].)
    std::string vc;
    auto r = LOREditEffect::RescaleWithRangeI("R2R8", "E_VALUECURVE_X", 0, 10, 0, 100, vc, 0, 100);
    CHECK(r == "20");
    CHECK(Has(vc, "P1=20.00|P2=80.00"));
}

TEST_CASE("LOR butterfly hue of zero adds no hue adjust", "[impexp2][lor][effects][!mayfail]") {
    // BUG: hue 0 is rescaled 0..359 -> -100..100 giving -100 so the `hue != "0"` guard never skips it; only ~180 would
    auto c = Convert(Settings(Layer("butterfly", "linear,5,3,forward,0,10,rainbow")));
    REQUIRE(c.ok);
    CHECK_FALSE(Has(c.palette, "HueAdjust"));
}

TEST_CASE("LOR butterfly ramp chunks carry both ends", "[impexp2][lor][effects][!mayfail]") {
    // BUG: see the RescaleWithRange ramp parsing above
    auto c = Convert(Settings(Layer("butterfly", "linear,R2R8,3,forward,0,10,rainbow")));
    REQUIRE(c.ok);
    HAS(c, "P1=2.00|P2=8.00");
}

TEST_CASE("LOR parameter helpers", "[impexp2][lor][helpers]") {
    std::vector<std::string> v{ "a", "12", "True", "true", "-7", "x9" };
    CHECK(LOREditEffect::SafeGetStringParm(v, 0) == "a");
    CHECK(LOREditEffect::SafeGetStringParm(v, 6) == "");
    CHECK(LOREditEffect::SafeGetStringParm(v, 100) == "");
    CHECK(LOREditEffect::SafeGetIntParm(v, 1) == 12);
    CHECK(LOREditEffect::SafeGetIntParm(v, 4) == -7);
    CHECK(LOREditEffect::SafeGetIntParm(v, 5) == 0);
    CHECK(LOREditEffect::SafeGetIntParm(v, 6) == 0);
    CHECK(LOREditEffect::SafeGetBoolParm(v, 2));
    CHECK_FALSE(LOREditEffect::SafeGetBoolParm(v, 3));
    CHECK_FALSE(LOREditEffect::SafeGetBoolParm(v, 7));
    CHECK(LOREditEffect::SafeGetStringParm({}, 0) == "");
}

TEST_CASE("LOR parameter helpers with a negative index", "[impexp2][lor][helpers][.][known-crash]") {
    // BUG: SafeGet*Parm compare `param < size` only, so a negative index reads arr[-1]
    std::vector<std::string> v{ "a" };
    CHECK(LOREditEffect::SafeGetStringParm(v, -1) == "");
}

TEST_CASE("LOR ProperCase", "[impexp2][lor][helpers]") {
    CHECK(LOREditEffect::ProperCase("circle") == "Circle");
    CHECK(LOREditEffect::ProperCase("star shape") == "Star Shape");
    CHECK(LOREditEffect::ProperCase("") == "");
    CHECK(LOREditEffect::ProperCase("a\tb") == "A\tB");
    CHECK(LOREditEffect::ProperCase("ABC") == "ABC");
    CHECK(LOREditEffect::ProperCase("  x") == "  X");
}

TEST_CASE("LOR GetLayers reports which sides hold an effect", "[impexp2][lor][helpers]") {
    struct Row { const char* s; int l, r; };
    auto row = GENERATE(Row{ "Mix_Average|0|0|full|10|lightorama_fire:x:y|lightorama_none::", 1, 0 },
                        Row{ "Mix_Average|0|0|full|10|lightorama_none::|lightorama_fire:x:y", 0, 1 },
                        Row{ "Mix_Average|0|0|full|10|lightorama_fire:x:y|lightorama_bars:x:y", 1, 1 },
                        Row{ "Mix_Average|0|0|full|10|lightorama_none::|lightorama_none::", 0, 0 },
                        Row{ "Mix_Average|0|0|full|10|fire|bars", 0, 0 },
                        Row{ "Mix_Average|0|0|full|10|lightorama_fire", 0, 0 },
                        Row{ "INTENSITY", 0, 0 },
                        Row{ "", 0, 0 },
                        Row{ "a|b|c|d|e|lightorama_x|lightorama_y|extra|more", 1, 1 },
                        Row{ "a|b|c|d|e|lightorama_none|lightorama_y", 0, 1 });
    int l = 9, r = 9;
    LOREdit::GetLayers(row.s, l, r);
    CHECK(l == row.l);
    CHECK(r == row.r);
}

TEST_CASE("LOR GetColor needs exactly six comma fields", "[impexp2][lor][helpers]") {
    CHECK(LOREdit::GetColor("a,b,c,d,e,Red") == "Red");
    CHECK(LOREdit::GetColor("a,b,c,d,e") == "");
    CHECK(LOREdit::GetColor("a,b,c,d,e,f,g") == "");
    CHECK(LOREdit::GetColor("") == "");
    CHECK(LOREdit::GetColor(",,,,,") == "");
}

TEST_CASE("LOR node strand mapping detection", "[impexp2][lor][helpers]") {
    struct Row { const char* s; bool expect; };
    auto r = GENERATE(Row{ "Tree[1,2,3][Red]", true }, Row{ "Tree[0,0,0][]", true }, Row{ "Tree", false }, Row{ "Tree[1,2]", false }, Row{ "Tree[a,b,c]", false },
                      Row{ "", false }, Row{ "[10,20,30]", true }, Row{ "[1,2,3", false }, Row{ "[-1,2,3]", false });
    CHECK(LOREdit::IsNodeStrandMapping(r.s) == r.expect);
}

TEST_CASE("LOR setNodeColor", "[impexp2][lor][helpers]") {
    LOREditEffect e;
    e.startColour = xlWHITE;
    e.endColour = xlWHITE;
    LOREdit::setNodeColor("", e);
    CHECK(e.startColour == xlWHITE);
    LOREdit::setNodeColor("Multi Color", e);
    CHECK(e.startColour == xlWHITE);
    LOREdit::setNodeColor("#FF0000", e);
    CHECK(e.startColour == xlColor(255, 0, 0));
    CHECK(e.endColour == xlColor(255, 0, 0));
    LOREdit::setNodeColor("#00ff00", e);
    CHECK(e.endColour == xlColor(0, 255, 0));
    LOREdit::setNodeColor("not a colour", e);
}

TEST_CASE("LOR embedded picture names", "[impexp2][lor][helpers]") {
    struct Row { const char* in; const char* out; };
    auto r = GENERATE(Row{ "*guitar*f526c123-50dc-48ea-8238-5c2a4855f32e*.png", "guitar-f526c123.png" },
                      Row{ "*a*b*c*12345678-x*.gif", "abc-12345678.gif" },
                      Row{ "*short*abc*.jpg", "short-abc.jpg" },
                      Row{ "plain.png", "plain.png" },
                      Row{ "*onlyone*", "onlyone" },
                      Row{ "", "" },
                      Row{ "***", "" },
                      Row{ "*name*guid*", "nameguid" });
    CHECK(LOREdit::EmbeddedPictureName(r.in) == r.out);
}

TEST_CASE("LOR mouth shape names map to phonemes", "[impexp2][lor][faces]") {
    struct Row { const char* in; const char* out; };
    auto r = GENERATE(Row{ "Closed", "rest" }, Row{ "rest", "rest" }, Row{ "AI (Full Open)", "AI" }, Row{ "ai", "AI" }, Row{ "Full Open", "AI" }, Row{ "AH", "AI" },
                      Row{ "E (Half Open)", "E" }, Row{ "Half Open", "E" }, Row{ "O", "O" }, Row{ "Oh", "O" }, Row{ "OU", "O" }, Row{ "U", "U" }, Row{ "WQ", "WQ" },
                      Row{ "L(th)", "L" }, Row{ "L", "L" }, Row{ "FV", "FV" }, Row{ "MBP", "MBP" }, Row{ "etc", "etc" }, Row{ "ETC", "etc" },
                      Row{ "  Closed  ", "rest" }, Row{ "\"AI\"", "AI" }, Row{ "banana", "" }, Row{ "", "" }, Row{ "(x)", "" }, Row{ "()", "" });
    CHECK(LOREdit::MouthShapeToPhoneme(r.in) == r.out);
}

// ---------- sketch ----------

namespace {
std::string Sketch(const std::string& drawing, const std::string& rest = "100,100,0,0,0,FFFF") { return drawing + "," + rest; }
const std::string kSquarePath = "GSVG a b c d e f g h FFFF0000-1 2.0 True~PPath M 0 0 L 1 0 L 1 1 L 0 1 Z";
}

TEST_CASE("LOR sketch becomes an embedded SVG picture", "[impexp2][lor][sketch]") {
    auto c = Convert(Settings(Layer("sketch", Sketch(kSquarePath))));
    REQUIRE(c.ok);
    CHECK(c.name == "Pictures");
    auto svg = c.eff.GetSketchSVG();
    INFO(svg);
    CHECK(svg.rfind("<svg ", 0) == 0);
    CHECK(Has(svg, "fill-rule=\"evenodd\" fill=\"#FF0000\""));
    CHECK(Has(svg, "d=\"M 0 0 L 1 0 L 1 1 L 0 1 Z\""));
    CHECK(Has(svg, "scale(1.0000 1.0000)"));
    CHECK(Has(svg, "translate(0.0000 0.0000)"));
    CHECK(Has(c.settings, "E_TEXTCTRL_Pictures_Filename=sketch-"));
    CHECK(Has(c.settings, ".svg"));
    CHECK(Has(c.settings, "E_CHOICE_Pictures_Direction=none"));
    CHECK(c.eff.GetSketchPictureName() == c.eff.GetSketchPictureName());
}

TEST_CASE("LOR sketch picture name depends on the drawing", "[impexp2][lor][sketch]") {
    auto a = Convert(Settings(Layer("sketch", Sketch(kSquarePath))));
    auto b = Convert(Settings(Layer("sketch", Sketch(kSquarePath, "50,100,0,0,0,FFFF"))));
    auto d = Convert(Settings(Layer("sketch", Sketch(kSquarePath))));
    CHECK(a.eff.GetSketchPictureName() != b.eff.GetSketchPictureName());
    CHECK(a.eff.GetSketchPictureName() == d.eff.GetSketchPictureName());
    CHECK(a.eff.GetSketchPictureName().size() == std::string("sketch-").size() + 12 + 4);
}

TEST_CASE("LOR sketch unfilled draws a stroke", "[impexp2][lor][sketch]") {
    auto c = Convert(Settings(Layer("sketch", Sketch("GSVG a b c d e f g h FF00FF00-1 250 False~PPath M 0 0 L 1 1"))));
    REQUIRE(c.ok);
    auto svg = c.eff.GetSketchSVG();
    CHECK(Has(svg, "fill=\"none\" stroke=\"#00FF00\" stroke-width=\"2.5000\""));
    CHECK(c.eff.effectType == "sketch");
}

TEST_CASE("LOR sketch pen below one is raised to one", "[impexp2][lor][sketch]") {
    auto c = Convert(Settings(Layer("sketch", Sketch("GSVG a b c d e f g h FF00FF00-1 0 False~PPath M 0 0 L 1 1"))));
    REQUIRE(c.ok);
    CHECK(Has(c.eff.GetSketchSVG(), "stroke-width=\"0.0100\""));
}

TEST_CASE("LOR sketch translucent colour sets opacity", "[impexp2][lor][sketch]") {
    auto c = Convert(Settings(Layer("sketch", Sketch("GSVG a b c d e f g h 80FF0000-1 2 True~PPath M 0 0 L 1 1"))));
    REQUIRE(c.ok);
    auto svg = c.eff.GetSketchSVG();
    CHECK(Has(svg, "fill-opacity=\"0.502\""));
    CHECK(Has(svg, "stroke-opacity=\"0.502\""));
}

TEST_CASE("LOR sketch picks the selected colour out of several", "[impexp2][lor][sketch]") {
    auto c = Convert(Settings(Layer("sketch", Sketch("GSVG a b c d e f g h FF111111-0;FF222222-1;FF333333-0 2 True~PPath M 0 0 L 1 1"))));
    REQUIRE(c.ok);
    CHECK(Has(c.eff.GetSketchSVG(), "fill=\"#222222\""));
}

TEST_CASE("LOR sketch with no selected colour takes the first", "[impexp2][lor][sketch]") {
    auto c = Convert(Settings(Layer("sketch", Sketch("GSVG a b c d e f g h FF111111-0;FF222222-0 2 True~PPath M 0 0 L 1 1"))));
    REQUIRE(c.ok);
    CHECK(Has(c.eff.GetSketchSVG(), "fill=\"#111111\""));
}

TEST_CASE("LOR sketch with a short colour keeps the default white", "[impexp2][lor][sketch]") {
    auto c = Convert(Settings(Layer("sketch", Sketch("GSVG a b c d e f g h FF 2 True~PPath M 0 0 L 1 1"))));
    REQUIRE(c.ok);
    CHECK(Has(c.eff.GetSketchSVG(), "fill=\"#FFFFFF\""));
}

TEST_CASE("LOR sketch groups become separate paths", "[impexp2][lor][sketch]") {
    auto c = Convert(Settings(Layer("sketch", Sketch("GSVG a b c d e f g h FFFF0000-1 2 True~PPath M 0 0 L 1 1~PPath M 2 2 L 3 3~GSVG a b c d e f g h FF0000FF-1 2 True~PPath M 4 4 L 5 5"))));
    REQUIRE(c.ok);
    auto svg = c.eff.GetSketchSVG();
    CHECK(Has(svg, "d=\"M 0 0 L 1 1 M 2 2 L 3 3\""));
    CHECK(Has(svg, "fill=\"#0000FF\""));
    CHECK(Has(svg, "d=\"M 4 4 L 5 5\""));
}

TEST_CASE("LOR sketch conic segments become cubics", "[impexp2][lor][sketch]") {
    auto c = Convert(Settings(Layer("sketch", Sketch("GSVG a b c d e f g h FFFF0000-1 2 True~PPath M 1 0 A 1 1 0 1 0.7071"))));
    REQUIRE(c.ok);
    auto svg = c.eff.GetSketchSVG();
    INFO(svg);
    CHECK(Has(svg, "M 1 0 C "));
    CHECK_FALSE(Has(svg, " A "));
}

TEST_CASE("LOR sketch path commands", "[impexp2][lor][sketch]") {
    struct Row { const char* path; const char* expect; };
    auto r = GENERATE(Row{ "M 0 0 L 1 1", "M 0 0 L 1 1" },
                      Row{ "M 0 0 C 1 1 2 2 3 3", "M 0 0 C 1 1 2 2 3 3" },
                      Row{ "M 0 0 L 1 1 L 2 2 Z", "M 0 0 L 1 1 L 2 2 Z" },
                      Row{ "M 0.123456789 0 L 1 1", "M 0.12346 0 L 1 1" },
                      Row{ "M 0 0 L 1", "M 0 0" },
                      Row{ "M 0 0 C 1 1 2 2 3", "M 0 0" },
                      Row{ "M 0 0 Q 1 1 2 2", "M 0 0" },
                      Row{ "M 0 0 L 1 1 2 2", "M 0 0 L 1 1 L 2 2" },
                      Row{ "M 0 0  L  1 1", "M 0 0 L 1 1" },
                      Row{ "M 5 5 M 6 6", "M 5 5 M 6 6" });
    auto c = Convert(Settings(Layer("sketch", Sketch(std::string("GSVG a b c d e f g h FFFF0000-1 2 True~PPath ") + r.path))));
    REQUIRE(c.ok);
    auto svg = c.eff.GetSketchSVG();
    INFO(svg);
    CHECK(Has(svg, std::string(" d=\"") + r.expect + "\""));
}

TEST_CASE("LOR sketch size rotation and anchor transform", "[impexp2][lor][sketch]") {
    SECTION("half size centred") {
        auto c = Convert(Settings(Layer("sketch", Sketch(kSquarePath, "50,50,0,0,0,FFFF"))));
        auto svg = c.eff.GetSketchSVG();
        CHECK(Has(svg, "translate(0.2500 0.2500) scale(0.5000 0.5000)"));
    }
    SECTION("top anchored") {
        auto c = Convert(Settings(Layer("sketch", Sketch(kSquarePath, "50,50,0,0,0,FFTF"))));
        CHECK(Has(c.eff.GetSketchSVG(), "translate(0.2500 0.0000) scale(0.5000 0.5000)"));
    }
    SECTION("bottom anchored") {
        auto c = Convert(Settings(Layer("sketch", Sketch(kSquarePath, "50,50,0,0,0,FFFT"))));
        CHECK(Has(c.eff.GetSketchSVG(), "translate(0.2500 0.5000) scale(0.5000 0.5000)"));
    }
    SECTION("left anchored") {
        auto c = Convert(Settings(Layer("sketch", Sketch(kSquarePath, "50,50,0,0,0,TFFF"))));
        CHECK(Has(c.eff.GetSketchSVG(), "translate(0.0000 0.2500)"));
    }
    SECTION("right anchored") {
        auto c = Convert(Settings(Layer("sketch", Sketch(kSquarePath, "50,50,0,0,0,FTFF"))));
        CHECK(Has(c.eff.GetSketchSVG(), "translate(0.5000 0.2500)"));
    }
    SECTION("both edges anchored is centred") {
        auto c = Convert(Settings(Layer("sketch", Sketch(kSquarePath, "50,50,0,0,0,TTFF"))));
        CHECK(Has(c.eff.GetSketchSVG(), "translate(0.2500 0.2500)"));
    }
    SECTION("short anchor is padded") {
        auto c = Convert(Settings(Layer("sketch", Sketch(kSquarePath, "50,50,0,0,0,T"))));
        CHECK(Has(c.eff.GetSketchSVG(), "translate(0.0000 0.2500)"));
    }
    SECTION("rotation") {
        auto c = Convert(Settings(Layer("sketch", Sketch(kSquarePath, "100,100,0,0,90,FFFF"))));
        CHECK(Has(c.eff.GetSketchSVG(), "rotate(90.00 0.5000 0.5000) translate("));
    }
    SECTION("rotation wraps") {
        auto c = Convert(Settings(Layer("sketch", Sketch(kSquarePath, "100,100,0,0,450,FFFF"))));
        CHECK(Has(c.eff.GetSketchSVG(), "rotate(90.00 "));
    }
    SECTION("full turn is no rotation") {
        auto c = Convert(Settings(Layer("sketch", Sketch(kSquarePath, "100,100,0,0,360,FFFF"))));
        CHECK_FALSE(Has(c.eff.GetSketchSVG(), "rotate("));
    }
    SECTION("empty size defaults to 100 percent") {
        auto c = Convert(Settings(Layer("sketch", Sketch(kSquarePath, ",,0,0,0,FFFF"))));
        CHECK(Has(c.eff.GetSketchSVG(), "scale(1.0000 1.0000)"));
    }
    SECTION("zoom ramp uses the middle size") {
        auto c = Convert(Settings(Layer("sketch", Sketch(kSquarePath, "R50R150R1.00R2.00R0.00,100,0,0,0,FFFF"))));
        CHECK(Has(c.eff.GetSketchSVG(), "scale(1.0000 1.0000)"));
    }
    SECTION("oscillating size uses the middle") {
        auto c = Convert(Settings(Layer("sketch", Sketch(kSquarePath, "O50O150O1.00,100,0,0,0,FFFF"))));
        CHECK(Has(c.eff.GetSketchSVG(), "scale(1.0000 1.0000)"));
    }
    SECTION("zero size is a degenerate but valid transform") {
        auto c = Convert(Settings(Layer("sketch", Sketch(kSquarePath, "0,0,0,0,0,FFFF"))));
        CHECK(Has(c.eff.GetSketchSVG(), "scale(0.0000 0.0000)"));
    }
    SECTION("negative size mirrors") {
        auto c = Convert(Settings(Layer("sketch", Sketch(kSquarePath, "-100,100,0,0,0,FFFF"))));
        CHECK(Has(c.eff.GetSketchSVG(), "scale(-1.0000 1.0000)"));
    }
    SECTION("offset is baked in") {
        auto c = Convert(Settings(Layer("sketch", Sketch(kSquarePath, "100,100,10,-10,0,FFFF"))));
        CHECK(Has(c.eff.GetSketchSVG(), "translate(0.3000 -0.3000)"));
    }
}

TEST_CASE("LOR sketch position ramp moves with a vector", "[impexp2][lor][sketch]") {
    auto c = Convert(Settings(Layer("sketch", Sketch(kSquarePath, "100,100,R0R10R1.00,0,0,FFFF"))));
    REQUIRE(c.ok);
    INFO(c.settings);
    CHECK(Has(c.settings, "E_CHOICE_Pictures_Direction=vector"));
    CHECK(Has(c.settings, "E_SLIDER_PicturesXC=0,E_SLIDER_PicturesEndXC=30"));
    CHECK(Has(c.settings, "E_SLIDER_PicturesYC=0"));
    CHECK(Has(c.settings, "E_TEXTCTRL_Pictures_Speed=1.0"));
}

TEST_CASE("LOR sketch vertical ramp inverts y", "[impexp2][lor][sketch]") {
    auto c = Convert(Settings(Layer("sketch", Sketch(kSquarePath, "100,100,0,R0R10R1.00,0,FFFF"))));
    REQUIRE(c.ok);
    CHECK(Has(c.settings, "E_SLIDER_PicturesYC=0,E_SLIDER_PicturesEndYC=-30"));
}

TEST_CASE("LOR sketch ramp offset is clamped to the prop", "[impexp2][lor][sketch]") {
    auto c = Convert(Settings(Layer("sketch", Sketch(kSquarePath, "100,100,R0R9999R1.00,0,0,FFFF"))));
    REQUIRE(c.ok);
    CHECK(Has(c.settings, "E_SLIDER_PicturesEndXC=100"));
}

TEST_CASE("LOR sketch with nothing drawable is skipped", "[impexp2][lor][sketch]") {
    SECTION("empty drawing") {
        auto c = Convert(Settings(Layer("sketch", ",100,100,0,0,0,FFFF")));
        REQUIRE(c.ok);
        CHECK(c.eff.GetSketchSVG().empty());
        CHECK(c.name.empty());
    }
    SECTION("no params") {
        auto c = Convert(Settings(Layer("sketch", "")));
        REQUIRE(c.ok);
        CHECK(c.name.empty());
    }
    SECTION("group header only") {
        auto c = Convert(Settings(Layer("sketch", "GSVG a b c d e f g h FFFF0000-1 2 True,100,100,0,0,0,FFFF")));
        REQUIRE(c.ok);
        CHECK(c.name.empty());
    }
    SECTION("path without move") {
        auto c = Convert(Settings(Layer("sketch", "PPath L 1 1,100,100,0,0,0,FFFF")));
        REQUIRE(c.ok);
        CHECK(c.name.empty());
    }
    SECTION("path with an empty command string") {
        auto c = Convert(Settings(Layer("sketch", "PPath M ,100,100,0,0,0,FFFF")));
        CHECK(c.ok);
    }
    SECTION("unknown part types ignored") {
        auto c = Convert(Settings(Layer("sketch", "XJunk~Q1 2 3,100,100,0,0,0,FFFF")));
        REQUIRE(c.ok);
        CHECK(c.name.empty());
    }
    SECTION("effect with only one effect setting") {
        LOREditEffect e;
        e.effectType = "sketch";
        e.effectSettings = { "FFFF0000,1" };
        CHECK(e.GetSketchSVG().empty());
        std::string pal;
        CHECK(e.GetxLightsEffect().empty());
    }
    SECTION("wrong effect type") {
        LOREditEffect e;
        e.effectType = "picture";
        e.effectSettings = { "x", kSquarePath };
        CHECK(e.GetSketchSVG().empty());
    }
}

TEST_CASE("LOR sketch path with a path before any group header uses defaults", "[impexp2][lor][sketch]") {
    auto c = Convert(Settings(Layer("sketch", "PPath M 0 0 L 1 1,100,100,0,0,0,FFFF")));
    REQUIRE(c.ok);
    auto svg = c.eff.GetSketchSVG();
    CHECK(Has(svg, "fill=\"#FFFFFF\""));
    CHECK(Has(svg, "fill-rule=\"evenodd\""));
}

TEST_CASE("LOR sketch group colour with huge or invalid alpha", "[impexp2][lor][sketch]") {
    auto c = Convert(Settings(Layer("sketch", Sketch("GSVG a b c d e f g h ZZ123456-1 2 True~PPath M 0 0 L 1 1"))));
    REQUIRE(c.ok);
    auto svg = c.eff.GetSketchSVG();
    CHECK(Has(svg, "fill=\"#123456\""));
    CHECK(Has(svg, "fill-opacity=\"0.000\""));
}

TEST_CASE("LOR zero length effects convert without trapping", "[impexp2][lor][effects]") {
    // speed maths divide by the duration
    for (const char* t : { "spirals", "bars", "blendedbars" }) {
        INFO(t);
        auto c = Convert(Settings(Layer(t, "1,left_to_right,20,50,0,False,none,50")), 100, 100);
        CHECK(c.ok);
    }
}

TEST_CASE("LOR backwards effects convert without trapping", "[impexp2][lor][effects]") {
    for (const char* t : { "spirals", "bars", "blendedbars" }) {
        INFO(t);
        auto c = Convert(Settings(Layer(t, "1,left_to_right,20,50,0,False,none,50")), 500, 100);
        CHECK(c.ok);
    }
}
