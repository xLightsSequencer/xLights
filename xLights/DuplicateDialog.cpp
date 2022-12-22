/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "DuplicateDialog.h"

//(*InternalHeaders(DuplicateDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/config.h>


//(*IdInit(DuplicateDialog)
const long DuplicateDialog::ID_SPINCTRL_COUNT = wxNewId();
const long DuplicateDialog::ID_STATICTEXT2 = wxNewId();
const long DuplicateDialog::ID_SPINCTRL_GAP = wxNewId();
const long DuplicateDialog::ID_BUTTON_OK = wxNewId();
const long DuplicateDialog::ID_BUTTON_CLOSE = wxNewId();
//*)

BEGIN_EVENT_TABLE(DuplicateDialog,wxDialog)
	//(*EventTable(DuplicateDialog)
	//*)
END_EVENT_TABLE()

DuplicateDialog::DuplicateDialog(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size)
{
    //(*Initialize(DuplicateDialog)
    wxBoxSizer* BoxSizer1;
    wxBoxSizer* BoxSizer2;
    wxFlexGridSizer* FlexGridSizer2;

    Create(parent, wxID_ANY, _("Duplicate Effect"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
    BoxSizer1 = new wxBoxSizer(wxVERTICAL);
    FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    StaticText1 = new wxStaticText(this, wxID_ANY, _("Count:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
    FlexGridSizer2->Add(StaticText1, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    SpinCtrl_Count = new wxSpinCtrl(this, ID_SPINCTRL_COUNT, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 100, 1, _T("ID_SPINCTRL_COUNT"));
    SpinCtrl_Count->SetValue(_T("1"));
    FlexGridSizer2->Add(SpinCtrl_Count, 1, wxALL | wxEXPAND, 5);
    StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Gap:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer2->Add(StaticText2, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    SpinCtrl_Gap = new wxSpinCtrl(this, ID_SPINCTRL_GAP, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 0, 100, 0, _T("ID_SPINCTRL_GAP"));
    SpinCtrl_Gap->SetValue(_T("0"));
    FlexGridSizer2->Add(SpinCtrl_Gap, 1, wxALL | wxEXPAND, 5);
    BoxSizer1->Add(FlexGridSizer2, 1, wxALL | wxEXPAND, 5);
    BoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    Button_Ok = new wxButton(this, ID_BUTTON_OK, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_OK"));
    BoxSizer2->Add(Button_Ok, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    Button_Close = new wxButton(this, ID_BUTTON_CLOSE, _("Close"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CLOSE"));
    BoxSizer2->Add(Button_Close, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    BoxSizer1->Add(BoxSizer2, 0, wxALL | wxEXPAND, 5);
    SetSizer(BoxSizer1);
    BoxSizer1->Fit(this);
    BoxSizer1->SetSizeHints(this);

    Connect(ID_BUTTON_OK, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&DuplicateDialog::OnButton_OkClick);
    Connect(ID_BUTTON_CLOSE, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&DuplicateDialog::OnButton_CloseClick);
    //*)
    wxConfigBase* config = wxConfigBase::Get();
    if (config != nullptr) {
        int count { 1 };
        int gap { 0 };
        config->Read("DuplicateDialogCount", &count);
        config->Read("DuplicateDialogGap", &gap);
        SpinCtrl_Count->SetValue(count);
        SpinCtrl_Gap->SetValue(gap);
    }
}

DuplicateDialog::~DuplicateDialog()
{
	//(*Destroy(DuplicateDialog)
	//*)
}

void DuplicateDialog::OnButton_CloseClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void DuplicateDialog::OnButton_OkClick(wxCommandEvent& event)
{
    wxConfigBase* config = wxConfigBase::Get();
    if (config != nullptr) {
        config->Write("DuplicateDialogCount", SpinCtrl_Count->GetValue());
        config->Write("DuplicateDialogGap", SpinCtrl_Gap->GetValue());
        config->Flush();
    }
    EndDialog(wxID_OK);
}
