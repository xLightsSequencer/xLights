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
#include <atomic>
#include <wx/colour.h>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/graphics.h>
#include <wx/image.h>
#include <wx/textfile.h>

#include "../include/globals.h"

#include "Color.h"
#include "ColorCurve.h"
#include "models/Node.h"

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
protected:
    DrawingContext(int BufferWi, int BufferHt, bool allowShared, bool alpha);
    virtual ~DrawingContext();

public:
    static void Initialize(wxWindow *parent);
    static void CleanUp();

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

    static PathDrawingContext* GetContext();
    static void ReleaseContext(PathDrawingContext* pdc);
    
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
    
    static TextDrawingContext* GetContext();
    static void ReleaseContext(TextDrawingContext* pdc);

    virtual void Clear() override;
    virtual bool AllowAlphaChannel() override;

    void SetPen(wxPen& pen);

    void SetFont(wxFontInfo &font, const xlColor &color);
    void DrawText(const wxString &msg, int x, int y, double rotation);
    void DrawText(const wxString &msg, int x, int y);
    void GetTextExtent(const wxString &msg, double *width, double *height);
    void GetTextExtents(const wxString &msg, wxArrayDouble &extents);

private:
    wxString fontName;
    int fontStyle;
    int fontSize;
    xlColor fontColor;
    wxGraphicsFont font;
};

class PaletteClass
{
private:
    xlColorVector color;
    hsvVector hsv;
    xlColorCurveVector cc;
    const ColorCurve nilcc;

public:

    void UpdateForProgress(float progress)
    {
        int i = 0;
        for (const auto& it : cc)
        {
            if (it.IsActive())
            {
                color[i] = xlColor(it.GetValueAt(progress));
                hsv[i] = color[i].asHSV();
            }
            i++;
        }
    }

    void Set(xlColorVector& newcolors, xlColorCurveVector newcc)
    {
        wxASSERT(newcolors.size() == newcc.size());

        cc = newcc;
        color = newcolors;
        hsv.clear();
        for (size_t i = 0; i < newcolors.size(); i++)
        {
            hsv.push_back(newcolors[i].asHSV());
        }
    }

    size_t Size() const
    {
        return std::max(1, (int)color.size());
    }

    const ColorCurve& GetColorCurve(size_t idx) const
    {
        if (idx >= cc.size()) {
            return nilcc;
        }
        return cc[idx];
    }

    const xlColor& GetColor(size_t idx) const {
        if (idx >= color.size()) {
            return xlWHITE;
        }

        return color[idx];
    }

    xlColor GetColor(size_t idx, float progress) const {
        if (idx >= color.size()) {
            return xlWHITE;
        }

        if (cc[idx].IsActive())
        {
            return cc[idx].GetValueAt(progress);
        }
        return color[idx];
    }

    void GetColor(size_t idx, xlColor& c) const
    {
        if (idx >= color.size())
        {
            c.Set(255, 255, 255);
        }
        else
        {
            c = color[idx];
        }
    }

    bool IsSpatial(size_t idx) const
    {
        if (idx >= color.size()) return false;
        return (cc[idx].IsActive() && cc[idx].GetTimeCurve() != TC_TIME);
    }

    xlColor CalcRoundColor(int idx, double round, int type) const
    {
        if (idx < cc.size())
        {
            if (type == TC_CW)
            {
                return cc[idx].GetValueAt(round);
            }
            else
            {
                return cc[idx].GetValueAt(1.0 - round);
            }
        }
        else
        {
            return xlWHITE;
        }
    }

    xlColor CalcRadialColour(int idx, int centrex, int centrey, int maxradius, int x, int y, int type) const
    {
        if (idx < cc.size())
        {
            double len = sqrt((x - centrex) * (x - centrex) + (y - centrey) * (y - centrey));
            if (type == TC_RADIALIN)
                return cc[idx].GetValueAt(1.0 - len / maxradius);
            else
                return cc[idx].GetValueAt(len / maxradius);
        }
        else
        {
            return xlWHITE;
        }
    }

