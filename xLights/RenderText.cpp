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

    long DefaultPixelHt=BufferHt/2;
//    if (DefaultPixelHt < 10) DefaultPixelHt=10; // min height
    if (DefaultPixelHt < 8) DefaultPixelHt=8; // min height; allow smaller grids -DJ
    wxSize pixelSize(0,DefaultPixelHt);
    wxFont Font1(pixelSize,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
    wxFont Font2(pixelSize,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
    if (!FontString1.IsEmpty()) {
        Font1.SetNativeFontInfoUserDesc(FontString1);
    }
    if (!FontString2.IsEmpty()) {
        Font2.SetNativeFontInfoUserDesc(FontString2);
    }

#ifndef __WXMSW__
    // wxMemoryDC is not cleared on creation
    dc.SetBackground(*wxBLACK_BRUSH);
    dc.Clear();
#endif

#ifdef __WXMSW__
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
    s.Replace(";2;",";3;",false);
    Font1.SetNativeFontInfo(s);
    s = Font2.GetNativeFontInfoDesc();
    s.Replace(";2;",";3;",false);
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

    //convert to image to get the pixel data
    wxImage image(bitmap.ConvertToImage());
    for(wxCoord x=0; x<BufferWi; x++)
    {
        for(wxCoord y=0; y<BufferHt; y++)
        {
            c.Set(image.GetRed(x, BufferHt-y-1),
                  image.GetGreen(x, BufferHt-y-1),
                  image.GetBlue(x, BufferHt-y-1));
            SetPixel(x,y,c);
        }
    }
}

// dir is 0: move left, 1: move right, 2: up, 3: down, 4: no movement
// Effect is 0: normal, 1: vertical text down, 2: vertical text up,
//           3: timer in seconds, where Line is the starting value in seconds
//           4: timer in days, hours, minute, seconds, where Line is the target date as YYYYMMDD

//these must match list in xLightsMain.cpp: -DJ
#define COUNTDOWN_NONE  0
#define COUNTDOWN_SECONDS  1
#define COUNTDOWN_D_H_M_S  2
#define COUNTDOWN_H_M_S  3
#define COUNTDOWN_M_or_S  4
#define COUNTDOWN_S  5

//NOTE: parameter in the following is just for readability (rather than using an embedded comment):
#define TEXTDIR_LEFT(comment)   0
#define TEXTDIR_RIGHT(comment)  1
#define TEXTDIR_UP(comment)  2
#define TEXTDIR_DOWN(comment)  3
#define TEXTDIR_NONE(comment)  4
#define TEXTDIR_UPLEFT(comment)  5
#define TEXTDIR_DOWNLEFT(comment)  6
#define TEXTDIR_UPRIGHT(comment)  7
#define TEXTDIR_DOWNRIGHT(comment)  8
#define TEXTDIR_WAVEY_LRUPDOWN(comment)  9
#define TEXTDIR_BOUNCING_LRUPDOWN(comment)  10 //TODO

//provide back-and-forth movement (linear):
//in future this could use exp/log functions for "gravity" type bounces, but for now linear is adequate
#define zigzag(value, range)  \
    ((((value) / (range)) & 1)? \
        (value) % (range): /*increase during odd cycles*/ \
        (range) - (value) % (range) - 1) /*descrease during even cycles*/


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
    case COUNTDOWN_SECONDS:
        // countdown seconds
        if (state==0) {
            if (!Line.ToLong(&tempLong)) tempLong=0;
            timer_countdown[idx] = curPeriod+tempLong*20+19;  // capture 0 period
        }
        seconds=(timer_countdown[idx]-curPeriod)/20;
        if(seconds < 0) seconds=0;
        msg=wxString::Format("%i",seconds);
        break;
    case COUNTDOWN_D_H_M_S:
    case COUNTDOWN_H_M_S:
    case COUNTDOWN_M_or_S:
    case COUNTDOWN_S:
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
            timer_countdown[idx]=longsecs;
        } else {
            longsecs=timer_countdown[idx];
        }
        days = longsecs / 60 / 60 / 24;
        hours = (longsecs / 60 / 60) % 24;
        minutes = (longsecs / 60) % 60;
        seconds = longsecs % 60;
        if (Countdown == COUNTDOWN_D_H_M_S)
            msg=wxString::Format("%dd %dh %dm %ds",days,hours,minutes,seconds);
        else if (Countdown == COUNTDOWN_H_M_S)
            msg = wxString::Format("%d : %d : %d", hours, minutes, seconds);
        else if (Countdown == COUNTDOWN_S)
            msg = wxString::Format("%d", 60*60 * hours + 60 * minutes + seconds);
        else if (60 * hours + minutes < 5) //COUNTDOWN_M_or_S: show seconds
            msg = wxString::Format("%d", 60*60 * hours + 60 * minutes + seconds);
        else //COUNTDOWN_M_or_S: show minutes
            msg = wxString::Format("%d m", 60 * hours + minutes);
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
        case TEXTDIR_LEFT(0): rect.Offset(xlimit/16 - state % xlimit/8, OffsetTop); break; // left
        case TEXTDIR_RIGHT(1): rect.Offset(state % xlimit/8 - xlimit/16, OffsetTop); break; // right
        case TEXTDIR_UP(2): rect.Offset(OffsetLeft, ylimit/16 - state % ylimit/8); break; // up
        case TEXTDIR_DOWN(3): rect.Offset(OffsetLeft, state % ylimit/8 - ylimit/16); break; // down
        case TEXTDIR_UPLEFT(5): rect.Offset(xlimit/16 - state % xlimit/8, ylimit/16 - state % ylimit/8); break; // up-left
        case TEXTDIR_DOWNLEFT(6): rect.Offset(xlimit/16 - state % xlimit/8, state % ylimit/8 - ylimit/16); break; // down-left
        case TEXTDIR_UPRIGHT(7): rect.Offset(state % xlimit/8 - xlimit/16, ylimit/16 - state % ylimit/8); break; // up-right
        case TEXTDIR_DOWNRIGHT(8): rect.Offset(state % xlimit/8 - xlimit/16, state % ylimit/8 - ylimit/16); break; // down-right
        case TEXTDIR_WAVEY_LRUPDOWN(9): rect.Offset(xlimit/16 - state % xlimit/8, zigzag(state/4, totheight)/2 - totheight/4); break; // left-to-right, wavey up-down 1/2 height (too bouncy if full height is used), slow down up/down motion (too fast unless scaled)
        case TEXTDIR_NONE(4): //fall thru to default
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
        case 5: dc.DrawRotatedText(msg, BufferWi - state % xlimit/8 + xoffset, totheight - state % ylimit/8 - yoffset, TextRotation); break; // up-left
        case 6: dc.DrawRotatedText(msg, BufferWi - state % xlimit/8 + xoffset, state % ylimit/8 - yoffset, TextRotation); break; // down-left
        case 7: dc.DrawRotatedText(msg, state % xlimit/8 - txtwidth + xoffset, totheight - state % ylimit/8 - yoffset, TextRotation); break; // up-right
        case 8: dc.DrawRotatedText(msg, state % xlimit/8 - txtwidth + xoffset, state % ylimit/8 - yoffset, TextRotation); break; // down-right
        default: dc.DrawRotatedText(msg, 0, OffsetTop, TextRotation); break; // static
        }
    }
}

