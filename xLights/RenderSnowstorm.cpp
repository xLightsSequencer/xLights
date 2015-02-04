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

// 0 <= idx <= 7
wxPoint RgbEffects::SnowstormVector(int idx)
{
    wxPoint xy;
    switch (idx)
    {
    case 0:
        xy.x=-1;
        xy.y=0;
        break;
    case 1:
        xy.x=-1;
        xy.y=-1;
        break;
    case 2:
        xy.x=0;
        xy.y=-1;
        break;
    case 3:
        xy.x=1;
        xy.y=-1;
        break;
    case 4:
        xy.x=1;
        xy.y=0;
        break;
    case 5:
        xy.x=1;
        xy.y=1;
        break;
    case 6:
        xy.x=0;
        xy.y=1;
        break;
    default:
        xy.x=-1;
        xy.y=1;
        break;
    }
    return xy;
}

void RgbEffects::SnowstormAdvance(SnowstormClass& ssItem)
{
    const int cnt = 8;  // # of integers in each set in arr[]
    const int arr[] = {30,20,10,5,0,5,10,20,20,15,10,10,10,10,10,15}; // 2 sets of 8 numbers, each of which add up to 100
    wxPoint adv = SnowstormVector(7);
    int i0 = ssItem.idx % 7 <= 4 ? 0 : cnt;
    int r=rand() % 100;
    for(int i=0, val=0; i < cnt; i++)
    {
        val+=arr[i0+i];
        if (r < val)
        {
            adv=SnowstormVector(i);
            break;
        }
    }
    if (ssItem.idx % 3 == 0)
    {
        adv.x *= 2;
        adv.y *= 2;
    }
    wxPoint xy=ssItem.points.back()+adv;
    xy.x %= BufferWi;
    xy.y %= BufferHt;
    if (xy.x < 0) xy.x+=BufferWi;
    if (xy.y < 0) xy.y+=BufferHt;
    ssItem.points.push_back(xy);
}

void RgbEffects::RenderSnowstorm(int Count, int TailLength)
{
    // create new meteors
    wxImage::HSVValue hsv,hsv0,hsv1;
    palette.GetHSV(0,hsv0);
    palette.GetHSV(1,hsv1);
    SnowstormClass ssItem;
    wxPoint xy;
    int r;

    if (state == 0 || Count != LastSnowstormCount)
    {
        // create snowstorm elements
        LastSnowstormCount=Count;
        SnowstormItems.clear();
        for(int i=0; i<Count; i++)
        {
            ssItem.idx=i;
            ssItem.ssDecay=0;
            ssItem.points.clear();
            SetRangeColor(hsv0,hsv1,ssItem.hsv);
            // start in a random state
            r=rand() % (2*TailLength);
            if (r > 0)
            {
                xy.x=rand() % BufferWi;
                xy.y=rand() % BufferHt;
                ssItem.points.push_back(xy);
            }
            if (r >= TailLength)
            {
                ssItem.ssDecay = r - TailLength;
                r = TailLength;
            }
            for (int j=1; j < r; j++)
            {
                SnowstormAdvance(ssItem);
            }
            SnowstormItems.push_back(ssItem);
        }
    }

    // render Snowstorm Items
    int sz;
    int cnt=0;
    for (SnowstormList::iterator it=SnowstormItems.begin(); it!=SnowstormItems.end(); ++it)
    {
        if (it->points.size() > TailLength)
        {
            if (it->ssDecay > TailLength)
            {
                it->points.clear();  // start over
                it->ssDecay=0;
            }
            else if (rand() % 20 < speed)
            {
                it->ssDecay++;
            }
        }
        if (it->points.empty())
        {
            xy.x=rand() % BufferWi;
            xy.y=rand() % BufferHt;
            it->points.push_back(xy);
        }
        else if (rand() % 20 < speed)
        {
            SnowstormAdvance(*it);
        }
        sz=it->points.size();
        for(int pt=0; pt < sz; pt++)
        {
            hsv=it->hsv;
            hsv.value=1.0 - double(sz - pt + it->ssDecay)/TailLength;
            if (hsv.value < 0.0) hsv.value=0.0;
            SetPixel(it->points[pt].x,it->points[pt].y,hsv);
        }
        cnt++;
    }
}
