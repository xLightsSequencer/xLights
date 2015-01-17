#include "RenameTextDialog.h"

//(*InternalHeaders(RenameTextDialog)
#include <wx/font.h>
#include <wx/intl.h>
#include <wx/button.h>
#include <wx/string.h>
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
	wxFont StaticText_Rename_LabelFont(12,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,_T("Arial"),wxFONTENCODING_DEFAULT);
	StaticText_Rename_Label->SetFont(StaticText_Rename_LabelFont);
	FlexGridSizer1->Add(StaticText_Rename_Label, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Rename_Text = new wxTextCtrl(this, ID_TEXTCTRL_Rename_Text, wxEmptyString, wxDefaultPosition, wxSize(254,21), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Rename_Text"));
	FlexGridSizer1->Add(TextCtrl_Rename_Text, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 15);
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
