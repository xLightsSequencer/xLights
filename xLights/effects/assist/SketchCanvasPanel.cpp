#include "SketchCanvasPanel.h"
#include "../SketchEffectDrawing.h"

#include <wx/dcbuffer.h>
#include <wx/graphics.h>

namespace
{
    const int BorderWidth = 5;
 
    const int MouseWheelLimit = 1440;

    const double HandleRadius = 4.5;
    const double HandleRadiusSquared = HandleRadius * HandleRadius;
    const double HandleDiameter = 2 * HandleRadius;

    const double ZoomPointChangeThreshold = 20.;

    struct LinearInterpolater {
        double operator()(double t) const
        {
            return t;
        }
    };

    template<class T>
    double interpolate(double x, double loIn, double loOut, double hiIn, double hiOut, const T& interpolater)
    {
        return (loIn != hiIn)
                   ? (loOut + (hiOut - loOut) * interpolater((x - loIn) / (hiIn - loIn)))
                   : ((loOut + hiOut) / 2);
    }
}

BEGIN_EVENT_TABLE(SketchCanvasPanel, wxPanel)
    EVT_PAINT(SketchCanvasPanel::OnSketchPaint)
    EVT_KEY_DOWN(SketchCanvasPanel::OnSketchKeyDown)
    EVT_LEFT_DOWN(SketchCanvasPanel::OnSketchLeftDown)
    EVT_LEFT_UP(SketchCanvasPanel::OnSketchLeftUp)
    EVT_MOTION(SketchCanvasPanel::OnSketchMouseMove)
    EVT_ENTER_WINDOW(SketchCanvasPanel::OnSketchEntered)
    EVT_MOUSEWHEEL(SketchCanvasPanel::OnSketchMouseWheel)
    EVT_MIDDLE_DOWN(SketchCanvasPanel::OnSketchMidDown)
END_EVENT_TABLE()


SketchCanvasPanel::SketchCanvasPanel(ISketchCanvasParent* sketchCanvasParent, wxWindow* parent, wxWindowID id /*=wxID_ANY*/, const wxPoint& pos /*= wxDefaultPosition*/, const wxSize& size /*=wxDefaultSize*/) :
    wxPanel(parent, id, pos, size, wxNO_BORDER | wxWANTS_CHARS),
    m_sketchCanvasParent(sketchCanvasParent)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
}

