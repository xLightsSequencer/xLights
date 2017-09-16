#include "WaveEffect.h"
#include "WavePanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

#include "../../include/wave-16.xpm"
#include "../../include/wave-24.xpm"
#include "../../include/wave-32.xpm"
#include "../../include/wave-48.xpm"
#include "../../include/wave-64.xpm"

WaveEffect::WaveEffect(int id) : RenderableEffect(id, "Wave", wave_16, wave_24, wave_32, wave_48, wave_64)
{
    //ctor
}

WaveEffect::~WaveEffect()
{
    //dtor
}
wxPanel *WaveEffect::CreatePanel(wxWindow *parent) {
    return new WavePanel(parent);
}



#define WAVETYPE_SINE  0
#define WAVETYPE_TRIANGLE  1
#define WAVETYPE_SQUARE  2
#define WAVETYPE_DECAYSINE  3
#define WAVETYPE_IVYFRACTAL  4

//#define WANT_DEBUG_IMPL
//#define WANT_DEBUG  -99 //unbuffered in case app crashes
//#include "djdebug.cpp"
#ifndef debug_function //dummy defs if debug cpp not included above
#define debug(level, ...)
#define debug_more(level, ...)
#define debug_function(level)
#endif


class WaveRenderCache : public EffectRenderCache {
public:
    WaveRenderCache() {};
    virtual ~WaveRenderCache() {};

    std::vector<int> WaveBuffer;
};

static inline int GetWaveType(const std::string & WaveType) {
    if (WaveType == "Sine") {
        return WAVETYPE_SINE;
    } else if (WaveType == "Triangle") {
        return WAVETYPE_TRIANGLE;
    } else if (WaveType == "Square") {
        return WAVETYPE_SQUARE;
    } else if (WaveType == "Decaying Sine") {
        return WAVETYPE_DECAYSINE;
    } else if (WaveType == "Fractal/ivy") {
        return WAVETYPE_IVYFRACTAL;
    }
    return 0;
}
static inline int GetWaveFillColor(const std::string &color) {
    if (color == "Rainbow") {
        return 1;
    } else if (color == "Palette") {
        return 2;
    }
    return 0; //None
}

void WaveEffect::SetDefaultParameters(Model *cls) {
    WavePanel *wp = (WavePanel*)panel;
    if (wp == nullptr) {
        return;
    }

    SetChoiceValue(wp->Choice_Wave_Type, "Sine");
    SetChoiceValue(wp->Choice_Fill_Colors, "None");
    SetCheckBoxValue(wp->CheckBox_Mirror_Wave, false);
    SetSliderValue(wp->Slider_Number_Waves, 900);
    SetSliderValue(wp->Slider_Thickness_Percentage, 5);
    SetSliderValue(wp->Slider_Wave_Height, 50);
    SetSliderValue(wp->Slider_Wave_Speed, 10);
    SetChoiceValue(wp->Choice_Wave_Direction, "Right to Left");
    wp->BitmapButton_Number_WavesVC->SetActive(false);
    wp->BitmapButton_Thickness_PercentageVC->SetActive(false);
    wp->BitmapButton_Wave_HeightVC->SetActive(false);
    wp->BitmapButton_Wave_SpeedVC->SetActive(false);
}

