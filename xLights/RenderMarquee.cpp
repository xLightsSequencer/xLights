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
            if( position > color_size )
            {
                band_color++;
                band_color %= colorcnt;
                position = 1;
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
            if( position < 1 )
            {
                band_color--;
                if( band_color < 0 ) band_color = colorcnt-1;
                position = color_size;
            }
            index--;
        }
    }
}

void RgbEffects::RenderMarquee(int BandSize, int SkipSize, int Thickness, int stagger, int mSpeed, bool reverse_dir)
{
    int x = 0;
    xlColour color;
    size_t colorcnt = GetColorCount();
    int color_size = BandSize +  SkipSize;
    int repeat_size = color_size * colorcnt;
    x = (mSpeed * effectState++) / 5;
    int corner_x1 = 0;
    int corner_y1 = 0;
    int corner_x2 = BufferWi-1;
    int corner_y2 = BufferHt-1;
    int sign = 1;
    if( reverse_dir ) {
        sign = -1;
    }

    for( int thick = 0; thick < Thickness; thick++ )
    {
        int current_color = (x % repeat_size) / color_size;
        int current_pos = ((x % repeat_size) % color_size);
        UpdateMarqueeColor(current_pos, current_color, colorcnt, color_size, thick*(stagger+1) * sign);
        for( int pos = corner_x1; pos <= corner_x2; pos++ )
        {
            color = xlBLACK;
            if( current_pos <= BandSize )
            {
                palette.GetColor(current_color, color);
            }
            SetPixel(pos, corner_y2, color);
            UpdateMarqueeColor(current_pos, current_color, colorcnt, color_size, 1*sign);
        }
        UpdateMarqueeColor(current_pos, current_color, colorcnt, color_size, thick*2*sign);
        for( int pos = corner_y2; pos >=corner_y1 ; pos-- )
        {
            color = xlBLACK;
            if( current_pos <= BandSize )
            {
                palette.GetColor(current_color, color);
            }
            SetPixel(corner_x2, pos, color);
            UpdateMarqueeColor(current_pos, current_color, colorcnt, color_size, 1*sign);
        }
        UpdateMarqueeColor(current_pos, current_color, colorcnt, color_size, thick*2*sign);
        for( int pos = corner_x2; pos >= corner_x1; pos-- )
        {
            color = xlBLACK;
            if( current_pos <= BandSize )
            {
                palette.GetColor(current_color, color);
            }
            SetPixel(pos, corner_y1, color);
            UpdateMarqueeColor(current_pos, current_color, colorcnt, color_size, 1*sign);
        }
        UpdateMarqueeColor(current_pos, current_color, colorcnt, color_size, thick*2*sign);
        for( int pos = corner_y1; pos <= corner_y2; pos++ )
        {
            color = xlBLACK;
            if( current_pos <= BandSize )
            {
                palette.GetColor(current_color, color);
            }
            SetPixel(corner_x1, pos, color);
            UpdateMarqueeColor(current_pos, current_color, colorcnt, color_size, 1*sign);
        }
        corner_x1++;
        corner_y1++;
        corner_x2--;
        corner_y2--;
    }
}
