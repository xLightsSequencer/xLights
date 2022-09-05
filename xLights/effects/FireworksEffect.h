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

#define FIREWORKSCOUNT_MIN 1
#define FIREWORKSCOUNT_MAX 100

#define FIREWORKSFADE_MIN 1
#define FIREWORKSFADE_MAX 100

#define FIREWORKSVELOCITY_MIN 1
#define FIREWORKSVELOCITY_MAX 10

#define FIREWORKSXVELOCITY_MIN -100
#define FIREWORKSXVELOCITY_MAX 100

#define FIREWORKSYVELOCITY_MIN -100
#define FIREWORKSYVELOCITY_MAX 100

#define FIREWORKSXLOCATION_MIN -1
#define FIREWORKSXLOCATION_MAX 100

#define FIREWORKSYLOCATION_MIN -1
#define FIREWORKSYLOCATION_MAX 100

class FireworksEffect : public RenderableEffect
{
public:
    FireworksEffect(int id);
    virtual ~FireworksEffect();
    virtual void RenameTimingTrack(std::string oldname, std::string newname, Effect* effect) override;
    virtual void SetDefaultParameters() override;
    virtual void SetPanelStatus(Model* cls) override;
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache) override;
    virtual bool AppropriateOnNodes() const override
    {
        return false;
    }

    virtual double GetSettingVCMin(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Fireworks_Count")
            return FIREWORKSCOUNT_MIN;
        if (name == "E_VALUECURVE_Fireworks_Fade")
            return FIREWORKSFADE_MIN;
        if (name == "E_VALUECURVE_Fireworks_Velocity")
            return FIREWORKSVELOCITY_MIN;
        if (name == "E_VALUECURVE_Fireworks_XVelocity")
            return FIREWORKSXVELOCITY_MIN;
        if (name == "E_VALUECURVE_Fireworks_YVelocity")
            return FIREWORKSYVELOCITY_MIN;
        if (name == "E_VALUECURVE_Fireworks_XLocation")
            return FIREWORKSXLOCATION_MIN;
        if (name == "E_VALUECURVE_Fireworks_YLocation")
            return FIREWORKSYLOCATION_MIN;
        return RenderableEffect::GetSettingVCMin(name);
    }

    virtual double GetSettingVCMax(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Fireworks_Count")
            return FIREWORKSCOUNT_MAX;
        if (name == "E_VALUECURVE_Fireworks_Fade")
            return FIREWORKSFADE_MAX;
        if (name == "E_VALUECURVE_Fireworks_Velocity")
            return FIREWORKSVELOCITY_MAX;
        if (name == "E_VALUECURVE_Fireworks_XVelocity")
            return FIREWORKSXVELOCITY_MAX;
        if (name == "E_VALUECURVE_Fireworks_YVelocity")
            return FIREWORKSYVELOCITY_MAX;
        if (name == "E_VALUECURVE_Fireworks_XLocation")
            return FIREWORKSXLOCATION_MAX;
        if (name == "E_VALUECURVE_Fireworks_YLocation")
            return FIREWORKSYLOCATION_MAX;
        return RenderableEffect::GetSettingVCMax(name);
    }

protected:
    virtual xlEffectPanel* CreatePanel(wxWindow* parent) override;
    void SetPanelTimingTracks() const;
    static std::pair<int, int> GetFireworkLocation(int width, int height, int overridex = -1, int overridey = -1);
    virtual bool needToAdjustSettings(const std::string& version) override;
    virtual void adjustSettings(const std::string& version, Effect* effect, bool removeDefaults = true) override;
};
