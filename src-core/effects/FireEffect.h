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

class FireEffect : public RenderableEffect
{
public:
    FireEffect(int id);
    virtual ~FireEffect();
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache) override;

    // Cached from Fire.json by OnMetadataLoaded(). Exposed as statics so any
    // cold-path code (imports, Metal subclass, tests) can read them without
    // needing a pointer to the singleton effect instance.
    static int sHeightDefault;
    static int sHeightMin;
    static int sHeightMax;
    static int sHueShiftDefault;
    static int sHueShiftMin;
    static int sHueShiftMax;
    static double sGrowthCyclesDefault;
    static double sGrowthCyclesMin;
    static double sGrowthCyclesMax;
    static int sGrowthCyclesDivisor;
    static bool sGrowWithMusicDefault;
    static std::string sLocationDefault;

protected:
    virtual void OnMetadataLoaded() override;
};
