/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "SnowflakesPanel.h"
#include "EffectPanelUtils.h"
#include "SnowflakesEffect.h"

//(*InternalHeaders(SnowflakesPanel)
#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#include <wx/choice.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>
//*)

//(*IdInit(SnowflakesPanel)
const long SnowflakesPanel::ID_STATICTEXT_Snowflakes_Count = wxNewId();
const long SnowflakesPanel::ID_SLIDER_Snowflakes_Count = wxNewId();
const long SnowflakesPanel::ID_VALUECURVE_Snowflakes_Count = wxNewId();
const long SnowflakesPanel::IDD_TEXTCTRL_Snowflakes_Count = wxNewId();
const long SnowflakesPanel::ID_BITMAPBUTTON_SLIDER_Snowflakes_Count = wxNewId();
const long SnowflakesPanel::ID_STATICTEXT_Snowflakes_Type = wxNewId();
const long SnowflakesPanel::ID_SLIDER_Snowflakes_Type = wxNewId();
const long SnowflakesPanel::IDD_TEXTCTRL_Snowflakes_Type = wxNewId();
const long SnowflakesPanel::ID_BITMAPBUTTON_SLIDER_Snowflakes_Type = wxNewId();
const long SnowflakesPanel::ID_STATICTEXT_Snowflakes_Speed = wxNewId();
const long SnowflakesPanel::ID_SLIDER_Snowflakes_Speed = wxNewId();
const long SnowflakesPanel::ID_VALUECURVE_Snowflakes_Speed = wxNewId();
const long SnowflakesPanel::IDD_TEXTCTRL_Snowflakes_Speed = wxNewId();
const long SnowflakesPanel::ID_BITMAPBUTTON_SLIDER_Snowflakes_Speed = wxNewId();
const long SnowflakesPanel::ID_STATICTEXT_Falling = wxNewId();
const long SnowflakesPanel::ID_CHOICE_Falling = wxNewId();
const long SnowflakesPanel::ID_STATICTEXT2 = wxNewId();
const long SnowflakesPanel::ID_BITMAPBUTTON_CHOICE_Falling = wxNewId();
const long SnowflakesPanel::ID_STATICTEXT1 = wxNewId();
const long SnowflakesPanel::ID_SLIDER_Snowflakes_WarmupFrames = wxNewId();
const long SnowflakesPanel::IDD_TEXTCTRL_Snowflakes_WarmupFrames = wxNewId();
//*)

BEGIN_EVENT_TABLE(SnowflakesPanel,wxPanel)
	//(*EventTable(SnowflakesPanel)
	//*)
END_EVENT_TABLE()

