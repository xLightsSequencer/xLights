#include "SketchCanvasPanel.h"
#include "../assist/SketchAssistPanel.h"

#include <wx/dcbuffer.h>
#include <wx/filepicker.h>
#include <wx/graphics.h>
#include <wx/listbox.h>

namespace
{
    const int BorderWidth = 5;
 
    const int MouseWheelLimit = 1440;

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


SketchCanvasPanel::SketchCanvasPanel(SketchAssistPanel* parentPanel, wxWindow*parent, wxWindowID id /*=wxID_ANY*/, const wxPoint& pos /*= wxDefaultPosition*/, const wxSize& size /*=wxDefaultSize*/) :
    wxPanel(parent, id, pos, size, wxNO_BORDER | wxWANTS_CHARS),
    m_parentPanel(parentPanel)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
}

void SketchCanvasPanel::OnSketchPaint(wxPaintEvent& /*event*/)
{
    wxAutoBufferedPaintDC pdc(this);
    wxSize sz(GetSize());
    wxRect borderRect(BorderWidth, BorderWidth, sz.GetWidth() - 2 * BorderWidth - 2, sz.GetHeight() - 2 * BorderWidth - 2);
    wxRect bgRect(wxRect(borderRect).Deflate(1, 1));

    pdc.SetPen(*wxWHITE_PEN);
    pdc.SetBrush(*wxWHITE_BRUSH);
    pdc.DrawRectangle(wxPoint(0, 0), sz);

    pdc.SetPen(*wxLIGHT_GREY_PEN);
    pdc.DrawRectangle(borderRect);

    {
        std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(pdc));
        double zoomLevel = 1.;
        if (m_wheelRotation) {
            zoomLevel = interpolate(m_wheelRotation, 0, 1., MouseWheelLimit, 8., LinearInterpolater());
            wxGraphicsMatrix m = gc->CreateMatrix();
            wxPoint2DDouble pt(NormalizedToUI(m_normalizedZoomPt));
            m.Translate(-m_canvasTranslation.m_x, -m_canvasTranslation.m_y);
            m.Translate(pt.m_x, pt.m_y);
            m.Scale(zoomLevel, zoomLevel);
            m.Translate(-pt.m_x, -pt.m_y);
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

        long selectedPathIndex = m_parentPanel->m_pathsListBox->GetSelection();
        long pathIndex = 0;
        for (const auto& path : m_parentPanel->m_sketch.paths()) {
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
        pen = gc->CreatePen(wxGraphicsPenInfo(*wxBLACK, 1. / zoomLevel));
        gc->SetPen(pen);
        std::vector<wxPoint>::size_type n = m_handles.size();
        wxGraphicsPath path(gc->CreatePath());
        for (std::vector<wxPoint>::size_type i = 0; i < n; ++i) {
            wxPoint2DDouble pt(NormalizedToUI(m_handles[i].pt));
            if (i == 0)
                path.MoveToPoint(pt);
            else if (m_handles[i].handlePointType == Point)
                path.AddLineToPoint(pt);
            else if (m_handles[i].handlePointType == QuadraticControlPt) {
                auto endPt = NormalizedToUI(m_handles[i + 1].pt);
                path.AddQuadCurveToPoint(pt.m_x, pt.m_y, endPt.m_x, endPt.m_y);
                i += 1;
            } else if (m_handles[i].handlePointType == CubicControlPt1) {
                auto cp2 = NormalizedToUI(m_handles[i + 1].pt);
                auto endPt = NormalizedToUI(m_handles[i + 2].pt);
                path.AddCurveToPoint(pt, cp2, endPt);
                i += 2;
            }
        }

        if (m_pathClosed && m_handles.size() > 2)
            path.CloseSubpath();
        gc->DrawPath(path);

        // Next, if we are in a segment-adding state, draw that potential segment
        if ((m_pathState == LineToNewPoint || m_pathState == QuadraticCurveToNewPoint || m_pathState == CubicCurveToNewPoint) && !m_handles.empty()) {
            switch (m_pathState) {
            case LineToNewPoint:
                pen = gc->CreatePen(wxGraphicsPenInfo(*wxBLACK, 2. / zoomLevel, wxPENSTYLE_SHORT_DASH));
                break;
            case QuadraticCurveToNewPoint:
                pen = gc->CreatePen(wxGraphicsPenInfo(*wxRED, 2. / zoomLevel, wxPENSTYLE_SHORT_DASH));
                break;
            case CubicCurveToNewPoint:
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

            gc->DrawEllipse(pt.m_x - 4.5 / zoomLevel, pt.m_y - 4.5 / zoomLevel, 9 / zoomLevel, 9 / zoomLevel);
        }
    }
}

void SketchCanvasPanel::OnSketchKeyDown(wxKeyEvent& event)
{
    int keycode = event.GetKeyCode();
    if (keycode == WXK_DELETE && (m_pathState == Undefined || m_pathState == DefineStartPoint)) {
        int pathIndex = m_parentPanel->m_pathsListBox->GetSelection();
        std::optional<std::pair<int, int>> toErase;
        int index = 0;
        for (auto iter = m_handles.begin(); iter != m_handles.end(); ++iter, ++index) {
            if (iter->state) {
                switch (iter->handlePointType) {
                case Point:
                    toErase = std::make_pair(index, 1);
                    break;
                case QuadraticControlPt:
                    toErase = std::make_pair(index, 2);
                    break;
                case QuadraticCurveEnd:
                    toErase = std::make_pair(index - 1, 2);
                    break;
                case CubicControlPt1:
                    toErase = std::make_pair(index, 3);
                    break;
                case CubicControlPt2:
                    toErase = std::make_pair(index - 1, 3);
                    break;
                case CubicCurveEnd:
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
                SketchEffectSketch& sketch(m_parentPanel->m_sketch);
                sketch.deletePath(pathIndex);
                m_parentPanel->sketchUpdatedFromCanvasPanel();
                m_parentPanel->populatePathListBoxFromSketch();
            } else
                UpdatePathFromHandles();
        }

        Refresh();
    } else if (keycode == WXK_ESCAPE) {
        UpdatePathState(Undefined);
        m_parentPanel->sketchUpdatedFromCanvasPanel();
    } else if (keycode == WXK_SPACE) {
        m_pathClosed = true;
        UpdatePathState(Undefined);
        m_parentPanel->sketchUpdatedFromCanvasPanel();
    } else if (keycode == WXK_SHIFT) {
        switch (m_pathState) {
        case LineToNewPoint:
            m_pathState = QuadraticCurveToNewPoint;
            break;
        case QuadraticCurveToNewPoint:
            m_pathState = CubicCurveToNewPoint;
            break;
        case CubicCurveToNewPoint:
            m_pathState = LineToNewPoint;
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
    if (m_pathState == DefineStartPoint) {
        m_handles.push_back(pt);
        UpdatePathState(LineToNewPoint);
        return;
    } else if (m_pathState == LineToNewPoint) {
        m_handles.push_back(pt);
        Refresh();
        return;
    } else if (m_pathState == QuadraticCurveToNewPoint) {
        wxPoint2DDouble startPt = m_handles.back().pt;
        wxPoint2DDouble endPt = pt;
        wxPoint2DDouble cp = 0.5 * startPt + 0.5 * endPt;

        m_handles.push_back(HandlePoint(cp, QuadraticControlPt));
        m_handles.push_back(HandlePoint(endPt, QuadraticCurveEnd));
        Refresh();
        return;
    } else if (m_pathState == CubicCurveToNewPoint) {
        wxPoint2DDouble startPt = m_handles.back().pt;
        wxPoint2DDouble endPt = pt;
        wxPoint2DDouble cp1 = 0.75 * startPt + 0.25 * endPt;
        wxPoint2DDouble cp2 = 0.25 * startPt + 0.75 * endPt;

        m_handles.push_back(HandlePoint(cp1, CubicControlPt1));
        m_handles.push_back(HandlePoint(cp2, CubicControlPt2));
        m_handles.push_back(HandlePoint(endPt, CubicCurveEnd));
        Refresh();
        return;
    }

    // Updating 'grabbed' handle
    for (std::vector<HandlePoint>::size_type i = 0; i < m_handles.size(); ++i) {
        if (m_handles[i].state) {
            m_grabbedHandleIndex = i;
            break;
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

    if (m_pathState == LineToNewPoint || m_pathState == QuadraticCurveToNewPoint || m_pathState == CubicCurveToNewPoint) {
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

    if (m_grabbedHandleIndex != -1) {
        m.Invert();
        m_handles[m_grabbedHandleIndex].pt = UItoNormalized(m.TransformPoint(m_mousePos));
        Refresh();
    } else {
        bool somethingChanged = false;
        for (auto& handle : m_handles) {
            wxPoint2DDouble handlePos(NormalizedToUI(handle.pt));
            wxPoint2DDouble transformedHandlePos(m.TransformPoint(handlePos));
            bool state = m_mousePos.GetDistanceSquare(transformedHandlePos) <= 20.25;
            if (state != handle.state) {
                handle.state = state;
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
    if (!m_wheelRotation)
        m_canvasTranslation = wxPoint2DDouble();

    // todo? - take zoom and/or canvas translation into account
    m_normalizedZoomPt = UItoNormalized(event.GetPosition());

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
    wxSize sz(GetSize() - wxSize(2 * BorderWidth - 2, 2 * BorderWidth - 2));

    double x = double(pt.m_x - o.x) / sz.GetWidth();
    double y = double(pt.m_y - o.y) / sz.GetHeight();

    return wxPoint2DDouble(x, 1. - y);
}

wxPoint2DDouble SketchCanvasPanel::NormalizedToUI(const wxPoint2DDouble& pt) const
{
    wxPoint o(BorderWidth + 1, BorderWidth + 1);
    wxSize sz(GetSize() - wxSize(2 * BorderWidth - 2, 2 * BorderWidth - 2));

    double x = pt.m_x * sz.GetWidth();
    double y = pt.m_y * sz.GetHeight();
    return wxPoint2DDouble(o.x + x, o.y + (sz.GetHeight() - y - 1));
}

bool SketchCanvasPanel::IsControlPoint(const HandlePoint& handlePt)
{
    auto hpt = handlePt.handlePointType;
    return hpt == QuadraticControlPt || hpt == CubicControlPt1 || hpt == CubicControlPt2;
}

void SketchCanvasPanel::UpdatePathState(PathState pathState)
{
    m_pathState = pathState;

    switch (m_pathState) {
    case Undefined:
        m_parentPanel->m_startPathBtn->Enable();
        m_parentPanel->m_endPathBtn->Disable();
        m_parentPanel->m_closePathBtn->Disable();
        break;
    case DefineStartPoint:
        m_parentPanel->m_startPathBtn->Disable();
        m_parentPanel->m_endPathBtn->Disable();
        m_parentPanel->m_closePathBtn->Disable();
        break;
    case LineToNewPoint:
    case QuadraticCurveToNewPoint:
    case CubicCurveToNewPoint:
        m_parentPanel->m_startPathBtn->Disable();
        m_parentPanel->m_endPathBtn->Enable();
        m_parentPanel->m_closePathBtn->Enable();
        break;
    }

    Refresh();

    // If we're Undefined, have some handles, and no path
    // selected, I think we've added a new one!!
    if (m_pathState == Undefined && !m_handles.empty() && m_parentPanel->m_pathsListBox->GetSelection() < 0) {
        auto path = CreatePathFromHandles();
        if (path != nullptr) {
            SketchEffectSketch& sketch(m_parentPanel->m_sketch);
            sketch.appendPath(path);
            int n = static_cast<int>(sketch.pathCount());
            wxString str;
            str.sprintf("Path %d", n);
            m_parentPanel->m_pathsListBox->Insert(str, n - 1);
            m_parentPanel->m_pathsListBox->Select(n - 1);
        }
    }
}

void SketchCanvasPanel::ResetHandlesState(PathState state /*Undefined*/)
{
    m_handles.clear();
    m_grabbedHandleIndex = -1;
    m_pathClosed = false;
    UpdatePathState(state);
}

void SketchCanvasPanel::UpdateHandlesForPath(long pathIndex)
{
    const SketchEffectSketch& sketch(m_parentPanel->m_sketch);

    if (pathIndex < 0 || pathIndex >= sketch.paths().size())
        return;

    ResetHandlesState();

    auto iter = sketch.paths().cbegin();
    std::advance(iter, pathIndex);

    auto pathSegments((*iter)->segments());
    m_handles.push_back(HandlePoint(pathSegments.front()->StartPoint()));
    for (auto iter = pathSegments.cbegin(); iter != pathSegments.cend(); ++iter) {
        std::shared_ptr<SketchPathSegment> pathSegment = *iter;

        std::shared_ptr<SketchQuadraticBezier> quadratic;
        std::shared_ptr<SketchCubicBezier> cubic;
        if (std::dynamic_pointer_cast<SketchLine>(pathSegment) != nullptr) {
            m_handles.push_back(HandlePoint(pathSegment->EndPoint()));
        } else if ((quadratic = std::dynamic_pointer_cast<SketchQuadraticBezier>(pathSegment)) != nullptr) {
            m_handles.push_back(HandlePoint(quadratic->ControlPoint(), QuadraticControlPt));
            m_handles.push_back(HandlePoint(quadratic->EndPoint(), QuadraticCurveEnd));
        } else if ((cubic = std::dynamic_pointer_cast<SketchCubicBezier>(pathSegment)) != nullptr) {
            m_handles.push_back(HandlePoint(cubic->ControlPoint1(), CubicControlPt1));
            m_handles.push_back(HandlePoint(cubic->ControlPoint2(), CubicControlPt2));
            m_handles.push_back(HandlePoint(cubic->EndPoint(), CubicCurveEnd));
        }
    }

    if ((*iter)->isClosed() && m_handles.size() >= 3) {
        m_handles.pop_back();
        m_pathClosed = true;
    }

    Refresh();
}

void SketchCanvasPanel::UpdatePathFromHandles(long handleIndex)
{
    SketchEffectSketch& sketch(m_parentPanel->m_sketch);
    auto paths(sketch.paths());

    auto pathIndex = m_parentPanel->m_pathsListBox->GetSelection();
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
        m_parentPanel->sketchUpdatedFromCanvasPanel();
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
    m_parentPanel->sketchUpdatedFromCanvasPanel();
}

void SketchCanvasPanel::UpdatePathFromHandles()
{
    // todo
}

std::shared_ptr<SketchEffectPath> SketchCanvasPanel::CreatePathFromHandles() const
{
    if (m_handles.size() < 2)
        return nullptr;

    auto path = std::make_shared<SketchEffectPath>();
    for (size_t index = 0; index < m_handles.size() - 1;) {
        std::shared_ptr<SketchPathSegment> segment;

        switch (m_handles[index + 1].handlePointType) {
        case Point:
            segment = std::make_shared<SketchLine>(m_handles[index].pt, m_handles[index + 1].pt);
            ++index;
            break;
        case QuadraticControlPt:
            segment = std::make_shared<SketchQuadraticBezier>(m_handles[index].pt,
                                                              m_handles[index + 1].pt,
                                                              m_handles[index + 2].pt);
            index += 2;
            break;
        case CubicControlPt1:
            segment = std::make_shared<SketchCubicBezier>(m_handles[index].pt,
                                                          m_handles[index + 1].pt,
                                                          m_handles[index + 2].pt,
                                                          m_handles[index + 3].pt);
            index += 3;
            break;
        default:
            break;
        }

        if (segment != nullptr)
            path->appendSegment(segment);
    }
    if (m_pathClosed)
        path->closePath();

    return path;
}

void SketchCanvasPanel::setBackgroundBitmap(std::unique_ptr<wxBitmap> bm)
{
    m_bgBitmap = std::move(bm);
    Refresh();
}
