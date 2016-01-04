/***************************************************************
 * Name:      RenderPiano.cpp
 * Purpose:   Implements RGB effects
 * Author:    Matt Brown (dowdybrown@yahoo.com)
 * Created:   2012-12-23
 * Copyright: 2012 by Matt Brown
 * License:
     thiss file is part of xLights.

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
#include <wx/textfile.h>
#include <wx/string.h>
#include <wx/tokenzr.h>

#include <ctype.h>
#include <algorithm> //sort

//#define WANT_DEBUG  99
//void djdebug(const char* fmt, ...);
//#include "djdebug.cpp"
 #define debug(level, ...)
 #define debug_more(level, ...)
 #define debug_function(level)


#define divup(num, den)  (((num) + (den) - 1)/(den)) //round up


#if 0
#ifdef _MSC_VER
 #include <hash_map>
#else
 #include <unordered_map>
 #define hash_map  unordered_map //c++ 2011
#endif
//#include <unordered_set>
#include <queue> //priority_queue
#include <vector>
#include <algorithm> //sort
//#include <list>
//#include <deque> //fifo

using namespace std;
//#define tostr(wxstr)  ((string)wxstr)

//kludge to access protected members:
//static void flush_cues(void); //fwd ref
template <typename Tdata, typename Tcontainer = vector<Tdata>, typename Tsorter = Tdata>
class MyQueue: public /*std::*/priority_queue<Tdata, /*std::* /vector<Tdata>*/ Tcontainer, Tsorter>
{
friend /*static*/ void flush_cues(void);
friend class RgbEffects; //::RenderPiano(int Style, int NumKeys, int NumRows, int DrawMode, bool Clipping, const wxString& CueFilename, const wxString& MapFilename, const wxString& ShapeFilename)
public:
    inline void clear() { c.clear(); } //add missing member
};
#define priority_queue  MyQueue //wedge in friend
#endif

//#define MIN(a, b)  (((a) < (b))? (a): (b))
//#define MAX(a, b)  (((a) > (b))? (a): (b))



//these must match choice list in EffectsPanel UI:
//Anim Image (face)
//Color Organ (BROKEN)
//Equalizer (bars)
//Piano Keys (top/edge)
//Player Piano (scroll)
//RGB Icicles (BROKEN)
#define PIANO_STYLE_ANIMAGE  0
#define PIANO_STYLE_CORGAN  1-10 //TODO
#define PIANO_STYLE_EQBARS  2-1
#define PIANO_STYLE_KEYS  3-1
#define PIANO_STYLE_SCROLLING  4-1
#define PIANO_STYLE_ICICLES  5-10 //TODO

#define WantHistory(style)  ((style == PIANO_STYLE_SCROLLING) || (style == PIANO_STYLE_EQBARS) || (style == PIANO_STYLE_ICICLES)) //show multiple rows for these styles
//#define NoteVolume(key, time)  ((key + time) & 7) //TODO
//#define IsNoteOn(key, time)  (NoteVolume(key, time) >= 2) //TODO

static inline int GetPianoStyle(const wxString &style) {
    if (style == "Anim Image (face)") {
        return PIANO_STYLE_ANIMAGE;
    } else if (style == "Equalizer (bars)") {
        return PIANO_STYLE_EQBARS;
    } else if (style == "Piano Keys (top/edge)") {
        return PIANO_STYLE_KEYS;
    } else if (style == "Player Piano (scroll)") {
        return PIANO_STYLE_SCROLLING;
    }
    return PIANO_STYLE_KEYS;
}

//clear active queues:
#if 0
static void clear_queue(std::priority_queue<Cue*, std::vector<Cue*>, Cue /*SortByStop*/>* que)
{
//    ActiveCues = std::priority_queue<Cue*, std::vector<Cue*>, Cue /*SortByStop*/> ActiveCues;
//    ActiveCues.clear();
//priority_queue doesn't have a clear() functon,
}
#endif


void RgbEffects::Piano_flush_cues(void)
{
    debug_function(10); //("flush_cues");
    CuesByStart.resize(0);
//    ActiveCues = std::priority_queue<Cue*, std::vector<Cue*>, Cue /*SortByStop*/> ActiveCues;
//    ActiveCues.clear();
//    while (ActiveCues.size()) ActiveCues.pop(); //TODO: clear all at once
//    ActiveCues = std::priority_queue<Cue*, std::vector<Cue*>, Cue /*SortByStop*/>();
    ActiveCues.clear();
    CachedCueFilename.clear();
}


void RgbEffects::Piano_flush_map(void)
{
    debug_function(10); //Debug debug("flush_map");
    Piano_flush_cues(); //flush dependent data also
    AllSprites.clear();
//    ByLayer.resize(0);
    CachedMapFilename.clear();
}


void RgbEffects::Piano_flush_shapes(void)
{
    debug_function(10); //Debug debug("flush_shapes");
    Piano_flush_map(); //flush dependent data also
//    if (Shapes.GetWidth() || Shapes.GetHeight())
    if (Shapes.IsOk()) Shapes.Clear(); //CAUTION: don't clear unless non-empty (causes access violation)
//    ShapePalette.resize(0);
    ColorMap.clear();
    CachedShapeFilename.clear();
}


//all shapes are loaded from same image file to reduce file I/O and caching
//thiss also allows animated images to be self-contained
void RgbEffects::Piano_load_shapes(const wxString& filename)
{
    debug_function(10); //Debug debug("load_shapes('%s')", (const char*)filename.c_str());
    debug(1, "load shapes file '%s'", (const char*)filename.c_str());
//reload shapes even if file name hasn't changed; color map might be different now
//    if (!CachedShapeFilename.CmpNoCase(filename)) { debug_more(2, ", no change"); return; } //no change
    if (!wxFileExists(filename)) return;
    Piano_flush_shapes(); //invalidate cached data
    if (!Shapes.LoadFile(filename, wxBITMAP_TYPE_ANY, 0) || !Shapes.IsOk())
    {
        //wxMessageBox("Error loading image file: "+NewPictureName);
        Shapes.Clear();
        return;
    }

    if (GetColorCount() < 2) return; //use colors from shapes file if no user-selected colors
//    int imgwidth=image.GetWidth();
//    int imght   =image.GetHeight();
//    std::hash_map<WXCOLORREF, int> palcounts;
//TODO: use wxImage.GetData for better performance?
//TODO: use multiple images within same file?
    for (int y = Shapes.GetHeight() - 1; y >= 0; --y) //bottom->top
        for (int x = 0; x < Shapes.GetWidth(); ++x) //left->right
            if (!Shapes.IsTransparent(x, y))
            {
                xlColor color, mapped;
                color.Set(Shapes.GetRed(x, y), Shapes.GetGreen(x, y), Shapes.GetBlue(x, y));
                if (ColorMap.find(color.GetRGB()) != ColorMap.end()) continue; //already saw this color
                palette.GetColor(ColorMap.size() % GetColorCount(), mapped); //assign user-selected colors to shape palette sequentially, loop if run out of colors
                debug(10, "shape color[%d] 0x%x => user-selected color [%d] 0x%x", ColorMap.size(), color.GetRGB(), ColorMap.size() % GetColorCount(), mapped.GetRGB());
                ColorMap[color.GetRGB()] = mapped; //.GetRGB();
//                ShapePalette.push_back(c.GetRGB()); //keep a list of unique colors in order of occurrence from origin L-R, B-T
            }
    debug(2, "w %d, h %d, #colors %d", Shapes.GetWidth(), Shapes.GetHeight(), ColorMap.size());
    CachedShapeFilename = filename; //don't load same file again
}


