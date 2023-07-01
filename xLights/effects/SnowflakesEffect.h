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

#define SNOWFLAKES_COUNT_MIN 1
#define SNOWFLAKES_COUNT_MAX 100

#define SNOWFLAKES_SPEED_MIN 0
#define SNOWFLAKES_SPEED_MAX 50

class SnowflakesEffect : public RenderableEffect
{
public:
    SnowflakesEffect(int id);
    virtual ~SnowflakesEffect();
    virtual void SetDefaultParameters() override;
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;

    virtual double GetSettingVCMin(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Snowflakes_Count")
            return SNOWFLAKES_COUNT_MIN;
        if (name == "E_VALUECURVE_Snowflakes_Speed")
            return SNOWFLAKES_SPEED_MIN;
        return RenderableEffect::GetSettingVCMin(name);
    }

    virtual double GetSettingVCMax(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Snowflakes_Count")
            return SNOWFLAKES_COUNT_MAX;
        if (name == "E_VALUECURVE_Snowflakes_Speed")
            return SNOWFLAKES_SPEED_MAX;
        return RenderableEffect::GetSettingVCMax(name);
    }

protected:
    virtual xlEffectPanel* CreatePanel(wxWindow* parent) override;
    virtual bool needToAdjustSettings(const std::string& version) override;
    virtual void adjustSettings(const std::string& version, Effect* effect, bool removeDefaults = true) override;
    void MoveFlakes(RenderBuffer& buffer, int snowflakeType, const std::string& falling, int count, const xlColor& color1, int& effectState);
};
