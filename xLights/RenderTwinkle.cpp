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

void RgbEffects::RenderTwinkle(wxXmlNode* model,int Count,int Steps, bool Strobe)
{

    int x,y,i,i7,ColorIdx;
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
    if(Strobe) srand (time(NULL)); // for strobe effect, make lights be random
    else srand(1); // else always have the same random numbers for each frame (state)
    wxImage::HSVValue hsv; //   we will define an hsv color model. The RGB colot model would have been "wxColour color;"

    size_t colorcnt=GetColorCount();

    i=0;

    for (y=0; y<BufferHt; y++) // For my 20x120 megatree, BufferHt=120
    {
        for (x=0; x<BufferWi; x++) // BufferWi=20 in the above example
        {
            i++;

            if(i%step==1 || step==1) // Should we draw a light?
            {
                // Yes, so now decide on what color it should be

                ColorIdx=rand() % colorcnt; // Select random numbers from 0 up to number of colors the user has checked. 0-5 if 6 boxes checked
                palette.GetHSV(ColorIdx, hsv); // Now go and get the hsv value for this ColorIdx
                //    mod_number=rand()% max_modulo;
                //    if(mod_number<1) mod_number=1;
                //     i7=(state/4) % mod_number; // Our twinkle is 9 steps. 4 ramping up, 5th at full brightness and then 4 more ramping down
                //  Note that we are adding state to this calculation, this causes a different blink rate for each light
                i7=(state+rand())% max_modulo;

//                if(i7==0 || i7==8)  hsv.value = 0.1;
//                if(i7==1 || i7==7)  hsv.value = 0.3;
//                if(i7==2 || i7==6)  hsv.value = 0.5;
//                if(i7==3 || i7==5)  hsv.value = 0.7;
//                if(i7==4)  hsv.value = 1.0;
//                else  hsv.value = 0.0;

                if(i7<=max_modulo2)
                {
                    if(max_modulo2>0) hsv.value = (1.0*i7)/max_modulo2;
                    else  hsv.value =0;
                }
                else
                {
                    if(max_modulo2>0) hsv.value = (max_modulo-i7)*1.0/(max_modulo2);
                    else hsv.value = 0;
                }
                if(hsv.value<0.0) hsv.value=0.0;

                if(Strobe)
                {
                    if(i7==max_modulo2)  hsv.value = 1.0;
                    else  hsv.value = 0.0;
                }


                //  we left the Hue and Saturation alone, we are just modifiying the Brightness Value
                SetPixel(x,y,hsv); // Turn pixel on
            }
        }
    }
}
