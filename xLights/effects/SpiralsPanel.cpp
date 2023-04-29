/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "SpiralsPanel.h"
#include "EffectPanelUtils.h"
#include "SpiralsEffect.h"

//(*InternalHeaders(SpiralsPanel)
#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#include <wx/checkbox.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>
//*)

//(*IdInit(SpiralsPanel)
const long SpiralsPanel::ID_STATICTEXT_Spirals_Count = wxNewId();
const long SpiralsPanel::ID_SLIDER_Spirals_Count = wxNewId();
const long SpiralsPanel::ID_VALUECURVE_Spirals_Count = wxNewId();
const long SpiralsPanel::IDD_TEXTCTRL_Spirals_Count = wxNewId();
const long SpiralsPanel::ID_BITMAPBUTTON_SLIDER_Spirals_Count = wxNewId();
const long SpiralsPanel::ID_STATICTEXT_Spirals_Rotation = wxNewId();
const long SpiralsPanel::ID_SLIDER_Spirals_Rotation = wxNewId();
const long SpiralsPanel::ID_VALUECURVE_Spirals_Rotation = wxNewId();
const long SpiralsPanel::IDD_TEXTCTRL_Spirals_Rotation = wxNewId();
const long SpiralsPanel::ID_BITMAPBUTTON_SLIDER_Spirals_Rotation = wxNewId();
const long SpiralsPanel::ID_STATICTEXT_Spirals_Thickness = wxNewId();
const long SpiralsPanel::ID_SLIDER_Spirals_Thickness = wxNewId();
const long SpiralsPanel::ID_VALUECURVE_Spirals_Thickness = wxNewId();
const long SpiralsPanel::IDD_TEXTCTRL_Spirals_Thickness = wxNewId();
const long SpiralsPanel::ID_BITMAPBUTTON_SLIDER_Spirals_Thickness = wxNewId();
const long SpiralsPanel::ID_STATICTEXT_Spirals_Movement = wxNewId();
const long SpiralsPanel::IDD_SLIDER_Spirals_Movement = wxNewId();
const long SpiralsPanel::ID_VALUECURVE_Spirals_Movement = wxNewId();
const long SpiralsPanel::ID_TEXTCTRL_Spirals_Movement = wxNewId();
const long SpiralsPanel::ID_BITMAPBUTTON_SLIDER_Spirals_Movement = wxNewId();
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

