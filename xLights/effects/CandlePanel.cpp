#include "CandlePanel.h"
#include "../../include/padlock16x16-blue.xpm"
#include "EffectPanelUtils.h"
#include "CandleEffect.h"

//(*InternalHeaders(CandlePanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/bitmap.h>
#include <wx/slider.h>
#include <wx/settings.h>
#include <wx/bmpbuttn.h>
#include <wx/intl.h>
#include <wx/image.h>
#include <wx/string.h>
//*)

//(*IdInit(CandlePanel)
const long CandlePanel::ID_STATICTEXT_Candle_FlameAgility = wxNewId();
const long CandlePanel::IDD_SLIDER_Candle_FlameAgility = wxNewId();
const long CandlePanel::ID_VALUECURVE_Candle_FlameAgility = wxNewId();
const long CandlePanel::ID_TEXTCTRL_Candle_FlameAgility = wxNewId();
const long CandlePanel::ID_BITMAPBUTTON_SLIDER_Candle_FlameAgility = wxNewId();
const long CandlePanel::ID_STATICTEXT_Candle_WindBaseline = wxNewId();
const long CandlePanel::IDD_SLIDER_Candle_WindBaseline = wxNewId();
const long CandlePanel::ID_VALUECURVE_Candle_WindBaseline = wxNewId();
const long CandlePanel::ID_TEXTCTRL_Candle_WindBaseline = wxNewId();
const long CandlePanel::ID_BITMAPBUTTON_SLIDER_Cande_WindBaseline = wxNewId();
const long CandlePanel::ID_STATICTEXT_Candle_WindVariability = wxNewId();
const long CandlePanel::IDD_SLIDER_Candle_WindVariability = wxNewId();
const long CandlePanel::ID_VALUECURVE_Candle_WindVariability = wxNewId();
const long CandlePanel::ID_TEXTCTRL_Candle_WindVariability = wxNewId();
const long CandlePanel::ID_BITMAPBUTTON_CHECKBOX_Cande_WindVariability = wxNewId();
const long CandlePanel::ID_STATICTEXT_Candle_WindCalmness = wxNewId();
const long CandlePanel::IDD_SLIDER_Candle_WindCalmness = wxNewId();
const long CandlePanel::ID_VALUECURVE_Candle_WindCalmness = wxNewId();
const long CandlePanel::ID_TEXTCTRL_Candle_WindCalmness = wxNewId();
const long CandlePanel::ID_BITMAPBUTTON_Candle_WindCalmness = wxNewId();
const long CandlePanel::ID_CHECKBOX_PerNode = wxNewId();
//*)

BEGIN_EVENT_TABLE(CandlePanel,wxPanel)
	//(*EventTable(CandlePanel)
	//*)
END_EVENT_TABLE()

