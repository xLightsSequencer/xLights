/***************************************************************
 * Name:      RgbEffects.h
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

#ifndef XLIGHTS_RGBEFFECTS_H
#define XLIGHTS_RGBEFFECTS_H

#include <stdint.h>
#include <map>
#include <list>
#include <vector>
#include <wx/colour.h>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/graphics.h>
#include <wx/image.h>
#include <wx/textfile.h>

#include "../include/globals.h"

#include "RenderBuffer.h"

#include "Color.h"

//added hash_map, queue, vector: -DJ
#ifdef _MSC_VER
#include <hash_map>
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#else
#include <unordered_map>
#define hash_map  unordered_map //c++ 2011
#endif
#include <queue> //priority_queue
#include <deque>
#include <vector>
#define wxPoint_  long //std::pair<int, int> //kludge: wxPoint doesn't work with std::hash_map, so use equiv sttr

//kludge to access protected members: -DJ
template <typename Tdata, typename Tcontainer = std::vector<Tdata>, typename Tsorter = Tdata>
class MyQueue: public std::priority_queue<Tdata, Tcontainer, Tsorter>
{
    friend class RgbEffects;
public:
    inline void clear()
    {
        std::priority_queue<Tdata, Tcontainer, Tsorter>::c.clear();    //add missing member
    }
};
#define priority_queue  MyQueue //wedge in friend


class RgbEffects : public RenderBuffer {
public:

    RgbEffects();
    ~RgbEffects();

    virtual void InitBuffer(int newBufferHt, int newBufferWi);

#include "Effects.h"


protected:


    // for meteor effect
    class MeteorHasExpiredX;
    class MeteorHasExpiredY;
    class MeteorHasExpiredImplode;
    class MeteorHasExpiredExplode;
    class IcicleHasExpired; //reuse meteor code for icicle drip effect -DJ

    class MeteorClass
    {
    public:

        int x,y;
        wxImage::HSVValue hsv;
        int h; //variable length; only used for icicle drip -DJ
    };

    // for radial meteor effect
    class MeteorRadialClass
    {
    public:

        double x,y,dx,dy;
        int cnt;
        wxImage::HSVValue hsv;
    };

    typedef std::list<MeteorClass> MeteorList;
    typedef std::list<MeteorRadialClass> MeteorRadialList;


    class SnowstormClass
    {
    public:
        wxPointVector points;
        wxImage::HSVValue hsv;
        int idx,ssDecay;
        ~SnowstormClass()
        {
            points.clear();
        }
    };

    typedef std::list<SnowstormClass> SnowstormList;


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
        //	wxString filename; //image file containing sprite
        std::vector<wxPoint> xy; //size 1 => on/off; > 1 => animation
        int ani_state; //display state; bumped after each state change while sprite is active
        //	int repeat; //0 => one-shot, > 0 => loop count, < 0 => loop count with random delay
        wxSize wh; //size in src image; might be scaled up/down when rendered onto canvas
        wxPoint destxy; //where to place it on canvas
        xlColor on, off; //first visible pixel color (from bottom left); used for on/off redraw (scrolling fx)
        int destz; //controls draw order for overlapping sprites
    public:
//copy sprite image to canvas:
//        bool render(int placement, bool clip) { return Piano_RenderKey(this, placement, clip); }
    };

//a Cue is a sprite with state info that changes pver a specific time interval
//Audacity bar, beat, or onset timing marks can be used as cues
//phonemes can also be used as cue for lip/face syncing
//template<int sorter>
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

//cached state info:
//NOTE: caching assumes only one Piano effect will be active
//TODO: move these into RgbEffects.h?
    wxString CachedCueFilename, CachedMapFilename, CachedShapeFilename; //keep track of which files are cached; only reload if changed
    wxImage Shapes;
//    std::vector</*wxColor*/ WXCOLORREF> ShapePalette; //color palette of Shapes
    std::hash_map<std::string, Sprite> AllSprites; //can't use wxString here, so just use std::string
//static /*std::*/vector</*std::*/vector<Sprite*>> ByLayer;
//static vector<Sprite*> VisibleSprites; //only the sprites that appear on canvas
//hash_map<Cue> CueList;
    std::vector<Cue> CuesByStart; //all cues, sorted by start frame
    /*std::*/priority_queue<Cue*, std::vector<Cue*>, Cue /*SortByStop*/> ActiveCues;
