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

#define PLASMA_SPEED_MIN 0
#define PLASMA_SPEED_MAX 100

class PlasmaEffect : public RenderableEffect
{
public:
    PlasmaEffect(int id);
    virtual ~PlasmaEffect();
    virtual void SetDefaultParameters() override;
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual bool CanRenderPartialTimeInterval() const override
    {
        return true;
    }
    virtual bool SupportsRenderCache(const SettingsMap& settings) const override
    {
        return true;
    }

    virtual double GetSettingVCMin(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Plasma_Speed")
            return PLASMA_SPEED_MIN;
        return RenderableEffect::GetSettingVCMin(name);
    }
    virtual double GetSettingVCMax(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Plasma_Speed")
            return PLASMA_SPEED_MAX;
        return RenderableEffect::GetSettingVCMax(name);
    }

    static int GetPlasmaColorScheme(const std::string &ColorSchemeStr);

protected:
    virtual xlEffectPanel* CreatePanel(wxWindow* parent) override;
};
