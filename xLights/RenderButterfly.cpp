/***************************************************************
 * Name:      RenderButterfly.cpp
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

void RgbEffects::RenderButterfly(int ColorScheme, int Style, int Chunks, int Skip, int ButterflyDirection)
{
    int x,y,d;
    double n,x1,y1,f;
    double h=0.0;
    static const double pi2=6.283185307;
    wxColour color;
    wxImage::HSVValue hsv;
    int maxframe=BufferHt*2;
    int frame=(BufferHt * state / 200)%maxframe;
    double offset=double(state)/200.0;
    if(ButterflyDirection==1) offset = -offset;
    for (x=0; x<BufferWi; x++)
    {
        for (y=0; y<BufferHt; y++)
        {
            switch (Style)
            {
            case 1:

                n = abs((x*x - y*y) * sin (offset + ((x+y)*pi2 / (BufferHt+BufferWi))));
                d = x*x + y*y;
                if(d>0.001) h=n/d;
                else
                    h=0.0;
                break;
            case 2:
                f=(frame < maxframe/2) ? frame+1 : maxframe - frame;
                x1=(double(x)-BufferWi/2.0)/f;
                y1=(double(y)-BufferHt/2.0)/f;
                h=sqrt(x1*x1+y1*y1);
                break;
            case 3:
                f=(frame < maxframe/2) ? frame+1 : maxframe - frame;
                f=f*0.1+double(BufferHt)/60.0;
                x1 = (x-BufferWi/2.0)/f;
                y1 = (y-BufferHt/2.0)/f;
                h=sin(x1) * cos(y1);
                break;
            case 4:
                /*
                The butterfly curve is a transcendental plane curve discovered by Temple H. Fay. The curve is given by the following parametric equations:

                x = \sin(t) \left(e^{\cos(t)} - 2\cos(4t) - \sin^5\left({t \over 12}\right)\right)
                y = \cos(t) \left(e^{\cos(t)} - 2\cos(4t) - \sin^5\left({t \over 12}\right)\right)
                or by the following polar equation:

                r=e^{\cos \theta} - 2 \cos (4 \theta ) + \sin^5\left(\frac{2 \theta - \pi}{24}\right)


                x = \cos(a t) - \cos(b t)^j
                y = \sin(c t) - \sin(d t)^k

                x=2*sin(3*t)*cos(t)
                y=2*sin(3*t)*sin(t)
                */
                h=2*sin(3*x)*cos(y) * sin (offset + ((x+y)*pi2 / (BufferHt+BufferWi)));
                h =  x*(y^3) - y*(x^3);
                break;
            }
            hsv.saturation=1.0;
            hsv.value=1.0;
            if (Chunks <= 1 || int(h*Chunks) % Skip != 0)
            {
                if (ColorScheme == 0)
                {
                    hsv.hue=h;
                    SetPixel(x,y,hsv);
                }
                else
                {
                    GetMultiColorBlend(h,false,color);
                    SetPixel(x,y,color);
                }
            }
        }
    }
}
