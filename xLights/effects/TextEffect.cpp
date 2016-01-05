#include "TextEffect.h"

#include "TextPanel.h"
#include <wx/checkbox.h>

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"


TextEffect::TextEffect(int id) : RenderableEffect(id, "Text")
{
    //ctor
}

TextEffect::~TextEffect()
{
    //dtor
}

wxPanel *TextEffect::CreatePanel(wxWindow *parent) {
    return new TextPanel(parent);
}


static inline void SetCheckboxValue(wxWindow *w, int id, bool b) {
    wxCheckBox *c = (wxCheckBox*)w->FindWindowById(id);
    c->SetValue(b);
    wxCommandEvent evt(wxEVT_COMMAND_CHECKBOX_CLICKED, id);
    evt.SetEventObject(c);
    evt.SetInt(b);
    c->ProcessWindowEvent(evt);
}

void TextEffect::SetDefaultParameters(ModelClass *cls) {
    TextPanel *tp = (TextPanel*)panel;
    if (tp == nullptr) {
        return;
    }
    
    SetCheckboxValue(tp, tp->ID_CHECKBOX_TextToCenter1, false);
    SetCheckboxValue(tp, tp->ID_CHECKBOX_TextToCenter2, false);
    SetCheckboxValue(tp, tp->ID_CHECKBOX_TextToCenter3, false);
    SetCheckboxValue(tp, tp->ID_CHECKBOX_TextToCenter4, false);
}

bool TextEffect::CanRenderOnBackgroundThread() {
#ifdef LINUX
    return false;
#else 
    return true;
#endif
}

//formatting notes:
//countdown == seconds: put a non-0 value in text line 1 to count down
//countdown == any of the "to date" options: put "Sat, 18 Dec 1999 00:48:30 +0100" in the text line
//countdown = !to date!%fmt: put delimiter + target date + same delimiter + format string with %x markers in it (described down below)


#define WANT_TEXT_LINES_SYNCED //sync text lines together (experimental) -DJ

wxMutex FONT_MAP_LOCK;
std::map<wxString, wxFont> FONT_MAP;

void SetFont(DrawingContext *dc, const wxString& FontString, const xlColor &color) {
    wxMutexLocker locker(FONT_MAP_LOCK);
    if (FONT_MAP.find(FontString) == FONT_MAP.end()) {
        wxFont font;
        if (!FontString.IsEmpty())
        {
            font.SetNativeFontInfoUserDesc(FontString);
            //we want "Arial 8" to be 8 pixels high and not depend on the System DPI
            font.SetPixelSize(wxSize(0, font.GetPointSize()));
        }
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
        wxString s = font.GetNativeFontInfoDesc();
        s.Replace(";2;",";3;",false);
        font.SetNativeFontInfo(s);
#endif
        FONT_MAP[FontString] = font;
        font.UnShare();
        dc->SetFont(font, color);
        return;
    }
    wxFont font = FONT_MAP[FontString];
    font.UnShare();
    dc->SetFont(font, color);
}

