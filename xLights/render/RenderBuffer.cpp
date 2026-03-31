
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <algorithm>
#include <cmath>
#ifdef _MSC_VER
	// required so M_PI will be defined by MSC
	#define _USE_MATH_DEFINES
	#include <math.h>
#endif

#include "RenderBuffer.h"
#include "Effect.h"
#include "RenderContext.h"
#include "SequenceElements.h"
#include "SequenceFile.h"
#include "PixelBuffer.h"
#include "UtilFunctions.h"
#include "models/DMX/DmxModel.h"
#include "models/DMX/DmxColorAbility.h"
#include "GPURenderUtils.h"
#include "ui/sequencer/BufferPanel.h"

#include <log.h>
#include "Parallel.h"

// TextDrawingContext pool and implementation moved to render/TextDrawingContext.cpp and ui/wxTextDrawingContext.cpp


EffectRenderCache::EffectRenderCache() {}
EffectRenderCache::~EffectRenderCache() {}
void RenderBuffer::SetAllowAlphaChannel(bool a) { allowAlpha = a; }
void RenderBuffer::SetFrameTimeInMs(int i) { frameTimeInMs = i; }


AudioManager* RenderBuffer::GetMedia() const
{
	if (renderContext == nullptr) {
		return nullptr;
	}
	return renderContext->GetCurrentMediaManager();
}

const Model* RenderBuffer::GetModel() const
{
    return model;
}

const std::string &RenderBuffer::GetModelName() const
{
    return cur_model;
}

const std::string &RenderBuffer::GetXmlHeaderInfo(HEADER_INFO_TYPES node_type) const
{
    if (renderContext != nullptr) {
        return renderContext->GetHeaderInfo(node_type);
    }
    static const std::string empty;
    return empty;
}

SequenceMedia* RenderBuffer::GetSequenceMedia() const
{
    if (renderContext == nullptr) {
        return nullptr;
    }
    return &renderContext->GetSequenceElements().GetSequenceMedia();
}

void RenderBuffer::AlphaBlend(const RenderBuffer& src)
{
    if (src.BufferWi != BufferWi || src.BufferHt != BufferHt) return;

    parallel_for(0, GetPixelCount(), [&src, this](int idx) {
        const auto &pnew = src.pixels[idx];
        auto &pold = pixels[idx];
        if (pnew.alpha == 255 || pold == xlBLACK) {
            pold = pnew;
        } else if (pnew.alpha > 0 && pnew != xlBLACK) {
            xlColor c;
            int r = pnew.red + pold.red * (255 - pnew.alpha) / 255;
            if (r > 255) r = 255;
            pold.red = r;
            int g = pnew.green + pold.green * (255 - pnew.alpha) / 255;
            if (g > 255) g = 255;
            pold.green = g;
            int b = pnew.blue + pold.blue * (255 - pnew.alpha) / 255;
            if (b > 255) b = 255;
            pold.blue = b;
            int a = pnew.alpha + pold.alpha * (255 - pnew.alpha) / 255;
            if (a > 255) a = 255;
            pold.alpha = a;
        }
    }, 2000);
}


RenderBuffer::RenderBuffer(RenderContext *ctx, PixelBufferClass *p, const Model *m) : renderContext(ctx), parent(p)
{
    model = m == nullptr ? p->GetModel() : m;
    cur_model = model->GetFullName();
    dmx_buffer = IsDmxDisplayType(model->GetDisplayAs());
    BufferHt = 0;
    BufferWi = 0;
    curPeriod = 0;
    curEffStartPer = 0;
    curEffEndPer = 0;
    fadeinsteps = 0;
    fadeoutsteps = 0;
    allowAlpha = false;
    needToInit = true;
    _nodeBuffer = false;
    frameTimeInMs = 50;
    _textDrawingContext = nullptr;
    isTransformed = false;
}

RenderBuffer::~RenderBuffer()
{
    if (_isCopy) Forget();

    //dtor
    if (_textDrawingContext != nullptr) {
        TextDrawingContext::ReleaseContext(_textDrawingContext);
    }
    for (auto& it : infoCache) {
        delete it.second;
        it.second = nullptr;
    }
    if (gpuRenderData) {
        GPURenderUtils::cleanUp(this);
        gpuRenderData = nullptr;
    }
}

TextDrawingContext* RenderBuffer::GetTextDrawingContext()
{
    if (_textDrawingContext == nullptr) {
        _textDrawingContext = TextDrawingContext::GetContext();
        _textDrawingContext->ResetSize(BufferWi, BufferHt);
    } else if (_textDrawingContext->GetWidth() != (size_t)BufferWi || _textDrawingContext->GetHeight() != (size_t)BufferHt) {
        // varying subbuffers the size may have changed
        _textDrawingContext->ResetSize(BufferWi, BufferHt);
    }

    return _textDrawingContext;
}

