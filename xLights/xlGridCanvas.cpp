#include "xlGridCanvas.h"
#include "BitmapCache.h"
#include "DrawGLUtils.h"
#include "ModelClass.h"

BEGIN_EVENT_TABLE(xlGridCanvas, xlGLCanvas)
END_EVENT_TABLE()

xlGridCanvas::xlGridCanvas(wxWindow* parent, wxWindowID id, const wxPoint &pos, const wxSize &size,long style, const wxString &name)
    : xlGLCanvas(parent, id, pos, size, style, name),
      mEffect(nullptr),
      mModelClass(nullptr),
      mGridlineColor(new xlColor(0,153,153)),
      mCellSize(20),
      mColumns(0),
      mRows(0),
      mDragging(false)
{
}

xlGridCanvas::~xlGridCanvas()
{
    delete mGridlineColor;
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

int xlGridCanvas::calcCellFromPercent(int value, int base)
{
    if( value == 100 ) return (base - 1);
    double band = 100.0 / (double)base;
    return (int)((double)value / band);
}

int xlGridCanvas::calcPercentFromCell(int value, int base)
{
    if( value == 0 ) return 0;
    if( value == base-1 ) return 100;
    double band = 100.0 / (double)base;
    return (int)(((double)value + 0.5) * band);
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

