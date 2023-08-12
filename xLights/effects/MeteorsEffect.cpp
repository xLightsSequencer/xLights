/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "MeteorsEffect.h"
#include "MeteorsPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../AudioManager.h"
#include "../models/Model.h"
#include "../UtilFunctions.h"

#include "../../include/meteors-16.xpm"
#include "../../include/meteors-24.xpm"
#include "../../include/meteors-32.xpm"
#include "../../include/meteors-48.xpm"
#include "../../include/meteors-64.xpm"
#include "../UtilFunctions.h"

#include "../Parallel.h"

MeteorsEffect::MeteorsEffect(int id) : RenderableEffect(id, "Meteors", meteors_16, meteors_24, meteors_32, meteors_48, meteors_64)
{
    //ctor
}

MeteorsEffect::~MeteorsEffect()
{
    //dtor
}

std::list<std::string> MeteorsEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache)
{
    std::list<std::string> res;

    if (media == nullptr && settings.GetBool("E_CHECKBOX_Meteors_UseMusic", false)) {
        res.push_back(wxString::Format("    WARN: Meteors effect cant follow music if there is no music. Model '%s', Start %s", model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    }

    return res;
}

xlEffectPanel *MeteorsEffect::CreatePanel(wxWindow *parent) {
    return new MeteorsPanel(parent);
}

//these must match list indexes in xLightsMain.h: -DJ
#define METEORS_DOWN  0
#define METEORS_UP  1
#define METEORS_LEFT  2
#define METEORS_RIGHT  3
#define METEORS_IMPLODE  4
#define METEORS_EXPLODE  5
#define METEORS_ICICLES  6 //random length drip effect -DJ
#define METEORS_ICICLES_BKG  7 //with bkg (dim) icicles -DJ


static inline int GetMeteorEffect(const std::string &dir) {
    if (dir == "Down") {
        return 0;
    } else if (dir == "Up") {
        return 1;
    } else if (dir == "Left") {
        return 2;
    } else if (dir == "Right") {
        return 3;
    } else if (dir == "Implode") {
        return 4;
    } else if (dir == "Explode") {
        return 5;
    } else if (dir == "Icicles") {
        return 6;
    } else if (dir == "Icicles + bkg") {
        return 7;
    }
    return 0; //down
}
static inline int GetMeteorColorScheme(const std::string &color) {
    if (color == "Rainbow") {
        return 0;
    } else if (color == "Range") {
        return 1;
    } else if (color == "Palette") {
        return 2;
    }

    return 0;
}

class MeteorClass {
public:

    int x,y;
    HSVValue hsv;
    int h; //variable length; only used for icicle drip -DJ
};

// for radial meteor effect
class MeteorRadialClass {
public:

    double x,y,dx,dy;
    int cnt;
    HSVValue hsv;
};

typedef std::list<MeteorClass> MeteorList;
typedef std::list<MeteorRadialClass> MeteorRadialList;

class MeteorsRenderCache : public EffectRenderCache {
public:
    MeteorsRenderCache() {};
    virtual ~MeteorsRenderCache() {};

    int effectState = 0;
    MeteorList meteors;
    MeteorRadialList meteorsRadial;
};


static MeteorsRenderCache* GetCache(RenderBuffer &buffer, int id) {
    MeteorsRenderCache *cache = (MeteorsRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new MeteorsRenderCache();
        buffer.infoCache[id] = cache;
    }
    return cache;
}

void MeteorsEffect::SetDefaultParameters() {
    MeteorsPanel *mp = (MeteorsPanel*)panel;
    if (mp == nullptr) {
        return;
    }

    mp->BitmapButton_Meteors_Count->SetActive(false);
    mp->BitmapButton_Meteors_Length->SetActive(false);
    mp->BitmapButton_Meteors_Speed->SetActive(false);
    mp->BitmapButton_Meteors_Swirl_Intensity->SetActive(false);
    mp->BitmapButton_Meteors_XOffsetVC->SetActive(false);
    mp->BitmapButton_Meteors_YOffsetVC->SetActive(false);

    SetChoiceValue(mp->Choice_Meteors_Effect, "Down");
    SetChoiceValue(mp->Choice_Meteors_Type, "Rainbow");

    SetSliderValue(mp->Slider_Meteors_Count, 10);
    SetSliderValue(mp->Slider_Meteors_Length, 25);
    SetSliderValue(mp->Slider_Meteors_Swirl_Intensity, 0);
    SetSliderValue(mp->Slider_Meteors_Speed, 10);
    SetSliderValue(mp->Slider_Meteors_XOffset, 0);
    SetSliderValue(mp->Slider_Meteors_YOffset, 0);

    SetCheckBoxValue(mp->CheckBox_Meteors_UseMusic, false);
    SetCheckBoxValue(mp->CheckBox_FadeWithDistance, false);
}

// ColorScheme: 0=rainbow, 1=range, 2=palette
// MeteorsEffect: 0=down, 1=up, 2=left, 3=right, 4=implode, 5=explode
void MeteorsEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {

    float oset = buffer.GetEffectTimeIntervalPosition();
    int Count = GetValueCurveInt("Meteors_Count", 10, SettingsMap, oset, METEORS_COUNT_MIN, METEORS_COUNT_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());

    int Length = GetValueCurveInt("Meteors_Length", 25, SettingsMap, oset, METEORS_LENGTH_MIN, METEORS_LENGTH_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int SwirlIntensity = GetValueCurveInt("Meteors_Swirl_Intensity", 0, SettingsMap, oset, METEORS_SWIRL_MIN, METEORS_SWIRL_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int mSpeed = GetValueCurveInt("Meteors_Speed", 10, SettingsMap, oset, METEORS_SPEED_MIN, METEORS_SPEED_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());

    int MeteorsEffect = GetMeteorEffect(SettingsMap["CHOICE_Meteors_Effect"]);
    int ColorScheme = GetMeteorColorScheme(SettingsMap["CHOICE_Meteors_Type"]);
    int xoffset = GetValueCurveInt("Meteors_XOffset", 0, SettingsMap, oset, METEORS_XOFFSET_MIN, METEORS_XOFFSET_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int yoffset = GetValueCurveInt("Meteors_YOffset", 0, SettingsMap, oset, METEORS_YOFFSET_MIN, METEORS_YOFFSET_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    bool fadeWithDistance = SettingsMap.GetBool("CHECKBOX_FadeWithDistance", false);
    int warmupFrames = SettingsMap.GetInt("SLIDER_Meteors_WamupFrames", 0);

    if (SettingsMap.GetBool("CHECKBOX_Meteors_UseMusic", false)) {
        float f = 0.0;
        if (buffer.GetMedia() != nullptr) {
            std::list<float> const * const pf = buffer.GetMedia()->GetFrameData(buffer.curPeriod, FRAMEDATA_HIGH, "");
            if (pf != nullptr) {
                f = *pf->cbegin();
            }
        }
        Count = (float)Count * f;
    }

    MeteorsRenderCache *cache = GetCache(buffer, id);

    if (buffer.needToInit) {
        buffer.needToInit = false;
        cache->meteors.clear();
        cache->meteorsRadial.clear();
        cache->effectState = mSpeed * buffer.frameTimeInMs / 50;
    }

    switch (MeteorsEffect) {
        case METEORS_DOWN: //0:
        case METEORS_UP: //1:
            RenderMeteorsVertical(buffer, ColorScheme, Count, Length, MeteorsEffect, SwirlIntensity, mSpeed, warmupFrames);
            break;
        case METEORS_LEFT: //2:
        case METEORS_RIGHT: //3:
            RenderMeteorsHorizontal(buffer, ColorScheme, Count, Length, MeteorsEffect, SwirlIntensity, mSpeed, warmupFrames);
            break;
        case METEORS_IMPLODE: //4:
            RenderMeteorsImplode(buffer, ColorScheme, Count, Length, SwirlIntensity, mSpeed, xoffset, yoffset, fadeWithDistance, warmupFrames);
            break;
        case METEORS_EXPLODE: //5:
            RenderMeteorsExplode(buffer, ColorScheme, Count, Length, SwirlIntensity, mSpeed, xoffset, yoffset, fadeWithDistance, warmupFrames);
            break;
        case METEORS_ICICLES: //6
            RenderIcicleDrip(buffer, ColorScheme, Count, Length, MeteorsEffect, SwirlIntensity, mSpeed, warmupFrames);
            break;
        case METEORS_ICICLES_BKG: //7
            RenderIcicleDrip(buffer, ColorScheme, Count, -Length, MeteorsEffect, SwirlIntensity, mSpeed, warmupFrames);
            break;
    }
}

/*
 * *************************************************************
 *  Horizontal
 * *************************************************************
 */

#pragma region Horizontal

class MeteorHasExpiredX
{
    int TailLength;
public:
    MeteorHasExpiredX(int t)
    : TailLength(t)
    {}

    // operator() is what's called when you do MeteorHasExpired()
    bool operator()(const MeteorClass& obj) {
        return obj.x + TailLength < 0;
    }
};

void MeteorsEffect::HorizontalAddMeteors(RenderBuffer& buffer, int ColorScheme, int Count)
{
    MeteorsRenderCache* cache = GetCache(buffer, id);

    HSVValue hsv0, hsv1;
    buffer.palette.GetHSV(0, hsv0);
    buffer.palette.GetHSV(1, hsv1);
    size_t colorcnt = buffer.GetColorCount();

    MeteorClass m;
    for (int i = 0; i < buffer.BufferHt; i++) {
        if (rand() % 200 < Count) {
            m.x = buffer.BufferWi - 1;
            m.y = i;

            switch (ColorScheme) {
            case 1:
                buffer.SetRangeColor(hsv0, hsv1, m.hsv);
                break;
            case 2:
                buffer.palette.GetHSV(rand() % colorcnt, m.hsv);
                break;
            }
            cache->meteors.push_back(m);
        }
    }
}

void MeteorsEffect::HorizontalMoveMeteors(RenderBuffer& buffer, int mspeed)
{
    MeteorsRenderCache* cache = GetCache(buffer, id);

    std::function<void(MeteorClass&, int)> f = [mspeed](MeteorClass& meteor, int n) {
        meteor.x -= mspeed;
    };
    parallel_for(cache->meteors, f, 500);
}

void MeteorsEffect::HorizontalRemoveMeteors(RenderBuffer& buffer, int Length)
{
    MeteorsRenderCache* cache = GetCache(buffer, id);

    int TailLength = (buffer.BufferWi < 10) ? Length / 10 : buffer.BufferWi * Length / 100;
    if (TailLength < 1)
        TailLength = 1;

    // delete old meteors
    cache->meteors.remove_if(MeteorHasExpiredX(TailLength));
}

void MeteorsEffect::RenderMeteorsHorizontal(RenderBuffer& buffer, int ColorScheme, int Count, int Length, int MeteorsEffect, int SwirlIntensity, int mSpeed, int warmupFrames)
{
    MeteorsRenderCache* cache = GetCache(buffer, id);

    if (buffer.curPeriod == buffer.curEffStartPer) {
        for (int i = 0; i < warmupFrames; ++i) {
            cache->effectState += mSpeed * buffer.frameTimeInMs / 50;
            int mspeed = cache->effectState / 4;
            cache->effectState -= mspeed * 4;

            HorizontalAddMeteors(buffer, ColorScheme, Count);
            HorizontalMoveMeteors(buffer, mspeed);
            HorizontalRemoveMeteors(buffer, Length);
        }
    }

    cache->effectState += mSpeed * buffer.frameTimeInMs / 50;
    int speed = cache->effectState / 4;
    cache->effectState -= speed * 4;

    int TailLength = (buffer.BufferWi < 10) ? Length / 10 : buffer.BufferWi * Length / 100;
    if (TailLength < 1)
        TailLength = 1;

    // create new meteors
    HorizontalAddMeteors(buffer, ColorScheme, Count);

    // render meteors

    std::function<void(MeteorClass&, int)> f = [&buffer, MeteorsEffect, TailLength, SwirlIntensity, ColorScheme](MeteorClass& meteor, int n) {
        HSVValue hsv;
        for (int ph = 0; ph <= TailLength; ph++) {
            switch (ColorScheme) {
            case 0:
                hsv.hue = double(rand() % 1000) / 1000.0;
                hsv.saturation = 1.0;
                hsv.value = 1.0;
                break;
            default:
                hsv = meteor.hsv;
                break;
            }

            double swirl_phase = double(meteor.x) / 5.0 + double(n) / 100.0;
            int dy = int(double(SwirlIntensity * buffer.BufferHt) / 80.0 * buffer.sin(swirl_phase));

            int x = meteor.x + ph;
            int y = meteor.y + dy;
            if (MeteorsEffect == 3)
                x = buffer.BufferWi - x;

            if (buffer.allowAlpha) {
                xlColor c(hsv);
                c.alpha = 255.0 * (1.0 - double(ph) / TailLength);
                buffer.SetPixel(x, y, c);
            } else {
                hsv.value *= 1.0 - double(ph) / TailLength;
                buffer.SetPixel(x, y, hsv);
            }
        }
    };
    parallel_for(cache->meteors, f, 500);

    HorizontalMoveMeteors(buffer, speed);

    HorizontalRemoveMeteors(buffer, Length);
}

#pragma endregion

/*
 * *************************************************************
 *  Vertical
 * *************************************************************
 */

#pragma region Vertical

class MeteorHasExpiredY
{
    int TailLength;
public:
    MeteorHasExpiredY(int t)
    : TailLength(t)
    {}

    // operator() is what's called when you do MeteorHasExpired()
    bool operator()(const MeteorClass& obj) {
        return obj.y + TailLength < 0;
    }
};

void MeteorsEffect::VerticalAddMeteors(RenderBuffer& buffer, int ColorScheme, int Count)
{
    MeteorsRenderCache* cache = GetCache(buffer, id);

    HSVValue hsv0, hsv1;
    buffer.palette.GetHSV(0, hsv0);
    buffer.palette.GetHSV(1, hsv1);
    size_t colorcnt = buffer.GetColorCount();

    MeteorClass m;

    for (int i = 0; i < buffer.BufferWi; i++) {
        if (rand() % 200 < Count) {
            m.x = i;
            m.y = buffer.BufferHt - 1;

            switch (ColorScheme) {
            case 1:
                    buffer.SetRangeColor(hsv0, hsv1, m.hsv);
                    break;
            case 2:
                    buffer.palette.GetHSV(rand() % colorcnt, m.hsv);
                    break;
            }
            cache->meteors.push_back(m);
        }
    }
}

void MeteorsEffect::VerticalMoveMeteors(RenderBuffer& buffer, int mspeed)
{
    MeteorsRenderCache* cache = GetCache(buffer, id);

    std::function<void(MeteorClass&, int)> f = [mspeed](MeteorClass& meteor, int n) {
        meteor.y -= mspeed;
    };
    parallel_for(cache->meteors, f, 500);
}

void MeteorsEffect::VerticalRemoveMeteors(RenderBuffer& buffer, int Length)
{
    MeteorsRenderCache* cache = GetCache(buffer, id);

    int TailLength = (buffer.BufferHt < 10) ? Length / 10 : buffer.BufferHt * Length / 100;
    if (TailLength < 1)
        TailLength = 1;

    // delete old meteors
    cache->meteors.remove_if(MeteorHasExpiredY(TailLength));
}

void MeteorsEffect::RenderMeteorsVertical(RenderBuffer& buffer, int ColorScheme, int Count, int Length, int MeteorsEffect, int SwirlIntensity, int mSpeed, int warmupFrames)
{
    MeteorsRenderCache* cache = GetCache(buffer, id);

    if (buffer.curPeriod == buffer.curEffStartPer) {
        for (int i = 0; i < warmupFrames; ++i) {
            cache->effectState += mSpeed * buffer.frameTimeInMs / 50;
            int mspeed = cache->effectState / 4;
            cache->effectState -= mspeed * 4;

            VerticalAddMeteors(buffer, ColorScheme, Count);
            VerticalMoveMeteors(buffer, mspeed);
            VerticalRemoveMeteors(buffer, Length);
        }
    }

    cache->effectState += mSpeed * buffer.frameTimeInMs / 50;
    int speed = cache->effectState / 4;
    cache->effectState -= speed * 4;

    int TailLength = (buffer.BufferHt < 10) ? Length / 10 : buffer.BufferHt * Length / 100;
    if (TailLength < 1)
        TailLength = 1;

    // create new meteors
    VerticalAddMeteors(buffer, ColorScheme, Count);

    // render meteors

    std::function<void(MeteorClass&, int)> f = [&buffer, MeteorsEffect, TailLength, SwirlIntensity, ColorScheme](MeteorClass& meteor, int n) {
        HSVValue hsv;
        for (int ph = 0; ph <= TailLength; ph++) {
            switch (ColorScheme) {
            case 0:
                hsv.hue = double(rand() % 1000) / 1000.0;
                hsv.saturation = 1.0;
                hsv.value = 1.0;
                break;
            default:
                hsv = meteor.hsv;
                break;
            }

            // we adjust x axis with some sine function if swirl1 or swirl2
            // swirling more than 25% of the buffer width doesn't look good
            double swirl_phase = double(meteor.y) / 5.0 + double(n) / 100.0;
            int dx = int(double(SwirlIntensity * buffer.BufferWi) / 80.0 * buffer.sin(swirl_phase));
            int x = meteor.x + dx;
            int y = meteor.y + ph;
            if (MeteorsEffect == 1)
                y = buffer.BufferHt - y;

            if (buffer.allowAlpha) {
                xlColor c(hsv);
                c.alpha = 255.0 * (1.0 - double(ph) / TailLength);
                buffer.SetPixel(x, y, c);
            } else {
                hsv.value *= 1.0 - double(ph) / TailLength;
                buffer.SetPixel(x, y, hsv);
            }
        }
    };
    parallel_for(cache->meteors, f, 500);

    VerticalMoveMeteors(buffer, speed);

    VerticalRemoveMeteors(buffer, Length);
}

#pragma endregion

/*
 * *************************************************************
 *  Icicle
 * *************************************************************
 */

#pragma region Icicle

// predicate to remove variable length meteors (icicles):
class IcicleHasExpired
{
public:
    bool operator()(const MeteorClass& obj)
    {
        return obj.y < -obj.h;
    }
};

void MeteorsEffect::IcicleAddMeteors(RenderBuffer& buffer, int ColorScheme, int Count)
{
    MeteorsRenderCache* cache = GetCache(buffer, id);

    HSVValue hsv0, hsv1;
    buffer.palette.GetHSV(0, hsv0);
    buffer.palette.GetHSV(1, hsv1);
    size_t colorcnt = buffer.GetColorCount();

    MeteorClass m;
    for (int i = 0; i < buffer.BufferWi; i++) {
        if (rand() % 200 < Count) {
            m.x = i;
            m.y = buffer.BufferHt - 1;
            m.h = (rand() % (2 * buffer.BufferHt)) / 3; // somewhat variable length -DJ

            switch (ColorScheme) {
            case 1:
                    buffer.SetRangeColor(hsv0, hsv1, m.hsv);
                    break;
            case 2:
                    buffer.palette.GetHSV(rand() % colorcnt, m.hsv);
                    break;
            }
            cache->meteors.push_back(m);
        }
    }
}

void MeteorsEffect::IcicleMoveMeteors(RenderBuffer& buffer, int mspeed)
{
    MeteorsRenderCache* cache = GetCache(buffer, id);

    std::function<void(MeteorClass&, int)> f = [mspeed](MeteorClass& meteor, int n) {
        meteor.y -= mspeed;
    };
    parallel_for(cache->meteors, f, 500);
}

void MeteorsEffect::IcicleRemoveMeteors(RenderBuffer& buffer)
    {
    MeteorsRenderCache* cache = GetCache(buffer, id);

    // delete old meteors
    cache->meteors.remove_if(IcicleHasExpired());
}

#define numents(thing) (sizeof(thing) / sizeof(thing[0]))
//icicle drip effect, based on RenderMeteorsVertical: -DJ
void MeteorsEffect::RenderIcicleDrip(RenderBuffer& buffer, int ColorScheme, int Count, int Length, int MeteorsEffect, int SwirlIntensity, int mSpeed, int warmupFrames)
{
    MeteorsRenderCache* cache = GetCache(buffer, id);

    if (buffer.curPeriod == buffer.curEffStartPer) {
        for (int i = 0; i < warmupFrames; ++i) {
            cache->effectState += mSpeed * buffer.frameTimeInMs / 50;
            int mspeed = cache->effectState / 4;
            cache->effectState -= mspeed * 4;

            IcicleAddMeteors(buffer, ColorScheme, Count);
            IcicleMoveMeteors(buffer, mspeed);
            IcicleRemoveMeteors(buffer);
        }
    }

    cache->effectState += mSpeed * buffer.frameTimeInMs / 50;
    int speed = cache->effectState / 4;
    cache->effectState -= speed * 4;

    bool want_bkg = (Length < 0);
    if (want_bkg)
        Length = -Length; // kludge; too lazy to add another parameter ;)

    int TailLength = (buffer.BufferHt < 10) ? Length / 10 : buffer.BufferHt * Length / 100;
    if (TailLength < 1)
        TailLength = 1;

    // create new meteors
    IcicleAddMeteors(buffer, ColorScheme, Count);

    // render meteors

    // draw some dim icicles for bkg:
    if (want_bkg) {
        xlColor c(100, 50, 255); // light blue
        // HSV hsv = c;
        //        m.hsv.saturation = 0.5;
        //        m.hsv.value = 1.0;
        // c = m.hsv;
        int ystaggered[] = { 0, 5, 1, 2, 4 };
        for (int x = 0; x < buffer.BufferWi; x += 3)
            for (int y = 0; y < buffer.BufferHt; y += 3)
                    buffer.SetPixel(x, y + ystaggered[(x / 3) % numents(ystaggered)], c);
    }

    std::function<void(MeteorClass&, int)> f = [&buffer, MeteorsEffect, TailLength, SwirlIntensity](MeteorClass& meteor, int n) {
        int x, y, dx;
        HSVValue hsv;
        for (int ph = 0; ph <= TailLength; ph++) {
            if (!ph || (ph <= meteor.h - meteor.y))
                hsv = meteor.hsv; // only make the end of the drip colored
            else {
                hsv.value = .4;
                hsv.hue = hsv.saturation = 0;
            } // white icicle

            // we adjust x axis with some sine function if swirl1 or swirl2
            // swirling more than 25% of the buffer width doesn't look good
            float swirl_phase = float(meteor.y) / 5.0f + float(n) / 100.0f;
            dx = int(float(SwirlIntensity * buffer.BufferWi) / 80.0f * buffer.sin(swirl_phase));

            x = meteor.x + dx;
            y = meteor.y + ph;
            if (MeteorsEffect == 1)
                y = buffer.BufferHt - y;
            if (y < meteor.h)
                continue; // variable length icicle drips -DJ
            buffer.SetPixel(x, y, hsv);
        }
    };
    parallel_for(cache->meteors, f, 500);

    IcicleMoveMeteors(buffer, speed);

    IcicleRemoveMeteors(buffer);
}

#pragma endregion

/*
 * *************************************************************
 *  Implode
 * *************************************************************
 */

#pragma region Implode

class MeteorHasExpiredImplode
{
    int cx, cy;

public:
    MeteorHasExpiredImplode(int centerX, int centerY)
    {
        cx = centerX;
        cy = centerY;
    }

    // operator() is what's called when you do MeteorHasExpired()
    bool operator()(const MeteorRadialClass& obj)
    {
        return (std::abs(obj.y - cy) < 2) && (std::abs(obj.x - cx) < 2);
    }
};

void MeteorsEffect::ImplodeAddMeteors(RenderBuffer& buffer, int ColorScheme, int Count, int Length, int xoffset, int yoffset)
{
    MeteorsRenderCache* cache = GetCache(buffer, id);

    HSVValue hsv0, hsv1;
    buffer.palette.GetHSV(0, hsv0);
    buffer.palette.GetHSV(1, hsv1);
    size_t colorcnt = buffer.GetColorCount();

    int truexoffset = xoffset * buffer.BufferWi / 2 / 100;
    int trueyoffset = yoffset * buffer.BufferHt / 2 / 100;
    int centerX = buffer.BufferWi / 2 + truexoffset;
    int centerY = buffer.BufferHt / 2 + trueyoffset;
    int MinDimension = buffer.BufferHt < buffer.BufferWi ? buffer.BufferHt : buffer.BufferWi;
    int maxdiag = std::max(sqrt((0 - centerX) * (0 - centerX) + (0 - centerY) * (0 - centerY)),
                           std::max(sqrt((0 - centerX) * (0 - centerX) + (buffer.BufferHt - centerY) * (buffer.BufferHt - centerY)),
                                    std::max(sqrt((buffer.BufferWi - centerX) * (buffer.BufferWi - centerX) + (0 - centerY) * (0 - centerY)),
                                             sqrt((buffer.BufferWi - centerX) * (buffer.BufferWi - centerX) + (buffer.BufferHt - centerY) * (buffer.BufferHt - centerY)))));
    int TailLength = (maxdiag < 10) ? Length / 10 : maxdiag * Length / 100;
    if (TailLength < 1)
        TailLength = 1;

    MeteorRadialClass m;
    m.cnt = 1;

    for (int i = 0; i < MinDimension; i++) {
        if (rand() % 200 < Count) {
            double angle;
            if (buffer.BufferHt == 1) {
                angle = double(rand() % 2) * M_PI;
            } else if (buffer.BufferWi == 1) {
                angle = double(rand() % 2) * M_PI - (M_PI / 2.0);
            } else {
                angle = rand01() * 2.0 * M_PI;
            }
            m.dx = buffer.cos(angle);
            m.dy = buffer.sin(angle);
            // m.x = centerX + double(halfdiag + TailLength)*m.dx;
            // m.y = centerY + double(halfdiag + TailLength)*m.dy;
            m.x = centerX + double(maxdiag + TailLength) * m.dx;
            m.y = centerY + double(maxdiag + TailLength) * m.dy;

            switch (ColorScheme) {
            case 1:
                buffer.SetRangeColor(hsv0, hsv1, m.hsv);
                break;
            case 2:
                buffer.palette.GetHSV(rand() % colorcnt, m.hsv);
                break;
            }
            cache->meteorsRadial.push_back(m);
        }
    }
}

void MeteorsEffect::ImplodeMoveMeteors(RenderBuffer& buffer, int mspeed, int xoffset, int yoffset, bool fadeWithDistance)
{
    MeteorsRenderCache* cache = GetCache(buffer, id);

    int truexoffset = xoffset * buffer.BufferWi / 2 / 100;
    int trueyoffset = yoffset * buffer.BufferHt / 2 / 100;
    int centerX = buffer.BufferWi / 2 + truexoffset;
    int centerY = buffer.BufferHt / 2 + trueyoffset;
    int maxdiag = std::max(sqrt((0 - centerX) * (0 - centerX) + (0 - centerY) * (0 - centerY)),
                           std::max(sqrt((0 - centerX) * (0 - centerX) + (buffer.BufferHt - centerY) * (buffer.BufferHt - centerY)),
                                    std::max(sqrt((buffer.BufferWi - centerX) * (buffer.BufferWi - centerX) + (0 - centerY) * (0 - centerY)),
                                             sqrt((buffer.BufferWi - centerX) * (buffer.BufferWi - centerX) + (buffer.BufferHt - centerY) * (buffer.BufferHt - centerY)))));

    std::function<void(MeteorRadialClass&, int)> f = [fadeWithDistance, centerX, centerY, maxdiag, mspeed](MeteorRadialClass& meteor, int n) {
        float hdistance = 1.0f;
        if (fadeWithDistance) {
            float x = meteor.x;
            float y = meteor.y;
            hdistance = std::max(0.1f, (float)sqrt((x - (float)centerX) * (x - (float)centerX) + (y - (float)centerY) * (y - (float)centerY)) / (float)maxdiag);
        }

        meteor.x -= meteor.dx * mspeed * hdistance;
        meteor.y -= meteor.dy * mspeed * hdistance;
        meteor.cnt++;
    };
    parallel_for(cache->meteorsRadial, f, 500);
}

void MeteorsEffect::ImplodeRemoveMeteors(RenderBuffer& buffer, int xoffset, int yoffset)
{
    MeteorsRenderCache* cache = GetCache(buffer, id);

    int truexoffset = xoffset * buffer.BufferWi / 2 / 100;
    int trueyoffset = yoffset * buffer.BufferHt / 2 / 100;

    // delete old meteors
    cache->meteorsRadial.remove_if(MeteorHasExpiredImplode(buffer.BufferWi / 2 + truexoffset, buffer.BufferHt / 2 + trueyoffset));
}

void MeteorsEffect::RenderMeteorsImplode(RenderBuffer& buffer, int ColorScheme, int Count, int Length, int SwirlIntensity, int mSpeed, int xoffset, int yoffset, bool fadeWithDistance, int warmupFrames)
{
    MeteorsRenderCache* cache = GetCache(buffer, id);

    if (buffer.curPeriod == buffer.curEffStartPer) {
        for (int i = 0; i < warmupFrames; ++i) {
            cache->effectState += mSpeed * buffer.frameTimeInMs / 50;
            int mspeed = cache->effectState / 4;
            cache->effectState -= mspeed * 4;

            ImplodeAddMeteors(buffer, ColorScheme, Count, Length, xoffset, yoffset);
            ImplodeMoveMeteors(buffer, mspeed, xoffset, yoffset, fadeWithDistance);
            ImplodeRemoveMeteors(buffer, xoffset, yoffset);
        }
    }

    cache->effectState += mSpeed * buffer.frameTimeInMs / 50;
    int speed = cache->effectState / 4;
    cache->effectState -= speed * 4;

    int truexoffset = xoffset * buffer.BufferWi / 2 / 100;
    int trueyoffset = yoffset * buffer.BufferHt / 2 / 100;

    // int halfdiag=(sqrt( (double)buffer.BufferHt*buffer.BufferHt + buffer.BufferWi*buffer.BufferWi))/2; // 1/2 the length of the diagonal
    int centerX = buffer.BufferWi / 2 + truexoffset;
    int centerY = buffer.BufferHt / 2 + trueyoffset;
    int maxdiag = std::max(sqrt((0 - centerX) * (0 - centerX) + (0 - centerY) * (0 - centerY)),
                           std::max(sqrt((0 - centerX) * (0 - centerX) + (buffer.BufferHt - centerY) * (buffer.BufferHt - centerY)),
                                    std::max(sqrt((buffer.BufferWi - centerX) * (buffer.BufferWi - centerX) + (0 - centerY) * (0 - centerY)),
                                             sqrt((buffer.BufferWi - centerX) * (buffer.BufferWi - centerX) + (buffer.BufferHt - centerY) * (buffer.BufferHt - centerY)))));

    int TailLength = (maxdiag < 10) ? Length / 10 : maxdiag * Length / 100;
    if (TailLength < 1)
        TailLength = 1;

    // create new meteors
    ImplodeAddMeteors(buffer, ColorScheme, Count, Length, xoffset, yoffset);

    // render meteors
    std::function<void(MeteorRadialClass&, int)> f = [&buffer, fadeWithDistance, centerX, centerY, maxdiag, TailLength, ColorScheme](MeteorRadialClass& meteor, int n) {
        HSVValue hsv;

        for (int ph = 0; ph <= TailLength; ph++) {
            switch (ColorScheme) {
            case 0:
                hsv.hue = double(rand() % 1000) / 1000.0;
                hsv.saturation = 1.0;
                hsv.value = 1.0;
                break;
            default:
                hsv = meteor.hsv;
                break;
            }
            // if we were to swirl, it would need to alter the angle here

            int x = int(meteor.x - meteor.dx * double(ph));
            int y = int(meteor.y - meteor.dy * double(ph));

            // the next line cannot test for exact center! Some lines miss by 1 because of rounding.
            if ((abs(y - centerY) < 2) && (abs(x - centerX) < 2))
                break;

            if (fadeWithDistance) {
                // distance
                int distance = sqrt((x - centerX) * (x - centerX) + (y - centerY) * (y - centerY));
                if (distance < 10) {
                    distance = 10;
                }
                hsv.value *= double(distance) / maxdiag;
            }

            if (buffer.allowAlpha) {
                xlColor c(hsv);
                c.alpha = 255.0 * (double(ph) / TailLength);
                buffer.SetPixel(x, y, c);
            } else {
                hsv.value *= double(ph) / TailLength;
                buffer.SetPixel(x, y, hsv);
            }
        }
    };
    parallel_for(cache->meteorsRadial, f, 500);

    ImplodeMoveMeteors(buffer, speed, xoffset, yoffset, fadeWithDistance);

    ImplodeRemoveMeteors(buffer, xoffset, yoffset);
}
#pragma endregion

/*
 * *************************************************************
 *  Explode
 * *************************************************************
 */
#pragma region Explode

class MeteorHasExpiredExplode
{
    int ht, wi;

public:
    MeteorHasExpiredExplode(int h, int w)
    {
        ht = h;
        wi = w;
    }

    // operator() is what's called when you do MeteorHasExpired()
    bool operator()(const MeteorRadialClass& obj)
    {
        return obj.y < 0 || obj.x < 0 || obj.y > ht || obj.x > wi;
    }
};

void MeteorsEffect::ExplodeAddMeteors(RenderBuffer& buffer, int ColorScheme, int Count, int xoffset, int yoffset)
{
    MeteorsRenderCache* cache = GetCache(buffer, id);
    size_t colorcnt = buffer.GetColorCount();

    int truexoffset = xoffset * buffer.BufferWi / 2 / 100;
    int trueyoffset = yoffset * buffer.BufferHt / 2 / 100;

    HSVValue hsv0, hsv1;
    buffer.palette.GetHSV(0, hsv0);
    buffer.palette.GetHSV(1, hsv1);

    int MinDimension = buffer.BufferHt < buffer.BufferWi ? buffer.BufferHt : buffer.BufferWi;

    MeteorRadialClass m;
    m.x = buffer.BufferWi / 2 + truexoffset;
    m.y = buffer.BufferHt / 2 + trueyoffset;
    m.cnt = 1;
    for (int i = 0; i < MinDimension; i++) {
        if (rand() % 200 < Count) {
            double angle;
            if (buffer.BufferHt == 1) {
                    angle = double(rand() % 2) * M_PI;
            } else if (buffer.BufferWi == 1) {
                    angle = double(rand() % 2) * M_PI - (M_PI / 2.0);
            } else {
                    angle = rand01() * 2.0 * M_PI;
            }
            m.dx = buffer.cos(angle);
            m.dy = buffer.sin(angle);

            switch (ColorScheme) {
            case 1:
                    buffer.SetRangeColor(hsv0, hsv1, m.hsv);
                    break;
            case 2:
                    buffer.palette.GetHSV(rand() % colorcnt, m.hsv);
                    break;
            }
            cache->meteorsRadial.push_back(m);
        }
    }
}

void MeteorsEffect::ExplodeMoveMeteors(RenderBuffer& buffer, int mspeed, int xoffset, int yoffset, bool fadeWithDistance)
{
    MeteorsRenderCache* cache = GetCache(buffer, id);

    int truexoffset = xoffset * buffer.BufferWi / 2 / 100;
    int trueyoffset = yoffset * buffer.BufferHt / 2 / 100;
    int centerX = buffer.BufferWi / 2 + truexoffset;
    int centerY = buffer.BufferHt / 2 + trueyoffset;

    int maxdiag = std::max(sqrt((0 - centerX) * (0 - centerX) + (0 - centerY) * (0 - centerY)),
                           std::max(sqrt((0 - centerX) * (0 - centerX) + (buffer.BufferHt - centerY) * (buffer.BufferHt - centerY)),
                                    std::max(sqrt((buffer.BufferWi - centerX) * (buffer.BufferWi - centerX) + (0 - centerY) * (0 - centerY)),
                                             sqrt((buffer.BufferWi - centerX) * (buffer.BufferWi - centerX) + (buffer.BufferHt - centerY) * (buffer.BufferHt - centerY)))));

    std::function<void(MeteorRadialClass&, int)> f = [fadeWithDistance, centerX, centerY, maxdiag, mspeed](MeteorRadialClass& meteor, int n) {
        float hdistance = 1.0f;
        if (fadeWithDistance) {
            float x = meteor.x;
            float y = meteor.y;
            hdistance = std::max(0.1f, (float)sqrt((x - (float)centerX) * (x - (float)centerX) + (y - (float)centerY) * (y - (float)centerY)) / (float)maxdiag);
        }

        meteor.x += meteor.dx * mspeed * hdistance;
        meteor.y += meteor.dy * mspeed * hdistance;
        meteor.cnt++;
    };
    parallel_for(cache->meteorsRadial, f, 500);
}

void MeteorsEffect::ExplodeRemoveMeteors(RenderBuffer& buffer)
{
    MeteorsRenderCache* cache = GetCache(buffer, id);

    // delete old meteors
    cache->meteorsRadial.remove_if(MeteorHasExpiredExplode(buffer.BufferHt, buffer.BufferWi));
}

void MeteorsEffect::RenderMeteorsExplode(RenderBuffer& buffer, int ColorScheme, int Count, int Length, int SwirlIntensity, int mSpeed, int xoffset, int yoffset, bool fadeWithDistance, int warmupFrames)
{
    MeteorsRenderCache* cache = GetCache(buffer, id);

    if (buffer.curPeriod == buffer.curEffStartPer) {
        for (int i = 0; i < warmupFrames; ++i) {
            cache->effectState += mSpeed * buffer.frameTimeInMs / 50;
            int mspeed = cache->effectState / 4;
            cache->effectState -= mspeed * 4;

            ExplodeAddMeteors(buffer, ColorScheme, Count, xoffset, yoffset);
            ExplodeMoveMeteors(buffer, mspeed, xoffset, yoffset, fadeWithDistance);
            ExplodeRemoveMeteors(buffer);
        }
    }

    cache->effectState += mSpeed * buffer.frameTimeInMs / 50;
    int speed = cache->effectState / 4;
    cache->effectState -= speed * 4;

    int truexoffset = xoffset * buffer.BufferWi / 2 / 100;
    int trueyoffset = yoffset * buffer.BufferHt / 2 / 100;

    // int halfdiag=(sqrt( (double)buffer.BufferHt*buffer.BufferHt + buffer.BufferWi*buffer.BufferWi))/2; // 1/2 the length of the diagonal
    int centerX = buffer.BufferWi / 2 + truexoffset;
    int centerY = buffer.BufferHt / 2 + trueyoffset;
    int maxdiag = std::max(sqrt((0 - centerX) * (0 - centerX) + (0 - centerY) * (0 - centerY)),
                           std::max(sqrt((0 - centerX) * (0 - centerX) + (buffer.BufferHt - centerY) * (buffer.BufferHt - centerY)),
                                    std::max(sqrt((buffer.BufferWi - centerX) * (buffer.BufferWi - centerX) + (0 - centerY) * (0 - centerY)),
                                             sqrt((buffer.BufferWi - centerX) * (buffer.BufferWi - centerX) + (buffer.BufferHt - centerY) * (buffer.BufferHt - centerY)))));

    HSVValue hsv, hsv0, hsv1;
    buffer.palette.GetHSV(0, hsv0);
    buffer.palette.GetHSV(1, hsv1);
    int TailLength = (maxdiag < 10) ? Length / 10 : maxdiag * Length / 100;
    if (TailLength < 1)
        TailLength = 1;

    // create new meteors
    ExplodeAddMeteors(buffer, ColorScheme, Count, xoffset, yoffset);

    // render meteors

    std::function<void(MeteorRadialClass&, int)> f = [&buffer, fadeWithDistance, centerX, centerY, maxdiag, TailLength, ColorScheme](MeteorRadialClass& meteor, int n) {
        HSVValue hsv;
        for (int ph = 0; ph <= TailLength; ph++) {
            // if (ph >= it->cnt) continue;
            switch (ColorScheme) {
            case 0:
                hsv.hue = double(rand() % 1000) / 1000.0;
                hsv.saturation = 1.0;
                hsv.value = 1.0;
                break;
            default:
                hsv = meteor.hsv;
                break;
            }

            // if we were to swirl, it would need to alter the angle here

            int x = int(meteor.x + meteor.dx * double(ph));
            int y = int(meteor.y + meteor.dy * double(ph));

            if (fadeWithDistance) {
                // distance
                int distance = sqrt((x - centerX) * (x - centerX) + (y - centerY) * (y - centerY));
                if (distance < 10) {
                    distance = 10;
                }
                hsv.value *= double(distance) / maxdiag;
            }

            if (buffer.allowAlpha) {
                xlColor c(hsv);
                c.alpha = 255.0 * (double(ph) / TailLength);
                buffer.SetPixel(x, y, c);
            } else {
                hsv.value *= double(ph) / TailLength;
                buffer.SetPixel(x, y, hsv);
            }
        }
    };
    parallel_for(cache->meteorsRadial, f, 500);

    ExplodeMoveMeteors(buffer, speed, xoffset, yoffset, fadeWithDistance);

    ExplodeRemoveMeteors(buffer);
}

#pragma endregion
