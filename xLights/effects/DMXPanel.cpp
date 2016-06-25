#include "DMXPanel.h"

#include "EffectPanelUtils.h"
//(*InternalHeaders(DMXPanel)
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/bitmap.h>
#include <wx/slider.h>
#include <wx/choice.h>
#include <wx/bmpbuttn.h>
#include <wx/intl.h>
#include <wx/image.h>
#include <wx/string.h>
//*)

//(*IdInit(DMXPanel)
const long DMXPanel::ID_SLIDER_DMX1 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX1 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX1 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX2 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX2 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX2 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX3 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX3 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX3 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX4 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX4 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX4 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX5 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX5 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX5 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX6 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX6 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX6 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX7 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX7 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX7 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX8 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX8 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX8 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX9 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX9 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX9 = wxNewId();
const long DMXPanel::ID_CHOICE_Num_Dmx_Channels = wxNewId();
const long DMXPanel::ID_PANEL6 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX10 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX10 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX10 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX11 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX11 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX11 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX12 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX12 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX12 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX13 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX13 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX13 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX14 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX14 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX14 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX15 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX15 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX15 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX16 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX16 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX16 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX17 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX17 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX17 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX18 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX18 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX18 = wxNewId();
const long DMXPanel::ID_PANEL28 = wxNewId();
const long DMXPanel::ID_NOTEBOOK1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(DMXPanel,wxPanel)
	//(*EventTable(DMXPanel)
	//*)
END_EVENT_TABLE()

