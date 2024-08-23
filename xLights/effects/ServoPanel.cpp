/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ServoPanel.h"
#include "ServoEffect.h"
#include "EffectPanelUtils.h"

//(*InternalHeaders(ServoPanel)
#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>
#include <wx/tglbtn.h>
//*)

#include "xLightsMain.h"

//(*IdInit(ServoPanel)
const wxWindowID ServoPanel::ID_STATICTEXT_Channel = wxNewId();
const wxWindowID ServoPanel::ID_CHOICE_Channel = wxNewId();
const wxWindowID ServoPanel::ID_CHECKBOX_16bit = wxNewId();
const wxWindowID ServoPanel::ID_CHECKBOX_Timing_Track = wxNewId();
const wxWindowID ServoPanel::ID_CHOICE_Servo_TimingTrack = wxNewId();
const wxWindowID ServoPanel::ID_TOGGLEBUTTON_Start = wxNewId();
const wxWindowID ServoPanel::ID_STATICTEXT_Servo = wxNewId();
const wxWindowID ServoPanel::IDD_SLIDER_Servo = wxNewId();
const wxWindowID ServoPanel::ID_VALUECURVE_Servo = wxNewId();
const wxWindowID ServoPanel::ID_TEXTCTRL_Servo = wxNewId();
const wxWindowID ServoPanel::ID_TOGGLEBUTTON_End = wxNewId();
const wxWindowID ServoPanel::ID_STATICTEXT1 = wxNewId();
const wxWindowID ServoPanel::IDD_SLIDER_EndValue = wxNewId();
const wxWindowID ServoPanel::ID_TEXTCTRL_EndValue = wxNewId();
const wxWindowID ServoPanel::IDD_CHECKBOX_Sync = wxNewId();
const wxWindowID ServoPanel::ID_BUTTON1 = wxNewId();
const wxWindowID ServoPanel::IDD_SwapButton = wxNewId();
//*)

BEGIN_EVENT_TABLE(ServoPanel,wxPanel)
	//(*EventTable(ServoPanel)
	//*)
END_EVENT_TABLE()

