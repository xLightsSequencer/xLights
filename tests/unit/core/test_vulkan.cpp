// The Vulkan backend (Linux/Windows). Compiled only where the core is built with
// HAVE_VULKAN, which on the CMake test build means Linux.
//
// GLSL -> SPIR-V for the Vulkan Shader effect; needs no device. The GPU render
// comparisons, shared with Metal, are in test_gpu_effects.cpp and
// test_gpu_rotozoom.cpp.
#ifdef HAVE_VULKAN

#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <cstdint>
#include <string>
#include <thread>
#include <vector>

#include "effects/vulkan/VulkanShaderTranslate.h"

namespace {

constexpr uint32_t kSpirvMagic = 0x07230203;

const char* kVertex = R"(#version 450
layout(location = 0) in vec2 pos;
layout(location = 0) out vec2 uv;
void main() {
    uv = pos * 0.5 + 0.5;
    gl_Position = vec4(pos, 0.0, 1.0);
}
)";

const char* kFragment = R"(#version 450
layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 color;
layout(set = 0, binding = 0) uniform Params { float t; } params;
void main() {
    color = vec4(uv, params.t, 1.0);
}
)";

} // namespace

TEST_CASE("Vulkan GLSL translation produces SPIR-V for both stages", "[vulkan][translate]") {
    std::vector<uint32_t> spirv;
    std::string error;
    REQUIRE(VulkanShaderTranslate::ToSpirv(kVertex, VulkanShaderTranslate::Stage::Vertex, spirv, error));
    CHECK(error.empty());
    REQUIRE(spirv.size() > 5);
    CHECK(spirv[0] == kSpirvMagic);

    spirv.clear();
    REQUIRE(VulkanShaderTranslate::ToSpirv(kFragment, VulkanShaderTranslate::Stage::Fragment, spirv, error));
    REQUIRE(spirv.size() > 5);
    CHECK(spirv[0] == kSpirvMagic);
}

TEST_CASE("Vulkan GLSL translation reports a syntax error", "[vulkan][translate]") {
    std::vector<uint32_t> spirv;
    std::string error;
    CHECK_FALSE(VulkanShaderTranslate::ToSpirv("#version 450\nvoid main() { vec4 c = ; }\n",
                                               VulkanShaderTranslate::Stage::Fragment, spirv, error));
    CHECK_FALSE(error.empty());
}

TEST_CASE("Vulkan GLSL translation rejects a shader compiled for the wrong stage", "[vulkan][translate]") {
    // gl_FragCoord only exists in a fragment shader.
    const char* frag = "#version 450\nlayout(location = 0) out vec4 c;\nvoid main() { c = gl_FragCoord; }\n";
    std::vector<uint32_t> spirv;
    std::string error;
    CHECK(VulkanShaderTranslate::ToSpirv(frag, VulkanShaderTranslate::Stage::Fragment, spirv, error));
    spirv.clear();
    error.clear();
    CHECK_FALSE(VulkanShaderTranslate::ToSpirv(frag, VulkanShaderTranslate::Stage::Vertex, spirv, error));
    CHECK_FALSE(error.empty());
}

TEST_CASE("Vulkan GLSL translation is safe to call from several threads", "[vulkan][translate]") {
    std::atomic<int> ok{ 0 };
    std::vector<std::thread> threads;
    for (int i = 0; i < 8; ++i) {
        threads.emplace_back([&ok, i]() {
            std::vector<uint32_t> spirv;
            std::string error;
            bool vert = (i % 2) == 0;
            if (VulkanShaderTranslate::ToSpirv(vert ? kVertex : kFragment,
                                               vert ? VulkanShaderTranslate::Stage::Vertex : VulkanShaderTranslate::Stage::Fragment,
                                               spirv, error) &&
                !spirv.empty() && spirv[0] == kSpirvMagic) {
                ++ok;
            }
        });
    }
    for (auto& t : threads) t.join();
    CHECK(ok == 8);
}

#endif
