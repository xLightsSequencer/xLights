/***************************************************************
 * Name:      RenderTwinkle.cpp
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


void RgbEffects::RenderTwinkle(int Count,int Steps, bool Strobe, bool reRandomize)
{

    int i,ColorIdx;
    int lights = (BufferHt*BufferWi)*(Count/100.0); // Count is in range of 1-100 from slider bar
    int step;
    if(lights>0) step=BufferHt*BufferWi/lights;
    else step=1;
    int max_modulo;
    max_modulo=Steps;
    if(max_modulo<2) max_modulo=2;  // scm  could we be getting 0 passed in?
    int max_modulo2=max_modulo/2;
    if(max_modulo2<1) max_modulo2=1;

    if(step<1) step=1;

    size_t colorcnt=GetColorCount();
    i = 0;
    if (needToInit) {
        needToInit = false;
        strobe.clear();
        for (int y=0; y < BufferHt; y++) {
            for (int x=0; x < BufferWi; x++) {
                i++;
                if (i%step==1 || step==1) {
                    int s = strobe.size();
                    strobe.resize(s + 1);
                    strobe[s].duration = rand() % max_modulo;
                    
                    strobe[s].x = x;
                    strobe[s].y = y;
                    
                    ColorIdx=rand()%colorcnt;
                    palette.GetHSV(ColorIdx, strobe[s].hsv);
                    palette.GetColor(ColorIdx, strobe[s].color);
                }
            }
        }
    }

    for (int x = 0; x < strobe.size(); x++) {
        strobe[x].duration++;
        if (strobe[x].duration < 0) {
            continue;
        }
        if (strobe[x].duration == max_modulo) {
            strobe[x].duration = 0;
            if (reRandomize) {
                strobe[x].duration -= rand() % max_modulo2;
                ColorIdx=rand()%colorcnt;
                palette.GetHSV(ColorIdx, strobe[x].hsv);
                palette.GetColor(ColorIdx, strobe[x].color);
            }
        }
        int i7 = strobe[x].duration;
        double v = strobe[x].hsv.value;
        if(i7<=max_modulo2)
        {
            if(max_modulo2>0) v = (1.0*i7)/max_modulo2;
            else v =0;
        }
        else
        {
            if(max_modulo2>0)v = (max_modulo-i7)*1.0/(max_modulo2);
            else v = 0;
        }
        if(v<0.0) v=0.0;
        
        if(Strobe)
        {
            if(i7==max_modulo2)  v = 1.0;
            else v = 0.0;
        }
        if (allowAlpha) {
            xlColor color(strobe[x].color);
            color.alpha = 255.0 * v;
            SetPixel(strobe[x].x,strobe[x].y,color); // Turn pixel on
        } else {
            wxImage::HSVValue hsv = strobe[x].hsv;
            //  we left the Hue and Saturation alone, we are just modifiying the Brightness Value
            hsv.value = v;
            SetPixel(strobe[x].x,strobe[x].y,hsv); // Turn pixel on
        }
    }
}
