/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "TwinkleEffect.h"
#include "TwinklePanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

#include "../../include/twinkle-16.xpm"
#include "../../include/twinkle-24.xpm"
#include "../../include/twinkle-32.xpm"
#include "../../include/twinkle-48.xpm"
#include "../../include/twinkle-64.xpm"
#include "ValueCurve.h"

#include "../Parallel.h"

#include <random>

static std::random_device rd;
static std::default_random_engine eng{ rd() };
static std::uniform_int_distribution<> dist(0, INT_MAX);

TwinkleEffect::TwinkleEffect(int id) : RenderableEffect(id, "Twinkle", twinkle_16, twinkle_24, twinkle_32, twinkle_48, twinkle_64)
{
    //ctor
}

TwinkleEffect::~TwinkleEffect()
{
    //dtor
}
xlEffectPanel *TwinkleEffect::CreatePanel(wxWindow *parent) {
    return new TwinklePanel(parent);
}

class StrobeClass
{
public:
    
    int x,y;
    int duration; // How frames strobe light stays on. Will be decremented each frame
    int colorindex;
    int strobing;
};

class TwinkleRenderCache : public EffectRenderCache {
public:
    TwinkleRenderCache() {};
    virtual ~TwinkleRenderCache() {};
    
    std::vector<StrobeClass> strobe;
    int num_lights;
    int curNumStrobe;
    std::atomic_int lights_to_renew;
};

void TwinkleEffect::SetDefaultParameters()
{
    TwinklePanel *tp = (TwinklePanel*)panel;
    if (tp == nullptr) {
        return;
    }

    tp->BitmapButton_Twinkle_CountVC->SetActive(false);
    tp->BitmapButton_Twinkle_StepsVC->SetActive(false);

    SetSliderValue(tp->Slider_Twinkle_Count, 3);
    SetSliderValue(tp->Slider_Twinkle_Steps, 30);
    SetCheckBoxValue(tp->CheckBox_Twinkle_Strobe, false);
    SetCheckBoxValue(tp->CheckBox_Twinkle_ReRandom, false);
    SetChoiceValue(tp->Choice_Twinkle_Style, "New Render Method");
}

bool TwinkleEffect::needToAdjustSettings(const std::string& version) {
    // give the base class a chance to adjust any settings
    return RenderableEffect::needToAdjustSettings(version) || IsVersionOlder("2020.57", version);
}

void TwinkleEffect::adjustSettings(const std::string& version, Effect* effect, bool removeDefaults) {
    // give the base class a chance to adjust any settings
    if (RenderableEffect::needToAdjustSettings(version))
    {
        RenderableEffect::adjustSettings(version, effect, removeDefaults);
    }
    if (IsVersionOlder("2020.57", version)) {
        SettingsMap& settings = effect->GetSettings();
        settings["E_CHOICE_Twinkle_Style"] = "Old Render Method";
    }
}

