#include "ShockwaveEffect.h"
#include "ShockwavePanel.h"
#include <cmath>

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

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

wxPanel *ShockwaveEffect::CreatePanel(wxWindow *parent) {
    return new ShockwavePanel(parent);
}

int ShockwaveEffect::DrawEffectBackground(const Effect *e, int x1, int y1, int x2, int y2,
                                          DrawGLUtils::xlVertexColorAccumulator &backgrounds) {
    backgrounds.AddHBlendedRectangle(e->GetPalette(), x1, y1, x2, y2);
    return 2;
}

void ShockwaveEffect::SetDefaultParameters(Model *cls) {
    ShockwavePanel *sp = (ShockwavePanel*)panel;
    if (sp == nullptr) {
        return;
    }

    SetSliderValue(sp->Slider_Shockwave_Accel, 0);
    SetSliderValue(sp->Slider_Shockwave_CenterX, 50);
    SetSliderValue(sp->Slider_Shockwave_CenterY, 50);
    SetSliderValue(sp->Slider_Shockwave_End_Radius, 10);
    SetSliderValue(sp->Slider_Shockwave_End_Width, 10);
    SetSliderValue(sp->Slider_Shockwave_Start_Radius, 1);
    SetSliderValue(sp->Slider_Shockwave_Start_Width, 5);

    SetCheckBoxValue(sp->CheckBox_Shockwave_Blend_Edges, true);
}

void ShockwaveEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    double eff_pos = buffer.GetEffectTimeIntervalPosition();
    int center_x = GetValueCurveInt("Shockwave_CenterX", 50, SettingsMap, eff_pos);
    int center_y = GetValueCurveInt("Shockwave_CenterY", 50, SettingsMap, eff_pos);
    int start_radius = GetValueCurveInt("Shockwave_Start_Radius", 0, SettingsMap, eff_pos);
    int end_radius = GetValueCurveInt("Shockwave_End_Radius", 0, SettingsMap, eff_pos);
    int start_width = GetValueCurveInt("Shockwave_Start_Width", 0, SettingsMap, eff_pos);
    int end_width = GetValueCurveInt("Shockwave_End_Width", 0, SettingsMap, eff_pos);
    int acceleration = SettingsMap.GetInt("SLIDER_Shockwave_Accel", 0);
    bool blend_edges = SettingsMap.GetBool("CHECKBOX_Shockwave_Blend_Edges");

    int num_colors = buffer.palette.Size();
    if( num_colors == 0 )
        num_colors = 1;
    double eff_pos_adj = buffer.calcAccel(eff_pos, acceleration);

    HSVValue hsv, hsv1;
    xlColor color;
    double blend_pct = 1.0 / (num_colors-1);
    double color_pct1 = eff_pos_adj / blend_pct;
    int color_index = (int)color_pct1;
    blend_pct = color_pct1 - (double)color_index;
    buffer.Get2ColorBlend(color_index, std::min(color_index+1,num_colors-1), std::min( blend_pct, 1.0), color);

    int xc_adj = (center_x-50)*buffer.BufferWi / 100;
    int yc_adj = (center_y-50)*buffer.BufferHt / 100;

    double radius1 = start_radius;
    double radius2 = end_radius;
    double radius_center = radius1 + (radius2 - radius1) * eff_pos_adj;
    double half_width = (start_width + (end_width - start_width) * eff_pos_adj) / 2.0;
    radius1 = radius_center - half_width;
    radius2 = radius_center + half_width;
    radius1 = std::max(0.0, radius1);

    int max_radius = std::max(start_radius, end_radius);

    for (int x = 0; x < buffer.BufferWi; x++)
    {
        int x1 = x - xc_adj - (buffer.BufferWi / 2);
        for (int y = 0; y < buffer.BufferHt; y++)
        {
            int y1 = y - yc_adj - (buffer.BufferHt / 2);
            double r = std::hypot(x1, y1);
            if( r >= radius1 && r <= radius2 ) {
                double color_pct = 1.0 - std::abs(r-radius_center)/half_width;
                if (buffer.palette.IsSpatial(color_index))
                {
                    buffer.palette.GetSpatialColor(color_index, xc_adj + buffer.BufferWi / 2, yc_adj + buffer.BufferHt / 2, x, y, color_pct, max_radius, color);
                    hsv = color.asHSV();
                }
                else
                {
                    hsv = color;
                }
                if( blend_edges )
                {
                    if (buffer.allowAlpha) {
                        color.alpha = 255.0 * color_pct;
                    }
                    else {
                        hsv.value = hsv.value * color_pct;
                        color = hsv;
                    }
                }
                buffer.SetPixel(x, y, color);
            }
        }
    }
}
