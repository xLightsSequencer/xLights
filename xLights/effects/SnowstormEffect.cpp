/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "SnowstormEffect.h"
#include "SnowstormPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

#include "../../include/snowstorm-16.xpm"
#include "../../include/snowstorm-24.xpm"
#include "../../include/snowstorm-32.xpm"
#include "../../include/snowstorm-48.xpm"
#include "../../include/snowstorm-64.xpm"

SnowstormEffect::SnowstormEffect(int id) : RenderableEffect(id, "Snowstorm", snowstorm_16, snowstorm_24, snowstorm_32, snowstorm_48, snowstorm_64)
{
    tooltip = "Snow Storm";
}

SnowstormEffect::~SnowstormEffect() {}

wxPanel *SnowstormEffect::CreatePanel(wxWindow *parent) {
    return new SnowstormPanel(parent);
}

class SnowstormClass
{
public:
    std::vector<wxPoint> points;
    HSVValue hsv;
    int idx,ssDecay;
    ~SnowstormClass()
    {
        points.clear();
    }
};

// 0 <= idx <= 7
static wxPoint SnowstormVector(int idx)
{
    wxPoint xy;
    switch (idx) {
    case 0:
        xy.x = -1;
        xy.y = 0;
        break;
    case 1:
        xy.x = -1;
        xy.y = -1;
        break;
    case 2:
        xy.x = 0;
        xy.y = -1;
        break;
    case 3:
        xy.x = 1;
        xy.y = -1;
        break;
    case 4:
        xy.x = 1;
        xy.y = 0;
        break;
    case 5:
        xy.x = 1;
        xy.y = 1;
        break;
    case 6:
        xy.x = 0;
        xy.y = 1;
        break;
    default:
        xy.x = -1;
        xy.y = 1;
        break;
    }
    return xy;
}

static void SnowstormAdvance(RenderBuffer& buffer, SnowstormClass& ssItem)
{
    const int cnt = 8;  // # of integers in each set in arr[]
    const int arr[] = { 30,20,10,5,0,5,10,20,20,15,10,10,10,10,10,15 }; // 2 sets of 8 numbers, each of which add up to 100
    wxPoint adv = SnowstormVector(7);
    int i0 = ssItem.idx % 7 <= 4 ? 0 : cnt;
    int r = rand() % 100;
    for (int i = 0, val = 0; i < cnt; i++)
    {
        val += arr[i0 + i];
        if (r < val)
        {
            adv = SnowstormVector(i);
            break;
        }
    }

    if (ssItem.idx % 3 == 0) {
        adv.x *= 2;
        adv.y *= 2;
    }

    wxPoint xy = ssItem.points.back() + adv;
    xy.x %= buffer.BufferWi;
    xy.y %= buffer.BufferHt;
    if (xy.x < 0) xy.x += buffer.BufferWi;
    if (xy.y < 0) xy.y += buffer.BufferHt;
    ssItem.points.push_back(xy);
}

class SnowstormRenderCache : public EffectRenderCache {
public:
    SnowstormRenderCache() {};
    virtual ~SnowstormRenderCache() {};
    
    int LastSnowstormCount;
    std::list<SnowstormClass> SnowstormItems;
};

void SnowstormEffect::SetDefaultParameters()
{
    SnowstormPanel *sp = (SnowstormPanel*)panel;
    if (sp == nullptr) {
        return;
    }

    SetSliderValue(sp->Slider_Snowstorm_Count, 50);
    SetSliderValue(sp->Slider_Snowstorm_Length, 50);
    SetSliderValue(sp->Slider_Snowstorm_Speed, 10);
}

void SnowstormEffect::Render(Effect* effect, SettingsMap& SettingsMap, RenderBuffer& buffer) {

    int Count = SettingsMap.GetInt("SLIDER_Snowstorm_Count", 50);
    int TailLength = SettingsMap.GetInt("SLIDER_Snowstorm_Length", 50);
    int sSpeed = SettingsMap.GetInt("SLIDER_Snowstorm_Speed", 10);

    float progress = buffer.GetEffectTimeIntervalPosition();
    HSVValue hsv0;
    HSVValue hsv1;
    buffer.palette.GetHSV(0, hsv0, progress);
    buffer.palette.GetHSV(1, hsv1, progress);

    if (TailLength == 0) TailLength = 1;

    SnowstormRenderCache* cache = (SnowstormRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new SnowstormRenderCache();
        buffer.infoCache[id] = cache;
    }
    std::list<SnowstormClass>& SnowstormItems = cache->SnowstormItems;

    if (buffer.needToInit || Count != cache->LastSnowstormCount) {
        buffer.needToInit = false;
        // create snowstorm elements
        cache->LastSnowstormCount = Count;
        SnowstormItems.clear();
        for (int i = 0; i < Count; i++)
        {
            SnowstormClass ssItem;
            ssItem.idx = i;
            ssItem.ssDecay = 0;
            ssItem.points.clear();
            buffer.SetRangeColor(hsv0, hsv1, ssItem.hsv);

            // start in a random state
            int r = rand() % (2 * TailLength);
            if (r > 0) {
                wxPoint xy;
                xy.x = rand() % buffer.BufferWi;
                xy.y = rand() % buffer.BufferHt;
                ssItem.points.push_back(xy);
            }
            if (r >= TailLength) {
                ssItem.ssDecay = r - TailLength;
                r = TailLength;
            }
            for (int j = 1; j < r; j++) {
                SnowstormAdvance(buffer, ssItem);
            }
            SnowstormItems.push_back(ssItem);
        }
    }
    else
    {
        // This updates the colours where using colour curves
        for (auto& it : SnowstormItems) {
            int val = it.hsv.value;
            buffer.SetRangeColor(hsv0, hsv1, it.hsv);
            it.hsv.value = val;
        }
    }

    // render Snowstorm Items
    for (auto& it : SnowstormItems) {
        
        if (it.points.size() > TailLength) {
            if (it.ssDecay > TailLength) {
                it.points.clear();  // start over
                it.ssDecay = 0;
            }
            else if (rand() % 20 < sSpeed) {
                it.ssDecay++;
            }
        }

        if (it.points.empty()) {
            wxPoint xy;
            xy.x = rand() % buffer.BufferWi;
            xy.y = rand() % buffer.BufferHt;
            it.points.push_back(xy);
        }
        else if (rand() % 20 < sSpeed) {
            SnowstormAdvance(buffer, it);
        }

        int sz = it.points.size();
        for (int pt = 0; pt < sz; pt++) {
            HSVValue hsv = it.hsv;
            if (buffer.allowAlpha) {
                xlColor c(hsv);
                c.alpha = 255.8 * (1.0 - double(sz - pt + it.ssDecay) / TailLength);
                buffer.SetPixel(it.points[pt].x, it.points[pt].y, c);
            }
            else {
                hsv.value = 1.0 - double(sz - pt + it.ssDecay) / TailLength;
                if (hsv.value < 0.0) hsv.value = 0.0;
                buffer.SetPixel(it.points[pt].x, it.points[pt].y, hsv);
            }
        }
    }
}
