/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "WaveEffect.h"

#include <cmath>
#include <cstdlib>
#include <format>

#include "render/ValueCurve.h"
#include "../render/Effect.h"
#include "../render/RenderBuffer.h"
#include "UtilClasses.h"
#include "UtilFunctions.h"

#include "../../include/wave-16.xpm"
#include "../../include/wave-24.xpm"
#include "../../include/wave-32.xpm"
#include "../../include/wave-48.xpm"
#include "../../include/wave-64.xpm"

// Fallback defaults (used until OnMetadataLoaded replaces them with Wave.json values).
std::string WaveEffect::sWaveTypeDefault = "Sine";
std::string WaveEffect::sFillColorsDefault = "None";
bool WaveEffect::sMirrorWaveDefault = false;
// Number_Waves is a float count of wave cycles. JSON stores min/max as the
// pre-divisor degree values (180..3600) so the VC scale is compatible with
// older sequences; the divisor (360) converts to "cycles" at render time.
double WaveEffect::sNumberWavesDefault = 2.5;
double WaveEffect::sNumberWavesMin = 180;
double WaveEffect::sNumberWavesMax = 3600;
int WaveEffect::sNumberWavesDivisor = 360;
int WaveEffect::sThicknessDefault = 5;
int WaveEffect::sThicknessMin = 0;
int WaveEffect::sThicknessMax = 100;
int WaveEffect::sWaveHeightDefault = 50;
int WaveEffect::sWaveHeightMin = 0;
int WaveEffect::sWaveHeightMax = 100;
double WaveEffect::sWaveSpeedDefault = 10.0;
double WaveEffect::sWaveSpeedMin = 0;
double WaveEffect::sWaveSpeedMax = 5000;
int WaveEffect::sWaveSpeedDivisor = 100;
std::string WaveEffect::sWaveDirectionDefault = "Right to Left";
int WaveEffect::sYOffsetDefault = 0;
int WaveEffect::sYOffsetMin = -250;
int WaveEffect::sYOffsetMax = 250;

WaveEffect::WaveEffect(int id) : RenderableEffect(id, "Wave", wave_16, wave_24, wave_32, wave_48, wave_64)
{
    //ctor
}

WaveEffect::~WaveEffect()
{
    //dtor
}

void WaveEffect::OnMetadataLoaded()
{
    sWaveTypeDefault = GetStringDefault("Wave_Type", sWaveTypeDefault);
    sFillColorsDefault = GetStringDefault("Fill_Colors", sFillColorsDefault);
    sMirrorWaveDefault = GetBoolDefault("Mirror_Wave", sMirrorWaveDefault);
    sNumberWavesDefault = GetDoubleDefault("Number_Waves", sNumberWavesDefault);
    sNumberWavesMin = GetMinFromMetadata("Number_Waves", sNumberWavesMin);
    sNumberWavesMax = GetMaxFromMetadata("Number_Waves", sNumberWavesMax);
    sNumberWavesDivisor = GetDivisorFromMetadata("Number_Waves", sNumberWavesDivisor);
    sThicknessDefault = GetIntDefault("Thickness_Percentage", sThicknessDefault);
    sThicknessMin = (int)GetMinFromMetadata("Thickness_Percentage", sThicknessMin);
    sThicknessMax = (int)GetMaxFromMetadata("Thickness_Percentage", sThicknessMax);
    sWaveHeightDefault = GetIntDefault("Wave_Height", sWaveHeightDefault);
    sWaveHeightMin = (int)GetMinFromMetadata("Wave_Height", sWaveHeightMin);
    sWaveHeightMax = (int)GetMaxFromMetadata("Wave_Height", sWaveHeightMax);
    sWaveSpeedDefault = GetDoubleDefault("Wave_Speed", sWaveSpeedDefault);
    sWaveSpeedMin = GetMinFromMetadata("Wave_Speed", sWaveSpeedMin);
    sWaveSpeedMax = GetMaxFromMetadata("Wave_Speed", sWaveSpeedMax);
    sWaveSpeedDivisor = GetDivisorFromMetadata("Wave_Speed", sWaveSpeedDivisor);
    sWaveDirectionDefault = GetStringDefault("Wave_Direction", sWaveDirectionDefault);
    sYOffsetDefault = GetIntDefault("Wave_YOffset", sYOffsetDefault);
    sYOffsetMin = (int)GetMinFromMetadata("Wave_YOffset", sYOffsetMin);
    sYOffsetMax = (int)GetMaxFromMetadata("Wave_YOffset", sYOffsetMax);
}
bool WaveEffect::needToAdjustSettings(const std::string& version)
{
    return IsVersionOlder("2026.05.2", version);
}

