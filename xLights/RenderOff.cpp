/***************************************************************
 * Name:      RenderOff.cpp
 * Purpose:   Implements RGB effects
 * Author:    Sean Meighan (sean@meighan.net)
 * Created:   2014-11-11
 * Copyright: 2014 by Sean Meighan
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

void RgbEffects::RenderOff(void)
{
    int x,y,d,xc,yc,x0,y0;


    wxImage::HSVValue hsv;

//  Every Node, every frame set to BLACK
    for (x=0; x<BufferWi; x++)
    {
        for (y=0; y<BufferHt; y++)
        {
            hsv.saturation=0.0; // Setup BLACK
            hsv.value=0.0;
            hsv.hue=0.0;
            SetPixel(x,y,hsv);
        }
    }
}
