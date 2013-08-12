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

class MeteorHasExpiredX
{
    int TailLength;
public:
    MeteorHasExpiredX(int t)
        : TailLength(t)
    {}

    // operator() is what's called when you do MeteorHasExpired()
    bool operator()(const MeteorClass& obj)
    {
        return obj.x + TailLength < 0;
    }
};

class MeteorHasExpiredY
{
    int TailLength;
public:
    MeteorHasExpiredY(int t)
        : TailLength(t)
    {}

    // operator() is what's called when you do MeteorHasExpired()
    bool operator()(const MeteorClass& obj)
    {
        return obj.y + TailLength < 0;
    }
};

// ColorScheme: 0=rainbow, 1=range, 2=palette
// MeteorsEffect: 0=fall down, 1=shoot up, 2=implode, 3=explode
void RgbEffects::RenderMeteors(int ColorScheme, int Count, int Length, int MeteorsEffect, int SwirlIntensity)
{
    if (MeteorsEffect < 2) {
        RenderMeteorsVertical(ColorScheme, Count, Length, MeteorsEffect, SwirlIntensity);
    } else if (MeteorsEffect < 4) {
        RenderMeteorsHorizontal(ColorScheme, Count, Length, MeteorsEffect, SwirlIntensity);
    } else {
        RenderMeteorsPolar(ColorScheme, Count, Length, MeteorsEffect, SwirlIntensity);
    }
}

