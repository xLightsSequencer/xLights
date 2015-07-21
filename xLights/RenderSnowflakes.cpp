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

 * 2015 Keith Westley added support for accumulation of snow for type 1 flakes only
**************************************************************/
#include <cmath>
#include "RgbEffects.h"

void RgbEffects::RenderSnowflakes(int Count, int SnowflakeType, int sSpeed, bool accumulate)
{
    // accumulate only works with type 1 snowflakes
    if (accumulate)
    {
        SnowflakeType = 1;
    }

    int i,n,x,x0,y0,y,check,delta_y;
    xlColour color1,color2;

    if (needToInit || Count != LastSnowflakeCount || SnowflakeType != LastSnowflakeType)
    {
        // initialize
        needToInit = false;
        LastSnowflakeCount=Count;
        LastSnowflakeType=SnowflakeType;
        ClearTempBuf();
        effectState = 0;

        // when we are not in accumulate mode then we pre-create the background

        palette.GetColor(0,color1);
        palette.GetColor(1,color2);

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
                if (GetTempPixelRGB(x,y) == xlBLACK) {
                    effectState++;
                    break;
                }
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
    
    int movement = (curPeriod - curEffStartPer) * sSpeed * frameTimeInMs / 50;
    // move snowflakes
    int new_x,new_y;
    int starty = accumulate ? 1 : 0;
    for (x=0; x<BufferWi; x++) {
        new_x = (x+movement/20) % BufferWi; // CW
        for (y=starty; y<BufferHt; y++) {
            if (accumulate) {
                // get the pixel
                GetTempPixel(x, y, color1);
                
                bool isAtBottom = true;
                for (int yt = 0; yt < y; yt++) {
                    if (GetTempPixel(x, yt) == xlBLACK) {
                        isAtBottom = false;
                    }
                }
                // if there is a flake to move
                if (color1 != xlBLACK && !isAtBottom)
                {
                    //we have something to move
                    // randomly move the flake left or right
                    switch(rand() % 5)
                    {
                        case 0:
                            x0 = x - 1;
                            break;
                        case 1:
                            x0 = x + 1;
                            break;
                        default:  //down more often then left/right to look less "jittery"
                            x0 = x;
                            break;
                    }
                    
                    // handle wrap around
                    if (x0 < 0)
                    {
                        x0 += BufferWi;
                    }
                    else if (x0 >= BufferWi)
                    {
                        x0 -= BufferWi;
                    }
                    
                    // and move it down
                    y0 = y - 1;
                    
                    // if there is already a flake there then we dont do anything
                    xlColor color3;
                    GetTempPixel(x0, y0, color3);
                    if (color3 == color1 && x0 != x) {
                        x0 = x;
                        //couldn't move left or right, try straight down
                        GetTempPixel(x0, y0, color3);
                    }

                    if (color3 != color1)
                    {
                        // move the flake down
                        SetTempPixel(x0,y0,color1);
                        
                        isAtBottom = true;
                        for (int yt = 0; yt < y0; yt++) {
                            if (GetTempPixel(x0, yt) == xlBLACK) {
                                isAtBottom = false;
                            }
                        }
                        if (isAtBottom) {
                            //this flake has reached the end, record so we add a new flake at the top
                            effectState--;
                        }
                        SetTempPixel(x,y,xlBLACK);
                    }
                }
            } else {
                new_y = (y+movement/10) % BufferHt;
                
                GetTempPixel(new_x, new_y, color1);
                SetPixel(x,y,color1);
            }
        }
    }

    

    if (accumulate)
    {
        // add new flakes to the top
        palette.GetColor(0, color1);
        check = 0;
        int placedFullCount = 0;
        while (effectState < Count && check < 20) {
            // find unused space
            for (check=0; check < 20; check++) {
                x=rand() % BufferWi;
                if (GetTempPixelRGB(x,y) == xlBLACK) {
                    effectState++;
                    SetTempPixel(x,BufferHt - 1,color1);
                    
                    bool isAtBottom = true;
                    for (int yt = 0; yt < BufferHt; yt++) {
                        if (GetTempPixel(x, yt) == xlBLACK) {
                            isAtBottom = false;
                        }
                    }
                    if (isAtBottom) {
                        //the placed pixel fills the column, make sure we note that so we can place
                        //another snowflake
                        placedFullCount++;
                    }
                    break;
                }
            }
        }
        effectState -= placedFullCount;
        

        // paint my current state
        for (int y=0; y < BufferHt; y++)
        {
            for (int x=0; x < BufferWi; x++)
            {
                GetTempPixel(x, y, color1);
                if (color1 != xlBLACK)
                {
                    SetPixel(x, y, color1);
                }
            }
        }
    }
}
