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

#define SPIRALS_COUNT_MIN 1
#define SPIRALS_COUNT_MAX 5

#define SPIRALS_MOVEMENT_MIN -200
#define SPIRALS_MOVEMENT_MAX 200
#define SPIRALS_MOVEMENT_DIVISOR 10

#define SPIRALS_ROTATION_MIN -300
#define SPIRALS_ROTATION_MAX 300
#define SPIRALS_ROTATION_DIVISOR 10

#define SPIRALS_THICKNESS_MIN 0
#define SPIRALS_THICKNESS_MAX 100

class SpiralsEffect : public RenderableEffect
{
public:
    SpiralsEffect(int id);
    virtual ~SpiralsEffect();
    virtual void SetDefaultParameters() override;
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual bool SupportsLinearColorCurves(const SettingsMap& SettingsMap) const override;
    virtual bool CanRenderPartialTimeInterval() const override
    {
        return true;
    }

    virtual double GetSettingVCMin(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Spirals_Count")
            return SPIRALS_COUNT_MIN;
        if (name == "E_VALUECURVE_Spirals_Movement")
            return SPIRALS_MOVEMENT_MIN;
        if (name == "E_VALUECURVE_Spirals_Rotation")
            return SPIRALS_ROTATION_MIN;
        if (name == "E_VALUECURVE_Spirals_Thickness")
            return SPIRALS_THICKNESS_MIN;
        return RenderableEffect::GetSettingVCMin(name);
    }

    virtual double GetSettingVCMax(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Spirals_Count")
            return SPIRALS_COUNT_MAX;
        if (name == "E_VALUECURVE_Spirals_Movement")
            return SPIRALS_MOVEMENT_MAX;
        if (name == "E_VALUECURVE_Spirals_Rotation")
            return SPIRALS_ROTATION_MAX;
        if (name == "E_VALUECURVE_Spirals_Thickness")
            return SPIRALS_THICKNESS_MAX;
        return RenderableEffect::GetSettingVCMax(name);
    }

    virtual int GetSettingVCDivisor(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Spirals_Movement")
            return SPIRALS_MOVEMENT_DIVISOR;
        if (name == "E_VALUECURVE_Spirals_Rotation")
            return SPIRALS_ROTATION_DIVISOR;
        return RenderableEffect::GetSettingVCDivisor(name);
    }

protected:
    virtual xlEffectPanel* CreatePanel(wxWindow* parent) override;
};
