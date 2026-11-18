// SingleStrand effect: the Chase and Skips modes and the "FX" mode (the WLED/WS2812FX port in FX.cpp).
//
// The FX port writes every buffer column as one colour (setPixelColor fills the whole column), animates
// on the effect-relative clock (frame * frame time) and draws from the effect palette or the WLED palettes.
// Most of these checks are behavioural (column-uniform output, palette-only colours, motion, option A vs B)
// so they survive the -ffast-math differences between build configurations.

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <map>
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
#include "render/SequenceElements.h"
#include "utils/UtilClasses.h"

namespace {

const char* kRedBlue = "C_BUTTON_Palette1=#FF0000,C_CHECKBOX_Palette1=1,C_BUTTON_Palette2=#0000FF,C_CHECKBOX_Palette2=1";
const char* kRGB = "C_BUTTON_Palette1=#FF0000,C_CHECKBOX_Palette1=1,C_BUTTON_Palette2=#00FF00,C_CHECKBOX_Palette2=1,"
                   "C_BUTTON_Palette3=#0000FF,C_CHECKBOX_Palette3=1";
const char* kRed = "C_BUTTON_Palette1=#FF0000,C_CHECKBOX_Palette1=1";

using Pixels = std::vector<xlColor>;

uint32_t Rgb(const xlColor& c) {
    return ((uint32_t)c.red << 16) | ((uint32_t)c.green << 8) | c.blue;
}

bool IsLit(const xlColor& c) {
    return c.alpha != 0 && (c.red || c.green || c.blue);
}

struct Frames {
    int w = 0;
    int h = 0;
    std::vector<Pixels> f;

    const xlColor& at(size_t frame, int x, int y = 0) const { return f[frame][(size_t)y * w + x]; }
    int lit(size_t frame) const {
        int n = 0;
        for (const auto& c : f[frame]) {
            if (IsLit(c)) ++n;
        }
        return n;
    }
    std::set<int> litColumns(size_t frame) const {
        std::set<int> cols;
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                if (IsLit(at(frame, x, y))) cols.insert(x);
            }
        }
        return cols;
    }
    // RGB values present in a frame (alpha ignored, black included).
    std::set<uint32_t> colours(size_t frame) const {
        std::set<uint32_t> s;
        for (const auto& c : f[frame]) s.insert(Rgb(c));
        return s;
    }
    bool columnUniform(size_t frame) const {
        for (int y = 1; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                if (Rgb(at(frame, x, y)) != Rgb(at(frame, x, 0))) return false;
            }
        }
        return true;
    }
    // xlColor::operator== ignores alpha, so frames are compared channel by channel including alpha
    static bool SamePixels(const Pixels& a, const Pixels& b) {
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); ++i) {
            if (Rgb(a[i]) != Rgb(b[i]) || a[i].alpha != b[i].alpha) return false;
        }
        return true;
    }
    bool sameFrame(size_t a, size_t b) const { return SamePixels(f[a], f[b]); }
    bool operator==(const Frames& o) const {
        if (w != o.w || h != o.h || f.size() != o.f.size()) return false;
        for (size_t i = 0; i < f.size(); ++i) {
            if (!SamePixels(f[i], o.f[i])) return false;
        }
        return true;
    }
};

struct RenderOpts {
    int frameMs = 50;
    int endMs = 1000;
    bool allowAlpha = false;
    bool keepEmptyPalette = false;
};

// A windowless show with one model; the buffer is driven directly (Clear, SetState, Render) the same way
// RenderEngine drives it.
struct Fixture {
    HeadlessRenderContext ctx;
    std::unique_ptr<Model> model;
    Element* element = nullptr;
    EffectLayer* layer = nullptr;
    Effect* last = nullptr;

    Fixture() {
        model.reset(ctx.AllModels.CreateDefaultModel("Single Line"));
        element = ctx._sequenceElements.AddElement("Fixture", "model", true, false, true, false, false);
        layer = element->AddEffectLayer();
    }

    RenderableEffect* effect() { return ctx.effectManager.GetEffect("SingleStrand"); }

    Frames Render(const std::string& settings, const std::string& palette, int w, int h, const std::vector<int>& periods,
                  const RenderOpts& o = {}) {
        Effect* e = layer->AddEffect(0, "SingleStrand", settings, palette, 0, o.endMs, 0, false);
        REQUIRE(e != nullptr);
        last = e;
        RenderableEffect* fx = effect();
        REQUIRE(fx != nullptr);

        RenderBuffer rb(&ctx, nullptr, model.get());
        rb.InitBuffer(h, w, "None");
        xlColorVector colors;
        xlColorCurveVector curves;
        e->CopyPalette(colors, curves);
        if (colors.empty() && !o.keepEmptyPalette) {
            colors.push_back(xlWHITE);
            curves.push_back(ColorCurve());
        }
        rb.SetPalette(colors, curves);
        rb.SetFrameTimeInMs(o.frameMs);
        rb.SetEffectDuration(0, o.endMs);
        rb.SetAllowAlphaChannel(o.allowAlpha);

        SettingsMap renderSettings;
        e->CopySettingsMap(renderSettings, true);

        Frames out;
        out.w = w;
        out.h = h;
        bool first = true;
        for (int p : periods) {
            rb.Clear();
            rb.SetState(p, first);
            first = false;
            fx->Render(e, renderSettings, rb);
            out.f.emplace_back(rb.GetPixels(), rb.GetPixels() + (size_t)w * h);
        }
        return out;
    }
};

std::vector<int> Range(int from, int to) {
    std::vector<int> r;
    for (int i = from; i <= to; ++i) r.push_back(i);
    return r;
}

std::string FX(const std::string& mode, const std::string& extra = "") {
    std::string s = "E_NOTEBOOK_SSEFFECT_TYPE=FX,E_CHOICE_SingleStrand_FX=" + mode;
    if (!extra.empty()) s += "," + extra;
    return s;
}

std::string Chase(const std::string& extra = "") {
    std::string s = "E_NOTEBOOK_SSEFFECT_TYPE=Chase";
    if (!extra.empty()) s += "," + extra;
    return s;
}

std::string Skips(const std::string& extra = "") {
    std::string s = "E_NOTEBOOK_SSEFFECT_TYPE=Skips";
    if (!extra.empty()) s += "," + extra;
    return s;
}

// Mirror a frame horizontally.
Pixels Mirror(const Frames& fr, size_t frame) {
    Pixels p(fr.f[frame].size());
    for (int y = 0; y < fr.h; ++y) {
        for (int x = 0; x < fr.w; ++x) {
            p[(size_t)y * fr.w + x] = fr.at(frame, fr.w - 1 - x, y);
        }
    }
    return p;
}

std::vector<uint32_t> Key(const Pixels& p) {
    std::vector<uint32_t> k;
    k.reserve(p.size());
    for (const auto& c : p) k.push_back(Rgb(c));
    return k;
}

bool RgbEqual(const Pixels& a, const Pixels& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (Rgb(a[i]) != Rgb(b[i])) return false;
    }
    return true;
}

const std::vector<std::string>& FxModes() {
    static const std::vector<std::string> v = [] {
        Fixture f;
        return f.effect()->GetSettingOptions("SingleStrand_FX");
    }();
    return v;
}