//find first color in sprite:
static xlColor find_color(wxImage& Shapes, std::hash_map</*xlColor*/ wxUint32, xlColor>& ColorMap, wxPoint xy, wxSize wh, const char* which)
{
    xlColor color;
    for (int y = xy.y; y < xy.y + wh.y; ++y) //bottom->top
        for (int x = xy.x; x < xy.x + wh.x; ++x) //left->right
        {
            if (Shapes.IsTransparent(x, y)) continue;
            color.Set(Shapes.GetRed(x, y), Shapes.GetGreen(x, y), Shapes.GetBlue(x, y));
            if (ColorMap.find(color.GetRGB()) != ColorMap.end()) color = ColorMap[color.GetRGB()];
            debug_more(10, ", %s 0x%x", which, color.GetRGB());
            return color;
        }
    //if (!strcasecmp("on", which)) color.Set(255, 255, 255);
    //else color.Set(64, 64, 64); //dim, not still visible
    debug_more(10, ", %s (0x%x)", which, color.GetRGB());
    return color;
}


//map sprites to screen location:
void RgbEffects::Piano_load_sprite_map(const wxString& filename) //, int BufferWi, int BufferHt) //, bool clip)
{
    debug_function(10);
    debug(1, "load sprite map file %s", (const char*)filename.c_str());
    wxTextFile f;
    int numbad = 0;

    if (!CachedMapFilename.CmpNoCase(filename)) { debug_more(2, ", no change"); return; } //no change
    if (!wxFileExists(filename)) return;
    if (!f.Open(filename.c_str())) return;
    Piano_flush_map(); //invalidate cached data
    debug(3, "read file");
    for (wxString linebuf = f.GetFirstLine(); !f.Eof(); linebuf = f.GetNextLine())
    {
        std::string::size_type ofs;
        if ((ofs = linebuf.find("#")) != std::string::npos) linebuf.erase(ofs); //remove comments
        while (!linebuf.empty() && (linebuf.Last() == '\\')) //line continuation
        {
            linebuf.RemoveLast(); //remove trailing "\"
            /*std::*/wxString morebuf = f.GetNextLine();
            if (f.Eof()) break;
            linebuf += morebuf;
        }
        while (!linebuf.empty() && isspace(linebuf.Last())) linebuf.RemoveLast(); //trim trailing spaces
        if (linebuf.empty()) continue; //skip blank lines

//#Sprite-name	Xon	Yon	Xoff	Yoff	W	H	DestX	DestY	DestZ
        debug(20, "got line '%s'", (const char*)linebuf.c_str());
        linebuf += "\teol"; //end-of-line check for missing params
        wxStringTokenizer tkz(linebuf, "\t");
        Sprite spr;
        spr.name = tkz.GetNextToken(); //first column = sprite name
        if (spr.name.find(".") == -1) spr.name += ".000"; //kludge: change name to match Audacity Polyphonic nodes
        int srcx_on = wxAtoi(tkz.GetNextToken()); //x coordinate for "on" state
        int srcy_on = wxAtoi(tkz.GetNextToken()); //y coordinate for "on" state
        int srcx_off = wxAtoi(tkz.GetNextToken()); //x coordinate for "off" state
        int srcy_off = wxAtoi(tkz.GetNextToken()); //y coordinate for "off" state
        spr.wh.x = wxAtoi(tkz.GetNextToken()); //sprite width
        spr.wh.y = wxAtoi(tkz.GetNextToken()); //spriate height
        spr.destxy.x = wxAtoi(tkz.GetNextToken()); //destination x coordinate
        spr.destxy.y = wxAtoi(tkz.GetNextToken()); //destination y coordinate
        spr.destz = wxAtoi(tkz.GetNextToken()); //destination layer
//        if (spr.destz > 10) spr.destz = 10; //limit to 10 layers
        wxString junk = tkz.GetNextToken();
        if (/* !junk.empty()*/ junk.Cmp("eol")) { debug_more(20, ", junk at end '%s'", (const char*)junk.c_str()); continue; } //TODO: show error messages?
        debug_more(10, " => srcx/y on %d/%d, off %d/%d, sprite w/h %d/%d, dest x/y/z %d/%d/%d, junk? '%s'", srcx_on, srcy_on, srcx_off, srcy_off, spr.wh.x, spr.wh.y, spr.destxy.x, spr.destxy.y, spr.destz, (const char*)junk.c_str());

//		if (!clip) { spr.destxy.x %= BufferWi; spr.destxy.y %= BufferWi; } //wrap
//		else if ((spr.destxy.x >= BufferWi) || (spr.destxy.y >= BufferHi) || (spr.destxy.x + keyw < 0) || (spr.destxy.y + keyh < 0)) continue; //outside of visible rect
		if ((srcx_off != srcx_on) || (srcy_off != srcy_on)) //need to draw when off also
		{
			if ((srcx_off < 0) || (srcx_off + spr.wh.x > Shapes.GetWidth()) || (srcy_off < 0) || (srcy_off + spr.wh.y > Shapes.GetHeight())) { debug_more(10, ": NO1"); ++numbad; continue; } //ignore invalid sprites
			spr.xy.push_back(wxPoint(srcx_off, srcy_off)); //state 0 == key off/up
            spr.off = find_color(Shapes, ColorMap, spr.xy.back(), spr.wh, "off");
		}
        if ((srcx_on < 0) || (srcx_on + spr.wh.x > Shapes.GetWidth()) || (srcy_on < 0) || (srcy_on + spr.wh.y > Shapes.GetHeight())) { debug_more(10, ": NO2"); ++numbad; continue; } //ignore invalid sprites
        spr.xy.push_back(wxPoint(srcx_on, srcy_on)); //state 1 == key on/down
        spr.on = find_color(Shapes, ColorMap, spr.xy.back(), spr.wh, "on");
        spr.ani_state = 0; //inactive
//        spr.repeat = 0;
		AllSprites[spr.name.ToStdString()] = spr;
//		if (spr.destz >= ByLayer.size()) ByLayer.resize(spr.destz + 1); //add new layer
//		ByLayer[spr.destz].push_back(&AllSprites[spr.name.ToStdString()]);
		debug_more(10, ", added '%s' ok? %d", spr.name.ToStdString().c_str(), AllSprites.find(spr.name.ToStdString()) != AllSprites.end());
	}
    debug(1, "%d shapes loaded, %d invalid", AllSprites.size(), numbad); //, ByLayer.size());
    CachedMapFilename = filename; //don't load same file again
}


