#pragma once

// GPU-vs-CPU render comparison for the [gpu] tests: render one show with the
// GPU preference off and then on, and compare the frames.
//
// These need a usable GPU compute backend: Metal on Apple, Vulkan elsewhere. A
// software Vulkan device (lavapipe/llvmpipe) only takes compute work with
// XL_VULKAN_ALLOW_CPU=1, so on Linux run them as
// `XL_VULKAN_ALLOW_CPU=1 xlights_unit_tests "[gpu]"` (ctest sets it for them).

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <string>
#include <vector>

#include "render/GPURenderUtils.h"
#include "test_render_common.h"

namespace gpuhelp {

using namespace renderhelp;

// Big enough to clear the backends' size thresholds, below which they hand the
// buffer to the CPU path.
constexpr int kSide = 100;

inline bool GpuCompute() {
    return GPURenderUtils::IsEnabled();
}

#define XL_REQUIRE_GPU() \
    if (!gpuhelp::GpuCompute()) SKIP("no GPU compute backend (on Linux, XL_VULKAN_ALLOW_CPU=1 allows a software Vulkan device)")

struct GpuPreference {
    bool saved = GPURenderUtils::IsPreferenceEnabled();
    ~GpuPreference() { GPURenderUtils::SetEnabled(saved); }
};

inline ShowSpec MatrixShow(std::vector<EffectSpec> effects) {
    ShowSpec s;
    s.models = Matrix("M", 1, kSide, kSide);
    s.durationMs = 500;
    s.effects = std::move(effects);
    return s;
}

struct Diff {
    size_t channels = 0;
    int maxDelta = 0;
};

inline Diff Compare(const Rendered& a, const Rendered& b) {
    Diff d;
    for (unsigned int f = 0; f < a.frames; ++f) {
        for (unsigned int c = 0; c < a.channels; ++c) {
            int delta = std::abs((int)a.at(f, c) - (int)b.at(f, c));
            if (delta) {
                ++d.channels;
                d.maxDelta = std::max(d.maxDelta, delta);
            }
        }
    }
    return d;
}

inline size_t Lit(const Rendered& r) {
    size_t n = 0;
    for (const auto& f : r.data) {
        for (uint8_t v : f) n += v != 0;
    }
    return n;
}

// How far a GPU render may stray from the CPU render of the same show: at most
// `fraction` of all channels over all frames may differ, by at most `maxDelta`.
// The default allows float rounding (GPU fast-math against ISPC/scalar);
// anything bigger is listed where it is used, with the reason.
struct Budget {
    double fraction = 0.001;
    int maxDelta = 2;
};

struct Pair {
    Rendered cpu, gpu;
};

// Renders the show on the CPU and then on the GPU. `counter` reads a backend
// statistic the GPU path must advance, so a quiet fallback to the CPU fails.
inline Pair RenderBoth(const ShowSpec& spec, const std::function<uint64_t()>& counter) {
    GpuPreference keep;
    Pair p;
    GPURenderUtils::SetEnabled(false);
    p.cpu = Render(spec);
    GPURenderUtils::SetEnabled(true);
    uint64_t before = counter();
    p.gpu = Render(spec);
    INFO("the GPU path did not run");
    CHECK(counter() > before);
    return p;
}

inline void CheckWithin(const Pair& p, const Budget& b) {
    REQUIRE(p.cpu.rendered);
    REQUIRE(p.gpu.rendered);
    REQUIRE(p.cpu.channels == p.gpu.channels);
    REQUIRE(p.cpu.frames == p.gpu.frames);
    CHECK(Lit(p.cpu) > 0);
    CHECK(Lit(p.gpu) > 0);
    Diff d = Compare(p.cpu, p.gpu);
    size_t total = (size_t)p.cpu.channels * p.cpu.frames;
    INFO(d.channels << " of " << total << " channels differ, by up to " << d.maxDelta);
    CHECK(d.channels <= (size_t)(b.fraction * total));
    CHECK(d.maxDelta <= b.maxDelta);
}

} // namespace gpuhelp
