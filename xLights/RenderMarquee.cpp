/***************************************************************
 * Name:      RenderMarquee.cpp
 * Purpose:   Implements RGB effects
 * Author:    Gil Jones (gil@threebuttes.com)
 * Created:   2015-8-28
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

#include <cmath>
#include "RgbEffects.h"

static void UpdateMarqueeColor(int &position, int &band_color, int colorcnt, int color_size, int shift)
{
    if( shift == 0 ) return;
    if( shift > 0 )
    {
        int index = 0;
        while( index < shift )
        {
            position++;
            if( position >= color_size )
            {
                band_color++;
                band_color %= colorcnt;
                position = 0;
            }
            index++;
        }
    }
    else
    {
        int index = 0;
        while( index > shift )
        {
            position--;
            if( position < 0 )
            {
                band_color--;
                if( band_color < 0 ) band_color = colorcnt-1;
                position = color_size-1;
            }
            index--;
        }
    }
}

void RgbEffects::RenderMarquee(int BandSize, int SkipSize, int Thickness, int stagger, int mSpeed, int mStart, bool reverse_dir,
                               int x_scale, int y_scale, int xc_adj, int yc_adj, bool pixelOffsets, bool wrap_x)
{
    int x = 0;
    xlColour color;
    size_t colorcnt = GetColorCount();
    int color_size = BandSize +  SkipSize;
    int repeat_size = color_size * colorcnt;
    int eff_pos = curPeriod - curEffStartPer;

    x = (mSpeed * eff_pos) / 5;
    int corner_x1 = 0;
    int corner_y1 = 0;
    int corner_x2 = (int)((double)BufferWi * (double)x_scale / 100.0) - 1;
    int corner_y2 = (int)((double)BufferHt * (double)y_scale / 100.0) - 1;
    int sign = 1;
    if( reverse_dir ) {
        sign = -1;
    }

    int xoffset_adj = xc_adj;
    int yoffset_adj = yc_adj;
    if (!pixelOffsets) {
        xoffset_adj = (xoffset_adj*BufferWi)/100.0; // xc_adj is from -100 to 100
        yoffset_adj = (yoffset_adj*BufferHt)/100.0; // yc_adj is from -100 to 100
    }

    for( int thick = 0; thick < Thickness; thick++ )
    {
        int current_color = ((x + mStart) % repeat_size) / color_size;
        int current_pos = (((x + mStart) % repeat_size) % color_size);
        UpdateMarqueeColor(current_pos, current_color, colorcnt, color_size, thick*(stagger+1) * sign);
        for( int x_pos = corner_x1; x_pos <= corner_x2; x_pos++ )
        {
            color = xlBLACK;
            if( current_pos < BandSize )
            {
                palette.GetColor(current_color, color);
            }
            ProcessPixel(x_pos + xoffset_adj, corner_y2 + yoffset_adj, color, wrap_x, BufferWi);
            UpdateMarqueeColor(current_pos, current_color, colorcnt, color_size, 1*sign);
        }
        UpdateMarqueeColor(current_pos, current_color, colorcnt, color_size, thick*2*sign);
        for( int y_pos = corner_y2; y_pos >=corner_y1 ; y_pos-- )
        {
            color = xlBLACK;
            if( current_pos < BandSize )
            {
                palette.GetColor(current_color, color);
            }
            ProcessPixel(corner_x2 + xoffset_adj, y_pos + yoffset_adj, color, wrap_x, BufferWi);
            UpdateMarqueeColor(current_pos, current_color, colorcnt, color_size, 1*sign);
        }
        UpdateMarqueeColor(current_pos, current_color, colorcnt, color_size, thick*2*sign);
        for( int x_pos = corner_x2; x_pos >= corner_x1; x_pos-- )
        {
            color = xlBLACK;
            if( current_pos < BandSize )
            {
                palette.GetColor(current_color, color);
            }
            ProcessPixel(x_pos + xoffset_adj, corner_y1 + yoffset_adj, color, wrap_x, BufferWi);
            UpdateMarqueeColor(current_pos, current_color, colorcnt, color_size, 1*sign);
        }
        UpdateMarqueeColor(current_pos, current_color, colorcnt, color_size, thick*2*sign);
        for( int y_pos = corner_y1; y_pos <= corner_y2; y_pos++ )
        {
            color = xlBLACK;
            if( current_pos < BandSize )
            {
                palette.GetColor(current_color, color);
            }
            ProcessPixel(corner_x1 + xoffset_adj, y_pos + yoffset_adj, color, wrap_x, BufferWi);
            UpdateMarqueeColor(current_pos, current_color, colorcnt, color_size, 1*sign);
        }
        corner_x1++;
        corner_y1++;
        corner_x2--;
        corner_y2--;
    }
}
