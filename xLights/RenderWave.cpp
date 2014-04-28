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



void RgbEffects::RenderWave(int WaveType,int FillColor,bool MirrorWave,int NumberWaves,int ThicknessWave,int WaveHeight, int WaveDirection)
{
    /*
    WaveType.Add("Sine");       // 0
    WaveType.Add("Triangle");   // 1
    WaveType.Add("Square");     //2
    FillColors.Add("None");     // 0
    FillColors.Add("Rainbow");  // 1
    FillColors.Add("Palette");  // 2
    */

    int x,y,y1,y2,y1mirror,y2mirror,ystart,d,dy,modx,modx2;
    double a,n,rx,ry,r,yc,deltay;
    double degree,radian,degree_per_x,h=0.0;
    static const double pi2=6.283185307;
    static const double pi_180 = 0.01745329;
    wxColour color;
    wxImage::HSVValue hsv,hsv0,hsv1;
    palette.GetHSV(0,hsv0);
    palette.GetHSV(1,hsv1);
    size_t colorcnt=GetColorCount();

    yc = BufferHt/2.0;
    r=yc;
    degree_per_x = NumberWaves/BufferWi;
    degree = 1+ state%NumberWaves;
    hsv.saturation=1.0;
    hsv.value=1.0;
    hsv.hue=1.0;
    for (x=0; x<BufferWi; x++)
    {
        if (WaveDirection==0)
            degree = x * degree_per_x + state; // state causes it to move
        else
            degree = x * degree_per_x - state; // state causes it to move
        radian = degree * pi_180;
        if(WaveType==1) // Triangle
        {
            /*
            .
            .
            .      x
            .     x x
            .    x   x
            .   x     x
            .  x       x
            . x         x
            ******************************************** yc
            .      a
            .
            .
            .
            .
            .
            .
            */

            a = BufferWi/(NumberWaves/180) ;
            if (a <1) a=1;

            dy = a/2;
            if(dy<1) dy=1;

            modx = state%(int) dy;
            modx2 = state%(int) a;
            if(modx2>dy)
                ystart = yc - modx*dy ;
            else
                ystart = yc + modx*dy ;


            //  if( sin(radian)<0.0) ystart=-ystart;
        }
        else
        {
            ystart = (int) (r*(WaveHeight/100.0) * sin(radian) +yc);
        }
//        x=r * cos(t);
//        y=r * sin(t);

        if(x>=0 and x<BufferWi and ystart>=0 and ystart <BufferHt)
        {
            //  SetPixel(x,ystart,hsv0);  // just leading edge
            /*

                BufferHt
                .
                .
                .
                x <- y2
                x <- ystart. calculated point of wave
                x <- y1
                .
                .
                + < - yc
               .
                .
                x <- y2mirror
                x
                x <- y1mirror
                .
                .
                .
                0
            */


            y1=(int) (ystart - (r*(ThicknessWave/100.0)));
            y2=(int) (ystart + (r*(ThicknessWave/100.0)));
            y1mirror= yc + (yc -y1);
            y2mirror= yc + (yc -y2);
            deltay = y2-y1;


            if(WaveType==2) // Square Wave
            {
                if(sin(radian)>0.0)
                {
                    y1=yc+1;
                    y2=yc + yc*(WaveHeight/100.0);
                }
                else
                {
                    y1=yc - yc*(WaveHeight/100.0);
                    y2=yc;
                }
            }
            for (y=y1; y<y2; y++)
            {
                if(FillColor==0)
                {
                    SetPixel(x,y,hsv0);  // fill with color 2
                    //       hsv.hue=(double)(BufferHt-y)/deltay;
                }
                else if(FillColor==1)
                {

                    hsv.hue=(double)(y-y1)/deltay;
                    SetPixel(x,y,hsv); // rainbow
                }
                else if(FillColor==2)
                {
                    hsv.hue=(double)(y-y1)/deltay;
                    GetMultiColorBlend(hsv.hue,false,color);
                    SetPixel(x,y,color); // palete fill
                }
            }

            if(MirrorWave)
            {

                if(y1mirror<y2mirror)
                {
                    y1=y1mirror;
                    y2=y2mirror;
                }
                else
                {
                    y2=y1mirror;
                    y1=y2mirror;
                }

                for (y=y1; y<y2; y++)
                {
                    if(FillColor==0)
                    {
                        SetPixel(x,y,hsv0);  // fill with color 2
                        //       hsv.hue=(double)(BufferHt-y)/deltay;
                    }
                    else if(FillColor==1)
                    {

                        hsv.hue=(double)(y-y1)/deltay;
                        SetPixel(x,y,hsv); // rainbow
                    }
                    else if(FillColor==2)
                    {
                        hsv.hue=(double)(y-y1)/deltay;
                        GetMultiColorBlend(hsv.hue,false,color);
                        SetPixel(x,y,color); // palete fill
                    }
                }
            }
        }
    }
}
