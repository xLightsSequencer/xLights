/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "LiquidEffect.h"

#include <cmath>
#include <cstdlib>
#include <spdlog/fmt/fmt.h>
#include <spdlog/spdlog.h>
#include <Box2D/Box2D.h>
#include "../render/Effect.h"
#include "../render/EffectLayer.h"
#include "../render/Element.h"
#include "../render/RenderBuffer.h"
#include "../render/SequenceElements.h"
#include "../render/ValueCurve.h"
#include "UtilClasses.h"
#include "media/AudioManager.h"
#include "UtilFunctions.h"
#include "../models/Model.h"
#include "Parallel.h"

#include "../../include/liquid-16.xpm"
#include "../../include/liquid-24.xpm"
#include "../../include/liquid-32.xpm"
#include "../../include/liquid-48.xpm"
#include "../../include/liquid-64.xpm"

#include <log.h>

#define MAX_PARTICLES 100000

bool LiquidEffect::sTopBarrierDefault = false;
bool LiquidEffect::sBottomBarrierDefault = true;
bool LiquidEffect::sLeftBarrierDefault = false;
bool LiquidEffect::sRightBarrierDefault = false;
bool LiquidEffect::sHoldColorDefault = true;
bool LiquidEffect::sMixColorsDefault = false;
std::string LiquidEffect::sParticleTypeDefault = "Elastic";
int LiquidEffect::sLifeTimeDefault = 1000;
int LiquidEffect::sLifeTimeMin = 0;
int LiquidEffect::sLifeTimeMax = 1000;
int LiquidEffect::sSizeDefault = 500;
int LiquidEffect::sWarmUpTimeDefault = 0;
int LiquidEffect::sDespeckleDefault = 0;
double LiquidEffect::sGravityDefault = 10.0;
double LiquidEffect::sGravityMin = -1000;
double LiquidEffect::sGravityMax = 1000;
int LiquidEffect::sGravityDivisor = 10;
int LiquidEffect::sGravityAngleDefault = 0;
int LiquidEffect::sGravityAngleMin = 0;
int LiquidEffect::sGravityAngleMax = 360;
bool LiquidEffect::sEnabled1Default = true;
int LiquidEffect::sX1Default = 50;
int LiquidEffect::sY1Default = 100;
int LiquidEffect::sDirection1Default = 270;
int LiquidEffect::sVelocity1Default = 100;
int LiquidEffect::sFlow1Default = 100;
int LiquidEffect::sSourceSize1Default = 0;
bool LiquidEffect::sFlowMusic1Default = false;
bool LiquidEffect::sEnabled2Default = false;
int LiquidEffect::sX2Default = 0;
int LiquidEffect::sY2Default = 50;
int LiquidEffect::sDirection2Default = 0;
int LiquidEffect::sVelocity2Default = 100;
int LiquidEffect::sFlow2Default = 100;
int LiquidEffect::sSourceSize2Default = 0;
bool LiquidEffect::sFlowMusic2Default = false;
bool LiquidEffect::sEnabled3Default = false;
int LiquidEffect::sX3Default = 50;
int LiquidEffect::sY3Default = 0;
int LiquidEffect::sDirection3Default = 90;
int LiquidEffect::sVelocity3Default = 100;
int LiquidEffect::sFlow3Default = 100;
int LiquidEffect::sSourceSize3Default = 0;
bool LiquidEffect::sFlowMusic3Default = false;
bool LiquidEffect::sEnabled4Default = false;
int LiquidEffect::sX4Default = 100;
int LiquidEffect::sY4Default = 50;
int LiquidEffect::sDirection4Default = 180;
int LiquidEffect::sVelocity4Default = 100;
int LiquidEffect::sFlow4Default = 100;
int LiquidEffect::sSourceSize4Default = 0;
bool LiquidEffect::sFlowMusic4Default = false;
int LiquidEffect::sXMin = 0;
int LiquidEffect::sXMax = 100;
int LiquidEffect::sYMin = 0;
int LiquidEffect::sYMax = 100;
int LiquidEffect::sDirectionMin = 0;
int LiquidEffect::sDirectionMax = 360;
int LiquidEffect::sVelocityMin = 0;
int LiquidEffect::sVelocityMax = 1000;
int LiquidEffect::sFlowMin = 0;
int LiquidEffect::sFlowMax = 1000;
int LiquidEffect::sSourceSizeMin = 0;
int LiquidEffect::sSourceSizeMax = 100;

LiquidEffect::LiquidEffect(int id) : RenderableEffect(id, "Liquid", liquid_16, liquid_24, liquid_32, liquid_48, liquid_64)
{
}

LiquidEffect::~LiquidEffect()
{
}