const std::vector<std::string>& FxPalettes() {
    static const std::vector<std::string> v = [] {
        Fixture f;
        return f.effect()->GetSettingOptions("SingleStrand_FX_Palette");
    }();
    return v;
}

} // namespace

// ---------------------------------------------------------------------------------------------------------------
// Options and metadata

TEST_CASE("SingleStrand lists every FX mode and FX palette once, sorted", "[effects][singlestrand]") {
    auto modes = FxModes();
    CHECK(modes.size() == 118);
    CHECK(std::is_sorted(modes.begin(), modes.end()));
    CHECK(std::set<std::string>(modes.begin(), modes.end()).size() == modes.size());
    for (const char* m : { "Solid", "Blink", "Fire 2012", "TV Simulator", "Dynamic Smooth", "Sparkle+" }) {
        CHECK(std::find(modes.begin(), modes.end(), m) != modes.end());
    }

    auto pals = FxPalettes();
    CHECK(pals.size() == 71);
    CHECK(std::is_sorted(pals.begin(), pals.end()));
    CHECK(std::set<std::string>(pals.begin(), pals.end()).size() == pals.size());
    for (const char* p : { "Default", "* Random Cycle", "* Colors 1&2", "Candy2", "Red & Blue" }) {
        CHECK(std::find(pals.begin(), pals.end(), p) != pals.end());
    }

    Fixture f;
    CHECK(f.effect()->GetSettingOptions("Chase_Type1").empty());
}

TEST_CASE("SingleStrand only the FX sub-mode carries state between frames", "[effects][singlestrand]") {
    Fixture f;
    auto* e = f.effect();
    SettingsMap fx;
    fx["NOTEBOOK_SSEFFECT_TYPE"] = "FX";
    SettingsMap chase;
    chase["NOTEBOOK_SSEFFECT_TYPE"] = "Chase";
    SettingsMap skips;
    skips["NOTEBOOK_SSEFFECT_TYPE"] = "Skips";
    CHECK(e->GetFrameParallelism(fx) == RenderableEffect::FrameParallelism::Stateful);
    CHECK(e->GetFrameParallelism(chase) == RenderableEffect::FrameParallelism::Pure);
    CHECK(e->GetFrameParallelism(skips) == RenderableEffect::FrameParallelism::Pure);
    CHECK(e->GetFrameParallelism(SettingsMap()) == RenderableEffect::FrameParallelism::Pure);
    CHECK(e->SupportsLinearColorCurves(fx));
    CHECK(e->CanRenderPartialTimeInterval());
}

TEST_CASE("SingleStrand renames only a matching timing track", "[effects][singlestrand]") {
    Fixture f;
    Effect* e = f.layer->AddEffect(0, "SingleStrand", "E_CHOICE_SingleStrand_TimingTrack=Beats", "", 0, 1000, 0, false);
    f.effect()->RenameTimingTrack("Other", "New", e);
    CHECK(e->GetSettings().Get("E_CHOICE_SingleStrand_TimingTrack", "") == "Beats");
    f.effect()->RenameTimingTrack("Beats", "Bars", e);
    CHECK(e->GetSettings().Get("E_CHOICE_SingleStrand_TimingTrack", "") == "Bars");
}

// ---------------------------------------------------------------------------------------------------------------
// FX mode: whole-catalogue sweep

TEST_CASE("SingleStrand FX: every mode renders column-uniform, deterministic, visible output", "[effects][singlestrand]") {
    Fixture a;
    Fixture b;
    // Noise Pal builds its own palette only once its palette-change interval (4 s or more) has passed on the
    // effect clock, which starts at 0, so it is black for the first seconds (covered by its own test below).
    // Fireworks and Rain spawn a spark on a 1-in-(129 - intensity / 2) chance per frame, so a short render can
    // stay dark (their own test below uses full intensity).
    const std::set<std::string> mayBeBlank = { "Noise Pal", "Fireworks", "Rain" };
    // Modes that do not change within the rendered 1.5 s at these settings: Percent at intensity 200 has
    // reached its (empty) target, ICU and Traffic Light step on a longer period.
    const std::set<std::string> slow = { "Percent", "ICU", "Traffic Light" };
    // Modes whose output is a fixed pattern for fixed settings (no animation).
    const std::set<std::string> isStatic = { "Solid", "Solid Pattern", "Solid Pattern Tri", "Spots" };

    const int W = 24, H = 2;
    const auto frames = Range(0, 30);
    for (const auto& mode : FxModes()) {
        INFO("mode " << mode);
        const std::string settings = FX(mode, "E_SLIDER_FX_Speed=200,E_SLIDER_FX_Intensity=200");
        Frames fa = a.Render(settings, kRGB, W, H, frames);
        Frames fb = b.Render(settings, kRGB, W, H, frames);
        CHECK(fa == fb);

        // The effect clock starts at 0 on the first frame, which is inside WLED's minimum show delay, so
        // nothing is drawn yet.
        CHECK(fa.lit(0) == 0);

        bool anyLit = false;
        bool uniform = true;
        bool changes = false;
        for (size_t i = 1; i < fa.f.size(); ++i) {
            uniform = uniform && fa.columnUniform(i);
            anyLit = anyLit || fa.lit(i) > 0;
            if (i > 1 && !fa.sameFrame(i, i - 1)) changes = true;
        }
        CHECK(uniform);
        if (!mayBeBlank.count(mode)) CHECK(anyLit);
        if (isStatic.count(mode)) {
            CHECK_FALSE(changes);
        } else if (!mayBeBlank.count(mode) && !slow.count(mode)) {
            CHECK(changes);
        }
    }
}

TEST_CASE("SingleStrand FX: every mode survives extreme speed and intensity", "[effects][singlestrand]") {
    Fixture f;
    const int W = 20, H = 1;
    const auto frames = Range(0, 16);
    const std::vector<std::pair<int, int>> combos = { { 0, 0 }, { 255, 255 }, { 255, 0 }, { 0, 255 } };
    for (const auto& mode : FxModes()) {
        for (auto [sp, in] : combos) {
            INFO("mode " << mode << " speed " << sp << " intensity " << in);
            Frames fr = f.Render(FX(mode, "E_SLIDER_FX_Speed=" + std::to_string(sp) + ",E_SLIDER_FX_Intensity=" + std::to_string(in)),
                                 kRedBlue, W, H, frames, { 25, 1000 });
            REQUIRE(fr.f.size() == frames.size());
            for (const auto& px : fr.f) {
                for (const auto& c : px) {
                    // Every pixel the FX port writes is opaque; untouched ones stay cleared.
                    CHECK((c.alpha == 255 || (c.alpha == 0 && !IsLit(c))));
                }
            }
        }
    }
}

