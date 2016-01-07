#include "PianoEffect.h"

#include "PianoPanel.h"
#include <wx/filename.h>
#include <wx/tokenzr.h>

#include "../sequencer/SequenceElements.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

#include <queue>
#include <map>

#include "../../include/piano.xpm"

PianoEffect::PianoEffect(int id) : RenderableEffect(id, "Piano", piano, piano, piano, piano, piano)
{
    //ctor
}

PianoEffect::~PianoEffect()
{
    //dtor
}


void PianoEffect::SetSequenceElements(SequenceElements *els) {
    mSequenceElements = els;
    if (panel == nullptr) {
        return;
    }
    wxFileName fn(els->GetFileName());
    
    ((PianoPanel*)panel)->defaultDir = fn.GetPath();
}

wxPanel *PianoEffect::CreatePanel(wxWindow *parent) {
    return new PianoPanel(parent);
}

void PianoEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    RenderPiano(buffer,
                SettingsMap["CHOICE_Piano_Style"],
                wxAtoi(SettingsMap["SLIDER_Piano_NumKeys"]),
                wxAtoi(SettingsMap["SLIDER_Piano_NumRows"]),
                SettingsMap["CHOICE_Piano_Placement"],
                SettingsMap["CHECKBOX_Piano_Clipping"] == "1",
                SettingsMap["TEXTCTRL_Piano_CueFilename"],
                SettingsMap["TEXTCTRL_Piano_MapFilename"],
                SettingsMap["TEXTCTRL_Piano_ShapeFilename"]);
}




#define debug(level, ...)
#define debug_more(level, ...)
#define debug_function(level)
#define divup(num, den)  (((num) + (den) - 1)/(den)) //round up

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

//added sprites + cues for piano and related fx: -DJ
//NOTE: these could be used by other fx, or promoted/generalized to be part of all fx
//convert a time string to frame#:

//a Sprite is a rectangular portion of a larger bitmap
//multiple related logical bitmaps can be loaded with only one physical file
//TODO: subclass image/bitmap into sprites?
class Sprite
{
public:
    wxString name;
    std::vector<wxPoint> xy; //size 1 => on/off; > 1 => animation
    int ani_state; //display state; bumped after each state change while sprite is active
    //	int repeat; //0 => one-shot, > 0 => loop count, < 0 => loop count with random delay
    wxSize wh; //size in src image; might be scaled up/down when rendered onto canvas
    wxPoint destxy; //where to place it on canvas
    xlColor on, off; //first visible pixel color (from bottom left); used for on/off redraw (scrolling fx)
    int destz; //controls draw order for overlapping sprites
public:
    //copy sprite image to canvas:
    //bool render(int placement, bool clip) { return Piano_RenderKey(this, placement, clip); }
};

//a Cue is a sprite with state info that changes pver a specific time interval
//Audacity bar, beat, or onset timing marks can be used as cues
//phonemes can also be used as cue for lip/face syncing
class Cue
{
public:
    //    int frames[2];
    int start_frame; //int& start_frame = frames[0];
    int stop_frame; //int& stop_frame = frames[1];
    //	int& sort_frame = stop_frame; //sorter? stop_frame: start_frame; //frames[sorter];
    //	int state; //state bumped during each active frame if repeat != 0;
    Sprite* sprite;
public:
    //        Cue(void) {} //need to provide default ctor since nop-default also provided
    //        Cue(int frame): start_frame(frame) {} //ctor used with lower_bound()
public: //comparison operators for sorting
    inline bool operator() (const Cue& lhs, const Cue& rhs) const
    {
        return lhs.stop_frame < rhs.stop_frame;    //used by priority_queue
    }
    inline bool operator() (Cue* lhs, Cue* rhs) const
    {
        return lhs->stop_frame < rhs->stop_frame;    //used by priority_queue?
    }
    static bool SortByStart(const Cue& lhs, const Cue& rhs)
    {
        return lhs.start_frame < rhs.start_frame;    //used by sort()
    }
    static bool SortByStop(const Cue& lhs, const Cue& rhs)
    {
        return lhs.stop_frame < rhs.stop_frame;    //used by sort()
    }
    //    static bool ByStart(Cue* lhs, Cue* rhs) { return lhs->start_frame < rhs->start_frame; } //used by sort()
public:
    static int Time2Frame(const wxString& timestr, int round)
    {
        double timeval;
        int msec;
        
        msec = timestr.ToDouble(&timeval)? (int)(timeval * 1000): 0;
        return (round > 0)? (msec + 49)/ 50: (round < 0)? msec / 50: (msec + 25)/ 50; //round up/down/closest
    }
};


