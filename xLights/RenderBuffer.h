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

#ifndef XLIGHTS_RENDERBUFFER_H
#define XLIGHTS_RENDERBUFFER_H

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

#include "Color.h"

//added hash_map, queue, vector: -DJ
#ifdef _MSC_VER
#define _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS
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

class AudioManager;
class xLightsFrame;

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

class Effect;
class SettingsMap;
class SequenceElements;


class DrawingContext {
public:
    DrawingContext(int BufferWi, int BufferHt, bool allowShared, bool alpha);
    virtual ~DrawingContext();

    
    void ResetSize(int BufferWi, int BufferHt);
    virtual void Clear();
    virtual wxImage *FlushAndGetImage();
    virtual bool AllowAlphaChannel() { return true;};
protected:
    wxImage *image;
    wxBitmap *bitmap;
    wxBitmap nullBitmap;
    wxMemoryDC *dc;
    wxGraphicsContext *gc;
};

class PathDrawingContext : public DrawingContext {
public:
    PathDrawingContext(int BufferWi, int BufferHt, bool allowShared);
    virtual ~PathDrawingContext();
    
    virtual void Clear() override;
    
    void SetPen(wxPen& pen);
    
    wxGraphicsPath CreatePath();
    void StrokePath(wxGraphicsPath& path);
private:
};

class TextDrawingContext : public DrawingContext {
public:
    TextDrawingContext(int BufferWi, int BufferHt, bool allowShared);
    virtual ~TextDrawingContext();

    virtual void Clear() override;
    virtual bool AllowAlphaChannel() override;

    void SetPen(wxPen& pen);
    
    void SetFont(wxFontInfo &font, const xlColor &color);
    void DrawText(const wxString &msg, int x, int y, double rotation);
    void DrawText(const wxString &msg, int x, int y);
    void GetTextExtent(const wxString &msg, double *width, double *height);
    
private:
};

class PaletteClass
{
private:
    xlColorVector color;
    hsvVector hsv;
public:

    void Set(xlColorVector& newcolors)
    {
        color=newcolors;
        hsv.clear();
        for(size_t i=0; i<newcolors.size(); i++)
        {
            hsv.push_back(newcolors[i].asHSV());
        }
    }

    size_t Size()
    {
        size_t colorcnt=color.size();
        if (colorcnt < 1) colorcnt=1;
        return colorcnt;
    }

    void GetColor(size_t idx, xlColor& c)
    {
        if (idx >= color.size())
        {
            c.Set(255, 255, 255);
        }
        else
        {
            c=color[idx];
        }
    }
    
    void GetHSV(size_t idx, HSVValue& c)
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

class /*NCCDLLEXPORT*/ EffectRenderCache {
public:
	EffectRenderCache();
	virtual ~EffectRenderCache();
};
 
class NCCDLLEXPORT RenderBuffer {
public:
    RenderBuffer(xLightsFrame *frame, bool onlyOnMain);
    ~RenderBuffer();
    void InitBuffer(int newBufferHt, int newBufferWi);
	AudioManager* GetMedia();

    void Clear(const xlColor& bgColor);
    void SetPalette(xlColorVector& newcolors);
    size_t GetColorCount();
	void SetAllowAlphaChannel(bool a);

    void SetState(int period, bool reset, const std::string& model_name);

    void SetEffectDuration(int startMsec, int endMsec);
    void GetEffectPeriods( int& curEffStartPer, int& curEffEndPer);  // nobody wants endPer?
    void SetFrameTimeInMs(int i);

    void GetPixel(int x, int y, xlColor &color);
    void SetPixel(int x, int y, const xlColor &color, bool wrap = false);
    void SetPixel(int x, int y, const HSVValue& hsv, bool wrap = false);
    void CopyPixel(int srcx, int srcy, int destx, int desty);
    void ProcessPixel(int x, int y, const xlColor &color, bool wrap_x, int width);

    void ClearTempBuf();
    const xlColor &GetTempPixelRGB(int x, int y);
    void SetTempPixel(int x, int y, const xlColor &color, int alpha);
    void SetTempPixel(int x, int y, const xlColor &color);
    void GetTempPixel(int x, int y, xlColor &color);
    const xlColor &GetTempPixel(int x, int y);

    void DrawHLine(int y, int xstart, int xend, const xlColor& color, bool wrap = false);
    void DrawVLine(int x, int ystart, int yend, const xlColor& color, bool wrap = false);
    void DrawBox(int x1, int y1, int x2, int y2, const xlColor& color, bool wrap = false);
    void DrawFadingCircle(int x0, int y0, int radius, const xlColor& rgb, bool wrap = false);
    void DrawCircle(int xc, int yc, int r, const xlColor& color, bool filled = false, bool wrap = false);
    void DrawLine( const int x1_, const int y1_, const int x2_, const int y2_, const xlColor& color );
    void DrawThickLine( const int x1_, const int y1_, const int x2_, const int y2_, const xlColor& color, bool direction );

    //aproximation of sin/cos, but much faster
    float sin(float rad) const;
    float cos(float rad) const;
    
    double rand01();
    double calcAccel(double ratio, double accel);
    double GetStepAngle(int width, int height);

    wxByte ChannelBlend(wxByte c1, wxByte c2, double ratio);
    void Get2ColorBlend(int coloridx1, int coloridx2, double ratio, xlColor &color);
    void Get2ColorAlphaBlend(const xlColor& c1, const xlColor& c2, double ratio, xlColor &color);
    void GetMultiColorBlend(double n, bool circular, xlColor &color);
    void SetRangeColor(const HSVValue& hsv1, const HSVValue& hsv2, HSVValue& newhsv);
    double RandomRange(double num1, double num2);
    void Color2HSV(const xlColor& color, HSVValue& hsv);

    HSVValue Get2ColorAdditive(HSVValue& hsv1, HSVValue& hsv2);
    double GetEffectTimeIntervalPosition();
    double GetEffectTimeIntervalPosition(float cycles);



    void CopyPixelsToDisplayListX(Effect *eff, int y, int sx, int ex, int inc = 1);
    // must hold the lock and be sized appropriately
    void SetDisplayListHRect(Effect *eff, int startIdx, double x1, double y1, double x2, double y2,
                             const xlColor &cx1, const xlColor &cx2);
    void SetDisplayListVRect(Effect *eff, int startIdx, double x1, double y1, double x2, double y2,
                             const xlColor &cy1, const xlColor &cy2);
    void SetDisplayListRect(Effect *eff, int startIdx, double x1, double y1, double x2, double y2,
                            const xlColor &cx1y1, const xlColor &cx1y2,
                            const xlColor &cx2y1, const xlColor &cx2y2);
    int Normalise(int val, int min, int max);

    int BufferHt,BufferWi;  // size of the buffer
    
    xlColorVector pixels; // this is the calculation buffer
    xlColorVector tempbuf;
    PaletteClass palette;

    xLightsFrame *frame;
    std::string cur_model; //model currently in effect

    int curPeriod;
    int curEffStartPer;    /**< Start period of current effect. */
    int curEffEndPer;      /**<  */
    int frameTimeInMs;

    int fadeinsteps;
    int fadeoutsteps;

    PathDrawingContext *pathDrawingContext;
    TextDrawingContext *textDrawingContext;

    bool needToInit;
    bool allowAlpha;
    bool InhibitClear;

    /* Places to store and data that is needed from one frame to another */
    std::map<int, EffectRenderCache*> infoCache;
    int tempInt;
    int tempInt2;
    
private:
    bool onlyOnMain;
};


#endif // XLIGHTS_RENDERBUFFER_H
