#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Runtime GLSL (Vulkan dialect) -> SPIR-V for the native Vulkan Shader effect,
// via glslang.  Vulkan consumes SPIR-V directly, so this is the whole pipeline
// (no SPIRV-Tools/spirv-cross, unlike the Metal GLSL->SPIR-V->MSL path).  This
// mirrors MetalShaderTranslator's glslToSpirv step and can be unified with it
// into a shared wx-free ShaderTranslate in the NativeShaderEffect work.
// Compiled only under HAVE_VULKAN.
#ifdef HAVE_VULKAN

#include <cstdint>
#include <string>
#include <vector>

namespace VulkanShaderTranslate {

enum class Stage { Vertex, Fragment };

// Compile one Vulkan-dialect GLSL stage to SPIR-V words.  Returns true on
// success; on failure returns false with `error` populated.  Thread-safe
// (glslang's process init is one-time; parsing is serialized internally).
bool ToSpirv(const std::string& glsl, Stage stage, std::vector<uint32_t>& out, std::string& error);

}  // namespace VulkanShaderTranslate

#endif