enum TextDirection {
    TEXTDIR_LEFT,
    TEXTDIR_RIGHT,
    TEXTDIR_UP,
    TEXTDIR_DOWN,
    TEXTDIR_NONE,
    TEXTDIR_UPLEFT,
    TEXTDIR_DOWNLEFT,
    TEXTDIR_UPRIGHT,
    TEXTDIR_DOWNRIGHT,
    TEXTDIR_WAVEY_LRUPDOWN,
    TEXTDIR_VECTOR
};
static TextDirection TextEffectDirectionsIndex(const wxString &st) {
    if (st == "left") return TEXTDIR_LEFT;
    if (st == "right") return TEXTDIR_RIGHT;
    if (st == "up") return TEXTDIR_UP;
    if (st == "down") return TEXTDIR_DOWN;
    if (st == "none") return TEXTDIR_NONE;
    if (st == "up-left") return TEXTDIR_UPLEFT;
    if (st == "down-left") return TEXTDIR_DOWNLEFT;
    if (st == "up-right") return TEXTDIR_UPRIGHT;
    if (st == "down-right") return TEXTDIR_DOWNRIGHT;
    if (st == "wavey L-R/up-down") return TEXTDIR_WAVEY_LRUPDOWN;
    if (st == "vector") return TEXTDIR_VECTOR;
    return TEXTDIR_NONE;
}
static int TextCountDownIndex(const wxString &st) {
    if (st == "seconds") return 1;
    if (st == "to date 'd h m s'") return 2;
    if (st == "to date 'h:m:s'") return 3;
    if (st == "to date 'm' or 's'") return 4;
    if (st == "to date 's'") return 5;
    if (st == "!to date!%fmt") return 6;
    return 0;
}
static int TextEffectsIndex(const wxString &st) {
    if (st == "vert text up") return 1;
    if (st == "vert text down") return 2;
    if (st == "rotate up 45") return 3;
    if (st == "rotate up 90") return 4;
    if (st == "rotate down 45") return 5;
    if (st == "rotate down 90") return 6;
    return 0;
}
void TextEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    xlColour c;
    buffer.drawingContext->Clear();
    size_t colorcnt=buffer.GetColorCount();
    
    for (int pass = 0; pass < 2; ++pass)
    {
#ifndef WANT_TEXT_LINES_SYNCED
        if (!pass) continue; //don't need 2 passes
#endif // WANT_TEXT_LINES_SYNCED
        buffer.palette.GetColor(0,c);
        
        for (int line = 1;  line <= 4; line++) {
            wxString lp = wxString::Format("%d", line);
            wxString text = SettingsMap["TEXTCTRL_Text_Line" + lp];
            if (text != "") {
                if (colorcnt >= line) {
                    buffer.palette.GetColor(line - 1, c);
                }
                wxString fontString = SettingsMap["FONTPICKER_Text_Font" + lp];
                SetFont(buffer.drawingContext,fontString,c);
                
                bool pixelOffsets = false;
                int startx = 0;
                int starty = 0;
                int endx = 0;
                int endy = 0;
                
                if (line == 1) {
                    starty = wxAtoi(SettingsMap.Get("SLIDER_Text_YStart" + lp, "0"));
                    startx = wxAtoi(SettingsMap.Get("SLIDER_Text_XStart" + lp, "0"));
                    endy = wxAtoi(SettingsMap.Get("SLIDER_Text_YEnd" + lp, "0"));
                    endx = wxAtoi(SettingsMap.Get("SLIDER_Text_XEnd" + lp, "0"));
                    pixelOffsets = wxAtoi(SettingsMap.Get("CHECKBOX_Text_PixelOffsets" + lp, "0"));
                } else {
                    starty = wxAtoi(SettingsMap["SLIDER_Text_Position" + lp]) * 2 - 100;
                    endy = starty;
                    startx = starty;
                    endx = endy;
                }
                
                RenderTextLine(buffer,
                               buffer.drawingContext, line - 1,
                               text,
                               TextEffectDirectionsIndex(SettingsMap["CHOICE_Text_Dir" + lp]),
                               wxAtoi(SettingsMap["CHECKBOX_TextToCenter" + lp]),
                               TextEffectsIndex(SettingsMap["CHOICE_Text_Effect" + lp]),
                               TextCountDownIndex(SettingsMap["CHOICE_Text_Count" + lp]),
                               pass,
                               wxAtoi(SettingsMap.Get("TEXTCTRL_Text_Speed" + lp, "10")),
                               startx, starty, endx, endy, pixelOffsets);
            }
        }
    }
    
    wxImage * i = buffer.drawingContext->FlushAndGetImage();
    
    bool ha = i->HasAlpha();
    for(wxCoord x=0; x<buffer.BufferWi; x++)
    {
        for(wxCoord y=0; y<buffer.BufferHt; y++)
        {
            if (ha) {
                c.Set(i->GetRed(x, buffer.BufferHt-y-1),
                      i->GetGreen(x, buffer.BufferHt-y-1),
                      i->GetBlue(x, buffer.BufferHt-y-1),
                      i->GetAlpha(x, buffer.BufferHt-y-1));
            } else {
                c.Set(i->GetRed(x, buffer.BufferHt-y-1),
                      i->GetGreen(x, buffer.BufferHt-y-1),
                      i->GetBlue(x, buffer.BufferHt-y-1));
                if (c == xlBLACK) {
                    c.alpha = 0;
                }
            }
            buffer.SetPixel(x,y,c);
        }
    }
}


