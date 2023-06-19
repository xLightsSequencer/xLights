/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "LightningPanel.h"
#include "EffectPanelUtils.h"
#include "LightningEffect.h"

//(*InternalHeaders(LightningPanel)
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

//(*IdInit(LightningPanel)
const long LightningPanel::ID_STATICTEXT_Number_Bolts = wxNewId();
const long LightningPanel::ID_SLIDER_Number_Bolts = wxNewId();
const long LightningPanel::ID_VALUECURVE_Number_Bolts = wxNewId();
const long LightningPanel::IDD_TEXTCTRL_Number_Bolts = wxNewId();
const long LightningPanel::ID_BITMAPBUTTON_SLIDER_Number_Bolts = wxNewId();
const long LightningPanel::ID_STATICTEXT_Number_Segments = wxNewId();
const long LightningPanel::ID_SLIDER_Number_Segments = wxNewId();
const long LightningPanel::ID_VALUECURVE_Number_Segments = wxNewId();
const long LightningPanel::IDD_TEXTCTRL_Number_Segments = wxNewId();
const long LightningPanel::ID_BITMAPBUTTON_SLIDER_Number_Segments = wxNewId();
const long LightningPanel::ID_STATICTEXT_ForkedLightning = wxNewId();
const long LightningPanel::ID_CHECKBOX_ForkedLightning = wxNewId();
const long LightningPanel::ID_BITMAPBUTTON_CHECKBOX_ForkedLightning = wxNewId();
const long LightningPanel::ID_STATICTEXT_Lightning_TopX = wxNewId();
const long LightningPanel::ID_SLIDER_Lightning_TopX = wxNewId();
const long LightningPanel::ID_VALUECURVE_Lightning_TopX = wxNewId();
const long LightningPanel::IDD_TEXTCTRL_Lightning_TopX = wxNewId();
const long LightningPanel::ID_BITMAPBUTTON_SLIDER_Lightning_TopX = wxNewId();
const long LightningPanel::ID_STATICTEXT_Lightning_TopY = wxNewId();
const long LightningPanel::ID_SLIDER_Lightning_TopY = wxNewId();
const long LightningPanel::ID_VALUECURVE_Lightning_TopY = wxNewId();
const long LightningPanel::IDD_TEXTCTRL_Lightning_TopY = wxNewId();
const long LightningPanel::ID_BITMAPBUTTON_SLIDER_Lightning_TopY = wxNewId();
const long LightningPanel::ID_STATICTEXT_Lightning_BOTX = wxNewId();
const long LightningPanel::ID_SLIDER_Lightning_BOTX = wxNewId();
const long LightningPanel::IDD_TEXTCTRL_Lightning_BOTX = wxNewId();
const long LightningPanel::ID_BITMAPBUTTON_SLIDER_Lightning_BOTX = wxNewId();
const long LightningPanel::ID_STATICTEXT_Lightning_WIDTH = wxNewId();
const long LightningPanel::ID_SLIDER_Lightning_WIDTH = wxNewId();
const long LightningPanel::IDD_TEXTCTRL_Lightning_WIDTH = wxNewId();
const long LightningPanel::ID_BITMAPBUTTON_SLIDER_Lightning_WIDTH = wxNewId();
const long LightningPanel::ID_STATICTEXT_Lightning_Direction = wxNewId();
const long LightningPanel::ID_CHOICE_Lightning_Direction = wxNewId();
//*)

BEGIN_EVENT_TABLE(LightningPanel,wxPanel)
	//(*EventTable(LightningPanel)
	//*)
END_EVENT_TABLE()

