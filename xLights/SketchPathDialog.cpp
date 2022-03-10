#include "SketchPathDialog.h"

#include <wx/button.h>
#include <wx/statbox.h>
#include <wx/sizer.h>

BEGIN_EVENT_TABLE(SketchPathDialog, wxDialog)
END_EVENT_TABLE()

SketchPathDialog::SketchPathDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    Create(parent, wxID_ANY, "Define Sketch", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxCLOSE_BOX, _T("wxID_ANY"));

	auto mainSizer = new wxFlexGridSizer(2, 1, 0, 0);
    mainSizer->AddGrowableCol(0);
    mainSizer->AddGrowableRow(0);

	auto label = new wxStaticBox(this, wxID_ANY, "Current Point", wxDefaultPosition, wxDefaultSize, wxBORDER_STATIC);
    mainSizer->Add(label, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);

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

	Connect(okButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SketchPathDialog::OnButton_Ok);
    Connect(cancelButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SketchPathDialog::OnButton_Cancel);
}

void SketchPathDialog::OnButton_Ok(wxCommandEvent& /*event*/)
{
    EndDialog(wxID_OK);
}

void SketchPathDialog::OnButton_Cancel(wxCommandEvent& /*event*/)
{
    EndDialog(wxID_CANCEL);
}
