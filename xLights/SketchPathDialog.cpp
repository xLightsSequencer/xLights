#include "SketchPathDialog.h"

#include <optional>
#include <utility>
#include <xutility>

#include <wx/brush.h>
#include <wx/button.h>
#include <wx/dcbuffer.h>
#include <wx/graphics.h>
#include <wx/statbox.h>
#include <wx/panel.h>
#include <wx/pen.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

BEGIN_EVENT_TABLE(SketchPathDialog, wxDialog)
END_EVENT_TABLE()

namespace
{
    const int BorderWidth = 5;
}

SketchPathDialog::SketchPathDialog(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size)
{
    Create(parent, wxID_ANY, "Define Sketch", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxCLOSE_BOX, _T("wxID_ANY"));

    auto mainSizer = new wxFlexGridSizer(2, 1, 0, 0);
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

    Connect(okButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SketchPathDialog::OnButton_Ok);
    Connect(cancelButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SketchPathDialog::OnButton_Cancel);

    UpdatePathState(Undefined);
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

        gc->SetPen(*wxBLACK_PEN);

        std::vector<wxPoint>::size_type n = m_handles.size();

        // First, draw the path
        wxGraphicsPath path = gc->CreatePath();
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
            gc->SetPen(pen);
            auto ptFrom = NormalizedToUI(m_handles.back().pt);
            auto ptTo(m_mousePos);
            gc->StrokeLine(ptFrom.m_x, ptFrom.m_y, ptTo.m_x, ptTo.m_y);

            gc->SetPen(*wxBLACK_PEN);
        }

        // Second, draw the handles
        for (std::vector<wxPoint>::size_type i = 0; i < n; ++i)
        {
            wxPoint2DDouble pt(NormalizedToUI(m_handles[i].pt));
            if (i == 0 && !m_handles[i].state )
                gc->SetBrush(*wxGREEN_BRUSH);
            else if (i == n - 1 && !m_handles[i].state )
                gc->SetBrush(*wxRED_BRUSH);
            else
                gc->SetBrush((m_handles[i].state) ? (*wxYELLOW_BRUSH) : (*wxLIGHT_GREY_BRUSH));

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
        return;
    } else if ( m_pathState == QuadraticCurveToNewPoint )
    {
        wxPoint2DDouble startPt = m_handles.back().pt;
        wxPoint2DDouble endPt = UItoNormalized(event.GetPosition());
        wxPoint2DDouble cp = 0.5 * startPt + 0.5 * endPt;

        m_handles.push_back(HandlePoint(cp, QuadraticControlPt));
        m_handles.push_back(HandlePoint(endPt, QuadraticCurveEnd));
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

void SketchPathDialog::OnSketchLeftUp(wxMouseEvent& event)
{
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

void SketchPathDialog::OnButton_StartPath(wxCommandEvent& event)
{
    m_handles.clear(); // only one path allowed currently
    m_pathClosed = false;
    UpdatePathState(DefineStartPoint);
}

void SketchPathDialog::OnButton_EndPath(wxCommandEvent& event)
{
    UpdatePathState(Undefined);
}

void SketchPathDialog::OnButton_ClosePath(wxCommandEvent& event)
{
    m_pathClosed = true;
    UpdatePathState(Undefined);
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
}
