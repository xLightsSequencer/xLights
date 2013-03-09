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

void RgbEffects::RenderBars(int PaletteRepeat, int Direction, bool Highlight, bool Show3D)
{
    int x,y,n,pixel_ratio,ColorIdx;
    bool IsMovingDown,IsHighlightRow;
    wxImage::HSVValue hsv;
    size_t colorcnt=GetColorCount();
    int BarCount = PaletteRepeat * colorcnt;
    int BarHt = BufferHt/BarCount+1;
    int HalfHt = BufferHt/2;
    int BlockHt=colorcnt * BarHt;
    int f_offset = state/4 % BlockHt;
    for (y=0; y<BufferHt; y++)
    {
        switch (Direction)
        {
        case 1:
            IsMovingDown=true;
            break;
        case 2:
            IsMovingDown=(y <= HalfHt);
            break;
        case 3:
            IsMovingDown=(y > HalfHt);
            break;
        default:
            IsMovingDown=false;
            break;
        }
        if (IsMovingDown)
        {
            n=y+f_offset;
            pixel_ratio = BarHt - n%BarHt - 1;
            IsHighlightRow=n % BarHt == 0;
        }
        else
        {
            n=y-f_offset+BlockHt;
            pixel_ratio = n%BarHt;
            IsHighlightRow=(n % BarHt == BarHt-1); // || (y == BufferHt-1);
        }
        ColorIdx=(n % BlockHt) / BarHt;
        palette.GetHSV(ColorIdx, hsv);
        if (Highlight && IsHighlightRow) hsv.saturation=0.0;
        if (Show3D) hsv.value *= double(pixel_ratio) / BarHt;
        for (x=0; x<BufferWi; x++)
        {
            SetPixel(x,y,hsv);
        }
    }
}

void RgbEffects::RenderButterfly(int ColorScheme, int Style, int Chunks, int Skip)
{
    int x,y,d;
    double n,x1,y1,f;
    double h=0.0;
    static const double pi2=6.283185307;
    wxColour color;
    wxImage::HSVValue hsv;
    int maxframe=BufferHt*2;
    int frame=(BufferHt * state / 200)%maxframe;
    double offset=double(state)/100.0;

    for (x=0; x<BufferWi; x++)
    {
        for (y=0; y<BufferHt; y++)
        {
            switch (Style)
            {
            case 1:
                n = abs((x*x - y*y) * sin (offset + ((x+y)*pi2 / (BufferHt+BufferWi))));
                d = x*x + y*y+1;
                h=n/d;
                break;
            case 2:
                f=(frame < maxframe/2) ? frame+1 : maxframe - frame;
                x1=(double(x)-BufferWi/2.0)/f;
                y1=(double(y)-BufferHt/2.0)/f;
                h=sqrt(x1*x1+y1*y1);
                break;
            case 3:
                f=(frame < maxframe/2) ? frame+1 : maxframe - frame;
                f=f*0.1+double(BufferHt)/60.0;
                x1 = (x-BufferWi/2.0)/f;
                y1 = (y-BufferHt/2.0)/f;
                h=sin(x1) * cos(y1);
                break;
            }
            hsv.saturation=1.0;
            hsv.value=1.0;
            if (Chunks <= 1 || int(h*Chunks) % Skip != 0)
            {
                if (ColorScheme == 0)
                {
                    hsv.hue=h;
                    SetPixel(x,y,hsv);
                }
                else
                {
                    GetMultiColorBlend(h,false,color);
                    SetPixel(x,y,color);
                }
            }
        }
    }
}

void RgbEffects::RenderColorWash(bool HorizFade, bool VertFade, int RepeatCount)
{
    static int SpeedFactor=200;
    int x,y;
    wxColour color;
    wxImage::HSVValue hsv,hsv2;
    size_t colorcnt=GetColorCount();
    int CycleLen=colorcnt*SpeedFactor;
    if (state > (colorcnt-1)*SpeedFactor*RepeatCount && RepeatCount < 10)
    {
        GetMultiColorBlend(double(RepeatCount%2), false, color);
    }
    else
    {
        GetMultiColorBlend(double(state % CycleLen) / double(CycleLen), true, color);
    }
    Color2HSV(color,hsv);
    double HalfHt=double(BufferHt-1)/2.0;
    double HalfWi=double(BufferWi-1)/2.0;
    for (x=0; x<BufferWi; x++)
    {
        for (y=0; y<BufferHt; y++)
        {
            hsv2=hsv;
            if (HorizFade) hsv2.value*=1.0-abs(HalfWi-x)/HalfWi;
            if (VertFade) hsv2.value*=1.0-abs(HalfHt-y)/HalfHt;
            SetPixel(x,y,hsv2);
        }
    }
}

// 0 <= x < BufferWi
// 0 <= y < BufferHt
void RgbEffects::SetFireBuffer(int x, int y, int PaletteIdx)
{
    if (x >= 0 && x < BufferWi && y >= 0 && y < BufferHt)
    {
        FireBuffer[y*BufferWi+x] = PaletteIdx;
    }
}

