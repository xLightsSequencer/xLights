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

void RgbEffects::RenderSean(int Count)
{

    int x,y,i,i7,r,ColorIdx;
    int lights = (BufferHt*BufferWi)*(Count/100.0); // Count is in range of 1-100 from slider bar
    int step=BufferHt*BufferWi/lights;
    if(step<1) step=1;
    double damping=0.8;
    srand(1); // always have the same random numbers for each frame (state)
    wxImage::HSVValue hsv,hsv0,hsv1; //   we will define an hsv color model. The RGB colot model would have been "wxColour color;"
    wxColour color;
    int color1,color2,color3,color4,new_color,old_color;
    size_t colorcnt=GetColorCount();
    i=0;
    palette.GetHSV(0, hsv0); // hsv0.hue, hsv0.saturation, hsv0.value
    palette.GetHSV(1, hsv1);


    if(state==0)
    {
        ClearWaveBuffer1();
        ClearWaveBuffer2();
        color1=100;
    }
    if(state%20==0)
    {
        color1=100;

        SetWaveBuffer2((BufferWi+state)%BufferWi,(BufferHt+state)%BufferHt,color1);
        SetWaveBuffer2((BufferWi+state)%BufferWi,(BufferHt+state)%BufferHt,color1);
    }

    for (y=1; y<BufferHt-1; y++) // For my 20x120 megatree, BufferHt=120
    {
        for (x=1; x<BufferWi-1; x++) // BufferWi=20 in the above example
        {
//            GetTempPixel(x,y,color);
//            isLive=(color.GetRGB() != 0);

            old_color=GetWaveBuffer1(x,y) ;
            color1=GetWaveBuffer2(x-1,y-1);
            color2=GetWaveBuffer2(x+1,y+1) ;
            color3=GetWaveBuffer2(x+1,y-1) ;
            color4=GetWaveBuffer2(x+1,y+1) ;

            new_color = (color1+color2+color3+color4)/2 - old_color;
          //  new_color = (127.0+224.0*0.99999*new_color/8192.);
            new_color=new_color/8.0;
            SetWaveBuffer1(x,y,new_color);
        }
    }

    for (y=0; y<BufferHt; y++) // For my 20x120 megatree, BufferHt=120
    {
        for (x=0; x<BufferWi; x++) // BufferWi=20 in the above example
        {
            new_color=GetWaveBuffer1(x,y);
            if(new_color>= -0.01 && new_color<= 0.01)
            {
                SetPixel(x,y,hsv1);
            }
            else
            {
                if(new_color>0)
                {
                    hsv0.value = (new_color%100)/100.0;
                    SetPixel(x,y,hsv0);
                }
                else
                {
                    hsv0.hue = 0.3;
                    hsv0.value = -(new_color%100)/100.0;
                    SetPixel(x,y,hsv0);
                }
            }
        }
    }
    WaveBuffer0=WaveBuffer2;
    WaveBuffer2=WaveBuffer1;
    WaveBuffer1=WaveBuffer0;
}

