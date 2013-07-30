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

/**
 * \brief enumeration of the different techniques used in layering effects
 */

enum MixTypes
{
    Mix_Effect1, /**<  Effect 1 only */
    Mix_Effect2, /**<  Effect 2 only */
    Mix_Mask1,   /**<  Effect 2 color shows where Effect 1 is black */
    Mix_Mask2,   /**<  Effect 1 color shows where Effect 2 is black */
    Mix_Unmask1, /**<  Effect 2 color shows where Effect 1 is not black */
    Mix_Unmask2, /**<  Effect 1 color shows where Effect 2 is black */
    Mix_Layered, /**<  Effect 1 is back ground and shows only when effect 2 is black */
    Mix_Average, /**<  Average color value between effects per pixel */
};


class PixelBufferClass : public ModelClass
{
private:

    int sparkle_count;
    int brightness;
    int contrast;
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
    void SetBrightness(int value);
    void SetContrast(int value);
    void RenderBars(int PaletteRepeat, int Direction, bool Highlight, bool Show3D);
    void RenderButterfly(int ColorScheme, int Style, int Chunks, int Skip);
    void RenderColorWash(bool HorizFade, bool VertFade, int RepeatCount);
    void RenderFire(int HeightPct,int HueShift,bool GrowFire);
    void RenderGarlands(int GarlandType, int Spacing);
    void RenderLife(int Count, int Seed);
    void RenderMeteors(int MeteorType, int Count, int Length, bool FallUp,int MeteorsEffect,int SwirlIntensity);
    void RenderPictures(int dir, const wxString& NewPictureName,int GifSpeed);
    void RenderSnowflakes(int Count, int SnowflakeType);
    void RenderSnowstorm(int Count, int Length);
    void RenderSpirals(int PaletteRepeat, int Direction, int Rotation, int Thickness, bool Blend, bool Show3D);
    void RenderText(int Position1, const wxString& Line1, const wxString& FontString1,int dir1,int Effect1,int Countdown1,
                    int Position2, const wxString& Line2, const wxString& FontString2,int dir2,int Effect2,int Countdown2);
    void RenderTwinkle(int Count,int Steps, bool Strobe);
    void RenderTree(int Branches);
    void RenderSpirograph(int R, int r, int d, bool Animate);
    void RenderFireworks(int Number_Explosions,int Count,float Velocity,int Fade);
    void RenderPiano(int Keyboard);
    void RenderCircles(int Count,int Steps, bool Strobe);

    void RenderRadial(int x, int y,int thickness, int colorCnt);
    void CalcOutput();
};

#endif // PIXELBUFFER_H