void RenderBuffer::InitBuffer(int newBufferHt, int newBufferWi, const std::string& bufferTransform, bool nodeBuffer)
{
    if (_textDrawingContext != nullptr && (BufferHt != newBufferHt || BufferWi != newBufferWi)) {
        _textDrawingContext->ResetSize(newBufferWi, newBufferHt);
    }
    _nodeBuffer = nodeBuffer;
    BufferHt = newBufferHt;
    BufferWi = newBufferWi;

    size_t NumPixels = BufferHt * BufferWi;
    // This is an absurdly high number but there are circumstances right now when creating a buffer based on a zoomed in camera when these can be hit.
    //wxASSERT(NumPixels < 500000);

    if (NumPixels != pixelVector.size()) {
        bool resetPtr = pixelVector.size() == 0 || pixels == &pixelVector[0];
        bool resetTPtr = tempbufVector.size() == 0 || tempbuf == &tempbufVector[0];
        pixelVector.resize(NumPixels);
        tempbufVector.resize(NumPixels);
        if (resetPtr) {
            // If the pixels or tempbuf ptr did not point to the first element
            // originally, then it is pointing into GPU memory and we need
            // to keep that pointer pointing there so the data can be retreived
            // from the GPU.
            pixels = &pixelVector[0];
        }
        if (resetTPtr) {
            tempbuf = &tempbufVector[0];
        }
    }
    
    blendBuffer.resize(Nodes.size());
    int indexCount = Nodes.size();
    for (auto &n : Nodes) {
        if (n->Coords.size() > 1) {
            indexCount += n->Coords.size() + 1;
        }
    }
    if (indexVector.size() < (size_t)indexCount) {
        indexVector.resize(indexCount);
    }
    allSimpleIndex = true;
    int idx = 0;
    int extraIdx = Nodes.size();
    for (auto &n : Nodes) {
        if (n->Coords.size() > 1) {
            allSimpleIndex = false;
            indexVector[idx] = extraIdx | 0x80000000;
            int countIdx = extraIdx++;
            indexVector[countIdx] = n->Coords.size();
            for (auto &c : n->Coords) {
                if (c.bufY < 0 || c.bufY >= BufferHt ||
                    c.bufX < 0 || c.bufX >= BufferWi ) {
                    indexVector[countIdx] -= 1;
                } else {
                    int32_t pidx = c.bufY * BufferWi + c.bufX;
                    indexVector[extraIdx++] = pidx;
                }
            }
        } else if (n->Coords[0].bufY < 0 || n->Coords[0].bufY >= BufferHt ||
                   n->Coords[0].bufX < 0 || n->Coords[0].bufX >= BufferWi ) {
            indexVector[idx] = 0xFFFFFFFF;
        } else {
            int32_t pidx = n->Coords[0].bufY * BufferWi + n->Coords[0].bufX;
            indexVector[idx] = pidx;
        }
        ++idx;
    }
    
    isTransformed = (bufferTransform != "None");
}

void RenderBuffer::Clear()
{
    if (pixelVector.size() > 0) {
        std::fill(pixels, pixels + pixelVector.size(), xlColor(0, 0, 0, 0));
    }
}

void RenderBuffer::SetPalette(xlColorVector& newcolors, xlColorCurveVector& newcc)
{
    palette.Set(newcolors, newcc);
}

size_t RenderBuffer::GetColorCount()
{
    return palette.Size();
}

// generates a random number between num1 and num2 inclusive
double RenderBuffer::RandomRange(double num1, double num2) const
{
    double hi,lo;
    if (num1 < num2)
    {
        lo = num1;
        hi = num2;
    }
    else
    {
        lo = num2;
        hi = num1;
    }
    return rand01()*(hi-lo)+ lo;
}

void RenderBuffer::Color2HSV(const xlColor& color, HSVValue& hsv) const
{
    color.toHSV(hsv);
}

// sets newcolor to a random color between hsv1 and hsv2
void RenderBuffer::SetRangeColor(const HSVValue& hsv1, const HSVValue& hsv2, HSVValue& newhsv)
{
    newhsv.hue=RandomRange(hsv1.hue,hsv2.hue);
    newhsv.saturation=RandomRange(hsv1.saturation,hsv2.saturation);
    newhsv.value=1.0;
}

// return a value between c1 and c2
uint8_t RenderBuffer::ChannelBlend(uint8_t c1, uint8_t c2, float ratio)
{
    return c1 + floor(ratio * (c2 - c1) + 0.5);
}

void RenderBuffer::Get2ColorBlend(int coloridx1, int coloridx2, float ratio, xlColor &color)
{
    color = palette.GetColor(coloridx1);
    const xlColor &c2 = palette.GetColor(coloridx2);
    Get2ColorBlend(color, c2, ratio);
}

void RenderBuffer::Get2ColorBlend(xlColor& color, xlColor color2, float ratio)
{
    color.Set(ChannelBlend(color.Red(), color2.Red(), ratio), ChannelBlend(color.Green(), color2.Green(), ratio), ChannelBlend(color.Blue(), color2.Blue(), ratio));
}

void RenderBuffer::Get2ColorAlphaBlend(const xlColor& c1, const xlColor& c2, float ratio, xlColor &color)
{
    color.Set(ChannelBlend(c1.Red(),c2.Red(),ratio), ChannelBlend(c1.Green(),c2.Green(),ratio), ChannelBlend(c1.Blue(),c2.Blue(),ratio));
}

inline uint8_t SumUInt8(uint8_t c1, uint8_t c2)
{
    int x = c1;
    x += c2;
    if (x > 255) x = 255;
    return x;
}

HSVValue RenderBuffer::Get2ColorAdditive(HSVValue& hsv1, HSVValue& hsv2) const
{
    xlColor rgb;
    xlColor rgb1(hsv1);
    xlColor rgb2(hsv2);
    rgb.red = SumUInt8(rgb1.red, rgb2.red);
    rgb.green = SumUInt8(rgb1.green, rgb2.green);
    rgb.blue = SumUInt8(rgb1.blue, rgb2.blue);
    return rgb.asHSV();
}
// 0 <= n < 1
void RenderBuffer::GetMultiColorBlend(float n, bool circular, xlColor &color, int reserveColours)
{
    size_t colorcnt = GetColorCount() - reserveColours;
    if (colorcnt <= 1)
    {
        palette.GetColor(0, color);
        return;
    }

    if (n >= 1.0) n = 0.99999f;
    if (n < 0.0) n = 0.0f;
    float realidx = circular ? n * colorcnt : n * (colorcnt - 1);
    int coloridx1 = floor(realidx);
    int coloridx2 = (coloridx1 + 1) % colorcnt;
    float ratio = realidx - float(coloridx1);
    Get2ColorBlend(coloridx1, coloridx2, ratio, color);
}

