/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "xlColorCanvas.h"
#include <algorithm>
#include <cmath>

BEGIN_EVENT_TABLE(xlColorCanvas, GRAPHICS_BASE_CLASS)
EVT_PAINT(xlColorCanvas::render)
EVT_MOTION(xlColorCanvas::mouseMoved)
EVT_LEFT_DOWN(xlColorCanvas::mouseDown)
EVT_LEFT_UP(xlColorCanvas::mouseReleased)
END_EVENT_TABLE()

#define CORNER_NOT_SELECTED     0
#define CORNER_1A_SELECTED      1
#define CORNER_1B_SELECTED      2
#define CORNER_2A_SELECTED      3
#define CORNER_2B_SELECTED      4

xlColorCanvas::xlColorCanvas(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name) :
    GRAPHICS_BASE_CLASS(parent, id, pos, size, style, name),
    mHSV(0.0, 1.0, 1.0),
    mRGB(255, 0, 0),
    background(nullptr) {
}

xlColorCanvas::~xlColorCanvas() {
    if (background) {
        delete background;
    }
}

const HSVValue& xlColorCanvas::GetHSV() const {
    return mHSV;
}

void xlColorCanvas::SetMode(ColorDisplayMode mode) {
    if (mDisplayMode != mode) {
        mDisplayMode = mode;
        render();
    }
}

void xlColorCanvas::SetHSV(const HSVValue& hsv) {
    xlColor c = hsv;
    if (c != mRGB) {
        mHSV = hsv;
        mRGB = c;
        render();
    }
}

void xlColorCanvas::SetRGB(xlColor rgb) {
    if (mRGB != rgb) {
        mRGB = rgb;
        rgb.toHSV(mHSV);
        render();
    }
}

void xlColorCanvas::mouseDown(wxMouseEvent& event) {
    mDragging = true;
    if (mDisplayType == xlColorCanvas::DisplayType::TYPE_SLIDER) {
        ProcessSliderClick(mapLogicalToAbsolute(event.GetY()));
        wxCommandEvent eventColor(EVT_CP_SLIDER_CHANGED);
        eventColor.SetClientData(&mHSV);
        GetParent()->ProcessWindowEvent(eventColor);
    } else if (mDisplayType == xlColorCanvas::DisplayType::TYPE_PALETTE) {
        ProcessPaletteClick(mapLogicalToAbsolute(event.GetY()), mapLogicalToAbsolute(event.GetX()));
        wxCommandEvent eventColor(EVT_CP_PALETTE_CHANGED);
        eventColor.SetClientData(&mHSV);
        GetParent()->ProcessWindowEvent(eventColor);
    } else if (mDisplayType == xlColorCanvas::DisplayType::TYPE_WHEEL) {
        ProcessWheelClick(mapLogicalToAbsolute(event.GetY()), mapLogicalToAbsolute(event.GetX()));
        wxCommandEvent eventColor(EVT_CP_PALETTE_CHANGED);
        eventColor.SetClientData(&mHSV);
        GetParent()->ProcessWindowEvent(eventColor);
    }
    CaptureMouse();
    render();
}

void xlColorCanvas::mouseMoved(wxMouseEvent& event) {
    if (mDragging) {
        if (mDisplayType == xlColorCanvas::DisplayType::TYPE_SLIDER) {
            ProcessSliderClick(mapLogicalToAbsolute(event.GetY()));
            wxCommandEvent eventColor(EVT_CP_SLIDER_CHANGED);
            eventColor.SetClientData(&mHSV);
            GetParent()->ProcessWindowEvent(eventColor);
        } else if (mDisplayType == xlColorCanvas::DisplayType::TYPE_PALETTE) {
            ProcessPaletteClick(mapLogicalToAbsolute(event.GetY()), mapLogicalToAbsolute(event.GetX()));
            wxCommandEvent eventColor(EVT_CP_PALETTE_CHANGED);
            eventColor.SetClientData(&mHSV);
            GetParent()->ProcessWindowEvent(eventColor);
        } else if (mDisplayType == xlColorCanvas::DisplayType::TYPE_WHEEL) {
            ProcessWheelClick(mapLogicalToAbsolute(event.GetY()), mapLogicalToAbsolute(event.GetX()));
            wxCommandEvent eventColor(EVT_CP_PALETTE_CHANGED);
            eventColor.SetClientData(&mHSV);
            GetParent()->ProcessWindowEvent(eventColor);
        }
    }
}

