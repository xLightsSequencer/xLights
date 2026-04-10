/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <cassert>

#include "SpirographEffect.h"

#include "../render/Effect.h"
#include "../render/RenderBuffer.h"
#include "UtilClasses.h"

#include "../../include/spirograph-16.xpm"
#include "../../include/spirograph-24.xpm"
#include "../../include/spirograph-32.xpm"
#include "../../include/spirograph-48.xpm"
#include "../../include/spirograph-64.xpm"

// Fallback defaults (used until OnMetadataLoaded replaces them with Spirograph.json values).
int SpirographEffect::sRDefault = 20;
int SpirographEffect::sRMin = 1;
int SpirographEffect::sRMax = 100;
int SpirographEffect::srDefault = 10;
int SpirographEffect::srMin = 1;
int SpirographEffect::srMax = 100;
int SpirographEffect::sdDefault = 30;
int SpirographEffect::sdMin = 1;
int SpirographEffect::sdMax = 100;
int SpirographEffect::sAnimateDefault = 0;
int SpirographEffect::sAnimateMin = -50;
int SpirographEffect::sAnimateMax = 50;
int SpirographEffect::sSpeedDefault = 10;
int SpirographEffect::sSpeedMin = 0;
int SpirographEffect::sSpeedMax = 50;
int SpirographEffect::sLengthDefault = 20;
int SpirographEffect::sLengthMin = 0;
int SpirographEffect::sLengthMax = 50;
int SpirographEffect::sWidthDefault = 1;
int SpirographEffect::sWidthMin = 1;
int SpirographEffect::sWidthMax = 50;

SpirographEffect::SpirographEffect(int id) : RenderableEffect(id, "Spirograph", spirograph_16, spirograph_24, spirograph_32, spirograph_48, spirograph_64)
{
    //ctor
}

SpirographEffect::~SpirographEffect()
{
    //dtor
}

void SpirographEffect::OnMetadataLoaded()
{
    sRDefault = GetIntDefault("Spirograph_R", sRDefault);
    sRMin = (int)GetMinFromMetadata("Spirograph_R", sRMin);
    sRMax = (int)GetMaxFromMetadata("Spirograph_R", sRMax);
    srDefault = GetIntDefault("Spirograph_r", srDefault);
    srMin = (int)GetMinFromMetadata("Spirograph_r", srMin);
    srMax = (int)GetMaxFromMetadata("Spirograph_r", srMax);
    sdDefault = GetIntDefault("Spirograph_d", sdDefault);
    sdMin = (int)GetMinFromMetadata("Spirograph_d", sdMin);
    sdMax = (int)GetMaxFromMetadata("Spirograph_d", sdMax);
    sAnimateDefault = GetIntDefault("Spirograph_Animate", sAnimateDefault);
    sAnimateMin = (int)GetMinFromMetadata("Spirograph_Animate", sAnimateMin);
    sAnimateMax = (int)GetMaxFromMetadata("Spirograph_Animate", sAnimateMax);
    sSpeedDefault = GetIntDefault("Spirograph_Speed", sSpeedDefault);
    sSpeedMin = (int)GetMinFromMetadata("Spirograph_Speed", sSpeedMin);
    sSpeedMax = (int)GetMaxFromMetadata("Spirograph_Speed", sSpeedMax);
    sLengthDefault = GetIntDefault("Spirograph_Length", sLengthDefault);
    sLengthMin = (int)GetMinFromMetadata("Spirograph_Length", sLengthMin);
    sLengthMax = (int)GetMaxFromMetadata("Spirograph_Length", sLengthMax);
    sWidthDefault = GetIntDefault("Spirograph_Width", sWidthDefault);
    sWidthMin = (int)GetMinFromMetadata("Spirograph_Width", sWidthMin);
    sWidthMax = (int)GetMaxFromMetadata("Spirograph_Width", sWidthMax);
}

