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

#define WANT_DEBUG_IMPL
#define WANT_DEBUG  -99 //unbuffered in case app crashes
#include "djdebug.cpp"
#ifndef debug_function //dummy defs if debug cpp not included above
#define debug(level, ...)
#define debug_more(level, ...)
#define debug_function(level)
#endif

//cut down on mem allocs outside debug() when WANT_DEBUG is off:
#ifdef WANT_DEBUG
#define IFDEBUG(stmt)  stmt
#else
#define IFDEBUG(stmt)
#endif // WANT_DEBUG




void RgbEffects::RenderShimmer(int Count,int Duty_Factor, bool Strobe)
{

    int x,y,i,i7,ColorIdx;
    int lights = (BufferHt*BufferWi)*(Count/100.0); // Count is in range of 1-100 from slider bar
    int step;
    if(lights>0) step=BufferHt*BufferWi/lights;
    else step=1;
    int max_modulo;
    max_modulo=Duty_Factor;
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
    int irandom=Strobe; // Should we randomly assign colors from palette or cycle thru sequentially?
    int istate=state%1000;
    int imod=state%100;
    debug(10, "State %d ", state);
    int icolor=(state/10)%colorcnt;
    for (y=0; y<BufferHt; y++) // For my 20x120 megatree, BufferHt=120
    {
        for (x=0; x<BufferWi; x++) // BufferWi=20 in the above example
        {
            i++;
            if(irandom==1) // Should we randomly assign colors from palette or cycle thru sequentially?
            {
                ColorIdx=rand()% colorcnt; // Select random numbers from 0 up to number of colors the user has checked. 0-5 if 6 boxes checked
                palette.GetHSV(ColorIdx, hsv); // Now go and get the hsv value for this ColorIdx
            }
            else
                palette.GetHSV(icolor, hsv); // Now go and get the hsv value for this ColorIdx
            if(imod<=Duty_Factor)  // Should we draw a light?
            {
                // Yes, use HSV value calculated above
            }
            else
            {
                hsv.value=0.0; // this is the off cycle for a light so set to BLACK
                hsv.saturation=1.0;
            }


            SetPixel(x,y,hsv); // Turn pixel
        }
    }
}
