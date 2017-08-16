#include "SpiralsPanel.h"
#include "../../include/padlock16x16-blue.xpm"
#include "EffectPanelUtils.h"
#include "SpiralsEffect.h"

//(*InternalHeaders(SpiralsPanel)
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

//(*IdInit(SpiralsPanel)
const long SpiralsPanel::ID_STATICTEXT38 = wxNewId();
const long SpiralsPanel::ID_SLIDER_Spirals_Count = wxNewId();
const long SpiralsPanel::ID_VALUECURVE_Spirals_Count = wxNewId();
const long SpiralsPanel::IDD_TEXTCTRL_Spirals_Count = wxNewId();
const long SpiralsPanel::ID_BITMAPBUTTON_SLIDER_Spirals_Count = wxNewId();
const long SpiralsPanel::ID_SLIDER_Spirals_Rotation = wxNewId();
const long SpiralsPanel::ID_VALUECURVE_Spirals_Rotation = wxNewId();
const long SpiralsPanel::IDD_TEXTCTRL_Spirals_Rotation = wxNewId();
const long SpiralsPanel::ID_BITMAPBUTTON_SLIDER_Spirals_Rotation = wxNewId();
const long SpiralsPanel::ID_STATICTEXT42 = wxNewId();
const long SpiralsPanel::ID_SLIDER_Spirals_Thickness = wxNewId();
const long SpiralsPanel::ID_VALUECURVE_Spirals_Thickness = wxNewId();
const long SpiralsPanel::IDD_TEXTCTRL_Spirals_Thickness = wxNewId();
const long SpiralsPanel::ID_BITMAPBUTTON_SLIDER_Spirals_Thickness = wxNewId();
const long SpiralsPanel::ID_STATICTEXT44 = wxNewId();
const long SpiralsPanel::IDD_SLIDER_Spirals_Movement = wxNewId();
const long SpiralsPanel::ID_VALUECURVE_Spirals_Movement = wxNewId();
const long SpiralsPanel::ID_TEXTCTRL_Spirals_Movement = wxNewId();
const long SpiralsPanel::ID_BITMAPBUTTON_SLIDER_Spirals_Direction = wxNewId();
const long SpiralsPanel::ID_CHECKBOX_Spirals_Blend = wxNewId();
const long SpiralsPanel::ID_BITMAPBUTTON_CHECKBOX_Spirals_Blend = wxNewId();
const long SpiralsPanel::ID_CHECKBOX_Spirals_3D = wxNewId();
const long SpiralsPanel::ID_BITMAPBUTTON_CHECKBOX_Spirals_3D = wxNewId();
const long SpiralsPanel::ID_CHECKBOX_Spirals_Grow = wxNewId();
const long SpiralsPanel::ID_BITMAPBUTTON_CHECKBOX_Spirals_Grow = wxNewId();
const long SpiralsPanel::ID_CHECKBOX_Spirals_Shrink = wxNewId();
const long SpiralsPanel::ID_BITMAPBUTTON_CHECKBOX_Spirals_Shrink = wxNewId();
//*)

BEGIN_EVENT_TABLE(SpiralsPanel,wxPanel)
	//(*EventTable(SpiralsPanel)
	//*)
END_EVENT_TABLE()

