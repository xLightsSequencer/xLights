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
    int TailLength;
public:
    StrobeHasExpired(int t)
        : TailLength(t)
    {}

    // operator() is what's called when you do MeteorHasExpired()
    bool operator()(const StrobeClass& obj)
    {
        return obj.x + TailLength < 0;
    }
};

void RgbEffects::RenderStrobe(int Number_Strobes, int StrobeDuration)
{
    if (state == 0) strobe.clear();
    int mspeed=state/4;
    state-=mspeed*4;


    StrobeClass m;
    wxImage::HSVValue hsv,hsv0,hsv1;
    palette.GetHSV(0,hsv0);
    palette.GetHSV(1,hsv1);
    size_t colorcnt=GetColorCount();

    // create new strobe, randomly place a strobe

    for(int i=0; i<BufferHt; i++)
    {
        if (rand() % 200 < Number_Strobes)
        {
            m.x=BufferWi - 1;
            m.y=i;

            // Now store how many frames this lights will stay on for
            m.duration = StrobeDuration;
            palette.GetHSV(0, m.hsv); // take first checked color as color of flash

            strobe.push_back(m); // Store this strobe into the list
        }
    }

    // render strobe, we go through all storbes and decide if they should be turned on

    int x,y,dy,n=0;
    for (StrobeList::iterator it=strobe.begin(); it!=strobe.end(); ++it)
    {
        n++;

        hsv=it->hsv;
       // x=it->x+ph;
//y=it->y+dy;
        if(it->duration>0)
            SetPixel(x,y,hsv);

        it->duration--;  // decreas the frame counter on this strobe, when it gets to zero we no longer will turn it on


        // delete old strobe
        if(it->duration<1)
            strobe.remove_if(StrobeHasExpired(it->duration));
    }
}
