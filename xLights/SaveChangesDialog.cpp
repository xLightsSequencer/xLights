/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "SaveChangesDialog.h"

//(*InternalHeaders(SaveChangesDialog)
#include <wx/font.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(SaveChangesDialog)
const long SaveChangesDialog::ID_STATICTEXT2 = wxNewId();
const long SaveChangesDialog::ID_STATICTEXT1 = wxNewId();
const long SaveChangesDialog::ID_BUTTON_SaveChanges = wxNewId();
const long SaveChangesDialog::ID_BUTTON_DiscardChanges = wxNewId();
const long SaveChangesDialog::ID_BUTTON_CANCEL = wxNewId();
//*)

BEGIN_EVENT_TABLE(SaveChangesDialog,wxDialog)
	//(*EventTable(SaveChangesDialog)
	//*)
END_EVENT_TABLE()

SaveChangesDialog::SaveChangesDialog(wxWindow* parent)
: mSaveChanges(false)
{
	//(*Initialize(SaveChangesDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;

	Create(parent, wxID_ANY, _("Save Sequence Changes\?"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("*** The sequence you are closing has unsaved changes. ***"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	StaticText2->SetForegroundColour(wxColour(255,0,0));
	wxFont StaticText2Font(12,wxFONTFAMILY_SWISS,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL,false,_T("Arial"),wxFONTENCODING_DEFAULT);
	StaticText2->SetFont(StaticText2Font);
	FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Please choose whether you would like to Save or Discard the changes \nor hit Cancel to return to the sequence."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	wxFont StaticText1Font(12,wxFONTFAMILY_SWISS,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
	StaticText1->SetFont(StaticText1Font);
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_SaveChanges = new wxButton(this, ID_BUTTON_SaveChanges, _("&Save Changes"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SaveChanges"));
	FlexGridSizer3->Add(Button_SaveChanges, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_DiscardChanges = new wxButton(this, ID_BUTTON_DiscardChanges, _("&Discard Changes"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_DiscardChanges"));
	FlexGridSizer3->Add(Button_DiscardChanges, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CANCEL"));
	FlexGridSizer3->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	Center();

	Connect(ID_BUTTON_SaveChanges,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SaveChangesDialog::OnButton_SaveChangesClick);
	Connect(ID_BUTTON_DiscardChanges,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SaveChangesDialog::OnButton_DiscardChangesClick);
	Connect(ID_BUTTON_CANCEL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SaveChangesDialog::OnButton_CancelClick);
	//*)

    SetEscapeId(Button_Cancel->GetId());
    Button_Cancel->SetFocus();
}

SaveChangesDialog::~SaveChangesDialog()
{
	//(*Destroy(SaveChangesDialog)
	//*)
}


void SaveChangesDialog::OnButton_SaveChangesClick(wxCommandEvent& event)
{
    mSaveChanges = true;
    EndModal(wxID_OK);
}

void SaveChangesDialog::OnButton_DiscardChangesClick(wxCommandEvent& event)
{
    mSaveChanges = false;
    EndModal(wxID_OK);
}

void SaveChangesDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndModal(wxID_CANCEL);
}
