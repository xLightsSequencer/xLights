#include "FanEffect.h"
#include "FanPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"


#include "../../include/fan-16.xpm"
#include "../../include/fan-24.xpm"
#include "../../include/fan-32.xpm"
#include "../../include/fan-48.xpm"
#include "../../include/fan-64.xpm"

FanEffect::FanEffect(int id) : RenderableEffect(id, "Fan", fan_16, fan_24, fan_32, fan_48, fan_64)
{
    //ctor
}

FanEffect::~FanEffect()
{
    //dtor
}

wxPanel *FanEffect::CreatePanel(wxWindow *parent) {
    return new FanPanel(parent);
}


int FanEffect::DrawEffectBackground(const Effect *e, int x1, int y1, int x2, int y2,
                                    DrawGLUtils::xlVertexColorAccumulator &backgrounds) {
    int head_duration = e->GetSettings().GetInt("E_SLIDER_Fan_Duration", 50);
    int num_colors = e->GetPalette().size();
    xlColor head_color = e->GetPalette()[0];
    int x_mid = (int)((float)(x2-x1) * (float)head_duration / 100.0) + x1;
    int head_length;
    int color_length;
    if( num_colors > 1 ) {
        head_length = (x_mid - x1) / (num_colors-1);
        color_length = (x2 - x_mid) / (num_colors-1);
    } else {
        head_length = (x_mid - x1);
        color_length = (x2 - x_mid);
    }
    for(int i = 0; i < num_colors; i++ ) {
        int cx = x1 + (i*head_length);
        int cx1 = x_mid + (i*color_length);
        if( i == (num_colors-1) ) // fix any roundoff error for last color
        {
            backgrounds.AddHBlendedRectangle(e->GetPalette()[i], e->GetPalette()[i], cx, y1+1, x_mid, y2-1);
            backgrounds.AddHBlendedRectangle(e->GetPalette()[i], e->GetPalette()[i], cx1, y1+4, x2, y2-4);
        } else {
            backgrounds.AddHBlendedRectangle(e->GetPalette()[i], e->GetPalette()[i+1], cx, y1+1, cx+head_length, y2-1);
            backgrounds.AddHBlendedRectangle(e->GetPalette()[i], e->GetPalette()[i+1], cx1, y1+4, cx1+color_length, y2-4);
        }
    }
    return 0;
}

void FanEffect::SetDefaultParameters(Model *cls) {
    FanPanel *fp = (FanPanel*)panel;
    if (fp == nullptr) {
        return;
    }

    fp->ValueCurve_Fan_Blade_Angle->SetActive(false);
    fp->ValueCurve_Fan_Accel->SetActive(false);
    fp->ValueCurve_Fan_Blade_Width->SetActive(false);
    fp->ValueCurve_Fan_CenterX->SetActive(false);
    fp->ValueCurve_Fan_CenterY->SetActive(false);
    fp->ValueCurve_Fan_Duration->SetActive(false);
    fp->ValueCurve_Fan_Element_Width->SetActive(false);
    fp->ValueCurve_Fan_End_Radius->SetActive(false);
    fp->ValueCurve_Fan_Num_Blades->SetActive(false);
    fp->ValueCurve_Fan_Num_Elements->SetActive(false);
    fp->ValueCurve_Fan_Revolutions->SetActive(false);
    fp->ValueCurve_Fan_Start_Angle->SetActive(false);
    fp->ValueCurve_Fan_Start_Radius->SetActive(false);

    SetSliderValue(fp->Slider_Fan_Accel, 0);
    SetSliderValue(fp->Slider_Fan_Blade_Angle, 90);
    SetSliderValue(fp->Slider_Fan_Blade_Width, 100);
    SetSliderValue(fp->Slider_Fan_CenterX, 50);
    SetSliderValue(fp->Slider_Fan_CenterY, 50);
    SetSliderValue(fp->Slider_Fan_Duration, 80);
    SetSliderValue(fp->Slider_Fan_Element_Width, 50);
    SetSliderValue(fp->Slider_Fan_End_Radius, 10);
    SetSliderValue(fp->Slider_Fan_Num_Blades, 3);
    SetSliderValue(fp->Slider_Fan_Num_Elements, 1);
    SetSliderValue(fp->Slider_Fan_Revolutions, 720);
    SetSliderValue(fp->Slider_Fan_Start_Angle, 0);
    SetSliderValue(fp->Slider_Fan_Start_Radius, 1);

    SetCheckBoxValue(fp->CheckBox_Fan_Blend_Edges, true);
    SetCheckBoxValue(fp->CheckBox_Fan_Reverse, false);
}

