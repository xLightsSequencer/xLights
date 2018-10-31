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
                                    DrawGLUtils::xlAccumulator &backgrounds, xlColor* colorMask, bool ramps) {
    int head_duration = e->GetSettings().GetInt("E_SLIDER_Fan_Duration", 50);
    int num_colors = e->GetPalette().size();
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
            xlColor c1 = e->GetPalette()[i];
            c1.ApplyMask(colorMask);
            backgrounds.AddHBlendedRectangle(c1, c1, cx, y1+1, x_mid, y2-1);
            backgrounds.AddHBlendedRectangle(c1, c1, cx1, y1+4, x2, y2-4);
        } else {
            xlColor c1 = e->GetPalette()[i];
            c1.ApplyMask(colorMask);
            xlColor c2 = e->GetPalette()[i + 1];
            c2.ApplyMask(colorMask);
            backgrounds.AddHBlendedRectangle(c1, c2, cx, y1+1, cx+head_length, y2-1);
            backgrounds.AddHBlendedRectangle(c1, c2, cx1, y1+4, cx1+color_length, y2-4);
        }
    }
    return 0;
}

void FanEffect::SetDefaultParameters() {
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
    SetSliderValue(fp->Slider_Fan_Element_Width, 100);
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

void FanEffect::Render(Effect *effect, SettingsMap &SettingsMap, RenderBuffer &buffer) {
    double eff_pos = buffer.GetEffectTimeIntervalPosition();
    int center_x = GetValueCurveInt("Fan_CenterX", 50, SettingsMap, eff_pos, FAN_CENTREX_MIN , FAN_CENTREX_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int center_y = GetValueCurveInt("Fan_CenterY", 50, SettingsMap, eff_pos, FAN_CENTREY_MIN, FAN_CENTREY_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int start_radius = GetValueCurveInt("Fan_Start_Radius", 1, SettingsMap, eff_pos, FAN_STARTRADIUS_MIN, FAN_STARTRADIUS_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int end_radius = GetValueCurveInt("Fan_End_Radius", 10, SettingsMap, eff_pos, FAN_ENDRADIUS_MIN, FAN_ENDRADIUS_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int start_angle = GetValueCurveInt("Fan_Start_Angle", 0, SettingsMap, eff_pos, FAN_STARTANGLE_MIN, FAN_STARTANGLE_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int revolutions = GetValueCurveInt("Fan_Revolutions", 720, SettingsMap, eff_pos, FAN_REVOLUTIONS_MIN, FAN_REVOLUTIONS_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), 360);
    int num_blades = GetValueCurveInt("Fan_Num_Blades", 3, SettingsMap, eff_pos, FAN_BLADES_MIN, FAN_BLADES_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int blade_width = GetValueCurveInt("Fan_Blade_Width", 50, SettingsMap, eff_pos, FAN_BLADEWIDTH_MIN, FAN_BLADEWIDTH_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int blade_angle = GetValueCurveInt("Fan_Blade_Angle", 90, SettingsMap, eff_pos, FAN_BLADEANGLE_MIN, FAN_BLADEANGLE_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int num_elements = GetValueCurveInt("Fan_Num_Elements", 1, SettingsMap, eff_pos, FAN_NUMELEMENTS_MIN, FAN_NUMELEMENTS_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int element_width = GetValueCurveInt("Fan_Element_Width", 100, SettingsMap, eff_pos, FAN_ELEMENTWIDTH_MIN, FAN_ELEMENTWIDTH_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int duration = GetValueCurveInt("Fan_Duration", 80, SettingsMap, eff_pos, FAN_DURATION_MIN, FAN_DURATION_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int acceleration = GetValueCurveInt("Fan_Accel", 0, SettingsMap, eff_pos, FAN_ACCEL_MIN, FAN_ACCEL_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    bool reverse_dir = SettingsMap.GetBool("CHECKBOX_Fan_Reverse");
    bool blend_edges = SettingsMap.GetBool("CHECKBOX_Fan_Blend_Edges");

    HSVValue hsv, hsv1;
    int num_colors = buffer.palette.Size();
    if( num_colors == 0 )
        num_colors = 1;
    xlColor color;
    double eff_pos_adj = buffer.calcAccel(eff_pos, acceleration);
    double revs = (double)revolutions;

    double effect_duration = duration/100.0;    // time the head is in the frame
    double radius_rampup = (1.0 - effect_duration)/2.0;

    double radius1 = start_radius;
    double radius2 = end_radius;

    int xc_adj = (center_x-50)*buffer.BufferWi / 100;
    int yc_adj = (center_y-50)*buffer.BufferHt / 100;

    double blade_div_angle = 360.0 / (double)num_blades;
    double blade_width_angle = blade_div_angle * (double)blade_width / 100.0;
    double color_angle = blade_width_angle / (double)num_colors;
    double angle_offset = eff_pos_adj * revs;
    double element_angle = color_angle / (double)num_elements;
    double element_size = element_angle * (double)element_width/ 100.0;

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

    int max_radius = std::max(start_radius, end_radius);

    for (int x = 0; x < buffer.BufferWi; x++)
    {
        int x1 = x - xc_adj - (buffer.BufferWi / 2);
        for (int y = 0; y < buffer.BufferHt; y++)
        {
            int y1 = y - yc_adj - (buffer.BufferHt / 2);
            double r = std::hypot(x1, y1);
            if( r >= radius1 && r <= radius2 ) {
                double degrees_twist = (r / max_radius)*blade_angle;
                double theta = ((std::atan2(x1, y1) * 180.0 / PI)) + degrees_twist + start_angle;
                if (reverse_dir == 1)
                {
                    theta = angle_offset - theta + 180.0;
                } else {
                    theta = theta + 180.0 + angle_offset;
                }
                if( theta < 0.0 ) { theta += 360.0; }
                double current_blade = theta / blade_div_angle;
                double current_blade_angle = theta - (double)((int)current_blade * blade_div_angle);

                if( current_blade_angle <= blade_width_angle ) {

                    double current_element = current_blade_angle / element_angle;
                    double current_element_angle = current_blade_angle - (double)((int)current_element * element_angle);

                    if( current_element_angle <= element_size ) {
                        int color_index = (int)(current_blade_angle / color_angle);
                        buffer.palette.GetColor(color_index, color);

                        double round = (float)current_element_angle / (float)element_size;
                        double color_pct = 1.0 - ((std::abs(current_element_angle - (element_size/2)) * 2) / element_size);

                        if (buffer.palette.IsSpatial(color_index))
                        {
                            buffer.palette.GetSpatialColor(color_index, xc_adj + (buffer.BufferWi / 2), yc_adj + (buffer.BufferHt / 2), x, y, round, max_radius, color);
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
    }
}