//kludge to access protected members: -DJ
template <typename Tdata, typename Tcontainer = std::vector<Tdata>, typename Tsorter = Tdata>
class MyQueue: public std::priority_queue<Tdata, Tcontainer, Tsorter>
{
    friend class PianoRenderCache;
    friend class PianoEffect;
public:
    inline void clear()
    {
        std::priority_queue<Tdata, Tcontainer, Tsorter>::c.clear();    //add missing member
    }
};


class PianoRenderCache : public EffectRenderCache {
public:
    PianoRenderCache() {};
    virtual ~PianoRenderCache() {};

    
    void Piano_flush_cues(void) {
        debug_function(10); //("flush_cues");
        CuesByStart.resize(0);
        ActiveCues.clear();
        CachedCueFilename.clear();
    }
    void Piano_flush_map(void)
    {
        debug_function(10); //Debug debug("flush_map");
        Piano_flush_cues(); //flush dependent data also
        AllSprites.clear();
        CachedMapFilename.clear();
    }
    void Piano_flush_shapes(void)
    {
        debug_function(10); //Debug debug("flush_shapes");
        Piano_flush_map(); //flush dependent data also
        //    if (Shapes.GetWidth() || Shapes.GetHeight())
        if (Shapes.IsOk()) Shapes.Clear(); //CAUTION: don't clear unless non-empty (causes access violation)
        //    ShapePalette.resize(0);
        ColorMap.clear();
        CachedShapeFilename.clear();
    }
    void Piano_load_shapes(RenderBuffer &buffer, const wxString& filename);
    void Piano_load_sprite_map(const wxString& filename);
    void Piano_load_cues(const wxString& filename);
    void Piano_update_bkg(RenderBuffer &buffer, int Style, wxSize& canvas, int rowh);
    bool Piano_RenderKey(RenderBuffer &buffer, Sprite* sprite, std::hash_map<wxPoint_, int>& drawn,
                         int style, wxSize& canvas, wxSize& keywh, const wxString &placement, bool clip);

    
    std::vector<Cue> CuesByStart; //all cues, sorted by start frame
    MyQueue<Cue*, std::vector<Cue*>, Cue /*SortByStop*/> ActiveCues;
    wxString CachedCueFilename, CachedMapFilename, CachedShapeFilename; //keep track of which files are cached; only reload if changed
    std::hash_map<std::string, Sprite> AllSprites; //can't use wxString here, so just use std::string
    wxImage Shapes;
    std::hash_map</*wxColor*/ wxUint32, xlColor> ColorMap;
    xlColor cached_rgb; //cached mapped pixel color
    wxPoint cached_xy;
};





//all shapes are loaded from same image file to reduce file I/O and caching
//thiss also allows animated images to be self-contained
void PianoRenderCache::Piano_load_shapes(RenderBuffer &buffer, const wxString& filename)
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
    
    if (buffer.GetColorCount() < 2) return; //use colors from shapes file if no user-selected colors
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
                buffer.palette.GetColor(ColorMap.size() % buffer.GetColorCount(), mapped); //assign user-selected colors to shape palette sequentially, loop if run out of colors
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
void PianoRenderCache::Piano_load_sprite_map(const wxString& filename) //, int BufferWi, int BufferHt) //, bool clip)
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
void PianoRenderCache::Piano_load_cues(const wxString& filename)
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