SpiralsPanel::SpiralsPanel(wxWindow* parent)
{
	//(*Initialize(SpiralsPanel)
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer45;
	wxFlexGridSizer* FlexGridSizer3;
	wxTextCtrl* TextCtrl37;
	wxFlexGridSizer* FlexGridSizer130;
	wxTextCtrl* TextCtrl39;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer129;
	wxFlexGridSizer* FlexGridSizer1;
	wxStaticText* StaticText36;
	wxTextCtrl* TextCtrl38;
	wxTextCtrl* TextCtrl36;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer45 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer45->AddGrowableCol(0);
	FlexGridSizer129 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer129->AddGrowableCol(1);
	StaticText34 = new wxStaticText(this, ID_STATICTEXT38, _("Palette Rep"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT38"));
	FlexGridSizer129->Add(StaticText34, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	Slider_Spirals_Count = new wxSlider(this, ID_SLIDER_Spirals_Count, 1, 1, 5, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Spirals_Count"));
	FlexGridSizer1->Add(Slider_Spirals_Count, 1, wxALL|wxEXPAND, 2);
	BitmapButton_VCSpiralsCount = new ValueCurveButton(this, ID_VALUECURVE_Spirals_Count, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Spirals_Count"));
	FlexGridSizer1->Add(BitmapButton_VCSpiralsCount, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer129->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 2);
	TextCtrl36 = new wxTextCtrl(this, IDD_TEXTCTRL_Spirals_Count, _("1"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Spirals_Count"));
	TextCtrl36->SetMaxLength(2);
	FlexGridSizer129->Add(TextCtrl36, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_SpiralsCount = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Spirals_Count, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Spirals_Count"));
	BitmapButton_SpiralsCount->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer129->Add(BitmapButton_SpiralsCount, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText36 = new wxStaticText(this, wxID_ANY, _("Spiral Wraps"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer129->Add(StaticText36, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	Slider_Spirals_Rotation = new wxSlider(this, ID_SLIDER_Spirals_Rotation, 20, -300, 300, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Spirals_Rotation"));
	Slider_Spirals_Rotation->SetToolTip(_("Number of times a band wraps around the matrix"));
	FlexGridSizer2->Add(Slider_Spirals_Rotation, 1, wxALL|wxEXPAND, 2);
	BitmapButton_VCSpirals_Rotation = new ValueCurveButton(this, ID_VALUECURVE_Spirals_Rotation, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Spirals_Rotation"));
	FlexGridSizer2->Add(BitmapButton_VCSpirals_Rotation, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer129->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 2);
	TextCtrl37 = new wxTextCtrl(this, IDD_TEXTCTRL_Spirals_Rotation, _("2.0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Spirals_Rotation"));
	TextCtrl37->SetMaxLength(5);
	FlexGridSizer129->Add(TextCtrl37, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_SpiralsRotation = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Spirals_Rotation, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Spirals_Rotation"));
	BitmapButton_SpiralsRotation->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer129->Add(BitmapButton_SpiralsRotation, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText38 = new wxStaticText(this, ID_STATICTEXT42, _("Thickness"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT42"));
	FlexGridSizer129->Add(StaticText38, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	Slider_Spirals_Thickness = new wxSlider(this, ID_SLIDER_Spirals_Thickness, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Spirals_Thickness"));
	FlexGridSizer3->Add(Slider_Spirals_Thickness, 1, wxALL|wxEXPAND, 2);
	BitmapButton_VCSpirals_Thickness = new ValueCurveButton(this, ID_VALUECURVE_Spirals_Thickness, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Spirals_Thickness"));
	FlexGridSizer3->Add(BitmapButton_VCSpirals_Thickness, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer129->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 2);
	TextCtrl38 = new wxTextCtrl(this, IDD_TEXTCTRL_Spirals_Thickness, _("50"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Spirals_Thickness"));
	TextCtrl38->SetMaxLength(3);
	FlexGridSizer129->Add(TextCtrl38, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_SpiralsThickness = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Spirals_Thickness, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Spirals_Thickness"));
	BitmapButton_SpiralsThickness->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer129->Add(BitmapButton_SpiralsThickness, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText40 = new wxStaticText(this, ID_STATICTEXT44, _("Movement"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT44"));
	FlexGridSizer129->Add(StaticText40, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer4->AddGrowableCol(0);
	Slider_Spirals_Movement = new wxSlider(this, IDD_SLIDER_Spirals_Movement, 10, -200, 200, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Spirals_Movement"));
	FlexGridSizer4->Add(Slider_Spirals_Movement, 1, wxALL|wxEXPAND, 5);
	BitmapButton_VCSpirals_Movement = new ValueCurveButton(this, ID_VALUECURVE_Spirals_Movement, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Spirals_Movement"));
	FlexGridSizer4->Add(BitmapButton_VCSpirals_Movement, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer129->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 2);
	TextCtrl39 = new wxTextCtrl(this, ID_TEXTCTRL_Spirals_Movement, _("1.0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Spirals_Movement"));
	TextCtrl39->SetMaxLength(5);
	FlexGridSizer129->Add(TextCtrl39, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_SpiralsDirection = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Spirals_Direction, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Spirals_Direction"));
	BitmapButton_SpiralsDirection->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer129->Add(BitmapButton_SpiralsDirection, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer45->Add(FlexGridSizer129, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer130 = new wxFlexGridSizer(0, 5, 0, 0);
	CheckBox_Spirals_Blend = new wxCheckBox(this, ID_CHECKBOX_Spirals_Blend, _("Blend"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Spirals_Blend"));
	CheckBox_Spirals_Blend->SetValue(false);
	FlexGridSizer130->Add(CheckBox_Spirals_Blend, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_SpiralsBlend = new wxBitmapButton(this, ID_BITMAPBUTTON_CHECKBOX_Spirals_Blend, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Spirals_Blend"));
	BitmapButton_SpiralsBlend->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer130->Add(BitmapButton_SpiralsBlend, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	wxSize __SpacerSize_1 = wxDLG_UNIT(this,wxSize(20,-1));
	FlexGridSizer130->Add(__SpacerSize_1.GetWidth(),__SpacerSize_1.GetHeight(),1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Spirals_3D = new wxCheckBox(this, ID_CHECKBOX_Spirals_3D, _("3D"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Spirals_3D"));
	CheckBox_Spirals_3D->SetValue(false);
	FlexGridSizer130->Add(CheckBox_Spirals_3D, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Spirals3D = new wxBitmapButton(this, ID_BITMAPBUTTON_CHECKBOX_Spirals_3D, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Spirals_3D"));
	BitmapButton_Spirals3D->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer130->Add(BitmapButton_Spirals3D, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	CheckBox_Spirals_Grow = new wxCheckBox(this, ID_CHECKBOX_Spirals_Grow, _("Grow"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Spirals_Grow"));
	CheckBox_Spirals_Grow->SetValue(false);
	FlexGridSizer130->Add(CheckBox_Spirals_Grow, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_SpiralsGrow = new wxBitmapButton(this, ID_BITMAPBUTTON_CHECKBOX_Spirals_Grow, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Spirals_Grow"));
	BitmapButton_SpiralsGrow->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer130->Add(BitmapButton_SpiralsGrow, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	wxSize __SpacerSize_2 = wxDLG_UNIT(this,wxSize(20,-1));
	FlexGridSizer130->Add(__SpacerSize_2.GetWidth(),__SpacerSize_2.GetHeight(),1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Spirlas_Shrink = new wxCheckBox(this, ID_CHECKBOX_Spirals_Shrink, _("Shrink"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Spirals_Shrink"));
	CheckBox_Spirlas_Shrink->SetValue(false);
	FlexGridSizer130->Add(CheckBox_Spirlas_Shrink, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_SpiralsShrink = new wxBitmapButton(this, ID_BITMAPBUTTON_CHECKBOX_Spirals_Shrink, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Spirals_Shrink"));
	BitmapButton_SpiralsShrink->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer130->Add(BitmapButton_SpiralsShrink, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer45->Add(FlexGridSizer130, 1, wxALL|wxEXPAND, 0);
	SetSizer(FlexGridSizer45);
	FlexGridSizer45->Fit(this);
	FlexGridSizer45->SetSizeHints(this);

	Connect(ID_SLIDER_Spirals_Count,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&SpiralsPanel::UpdateLinkedTextCtrlVC);
	Connect(ID_VALUECURVE_Spirals_Count,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SpiralsPanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_Spirals_Count,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&SpiralsPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_SLIDER_Spirals_Count,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SpiralsPanel::OnLockButtonClick);
	Connect(ID_SLIDER_Spirals_Rotation,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&SpiralsPanel::UpdateLinkedTextCtrlFloatVC);
	Connect(ID_VALUECURVE_Spirals_Rotation,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SpiralsPanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_Spirals_Rotation,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&SpiralsPanel::UpdateLinkedSliderFloat);
	Connect(ID_BITMAPBUTTON_SLIDER_Spirals_Rotation,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SpiralsPanel::OnLockButtonClick);
	Connect(ID_SLIDER_Spirals_Thickness,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&SpiralsPanel::UpdateLinkedTextCtrlVC);
	Connect(ID_VALUECURVE_Spirals_Thickness,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SpiralsPanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_Spirals_Thickness,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&SpiralsPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_SLIDER_Spirals_Thickness,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SpiralsPanel::OnLockButtonClick);
	Connect(IDD_SLIDER_Spirals_Movement,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&SpiralsPanel::UpdateLinkedTextCtrlFloatVC);
	Connect(ID_VALUECURVE_Spirals_Movement,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SpiralsPanel::OnVCButtonClick);
	Connect(ID_TEXTCTRL_Spirals_Movement,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&SpiralsPanel::UpdateLinkedSliderFloat);
	Connect(ID_BITMAPBUTTON_SLIDER_Spirals_Direction,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SpiralsPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Spirals_Blend,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SpiralsPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Spirals_3D,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SpiralsPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Spirals_Grow,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SpiralsPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Spirals_Shrink,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SpiralsPanel::OnLockButtonClick);
	//*)
    SetName("ID_PANEL_SPIRALS");

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&SpiralsPanel::OnVCChanged, 0, this);

    BitmapButton_VCSpiralsCount->GetValue()->SetLimits(SPIRALS_COUNT_MIN, SPIRALS_COUNT_MAX);
    BitmapButton_VCSpirals_Movement->GetValue()->SetLimits(SPIRALS_MOVEMENT_MIN, SPIRALS_MOVEMENT_MAX);
    BitmapButton_VCSpirals_Movement->GetValue()->SetDivisor(10);
    BitmapButton_VCSpirals_Rotation->GetValue()->SetLimits(SPIRALS_ROTATION_MIN, SPIRALS_ROTATION_MAX);
    BitmapButton_VCSpirals_Rotation->GetValue()->SetDivisor(10);
    BitmapButton_VCSpirals_Thickness->GetValue()->SetLimits(SPIRALS_THICKNESS_MIN, SPIRALS_THICKNESS_MAX);
}

SpiralsPanel::~SpiralsPanel()
{
	//(*Destroy(SpiralsPanel)
	//*)
}

PANEL_EVENT_HANDLERS(SpiralsPanel)
