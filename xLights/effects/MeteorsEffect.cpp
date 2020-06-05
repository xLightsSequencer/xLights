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

wxPanel *MeteorsEffect::CreatePanel(wxWindow *parent) {
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

    int effectState;
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
void MeteorsEffect::Render(Effect *effect, SettingsMap &SettingsMap, RenderBuffer &buffer) {

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
    } else {
        cache->effectState += mSpeed * buffer.frameTimeInMs / 50;
    }
    //wierd calculation, but adds a slight abmount of jitter to the speed
    //example: if mSpeed is 10, the speeds for the first few frames would be:  2, 3, 2...
    int mspeed = cache->effectState / 4;
    cache->effectState -= mspeed * 4;

    switch (MeteorsEffect) {
        case METEORS_DOWN: //0:
        case METEORS_UP: //1:
            RenderMeteorsVertical(buffer, ColorScheme, Count, Length, MeteorsEffect, SwirlIntensity, mspeed);
            break;
        case METEORS_LEFT: //2:
        case METEORS_RIGHT: //3:
            RenderMeteorsHorizontal(buffer, ColorScheme, Count, Length, MeteorsEffect, SwirlIntensity, mspeed);
            break;
        case METEORS_IMPLODE: //4:
            RenderMeteorsImplode(buffer, ColorScheme, Count, Length, SwirlIntensity, mspeed, xoffset, yoffset, fadeWithDistance);
            break;
        case METEORS_EXPLODE: //5:
            RenderMeteorsExplode(buffer, ColorScheme, Count, Length, SwirlIntensity, mspeed, xoffset, yoffset, fadeWithDistance);
            break;
        case METEORS_ICICLES: //6
            RenderIcicleDrip(buffer, ColorScheme, Count, Length, MeteorsEffect, SwirlIntensity, mspeed);
            break;
        case METEORS_ICICLES_BKG: //7
            RenderIcicleDrip(buffer, ColorScheme, Count, -Length, MeteorsEffect, SwirlIntensity, mspeed);
            break;
    }
}

/*
 * *************************************************************
 *  Horizontal
 * *************************************************************
 */

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

void MeteorsEffect::RenderMeteorsHorizontal(RenderBuffer &buffer, int ColorScheme, int Count, int Length, int MeteorsEffect, int SwirlIntensity, int mspeed)
{
    MeteorClass m;
    HSVValue hsv,hsv0,hsv1;
    buffer.palette.GetHSV(0,hsv0);
    buffer.palette.GetHSV(1,hsv1);
    size_t colorcnt=buffer.GetColorCount();
    int TailLength=(buffer.BufferWi < 10) ? Length / 10 : buffer.BufferWi * Length / 100;
    if (TailLength < 1) TailLength=1;

    MeteorsRenderCache *cache = GetCache(buffer, id);

    // create new meteors

    for (int i = 0; i < buffer.BufferHt; i++) {
        if (rand() % 200 < Count) {
            m.x=buffer.BufferWi - 1;
            m.y=i;

            switch (ColorScheme) {
                case 1:
                    buffer.SetRangeColor(hsv0,hsv1,m.hsv);
                    break;
                case 2:
                    buffer.palette.GetHSV(rand()%colorcnt, m.hsv);
                    break;
            }
            cache->meteors.push_back(m);
        }
    }

    // render meteors

    std::function<void(MeteorClass &, int)> f = [&buffer, MeteorsEffect, TailLength, mspeed, SwirlIntensity, ColorScheme] (MeteorClass &meteor, int n) {
        int x,y,dy;
        HSVValue hsv;
        for (int ph = 0; ph <= TailLength; ph++) {
            switch (ColorScheme) {
                case 0:
                    hsv.hue=double(rand() % 1000) / 1000.0;
                    hsv.saturation=1.0;
                    hsv.value=1.0;
                    break;
                default:
                    hsv=meteor.hsv;
                    break;
            }

            double swirl_phase=double(meteor.x)/5.0+double(n)/100.0;
            dy=int(double(SwirlIntensity*buffer.BufferHt)/80.0*buffer.sin(swirl_phase));

            x=meteor.x+ph;
            y=meteor.y+dy;
            if (MeteorsEffect==3) x=buffer.BufferWi-x;

            if (buffer.allowAlpha) {
                xlColor c(hsv);
                c.alpha = 255.0 * (1.0 - double(ph)/TailLength);
                buffer.SetPixel(x,y,c);
            } else {
                hsv.value*= 1.0 - double(ph)/TailLength;
                buffer.SetPixel(x,y,hsv);
            }
        }

        meteor.x -= mspeed;
    };
    parallel_for(cache->meteors, f, 500);

    // delete old meteors
    cache->meteors.remove_if(MeteorHasExpiredX(TailLength));
}

