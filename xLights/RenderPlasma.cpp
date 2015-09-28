/***************************************************************
 * Name:      RenderPlasma.cpp
 * Purpose:   Implements Plasma Effect
 * Author:    Sean Meighan (sean@meighan.net)
 * Created:   2015-09-24
 * Copyright: 2015 by Sean Meighan
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


#define PLASMA_NORMAL_COLORS    0
#define PLASMA_PRESET1          1
#define PLASMA_PRESET2          2
#define PLASMA_PRESET3          3
#define PLASMA_PRESET4          4

void RgbEffects::RenderPlasma(int ColorScheme, int Style, int Line_Density,int PlasmaDirection, int PlasmaSpeed)
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

    int curState = (curPeriod - curEffStartPer) * PlasmaSpeed * frameTimeInMs / 50;
    int frame=(BufferHt * curState / 200)%maxframe;
    double offset=double(curState)/200.0;


    if(PlasmaDirection==1) offset = -offset;
    xc=BufferWi/2;
    yc=BufferHt/2;
    // PlasmaSpeed=50;
    //  Line_Density=1;
    for (x=0; x<BufferWi; x++)
    {
        for (y=0; y<BufferHt; y++)
        {
            // reference: http://www.bidouille.org/prog/plasma

            state = (curPeriod - curEffStartPer); // frames 0 to N
            Speed_plasma = (101-PlasmaSpeed)*3; // we want a large number to divide by
            time = (state+1.0)/Speed_plasma;
            v=0;
            rx = ((float)x/(BufferWi-1)) ; // rx is now in the range 0.0 to 1.0
            ry = ((float)y/(BufferHt-1)) ;

            // 1st equation
            v=sin(rx*10+time);

//  second equation
            v+=sin(10*(rx*sin(time/2)+ry*cos(time/3))+time);

//  third equation
            cx=rx+.5*sin(time/5);
            cy=ry+.5*cos(time/3);
            v+=sin ( sqrt((Style*50)*((cx*cx)+(cy*cy))+time));


//    vec2 c = v_coords * u_k - u_k/2.0;
            v += sin(rx+time);
            v += sin((ry+time)/2.0);
            v += sin((rx+ry+time)/2.0);
//   c += u_k/2.0 * vec2(sin(u_time/3.0), cos(u_time/2.0));
            v += sin(sqrt(rx*rx+ry*ry)+time);
            v = v/2.0;
            // vec3 col = vec3(1, sin(PI*v), cos(PI*v));
//   gl_FragColor = vec4(col*.5 + .5, 1);


            switch (ColorScheme)
            {
            case PLASMA_NORMAL_COLORS:

                h = sin(v*Line_Density*pi+2*pi/3)+1*0.5;
                GetMultiColorBlend(h,false,color);
                hsv.hue=h;
                break;
            case PLASMA_PRESET1:
                color.red = (sin(v*Line_Density*pi)+1)*128;
                color.green= (cos(v*Line_Density*pi)+1)*128;
                color.blue =0;
                break;
            case PLASMA_PRESET2:
                color.red = 1;
                color.green= (cos(v*Line_Density*pi)+1)*128;
                color.blue =(sin(v*Line_Density*pi)+1)*128;
                break;

            case PLASMA_PRESET3:
                color.red = (sin(v*Line_Density*pi)+1)*128;
                color.green= (sin(v*Line_Density*pi + 2*pi/3)+1)*128;
                color.blue =(sin(v*Line_Density*pi+4*pi/3)+1)*128;
                break;
            case PLASMA_PRESET4:
                color.red=color.green=color.blue=(sin(v*Line_Density*pi) +1) * 128;
                break;

            }

            SetPixel(x,y,color);


        }
    }
}
