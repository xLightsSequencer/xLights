#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstdlib>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "effects/EffectManager.h"
#include "effects/RenderableEffect.h"
#include "models/Model.h"
#include "models/ModelManager.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/RenderBuffer.h"
#include "utils/UtilClasses.h"

namespace {

const char* kRedBlue = "C_BUTTON_Palette1=#FF0000,C_CHECKBOX_Palette1=1,C_BUTTON_Palette2=#0000FF,C_CHECKBOX_Palette2=1";
const char* kRed = "C_BUTTON_Palette1=#FF0000,C_CHECKBOX_Palette1=1";

using Pixels = std::vector<xlColor>;

struct Frames {
    int w = 0;
    int h = 0;
    std::vector<Pixels> f;

    const xlColor& at(size_t frame, int x, int y) const { return f[frame][(size_t)y * w + x]; }
    int lit(size_t frame) const {
        int n = 0;
        for (const auto& c : f[frame]) {
            if (c.alpha != 0 && (c.red || c.green || c.blue)) ++n;
        }
        return n;
    }
    bool operator==(const Frames& o) const { return w == o.w && h == o.h && f == o.f; }
};

bool SameRgba(const xlColor& a, const xlColor& b) {
    return a.red == b.red && a.green == b.green && a.blue == b.blue && a.alpha == b.alpha;
}

// A windowless show with one model. The buffer is driven directly (Clear, SetState, Render)
// the same way RenderEngine drives it, without any pixel-buffer layering or GPU work.
struct RenderFixture {
    HeadlessRenderContext ctx;
    std::unique_ptr<Model> model;
    Element* element = nullptr;
    EffectLayer* layer = nullptr;

    RenderFixture() {
        model.reset(ctx.AllModels.CreateDefaultModel("Single Line"));
        element = ctx._sequenceElements.AddElement("Fixture", "model", true, false, true, false, false);
        layer = element->AddEffectLayer();
    }

    Frames Render(const std::string& name, const std::string& settings, const std::string& palette,
                  int w, int h, const std::vector<int>& periods, int endMs = 1000, bool persistState = true,
                  const xlColor* prefill = nullptr) {
        Effect* e = layer->AddEffect(0, name, settings, palette, 0, endMs, 0, false);
        REQUIRE(e != nullptr);
        RenderableEffect* fx = ctx.effectManager.GetEffect(name);
        REQUIRE(fx != nullptr);

        RenderBuffer rb(&ctx, nullptr, model.get());
        rb.InitBuffer(h, w, "None");
        xlColorVector colors;
        xlColorCurveVector curves;
        e->CopyPalette(colors, curves);
        if (colors.empty()) {
            colors.push_back(xlWHITE);
            curves.push_back(ColorCurve());
        }
        rb.SetPalette(colors, curves);
        rb.SetFrameTimeInMs(50);
        rb.SetEffectDuration(0, endMs);

        SettingsMap renderSettings;
        e->CopySettingsMap(renderSettings, true);

        Frames out;
        out.w = w;
        out.h = h;
        bool first = true;
        for (int p : periods) {
            rb.Clear();
            if (prefill != nullptr && w * h > 0) {
                std::fill(rb.GetPixels(), rb.GetPixels() + (size_t)w * h, *prefill);
            }
            rb.SetState(p, first || !persistState);
            first = false;
            fx->Render(e, renderSettings, rb);
            Pixels px;
            if (w > 0 && h > 0) {
                px.assign(rb.GetPixels(), rb.GetPixels() + (size_t)w * h);
            }
            out.f.push_back(std::move(px));
        }
        return out;
    }
};

std::vector<int> Range(int from, int to) {
    std::vector<int> r;
    for (int i = from; i <= to; ++i) r.push_back(i);
    return r;
}

// Effects that render on the CPU from settings alone (no media, audio, timing tracks, fonts or GPU).
const std::vector<std::string>& CpuEffects() {
    static const std::vector<std::string> v = {
        "Bars", "Butterfly", "Candle", "Circles", "Color Wash", "Curtain", "Fan", "Fill", "Fire", "Fireworks",
        "Galaxy", "Garlands", "Kaleidoscope", "Life", "Lightning", "Lines", "Marquee", "Meteors", "Morph", "On", "Off",
        "Pinwheel", "Plasma", "Ripple", "Shimmer", "Shockwave", "SingleStrand", "Snowflakes", "Snowstorm", "Spirals",
        "Spirograph", "Strobe", "Tendril", "Tree", "Twinkle", "Wave"
    };
    return v;
}

} // namespace