void SketchCanvasPanel::OnSketchPaint(wxPaintEvent& /*event*/)
{
    wxAutoBufferedPaintDC pdc(this);
    wxSize sz(GetSize());
    wxRect borderRect(BorderWidth, BorderWidth, sz.GetWidth() - 2 * BorderWidth, sz.GetHeight() - 2 * BorderWidth);
    wxRect bgRect(wxRect(borderRect).Deflate(1, 1));

    pdc.SetPen(*wxWHITE_PEN);
    pdc.SetBrush(*wxWHITE_BRUSH);
    pdc.DrawRectangle(wxPoint(0, 0), sz);

    pdc.SetPen(*wxLIGHT_GREY_PEN);
    pdc.DrawRectangle(borderRect);

    if( m_drawGrid ) {
        for( int i = 0; i < 9; ++i ) {
            wxPoint2DDouble start_x(0.1 * (float)i + 0.1, 0);
            wxPoint2DDouble end_x(0.1 * (float)i + 0.1, 1);
            wxPoint2DDouble start_y(0, 0.1 * (float)i + 0.1);
            wxPoint2DDouble end_y(1, 0.1 * (float)i + 0.1);
            if( i == 4 ) {
                pdc.SetPen(*wxGREY_PEN);
            } else {
                pdc.SetPen(*wxLIGHT_GREY_PEN);
            }
            pdc.DrawLine(NormalizedToUI2(start_x), NormalizedToUI2(end_x));
            pdc.DrawLine(NormalizedToUI2(start_y), NormalizedToUI2(end_y));
        }
    }

    {
        std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(pdc));
        double zoomLevel {1.0};
        if (m_wheelRotation) {
            zoomLevel = interpolate(m_wheelRotation, 0, 1., MouseWheelLimit, 8., LinearInterpolater());
            wxGraphicsMatrix m = gc->CreateMatrix();
            m.Translate(-m_canvasTranslation.m_x, -m_canvasTranslation.m_y);
            m.Translate(m_zoomPoint.m_x, m_zoomPoint.m_y);
            m.Scale(zoomLevel, zoomLevel);
            m.Translate(-m_zoomPoint.m_x, -m_zoomPoint.m_y);
            gc->SetTransform(m);

            m.Get(m_matrixComponents, m_matrixComponents + 1, m_matrixComponents + 2,
                  m_matrixComponents + 3, m_matrixComponents + 4, m_matrixComponents + 5);
        }

        // First, draw the background
        if (m_bgBitmap != nullptr)
            gc->DrawBitmap(*(m_bgBitmap.get()), bgRect.x, bgRect.y, bgRect.width, bgRect.height);

        // Next, draw the unselected path(s)
        wxGraphicsPen pen = gc->CreatePen(wxGraphicsPenInfo(*wxLIGHT_GREY, 1. / zoomLevel));
        gc->SetPen(pen);

        const SketchEffectSketch& sketch(m_sketchCanvasParent->GetSketch());
        long selectedPathIndex = m_sketchCanvasParent->GetSelectedPathIndex();
        long pathIndex {0};
        for (const auto& path : sketch.paths()) {
            if (pathIndex++ == selectedPathIndex)
                continue;
            wxGraphicsPath graphicsPath(gc->CreatePath());
            const auto& firstSegment(path->segments().front());
            wxPoint2DDouble startPt(NormalizedToUI(firstSegment->StartPoint()));
            graphicsPath.MoveToPoint(startPt);

            for (const auto& segment : path->segments()) {
                std::shared_ptr<SketchQuadraticBezier> quadratic;
                std::shared_ptr<SketchCubicBezier> cubic;

                if (std::dynamic_pointer_cast<SketchLine>(segment) != nullptr) {
                    graphicsPath.AddLineToPoint(NormalizedToUI(segment->EndPoint()));
                } else if ((quadratic = std::dynamic_pointer_cast<SketchQuadraticBezier>(segment)) != nullptr) {
                    wxPoint2DDouble ctrlPt(NormalizedToUI(quadratic->ControlPoint()));
                    wxPoint2DDouble endPt(NormalizedToUI(quadratic->EndPoint()));
                    graphicsPath.AddQuadCurveToPoint(ctrlPt.m_x, ctrlPt.m_y, endPt.m_x, endPt.m_y);
                } else if ((cubic = std::dynamic_pointer_cast<SketchCubicBezier>(segment)) != nullptr) {
                    wxPoint2DDouble ctrlPt1(NormalizedToUI(cubic->ControlPoint1()));
                    wxPoint2DDouble ctrlPt2(NormalizedToUI(cubic->ControlPoint2()));
                    wxPoint2DDouble endPt(NormalizedToUI(cubic->EndPoint()));
                    graphicsPath.AddCurveToPoint(ctrlPt1, ctrlPt2, endPt);
                }
            }
            if (path->isClosed())
                graphicsPath.CloseSubpath();

            gc->DrawPath(graphicsPath);
        }

        // Next, draw the selected path
        pen = gc->CreatePen(wxGraphicsPenInfo(m_pathHoveredOrGrabbed ? *wxRED : *wxBLACK, 1. / zoomLevel));
        gc->SetPen(pen);
        std::vector<wxPoint>::size_type n = m_handles.size();
        wxGraphicsPath path(gc->CreatePath());
        for (std::vector<wxPoint>::size_type i = 0; i < n; ++i) {
            wxPoint2DDouble pt(NormalizedToUI(m_handles[i].pt));
            if (i == 0)
                path.MoveToPoint(pt);
            else if (m_handles[i].handlePointType == HandlePointType::Point)
                path.AddLineToPoint(pt);
            else if (m_handles[i].handlePointType == HandlePointType::QuadraticControlPt) {
                auto endPt = NormalizedToUI(m_handles[i + 1].pt);
                if( m_pathClosed && i == n - 1 ) {
                    endPt = NormalizedToUI(m_handles[0].pt);
                }
                path.AddQuadCurveToPoint(pt.m_x, pt.m_y, endPt.m_x, endPt.m_y);
                i += 1;
            } else if (m_handles[i].handlePointType == HandlePointType::CubicControlPt1) {
                auto cp2 = NormalizedToUI(m_handles[i + 1].pt);
                auto endPt = NormalizedToUI(m_handles[i + 2].pt);
                if( m_pathClosed && i == n - 2 ) {
                    endPt = NormalizedToUI(m_handles[0].pt);
                }
                path.AddCurveToPoint(pt, cp2, endPt);
                i += 2;
            }
        }

        if (m_pathClosed && m_handles.size() > 2)
            path.CloseSubpath();
        gc->DrawPath(path);

        // Next, if we are in a segment-adding state, draw that potential segment
        if ((m_pathState == SketchCanvasPathState::LineToNewPoint || m_pathState == SketchCanvasPathState::QuadraticCurveToNewPoint || m_pathState == SketchCanvasPathState::CubicCurveToNewPoint) && !m_handles.empty()) {
            switch (m_pathState) {
            case SketchCanvasPathState::LineToNewPoint:
                pen = gc->CreatePen(wxGraphicsPenInfo(*wxBLACK, 2. / zoomLevel, wxPENSTYLE_SHORT_DASH));
                break;
            case SketchCanvasPathState::QuadraticCurveToNewPoint:
                pen = gc->CreatePen(wxGraphicsPenInfo(*wxRED, 2. / zoomLevel, wxPENSTYLE_SHORT_DASH));
                break;
            case SketchCanvasPathState::CubicCurveToNewPoint:
                pen = gc->CreatePen(wxGraphicsPenInfo(*wxBLUE, 2. / zoomLevel, wxPENSTYLE_SHORT_DASH));
                break;
            default:
                break;
            }
            gc->SetPen(pen);

            wxAffineMatrix2D m;
            if (m_wheelRotation) {
                wxMatrix2D m2d(m_matrixComponents[0], m_matrixComponents[1],
                               m_matrixComponents[2], m_matrixComponents[3]);
                wxPoint2DDouble mt(m_matrixComponents[4], m_matrixComponents[5]);
                m.Set(m2d, mt);
                m.Invert();
            }

            auto ptFrom = NormalizedToUI(m_handles.back().pt);
            auto ptTo(m.TransformPoint(m_mousePos));
            gc->StrokeLine(ptFrom.m_x, ptFrom.m_y, ptTo.m_x, ptTo.m_y);

            pen = gc->CreatePen(wxGraphicsPenInfo(*wxBLACK, 1. / zoomLevel, wxPENSTYLE_SOLID));
            gc->SetPen(pen);
        }

        // Finally, draw the handles
        for (std::vector<wxPoint>::size_type i = 0; i < n; ++i) {
            wxPoint2DDouble pt(NormalizedToUI(m_handles[i].pt));
            if (i == 0 && !m_handles[i].state)
                gc->SetBrush(*wxGREEN_BRUSH);
            else if (i == n - 1 && !m_handles[i].state)
                gc->SetBrush(*wxRED_BRUSH);
            else
                gc->SetBrush((m_handles[i].state) ? (*wxYELLOW_BRUSH)
                                                  : (IsControlPoint(m_handles[i]) ? (*wxBLUE_BRUSH) : (*wxLIGHT_GREY_BRUSH)));

            gc->DrawEllipse(pt.m_x - HandleRadius / zoomLevel, pt.m_y - HandleRadius / zoomLevel, HandleDiameter / zoomLevel, HandleDiameter / zoomLevel);
        }
    }
}

