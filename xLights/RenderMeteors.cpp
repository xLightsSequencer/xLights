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

void RgbEffects::RenderMeteors(int MeteorType, int Count, int Length,bool FallUp)
{
    if (state == 0) meteors.clear();
    int mspeed=state/4;
    state-=mspeed*4;

    // create new meteors
    MeteorClass m;
    wxImage::HSVValue hsv,hsv0,hsv1;
    palette.GetHSV(0,hsv0);
    palette.GetHSV(1,hsv1);
    size_t colorcnt=GetColorCount();
  //  bool FallUp=true;
    Count=BufferWi * Count / 100;
    int TailLength=(BufferHt < 10) ? Length / 10 : BufferHt * Length / 100;
    if (TailLength < 1) TailLength=1;
    int TailStart=BufferHt - TailLength;
    if (TailStart < 1) TailStart=1;
    for(int i=0; i<Count; i++) // go through Count of Meteors
    {
        m.x=rand() % BufferWi; // assign start x,y for each meteor head
        m.y=BufferHt - 1 - rand() % TailStart;

        switch (MeteorType)
        {
        case 1:
            SetRangeColor(hsv0,hsv1,m.hsv);
            break;
        case 2:
            palette.GetHSV(rand()%colorcnt, m.hsv);
            break;
        }
        meteors.push_back(m);
    }

    // render meteors
    for (MeteorList::iterator it=meteors.begin(); it!=meteors.end(); ++it)
    {
        for(int ph=0; ph<TailLength; ph++)
        {
            switch (MeteorType)
            {
            case 0:
                hsv.hue=double(rand() % 1000) / 1000.0;
                hsv.saturation=1.0;
                hsv.value=1.0;
                break;
            default:
                hsv=it->hsv;
                break;
            }
            hsv.value*=1.0 - double(ph)/TailLength;
            if(FallUp)
                SetPixel(it->x,it->y-ph,hsv);
            else
                SetPixel(it->x,it->y+ph,hsv);
        }
        if(FallUp)
            it->y += mspeed;
        else
            it->y -= mspeed;
    }

    // delete old meteors
    meteors.remove_if(MeteorHasExpired(TailLength));
}
