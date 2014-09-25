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
#include <list>
#include <vector>
#include <wx/colour.h>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/image.h>
#include <wx/textfile.h>

#include "../include/globals.h"

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

typedef enum
{
    //effect: 0=open, 1=close, 2=open then close, 3=close then open
    E_CURTAIN_OPEN =0,
    E_CURTAIN_CLOSE,
    E_CURTAIN_OPEN_CLOSE,
    E_CURTAIN_CLOSE_OPEN
} CURTAIN_EFFECT_e;

typedef std::vector<wxColour> wxColourVector;
typedef std::vector<wxImage::HSVValue> hsvVector;
typedef std::vector<wxPoint> wxPointVector;
typedef wxImage::HSVValue HSVValue;

#define rgb_MAX_BALLS 20

// eventually this will go in some header..
// the idea is to define this (currently) for the MS compiler
// and to switch its value based on creating vs using the dll
// NCCDLLIMPL is set by the project creating the dll
#ifdef _MSC_VER
#ifdef NCCDLLIMPL
#define NCCDLLEXPORT __declspec(dllexport)
#else
#define NCCDLLEXPORT __declspec(dllimport)
#endif
#else
#define NCCDLLEXPORT
#endif

class NCCDLLEXPORT RgbEffects
{
public:
//BL: it turns out this actually works... the pix buf class is the only consumer of this...
//protected:
//    friend class PixelBufferClass;

    RgbEffects();
    ~RgbEffects();
    void InitBuffer(int newBufferHt, int newBufferWi);
    void Clear(const wxColour& bgColor);
    void SetPalette(wxColourVector& newcolors);
    void SetState(int period, int NewSpeed, bool ResetState);
    void GetPixel(int x, int y, wxColour &color);

    void SetFadeTimes(float fadeIn, float fadeOut );
    void SetEffectDuration(int startMsec, int endMsec, int nextMsec);
    void SetFitToTime(bool fit);

    void GetFadeSteps( int& fadeInSteps, int& fadeOutSteps);
    void GetEffectPeriods( int& curEffStartPer, int& nextEffTimePeriod, int& curEffEndPer);  // nobody wants endPer?

#include "Effects.h"

protected:
    // was public, but not used external to this class
    size_t GetColorCount();

    int fadeinsteps, fadeoutsteps;
    int curEffStartPer;    /**< Start 50ms period of current effect. */
    int curEffEndPer;      /**<  */
    int nextEffTimePeriod; /**<  */

    class RgbFireworks
    {
    public:
        //static const float velocity = 2.5;
        static const int maxCycle = 4096;
        static const int maxNewBurstFlakes = 10;
        float _x;
        float _y;
        float _dx;
        float _dy;
        float vel;
        float angle;
        bool _bActive;
        int _cycles;
        wxImage::HSVValue _hsv;

        void Reset(int x, int y, bool active, float velocity, wxImage::HSVValue hsv)
        {
            _x       = x;
            _y       = y;
            vel      = (rand()-RAND_MAX/2)*velocity/(RAND_MAX/2);
            angle    = 2*M_PI*rand()/RAND_MAX;
            _dx      = vel*cos(angle);
            _dy      = vel*sin(angle);
            _bActive = active;
            _cycles  = 0;
            _hsv     = hsv;
        }
    protected:
    private:
    };

    class RgbBalls
    {
    public:
        float _x;
        float _y;
        float _dx;
        float _dy;
        float _radius;
        float _t;
        float dir;
        wxImage::HSVValue hsvcolor;

        void Reset(float x, float y, float speed, float angle, float radius, wxImage::HSVValue color)
        {
            _x=x;
            _y=y;
            _dx=speed*cos(angle);
            _dy=speed*sin(angle);
            _radius = radius;
            hsvcolor = color;
            _t=(float)M_PI/6.0;
            dir =1.0f;

        }
        void updatePositionArc(int x,int y, int r)
        {
            _x=x+r*cos(_t);
            _y=y+r*sin(_t);
            _t+=dir* (M_PI/9.0);
            dir *= _t < M_PI/6.0 || _t > (2*M_PI)/3?-1.0:1.0;
        }
        void updatePosition(float incr, int width, int height)
        {
            _x+=_dx*incr;
            _x = _x>width?0:_x;
            _x = _x<0?width:_x;
            _y+=_dy*incr;
            _y = _y>height?0:_y;
            _y = _y<0?height:_y;
        }