wxSize GetMultiLineTextExtent(DrawingContext *dc,
                              const wxString& text,
                              wxCoord *widthText,
                              wxCoord *heightText,
                              wxCoord *hl)
{
    double widthTextMax = 0, widthLine;
    double heightTextTotal = 0;
    double heightLineDefault = 0, heightLine = 0;
    
    wxString curLine;
    for ( wxString::const_iterator pc = text.begin(); ; ++pc )
    {
        if ( pc == text.end() || *pc == wxS('\n') )
        {
            if ( curLine.empty() )
            {
                // we can't use GetTextExtent - it will return 0 for both width
                // and height and an empty line should count in height
                // calculation
                
                // assume that this line has the same height as the previous
                // one
                if ( !heightLineDefault )
                    heightLineDefault = heightLine;
                
                if ( !heightLineDefault )
                {
                    // but we don't know it yet - choose something reasonable
                    double dummy;
                    dc->GetTextExtent(wxS("W"), &dummy, &heightLineDefault);
                }
                
                heightTextTotal += heightLineDefault;
            }
            else
            {
                dc->GetTextExtent(curLine, &widthLine, &heightLine);
                if ( widthLine > widthTextMax )
                    widthTextMax = widthLine;
                heightTextTotal += heightLine;
            }
            
            if ( pc == text.end() )
            {
                break;
            }
            else // '\n'
            {
                curLine.clear();
            }
        }
        else
        {
            curLine += *pc;
        }
    }
    *widthText = widthTextMax;
    *heightText = heightTextTotal;
    *hl = heightLine;
    return wxSize(widthTextMax, heightTextTotal);
}

wxSize GetMultiLineTextExtent(DrawingContext *dc,
                              const wxString& text)
{
    wxCoord x,y,z;
    return GetMultiLineTextExtent(dc, text, &x, &y, &z);
}