/*
 * *************************************************************
 *  Vertical
 * *************************************************************
 */

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

//predicate to remove variable length meteors (icicles):
class IcicleHasExpired
{
public:
    bool operator()(const MeteorClass& obj) { return obj.y < -obj.h; }
};
//bool end_of_icicle(const MeteorClass& obj) { return obj.y > obj.h; }


void MeteorsEffect::RenderMeteorsVertical(RenderBuffer &buffer, int ColorScheme, int Count, int Length, int MeteorsEffect, int SwirlIntensity, int mspeed)
{
    MeteorClass m;
    HSVValue hsv,hsv0,hsv1;
    buffer.palette.GetHSV(0,hsv0);
    buffer.palette.GetHSV(1,hsv1);
    size_t colorcnt=buffer.GetColorCount();
    int TailLength=(buffer.BufferHt < 10) ? Length / 10 : buffer.BufferHt * Length / 100;
    if (TailLength < 1) TailLength=1;
    MeteorsRenderCache *cache = GetCache(buffer, id);

    // create new meteors

    for (int i = 0; i < buffer.BufferWi; i++) {
        if (rand() % 200 < Count) {
            m.x=i;
            m.y=buffer.BufferHt - 1;

            switch (ColorScheme) {
                case 1:
                    buffer.SetRangeColor(hsv0,hsv1,m.hsv);
                    break;
                case 2:
                    buffer.palette.GetHSV(rand()%colorcnt, m.hsv);
                    break;
            }
            cache->meteors.push_back(m);
        }
    }

    // render meteors

    std::function<void(MeteorClass &, int)> f = [&buffer, MeteorsEffect, TailLength, mspeed, SwirlIntensity, ColorScheme] (MeteorClass &meteor, int n) {
        int x,y,dx;
        HSVValue hsv;
        for (int ph = 0; ph <= TailLength; ph++) {
            switch (ColorScheme) {
                case 0:
                    hsv.hue=double(rand() % 1000) / 1000.0;
                    hsv.saturation=1.0;
                    hsv.value=1.0;
                    break;
                default:
                    hsv=meteor.hsv;
                    break;
            }

            // we adjust x axis with some sine function if swirl1 or swirl2
            // swirling more than 25% of the buffer width doesn't look good
            double swirl_phase=double(meteor.y)/5.0+double(n)/100.0;
            dx=int(double(SwirlIntensity*buffer.BufferWi)/80.0*buffer.sin(swirl_phase));
            x=meteor.x+dx;
            y=meteor.y+ph;
            if (MeteorsEffect==1) y=buffer.BufferHt-y;

            if (buffer.allowAlpha) {
                xlColor c(hsv);
                c.alpha = 255.0 * (1.0 - double(ph)/TailLength);
                buffer.SetPixel(x,y,c);
            } else {
                hsv.value*= 1.0 - double(ph)/TailLength;
                buffer.SetPixel(x,y,hsv);
            }
        }

        meteor.y -= mspeed;
    };
    parallel_for(cache->meteors, f, 500);


    // delete old meteors
    cache->meteors.remove_if(MeteorHasExpiredY(TailLength));
}

#define numents(thing)  (sizeof(thing) / sizeof(thing[0]))