        void Bounce(int width, int height)
        {
            if (_x-_radius<=0)
            {
                _dx=fabs(_dx);
                if (_dx < 0.2f) _dx=0.2f;
            }
            if (_x+_radius>=width)
            {
                _dx=-fabs(_dx);
                if (_dx > -0.2f) _dx=-0.2f;
            }
            if (_y-_radius<=0)
            {
                _dy=fabs(_dy);
                if (_dy < 0.2f) _dy=0.2f;
            }
            if (_y+_radius>=height)
            {
                _dy=-fabs(_dy);
                if (_dy > -0.2f) _dy=-0.2f;
            }
        }

    };

    class MetaBall : public RgbBalls
    {
    public:
        float Equation(float x, float y)
        {
//            if(x==_x || y==_y) return 1; //this is incorrect
            if((x==_x) && (y==_y)) return 1; //only return 1 if *both* coordinates match; else gives extraneous horiz or vert lines -DJ
            return (_radius/(sqrt(pow(x-_x,2)+pow(y-_y,2))));
        }
    };

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
        wxColor on, off; //first visible pixel color (from bottom left); used for on/off redraw (scrolling fx)
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
    std::hash_map</*wxColor*/ wxUint32, wxColor> ColorMap; //can't use wxColor as key, so use bare RGB value instead; OTOH SetPixel() wants a wxColor, so use it as value type
//    std::vector</*wxColor*/ WXCOLORREF> PrevRender; //keep persistent pixels to reduce expensive redraws

    void Piano_flush_cues(void);
    void Piano_flush_map(void);
    void Piano_flush_shapes(void);
    void Piano_load_shapes(const wxString& filename);
    void Piano_load_sprite_map(const wxString& filename);
    void Piano_load_cues(const wxString& filename);
    void Piano_update_bkg(int Style, wxSize& canvas, int rowh);
    void Piano_map_colors(void);
    bool Piano_RenderKey(Sprite* sprite, std::hash_map<wxPoint_, int>& drawn, int style, wxSize& canvas, wxSize& keywh, int placement, bool clip);
    wxColor cached_rgb; //cached mapped pixel color
    wxPoint cached_xy;
//end of piano support stuff -DJ

//cached list of pixels to turn on, indexed by frame (timestamp):
//this is the required output after applying the effect in Nutcracker
//xLights will then remap these pixels thru the custom or built-in model back to xLights channels
    typedef std::vector<std::pair<wxPoint, wxColor>> PixelVector;
    std::vector<PixelVector> PixelsByFrame; //list of pixels and their associated values, indexed by frame#
//remapped Vixen channels for Picture effect: -DJ
    void LoadPixelsFromTextFile(wxFile& debug, const wxString& filename);


    class PaletteClass
    {
    private:
        wxColourVector color;
        hsvVector hsv;
    public:

        void Set(wxColourVector& newcolors)
        {
            color=newcolors;
            hsv.clear();
            wxImage::RGBValue newrgb;
            wxImage::HSVValue newhsv;
            for(size_t i=0; i<newcolors.size(); i++)
            {
                newrgb.red=newcolors[i].Red();
                newrgb.green=newcolors[i].Green();
                newrgb.blue=newcolors[i].Blue();
                newhsv=wxImage::RGBtoHSV(newrgb);
                hsv.push_back(newhsv);
            }
        }

        size_t Size()
        {
            size_t colorcnt=color.size();
            if (colorcnt < 1) colorcnt=1;
            return colorcnt;
        }

        void GetColor(size_t idx, wxColour& c)
        {
            if (idx >= color.size())
            {
                c.Set(255,255,255);
            }
            else
            {
                c=color[idx];
            }
        }

        void GetHSV(size_t idx, wxImage::HSVValue& c)
        {
            if (hsv.size() == 0)
            {
                // white
                c.hue=0.0;
                c.saturation=0.0;
                c.value=1.0;
            }
            else
            {
                c=hsv[idx % hsv.size()];
            }
        }
    };








    void SetPixel(int x, int y, const wxColour &color);
    void SetPixel(int x, int y, const wxImage::HSVValue& hsv);
    void CopyPixel(int srcx, int srcy, int destx, int desty); //-DJ
    void SetTempPixel(int x, int y, const wxColour &color);
    void GetTempPixel(int x, int y, wxColour &color);
    wxUint32 GetTempPixelRGB(int x, int y);
    void SetFireBuffer(int x, int y, int PaletteIdx);
    int GetFireBuffer(int x, int y);
    void SetWaveBuffer1(int x, int y, int value);
    int GetWaveBuffer1(int x, int y);
    void SetWaveBuffer2(int x, int y, int value);
    int GetWaveBuffer2(int x, int y);

