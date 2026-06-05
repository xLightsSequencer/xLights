#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/wx.h>
#include <wx/grid.h>
#include <wx/dcmemory.h>
#include <wx/renderer.h>

class wxModelGridCellRenderer : public wxGridCellStringRenderer
{
public:
    wxModelGridCellRenderer(wxImage* image_, wxGrid& grid);
    virtual ~wxModelGridCellRenderer() {}

    virtual void Draw(wxGrid &grid, wxGridCellAttr &attr, wxDC &dc, const wxRect &rect, int row, int col, bool isSelected) wxOVERRIDE;

    void UpdateSize(wxGrid& grid, bool draw_picture_, int lightness_, float offset_x_ = 0.0f, float offset_y_ = 0.0f);
    void CreateImage();
    void SetImage(wxImage* image);
    void DetermineGridSize(wxGrid& grid);

private:
    wxImage* image;
    wxBitmap bmp;
    wxMemoryDC bmpDC;
    int width;
    int height;
    int cell_w = 1;
    int cell_h = 1;
    bool draw_picture;
    int lightness;
    float offset_x = 0.0f;
    float offset_y = 0.0f;
};