// 0,0 is lower left
void RenderBuffer::SetPixel(int x, int y, const xlColor &color, bool wrap, bool useAlpha, bool dmx_ignore)
{
    if (!dmx_ignore && dmx_buffer) {
        SetPixelDMXModel(x, y, color);
        return;
    }

    if (wrap) {
        while (x < 0) {
            x += BufferWi;
        }
        while (y < 0) {
            y += BufferHt;
        }
        while (x > BufferWi) {
            x -= BufferWi;
        }
        while (y > BufferHt) {
            y -= BufferHt;
        }
    }

    // I dont like this ... it should actually never happen
    if (x >= 0 && x < BufferWi && y >= 0 && y < BufferHt && (size_t)(y*BufferWi + x) < pixelVector.size())
    {
        // if you do this sparkles dont work when 100% transparent on effect ... so dont do it
        //if (color.alpha == 0)
        //{
            // transparent ... dont do anything
        //}
        //else
        if (useAlpha && color.Alpha() != 255)
        {
            xlColor pnew = color;
            xlColor pold = pixels[y*BufferWi + x];

            xlColor c;
            int r = pnew.red + (pold.red * (255 - pnew.alpha)) / 255;
            if (r > 255) r = 255;
            c.red = r;
            int g = pnew.green + (pold.green * (255 - pnew.alpha)) / 255;
            if (g > 255) g = 255;
            c.green = g;
            int b = pnew.blue + (pold.blue * (255 - pnew.alpha)) / 255;
            if (b > 255) b = 255;
            c.blue = b;
            int a = pnew.alpha + (pold.alpha * (255 - pnew.alpha)) / 255;
            if (a > 255) a = 255;
            c.alpha = a;

            pixels[y*BufferWi + x] = c;
        }
        else
        {
            pixels[y*BufferWi + x] = color;
        }
    }
}

void RenderBuffer::ProcessPixel(int x_pos, int y_pos, const xlColor &color, bool wrap_x, bool wrap_y)
{
    int x_value = x_pos;
    if (wrap_x)  // if set wrap image at boundary
    {
        x_value %= BufferWi;
        x_value = (x_value >= 0) ? (x_value) : (BufferWi + x_value);
    }
    int y_value = y_pos;
    if (wrap_y)
    {
        y_value %= BufferHt;
        y_value = (y_value >= 0) ? (y_value) : (BufferHt + y_value);
    }
    SetPixel(x_value, y_value, color);
}

// 0,0 is lower left
void RenderBuffer::SetPixel(int x, int y, const HSVValue& hsv, bool wrap)
{
    if (dmx_buffer) {
        SetPixelDMXModel(x, y, xlColor(hsv));
        return;
    }

    if (wrap) {
        while (x < 0) {
            x += BufferWi;
        }
        while (y < 0) {
            y += BufferHt;
        }
        while (x > BufferWi) {
            x -= BufferWi;
        }
        while (y > BufferHt) {
            y -= BufferHt;
        }
    }
    if (x >= 0 && x < BufferWi && y >= 0 && y < BufferHt && (size_t)(y*BufferWi + x) < pixelVector.size())
    {
        pixels[y*BufferWi+x] = hsv;
    }
}

void RenderBuffer::SetNodePixel(int nodeNum, const xlColor &color, bool dmx_ignore) {
    if (nodeNum < (int)Nodes.size()) {
        for (auto &a : Nodes[nodeNum]->Coords) {
            SetPixel(a.bufX, a.bufY, color, false, false, dmx_ignore);
        }
    }
}

//copy src to dest: -DJ
void RenderBuffer::CopyPixel(int srcx, int srcy, int destx, int desty)
{
    if ((srcx >= 0) && (srcx < BufferWi) && (srcy >= 0) && (srcy < BufferHt) && (size_t)(srcy*BufferWi + srcx) < pixelVector.size())
        if ((destx >= 0) && (destx < BufferWi) && (desty >= 0) && (desty < BufferHt) && (size_t)(desty*BufferWi + destx) < pixelVector.size())
        {
            pixels[desty * BufferWi + destx] = pixels[srcy * BufferWi + srcx];
        }
}

void RenderBuffer::DrawHLine(int y, int xstart, int xend, const xlColor &color, bool wrap) {
    if (xstart > xend) {
        int i = xstart;
        xstart = xend;
        xend = i;
    }
    for (int x = xstart; x <= xend; x++) {
        SetPixel(x, y, color, wrap);
    }
}
void RenderBuffer::DrawVLine(int x, int ystart, int yend, const xlColor &color, bool wrap) {
    if (ystart > yend) {
        int i = ystart;
        ystart = yend;
        yend = i;
    }
    for (int y = ystart; y <= yend; y++) {
        SetPixel(x, y, color, wrap);
    }
}
void RenderBuffer::DrawBox(int x1, int y1, int x2, int y2, const xlColor& color, bool wrap, bool useAlpha) {
    if (y1 > y2) {
        int i = y1;
        y1 = y2;
        y2 = i;
    }
    if (x1 > x2) {
        int i = x1;
        x1 = x2;
        x2 = i;
    }
    for (int x = x1; x <= x2; x++) {
        for (int y = y1; y <= y2; y++) {
            SetPixel(x, y, color, wrap, useAlpha);
        }
    }
}