SpiralsPanel::SpiralsPanel(wxWindow* parent) : xlEffectPanel(parent)
{
	//(*Initialize(SpiralsPanel)
	BulkEditTextCtrl* TextCtrl36;
	BulkEditTextCtrl* TextCtrl38;
	BulkEditTextCtrlF1* TextCtrl37;
	BulkEditTextCtrlF1* TextCtrl39;
	wxFlexGridSizer* FlexGridSizer129;
	wxFlexGridSizer* FlexGridSizer130;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer45;
	wxFlexGridSizer* FlexGridSizer4;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer45 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer45->AddGrowableCol(0);
	FlexGridSizer129 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer129->AddGrowableCol(1);
	StaticText34 = new wxStaticText(this, ID_STATICTEXT_Spirals_Count, _("Palette Rep"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Spirals_Count"));
	FlexGridSizer129->Add(StaticText34, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	Slider_Spirals_Count = new BulkEditSlider(this, ID_SLIDER_Spirals_Count, 1, 1, 5, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Spirals_Count"));
	FlexGridSizer1->Add(Slider_Spirals_Count, 1, wxALL|wxEXPAND, 2);
	BitmapButton_VCSpiralsCount = new BulkEditValueCurveButton(this, ID_VALUECURVE_Spirals_Count, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Spirals_Count"));
	FlexGridSizer1->Add(BitmapButton_VCSpiralsCount, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer129->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 2);
	TextCtrl36 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Spirals_Count, _("1"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Spirals_Count"));
	TextCtrl36->SetMaxLength(2);
	FlexGridSizer129->Add(TextCtrl36, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_SpiralsCount = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Spirals_Count, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Spirals_Count"));
	BitmapButton_SpiralsCount->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer129->Add(BitmapButton_SpiralsCount, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText36 = new wxStaticText(this, ID_STATICTEXT_Spirals_Rotation, _("Spiral Wraps"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Spirals_Rotation"));
	FlexGridSizer129->Add(StaticText36, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	Slider_Spirals_Rotation = new BulkEditSliderF1(this, ID_SLIDER_Spirals_Rotation, 20, -300, 300, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Spirals_Rotation"));
	Slider_Spirals_Rotation->SetToolTip(_("Number of times a band wraps around the matrix"));
	FlexGridSizer2->Add(Slider_Spirals_Rotation, 1, wxALL|wxEXPAND, 2);
	BitmapButton_VCSpirals_Rotation = new BulkEditValueCurveButton(this, ID_VALUECURVE_Spirals_Rotation, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Spirals_Rotation"));
	FlexGridSizer2->Add(BitmapButton_VCSpirals_Rotation, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer129->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 2);
	TextCtrl37 = new BulkEditTextCtrlF1(this, IDD_TEXTCTRL_Spirals_Rotation, _("2.0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Spirals_Rotation"));
	TextCtrl37->SetMaxLength(5);
	FlexGridSizer129->Add(TextCtrl37, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_SpiralsRotation = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Spirals_Rotation, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Spirals_Rotation"));
	BitmapButton_SpiralsRotation->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer129->Add(BitmapButton_SpiralsRotation, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText38 = new wxStaticText(this, ID_STATICTEXT_Spirals_Thickness, _("Thickness"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Spirals_Thickness"));
	FlexGridSizer129->Add(StaticText38, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	Slider_Spirals_Thickness = new BulkEditSlider(this, ID_SLIDER_Spirals_Thickness, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Spirals_Thickness"));
	FlexGridSizer3->Add(Slider_Spirals_Thickness, 1, wxALL|wxEXPAND, 2);
	BitmapButton_VCSpirals_Thickness = new BulkEditValueCurveButton(this, ID_VALUECURVE_Spirals_Thickness, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Spirals_Thickness"));
	FlexGridSizer3->Add(BitmapButton_VCSpirals_Thickness, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer129->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 2);
	TextCtrl38 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Spirals_Thickness, _("50"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Spirals_Thickness"));
	TextCtrl38->SetMaxLength(3);
	FlexGridSizer129->Add(TextCtrl38, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_SpiralsThickness = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Spirals_Thickness, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Spirals_Thickness"));
	BitmapButton_SpiralsThickness->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer129->Add(BitmapButton_SpiralsThickness, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText40 = new wxStaticText(this, ID_STATICTEXT_Spirals_Movement, _("Movement"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Spirals_Movement"));
	FlexGridSizer129->Add(StaticText40, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer4->AddGrowableCol(0);
	Slider_Spirals_Movement = new BulkEditSliderF1(this, IDD_SLIDER_Spirals_Movement, 10, -200, 200, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Spirals_Movement"));
	FlexGridSizer4->Add(Slider_Spirals_Movement, 1, wxALL|wxEXPAND, 5);
	BitmapButton_VCSpirals_Movement = new BulkEditValueCurveButton(this, ID_VALUECURVE_Spirals_Movement, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Spirals_Movement"));
	FlexGridSizer4->Add(BitmapButton_VCSpirals_Movement, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer129->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 2);
	TextCtrl39 = new BulkEditTextCtrlF1(this, ID_TEXTCTRL_Spirals_Movement, _("1.0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Spirals_Movement"));
	TextCtrl39->SetMaxLength(5);
	FlexGridSizer129->Add(TextCtrl39, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_SpiralsDirection = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Spirals_Movement, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Spirals_Movement"));
	BitmapButton_SpiralsDirection->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer129->Add(BitmapButton_SpiralsDirection, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer45->Add(FlexGridSizer129, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer130 = new wxFlexGridSizer(0, 5, 0, 0);
	CheckBox_Spirals_Blend = new BulkEditCheckBox(this, ID_CHECKBOX_Spirals_Blend, _("Blend"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Spirals_Blend"));
	CheckBox_Spirals_Blend->SetValue(false);
	FlexGridSizer130->Add(CheckBox_Spirals_Blend, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_SpiralsBlend = new xlLockButton(this, ID_BITMAPBUTTON_CHECKBOX_Spirals_Blend, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Spirals_Blend"));
	BitmapButton_SpiralsBlend->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer130->Add(BitmapButton_SpiralsBlend, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	wxSize __SpacerSize_1 = wxDLG_UNIT(this,wxSize(20,-1));
	FlexGridSizer130->Add(__SpacerSize_1.GetWidth(),__SpacerSize_1.GetHeight(),1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Spirals_3D = new BulkEditCheckBox(this, ID_CHECKBOX_Spirals_3D, _("3D"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Spirals_3D"));
	CheckBox_Spirals_3D->SetValue(false);
	FlexGridSizer130->Add(CheckBox_Spirals_3D, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Spirals3D = new xlLockButton(this, ID_BITMAPBUTTON_CHECKBOX_Spirals_3D, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Spirals_3D"));
	BitmapButton_Spirals3D->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer130->Add(BitmapButton_Spirals3D, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	CheckBox_Spirals_Grow = new BulkEditCheckBox(this, ID_CHECKBOX_Spirals_Grow, _("Grow"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Spirals_Grow"));
	CheckBox_Spirals_Grow->SetValue(false);
	FlexGridSizer130->Add(CheckBox_Spirals_Grow, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_SpiralsGrow = new xlLockButton(this, ID_BITMAPBUTTON_CHECKBOX_Spirals_Grow, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Spirals_Grow"));
	BitmapButton_SpiralsGrow->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer130->Add(BitmapButton_SpiralsGrow, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	wxSize __SpacerSize_2 = wxDLG_UNIT(this,wxSize(20,-1));
	FlexGridSizer130->Add(__SpacerSize_2.GetWidth(),__SpacerSize_2.GetHeight(),1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Spirlas_Shrink = new BulkEditCheckBox(this, ID_CHECKBOX_Spirals_Shrink, _("Shrink"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Spirals_Shrink"));
	CheckBox_Spirlas_Shrink->SetValue(false);
	FlexGridSizer130->Add(CheckBox_Spirlas_Shrink, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_SpiralsShrink = new xlLockButton(this, ID_BITMAPBUTTON_CHECKBOX_Spirals_Shrink, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Spirals_Shrink"));
	BitmapButton_SpiralsShrink->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer130->Add(BitmapButton_SpiralsShrink, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer45->Add(FlexGridSizer130, 1, wxALL|wxEXPAND, 0);
	SetSizer(FlexGridSizer45);
	FlexGridSizer45->Fit(this);
	FlexGridSizer45->SetSizeHints(this);

	Connect(ID_VALUECURVE_Spirals_Count,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SpiralsPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Spirals_Count,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SpiralsPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Spirals_Rotation,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SpiralsPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Spirals_Rotation,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SpiralsPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Spirals_Thickness,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SpiralsPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Spirals_Thickness,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SpiralsPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Spirals_Movement,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SpiralsPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Spirals_Movement,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SpiralsPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Spirals_Blend,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SpiralsPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Spirals_3D,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SpiralsPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Spirals_Grow,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SpiralsPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Spirals_Shrink,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SpiralsPanel::OnLockButtonClick);
	//*)
    SetName("ID_PANEL_SPIRALS");

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&SpiralsPanel::OnVCChanged, 0, this);
    Connect(wxID_ANY, EVT_VALIDATEWINDOW, (wxObjectEventFunction)&SpiralsPanel::OnValidateWindow, 0, this);

    BitmapButton_VCSpiralsCount->GetValue()->SetLimits(SPIRALS_COUNT_MIN, SPIRALS_COUNT_MAX);
    BitmapButton_VCSpirals_Movement->GetValue()->SetLimits(SPIRALS_MOVEMENT_MIN, SPIRALS_MOVEMENT_MAX);
    BitmapButton_VCSpirals_Movement->GetValue()->SetDivisor(SPIRALS_MOVEMENT_DIVISOR);
    BitmapButton_VCSpirals_Rotation->GetValue()->SetLimits(SPIRALS_ROTATION_MIN, SPIRALS_ROTATION_MAX);
    BitmapButton_VCSpirals_Rotation->GetValue()->SetDivisor(SPIRALS_ROTATION_DIVISOR);
    BitmapButton_VCSpirals_Thickness->GetValue()->SetLimits(SPIRALS_THICKNESS_MIN, SPIRALS_THICKNESS_MAX);

    ValidateWindow();
}

SpiralsPanel::~SpiralsPanel()
{
	//(*Destroy(SpiralsPanel)
	//*)
}

void SpiralsPanel::ValidateWindow()
{
}
