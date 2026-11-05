// The GPU effects and pipeline stages (Metal and Vulkan) against the CPU render
// of the same show. Skips without a GPU compute backend; see test_gpu_common.h.

#include <catch2/catch_test_macros.hpp>

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "test_gpu_common.h"

using namespace gpuhelp;

namespace {

struct GpuEffect {
    const char* name;
    // Settings that select the variant with a GPU kernel (the defaults of
    // several of these take the CPU path).
    const char* settings;
    // Kaleidoscope and Warp transform the layer below them.
    bool canvas;
    Budget budget = {};
};

#ifdef __APPLE__
// On Metal these two drift from the CPU by a few levels over much of the
// buffer (Butterfly ~17% of channels by up to 4, Plasma ~2.4% by 1); the
// Metal kernels build with -fmetal-math-mode=fast.
constexpr Budget kButterfly = { 0.25, 6 };
constexpr Budget kPlasma = { 0.05, 2 };
#else
constexpr Budget kButterfly = {};
constexpr Budget kPlasma = {};
#endif

const std::vector<GpuEffect>& GpuEffects() {
    static const std::vector<GpuEffect> v = {
        { "Bars", "E_CHOICE_Bars_Direction=up", false },
        { "Butterfly", "", false, kButterfly },
        { "Candle", "E_CHECKBOX_PerNode=1", false },
        { "Circles", "", false },
        { "Color Wash", "", false },
        { "Fan", "", false },
        { "Fill", "", false },
        { "Fire", "E_CHOICE_Fire_Style=New Render Method", false },
        { "Galaxy", "", false },
        { "Garlands", "", false },
        // Samples the folded source position; a handful of pixels on the fold
        // edges land on the neighbouring source pixel.
        { "Kaleidoscope", "E_CHOICE_Kaleidoscope_Type=6-Fold", true, { 0.003, 255 } },
        { "Life", "", false },
        { "Meteors", "", false },
        { "Pinwheel", "E_CHOICE_Pinwheel_Style=New Render Method", false },
        { "Plasma", "", false, kPlasma },
        { "Shimmer", "", false },
        { "Shockwave", "", false },
        // A few pixels on the spiral edges round the other way.
        { "Spirals", "", false, { 0.0002, 255 } },
        { "Tree", "", false },
        { "Twinkle", "", false },
        // The ripple displacement goes through GPU sin(); ~2% of pixels sample
        // a neighbour.
        { "Warp", "E_CHOICE_Warp_Type=ripple", true, { 0.04, 255 } },
        { "Wave", "", false },
    };
    return v;
}

std::vector<EffectSpec> EffectOnly(const GpuEffect& e) {
    std::string settings = e.settings;
    if (!e.canvas) {
        return { Eff("M", 0, e.name, 0, 500, Pal("#FF0000", "#00FF00"), settings) };
    }
    if (!settings.empty()) settings += ",";
    settings += "T_CHECKBOX_Canvas=1";
    return { Eff("M", 0, e.name, 0, 500, Pal("#FF0000", "#00FF00"), settings),
             Eff("M", 1, "Bars", 0, 500, Pal("#FF0000", "#0000FF"), "E_CHOICE_Bars_Direction=Left") };
}

// XL_TEST_GPU_HASH=1 prints a hash of each GPU render, for checking that a
// backend refactor leaves the GPU output unchanged.
void MaybePrintHash(const char* name, const Rendered& r) {
    if (getenv("XL_TEST_GPU_HASH") == nullptr) return;
    uint64_t h = 1469598103934665603ull;
    for (const auto& f : r.data) {
        for (uint8_t v : f) h = (h ^ v) * 1099511628211ull;
    }
    printf("GPUHASH %s %016llx\n", name, (unsigned long long)h);
}

} // namespace

TEST_CASE("GPU effects match their CPU render", "[gpu][effects]") {
    XL_REQUIRE_GPU();
    for (const auto& e : GpuEffects()) {
        INFO(e.name);
        Pair p = RenderBoth(MatrixShow(EffectOnly(e)), [] { return GPURenderUtils::stats.effect.load(); });
        MaybePrintHash(e.name, p.gpu);
        CheckWithin(p, e.budget);
    }
}

TEST_CASE("GPU pipeline stages match their CPU render", "[gpu][stages]") {
    XL_REQUIRE_GPU();
    auto& st = GPURenderUtils::stats;
    const std::string bars = "E_CHOICE_Bars_Direction=Left";
    struct Stage {
        const char* name;
        std::atomic<uint64_t>* counter;
        std::vector<EffectSpec> effects;
        Budget budget = {};
    };
    // The GPU blur is a separable tent, faster than the CPU's three box passes
    // and kept on purpose; its budget pins today's difference so a broken
    // kernel still fails.
    std::vector<Stage> stages = {
        { "blur", &st.blur, { Eff("M", 0, "Bars", 0, 500, Pal("#FF0000", "#0000FF"), bars + ",B_SLIDER_Blur=8") }, { 0.25, 16 } },
        { "blend", &st.blend, { Eff("M", 0, "Bars", 0, 500, Pal("#FF0000", "#0000FF"), bars + ",T_CHOICE_LayerMethod=Additive"),
                                Eff("M", 1, "Fill", 0, 500, Pal("#00FF00")) } },
        { "transition", &st.transition, { Eff("M", 0, "Bars", 0, 500, Pal("#FF0000", "#0000FF"),
                                              bars + ",T_CHOICE_In_Transition_Type=Wipe,T_TEXTCTRL_Fadein=0.4,T_CHOICE_LayerMethod=Normal") }, { 0.002, 255 } },
    };
    for (auto& s : stages) {
        INFO(s.name);
        std::atomic<uint64_t>* counter = s.counter;
        Pair p = RenderBoth(MatrixShow(s.effects), [counter] { return counter->load(); });
        MaybePrintHash(s.name, p.gpu);
        CheckWithin(p, s.budget);
    }
}
