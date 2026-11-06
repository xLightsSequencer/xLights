#include <catch2/catch_test_macros.hpp>

#include "test_render_common.h"
#include "render/PixelBuffer.h"

using namespace renderhelp;

namespace {

using Rgb = Rendered::Rgb;

bool Near(const Rgb& a, int r, int g, int b, int tol = 2) {
    return std::abs(a.r - r) <= tol && std::abs(a.g - g) <= tol && std::abs(a.b - b) <= tol;
}

// Layer 0 (effect 1, carries the layer method) is blended onto layer 1 (effect 2).
Rendered TwoLayers(const std::string& top, const std::string& bottom, const std::string& method, const std::string& extra = "", const std::string& models = "") {
    ShowSpec s;
    if (!models.empty()) s.models = models;
    s.effects.push_back(Eff("M", 0, "On", 0, 1000, Pal(top), "T_CHOICE_LayerMethod=" + method + extra));
    s.effects.push_back(Eff("M", 1, "On", 0, 1000, Pal(bottom)));
    Rendered r = Render(s);
    REQUIRE(r.rendered);
    return r;
}

Rgb Mix(const std::string& top, const std::string& bottom, const std::string& method, const std::string& extra = "") {
    return TwoLayers(top, bottom, method, extra).rgb(5, 0);
}

int CountNodes(const Rendered& r, unsigned frame, unsigned nodes, const Rgb& c) {
    int n = 0;
    for (unsigned i = 0; i < nodes; ++i) {
        if (Near(r.rgb(frame, i), c.r, c.g, c.b)) ++n;
    }
    return n;
}

} // namespace

TEST_CASE("Mix type list names every blend mode", "[render][blend]") {
    auto names = PixelBufferClass::GetMixTypes();
    for (const char* n : { "Normal", "Effect 1", "Effect 2", "1 is Mask", "2 is Mask", "1 is Unmask", "2 is Unmask", "1 is True Unmask", "2 is True Unmask", "1 reveals 2", "2 reveals 1", "Shadow 1 on 2", "Shadow 2 on 1", "Layered", "Highlight", "Highlight Vibrant", "Additive", "Subtractive", "Brightness", "Average", "Bottom-Top", "Left-Right", "Max", "Min" }) {
        INFO(n);
        CHECK(std::find(names.begin(), names.end(), n) != names.end());
    }
    CHECK(names.size() == 24);
}

TEST_CASE("Normal blend shows the top layer when opaque", "[render][blend]") {
    CHECK(Mix("#FF0000", "#0000FF", "Normal") == Rgb{ 255, 0, 0 });
}

TEST_CASE("Normal blend with a black top layer is not transparent", "[render][blend]") {
    // black is an opaque colour, so it covers the layer below
    Rgb c = Mix("#000000", "#0000FF", "Normal");
    CHECK(Near(c, 0, 0, 0));
}

TEST_CASE("Effect 1 shows only the top layer", "[render][blend]") {
    CHECK(Mix("#FF0000", "#0000FF", "Effect 1") == Rgb{ 255, 0, 0 });
    CHECK(Mix("#336699", "#FFFF00", "Effect 1") == Rgb{ 0x33, 0x66, 0x99 });
}

TEST_CASE("Effect 2 shows only the bottom layer", "[render][blend]") {
    CHECK(Mix("#FF0000", "#0000FF", "Effect 2") == Rgb{ 0, 0, 255 });
    CHECK(Mix("#336699", "#FFFF00", "Effect 2") == Rgb{ 255, 255, 0 });
}

TEST_CASE("Unknown layer method falls back to Effect 1", "[render][blend]") {
    CHECK(Mix("#FF0000", "#0000FF", "No Such Method") == Rgb{ 255, 0, 0 });
}

