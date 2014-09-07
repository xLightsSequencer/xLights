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

void RgbEffects::RenderGarlands(int GarlandType, int Spacing)
{
    int x,y,yadj,ylimit,ring;
    double ratio;
    wxColour color;
    int PixelSpacing=Spacing*BufferHt/100+3;
    if(PixelSpacing<1) PixelSpacing=1;
    int limit=BufferHt*PixelSpacing*4;
    if(limit<1) limit=1;
    int GarlandsState=(limit - (state % limit))/4;
    // ring=0 is the top ring
    int up=1;
    for (ring=0; ring<BufferHt; ring++)
    {
        ratio=double(ring)/double(BufferHt);
        GetMultiColorBlend(ratio, false, color);
        y=GarlandsState - ring*PixelSpacing;
        ylimit=BufferHt-ring-1;
        for (x=0; x<BufferWi; x++)
        {
            yadj=y;
            switch (GarlandType)
            {
            case 1:
                switch (x%5)
                {
                case 2:
                    if(up==1) yadj-=2;
                    else yadj+=2;
                    break;
                case 1:
                case 3:
                    if(up==1)yadj-=1;
                    else yadj+=1;
                    break;
                }
                break;
            case 2:
                switch (x%5)
                {
                case 2:
                    if(up==1)yadj-=4;
                    else yadj+=4;
                    break;
                case 1:
                case 3:
                    if(up==1)yadj-=2;
                    else yadj+=2;
                    break;
                }
                break;
            case 3:
                switch (x%6)
                {
                case 3:
                    if(up==1)yadj-=6;
                    else yadj+=6;
                    break;
                case 2:
                case 4:
                    if(up==1)yadj-=4;
                    else yadj+=4;
                    break;
                case 1:
                case 5:
                    if(up==1)yadj-=2;
                    else yadj+=2;
                    break;
                }
                break;
            case 4:
                switch (x%5)
                {
                case 1:
                case 3:
                    if(up==1)yadj-=2;
                    else yadj+=2;
                    break;
                }
                break;
            }
            if (yadj < ylimit) yadj=ylimit;
            if (yadj < BufferHt) SetPixel(x,yadj,color);
        }
    }
}
