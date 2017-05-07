#include "CandleEffect.h"
#include "CandlePanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../AudioManager.h"
#include "../models/Model.h"
#include "../SequenceCheck.h"

#include "../../include/candle-16.xpm"
#include "../../include/candle-24.xpm"
#include "../../include/candle-32.xpm"
#include "../../include/candle-48.xpm"
#include "../../include/candle-64.xpm"
#include <UtilFunctions.h>

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



class CandleRenderCache : public EffectRenderCache {
public:
    CandleRenderCache() {};
    virtual ~CandleRenderCache() {};

    wxByte flameprimer;
    wxByte flamer;
    wxByte wind;
    wxByte flameprimeg;
    wxByte flameg;
};


static CandleRenderCache* GetCache(RenderBuffer &buffer, int id) {
    CandleRenderCache *cache = (CandleRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new CandleRenderCache();
        buffer.infoCache[id] = cache;
    }
    return cache;
}

void CandleEffect::SetDefaultParameters(Model *cls) {
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
    if (flame<255) {
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
        if (flameprime >(flameAgility)) {
            flameprime -= flameAgility;
        }
    }

    //How do we prevent jittering when the two are equal?
    //We don't. It adds to the realism.
}

// 10 <= HeightPct <= 100
void CandleEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {

    float oset = buffer.GetEffectTimeIntervalPosition();
    int flameAgility = GetValueCurveInt("Candle_FlameAgility", 2, SettingsMap, oset, 1, 10);
    int windCalmness = GetValueCurveInt("Candle_WindCalmness", 2, SettingsMap, oset, 0, 10);
    int windVariability = GetValueCurveInt("Candle_WindVariability", 5, SettingsMap, oset, 0, 10);
    int windBaseline = GetValueCurveInt("Candle_WindBaseline", 30, SettingsMap, oset, 0, 255);

    CandleRenderCache *cache = GetCache(buffer, id);
    wxByte& flameprimer = cache->flameprimer;
    wxByte& flamer = cache->flamer;
    wxByte& wind = cache->wind;
    wxByte& flameprimeg = cache->flameprimeg;
    wxByte& flameg = cache->flameg;

    if (buffer.needToInit)
    {
        buffer.needToInit = false;

        flamer = rand01() * 255;
        flameprimer = rand01() * 255;

        flameg = rand01() * flamer;
        flameprimeg = rand01() * flameprimer;

        wind = rand01() * 255;
    }

    Update(flameprimer, flamer, wind, windVariability, flameAgility, windCalmness, windBaseline);
    Update(flameprimeg, flameg, wind, windVariability, flameAgility, windCalmness, windBaseline);

    if (flameprimeg > flameprimer) flameprimeg = flameprimer;
    if (flameg > flamer) flameprimeg = flameprimer;

    //  Now play Candle
    xlColor c = xlColor(flameprimer, flameprimeg / 2, 0);
    for (size_t y=0; y<buffer.BufferHt; y++)
    {
        for (size_t x=0; x<buffer.BufferWi; x++)
        {
            buffer.SetPixel(x, y, c);
        }
    }
}
