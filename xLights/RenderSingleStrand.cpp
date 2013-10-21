/***************************************************************
 * Name:      RenderSingleStrand.cpp
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

void RgbEffects::RenderSingleStrand(int Color_Mix1,int Color_Spacing1,bool Group_Arches1,
                                    int Color_Mix2,int Color_Spacing2,bool Group_Arches2)
{

    int x,x1,y,i,i7,ColorIdx;
    int lights = (BufferHt*BufferWi)*(Color_Mix1/100.0); // Count is in range of 1-100 from slider bar
    if(lights<1) lights=1;
    int step;
    if(lights>0) step=BufferHt*BufferWi/lights;
    else step=1;

    //  size_t NodeCount=GetNodeCount();

    int max_modulo;
    max_modulo=Color_Spacing1;
    if(max_modulo<2) max_modulo=2;  // scm  could we be getting 0 passed in?
    int max_modulo2=max_modulo/2;
    if(max_modulo2<1) max_modulo2=1;

    if(step<1) step=1;
    if(Group_Arches1) srand (time(NULL)); // for strobe effect, make lights be random
    else srand(1); // else always have the same random numbers for each frame (state)
    wxImage::HSVValue hsv; //   we will define an hsv color model. The RGB colot model would have been "wxColour color;"

    size_t colorcnt=GetColorCount(); // global now set to how many colors have been picked
    y=BufferHt;

    i=0;
    int state_width = state/BufferWi;
    int state_col = (state_width*BufferWi) + state%10;
    int state_ht = state_width%BufferHt; // 0 .. (BufferHt-1)
    int max_width = BufferWi * (Color_Mix1/100.0);
    int max_width2 = max_width/2;
    int max_width3 = max_width/3;
    if(max_width<1) max_width=1;
    if(max_width2<1) max_width2=1;
    if(max_width3<1) max_width3=1;
//  curEffStartPer = startMsec/XTIMER_INTERVAL;
//    curEffEndPer = endMsec/XTIMER_INTERVAL;
//    nextEffTimePeriod = nextMsec/XTIMER_INTERVAL;
    int curEffStartPer;
    int curEffEndPer;
    int nextEffTimePeriod;
    GetEffectPeriods( curEffStartPer, nextEffTimePeriod, curEffEndPer);



    if(Color_Spacing1<1) Color_Spacing1=1;
    for (y=0; y<BufferHt; y++) // For my 20x120 megatree, BufferHt=120
    {
       /*
        for (x1=0; x1<BufferWi; x1++)
        {
            ColorIdx=-1;
            hsv.value=0.0;
            hsv.saturation=1.0;
            hsv.hue=0.0;
            x=(state/10)+x1;
            if(x>BufferWi) x=x%BufferWi;
            if(x<max_width3 and colorcnt==3)
                ColorIdx=2;
            else if(x<max_width2 and colorcnt==2)
                ColorIdx=1;
            else  if(x<max_width)
                ColorIdx=0;
            if(Color_Spacing1>1 and x1%Color_Spacing1>0) ColorIdx=-1;
            if(ColorIdx>=0) palette.GetHSV(ColorIdx, hsv); // Now go and get the hsv value for this ColorIdx


            SetPixel(x,y,hsv); // Turn pixel on
        }
        */

        x= nextEffTimePeriod-curEffStartPer;
           ColorIdx=0;

            if(ColorIdx>=0) palette.GetHSV(ColorIdx, hsv); // Now go and get the hsv value for this ColorIdx

         SetPixel(x,y,hsv); // Turn pixel on
    }
}
