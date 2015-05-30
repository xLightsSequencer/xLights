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

#define numents(thing)  (sizeof(thing) / sizeof(thing[0]))

//these must match list indexes in xLightsMain.h: -DJ
#define METEORS_DOWN  0
#define METEORS_UP  1
#define METEORS_LEFT  2
#define METEORS_RIGHT  3
#define METEORS_IMPLODE  4
#define METEORS_EXPLODE  5
#define METEORS_ICICLES  6 //random length drip effect -DJ
#define METEORS_ICICLES_BKG  7 //with bkg (dim) icicles -DJ


// ColorScheme: 0=rainbow, 1=range, 2=palette
// MeteorsEffect: 0=down, 1=up, 2=left, 3=right, 4=implode, 5=explode
void RgbEffects::RenderMeteors(int ColorScheme, int Count, int Length, int MeteorsEffect, int SwirlIntensity, int mSpeed)
{
    if (needToInit) {
        needToInit = false;
        meteors.clear();
        meteorsRadial.clear();
        effectState = mSpeed * frameTimeInMs / 50;
    } else {
        effectState += mSpeed * frameTimeInMs / 50;
    }
    //wierd calculation, but adds a slight abmount of jitter to the speed
    //example: if mSpeed is 10, the speeds for the first few frames would be:  2, 3, 2...
    int mspeed = effectState / 4;
    effectState -= mspeed * 4;
    
    switch (MeteorsEffect) {
        case METEORS_DOWN: //0:
        case METEORS_UP: //1:
            RenderMeteorsVertical(ColorScheme, Count, Length, MeteorsEffect, SwirlIntensity, mspeed);
            break;
        case METEORS_LEFT: //2:
        case METEORS_RIGHT: //3:
            RenderMeteorsHorizontal(ColorScheme, Count, Length, MeteorsEffect, SwirlIntensity, mspeed);
            break;
        case METEORS_IMPLODE: //4:
            RenderMeteorsImplode(ColorScheme, Count, Length, SwirlIntensity, mspeed);
            break;
        case METEORS_EXPLODE: //5:
            RenderMeteorsExplode(ColorScheme, Count, Length, SwirlIntensity, mspeed);
            break;
        case METEORS_ICICLES: //6
            RenderIcicleDrip(ColorScheme, Count, Length, MeteorsEffect, SwirlIntensity, mspeed);
            break;
        case METEORS_ICICLES_BKG: //7
            RenderIcicleDrip(ColorScheme, Count, -Length, MeteorsEffect, SwirlIntensity, mspeed);
            break;
    }
}

/*
 * *************************************************************
 *  Horizontal
 * *************************************************************
 */

class RgbEffects::MeteorHasExpiredX
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

void RgbEffects::RenderMeteorsHorizontal(int ColorScheme, int Count, int Length, int MeteorsEffect, int SwirlIntensity, int mspeed)
{
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

            swirl_phase=double(it->x)/5.0+double(n)/100.0;
            dy=int(double(SwirlIntensity*BufferHt)/80.0*sin(swirl_phase));

            x=it->x+ph;
            y=it->y+dy;
            if (MeteorsEffect==3) x=BufferWi-x;
            
            if (allowAlpha) {
                xlColor c(hsv);
                c.alpha = 255.0 * (1.0 - double(ph)/TailLength);
                SetPixel(x,y,c);
            } else {
                hsv.value*= 1.0 - double(ph)/TailLength;
                SetPixel(x,y,hsv);
            }
        }

        it->x -= mspeed;
    }

    // delete old meteors
    meteors.remove_if(MeteorHasExpiredX(TailLength));
}

/*
 * *************************************************************
 *  Vertical
 * *************************************************************
 */

class RgbEffects::MeteorHasExpiredY
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

//predicate to remove variable length meteors (icicles):
class RgbEffects::IcicleHasExpired
{
public:
    bool operator()(const MeteorClass& obj) { return obj.y < -obj.h; }
};
//bool end_of_icicle(const MeteorClass& obj) { return obj.y > obj.h; }


