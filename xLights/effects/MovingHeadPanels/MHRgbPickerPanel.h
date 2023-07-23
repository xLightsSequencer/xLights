#pragma once

#include <wx/geometry.h>
#include <wx/panel.h>

#include <memory>
#include <vector>

class IMHRgbPickerPanelParent
{
public:
    virtual ~IMHRgbPickerPanelParent() {}

    virtual void NotifyColorUpdated() = 0;
};


class MHRgbPickerPanel : public wxPanel
{
public:
    MHRgbPickerPanel(IMHRgbPickerPanelParent* rgbPickerParent, wxWindow* parent, wxWindowID id = wxID_ANY,
                      const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
    virtual ~MHRgbPickerPanel();

    std::string GetColour();
    bool HasColour() { return m_handles.size() > 0; }

private:

    struct HandlePoint {
        HandlePoint(wxPoint2DDouble _pt, wxColour _color ) :
            pt(_pt),
            color(_color)
        {}
        wxPoint2DDouble pt;
        wxColour color;
    };

    DECLARE_EVENT_TABLE()

    void OnPaint(wxPaintEvent& event);
    void OnLeftDown(wxMouseEvent& event);
    void OnLeftUp(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void OnEntered(wxMouseEvent& event);
    void OnKeyDown(wxKeyEvent& event);
    void OnKeyUp(wxKeyEvent& event);

    wxPoint2DDouble UItoNormalized(const wxPoint2DDouble& pt) const;
    wxPoint2DDouble NormalizedToUI(const wxPoint2DDouble& pt) const;
    wxPoint NormalizedToUI2(const wxPoint2DDouble& pt) const;

    IMHRgbPickerPanelParent* const m_rgbPickerParent {nullptr};
    wxPoint2DDouble m_mousePos;
    bool m_mouseDown {false};
    bool m_mouseDClick {false};
    bool m_shiftdown {false};

    int HitTest( wxPoint2DDouble& ptUI );
    bool insideColors(int x, int y);
    xlColor GetPointColor(int x, int y);
    std::vector<HandlePoint> m_handles;
    int selected_point {-1};
    int active_handle {-1};
    float center {0};
    float radius {0};

    void CreateHsvBitmap(const wxSize& newSize);
    void CreateHsvBitmapMask();
    wxBitmap* m_hsvBitmap;
    wxMask* m_hsvMask;
};