void LiquidEffect::OnMetadataLoaded()
{
    sTopBarrierDefault = GetBoolDefault("TopBarrier", sTopBarrierDefault);
    sBottomBarrierDefault = GetBoolDefault("BottomBarrier", sBottomBarrierDefault);
    sLeftBarrierDefault = GetBoolDefault("LeftBarrier", sLeftBarrierDefault);
    sRightBarrierDefault = GetBoolDefault("RightBarrier", sRightBarrierDefault);
    sHoldColorDefault = GetBoolDefault("HoldColor", sHoldColorDefault);
    sMixColorsDefault = GetBoolDefault("MixColors", sMixColorsDefault);
    sParticleTypeDefault = GetStringDefault("ParticleType", sParticleTypeDefault);
    sLifeTimeDefault = GetIntDefault("LifeTime", sLifeTimeDefault);
    sLifeTimeMin = (int)GetMinFromMetadata("LifeTime", sLifeTimeMin);
    sLifeTimeMax = (int)GetMaxFromMetadata("LifeTime", sLifeTimeMax);
    sSizeDefault = GetIntDefault("Size", sSizeDefault);
    sWarmUpTimeDefault = GetIntDefault("WarmUpTime", sWarmUpTimeDefault);
    sDespeckleDefault = GetIntDefault("Despeckle", sDespeckleDefault);
    sGravityDefault = GetDoubleDefault("Liquid_Gravity", sGravityDefault);
    sGravityMin = GetMinFromMetadata("Liquid_Gravity", sGravityMin);
    sGravityMax = GetMaxFromMetadata("Liquid_Gravity", sGravityMax);
    sGravityDivisor = GetDivisorFromMetadata("Liquid_Gravity", sGravityDivisor);
    sGravityAngleDefault = GetIntDefault("Liquid_GravityAngle", sGravityAngleDefault);
    sGravityAngleMin = (int)GetMinFromMetadata("Liquid_GravityAngle", sGravityAngleMin);
    sGravityAngleMax = (int)GetMaxFromMetadata("Liquid_GravityAngle", sGravityAngleMax);

    sEnabled1Default = GetBoolDefault("Enabled1", sEnabled1Default);
    sX1Default = GetIntDefault("X1", sX1Default);
    sY1Default = GetIntDefault("Y1", sY1Default);
    sDirection1Default = GetIntDefault("Direction1", sDirection1Default);
    sVelocity1Default = GetIntDefault("Velocity1", sVelocity1Default);
    sFlow1Default = GetIntDefault("Flow1", sFlow1Default);
    sSourceSize1Default = GetIntDefault("Liquid_SourceSize1", sSourceSize1Default);
    sFlowMusic1Default = GetBoolDefault("FlowMusic1", sFlowMusic1Default);

    sEnabled2Default = GetBoolDefault("Enabled2", sEnabled2Default);
    sX2Default = GetIntDefault("X2", sX2Default);
    sY2Default = GetIntDefault("Y2", sY2Default);
    sDirection2Default = GetIntDefault("Direction2", sDirection2Default);
    sVelocity2Default = GetIntDefault("Velocity2", sVelocity2Default);
    sFlow2Default = GetIntDefault("Flow2", sFlow2Default);
    sSourceSize2Default = GetIntDefault("Liquid_SourceSize2", sSourceSize2Default);
    sFlowMusic2Default = GetBoolDefault("FlowMusic2", sFlowMusic2Default);

    sEnabled3Default = GetBoolDefault("Enabled3", sEnabled3Default);
    sX3Default = GetIntDefault("X3", sX3Default);
    sY3Default = GetIntDefault("Y3", sY3Default);
    sDirection3Default = GetIntDefault("Direction3", sDirection3Default);
    sVelocity3Default = GetIntDefault("Velocity3", sVelocity3Default);
    sFlow3Default = GetIntDefault("Flow3", sFlow3Default);
    sSourceSize3Default = GetIntDefault("Liquid_SourceSize3", sSourceSize3Default);
    sFlowMusic3Default = GetBoolDefault("FlowMusic3", sFlowMusic3Default);

    sEnabled4Default = GetBoolDefault("Enabled4", sEnabled4Default);
    sX4Default = GetIntDefault("X4", sX4Default);
    sY4Default = GetIntDefault("Y4", sY4Default);
    sDirection4Default = GetIntDefault("Direction4", sDirection4Default);
    sVelocity4Default = GetIntDefault("Velocity4", sVelocity4Default);
    sFlow4Default = GetIntDefault("Flow4", sFlow4Default);
    sSourceSize4Default = GetIntDefault("Liquid_SourceSize4", sSourceSize4Default);
    sFlowMusic4Default = GetBoolDefault("FlowMusic4", sFlowMusic4Default);

    // Shared min/max values across all sources (all X*/Y*/Direction*/Velocity*/Flow*/Liquid_SourceSize*
    // in Liquid.json share the same range, so read from source 1 as a representative).
    sXMin = (int)GetMinFromMetadata("X1", sXMin);
    sXMax = (int)GetMaxFromMetadata("X1", sXMax);
    sYMin = (int)GetMinFromMetadata("Y1", sYMin);
    sYMax = (int)GetMaxFromMetadata("Y1", sYMax);
    sDirectionMin = (int)GetMinFromMetadata("Direction1", sDirectionMin);
    sDirectionMax = (int)GetMaxFromMetadata("Direction1", sDirectionMax);
    sVelocityMin = (int)GetMinFromMetadata("Velocity1", sVelocityMin);
    sVelocityMax = (int)GetMaxFromMetadata("Velocity1", sVelocityMax);
    sFlowMin = (int)GetMinFromMetadata("Flow1", sFlowMin);
    sFlowMax = (int)GetMaxFromMetadata("Flow1", sFlowMax);
    sSourceSizeMin = (int)GetMinFromMetadata("Liquid_SourceSize1", sSourceSizeMin);
    sSourceSizeMax = (int)GetMaxFromMetadata("Liquid_SourceSize1", sSourceSizeMax);
}

bool LiquidEffect::needToAdjustSettings(const std::string& version)
{
    if (IsVersionOlder("2026.07.1", version)) {
        return true;
    }
    return RenderableEffect::needToAdjustSettings(version);
}