//load start/stop times fort sprites:
//cues can overlap
void RgbEffects::Piano_load_cues(const wxString& filename)
{
    debug_function(10);
    debug(1, "load file %s", (const char*)filename.c_str());
    wxTextFile f;

    if (!CachedCueFilename.CmpNoCase(filename)) { debug_more(2, ", no change"); return; } //no change
    if (!wxFileExists(filename)) return;
    if (!f.Open(filename.c_str())) return;
    Piano_flush_cues(); //invalidate cached data
    debug(3, "read file");
    for (wxString linebuf = f.GetFirstLine(); !f.Eof(); linebuf = f.GetNextLine())
    {
        std::string::size_type ofs;
        if ((ofs = linebuf.find("#")) != std::string::npos) linebuf.erase(ofs); //remove comments
        while (!linebuf.empty() && (linebuf.Last() == '\\')) //line continuation
        {
            linebuf.RemoveLast(); //remove trailing "\"
            /*std::*/wxString morebuf = f.GetNextLine();
            if (f.Eof()) break;
            linebuf += morebuf;
        }
        while (!linebuf.empty() && isspace(linebuf.Last())) linebuf.RemoveLast(); //trim trailing spaces
        if (linebuf.empty()) continue; //skip blank lines

//start-time    end-time    shape-name
        debug(20, "got line '%s'", (const char*)linebuf.c_str());
//        linebuf += "\teol"; //end-of-line check for missing params
        wxStringTokenizer tkz(linebuf, "\t");
        Cue cue;
        cue.start_frame = Cue::Time2Frame(tkz.GetNextToken(), -1); //first column = start time (round down)
        cue.stop_frame = Cue::Time2Frame(tkz.GetNextToken(), +1); //second column = stop time (round up)
//        wxString junk = tkz.GetNextToken();
//        if (/* !junk.empty()*/ junk.Cmp("eol")) { debug.Append(": junk at end '%s'", (const char*)junk.c_str()).Flush(true); continue; } //TODO: show error messages?
        debug_more(10, " => start %d, stop %d, ok? %d", cue.start_frame, cue.stop_frame, cue.stop_frame >= cue.start_frame);
		if (cue.stop_frame < cue.start_frame) continue; //ignore null cues
        for (;;) //use remaining tokens as sprite names
        {
            wxString name = tkz.GetNextToken();
            if (name.empty()) break;
            if (name.find(".") == -1) name += ".000"; //kludge: change name to match Audacity Polyphonic nodes
//            debug.Append("add cue for sprite '%s'? %d", /*(const char*)name.c_str()*/ (const char*)name.ToStdString().c_str(), AllSprites.find(name.ToStdString()) != AllSprites.end()).Flush(true);
            if (AllSprites.find(name.ToStdString()) == AllSprites.end()) continue; //ignore missing sprites
            cue.sprite = &AllSprites[name.ToStdString()];
            CuesByStart.push_back(cue);
        }
	}
	/*std::*/sort(CuesByStart.begin(), CuesByStart.end(), Cue::SortByStart);
    debug(3, "%d cues loaded, first '%s' starts/ends %d/%d, last '%s' starts/ends %d/%d", CuesByStart.size(), CuesByStart.size()? CuesByStart.front().sprite->name.ToStdString().c_str(): "", CuesByStart.size()? CuesByStart.front().start_frame: -1, CuesByStart.size()? CuesByStart.front().stop_frame: -1, CuesByStart.size()? CuesByStart.back().sprite->name.ToStdString().c_str(): "", CuesByStart.size()? CuesByStart.back().start_frame: -1, CuesByStart.size()? CuesByStart.back().stop_frame: -1);
    CachedCueFilename = filename; //don't load same file again
}


//resolve overlapping or clipped sprites:
//thiss must be repeated if clip/wrap option or canvas size is changed
#if 0
void resolve_visible(int Buffer_Wi, int Buffer_Ht, bool Clipping)
{
    Debug debug("resolve_visible(w %d, h %d, clip? %d)", Buffer_Wi, Buffer_Ht, Clipping);

    VisibleSprites.clear();
    for (auto it = AllSprites.begin(); it != AllSprites.end(); ++it)
    {
        if (Clipping && (destxy.x >= BufferWi) || (destxy.y >= BufferHt) || (destxy.x + keyw < 0) || (destxy.y + rowh < 0)) continue; //outside of visible rect

    }
}
#endif


//update previous bkg:
//if it's a scrolling piano, scroll previous notes up one row
//for eqbars, shorten them
void RgbEffects::Piano_update_bkg(int Style, wxSize& canvas, int rowh)
{
    debug_function(10);
    debug(1, "style %d", Style);
    xlColor c;
//TODO: should use GDI+ functions on Windows
//TODO: use GetData for better performance; probably doesn't matter since grid is fairly small (as compared to normal images or screen canvas)
//TODO: speed
    switch (Style)
    {
        case -1: //initialize
            debug_more(5, ", init canvas %d x %d", canvas.x, canvas.y);
//            PrevRender.clear(); //start with blank canvas
//            PrevRender.resize(canvas.x * cnavas.y); //set all pixels off
            for (int x = 0; x < canvas.x; ++x)
                for (int y = 0; y < canvas.y; ++y)
                    SetPixel(x, y, c); //clear all (background canvas is persistent while piano effect is active)
            return;
        case PIANO_STYLE_SCROLLING: //scroll up one row
            debug_more(5, ", scroll %d x %d up by %d", canvas.x, canvas.y, rowh);
            for (int x = 0; x < canvas.x; ++x)
                for (int y = 0; y < canvas.y; ++y)
                    if (y < canvas.y - rowh) { debug_more(30, ", (%d,%d)->(%d,%d)", x, canvas.y - y - rowh - 1, x, canvas.y - y - 1); CopyPixel(x, canvas.y - y - rowh - 1, x, canvas.y - y - 1); }
                    else { debug_more(30, ", (%d,%d)<-0", x, canvas.y - y - 1); SetPixel(x, canvas.y - y - 1, c); } //clear bottom row, scroll others
            return;
        case PIANO_STYLE_EQBARS: //scroll down one row (decaying bars)
            debug_more(5, ", scroll %d x %d down by %d", canvas.x, canvas.y, rowh);
//            c.Set(255, 255, 255); //debug
            for (int x = 0; x < canvas.x; ++x)
                for (int y = 0; y < canvas.y; ++y)
                    if (y < canvas.y - rowh) { debug_more(30, ", (%d,%d)->(%d,%d)", x, y + rowh, x, y); CopyPixel(x, y + rowh, x, y); }
                    else { debug_more(30, ", (%d,%d)<-0", x, y); SetPixel(x, y, c); } //clear top row, scroll other rows
            return;
        case PIANO_STYLE_ICICLES: //scroll down one pixel (drip)
            debug_more(5, ", scroll %d x %d", canvas.x, canvas.y);
            for (int x = 0; x < canvas.x; ++x)
                for (int y = 0; y < canvas.y; ++y)
                    if (y < canvas.y - 1) { debug_more(30, ", (%d,%d)->(%d,%d)", x, y + 1, x, y); CopyPixel(x, y + 1, x, y); }
//                    else { debug_more(30, ", (%d,%d)<-0", x, y); SetPixel(x, y, c); } //clear top pixel, scroll other pixels
            return;
        default:
            debug_more(5, ", no scrolling");
    }
}


