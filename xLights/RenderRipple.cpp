/***************************************************************
 * Name:      RenderRipple.cpp
 * Purpose:  Sean Meighan (sean@meighan.net)
 * Created:   2012-12-23
 * Copyright: 2012 by Sean Meighan
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
#include <wx/utils.h>
#include "djdebug.cpp"

#define WANT_DEBUG_IMPL
#define WANT_DEBUG  -99 //unbuffered in case app crashes

#ifndef debug_function //dummy defs if debug cpp not included above
#define debug(level, ...)
#define debug_more(level, ...)
#define debug_function(level)
#endif

//cut down on mem allocs outside debug() when WANT_DEBUG is off:
#ifdef WANT_DEBUG
#define IFDEBUG(stmt)  stmt
#else
#define IFDEBUG(stmt)
#endif // WANT_DEBUG


#define RENDER_RIPPLE_CIRCLE     0
#define RENDER_RIPPLE_SQUARE     1
#define RENDER_RIPPLE_TRIANGLE   2

#define MOVEMENT_EXPLODE    0
#define MOVEMENT_IMPLODE    1



void RgbEffects::RenderRipple(int Object_To_Draw, int Movement)
{

    int x,y,i,i7,ColorIdx;
    int xc,yc;

#if 0
    if(step<1) step=1;
    if(Use_All_Colors) srand (time(NULL)); // for Use_All_Colors effect, make lights be random
    else srand(1); // else always have the same random numbers for each frame (state)
#endif

    wxImage::HSVValue hsv; //   we will define an hsv color model. The RGB colot model would have been "wxColour color;"
    srand (time(NULL));
    size_t colorcnt=GetColorCount();

    i=0;
    double position = GetEffectTimeIntervalPosition(); // how far are we into the row> value is 0.0 to 1.0
    float rx;
    xc = BufferWi/2;
    yc=BufferHt/2;
    int on_off=0;

    int slices=200;
    int istate=state/slices; // istate will be a counter every slices units of state. each istate is a square wave
    int imod=(state/(slices/10))%10; // divide this square
    int icolor=istate%colorcnt;
    wxString TimeNow =wxNow();
    rx=(state%slices)/(slices*1.0);

    int x1 = xc - (xc*rx);
    int x2 = xc + (xc*rx);
    int y1 = yc - (yc*rx);
    int y2 = yc + (yc*rx);
    enum {Square, Circle, Triangle} shape = Circle;
    double radius;
    //  debug(10, "%s:%6d istate=%4d imod=%4d icolor=%1d", (const char*)TimeNow,state,istate,imod,icolor);
    ColorIdx=rand()% colorcnt; // Select random numbers from 0 up to number of colors the user has checked. 0-5 if 6 boxes checked
    palette.GetHSV(ColorIdx, hsv); // Now go and get the hsv value for this ColorIdx
    int explode;
    switch (Object_To_Draw)
    {

    case RENDER_RIPPLE_SQUARE:
        explode=1;
        if(Movement==MOVEMENT_EXPLODE)
        {
            // This is the object expanding out, or explode looikng
            int x1 = xc - (xc*rx);
            int x2 = xc + (xc*rx);
            int y1 = yc - (yc*rx);
            int y2 = yc + (yc*rx);
            for(y=y1; y<=y2; y++)
            {
                SetP(x1,y,hsv); // Turn pixel
                SetP(x2,y,hsv); // Turn pixel
            }
            for(x=x1; x<=x2; x++)
            {
                SetP(x,y1,hsv); // Turn pixel
                SetP(x,y2,hsv); // Turn pixel
            }

            hsv.value = (hsv.value /3)*2;
            for(y=y1; y<=y2; y++)
            {
                SetP(x1+1,y,hsv); // Turn pixel
                SetP(x2-1,y,hsv); // Turn pixel
            }
            for(x=x1; x<=x2; x++)
            {
                SetP(x,y1+1,hsv); // Turn pixel
                SetP(x,y2-1,hsv); // Turn pixel
            }

            hsv.value = hsv.value /3;
            for(y=y1; y<=y2; y++)
            {
                SetP(x1+2,y,hsv); // Turn pixel
                SetP(x2-2,y,hsv); // Turn pixel
            }
            for(x=x1; x<=x2; x++)
            {
                SetP(x,y1+2,hsv); // Turn pixel
                SetP(x,y2-2,hsv); // Turn pixel
            }


        }
        else if(Movement==MOVEMENT_IMPLODE)
        {
            int x1 = (xc*rx);
            int x2 = BufferWi - (xc*rx);
            int y1 =  (yc*rx);
            int y2 = BufferHt - (yc*rx);
            for(y=y2; y>=y1; y--)
            {
                SetP(x1,y,hsv); // Turn pixel
                SetP(x2,y,hsv); // Turn pixel
            }
            for(x=x2; x>=x1; x--)
            {
                SetP(x,y1,hsv); // Turn pixel
                SetP(x,y2,hsv); // Turn pixel
            }
        }
        break;
    case RENDER_RIPPLE_CIRCLE:
        if(Movement==MOVEMENT_IMPLODE)
            radius = xc-(xc*rx);
        else
            radius = (xc*rx);

        Drawcircle( xc, yc, radius, hsv);
        radius=radius/2;
        Drawcircle( xc, yc, radius, hsv);
        radius=radius/2;
        Drawcircle( xc, yc, radius, hsv);
        radius=radius/2;
        Drawcircle( xc, yc, radius, hsv);
        break;
    case RENDER_RIPPLE_TRIANGLE:
        break;
    }
}

void RgbEffects::Drawcircle(int xc,int yc,double radius,wxImage::HSVValue hsv)
{
    /*
    double 	wxDegToRad (double deg)
    Convert degrees to radians.

    double 	wxRadToDeg (double rad)
    Convert radians to degrees.

    Inside of #include <math.h> is this code, M_PI is wxwidgets definition of Pi

    #ifndef M_PI
    #define M_PI 3.1415926535897932384626433832795
    #endif

    t = (i+mod1440)*M_PI/180;
    x = (R-r) * cos (t) + d*cos (((R-r)/r)*t) + xc;
    y = (R-r) * sin (t) + d*sin (((R-r)/r)*t) + yc;

    if(colorcnt>0) d_mod = (int) BufferWi/colorcnt;
    else d_mod=1;

    x2= pow ((double)(x-xc),2);
    y2= pow ((double)(y-yc),2);
    hyp = (sqrt(x2 + y2)/BufferWi) * 100.0;
    */
    double degrees,radian;
    int x,y;
    for (degrees=0.0; degrees<360.0; degrees+=1.0)
    {
        radian = 	degrees * (M_PI/180.0);
        x = radius * cos(radian) + xc;
        y = radius * sin(radian) + yc;
        SetP(x,y,hsv); // Turn pixel
    }

}

void RgbEffects::SetP(int x, int y, wxImage::HSVValue hsv)
{
    if(x>=0 && x<BufferWi and y>=0 && y<BufferHt)
        SetPixel(x,y,hsv);
}
