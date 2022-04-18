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
#include "graphics/xlGraphicsBase.h"
#include "models/Model.h"

BEGIN_EVENT_TABLE(xlGridCanvas, GRAPHICS_BASE_CLASS)
END_EVENT_TABLE()

xlGridCanvas::xlGridCanvas(wxWindow* parent, wxWindowID id, const wxPoint &pos, const wxSize &size,long style, const wxString &name)
    : GRAPHICS_BASE_CLASS(parent, id, pos, size, style, name),
      mEffect(nullptr),
      mModel(nullptr),
      mGridlineColor(new xlColor(0,153,153)),
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
    float w = parent_size.GetWidth() - 15.0f;
    float h = parent_size.GetHeight() - 15.0f;

    float cell_height = (h - 10.0) / (float)mRows;
    float cell_width = (w - 10.0) / (float)mColumns;
    float cellSize = std::min(cell_height, cell_width);


    wxSize grid_size;
    w = cellSize * mColumns + 10;
    h = cellSize * mRows + 10;
    grid_size.SetWidth(std::max((int)w, 12));
    grid_size.SetHeight(std::max((int)h, 12));
    SetMinSize(grid_size);
    SetSize(grid_size);
}
float xlGridCanvas::getCellSize(bool logical) const {
    if (drawingUsingLogicalSize() || !logical) {
        // mWindow* are in absolute pixels
        float cell_height = float(mWindowHeight - mapLogicalToAbsolute(10.0f)) / (float)mRows ;
        float cell_width = float(mWindowWidth - mapLogicalToAbsolute(10.0f)) / (float)mColumns;
        return std::min(cell_height, cell_width);
    }

    float cell_height = float(float(mWindowHeight)/GetContentScaleFactor() - 10.0f) / (float)mRows ;
    float cell_width = float(float(mWindowWidth)/GetContentScaleFactor() - 10.0f) / (float)mRows ;
    return std::min(cell_height, cell_width);
}

float xlGridCanvas::calcCellFromPercent(int value, int base) const
{
    if (value == 100) return (base - 1);
    float band = 100.0 / (float)base;
    return ((float)value / band);
}
float xlGridCanvas::GetRowCenter(int percent, bool logical)
{
    float row = calcCellFromPercent(percent, mRows) + 0.5;
    return ((mRows - row) * getCellSize(logical)) + (logical ? 5.0 : mapLogicalToAbsolute(5.0));
}
float xlGridCanvas::GetColumnCenter(int percent, bool logical)
{
    float col = calcCellFromPercent(percent, mColumns) + 0.5;
    return (col * getCellSize(logical)) + (logical ? 5.0 : mapLogicalToAbsolute(5.0));
}

int xlGridCanvas::GetCellFromPosition(int position, bool logical) const
{
    float offset = (logical ? 5.0 : mapLogicalToAbsolute(5.0));
    float p = ((float)position) - offset;
    if (p < 0) {
        p = 0;
    }
    float result = (p / getCellSize());
    return result;
}
int xlGridCanvas::SetRowCenter(int position, bool logical)
{
    int row = GetCellFromPosition(position, logical);
    row = mRows - row - 1;
    float p = calcPercentFromCell(row, mRows, logical);
    if (mRows >= 100) {
        return std::round(p);
    }
    return std::ceil(p);
}
int xlGridCanvas::SetColumnCenter(int position, bool logical)
{
    int col = GetCellFromPosition(position, logical);
    float p = calcPercentFromCell(col, mColumns, logical);
    if (mColumns >= 100) {
        return std::round(p);
    }
    return std::ceil(p);
}


float xlGridCanvas::calcPercentFromCell(int value, int base, bool logical) const
{
    if (value <= 0) return 0;
    if (value >= (base-1)) return 100;
    if (base == 1) return 100;

    float f = 100.f / ((float)base);
    f *= value;
    return f;
}

void xlGridCanvas::DrawBaseGrid(xlGraphicsContext *ctx)
{
    auto *va = ctx->createVertexAccumulator();

    float cellSize = getCellSize();
    float height = cellSize * mRows;
    float width = cellSize * mColumns;
    float inset = mapLogicalToAbsolute(5.0) + 0.25;
    va->PreAlloc((mRows + mColumns + 2) * 2 * 3);

    float lwidth = mapLogicalToAbsolute(0.8);
    if (lwidth < 1.1)
        lwidth = 1.1;

    for (int row = 0; row <= mRows; row++ ) {
        float y = std::round(cellSize * row + inset) - lwidth / 2.0;
        va->AddVertex(inset, y);
        va->AddVertex(inset + width, y);
        va->AddVertex(inset, y + lwidth);
        va->AddVertex(inset + width, y);
        va->AddVertex(inset, y + lwidth);
        va->AddVertex(inset + width, y + lwidth);
    }
    for(int col = 0; col <= mColumns; col++ ) {
        float x = std::round(cellSize * col + inset) - lwidth / 2.0;
        va->AddVertex(x, inset);
        va->AddVertex(x, inset + height);
        va->AddVertex(x + lwidth, inset);
        va->AddVertex(x, inset + height);
        va->AddVertex(x + lwidth, inset);
        va->AddVertex(x + lwidth, inset + height);
    }

    // use triangles as lines on retina displays are VERY thin, almost invisible
    ctx->drawTriangles(va, *mGridlineColor);
    delete va;
}

