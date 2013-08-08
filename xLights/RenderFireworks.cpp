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


void RgbEffects::RenderFireworks(int Number_Explosions,int Count,float Velocity,int Fade)
{
    int idxFlakes=0;
    int i=0,x,y,mod100;
    int x25,x75,y25,y75,stateChunk,denom;
    const int maxFlakes = 1000;
    //float velocity = 3.5;
    int startX;
    int startY,ColorIdx;
    float v;
    wxImage::HSVValue hsv;
    wxColour color,rgbcolor;
    size_t colorcnt=GetColorCount();

    if(state==0)
        for(i=0; i<maxFlakes; i++)
        {
            fireworkBursts[i]._bActive = false;
        }
    denom = (101-Number_Explosions)*100;
    if(denom<1) denom=1;
    stateChunk = (int)state/denom;
    if(stateChunk<1) stateChunk=1;


    mod100 = state%((101-Number_Explosions)*20);
//        mod100 = (int)(state/stateChunk);
//        mod100 = mod100%10;
    if(mod100 == 0)
    {

        x25=(int)BufferWi*0.25;
        x75=(int)BufferWi*0.75;
        y25=(int)BufferHt*0.25;
        y75=(int)BufferHt*0.75;
        startX=(int)BufferWi/2;
        startY=(int)BufferHt/2;
        if((x75-x25)>0) startX = x25 + rand()%(x75-x25); else startX=0;
        if((y75-y25)>0) startY = y25 + rand()%(y75-y25); else startY=0;
        // turn off all bursts

        // Create new bursts
        ColorIdx=rand() % colorcnt; // Select random numbers from 0 up to number of colors the user has checked. 0-5 if 6 boxes checked
        palette.GetHSV(ColorIdx, hsv); // Now go and get the hsv value for this ColorIdx
        for(i=0; i<Count; i++)
        {
            do
            {
                idxFlakes = (idxFlakes + 1) % maxFlakes;
            }
            while (fireworkBursts[idxFlakes]._bActive);
            fireworkBursts[idxFlakes].Reset(startX, startY, true, Velocity, hsv);
        }
    }
    else if (mod100<10)
    {
        rgbcolor = wxColour(0,255,255);
          Color2HSV(color,hsv);
        y=(int)(startY-startY*(1.0/(mod100+1)));
        SetPixel(startX,y,hsv);
    }
    else
    {
        for (i=0; i<maxFlakes; i++)
        {
            // ... active flakes:
            if (fireworkBursts[i]._bActive)
            {
                // Update position
                fireworkBursts[i]._x += fireworkBursts[i]._dx;
                fireworkBursts[i]._y += (-fireworkBursts[i]._dy - fireworkBursts[i]._cycles*fireworkBursts[i]._cycles/10000000.0);
                // If this flake run for more than maxCycle, time to switch it off
                fireworkBursts[i]._cycles+=20;
                if (10000 == fireworkBursts[i]._cycles) // if (10000 == fireworkBursts[i]._cycles)
                {
                    fireworkBursts[i]._bActive = false;
                    continue;
                }
                // If this flake hit the earth, time to switch it off
                if (fireworkBursts[i]._y>=BufferHt)
                {
                    fireworkBursts[i]._bActive = false;
                    continue;
                }
                // Draw the flake, if its X-pos is within frame
                if (fireworkBursts[i]._x>=0. && fireworkBursts[i]._x<BufferWi)
                {
                    // But only if it is "under" the roof!
                    if (fireworkBursts[i]._y>=0.)
                    {
                        // sean we need to set color here
                    }
                }
                else
                {
                    // otherwise it just got outside the valid X-pos, so switch it off
                    fireworkBursts[i]._bActive = false;
                    continue;
                }
            }
        }
    }

    for(i=0; i < 1000; i++)
    {
        if(fireworkBursts[i]._bActive == true)
        {
            v = ((Fade*10.0)-fireworkBursts[i]._cycles)/(Fade*10.0);
            if(v<0) v=0.0;
            hsv=fireworkBursts[i]._hsv;
            hsv.value=v;
            SetPixel(fireworkBursts[i]._x, fireworkBursts[i]._y, hsv);
        }
    }
}
