/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "OnPanel.h"
#include "OnEffect.h"

//(*InternalHeaders(OnPanel)
#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#include <wx/checkbox.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>
//*)

//(*IdInit(OnPanel)
const long OnPanel::ID_STATICTEXT_Eff_On_Start = wxNewId();
const long OnPanel::IDD_SLIDER_Eff_On_Start = wxNewId();
const long OnPanel::ID_TEXTCTRL_Eff_On_Start = wxNewId();
const long OnPanel::ID_STATICTEXT_Eff_On_End = wxNewId();
const long OnPanel::IDD_SLIDER_Eff_On_End = wxNewId();
const long OnPanel::ID_TEXTCTRL_Eff_On_End = wxNewId();
const long OnPanel::ID_STATICTEXT_On_Transparency = wxNewId();
const long OnPanel::IDD_SLIDER_On_Transparency = wxNewId();
const long OnPanel::ID_VALUECURVE_On_Transparency = wxNewId();
const long OnPanel::ID_TEXTCTRL_On_Transparency = wxNewId();
const long OnPanel::ID_STATICTEXT_On_Cycles = wxNewId();
const long OnPanel::IDD_SLIDER_On_Cycles = wxNewId();
const long OnPanel::ID_TEXTCTRL_On_Cycles = wxNewId();
const long OnPanel::ID_CHECKBOX_On_Shimmer = wxNewId();
//*)

BEGIN_EVENT_TABLE(OnPanel,wxPanel)
	//(*EventTable(OnPanel)
	//*)
END_EVENT_TABLE()

#include "EffectPanelUtils.h"

OnPanel::OnPanel(wxWindow* parent)
{
	//(*Initialize(OnPanel)
	wxFlexGridSizer* FlexGridSizer18;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer92;
	wxFlexGridSizer* FlexGridSizer93;
	wxFlexGridSizer* FlexGridSizer95;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer92 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer92->AddGrowableCol(0);
	FlexGridSizer93 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer93->AddGrowableCol(0);
	FlexGridSizer95 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer95->AddGrowableCol(1);
	StaticText113 = new wxStaticText(this, ID_STATICTEXT_Eff_On_Start, _("Start Intensity"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Eff_On_Start"));
	FlexGridSizer95->Add(StaticText113, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	SliderStart = new BulkEditSlider(this, IDD_SLIDER_Eff_On_Start, 100, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Eff_On_Start"));
	FlexGridSizer1->Add(SliderStart, 1, wxALL|wxEXPAND, 2);
	TextCtrlStart = new BulkEditTextCtrl(this, ID_TEXTCTRL_Eff_On_Start, _("100"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(30,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_Eff_On_Start"));
	FlexGridSizer1->Add(TextCtrlStart, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer95->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 0);
	StaticText114 = new wxStaticText(this, ID_STATICTEXT_Eff_On_End, _("End Intensity"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Eff_On_End"));
	FlexGridSizer95->Add(StaticText114, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	SliderEnd = new BulkEditSlider(this, IDD_SLIDER_Eff_On_End, 100, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Eff_On_End"));
	FlexGridSizer2->Add(SliderEnd, 1, wxALL|wxEXPAND, 2);
	TextCtrlEnd = new BulkEditTextCtrl(this, ID_TEXTCTRL_Eff_On_End, _("100"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(30,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_Eff_On_End"));
	FlexGridSizer2->Add(TextCtrlEnd, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer95->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 0);
	StaticText137 = new wxStaticText(this, ID_STATICTEXT_On_Transparency, _("Transparency"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_On_Transparency"));
	FlexGridSizer95->Add(StaticText137, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer18 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer18->AddGrowableCol(0);
	Slider_On_Transparency = new BulkEditSlider(this, IDD_SLIDER_On_Transparency, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_On_Transparency"));
	FlexGridSizer18->Add(Slider_On_Transparency, 1, wxALL|wxEXPAND, 2);
	BitmapButton_On_Transparency = new BulkEditValueCurveButton(this, ID_VALUECURVE_On_Transparency, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_On_Transparency"));
	FlexGridSizer18->Add(BitmapButton_On_Transparency, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	TextCtrlOnTransparency = new BulkEditTextCtrl(this, ID_TEXTCTRL_On_Transparency, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(30,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_On_Transparency"));
	TextCtrlOnTransparency->SetMaxLength(3);
	FlexGridSizer18->Add(TextCtrlOnTransparency, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer95->Add(FlexGridSizer18, 1, wxALL|wxEXPAND, 0);
	StaticText174 = new wxStaticText(this, ID_STATICTEXT_On_Cycles, _("Cycle Count"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_On_Cycles"));
	FlexGridSizer95->Add(StaticText174, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	SliderCycles = new BulkEditSliderF1(this, IDD_SLIDER_On_Cycles, 10, 0, 1000, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_On_Cycles"));
	FlexGridSizer3->Add(SliderCycles, 1, wxALL|wxEXPAND, 2);
	TextCtrlCycles = new BulkEditTextCtrlF1(this, ID_TEXTCTRL_On_Cycles, _("1.0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(30,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_On_Cycles"));
	TextCtrlCycles->SetMaxLength(5);
	FlexGridSizer3->Add(TextCtrlCycles, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer95->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer93->Add(FlexGridSizer95, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer92->Add(FlexGridSizer93, 1, wxALL|wxEXPAND, 2);
	CheckBoxShimmer = new BulkEditCheckBox(this, ID_CHECKBOX_On_Shimmer, _("Shimmer"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_On_Shimmer"));
	CheckBoxShimmer->SetValue(false);
	FlexGridSizer92->Add(CheckBoxShimmer, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer92);
	FlexGridSizer92->Fit(this);
	FlexGridSizer92->SetSizeHints(this);

	Connect(ID_VALUECURVE_On_Transparency,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OnPanel::OnVCButtonClick);
	//*)

    BitmapButton_On_Transparency->SetLimits(ON_TRANSPARENCY_MIN, ON_TRANSPARENCY_MAX);

    SetName("ID_PANEL_ON");
}

OnPanel::~OnPanel()
{
	//(*Destroy(OnPanel)
	//*)
}

PANEL_EVENT_HANDLERS(OnPanel)

