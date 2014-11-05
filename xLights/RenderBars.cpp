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
void RgbEffects::RenderBars(wxXmlNode* model,int PaletteRepeat, int Direction, bool Highlight, bool Show3D)
{
    int x,y,n,ColorIdx;
    wxImage::HSVValue hsv;
    size_t colorcnt=GetColorCount();
    int BarCount = PaletteRepeat * colorcnt;
    double position = GetEffectTimeIntervalPosition();
    if(BarCount<1) BarCount=1;


    if (Direction < 4 || Direction == 8 || Direction == 9)
    {
        int BarHt = BufferHt/BarCount+1;
        if(BarHt<1) BarHt=1;
        int HalfHt = BufferHt/2;
        int BlockHt=colorcnt * BarHt;
        if(BlockHt<1) BlockHt=1;
        int f_offset = (fitToTime)?position*BlockHt: state/4 % BlockHt;
        f_offset = Direction == 8 || Direction == 9 ? (state/20)*BarHt: f_offset;
        Direction = Direction > 4?Direction-8:Direction;

        for (y=0; y<BufferHt; y++)
        {
            n=y+f_offset;
            ColorIdx=(n % BlockHt) / BarHt;
            palette.GetHSV(ColorIdx, hsv);
            if (Highlight && n % BarHt == 0) hsv.saturation=0.0;
            if (Show3D) hsv.value *= double(BarHt - n%BarHt - 1) / BarHt;
            switch (Direction)
            {
            case 1:
                // down
                for (x=0; x<BufferWi; x++)
                {
                    SetPixel(x,y,hsv);
                }
                break;
            case 2:
                // expand
                if (y <= HalfHt) {
                    for (x=0; x<BufferWi; x++)
                    {
                        SetPixel(x,y,hsv);
                        SetPixel(x,BufferHt-y-1,hsv);
                    }
                }
                break;
            case 3:
                // compress
                if (y >= HalfHt) {
                    for (x=0; x<BufferWi; x++)
                    {
                        SetPixel(x,y,hsv);
                        SetPixel(x,BufferHt-y-1,hsv);
                    }
                }
                break;
            default:
                // up
                for (x=0; x<BufferWi; x++)
                {
                    SetPixel(x,BufferHt-y-1,hsv);
                }
                break;
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
        int f_offset = (fitToTime)?position*BlockWi:state/4 % BlockWi;
        f_offset = Direction > 9 ? (state/20)*BarWi: f_offset;
        Direction = Direction > 9?Direction-6:Direction;
        for (x=0; x<BufferWi; x++)
        {
            n=x+f_offset;
            ColorIdx=(n % BlockWi) / BarWi;
            palette.GetHSV(ColorIdx, hsv);
            if (Highlight && n % BarWi == 0) hsv.saturation=0.0;
            if (Show3D) hsv.value *= double(BarWi - n%BarWi - 1) / BarWi;
            switch (Direction)
            {
            case 5:
                // right
                for (y=0; y<BufferHt; y++)
                {
                    SetPixel(BufferWi-x-1,y,hsv);
                }
                break;
            case 6:
                // H-expand
                if (x <= HalfWi) {
                    for (y=0; y<BufferHt; y++)
                    {
                        SetPixel(x,y,hsv);
                        SetPixel(BufferWi-x-1,y,hsv);
                    }
                }
                break;
            case 7:
                // H-compress
                if (x >= HalfWi) {
                    for (y=0; y<BufferHt; y++)
                    {
                        SetPixel(x,y,hsv);
                        SetPixel(BufferWi-x-1,y,hsv);
                    }
                }
                break;
            default:
                // left
                for (y=0; y<BufferHt; y++)
                {
                    SetPixel(x,y,hsv);
                }
                break;
            }
        }
    }
}
