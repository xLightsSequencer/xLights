/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "SpiralsEffect.h"
#include "SpiralsPanel.h"
#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

#include "../../include/spirals-16.xpm"
#include "../../include/spirals-24.xpm"
#include "../../include/spirals-32.xpm"
#include "../../include/spirals-48.xpm"
#include "../../include/spirals-64.xpm"

SpiralsEffect::SpiralsEffect(int id) : RenderableEffect(id, "Spirals", spirals_16, spirals_24, spirals_32, spirals_48, spirals_64)
{
    //ctor
}

SpiralsEffect::~SpiralsEffect()
{
    //dtor
}
wxPanel *SpiralsEffect::CreatePanel(wxWindow *parent) {
    return new SpiralsPanel(parent);
}

void SpiralsEffect::SetDefaultParameters()
{
    SpiralsPanel *sp = (SpiralsPanel*)panel;
    if (sp == nullptr) {
        return;
    }

    sp->BitmapButton_VCSpiralsCount->SetActive(false);
    sp->BitmapButton_VCSpirals_Movement->SetActive(false);
    sp->BitmapButton_VCSpirals_Rotation->SetActive(false);
    sp->BitmapButton_VCSpirals_Thickness->SetActive(false);

    SetSliderValue(sp->Slider_Spirals_Count, 1);
    SetSliderValue(sp->Slider_Spirals_Rotation, 20);
    SetSliderValue(sp->Slider_Spirals_Thickness, 50);
    SetSliderValue(sp->Slider_Spirals_Movement, 10);

    SetCheckBoxValue(sp->CheckBox_Spirals_3D, false);
    SetCheckBoxValue(sp->CheckBox_Spirals_Blend, false);
    SetCheckBoxValue(sp->CheckBox_Spirals_Grow, false);
    SetCheckBoxValue(sp->CheckBox_Spirlas_Shrink, false);
}

bool SpiralsEffect::SupportsLinearColorCurves(const SettingsMap &SettingsMap) const
{
    // The blend setting is incompatible with linear colour curves
    return !SettingsMap.GetBool("E_CHECKBOX_Spirals_Blend");
}

void SpiralsEffect::Render(Effect *effect, SettingsMap &SettingsMap, RenderBuffer &buffer) {
    float offset = buffer.GetEffectTimeIntervalPosition();
    int PaletteRepeat = GetValueCurveInt("Spirals_Count", 1, SettingsMap, offset, SPIRALS_COUNT_MIN, SPIRALS_COUNT_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    float Movement = GetValueCurveDouble("Spirals_Movement", 1.0, SettingsMap, offset, SPIRALS_MOVEMENT_MIN, SPIRALS_MOVEMENT_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), SPIRALS_MOVEMENT_DIVISOR);
    float Rotation = GetValueCurveDouble("Spirals_Rotation", 0.0, SettingsMap, offset, SPIRALS_ROTATION_MIN, SPIRALS_ROTATION_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), SPIRALS_ROTATION_DIVISOR);
    // This is because spirals uses the slider while most others use the TextCtrl
    if (SettingsMap.Contains("VALUECURVE_Spirals_Rotation") && wxString(SettingsMap["VALUECURVE_Spirals_Rotation"]).Contains("Active=TRUE"))
    {
        Rotation *= 10;
    }
    int Thickness = GetValueCurveInt("Spirals_Thickness", 0, SettingsMap, offset, SPIRALS_THICKNESS_MIN, SPIRALS_THICKNESS_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    bool Blend = SettingsMap.GetBool("CHECKBOX_Spirals_Blend");
    bool Show3D = SettingsMap.GetBool("CHECKBOX_Spirals_3D");
    bool grow = SettingsMap.GetBool("CHECKBOX_Spirals_Grow");
    bool shrink = SettingsMap.GetBool("CHECKBOX_Spirals_Shrink");

    if (PaletteRepeat == 0) {
        PaletteRepeat = 1;
    }
    size_t colorcnt = buffer.GetColorCount();
    int SpiralCount = colorcnt * PaletteRepeat;
    double deltaStrands = buffer.BufferWi / SpiralCount;
    double SpiralThickness = (deltaStrands * Thickness / 100) + 1;
    double spiralGap = deltaStrands - SpiralThickness;

    int Direction = Movement > 0.001 ? 1 : (Movement < -0.001 ? -1 : 0);
    double position = buffer.GetEffectTimeIntervalPosition(std::abs(Movement));
    long ThicknessState = 0;
    if (grow && shrink)
    {
        ThicknessState = position <= 0.5 ? spiralGap * (position * 2) : spiralGap * ((1 - position) * 2);
    }
    else if (grow)
    {
        ThicknessState = spiralGap * position;
    }
    else if (shrink)
    {
        ThicknessState = spiralGap * (1.0 - position);
    }
    long SpiralState = position * buffer.BufferWi * 10 * Direction;

    SpiralThickness += ThicknessState;

    for (int ns = 0; ns < SpiralCount; ns++)
    {
        int strand_base = ns * deltaStrands;
        int ColorIdx = ns % colorcnt;
        xlColor color;
        buffer.palette.GetColor(ColorIdx, color);
        for (int thick = 0; thick < SpiralThickness; thick++)
        {
            int strand = (strand_base + thick) % buffer.BufferWi;
            for (int y = 0; y < buffer.BufferHt; y++)
            {
                int x = (int)((strand + SpiralState / 10.0 + y * Rotation / buffer.BufferHt)) % buffer.BufferWi;
                if (x < 0) x += buffer.BufferWi;

                if (buffer.palette.IsSpatial(ColorIdx))
                {
                    buffer.palette.GetSpatialColor(ColorIdx, (float)thick / (float)SpiralThickness, (float)y / (float)buffer.BufferHt, color);
                }

                if (Blend)
                {
                    buffer.GetMultiColorBlend(double(buffer.BufferHt - y - 1) / double(buffer.BufferHt), false, color);
                }
                if (Show3D)
                {
                    double f = 1.0;

                    if (Rotation < 0)
                    {
                        f = double(thick + 1) / SpiralThickness;
                    }
                    else
                    {
                        f = double(SpiralThickness - thick) / SpiralThickness;
                    }
                    if (buffer.allowAlpha) {
                        xlColor c(color);
                        c.alpha = 255.0 * f;
                        buffer.SetPixel(x, y, c);
                    }
                    else {
                        HSVValue hsv;
                        buffer.Color2HSV(color, hsv);
                        hsv.value *= f;
                        buffer.SetPixel(x, y, hsv);
                    }
                }
                else
                {
                    buffer.SetPixel(x, y, color);
                }
            }
        }
    }
}