//map user-selected colors to sprite colors:
//    size_t colorcnt = GetColorCount();
//    int ColorIdx = rand() % colorcnt; //TODO: assign random colors?
//    wxImage::HSVValue hsv;
//    Color2HSV(color, hsv);
#if 0
void RgbEffects::Piano_map_colors(void)
{
    debug_function(10);
    color_map.clear();
    if (GetColorCount() < 2) return; //use colors from shapes file if no user-selected colors
    for (auto /*std::vector<int>::iterator*/ it = ShapePalette.begin(); it != ShapePalette.end(); ++it)
    {
        xlColor color;
        palette.GetColor((it - ShapePalette.begin()) % GetColorCount(), color); //assign user-selected colors to shape palette sequentially, loop if run out of colors
        color_map[*it] = color; //.GetRGB();
        debug(10, "shape color[%d/%d] 0x%x => user-selected color [%d/%d] 0x%x", it - ShapePalette.begin(), ShapePalette.size(), *it, (it - ShapePalette.begin()) % GetColorCount(), GetColorCount(), color.GetRGB());
    }
}
#endif


//render piano fx during sequence:
void RgbEffects::RenderPiano(const wxString & StyleStr, int NumKeys, int NumRows, const wxString & Placement, bool Clipping, const wxString& CueFilename, const wxString& MapFilename, const wxString& ShapeFilename)
{
    int Style = GetPianoStyle(StyleStr);
//    wxImage::HSVValue hsv;
//    xlColor color;
//    int ColorIdx;
//    int keys_mod;
    debug_function(9);
    static std::vector<Cue>::iterator cue_cursor; //cached cue ptr
//    static /*std::*/deque</*std::*/vector<int>> history; //cached history
    static int prev_update;
    static unsigned int cached_state = -1;
    int curframe = curPeriod - curEffStartPer; //units of 50 msec
    int state = curframe;
    debug(2, "RenderPiano[%d -> %d] speed %d, frame %d, %f sec, empty? %d", cached_state, state, speed, curframe, (float)curframe*50/1000, CuesByStart.empty());
    if (Style != PIANO_STYLE_ANIMAGE) InhibitClear = true; //allow canvas to be persistent so we don't need to keep redrawing it

    NumKeys = BufferWi/3; //TODO: make this look better for other values

//get/check/fix up rendering params:
    if (NumKeys < 2) NumKeys = 7; //default to 1 octave
    if (NumKeys > BufferWi) NumKeys = BufferWi; //give each key at least 1 pixel in order to be visible
    if (!WantHistory(Style)) NumRows = 1;
    else if (NumRows < 2) NumRows = NumKeys * BufferHt / BufferWi; //default square keys
    if (NumRows > BufferHt) NumRows = BufferHt; //each row needs at least 1 pixel in order to be visible
    if (Style == PIANO_STYLE_ANIMAGE) NumKeys = NumRows = 1; //use entire canvas
    int adjustw = Clipping? divup(NumKeys, 7): NumKeys/7; //round up vs. down
    if (adjustw < 1) adjustw = 1; //kludge: compiler bug; gcc is generating a imul instr here! gives 0 for NumKeys == 10
    wxSize keywh(BufferWi / NumKeys, BufferHt / NumRows), /*BufferWH_full(BufferWi, BufferHt),*/ BufferWH_octave(7 * adjustw * keywh.x, NumRows * keywh.y); //wrap on octave boundaries only (so notes don't move); NOTE: only count white keys (black ones overlap), so octave width is actually 7 keys, not 12
    wxSize keywh_1row(BufferWi / NumKeys, BufferHt / 5 + rand() % (BufferHt * 4/5 + 1)); //kludge: simulate varying amplitudes for eq bars
    if (Style == PIANO_STYLE_ANIMAGE) BufferWH_octave.x = keywh.x; //use entire canvas
    if (Style == PIANO_STYLE_EQBARS) BufferWH_octave.y = BufferHt; //use entire height
//    int yscroll = 0;
    debug_more(5, ": style %d, #keys %d, keyw %d/%d(%d), #rows %d, rowh %d/%d(%d)", Style, NumKeys, keywh.x, BufferWH_octave.x, BufferWi, NumRows, keywh.y, BufferWH_octave.y, BufferHt);

//NOTE: overlap needs more work
//if 2 keys wrap to the same canvas location and one turns off, should it be redrawn?
//if any keys are on, you want to see them; OTOH if anything changes, you also want to see that
//since the purpose of thiss is animation, we'll take the latter option for now
    std::hash_map<wxPoint_, int> drawn; //don't draw overlapping regions more than once

//initialize cached data:
//NOTE: must come thru here if fx params changed
	if ((state < cached_state) || CuesByStart.empty()) //start of playback or rewind/backtrack, or keep trying to load cues
	{
		Piano_load_shapes(ShapeFilename);
		Piano_load_sprite_map(MapFilename); //, Clipping);
		Piano_load_cues(CueFilename);
//		history.clear();
//		resolve_visible(Buffer_Wi, Buffer_Ht, Clipping);
//        Piano_map_colors();
		cue_cursor = CuesByStart.begin(); //rewind
//        ActiveCues = std::priority_queue<Cue*, std::vector<Cue*>, Cue /*SortByStop*/>();
        ActiveCues.clear();
        Piano_update_bkg(-1, BufferWH_octave, keywh.y); //initialize canvas
        for (auto it = AllSprites.begin(); it != AllSprites.end(); ++it) //draw all sprites in their initial state
        {
            it->second.ani_state = 0; //always restart keys in inactive state?
            if (Style != PIANO_STYLE_ANIMAGE)
                Piano_RenderKey(&it->second, drawn, Style, BufferWH_octave, keywh, Placement, Clipping);
        }
		prev_update = 0;
        cached_xy = wxPoint(-1, -1);
	}
	cached_state = state; //detect rewind
    if (CuesByStart.empty()) return; //nothing to draw

//    if (ShowHistory(Style))
//    {
//        history.push_back(/*std::*/vector<int>(NumKeys)); //.emplace_back(); //add new row for current frame
//        if (history.size() > NumRows) history.pop_front(); //drop oldest row
//    }

    if (WantHistory(Style) && (state - prev_update >= 1000/50)) //time to scroll; speed == #times/sec
    {
        Piano_update_bkg(Style, BufferWH_octave, keywh.y); //scrolling
        prev_update = state;
    }
//                SetPixel(x-xoffset,(state % ((imght+BufferHt)*speedfactor)) / speedfactor-y,c); //moving up
//                    SetPixel(x-xoffset,BufferHt+imght-y-(state % ((imght+BufferHt)*speedfactor)) / speedfactor,c); //moving down

//prune expired cues, update sprite state:
//    drawn.clear(); //redraw Off sprites only once
//#define top  front
//#define pop  pop_front
//#define push  push_back
    bool repaint = false;
    debug(6, "checking %d active cues for expiration <= frame %d, first is '%s' %d", ActiveCues.size(), curframe, ActiveCues.size()? (const char*)ActiveCues.top()->sprite->name.c_str(): "", ActiveCues.size()? ActiveCues.top()->stop_frame: -1);
	while (ActiveCues.size() && (ActiveCues.top()->stop_frame <= curframe)) //stopped during previous frame
	{
	    debug(7, "prune: '%s' is first of %d stops @%d vs. %d", (const char*)ActiveCues.top()->sprite->name.c_str(), ActiveCues.size(), ActiveCues.top()->stop_frame, curframe);
		ActiveCues.top()->sprite->ani_state = 0; //inactive state
		if (Style != PIANO_STYLE_ANIMAGE)
            Piano_RenderKey(ActiveCues.top()->sprite, drawn, Style, BufferWH_octave, keywh, Placement, Clipping);
		ActiveCues.pop(); //remove first element (soonest to expire)
		if (ActiveCues.size()) { debug_more(7, ", next is '%s' %d, ", (const char*)ActiveCues.top()->sprite->name.c_str(), ActiveCues.top()->stop_frame); }
		repaint = true;
	}

//add new cues, update sprite state:
//NOTE: do thiss after pruning so cues with short durations get at least one frame
//	if (!state) start_cursor = ByStart.begin(); //rewind at start of playback
//	while ((start_cursor != ByStart.end()) && (start_cursor->start_frame < now + speed)) //start during thiss frame
//    drawn.clear(); //redraw On sprites over Off sprites only once
    debug(6, "checking %d - %d cues for activation <= frame %d, next is '%s' %d", CuesByStart.size(), cue_cursor - CuesByStart.begin(), curframe, (cue_cursor != CuesByStart.end())? (const char*)cue_cursor->sprite->name.c_str(): "", (cue_cursor != CuesByStart.end())? cue_cursor->start_frame: -1);
//    for (cue_cursor = /*CuesByStart.begin()*/ /*std::*/ lower_bound(/*cue_cursor*/ CuesByStart.begin(), CuesByStart.end(), Cue(state), Cue::SortByStart); /*(cue_cursor != CuesByStart.end()) && (cue_cursor->start_frame <= state)*/; ++cue_cursor)
    while ((cue_cursor != CuesByStart.end()) && (cue_cursor->start_frame <= curframe)) //TODO: use lower_bound for better performance
	{
//	    debug.Append("activate check: cur@ %d, end? %d, start_frame %d <= frame %d? %d", cue_cursor - CuesByStart.begin(), cue_cursor == CuesByStart.end(), cue_cursor->start_frame, state, cue_cursor->start_frame <= state).Flush(true);
//        if ((cue_cursor == CuesByStart.end()) || (cue_cursor->start_frame > state)) break;
	    debug(7, "activate: '%s' starts/stops %d/%d, has %d states", (const char*)cue_cursor->sprite->name.c_str(), cue_cursor->start_frame, cue_cursor->stop_frame, cue_cursor->sprite->xy.size());
		cue_cursor->sprite->ani_state = 1; //first active state
		if (Style != PIANO_STYLE_ANIMAGE)
            Piano_RenderKey(cue_cursor->sprite, drawn, Style, BufferWH_octave, (Style == PIANO_STYLE_EQBARS)? keywh_1row: keywh, Placement, Clipping);
		if (Style != PIANO_STYLE_EQBARS) //eq bars will age out by themselves
            ActiveCues.push(&*cue_cursor);
        else if ((ActiveCues.size() > 1) && (ActiveCues.c[0]->stop_frame > ActiveCues.c[1]->stop_frame)) //paranoid check
        {
            debug(5, "priority_queue broken! [0] %d > [1] %d", ActiveCues.c[0]->stop_frame, ActiveCues.c[1]->stop_frame);
//        sort(ActiveCues.begin(), ActiveCues.end(), Cue::SortByStop); //kludge: priority_queue not working, so explicitly sort it
//                std::swap(ActiveCues.c.begin(), ActiveCues.c.begin() + 1);
            ActiveCues.c.erase(ActiveCues.c.begin()); //remove and try again
            for (auto it = ActiveCues.c.begin();; ++it)
                if ((it == ActiveCues.c.end()) || (cue_cursor->stop_frame < (*it)->stop_frame))
                {
                    ActiveCues.c.insert(it, &*cue_cursor);
                    debug_more(5, ", fixed @%d", it - ActiveCues.c.begin());
                    break;
                }
        }
		++cue_cursor;
		repaint = true;
	}
//#undef top
//#undef pop
//#undef push

//TODO: fix repaint logic
    if (Style == PIANO_STYLE_ANIMAGE)//&& repaint) //repaint all now
        for (auto it = AllSprites.begin(); it != AllSprites.end(); ++it) //draw all sprites in their initial state
            if (it->second.ani_state) //redraw active sprites
                Piano_RenderKey(&it->second, drawn, Style, BufferWH_octave, keywh, Placement, Clipping);
}


