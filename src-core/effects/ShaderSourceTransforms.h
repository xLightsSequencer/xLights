#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <string>

// GLSL-source-level parity transforms applied to the ShaderConfig-assembled
// fragment source before any backend (Metal, Vulkan) translates it. Real-world
// ISF shaders rely on lenient GL-driver behavior; these make that behavior
// deterministic on strict SPIR-V backends:
//
//  - pow(x, y) with a negative base is undefined in GLSL, but GL drivers
//    strength-reduce constant integral exponents (pow(x, 2.0) -> x*x), so
//    shaders depend on it working. Rewrites pow( -> xl_pow( and injects
//    GL-compatible overloads (negative base + integral exponent uses
//    pow(|x|, y) with the sign of an odd power).
//
//  - GL drivers zero-initialize shader locals and shaders read
//    declared-but-unassigned variables (e.g. `float i,g,d=1.;`). Strict
//    backends get garbage/NaN -> black. Adds explicit zero initializers to
//    uninitialized local scalar/vector declarations.
namespace ShaderSourceTransforms {

std::string Apply(const std::string& assembledFragment);

} // namespace ShaderSourceTransforms