void SketchCanvasPanel::OnSketchKeyDown(wxKeyEvent& event)
{
    int keycode = event.GetKeyCode();
    if (keycode == WXK_DELETE && (m_pathState == SketchCanvasPathState::Undefined || m_pathState == SketchCanvasPathState::DefineStartPoint)) {
        int pathIndex = m_sketchCanvasParent->GetSelectedPathIndex();
        std::optional<std::pair<int, int>> toErase;
        int index = 0;
        for (auto iter = m_handles.begin(); iter != m_handles.end(); ++iter, ++index) {
            if (iter->state) {
                switch (iter->handlePointType) {
                case HandlePointType::Point:
                    toErase = std::make_pair(index, 1);
                    break;
                case HandlePointType::QuadraticControlPt:
                    toErase = std::make_pair(index, 2);
                    break;
                case HandlePointType::QuadraticCurveEnd:
                    toErase = std::make_pair(index - 1, 2);
                    break;
                case HandlePointType::CubicControlPt1:
                    toErase = std::make_pair(index, 3);
                    break;
                case HandlePointType::CubicControlPt2:
                    toErase = std::make_pair(index - 1, 3);
                    break;
                case HandlePointType::CubicCurveEnd:
                    toErase = std::make_pair(index - 2, 3);
                    break;
                }
                break;
            }
        }
        if (toErase.has_value()) {
            auto startIter = m_handles.cbegin();
            auto endIter = m_handles.cbegin();
            auto p = toErase.value();
            std::advance(startIter, p.first);
            std::advance(endIter, p.second + p.first);
            m_handles.erase(startIter, endIter);
            if (m_handles.size() == 1)
                m_handles.clear();

            if (m_handles.empty()) {
                SketchEffectSketch& sketch(m_sketchCanvasParent->GetSketch());
                sketch.deletePath(pathIndex);
                m_sketchCanvasParent->NotifySketchUpdated();
                m_sketchCanvasParent->NotifySketchPathsUpdated();
            } else
                UpdatePathFromHandles();
        }

        Refresh();
    } else if (keycode == WXK_ESCAPE) {
        m_ClosedState = m_pathState;
        UpdatePathState(SketchCanvasPathState::Undefined);
        m_sketchCanvasParent->NotifySketchUpdated();
    } else if (keycode == WXK_SPACE) {
        m_ClosedState = m_pathState;
        ClosePath();
        UpdatePathState(SketchCanvasPathState::Undefined);
        m_sketchCanvasParent->NotifySketchUpdated();
    } else if (keycode == WXK_SHIFT) {
        switch (m_pathState) {
        case SketchCanvasPathState::LineToNewPoint:
            m_pathState = SketchCanvasPathState::QuadraticCurveToNewPoint;
            break;
        case SketchCanvasPathState::QuadraticCurveToNewPoint:
            m_pathState = SketchCanvasPathState::CubicCurveToNewPoint;
            break;
        case SketchCanvasPathState::CubicCurveToNewPoint:
            m_pathState = SketchCanvasPathState::LineToNewPoint;
            break;
        default:
            break;
        }
        Refresh();
    }
}

