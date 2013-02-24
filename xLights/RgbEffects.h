/***************************************************************
 * Name:      RgbEffects.h
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

#ifndef XLIGHTS_RGBEFFECTS_H
#define XLIGHTS_RGBEFFECTS_H

#include <stdint.h>
#include <list>
#include <vector>
#include <wx/colour.h>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/image.h>


typedef std::vector<wxColour> wxColourVector;
typedef std::vector<wxImage::HSVValue> hsvVector;
typedef std::vector<wxPoint> wxPointVector;


// for meteor effect
class MeteorClass
{
public:

    int x,y;
    wxImage::HSVValue hsv;
};

typedef std::list<MeteorClass> MeteorList;

class MeteorHasExpired
{
    int TailLength;
public:
    MeteorHasExpired(int t)
        : TailLength(t)
    {}

    // operator() is what's called when you do MeteorHasExpired()
    bool operator()(const MeteorClass& obj)
    {
        return obj.y + TailLength < 0;
    }
};


class SnowstormClass
{
public:
    wxPointVector points;
    wxImage::HSVValue hsv;
    int idx,ssDecay;
    ~SnowstormClass()
    {
        points.clear();
    }
};

typedef std::list<SnowstormClass> SnowstormList;


class PaletteClass
{
private:
    wxColourVector color;
    hsvVector hsv;
public:

    void Set(wxColourVector& newcolors)
    {
        color=newcolors;
        hsv.clear();
        wxImage::RGBValue newrgb;
        wxImage::HSVValue newhsv;
        for(size_t i=0; i<newcolors.size(); i++)
        {
            newrgb.red=newcolors[i].Red();
            newrgb.green=newcolors[i].Green();
            newrgb.blue=newcolors[i].Blue();
            newhsv=wxImage::RGBtoHSV(newrgb);
            hsv.push_back(newhsv);
        }
    }

    size_t Size()
    {
        return color.size();
    }

    void GetColor(size_t idx, wxColour& c)
    {
        if (idx >= color.size())
        {
            c.Set(255,255,255);
        }
        else
        {
            c=color[idx];
        }
    }

    void GetHSV(size_t idx, wxImage::HSVValue& c)
    {
        if (hsv.size() == 0)
        {
            // white
            c.hue=0.0;
            c.saturation=0.0;
            c.value=1.0;
        }
        else
        {
            c=hsv[idx % hsv.size()];
        }
    }
};


class RgbEffects
{
public:
    RgbEffects();
    ~RgbEffects();
    void InitBuffer(int newBufferHt, int newBufferWi);
    void Clear(const wxColour& bgColor);
    void SetPalette(wxColourVector& newcolors);
    size_t GetColorCount();
    void SetState(int period, int NewSpeed, bool ResetState);
    void GetPixel(int x, int y, wxColour &color);
    void RenderBars(int PaletteRepeat, int Direction, bool Highlight, bool Show3D);
    void RenderButterfly(int ColorScheme, int Style, int Chunks, int Skip);
    void RenderColorWash(bool HorizFade, bool VertFade, int RepeatCount);
    void RenderFire(int HeightPct);
    void RenderGarlands(int GarlandType, int Spacing);
    void RenderLife(int Count, int Type);
    void RenderMeteors(int MeteorType, int Count, int Length);
    void RenderPictures(int dir, const wxString& NewPictureName);
    void RenderSnowflakes(int Count, int SnowflakeType);
    void RenderSnowstorm(int Count, int Length);
    void RenderSpirals(int PaletteRepeat, int Direction, int Rotation, int Thickness, bool Blend, bool Show3D);
    void RenderText(int Top, const wxString& Line1, const wxString& Line2, const wxString& FontString, int dir);

protected:
    void SetPixel(int x, int y, const wxColour &color);
    void SetPixel(int x, int y, const wxImage::HSVValue& hsv);
    void SetTempPixel(int x, int y, const wxColour &color);
    void GetTempPixel(int x, int y, wxColour &color);
    wxUint32 GetTempPixelRGB(int x, int y);
    void SetFireBuffer(int x, int y, int PaletteIdx);
    int GetFireBuffer(int x, int y);
    double rand01();
    wxByte ChannelBlend(wxByte c1, wxByte c2, double ratio);
    void Get2ColorBlend(int coloridx1, int coloridx2, double ratio, wxColour &color);
    void GetMultiColorBlend(double n, bool circular, wxColour &color);
    void SetRangeColor(const wxImage::HSVValue& hsv1, const wxImage::HSVValue& hsv2, wxImage::HSVValue& newhsv);
    double RandomRange(double num1, double num2);
    void Color2HSV(const wxColour& color, wxImage::HSVValue& hsv);
    wxPoint SnowstormVector(int idx);
    void SnowstormAdvance(SnowstormClass& ssItem);
    void ClearTempBuf();
    int Life_CountNeighbors(int x, int y);

    int BufferHt,BufferWi;  // size of the buffer
    wxColourVector pixels; // this is the calculation buffer
    wxColourVector tempbuf;
    wxColourVector FirePalette;
    std::vector<int> FireBuffer;
    MeteorList meteors;
    SnowstormList SnowstormItems;
    PaletteClass palette;
    wxImage image;
    wxString PictureName;
    int LastSnowflakeCount;
    int LastSnowflakeType;
    int LastSnowstormCount;
    int LastLifeCount;
    int LastLifeType;
    long state;
    long LastLifeState;
    int speed;
    int lastperiod;

private:
};

#endif // XLIGHTS_RGBEFFECTS_H