//    std::deque<Cue*> ActiveCues; //sorted by stop frame
    std::hash_map</*wxColor*/ wxUint32, xlColor> ColorMap; //can't use wxColor as key, so use bare RGB value instead; OTOH SetPixel() wants a xlColor, so use it as value type
//    std::vector</*wxColor*/ WXCOLORREF> PrevRender; //keep persistent pixels to reduce expensive redraws

    void Piano_flush_cues(void);
    void Piano_flush_map(void);
    void Piano_flush_shapes(void);
    void Piano_load_shapes(const wxString& filename);
    void Piano_load_sprite_map(const wxString& filename);
    void Piano_load_cues(const wxString& filename);
    void Piano_update_bkg(int Style, wxSize& canvas, int rowh);
    void Piano_map_colors(void);
    bool Piano_RenderKey(Sprite* sprite, std::hash_map<wxPoint_, int>& drawn, int style, wxSize& canvas, wxSize& keywh, const wxString &placement, bool clip);
    xlColor cached_rgb; //cached mapped pixel color
    wxPoint cached_xy;
//end of piano support stuff -DJ

//cached list of pixels to turn on, indexed by frame (timestamp):
//this is the required output after applying the effect in Nutcracker
//xLights will then remap these pixels thru the custom or built-in model back to xLights channels
    typedef std::vector< std::pair<wxPoint, xlColor> > PixelVector;
    std::vector<PixelVector> PixelsByFrame; //list of pixels and their associated values, indexed by frame#
//remapped Vixen channels for Picture effect: -DJ
    void LoadPixelsFromTextFile(wxFile& debug, const wxString& filename);




    wxPoint SnowstormVector(int idx);
    void SnowstormAdvance(SnowstormClass& ssItem);

    void RenderMeteorsVertical(int ColorScheme, int Count, int Length, int MeteorsEffect, int SwirlIntensity, int mspeed);
    void RenderMeteorsHorizontal(int ColorScheme, int Count, int Length, int MeteorsEffect, int SwirlIntensity, int mspeed);
    void RenderMeteorsImplode(int ColorScheme, int Count, int Length, int SwirlIntensity, int mspeed);
    void RenderIcicleDrip(int ColorScheme, int Count, int Length, int MeteorsEffect, int SwirlIntensity, int mspeed);
    void RenderMeteorsExplode(int ColorScheme, int Count, int Length, int SwirlIntensity, int mspeed);

    void DrawCurtain(bool LeftEdge, int xlimit, const wxArrayInt &SwagArray);
    void DrawCurtainVertical(bool LeftEdge, int xlimit, const wxArrayInt &SwagArray);
    void mouth(int Phoneme,int BufferHt, int BufferWt);
    void coroface(int Phoneme, const wxString& x_y, const wxString& Outline_x_y, const wxString& Eyes_x_y);
    void drawline1(int Phoneme,int x1,int x2,int y1, int y2);
    void drawoutline(int Phoneme, bool outline, const wxString &eyes, int BufferHt,int BufferWi);
    double calcAccel(double ratio, double accel);
    double GetStepAngle(int width, int height);

    void facesCircle(int Phoneme, int xc,int yc,double radius,int start_degrees, int end_degrees);
    void drawline3 (int Phoneme, int x1,int x2,int y6,int y7);

    int ChaseDirection; // 0 = R-L, 1=L-R
    int DiagLen;  // length of the diagonal
    int NumPixels;

    std::vector<int> FireBuffer;
    MeteorList meteors;
    MeteorRadialList meteorsRadial;
    SnowstormList SnowstormItems;

    wxImage image;
    int imageCount;
    int imageIndex;
    int frame;
    int maxmovieframes;

    wxString PictureName;
    int LastSnowflakeCount;
    int LastSnowflakeType;
    bool LastSnowflakeAccumulate;
    int LastSnowstormCount;

    int nextBlinkTime;
    int blinkEndTime;

    size_t GetNodeCount();
    //int face[52][52];

public:

private:
    void ProcessPixel(int x_pos, int y_pos, const xlColour &color, bool wrap_x, int width);
};

#endif // XLIGHTS_RGBEFFECTS_H
