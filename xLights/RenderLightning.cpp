/***************************************************************
 * Name:      RenderLightning.cpp
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

#include <wx/textfile.h>


void RgbEffects::RenderLightning(int Number_Bolts, int Number_Segments, bool Use_All_Colors,
                                 int topX,int topY, int botX, int botY) {
    int colorcnt=GetColorCount();
    xlColor color;
    wxImage::HSVValue hsv;
    int x,y,x1,x2,y1,y2,xc,yc,step,ystep;
    int i,j,segment,StepSegment;
    wxString str;
    int curState = (curPeriod - curEffStartPer);



    ystep=BufferHt/10;
    xc=BufferWi/2;
    yc=BufferHt/2;

    i=0;
    /*
        if (curState==0) {
            needToInit = false;
            bolt.resize(50);
            for (BoltList::iterator it=bolt.begin(); it!=bolt.end(); ++it) {

                segment=i;
                it->segment = i;
                if(segment%2==0)
                    it->x1 = xc+i - (segment%step);
                else
                    it->x1 = xc+segment + (segment%step);

                it->y1 = BufferHt-segment;

                i++;
            }
        }
        */

    step=5;
    StepSegment=BufferHt/Number_Bolts;
    segment=curState%Number_Bolts;
    segment*=4;
    if(segment>Number_Bolts) segment=Number_Bolts;
    if(curState>Number_Bolts)  segment=Number_Bolts;
    palette.GetColor(0, color);
    x1=xc + topX;
    y1=BufferHt - topY;
    for(i=0; i<=segment; i++) {
        //0  x2=bolt[i].x1;
        //  y2=bolt[i].y1;
        j=rand()+1;
        if(i%2==0) {
            if(rand()%2==0)
                x2 = xc + topX - (j%Number_Segments);
            else  x2 = xc + topX + (2*j%Number_Segments);
        } else {
            if(rand()%2==0)
                x2 = xc + topX + (j%Number_Segments);
            else
                x2 = xc + topX - (3*j%Number_Segments);
        }
        y2 = BufferHt-(i*StepSegment) - topY;



        LightningDrawBolt(x1,y1,x2,y2,color,curState);

        x1=x2;
        y1=y2;
    }


}

void RgbEffects::LightningDrawBolt(const int x0_, const int y0_, const int x1_, const int y1_,  xlColor& color ,int curState) {
    int x0 = x0_;
    int x1 = x1_;
    int y0 = y0_;
    int y1 = y1_;
    xlColor color2;
    wxImage::HSVValue hsv = color.asHSV();
    wxImage::HSVValue hsv2 = color2.asHSV();
//   if(x0<0 || x1<0 || y1<0 || y0<0) return;

    int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
    int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1;
    int err = (dx>dy ? dx : -dy)/2, e2;
    color2=color;
    color2.red=color2.green=color2.blue=200;
    if(curState>40) {
        hsv.value = hsv.value - (1-(curState/80.0));
        if(hsv.value<0.0)
            hsv.value=0.0;
        hsv2.value = hsv2.value - (1-(curState/60.0));
        if(hsv2.value<0.0)
            hsv2.value=0.0;
        color = hsv;
        color2 = hsv2;
    }
    for(;;) {
        SetPixel(x0,y0, color);
        SetPixel(x0-1,y0, color2);
        SetPixel(x0+1,y0, color2);
        if (x0==x1 && y0==y1) break;
        e2 = err;
        if (e2 >-dx) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dy) {
            err += dx;
            y0 += sy;
        }
    }
}

