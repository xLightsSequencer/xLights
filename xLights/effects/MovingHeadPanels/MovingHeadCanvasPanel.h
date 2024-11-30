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
#include <wx/panel.h>

#include <memory>
#include <vector>

class IMovingHeadCanvasParent
{
public:
    virtual ~IMovingHeadCanvasParent() {}

    virtual void NotifyPositionUpdated() = 0;
};


class MovingHeadCanvasPanel : public wxPanel
{
public:
    MovingHeadCanvasPanel(IMovingHeadCanvasParent* movingHeadCanvasParent, wxWindow* parent, wxWindowID id = wxID_ANY,
                      const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
    virtual ~MovingHeadCanvasPanel() = default;

    void SetPosition(wxPoint2DDouble pos);
    wxPoint2DDouble GetPosition() { return m_mousePos; }
    void OnSize(wxSizeEvent& event);

private:

    DECLARE_EVENT_TABLE()

    void OnMovingHeadPaint(wxPaintEvent& event);
    void OnMovingHeadLeftDown(wxMouseEvent& event);
    void OnMovingHeadLeftUp(wxMouseEvent& event);
    void OnMovingHeadMouseMove(wxMouseEvent& event);
    void OnMovingHeadEntered(wxMouseEvent& event);

    wxPoint2DDouble UItoNormalized(const wxPoint2DDouble& pt) const;
    wxPoint2DDouble NormalizedToUI(const wxPoint2DDouble& pt) const;
    wxPoint NormalizedToUI2(const wxPoint2DDouble& pt) const;
    void SnapToLines(wxPoint2DDouble& pos);

    IMovingHeadCanvasParent* const m_movingHeadCanvasParent = nullptr;
    wxPoint2DDouble m_mousePos;
    bool m_mouseDown = false;
};
