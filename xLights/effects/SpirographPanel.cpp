#include "SpirographPanel.h"
#include "../../include/padlock16x16-blue.xpm"
#include "EffectPanelUtils.h"
#include "SpirographEffect.h"

//(*InternalHeaders(SpirographPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/bitmap.h>
#include <wx/slider.h>
#include <wx/settings.h>
#include <wx/bmpbuttn.h>
#include <wx/intl.h>
#include <wx/image.h>
#include <wx/string.h>
//*)

//(*IdInit(SpirographPanel)
const long SpirographPanel::ID_STATICTEXT_Spirograph_Speed = wxNewId();
const long SpirographPanel::IDD_SLIDER_Spirograph_Speed = wxNewId();
const long SpirographPanel::ID_VALUECURVE_Spirograph_Speed = wxNewId();
const long SpirographPanel::ID_TEXTCTRL_Spirograph_Speed = wxNewId();
const long SpirographPanel::ID_BITMAPBUTTON32 = wxNewId();
const long SpirographPanel::ID_STATICTEXT_Spirograph_R = wxNewId();
const long SpirographPanel::ID_SLIDER_Spirograph_R = wxNewId();
const long SpirographPanel::ID_VALUECURVE_Spirograph_R = wxNewId();
const long SpirographPanel::IDD_TEXTCTRL_Spirograph_R = wxNewId();
const long SpirographPanel::ID_BITMAPBUTTON_SLIDER_Spirograph_R = wxNewId();
const long SpirographPanel::ID_STATICTEXT_Spirograph_r = wxNewId();
const long SpirographPanel::ID_SLIDER_Spirograph_r = wxNewId();
const long SpirographPanel::ID_VALUECURVE_Spirograph_r = wxNewId();
const long SpirographPanel::IDD_TEXTCTRL_Spirograph_r = wxNewId();
const long SpirographPanel::ID_BITMAPBUTTON_SLIDER_Spirograph_r = wxNewId();
const long SpirographPanel::ID_STATICTEXT_Spirograph_d = wxNewId();
const long SpirographPanel::ID_SLIDER_Spirograph_d = wxNewId();
const long SpirographPanel::ID_VALUECURVE_Spirograph_d = wxNewId();
const long SpirographPanel::IDD_TEXTCTRL_Spirograph_d = wxNewId();
const long SpirographPanel::ID_BITMAPBUTTON_SLIDER_Spirograph_d = wxNewId();
const long SpirographPanel::ID_STATICTEXT_Spirograph_Animate = wxNewId();
const long SpirographPanel::IDD_SLIDER_Spirograph_Animate = wxNewId();
const long SpirographPanel::ID_VALUECURVE_Spirograph_Animate = wxNewId();
const long SpirographPanel::ID_TEXTCTRL_Spirograph_Animate = wxNewId();
const long SpirographPanel::ID_BITMAPBUTTON_CHECKBOX_Spirograph_Animate = wxNewId();
const long SpirographPanel::ID_STATICTEXT_Spirograph_Length = wxNewId();
const long SpirographPanel::IDD_SLIDER_Spirograph_Length = wxNewId();
const long SpirographPanel::ID_VALUECURVE_Spirograph_Length = wxNewId();
const long SpirographPanel::ID_TEXTCTRL_Spirograph_Length = wxNewId();
const long SpirographPanel::ID_BITMAPBUTTON33 = wxNewId();
const long SpirographPanel::ID_STATICTEXT91 = wxNewId();
//*)

BEGIN_EVENT_TABLE(SpirographPanel,wxPanel)
	//(*EventTable(SpirographPanel)
	//*)
END_EVENT_TABLE()

