#include "DMXPanel.h"
#include "DMXEffect.h"

#include "EffectPanelUtils.h"
//(*InternalHeaders(DMXPanel)
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/bitmap.h>
#include <wx/slider.h>
#include <wx/bmpbuttn.h>
#include <wx/intl.h>
#include <wx/image.h>
#include <wx/string.h>
//*)

//(*IdInit(DMXPanel)
const long DMXPanel::ID_STATICTEXT_DMX1 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX1 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX1 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX1 = wxNewId();
const long DMXPanel::ID_STATICTEXT_DMX2 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX2 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX2 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX2 = wxNewId();
const long DMXPanel::ID_STATICTEXT_DMX3 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX3 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX3 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX3 = wxNewId();
const long DMXPanel::ID_STATICTEXT_DMX4 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX4 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX4 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX4 = wxNewId();
const long DMXPanel::ID_STATICTEXT_DMX5 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX5 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX5 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX5 = wxNewId();
const long DMXPanel::ID_STATICTEXT_DMX6 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX6 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX6 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX6 = wxNewId();
const long DMXPanel::ID_STATICTEXT_DMX7 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX7 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX7 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX7 = wxNewId();
const long DMXPanel::ID_STATICTEXT_DMX8 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX8 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX8 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX8 = wxNewId();
const long DMXPanel::ID_STATICTEXT_DMX9 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX9 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX9 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX9 = wxNewId();
const long DMXPanel::ID_PANEL6 = wxNewId();
const long DMXPanel::ID_STATICTEXT_DMX10 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX10 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX10 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX10 = wxNewId();
const long DMXPanel::ID_STATICTEXT_DMX11 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX11 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX11 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX11 = wxNewId();
const long DMXPanel::ID_STATICTEXT_DMX12 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX12 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX12 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX12 = wxNewId();
const long DMXPanel::ID_STATICTEXT_DMX13 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX13 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX13 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX13 = wxNewId();
const long DMXPanel::ID_STATICTEXT_DMX14 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX14 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX14 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX14 = wxNewId();
const long DMXPanel::ID_STATICTEXT_DMX15 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX15 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX15 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX15 = wxNewId();
const long DMXPanel::ID_STATICTEXT_DMX16 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX16 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX16 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX16 = wxNewId();
const long DMXPanel::ID_STATICTEXT_DMX17 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX17 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX17 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX17 = wxNewId();
const long DMXPanel::ID_STATICTEXT_DMX18 = wxNewId();
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
	BulkEditTextCtrl* TextCtrl1_DMX1;
	BulkEditTextCtrl* TextCtrl1_DMX5;
	BulkEditTextCtrl* TextCtrl_DMX9;
	BulkEditTextCtrl* TextCtrl_DMX18;
	BulkEditTextCtrl* TextCtrl1_DMX2;
	BulkEditTextCtrl* TextCtrl1_DMX3;
	BulkEditTextCtrl* TextCtrl_DMX12;
	BulkEditTextCtrl* TextCtrl_DMX11;
	BulkEditTextCtrl* TextCtrl1_DMX8;
	BulkEditTextCtrl* TextCtrl_DMX16;
	BulkEditTextCtrl* TextCtrl_DMX13;
	BulkEditTextCtrl* TextCtrl_DMX4;
	BulkEditTextCtrl* TextCtrl_DMX10;
	BulkEditTextCtrl* TextCtrl_DMX15;
	BulkEditTextCtrl* TextCtrl1_DMX7;
	BulkEditTextCtrl* TextCtrl1_DMX6;
	BulkEditTextCtrl* TextCtrl_DMX14;
	BulkEditTextCtrl* TextCtrl_DMX17;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer_Main = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer_Main->AddGrowableCol(0);
	Notebook7 = new wxNotebook(this, ID_NOTEBOOK1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK1"));
	ChannelPanel1 = new wxPanel(Notebook7, ID_PANEL6, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL6"));
	FlexGridSizer_Panel1 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer_Panel1->AddGrowableCol(1);
	Label_DMX1 = new wxStaticText(ChannelPanel1, ID_STATICTEXT_DMX1, _("Channel 1:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX1"));
	FlexGridSizer_Panel1->Add(Label_DMX1, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX1 = new BulkEditSlider(ChannelPanel1, ID_SLIDER_DMX1, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX1"));
	FlexGridSizer_Panel1->Add(Slider_DMX1, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX1 = new BulkEditValueCurveButton(ChannelPanel1, ID_VALUECURVE_DMX1, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX1"));
	FlexGridSizer_Panel1->Add(ValueCurve_DMX1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl1_DMX1 = new BulkEditTextCtrl(ChannelPanel1, IDD_TEXTCTRL_DMX1, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel1,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX1"));
	FlexGridSizer_Panel1->Add(TextCtrl1_DMX1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX2 = new wxStaticText(ChannelPanel1, ID_STATICTEXT_DMX2, _("Channel 2:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX2"));
	FlexGridSizer_Panel1->Add(Label_DMX2, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX2 = new BulkEditSlider(ChannelPanel1, ID_SLIDER_DMX2, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX2"));
	FlexGridSizer_Panel1->Add(Slider_DMX2, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX2 = new BulkEditValueCurveButton(ChannelPanel1, ID_VALUECURVE_DMX2, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX2"));
	FlexGridSizer_Panel1->Add(ValueCurve_DMX2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl1_DMX2 = new BulkEditTextCtrl(ChannelPanel1, IDD_TEXTCTRL_DMX2, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel1,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX2"));
	FlexGridSizer_Panel1->Add(TextCtrl1_DMX2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX3 = new wxStaticText(ChannelPanel1, ID_STATICTEXT_DMX3, _("Channel 3:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX3"));
	FlexGridSizer_Panel1->Add(Label_DMX3, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX3 = new BulkEditSlider(ChannelPanel1, ID_SLIDER_DMX3, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX3"));
	FlexGridSizer_Panel1->Add(Slider_DMX3, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX3 = new BulkEditValueCurveButton(ChannelPanel1, ID_VALUECURVE_DMX3, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX3"));
	FlexGridSizer_Panel1->Add(ValueCurve_DMX3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl1_DMX3 = new BulkEditTextCtrl(ChannelPanel1, IDD_TEXTCTRL_DMX3, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel1,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX3"));
	FlexGridSizer_Panel1->Add(TextCtrl1_DMX3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX4 = new wxStaticText(ChannelPanel1, ID_STATICTEXT_DMX4, _("Channel 4:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX4"));
	FlexGridSizer_Panel1->Add(Label_DMX4, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX4 = new BulkEditSlider(ChannelPanel1, ID_SLIDER_DMX4, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX4"));
	FlexGridSizer_Panel1->Add(Slider_DMX4, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX4 = new BulkEditValueCurveButton(ChannelPanel1, ID_VALUECURVE_DMX4, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX4"));
	FlexGridSizer_Panel1->Add(ValueCurve_DMX4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_DMX4 = new BulkEditTextCtrl(ChannelPanel1, IDD_TEXTCTRL_DMX4, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel1,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX4"));
	FlexGridSizer_Panel1->Add(TextCtrl_DMX4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX5 = new wxStaticText(ChannelPanel1, ID_STATICTEXT_DMX5, _("Channel 5:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX5"));
	FlexGridSizer_Panel1->Add(Label_DMX5, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX5 = new BulkEditSlider(ChannelPanel1, ID_SLIDER_DMX5, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX5"));
	FlexGridSizer_Panel1->Add(Slider_DMX5, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX5 = new BulkEditValueCurveButton(ChannelPanel1, ID_VALUECURVE_DMX5, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX5"));
	FlexGridSizer_Panel1->Add(ValueCurve_DMX5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl1_DMX5 = new BulkEditTextCtrl(ChannelPanel1, IDD_TEXTCTRL_DMX5, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel1,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX5"));
	FlexGridSizer_Panel1->Add(TextCtrl1_DMX5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX6 = new wxStaticText(ChannelPanel1, ID_STATICTEXT_DMX6, _("Channel 6:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX6"));
	FlexGridSizer_Panel1->Add(Label_DMX6, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX6 = new BulkEditSlider(ChannelPanel1, ID_SLIDER_DMX6, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX6"));
	FlexGridSizer_Panel1->Add(Slider_DMX6, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX6 = new BulkEditValueCurveButton(ChannelPanel1, ID_VALUECURVE_DMX6, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX6"));
	FlexGridSizer_Panel1->Add(ValueCurve_DMX6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl1_DMX6 = new BulkEditTextCtrl(ChannelPanel1, IDD_TEXTCTRL_DMX6, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel1,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX6"));
	FlexGridSizer_Panel1->Add(TextCtrl1_DMX6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX7 = new wxStaticText(ChannelPanel1, ID_STATICTEXT_DMX7, _("Channel 7:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX7"));
	FlexGridSizer_Panel1->Add(Label_DMX7, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX7 = new BulkEditSlider(ChannelPanel1, ID_SLIDER_DMX7, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX7"));
	FlexGridSizer_Panel1->Add(Slider_DMX7, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX7 = new BulkEditValueCurveButton(ChannelPanel1, ID_VALUECURVE_DMX7, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX7"));
	FlexGridSizer_Panel1->Add(ValueCurve_DMX7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl1_DMX7 = new BulkEditTextCtrl(ChannelPanel1, IDD_TEXTCTRL_DMX7, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel1,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX7"));
	FlexGridSizer_Panel1->Add(TextCtrl1_DMX7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX8 = new wxStaticText(ChannelPanel1, ID_STATICTEXT_DMX8, _("Channel 8:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX8"));
	FlexGridSizer_Panel1->Add(Label_DMX8, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX8 = new BulkEditSlider(ChannelPanel1, ID_SLIDER_DMX8, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX8"));
	FlexGridSizer_Panel1->Add(Slider_DMX8, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX8 = new BulkEditValueCurveButton(ChannelPanel1, ID_VALUECURVE_DMX8, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX8"));
	FlexGridSizer_Panel1->Add(ValueCurve_DMX8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl1_DMX8 = new BulkEditTextCtrl(ChannelPanel1, IDD_TEXTCTRL_DMX8, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel1,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX8"));
	FlexGridSizer_Panel1->Add(TextCtrl1_DMX8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX9 = new wxStaticText(ChannelPanel1, ID_STATICTEXT_DMX9, _("Channel 9:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX9"));
	FlexGridSizer_Panel1->Add(Label_DMX9, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX9 = new BulkEditSlider(ChannelPanel1, ID_SLIDER_DMX9, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX9"));
	FlexGridSizer_Panel1->Add(Slider_DMX9, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX9 = new BulkEditValueCurveButton(ChannelPanel1, ID_VALUECURVE_DMX9, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX9"));
	FlexGridSizer_Panel1->Add(ValueCurve_DMX9, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_DMX9 = new BulkEditTextCtrl(ChannelPanel1, IDD_TEXTCTRL_DMX9, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel1,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX9"));
	FlexGridSizer_Panel1->Add(TextCtrl_DMX9, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	ChannelPanel1->SetSizer(FlexGridSizer_Panel1);
	FlexGridSizer_Panel1->Fit(ChannelPanel1);
	FlexGridSizer_Panel1->SetSizeHints(ChannelPanel1);
	ChannelPanel2 = new wxPanel(Notebook7, ID_PANEL28, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL28"));
	FlexGridSizer_Panel2 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer_Panel2->AddGrowableCol(1);
	Label_DMX10 = new wxStaticText(ChannelPanel2, ID_STATICTEXT_DMX10, _("Channel 10:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX10"));
	FlexGridSizer_Panel2->Add(Label_DMX10, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX10 = new BulkEditSlider(ChannelPanel2, ID_SLIDER_DMX10, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX10"));
	FlexGridSizer_Panel2->Add(Slider_DMX10, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX10 = new ValueCurveButton(ChannelPanel2, ID_VALUECURVE_DMX10, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX10"));
	FlexGridSizer_Panel2->Add(ValueCurve_DMX10, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_DMX10 = new BulkEditTextCtrl(ChannelPanel2, IDD_TEXTCTRL_DMX10, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel2,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX10"));
	FlexGridSizer_Panel2->Add(TextCtrl_DMX10, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX11 = new wxStaticText(ChannelPanel2, ID_STATICTEXT_DMX11, _("Channel 11:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX11"));
	FlexGridSizer_Panel2->Add(Label_DMX11, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX11 = new BulkEditSlider(ChannelPanel2, ID_SLIDER_DMX11, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX11"));
	FlexGridSizer_Panel2->Add(Slider_DMX11, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX11 = new ValueCurveButton(ChannelPanel2, ID_VALUECURVE_DMX11, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX11"));
	FlexGridSizer_Panel2->Add(ValueCurve_DMX11, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_DMX11 = new BulkEditTextCtrl(ChannelPanel2, IDD_TEXTCTRL_DMX11, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel2,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX11"));
	FlexGridSizer_Panel2->Add(TextCtrl_DMX11, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX12 = new wxStaticText(ChannelPanel2, ID_STATICTEXT_DMX12, _("Channel 12:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX12"));
	FlexGridSizer_Panel2->Add(Label_DMX12, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX12 = new BulkEditSlider(ChannelPanel2, ID_SLIDER_DMX12, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX12"));
	FlexGridSizer_Panel2->Add(Slider_DMX12, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX12 = new ValueCurveButton(ChannelPanel2, ID_VALUECURVE_DMX12, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX12"));
	FlexGridSizer_Panel2->Add(ValueCurve_DMX12, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_DMX12 = new BulkEditTextCtrl(ChannelPanel2, IDD_TEXTCTRL_DMX12, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel2,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX12"));
	FlexGridSizer_Panel2->Add(TextCtrl_DMX12, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX13 = new wxStaticText(ChannelPanel2, ID_STATICTEXT_DMX13, _("Channel 13:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX13"));
	FlexGridSizer_Panel2->Add(Label_DMX13, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX13 = new BulkEditSlider(ChannelPanel2, ID_SLIDER_DMX13, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX13"));
	FlexGridSizer_Panel2->Add(Slider_DMX13, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX13 = new ValueCurveButton(ChannelPanel2, ID_VALUECURVE_DMX13, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX13"));
	FlexGridSizer_Panel2->Add(ValueCurve_DMX13, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_DMX13 = new BulkEditTextCtrl(ChannelPanel2, IDD_TEXTCTRL_DMX13, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel2,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX13"));
	FlexGridSizer_Panel2->Add(TextCtrl_DMX13, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX14 = new wxStaticText(ChannelPanel2, ID_STATICTEXT_DMX14, _("Channel 14:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX14"));
	FlexGridSizer_Panel2->Add(Label_DMX14, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX14 = new BulkEditSlider(ChannelPanel2, ID_SLIDER_DMX14, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX14"));
	FlexGridSizer_Panel2->Add(Slider_DMX14, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX14 = new ValueCurveButton(ChannelPanel2, ID_VALUECURVE_DMX14, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX14"));
	FlexGridSizer_Panel2->Add(ValueCurve_DMX14, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_DMX14 = new BulkEditTextCtrl(ChannelPanel2, IDD_TEXTCTRL_DMX14, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel2,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX14"));
	FlexGridSizer_Panel2->Add(TextCtrl_DMX14, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX15 = new wxStaticText(ChannelPanel2, ID_STATICTEXT_DMX15, _("Channel 15:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX15"));
	FlexGridSizer_Panel2->Add(Label_DMX15, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX15 = new BulkEditSlider(ChannelPanel2, ID_SLIDER_DMX15, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX15"));
	FlexGridSizer_Panel2->Add(Slider_DMX15, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX15 = new ValueCurveButton(ChannelPanel2, ID_VALUECURVE_DMX15, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX15"));
	FlexGridSizer_Panel2->Add(ValueCurve_DMX15, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_DMX15 = new BulkEditTextCtrl(ChannelPanel2, IDD_TEXTCTRL_DMX15, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel2,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX15"));
	FlexGridSizer_Panel2->Add(TextCtrl_DMX15, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX16 = new wxStaticText(ChannelPanel2, ID_STATICTEXT_DMX16, _("Channel 16:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX16"));
	FlexGridSizer_Panel2->Add(Label_DMX16, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX16 = new BulkEditSlider(ChannelPanel2, ID_SLIDER_DMX16, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX16"));
	FlexGridSizer_Panel2->Add(Slider_DMX16, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX16 = new ValueCurveButton(ChannelPanel2, ID_VALUECURVE_DMX16, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX16"));
	FlexGridSizer_Panel2->Add(ValueCurve_DMX16, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_DMX16 = new BulkEditTextCtrl(ChannelPanel2, IDD_TEXTCTRL_DMX16, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel2,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX16"));
	FlexGridSizer_Panel2->Add(TextCtrl_DMX16, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX17 = new wxStaticText(ChannelPanel2, ID_STATICTEXT_DMX17, _("Channel 17:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX17"));
	FlexGridSizer_Panel2->Add(Label_DMX17, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX17 = new BulkEditSlider(ChannelPanel2, ID_SLIDER_DMX17, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX17"));
	FlexGridSizer_Panel2->Add(Slider_DMX17, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX17 = new ValueCurveButton(ChannelPanel2, ID_VALUECURVE_DMX17, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX17"));
	FlexGridSizer_Panel2->Add(ValueCurve_DMX17, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_DMX17 = new BulkEditTextCtrl(ChannelPanel2, IDD_TEXTCTRL_DMX17, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel2,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX17"));
	FlexGridSizer_Panel2->Add(TextCtrl_DMX17, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX18 = new wxStaticText(ChannelPanel2, ID_STATICTEXT_DMX18, _("Channel 18:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX18"));
	FlexGridSizer_Panel2->Add(Label_DMX18, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX18 = new BulkEditSlider(ChannelPanel2, ID_SLIDER_DMX18, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX18"));
	FlexGridSizer_Panel2->Add(Slider_DMX18, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX18 = new ValueCurveButton(ChannelPanel2, ID_VALUECURVE_DMX18, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX18"));
	FlexGridSizer_Panel2->Add(ValueCurve_DMX18, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_DMX18 = new BulkEditTextCtrl(ChannelPanel2, IDD_TEXTCTRL_DMX18, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel2,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX18"));
	FlexGridSizer_Panel2->Add(TextCtrl_DMX18, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ChannelPanel2->SetSizer(FlexGridSizer_Panel2);
	FlexGridSizer_Panel2->Fit(ChannelPanel2);
	FlexGridSizer_Panel2->SetSizeHints(ChannelPanel2);
	Notebook7->AddPage(ChannelPanel1, _("Channels 1-9"), false);
	Notebook7->AddPage(ChannelPanel2, _("Channels 10-18"), false);
	FlexGridSizer_Main->Add(Notebook7, 1, wxALL|wxEXPAND, 2);
	SetSizer(FlexGridSizer_Main);
	FlexGridSizer_Main->Fit(this);
	FlexGridSizer_Main->SetSizeHints(this);

	Connect(ID_VALUECURVE_DMX1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_DMX2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_DMX3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_DMX4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_DMX5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_DMX6,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_DMX7,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_DMX8,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_DMX9,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_DMX10,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_DMX11,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_DMX12,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_DMX13,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_DMX14,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_DMX15,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_DMX16,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_DMX17,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_DMX18,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	//*)
    SetName("ID_PANEL_DMX");

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&DMXPanel::OnVCChanged, 0, this);

    ValueCurve_DMX1->GetValue()->SetLimits(DMX_MIN, DMX_MAX);
    ValueCurve_DMX2->GetValue()->SetLimits(DMX_MIN, DMX_MAX);
    ValueCurve_DMX3->GetValue()->SetLimits(DMX_MIN, DMX_MAX);
    ValueCurve_DMX4->GetValue()->SetLimits(DMX_MIN, DMX_MAX);
    ValueCurve_DMX5->GetValue()->SetLimits(DMX_MIN, DMX_MAX);
    ValueCurve_DMX6->GetValue()->SetLimits(DMX_MIN, DMX_MAX);
    ValueCurve_DMX7->GetValue()->SetLimits(DMX_MIN, DMX_MAX);
    ValueCurve_DMX8->GetValue()->SetLimits(DMX_MIN, DMX_MAX);
    ValueCurve_DMX9->GetValue()->SetLimits(DMX_MIN, DMX_MAX);
    ValueCurve_DMX10->GetValue()->SetLimits(DMX_MIN, DMX_MAX);
    ValueCurve_DMX11->GetValue()->SetLimits(DMX_MIN, DMX_MAX);
    ValueCurve_DMX12->GetValue()->SetLimits(DMX_MIN, DMX_MAX);
    ValueCurve_DMX13->GetValue()->SetLimits(DMX_MIN, DMX_MAX);
    ValueCurve_DMX14->GetValue()->SetLimits(DMX_MIN, DMX_MAX);
    ValueCurve_DMX15->GetValue()->SetLimits(DMX_MIN, DMX_MAX);
    ValueCurve_DMX16->GetValue()->SetLimits(DMX_MIN, DMX_MAX);
    ValueCurve_DMX17->GetValue()->SetLimits(DMX_MIN, DMX_MAX);
    ValueCurve_DMX18->GetValue()->SetLimits(DMX_MIN, DMX_MAX);
}

DMXPanel::~DMXPanel()
{
	//(*Destroy(DMXPanel)
	//*)
}

PANEL_EVENT_HANDLERS(DMXPanel)
