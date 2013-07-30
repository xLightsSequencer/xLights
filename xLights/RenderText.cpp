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
void RgbEffects::RenderText(int Position1, const wxString& Line1, const wxString& FontString1,int dir1,int Effect1,int Countdown1,
                            int Position2, const wxString& Line2, const wxString& FontString2,int dir2,int Effect2,int Countdown2)
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
    if (!FontString1.IsEmpty()) {
        Font1.SetNativeFontInfoUserDesc(FontString1);
    }
    if (!FontString2.IsEmpty()) {
        Font2.SetNativeFontInfoUserDesc(FontString2);
    }

#if defined(__WXMSW__)
    /*
    Here is the format for NativeFontInfo on Windows (taken from the source)
    We want to change lfQuality from 2 to 3 - this disables antialiasing
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
    wxString s = Font1.GetNativeFontInfoDesc();
    s.Replace(wxT(";2;"),wxT(";3;"),false);
    Font1.SetNativeFontInfo(s);
    s = Font2.GetNativeFontInfoDesc();
    s.Replace(wxT(";2;"),wxT(";3;"),false);
    Font2.SetNativeFontInfo(s);
#endif

    dc.SetFont(Font1);
    size_t colorcnt=GetColorCount();
    palette.GetColor(0,c);
    dc.SetTextForeground(c);
    RenderTextLine(dc,0,Position1,Line1,dir1,Effect1,Countdown1);

    dc.SetFont(Font2);
    if(colorcnt>1)
    {
        palette.GetColor(1,c); // scm 7-18-13. added if,. only pull color if we have at least two colors checked in palette
        dc.SetTextForeground(c);
    }
    RenderTextLine(dc,1,Position2,Line2,dir2,Effect2,Countdown2);

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
void RgbEffects::RenderTextLine(wxMemoryDC& dc, int idx, int Position, const wxString& Line, int dir, int Effect, int Countdown)
{
    long tempLong,longsecs;
    wxString msg,tempmsg;
    int i,days,hours,minutes,seconds;
    wxDateTime dt;
    wxTimeSpan ts;
    wxString::const_iterator end;
    if (Line.IsEmpty()) return;

    switch(Countdown)
    {
    case 1:
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
    case 2:
        // countdown to date
        if (state%20 == 0) {
            if ( dt.ParseDateTime(Line, &end) ) {
                // dt is valid, so calc # of seconds until then
                ts=dt.Subtract(wxDateTime::Now());
                wxLongLong ll=ts.GetSeconds();
                if (ll > LONG_MAX) ll=LONG_MAX;
                if (ll < 0) ll=0;
                longsecs=ll.ToLong();
            } else {
                // invalid date/time
                longsecs=0;
            }
            old_longsecs[idx]=longsecs;
        } else {
            longsecs=old_longsecs[idx];
        }
        days = longsecs / 60 / 60 / 24;
        hours = (longsecs / 60 / 60) % 24;
        minutes = (longsecs / 60) % 60;
        seconds = longsecs % 60;
        msg=wxString::Format(wxT("%dd %dh %dm %ds"),days,hours,minutes,seconds);
        break;
    default:
        msg=Line;
        break;
    }

    double TextRotation=0.0;
    switch(Effect)
    {
    case 1:
        // vertical text up
        tempmsg=msg;
        msg.clear();
        for(i=0; i<tempmsg.length(); i++)
        {
            msg = msg + Line.GetChar(tempmsg.length()-i-1) + "\n";
        }
        break;
    case 2:
        // vertical dext down
        tempmsg=msg;
        msg.clear();
        for(i=0; i<tempmsg.length(); i++)
        {
            msg = msg + tempmsg.GetChar(i) + "\n";
        }
        break;
    }

    wxSize textsize = dc.GetMultiLineTextExtent(msg);
    int xoffset=0;
    int yoffset=0;

    switch(Effect)
    {
    case 3:
        // rotate up 45
        TextRotation=45.0;
        yoffset=int(0.707*double(textsize.GetHeight()));
        i=int(0.707*double(textsize.GetWidth()+textsize.GetHeight()));
        textsize.Set(i,i);
        break;
    case 4:
        // rotate up 90
        TextRotation=90.0;
        textsize.Set(textsize.GetHeight(),textsize.GetWidth());  // swap width & height
        break;
    case 5:
        // rotate down 45
        TextRotation=-45.0;
        xoffset=int(0.707*double(textsize.GetHeight()));
        i=int(0.707*double(textsize.GetWidth()+textsize.GetHeight()));
        textsize.Set(i,i);
        yoffset=i;
        break;
    case 6:
        // rotate down 90
        TextRotation=-90.0;
        xoffset=textsize.GetHeight();
        yoffset=textsize.GetWidth();
        textsize.Set(textsize.GetHeight(),textsize.GetWidth());  // swap width & height
        break;
    }
    //msg.Printf(wxS("w=%d, h=%d"),textsize.GetWidth(),textsize.GetHeight());

    int txtwidth=textsize.GetWidth();
    int totwidth=BufferWi+txtwidth;
    int totheight=BufferHt+textsize.GetHeight();
    int OffsetLeft= (Position * totwidth / 100) - totwidth/2;
    int OffsetTop= totheight/2 - (Position * totheight / 100);
    int xlimit=totwidth*8 + 1;
    int ylimit=totheight*8 + 1;

    if (TextRotation == 0.0) {
        wxRect rect(0,0,BufferWi,BufferHt);
        switch (dir)
        {
        case 0: rect.Offset(xlimit/16 - state % xlimit/8, OffsetTop); break; // left
        case 1: rect.Offset(state % xlimit/8 - xlimit/16, OffsetTop); break; // right
        case 2: rect.Offset(OffsetLeft, ylimit/16 - state % ylimit/8); break; // up
        case 3: rect.Offset(OffsetLeft, state % ylimit/8 - ylimit/16); break; // down
        default: rect.Offset(0, OffsetTop); break; // static
        }
        dc.DrawLabel(msg,rect,wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL);
    } else {
        switch (dir)
        {
        case 0: dc.DrawRotatedText(msg, BufferWi - state % xlimit/8 + xoffset, OffsetTop, TextRotation); break; // left
        case 1: dc.DrawRotatedText(msg, state % xlimit/8 - txtwidth + xoffset, OffsetTop, TextRotation); break; // right
        case 2: dc.DrawRotatedText(msg, OffsetLeft, totheight - state % ylimit/8 - yoffset, TextRotation); break; // up
        case 3: dc.DrawRotatedText(msg, OffsetLeft, state % ylimit/8 - yoffset, TextRotation); break; // down
        default: dc.DrawRotatedText(msg, 0, OffsetTop, TextRotation); break; // static
        }
    }
}

