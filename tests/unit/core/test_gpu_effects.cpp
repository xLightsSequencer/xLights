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

const std::vector<GpuEffect>& GpuEffects() {
    static const std::vector<GpuEffect> v = {
        { "Bars", "E_CHOICE_Bars_Direction=up", false },
        { "Butterfly", "", false },
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
        { "Plasma", "", false },
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

// Each style and colour mode of the two effects with more than one GPU kernel.
TEST_CASE("GPU Butterfly and Plasma variants match their CPU render", "[gpu][effects]") {
    XL_REQUIRE_GPU();
    std::vector<std::pair<std::string, std::string>> variants;
    for (int style = 1; style <= 10; ++style) {
        for (const char* colors : { "Rainbow", "Palette" }) {
            variants.push_back({ "Butterfly", "E_SLIDER_Butterfly_Style=" + std::to_string(style) + ",E_CHOICE_Butterfly_Colors=" + colors });
        }
    }
    variants.push_back({ "Butterfly", "E_SLIDER_Butterfly_Style=1,E_SLIDER_Butterfly_Chunks=4,E_SLIDER_Butterfly_Skip=3" });
    for (const char* color : { "Normal", "Preset Colors 1", "Preset Colors 2", "Preset Colors 3", "Preset Colors 4" }) {
        variants.push_back({ "Plasma", std::string("E_CHOICE_Plasma_Color=") + color + ",E_SLIDER_Plasma_Style=3,E_SLIDER_Plasma_Line_Density=2" });
    }
    for (const auto& [name, settings] : variants) {
        INFO(name << " " << settings);
        Pair p = RenderBoth(MatrixShow({ Eff("M", 0, name, 0, 500, Pal("#FF0000", "#00FF00", "C_BUTTON_Palette3=#0000FF,C_CHECKBOX_Palette3=1"), settings) }),
                            [] { return GPURenderUtils::stats.effect.load(); });
        Budget budget = {};
        if (settings.find("Style=4,") != std::string::npos && settings.find("Palette") != std::string::npos) {
            // Style 4 wraps the hue with h - floor(h), and the palette blend is not
            // circular: where CPU and GPU sin() differ by an ulp at the wrap, a pixel
            // takes the last colour on one and the first on the other (seen on a
            // paravirtual Metal device: one pixel, 2 channels by 255).
            budget.seams = 6;
        }
        CheckWithin(p, budget);
    }
}
