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

void RgbEffects::RenderPlasma(int ColorScheme, int Style, int Chunks,  int ButterflyDirection, int butterFlySpeed)
{
    int x,y,d,xc,yc,x0,y0;
    double n,x1,y1,f;
    double h=0.0;
    double  fractpart, intpart;
    static const double pi2=6.283185307;
    xlColour color;
    wxImage::HSVValue hsv;
    int maxframe=BufferHt*2;

    //  These are for Plasma effect
    double rx,ry,cx,cy,v,time,Speed_plasma,multiplier;
    static const double pi=3.1415926535897932384626433832;
    int i;
    int state;

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
            // reference: http://www.bidouille.org/prog/plasma

            state = (curPeriod - curEffStartPer); // frames 0 to N
            if(Style==10) // Style 10 is for custom colors on Plasma
                Speed_plasma = (101-butterFlySpeed)*3; // we want a large number to divide by
            else
                Speed_plasma = (101-butterFlySpeed)*5; // we want a large number to divide by

            time = (state+1.0)/Speed_plasma;

            v=0;

            rx = ((float)x/BufferWi) -0.5;
            ry = ((float)y/BufferHt) -0.5;



            // 1st equation
            v=sin(rx*10+time);

//  second equation
            v+=sin(10*(rx*sin(time/2)+ry*cos(time/3))+time);

//  third equation
            cx=rx+.5*sin(time/5);
            cy=ry+.5*cos(time/3);
            v+=sin ( sqrt(100*((cx*cx)+(cy*cy))+1+time));


//    vec2 c = v_coords * u_k - u_k/2.0;
            v += sin(rx+time);
            v += sin((ry+time)/2.0);
            v += sin((rx+ry+time)/2.0);
//   c += u_k/2.0 * vec2(sin(u_time/3.0), cos(u_time/2.0));
            v += sin(sqrt(rx*rx+ry*ry+1.0)+time);
            v = v/2.0;
            // vec3 col = vec3(1, sin(PI*v), cos(PI*v));
//   gl_FragColor = vec4(col*.5 + .5, 1);

            GetMultiColorBlend(h,false,color);
//color.red=color.green=color.blue=h*255;
            switch (Style)
            {
            case 6:
                color.red = (sin(v*Chunks*pi)+1)*128;
                color.green= (cos(v*Chunks*pi)+1)*128;
                color.blue =0;
                break;
            case 7:
                color.red = 1;
                color.green= (cos(v*Chunks*pi)+1)*128;
                color.blue =(sin(v*Chunks*pi)+1)*128;
                break;

            case 8:
                color.red = (sin(v*Chunks*pi)+1)*128;
                color.green= (sin(v*Chunks*pi + 2*pi/3)+1)*128;
                color.blue =(sin(v*Chunks*pi+4*pi/3)+1)*128;
                break;

            case 9:
                color.red=color.green=color.blue=(sin(v*Chunks*pi) +1) * 128;
                break;
            case 10:
                int colorcnt=2;
                if(colorcnt>=2)
                {
                    GetMultiColorBlend(h,false,color);

/*                    hue1=.1;
                    hue1=0;
                    hue2=.7;
                    hue2=1;
                    multiplier=(hue2-hue1)/2;
                    h=hue1+ multiplier*(v+1); // v is between -1 to 1. h
                    */
                    h = sin(v*Chunks*pi+2*pi/3)+1*0.5;

                    hsv.hue=h;
                    //  hsv.hue=hsv.hue + (v+1)/20.0;
//color.red = (sin(v*color.red)+1)*128;
//color.green = (sin(v*color.green)+1)*128;
//color.blue = (sin(v*color.blue)+1)*128;

                }
                break;
            }

            SetPixel(x,y,color);
        }
    }
}
