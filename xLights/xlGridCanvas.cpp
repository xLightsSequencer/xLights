#include "xlGridCanvas.h"
#include "BitmapCache.h"
#include "DrawGLUtils.h"
#include "ModelClass.h"

static int calcCellFromPercent(int value, int base)
{
    double band = 101.0 / (double)base;
    return (int)((double)value / band);
}

static int calcPercentFromCell(int value, int base)
{
    if( value == 0 ) return 0;
    if( value == base-1 ) return 100;
    double band = 101.0 / (double)base;
    return (int)(((double)value + 0.5) * band);
}

BEGIN_EVENT_TABLE(xlGridCanvas, xlGLCanvas)
EVT_PAINT(xlGridCanvas::render)
EVT_MOTION(xlGridCanvas::mouseMoved)
EVT_LEFT_DOWN(xlGridCanvas::mouseDown)
EVT_LEFT_UP(xlGridCanvas::mouseReleased)
END_EVENT_TABLE()

#define CORNER_NOT_SELECTED     0
#define CORNER_1A_SELECTED      1
#define CORNER_1B_SELECTED      2
#define CORNER_2A_SELECTED      3
#define CORNER_2B_SELECTED      4

xlGridCanvas::xlGridCanvas(wxWindow* parent, wxWindowID id, const wxPoint &pos, const wxSize &size,long style, const wxString &name)
    : xlGLCanvas(parent, id, pos, size, style, name),
      mEffect(nullptr),
      mModelClass(nullptr),
      mGridlineColor(new xlColor(0,153,153)),
      mCellSize(20),
      mColumns(0),
      mRows(0),
      x1a(0),
      x1b(0),
      x2a(0),
      x2b(0),
      y1a(0),
      y1b(0),
      y2a(0),
      y2b(0),
      mSelectedCorner(CORNER_NOT_SELECTED),
      mDragging(false)
{
}

xlGridCanvas::~xlGridCanvas()
{
    delete mGridlineColor;
}

int xlGridCanvas::CheckForCornerHit(int x, int y)
{
    int half = mCellSize/2;
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

void xlGridCanvas::mouseDown(wxMouseEvent& event)
{
    if( mEffect == NULL ) return;
    if( mEffect->GetEffectIndex() == BitmapCache::eff_MORPH )
    {
        mSelectedCorner = CheckForCornerHit(event.GetX(), event.GetY());
        if( mSelectedCorner != CORNER_NOT_SELECTED )
        {
            SetCursor(wxCURSOR_HAND);
            mDragging = true;
            CaptureMouse();
            Refresh(false);
        }
    }
}

void xlGridCanvas::mouseMoved(wxMouseEvent& event)
{
    if( mEffect == NULL ) return;
    if( mEffect->GetEffectIndex() == BitmapCache::eff_MORPH )
    {
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
    }
}

void xlGridCanvas::mouseReleased(wxMouseEvent& event)
{
    if( mEffect == NULL ) return;
    if( mDragging )
    {
        if( mEffect->GetEffectIndex() == BitmapCache::eff_MORPH )
        {
            StoreUpdatedMorphPositions();
            mSelectedCorner = CORNER_NOT_SELECTED;
        }
        ReleaseMouse();
        mDragging = false;
    }
}

void xlGridCanvas::UpdateSelectedMorphCorner(int x, int y)
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
    }
    else if( mSelectedCorner == CORNER_2B_SELECTED )
    {
        x2b = x_pos;
        y2b = y_pos;
    }
    Refresh(false);
}