int TwinkleEffect::DrawEffectBackground(const Effect *e, int x1, int y1, int x2, int y2,
    DrawGLUtils::xlAccumulator &bg, xlColor* colorMask, bool ramp)
{
    if (ramp)
    {
        float endi;
        float starti;
        std::string vcs = e->GetPaletteMap().Get("C_VALUECURVE_Brightness", "");
        if (vcs == "")
        {
            starti = e->GetPaletteMap().GetInt("C_SLIDER_Brightness", 100);
            if (starti > 100) starti = 100;
            endi = starti;
        }
        else
        {
            ValueCurve vc(vcs);
            starti = vc.GetOutputValueAt(0.0, e->GetStartTimeMS(), e->GetEndTimeMS());
            if (starti > 100) starti = 100;
            endi = vc.GetOutputValueAt(1.0, e->GetStartTimeMS(), e->GetEndTimeMS());
            if (endi > 100) endi = 100;
        }

        xlColor color = e->GetPalette()[0];
        color.ApplyMask(colorMask);

        int height = y2 - y1;
        float starty = (float)y2 - starti * (float)height / 100.0f;
        float endy = (float)y2 - endi * (float)height / 100.0f;
        float m = float(endy - starty) / float(x2 - x1);

        const int gap = 10;

        bg.Finish(GL_TRIANGLES);
        bg.AddVertex(x1, starty, color);
        bg.AddVertex(x1, y2, color);
        bg.AddVertex(x1, y2, color);
        bg.AddVertex(x2, y2, color);
        bg.AddVertex(x2, y2, color);
        bg.AddVertex(x2, endy, color);
        bg.AddVertex(x2, endy, color);
        bg.AddVertex(x1, starty, color);

        int lastx = x1;
        for (int x = x1 + gap; x < x2; x += gap) {
            float newY = m * (x - x1 - gap) + starty;
            float newY2 = m * (x - x1) + starty;
            bg.AddVertex(x-gap, newY, color);
            bg.AddVertex(x, y2, color);
            bg.AddVertex(x, newY2, color);
            bg.AddVertex(x-gap, y2, color);
            lastx = x;

            // draw a second outline
            float ylow1 = y2 + 1;
            float ylow2 = y2 + 1;
            if (ylow1 > newY) ylow1 = newY;
            if (ylow2 > newY2) ylow2 = newY2;
            bg.AddVertex(x - gap, ylow1, color);
            bg.AddVertex(x, ylow2, color);

            float yhigh1 = newY - 1;
            float yhigh2 = newY2 - 1;
            if (yhigh1 < y2) yhigh1 = y2;
            if (yhigh2 < y2) yhigh2 = y2;
            bg.AddVertex(x - gap, yhigh1, color);
            bg.AddVertex(x, yhigh2, color);
        }

        // fill in the end
        if (lastx != x2)
        {
            float newY = m * (lastx - x1) + starty;
            float newY2 = m * (x2-x1) + starty;
            bg.AddVertex(lastx, newY, color);
            bg.AddVertex(x2, y2, color);
            bg.AddVertex(x2, newY2, color);
            bg.AddVertex(lastx, y2, color);

            // draw a second outline
            float ylow1 = y2 + 1;
            float ylow2 = y2 + 1;
            if (ylow1 > newY) ylow1 = newY;
            if (ylow2 > newY2) ylow2 = newY2;
            bg.AddVertex(lastx, ylow1, color);
            bg.AddVertex(x2, ylow2, color);

            float yhigh1 = newY - 1;
            float yhigh2 = newY2 - 1;
            if (yhigh1 < y2) yhigh1 = y2;
            if (yhigh2 < y2) yhigh2 = y2;
            bg.AddVertex(lastx, yhigh1, color);
            bg.AddVertex(x2, yhigh2, color);
        }

        bg.Finish(GL_LINES);

        return 2;
    }

    return 1;
}

static void place_twinkles(int lights_to_place, int &curIndex, std::vector<StrobeClass>& strobe, RenderBuffer& buffer,
                           int max_modulo, size_t colorcnt) {
    while (lights_to_place > 0 && (curIndex < strobe.size())) {
        int idx = dist(eng) % (strobe.size() - curIndex) + curIndex;
        if (idx != curIndex) {
            std::swap(strobe[idx], strobe[curIndex]);
        }
        strobe[curIndex].duration = dist(eng) % max_modulo;
        strobe[curIndex].colorindex = dist(eng) % colorcnt;
        strobe[curIndex].strobing = true;
        curIndex++;
        lights_to_place--;
    }
}

