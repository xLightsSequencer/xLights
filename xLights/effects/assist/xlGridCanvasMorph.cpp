#include "xlGridCanvasMorph.h"
#include "../../BitmapCache.h"
#include "../../DrawGLUtils.h"

#include "../../ValueCurveButton.h"
#include <log4cpp/Category.hh>

BEGIN_EVENT_TABLE(xlGridCanvasMorph, xlGridCanvas)
    EVT_PAINT(xlGridCanvasMorph::render)
    EVT_MOTION(xlGridCanvasMorph::mouseMoved)
    EVT_LEFT_DOWN(xlGridCanvasMorph::mouseLeftDown)
    EVT_LEFT_UP(xlGridCanvasMorph::mouseLeftUp)
    EVT_RIGHT_DOWN(xlGridCanvasMorph::mouseRightDown)
    EVT_RIGHT_UP(xlGridCanvasMorph::mouseRightUp)
END_EVENT_TABLE()

#define CORNER_NOT_SELECTED     0
#define CORNER_1A_SELECTED      1
#define CORNER_1B_SELECTED      2
#define CORNER_2A_SELECTED      3
#define CORNER_2B_SELECTED      4

xlGridCanvasMorph::xlGridCanvasMorph(wxWindow* parent, wxWindowID id, const wxPoint &pos, const wxSize &size,long style, const wxString &name)
    : xlGridCanvas(parent, id, pos, size, style, name),
      x1a(0),
      x1b(0),
      x2a(0),
      x2b(0),
      y1a(0),
      y1b(0),
      y2a(0),
      y2b(0),
      mSelectedCorner(CORNER_NOT_SELECTED),
      mMorphStartLinked(false),
      mMorphEndLinked(false)
{
}

xlGridCanvasMorph::~xlGridCanvasMorph()
{
}

void xlGridCanvasMorph::ForceRefresh()
{
}

void xlGridCanvasMorph::SetEffect(Effect* effect_)
{
    mEffect = effect_;
}

int xlGridCanvasMorph::CheckForCornerHit(int x, int y)
{
    int corner_size = std::max(mMinCornerSize, mCellSize);
    int half = corner_size/2;
    if( x > x1a-half && x < x1a + half && y > y1a-half && y < y1a + half)
    {
        return CORNER_1A_SELECTED;
    }
    if( x > x1b-half && x < x1b + half && y > y1b-half && y < y1b + half)
    {
        return CORNER_1B_SELECTED;
    }
    if( x > x2a-half && x < x2a + half && y > y2a-half && y < y2a + half)
    {
        return CORNER_2A_SELECTED;
    }
    if( x > x2b-half && x < x2b + half && y > y2b-half && y < y2b + half)
    {
        return CORNER_2B_SELECTED;
    }
    return CORNER_NOT_SELECTED;
}

void xlGridCanvasMorph::mouseLeftDown(wxMouseEvent& event)
{
    if( mEffect == nullptr ) return;
    mSelectedCorner = CheckForCornerHit(event.GetX(), event.GetY());
    if( mSelectedCorner == CORNER_NOT_SELECTED )
    {
        SetMorphCorner1a(event.GetX(), event.GetY());
        StoreUpdatedMorphPositions();
        SetMorphCorner1b(event.GetX(), event.GetY());
        StoreUpdatedMorphPositions();
        Update();
    }
    mDragging = true;
    CaptureMouse();
    SetCursor(wxCURSOR_HAND);
    Refresh(false);
}

void xlGridCanvasMorph::mouseRightDown(wxMouseEvent& event)
{
    if( mEffect == nullptr ) return;
    mSelectedCorner = CheckForCornerHit(event.GetX(), event.GetY());
    if( mSelectedCorner == CORNER_NOT_SELECTED )
    {
        SetMorphCorner2a(event.GetX(), event.GetY());
        StoreUpdatedMorphPositions();
        SetMorphCorner2b(event.GetX(), event.GetY());
        StoreUpdatedMorphPositions();
        Update();
    }
    mDragging = true;
    CaptureMouse();
    SetCursor(wxCURSOR_HAND);
    Refresh(false);
}

void xlGridCanvasMorph::mouseMoved(wxMouseEvent& event)
{
    if( mEffect == nullptr ) return;

    if( !mDragging )
    {
        if( CheckForCornerHit(event.GetX(), event.GetY()) == CORNER_NOT_SELECTED )
        {
            SetCursor(wxCURSOR_DEFAULT);
        }
        else
        {
            SetCursor(wxCURSOR_HAND);
        }
    }
    else
    {
        UpdateSelectedMorphCorner(event.GetX(), event.GetY());
        StoreUpdatedMorphPositions();
        Update();
    }

    SetTooltip(event.GetX(), event.GetY());
}