void xlGridCanvas::StoreUpdatedMorphPositions()
{
    wxString settings = mEffect->GetSettingsAsString();
    wxArrayString all_settings = wxSplit(settings, ',');
    for( int s = 0; s < all_settings.size(); s++ )
    {
        wxArrayString parts = wxSplit(all_settings[s], '=');
        int percent = wxAtoi(parts[1]);
        if( parts[0] == "E_SLIDER_Morph_Start_X1" && mSelectedCorner == CORNER_1A_SELECTED ) {
            percent = SetColumnCenter(x1a);
        }
        else if( parts[0] == "E_SLIDER_Morph_Start_X2" && mSelectedCorner == CORNER_1B_SELECTED ) {
            percent = SetColumnCenter(x1b);
        }
        else if( parts[0] == "E_SLIDER_Morph_Start_Y1" && mSelectedCorner == CORNER_1A_SELECTED ) {
            percent = SetRowCenter(y1a);
        }
        else if( parts[0] == "E_SLIDER_Morph_Start_Y2" && mSelectedCorner == CORNER_1B_SELECTED ) {
            percent = SetRowCenter(y1b);
        }
        else if( parts[0] == "E_SLIDER_Morph_End_X1" && mSelectedCorner == CORNER_2A_SELECTED ) {
            percent = SetColumnCenter(x2a);
        }
        else if( parts[0] == "E_SLIDER_Morph_End_X2" && mSelectedCorner == CORNER_2B_SELECTED ) {
            percent = SetColumnCenter(x2b);
        }
        else if( parts[0] == "E_SLIDER_Morph_End_Y1" && mSelectedCorner == CORNER_2A_SELECTED ) {
            percent = SetRowCenter(y2a);
        }
        else if( parts[0] == "E_SLIDER_Morph_End_Y2" && mSelectedCorner == CORNER_2B_SELECTED ) {
            percent = SetRowCenter(y2b);
        }
        parts[1] = wxString::Format("%d", percent);
        all_settings[s] = wxJoin(parts, '=');
    }
    settings = wxJoin(all_settings, ',');
    mEffect->SetSettings(settings);

    wxCommandEvent eventEffectChanged(EVT_EFFECT_CHANGED);
    eventEffectChanged.SetClientData(mEffect);
    wxPostEvent(GetParent(), eventEffectChanged);
}

void xlGridCanvas::CreateCornerTextures()
{
    wxString tooltip;
    for( int i = 0; i < 4; i++ )
    {
        DrawGLUtils::CreateOrUpdateTexture(BitmapCache::GetCornerIcon(i, tooltip, 64, true),
                                           BitmapCache::GetCornerIcon(i, tooltip, 32, true),
                                           BitmapCache::GetCornerIcon(i, tooltip, 16, true),
                                           &mCornerTextures[i]);
    }
}

void xlGridCanvas::InitializeGLCanvas()
{
    if(!IsShownOnScreen()) return;
    SetCurrentGLContext();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // Black Background
    glDisable(GL_TEXTURE_2D);   // textures
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    prepare2DViewport(0,0,mWindowWidth, mWindowHeight);
    CreateCornerTextures();
    mIsInitialized = true;
}

void xlGridCanvas::AdjustSize(wxSize& parent_size)
{
    wxSize grid_size;
    grid_size.SetWidth(parent_size.GetWidth()-10);
    grid_size.SetHeight(parent_size.GetHeight()-10);
    int cell_height = grid_size.GetHeight() / (mRows + 2);
    int cell_width = grid_size.GetWidth() / (mColumns + 2);
    mCellSize = std::min( cell_height, cell_width );
    grid_size.SetWidth(mCellSize*(mColumns+2));
    grid_size.SetHeight(mCellSize*(mRows+2));
    SetMinSize(grid_size);
    SetSize(grid_size);
}

void xlGridCanvas::render( wxPaintEvent& event )
{
    if(!mIsInitialized) { InitializeGLCanvas(); }
    if(!IsShownOnScreen()) return;

    SetCurrentGLContext();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    wxPaintDC(this);
    if( mWindowResized )
    {
        prepare2DViewport(0,0,mWindowWidth, mWindowHeight);
    }

    if( mEffect != nullptr )
    {
        DrawBaseGrid();
        DrawEffect();
    }

    glFlush();
    SwapBuffers();
}

