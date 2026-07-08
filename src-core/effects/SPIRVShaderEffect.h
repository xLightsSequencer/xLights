#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ShaderEffect.h"
#include "../render/RenderBuffer.h" // EffectRenderCache

#include <array>
#include <string>
#include <unordered_map>

// Shared native (SPIR-V based) Shader effect render lifecycle: everything that
// is identical between the Metal and Vulkan backends lives here — the
// per-buffer cache keying/reset semantics, config parsing, error fills, TIME
// accumulation, ISF uniform value computation (including the dynamic parm
// loop), audio-FFT data preparation and the input-texture-kind decision. The
// backends only translate the (pre-transformed) fragment source, build a
// pipeline, and encode a frame.
class SPIRVShaderEffect : public ShaderEffect {
public:
    SPIRVShaderEffect(int i) :
        ShaderEffect(i) {}
    virtual ~SPIRVShaderEffect() {}

    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;

    using UniformValues = std::unordered_map<std::string, std::array<float, 4>>;

    enum class InputKind {
        Canvas, // sample the buffer's own pixels (feedback / canvas-mode)
        Audio   // sample the 128x1 float FFT/intensity texture
    };

    // Shared per-buffer cache state; backends extend with their pipeline and
    // per-size resources and release them in platformReset().
    class CacheBase : public EffectRenderCache {
    public:
        CacheBase() {}
        virtual ~CacheBase();

        ShaderConfig* config = nullptr;
        std::string shaderFile;    // cache key: rebuild when the effect's .fs changes
        std::string transformedSource; // ShaderSourceTransforms::Apply(config->GetCode())
        long timeMS = 0;
        int width = 0;
        int height = 0;
        bool built = false;
        bool failed = false;

        void reset();
        virtual void platformReset() = 0;
    };

protected:
    // ---- backend hooks ----------------------------------------------------
    virtual bool nativeAvailable() const = 0;
    virtual CacheBase* newCache() const = 0;
    // Translate cache->transformedSource, build the pipeline and the
    // per-buffer-size resources. Failure renders solid yellow (latched).
    virtual bool nativeBuild(CacheBase* cache, RenderBuffer& buffer) = 0;
    // Encode one frame. audio128 is non-null (128 floats) iff kind == Audio.
    // Return false to fill this frame yellow without latching failure.
    virtual bool nativeEncode(CacheBase* cache, RenderBuffer& buffer,
                              const UniformValues& vals, InputKind kind,
                              const float* audio128) = 0;
};
