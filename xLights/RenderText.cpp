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
#include <wx/log.h>
#include <wx/font.h>
#include <wx/fontutil.h>

// Render 2 independent strings of text
// FontString is a value that can be fed to SetNativeFontInfoUserDesc
// dir is 0: move left, 1: move right, 2: up, 3: down, 4: 5: , 6: no movement
// Effect is 0: normal, 1: vertical text down, 2: vertical text up,
//           3: timer in seconds, where Line is the starting value in seconds
//           4: timer in days, hours, minute, seconds, where Line is the target date as YYYYMMDD
void RgbEffects::RenderText(int Position1, const wxString& Line1, const wxString& FontString1,int dir1,int TextRotation1,int Effect1,
                            int Position2, const wxString& Line2, const wxString& FontString2,int dir2,int TextRotation2,int Effect2)
{
    wxColour c;
    wxBitmap bitmap(BufferWi,BufferHt);
    wxMemoryDC dc(bitmap);
    //wxNativeFontInfo NFont1,NFont2;  // cleartype off

    long DefaultPixelHt=BufferHt/2;
    if (DefaultPixelHt < 10) DefaultPixelHt=10; // min height
    wxSize pixelSize(0,DefaultPixelHt);
    wxFont Font1(pixelSize,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
    wxFont Font2(pixelSize,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
    //Font1.m_nativeFontInfo->lf.lfQuality=3; // NONANTIALIASED_QUALITY
    if (!FontString1.IsEmpty()) {
        Font1.SetNativeFontInfoUserDesc(FontString1);
    }
    if (!FontString2.IsEmpty()) {
        Font2.SetNativeFontInfoUserDesc(FontString2);
    }

    wxString s = Font1.GetNativeFontInfoDesc();
    s.Replace(wxT(";2;"),wxT(";3;"),false);
    /*
    s.Printf(wxS("%d;%ld;%ld;%ld;%ld;%ld;%d;%d;%d;%d;%d;%d;%d;%d;%s"),
             0, // version, in case we want to change the format later
             lf.lfHeight,
             lf.lfWidth,
             lf.lfEscapement,
             lf.lfOrientation,
             lf.lfWeight,
             lf.lfItalic,
             lf.lfUnderline,
             lf.lfStrikeOut,
             lf.lfCharSet,
             lf.lfOutPrecision,
             lf.lfClipPrecision,
             lf.lfQuality,
             lf.lfPitchAndFamily,
             lf.lfFaceName);*/
    Font1.SetNativeFontInfo(s);
/*
    LOGFONT lf;
    lf.lfQuality=3; // NONANTIALIASED_QUALITY
    lf.lfOrientation=0;
    lf.lfWeight=400;  // standard weight (not bold)
    lf.lfItalic=0;
    lf.lfUnderline=0;
    lf.lfStrikeOut=0;
    lf.lfHeight=-DefaultPixelHt;*/
    //Font1.SetNativeFontInfo(wxNativeFontInfo(lf));
    //Font1.SetFaceName(wxT("Arial"));
    //Font1.SetPointSize(10);
/*
*/
    //wxNativeFontInfo NFont2(lf);
    //NFont2.SetFaceName(wxT("Arial"));
/*
        //NFont1.InitFromFont(UserFont);
        //const wxNativeFontInfo *info=UserFont.GetNativeFontInfo();
        //wxString msg = wxNativeFontInfo(lf).ToString();
        //wxLogDebug(msg);
        //lf.lfHeight=-UserFont.GetPixelSize().GetY();
        NFont1.SetFaceName(UserFont.GetFaceName());
        //NFont1.SetPixelSize(UserFont.GetPixelSize());
        NFont1.SetWeight(UserFont.GetWeight());
        NFont1.SetStyle(UserFont.GetStyle());
        NFont1.SetUnderlined(UserFont.GetUnderlined());
    }
*/
    //Font2.SetNativeFontInfo(NFont2);
    dc.SetFont(Font1);
    size_t colorcnt=GetColorCount();
    palette.GetColor(0,c);
    dc.SetTextForeground(c);
    RenderTextLine(dc,0,Position1,Line1,dir1,TextRotation1,Effect1);
/*
    if (!FontString2.IsEmpty()) {
        UserFont.SetNativeFontInfoUserDesc(FontString2);
        Font2.SetPointSize(UserFont.GetPointSize());
        Font2.SetFaceName(UserFont.GetFaceName());
        Font2.SetWeight(UserFont.GetWeight());
        Font2.SetStyle(UserFont.GetStyle());
        Font2.SetUnderlined(UserFont.GetUnderlined());
    }
    dc.SetFont(Font2);
*/
    if(colorcnt>1)
    {
        palette.GetColor(1,c); // scm 7-18-13. added if,. only pull color if we have at least two colors checked in palette
        dc.SetTextForeground(c);
    }
    RenderTextLine(dc,1,Position2,Line2,dir2,TextRotation2,Effect2);

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

// dir is 0: move left, 1: move right, 2: up, 3: down, 4: no movement
// Effect is 0: normal, 1: vertical text down, 2: vertical text up,
//           3: timer in seconds, where Line is the starting value in seconds
//           4: timer in days, hours, minute, seconds, where Line is the target date as YYYYMMDD
void RgbEffects::RenderTextLine(wxMemoryDC& dc, int idx, int Position, const wxString& Line, int dir, int TextRotation, int Effect)
{
    long tempLong,longsecs;
    wxString msg;
    int i,days,hours,minutes,seconds;
    if (Line.IsEmpty()) return;
    switch(Effect)
    {
    case 1:
        for(i=0; i<Line.length(); i++)
        {
            msg = msg + Line.GetChar(i) + "\n";
        }
        break;
    case 2:
        for(i=0; i<Line.length(); i++)
        {
            msg = msg + Line.GetChar(Line.length()-i-1) + "\n";
        }
        break;
    case 3:
        // countdown seconds
        tempLong=0;
        if (state==0 && Line.ToLong(&tempLong)) {
            old_longsecs[idx]=-1;
            timer_countdown[idx]=tempLong+1; // set their counter one higher since the first thing we do is subtract one from it.
        }
        longsecs=wxGetUTCTime();
        if(longsecs != old_longsecs[idx]) timer_countdown[idx]--;
        old_longsecs[idx]=longsecs;
        if(timer_countdown[idx] < 0) timer_countdown[idx]=0;
        msg=wxString::Format(wxT("%i"),timer_countdown[idx]);
        break;
    case 4:
        // countdown to date
        tempLong=0;
        if (state==0 && Line.ToLong(&tempLong)) {
            old_longsecs[idx]=-1;
            timer_countdown[idx]=tempLong+1; // set their counter one higher since the first thing we do is subtract one from it.
        }
        longsecs=wxGetUTCTime();
        if(longsecs != old_longsecs[idx]) timer_countdown[idx]--;
        old_longsecs[idx]=longsecs;
        if(timer_countdown[idx] < 0) timer_countdown[idx]=0;
        days = timer_countdown[idx] / 60 / 60 / 24;
        hours = (timer_countdown[idx] / 60 / 60) % 24;
        minutes = (timer_countdown[idx] / 60) % 60;
        seconds = timer_countdown[idx] % 60;
        msg=wxString::Format(wxT("%i d %i h %i m %i s"),days,hours,minutes,seconds);
        break;
    default:
        msg=Line;
        break;
    }

    wxRect rect(0,0,BufferWi,BufferHt);
    wxSize textsize = dc.GetMultiLineTextExtent(msg);

    int totwidth=BufferWi+textsize.GetWidth();
    int totheight=BufferHt+textsize.GetHeight();
    int OffsetLeft= totwidth/2 - (Position * totwidth / 100);
    int OffsetTop= totheight/2 - (Position * totheight / 100);
    int xlimit=totwidth*8 + 1;
    int ylimit=totheight*8 + 1;

    switch (dir)
    {
    case 0: rect.Offset(xlimit/16 - state % xlimit/8, OffsetTop); break; // left
    case 1: rect.Offset(state % xlimit/8 - xlimit/16, OffsetTop); break; // right
    case 2: rect.Offset(OffsetLeft, ylimit/16 - state % ylimit/8); break; // up
    case 3: rect.Offset(OffsetLeft, state % ylimit/8 - ylimit/16); break; // down
    default: rect.Offset(0, OffsetTop); break; // static
    }
    dc.DrawLabel(msg,rect,wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL);
}

