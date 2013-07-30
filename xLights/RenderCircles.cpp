/***************************************************************
 * Name:      RenderCircles
 * Purpose:   Extends RGB effects with Circle effects
 * Author:    Frank Reichstein (phar_hp@yahoo.com)
 * Created:   2013-07-28
 * Copyright: 2013 by Frank Reichstein
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


void RgbEffects::RenderCircles(int number,int radius, bool bounce, bool collide, bool random,
                               bool radial, int start_x, int start_y)
{
    int ii;
    int colorIdx;
    size_t colorCnt = palette.Size();
    wxImage::HSVValue hsv;
    float spd;
    float angle;

    if (radial)
    {
        RenderRadial(start_x, start_y, radius, colorCnt);
        return; //radial is the easiest case so just get out.
    }

    if ( 0 == state )
    {
        for(ii=0; ii<number; ii++)
        {
            start_x = rand()%(BufferWi-2*radius) + radius;
            start_y = rand()%(BufferHt-2*radius) + radius;
            colorIdx = ii%colorCnt;
            palette.GetHSV(colorIdx, hsv);
            spd = rand()%5;
            angle = rand()%90;
            balls[ii].Reset((float) start_x, (float) start_y, spd, angle, (float)radius, hsv);
        }
    }
    else
    {
        RenderCirclesUpdate(number);
    }

    if (bounce)
    {
        //update position in case something hit a wall
    }
    if(collide)
    {
        //update position if two balls collided
    }

    for (ii=0; ii<number; ii++)
    {
        hsv = balls[ii].hsvcolor;
        for(int r = balls[ii]._radius; r >= 0; r--)
        {
            DrawCircle(balls[ii]._x, balls[ii]._y, r, hsv);
        }
    }

}

void RgbEffects::RenderCirclesUpdate(int ballCnt)
{
    int ii;
    for (ii=0; ii <ballCnt; ii++)
    {
        balls[ii].updatePosition(speed);
    }
}

void RgbEffects::RenderRadial(int x, int y,int thickness, int colorCnt)
{
    wxImage::HSVValue hsv;
    int ii;
    int colorIdx;
    int maxRadius = state + thickness;

    palette.GetHSV(0,hsv);
    for( ii = maxRadius ; ii >= 0;  ii--)
    {
        if ( 0 == ii%thickness)
        {
            colorIdx = (colorIdx+1)%colorCnt;
            palette.GetHSV(colorIdx,hsv);
        }
        DrawCircle(x, y, ii, hsv);
    }
}