void RgbEffects::RenderMeteorsVertical(int ColorScheme, int Count, int Length, int MeteorsEffect, int SwirlIntensity, int mspeed)
{
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

            // we adjust x axis with some sine function if swirl1 or swirl2
            // swirling more than 25% of the buffer width doesn't look good
            swirl_phase=double(it->y)/5.0+double(n)/100.0;
            dx=int(double(SwirlIntensity*BufferWi)/80.0*sin(swirl_phase));
            x=it->x+dx;
            y=it->y+ph;
            if (MeteorsEffect==1) y=BufferHt-y;

            if (allowAlpha) {
                xlColor c(hsv);
                c.alpha = 255.0 * (1.0 - double(ph)/TailLength);
                SetPixel(x,y,c);
            } else {
                hsv.value*= 1.0 - double(ph)/TailLength;
                SetPixel(x,y,hsv);
            }
        }

        it->y -= mspeed;
    }

    // delete old meteors
    meteors.remove_if(MeteorHasExpiredY(TailLength));
}

//#define WANT_DEBUG 100
////#define WANT_DEBUG_IMPL 100
//#include "djdebug.cpp"

//icicle drip effect, based on RenderMeteorsVertical: -DJ
void RgbEffects::RenderIcicleDrip(int ColorScheme, int Count, int Length, int MeteorsEffect, int SwirlIntensity, int mspeed)
{
    double swirl_phase;
    bool want_bkg = (Length < 0);
    if (want_bkg) Length = -Length; //kludge; too lazy to add another parameter ;)

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
//            m.h = TailLength;
            m.h = (rand() % (2 * BufferHt))/3; //somewhat variable length -DJ

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

//draw some dim icicles for bkg:
    if (want_bkg)
    {
        xlColour c(100, 100, 255); //light blue
        Color2HSV(c, m.hsv);
//        m.hue = 240;
//        m.hsv.saturation = 0.5;
//        m.hsv.value = 1.0;

        int ystaggered[] = {0, 5, 1, 2, 4};
        for (int x = 0; x < BufferWi; x += 3)
            for (int y = 0; y < BufferHt; y += 3)
                SetPixel(x, y + ystaggered[(x/3) % numents(ystaggered)], m.hsv);
    }

    int x,y,dx,n=0;
    for (MeteorList::iterator it=meteors.begin(); it!=meteors.end(); ++it)
    {
        n++;
        for(int ph=0; ph<=TailLength; ph++)
        {
#if 0
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
#endif
            if (!ph || (ph <= it->h - it->y)) hsv = it->hsv; //only make the end of the drip colored
            else { hsv.value = .4; hsv.hue = hsv.saturation = 0; } //white icicle

            // we adjust x axis with some sine function if swirl1 or swirl2
            // swirling more than 25% of the buffer width doesn't look good
            swirl_phase=double(it->y)/5.0+double(n)/100.0;
            dx=int(double(SwirlIntensity*BufferWi)/80.0*sin(swirl_phase));

            x=it->x+dx;
            y=it->y+ph;
            if (MeteorsEffect==1) y=BufferHt-y;
            if (y < it->h) continue; //variable length icicle drips -DJ
            SetPixel(x,y,hsv);
        }

        it->y -= mspeed;
    }

    // delete old meteors
//    meteors.remove_if(MeteorHasExpiredY(TailLength));
    meteors.remove_if(IcicleHasExpired());
}

/*
 * *************************************************************
 *  Implode
 * *************************************************************
 */

class RgbEffects::MeteorHasExpiredImplode
{
    int cx, cy;
public:
    MeteorHasExpiredImplode(int centerX, int centerY)
    { cx=centerX; cy=centerY; }

    // operator() is what's called when you do MeteorHasExpired()
    bool operator()(const MeteorRadialClass& obj)
    {
        return (std::abs(obj.y - cy) < 2) && (std::abs(obj.x - cx) < 2);
    }
};