//icicle drip effect, based on RenderMeteorsVertical: -DJ
void MeteorsEffect::RenderIcicleDrip(RenderBuffer &buffer, int ColorScheme, int Count, int Length, int MeteorsEffect, int SwirlIntensity, int mspeed)
{
    bool want_bkg = (Length < 0);
    if (want_bkg) Length = -Length; //kludge; too lazy to add another parameter ;)

    HSVValue hsv,hsv0,hsv1;
    buffer.palette.GetHSV(0,hsv0);
    buffer.palette.GetHSV(1,hsv1);
    size_t colorcnt=buffer.GetColorCount();
    int TailLength=(buffer.BufferHt < 10) ? Length / 10 : buffer.BufferHt * Length / 100;
    if (TailLength < 1) TailLength=1;
    MeteorsRenderCache *cache = GetCache(buffer, id);
    if (buffer.needToInit) {
        buffer.needToInit = false;
        cache->meteors.clear();
    }

    // create new meteors

    MeteorClass m;
    for (int i = 0; i < buffer.BufferWi; i++) {
        if (rand() % 200 < Count) {
            m.x=i;
            m.y=buffer.BufferHt - 1;
            //            m.h = TailLength;
            m.h = (rand() % (2 * buffer.BufferHt))/3; //somewhat variable length -DJ

            switch (ColorScheme) {
                case 1:
                    buffer.SetRangeColor(hsv0,hsv1,m.hsv);
                    break;
                case 2:
                    buffer.palette.GetHSV(rand()%colorcnt, m.hsv);
                    break;
            }
            cache->meteors.push_back(m);
        }
    }

    // render meteors

    //draw some dim icicles for bkg:
    if (want_bkg) {
        xlColor c(100, 50, 255); //light blue
        // HSV hsv = c;
        //        m.hsv.saturation = 0.5;
        //        m.hsv.value = 1.0;
        //c = m.hsv;
        int ystaggered[] = {0, 5, 1, 2, 4};
        for (int x = 0; x < buffer.BufferWi; x += 3)
            for (int y = 0; y < buffer.BufferHt; y += 3)
                buffer.SetPixel(x, y + ystaggered[(x/3) % numents(ystaggered)], c);
    }

    std::function<void(MeteorClass &, int)> f = [&buffer, MeteorsEffect, TailLength, mspeed, SwirlIntensity] (MeteorClass &meteor, int n) {
        int x,y,dx;
        HSVValue hsv;
        for (int ph = 0; ph <= TailLength; ph++) {
            if (!ph || (ph <= meteor.h - meteor.y)) hsv = meteor.hsv; //only make the end of the drip colored
            else { hsv.value = .4; hsv.hue = hsv.saturation = 0; } //white icicle

            // we adjust x axis with some sine function if swirl1 or swirl2
            // swirling more than 25% of the buffer width doesn't look good
            float swirl_phase=float(meteor.y)/5.0f+float(n)/100.0f;
            dx=int(float(SwirlIntensity*buffer.BufferWi)/80.0f*buffer.sin(swirl_phase));

            x=meteor.x+dx;
            y=meteor.y+ph;
            if (MeteorsEffect==1) y=buffer.BufferHt-y;
            if (y < meteor.h) continue; //variable length icicle drips -DJ
            buffer.SetPixel(x,y,hsv);
        }
        meteor.y -= mspeed;
    };
    parallel_for(cache->meteors, f, 500);

    // delete old meteors
    //    meteors.remove_if(MeteorHasExpiredY(TailLength));
    cache->meteors.remove_if(IcicleHasExpired());
}

/*
 * *************************************************************
 *  Implode
 * *************************************************************
 */

class MeteorHasExpiredImplode
{
    int cx, cy;
public:
    MeteorHasExpiredImplode(int centerX, int centerY)
    { cx=centerX; cy=centerY; }

    // operator() is what's called when you do MeteorHasExpired()
    bool operator()(const MeteorRadialClass& obj) {
        return (std::abs(obj.y - cy) < 2) && (std::abs(obj.x - cx) < 2);
    }
};