bool RgbEffects::Piano_RenderKey(Sprite* sprite, std::hash_map<wxPoint_, int>& drawn, int style, wxSize& canvas, wxSize& keywh, const wxString &placement, bool clip)
//bool RgbEffects::Sprite::render(wxSize& keywh, wxSize& BufferWH_int, int yscroll, bool Clipping)
{
    debug_function(9);

//hash_map<pair<wxPoint, wxSize>, int>& drawn)
//PIANO_STYLE_KEYS sprites have 2 states: on (down) and off (up); draw all sprites; top view or edge view
    int drawstate = sprite->ani_state++; //bump to next active (animation) state
    if (!drawstate) sprite->ani_state = 0; //stay in inactive state
    else
//    if ((xy.size() == 1) && (drawstate == -1)) return false; //inactive on/off sprite; don't need tp draw anything
        if (drawstate >= sprite->xy.size()) //end of animation
//				if (it->repeat > 0) drawstate = 0; //loop immediately
//				else if (it->repeat < 0) drawstate = -rnd(); //loop with delay
        /*else*/ drawstate = sprite->xy.size() - 1; //stay at last state; don't loop

//    wxPoint realxy = sprite->destxy;
//    realxy.y += yscroll; //scrolling
    debug_more(30, ", dest (%d => %d, %d), #drawn %d", sprite->destxy.x, (clip? sprite->destxy.x: sprite->destxy.x % canvas.x), sprite->destxy.y, drawn.size());
    if (clip)
        if ((sprite->destxy.x >= BufferWi) || (sprite->destxy.y >= BufferHt) || (sprite->destxy.x + keywh.x < 0) || (sprite->destxy.y + keywh.y < 0)) return false; //outside of visible rect
//    debug_more(30, ", here1");
    wxPoint_ where = sprite->destxy.y * 65536 + (clip? sprite->destxy.x: sprite->destxy.x % canvas.x); //wrap on even key boundary
//    debug_more(30, ", here2");
    if ((style != PIANO_STYLE_ANIMAGE) && (drawn.find(where) != drawn.end()) && (drawstate <= drawn[where])) { debug_more(30, ", already drawn[0x%x]=%d vs %d", where, drawn[where], drawstate); return false; } //do not redraw older states in same location
    drawn[where] = drawstate; //remember highest state drawn in this location

//don't draw overlapping regions more than once

//                    SetPixel(x-xoffset,(state % ((imght+BufferHt)*speedfactor)) / speedfactor-y,c); //moving up
//                    SetPixel(x-xoffset,BufferHt+imght-y-(state % ((imght+BufferHt)*speedfactor)) / speedfactor,c); //moving down
//copy sprite image to pixel buffer, scale up/down:
//iterate thru target pixels and pull from sprite in case sizes don't match (need to set all target pixels, but okay to skip some source pixels)
    float xscale = (float)sprite->wh.x / keywh.x, yscale = (float)sprite->wh.y / keywh.y; //src -> dest scale factor
//    debug_more(30, ", here3");
//TODO: use wxImage.GetData for better performance?
    int xofs = !clip? (BufferWi % (7 * keywh.x)) / 2: 0; //center keys if not clipped
    if (WantHistory(style)) debug(20, "draw sprite '%s': set x/y %d/%d + %d/%d to 0x%x", (const char*)sprite->name.ToStdString().c_str(), sprite->destxy.x, sprite->destxy.y, keywh.x, keywh.y, drawstate? sprite->on.GetRGB(): sprite->off.GetRGB()); //.Flush(true);
    else debug(20, "draw sprite '%s': copy from x/y[%d/%d] %d/%d + %d/%d => x/y %d/%d + %d/%d, x/y scale = %f/%f", (const char*)sprite->name.ToStdString().c_str(), drawstate, sprite->xy.size(), sprite->xy[drawstate].x, sprite->xy[drawstate].y, sprite->wh.x, sprite->wh.y, sprite->destxy.x, sprite->destxy.y, keywh.x, keywh.y, 1.0 / xscale, 1.0 / yscale); //.Flush(true);
    for (int x = 0; x < keywh.x; ++x) //copying to it->w columns in dest
        for (int y = 0; y < keywh.y; ++y) //copying to it->h rows in dest; vert scaling is more likely, so make Y the inner loop for better pixel caching
        {
//            static xlColor cached_rgb; //cached mapped pixel color
//            static wxPoint cached_xy(-1, -1);
            wxPoint src_xy(sprite->xy[drawstate].x + x * xscale, sprite->xy[drawstate].y + y * yscale);
//TODO: scale doesn't make sense for all cases
            src_xy.y = Shapes.GetHeight() - src_xy.y - 1; //whoops, origin is top left but wanted bottom left
            bool transparent = 0;
            if (WantHistory(style)) cached_rgb = drawstate? sprite->on: sprite->off; //kludge: fill rect with same color to avoid losing pixels due to scaling
            else if ((src_xy.x != cached_xy.x) || (src_xy.y != cached_xy.y)) //update cached pixel info
            {
                cached_xy = src_xy; //prev_xy.x = src_xy.x; prev_y = srcy; //not sure how expensive wx pixel functions are, so cache current pixel info just in case; aliasing/averaging and color mapping also makes thiss more expensive
                if (Shapes.IsTransparent(src_xy.x, src_xy.y)) transparent = 1; //-1; //-1 matches white, so use + instead
                else
                {
//                        xlColor c;
//TODO: tile, center, anti-aliasing
                    cached_rgb.Set(Shapes.GetRed(src_xy.x, src_xy.y), Shapes.GetGreen(src_xy.x, src_xy.y), Shapes.GetBlue(src_xy.x, src_xy.y)); //NOTE: need to do pixel merging if scale is not 1:1
                    if (!ColorMap.empty()) cached_rgb = ColorMap[cached_rgb.GetRGB()]; //map to user-selected colors
                }
                debug_more(20, ", LK(%d,%d)", cached_xy.x, cached_xy.y);
            }
            if (transparent == 1 /*-1*/) continue; //don't need to draw pixel
            int wrapx = sprite->destxy.x + x, scrolly = sprite->destxy.y;
//            if (style == PIANO_STYLE_ANIMAGE) { wrapx *= xscale; scrolly *= yscale; }
            if (!clip) wrapx %= canvas.x; //wrap on even key boundary
//            if ((style == PIANO_STYLE_ICICLES) || (style == PIANO_STYLE_EQBARS)) scrolly += canvas.y - keywh.y; //draw at top instead of bottom
            if (style == PIANO_STYLE_ICICLES) scrolly += canvas.y - keywh.y; //draw at top instead of bottom
//            debug_more(20, ", %d+%d vs. %d-%d? %d", xofs, wrapx, BufferWi, xofs, xofs + wrapx < BufferWi - xofs);
//            if (!clip) wrapx = (wrapx + 2 * xofs) % BufferWi - 2 * xofs; //wrap within reduced area, not expanded area
            debug_more(20, ", (%d,%d)<-0x%x", wrapx, sprite->destxy.y + y, cached_rgb.GetRGB());
            if (xofs + wrapx < BufferWi - xofs) SetPixel(xofs + wrapx, sprite->destxy.y + y, cached_rgb); //no vertical wrap, only horizontal wrap
        }
//    debug.Flush(true);
    return true;
}


