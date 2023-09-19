#include "AutoLabelDialog.h"

//(*InternalHeaders(AutoLabelDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(AutoLabelDialog)
const long AutoLabelDialog::ID_STATICTEXT1 = wxNewId();
const long AutoLabelDialog::ID_SPINCTRL1 = wxNewId();
const long AutoLabelDialog::ID_STATICTEXT2 = wxNewId();
const long AutoLabelDialog::ID_SPINCTRL2 = wxNewId();
const long AutoLabelDialog::ID_CHECKBOX1 = wxNewId();
const long AutoLabelDialog::ID_BUTTON1 = wxNewId();
const long AutoLabelDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(AutoLabelDialog,wxDialog)
	//(*EventTable(AutoLabelDialog)
	//*)
END_EVENT_TABLE()

AutoLabelDialog::AutoLabelDialog(wxWindow* parent,wxWindowID id)
{
	//(*Initialize(AutoLabelDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;

	Create(parent, id, _("Auto Label Timing Track"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxBORDER_SIMPLE, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Start Label:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Start = new wxSpinCtrl(this, ID_SPINCTRL1, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 0, 1000, 1, _T("ID_SPINCTRL1"));
	SpinCtrl_Start->SetValue(_T("1"));
	FlexGridSizer1->Add(SpinCtrl_Start, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("End Label:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_end = new wxSpinCtrl(this, ID_SPINCTRL2, _T("4"), wxDefaultPosition, wxDefaultSize, 0, 0, 1000, 4, _T("ID_SPINCTRL2"));
	SpinCtrl_end->SetValue(_T("4"));
	FlexGridSizer1->Add(SpinCtrl_end, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(0,0,1, wxALL|wxEXPAND, 5);
	CheckBox_Overwrite = new wxCheckBox(this, ID_CHECKBOX1, _("Overwrite Existing Labels"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_Overwrite->SetValue(true);
	FlexGridSizer1->Add(CheckBox_Overwrite, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(0,0,1, wxALL|wxEXPAND, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	Button_Ok->SetDefault();
	FlexGridSizer2->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer2->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&AutoLabelDialog::OnButton_OkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&AutoLabelDialog::OnButton_CancelClick);
	//*)

    SetEscapeId(Button_Cancel->GetId());
}

AutoLabelDialog::~AutoLabelDialog()
{
	//(*Destroy(AutoLabelDialog)
	//*)
}


void AutoLabelDialog::OnButton_OkClick(wxCommandEvent& event)
{
    EndDialog(wxOK);
}

void AutoLabelDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxCANCEL);
}

uint32_t AutoLabelDialog::GetStart() const
{
    return SpinCtrl_Start->GetValue();
}

uint32_t AutoLabelDialog::GetEnd() const
{
    return SpinCtrl_end->GetValue();
}

bool AutoLabelDialog::IsOverwrite() const
{
    return CheckBox_Overwrite->IsChecked();
}