void WaveEffect::Render(Effect *effect, SettingsMap &SettingsMap, RenderBuffer &buffer) {

    float oset = buffer.GetEffectTimeIntervalPosition();

    int WaveType = GetWaveType(SettingsMap["CHOICE_Wave_Type"]);
    int FillColor = GetWaveFillColor(SettingsMap["CHOICE_Fill_Colors"]);

    bool MirrorWave = SettingsMap.GetBool("CHECKBOX_Mirror_Wave");
    int NumberWaves = GetValueCurveInt("Number_Waves", 1, SettingsMap, oset, WAVE_NUMBER_MIN, WAVE_NUMBER_MAX);
    int ThicknessWave = GetValueCurveInt("Thickness_Percentage", 5, SettingsMap, oset, WAVE_THICKNESS_MIN, WAVE_THICKNESS_MAX);
    int WaveHeight = GetValueCurveInt("Wave_Height", 50, SettingsMap, oset, WAVE_HEIGHT_MIN, WAVE_HEIGHT_MAX);
    int wspeed = GetValueCurveInt("Wave_Speed", 10, SettingsMap, oset, WAVE_SPEED_MIN, WAVE_SPEED_MAX);

    bool WaveDirection = "Left to Right" == SettingsMap["CHOICE_Wave_Direction"] ? true : false;


    WaveRenderCache *cache = (WaveRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new WaveRenderCache();
        buffer.infoCache[id] = cache;
    }
    std::vector<int> &WaveBuffer0 = cache->WaveBuffer;

    /*
     WaveType.Add("Sine");       // 0
     WaveType.Add("Triangle");   // 1
     WaveType.Add("Square");     //2
     WaveType.Add("Decaying Sine"); //3
     WaveType.Add("Ivy/fractal"); //4
     FillColors.Add("None");     // 0
     FillColors.Add("Rainbow");  // 1
     FillColors.Add("Palette");  // 2
     */

    int x,y,y1,y2,y1mirror,y2mirror,ystart;
    double r,yc,deltay;
    double degree,radian,degree_per_x;
    static const double pi_180 = 0.01745329;
    xlColor color;
    HSVValue hsv,hsv0,hsv1;
    buffer.palette.GetHSV(0,hsv0);
    buffer.palette.GetHSV(1,hsv1);

    if (NumberWaves == 0) {
        NumberWaves = 1;
    }
    int state = (buffer.curPeriod - buffer.curEffStartPer) * wspeed * buffer.frameTimeInMs / 50;

    yc = buffer.BufferHt/2.0;
    r=yc;
    if (WaveType == WAVETYPE_DECAYSINE) {
        r -= state/4;
        //        if (r < 100./ThicknessWave) r = 100./ThicknessWave; //turn into straight line; don't completely disappear
        if (r < 0) r = 0; //turn into straight line; don't completely disappear
    }
    else if (WaveType == WAVETYPE_IVYFRACTAL) //generate branches at start of effect
    {
        if (!buffer.needToInit || (WaveBuffer0.size() != NumberWaves * buffer.BufferWi)) {
            r = 0;
            debug(10, "regen wave path, state %d", state);
            int delay = 0;
            int delta = 0; //next branch length, angle
            WaveBuffer0.resize(NumberWaves * buffer.BufferWi);
            for (int x1 = 0; x1 < NumberWaves * buffer.BufferWi; ++x1) {
                //                if (delay < 1) angle = (rand() % 45) - 22.5;
                //                int xx = WaveDirection? NumberWaves * BufferWi - x - 1: x;
                WaveBuffer0[x1] = (delay-- > 0) ? WaveBuffer0[x1 - 1] + delta : 2 * yc; 
                if (WaveBuffer0[x1] >= 2 * buffer.BufferHt) { delta = -2; WaveBuffer0[x1] = 2 * buffer.BufferHt - 1; if (delay > 1) delay = 1; }
                if (WaveBuffer0[x1] < 0) { delta = 2; WaveBuffer0[x1] = 0; if (delay > 1) delay = 1; }
                if (delay < 1) {
                    delta = (rand() % 7) - 3;
                    delay = 2 + (rand() % 3);
                }
            }
        }
    }
    degree_per_x = NumberWaves/buffer.BufferWi;
    degree = 1+ state%NumberWaves;
    hsv.saturation=1.0;
    hsv.value=1.0;
    hsv.hue=1.0;
    for (x=0; x<buffer.BufferWi; x++) {
        if (!WaveDirection)
            degree = x * degree_per_x + state; // state causes it to move
        else
            degree = x * degree_per_x - state; // state causes it to move
        radian = degree * pi_180;
        if(WaveType==WAVETYPE_TRIANGLE) { // Triangle
            /*
             .
             .
             .      x
             .     x x
             .    x   x
             .   x     x
             .  x       x
             . x         x
             ******************************************** yc
             .      a
             .
             .
             .
             .
             .
             .
             */

            double waves = ((double)NumberWaves/180.0) / 5; // number of waves
            int amp = buffer.BufferHt * WaveHeight / 100;

            int xx = x;
            if (WaveDirection)
            {
                xx = buffer.BufferWi - x - 1;
            }

            if (amp == 0)
            {
                ystart = 0;
            }
            else
            {
                ystart = (buffer.BufferHt - amp) / 2 + abs((int)((state / 10 + xx) * waves) % (int)(2 * amp) - amp);
            }
            if (ystart > buffer.BufferHt - 1) ystart = buffer.BufferHt - 1;
        } else if (WaveType == WAVETYPE_IVYFRACTAL) {
            int eff_x = (WaveDirection? x: buffer.BufferWi - x - 1) + buffer.BufferWi * (state / 2 / buffer.BufferWi); //effective x before wrap
            if (eff_x >= NumberWaves * buffer.BufferWi) break;
            if (!WaveDirection) eff_x = NumberWaves * buffer.BufferWi - eff_x - 1;
            bool ok = WaveDirection? (eff_x <= state/2): (eff_x >= NumberWaves * buffer.BufferWi - state/2 - 1); //ivy "grows"
            if (!ok) continue;
            ystart = WaveBuffer0[eff_x] / 2;
        } else {
            ystart = (int) (r*(WaveHeight/100.0) * buffer.sin(radian) +yc);
        }

        if(x>=0 && x<buffer.BufferWi && ystart>=0 && ystart <buffer.BufferHt) {
            //  SetPixel(x,ystart,hsv0);  // just leading edge
            /*

             BufferHt
             .
             .
             .
             x <- y2
             x <- ystart. calculated point of wave
             x <- y1
             .
             .
             + < - yc
             .
             .
             x <- y2mirror
             x
             x <- y1mirror
             .
             .
             .
             0
             */


            y1=(int) (ystart - (r*(ThicknessWave/100.0)));
            y2=(int) (ystart + (r*(ThicknessWave/100.0)));
            if (y2 <= y1) y2 = y1 + 1; //minimum height
            y1mirror= yc + (yc -y1);
            y2mirror= yc + (yc -y2);
            deltay = y2-y1;
            wxASSERT(deltay > 0);

            //if (x < 2) debug(10, "wave out: x %d, y %d..%d", x, y1, y2);

            if(WaveType==WAVETYPE_SQUARE) { // Square Wave
                if(buffer.sin(radian)>0.0) {
                    y1=yc+1;
                    y2=yc + yc*(WaveHeight/100.0);
                } else {
                    y1=yc - yc*(WaveHeight/100.0);
                    y2=yc;
                }
            }
            for (y=y1; y<y2; y++) {
                if(FillColor<=0) { //default to this if no selection -DJ
                    buffer.SetPixel(x,y,hsv0);  // fill with color 2
                    //       hsv.hue=(double)(BufferHt-y)/deltay;
                } else if(FillColor==1) {

                    hsv.hue=(double)(y-y1)/deltay;
                    buffer.SetPixel(x,y,hsv); // rainbow
                } else if(FillColor==2) {
                    hsv.hue=(double)(y-y1)/deltay;
                    buffer.GetMultiColorBlend(hsv.hue,false,color);
                    buffer.SetPixel(x,y,color); // palete fill
                }
            }

            if (MirrorWave) {
                if(y1mirror<y2mirror) {
                    y1=y1mirror;
                    y2=y2mirror;
                } else {
                    y2=y1mirror;
                    y1=y2mirror;
                }

                for (y=y1; y<y2; y++) {
                    if(FillColor<=0) { //default to this if no selection -DJ
                        buffer.SetPixel(x,y,hsv0);  // fill with color 2
                        //       hsv.hue=(double)(BufferHt-y)/deltay;
                    } else if(FillColor==1) {

                        hsv.hue=(double)(y-y1)/deltay;
                        buffer.SetPixel(x,y,hsv); // rainbow
                    } else if(FillColor==2) {
                        hsv.hue=(double)(y-y1)/deltay;
                        buffer.GetMultiColorBlend(hsv.hue,false,color);
                        buffer.SetPixel(x,y,color); // palete fill
                    }
                }
            }
        }
    }
}
