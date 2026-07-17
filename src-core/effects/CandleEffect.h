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
#include <vector>

#include "RenderableEffect.h"

class CandleState {
public:
    CandleState() {
    }
    void init(RenderBuffer& buffer);
    uint8_t flameprimer;
    uint8_t flamer;
    uint8_t wind;
    uint8_t flameprimeg;
    uint8_t flameg;
};

class CandleEffect : public RenderableEffect
{
public:
    CandleEffect(int id);
    virtual ~CandleEffect();
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual FrameParallelism GetFrameParallelism(const SettingsMap& settings) const override;
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
    void Update(RenderBuffer& buffer, uint32_t seed, uint8_t& flameprime, uint8_t& flame, uint8_t& wind, size_t windVariability, size_t flameAgility, size_t windCalmness, size_t windBaseline);
    std::vector<CandleState>* getPerNodeStates(RenderBuffer& buffer, const SettingsMap& settings, int& maxWid);
};