// Xiaolin Wu's anti-aliased line algorithm
// Uses direct color blending with background rather than alpha, so overlapping
// draws don't accumulate.
void RenderBuffer::DrawAALine(const float x0, const float y0, const float x1, const float y1, const xlColor& color)
{
    auto plot = [&](int ix, int iy, float brightness) {
        if (brightness <= 0.0f) return;
        brightness = std::min(brightness, 1.0f);
        xlColor bg = GetPixel(ix, iy);
        uint8_t r = (uint8_t)(bg.red + (color.red - bg.red) * brightness);
        uint8_t g = (uint8_t)(bg.green + (color.green - bg.green) * brightness);
        uint8_t b = (uint8_t)(bg.blue + (color.blue - bg.blue) * brightness);
        SetPixel(ix, iy, xlColor(r, g, b));
    };

    bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);
    float ax0 = x0, ay0 = y0, ax1 = x1, ay1 = y1;
    if (steep) { std::swap(ax0, ay0); std::swap(ax1, ay1); }
    if (ax0 > ax1) { std::swap(ax0, ax1); std::swap(ay0, ay1); }

    float dx = ax1 - ax0;
    float dy = ay1 - ay0;
    float gradient = (dx < 0.001f) ? 1.0f : dy / dx;

    // first endpoint
    float xend = std::round(ax0);
    float yend = ay0 + gradient * (xend - ax0);
    float xgap = 1.0f - (ax0 + 0.5f - std::floor(ax0 + 0.5f));
    int xpxl1 = (int)xend;
    int ypxl1 = (int)std::floor(yend);
    if (steep) {
        plot(ypxl1, xpxl1, (1.0f - (yend - ypxl1)) * xgap);
        plot(ypxl1 + 1, xpxl1, (yend - ypxl1) * xgap);
    } else {
        plot(xpxl1, ypxl1, (1.0f - (yend - ypxl1)) * xgap);
        plot(xpxl1, ypxl1 + 1, (yend - ypxl1) * xgap);
    }
    float intery = yend + gradient;

    // second endpoint
    xend = std::round(ax1);
    yend = ay1 + gradient * (xend - ax1);
    xgap = ax1 + 0.5f - std::floor(ax1 + 0.5f);
    int xpxl2 = (int)xend;
    int ypxl2 = (int)std::floor(yend);
    if (steep) {
        plot(ypxl2, xpxl2, (1.0f - (yend - ypxl2)) * xgap);
        plot(ypxl2 + 1, xpxl2, (yend - ypxl2) * xgap);
    } else {
        plot(xpxl2, ypxl2, (1.0f - (yend - ypxl2)) * xgap);
        plot(xpxl2, ypxl2 + 1, (yend - ypxl2) * xgap);
    }

    // main loop
    for (int x = xpxl1 + 1; x < xpxl2; ++x) {
        int iy = (int)std::floor(intery);
        float frac = intery - iy;
        if (steep) {
            plot(iy, x, 1.0f - frac);
            plot(iy + 1, x, frac);
        } else {
            plot(x, iy, 1.0f - frac);
            plot(x, iy + 1, frac);
        }
        intery += gradient;
    }
}

// Anti-aliased filled circle using distance from center for edge smoothing.
// Uses direct color blending with background rather than alpha, so overlapping
// draws don't accumulate.
void RenderBuffer::DrawAACircle(const float cx, const float cy, const float radius, const xlColor& color)
{
    int r = (int)std::ceil(radius + 1.0f);
    int ixc = (int)std::round(cx);
    int iyc = (int)std::round(cy);
    int xmin = std::max(0, ixc - r);
    int xmax = std::min(BufferWi - 1, ixc + r);
    int ymin = std::max(0, iyc - r);
    int ymax = std::min(BufferHt - 1, iyc + r);

    float outer = radius + 0.75f;
    float outer2 = outer * outer;
    float inner = std::max(0.0f, radius - 0.75f);
    float inner2 = inner * inner;

    for (int y = ymin; y <= ymax; ++y) {
        float dy = (float)y - cy;
        float dy2 = dy * dy;
        for (int x = xmin; x <= xmax; ++x) {
            float dx = (float)x - cx;
            float d2 = dx * dx + dy2;
            if (d2 > outer2) continue;
            if (d2 <= inner2) {
                SetPixel(x, y, color);
            } else {
                float d = std::sqrt(d2);
                float coverage = std::max(0.0f, std::min(1.0f, (outer - d) / (outer - inner)));
                xlColor bg = GetPixel(x, y);
                uint8_t cr = (uint8_t)(bg.red + (color.red - bg.red) * coverage);
                uint8_t cg = (uint8_t)(bg.green + (color.green - bg.green) * coverage);
                uint8_t cb = (uint8_t)(bg.blue + (color.blue - bg.blue) * coverage);
                SetPixel(x, y, xlColor(cr, cg, cb));
            }
        }
    }
}

// Bresenham's line algorithm
void RenderBuffer::DrawLine( const int x0_, const int y0_, const int x1_, const int y1_, const xlColor& color, bool useAlpha)
{
    int x0 = x0_;
    int x1 = x1_;
    int y0 = y0_;
    int y1 = y1_;

    int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
    int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1;
    int err = (dx>dy ? dx : -dy)/2;

  for(;;){
    SetPixel(x0,y0, color, false, useAlpha);
    if (x0==x1 && y0==y1) break;
    int e2 = err;
    if (e2 >-dx) { err -= dy; x0 += sx; }
    if (e2 < dy) { err += dx; y0 += sy; }
  }
}

