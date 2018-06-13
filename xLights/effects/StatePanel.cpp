#include "StatePanel.h"
#include "EffectPanelUtils.h"
#include <list>
#include "StateEffect.h"

//(*InternalHeaders(StatePanel)
#include <wx/choice.h>
#include <wx/intl.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/string.h>
//*)

//(*IdInit(StatePanel)
const long StatePanel::ID_STATICTEXT15 = wxNewId();
const long StatePanel::ID_CHOICE_State_StateDefinition = wxNewId();
const long StatePanel::IDD_RADIOBUTTON_State_State = wxNewId();
const long StatePanel::ID_CHOICE_State_State = wxNewId();
const long StatePanel::IDD_RADIOBUTTON_State_TimingTrack = wxNewId();
const long StatePanel::ID_CHOICE_State_TimingTrack = wxNewId();
const long StatePanel::ID_STATICTEXT_State_Mode = wxNewId();
const long StatePanel::ID_CHOICE_State_Mode = wxNewId();
const long StatePanel::ID_STATICTEXT_State_Color = wxNewId();
const long StatePanel::ID_CHOICE_State_Color = wxNewId();
//*)

BEGIN_EVENT_TABLE(StatePanel,wxPanel)
	//(*EventTable(StatePanel)
	//*)
END_EVENT_TABLE()

StatePanel::StatePanel(wxWindow* parent)
{
    _effect = nullptr;
    _model = nullptr;

	//(*Initialize(StatePanel)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer47;
	wxFlexGridSizer* FlexGridSizer97;
	wxFlexGridSizer* FlexGridSizer98;
	wxStaticBoxSizer* StaticBoxSizer2;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer47 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer47->AddGrowableCol(0);
	FlexGridSizer98 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer98->AddGrowableCol(1);
	StaticText14 = new wxStaticText(this, ID_STATICTEXT15, _("State Definition"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT15"));
	FlexGridSizer98->Add(StaticText14, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_StateDefinitonChoice = new wxChoice(this, ID_CHOICE_State_StateDefinition, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_State_StateDefinition"));
	FlexGridSizer98->Add(Choice_StateDefinitonChoice, 1, wxALL|wxEXPAND, 5);
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
	Choice_State_TimingTrack = new BulkEditChoice(this, ID_CHOICE_State_TimingTrack, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_State_TimingTrack"));
	Choice_State_TimingTrack->Disable();
	FlexGridSizer97->Add(Choice_State_TimingTrack, 1, wxALL|wxEXPAND, 5);
	StaticBoxSizer2->Add(FlexGridSizer97, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer47->Add(StaticBoxSizer2, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT_State_Mode, _("Mode"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_State_Mode"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_State_Mode = new BulkEditChoice(this, ID_CHOICE_State_Mode, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_State_Mode"));
	Choice_State_Mode->SetSelection( Choice_State_Mode->Append(_("Default")) );
	Choice_State_Mode->Append(_("Countdown"));
	Choice_State_Mode->Append(_("Time Countdown"));
	Choice_State_Mode->Append(_("Number"));
	Choice_State_Mode->Append(_("Iterate"));
	FlexGridSizer1->Add(Choice_State_Mode, 1, wxALL|wxEXPAND, 2);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT_State_Color, _("Color"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_State_Color"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_State_Color = new BulkEditChoice(this, ID_CHOICE_State_Color, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_State_Color"));
	Choice_State_Color->SetSelection( Choice_State_Color->Append(_("Graduate")) );
	Choice_State_Color->Append(_("Cycle"));
	Choice_State_Color->Append(_("Allocate"));
	FlexGridSizer1->Add(Choice_State_Color, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer47->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 2);
	SetSizer(FlexGridSizer47);
	FlexGridSizer47->Fit(this);
	FlexGridSizer47->SetSizeHints(this);

	Connect(ID_CHOICE_State_StateDefinition,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&StatePanel::OnState_StateDefinitonChoiceSelect);
	Connect(IDD_RADIOBUTTON_State_State,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&StatePanel::OnMouthMovementTypeSelected);
	Connect(IDD_RADIOBUTTON_State_TimingTrack,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&StatePanel::OnMouthMovementTypeSelected);
	//*)

    UpdateStateList();

    SetName("ID_PANEL_State");
}

StatePanel::~StatePanel()
{
	//(*Destroy(StatePanel)
	//*)
}

PANEL_EVENT_HANDLERS(StatePanel)

void StatePanel::UpdateStateList()
{
    if (_effect != NULL)
    {
        wxString selected = Choice_State_State->GetStringSelection();
        Choice_State_State->Clear();
        std::list<std::string> states = _effect->GetStates(_model, Choice_StateDefinitonChoice->GetStringSelection().ToStdString());

        for (auto it = states.begin(); it != states.end(); ++it)
        {
            int item = Choice_State_State->Append(*it);
            if (*it == selected)
            {
                Choice_State_State->SetSelection(item);
            }
        }
    }
}

void StatePanel::SetEffect(StateEffect* effect, Model* model)
{
    _effect = effect;
    _model = model;

    UpdateStateList();
}

void StatePanel::OnMouthMovementTypeSelected(wxCommandEvent& event)
{
    if (event.GetId() == IDD_RADIOBUTTON_State_State) {
        Choice_State_State->Enable();
        Choice_State_TimingTrack->Disable();
        Choice_State_Mode->Disable();
    } else {
        Choice_State_State->Disable();
        Choice_State_TimingTrack->Enable();
        Choice_State_Mode->Enable();
    }
}

void StatePanel::OnState_StateDefinitonChoiceSelect(wxCommandEvent& event)
{
    UpdateStateList();
}