void SpirographEffect::Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer) {

    float oset = buffer.GetEffectTimeIntervalPosition();

    int int_R = GetValueCurveInt("Spirograph_R", sRDefault, SettingsMap, oset, sRMin, sRMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int int_r = GetValueCurveInt("Spirograph_r", srDefault, SettingsMap, oset, srMin, srMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int int_d = GetValueCurveInt("Spirograph_d", sdDefault, SettingsMap, oset, sdMin, sdMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int Animate = GetValueCurveInt("Spirograph_Animate", sAnimateDefault, SettingsMap, oset, sAnimateMin, sAnimateMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int sspeed = GetValueCurveInt("Spirograph_Speed", sSpeedDefault, SettingsMap, oset, sSpeedMin, sSpeedMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int length = GetValueCurveInt("Spirograph_Length", sLengthDefault, SettingsMap, oset, sLengthMin, sLengthMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int width = GetValueCurveInt("Spirograph_Width", sWidthDefault, SettingsMap, oset, sWidthMin, sWidthMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());

    int d_mod;
    HSVValue hsv, hsv0, hsv1;
    size_t colorcnt = buffer.GetColorCount();

    int state = (buffer.curPeriod - buffer.curEffStartPer) * sspeed * buffer.frameTimeInMs / 50;
    double animateState = double((buffer.curPeriod - buffer.curEffStartPer) * Animate * buffer.frameTimeInMs) / 5000.0;

    length = length * 18;

    float xc = ((float)buffer.BufferWi / 2.0); // 20x100 flex strips with 2 fols per strip = 40x50
    float yc = ((float)buffer.BufferHt / 2.0);
    float R = xc * (int_R / 100.0);   //  Radius of the large circle just fits in the width of model
    float r = xc * (int_r / 100.0); // start little circle at 1/4 of max width
    if (r == 0) {
        // little r cannot be 0 or we get a divide by 0 / INF
        r = 0.00001f;
    }
    if (r > R) r = R;
    float d = xc * (int_d / 100.0);

    //  palette.GetHSV(1, hsv1);
    //
    //    A hypotrochoid is a roulette traced by a point attached to a circle of radius r rolling around the inside of a fixed circle of radius R, where the point is a distance d from the center of the interior circle.
    //The parametric equations for a hypotrochoid are:[citation needed]
    //
    //  more info: http://en.wikipedia.org/wiki/Hypotrochoid
    //
    //x(t) = (R-r) * cos t + d*buffer.cos ((R-r/r)*t);
    //y(t) = (R-r) * sin t + d*buffer.sin ((R-r/r)*t);

    int mod1440 = state % 1440;
    float d_orig = d;
    if (Animate) d = d_orig + animateState * d_orig; // should we modify the distance variable each pass through?
    float step = 1.0 / width;
    float stepw = 1.0 / (log10(width) + 1);
    assert(step != 0);
    assert(stepw != 0);
    for (float i = 1.0; i <= length; i += step)
    {
        float t = (i + mod1440) * M_PI / 180.0;
        float x = (R - r) * buffer.cos(t) + d * buffer.cos(((R - r) / r) * t) + xc;
        float y = (R - r) * buffer.sin(t) + d * buffer.sin(((R - r) / r) * t) + yc;

        if (colorcnt > 0) d_mod = (int)buffer.BufferWi / colorcnt;
        else d_mod = 1;

        double x2 = pow((double)(x - xc), 2.0);
        double y2 = pow((double)(y - yc), 2.0);
        double hyp = (sqrt(x2 + y2) / buffer.BufferWi) * 100.0;
        int ColorIdx = (int)(hyp / d_mod); // Select random numbers from 0 up to number of colors the user has checked. 0-5 if 6 boxes checked

        if (ColorIdx >= (int)colorcnt) ColorIdx = (int)colorcnt - 1;

        buffer.palette.GetHSV(ColorIdx, hsv); // Now go and get the hsv value for this ColorIdx

        buffer.palette.GetHSV(0, hsv0);
        ColorIdx = (colorcnt > 0) ? (state + rand()) % colorcnt : 0; // Select random numbers from 0 up to number of colors the user has checked. 0-5 if 6 boxes checked
        buffer.palette.GetHSV(ColorIdx, hsv1); // Now go and get the hsv value for this ColorIdx

        // work out the normal to the point being drawn
        float tt = ((R - r) / r) * t;
        for (float w = -width / 2; w <= width / 2; w += stepw)
        {
            int xx = x + w * buffer.cos(tt);
            int yy = y + w * buffer.sin(tt);
            buffer.SetPixel(xx, yy, hsv);
        }
    }
}
