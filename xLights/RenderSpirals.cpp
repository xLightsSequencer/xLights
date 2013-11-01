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

void RgbEffects::RenderSpirals(int PaletteRepeat, int Direction, int Rotation, int Thickness,
                               bool Blend, bool Show3D, bool grow, bool shrink)
{
    int strand_base,strand,thick,x,y,ColorIdx;
    size_t colorcnt=GetColorCount();
    int SpiralCount=colorcnt * PaletteRepeat;
    int deltaStrands=BufferWi / SpiralCount;
    int SpiralThickness=(deltaStrands * Thickness / 100) + 1;
    int spiralGap = deltaStrands - SpiralThickness;
    long SpiralState;
    long ThicknessState = state/10;
    wxImage::HSVValue hsv;
    wxColour color;


    if (fitToTime)
    {
        double position = GetEffectTimeIntervalPosition();
        if (grow&&shrink)
        {
            ThicknessState = position <= 0.5?spiralGap*(position*2):spiralGap*((1-position) * 2);
        }
        else if (grow)
        {
                ThicknessState = spiralGap *position;
        }
        else if (shrink)
        {
            ThicknessState = spiralGap * (1-position);
        }
        SpiralState = position*BufferWi*10*Direction;
    }
    else
    {
        SpiralState=state*Direction;
    }

    spiralGap += (spiralGap==0);
    if (grow && (!shrink || ((ThicknessState/spiralGap)%2)==0))
    {
        SpiralThickness += ThicknessState%(spiralGap);
    }
    else if (shrink && (!grow || ((ThicknessState/spiralGap)%2)==1))
    {
        SpiralThickness +=spiralGap-ThicknessState%(spiralGap);
    }

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
                    Color2HSV(color,hsv);
                    if (Rotation < 0)
                    {
                        hsv.value*=double(thick+1)/SpiralThickness;
                    }
                    else
                    {
                        hsv.value*=double(SpiralThickness-thick)/SpiralThickness;
                    }
                    SetPixel(x,y,hsv);
                }
                else
                {
                    SetPixel(x,y,color);
                }
            }
        }
    }
}
