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



RgbEffects::RgbEffects()
{
    frameTimeInMs = 50;
    fireworkBursts = NULL;
    balls = NULL;
    metaballs = NULL;
}

RgbEffects::~RgbEffects()
{
    //dtor
    if (fireworkBursts != NULL) {
        delete [] fireworkBursts;
    }
    if (balls != NULL) {
        delete [] balls;
    }
    if (metaballs != NULL) {
        delete [] metaballs;
    }
}

void RgbEffects::InitBuffer(int newBufferHt, int newBufferWi)
{
    BufferHt=newBufferHt;
    BufferWi=newBufferWi;
    DiagLen=sqrt( (double)BufferHt*BufferHt + BufferWi*BufferWi);
    NumPixels=BufferHt * BufferWi;
    pixels.resize(NumPixels);
    tempbuf.resize(NumPixels);
    FireBuffer.resize(NumPixels);
    WaveBuffer0.resize(NumPixels);
    WaveBuffer1.resize(NumPixels);
    WaveBuffer2.resize(NumPixels);
    state = 0;
    effectState = 0;
}

void RgbEffects::Clear(const xlColour& bgColor)
{
    if (InhibitClear) { InhibitClear = false; return; } //allow canvas to be persistent for piano fx (self-reseting for safety) -DJ
    for(size_t i=0; i<pixels.size(); i++)
    {
        pixels[i]=bgColor;
    }
}

void RgbEffects::SetPalette(xlColourVector& newcolors)
{
    palette.Set(newcolors);
}

size_t RgbEffects::GetColorCount()
{
    return palette.Size();
}

// return a random number between 0 and 1 inclusive
double RgbEffects::rand01()
{
    return (double)rand()/(double)RAND_MAX;
}

// generates a random number between num1 and num2 inclusive
double RgbEffects::RandomRange(double num1, double num2)
{
    double hi,lo;
    if (num1 < num2)
    {
        lo = num1;
        hi = num2;
    }
    else
    {
        lo = num2;
        hi = num1;
    }
    return rand01()*(hi-lo)+ lo;
}

void RgbEffects::Color2HSV(const xlColour& color, wxImage::HSVValue& hsv)
{
    wxImage::RGBValue rgb(color.Red(),color.Green(),color.Blue());
    hsv=wxImage::RGBtoHSV(rgb);
}

// sets newcolor to a random color between hsv1 and hsv2
void RgbEffects::SetRangeColor(const wxImage::HSVValue& hsv1, const wxImage::HSVValue& hsv2, wxImage::HSVValue& newhsv)
{
    newhsv.hue=RandomRange(hsv1.hue,hsv2.hue);
    newhsv.saturation=RandomRange(hsv1.saturation,hsv2.saturation);
    newhsv.value=1.0;
}

// return a value between c1 and c2
wxByte RgbEffects::ChannelBlend(wxByte c1, wxByte c2, double ratio)
{
    return c1 + floor(ratio*(c2-c1)+0.5);
}

void RgbEffects::Get2ColorBlend(int coloridx1, int coloridx2, double ratio, xlColour &color)
{
    xlColour c1,c2;
    palette.GetColor(coloridx1,c1);
    palette.GetColor(coloridx2,c2);
    color.Set(ChannelBlend(c1.Red(),c2.Red(),ratio), ChannelBlend(c1.Green(),c2.Green(),ratio), ChannelBlend(c1.Blue(),c2.Blue(),ratio));
}

void RgbEffects::Get2ColorAlphaBlend(const xlColour& c1, const xlColour& c2, double ratio, xlColour &color)
{
    color.Set(ChannelBlend(c1.Red(),c2.Red(),ratio), ChannelBlend(c1.Green(),c2.Green(),ratio), ChannelBlend(c1.Blue(),c2.Blue(),ratio));
}

