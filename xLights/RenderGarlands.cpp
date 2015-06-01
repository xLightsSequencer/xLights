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

int GetDirection(const wxString &direction) {
    if ("Up" == direction) {
        return 0;
    } else if ("Down" == direction) {
        return 1;
    } else if ("Left" == direction) {
        return 2;
    } else if ("Right" == direction) {
        return 3;
    } else if ("Up then Down" == direction) {
        return 4;
    } else if ("Down then Up" == direction) {
        return 5;
    } else if ("Left then Right" == direction) {
        return 6;
    } else if ("Right then Left" == direction) {
        return 7;
    }
    
    return 0;
}

void RgbEffects::RenderGarlands(int GarlandType, int Spacing, float cycles, const wxString &direction)
{
    if (Spacing < 1) {
        Spacing = 1;
    }
    int x,y,yadj,ylimit,ring;
    double ratio;
    xlColour color;
    int dir = GetDirection(direction);
    double position = GetEffectTimeIntervalPosition(cycles);
    if (dir > 3) {
        dir -= 4;
        if (position > 0.5) {
            position = (1.0 - position) * 2.0;
        } else {
            position *= 2.0;
        }
    }
    int buffMax = BufferHt;
    int garlandWid = BufferWi;
    if (dir > 1) {
        buffMax = BufferWi;
        garlandWid = BufferHt;
    }
    double PixelSpacing=Spacing*buffMax/100.0;
    if (PixelSpacing < 2.0) PixelSpacing=2.0;
    
    
    double total = buffMax * PixelSpacing - buffMax + 1;
    double positionOffset = total * position;
    
    for (ring = 0; ring < buffMax; ring++)
    {
        ratio=double(buffMax-ring-1)/double(buffMax);
        GetMultiColorBlend(ratio, false, color);
        
        y = 1.0 + ring*PixelSpacing - positionOffset;

        
        ylimit=ring;
        for (x=0; x<garlandWid; x++)
        {
            yadj=y;
            switch (GarlandType)
            {
            case 1:
                switch (x%5)
                {
                case 2:
                    yadj-=2;
                    break;
                case 1:
                case 3:
                    yadj-=1;
                    break;
                }
                break;
            case 2:
                switch (x%5)
                {
                case 2:
                    yadj-=4;
                    break;
                case 1:
                case 3:
                    yadj-=2;
                    break;
                }
                break;
            case 3:
                switch (x%6)
                {
                case 3:
                    yadj-=6;
                    break;
                case 2:
                case 4:
                    yadj-=4;
                    break;
                case 1:
                case 5:
                    yadj-=2;
                    break;
                }
                break;
            case 4:
                switch (x%5)
                {
                case 1:
                case 3:
                    yadj-=2;
                    break;
                }
                break;
            }
            if (yadj < ylimit) yadj=ylimit;
            if (yadj < buffMax) {
                if (dir == 1 || dir == 2) {
                    yadj = buffMax - yadj - 1;
                }
                if (dir > 1) {
                    SetPixel(yadj,x,color);
                } else {
                    SetPixel(x,yadj,color);
                }
            }
        }
    }
}