void RgbEffects::RenderMeteorsHorizontal(int ColorScheme, int Count, int Length, int MeteorsEffect, int SwirlIntensity)
{
    if (state == 0) meteors.clear();
    int mspeed=state/4;
    state-=mspeed*4;
    double swirl_phase;

    MeteorClass m;
    wxImage::HSVValue hsv,hsv0,hsv1;
    palette.GetHSV(0,hsv0);
    palette.GetHSV(1,hsv1);
    size_t colorcnt=GetColorCount();
    int TailLength=(BufferWi < 10) ? Length / 10 : BufferWi * Length / 100;
    if (TailLength < 1) TailLength=1;

    // create new meteors

    for(int i=0; i<BufferHt; i++)
    {
        if (rand() % 200 < Count) {
            m.x=BufferWi - 1;
            m.y=i;

            switch (ColorScheme)
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
    }

    // render meteors

    int x,y,dy,n=0;
    int MeteorHeadType=0;
    //int x1,y1,r,phi,xp,yp;
    for (MeteorList::iterator it=meteors.begin(); it!=meteors.end(); ++it)
    {
        n++;
        for(int ph=0; ph<=TailLength; ph++)
        {
            switch (ColorScheme)
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
            hsv.value*= 1.0 - double(ph)/TailLength;

            swirl_phase=double(it->x)/5.0+double(n)/100.0;
            dy=int(double(SwirlIntensity*BufferHt)/80.0*sin(swirl_phase));

            x=it->x+ph;
            y=it->y+dy;
            if (MeteorsEffect==3) x=BufferWi-x;
            SetPixel(x,y,hsv);
        }

        it->x -= mspeed;
    }

    // delete old meteors
    meteors.remove_if(MeteorHasExpiredX(TailLength));
}

void RgbEffects::RenderMeteorsVertical(int ColorScheme, int Count, int Length, int MeteorsEffect, int SwirlIntensity)
{
    if (state == 0) meteors.clear();
    int mspeed=state/4;
    state-=mspeed*4;
    double swirl_phase;

    MeteorClass m;
    wxImage::HSVValue hsv,hsv0,hsv1;
    palette.GetHSV(0,hsv0);
    palette.GetHSV(1,hsv1);
    size_t colorcnt=GetColorCount();
    int TailLength=(BufferHt < 10) ? Length / 10 : BufferHt * Length / 100;
    if (TailLength < 1) TailLength=1;

    // create new meteors

    for(int i=0; i<BufferWi; i++)
    {
        if (rand() % 200 < Count) {
            m.x=i;
            m.y=BufferHt - 1;

            switch (ColorScheme)
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
    }

    // render meteors

    int x,y,dx,n=0;
    int MeteorHeadType=0;
    //int x1,y1,r,phi,xp,yp;
    for (MeteorList::iterator it=meteors.begin(); it!=meteors.end(); ++it)
    {
        n++;
        for(int ph=0; ph<=TailLength; ph++)
        {
            switch (ColorScheme)
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
            hsv.value*= 1.0 - double(ph)/TailLength;

            // we adjust x axis with some sine function if swirl1 or swirl2
            // swirling more than 25% of the buffer width doesn't look good
            swirl_phase=double(it->y)/5.0+double(n)/100.0;
            dx=int(double(SwirlIntensity*BufferWi)/80.0*sin(swirl_phase));

            x=it->x+dx;
            y=it->y+ph;
            if (MeteorsEffect==1) y=BufferHt-y;
            SetPixel(x,y,hsv);
        }

        it->y -= mspeed;
    }

    // delete old meteors
    meteors.remove_if(MeteorHasExpiredY(TailLength));
}

void RgbEffects::RenderMeteorsPolar(int ColorScheme, int Count, int Length, int MeteorsEffect, int SwirlIntensity)
{
    if (state == 0) meteorsRadial.clear();
    int mspeed=state/4;
    state-=mspeed*4;
    double swirl_phase,angle;
    int halfdiag=DiagLen/2; // 1/2 the length of the diagonal

    MeteorRadialClass m;
    wxImage::HSVValue hsv,hsv0,hsv1;
    palette.GetHSV(0,hsv0);
    palette.GetHSV(1,hsv1);
    size_t colorcnt=GetColorCount();
    int TailLength=(halfdiag < 10) ? Length / 10 : halfdiag * Length / 100;
    if (TailLength < 1) TailLength=1;
    int MinDimension = BufferHt < BufferWi ? BufferHt : BufferWi;

    // create new meteors

    for(int i=0; i<MinDimension; i++)
    {
        if (rand() % 200 < Count) {
            angle=rand01()*2.0*M_PI;
            m.dx=cos(angle);
            m.dy=sin(angle);
            m.x=i;
            m.y=BufferHt - 1;

            switch (ColorScheme)
            {
            case 1:
                SetRangeColor(hsv0,hsv1,m.hsv);
                break;
            case 2:
                palette.GetHSV(rand()%colorcnt, m.hsv);
                break;
            }
            //meteorsRadial.push_back(m);
        }
    }

    // render meteors

    int x,y,dx,n=0;
    int MeteorHeadType=0;
}


                /*
                if(ph == 0)
                {
                    // we are at the head of meteor, decide if we will draw with something other than a point
                    // 1=plus 1 pixel, 2= plus 2 pixel, 3 = cross
                    switch (MeteorHeadType)
                    {
                    case 1: // plus 1 pixel
                        SetPixel(x+1,y,hsv);
                        SetPixel(x-1,y,hsv);
                        SetPixel(x,y+1,hsv);
                        SetPixel(x,y-1,hsv);
                        break;
                    case 2: // plus 2 pixel
                        SetPixel(x+1,y,hsv);
                        SetPixel(x-1,y,hsv);
                        SetPixel(x,y+1,hsv);
                        SetPixel(x,y-1,hsv);

                        SetPixel(x+2,y,hsv);
                        SetPixel(x-2,y,hsv);
                        SetPixel(x,y+2,hsv);
                        SetPixel(x,y-2,hsv);
                        break;
                    case 3: // cross 1 pixel
                        SetPixel(x+1,y+1,hsv);
                        SetPixel(x-1,y-1,hsv);
                        SetPixel(x-1,y+1,hsv);
                        SetPixel(x+1,y-1,hsv);
                        break;
                    }
                }
                */