HSVValue RgbEffects::Get2ColorAdditive(HSVValue& hsv1, HSVValue& hsv2)
{
    wxImage::RGBValue rgb;
    wxImage::RGBValue rgb1 = wxImage::HSVtoRGB(hsv1);
    wxImage::RGBValue rgb2 = wxImage::HSVtoRGB(hsv2);
    rgb.red = rgb1.red + rgb2.red;
    rgb.green = rgb1.green + rgb2.green;
    rgb.blue = rgb1.blue + rgb2.blue;
    return wxImage::RGBtoHSV(rgb);
}
// 0 <= n < 1
void RgbEffects::GetMultiColorBlend(double n, bool circular, xlColour &color)
{
    size_t colorcnt=GetColorCount();
    if (colorcnt <= 1)
    {
        palette.GetColor(0,color);
        return;
    }
    if (n >= 1.0) n=0.99999;
    if (n < 0.0) n=0.0;
    double realidx=circular ? n*colorcnt : n*(colorcnt-1);
    int coloridx1=floor(realidx);
    int coloridx2=(coloridx1+1) % colorcnt;
    double ratio=realidx-double(coloridx1);
    Get2ColorBlend(coloridx1,coloridx2,ratio,color);
}


// 0,0 is lower left
void RgbEffects::SetPixel(int x, int y, const xlColour &color, bool wrap)
{
    if (wrap) {
        while (x < 0) {
            x += BufferWi;
        }
        while (y < 0) {
            y += BufferHt;
        }
        while (x > BufferWi) {
            x -= BufferWi;
        }
        while (y > BufferHt) {
            y -= BufferHt;
        }
    }
    if (x >= 0 && x < BufferWi && y >= 0 && y < BufferHt)
    {
        pixels[y*BufferWi+x] = color;
    }
}

// 0,0 is lower left
void RgbEffects::SetPixel(int x, int y, const wxImage::HSVValue& hsv, bool wrap)
{
    if (wrap) {
        while (x < 0) {
            x += BufferWi;
        }
        while (y < 0) {
            y += BufferHt;
        }
        while (x > BufferWi) {
            x -= BufferWi;
        }
        while (y > BufferHt) {
            y -= BufferHt;
        }
    }
    if (x >= 0 && x < BufferWi && y >= 0 && y < BufferHt)
    {
        pixels[y*BufferWi+x] = hsv;
    }
}

//copy src to dest: -DJ
void RgbEffects::CopyPixel(int srcx, int srcy, int destx, int desty)
{
    if ((srcx >= 0) && (srcx < BufferWi) && (srcy >= 0) && (srcy < BufferHt))
        if ((destx >= 0) && (destx < BufferWi) && (desty >= 0) && (desty < BufferHt))
            pixels[desty * BufferWi + destx] = pixels[srcy * BufferWi + srcx];
}

void RgbEffects::DrawHLine(int y, int xstart, int xend, const xlColor &color, bool wrap) {
    if (xstart > xend) {
        int i = xstart;
        xstart = xend;
        xend = i;
    }
    for (int x = xstart; x <= xend; x++) {
        SetPixel(x, y, color, wrap);
    }
}
void RgbEffects::DrawVLine(int x, int ystart, int yend, const xlColor &color, bool wrap) {
    if (ystart > yend) {
        int i = ystart;
        ystart = yend;
        yend = i;
    }
    for (int y = ystart; y <= yend; y++) {
        SetPixel(x, y, color, wrap);
    }
}
void RgbEffects::DrawBox(int x1, int y1, int x2, int y2, const xlColor& color, bool wrap) {
    if (y1 > y2) {
        int i = y1;
        y1 = y2;
        y2 = i;
    }
    if (x1 > x2) {
        int i = x1;
        x1 = x2;
        x2 = i;
    }
    for (int x = x1; x <= x2; x++) {
        for (int y = y1; y <= y2; y++) {
            SetPixel(x, y, color, wrap);
        }
    }
}

// Bresenham's line algorithm
void RgbEffects::DrawLine( const int x0_, const int y0_, const int x1_, const int y1_, const xlColor& color )
{
    int x0 = x0_;
    int x1 = x1_;
    int y0 = y0_;
    int y1 = y1_;

    int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
    int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1;
    int err = (dx>dy ? dx : -dy)/2, e2;

  for(;;){
    SetPixel(x0,y0, color);
    if (x0==x1 && y0==y1) break;
    e2 = err;
    if (e2 >-dx) { err -= dy; x0 += sx; }
    if (e2 < dy) { err += dx; y0 += sy; }
  }
}

