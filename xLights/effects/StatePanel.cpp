#include "StatePanel.h"
#include "../../include/padlock16x16-blue.xpm"
#include "EffectPanelUtils.h"

//(*InternalHeaders(StatePanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/radiobut.h>
#include <wx/choice.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(StatePanel)
const long StatePanel::ID_STATICTEXT15 = wxNewId();
const long StatePanel::ID_CHOICE_State_StateDefinition = wxNewId();
const long StatePanel::IDD_RADIOBUTTON_State_State = wxNewId();
const long StatePanel::ID_CHOICE_State_State = wxNewId();
const long StatePanel::IDD_RADIOBUTTON_State_TimingTrack = wxNewId();
const long StatePanel::ID_CHOICE_State_TimingTrack = wxNewId();
//*)

BEGIN_EVENT_TABLE(StatePanel,wxPanel)
	//(*EventTable(StatePanel)
	//*)
END_EVENT_TABLE()

StatePanel::StatePanel(wxWindow* parent)
{
	//(*Initialize(StatePanel)
	wxStaticBoxSizer* StaticBoxSizer2;
	wxFlexGridSizer* FlexGridSizer47;
	wxFlexGridSizer* FlexGridSizer97;
	wxFlexGridSizer* FlexGridSizer98;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer47 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer47->AddGrowableCol(0);
	FlexGridSizer98 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer98->AddGrowableCol(1);
	StaticText14 = new wxStaticText(this, ID_STATICTEXT15, _("State Definition"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT15"));
	FlexGridSizer98->Add(StaticText14, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	State_StateDefinitonChoice = new wxChoice(this, ID_CHOICE_State_StateDefinition, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_State_StateDefinition"));
	FlexGridSizer98->Add(State_StateDefinitonChoice, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer47->Add(FlexGridSizer98, 1, wxALL|wxEXPAND, 2);
	StaticBoxSizer2 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("States"));
	FlexGridSizer97 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer97->AddGrowableCol(1);
	RadioButton1 = new wxRadioButton(this, IDD_RADIOBUTTON_State_State, _("State"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_RADIOBUTTON_State_State"));
	RadioButton1->SetValue(true);
	FlexGridSizer97->Add(RadioButton1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_State_State = new wxChoice(this, ID_CHOICE_State_State, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_State_State"));
	FlexGridSizer97->Add(Choice_State_State, 1, wxALL|wxEXPAND, 5);
	RadioButton2 = new wxRadioButton(this, IDD_RADIOBUTTON_State_TimingTrack, _("Timing Track"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_RADIOBUTTON_State_TimingTrack"));
	FlexGridSizer97->Add(RadioButton2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_State_TimingTrack = new wxChoice(this, ID_CHOICE_State_TimingTrack, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_State_TimingTrack"));
	Choice_State_TimingTrack->Disable();
	FlexGridSizer97->Add(Choice_State_TimingTrack, 1, wxALL|wxEXPAND, 5);
	StaticBoxSizer2->Add(FlexGridSizer97, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer47->Add(StaticBoxSizer2, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer47);
	FlexGridSizer47->Fit(this);
	FlexGridSizer47->SetSizeHints(this);

	Connect(IDD_RADIOBUTTON_State_State,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&StatePanel::OnMouthMovementTypeSelected);
	Connect(IDD_RADIOBUTTON_State_TimingTrack,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&StatePanel::OnMouthMovementTypeSelected);
	//*)
    SetName("ID_PANEL_State");
}

StatePanel::~StatePanel()
{
	//(*Destroy(StatePanel)
	//*)
}

PANEL_EVENT_HANDLERS(StatePanel)

void StatePanel::OnMouthMovementTypeSelected(wxCommandEvent& event)
{
    if (event.GetId() == IDD_RADIOBUTTON_State_Phoneme) {
        Choice_State_Phoneme->Enable();
        Choice_State_TimingTrack->Disable();
    } else {
        Choice_State_Phoneme->Disable();
        Choice_State_TimingTrack->Enable();
    }
}