    void GetSpatialColor(size_t idx, float xcentre, float ycentre, float x, float y, float round, float maxradius, xlColor& c) const
    {
        if (idx >= color.size())
        {
            c.Set(255, 255, 255);
        }
        else
        {
            if (idx < cc.size() && cc[idx].IsActive())
            {
                switch (cc[idx].GetTimeCurve())
                {
                case TC_CW:
                    c = CalcRoundColor(idx, round, TC_CW);
                    break;
                case TC_CCW:
                    c = CalcRoundColor(idx, round, TC_CCW);
                    break;
                case TC_RADIALIN:
                    c = CalcRadialColour(idx, xcentre, ycentre, maxradius, x, y, TC_RADIALIN);
                    break;
                case TC_RADIALOUT:
                    c = CalcRadialColour(idx, xcentre, ycentre, maxradius, x, y, TC_RADIALOUT);
                    break;
                default:
                    c = color[idx];
                    break;
                }
            }
            else
            {
                c = color[idx];
            }
        }
    }

    void GetSpatialColor(size_t idx, float x, float y, xlColor& c) const
    {
        if (idx >= color.size())
        {
            c.Set(255, 255, 255);
        }
        else
        {
            if (idx < cc.size() && cc[idx].IsActive())
            {
                switch (cc[idx].GetTimeCurve())
                {
                case TC_RIGHT:
                    c = cc[idx].GetValueAt(x);
                    break;
                case TC_LEFT:
                    c = cc[idx].GetValueAt(1.0 - x);
                    break;
                case TC_UP:
                    c = cc[idx].GetValueAt(y);
                    break;
                case TC_DOWN:
                    c = cc[idx].GetValueAt(1.0 - y);
                    break;
                default:
                    c = color[idx];
                    break;
                }
            }
            else
            {
                c = color[idx];
            }
        }
    }

    void GetColor(size_t idx, xlColor& c, float progress) const
    {
        if (idx >= color.size())
        {
            c.Set(255, 255, 255);
        }
        else
        {
            if (idx < cc.size() && cc[idx].IsActive())
            {
                c = cc[idx].GetValueAt(progress);
            }
            else
            {
                c = color[idx];
            }
        }
    }

    void GetHSV(size_t idx, HSVValue& c) const
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
            c = hsv[idx % hsv.size()];
        }
    }

    void GetHSV(size_t idx, HSVValue& c, float progress) const
    {
        if (hsv.size() == 0)
        {
            // white
            c.hue = 0.0;
            c.saturation = 0.0;
            c.value = 1.0;
        }
        else
        {
            if (idx < cc.size() && cc[idx].IsActive())
            {
                c = xlColor(cc[idx].GetValueAt(progress)).asHSV();
            }
            else
            {
                c = hsv[idx % hsv.size()];
            }
        }
    }
};

class /*NCCDLLEXPORT*/ EffectRenderCache {
public:
	EffectRenderCache();
	virtual ~EffectRenderCache();
};

class /*NCCDLLEXPORT*/ RenderBuffer {
public:
    RenderBuffer(xLightsFrame *frame);
    ~RenderBuffer();
    RenderBuffer(RenderBuffer& buffer);
    void InitBuffer(int newBufferHt, int newBufferWi, int newModelBufferHt, int newModelBufferWi, const std::string& bufferTransform, bool nodeBuffer = false);
    AudioManager* GetMedia() const;
    Model* GetModel() const;
    Model* GetPermissiveModel() const; // gets the model even if it is a submodel/strand
    std::string GetModelName() const;

    void AlphaBlend(const RenderBuffer& src);
    bool IsNodeBuffer() const { return _nodeBuffer; }
    void Clear();
    void SetPalette(xlColorVector& newcolors, xlColorCurveVector& newcc);
    size_t GetColorCount();
    void SetAllowAlphaChannel(bool a);
    bool IsDmxBuffer() const { return dmx_buffer; }

    void SetState(int period, bool reset, const std::string& model_name);

    void SetEffectDuration(int startMsec, int endMsec);
    void GetEffectPeriods(int& curEffStartPer, int& curEffEndPer);  // nobody wants endPer?
    void SetFrameTimeInMs(int i);
    long GetStartTimeMS() const { return curEffStartPer * frameTimeInMs; }
    long GetEndTimeMS() const { return curEffEndPer * frameTimeInMs; }

