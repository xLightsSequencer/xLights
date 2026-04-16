/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "RenderableEffect.h"

#include <cstdlib>
#include <spdlog/fmt/fmt.h>
#include <nlohmann/json.hpp>

#include "../render/Effect.h"
#include "../render/RenderBuffer.h"
#include "../render/SequenceElements.h"
#include "../render/ValueCurve.h"
#include "UtilFunctions.h"
#include "EffectManager.h"

#include "../render/ValueCurveConsts.h"
#include "../render/RenderContext.h"
#include "../models/SubModel.h"

RenderableEffect::RenderableEffect(int i, std::string n,
                                   const char **data16,
                                   const char **data24,
                                   const char **data32,
                                   const char **data48,
                                   const char **data64)
    : name(n), tooltip(n), id(i), mSequenceElements(nullptr),
      iconData{data16, data24, data32, data48, data64}
{
}

RenderableEffect::~RenderableEffect()
{
    //dtor
}

void RenderableEffect::SetMetadata(nlohmann::json md)
{
    mMetadata = std::move(md);
    // Give subclasses a chance to cache defaults/min/max into plain member
    // variables so Render() never has to touch the JSON.
    OnMetadataLoaded();
}

const nlohmann::json& RenderableEffect::GetMetadata() const
{
    return mMetadata;
}

bool RenderableEffect::HasMetadata() const
{
    return !mMetadata.is_null() && !mMetadata.empty();
}

const nlohmann::json* RenderableEffect::GetPropertyMetadata(const std::string& propId) const
{
    if (!HasMetadata()) {
        return nullptr;
    }
    auto propsIt = mMetadata.find("properties");
    if (propsIt == mMetadata.end() || !propsIt->is_array()) {
        return nullptr;
    }
    for (const auto& prop : *propsIt) {
        auto idIt = prop.find("id");
        if (idIt != prop.end() && idIt->is_string() && idIt->get<std::string>() == propId) {
            return &prop;
        }
    }
    return nullptr;
}

int RenderableEffect::GetIntDefault(const std::string& propId, int fallback) const
{
    const nlohmann::json* prop = GetPropertyMetadata(propId);
    if (prop == nullptr) {
        return fallback;
    }
    auto it = prop->find("default");
    if (it == prop->end() || !it->is_number()) {
        return fallback;
    }
    return it->get<int>();
}

double RenderableEffect::GetDoubleDefault(const std::string& propId, double fallback) const
{
    const nlohmann::json* prop = GetPropertyMetadata(propId);
    if (prop == nullptr) {
        return fallback;
    }
    auto it = prop->find("default");
    if (it == prop->end() || !it->is_number()) {
        return fallback;
    }
    return it->get<double>();
}

bool RenderableEffect::GetBoolDefault(const std::string& propId, bool fallback) const
{
    const nlohmann::json* prop = GetPropertyMetadata(propId);
    if (prop == nullptr) {
        return fallback;
    }
    auto it = prop->find("default");
    if (it == prop->end() || !it->is_boolean()) {
        return fallback;
    }
    return it->get<bool>();
}

std::string RenderableEffect::GetStringDefault(const std::string& propId, const std::string& fallback) const
{
    const nlohmann::json* prop = GetPropertyMetadata(propId);
    if (prop == nullptr) {
        return fallback;
    }
    auto it = prop->find("default");
    if (it == prop->end() || !it->is_string()) {
        return fallback;
    }
    return it->get<std::string>();
}

double RenderableEffect::GetMinFromMetadata(const std::string& propId, double fallback) const
{
    const nlohmann::json* prop = GetPropertyMetadata(propId);
    if (prop == nullptr) {
        return fallback;
    }
    auto it = prop->find("min");
    if (it == prop->end() || !it->is_number()) {
        return fallback;
    }
    return it->get<double>();
}

double RenderableEffect::GetMaxFromMetadata(const std::string& propId, double fallback) const
{
    const nlohmann::json* prop = GetPropertyMetadata(propId);
    if (prop == nullptr) {
        return fallback;
    }
    auto it = prop->find("max");
    if (it == prop->end() || !it->is_number()) {
        return fallback;
    }
    return it->get<double>();
}

int RenderableEffect::GetDivisorFromMetadata(const std::string& propId, int fallback) const
{
    const nlohmann::json* prop = GetPropertyMetadata(propId);
    if (prop == nullptr) {
        return fallback;
    }
    auto it = prop->find("divisor");
    if (it == prop->end() || !it->is_number()) {
        return fallback;
    }
    return it->get<int>();
}

