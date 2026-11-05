#include <catch2/catch_test_macros.hpp>

#include "test_render_common.h"

using namespace renderhelp;

namespace {

using Rgb = Rendered::Rgb;

const std::string kMatrix8 = Matrix("M", 1, 8, 8);

std::vector<int> LitPerFrame(const Rendered& r, unsigned nodes) {
    std::vector<int> v;
    for (unsigned f = 0; f < r.frames; ++f) {
        int n = 0;
        for (unsigned i = 0; i < nodes; ++i) {
            auto c = r.rgb(f, i);
            if (c.r || c.g || c.b) ++n;
        }
        v.push_back(n);
    }
    return v;
}

Rendered Transition(const std::string& dir, const std::string& type, const std::string& extra = "", const std::string& models = kMatrix8, bool overBlue = false) {
    ShowSpec s;
    s.models = models;
    const std::string pre = dir == "in" ? "In" : "Out";
    const std::string time = dir == "in" ? "Fadein" : "Fadeout";
    s.effects.push_back(Eff("M", 0, "On", 0, 1000, Pal("#FF0000"),
                            "T_CHOICE_" + pre + "_Transition_Type=" + type + ",T_TEXTCTRL_" + time + "=0.5,T_CHOICE_LayerMethod=Normal" + extra));
    if (overBlue) s.effects.push_back(Eff("M", 1, "On", 0, 1000, Pal("#0000FF")));
    Rendered r = Render(s);
    REQUIRE(r.rendered);
    return r;
}

} // namespace

TEST_CASE("Fade in ramps the brightness over the fade time", "[render][transitions]") {
    ShowSpec s;
    s.effects.push_back(Eff("M", 0, "On", 0, 1000, Pal("#FF0000"), "T_TEXTCTRL_Fadein=0.5"));
    Rendered r = Render(s);
    REQUIRE(r.rendered);
    int prev = 0;
    for (unsigned f = 0; f < 10; ++f) {
        INFO("frame " << f);
        CHECK(r.rgb(f, 0).r > prev);
        CHECK(r.rgb(f, 0).g == 0);
        prev = r.rgb(f, 0).r;
    }
    CHECK(std::abs(r.rgb(0, 0).r - 25) <= 2);
    CHECK(std::abs(r.rgb(4, 0).r - 127) <= 2);
    for (unsigned f = 9; f < 20; ++f) CHECK(r.rgb(f, 0).r == 255);
}

TEST_CASE("Fade out ramps the brightness down to black on the last frame", "[render][transitions]") {
    ShowSpec s;
    s.effects.push_back(Eff("M", 0, "On", 0, 1000, Pal("#FF0000"), "T_TEXTCTRL_Fadeout=0.5"));
    Rendered r = Render(s);
    REQUIRE(r.rendered);
    for (unsigned f = 0; f <= 9; ++f) CHECK(r.rgb(f, 0).r == 255);
    int prev = 256;
    for (unsigned f = 10; f < 20; ++f) {
        CHECK(r.rgb(f, 0).r < prev);
        prev = r.rgb(f, 0).r;
    }
    CHECK(r.rgb(19, 0) == Rgb{ 0, 0, 0 });
}

TEST_CASE("Fade in and out together rise and fall", "[render][transitions]") {
    ShowSpec s;
    s.effects.push_back(Eff("M", 0, "On", 0, 1000, Pal("#FF0000"), "T_TEXTCTRL_Fadein=0.5,T_TEXTCTRL_Fadeout=0.5"));
    Rendered r = Render(s);
    REQUIRE(r.rendered);
    int peak = 0;
    unsigned peakFrame = 0;
    for (unsigned f = 0; f < 20; ++f) {
        if (r.rgb(f, 0).r > peak) {
            peak = r.rgb(f, 0).r;
            peakFrame = f;
        }
    }
    CHECK(peak == 255);
    CHECK(peakFrame == 9);
    CHECK(r.rgb(0, 0).r < r.rgb(5, 0).r);
    CHECK(r.rgb(19, 0).r < r.rgb(14, 0).r);
}

TEST_CASE("Fade shorter than one frame is a no-op", "[render][transitions]") {
    ShowSpec s;
    s.effects.push_back(Eff("M", 0, "On", 0, 1000, Pal("#FF0000"), "T_TEXTCTRL_Fadein=0.01,T_TEXTCTRL_Fadeout=0.01"));
    Rendered r = Render(s);
    REQUIRE(r.rendered);
    for (unsigned f = 0; f < 20; ++f) CHECK(r.rgb(f, 0) == Rgb{ 255, 0, 0 });
}

TEST_CASE("Fade longer than the effect never reaches full brightness", "[render][transitions]") {
    ShowSpec s;
    s.effects.push_back(Eff("M", 0, "On", 0, 500, Pal("#FF0000"), "T_TEXTCTRL_Fadein=2"));
    Rendered r = Render(s);
    REQUIRE(r.rendered);
    for (unsigned f = 0; f < 10; ++f) {
        CHECK(r.rgb(f, 0).r < 255);
        CHECK(r.rgb(f, 0).r > 0);
    }
}

