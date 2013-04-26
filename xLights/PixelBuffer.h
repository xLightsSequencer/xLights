/***************************************************************
 * Name:      PixelBuffer.h
 * Purpose:   Implements pixel buffer and effects
 * Author:    Matt Brown (dowdybrown@yahoo.com)
 * Created:   2012-10-21
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

#ifndef PIXELBUFFER_H
#define PIXELBUFFER_H

#include <wx/xml/xml.h>

#include "ModelClass.h"
#include "RgbEffects.h"

enum MixTypes
{
    Mix_Effect1,
    Mix_Effect2,
    Mix_Mask1,
    Mix_Mask2,
    Mix_Unmask1,
    Mix_Unmask2,
    Mix_Layered,
    Mix_Average,
};


class PixelBufferClass : public ModelClass
{
private:

    int sparkle_count;
    int CurrentLayer;  // 0 or 1
    MixTypes MixType;
    RgbEffects Effect[2];
    void GetMixedColor(wxCoord x, wxCoord y, wxColour& c);

public:
    PixelBufferClass();
    ~PixelBufferClass();
    void InitBuffer(wxXmlNode* ModelNode);
    void Clear();
    size_t GetColorCount(int layer);
    void SetMixType(const wxString& MixName);
    void SetPalette(int layer, wxColourVector& newcolors);
    void SetLayer(int newlayer, int period, int speed, bool ResetState);
    void SetSparkle(int freq);
    void RenderBars(int PaletteRepeat, int Direction, bool Highlight, bool Show3D);
    void RenderButterfly(int ColorScheme, int Style, int Chunks, int Skip);
    void RenderColorWash(bool HorizFade, bool VertFade, int RepeatCount);
    void RenderFire(int HeightPct);
    void RenderGarlands(int GarlandType, int Spacing);
    void RenderLife(int Count, int Seed);
    void RenderMeteors(int MeteorType, int Count, int Length);
    void RenderPictures(int dir, const wxString& NewPictureName);
    void RenderSnowflakes(int Count, int SnowflakeType);
    void RenderSnowstorm(int Count, int Length);
    void RenderSpirals(int PaletteRepeat, int Direction, int Rotation, int Thickness, bool Blend, bool Show3D);
    void RenderText(int Top, const wxString& Line1, const wxString& Line2, const wxString& FontString, int dir);
    void RenderTwinkle(int Count);
    void RenderTree(int Branches);
    void RenderSpirograph(int R, int r, int d, bool Animate);
    void RenderFireworks(int Number_Explosions,int Count,float Velocity,int Fade);
    void CalcOutput();
};

#endif // PIXELBUFFER_H