void xlColorCanvas::mouseReleased(wxMouseEvent& event) {
    if (mDragging) {
        ReleaseMouse();
        mDragging = false;
    }
}

int xlColorCanvas::GetRGBColorFromRangeValue(int position, int range, int max_value, bool invert) {
    if (invert) {
        return (int)(max_value * (1.0 - (double)position / (double)range));
    } else {
        return (int)(max_value * (double)position / (double)range);
    }
}

void xlColorCanvas::ProcessSliderClick(int row) {
    float dYrange = mWindowHeight - 1;
    int iYrange = mWindowHeight - 1;
    if (row < 0)
        row = 0;
    else if (row > (int)mWindowHeight)
        row = (int)mWindowHeight;
    switch (mDisplayMode) {
    case MODE_HUE:
        mHSV.hue = 1.0 - (double)row / dYrange;
        mRGB = mHSV;
        break;
    case MODE_SATURATION:
        mHSV.saturation = 1.0 - (double)row / dYrange;
        mRGB = mHSV;
        break;
    case MODE_BRIGHTNESS:
        mHSV.value = 1.0 - (double)row / dYrange;
        mRGB = mHSV;
        break;
    case MODE_RED:
        mRGB.red = GetRGBColorFromRangeValue(row, iYrange, 255, true);
        mHSV = mRGB;
        break;
    case MODE_GREEN:
        mRGB.green = GetRGBColorFromRangeValue(row, iYrange, 255, true);
        mHSV = mRGB;
        break;
    case MODE_BLUE:
        mRGB.blue = GetRGBColorFromRangeValue(row, iYrange, 255, true);
        mHSV = mRGB;
        break;
    }
    render();
}

void xlColorCanvas::ProcessPaletteClick(int row, int column) {
    float dYrange = mWindowHeight - 1;
    float dXrange = mWindowWidth - 1;
    int iYrange = mWindowHeight - 1;
    int iXrange = mWindowWidth - 1;

    if (row < 0)
        row = 0;
    else if (row > iYrange)
        row = iYrange;
    if (column < 0)
        column = 0;
    else if (column > iXrange)
        column = iXrange;
    switch (mDisplayMode) {
    case MODE_HUE:
        mHSV.saturation = (double)column / dXrange;
        mHSV.value = 1.0 - (double)row / dYrange;
        mRGB = mHSV;
        break;
    case MODE_SATURATION:
        mHSV.hue = (double)column / dXrange;
        mHSV.value = 1.0 - (double)row / dYrange;
        mRGB = mHSV;
        break;
    case MODE_BRIGHTNESS:
        mHSV.hue = (double)column / dXrange;
        mHSV.saturation = 1.0 - (double)row / dYrange;
        mRGB = mHSV;
        break;
    case MODE_RED:
        mRGB.green = GetRGBColorFromRangeValue(column, iXrange, 255, false);
        mRGB.blue = GetRGBColorFromRangeValue(row, iYrange, 255, true);
        mHSV = mRGB;
        break;
    case MODE_GREEN:
        mRGB.red = GetRGBColorFromRangeValue(column, iXrange, 255, false);
        mRGB.blue = GetRGBColorFromRangeValue(row, iYrange, 255, true);
        mHSV = mRGB;
        break;
    case MODE_BLUE:
        mRGB.red = GetRGBColorFromRangeValue(column, iXrange, 255, false);
        mRGB.green = GetRGBColorFromRangeValue(row, iYrange, 255, true);
        mHSV = mRGB;
        break;
    }
    render();
}

void xlColorCanvas::render(wxPaintEvent& event) {
    wxPaintDC dc(this);
    render();
}

void xlColorCanvas::render() {
    if (!IsShownOnScreen()) {
        return;
    }
    if (!mIsInitialized) {
        PrepareCanvas();
    }

    xlGraphicsContext* ctx = PrepareContextForDrawing();
    if (ctx == nullptr) {
        return;
    }
    ctx->SetViewport(0, 0, mWindowWidth, mWindowHeight);

    if (mDisplayType == xlColorCanvas::DisplayType::TYPE_SLIDER) {
        DrawSlider(ctx);
    } else if (mDisplayType == xlColorCanvas::DisplayType::TYPE_WHEEL) {
        DrawWheel(ctx);
    } else {
        DrawPalette(ctx);
    }
    FinishDrawing(ctx);
}

