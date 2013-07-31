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
    //ctor
    // initialize FirePalette[]
    wxImage::HSVValue hsv;
    wxImage::RGBValue rgb;
    wxColour color;
    int i;
    // calc 100 reds, black to bright red
    hsv.hue=0.0;
    hsv.saturation=1.0;
    for (i=0; i<100; i++)
    {
        hsv.value=double(i)/100.0;
        rgb = wxImage::HSVtoRGB(hsv);
        color.Set(rgb.red,rgb.green,rgb.blue);
        FirePalette.push_back(color);
    }

    // gives 100 hues red to yellow
    hsv.value=1.0;
    for (i=0; i<100; i++)
    {
        rgb = wxImage::HSVtoRGB(hsv);
        color.Set(rgb.red,rgb.green,rgb.blue);
        FirePalette.push_back(color);
        hsv.hue+=0.00166666;
    }
}

RgbEffects::~RgbEffects()
{
    //dtor
}

void RgbEffects::InitBuffer(int newBufferHt, int newBufferWi)
{
    BufferHt=newBufferHt;
    BufferWi=newBufferWi;
    int NumPixels=BufferHt * BufferWi;
    pixels.resize(NumPixels);
    tempbuf.resize(NumPixels);
    FireBuffer.resize(NumPixels);
    WaveBuffer0.resize(NumPixels);
    WaveBuffer1.resize(NumPixels);
    WaveBuffer2.resize(NumPixels);
    state=0;
}

void RgbEffects::Clear(const wxColour& bgColor)
{
    for(size_t i=0; i<pixels.size(); i++)
    {
        pixels[i]=bgColor;
    }
}

void RgbEffects::SetPalette(wxColourVector& newcolors)
{
    palette.Set(newcolors);
}

size_t RgbEffects::GetColorCount()
{
    size_t colorcnt=palette.Size();
    if (colorcnt < 1) colorcnt=1;
    return colorcnt;
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

void RgbEffects::Color2HSV(const wxColour& color, wxImage::HSVValue& hsv)
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

void RgbEffects::Get2ColorBlend(int coloridx1, int coloridx2, double ratio, wxColour &color)
{
    wxColour c1,c2;
    palette.GetColor(coloridx1,c1);
    palette.GetColor(coloridx2,c2);
    color.Set(ChannelBlend(c1.Red(),c2.Red(),ratio), ChannelBlend(c1.Green(),c2.Green(),ratio), ChannelBlend(c1.Blue(),c2.Blue(),ratio));
}

// 0 <= n < 1
void RgbEffects::GetMultiColorBlend(double n, bool circular, wxColour &color)
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
void RgbEffects::SetPixel(int x, int y, const wxColour &color)
{
    if (x >= 0 && x < BufferWi && y >= 0 && y < BufferHt)
    {
        pixels[y*BufferWi+x]=color;
    }
}

// 0,0 is lower left
void RgbEffects::SetPixel(int x, int y, const wxImage::HSVValue& hsv)
{
    wxImage::RGBValue rgb = wxImage::HSVtoRGB(hsv);
    wxColour color(rgb.red,rgb.green,rgb.blue);
    SetPixel(x,y,color);
}

void RgbEffects::DrawCircle(int xc, int yc, int r, const wxImage::HSVValue& hsv)
{
    int x, y, p;
    x=0;y=r;
    p=1-r;
    CirclePlot(xc,yc,x,y, hsv);
    while(x<y)
    {
        x++;
        if(p<0)
            p+=2*x+1;
        else
        {
            y--;
            p+=2*(x-y)+1;
        }
        CirclePlot(xc,yc,x,y, hsv);
    }
}

void RgbEffects::CirclePlot(int xc, int yc, int x, int y, const wxImage::HSVValue& hsv)
{
    SetPixel((xc+x)%BufferWi,(yc+y)%BufferHt,hsv);
    SetPixel((xc-x)<0?BufferWi+(xc-x):xc-x,(yc+y)%BufferHt,hsv);
    SetPixel((xc+x)%BufferWi,(yc-y)<0?BufferHt+(yc-y):(yc-y),hsv);
    SetPixel((xc-x)<0?BufferWi+(xc-x):xc-x,(yc-y)<0?BufferHt+(yc-y):(yc-y),hsv);
    SetPixel((xc+y)%BufferWi,(yc+x)%BufferHt,hsv);
    SetPixel((xc-y)<0?BufferWi+(xc-y):(xc-y),(yc+x)%BufferHt,hsv);
    SetPixel((xc+y)%BufferWi,(yc-x)<0?BufferHt+(yc-x):(yc-x),hsv);
    SetPixel((xc-y)<0?BufferWi+(xc-y):(xc-y),(yc-x)<0?BufferHt+(yc-x):(yc-x),hsv);
}

// 0,0 is lower left
void RgbEffects::GetPixel(int x, int y, wxColour &color)
{
    if (x >= 0 && x < BufferWi && y >= 0 && y < BufferHt)
    {
        color=pixels[y*BufferWi+x];
    }
}


// 0,0 is lower left
void RgbEffects::SetTempPixel(int x, int y, const wxColour &color)
{
    if (x >= 0 && x < BufferWi && y >= 0 && y < BufferHt)
    {
        tempbuf[y*BufferWi+x]=color;
    }
}

// 0,0 is lower left
void RgbEffects::GetTempPixel(int x, int y, wxColour &color)
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

void RgbEffects::SetState(int period, int NewSpeed, bool ResetState)
{
    if (ResetState)
    {
        state=0;
    }
    else
    {
        state+=(period-lastperiod) * NewSpeed;
    }
    speed=NewSpeed;
    lastperiod=period;
}
void RgbEffects::ClearTempBuf()
{
    for (size_t i=0; i < tempbuf.size(); i++)
    {
        tempbuf[i]=*wxBLACK;
    }
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


//  Now we come to including the programs that actually do the effects.


//#include "RenderBars.cpp"
//#include "RenderButterfly.cpp"
//#include "RenderColorWash.cpp"
//#include "RenderFire.cpp"
//#include "RenderGarlands.cpp"
//#include "RenderLife.cpp"
//#include "RenderMeteors.cpp"
//#include "RenderPictures.cpp"
//#include "RenderSnowflakes.cpp"
//#include "RenderSnowstorm.cpp"
//#include "RenderSpirals.cpp"
//#include "RenderText.cpp"
//#include "RenderTwinkle.cpp"
//#include "RenderTree.cpp"
//#include "RenderSpirograph.cpp"
//#include "RenderFireworks.cpp"