TEST_CASE("Effect 1 with the mix slider cross fades", "[render][blend]") {
    CHECK(Near(Mix("#FF0000", "#0000FF", "Effect 1", ",T_SLIDER_EffectLayerMix=0"), 255, 0, 0));
    CHECK(Near(Mix("#FF0000", "#0000FF", "Effect 1", ",T_SLIDER_EffectLayerMix=100"), 0, 0, 255));
    Rgb mid = Mix("#FF0000", "#0000FF", "Effect 1", ",T_SLIDER_EffectLayerMix=50");
    CHECK(mid.g == 0);
    CHECK(std::abs(mid.r - mid.b) <= 2);
    CHECK(mid.r > 100);
    CHECK(mid.r < 255);
}

TEST_CASE("Effect 2 with the mix slider is the mirror of Effect 1", "[render][blend]") {
    CHECK(Near(Mix("#FF0000", "#0000FF", "Effect 2", ",T_SLIDER_EffectLayerMix=0"), 0, 0, 255));
    CHECK(Near(Mix("#FF0000", "#0000FF", "Effect 2", ",T_SLIDER_EffectLayerMix=100"), 255, 0, 0));
}

TEST_CASE("1 reveals 2 shows the top colour where it is not black", "[render][blend]") {
    CHECK(Mix("#FF0000", "#0000FF", "1 reveals 2") == Rgb{ 255, 0, 0 });
}

TEST_CASE("1 reveals 2 shows the bottom layer where the top is black", "[render][blend]") {
    CHECK(Mix("#000000", "#0000FF", "1 reveals 2") == Rgb{ 0, 0, 255 });
}

TEST_CASE("1 reveals 2 honours the mix threshold", "[render][blend]") {
    CHECK(Near(Mix("#800000", "#0000FF", "1 reveals 2", ",T_SLIDER_EffectLayerMix=40"), 128, 0, 0, 1));
    CHECK(Near(Mix("#800000", "#0000FF", "1 reveals 2", ",T_SLIDER_EffectLayerMix=60"), 0, 0, 255, 1));
}

TEST_CASE("2 reveals 1 shows the top layer only where the bottom is black", "[render][blend]") {
    CHECK(Mix("#FF0000", "#0000FF", "2 reveals 1") == Rgb{ 0, 0, 255 });
    CHECK(Mix("#FF0000", "#000000", "2 reveals 1") == Rgb{ 255, 0, 0 });
}

TEST_CASE("1 is Mask blanks the bottom where the top is lit", "[render][blend]") {
    CHECK(Mix("#FF0000", "#0000FF", "1 is Mask") == Rgb{ 0, 0, 0 });
    CHECK(Mix("#000000", "#0000FF", "1 is Mask") == Rgb{ 0, 0, 255 });
}

TEST_CASE("2 is Mask uses the top colour where the bottom is black", "[render][blend]") {
    CHECK(Mix("#FF0000", "#000000", "2 is Mask") == Rgb{ 255, 0, 0 });
    CHECK(Mix("#FF0000", "#0000FF", "2 is Mask") == Rgb{ 0, 0, 0 });
}

TEST_CASE("1 is Unmask keeps the bottom hue at the top brightness", "[render][blend]") {
    CHECK(Near(Mix("#800000", "#00FF40", "1 is Unmask"), 0, 128, 32, 1));
    CHECK(Mix("#000000", "#00FF40", "1 is Unmask") == Rgb{ 0, 0, 0 });
}

TEST_CASE("2 is Unmask keeps the top hue at the bottom brightness", "[render][blend]") {
    CHECK(Near(Mix("#800000", "#00FF40", "2 is Unmask"), 255, 0, 0, 1));
    CHECK(Mix("#800000", "#000000", "2 is Unmask") == Rgb{ 0, 0, 0 });
}

TEST_CASE("1 is True Unmask shows the bottom where the top is lit", "[render][blend]") {
    CHECK(Mix("#800000", "#00FF40", "1 is True Unmask") == Rgb{ 0, 255, 64 });
    CHECK(Mix("#000000", "#00FF40", "1 is True Unmask") == Rgb{ 0, 0, 0 });
}

