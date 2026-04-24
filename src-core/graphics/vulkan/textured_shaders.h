#pragma once

#include <vector>

// Embedded SPIR-V for a minimal textured quad vertex and fragment shader.
// These were compiled with glslangValidator from simple GLSL shaders.

// Vertex shader (input: vec3 pos, vec2 uv) outputs vec2 uv and positions in NDC using push constants for rect
extern const uint32_t textured_vert_spv[];
extern const size_t textured_vert_spv_size;

// Fragment shader samples combined image sampler at uv
extern const uint32_t textured_frag_spv[];
extern const size_t textured_frag_spv_size;

static inline std::vector<uint32_t> get_textured_vert_spv() {
    return std::vector<uint32_t>(textured_vert_spv, textured_vert_spv + textured_vert_spv_size / sizeof(uint32_t));
}
static inline std::vector<uint32_t> get_textured_frag_spv() {
    return std::vector<uint32_t>(textured_frag_spv, textured_frag_spv + textured_frag_spv_size / sizeof(uint32_t));
}