void xlGridCanvasMorph::mouseLeftUp(wxMouseEvent& event)
{
    SetTooltip(-1, -1);
    if( mEffect == nullptr ) return;
    if( mDragging )
    {
        StoreUpdatedMorphPositions();
        mSelectedCorner = CORNER_NOT_SELECTED;
        ReleaseMouse();
        mDragging = false;
    }
}

void xlGridCanvasMorph::mouseRightUp(wxMouseEvent& event)
{
    SetTooltip(-1, -1);
    if( mEffect == nullptr ) return;
    if( mDragging )
    {
        StoreUpdatedMorphPositions();
        mSelectedCorner = CORNER_NOT_SELECTED;
        ReleaseMouse();
        mDragging = false;
    }
}

void xlGridCanvasMorph::UpdateSelectedMorphCorner(int x, int y)
{
    if( x < mCellSize ) x = mCellSize;
    if( x > mColumns * mCellSize ) x = mColumns * mCellSize;
    if( y < mCellSize ) y = mCellSize;
    if( y > mRows * mCellSize ) y = mRows * mCellSize;

    int x_pos = (x / mCellSize) * mCellSize + mCellSize/2;
    int y_pos = (y / mCellSize) * mCellSize + mCellSize/2;

    if( mSelectedCorner == CORNER_1A_SELECTED )
    {
        x1a = x_pos;
        y1a = y_pos;
        if( mMorphStartLinked )
        {
            x1b = x_pos;
            y1b = y_pos;
        }
    }
    else if( mSelectedCorner == CORNER_1B_SELECTED )
    {
        x1b = x_pos;
        y1b = y_pos;
    }
    else if( mSelectedCorner == CORNER_2A_SELECTED )
    {
        x2a = x_pos;
        y2a = y_pos;
        if( mMorphEndLinked )
        {
            x2b = x_pos;
            y2b = y_pos;
        }
    }
    else if( mSelectedCorner == CORNER_2B_SELECTED )
    {
        x2b = x_pos;
        y2b = y_pos;
    }
    Refresh(false);
}

void xlGridCanvasMorph::SetMorphCorner1a(int x, int y)
{
    if( x < mCellSize ) x = mCellSize;
    if( x > mColumns * mCellSize ) x = mColumns * mCellSize;
    if( y < mCellSize ) y = mCellSize;
    if( y > mRows * mCellSize ) y = mRows * mCellSize;

    int x_pos = (x / mCellSize) * mCellSize + mCellSize/2;
    int y_pos = (y / mCellSize) * mCellSize + mCellSize/2;

    x1a = x_pos;
    y1a = y_pos;
    if( mMorphStartLinked )
    {
        x1b = x_pos;
        y1b = y_pos;
    }
    mSelectedCorner = CORNER_1A_SELECTED;
}

void xlGridCanvasMorph::SetMorphCorner1b(int x, int y)
{
    if( x < mCellSize ) x = mCellSize;
    if( x > mColumns * mCellSize ) x = mColumns * mCellSize;
    if( y < mCellSize ) y = mCellSize;
    if( y > mRows * mCellSize ) y = mRows * mCellSize;

    int x_pos = (x / mCellSize) * mCellSize + mCellSize/2;
    int y_pos = (y / mCellSize) * mCellSize + mCellSize/2;

    x1b = x_pos;
    y1b = y_pos;
    if( !mMorphStartLinked )
    {
        mSelectedCorner = CORNER_1B_SELECTED;
    }
}

void xlGridCanvasMorph::SetMorphCorner2a(int x, int y)
{
    if( x < mCellSize ) x = mCellSize;
    if( x > mColumns * mCellSize ) x = mColumns * mCellSize;
    if( y < mCellSize ) y = mCellSize;
    if( y > mRows * mCellSize ) y = mRows * mCellSize;

    int x_pos = (x / mCellSize) * mCellSize + mCellSize/2;
    int y_pos = (y / mCellSize) * mCellSize + mCellSize/2;

    x2a = x_pos;
    y2a = y_pos;
    if( mMorphEndLinked )
    {
        x2b = x_pos;
        y2b = y_pos;
    }
    mSelectedCorner = CORNER_2A_SELECTED;
}

void xlGridCanvasMorph::SetMorphCorner2b(int x, int y)
{
    if( x < mCellSize ) x = mCellSize;
    if( x > mColumns * mCellSize ) x = mColumns * mCellSize;
    if( y < mCellSize ) y = mCellSize;
    if( y > mRows * mCellSize ) y = mRows * mCellSize;

    int x_pos = (x / mCellSize) * mCellSize + mCellSize/2;
    int y_pos = (y / mCellSize) * mCellSize + mCellSize/2;

    x2b = x_pos;
    y2b = y_pos;
    if( !mMorphStartLinked )
    {
        mSelectedCorner = CORNER_2B_SELECTED;
    }
}

