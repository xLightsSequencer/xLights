/***************************************************************
 * Name:      RenderStrobe.cpp
 * Purpose:   Create lights that flash on and off similar to strobe lights
 * Author:    Sean Meighan (sean@meighan.net)
 * Created:   2014-09-01
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

#include <cmath>
#include "RgbEffects.h"

#define numents(thing)  (sizeof(thing) / sizeof(thing[0]))



class RgbEffects::StrobeHasExpired
{
public:
    StrobeHasExpired() {}

    // operator() is what's called when you do MeteorHasExpired()
    bool operator()(const StrobeClass& obj)
    {
        return obj.duration < 0;
    }
};

void RgbEffects::RenderStrobe(int Number_Strobes, int StrobeDuration,int Strobe_Type)
{
    if (state == 0) strobe.clear();
    int mspeed=state/4;
    state-=mspeed*4;

    int ColorIdx;
    StrobeClass m;
    wxImage::HSVValue hsv,hsv0,hsv1;
    palette.GetHSV(0,hsv0);
    palette.GetHSV(1,hsv1);
    size_t colorcnt=GetColorCount();

    // create new strobe, randomly place a strobe

    for(int i = 0; i < Number_Strobes; i++)
    {
        m.x =rand() % BufferWi; // randomly pick a x,y location for strobe to fire
        m.y =rand() % BufferHt;
        m.duration = StrobeDuration;

        ColorIdx=rand()%colorcnt;
        palette.GetHSV(ColorIdx, m.hsv); // take first checked color as color of flash

        strobe.push_back(m); // Store this strobe into the list
    }

    // render strobe, we go through all storbes and decide if they should be turned on

    int x,y,n=0;
    
    for (StrobeList::iterator it=strobe.begin(); it!=strobe.end(); ++it) {
        n++;
        hsv=it->hsv;
        x=it->x;
        y=it->y;
        if(it->duration > 0)
        {
            SetPixel(x,y,hsv);
        }

        if(it->duration==1)
        {
            hsv.value /=2;
        }
        else if(it->duration==2)
        {
            hsv.value /=1.5;
        }

        if(Strobe_Type==2)
        {
            int r = rand()%2;
            if(r==0)
            {
                SetPixel(x,y-1,hsv);
                SetPixel(x,y+1,hsv);
            }
            else
            {
                SetPixel(x-1,y,hsv);
                SetPixel(x+1,y,hsv);
            }

        }
        if(Strobe_Type==3)
        {
            SetPixel(x,y-1,hsv);
            SetPixel(x,y+1,hsv);
            SetPixel(x-1,y,hsv);
            SetPixel(x+1,y,hsv);
        }
        if(Strobe_Type==4)
        {
            int r = rand()%2;
            if(r==0)
            {
                SetPixel(x,y-1,hsv);
                SetPixel(x,y+1,hsv);
                SetPixel(x-1,y,hsv);
                SetPixel(x+1,y,hsv);
            }
            else
            {
                SetPixel(x+1,y-1,hsv);
                SetPixel(x+1,y+1,hsv);
                SetPixel(x-1,y-1,hsv);
                SetPixel(x-1,y+1,hsv);
            }
        }

        it->duration--;  // decrease the frame counter on this strobe, when it gets to zero we no longer will turn it on
    }
    // delete old strobes
    x = strobe.size();
    strobe.remove_if(StrobeHasExpired());
}
