/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "TwinklePanel.h"
#include "TwinkleEffect.h"
#include "EffectPanelUtils.h"

//(*InternalHeaders(TwinklePanel)
#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>
//*)

//(*IdInit(TwinklePanel)
const long TwinklePanel::ID_STATICTEXT_Twinkle_Count = wxNewId();
const long TwinklePanel::ID_SLIDER_Twinkle_Count = wxNewId();
const long TwinklePanel::ID_VALUECURVE_Twinkle_Count = wxNewId();
const long TwinklePanel::IDD_TEXTCTRL_Twinkle_Count = wxNewId();
const long TwinklePanel::ID_BITMAPBUTTON_SLIDER_Twinkle_Count = wxNewId();
const long TwinklePanel::ID_STATICTEXT_Twinkle_Steps = wxNewId();
const long TwinklePanel::ID_SLIDER_Twinkle_Steps = wxNewId();
const long TwinklePanel::ID_VALUECURVE_Twinkle_Steps = wxNewId();
const long TwinklePanel::IDD_TEXTCTRL_Twinkle_Steps = wxNewId();
const long TwinklePanel::ID_BITMAPBUTTON_SLIDER_Twinkle_Steps = wxNewId();
const long TwinklePanel::ID_STATICTEXT1 = wxNewId();
const long TwinklePanel::ID_CHOICE_Twinkle_Style = wxNewId();
const long TwinklePanel::ID_CHECKBOX_Twinkle_Strobe = wxNewId();
const long TwinklePanel::ID_BITMAPBUTTON_CHECKBOX_Twinkle_Strobe = wxNewId();
const long TwinklePanel::ID_CHECKBOX_Twinkle_ReRandom = wxNewId();
const long TwinklePanel::ID_BITMAPBUTTON_CHECKBOX_Twinkle_ReRandom = wxNewId();
//*)

BEGIN_EVENT_TABLE(TwinklePanel,wxPanel)
	//(*EventTable(TwinklePanel)
	//*)
END_EVENT_TABLE()

TwinklePanel::TwinklePanel(wxWindow* parent) : xlEffectPanel(parent)
{
	//(*Initialize(TwinklePanel)
	BulkEditTextCtrl* TextCtrl79;
	BulkEditTextCtrl* TextCtrl80;
	wxFlexGridSizer* FlexGridSizer4;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer4 = new wxFlexGridSizer(0, 5, 0, 0);
	FlexGridSizer4->AddGrowableCol(1);
	StaticText85 = new wxStaticText(this, ID_STATICTEXT_Twinkle_Count, _("Percent of Lights"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Twinkle_Count"));
	FlexGridSizer4->Add(StaticText85, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Twinkle_Count = new BulkEditSlider(this, ID_SLIDER_Twinkle_Count, 3, 2, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Twinkle_Count"));
	FlexGridSizer4->Add(Slider_Twinkle_Count, 1, wxALL|wxEXPAND, 5);
	BitmapButton_Twinkle_CountVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Twinkle_Count, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_VALUECURVE_Twinkle_Count"));
	FlexGridSizer4->Add(BitmapButton_Twinkle_CountVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl79 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Twinkle_Count, _("3"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Twinkle_Count"));
	TextCtrl79->SetMaxLength(3);
	FlexGridSizer4->Add(TextCtrl79, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_TwinkleCount = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Twinkle_Count, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Twinkle_Count"));
	BitmapButton_TwinkleCount->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer4->Add(BitmapButton_TwinkleCount, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText104 = new wxStaticText(this, ID_STATICTEXT_Twinkle_Steps, _("Twinkle Steps"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Twinkle_Steps"));
	FlexGridSizer4->Add(StaticText104, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Twinkle_Steps = new BulkEditSlider(this, ID_SLIDER_Twinkle_Steps, 30, 2, 400, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Twinkle_Steps"));
	FlexGridSizer4->Add(Slider_Twinkle_Steps, 1, wxALL|wxEXPAND, 5);
	BitmapButton_Twinkle_StepsVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Twinkle_Steps, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_VALUECURVE_Twinkle_Steps"));
	FlexGridSizer4->Add(BitmapButton_Twinkle_StepsVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl80 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Twinkle_Steps, _("30"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Twinkle_Steps"));
	TextCtrl80->SetMaxLength(3);
	FlexGridSizer4->Add(TextCtrl80, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_TwinkleSteps = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Twinkle_Steps, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Twinkle_Steps"));
	BitmapButton_TwinkleSteps->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer4->Add(BitmapButton_TwinkleSteps, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Style"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer4->Add(StaticText1, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Twinkle_Style = new BulkEditChoice(this, ID_CHOICE_Twinkle_Style, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Twinkle_Style"));
	Choice_Twinkle_Style->SetSelection( Choice_Twinkle_Style->Append(_("Old Render Method")) );
	Choice_Twinkle_Style->Append(_("New Render Method"));
	FlexGridSizer4->Add(Choice_Twinkle_Style, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Twinkle_Strobe = new BulkEditCheckBox(this, ID_CHECKBOX_Twinkle_Strobe, _("Strobe"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Twinkle_Strobe"));
	CheckBox_Twinkle_Strobe->SetValue(false);
	FlexGridSizer4->Add(CheckBox_Twinkle_Strobe, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_TwinkleStrobe = new xlLockButton(this, ID_BITMAPBUTTON_CHECKBOX_Twinkle_Strobe, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Twinkle_Strobe"));
	BitmapButton_TwinkleStrobe->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer4->Add(BitmapButton_TwinkleStrobe, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer4->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Twinkle_ReRandom = new BulkEditCheckBox(this, ID_CHECKBOX_Twinkle_ReRandom, _("Re-Randomize after twinkle"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Twinkle_ReRandom"));
	CheckBox_Twinkle_ReRandom->SetValue(false);
	FlexGridSizer4->Add(CheckBox_Twinkle_ReRandom, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton24 = new xlLockButton(this, ID_BITMAPBUTTON_CHECKBOX_Twinkle_ReRandom, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Twinkle_ReRandom"));
	BitmapButton24->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer4->Add(BitmapButton24, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer4);
	FlexGridSizer4->Fit(this);
	FlexGridSizer4->SetSizeHints(this);

	Connect(ID_BITMAPBUTTON_SLIDER_Twinkle_Count,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TwinklePanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Twinkle_Steps,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TwinklePanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Twinkle_ReRandom,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TwinklePanel::OnLockButtonClick);
	//*)

	Connect(ID_VALUECURVE_Twinkle_Count, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&TwinklePanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_Twinkle_Steps, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&TwinklePanel::OnVCButtonClick);
	Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&TwinklePanel::OnVCChanged, 0, this);
    Connect(wxID_ANY, EVT_VALIDATEWINDOW, (wxObjectEventFunction)&TwinklePanel::OnValidateWindow, 0, this);

	BitmapButton_Twinkle_CountVC->GetValue()->SetLimits(TWINKLE_COUNT_MIN, TWINKLE_COUNT_MAX);
    BitmapButton_Twinkle_StepsVC->GetValue()->SetLimits(TWINKLE_STEPS_MIN, TWINKLE_STEPS_MAX);

    SetName("ID_PANEL_TWINKLE");

    ValidateWindow();
}

TwinklePanel::~TwinklePanel()
{
	//(*Destroy(TwinklePanel)
	//*)
}

void TwinklePanel::ValidateWindow()
{
}