TEST_CASE("On fills every pixel with the first palette colour", "[effects][render]") {
    RenderFixture f;
    Frames fr = f.Render("On", "", kRedBlue, 6, 4, { 0, 5, 10, 19 });
    for (size_t i = 0; i < fr.f.size(); ++i) {
        for (int y = 0; y < fr.h; ++y) {
            for (int x = 0; x < fr.w; ++x) {
                INFO("frame " << i << " pixel " << x << "," << y);
                CHECK(SameRgba(fr.at(i, x, y), xlColor(255, 0, 0, 255)));
            }
        }
    }
}

TEST_CASE("On uses a different single colour when the palette changes", "[effects][render]") {
    RenderFixture f;
    Frames fr = f.Render("On", "", "C_BUTTON_Palette1=#12AB34,C_CHECKBOX_Palette1=1", 3, 3, { 0 });
    CHECK(SameRgba(fr.at(0, 1, 1), xlColor(0x12, 0xAB, 0x34, 255)));
    CHECK(fr.lit(0) == 9);
}

TEST_CASE("On with equal start and end intensity scales brightness", "[effects][render]") {
    RenderFixture f;
    Frames fr = f.Render("On", "E_TEXTCTRL_Eff_On_Start=50,E_TEXTCTRL_Eff_On_End=50", kRed, 4, 4, { 0, 10 });
    for (size_t i = 0; i < fr.f.size(); ++i) {
        const xlColor& c = fr.at(i, 2, 2);
        CHECK(std::abs((int)c.red - 128) <= 1);
        CHECK(c.green == 0);
        CHECK(c.blue == 0);
    }
}

TEST_CASE("On with zero intensity is black", "[effects][render]") {
    RenderFixture f;
    Frames fr = f.Render("On", "E_TEXTCTRL_Eff_On_Start=0,E_TEXTCTRL_Eff_On_End=0", kRed, 4, 4, { 3 });
    CHECK(fr.lit(0) == 0);
}

TEST_CASE("On ramps brightness up across the effect", "[effects][render]") {
    RenderFixture f;
    Frames fr = f.Render("On", "E_TEXTCTRL_Eff_On_Start=0,E_TEXTCTRL_Eff_On_End=100", kRed, 4, 4, Range(0, 19));
    int prev = -1;
    for (size_t i = 0; i < fr.f.size(); ++i) {
        const xlColor& c = fr.at(i, 1, 1);
        CHECK(c.red >= prev);
        CHECK(c.green == 0);
        prev = c.red;
    }
    CHECK(fr.at(0, 0, 0).red == 0);
    CHECK(fr.at(19, 0, 0).red == 255);
    CHECK(fr.at(10, 0, 0).red > 90);
    CHECK(fr.at(10, 0, 0).red < 170);
}

TEST_CASE("On ramps brightness down across the effect", "[effects][render]") {
    RenderFixture f;
    Frames fr = f.Render("On", "E_TEXTCTRL_Eff_On_Start=100,E_TEXTCTRL_Eff_On_End=0", kRed, 4, 4, Range(0, 19));
    int prev = 256;
    for (size_t i = 0; i < fr.f.size(); ++i) {
        CHECK(fr.at(i, 1, 1).red <= prev);
        prev = fr.at(i, 1, 1).red;
    }
    CHECK(fr.at(0, 0, 0).red == 255);
    CHECK(fr.at(19, 0, 0).red == 0);
}

