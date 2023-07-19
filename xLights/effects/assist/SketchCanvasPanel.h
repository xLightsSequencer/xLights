#pragma once

#include <wx/geometry.h>
#include <wx/panel.h>

#include <memory>
#include <vector>

class SketchEffectPath;
class SketchEffectSketch;

class wxBitmap;

enum class SketchCanvasPathState : int {
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
    
    [[nodiscard]] virtual SketchEffectSketch& GetSketch() = 0;
    [[nodiscard]] virtual int GetSelectedPathIndex() = 0;
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

    [[nodiscard]] bool AcceptsFocus() const override
    {
        return true;
    }

    void clearBackgroundBitmap();
    void setBackgroundBitmap(std::unique_ptr<wxBitmap> bm);
    void UpdatePathState(SketchCanvasPathState state);
    void ResetHandlesState(SketchCanvasPathState pathState = SketchCanvasPathState::Undefined);
    void UpdateHandlesForPath(long pathIndex);
    [[nodiscard]] SketchCanvasPathState GetPathState() { return m_pathState; }

    void Changed();
    void ClosePath();
    void DrawGrid(bool val) { m_drawGrid = val; }

    void OnSketchKeyDown(wxKeyEvent& event); // for SketchAssistPanel access

private:
    enum class HandlePointType : int{ Point,
                           QuadraticControlPt,
                           QuadraticCurveEnd,
                           CubicControlPt1,
                           CubicControlPt2,
                           CubicCurveEnd };

    struct HandlePoint {
        HandlePoint(wxPoint2DDouble _pt, HandlePointType _handlePointType = HandlePointType::Point) :
            pt(_pt),
            handlePointType(_handlePointType)
        {}
        wxPoint2DDouble pt;
        bool state {false};
        HandlePointType handlePointType {HandlePointType::Point};
    };

    DECLARE_EVENT_TABLE()

    void OnSketchPaint(wxPaintEvent& event);
    void OnSketchLeftDown(wxMouseEvent& event);
    void OnSketchLeftUp(wxMouseEvent& event);
    void OnSketchMouseMove(wxMouseEvent& event);
    void OnSketchEntered(wxMouseEvent& event);
    void OnSketchMouseWheel(wxMouseEvent& event);
    void OnSketchMidDown(wxMouseEvent& event);

    [[nodiscard]] wxPoint2DDouble UItoNormalized(const wxPoint2DDouble& pt) const;
    [[nodiscard]] wxPoint2DDouble NormalizedToUI(const wxPoint2DDouble& pt) const;
    [[nodiscard]] wxPoint NormalizedToUI2(const wxPoint2DDouble& pt) const;
    [[nodiscard]] static bool IsControlPoint(const HandlePoint& handlePt);
    void UpdatePathFromHandles(long handleIndex);
    void UpdatePathFromHandles();
    [[nodiscard]] std::shared_ptr<SketchEffectPath> CreatePathFromHandles() const;
    [[nodiscard]] bool HandleHoveredOrGrabbed() const;

    // Handles and PathState are for the currently active path
    std::vector<HandlePoint> m_handles;
    size_t m_grabbedHandleIndex = -1; //unsigned type so wrap-around
    bool m_pathHoveredOrGrabbed {false};
    bool m_pathGrabbed {false};
    wxPoint2DDouble m_pathGrabbedPos;
    SketchCanvasPathState m_pathState {SketchCanvasPathState::Undefined};
    bool m_pathClosed {false};
    bool m_drawGrid {false};
    SketchCanvasPathState m_ClosedState {SketchCanvasPathState::LineToNewPoint};
    ISketchCanvasParent* const m_sketchCanvasParent = nullptr;
    std::unique_ptr<wxBitmap> m_bgBitmap;
    int m_wheelRotation {0};
    wxPoint2DDouble m_zoomPoint;
    wxPoint2DDouble m_canvasTranslation;
    double m_matrixComponents[6] {0.0,0.0,0.0,0.0,0.0,0.0};
    wxPoint2DDouble m_mousePos;
};
