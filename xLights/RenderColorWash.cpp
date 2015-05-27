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

void RgbEffects::RenderColorWash(bool HorizFade, bool VertFade, int cycles,
                                 bool EntireModel, int x1, int y1, int x2, int y2,
                                 bool shimmer,
                                 bool circularPalette)
{
    if (shimmer) {
        int tot = curPeriod - curEffStartPer;
        if (tot % 2) {
            return;
        }
    }
    int x,y;
    xlColour color, orig;
    
    double position = GetEffectTimeIntervalPosition();
    if (cycles > 0) {
        position *= cycles / 10.0;
        while (position > 1.0) {
            position -= 1.0;
        }
    }
    GetMultiColorBlend(position, circularPalette, color);

    int startX = 0;
    int startY = 0;
    int endX = BufferWi - 1;
    int endY = BufferHt - 1;
    
    if (!EntireModel) {
        startX = std::min(x1, x2);
        endX = std::max(x1, x2);
        startY = std::min(y1, y2);
        endY = std::max(y1, y2);
        startX = std::round(double(BufferWi - 0.5) * (double)startX / 100.0);
        endX = std::round(double(BufferWi - 0.5) * (double)endX / 100.0);
        startY = std::round(double(BufferHt - 0.5) * (double)startY / 100.0);
        endY = std::round(double(BufferHt - 0.5) * (double)endY / 100.0);
        startX = std::max(startX, 0);
        endX = std::min(endX, BufferWi - 1);
        startY = std::max(startY, 0);
        endY = std::min(endY, BufferHt - 1);
    }
    
    double HalfHt=double(endY - startY)/2.0;
    double HalfWi=double(endX - startX)/2.0;
    
    orig = color;
    wxImage::HSVValue hsvOrig = color.asHSV();
    for (x=startX; x <= endX; x++)
    {
        wxImage::HSVValue hsv = hsvOrig;
        if (HorizFade) {
            if (allowAlpha) {
                color.alpha = (double)orig.alpha*(1.0-std::abs(HalfWi-x-startX)/HalfWi);
            } else {
                hsv.value*=1.0-std::abs(HalfWi-x-startX)/HalfWi;
                color = hsv;
            }
        }
        for (y=startY; y<=endY; y++)
        {
            if (VertFade) {
                if (allowAlpha) {
                    color.alpha = (double)orig.alpha*(1.0-std::abs(HalfHt-(y-startY))/HalfHt);
                } else {
                    hsv.value*=1.0-std::abs(HalfHt-y-startY)/HalfHt;
                    color = hsv;
                }
            }
            SetPixel(x, y, color);
        }
    }
}