double RenderableEffect::GetVCMinFromMetadata(const std::string& propId, double fallback) const
{
    const nlohmann::json* prop = GetPropertyMetadata(propId);
    if (prop == nullptr) {
        return fallback;
    }
    auto it = prop->find("vcMin");
    if (it != prop->end() && it->is_number()) {
        return it->get<double>();
    }
    it = prop->find("min");
    if (it != prop->end() && it->is_number()) {
        return it->get<double>();
    }
    return fallback;
}

double RenderableEffect::GetVCMaxFromMetadata(const std::string& propId, double fallback) const
{
    const nlohmann::json* prop = GetPropertyMetadata(propId);
    if (prop == nullptr) {
        return fallback;
    }
    auto it = prop->find("vcMax");
    if (it != prop->end() && it->is_number()) {
        return it->get<double>();
    }
    it = prop->find("max");
    if (it != prop->end() && it->is_number()) {
        return it->get<double>();
    }
    return fallback;
}

// Strips the "E_VALUECURVE_" prefix (used by UpgradeValueCurve to identify the
// value-curve setting key) and looks up the matching property in metadata.
// Returns nullptr if the prefix is absent or the property is not described.
static const nlohmann::json* LookupVCProperty(const RenderableEffect* eff, const std::string& name)
{
    constexpr std::string_view kPrefix = "E_VALUECURVE_";
    if (name.size() <= kPrefix.size() || name.compare(0, kPrefix.size(), kPrefix) != 0) {
        return nullptr;
    }
    return eff->GetPropertyMetadata(name.substr(kPrefix.size()));
}

double RenderableEffect::GetSettingVCMin(const std::string& name) const
{
    const nlohmann::json* prop = LookupVCProperty(this, name);
    if (prop != nullptr) {
        // Prefer vcMin — a property can expose a narrower VC range than its
        // slider range (see Twinkle_Steps: slider 2..400, VC 2..100).
        auto it = prop->find("vcMin");
        if (it != prop->end() && it->is_number()) {
            return it->get<double>();
        }
        it = prop->find("min");
        if (it != prop->end() && it->is_number()) {
            return it->get<double>();
        }
    }
    assert(false);
    return 0.0;
}

double RenderableEffect::GetSettingVCMax(const std::string& name) const
{
    const nlohmann::json* prop = LookupVCProperty(this, name);
    if (prop != nullptr) {
        auto it = prop->find("vcMax");
        if (it != prop->end() && it->is_number()) {
            return it->get<double>();
        }
        it = prop->find("max");
        if (it != prop->end() && it->is_number()) {
            return it->get<double>();
        }
    }
    assert(false);
    return 100.0;
}

int RenderableEffect::GetSettingVCDivisor(const std::string& name) const
{
    const nlohmann::json* prop = LookupVCProperty(this, name);
    if (prop != nullptr) {
        auto it = prop->find("divisor");
        if (it != prop->end() && it->is_number()) {
            return it->get<int>();
        }
    }
    return 1;
}

int RenderableEffect::DrawEffectBackground(const Effect *e, int x1, int y1, int x2, int y2,
                                           xlVertexColorAccumulator &background, xlColor* colorMask, bool ramps) {
    if (e->HasBackgroundDisplayList()) {
        e->GetBackgroundDisplayList().addToAccumulator(x1, y1, x2-x1, y2-y1, background);
        return e->GetBackgroundDisplayList().iconSize;
    }
    return 1;
}


// return true if version string is older than compare string
bool RenderableEffect::IsVersionOlder(const std::string& compare, const std::string& version)
{
    return ::IsVersionOlder(compare, version);
}

bool RenderableEffect::SupportsRenderCache(const SettingsMap& settings) const
{
    for (const auto& it : settings.keys()) {
        // we want to cache blur because of compute cost
        if (Contains(it, "SLIDER_Blur") ||
            Contains(it, "VALUECURVE_Blur")) {
            return true;
        }
        
        // we want to cache rotations because of compute cost
        if (Contains(it, "VALUECURVE_Rotations")) {
            return true;
        }
    }
    return false;
}

bool RenderableEffect::needToAdjustSettings(const std::string &version) {
    return IsVersionOlder("2024.05", version);
}

