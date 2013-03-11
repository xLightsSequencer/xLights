/***************************************************************
 * Name:      RgbEffects.cpp
 * Purpose:   Implements RGB effects
 * Author:    Matt Brown (dowdybrown@yahoo.com)
 * Created:   2012-12-23
 * Copyright: 2012 by Matt Brown
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

void RgbEffects::RenderColorWash(bool HorizFade, bool VertFade, int RepeatCount)
{
    static int SpeedFactor=200;
    int x,y;
    wxColour color;
    wxImage::HSVValue hsv,hsv2;
    size_t colorcnt=GetColorCount();
    int CycleLen=colorcnt*SpeedFactor;
    if (state > (colorcnt-1)*SpeedFactor*RepeatCount && RepeatCount < 10)
    {
        GetMultiColorBlend(double(RepeatCount%2), false, color);
    }
    else
    {
        GetMultiColorBlend(double(state % CycleLen) / double(CycleLen), true, color);
    }
    Color2HSV(color,hsv);
    double HalfHt=double(BufferHt-1)/2.0;
    double HalfWi=double(BufferWi-1)/2.0;
    for (x=0; x<BufferWi; x++)
    {
        for (y=0; y<BufferHt; y++)
        {
            hsv2=hsv;
            if (HorizFade) hsv2.value*=1.0-abs(HalfWi-x)/HalfWi;
            if (VertFade) hsv2.value*=1.0-abs(HalfHt-y)/HalfHt;
            SetPixel(x,y,hsv2);
        }
    }
}
