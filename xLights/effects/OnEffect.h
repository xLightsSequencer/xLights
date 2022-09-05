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

#define ON_TRANSPARENCY_MIN 0
#define ON_TRANSPARENCY_MAX 100

class OnEffect : public RenderableEffect
{
public:
    OnEffect(int id);
    virtual ~OnEffect();
    virtual bool CanBeRandom() override
    {
        return false;
    }
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual int DrawEffectBackground(const Effect* e, int x1, int y1, int x2, int y2, xlVertexColorAccumulator& backgrounds, xlColor* colorMask, bool ramps) override;
    virtual bool SupportsLinearColorCurves(const SettingsMap& SettingsMap) const override
    {
        return true;
    }
    virtual bool SupportsRadialColorCurves(const SettingsMap& SettingsMap) const override
    {
        return true;
    }
    virtual void SetDefaultParameters() override;
    virtual wxString GetEffectString() override;
    virtual bool CanRenderPartialTimeInterval() const override
    {
        return true;
    }

    virtual double GetSettingVCMin(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_On_Transparency")
            return ON_TRANSPARENCY_MIN;
        return RenderableEffect::GetSettingVCMin(name);
    }
    virtual double GetSettingVCMax(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_On_Transparency")
            return ON_TRANSPARENCY_MAX;
        return RenderableEffect::GetSettingVCMax(name);
    }

protected:
    virtual void RemoveDefaults(const std::string& version, Effect* effect) override;
    virtual xlEffectPanel* CreatePanel(wxWindow* parent) override;
};