void LiquidEffect::adjustSettings(const std::string& version, Effect* effect, bool removeDefaults)
{
    if (RenderableEffect::needToAdjustSettings(version)) {
        RenderableEffect::adjustSettings(version, effect, removeDefaults);
    }

    SettingsMap& settings = effect->GetSettings();

    // 2026.08: Liquid effect went frame-rate independent. Sequences
    // saved before this used per-frame semantics; we need the original
    // sequence frame rate to convert them so old sequences render
    // visually identically at any new frame rate.
    if (IsVersionOlder("2026.07.1", version)) {
        // Look up the sequence's frame interval. Defaults to 50ms (the
        // long-time historical default) if anything in the chain is
        // missing — adjustSettings can run in contexts where the
        // effect isn't fully attached yet.
        int oldFrameMs = 50;
        if (effect != nullptr) {
            EffectLayer* layer = effect->GetParentEffectLayer();
            if (layer != nullptr) {
                Element* element = layer->GetParentElement();
                if (element != nullptr) {
                    SequenceElements* seqElems = element->GetSequenceElements();
                    if (seqElems != nullptr) {
                        const int fm = seqElems->GetFrameMS();
                        if (fm > 0) oldFrameMs = fm;
                    }
                }
            }
        }
        const float oldFps = 1000.0F / static_cast<float>(oldFrameMs);

        auto migrate = [&](const char* keyBase, float scale, long minVal, long maxVal) {
            for (int n = 1; n <= 4; ++n) {
                const std::string sliderKey = fmt::format("E_TEXTCTRL_{}{}", keyBase, n);
                const std::string vcKey = fmt::format("E_VALUECURVE_{}{}", keyBase, n);

                const std::string slider = settings.Get(sliderKey, "");
                if (!slider.empty()) {
                    const long oldVal = std::strtol(slider.c_str(), nullptr, 10);
                    const long newVal = std::min<long>(static_cast<long>(oldVal * scale), maxVal);
                    settings[sliderKey] = std::to_string(newVal);
                }

                const std::string vc = settings.Get(vcKey, "");
                if (!vc.empty()) {
                    ValueCurve valc;
                    valc.SetDivisor(1.0F);
                    valc.SetLimits(static_cast<float>(minVal), static_cast<float>(maxVal));
                    valc.Deserialise(vc);
                    valc.ScaleAndOffsetValues(scale, 0);
                    settings[vcKey] = valc.Serialise();
                }
            }
        };

        // Velocity went from physical = user × 10 to physical = user / 10.
        // Multiply stored user value by 100 to preserve physical motion.
        // Frame-rate independent (always was — velocity is in buf-u/sec).
        migrate("Velocity", 100.0F, sVelocityMin, sVelocityMax);

        // Flow went from particles-per-frame to particles-per-second
        // (mapped through the linear+exponential curve). Old per-frame
        // value V at oldFps becomes per-sec V × oldFps. Slider maps
        // particles/sec via curve: 0..500 linear (slope 4), 500..1000
        // exponential. We invert the curve to find the new slider
        // value that gives the same per-sec rate.
        for (int n = 1; n <= 4; ++n) {
            const std::string sliderKey = fmt::format("E_TEXTCTRL_Flow{}", n);
            const std::string vcKey = fmt::format("E_VALUECURVE_Flow{}", n);

            auto perSecToSlider = [](float perSec) -> long {
                if (perSec <= 0.0F) return 0;
                if (perSec <= 2000.0F) return static_cast<long>(perSec * 0.25F);  // inverse of slider*4
                // 500 + 500 * log10(perSec / 2000)
                const float t = std::log10(perSec / 2000.0F);
                return std::min<long>(500L + static_cast<long>(500.0F * t), 1000L);
            };

            const std::string slider = settings.Get(sliderKey, "");
            if (!slider.empty()) {
                const long oldVal = std::strtol(slider.c_str(), nullptr, 10);
                const float oldPerSec = static_cast<float>(oldVal) * oldFps;
                settings[sliderKey] = std::to_string(perSecToSlider(oldPerSec));
            }

            const std::string vc = settings.Get(vcKey, "");
            if (!vc.empty()) {
                // Value curves store points scaled to [min, max]. Old
                // points were V (per-frame); new points need to be
                // perSecToSlider(V × oldFps). Approximate by scaling
                // the whole curve linearly — accurate inside the
                // linear half of the new curve, slightly compressed in
                // the exponential half. Most users keep flow value
                // curves below the exponential breakpoint anyway, so
                // this is close enough.
                ValueCurve valc;
                valc.SetDivisor(1.0F);
                valc.SetLimits(static_cast<float>(sFlowMin), static_cast<float>(sFlowMax));
                valc.Deserialise(vc);
                // V/frame × oldFps = perSec; perSec / 4 = slider (linear half).
                valc.ScaleAndOffsetValues(oldFps * 0.25F, 0);
                settings[vcKey] = valc.Serialise();
            }
        }

        // WarmUpFrames (count of frames at oldFps) → WarmUpTime
        // (hundredths of a second). 100 hundredths = 1 sec, so
        // hundredths = frames * (100 / oldFps) = frames * oldFrameMs / 10.
        {
            const std::string oldKey = "E_TEXTCTRL_WarmUpFrames";
            const std::string newKey = "E_TEXTCTRL_WarmUpTime";
            const std::string slider = settings.Get(oldKey, "");
            if (!slider.empty()) {
                const long oldVal = std::strtol(slider.c_str(), nullptr, 10);
                const long newVal = std::min<long>(oldVal * oldFrameMs / 10, 2500L);
                settings[newKey] = std::to_string(newVal);
                settings.erase(oldKey);
            }
        }
    }
}