const double PI  =3.141592653589793238463;
#define ToRadians(x) ((double)x * PI / (double)180.0)
void FanEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    double eff_pos = buffer.GetEffectTimeIntervalPosition();
    int center_x = GetValueCurveInt("Fan_CenterX", 50, SettingsMap, eff_pos);
    int center_y = GetValueCurveInt("Fan_CenterY", 50, SettingsMap, eff_pos);
    int start_radius = GetValueCurveInt("Fan_Start_Radius", 1, SettingsMap, eff_pos);
    int end_radius = GetValueCurveInt("Fan_End_Radius", 10, SettingsMap, eff_pos);
    int start_angle = GetValueCurveInt("Fan_Start_Angle", 0, SettingsMap, eff_pos);
    int revolutions = GetValueCurveInt("Fan_Revolutions", 720, SettingsMap, eff_pos);
    int num_blades = GetValueCurveInt("Fan_Num_Blades", 3, SettingsMap, eff_pos);
    int blade_width = GetValueCurveInt("Fan_Blade_Width", 50, SettingsMap, eff_pos);
    int blade_angle = GetValueCurveInt("Fan_Blade_Angle", 90, SettingsMap, eff_pos);
    int num_elements = GetValueCurveInt("Fan_Num_Elements", 1, SettingsMap, eff_pos);
    int element_width = GetValueCurveInt("Fan_Element_Width", 100, SettingsMap, eff_pos);
    int duration = GetValueCurveInt("Fan_Duration", 80, SettingsMap, eff_pos);
    int acceleration = GetValueCurveInt("Fan_Accel", 0, SettingsMap, eff_pos);
    bool reverse_dir = SettingsMap.GetBool("CHECKBOX_Fan_Reverse");
    bool blend_edges = SettingsMap.GetBool("CHECKBOX_Fan_Blend_Edges");

    std::vector< std::vector<double> > temp_colors_pct(buffer.BufferWi, std::vector<double>(buffer.BufferHt));
    int num_colors = buffer.palette.Size();
    if( num_colors == 0 )
        num_colors = 1;
    xlColor color, c_old, c_new;
    double eff_pos_adj = buffer.calcAccel(eff_pos, acceleration);
    double revs = (double)revolutions;

    double pos_x = buffer.BufferWi * center_x/100.0;
    double pos_y = buffer.BufferHt * center_y/100.0;

    double effect_duration = duration/100.0;    // time the head is in the frame
    double radius_rampup = (1.0 - effect_duration)/2.0;

    double radius1 = start_radius;
    double radius2 = end_radius;

    double blade_div_angle = 360.0 / (double)num_blades;
    double blade_width_angle = blade_div_angle * (double)blade_width / 100.0;
    double color_angle = blade_width_angle / (double)num_colors;
    double angle_offset = eff_pos_adj * revs;
    double element_angle = color_angle / (double)num_elements;
    double element_size = element_angle * (double)element_width/ 100.0;

    for( int x = 0; x < buffer.BufferWi; x++ )
    {
        for( int y = 0; y < buffer.BufferHt; y++ )
        {
            temp_colors_pct[x][y] = 0.0;
        }
    }
    buffer.ClearTempBuf();

    if( reverse_dir ^ (blade_angle < 0) )
    {
        angle_offset *= -1.0;
    }

    if( effect_duration < 1.0 )
    {
        double radius_delta = std::abs(radius2 - radius1);
        if( eff_pos_adj < radius_rampup )  // blade growing
        {
            double pct = 1.0 - (eff_pos_adj / radius_rampup);
            if( radius2 > radius1 )
                radius2 = radius2 - radius_delta * pct;
            else
                radius2 = radius2 + radius_delta * pct;
        }
        else if( eff_pos_adj > (1.0 - radius_rampup) )  // blade shrinking
        {
            double pct = (1.0 - eff_pos_adj) / radius_rampup;
            if( radius2 > radius1 )
                radius1 = radius2 - radius_delta * pct;
            else
                radius1 = radius2 + radius_delta * pct;
        }
    }

    if( radius1 > radius2 )
    {
        std::swap(radius1, radius2);
    }

    double step = buffer.GetStepAngle(radius1, radius2);

    double a_const = radius2 / ToRadians(std::abs((double)blade_angle));

    for( int blade = 0; blade < num_blades; blade++ )
    {
        for( int section = 0; section < num_colors; section++ )
        {
            for( int element = 0.0; element < num_elements; element++ )
            {
                double blade_element_start_angle = (double)start_angle + blade_div_angle*(double)blade + color_angle*(double)section + angle_offset + element_angle*(double)element;
                for( double current_angle = 0.0; current_angle <= element_size; current_angle += step )
                {
                    double mid_angle = element_size / 2.0;
                    double color_pct = 1.0 - std::abs(current_angle - mid_angle)/mid_angle;
                    for( double i = radius1; i <= radius2; i += 0.5 )
                    {
                        double calc_angle = i / a_const * 180.0 / PI;
                        double adj_angle = calc_angle + blade_element_start_angle + current_angle;
                        if( blade_angle < 0.0 )
                        {
                            adj_angle *= -1.0;
                        }
                        buffer.palette.GetColor(section, color);

                        double x1 = buffer.sin(ToRadians(adj_angle)) * i + (double)pos_x;
                        double y1 = buffer.cos(ToRadians(adj_angle)) * i + (double)pos_y;

                        if( blend_edges )
                        {
                            if( color_pct > 0.0 )
                            {
                                if (x1 >= 0 && x1 < buffer.BufferWi && y1 >= 0 && y1 < buffer.BufferHt)
                                {
                                    temp_colors_pct[(int)x1][(int)y1] = color_pct;
                                    buffer.SetTempPixel((int)x1,(int)y1,color);
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
                if( blend_edges )
                {
                    for( int x = 0; x < buffer.BufferWi; x++ )
                    {
                        for( int y = 0; y < buffer.BufferHt; y++ )
                        {
                            if( temp_colors_pct[x][y] > 0.0 )
                            {
                                if( buffer.allowAlpha )
                                {
                                    buffer.GetTempPixel(x,y,color);
                                    color.alpha = 255 * temp_colors_pct[x][y];
                                    buffer.SetPixel(x,y,color);
                                    temp_colors_pct[x][y] = 0.0;
                                }
                                else
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
            }
        }
    }
}