TEST_CASE("On transparency sets the alpha channel", "[effects][render]") {
    RenderFixture f;
    Frames fr = f.Render("On", "E_SLIDER_On_Transparency=50", kRed, 4, 4, { 0, 10 });
    for (size_t i = 0; i < fr.f.size(); ++i) {
        const xlColor& c = fr.at(i, 1, 1);
        CHECK(c.red == 255);
        CHECK(std::abs((int)c.alpha - 128) <= 1);
    }
    Frames full = f.Render("On", "E_SLIDER_On_Transparency=100", kRed, 4, 4, { 0 });
    CHECK(full.at(0, 0, 0).alpha == 0);
}

TEST_CASE("On shimmer alternates between the first two colours", "[effects][render]") {
    RenderFixture f;
    Frames fr = f.Render("On", "E_CHECKBOX_On_Shimmer=1", kRedBlue, 4, 4, { 0, 1, 2, 3 });
    CHECK(SameRgba(fr.at(0, 1, 1), xlColor(255, 0, 0, 255)));
    CHECK(SameRgba(fr.at(1, 1, 1), xlColor(0, 0, 255, 255)));
    CHECK(SameRgba(fr.at(2, 1, 1), xlColor(255, 0, 0, 255)));
    CHECK(SameRgba(fr.at(3, 1, 1), xlColor(0, 0, 255, 255)));
}

TEST_CASE("On shimmer with a single colour is dark on alternate frames", "[effects][render]") {
    RenderFixture f;
    Frames fr = f.Render("On", "E_CHECKBOX_On_Shimmer=1", kRed, 4, 4, { 0, 1, 2, 3 });
    CHECK(fr.lit(0) == 16);
    CHECK(fr.lit(1) == 0);
    CHECK(fr.lit(2) == 16);
    CHECK(fr.lit(3) == 0);
}

TEST_CASE("Off style Black paints opaque black", "[effects][render]") {
    RenderFixture f;
    const xlColor white = xlWHITE;
    Frames fr = f.Render("Off", "E_CHOICE_Off_Style=Black", kRed, 4, 4, { 0 }, 1000, true, &white);
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
            CHECK(SameRgba(fr.at(0, x, y), xlColor(0, 0, 0, 255)));
        }
    }
}

TEST_CASE("Off style Transparent leaves existing pixels untouched", "[effects][render]") {
    RenderFixture f;
    const xlColor existing(10, 20, 30, 255);
    Frames fr = f.Render("Off", "E_CHOICE_Off_Style=Transparent", kRed, 4, 4, { 0, 1 }, 1000, true, &existing);
    for (size_t i = 0; i < fr.f.size(); ++i) {
        for (int y = 0; y < 4; ++y) {
            for (int x = 0; x < 4; ++x) {
                CHECK(SameRgba(fr.at(i, x, y), existing));
            }
        }
    }
}

TEST_CASE("Color Wash is spatially uniform without fades", "[effects][render]") {
    RenderFixture f;
    Frames fr = f.Render("Color Wash", "", kRedBlue, 6, 4, { 0, 5, 10, 15, 19 });
    for (size_t i = 0; i < fr.f.size(); ++i) {
        for (int y = 0; y < 4; ++y) {
            for (int x = 0; x < 6; ++x) {
                CHECK(SameRgba(fr.at(i, x, y), fr.at(i, 0, 0)));
            }
        }
    }
}

TEST_CASE("Color Wash moves from the first colour to the last over the effect", "[effects][render]") {
    RenderFixture f;
    Frames fr = f.Render("Color Wash", "", kRedBlue, 4, 4, Range(0, 19));
    CHECK(SameRgba(fr.at(0, 0, 0), xlColor(255, 0, 0, 255)));
    CHECK(SameRgba(fr.at(19, 0, 0), xlColor(0, 0, 255, 255)));
    int prevRed = 256;
    int prevBlue = -1;
    for (size_t i = 0; i < fr.f.size(); ++i) {
        const xlColor& c = fr.at(i, 0, 0);
        CHECK(c.red <= prevRed);
        CHECK(c.blue >= prevBlue);
        CHECK(c.green == 0);
        prevRed = c.red;
        prevBlue = c.blue;
    }
    const xlColor& mid = fr.at(10, 0, 0);
    CHECK(mid.red > 60);
    CHECK(mid.blue > 60);
}