    const xlColor &GetPixel(int x, int y) const;
    void GetPixel(int x, int y, xlColor &color) const;
    void SetPixel(int x, int y, const xlColor &color, bool wrap = false, bool useAlpha = false, bool dmx_ignore = false);
    void SetPixel(int x, int y, const HSVValue& hsv, bool wrap = false);
    void SetNodePixel(int nodeNum, const xlColor &color);
    void CopyNodeColorsToPixels(std::vector<bool> &done);
    
    void CopyPixel(int srcx, int srcy, int destx, int desty);
    void ProcessPixel(int x, int y, const xlColor &color, bool wrap_x = false, bool wrap_y = false);

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
    void DrawLine(const int x1_, const int y1_, const int x2_, const int y2_, const xlColor& color, bool useAlpha = false);
    void DrawThickLine(const int x1_, const int y1_, const int x2_, const int y2_, const xlColor& color, int thickness, bool useAlpha = false);
    void DrawThickLine(const int x1_, const int y1_, const int x2_, const int y2_, const xlColor& color, bool direction);

    //aproximation of sin/cos, but much faster
    static float sin(float rad);
    static float cos(float rad);

    double calcAccel(double ratio, double accel);

    uint8_t ChannelBlend(uint8_t c1, uint8_t c2, float ratio);
    void Get2ColorBlend(int coloridx1, int coloridx2, float ratio, xlColor &color);
    void Get2ColorBlend(xlColor& color, xlColor color2, float ratio);
    void Get2ColorAlphaBlend(const xlColor& c1, const xlColor& c2, float ratio, xlColor &color);
    void GetMultiColorBlend(float n, bool circular, xlColor &color, int reserveColors = 0);
    void SetRangeColor(const HSVValue& hsv1, const HSVValue& hsv2, HSVValue& newhsv);
    double RandomRange(double num1, double num2);
    void Color2HSV(const xlColor& color, HSVValue& hsv);
    PaletteClass& GetPalette() { return palette; }

    HSVValue Get2ColorAdditive(HSVValue& hsv1, HSVValue& hsv2);
    float GetEffectTimeIntervalPosition();
    float GetEffectTimeIntervalPosition(float cycles);

    PathDrawingContext * GetPathDrawingContext();
    TextDrawingContext * GetTextDrawingContext();

    void CopyPixelsToDisplayListX(Effect *eff, int y, int sx, int ex, int inc = 1);
    // must hold the lock and be sized appropriately
    void SetDisplayListHRect(Effect *eff, int startIdx, float x1, float y1, float x2, float y2,
                             const xlColor &cx1, const xlColor &cx2);
    void SetDisplayListVRect(Effect *eff, int startIdx, float x1, float y1, float x2, float y2,
                             const xlColor &cy1, const xlColor &cy2);
    void SetDisplayListRect(Effect *eff, int startIdx, float x1, float y1, float x2, float y2,
                            const xlColor &cx1y1, const xlColor &cx1y2,
                            const xlColor &cx2y1, const xlColor &cx2y2);

    int BufferHt,BufferWi;  // size of the buffer
    int ModelBufferHt, ModelBufferWi;  // size of the buffer

    xlColorVector pixels; // this is the calculation buffer
    xlColorVector tempbuf;
    PaletteClass palette;
    bool _nodeBuffer;

    xLightsFrame *frame;
    std::string cur_model; //model currently in effect

    int curPeriod;
    int curEffStartPer;    /**< Start period of current effect. */
    int curEffEndPer;      /**<  */
    int frameTimeInMs;
    bool isTransformed;

    int fadeinsteps;
    int fadeoutsteps;

    bool needToInit;
    bool allowAlpha;
    bool dmx_buffer;

    /* Places to store and data that is needed from one frame to another */
    std::map<int, EffectRenderCache*> infoCache;
    int tempInt;
    int tempInt2;

private:
    friend class PixelBufferClass;
    std::vector<NodeBaseClassPtr> Nodes;
    PathDrawingContext *_pathDrawingContext;
    TextDrawingContext *_textDrawingContext;

    void SetPixelDMXModel(int x, int y, const xlColor& color);
};


#endif // XLIGHTS_RENDERBUFFER_H
