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

#define FILL_POSITION_MIN 0
#define FILL_POSITION_MAX 100

#define FILL_BANDSIZE_MIN 0
#define FILL_BANDSIZE_MAX 250

#define FILL_SKIPSIZE_MIN 0
#define FILL_SKIPSIZE_MAX 250

#define FILL_OFFSET_MIN 0
#define FILL_OFFSET_MAX 100

class FillEffect : public RenderableEffect
{
public:
    FillEffect(int id);
    virtual ~FillEffect();
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual bool needToAdjustSettings(const std::string& version) override;
    virtual void adjustSettings(const std::string& version, Effect* effect, bool removeDefaults = true) override;
    virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache) override;
    virtual void SetDefaultParameters() override;
    virtual bool CanRenderPartialTimeInterval() const override
    {
        return true;
    }

    virtual double GetSettingVCMin(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Fill_Position")
            return FILL_POSITION_MIN;
        if (name == "E_VALUECURVE_Fill_Band_Size")
            return FILL_BANDSIZE_MIN;
        if (name == "E_VALUECURVE_Fill_Skip_Size")
            return FILL_SKIPSIZE_MIN;
        if (name == "E_VALUECURVE_Fill_Offset")
            return FILL_OFFSET_MIN;

        return RenderableEffect::GetSettingVCMin(name);
    }
    virtual double GetSettingVCMax(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Fill_Position")
            return FILL_POSITION_MAX;
        if (name == "E_VALUECURVE_Fill_Band_Size")
            return FILL_BANDSIZE_MAX;
        if (name == "E_VALUECURVE_Fill_Skip_Size")
            return FILL_SKIPSIZE_MAX;
        if (name == "E_VALUECURVE_Fill_Offset")
            return FILL_OFFSET_MAX;
        return RenderableEffect::GetSettingVCMax(name);
    }

protected:
    virtual xlEffectPanel* CreatePanel(wxWindow* parent) override;
};