TEST_CASE("Color Wash with a single colour stays that colour", "[effects][render]") {
    RenderFixture f;
    Frames fr = f.Render("Color Wash", "", kRed, 4, 4, { 0, 7, 19 });
    for (size_t i = 0; i < fr.f.size(); ++i) {
        CHECK(SameRgba(fr.at(i, 2, 2), xlColor(255, 0, 0, 255)));
    }
}

TEST_CASE("Bars left direction draws vertical stripes", "[effects][render]") {
    RenderFixture f;
    Frames fr = f.Render("Bars", "E_CHOICE_Bars_Direction=Left,E_SLIDER_Bars_BarCount=2", kRedBlue, 6, 4, { 0 });
    const xlColor red(255, 0, 0, 255);
    const xlColor blue(0, 0, 255, 255);
    for (int y = 0; y < 4; ++y) {
        CHECK(SameRgba(fr.at(0, 0, y), red));
        CHECK(SameRgba(fr.at(0, 1, y), red));
        CHECK(SameRgba(fr.at(0, 2, y), blue));
        CHECK(SameRgba(fr.at(0, 3, y), blue));
        CHECK(SameRgba(fr.at(0, 4, y), red));
        CHECK(SameRgba(fr.at(0, 5, y), red));
    }
}

TEST_CASE("Bars only use palette colours", "[effects][render]") {
    RenderFixture f;
    Frames fr = f.Render("Bars", "E_CHOICE_Bars_Direction=Left,E_SLIDER_Bars_BarCount=3", kRedBlue, 12, 5, Range(0, 19));
    for (size_t i = 0; i < fr.f.size(); ++i) {
        for (const auto& c : fr.f[i]) {
            CHECK(SameRgba(c, xlColor(255, 0, 0, 255)) == !SameRgba(c, xlColor(0, 0, 255, 255)));
        }
    }
}

TEST_CASE("Bars vertical direction draws rows of uniform colour", "[effects][render]") {
    RenderFixture f;
    Frames fr = f.Render("Bars", "E_CHOICE_Bars_Direction=up,E_SLIDER_Bars_BarCount=1", kRedBlue, 6, 8, { 0, 4, 10 });
    for (size_t i = 0; i < fr.f.size(); ++i) {
        for (int y = 0; y < 8; ++y) {
            for (int x = 1; x < 6; ++x) {
                CHECK(SameRgba(fr.at(i, x, y), fr.at(i, 0, y)));
            }
        }
    }
}

TEST_CASE("Fill produces horizontal bands that grade through the palette", "[effects][render]") {
    RenderFixture f;
    Frames fr = f.Render("Fill", "", kRedBlue, 6, 4, { 0, 10 });
    for (size_t i = 0; i < fr.f.size(); ++i) {
        for (int y = 0; y < 4; ++y) {
            for (int x = 1; x < 6; ++x) {
                CHECK(SameRgba(fr.at(i, x, y), fr.at(i, 0, y)));
            }
        }
        CHECK(SameRgba(fr.at(i, 0, 0), xlColor(255, 0, 0, 255)));
        CHECK(SameRgba(fr.at(i, 0, 3), xlColor(0, 0, 255, 255)));
        CHECK(fr.at(i, 0, 1).blue > fr.at(i, 0, 0).blue);
        CHECK(fr.at(i, 0, 2).blue > fr.at(i, 0, 1).blue);
    }
}

TEST_CASE("Marquee lights the border and leaves the middle clear", "[effects][render]") {
    RenderFixture f;
    Frames fr = f.Render("Marquee", "E_SLIDER_Marquee_Band_Size=3,E_SLIDER_Marquee_Skip_Size=0,E_SLIDER_Marquee_Thickness=1", kRedBlue, 6, 4, { 0 });
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 6; ++x) {
            const bool border = x == 0 || y == 0 || x == 5 || y == 3;
            INFO("pixel " << x << "," << y);
            if (border) {
                CHECK(fr.at(0, x, y).alpha != 0);
            } else {
                CHECK(fr.at(0, x, y).alpha == 0);
            }
        }
    }
}

