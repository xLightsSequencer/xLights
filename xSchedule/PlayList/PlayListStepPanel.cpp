#include "PlayListStepPanel.h"

#include "PlayListStep.h"

//(*InternalHeaders(PlayListStepPanel)
#include <wx/intl.h>
#include <wx/string.h>
#include "PlayListDialog.h"
//*)

//(*IdInit(PlayListStepPanel)
const long PlayListStepPanel::ID_STATICTEXT1 = wxNewId();
const long PlayListStepPanel::ID_TEXTCTRL1 = wxNewId();
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
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListStepPanel::OnTextCtrl_PlayListStepNameText);
	//*)

    TextCtrl_PlayListStepName->SetValue(step->GetName());
}

PlayListStepPanel::~PlayListStepPanel()
{
	//(*Destroy(PlayListStepPanel)
	//*)
}


void PlayListStepPanel::OnTextCtrl_PlayListStepNameText(wxCommandEvent& event)
{
    _step->SetName(TextCtrl_PlayListStepName->GetValue().ToStdString());
    ((PlayListDialog*)GetParent()->GetParent()->GetParent()->GetParent())->UpdateTree();
}