LightningPanel::LightningPanel(wxWindow* parent) : xlEffectPanel(parent)
{
	//(*Initialize(LightningPanel)
	BulkEditTextCtrl* TextCtrl_Number_Bolts;
	BulkEditTextCtrl* TextCtrl_Number_Segments;
	wxFlexGridSizer* FlexGridSizer140;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer4;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer140 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer140->AddGrowableCol(1);
	StaticText203 = new wxStaticText(this, ID_STATICTEXT_Number_Bolts, _("Number of Segments"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Number_Bolts"));
	FlexGridSizer140->Add(StaticText203, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	Slider_Number_Bolts = new BulkEditSlider(this, ID_SLIDER_Number_Bolts, 10, 1, 50, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Number_Bolts"));
	FlexGridSizer1->Add(Slider_Number_Bolts, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Number_BoltsVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Number_Bolts, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Number_Bolts"));
	FlexGridSizer1->Add(BitmapButton_Number_BoltsVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer140->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 0);
	TextCtrl_Number_Bolts = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Number_Bolts, _("10"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Number_Bolts"));
	TextCtrl_Number_Bolts->SetMaxLength(3);
	FlexGridSizer140->Add(TextCtrl_Number_Bolts, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_NumberBolts = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Number_Bolts, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Number_Bolts"));
	BitmapButton_NumberBolts->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer140->Add(BitmapButton_NumberBolts, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	StaticText205 = new wxStaticText(this, ID_STATICTEXT_Number_Segments, _("Width of Bolt"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Number_Segments"));
	FlexGridSizer140->Add(StaticText205, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	Slider_Number_Segments = new BulkEditSlider(this, ID_SLIDER_Number_Segments, 5, 1, 20, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Number_Segments"));
	FlexGridSizer2->Add(Slider_Number_Segments, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Number_SegmentsVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Number_Segments, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Number_Segments"));
	FlexGridSizer2->Add(BitmapButton_Number_SegmentsVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer140->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 0);
	TextCtrl_Number_Segments = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Number_Segments, _("5"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Number_Segments"));
	TextCtrl_Number_Segments->SetMaxLength(4);
	FlexGridSizer140->Add(TextCtrl_Number_Segments, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_NumberSegments = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Number_Segments, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Number_Segments"));
	BitmapButton_NumberSegments->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer140->Add(BitmapButton_NumberSegments, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText206 = new wxStaticText(this, ID_STATICTEXT_ForkedLightning, _("Forked Lightning"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_ForkedLightning"));
	FlexGridSizer140->Add(StaticText206, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_ForkedLightning = new BulkEditCheckBox(this, ID_CHECKBOX_ForkedLightning, _("Yes"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_ForkedLightning"));
	CheckBox_ForkedLightning->SetValue(false);
	FlexGridSizer140->Add(CheckBox_ForkedLightning, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer140->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_ForkedLightning = new xlLockButton(this, ID_BITMAPBUTTON_CHECKBOX_ForkedLightning, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_ForkedLightning"));
	BitmapButton_ForkedLightning->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer140->Add(BitmapButton_ForkedLightning, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	StaticText207 = new wxStaticText(this, ID_STATICTEXT_Lightning_TopX, _("Top X Location"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Lightning_TopX"));
	FlexGridSizer140->Add(StaticText207, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	Slider_Lightning_TopX = new BulkEditSlider(this, ID_SLIDER_Lightning_TopX, 0, -50, 50, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Lightning_TopX"));
	FlexGridSizer3->Add(Slider_Lightning_TopX, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Lightning_TopXVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Lightning_TopX, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Lightning_TopX"));
	FlexGridSizer3->Add(BitmapButton_Lightning_TopXVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer140->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 0);
	TextCtrl86 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Lightning_TopX, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Lightning_TopX"));
	TextCtrl86->SetMaxLength(4);
	FlexGridSizer140->Add(TextCtrl86, 1, wxALL|wxEXPAND, 2);
	BitmapButton_LightningTopX = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Lightning_TopX, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Lightning_TopX"));
	BitmapButton_LightningTopX->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer140->Add(BitmapButton_LightningTopX, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText208 = new wxStaticText(this, ID_STATICTEXT_Lightning_TopY, _("Top Y Location"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Lightning_TopY"));
	FlexGridSizer140->Add(StaticText208, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer4->AddGrowableCol(0);
	Slider_Lightning_TopY = new BulkEditSlider(this, ID_SLIDER_Lightning_TopY, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Lightning_TopY"));
	FlexGridSizer4->Add(Slider_Lightning_TopY, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Lightning_TopYVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Lightning_TopY, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Lightning_TopY"));
	FlexGridSizer4->Add(BitmapButton_Lightning_TopYVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer140->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 0);
	TextCtrl88 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Lightning_TopY, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Lightning_TopY"));
	FlexGridSizer140->Add(TextCtrl88, 1, wxALL|wxEXPAND, 2);
	BitmapButton_LightningTopY = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Lightning_TopY, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Lightning_TopY"));
	BitmapButton_LightningTopY->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer140->Add(BitmapButton_LightningTopY, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText209 = new wxStaticText(this, ID_STATICTEXT_Lightning_BOTX, _("X Movement"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Lightning_BOTX"));
	FlexGridSizer140->Add(StaticText209, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Lightning_BOTX = new BulkEditSlider(this, ID_SLIDER_Lightning_BOTX, 0, -50, 50, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Lightning_BOTX"));
	FlexGridSizer140->Add(Slider_Lightning_BOTX, 1, wxALL|wxEXPAND, 2);
	TextCtrl89 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Lightning_BOTX, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Lightning_BOTX"));
	FlexGridSizer140->Add(TextCtrl89, 1, wxALL|wxEXPAND, 2);
	BitmapButton_LightningBOTX = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Lightning_BOTX, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Lightning_BOTX"));
	BitmapButton_LightningBOTX->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer140->Add(BitmapButton_LightningBOTX, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText210 = new wxStaticText(this, ID_STATICTEXT_Lightning_WIDTH, _("Width"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Lightning_BOTY"));
	FlexGridSizer140->Add(StaticText210, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Lightning_WIDTH = new BulkEditSlider(this, ID_SLIDER_Lightning_WIDTH, 1, 1, 7, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Lightning_WIDTH"));
	FlexGridSizer140->Add(Slider_Lightning_WIDTH, 1, wxALL|wxEXPAND, 2);
	TextCtrl90 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Lightning_WIDTH, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Lightning_WIDTH"));
	FlexGridSizer140->Add(TextCtrl90, 1, wxALL|wxEXPAND, 2);
	BitmapButton_LightningWIDTH = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Lightning_WIDTH, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Lightning_WIDTH"));
	BitmapButton_LightningWIDTH->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer140->Add(BitmapButton_LightningWIDTH, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT_Lightning_Direction, _("Direction"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Lightning_Direction"));
	FlexGridSizer140->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	CHOICE_Lightning_Direction = new BulkEditChoice(this, ID_CHOICE_Lightning_Direction, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Lightning_Direction"));
	CHOICE_Lightning_Direction->SetSelection( CHOICE_Lightning_Direction->Append(_("Up")) );
	CHOICE_Lightning_Direction->Append(_("Down"));
	FlexGridSizer140->Add(CHOICE_Lightning_Direction, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer140);

	Connect(ID_VALUECURVE_Number_Bolts,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LightningPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Number_Bolts,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LightningPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Number_Segments,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LightningPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Number_Segments,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LightningPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_ForkedLightning,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LightningPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Lightning_TopX,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LightningPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Lightning_TopX,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LightningPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Lightning_TopY,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LightningPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Lightning_TopY,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LightningPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Lightning_BOTX,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LightningPanel::OnLockButtonClick);
    Connect(ID_BITMAPBUTTON_SLIDER_Lightning_WIDTH, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&LightningPanel::OnLockButtonClick);
	//*)

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&LightningPanel::OnVCChanged, 0, this);
    Connect(wxID_ANY, EVT_VALIDATEWINDOW, (wxObjectEventFunction)&LightningPanel::OnValidateWindow, 0, this);

    BitmapButton_Lightning_TopXVC->GetValue()->SetLimits(LIGHTNING_TOPX_MIN, LIGHTNING_TOPX_MAX);
    BitmapButton_Lightning_TopYVC->GetValue()->SetLimits(LIGHTNING_TOPY_MIN, LIGHTNING_TOPY_MAX);
    BitmapButton_Number_BoltsVC->GetValue()->SetLimits(LIGHTNING_BOLTS_MIN, LIGHTNING_BOLTS_MAX);
    BitmapButton_Number_SegmentsVC->GetValue()->SetLimits(LIGHTNING_SEGMENTS_MIN, LIGHTNING_SEGMENTS_MAX);

    SetName("ID_PANEL_LIGHTNING");

	ValidateWindow();
}

LightningPanel::~LightningPanel()
{
	//(*Destroy(LightningPanel)
	//*)
}

void LightningPanel::ValidateWindow()
{
}
