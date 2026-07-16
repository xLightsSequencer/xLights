/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "SPIRVShaderEffect.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>

#include "EffectManager.h"
#include "ShaderSourceTransforms.h"
#include "UtilClasses.h"
#include "../render/GPURenderUtils.h"
#include "../render/RenderBuffer.h"
#include "../render/SequenceElements.h" // EffectLayer/Effect for EVENT parms
#include "../media/AudioManager.h"      // FFT frame data for audio shaders

SPIRVShaderEffect::CacheBase::~CacheBase() {
    delete config;
}

void SPIRVShaderEffect::CacheBase::reset() {
    delete config;
    config = nullptr;
    transformedSource.clear();
    built = false;
    failed = false;
    platformReset();
}

void SPIRVShaderEffect::Render(Effect* eff, const SettingsMap& SettingsMap, RenderBuffer& buffer) {
    // XL_NO_NATIVE_SHADER=1 falls back to the previous path (OpenGL where it
    // still exists) for diagnostics/comparison.
    static const bool nativeEnabled = getenv("XL_NO_NATIVE_SHADER") == nullptr;
    // IsEnabled() must be tested BEFORE nativeAvailable(): the latter brings up
    // the GPU backend as a side effect (device, render pass, pipelines, and the
    // per-thread command pools/images the render path then allocates).  The GPU
    // rendering preference is the user's safety net for a misbehaving driver, so
    // when it is off nothing GPU-side may be created at all.  Testing it is
    // allocation-free: both backends short-circuit on their own flag before
    // touching computeEnabled().
    //
    // This never strands the iPad on ShaderEffect::Render (a red-fill stub
    // there, with no GL to fall back to): a MetalShaderEffect only exists at all
    // when computeEnabled() was already true, so IsEnabled() collapses to the
    // desktop-only preference flag, which the iPad never clears.
    if (!nativeEnabled || !GPURenderUtils::IsEnabled() || !nativeAvailable()) {
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
            cache->config = ShaderEffect::ParseShader(shaderFile, GetSequenceElements(buffer));
        }
        if (cache->config == nullptr || cache->config->GetCode().empty()) {
            delete cache->config;
            cache->config = nullptr; // signals red (missing/unparseable) on later frames
            cache->failed = true;
            buffer.Fill(xlRED);
            return;
        }
        if (cache->transformedSource.empty()) {
            cache->transformedSource = ShaderSourceTransforms::Apply(cache->config->GetCode());
        }
        cache->built = false;
        if (!nativeBuild(cache, buffer)) {
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
    UniformValues vals;
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
        std::time_t nowt = std::time(nullptr);
        std::tm tmbuf;
#ifdef _MSC_VER
        localtime_s(&tmbuf, &nowt);
#else
        localtime_r(&nowt, &tmbuf);
#endif
        set4("DATE", tmbuf.tm_year + 1900, tmbuf.tm_mon + 1, tmbuf.tm_mday, tmbuf.tm_hour * 3600 + tmbuf.tm_min * 60 + tmbuf.tm_sec);
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

    if (!nativeEncode(cache, buffer, vals, kind, audio)) {
        // Transient (command buffer / descriptor exhaustion): fill this frame
        // rather than latching failure.
        buffer.Fill(xlYELLOW);
        return;
    }
    buffer.needToInit = false;
}