ServoPanel::ServoPanel(wxWindow* parent) : xlEffectPanel(parent)
{
	//(*Initialize(ServoPanel)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer_Main = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer_Main->AddGrowableCol(0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer2->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT_Channel, _("Base Channel:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Channel"));
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Channel = new BulkEditChoice(this, ID_CHOICE_Channel, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Channel"));
	FlexGridSizer2->Add(Choice_Channel, 1, wxALL, 5);
	CheckBox_16bit = new BulkEditCheckBox(this, ID_CHECKBOX_16bit, _("16 bit"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_16bit"));
	CheckBox_16bit->SetValue(true);
	FlexGridSizer2->Add(CheckBox_16bit, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Timing_Track = new BulkEditCheckBox(this, ID_CHECKBOX_Timing_Track, _("Use Timing Track"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Timing_Track"));
	CheckBox_Timing_Track->SetValue(false);
	FlexGridSizer2->Add(CheckBox_Timing_Track, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Servo_TimingTrack = new BulkEditChoice(this, ID_CHOICE_Servo_TimingTrack, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Servo_TimingTrack"));
	Choice_Servo_TimingTrack->Disable();
	FlexGridSizer2->Add(Choice_Servo_TimingTrack, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer_Main->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer1 = new wxFlexGridSizer(0, 5, 0, 0);
	FlexGridSizer1->AddGrowableCol(2);
	StartLinkedButton = new LinkedToggleButton(this, ID_TOGGLEBUTTON_Start, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TOGGLEBUTTON_Start"));
	FlexGridSizer1->Add(StartLinkedButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Label_DMX1 = new wxStaticText(this, ID_STATICTEXT_Servo, _("Start:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Servo"));
	FlexGridSizer1->Add(Label_DMX1, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Servo = new BulkEditSliderF1(this, IDD_SLIDER_Servo, 0, 0, 1000, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Servo"));
	FlexGridSizer1->Add(Slider_Servo, 1, wxALL|wxEXPAND, 2);
	ValueCurve_Servo = new BulkEditValueCurveButton(this, ID_VALUECURVE_Servo, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Servo"));
	FlexGridSizer1->Add(ValueCurve_Servo, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Servo = new BulkEditTextCtrlF1(this, ID_TEXTCTRL_Servo, _T("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_Servo"));
	FlexGridSizer1->Add(TextCtrl_Servo, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	EndLinkedButton = new LinkedToggleButton(this, ID_TOGGLEBUTTON_End, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TOGGLEBUTTON_End"));
	FlexGridSizer1->Add(EndLinkedButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	EndDMXLabel = new wxStaticText(this, ID_STATICTEXT1, _("End:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(EndDMXLabel, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	SliderEndValue = new BulkEditSliderF1(this, IDD_SLIDER_EndValue, 0, 0, 1000, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_EndValue"));
	FlexGridSizer1->Add(SliderEndValue, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_EndValue = new BulkEditTextCtrlF1(this, ID_TEXTCTRL_EndValue, _T("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_EndValue"));
	FlexGridSizer1->Add(TextCtrl_EndValue, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer_Main->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer3 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	FlexGridSizer3->AddGrowableCol(1);
	SyncCheckBox = new wxCheckBox(this, IDD_CHECKBOX_Sync, _("Sync"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_CHECKBOX_Sync"));
	SyncCheckBox->SetValue(false);
	FlexGridSizer3->Add(SyncCheckBox, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3->Add(-1,-1,1, wxALL|wxEXPAND, 5);
	EqualButton = new wxButton(this, ID_BUTTON1, _("Equal"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer3->Add(EqualButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SwapButton = new wxButton(this, IDD_SwapButton, _("Swap"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SwapButton"));
	FlexGridSizer3->Add(SwapButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer_Main->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 0);
	SetSizer(FlexGridSizer_Main);

	Connect(ID_CHECKBOX_Timing_Track, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&ServoPanel::OnCheckBox_Timing_TrackClick);
	Connect(IDD_SLIDER_Servo, wxEVT_SLIDER, (wxObjectEventFunction)&ServoPanel::StartValueUpdated);
	Connect(ID_TEXTCTRL_Servo, wxEVT_COMMAND_TEXT_UPDATED, (wxObjectEventFunction)&ServoPanel::StartValueUpdated);
	Connect(IDD_SLIDER_EndValue, wxEVT_SLIDER, (wxObjectEventFunction)&ServoPanel::EndValueUpdated);
	Connect(ID_TEXTCTRL_EndValue, wxEVT_COMMAND_TEXT_UPDATED, (wxObjectEventFunction)&ServoPanel::EndValueUpdated);
	Connect(IDD_CHECKBOX_Sync, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&ServoPanel::OnSyncCheckBoxClick);
	Connect(ID_BUTTON1, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ServoPanel::OnEqualButtonClick);
	Connect(IDD_SwapButton, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ServoPanel::OnSwapButtonClick);
	//*)

    SetName("ID_PANEL_SERVO");

	Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&ServoPanel::OnVCChanged, nullptr, this);
	Connect(wxID_ANY, EVT_VALIDATEWINDOW, (wxObjectEventFunction)&ServoPanel::OnValidateWindow, nullptr, this);
    Connect(ID_VALUECURVE_Servo, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ServoPanel::OnVCButtonClick);
    
    ValueCurve_Servo->GetValue()->SetLimits(SERVO_MIN, SERVO_MAX);
    ValueCurve_Servo->GetValue()->SetDivisor(SERVO_DIVISOR);

	ValidateWindow();
}

ServoPanel::~ServoPanel()
{
	//(*Destroy(ServoPanel)
	//*)
}

void ServoPanel::ValidateWindow()
{
	if (CheckBox_Timing_Track->IsChecked()) {
		Choice_Servo_TimingTrack->Enable();
        if (!Choice_Servo_TimingTrack->GetStringSelection().empty()) {
            SliderEndValue->Disable();
            TextCtrl_EndValue->Disable();
            EndDMXLabel->Disable();
            TextCtrl_Servo->Disable();
            Slider_Servo->Disable();
            ValueCurve_Servo->Disable();
            Label_DMX1->Disable();
            SwapButton->Disable();
            EqualButton->Disable();
            SyncCheckBox->Disable();
            StartLinkedButton->Disable();
            EndLinkedButton->Disable();
        }
	} else {
		Choice_Servo_TimingTrack->Disable();

        TextCtrl_Servo->Enable();
        Slider_Servo->Enable();
        ValueCurve_Servo->Enable();
        Label_DMX1->Enable();

        if (ValueCurve_Servo->GetValue()->IsActive()) {
            Label_DMX1->SetLabelText("Value:");
            SliderEndValue->Disable();
            TextCtrl_EndValue->Disable();
            EndDMXLabel->Disable();
            SwapButton->Disable();
            EqualButton->Disable();
            SyncCheckBox->SetValue(false);
            SyncCheckBox->Disable();
            StartLinkedButton->Disable();
            EndLinkedButton->Disable();
        } else {
            Label_DMX1->SetLabelText("Start:");
            EndDMXLabel->Enable();
            SwapButton->Enable();
            EqualButton->Enable();
            SyncCheckBox->Enable();
            if (TextCtrl_Servo->GetValue() != TextCtrl_EndValue->GetValue()) {
                SyncCheckBox->SetValue(false);
            }
            SliderEndValue->Enable();
            TextCtrl_EndValue->Enable();
            StartLinkedButton->Enable();
            EndLinkedButton->Enable();
        }
	}
}

void ServoPanel::OnCheckBox_Timing_TrackClick(wxCommandEvent& event)
{
	ValidateWindow();
}

void ServoPanel::OnSwapButtonClick(wxCommandEvent& event)
{
    wxString v1 = TextCtrl_Servo->GetValue();
    wxString v2 = TextCtrl_EndValue->GetValue();
    TextCtrl_Servo->SetValue(v2);
    TextCtrl_EndValue->SetValue(v1);
}

void ServoPanel::OnEqualButtonClick(wxCommandEvent& event)
{
    wxString v1 = TextCtrl_Servo->GetValue();
    TextCtrl_EndValue->SetValue(v1);
}

void ServoPanel::OnSyncCheckBoxClick(wxCommandEvent& event)
{
    if (SyncCheckBox->IsChecked()) {
        SliderEndValue->SetValue(Slider_Servo->GetValue());
        TextCtrl_EndValue->SetValue(TextCtrl_Servo->GetValue());
        FireChangeEvent();
    }
}

void ServoPanel::StartValueUpdated(wxCommandEvent& event)
{
    bool changed = false;
    if (SyncCheckBox->IsChecked()) {
        SliderEndValue->SetValue(Slider_Servo->GetValue());
        TextCtrl_EndValue->ChangeValue(TextCtrl_Servo->GetValue());
        changed = true;
    }
    if (StartLinkedButton->IsEnabled() && StartLinkedButton->GetValue()) {
        xLightsFrame::GetFrame()->CallOnEffectBeforeSelected([&changed, this](Effect *eff) {
            if (eff->GetEffectIndex() == EffectManager::eff_SERVO) {
                changed |= eff->SetSetting("E_TEXTCTRL_EndValue", TextCtrl_Servo->GetValue());
            }
            return changed;
        });
    }
    if (changed) {
        FireChangeEvent();
    }

}

void ServoPanel::EndValueUpdated(wxCommandEvent& event)
{
    bool changed = false;
    if (SyncCheckBox->IsChecked()) {
        Slider_Servo->SetValue(SliderEndValue->GetValue());
        TextCtrl_Servo->ChangeValue(TextCtrl_EndValue->GetValue());
        changed = true;
    }
    if (EndLinkedButton->IsEnabled() && EndLinkedButton->GetValue()) {
        xLightsFrame::GetFrame()->CallOnEffectAfterSelected([&changed, this](Effect *eff) {
            if (eff->GetEffectIndex() == EffectManager::eff_SERVO) {
                changed |= eff->SetSetting("E_TEXTCTRL_Servo", TextCtrl_EndValue->GetValue());
            }
            return changed;
        });
    }
    if (changed) {
        FireChangeEvent();
    }
}
