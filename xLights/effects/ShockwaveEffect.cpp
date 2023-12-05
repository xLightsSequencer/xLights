/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "ShockwaveEffect.h"
#include "ShockwavePanel.h"
#include <cmath>

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../UtilFunctions.h"

#include "../../include/shockwave-16.xpm"
#include "../../include/shockwave-24.xpm"
#include "../../include/shockwave-32.xpm"
#include "../../include/shockwave-48.xpm"
#include "../../include/shockwave-64.xpm"

ShockwaveEffect::ShockwaveEffect(int id) : RenderableEffect(id, "Shockwave", shockwave_16, shockwave_24, shockwave_32, shockwave_48, shockwave_64)
{
    //ctor
}

ShockwaveEffect::~ShockwaveEffect()
{
    //dtor
}

xlEffectPanel *ShockwaveEffect::CreatePanel(wxWindow *parent) {
    return new ShockwavePanel(parent);
}

int ShockwaveEffect::DrawEffectBackground(const Effect *e, int x1, int y1, int x2, int y2,
                                          xlVertexColorAccumulator &backgrounds, xlColor* colorMask, bool ramps) {
    int cycles = e->GetSettings().GetInt("E_SLIDER_Shockwave_Cycles", 1);
    int totalsize = x2 - x1;
    double x_size = totalsize / (double)cycles;
    x_size = std::max(x_size, 0.01);
    for (int i = 0; i < cycles; ++i) {
        backgrounds.AddHBlendedRectangleAsTriangles(x1 + (i * x_size), y1, x1 + (i * x_size) +  x_size, y2, colorMask, 0, e->GetPalette());
    }
    
    return 2;
}

void ShockwaveEffect::SetDefaultParameters() {
    ShockwavePanel *sp = (ShockwavePanel*)panel;
    if (sp == nullptr) {
        return;
    }

    sp->BitmapButton_Shockwave_CenterX->SetActive(false);
    sp->BitmapButton_Shockwave_CenterY->SetActive(false);
    sp->BitmapButton_Shockwave_End_Radius->SetActive(false);
    sp->BitmapButton_Shockwave_Start_Radius->SetActive(false);
    sp->BitmapButton_Shockwave_Start_Width->SetActive(false);
    sp->BitmapButton_Shockwave_End_Width->SetActive(false);

    SetSliderValue(sp->Slider_Shockwave_Accel, 0);
    SetSliderValue(sp->Slider_Shockwave_CenterX, 50);
    SetSliderValue(sp->Slider_Shockwave_CenterY, 50);
    SetSliderValue(sp->Slider_Shockwave_End_Radius, 10);
    SetSliderValue(sp->Slider_Shockwave_End_Width, 10);
    SetSliderValue(sp->Slider_Shockwave_Start_Radius, 1);
    SetSliderValue(sp->Slider_Shockwave_Start_Width, 5);
    SetSliderValue(sp->Slider_Shockwave_Cycles, 1);

    SetCheckBoxValue(sp->CheckBox_Shockwave_Blend_Edges, true);
    SetCheckBoxValue(sp->CheckBox_Shockwave_Scale, false);
}

#define ToRadians(x) ((double)x * PI / (double)180.0)

void ShockwaveEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    int cycles = SettingsMap.GetInt("SLIDER_Shockwave_Cycles", 1);
    double eff_pos = buffer.GetEffectTimeIntervalPosition(cycles);
    int center_x = GetValueCurveInt("Shockwave_CenterX", 50, SettingsMap, eff_pos, SHOCKWAVE_X_MIN, SHOCKWAVE_X_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int center_y = GetValueCurveInt("Shockwave_CenterY", 50, SettingsMap, eff_pos, SHOCKWAVE_Y_MIN, SHOCKWAVE_Y_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int start_radius = GetValueCurveInt("Shockwave_Start_Radius", 0, SettingsMap, eff_pos, SHOCKWAVE_STARTRADIUS_MIN, SHOCKWAVE_STARTRADIUS_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int end_radius = GetValueCurveInt("Shockwave_End_Radius", 0, SettingsMap, eff_pos, SHOCKWAVE_ENDRADIUS_MIN, SHOCKWAVE_ENDRADIUS_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int start_width = GetValueCurveInt("Shockwave_Start_Width", 0, SettingsMap, eff_pos, SHOCKWAVE_STARTWIDTH_MIN, SHOCKWAVE_STARTWIDTH_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int end_width = GetValueCurveInt("Shockwave_End_Width", 0, SettingsMap, eff_pos, SHOCKWAVE_ENDWIDTH_MIN, SHOCKWAVE_ENDWIDTH_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int acceleration = SettingsMap.GetInt("SLIDER_Shockwave_Accel", 0);
    bool blend_edges = SettingsMap.GetBool("CHECKBOX_Shockwave_Blend_Edges");
    bool scale = SettingsMap.GetBool("CHECKBOX_Shockwave_Scale", false);

    int num_colors = buffer.palette.Size();
    if( num_colors == 0 )
        num_colors = 1;

    double eff_pos_adj = buffer.calcAccel(eff_pos, acceleration);

    HSVValue hsv, hsv1;
    xlColor color;
    double blend_pct = 1.0;
    if (num_colors > 1) {
        blend_pct = 1.0 / (num_colors-1);
    }
    double color_pct1 = eff_pos_adj / blend_pct;
    int color_index = (int)color_pct1;
    blend_pct = color_pct1 - (double)color_index;
    buffer.Get2ColorBlend(std::min(color_index, num_colors - 1), std::min(color_index+1,num_colors-1), std::min( blend_pct, 1.0), color);

    //if (color == *wxWHITE) wxASSERT(false);

    int xc_adj = center_x * buffer.BufferWi / 100;
    int yc_adj = center_y * buffer.BufferHt / 100;

    double radius1 = start_radius;
    double radius2 = end_radius;
    if (scale) {//convert to percentage of buffer, i.e 100 is 100% of buffer size
        double bufferMax = std::max(buffer.BufferHt, buffer.BufferWi);
        radius1 = radius1 * (bufferMax / 200.0); //200 bc radius is half of the width
        radius2 = radius2 * (bufferMax / 200.0);
        start_width = start_width * (bufferMax / 100.0);
        end_width = end_width * (bufferMax / 100.0);
    }
    double radius_center = radius1 + (radius2 - radius1) * eff_pos_adj;
    double half_width = (start_width + (end_width - start_width) * eff_pos_adj) / 2.0;
    if (half_width < 0.25) {
        half_width = 0.25;
    }
    radius1 = radius_center - half_width;
    radius2 = radius_center + half_width;
    radius1 = std::max(0.0, radius1);

    for (int x = 0; x < buffer.BufferWi; x++)
    {
        int x1 = x - xc_adj;
        for (int y = 0; y < buffer.BufferHt; y++)
        {
            int y1 = y - yc_adj;
            double r = std::hypot(x1, y1);
            if( r >= radius1 && r <= radius2 ) {
                if (buffer.palette.IsSpatial(color_index))
                {
                    double theta = (((std::atan2(x1, y1) * 180.0 / PI)) + 180.0) / 360.0;
                    buffer.palette.GetSpatialColor(color_index, radius1, 0, r, 0, theta, radius2, color);
                    hsv = color.asHSV();
                }
                else
                {
                    hsv = color;
                }
                if( blend_edges )
                {
                    double color_pct = 1.0 - std::abs(r-radius_center)/half_width;
                    xlColor ncolor(color);
                    if (buffer.allowAlpha) {
                        ncolor.alpha = 255.0 * color_pct;
                    }
                    else {
                        hsv.value = hsv.value * color_pct;
                        ncolor = hsv;
                    }
                    buffer.SetPixel(x, y, ncolor);
                } else {
                    buffer.SetPixel(x, y, color);
                }
            }
        }
    }
}
