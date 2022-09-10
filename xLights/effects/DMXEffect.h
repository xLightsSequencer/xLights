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
#include "../UtilFunctions.h"

#define DMX_MIN 0
#define DMX_MAX 255

constexpr int DMX_CHANNELS = 40;

class DMXEffect : public RenderableEffect
{
public:
    DMXEffect(int id);
    virtual ~DMXEffect();
    virtual bool CanBeRandom() override
    {
        return false;
    }
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual void SetPanelStatus(Model* cls) override;
    virtual void SetDefaultParameters() override;
    virtual bool CanRenderPartialTimeInterval() const override
    {
        return true;
    }
    void RemapSelectedDMXEffectValues(Effect* effect, const std::vector<std::tuple<int, int, float, int>>& dmxmappings) const;

    virtual double GetSettingVCMin(const std::string& name) const override
    {
        if (StartsWith(name, "E_VALUECURVE_DMX"))
            return DMX_MIN;
        return RenderableEffect::GetSettingVCMin(name);
    }
    virtual double GetSettingVCMax(const std::string& name) const override
    {
        if (StartsWith(name, "E_VALUECURVE_DMX"))
            return DMX_MAX;
        return RenderableEffect::GetSettingVCMax(name);
    }

protected:
    virtual xlEffectPanel* CreatePanel(wxWindow* parent) override;
    virtual bool needToAdjustSettings(const std::string& version) override
    {
        return true;
    };
    virtual void adjustSettings(const std::string& version, Effect* effect, bool removeDefaults = true) override;

private:
    bool SetDMXSinglColorPixel(int chan, int num_channels, const SettingsMap& SettingsMap, double eff_pos, xlColor& color, RenderBuffer& buffer);
    bool SetDMXRGBNode(int node, int num_channels, const SettingsMap& SettingsMap, double eff_pos, xlColor& color, RenderBuffer& buffer, const std::string& string_type);
    void SetColorBasedOnStringType(int value, int slot, xlColor& color, const std::string& string_type);
};