DMXPanel::DMXPanel(wxWindow* parent)
{
	//(*Initialize(DMXPanel)
	wxStaticText* StaticText215;
	wxStaticText* StaticText217;
	wxSlider* Slider_DMX16;
	wxStaticText* StaticText223;
	wxStaticText* StaticText219;
	wxStaticText* StaticText224;
	wxStaticText* StaticText218;
	wxTextCtrl* TextCtrl1_DMX5;
	wxSlider* Slider_DMX3;
	wxStaticText* StaticText239;
	wxSlider* Slider_DMX6;
	wxFlexGridSizer* FlexGridSizer149;
	wxTextCtrl* TextCtrl1_DMX6;
	wxStaticText* StaticText220;
	wxSlider* Slider_DMX14;
	wxSlider* Slider_DMX2;
	wxTextCtrl* TextCtrl_DMX18;
	wxSlider* Slider_DMX4;
	wxStaticText* StaticText2;
	wxTextCtrl* TextCtrl1_DMX7;
	wxSlider* Slider_DMX15;
	wxSlider* Slider_DMX18;
	wxFlexGridSizer* FlexGridSizer146;
	wxSlider* Slider_DMX1;
	wxStaticText* StaticText238;
	wxTextCtrl* TextCtrl_DMX9;
	wxStaticText* StaticText1;
	wxTextCtrl* TextCtrl1_DMX1;
	wxStaticText* StaticText3;
	wxTextCtrl* TextCtrl_DMX13;
	wxStaticText* StaticText225;
	wxTextCtrl* TextCtrl_DMX17;
	wxTextCtrl* TextCtrl1_DMX2;
	wxTextCtrl* TextCtrl_DMX15;
	wxTextCtrl* TextCtrl_DMX14;
	wxSlider* Slider_DMX11;
	wxSlider* Slider_DMX7;
	wxTextCtrl* TextCtrl_DMX11;
	wxTextCtrl* TextCtrl_DMX4;
	wxStaticText* StaticText216;
	wxStaticText* StaticText100;
	wxSlider* Slider_DMX13;
	wxTextCtrl* TextCtrl_DMX10;
	wxTextCtrl* TextCtrl1_DMX8;
	wxStaticText* StaticText222;
	wxStaticText* StaticText240;
	wxSlider* Slider_DMX9;
	wxSlider* Slider_DMX17;
	wxStaticText* StaticText221;
	wxSlider* Slider_DMX8;
	wxTextCtrl* TextCtrl1_DMX3;
	wxTextCtrl* TextCtrl_DMX12;
	wxSlider* Slider_DMX10;
	wxSlider* Slider_DMX5;
	wxFlexGridSizer* FlexGridSizer147;
	wxSlider* Slider_DMX12;
	wxTextCtrl* TextCtrl_DMX16;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer146 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer146->AddGrowableCol(0);
	Notebook7 = new wxNotebook(this, ID_NOTEBOOK1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK1"));
	Panel18 = new wxPanel(Notebook7, ID_PANEL6, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL6"));
	FlexGridSizer149 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer149->AddGrowableCol(1);
	StaticText215 = new wxStaticText(Panel18, wxID_ANY, _("Channel 1:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer149->Add(StaticText215, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX1 = new wxSlider(Panel18, ID_SLIDER_DMX1, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX1"));
	FlexGridSizer149->Add(Slider_DMX1, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX1 = new ValueCurveButton(Panel18, ID_VALUECURVE_DMX1, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX1"));
	FlexGridSizer149->Add(ValueCurve_DMX1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl1_DMX1 = new wxTextCtrl(Panel18, IDD_TEXTCTRL_DMX1, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel18,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX1"));
	FlexGridSizer149->Add(TextCtrl1_DMX1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText100 = new wxStaticText(Panel18, wxID_ANY, _("Channel 2:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer149->Add(StaticText100, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX2 = new wxSlider(Panel18, ID_SLIDER_DMX2, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX2"));
	FlexGridSizer149->Add(Slider_DMX2, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX2 = new ValueCurveButton(Panel18, ID_VALUECURVE_DMX2, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX2"));
	FlexGridSizer149->Add(ValueCurve_DMX2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl1_DMX2 = new wxTextCtrl(Panel18, IDD_TEXTCTRL_DMX2, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel18,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX2"));
	FlexGridSizer149->Add(TextCtrl1_DMX2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText216 = new wxStaticText(Panel18, wxID_ANY, _("Channel 3:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer149->Add(StaticText216, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX3 = new wxSlider(Panel18, ID_SLIDER_DMX3, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX3"));
	FlexGridSizer149->Add(Slider_DMX3, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX3 = new ValueCurveButton(Panel18, ID_VALUECURVE_DMX3, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX3"));
	FlexGridSizer149->Add(ValueCurve_DMX3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl1_DMX3 = new wxTextCtrl(Panel18, IDD_TEXTCTRL_DMX3, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel18,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX3"));
	FlexGridSizer149->Add(TextCtrl1_DMX3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText217 = new wxStaticText(Panel18, wxID_ANY, _("Channel 4:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer149->Add(StaticText217, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX4 = new wxSlider(Panel18, ID_SLIDER_DMX4, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX4"));
	FlexGridSizer149->Add(Slider_DMX4, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX4 = new ValueCurveButton(Panel18, ID_VALUECURVE_DMX4, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX4"));
	FlexGridSizer149->Add(ValueCurve_DMX4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_DMX4 = new wxTextCtrl(Panel18, IDD_TEXTCTRL_DMX4, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel18,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX4"));
	FlexGridSizer149->Add(TextCtrl_DMX4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText218 = new wxStaticText(Panel18, wxID_ANY, _("Channel 5:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer149->Add(StaticText218, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX5 = new wxSlider(Panel18, ID_SLIDER_DMX5, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX5"));
	FlexGridSizer149->Add(Slider_DMX5, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX5 = new ValueCurveButton(Panel18, ID_VALUECURVE_DMX5, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX5"));
	FlexGridSizer149->Add(ValueCurve_DMX5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl1_DMX5 = new wxTextCtrl(Panel18, IDD_TEXTCTRL_DMX5, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel18,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX5"));
	FlexGridSizer149->Add(TextCtrl1_DMX5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText219 = new wxStaticText(Panel18, wxID_ANY, _("Channel 6:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer149->Add(StaticText219, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX6 = new wxSlider(Panel18, ID_SLIDER_DMX6, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX6"));
	FlexGridSizer149->Add(Slider_DMX6, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX6 = new ValueCurveButton(Panel18, ID_VALUECURVE_DMX6, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX6"));
	FlexGridSizer149->Add(ValueCurve_DMX6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl1_DMX6 = new wxTextCtrl(Panel18, IDD_TEXTCTRL_DMX6, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel18,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX6"));
	FlexGridSizer149->Add(TextCtrl1_DMX6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText220 = new wxStaticText(Panel18, wxID_ANY, _("Channel 7:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer149->Add(StaticText220, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX7 = new wxSlider(Panel18, ID_SLIDER_DMX7, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX7"));
	FlexGridSizer149->Add(Slider_DMX7, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX7 = new ValueCurveButton(Panel18, ID_VALUECURVE_DMX7, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX7"));
	FlexGridSizer149->Add(ValueCurve_DMX7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl1_DMX7 = new wxTextCtrl(Panel18, IDD_TEXTCTRL_DMX7, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel18,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX7"));
	FlexGridSizer149->Add(TextCtrl1_DMX7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText221 = new wxStaticText(Panel18, wxID_ANY, _("Channel 8:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer149->Add(StaticText221, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX8 = new wxSlider(Panel18, ID_SLIDER_DMX8, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX8"));
	FlexGridSizer149->Add(Slider_DMX8, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX8 = new ValueCurveButton(Panel18, ID_VALUECURVE_DMX8, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX8"));
	FlexGridSizer149->Add(ValueCurve_DMX8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl1_DMX8 = new wxTextCtrl(Panel18, IDD_TEXTCTRL_DMX8, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel18,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX8"));
	FlexGridSizer149->Add(TextCtrl1_DMX8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText222 = new wxStaticText(Panel18, wxID_ANY, _("Channel 9:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer149->Add(StaticText222, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX9 = new wxSlider(Panel18, ID_SLIDER_DMX9, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX9"));
	FlexGridSizer149->Add(Slider_DMX9, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX9 = new ValueCurveButton(Panel18, ID_VALUECURVE_DMX9, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX9"));
	FlexGridSizer149->Add(ValueCurve_DMX9, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_DMX9 = new wxTextCtrl(Panel18, IDD_TEXTCTRL_DMX9, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel18,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX9"));
	FlexGridSizer149->Add(TextCtrl_DMX9, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer149->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Num_Dmx_Channels = new wxChoice(Panel18, ID_CHOICE_Num_Dmx_Channels, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Num_Dmx_Channels"));
	Choice_Num_Dmx_Channels->SetSelection( Choice_Num_Dmx_Channels->Append(_("Use 1 channel")) );
	Choice_Num_Dmx_Channels->Append(_("Use 2 channels"));
	Choice_Num_Dmx_Channels->Append(_("Use 3 channels"));
	Choice_Num_Dmx_Channels->Append(_("Use 4 channels"));
	Choice_Num_Dmx_Channels->Append(_("Use 5 channels"));
	Choice_Num_Dmx_Channels->Append(_("Use 6 channels"));
	Choice_Num_Dmx_Channels->Append(_("Use 7 channels"));
	Choice_Num_Dmx_Channels->Append(_("Use 8 channels"));
	Choice_Num_Dmx_Channels->Append(_("Use 9 channels"));
	Choice_Num_Dmx_Channels->Append(_("Use 10 channels"));
	Choice_Num_Dmx_Channels->Append(_("Use 11 channels"));
	Choice_Num_Dmx_Channels->Append(_("Use 12 channels"));
	Choice_Num_Dmx_Channels->Append(_("Use 13 channels"));
	Choice_Num_Dmx_Channels->Append(_("Use 14 channels"));
	Choice_Num_Dmx_Channels->Append(_("Use 15 channels"));
	Choice_Num_Dmx_Channels->Append(_("Use 16 channels"));
	Choice_Num_Dmx_Channels->Append(_("Use 17 channels"));
	Choice_Num_Dmx_Channels->Append(_("Use 18 channels"));
	FlexGridSizer149->Add(Choice_Num_Dmx_Channels, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer149->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel18->SetSizer(FlexGridSizer149);
	FlexGridSizer149->Fit(Panel18);
	FlexGridSizer149->SetSizeHints(Panel18);
	Panel19 = new wxPanel(Notebook7, ID_PANEL28, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL28"));
	FlexGridSizer147 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer147->AddGrowableCol(1);
	StaticText223 = new wxStaticText(Panel19, wxID_ANY, _("Channel 10:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer147->Add(StaticText223, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX10 = new wxSlider(Panel19, ID_SLIDER_DMX10, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX10"));
	FlexGridSizer147->Add(Slider_DMX10, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX10 = new ValueCurveButton(Panel19, ID_VALUECURVE_DMX10, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX10"));
	FlexGridSizer147->Add(ValueCurve_DMX10, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_DMX10 = new wxTextCtrl(Panel19, IDD_TEXTCTRL_DMX10, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel19,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX10"));
	FlexGridSizer147->Add(TextCtrl_DMX10, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText224 = new wxStaticText(Panel19, wxID_ANY, _("Channel 11:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer147->Add(StaticText224, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX11 = new wxSlider(Panel19, ID_SLIDER_DMX11, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX11"));
	FlexGridSizer147->Add(Slider_DMX11, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX11 = new ValueCurveButton(Panel19, ID_VALUECURVE_DMX11, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX11"));
	FlexGridSizer147->Add(ValueCurve_DMX11, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_DMX11 = new wxTextCtrl(Panel19, IDD_TEXTCTRL_DMX11, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel19,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX11"));
	FlexGridSizer147->Add(TextCtrl_DMX11, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText225 = new wxStaticText(Panel19, wxID_ANY, _("Channel 12:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer147->Add(StaticText225, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX12 = new wxSlider(Panel19, ID_SLIDER_DMX12, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX12"));
	FlexGridSizer147->Add(Slider_DMX12, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX12 = new ValueCurveButton(Panel19, ID_VALUECURVE_DMX12, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX12"));
	FlexGridSizer147->Add(ValueCurve_DMX12, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_DMX12 = new wxTextCtrl(Panel19, IDD_TEXTCTRL_DMX12, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel19,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX12"));
	FlexGridSizer147->Add(TextCtrl_DMX12, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText238 = new wxStaticText(Panel19, wxID_ANY, _("Channel 13:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer147->Add(StaticText238, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX13 = new wxSlider(Panel19, ID_SLIDER_DMX13, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX13"));
	FlexGridSizer147->Add(Slider_DMX13, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX13 = new ValueCurveButton(Panel19, ID_VALUECURVE_DMX13, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX13"));
	FlexGridSizer147->Add(ValueCurve_DMX13, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_DMX13 = new wxTextCtrl(Panel19, IDD_TEXTCTRL_DMX13, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel19,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX13"));
	FlexGridSizer147->Add(TextCtrl_DMX13, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText239 = new wxStaticText(Panel19, wxID_ANY, _("Channel 14:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer147->Add(StaticText239, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX14 = new wxSlider(Panel19, ID_SLIDER_DMX14, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX14"));
	FlexGridSizer147->Add(Slider_DMX14, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX14 = new ValueCurveButton(Panel19, ID_VALUECURVE_DMX14, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX14"));
	FlexGridSizer147->Add(ValueCurve_DMX14, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_DMX14 = new wxTextCtrl(Panel19, IDD_TEXTCTRL_DMX14, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel19,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX14"));
	FlexGridSizer147->Add(TextCtrl_DMX14, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText240 = new wxStaticText(Panel19, wxID_ANY, _("Channel 15:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer147->Add(StaticText240, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX15 = new wxSlider(Panel19, ID_SLIDER_DMX15, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX15"));
	FlexGridSizer147->Add(Slider_DMX15, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX15 = new ValueCurveButton(Panel19, ID_VALUECURVE_DMX15, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX15"));
	FlexGridSizer147->Add(ValueCurve_DMX15, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_DMX15 = new wxTextCtrl(Panel19, IDD_TEXTCTRL_DMX15, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel19,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX15"));
	FlexGridSizer147->Add(TextCtrl_DMX15, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText1 = new wxStaticText(Panel19, wxID_ANY, _("Channel 16:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer147->Add(StaticText1, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX16 = new wxSlider(Panel19, ID_SLIDER_DMX16, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX16"));
	FlexGridSizer147->Add(Slider_DMX16, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX16 = new ValueCurveButton(Panel19, ID_VALUECURVE_DMX16, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX16"));
	FlexGridSizer147->Add(ValueCurve_DMX16, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_DMX16 = new wxTextCtrl(Panel19, IDD_TEXTCTRL_DMX16, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel19,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX16"));
	FlexGridSizer147->Add(TextCtrl_DMX16, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText2 = new wxStaticText(Panel19, wxID_ANY, _("Channel 17:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer147->Add(StaticText2, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX17 = new wxSlider(Panel19, ID_SLIDER_DMX17, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX17"));
	FlexGridSizer147->Add(Slider_DMX17, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX17 = new ValueCurveButton(Panel19, ID_VALUECURVE_DMX17, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX17"));
	FlexGridSizer147->Add(ValueCurve_DMX17, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_DMX17 = new wxTextCtrl(Panel19, IDD_TEXTCTRL_DMX17, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel19,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX17"));
	FlexGridSizer147->Add(TextCtrl_DMX17, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText3 = new wxStaticText(Panel19, wxID_ANY, _("Channel 18:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer147->Add(StaticText3, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX18 = new wxSlider(Panel19, ID_SLIDER_DMX18, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX18"));
	FlexGridSizer147->Add(Slider_DMX18, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX18 = new ValueCurveButton(Panel19, ID_VALUECURVE_DMX18, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX18"));
	FlexGridSizer147->Add(ValueCurve_DMX18, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_DMX18 = new wxTextCtrl(Panel19, IDD_TEXTCTRL_DMX18, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel19,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX18"));
	FlexGridSizer147->Add(TextCtrl_DMX18, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel19->SetSizer(FlexGridSizer147);
	FlexGridSizer147->Fit(Panel19);
	FlexGridSizer147->SetSizeHints(Panel19);
	Notebook7->AddPage(Panel18, _("Channels 1-9"), false);
	Notebook7->AddPage(Panel19, _("Channels 10-18"), false);
	FlexGridSizer146->Add(Notebook7, 1, wxALL|wxEXPAND, 2);
	SetSizer(FlexGridSizer146);
	FlexGridSizer146->Fit(this);
	FlexGridSizer146->SetSizeHints(this);

	Connect(ID_SLIDER_DMX1,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&DMXPanel::UpdateLinkedTextCtrl);
	Connect(ID_VALUECURVE_DMX1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_DMX1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&DMXPanel::UpdateLinkedSlider);
	Connect(ID_SLIDER_DMX2,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&DMXPanel::UpdateLinkedTextCtrl);
	Connect(ID_VALUECURVE_DMX2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_DMX2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&DMXPanel::UpdateLinkedSlider);
	Connect(ID_SLIDER_DMX3,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&DMXPanel::UpdateLinkedTextCtrl);
	Connect(ID_VALUECURVE_DMX3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_DMX3,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&DMXPanel::UpdateLinkedSlider);
	Connect(ID_SLIDER_DMX4,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&DMXPanel::UpdateLinkedTextCtrl);
	Connect(ID_VALUECURVE_DMX4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_DMX4,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&DMXPanel::UpdateLinkedSlider);
	Connect(ID_SLIDER_DMX5,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&DMXPanel::UpdateLinkedTextCtrl);
	Connect(ID_VALUECURVE_DMX5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_DMX5,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&DMXPanel::UpdateLinkedSlider);
	Connect(ID_SLIDER_DMX6,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&DMXPanel::UpdateLinkedTextCtrl);
	Connect(ID_VALUECURVE_DMX6,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_DMX6,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&DMXPanel::UpdateLinkedSlider);
	Connect(ID_SLIDER_DMX7,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&DMXPanel::UpdateLinkedTextCtrl);
	Connect(ID_VALUECURVE_DMX7,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_DMX7,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&DMXPanel::UpdateLinkedSlider);
	Connect(ID_SLIDER_DMX8,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&DMXPanel::UpdateLinkedTextCtrl);
	Connect(ID_VALUECURVE_DMX8,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_DMX8,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&DMXPanel::UpdateLinkedSlider);
	Connect(ID_SLIDER_DMX9,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&DMXPanel::UpdateLinkedTextCtrl);
	Connect(ID_VALUECURVE_DMX9,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_DMX9,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&DMXPanel::UpdateLinkedSlider);
	Connect(ID_SLIDER_DMX10,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&DMXPanel::UpdateLinkedTextCtrl);
	Connect(ID_VALUECURVE_DMX10,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_DMX10,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&DMXPanel::UpdateLinkedSlider);
	Connect(ID_SLIDER_DMX11,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&DMXPanel::UpdateLinkedTextCtrl);
	Connect(ID_VALUECURVE_DMX11,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_DMX11,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&DMXPanel::UpdateLinkedSlider);
	Connect(ID_SLIDER_DMX12,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&DMXPanel::UpdateLinkedTextCtrl);
	Connect(ID_VALUECURVE_DMX12,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_DMX12,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&DMXPanel::UpdateLinkedSlider);
	Connect(ID_SLIDER_DMX13,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&DMXPanel::UpdateLinkedTextCtrl);
	Connect(ID_VALUECURVE_DMX13,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_DMX13,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&DMXPanel::UpdateLinkedSlider);
	Connect(ID_SLIDER_DMX14,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&DMXPanel::UpdateLinkedTextCtrl);
	Connect(ID_VALUECURVE_DMX14,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_DMX14,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&DMXPanel::UpdateLinkedSlider);
	Connect(ID_SLIDER_DMX15,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&DMXPanel::UpdateLinkedTextCtrl);
	Connect(ID_VALUECURVE_DMX15,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_DMX15,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&DMXPanel::UpdateLinkedSlider);
	Connect(ID_SLIDER_DMX16,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&DMXPanel::UpdateLinkedTextCtrl);
	Connect(ID_VALUECURVE_DMX16,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_DMX16,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&DMXPanel::UpdateLinkedSlider);
	Connect(ID_SLIDER_DMX17,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&DMXPanel::UpdateLinkedTextCtrl);
	Connect(ID_VALUECURVE_DMX17,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_DMX17,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&DMXPanel::UpdateLinkedSlider);
	Connect(ID_SLIDER_DMX18,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&DMXPanel::UpdateLinkedTextCtrl);
	Connect(ID_VALUECURVE_DMX18,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_DMX18,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&DMXPanel::UpdateLinkedSlider);
	//*)
    SetName("ID_PANEL_DMX");

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&DMXPanel::OnVCChanged, 0, this);

    ValueCurve_DMX1->GetValue()->SetLimits(0, 255);
    ValueCurve_DMX2->GetValue()->SetLimits(0, 255);
    ValueCurve_DMX3->GetValue()->SetLimits(0, 255);
    ValueCurve_DMX4->GetValue()->SetLimits(0, 255);
    ValueCurve_DMX5->GetValue()->SetLimits(0, 255);
    ValueCurve_DMX6->GetValue()->SetLimits(0, 255);
    ValueCurve_DMX7->GetValue()->SetLimits(0, 255);
    ValueCurve_DMX8->GetValue()->SetLimits(0, 255);
    ValueCurve_DMX9->GetValue()->SetLimits(0, 255);
    ValueCurve_DMX10->GetValue()->SetLimits(0, 255);
    ValueCurve_DMX11->GetValue()->SetLimits(0, 255);
    ValueCurve_DMX12->GetValue()->SetLimits(0, 255);
    ValueCurve_DMX13->GetValue()->SetLimits(0, 255);
    ValueCurve_DMX14->GetValue()->SetLimits(0, 255);
    ValueCurve_DMX15->GetValue()->SetLimits(0, 255);
    ValueCurve_DMX16->GetValue()->SetLimits(0, 255);
    ValueCurve_DMX17->GetValue()->SetLimits(0, 255);
    ValueCurve_DMX18->GetValue()->SetLimits(0, 255);
}

DMXPanel::~DMXPanel()
{
	//(*Destroy(DMXPanel)
	//*)
}

PANEL_EVENT_HANDLERS(DMXPanel)