SnowflakesPanel::SnowflakesPanel(wxWindow* parent) : xlEffectPanel(parent)
{
	//(*Initialize(SnowflakesPanel)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer43;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer43 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer43->AddGrowableCol(1);
	StaticText79 = new wxStaticText(this, ID_STATICTEXT_Snowflakes_Count, _("Max flakes"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Snowflakes_Count"));
	FlexGridSizer43->Add(StaticText79, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	Slider_Snowflakes_Count = new BulkEditSlider(this, ID_SLIDER_Snowflakes_Count, 5, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Snowflakes_Count"));
	FlexGridSizer1->Add(Slider_Snowflakes_Count, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Snowflakes_Count = new BulkEditValueCurveButton(this, ID_VALUECURVE_Snowflakes_Count, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Snowflakes_Count"));
	FlexGridSizer1->Add(BitmapButton_Snowflakes_Count, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer43->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Snowflakes_Count = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Snowflakes_Count, _("5"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Snowflakes_Count"));
	TextCtrl_Snowflakes_Count->SetMaxLength(3);
	FlexGridSizer43->Add(TextCtrl_Snowflakes_Count, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_SnowflakesCount = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Snowflakes_Count, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Snowflakes_Count"));
	FlexGridSizer43->Add(BitmapButton_SnowflakesCount, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText80 = new wxStaticText(this, ID_STATICTEXT_Snowflakes_Type, _("Type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Snowflakes_Type"));
	FlexGridSizer43->Add(StaticText80, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Snowflakes_Type = new BulkEditSlider(this, ID_SLIDER_Snowflakes_Type, 1, 0, 9, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Snowflakes_Type"));
	FlexGridSizer43->Add(Slider_Snowflakes_Type, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Snowflakes_Type = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Snowflakes_Type, _("1"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Snowflakes_Type"));
	TextCtrl_Snowflakes_Type->SetMaxLength(3);
	FlexGridSizer43->Add(TextCtrl_Snowflakes_Type, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_SnowflakesType = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Snowflakes_Type, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Snowflakes_Type"));
	FlexGridSizer43->Add(BitmapButton_SnowflakesType, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText181 = new wxStaticText(this, ID_STATICTEXT_Snowflakes_Speed, _("Speed"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Snowflakes_Speed"));
	FlexGridSizer43->Add(StaticText181, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	Slider_Snowflakes_Speed = new BulkEditSlider(this, ID_SLIDER_Snowflakes_Speed, 10, 0, 50, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Snowflakes_Speed"));
	FlexGridSizer3->Add(Slider_Snowflakes_Speed, 1, wxALL|wxEXPAND, 5);
	BitmapButton_Snowflakes_Speed = new BulkEditValueCurveButton(this, ID_VALUECURVE_Snowflakes_Speed, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Snowflakes_Speed"));
	FlexGridSizer3->Add(BitmapButton_Snowflakes_Speed, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer43->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Snowflakes_Speed = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Snowflakes_Speed, _("10"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Snowflakes_Speed"));
	TextCtrl_Snowflakes_Speed->SetMaxLength(3);
	FlexGridSizer43->Add(TextCtrl_Snowflakes_Speed, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_SnowflakesSpeed = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Snowflakes_Speed, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Snowflakes_Speed"));
	FlexGridSizer43->Add(BitmapButton_SnowflakesSpeed, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT_Falling, _("Falling"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Falling"));
	FlexGridSizer43->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Falling = new BulkEditChoice(this, ID_CHOICE_Falling, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Falling"));
	Choice_Falling->SetSelection( Choice_Falling->Append(_("Driving")) );
	Choice_Falling->Append(_("Falling"));
	Choice_Falling->Append(_("Falling & Accumulating"));
	FlexGridSizer43->Add(Choice_Falling, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer43->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Falling = new xlLockButton(this, ID_BITMAPBUTTON_CHOICE_Falling, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Falling"));
	FlexGridSizer43->Add(BitmapButton_Falling, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT1, _("Warm up frames"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer43->Add(StaticText3, 1, wxALL|wxEXPAND, 2);
	Slider_WarmupFrames = new BulkEditSlider(this, ID_SLIDER_Snowflakes_WarmupFrames, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Snowflakes_WarmupFrames"));
	FlexGridSizer43->Add(Slider_WarmupFrames, 1, wxALL|wxEXPAND, 2);
	TextCtrl_WarmupFrames = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Snowflakes_WarmupFrames, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Snowflakes_WarmupFrames"));
	TextCtrl_WarmupFrames->SetMaxLength(3);
	FlexGridSizer43->Add(TextCtrl_WarmupFrames, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer43->Add(-1,-1,1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer43);
	FlexGridSizer43->Fit(this);
	FlexGridSizer43->SetSizeHints(this);

	Connect(ID_VALUECURVE_Snowflakes_Count,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SnowflakesPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Snowflakes_Count,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SnowflakesPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Snowflakes_Type,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SnowflakesPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Snowflakes_Speed,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SnowflakesPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Snowflakes_Speed,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SnowflakesPanel::OnLockButtonClick);
	Connect(ID_CHOICE_Falling,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&SnowflakesPanel::OnChoice_FallingSelect);
	Connect(ID_BITMAPBUTTON_CHOICE_Falling,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SnowflakesPanel::OnLockButtonClick);
	//*)
    SetName("ID_PANEL_SNOWFLAKES");

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&SnowflakesPanel::OnVCChanged, 0, this);
    Connect(wxID_ANY, EVT_VALIDATEWINDOW, (wxObjectEventFunction)&SnowflakesPanel::OnValidateWindow, 0, this);

    BitmapButton_Snowflakes_Count->GetValue()->SetLimits(SNOWFLAKES_COUNT_MIN, SNOWFLAKES_COUNT_MAX);
    BitmapButton_Snowflakes_Speed->GetValue()->SetLimits(SNOWFLAKES_SPEED_MIN, SNOWFLAKES_SPEED_MAX);

    ValidateWindow();
}

SnowflakesPanel::~SnowflakesPanel()
{
	//(*Destroy(SnowflakesPanel)
	//*)
}

void SnowflakesPanel::OnChoice_FallingSelect(wxCommandEvent& event)
{
    ValidateWindow();
}

void SnowflakesPanel::ValidateWindow()
{
    if (Choice_Falling->GetStringSelection() == "Driving") {
        BitmapButton_Snowflakes_Count->Disable();
        BitmapButton_Snowflakes_Speed->Disable();
        BitmapButton_Snowflakes_Count->GetValue()->SetActive(false);
        BitmapButton_Snowflakes_Speed->GetValue()->SetActive(false);

        Slider_WarmupFrames->Enable(false);
        TextCtrl_WarmupFrames->Enable(false);

        // Because disabling value curve also disables slider and text we need to re-enable them
        Slider_Snowflakes_Count->Enable();
        Slider_Snowflakes_Speed->Enable();
        TextCtrl_Snowflakes_Count->Enable();
        TextCtrl_Snowflakes_Speed->Enable();
    } else {
        BitmapButton_Snowflakes_Count->Enable();
        BitmapButton_Snowflakes_Speed->Enable();
        Slider_WarmupFrames->Enable();
        TextCtrl_WarmupFrames->Enable();
    }
}