void xlGridCanvas::DrawBaseGrid()
{
    //*mGridlineColor = xlCYAN;

    int height = mCellSize * mRows;
    int width = mCellSize * mColumns;
    glEnable(GL_BLEND);
    for(int row = 0; row <= mRows; row++ )
    {
        int y = mCellSize*(row+1);
        DrawGLUtils::DrawLine(*mGridlineColor,255,mCellSize,y,width+mCellSize,y,0.5);
    }
    for(int col = 0; col <= mColumns; col++ )
    {
        int x = mCellSize*(col+1);
        DrawGLUtils::DrawLine(*mGridlineColor,255,x,mCellSize,x,height+mCellSize,0.5);
    }
    glDisable(GL_BLEND);
}

int xlGridCanvas::GetRowCenter(int percent)
{
    int row = calcCellFromPercent(percent, mRows);
    if( mModelClass->GetIsBtoT()) {
        return ((mRows-row-1)* mCellSize) + (1.5 * mCellSize);
    }
    else {
        return (row * mCellSize) + (1.5 * mCellSize);
    }
}

int xlGridCanvas::GetColumnCenter(int percent)
{
    int col = calcCellFromPercent(percent, mColumns);
    if( mModelClass->GetIsLtoR()) {
        return (col * mCellSize) + (1.5 * mCellSize);
    }
    else {
        return ((mColumns-col-1)* mCellSize) + (1.5 * mCellSize);
    }
}

int xlGridCanvas::GetCellFromPosition(int position)
{
    return (position / mCellSize) - 1;
}

int xlGridCanvas::SetRowCenter(int position)
{
    int row = GetCellFromPosition(position);

    if( mModelClass->GetIsBtoT()) {
        row = mRows - row - 1;
    }

    return calcPercentFromCell(row, mRows);
}

int xlGridCanvas::SetColumnCenter(int position)
{
    int col = GetCellFromPosition(position);

    if( !mModelClass->GetIsLtoR()) {
        col = mColumns - col - 1;
    }

    return calcPercentFromCell(col, mColumns);
}

void xlGridCanvas::UpdateMorphPositionsFromEffect()
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
    }
}

void xlGridCanvas::DrawEffect()
{
    if( mEffect->GetEffectIndex() == BitmapCache::eff_MORPH )
    {
        if( !mDragging )
        {
            UpdateMorphPositionsFromEffect();
        }
        xlColor yellowLine = xlYELLOW;
        xlColor redLine = xlRED;
        glEnable(GL_BLEND);
        DrawGLUtils::DrawLine(redLine,255,x1a,y1a,x1b,y1b,0.5);
        DrawGLUtils::DrawLine(redLine,255,x2a,y2a,x2b,y2b,0.5);
        DrawGLUtils::DrawLine(yellowLine,255,x1a,y1a,x2a,y2a,0.5);
        DrawGLUtils::DrawLine(yellowLine,255,x1b,y1b,x2b,y2b,0.5);
        glDisable(GL_BLEND);

        // draw the corners
        DrawGLUtils::DrawTexture(&mCornerTextures[3], x2b-mCellSize/2, y2b-mCellSize/2, x2b+mCellSize/2, y2b+mCellSize/2);
        DrawGLUtils::DrawTexture(&mCornerTextures[2], x2a-mCellSize/2, y2a-mCellSize/2, x2a+mCellSize/2, y2a+mCellSize/2);
        DrawGLUtils::DrawTexture(&mCornerTextures[1], x1b-mCellSize/2, y1b-mCellSize/2, x1b+mCellSize/2, y1b+mCellSize/2);
        DrawGLUtils::DrawTexture(&mCornerTextures[0], x1a-mCellSize/2, y1a-mCellSize/2, x1a+mCellSize/2, y1a+mCellSize/2);
    }
}

