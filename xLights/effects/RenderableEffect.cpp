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
#include <format>

#include "../render/Effect.h"
#include "../render/RenderBuffer.h"
#include "../render/SequenceElements.h"
#include "../render/ValueCurve.h"
#include "../UtilFunctions.h"
#include "EffectManager.h"

#include "../BufferPanel.h"
#include "../ColorPanel.h"
#include "../TimingPanel.h"
#include "../xLightsApp.h"
#include "../xLightsMain.h"
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
        SubModel * m = dynamic_cast<SubModel*>(xLightsApp::GetFrame()->GetModel(mn));
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
        res.push_back(std::format("    WARN: Effect using legacy buffer format '{}' which will be removed in the future. Model '{}', Start {}",
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

    for (int i = 0; i < (int)mSequenceElements->GetElementCount(); i++) {
        Element* e = mSequenceElements->GetElement(i);
        if (e->GetType() == ElementType::ELEMENT_TYPE_TIMING && e->GetName() == timingtrack) {
            return e->GetEffectLayer(0);
        }
    }
    return nullptr;
}

std::string RenderableEffect::GetTimingTracks(const int max, const int equals) const
{
    std::string timingtracks = "";
    for (size_t i = 0; i < mSequenceElements->GetElementCount(); i++)
    {
        Element* e = mSequenceElements->GetElement(i);
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
                if (ColorPanel::GetSettingVCDivisor(name) != 0xFFFF) {
                    min = ColorPanel::GetSettingVCMin(name);
                    max = ColorPanel::GetSettingVCMax(name);
                    div = ColorPanel::GetSettingVCDivisor(name);
                    doit = true;
                }
            } else if (StartsWith(name, "T_VALUECURVE")) {
                if (TimingPanel::GetSettingVCDivisor(name) != 0xFFFF) {
                    min = TimingPanel::GetSettingVCMin(name);
                    max = TimingPanel::GetSettingVCMax(name);
                    div = TimingPanel::GetSettingVCDivisor(name);
                    doit = true;
                }
            } else if (StartsWith(name, "B_VALUECURVE")) {
                if (BufferPanel::GetSettingVCDivisor(name) != 0xFFFF) {
                    min = BufferPanel::GetSettingVCMin(name);
                    max = BufferPanel::GetSettingVCMax(name);
                    div = BufferPanel::GetSettingVCDivisor(name);
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
