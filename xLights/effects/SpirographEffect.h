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

#define SPIROGRAPH_ANIMATE_MIN -50
#define SPIROGRAPH_ANIMATE_MAX 50

#define SPIROGRAPH_LENGTH_MIN 0
#define SPIROGRAPH_LENGTH_MAX 50

#define SPIROGRAPH_WIDTH_MIN 1
#define SPIROGRAPH_WIDTH_MAX 50

#define SPIROGRAPH_R_MIN 1
#define SPIROGRAPH_R_MAX 100

#define SPIROGRAPH_r_MIN 1
#define SPIROGRAPH_r_MAX 100

#define SPIROGRAPH_SPEED_MIN 0
#define SPIROGRAPH_SPEED_MAX 50

#define SPIROGRAPH_d_MIN 1
#define SPIROGRAPH_d_MAX 100

class SpirographEffect : public RenderableEffect
{
public:
    SpirographEffect(int id);
    virtual ~SpirographEffect();
    virtual void SetDefaultParameters() override;
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual bool AppropriateOnNodes() const override
    {
        return false;
    }
    virtual bool CanRenderPartialTimeInterval() const override
    {
        return true;
    }

    virtual double GetSettingVCMin(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Spirograph_Animate")
            return SPIROGRAPH_ANIMATE_MIN;
        if (name == "E_VALUECURVE_Spirograph_Length")
            return SPIROGRAPH_LENGTH_MIN;
        if (name == "E_VALUECURVE_Spirograph_Width")
            return SPIROGRAPH_WIDTH_MIN;
        if (name == "E_VALUECURVE_Spirograph_R")
            return SPIROGRAPH_R_MIN;
        if (name == "E_VALUECURVE_Spirograph_r")
            return SPIROGRAPH_r_MIN;
        if (name == "E_VALUECURVE_Spirograph_Speed")
            return SPIROGRAPH_SPEED_MIN;
        if (name == "E_VALUECURVE_Spirograph_d")
            return SPIROGRAPH_d_MIN;
        return RenderableEffect::GetSettingVCMin(name);
    }

    virtual double GetSettingVCMax(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Spirograph_Animate")
            return SPIROGRAPH_ANIMATE_MAX;
        if (name == "E_VALUECURVE_Spirograph_Length")
            return SPIROGRAPH_LENGTH_MAX;
        if (name == "E_VALUECURVE_Spirograph_Width")
            return SPIROGRAPH_WIDTH_MAX;
        if (name == "E_VALUECURVE_Spirograph_R")
            return SPIROGRAPH_R_MAX;
        if (name == "E_VALUECURVE_Spirograph_r")
            return SPIROGRAPH_r_MAX;
        if (name == "E_VALUECURVE_Spirograph_Speed")
            return SPIROGRAPH_SPEED_MAX;
        if (name == "E_VALUECURVE_Spirograph_d")
            return SPIROGRAPH_d_MAX;
        return RenderableEffect::GetSettingVCMax(name);
    }

protected:
    virtual xlEffectPanel* CreatePanel(wxWindow* parent) override;
};
