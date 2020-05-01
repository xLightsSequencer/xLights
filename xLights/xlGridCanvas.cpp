/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "xlGridCanvas.h"
#include "DrawGLUtils.h"
#include "models/Model.h"

BEGIN_EVENT_TABLE(xlGridCanvas, xlGLCanvas)
END_EVENT_TABLE()

xlGridCanvas::xlGridCanvas(wxWindow* parent, wxWindowID id, const wxPoint &pos, const wxSize &size,long style, const wxString &name)
    : xlGLCanvas(parent, id, pos, size, style, name),
      mEffect(nullptr),
      mModel(nullptr),
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
    return ((mRows-row-1)* mCellSize) + (1.5 * mCellSize);
}

int xlGridCanvas::GetColumnCenter(int percent)
{
    int col = calcCellFromPercent(percent, mColumns);
    return (col * mCellSize) + (1.5 * mCellSize);
}

int xlGridCanvas::GetCellFromPosition(int position) const
{
    return (position / mCellSize) - 1;
}

int xlGridCanvas::SetRowCenter(int position)
{
    int row = GetCellFromPosition(position);
    row = mRows - row - 1;
    return calcPercentFromCell(row, mRows);
}

int xlGridCanvas::SetColumnCenter(int position)
{
    int col = GetCellFromPosition(position);
    return calcPercentFromCell(col, mColumns);
}

int xlGridCanvas::calcCellFromPercent(int value, int base) const
{
    if( value == 100 ) return (base - 1);
    double band = 100.0 / (double)base;
    return (int)((double)value / band);
}

int xlGridCanvas::calcPercentFromCell(int value, int base) const
{
    if( value == 0 ) return 0;
    if( value == base-1 ) return 100;
    double band = 100.0 / (double)base;
    return (int)(((double)value + 0.5) * band);
}

void xlGridCanvas::DrawBaseGrid()
{
    //*mGridlineColor = xlCYAN;
    DrawGLUtils::xlVertexAccumulator va;
    
    int height = mCellSize * mRows;
    int width = mCellSize * mColumns;
    va.PreAlloc((mRows + mColumns + 2) * 2);
    for(int row = 0; row <= mRows; row++ )
    {
        int y = mCellSize*(row+1);
        va.AddVertex(mCellSize, y);
        va.AddVertex(width+mCellSize,y);
    }
    for(int col = 0; col <= mColumns; col++ )
    {
        int x = mCellSize*(col+1);
        va.AddVertex(x,mCellSize);
        va.AddVertex(x,height+mCellSize);
    }
    DrawGLUtils::SetLineWidth(0.5);
    DrawGLUtils::Draw(va, *mGridlineColor, GL_LINES, GL_BLEND);
}