void SketchCanvasPanel::OnSketchLeftDown(wxMouseEvent& event)
{
    wxAffineMatrix2D m;
    if (m_wheelRotation) {
        wxMatrix2D m2d(m_matrixComponents[0], m_matrixComponents[1],
                       m_matrixComponents[2], m_matrixComponents[3]);
        wxPoint2DDouble mt(m_matrixComponents[4], m_matrixComponents[5]);
        m.Set(m2d, mt);
        m.Invert();
    }
    wxPoint2DDouble ptUI(m.TransformPoint(event.GetPosition()));
    wxPoint2DDouble pt(UItoNormalized(ptUI));

    // Defining-new-segment stuff
    if (m_pathState == SketchCanvasPathState::DefineStartPoint) {
        m_handles.push_back(pt);
        UpdatePathState(SketchCanvasPathState::LineToNewPoint);
        return;
    } else if (m_pathState == SketchCanvasPathState::LineToNewPoint) {
        m_handles.push_back(pt);
        Refresh();
        return;
    } else if (m_pathState == SketchCanvasPathState::QuadraticCurveToNewPoint) {
        wxPoint2DDouble startPt = m_handles.back().pt;
        wxPoint2DDouble endPt = pt;
        wxPoint2DDouble cp = 0.5 * startPt + 0.5 * endPt;

        m_handles.push_back(HandlePoint(cp, HandlePointType::QuadraticControlPt));
        m_handles.push_back(HandlePoint(endPt, HandlePointType::QuadraticCurveEnd));
        Refresh();
        return;
    } else if (m_pathState == SketchCanvasPathState::CubicCurveToNewPoint) {
        wxPoint2DDouble startPt = m_handles.back().pt;
        wxPoint2DDouble endPt = pt;
        wxPoint2DDouble cp1 = 0.75 * startPt + 0.25 * endPt;
        wxPoint2DDouble cp2 = 0.25 * startPt + 0.75 * endPt;

        m_handles.push_back(HandlePoint(cp1, HandlePointType::CubicControlPt1));
        m_handles.push_back(HandlePoint(cp2, HandlePointType::CubicControlPt2));
        m_handles.push_back(HandlePoint(endPt, HandlePointType::CubicCurveEnd));
        Refresh();
        return;
    }

    // Potentially moving a path around
    if (m_pathHoveredOrGrabbed) {
        m_pathGrabbedPos = event.GetPosition();
        m_pathGrabbed = true;
    // Updating 'grabbed' handle
    } else {
        for (std::vector<HandlePoint>::size_type i = 0; i < m_handles.size(); ++i) {
            if (m_handles[i].state) {
                m_grabbedHandleIndex = i;
                break;
            }
        }
    }
}

void SketchCanvasPanel::OnSketchLeftUp(wxMouseEvent& /*event*/)
{
    if (m_grabbedHandleIndex != -1) {
        UpdatePathFromHandles(m_grabbedHandleIndex);
        // temp for debugging handles-path synchronization
        // UpdateHandlesForPath(m_pathsListView->GetFirstSelected());
    }

    m_grabbedHandleIndex = -1;
    if (m_pathGrabbed) {
        UpdatePathFromHandles();
        m_sketchCanvasParent->NotifySketchUpdated();
        m_pathGrabbed = false;
    }
}