void RgbEffects::DrawThickLine( const int x0_, const int y0_, const int x1_, const int y1_, const xlColor& color, bool direction )
{
    int x0 = x0_;
    int x1 = x1_;
    int y0 = y0_;
    int y1 = y1_;
    int lastx = x0;
    int lasty = y0;

    int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
    int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1;
    int err = (dx>dy ? dx : -dy)/2, e2;

  for(;;){
    SetPixel(x0,y0, color);
    if( (x0 != lastx) && (y0 != lasty) && (x0_ != x1_) && (y0_ != y1_) )
    {
        int fix = 0;
        if( x0 > lastx ) fix += 1;
        if( y0 > lasty ) fix += 2;
        if( direction  ) fix += 4;
        switch (fix)
        {
        case 2:
        case 4:
            if( x0 < BufferWi -2 ) SetPixel(x0+1,y0, color);
            break;
        case 3:
        case 5:
            if( x0 > 0 ) SetPixel(x0-1,y0, color);
            break;
        case 0:
        case 1:
            if( y0 < BufferHt -2 )SetPixel(x0,y0+1, color);
            break;
        case 6:
        case 7:
            if( y0 > 0 )SetPixel(x0,y0-1, color);
            break;
        }
    }
    lastx = x0;
    lasty = y0;
    if (x0==x1 && y0==y1) break;
    e2 = err;
    if (e2 >-dx) { err -= dy; x0 += sx; }
    if (e2 < dy) { err += dx; y0 += sy; }
  }
}

void RgbEffects::DrawFadingCircle(int x0, int y0, int radius, const xlColor& rgb, bool wrap)
{
    HSVValue hsv = wxImage::RGBtoHSV(rgb);
    xlColor color(rgb);
    int r = radius;
    if (allowAlpha) {
        while(r >= 0)
        {
            color.alpha = (double)rgb.alpha * (1.0 - (double)(r) / (double)radius);
            DrawCircle(x0, y0, r, color, wrap);
            r--;
        }
    } else {
        double full_brightness = hsv.value;
        while(r >= 0)
        {
            hsv.value = full_brightness * (1.0 - (double)(r) / (double)radius);
            if( hsv.value > 0.0 )
            {
                color = wxImage::HSVtoRGB(hsv);
                DrawCircle(x0, y0, r, color, wrap);
            }
            r--;
        }
    }
}

void RgbEffects::DrawCircle(int x0, int y0, int radius, const xlColor& rgb, bool filled, bool wrap)
{
    int x = radius;
    int y = 0;
    int radiusError = 1 - x;

    while(x >= y) {
        if (!filled) {
            SetPixel(x + x0, y + y0, rgb, wrap);
            SetPixel(y + x0, x + y0, rgb, wrap);
            SetPixel(-x + x0, y + y0, rgb, wrap);
            SetPixel(-y + x0, x + y0, rgb, wrap);
            SetPixel(-x + x0, -y + y0, rgb, wrap);
            SetPixel(-y + x0, -x + y0, rgb, wrap);
            SetPixel(x + x0, -y + y0, rgb, wrap);
            SetPixel(y + x0, -x + y0, rgb, wrap);
        } else {
            DrawVLine(x0 - x, y0 - y, y0 + y, rgb, wrap);
            DrawVLine(x0 + x, y0 - y, y0 + y, rgb, wrap);
            DrawVLine(x0 - y, y0 - x, y0 + x, rgb, wrap);
            DrawVLine(x0 + y, y0 - x, y0 + x, rgb, wrap);
        }
        y++;
        if (radiusError<0) {
            radiusError += 2 * y + 1;
        } else {
            x--;
            radiusError += 2 * (y - x) + 1;
        }
    }
}


