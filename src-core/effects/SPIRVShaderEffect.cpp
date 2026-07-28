/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "SPIRVShaderEffect.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <map>
#include <mutex>
#include <vector>

#include "EffectManager.h"
#include "ShaderSourceTransforms.h"
#include "UtilClasses.h"
#include "../render/GPURenderUtils.h"
#include "../render/RenderBuffer.h"
#include "../render/SequenceElements.h" // EffectLayer/Effect for EVENT parms
#include "../media/AudioManager.h"      // FFT frame data for audio shaders

namespace ShaderBuildStats {
namespace {
struct Counters {
    std::atomic<uint64_t> parseNs{ 0 }, parseN{ 0 };
    std::atomic<uint64_t> transformNs{ 0 }, transformN{ 0 };
    std::atomic<uint64_t> buildNs{ 0 }, buildN{ 0 };
    std::atomic<uint64_t> translateNs{ 0 }, translateN{ 0 };
    std::atomic<uint64_t> cacheHits{ 0 };
    std::atomic<uint64_t> encodeNs{ 0 }, encodeN{ 0 };
    std::atomic<uint64_t> uploadNs{ 0 }, uploadN{ 0 };
    std::atomic<uint64_t> recordNs{ 0 }, recordN{ 0 };
    std::atomic<uint64_t> submitNs{ 0 }, submitN{ 0 };
    std::atomic<uint64_t> fenceNs{ 0 }, fenceN{ 0 };
    std::atomic<uint64_t> readbackNs{ 0 }, readbackN{ 0 };
    std::atomic<uint64_t> gpuExecNs{ 0 }, gpuExecN{ 0 };

    struct PerShader {
        uint64_t ns = 0;
        uint64_t frames = 0;
        uint32_t w = 0, h = 0;
    };
    std::mutex perShaderMtx;
    std::map<std::string, PerShader> perShader;
    std::atomic<bool> dumped{ false };

    ~Counters() {
        dump();
    }

