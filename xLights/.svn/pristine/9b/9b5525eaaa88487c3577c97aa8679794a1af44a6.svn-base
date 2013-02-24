#include "PlaybackOptionsDialog.h"

//(*InternalHeaders(PlaybackOptionsDialog)
#include <wx/intl.h>
#include <wx/button.h>
#include <wx/string.h>
//*)

//(*IdInit(PlaybackOptionsDialog)
const long PlaybackOptionsDialog::ID_CHECKBOX_REPEAT = wxNewId();
const long PlaybackOptionsDialog::ID_CHECKBOX_FIRSTITEM = wxNewId();
const long PlaybackOptionsDialog::ID_CHECKBOX_LASTITEM = wxNewId();
const long PlaybackOptionsDialog::ID_CHECKBOX_RANDOM = wxNewId();
//*)

BEGIN_EVENT_TABLE(PlaybackOptionsDialog,wxDialog)
	//(*EventTable(PlaybackOptionsDialog)
	//*)
END_EVENT_TABLE()

PlaybackOptionsDialog::PlaybackOptionsDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(PlaybackOptionsDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, id, _("Playback Options"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	CheckBoxRepeat = new wxCheckBox(this, ID_CHECKBOX_REPEAT, _("Repeat"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_REPEAT"));
	CheckBoxRepeat->SetValue(false);
	FlexGridSizer1->Add(CheckBoxRepeat, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	CheckBoxFirstItem = new wxCheckBox(this, ID_CHECKBOX_FIRSTITEM, _("Play first item once at beginning (if repeating)"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_FIRSTITEM"));
	CheckBoxFirstItem->SetValue(false);
	FlexGridSizer1->Add(CheckBoxFirstItem, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	CheckBoxLastItem = new wxCheckBox(this, ID_CHECKBOX_LASTITEM, _("Play last item once at end (if repeating)"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_LASTITEM"));
	CheckBoxLastItem->SetValue(false);
	FlexGridSizer1->Add(CheckBoxLastItem, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	CheckBoxRandom = new wxCheckBox(this, ID_CHECKBOX_RANDOM, _("Random Order"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_RANDOM"));
	CheckBoxRandom->SetValue(false);
	FlexGridSizer1->Add(CheckBoxRandom, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
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

PlaybackOptionsDialog::~PlaybackOptionsDialog()
{
	//(*Destroy(PlaybackOptionsDialog)
	//*)
}