// 0 <= x < BufferWi
// 0 <= y < BufferHt
int RgbEffects::GetFireBuffer(int x, int y)
{
    if (x >= 0 && x < BufferWi && y >= 0 && y < BufferHt)
    {
        return FireBuffer[y*BufferWi+x];
    }
    return -1;
}

void RgbEffects::SetWaveBuffer1(int x, int y, int value)
{
    if (x >= 0 && x < BufferWi && y >= 0 && y < BufferHt)
    {
        WaveBuffer1[y*BufferWi+x] = value;
    }
}
void RgbEffects::SetWaveBuffer2(int x, int y, int value)
{
    if (x >= 0 && x < BufferWi && y >= 0 && y < BufferHt)
    {
        WaveBuffer2[y*BufferWi+x] = value;
    }
}

int RgbEffects::GetWaveBuffer1(int x, int y)
{
    if (x >= 0 && x < BufferWi && y >= 0 && y < BufferHt)
    {
        return WaveBuffer1[y*BufferWi+x];
    }
    return -1;
}
int RgbEffects::GetWaveBuffer2(int x, int y)
{
    if (x >= 0 && x < BufferWi && y >= 0 && y < BufferHt)
    {
        return WaveBuffer2[y*BufferWi+x];
    }
    return -1;
}

// 10 <= HeightPct <= 100
void RgbEffects::RenderFire(int HeightPct)
{
    int x,y,i,r,v1,v2,v3,v4,n,new_index;
    if (state == 0)
    {
        for (i=0; i < FireBuffer.size(); i++)
        {
            FireBuffer[i]=0;
        }
    }
    // build fire
    for (x=0; x<BufferWi; x++)
    {
        r=x%2==0 ? 190+(rand() % 10) : 100+(rand() % 50);
        SetFireBuffer(x,0,r);
    }
    int step=255*100/BufferHt/HeightPct;
    int sum;
    for (y=1; y<BufferHt; y++)
    {
        for (x=0; x<BufferWi; x++)
        {
            v1=GetFireBuffer(x-1,y-1);
            v2=GetFireBuffer(x+1,y-1);
            v3=GetFireBuffer(x,y-1);
            v4=GetFireBuffer(x,y-1);
            n=0;
            sum=0;
            if(v1>=0)
            {
                sum+=v1;
                n++;
            }
            if(v2>=0)
            {
                sum+=v2;
                n++;
            }
            if(v3>=0)
            {
                sum+=v3;
                n++;
            }
            if(v4>=0)
            {
                sum+=v4;
                n++;
            }
            new_index=n > 0 ? sum / n : 0;
            if (new_index > 0)
            {
                new_index+=(rand() % 100 < 20) ? step : -step;
                if (new_index < 0) new_index=0;
                if (new_index >= FirePalette.size()) new_index = FirePalette.size()-1;
            }
            SetFireBuffer(x,y,new_index);
        }
    }
    for (y=0; y<BufferHt; y++)
    {
        for (x=0; x<BufferWi; x++)
        {
            //SetPixel(x,y,FirePalette[y]);
            SetPixel(x,y,FirePalette[GetFireBuffer(x,y)]);
        }
    }
}

void RgbEffects::RenderGarlands(int GarlandType, int Spacing)
{
    int x,y,yadj,ylimit,ring;
    double ratio;
    wxColour color;
    int PixelSpacing=Spacing*BufferHt/100+3;
    int limit=BufferHt*PixelSpacing*4;
    int GarlandsState=(limit - (state % limit))/4;
    // ring=0 is the top ring
    for (ring=0; ring<BufferHt; ring++)
    {
        ratio=double(ring)/double(BufferHt);
        GetMultiColorBlend(ratio, false, color);
        y=GarlandsState - ring*PixelSpacing;
        ylimit=BufferHt-ring-1;
        for (x=0; x<BufferWi; x++)
        {
            yadj=y;
            switch (GarlandType)
            {
            case 1:
                switch (x%5)
                {
                case 2:
                    yadj-=2;
                    break;
                case 1:
                case 3:
                    yadj-=1;
                    break;
                }
                break;
            case 2:
                switch (x%5)
                {
                case 2:
                    yadj-=4;
                    break;
                case 1:
                case 3:
                    yadj-=2;
                    break;
                }
                break;
            case 3:
                switch (x%6)
                {
                case 3:
                    yadj-=6;
                    break;
                case 2:
                case 4:
                    yadj-=4;
                    break;
                case 1:
                case 5:
                    yadj-=2;
                    break;
                }
                break;
            case 4:
                switch (x%5)
                {
                case 1:
                case 3:
                    yadj-=2;
                    break;
                }
                break;
            }
            if (yadj < ylimit) yadj=ylimit;
            if (yadj < BufferHt) SetPixel(x,yadj,color);
        }
    }
}

int RgbEffects::Life_CountNeighbors(int x0, int y0)
{
    //     2   3   4
    //     1   X   5
    //     0   7   6
    static int n_x[] = {-1,-1,-1,0,1,1,1,0};
    static int n_y[] = {-1,0,1,1,1,0,-1,-1};
    int x,y,cnt=0;
    for (int i=0; i < 8; i++)
    {
        x=(x0+n_x[i]) % BufferWi;
        y=(y0+n_y[i]) % BufferHt;
        if (x < 0) x+=BufferWi;
        if (y < 0) y+=BufferHt;
        if (GetTempPixelRGB(x,y) != 0) cnt++;
    }
    return cnt;
}