void RenderBuffer::DrawThickLine(const int x1_, const int y1_, const int x2_, const int y2_, const xlColor& color, int thickness, bool useAlpha)
{
    if (thickness < 1) return;
    if (thickness == 1)
    {
        DrawLine(x1_, y1_, x2_, y2_, color, useAlpha);
    }
    else
    {
        DrawCircle(x1_, y1_, thickness / 2, color, true);
        DrawCircle(x2_, y2_, thickness / 2, color, true);
        for (int i =0; i < thickness; i++)
        {
            int adjust = i - thickness / 2;
            DrawLine(x1_ + adjust, y1_, x2_ + adjust, y2_, color, useAlpha);
            DrawLine(x1_, y1_ + adjust, x2_, y2_ + adjust, color, useAlpha);
            DrawLine(x1_ + adjust, y1_ + adjust, x2_ + adjust, y2_ + adjust, color, useAlpha);
        }
    }
}

void RenderBuffer::DrawThickLine( const int x0_, const int y0_, const int x1_, const int y1_, const xlColor& color, bool direction )
{
    int x0 = x0_;
    int x1 = x1_;
    int y0 = y0_;
    int y1 = y1_;
    int lastx = x0;
    int lasty = y0;

    int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
    int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1;
    int err = (dx>dy ? dx : -dy)/2, e2;

  for(;;){
    SetPixel(x0,y0, color);
    if( (x0 != lastx) && (y0 != lasty) && (x0_ != x1_) && (y0_ != y1_) )
    {
        int fix = 0;
        if( x0 > lastx ) fix += 1;
        if( y0 > lasty ) fix += 2;
        if( direction  ) fix += 4;
        switch (fix)
        {
        case 2:
        case 4:
            if( x0 < BufferWi -2 ) SetPixel(x0+1,y0, color);
            break;
        case 3:
        case 5:
            if( x0 > 0 ) SetPixel(x0-1,y0, color);
            break;
        case 0:
        case 1:
            if( y0 < BufferHt -2 )SetPixel(x0,y0+1, color);
            break;
        case 6:
        case 7:
            if( y0 > 0 )SetPixel(x0,y0-1, color);
            break;
        default: break;
        }
    }
    lastx = x0;
    lasty = y0;
    if (x0==x1 && y0==y1) break;
    e2 = err;
    if (e2 >-dx) { err -= dy; x0 += sx; }
    if (e2 < dy) { err += dx; y0 += sy; }
  }
}

typedef std::pair<int, int> HLine;

static void ScanEdge(int x1, int y1, int x2, int y2, int setx, bool skip, std::vector<HLine> &lines, int &eidx)
{
  int dx = x2 - x1;
  int dy = y2 - y1;
  if (dy <= 0)
    return;
  double invs = (double)dx / (double)dy;

  int idx = eidx;

  for (int y = y1 + (skip ? 1 : 0); y<y2; ++y, ++idx) {
    if (setx)
        lines[idx].first = x1 + (int)(ceil((y - y1) * invs));
    else
        lines[idx].second = x1 + (int)(ceil((y - y1) * invs));
  }
  eidx = idx;
}

void RenderBuffer::FillConvexPoly(const std::vector<std::pair<int, int>>& opoly, const xlColor& color)
{
    if (opoly.empty())
        return;
    std::vector<std::pair<int, int>> poly;
    poly.push_back(opoly[0]);
    for (size_t i = 1; i < opoly.size(); ++i) {
        if (opoly[i] != opoly[i - 1]) {
            poly.push_back(opoly[i]);
        }
    }
    if (poly[0] == poly[poly.size() - 1]) {
        poly.pop_back();
    }

    // Loosely based on Michael Abrash's Graphics Programming Black Book (TGPBB)
    // Feels very low tech compared to what should be here, but high tech compared to the
    //    rest of the stuff that actually is here (shrug)
    if (poly.size() < 3)
       return;
    int miny, maxy, minx, maxx;
    minx = maxx = poly[0].first;
    miny = maxy = poly[0].second;
    int minidxl = 0, maxidx = 0;

    // Find the top and bottom
    for (size_t i = 1; i < poly.size(); ++i) {
        if (poly[i].second < miny) {
            minidxl = i;
            miny = poly[i].second;
        }
        if (poly[i].second > maxy) {
            maxidx = i;
            maxy = poly[i].second;
        }
        minx = std::min(minx, poly[i].first);
        maxx = std::max(maxx, poly[i].first);
    }

    // Empty? Off Screen?
    if (miny == maxy)
       return;
    if (minx >= this->BufferWi || maxx <= 0)
        return;
    if (miny >= this->BufferHt || maxy <= 0)
       return;

    int minidxr = minidxl;
    while (poly[minidxr].second == miny)
       minidxr = (minidxr + 1) % poly.size();
    minidxr = (minidxr + poly.size() - 1) % poly.size();

    while (poly[minidxl].second == miny)
       minidxl = (minidxl + poly.size() - 1) % poly.size();
    minidxl = (minidxl + 1) % poly.size();

    int ledir = -1;
    bool tif = (poly[minidxl].first != poly[minidxr].first);
    if (tif) {
       if (poly[minidxl].first > poly[minidxr].first) {
            ledir = 1;
            std::swap(minidxl, minidxr);
       }
    } else {
       int nidx = minidxr;
       nidx = (nidx + 1) % poly.size();
       int pidx = minidxl;
       pidx = (pidx + poly.size() - 1) % poly.size();
       int dxn = poly[nidx].first - poly[minidxl].first;
       int dyn = poly[nidx].second - poly[minidxl].second;
       int dxp = poly[pidx].first - poly[minidxl].first;
       int dyp = poly[pidx].second - poly[minidxl].second;
       if (((long long)dxn * dyp - (long long)dyn * dxp) < 0L) {
            ledir = 1;
            std::swap(minidxl, minidxr);
       }
    }

    int wheight = maxy - miny - 1 + (tif ? 1 : 0);
    if (wheight <= 0)
       return;
    int ystart = miny + 1 - (tif ? 1 : 0);

    std::vector<HLine> hlines(wheight);

    int edgept = 0;
    int cidx = minidxl, pidx = minidxl;
    bool skip = tif ? 0 : 1;

    /* Scan convert each line in the left edge from top to bottom */
    do {
       cidx = (cidx + poly.size() + ledir) % poly.size();
       ScanEdge(poly[pidx].first, poly[pidx].second,
                poly[cidx].first, poly[cidx].second,
                true, skip, hlines, edgept);
       pidx = cidx;
       skip = false;
    } while (cidx != maxidx);

    edgept = 0;
    pidx = cidx = minidxr;

    skip = tif ? 0 : 1;
    /* Scan convert the right edge, top to bottom. X coordinates are
       adjusted 1 to the left, effectively causing scan conversion of
       the nearest points to the left of but not exactly on the edge */
    do {
       cidx = (cidx + poly.size() - ledir) % poly.size();
       ScanEdge(poly[pidx].first - 1, poly[pidx].second,
                poly[cidx].first - 1, poly[cidx].second,
                false, skip, hlines, edgept);
       pidx = cidx;
       skip = false;
    } while (cidx != maxidx);

    // Draw the line list representing the scan converted polygon
    for (int y = ystart, en = 0; y < int(ystart + hlines.size()); ++y, ++en) {
       if (y < 0 || y >= BufferHt)
            continue;
       int sx = std::max(0, hlines[en].first);
       int ex = std::min(hlines[en].second, BufferWi - 1);
       for (int x = sx; x <= ex; ++x)
            SetPixel(x, y, color, false);
    }
}

