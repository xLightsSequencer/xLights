/***************************************************************
 * Name:      RenderRipple.cpp
 * Purpose:  Create radiating waves of circles or square
 *  Created by: Sean Meighan (sean@meighan.net)
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
#include <wx/utils.h>
#include "djdebug.cpp"
//#define WANT_DEBUG_IMPL
//#define WANT_DEBUG  -99 //unbuffered in case app crashes

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



void RgbEffects::RenderRipple(int Object_To_Draw, int Movement, int Ripple_Thickness,int CheckBox_Ripple3D)
{

    int i,ColorIdx;
    int xc,yc;

#if 0
    if(step<1) step=1;
    1) Normal - threaded
    2) Normal + Fast Save - threaded but only rendering the stuff that has changed
    3) Threading disabled (settings menu) - all rendering is on the main thread, one column at a time.
    4) Threading disabled + fast save - all rendering is on the main thread, one column at a time, but starting with the top left most changed cell.

    if(Use_All_Colors) srand (time(NULL)); // for Use_All_Colors effect, make lights be random
    else srand(1); // else always have the same random numbers for each frame (state)
#endif

        wxImage::HSVValue hsv; //   we will define an hsv color model. The RGB colot model would have been "wxColour color;"
        size_t colorcnt=GetColorCount();

        i=0;
        double position = GetEffectTimeIntervalPosition(); // how far are we into the row> value is 0.0 to 1.0
        int slices=200;
        if (position > 1)
        {
            position =(state%slices)/(slices*1.0);
            }
    float rx;
    xc = BufferWi/2;
    yc=BufferHt/2;

    if (fitToTime)
{
    rx=position;
}
else
{
    rx=(state%slices)/(slices*1.0);
    }
    ColorIdx=(int)(rx * colorcnt);
    if(ColorIdx==colorcnt) ColorIdx--; // ColorIdx goes from 0-3 when colorcnt goes from 1-4. Make sure that is true

    double radius;
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
                Drawsquare(Movement,  x1,  x2,  y1, y2, Ripple_Thickness, CheckBox_Ripple3D,hsv);
            }
            else if(Movement==MOVEMENT_IMPLODE)
            {
                int x1 = (xc*rx);
                int x2 = BufferWi - (xc*rx);
                int y1 =  (yc*rx);
                int y2 = BufferHt - (yc*rx);
                Drawsquare(Movement,  x1,  x2,  y1, y2, Ripple_Thickness, CheckBox_Ripple3D,hsv);
            }
            break;
        case RENDER_RIPPLE_CIRCLE:
            if(Movement==MOVEMENT_IMPLODE)
                radius = xc-(xc*rx);
            else
                radius = (xc*rx);


            Drawcircle( Movement,xc, yc, radius, hsv, Ripple_Thickness, CheckBox_Ripple3D);
            radius=radius/2;
            /*Drawcircle( Movement,xc, yc, radius, hsv, Ripple_Thickness, CheckBox_Ripple3D);
            radius=radius/2;
            Drawcircle( Movement,xc, yc, radius, hsv, Ripple_Thickness, CheckBox_Ripple3D);
            radius=radius/2;
            Drawcircle( Movement,xc, yc, radius, hsv, Ripple_Thickness, CheckBox_Ripple3D);
            */
            break;
        case RENDER_RIPPLE_TRIANGLE:
            break;
        }
}

void RgbEffects::Drawsquare(int Movement, int x1, int x2, int y1,int y2,int Ripple_Thickness,int CheckBox_Ripple3D,wxImage::HSVValue hsv)
{
    int i,x,y;
    xlColor color(hsv);

    for (i=0; i<Ripple_Thickness; i++)
    {
        if (CheckBox_Ripple3D) {
            hsv.value *= 1.0-((float(i)/2.0)/float(Ripple_Thickness)); // we multiply by 1.0 when Ripple_Thickness=0
            color = hsv;
        }
        if(Movement==MOVEMENT_EXPLODE)
        {
            for(y=y1+i; y<=y2-i; y++)
            {
                SetPixel(x1+i,y,color); // Turn pixel
                SetPixel(x2-i,y,color); // Turn pixel
            }
            for(x=x1+i; x<=x2-i; x++)
            {
                SetPixel(x,y1+i,color); // Turn pixel
                SetPixel(x,y2-i,color); // Turn pixel
            }
        }
        if(Movement==MOVEMENT_IMPLODE)
        {
            for(y=y2+i; y>=y1-i; y--)
            {
                SetPixel(x1-i,y,color); // Turn pixel
                SetPixel(x2+i,y,color); // Turn pixel
            }
            for(x=x2+i; x>=x1-i; x--)
            {
                SetPixel(x,y1-i,color); // Turn pixel
                SetPixel(x,y2+i,color); // Turn pixel
            }
        }
    }
}
void RgbEffects::Drawcircle(int Movement,int xc,int yc,double radius,wxImage::HSVValue hsv, int Ripple_Thickness,int CheckBox_Ripple3D)
{
    double degrees,radian;
    int x,y;
    float i;
    xlColor color(hsv);

    for (i=0; i<Ripple_Thickness; i+=.5)
    {
        if(CheckBox_Ripple3D) {
            hsv.value *= 1.0-(float(i)/float(Ripple_Thickness)); // we multiply by 1.0 when steps=0
            color = hsv;
        }
        if(Movement==MOVEMENT_EXPLODE)
        {
            radius = radius - i;
        }
        else
        {
            radius = radius + i;
        }
        for (degrees=0.0; degrees<360.0; degrees+=1.0)
        {
            radian = 	degrees * (M_PI/180.0);
            x = radius * cos(radian) + xc;
            y = radius * sin(radian) + yc;
            SetPixel(x,y,color); // Turn pixel
        }
    }

}