void SketchCanvasPanel::OnSketchMouseMove(wxMouseEvent& event)
{
    // handling drag of canvas as a special case for now...
    if (event.ButtonIsDown(wxMOUSE_BTN_MIDDLE)) {
        m_canvasTranslation += m_mousePos - event.GetPosition();
        m_mousePos = event.GetPosition();
        Refresh();
        return;
    }

    m_mousePos = event.GetPosition();

    if (m_pathState == SketchCanvasPathState::LineToNewPoint || m_pathState == SketchCanvasPathState::QuadraticCurveToNewPoint || m_pathState == SketchCanvasPathState::CubicCurveToNewPoint) {
        Refresh();
        return;
    }

    wxAffineMatrix2D m;
    if (m_wheelRotation) {
        wxMatrix2D m2d(m_matrixComponents[0], m_matrixComponents[1],
                       m_matrixComponents[2], m_matrixComponents[3]);
        wxPoint2DDouble mt(m_matrixComponents[4], m_matrixComponents[5]);
        m.Set(m2d, mt);
    }

    // dragging a handle
    if (m_grabbedHandleIndex != -1 && event.ButtonIsDown(wxMOUSE_BTN_LEFT)) {
        m.Invert();
        m_handles[m_grabbedHandleIndex].pt = UItoNormalized(m.TransformPoint(m_mousePos));
        Refresh();

    // dragging a path
    } else if (m_pathGrabbed && event.ButtonIsDown(wxMOUSE_BTN_LEFT)) {
        m.Invert();
        wxPoint2DDouble delta(m.TransformPoint(m_mousePos) - m.TransformPoint(m_pathGrabbedPos));
        for (auto& handle : m_handles) {
            wxPoint2DDouble handlePos(NormalizedToUI(handle.pt));
            handlePos += delta;
            handle.pt = UItoNormalized(handlePos);
        }
        m_pathGrabbedPos = m_mousePos;
        Refresh();
    } else {
        bool somethingChanged = false;
        for (auto& handle : m_handles) {
            wxPoint2DDouble handlePos(NormalizedToUI(handle.pt));
            wxPoint2DDouble transformedHandlePos(m.TransformPoint(handlePos));
            bool state = m_mousePos.GetDistanceSquare(transformedHandlePos) <= HandleRadiusSquared;
            if (state != handle.state) {
                handle.state = state;
                m_pathHoveredOrGrabbed = false;
                somethingChanged = true;
            }
        }

        // do hit-testing over segments if we're not hovered over a handle
        if (!somethingChanged && !HandleHoveredOrGrabbed()) {
            bool hovered = false;

            m.Invert();
            wxPoint2DDouble transformedMousePos(m.TransformPoint(m_mousePos));
            wxPoint2DDouble pt(UItoNormalized(transformedMousePos));

            int pathIndex = m_sketchCanvasParent->GetSelectedPathIndex();
            if (pathIndex >= 0) {
                const SketchEffectSketch& sketch(m_sketchCanvasParent->GetSketch());
                auto paths = sketch.paths();
                if( paths.size() > 0 ) {
                    auto segments(paths[pathIndex]->segments());
                    for (const auto& segment : segments) {
                        if (segment->HitTest(pt)) {
                            hovered = true;
                            break;
                        }
                    }
                }
            }
            if (hovered != m_pathHoveredOrGrabbed) {
                m_pathHoveredOrGrabbed = hovered;
                somethingChanged = true;
            }
        }

        if (somethingChanged)
            Refresh();
    }
}

void SketchCanvasPanel::OnSketchEntered(wxMouseEvent& /*event*/)
{
    SetFocus();
}

void SketchCanvasPanel::OnSketchMouseWheel(wxMouseEvent& event)
{
    m_wheelRotation += event.GetWheelRotation();
    m_wheelRotation = std::clamp(m_wheelRotation, 0, MouseWheelLimit);
    if (!m_wheelRotation) {
        m_canvasTranslation = wxPoint2DDouble();
        m_zoomPoint = wxPoint2DDouble();
    }  else {
        if (m_zoomPoint == wxPoint2DDouble() )
            m_zoomPoint = event.GetPosition();
        double distance = m_zoomPoint.GetDistance(event.GetPosition() + m_canvasTranslation);
        if (distance > ZoomPointChangeThreshold) {
            // Much of the time, we're zooming in and out near the same point.
            // If that zoom-point seems to be different, we adjust.
            m_canvasTranslation -= event.GetPosition() - m_zoomPoint;
            m_zoomPoint = event.GetPosition();
        } else {
            m_zoomPoint = event.GetPosition() + m_canvasTranslation;
        }
    }

    Refresh();
}

void SketchCanvasPanel::OnSketchMidDown(wxMouseEvent& event)
{
    m_mousePos = event.GetPosition();
}

// Usable area rect: BorderWidth+1, BorderWidth+1, sz.GetWidth()-2*BorderWidth-2, sz.GetHeight()-2*BorderWidth-2;

wxPoint2DDouble SketchCanvasPanel::UItoNormalized(const wxPoint2DDouble& pt) const
{
    wxPoint o(BorderWidth + 1, BorderWidth + 1);
    wxSize sz(GetSize() - wxSize(2 * BorderWidth + 2, 2 * BorderWidth + 2));

    double x = double(pt.m_x - o.x) / sz.GetWidth();
    double y = double(pt.m_y - o.y) / sz.GetHeight();

    return wxPoint2DDouble(x, 1. - y);
}

