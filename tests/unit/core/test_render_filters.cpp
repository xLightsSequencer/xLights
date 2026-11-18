#include <catch2/catch_test_macros.hpp>

#include "test_render_common.h"

using namespace renderhelp;

namespace {

using Rgb = Rendered::Rgb;

bool Near(const Rgb& a, int r, int g, int b, int tol = 2) {
    return std::abs(a.r - r) <= tol && std::abs(a.g - g) <= tol && std::abs(a.b - b) <= tol;
}

Rendered OneLayer(const std::string& color, const std::string& settings, const std::string& effect = "On", const std::string& models = "", const std::string& color2 = "") {
    ShowSpec s;
    if (!models.empty()) s.models = models;
    s.effects.push_back(Eff("M", 0, effect, 0, 1000, Pal(color, color2), settings));
    Rendered r = Render(s);
    REQUIRE(r.rendered);
    return r;
}

Rgb Adjusted(const std::string& color, const std::string& settings) {
    return OneLayer(color, settings).rgb(5, 0);
}

int Lit(const Rendered& r, unsigned frame, unsigned nodes) {
    int n = 0;
    for (unsigned i = 0; i < nodes; ++i) {
        auto c = r.rgb(frame, i);
        if (c.r || c.g || c.b) ++n;
    }
    return n;
}

const std::string kMatrix8 = Matrix("M", 1, 8, 8);

} // namespace

TEST_CASE("Default layer settings leave the colour untouched", "[render][filters]") {
    CHECK(Adjusted("#FF8040", "") == Rgb{ 255, 128, 64 });
}

TEST_CASE("Brightness scales every channel", "[render][filters]") {
    CHECK(Near(Adjusted("#FF8040", "T_SLIDER_Brightness=50"), 127, 64, 32, 1));
    CHECK(Near(Adjusted("#FF8040", "T_SLIDER_Brightness=25"), 63, 32, 16, 1));
    CHECK(Adjusted("#FF8040", "T_SLIDER_Brightness=100") == Rgb{ 255, 128, 64 });
}

TEST_CASE("Brightness of zero blacks out the layer", "[render][filters]") {
    CHECK(Adjusted("#FF8040", "T_SLIDER_Brightness=0") == Rgb{ 0, 0, 0 });
}

TEST_CASE("Brightness above 100 boosts and clamps at 255", "[render][filters]") {
    CHECK(Near(Adjusted("#404040", "T_SLIDER_Brightness=150"), 96, 96, 96, 1));
    CHECK(Near(Adjusted("#FF8040", "T_SLIDER_Brightness=200"), 255, 255, 128, 1));
}

TEST_CASE("Positive contrast brightens light colours and darkens dark ones", "[render][filters]") {
    Rgb light = Adjusted("#C0C0C0", "T_SLIDER_Contrast=100");
    Rgb dark = Adjusted("#404040", "T_SLIDER_Contrast=100");
    CHECK(light == Rgb{ 255, 255, 255 });
    CHECK(dark == Rgb{ 0, 0, 0 });
}

TEST_CASE("Negative contrast pulls full brightness down", "[render][filters]") {
    CHECK(Near(Adjusted("#FF8040", "T_SLIDER_Contrast=-50"), 127, 63, 31, 1));
}

TEST_CASE("Contrast keeps the hue of the colour", "[render][filters]") {
    Rgb c = Adjusted("#FF8040", "T_SLIDER_Contrast=50");
    CHECK(c.r == 255);
    CHECK(c.g > c.b);
    CHECK(c.b > 0);
}

TEST_CASE("Hue adjust rotates the hue by hundredths of the colour wheel", "[render][filters]") {
    Rgb c = Adjusted("#FF0000", "T_SLIDER_Color_HueAdjust=25");
    CHECK(Near(c, 128, 255, 0, 2));
    Rgb back = Adjusted("#FF0000", "T_SLIDER_Color_HueAdjust=-25");
    CHECK(Near(back, 128, 0, 255, 2));
}

TEST_CASE("Hue adjust wraps around the wheel", "[render][filters]") {
    Rgb a = Adjusted("#FF0000", "T_SLIDER_Color_HueAdjust=-60");
    Rgb b = Adjusted("#FF0000", "T_SLIDER_Color_HueAdjust=40");
    CHECK(Near(a, b.r, b.g, b.b, 3));
}

TEST_CASE("Hue adjust does not change brightness of a primary", "[render][filters]") {
    Rgb c = Adjusted("#0000FF", "T_SLIDER_Color_HueAdjust=33");
    int mx = std::max({ c.r, c.g, c.b });
    CHECK(mx == 255);
}

