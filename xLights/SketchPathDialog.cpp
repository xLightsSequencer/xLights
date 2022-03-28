#include "SketchPathDialog.h"

#include <optional>
#include <utility>
#include <xutility>

#include <wx/brush.h>
#include <wx/button.h>
#include <wx/dcbuffer.h>
#include <wx/graphics.h>
#include <wx/listctrl.h>
#include <wx/panel.h>
#include <wx/pen.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

BEGIN_EVENT_TABLE(SketchPathDialog, wxDialog)
END_EVENT_TABLE()

namespace
{
    const int BorderWidth = 5;

    const char HotkeysText[] =
        "Shift\tToggle segment type (line, one-point curve, two-point curve)\n"
        "Esc\tEnd current path\n"
        "Space\tClose current path\n"
        "Delete\tDelete point/segment\n";
}

SketchPathDialog::SketchPathDialog(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size)
{
    Create(parent, wxID_ANY, "Define Sketch", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxCLOSE_BOX, _T("wxID_ANY"));

    auto mainSizer = new wxFlexGridSizer(3, 1, 0, 0);
    mainSizer->AddGrowableCol(0);
    mainSizer->AddGrowableRow(0);

    auto pathUISizer = new wxFlexGridSizer(3, 1, 5, 0);
    pathUISizer->AddGrowableRow(0);
    pathUISizer->AddGrowableCol(0);

    // canvas
    auto frame = new wxStaticBox(this, wxID_ANY, wxEmptyString);
    auto frameSizer = new wxFlexGridSizer(1, 1, 0, 0);
    frameSizer->AddGrowableRow(0);
    frameSizer->AddGrowableCol(0);
    m_sketchPanel = new wxPanel(frame, wxID_ANY, wxDefaultPosition, wxSize(400, 300), wxALL | wxEXPAND);
    m_sketchPanel->SetBackgroundStyle(wxBG_STYLE_PAINT);
    frameSizer->Add(m_sketchPanel, 1, wxALL | wxEXPAND);
    frame->SetSizer(frameSizer);
    pathUISizer->Add(frame, 1, wxALL | wxEXPAND, 5);

    // background image controls
    auto bgSizer = new wxFlexGridSizer(1, 3, 0, 0);
    bgSizer->AddGrowableCol(1);
    pathUISizer->Add(bgSizer, 1, wxALL | wxEXPAND);
    auto bgLabel = new wxStaticText(this, wxID_ANY, "Background:");
    auto bgTextCtrl = new wxTextCtrl(this, wxID_ANY);
    // bgTextCtrl->Disable();
    bgTextCtrl->SetEditable(false);
    auto chooseBgBtn = new wxButton(this, wxID_ANY, "...", wxDefaultPosition, wxDLG_UNIT(this, wxSize(16, -1)));
    bgSizer->Add(bgLabel, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);
    bgSizer->Add(bgTextCtrl, 1, wxALL | wxEXPAND, 2);
    bgSizer->Add(chooseBgBtn, 1, wxALL, 2);

    // path controls
    auto pathCtrlsSizer = new wxFlexGridSizer(1, 4, 0, 0);
    pathCtrlsSizer->AddGrowableCol(3);
    pathUISizer->Add(pathCtrlsSizer, 1, wxALL | wxEXPAND);
    m_startPathBtn = new wxButton(this, wxID_ANY, "Start Path");
    pathCtrlsSizer->Add(m_startPathBtn, 1, wxALL, 2);
    m_endPathBtn = new wxButton(this, wxID_ANY, "End Path");
    pathCtrlsSizer->Add(m_endPathBtn, 1, wxALL, 2);
    m_closePathBtn = new wxButton(this, wxID_ANY, "Close Path");
    pathCtrlsSizer->Add(m_closePathBtn, 1, wxALL, 2);

    mainSizer->Add(pathUISizer, 1, wxALL | wxEXPAND, 5);

    // Sketch UI
    auto sketchUISizer = new wxFlexGridSizer(2, 1, 5, 0);
    sketchUISizer->AddGrowableRow(1);
    sketchUISizer->AddGrowableCol(0);
    auto hotkeysSizer = new wxStaticBoxSizer(wxVERTICAL, this, "Canvas hotkeys");
    hotkeysSizer->Add(new wxStaticText(hotkeysSizer->GetStaticBox(), wxID_ANY, HotkeysText), 1, wxALL | wxEXPAND, 3);

    m_pathsListView = new wxListView(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
    //m_pathsListView->AppendColumn("Paths", wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);

    sketchUISizer->Add(hotkeysSizer, 1, wxALL | wxEXPAND);
    sketchUISizer->Add(m_pathsListView, 1, wxALL | wxEXPAND, 5);
    mainSizer->Add(sketchUISizer, 1, wxALL | wxEXPAND, 5);

    // Ok / Cancel
    auto okCancelSizer = new wxFlexGridSizer(1, 3, 0, 0);
    okCancelSizer->AddGrowableCol(0);
    okCancelSizer->AddStretchSpacer();
    auto okButton = new wxButton(this, wxID_ANY, "Ok");
    okCancelSizer->Add(okButton, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    auto cancelButton = new wxButton(this, wxID_ANY, "Cancel");
    okCancelSizer->Add(cancelButton, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    mainSizer->Add(okCancelSizer, 1, wxALL | wxEXPAND, 5);

    SetSizer(mainSizer);
    mainSizer->Fit(this);
    mainSizer->SetSizeHints(this);

    m_sketchPanel->Connect(wxEVT_PAINT, (wxObjectEventFunction)&SketchPathDialog::OnSketchPaint, nullptr, this);
    m_sketchPanel->Connect(wxEVT_KEY_DOWN, (wxObjectEventFunction)&SketchPathDialog::OnSketchKeyDown, nullptr, this);
    m_sketchPanel->Connect(wxEVT_LEFT_DOWN, (wxObjectEventFunction)&SketchPathDialog::OnSketchLeftDown, nullptr, this);
    m_sketchPanel->Connect(wxEVT_LEFT_UP, (wxObjectEventFunction)&SketchPathDialog::OnSketchLeftUp, nullptr, this);
    m_sketchPanel->Connect(wxEVT_MOTION, (wxObjectEventFunction)&SketchPathDialog::OnSketchMouseMove, nullptr, this);

    Connect(m_startPathBtn->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SketchPathDialog::OnButton_StartPath);
    Connect(m_endPathBtn->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SketchPathDialog::OnButton_EndPath);
    Connect(m_closePathBtn->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SketchPathDialog::OnButton_ClosePath);

    Connect(m_pathsListView->GetId(), wxEVT_LIST_ITEM_SELECTED, (wxObjectEventFunction)&SketchPathDialog::OnListView_PathSelected);

    Connect(okButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SketchPathDialog::OnButton_Ok);
    Connect(cancelButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SketchPathDialog::OnButton_Cancel);

    UpdatePathState(Undefined);
}

void SketchPathDialog::setSketch(const std::string& sketchStr)
{
    m_sketch = SketchEffectSketch::SketchFromString(sketchStr);

    m_pathsListView->ClearAll();
    m_pathsListView->AppendColumn("Paths", wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);

    int i = 0;
    for (const auto& path : m_sketch.paths()) {
        wxString text;
        text.sprintf("Path %d", i + 1);
        m_pathsListView->InsertItem(i++, text);
    }
}

std::string SketchPathDialog::sketchDefString() const
{
    return m_sketch.toString();
}

void SketchPathDialog::OnSketchPaint(wxPaintEvent& event)
{
    wxAutoBufferedPaintDC pdc(m_sketchPanel);
    wxSize sz(m_sketchPanel->GetSize());

    pdc.SetPen(*wxWHITE_PEN);
    pdc.SetBrush(*wxWHITE_BRUSH);
    pdc.DrawRectangle(wxPoint(0, 0), sz);

    pdc.SetPen(*wxLIGHT_GREY_PEN);
    pdc.DrawRectangle(BorderWidth, BorderWidth, sz.GetWidth() - 2 * BorderWidth-2, sz.GetHeight() - 2 * BorderWidth-2);

    {
        std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(pdc));

        // First, draw the unselected path(s)
        gc->SetPen(*wxLIGHT_GREY_PEN);

        long selectedPathIndex = m_pathsListView->GetFirstSelected();
        long pathIndex = 0;
        for (const auto& path : m_sketch.paths() ) {
            if (pathIndex++ == selectedPathIndex)
                continue;
            wxGraphicsPath graphicsPath(gc->CreatePath());
            const auto& firstSegment( path->segments().front() );
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

        // Second, draw the selected path
        gc->SetPen(*wxBLACK_PEN);
        std::vector<wxPoint>::size_type n = m_handles.size();
        wxGraphicsPath path( gc->CreatePath() );
        for (std::vector<wxPoint>::size_type i = 0; i < n; ++i)
        {
            wxPoint2DDouble pt(NormalizedToUI(m_handles[i].pt));
            if (i == 0)
                path.MoveToPoint(pt);
            else if (m_handles[i].handlePointType == Point)
                path.AddLineToPoint(pt);
            else if (m_handles[i].handlePointType == QuadraticControlPt) {
                auto endPt = NormalizedToUI(m_handles[i + 1].pt);
                path.AddQuadCurveToPoint(pt.m_x, pt.m_y, endPt.m_x, endPt.m_y);
                i += 1;
            }
            else if (m_handles[i].handlePointType == CubicControlPt1)
            {
                auto cp2 = NormalizedToUI(m_handles[i + 1].pt);
                auto endPt = NormalizedToUI(m_handles[i + 2].pt);
                path.AddCurveToPoint(pt, cp2, endPt);
                i += 2;
            }
        }

        if (m_pathClosed && m_handles.size() > 2)
            path.CloseSubpath();
        gc->DrawPath(path);

        // Third, if we are in a segment-adding state, draw that potential segment
        if ((m_pathState == LineToNewPoint || m_pathState == QuadraticCurveToNewPoint || m_pathState == CubicCurveToNewPoint) && !m_handles.empty())
        {
            wxPen pen;
            switch (m_pathState) {
            case LineToNewPoint:
                pen = *wxBLACK;
                break;
            case QuadraticCurveToNewPoint:
                pen = *wxRED;
                break;
            case CubicCurveToNewPoint:
                pen = *wxBLUE;
                break;
            }
            wxDash dashes[2] = { 2, 3 };
            pen.SetStyle(wxPENSTYLE_USER_DASH);
            pen.SetDashes(2, dashes);
            pen.SetWidth(2);
            gc->SetPen(pen);
            auto ptFrom = NormalizedToUI(m_handles.back().pt);
            auto ptTo(m_mousePos);
            gc->StrokeLine(ptFrom.m_x, ptFrom.m_y, ptTo.m_x, ptTo.m_y);

            gc->SetPen(*wxBLACK_PEN);
        }

        // Fourth, draw the handles
        for (std::vector<wxPoint>::size_type i = 0; i < n; ++i)
        {
            wxPoint2DDouble pt(NormalizedToUI(m_handles[i].pt));
            if (i == 0 && !m_handles[i].state)
                gc->SetBrush(*wxGREEN_BRUSH);
            else if (i == n - 1 && !m_handles[i].state)
                gc->SetBrush(*wxRED_BRUSH);
            else
                gc->SetBrush((m_handles[i].state) ? (*wxYELLOW_BRUSH)
                                                  : (isControlPoint(m_handles[i]) ? (*wxBLUE_BRUSH) : (*wxLIGHT_GREY_BRUSH)));

             gc->DrawEllipse(pt.m_x - 4, pt.m_y - 4, 8, 8);
        }
    }
}

void SketchPathDialog::OnSketchKeyDown(wxKeyEvent& event)
{
    int keycode = event.GetKeyCode();
    if (keycode == WXK_DELETE) {
        std::optional<std::pair<int,int>> toErase;
        int index = 0;
        for (auto iter = m_handles.begin(); iter != m_handles.end(); ++iter, ++index) {
            if (iter->state) {
                switch (iter->handlePointType) {
                case Point:
                    m_handles.erase(iter);
                    break;
                case QuadraticControlPt:
                    toErase = std::make_pair(index, 2);
                    break;
                case QuadraticCurveEnd:
                    toErase = std::make_pair(index-1, 2);
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
        if (toErase.has_value() ) {
            auto startIter = m_handles.cbegin();
            auto endIter = m_handles.cbegin();
            auto p = toErase.value();
            std::advance(startIter, p.first);
            std::advance(endIter, p.second + p.first);
            m_handles.erase(startIter, endIter);

            // todo - we shouldn't end up with orpaned control points or curve ends
            //        but deleting curves sometimes does unexpected things... maybe
            //        instead of deleting CurveEnd points they should be just changed
            //        to Points... unless it is the CurveEnd point that is being
            //        deleted? It's not entirely clear what the expected behavior
            //        might be though for deleting curve points.
        }
        if (m_handles.empty()) {
            UpdatePathState(Undefined);
            m_pathClosed = false;
        } else if (m_handles.size() == 1) {
            UpdatePathState(LineToNewPoint);
            m_pathClosed = false;
        } else
            m_sketchPanel->Refresh();
    }
    else if (keycode == WXK_ESCAPE) {
        UpdatePathState(Undefined);
    } else if (keycode == WXK_SPACE) {
        m_pathClosed = true;
        UpdatePathState(Undefined);
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
        }
        m_sketchPanel->Refresh();
    }
}

void SketchPathDialog::OnSketchLeftDown(wxMouseEvent& event)
{
    m_sketchPanel->SetFocus();

    if (m_pathState == DefineStartPoint)
    {
        m_handles.push_back(UItoNormalized(event.GetPosition()));
        UpdatePathState(LineToNewPoint);
        return;
    }
    else if (m_pathState == LineToNewPoint)
    {
        m_handles.push_back(UItoNormalized(event.GetPosition()));
        m_sketchPanel->Refresh();
        return;
    } else if ( m_pathState == QuadraticCurveToNewPoint )
    {
        wxPoint2DDouble startPt = m_handles.back().pt;
        wxPoint2DDouble endPt = UItoNormalized(event.GetPosition());
        wxPoint2DDouble cp = 0.5 * startPt + 0.5 * endPt;

        m_handles.push_back(HandlePoint(cp, QuadraticControlPt));
        m_handles.push_back(HandlePoint(endPt, QuadraticCurveEnd));
        m_sketchPanel->Refresh();
        return;
    }
    else if (m_pathState == CubicCurveToNewPoint)
    {
        wxPoint2DDouble startPt = m_handles.back().pt;
        wxPoint2DDouble endPt = UItoNormalized(event.GetPosition());
        wxPoint2DDouble cp1 = 0.75 * startPt + 0.25 * endPt;
        wxPoint2DDouble cp2 = 0.25 * startPt + 0.75 * endPt;

        m_handles.push_back(HandlePoint(cp1, CubicControlPt1));
        m_handles.push_back(HandlePoint(cp2, CubicControlPt2));
        m_handles.push_back(HandlePoint(endPt, CubicCurveEnd));
        m_sketchPanel->Refresh();
        return;
    }

    for ( std::vector<HandlePoint>::size_type i = 0; i < m_handles.size(); ++i)
    {
        if (m_handles[i].state)
        {
            m_grabbedHandleIndex = i;
            break;
        }
    }
}

void SketchPathDialog::OnSketchLeftUp(wxMouseEvent& /*event*/)
{
    if (m_grabbedHandleIndex != -1) {
        UpdatePathForHandles(m_grabbedHandleIndex);
        // temp for debugging handles-path synchronization
        //UpdateHandlesForPath(m_pathsListView->GetFirstSelected());
    }

    m_grabbedHandleIndex = -1;
}

void SketchPathDialog::OnSketchMouseMove(wxMouseEvent& event)
{
    m_mousePos = event.GetPosition();

    if (m_pathState == LineToNewPoint || m_pathState == QuadraticCurveToNewPoint || m_pathState == CubicCurveToNewPoint)
    {
        m_sketchPanel->Refresh();
        return;
    }

    if ( m_grabbedHandleIndex != -1 )
    {
        m_handles[m_grabbedHandleIndex].pt = UItoNormalized(m_mousePos);
        m_sketchPanel->Refresh();
    }
    else
    {
        bool somethingChanged = false;
        for (auto& handle : m_handles)
        {
            wxPoint2DDouble handlePos(NormalizedToUI(handle.pt));
            bool state = m_mousePos.GetDistanceSquare(handlePos) <= 16.;
            if (state != handle.state)
            {
                handle.state = state;
                somethingChanged = true;
            }
        }

        if (somethingChanged)
            m_sketchPanel->Refresh();
    }
}

void SketchPathDialog::OnButton_StartPath(wxCommandEvent& /*event*/)
{
    m_handles.clear();
    m_grabbedHandleIndex = -1;
    m_pathClosed = false;
    UpdatePathState(DefineStartPoint);

    // I think we want no path selected while a path is being defined?
    m_pathsListView->Select(m_pathsListView->GetFirstSelected(), false);
}

void SketchPathDialog::OnButton_EndPath(wxCommandEvent& /*event*/)
{
    UpdatePathState(Undefined);
}

void SketchPathDialog::OnButton_ClosePath(wxCommandEvent& /*event*/)
{
    m_pathClosed = true;
    UpdatePathState(Undefined);
}

void SketchPathDialog::OnListView_PathSelected(wxCommandEvent& /*event*/)
{
    UpdateHandlesForPath( m_pathsListView->GetFirstSelected() );
}

void SketchPathDialog::OnButton_Ok(wxCommandEvent& /*event*/)
{
    EndDialog(wxID_OK);
}

void SketchPathDialog::OnButton_Cancel(wxCommandEvent& /*event*/)
{
    EndDialog(wxID_CANCEL);
}

// Usable area rect: BorderWidth+1, BorderWidth+1, sz.GetWidth()-2*BorderWidth-2, sz.GetHeight()-2*BorderWidth-2;

wxPoint2DDouble SketchPathDialog::UItoNormalized(const wxPoint2DDouble& pt) const
{
    wxPoint o(BorderWidth + 1, BorderWidth + 1);
    wxSize sz(m_sketchPanel->GetSize() - wxSize(2 * BorderWidth - 2, 2 * BorderWidth - 2));

    double x = double(pt.m_x - o.x) / sz.GetWidth();
    double y = double(pt.m_y - o.y) / sz.GetHeight();

    return wxPoint2DDouble(x, 1. - y);
}

wxPoint2DDouble SketchPathDialog::NormalizedToUI(const wxPoint2DDouble& pt) const
{
    wxPoint o(BorderWidth + 1, BorderWidth + 1);
    wxSize sz(m_sketchPanel->GetSize() - wxSize(2 * BorderWidth - 2, 2 * BorderWidth - 2));

    double x = pt.m_x * sz.GetWidth();
    double y = pt.m_y * sz.GetHeight();
    return wxPoint2DDouble(o.x + x, o.y + (sz.GetHeight() - y - 1));
}

void SketchPathDialog::UpdateHandlesForPath(long pathIndex)
{
    m_handles.clear();
    m_grabbedHandleIndex = -1;
    m_pathState = Undefined;
    m_pathClosed = false;

    if (pathIndex < 0 || pathIndex >= m_sketch.paths().size())
        return;

    auto iter = m_sketch.paths().cbegin();
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

    m_sketchPanel->Refresh();
}

void SketchPathDialog::UpdatePathForHandles(long handleIndex)
{
    auto paths(m_sketch.paths());
    auto pathIndex = m_pathsListView->GetFirstSelected();
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
}

void SketchPathDialog::UpdatePathState(PathState state)
{
    m_pathState = state;

    switch ( m_pathState )
    {
    case Undefined:
        m_startPathBtn->Enable();
        m_endPathBtn->Disable();
        m_closePathBtn->Disable();
        break;
    case DefineStartPoint:
        m_startPathBtn->Disable();
        m_endPathBtn->Disable();
        m_closePathBtn->Disable();
        break;
    case LineToNewPoint:
    case QuadraticCurveToNewPoint:
    case CubicCurveToNewPoint:
        m_startPathBtn->Disable();
        m_endPathBtn->Enable();
        m_closePathBtn->Enable();
        break;
    }

    m_sketchPanel->Refresh();

    // If we're Undefined, have some handles, and no path
    // selected, I think we've added a new one!!
    if (m_pathState == Undefined && !m_handles.empty() && m_pathsListView->GetFirstSelected() < 0) {
        auto path = CreatePathFromHandles();
        if (path != nullptr) {
            m_sketch.appendPath(path);
            int n = static_cast<int>(m_sketch.pathCount());
            wxString str;
            str.sprintf("Path %d", n);
            m_pathsListView->InsertItem(n, str);
            m_pathsListView->Select(n - 1);
        }
    }
}

bool SketchPathDialog::isControlPoint(const HandlePoint& handlePt)
{
    auto hpt = handlePt.handlePointType;
    return hpt == QuadraticControlPt || hpt == CubicControlPt1 || hpt == CubicControlPt2;
}

std::shared_ptr<SketchEffectPath> SketchPathDialog::CreatePathFromHandles() const
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
        }

        if (segment != nullptr)
            path->appendSegment(segment);
    }
    if (m_pathClosed)
        path->closePath();

    return path;
}
