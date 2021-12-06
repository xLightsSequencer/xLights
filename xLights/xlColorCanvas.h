#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "wx/wx.h"
#include "graphics/xlGraphicsBase.h"
#include "Color.h"

wxDECLARE_EVENT(EVT_CP_SLIDER_CHANGED, wxCommandEvent);
wxDECLARE_EVENT(EVT_CP_PALETTE_CHANGED, wxCommandEvent);

class xlColorCanvas : public GRAPHICS_BASE_CLASS {
public:
    xlColorCanvas(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition,
                  const wxSize& size = wxDefaultSize, long style = 0, const wxString& name = wxPanelNameStr);
    virtual ~xlColorCanvas();

    enum class DisplayType {
        TYPE_PALETTE,
        TYPE_SLIDER
    };

    void SetType(xlColorCanvas::DisplayType type) {
        mDisplayType = type;
    }
    void SetMode(ColorDisplayMode mode);
    const HSVValue& GetHSV() const;
    void SetHSV(const HSVValue& hsv);
    wxColor GetRGB() const {
        return mRGB.asWxColor();
    }
    void SetRGB(xlColor rgb);

protected:
private:
    void mouseMoved(wxMouseEvent& event);
    void mouseDown(wxMouseEvent& event);
    void mouseReleased(wxMouseEvent& event);
    void render(wxPaintEvent& event);
    void render();
    void DrawSlider(xlGraphicsContext* ctx);
    void DrawPalette(xlGraphicsContext* ctx);
    void ProcessSliderClick(int row);
    void ProcessPaletteClick(int row, int column);
    int GetRGBColorFromRangeValue(int position, int range, int max_value, bool invert);

    DisplayType mDisplayType = xlColorCanvas::DisplayType::TYPE_PALETTE;
    ColorDisplayMode mDisplayMode = MODE_SATURATION;
    bool mDragging = false;
    HSVValue mHSV;
    xlColor mRGB;

    xlVertexColorAccumulator* background;
    DECLARE_EVENT_TABLE()
};