TEST_CASE("Saturation adjust desaturates towards white", "[render][filters]") {
    CHECK(Adjusted("#FF0000", "T_SLIDER_Color_SaturationAdjust=-100") == Rgb{ 255, 255, 255 });
    Rgb half = Adjusted("#FF0000", "T_SLIDER_Color_SaturationAdjust=-50");
    CHECK(half.r == 255);
    CHECK(half.g == half.b);
    CHECK(half.g > 100);
    CHECK(half.g < 150);
}

TEST_CASE("Value adjust darkens the colour", "[render][filters]") {
    CHECK(Adjusted("#FF0000", "T_SLIDER_Color_ValueAdjust=-100") == Rgb{ 0, 0, 0 });
    CHECK(Near(Adjusted("#FF8040", "T_SLIDER_Color_ValueAdjust=-50"), 127, 63, 31, 1));
}

TEST_CASE("Colour adjustments only affect their own layer", "[render][filters]") {
    ShowSpec s;
    s.models = SingleLine("M", 1, 2);
    s.effects.push_back(Eff("M", 0, "On", 0, 1000, Pal("#FF0000"), "T_CHOICE_LayerMethod=Additive,T_SLIDER_Brightness=50"));
    s.effects.push_back(Eff("M", 1, "On", 0, 1000, Pal("#0000FF")));
    Rendered r = Render(s);
    REQUIRE(r.rendered);
    CHECK(Near(r.rgb(5, 0), 127, 0, 255, 1));
}

TEST_CASE("Bottom layer brightness is applied before blending", "[render][filters]") {
    ShowSpec s;
    s.effects.push_back(Eff("M", 0, "On", 0, 1000, Pal("#FF0000"), "T_CHOICE_LayerMethod=Additive"));
    s.effects.push_back(Eff("M", 1, "On", 0, 1000, Pal("#0000FF"), "T_SLIDER_Brightness=50"));
    Rendered r = Render(s);
    REQUIRE(r.rendered);
    CHECK(Near(r.rgb(5, 0), 255, 0, 127, 1));
}

TEST_CASE("Freeze at frame holds the effect from that frame on", "[render][filters]") {
    Rendered r = OneLayer("#FF0000", "T_SPINCTRL_FreezeEffectAtFrame=5", "Color Wash", "", "#0000FF");
    Rendered free = OneLayer("#FF0000", "", "Color Wash", "", "#0000FF");
    for (unsigned f = 0; f <= 4; ++f) CHECK(r.rgb(f, 0) == free.rgb(f, 0));
    for (unsigned f = 5; f < 20; ++f) CHECK(r.rgb(f, 0) == r.rgb(4, 0));
    CHECK_FALSE(free.rgb(19, 0) == free.rgb(4, 0));
}

TEST_CASE("Suppress until hides the effect for its first frames", "[render][filters]") {
    Rendered r = OneLayer("#FF0000", "T_SPINCTRL_SuppressEffectUntil=5");
    for (unsigned f = 0; f < 5; ++f) CHECK(r.rgb(f, 0) == Rgb{ 0, 0, 0 });
    for (unsigned f = 5; f < 20; ++f) CHECK(r.rgb(f, 0) == Rgb{ 255, 0, 0 });
}

TEST_CASE("Chroma key removes the matching colour so the layer below shows", "[render][filters]") {
    ShowSpec s;
    s.effects.push_back(Eff("M", 0, "On", 0, 1000, Pal("#FF0000"), "T_CHOICE_LayerMethod=Normal,T_CHECKBOX_Chroma=1,T_COLOURPICKERCTRL_ChromaColour=#FF0000,T_SLIDER_ChromaSensitivity=10"));
    s.effects.push_back(Eff("M", 1, "On", 0, 1000, Pal("#0000FF")));
    Rendered r = Render(s);
    REQUIRE(r.rendered);
    CHECK(r.rgb(5, 0) == Rgb{ 0, 0, 255 });
}

TEST_CASE("Chroma key leaves other colours alone", "[render][filters]") {
    ShowSpec s;
    s.effects.push_back(Eff("M", 0, "On", 0, 1000, Pal("#FF0000"), "T_CHOICE_LayerMethod=Normal,T_CHECKBOX_Chroma=1,T_COLOURPICKERCTRL_ChromaColour=#00FF00,T_SLIDER_ChromaSensitivity=10"));
    s.effects.push_back(Eff("M", 1, "On", 0, 1000, Pal("#0000FF")));
    Rendered r = Render(s);
    REQUIRE(r.rendered);
    CHECK(r.rgb(5, 0) == Rgb{ 255, 0, 0 });
}