void MeteorsEffect::RenderMeteorsImplode(RenderBuffer &buffer, int ColorScheme, int Count, int Length, int SwirlIntensity, int mspeed, int xoffset, int yoffset, bool fadeWithDistance)
{
    int truexoffset = xoffset * buffer.BufferWi / 2 / 100;
    int trueyoffset = yoffset * buffer.BufferHt / 2 / 100;

    double angle;
    //int halfdiag=(sqrt( (double)buffer.BufferHt*buffer.BufferHt + buffer.BufferWi*buffer.BufferWi))/2; // 1/2 the length of the diagonal
    int centerX=buffer.BufferWi/2 + truexoffset;
    int centerY=buffer.BufferHt/2 + trueyoffset;
    int maxdiag = std::max(sqrt((0 - centerX)*(0 - centerX) + (0 - centerY)*(0 - centerY)),
        std::max(sqrt((0 - centerX)*(0 - centerX) + (buffer.BufferHt - centerY)*(buffer.BufferHt - centerY)),
            std::max(sqrt((buffer.BufferWi - centerX)*(buffer.BufferWi - centerX) + (0 - centerY)*(0 - centerY)),
                sqrt((buffer.BufferWi - centerX)*(buffer.BufferWi - centerX) + (buffer.BufferHt - centerY)*(buffer.BufferHt - centerY)))));

    MeteorRadialClass m;
    HSVValue hsv,hsv0,hsv1;
    buffer.palette.GetHSV(0,hsv0);
    buffer.palette.GetHSV(1,hsv1);
    size_t colorcnt=buffer.GetColorCount();
    //int TailLength = (halfdiag < 10) ? Length / 10 : halfdiag * Length / 100;
    int TailLength = (maxdiag < 10) ? Length / 10 : maxdiag * Length / 100;
    if (TailLength < 1) TailLength=1;
    int MinDimension = buffer.BufferHt < buffer.BufferWi ? buffer.BufferHt : buffer.BufferWi;
    MeteorsRenderCache *cache = GetCache(buffer, id);

    // create new meteors

    m.cnt=1;
    for (int i = 0; i < MinDimension; i++) {
        if (rand() % 200 < Count) {
            if (buffer.BufferHt == 1) {
                angle=double(rand() % 2) * M_PI;
            } else if (buffer.BufferWi == 1) {
                angle=double(rand() % 2) * M_PI - (M_PI/2.0);
            } else {
                angle=rand01()*2.0*M_PI;
            }
            m.dx=buffer.cos(angle);
            m.dy=buffer.sin(angle);
            //m.x = centerX + double(halfdiag + TailLength)*m.dx;
            //m.y = centerY + double(halfdiag + TailLength)*m.dy;
            m.x = centerX + double(maxdiag + TailLength)*m.dx;
            m.y = centerY + double(maxdiag + TailLength)*m.dy;

            switch (ColorScheme) {
                case 1:
                    buffer.SetRangeColor(hsv0,hsv1,m.hsv);
                    break;
                case 2:
                    buffer.palette.GetHSV(rand()%colorcnt, m.hsv);
                    break;
            }
            cache->meteorsRadial.push_back(m);
        }
    }

    // render meteors

    std::function<void(MeteorRadialClass&, int)> f = [&buffer, fadeWithDistance, centerX, centerY, maxdiag, TailLength, ColorScheme, mspeed](MeteorRadialClass& meteor, int n) {
        int x,y;
        HSVValue hsv;
        float hdistance = 1.0f;
        if (fadeWithDistance) {
            float x = meteor.x;
            float y = meteor.y;
            hdistance = std::max(0.1f, (float)sqrt((x - (float)centerX) * (x - (float)centerX) + (y - (float)centerY) * (y - (float)centerY)) / (float)maxdiag);
        }

        for (int ph = 0; ph <= TailLength; ph++) {
            switch (ColorScheme) {
                case 0:
                    hsv.hue=double(rand() % 1000) / 1000.0;
                    hsv.saturation=1.0;
                    hsv.value=1.0;
                    break;
                default:
                    hsv=meteor.hsv;
                    break;
            }
            // if we were to swirl, it would need to alter the angle here

            x = int(meteor.x-meteor.dx*double(ph));
            y = int(meteor.y-meteor.dy*double(ph));

            // the next line cannot test for exact center! Some lines miss by 1 because of rounding.
            if ((abs(y - centerY) < 2) && (abs(x - centerX) < 2)) break;

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
                c.alpha = 255.0 * (double(ph)/TailLength);
                buffer.SetPixel(x,y,c);
            } else {
                hsv.value*= double(ph)/TailLength;
                buffer.SetPixel(x,y,hsv);
            }
        }

        meteor.x -= meteor.dx*mspeed * hdistance;
        meteor.y -= meteor.dy*mspeed * hdistance;
        meteor.cnt++;
    };
    parallel_for(cache->meteorsRadial, f, 500);

    // delete old meteors
    cache->meteorsRadial.remove_if(MeteorHasExpiredImplode(buffer.BufferWi/2+truexoffset,buffer.BufferHt/2+trueyoffset));
}

/*
 * *************************************************************
 *  Explode
 * *************************************************************
 */

class MeteorHasExpiredExplode
{
    int ht, wi;
public:
    MeteorHasExpiredExplode(int h, int w)
    { ht=h; wi=w; }

    // operator() is what's called when you do MeteorHasExpired()
    bool operator()(const MeteorRadialClass& obj) {
        return obj.y < 0 || obj.x < 0 || obj.y > ht || obj.x > wi;
    }
};