// use tempbuf for calculations
void RgbEffects::RenderLife(int Count, int Type)
{
    int i,x,y,cnt;
    bool isLive;
    wxColour color;
    Count=BufferWi * BufferHt * Count / 200 + 1;
    if (state == 0 || Count != LastLifeCount || Type != LastLifeType)
    {
        // seed tempbuf
        LastLifeCount=Count;
        LastLifeType=Type;
        ClearTempBuf();
        for(i=0; i<Count; i++)
        {
            x=rand() % BufferWi;
            y=rand() % BufferHt;
            GetMultiColorBlend(rand01(),false,color);
            SetTempPixel(x,y,color);
        }
    }
    long TempState=state % 400 / 20;
    if (TempState == LastLifeState)
    {
        pixels=tempbuf;
        return;
    }
    else
    {
        LastLifeState=TempState;
    }
    for (x=0; x < BufferWi; x++)
    {
        for (y=0; y < BufferHt; y++)
        {
            GetTempPixel(x,y,color);
            isLive=(color.GetRGB() != 0);
            cnt=Life_CountNeighbors(x,y);
            switch (Type)
            {
            case 0:
                // B3/S23
                /*
                Any live cell with fewer than two live neighbours dies, as if caused by under-population.
                Any live cell with two or three live neighbours lives on to the next generation.
                Any live cell with more than three live neighbours dies, as if by overcrowding.
                Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
                */
                if (isLive && cnt >= 2 && cnt <= 3)
                {
                    SetPixel(x,y,color);
                }
                else if (!isLive && cnt == 3)
                {
                    GetMultiColorBlend(rand01(),false,color);
                    SetPixel(x,y,color);
                }
                break;
            case 1:
                // B35/S236
                if (isLive && (cnt == 2 || cnt == 3 || cnt == 6))
                {
                    SetPixel(x,y,color);
                }
                else if (!isLive && (cnt == 3 || cnt == 5))
                {
                    GetMultiColorBlend(rand01(),false,color);
                    SetPixel(x,y,color);
                }
                break;
            case 2:
                // B357/S1358
                if (isLive && (cnt == 1 || cnt == 3 || cnt == 5 || cnt == 8))
                {
                    SetPixel(x,y,color);
                }
                else if (!isLive && (cnt == 3 || cnt == 5 || cnt == 7))
                {
                    GetMultiColorBlend(rand01(),false,color);
                    SetPixel(x,y,color);
                }
                break;
            case 3:
                // B378/S235678
                if (isLive && (cnt == 2 || cnt == 3 || cnt >= 5))
                {
                    SetPixel(x,y,color);
                }
                else if (!isLive && (cnt == 3 || cnt == 7 || cnt == 8))
                {
                    GetMultiColorBlend(rand01(),false,color);
                    SetPixel(x,y,color);
                }
                break;
            case 4:
                // B25678/S5678
                if (isLive && (cnt >= 5))
                {
                    SetPixel(x,y,color);
                }
                else if (!isLive && (cnt == 2 || cnt >= 5))
                {
                    GetMultiColorBlend(rand01(),false,color);
                    SetPixel(x,y,color);
                }
                break;
            }
        }
    }
    // copy new life state to tempbuf
    tempbuf=pixels;
}

void RgbEffects::RenderMeteors(int MeteorType, int Count, int Length)
{
    if (state == 0) meteors.clear();
    int mspeed=state/4;
    state-=mspeed*4;

    // create new meteors
    MeteorClass m;
    wxImage::HSVValue hsv,hsv0,hsv1;
    palette.GetHSV(0,hsv0);
    palette.GetHSV(1,hsv1);
    size_t colorcnt=GetColorCount();
    Count=BufferWi * Count / 100;
    int TailLength=(BufferHt < 10) ? Length / 10 : BufferHt * Length / 100;
    if (TailLength < 1) TailLength=1;
    int TailStart=BufferHt - TailLength;
    if (TailStart < 1) TailStart=1;
    for(int i=0; i<Count; i++)
    {
        m.x=rand() % BufferWi;
        m.y=BufferHt - 1 - rand() % TailStart;
        switch (MeteorType)
        {
        case 1:
            SetRangeColor(hsv0,hsv1,m.hsv);
            break;
        case 2:
            palette.GetHSV(rand()%colorcnt, m.hsv);
            break;
        }
        meteors.push_back(m);
    }

    // render meteors
    for (MeteorList::iterator it=meteors.begin(); it!=meteors.end(); ++it)
    {
        for(int ph=0; ph<TailLength; ph++)
        {
            switch (MeteorType)
            {
            case 0:
                hsv.hue=double(rand() % 1000) / 1000.0;
                hsv.saturation=1.0;
                hsv.value=1.0;
                break;
            default:
                hsv=it->hsv;
                break;
            }
            hsv.value*=1.0 - double(ph)/TailLength;
            SetPixel(it->x,it->y+ph,hsv);
        }
        it->y -= mspeed;
    }

    // delete old meteors
    meteors.remove_if(MeteorHasExpired(TailLength));
}

