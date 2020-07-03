/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "ShimmerEffect.h"
#include "ShimmerPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

#include "../../include/shimmer.xpm"

ShimmerEffect::ShimmerEffect(int id) : RenderableEffect(id, "Shimmer", shimmer, shimmer, shimmer, shimmer, shimmer)
{
    //ctor
}

ShimmerEffect::~ShimmerEffect()
{
    //dtor
}

wxPanel *ShimmerEffect::CreatePanel(wxWindow *parent) {
    return new ShimmerPanel(parent);
}

void ShimmerEffect::SetDefaultParameters()
{
    ShimmerPanel *sp = (ShimmerPanel*)panel;
    if (sp == nullptr) {
        return;
    }

    sp->BitmapButton_Shimmer_CyclesVC->SetActive(false);
    sp->BitmapButton_Shimmer_Duty_FactorVC->SetActive(false);

    SetSliderValue(sp->Slider_Shimmer_Duty_Factor, 50);
    SetSliderValue(sp->Slider_Shimmer_Cycles, 10);

    SetCheckBoxValue(sp->CheckBox_Shimmer_Use_All_Colors, false);
    SetCheckBoxValue(sp->CheckBox_PRE_2017_7, false);
}

bool ShimmerEffect::needToAdjustSettings(const std::string &version)
{
    return IsVersionOlder("2017.7", version);
}

void ShimmerEffect::adjustSettings(const std::string &version, Effect *effect, bool removeDefaults)
{
    if (IsVersionOlder("2017.7", version))
    {
        SettingsMap &settings = effect->GetSettings();

        int old = settings.GetInt("E_CHECKBOX_PRE_2017_7", 2);
        if (old == 2)
        {
            settings["E_CHECKBOX_PRE_2017_7"] = "1";
        }
    }

    // also give the base class a chance to adjust any settings
    if (RenderableEffect::needToAdjustSettings(version))
    {
        RenderableEffect::adjustSettings(version, effect, removeDefaults);
    }
}

void ShimmerEffect::Render(Effect* effect, SettingsMap& SettingsMap, RenderBuffer& buffer) {

    float oset = buffer.GetEffectTimeIntervalPosition();
    int Duty_Factor = GetValueCurveInt("Shimmer_Duty_Factor", 50, SettingsMap, oset, SHIMMER_DUTYFACTOR_MIN, SHIMMER_DUTYFACTOR_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    bool Use_All_Colors = SettingsMap.GetBool("CHECKBOX_Shimmer_Use_All_Colors", false);
    double cycles = GetValueCurveDouble("Shimmer_Cycles", 1.0, SettingsMap, oset, SHIMMER_CYCLES_MIN, SHIMMER_CYCLES_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), 10);
    bool pre2017_7 = SettingsMap.GetBool("CHECKBOX_PRE_2017_7", false);
    int colorcnt = buffer.GetColorCount();

    int ColorIdx = 0;
    if (pre2017_7)
    {
        double position = buffer.GetEffectTimeIntervalPosition(cycles);

        ColorIdx = round(position * 0.999 * (double)colorcnt);

        double pos2 = position * colorcnt;
        while (pos2 > 1.0) {
            pos2 -= 1.0;
        }
        if (pos2 * 100 > Duty_Factor) {
            return;
        }
    }
    else
    {
        // If cycles are too high maximise out at on and off
        if (cycles > ((double)buffer.curEffEndPer - (double)buffer.curEffStartPer) / 2.0)
        {
            ColorIdx = (buffer.curPeriod - buffer.curEffStartPer) % (2 * colorcnt);
            if (ColorIdx % 2 == 0)
            {
                ColorIdx /= 2;
            }
            else
            {
                return;
            }
        }
        else
        {
            double position = buffer.GetEffectTimeIntervalPosition(cycles);
            if (position > 1.0) position = 0.0;

            // black if we are beyond the duty factor
            if (position >= (double)Duty_Factor / 100.0) return;

            // now we need to work out the color

            // scale up the position
            int cycle = ((buffer.curPeriod - buffer.curEffStartPer) * cycles) / (buffer.curEffEndPer - buffer.curEffStartPer);
            ColorIdx = cycle % colorcnt;
        }
    }

    xlColor color;
    buffer.palette.GetColor(ColorIdx, color);
    for (int y = 0; y < buffer.BufferHt; y++) {
        for (int x = 0; x < buffer.BufferWi; x++) {
            if (Use_All_Colors) { // Should we randomly assign colors from palette or cycle thru sequentially?
                ColorIdx = rand() % colorcnt; // Select random numbers from 0 up to number of colors the user has checked. 0-5 if 6 boxes checked
                buffer.palette.GetColor(ColorIdx, color); // Now go and get the hsv value for this ColorIdx
            }
            else
            {
                buffer.palette.GetSpatialColor(ColorIdx, (float)x / (float)buffer.BufferWi, (float)y / (float)buffer.BufferHt, color);
            }

            buffer.SetPixel(x, y, color); // Turn pixel
        }
    }
}
