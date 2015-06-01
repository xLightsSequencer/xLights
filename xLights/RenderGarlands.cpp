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

void RgbEffects::RenderGarlands(int GarlandType, int Spacing, float cycles)
{
    if (Spacing < 1) {
        Spacing = 1;
    }
    int x,y,yadj,ylimit,ring;
    double ratio;
    xlColour color;
    
    double PixelSpacing=Spacing*BufferHt/100.0;
    if (PixelSpacing < 2.0) PixelSpacing=2.0;
    
    double position = GetEffectTimeIntervalPosition(cycles);
    int up=1;
    
    double total = BufferHt * PixelSpacing - BufferHt + 1;
    double positionOffset = total * position;
    
    for (ring = 0; ring < BufferHt; ring++)
    {
        ratio=double(BufferHt-ring-1)/double(BufferHt);
        GetMultiColorBlend(ratio, false, color);
        
        y = 1.0 + ring*PixelSpacing - positionOffset;

        
        ylimit=ring; // BufferHt-ring-1;
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
