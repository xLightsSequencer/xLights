/***************************************************************
 * Name:      RenderFireworks.cpp
 * Purpose:   Implements Fireworks and Explosions effects
 * Author:    Dave Pitts (dowdybrown@yahoo.com)
 *            Sean Meighan
 * Created:   2012-12-23
 * Copyright: 2013 by Sean Meighan
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

//Max of 50 explosions * 100 particals
static const int maxFlakes = 5000;

void RgbEffects::RenderFireworks(int Number_Explosions,int Count,float Velocity,int Fade)
{
    int i=0;
    int x25,x75,y25,y75;
    //float velocity = 3.5;
    int startX;
    int startY,ColorIdx;
    float v;
    wxImage::HSVValue hsv;
    size_t colorcnt=GetColorCount();
    
    if (fireworkBursts == NULL) {
        fireworkBursts = new RgbFireworks[maxFlakes];
    }

    if (curPeriod == curEffStartPer) {
        for(i=0; i<maxFlakes; i++) {
            fireworkBursts[i]._bActive = false;
        }
        for (int x = 0; x < Number_Explosions; x++) {
            double start = curEffStartPer + rand01() * (curEffEndPer - curEffStartPer);
            
            x25=(int)BufferWi*0.25;
            x75=(int)BufferWi*0.75;
            y25=(int)BufferHt*0.25;
            y75=(int)BufferHt*0.75;
            startX=(int)BufferWi/2;
            startY=(int)BufferHt/2;
            if((x75-x25)>0) startX = x25 + rand()%(x75-x25); else startX=0;
            if((y75-y25)>0) startY = y25 + rand()%(y75-y25); else startY=0;
            
            // Create a new burst
            ColorIdx=rand() % colorcnt; // Select random numbers from 0 up to number of colors the user has checked. 0-5 if 6 boxes checked
            palette.GetHSV(ColorIdx, hsv); // Now go and get the hsv value for this ColorIdx
            for(i=0; i<Count; i++) {
                fireworkBursts[x * Count + i].Reset(startX, startY, false, Velocity, hsv, start);
            }
        }
    }

    for (i=0; i<(Count*Number_Explosions); i++) {
        if (fireworkBursts[i].startPeriod == curPeriod) {
            fireworkBursts[i]._bActive = true;
        }
        
        // ... active flakes:
        if (fireworkBursts[i]._bActive)
        {
            // Update position
            fireworkBursts[i]._x += fireworkBursts[i]._dx;
            fireworkBursts[i]._y += (-fireworkBursts[i]._dy - fireworkBursts[i]._cycles*fireworkBursts[i]._cycles/10000000.0);
            // If this flake run for more than maxCycle or this flake is out of bounds, time to switch it off
            fireworkBursts[i]._cycles+=20;
            if (fireworkBursts[i]._cycles >= 10000 || fireworkBursts[i]._y >= BufferHt ||
                fireworkBursts[i]._x < 0. || fireworkBursts[i]._x >= BufferWi)
            {
                fireworkBursts[i]._bActive = false;
                continue;
            }
        }
        if(fireworkBursts[i]._bActive == true)
        {
            v = ((Fade*10.0)-fireworkBursts[i]._cycles)/(Fade*10.0);
            if(v<0) v=0.0;
            if (allowAlpha) {
                xlColor c(fireworkBursts[i]._hsv);
                c.alpha = 255.0 * v;
                SetPixel(fireworkBursts[i]._x, fireworkBursts[i]._y, c);
            } else {
                hsv=fireworkBursts[i]._hsv;
                hsv.value=v;
                SetPixel(fireworkBursts[i]._x, fireworkBursts[i]._y, hsv);
            }
        }
    }
}
