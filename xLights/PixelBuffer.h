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
    float effectMixThreshold;
    MixTypes MixType;
    RgbEffects Effect[2];
    void GetMixedColor(wxCoord x, wxCoord y, wxColour& c);
    double fadeFactor[2];

public:
    PixelBufferClass();
    ~PixelBufferClass();
    void InitBuffer(wxXmlNode* ModelNode);
    void Clear();
    size_t GetColorCount(int layer);
    void SetMixType(const wxString& MixName);
    void SetPalette(int layer, wxColourVector& newcolors);
    void SetLayer(int newlayer, int period, int speed, bool ResetState);
    void SetTimes(int layer, int startTime, int endTime, int nextTime);
    void SetFadeTimes(int layer, float inTime, float outTime);
    void SetSparkle(int freq);
    void SetBrightness(int value);
    void SetContrast(int value);
    void SetMixThreshold(int value);
    void CalcOutput(int EffectPeriod);
    void DisplayEffectOnWindow(wxWindow* window);

#include "Effects.h"

};

#endif // PIXELBUFFER_H
