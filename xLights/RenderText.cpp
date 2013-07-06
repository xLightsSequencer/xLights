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
#include <wx/time.h>

void RgbEffects::RenderText(int Position1, const wxString& Line1, const wxString& FontString1,int dir1,int TextRotation1,bool COUNTDOWN1,
                            int Position2, const wxString& Line2, const wxString& FontString2,int dir2,int TextRotation2,bool COUNTDOWN2)
{
    wxColour c;
    wxString vertMsg;
    wxBitmap bitmap(BufferWi,BufferHt);
    wxMemoryDC dc(bitmap);

    wxFont font;
    int ColorIdx,itmp,i;
    long L1,longsecs1,longsecs2,seconds;
    bool COUNTDOWN=true;
    int days,hours,minutes;
    bool DAYS_STRING=true;


    size_t colorcnt=GetColorCount();
    srand(1); // always have the same random numbers for each frame (state)
    wxImage::HSVValue hsv; //   we will define an hsv color model. The RGB colot model would have been "wxColour color;"
    ColorIdx=rand() % colorcnt; // Select random numbers from 0 up to number of colors the user has checked. 0-5 if 6 boxes checked
    palette.GetHSV(ColorIdx, hsv); // Now go and get the hsv value for this ColorIdx

    font.SetNativeFontInfoUserDesc(FontString1);
    dc.SetFont(font);
    palette.GetColor(0,c);
    dc.SetTextForeground(c);
    wxString msg;
    wxSize sz1 = dc.GetTextExtent(Line1);
    int maxwidth=sz1.GetWidth();
    int maxht=sz1.GetHeight();

    L1=0;
    if(state==0 and COUNTDOWN1 and Line1.ToLong(&L1))
    {
        timer_countdown1=L1+1; // set their counter one higher since teh first thing we do it subtract one from it.
    }
    if(state==0 and COUNTDOWN2 and Line2.ToLong(&L1))
    {
        timer_countdown2=L1+1; // we can have concurrent timers, one for each line of text
    }



    if(dir1==4)
    {
        maxht = maxht*Line1.length();
        for(i=0; i<Line1.length(); i++)
        {
            msg = msg + Line1.GetChar(i) + "\n";
        }
    }
    else if(dir1==5)
    {
        maxht = maxht*Line1.length();
        for(i=0; i<Line1.length(); i++)
        {
            msg = msg + Line1.GetChar(Line1.length()-i-1) + "\n";
        }
    }
    else
    {
        msg = Line1;
        if(COUNTDOWN1)
        {
            longsecs1=wxGetUTCTime	()	;
            if(longsecs1 != old_longsecs1)  timer_countdown1--;
            old_longsecs1=longsecs1;
            if(timer_countdown1 < 0) timer_countdown1=0;
            msg=wxString::Format(wxT("%i"),timer_countdown1);

        }
        if(TextRotation1==1)
        {
            itmp=maxwidth;
            maxwidth=maxht;
            maxht=itmp;
        }
    }


    int dctop= Position1 * BufferHt / 50 - BufferHt/2;
    int xlimit=(BufferWi+maxwidth)*8 + 1;
    int ylimit=(BufferHt+maxht)*8 + 1;
//  int xcentered=(BufferWi-maxwidth)/2;  // original way
    int xcentered=Position1 * BufferWi / 50 - BufferWi/2;


    TextRotation1 *=90.0;
    switch (dir1)
    {
    case 0:
        // left
        //dc.DrawText(msg,BufferWi-state % xlimit/8,dctop);

        dc.DrawRotatedText(msg,BufferWi-state % xlimit/8,dctop,TextRotation1);
        break;
    case 1:
        // right
        //dc.DrawText(msg,state % xlimit/8-BufferWi,dctop);
        dc.DrawRotatedText(msg,state % xlimit/8-BufferWi,dctop,TextRotation1);
        break;
    case 2:
        // up
        //  dc.DrawText(msg,xcentered,BufferHt-state % ylimit/8);
        dc.DrawRotatedText(msg,xcentered,BufferHt-state % ylimit/8,TextRotation1);
        break;
    case 3:
        // down
        //  dc.DrawText(msg,xcentered,state % ylimit / 8 - BufferHt);
        dc.DrawRotatedText(msg,xcentered,state % ylimit / 8 - BufferHt,TextRotation1);
        break;
    case 4:
        // vertical text up
        dc.DrawText(msg,xcentered,BufferHt-(state % ylimit/8));
        break;
    case 5:
        // vertical text down
        dc.DrawText(msg,xcentered,(state % ylimit/8) - maxht);
        break;
    default:
        // no movement - centered
        //   dc.DrawText(msg,xcentered,dctop);
        dc.DrawRotatedText(msg,xcentered,dctop,TextRotation1);
        break;
    }


    // Line2
    msg="";
    font.SetNativeFontInfoUserDesc(FontString2);
    dc.SetFont(font);
    palette.GetColor(1,c);
    dc.SetTextForeground(c);
    wxSize sz2 = dc.GetTextExtent(Line2);
    maxwidth=sz2.GetWidth();
    maxht=sz2.GetHeight();

    if(dir2==4)
    {
        maxht = maxht*Line2.length();
        for(i=0; i<Line2.length(); i++)
        {
            msg = msg + Line2.GetChar(i) + "\n";
        }
    }
    else if(dir2==5)
    {
        maxht = maxht*Line2.length();
        for(i=0; i<Line2.length(); i++)
        {
            msg = msg + Line2.GetChar(Line2.length()-i-1) + "\n";
        }
    }
    else
    {
        msg = Line2;
        if(COUNTDOWN2)
        {
            longsecs2=wxGetUTCTime	()	;
            if(longsecs2 != old_longsecs2)  timer_countdown2--;
            old_longsecs2=longsecs2;
            if(timer_countdown2 < 0) timer_countdown2=0;
            if(DAYS_STRING)
            {
                days = timer_countdown2 / 60 / 60 / 24;
                hours = (timer_countdown2 / 60 / 60) % 24;
                minutes = (timer_countdown2 / 60) % 60;
                seconds = timer_countdown2 % 60;
                msg=wxString::Format(wxT("%i d %i h %i m %i s"),days,hours,minutes,seconds);
            }
            else
                {
                    msg=wxString::Format(wxT("%i"),timer_countdown2);
                }
        }
        if(TextRotation2==1)
        {
            itmp=maxwidth;
            maxwidth=maxht;
            maxht=itmp;
        }
    }


    dctop= Position2 * BufferHt / 50 - BufferHt/2;
    xlimit=(BufferWi+maxwidth)*8 + 1;
    ylimit=(BufferHt+maxht)*8 + 1;
//  int xcentered=(BufferWi-maxwidth)/2;  // original way
    xcentered=Position2 * BufferWi / 50 - BufferWi/2;


    TextRotation2 *=90.0;
    switch (dir2)
    {
    case 0:
        // left
        //dc.DrawText(msg,BufferWi-state % xlimit/8,dctop);

        dc.DrawRotatedText(msg,BufferWi-state % xlimit/8,dctop,TextRotation2);
        break;
    case 1:
        // right
        //dc.DrawText(msg,state % xlimit/8-BufferWi,dctop);
        dc.DrawRotatedText(msg,state % xlimit/8-BufferWi,dctop,TextRotation2);
        break;
    case 2:
        // up
        //  dc.DrawText(msg,xcentered,BufferHt-state % ylimit/8);
        dc.DrawRotatedText(msg,xcentered,BufferHt-state % ylimit/8,TextRotation2);
        break;
    case 3:
        // down
        //  dc.DrawText(msg,xcentered,state % ylimit / 8 - BufferHt);
        dc.DrawRotatedText(msg,xcentered,state % ylimit / 8 - BufferHt,TextRotation2);
        break;
    case 4:
        // vertical text up
        dc.DrawText(msg,xcentered,BufferHt-(state % ylimit/8));
        break;
    case 5:
        // vertical text down
        dc.DrawText(msg,xcentered,(state % ylimit/8) - maxht);
        break;
    default:
        // no movement - centered
        //   dc.DrawText(msg,xcentered,dctop);
        dc.DrawRotatedText(msg,xcentered,dctop,TextRotation2);
        break;
    }


    // copy dc to buffer
    for(wxCoord x=0; x<BufferWi; x++)
    {
        for(wxCoord y=0; y<BufferHt; y++)
        {
            dc.GetPixel(x,BufferHt-y-1,&c);
            SetPixel(x,y,c);

//        ColorIdx=(n % BlockHt) / BarHt;
//        palette.GetHSV(ColorIdx, hsv);

        }
    }
}

