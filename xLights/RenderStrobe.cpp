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
    if (state == 0) {
        strobe.resize(StrobeDuration * Number_Strobes);
        for (int x = 0; x < strobe.size(); x++) {
            strobe[x].duration = -1;
        }
    }
    int offset = ((state / speed) % StrobeDuration) * Number_Strobes;

    int ColorIdx;
    StrobeClass m;
    wxImage::HSVValue hsv;
    size_t colorcnt=GetColorCount();

    // create new strobe, randomly place a strobe

    for(int i = 0; i < Number_Strobes; i++)
    {
        strobe[offset + i].x =rand() % BufferWi; // randomly pick a x,y location for strobe to fire
        strobe[offset + i].y =rand() % BufferHt;
        strobe[offset + i].duration = StrobeDuration;

        ColorIdx=rand()%colorcnt;
        palette.GetHSV(ColorIdx, strobe[offset + i].hsv); // take first checked color as color of flash
        palette.GetColor(ColorIdx, strobe[offset + i].color); // take first checked color as color of flash
    }

    // render strobe, we go through all storbes and decide if they should be turned on

    int x,y,n=0;
    
    for (StrobeList::iterator it=strobe.begin(); it!=strobe.end(); ++it) {
        n++;
        hsv=it->hsv;
        xlColor color(it->color);
        x=it->x;
        y=it->y;
        if(it->duration > 0)
        {
            SetPixel(x,y,color);
        }

        double v = 1.0;
        if(it->duration==1)
        {
            v = 0.5;
        }
        else if(it->duration==2)
        {
            v = 0.75;
        }
        if (allowAlpha) {
            color.alpha = 255.0 * v;
        } else {
            hsv.value *= v;
            color = hsv;
        }

        if(Strobe_Type==2)
        {
            int r = rand()%2;
            if(r==0)
            {
                SetPixel(x,y-1,color);
                SetPixel(x,y+1,color);
            }
            else
            {
                SetPixel(x-1,y,color);
                SetPixel(x+1,y,color);
            }

        }
        if(Strobe_Type==3)
        {
            SetPixel(x,y-1,color);
            SetPixel(x,y+1,color);
            SetPixel(x-1,y,color);
            SetPixel(x+1,y,color);
        }
        if(Strobe_Type==4)
        {
            int r = rand()%2;
            if(r==0)
            {
                SetPixel(x,y-1,color);
                SetPixel(x,y+1,color);
                SetPixel(x-1,y,color);
                SetPixel(x+1,y,color);
            }
            else
            {
                SetPixel(x+1,y-1,color);
                SetPixel(x+1,y+1,color);
                SetPixel(x-1,y-1,color);
                SetPixel(x-1,y+1,color);
            }
        }

        it->duration--;  // decrease the frame counter on this strobe, when it gets to zero we no longer will turn it on
    }
}