// 0,0 is lower left
void RgbEffects::GetPixel(int x, int y, xlColour &color)
{
    if (x >= 0 && x < BufferWi && y >= 0 && y < BufferHt)
    {
        color=pixels[y*BufferWi+x];
    }
}


// 0,0 is lower left
void RgbEffects::SetTempPixel(int x, int y, const xlColour &color)
{
    if (x >= 0 && x < BufferWi && y >= 0 && y < BufferHt)
    {
        tempbuf[y*BufferWi+x]=color;
    }
}

// 0,0 is lower left
void RgbEffects::GetTempPixel(int x, int y, xlColour &color)
{
    if (x >= 0 && x < BufferWi && y >= 0 && y < BufferHt)
    {
        color=tempbuf[y*BufferWi+x];
    }
}

wxUint32 RgbEffects::GetTempPixelRGB(int x, int y)
{
    if (x >= 0 && x < BufferWi && y >= 0 && y < BufferHt)
    {
        return tempbuf[y*BufferWi+x].GetRGB();
    }
    return 0;
}

void RgbEffects::SetState(int period, int NewSpeed, bool ResetState, const wxString& model_name)
{
    if (ResetState)
    {
        state = 0;
        effectState = 0;
        needToInit = true;
    }
    else
    {
        //xLights effect speeds are set on 50ms, this scales appropriately so the speed stays
        //visually the same for 25ms and others
        state+=(period-lastperiod) * NewSpeed * frameTimeInMs / 50;
    }
    speed=NewSpeed;
    lastperiod=curPeriod=period;
    cur_model = model_name;
    curPeriod = period;
}
void RgbEffects::ClearTempBuf()
{
    for (size_t i=0; i < tempbuf.size(); i++)
    {
        tempbuf[i].Set(0, 0, 0, 0);
    }
}
double RgbEffects::GetEffectTimeIntervalPosition(float cycles) {
    double retval = (double)(curPeriod-curEffStartPer)/(double)(curEffEndPer-curEffStartPer);
    retval *= cycles;
    while (retval > 1.0) {
        retval -= 1.0;
    }
    return retval;
}
double RgbEffects::GetEffectTimeIntervalPosition()
{
    double retval = (double)(curPeriod-curEffStartPer)/(double)(curEffEndPer-curEffStartPer);
//    debug(10, "GetEffTiIntPos(fr last? %d): (cur %d - curst %d)/(curend %d - curst) = %f, (cur - curst)/(next %d - curst) = %f, (cur - prev %d)/(curend - prev) = %f", from_last, curPeriod, curEffStartPer, curEffEndPer, GetEffectPeriodPosition(), nextEffTimePeriod, (double)(curPeriod-curEffStartPer)/(nextEffTimePeriod-curEffStartPer), prevNonBlankStartPeriod, (double)(curPeriod - prevNonBlankStartPeriod) / (curEffEndPer - prevNonBlankStartPeriod));
    return retval;
}
void RgbEffects::ClearWaveBuffer1()
{
    for (size_t i=0; i < WaveBuffer1.size(); i++)
    {
        WaveBuffer1[i]=0;
    }
}
void RgbEffects::ClearWaveBuffer2()
{
    for (size_t i=0; i < WaveBuffer2.size(); i++)
    {
        WaveBuffer2[i]=0;
    }
}

void RgbEffects::SetFadeTimes(float fadeInDuration, float fadeOutDuration )
{
    fadeinsteps = (int)(fadeInDuration*1000)/frameTimeInMs;
    fadeoutsteps = (int)(fadeOutDuration*1000)/frameTimeInMs;
}
void RgbEffects::GetFadeSteps( int& fadeInSteps, int& fadeOutSteps)
{
    fadeInSteps = fadeinsteps;
    fadeOutSteps = fadeoutsteps;
}

void RgbEffects::SetEffectDuration(int startMsec, int endMsec)
{
    curEffStartPer = startMsec / frameTimeInMs;
    curEffEndPer = (endMsec - 1) / frameTimeInMs;
}

void RgbEffects::GetEffectPeriods( int& start, int& endp)
{
    start = curEffStartPer;
    endp = curEffEndPer;
}

