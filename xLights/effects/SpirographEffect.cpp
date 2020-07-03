/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "SpirographEffect.h"
#include "SpirographPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"


#include "../../include/spirograph-16.xpm"
#include "../../include/spirograph-24.xpm"
#include "../../include/spirograph-32.xpm"
#include "../../include/spirograph-48.xpm"
#include "../../include/spirograph-64.xpm"

SpirographEffect::SpirographEffect(int id) : RenderableEffect(id, "Spirograph", spirograph_16, spirograph_24, spirograph_32, spirograph_48, spirograph_64)
{
    //ctor
}

SpirographEffect::~SpirographEffect()
{
    //dtor
}
wxPanel *SpirographEffect::CreatePanel(wxWindow *parent) {
    return new SpirographPanel(parent);
}

void SpirographEffect::SetDefaultParameters()
{
    SpirographPanel *sp = (SpirographPanel*)panel;
    if (sp == nullptr) {
        return;
    }

    sp->BitmapButton_Spirograph_AnimateVC->SetActive(false);
    sp->BitmapButton_Spirograph_LengthVC->SetActive(false);
    sp->BitmapButton_Spirograph_WidthVC->SetActive(false);
    sp->BitmapButton_Spirograph_dVC->SetActive(false);
    sp->BitmapButton_Spirograph_RVC->SetActive(false);
    sp->BitmapButton_Spirograph_rVC->SetActive(false);
    sp->BitmapButton_Spirograph_SpeedrVC->SetActive(false);

    SetSliderValue(sp->Slider_Spirograph_Speed, 10);
    SetSliderValue(sp->Slider_Spirograph_R, 20);
    SetSliderValue(sp->Slider_Spirograph_r, 10);
    SetSliderValue(sp->Slider_Spirograph_d, 30);
    SetSliderValue(sp->Slider_Spirograph_Animate, 0);
    SetSliderValue(sp->Slider_Spirograph_Length, 20);
    SetSliderValue(sp->Slider_Spirograph_Width, 1);
}

void SpirographEffect::Render(Effect* effect, SettingsMap& SettingsMap, RenderBuffer& buffer) {

    float oset = buffer.GetEffectTimeIntervalPosition();

    int int_R = GetValueCurveInt("Spirograph_R", 20, SettingsMap, oset, SPIROGRAPH_R_MIN, SPIROGRAPH_R_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int int_r = GetValueCurveInt("Spirograph_r", 10, SettingsMap, oset, SPIROGRAPH_r_MIN, SPIROGRAPH_r_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int int_d = GetValueCurveInt("Spirograph_d", 30, SettingsMap, oset, SPIROGRAPH_d_MIN, SPIROGRAPH_d_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int Animate = GetValueCurveInt("Spirograph_Animate", 0, SettingsMap, oset, SPIROGRAPH_ANIMATE_MIN, SPIROGRAPH_ANIMATE_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int sspeed = GetValueCurveInt("Spirograph_Speed", 10, SettingsMap, oset, SPIROGRAPH_SPEED_MIN, SPIROGRAPH_SPEED_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int length = GetValueCurveInt("Spirograph_Length", 20, SettingsMap, oset, SPIROGRAPH_LENGTH_MIN, SPIROGRAPH_LENGTH_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int width = GetValueCurveInt("Spirograph_Width", 1, SettingsMap, oset, SPIROGRAPH_WIDTH_MIN, SPIROGRAPH_WIDTH_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());

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
    wxASSERT(step != 0);
    wxASSERT(stepw != 0);
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

        if (ColorIdx >= colorcnt) ColorIdx = colorcnt - 1;

        buffer.palette.GetHSV(ColorIdx, hsv); // Now go and get the hsv value for this ColorIdx

        buffer.palette.GetHSV(0, hsv0);
        ColorIdx = (state + rand()) % colorcnt; // Select random numbers from 0 up to number of colors the user has checked. 0-5 if 6 boxes checked
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