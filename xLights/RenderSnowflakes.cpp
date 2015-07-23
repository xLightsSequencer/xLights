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

void RgbEffects::set_pixel_if_not_color(int x, int y, xlColor toColor, xlColor notColor, bool wrapx, bool wrapy)
{
    int adjx = x;
    int adjy = y;

    if (x < 0) {
        if (wrapx) {
            adjx += BufferWi;
        }
        else {
            return;
        }
    }
    else if (x >= BufferWi) {
        if (wrapx) {
            adjx -= BufferWi;
        }
        else {
            return;
        }
    }
    if (y < 0) {
        if (wrapy) {
            adjy += BufferHt;
        }
        else {
            return;
        }
    }
    else if (y >= BufferHt) {
        if (wrapy) {
            adjy -= BufferHt;
        }
        else {
            return;
        }
    }

    if (GetTempPixel(adjx, adjy) != notColor) {
        SetPixel(adjx, adjy, toColor);
    }
}

void RgbEffects::RenderSnowflakes(int Count, int SnowflakeType, int sSpeed, bool accumulate)
{
    int i,n,x,x0,y0,y,check,delta_y;
    xlColour color1,color2;
    bool wrapx = false; // set to true if you want snowflakes to draw wrapped around when near edges in the accumulate effect.

    palette.GetColor(0, color1);
    palette.GetColor(1, color2);

    if (needToInit ||
        Count != LastSnowflakeCount ||
        SnowflakeType != LastSnowflakeType ||
        accumulate != LastSnowflakeAccumulate) {

        // initialize
        needToInit = false;
        LastSnowflakeCount=Count;
        LastSnowflakeType=SnowflakeType;
        LastSnowflakeAccumulate=accumulate;
        ClearTempBuf();
        effectState = 0;

        // place Count snowflakes
        if(!accumulate)
        {
            for (n=0; n < Count; n++) {

                delta_y=BufferHt/4;
                y0=(n % 4)*delta_y;

                if (y0+delta_y > BufferHt) delta_y = BufferHt-y0;
                if (delta_y<1) delta_y=1;

                // find unused space
                for (check=0; check < 20; check++)
                {
                    x=rand() % BufferWi;
                    y=y0 + (rand() % delta_y);
                    if (GetTempPixel(x,y) == xlBLACK) {
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
    }

    // move snowflakes
    int movement = (curPeriod - curEffStartPer) * sSpeed * frameTimeInMs / 50;
    int new_x,new_y;
    int starty = accumulate ? 1 : 0;

    for (x=0; x<BufferWi; x++) {
        new_x = (x+movement/20) % BufferWi; // CW

        for (y=starty; y<BufferHt; y++) {
            if (accumulate) {

                // if there is a flake to move
                if (GetTempPixel(x, y) != xlBLACK) {

                    // check where we can move to?
                    bool moveleft = (GetTempPixel(x-1 < 0 ? x-1+BufferWi : x-1, y-1) == xlBLACK);
                    bool movedown (GetTempPixel(x, y-1) == xlBLACK);
                    bool moveright (GetTempPixel(x+1 >= BufferWi ? x+1-BufferWi : x+1, y-1) == xlBLACK);
                    x0 = x;

                    //we have something to move
                    // randomly move the flake left or right
                    if (moveleft || movedown || moveright)
                    {
                        switch(rand() % 5)
                        {
                            case 0:
                                if (moveleft) {
                                    x0 = x - 1;
                                }
                                else {
                                    if (movedown) {
                                        x0 = x;
                                    }
                                    else {
                                        x0 = x + 1;
                                    }
                                }
                                break;
                            case 1:
                                if (moveright) {
                                    x0 = x + 1;
                                }
                                else {
                                    if (movedown) {
                                        x0 = x;
                                    }
                                    else {
                                        x0 = x - 1;
                                    }
                                }
                                break;
                            default:  //down more often then left/right to look less "jittery"
                                if (movedown) {
                                    x0 = x;
                                }
                                else if (moveright && !moveleft) {
                                    x0 = x + 1;
                                }
                                else if (!moveright && moveleft) {
                                    x0 = x - 1;
                                }
                                else {
                                    switch(rand() % 2)
                                    {
                                    case 0:
                                        x0 = x+1;
                                        break;
                                    default:
                                        x0 = x-1;
                                        break;
                                    }
                                }
                                break;
                        }

                        // handle wrap around
                        if (x0 < 0) {
                            x0 += BufferWi;
                        }
                        else if (x0 >= BufferWi) {
                            x0 -= BufferWi;
                        }

                        // and move it down
                        y0 = y - 1;

                        // if there is already a flake there then we dont do anything
                        //if (GetTempPixel(x0, y0) == xlBLACK)
                        //{
                            // move the flake down
                            SetTempPixel(x0,y0,color1);
                            SetTempPixel(x,y,xlBLACK);

                            bool isAtBottom = true;
                            for (int yt = 0; yt < y0; yt++) {
                                if (GetTempPixel(x0, yt) == xlBLACK) {
                                    isAtBottom = false;
                                    break;
                                }
                            }

                            if (isAtBottom) {
                                // we cant move any further so we can add one at the top
                                effectState--;
                            }
                        //}
                    }
                }
            } else {
                new_y = (y+movement/10) % BufferHt;
                SetPixel(x, y, GetTempPixel(new_x, new_y));
            }
        }
    }

    if (accumulate)
    {
        // add new flakes to the top
        check = 0;
        int placedFullCount = 0;
        while (effectState < Count && check < 20) {
            // find unused space
            x=rand() % BufferWi;
            if (GetTempPixel(x, BufferHt-1) == xlBLACK) {
                effectState++;
                SetTempPixel(x, BufferHt-1, color1);

                bool isAtBottom = true;
                for (int yt = 0; yt < BufferHt-1; yt++) {
                    if (GetTempPixel(x, yt) == xlBLACK) {
                        isAtBottom = false;
                        break;
                    }
                }
                if (isAtBottom) {
                    //the placed pixel fills the column, make sure we note that so we can place
                    //another snowflake
                    placedFullCount++;
                }
            }
            check++;
        }
        effectState -= placedFullCount;

        // paint my current state
        for (int y=0; y < BufferHt; y++) {
            for (int x=0; x < BufferWi; x++) {

                if (GetTempPixel(x, y) == color1)
                {
                    // draw flake, SnowflakeType=0 is random type
                    switch (SnowflakeType == 0 ? rand() % 5 : SnowflakeType-1)
                    {
                        case 0:
                            // single node
                            SetPixel(x, y, color1);
                            break;
                        case 1:
                            // 5 nodes
                            SetPixel(x,y,color1);
                            set_pixel_if_not_color(x-1, y, color2, color1, wrapx, false);
                            set_pixel_if_not_color(x+1, y, color2, color1, wrapx, false);
                            set_pixel_if_not_color(x, y-1, color2, color1, wrapx, false);
                            set_pixel_if_not_color(x, y+1, color2, color1, wrapx, false);
                            break;
                        case 2:
                            {
                                // 3 nodes
                                SetPixel(x,y,color1);
                                bool isAtBottom = true;
                                for (int yt = 0; yt < y - 1; yt++) {
                                    if (GetTempPixel(x, yt) == xlBLACK) {
                                        isAtBottom = false;
                                        break;
                                    }
                                }

                                // when flake has settled always paint it horizontally
                                if (isAtBottom)
                                {
                                    set_pixel_if_not_color(x-1, y, color2, color1, wrapx, false);
                                    set_pixel_if_not_color(x+1, y, color2, color1, wrapx, false);
                                }
                                else {
                                    if (rand() % 100 > 50)      // % 2 was not so random
                                    {
                                        set_pixel_if_not_color(x-1, y, color2, color1, wrapx, false);
                                        set_pixel_if_not_color(x+1, y, color2, color1, wrapx, false);
                                    }
                                    else
                                    {
                                        set_pixel_if_not_color(x, y-1, color2, color1, wrapx, false);
                                        set_pixel_if_not_color(x, y+1, color2, color1, wrapx, false);
                                    }
                                }
                            }
                            break;
                        case 3:
                            // 9 nodes
                            SetPixel(x,y,color1);
                            for (i=1; i<=2; i++)
                            {
                                set_pixel_if_not_color(x-i, y, color2, color1, wrapx, false);
                                set_pixel_if_not_color(x+i, y, color2, color1, wrapx, false);
                                set_pixel_if_not_color(x, y-i, color2, color1, wrapx, false);
                                set_pixel_if_not_color(x, y+i, color2, color1, wrapx, false);
                            }
                            break;
                        case 4:
                            // 13 nodes
                            SetPixel(x,y,color1);
                            set_pixel_if_not_color(x-1, y, color2, color1, wrapx, false);
                            set_pixel_if_not_color(x+1, y, color2, color1, wrapx, false);
                            set_pixel_if_not_color(x, y+1, color2, color1, wrapx, false);
                            set_pixel_if_not_color(x, y-1, color2, color1, wrapx, false);

                            set_pixel_if_not_color(x-1, y+2, color2, color1, wrapx, false);
                            set_pixel_if_not_color(x+1, y+2, color2, color1, wrapx, false);
                            set_pixel_if_not_color(x-1, y-2, color2, color1, wrapx, false);
                            set_pixel_if_not_color(x+1, y-2, color2, color1, wrapx, false);
                            set_pixel_if_not_color(x+2, y-1, color2, color1, wrapx, false);
                            set_pixel_if_not_color(x+2, y+1, color2, color1, wrapx, false);
                            set_pixel_if_not_color(x-2, y-1, color2, color1, wrapx, false);
                            set_pixel_if_not_color(x-2, y+1, color2, color1, wrapx, false);
                            break;
                        case 5:
                            // 45 nodes (not enabled)
                            break;
                    }
                }
            }
        }
    }
}
