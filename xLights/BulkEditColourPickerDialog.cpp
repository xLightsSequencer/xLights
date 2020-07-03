/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "BulkEditColourPickerDialog.h"

//(*InternalHeaders(BulkEditColourPickerDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(BulkEditColourPickerDialog)
const long BulkEditColourPickerDialog::ID_STATICTEXT1 = wxNewId();
const long BulkEditColourPickerDialog::ID_COLOURPICKERCTRL1 = wxNewId();
const long BulkEditColourPickerDialog::ID_BUTTON1 = wxNewId();
const long BulkEditColourPickerDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(BulkEditColourPickerDialog,wxDialog)
	//(*EventTable(BulkEditColourPickerDialog)
	//*)
END_EVENT_TABLE()

BulkEditColourPickerDialog::BulkEditColourPickerDialog(wxWindow* parent, const std::string& label, wxColour value, wxWindowID id)
{
	//(*Initialize(BulkEditColourPickerDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;

	Create(parent, id, _("Bulk Edit Colour"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Color:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ColourPickerCtrl1 = new wxColourPickerCtrl(this, ID_COLOURPICKERCTRL1, wxColour(0,0,0), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_COLOURPICKERCTRL1"));
	FlexGridSizer1->Add(ColourPickerCtrl1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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

	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BulkEditColourPickerDialog::OnButton_OkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BulkEditColourPickerDialog::OnButton_CancelClick);
	//*)

	SetEscapeId(Button_Cancel->GetId());
	Button_Ok->SetFocus();
	Button_Ok->SetDefault();

	StaticText1->SetLabel(label);

	ColourPickerCtrl1->SetColour(value);
}

BulkEditColourPickerDialog::~BulkEditColourPickerDialog()
{
	//(*Destroy(BulkEditColourPickerDialog)
	//*)
}

wxColour BulkEditColourPickerDialog::GetValue() const
{
	return ColourPickerCtrl1->GetColour();
}

void BulkEditColourPickerDialog::OnButton_OkClick(wxCommandEvent& event)
{
	EndDialog(wxID_OK);
}

void BulkEditColourPickerDialog::OnButton_CancelClick(wxCommandEvent& event)
{
	EndDialog(wxID_CANCEL);
}