wxPoint2DDouble SketchCanvasPanel::NormalizedToUI(const wxPoint2DDouble& pt) const
{
    wxPoint o(BorderWidth + 1, BorderWidth + 1);
    wxSize sz(GetSize() - wxSize(2 * BorderWidth + 2, 2 * BorderWidth + 2));

    double x = pt.m_x * sz.GetWidth();
    double y = pt.m_y * sz.GetHeight();
    return wxPoint2DDouble(o.x + x, o.y + (sz.GetHeight() - y));
}

wxPoint SketchCanvasPanel::NormalizedToUI2(const wxPoint2DDouble& pt) const
{
    wxPoint2DDouble pt1 = NormalizedToUI(pt);
    return wxPoint((int)pt1.m_x, (int)pt1.m_y);
}

bool SketchCanvasPanel::IsControlPoint(const HandlePoint& handlePt)
{
    auto hpt = handlePt.handlePointType;
    return hpt == HandlePointType::QuadraticControlPt || hpt == HandlePointType::CubicControlPt1 || hpt == HandlePointType::CubicControlPt2;
}

void SketchCanvasPanel::UpdatePathState(SketchCanvasPathState pathState)
{
    m_pathState = pathState;

    m_sketchCanvasParent->NotifyPathStateUpdated(m_pathState);

    Refresh();

    // If we're Undefined, have some handles, and no path
    // selected, I think we've added a new one!!
    bool inUndefinedStateAndHaveMultipleHandles = (m_pathState == SketchCanvasPathState::Undefined && m_handles.size() > 1);
    if (inUndefinedStateAndHaveMultipleHandles && m_sketchCanvasParent->GetSelectedPathIndex() < 0) {
        auto path = CreatePathFromHandles();
        if (path != nullptr) {
            SketchEffectSketch& sketch(m_sketchCanvasParent->GetSketch());
            sketch.appendPath(path);

            m_sketchCanvasParent->NotifySketchPathsUpdated();
            m_sketchCanvasParent->SelectLastPath();
        }
    // 'continuing an existing path' case
    } else if (inUndefinedStateAndHaveMultipleHandles) {
        auto path = CreatePathFromHandles();
        if (path != nullptr) {
            SketchEffectSketch& sketch(m_sketchCanvasParent->GetSketch());
            auto& paths( sketch.paths() );
            int index = m_sketchCanvasParent->GetSelectedPathIndex();
            if (index <= paths.size()) {
                paths[index] = path;
                m_sketchCanvasParent->NotifySketchUpdated();
            }
        }
    }

    // A completed path should never result in a single handle
    if (m_pathState == SketchCanvasPathState::Undefined && m_handles.size() == 1) {
        m_handles.clear();
        Refresh();
    }
}

void SketchCanvasPanel::ResetHandlesState(SketchCanvasPathState state /*Undefined*/) 
{
    m_handles.clear();
    m_grabbedHandleIndex = -1;
    m_pathClosed = false;
    UpdatePathState(state);
}

void SketchCanvasPanel::UpdateHandlesForPath(long pathIndex)
{
    const SketchEffectSketch& sketch(m_sketchCanvasParent->GetSketch());

    if (pathIndex < 0 || pathIndex >= sketch.paths().size())
        return;

    ResetHandlesState();

    auto iter = sketch.paths().cbegin();
    std::advance(iter, pathIndex);
 
    enum class SegmentType : int { Unknown,
                       Line,
                       Quadratic,
                       Cubic } finalSegmentType = SegmentType::Unknown;
    auto pathSegments((*iter)->segments());
    m_handles.push_back(HandlePoint(pathSegments.front()->StartPoint()));
    for (auto iter = pathSegments.cbegin(); iter != pathSegments.cend(); ++iter) {
        std::shared_ptr<SketchPathSegment> pathSegment = *iter;

        std::shared_ptr<SketchQuadraticBezier> quadratic;
        std::shared_ptr<SketchCubicBezier> cubic;
        if (std::dynamic_pointer_cast<SketchLine>(pathSegment) != nullptr) {
            m_handles.push_back(HandlePoint(pathSegment->EndPoint()));
            finalSegmentType = SegmentType::Line;
        } else if ((quadratic = std::dynamic_pointer_cast<SketchQuadraticBezier>(pathSegment)) != nullptr) {
            m_handles.push_back(HandlePoint(quadratic->ControlPoint(), HandlePointType::QuadraticControlPt));
            m_handles.push_back(HandlePoint(quadratic->EndPoint(), HandlePointType::QuadraticCurveEnd));
            finalSegmentType = SegmentType::Quadratic;
        } else if ((cubic = std::dynamic_pointer_cast<SketchCubicBezier>(pathSegment)) != nullptr) {
            m_handles.push_back(HandlePoint(cubic->ControlPoint1(), HandlePointType::CubicControlPt1));
            m_handles.push_back(HandlePoint(cubic->ControlPoint2(), HandlePointType::CubicControlPt2));
            m_handles.push_back(HandlePoint(cubic->EndPoint(), HandlePointType::CubicCurveEnd));
            finalSegmentType = SegmentType::Cubic;
        }
    }

    if ((*iter)->isClosed() && m_handles.size() >= 3) {
        switch (finalSegmentType) {
        case SegmentType::Line:
            m_ClosedState = SketchCanvasPathState::LineToNewPoint;
            break;
        case SegmentType::Quadratic:
            m_handles.pop_back();
            m_ClosedState = SketchCanvasPathState::QuadraticCurveToNewPoint;
            break;
        case SegmentType::Cubic:
            m_handles.pop_back();
            m_ClosedState = SketchCanvasPathState::CubicCurveToNewPoint;
            break;
        case SegmentType::Unknown:
            m_ClosedState = SketchCanvasPathState::Undefined;
            break;
        }
        m_pathClosed = true;
    }

    Refresh();
}

