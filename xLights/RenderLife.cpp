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

int RgbEffects::Life_CountNeighbors(int x0, int y0)
{
    //     2   3   4
    //     1   X   5
    //     0   7   6
    static int n_x[] = {-1,-1,-1,0,1,1,1,0};
    static int n_y[] = {-1,0,1,1,1,0,-1,-1};
    int x,y,cnt=0;
    for (int i=0; i < 8; i++)
    {
        x=(x0+n_x[i]) % BufferWi;
        y=(y0+n_y[i]) % BufferHt;
        if (x < 0) x+=BufferWi;
        if (y < 0) y+=BufferHt;
        if (GetTempPixelRGB(x,y) != 0) cnt++;
    }
    return cnt;
}

// use tempbuf for calculations
void RgbEffects::RenderLife(int Count, int Type, int lspeed)
{
    int i,x,y,cnt;
    bool isLive;
    xlColour color;
    if(BufferHt<1) BufferHt=1;
    Count=BufferWi * BufferHt * Count / 200 + 1;
    if (needToInit || Count != LastLifeCount || Type != LastLifeType)
    {
        needToInit = false;
        // seed tempbuf
        LastLifeCount=Count;
        LastLifeType=Type;
        ClearTempBuf();
        for(i=0; i<Count; i++)
        {
            x=rand() % BufferWi;
            y=rand() % BufferHt;
            GetMultiColorBlend(rand01(),false,color);
            SetTempPixel(x,y,color);
        }
    }
    effectState = (curPeriod-curEffStartPer) * lspeed * frameTimeInMs / 50;

    long TempState=effectState % 400 / 20;
    if (TempState == LastLifeState)
    {
        pixels=tempbuf;
        return;
    }
    else
    {
        LastLifeState=TempState;
    }
    for (x=0; x < BufferWi; x++)
    {
        for (y=0; y < BufferHt; y++)
        {
            GetTempPixel(x,y,color);
            isLive=(color.GetRGB() != 0);
            cnt=Life_CountNeighbors(x,y);
            switch (Type)
            {
            case 0:
                // B3/S23
                /*
                Any live cell with fewer than two live neighbours dies, as if caused by under-population.
                Any live cell with two or three live neighbours lives on to the next generation.
                Any live cell with more than three live neighbours dies, as if by overcrowding.
                Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
                */
                if (isLive && cnt >= 2 && cnt <= 3)
                {
                    SetPixel(x,y,color);
                }
                else if (!isLive && cnt == 3)
                {
                    GetMultiColorBlend(rand01(),false,color);
                    SetPixel(x,y,color);
                }
                break;
            case 1:
                // B35/S236
                if (isLive && (cnt == 2 || cnt == 3 || cnt == 6))
                {
                    SetPixel(x,y,color);
                }
                else if (!isLive && (cnt == 3 || cnt == 5))
                {
                    GetMultiColorBlend(rand01(),false,color);
                    SetPixel(x,y,color);
                }
                break;
            case 2:
                // B357/S1358
                if (isLive && (cnt == 1 || cnt == 3 || cnt == 5 || cnt == 8))
                {
                    SetPixel(x,y,color);
                }
                else if (!isLive && (cnt == 3 || cnt == 5 || cnt == 7))
                {
                    GetMultiColorBlend(rand01(),false,color);
                    SetPixel(x,y,color);
                }
                break;
            case 3:
                // B378/S235678
                if (isLive && (cnt == 2 || cnt == 3 || cnt >= 5))
                {
                    SetPixel(x,y,color);
                }
                else if (!isLive && (cnt == 3 || cnt == 7 || cnt == 8))
                {
                    GetMultiColorBlend(rand01(),false,color);
                    SetPixel(x,y,color);
                }
                break;
            case 4:
                // B25678/S5678
                if (isLive && (cnt >= 5))
                {
                    SetPixel(x,y,color);
                }
                else if (!isLive && (cnt == 2 || cnt >= 5))
                {
                    GetMultiColorBlend(rand01(),false,color);
                    SetPixel(x,y,color);
                }
                break;
            }
        }
    }
    // copy new life state to tempbuf
    tempbuf=pixels;
}
