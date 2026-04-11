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

class FireworksEffect : public RenderableEffect
{
public:
    FireworksEffect(int id);
    virtual ~FireworksEffect();
    virtual void RenameTimingTrack(std::string oldname, std::string newname, Effect* effect) override;
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache) override;
    virtual bool AppropriateOnNodes() const override
    {
        return false;
    }

    // Cached from Fireworks.json by OnMetadataLoaded().
    static int sExplosionsDefault;
    static int sExplosionsMin;
    static int sExplosionsMax;
    static int sCountDefault;
    static int sCountMin;
    static int sCountMax;
    static double sVelocityDefault;
    static double sVelocityMin;
    static double sVelocityMax;
    static int sXVelocityDefault;
    static int sXVelocityMin;
    static int sXVelocityMax;
    static int sYVelocityDefault;
    static int sYVelocityMin;
    static int sYVelocityMax;
    static int sXLocationDefault;
    static int sXLocationMin;
    static int sXLocationMax;
    static int sYLocationDefault;
    static int sYLocationMin;
    static int sYLocationMax;
    static bool sHoldColourDefault;
    static bool sGravityDefault;
    static int sFadeDefault;
    static int sFadeMin;
    static int sFadeMax;
    static bool sUseMusicDefault;
    static int sSensitivityDefault;
    static bool sFireTimingDefault;
    static std::string sFireTimingTrackDefault;

protected:
    virtual void OnMetadataLoaded() override;
    static std::pair<int, int> GetFireworkLocation(int width, int height, int overridex = -1, int overridey = -1);
};
