/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "StrobeEffect.h"
#include "StrobePanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../AudioManager.h"
#include "../models/Model.h"
#include "../../include/strobe.xpm"
#include "../UtilFunctions.h"

StrobeEffect::StrobeEffect(int id) : RenderableEffect(id, "Strobe", strobe, strobe, strobe, strobe, strobe)
{
    //ctor
}

StrobeEffect::~StrobeEffect()
{
    //dtor
}

std::list<std::string> StrobeEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache)
{
    std::list<std::string> res;

    if (media == nullptr && settings.GetBool("E_CHECKBOX_Strobe_Music", false)) {
        res.push_back(wxString::Format("    WARN: Strobe effect cant follow music if there is no music. Model '%s', Start %s", model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    }

    return res;
}

wxPanel *StrobeEffect::CreatePanel(wxWindow *parent) {
    return new StrobePanel(parent);
}


class StrobeClass
{
public:

    int x,y;
    int duration; // How frames strobe light stays on. Will be decremented each frame
    HSVValue hsv;
    xlColor color;

    bool operator==(const StrobeClass& r)
    {
        return x == r.x && y == r.y && hsv.hue == r.hsv.hue && hsv.saturation == r.hsv.saturation && hsv.value == r.hsv.value && color == r.color && duration == r.duration;
    }
    StrobeClass(int _x, int _y, int _duration, HSVValue _hsv, xlColor _color)
    {
        x = _x;
        y = _y;
        duration = _duration;
        hsv = _hsv;
        color = _color;
    }
};

class StrobeRenderCache : public EffectRenderCache {
public:
    StrobeRenderCache() {};
    virtual ~StrobeRenderCache() {};

    std::list<StrobeClass> strobe;
};

void StrobeEffect::SetDefaultParameters()
{
    StrobePanel *sp = (StrobePanel*)panel;
    if (sp == nullptr) {
        return;
    }

    SetSliderValue(sp->Slider_Number_Strobes, 3);
    SetSliderValue(sp->Slider_Strobe_Duration, 10);
    SetSliderValue(sp->Slider_Strobe_Type, 1);
    SetCheckBoxValue(sp->CheckBox_Strobe_Music, false);
}

void StrobeEffect::Render(Effect *effect, SettingsMap &SettingsMap, RenderBuffer &buffer) {
    int Number_Strobes = SettingsMap.GetInt("SLIDER_Number_Strobes", 3);
    int StrobeDuration = SettingsMap.GetInt("SLIDER_Strobe_Duration", 10);
    int Strobe_Type = SettingsMap.GetInt("SLIDER_Strobe_Type", 1);
    bool reactToMusic = SettingsMap.GetBool("CHECKBOX_Strobe_Music", false);

    if (reactToMusic) {
        float f = 0.0;
        if (buffer.GetMedia() != nullptr) {
            std::list<float> const * const pf = buffer.GetMedia()->GetFrameData(buffer.curPeriod, FRAMEDATA_HIGH, "");
            if (pf != nullptr) {
                f = *pf->cbegin();
            }
        }
        Number_Strobes *= f;
    }

    StrobeRenderCache *cache = (StrobeRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new StrobeRenderCache();
        buffer.infoCache[id] = cache;
    }
    std::list<StrobeClass> &strobe = cache->strobe;

    if (StrobeDuration == 0) {
        StrobeDuration = 1;
    }

    int ColorIdx;
    HSVValue hsv;
    size_t colorcnt = buffer.GetColorCount();

    if (buffer.needToInit) {
        buffer.needToInit = false;
        strobe.clear();

        // prepopulate first frame
        for (int i = 0; i < Number_Strobes * StrobeDuration; i++) {
            xlColor color;
            ColorIdx = rand() % colorcnt;
            buffer.palette.GetHSV(ColorIdx, hsv); // take first checked color as color of flash
            buffer.palette.GetColor(ColorIdx, color); // take first checked color as color of flash
            strobe.push_back(StrobeClass(rand() % buffer.BufferWi,
                rand() % buffer.BufferHt, i % StrobeDuration, hsv, color));
        }
    }

    // create new strobe, randomly place a strobe
    while (strobe.size() < Number_Strobes * StrobeDuration) {
        HSVValue hsv;
        xlColor color;
        ColorIdx = rand() % colorcnt;
        buffer.palette.GetHSV(ColorIdx, hsv); // take first checked color as color of flash
        buffer.palette.GetColor(ColorIdx, color); // take first checked color as color of flash
        strobe.push_back(StrobeClass(rand() % buffer.BufferWi,
            rand() % buffer.BufferHt, StrobeDuration, hsv, color));
    }

    // render strobe, we go through all storbes and decide if they should be turned on
    int n = 0;
    std::list<StrobeClass>::iterator it = strobe.begin();
    while (it != strobe.end()) {
        n++;
        hsv = it->hsv;
        xlColor color(it->color);
        int x = it->x;
        int y = it->y;
        if (it->duration > 0) {
            buffer.SetPixel(x, y, color);
        }

        double v = 1.0;
        if (it->duration == 1) {
            v = 0.5;
        } else if (it->duration == 2) {
            v = 0.75;
        }
        if (buffer.allowAlpha) {
            color.alpha = 255.0 * v;
        } else {
            hsv.value *= v;
            color = hsv;
        }

        if (Strobe_Type == 2) {
            int r = rand() % 2;
            if (r == 0) {
                buffer.SetPixel(x, y - 1, color);
                buffer.SetPixel(x, y + 1, color);
            } else {
                buffer.SetPixel(x - 1, y, color);
                buffer.SetPixel(x + 1, y, color);
            }
        }
        if (Strobe_Type == 3) {
            buffer.SetPixel(x, y - 1, color);
            buffer.SetPixel(x, y + 1, color);
            buffer.SetPixel(x - 1, y, color);
            buffer.SetPixel(x + 1, y, color);
        }
        if (Strobe_Type == 4) {
            int r = rand() % 2;
            if (r == 0) {
                buffer.SetPixel(x, y - 1, color);
                buffer.SetPixel(x, y + 1, color);
                buffer.SetPixel(x - 1, y, color);
                buffer.SetPixel(x + 1, y, color);
            } else {
                buffer.SetPixel(x + 1, y - 1, color);
                buffer.SetPixel(x + 1, y + 1, color);
                buffer.SetPixel(x - 1, y - 1, color);
                buffer.SetPixel(x - 1, y + 1, color);
            }
        }

        it->duration--;  // decrease the frame counter on this strobe, when it gets to zero we no longer will turn it on

        if (it->duration <= 0) {
            std::list<StrobeClass>::iterator del = it;
            ++it;
            strobe.erase(del);
        } else {
            ++it;
        }
    }

}
