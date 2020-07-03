/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "ShimmerPanel.h"
#include "EffectPanelUtils.h"
#include "ShimmerEffect.h"

//(*InternalHeaders(ShimmerPanel)
#include <wx/artprov.h>
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

//(*IdInit(ShimmerPanel)
const long ShimmerPanel::ID_STATICTEXT_Shimmer_Duty_Factor = wxNewId();
const long ShimmerPanel::ID_SLIDER_Shimmer_Duty_Factor = wxNewId();
const long ShimmerPanel::ID_VALUECURVE_Shimmer_Duty_Factor = wxNewId();
const long ShimmerPanel::IDD_TEXTCTRL_Shimmer_Duty_Factor = wxNewId();
const long ShimmerPanel::ID_BITMAPBUTTON_SLIDER_Shimmer_Duty_Factor = wxNewId();
const long ShimmerPanel::ID_STATICTEXT_Shimmer_Cycles = wxNewId();
const long ShimmerPanel::IDD_SLIDER_Shimmer_Cycles = wxNewId();
const long ShimmerPanel::ID_VALUECURVE_Shimmer_Cycles = wxNewId();
const long ShimmerPanel::ID_TEXTCTRL_Shimmer_Cycles = wxNewId();
const long ShimmerPanel::ID_BITMAPBUTTON_SLIDER_Shimmer_Cycles = wxNewId();
const long ShimmerPanel::ID_STATICTEXT69 = wxNewId();
const long ShimmerPanel::ID_CHECKBOX_Shimmer_Use_All_Colors = wxNewId();
const long ShimmerPanel::ID_BITMAPBUTTON_CHECKBOX_Shimmer_Use_All_Colors = wxNewId();
const long ShimmerPanel::ID_CHECKBOX_PRE_2017_7 = wxNewId();
//*)

BEGIN_EVENT_TABLE(ShimmerPanel,wxPanel)
	//(*EventTable(ShimmerPanel)
	//*)
END_EVENT_TABLE()

ShimmerPanel::ShimmerPanel(wxWindow* parent)
{
	//(*Initialize(ShimmerPanel)
	BulkEditTextCtrl* TextCtrl46;
	BulkEditTextCtrlF1* TextCtrl47;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer56;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer56 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer56->AddGrowableCol(1);
	StaticText66 = new wxStaticText(this, ID_STATICTEXT_Shimmer_Duty_Factor, _("Duty Factor"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Shimmer_Duty_Factor"));
	FlexGridSizer56->Add(StaticText66, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	Slider_Shimmer_Duty_Factor = new BulkEditSlider(this, ID_SLIDER_Shimmer_Duty_Factor, 50, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Shimmer_Duty_Factor"));
	FlexGridSizer1->Add(Slider_Shimmer_Duty_Factor, 1, wxALL|wxEXPAND, 5);
	BitmapButton_Shimmer_Duty_FactorVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Shimmer_Duty_Factor, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Shimmer_Duty_Factor"));
	FlexGridSizer1->Add(BitmapButton_Shimmer_Duty_FactorVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer56->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 2);
	TextCtrl46 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Shimmer_Duty_Factor, _("50"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Shimmer_Duty_Factor"));
	TextCtrl46->SetMaxLength(3);
	FlexGridSizer56->Add(TextCtrl46, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Shimmer_Duty_Factor = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Shimmer_Duty_Factor, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Shimmer_Duty_Factor"));
	BitmapButton_Shimmer_Duty_Factor->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer56->Add(BitmapButton_Shimmer_Duty_Factor, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	StaticText65 = new wxStaticText(this, ID_STATICTEXT_Shimmer_Cycles, _("Cycle Count"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Shimmer_Cycles"));
	FlexGridSizer56->Add(StaticText65, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	Slider_Shimmer_Cycles = new BulkEditSliderF1(this, IDD_SLIDER_Shimmer_Cycles, 10, 0, 6000, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Shimmer_Cycles"));
	FlexGridSizer2->Add(Slider_Shimmer_Cycles, 1, wxALL|wxEXPAND, 5);
	BitmapButton_Shimmer_CyclesVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Shimmer_Cycles, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Shimmer_Cycles"));
	FlexGridSizer2->Add(BitmapButton_Shimmer_CyclesVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer56->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 2);
	TextCtrl47 = new BulkEditTextCtrlF1(this, ID_TEXTCTRL_Shimmer_Cycles, _("1.0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Shimmer_Cycles"));
	TextCtrl47->SetMaxLength(5);
	FlexGridSizer56->Add(TextCtrl47, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_ShimmerCycles = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Shimmer_Cycles, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Shimmer_Cycles"));
	BitmapButton_ShimmerCycles->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer56->Add(BitmapButton_ShimmerCycles, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText67 = new wxStaticText(this, ID_STATICTEXT69, _("Use All Colors"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT69"));
	FlexGridSizer56->Add(StaticText67, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Shimmer_Use_All_Colors = new BulkEditCheckBox(this, ID_CHECKBOX_Shimmer_Use_All_Colors, _("Yes"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Shimmer_Use_All_Colors"));
	CheckBox_Shimmer_Use_All_Colors->SetValue(false);
	FlexGridSizer56->Add(CheckBox_Shimmer_Use_All_Colors, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer56->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Shimmer_Use_All_Colors = new xlLockButton(this, ID_BITMAPBUTTON_CHECKBOX_Shimmer_Use_All_Colors, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Shimmer_Use_All_Colors"));
	BitmapButton_Shimmer_Use_All_Colors->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer56->Add(BitmapButton_Shimmer_Use_All_Colors, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer56->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_PRE_2017_7 = new BulkEditCheckBox(this, ID_CHECKBOX_PRE_2017_7, _("Pre v2017.7 Shimmer"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_PRE_2017_7"));
	CheckBox_PRE_2017_7->SetValue(false);
	FlexGridSizer56->Add(CheckBox_PRE_2017_7, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer56->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer56->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer56);
	FlexGridSizer56->Fit(this);
	FlexGridSizer56->SetSizeHints(this);

	Connect(ID_VALUECURVE_Shimmer_Duty_Factor,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ShimmerPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Shimmer_Duty_Factor,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ShimmerPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Shimmer_Cycles,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ShimmerPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Shimmer_Cycles,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ShimmerPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Shimmer_Use_All_Colors,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ShimmerPanel::OnLockButtonClick);
	//*)

    SetName("ID_PANEL_SHIMMER");

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&ShimmerPanel::OnVCChanged, nullptr, this);

    BitmapButton_Shimmer_CyclesVC->GetValue()->SetLimits(SHIMMER_CYCLES_MIN, SHIMMER_CYCLES_MAX);
    BitmapButton_Shimmer_CyclesVC->GetValue()->SetDivisor(10);
    BitmapButton_Shimmer_Duty_FactorVC->GetValue()->SetLimits(SHIMMER_DUTYFACTOR_MIN, SHIMMER_DUTYFACTOR_MAX);
}

ShimmerPanel::~ShimmerPanel()
{
	//(*Destroy(ShimmerPanel)
	//*)
}

PANEL_EVENT_HANDLERS(ShimmerPanel)