    void dump() {
        if (!Enabled() || dumped.exchange(true)) {
            return;
        }
        auto row = [](const char* name, uint64_t n, uint64_t ns) {
            fprintf(stderr, "%-28s %10llu %12.1f %12.4f\n", name,
                    (unsigned long long)n, ns / 1.0e6,
                    n ? (ns / 1.0e6) / (double)n : 0.0);
        };
        fprintf(stderr, "\n=== XL_SHADER_BUILD_STATS ===\n");
        fprintf(stderr, "%-28s %10s %12s %12s\n", "stage", "count", "total ms", "ms each");
        row("ParseShader", parseN, parseNs);
        row("SourceTransforms::Apply", transformN, transformNs);
        row("nativeBuild (total)", buildN, buildNs);
        row("  glslang+pipeline (miss)", translateN, translateNs);
        row("  program-cache hit", cacheHits, 0);
        row("nativeEncode (per frame)", encodeN, encodeNs);
        if (uploadN || recordN || submitN || fenceN || readbackN) {
            // Sums to slightly less than nativeEncode: the remainder is uniform
            // packing and descriptor writes.
            fprintf(stderr, "  -- inside nativeEncode --\n");
            row("  input upload (own trip)", uploadN, uploadNs);
            row("  record cb", recordN, recordNs);
            row("  queue submit", submitN, submitNs);
            row("  fence wait", fenceN, fenceNs);
            row("  readback memcpy", readbackN, readbackNs);
            if (gpuExecN) {
                // Device timestamps: this frame's work alone.  When waits are
                // synchronous, (fence wait - gpu exec) is time queued behind
                // other rows; with deferred completion the wait can drop far
                // below exec because the GPU work overlapped other CPU work.
                row("  gpu exec (device ts)", gpuExecN, gpuExecNs);
                row("  wait minus exec", gpuExecN,
                    fenceNs > gpuExecNs ? (uint64_t)(fenceNs - gpuExecNs) : 0);
            }
        }
        if (!perShader.empty()) {
            std::vector<std::pair<std::string, PerShader>> sorted(perShader.begin(), perShader.end());
            std::sort(sorted.begin(), sorted.end(),
                      [](const auto& a, const auto& b) { return a.second.ns > b.second.ns; });
            fprintf(stderr, "  -- per shader (top 20 by total; Vulkan: device-ts exec, GL: CPU wall) --\n");
            fprintf(stderr, "  %-44s %9s %11s %10s %10s\n", "shader", "frames", "total ms", "ms each", "size");
            size_t n = 0;
            for (const auto& [file, s] : sorted) {
                if (++n > 20) break;
                std::string base = file;
                size_t slash = base.find_last_of("/\\");
                if (slash != std::string::npos) base = base.substr(slash + 1);
                if (base.size() > 44) base = base.substr(0, 44);
                fprintf(stderr, "  %-44s %9llu %11.1f %10.4f %6ux%u\n", base.c_str(),
                        (unsigned long long)s.frames, s.ns / 1.0e6,
                        s.frames ? (s.ns / 1.0e6) / (double)s.frames : 0.0, s.w, s.h);
            }
        }
        fprintf(stderr, "=== end XL_SHADER_BUILD_STATS ===\n");
    }
};
Counters& counters() {
    static Counters c;
    return c;
}
} // namespace

bool Enabled() {
    static const bool e = getenv("XL_SHADER_BUILD_STATS") != nullptr;
    return e;
}
void AddParse(uint64_t ns) { counters().parseNs += ns; counters().parseN++; }
void AddTransform(uint64_t ns) { counters().transformNs += ns; counters().transformN++; }
void AddBuild(uint64_t ns) { counters().buildNs += ns; counters().buildN++; }
void AddTranslate(uint64_t ns) { counters().translateNs += ns; counters().translateN++; }
void AddCacheHit() { counters().cacheHits++; }
void AddEncode(uint64_t ns) { counters().encodeNs += ns; counters().encodeN++; }
void AddUpload(uint64_t ns) { counters().uploadNs += ns; counters().uploadN++; }
void AddRecord(uint64_t ns) { counters().recordNs += ns; counters().recordN++; }
void AddSubmit(uint64_t ns) { counters().submitNs += ns; counters().submitN++; }
void AddFenceWait(uint64_t ns) { counters().fenceNs += ns; counters().fenceN++; }
void AddReadback(uint64_t ns) { counters().readbackNs += ns; counters().readbackN++; }
void AddGpuExec(uint64_t ns) { counters().gpuExecNs += ns; counters().gpuExecN++; }
void AddPerShader(const std::string& file, uint64_t ns, uint32_t w, uint32_t h) {
    Counters& c = counters();
    std::lock_guard<std::mutex> lk(c.perShaderMtx);
    auto& s = c.perShader[file];
    s.ns += ns;
    s.frames++;
    s.w = w;
    s.h = h;
}
void Dump() {
    if (Enabled()) {
        counters().dump();
    }
}
} // namespace ShaderBuildStats

namespace {
// Fixed calendar anchor for the ISF DATE uniform's y/m/d fields; its seconds
// field carries the sequence position. See where DATE is packed.
constexpr int kShaderDateYear = 2000;
constexpr int kShaderDateMonth = 1;
constexpr int kShaderDateDay = 1;

// Scoped nanosecond timer. The clock is only read when the stats flag is on, so
// the disabled path costs a predictable-branch test - this sits in the per-frame
// encode path, which runs six figures of times in a normal render.
struct StatTimer {
    std::chrono::steady_clock::time_point t0;
    StatTimer() {
        if (ShaderBuildStats::Enabled()) {
            t0 = std::chrono::steady_clock::now();
        }
    }
    uint64_t elapsed() const {
        return (uint64_t)std::chrono::duration_cast<std::chrono::nanoseconds>(
                   std::chrono::steady_clock::now() - t0)
            .count();
    }
};
} // namespace

SPIRVShaderEffect::CacheBase::~CacheBase() {
    delete config;
}

void SPIRVShaderEffect::CacheBase::reset() {
    delete config;
    config = nullptr;
    transformedSource.clear();
    built = false;
    failed = false;
    vals.clear(); // the next shader may declare a different set of uniforms
    platformReset();
}

