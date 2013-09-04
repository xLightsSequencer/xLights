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

// 0 <= x < BufferWi
// 0 <= y < BufferHt
void RgbEffects::SetFireBuffer(int x, int y, int PaletteIdx)
{
    if (x >= 0 && x < BufferWi && y >= 0 && y < BufferHt)
    {
        FireBuffer[y*BufferWi+x] = PaletteIdx;
    }
}

// 0 <= x < BufferWi
// 0 <= y < BufferHt
int RgbEffects::GetFireBuffer(int x, int y)
{
    if (x >= 0 && x < BufferWi && y >= 0 && y < BufferHt)
    {
        return FireBuffer[y*BufferWi+x];
    }
    return -1;
}

void RgbEffects::SetWaveBuffer1(int x, int y, int value)
{
    if (x >= 0 && x < BufferWi && y >= 0 && y < BufferHt)
    {
        WaveBuffer1[y*BufferWi+x] = value;
    }
}
void RgbEffects::SetWaveBuffer2(int x, int y, int value)
{
    if (x >= 0 && x < BufferWi && y >= 0 && y < BufferHt)
    {
        WaveBuffer2[y*BufferWi+x] = value;
    }
}

int RgbEffects::GetWaveBuffer1(int x, int y)
{
    if (x >= 0 && x < BufferWi && y >= 0 && y < BufferHt)
    {
        return WaveBuffer1[y*BufferWi+x];
    }
    return -1;
}
int RgbEffects::GetWaveBuffer2(int x, int y)
{
    if (x >= 0 && x < BufferWi && y >= 0 && y < BufferHt)
    {
        return WaveBuffer2[y*BufferWi+x];
    }
    return -1;
}


// 10 <= HeightPct <= 100
void RgbEffects::RenderFire(int HeightPct,int HueShift,bool GrowFire)
{
    int x,y,i,r,v1,v2,v3,v4,n,new_index;
    wxColour color;
    wxImage::HSVValue hsv;



    if(GrowFire) HeightPct+=(state%500)/10;
    if(HeightPct<1) HeightPct=1;
    if(BufferHt<1) BufferHt=1;

    if (state == 0)
    {
        for (i=0; i < FireBuffer.size(); i++)
        {
            FireBuffer[i]=0;
        }
    }
    // build fire
    for (x=0; x<BufferWi; x++)
    {
        r=x%2==0 ? 190+(rand() % 10) : 100+(rand() % 50);
        // r=x%2==0 ? 190+((state+rand()) % 10) : 100+((state+rand()) % 50);
        SetFireBuffer(x,0,r);
    }
    int step=255*100/BufferHt/HeightPct;
    int sum;
    for (y=1; y<BufferHt; y++)
    {
        for (x=0; x<BufferWi; x++)
        {
            v1=GetFireBuffer(x-1,y-1);
            v2=GetFireBuffer(x+1,y-1);
            v3=GetFireBuffer(x,y-1);
            v4=GetFireBuffer(x,y-1);
            n=0;
            sum=0;
            if(v1>=0)
            {
                sum+=v1;
                n++;
            }
            if(v2>=0)
            {
                sum+=v2;
                n++;
            }
            if(v3>=0)
            {
                sum+=v3;
                n++;
            }
            if(v4>=0)
            {
                sum+=v4;
                n++;
            }
            new_index=n > 0 ? sum / n : 0;
            if (new_index > 0)
            {
                new_index+=(rand() % 100 < 20) ? step : -step;
                if (new_index < 0) new_index=0;
                if (new_index >= FirePalette.size()) new_index = FirePalette.size()-1;
            }
            SetFireBuffer(x,y,new_index);
        }
    }

    //  Now play fire
    float rx,ry;
    float mod_state;
    mod_state = 4 / (state%4+1);
    ry = 1.0 / mod_state;
    for (y=0; y<BufferHt; y++)
    {
        //y=(int)ry;
        for (x=0; x<BufferWi; x++)
        {
            rx = 1.0 / mod_state;
           // x=(int)rx;
            //SetPixel(x,y,FirePalette[y]);
            //  first get the calculated color fo normal fire.

            color=FirePalette[GetFireBuffer(x,y)];

            if(HueShift>0)
            {
                wxImage::RGBValue rgb(color.Red(),color.Green(),color.Blue());
                hsv = wxImage::RGBtoHSV(rgb);
                hsv.hue = hsv.hue +(HueShift/100.0);
                if (hsv.hue>1.0) hsv.hue=1.0;
                rgb = wxImage::HSVtoRGB(hsv);
                color = wxColor(rgb.red,rgb.green,rgb.blue);
            }
            SetPixel(x,y,color);
        }
    }
}
