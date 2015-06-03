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

#include "Effect.h"

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

void RgbEffects::RenderSingleStrandSkips(Effect *eff, int Skips_BandSize, int Skips_SkipSize, int Skips_StartPos,
                                         const wxString & Skips_Direction, int advances)
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
    
    if (needToInit) {
        wxMutexLocker lock(eff->GetBackgroundDisplayList().lock);
        int rects = (Skips_SkipSize + Skips_BandSize) * (curEffEndPer - curEffStartPer + 1);
        eff->GetBackgroundDisplayList().resize(rects * 4);
    }

    int firstX = x;
    int colorIdx = 0;

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
    
    max = Skips_SkipSize + Skips_BandSize - 1;
    if (max >=  BufferWi) {
        max = BufferWi - 1;
    }
    CopyPixelsToDisplayListX(eff, 0, 0, max);
}

int mapChaseType(const wxString &Chase_Type) {
    if ("Left-Right" == Chase_Type) {
        return 0;
    }
    if ("Right-Left" == Chase_Type) {
        return 1;
    }
    if ("Bounce from Left" == Chase_Type) {
        return 2;
    }
    if ("Bounce from Right" == Chase_Type) {
        return 3;
    }
    if ("Dual Bounce" == Chase_Type) {
        return 4;
    }
    return 0;
}
void RgbEffects::RenderSingleStrandChase(const wxString & ColorSchemeName,int Number_Chases, int chaseSize,
                                    const wxString &Chase_Type1,
                                    bool Chase_Fade3d1,bool Chase_Group_All,
                                    float chaseSpeed)
{

    int x,y,i,width;
    int MaxNodes;
    int Dual_Chases = 0;
    float dx;
    
    int ColorScheme = "Palette" == ColorSchemeName;

    bool R_TO_L1 = 0;

    int chaseType = mapChaseType(Chase_Type1);
    
    size_t colorcnt=GetColorCount(); // global now set to how many colors have been picked
    y=BufferHt;
    i=0;

    int curEffStartPer, curEffEndPer;

    GetEffectPeriods( curEffStartPer, curEffEndPer);

    if (Chase_Group_All) MaxNodes= BufferWi*BufferHt;
    else MaxNodes=BufferWi;

    int MaxChase=MaxNodes*(chaseSize/100.0);
    if(MaxChase<1) MaxChase=1;
    
    int nodes_per_color = int(MaxChase/colorcnt);
    if(nodes_per_color<1)  nodes_per_color=1;    //  fill chase buffer


    int AutoReverse=0;
    if (needToInit) {
        needToInit = false;
        ChaseDirection = chaseType == 0 || chaseType == 2; // initialize it once at the beggining of this sequence.
    }
    switch (chaseType)
    {
    case 0: // "Normal. L-R"
        R_TO_L1=1;
        break;
    case 1: // "Normal. R-L"
        R_TO_L1=0;
        break;
    case 2: // "Auto reverse l-r"
        R_TO_L1=1;
        AutoReverse=1;
        break;
    case 3: // "Auto reverse r-l"
        AutoReverse=1;
        break;
    case 4: // "Bounce"
        Dual_Chases=1;
        break;
    }
    
    //double rtval = GetEffectTimeIntervalPosition(chaseSpeed) * (AutoReverse ? 1.999 : 0.999);
    if (Chase_Group_All) {
        width=MaxNodes;
    } else {
        width=BufferWi;
    }
    double chaseOffset = width * chaseSize / 100.0 - 1;

    double rtval;
    if (AutoReverse) {
        rtval = (double)(curPeriod-curEffStartPer)/(double)(curEffEndPer-curEffStartPer);
        rtval *= chaseSpeed;
        while (rtval > 1.0) {
            rtval -= 1.0;
        }
        rtval *= 2.0;
    } else {
        rtval = (double)(curPeriod-curEffStartPer)/(double)(curEffEndPer-curEffStartPer + (Number_Chases == 1 ? 1 : 0));
        rtval *= chaseSpeed;
        while (rtval > 1.0) {
            rtval -= 1.0;
        }
    }

    
    if (Number_Chases < 1) Number_Chases=1;
    if (ColorScheme < 0) ColorScheme=0;
    dx = double(width)/double(Number_Chases);
    if(dx<1) dx=1.0;

    double startState = (width + width * chaseSize / 100.0 - 1) * rtval;
    if (Number_Chases > 1) {
        startState = width * rtval;
    }
    if (chaseOffset < 0) {
        chaseOffset = 0;
        startState = (width - 1) * rtval;
    }
    for(int chase=0; chase<Number_Chases; chase++)
    {
        if (AutoReverse) {
            x = chase*dx + width*rtval - width * chaseSize / 200.0;
        } else {
            double x1 = chase*dx + startState - chaseOffset; // L-R
            int maxChaseWid = (width * chaseSize/100.0);
            if (round(chaseOffset) == maxChaseWid) {
                x1 = chase*dx + startState - trunc(chaseOffset);
            }
            x = std::round(x1);
        }
        
        draw_chase(x, Chase_Group_All, ColorScheme,Number_Chases,AutoReverse, width,chaseSize,Chase_Fade3d1,ChaseDirection); // Turn pixel on
        if(Dual_Chases) {
            draw_chase(x, Chase_Group_All,ColorScheme,Number_Chases,AutoReverse,width,chaseSize,Chase_Fade3d1,!ChaseDirection); //
        }
    }
}

