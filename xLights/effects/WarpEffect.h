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

#define WARP_X_MIN 0
#define WARP_X_MAX 100

#define WARP_Y_MIN 0
#define WARP_Y_MAX 100

class WarpEffect : public RenderableEffect
{
public:
    enum WarpType {
        WATER_DROPS = 0,
        SINGLE_WATER_DROP,
        CIRCLE_REVEAL,
        BANDED_SWIRL,
        CIRCULAR_SWIRL,
        DISSOLVE,
        RIPPLE,
        DROP,
        WAVY,
        SAMPLE_ON,
        MIRROR,
        COPY,

        COUNT_WARP_STYLES
    };
    WarpType mapWarpType(const std::string& wt);

    WarpEffect(int id);
    virtual ~WarpEffect();
    virtual bool CanBeRandom() override
    {
        return false;
    }
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual bool SupportsLinearColorCurves(const SettingsMap& SettingsMap) const override
    {
        return false;
    }
    virtual void SetDefaultParameters() override;
    virtual bool needToAdjustSettings(const std::string& version) override;
    virtual void adjustSettings(const std::string& version, Effect* effect, bool removeDefaults = true) override;
    virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache) override;

    virtual double GetSettingVCMin(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Warp_X")
            return WARP_X_MIN;
        if (name == "E_VALUECURVE_Warp_Y")
            return WARP_Y_MIN;
        return RenderableEffect::GetSettingVCMin(name);
    }
    virtual double GetSettingVCMax(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Warp_X")
            return WARP_X_MAX;
        if (name == "E_VALUECURVE_Warp_Y")
            return WARP_Y_MAX;
        return RenderableEffect::GetSettingVCMax(name);
    }

protected:
    virtual void RemoveDefaults(const std::string& version, Effect* effect) override;
    virtual xlEffectPanel* CreatePanel(wxWindow* parent) override;
};
