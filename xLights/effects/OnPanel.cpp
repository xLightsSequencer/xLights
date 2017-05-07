#include "OnPanel.h"

//(*InternalHeaders(OnPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/bitmap.h>
#include <wx/slider.h>
#include <wx/bmpbuttn.h>
#include <wx/intl.h>
#include <wx/image.h>
#include <wx/string.h>
//*)

//(*IdInit(OnPanel)
const long OnPanel::IDD_SLIDER_Eff_On_Start = wxNewId();
const long OnPanel::ID_TEXTCTRL_Eff_On_Start = wxNewId();
const long OnPanel::IDD_SLIDER_Eff_On_End = wxNewId();
const long OnPanel::ID_TEXTCTRL_Eff_On_End = wxNewId();
const long OnPanel::IDD_SLIDER_On_Transparency = wxNewId();
const long OnPanel::ID_VALUECURVE_On_Transparency = wxNewId();
const long OnPanel::ID_TEXTCTRL_On_Transparency = wxNewId();
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
	wxStaticText* StaticText137;
	wxFlexGridSizer* FlexGridSizer95;
	wxStaticText* StaticText174;
	wxFlexGridSizer* FlexGridSizer3;
	wxStaticText* StaticText114;
	wxFlexGridSizer* FlexGridSizer2;
	wxStaticText* StaticText113;
	wxFlexGridSizer* FlexGridSizer18;
	wxFlexGridSizer* FlexGridSizer93;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer92;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer92 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer92->AddGrowableCol(0);
	FlexGridSizer93 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer93->AddGrowableCol(0);
	FlexGridSizer95 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer95->AddGrowableCol(1);
	StaticText113 = new wxStaticText(this, wxID_ANY, _("Start Intensity"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer95->Add(StaticText113, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	SliderStart = new wxSlider(this, IDD_SLIDER_Eff_On_Start, 100, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Eff_On_Start"));
	FlexGridSizer1->Add(SliderStart, 1, wxALL|wxEXPAND, 2);
	TextCtrlStart = new wxTextCtrl(this, ID_TEXTCTRL_Eff_On_Start, _("100"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(30,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_Eff_On_Start"));
	FlexGridSizer1->Add(TextCtrlStart, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer95->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 0);
	StaticText114 = new wxStaticText(this, wxID_ANY, _("End Intensity"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer95->Add(StaticText114, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	SliderEnd = new wxSlider(this, IDD_SLIDER_Eff_On_End, 100, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Eff_On_End"));
	FlexGridSizer2->Add(SliderEnd, 1, wxALL|wxEXPAND, 2);
	TextCtrlEnd = new wxTextCtrl(this, ID_TEXTCTRL_Eff_On_End, _("100"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(30,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_Eff_On_End"));
	FlexGridSizer2->Add(TextCtrlEnd, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer95->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 0);
	StaticText137 = new wxStaticText(this, wxID_ANY, _("Transparency"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer95->Add(StaticText137, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer18 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer18->AddGrowableCol(0);
	Slider_On_Transparency = new wxSlider(this, IDD_SLIDER_On_Transparency, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_On_Transparency"));
	FlexGridSizer18->Add(Slider_On_Transparency, 1, wxALL|wxEXPAND, 2);
	BitmapButton_On_Transparency = new ValueCurveButton(this, ID_VALUECURVE_On_Transparency, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_On_Transparency"));
	FlexGridSizer18->Add(BitmapButton_On_Transparency, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	TextCtrlOnTransparency = new wxTextCtrl(this, ID_TEXTCTRL_On_Transparency, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(30,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_On_Transparency"));
	TextCtrlOnTransparency->SetMaxLength(3);
	FlexGridSizer18->Add(TextCtrlOnTransparency, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer95->Add(FlexGridSizer18, 1, wxALL|wxEXPAND, 0);
	StaticText174 = new wxStaticText(this, wxID_ANY, _("Cycle Count"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer95->Add(StaticText174, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	SliderCycles = new wxSlider(this, IDD_SLIDER_On_Cycles, 10, 0, 1000, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_On_Cycles"));
	FlexGridSizer3->Add(SliderCycles, 1, wxALL|wxEXPAND, 2);
	TextCtrlCycles = new wxTextCtrl(this, ID_TEXTCTRL_On_Cycles, _("1.0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(30,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_On_Cycles"));
	TextCtrlCycles->SetMaxLength(5);
	FlexGridSizer3->Add(TextCtrlCycles, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer95->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer93->Add(FlexGridSizer95, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer92->Add(FlexGridSizer93, 1, wxALL|wxEXPAND, 2);
	CheckBoxShimmer = new wxCheckBox(this, ID_CHECKBOX_On_Shimmer, _("Shimmer"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_On_Shimmer"));
	CheckBoxShimmer->SetValue(false);
	FlexGridSizer92->Add(CheckBoxShimmer, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer92);
	FlexGridSizer92->Fit(this);
	FlexGridSizer92->SetSizeHints(this);

	Connect(IDD_SLIDER_Eff_On_Start,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&OnPanel::UpdateLinkedTextCtrl);
	Connect(ID_TEXTCTRL_Eff_On_Start,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&OnPanel::UpdateLinkedSlider);
	Connect(IDD_SLIDER_Eff_On_End,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&OnPanel::UpdateLinkedTextCtrl);
	Connect(ID_TEXTCTRL_Eff_On_End,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&OnPanel::UpdateLinkedSlider);
	Connect(IDD_SLIDER_On_Transparency,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&OnPanel::UpdateLinkedTextCtrlVC);
	Connect(ID_VALUECURVE_On_Transparency,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OnPanel::OnVCButtonClick);
	Connect(ID_TEXTCTRL_On_Transparency,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&OnPanel::UpdateLinkedSlider);
	Connect(IDD_SLIDER_On_Cycles,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&OnPanel::UpdateLinkedTextCtrlFloat);
	Connect(ID_TEXTCTRL_On_Cycles,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&OnPanel::UpdateLinkedSliderFloat);
	//*)

    BitmapButton_On_Transparency->SetLimits(0, 100);

    SetName("ID_PANEL_ON");
}

OnPanel::~OnPanel()
{
	//(*Destroy(OnPanel)
	//*)
}

PANEL_EVENT_HANDLERS(OnPanel)

