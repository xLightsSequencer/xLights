/***************************************************************
 * Name:      RenderFan.cpp
 * Purpose:   Implements RGB effects
 * Author:    Gil Jones (gil@threebuttes.com)
 * Created:   2015-4-18
 * Copyright: 2015 by Gil Jones
 * License:
     This file is part of xLights.

    xLights is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    xLights is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with xLights.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************/

#include "RgbEffects.h"
#include <cmath>

const double PI  =3.141592653589793238463;
#define ToRadians(x) ((double)x * PI / (double)180.0)

void RgbEffects::RenderFan(int center_x, int center_y, int start_radius, int end_radius, int start_angle, int revolutions,
                           int duration, int acceleration, bool reverse_dir, bool blend_edges,
                           int num_blades, int blade_width, int blade_angle, int num_elements, int element_width )
{
    double step = 0.5;
    double temp_colors_pct[BufferWi][BufferHt];
    double eff_pos = GetEffectTimeIntervalPosition();
    int num_colors = palette.Size();
    if( num_colors == 0 )
        num_colors = 1;
    xlColor color, c_old, c_new;
    double eff_pos_adj = eff_pos * calcAccel(eff_pos, acceleration);
    double revs = (double)revolutions;

    double pos_x = BufferWi * center_x/100.0;
    double pos_y = BufferHt * center_y/100.0;

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

    for( int x = 0; x < BufferWi; x++ )
    {
        for( int y = 0; y < BufferHt; y++ )
        {
            temp_colors_pct[x][y] = 0.0;
        }
    }
    ClearTempBuf();

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
                    for( double i = radius1; i <= radius2; i += step )
                    {
                        double calc_angle = i / a_const * 180.0 / PI;
                        double adj_angle = calc_angle + blade_element_start_angle + current_angle;
                        if( blade_angle < 0.0 )
                        {
                            adj_angle *= -1.0;
                        }
                        palette.GetColor(section, color);

                        double x1 = std::sin(ToRadians(adj_angle)) * i + (double)pos_x;
                        double y1 = std::cos(ToRadians(adj_angle)) * i + (double)pos_y;

                        if( blend_edges )
                        {
                            if( color_pct > 0.0 )
                            {
                                if (x1 >= 0 && x1 < BufferWi && y1 >= 0 && y1 < BufferHt)
                                {
                                    temp_colors_pct[(int)x1][(int)y1] = color_pct;
                                    SetTempPixel((int)x1,(int)y1,color);
                                }
                            }
                        }
                        else
                        {
                            SetPixel((int)x1,(int)y1,color);
                        }
                    }
                }
                // blend element data into final buffer
                if( blend_edges )
                {
                    for( int x = 0; x < BufferWi; x++ )
                    {
                        for( int y = 0; y < BufferHt; y++ )
                        {
                            if( temp_colors_pct[x][y] > 0.0 )
                            {
                                GetTempPixel(x,y,c_new);
                                GetPixel(x,y,c_old);
                                Get2ColorAlphaBlend(c_old, c_new, temp_colors_pct[x][y], color);
                                SetPixel(x,y,color);
                                temp_colors_pct[x][y] = 0.0;
                            }
                        }
                    }
                }
            }
        }
    }
}

