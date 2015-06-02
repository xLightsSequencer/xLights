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

void RgbEffects::RenderSnowflakes(int Count, int SnowflakeType, int sSpeed)
{
    int i,n,x,y0,y,check,delta_y;
    xlColour color1,color2;
    effectState = (curPeriod - curEffStartPer) * sSpeed * frameTimeInMs / 50;
    
    if (needToInit || Count != LastSnowflakeCount || SnowflakeType != LastSnowflakeType)
    {
        // initialize
        needToInit = false;
        LastSnowflakeCount=Count;
        LastSnowflakeType=SnowflakeType;
        palette.GetColor(0,color1);
        palette.GetColor(1,color2);
        ClearTempBuf();
        // place Count snowflakes
        for (n=0; n < Count; n++)
        {
            delta_y=BufferHt/4;

            y0=(n % 4)*delta_y;
            if (y0+delta_y > BufferHt) delta_y = BufferHt-y0;
             if(delta_y<1) delta_y=1;
            // find unused space
            for (check=0; check < 20; check++)
            {
                x=rand() % BufferWi;
                y=y0 + (rand() % delta_y);
                if (GetTempPixelRGB(x,y) == xlBLACK) break;
            }
            // draw flake, SnowflakeType=0 is random type
            switch (SnowflakeType == 0 ? rand() % 5 : SnowflakeType-1)
            {
            case 0:
                // single node
                SetTempPixel(x,y,color1);
                break;
            case 1:
                // 5 nodes
                if (x < 1) x+=1;
                if (y < 1) y+=1;
                if (x > BufferWi-2) x-=1;
                if (y > BufferHt-2) y-=1;
                SetTempPixel(x,y,color1);
                SetTempPixel(x-1,y,color2);
                SetTempPixel(x+1,y,color2);
                SetTempPixel(x,y-1,color2);
                SetTempPixel(x,y+1,color2);
                break;
            case 2:
                // 3 nodes
                if (x < 1) x+=1;
                if (y < 1) y+=1;
                if (x > BufferWi-2) x-=1;
                if (y > BufferHt-2) y-=1;
                SetTempPixel(x,y,color1);
                if (rand() % 100 > 50)      // % 2 was not so random
                {
                    SetTempPixel(x-1,y,color2);
                    SetTempPixel(x+1,y,color2);
                }
                else
                {
                    SetTempPixel(x,y-1,color2);
                    SetTempPixel(x,y+1,color2);
                }
                break;
            case 3:
                // 9 nodes
                if (x < 2) x+=2;
                if (y < 2) y+=2;
                if (x > BufferWi-3) x-=2;
                if (y > BufferHt-3) y-=2;
                SetTempPixel(x,y,color1);
                for (i=1; i<=2; i++)
                {
                    SetTempPixel(x-i,y,color2);
                    SetTempPixel(x+i,y,color2);
                    SetTempPixel(x,y-i,color2);
                    SetTempPixel(x,y+i,color2);
                }
                break;
            case 4:
                // 13 nodes
                if (x < 2) x+=2;
                if (y < 2) y+=2;
                if (x > BufferWi-3) x-=2;
                if (y > BufferHt-3) y-=2;
                SetTempPixel(x,y,color1);
                SetTempPixel(x-1,y,color2);
                SetTempPixel(x+1,y,color2);
                SetTempPixel(x,y-1,color2);
                SetTempPixel(x,y+1,color2);

                SetTempPixel(x-1,y+2,color2);
                SetTempPixel(x+1,y+2,color2);
                SetTempPixel(x-1,y-2,color2);
                SetTempPixel(x+1,y-2,color2);
                SetTempPixel(x+2,y-1,color2);
                SetTempPixel(x+2,y+1,color2);
                SetTempPixel(x-2,y-1,color2);
                SetTempPixel(x-2,y+1,color2);
                break;
            case 5:
                // 45 nodes (not enabled)
                break;
            }
        }
    }

    // move snowflakes
    int new_x,new_y,new_x2,new_y2;
    for (x=0; x<BufferWi; x++)
    {
        new_x = (x+effectState/20) % BufferWi; // CW
        new_x2 = (x-effectState/20) % BufferWi; // CCW
        if (new_x2 < 0) new_x2+=BufferWi;
        for (y=0; y<BufferHt; y++)
        {
            new_y = (y+effectState/10) % BufferHt;
            new_y2 = (new_y + BufferHt/2) % BufferHt;
            GetTempPixel(new_x,new_y,color1);
            if (color1 == xlBLACK) GetTempPixel(new_x2,new_y2,color1);
            SetPixel(x,y,color1);
        }
    }
}