TEST_CASE("2 is True Unmask shows the top where the bottom is lit", "[render][blend]") {
    CHECK(Mix("#800000", "#00FF40", "2 is True Unmask") == Rgb{ 128, 0, 0 });
    CHECK(Mix("#800000", "#000000", "2 is True Unmask") == Rgb{ 0, 0, 0 });
}

TEST_CASE("Additive adds channels and clamps at 255", "[render][blend]") {
    CHECK(Mix("#FF0000", "#0000FF", "Additive") == Rgb{ 255, 0, 255 });
    CHECK(Mix("#102030", "#010203", "Additive") == Rgb{ 0x11, 0x22, 0x33 });
    CHECK(Mix("#C0C0C0", "#C0C0C0", "Additive") == Rgb{ 255, 255, 255 });
}

TEST_CASE("Subtractive subtracts the top from the bottom and clamps at zero", "[render][blend]") {
    CHECK(Mix("#FF0000", "#0000FF", "Subtractive") == Rgb{ 0, 0, 255 });
    CHECK(Mix("#102030", "#305070", "Subtractive") == Rgb{ 0x20, 0x30, 0x40 });
    CHECK(Mix("#FFFFFF", "#808080", "Subtractive") == Rgb{ 0, 0, 0 });
}

TEST_CASE("Average halves the sum of two lit layers", "[render][blend]") {
    CHECK(Near(Mix("#FF0000", "#0000FF", "Average"), 127, 0, 127, 1));
    CHECK(Near(Mix("#FFFFFF", "#000040", "Average"), 127, 127, 159, 1));
}

TEST_CASE("Average leaves the other layer alone when one is black", "[render][blend]") {
    CHECK(Mix("#000000", "#0000FF", "Average") == Rgb{ 0, 0, 255 });
    CHECK(Mix("#FF0000", "#000000", "Average") == Rgb{ 255, 0, 0 });
}

TEST_CASE("Max takes the per channel maximum", "[render][blend]") {
    CHECK(Mix("#FF0000", "#0000FF", "Max") == Rgb{ 255, 0, 255 });
    CHECK(Mix("#204060", "#6040FF", "Max") == Rgb{ 0x60, 0x40, 0xFF });
}

TEST_CASE("Min takes the per channel minimum", "[render][blend]") {
    CHECK(Mix("#FF0000", "#0000FF", "Min") == Rgb{ 0, 0, 0 });
    CHECK(Mix("#FFFF00", "#00FFFF", "Min") == Rgb{ 0, 255, 0 });
    CHECK(Mix("#204060", "#6040FF", "Min") == Rgb{ 0x20, 0x40, 0x60 });
}

TEST_CASE("Brightness multiplies the layers channel by channel", "[render][blend]") {
    CHECK(Mix("#FF0000", "#0000FF", "Brightness") == Rgb{ 0, 0, 0 });
    CHECK(Near(Mix("#FFFFFF", "#804020", "Brightness"), 0x80, 0x40, 0x20, 1));
    CHECK(Near(Mix("#808080", "#FFFFFF", "Brightness"), 0x80, 0x80, 0x80, 1));
}

TEST_CASE("Layered shows the top only where the bottom is black", "[render][blend]") {
    CHECK(Mix("#FF0000", "#0000FF", "Layered") == Rgb{ 0, 0, 255 });
    CHECK(Mix("#FF0000", "#000000", "Layered") == Rgb{ 255, 0, 0 });
}

TEST_CASE("Highlight shows the top where both layers have colour", "[render][blend]") {
    CHECK(Mix("#FF0000", "#0000FF", "Highlight") == Rgb{ 255, 0, 0 });
    CHECK(Mix("#FF0000", "#000000", "Highlight") == Rgb{ 0, 0, 0 });
    CHECK(Mix("#000000", "#0000FF", "Highlight") == Rgb{ 0, 0, 255 });
}

