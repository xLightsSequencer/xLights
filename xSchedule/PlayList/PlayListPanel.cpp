#include "PlayListPanel.h"

#include "PlayList.h"
#include "PlayListDialog.h"

//(*InternalHeaders(PlayListPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(PlayListPanel)
const long PlayListPanel::ID_STATICTEXT1 = wxNewId();
const long PlayListPanel::ID_TEXTCTRL1 = wxNewId();
const long PlayListPanel::ID_CHECKBOX1 = wxNewId();
const long PlayListPanel::ID_CHECKBOX2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PlayListPanel,wxPanel)
	//(*EventTable(PlayListPanel)
	//*)
END_EVENT_TABLE()

PlayListPanel::PlayListPanel(wxWindow* parent, PlayList* playlist, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _playlist = playlist;

	//(*Initialize(PlayListPanel)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT, 5);
	TextCtrl_PlayListName = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(TextCtrl_PlayListName, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_FirstOnce = new wxCheckBox(this, ID_CHECKBOX1, _("First step once only"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_FirstOnce->SetValue(false);
	FlexGridSizer1->Add(CheckBox_FirstOnce, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_LastStepOnce = new wxCheckBox(this, ID_CHECKBOX2, _("Last step once only"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
	CheckBox_LastStepOnce->SetValue(false);
	FlexGridSizer1->Add(CheckBox_LastStepOnce, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListPanel::OnTextCtrl_PlayListNameText);
	//*)

    wxTextValidator tv(wxFILTER_EXCLUDE_CHAR_LIST);
    tv.SetCharExcludes("\"'`");
    TextCtrl_PlayListName->SetValidator(tv);

    TextCtrl_PlayListName->SetValue(playlist->GetNameNoTime());
    CheckBox_FirstOnce->SetValue(playlist->GetFirstOnce());
    CheckBox_LastStepOnce->SetValue(playlist->GetLastOnce());
}

PlayListPanel::~PlayListPanel()
{
	//(*Destroy(PlayListPanel)
	//*)
    _playlist->SetName(TextCtrl_PlayListName->GetValue().ToStdString());
    _playlist->SetFirstOnce(CheckBox_FirstOnce->GetValue());
    _playlist->SetLastOnce(CheckBox_LastStepOnce->GetValue());
}

void PlayListPanel::OnTextCtrl_PlayListNameText(wxCommandEvent& event)
{
    _playlist->SetName(TextCtrl_PlayListName->GetValue().ToStdString());
    ((PlayListDialog*)GetParent()->GetParent()->GetParent()->GetParent())->UpdateTree();
}