TEST_CASE("Marquee only uses palette colours", "[effects][render]") {
    RenderFixture f;
    Frames fr = f.Render("Marquee", "", kRedBlue, 8, 8, Range(0, 9));
    for (size_t i = 0; i < fr.f.size(); ++i) {
        for (const auto& c : fr.f[i]) {
            const bool ok = SameRgba(c, xlColor(255, 0, 0, 255)) || SameRgba(c, xlColor(0, 0, 255, 255)) || c.alpha == 0;
            CHECK(ok);
        }
    }
}

TEST_CASE("Strobe only uses palette colours and lights something", "[effects][render]") {
    RenderFixture f;
    Frames fr = f.Render("Strobe", "", kRedBlue, 8, 8, Range(0, 9));
    int total = 0;
    for (size_t i = 0; i < fr.f.size(); ++i) {
        total += fr.lit(i);
        for (const auto& c : fr.f[i]) {
            const bool ok = SameRgba(c, xlColor(255, 0, 0, 255)) || SameRgba(c, xlColor(0, 0, 255, 255)) || c.alpha == 0;
            CHECK(ok);
        }
    }
    CHECK(total > 0);
}

TEST_CASE("SingleStrand chase starts at the first pixel and travels along the strand", "[effects][render]") {
    RenderFixture f;
    Frames fr = f.Render("SingleStrand", "", kRedBlue, 6, 4, { 0, 10, 19 });
    auto litColumns = [&](size_t i) {
        std::set<int> cols;
        for (int y = 0; y < 4; ++y) {
            for (int x = 0; x < 6; ++x) {
                const xlColor& c = fr.at(i, x, y);
                if (c.alpha != 0 && (c.red || c.green || c.blue)) cols.insert(x);
            }
        }
        return cols;
    };
    auto c0 = litColumns(0);
    auto c1 = litColumns(1);
    auto c2 = litColumns(2);
    REQUIRE(c0.size() == 1);
    REQUIRE(c1.size() == 1);
    REQUIRE(c2.size() == 1);
    CHECK(*c0.begin() == 0);
    CHECK(*c1.begin() > *c0.begin());
    CHECK(*c2.begin() > *c1.begin());
    CHECK(*c2.begin() == 5);
}

TEST_CASE("Rendering is deterministic across independent runs", "[effects][render][determinism]") {
    for (const auto& name : CpuEffects()) {
        INFO("effect " << name);
        RenderFixture a;
        RenderFixture b;
        Frames fa = a.Render(name, "", kRedBlue, 12, 10, Range(0, 9));
        Frames fb = b.Render(name, "", kRedBlue, 12, 10, Range(0, 9));
        CHECK(fa == fb);
    }
}

TEST_CASE("Rendering the same effect twice in one fixture is identical", "[effects][render][determinism]") {
    RenderFixture f;
    for (const auto& name : { "Twinkle", "Strobe", "Snowflakes", "Meteors", "Fireworks", "Plasma", "Life" }) {
        INFO("effect " << name);
        Frames a = f.Render(name, "", kRedBlue, 10, 10, Range(0, 7));
        Frames b = f.Render(name, "", kRedBlue, 10, 10, Range(0, 7));
        CHECK(a == b);
    }
}

TEST_CASE("Different palettes produce different output", "[effects][render]") {
    RenderFixture f;
    for (const auto& name : { "On", "Color Wash", "Bars", "Fill", "Marquee" }) {
        INFO("effect " << name);
        Frames red = f.Render(name, "", kRed, 8, 8, { 3 });
        Frames green = f.Render(name, "", "C_BUTTON_Palette1=#00FF00,C_CHECKBOX_Palette1=1", 8, 8, { 3 });
        CHECK_FALSE(red == green);
    }
}

