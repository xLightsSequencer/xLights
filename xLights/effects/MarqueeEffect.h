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

#define MARQUEE_XC_MIN -100
#define MARQUEE_XC_MAX 100

#define MARQUEE_YC_MIN -100
#define MARQUEE_YC_MAX 100

#define MARQUEE_SCALEX_MIN 1
#define MARQUEE_SCALEX_MAX 100

#define MARQUEE_SCALEY_MIN 1
#define MARQUEE_SCALEY_MAX 100

#define MARQUEE_BAND_SIZE_MIN 1
#define MARQUEE_BAND_SIZE_MAX 100

#define MARQUEE_SKIP_SIZE_MIN 0
#define MARQUEE_SKIP_SIZE_MAX 100

#define MARQUEE_SPEED_MIN 0
#define MARQUEE_SPEED_MAX 50

#define MARQUEE_STAGGER_MIN 0
#define MARQUEE_STAGGER_MAX 50

#define MARQUEE_START_MIN 0
#define MARQUEE_START_MAX 100

#define MARQUEE_THICKNESS_MIN 1
#define MARQUEE_THICKNESS_MAX 100

class MarqueeEffect : public RenderableEffect
{
public:
    MarqueeEffect(int id);
    virtual ~MarqueeEffect();
    virtual void SetDefaultParameters() override;
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual bool CanRenderPartialTimeInterval() const override
    {
        return true;
    }

    virtual double GetSettingVCMin(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_MarqueeXC")
            return MARQUEE_XC_MIN;
        if (name == "E_VALUECURVE_MarqueeYC")
            return MARQUEE_YC_MIN;
        if (name == "E_VALUECURVE_Marquee_ScaleX")
            return MARQUEE_SCALEX_MIN;
        if (name == "E_VALUECURVE_Marquee_ScaleY")
            return MARQUEE_SCALEY_MIN;
        if (name == "E_VALUECURVE_Marquee_Band_Size")
            return MARQUEE_BAND_SIZE_MIN;
        if (name == "E_VALUECURVE_Marquee_Skip_Size")
            return MARQUEE_SKIP_SIZE_MIN;
        if (name == "E_VALUECURVE_Marquee_Speed")
            return MARQUEE_SPEED_MIN;
        if (name == "E_VALUECURVE_Marquee_Stagger")
            return MARQUEE_STAGGER_MIN;
        if (name == "E_VALUECURVE_Marquee_Start")
            return MARQUEE_START_MIN;
        if (name == "E_VALUECURVE_Marquee_Thickness")
            return MARQUEE_THICKNESS_MIN;
        return RenderableEffect::GetSettingVCMin(name);
    }

    virtual double GetSettingVCMax(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_MarqueeXC")
            return MARQUEE_XC_MAX;
        if (name == "E_VALUECURVE_MarqueeYC")
            return MARQUEE_YC_MAX;
        if (name == "E_VALUECURVE_Marquee_ScaleX")
            return MARQUEE_SCALEX_MAX;
        if (name == "E_VALUECURVE_Marquee_ScaleY")
            return MARQUEE_SCALEY_MAX;
        if (name == "E_VALUECURVE_Marquee_Band_Size")
            return MARQUEE_BAND_SIZE_MAX;
        if (name == "E_VALUECURVE_Marquee_Skip_Size")
            return MARQUEE_SKIP_SIZE_MAX;
        if (name == "E_VALUECURVE_Marquee_Speed")
            return MARQUEE_SPEED_MAX;
        if (name == "E_VALUECURVE_Marquee_Stagger")
            return MARQUEE_STAGGER_MAX;
        if (name == "E_VALUECURVE_Marquee_Start")
            return MARQUEE_START_MAX;
        if (name == "E_VALUECURVE_Marquee_Thickness")
            return MARQUEE_THICKNESS_MAX;
        return RenderableEffect::GetSettingVCMax(name);
    }

protected:
    virtual xlEffectPanel* CreatePanel(wxWindow* parent) override;
};