void WaveEffect::adjustSettings(const std::string& version, Effect* effect, bool removeDefaults)
{
    SettingsMap& settings = effect->GetSettings();

    if (IsVersionOlder("2022.06", version)) {
        // speed was changed from an integer to a float with 2 decimal places so the value must be multiplied by 100 to be the same as it was
        std::string speed = settings.Get("E_SLIDER_Wave_Speed", "");
        if (speed != "") {
            settings.erase("E_SLIDER_Wave_Speed");
            settings["E_TEXTCTRL_Wave_Speed"] = std::to_string(std::strtol(speed.c_str(), nullptr, 10));
        } else {
            speed = settings.Get("E_VALUECURVE_Wave_Speed", "");
            if (Contains(speed, "Active=TRUE")) {
                // its a value curve
                ValueCurve vc(speed);
                vc.ConvertDivider(1, 100);
                settings["E_VALUECURVE_Wave_Speed"] = vc.Serialise();
            }
        }
    }

    if (IsVersionOlder("2026.05.2", version)) {
        // Number_Waves was an integer degree count (e.g. 900 = 2.5 cycles).
        // It is now a float cycles slider with divisor 360, stored as a
        // TEXTCTRL float. The VC form is unchanged (Min/Max/P1/P2 remain in
        // degree space; Render applies the divisor at read time), but the
        // plain slider key must migrate to the float representation.
        std::string numWaves = settings.Get("E_SLIDER_Number_Waves", "");
        if (!numWaves.empty()) {
            long degrees = std::strtol(numWaves.c_str(), nullptr, 10);
            settings.erase("E_SLIDER_Number_Waves");
            settings["E_TEXTCTRL_Number_Waves"] = std::format("{:.2f}", degrees / 360.0);
        }
    }
}

#define WAVETYPE_SINE  0
#define WAVETYPE_TRIANGLE  1
#define WAVETYPE_SQUARE  2
#define WAVETYPE_DECAYSINE  3
#define WAVETYPE_IVYFRACTAL  4



class WaveRenderCache : public EffectRenderCache {
public:
    WaveRenderCache() = default;
    ~WaveRenderCache() override = default;