void RgbEffects::RenderPictures(int dir, const wxString& NewPictureName)
{
    const int speedfactor=4;
    if (NewPictureName != PictureName)
    {
        if (!image.LoadFile(NewPictureName))
        {
            //wxMessageBox("Error loading image file: "+NewPictureName);
            image.Clear();
        }
        PictureName=NewPictureName;
    }
    if (!image.IsOk()) return;
    int imgwidth=image.GetWidth();
    int imght=image.GetHeight();
    int yoffset=(BufferHt+imght)/2;
    int xoffset=(imgwidth-BufferWi)/2;
    int limit=(dir < 2) ? imgwidth+BufferWi : imght+BufferHt;
    int movement=(state % (limit*speedfactor)) / speedfactor;

    // copy image to buffer
    wxColour c;
    for(int x=0; x<imgwidth; x++)
    {
        for(int y=0; y<imght; y++)
        {
            if (!image.IsTransparent(x,y))
            {
                c.Set(image.GetRed(x,y),image.GetGreen(x,y),image.GetBlue(x,y));
                switch (dir)
                {
                case 0:
                    // left
                    SetPixel(x+BufferWi-movement,yoffset-y,c);
                    break;
                case 1:
                    // right
                    SetPixel(x+movement-imgwidth,yoffset-y,c);
                    break;
                case 2:
                    // up
                    SetPixel(x-xoffset,movement-y,c);
                    break;
                case 3:
                    // down
                    SetPixel(x-xoffset,BufferHt+imght-y-movement,c);
                    break;
                default:
                    // no movement - centered
                    SetPixel(x-xoffset,yoffset-y,c);
                    break;
                }
            }
        }
    }
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
void RgbEffects::RenderSnowflakes(int Count, int SnowflakeType)
{
    int i,n,x,y0,y,check,delta_y;
    wxColour color1,color2;
    if (state == 0 || Count != LastSnowflakeCount || SnowflakeType != LastSnowflakeType)
    {
        // initialize
        LastSnowflakeCount=Count;
        LastSnowflakeType=SnowflakeType;
        palette.GetColor(0,color1);
        palette.GetColor(1,color2);
        ClearTempBuf();
        // place Count snowflakes
        for (n=0; n < Count; n++)
        {
            delta_y=BufferHt/4;
            y0=(n % 4)*delta_y;
            if (y0+delta_y > BufferHt) delta_y = BufferHt-y0;
            // find unused space
            for (check=0; check < 20; check++)
            {
                x=rand() % BufferWi;
                y=y0 + (rand() % delta_y);
                if (GetTempPixelRGB(x,y) == 0) break;
            }
            // draw flake, SnowflakeType=0 is random type
            switch (SnowflakeType == 0 ? rand() % 5 : SnowflakeType-1)
            {
            case 0:
                // single node
                SetTempPixel(x,y,color1);
                break;
            case 1:
                // 5 nodes
                if (x < 1) x+=1;
                if (y < 1) y+=1;
                if (x > BufferWi-2) x-=1;
                if (y > BufferHt-2) y-=1;
                SetTempPixel(x,y,color1);
                SetTempPixel(x-1,y,color2);
                SetTempPixel(x+1,y,color2);
                SetTempPixel(x,y-1,color2);
                SetTempPixel(x,y+1,color2);
                break;
            case 2:
                // 3 nodes
                if (x < 1) x+=1;
                if (y < 1) y+=1;
                if (x > BufferWi-2) x-=1;
                if (y > BufferHt-2) y-=1;
                SetTempPixel(x,y,color1);
                if (rand() % 100 > 50)      // % 2 was not so random
                {
                    SetTempPixel(x-1,y,color2);
                    SetTempPixel(x+1,y,color2);
                }
                else
                {
                    SetTempPixel(x,y-1,color2);
                    SetTempPixel(x,y+1,color2);
                }
                break;
            case 3:
                // 9 nodes
                if (x < 2) x+=2;
                if (y < 2) y+=2;
                if (x > BufferWi-3) x-=2;
                if (y > BufferHt-3) y-=2;
                SetTempPixel(x,y,color1);
                for (i=1; i<=2; i++)
                {
                    SetTempPixel(x-i,y,color2);
                    SetTempPixel(x+i,y,color2);
                    SetTempPixel(x,y-i,color2);
                    SetTempPixel(x,y+i,color2);
                }
                break;
            case 4:
                // 13 nodes
                if (x < 2) x+=2;
                if (y < 2) y+=2;
                if (x > BufferWi-3) x-=2;
                if (y > BufferHt-3) y-=2;
                SetTempPixel(x,y,color1);
                SetTempPixel(x-1,y,color2);
                SetTempPixel(x+1,y,color2);
                SetTempPixel(x,y-1,color2);
                SetTempPixel(x,y+1,color2);

                SetTempPixel(x-1,y+2,color2);
                SetTempPixel(x+1,y+2,color2);
                SetTempPixel(x-1,y-2,color2);
                SetTempPixel(x+1,y-2,color2);
                SetTempPixel(x+2,y-1,color2);
                SetTempPixel(x+2,y+1,color2);
                SetTempPixel(x-2,y-1,color2);
                SetTempPixel(x-2,y+1,color2);
                break;
            case 5:
                // 45 nodes (not enabled)
                break;
            }
        }
    }

    // move snowflakes
    int new_x,new_y,new_x2,new_y2;
    for (x=0; x<BufferWi; x++)
    {
        new_x = (x+state/20) % BufferWi; // CW
        new_x2 = (x-state/20) % BufferWi; // CCW
        if (new_x2 < 0) new_x2+=BufferWi;
        for (y=0; y<BufferHt; y++)
        {
            new_y = (y+state/10) % BufferHt;
            new_y2 = (new_y + BufferHt/2) % BufferHt;
            GetTempPixel(new_x,new_y,color1);
            if (color1.GetRGB() == 0) GetTempPixel(new_x2,new_y2,color1);
            SetPixel(x,y,color1);
        }
    }
}

// 0 <= idx <= 7
wxPoint RgbEffects::SnowstormVector(int idx)
{
    wxPoint xy;
    switch (idx)
    {
    case 0:
        xy.x=-1;
        xy.y=0;
        break;
    case 1:
        xy.x=-1;
        xy.y=-1;
        break;
    case 2:
        xy.x=0;
        xy.y=-1;
        break;
    case 3:
        xy.x=1;
        xy.y=-1;
        break;
    case 4:
        xy.x=1;
        xy.y=0;
        break;
    case 5:
        xy.x=1;
        xy.y=1;
        break;
    case 6:
        xy.x=0;
        xy.y=1;
        break;
    default:
        xy.x=-1;
        xy.y=1;
        break;
    }
    return xy;
}

void RgbEffects::SnowstormAdvance(SnowstormClass& ssItem)
{
    const int cnt = 8;  // # of integers in each set in arr[]
    const int arr[] = {30,20,10,5,0,5,10,20,20,15,10,10,10,10,10,15}; // 2 sets of 8 numbers, each of which add up to 100
    wxPoint adv = SnowstormVector(7);
    int i0 = ssItem.idx % 7 <= 4 ? 0 : cnt;
    int r=rand() % 100;
    for(int i=0, val=0; i < cnt; i++)
    {
        val+=arr[i0+i];
        if (r < val)
        {
            adv=SnowstormVector(i);
            break;
        }
    }
    if (ssItem.idx % 3 == 0)
    {
        adv.x *= 2;
        adv.y *= 2;
    }
    wxPoint xy=ssItem.points.back()+adv;
    xy.x %= BufferWi;
    xy.y %= BufferHt;
    if (xy.x < 0) xy.x+=BufferWi;
    if (xy.y < 0) xy.y+=BufferHt;
    ssItem.points.push_back(xy);
}

void RgbEffects::RenderSnowstorm(int Count, int Length)
{
    // create new meteors
    wxImage::HSVValue hsv,hsv0,hsv1;
    palette.GetHSV(0,hsv0);
    palette.GetHSV(1,hsv1);
    size_t colorcnt=GetColorCount();
    Count=BufferWi * BufferHt * Count / 2000 + 1;
    int TailLength=BufferWi * BufferHt * Length / 2000 + 2;
    SnowstormClass ssItem;
    wxPoint xy;
    int r;

    if (state == 0 || Count != LastSnowstormCount)
    {
        // create snowstorm elements
        LastSnowstormCount=Count;
        SnowstormItems.clear();
        for(int i=0; i<Count; i++)
        {
            ssItem.idx=i;
            ssItem.ssDecay=0;
            ssItem.points.clear();
            SetRangeColor(hsv0,hsv1,ssItem.hsv);
            // start in a random state
            r=rand() % (2*TailLength);
            if (r > 0)
            {
                xy.x=rand() % BufferWi;
                xy.y=rand() % BufferHt;
                ssItem.points.push_back(xy);
            }
            if (r >= TailLength)
            {
                ssItem.ssDecay = r - TailLength;
                r = TailLength;
            }
            for (int j=1; j < r; j++)
            {
                SnowstormAdvance(ssItem);
            }
            SnowstormItems.push_back(ssItem);
        }
    }

    // render Snowstorm Items
    int sz;
    int cnt=0;
    for (SnowstormList::iterator it=SnowstormItems.begin(); it!=SnowstormItems.end(); ++it)
    {
        if (it->points.size() > TailLength)
        {
            if (it->ssDecay > TailLength)
            {
                it->points.clear();  // start over
                it->ssDecay=0;
            }
            else if (rand() % 20 < speed)
            {
                it->ssDecay++;
            }
        }
        if (it->points.empty())
        {
            xy.x=rand() % BufferWi;
            xy.y=rand() % BufferHt;
            it->points.push_back(xy);
        }
        else if (rand() % 20 < speed)
        {
            SnowstormAdvance(*it);
        }
        sz=it->points.size();
        for(int pt=0; pt < sz; pt++)
        {
            hsv=it->hsv;
            hsv.value=1.0 - double(sz - pt + it->ssDecay)/TailLength;
            if (hsv.value < 0.0) hsv.value=0.0;
            SetPixel(it->points[pt].x,it->points[pt].y,hsv);
        }
        cnt++;
    }
}

void RgbEffects::RenderSpirals(int PaletteRepeat, int Direction, int Rotation, int Thickness, bool Blend, bool Show3D)
{
    int strand_base,strand,thick,x,y,ColorIdx;
    size_t colorcnt=GetColorCount();
    int SpiralCount=colorcnt * PaletteRepeat;
    int deltaStrands=BufferWi / SpiralCount;
    int SpiralThickness=(deltaStrands * Thickness / 100) + 1;
    long SpiralState=state*Direction;
    wxImage::HSVValue hsv;
    wxColour color;
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

void RgbEffects::RenderText(int Top, const wxString& Line1, const wxString& Line2, const wxString& FontString, int dir)
{
    wxColour c;
    wxBitmap bitmap(BufferWi,BufferHt);
    wxMemoryDC dc(bitmap);
    wxFont font;
    font.SetNativeFontInfoUserDesc(FontString);
    dc.SetFont(font);
    palette.GetColor(0,c);
    dc.SetTextForeground(c);
    wxString msg = Line1;
    if (!Line2.IsEmpty()) msg+=wxT("\n")+Line2;
    wxSize sz1 = dc.GetTextExtent(Line1);
    wxSize sz2 = dc.GetTextExtent(Line2);
    int maxwidth=sz1.GetWidth() > sz2.GetWidth() ? sz1.GetWidth() : sz2.GetWidth();
    int maxht=sz1.GetHeight() > sz2.GetHeight() ? sz1.GetHeight() : sz2.GetHeight();
    int dctop=Top * BufferHt / 50 - BufferHt/2;
    int xlimit=(BufferWi+maxwidth)*8 + 1;
    int ylimit=(BufferHt+maxht)*8 + 1;
    int xcentered=(BufferWi-maxwidth)/2;
    switch (dir)
    {
    case 0:
        // left
        dc.DrawText(msg,BufferWi-state % xlimit/8,dctop);
        break;
    case 1:
        // right
        dc.DrawText(msg,state % xlimit/8-BufferWi,dctop);
        break;
    case 2:
        // up
        dc.DrawText(msg,xcentered,BufferHt-state % ylimit/8);
        break;
    case 3:
        // down
        dc.DrawText(msg,xcentered,state % ylimit / 8 - BufferHt);
        break;
    default:
        // no movement - centered
        dc.DrawText(msg,xcentered,dctop);
        break;
    }

    // copy dc to buffer
    for(wxCoord x=0; x<BufferWi; x++)
    {
        for(wxCoord y=0; y<BufferHt; y++)
        {
            dc.GetPixel(x,BufferHt-y-1,&c);
            SetPixel(x,y,c);
        }
    }
}

void RgbEffects::RenderTwinkle(int Count)
{

    int x,y,i,i7,r,ColorIdx;
    int lights = (BufferHt*BufferWi)*(Count/100.0); // Count is in range of 1-100 from slider bar
    int step=BufferHt*BufferWi/lights;
    if(step<1) step=1;
    srand(1); // always have the same random numbers for each frame (state)
    wxImage::HSVValue hsv; //   we will define an hsv color model. The RGB colot model would have been "wxColour color;"

    size_t colorcnt=GetColorCount();

    i=0;

    for (y=0; y<BufferHt; y++) // For my 20x120 megatree, BufferHt=120
    {
        for (x=0; x<BufferWi; x++) // BufferWi=20 in the above example
        {
            i++;
            if(i%step==0) // Should we draw a light?
            {
                // Yes, so now decide on what color it should be

                ColorIdx=rand() % colorcnt; // Select random numbers from 0 up to number of colors the user has checked. 0-5 if 6 boxes checked
                palette.GetHSV(ColorIdx, hsv); // Now go and get the hsv value for this ColorIdx
                i7=(state/4+rand())%9; // Our twinkle is 9 steps. 4 ramping up, 5th at full brightness and then 4 more ramping down
                //  Note that we are adding state to this calculation, this causes a different blink rate for each light

                if(i7==0 || i7==8)  hsv.value = 0.1;
                if(i7==1 || i7==7)  hsv.value = 0.3;
                if(i7==2 || i7==6)  hsv.value = 0.5;
                if(i7==3 || i7==5)  hsv.value = 0.7;
                if(i7==4  )  hsv.value = 1.0;
                //  we left the Hue and Saturation alone, we are just modifiying the Brightness Value
                SetPixel(x,y,hsv); // Turn pixel on
            }
        }
    }
}


void RgbEffects::RenderTree(int Branches)
{

    int x,y,i,i7,r,ColorIdx,Count,pixels_per_branch;
    int maxFrame,mod,branch,row,b,f_mod,m,frame;
    int number_garlands,f_mod_odd,s_odd_row,odd_even;
    float V,H;

    number_garlands=1;
    srand(1); // always have the same random numbers for each frame (state)
    wxImage::HSVValue hsv; //   we will define an hsv color model. The RGB colot model would have been "wxColour color;"
    pixels_per_branch=(int)(0.5+BufferHt/Branches);
    maxFrame=(Branches+1) *BufferWi;
    size_t colorcnt=GetColorCount();
    frame = (state/4)%maxFrame;

    i=0;

    for (y=0; y<BufferHt; y++) // For my 20x120 megatree, BufferHt=120
    {
        for (x=0; x<BufferWi; x++) // BufferWi=20 in the above example
        {
            mod=y%pixels_per_branch;
            if(mod==0) mod=pixels_per_branch;
            V=1-(1.0*mod/pixels_per_branch)*0.70;
            i++;

            ColorIdx=rand() % colorcnt; // Select random numbers from 0 up to number of colors the user has checked. 0-5 if 6 boxes checked
            ColorIdx=0;
            palette.GetHSV(ColorIdx, hsv); // Now go and get the hsv value for this ColorIdx
            hsv.value = V; // we have now set the color for the background tree

            //   $orig_rgbval=$rgb_val;
            branch = (int)((y-1)/pixels_per_branch);
            row = pixels_per_branch-mod; // now row=0 is bottom of branch, row=1 is one above bottom
            //  mod = which pixel we are in the branch
            //	mod=1,row=pixels_per_branch-1   top picrl in branch
            //	mod=2, second pixel down into branch
            //	mod=pixels_per_branch,row=0  last pixel in branch
            //
            //	row = 0, the $p is in the bottom row of tree
            //	row =1, the $p is in second row from bottom
            b = (int) ((state)/BufferWi)%Branches; // what branch we are on based on frame #
            //
            //	b = 0, we are on bottomow row of tree during frames 1 to BufferWi
            //	b = 1, we are on second row from bottom, frames = BufferWi+1 to 2*BufferWi
            //	b = 2, we are on third row from bottome, frames - 2*BufferWi+1 to 3*BufferWi
            f_mod = (state/4)%BufferWi;
            //   if(f_mod==0) f_mod=BufferWi;
            //	f_mod is  to BufferWi-1 on each row
            //	f_mod == 0, left strand of this row
            //	f_mod==BufferWi, right strand of this row
            //
            m=(x%6);
            if(m==0) m=6;  // use $m to indicate where we are in horizontal pattern
            // m=1, 1sr strand
            // m=2, 2nd strand
            // m=6, last strand in 6 strand pattern



            r=branch%5;
            H = r/4.0;

            odd_even=b%2;
            s_odd_row = BufferWi-x+1;
            f_mod_odd = BufferWi-f_mod+1;

            if(branch<=b && x<=frame && // for branches below or equal to current row
                    (((row==3 or (number_garlands==2 and row==6)) and (m==1 or m==6))
                     ||
                     ((row==2 or (number_garlands==2 and row==5)) and (m==2 or m==5))
                     ||
                     ((row==1 or (number_garlands==2 and row==4)) and (m==3 or m==4))
                    ))
                if((odd_even ==0 and x<=f_mod) || (odd_even ==1 and s_odd_row<=f_mod))
                {
                    hsv.hue = H;
                    hsv.saturation=1.0;
                    hsv.value=1.0;
                }
            //	if(branch>b)
//	{
//		return $rgb_val; // for branches below current, dont dont balnk anything out
//	}
//	else if(branch==b)
//	{
//		if(odd_even ==0 and x>f_mod)
//		{
//			$rgb_val=$orig_rgbval;// we are even row ,counting from bottom as zero
//		}
//		if(odd_even ==1 and s_odd_row>f_mod)
//		{
//			$rgb_val=$orig_rgbval;// we are even row ,counting from bottom as zero
//		}
//	}
            //if($branch>$b) $rgb_val=$orig_rgbval; // erase rows above our current row.


            // Yes, so now decide on what color it should be


            //  we left the Hue and Saturation alone, we are just modifiying the Brightness Value
            SetPixel(x,y,hsv); // Turn pixel on

        }
    }
}


// for (y=0; y<BufferHt; y++) // For my 20x120 megatree, BufferHt=120
//    {
//        for (x=0; x<BufferWi; x++) // BufferWi=20 in the above example
//        {


void RgbEffects::RenderSpirograph(int int_R, int int_r, int int_d)
{
#define PI 3.14159265
    int i,x,y,k,xc,yc,ColorIdx;
    int mod1440,state360;
    srand(1);
    float R,r,d,d_orig,t;
    wxImage::HSVValue hsv,hsv0,hsv1; //   we will define an hsv color model. The RGB colot model would have been "wxColour color;"
    size_t colorcnt=GetColorCount();

    xc= (int)(BufferWi/2); // 20x100 flex strips with 2 fols per strip = 40x50
    yc= (int)(BufferHt/2);
    R=xc*(int_R/100.0);   //  Radius of the large circle just fits in the width of model
    r=xc*(int_r/100.0); // start little circle at 1/4 of max width
    if(r>R) r=R;
    d=xc*(int_d/100.0);
    ColorIdx=rand() % colorcnt; // Select random numbers from 0 up to number of colors the user has checked. 0-5 if 6 boxes checked
    palette.GetHSV(ColorIdx, hsv); // Now go and get the hsv value for this ColorIdx
    palette.GetHSV(0, hsv0);
    palette.GetHSV(1, hsv1);
//
//    A hypotrochoid is a roulette traced by a point attached to a circle of radius r rolling around the inside of a fixed circle of radius R, where the point is a distance d from the center of the interior circle.
//The parametric equations for a hypotrochoid are:[citation needed]
//
//  more info: http://en.wikipedia.org/wiki/Hypotrochoid
//
//x(t) = (R-r) * cos t + d*cos ((R-r/r)*t);
//y(t) = (R-r) * sin t + d*sin ((R-r/r)*t);

    mod1440=state%1440;
    state360 = state%360;
    d_orig=d;
    for(i=1; i<=360; i++)
    {
        d = (int)(d_orig+state/2)%100;
        t = (i+mod1440)*PI/180;
        x = (R-r) * cos (t) + d*cos ((R-r/r)*t) + xc;
        y = (R-r) * sin (t) + d*sin ((R-r/r)*t) + yc;
        if(i<=state360) SetPixel(x,y,hsv0); // Turn pixel on
        else SetPixel(x,y,hsv1);
    }
}


void RgbEffects::RenderSean(int Count)
{

    int x,y,i,i7,r,ColorIdx;
    int lights = (BufferHt*BufferWi)*(Count/100.0); // Count is in range of 1-100 from slider bar
    int step=BufferHt*BufferWi/lights;
    if(step<1) step=1;
    double damping=0.8;
    srand(1); // always have the same random numbers for each frame (state)
    wxImage::HSVValue hsv,hsv0,hsv1; //   we will define an hsv color model. The RGB colot model would have been "wxColour color;"
    wxColour color;
    int color1,color2,color3,color4,new_color,old_color;
    size_t colorcnt=GetColorCount();
    i=0;
    palette.GetHSV(0, hsv0); // hsv0.hue, hsv0.saturation, hsv0.value
    palette.GetHSV(1, hsv1);


    if(state==0)
    {
        ClearWaveBuffer1();
        ClearWaveBuffer2();
        color1=100;
    }
    if(state%20==0)
    {
        color1=100;

        SetWaveBuffer2((BufferWi+state)%BufferWi,(BufferHt+state)%BufferHt,color1);
        SetWaveBuffer2((BufferWi+state)%BufferWi,(BufferHt+state)%BufferHt,color1);
    }

    for (y=1; y<BufferHt-1; y++) // For my 20x120 megatree, BufferHt=120
    {
        for (x=1; x<BufferWi-1; x++) // BufferWi=20 in the above example
        {
//            GetTempPixel(x,y,color);
//            isLive=(color.GetRGB() != 0);

            old_color=GetWaveBuffer1(x,y) ;
            color1=GetWaveBuffer2(x-1,y);
            color2=GetWaveBuffer2(x+1,y) ;
            color3=GetWaveBuffer2(x,y+1) ;
            color4=GetWaveBuffer2(x,y+2) ;

            new_color = (color1+color2+color3+color4)/2 - old_color;
            new_color = new_color /8.0;
            SetWaveBuffer1(x,y,new_color);
        }
    }

    for (y=0; y<BufferHt; y++) // For my 20x120 megatree, BufferHt=120
    {
        for (x=0; x<BufferWi; x++) // BufferWi=20 in the above example
        {
            new_color=GetWaveBuffer1(x,y);
            if(new_color>= -0.01 && new_color<= 0.01)
            {
                SetPixel(x,y,hsv1);
            }
            else
            {
                if(new_color>0)
                {
                    hsv0.value = (new_color%100)/100.0;
                    SetPixel(x,y,hsv0);
                }
                else
                {
                    hsv0.hue = 0.3;
                    hsv0.value = -(new_color%100)/100.0;
                    SetPixel(x,y,hsv0);
                }
            }
        }
    }
    WaveBuffer0=WaveBuffer2;
    WaveBuffer2=WaveBuffer1;
    WaveBuffer1=WaveBuffer0;
}


//ClearTempBuf();
//
//SetTempPixel(x-1,y+2,color2);
//SetTempPixel(x+1,y+2,color2);
//
//// move snowflakes
//int new_x,new_y,new_x2,new_y2;
//for (x=0; x<BufferWi; x++)
//{
//    new_x = (x+state/20) % BufferWi; // CW
//    new_x2 = (x-state/20) % BufferWi; // CCW
//    if (new_x2 < 0) new_x2+=BufferWi;
//    for (y=0; y<BufferHt; y++)
//    {
//        new_y = (y+state/10) % BufferHt;
//        new_y2 = (new_y + BufferHt/2) % BufferHt;
//        GetTempPixel(new_x,new_y,color1);
//        if (color1.GetRGB() == 0) GetTempPixel(new_x2,new_y2,color1);
//        SetPixel(x,y,color1);
//    }
//}
//
//// copy new life state to tempbuf
//tempbuf=pixels;