TEST_CASE("SingleStrand FX: speed changes the animation of moving modes", "[effects][singlestrand]") {
    Fixture f;
    const int W = 24, H = 1;
    const auto frames = Range(0, 20);
    int differ = 0;
    std::vector<std::string> same;
    for (const auto& mode : FxModes()) {
        if (mode == "Solid" || mode == "Solid Pattern Tri") continue;
        Frames slow = f.Render(FX(mode, "E_SLIDER_FX_Speed=10"), kRGB, W, H, frames);
        Frames fast = f.Render(FX(mode, "E_SLIDER_FX_Speed=250"), kRGB, W, H, frames);
        if (slow == fast) {
            same.push_back(mode);
        } else {
            ++differ;
        }
    }
    INFO("speed-insensitive modes: " << [&] { std::string s; for (auto& m : same) s += m + ";"; return s; }());
    // A handful of WLED modes ignore speed (or only use it on a longer time scale); nearly all must react.
    CHECK(differ >= 100);
}

TEST_CASE("SingleStrand FX: palette-colour modes draw only effect palette colours", "[effects][singlestrand]") {
    Fixture f;
    // With the "Default" FX palette these modes take their colours from the effect palette (or black for the
    // empty colour slots).
    const std::set<uint32_t> allowed = { 0xFF0000, 0x00FF00, 0x0000FF, 0x000000 };
    for (const char* mode : { "Solid", "Blink", "Strobe", "Solid Pattern", "Solid Pattern Tri", "Percent", "Theater",
                              "Wipe", "Sweep", "Scan", "Scan Dual", "Chase 2", "Chase 3", "Two Dots",
                              "Tri Wipe" }) {
        INFO("mode " << mode);
        Frames fr = f.Render(FX(mode, "E_SLIDER_FX_Speed=250"), kRGB, 16, 1, Range(0, 20));
        std::set<uint32_t> seen;
        for (size_t i = 0; i < fr.f.size(); ++i) {
            auto c = fr.colours(i);
            seen.insert(c.begin(), c.end());
        }
        bool onlyAllowed = std::all_of(seen.begin(), seen.end(), [&](uint32_t c) { return allowed.count(c) != 0; });
        if (std::string(mode) == "Wipe" || std::string(mode) == "Sweep" || std::string(mode) == "Tri Wipe") {
            // the wipe front pixel is a blend of two palette colours; everything else is a palette colour
            int other = 0;
            for (size_t i = 1; i < fr.f.size(); ++i) {
                int n = 0;
                for (int x = 0; x < fr.w; ++x) {
                    if (!allowed.count(Rgb(fr.at(i, x)))) ++n;
                }
                other = std::max(other, n);
            }
            CHECK(other <= 1);
        } else {
            CHECK(onlyAllowed);
        }
        CHECK(seen.count(0xFF0000));
    }
}

TEST_CASE("SingleStrand FX: rainbow modes draw colours outside the effect palette", "[effects][singlestrand]") {
    Fixture f;
    for (const char* mode : { "Rainbow", "Colorloop", "Blink Rainbow", "Theater Rainbow", "Chase Rainbow", "Pride 2015" }) {
        INFO("mode " << mode);
        Frames fr = f.Render(FX(mode, "E_SLIDER_FX_Speed=200"), kRed, 16, 1, Range(0, 20));
        std::set<uint32_t> seen;
        for (size_t i = 1; i < fr.f.size(); ++i) {
            auto c = fr.colours(i);
            seen.insert(c.begin(), c.end());
        }
        seen.erase(0xFF0000);
        seen.erase(0x000000);
        CHECK(seen.size() >= 3);
    }
}

// ---------------------------------------------------------------------------------------------------------------
// FX mode: specific modes with exact integer behaviour

TEST_CASE("SingleStrand FX Solid fills the strand with the first palette colour", "[effects][singlestrand]") {
    Fixture f;
    Frames fr = f.Render(FX("Solid"), kRedBlue, 8, 3, Range(0, 10));
    for (size_t i = 1; i < fr.f.size(); ++i) {
        INFO("frame " << i);
        CHECK(fr.colours(i) == std::set<uint32_t>{ 0xFF0000 });
        CHECK(fr.lit(i) == 24);
    }
    Frames green = f.Render(FX("Solid"), "C_BUTTON_Palette1=#00FF00,C_CHECKBOX_Palette1=1", 8, 3, Range(0, 3));
    CHECK(green.colours(2) == std::set<uint32_t>{ 0x00FF00 });
}

TEST_CASE("SingleStrand FX with no palette colours draws white", "[effects][singlestrand]") {
    Fixture f;
    RenderOpts o;
    o.keepEmptyPalette = true;
    Frames fr = f.Render(FX("Solid"), "", 6, 1, Range(0, 3), o);
    CHECK(fr.colours(2) == std::set<uint32_t>{ 0xFFFFFF });
}

TEST_CASE("SingleStrand FX Solid Pattern alternates lit and unlit runs sized by speed and intensity", "[effects][singlestrand]") {
    // lit run = 1 + speed, unlit run = 1 + intensity; lit pixels are colour 1, unlit are colour 2
    Fixture f;
    Frames fr = f.Render(FX("Solid Pattern", "E_SLIDER_FX_Speed=1,E_SLIDER_FX_Intensity=2"), kRedBlue, 15, 1, Range(0, 3));
    for (int x = 0; x < 15; ++x) {
        INFO("x " << x);
        bool lit = (x % 5) < 2;
        CHECK(Rgb(fr.at(2, x)) == (lit ? 0xFF0000u : 0x0000FFu));
    }
}

TEST_CASE("SingleStrand FX Solid Pattern Tri cycles three colours in bands sized by intensity", "[effects][singlestrand]") {
    // band size = (intensity >> 5) + 1
    Fixture f;
    Frames fr = f.Render(FX("Solid Pattern Tri", "E_SLIDER_FX_Intensity=64"), kRGB, 18, 1, Range(0, 3));
    const uint32_t cols[3] = { 0xFF0000, 0x00FF00, 0x0000FF };
    for (int x = 0; x < 18; ++x) {
        INFO("x " << x);
        CHECK(Rgb(fr.at(2, x)) == cols[(x / 3) % 3]);
    }
}

TEST_CASE("SingleStrand FX Percent lights the requested share of the strand from the correct end", "[effects][singlestrand]") {
    // speed 255 moves straight to the target; intensity < 100 fills from the start, > 100 from the end
    Fixture f;
    Frames low = f.Render(FX("Percent", "E_SLIDER_FX_Speed=255,E_SLIDER_FX_Intensity=25"), kRedBlue, 20, 1, Range(0, 6));
    Frames high = f.Render(FX("Percent", "E_SLIDER_FX_Speed=255,E_SLIDER_FX_Intensity=175"), kRedBlue, 20, 1, Range(0, 6));
    for (int x = 0; x < 20; ++x) {
        INFO("x " << x);
        CHECK(Rgb(low.at(5, x)) == (x < 5 ? 0xFF0000u : 0x0000FFu));
        CHECK(Rgb(high.at(5, x)) == (x >= 15 ? 0xFF0000u : 0x0000FFu));
    }
}

TEST_CASE("SingleStrand FX Percent grows gradually at low speed", "[effects][singlestrand]") {
    Fixture f;
    Frames fr = f.Render(FX("Percent", "E_SLIDER_FX_Speed=0,E_SLIDER_FX_Intensity=99"), kRedBlue, 20, 1, Range(0, 30));
    auto redCount = [&](size_t i) {
        int n = 0;
        for (int x = 0; x < 20; ++x) n += Rgb(fr.at(i, x)) == 0xFF0000 ? 1 : 0;
        return n;
    };
    int prev = 0;
    for (size_t i = 1; i < fr.f.size(); ++i) {
        INFO("frame " << i);
        CHECK(redCount(i) >= prev);
        CHECK(redCount(i) <= prev + 1);
        prev = redCount(i);
    }
    CHECK(prev >= 15);
}