void RgbEffects::draw_chase(int x, bool GroupAll,
                            int ColorScheme,
                            int Number_Chases,
                            bool AutoReverse,
                            int width,
                            int Chase_Width,
                            bool Chase_Fade3d1,
                            int ChaseDirection)
{
    float  orig_v;
    int new_x,i,max_chase_width,pixels_per_chase;
    size_t colorcnt=GetColorCount();
    int ColorIdx;

    max_chase_width = width * Chase_Width/100.0;
    if (max_chase_width < 1) {
        max_chase_width = 1;
    }
    pixels_per_chase = width/Number_Chases;
    if (pixels_per_chase < 1) {
        pixels_per_chase = 1;
    }

    wxImage::HSVValue hsv;
    palette.GetHSV(0, hsv);
    orig_v = hsv.value;
    xlColor color;
    
    
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
            if(ColorScheme==0) {
                if (max_chase_width) hsv.hue = 1.0 - (i*1.0/max_chase_width); // rainbow hue
                color = hsv;
            }
            new_x = x+i;
            
            if (AutoReverse) {
                int tmpx = new_x;
                
                if (tmpx < 0 || tmpx >= width) {
                    int dif = - tmpx;
                    int dir = 1;
                    if (tmpx < 0) {
                        tmpx = -1;
                    } else {
                        dif = tmpx - width + 1;
                        tmpx = width;
                        dir = -1;
                    }
                    while (dif) {
                        dif--;
                        tmpx += dir;
                        if (tmpx == (width - 1)) {
                            dir = -1;
                        }
                        if (tmpx == 0) {
                            dir = 1;
                        }
                    }
                }
                new_x = tmpx;
            } else if (Number_Chases > 1) {
                while (new_x < 0) {
                    new_x += width;
                }
                while (new_x >= width) {
                    new_x -= width;
                }
            }

            //new_x=new_x%BufferWi;
            if(i < pixels_per_chase) // as long as the chase fits, keep drawing it
            {
                if (ColorScheme != 0) {
                    if (colorcnt==1) {
                        ColorIdx=0;
                    } else {
                        ColorIdx=((double)((max_chase_width - i + 1)*colorcnt))/max_chase_width;
                    }
                    if (ColorIdx >= colorcnt) ColorIdx=colorcnt-1;
                    if (Chase_Fade3d1) {
                        if (allowAlpha) {
                            palette.GetColor(ColorIdx, color);
                            color.alpha = 255.0 * (i + 1.0)/max_chase_width;
                        } else {
                            palette.GetHSV(ColorIdx, hsv);
                            hsv.value=orig_v - ((max_chase_width - (i + 1.0))/max_chase_width); // fades data down over chase width
                            if (hsv.value<0.0) hsv.value=0.0;
                            color = hsv;
                        }
                    } else {
                        palette.GetColor(ColorIdx, color);
                    }
                    
                }
                if (ChaseDirection == 0) {// are we going R-L?
                    new_x = width - new_x - 1;
                }
                if (GroupAll) {
                    int y = 0;
                    while (new_x > BufferWi) {
                        y++;
                        new_x -= BufferWi;
                    }
                    if (Chase_Fade3d1) {
                        xlColor c;
                        GetPixel(new_x, y, c);
                        if (c != xlBLACK) {
                            int a = color.alpha;
                            color = color.AlphaBlend(c);
                            color.alpha = c.alpha > a ? c.alpha : a;
                        }
                    }
                    SetPixel(new_x,y,color); // Turn pixel on
                } else {
                    if (Chase_Fade3d1) {
                        xlColor c;
                        GetPixel(new_x, 0, c);
                        if (c != xlBLACK) {
                            int a = color.alpha;
                            color = color.AlphaBlend(c);
                            color.alpha = c.alpha > a ? c.alpha : a;
                        }
                    }
                    
                    for (int y=0; y<BufferHt; y++) {
                        SetPixel(new_x,y,color); // Turn pixel on
                    }
                }

            }
        }
    }
}

