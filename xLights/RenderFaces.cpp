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
#include "xLightsMain.h" //xLightsFrame


#define WANT_DEBUG_IMPL
#define WANT_DEBUG  99
#include "djdebug.cpp"

void RgbEffects::RenderFaces(int Phoneme)
{
    int ColorIdx;
    size_t i,idx;
    int n,x,y,s;

    /*
        FacesPhoneme.Add("AI");     0
        FacesPhoneme.Add("E");      1
        FacesPhoneme.Add("FV");     2
        FacesPhoneme.Add("L");      3
        FacesPhoneme.Add("MBP");    4
        FacesPhoneme.Add("O");      5
        FacesPhoneme.Add("U");      6
        FacesPhoneme.Add("WQ");     7
        FacesPhoneme.Add("etc");    8
        FacesPhoneme.Add("rest");   9
    */
    wxColour color;
    wxImage::HSVValue hsv;
    int maxframe=BufferHt*2;
    int frame=(BufferHt * state / 200)%maxframe;
    double offset=double(state)/100.0;
    size_t colorcnt=GetColorCount();


    std::vector<int> chmap;
    chmap.resize(BufferHt * BufferWi,0);
    wxString html = "<html><body><table border=0>";
    int Ht, Wt;
    Ht = BufferHt;
    Wt = BufferWi;
    mouth( Phoneme, Ht,  Wt); // draw a mouth syllable


//size_t NodeCount=GetNodeCount();

//    above is from ModelClass::ChannelLayoutHtml()

#if 1 //DEBUG
//get list of models:
    wxString buf;
    for (auto it = xLightsFrame::PreviewModels.begin(); it != xLightsFrame::PreviewModels.end(); ++it)
        buf += ", " + (*it)->name; //ModelClassPtr*
    debug(1, "faces: models = %s", (const char*)buf + 2);

//get info about one of the models:
    buf = xLightsFrame::PreviewModels[0]->name;
    debug(1, "first model is %s", (const char*)buf);
    buf = xLightsFrame::PreviewModels[0]->ChannelLayoutHtml();
    if (buf.size() > 500) buf.resize(500);
    debug(1, "first 500 char of layout html = %s", (const char*)buf);
#endif



}

void RgbEffects::mouth(int Phoneme,int BufferHt, int BufferWi)
{
    /*
    FacesPhoneme.Add("AI");     0
    FacesPhoneme.Add("E");      1
    FacesPhoneme.Add("FV");     2
    FacesPhoneme.Add("L");      3
    FacesPhoneme.Add("MBP");    4
    FacesPhoneme.Add("O");      5
    FacesPhoneme.Add("U");      6
    FacesPhoneme.Add("WQ");     7
    FacesPhoneme.Add("etc");    8
    FacesPhoneme.Add("rest");   9

    ...............|...............
    ...............|...............
    ...............|...............
    ...............|...............
    ...............|...............
    ...............|...............
    ...............|...............
    ...............|...............
    ...............|...............
    ...............|...............
    ...............|...............
    ...............|...............
    ...............|...............
    ...............|...............
    ...............|...............
    ...............|...............
    -1-------------+-------------1-         5% and 95% in x, 52% in y
    .1.............|.............1.         55% in y
    .11111111111111111111111111111.         58% in y
    .2..........55.|.55..........2.
    .2........55...|...55........2.
    .2......55.....|.....55......2.
    .2....55.......|.......55....2.
    .2..55.........|.........55..2.
    .255...........|...........552.
    .2.............|.............2.      y 81%
    .2.............|.............2.
    .255...........|...........552.
    .2..55.........|.........55..2.
    .2....55.......|.......55....2.
    .2......55.....|.....55......2.
    .2........55...|...55........2.
    ............55.|.55............
    */

    int Wt, Ht, x1,x2,x3,x4,y1,y2,y3,y4,y5,y6,y7;
    int xc,yc;
    double radius,offset=0.0;
    Ht = BufferHt-1;
    Wt = BufferWi-1;
    x1=(int)(offset + Wt*0.25);
    x2=(int)(offset + Wt*0.75);
    x3=(int)(offset + Wt*0.30);
    x4=(int)(offset + Wt*0.70);

    y1=(int)(offset + Ht*0.48);
    y2=(int)(offset + Ht*0.40);
    y3=(int)(offset + Ht*0.25);
    y4=(int)(offset + Ht*0.20);
    y5=(int)(offset + Ht*0.30);
    y6=(int)(offset + Ht*0.20);
    y7=(int)(offset + Ht*0.30);

    // eyes


    drawoutline(Phoneme,BufferHt,BufferWi);

    switch (Phoneme)
    {
    case 0:         // AI

        drawline1( Phoneme, x1, x2, y1,y2);
        drawline1( Phoneme, x1, x2, y1,y4);
        break;
    case 3:
    case 1:       // E, L
        drawline1( Phoneme, x1, x2, y1,y2);
        drawline1( Phoneme, x1, x2, y1,y3);
        break;
    case 2:       // FV
        drawline1( Phoneme, x1, x2, y1,y2);
        drawline1( Phoneme, x1, x2, y1,y2-1);
        break;
    case 4:
    case 9:     //  MBP,rest

        drawline1( Phoneme, x1, x2, y1,y2);
        break;
    case 5:
    case 6:       // O,U,WQ
    case 7:
        xc = (int)(0.5 + Wt*0.50);
        yc = (int) (y2-y5)/2 + y5;
        radius = Wt*0.15;  // O
        if(Phoneme==6) radius = Wt*0.10;  // U
        if(Phoneme==7) radius = Wt*0.05;  // WQ
        facesCircle(Phoneme,xc, yc, radius,0,360);
        break;
    case 8:       // WQ, etc
        drawline3( Phoneme, x3, x4, y5, y2);
        break;

    }
}

