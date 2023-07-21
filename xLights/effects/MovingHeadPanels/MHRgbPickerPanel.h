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
    virtual ~MHRgbPickerPanel() = default;

    void SetPosition(wxPoint2DDouble pos);
    wxPoint2DDouble GetPosition() { return m_mousePos; }

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

    IMHRgbPickerPanelParent* const m_rgbPickerParent = nullptr;
    wxPoint2DDouble m_mousePos;
    bool m_mouseDown = false;
};