void RenderBuffer::DrawFadingCircle(int x0, int y0, int radius, const xlColor& rgb, bool wrap)
{
    HSVValue hsv(rgb);
    xlColor color(rgb);

    double full_brightness = hsv.value;

    for (int x = -radius; x < radius; ++x) {
        for (int y = -radius; y < radius; ++y) {
            double d = std::sqrt(x * x + y * y);
            if (d <= radius) {
                if (allowAlpha) {
                    double alpha = (double)rgb.alpha - ((double)rgb.alpha * d) / double(radius);
                    if (alpha > 0.0) {
                        color.alpha = alpha;
                        SetPixel(x + x0, y + y0, color, wrap, false);
                    }
                }
                else {
                    double alpha = full_brightness - (full_brightness * d) / (double)radius;
                    if (alpha > 0.0) {
                        hsv.value = alpha;
                        color = hsv;
                        SetPixel(x + x0, y + y0, color, wrap);
                    }
                }
            }
        }
    }
}

void RenderBuffer::DrawCircle(int x0, int y0, int radius, const xlColor& rgb, bool filled, bool wrap)
{
    int x = radius;
    int y = 0;
    int radiusError = 1 - x;

    while(x >= y) {
        if (!filled) {
            SetPixel(x + x0, y + y0, rgb, wrap);
            SetPixel(y + x0, x + y0, rgb, wrap);
            SetPixel(-x + x0, y + y0, rgb, wrap);
            SetPixel(-y + x0, x + y0, rgb, wrap);
            SetPixel(-x + x0, -y + y0, rgb, wrap);
            SetPixel(-y + x0, -x + y0, rgb, wrap);
            SetPixel(x + x0, -y + y0, rgb, wrap);
            SetPixel(y + x0, -x + y0, rgb, wrap);
        } else {
            DrawVLine(x0 - x, y0 - y, y0 + y, rgb, wrap);
            DrawVLine(x0 + x, y0 - y, y0 + y, rgb, wrap);
            DrawVLine(x0 - y, y0 - x, y0 + x, rgb, wrap);
            DrawVLine(x0 + y, y0 - x, y0 + x, rgb, wrap);
        }
        y++;
        if (radiusError<0) {
            radiusError += 2 * y + 1;
        } else {
            x--;
            radiusError += 2 * (y - x) + 1;
        }
    }
}

void RenderBuffer::Fill(const xlColor& color) {
    std::fill_n(pixels, pixelVector.size(), color);
}


// 0,0 is lower left
void RenderBuffer::GetPixel(int x, int y, xlColor &color) const
{
    // I also dont like this ... I shouldnt need to check against pixel size
    int pidx = y * BufferWi + x;
    if (x >= 0 && x < BufferWi && y >= 0 && y < BufferHt && pidx < (int)pixelVector.size()) {
        color = pixels[pidx];
    } else {
        color = this->allowAlpha ? xlCLEAR : xlBLACK;
        
    }
}

const xlColor& RenderBuffer::GetPixel(int x, int y) const {
    int pidx = y * BufferWi + x;
    if (x >= 0 && x < BufferWi && y >= 0 && y < BufferHt && pidx < (int)pixelVector.size()) {
        return pixels[pidx];
    }
    return this->allowAlpha ? xlCLEAR : xlBLACK;
}

// 0,0 is lower left
void RenderBuffer::SetTempPixel(int x, int y, const xlColor& color) {
    int pidx = y * BufferWi + x;
    if (x >= 0 && x < BufferWi && y >= 0 && y < BufferHt && pidx < (int)tempbufVector.size()) {
        tempbuf[pidx] = color;
    }
}

void RenderBuffer::SetTempPixel(int x, int y, const xlColor & color, int alpha) {

    xlColor c(color.Red(), color.Green(), color.Blue(), alpha);
    SetTempPixel(x, y, c);
}