TEST_CASE("SingleStrand FX Wipe fills from the start of the strand", "[effects][singlestrand]") {
    // speed 255 gives a 750 ms cycle, so the first half (frames 1..7 at 50 ms) is a growing colour-1 prefix
    Fixture f;
    Frames fr = f.Render(FX("Wipe", "E_SLIDER_FX_Speed=255"), kRedBlue, 20, 1, Range(0, 8));
    int prev = -1;
    for (size_t i = 1; i <= 7; ++i) {
        INFO("frame " << i);
        int k = 0;
        while (k < 20 && Rgb(fr.at(i, k)) == 0xFF0000) ++k;
        for (int x = k + 1; x < 20; ++x) CHECK(Rgb(fr.at(i, x)) == 0x0000FF);
        CHECK(k > prev);
        prev = k;
    }
    CHECK(prev >= 17);
}

TEST_CASE("SingleStrand FX Blink alternates between the first two colours", "[effects][singlestrand]") {
    Fixture f;
    Frames fr = f.Render(FX("Blink", "E_SLIDER_FX_Speed=230,E_SLIDER_FX_Intensity=128"), kRedBlue, 6, 1, Range(0, 40));
    int red = 0, blue = 0;
    for (size_t i = 1; i < fr.f.size(); ++i) {
        auto c = fr.colours(i);
        REQUIRE(c.size() == 1);
        if (c.count(0xFF0000)) ++red;
        if (c.count(0x0000FF)) ++blue;
    }
    CHECK(red > 5);
    CHECK(blue > 5);
    CHECK(red + blue == 40);
}

TEST_CASE("SingleStrand FX output is mode dependent", "[effects][singlestrand]") {
    Fixture f;
    auto modes = FxModes();
    std::map<std::vector<uint32_t>, std::string> seen;
    int distinct = 0;
    for (const auto& mode : modes) {
        Frames fr = f.Render(FX(mode), kRGB, 24, 1, Range(0, 12));
        if (seen.emplace(Key(fr.f.back()), mode).second) ++distinct;
    }
    CHECK(distinct >= 100);
}

// ---------------------------------------------------------------------------------------------------------------
// FX palettes

TEST_CASE("SingleStrand FX: every FX palette renders with the Palette mode", "[effects][singlestrand]") {
    Fixture f;
    const auto pals = FxPalettes();
    std::set<std::vector<uint32_t>> outputs;
    for (const auto& pal : pals) {
        INFO("palette " << pal);
        Frames fr = f.Render(FX("Palette", "E_CHOICE_SingleStrand_FX_Palette=" + pal), kRGB, 24, 1, Range(0, 8));
        CHECK(fr.lit(8) > 0);
        outputs.insert(Key(fr.f.back()));
    }
    // "Default", "* Color Gradient" and the effect-colour palettes can coincide; the gradient palettes cannot.
    CHECK(outputs.size() >= pals.size() - 4);
}

TEST_CASE("SingleStrand FX: palette-driven modes cover every palette", "[effects][singlestrand]") {
    Fixture f;
    // Modes with their own default palette, swept over a few palettes each.
    for (const char* mode : { "Fire 2012", "Colorwaves", "Fill Noise", "Noise 1", "Noise 2", "Noise 3", "Noise 4",
                              "Glitter", "Sunrise", "Flow", "Meteor", "Pacifica" }) {
        for (const char* pal : { "Default", "* Random Cycle", "* Color 1", "Lava" }) {
            INFO("mode " << mode << " palette " << pal);
            Frames fr = f.Render(FX(mode, std::string("E_CHOICE_SingleStrand_FX_Palette=") + pal), kRGB, 20, 1, Range(0, 25));
            bool any = false;
            for (size_t i = 1; i < fr.f.size(); ++i) any = any || fr.lit(i) > 0;
            if (std::string(mode) != "Sunrise") CHECK(any);
            CHECK(fr.columnUniform(25));
        }
    }
}

TEST_CASE("SingleStrand FX: single colour palette keeps a palette mode to that colour's hue", "[effects][singlestrand]") {
    Fixture f;
    Frames fr = f.Render(FX("Palette", "E_CHOICE_SingleStrand_FX_Palette=* Color 1"), kRedBlue, 16, 1, Range(0, 6));
    for (int x = 0; x < 16; ++x) {
        const xlColor& c = fr.at(6, x);
        CHECK(c.green == 0);
        CHECK(c.blue == 0);
        CHECK(c.red > 200);
    }
    Frames two = f.Render(FX("Palette", "E_CHOICE_SingleStrand_FX_Palette=* Colors 1&2"), kRedBlue, 16, 1, Range(0, 6));
    bool anyBlue = false;
    for (int x = 0; x < 16; ++x) {
        CHECK(two.at(6, x).green == 0);
        anyBlue = anyBlue || two.at(6, x).blue > 200;
    }
    CHECK(anyBlue);
}

std::string NColours(int n) {
    const char* hex[] = { "#FF0000", "#00FF00", "#0000FF", "#FFFF00", "#00FFFF", "#FF00FF", "#FFFFFF", "#808080" };
    std::string pal;
    for (int i = 1; i <= n; ++i) {
        pal += std::string(i > 1 ? "," : "") + "C_BUTTON_Palette" + std::to_string(i) + "=" + hex[i - 1] + ",C_CHECKBOX_Palette" + std::to_string(i) + "=1";
    }
    return pal;
}

TEST_CASE("SingleStrand FX: the colour gradient palette uses as many effect colours as are set", "[effects][singlestrand]") {
    Fixture f;
    std::set<std::vector<uint32_t>> outs;
    for (int n = 3; n <= 8; ++n) {
        INFO("colours " << n);
        Frames fr = f.Render(FX("Palette", "E_CHOICE_SingleStrand_FX_Palette=* Color Gradient"), NColours(n), 32, 1, Range(0, 4));
        CHECK(fr.lit(4) > 0);
        outs.insert(Key(fr.f.back()));
        // the last colour set shows up somewhere along the strand
        bool found = false;
        for (int x = 0; x < 32 && !found; ++x) {
            const xlColor& c = fr.at(4, x);
            if (n == 3) found = c.blue > 200 && c.red < 60 && c.green < 60;
            if (n == 4) found = c.red > 200 && c.green > 200 && c.blue < 60;
            if (n >= 5) found = true;
        }
        CHECK(found);
    }
    CHECK(outs.size() == 6);
}

TEST_CASE("SingleStrand FX: the colour gradient palette with one or two colours includes colour 1", "[effects][singlestrand]") {
    Fixture f;
    Frames one = f.Render(FX("Palette", "E_CHOICE_SingleStrand_FX_Palette=* Color Gradient"), NColours(1), 32, 1, Range(0, 4));
    CHECK(one.lit(4) > 0);
    Frames two = f.Render(FX("Palette", "E_CHOICE_SingleStrand_FX_Palette=* Color Gradient"), NColours(2), 32, 1, Range(0, 4));
    bool red = false;
    for (int x = 0; x < 32; ++x) red = red || (two.at(4, x).red > 200 && two.at(4, x).green < 60);
    CHECK(red);
}

