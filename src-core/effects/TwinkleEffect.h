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

#include "RenderableEffect.h"

class TwinkleEffect : public RenderableEffect
{
public:
    TwinkleEffect(int id);
    virtual ~TwinkleEffect();
    virtual bool needToAdjustSettings(const std::string& version) override;
    virtual void adjustSettings(const std::string& version, Effect* effect, bool removeDefaults = true) override;
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual int DrawEffectBackground(const Effect* e, int x1, int y1, int x2, int y2, xlVertexColorAccumulator& backgrounds, xlColor* colorMask, bool ramps) override;
    virtual int GetSettingVCDivisor(const std::string& name) const override
    {
        // Return 0xFFFF to opt out of UpgradeValueCurve auto-rescaling for
        // Twinkle_Steps. Old sequences stored Max=100; adjustSettings migrates
        // them to Max=400 while preserving the actual step values.
        if (name == "E_VALUECURVE_Twinkle_Steps")
            return 0xFFFF;
        return RenderableEffect::GetSettingVCDivisor(name);
    }

    // Cached from Twinkle.json by OnMetadataLoaded().
    static int sCountDefault;
    static int sCountMin;
    static int sCountMax;
    static int sStepsDefault;
    // Twinkle_Steps slider and VC both go 2..400 (vcMax=400 in JSON).
    // GetSettingVCDivisor returns 0xFFFF to disable UpgradeValueCurve
    // auto-rescaling; adjustSettings manually migrates old VCs (Max=100)
    // to the new limits (Max=400) without changing their stored values.
    static int sStepsVCMin;
    static int sStepsVCMax;
    static bool sStrobeDefault;
    static bool sReRandomDefault;
    static std::string sStyleDefault;

protected:
    virtual void OnMetadataLoaded() override;
};