//sort active spites by state:
//older (lower state) sprites will be drawn first, higher state sprites will be drawn last
//thiss allows piano keyboard to look correct if keys are wrapped (don't want an Off key to overwrite an On key)
#if 0
    priority_queue<int, vector<int>, greater<int>> ActiveStates;
    hash_map<int, vector<Sprite*>> ByState;
    for (auto it = AllSprites.begin(); it != AllSprites.end(); ++it)
    {
        if (ByState[it->second.state].empty()) ActiveStates.push(it->second.state);
        ByState[it->second.state].push_back(&it->second);
    }
    debug.Append("%d active states to draw: ", ActiveStates.size());
    for (auto it = ActiveStates.c.begin(); it != ActiveStates.c.end(); ++it)
        debug.Append("%d, ", *it);
    debug.Flush(true);
#endif

//update state and draw all active sprites:
//target canvas starts blank, so we don't need to draw transparent pixels?
#if 0
    int draw_count = 0; //debug
    while (ActiveStates.size())
    {
// if (draw_count > 0) break;
        for (auto it = ByState[ActiveStates.top()].begin(); it != ByState[ActiveStates.top()].end(); ++it)
//	for (int zinx = 0; zinx < 1 /*ByLayer.size()*/; ++zinx) //use layers to control overlapping regions
//		for (auto /*std::vector<int, std::vector<Sprite*>>*/ it = ByLayer[zinx].begin(); it != ByLayer[zinx].end(); ++it)
		{
//TODO: PIANO_STYLE_CORGAN
//TODO: PIANO_STYLE_EQBARS
//TODO: PIANO_STYLE_SCROLLING
//TODO: PIANO_STYLE_ICICLES
//                    SetPixel(x-xoffset,(state % ((imght+BufferHt)*speedfactor)) / speedfactor-y,c); //moving up
//                    SetPixel(x-xoffset,BufferHt+imght-y-(state % ((imght+BufferHt)*speedfactor)) / speedfactor,c); //moving down

//PIANO_STYLE_KEYS sprites have 2 states: on (down) and off (up); draw all sprites; top view or edge view
			int drawstate = (*it)->state;
			if (((*it)->xy.size() == 1) && (drawstate == -1)) continue; //inactive on/off sprite; don't redraw
			if (++drawstate >= (*it)->xy.size()) //end of animation
//				if (it->repeat > 0) drawstate = 0; //loop immediately
//				else if (it->repeat < 0) drawstate = -rnd(); //loop with delay
				/*else*/ drawstate = (*it)->xy.size() - 1; //don't loop
            wxPoint destxy = (*it)->destxy;
            if (Clipping && (destxy.x >= BufferWi) || (destxy.y >= BufferHt) || (destxy.x + keyw < 0) || (destxy.y + rowh < 0)) continue; //outside of visible rect
            ++draw_count;
//            if (draw_count > 10) break;

//copy sprite image to pixel buffer:
//iterate thru target pixels and pull from sprite in case sizes don't match (need to set all target pixels, but okay to skip some source pixels)
            float xscale = (float)(*it)->wh.x / keyw, yscale = (float)(*it)->wh.y / rowh; //src -> dest scale factor
//TODO: use wxImage.GetData for better performance?
            debug.Append("draw sprite '%s': copy from x/y[%d/%d] %d/%d + %d/%d => x/y %d/%d + %d/%d, x/y scale = %f/%f", (const char*)(*it)->name.c_str(), drawstate, (*it)->xy.size(), (*it)->xy[drawstate].x, (*it)->xy[drawstate].y, (*it)->wh.x, (*it)->wh.y, destxy.x, destxy.y, keyw, rowh, xscale, yscale); //.Flush(true);
            xlColor rgb; //cached mapped pixel color
            for (int x = 0; x < keyw; ++x) //copying to it->w columns in dest
                for (int y = 0; y < rowh; ++y) //copying to it->h rows in dest; vert scaling is more likely, so make Y the inner loop for better pixel caching
                {
                    static int prev_x = -1, prev_y = -1;
                    int srcx = (*it)->xy[drawstate].x + x * xscale, srcy = (*it)->xy[drawstate].y + y * yscale;
                    srcy = Shapes.GetHeight() - srcy - 1; //whoops, origin is top left but wanted bottom left
                    if ((srcx != prev_x) || (srcy != prev_y)) //update cached pixel info
                    {
                        prev_x = srcx; prev_y = srcy; //not sure how expensive wx pixel functions are, so cache current pixel info just in case; aliasing/averaging and color mapping also makes thiss more expensive
                        if (Shapes.IsTransparent(srcx, srcy)) rgb = -1;
                        else
                        {
//                            xlColor c;
//TODO: tile, center, anti-aliasing
                            rgb.Set(Shapes.GetRed(srcx, srcy), Shapes.GetGreen(srcx, srcy), Shapes.GetBlue(srcx, srcy)); //NOTE: need to do pixel merging if scale is not 1:1
                            if (!color_map.empty()) rgb = color_map[rgb.GetRGB()]; //map to user-selected colors
                        }
                        debug.Append(", LK=0x%x", rgb.GetRGB());
                    }
                    int wrapx = destxy.x + x;
                    if (!Clipping) wrapx %= BufferWi_int; //wrap on even key boundary
                    if (rgb != -1) debug.Append(", (%d,%d)->(%d,%d)", srcx, srcy, wrapx, destxy.y + y);
                    if (rgb != -1) SetPixel(wrapx, destxy.y + y, rgb); //no vertical wrap
                }
            debug.Flush(true);
		}
		ActiveStates.pop();
    }
    debug.Append("shapes drawn: %d", draw_count).Flush(true);