TEST_CASE("SingleStrand FX: Phased renders the same effect the same way twice", "[effects][singlestrand]") {
    // the phase is per effect, so an earlier render can't shift a later one
    Fixture f;
    for (const char* mode : { "Phased", "Phased Noise" }) {
        INFO("mode " << mode);
        Frames a = f.Render(FX(mode), kRGB, 24, 1, Range(0, 10));
        Frames b = f.Render(FX(mode), kRGB, 24, 1, Range(0, 10));
        CHECK(a == b);
    }
}

TEST_CASE("SingleStrand FX: Noise Pal lights up once its first palette is built", "[effects][singlestrand]") {
    // the palette is first generated after 4 s + speed * 10 ms of effect time
    Fixture f;
    RenderOpts o;
    o.endMs = 6000;
    Frames fr = f.Render(FX("Noise Pal", "E_SLIDER_FX_Speed=0"), kRGB, 16, 1, { 0, 40, 79, 85, 100, 119 }, o);
    CHECK(fr.lit(1) == 0);
    CHECK(fr.lit(2) == 0);
    CHECK(fr.lit(5) > 0);
    // with a WLED palette selected it draws straight away
    Frames lava = f.Render(FX("Noise Pal", "E_CHOICE_SingleStrand_FX_Palette=Lava"), kRGB, 16, 1, Range(0, 5), o);
    CHECK(lava.lit(5) > 0);
}

TEST_CASE("SingleStrand FX: Fireworks and Rain spawn sparks at high intensity", "[effects][singlestrand]") {
    Fixture f;
    for (const char* mode : { "Fireworks", "Rain" }) {
        INFO("mode " << mode);
        Frames lo = f.Render(FX(mode, "E_SLIDER_FX_Intensity=0"), kRGB, 40, 1, Range(0, 20));
        Frames hi = f.Render(FX(mode, "E_SLIDER_FX_Intensity=255"), kRGB, 40, 1, Range(0, 20));
        int litLo = 0, litHi = 0;
        for (size_t i = 0; i < hi.f.size(); ++i) {
            litLo += lo.lit(i);
            litHi += hi.lit(i);
        }
        CHECK(litHi > litLo);
        CHECK(litHi > 20);
    }
}

TEST_CASE("SingleStrand FX: colours-only palette with two or three colours", "[effects][singlestrand]") {
    Fixture f;
    Frames two = f.Render(FX("Palette", "E_CHOICE_SingleStrand_FX_Palette=* Colors Only"), kRedBlue, 32, 1, Range(0, 4));
    Frames three = f.Render(FX("Palette", "E_CHOICE_SingleStrand_FX_Palette=* Colors Only"), kRGB, 32, 1, Range(0, 4));
    CHECK(two.lit(4) > 0);
    CHECK(three.lit(4) > 0);
    CHECK_FALSE(two == three);
    bool anyGreen = false;
    for (int x = 0; x < 32; ++x) {
        CHECK(two.at(4, x).green == 0);
        anyGreen = anyGreen || three.at(4, x).green > 200;
    }
    CHECK(anyGreen);
}

// ---------------------------------------------------------------------------------------------------------------
// Chase mode

TEST_CASE("SingleStrand chase: every chase type, fade and grouping renders deterministically", "[effects][singlestrand]") {
    Fixture a, b;
    const std::vector<std::string> types = { "Left-Right", "Right-Left", "Bounce from Left", "Bounce from Right", "Dual Chase",
                                             "From Middle", "To Middle", "Bounce to Middle", "Bounce from Middle",
                                             "Static Left-Right", "Static Right-Left", "Static Dual", "Static From Middle",
                                             "Static To Middle", "Static Double-Ended" };
    const std::vector<std::string> fades = { "None", "From Head", "From Tail", "Head and Tail", "Middle" };
    for (const auto& t : types) {
        for (const auto& fade : fades) {
            for (int group = 0; group < 2; ++group) {
                for (int alpha = 0; alpha < 2; ++alpha) {
                    INFO("type " << t << " fade " << fade << " group " << group << " alpha " << alpha);
                    std::string s = Chase("E_CHOICE_Chase_Type1=" + t + ",E_CHOICE_Fade_Type=" + fade +
                                          ",E_SLIDER_Color_Mix1=40,E_SLIDER_Number_Chases=" + std::string(group ? "1" : "2") +
                                          ",E_CHOICE_SingleStrand_Colors=" + (alpha ? "Palette" : "Rainbow") +
                                          (group ? ",E_CHECKBOX_Chase_Group_All=1" : ""));
                    RenderOpts o;
                    o.allowAlpha = alpha != 0;
                    Frames fa = a.Render(s, kRedBlue, 10, 3, { 0, 3, 7, 10, 14, 19 }, o);
                    Frames fb = b.Render(s, kRedBlue, 10, 3, { 0, 3, 7, 10, 14, 19 }, o);
                    CHECK(fa == fb);
                    int total = 0;
                    for (size_t i = 0; i < fa.f.size(); ++i) total += fa.lit(i);
                    // a static chase only shows its leading pixel, which the tail fades take to black
                    if (t.rfind("Static", 0) != 0) CHECK(total > 0);
                    if (!group) {
                        for (size_t i = 0; i < fa.f.size(); ++i) CHECK(fa.columnUniform(i));
                    }
                }
            }
        }
    }
}

TEST_CASE("SingleStrand chase Right-Left mirrors Left-Right", "[effects][singlestrand]") {
    Fixture f;
    const auto frames = Range(0, 19);
    for (const char* fade : { "None", "From Head", "Middle" }) {
        INFO("fade " << fade);
        std::string common = std::string(",E_SLIDER_Color_Mix1=30,E_CHOICE_SingleStrand_Colors=Palette,E_CHOICE_Fade_Type=") + fade;
        Frames lr = f.Render(Chase("E_CHOICE_Chase_Type1=Left-Right" + common), kRedBlue, 12, 2, frames);
        Frames rl = f.Render(Chase("E_CHOICE_Chase_Type1=Right-Left" + common), kRedBlue, 12, 2, frames);
        for (size_t i = 0; i < frames.size(); ++i) {
            INFO("frame " << i);
            CHECK(RgbEqual(Mirror(lr, i), rl.f[i]));
        }
    }
}

TEST_CASE("SingleStrand chase Bounce from Right mirrors Bounce from Left", "[effects][singlestrand]") {
    Fixture f;
    const auto frames = Range(0, 19);
    Frames l = f.Render(Chase("E_CHOICE_Chase_Type1=Bounce from Left,E_SLIDER_Color_Mix1=20"), kRed, 10, 1, frames);
    Frames r = f.Render(Chase("E_CHOICE_Chase_Type1=Bounce from Right,E_SLIDER_Color_Mix1=20"), kRed, 10, 1, frames);
    for (size_t i = 0; i < frames.size(); ++i) {
        INFO("frame " << i);
        CHECK(RgbEqual(Mirror(l, i), r.f[i]));
    }
    // it bounces: the head reaches the far end mid-effect and is back near the start at the end
    CHECK(l.litColumns(0).count(0));
    CHECK(l.litColumns(10).count(9));
    CHECK(*l.litColumns(19).begin() <= 1);
}