inline void setSixVertices(xlVertexColorAccumulator* vac, const xlColor& c, uint32_t& idx) {
    for (int x = 0; x < 6; x++) {
        vac->SetVertex(idx++, c);
    }
}

void xlColorCanvas::DrawPalette(xlGraphicsContext* ctx) {
    float dYrange = mWindowHeight - 1;
    float dXrange = mWindowWidth - 1;
    int iYrange = mWindowHeight - 1;
    int iXrange = mWindowWidth - 1;

    HSVValue hsv;
    xlColor color;
    double focus_row = 0;
    double focus_col = 0;

    unsigned int vcount = (iXrange + 1) * (iYrange + 1) * 6;
    if (background == nullptr || background->getCount() != vcount) {
        if (background) {
            delete background;
        }
        background = ctx->createVertexColorAccumulator();
        background->PreAlloc(vcount);

        //create the rectangles
        for (int col = 0; col <= iXrange; col++) {
            for (int row = 0; row <= iYrange; row++) {
                background->AddRectAsTriangles(col, row, col + 1, row + 1, xlBLACK);
            }
        }
        //rectangles are created, we can finalize the vertices, colors will change
        background->Finalize(false, true);
    }

    uint32_t idx = 0;
    switch (mDisplayMode) {
    case MODE_HUE:
        hsv.hue = mHSV.hue;
        for (int col = 0; col <= iXrange; col++) {
            hsv.saturation = (double)col / dXrange;
            for (int row = 0; row <= iYrange; row++) {
                hsv.value = (1.0 - (double)row / dYrange);
                color = hsv;
                setSixVertices(background, color, idx);
            }
        }
        focus_col = mHSV.saturation * dXrange;
        focus_row = dYrange - mHSV.value * dYrange;
        break;
    case MODE_SATURATION:
        hsv.saturation = mHSV.saturation;
        for (int col = 0; col <= iXrange; col++) {
            hsv.hue = (double)col / dXrange;
            for (int row = 0; row <= iYrange; row++) {
                hsv.value = (1.0 - (double)row / dYrange);
                color = hsv;
                setSixVertices(background, color, idx);
            }
        }
        focus_col = mHSV.hue * dXrange;
        focus_row = dYrange - mHSV.value * dYrange;
        break;
    case MODE_BRIGHTNESS:
        hsv.value = mHSV.value;
        for (int col = 0; col <= iXrange; col++) {
            hsv.hue = (double)col / dXrange;
            for (int row = 0; row <= iYrange; row++) {
                hsv.saturation = (1.0 - (double)row / dYrange);
                color = hsv;
                setSixVertices(background, color, idx);
            }
        }
        focus_col = mHSV.hue * dXrange;
        focus_row = dYrange - mHSV.saturation * dYrange;
        break;
    case MODE_RED:
        color.red = mRGB.red;
        for (int col = 0; col <= iXrange; col++) {
            color.green = GetRGBColorFromRangeValue(col, iXrange, 255, false);
            for (int row = 0; row <= iYrange; row++) {
                color.blue = GetRGBColorFromRangeValue(row, iYrange, 255, true);
                setSixVertices(background, color, idx);
            }
        }
        focus_col = ((double)mRGB.green / 255.0) * dXrange;
        focus_row = dXrange - ((double)mRGB.blue / 255.0) * dXrange;
        break;
    case MODE_GREEN:
        color.green = mRGB.green;
        for (int col = 0; col <= iXrange; col++) {
            color.red = GetRGBColorFromRangeValue(col, iXrange, 255, false);
            for (int row = 0; row <= iYrange; row++) {
                color.blue = GetRGBColorFromRangeValue(row, iYrange, 255, true);
                setSixVertices(background, color, idx);
            }
        }
        focus_col = ((double)mRGB.red / 255.0) * dXrange;
        focus_row = dXrange - ((double)mRGB.blue / 255.0) * dXrange;
        break;
    case MODE_BLUE:
        color.blue = mRGB.blue;
        for (int col = 0; col <= iXrange; col++) {
            color.red = GetRGBColorFromRangeValue(col, iXrange, 255, false);
            for (int row = 0; row <= iYrange; row++) {
                color.green = GetRGBColorFromRangeValue(row, iYrange, 255, true);
                setSixVertices(background, color, idx);
            }
        }
        focus_col = ((double)mRGB.red / 255.0) * dXrange;
        focus_row = dXrange - ((double)mRGB.green / 255.0) * dXrange;
        break;
    }
    background->FlushRange(0, vcount);
    ctx->drawTriangles(background);

    double radius = std::max(4.0, dXrange / 40.0);

    auto* va2 = ctx->createVertexAccumulator();
    va2->AddCircleAsLines(focus_col, focus_row, radius);
    if (mHSV.value > 0.6) {
        ctx->drawLineStrip(va2, xlBLACK);
    } else {
        ctx->drawLineStrip(va2, xlWHITE);
    }
    delete va2;
}

