/***************************************************************
 * Name:      RenderShimmer.cpp
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

void RgbEffects::RenderShimmer(int Count,int Steps, bool Strobe)
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
    srand (time(NULL));
    size_t colorcnt=GetColorCount();

    i=0;
    int on_off=0;
    int irandom=Strobe;
    for (y=0; y<BufferHt; y++) // For my 20x120 megatree, BufferHt=120
    {
        for (x=0; x<BufferWi; x++) // BufferWi=20 in the above example
        {
            i++;
            int imod=(state/10)%2;
            int icolor=(state/10)%colorcnt;
            if(irandom==1)
            {
                ColorIdx=rand()% colorcnt; // Select random numbers from 0 up to number of colors the user has checked. 0-5 if 6 boxes checked
                palette.GetHSV(ColorIdx, hsv); // Now go and get the hsv value for this ColorIdx
            }
            else
                palette.GetHSV(icolor, hsv); // Now go and get the hsv value for this ColorIdx
            if(on_off==1)
            {
                if(imod==1) // Should we draw a light?
                {
                    // Yes, so now decide on what color it should be

                }
                else
                {
                    hsv.value=0.0;
                }
            }
            else
            {

            }
            SetPixel(x,y,hsv); // Turn pixel
        }
    }
}
