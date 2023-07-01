#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "RenderableEffect.h"

#define TWINKLE_COUNT_MIN 2
#define TWINKLE_COUNT_MAX 100

#define TWINKLE_STEPS_MIN 2
#define TWINKLE_STEPS_MAX 100

class TwinkleEffect : public RenderableEffect
{
public:
    TwinkleEffect(int id);
    virtual ~TwinkleEffect();
    virtual void SetDefaultParameters() override;
    virtual bool needToAdjustSettings(const std::string& version) override;
    virtual void adjustSettings(const std::string& version, Effect* effect, bool removeDefaults = true) override;
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual int DrawEffectBackground(const Effect* e, int x1, int y1, int x2, int y2, xlVertexColorAccumulator& backgrounds, xlColor* colorMask, bool ramps) override;
    virtual double GetSettingVCMin(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Twinkle_Count")
            return TWINKLE_COUNT_MIN;
        if (name == "E_VALUECURVE_Twinkle_Steps")
            return TWINKLE_STEPS_MIN;
        return RenderableEffect::GetSettingVCMin(name);
    }
    virtual double GetSettingVCMax(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Twinkle_Count")
            return TWINKLE_COUNT_MAX;
        if (name == "E_VALUECURVE_Twinkle_Steps")
            return TWINKLE_STEPS_MAX;
        return RenderableEffect::GetSettingVCMax(name);
    }

protected:
    virtual xlEffectPanel* CreatePanel(wxWindow* parent) override;
};