std::list<std::string> LiquidEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache)
{
    std::list<std::string> res = RenderableEffect::CheckEffectSettings(settings, media, model, eff, renderCache);

    if (media == nullptr && (settings.GetBool("E_CHECKBOX_FlowMusic1", false) ||
                             settings.GetBool("E_CHECKBOX_FlowMusic2", false) ||
                             settings.GetBool("E_CHECKBOX_FlowMusic3", false) ||
                             settings.GetBool("E_CHECKBOX_FlowMusic4", false))) {
        res.push_back(fmt::format("    WARN: Liquid effect cant change flow to music if there is no music. Model '{}', Start {}", model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())));
    }

    int frameInterval = 50;
    if (media != nullptr)
        frameInterval = media->GetFrameInterval();
    int lifetimeFrames = (1.1 * GetValueCurveIntMax("LifeTime", sLifeTimeDefault, settings, sLifeTimeMin, sLifeTimeMax)) / 100 * frameInterval / 1000; // this is the lifetime in frames
    if (lifetimeFrames == 0) {
        lifetimeFrames = (eff->GetEndTimeMS() - eff->GetStartTimeMS()) / frameInterval;
    }
    lifetimeFrames = std::min(lifetimeFrames, (eff->GetEndTimeMS() - eff->GetStartTimeMS()) / frameInterval);
    // Flow is now in particles-per-second (frame-rate independent),
    // mapped through linear/exponential curve. Estimated steady-state
    // particle count = lifetime_seconds * total_per_sec, which is
    // independent of frame rate.
    auto flowToPerSec = [](int u) -> float {
        if (u <= 500) return (float)u * 4.0f;
        return 2000.0f * std::pow(10.0f, (float)(u - 500) * 0.002f);
    };
    int flow1 = settings.GetBool("E_CHECKBOX_Enabled1", sEnabled1Default) ? GetValueCurveIntMax("Flow1", sFlow1Default, settings, sFlowMin, sFlowMax) : 0;
    int flow2 = settings.GetBool("E_CHECKBOX_Enabled2", sEnabled2Default) ? GetValueCurveIntMax("Flow2", sFlow2Default, settings, sFlowMin, sFlowMax) : 0;
    int flow3 = settings.GetBool("E_CHECKBOX_Enabled3", sEnabled3Default) ? GetValueCurveIntMax("Flow3", sFlow3Default, settings, sFlowMin, sFlowMax) : 0;
    int flow4 = settings.GetBool("E_CHECKBOX_Enabled4", sEnabled4Default) ? GetValueCurveIntMax("Flow4", sFlow4Default, settings, sFlowMin, sFlowMax) : 0;
    const float totalPerSec = flowToPerSec(flow1) + flowToPerSec(flow2) + flowToPerSec(flow3) + flowToPerSec(flow4);
    const float lifetimeSec = (float)lifetimeFrames * (float)frameInterval / 1000.0f;
    int count = (int)(lifetimeSec * totalPerSec);

    if (count > MAX_PARTICLES) {
        res.push_back(fmt::format("    WARN: Liquid effect lifetime * (flow 1 + flow 2 + flow 3 + flow 4) = {} exceeds {}. Particle count will be limited. Model '{}', Start {}", count, MAX_PARTICLES, model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())));
    }

    if (settings.GetInt("E_TEXTCTRL_Size", sSizeDefault) > 1000) {
        res.push_back(fmt::format("    WARN: Liquid effect particle size > 1000 can slow render times significantly. Model '{}', Start {}", model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())));
    }

    return res;
}

