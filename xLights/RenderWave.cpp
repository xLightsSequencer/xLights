/***************************************************************
 * Name:      RenderWave.cpp
 * Purpose:   Implements RGB effects
 * Author:    Sean Meighan (sean@meighan.net)
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



void RgbEffects::RenderWave(int WaveType)
{
    int x,y,d;
    double n,rx,ry,r;
    double h=0.0;
    static const double pi2=6.283185307;
    wxColour color;
    wxImage::HSVValue hsv,hsv0,hsv1;
    palette.GetHSV(0,hsv0);
    palette.GetHSV(1,hsv1);
    size_t colorcnt=GetColorCount();

    int maxframe=BufferHt*2;
    int frame=(BufferHt * state / 200)%maxframe;
    double offset=double(state)/200.0;
    r = BufferHt/2.0;
    for (x=0; x<BufferWi; x++)
    {
        rx=(x / (state%BufferWi)) * 3.1415926;;

        y = (int) (r * sin(rx) +r);

//        x=r * cos(t);
//        y=r * sin(t);
        if(x>=0 and x<BufferWi and y>=0 and y <BufferHt)
            SetPixel(x,y,hsv);

    }
}
