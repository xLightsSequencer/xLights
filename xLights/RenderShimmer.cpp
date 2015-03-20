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
#include <wx/utils.h>
#include "djdebug.cpp"

#define WANT_DEBUG_IMPL
#define WANT_DEBUG  -99 //unbuffered in case app crashes

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




void RgbEffects::RenderShimmer(int Duty_Factor,bool Use_All_Colors,bool Blink_Timing,int Blinks_Per_Row)
{

    int x,y,i,ColorIdx;


#if 0
    if(step<1) step=1;
    if(Use_All_Colors) srand (time(NULL)); // for Use_All_Colors effect, make lights be random
    else srand(1); // else always have the same random numbers for each frame (state)
#endif

    wxImage::HSVValue hsv; //   we will define an hsv color model. The RGB colot model would have been "wxColour color;"
    srand (time(NULL));
    size_t colorcnt=GetColorCount();

    i=0;

    int slices=100;
    int istate=state/slices; // istate will be a counter every slices units of state. each istate is a square wave
    int imod=(state/(slices/10))%10; // divide this square
    int icolor=istate%colorcnt;
    wxString TimeNow =wxNow();

    debug(10, "%s:%6d istate=%4d imod=%4d icolor=%1d", (const char*)TimeNow,state,istate,imod,icolor);
    for (y=0; y<BufferHt; y++) // For my 20x120 megatree, BufferHt=120
    {
        for (x=0; x<BufferWi; x++) // BufferWi=20 in the above example
        {
            i++;
            if(Use_All_Colors) // Should we randomly assign colors from palette or cycle thru sequentially?
            {
                ColorIdx=rand()% colorcnt; // Select random numbers from 0 up to number of colors the user has checked. 0-5 if 6 boxes checked
                palette.GetHSV(ColorIdx, hsv); // Now go and get the hsv value for this ColorIdx
            }
            else
                palette.GetHSV(icolor, hsv); // Now go and get the hsv value for this ColorIdx
            if(imod<=(Duty_Factor/10))  // Should we draw a light?
            {
                // Yes, use HSV value calculated above
            }
            else
            {
                hsv.value=0.0; // No, this is the off cycle for a light so set to BLACK
                hsv.saturation=1.0;
            }
            SetPixel(x,y,hsv); // Turn pixel
        }
    }
}