void RgbEffects::drawline1(int Phoneme, int x1,int x2,int y1,int y2)
{
    wxColour color;
    wxImage::HSVValue hsv;
    int ColorIdx,x,y;
    size_t colorcnt=GetColorCount();


    ColorIdx=rand() % colorcnt; // Select random numbers from 0 up to number of colors the user has checked. 0-5 if 6 boxes checked
    palette.GetHSV(ColorIdx, hsv); // Now go and get the hsv value for this ColorIdx
    hsv.hue = (float)Phoneme/10.0;
    hsv.value=1.0;


    for(x=x1+1; x<x2; x++)
    {
        SetPixel(x,y2,hsv); // Turn pixel on
    }
    for(y=y2+1; y<=y1; y++)
    {
        SetPixel(x1,y,hsv); // Left side of mouyh
        SetPixel(x2,y,hsv); // rightside
    }
}



void RgbEffects::drawline3 (int Phoneme, int x1,int x2,int y6,int y7)
{
    wxColour color;
    wxImage::HSVValue hsv;
    double radius;
    int ColorIdx,x,y,xc,yc,dy;
    size_t colorcnt=GetColorCount();


    ColorIdx=rand() % colorcnt; // Select random numbers from 0 up to number of colors the user has checked. 0-5 if 6 boxes checked
    palette.GetHSV(ColorIdx, hsv);
    hsv.hue = (float)Phoneme/10.0;
    hsv.value=1.0;

    for(y=y6+1; y<y7; y++)
    {
        SetPixel(x1,y,hsv); // Left side of mouyh
        SetPixel(x2,y,hsv); // rightside
    }
    for(x=x1+1; x<x2; x++)
    {
        SetPixel(x,y6,hsv); // Bottom
        SetPixel(x,y7,hsv); // Bottom
    }
    /*
    dy = (y7-y6)/2;
    xc = x1+dy;
    yc = y7+dy;
    radius = (double) dy;
    facesCircle(Phoneme,xc, yc, radius,90,270);
    xc = x1-dy;
    facesCircle(Phoneme,xc, yc, radius,270,630);
    */

}
/*
faces draw circle
*/
void RgbEffects::facesCircle(int Phoneme, int xc,int yc,double radius,int start_degrees, int end_degrees)
{
    int x,y,degrees;
    wxImage::HSVValue hsv;
    palette.GetHSV(0, hsv);
    hsv.hue = (float)Phoneme/10.0;
    hsv.value=1.0;
    double angle,t,PI=3.1415926;
    for(degrees=start_degrees; degrees<end_degrees; degrees+=1)
    {
        t = degrees * (PI/180);
        x = (int)xc+radius*cos(t);
        y = (int)yc+radius*sin(t);
        SetPixel(x,y,hsv); // Bottom
    }
}

void RgbEffects::drawoutline(int Phoneme,int BufferHt,int BufferWi)
{
    wxColour color;
    wxImage::HSVValue hsv;
    double radius;
    int ColorIdx,x,y,xc,yc,dy;
    int Ht, Wt;
    size_t colorcnt=GetColorCount();
    Ht = BufferHt-1;
    Wt = BufferWi-1;


    ColorIdx=rand() % colorcnt; // Select random numbers from 0 up to number of colors the user has checked. 0-5 if 6 boxes checked
    palette.GetHSV(ColorIdx, hsv);
    hsv.hue = 0.0;
    hsv.saturation = 1.0;
    hsv.value=1.0;

//  DRAW EYES
    int start_degrees=0;
    int end_degrees=360;
    if(Phoneme==5  or Phoneme==6 or Phoneme==7)
    {
        start_degrees=180;
        end_degrees=360;
    }
    xc = (int)(0.5 + Wt*0.33); // left eye
    yc = (int)(0.5 + Ht*0.75);
    radius = Wt*0.08;
    facesCircle( Phoneme,xc, yc, radius,start_degrees,end_degrees);
    xc = (int)(0.5 + Wt*0.66); // right eye
    yc = (int)(0.5 + Ht*0.75);
    radius = Wt*0.08;
    facesCircle(Phoneme,xc, yc, radius,start_degrees,end_degrees);


    /*
    ...********...
    ..*
    .*
    *
    *
    *
    */

    for(y=3; y<BufferHt-3; y++)
    {
        SetPixel(0,y,hsv); // Left side of mouyh
        SetPixel(BufferWi-1,y,hsv); // rightside
    }
    for(x=3; x<BufferWi-3; x++)
    {
        SetPixel(x,0,hsv); // Bottom
        SetPixel(x,BufferHt-1,hsv); // Bottom
    }
    SetPixel(2,1,hsv); // Bottom left
    SetPixel(1,2,hsv); //

    SetPixel(BufferWi-3,1,hsv); // Bottom Right
    SetPixel(BufferWi-2,2,hsv); //

    SetPixel(BufferWi-3,BufferHt-2,hsv); // Bottom Right
    SetPixel(BufferWi-2,BufferHt-3,hsv); //

    SetPixel(2,BufferHt-2,hsv); // Bottom Right
    SetPixel(1,BufferHt-3,hsv); //
}
