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

const double PI  =3.141592653589793238463;
#define ToRadians(x) ((double)x * PI / (double)180.0)

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

    std::vector< std::vector<double> > temp_colors_pct(buffer.BufferWi, std::vector<double>(buffer.BufferHt));
    int num_colors = buffer.palette.Size();
    if( num_colors == 0 )
        num_colors = 1;
    xlColor color, c_old, c_new;
    double eff_pos_adj = buffer.calcAccel(eff_pos, acceleration);

    double blend_pct = 1.0 / (num_colors-1);
    double color_pct1 = eff_pos_adj / blend_pct;
    int color_index = (int)color_pct1;
    blend_pct = color_pct1 - (double)color_index;
    buffer.Get2ColorBlend(color_index, std::min(color_index+1,num_colors-1), std::min( blend_pct, 1.0), color);

    double pos_x = buffer.BufferWi * center_x/100.0;
    double pos_y = buffer.BufferHt * center_y/100.0;

    double radius1 = start_radius;
    double radius2 = end_radius;
    double radius_center = radius1 + (radius2 - radius1) * eff_pos_adj;
    double half_width = (start_width + (end_width - start_width) * eff_pos_adj) / 2.0;
    radius1 = radius_center - half_width;
    radius2 = radius_center + half_width;

    double step = buffer.GetStepAngle(radius1, radius2);

    for( int x = 0; x < buffer.BufferWi; x++ )
    {
        for( int y = 0; y < buffer.BufferHt; y++ )
        {
            temp_colors_pct[x][y] = 0.0;
        }
    }
    buffer.ClearTempBuf();

    for( double current_angle = 0.0; current_angle <= 360.0; current_angle += step )
    {
        for( double r = std::max(0.0, radius1); r <= radius2; r += 0.5 )
        {
            double x1 = buffer.sin(ToRadians(current_angle)) * r + (double)pos_x;
            double y1 = buffer.cos(ToRadians(current_angle)) * r + (double)pos_y;

            if( blend_edges )
            {
                double color_pct = 1.0 - std::abs(r-radius_center)/half_width;
                if( color_pct > 0.0 )
                {
                    if (x1 >= 0 && x1 < buffer.BufferWi && y1 >= 0 && y1 < buffer.BufferHt)
                    {
                        if (buffer.allowAlpha) {
                            color.alpha = 255.0 * color_pct;
                            buffer.SetPixel((int)x1,(int)y1,color);
                        } else {
                            temp_colors_pct[(int)x1][(int)y1] = color_pct;
                            buffer.SetTempPixel((int)x1,(int)y1,color);
                        }
                    }
                }
            }
            else
            {
                buffer.SetPixel((int)x1,(int)y1,color);
            }
        }
    }

    // blend element data into final buffer
    if( blend_edges && !buffer.allowAlpha )
    {
        for( int x = 0; x < buffer.BufferWi; x++ )
        {
            for( int y = 0; y < buffer.BufferHt; y++ )
            {
                if( temp_colors_pct[x][y] > 0.0 )
                {
                    buffer.GetTempPixel(x,y,c_new);
                    buffer.GetPixel(x,y,c_old);
                    buffer.Get2ColorAlphaBlend(c_old, c_new, temp_colors_pct[x][y], color);
                    buffer.SetPixel(x,y,color);
                    temp_colors_pct[x][y] = 0.0;
                }
            }
        }
    }
}