TEST_CASE("SingleStrand chase middle and dual types are symmetric", "[effects][singlestrand]") {
    Fixture f;
    const auto frames = Range(0, 19);
    for (const char* t : { "From Middle", "To Middle", "Dual Chase", "Static From Middle", "Static To Middle", "Static Dual",
                           "Bounce to Middle", "Bounce from Middle" }) {
        INFO("type " << t);
        Frames fr = f.Render(Chase(std::string("E_CHOICE_Chase_Type1=") + t + ",E_SLIDER_Color_Mix1=20"), kRed, 12, 1, frames);
        int litFrames = 0;
        for (size_t i = 0; i < frames.size(); ++i) {
            INFO("frame " << i);
            CHECK(RgbEqual(Mirror(fr, i), fr.f[i]));
            litFrames += fr.lit(i) > 0;
        }
        CHECK(litFrames > 0);
    }
}

TEST_CASE("SingleStrand chase static types do not move", "[effects][singlestrand]") {
    Fixture f;
    const auto frames = Range(0, 19);
    for (const char* t : { "Static Left-Right", "Static Right-Left", "Static Dual", "Static From Middle", "Static To Middle", "Static Double-Ended" }) {
        INFO("type " << t);
        Frames fr = f.Render(Chase(std::string("E_CHOICE_Chase_Type1=") + t + ",E_SLIDER_Color_Mix1=30"), kRedBlue, 12, 1, frames);
        CHECK(fr.lit(0) > 0);
        for (size_t i = 1; i < frames.size(); ++i) CHECK(fr.sameFrame(i, 0));
    }
    Frames moving = f.Render(Chase("E_CHOICE_Chase_Type1=Left-Right,E_SLIDER_Color_Mix1=30"), kRedBlue, 12, 1, frames);
    CHECK_FALSE(moving.sameFrame(0, 10));
}

TEST_CASE("SingleStrand chase size sets the number of lit pixels", "[effects][singlestrand]") {
    Fixture f;
    // mid-effect on a 20 pixel strand: chase size is Color_Mix1 percent of the strand
    for (int pct : { 10, 25, 50 }) {
        INFO("pct " << pct);
        Frames fr = f.Render(Chase("E_SLIDER_Color_Mix1=" + std::to_string(pct)), kRed, 20, 1, { 0, 10 });
        CHECK(fr.lit(1) == 20 * pct / 100);
    }
}

TEST_CASE("SingleStrand chase count adds evenly spaced chases", "[effects][singlestrand]") {
    Fixture f;
    Frames one = f.Render(Chase("E_SLIDER_Color_Mix1=10,E_SLIDER_Number_Chases=1"), kRed, 20, 1, { 0, 9 });
    Frames four = f.Render(Chase("E_SLIDER_Color_Mix1=10,E_SLIDER_Number_Chases=4"), kRed, 20, 1, { 0, 9 });
    CHECK(one.lit(1) == 2);
    CHECK(four.lit(1) == 8);
    auto cols = four.litColumns(1);
    std::vector<int> v(cols.begin(), cols.end());
    REQUIRE(v.size() == 8);
    // four pairs, 5 pixels apart (wrapping)
    std::set<int> starts;
    for (int c : v) starts.insert(c % 5);
    CHECK(starts.size() == 2);
}

TEST_CASE("SingleStrand chase cycles and offset move the head", "[effects][singlestrand]") {
    Fixture f;
    const auto frames = Range(0, 19);
    Frames once = f.Render(Chase("E_SLIDER_Color_Mix1=10,E_TEXTCTRL_Chase_Rotations=1.0"), kRed, 20, 1, frames);
    Frames twice = f.Render(Chase("E_SLIDER_Color_Mix1=10,E_TEXTCTRL_Chase_Rotations=2.0"), kRed, 20, 1, frames);
    CHECK_FALSE(once == twice);
    // two cycles: the head is back near the start in the middle of the effect, while one cycle is mid strand
    CHECK(*twice.litColumns(10).begin() <= 3);
    CHECK(*once.litColumns(10).begin() >= 8);

    Frames shifted = f.Render(Chase("E_SLIDER_Color_Mix1=10,E_TEXTCTRL_Chase_Offset=50.0"), kRed, 20, 1, frames);
    CHECK_FALSE(once == shifted);
    CHECK(*shifted.litColumns(0).begin() >= 8);
    Frames negative = f.Render(Chase("E_SLIDER_Color_Mix1=10,E_TEXTCTRL_Chase_Offset=-50.0"), kRed, 20, 1, frames);
    CHECK(*negative.litColumns(0).begin() >= 8);
}

TEST_CASE("SingleStrand chase palette scheme draws palette bands, rainbow scheme draws a hue ramp", "[effects][singlestrand]") {
    Fixture f;
    // A 50% Left-Right chase on 20 pixels is fully on the strand at frame 10, covering pixels 6..15. The
    // palette scheme splits it into one band per colour, last colour at the tail end.
    Frames pal = f.Render(Chase("E_SLIDER_Color_Mix1=50,E_CHOICE_SingleStrand_Colors=Palette"), kRedBlue, 20, 1, { 0, 10 });
    for (int x = 0; x < 20; ++x) {
        INFO("x " << x);
        uint32_t want = x < 6 || x > 15 ? 0u : (x <= 10 ? 0x0000FFu : 0xFF0000u);
        CHECK(Rgb(pal.at(1, x)) == want);
    }

    Frames rb = f.Render(Chase("E_SLIDER_Color_Mix1=50,E_CHOICE_SingleStrand_Colors=Rainbow"), kRedBlue, 20, 1, { 0, 10 });
    CHECK(rb.litColumns(1) == pal.litColumns(1));
    std::set<uint32_t> r = rb.colours(1);
    r.erase(0);
    CHECK(r.size() >= 8);
}

TEST_CASE("SingleStrand chase fades darken one end of the chase", "[effects][singlestrand]") {
    Fixture f;
    auto render = [&](const char* fade, bool alpha) {
        RenderOpts o;
        o.allowAlpha = alpha;
        return f.Render(Chase(std::string("E_SLIDER_Color_Mix1=50,E_CHOICE_SingleStrand_Colors=Palette,E_CHOICE_Fade_Type=") + fade),
                        kRed, 20, 1, { 0, 10 }, o);
    };
    // frame 10: the chase covers pixels 6 (tail) .. 15 (head)
    auto level = [](const xlColor& c, bool alpha) { return alpha ? (int)c.alpha : (int)c.red; };
    for (bool alpha : { false, true }) {
        INFO("alpha " << alpha);
        Frames none = render("None", alpha);
        Frames head = render("From Head", alpha);
        Frames tail = render("From Tail", alpha);
        Frames both = render("Head and Tail", alpha);
        Frames mid = render("Middle", alpha);
        CHECK(level(none.at(1, 6), alpha) == 255);
        CHECK(level(none.at(1, 15), alpha) == 255);
        // From Head: dark at the tail, full at the head
        CHECK(level(head.at(1, 6), alpha) < level(head.at(1, 10), alpha));
        CHECK(level(head.at(1, 10), alpha) < level(head.at(1, 15), alpha));
        CHECK(level(head.at(1, 15), alpha) >= 250);
        // From Tail: the reverse ramp, falling towards the head
        CHECK(level(tail.at(1, 6), alpha) > level(tail.at(1, 7), alpha));
        CHECK(level(tail.at(1, 7), alpha) > level(tail.at(1, 11), alpha));
        CHECK(level(tail.at(1, 11), alpha) > level(tail.at(1, 15), alpha));
        // Head and Tail and Middle differ from no fade and from each other
        CHECK_FALSE(both == none);
        CHECK_FALSE(mid == none);
        CHECK_FALSE(both == mid);
        // all chase pixels stay red
        for (int x = 6; x <= 15; ++x) {
            CHECK(head.at(1, x).green == 0);
            CHECK(tail.at(1, x).blue == 0);
        }
    }
}