void SketchCanvasPanel::UpdatePathFromHandles(long handleIndex)
{
    SketchEffectSketch& sketch(m_sketchCanvasParent->GetSketch());
    auto paths(sketch.paths());

    auto pathIndex = m_sketchCanvasParent->GetSelectedPathIndex();
    if (pathIndex < 0 || pathIndex >= paths.size())
        return;

    auto iter = paths.cbegin();
    std::advance(iter, pathIndex);

    auto segments = (*iter)->segments();
    if (segments.empty())
        return;

    int index = 0;
    auto normalizedHandlePt(m_handles[handleIndex].pt);

    // Can early-return when adjusting the start-point of the path
    if (handleIndex == index++) {
        segments.front()->SetStartPoint(normalizedHandlePt);

        // yuck... closed paths have an extra SketchLine to close the path, so if we update
        //         the start point, we need to update that last 'extra' segment
        if ((*iter)->isClosed())
            segments.back()->SetEndPoint(normalizedHandlePt);
        m_sketchCanvasParent->NotifySketchUpdated();
        return;
    }

    for (int segmentIndex = 0; segmentIndex < segments.size(); ++segmentIndex) {
        std::shared_ptr<SketchPathSegment> segment = segments[segmentIndex];
        std::shared_ptr<SketchQuadraticBezier> quadratic;
        std::shared_ptr<SketchCubicBezier> cubic;

        if (std::dynamic_pointer_cast<SketchLine>(segment) != nullptr) {
            if (handleIndex == index++) {
                segment->SetEndPoint(normalizedHandlePt);
                if (segmentIndex < segments.size() - 1)
                    segments[segmentIndex + 1]->SetStartPoint(normalizedHandlePt);
                break;
            }
        } else if ((quadratic = std::dynamic_pointer_cast<SketchQuadraticBezier>(segment)) != nullptr) {
            if (handleIndex == index++) {
                quadratic->SetControlPoint(normalizedHandlePt);
                break;
            }
            if (handleIndex == index++) {
                segment->SetEndPoint(normalizedHandlePt);
                if (segmentIndex < segments.size() - 1)
                    segments[segmentIndex + 1]->SetStartPoint(normalizedHandlePt);
                break;
            }
        } else if ((cubic = std::dynamic_pointer_cast<SketchCubicBezier>(segment)) != nullptr) {
            if (handleIndex == index++) {
                cubic->SetControlPoint1(normalizedHandlePt);
                break;
            }
            if (handleIndex == index++) {
                cubic->SetControlPoint2(normalizedHandlePt);
                break;
            }
            if (handleIndex == index++) {
                segment->SetEndPoint(normalizedHandlePt);
                if (segmentIndex < segments.size() - 1)
                    segments[segmentIndex + 1]->SetStartPoint(normalizedHandlePt);
                break;
            }
        }
    }
    m_sketchCanvasParent->NotifySketchUpdated();
}

void SketchCanvasPanel::UpdatePathFromHandles()
{
    if (m_handles.size() < 2)
        return;

    SketchEffectSketch& sketch(m_sketchCanvasParent->GetSketch());
    auto pathIndex = m_sketchCanvasParent->GetSelectedPathIndex();
    if (pathIndex < 0 || pathIndex >= sketch.pathCount())
        return;

    auto path = std::make_shared<SketchEffectPath>();
    auto startPt = m_handles.front().pt;
    for (size_t i = 1; i < m_handles.size();) {
        std::shared_ptr<SketchPathSegment> segment;
        switch (m_handles[i].handlePointType) {
        case HandlePointType::Point:
            segment = std::make_shared<SketchLine>(startPt, m_handles[i].pt);
            ++i;
            break;
        case HandlePointType::QuadraticControlPt:
            if( !m_pathClosed || (i < m_handles.size() - 1) ) {
                segment = std::make_shared<SketchQuadraticBezier>(startPt, m_handles[i].pt, m_handles[i + 1].pt);
            }
            i += 2;
            break;
        case HandlePointType::CubicControlPt1:
            if( !m_pathClosed || (i < m_handles.size() - 1) ) {
                segment = std::make_shared<SketchCubicBezier>(startPt, m_handles[i].pt, m_handles[i + 1].pt, m_handles[i + 2].pt);
            }
            i += 3;
            break;
        default:
            break;
        }
        path->appendSegment(segment);
        startPt = segment->EndPoint();
    }
    if (m_pathClosed)
        path->closePath(true, m_ClosedState);

    sketch.updatePath(pathIndex, path);
    m_sketchCanvasParent->NotifySketchUpdated();
}

