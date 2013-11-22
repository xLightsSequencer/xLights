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

void RgbEffects::RenderSingleStrand(int Color_Mix1,int Chase_Spacing1,int Chase_Type1,bool Group_Arches1, bool R_TO_L1,
                                    int Color_Mix2,int Chase_Spacing2,int Chase_Type2,bool Group_Arches2, bool R_TO_L2)
{

    int x,x1,x0,y,i,maxx,ColorIdx;
    int color_index,x1_mod;
    int MaxNodes,xt;
    int xstart,xend,xstart_group;

    if(Group_Arches1) srand (time(NULL)); // for strobe effect, make lights be random
    else srand(1); // else always have the same random numbers for each frame (state)
    wxImage::HSVValue hsv; //   we will define an hsv color model. The RGB colot model would have been "wxColour color;"

    size_t colorcnt=GetColorCount(); // global now set to how many colors have been picked
    y=BufferHt;
    i=0;
    int state_width = state/BufferWi;
    int state_col = (state_width*BufferWi) + state%10;
    int state_ht = state_width%BufferHt; // 0 .. (BufferHt-1)

    int curEffStartPer, curEffEndPer,  nextEffTimePeriod;

    GetEffectPeriods( curEffStartPer, nextEffTimePeriod, curEffEndPer);
    double rtval = GetEffectTimeIntervalPosition();


    if(Chase_Spacing1<1) Chase_Spacing1=1;
    int ymax=BufferHt;
    int chase_buffer[1000];
    float brightness[1000];
    for(x=0; x<1000; x++) // fill up chase buffer with invalid ColorIndex value. Or in other words, intialize array
    {
        chase_buffer[x]=-1;
        brightness[x1] =1;
    }
    if(Group_Arches1) MaxNodes= BufferWi*BufferHt;
    else MaxNodes=BufferWi*2;

    int MaxChase=MaxNodes*(Color_Mix1/100.0);
    if(MaxChase<1) MaxChase=1;

    int nodes_per_color = int(MaxChase/colorcnt);
    if(nodes_per_color<1)  nodes_per_color=1;    //  fill chase buffer
    for (x1=0; x1<MaxChase; x1++) // fill up chase buffer with pattern we will be using to scroll across arches
    {
        color_index = int(x1/nodes_per_color);
        chase_buffer[x1]=color_index;
        brightness[x1] = 1.0-(x1%nodes_per_color)/(nodes_per_color*1.0);
    }

    hsv.value=0.0;
    hsv.saturation=1.0;
    hsv.hue=0.0;


    xstart = state % BufferWi;
    xstart = (int)(state/4) % MaxNodes; // divide by 4 slows down chase
    xstart_group = (int)(state/4) / MaxNodes; // divide by 4 slows down chase
    if(xstart_group%2==1)
        R_TO_L1=1;
    else
        R_TO_L1=0;
    xend = xstart+BufferHt;
    x0=xstart;
    int FIT_TO_TIME=0;
    if(FIT_TO_TIME==0)
        for(y=0; y<BufferHt; y++)
        {
            for(x1=0; x1<MaxChase; x1++)
            {
                xt=x0-x1;
                if(R_TO_L1) xt=MaxNodes-x0-x1;
                if(xt<0) xt+=MaxNodes; // was xt=0;
                //  y=int (xt/BufferWi);
                x=xt%BufferWi;
                ColorIdx=chase_buffer[x1];
                if(R_TO_L1)   ColorIdx=chase_buffer[MaxChase-x1];
                if(ColorIdx>=0)
                {
                    palette.GetHSV(ColorIdx, hsv); // Now go and get the hsv value for this ColorIdx
                    if(R_TO_L1)
                        hsv.value=1.0-brightness[x1];
                    else
                        hsv.value=brightness[x1];

                    x1_mod=x1%Chase_Spacing1;
                    if(x>=BufferWi or x<0 or (Chase_Spacing1>1 and x1_mod != 1))
                    {
                        hsv.value=0.0;
                        hsv.saturation=1.0;
                        hsv.hue=0.0;
                    }
                    SetPixel(x,y,hsv); // Turn pixel on
                }
            }
        }

    if(FIT_TO_TIME==1)
    {


        if(Group_Arches1)
        {
            for(x1=0; x1<MaxChase; x1++)
            {
                xt=x0-x1;
                if(R_TO_L1) xt=MaxNodes-x0-x1;
                if(xt<0) xt=0;
                y=int (xt/BufferWi);
                x=xt%BufferWi;
                ColorIdx=chase_buffer[x1];
                if(ColorIdx>=0) palette.GetHSV(ColorIdx, hsv); // Now go and get the hsv value for this ColorIdx
                x1_mod=x1%Chase_Spacing1;
                if(x>=BufferWi or x<0 or (Chase_Spacing1>1 and x1_mod != 1))
                {
                    hsv.value=0.0;
                    hsv.saturation=1.0;
                    hsv.hue=0.0;
                }
                SetPixel(x,y,hsv); // Turn pixel on
            }
        }
        else
        {
            for(y=0; y<BufferHt; y++)
            {
                for(x1=0; x1<MaxChase; x1++)
                {
                    xt=x0-x1;
                    if(R_TO_L1) xt=MaxNodes-x0-x1;
                    if(xt<0) xt=0;
                    x=xt%BufferWi;
                    ColorIdx=chase_buffer[x1];
                    if(ColorIdx>=0) palette.GetHSV(ColorIdx, hsv); // Now go and get the hsv value for this ColorIdx
                    x1_mod=x1%Chase_Spacing1;
                    if(x>=BufferWi or x<0 or (Chase_Spacing1>1 and x1_mod != 0  and x1_mod != 1))
                    {
                        hsv.value=0.0;
                        hsv.saturation=1.0;
                        hsv.hue=0.0;
                    }
                    SetPixel(x,y,hsv); // Turn pixel on
                }
            }
        }
    }
}
