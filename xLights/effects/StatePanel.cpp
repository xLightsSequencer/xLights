/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "StatePanel.h"
#include "EffectPanelUtils.h"
#include <list>
#include "StateEffect.h"

//(*InternalHeaders(StatePanel)
#include <wx/choice.h>
#include <wx/intl.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>
//*)

//(*IdInit(StatePanel)
const wxWindowID StatePanel::ID_STATICTEXT15 = wxNewId();
const wxWindowID StatePanel::ID_CHOICE_State_StateDefinition = wxNewId();
const wxWindowID StatePanel::IDD_RADIOBUTTON_State_State = wxNewId();
const wxWindowID StatePanel::ID_CHOICE_State_State = wxNewId();
const wxWindowID StatePanel::IDD_RADIOBUTTON_State_TimingTrack = wxNewId();
const wxWindowID StatePanel::ID_CHOICE_State_TimingTrack = wxNewId();
const wxWindowID StatePanel::ID_STATICTEXT_State_Mode = wxNewId();
const wxWindowID StatePanel::ID_CHOICE_State_Mode = wxNewId();
const wxWindowID StatePanel::ID_STATICTEXT_State_Color = wxNewId();
const wxWindowID StatePanel::ID_CHOICE_State_Color = wxNewId();
const wxWindowID StatePanel::ID_STATICTEXT1 = wxNewId();
const wxWindowID StatePanel::ID_SLIDER_State_Fade_Time = wxNewId();
const wxWindowID StatePanel::IDD_TEXTCTRL_State_Fade_Time = wxNewId();
//*)

BEGIN_EVENT_TABLE(StatePanel,wxPanel)
	//(*EventTable(StatePanel)
	//*)
END_EVENT_TABLE()

StatePanel::StatePanel(wxWindow* parent) : xlEffectPanel(parent)
{
    _effect = nullptr;
    _model = nullptr;

	//(*Initialize(StatePanel)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
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
	Choice_StateDefinitonChoice = new BulkEditStateChoice(this, ID_CHOICE_State_StateDefinition, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_State_StateDefinition"));
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
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer2->AddGrowableCol(1);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT1, _("Fade(ms)"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer2->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SLIDER_State_Fade_Time = new BulkEditSlider(this, ID_SLIDER_State_Fade_Time, 0, 0, 1000, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_State_Fade_Time"));
	SLIDER_State_Fade_Time->SetTickFreq(100);
	SLIDER_State_Fade_Time->SetTick(1);
	FlexGridSizer2->Add(SLIDER_State_Fade_Time, 1, wxALL|wxEXPAND, 5);
	TextCtrl_State_Fade_Time = new BulkEditTextCtrl(this, IDD_TEXTCTRL_State_Fade_Time, _T("0"), wxDefaultPosition, wxSize(45,-1), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_State_Fade_Time"));
	TextCtrl_State_Fade_Time->SetMaxLength(4);
	FlexGridSizer2->Add(TextCtrl_State_Fade_Time, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer47->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 2);
	SetSizer(FlexGridSizer47);

	Connect(ID_CHOICE_State_StateDefinition, wxEVT_COMMAND_CHOICE_SELECTED, (wxObjectEventFunction)&StatePanel::OnState_StateDefinitonChoiceSelect);
	Connect(IDD_RADIOBUTTON_State_State, wxEVT_COMMAND_RADIOBUTTON_SELECTED, (wxObjectEventFunction)&StatePanel::OnMouthMovementTypeSelected);
	Connect(IDD_RADIOBUTTON_State_TimingTrack, wxEVT_COMMAND_RADIOBUTTON_SELECTED, (wxObjectEventFunction)&StatePanel::OnMouthMovementTypeSelected);
	//*)

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&StatePanel::OnVCChanged, 0, this);
    Connect(wxID_ANY, EVT_VALIDATEWINDOW, (wxObjectEventFunction)&StatePanel::OnValidateWindow, 0, this);

    UpdateStateList();

    SetName("ID_PANEL_State");

    ValidateWindow();
}

StatePanel::~StatePanel()
{
	//(*Destroy(StatePanel)
	//*)
}

void StatePanel::ValidateWindow()
{
	if (_effect == nullptr) 		{
		Choice_State_State->Enable();
		Choice_State_TimingTrack->Disable();
		Choice_State_Mode->Disable();
	}
	else {
		if (RadioButton1->GetValue()) {
			Choice_State_State->Enable();
			Choice_State_TimingTrack->Disable();
			Choice_State_Mode->Disable();
		}
		else {
			Choice_State_State->Disable();
			Choice_State_TimingTrack->Enable();
			Choice_State_Mode->Enable();
		}
	}
}

void StatePanel::UpdateStateList()
{
	if (_effect != nullptr) {
		wxString selected = Choice_State_State->GetStringSelection();
		Choice_State_State->Clear();
		std::list<std::string> states = _effect->GetStates(_model, Choice_StateDefinitonChoice->GetStringSelection().ToStdString());

		Choice_State_State->Append("<ALL>");
		if (selected == "<ALL>") {
			Choice_State_State->SetSelection(0);
		}

		for (const auto& it : states) {
			int item = Choice_State_State->Append(it);
			if (it == selected) {
				Choice_State_State->SetSelection(item);
			}
		}
	}
	else {
		RadioButton1->SetValue(true);
		Choice_State_State->Clear();
		Choice_State_State->Append("<ALL>");
		Choice_State_State->SetSelection(0);
	}

	ValidateWindow();
}

void StatePanel::SetEffect(StateEffect* effect, Model* model)
{
    _effect = effect;
    _model = model;

    UpdateStateList();
}

void StatePanel::OnMouthMovementTypeSelected(wxCommandEvent& event)
{
	ValidateWindow();
}

void StatePanel::OnState_StateDefinitonChoiceSelect(wxCommandEvent& event)
{
    UpdateStateList();
}
