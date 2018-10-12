#include "RenameTextDialog.h"

//(*InternalHeaders(RenameTextDialog)
#include <wx/button.h>
#include <wx/string.h>
#include <wx/intl.h>
#include <wx/font.h>
//*)

//(*IdInit(RenameTextDialog)
const long RenameTextDialog::ID_STATICTEXT_Rename_Label = wxNewId();
const long RenameTextDialog::ID_TEXTCTRL_Rename_Text = wxNewId();
//*)

BEGIN_EVENT_TABLE(RenameTextDialog,wxDialog)
	//(*EventTable(RenameTextDialog)
	//*)
END_EVENT_TABLE()

RenameTextDialog::RenameTextDialog(wxWindow* parent,wxWindowID id)
{
	//(*Initialize(RenameTextDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	StaticText_Rename_Label = new wxStaticText(this, ID_STATICTEXT_Rename_Label, _("Modify Text then hit OK"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Rename_Label"));
	wxFont StaticText_Rename_LabelFont(12,wxFONTFAMILY_SWISS,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL,false,_T("Arial"),wxFONTENCODING_DEFAULT);
	StaticText_Rename_Label->SetFont(StaticText_Rename_LabelFont);
	FlexGridSizer1->Add(StaticText_Rename_Label, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Rename_Text = new wxTextCtrl(this, ID_TEXTCTRL_Rename_Text, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_Rename_Text"));
	FlexGridSizer1->Add(TextCtrl_Rename_Text, 1, wxALL|wxEXPAND, 15);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL_Rename_Text,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&RenameTextDialog::OnTextCtrl_Rename_TextText);
	//*)
    SetEscapeId(wxID_CANCEL);
}

RenameTextDialog::~RenameTextDialog()
{
	//(*Destroy(RenameTextDialog)
	//*)
}


void RenameTextDialog::OnTextCtrl_Rename_TextText(wxCommandEvent& event)
{
    rename_text = TextCtrl_Rename_Text->GetValue();
}
