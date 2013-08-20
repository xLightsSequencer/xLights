/***************************************************************
 * Name:      RgbCurtain.cpp
 * Purpose:   Implements RGB effects
 * Author:    Matt Brown (dowdybrown@yahoo.com)
 * Created:   2013-08-19
 * Copyright: 2013 by Matt Brown
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

// edge: 0=left, 1=center, 2=right
// effect: 0=open, 1=close, 2=open then close, 3=close then open
void RgbEffects::RenderCurtain(int edge, int effect, bool repeat)
{
    int x,y;
    wxColour color;
    wxImage::HSVValue hsv;
    //double HalfHt=double(BufferHt-1)/2.0;
    //double HalfWi=double(BufferWi-1)/2.0;
    for (x=0; x<BufferWi; x++)
    {
        GetMultiColorBlend(double(x) / double(BufferWi), true, color);
        for (y=0; y<BufferHt; y++)
        {
            SetPixel(x,y,color);
        }
    }
}
