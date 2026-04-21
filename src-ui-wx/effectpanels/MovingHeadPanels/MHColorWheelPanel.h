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

#include "Color.h"
#include "MHColorPanel.h"

class IMHColorWheelPanelParent
{
public:
    virtual ~IMHColorWheelPanelParent() {}

    virtual void NotifyColorUpdated() = 0;
};


class MHColorWheelPanel : public MHColorPanel
{
public:
    MHColorWheelPanel(IMHColorWheelPanelParent* colorWheelParent, wxWindow* parent, wxWindowID id = wxID_ANY,
                      const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
    virtual ~MHColorWheelPanel();

    virtual std::string GetColour() override;
    virtual bool HasColour() override { return m_handles.size() > 0; }
    virtual void SetColours( const std::string& _colors ) override;
    virtual void ResetColours() override;
    void DefineColours( xlColorVector& _colors );
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

    struct FilterLocation {
        FilterLocation(int _x, int _y, float _radius, HSVValue _color ) :
            x(_x),
            y(_y),
            radius(_radius),
            color(_color)
        {}
        float x;
        float y;
        float radius;
        HSVValue color;
    };

    DECLARE_EVENT_TABLE()

    void OnPaint(wxPaintEvent& event);
    void OnLeftDown(wxMouseEvent& event);
    void OnLeftUp(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void OnEntered(wxMouseEvent& event);
    void OnLeave(wxMouseEvent& event);
    void OnKeyDown(wxKeyEvent& event);
    void OnKeyUp(wxKeyEvent& event);

    wxPoint2DDouble UItoNormalized(const wxPoint2DDouble& pt) const;
    wxPoint2DDouble NormalizedToUI(const wxPoint2DDouble& pt) const;
    wxPoint NormalizedToUI2(const wxPoint2DDouble& pt) const;

    IMHColorWheelPanelParent* const m_colorWheelParent {nullptr};
    wxPoint2DDouble m_mousePos;
    bool m_mouseDown {false};
    bool m_mouseDClick {false};
    bool m_shiftdown {false};

    int HitTest( wxPoint2DDouble& ptUI );
    bool insideColors(int x, int y, HSVValue& hsv);
    xlColor GetPointColor(int x, int y);
    std::vector<HandlePoint> m_handles;
    std::vector<FilterLocation> m_filters;
    int selected_point {-1};
    int active_handle {-1};
    double center {0};
    double radius {0};

    void CreateHsvBitmap(const wxSize& newSize);
    void CreateHsvBitmapMask();
    wxBitmap* m_hsvBitmap{ nullptr };
    wxMask* m_hsvMask{ nullptr };
    
    xlColorVector colors;
};