void RenderableEffect::adjustSettings(const std::string &version, Effect *effect, bool removeDefaults) {
    if (IsVersionOlder("2024.05", version)) {
        std::string mn = effect->GetParentEffectLayer()->GetParentElement()->GetFullName();
        auto* ctx = effect->GetParentEffectLayer()->GetParentElement()->GetSequenceElements()->GetRenderContext();
        SubModel * m = dynamic_cast<SubModel*>(ctx->GetModel(mn));
        if (m != nullptr) {
            uint32_t mx = 0;
            for (size_t x = 0; x < m->GetNodeCount(); ++x) {
                mx = std::max(m->GetNode(x)->Coords.size(), mx);
            }
            if (mx > 1) {
                // SubModels with duplicate nodes using "Single Line" on old effects
                // need to use a legacy Single Line style
                std::string bs = effect->GetSettings().Get("B_CHOICE_BufferStyle", "Default");
                if (bs == "Single Line") {
                    effect->GetSettings()["B_CHOICE_BufferStyle"] = "** Single Line Legacy";
                }
            }
        }
    }
}
std::list<std::string> RenderableEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache)
{
    std::list<std::string> res;
    if (settings.Get("B_CHOICE_BufferStyle", "").starts_with("** ")) {
        res.push_back(fmt::format("    WARN: Effect using legacy buffer format '{}' which will be removed in the future. Model '{}', Start {}",
                                  settings.Get("B_CHOICE_BufferStyle", ""), model->GetFullName(),
                                  FORMATTIME(eff->GetStartTimeMS())));
    }
    return res;
};


double RenderableEffect::GetValueCurveDouble(const std::string &name, double def, const SettingsMap &SettingsMap, float offset, double min, double max, long startMS, long endMS, int divisor)
{
    double res = def;
    const std::string vn = "VALUECURVE_" + name;
    const std::string &vc = SettingsMap.Get(vn, xlEMPTY_STRING);
    if (vc != xlEMPTY_STRING) {
        ValueCurve valc(vc);
        if (valc.IsActive()) {
            valc.SetLimits(min, max);
            valc.SetDivisor(divisor);
            return valc.GetOutputValueAtDivided(offset, startMS, endMS);
        }
    }
    
    const std::string sn = "SLIDER_" + name;
    const std::string tn = "TEXTCTRL_" + name;
    if (SettingsMap.Contains(sn)) {
        res = SettingsMap.GetDouble(sn, def);
    } else if (SettingsMap.Contains(tn)) {
        res = SettingsMap.GetDouble(tn, def);
    }
    return res;
}

int RenderableEffect::GetValueCurveIntMax(const std::string& name, int def, const SettingsMap& SettingsMap, int min, int max, int divisor)
{
    int res = def;

    const std::string vn = "E_VALUECURVE_" + name;
    if (SettingsMap.Contains(vn)) {
        const std::string& vc = SettingsMap.Get(vn, xlEMPTY_STRING);

        ValueCurve valc;
        valc.SetDivisor(divisor);
        valc.SetLimits(min, max);
        valc.Deserialise(vc);
        if (valc.IsActive()) {
            return valc.GetMaxValueDivided();
        }
    }

    const std::string sn = "E_SLIDER_" + name;
    const std::string tn = "E_TEXTCTRL_" + name;
    // bool slider = false;
    if (SettingsMap.Contains(sn)) {
        res = SettingsMap.GetInt(sn, def);
        // slider = true;
    } else if (SettingsMap.Contains(tn)) {
        res = SettingsMap.GetInt(tn, def);
    }
    return res;
}

int RenderableEffect::GetValueCurveInt(const std::string &name, int def, const SettingsMap &SettingsMap, float offset, int min, int max, long startMS, long endMS, int divisor)
{
    int res = def;
    const std::string vn = "VALUECURVE_" + name;
    if (SettingsMap.Contains(vn)) {
        const std::string &vc = SettingsMap.Get(vn, xlEMPTY_STRING);

        ValueCurve valc;
        valc.SetDivisor(divisor);
        valc.SetLimits(min, max);
        valc.Deserialise(vc);
        if (valc.IsActive()) {
            return valc.GetOutputValueAt(offset, startMS, endMS);
        }
    }
    const std::string sn = "SLIDER_" + name;
    const std::string tn = "TEXTCTRL_" + name;
    //bool slider = false;
    if (SettingsMap.Contains(sn)) {
        res = SettingsMap.GetInt(sn, def);
        //slider = true;
    } else if (SettingsMap.Contains(tn)) {
        res = SettingsMap.GetInt(tn, def);
    }
    return res;
}

