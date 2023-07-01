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

class xlColor;

#define LIGHTNING_TOPX_MIN -50
#define LIGHTNING_TOPX_MAX 50

#define LIGHTNING_TOPY_MIN 0
#define LIGHTNING_TOPY_MAX 100

#define LIGHTNING_BOLTS_MIN 1
#define LIGHTNING_BOLTS_MAX 50

#define LIGHTNING_SEGMENTS_MIN 1
#define LIGHTNING_SEGMENTS_MAX 20

class LightningEffect : public RenderableEffect
{
public:
    LightningEffect(int id);
    virtual ~LightningEffect();
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
        if (name == "E_VALUECURVE_Lightning_TopX")
            return LIGHTNING_TOPX_MIN;
        if (name == "E_VALUECURVE_Lightning_TopY")
            return LIGHTNING_TOPY_MIN;
        if (name == "E_VALUECURVE_Number_Bolts")
            return LIGHTNING_BOLTS_MIN;
        if (name == "E_VALUECURVE_Number_Segments")
            return LIGHTNING_SEGMENTS_MIN;
        return RenderableEffect::GetSettingVCMin(name);
    }

    virtual double GetSettingVCMax(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Lightning_TopX")
            return LIGHTNING_TOPX_MAX;
        if (name == "E_VALUECURVE_Lightning_TopY")
            return LIGHTNING_TOPY_MAX;
        if (name == "E_VALUECURVE_Number_Bolts")
            return LIGHTNING_BOLTS_MAX;
        if (name == "E_VALUECURVE_Number_Segments")
            return LIGHTNING_SEGMENTS_MAX;
        return RenderableEffect::GetSettingVCMax(name);
    }

    virtual bool needToAdjustSettings(const std::string& version) override;

    virtual void adjustSettings(const std::string& version, Effect* effect, bool removeDefaults = true) override;

protected:
    virtual xlEffectPanel* CreatePanel(wxWindow* parent) override;
};
