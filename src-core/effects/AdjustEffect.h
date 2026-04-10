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
#include "UtilFunctions.h"

class AdjustEffect : public RenderableEffect
{
public:
    AdjustEffect(int id);
    virtual ~AdjustEffect();
    virtual bool CanBeRandom() override
    {
        return false;
    }
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual bool CanRenderPartialTimeInterval() const override
    {
        return true;
    }
    virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache) override;

    static std::string sActionDefault;
    static int sValue1Default;
    static int sValue2Default;
    static int sNthChannelDefault;
    static int sStartingAtDefault;
    static int sCountDefault;

protected:
    virtual void OnMetadataLoaded() override;
    void AdjustChannels(bool singleColour, int numChannels, RenderBuffer& buffer, const std::string& action, int value1, int value2, int nth, int starting, int count);
};

