/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "CandleEffect.h"

#include <cassert>
#include <spdlog/fmt/fmt.h>
#include <map>

#include "../render/Effect.h"
#include "../render/RenderBuffer.h"
#include "UtilClasses.h"
#include "../models/Model.h"
#include "UtilFunctions.h"
#include "Parallel.h"
#include "../utils/xlSize.h"

#include "../../include/candle-16.xpm"
#include "../../include/candle-24.xpm"
#include "../../include/candle-32.xpm"
#include "../../include/candle-48.xpm"
#include "../../include/candle-64.xpm"

int CandleEffect::sFlameAgilityDefault = 2;
int CandleEffect::sFlameAgilityMin = 1;
int CandleEffect::sFlameAgilityMax = 10;
int CandleEffect::sWindBaselineDefault = 30;
int CandleEffect::sWindBaselineMin = 0;
int CandleEffect::sWindBaselineMax = 255;
int CandleEffect::sWindVariabilityDefault = 5;
int CandleEffect::sWindVariabilityMin = 0;
int CandleEffect::sWindVariabilityMax = 10;
int CandleEffect::sWindCalmnessDefault = 2;
int CandleEffect::sWindCalmnessMin = 0;
int CandleEffect::sWindCalmnessMax = 10;
bool CandleEffect::sPerNodeDefault = false;
bool CandleEffect::sUsePaletteDefault = false;

CandleEffect::CandleEffect(int id) : RenderableEffect(id, "Candle", candle_16, candle_24, candle_32, candle_48, candle_64)
{
    //ctor
}

CandleEffect::~CandleEffect()
{
    //dtor
}

void CandleEffect::OnMetadataLoaded()
{
    sFlameAgilityDefault = GetIntDefault("Candle_FlameAgility", sFlameAgilityDefault);
    sFlameAgilityMin = (int)GetMinFromMetadata("Candle_FlameAgility", sFlameAgilityMin);
    sFlameAgilityMax = (int)GetMaxFromMetadata("Candle_FlameAgility", sFlameAgilityMax);
    sWindBaselineDefault = GetIntDefault("Candle_WindBaseline", sWindBaselineDefault);
    sWindBaselineMin = (int)GetMinFromMetadata("Candle_WindBaseline", sWindBaselineMin);
    sWindBaselineMax = (int)GetMaxFromMetadata("Candle_WindBaseline", sWindBaselineMax);
    sWindVariabilityDefault = GetIntDefault("Candle_WindVariability", sWindVariabilityDefault);
    sWindVariabilityMin = (int)GetMinFromMetadata("Candle_WindVariability", sWindVariabilityMin);
    sWindVariabilityMax = (int)GetMaxFromMetadata("Candle_WindVariability", sWindVariabilityMax);
    sWindCalmnessDefault = GetIntDefault("Candle_WindCalmness", sWindCalmnessDefault);
    sWindCalmnessMin = (int)GetMinFromMetadata("Candle_WindCalmness", sWindCalmnessMin);
    sWindCalmnessMax = (int)GetMaxFromMetadata("Candle_WindCalmness", sWindCalmnessMax);
    sPerNodeDefault = GetBoolDefault("PerNode", sPerNodeDefault);
    sUsePaletteDefault = GetBoolDefault("UsePalette", sUsePaletteDefault);
}

std::list<std::string> CandleEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache)
{
    std::list<std::string> res = RenderableEffect::CheckEffectSettings(settings, media, model, eff, renderCache);

    if (media == nullptr && settings.GetBool("E_CHECKBOX_Candle_GrowWithMusic", false)) {
        res.push_back(fmt::format("    WARN: Candle effect cant grow to music if there is no music. Model '{}', Start {}", model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())));
    }

    return res;
}

class CandleState {
public:
    CandleState() {
    }
    void init() {
        flamer = rand01() * 255;
        flameprimer = rand01() * 255;
        flameg = rand01() * flamer;
        flameprimeg = rand01() * flameprimer;
        wind = rand01() * 255;
    }
    uint8_t flameprimer;
    uint8_t flamer;
    uint8_t wind;
    uint8_t flameprimeg;
    uint8_t flameg;
};

class CandleRenderCache : public EffectRenderCache
{
public:
    std::vector<CandleState> _states;
    int maxWid;

    CandleRenderCache(){};
    virtual ~CandleRenderCache() {
        _states.clear();
    };
};

static CandleRenderCache* GetCache(RenderBuffer& buffer, int id)
{
    CandleRenderCache* cache = (CandleRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new CandleRenderCache();
        buffer.infoCache[id] = cache;
    }
    return cache;
}


void CandleEffect::Update(uint8_t& flameprime, uint8_t& flame, uint8_t& wind, size_t windVariability, size_t flameAgility, size_t windCalmness, size_t windBaseline)
{
    // We simulate a gust of wind by setting the wind var to a random value
    if (uint8_t(rand01() * 255.0) < windVariability) {
        wind = uint8_t(rand01() * 255.0);
    }

    // The wind constantly settles towards its baseline value
    if (wind > windBaseline) {
        wind--;
    }

    // The flame constantly gets brighter till the wind knocks it down
    if (flame < 255) {
        flame++;
    }

    // Depending on the wind strength and the calmness modifier we calculate the odds
    // of the wind knocking down the flame by setting it to random values
    if (uint8_t(rand01() * 255) < (wind >> windCalmness)) {
        flame = uint8_t(rand01() * 255);
    }

    // Real flames ook like they have inertia so we use this constant-aproach-rate filter
    // To lowpass the flame height
    if (flame > flameprime) {
        if (flameprime < (255 - flameAgility)) {
            flameprime += flameAgility;
        }
    } else {
        if (flameprime > flameAgility) {
            flameprime -= flameAgility;
        }
    }

    // How do we prevent jittering when the two are equal?
    // We don't. It adds to the realism.
}

