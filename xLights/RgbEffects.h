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

#include "../include/globals.h"


typedef std::vector<wxColour> wxColourVector;
typedef std::vector<wxImage::HSVValue> hsvVector;
typedef std::vector<wxPoint> wxPointVector;
typedef wxImage::HSVValue HSVValue;

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
    wxImage::HSVValue _hsv;

    void Reset(int x, int y, bool active, float velocity, wxImage::HSVValue hsv)
    {
        _x       = x;
        _y       = y;
        vel      = (rand()-RAND_MAX/2)*velocity/(RAND_MAX/2);
        angle    = 2*M_PI*rand()/RAND_MAX;
        _dx      = vel*cos(angle);
        _dy      = vel*sin(angle);
        _bActive = active;
        _cycles  = 0;
        _hsv     = hsv;
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
    float _t;
    float dir;
    wxImage::HSVValue hsvcolor;

    void Reset(float x, float y, float speed, float angle, float radius, wxImage::HSVValue color)
    {
        _x=x;
        _y=y;
        _dx=speed*cos(angle);
        _dy=speed*sin(angle);
        _radius = radius;
        hsvcolor = color;
        _t=M_PI/6.0;
        dir =1.0;

    }
    void updatePositionArc(int x,int y, int r)
    {
        _x=x+r*cos(_t);
        _y=y+r*sin(_t);
        _t+=dir* (M_PI/9.0);
        dir *= _t < M_PI/6.0 || _t > (2*M_PI)/3?-1.0:1.0;
    }
    void updatePosition(float incr, int width, int height)
    {
        _x+=_dx*incr;
        _x = _x>width?0:_x;
        _x = _x<0?width:_x;
        _y+=_dy*incr;
        _y = _y>height?0:_y;
        _y = _y<0?height:_y;
    }

    void Bounce(int width, int height)
    {
        if (_x-_radius<=0) {
            _dx=abs(_dx);
            if (_dx < 0.2) _dx=0.2;
        }
        if (_x+_radius>=width) {
            _dx=-abs(_dx);
            if (_dx > -0.2) _dx=-0.2;
        }
        if (_y-_radius<=0) {
            _dy=abs(_dy);
            if (_dy < 0.2) _dy=0.2;
        }
        if (_y+_radius>=height) {
            _dy=-abs(_dy);
            if (_dy > -0.2) _dy=-0.2;
        }
    }

};

class MetaBall : public RgbBalls
{
public:
    float Equation(float x, float y)
    {
        if(x==_x || y==_y) return 1;
        return (_radius/(sqrt(pow(x-_x,2)+pow(y-_y,2))));
    }
};

// for meteor effect
class MeteorClass
{
public:

    int x,y;
    wxImage::HSVValue hsv;
};

// for radial meteor effect
class MeteorRadialClass
{
public:

    double x,y,dx,dy;
    int cnt;
    wxImage::HSVValue hsv;
};

typedef std::list<MeteorClass> MeteorList;
typedef std::list<MeteorRadialClass> MeteorRadialList;


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
        size_t colorcnt=color.size();
        if (colorcnt < 1) colorcnt=1;
        return colorcnt;
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

    void SetFadeTimes(float fadeIn, float fadeOut );
    void SetEffectDuration(int startMsec, int endMsec, int nextMsec);

    int fadein, fadeout;
    int curEffStartPer, curEffEndPer, nextEffTimePeriod;

#include "Effects.h"

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

    void DrawCircleClipped(int xc, int yc, int r, const wxImage::HSVValue& hsv);
    void CirclePlotClipped(int xc, int xy, int x, int y, const wxImage::HSVValue& hsv);

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
    void RenderMeteorsVertical(int ColorScheme, int Count, int Length, int MeteorsEffect, int SwirlIntensity);
    void RenderMeteorsHorizontal(int ColorScheme, int Count, int Length, int MeteorsEffect, int SwirlIntensity);
    void RenderMeteorsImplode(int ColorScheme, int Count, int Length, int SwirlIntensity);
    HSVValue Get2ColorAdditive(HSVValue& hsv1, HSVValue& hsv2);
    void RenderMeteorsExplode(int ColorScheme, int Count, int Length, int SwirlIntensity);
    void RenderMetaBalls(int numBalls);

    int BufferHt,BufferWi;  // size of the buffer
    int DiagLen;  // length of the diagonal
    int NumPixels;
    wxColourVector pixels; // this is the calculation buffer
    wxColourVector tempbuf;
    wxColourVector FirePalette;
    std::vector<int> FireBuffer;
    std::vector<int> WaveBuffer0;
    std::vector<int> WaveBuffer1;
    std::vector<int> WaveBuffer2;
    MeteorList meteors;
    MeteorRadialList meteorsRadial;
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
    int lastperiod, curPeriod;
    RgbFireworks fireworkBursts[20000];
    RgbBalls balls[rgb_MAX_BALLS];
    int maxmovieframes;
    long timer_countdown[1];

    double GetEffectPeriodPosition();
    double GetEffectTimeIntervalPosition();
    MetaBall metaballs[10];


private:
    void RenderRadial(int start_x,int start_y,int radius,int colorCnt, int number, bool radial_3D);
    void RenderCirclesUpdate(int number, RgbBalls* effObjs);
};

#endif // XLIGHTS_RGBEFFECTS_H
