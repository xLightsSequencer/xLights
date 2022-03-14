#include "SketchPathDialog.h"

#include <memory>

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
    auto startPathBtn = new wxButton(this, wxID_ANY, "Start Path");
    pathCtrlsSizer->Add(startPathBtn, 1, wxALL, 2);
    auto endPathBtn = new wxButton(this, wxID_ANY, "End Path");
    endPathBtn->Disable();
    pathCtrlsSizer->Add(endPathBtn, 1, wxALL, 2);
    auto closePathBtn = new wxButton(this, wxID_ANY, "Close Path");
    closePathBtn->Disable();
    pathCtrlsSizer->Add(closePathBtn, 1, wxALL, 2);

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

    Connect(startPathBtn->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SketchPathDialog::OnButton_StartPath);

    Connect(okButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SketchPathDialog::OnButton_Ok);
    Connect(cancelButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SketchPathDialog::OnButton_Cancel);

    m_handles.push_back(wxPoint(100, 100));
    m_handles.push_back(wxPoint(150, 150));
    m_handles.push_back(wxPoint(200, 200));
}

void SketchPathDialog::OnSketchPaint(wxPaintEvent& event)
{
    wxAutoBufferedPaintDC pdc(m_sketchPanel);
    wxSize sz(m_sketchPanel->GetSize());

    pdc.SetPen(*wxWHITE_PEN);
    pdc.SetBrush(*wxWHITE_BRUSH);
    pdc.DrawRectangle(wxPoint(0, 0), sz);

    pdc.SetPen(*wxLIGHT_GREY_PEN);
    pdc.DrawRectangle(BorderWidth, BorderWidth, sz.GetWidth() - 2 * BorderWidth, sz.GetHeight() - 2 * BorderWidth);

    //pdc.SetPen(*wxBLUE_PEN);
    //pdc.DrawLine(BorderWidth + 1, BorderWidth + 1, sz.GetWidth() - BorderWidth - 1, BorderWidth + 1);

    {
        std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(pdc));

        gc->SetPen(*wxBLACK_PEN);

        std::vector<wxPoint>::size_type n = m_handles.size();

        // First, draw the path
        wxGraphicsPath path = gc->CreatePath();
        for (std::vector<wxPoint>::size_type i = 0; i < n; ++i)
        {
            if (i == 0)
                path.MoveToPoint(m_handles[i].pt);
            else
                path.AddLineToPoint(m_handles[i].pt);
        }
        gc->DrawPath(path);

        // Second, draw the handles
        for (std::vector<wxPoint>::size_type i = 0; i < n; ++i)
        {
            if (i == 0 && !m_handles[i].state )
                gc->SetBrush(*wxGREEN_BRUSH);
            else if (i == n - 1 && !m_handles[i].state )
                gc->SetBrush(*wxRED_BRUSH);
            else
                gc->SetBrush((m_handles[i].state) ? (*wxYELLOW_BRUSH) : (*wxLIGHT_GREY_BRUSH));

             gc->DrawEllipse(m_handles[i].pt.x - 4, m_handles[i].pt.y - 4, 8, 8);
        }
    }
}

void SketchPathDialog::OnSketchKeyDown(wxKeyEvent& event)
{
}

void SketchPathDialog::OnSketchLeftDown(wxMouseEvent& event)
{
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
    wxPoint2DDouble pos(event.GetPosition());

    if ( m_grabbedHandleIndex != -1 )
    {
        m_handles[m_grabbedHandleIndex].pt = event.GetPosition();
        m_sketchPanel->Refresh();
    }
    else
    {
        bool somethingChanged = false;
        for (auto& handle : m_handles)
        {
            wxPoint2DDouble handlePos(handle.pt);
            bool state = pos.GetDistanceSquare(handlePos) <= 16.;
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

}

void SketchPathDialog::OnButton_Ok(wxCommandEvent& /*event*/)
{
    EndDialog(wxID_OK);
}

void SketchPathDialog::OnButton_Cancel(wxCommandEvent& /*event*/)
{
    EndDialog(wxID_CANCEL);
}