    std::vector<int> WaveBuffer;
    float state = 0.0f;
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

void WaveEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {

    float oset = buffer.GetEffectTimeIntervalPosition();

    int WaveType = GetWaveType(SettingsMap.Get("CHOICE_Wave_Type", sWaveTypeDefault));
    int FillColor = GetWaveFillColor(SettingsMap.Get("CHOICE_Fill_Colors", sFillColorsDefault));

    bool MirrorWave = SettingsMap.GetBool("CHECKBOX_Mirror_Wave", sMirrorWaveDefault);
    // Number_Waves is a float "cycles" slider (divisor 360). The underlying
    // wave math uses degrees across the buffer width, so multiply back up.
    double waveCycles = GetValueCurveDouble("Number_Waves", sNumberWavesDefault, SettingsMap, oset, sNumberWavesMin, sNumberWavesMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), sNumberWavesDivisor);
    int NumberWaves = (int)std::round(waveCycles * sNumberWavesDivisor);
    int ThicknessWave = GetValueCurveInt("Thickness_Percentage", sThicknessDefault, SettingsMap, oset, sThicknessMin, sThicknessMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int WaveHeight = GetValueCurveInt("Wave_Height", sWaveHeightDefault, SettingsMap, oset, sWaveHeightMin, sWaveHeightMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    float wspeed = GetValueCurveDouble("Wave_Speed", sWaveSpeedDefault, SettingsMap, oset, sWaveSpeedMin, sWaveSpeedMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), sWaveSpeedDivisor);
    int yoffset = GetValueCurveInt("Wave_YOffset", sYOffsetDefault, SettingsMap, oset, sYOffsetMin, sYOffsetMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());

    bool WaveDirection = "Left to Right" == SettingsMap.Get("CHOICE_Wave_Direction", sWaveDirectionDefault) ? true : false;

    double WaveYOffset = (buffer.BufferHt / 2.0) * (yoffset * 0.01);
    int roundedWaveYOffset = std::round(WaveYOffset);

    WaveRenderCache *cache = (WaveRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new WaveRenderCache();
        buffer.infoCache[id] = cache;
    }
    std::vector<int> &WaveBuffer0 = cache->WaveBuffer;

    int y, ystart;
    double deltay;
    static const double pi_180 = 0.01745329;
    xlColor color;
    HSVValue hsv, hsv0, hsv1;
    buffer.palette.GetHSV(0, hsv0);
    buffer.palette.GetHSV(1, hsv1);

    if (NumberWaves == 0) {
        NumberWaves = 1;
    }
    if (buffer.needToInit) {
        cache->state = 0.0f;
        buffer.needToInit = false;
    }
    cache->state += wspeed * ((float)buffer.frameTimeInMs / 50.0f);
    float state = cache->state;

    double yc = buffer.BufferHt / 2.0;
    double r = yc;
    if (WaveType == WAVETYPE_DECAYSINE) {
        r -= state / 4;
        //        if (r < 100./ThicknessWave) r = 100./ThicknessWave; //turn into straight line; don't completely disappear
        if (r < 0) r = 0; //turn into straight line; don't completely disappear
    } else if (WaveType == WAVETYPE_IVYFRACTAL) { //generate branches at start of effect
        if (buffer.needToInit || ((int)WaveBuffer0.size() != NumberWaves * buffer.BufferWi)) {
            r = 0;
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
            buffer.needToInit = false;
        }
    }
    double degree_per_x = static_cast<double>(NumberWaves) / buffer.BufferWi;
    hsv.saturation = 1.0;
    hsv.value = 1.0;
    hsv.hue = 1.0;
    for (int x = 0; x < buffer.BufferWi; x++) {
        double degree;
        if (!WaveDirection)
            degree = x * degree_per_x + state; // state causes it to move
        else
            degree = x * degree_per_x - state; // state causes it to move
        double radian = degree * pi_180;

        double degreeMinus1;
        if (!WaveDirection)
            degreeMinus1 = (x - 1) * degree_per_x + state; // state causes it to move
        else
            degreeMinus1 = (x - 1) * degree_per_x - state; // state causes it to move
        double radianMinus1 = degreeMinus1 * pi_180;

        double sinrad = buffer.sin(radian);
        double sinradMinus1 = buffer.sin(radianMinus1);

        if (WaveType == WAVETYPE_TRIANGLE) { // Triangle
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

            double waves = ((double)NumberWaves / 180.0) / 5; // number of waves
            int amp = buffer.BufferHt * WaveHeight / 100;

            int xx = x;
            if (WaveDirection) {
                xx = buffer.BufferWi - x - 1;
            }

            if (amp == 0) {
                ystart = 0;
            } else {
                ystart = (buffer.BufferHt - amp) / 2 + abs((int)((state / 10 + xx) * waves) % (int)(2 * amp) - amp);
            }
            if (ystart > buffer.BufferHt - 1) ystart = buffer.BufferHt - 1;
        } else if (WaveType == WAVETYPE_IVYFRACTAL) {
            int istate = std::round(state);
            int eff_x = (WaveDirection ? x : buffer.BufferWi - x - 1) + buffer.BufferWi * (istate / 2 / buffer.BufferWi); //effective x before wrap
            if (eff_x >= NumberWaves * buffer.BufferWi) break;
            if (!WaveDirection) eff_x = NumberWaves * buffer.BufferWi - eff_x - 1;
            bool ok = WaveDirection ? (eff_x <= istate / 2) : (eff_x >= NumberWaves * buffer.BufferWi - istate / 2 - 1); //ivy "grows"
            if (!ok) continue;
            ystart = WaveBuffer0[eff_x] / 2;
        } else {
            ystart = (int)(r*(WaveHeight / 100.0) * sinrad + yc);
        }

        if (x >= 0 && x < buffer.BufferWi && ystart >= 0 && ystart < buffer.BufferHt) {
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


            int y1 = (int)(ystart - (r*(ThicknessWave / 100.0)));
            int y2 = (int)(ystart + (r*(ThicknessWave / 100.0)));
            if (y2 <= y1) y2 = y1 + 1; //minimum height

            //if (x < 2) debug(10, "wave out: x %d, y %d..%d", x, y1, y2);

            if (WaveType == WAVETYPE_SQUARE) { // Square Wave
                if (std::signbit(sinrad) != std::signbit(sinradMinus1)) {
                    y1 = yc - yc * (WaveHeight / 100.0);
                    y2 = yc + yc * (WaveHeight / 100.0);
                } else if (sinrad > 0.0) {
                    y1 = yc + 1 + yc * (WaveHeight / 100.0) * ((100.0 - ThicknessWave) / 100.0);
                    y2 = yc + yc * (WaveHeight / 100.0);
                } else {
                    y1 = yc - yc * (WaveHeight / 100.0);
                    y2 = yc - yc * (WaveHeight / 100.0) * ((100.0 - ThicknessWave) / 100.0);
                }

                if (y1 < 0) y1 = 0;
                if (y2 < 1) y2 = 1;
                if (y1 > buffer.BufferHt - 1) y1 = buffer.BufferHt - 1;
                if (y2 > buffer.BufferHt) y2 = buffer.BufferHt;

                if (y2 <= y1) {
                    y2 = y1 + 1;
                }
            }

            int y1mirror = yc + (yc - y1);
            int y2mirror = yc + (yc - y2);
            deltay = y2 - y1;

            for (y = y1; y <= y2; y++) {
                int adjustedY = y + roundedWaveYOffset;
                if (FillColor <= 0) { //default to this if no selection -DJ
                    buffer.SetPixel(x, adjustedY, hsv0);  // fill with color 2
                } else if (FillColor == 1) {

                    hsv.hue = (double)(y - y1) / deltay;
                    buffer.SetPixel(x, adjustedY, hsv); // rainbow
                } else if (FillColor == 2) {
                    hsv.hue = (double)(y - y1) / deltay;
                    buffer.GetMultiColorBlend(hsv.hue, false, color);
                    buffer.SetPixel(x, adjustedY, color); // palete fill
                }
            }

            if (MirrorWave) {

                if (y1mirror < y2mirror) {
                    y1 = y1mirror;
                    y2 = y2mirror;
                } else {
                    y2 = y1mirror;
                    y1 = y2mirror;
                }

                for (y = y1; y <= y2; y++) {
                    int adjustedY = y + roundedWaveYOffset;
                    if (FillColor <= 0) { //default to this if no selection -DJ
                        buffer.SetPixel(x, adjustedY, hsv0);  // fill with color 2
                    } else if (FillColor == 1) {

                        hsv.hue = (double)(y - y1) / deltay;
                        buffer.SetPixel(x, adjustedY, hsv); // rainbow
                    } else if (FillColor == 2) {
                        hsv.hue = (double)(y - y1) / deltay;
                        buffer.GetMultiColorBlend(hsv.hue, false, color);
                        buffer.SetPixel(x, adjustedY, color); // palete fill
                    }
                }
            }
        }
    }
}
