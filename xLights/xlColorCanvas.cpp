#include "xlColorCanvas.h"
#include "DrawGLUtils.h"
#include <algorithm>

BEGIN_EVENT_TABLE(xlColorCanvas, xlGLCanvas)
EVT_PAINT(xlColorCanvas::render)
EVT_MOTION(xlColorCanvas::mouseMoved)
EVT_LEFT_DOWN(xlColorCanvas::mouseDown)
EVT_LEFT_UP(xlColorCanvas::mouseReleased)
EVT_SIZE(xlColorCanvas::OnCanvasResize)
END_EVENT_TABLE()

#define CORNER_NOT_SELECTED     0
#define CORNER_1A_SELECTED      1
#define CORNER_1B_SELECTED      2
#define CORNER_2A_SELECTED      3
#define CORNER_2B_SELECTED      4

xlColorCanvas::xlColorCanvas(wxWindow* parent, wxWindowID id, const wxPoint &pos, const wxSize &size,long style, const wxString &name)
    : xlGLCanvas(parent, id, pos, size, style, name, true),
      mDisplayType(TYPE_PALETTE),
      mDisplayMode(MODE_SATURATION),
      mDragging(false),
      mHSV(0.0,1.0,1.0),
      mRGB(255,0,0)
{
}

xlColorCanvas::~xlColorCanvas()
{
}
HSVValue &xlColorCanvas::GetHSV()
{
    return mHSV;
}

void xlColorCanvas::OnCanvasResize(wxSizeEvent& evt)
{
    iXrange = evt.GetSize().GetWidth()-1;
    iYrange = evt.GetSize().GetHeight()-1;
    dXrange = (double)iXrange;
    dYrange = (double)iYrange;
    evt.Skip();
}

void xlColorCanvas::SetMode( ColorDisplayMode mode )
{
    mDisplayMode = mode;
}

void xlColorCanvas::SetHSV(const HSVValue &hsv)
{
    mHSV = hsv;
    mRGB = hsv;
    Refresh(false);
    Update();
}

void xlColorCanvas::SetRGB( xlColor rgb)
{
    mRGB = rgb;
    rgb.toHSV(mHSV);
    Refresh(false);
    Update();
}

void xlColorCanvas::mouseDown(wxMouseEvent& event)
{
    mDragging = true;
    if( mDisplayType == TYPE_SLIDER )
    {
        ProcessSliderClick(event.GetY());
        wxCommandEvent eventColor(EVT_CP_SLIDER_CHANGED);
        eventColor.SetClientData(&mHSV);
        wxPostEvent(GetParent(), eventColor);
    }
    else if( mDisplayType == TYPE_PALETTE )
    {
        ProcessPaletteClick(event.GetY(), event.GetX());
        wxCommandEvent eventColor(EVT_CP_PALETTE_CHANGED);
        eventColor.SetClientData(&mHSV);
        wxPostEvent(GetParent(), eventColor);
    }
    CaptureMouse();
    Refresh(false);
}

void xlColorCanvas::mouseMoved(wxMouseEvent& event)
{
    if( mDragging )
    {
        if( mDisplayType == TYPE_SLIDER )
        {
            ProcessSliderClick(event.GetY());
            wxCommandEvent eventColor(EVT_CP_SLIDER_CHANGED);
            eventColor.SetClientData(&mHSV);
            wxPostEvent(GetParent(), eventColor);
        }
        else if( mDisplayType == TYPE_PALETTE )
        {
            ProcessPaletteClick(event.GetY(), event.GetX());
            wxCommandEvent eventColor(EVT_CP_PALETTE_CHANGED);
            eventColor.SetClientData(&mHSV);
            wxPostEvent(GetParent(), eventColor);
        }
    }
}

void xlColorCanvas::mouseReleased(wxMouseEvent& event)
{
    if( mDragging )
    {
        ReleaseMouse();
        mDragging = false;
    }
}

int xlColorCanvas::GetRGBColorFromRangeValue( int position, int range, int max_value, bool invert )
{
    if( invert )
    {
        return (int)(max_value * (1.0 - (double)position/(double)range));
    }
    else
    {
        return (int)(max_value * (double)position/(double)range);
    }
}

