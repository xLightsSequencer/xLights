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
#include <atomic>
#include <cstdint>
#include <string>
#include <unordered_map>

// XL_SHADER_BUILD_STATS=1 accumulates where the native Shader path's CPU time
// actually goes and dumps a table to stderr at exit. Split out because "shader
// compiling is slow" can mean any of four unrelated things: re-parsing the .fs,
// re-running the source transforms, glslang+pipeline translation, or per-frame
// encode. Zero cost when unset (Enabled() is read once).
namespace ShaderBuildStats {
bool Enabled();
void AddParse(uint64_t ns);
void AddTransform(uint64_t ns);
void AddBuild(uint64_t ns);
void AddTranslate(uint64_t ns);
void AddCacheHit();
void AddEncode(uint64_t ns);
// Breakdown *inside* one encode, so the per-frame cost can be attributed rather
// than inferred. The backend that submits its own work (Vulkan today) reports
// these; a backend that rides the engine's command buffer leaves them at zero.
void AddUpload(uint64_t ns);   // input staged (host memcpy; upload rides the render cb)
void AddRecord(uint64_t ns);   // pool reset + begin + record + end
void AddSubmit(uint64_t ns);   // vkQueueSubmit, including the queue mutex
void AddFenceWait(uint64_t ns);// vkWaitForFences at completion.  Deferred frames
                               // wait when the pixels are consumed, so this can
                               // be far below gpu exec when work overlapped.
void AddReadback(uint64_t ns); // host memcpy out of the mapped readback buffer
// Device-timestamp span of the submitted command buffer (Vulkan): true GPU
// execution of this frame's work alone, regardless of where the wait happened.
void AddGpuExec(uint64_t ns);
// Per-shader-file cost attribution, dumped as a top-N table at exit.  The two
// backends report different quantities on purpose: Vulkan reports the
// device-timestamp execution span, GL reports CPU wall for the synchronous
// draw+glReadPixels block — comparing the tables across two runs localises
// whether an execution-cost gap is shader-specific (codegen) or uniform
// (structural).
void AddPerShader(const std::string& file, uint64_t ns, uint32_t w, uint32_t h);
// Print the table immediately (idempotent — the static destructor also calls
// this).  The headless path calls it before std::exit because on the GL-only
// path some earlier static teardown prevents the destructor from ever running.
void Dump();
} // namespace ShaderBuildStats

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

        // Per-frame uniform values, kept across frames rather than rebuilt.
        // Every frame writes the same key set, so once it is populated
        // operator[] finds each node and allocates nothing; a fresh map instead
        // allocated a node per uniform plus a bucket array on every frame, which
        // at six figures of frames is the dominant cost of assembling them.
        // Cleared by reset(), i.e. whenever the shader changes and the key set
        // could differ.
        UniformValues vals;

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
