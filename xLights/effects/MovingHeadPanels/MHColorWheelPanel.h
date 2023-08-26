#pragma once

#include <wx/geometry.h>
#include <wx/panel.h>

#include <memory>
#include <vector>

class IMHColorWheelPanelParent
{
public:
    virtual ~IMHColorWheelPanelParent() {}

    virtual void NotifyColorUpdated() = 0;
};


class MHColorWheelPanel : public wxPanel
{
public:
    MHColorWheelPanel(IMHColorWheelPanelParent* colorWheelParent, wxWindow* parent, wxWindowID id = wxID_ANY,
                      const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
    virtual ~MHColorWheelPanel();

    std::string GetColour();
    bool HasColour() { return false; }
    void SetColours( const std::string& _colors );
    void DefineColours( xlColorVector& _colors );

private:

    DECLARE_EVENT_TABLE()

    void OnPaint(wxPaintEvent& event);
    void OnLeftDown(wxMouseEvent& event);
    void OnLeftUp(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void OnEntered(wxMouseEvent& event);

    wxPoint2DDouble UItoNormalized(const wxPoint2DDouble& pt) const;
    wxPoint2DDouble NormalizedToUI(const wxPoint2DDouble& pt) const;
    wxPoint NormalizedToUI2(const wxPoint2DDouble& pt) const;

    IMHColorWheelPanelParent* const m_colorWheelParent {nullptr};
    wxPoint2DDouble m_mousePos;
    bool m_mouseDown {false};
    bool m_mouseDClick {false};

    int HitTest( wxPoint2DDouble& ptUI );
    bool insideColors(int x, int y);
    xlColor GetPointColor(int x, int y);
    double center {0};
    double radius {0};

    void CreateHsvBitmap(const wxSize& newSize);
    void CreateHsvBitmapMask();
    wxBitmap* m_hsvBitmap;
    wxMask* m_hsvMask;
    
    xlColorVector colors;
};