void MeteorsEffect::RenderMeteorsExplode(RenderBuffer &buffer, int ColorScheme, int Count, int Length, int SwirlIntensity, int mspeed, int xoffset, int yoffset, bool fadeWithDistance)
{
    int truexoffset = xoffset * buffer.BufferWi / 2 / 100;
    int trueyoffset = yoffset * buffer.BufferHt / 2 / 100;

    double angle;
    //int halfdiag=(sqrt( (double)buffer.BufferHt*buffer.BufferHt + buffer.BufferWi*buffer.BufferWi))/2; // 1/2 the length of the diagonal
    int centerX = buffer.BufferWi / 2 + truexoffset;
    int centerY = buffer.BufferHt / 2 + trueyoffset;
    int maxdiag = std::max(sqrt((0 - centerX)*(0 - centerX) + (0 - centerY)*(0 - centerY)),
        std::max(sqrt((0 - centerX)*(0 - centerX) + (buffer.BufferHt - centerY)*(buffer.BufferHt - centerY)),
            std::max(sqrt((buffer.BufferWi - centerX)*(buffer.BufferWi - centerX) + (0 - centerY)*(0 - centerY)),
                sqrt((buffer.BufferWi - centerX)*(buffer.BufferWi - centerX) + (buffer.BufferHt - centerY)*(buffer.BufferHt - centerY)))));

    MeteorRadialClass m;
    HSVValue hsv,hsv0,hsv1;
    buffer.palette.GetHSV(0,hsv0);
    buffer.palette.GetHSV(1,hsv1);
    size_t colorcnt=buffer.GetColorCount();
    //int TailLength = (halfdiag < 10) ? Length / 10 : halfdiag * Length / 100;
    int TailLength = (maxdiag < 10) ? Length / 10 : maxdiag * Length / 100;
    if (TailLength < 1) TailLength=1;
    int MinDimension = buffer.BufferHt < buffer.BufferWi ? buffer.BufferHt : buffer.BufferWi;
    MeteorsRenderCache *cache = GetCache(buffer, id);

    // create new meteors

    m.x=buffer.BufferWi/2+truexoffset;
    m.y=buffer.BufferHt/2+trueyoffset;
    m.cnt=1;
    for (int i = 0; i < MinDimension; i++) {
        if (rand() % 200 < Count) {
            if (buffer.BufferHt == 1) {
                angle=double(rand() % 2) * M_PI;
            } else if (buffer.BufferWi == 1) {
                angle=double(rand() % 2) * M_PI - (M_PI/2.0);
            } else {
                angle=rand01()*2.0*M_PI;
            }
            m.dx=buffer.cos(angle);
            m.dy=buffer.sin(angle);

            switch (ColorScheme) {
                case 1:
                    buffer.SetRangeColor(hsv0,hsv1,m.hsv);
                    break;
                case 2:
                    buffer.palette.GetHSV(rand()%colorcnt, m.hsv);
                    break;
            }
            cache->meteorsRadial.push_back(m);
        }
    }

    // render meteors

    std::function<void(MeteorRadialClass&, int)> f = [&buffer, fadeWithDistance, centerX, centerY, maxdiag, TailLength, ColorScheme, mspeed](MeteorRadialClass& meteor, int n) {
        int x,y;
        HSVValue hsv;

        float hdistance = 1.0f;
        if (fadeWithDistance) {
            float x = meteor.x;
            float y = meteor.y;
            hdistance = std::max(0.1f, (float)sqrt((x - (float)centerX) * (x - (float)centerX) + (y - (float)centerY) * (y - (float)centerY)) / (float)maxdiag);
        }

        for(int ph = 0; ph <= TailLength; ph++) {
            //if (ph >= it->cnt) continue;
            switch (ColorScheme) {
                case 0:
                    hsv.hue=double(rand() % 1000) / 1000.0;
                    hsv.saturation=1.0;
                    hsv.value=1.0;
                    break;
                default:
                    hsv=meteor.hsv;
                    break;
            }

            // if we were to swirl, it would need to alter the angle here

            x=int(meteor.x+meteor.dx*double(ph));
            y=int(meteor.y+meteor.dy*double(ph));

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
                c.alpha = 255.0 * (double(ph)/TailLength);
                buffer.SetPixel(x,y,c);
            } else {
                hsv.value*= double(ph)/TailLength;
                buffer.SetPixel(x,y,hsv);
            }
        }

        meteor.x += meteor.dx*mspeed * hdistance;
        meteor.y += meteor.dy*mspeed * hdistance;
        meteor.cnt++;
    };
    parallel_for(cache->meteorsRadial, f, 500);

    // delete old meteors
    cache->meteorsRadial.remove_if(MeteorHasExpiredExplode(buffer.BufferHt,buffer.BufferWi));
}