void SPIRVShaderEffect::Render(Effect* eff, const SettingsMap& SettingsMap, RenderBuffer& buffer) {
    // XL_NO_NATIVE_SHADER=1 falls back to the previous path (OpenGL where it
    // still exists) for diagnostics/comparison.
    static const bool nativeEnabled = getenv("XL_NO_NATIVE_SHADER") == nullptr;
    // The PREFERENCE must be tested BEFORE nativeAvailable(): the latter brings up
    // the GPU backend as a side effect (device, render pass, pipelines, and the
    // per-thread command pools/images the render path then allocates).  The GPU
    // rendering preference is the user's safety net for a misbehaving driver, so
    // when it is off nothing GPU-side may be created at all.  Testing it is
    // allocation-free: both backends short-circuit on their own flag before
    // touching computeEnabled().
    //
    // This never strands the iPad on ShaderEffect::Render (a red-fill stub
    // there, with no GL to fall back to): a MetalShaderEffect only exists at all
    // when computeEnabled() was already true, and IsPreferenceEnabled() is the
    // desktop-only preference flag, which the iPad never clears.
    //
    // IsPreferenceEnabled() rather than IsEnabled() because the latter also
    // folds in "the compute backend is usable", and this effect uses the
    // graphics pipeline, not compute.  On a machine whose only Vulkan device is
    // a software one, compute correctly declines it (ISPC is faster) while this
    // effect - which has no CPU implementation - still needs it.
    if (!nativeEnabled || !GPURenderUtils::IsPreferenceEnabled() || !nativeAvailable()) {
        ShaderEffect::Render(eff, SettingsMap, buffer);
        return;
    }

    const std::string shaderFile = SettingsMap.Get("0FILEPICKERCTRL_IFS", "");
    if (shaderFile.empty()) {
        buffer.Fill(xlRED); // no shader file configured — same as the base path
        return;
    }

    const int slot = id + 2 * EffectManager::eff_LASTEFFECT; // distinct from the base ShaderRenderCache slot
    auto* cache = static_cast<CacheBase*>(buffer.infoCache[slot]);
    if (cache == nullptr) {
        cache = newCache();
        buffer.infoCache[slot] = cache;
    }

    // A new effect (or a different .fs) on the same buffer must not reuse the
    // previous pipeline/config — or its failed flag. The infoCache slot is per
    // (buffer, effect TYPE), so this reset runs BEFORE the failed check.
    if (cache->shaderFile != shaderFile) {
        cache->reset();
        cache->shaderFile = shaderFile;
    }
    if (cache->failed) {
        buffer.Fill(cache->config == nullptr ? xlRED : xlYELLOW);
        return;
    }

    const bool effectStart = buffer.needToInit;

    // ---- build config + pipeline on first frame / file change / resize ----
    if (!cache->built || cache->width != buffer.BufferWi || cache->height != buffer.BufferHt) {
        if (cache->config == nullptr) {
            StatTimer t;
            cache->config = ShaderEffect::ParseShader(shaderFile, GetSequenceElements(buffer));
            if (ShaderBuildStats::Enabled()) {
                ShaderBuildStats::AddParse(t.elapsed());
            }
        }
        if (cache->config == nullptr || cache->config->GetCode().empty()) {
            delete cache->config;
            cache->config = nullptr; // signals red (missing/unparseable) on later frames
            cache->failed = true;
            buffer.Fill(xlRED);
            return;
        }
        if (cache->transformedSource.empty()) {
            StatTimer t;
            cache->transformedSource = ShaderSourceTransforms::Apply(cache->config->GetCode());
            if (ShaderBuildStats::Enabled()) {
                ShaderBuildStats::AddTransform(t.elapsed());
            }
        }
        cache->built = false;
        StatTimer buildTimer;
        const bool buildOk = nativeBuild(cache, buffer);
        if (ShaderBuildStats::Enabled()) {
            ShaderBuildStats::AddBuild(buildTimer.elapsed());
        }
        if (!buildOk) {
            cache->failed = true; // translation/pipeline failure — like a GL compile failure
            buffer.Fill(xlYELLOW);
            return;
        }
        cache->width = buffer.BufferWi;
        cache->height = buffer.BufferHt;
        cache->built = true;
    }

    if (effectStart) {
        cache->timeMS = SettingsMap.GetInt("TEXTCTRL_Shader_LeadIn", 0) * buffer.frameTimeInMs;
    }

    float oset = buffer.GetEffectTimeIntervalPosition();
    double timeRate = GetValueCurveDouble("Shader_Speed", 100, SettingsMap, oset, SHADER_SPEED_MIN, SHADER_SPEED_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), 1) / 100.0;
    cache->timeMS += buffer.frameTimeInMs * timeRate;

    double offsetX = GetValueCurveInt("Shader_Offset_X", 0, SettingsMap, oset, SHADER_OFFSET_X_MIN, SHADER_OFFSET_X_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), 1) / 200.0 + 0.5;
    double offsetY = GetValueCurveInt("Shader_Offset_Y", 0, SettingsMap, oset, SHADER_OFFSET_Y_MIN, SHADER_OFFSET_Y_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), 1) / 200.0 + 0.5;
    double zoom = GetValueCurveInt("Shader_Zoom", 0, SettingsMap, oset, SHADER_ZOOM_MIN, SHADER_ZOOM_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), 1);
    if (zoom < 0) {
        zoom = 1.0 - std::abs(zoom) / 100.0;
    } else if (zoom > 0) {
        zoom = 1.0 + (zoom * 9.0) / 100.0;
    } else {
        zoom = 1.0;
    }

    // Compute all uniform values as floats; each backend marshals float bits vs
    // int bits from its reflected/declared member types.
    // Reused across frames (see CacheBase::vals): the key set is identical every
    // frame, so this allocates on the first frame of an effect and never again.
    UniformValues& vals = cache->vals;
    auto set1 = [&](const std::string& n, float a) { vals[n] = { a, 0, 0, 0 }; };
    auto set2 = [&](const std::string& n, float a, float b) { vals[n] = { a, b, 0, 0 }; };
    auto set4 = [&](const std::string& n, float a, float b, float c, float d) { vals[n] = { a, b, c, d }; };

    set2("RENDERSIZE", buffer.BufferWi, buffer.BufferHt);
    set2("XL_OFFSET", offsetX, offsetY);
    set1("XL_ZOOM", zoom);
    set1("XL_DURATION", (buffer.GetEndTimeMS() - buffer.GetStartTimeMS()) / 1000.0);
    set1("TIME", cache->timeMS / 1000.0);
    set1("TIMEDELTA", buffer.frameTimeInMs / 1000.f);
    {
        // Derived from the sequence timeline, NOT the wall clock. An .fseq is a
        // baked artifact: a render-time clock is meaningless on playback, and it
        // makes the render irreproducible - two runs seconds apart feed a
        // different DATE into the shader, which is enough to break the
        // byte-identity regression gate for the whole sequence when the shader
        // feeds back on itself. Anchoring to the sequence also makes clock-style
        // shaders advance with the sequence instead of sitting frozen at
        // whatever time the render happened to start.
        const double seqSeconds = (buffer.curPeriod * (double)buffer.frameTimeInMs) / 1000.0;
        set4("DATE", kShaderDateYear, kShaderDateMonth, kShaderDateDay, seqSeconds);
    }
    set1("NUMCOLORS", buffer.GetColorCount());
    set1("PASSINDEX", 0);
    set1("FRAMEINDEX", cache->timeMS / buffer.frameTimeInMs);
    set1("clearBuffer", SettingsMap.GetBool("CHECKBOX_OverlayBkg", false) ? 1.f : 0.f);
    set1("resetNow", (buffer.curPeriod == buffer.curEffStartPer) ? 1.f : 0.f);

    int colourIndex = 0;
    for (const auto& it : cache->config->GetParms()) {
        switch (it._type) {
        case ShaderParmType::SHADER_PARM_FLOAT:
            set1(it._name, GetValueCurveDouble(it.GetUndecoratedId(ShaderCtrlType::SHADER_CTRL_VALUECURVE), it._default * 100.0, SettingsMap, oset, it._min * 100.0, it._max * 100.0, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), 1) / 100.0);
            break;
        case ShaderParmType::SHADER_PARM_POINT2D: {
            double x = GetValueCurveDouble(it.GetUndecoratedId(ShaderCtrlType::SHADER_CTRL_VALUECURVE) + "X", it._defaultPt.x * 100, SettingsMap, oset, it._minPt.x * 100, it._maxPt.x * 100, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), 1) / 100.0;
            double y = GetValueCurveDouble(it.GetUndecoratedId(ShaderCtrlType::SHADER_CTRL_VALUECURVE) + "Y", it._defaultPt.y * 100, SettingsMap, oset, it._minPt.y * 100, it._maxPt.y * 100, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), 1) / 100.0;
            set2(it._name, x, y);
            break;
        }
        case ShaderParmType::SHADER_PARM_BOOL:
            set1(it._name, SettingsMap.GetBool(it.GetUndecoratedId(ShaderCtrlType::SHADER_CTRL_CHECKBOX)) ? 1.f : 0.f);
            break;
        case ShaderParmType::SHADER_PARM_EVENT: {
            auto timingtrack = SettingsMap.Get(it.GetUndecoratedId(ShaderCtrlType::SHADER_CTRL_TIMING), "");
            EffectLayer* el = GetTiming(timingtrack, GetSequenceElements(buffer));
            bool b = false;
            if (el != nullptr) {
                int ms = buffer.curPeriod * buffer.frameTimeInMs;
                for (int j = 0; j < el->GetEffectCount(); j++) {
                    int ems = el->GetEffect(j)->GetStartTimeMS();
                    if (ems == ms) {
                        b = true;
                        break;
                    }
                    if (ems > ms) {
                        break;
                    }
                }
            }
            set1(it._name, b ? 1.f : 0.f);
            break;
        }
        case ShaderParmType::SHADER_PARM_LONGCHOICE:
            set1(it._name, (float)it.EncodeChoice(SettingsMap[it.GetUndecoratedId(ShaderCtrlType::SHADER_CTRL_CHOICE)]));
            break;
        case ShaderParmType::SHADER_PARM_LONG:
            set1(it._name, (float)GetValueCurveInt(it.GetUndecoratedId(ShaderCtrlType::SHADER_CTRL_VALUECURVE), it._default, SettingsMap, oset, it._min, it._max, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), 1));
            break;
        case ShaderParmType::SHADER_PARM_COLOUR: {
            xlColor c = buffer.palette.GetColor(colourIndex);
            colourIndex++;
            if (colourIndex > (int)buffer.GetColorCount()) {
                colourIndex = 0;
            }
            set4(it._name, c.red / 255.0, c.green / 255.0, c.blue / 255.0, 1.0);
            break;
        }
        default:
            break;
        }
    }

    // ---- input texture data: audio replaces the canvas/feedback input ------
    InputKind kind = InputKind::Canvas;
    float fft[128] = { 0 };
    const float* audio = nullptr;
    if (cache->config->IsAudioFFTShader() || cache->config->IsAudioIntensityShader()) {
        kind = InputKind::Audio;
        audio = fft;
        AudioManager* audioManager = buffer.GetMedia();
        if (audioManager != nullptr) {
            auto fftData = audioManager->GetFrameData(buffer.curPeriod, "");
            if (fftData) {
                if (cache->config->IsAudioFFTShader()) {
                    const size_t n = std::min<size_t>(fftData->vu.size(), 128);
                    for (size_t i = 0; i < n; i++) {
                        fft[i] = fftData->vu[i];
                    }
                } else {
                    for (int i = 0; i < 127; i++) {
                        fft[i] = fftData->max;
                    }
                }
            }
        }
    }

    StatTimer encodeTimer;
    const bool encodeOk = nativeEncode(cache, buffer, vals, kind, audio);
    if (ShaderBuildStats::Enabled()) {
        ShaderBuildStats::AddEncode(encodeTimer.elapsed());
    }
    if (!encodeOk) {
        // Transient (command buffer / descriptor exhaustion): fill this frame
        // rather than latching failure.
        buffer.Fill(xlYELLOW);
        return;
    }
    buffer.needToInit = false;
}