void xlColorCanvas::ProcessSliderClick( int row )
{
    if( row < 0 ) row = 0;
    else if( row > iYrange ) row = iYrange;
    switch( mDisplayMode )
    {
    case MODE_HUE:
        mHSV.hue = 1.0 - (double)row/dYrange;
        mRGB = mHSV;
        break;
    case MODE_SATURATION:
        mHSV.saturation = 1.0 - (double)row/dYrange;
        mRGB = mHSV;
        break;
    case MODE_BRIGHTNESS:
        mHSV.value = 1.0 - (double)row/dYrange;
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
}

void xlColorCanvas::ProcessPaletteClick( int row, int column )
{
    if( row < 0 ) row = 0;
    else if( row > iYrange ) row = iYrange;
    if( column < 0 ) column = 0;
    else if( column > iXrange ) column = iXrange;
    switch( mDisplayMode )
    {
    case MODE_HUE:
        mHSV.saturation = (double)column/dXrange;
        mHSV.value = 1.0 - (double)row/dYrange;
        mRGB = mHSV;
        break;
    case MODE_SATURATION:
        mHSV.hue = (double)column/dXrange;
        mHSV.value = 1.0 - (double)row/dYrange;
        mRGB = mHSV;
        break;
    case MODE_BRIGHTNESS:
        mHSV.hue = (double)column/dXrange;
        mHSV.saturation = 1.0 - (double)row/dYrange;
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
}

void xlColorCanvas::InitializeGLCanvas()
{
#ifdef __LINUX__
    if(!IsShownOnScreen()) return;
#endif
    SetCurrentGLContext();

    LOG_GL_ERRORV(glClearColor(0.0f, 0.0f, 0.0f, 0.0f)); // Black Background
    LOG_GL_ERRORV(glEnable(GL_BLEND));
    LOG_GL_ERRORV(glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA));
    LOG_GL_ERRORV(glClear(GL_COLOR_BUFFER_BIT));
    prepare2DViewport(0,0,mWindowWidth, mWindowHeight);
    mIsInitialized = true;
}

void xlColorCanvas::render( wxPaintEvent& event )
{
    if(!mIsInitialized) { InitializeGLCanvas(); }
#ifdef __LINUX__
    if(!IsShownOnScreen()) return;
#endif

    SetCurrentGLContext();

    LOG_GL_ERRORV(glClear(GL_COLOR_BUFFER_BIT));
    prepare2DViewport(0,0,mWindowWidth, mWindowHeight);
    
    if( mDisplayType == TYPE_SLIDER )
    {
        DrawSlider();
    }
    else
    {
        DrawPalette();
    }

    SwapBuffers();
}

void xlColorCanvas::DrawPalette()
{
    HSVValue hsv;
    xlColor color;
    double focus_row = 0;
    double focus_col = 0;
    
    DrawGLUtils::PreAlloc((iXrange + 1) * (iYrange + 1) * 6);

    switch( mDisplayMode )
    {
    case MODE_HUE:
        hsv.hue = mHSV.hue;
        for( int col = 0; col <= iXrange; col++ )
        {
            hsv.saturation = (double)col/dXrange;
            for( int row = 0; row <= iYrange; row++ )
            {
                hsv.value = (1.0 - (double)row/dYrange);
                color = hsv;
                DrawGLUtils::AddRectAsTriangles(col, row, col + 1, row + 1, color, 0);
            }
        }
        focus_col = mHSV.saturation * dXrange;
        focus_row = dYrange - mHSV.value * dYrange;
        break;
    case MODE_SATURATION:
        hsv.saturation = mHSV.saturation;
        for( int col = 0; col <= iXrange; col++ )
        {
            hsv.hue = (double)col/dXrange;
            for( int row = 0; row <= iYrange; row++ )
            {
                hsv.value = (1.0 - (double)row/dYrange);
                color = hsv;
                DrawGLUtils::AddRectAsTriangles(col, row, col + 1, row + 1, color, 0);
            }
        }
        focus_col = mHSV.hue * dXrange;
        focus_row = dYrange - mHSV.value * dYrange;
        break;
    case MODE_BRIGHTNESS:
        hsv.value = mHSV.value;
        for( int col = 0; col <= iXrange; col++ )
        {
            hsv.hue = (double)col/dXrange;
            for( int row = 0; row <= iYrange; row++ )
            {
                hsv.saturation = (1.0 - (double)row/dYrange);
                color = hsv;
                DrawGLUtils::AddRectAsTriangles(col, row, col + 1, row + 1, color, 0);
            }
        }
        focus_col = mHSV.hue * dXrange;
        focus_row = dYrange - mHSV.saturation * dYrange;
        break;
    case MODE_RED:
        color.red = mRGB.red;
        for( int col = 0; col <= iXrange; col++ )
        {
            color.green = GetRGBColorFromRangeValue(col, iXrange, 255, false);
            for( int row = 0; row <= iYrange; row++ )
            {
                color.blue = GetRGBColorFromRangeValue(row, iYrange, 255, true);
                DrawGLUtils::AddRectAsTriangles(col, row, col + 1, row + 1, color, 0);
            }
        }
        focus_col = ((double)mRGB.green / 255.0) * dXrange;
        focus_row = dXrange - ((double)mRGB.blue / 255.0) * dXrange;
        break;
    case MODE_GREEN:
        color.green = mRGB.green;
        for( int col = 0; col <= iXrange; col++ )
        {
            color.red = GetRGBColorFromRangeValue(col, iXrange, 255, false);
            for( int row = 0; row <= iYrange; row++ )
            {
                color.blue = GetRGBColorFromRangeValue(row, iYrange, 255, true);
                DrawGLUtils::AddRectAsTriangles(col, row, col + 1, row + 1, color, 0);
            }
        }
        focus_col = ((double)mRGB.red / 255.0) * dXrange;
        focus_row = dXrange - ((double)mRGB.blue / 255.0) * dXrange;
        break;
    case MODE_BLUE:
        color.blue = mRGB.blue;
        for( int col = 0; col <= iXrange; col++ )
        {
            color.red = GetRGBColorFromRangeValue(col, iXrange, 255, false);
            for( int row = 0; row <= iYrange; row++ )
            {
                color.green = GetRGBColorFromRangeValue(row, iYrange, 255, true);
                DrawGLUtils::AddRectAsTriangles(col, row, col + 1, row + 1, color, 0);
            }
        }
        focus_col = ((double)mRGB.red / 255.0) * dXrange;
        focus_row = dXrange - ((double)mRGB.green / 255.0) * dXrange;
        break;
    }
    DrawGLUtils::End(GL_TRIANGLES);
    
    double radius = std::max(4.0, dXrange / 40.0);
    if( mHSV.value > 0.6 )
    {
        DrawGLUtils::DrawCircleUnfilled( xlWHITE, focus_col, focus_row, radius, 4.0 );
        DrawGLUtils::DrawCircleUnfilled( xlBLACK, focus_col, focus_row, radius, 2.0 );
    }
    else
    {
        DrawGLUtils::DrawCircleUnfilled( xlWHITE, focus_col, focus_row, radius, 2.0 );
    }
}

void xlColorCanvas::DrawSlider()
{
    HSVValue hsv;
    xlColor color;
    
    switch( mDisplayMode )
    {
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
    
    DrawGLUtils::PreAlloc((iYrange +1 ) * 6);
    for( int row = 0; row <= iYrange; row++ )
    {
        switch( mDisplayMode )
        {
            case MODE_HUE:
                hsv.hue = (1.0 - (double)row/dYrange);
                color = hsv;
                break;
            case MODE_SATURATION:
                hsv.saturation = (1.0 - (double)row/dYrange);
                color = hsv;
                break;
            case MODE_BRIGHTNESS:
                hsv.value = (1.0 - (double)row/dYrange);
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
        DrawGLUtils::AddRectAsTriangles(0, row, mWindowWidth-1, row + 1, color, 0);
    }
    DrawGLUtils::End(GL_TRIANGLES);
}