EffectLayer* RenderableEffect::GetTiming(const std::string& timingtrack) const
{
    if (timingtrack == "") return nullptr;

    auto* seqEl = GetSequenceElements();
    if (!seqEl) return nullptr;
    for (int i = 0; i < (int)seqEl->GetElementCount(); i++) {
        Element* e = seqEl->GetElement(i);
        if (e->GetType() == ElementType::ELEMENT_TYPE_TIMING && e->GetName() == timingtrack) {
            return e->GetEffectLayer(0);
        }
    }
    return nullptr;
}

SequenceElements* RenderableEffect::GetSequenceElements(RenderBuffer& buffer) const {
    if (buffer.renderContext) {
        return &buffer.renderContext->GetSequenceElements();
    }
    return mSequenceElements;
}

std::string RenderableEffect::GetTimingTracks(const int max, const int equals) const
{
    std::string timingtracks = "";
    auto* seqEl = GetSequenceElements();
    if (!seqEl) return timingtracks;
    for (size_t i = 0; i < seqEl->GetElementCount(); i++)
    {
        Element* e = seqEl->GetElement(i);
        if (e->GetType() == ElementType::ELEMENT_TYPE_TIMING && (max < 1 || (int)e->GetEffectLayerCount() <= max) && (equals == 0 || (int)e->GetEffectLayerCount() == equals))
        {
            if (timingtracks != "")
            {
                timingtracks += "|";
            }
            timingtracks += e->GetName();
        }
    }
    return timingtracks;
}

Effect* RenderableEffect::GetCurrentTiming(const RenderBuffer& buffer, const std::string& timingtrack) const
{
    EffectLayer* el = GetTiming(timingtrack);

    if (el == nullptr) return nullptr;

    int currentMS = buffer.curPeriod * buffer.frameTimeInMs;
    for (int j = 0; j < el->GetEffectCount(); j++)
    {
        if (el->GetEffect(j)->GetStartTimeMS() <= currentMS &&
            el->GetEffect(j)->GetEndTimeMS() > currentMS)
        {
            return el->GetEffect(j);
        }
    }

    return nullptr;
}

// Upgrades any value curve where not stored as real values or the min/max/divisor has changed since the file was saved
std::string RenderableEffect::UpgradeValueCurve(EffectManager* effectManager, const std::string& name, const std::string& value, const std::string& effectName)
{
    // value curve has to be active
    if (value.find("Active=TRUE") != std::string::npos) {
        RenderableEffect* effect = effectManager->GetEffect(effectName);

        if (effect != nullptr) {
            double min = 0;
            double max = 100;
            int div = 1;
            bool doit = false;
            if (StartsWith(name, "E_VALUECURVE")) {
                // if divisor is 0xFFFF then the curve does not allow upgrading as the min/max/divisor dont come from xLights - mostly used in shaders
                if (effect->GetSettingVCDivisor(name) != 0xFFFF) {
                    min = effect->GetSettingVCMin(name);
                    max = effect->GetSettingVCMax(name);
                    div = effect->GetSettingVCDivisor(name);
                    doit = true;
                }
            } else if (StartsWith(name, "C_VALUECURVE")) {
                if (ValueCurveConsts::GetColorSettingVCDivisor(name) != 0xFFFF) {
                    min = ValueCurveConsts::GetColorSettingVCMin(name);
                    max = ValueCurveConsts::GetColorSettingVCMax(name);
                    div = ValueCurveConsts::GetColorSettingVCDivisor(name);
                    doit = true;
                }
            } else if (StartsWith(name, "T_VALUECURVE")) {
                if (ValueCurveConsts::GetTimingSettingVCDivisor(name) != 0xFFFF) {
                    min = ValueCurveConsts::GetTimingSettingVCMin(name);
                    max = ValueCurveConsts::GetTimingSettingVCMax(name);
                    div = ValueCurveConsts::GetTimingSettingVCDivisor(name);
                    doit = true;
                }
            } else if (StartsWith(name, "B_VALUECURVE")) {
                if (ValueCurveConsts::GetBufferSettingVCDivisor(name) != 0xFFFF) {
                    min = ValueCurveConsts::GetBufferSettingVCMin(name);
                    max = ValueCurveConsts::GetBufferSettingVCMax(name);
                    div = ValueCurveConsts::GetBufferSettingVCDivisor(name);
                    doit = true;
                }
            }
            if (doit) {
                ValueCurve valc;
                valc.SetLimits(min, max); // now set the limits
                valc.SetDivisor(div);
                valc.Deserialise(value, false);
                return valc.Serialise();
            }
        }
    }

    return value;
}
