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

#include <cstdint>

#include "RenderableEffect.h"

class CandleEffect : public RenderableEffect
{
public:
    CandleEffect(int id);
    virtual ~CandleEffect();
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache) override;

    static int sFlameAgilityDefault;
    static int sFlameAgilityMin;
    static int sFlameAgilityMax;
    static int sWindBaselineDefault;
    static int sWindBaselineMin;
    static int sWindBaselineMax;
    static int sWindVariabilityDefault;
    static int sWindVariabilityMin;
    static int sWindVariabilityMax;
    static int sWindCalmnessDefault;
    static int sWindCalmnessMin;
    static int sWindCalmnessMax;
    static bool sPerNodeDefault;
    static bool sUsePaletteDefault;

protected:
    virtual void OnMetadataLoaded() override;
    void Update(uint8_t& flameprime, uint8_t& flame, uint8_t& wind, size_t windVariability, size_t flameAgility, size_t windCalmness, size_t windBaseline);
};
