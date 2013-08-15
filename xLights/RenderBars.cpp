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

// Direction: 0=up, 1=down, 2=expand, 3=compress, 4=left, 5=right, 6=H-expand, 7=H-compress
void RgbEffects::RenderBars(int PaletteRepeat, int Direction, bool Highlight, bool Show3D)
{
    int x,y,n,pixel_ratio,ColorIdx;
    bool IsMovingDown,IsMovingLeft,IsHighlightRow;
    wxImage::HSVValue hsv;
    size_t colorcnt=GetColorCount();
    int BarCount = PaletteRepeat * colorcnt;
    if(BarCount<1) BarCount=1;
    if (Direction < 4)
    {
        int BarHt = BufferHt/BarCount+1;
        if(BarHt<1) BarHt=1;
        int HalfHt = BufferHt/2;
        int BlockHt=colorcnt * BarHt;
        if(BlockHt<1) BlockHt=1;
        int f_offset = state/4 % BlockHt;
        for (y=0; y<BufferHt; y++)
        {
            switch (Direction)
            {
            case 1:
                IsMovingDown=true;
                break;
            case 2:
                IsMovingDown=(y <= HalfHt);
                break;
            case 3:
                IsMovingDown=(y > HalfHt);
                break;
            default:
                IsMovingDown=false;
                break;
            }
            if (IsMovingDown)
            {
                n=y+f_offset;
                pixel_ratio = BarHt - n%BarHt - 1;
                IsHighlightRow=n % BarHt == 0;
            }
            else
            {
                n=y-f_offset+BlockHt;
                pixel_ratio = n%BarHt;
                IsHighlightRow=(n % BarHt == BarHt-1); // || (y == BufferHt-1);
            }
            ColorIdx=(n % BlockHt) / BarHt;
            palette.GetHSV(ColorIdx, hsv);
            if (Highlight && IsHighlightRow) hsv.saturation=0.0;
            if (Show3D) hsv.value *= double(pixel_ratio) / BarHt;
            for (x=0; x<BufferWi; x++)
            {
                SetPixel(x,y,hsv);
            }
        }
    }
    else
    {
        int BarWi = BufferWi/BarCount+1;
        if(BarWi<1) BarWi=1;
        int HalfWi = BufferWi/2;
        int BlockWi=colorcnt * BarWi;
        if(BlockWi<1) BlockWi=1;
        int f_offset = state/4 % BlockWi;
        for (x=0; x<BufferWi; x++)
        {
            switch (Direction)
            {
            case 5:
                IsMovingLeft=false;
                break;
            case 6:
                IsMovingLeft=(x <= HalfWi);
                break;
            case 7:
                IsMovingLeft=(x > HalfWi);
                break;
            default:
                IsMovingLeft=true;
                break;
            }
            if (IsMovingLeft)
            {
                n=x+f_offset;
                pixel_ratio = BarWi - n%BarWi - 1;
                IsHighlightRow=n % BarWi == 0;
            }
            else
            {
                n=x-f_offset+BlockWi;
                pixel_ratio = n%BarWi;
                IsHighlightRow=(n % BarWi == BarWi-1);
            }
            ColorIdx=(n % BlockWi) / BarWi;
            palette.GetHSV(ColorIdx, hsv);
            if (Highlight && IsHighlightRow) hsv.saturation=0.0;
            if (Show3D) hsv.value *= double(pixel_ratio) / BarWi;
            for (y=0; y<BufferHt; y++)
            {
                SetPixel(x,y,hsv);
            }
        }
    }
}
