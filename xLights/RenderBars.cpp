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

void RgbEffects::RenderBars(int PaletteRepeat, int Direction, bool Highlight, bool Show3D)
{
    int x,y,n,pixel_ratio,ColorIdx;
    bool IsMovingDown,IsHighlightRow;
    wxImage::HSVValue hsv;
    size_t colorcnt=GetColorCount();
    int BarCount = PaletteRepeat * colorcnt;
    int BarHt = BufferHt/BarCount+1;
    int HalfHt = BufferHt/2;
    int BlockHt=colorcnt * BarHt;
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