void xlColorCanvas::ProcessWheelClick(int row, int col) {
    static const double TWO_PI = 2.0 * std::acos(-1.0);
    static const double PI     = std::acos(-1.0);

    float cx     = mWindowWidth  / 2.0f;
    float cy     = mWindowHeight / 2.0f;
    float radius = std::min(cx, cy) * 0.95f;

    float dx = col - cx;
    float dy = row - cy;
    float r  = std::sqrt(dx * dx + dy * dy);

    double hue = (std::atan2((double)dy, (double)-dx) + PI) / TWO_PI;
    double sat = (double)(r / radius);
    mHSV.hue        = std::max(0.0, std::min(1.0, hue));
    mHSV.saturation = std::max(0.0, std::min(1.0, sat));
    mRGB = mHSV;
    render();
}

void xlColorCanvas::DrawWheel(xlGraphicsContext* ctx) {
    // Full HSV disc: angle = hue, radius = saturation, brightness = mHSV.value
    static const double TWO_PI = 2.0 * std::acos(-1.0);
    static const double PI     = std::acos(-1.0);

    int iYrange = (int)mWindowHeight - 1;
    int iXrange = (int)mWindowWidth  - 1;

    float cx     = mWindowWidth  / 2.0f;
    float cy     = mWindowHeight / 2.0f;
    float radius = std::min(cx, cy) * 0.95f;

    unsigned int vcount = (unsigned int)(iXrange + 1) * (iYrange + 1) * 6;
    // Invalidate on ANY size change (not just shrink) — a grown window would
    // otherwise overrun a too-small buffer and corrupt the render.
    if (background == nullptr || background->getCount() != vcount) {
        if (background) delete background;
        background = ctx->createVertexColorAccumulator();
        background->PreAlloc(vcount);
        for (int col = 0; col <= iXrange; col++) {
            for (int row = 0; row <= iYrange; row++) {
                background->AddRectAsTriangles(col, row, col + 1, row + 1, xlBLACK);
            }
        }
        background->Finalize(false, true);
    }

    static const xlColor BG(48, 48, 48);
    uint32_t idx = 0;

    for (int col = 0; col <= iXrange; col++) {
        float dx = (float)col - cx;
        for (int row = 0; row <= iYrange; row++) {
            float dy = (float)row - cy;
            float r  = std::sqrt(dx * dx + dy * dy);
            xlColor color;
            if (r <= radius) {
                double hue = (std::atan2((double)dy, (double)-dx) + PI) / TWO_PI;
                double sat = (double)(r / radius);
                HSVValue hsv(hue, sat, mHSV.value);
                color = hsv;
            } else {
                color = BG;
            }
            setSixVertices(background, color, idx);
        }
    }
    background->FlushRange(0, vcount);
    ctx->drawTriangles(background);

    // Selection indicator: small square at current hue/sat position
    float angle      = (float)(mHSV.hue * TWO_PI - PI);
    float indicatorR = (float)(mHSV.saturation * radius);
    float ix = cx - indicatorR * std::cos(angle);
    float iy = cy + indicatorR * std::sin(angle);

    // Dual-contrast handle: a black outline one pixel outside a white outline so the marker stays
    // visible against any underlying color (light or dark) without relying on a brightness heuristic.
    auto drawSquare = [&](float s, const xlColor& c) {
        auto* va = ctx->createVertexAccumulator();
        va->AddVertex(ix - s, iy - s);
        va->AddVertex(ix + s, iy - s);
        va->AddVertex(ix + s, iy + s);
        va->AddVertex(ix - s, iy + s);
        va->AddVertex(ix - s, iy - s);   // close
        ctx->drawLineStrip(va, c);
        delete va;
    };
    drawSquare(6.0f, xlBLACK);
    drawSquare(5.0f, xlWHITE);
}

