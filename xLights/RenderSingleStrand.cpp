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

void RgbEffects::RenderSingleStrand(int Color_Mix1,int Chase_Spacing1,int Chase_Type1,bool Chase_Fade3d1)
{

    int x,x1,x0,y,i,maxx,ColorIdx;
    int x2=0;
    int x_2=0;
    int x2_mod=0;
    int color_index,x1_mod;
    int MaxNodes,xt,Dual_Chases=0;
    int start1,start2,start1_mid,xend,start1_group;
 bool R_TO_L1;
    //srand (time(NULL)); // for strobe effect, make lights be random
    srand(1); // else always have the same random numbers for each frame (state)
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
        brightness[x] =1;
    }
    if(Chase_Type1==3) MaxNodes= BufferWi*BufferHt;
    else MaxNodes=BufferWi;

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

    /*
        Chase_Type1:

      SingleStrandTypes.Add("Normal. L-R");
      SingleStrandTypes.Add("Normal. R-L");
      SingleStrandTypes.Add("Auto reverse");
      SingleStrandTypes.Add("Bounce");

        */
    int Fade3D,AutoReverse=0;
    start1 = state % BufferWi;
    start1 = (int)(state/4) % MaxNodes; // divide by 4 slows down chase
    start1_group = (int)(state/4) / MaxNodes; // divide by 4 slows down chase
    start2 = MaxNodes-start1;
    start1_mid = MaxNodes/2;

    switch (Chase_Type1)
    {
    case 0: // "Normal. L-R"
        R_TO_L1=0;
        break;

    case 1: // "Normal. R-L"
        R_TO_L1=1;
        break;

    case 2: // "Auto reverse"
        AutoReverse=1;
        break;

    case 3: // "Bounce"
        Dual_Chases=1;
        break;

    }

    if(AutoReverse==1)
    {
        if(start1_group%2==1)
            R_TO_L1=1;
        else
            R_TO_L1=0;
    }
    xend = start1+BufferHt;
    x0=start1;
    int FIT_TO_TIME=0;
    if(FIT_TO_TIME==0)
        for(y=0; y<BufferHt; y++)
        {
            for(x1=0; x1<MaxChase; x1++)
            {
                xt=x0-x1;
                if(R_TO_L1) xt=MaxNodes-x0-x1;
                if(Dual_Chases==1) x2=MaxNodes-x0-x1;
                if(xt<0) xt+=MaxNodes; // was xt=0;
                if(x2<0) x2+=MaxNodes; // was xt=0;
                //  y=int (xt/BufferWi);
                x=xt%BufferWi;
                x_2=x2%BufferWi;
                ColorIdx=chase_buffer[x1];
                if(R_TO_L1)   ColorIdx=chase_buffer[MaxChase-x1];
                if(ColorIdx>=0)
                {
                    palette.GetHSV(ColorIdx, hsv); // Now go and get the hsv value for this ColorIdx
                    if(Chase_Fade3d1==1)
                    {
                        if(R_TO_L1)
                            hsv.value=1.0-brightness[x1];
                        else
                            hsv.value=brightness[x1];
                    }
                    x1_mod=x1%Chase_Spacing1;
                    if(x>=BufferWi or x<0 or (Chase_Spacing1>1 and x1_mod != 1))
                    {
                        hsv.value=0.0;
                        hsv.saturation=1.0;
                        hsv.hue=0.0;
                    }
                    SetPixel(x,y,hsv); // Turn pixel on

                    if(Dual_Chases==1)
                    {


                        x2_mod=x_2%Chase_Spacing1;
                        if(x>=BufferWi or x<0 or (Chase_Spacing1>1 and x2_mod != 1))
                        {
                            hsv.value=0.0;
                            hsv.saturation=1.0;
                            hsv.hue=0.0;
                        }
                        SetPixel(x_2,y,hsv); // Turn pixel on
                    }
                }
            }
        }

    if(FIT_TO_TIME==1)
    {


        if(Chase_Fade3d1)
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