void DeactivateValueCurve(SettingsMap& settings, const std::string& setting)
{
    if (settings.Contains(setting))
    {
        wxString s = wxString(settings[setting]);
        s.Replace("Active=TRUE", "Active=FALSE");
        settings[setting] = s;
    }
}

void xlGridCanvasMorph::StoreUpdatedMorphPositions()
{
    SettingsMap& settings = mEffect->GetSettings();
    if( mSelectedCorner == CORNER_1A_SELECTED ) {
        settings["E_SLIDER_Morph_Start_X1"] = wxString::Format("%d", SetColumnCenter(x1a));
        settings["E_SLIDER_Morph_Start_Y1"] = wxString::Format("%d", SetRowCenter(y1a));
        DeactivateValueCurve(settings, "E_VALUECURVE_Morph_Start_X1");
        DeactivateValueCurve(settings, "E_VALUECURVE_Morph_Start_Y1");
    }
    else if( mSelectedCorner == CORNER_1B_SELECTED && !mMorphStartLinked ) {
        settings["E_SLIDER_Morph_Start_X2"] = wxString::Format("%d", SetColumnCenter(x1b));
        settings["E_SLIDER_Morph_Start_Y2"] = wxString::Format("%d", SetRowCenter(y1b));
        DeactivateValueCurve(settings, "E_VALUECURVE_Morph_Start_X2");
        DeactivateValueCurve(settings, "E_VALUECURVE_Morph_Start_Y2");
    }
    else if( mSelectedCorner == CORNER_2A_SELECTED ) {
        settings["E_SLIDER_Morph_End_X1"] = wxString::Format("%d", SetColumnCenter(x2a));
        settings["E_SLIDER_Morph_End_Y1"] = wxString::Format("%d", SetRowCenter(y2a));
        DeactivateValueCurve(settings, "E_VALUECURVE_Morph_End_X1");
        DeactivateValueCurve(settings, "E_VALUECURVE_Morph_End_Y1");
    }
    else if( mSelectedCorner == CORNER_2B_SELECTED && !mMorphEndLinked ) {
        settings["E_SLIDER_Morph_End_X2"] = wxString::Format("%d", SetColumnCenter(x2b));
        settings["E_SLIDER_Morph_End_Y2"] = wxString::Format("%d", SetRowCenter(y2b));
        DeactivateValueCurve(settings, "E_VALUECURVE_Morph_End_X2");
        DeactivateValueCurve(settings, "E_VALUECURVE_Morph_End_Y2");
    }

    wxCommandEvent eventVCChange(EVT_VC_CHANGED);
    eventVCChange.SetEventObject(this);
    wxPostEvent(GetParent(), eventVCChange);

    wxCommandEvent eventEffectChanged(EVT_EFFECT_CHANGED);
    eventEffectChanged.SetClientData(mEffect);
    wxPostEvent(GetParent(), eventEffectChanged);
}

void xlGridCanvasMorph::SetTooltip(int x, int y)
{
    if (x == -1 && y == -1)
    {
        UnsetToolTip();
        return;
    }

    if (x < mCellSize) x = mCellSize;
    if (x > mColumns * mCellSize) x = mColumns * mCellSize;
    if (y < mCellSize) y = mCellSize;
    if (y > mRows * mCellSize) y = mRows * mCellSize;

    x = x / mCellSize * 100 / mColumns;
    y = 100 - y / mCellSize * 100 / mRows;

    wxString tt = wxString::Format("%d%%, %d%%", x, y);
    SetToolTip(tt);
}

void xlGridCanvasMorph::CreateCornerTextures()
{
    wxString tooltip;
    for( int i = 0; i < 6; i++ )
    {
        DrawGLUtils::CreateOrUpdateTexture(BitmapCache::GetCornerIcon(i, tooltip, 64, true),
                                           BitmapCache::GetCornerIcon(i, tooltip, 32, true),
                                           BitmapCache::GetCornerIcon(i, tooltip, 16, true),
                                           &mCornerTextures[i]);
    }
}

void xlGridCanvasMorph::InitializeGLCanvas()
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
    CreateCornerTextures();
    mIsInitialized = true;
}

