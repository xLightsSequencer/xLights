/***************************************************************
 * Name:      RenderOn.cpp
 * Purpose:   Implements RGB effects
 * Author:    Gil Jones (gil@threebuttes.com)
 * Created:   2014-12-12
 * Copyright: 2014 by Gil Jones
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

#include "RgbEffects.h"
#include "Effect.h"

void RgbEffects::RenderOn(Effect *eff, int start, int end, bool shimmer, float cycles)
{
    int x,y;
    int cidx = 0;
    if (shimmer) {
        int tot = curPeriod - curEffStartPer;
        if (tot % 2) {
            if (palette.Size() <= 1) {
                return;
            }
            cidx = 1;
        }
    }

    double adjust = GetEffectTimeIntervalPosition(cycles);
    
    xlColor color;
    if (start == 100 && end == 100) {
        palette.GetColor(cidx, color);
    } else {
        wxImage::HSVValue hsv;
        palette.GetHSV(cidx,hsv);
        double d = adjust;
        d = start + (end - start) * d;
        d = d / 100.0;
        hsv.value = hsv.value * d;
        color = hsv;
    }

    //Every Node set to selected color
    for (x=0; x<BufferWi; x++)
    {
        for (y=0; y<BufferHt; y++)
        {
            SetPixel(x,y,color);
        }
    }
    if (shimmer || cycles != 1.0) {
        wxMutexLocker lock(eff->GetBackgroundDisplayList().lock);
        eff->GetBackgroundDisplayList().resize((curEffEndPer - curEffStartPer + 1) * 4);
        CopyPixelsToDisplayListX(eff, 0, 0, 0);
    } else if (needToInit) {
        wxMutexLocker lock(eff->GetBackgroundDisplayList().lock);
        eff->GetBackgroundDisplayList().resize(4);
        if (start == 100 && end == 100) {
            palette.GetColor(0, color);
            SetDisplayListHRect(eff, 0, 0.0, 0.0, 1.0, 1.0, color, color);
        } else {
            wxImage::HSVValue hsv;
            palette.GetHSV(cidx,hsv);
            hsv.value = hsv.value * start / 100.0;
            color = hsv;
        
            palette.GetHSV(cidx,hsv);
            hsv.value = hsv.value * end / 100.0;
            SetDisplayListHRect(eff, 0, 0.0, 0.0, 1.0, 1.0, color, xlColor(hsv));
        }
        needToInit = false;
    }
}
