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


void RgbEffects::RenderMeteors(int MeteorType, int Count, int Length,bool FallUp,int MeteorsEffect,int SwirlIntensity)
{
    if (state == 0) meteors.clear();
    int mspeed=state/4;
    state-=mspeed*4;

    int swirl=0,Polar=0,meteor=0;
    if(MeteorsEffect==0) meteor=1;
    if(MeteorsEffect==1) swirl=1;
    if(MeteorsEffect==2) swirl=2;
    if(MeteorsEffect==3) Polar=1;
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


    double dx,n=0;
    int ph_sign,MeteorHeadType=0;
    double x,y,x1,y1,r,phi,xp,yp;
    for (MeteorList::iterator it=meteors.begin(); it!=meteors.end(); ++it)
    {
        n++;
        for(int ph=0; ph<=TailLength; ph++)
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

            if(FallUp)
                hsv.value*=double(ph)/TailLength;
            else
                hsv.value*=1.0 - double(ph)/TailLength;
            double i=(it->y)/5.0+(n/100.0);

            dx-0.0;
            if(swirl==1)  dx=SwirlIntensity*sin(i);
            if(swirl==2)
            {
                dx=SwirlIntensity*sin(i);
                if ( (int)(it->x)%2==0 )  dx=-SwirlIntensity*sin(i); // we adjust x axis with some sine function if swirl1 or swirl2
            }
            ph_sign=1;
            if(FallUp) ph_sign=-1; // we can make everything fall up
            x=it->x+dx;
            y=it->y+(ph_sign*ph);
            if(Polar)
            {
                if(BufferHt>0) x1 = x-BufferHt; // convert x1 to be 0 to 1.0. This will be used to calculate radius for polar coordinates
                if(BufferWi>0) y1 = y-BufferWi;
                r = sqrt(x1*x1 + y1*y1);
                if(Count>0) phi = 6.28 *  n/(double) (Count);
                else phi=6.28;
                xp = r * cos(phi) + BufferWi/2;
                yp = r * sin(phi) + BufferHt/2;
                SetPixel(xp,yp,hsv);
            }
            else
            {
                SetPixel(it->x+dx,it->y+ph,hsv);
                //  BufferWi, BufferHt
                if(ph == 0) // if are are head of meteor, decide if we will draw with something other than a point
                {
                    //   hsv.hue=double(rand() % 1000) / 1000.0;
                    switch (MeteorHeadType)
                        // 1=plus 1 pixel, 2= plus 2 pixel, 3 = cross
                    {
                    case 1: // plus 1 pixel
                        SetPixel(x+dx+1,y,hsv);
                        SetPixel(x+dx-1,y,hsv);
                        SetPixel(x+dx,y+1,hsv);
                        SetPixel(x+dx,y-1,hsv);
                        break;
                    case 2: // plus 2 pixel
                        SetPixel(x+dx+1,y,hsv);
                        SetPixel(x+dx-1,y,hsv);
                        SetPixel(x+dx,y+1,hsv);
                        SetPixel(x+dx,y-1,hsv);

                        SetPixel(x+dx+2,y,hsv);
                        SetPixel(x+dx-2,y,hsv);
                        SetPixel(x+dx,y+2,hsv);
                        SetPixel(x+dx,y-2,hsv);
                        break;
                    case 3: // cross 1 pixel
                        SetPixel(x+dx+1,y+1,hsv);
                        SetPixel(x+dx-1,y-1,hsv);
                        SetPixel(x+dx-1,y+1,hsv);
                        SetPixel(x+dx+1,y-1,hsv);
                        break;
                    }
                }
            }
        }

        if(FallUp)
            it->y += mspeed;
        else
            it->y -= mspeed;
    }

    // delete old meteors
    meteors.remove_if(MeteorHasExpired(TailLength));
}