TEST_CASE("In transitions start partly covered and finish fully visible", "[render][transitions]") {
    for (const char* type : { "Wipe", "Clock", "From Middle", "Square Explode", "Circle Explode", "Blinds", "Blend", "Slide Checks", "Slide Bars", "Dissolve" }) {
        INFO(type);
        Rendered r = Transition("in", type);
        auto lit = LitPerFrame(r, 64);
        CHECK(lit[0] < 64);
        for (size_t f = 10; f < lit.size(); ++f) CHECK(lit[f] == 64);
        CHECK(lit[9] == 64);
    }
}

TEST_CASE("In transitions reveal more of the effect as they progress", "[render][transitions]") {
    for (const char* type : { "Wipe", "Clock", "From Middle", "Square Explode", "Circle Explode", "Blinds", "Blend", "Slide Checks", "Slide Bars", "Dissolve" }) {
        INFO(type);
        auto lit = LitPerFrame(Transition("in", type), 64);
        for (size_t f = 1; f < lit.size(); ++f) {
            INFO("frame " << f);
            CHECK(lit[f] >= lit[f - 1]);
        }
        CHECK(lit[8] > lit[0]);
    }
}

TEST_CASE("Out transitions remove more of the effect as they progress", "[render][transitions]") {
    for (const char* type : { "Wipe", "Clock", "Dissolve" }) {
        INFO(type);
        auto lit = LitPerFrame(Transition("out", type), 64);
        for (size_t f = 0; f <= 9; ++f) CHECK(lit[f] == 64);
        for (size_t f = 11; f < lit.size(); ++f) {
            INFO("frame " << f);
            CHECK(lit[f] <= lit[f - 1]);
        }
        CHECK(lit[19] < 20);
    }
}

TEST_CASE("Fade out transition applies brightness not a mask", "[render][transitions]") {
    auto lit = LitPerFrame(Transition("out", "Fade"), 64);
    for (size_t f = 0; f < 19; ++f) CHECK(lit[f] == 64);
    CHECK(lit[19] == 0);
}

TEST_CASE("Transition mask lets the layer below show through", "[render][transitions]") {
    Rendered r = Transition("in", "Wipe", "", kMatrix8, true);
    for (unsigned f = 0; f < 20; ++f) {
        int red = 0, blue = 0, other = 0;
        for (unsigned n = 0; n < 64; ++n) {
            auto c = r.rgb(f, n);
            if (c == Rgb{ 255, 0, 0 }) ++red;
            else if (c == Rgb{ 0, 0, 255 }) ++blue;
            else ++other;
        }
        INFO("frame " << f);
        CHECK(red + blue == 64);
        CHECK(other == 0);
        if (f >= 10) CHECK(red == 64);
    }
    int red0 = 0;
    for (unsigned n = 0; n < 64; ++n) {
        if (r.rgb(0, n) == Rgb{ 255, 0, 0 }) ++red0;
    }
    CHECK(red0 < 64);
    CHECK(red0 > 0);
}

TEST_CASE("Reversing a transition changes which pixels are revealed first", "[render][transitions]") {
    Rendered fwd = Transition("in", "Wipe");
    Rendered rev = Transition("in", "Wipe", ",T_CHECKBOX_In_Transition_Reverse=1");
    CHECK(fwd.data[2] != rev.data[2]);
    auto a = LitPerFrame(fwd, 64);
    auto b = LitPerFrame(rev, 64);
    CHECK(b[9] == 64);
    CHECK(b[0] < 64);
    CHECK(a[2] > 0);
    CHECK(b[2] > 0);
}

TEST_CASE("Transition adjust changes the pattern", "[render][transitions]") {
    Rendered a = Transition("in", "Wipe", ",T_SLIDER_In_Transition_Adjust=10");
    Rendered b = Transition("in", "Wipe", ",T_SLIDER_In_Transition_Adjust=90");
    CHECK(a.data[3] != b.data[3]);
}

TEST_CASE("Dissolve transition is deterministic", "[render][transitions]") {
    Rendered a = Transition("in", "Dissolve");
    Rendered b = Transition("in", "Dissolve");
    CHECK(a.data == b.data);
}

TEST_CASE("Unknown transition type renders the effect without a mask", "[render][transitions]") {
    auto lit = LitPerFrame(Transition("in", "No Such Transition"), 64);
    for (int n : lit) CHECK(n == 64);
}

TEST_CASE("Transitions on tiny buffers do not crash", "[render][transitions]") {
    for (const std::string& model : { Matrix("M", 1, 1, 1), Matrix("M", 1, 1, 6), Matrix("M", 1, 6, 1), Matrix("M", 1, 3, 5), SingleLine("M", 1, 1), SingleLine("M", 1, 9) }) {
        for (const char* type : { "Fade", "Wipe", "Clock", "From Middle", "Square Explode", "Circle Explode", "Blinds", "Blend", "Slide Checks", "Slide Bars", "Dissolve" }) {
            INFO(model << " " << type);
            Rendered in = Transition("in", type, "", model);
            Rendered out = Transition("out", type, "", model);
            CHECK(in.frames == 20);
            CHECK(out.frames == 20);
        }
    }
}

TEST_CASE("Transition end state is full colour for every in transition", "[render][transitions]") {
    for (const char* type : { "Fade", "Wipe", "Clock", "Blend", "Dissolve" }) {
        INFO(type);
        Rendered r = Transition("in", type);
        for (unsigned n = 0; n < 64; ++n) {
            CHECK(r.rgb(15, n) == Rgb{ 255, 0, 0 });
        }
    }
}
