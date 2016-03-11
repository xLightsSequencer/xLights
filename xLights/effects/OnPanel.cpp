#include "OnPanel.h"

//(*InternalHeaders(OnPanel)
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/string.h>
#include <wx/slider.h>
#include <wx/intl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

//(*IdInit(OnPanel)
const long OnPanel::IDD_SLIDER_Eff_On_Start = wxNewId();
const long OnPanel::ID_TEXTCTRL_Eff_On_Start = wxNewId();
const long OnPanel::IDD_SLIDER_Eff_On_End = wxNewId();
const long OnPanel::ID_TEXTCTRL_Eff_On_End = wxNewId();
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
	wxFlexGridSizer* FlexGridSizer95;
	wxStaticText* StaticText113;
	wxFlexGridSizer* FlexGridSizer96;
	wxFlexGridSizer* FlexGridSizer92;
	wxStaticText* StaticText114;
	wxStaticText* StaticText174;
	wxFlexGridSizer* FlexGridSizer93;
	wxStaticText* StaticText73;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer92 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer92->AddGrowableCol(0);
	FlexGridSizer96 = new wxFlexGridSizer(0, 1, 0, 0);
	StaticText73 = new wxStaticText(this, wxID_ANY, _("This Effect turns every pixel on."), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer96->Add(StaticText73, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer92->Add(FlexGridSizer96, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer93 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer93->AddGrowableCol(0);
	FlexGridSizer95 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer95->AddGrowableCol(1);
	StaticText113 = new wxStaticText(this, wxID_ANY, _("Start Intensity"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer95->Add(StaticText113, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	SliderStart = new wxSlider(this, IDD_SLIDER_Eff_On_Start, 100, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Eff_On_Start"));
	FlexGridSizer95->Add(SliderStart, 1, wxALL|wxEXPAND, 2);
	TextCtrlStart = new wxTextCtrl(this, ID_TEXTCTRL_Eff_On_Start, _("100"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_Eff_On_Start"));
	FlexGridSizer95->Add(TextCtrlStart, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText114 = new wxStaticText(this, wxID_ANY, _("End Intensity"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer95->Add(StaticText114, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	SliderEnd = new wxSlider(this, IDD_SLIDER_Eff_On_End, 100, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Eff_On_End"));
	FlexGridSizer95->Add(SliderEnd, 1, wxALL|wxEXPAND, 2);
	TextCtrlEnd = new wxTextCtrl(this, ID_TEXTCTRL_Eff_On_End, _("100"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_Eff_On_End"));
	FlexGridSizer95->Add(TextCtrlEnd, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText174 = new wxStaticText(this, wxID_ANY, _("Cycle Count"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer95->Add(StaticText174, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	SliderCycles = new wxSlider(this, IDD_SLIDER_On_Cycles, 10, 0, 200, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_On_Cycles"));
	FlexGridSizer95->Add(SliderCycles, 1, wxALL|wxEXPAND, 2);
	TextCtrlCycles = new wxTextCtrl(this, ID_TEXTCTRL_On_Cycles, _("1.0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_On_Cycles"));
	TextCtrlCycles->SetMaxLength(4);
	FlexGridSizer95->Add(TextCtrlCycles, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
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
	Connect(IDD_SLIDER_On_Cycles,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&OnPanel::UpdateLinkedTextCtrlFloat);
	Connect(ID_TEXTCTRL_On_Cycles,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&OnPanel::UpdateLinkedSliderFloat);
	//*)
    
    SetName("ID_PANEL_ON");
}

OnPanel::~OnPanel()
{
	//(*Destroy(OnPanel)
	//*)
}

PANEL_EVENT_HANDLERS(OnPanel)

