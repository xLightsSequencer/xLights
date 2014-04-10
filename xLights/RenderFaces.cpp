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
    mouth( Phoneme, Ht,  Wt);
//size_t NodeCount=GetNodeCount();
    /*
    //  I need this code to work. This should allow me to see what the string and node# are from the custom
    //    model.

      size_t NodeCount=GetNodeCount();
       for(i=0; i<NodeCount; i++)
       {
           idx=Nodes[i]->Coords[0].bufY * BufferWi + Nodes[i]->Coords[0].bufX;
           if (idx < chmap.size()) chmap[idx]=GetNodeNumber(i);
       }
       for(y=BufferHt-1; y>=0; y--)
       {
           html+="<tr>";
           for(x=0; x<BufferWi; x++)
           {
               n=chmap[y*BufferWi+x];
               if (n==0)
               {
                   html+="<td></td>";
               }
               else
               {
                   s=Nodes[n-1]->StringNum+1;
                   bgcolor=s%2 == 1 ? "#ADD8E6" : "#90EE90";
                   html+=wxString::Format("<td bgcolor='"+bgcolor+"'>n%ds%d</td>",n,s);
               }
           }
           html+="</tr>";
       }
    */
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


// the following is just code to generate some "twinkle" lights for now. later
//  we will put in real code
    /*
     for (x=0; x<BufferWi; x++)
     {
         for (y=0; y<BufferHt; y++)
         {
             ColorIdx=rand() % colorcnt; // Select random numbers from 0 up to number of colors the user has checked. 0-5 if 6 boxes checked
             palette.GetHSV(ColorIdx, hsv); // Now go and get the hsv value for this ColorIdx
             SetPixel(x,y,hsv); // Turn pixel on

         }
     }
     */
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
    double radius;
    Ht = BufferHt-1;
    Wt = BufferWi-1;
    x1=(int)(0.5 + Wt*0.15);
    x2=(int)(0.5 + Wt*0.85);
    x3=(int)(0.5 + Wt*0.30);
    x4=(int)(0.5 + Wt*0.70);
    y1=(int)(0.5 + Ht*0.48);
    y2=(int)(0.5 + Ht*0.44);
    y3=(int)(0.5 + Ht*0.05);
    y4=(int)(0.5 + Ht*0.03);
    y5=(int)(0.5 + Ht*0.25);
    y6=(int)(0.5 + Ht*0.20);
    y7=(int)(0.5 + Ht*0.30);

    // eyes
    xc = (int)(0.5 + Wt*0.33); // left eye
    yc = (int)(0.5 + Ht*0.75);
    radius = Wt*0.08;
    facesCircle( Phoneme,xc, yc, radius);
    xc = (int)(0.5 + Wt*0.66); // right eye
    yc = (int)(0.5 + Ht*0.75);
    radius = Wt*0.08;
    facesCircle(Phoneme,xc, yc, radius);


    switch (Phoneme)
    {
    case 0:         // AI
        drawline1( Phoneme, x1, x2, y1,y2);
        drawline1( Phoneme, x1, x2, y1,y3);
        break;
    case 3:
    case 1:       // E,L
        drawline1( Phoneme, x1, x2, y1,y2);
        drawline1( Phoneme, x1, x2, y1,y3);
        break;
    case 2:
    case 4:
    case 9:     // FV, MBP,rest

        drawline1( Phoneme, x1, x2, y1,y2);
        break;
    case 5:
    case 6:       // O,U
        xc = (int)(0.5 + Wt*0.50);
        yc = (int)(0.5 + Ht*0.25);
        radius = Wt*0.20;
        facesCircle(Phoneme,xc, yc, radius);
        break;
    case 7:
    case 8:       // WQ, etc
        drawline3( Phoneme, x3, x4, y6, y7);
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


    for(x=x1; x<=x2; x++)
    {
        SetPixel(x,y2,hsv); // Turn pixel on
    }
    for(y=y1; y<=y2; y++)
    {
        SetPixel(x1,y,hsv); // Left side of mouyh
        SetPixel(x2,y,hsv); // rightside
    }
}

void RgbEffects::drawline2(int Phoneme, int x1,int x2,int y1,int y2,int y3,int y4)
{
    wxColour color;
    wxImage::HSVValue hsv;
    int ColorIdx,x,y;
    size_t colorcnt=GetColorCount();


    ColorIdx=rand() % colorcnt; // Select random numbers from 0 up to number of colors the user has checked. 0-5 if 6 boxes checked
    palette.GetHSV(ColorIdx, hsv);
    hsv.hue = (float)Phoneme/10.0;
    hsv.value=1.0;

    for(y=y1; y<=y4; y++)
    {
        SetPixel(x1,y,hsv); // Left side of mouyh
        SetPixel(x2,y,hsv); // rightside
    }
    for(x=x1; x<=x2; x++)
    {
        SetPixel(x,y4,hsv); // Bottom
    }
}

void RgbEffects::drawline3 (int Phoneme, int x1,int x2,int y6,int y7)
{
    wxColour color;
    wxImage::HSVValue hsv;
    int ColorIdx,x,y;
    size_t colorcnt=GetColorCount();


    ColorIdx=rand() % colorcnt; // Select random numbers from 0 up to number of colors the user has checked. 0-5 if 6 boxes checked
    palette.GetHSV(ColorIdx, hsv);
    hsv.hue = (float)Phoneme/10.0;
    hsv.value=1.0;

    for(y=y6; y<=y7; y++)
    {
        SetPixel(x1,y,hsv); // Left side of mouyh
        SetPixel(x2,y,hsv); // rightside
    }
    for(x=x1; x<=x2; x++)
    {
        SetPixel(x,y6,hsv); // Bottom
        SetPixel(x,y7,hsv); // Bottom
    }
}
/*
faces draw circle
*/
void RgbEffects::facesCircle(int Phoneme, int xc,int yc,double radius)
{
    int x,y,degrees;
    wxImage::HSVValue hsv;
    palette.GetHSV(0, hsv);
    hsv.hue = (float)Phoneme/10.0;
    hsv.value=1.0;
    double angle,t,PI=3.1415926;
    for(degrees=0; degrees<360; degrees+=10)
    {
        t = degrees * (PI/180);
        x = (int)xc+radius*cos(t);
        y = (int)yc+radius*sin(t);
        SetPixel(x,y,hsv); // Bottom
    }
}