// 0,0 is lower left
void RenderBuffer::GetTempPixel(int x, int y, xlColor& color)
{
    if (x >= 0 && x < BufferWi && y >= 0 && y < BufferHt && y * BufferWi + x < (int)tempbufVector.size()) {
        color = tempbuf[y * BufferWi + x];
    }
}

const xlColor& RenderBuffer::GetTempPixel(int x, int y) {
    if (x >= 0 && x < BufferWi && y >= 0 && y < BufferHt && y * BufferWi + x < (int)tempbufVector.size()) {
        return tempbuf[y * BufferWi + x];
    }
    return this->allowAlpha ? xlCLEAR : xlBLACK;
}

const xlColor& RenderBuffer::GetTempPixelRGB(int x, int y)
{
    if (x >= 0 && x < BufferWi && y >= 0 && y < BufferHt && y * BufferWi + x < (int)tempbufVector.size()) {
        return tempbuf[y * BufferWi + x];
    }
    return this->allowAlpha ? xlCLEAR : xlBLACK;
}

void RenderBuffer::SetState(int period, bool ResetState)
{
    if (ResetState) {
        needToInit = true;
    }
    curPeriod = period;
    curPeriod = period;
    palette.UpdateForProgress(GetEffectTimeIntervalPosition());
}

void RenderBuffer::ClearTempBuf()
{
    for (size_t i = 0; i < tempbufVector.size(); i++) {
        tempbuf[i].Set(0, 0, 0, 0);
    }
}
void RenderBuffer::CopyTempBufToPixels() {
    std::copy(tempbuf, tempbuf + pixelVector.size(), pixels);
}
void RenderBuffer::CopyPixelsToTempBuf() {
    std::copy(pixels, pixels + pixelVector.size(), tempbuf);
}

// Gets the maximum oversized buffer size a model could have
// Useful for models which need to track data per cell as with value curves the buffer size could
// get as large as this during the effect
xlSize RenderBuffer::GetMaxBuffer(const SettingsMap& SettingsMap) const
{
    Model* m = renderContext->GetModel(cur_model);
    if (m == nullptr) {
        return xlSize(-1, -1);
    }
    std::string bufferstyle = SettingsMap.Get("CHOICE_BufferStyle", "Default");
    std::string transform = SettingsMap.Get("CHOICE_BufferTransform", "None");
    std::string camera = SettingsMap.Get("CHOICE_PerPreviewCamera", "2D");
    int w, h;
    
    static const std::string PER_MODEL("Per Model");
    static const std::string DEEP("Deep");
    if (bufferstyle.compare(0, 9, PER_MODEL) == 0) {
        bufferstyle = bufferstyle.substr(10);
        if (bufferstyle.compare(bufferstyle.length() - 4, 4, DEEP) == 0) {
            bufferstyle = bufferstyle.substr(0, bufferstyle.length() - 5);
        }
    }
    
    m->GetBufferSize(bufferstyle, camera, transform, w, h, SettingsMap.GetInt("B_SPINCTRL_BufferStagger", 0));
    float xScale = (SB_RIGHT_TOP_MAX - SB_LEFT_BOTTOM_MIN) / 100.0;
    float yScale = (SB_RIGHT_TOP_MAX - SB_LEFT_BOTTOM_MIN) / 100.0;
    return xlSize(xScale * w, yScale * h);
}

float RenderBuffer::GetEffectTimeIntervalPosition(float cycles) const {
    if (curEffEndPer == curEffStartPer) {
        return 0.0f;
    }
    float periods = curEffEndPer - curEffStartPer + 1; //inclusive
    float periodsPerCycle = periods / cycles;
    if (periodsPerCycle <= 1.0) {
        return 0.0f;
    }
    float retval = (float)(curPeriod - curEffStartPer);
    while (retval >= periodsPerCycle) {
        retval -= periodsPerCycle;
    }
    retval /= (periodsPerCycle - 1);
    return retval > 1.0f ? 1.0f : retval;
}

float RenderBuffer::GetEffectTimeIntervalPosition() const
{
    if (curEffEndPer == curEffStartPer) {
        return 0.0;
    }
    return (float)(curPeriod - curEffStartPer) / (float)(curEffEndPer - curEffStartPer);
}

void RenderBuffer::SetEffectDuration(int startMsec, int endMsec)
{
    curEffStartPer = startMsec / frameTimeInMs;
    curEffEndPer = (endMsec - 1) / frameTimeInMs;
}

void RenderBuffer::GetEffectPeriods(int& start, int& endp) const {
    start = curEffStartPer;
    endp = curEffEndPer;
}

