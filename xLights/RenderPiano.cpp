/***************************************************************
 * Name:      RenderPiano.cpp
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

void RgbEffects::RenderPiano(int Keyboard)
{
    wxImage::HSVValue hsv;
    wxColour color;
    int x,y,xc,yc,n,pixel_ratio,ColorIdx;
    size_t colorcnt=GetColorCount();
    int keys,keys_mod,width,height;
    switch (Keyboard)
    {
    case 1: // smallest keyboard
    {
        width=1;
        height=3;
        break;
    }
    case 2: // medium keyboard
    {
        width=2;

        height=5;
        break;
    }
    case 3: // largest keyboard
    {
        width=3;
        height=10;
        break;
    }
    }
    //  BufferWi ,BufferHt
    int y_start = BufferHt/2 - height;
    int y_end = y_start + height;
    int x_start=1;
    int x_end;

    ColorIdx=rand() % colorcnt;
    palette.GetColor(ColorIdx,color);
    Color2HSV(color,hsv);
    for (keys=1; keys<=14; keys++)
    {
        keys_mod=keys%12;
        switch (keys_mod)
        {
        case 1: // C
        {
            x_end=x_start+(keys*width);
            for(x=x_start; x<=x_end; x++)
            {
                for (y=y_start; yc<=y_end; y++)
                {
                    SetPixel(x,y,color);
                }
            }
        }
        default:
       {
            x_end=x_start+(keys*width);
            for(x=x_start; x<=x_end; x++)
            {
                for (y=y_start; yc<=y_end; y++)
                {
                    SetPixel(x,y,color);
                }
            }
        }
        } // switch (keys_mod)
    } //  for (keys=1; keys<=14; keys++)
}