//update previous bkg:
//if it's a scrolling piano, scroll previous notes up one row
//for eqbars, shorten them
void PianoRenderCache::Piano_update_bkg(RenderBuffer &buffer, int Style, wxSize& canvas, int rowh)
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
                    buffer.SetPixel(x, y, c); //clear all (background canvas is persistent while piano effect is active)
            return;
        case PIANO_STYLE_SCROLLING: //scroll up one row
            debug_more(5, ", scroll %d x %d up by %d", canvas.x, canvas.y, rowh);
            for (int x = 0; x < canvas.x; ++x)
                for (int y = 0; y < canvas.y; ++y)
                    if (y < canvas.y - rowh) {
                        debug_more(30, ", (%d,%d)->(%d,%d)", x, canvas.y - y - rowh - 1, x, canvas.y - y - 1);
                        buffer.CopyPixel(x, canvas.y - y - rowh - 1, x, canvas.y - y - 1);
                    } else {
                        debug_more(30, ", (%d,%d)<-0", x, canvas.y - y - 1);
                        buffer.SetPixel(x, canvas.y - y - 1, c);  //clear bottom row, scroll others
                    }
            return;
        case PIANO_STYLE_EQBARS: //scroll down one row (decaying bars)
            debug_more(5, ", scroll %d x %d down by %d", canvas.x, canvas.y, rowh);
            //            c.Set(255, 255, 255); //debug
            for (int x = 0; x < canvas.x; ++x)
                for (int y = 0; y < canvas.y; ++y)
                    if (y < canvas.y - rowh) {
                        debug_more(30, ", (%d,%d)->(%d,%d)", x, y + rowh, x, y);
                        buffer.CopyPixel(x, y + rowh, x, y);
                    } else {
                        debug_more(30, ", (%d,%d)<-0", x, y);
                        buffer.SetPixel(x, y, c); //clear top row, scroll other rows
                    }
            return;
        case PIANO_STYLE_ICICLES: //scroll down one pixel (drip)
            debug_more(5, ", scroll %d x %d", canvas.x, canvas.y);
            for (int x = 0; x < canvas.x; ++x)
                for (int y = 0; y < canvas.y; ++y)
                    if (y < canvas.y - 1) {
                        debug_more(30, ", (%d,%d)->(%d,%d)", x, y + 1, x, y);
                        buffer.CopyPixel(x, y + 1, x, y);
                    }
            //      else { debug_more(30, ", (%d,%d)<-0", x, y); buffer.SetPixel(x, y, c); } //clear top pixel, scroll other pixels
            return;
        default:
            debug_more(5, ", no scrolling");
    }
}



