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

void RgbEffects::RenderSpirals(int PaletteRepeat, int Movement, int Rotation, int Thickness,
                               bool Blend, bool Show3D, bool grow, bool shrink)
{
    int strand_base,strand,thick,x,y,ColorIdx;
    if (PaletteRepeat == 0) {
        PaletteRepeat = 1;
    }
    size_t colorcnt=GetColorCount();
    int SpiralCount=colorcnt * PaletteRepeat;
    int deltaStrands=BufferWi / SpiralCount;
    int SpiralThickness=(deltaStrands * Thickness / 100) + 1;
    int spiralGap = deltaStrands - SpiralThickness;
    long SpiralState;
    long ThicknessState = 0;
    wxImage::HSVValue hsv;
    xlColour color;

    int Direction = Movement > 0.001 ? 1 : (Movement < -0.001 ? -1 : 0);
    double position = GetEffectTimeIntervalPosition();
    //movement is -200 - 200 representing rrotations of 0 - 20, left/right
    if (position > 0) {
        position *= std::abs(Movement / 10.0);
        while (position > 1.0) {
            position -= 1.0;
        }
    }
    if (grow && shrink)
    {
        ThicknessState = position <= 0.5?spiralGap*(position*2):spiralGap*((1-position) * 2);
    }
    else if (grow)
    {
        ThicknessState = spiralGap * position;
    }
    else if (shrink)
    {
        ThicknessState = spiralGap * (1.0-position);
    }
    SpiralState = position*BufferWi*10*Direction;

    spiralGap += (spiralGap==0);
    SpiralThickness += ThicknessState;

    for(int ns=0; ns < SpiralCount; ns++)
    {
        strand_base=ns * deltaStrands;
        ColorIdx=ns % colorcnt;
        palette.GetColor(ColorIdx,color);
        for(thick=0; thick < SpiralThickness; thick++)
        {
            strand = (strand_base + thick) % BufferWi;
            for(y=0; y < BufferHt; y++)
            {
                x=(strand + SpiralState/10 + y*Rotation/BufferHt) % BufferWi;
                if (x < 0) x += BufferWi;
                if (Blend)
                {
                    GetMultiColorBlend(double(BufferHt-y-1)/double(BufferHt), false, color);
                }
                if (Show3D)
                {
                    double f = 1.0;
                    
                    if (Rotation < 0)
                    {
                        f=double(thick+1)/SpiralThickness;
                    }
                    else
                    {
                        f=double(SpiralThickness-thick)/SpiralThickness;
                    }
                    if (allowAlpha) {
                        xlColor c(color);
                        c.alpha = 255.0 * f;
                        SetPixel(x,y,c);
                    } else {
                        Color2HSV(color,hsv);
                        hsv.value *= f;
                        SetPixel(x,y,hsv);
                    }
                }
                else
                {
                    SetPixel(x,y,color);
                }
            }
        }
    }
}
