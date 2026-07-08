/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#ifdef HAVE_VULKAN_SHADER

#include "VulkanShaderTranslate.h"

#include <mutex>

#include <glslang/Public/ShaderLang.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/SPIRV/GlslangToSpv.h>

// Windows .sln build links deps via #pragma comment (the CMake build defines
// XLIGHTS_CMAKE_BUILD and links glslang via target_link_libraries instead).
// glslang was built from tag sdk-1.3.231.1 into lib/windows64 (reflection-only,
// SPIRV-Tools optimizer disabled); debug libs carry the 'd' suffix.
#if defined(_WIN32) && !defined(XLIGHTS_CMAKE_BUILD)
#ifdef _DEBUG
#pragma comment(lib, "glslangd.lib")
#pragma comment(lib, "MachineIndependentd.lib")
#pragma comment(lib, "GenericCodeGend.lib")
#pragma comment(lib, "OSDependentd.lib")
#pragma comment(lib, "OGLCompilerd.lib")
#pragma comment(lib, "SPIRVd.lib")
#else
#pragma comment(lib, "glslang.lib")
#pragma comment(lib, "MachineIndependent.lib")
#pragma comment(lib, "GenericCodeGen.lib")
#pragma comment(lib, "OSDependent.lib")
#pragma comment(lib, "OGLCompiler.lib")
#pragma comment(lib, "SPIRV.lib")
#endif
#endif

namespace VulkanShaderTranslate {

static void ensureProcess() {
    // glslang requires exactly one process-wide InitializeProcess().
    static std::once_flag once;
    std::call_once(once, []() { glslang::InitializeProcess(); });
}

bool ToSpirv(const std::string& glsl, Stage stage, std::vector<uint32_t>& out, std::string& error) {
    ensureProcess();
    // glslang's parser/linker are not internally synchronized across TShaders;
    // serialize (callers translate once per unique shader and cache anyway).
    static std::mutex mtx;
    std::lock_guard<std::mutex> lock(mtx);

    const EShLanguage lang = (stage == Stage::Vertex) ? EShLangVertex : EShLangFragment;
    glslang::TShader shader(lang);
    const char* src = glsl.c_str();
    shader.setStrings(&src, 1);
    shader.setEnvInput(glslang::EShSourceGlsl, lang, glslang::EShClientVulkan, 100);
    shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_1);
    shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_3);
    shader.setAutoMapBindings(true);
    shader.setAutoMapLocations(true);

    if (!shader.parse(GetDefaultResources(), 100, false, EShMsgDefault)) {
        error = shader.getInfoLog();
        return false;
    }
    glslang::TProgram program;
    program.addShader(&shader);
    if (!program.link(EShMsgDefault) || !program.mapIO()) {
        error = program.getInfoLog();
        return false;
    }
    out.clear();
    glslang::GlslangToSpv(*program.getIntermediate(lang), out);
    if (out.empty()) {
        error = "GlslangToSpv produced no words";
        return false;
    }
    return true;
}

}  // namespace VulkanShaderTranslate

#endif
