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


class FirePaletteClass {
public:
    FirePaletteClass() {
        wxImage::HSVValue hsv;
        int i;
        // calc 100 reds, black to bright red
        hsv.hue=0.0;
        hsv.saturation=1.0;
        for (i=0; i<100; i++)
        {
            hsv.value=double(i)/100.0;
            firePalette.push_back(hsv);
            firePaletteColors.push_back(hsv);
            firePaletteColorsAlpha.push_back(xlColor(255, 0, 0, i * 255 / 100));
        }
        
        // gives 100 hues red to yellow
        hsv.value=1.0;
        for (i=0; i<100; i++)
        {
            firePalette.push_back(hsv);
            firePaletteColors.push_back(hsv);
            firePaletteColorsAlpha.push_back(hsv);
            hsv.hue+=0.00166666;
        }
    }
    int size() const {
        return firePalette.size();
    }
    const wxImage::HSVValue &operator[](int x) const {
        return firePalette[x];
    }
    const xlColor &asColor(int x) const {
        return firePaletteColors[x];
    }
    const xlColor &asAlphaColor(int x) const {
        return firePaletteColorsAlpha[x];
    }

private:
    hsvVector firePalette;
    xlColorVector firePaletteColors;
    xlColorVector firePaletteColorsAlpha;
};
static const FirePaletteClass FirePalette;

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
void RgbEffects::RenderFire(int HeightPct,int HueShift,int cycles)
{
    int x,y,i,r,v1,v2,v3,v4,n,new_index;
    wxImage::HSVValue hsv;

    //cycles is 0 - 200 representing growth cycle count of 0 - 20
    if (cycles > 0) {
        double adjust = GetEffectTimeIntervalPosition();
        adjust *= cycles / 10.0;
        while (adjust > 1.0) {
            adjust -= 1.0;
        }
        adjust = 0.5 - std::abs(adjust - 0.5);
        HeightPct += adjust * 100;
    }
    
    if (HeightPct<1) HeightPct=1;
    if (BufferHt<1) BufferHt=1;

    float mod_state = 4.0;
    if (curPeriod == curEffStartPer) {
        for (i=0; i < FireBuffer.size(); i++) {
            FireBuffer[i]=0;
        }
    } else {
        mod_state = 4 / (curPeriod%4+1);
    }
    // build fire
    for (x=0; x<BufferWi; x++) {
        r=x%2==0 ? 190+(rand() % 10) : 100+(rand() % 50);
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

            if (HueShift>0) {
                hsv = FirePalette[GetFireBuffer(x,y)];
                hsv.hue = hsv.hue +(HueShift/100.0);
                if (hsv.hue>1.0) hsv.hue=1.0;
                if (allowAlpha) {
                    xlColor c(hsv);
                    c.alpha = FirePalette.asAlphaColor(GetFireBuffer(x,y)).Alpha();
                    SetPixel(x, y, c);
                } else {
                    SetPixel(x, y, hsv);
                }
            } else {
                if (allowAlpha) {
                    SetPixel(x, y, FirePalette.asAlphaColor(GetFireBuffer(x,y)));
                } else {
                    SetPixel(x, y, FirePalette.asColor(GetFireBuffer(x,y)));
                }
            }
        }
    }
}
