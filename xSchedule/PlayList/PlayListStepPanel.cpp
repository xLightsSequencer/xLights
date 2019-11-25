#include "PlayListStepPanel.h"

#include "PlayListStep.h"
#include "PlayListDialog.h"

//(*InternalHeaders(PlayListStepPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(PlayListStepPanel)
const long PlayListStepPanel::ID_STATICTEXT1 = wxNewId();
const long PlayListStepPanel::ID_TEXTCTRL1 = wxNewId();
const long PlayListStepPanel::ID_CHECKBOX1 = wxNewId();
const long PlayListStepPanel::ID_CHECKBOX2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PlayListStepPanel,wxPanel)
	//(*EventTable(PlayListStepPanel)
	//*)
END_EVENT_TABLE()

PlayListStepPanel::PlayListStepPanel(wxWindow* parent, PlayListStep* step, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _step = step;

	//(*Initialize(PlayListStepPanel)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_PlayListStepName = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(TextCtrl_PlayListStepName, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_ExcludeFromRandom = new wxCheckBox(this, ID_CHECKBOX1, _("Exclude from shuffle"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_ExcludeFromRandom->SetValue(false);
	FlexGridSizer1->Add(CheckBox_ExcludeFromRandom, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_EveryStep = new wxCheckBox(this, ID_CHECKBOX2, _("Run everything in this step in every step"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
	CheckBox_EveryStep->SetValue(false);
	FlexGridSizer1->Add(CheckBox_EveryStep, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListStepPanel::OnTextCtrl_PlayListStepNameText);
	//*)

    wxTextValidator tv(wxFILTER_EXCLUDE_CHAR_LIST);
    tv.SetCharExcludes("\"'`");
    TextCtrl_PlayListStepName->SetValidator(tv);

    TextCtrl_PlayListStepName->SetValue(step->GetRawName());
    CheckBox_ExcludeFromRandom->SetValue(step->GetExcludeFromRandom());
    CheckBox_EveryStep->SetValue(step->GetEveryStep());
}

PlayListStepPanel::~PlayListStepPanel()
{
	//(*Destroy(PlayListStepPanel)
	//*)
    _step->SetName(TextCtrl_PlayListStepName->GetValue().ToStdString());
    _step->SetExcludeFromRandom(CheckBox_ExcludeFromRandom->GetValue());
    _step->SetEveryStep(CheckBox_EveryStep->GetValue());
}

void PlayListStepPanel::OnTextCtrl_PlayListStepNameText(wxCommandEvent& event)
{
    _step->SetName(TextCtrl_PlayListStepName->GetValue().ToStdString());
    wxCommandEvent e(EVT_UPDATEITEMNAME);
    wxPostEvent(GetParent()->GetParent()->GetParent()->GetParent(), e);
}