SpirographPanel::SpirographPanel(wxWindow* parent)
{
	//(*Initialize(SpirographPanel)
	BulkEditTextCtrl* TextCtrl81;
	BulkEditTextCtrl* TextCtrl83;
	wxFlexGridSizer* FlexGridSizer132;
	BulkEditTextCtrl* TextCtrl82;
	wxFlexGridSizer* FlexGridSizer134;
	wxFlexGridSizer* FlexGridSizer133;
	BulkEditTextCtrl* TextCtrl22;
	BulkEditTextCtrl* TextCtrl21;
	wxFlexGridSizer* FlexGridSizer127;
	wxFlexGridSizer* FlexGridSizer125;
	BulkEditTextCtrl* TextCtrl23;
	wxFlexGridSizer* FlexGridSizer72;
	wxFlexGridSizer* FlexGridSizer126;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer72 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer72->AddGrowableCol(1);
	StaticText190 = new wxStaticText(this, ID_STATICTEXT_Spirograph_Speed, _("Speed"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Spirograph_Speed"));
	FlexGridSizer72->Add(StaticText190, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer132 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer132->AddGrowableCol(0);
	Slider_Spirograph_Speed = new BulkEditSlider(this, IDD_SLIDER_Spirograph_Speed, 10, 0, 50, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Spirograph_Speed"));
	FlexGridSizer132->Add(Slider_Spirograph_Speed, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Spirograph_SpeedrVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Spirograph_Speed, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Spirograph_Speed"));
	FlexGridSizer132->Add(BitmapButton_Spirograph_SpeedrVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl81 = new BulkEditTextCtrl(this, ID_TEXTCTRL_Spirograph_Speed, _("10"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Spirograph_Speed"));
	TextCtrl81->SetMaxLength(3);
	FlexGridSizer132->Add(TextCtrl81, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer72->Add(FlexGridSizer132, 1, wxALL|wxEXPAND, 2);
	BitmapButton14 = new wxBitmapButton(this, ID_BITMAPBUTTON32, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON32"));
	BitmapButton14->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer72->Add(BitmapButton14, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText87 = new wxStaticText(this, ID_STATICTEXT_Spirograph_R, _("R - Radius of outer circle"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Spirograph_R"));
	FlexGridSizer72->Add(StaticText87, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer125 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer125->AddGrowableCol(0);
	Slider_Spirograph_R = new BulkEditSlider(this, ID_SLIDER_Spirograph_R, 20, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Spirograph_R"));
	FlexGridSizer125->Add(Slider_Spirograph_R, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Spirograph_RVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Spirograph_R, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Spirograph_R"));
	FlexGridSizer125->Add(BitmapButton_Spirograph_RVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl21 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Spirograph_R, _("20"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Spirograph_R"));
	TextCtrl21->SetMaxLength(3);
	FlexGridSizer125->Add(TextCtrl21, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer72->Add(FlexGridSizer125, 1, wxALL|wxEXPAND, 2);
	BitmapButton_SpirographR = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Spirograph_R, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Spirograph_R"));
	BitmapButton_SpirographR->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer72->Add(BitmapButton_SpirographR, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText88 = new wxStaticText(this, ID_STATICTEXT_Spirograph_r, _("r - Radius of inner circle"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Spirograph_r"));
	FlexGridSizer72->Add(StaticText88, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer126 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer126->AddGrowableCol(0);
	Slider_Spirograph_r = new BulkEditSlider(this, ID_SLIDER_Spirograph_r, 10, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Spirograph_r"));
	FlexGridSizer126->Add(Slider_Spirograph_r, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Spirograph_rVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Spirograph_r, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Spirograph_r"));
	FlexGridSizer126->Add(BitmapButton_Spirograph_rVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl22 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Spirograph_r, _("10"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Spirograph_r"));
	TextCtrl22->SetMaxLength(3);
	FlexGridSizer126->Add(TextCtrl22, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer72->Add(FlexGridSizer126, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Spirographr = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Spirograph_r, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Spirograph_r"));
	BitmapButton_Spirographr->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer72->Add(BitmapButton_Spirographr, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText89 = new wxStaticText(this, ID_STATICTEXT_Spirograph_d, _("d - Distance"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Spirograph_d"));
	FlexGridSizer72->Add(StaticText89, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer127 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer127->AddGrowableCol(0);
	Slider_Spirograph_d = new BulkEditSlider(this, ID_SLIDER_Spirograph_d, 30, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Spirograph_d"));
	FlexGridSizer127->Add(Slider_Spirograph_d, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Spirograph_dVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Spirograph_d, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Spirograph_d"));
	FlexGridSizer127->Add(BitmapButton_Spirograph_dVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl23 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Spirograph_d, _("30"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Spirograph_d"));
	TextCtrl23->SetMaxLength(3);
	FlexGridSizer127->Add(TextCtrl23, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer72->Add(FlexGridSizer127, 1, wxALL|wxEXPAND, 2);
	BitmapButton_SpirographAnimate = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Spirograph_d, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Spirograph_d"));
	BitmapButton_SpirographAnimate->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer72->Add(BitmapButton_SpirographAnimate, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText92 = new wxStaticText(this, ID_STATICTEXT_Spirograph_Animate, _("a - Animation"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Spirograph_Animate"));
	FlexGridSizer72->Add(StaticText92, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer133 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer133->AddGrowableCol(0);
	Slider_Spirograph_Animate = new BulkEditSlider(this, IDD_SLIDER_Spirograph_Animate, 0, -50, 50, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Spirograph_Animate"));
	FlexGridSizer133->Add(Slider_Spirograph_Animate, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Spirograph_AnimateVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Spirograph_Animate, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Spirograph_Animate"));
	FlexGridSizer133->Add(BitmapButton_Spirograph_AnimateVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl82 = new BulkEditTextCtrl(this, ID_TEXTCTRL_Spirograph_Animate, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Spirograph_Animate"));
	TextCtrl82->SetMaxLength(3);
	FlexGridSizer133->Add(TextCtrl82, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer72->Add(FlexGridSizer133, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Spirographd = new wxBitmapButton(this, ID_BITMAPBUTTON_CHECKBOX_Spirograph_Animate, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Spirograph_Animate"));
	BitmapButton_Spirographd->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer72->Add(BitmapButton_Spirographd, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText191 = new wxStaticText(this, ID_STATICTEXT_Spirograph_Length, _("Length"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Spirograph_Length"));
	FlexGridSizer72->Add(StaticText191, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer134 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer134->AddGrowableCol(0);
	Slider_Spirograph_Length = new BulkEditSlider(this, IDD_SLIDER_Spirograph_Length, 20, 0, 50, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Spirograph_Length"));
	FlexGridSizer134->Add(Slider_Spirograph_Length, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Spirograph_LengthVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Spirograph_Length, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Spirograph_Length"));
	FlexGridSizer134->Add(BitmapButton_Spirograph_LengthVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl83 = new BulkEditTextCtrl(this, ID_TEXTCTRL_Spirograph_Length, _("20"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Spirograph_Length"));
	TextCtrl83->SetMaxLength(3);
	FlexGridSizer134->Add(TextCtrl83, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer72->Add(FlexGridSizer134, 1, wxALL|wxEXPAND, 2);
	BitmapButton15 = new wxBitmapButton(this, ID_BITMAPBUTTON33, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON33"));
	BitmapButton15->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer72->Add(BitmapButton15, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText90 = new wxStaticText(this, ID_STATICTEXT91, _("Note: r should be <= R"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT91"));
	FlexGridSizer72->Add(StaticText90, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	SetSizer(FlexGridSizer72);
	FlexGridSizer72->Fit(this);
	FlexGridSizer72->SetSizeHints(this);

	Connect(ID_VALUECURVE_Spirograph_Speed,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SpirographPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON32,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SpirographPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Spirograph_R,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SpirographPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Spirograph_R,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SpirographPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Spirograph_r,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SpirographPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Spirograph_r,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SpirographPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Spirograph_d,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SpirographPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Spirograph_d,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SpirographPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Spirograph_Animate,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SpirographPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Spirograph_Animate,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SpirographPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Spirograph_Length,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SpirographPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON33,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SpirographPanel::OnLockButtonClick);
	//*)
    SetName("ID_PANEL_SPIROGRAPH");

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&SpirographPanel::OnVCChanged, 0, this);

    BitmapButton_Spirograph_AnimateVC->GetValue()->SetLimits(SPIROGRAPH_ANIMATE_MIN, SPIROGRAPH_ANIMATE_MAX);
    BitmapButton_Spirograph_LengthVC->GetValue()->SetLimits(SPIROGRAPH_LENGTH_MIN, SPIROGRAPH_LENGTH_MAX);
    BitmapButton_Spirograph_RVC->GetValue()->SetLimits(SPIROGRAPH_R_MIN, SPIROGRAPH_R_MAX);
    BitmapButton_Spirograph_rVC->GetValue()->SetLimits(SPIROGRAPH_r_MIN, SPIROGRAPH_r_MAX);
    BitmapButton_Spirograph_SpeedrVC->GetValue()->SetLimits(SPIROGRAPH_SPEED_MIN, SPIROGRAPH_SPEED_MAX);
    BitmapButton_Spirograph_dVC->GetValue()->SetLimits(SPIROGRAPH_d_MIN, SPIROGRAPH_d_MAX);
}

SpirographPanel::~SpirographPanel()
{
	//(*Destroy(SpirographPanel)
	//*)
}

PANEL_EVENT_HANDLERS(SpirographPanel)