std::shared_ptr<SketchEffectPath> SketchCanvasPanel::CreatePathFromHandles() const
{
    if (m_handles.size() < 2)
        return nullptr;

    int final_handle = 0;
    auto path = std::make_shared<SketchEffectPath>();
    for (size_t index = 0; index < m_handles.size() - 1;) {
        std::shared_ptr<SketchPathSegment> segment;

        switch (m_handles[index + 1].handlePointType) {
        case HandlePointType::Point:
            final_handle = index + 1;
            if( m_pathClosed && (index == m_handles.size() - 1) ) {
                final_handle = 0;
            }
            segment = std::make_shared<SketchLine>(m_handles[index].pt, m_handles[final_handle].pt);
            ++index;
            break;
        case HandlePointType::QuadraticControlPt:
            final_handle = index + 2;
            if( m_pathClosed && (index == m_handles.size() - 2) ) {
                final_handle = 0;
            }
            segment = std::make_shared<SketchQuadraticBezier>(m_handles[index].pt,
                                                              m_handles[index + 1].pt,
                                                              m_handles[final_handle].pt);
            index += 2;
            break;
        case HandlePointType::CubicControlPt1:
            final_handle = index + 3;
            if( m_pathClosed && (index == m_handles.size() - 3) ) {
                final_handle = 0;
            }
            segment = std::make_shared<SketchCubicBezier>(m_handles[index].pt,
                                                          m_handles[index + 1].pt,
                                                          m_handles[index + 2].pt,
                                                          m_handles[final_handle].pt);
            index += 3;
            break;
        default:
            break;
        }

        if (segment != nullptr)
            path->appendSegment(segment);
    }
    if (m_pathClosed)
        path->closePath(false, m_ClosedState);

    return path;
}

bool SketchCanvasPanel::HandleHoveredOrGrabbed() const
{
    for (const auto& handle : m_handles)
        if (handle.state)
            return true;
    return false;
}

void SketchCanvasPanel::clearBackgroundBitmap()
{
    m_bgBitmap = nullptr;
    Refresh();
}

void SketchCanvasPanel::setBackgroundBitmap(std::unique_ptr<wxBitmap> bm)
{
    m_bgBitmap = std::move(bm);
    Refresh();
}

void SketchCanvasPanel::ClosePath()
{
    if( !m_pathClosed ) {
        m_pathClosed = true;
        std::shared_ptr<SketchPathSegment> segment;
        wxPoint2DDouble startPt = m_handles.back().pt;
        wxPoint2DDouble endPt = m_handles.front().pt;

        if (m_ClosedState == SketchCanvasPathState::LineToNewPoint) {
            segment = std::make_shared<SketchLine>(startPt, endPt);
        } else if (m_ClosedState == SketchCanvasPathState::QuadraticCurveToNewPoint) {
            wxPoint2DDouble cp = 0.5 * startPt + 0.5 * endPt;
            m_handles.push_back(HandlePoint(cp, HandlePointType::QuadraticControlPt));
            segment = std::make_shared<SketchQuadraticBezier>(startPt, cp, endPt);
        } else if (m_ClosedState == SketchCanvasPathState::CubicCurveToNewPoint) {
            wxPoint2DDouble cp1 = 0.75 * startPt + 0.25 * endPt;
            wxPoint2DDouble cp2 = 0.25 * startPt + 0.75 * endPt;
            m_handles.push_back(HandlePoint(cp1, HandlePointType::CubicControlPt1));
            m_handles.push_back(HandlePoint(cp2, HandlePointType::CubicControlPt2));
            segment = std::make_shared<SketchCubicBezier>(startPt, cp1, cp2, endPt);
        }

        if (segment != nullptr) {
            SketchEffectSketch& sketch(m_sketchCanvasParent->GetSketch());
            auto pathIndex = m_sketchCanvasParent->GetSelectedPathIndex();
            if (pathIndex < 0 || pathIndex >= sketch.pathCount())
                return;
            auto paths(sketch.paths());
            paths[pathIndex]->appendSegment(segment);
            paths[pathIndex]->closePath(true, m_ClosedState);
        }
    }
}

void SketchCanvasPanel::Changed()
{
    Refresh();
}