void RenderBuffer::SetDisplayListHRect(Effect *eff, int idx, float x1, float y1, float x2, float y2,
                                     const xlColor &c1, const xlColor &c2) {
    SetDisplayListRect(eff, idx, x1, y1, x2, y2, c1, c1, c2, c2);
}
void RenderBuffer::SetDisplayListVRect(Effect *eff, int idx, float x1, float y1, float x2, float y2,
                                     const xlColor &c1, const xlColor &c2) {
    SetDisplayListRect(eff, idx, x1, y1, x2, y2, c1, c2, c1, c2);
}
void RenderBuffer::SetDisplayListRect(Effect *eff, int idx, float x1, float y1, float x2, float y2,
                                    const xlColor &cx1y1, const xlColor &cx1y2,
                                    const xlColor &cx2y1, const xlColor &cx2y2) {
    if (!eff->IsBackgroundDisplayListEnabled()) {
        return;
    }

    xlColor* colorMask = eff->GetColorMask();
    xlColor maskcx1y1 = cx1y1;
    xlColor maskcx1y2 = cx1y2;
    xlColor maskcx2y1 = cx2y1;
    xlColor maskcx2y2 = cx2y2;
    maskcx1y1.ApplyMask(colorMask);
    maskcx1y2.ApplyMask(colorMask);
    maskcx2y1.ApplyMask(colorMask);
    maskcx2y2.ApplyMask(colorMask);

    eff->GetBackgroundDisplayList()[idx].color = maskcx1y1;
    eff->GetBackgroundDisplayList()[idx+1].color = maskcx1y2;
    eff->GetBackgroundDisplayList()[idx+2].color = maskcx2y2;
    eff->GetBackgroundDisplayList()[idx+3].color = maskcx2y2;
    eff->GetBackgroundDisplayList()[idx+4].color = maskcx2y1;
    eff->GetBackgroundDisplayList()[idx+5].color = maskcx1y1;

    eff->GetBackgroundDisplayList()[idx].x = x1;
    eff->GetBackgroundDisplayList()[idx+1].x = x1;
    eff->GetBackgroundDisplayList()[idx+2].x = x2;
    eff->GetBackgroundDisplayList()[idx+3].x = x2;
    eff->GetBackgroundDisplayList()[idx+4].x = x2;
    eff->GetBackgroundDisplayList()[idx+5].x = x1;

    eff->GetBackgroundDisplayList()[idx].y = y1;
    eff->GetBackgroundDisplayList()[idx+1].y = y2;
    eff->GetBackgroundDisplayList()[idx+2].y = y2;
    eff->GetBackgroundDisplayList()[idx+3].y = y2;
    eff->GetBackgroundDisplayList()[idx+4].y = y1;
    eff->GetBackgroundDisplayList()[idx+5].y = y1;
}

void RenderBuffer::CopyPixelsToDisplayListX(Effect *eff, int row, int sx, int ex, int inc) {
    if (!eff->IsBackgroundDisplayListEnabled()) {
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(eff->GetBackgroundDisplayList().lock);
    int count = curEffEndPer - curEffStartPer + 1;

    int total = curEffEndPer - curEffStartPer + 1;
    float x = float(curPeriod - curEffStartPer) / float(total);
    float x2 = (curPeriod - curEffStartPer + 1.0) / float(total);
    xlColor c;

    int cur = 0;
    for (int p = sx; p <= ex; p += inc) {
        float y = float(p - sx) / float(ex - sx + 1.0);
        float y2 = float(p - sx + 1.0) / float(ex - sx + 1.0);
        GetPixel(p, row, c);

        int idx = cur * count + (curPeriod - curEffStartPer);
        cur++;
        SetDisplayListHRect(eff, idx * 6, x, y, x2, y2, c, c);
    }
}

double RenderBuffer::calcAccel(double ratio, double accel)
{
    if( accel == 0 ) return ratio;

    double pct_accel = (std::abs(accel) - 1.0) / 9.0;
    double new_accel1 = pct_accel * 5 + (1.0 - pct_accel) * 1.5;
    double new_accel2 = 1.5 + (ratio * new_accel1);
    double final_accel = pct_accel * new_accel2 + (1.0 - pct_accel) * new_accel1;

    if( accel > 0 ) {
        return std::pow(ratio, final_accel);
    } else {
        return (1.0 - std::pow(1.0 - ratio, new_accel1));
    }
}

// create a copy of the buffer suitable only for copying out pixel data and fake rendering
RenderBuffer::RenderBuffer(RenderBuffer& buffer) : pixelVector(buffer.pixels, &buffer.pixels[buffer.pixelVector.size()])
{
    _isCopy = true;
    parent = buffer.parent;
    model = buffer.model;
    renderContext = buffer.renderContext;
    curPeriod = buffer.curPeriod;
    curEffStartPer = buffer.curEffStartPer;
    curEffEndPer = buffer.curEffEndPer;
    frameTimeInMs = buffer.frameTimeInMs;
    isTransformed = buffer.isTransformed;
    fadeinsteps = buffer.fadeinsteps;
    fadeoutsteps = buffer.fadeoutsteps;
    needToInit = buffer.needToInit;
    allowAlpha = buffer.allowAlpha;
    dmx_buffer = buffer.dmx_buffer;
    _nodeBuffer = buffer._nodeBuffer;
    BufferHt = buffer.BufferHt;
    BufferWi = buffer.BufferWi;
    cur_model = buffer.cur_model;

    pixels = &pixelVector[0];
    _textDrawingContext = buffer._textDrawingContext;
    gpuRenderData = nullptr;
}

void RenderBuffer::Forget()
{
    // Forget some stuff as this is a fake render buffer and we dont want it destroyed
    _textDrawingContext = nullptr;
}

void RenderBuffer::SetPixelDMXModel(int x, int y, const xlColor& color)
{
    const Model* model_info = GetModel();
    if (model_info != nullptr) {
        if (x != 0 || y != 0) return;  //Only render colors for the first pixel

        if (pixelVector.size() == 1) { //pixel size equals 1 when putting "on" effect at node level
            pixels[0] = color;
            return;
        }
        const DmxModel* dmx = dynamic_cast<const DmxModel*>(model_info);
        if (dmx != nullptr)  {
            if (dmx->HasColorAbility()) {
                DmxColorAbility* dmx_color = dmx->GetColorAbility();
                dmx_color->SetColorPixels(color,pixelVector);
            }
            dmx->EnableFixedChannels(pixelVector);
        }
    }
}

void RenderBuffer::EnableFixedDMXChannels(const DmxModel* dmx) {
    if (dmx != nullptr) {
        dmx->EnableFixedChannels(pixelVector);
    }
}
