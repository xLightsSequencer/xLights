#include <wx/wx.h>
#include <wx/grid.h>
#include <wx/renderer.h>

class wxModelGridCellRenderer : public wxGridCellStringRenderer
{
public:
    wxModelGridCellRenderer(wxImage* image_, wxGrid& grid);
    virtual ~wxModelGridCellRenderer() {}

    virtual void Draw(wxGrid &grid, wxGridCellAttr &attr, wxDC &dc, const wxRect &rect, int row, int col, bool isSelected) wxOVERRIDE;

    void UpdateSize(wxGrid& grid, bool draw_picture_, int lightness_);
    void CreateImage();
    void SetImage(wxImage* image);
    void DetermineGridSize(wxGrid& grid);

private:
    wxImage* image;
    wxBitmap bmp;
    int width;
    int height;
    bool draw_picture;
    int lightness;
};