TEST_CASE("Highlight Vibrant adds the top onto a lit bottom", "[render][blend]") {
    CHECK(Mix("#FF0000", "#0000FF", "Highlight Vibrant") == Rgb{ 255, 0, 255 });
    CHECK(Mix("#FF0000", "#000000", "Highlight Vibrant") == Rgb{ 0, 0, 0 });
}

TEST_CASE("Shadow 1 on 2 leaves the bottom alone when the top is black", "[render][blend]") {
    CHECK(Near(Mix("#000000", "#00FF40", "Shadow 1 on 2"), 0, 255, 64, 1));
}

TEST_CASE("Shadow 1 on 2 shifts the bottom hue away from the top hue", "[render][blend]") {
    Rgb c = Mix("#FF0000", "#0000FF", "Shadow 1 on 2");
    CHECK(c.b == 255);
    CHECK(c.g == 0);
    CHECK(c.r > 100);
    CHECK(c.r < 255);
}

TEST_CASE("Shadow 2 on 1 shifts the top hue using the bottom", "[render][blend]") {
    // hue(top)=1/3, hue(bottom)=0: hue becomes 0.5 (cyan)
    Rgb c = Mix("#00FF00", "#FF0000", "Shadow 2 on 1");
    CHECK(Near(c, 0, 255, 255, 2));
}

TEST_CASE("Shadow 2 on 1 with a hue that goes negative is not black", "[render][blend]") {
    Rgb c = Mix("#FF0000", "#00FF00", "Shadow 2 on 1");
    CHECK((c.r != 0 || c.g != 0 || c.b != 0));
}

TEST_CASE("Bottom-Top takes the top layer in the lower half of the buffer", "[render][blend]") {
    Rendered r = TwoLayers("#FF0000", "#0000FF", "Bottom-Top", "", Matrix("M", 1, 4, 4));
    CHECK(CountNodes(r, 5, 16, Rgb{ 255, 0, 0 }) == 8);
    CHECK(CountNodes(r, 5, 16, Rgb{ 0, 0, 255 }) == 8);
}

TEST_CASE("Left-Right takes the top layer in the left half of the buffer", "[render][blend]") {
    Rendered r = TwoLayers("#FF0000", "#0000FF", "Left-Right", "", Matrix("M", 1, 4, 4));
    CHECK(CountNodes(r, 5, 16, Rgb{ 255, 0, 0 }) == 8);
    CHECK(CountNodes(r, 5, 16, Rgb{ 0, 0, 255 }) == 8);
}

TEST_CASE("Bottom-Top and Left-Right split along different axes", "[render][blend]") {
    Rendered bt = TwoLayers("#FF0000", "#0000FF", "Bottom-Top", "", Matrix("M", 1, 4, 4));
    Rendered lr = TwoLayers("#FF0000", "#0000FF", "Left-Right", "", Matrix("M", 1, 4, 4));
    CHECK(bt.data[5] != lr.data[5]);
}

TEST_CASE("Three layers add up through nested blending", "[render][blend]") {
    ShowSpec s;
    s.effects.push_back(Eff("M", 0, "On", 0, 1000, Pal("#FF0000"), "T_CHOICE_LayerMethod=Additive"));
    s.effects.push_back(Eff("M", 1, "On", 0, 1000, Pal("#00FF00"), "T_CHOICE_LayerMethod=Additive"));
    s.effects.push_back(Eff("M", 2, "On", 0, 1000, Pal("#0000FF")));
    Rendered r = Render(s);
    REQUIRE(r.rendered);
    CHECK(r.rgb(5, 0) == Rgb{ 255, 255, 255 });
}

TEST_CASE("A layer with no effect at a given time does not contribute", "[render][blend]") {
    ShowSpec s;
    s.effects.push_back(Eff("M", 0, "On", 0, 500, Pal("#FF0000"), "T_CHOICE_LayerMethod=Additive"));
    s.effects.push_back(Eff("M", 1, "On", 0, 1000, Pal("#0000FF")));
    Rendered r = Render(s);
    REQUIRE(r.rendered);
    CHECK(r.rgb(3, 0) == Rgb{ 255, 0, 255 });
    CHECK(r.rgb(15, 0) == Rgb{ 0, 0, 255 });
}

