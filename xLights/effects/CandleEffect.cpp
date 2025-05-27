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
#include "CandlePanel.h"

#include <map>

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../AudioManager.h"
#include "../models/Model.h"
#include "../UtilFunctions.h"
#include "../Parallel.h"

#include "../../include/candle-16.xpm"
#include "../../include/candle-24.xpm"
#include "../../include/candle-32.xpm"
#include "../../include/candle-48.xpm"
#include "../../include/candle-64.xpm"

CandleEffect::CandleEffect(int id) : RenderableEffect(id, "Candle", candle_16, candle_24, candle_32, candle_48, candle_64)
{
    //ctor
}

CandleEffect::~CandleEffect()
{
    //dtor
}

std::list<std::string> CandleEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache)
{
    std::list<std::string> res = RenderableEffect::CheckEffectSettings(settings, media, model, eff, renderCache);

    if (media == nullptr && settings.GetBool("E_CHECKBOX_Candle_GrowWithMusic", false)) {
        res.push_back(wxString::Format("    WARN: Candle effect cant grow to music if there is no music. Model '%s', Start %s", model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    }

    return res;
}

xlEffectPanel *CandleEffect::CreatePanel(wxWindow *parent) {
    return new CandlePanel(parent);
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
    wxByte flameprimer;
    wxByte flamer;
    wxByte wind;
    wxByte flameprimeg;
    wxByte flameg;
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

void CandleEffect::SetDefaultParameters()
{
    CandlePanel* fp = (CandlePanel*)panel;
    if (fp == nullptr) {
        return;
    }

    fp->BitmapButton_Candle_FlameAgilityVC->SetActive(false);
    fp->BitmapButton_Candle_WindBaselineVC->SetActive(false);
    fp->BitmapButton_Candle_WindVariabilityVC->SetActive(false);
    fp->BitmapButton_Candle_WindCalmnessVC->SetActive(false);

    SetSliderValue(fp->Slider_Candle_FlameAgility, 2);
    SetSliderValue(fp->Slider_Candle_WindBaseline, 30);
    SetSliderValue(fp->Slider_Candle_WindCalmness, 2);
    SetSliderValue(fp->Slider_Candle_WindVariability, 5);

    SetCheckBoxValue(fp->CheckBox_PerNode, false);
    SetCheckBoxValue(fp->CheckBox_UsePalette, false);
}

void CandleEffect::Update(wxByte& flameprime, wxByte& flame, wxByte& wind, size_t windVariability, size_t flameAgility, size_t windCalmness, size_t windBaseline)
{
    // We simulate a gust of wind by setting the wind var to a random value
    if (wxByte(rand01() * 255.0) < windVariability) {
        wind = wxByte(rand01() * 255.0);
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
    if (wxByte(rand01() * 255) < (wind >> windCalmness)) {
        flame = wxByte(rand01() * 255);
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
    int flameAgility = GetValueCurveInt("Candle_FlameAgility", 2, SettingsMap, oset, CANDLE_AGILITY_MIN, CANDLE_AGILITY_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int windCalmness = GetValueCurveInt("Candle_WindCalmness", 2, SettingsMap, oset, CANDLE_WINDCALMNESS_MIN, CANDLE_WINDCALMNESS_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int windVariability = GetValueCurveInt("Candle_WindVariability", 5, SettingsMap, oset, CANDLE_WINDVARIABILITY_MIN, CANDLE_WINDVARIABILITY_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int windBaseline = GetValueCurveInt("Candle_WindBaseline", 30, SettingsMap, oset, CANDLE_WINDBASELINE_MIN, CANDLE_WINDBASELINE_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    bool perNode = SettingsMap.GetBool("CHECKBOX_PerNode", false);
    bool usePalette = SettingsMap.GetBool("CHECKBOX_UsePalette", false);

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
            wxPoint maxBuffer = buffer.GetMaxBuffer(SettingsMap);
            int maxMWi = maxBuffer.x == -1 ? buffer.BufferWi : maxBuffer.x;
            int maxMHt = maxBuffer.y == -1 ? buffer.BufferHt : maxBuffer.y;
            cache->maxWid = maxMWi;
            numStates = maxMWi * maxMHt;
        }
        if (numStates > states.size()) {
            states.resize(numStates);
        }
        for (int x = 0; x < numStates; x++) {
            states[x].init();
        }
    }

    if (perNode) {
        int maxW = cache->maxWid;
        parallel_for(0, buffer.BufferHt, [&buffer, &states, maxW, windVariability, flameAgility, windCalmness, windBaseline, usePalette, c1, c2, this](int y) {
            for (size_t x = 0; x < buffer.BufferWi; x++) {
                size_t index = y * maxW + x;
                if (index >= states.size()) {
                    // this should never happen
                    wxASSERT(false);
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
                        c.red = wxByte(c1.red * (1.0f - t) + c2.red * t);
                        c.green = wxByte(c1.green * (1.0f - t) + c2.green * t);
                        c.blue = wxByte(c1.blue * (1.0f - t) + c2.blue * t);
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
            c.red = wxByte(c1.red * (1.0f - t) + c2.red * t);
            c.green = wxByte(c1.green * (1.0f - t) + c2.green * t);
            c.blue = wxByte(c1.blue * (1.0f - t) + c2.blue * t);
        } else {
            c = xlColor(state->flameprimer, state->flameprimeg / 2, 0);
        }
        for (size_t y = 0; y < buffer.BufferHt; y++) {
            for (size_t x = 0; x < buffer.BufferWi; x++) {
                buffer.SetPixel(x, y, c);
            }
        }
    }
}
