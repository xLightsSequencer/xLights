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




void RgbEffects::RenderShimmer(int Duty_Factor, bool Use_All_Colors, float cycles)
{
    int colorcnt=GetColorCount();
    
    double position = GetEffectTimeIntervalPosition(cycles);

    double ColorIdx = round(position * 0.999 * (double)colorcnt);
    
    double pos2 = position * colorcnt;
    while (pos2 > 1.0) {
        pos2 -= 1.0;
    }
    if (pos2 * 100 > Duty_Factor) {
        return;
    }
    
    xlColor color;
    palette.GetColor(ColorIdx, color);
    for (int y=0; y<BufferHt; y++) {
        for (int x=0; x<BufferWi; x++) {
            if(Use_All_Colors) { // Should we randomly assign colors from palette or cycle thru sequentially?
                ColorIdx=rand() % colorcnt; // Select random numbers from 0 up to number of colors the user has checked. 0-5 if 6 boxes checked
                palette.GetColor(ColorIdx, color); // Now go and get the hsv value for this ColorIdx
            }

            SetPixel(x,y,color); // Turn pixel
        }
    }
}