// 10 <= HeightPct <= 100
void CandleEffect::Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer)
{
    float oset = buffer.GetEffectTimeIntervalPosition();
    int flameAgility = GetValueCurveInt("Candle_FlameAgility", sFlameAgilityDefault, SettingsMap, oset, sFlameAgilityMin, sFlameAgilityMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int windCalmness = GetValueCurveInt("Candle_WindCalmness", sWindCalmnessDefault, SettingsMap, oset, sWindCalmnessMin, sWindCalmnessMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int windVariability = GetValueCurveInt("Candle_WindVariability", sWindVariabilityDefault, SettingsMap, oset, sWindVariabilityMin, sWindVariabilityMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int windBaseline = GetValueCurveInt("Candle_WindBaseline", sWindBaselineDefault, SettingsMap, oset, sWindBaselineMin, sWindBaselineMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    bool perNode = SettingsMap.GetBool("CHECKBOX_PerNode", sPerNodeDefault);
    bool usePalette = SettingsMap.GetBool("CHECKBOX_UsePalette", sUsePaletteDefault);

    const auto& pal = effect->GetPalette();
    xlColor c1, c2;
    if (usePalette){  // We're using the palette.
        if (pal.empty()) {
            // No colors selected. Default to white. Set black as second color.
            c1 = xlWHITE;
            c2 = xlBLACK;
        } else {
            // One color selected, set black as second color.
            c1 = pal[0];
            c2 = (pal.size() > 1 ? pal[1] : xlBLACK);
        }
    }

    CandleRenderCache* cache = GetCache(buffer, id);
    std::vector<CandleState>& states = cache->_states;

    if (buffer.needToInit) {
        buffer.needToInit = false;

        int numStates = 1;
        if (perNode) {
            xlSize maxBuffer = buffer.GetMaxBuffer(SettingsMap);
            int maxMWi = maxBuffer.width == -1 ? buffer.BufferWi : maxBuffer.width;
            int maxMHt = maxBuffer.height == -1 ? buffer.BufferHt : maxBuffer.height;
            cache->maxWid = maxMWi;
            numStates = maxMWi * maxMHt;
        }
        if (numStates > (int)states.size()) {
            states.resize(numStates);
        }
        for (int x = 0; x < numStates; x++) {
            states[x].init();
        }
    }

    if (perNode) {
        int maxW = cache->maxWid;
        parallel_for(0, buffer.BufferHt, [&buffer, &states, maxW, windVariability, flameAgility, windCalmness, windBaseline, usePalette, c1, c2, this](int y) {
            for (int x = 0; x < buffer.BufferWi; x++) {
                size_t index = y * maxW + x;
                if (index >= states.size()) {
                    // this should never happen
                    assert(false);
                } else {
                    CandleState* state = &states[index];

                    Update(state->flameprimer, state->flamer, state->wind, windVariability, flameAgility, windCalmness, windBaseline);
                    Update(state->flameprimeg, state->flameg, state->wind, windVariability, flameAgility, windCalmness, windBaseline);

                    if (state->flameprimeg > state->flameprimer)
                        state->flameprimeg = state->flameprimer;
                    if (state->flameg > state->flamer)
                        state->flameprimeg = state->flameprimer;

                    xlColor c;
                    if (usePalette) {
                        float t = float(state->flameprimer) / 255.0f;
                        c.red = uint8_t(c1.red * (1.0f - t) + c2.red * t);
                        c.green = uint8_t(c1.green * (1.0f - t) + c2.green * t);
                        c.blue = uint8_t(c1.blue * (1.0f - t) + c2.blue * t);
                    } else {
                        c = xlColor(state->flameprimer, state->flameprimeg / 2, 0);
                    }
                    buffer.SetPixel(x, y, c);
                }
            }
        });
    } else {
        CandleState* state = &states[0];

        Update(state->flameprimer, state->flamer, state->wind, windVariability, flameAgility, windCalmness, windBaseline);
        Update(state->flameprimeg, state->flameg, state->wind, windVariability, flameAgility, windCalmness, windBaseline);

        if (state->flameprimeg > state->flameprimer)
            state->flameprimeg = state->flameprimer;
        if (state->flameg > state->flamer)
            state->flameprimeg = state->flameprimer;

        //  Now play Candle
        xlColor c;
        if (usePalette) {
            float t = float(state->flameprimer) / 255.0f;
            c.red = uint8_t(c1.red * (1.0f - t) + c2.red * t);
            c.green = uint8_t(c1.green * (1.0f - t) + c2.green * t);
            c.blue = uint8_t(c1.blue * (1.0f - t) + c2.blue * t);
        } else {
            c = xlColor(state->flameprimer, state->flameprimeg / 2, 0);
        }
        for (int y = 0; y < buffer.BufferHt; y++) {
            for (int x = 0; x < buffer.BufferWi; x++) {
                buffer.SetPixel(x, y, c);
            }
        }
    }
}