void LiquidEffect::Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer)
{
    float oset = buffer.GetEffectTimeIntervalPosition();
    Render(buffer,
           SettingsMap.GetBool("CHECKBOX_TopBarrier", sTopBarrierDefault),
           SettingsMap.GetBool("CHECKBOX_BottomBarrier", sBottomBarrierDefault),
           SettingsMap.GetBool("CHECKBOX_LeftBarrier", sLeftBarrierDefault),
           SettingsMap.GetBool("CHECKBOX_RightBarrier", sRightBarrierDefault),

           GetValueCurveInt("LifeTime", sLifeTimeDefault, SettingsMap, oset, sLifeTimeMin, sLifeTimeMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           SettingsMap.GetBool("CHECKBOX_HoldColor", sHoldColorDefault),
           SettingsMap.GetBool("CHECKBOX_MixColors", sMixColorsDefault),
           SettingsMap.GetInt("TEXTCTRL_Size", sSizeDefault),
           SettingsMap.GetInt("TEXTCTRL_WarmUpTime", sWarmUpTimeDefault),

           SettingsMap.GetBool("CHECKBOX_Enabled1", sEnabled1Default),
           GetValueCurveInt("Direction1", sDirection1Default, SettingsMap, oset, sDirectionMin, sDirectionMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           GetValueCurveInt("X1", sX1Default, SettingsMap, oset, sXMin, sXMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           GetValueCurveInt("Y1", sY1Default, SettingsMap, oset, sYMin, sYMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           GetValueCurveInt("Velocity1", sVelocity1Default, SettingsMap, oset, sVelocityMin, sVelocityMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           GetValueCurveInt("Flow1", sFlow1Default, SettingsMap, oset, sFlowMin, sFlowMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           GetValueCurveInt("Liquid_SourceSize1", sSourceSize1Default, SettingsMap, oset, sSourceSizeMin, sSourceSizeMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           SettingsMap.GetBool("CHECKBOX_FlowMusic1", sFlowMusic1Default),

           SettingsMap.GetBool("CHECKBOX_Enabled2", sEnabled2Default),
           GetValueCurveInt("Direction2", sDirection2Default, SettingsMap, oset, sDirectionMin, sDirectionMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           GetValueCurveInt("X2", sX2Default, SettingsMap, oset, sXMin, sXMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           GetValueCurveInt("Y2", sY2Default, SettingsMap, oset, sYMin, sYMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           GetValueCurveInt("Velocity2", sVelocity2Default, SettingsMap, oset, sVelocityMin, sVelocityMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           GetValueCurveInt("Flow2", sFlow2Default, SettingsMap, oset, sFlowMin, sFlowMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           GetValueCurveInt("Liquid_SourceSize2", sSourceSize2Default, SettingsMap, oset, sSourceSizeMin, sSourceSizeMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           SettingsMap.GetBool("CHECKBOX_FlowMusic2", sFlowMusic2Default),

           SettingsMap.GetBool("CHECKBOX_Enabled3", sEnabled3Default),
           GetValueCurveInt("Direction3", sDirection3Default, SettingsMap, oset, sDirectionMin, sDirectionMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           GetValueCurveInt("X3", sX3Default, SettingsMap, oset, sXMin, sXMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           GetValueCurveInt("Y3", sY3Default, SettingsMap, oset, sYMin, sYMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           GetValueCurveInt("Velocity3", sVelocity3Default, SettingsMap, oset, sVelocityMin, sVelocityMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           GetValueCurveInt("Flow3", sFlow3Default, SettingsMap, oset, sFlowMin, sFlowMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           GetValueCurveInt("Liquid_SourceSize3", sSourceSize3Default, SettingsMap, oset, sSourceSizeMin, sSourceSizeMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           SettingsMap.GetBool("CHECKBOX_FlowMusic3", sFlowMusic3Default),

           SettingsMap.GetBool("CHECKBOX_Enabled4", sEnabled4Default),
           GetValueCurveInt("Direction4", sDirection4Default, SettingsMap, oset, sDirectionMin, sDirectionMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           GetValueCurveInt("X4", sX4Default, SettingsMap, oset, sXMin, sXMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           GetValueCurveInt("Y4", sY4Default, SettingsMap, oset, sYMin, sYMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           GetValueCurveInt("Velocity4", sVelocity4Default, SettingsMap, oset, sVelocityMin, sVelocityMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           GetValueCurveInt("Flow4", sFlow4Default, SettingsMap, oset, sFlowMin, sFlowMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           GetValueCurveInt("Liquid_SourceSize4", sSourceSize4Default, SettingsMap, oset, sSourceSizeMin, sSourceSizeMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
           SettingsMap.GetBool("CHECKBOX_FlowMusic4", sFlowMusic4Default),
           SettingsMap.Get("CHOICE_ParticleType", sParticleTypeDefault),
           SettingsMap.GetInt("TEXTCTRL_Despeckle", sDespeckleDefault),
           GetValueCurveDouble("Liquid_Gravity", sGravityDefault, SettingsMap, oset, sGravityMin, sGravityMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), sGravityDivisor),
           GetValueCurveInt("Liquid_GravityAngle", sGravityAngleDefault, SettingsMap, oset, sGravityAngleMin, sGravityAngleMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS())
        );
}

class LiquidRenderCache : public EffectRenderCache {
public:
    LiquidRenderCache() : _world(nullptr) {};
    virtual ~LiquidRenderCache() {
        if (_world != nullptr) {
            spdlog::debug("[Liquid] Destroying remaining b2World during effect cache teardown");
            delete _world;
        }
    };
    b2World* _world;
    // Sub-particle flow accumulator per emitter. Slider values are
    // user-units that resolve to particles per 10 frames; fractional
    // amounts persist across frames so a low slider produces an
    // occasional emit instead of either nothing or a constant stream.
    float _flowAccumulator[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
};


void LiquidEffect::CreateBarrier(b2World* world, float x, float y, float width, float height)
{
    b2BodyDef groundBodyDef;
    groundBodyDef.position.Set(x, y);
    b2Body* groundBody = world->CreateBody(&groundBodyDef);
    b2PolygonShape groundBox;
    groundBox.SetAsBox(width / 2.0f, height / 2.0f);
    groundBody->CreateFixture((b2Shape*)&groundBox, 0.0f);
}

bool LiquidEffect::LostForever(int x, int y, int w, int h, float gravityX, float gravityY)
{
    // essentially vertical ... at this point it wont come back
    if (gravityX < 0.0001f && gravityX > -0.0001f) {
        if (x < -1 || x > w + 1)
            return true;
    }

    // gravity left and off the screen so it wont come back
    if (gravityX < 0.0001f)
    {
        if (x < -1)
            return true;
    }

    // gravity right and off the screen so it wont come back
    if (gravityX > -0.0001f) {
        if (x > w + 1)
            return true;
    }

    // essentially horizontal
    if (gravityY < 0.0001f && gravityY > -0.0001f) {
        if (y < -1 || y > h + 1)
            return true;
    }

    // gravity down and off the bottom of the screen
    if (gravityY < 0.0001f) {
        if (y < -1)
            return true;
    }

    // gravity up and off the top of the screen
    if (gravityY > -0.0001f) {
        if (y > h + 1)
            return true;
    }

    return false;
}

void LiquidEffect::Draw(RenderBuffer& buffer, b2ParticleSystem* ps, const xlColor& color, bool mixColors, int despeckle, float gravityX, float gravityY)
{
    int32 particleCount = ps->GetParticleCount();
    if (particleCount > 0) {
        const b2Vec2* positionBuffer = ps->GetPositionBuffer();
        const b2ParticleColor* colorBuffer = ps->GetColorBuffer();

        for (int i = 0; i < particleCount; ++i) {
            int x = positionBuffer[i].x;
            int y = positionBuffer[i].y;

            if (LostForever(x, y, buffer.BufferWi, buffer.BufferHt, gravityX, gravityY)) {
                ps->DestroyParticle(i);
            } else {
                if (mixColors && ps->GetColorBuffer()) {
                    auto c = colorBuffer[i].GetColor();
                    buffer.SetPixel(positionBuffer[i].x, positionBuffer[i].y, xlColor(c.r * 255, c.g * 255, c.b * 255));
                } else {
                    buffer.SetPixel(positionBuffer[i].x, positionBuffer[i].y, color);
                }
            }
        }
    }

    if (despeckle > 0) {
        for (int y = 0; y < buffer.BufferHt; ++y) {
            for (int x = 0; x < buffer.BufferWi; ++x) {
                if (buffer.GetPixel(x, y) == xlBLACK) {
                    buffer.SetPixel(x, y, GetDespeckleColor(buffer, x, y, despeckle));
                }
            }
        }
    }
}

xlColor LiquidEffect::GetDespeckleColor(RenderBuffer& buffer, size_t x, size_t y, int despeckle) const
{
    int red = 0;
    int green = 0;
    int blue = 0;
    int count = 0;

    int startx = x - 1;
    if (startx < 0) startx = 0;

    int starty = y - 1;
    if (starty < 0) starty = 0;

    int endx = x + 1;
    if (endx >= buffer.BufferWi) endx = buffer.BufferWi - 1;

    int endy = y + 1;
    if (endy >= buffer.BufferHt) endy = buffer.BufferHt - 1;

    int blacks = 0;

    for (int yy = starty; yy <= endy; ++yy)
    {
        for (int xx = startx; xx <= endx; ++xx)
        {
            if (yy != (int)y || xx != (int)x) // dont evaluate the pixel itself
            {
                xlColor c = buffer.GetPixel(xx, yy);

                // if any surrounding pixel is also black then we return black ... we only despeckly totally surrounded pixels
                if (c == xlBLACK)
                {
                    ++blacks;
                    if (blacks >= despeckle) return xlBLACK;
                }

                red += c.red;
                green += c.green;
                blue += c.blue;
                ++count;
            }
        }
    }

    if (count == 0) {
        return xlBLACK;
    }
    return xlColor(red / count, green / count, blue / count);
}

void LiquidEffect::CreateParticles(b2ParticleSystem* ps, int x, int y, int direction, int velocity, int flow, bool flowMusic, int lifetime, int width, int height, const xlColor& c, const std::string& particleType, bool mixcolors, float audioLevel, int sourceSize, float& flowAccumulator, float dt, int maxParticles)
{
    static const float pi2 = 6.283185307f;
    float posx = (float)x * (float)width / 100.0;
    float posy = (float)y * (float)height / 100.0;

    // Velocity scale factor: user 0..1000 → physical 0..100 buf-u/sec.
    // Previously this was * 10.0, which pushed the LF internal speed
    // clamp threshold (~60) down to user value 6 — only 0.6% of the
    // slider's range produced visibly different motion. The ÷10
    // scaling moves that clamp threshold to user value ~600, giving
    // a usable proportional slider over 0..600. Old sequences are
    // migrated to the new scale by adjustSettings (multiplies stored
    // velocity by 100).
    float velx = (float)velocity / 10.0 * RenderBuffer::cos(pi2 * (float)direction / 360.0);
    float vely = (float)velocity / 10.0 * RenderBuffer::sin(pi2 * (float)direction / 360.0);

    float velVariation = rand01() * 0.1;
    velVariation -= velVariation / 2.0;

    velx -= velx * velVariation;
    vely -= vely * velVariation;

    // if lifetime is 1000 ... then we live for 10 seconds
    float lt = lifetime / 100.0;

    // Flow → particles-per-second curve (frame-rate independent):
    //   slider   0..500  → linear,      0..2000  particles/sec
    //   slider 500..1000 → exponential, 2000..20000 particles/sec
    //                       (2000 * 10^((u-500)/500))
    // The linear half gives fine-grained low-end control (smallest
    // visible step = 4 particles/sec); the exponential half restores
    // dramatic high emit rates without hitting MAX_PARTICLES too fast.
    // Each frame we accumulate flowPerSec × dt (= particles to emit
    // this frame), and emit floor(accumulator) particles, carrying the
    // fractional remainder into the next frame. Result: visual
    // emission rate is identical at any sequence frame rate.
    float flowPerSec;
    if (flow <= 500) {
        flowPerSec = (float)flow * 4.0f;
    } else {
        flowPerSec = 2000.0f * std::pow(10.0f, (float)(flow - 500) * 0.002f);
    }
    if (flowMusic) {
        flowPerSec *= audioLevel;
    }
    flowAccumulator += flowPerSec * dt;
    int count = (int)flowAccumulator;
    flowAccumulator -= (float)count;
    if (count < 0) count = 0;
    count = std::min(count, maxParticles);

    // if we are going to exceed the maximum particles in 2 steps then we need to start flagging the older particles for deletion
    // DestroyOldestParticle does not delete them immediately
    // Use std::max(0,...) so that when count >= maxParticles/2 the threshold doesn't go negative,
    // which would cause the loop to destroy more particles than exist (ACCESS_VIOLATION).
    const int evictThreshold = std::max(0, maxParticles - (2 * count));
    if (ps->GetParticleCount() > evictThreshold) {
        const int toEvict = ps->GetParticleCount() - evictThreshold;
        for (int i = 0; i < toEvict; ++i) {
            ps->DestroyOldestParticle(i, true);
        }
    }

    for (int i = 0; i < count && ps->GetParticleCount() < maxParticles; ++i)
    {
        b2ParticleDef pd;
        if (particleType == "Elastic")
        {
            pd.flags = b2_elasticParticle;
        }
        else if (particleType == "Powder")
        {
            pd.flags = b2_powderParticle;
        }
        else if (particleType == "Tensile")
        {
            pd.flags = b2_tensileParticle;
        }
        else if (particleType == "Spring")
        {
            pd.flags = b2_springParticle;
        }
        else if (particleType == "Viscous")
        {
            pd.flags = b2_viscousParticle;
        }
        else if (particleType == "Static Pressure")
        {
            pd.flags = b2_staticPressureParticle;
        }
        else if (particleType == "Water")
        {
            pd.flags = b2_waterParticle;
        }
        else if (particleType == "Reactive")
        {
            pd.flags = b2_reactiveParticle;
        }
        else if (particleType == "Repulsive")
        {
            pd.flags = b2_repulsiveParticle;
        }

        if (mixcolors)
        {
            pd.flags |= b2_colorMixingParticle;
        }

        pd.color.Set(c.Red(), c.Green(), c.Blue(), 255);

        if (sourceSize == 0) {
            // Randomly pick a position within the emitter's radius.
            const float32 angle = rand01() * 2.0f * b2_pi;

            // Distance from the center of the circle.
            const float32 distance = rand01();
            b2Vec2 positionOnUnitCircle(RenderBuffer::sin(angle), RenderBuffer::cos(angle));

            // Initial position.
            pd.position.Set(
                posx + positionOnUnitCircle.x * distance * 0.5,
                posy + positionOnUnitCircle.y * distance * 0.5);
        } else {
            // Distance from the center of the circle.
            const float32 distance = rand01() * ((float)sourceSize - (float)sourceSize / 2.0);

            float offx = distance * RenderBuffer::cos(pi2 * ((float)direction + 90.0) / 360.0);
            float offy = distance * RenderBuffer::sin(pi2 * ((float)direction + 90.0) / 360.0);

            // Initial position.
            pd.position.Set(posx + (offx * (float)width / 200.0), posy + (offy * (float)height / 200.0));
        }

        // Send it flying
        pd.velocity.x = velx;
        pd.velocity.y = vely;

        // give it a lifetime
        if (lifetime > 0) {
            float randomlt = lt + (lt * 0.2 * rand01()) - (lt *.01);
            pd.lifetime = randomlt;
        }
        ps->CreateParticle(pd);
    }
}

void LiquidEffect::CreateParticleSystem(b2World* world, int lifetime, int size, int maxParticles)
{
    b2ParticleSystemDef particleSystemDef;
    auto particleSystem = world->CreateParticleSystem(&particleSystemDef);
    particleSystem->SetRadius((float)size / 1000.0f);
    particleSystem->SetMaxParticleCount(maxParticles);
    if (lifetime > 0) {
        particleSystem->SetDestructionByAge(true);
    }
}

void LiquidEffect::Step(b2World* world, RenderBuffer &buffer, bool enabled[], int lifetime, const std::string& particleType, bool mixcolors,
    int x1, int y1, int direction1, int velocity1, int flow1, int sourceSize1, bool flowMusic1,
    int x2, int y2, int direction2, int velocity2, int flow2, int sourceSize2, bool flowMusic2,
    int x3, int y3, int direction3, int velocity3, int flow3, int sourceSize3, bool flowMusic3,
    int x4, int y4, int direction4, int velocity4, int flow4, int sourceSize4, bool flowMusic4, float time,
    float flowAccumulators[4], int maxParticles
)
{
    // dt is the simulation time advanced per Box2D Step call, in
    // seconds. Box2D uses this to scale all forces and integrations,
    // so velocity (buf-u/sec) and gravity (buf-u/sec²) produce the
    // same visual motion regardless of sequence frame rate.
    float32 timeStep = (float)buffer.frameTimeInMs / 1000.0f;
    int32 velocityIterations = 6;
    int32 positionIterations = 2;
    // Adapt particle iterations so the CFL velocity clamp stays at
    // ~100 buffer-units/sec at any frame rate:
    //   criticalVelocity = particleDiameter * particleIterations / timeStep
    // With diameter = 1.0, choose particleIterations = ceil(timeStep × 100).
    //   20fps (0.050s) → 5
    //   40fps (0.025s) → 3
    //   60fps (0.0167s) → 2
    //   100fps (0.010s) → 1
    // CPU cost stays roughly constant per second of simulation: at
    // higher fps each frame is cheaper but there are more frames.
    int32 particleIterations = std::max(1, (int)std::ceil(timeStep * 100.0f));
    world->Step(timeStep, velocityIterations, positionIterations, particleIterations);

    // create new particles
    b2ParticleSystem* ps = world->GetParticleSystemList();
    if (ps != nullptr) {
        float audioLevel = 0.0001f;
        if (buffer.GetMedia() != nullptr) {
            auto pf = buffer.GetMedia()->GetFrameData(buffer.curPeriod, "");
            if (pf != nullptr) {
                audioLevel = pf->max;
            }
        }

        int j = 0;
        for (int i = 0; i < 4; ++i) {
            if (enabled[i]) {
                xlColor color;
                buffer.palette.GetColor(j % buffer.GetColorCount(), color, time);

                switch (i) {
                case 0:
                    CreateParticles(ps, x1, y1, direction1, velocity1, flow1, flowMusic1, lifetime, buffer.BufferWi, buffer.BufferHt, color, particleType, mixcolors, audioLevel, sourceSize1, flowAccumulators[0], timeStep, maxParticles);
                    break;
                case 1:
                    CreateParticles(ps, x2, y2, direction2, velocity2, flow2, flowMusic2, lifetime, buffer.BufferWi, buffer.BufferHt, color, particleType, mixcolors, audioLevel, sourceSize2, flowAccumulators[1], timeStep, maxParticles);
                    break;
                case 2:
                    CreateParticles(ps, x3, y3, direction3, velocity3, flow3, flowMusic3, lifetime, buffer.BufferWi, buffer.BufferHt, color, particleType, mixcolors, audioLevel, sourceSize3, flowAccumulators[2], timeStep, maxParticles);
                    break;
                case 3:
                    CreateParticles(ps, x4, y4, direction4, velocity4, flow4, flowMusic4, lifetime, buffer.BufferWi, buffer.BufferHt, color, particleType, mixcolors, audioLevel, sourceSize4, flowAccumulators[3], timeStep, maxParticles);
                    break;
                }
                ++j;
            }
        }
    }
}

void LiquidEffect::Render(RenderBuffer &buffer,
    bool top, bool bottom, bool left, bool right,
    int lifetime, bool holdcolor, bool mixcolors, int size, int warmUpTime,
    bool enabled1, int direction1, int x1, int y1, int velocity1, int flow1, int sourceSize1, bool flowMusic1,
    bool enabled2, int direction2, int x2, int y2, int velocity2, int flow2, int sourceSize2, bool flowMusic2,
    bool enabled3, int direction3, int x3, int y3, int velocity3, int flow3, int sourceSize3, bool flowMusic3,
    bool enabled4, int direction4, int x4, int y4, int velocity4, int flow4, int sourceSize4, bool flowMusic4,
    const std::string& particleType, int despeckle, float gravity, int gravityAngle)
{
    bool enabled[4];
    enabled[0] = enabled1;
    enabled[1] = enabled2;
    enabled[2] = enabled3;
    enabled[3] = enabled4;

    LiquidRenderCache *cache = (LiquidRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new LiquidRenderCache();
        buffer.infoCache[id] = cache;
    }
    b2World*& _world = cache->_world;

    float gravityX = gravity * std::cos(toRadians(360 - (gravityAngle + 90)));
    float gravityY = gravity * std::sin(toRadians(360 - (gravityAngle + 90)));

    b2Vec2 grav(gravityX, gravityY);

    const int effectiveMax = std::min(MAX_PARTICLES, std::max(100, buffer.BufferWi * buffer.BufferHt));

    if (buffer.needToInit) {
        buffer.needToInit = false;

        if (_world != nullptr) {
            delete _world;
            _world = nullptr;
        }

        spdlog::trace("[Liquid] INIT '{}' ({}x{}) maxParticles={}",
                      buffer.GetModelName(), buffer.BufferWi, buffer.BufferHt, effectiveMax);

        _world = new b2World(grav);

        if (bottom) {
            CreateBarrier(_world, (float)buffer.BufferWi / 2.0, -1.0f, (float)buffer.BufferWi, 0.001f);
        }
        if (top) {
            CreateBarrier(_world, (float)buffer.BufferWi / 2.0, buffer.BufferHt + 1.0f, (float)buffer.BufferWi, 0.001f);
        }
        if (left) {
            CreateBarrier(_world, -1.0f, (float)buffer.BufferHt / 2.0f, 0.001f, (float)buffer.BufferHt);
        }
        if (right) {
            CreateBarrier(_world, (float)buffer.BufferWi + 1.0f, (float)buffer.BufferHt / 2.0f, 0.001f, (float)buffer.BufferHt);
        }

        CreateParticleSystem(_world, lifetime, size, effectiveMax);

        // Convert warmUpTime (hundredths of seconds) to a frame count
        // at the current sequence frame rate. e.g. 200 (= 2 sec) at
        // 50ms = 40 frames; at 25ms = 80 frames. Frame-rate independent.
        const int warmUpFrames = (buffer.frameTimeInMs > 0)
            ? (warmUpTime * 10 / buffer.frameTimeInMs)
            : 0;
        for (int i = 0; i < warmUpFrames; ++i) {
            Step(_world, buffer, enabled, lifetime, particleType, mixcolors,
                x1, y1, direction1, velocity1, flow1, sourceSize1, flowMusic1,
                x2, y2, direction2, velocity2, flow2, sourceSize2, flowMusic2,
                x3, y3, direction3, velocity3, flow3, sourceSize3, flowMusic3,
                x4, y4, direction4, velocity4, flow4, sourceSize4, flowMusic4, 0.0,
                cache->_flowAccumulator, effectiveMax
            );
        }
    }

    // exit if no world
    if (_world == nullptr) return;

    _world->SetGravity(grav);

    Step(_world, buffer, enabled, lifetime, particleType, mixcolors,
        x1, y1, direction1, velocity1, flow1, sourceSize1, flowMusic1,
        x2, y2, direction2, velocity2, flow2, sourceSize2, flowMusic2,
        x3, y3, direction3, velocity3, flow3, sourceSize3, flowMusic3,
        x4, y4, direction4, velocity4, flow4, sourceSize4, flowMusic4, buffer.GetEffectTimeIntervalPosition(),
        cache->_flowAccumulator, effectiveMax
    );

    b2ParticleSystem* ps = _world->GetParticleSystemList();
    if (ps != nullptr) {
        xlColor color;
        buffer.palette.GetColor(0, color);
        Draw(buffer, ps, color, holdcolor || mixcolors, despeckle, gravityX, gravityY);
    }

    // because of memory usage delete our world when rendered the last frame
    if (buffer.curPeriod == buffer.curEffEndPer) {
        delete _world;
        _world = nullptr;
    }
}
