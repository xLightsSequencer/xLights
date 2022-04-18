#pragma once

#include <wx/geometry.h>
#include <wx/panel.h>

#include <memory>
#include <vector>

class SketchEffectPath;
class SketchEffectSketch;

class wxBitmap;

enum SketchCanvasPathState {
    Undefined,
    DefineStartPoint,
    LineToNewPoint,
    QuadraticCurveToNewPoint,
    CubicCurveToNewPoint
};

// This interface is an attempt to decouple the SketchCanvasPanel from it's
// parent UI. In the current implementation of the SketchCanvasPanel, only
// a single path from the sketch can be selected (or none). This interface
// assumes the parent UI follows that assumption with a SelectedPathIndex.
class ISketchCanvasParent
{
public:
    virtual ~ISketchCanvasParent() {}
    
    virtual SketchEffectSketch& GetSketch() = 0;
    virtual int GetSelectedPathIndex() = 0;
    virtual void NotifySketchUpdated() = 0;
    virtual void NotifySketchPathsUpdated() = 0;
    virtual void NotifyPathStateUpdated(SketchCanvasPathState state) = 0;
    virtual void SelectLastPath() = 0;
};

class SketchCanvasPanel : public wxPanel
{
public:
    SketchCanvasPanel(ISketchCanvasParent* sketchCanvasParent, wxWindow* parent, wxWindowID id = wxID_ANY,
                      const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
    virtual ~SketchCanvasPanel() = default;

    bool AcceptsFocus() const override
    {
        return true;
    }

    void setBackgroundBitmap(std::unique_ptr<wxBitmap> bm);
    void UpdatePathState(SketchCanvasPathState state);
    void ResetHandlesState(SketchCanvasPathState pathState = Undefined);
    void UpdateHandlesForPath(long pathIndex);
    void ClosePath()
    {
        m_pathClosed = true;
    }

    void OnSketchKeyDown(wxKeyEvent& event); // for SketchAssistPanel access

private:
    enum HandlePointType { Point,
                           QuadraticControlPt,
                           QuadraticCurveEnd,
                           CubicControlPt1,
                           CubicControlPt2,
                           CubicCurveEnd };

    struct HandlePoint {
        HandlePoint(wxPoint2DDouble _pt, HandlePointType _handlePointType = Point) :
            pt(_pt),
            handlePointType(_handlePointType)
        {}
        wxPoint2DDouble pt;
        bool state = false;
        HandlePointType handlePointType = Point;
    };

    DECLARE_EVENT_TABLE()

    void OnSketchPaint(wxPaintEvent& event);
    void OnSketchLeftDown(wxMouseEvent& event);
    void OnSketchLeftUp(wxMouseEvent& event);
    void OnSketchMouseMove(wxMouseEvent& event);
    void OnSketchEntered(wxMouseEvent& event);
    void OnSketchMouseWheel(wxMouseEvent& event);
    void OnSketchMidDown(wxMouseEvent& event);

    wxPoint2DDouble UItoNormalized(const wxPoint2DDouble& pt) const;
    wxPoint2DDouble NormalizedToUI(const wxPoint2DDouble& pt) const;
    static bool IsControlPoint(const HandlePoint& handlePt);
    void UpdatePathFromHandles(long handleIndex);
    void UpdatePathFromHandles();
    std::shared_ptr<SketchEffectPath> CreatePathFromHandles() const;
    bool HandleHoveredOrGrabbed() const;

    // Handles and PathState are for the currently active path
    std::vector<HandlePoint> m_handles;
    size_t m_grabbedHandleIndex = -1;
    bool m_pathHoveredOrGrabbbed = false;
    bool m_pathGrabbed = false;
    wxPoint2DDouble m_pathGrabbedPos;
    SketchCanvasPathState m_pathState = Undefined;
    bool m_pathClosed = false;

    ISketchCanvasParent* const m_sketchCanvasParent;
    std::unique_ptr<wxBitmap> m_bgBitmap;
    int m_wheelRotation = 0;
    wxPoint2DDouble m_zoomPoint;
    wxPoint2DDouble m_canvasTranslation;
    double m_matrixComponents[6];
    wxPoint2DDouble m_mousePos;
};