void RgbEffects::RenderMeteorsImplode(int ColorScheme, int Count, int Length, int SwirlIntensity, int mspeed)
{
    double angle;
    int halfdiag=DiagLen/2; // 1/2 the length of the diagonal
    int centerX=BufferWi/2;
    int centerY=BufferHt/2;

    MeteorRadialClass m;
    wxImage::HSVValue hsv,hsv0,hsv1;
    palette.GetHSV(0,hsv0);
    palette.GetHSV(1,hsv1);
    size_t colorcnt=GetColorCount();
    int TailLength=(halfdiag < 10) ? Length / 10 : halfdiag * Length / 100;
    if (TailLength < 1) TailLength=1;
    int MinDimension = BufferHt < BufferWi ? BufferHt : BufferWi;

    // create new meteors

    m.cnt=1;
    for(int i=0; i<MinDimension; i++)
    {
        if (rand() % 200 < Count) {
            if (BufferHt==1) {
                angle=double(rand() % 2) * M_PI;
            } else if (BufferWi==1) {
                angle=double(rand() % 2) * M_PI - (M_PI/2.0);
            } else {
                angle=rand01()*2.0*M_PI;
            }
            m.dx=cos(angle);
            m.dy=sin(angle);
            m.x=centerX+double(halfdiag+TailLength)*m.dx;
            m.y=centerY+double(halfdiag+TailLength)*m.dy;

            switch (ColorScheme)
            {
            case 1:
                SetRangeColor(hsv0,hsv1,m.hsv);
                break;
            case 2:
                palette.GetHSV(rand()%colorcnt, m.hsv);
                break;
            }
            meteorsRadial.push_back(m);
        }
    }

    // render meteors

    int x,y,n=0;
    for (MeteorRadialList::iterator it=meteorsRadial.begin(); it!=meteorsRadial.end(); ++it)
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
            // if we were to swirl, it would need to alter the angle here

            x=int(it->x-it->dx*double(ph));
            y=int(it->y-it->dy*double(ph));

            // the next line cannot test for exact center! Some lines miss by 1 because of rounding.
            if ((abs(y - centerY) < 2) && (abs(x - centerX) < 2)) break;

            if (allowAlpha) {
                xlColor c(hsv);
                c.alpha = 255.0 * (double(ph)/TailLength);
                SetPixel(x,y,c);
            } else {
                hsv.value*= double(ph)/TailLength;
                SetPixel(x,y,hsv);
            }
        }

        it->x -= it->dx*mspeed;
        it->y -= it->dy*mspeed;
        it->cnt++;
    }

    // delete old meteors
    meteorsRadial.remove_if(MeteorHasExpiredImplode(BufferWi/2,BufferHt/2));
}

/*
 * *************************************************************
 *  Explode
 * *************************************************************
 */

class RgbEffects::MeteorHasExpiredExplode
{
    int ht, wi;
public:
    MeteorHasExpiredExplode(int h, int w)
    { ht=h; wi=w; }

    // operator() is what's called when you do MeteorHasExpired()
    bool operator()(const MeteorRadialClass& obj)
    {
        return obj.y < 0 || obj.x < 0 || obj.y > ht || obj.x > wi;
    }
};

void RgbEffects::RenderMeteorsExplode(int ColorScheme, int Count, int Length, int SwirlIntensity, int mspeed)
{
    double angle;
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

    m.x=BufferWi/2;
    m.y=BufferHt/2;
    m.cnt=1;
    for(int i=0; i<MinDimension; i++)
    {
        if (rand() % 200 < Count) {
            if (BufferHt==1) {
                angle=double(rand() % 2) * M_PI;
            } else if (BufferWi==1) {
                angle=double(rand() % 2) * M_PI - (M_PI/2.0);
            } else {
                angle=rand01()*2.0*M_PI;
            }
            m.dx=cos(angle);
            m.dy=sin(angle);
            
            switch (ColorScheme)
            {
            case 1:
                SetRangeColor(hsv0,hsv1,m.hsv);
                break;
            case 2:
                palette.GetHSV(rand()%colorcnt, m.hsv);
                break;
            }
            meteorsRadial.push_back(m);
        }
    }

    // render meteors

    int x,y,n=0;
    for (MeteorRadialList::iterator it=meteorsRadial.begin(); it!=meteorsRadial.end(); ++it)
    {
        n++;
        for(int ph=0; ph<=TailLength; ph++)
        {
            //if (ph >= it->cnt) continue;
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

            // if we were to swirl, it would need to alter the angle here

            x=int(it->x+it->dx*double(ph));
            y=int(it->y+it->dy*double(ph));
            
            
            if (allowAlpha) {
                xlColor c(hsv);
                c.alpha = 255.0 * (double(ph)/TailLength);
                SetPixel(x,y,c);
            } else {
                hsv.value*= double(ph)/TailLength;
                SetPixel(x,y,hsv);
            }
        }

        it->x += it->dx*mspeed;
        it->y += it->dy*mspeed;
        it->cnt++;
    }

    // delete old meteors
    meteorsRadial.remove_if(MeteorHasExpiredExplode(BufferHt,BufferWi));
}