void TwinkleEffect::Render(Effect *effect, SettingsMap &SettingsMap, RenderBuffer &buffer) {
    
    float oset = buffer.GetEffectTimeIntervalPosition();
    int Count = GetValueCurveInt("Twinkle_Count", 3, SettingsMap, oset, 2, 100, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int Steps = GetValueCurveInt("Twinkle_Steps", 30, SettingsMap, oset, 2, 200, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    bool Strobe = SettingsMap.GetBool("CHECKBOX_Twinkle_Strobe", false);
    bool reRandomize = SettingsMap.GetBool("CHECKBOX_Twinkle_ReRandom", false);
    const std::string& twinkle_style = SettingsMap["CHOICE_Twinkle_Style"];
    bool new_algorithm = false;
    if (twinkle_style == "New Render Method") {
        new_algorithm = true;
    }

    int lights = (buffer.BufferHt*buffer.BufferWi)*(Count / 100.0); // Count is in range of 1-100 from slider bar
    if (buffer.BufferHt * buffer.BufferWi == 1) lights = 1;

    int step = 1;
    if (lights > 0) {
        step = buffer.BufferHt*buffer.BufferWi / lights;
    }
    int max_modulo = Steps;
    if (max_modulo<2) 
        max_modulo = 2;  // scm  could we be getting 0 passed in?
    int max_modulo2 = max_modulo / 2;
    if (max_modulo2<1) 
        max_modulo2 = 1;
    
    if (step<1)
        step=1;
    
    TwinkleRenderCache *cache = (TwinkleRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new TwinkleRenderCache();
        buffer.infoCache[id] = cache;
        cache->num_lights = lights;
        cache->lights_to_renew = lights;
        cache->curNumStrobe = 0;
    }
    std::vector<StrobeClass> &strobe = cache->strobe;

    if (new_algorithm) {
        cache->lights_to_renew += lights - cache->num_lights;
    }
    else {
        if (lights != cache->num_lights) {
            buffer.needToInit = true;
        }
    }
    cache->num_lights = lights;

    size_t colorcnt=buffer.GetColorCount();

    int i = 0;

    if (buffer.needToInit) {
        buffer.needToInit = false;
        cache->lights_to_renew = lights;
        cache->curNumStrobe = 0;
        if (new_algorithm) {
            strobe.clear();
            strobe.resize(buffer.BufferHt * buffer.BufferWi);
            int s = 0;
            for (int x = 0; x < buffer.BufferWi; x++) {
                for (int y = 0; y < buffer.BufferHt; y++) {
                    strobe[s].x = x;
                    strobe[s].y = y;
                    strobe[s].duration = 0;
                    strobe[s].strobing = -1;
                    s++;
                }
            }
            //randomize the locations
            for (int s = 0; s < strobe.size(); ++s) {

                int r = dist(eng) % strobe.size();
                if (r != s) {
                    std::swap(strobe[r], strobe[s]);
                }
            }
        }
        else {
            strobe.clear();
            cache->curNumStrobe = 0;
            for (int y = 0; y < buffer.BufferHt; y++) {
                for (int x = 0; x < buffer.BufferWi; x++) {
                    i++;
                    if (i % step == 1 || step == 1) {
                        int s = strobe.size();
                        strobe.resize(s + 1);
                        strobe[s].duration = dist(eng) % max_modulo;

                        strobe[s].x = x;
                        strobe[s].y = y;

                        strobe[s].colorindex = dist(eng) % colorcnt;
                        cache->curNumStrobe++;
                    }
                }
            }
        }
    }
    
    if (new_algorithm) {
        if (cache->lights_to_renew > 0) {
            while (cache->curNumStrobe && !strobe[cache->curNumStrobe-1].strobing) {
                cache->curNumStrobe--;
            }
            for (int x = 0; x < cache->curNumStrobe; x++) {
                if (!strobe[x].strobing) {
                    cache->curNumStrobe--;
                    if (x != cache->curNumStrobe) {
                        std::swap(strobe[x], strobe[cache->curNumStrobe]);
                    }
                    while (cache->curNumStrobe && !strobe[cache->curNumStrobe-1].strobing) {
                        cache->curNumStrobe--;
                    }
                }
            }
            place_twinkles(cache->lights_to_renew, cache->curNumStrobe, strobe, buffer, max_modulo, colorcnt);
            cache->lights_to_renew = 0;
        }
    }

    parallel_for(0, cache->curNumStrobe, [&strobe, &buffer, max_modulo, max_modulo2, colorcnt, reRandomize, Strobe, new_algorithm, cache](int x) {
        strobe[x].duration++;
        if (new_algorithm) {
            if (!strobe[x].strobing) {
                return;
            }
        }
        if (strobe[x].duration < 0) {
            return;
        }
        if (strobe[x].duration == max_modulo) {
            strobe[x].duration = 0;
            if (new_algorithm) {
                cache->lights_to_renew++;
                strobe[x].strobing = false;
            }
            else if (reRandomize) {
                strobe[x].duration -= dist(eng) % max_modulo2;
                strobe[x].colorindex = dist(eng) % colorcnt;
            }
        }
        int i7 = strobe[x].duration;
        HSVValue hsv;
        buffer.palette.GetHSV(strobe[x].colorindex, hsv);
        double v;
        if(i7<=max_modulo2) {
            if(max_modulo2>0) v = (1.0*i7)/max_modulo2;
            else v =0;
        } else {
            if(max_modulo2>0)v = (max_modulo-i7)*1.0/(max_modulo2);
            else v = 0;
        }
        if (v<0.0) v=0.0;
        
        if (Strobe) {
            if (i7==max_modulo2) v = 1.0;
            else v = 0.0;
        }
        if (buffer.allowAlpha) {
            xlColor color;
            buffer.palette.GetColor(strobe[x].colorindex, color);
            color.alpha = 255.0 * v;
            buffer.SetPixel(strobe[x].x, strobe[x].y, color); // Turn pixel on
        }
        else {
            buffer.palette.GetHSV(strobe[x].colorindex, hsv);
            //  we left the Hue and Saturation alone, we are just modifiying the Brightness Value
            hsv.value = v;
            buffer.SetPixel(strobe[x].x, strobe[x].y, hsv); // Turn pixel on
        }
    }, 500);
}
