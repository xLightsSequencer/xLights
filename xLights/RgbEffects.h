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

#define rgb_MAX_BALLS 20

class RgbFireworks
{
public:
//static const float velocity = 2.5;
    static const int maxCycle = 4096;
    static const int maxNewBurstFlakes = 10;
    float _x;
    float _y;
    float _dx;
    float _dy;
    float vel;
    float angle;
    bool _bActive;
    int _cycles;

    void Reset(int x, int y, bool active, float velocity)
    {
        _x = x;
        _y = y;
        vel = (rand()-RAND_MAX/2)*velocity/(RAND_MAX/2);
        angle = 2*M_PI*rand()/RAND_MAX;
        _dx = vel*cos(angle);
        _dy = vel*sin(angle);
        _bActive = active;
        _cycles = 0;
    }
protected:
private:
};

class RgbBalls
{
public:
    float _x;
    float _y;
    float _dx;
    float _dy;
    float _radius;
    wxColor _color;

    void Reset(float x, float y, float speed, float angle, float radius, wxColor color)
    {
        _x=x;
        _y=y;
        _dx=speed*cos(angle);
        _dy=speed*sin(angle);
        _radius = radius;
        _color = color;
    }

    void updatePosition(float incr)
    {
        _x+=_dx*incr;
        _y+=_dy*incr;
    }

};

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
    void RenderFire(int HeightPct,int HueShift,bool GrowFire);
    void RenderGarlands(int GarlandType, int Spacing);
    void RenderLife(int Count, int Type);
    void RenderMeteors(int MeteorType, int Count, int Length,bool FallUp,int MeteorsEffect,int SwirlIntensity);
    void RenderPictures(int dir, const wxString& NewPictureName,int GifSpeed);
    void RenderSnowflakes(int Count, int SnowflakeType);
    void RenderSnowstorm(int Count, int Length);
    void RenderSpirals(int PaletteRepeat, int Direction, int Rotation, int Thickness, bool Blend, bool Show3D);
    void RenderText(int Position1, const wxString& Line1, const wxString& FontString1,int dir1,int TextRotation1,int Effect1,
                    int Position2, const wxString& Line2, const wxString& FontString2,int dir2,int TextRotation2,int Effect2);
    void RenderTwinkle(int Count,int Steps,bool Strobe);
    void RenderTree(int Branches);
    void RenderSpirograph(int R, int r, int d,bool Animate);
    void RenderFireworks(int Number_Explosions,int Count,float Velocity,int Fade);
    void RenderPiano(int Keyboard);
    void RenderCircles(int Count,int Steps, bool Strobe);

    void RenderRadial(int x, int y,int thickness, int colorCnt);
    void RenderCircles(int number,int radius, bool bounce, bool collide, bool random,
                        bool radial, int start_x, int start_y);



protected:
    void SetPixel(int x, int y, const wxColour &color);
    void SetPixel(int x, int y, const wxImage::HSVValue& hsv);
    void SetTempPixel(int x, int y, const wxColour &color);
    void GetTempPixel(int x, int y, wxColour &color);
    wxUint32 GetTempPixelRGB(int x, int y);
    void SetFireBuffer(int x, int y, int PaletteIdx);
    int GetFireBuffer(int x, int y);
    void SetWaveBuffer1(int x, int y, int value);
    int GetWaveBuffer1(int x, int y);
    void SetWaveBuffer2(int x, int y, int value);
    int GetWaveBuffer2(int x, int y);

    void DrawCircle(int xc, int yc, int r, const wxImage::HSVValue& hsv);
    void CirclePlot(int xc, int xy, int x, int y, const wxImage::HSVValue& hsv);

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
    void ClearWaveBuffer1();
    void ClearWaveBuffer2();
    int Life_CountNeighbors(int x, int y);
    void RenderTextLine(wxMemoryDC& dc, int idx, int Position, const wxString& Line, int dir, int Effect, int Countdown);

    int BufferHt,BufferWi;  // size of the buffer
    wxColourVector pixels; // this is the calculation buffer
    wxColourVector tempbuf;
    wxColourVector FirePalette;
    std::vector<int> FireBuffer;
    std::vector<int> WaveBuffer0;
    std::vector<int> WaveBuffer1;
    std::vector<int> WaveBuffer2;
    MeteorList meteors;
    SnowstormList SnowstormItems;
    PaletteClass palette;

    wxImage image;
    int imageCount;
    int imageIndex;

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
    RgbFireworks fireworkBursts[20000];
    RgbBalls balls[rgb_MAX_BALLS];
    int maxmovieframes;
    long old_longsecs[1],timer_countdown[1];

private:
};

#endif // XLIGHTS_RGBEFFECTS_H
