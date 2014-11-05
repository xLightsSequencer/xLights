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


//formatting notes:
//countdown == seconds: put a non-0 value in text line 1 to count down
//countdown == any of the "to date" options: put "Sat, 18 Dec 1999 00:48:30 +0100" in the text line
//countdown = !to date!%fmt: put delimiter + target date + same delimiter + format string with %x markers in it (described down below)


#define WANT_TEXT_LINES_SYNCED //sync text lines together (experimental) -DJ


// Render 4 independent strings of text
// FontString is a value that can be fed to SetNativeFontInfoUserDesc
// dir is 0: move left, 1: move right, 2: up, 3: down, 4: 5: , 6: no movement
// Effect is 0: normal, 1: vertical text down, 2: vertical text up,
//           3: timer in seconds, where Line is the starting value in seconds
//           4: timer in days, hours, minute, seconds, where Line is the target date as YYYYMMDD
void RgbEffects::RenderText(int Position1, const wxString& Line1, const wxString& FontString1,int dir1,bool center1,int Effect1,int Countdown1,
                            int Position2, const wxString& Line2, const wxString& FontString2,int dir2,bool center2,int Effect2,int Countdown2,
                            int Position3, const wxString& Line3, const wxString& FontString3,int dir3,bool center3,int Effect3,int Countdown3,
                            int Position4, const wxString& Line4, const wxString& FontString4,int dir4,bool center4,int Effect4,int Countdown4)
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
    wxFont Font3(pixelSize,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
    wxFont Font4(pixelSize,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
    if (!FontString1.IsEmpty())
    {
        Font1.SetNativeFontInfoUserDesc(FontString1);
    }
    if (!FontString2.IsEmpty())
    {
        Font2.SetNativeFontInfoUserDesc(FontString2);
    }
    if (!FontString3.IsEmpty())
    {
        Font3.SetNativeFontInfoUserDesc(FontString3);
    }
    if (!FontString4.IsEmpty())
    {
        Font4.SetNativeFontInfoUserDesc(FontString4);
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

    s = Font3.GetNativeFontInfoDesc();
    s.Replace(";2;",";3;",false);
    Font3.SetNativeFontInfo(s);

    s = Font4.GetNativeFontInfoDesc();
    s.Replace(";2;",";3;",false);
    Font4.SetNativeFontInfo(s);
#endif

    for (int pass = 0; pass < 2; ++pass)
    {
#ifndef WANT_TEXT_LINES_SYNCED
        if (!pass) continue; //don't need 2 passes
#endif // WANT_TEXT_LINES_SYNCED

        dc.SetFont(Font1);
        size_t colorcnt=GetColorCount();
        palette.GetColor(0,c);
        dc.SetTextForeground(c);
        RenderTextLine(dc,0,Position1,Line1,dir1,center1,Effect1,Countdown1,pass);

        dc.SetFont(Font2);
        if(colorcnt>1)
        {
            palette.GetColor(1,c); // scm 7-18-13. added if,. only pull color if we have at least two colors checked in palette
            dc.SetTextForeground(c);
        }
        RenderTextLine(dc,1,Position2,Line2,dir2,center2,Effect2,Countdown2,pass);

        dc.SetFont(Font3);
        if(colorcnt>2)
        {
            palette.GetColor(2,c); // scm 7-18-13. added if,. only pull color if we have at least two colors checked in palette
            dc.SetTextForeground(c);
        }
        RenderTextLine(dc,2,Position3,Line3,dir3,center3,Effect3,Countdown3,pass);

        dc.SetFont(Font4);
        if(colorcnt>3)
        {
            palette.GetColor(3,c); // scm 7-18-13. added if,. only pull color if we have at least two colors checked in palette
            dc.SetTextForeground(c);
        }
        RenderTextLine(dc,3,Position4,Line4,dir4,center4,Effect4,Countdown4,pass);
    }


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
#define COUNTDOWN_FREEFMT  6

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

#define IsGoingLeft(dir)  (((dir) == TEXTDIR_LEFT(0)) || ((dir) == TEXTDIR_UPLEFT(5)) || ((dir) == TEXTDIR_DOWNLEFT(6)))
#define IsGoingRight(dir)  (((dir) == TEXTDIR_RIGHT(1)) || ((dir) == TEXTDIR_UPRIGHT(7)) || ((dir) == TEXTDIR_DOWNRIGHT(8)))
#define IsGoingUp(dir)  (((dir) == TEXTDIR_UP(2)) || ((dir) == TEXTDIR_UPLEFT(5)) || ((dir) == TEXTDIR_UPRIGHT(7)))
#define IsGoingDown(dir)  (((dir) == TEXTDIR_DOWN(3)) || ((dir) == TEXTDIR_DOWNLEFT(6)) || ((dir) == TEXTDIR_DOWNRIGHT(8)))

//provide back-and-forth movement (linear):
//in future this could use exp/log functions for "gravity" type bounces, but for now linear is adequate
#define zigzag(value, range)  \
    ((((value) / (range)) & 1)? \
        (value) % (range): /*increase during odd cycles*/ \
        (range) - (value) % (range) - 1) /*descrease during even cycles*/

//#define WANT_DEBUG 99
//#include "djdebug.cpp"

static wxString StripLeft(wxString str, wxString pattern)
{
    while (str.StartsWith(pattern, &str));
    return str;
}

static wxString StripRight(wxString str, wxString pattern)
{
    while (str.EndsWith(pattern, &str));
    return str;
}

void RgbEffects::RenderTextLine(wxMemoryDC& dc, int idx, int Position, const wxString& Line_orig, int dir, bool center, int Effect, int Countdown, bool WantRender)
{
    long tempLong,longsecs;
    wxString msg,tempmsg;
    int i,days,hours,minutes,seconds;
    wxDateTime dt;
    wxTimeSpan ts;
    wxString::const_iterator end;
    wxString fmt, Line = Line_orig; //make copy so it can be modified -DJ
    wxChar delim;
    if (Line.IsEmpty()) return;

    switch(Countdown)
    {
    case COUNTDOWN_SECONDS:
        // countdown seconds
        if (state==0)
        {
            if (!Line.ToLong(&tempLong)) tempLong=0;
            timer_countdown[idx] = curPeriod+tempLong*20+19;  // capture 0 period
        }
        seconds=(timer_countdown[idx]-curPeriod)/20;
        if(seconds < 0) seconds=0;
        msg=wxString::Format("%i",seconds);
        break;

    case COUNTDOWN_FREEFMT: //free format text with embedded formatting chars -DJ
#if 0
wxTimeSpan format chars are described at:
http://docs.wxwidgets.org/trunk/classwx_time_span.html
The following format specifiers are allowed after %:
        •H - Number of Hours
        •M - Number of Minutes
        •S - Number of Seconds
        •l - Number of Milliseconds
        •D - Number of Days
        •E - Number of Weeks
        •% - The percent character

//Format Characters are described at: http://www.cplusplus.com/reference/ctime/strftime/
TIME FORMAT CHARACTERS:
        %a Abbreviated weekday name eg. Thu
        %A Full weekday name eg. Thursday
        %b Abbreviated month name eg. Aug
        %B Full month name eg. August
        %c Date and time representation eg. Thu Aug 23 14:55:02 2001
        %d Day of the month (01-31) eg. 23
        %H Hour in 24h format (00-23) eg. 14
        %I Hour in 12h format (01-12) eg. 02
        %j Day of the year (001-366) eg. 235
        %m Month as a decimal number (01-12) eg. 08
        %M Minute (00-59) eg. 55
        %p AM or PM designation eg. PM
        %S Second (00-61) eg. 02
        %U Week number with the first Sunday as the first day of week one (00-53) eg. 33
        %w Weekday as a decimal number with Sunday as 0 (0-6) eg. 4
        %W Week number with the first Monday as the first day of week one (00-53) eg. 34
        %x Date representation eg. 08/23/01
        %X Time representation eg. 14:55:02
        %y Year, last two digits (00-99) eg. 01
        %Y Year eg. 2001
        %Z Timezone name or abbreviation CDT
        %% A % sign eg. %
#endif // 0
//time_local = time.Format(wxT("%T"), wxDateTime::A_EST).c_str();
        if (Line.size() >= 4)
        {
            delim = Line[0]; //use first char as date delimiter; date and format string follows that, separated by delimiter
            Line.Remove(0, 1); //.erase(Line.begin(), Line.begin() + 1); //remove leading delim
//            Line.RemoveLast(); //remove delimiter
            fmt = Line.After(delim);
            Line.Truncate(Line.find(delim)); //remove fmt string, leaving only count down date
        }
        else fmt.Empty();
//CAUTION: fall thru here
    case COUNTDOWN_D_H_M_S:
    case COUNTDOWN_H_M_S:
    case COUNTDOWN_M_or_S:
    case COUNTDOWN_S:
        // countdown to date
        if (state%20 == 0)   //1x/sec
        {
#if 0 //wxWidgets is broken; use this section to test it -DJ
            wxString test = _("Sat, 18 Dec 1999 00:48:30 +0100");  //valid RFC822 format
            if (!dt.ParseRfc822Date(test, &end))
            {
                msg = _T("broken-1");
                break;
            }
            else if (*end)     //end != test.end()) {
            {
                msg = wxString::Format(_("broken-2@%d"), end - test.begin());
                break;
            }
            else if (!dt.ParseDateTime(test, &end))
            {
                msg = _T("broken-3");
                break;
            }
            else if (*end)     //end != test.end()) {
            {
                msg = wxString::Format(_("broken-4@%d"), end - test.begin());
                break;
            }
#endif
//            if ( dt.ParseDateTime(Line, &end) ) { //broken, force RFC822 for now -DJ
            if ( dt.ParseRfc822Date(Line, &end) )
            {
                // dt is (at least partially) valid, so calc # of seconds until then
                ts=dt.Subtract(wxDateTime::Now());
                wxLongLong ll=ts.GetSeconds();
                if (ll > LONG_MAX) ll=LONG_MAX;
                if (ll < 0) ll=0;
                longsecs=ll.ToLong();
            }
            else
            {
                // invalid date/time
                longsecs = 0;
            }
            timer_countdown[idx]=longsecs;
        }
        else
        {
            longsecs=timer_countdown[idx];
            ts = wxTimeSpan(0, 0, longsecs, 0); //reconstruct wxTimeSpan so we can call .Format method -DJ
        }
        if (!longsecs)
        {
            msg = _T("invalid date");    //show when invalid -DJ
            break;
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
        else if (Countdown == COUNTDOWN_FREEFMT)
//            msg = _T("%%") + Line + _T("%%") + fmt + _T("%%");
            msg = ts.Format(fmt); //dt.Format(fmt)
        else //if (Countdown == COUNTDOWN_M_or_S)
            if (60 * hours + minutes < 5) //COUNTDOWN_M_or_S: show seconds
                msg = wxString::Format("%d", 60*60 * hours + 60 * minutes + seconds);
            else //COUNTDOWN_M_or_S: show minutes
                msg = wxString::Format("%d m", 60 * hours + minutes);
        break;
    default:
        msg=Line;
        msg.Replace("\\n", "\n", true); //allow vertical spacing (mainly for up/down) -DJ
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
    int extra_left = IsGoingLeft(dir)? textsize.x - dc.GetMultiLineTextExtent(wxString(msg).Trim(false)).x: 0; //CAUTION: trim() alters object, so make a copy first
    int extra_right = IsGoingRight(dir)? textsize.x - dc.GetMultiLineTextExtent(wxString(msg).Trim(true)).x: 0;
    int extra_down = IsGoingDown(dir)? textsize.y - dc.GetMultiLineTextExtent(StripRight(msg, "\n")).y: 0;
    int extra_up = IsGoingUp(dir)? textsize.y - dc.GetMultiLineTextExtent(StripLeft(msg, "\n")).y: 0;
//    debug(1, "size %d lstrip %d, rstrip %d, = %d, %d, text %s", dc.GetMultiLineTextExtent(msg).y, dc.GetMultiLineTextExtent(StripLeft(msg, "\n")).y, dc.GetMultiLineTextExtent(StripRight(msg, "\n")).y, extra_down, extra_up, (const char*)StripLeft(msg, "\n"));
    int lineh = dc.GetMultiLineTextExtent("X").y;
//    wxString debmsg = msg; debmsg.Replace("\n","\\n", true);
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

#ifdef WANT_TEXT_LINES_SYNCED //sync text lines together (experimental) -DJ
    static wxSize synced_textsize;
    if (!WantRender)   //for synced text lines, collect info first and then render after info is available
    {
        if (!idx) synced_textsize = textsize;
        else   //keep max value
        {
            if (textsize.x > synced_textsize.x) synced_textsize.x = textsize.x;
            if (textsize.y > synced_textsize.y) synced_textsize.y = textsize.y;
        }
//        debug(1, "text[%d]: pass %d, txtsiz %d/%d, synced %d/%d", idx, WantRender, textsize.x, textsize.y, synced_textsize.x, synced_textsize.y);
        return;
    }
    else textsize.x = synced_textsize.x;   //use composite size
//    debug(1, "text[%d]: pass %d, synced txtsiz %d/%d", idx, WantRender, textsize.x, textsize.y);
#endif // WANT_TEXT_LINES_SYNCED

    int txtwidth=textsize.GetWidth();
    int totwidth=BufferWi+txtwidth;
    int totheight=BufferHt+textsize.GetHeight();
    int OffsetLeft= (Position * totwidth / 100) - totwidth/2;
    int OffsetTop= totheight/2 - (Position * totheight / 100);
    int xlimit=totwidth*8 + 1;
    int ylimit=totheight*8 + 1;
    int state8;

    if (TextRotation == 0.0)
    {
        wxRect rect(0,0,BufferWi,BufferHt);
        switch (dir)
        {
        case TEXTDIR_LEFT(0):
//           debug(1, "l2r[%d] center? %d, xlim/16 %d, state %d, xofs %d, extra l %d r %d, text %s", idx, center, xlimit/16, state, center? std::max((int)(xlimit/16 - state /*% xlimit*/ /8), -extra_left/2): xlimit/16 - state % xlimit/8, extra_left, extra_right, (const char*)msg);
            // rect.Offset(center? std::max((int)(xlimit/16 - state /*% xlimit*/ /8), -extra_left/2): xlimit/16 - state % xlimit/8, OffsetTop);
            state8 = state /8;
            if(state8<0) state8+=32768;
            if(state>2000000)
                state=state+0;
            rect.Offset(center? std::max((int)(xlimit/16 - state8), -extra_left/2): xlimit/16 - state % xlimit/8, OffsetTop);
            break; // left, optionally stop at center
        case TEXTDIR_RIGHT(1):
            rect.Offset(center? std::min((int)(state /*% xlimit*/ /8 - xlimit/16), extra_right/2): state % xlimit/8 - xlimit/16, OffsetTop);
            break; // right, optionally stop at center
        case TEXTDIR_UP(2):
            rect.Offset(OffsetLeft, center? std::max((int)(ylimit/16 - state /*% ylimit*/ /8), lineh/2 - extra_up/2): ylimit/16 - state % ylimit/8);
            break; // up, optionally stop at center
        case TEXTDIR_DOWN(3):
//            debug(1, "t2b[%d] center? %d, totht %d, ylimit %d, extra u %d d %d, lineh %d, text %s => yofs min(%d - %d, %d + %d)", idx, center, totheight, ylimit, extra_up, extra_down, lineh, (const char*)debmsg, state /*% ylimit*/ /8, ylimit/16, -lineh/2, extra_down/2);
            rect.Offset(OffsetLeft, center? std::min((int)(state /*% ylimit*/ /8 - ylimit/16), -lineh/2 + extra_down/2): state % ylimit/8 - ylimit/16);
            break; // down, optionally stop at center
        case TEXTDIR_UPLEFT(5):
            rect.Offset(center? std::max((int)(xlimit/16 - state /*% xlimit*/ /8), -extra_left/2): xlimit/16 - state % xlimit/8, center? std::max((int)(ylimit/16 - state /*% ylimit*/ /8), lineh/2 - extra_up/2): ylimit/16 - state % ylimit/8);
            break; // up-left, optionally stop at center
        case TEXTDIR_DOWNLEFT(6):
            rect.Offset(center? std::max((int)(xlimit/16 - state /*% xlimit*/ /8), -extra_left/2): xlimit/16 - state % xlimit/8, center? std::min((int)(state /*% ylimit*/ /8 - ylimit/16), -lineh/2 + extra_down/2): state % ylimit/8 - ylimit/16);
            break; // down-left, optionally stop at center
        case TEXTDIR_UPRIGHT(7):
            rect.Offset(center? std::min((int)(state /*% xlimit*/ /8 - xlimit/16), extra_right/2): state % xlimit/8 - xlimit/16, center? std::max((int)(ylimit/16 - state /*% ylimit*/ /8), lineh/2 - extra_up/2): ylimit/16 - state % ylimit/8);
            break; // up-right, optionally stop at center
        case TEXTDIR_DOWNRIGHT(8):
            rect.Offset(center? std::min((int)(state /*% xlimit*/ /8 - xlimit/16), extra_right/2): state % xlimit/8 - xlimit/16, center? std::min((int)(state /*% ylimit*/ /8 - ylimit/16), -lineh/2 + extra_down/2): state % ylimit/8 - ylimit/16);
            break; // down-right, optionally stop at center
        case TEXTDIR_WAVEY_LRUPDOWN(9):
            if (center) //does to-center make sense with this one?
                rect.Offset(std::min((int)(state /*% xlimit*/ /8 - xlimit/16), extra_right/2), std::max((int)zigzag(state/4, totheight)/2 - totheight/4, -extra_left/2));
            else
                rect.Offset(xlimit/16 - state % xlimit/8, zigzag(state/4, totheight)/2 - totheight/4);
            break; // left-to-right, wavey up-down 1/2 height (too bouncy if full height is used), slow down up/down motion (too fast unless scaled)
        case TEXTDIR_NONE(4): //fall thru to default
        default:
            rect.Offset(0, OffsetTop);
            break; // static
        }
        dc.DrawLabel(msg,rect,wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL);
    }
    else
    {
        switch (dir)
        {
        case 0:
            dc.DrawRotatedText(msg, BufferWi - state % xlimit/8 + xoffset, OffsetTop, TextRotation);
            break; // left
        case 1:
            dc.DrawRotatedText(msg, state % xlimit/8 - txtwidth + xoffset, OffsetTop, TextRotation);
            break; // right
        case 2:
            dc.DrawRotatedText(msg, OffsetLeft, totheight - state % ylimit/8 - yoffset, TextRotation);
            break; // up
        case 3:
            dc.DrawRotatedText(msg, OffsetLeft, state % ylimit/8 - yoffset, TextRotation);
            break; // down
        case 5:
            dc.DrawRotatedText(msg, BufferWi - state % xlimit/8 + xoffset, totheight - state % ylimit/8 - yoffset, TextRotation);
            break; // up-left
        case 6:
            dc.DrawRotatedText(msg, BufferWi - state % xlimit/8 + xoffset, state % ylimit/8 - yoffset, TextRotation);
            break; // down-left
        case 7:
            dc.DrawRotatedText(msg, state % xlimit/8 - txtwidth + xoffset, totheight - state % ylimit/8 - yoffset, TextRotation);
            break; // up-right
        case 8:
            dc.DrawRotatedText(msg, state % xlimit/8 - txtwidth + xoffset, state % ylimit/8 - yoffset, TextRotation);
            break; // down-right
        default:
            dc.DrawRotatedText(msg, 0, OffsetTop, TextRotation);
            break; // static
        }
    }
}
//#ifndef WANT_TEXT_LINES_SYNCED
//static void FinishRender(wxMemoryDC& dc, int BufferWi, int BufferHt, int state, int idx, wxSize& textsize)
//#endif // WANT_TEXT_LINES_SYNCED
