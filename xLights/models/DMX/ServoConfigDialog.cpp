/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "ServoConfigDialog.h"

//(*InternalHeaders(ServoConfigDialog)
#include <wx/button.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(ServoConfigDialog)
const long ServoConfigDialog::ID_STATICTEXT1 = wxNewId();
const long ServoConfigDialog::ID_SPINCTRL_NumServos = wxNewId();
const long ServoConfigDialog::ID_STATICTEXT2 = wxNewId();
const long ServoConfigDialog::ID_SPINCTRL_NumStatic = wxNewId();
const long ServoConfigDialog::ID_STATICTEXT3 = wxNewId();
const long ServoConfigDialog::ID_SPINCTRL_NumMotion = wxNewId();
const long ServoConfigDialog::ID_CHECKBOX_16bits = wxNewId();
//*)

BEGIN_EVENT_TABLE(ServoConfigDialog,wxDialog)
	//(*EventTable(ServoConfigDialog)
	//*)
END_EVENT_TABLE()

ServoConfigDialog::ServoConfigDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(ServoConfigDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Number of Servos"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_NumServos = new wxSpinCtrl(this, ID_SPINCTRL_NumServos, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 25, 1, _T("ID_SPINCTRL_NumServos"));
	SpinCtrl_NumServos->SetValue(_T("1"));
	FlexGridSizer2->Add(SpinCtrl_NumServos, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Number of Static Meshs"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_NumStatic = new wxSpinCtrl(this, ID_SPINCTRL_NumStatic, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 24, 1, _T("ID_SPINCTRL_NumStatic"));
	SpinCtrl_NumStatic->SetValue(_T("1"));
	FlexGridSizer2->Add(SpinCtrl_NumStatic, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Number of Motion Meshs"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer2->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_NumMotion = new wxSpinCtrl(this, ID_SPINCTRL_NumMotion, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 24, 1, _T("ID_SPINCTRL_NumMotion"));
	SpinCtrl_NumMotion->SetValue(_T("1"));
	FlexGridSizer2->Add(SpinCtrl_NumMotion, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_16bits = new wxCheckBox(this, ID_CHECKBOX_16bits, _("16 Bits"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_16bits"));
	CheckBox_16bits->SetValue(true);
	FlexGridSizer2->Add(CheckBox_16bits, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	//*)
	Connect(ID_SPINCTRL_NumServos, wxEVT_COMMAND_SPINCTRL_UPDATED, (wxObjectEventFunction)&ServoConfigDialog::OnSpinCtrl_NumServosChange);
}

ServoConfigDialog::~ServoConfigDialog()
{
	//(*Destroy(ServoConfigDialog)
	//*)
}


void ServoConfigDialog::OnSpinCtrl_NumServosChange(wxSpinEvent& event)
{
	SpinCtrl_NumMotion->SetRange(1, SpinCtrl_NumServos->GetValue());
}
