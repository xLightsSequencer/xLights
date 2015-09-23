/***************************************************************
 * Name:      RenderPlasma.cpp
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

/*
01) x*y^3-y*x^3
(02) (x^2+3*y^2)*e^(-x^2-y^2)
	(03) -x*y*e^(-x^2-y^2)
	(04) -1/(x^2+y^2)
	(05) cos(abs(x)+abs(y))
	(06) cos(abs(x)+abs(y))*(abs(x)+abs(y))
*/

void RgbEffects::RenderPlasma(int ColorScheme, int Style, int Chunks, int Skip, int ButterflyDirection, int butterFlySpeed)
{
    int x,y,d,xc,yc,x0,y0;
    double n,x1,y1,f;
    double h=0.0;
    double  fractpart, intpart;
    static const double pi2=6.283185307;
    xlColour color;
    wxImage::HSVValue hsv;
    int maxframe=BufferHt*2;

    int curState = (curPeriod - curEffStartPer) * butterFlySpeed * frameTimeInMs / 50;
    int frame=(BufferHt * curState / 200)%maxframe;
    double offset=double(curState)/200.0;


    if(ButterflyDirection==1) offset = -offset;
    xc=BufferWi/2;
    yc=BufferHt/2;
    for (x=0; x<BufferWi; x++)
    {
        for (y=0; y<BufferHt; y++)
        {

            GetMultiColorBlend(h,false,color);
            SetPixel(x,y,color);
        }
    }
}