#endif

#if 0
//    char buf[100];
//    sprintf(buf, "Playback: piano fx, style %d, #keys %d, keyw %d, buf w %d, buf h %d", Style, NumKeys, KeyWidth, BufferWi, BufferHt);
//    StatusBar1->SetStatusText(_(buf));
//    djdebug("[%d] style %d, #keys %d, keyw %d, w %d, h %d, fx h %d, hist len %d", state, Style, NumKeys, KeyWidth, BufferWi, BufferHt, height, WasNoteOn.size());
    for (int xofs = 0; xofs < BufferWi; xofs += KeyWidth) // xofs/KeyWidth == key#
    {
        bool isdown = IsNoteOn(xofs/KeyWidth, state);
//how to handle colors?
//choose different color for each key?

        switch (Style)
        {
            case PIANO_STYLE_KEYEDGE: //edge of keys go up/down according to whether note is on/off, note color is ????
            case PIANO_STYLE_KEYTOP:
                height = isdown? BufferHt: BufferHt/2;
                hsv.value = isdown? .5: 1.0; //make "on" (down) keys brighter??
//                djdebug("  key notex %d/%d: ht %d, bright %f", xofs/KeyWidth, BufferWi/KeyWidth, height, hsv.value);
                break;
            case PIANO_STYLE_CORGAN: //don't even show keys that are off
//                djdebug("  corgan notex %d/%d: isdown? %d", xofs/KeyWidth, BufferWi/KeyWidth, isdown);
                if (!isdown) continue;
                break;
            case PIANO_STYLE_EQBARS: //height indicates strength/loudness of note
                height = BufferHt * NoteVolume(xofs/KeyWidth, state) / 7;
//                djdebug("  eq notex %d/%d: vol %d, ht %d", xofs/KeyWidth, BufferWi/KeyWidth, NoteVolume(xofs/KeyWidth, state), height);
                break;
            case PIANO_STYLE_SCROLLING: //history scrolling
                WasNoteOn.back()[xofs / KeyWidth] = isdown;
//                djdebug("  scr notex %d/%d: ison %d", xofs/KeyWidth, BufferWi/KeyWidth, isdown);
                break;
//            case PIANO_STYLE_ICICLES: //start drip when note is on
        }

        for (int x = 0; (x < KeyWidth) && (xofs + x < BufferWi); ++x)
            for (int y = 0; y < height; ++y)
            {
                if (Style == PIANO_STYLE_SCROLLING)
                {
                    int row = y / KeyWidth;
                    if (WasNoteOn.size() < row) continue; //row wasn't displayed yet
                    if (!WasNoteOn[row][xofs/KeyWidth]) continue; //note is not on currently
                }
                SetPixel(xofs + x, y, hsv);
            }
    }
#endif
#if 0
    switch (Style)
    {
        case PIANO_STYLE_KEYTOP: //view from above; black and white keys have different shapes according to place within octave
//fill one key for now:
            for (int x = 0; x < KeyWidth; ++x)
                for (int y = 0; y < height; ++y)
                    SetPixel(x, y, hsv);
            break;

        case PIANO_STYLE_KEYEDGE:

        case PIANO_STYLE_SCROLLING:

        case PIANO_STYLE_EQBARS:

        case PIANO_STYLE_ICICLES:
    }
                            SetPixel(x,y,hsv);

