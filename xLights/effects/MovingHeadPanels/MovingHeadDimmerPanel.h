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

class Element;

class IMovingHeadDimmerParent
{
public:
    virtual ~IMovingHeadDimmerParent() {}

    virtual void NotifyDimmerUpdated() = 0;
};


class MovingHeadDimmerPanel : public wxPanel
{
public:
    MovingHeadDimmerPanel(IMovingHeadDimmerParent* MovingHeadDimmerParent, wxWindow* parent, wxWindowID id = wxID_ANY,
                      const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
    virtual ~MovingHeadDimmerPanel() = default;

    std::string GetDimmerCommands();
    void SetDimmerCommands( const std::string& _dimmerCmds );

    void SetTimingTrack( const Element* timing );

    void SetEffectTimeRange(int startTimeMs, int endTimeMs);

    void OnSize(wxSizeEvent& event);

private:
    DECLARE_EVENT_TABLE()

    void OnMovingHeadPaint(wxPaintEvent& event);
    void OnKeyDown(wxKeyEvent& event);
    void OnMovingHeadLeftDown(wxMouseEvent& event);
    void OnMovingHeadLeftUp(wxMouseEvent& event);
    void OnMovingHeadMouseMove(wxMouseEvent& event);
    void OnMovingHeadEntered(wxMouseEvent& event);

    wxPoint2DDouble UItoNormalized(const wxPoint2DDouble& pt) const;
    wxPoint2DDouble NormalizedToUI(const wxPoint2DDouble& pt) const;
    wxPoint NormalizedToUI2(const wxPoint2DDouble& pt) const;
    void SnapToLines(wxPoint2DDouble& pos);
    int HitTest(wxPoint2DDouble& ptUI);

    IMovingHeadDimmerParent* const m_MovingHeadDimmerParent = nullptr;
    std::vector<wxPoint2DDouble> m_handles;
    int active_handle {-1};
    int selected_handle {-1};
    wxPoint2DDouble m_mousePos;
    bool m_mouseDown = false;
    std::vector<float> timings;
    int startTimeMs_ {0};
    int endTimeMs_ {0};
    const Element* timingTrack_ {nullptr};
};
