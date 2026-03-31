#include "FPPUploadProgressDialog.h"

#include <wx/gauge.h>

//(*InternalHeaders(FPPUploadProgressDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(FPPUploadProgressDialog)
const long FPPUploadProgressDialog::ID_STATICTEXT1 = wxNewId();
const long FPPUploadProgressDialog::ID_SCROLLEDWINDOW1 = wxNewId();
const long FPPUploadProgressDialog::ID_BUTTON1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(FPPUploadProgressDialog,wxDialog)
    //(*EventTable(FPPUploadProgressDialog)
    //*)
END_EVENT_TABLE()

FPPUploadProgressDialog::FPPUploadProgressDialog(wxWindow* parent,wxWindowID id)
{
    //(*Initialize(FPPUploadProgressDialog)
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer2;

    Create(parent, id, _("Upload Progress"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER, _T("id"));
    SetMinSize(wxSize(400,-1));
    FlexGridSizer1 = new wxFlexGridSizer(3, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer1->AddGrowableRow(1);
    ActionLabel = new wxStaticText(this, ID_STATICTEXT1, _("Action"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE, _T("ID_STATICTEXT1"));
    FlexGridSizer1->Add(ActionLabel, 1, wxALL|wxEXPAND, 5);
    scrolledWindow = new wxScrolledWindow(this, ID_SCROLLEDWINDOW1, wxDefaultPosition, wxDefaultSize, wxVSCROLL, _T("ID_SCROLLEDWINDOW1"));
    scrolledWindow->SetMinSize(wxDLG_UNIT(this,wxSize(400,0)));
    scrolledWindowSizer = new wxFlexGridSizer(0, 2, 0, 0);
    scrolledWindowSizer->AddGrowableCol(1);
    scrolledWindow->SetSizer(scrolledWindowSizer);
    FlexGridSizer1->Add(scrolledWindow, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
    CancelButton = new wxButton(this, ID_BUTTON1, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer2->Add(CancelButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
    SetSizer(FlexGridSizer1);
    Fit();

    Connect(ID_BUTTON1, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FPPUploadProgressDialog::OnCancelButtonClick);
    //*)
    SetEscapeId(ID_BUTTON1);
    
    FlexGridSizer1->SetSizeHints(this);
}

FPPUploadProgressDialog::~FPPUploadProgressDialog()
{
    //(*Destroy(FPPUploadProgressDialog)
    //*)
}


void FPPUploadProgressDialog::OnCancelButtonClick(wxCommandEvent& event)
{
    CancelButton->SetLabelText("Canceling...");
    CancelButton->Enable(false);
    cancelled = true;
}

void FPPUploadProgressDialog::setActionLabel(const std::string &action) {
    ActionLabel->SetLabel(action);
    ActionLabel->Update();
}

wxGauge *FPPUploadProgressDialog::addGauge(const std::string &name) {
    wxStaticText *label = new wxStaticText(scrolledWindow, wxID_ANY, name);
    scrolledWindowSizer->Add(label,1, wxALL |wxEXPAND,3);
    wxGauge *g = new wxGauge(scrolledWindow, wxID_ANY, 100);
    g->SetValue(0);
    g->SetMinSize(wxSize(200, -1));
    scrolledWindowSizer->Add(g, 1, wxALL |wxEXPAND,3);
    g->SetRange(1000);
    return g;
}
