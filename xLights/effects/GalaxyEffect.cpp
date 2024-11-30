/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "GalaxyEffect.h"
#include "GalaxyPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../UtilFunctions.h"
#include "../Parallel.h"

#include "../../include/galaxy-16.xpm"
#include "../../include/galaxy-24.xpm"
#include "../../include/galaxy-32.xpm"
#include "../../include/galaxy-48.xpm"
#include "../../include/galaxy-64.xpm"

GalaxyEffect::GalaxyEffect(int id) : RenderableEffect(id, "Galaxy", galaxy_16, galaxy_24, galaxy_32, galaxy_48, galaxy_64)
{
    //ctor
}

GalaxyEffect::~GalaxyEffect()
{
    //dtor
}

xlEffectPanel *GalaxyEffect::CreatePanel(wxWindow *parent) {
    return new GalaxyPanel(parent);
}

int GalaxyEffect::DrawEffectBackground(const Effect *e, int x1, int y1, int x2, int y2,
                                       xlVertexColorAccumulator &backgrounds, xlColor* colorMask, bool ramps) {
    int head_duration = e->GetSettings().GetInt("E_SLIDER_Galaxy_Duration", 20);
    int num_colors = e->GetPaletteSize();
    xlColor head_color = e->GetPalette()[0];
    head_color.ApplyMask(colorMask);
    int x_mid = (int)((float)(x2-x1) * (float)head_duration / 100.0) + x1;
    if( x_mid > x1 )
    {
        backgrounds.AddHBlendedRectangleAsTriangles(x1, y1+1, x_mid, y2-1, head_color, head_color);
    }
    int color_length = (x2 - x_mid) / num_colors;
    for(int i = 0; i < num_colors; i++ )
    {
        int cx1 = x_mid + (i*color_length);
        if( i == (num_colors-1) ) // fix any roundoff error for last color
        {
            xlColor c1 = e->GetPalette()[i];
            c1.ApplyMask(colorMask);
            xlColor c2 = e->GetPalette()[i];
            c2.ApplyMask(colorMask);
            backgrounds.AddHBlendedRectangleAsTriangles(cx1, y1+4, x2, y2-4, c1, c2);
        }
        else
        {
            xlColor c1 = e->GetPalette()[i];
            c1.ApplyMask(colorMask);
            xlColor c2 = e->GetPalette()[i + 1];
            c2.ApplyMask(colorMask);
            backgrounds.AddHBlendedRectangleAsTriangles(cx1, y1+4, cx1+color_length, y2-4, c1, c2);
        }
    }
    return 2; // draw small icon
}

void GalaxyEffect::SetDefaultParameters() {
    GalaxyPanel *gp = (GalaxyPanel*)panel;
    if (gp == nullptr) {
        return;
    }

    gp->ValueCurve_Galaxy_Accel->SetActive(false);
    gp->ValueCurve_Galaxy_CenterX->SetActive(false);
    gp->ValueCurve_Galaxy_CenterY->SetActive(false);
    gp->ValueCurve_Galaxy_Duration->SetActive(false);
    gp->ValueCurve_Galaxy_End_Radius->SetActive(false);
    gp->ValueCurve_Galaxy_End_Width->SetActive(false);
    gp->ValueCurve_Galaxy_Revolutions->SetActive(false);
    gp->ValueCurve_Galaxy_Start_Angle->SetActive(false);
    gp->ValueCurve_Galaxy_Start_Radius->SetActive(false);
    gp->ValueCurve_Galaxy_Start_Width->SetActive(false);

    SetSliderValue(gp->Slider_Galaxy_Accel, 0);
    SetSliderValue(gp->Slider_Galaxy_CenterX, 50);
    SetSliderValue(gp->Slider_Galaxy_CenterY, 50);
    SetSliderValue(gp->Slider_Galaxy_Duration, 20);
    SetSliderValue(gp->Slider_Galaxy_End_Radius, 10);
    SetSliderValue(gp->Slider_Galaxy_End_Width, 5);
    SetSliderValue(gp->Slider_Galaxy_Revolutions, 1440);
    SetSliderValue(gp->Slider_Galaxy_Start_Angle, 0);
    SetSliderValue(gp->Slider_Galaxy_Start_Radius, 1);
    SetSliderValue(gp->Slider_Galaxy_Start_Width, 5);

    SetCheckBoxValue(gp->CheckBox_Galaxy_Blend_Edges, true);
    SetCheckBoxValue(gp->CheckBox_Galaxy_Inward, false);
    SetCheckBoxValue(gp->CheckBox_Galaxy_Reverse, false);
}