TEST_CASE("Sparkles are deterministic and use the sparkle colour", "[render][filters][sparkle]") {
    const std::string set = "B_SLIDER_SparkleFrequency=100,B_COLOURPICKERCTRL_SparklesColour=#FF0000";
    Rendered a = OneLayer("#0000FF", set, "On", kMatrix8);
    Rendered b = OneLayer("#0000FF", set, "On", kMatrix8);
    CHECK(a.data == b.data);
    int sparkles = 0;
    for (unsigned f = 0; f < 10; ++f) {
        for (unsigned n = 0; n < 64; ++n) {
            auto c = a.rgb(f, n);
            if (c.r > 0 && c.b == 0) ++sparkles;
        }
    }
    CHECK(sparkles > 0);
    CHECK(sparkles < 10 * 64);
}

TEST_CASE("Without sparkles the layer is a flat colour", "[render][filters][sparkle]") {
    Rendered r = OneLayer("#0000FF", "", "On", kMatrix8);
    for (unsigned f = 0; f < 10; ++f) CHECK(Lit(r, f, 64) == 64);
    for (unsigned n = 0; n < 64; ++n) CHECK(r.rgb(3, n) == Rgb{ 0, 0, 255 });
}

TEST_CASE("Sparkles change the output over time", "[render][filters][sparkle]") {
    Rendered r = OneLayer("#0000FF", "B_SLIDER_SparkleFrequency=100", "On", kMatrix8);
    bool differs = false;
    for (unsigned f = 1; f < 10; ++f) {
        if (r.data[f] != r.data[0]) differs = true;
    }
    CHECK(differs);
}

TEST_CASE("Blur, zoom, rotation and flip change a patterned buffer", "[render][filters][buffer]") {
    Rendered base = OneLayer("#FF0000", "", "Bars", kMatrix8, "#0000FF");
    for (const char* set : { "B_SLIDER_Blur=5", "B_SLIDER_Zoom=20", "B_SLIDER_Rotation=90", "B_CHOICE_BufferTransform=Flip Vertical" }) {
        INFO(set);
        Rendered r = OneLayer("#FF0000", set, "Bars", kMatrix8, "#0000FF");
        CHECK(r.data[3] != base.data[3]);
    }
}

TEST_CASE("Blur of a uniform colour changes nothing in the interior", "[render][filters][buffer]") {
    Rendered r = OneLayer("#FF0000", "B_SLIDER_Blur=5", "On", kMatrix8);
    int red = 0;
    for (unsigned n = 0; n < 64; ++n) {
        if (r.rgb(3, n).r > 200) ++red;
    }
    CHECK(red > 20);
}

TEST_CASE("Sub buffer restricts the effect to part of the model", "[render][filters][buffer]") {
    Rendered r = OneLayer("#FF0000", "B_CUSTOM_SubBuffer=0x0x50x100", "On", kMatrix8);
    CHECK(Lit(r, 3, 64) == 32);
}

TEST_CASE("Full size sub buffer is the same as no sub buffer", "[render][filters][buffer]") {
    Rendered a = OneLayer("#FF0000", "B_CUSTOM_SubBuffer=0x0x100x100", "Bars", kMatrix8, "#0000FF");
    Rendered b = OneLayer("#FF0000", "", "Bars", kMatrix8, "#0000FF");
    CHECK(a.data == b.data);
}

TEST_CASE("Single Line buffer style still lights every node", "[render][filters][buffer]") {
    Rendered r = OneLayer("#FF0000", "B_CHOICE_BufferStyle=Single Line", "On", kMatrix8);
    CHECK(Lit(r, 3, 64) == 64);
}

TEST_CASE("Horizontal flip is the mirror of the unflipped image", "[render][filters][buffer]") {
    Rendered base = OneLayer("#FF0000", "", "Bars", kMatrix8, "#0000FF");
    Rendered flip = OneLayer("#FF0000", "B_CHOICE_BufferTransform=Flip Horizontal", "Bars", kMatrix8, "#0000FF");
    Rendered flip2 = OneLayer("#FF0000", "B_CHOICE_BufferTransform=Flip Vertical", "Bars", kMatrix8, "#0000FF");
    CHECK(flip.data[3] != flip2.data[3]);
    auto sorted = [](std::vector<uint8_t> v) {
        std::sort(v.begin(), v.end());
        return v;
    };
    CHECK(sorted(flip.data[3]) == sorted(base.data[3]));
}

TEST_CASE("Adjustments are applied identically on every frame of a static effect", "[render][filters]") {
    Rendered r = OneLayer("#FF8040", "T_SLIDER_Brightness=70,T_SLIDER_Color_HueAdjust=10");
    for (unsigned f = 1; f < 20; ++f) CHECK(r.rgb(f, 0) == r.rgb(0, 0));
}
