#include "CandleEffect.h"
#include "CandlePanel.h"

#include <map>

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../AudioManager.h"
#include "../models/Model.h"
#include "../UtilFunctions.h"

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

std::list<std::string> CandleEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff)
{
    std::list<std::string> res;

    if (media == nullptr && settings.GetBool("E_CHECKBOX_Candle_GrowWithMusic", false))
    {
        res.push_back(wxString::Format("    WARN: Candle effect cant grow to music if there is no music. Model '%s', Start %s", model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    }

    return res;
}

wxPanel *CandleEffect::CreatePanel(wxWindow *parent) {
    return new CandlePanel(parent);
}

struct CandleState
{
    wxByte flameprimer;
    wxByte flamer;
    wxByte wind;
    wxByte flameprimeg;
    wxByte flameg;
};

class CandleRenderCache : public EffectRenderCache {
public:
    std::map<int, CandleState*> _states;

    CandleRenderCache() {};
    virtual ~CandleRenderCache()
    {
        while (_states.size() > 0)
        {
            int index = _states.begin()->first;
            CandleState* todelete = _states[index];
            _states.erase(index);
            delete todelete;
        }

    };
};

static CandleRenderCache* GetCache(RenderBuffer &buffer, int id) {
    CandleRenderCache *cache = (CandleRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new CandleRenderCache();
        buffer.infoCache[id] = cache;
    }
    return cache;
}

void CandleEffect::SetDefaultParameters() {
    CandlePanel *fp = (CandlePanel*)panel;
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
}

void CandleEffect::Update(wxByte& flameprime, wxByte& flame, wxByte& wind, size_t windVariability, size_t flameAgility, size_t windCalmness, size_t windBaseline)
{
    //We simulate a gust of wind by setting the wind var to a random value
    if (wxByte(rand01() * 255.0) < windVariability) {
        wind = wxByte(rand01() * 255.0);
    }

    //The wind constantly settles towards its baseline value
    if (wind > windBaseline) {
        wind--;
    }

    //The flame constantly gets brighter till the wind knocks it down
    if (flame < 255) {
        flame++;
    }

    //Depending on the wind strength and the calmnes modifer we calcuate the odds
    //of the wind knocking down the flame by setting it to random values
    if (wxByte(rand01() * 255) < (wind >> windCalmness)) {
        flame = wxByte(rand01() * 255);
    }

    //Real flames ook like they have inertia so we use this constant-aproach-rate filter
    //To lowpass the flame height
    if (flame > flameprime) {
        if (flameprime < (255 - flameAgility)) {
            flameprime += flameAgility;
        }
    }
    else {
        if (flameprime > flameAgility) {
            flameprime -= flameAgility;
        }
    }

    //How do we prevent jittering when the two are equal?
    //We don't. It adds to the realism.
}

void InitialiseState(int node, std::map<int, CandleState*>& states)
{
    if (states.find(node) == states.end())
    {
        CandleState* state = new CandleState();
        states[node] = state;
    }

    states[node]->flamer = rand01() * 255;
    states[node]->flameprimer = rand01() * 255;

    states[node]->flameg = rand01() * states[node]->flamer;
    states[node]->flameprimeg = rand01() * states[node]->flameprimer;

    states[node]->wind = rand01() * 255;
}

// 10 <= HeightPct <= 100
void CandleEffect::Render(Effect *effect, SettingsMap &SettingsMap, RenderBuffer &buffer) {

    float oset = buffer.GetEffectTimeIntervalPosition();
    int flameAgility = GetValueCurveInt("Candle_FlameAgility", 2, SettingsMap, oset, CANDLE_AGILITY_MIN, CANDLE_AGILITY_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int windCalmness = GetValueCurveInt("Candle_WindCalmness", 2, SettingsMap, oset, CANDLE_WINDCALMNESS_MIN, CANDLE_WINDCALMNESS_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int windVariability = GetValueCurveInt("Candle_WindVariability", 5, SettingsMap, oset, CANDLE_WINDVARIABILITY_MIN, CANDLE_WINDVARIABILITY_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int windBaseline = GetValueCurveInt("Candle_WindBaseline", 30, SettingsMap, oset, CANDLE_WINDBASELINE_MIN, CANDLE_WINDBASELINE_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    bool perNode = SettingsMap.GetBool("CHECKBOX_PerNode", false);

    CandleRenderCache *cache = GetCache(buffer, id);
    std::map<int, CandleState*>& states = cache->_states;

    if (buffer.needToInit)
    {
        buffer.needToInit = false;

        if (perNode)
        {
            for (size_t x = 0; x < buffer.ModelBufferWi; ++x)
            {
                for (size_t y = 0; y < buffer.ModelBufferHt; ++y)
                {
                    size_t index = y * buffer.ModelBufferWi + x;
                    InitialiseState(index, states);
                }
            }
        }
        else
        {
            InitialiseState(0, states);
        }
    }

    if (perNode)
    {
        for (size_t y = 0; y < buffer.BufferHt; y++)
        {
            for (size_t x = 0; x < buffer.BufferWi; x++)
            {
                size_t index = y * buffer.ModelBufferWi + x;
                if (index >= states.size())
                {
                    // this should never happen
                    wxASSERT(false);
                }
                else
                {
                    CandleState* state = states[index];

                    Update(state->flameprimer, state->flamer, state->wind, windVariability, flameAgility, windCalmness, windBaseline);
                    Update(state->flameprimeg, state->flameg, state->wind, windVariability, flameAgility, windCalmness, windBaseline);

                    if (state->flameprimeg > state->flameprimer) state->flameprimeg = state->flameprimer;
                    if (state->flameg > state->flamer) state->flameprimeg = state->flameprimer;

                    //  Now play Candle
                    xlColor c = xlColor(state->flameprimer, state->flameprimeg / 2, 0);
                    buffer.SetPixel(x, y, c);
                }
            }
        }
    }
    else
    {
        CandleState* state = states[0];

        Update(state->flameprimer, state->flamer, state->wind, windVariability, flameAgility, windCalmness, windBaseline);
        Update(state->flameprimeg, state->flameg, state->wind, windVariability, flameAgility, windCalmness, windBaseline);

        if (state->flameprimeg > state->flameprimer) state->flameprimeg = state->flameprimer;
        if (state->flameg > state->flamer) state->flameprimeg = state->flameprimer;

        //  Now play Candle
        xlColor c = xlColor(state->flameprimer, state->flameprimeg / 2, 0);
        for (size_t y = 0; y < buffer.BufferHt; y++)
        {
            for (size_t x = 0; x < buffer.BufferWi; x++)
            {
                buffer.SetPixel(x, y, c);
            }
        }
    }
}
