#pragma once

#include <wx/geometry.h>
#include <wx/panel.h>

#include <memory>
#include <vector>

class IMovingHeadCanvasParent
{
public:
    virtual ~IMovingHeadCanvasParent() {}

    virtual void NotifyPositionUpdated() = 0;
    virtual wxPoint2DDouble GetPosition() = 0;
};


class MovingHeadCanvasPanel : public wxPanel
{
public:
    MovingHeadCanvasPanel(IMovingHeadCanvasParent* movingHeadCanvasParent, wxWindow* parent, wxWindowID id = wxID_ANY,
                      const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
    virtual ~MovingHeadCanvasPanel() = default;

    void SetPosition(wxPoint2DDouble pos);

private:

    DECLARE_EVENT_TABLE()

    void OnMovingHeadPaint(wxPaintEvent& event);
    void OnMovingHeadLeftDown(wxMouseEvent& event);
    void OnMovingHeadMouseMove(wxMouseEvent& event);
    void OnMovingHeadEntered(wxMouseEvent& event);

    wxPoint2DDouble UItoNormalized(const wxPoint2DDouble& pt) const;
    wxPoint2DDouble NormalizedToUI(const wxPoint2DDouble& pt) const;
    wxPoint NormalizedToUI2(const wxPoint2DDouble& pt) const;

    IMovingHeadCanvasParent* const m_movingHeadCanvasParent = nullptr;
    wxPoint2DDouble m_mousePos;
};
