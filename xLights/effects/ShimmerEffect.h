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

#define SHIMMER_CYCLES_MIN 0
#define SHIMMER_CYCLES_MAX 6000

#define SHIMMER_DUTYFACTOR_MIN 1
#define SHIMMER_DUTYFACTOR_MAX 100

class ShimmerEffect : public RenderableEffect
{
public:
    ShimmerEffect(int id);
    virtual ~ShimmerEffect();
    virtual void SetDefaultParameters() override;
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual bool SupportsLinearColorCurves(const SettingsMap& SettingsMap) const override
    {
        return true;
    }
    virtual bool CanRenderPartialTimeInterval() const override
    {
        return true;
    }

    virtual double GetSettingVCMin(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Shimmer_Cycles")
            return SHIMMER_CYCLES_MIN;
        if (name == "E_VALUECURVE_Shimmer_Duty_Factor")
            return SHIMMER_DUTYFACTOR_MIN;
        return RenderableEffect::GetSettingVCMin(name);
    }

    virtual double GetSettingVCMax(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Shimmer_Cycles")
            return SHIMMER_CYCLES_MAX;
        if (name == "E_VALUECURVE_Shimmer_Duty_Factor")
            return SHIMMER_DUTYFACTOR_MAX;
        return RenderableEffect::GetSettingVCMax(name);
    }

protected:
    virtual xlEffectPanel* CreatePanel(wxWindow* parent) override;
    virtual bool needToAdjustSettings(const std::string& version) override;
    virtual void adjustSettings(const std::string& version, Effect* effect, bool removeDefaults = true) override;
};
