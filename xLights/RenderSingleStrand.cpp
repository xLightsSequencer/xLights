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

void RgbEffects::RenderSingleStrand(int ColorScheme,int Number_Chases, int Color_Mix1,int Chase_Spacing1,int Chase_Type1,bool Chase_Fade3d1,bool Chase_Group_All)
{

    int x,x1,x0,y,i,ColorIdx,chases,width,slow_state;
    int x2=0;
    int x_2=0;
    int x2_mod=0;
    int color_index,x1_mod,mod_ChaseDirection;
    int MaxNodes,xt,Dual_Chases,All_Arches=0;
    int start1,start2,start1_mid,xend,start1_group;
    float dx;

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
    if(Chase_Group_All || Chase_Type1==3) MaxNodes= BufferWi*BufferHt;
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


    int AutoReverse=0;
    start1 = state % BufferWi;
    start1 = (int)(state/2) % MaxNodes; // divide by 4 slows down chase
    start1_group = (int)(state/2) / MaxNodes; // divide by 4 slows down chase
    start2 = MaxNodes-start1;
    start1_mid = MaxNodes/2;
    if(state==0) ChaseDirection=0; // initialize it once at the beggining of this sequence.
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
    case 4: // "Pacman"

        break;
    }
    //  we have a global variablal, ChaseDirection, which indicates which way we are moving.


    int FIT_TO_TIME=(fitToTime);
    int numberFrames=0;
    if(fitToTime) // is "Fit to Time" checked?
    {
        numberFrames=nextEffTimePeriod-nextEffTimePeriod;
        //   GetEffectPeriods( nextEffTimePeriod, nextEffTimePeriod, curEffEndPer);
//    double rtval = GetEffectTimeIntervalPosition();
//  rtval 0 to 1.0. This indicates how far we are through this row on the grid.
//      0.0 we are just starting the effect on this row
//      1.0 we have come t the end of time for this effect
    }
    hsv.value=1.0;
    hsv.saturation=1.0;
    hsv.hue=0.0; // RED
    if(Chase_Group_All)
    {
        width=MaxNodes;
    }
    else
    {
        width=BufferWi;
    }
    if(Number_Chases<1) Number_Chases=1;
    if(ColorScheme<0) ColorScheme=0;
    dx = width/Number_Chases;
    if(dx<1) dx=1.0;

    for(chases=1; chases<=Number_Chases; chases++)
    {
        if(fitToTime)
            slow_state = width * rtval;
        else
            slow_state = (state/4)%width;

        //   if(R_TO_L1)
        mod_ChaseDirection=ChaseDirection%2;    // 0= R-L, 1=L-R
        if(AutoReverse) // if we are bouncing back and forth
        {
            if(mod_ChaseDirection==1) // which direction should we be going
                x1=int(0.5 + (chases-1)*dx)+slow_state; // L-R
            else
                x1=int(0.5 + width-((chases-1)*dx)-slow_state); // R-L
        }
        else // we are just doing L-R or R-L
        {
            if(R_TO_L1) // 1 = L-R, 0=R-L
                x1=int(0.5 + (chases-1)*dx)+slow_state; // L-R
            else
                x1=int(0.5 + width-((chases-1)*dx)-slow_state); // R-L
        }

        if(x1<=0)
        {
            x1+=width;
            ChaseDirection++;

        }
        else if(x1>=width)
        {
            x1-=width;
            ChaseDirection++;
        }
        if(x1<0) x1=0;
        x=x1%BufferWi;
        if(Chase_Group_All)
        {
            y = (x1%MaxNodes)/BufferWi;
            draw_chase(x,y,hsv,ColorScheme,Number_Chases,width,R_TO_L1,Color_Mix1,Chase_Fade3d1,ChaseDirection); // Turn pixel on
        }
        else
        {
            // NOT A GROUP
            for(y=0; y<BufferHt; y++)
            {
                draw_chase(x,y,hsv,ColorScheme,Number_Chases,width,R_TO_L1,Color_Mix1,Chase_Fade3d1,ChaseDirection); // Turn pixel on
            if(Dual_Chases)
                {
                     if(R_TO_L1) // 1 = L-R, 0=R-L
                        x2=width-x-1;
                    else
                          x2=width-x-1;
                    draw_chase(x2,y,hsv,ColorScheme,Number_Chases,width,R_TO_L1,Color_Mix1,Chase_Fade3d1,ChaseDirection); //
                }
            }

        }
    }
}

