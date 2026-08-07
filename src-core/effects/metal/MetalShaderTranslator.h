#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <string>
#include <vector>
#include <cstdint>

// Runtime translation of xLights-assembled ISF GLSL to Metal Shading Language,
// via glslang (GLSL -> SPIR-V) -> SPIRV-Tools (merge-return + exhaustive inline)
// -> SPIRV-Cross (SPIR-V -> MSL). This is the exact pipeline validated by the
// shader-translate spike (100% of the ISF corpus). Header is wx-free; the SPIR-V
// step is also what a future native Vulkan shader path would consume directly.
namespace ShaderTranslate {

enum class Stage { Vertex, Fragment };

// Translate one GLSL stage to MSL. Returns the MSL source on success, or an
// empty string on failure with `error` populated. `forIOS` selects the MSL
// platform target (iOS vs macOS). Thread-safe (serialized internally; callers
// translate once per unique shader and cache the result).
std::string ToMSL(const std::string& glsl, Stage stage, bool forIOS, std::string& error);

// Smoke test / validation: translate a fragment GLSL string and compile the
// resulting MSL into a throwaway MTLLibrary on the system default device.
// Returns true iff the whole GLSL -> MSL -> MTLLibrary chain succeeds. Used to
// measure native-path coverage over a shader corpus before wiring the renderer.
bool TranslateAndCompile(const std::string& glslFragment, std::string& error);

// One uniform's MSL buffer index (from spirv-cross reflection). Uniforms the
// shader doesn't actually use are omitted (their GL analogue is a -1 location).
struct ShaderBinding {
    std::string name;
    uint32_t index;    // MSL [[buffer(index)]]
    uint8_t vecSize;   // 1/2/3/4 components
    bool isFloat;      // true=float, false=int/uint/bool (written as int bits)
};

struct ShaderStageInfo {
    std::vector<ShaderBinding> uniforms;
    int samplerTexture = -1; // texSampler [[texture(N)]] (fragment), or -1 if unused
    int attrVpos = -1;       // vertex [[attribute(N)]] for vpos, or -1
    int attrTpos = -1;       // vertex [[attribute(N)]] for tpos, or -1
};

// A fully translated vertex+fragment program plus the reflection the renderer
// needs to bind uniforms/textures/vertex attributes by name/slot. The two stages
// are linked together in glslang (mapIO) so their varying locations match.
struct TranslatedProgram {
    bool ok = false;
    std::string vertexMSL;
    std::string fragmentMSL;
    ShaderStageInfo vertex;
    ShaderStageInfo fragment;
    std::string error;
};

TranslatedProgram TranslateProgram(const std::string& vertexGLSL, const std::string& fragmentGLSL, bool forIOS);

// Validation: translate a vertex+fragment GLSL pair, compile both to MTLLibraries,
// and build a full MTLRenderPipelineState (RGBA8 target, vertex descriptor from
// reflection). Returns true iff the whole render-setup chain succeeds. Proves the
// native render path's setup before the per-frame encoder is wired.
bool ValidateRenderPipeline(const std::string& vertexGLSL, const std::string& fragmentGLSL, std::string& error);

} // namespace ShaderTranslate
