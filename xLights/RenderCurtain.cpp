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
void RgbEffects::RenderCurtain(int edge, int effect, int swag, bool repeat)
{
    double a;
    wxImage::HSVValue hsv;
    wxArrayInt SwagArray;
    int CurtainDir,xlimit,middle;
    int swaglen=BufferHt > 1 ? swag * BufferWi / 40 : 0;
    if (swaglen > 0) {
        a=double(BufferHt - 1) / (swaglen * swaglen);
        for (int x=0; x<swaglen; x++) {
            SwagArray.Add(int(a*x*x));
        }
    }
    if (effect < 2) {
        xlimit=repeat || state < 200 ? (state % 200) * BufferWi / 199 : BufferWi;
    } else {
        xlimit=repeat || state < 400 ? (state % 200) * BufferWi / 199 : 0;
    }
    if (state == 0 || effect < 2) {
        CurtainDir=effect % 2;
    } else if (xlimit < LastCurtainLimit) {
        CurtainDir=1-LastCurtainDir;
    } else {
        CurtainDir=LastCurtainDir;
    }
    LastCurtainDir=CurtainDir;
    LastCurtainLimit=xlimit;
    if (CurtainDir==0) xlimit=BufferWi-xlimit-1;
    switch (edge) {
        case 0:
            // left
            DrawCurtain(true,xlimit,SwagArray);
            break;
        case 1:
            // center
            middle=(xlimit+1)/2;
            DrawCurtain(true,middle,SwagArray);
            DrawCurtain(false,middle,SwagArray);
            break;
        case 2:
            // right
            DrawCurtain(false,xlimit,SwagArray);
            break;
    }
}

void RgbEffects::DrawCurtain(bool LeftEdge, int xlimit, const wxArrayInt &SwagArray)
{
    int i,x,y;
    wxColour color;
    for (i=0; i<xlimit; i++)
    {
        GetMultiColorBlend(double(i) / double(BufferWi), true, color);
        x=LeftEdge ? BufferWi-i-1 : i;
        for (y=BufferHt-1; y>=0; y--)
        {
            SetPixel(x,y,color);
        }
    }

    // swag
    for (i=0; i<SwagArray.Count(); i++)
    {
        x=xlimit+i;
        GetMultiColorBlend(double(x) / double(BufferWi), true, color);
        if (LeftEdge) x=BufferWi-x-1;
        for (y=BufferHt-1; y>SwagArray[i]; y--)
        {
            SetPixel(x,y,color);
        }
    }
}