#define ToRadians(x) ((double)x * PI / (double)180.0)

void CalcEndpointColor(double end_angle, double start_angle,
                       double head_end_of_tail, double color_length, int num_colors,
                       RenderBuffer& buffer, xlColor& color)
{
    double cv = (head_end_of_tail - end_angle) / color_length;
    int ci = (int)cv;
    double cp = cv - (double)ci;
    int c2 = std::min(ci + 1, num_colors - 1);
    if (ci < c2) {
        buffer.Get2ColorBlend(ci, c2, std::min(cp, 1.0), color);
    } else {
        buffer.palette.GetColor(c2, color);
    }
}

double GetStep(double radius)
{
    if( radius < 5 ) {
        return 0.1;
    }
    return (0.5 * 360.0 / (2.0 * PI * radius));
}

void GalaxyEffect::Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer)
{
    double eff_pos = buffer.GetEffectTimeIntervalPosition();
    int center_x = GetValueCurveInt("Galaxy_CenterX", 50, SettingsMap, eff_pos, GALAXY_CENTREX_MIN, GALAXY_CENTREX_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int center_y = GetValueCurveInt("Galaxy_CenterY", 50, SettingsMap, eff_pos, GALAXY_CENTREY_MIN, GALAXY_CENTREY_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int start_radius = GetValueCurveInt("Galaxy_Start_Radius", 1, SettingsMap, eff_pos, GALAXY_STARTRADIUS_MIN, GALAXY_STARTRADIUS_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int end_radius = GetValueCurveInt("Galaxy_End_Radius", 10, SettingsMap, eff_pos, GALAXY_ENDRADIUS_MIN, GALAXY_ENDRADIUS_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int start_angle = GetValueCurveInt("Galaxy_Start_Angle", 0, SettingsMap, eff_pos, GALAXY_STARTANGLE_MIN, GALAXY_STARTANGLE_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int revolutions = GetValueCurveInt("Galaxy_Revolutions", 1440, SettingsMap, eff_pos, GALAXY_REVOLUTIONS_MIN, GALAXY_REVOLUTIONS_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), 360);
    int start_width = GetValueCurveInt("Galaxy_Start_Width", 5, SettingsMap, eff_pos, GALAXY_STARTWIDTH_MIN, GALAXY_STARTWIDTH_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int end_width = GetValueCurveInt("Galaxy_End_Width", 5, SettingsMap, eff_pos, GALAXY_ENDWIDTH_MIN, GALAXY_ENDWIDTH_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int duration = GetValueCurveInt("Galaxy_Duration", 20, SettingsMap, eff_pos, GALAXY_DURATION_MIN, GALAXY_DURATION_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int acceleration = GetValueCurveInt("Galaxy_Accel", 0, SettingsMap, eff_pos, GALAXY_ACCEL_MIN, GALAXY_ACCEL_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    bool reverse_dir = SettingsMap.GetBool("CHECKBOX_Galaxy_Reverse");
    bool blend_edges = SettingsMap.GetBool("CHECKBOX_Galaxy_Blend_Edges");
    bool inward = SettingsMap.GetBool("CHECKBOX_Galaxy_Inward");
    bool scale = SettingsMap.GetBool("CHECKBOX_Galaxy_Scale");

    if (revolutions == 0)
        return;
    std::vector<std::vector<double>> temp_colors_pct(buffer.BufferWi, std::vector<double>(buffer.BufferHt, 0.0));
    std::vector<std::vector<double>> pixel_age(buffer.BufferWi, std::vector<double>(buffer.BufferHt, 0.0));

    int num_colors = buffer.palette.Size();
    double eff_pos_adj = buffer.calcAccel(eff_pos, acceleration);
    double revs = (double)revolutions;

    double pos_x = buffer.BufferWi * center_x / 100.0;
    double pos_y = buffer.BufferHt * center_y / 100.0;

    double head_duration = duration / 100.0; // time the head is in the frame
    double tail_length = revs * (1.0 - head_duration);
    double color_length = tail_length / num_colors;
    if (color_length < 1.0)
        color_length = 1.0;

    double tail_end_of_tail = ((revs + tail_length) * eff_pos_adj) - tail_length;
    double head_end_of_tail = tail_end_of_tail + tail_length;

    double radius1 = start_radius;
    double radius2 = end_radius;
    double width1 = start_width;
    double width2 = end_width;

    if (scale) { // convert to percentage of buffer, i.e 100 is 100% of buffer size
        double bufferMax = std::max(buffer.BufferHt, buffer.BufferWi);
        radius1 = radius1 * (bufferMax / 200.0); // 200 bc radius is half of the width
        radius2 = radius2 * (bufferMax / 200.0);
        width1 = width1 * (bufferMax / 100.0);
        width2 = width2 * (bufferMax / 100.0);
    }

    double half_width = 1;

    buffer.ClearTempBuf();

    double last_check = (inward ? std::min(head_end_of_tail, revs) : std::max(0.0, tail_end_of_tail)) + (double)start_angle;

    // This section rounds off the head / tail
    // It draws whichever end is underneath as it spirals.  Head when Inward is true otherwise this draws the tail.
    double adj_angle;
    double end_angle = (inward ? std::min(head_end_of_tail, revs) : std::max(0.0, tail_end_of_tail));
    xlColor color;
    CalcEndpointColor(end_angle, start_angle, head_end_of_tail, color_length, num_colors, buffer, color);
    double pct1 = end_angle / revs;
    double current_radius = radius2 * pct1 + radius1 * (1.0 - pct1);
    double current_width = width2 * pct1 + width1 * (1.0 - pct1);
    ;
    double current_delta = 0.0;
    double current_distance = 0.0;
    half_width = current_width / 2.0;
    double step = GetStep(current_radius + half_width);

    if (current_radius >= half_width && half_width > 0.0) {
        for (double i = end_angle; current_distance <= half_width; (inward ? i += step : i -= step)) {
            adj_angle = i + (double)start_angle;
            if (reverse_dir) {
                adj_angle *= -1.0;
            }
            current_delta = std::abs(end_angle - i);
            current_distance = (2.0 * PI * current_radius * current_delta) / 360.0;
            HSVValue hsv(color);
            double full_brightness = hsv.value;
            if (half_width > current_distance) {
                current_width = std::sqrt(half_width * half_width - current_distance * current_distance);
                double inside_radius = std::max(0.0, current_radius - current_width);
                for (double r = inside_radius;; r += 0.5) {
                    if (r > current_radius)
                        r = current_radius;
                    double x1 = buffer.sin(ToRadians(adj_angle)) * r + (double)pos_x;
                    double y1 = buffer.cos(ToRadians(adj_angle)) * r + (double)pos_y;
                    double outside_radius = current_radius + (current_radius - r);
                    double x2 = buffer.sin(ToRadians(adj_angle)) * outside_radius + (double)pos_x;
                    double y2 = buffer.cos(ToRadians(adj_angle)) * outside_radius + (double)pos_y;
                    double head_fade_pct = 1.0 - (current_distance / half_width);
                    head_fade_pct = std::max(0.0, head_fade_pct);
                    head_fade_pct = std::min(1.0, head_fade_pct);
                    double color_pct2 = ((r - inside_radius) / (current_radius - inside_radius)) * head_fade_pct;
                    if (blend_edges) {
                        if (hsv.value > 0.0) {
                            if ((int)x1 >= 0 && (int)x1 < buffer.BufferWi && (int)y1 >= 0 && (int)y1 < buffer.BufferHt) {
                                buffer.SetTempPixel((int)x1, (int)y1, color);
                                temp_colors_pct[(int)x1][(int)y1] = color_pct2;
                            }
                            if ((int)x2 >= 0 && (int)x2 < buffer.BufferWi && (int)y2 >= 0 && (int)y2 < buffer.BufferHt) {
                                buffer.SetTempPixel((int)x2, (int)y2, color);
                                temp_colors_pct[(int)x2][(int)y2] = color_pct2;
                            }
                        }
                    } else {
                        hsv.value = full_brightness * color_pct2;
                        if (hsv.value > 0.0) {
                            buffer.SetPixel(x1, y1, hsv);
                            buffer.SetPixel(x2, y2, hsv);
                        }
                    }
                    if (r >= current_radius)
                        break;
                }
            }
            step = GetStep(current_radius + half_width);
        }
    }

    // This section draws the main Galaxy spiral
    for (double i = (inward ? std::min(head_end_of_tail, revs) : std::max(0.0, tail_end_of_tail));
         (inward ? i >= std::max(0.0, tail_end_of_tail) : i <= std::min(head_end_of_tail, revs));
         (inward ? i -= step : i += step)) {
        double adj_angle = i + (double)start_angle;
        if (reverse_dir) {
            adj_angle *= -1.0;
        }
        double color_val = (head_end_of_tail - i) / color_length;
        int color_int = (int)color_val;
        double color_pct = color_val - (double)color_int;
        int color2 = std::min(color_int + 1, num_colors - 1);
        if (color_int < color2) {
            buffer.Get2ColorBlend(color_int, color2, std::min(color_pct, 1.0), color);
        } else {
            buffer.palette.GetColor(color2, color);
        }
        HSVValue hsv(color);
        double full_brightness = hsv.value;
        double pct = i / revs;
        current_radius = radius2 * pct + radius1 * (1.0 - pct);
        double current_width = width2 * pct + width1 * (1.0 - pct);
        half_width = current_width / 2.0;
        double inside_radius = current_radius - half_width;
        for (double r = inside_radius;; r += 0.5) {
            if (r > current_radius)
                r = current_radius;
            double x1 = buffer.sin(ToRadians(adj_angle)) * r + (double)pos_x;
            double y1 = buffer.cos(ToRadians(adj_angle)) * r + (double)pos_y;
            double outside_radius = current_radius + (current_radius - r);
            double x2 = buffer.sin(ToRadians(adj_angle)) * outside_radius + (double)pos_x;
            double y2 = buffer.cos(ToRadians(adj_angle)) * outside_radius + (double)pos_y;
            double color_pct2 = (r - inside_radius) / (current_radius - inside_radius);
            if (blend_edges) {
                if (hsv.value > 0.0) {
                    if ((int)x1 >= 0 && (int)x1 < buffer.BufferWi && (int)y1 >= 0 && (int)y1 < buffer.BufferHt) {
                        buffer.SetTempPixel((int)x1, (int)y1, color);
                        temp_colors_pct[(int)x1][(int)y1] = color_pct2;
                        pixel_age[(int)x1][(int)y1] = abs(adj_angle);
                    }
                    if ((int)x2 >= 0 && (int)x2 < buffer.BufferWi && (int)y2 >= 0 && (int)y2 < buffer.BufferHt) {
                        buffer.SetTempPixel((int)x2, (int)y2, color);
                        temp_colors_pct[(int)x2][(int)y2] = color_pct2;
                        pixel_age[(int)x2][(int)y2] = abs(adj_angle);
                    }
                }
            } else {
                hsv.value = full_brightness * color_pct2;
                if (hsv.value > 0.0) {
                    buffer.SetPixel(x1, y1, hsv);
                    buffer.SetPixel(x2, y2, hsv);
                }
            }
            if (r >= current_radius)
                break;
        }

        // blend old data down into final buffer
        if (blend_edges && ((inward ? (last_check - abs(adj_angle)) : (abs(adj_angle) - last_check)) >= 90.0)) {
            for (int x = 0; x < buffer.BufferWi; x++) {
                for (int y = 0; y < buffer.BufferHt; y++) {
                    if (temp_colors_pct[x][y] > 0.0 && ((inward ? (pixel_age[x][y] - abs(adj_angle)) : (abs(adj_angle) - pixel_age[x][y])) >= 180.0)) {
                        xlColor c_new;
                        buffer.GetTempPixel(x, y, c_new);
                        xlColor c_old;
                        buffer.GetPixel(x, y, c_old);
                        xlColor colour;
                        buffer.Get2ColorAlphaBlend(c_old, c_new, temp_colors_pct[x][y], colour);
                        buffer.SetPixel(x, y, colour);
                        temp_colors_pct[x][y] = 0.0;
                        pixel_age[x][y] = 0.0;
                    }
                }
            }
            last_check = abs(adj_angle);
        }
        step = GetStep(current_radius + half_width);
    }

    // This section rounds off the head / tail
    // Draws whichever end wasn't drawn in the top section.
    end_angle = inward ? std::max(0.1, tail_end_of_tail) : std::min(head_end_of_tail, revs);
    CalcEndpointColor(end_angle, start_angle, head_end_of_tail, color_length, num_colors, buffer, color);
    current_distance = 0.0;
    if (current_radius >= half_width && half_width > 0.0) {
        for (double i = end_angle; current_distance <= half_width; (inward ? i -= step : i += step)) {
            adj_angle = i + (double)start_angle;
            if (reverse_dir) {
                adj_angle *= -1.0;
            }
            current_delta = std::abs(end_angle - i);
            current_distance = (2.0 * PI * current_radius * current_delta) / 360.0;
            HSVValue hsv(color);
            double full_brightness = hsv.value;
            if (half_width > current_distance) {
                current_width = std::sqrt(half_width * half_width - current_distance * current_distance);
                double inside_radius = std::max(0.0, current_radius - current_width);
                for (double r = inside_radius;; r += 0.5) {
                    if (r > current_radius)
                        r = current_radius;
                    double x1 = buffer.sin(ToRadians(adj_angle)) * r + (double)pos_x;
                    double y1 = buffer.cos(ToRadians(adj_angle)) * r + (double)pos_y;
                    double outside_radius = current_radius + (current_radius - r);
                    double x2 = buffer.sin(ToRadians(adj_angle)) * outside_radius + (double)pos_x;
                    double y2 = buffer.cos(ToRadians(adj_angle)) * outside_radius + (double)pos_y;
                    double head_fade_pct = 1.0 - (current_distance / half_width);
                    head_fade_pct = std::max(0.0, head_fade_pct);
                    head_fade_pct = std::min(1.0, head_fade_pct);
                    double color_pct2 = ((r - inside_radius) / (current_radius - inside_radius)) * head_fade_pct;
                    if (blend_edges) {
                        if (hsv.value > 0.0) {
                            if ((int)x1 >= 0 && (int)x1 < buffer.BufferWi && (int)y1 >= 0 && (int)y1 < buffer.BufferHt) {
                                buffer.SetTempPixel((int)x1, (int)y1, color);
                                temp_colors_pct[(int)x1][(int)y1] = color_pct2;
                            }
                            if ((int)x2 >= 0 && (int)x2 < buffer.BufferWi && (int)y2 >= 0 && (int)y2 < buffer.BufferHt) {
                                buffer.SetTempPixel((int)x2, (int)y2, color);
                                temp_colors_pct[(int)x2][(int)y2] = color_pct2;
                            }
                        }
                    } else {
                        hsv.value = full_brightness * color_pct2;
                        if (hsv.value > 0.0) {
                            buffer.SetPixel(x1, y1, hsv);
                            buffer.SetPixel(x2, y2, hsv);
                        }
                    }
                    if (r >= current_radius)
                        break;
                }
            }
            step = GetStep(current_radius + half_width);
        }
    }

    // blend remaining data down into final buffer
    if (blend_edges) {
        parallel_for(0, buffer.BufferWi, [&buffer, &temp_colors_pct](int x) {
            for (int y = 0; y < buffer.BufferHt; y++) {
                if (temp_colors_pct[x][y] > 0.0) {
                    xlColor c_new;
                    buffer.GetTempPixel(x, y, c_new);
                    xlColor c_old;
                    buffer.GetPixel(x, y, c_old);
                    xlColor colour;
                    buffer.Get2ColorAlphaBlend(c_old, c_new, temp_colors_pct[x][y], colour);
                    buffer.SetPixel(x, y, colour);
                }
            }
        });
    }
}