CandlePanel::CandlePanel(wxWindow* parent)
{
	//(*Initialize(CandlePanel)
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer38;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer38 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer38->AddGrowableCol(1);
	StaticText83 = new wxStaticText(this, ID_STATICTEXT_Candle_FlameAgility, _("Flame Agility"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Candle_FlameAgility"));
	FlexGridSizer38->Add(StaticText83, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	Slider_Candle_FlameAgility = new BulkEditSlider(this, IDD_SLIDER_Candle_FlameAgility, 2, 1, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Candle_FlameAgility"));
	FlexGridSizer1->Add(Slider_Candle_FlameAgility, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Candle_FlameAgilityVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Candle_FlameAgility, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Candle_FlameAgility"));
	FlexGridSizer1->Add(BitmapButton_Candle_FlameAgilityVC, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer38->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 0);
	TextCtrl_Candle_FlameAgility = new BulkEditTextCtrl(this, ID_TEXTCTRL_Candle_FlameAgility, _("2"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Candle_FlameAgility"));
	TextCtrl_Candle_FlameAgility->SetMaxLength(2);
	FlexGridSizer38->Add(TextCtrl_Candle_FlameAgility, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Candle_FlameAgility = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Candle_FlameAgility, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Candle_FlameAgility"));
	BitmapButton_Candle_FlameAgility->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer38->Add(BitmapButton_Candle_FlameAgility, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText132 = new wxStaticText(this, ID_STATICTEXT_Candle_WindBaseline, _("Wind Baseline"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Candle_WindBaseline"));
	FlexGridSizer38->Add(StaticText132, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	Slider_Candle_WindBaseline = new BulkEditSlider(this, IDD_SLIDER_Candle_WindBaseline, 30, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Candle_WindBaseline"));
	FlexGridSizer2->Add(Slider_Candle_WindBaseline, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Candle_WindBaselineVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Candle_WindBaseline, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Candle_WindBaseline"));
	FlexGridSizer2->Add(BitmapButton_Candle_WindBaselineVC, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer38->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 0);
	TextCtrl_Candle_WindBaseline = new BulkEditTextCtrl(this, ID_TEXTCTRL_Candle_WindBaseline, _("30"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Candle_WindBaseline"));
	TextCtrl_Candle_WindBaseline->SetMaxLength(3);
	FlexGridSizer38->Add(TextCtrl_Candle_WindBaseline, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Cande_WindBaseline = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Cande_WindBaseline, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Cande_WindBaseline"));
	BitmapButton_Cande_WindBaseline->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer38->Add(BitmapButton_Cande_WindBaseline, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText30 = new wxStaticText(this, ID_STATICTEXT_Candle_WindVariability, _("Wind Variability"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Candle_WindVariability"));
	FlexGridSizer38->Add(StaticText30, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	Slider_Candle_WindVariability = new BulkEditSlider(this, IDD_SLIDER_Candle_WindVariability, 5, 0, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Candle_WindVariability"));
	FlexGridSizer3->Add(Slider_Candle_WindVariability, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Candle_WindVariabilityVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Candle_WindVariability, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Candle_WindVariability"));
	FlexGridSizer3->Add(BitmapButton_Candle_WindVariabilityVC, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer38->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 0);
	TextCtrl_Candle_WindVariability = new BulkEditTextCtrl(this, ID_TEXTCTRL_Candle_WindVariability, _("5"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Candle_WindVariability"));
	TextCtrl_Candle_WindVariability->SetMaxLength(2);
	FlexGridSizer38->Add(TextCtrl_Candle_WindVariability, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Cande_WinfVariability = new wxBitmapButton(this, ID_BITMAPBUTTON_CHECKBOX_Cande_WindVariability, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Cande_WindVariability"));
	BitmapButton_Cande_WinfVariability->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer38->Add(BitmapButton_Cande_WinfVariability, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT_Candle_WindCalmness, _("Wind Calmness"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Candle_WindCalmness"));
	FlexGridSizer38->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer4->AddGrowableCol(0);
	Slider_Candle_WindCalmness = new BulkEditSlider(this, IDD_SLIDER_Candle_WindCalmness, 2, 0, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Candle_WindCalmness"));
	FlexGridSizer4->Add(Slider_Candle_WindCalmness, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Candle_WindCalmnessVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Candle_WindCalmness, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Candle_WindCalmness"));
	FlexGridSizer4->Add(BitmapButton_Candle_WindCalmnessVC, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer38->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 0);
	TextCtrl_Candle_WindCalmness = new BulkEditTextCtrl(this, ID_TEXTCTRL_Candle_WindCalmness, _("2"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Candle_WindCalmness"));
	TextCtrl_Candle_WindCalmness->SetMaxLength(2);
	FlexGridSizer38->Add(TextCtrl_Candle_WindCalmness, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Candle_WindCalmness = new wxBitmapButton(this, ID_BITMAPBUTTON_Candle_WindCalmness, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Candle_WindCalmness"));
	BitmapButton_Candle_WindCalmness->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer38->Add(BitmapButton_Candle_WindCalmness, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer38->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_PerNode = new BulkEditCheckBox(this, ID_CHECKBOX_PerNode, _("Per node"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_PerNode"));
	CheckBox_PerNode->SetValue(false);
	FlexGridSizer38->Add(CheckBox_PerNode, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer38->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer38->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer38);
	FlexGridSizer38->Fit(this);
	FlexGridSizer38->SetSizeHints(this);

	Connect(ID_VALUECURVE_Candle_FlameAgility,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CandlePanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Candle_FlameAgility,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CandlePanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Candle_WindBaseline,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CandlePanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Cande_WindBaseline,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CandlePanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Candle_WindVariability,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CandlePanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Cande_WindVariability,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CandlePanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Candle_WindCalmness,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CandlePanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_Candle_WindCalmness,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CandlePanel::OnLockButtonClick);
	//*)

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&CandlePanel::OnVCChanged, 0, this);

    BitmapButton_Candle_FlameAgilityVC->GetValue()->SetLimits(CANDLE_AGILITY_MIN, CANDLE_AGILITY_MAX);
    BitmapButton_Candle_WindBaselineVC->GetValue()->SetLimits(CANDLE_WINDBASELINE_MIN, CANDLE_WINDBASELINE_MAX);
    BitmapButton_Candle_WindVariabilityVC->GetValue()->SetLimits(CANDLE_WINDVARIABILITY_MIN, CANDLE_WINDVARIABILITY_MAX);
    BitmapButton_Candle_WindCalmnessVC->GetValue()->SetLimits(CANDLE_WINDCALMNESS_MIN, CANDLE_WINDCALMNESS_MAX);

    SetName("ID_PANEL_Candle");
    ValidateWindow();
}

CandlePanel::~CandlePanel()
{
	//(*Destroy(CandlePanel)
	//*)
}

PANEL_EVENT_HANDLERS(CandlePanel)
void CandlePanel::OnCheckBox_Candle_GrowWithMusicClick(wxCommandEvent& event)
{
    ValidateWindow();
}

void CandlePanel::ValidateWindow()
{
}