TEST_CASE("Layer method is read per layer effect", "[render][blend]") {
    ShowSpec s;
    s.effects.push_back(Eff("M", 0, "On", 0, 500, Pal("#FF0000"), "T_CHOICE_LayerMethod=Additive"));
    s.effects.push_back(Eff("M", 0, "On", 500, 1000, Pal("#FF0000"), "T_CHOICE_LayerMethod=Effect 2"));
    s.effects.push_back(Eff("M", 1, "On", 0, 1000, Pal("#0000FF")));
    Rendered r = Render(s);
    REQUIRE(r.rendered);
    CHECK(r.rgb(3, 0) == Rgb{ 255, 0, 255 });
    CHECK(r.rgb(15, 0) == Rgb{ 0, 0, 255 });
}

TEST_CASE("Fading in a top layer cross fades over the layer below", "[render][blend][fade]") {
    ShowSpec s;
    s.effects.push_back(Eff("M", 0, "On", 0, 1000, Pal("#FF0000"), "T_CHOICE_LayerMethod=Normal,T_TEXTCTRL_Fadein=0.5"));
    s.effects.push_back(Eff("M", 1, "On", 0, 1000, Pal("#0000FF")));
    Rendered r = Render(s);
    REQUIRE(r.rendered);
    int prevR = -1;
    for (unsigned f = 0; f < 10; ++f) {
        auto c = r.rgb(f, 0);
        INFO("frame " << f);
        CHECK(c.r > prevR);
        CHECK(std::abs(c.r + c.b - 255) <= 2);
        CHECK(c.g == 0);
        prevR = c.r;
    }
    CHECK(Near(r.rgb(0, 0), 25, 0, 230, 2));
    CHECK(Near(r.rgb(4, 0), 127, 0, 127, 2));
    for (unsigned f = 10; f < 20; ++f) CHECK(r.rgb(f, 0) == Rgb{ 255, 0, 0 });
}

TEST_CASE("Fading out a top layer reveals the layer below", "[render][blend][fade]") {
    ShowSpec s;
    s.effects.push_back(Eff("M", 0, "On", 0, 1000, Pal("#FF0000"), "T_CHOICE_LayerMethod=Normal,T_TEXTCTRL_Fadeout=0.5"));
    s.effects.push_back(Eff("M", 1, "On", 0, 1000, Pal("#0000FF")));
    Rendered r = Render(s);
    REQUIRE(r.rendered);
    for (unsigned f = 0; f <= 9; ++f) CHECK(r.rgb(f, 0) == Rgb{ 255, 0, 0 });
    int prevR = 256;
    for (unsigned f = 10; f < 20; ++f) {
        auto c = r.rgb(f, 0);
        INFO("frame " << f);
        CHECK(c.r < prevR);
        prevR = c.r;
    }
    CHECK(Near(r.rgb(19, 0), 0, 0, 255, 2));
}

TEST_CASE("Morphing mix changes the blend over the effect duration", "[render][blend]") {
    Rendered r = TwoLayers("#FF0000", "#0000FF", "Effect 1", ",T_SLIDER_EffectLayerMix=100,T_CHECKBOX_LayerMorph=1");
    int firstB = r.rgb(0, 0).b;
    int lastB = r.rgb(19, 0).b;
    CHECK(firstB != lastB);
}

TEST_CASE("Blend result is stable across repeated renders", "[render][blend]") {
    for (const std::string m : { "Normal", "Additive", "Average", "Shadow 1 on 2", "1 is Unmask", "Highlight Vibrant" }) {
        Rendered a = TwoLayers("#C04020", "#2040C0", m);
        Rendered b = TwoLayers("#C04020", "#2040C0", m);
        INFO(m);
        CHECK(a.data == b.data);
    }
}
