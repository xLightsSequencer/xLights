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

#include <wx/geometry.h>
#include <wx/bitmap.h>

#include <memory>
#include <vector>

#include "MHColorPanel.h"

#include "../../Color.h"

class IMHRgbPickerPanelParent
{
public:
    virtual ~IMHRgbPickerPanelParent() {}

    virtual void NotifyColorUpdated() = 0;
};


class MHRgbPickerPanel : public MHColorPanel
{
public:
    MHRgbPickerPanel(IMHRgbPickerPanelParent* rgbPickerParent, wxWindow* parent, wxWindowID id = wxID_ANY,
                      const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
    virtual ~MHRgbPickerPanel();

    virtual std::string GetColour() override;
    virtual bool HasColour() override { return m_handles.size() > 0; }
    virtual void SetColours( const std::string& _colors ) override;
    virtual void ResetColours() override;
    void OnSize(wxSizeEvent& event);

private:

    struct HandlePoint {
        HandlePoint(wxPoint2DDouble _pt, HSVValue _color ) :
            pt(_pt),
            color(_color)
        {}
        wxPoint2DDouble pt;
        HSVValue color;
    };

    DECLARE_EVENT_TABLE()

    void OnPaint(wxPaintEvent& event);
    void OnLeftDown(wxMouseEvent& event);
    void OnLeftUp(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void OnEntered(wxMouseEvent& event);
    void OnKeyDown(wxKeyEvent& event);
    void OnKeyUp(wxKeyEvent& event);
    void OnLeave(wxMouseEvent& event);

    wxPoint2DDouble UItoNormalized(const wxPoint2DDouble& pt) const;
    wxPoint2DDouble NormalizedToUI(const wxPoint2DDouble& pt) const;
    wxPoint NormalizedToUI2(const wxPoint2DDouble& pt) const;

    IMHRgbPickerPanelParent* const m_rgbPickerParent {nullptr};
    wxPoint2DDouble m_mousePos;
    bool m_mouseDown {false};
    bool m_mouseDownV {false};
    bool m_mouseDClick {false};
    bool m_shiftdown {false};

    int HitTest( wxPoint2DDouble& ptUI );
    bool HitTestV( wxPoint2DDouble& ptUI );
    bool insideColors(int x, int y);
    xlColor GetPointColor(int x, int y);
    std::vector<HandlePoint> m_handles;
    int selected_point {-1};
    int active_handle {-1};
    double center {0};
    double radius {0};
    double v_left {0};
    double v_top {0};
    double v_width {0};
    double v_height {20.0};

    void CreateHsvBitmap(const wxSize& newSize);
    void CreateHsvBitmapMask();
    wxBitmap* m_hsvBitmap{ nullptr };
    wxMask* m_hsvMask{ nullptr };
};