//render piano fx during sequence:
void PianoEffect::RenderPiano(RenderBuffer &buffer, const std::string & StyleStr, int NumKeys, int NumRows, const std::string & Placement,
                             bool Clipping, const std::string& CueFilename, const std::string& MapFilename, const std::string& ShapeFilename)
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
    int curframe = buffer.curPeriod - buffer.curEffStartPer; //units of 50 msec
    int state = curframe;
    debug(2, "RenderPiano[%d -> %d] speed %d, frame %d, %f sec, empty? %d", cached_state, state, speed, curframe, (float)curframe*50/1000, CuesByStart.empty());
    if (Style != PIANO_STYLE_ANIMAGE) buffer.InhibitClear = true; //allow canvas to be persistent so we don't need to keep redrawing it
    
    NumKeys = buffer.BufferWi/3; //TODO: make this look better for other values
    
    //get/check/fix up rendering params:
    if (NumKeys < 2) NumKeys = 7; //default to 1 octave
    if (NumKeys > buffer.BufferWi) NumKeys = buffer.BufferWi; //give each key at least 1 pixel in order to be visible
    if (!WantHistory(Style)) NumRows = 1;
    else if (NumRows < 2) NumRows = NumKeys * buffer.BufferHt / buffer.BufferWi; //default square keys
    if (NumRows > buffer.BufferHt) NumRows = buffer.BufferHt; //each row needs at least 1 pixel in order to be visible
    if (Style == PIANO_STYLE_ANIMAGE) NumKeys = NumRows = 1; //use entire canvas
    int adjustw = Clipping? divup(NumKeys, 7): NumKeys/7; //round up vs. down
    if (adjustw < 1) adjustw = 1; //kludge: compiler bug; gcc is generating a imul instr here! gives 0 for NumKeys == 10
    wxSize keywh(buffer.BufferWi / NumKeys, buffer.BufferHt / NumRows), /*BufferWH_full(BufferWi, BufferHt),*/ BufferWH_octave(7 * adjustw * keywh.x, NumRows * keywh.y); //wrap on octave boundaries only (so notes don't move); NOTE: only count white keys (black ones overlap), so octave width is actually 7 keys, not 12
    wxSize keywh_1row(buffer.BufferWi / NumKeys, buffer.BufferHt / 5 + rand() % (buffer.BufferHt * 4/5 + 1)); //kludge: simulate varying amplitudes for eq bars
    if (Style == PIANO_STYLE_ANIMAGE) BufferWH_octave.x = keywh.x; //use entire canvas
    if (Style == PIANO_STYLE_EQBARS) BufferWH_octave.y = buffer.BufferHt; //use entire height
    //    int yscroll = 0;
    debug_more(5, ": style %d, #keys %d, keyw %d/%d(%d), #rows %d, rowh %d/%d(%d)", Style, NumKeys, keywh.x, BufferWH_octave.x, BufferWi, NumRows, keywh.y, BufferWH_octave.y, buffer.BufferHt);
    
    //NOTE: overlap needs more work
    //if 2 keys wrap to the same canvas location and one turns off, should it be redrawn?
    //if any keys are on, you want to see them; OTOH if anything changes, you also want to see that
    //since the purpose of thiss is animation, we'll take the latter option for now
    std::hash_map<wxPoint_, int> drawn; //don't draw overlapping regions more than once
    
    PianoRenderCache *cache = (PianoRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new PianoRenderCache();
        buffer.infoCache[id] = cache;
    }
    
    //initialize cached data:
    //NOTE: must come thru here if fx params changed
    if ((state < cached_state) || cache->CuesByStart.empty()) //start of playback or rewind/backtrack, or keep trying to load cues
    {
        cache->Piano_load_shapes(buffer, ShapeFilename);
        cache->Piano_load_sprite_map(MapFilename); //, Clipping);
        cache->Piano_load_cues(CueFilename);
        //		history.clear();
        //		resolve_visible(Buffer_Wi, Buffer_Ht, Clipping);
        //        Piano_map_colors();
        cue_cursor = cache->CuesByStart.begin(); //rewind
        //        ActiveCues = std::priority_queue<Cue*, std::vector<Cue*>, Cue /*SortByStop*/>();
        cache->ActiveCues.clear();
        cache->Piano_update_bkg(buffer, -1, BufferWH_octave, keywh.y); //initialize canvas
        for (auto it = cache->AllSprites.begin(); it != cache->AllSprites.end(); ++it) //draw all sprites in their initial state
        {
            it->second.ani_state = 0; //always restart keys in inactive state?
            if (Style != PIANO_STYLE_ANIMAGE)
                cache->Piano_RenderKey(buffer, &it->second, drawn, Style, BufferWH_octave, keywh, Placement, Clipping);
        }
        prev_update = 0;
        cache->cached_xy = wxPoint(-1, -1);
    }
    cached_state = state; //detect rewind
    if (cache->CuesByStart.empty()) return; //nothing to draw
    
    //    if (ShowHistory(Style))
    //    {
    //        history.push_back(/*std::*/vector<int>(NumKeys)); //.emplace_back(); //add new row for current frame
    //        if (history.size() > NumRows) history.pop_front(); //drop oldest row
    //    }
    
    if (WantHistory(Style) && (state - prev_update >= 1000/50)) //time to scroll; speed == #times/sec
    {
        cache->Piano_update_bkg(buffer, Style, BufferWH_octave, keywh.y); //scrolling
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
    while (cache->ActiveCues.size() && (cache->ActiveCues.top()->stop_frame <= curframe)) //stopped during previous frame
    {
        debug(7, "prune: '%s' is first of %d stops @%d vs. %d", (const char*)ActiveCues.top()->sprite->name.c_str(), ActiveCues.size(), ActiveCues.top()->stop_frame, curframe);
        cache->ActiveCues.top()->sprite->ani_state = 0; //inactive state
        if (Style != PIANO_STYLE_ANIMAGE)
            cache->Piano_RenderKey(buffer, cache->ActiveCues.top()->sprite, drawn, Style, BufferWH_octave, keywh, Placement, Clipping);
        cache->ActiveCues.pop(); //remove first element (soonest to expire)
        if (cache->ActiveCues.size()) { debug_more(7, ", next is '%s' %d, ", (const char*)cache->ActiveCues.top()->sprite->name.c_str(), cache->ActiveCues.top()->stop_frame); }
        repaint = true;
    }
    
    //add new cues, update sprite state:
    //NOTE: do thiss after pruning so cues with short durations get at least one frame
    //	if (!state) start_cursor = ByStart.begin(); //rewind at start of playback
    //	while ((start_cursor != ByStart.end()) && (start_cursor->start_frame < now + speed)) //start during thiss frame
    //    drawn.clear(); //redraw On sprites over Off sprites only once
    debug(6, "checking %d - %d cues for activation <= frame %d, next is '%s' %d", CuesByStart.size(), cue_cursor - CuesByStart.begin(), curframe, (cue_cursor != CuesByStart.end())? (const char*)cue_cursor->sprite->name.c_str(): "", (cue_cursor != CuesByStart.end())? cue_cursor->start_frame: -1);
    //    for (cue_cursor = /*CuesByStart.begin()*/ /*std::*/ lower_bound(/*cue_cursor*/ CuesByStart.begin(), CuesByStart.end(), Cue(state), Cue::SortByStart); /*(cue_cursor != CuesByStart.end()) && (cue_cursor->start_frame <= state)*/; ++cue_cursor)
    while ((cue_cursor != cache->CuesByStart.end()) && (cue_cursor->start_frame <= curframe)) //TODO: use lower_bound for better performance
    {
        //	    debug.Append("activate check: cur@ %d, end? %d, start_frame %d <= frame %d? %d", cue_cursor - CuesByStart.begin(), cue_cursor == CuesByStart.end(), cue_cursor->start_frame, state, cue_cursor->start_frame <= state).Flush(true);
        //        if ((cue_cursor == CuesByStart.end()) || (cue_cursor->start_frame > state)) break;
        debug(7, "activate: '%s' starts/stops %d/%d, has %d states", (const char*)cue_cursor->sprite->name.c_str(), cue_cursor->start_frame, cue_cursor->stop_frame, cue_cursor->sprite->xy.size());
        cue_cursor->sprite->ani_state = 1; //first active state
        if (Style != PIANO_STYLE_ANIMAGE)
            cache->Piano_RenderKey(buffer, cue_cursor->sprite, drawn, Style, BufferWH_octave, (Style == PIANO_STYLE_EQBARS)? keywh_1row: keywh, Placement, Clipping);
        if (Style != PIANO_STYLE_EQBARS) //eq bars will age out by themselves
            cache->ActiveCues.push(&*cue_cursor);
        else if ((cache->ActiveCues.size() > 1) && (cache->ActiveCues.c[0]->stop_frame > cache->ActiveCues.c[1]->stop_frame)) //paranoid check
        {
            debug(5, "priority_queue broken! [0] %d > [1] %d", ActiveCues.c[0]->stop_frame, ActiveCues.c[1]->stop_frame);
            //        sort(ActiveCues.begin(), ActiveCues.end(), Cue::SortByStop); //kludge: priority_queue not working, so explicitly sort it
            //                std::swap(ActiveCues.c.begin(), ActiveCues.c.begin() + 1);
            cache->ActiveCues.c.erase(cache->ActiveCues.c.begin()); //remove and try again
            for (auto it = cache->ActiveCues.c.begin();; ++it)
                if ((it == cache->ActiveCues.c.end()) || (cue_cursor->stop_frame < (*it)->stop_frame))
                {
                    cache->ActiveCues.c.insert(it, &*cue_cursor);
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
        for (auto it = cache->AllSprites.begin(); it != cache->AllSprites.end(); ++it) //draw all sprites in their initial state
            if (it->second.ani_state) //redraw active sprites
                cache->Piano_RenderKey(buffer, &it->second, drawn, Style, BufferWH_octave, keywh, Placement, Clipping);
}


bool PianoRenderCache::Piano_RenderKey(RenderBuffer &buffer, Sprite* sprite, std::hash_map<wxPoint_, int>& drawn,
                                 int style, wxSize& canvas, wxSize& keywh, const wxString &placement, bool clip)
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
        if ((sprite->destxy.x >= buffer.BufferWi) || (sprite->destxy.y >= buffer.BufferHt) || (sprite->destxy.x + keywh.x < 0) || (sprite->destxy.y + keywh.y < 0)) return false; //outside of visible rect
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
    int xofs = !clip? (buffer.BufferWi % (7 * keywh.x)) / 2: 0; //center keys if not clipped
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
            if (xofs + wrapx < buffer.BufferWi - xofs) buffer.SetPixel(xofs + wrapx, sprite->destxy.y + y, cached_rgb); //no vertical wrap, only horizontal wrap
        }
    //    debug.Flush(true);
    return true;
}