void xlGridCanvasMorph::render( wxPaintEvent& event )
{
    if(!mIsInitialized) { InitializeGLCanvas(); }
#ifdef __LINUX__
    if(!IsShownOnScreen()) return;
#endif


    SetCurrentGLContext();

    LOG_GL_ERRORV(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    prepare2DViewport(0,0,mWindowWidth, mWindowHeight);

    if( mEffect != nullptr )
    {

        DrawBaseGrid();
        DrawMorphEffect();
    }
    SwapBuffers();
}

void xlGridCanvasMorph::UpdateMorphPositionsFromEffect()
{
    wxString settings = mEffect->GetSettingsAsString();
    wxArrayString all_settings = wxSplit(settings, ',');
    for( int s = 0; s < all_settings.size(); s++ )
    {
        wxArrayString parts = wxSplit(all_settings[s], '=');
        if( parts[0] == "E_SLIDER_Morph_Start_X1" ) {
            x1a = GetColumnCenter(wxAtoi(parts[1]));
        }
        else if( parts[0] == "E_SLIDER_Morph_Start_X2" ) {
            x1b = GetColumnCenter(wxAtoi(parts[1]));
        }
        else if( parts[0] == "E_SLIDER_Morph_Start_Y1" ) {
            y1a = GetRowCenter(wxAtoi(parts[1]));
        }
        else if( parts[0] == "E_SLIDER_Morph_Start_Y2" ) {
            y1b = GetRowCenter(wxAtoi(parts[1]));
        }
        else if( parts[0] == "E_SLIDER_Morph_End_X1" ) {
            x2a = GetColumnCenter(wxAtoi(parts[1]));
        }
        else if( parts[0] == "E_SLIDER_Morph_End_X2" ) {
            x2b = GetColumnCenter(wxAtoi(parts[1]));
        }
        else if( parts[0] == "E_SLIDER_Morph_End_Y1" ) {
            y2a = GetRowCenter(wxAtoi(parts[1]));
        }
        else if( parts[0] == "E_SLIDER_Morph_End_Y2" ) {
            y2b = GetRowCenter(wxAtoi(parts[1]));
        }
        else if( parts[0] == "E_CHECKBOX_Morph_Start_Link" ) {
            mMorphStartLinked = wxAtoi(parts[1]) > 0;
        }
        else if( parts[0] == "E_CHECKBOX_Morph_End_Link" ) {
            mMorphEndLinked = wxAtoi(parts[1]) > 0;
        }
    }
    if( mMorphStartLinked )
    {
        x1b = x1a;
        y1b = y1a;
    }
    if( mMorphEndLinked )
    {
        x2b = x2a;
        y2b = y2a;
    }
}

void xlGridCanvasMorph::DrawMorphEffect()
{
    if( !mDragging )
    {
        UpdateMorphPositionsFromEffect();
    }
    xlColor yellowLine = xlYELLOW;
    xlColor redLine = xlRED;

    DrawGLUtils::xlVertexColorAccumulator va;
    va.PreAlloc(8);

    DrawGLUtils::SetLineWidth(0.5);
    if( !mMorphStartLinked )
    {
        va.AddVertex(x1a,y1a,redLine);
        va.AddVertex(x1b,y1b,redLine);
    }
    if( !mMorphEndLinked )
    {
        va.AddVertex(x2a,y2a,redLine);
        va.AddVertex(x2b,y2b,redLine);
    }
    va.AddVertex(x1a,y1a,yellowLine);
    va.AddVertex(x2a,y2a,yellowLine);
    va.AddVertex(x1b,y1b,yellowLine);
    va.AddVertex(x2b,y2b,yellowLine);
    LOG_GL_ERRORV(glEnable( GL_LINE_SMOOTH ));
    LOG_GL_ERRORV(glHint( GL_LINE_SMOOTH_HINT, GL_NICEST ));
    DrawGLUtils::Draw(va, GL_LINES, GL_BLEND);
    LOG_GL_ERRORV(glDisable( GL_LINE_SMOOTH ));

    // draw the corners
    int corner_size = std::max(mMinCornerSize, mCellSize);
    if( mMorphEndLinked )
    {
        DrawGLUtils::DrawTexture(mCornerTextures[5], x2a-corner_size/2, y2a-corner_size/2, x2a+corner_size/2, y2a+corner_size/2);
    }
    else
    {
        DrawGLUtils::DrawTexture(mCornerTextures[3], x2b-corner_size/2, y2b-corner_size/2, x2b+corner_size/2, y2b+corner_size/2);
        DrawGLUtils::DrawTexture(mCornerTextures[2], x2a-corner_size/2, y2a-corner_size/2, x2a+corner_size/2, y2a+corner_size/2);
    }
    if( mMorphStartLinked )
    {
        DrawGLUtils::DrawTexture(mCornerTextures[4], x1a-corner_size/2, y1a-corner_size/2, x1a+corner_size/2, y1a+corner_size/2);
    }
    else
    {
        DrawGLUtils::DrawTexture(mCornerTextures[1], x1b-corner_size/2, y1b-corner_size/2, x1b+corner_size/2, y1b+corner_size/2);
        DrawGLUtils::DrawTexture(mCornerTextures[0], x1a-corner_size/2, y1a-corner_size/2, x1a+corner_size/2, y1a+corner_size/2);
    }
}

