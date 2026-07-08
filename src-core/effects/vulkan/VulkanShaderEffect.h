#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Vulkan backend of the shared SPIRVShaderEffect: rewrites the (already
// source-transformed) fragment GLSL into Vulkan dialect (anonymous std140 UBO +
// binding'd sampler + layout'd varyings), translates to SPIR-V
// (VulkanShaderTranslate), builds a fullscreen graphics pipeline
// (VulkanGraphicsUtilities), packs the computed uniform values into a
// host-visible UBO, and renders into the RenderBuffer. All lifecycle, uniform
// value and audio logic lives in SPIRVShaderEffect. Compiled only under
// HAVE_VULKAN; unavailable Vulkan falls back to the base ShaderEffect (OpenGL).
#ifdef HAVE_VULKAN_SHADER

#include "../SPIRVShaderEffect.h"

class VulkanShaderEffect : public SPIRVShaderEffect {
public:
    VulkanShaderEffect(int i);
    virtual ~VulkanShaderEffect();

    // Headless validation for the --shadertranslate CLI: run the ISF->Vulkan
    // GLSL transform + glslang GLSL->SPIR-V on an assembled fragment shader
    // (both stages).  Returns true if both produced SPIR-V; else sets `error`.
    // No GPU/device required — proves the translation path alone.
    static bool ValidateTranslate(const std::string& assembledFragCode, std::string& error);

protected:
    virtual bool nativeAvailable() const override;
    virtual CacheBase* newCache() const override;
    virtual bool nativeBuild(CacheBase* cache, RenderBuffer& buffer) override;
    virtual bool nativeEncode(CacheBase* cache, RenderBuffer& buffer,
                              const UniformValues& vals, InputKind kind,
                              const float* audio128) override;
};

#endif
