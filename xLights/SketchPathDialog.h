#pragma once

#include "effects/SketchEffectDrawing.h"

#include <wx/dialog.h>
#include <wx/geometry.h>

#include <string>
#include <vector>

class wxButton;
class wxListView;
class wxPanel;

class SketchPathDialog: public wxDialog
{
public:
	SketchPathDialog(wxWindow* parent, wxWindowID id=wxID_ANY, const wxPoint& pos=wxDefaultPosition, const wxSize& size=wxDefaultSize);
    virtual ~SketchPathDialog() = default;

    void setSketch(const std::string& sketchStr);

    std::string sketchDefString() const;

private:
    DECLARE_EVENT_TABLE()

    enum PathState { Undefined,
                     DefineStartPoint,
                     LineToNewPoint,
                     QuadraticCurveToNewPoint,
                     CubicCurveToNewPoint };
    enum HandlePointType { Point,
                           QuadraticControlPt,
                           QuadraticCurveEnd,
                           CubicControlPt1,
                           CubicControlPt2,
                           CubicCurveEnd };
    struct HandlePoint {
        HandlePoint(wxPoint2DDouble _pt, HandlePointType _handlePointType=Point) :
            pt(_pt),
            handlePointType(_handlePointType)
        {}
        wxPoint2DDouble pt;
        bool state = false;
        HandlePointType handlePointType = Point;
    };

    wxPanel* m_sketchPanel = nullptr;
    wxButton* m_startPathBtn = nullptr;
    wxButton* m_endPathBtn = nullptr;
    wxButton* m_closePathBtn = nullptr;
    wxListView* m_pathsListView = nullptr;

	void OnSketchPaint(wxPaintEvent& event);
    void OnSketchKeyDown(wxKeyEvent& event);
    void OnSketchLeftDown(wxMouseEvent& event);
    void OnSketchLeftUp(wxMouseEvent& event);
    void OnSketchMouseMove(wxMouseEvent& event);

    void OnButton_StartPath(wxCommandEvent& event);
    void OnButton_EndPath(wxCommandEvent& event);
    void OnButton_ClosePath(wxCommandEvent& event);
    void OnButton_ClearSketch(wxCommandEvent& event);

    void OnListView_PathSelected(wxCommandEvent& event);

    void OnButton_Ok(wxCommandEvent& event);
    void OnButton_Cancel(wxCommandEvent& event);

    wxPoint2DDouble UItoNormalized(const wxPoint2DDouble& pt) const;
    wxPoint2DDouble NormalizedToUI(const wxPoint2DDouble& pt) const;
    void UpdateHandlesForPath(long pathIndex);
    void UpdatePathForHandles(long handleIndex);
    void UpdatePathState(PathState state);
    static bool isControlPoint(const HandlePoint& handlePt);
    std::shared_ptr<SketchEffectPath> CreatePathFromHandles() const;

    // Handles and PathState are for the currently active path
    std::vector<HandlePoint> m_handles;
    std::vector<HandlePoint>::size_type m_grabbedHandleIndex = -1;
    PathState m_pathState = Undefined;
    bool m_pathClosed = false;

    SketchEffectSketch m_sketch;
    wxPoint2DDouble m_mousePos;
};
