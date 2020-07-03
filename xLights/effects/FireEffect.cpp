/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "FireEffect.h"
#include "FirePanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../AudioManager.h"
#include "../models/Model.h"
#include "../UtilFunctions.h"

#include "../../include/fire-16.xpm"
#include "../../include/fire-24.xpm"
#include "../../include/fire-32.xpm"
#include "../../include/fire-48.xpm"
#include "../../include/fire-64.xpm"

FireEffect::FireEffect(int id) : RenderableEffect(id, "Fire", fire_16, fire_24, fire_32, fire_48, fire_64)
{
    //ctor
}

FireEffect::~FireEffect()
{
    //dtor
}

std::list<std::string> FireEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache)
{
    std::list<std::string> res;

    if (media == nullptr && settings.GetBool("E_CHECKBOX_Fire_GrowWithMusic", false))
    {
        res.push_back(wxString::Format("    WARN: Fire effect cant grow to music if there is no music. Model '%s', Start %s", model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    }

    return res;
}

wxPanel *FireEffect::CreatePanel(wxWindow *parent) {
    return new FirePanel(parent);
}

bool FireEffect::needToAdjustSettings(const std::string &version)
{
    return IsVersionOlder("2018.44", version);
}

void FireEffect::adjustSettings(const std::string &version, Effect *effect, bool removeDefaults)
{
    SettingsMap &settings = effect->GetSettings();

    wxString growthcycles = settings.Get("E_VALUECURVE_Fire_GrowthCycles", "");

    if (growthcycles.Contains("Active=TRUE"))
    {
        ValueCurve vc(growthcycles);
        vc.SetLimits(FIRE_GROWTHCYCLES_MIN, FIRE_GROWTHCYCLES_MAX);
        vc.SetDivisor(FIRE_GROWTHCYCLES_DIVISOR);
        vc.FixScale(10);
        settings["E_VALUECURVE_Fire_GrowthCycles"] = vc.Serialise();
    }

    // also give the base class a chance to adjust any settings
    if (RenderableEffect::needToAdjustSettings(version))
    {
        RenderableEffect::adjustSettings(version, effect, removeDefaults);
    }
}

class FirePaletteClass {
public:
    FirePaletteClass() {
        HSVValue hsv;
        int i;
        // calc 100 reds, black to bright red
        hsv.hue=0.0;
        hsv.saturation=1.0;
        for (i=0; i<100; i++)
        {
            hsv.value=double(i)/100.0;
            firePalette.push_back(hsv);
            firePaletteColors.push_back(hsv);
            firePaletteColorsAlpha.push_back(xlColor(255, 0, 0, i * 255 / 100));
        }

        // gives 100 hues red to yellow
        hsv.value=1.0;
        for (i=0; i<100; i++)
        {
            firePalette.push_back(hsv);
            firePaletteColors.push_back(hsv);
            firePaletteColorsAlpha.push_back(hsv);
            hsv.hue+=0.00166666;
        }
    }
    int size() const {
        return firePalette.size();
    }
    const HSVValue &operator[](int x) const {
        return firePalette[x];
    }
    const xlColor &asColor(int x) const {
        return firePaletteColors[x];
    }
    const xlColor &asAlphaColor(int x) const {
        return firePaletteColorsAlpha[x];
    }

private:
    hsvVector firePalette;
    xlColorVector firePaletteColors;
    xlColorVector firePaletteColorsAlpha;
};
static const FirePaletteClass FirePalette;

// 0 <= x < BufferWi
// 0 <= y < BufferHt
void SetFireBuffer(int x, int y, int PaletteIdx, std::vector<int> &FireBuffer, int maxWi, int maxHi)
{
    if (x >= 0 && x < maxWi && y >= 0 && y < maxHi)
    {
        FireBuffer[y*maxWi+x] = PaletteIdx;
    }
}

// 0 <= x < BufferWi
// 0 <= y < BufferHt
int GetFireBuffer(int x, int y, std::vector<int> &FireBuffer, int maxWi, int maxHi)
{
    if (x >= 0 && x < maxWi && y >= 0 && y < maxHi)
    {
        return FireBuffer[y*maxWi+x];
    }
    return -1;
}

static int GetLocation(const std::string &location) {
    if (location == "Bottom") {
        return 0;
    } else if (location == "Top") {
        return 1;
    } else if (location == "Left") {
        return 2;
    } else if (location == "Right") {
        return 3;
    }
    return 0;
}



class FireRenderCache : public EffectRenderCache {
public:
    FireRenderCache() {};
    virtual ~FireRenderCache() {};

    std::vector<int> FireBuffer;
};

static FireRenderCache* GetCache(RenderBuffer &buffer, int id) {
    FireRenderCache *cache = (FireRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new FireRenderCache();
        buffer.infoCache[id] = cache;
    }
    return cache;
}

void FireEffect::SetDefaultParameters() {
    FirePanel *fp = (FirePanel*)panel;
    if (fp == nullptr) {
        return;
    }

    fp->BitmapButton_Fire_GrowthCyclesVC->SetActive(false);
    fp->BitmapButton_Fire_HeightVC->SetActive(false);
    fp->BitmapButton_Fire_HueShiftVC->SetActive(false);

    SetSliderValue(fp->Slider_Fire_Height, 50);
    SetSliderValue(fp->Slider_Fire_HueShift, 0);
    SetSliderValue(fp->Slider_Fire_GrowthCycles, 0);

    SetCheckBoxValue(fp->CheckBox_Fire_GrowWithMusic, false);
}

// 10 <= HeightPct <= 100
void FireEffect::Render(Effect *effect, SettingsMap &SettingsMap, RenderBuffer &buffer) {

    float offset = buffer.GetEffectTimeIntervalPosition();
    int HeightPct = GetValueCurveInt("Fire_Height", 50, SettingsMap, offset, FIRE_HEIGHT_MIN, FIRE_HEIGHT_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int HueShift = GetValueCurveInt("Fire_HueShift", 0, SettingsMap, offset, FIRE_HUE_MIN, FIRE_HUE_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    float cycles = GetValueCurveDouble("Fire_GrowthCycles", 0.0f, SettingsMap, offset, FIRE_GROWTHCYCLES_MIN, FIRE_GROWTHCYCLES_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), FIRE_GROWTHCYCLES_DIVISOR);
    bool withMusic = SettingsMap.GetBool("CHECKBOX_Fire_GrowWithMusic", false);

    int x,y;
    int loc = GetLocation(SettingsMap.Get("CHOICE_Fire_Location", "Bottom"));

    if (withMusic)
    {
        HeightPct = 10;
        if (buffer.GetMedia() != nullptr)
        {
            float f = 0.0;
            std::list<float> const * const pf = buffer.GetMedia()->GetFrameData(buffer.curPeriod, FRAMEDATA_HIGH, "");
            if (pf != nullptr)
            {
                f = *pf->cbegin();
            }
            HeightPct += 90 * f;
        }
    }
    else
    {
        //cycles is 0 - 200 representing growth cycle count of 0 - 20
        if (cycles > 0) {
            double adjust = buffer.GetEffectTimeIntervalPosition(cycles);
            adjust = 0.5 - std::abs(adjust - 0.5);
            HeightPct += adjust * 100;
        }
    }
    if (HeightPct<1) HeightPct=1;

    int maxMHt = buffer.ModelBufferHt;
    int maxMWi = buffer.ModelBufferWi;
    if (loc == 2 || loc == 3) {
        maxMHt = buffer.ModelBufferWi;
        maxMWi = buffer.ModelBufferHt;
    }

    int maxHt = buffer.BufferHt;
    int maxWi = buffer.BufferWi;
    if (loc == 2 || loc == 3) {
        maxHt = buffer.BufferWi;
        maxWi = buffer.BufferHt;
    }

    if (maxMHt<1) maxMHt = 1;
    if (maxHt<1) maxHt=1;

    FireRenderCache *cache = GetCache(buffer, id);

    float mod_state = 4.0;
    if (buffer.needToInit) {
        buffer.needToInit = false;
        cache->FireBuffer.resize(maxMHt * maxMWi);
        for (size_t i=0; i < cache->FireBuffer.size(); i++) {
            cache->FireBuffer[i]=0;
        }
    } else {
        mod_state = 4 / (buffer.curPeriod%4+1);
    }
    // build fire
    for (x=0; x<maxMWi; x++) {
        int r = x%2==0 ? 190+(rand() % 10) : 100+(rand() % 50);
        SetFireBuffer(x,0,r, cache->FireBuffer, maxMWi, maxMHt);
    }
    int step=255*100/maxHt/HeightPct;
    for (y=1; y<maxHt; y++)
    {
        for (x=0; x<maxWi; x++)
        {
            int v1 = GetFireBuffer(x-1,y-1, cache->FireBuffer, maxMWi, maxMHt);
            int v2 = GetFireBuffer(x+1,y-1, cache->FireBuffer, maxMWi, maxMHt);
            int v3 = GetFireBuffer(x,y-1, cache->FireBuffer, maxMWi, maxMHt);
            int v4 = GetFireBuffer(x,y-1, cache->FireBuffer, maxMWi, maxMHt);
            int n = 0;
            int sum = 0;
            if(v1>=0)
            {
                sum+=v1;
                n++;
            }
            if(v2>=0)
            {
                sum+=v2;
                n++;
            }
            if(v3>=0)
            {
                sum+=v3;
                n++;
            }
            if(v4>=0)
            {
                sum+=v4;
                n++;
            }
            int new_index = n > 0 ? sum / n : 0;
            if (new_index > 0)
            {
                new_index+=(rand() % 100 < 20) ? step : -step;
                if (new_index < 0) new_index=0;
                if (new_index >= FirePalette.size()) new_index = FirePalette.size()-1;
            }
            SetFireBuffer(x,y,new_index, cache->FireBuffer, maxMWi, maxMHt);
        }
    }

    //  Now play fire
    for (y=0; y<maxHt; y++)
    {
        for (x=0; x<maxWi; x++)
        {
            int xp = x;
            int yp = y;
            if (loc == 1 || loc == 3) {
                yp = maxHt - y - 1;
            }
            if (loc == 2 || loc == 3) {
                int t = xp;
                xp = yp;
                yp = t;
            }
            if (HueShift>0) {
                HSVValue hsv = FirePalette[GetFireBuffer(x,y, cache->FireBuffer, maxMWi, maxMHt)];
                hsv.hue = hsv.hue +(HueShift/100.0);
                if (hsv.hue>1.0) hsv.hue=1.0;
                if (buffer.allowAlpha) {
                    xlColor c(hsv);
                    c.alpha = FirePalette.asAlphaColor(GetFireBuffer(x,y, cache->FireBuffer, maxMWi, maxMHt)).Alpha();
                    buffer.SetPixel(xp, yp, c);
                } else {
                    buffer.SetPixel(xp, yp, hsv);
                }
            } else {
                if (buffer.allowAlpha) {
                    buffer.SetPixel(xp, yp, FirePalette.asAlphaColor(GetFireBuffer(x,y, cache->FireBuffer, maxMWi, maxMHt)));
                } else {
                    buffer.SetPixel(xp, yp, FirePalette.asColor(GetFireBuffer(x,y, cache->FireBuffer, maxMWi, maxMHt)));
                }
            }
        }
    }
}