TEST_CASE("SingleStrand chase From Tail fade with alpha keeps the tail pixel opaque", "[effects][singlestrand]") {
    Fixture f;
    RenderOpts o;
    o.allowAlpha = true;
    Frames tail = f.Render(Chase("E_SLIDER_Color_Mix1=50,E_CHOICE_SingleStrand_Colors=Palette,E_CHOICE_Fade_Type=From Tail"),
                           kRed, 20, 1, { 0, 10 }, o);
    CHECK(tail.at(1, 6).alpha >= tail.at(1, 7).alpha);
}

TEST_CASE("SingleStrand chase group-all runs across every node of the buffer", "[effects][singlestrand]") {
    Fixture f;
    Frames grouped = f.Render(Chase("E_CHECKBOX_Chase_Group_All=1,E_SLIDER_Color_Mix1=10"), kRed, 5, 4, Range(0, 19));
    Frames plain = f.Render(Chase("E_SLIDER_Color_Mix1=10"), kRed, 5, 4, Range(0, 19));
    // grouped: 2 of 20 nodes lit (10%), in a single row at a time; plain: one column of 4
    std::set<int> rows;
    for (size_t i = 1; i < grouped.f.size(); ++i) {
        INFO("frame " << i);
        CHECK(grouped.lit(i) <= 2);
        for (int y = 0; y < 4; ++y) {
            for (int x = 0; x < 5; ++x) {
                if (IsLit(grouped.at(i, x, y))) rows.insert(y);
            }
        }
        CHECK(plain.lit(i) % 4 == 0);
    }
    CHECK(rows.size() == 4);
}

TEST_CASE("SingleStrand chase follows a timing track", "[effects][singlestrand]") {
    Fixture f;
    auto* t = dynamic_cast<TimingElement*>(f.ctx._sequenceElements.AddElement("Beats", "timing", true, false, true, false, false));
    REQUIRE(t != nullptr);
    if (t->GetEffectLayerCount() == 0) t->AddEffectLayer();
    EffectLayer* el = t->GetEffectLayer(0);
    el->AddEffect(0, "", "", "", 200, 500, 0, false);
    el->AddEffect(0, "", "", "", 500, 800, 0, false);

    const std::string s = Chase("E_SLIDER_Color_Mix1=10,E_CHOICE_SingleStrand_TimingTrack=Beats");
    Frames fr = f.Render(s, kRed, 20, 1, Range(0, 19));
    // nothing before the first mark (200 ms = frame 4) or after the last (800 ms = frame 16)
    for (int i : { 0, 1, 2, 3, 16, 17, 18, 19 }) {
        INFO("frame " << i);
        CHECK(fr.lit(i) == 0);
    }
    for (int i = 4; i < 16; ++i) {
        INFO("frame " << i);
        CHECK(fr.lit(i) > 0);
    }
    // each mark restarts the chase: frames 4 and 10 are both the start of a mark
    CHECK(fr.sameFrame(4, 10));
    CHECK_FALSE(fr.sameFrame(4, 7));

    // a track name that does not exist draws nothing at all
    Frames missing = f.Render(Chase("E_SLIDER_Color_Mix1=10,E_CHOICE_SingleStrand_TimingTrack=Nope"), kRed, 20, 1, Range(0, 19));
    for (size_t i = 0; i < missing.f.size(); ++i) CHECK(missing.lit(i) == 0);
}

// ---------------------------------------------------------------------------------------------------------------
// Skips mode

TEST_CASE("SingleStrand skips draws palette bands separated by gaps", "[effects][singlestrand]") {
    Fixture f;
    Frames fr = f.Render(Skips("E_SLIDER_Skips_BandSize=2,E_SLIDER_Skips_SkipSize=1,E_SLIDER_Skips_StartPos=1,E_CHOICE_Skips_Direction=Right"),
                         kRedBlue, 12, 2, { 0 });
    const uint32_t expect[12] = { 0xFF0000, 0xFF0000, 0, 0x0000FF, 0x0000FF, 0, 0xFF0000, 0xFF0000, 0, 0x0000FF, 0x0000FF, 0 };
    for (int x = 0; x < 12; ++x) {
        INFO("x " << x);
        CHECK(Rgb(fr.at(0, x, 0)) == expect[x]);
        CHECK(Rgb(fr.at(0, x, 1)) == expect[x]);
    }
}

TEST_CASE("SingleStrand skips start position fills backwards from the start", "[effects][singlestrand]") {
    Fixture f;
    Frames fr = f.Render(Skips("E_SLIDER_Skips_BandSize=2,E_SLIDER_Skips_SkipSize=1,E_SLIDER_Skips_StartPos=5,E_CHOICE_Skips_Direction=Right"),
                         kRedBlue, 12, 1, { 0 });
    // forward from x=4: red 4,5 / blue 7,8 / red 10,11; backward from 3 the colours run in reverse: skip 3, blue 2,1, skip 0
    const uint32_t expect[12] = { 0, 0x0000FF, 0x0000FF, 0, 0xFF0000, 0xFF0000, 0, 0x0000FF, 0x0000FF, 0, 0xFF0000, 0xFF0000 };
    for (int x = 0; x < 12; ++x) {
        INFO("x " << x);
        CHECK(Rgb(fr.at(0, x)) == expect[x]);
    }
}

TEST_CASE("SingleStrand skips Left mirrors Right", "[effects][singlestrand]") {
    Fixture f;
    const std::string common = "E_SLIDER_Skips_BandSize=3,E_SLIDER_Skips_SkipSize=2,E_SLIDER_Skips_Advance=4";
    const auto frames = Range(0, 19);
    Frames r = f.Render(Skips(common + ",E_CHOICE_Skips_Direction=Right"), kRedBlue, 13, 1, frames);
    Frames l = f.Render(Skips(common + ",E_CHOICE_Skips_Direction=Left"), kRedBlue, 13, 1, frames);
    for (size_t i = 0; i < frames.size(); ++i) {
        INFO("frame " << i);
        CHECK(RgbEqual(Mirror(r, i), l.f[i]));
    }
}

