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

#define SERVO_MIN 0
#define SERVO_MAX 1000
#define SERVO_DIVISOR 10

class ServoEffect : public RenderableEffect
{
public:
    ServoEffect(int id);
    virtual ~ServoEffect();
    virtual bool CanBeRandom() override
    {
        return false;
    }
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual void RenameTimingTrack(std::string oldname, std::string newname, Effect* effect) override;
    virtual void SetPanelStatus(Model* cls) override;
    virtual void SetDefaultParameters() override;
    virtual bool CanRenderPartialTimeInterval() const override
    {
        return true;
    }
    virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache) override;

    virtual double GetSettingVCMin(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Servo")
            return SERVO_MIN;
        return RenderableEffect::GetSettingVCMin(name);
    }
    virtual double GetSettingVCMax(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Servo")
            return SERVO_MAX;
        return RenderableEffect::GetSettingVCMax(name);
    }
    virtual int GetSettingVCDivisor(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Servo")
            return SERVO_DIVISOR;
        return RenderableEffect::GetSettingVCDivisor(name);
    }

protected:
    virtual xlEffectPanel* CreatePanel(wxWindow* parent) override;
    virtual bool needToAdjustSettings(const std::string& version) override
    {
        return false;
    }
    int GetPhonemeValue(RenderBuffer& buffer, SequenceElements* elements, const std::string& trackName);
};
