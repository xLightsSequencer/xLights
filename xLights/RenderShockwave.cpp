/***************************************************************
 * Name:      RenderShockwave.cpp
 * Purpose:   Implements RGB effects
 * Author:    Gil Jones (gil@threebuttes.com)
 * Created:   2015-4-25
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

double RgbEffects::GetStepAngle(int width, int height)
{
    double step = 0.5;
    int biggest = std::max(width, height);
    if( biggest > 50 )
    {
        step = 0.4;
    }
    if( biggest > 150 )
    {
        step = 0.3;
    }
    if( biggest > 250 )
    {
        step = 0.2;
    }
    if( biggest > 400 )
    {
        step = 0.1;
    }
    return step;
}

void RgbEffects::RenderShockwave(int center_x, int center_y, int start_radius, int end_radius,
                                 int start_width, int end_width, int acceleration, bool blend_edges )
{
    double step = GetStepAngle(BufferWi, BufferHt);
    std::vector< std::vector<double> > temp_colors_pct(BufferWi, std::vector<double>(BufferHt));
    double eff_pos = GetEffectTimeIntervalPosition();
    int num_colors = palette.Size();
    if( num_colors == 0 )
        num_colors = 1;
    xlColor color, c_old, c_new;
    double eff_pos_adj = eff_pos * calcAccel(eff_pos, acceleration);

    double blend_pct = 1.0 / (num_colors-1);
    double color_pct1 = eff_pos_adj / blend_pct;
    int color_index = (int)color_pct1;
    blend_pct = color_pct1 - (double)color_index;
    Get2ColorBlend(color_index, std::min(color_index+1,num_colors-1), std::min( blend_pct, 1.0), color);

    double pos_x = BufferWi * center_x/100.0;
    double pos_y = BufferHt * center_y/100.0;

    double radius1 = start_radius;
    double radius2 = end_radius;
    double radius_center = radius1 + (radius2 - radius1) * eff_pos_adj;
    double half_width = (start_width + (end_width - start_width) * eff_pos_adj) / 2.0;
    radius1 = radius_center - half_width;
    radius2 = radius_center + half_width;

    for( int x = 0; x < BufferWi; x++ )
    {
        for( int y = 0; y < BufferHt; y++ )
        {
            temp_colors_pct[x][y] = 0.0;
        }
    }
    ClearTempBuf();

    for( double current_angle = 0.0; current_angle <= 360.0; current_angle += step )
    {
        for( double r = std::max(0.0, radius1); r <= radius2; r += step )
        {
            double x1 = std::sin(ToRadians(current_angle)) * r + (double)pos_x;
            double y1 = std::cos(ToRadians(current_angle)) * r + (double)pos_y;

            if( blend_edges )
            {
                double color_pct = 1.0 - std::abs(r-radius_center)/half_width;
                if( color_pct > 0.0 )
                {
                    if (x1 >= 0 && x1 < BufferWi && y1 >= 0 && y1 < BufferHt)
                    {
                        if (allowAlpha) {
                            color.alpha = 255.0 * color_pct;
                            SetPixel((int)x1,(int)y1,color);
                        } else {
                            temp_colors_pct[(int)x1][(int)y1] = color_pct;
                            SetTempPixel((int)x1,(int)y1,color);
                        }
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
    if( blend_edges && !allowAlpha )
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