TEST_CASE("SingleStrand skips middle directions are symmetric", "[effects][singlestrand]") {
    Fixture f;
    const auto frames = Range(0, 19);
    for (const char* d : { "From Middle", "To Middle" }) {
        INFO("direction " << d);
        Frames fr = f.Render(Skips(std::string("E_SLIDER_Skips_BandSize=2,E_SLIDER_Skips_SkipSize=1,E_SLIDER_Skips_Advance=3,E_CHOICE_Skips_Direction=") + d),
                             kRedBlue, 14, 1, frames);
        for (size_t i = 0; i < frames.size(); ++i) {
            INFO("frame " << i);
            CHECK(RgbEqual(Mirror(fr, i), fr.f[i]));
            CHECK(fr.lit(i) > 0);
        }
    }
    Frames from = f.Render(Skips("E_SLIDER_Skips_BandSize=2,E_SLIDER_Skips_SkipSize=1,E_CHOICE_Skips_Direction=From Middle"), kRedBlue, 14, 1, { 0 });
    Frames to = f.Render(Skips("E_SLIDER_Skips_BandSize=2,E_SLIDER_Skips_SkipSize=1,E_CHOICE_Skips_Direction=To Middle"), kRedBlue, 14, 1, { 0 });
    CHECK_FALSE(from == to);
}

TEST_CASE("SingleStrand skips advance steps the pattern along by one band", "[effects][singlestrand]") {
    Fixture f;
    const auto frames = Range(0, 19);
    Frames still = f.Render(Skips("E_SLIDER_Skips_BandSize=2,E_SLIDER_Skips_SkipSize=2,E_CHOICE_Skips_Direction=Right"), kRedBlue, 16, 1, frames);
    for (size_t i = 1; i < frames.size(); ++i) CHECK(still.sameFrame(i, 0));

    Frames moving = f.Render(Skips("E_SLIDER_Skips_BandSize=2,E_SLIDER_Skips_SkipSize=2,E_SLIDER_Skips_Advance=1,E_CHOICE_Skips_Direction=Right"),
                             kRedBlue, 16, 1, frames);
    // advance 1: two positions over the effect; the second half is the first shifted right by the band size
    CHECK(moving.sameFrame(0, 9));
    CHECK_FALSE(moving.sameFrame(0, 19));
    for (int x = 2; x < 16; ++x) {
        INFO("x " << x);
        CHECK(Rgb(moving.at(19, x)) == Rgb(moving.at(0, x - 2)));
    }
}

TEST_CASE("SingleStrand skips with a single colour and no gap fills the strand", "[effects][singlestrand]") {
    Fixture f;
    Frames fr = f.Render(Skips("E_SLIDER_Skips_BandSize=3,E_SLIDER_Skips_SkipSize=0,E_CHOICE_Skips_Direction=Right"), kRed, 10, 2, { 0 });
    CHECK(fr.lit(0) == 20);
    CHECK(fr.colours(0) == std::set<uint32_t>{ 0xFF0000 });
}

// ---------------------------------------------------------------------------------------------------------------
// Spatial colour curves, odd widths and long FX runs

namespace {
const char* kSpatialRedBlue = "C_BUTTON_Palette1=Active=TRUE|Id=ID_BUTTON_Palette1|Values=x=0.000^c=#FF0000;x=1.000^c=#0000FF;|Timecurve=1|,"
                              "C_CHECKBOX_Palette1=1,C_BUTTON_Palette2=#00FF00,C_CHECKBOX_Palette2=1";

// distinct colours that are a red/blue mix (neither pure red nor pure blue, no green)
int RedBlueMixes(const Frames& fr, size_t frame) {
    std::set<uint32_t> mixes;
    for (int x = 0; x < fr.w; ++x) {
        const xlColor& c = fr.at(frame, x);
        if (c.green == 0 && c.red > 0 && c.blue > 0) mixes.insert(Rgb(c));
    }
    return (int)mixes.size();
}
} // namespace

TEST_CASE("SingleStrand skips and chase spread a spatial colour curve along the strand", "[effects][singlestrand]") {
    Fixture f;
    const std::string skips = Skips("E_SLIDER_Skips_BandSize=1,E_SLIDER_Skips_SkipSize=0,E_SLIDER_Skips_StartPos=6,E_CHOICE_Skips_Direction=Right");
    Frames curved = f.Render(skips, kSpatialRedBlue, 20, 1, { 0 });
    CHECK(RedBlueMixes(curved, 0) >= 3);
    Frames flat = f.Render(skips, kRGB, 20, 1, { 0 });
    CHECK(RedBlueMixes(flat, 0) == 0);

    Frames chase = f.Render(Chase("E_SLIDER_Color_Mix1=80,E_CHOICE_SingleStrand_Colors=Palette"), kSpatialRedBlue, 20, 1, { 0, 10 });
    CHECK(RedBlueMixes(chase, 1) >= 2);
}

TEST_CASE("SingleStrand chase middle types on an odd-width strand", "[effects][singlestrand]") {
    Fixture f;
    for (const char* t : { "From Middle", "To Middle", "Static From Middle" }) {
        INFO("type " << t);
        Frames fr = f.Render(Chase(std::string("E_CHOICE_Chase_Type1=") + t + ",E_SLIDER_Color_Mix1=30"), kRed, 13, 1, Range(0, 19));
        int lit = 0;
        for (size_t i = 0; i < fr.f.size(); ++i) {
            lit += fr.lit(i);
            CHECK(RgbEqual(Mirror(fr, i), fr.f[i]));
        }
        CHECK(lit > 0);
    }
}

TEST_CASE("SingleStrand FX: slow modes animate over a longer render", "[effects][singlestrand]") {
    // Modes whose state machines step on second-scale timers: render 12 s and check they keep changing,
    // stay column-uniform and stay visible.
    Fixture f;
    RenderOpts o;
    o.endMs = 12000;
    std::vector<int> frames;
    for (int p = 0; p < 240; p += 2) frames.push_back(p);
    for (const char* mode : { "Fireworks 1D", "Halloween Eyes", "ICU", "Dissolve", "Dissolve Rnd", "Tri Wipe", "Tri Fade",
                              "Twinklefox", "Twinklecat", "Traffic Light", "Popcorn", "Drip", "Fireworks Starburst",
                              "Chase Flash Rnd", "Bouncing Balls", "Lightning", "Candle Multi", "TV Simulator", "Aurora",
                              "Sunrise", "Railway", "Colorful", "Lighthouse", "Multi Comet", "Ripple", "Ripple Rainbow" }) {
        for (const char* opts : { "E_SLIDER_FX_Speed=255,E_SLIDER_FX_Intensity=255",
                                  "E_SLIDER_FX_Speed=180,E_SLIDER_FX_Intensity=60,E_CHOICE_SingleStrand_FX_Palette=C9 2" }) {
            INFO("mode " << mode << " " << opts);
            Frames fr = f.Render(FX(mode, opts), kRGB, 30, 2, frames, o);
            std::set<std::vector<uint32_t>> distinct;
            int lit = 0;
            for (size_t i = 1; i < fr.f.size(); ++i) {
                CHECK(fr.columnUniform(i));
                distinct.insert(Key(fr.f[i]));
                lit += fr.lit(i) > 0;
            }
            CHECK(distinct.size() >= 3);
            CHECK(lit > 10);
        }
    }
}