    void DrawCircle(int xc, int yc, int r, const wxImage::HSVValue& hsv);
    void CirclePlot(int xc, int xy, int x, int y, const wxImage::HSVValue& hsv);

    void DrawCircleClipped(int xc, int yc, int r, const wxImage::HSVValue& hsv);
    void CirclePlotClipped(int xc, int xy, int x, int y, const wxImage::HSVValue& hsv);

    double rand01();
    wxByte ChannelBlend(wxByte c1, wxByte c2, double ratio);
    void Get2ColorBlend(int coloridx1, int coloridx2, double ratio, wxColour &color);
    void GetMultiColorBlend(double n, bool circular, wxColour &color);
    void SetRangeColor(const wxImage::HSVValue& hsv1, const wxImage::HSVValue& hsv2, wxImage::HSVValue& newhsv);
    double RandomRange(double num1, double num2);
    void Color2HSV(const wxColour& color, wxImage::HSVValue& hsv);
    wxPoint SnowstormVector(int idx);
    void SnowstormAdvance(SnowstormClass& ssItem);
    void ClearTempBuf();
    void ClearWaveBuffer1();
    void ClearWaveBuffer2();
    int Life_CountNeighbors(int x, int y);
    void RenderTextLine(wxMemoryDC& dc, int idx, int Position, const wxString& Line, int dir, bool center, int Effect, int Countdown, bool WantRender);
    void RenderMeteorsVertical(int ColorScheme, int Count, int Length, int MeteorsEffect, int SwirlIntensity);
    void RenderMeteorsHorizontal(int ColorScheme, int Count, int Length, int MeteorsEffect, int SwirlIntensity);
    void RenderMeteorsImplode(int ColorScheme, int Count, int Length, int SwirlIntensity);
    void RenderIcicleDrip(int ColorScheme, int Count, int Length, int MeteorsEffect, int SwirlIntensity);
    HSVValue Get2ColorAdditive(HSVValue& hsv1, HSVValue& hsv2);
    void RenderMeteorsExplode(int ColorScheme, int Count, int Length, int SwirlIntensity);
    void RenderMetaBalls(int numBalls);
    void DrawCurtain(bool LeftEdge, int xlimit, const wxArrayInt &SwagArray);
    void DrawCurtainVertical(bool LeftEdge, int xlimit, const wxArrayInt &SwagArray);
    void mouth(int Phoneme,int BufferHt, int BufferWt);
    void coroface(int Phoneme, const wxString& x_y, const wxString& Outline_x_y, const wxString& Eyes_x_y);
    void drawline1(int Phoneme,int x1,int x2,int y1, int y2);
    void drawoutline(int Phoneme,int BufferHt,int BufferWi);

    void facesCircle(int Phoneme, int xc,int yc,double radius,int start_degrees, int end_degrees);
    void drawline3 (int Phoneme, int x1,int x2,int y6,int y7);

    int BufferHt,BufferWi;  // size of the buffer
    int ChaseDirection; // 0 = R-L, 1=L-R
    int DiagLen;  // length of the diagonal
    int NumPixels;
    bool InhibitClear; //allow canvas to be persistent for piano fx -DJ
    wxColourVector pixels; // this is the calculation buffer
    wxColourVector tempbuf;
    wxColourVector FirePalette;
    std::vector<int> FireBuffer;
    std::vector<int> WaveBuffer0;
    std::vector<int> WaveBuffer1;
    std::vector<int> WaveBuffer2;
    MeteorList meteors;
    MeteorRadialList meteorsRadial;
    SnowstormList SnowstormItems;
    PaletteClass palette;

    wxImage image;
    int imageCount;
    int imageIndex;

    wxString PictureName;
    int LastSnowflakeCount;
    int LastSnowflakeType;
    int LastSnowstormCount;
    int LastLifeCount;
    int LastLifeType;
    int LastCurtainDir;
    int LastCurtainLimit;
    long state;
    long LastLifeState;
    int speed;
    int lastperiod, curPeriod;
    RgbFireworks fireworkBursts[20000];
    RgbBalls balls[rgb_MAX_BALLS];
    int maxmovieframes;
    long timer_countdown[4]; // was  long timer_countdown[1];
    bool fitToTime;

    double GetEffectTimeIntervalPosition();
    MetaBall metaballs[10];

    size_t GetNodeCount();
    //int face[52][52];

public:
    double GetEffectPeriodPosition(); //made public -DJ

private:
    void RenderRadial(int start_x,int start_y,int radius,int colorCnt, int number, bool radial_3D);
    void RenderCirclesUpdate(int number, RgbBalls* effObjs);
};

#endif // XLIGHTS_RGBEFFECTS_H
