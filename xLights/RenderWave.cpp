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


//CAUTION: must match list box order
#define WAVETYPE_SINE  0
#define WAVETYPE_TRIANGLE  1
#define WAVETYPE_SQUARE  2
#define WAVETYPE_DECAYSINE  3
#define WAVETYPE_IVYFRACTAL  4


void RgbEffects::RenderWave(int WaveType,int FillColor,bool MirrorWave,int NumberWaves,int ThicknessWave,int WaveHeight, int WaveDirection)
{
    /*
    WaveType.Add("Sine");       // 0
    WaveType.Add("Triangle");   // 1
    WaveType.Add("Square");     //2
    WaveType.Add("Decaying Sine"); //3
    WaveType.Add("Ivy/fractal"); //4
    FillColors.Add("None");     // 0
    FillColors.Add("Rainbow");  // 1
    FillColors.Add("Palette");  // 2
    */

    int x,y,y1,y2,y1mirror,y2mirror,ystart,dy,modx,modx2;
    double a,r,yc,deltay;
    double degree,radian,degree_per_x,h=0.0;
    static const double pi2=6.283185307;
    static const double pi_180 = 0.01745329;
    xlColour color;
    wxImage::HSVValue hsv,hsv0,hsv1;
    palette.GetHSV(0,hsv0);
    palette.GetHSV(1,hsv1);
    size_t colorcnt=GetColorCount();
    static std::vector<int> ybranch;

    yc = BufferHt/2.0;
    r=yc;
    if (WaveType == WAVETYPE_DECAYSINE) {
        r -= state/4;
//        if (r < 100./ThicknessWave) r = 100./ThicknessWave; //turn into straight line; don't completely disappear
        if (r < 0) r = 0; //turn into straight line; don't completely disappear
    }
    else if (WaveType == WAVETYPE_IVYFRACTAL) //generate branches at start of effect
        if (!state || (ybranch.size() != BufferWi)) {
            r = 0;
            int delay = 0, delta; //next branch length, angle
            ybranch.resize(BufferWi);
            for (int x = 0; x < BufferWi; ++x) {
//                if (delay < 1) angle = (rand() % 45) - 22.5;
                ybranch[x] = (delay-- > 0)? ybranch[x - 1] + delta: 2 * yc;
                if (ybranch[x] >= 2 * BufferHt) { ybranch[x] = 2 * BufferHt - 1; if (delay > 1) delay = 1; }
                if (ybranch[x] < 0) { ybranch[x] = 0; if (delay > 1) delay = 1; }
                if (delay < 1) {
                    delta = (rand() % 7) - 4;
                    delay = 2 + (rand() % 3);
                }
            }
        }
    degree_per_x = NumberWaves/BufferWi;
    degree = 1+ state%NumberWaves;
    hsv.saturation=1.0;
    hsv.value=1.0;
    hsv.hue=1.0;
    for (x=0; x<BufferWi; x++) {
        if ((WaveType == WAVETYPE_IVYFRACTAL) && (x > state/2)) break; //ivy "grows"
        if (WaveDirection==0)
            degree = x * degree_per_x + state; // state causes it to move
        else
            degree = x * degree_per_x - state; // state causes it to move
        radian = degree * pi_180;
        if(WaveType==WAVETYPE_TRIANGLE) { // Triangle
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
        } else if (WaveType == WAVETYPE_IVYFRACTAL) {
            ystart = ybranch[x] / 2;
        } else {
            ystart = (int) (r*(WaveHeight/100.0) * sin(radian) +yc);
        }
//        x=r * cos(t);
//        y=r * sin(t);

        if(x>=0 && x<BufferWi && ystart>=0 && ystart <BufferHt) {
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
            if (y2 <= y1) y2 = y1 + 1; //minimum height
            y1mirror= yc + (yc -y1);
            y2mirror= yc + (yc -y2);
            deltay = y2-y1;


            if(WaveType==WAVETYPE_SQUARE) { // Square Wave
                if(sin(radian)>0.0) {
                    y1=yc+1;
                    y2=yc + yc*(WaveHeight/100.0);
                } else {
                    y1=yc - yc*(WaveHeight/100.0);
                    y2=yc;
                }
            }
            for (y=y1; y<y2; y++) {
                if(FillColor==0) {
                    SetPixel(x,y,hsv0);  // fill with color 2
                    //       hsv.hue=(double)(BufferHt-y)/deltay;
                } else if(FillColor==1) {

                    hsv.hue=(double)(y-y1)/deltay;
                    SetPixel(x,y,hsv); // rainbow
                } else if(FillColor==2) {
                    hsv.hue=(double)(y-y1)/deltay;
                    GetMultiColorBlend(hsv.hue,false,color);
                    SetPixel(x,y,color); // palete fill
                }
            }

            if(MirrorWave) {

                if(y1mirror<y2mirror) {
                    y1=y1mirror;
                    y2=y2mirror;
                } else {
                    y2=y1mirror;
                    y1=y2mirror;
                }

                for (y=y1; y<y2; y++) {
                    if(FillColor==0) {
                        SetPixel(x,y,hsv0);  // fill with color 2
                        //       hsv.hue=(double)(BufferHt-y)/deltay;
                    } else if(FillColor==1) {

                        hsv.hue=(double)(y-y1)/deltay;
                        SetPixel(x,y,hsv); // rainbow
                    } else if(FillColor==2) {
                        hsv.hue=(double)(y-y1)/deltay;
                        GetMultiColorBlend(hsv.hue,false,color);
                        SetPixel(x,y,color); // palete fill
                    }
                }
            }
        }
    }
}
