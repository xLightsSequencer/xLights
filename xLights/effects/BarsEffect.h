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

#define BARCOUNT_MIN 1
//#define BARCOUNT_MAX 50
#define BARCOUNT_MAX 5

#define BARCYCLES_MIN 0
//#define BARCYCLES_MAX 500
#define BARCYCLES_MAX 300

#define BARCENTER_MIN -100
#define BARCENTER_MAX 100

class BarsEffect : public RenderableEffect
{
public:
    BarsEffect(int id);
    virtual ~BarsEffect();
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
        if (name == "E_VALUECURVE_Bars_BarCount")
            return BARCOUNT_MIN;
        if (name == "E_VALUECURVE_Bars_Cycles")
            return BARCYCLES_MIN;
        if (name == "E_VALUECURVE_Bars_Center")
            return BARCENTER_MIN;
        return RenderableEffect::GetSettingVCMin(name);
    }
    virtual double GetSettingVCMax(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Bars_BarCount")
            return BARCOUNT_MAX;
        if (name == "E_VALUECURVE_Bars_Cycles")
            return BARCYCLES_MAX;
        if (name == "E_VALUECURVE_Bars_Center")
            return BARCENTER_MAX;
        return RenderableEffect::GetSettingVCMax(name);
    }

protected:
    virtual xlEffectPanel* CreatePanel(wxWindow* parent) override;
    void GetSpatialColor(xlColor& color, size_t colorIndex, float x, float y, RenderBuffer& buffer, bool gradient, const xlColor& highlightColor, bool highlight, bool show3d, int BarHt, int n, float pct, int color2Index);
};
