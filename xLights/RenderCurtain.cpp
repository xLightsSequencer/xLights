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
void RgbEffects::RenderCurtain(int edge, int effect, int swag, bool repeat, int curtainSpeed)
{
    double a;
    wxImage::HSVValue hsv;
    wxArrayInt SwagArray;
    int CurtainDir,xlimit,middle, ylimit;
    int swaglen=BufferHt > 1 ? swag * BufferWi / 40 : 0;
    double position = GetEffectTimeIntervalPosition();

    if (swaglen > 0) {
        a=double(BufferHt - 1) / (swaglen * swaglen);
        for (int x=0; x<swaglen; x++) {
            SwagArray.Add(int(a*x*x));
        }
    }
    
    //cycles is 0 - 200 representing repeat count of 0 - 20
    if (curtainSpeed > 0) {
        position *= curtainSpeed / 10.0;
        if (repeat) {
            while (position > 1.0) {
                position -= 1.0;
            }
        } else if (position > 1.0) {
            position = 1.0;
        }
    }
    
    if (effect < E_CURTAIN_OPEN_CLOSE) {
        xlimit=position * BufferWi;
        ylimit=position * BufferHt;
    } else {
        xlimit= position <= .5 ? position * 2 * BufferWi: (position -.5) * 2 * BufferWi ;
        ylimit= position <= .5 ? position * 2 * BufferHt: (position -.5) * 2 * BufferHt ;
    }
    if (curPeriod == curEffStartPer || effect < E_CURTAIN_OPEN_CLOSE) {
        CurtainDir=effect % 2;
    } else if (xlimit < LastCurtainLimit) {
        CurtainDir=1-LastCurtainDir;
    } else {
        CurtainDir=LastCurtainDir;
    }
    LastCurtainDir=CurtainDir;
    LastCurtainLimit=xlimit;
    if (CurtainDir==0)
    {
        xlimit=BufferWi-xlimit-1;
        ylimit=BufferHt-ylimit-1;
    }
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
        case 3:
            DrawCurtainVertical(true, ylimit, SwagArray);
            break;
        case 4:
            middle=(ylimit+1)/2;
            DrawCurtainVertical(true, middle, SwagArray);
            DrawCurtainVertical(false, middle, SwagArray);
            break;
        case 5:
            DrawCurtainVertical(false, ylimit, SwagArray);
            break;
    }
}

void RgbEffects::DrawCurtain(bool LeftEdge, int xlimit, const wxArrayInt &SwagArray)
{
    int i,x,y;
    xlColour color;
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

void RgbEffects::DrawCurtainVertical(bool topEdge, int ylimit, const wxArrayInt &SwagArray)
{
    int i,x,y;
    xlColour color;
    for (i=0; i<ylimit; i++)
    {
        GetMultiColorBlend(double(i) / double(BufferHt), true, color);
        y=topEdge ? BufferHt-i-1 : i;
        for (x=BufferWi-1; x>=0; x--)
        {
            SetPixel(x,y,color);
        }
    }

    // swag
    for (i=0; i<SwagArray.Count(); i++)
    {
        y=ylimit+i;
        GetMultiColorBlend(double(y) / double(BufferHt), true, color);
        if (topEdge) y=BufferHt-y-1;
        for (x=BufferWi-1; x>SwagArray[i];x--)
        {
            SetPixel(x,y,color);
        }
    }
}