void RgbEffects::draw_chase(int x,int y,wxImage::HSVValue hsv,int ColorScheme,int Number_Chases,
                            int width,bool R_TO_L1,int Chase_Width,bool Chase_Fade3d1,
                            int ChaseDirection)
{
    float  orig_v,new_v;
    int new_x,i,max_chase_width,pixels_per_chase;
    wxColour color;
    size_t colorcnt=GetColorCount();
    int ColorIdx;

    orig_v=hsv.value;
    SetPixel(x,y,hsv); // Turn pixel on

    max_chase_width = width * Chase_Width/100.0;
    pixels_per_chase = width/Number_Chases;

    int pulsar=0;
    int n;
    float val;
    int mid = 0.5 + (max_chase_width/2.0);
    int pixels_per_color=max_chase_width/colorcnt;
    if(pixels_per_color<1) pixels_per_color=1;
    /*


    RRRRGGGG........+........................
    .RRRRGGGG.......+........................
    ..RRRRGGGG......+........................
    ...RRRRGGGG.....+........................
    ....RRRRGGGG....+........................
    .....RRRRGGGG...+........................
    ......RRRRGGGG..+........................
    .......RRRRGGGG.+........................
    ........RRRRGGGG+..............<===========   this is what fist version would end at
    .........RRRRGGG+........................
     .........RRRRGG+........................
      .........RRRRG+........................
       .........RRRR+........................
        .........RRR+........................



    */
    if(max_chase_width>=1)
    {
        for (i=0; i<=max_chase_width; i++)
        {
            if(pulsar==1)
            {
                n=state%10;
                switch (n)
                {
                case 0:
                case 4:
                    pulse(x,y,hsv,0.30);
                    break;
                case 1:
                case 3:
                    pulse(x,y,hsv,0.50);
                    pulse(x-1,y,hsv,0.30);
                    pulse(x+1,y,hsv,0.30);

                    break;
                case 2:
                    pulse(x,y,hsv,1.0);
                    pulse(x-1,y,hsv,0.50);
                    pulse(x+1,y,hsv,0.50);
                    pulse(x-2,y,hsv,0.30);
                    pulse(x+2,y,hsv,0.30);
                    break;
                }
            }
            else // not pulsar
            {
                if(ColorScheme==0)
                {
                    if(max_chase_width) hsv.hue = 1.0 - (i*1.0/max_chase_width); // rainbow hue
                }
                //  if(R_TO_L1)
                if(ChaseDirection==1) // are we going R-L?
                    new_x = x-i;    //  yes
                else
                    new_x = x+i;
                if(new_x<0)
                {
                    y++;
                    ChaseDirection=1;   // we were going R to L, now switch to L-R
                    new_x+=width;
                }
                else if(new_x>width)
                {
                    y++;
                    ChaseDirection=0;   // we were going L-R, now switch to R-L
                    new_x-=width;
                }
                //new_x=new_x%BufferWi;
                if(i<=pixels_per_chase) // as long as the chase fits, keep drawing it
                {
                    if(ColorScheme==0)
                        SetPixel(new_x,y,hsv); // Turn pixel on
                    else
                    {
                        if(colorcnt==1)
                            ColorIdx=0;
                        else
                        {
                            ColorIdx=i/pixels_per_color;
                        }
                        if(ColorIdx>=colorcnt) ColorIdx=colorcnt-1;
                        palette.GetHSV(ColorIdx, hsv);
                        if(Chase_Fade3d1) hsv.value=orig_v - (i*1.0/max_chase_width); // fades data down over chase width
                        if(hsv.value<0.0) hsv.value=0.0;
                        SetPixel(new_x,y,hsv); // Turn pixel on
                    }
                }
            }
        }
    }
}
void RgbEffects::pulse(int x,int y,wxImage::HSVValue hsv,float adjust_brightness)
{
    if(x>0)
    {
        hsv.value *= adjust_brightness;
        SetPixel(x,y,hsv);
    }

}
