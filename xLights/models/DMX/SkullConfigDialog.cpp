/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "SkullConfigDialog.h"

//(*InternalHeaders(SkullConfigDialog)
#include <wx/button.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(SkullConfigDialog)
const long SkullConfigDialog::ID_CHECKBOX_Jaw = wxNewId();
const long SkullConfigDialog::ID_CHECKBOX_Pan = wxNewId();
const long SkullConfigDialog::ID_CHECKBOX_Tilt = wxNewId();
const long SkullConfigDialog::ID_CHECKBOX_Nod = wxNewId();
const long SkullConfigDialog::ID_CHECKBOX_EyeUD = wxNewId();
const long SkullConfigDialog::ID_CHECKBOX_EyeLR = wxNewId();
const long SkullConfigDialog::ID_CHECKBOX_Color = wxNewId();
const long SkullConfigDialog::ID_CHECKBOX_16bits = wxNewId();
const long SkullConfigDialog::ID_CHECKBOX_Skulltronix = wxNewId();
//*)

BEGIN_EVENT_TABLE(SkullConfigDialog,wxDialog)
	//(*EventTable(SkullConfigDialog)
	//*)
END_EVENT_TABLE()

SkullConfigDialog::SkullConfigDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(SkullConfigDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	CheckBox_Jaw = new wxCheckBox(this, ID_CHECKBOX_Jaw, _("Jaw Servo"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Jaw"));
	CheckBox_Jaw->SetValue(true);
	FlexGridSizer2->Add(CheckBox_Jaw, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Pan = new wxCheckBox(this, ID_CHECKBOX_Pan, _("Pan Servo"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Pan"));
	CheckBox_Pan->SetValue(true);
	FlexGridSizer2->Add(CheckBox_Pan, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Tilt = new wxCheckBox(this, ID_CHECKBOX_Tilt, _("Tilt Servo"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Tilt"));
	CheckBox_Tilt->SetValue(true);
	FlexGridSizer2->Add(CheckBox_Tilt, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Nod = new wxCheckBox(this, ID_CHECKBOX_Nod, _("Nod Servo"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Nod"));
	CheckBox_Nod->SetValue(true);
	FlexGridSizer2->Add(CheckBox_Nod, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_EyeUD = new wxCheckBox(this, ID_CHECKBOX_EyeUD, _("Eye Up/Down Servo"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_EyeUD"));
	CheckBox_EyeUD->SetValue(true);
	FlexGridSizer2->Add(CheckBox_EyeUD, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_EyeLR = new wxCheckBox(this, ID_CHECKBOX_EyeLR, _("Eye Left/Right Servo"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_EyeLR"));
	CheckBox_EyeLR->SetValue(true);
	FlexGridSizer2->Add(CheckBox_EyeLR, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Color = new wxCheckBox(this, ID_CHECKBOX_Color, _("Color Channels"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Color"));
	CheckBox_Color->SetValue(true);
	FlexGridSizer2->Add(CheckBox_Color, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_16bits = new wxCheckBox(this, ID_CHECKBOX_16bits, _("16 Bits"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_16bits"));
	CheckBox_16bits->SetValue(true);
	FlexGridSizer2->Add(CheckBox_16bits, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Skulltronix = new wxCheckBox(this, ID_CHECKBOX_Skulltronix, _("Skulltronix"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Skulltronix"));
	CheckBox_Skulltronix->SetValue(false);
	FlexGridSizer2->Add(CheckBox_Skulltronix, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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
}

SkullConfigDialog::~SkullConfigDialog()
{
	//(*Destroy(SkullConfigDialog)
	//*)
}