TEST_CASE("Rendering a 1x1 buffer does not crash", "[effects][render][edge]") {
    for (const auto& name : CpuEffects()) {
        INFO("effect " << name);
        RenderFixture f;
        Frames fr = f.Render(name, "", kRedBlue, 1, 1, Range(0, 3));
        CHECK(fr.f.size() == 4);
    }
}

TEST_CASE("Rendering a single row or column does not crash", "[effects][render][edge]") {
    for (const auto& name : CpuEffects()) {
        INFO("effect " << name);
        RenderFixture f;
        CHECK(f.Render(name, "", kRedBlue, 7, 1, Range(0, 2)).f.size() == 3);
        CHECK(f.Render(name, "", kRedBlue, 1, 7, Range(0, 2)).f.size() == 3);
    }
}

TEST_CASE("Rendering an empty buffer does not crash", "[effects][render][edge]") {
    for (const auto& name : CpuEffects()) {
        INFO("effect " << name);
        RenderFixture f;
        Frames fr = f.Render(name, "", kRedBlue, 0, 0, Range(0, 2));
        CHECK(fr.f.size() == 3);
    }
}

TEST_CASE("Rendering with a single colour palette does not crash", "[effects][render][edge]") {
    for (const auto& name : CpuEffects()) {
        INFO("effect " << name);
        RenderFixture f;
        Frames fr = f.Render(name, "", kRed, 8, 8, Range(0, 4));
        CHECK(fr.f.size() == 5);
    }
}

TEST_CASE("Rendering a very short effect does not crash", "[effects][render][edge]") {
    for (const auto& name : CpuEffects()) {
        INFO("effect " << name);
        RenderFixture f;
        Frames fr = f.Render(name, "", kRedBlue, 8, 8, { 0 }, 50);
        CHECK(fr.f.size() == 1);
    }
}

TEST_CASE("Pure effects render the same frame whether or not earlier frames were rendered", "[effects][render][parallel]") {
    RenderFixture probe;
    int checked = 0;
    for (const auto& name : CpuEffects()) {
        RenderableEffect* fx = probe.ctx.effectManager.GetEffect(name);
        REQUIRE(fx != nullptr);
        SettingsMap empty;
        if (fx->GetEffectiveFrameParallelism(empty) != RenderableEffect::FrameParallelism::Pure) {
            continue;
        }
        ++checked;
        INFO("effect " << name);
        RenderFixture seq;
        RenderFixture direct;
        Frames inOrder = seq.Render(name, "", kRedBlue, 10, 10, Range(0, 12));
        Frames jumped = direct.Render(name, "", kRedBlue, 10, 10, { 12 });
        CHECK(inOrder.f.back() == jumped.f.back());
    }
    CHECK(checked >= 3);
}

TEST_CASE("Rendering the same frame repeatedly gives the same output for pure effects", "[effects][render][parallel]") {
    RenderFixture probe;
    for (const auto& name : CpuEffects()) {
        SettingsMap empty;
        if (probe.ctx.effectManager.GetEffect(name)->GetEffectiveFrameParallelism(empty) != RenderableEffect::FrameParallelism::Pure) {
            continue;
        }
        INFO("effect " << name);
        RenderFixture f;
        Frames fr = f.Render(name, "", kRedBlue, 8, 8, { 5, 5, 5 }, 1000, false);
        CHECK(fr.f[0] == fr.f[1]);
        CHECK(fr.f[1] == fr.f[2]);
    }
}

TEST_CASE("Value curve free settings parse the same whether or not an E_ prefix is stripped", "[effects][render]") {
    RenderFixture f;
    Effect* e = f.layer->AddEffect(0, "On", "E_TEXTCTRL_Eff_On_Start=40,B_CHECKBOX_OverlayBkg=1,T_CHECKBOX_Canvas=1", kRed, 2000, 3000, 0, false);
    REQUIRE(e != nullptr);
    SettingsMap rs;
    e->CopySettingsMap(rs, true);
    CHECK(rs.GetInt("TEXTCTRL_Eff_On_Start", 100) == 40);
    CHECK(rs.GetBool("CHECKBOX_OverlayBkg", false));
    CHECK(rs.GetBool("CHECKBOX_Canvas", false));
}