void DrawLabel(DrawingContext *dc,
               const wxString& text,
               const wxRect& rect,
               int alignment)
{
    // find the text position
    wxCoord widthText, heightText, heightLine;
    GetMultiLineTextExtent(dc, text, &widthText, &heightText, &heightLine);
    
    wxCoord width, height;
    width = widthText;
    height = heightText;
    
    wxCoord x, y;
    if ( alignment & wxALIGN_RIGHT )
    {
        x = rect.GetRight() - width;
    }
    else if ( alignment & wxALIGN_CENTRE_HORIZONTAL )
    {
        x = (rect.GetLeft() + rect.GetRight() + 1 - width) / 2;
    }
    else // alignment & wxALIGN_LEFT
    {
        x = rect.GetLeft();
    }
    
    if ( alignment & wxALIGN_BOTTOM )
    {
        y = rect.GetBottom() - height;
    }
    else if ( alignment & wxALIGN_CENTRE_VERTICAL )
    {
        y = (rect.GetTop() + rect.GetBottom() + 1 - height) / 2;
    }
    else // alignment & wxALIGN_TOP
    {
        y = rect.GetTop();
    }
    
    // split the string into lines and draw each of them separately
    //
    // NB: while wxDC::DrawText() on some platforms supports drawing multi-line
    //     strings natively, this is not the case for all of them, notably not
    //     wxMSW which uses this function for multi-line texts, so we may only
    //     call DrawText() for single-line strings from here to avoid infinite
    //     recursion.
    wxString curLine;
    for ( wxString::const_iterator pc = text.begin(); ; ++pc )
    {
        if ( pc == text.end() || *pc == '\n' )
        {
            int xRealStart = x; // init it here to avoid compielr warnings
            if ( !curLine.empty() )
            {
                // NB: can't test for !(alignment & wxALIGN_LEFT) because
                //     wxALIGN_LEFT is 0
                if ( alignment & (wxALIGN_RIGHT | wxALIGN_CENTRE_HORIZONTAL) )
                {
                    wxCoord widthLine = GetMultiLineTextExtent(dc, curLine).x;
                    
                    if ( alignment & wxALIGN_RIGHT )
                    {
                        xRealStart += width - widthLine;
                    }
                    else // if ( alignment & wxALIGN_CENTRE_HORIZONTAL )
                    {
                        xRealStart += (width - widthLine) / 2;
                    }
                }
                //else: left aligned, nothing to do
                
                dc->DrawText(curLine, xRealStart, y);
            }
            
            y += heightLine;
            
            if ( pc == text.end() )
                break;
            
            curLine.clear();
        }
        else // not end of line
        {
            curLine += *pc;
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



#define IsGoingLeft(dir)  (((dir) == TEXTDIR_LEFT) || ((dir) == TEXTDIR_UPLEFT) || ((dir) == TEXTDIR_DOWNLEFT))
#define IsGoingRight(dir)  (((dir) == TEXTDIR_RIGHT) || ((dir) == TEXTDIR_UPRIGHT) || ((dir) == TEXTDIR_DOWNRIGHT))
#define IsGoingUp(dir)  (((dir) == TEXTDIR_UP) || ((dir) == TEXTDIR_UPLEFT) || ((dir) == TEXTDIR_UPRIGHT))
#define IsGoingDown(dir)  (((dir) == TEXTDIR_DOWN) || ((dir) == TEXTDIR_DOWNLEFT) || ((dir) == TEXTDIR_DOWNRIGHT))

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

class TextRenderCache : public EffectRenderCache {
public:
    TextRenderCache() {};
    virtual ~TextRenderCache() {};
    int timer_countdown[4];
    wxSize synced_textsize;
};

TextRenderCache *GetCache(RenderBuffer &buffer, int id) {
    TextRenderCache *cache = (TextRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new TextRenderCache();
        buffer.infoCache[id] = cache;
    }
    return cache;
}


void TextEffect::RenderTextLine(RenderBuffer &buffer,
                                DrawingContext* dc, int idx, const wxString& Line_orig, int dir,
                                bool center, int Effect, int Countdown, bool WantRender, int tspeed,
                                int startx, int starty, int endx, int endy,
                                bool isPixelBased)
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
    
    int state = (buffer.curPeriod - buffer.curEffStartPer) * tspeed * buffer.frameTimeInMs / 50;
    int framesPerSec = 1000 / buffer.frameTimeInMs;
    switch(Countdown)
    {
        case COUNTDOWN_SECONDS:
            // countdown seconds
            if (state==0)
            {
                if (!Line.ToLong(&tempLong)) tempLong=0;
                GetCache(buffer,id)->timer_countdown[idx] = buffer.curPeriod+tempLong*framesPerSec+framesPerSec-1;  // capture 0 period
            }
            seconds=(GetCache(buffer,id)->timer_countdown[idx]-buffer.curPeriod)/framesPerSec;
            if(seconds < 0) seconds=0;
            msg=wxString::Format("%i",seconds);
            break;
            
        case COUNTDOWN_FREEFMT: //free format text with embedded formatting chars -DJ
#if 0
            
            Aug 14,2015 <scm>
            Sample datestrings that are valid for the countdown timer
                Wed, 02 Oct 2015 15:00:00 +0200
                Wed, 02 Oct 2015 15:00:00 EST
                
                Note, dates must be in the future, any date in the past will show as "Invalid Date" when converted
                
                
                
                clear(
                
                )
                wxTimeSpan format chars are described at:
                http://docs.wxwidgets.org/trunk/classwx_time_span.html
                The following format specifiers are allowed after %:
                ïH - Number of Hours
                ïM - Number of Minutes
                ïS - Number of Seconds
                ïl - Number of Milliseconds
                ïD - Number of Days
                ïE - Number of Weeks
                ï% - The percent character
                
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
            if (state%framesPerSec == 0)   //1x/sec
            {
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
                GetCache(buffer,id)->timer_countdown[idx]=longsecs;
            }
            else
            {
                longsecs=GetCache(buffer,id)->timer_countdown[idx];
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
            // vertical text down
            tempmsg=msg;
            msg.clear();
            for(i=0; i<tempmsg.length(); i++)
            {
                msg = msg + tempmsg.GetChar(i) + "\n";
            }
            break;
    }
    
    wxSize textsize = GetMultiLineTextExtent(dc, msg);
    int extra_left = IsGoingLeft(dir)? textsize.x - GetMultiLineTextExtent(dc, wxString(msg).Trim(false)).x: 0; //CAUTION: trim() alters object, so make a copy first
    int extra_right = IsGoingRight(dir)? textsize.x - GetMultiLineTextExtent(dc, wxString(msg).Trim(true)).x: 0;
    int extra_down = IsGoingDown(dir)? textsize.y - GetMultiLineTextExtent(dc, StripRight(msg, "\n")).y: 0;
    int extra_up = IsGoingUp(dir)? textsize.y - GetMultiLineTextExtent(dc, StripLeft(msg, "\n")).y: 0;
    //    debug(1, "size %d lstrip %d, rstrip %d, = %d, %d, text %s", dc.GetMultiLineTextExtent(msg).y, dc.GetMultiLineTextExtent(StripLeft(msg, "\n")).y, dc.GetMultiLineTextExtent(StripRight(msg, "\n")).y, extra_down, extra_up, (const char*)StripLeft(msg, "\n"));
    int lineh = GetMultiLineTextExtent(dc, "X").y;
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
    if (!WantRender)   //for synced text lines, collect info first and then render after info is available
    {
        wxSize &synced_textsize = GetCache(buffer,id)->synced_textsize;
        if (!idx) synced_textsize = textsize;
        else   //keep max value
        {
            if (textsize.x > synced_textsize.x) synced_textsize.x = textsize.x;
            if (textsize.y > synced_textsize.y) synced_textsize.y = textsize.y;
        }
        //        debug(1, "text[%d]: pass %d, txtsiz %d/%d, synced %d/%d", idx, WantRender, textsize.x, textsize.y, synced_textsize.x, synced_textsize.y);
        return;
    }
    else textsize.x = GetCache(buffer,id)->synced_textsize.x;   //use composite size
    //    debug(1, "text[%d]: pass %d, synced txtsiz %d/%d", idx, WantRender, textsize.x, textsize.y);
#endif // WANT_TEXT_LINES_SYNCED
    
    int txtwidth=textsize.GetWidth();
    int totwidth=buffer.BufferWi+txtwidth;
    int totheight=buffer.BufferHt+textsize.GetHeight();
    
    int OffsetLeft = startx * buffer.BufferWi / 100;
    int OffsetTop = -starty * buffer.BufferHt / 100;
    if (isPixelBased) {
        OffsetLeft = startx;
        OffsetTop =  -starty;
    }
    
    int xlimit=totwidth*8 + 1;
    int ylimit=totheight*8 + 1;
    int state8;
    
    if (TextRotation == 0.0)
    {
        wxRect rect(0,0,buffer.BufferWi,buffer.BufferHt);
        switch (dir)
        {
            case TEXTDIR_VECTOR: {
                double position = buffer.GetEffectTimeIntervalPosition(1.0);
                double ex =  endx * buffer.BufferWi / 100;
                double ey = -endy * buffer.BufferHt / 100;
                if (isPixelBased) {
                    ex = endx;
                    ey = -endy;
                }
                ex = OffsetLeft + (ex - OffsetLeft) * position;
                ey = OffsetTop + (ey - OffsetTop) * position;
                rect.Offset(ex, ey);
            }
                break;
            case TEXTDIR_LEFT:
                //           debug(1, "l2r[%d] center? %d, xlim/16 %d, state %d, xofs %d, extra l %d r %d, text %s", idx, center, xlimit/16, state, center? std::max((int)(xlimit/16 - state /*% xlimit*/ /8), -extra_left/2): xlimit/16 - state % xlimit/8, extra_left, extra_right, (const char*)msg);
                // rect.Offset(center? std::max((int)(xlimit/16 - state /*% xlimit*/ /8), -extra_left/2): xlimit/16 - state % xlimit/8, OffsetTop);
                state8 = state /8;
                if(state8<0) state8+=32768;
                if(state>2000000)
                    state=state+0;
                rect.Offset(center? std::max((int)(xlimit/16 - state8), -extra_left/2): xlimit/16 - state % xlimit/8, OffsetTop);
                break; // left, optionally stop at center
            case TEXTDIR_RIGHT:
                rect.Offset(center? std::min((int)(state /*% xlimit*/ /8 - xlimit/16), extra_right/2): state % xlimit/8 - xlimit/16, OffsetTop);
                break; // right, optionally stop at center
            case TEXTDIR_UP:
                rect.Offset(OffsetLeft, center? std::max((int)(ylimit/16 - state /*% ylimit*/ /8), lineh/2 - extra_up/2): ylimit/16 - state % ylimit/8);
                break; // up, optionally stop at center
            case TEXTDIR_DOWN:
                //            debug(1, "t2b[%d] center? %d, totht %d, ylimit %d, extra u %d d %d, lineh %d, text %s => yofs min(%d - %d, %d + %d)", idx, center, totheight, ylimit, extra_up, extra_down, lineh, (const char*)debmsg, state /*% ylimit*/ /8, ylimit/16, -lineh/2, extra_down/2);
                rect.Offset(OffsetLeft, center? std::min((int)(state /*% ylimit*/ /8 - ylimit/16), -lineh/2 + extra_down/2): state % ylimit/8 - ylimit/16);
                break; // down, optionally stop at center
            case TEXTDIR_UPLEFT:
                rect.Offset(center? std::max((int)(xlimit/16 - state /*% xlimit*/ /8), -extra_left/2): xlimit/16 - state % xlimit/8, center? std::max((int)(ylimit/16 - state /*% ylimit*/ /8), lineh/2 - extra_up/2): ylimit/16 - state % ylimit/8);
                break; // up-left, optionally stop at center
            case TEXTDIR_DOWNLEFT:
                rect.Offset(center? std::max((int)(xlimit/16 - state /*% xlimit*/ /8), -extra_left/2): xlimit/16 - state % xlimit/8, center? std::min((int)(state /*% ylimit*/ /8 - ylimit/16), -lineh/2 + extra_down/2): state % ylimit/8 - ylimit/16);
                break; // down-left, optionally stop at center
            case TEXTDIR_UPRIGHT:
                rect.Offset(center? std::min((int)(state /*% xlimit*/ /8 - xlimit/16), extra_right/2): state % xlimit/8 - xlimit/16, center? std::max((int)(ylimit/16 - state /*% ylimit*/ /8), lineh/2 - extra_up/2): ylimit/16 - state % ylimit/8);
                break; // up-right, optionally stop at center
            case TEXTDIR_DOWNRIGHT:
                rect.Offset(center? std::min((int)(state /*% xlimit*/ /8 - xlimit/16), extra_right/2): state % xlimit/8 - xlimit/16, center? std::min((int)(state /*% ylimit*/ /8 - ylimit/16), -lineh/2 + extra_down/2): state % ylimit/8 - ylimit/16);
                break; // down-right, optionally stop at center
            case TEXTDIR_WAVEY_LRUPDOWN:
                if (center) //does to-center make sense with this one?
                    rect.Offset(std::min((int)(state /*% xlimit*/ /8 - xlimit/16), extra_right/2), std::max((int)zigzag(state/4, totheight)/2 - totheight/4, -extra_left/2));
                else
                    rect.Offset(xlimit/16 - state % xlimit/8, zigzag(state/4, totheight)/2 - totheight/4);
                break; // left-to-right, wavey up-down 1/2 height (too bouncy if full height is used), slow down up/down motion (too fast unless scaled)
            case TEXTDIR_NONE: //fall thru to default
            default:
                rect.Offset(0, OffsetTop);
                break; // static
        }
        DrawLabel(dc, msg,rect,wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL);
    }
    else
    {
        switch (dir) {
            case TEXTDIR_VECTOR: {
                double position = buffer.GetEffectTimeIntervalPosition(1.0);
                double ex = endx * buffer.BufferWi / 100;
                double ey = -endy * buffer.BufferHt / 100;
                if (isPixelBased) {
                    ex = endx;
                    ey = -endy;
                }
                ex = OffsetLeft + (ex - OffsetLeft) * position;
                ey = OffsetTop + (ey - OffsetTop) * position;
                if (TextRotation > 50) {
                    dc->DrawText(msg, buffer.BufferWi / 2 + ex - txtwidth / 2, buffer.BufferHt / 2 + ey + textsize.GetHeight() / 2, TextRotation);
                } else if (TextRotation > 0) {
                    dc->DrawText(msg, buffer.BufferWi / 2 + ex - txtwidth / 2, buffer.BufferHt / 2 + ey + yoffset * 2, TextRotation);
                } else if (TextRotation < -50) {
                    dc->DrawText(msg, buffer.BufferWi / 2 + ex + txtwidth / 2, buffer.BufferHt / 2 + ey - textsize.GetHeight() / 2, TextRotation);
                } else {
                    dc->DrawText(msg, buffer.BufferWi / 2 + ex - txtwidth / 2 + xoffset, buffer.BufferHt / 2 + ey - textsize.GetHeight() / 2, TextRotation);
                }
            }
                break;
            case TEXTDIR_LEFT:
                dc->DrawText(msg, buffer.BufferWi - state % xlimit/8 + xoffset, OffsetTop, TextRotation);
                break; // left
            case TEXTDIR_RIGHT:
                dc->DrawText(msg, state % xlimit/8 - txtwidth + xoffset, OffsetTop, TextRotation);
                break; // right
            case TEXTDIR_UP:
                dc->DrawText(msg, OffsetLeft, totheight - state % ylimit/8 - yoffset, TextRotation);
                break; // up
            case TEXTDIR_DOWN:
                dc->DrawText(msg, OffsetLeft, state % ylimit/8 - yoffset, TextRotation);
                break; // down
            case TEXTDIR_UPLEFT:
                dc->DrawText(msg, buffer.BufferWi - state % xlimit/8 + xoffset, totheight - state % ylimit/8 - yoffset, TextRotation);
                break; // up-left
            case TEXTDIR_DOWNLEFT:
                dc->DrawText(msg, buffer.BufferWi - state % xlimit/8 + xoffset, state % ylimit/8 - yoffset, TextRotation);
                break; // down-left
            case TEXTDIR_UPRIGHT:
                dc->DrawText(msg, state % xlimit/8 - txtwidth + xoffset, totheight - state % ylimit/8 - yoffset, TextRotation);
                break; // up-right
            case TEXTDIR_DOWNRIGHT:
                dc->DrawText(msg, state % xlimit/8 - txtwidth + xoffset, state % ylimit/8 - yoffset, TextRotation);
                break; // down-right
            default:
                dc->DrawText(msg, 0, OffsetTop, TextRotation);
                break; // static
        }
    }
}
