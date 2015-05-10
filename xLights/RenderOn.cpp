/***************************************************************
 * Name:      RenderOn.cpp
 * Purpose:   Implements RGB effects
 * Author:    Gil Jones (gil@threebuttes.com)
 * Created:   2014-12-12
 * Copyright: 2014 by Gil Jones
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

void RgbEffects::RenderOn(int start, int end)
{
    int x,y;

    xlColor color;
    if (start == 100 && end == 100) {
        palette.GetColor(0, color);
    } else {
        wxImage::HSVValue hsv;
        palette.GetHSV(0,hsv);
        double d;
        if (!fitToTime) {
            d = state/200.0;
            while (d > 1.0) {
                d -= 1.0;
            }
        } else {
            d = GetEffectTimeIntervalPosition();
        }
        d = start + (end - start) * d;
        d = d / 100.0;
        hsv.value = hsv.value * d;
        color = hsv;
    }

    //Every Node, every frame set to selected color
    for (x=0; x<BufferWi; x++)
    {
        for (y=0; y<BufferHt; y++)
        {
            SetPixel(x,y,color);
        }
    }
}
