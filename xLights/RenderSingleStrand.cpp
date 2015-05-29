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


int mapX(int x, int max, int direction, int &second) {
    second = -1;
    switch (direction) {
        case 0: //
            return x;
        case 1:
            return max - x - 1;
        case 2:
            second = max + x;
            return max - x - 1;
        case 3:
            second = max * 2 - x;
            return x;
    }
    return -1;
}

int mapDirection(const wxString & d) {
    if ("Left" == d) {
        return 1;
    }
    if ("Right" == d) {
        return 0;
    }
    if ("From Middle" == d) {
        return 2;
    }
    if ("To Middle" == d) {
        return 3;
    }

    return 0;
}

void RgbEffects::RenderSingleStrandSkips(int Skips_BandSize, int Skips_SkipSize, int Skips_StartPos, const wxString & Skips_Direction, int advances)
{
    int x = Skips_StartPos - 1;
    xlColour color;
    int second = 0;
    int max = BufferWi;
    int direction = mapDirection(Skips_Direction);
    if (direction > 1) {
        max /= 2;
    }

    size_t colorcnt = GetColorCount();
    double position = GetEffectTimeIntervalPosition();
    position = position * (advances + 0.99);
    x += position * Skips_BandSize;
    while (x > max) {
        x -= (Skips_BandSize +  Skips_SkipSize) * colorcnt;
    }

    int firstX = x;
    int colorIdx = 0;

    //printf("%d   %d\n", curPeriod, x);

    while (x < max) {
        palette.GetColor(colorIdx, color);
        colorIdx++;
        if (colorIdx >= colorcnt) {
            colorIdx = 0;
        }
        for (int cnt = 0; cnt < Skips_BandSize && x < max; cnt++) {
            int mappedX = mapX(x, max, direction, second);
            if (mappedX >= 0 && mappedX < BufferWi) {
                for (int y = 0; y < BufferHt; y++) {
                    SetPixel(mappedX, y, color);
                }
            }
            if (second >= 0 && second < BufferWi) {
                for (int y = 0; y < BufferHt; y++) {
                    SetPixel(second, y, color);
                }
            }
            x++;
        }
        x += Skips_SkipSize;
    }
    colorIdx = GetColorCount() - 1;
    x = firstX - 1;
    while (x >= 0) {
        x -= Skips_SkipSize;

        palette.GetColor(colorIdx, color);
        colorIdx--;
        if (colorIdx < 0) {
            colorIdx = GetColorCount() - 1;
        }
        for (int cnt = 0; cnt < Skips_BandSize && x >= 0; cnt++) {
            int mappedX = mapX(x, max, direction, second);
            if (mappedX >= 0 && mappedX < BufferWi) {
                for (int y = 0; y < BufferHt; y++) {
                    SetPixel(mappedX, y, color);
                }
            }
            if (second >= 0 && second < BufferWi) {
                for (int y = 0; y < BufferHt; y++) {
                    SetPixel(second, y, color);
                }
            }
            x--;
        }
    }
}
void RgbEffects::RenderSingleStrandChase(int ColorScheme,int Number_Chases, int Color_Mix1,
                                    int Chase_Type1,bool Chase_Fade3d1,bool Chase_Group_All,
                                    float chaseSpeed)
{

    int x,x1,y,i,chases,width,slow_state;
    int x2=0;
    int mod_ChaseDirection;
    int MaxNodes;
    int Dual_Chases = 0;
    float dx;

    bool R_TO_L1 = 0;

    wxImage::HSVValue hsv; //   we will define an hsv color model. The RGB colot model would have been "wxColour color;"

    size_t colorcnt=GetColorCount(); // global now set to how many colors have been picked
    y=BufferHt;
    i=0;

    int curEffStartPer, curEffEndPer;

    GetEffectPeriods( curEffStartPer, curEffEndPer);
    double rtval = GetEffectTimeIntervalPosition(chaseSpeed);
    rtval *= 0.99;

    if(Chase_Group_All || Chase_Type1==3) MaxNodes= BufferWi*BufferHt;
    else MaxNodes=BufferWi;

    int MaxChase=MaxNodes*(Color_Mix1/100.0);
    if(MaxChase<1) MaxChase=1;
    
    int nodes_per_color = int(MaxChase/colorcnt);
    if(nodes_per_color<1)  nodes_per_color=1;    //  fill chase buffer

    hsv.value=0.0;
    hsv.saturation=1.0;
    hsv.hue=0.0;


    int AutoReverse=0;
    if(curPeriod==curEffStartPer) ChaseDirection=0; // initialize it once at the beggining of this sequence.
    switch (Chase_Type1)
    {
    case 0: // "Normal. L-R"
        R_TO_L1=1;
        if(curPeriod==curEffStartPer) ChaseDirection=1; // initialize it once at the beggining of this sequence.
        break;

    case 1: // "Normal. R-L"
        R_TO_L1=0;
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
        slow_state = width * rtval;

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
    float  orig_v;
    int new_x,i,max_chase_width,pixels_per_chase;
    size_t colorcnt=GetColorCount();
    int ColorIdx;

    orig_v=hsv.value;
    SetPixel(x,y,hsv); // Turn pixel on

    max_chase_width = width * Chase_Width/100.0;
    pixels_per_chase = width/Number_Chases;

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
        for (i=0; i<max_chase_width; i++)
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
            if(i<pixels_per_chase) // as long as the chase fits, keep drawing it
            {
                if(ColorScheme==0)
                    SetPixel(new_x,y,hsv); // Turn pixel on
                else
                {
                    if(colorcnt==1)
                        ColorIdx=0;
                    else
                    {
                        ColorIdx=round(((double)(i*colorcnt))/max_chase_width);
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