void xlColorCanvas::DrawSlider(xlGraphicsContext* ctx) {
    HSVValue hsv;
    xlColor color;

    switch (mDisplayMode) {
    case MODE_HUE:
        hsv.saturation = 1.0;
        hsv.value = 1.0;
        break;
    case MODE_SATURATION:
        hsv.hue = mHSV.hue;
        hsv.value = mHSV.value;
        break;
    case MODE_BRIGHTNESS:
        hsv.saturation = mHSV.saturation;
        hsv.hue = mHSV.hue;
        break;
    case MODE_RED:
        color.green = mRGB.green;
        color.blue = mRGB.blue;
        break;
    case MODE_GREEN:
        color.red = mRGB.red;
        color.blue = mRGB.blue;
        break;
    case MODE_BLUE:
        color.red = mRGB.red;
        color.green = mRGB.green;
        break;
    }

    float dYrange = mWindowHeight - 1;
    int iYrange = mWindowHeight - 1;

    auto* va = ctx->createVertexColorAccumulator();
    va->PreAlloc((iYrange + 1) * 6);
    for (int row = 0; row <= iYrange; row++) {
        switch (mDisplayMode) {
        case MODE_HUE:
            hsv.hue = (1.0 - (double)row / dYrange);
            color = hsv;
            break;
        case MODE_SATURATION:
            hsv.saturation = (1.0 - (double)row / dYrange);
            color = hsv;
            break;
        case MODE_BRIGHTNESS:
            hsv.value = (1.0 - (double)row / dYrange);
            color = hsv;
            break;
        case MODE_RED:
            color.red = GetRGBColorFromRangeValue(row, iYrange, 255, true);
            break;
        case MODE_GREEN:
            color.green = GetRGBColorFromRangeValue(row, iYrange, 255, true);
            break;
        case MODE_BLUE:
            color.blue = GetRGBColorFromRangeValue(row, iYrange, 255, true);
            break;
        }
        va->AddRectAsTriangles(0, row, mWindowWidth - 1, row + 1, color);
    }
    ctx->drawTriangles(va);
    delete va;

    // Horizontal position indicator
    float indicatorY = 0;
    switch (mDisplayMode) {
    case MODE_HUE:        indicatorY = (float)((1.0 - mHSV.hue)        * (mWindowHeight - 1)); break;
    case MODE_SATURATION: indicatorY = (float)((1.0 - mHSV.saturation) * (mWindowHeight - 1)); break;
    case MODE_BRIGHTNESS: indicatorY = (float)((1.0 - mHSV.value)      * (mWindowHeight - 1)); break;
    case MODE_RED:        indicatorY = (float)((1.0 - mRGB.red   / 255.0) * (mWindowHeight - 1)); break;
    case MODE_GREEN:      indicatorY = (float)((1.0 - mRGB.green / 255.0) * (mWindowHeight - 1)); break;
    case MODE_BLUE:       indicatorY = (float)((1.0 - mRGB.blue  / 255.0) * (mWindowHeight - 1)); break;
    }
    auto* ind = ctx->createVertexAccumulator();
    ind->AddVertex(0,              indicatorY);
    ind->AddVertex(mWindowWidth,   indicatorY);
    // second line one pixel below for a 2-pixel thick marker
    ind->AddVertex(0,              indicatorY + 1);
    ind->AddVertex(mWindowWidth,   indicatorY + 1);
    xlColor lineColor = (indicatorY < mWindowHeight * 0.5f) ? xlBLACK : xlWHITE;
    ctx->drawLineStrip(ind, lineColor);
    delete ind;
}