//#if 1

    //  BufferWi ,BufferHt
    int y_start = (int) ((BufferHt/2.0) - (height/2.0));
    int y_end = y_start+height;

    int x_end,y_end2;
    y_end2 =(int) ( y_start+(height/2));


    hsv.hue=0.0;
    hsv.saturation=1.0;
    hsv.value=1.0;

    //   Keys C,F Type 1
    //  Keys D,G,A Type 2
    //  Keys E,B Type 3
    //  Keys C#,D#,F#,G#,A# Type 4

    int basex,x1,x2;
    for (keys=1; keys<=25; keys++)
    {
        int x_start;
        x_start=1+((keys-1)*width);
        x_end=x_start+width-1;
        keys_mod=keys%12; // 12 notes per octave

        switch (keys_mod)
        {
        case 1: // C
        case 6: // F
        {
            if(keys_mod==1) basex = 1;
            if(keys_mod==6)
            {
                basex = 3*width+1;
            }
            x_start=basex+((int)((keys-1)/12))*width*7;
            // hsv.hue=0.2 + (state%10)/10;
            if(hsv.hue>1.0)  hsv.hue = (state%10)/10;
            hsv.hue=.28;
            hsv.saturation=.50;
            hsv.value=.70;
            if(keys_mod==1)
            {
                hsv.hue=.60;
                hsv.saturation=.43;
                hsv.value=.79;
            }

            if(keys>99 && (int)((state/100)+keys)%2 == 1)
            {
                hsv.hue=.18;
                hsv.saturation=.71;
                hsv.value=.90;
            }
            if(Keyboard==3)
            {
                if(width==7)
                {
                    for(x=x_start; x<=x_start+3; x++)
                    {
                        for (y=y_start; y<=y_end; y++)
                        {
                            SetPixel(x,y,hsv);
                        }
                    }

                    for (y=y_start; y<=y_end2; y++)
                    {
                        SetPixel(x_start+4,y,hsv);
                        SetPixel(x_start+5,y,hsv);
                    }
                    // black out around accidental key
                    hsv.value=0.0; // black
                    for (y=y_end2; y<=y_end; y++)
                    {
                        SetPixel(x_start+3,y,hsv);
                    }
                    for(x=x_start+3; x<=x_start+5; x++)
                    {
                        SetPixel(x,y_end2,hsv);
                    }

                }
                else
                {


                    for(x=x_start; x<=x_end; x++)
                    {
                        for (y=y_start; y<=y_end; y++)
                        {
                            SetPixel(x,y,hsv);
                        }
                    }

                    for (y=y_end2; y<=y_end; y++)
                    {
                        SetPixel(x_end,y,hsv);
                    }
                }
            }
        }
        break;

        case 2:  // C#
        case 4:  // Eb
        case 7:  // F#
        case 9:  // Ab
        case 11: // Bb
        {
            //  hsv.hue=0.4  + (state%10)/10;
            if(keys_mod==2) basex = width+1;
            if(keys_mod==4) basex = 2*width+1;
            if(keys_mod==7) basex = 4*width+1;
            if(keys_mod==9) basex = 5*width+1;
            if(keys_mod==11) basex = 6*width+1;
            x_start=basex+((int)((keys-1)/12))*width*7;
            hsv.hue=.71;
            hsv.saturation=.43;
            hsv.value=.79;

            if(keys>99 &&  (int)((state/100)+keys)%2 == 0)
            {
                hsv.hue=.18;
                hsv.saturation=.71;
                hsv.value=.90;
            }

            if(width==7)
            {
                for(x=x_start-3 ; x<=x_start+1; x++)
                {
                    for (y=y_end2; y<=y_end; y++)
                    {
                        SetPixel(x,y,hsv);
                    }
                }
                // black out around accidental key
                x1=x_start-4; x2=x_start+1;
                if(keys_mod==4 || keys_mod==9)
                {
                     x1=x_start-3; x2=x_start+2;
                }
                hsv.value=0.0; // black
                for (y=y_end2; y<=y_end; y++)
                {
                    SetPixel(x1,y,hsv);
                    SetPixel(x2,y,hsv);
                }
                for(x=x1; x<=x2; x++)
                {
                    SetPixel(x,y_end2,hsv);
                }
            }
            else
            {
                for(x=x_start-1 ; x<=x_start; x++)
                {
                    for (y=y_start; y<=y_end2; y++)
                    {
                        SetPixel(x,y,hsv);
                    }
                }
            }



        }
        break;

        case 3:  // D
        case 8:  // G
        case 10: // A
        {
            if(keys_mod==3) basex = 1*width+1;
            if(keys_mod==8) basex = 4*width+1;
            if(keys_mod==10) basex = 5*width+1;
            x_start=basex+((int)((keys-1)/12))*width*7;
            // hsv.hue=0.6  + (state%10)/10;
            hsv.hue=.48;
            hsv.saturation=.73;
            hsv.value=.74;
            if(keys_mod==8)
            {
                hsv.hue=.60;
                hsv.saturation=.43;
                hsv.value=.79;
            }
            if(keys>99 && (int)((state/100)+keys)%3 == 1)
            {
                hsv.hue=.18;
                hsv.saturation=.71;
                hsv.value=.90;
            }
            if(Keyboard==3)
            {
                for (y=y_start; y<=y_end; y++)
                {
                    if(width==7)
                    {
                        SetPixel(x_start+2,y,hsv);
                        SetPixel(x_start+3,y,hsv);
                    }
                    else
                        SetPixel(x_start+1,y,hsv);
                }
                for (y=y_start; y<=y_end2; y++)
                {
                    if(width==7)
                    {
                        SetPixel(x_start,y,hsv);
                        SetPixel(x_start+1,y,hsv);
                        SetPixel(x_start+4,y,hsv);
                        SetPixel(x_start+5,y,hsv);
                    }
                    else
                    {
                        SetPixel(x_start,y,hsv);
                        SetPixel(x_end,y,hsv);
                    }
                }

                if(width==7)
                {
                    // black out around accidental key
                    hsv.value=0.0; // black
                    for (y=y_end2; y<=y_end; y++)
                    {
                        SetPixel(x_start+1,y,hsv);
                        SetPixel(x_start+4,y,hsv);
                    }
                    for(x=x_start; x<=x_start+1; x++)
                    {
                        SetPixel(x,y_end2,hsv);
                    }
                    for(x=x_start+4; x<=x_start+5; x++)
                    {
                        SetPixel(x,y_end2,hsv);
                    }
                }
            }
        }
        break;

        case 5:   // E
        case 0:   // B
        {
            if(keys_mod==5) basex = 2*width+1;
            if(keys_mod==0) basex = 6*width+1;

            x_start=basex+((int)((keys-1)/12))*width*7;
            // hsv.hue=0.2  + (state%10)/10;
            hsv.hue=.60;
            hsv.saturation=.43;
            hsv.value=.79;

            if(Keyboard==3)
            {
                if(width==7)
                {

                    for (y=y_start; y<=y_end; y++)
                    {
                        SetPixel(x_start+2,y,hsv);
                        SetPixel(x_start+3,y,hsv);
                        SetPixel(x_start+4,y,hsv);
                        SetPixel(x_start+5,y,hsv);
                    }

                    int y_end2 = y_start+height/2;
                    for (y=y_start; y<=y_end2; y++)
                    {
                        SetPixel(x_start,y,hsv);
                        SetPixel(x_start+1,y,hsv);
                    }

                    // black out around accidental key
                    hsv.value=0.0; // black
                    for (y=y_end2; y<=y_end; y++)
                    {
                        SetPixel(x_start+2,y,hsv);
                    }
                    for(x=x_start; x<=x_start+2; x++)
                    {
                        SetPixel(x,y_end2,hsv);
                    }
                }
                else
                {
                    for(x=x_start+1; x<=x_end; x++)
                    {
                        for (y=y_start; y<=y_end; y++)
                        {
                            SetPixel(x,y,hsv);
                        }
                    }
                    int y_end2 = y_start+height/2;
                    for (y=y_start; y<=y_end2; y++)
                    {
                        SetPixel(x_start,y,hsv);
                    }
                }

            }
        }
        break;

        default:
        {
            /*
            for(x=1; x<BufferWi; x++)
                for(y=0; y<BufferHt; y++)
                {
                    hsv.hue=0.0;
                    hsv.saturation=1.0;
                    hsv.value=1.0;
                    SetPixel(x,y,hsv);
                }
                */
        }
        break;
        } // switch (keys_mod)
    } //  for (keys=1; keys<=14; keys++)
#endif // 0
