/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

 //(*InternalHeaders(DMXPanel)
#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>
//*)

#include "DMXPanel.h"
#include "DMXEffect.h"
#include "../xLightsMain.h"
#include "../sequencer/MainSequencer.h"
#include "../sequencer/Effect.h"
#include "RemapDMXChannelsDialog.h"
#include "xLightsApp.h"
#include "EffectPanelUtils.h"

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
const long DMXPanel::ID_STATICTEXT_DMX10 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX10 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX10 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX10 = wxNewId();
const long DMXPanel::ID_PANEL6 = wxNewId();
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
const long DMXPanel::ID_STATICTEXT_DMX19 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX19 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX19 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX19 = wxNewId();
const long DMXPanel::ID_STATICTEXT_DMX20 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX20 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX20 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX20 = wxNewId();
const long DMXPanel::ID_PANEL28 = wxNewId();
const long DMXPanel::ID_STATICTEXT_DMX21 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX21 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX21 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX21 = wxNewId();
const long DMXPanel::ID_STATICTEXT_DMX22 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX22 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX22 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX22 = wxNewId();
const long DMXPanel::ID_STATICTEXT_DMX23 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX23 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX23 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX23 = wxNewId();
const long DMXPanel::ID_STATICTEXT_DMX24 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX24 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX24 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX24 = wxNewId();
const long DMXPanel::ID_STATICTEXT_DMX25 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX25 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX25 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX25 = wxNewId();
const long DMXPanel::ID_STATICTEXT_DMX26 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX26 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX26 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX26 = wxNewId();
const long DMXPanel::ID_STATICTEXT_DMX27 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX27 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX27 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX27 = wxNewId();
const long DMXPanel::ID_STATICTEXT_DMX28 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX28 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX28 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX28 = wxNewId();
const long DMXPanel::ID_STATICTEXT_DMX29 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX29 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX29 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX29 = wxNewId();
const long DMXPanel::ID_STATICTEXT_DMX30 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX30 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX30 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX30 = wxNewId();
const long DMXPanel::ID_PANEL1 = wxNewId();
const long DMXPanel::ID_STATICTEXT_DMX31 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX31 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX31 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX31 = wxNewId();
const long DMXPanel::ID_STATICTEXT_DMX32 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX32 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX32 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX32 = wxNewId();
const long DMXPanel::ID_STATICTEXT_DMX33 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX33 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX33 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX33 = wxNewId();
const long DMXPanel::ID_STATICTEXT_DMX34 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX34 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX34 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX34 = wxNewId();
const long DMXPanel::ID_STATICTEXT_DMX35 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX35 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX35 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX35 = wxNewId();
const long DMXPanel::ID_STATICTEXT_DMX36 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX36 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX36 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX36 = wxNewId();
const long DMXPanel::ID_STATICTEXT_DMX37 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX37 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX37 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX37 = wxNewId();
const long DMXPanel::ID_STATICTEXT_DMX38 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX38 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX38 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX38 = wxNewId();
const long DMXPanel::ID_STATICTEXT_DMX39 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX39 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX39 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX39 = wxNewId();
const long DMXPanel::ID_STATICTEXT_DMX40 = wxNewId();
const long DMXPanel::ID_SLIDER_DMX40 = wxNewId();
const long DMXPanel::ID_VALUECURVE_DMX40 = wxNewId();
const long DMXPanel::IDD_TEXTCTRL_DMX40 = wxNewId();
const long DMXPanel::ID_PANEL3 = wxNewId();
const long DMXPanel::ID_NOTEBOOK1 = wxNewId();
const long DMXPanel::ID_BUTTON1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(DMXPanel,wxPanel)
	//(*EventTable(DMXPanel)
	//*)
END_EVENT_TABLE()

DMXPanel::DMXPanel(wxWindow* parent)
{
	//(*Initialize(DMXPanel)
	BulkEditTextCtrl* TextCtrl1_DMX1;
	BulkEditTextCtrl* TextCtrl1_DMX2;
	BulkEditTextCtrl* TextCtrl1_DMX3;
	BulkEditTextCtrl* TextCtrl1_DMX5;
	BulkEditTextCtrl* TextCtrl1_DMX6;
	BulkEditTextCtrl* TextCtrl1_DMX7;
	BulkEditTextCtrl* TextCtrl1_DMX8;
	BulkEditTextCtrl* TextCtrl_DMX10;
	BulkEditTextCtrl* TextCtrl_DMX11;
	BulkEditTextCtrl* TextCtrl_DMX12;
	BulkEditTextCtrl* TextCtrl_DMX13;
	BulkEditTextCtrl* TextCtrl_DMX14;
	BulkEditTextCtrl* TextCtrl_DMX15;
	BulkEditTextCtrl* TextCtrl_DMX16;
	BulkEditTextCtrl* TextCtrl_DMX17;
	BulkEditTextCtrl* TextCtrl_DMX18;
	BulkEditTextCtrl* TextCtrl_DMX19;
	BulkEditTextCtrl* TextCtrl_DMX20;
	BulkEditTextCtrl* TextCtrl_DMX21;
	BulkEditTextCtrl* TextCtrl_DMX22;
	BulkEditTextCtrl* TextCtrl_DMX23;
	BulkEditTextCtrl* TextCtrl_DMX24;
	BulkEditTextCtrl* TextCtrl_DMX25;
	BulkEditTextCtrl* TextCtrl_DMX26;
	BulkEditTextCtrl* TextCtrl_DMX27;
	BulkEditTextCtrl* TextCtrl_DMX28;
	BulkEditTextCtrl* TextCtrl_DMX29;
	BulkEditTextCtrl* TextCtrl_DMX30;
	BulkEditTextCtrl* TextCtrl_DMX31;
	BulkEditTextCtrl* TextCtrl_DMX32;
	BulkEditTextCtrl* TextCtrl_DMX33;
	BulkEditTextCtrl* TextCtrl_DMX34;
	BulkEditTextCtrl* TextCtrl_DMX35;
	BulkEditTextCtrl* TextCtrl_DMX36;
	BulkEditTextCtrl* TextCtrl_DMX37;
	BulkEditTextCtrl* TextCtrl_DMX38;
	BulkEditTextCtrl* TextCtrl_DMX39;
	BulkEditTextCtrl* TextCtrl_DMX40;
	BulkEditTextCtrl* TextCtrl_DMX4;
	BulkEditTextCtrl* TextCtrl_DMX9;

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
	ValueCurve_DMX1 = new BulkEditValueCurveButton(ChannelPanel1, ID_VALUECURVE_DMX1, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX1"));
	FlexGridSizer_Panel1->Add(ValueCurve_DMX1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl1_DMX1 = new BulkEditTextCtrl(ChannelPanel1, IDD_TEXTCTRL_DMX1, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel1,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX1"));
	FlexGridSizer_Panel1->Add(TextCtrl1_DMX1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX2 = new wxStaticText(ChannelPanel1, ID_STATICTEXT_DMX2, _("Channel 2:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX2"));
	FlexGridSizer_Panel1->Add(Label_DMX2, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX2 = new BulkEditSlider(ChannelPanel1, ID_SLIDER_DMX2, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX2"));
	FlexGridSizer_Panel1->Add(Slider_DMX2, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX2 = new BulkEditValueCurveButton(ChannelPanel1, ID_VALUECURVE_DMX2, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX2"));
	FlexGridSizer_Panel1->Add(ValueCurve_DMX2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl1_DMX2 = new BulkEditTextCtrl(ChannelPanel1, IDD_TEXTCTRL_DMX2, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel1,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX2"));
	FlexGridSizer_Panel1->Add(TextCtrl1_DMX2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX3 = new wxStaticText(ChannelPanel1, ID_STATICTEXT_DMX3, _("Channel 3:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX3"));
	FlexGridSizer_Panel1->Add(Label_DMX3, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX3 = new BulkEditSlider(ChannelPanel1, ID_SLIDER_DMX3, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX3"));
	FlexGridSizer_Panel1->Add(Slider_DMX3, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX3 = new BulkEditValueCurveButton(ChannelPanel1, ID_VALUECURVE_DMX3, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX3"));
	FlexGridSizer_Panel1->Add(ValueCurve_DMX3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl1_DMX3 = new BulkEditTextCtrl(ChannelPanel1, IDD_TEXTCTRL_DMX3, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel1,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX3"));
	FlexGridSizer_Panel1->Add(TextCtrl1_DMX3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX4 = new wxStaticText(ChannelPanel1, ID_STATICTEXT_DMX4, _("Channel 4:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX4"));
	FlexGridSizer_Panel1->Add(Label_DMX4, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX4 = new BulkEditSlider(ChannelPanel1, ID_SLIDER_DMX4, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX4"));
	FlexGridSizer_Panel1->Add(Slider_DMX4, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX4 = new BulkEditValueCurveButton(ChannelPanel1, ID_VALUECURVE_DMX4, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX4"));
	FlexGridSizer_Panel1->Add(ValueCurve_DMX4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_DMX4 = new BulkEditTextCtrl(ChannelPanel1, IDD_TEXTCTRL_DMX4, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel1,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX4"));
	FlexGridSizer_Panel1->Add(TextCtrl_DMX4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX5 = new wxStaticText(ChannelPanel1, ID_STATICTEXT_DMX5, _("Channel 5:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX5"));
	FlexGridSizer_Panel1->Add(Label_DMX5, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX5 = new BulkEditSlider(ChannelPanel1, ID_SLIDER_DMX5, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX5"));
	FlexGridSizer_Panel1->Add(Slider_DMX5, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX5 = new BulkEditValueCurveButton(ChannelPanel1, ID_VALUECURVE_DMX5, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX5"));
	FlexGridSizer_Panel1->Add(ValueCurve_DMX5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl1_DMX5 = new BulkEditTextCtrl(ChannelPanel1, IDD_TEXTCTRL_DMX5, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel1,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX5"));
	FlexGridSizer_Panel1->Add(TextCtrl1_DMX5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX6 = new wxStaticText(ChannelPanel1, ID_STATICTEXT_DMX6, _("Channel 6:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX6"));
	FlexGridSizer_Panel1->Add(Label_DMX6, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX6 = new BulkEditSlider(ChannelPanel1, ID_SLIDER_DMX6, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX6"));
	FlexGridSizer_Panel1->Add(Slider_DMX6, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX6 = new BulkEditValueCurveButton(ChannelPanel1, ID_VALUECURVE_DMX6, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX6"));
	FlexGridSizer_Panel1->Add(ValueCurve_DMX6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl1_DMX6 = new BulkEditTextCtrl(ChannelPanel1, IDD_TEXTCTRL_DMX6, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel1,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX6"));
	FlexGridSizer_Panel1->Add(TextCtrl1_DMX6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX7 = new wxStaticText(ChannelPanel1, ID_STATICTEXT_DMX7, _("Channel 7:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX7"));
	FlexGridSizer_Panel1->Add(Label_DMX7, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX7 = new BulkEditSlider(ChannelPanel1, ID_SLIDER_DMX7, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX7"));
	FlexGridSizer_Panel1->Add(Slider_DMX7, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX7 = new BulkEditValueCurveButton(ChannelPanel1, ID_VALUECURVE_DMX7, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX7"));
	FlexGridSizer_Panel1->Add(ValueCurve_DMX7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl1_DMX7 = new BulkEditTextCtrl(ChannelPanel1, IDD_TEXTCTRL_DMX7, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel1,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX7"));
	FlexGridSizer_Panel1->Add(TextCtrl1_DMX7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX8 = new wxStaticText(ChannelPanel1, ID_STATICTEXT_DMX8, _("Channel 8:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX8"));
	FlexGridSizer_Panel1->Add(Label_DMX8, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX8 = new BulkEditSlider(ChannelPanel1, ID_SLIDER_DMX8, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX8"));
	FlexGridSizer_Panel1->Add(Slider_DMX8, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX8 = new BulkEditValueCurveButton(ChannelPanel1, ID_VALUECURVE_DMX8, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX8"));
	FlexGridSizer_Panel1->Add(ValueCurve_DMX8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl1_DMX8 = new BulkEditTextCtrl(ChannelPanel1, IDD_TEXTCTRL_DMX8, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel1,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX8"));
	FlexGridSizer_Panel1->Add(TextCtrl1_DMX8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX9 = new wxStaticText(ChannelPanel1, ID_STATICTEXT_DMX9, _("Channel 9:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX9"));
	FlexGridSizer_Panel1->Add(Label_DMX9, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX9 = new BulkEditSlider(ChannelPanel1, ID_SLIDER_DMX9, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX9"));
	FlexGridSizer_Panel1->Add(Slider_DMX9, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX9 = new BulkEditValueCurveButton(ChannelPanel1, ID_VALUECURVE_DMX9, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX9"));
	FlexGridSizer_Panel1->Add(ValueCurve_DMX9, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_DMX9 = new BulkEditTextCtrl(ChannelPanel1, IDD_TEXTCTRL_DMX9, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel1,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX9"));
	FlexGridSizer_Panel1->Add(TextCtrl_DMX9, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX10 = new wxStaticText(ChannelPanel1, ID_STATICTEXT_DMX10, _("Channel 10:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX10"));
	FlexGridSizer_Panel1->Add(Label_DMX10, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX10 = new BulkEditSlider(ChannelPanel1, ID_SLIDER_DMX10, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX10"));
	FlexGridSizer_Panel1->Add(Slider_DMX10, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX10 = new BulkEditValueCurveButton(ChannelPanel1, ID_VALUECURVE_DMX10, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX10"));
	FlexGridSizer_Panel1->Add(ValueCurve_DMX10, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_DMX10 = new BulkEditTextCtrl(ChannelPanel1, IDD_TEXTCTRL_DMX10, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel1,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX10"));
	FlexGridSizer_Panel1->Add(TextCtrl_DMX10, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	ChannelPanel1->SetSizer(FlexGridSizer_Panel1);
	FlexGridSizer_Panel1->Fit(ChannelPanel1);
	FlexGridSizer_Panel1->SetSizeHints(ChannelPanel1);
	ChannelPanel2 = new wxPanel(Notebook7, ID_PANEL28, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL28"));
	FlexGridSizer_Panel2 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer_Panel2->AddGrowableCol(1);
	Label_DMX11 = new wxStaticText(ChannelPanel2, ID_STATICTEXT_DMX11, _("Channel 11:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX11"));
	FlexGridSizer_Panel2->Add(Label_DMX11, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX11 = new BulkEditSlider(ChannelPanel2, ID_SLIDER_DMX11, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX11"));
	FlexGridSizer_Panel2->Add(Slider_DMX11, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX11 = new BulkEditValueCurveButton(ChannelPanel2, ID_VALUECURVE_DMX11, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX11"));
	FlexGridSizer_Panel2->Add(ValueCurve_DMX11, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_DMX11 = new BulkEditTextCtrl(ChannelPanel2, IDD_TEXTCTRL_DMX11, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel2,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX11"));
	FlexGridSizer_Panel2->Add(TextCtrl_DMX11, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX12 = new wxStaticText(ChannelPanel2, ID_STATICTEXT_DMX12, _("Channel 12:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX12"));
	FlexGridSizer_Panel2->Add(Label_DMX12, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX12 = new BulkEditSlider(ChannelPanel2, ID_SLIDER_DMX12, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX12"));
	FlexGridSizer_Panel2->Add(Slider_DMX12, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX12 = new BulkEditValueCurveButton(ChannelPanel2, ID_VALUECURVE_DMX12, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX12"));
	FlexGridSizer_Panel2->Add(ValueCurve_DMX12, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_DMX12 = new BulkEditTextCtrl(ChannelPanel2, IDD_TEXTCTRL_DMX12, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel2,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX12"));
	FlexGridSizer_Panel2->Add(TextCtrl_DMX12, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX13 = new wxStaticText(ChannelPanel2, ID_STATICTEXT_DMX13, _("Channel 13:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX13"));
	FlexGridSizer_Panel2->Add(Label_DMX13, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX13 = new BulkEditSlider(ChannelPanel2, ID_SLIDER_DMX13, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX13"));
	FlexGridSizer_Panel2->Add(Slider_DMX13, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX13 = new BulkEditValueCurveButton(ChannelPanel2, ID_VALUECURVE_DMX13, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX13"));
	FlexGridSizer_Panel2->Add(ValueCurve_DMX13, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_DMX13 = new BulkEditTextCtrl(ChannelPanel2, IDD_TEXTCTRL_DMX13, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel2,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX13"));
	FlexGridSizer_Panel2->Add(TextCtrl_DMX13, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX14 = new wxStaticText(ChannelPanel2, ID_STATICTEXT_DMX14, _("Channel 14:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX14"));
	FlexGridSizer_Panel2->Add(Label_DMX14, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX14 = new BulkEditSlider(ChannelPanel2, ID_SLIDER_DMX14, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX14"));
	FlexGridSizer_Panel2->Add(Slider_DMX14, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX14 = new BulkEditValueCurveButton(ChannelPanel2, ID_VALUECURVE_DMX14, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX14"));
	FlexGridSizer_Panel2->Add(ValueCurve_DMX14, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_DMX14 = new BulkEditTextCtrl(ChannelPanel2, IDD_TEXTCTRL_DMX14, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel2,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX14"));
	FlexGridSizer_Panel2->Add(TextCtrl_DMX14, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX15 = new wxStaticText(ChannelPanel2, ID_STATICTEXT_DMX15, _("Channel 15:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX15"));
	FlexGridSizer_Panel2->Add(Label_DMX15, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX15 = new BulkEditSlider(ChannelPanel2, ID_SLIDER_DMX15, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX15"));
	FlexGridSizer_Panel2->Add(Slider_DMX15, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX15 = new BulkEditValueCurveButton(ChannelPanel2, ID_VALUECURVE_DMX15, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX15"));
	FlexGridSizer_Panel2->Add(ValueCurve_DMX15, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_DMX15 = new BulkEditTextCtrl(ChannelPanel2, IDD_TEXTCTRL_DMX15, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel2,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX15"));
	FlexGridSizer_Panel2->Add(TextCtrl_DMX15, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX16 = new wxStaticText(ChannelPanel2, ID_STATICTEXT_DMX16, _("Channel 16:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX16"));
	FlexGridSizer_Panel2->Add(Label_DMX16, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX16 = new BulkEditSlider(ChannelPanel2, ID_SLIDER_DMX16, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX16"));
	FlexGridSizer_Panel2->Add(Slider_DMX16, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX16 = new BulkEditValueCurveButton(ChannelPanel2, ID_VALUECURVE_DMX16, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX16"));
	FlexGridSizer_Panel2->Add(ValueCurve_DMX16, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_DMX16 = new BulkEditTextCtrl(ChannelPanel2, IDD_TEXTCTRL_DMX16, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel2,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX16"));
	FlexGridSizer_Panel2->Add(TextCtrl_DMX16, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX17 = new wxStaticText(ChannelPanel2, ID_STATICTEXT_DMX17, _("Channel 17:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX17"));
	FlexGridSizer_Panel2->Add(Label_DMX17, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX17 = new BulkEditSlider(ChannelPanel2, ID_SLIDER_DMX17, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX17"));
	FlexGridSizer_Panel2->Add(Slider_DMX17, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX17 = new BulkEditValueCurveButton(ChannelPanel2, ID_VALUECURVE_DMX17, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX17"));
	FlexGridSizer_Panel2->Add(ValueCurve_DMX17, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_DMX17 = new BulkEditTextCtrl(ChannelPanel2, IDD_TEXTCTRL_DMX17, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel2,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX17"));
	FlexGridSizer_Panel2->Add(TextCtrl_DMX17, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX18 = new wxStaticText(ChannelPanel2, ID_STATICTEXT_DMX18, _("Channel 18:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX18"));
	FlexGridSizer_Panel2->Add(Label_DMX18, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX18 = new BulkEditSlider(ChannelPanel2, ID_SLIDER_DMX18, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX18"));
	FlexGridSizer_Panel2->Add(Slider_DMX18, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX18 = new BulkEditValueCurveButton(ChannelPanel2, ID_VALUECURVE_DMX18, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX18"));
	FlexGridSizer_Panel2->Add(ValueCurve_DMX18, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_DMX18 = new BulkEditTextCtrl(ChannelPanel2, IDD_TEXTCTRL_DMX18, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel2,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX18"));
	FlexGridSizer_Panel2->Add(TextCtrl_DMX18, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX19 = new wxStaticText(ChannelPanel2, ID_STATICTEXT_DMX19, _("Channel 19:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX19"));
	FlexGridSizer_Panel2->Add(Label_DMX19, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX19 = new BulkEditSlider(ChannelPanel2, ID_SLIDER_DMX19, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX19"));
	FlexGridSizer_Panel2->Add(Slider_DMX19, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX19 = new BulkEditValueCurveButton(ChannelPanel2, ID_VALUECURVE_DMX19, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX19"));
	FlexGridSizer_Panel2->Add(ValueCurve_DMX19, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_DMX19 = new BulkEditTextCtrl(ChannelPanel2, IDD_TEXTCTRL_DMX19, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel2,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX19"));
	FlexGridSizer_Panel2->Add(TextCtrl_DMX19, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX20 = new wxStaticText(ChannelPanel2, ID_STATICTEXT_DMX20, _("Channel 20:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX20"));
	FlexGridSizer_Panel2->Add(Label_DMX20, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX20 = new BulkEditSlider(ChannelPanel2, ID_SLIDER_DMX20, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX20"));
	FlexGridSizer_Panel2->Add(Slider_DMX20, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX20 = new BulkEditValueCurveButton(ChannelPanel2, ID_VALUECURVE_DMX20, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX20"));
	FlexGridSizer_Panel2->Add(ValueCurve_DMX20, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_DMX20 = new BulkEditTextCtrl(ChannelPanel2, IDD_TEXTCTRL_DMX20, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel2,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX20"));
	FlexGridSizer_Panel2->Add(TextCtrl_DMX20, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	ChannelPanel2->SetSizer(FlexGridSizer_Panel2);
	FlexGridSizer_Panel2->Fit(ChannelPanel2);
	FlexGridSizer_Panel2->SetSizeHints(ChannelPanel2);
	ChannelPanel3 = new wxPanel(Notebook7, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	Label_DMX21 = new wxStaticText(ChannelPanel3, ID_STATICTEXT_DMX21, _("Channel 21:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX21"));
	FlexGridSizer1->Add(Label_DMX21, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX21 = new BulkEditSlider(ChannelPanel3, ID_SLIDER_DMX21, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX21"));
	FlexGridSizer1->Add(Slider_DMX21, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX21 = new BulkEditValueCurveButton(ChannelPanel3, ID_VALUECURVE_DMX21, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX21"));
	FlexGridSizer1->Add(ValueCurve_DMX21, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_DMX21 = new BulkEditTextCtrl(ChannelPanel3, IDD_TEXTCTRL_DMX21, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel3,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX21"));
	FlexGridSizer1->Add(TextCtrl_DMX21, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX22 = new wxStaticText(ChannelPanel3, ID_STATICTEXT_DMX22, _("Channel 22:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX22"));
	FlexGridSizer1->Add(Label_DMX22, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX22 = new BulkEditSlider(ChannelPanel3, ID_SLIDER_DMX22, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX22"));
	FlexGridSizer1->Add(Slider_DMX22, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX22 = new BulkEditValueCurveButton(ChannelPanel3, ID_VALUECURVE_DMX22, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX22"));
	FlexGridSizer1->Add(ValueCurve_DMX22, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_DMX22 = new BulkEditTextCtrl(ChannelPanel3, IDD_TEXTCTRL_DMX22, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel3,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX22"));
	FlexGridSizer1->Add(TextCtrl_DMX22, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX23 = new wxStaticText(ChannelPanel3, ID_STATICTEXT_DMX23, _("Channel 23:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX23"));
	FlexGridSizer1->Add(Label_DMX23, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX23 = new BulkEditSlider(ChannelPanel3, ID_SLIDER_DMX23, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX23"));
	FlexGridSizer1->Add(Slider_DMX23, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX23 = new BulkEditValueCurveButton(ChannelPanel3, ID_VALUECURVE_DMX23, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX23"));
	FlexGridSizer1->Add(ValueCurve_DMX23, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_DMX23 = new BulkEditTextCtrl(ChannelPanel3, IDD_TEXTCTRL_DMX23, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel3,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX23"));
	FlexGridSizer1->Add(TextCtrl_DMX23, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX24 = new wxStaticText(ChannelPanel3, ID_STATICTEXT_DMX24, _("Channel 24:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX24"));
	FlexGridSizer1->Add(Label_DMX24, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX24 = new BulkEditSlider(ChannelPanel3, ID_SLIDER_DMX24, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX24"));
	FlexGridSizer1->Add(Slider_DMX24, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX24 = new BulkEditValueCurveButton(ChannelPanel3, ID_VALUECURVE_DMX24, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX24"));
	FlexGridSizer1->Add(ValueCurve_DMX24, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_DMX24 = new BulkEditTextCtrl(ChannelPanel3, IDD_TEXTCTRL_DMX24, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel3,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX24"));
	FlexGridSizer1->Add(TextCtrl_DMX24, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX25 = new wxStaticText(ChannelPanel3, ID_STATICTEXT_DMX25, _("Channel 25:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX25"));
	FlexGridSizer1->Add(Label_DMX25, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX25 = new BulkEditSlider(ChannelPanel3, ID_SLIDER_DMX25, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX25"));
	FlexGridSizer1->Add(Slider_DMX25, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX25 = new BulkEditValueCurveButton(ChannelPanel3, ID_VALUECURVE_DMX25, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX25"));
	FlexGridSizer1->Add(ValueCurve_DMX25, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_DMX25 = new BulkEditTextCtrl(ChannelPanel3, IDD_TEXTCTRL_DMX25, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel3,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX25"));
	FlexGridSizer1->Add(TextCtrl_DMX25, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX26 = new wxStaticText(ChannelPanel3, ID_STATICTEXT_DMX26, _("Channel 26:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX26"));
	FlexGridSizer1->Add(Label_DMX26, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX26 = new BulkEditSlider(ChannelPanel3, ID_SLIDER_DMX26, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX26"));
	FlexGridSizer1->Add(Slider_DMX26, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX26 = new BulkEditValueCurveButton(ChannelPanel3, ID_VALUECURVE_DMX26, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX26"));
	FlexGridSizer1->Add(ValueCurve_DMX26, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_DMX26 = new BulkEditTextCtrl(ChannelPanel3, IDD_TEXTCTRL_DMX26, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel3,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX26"));
	FlexGridSizer1->Add(TextCtrl_DMX26, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX27 = new wxStaticText(ChannelPanel3, ID_STATICTEXT_DMX27, _("Channel 27:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX27"));
	FlexGridSizer1->Add(Label_DMX27, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX27 = new BulkEditSlider(ChannelPanel3, ID_SLIDER_DMX27, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX27"));
	FlexGridSizer1->Add(Slider_DMX27, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX27 = new BulkEditValueCurveButton(ChannelPanel3, ID_VALUECURVE_DMX27, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX27"));
	FlexGridSizer1->Add(ValueCurve_DMX27, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_DMX27 = new BulkEditTextCtrl(ChannelPanel3, IDD_TEXTCTRL_DMX27, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel3,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX27"));
	FlexGridSizer1->Add(TextCtrl_DMX27, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX28 = new wxStaticText(ChannelPanel3, ID_STATICTEXT_DMX28, _("Channel 28:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX28"));
	FlexGridSizer1->Add(Label_DMX28, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX28 = new BulkEditSlider(ChannelPanel3, ID_SLIDER_DMX28, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX28"));
	FlexGridSizer1->Add(Slider_DMX28, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX28 = new BulkEditValueCurveButton(ChannelPanel3, ID_VALUECURVE_DMX28, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX28"));
	FlexGridSizer1->Add(ValueCurve_DMX28, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_DMX28 = new BulkEditTextCtrl(ChannelPanel3, IDD_TEXTCTRL_DMX28, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel3,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX28"));
	FlexGridSizer1->Add(TextCtrl_DMX28, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX29 = new wxStaticText(ChannelPanel3, ID_STATICTEXT_DMX29, _("Channel 29:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX29"));
	FlexGridSizer1->Add(Label_DMX29, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX29 = new BulkEditSlider(ChannelPanel3, ID_SLIDER_DMX29, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX29"));
	FlexGridSizer1->Add(Slider_DMX29, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX29 = new BulkEditValueCurveButton(ChannelPanel3, ID_VALUECURVE_DMX29, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX29"));
	FlexGridSizer1->Add(ValueCurve_DMX29, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_DMX29 = new BulkEditTextCtrl(ChannelPanel3, IDD_TEXTCTRL_DMX29, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel3,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX29"));
	FlexGridSizer1->Add(TextCtrl_DMX29, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX30 = new wxStaticText(ChannelPanel3, ID_STATICTEXT_DMX30, _("Channel 30:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX30"));
	FlexGridSizer1->Add(Label_DMX30, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX30 = new BulkEditSlider(ChannelPanel3, ID_SLIDER_DMX30, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX30"));
	FlexGridSizer1->Add(Slider_DMX30, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX30 = new BulkEditValueCurveButton(ChannelPanel3, ID_VALUECURVE_DMX30, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX30"));
	FlexGridSizer1->Add(ValueCurve_DMX30, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_DMX30 = new BulkEditTextCtrl(ChannelPanel3, IDD_TEXTCTRL_DMX30, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel3,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX30"));
	FlexGridSizer1->Add(TextCtrl_DMX30, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	ChannelPanel3->SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(ChannelPanel3);
	FlexGridSizer1->SetSizeHints(ChannelPanel3);
	ChannelPanel4 = new wxPanel(Notebook7, ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL3"));
	FlexGridSizer2 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer2->AddGrowableCol(1);
	Label_DMX31 = new wxStaticText(ChannelPanel4, ID_STATICTEXT_DMX31, _("Channel 31:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX31"));
	FlexGridSizer2->Add(Label_DMX31, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX31 = new BulkEditSlider(ChannelPanel4, ID_SLIDER_DMX31, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX31"));
	FlexGridSizer2->Add(Slider_DMX31, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX31 = new BulkEditValueCurveButton(ChannelPanel4, ID_VALUECURVE_DMX31, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX31"));
	FlexGridSizer2->Add(ValueCurve_DMX31, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_DMX31 = new BulkEditTextCtrl(ChannelPanel4, IDD_TEXTCTRL_DMX31, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel4,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX31"));
	FlexGridSizer2->Add(TextCtrl_DMX31, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX32 = new wxStaticText(ChannelPanel4, ID_STATICTEXT_DMX32, _("Channel 32:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX32"));
	FlexGridSizer2->Add(Label_DMX32, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX32 = new BulkEditSlider(ChannelPanel4, ID_SLIDER_DMX32, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX32"));
	FlexGridSizer2->Add(Slider_DMX32, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX32 = new BulkEditValueCurveButton(ChannelPanel4, ID_VALUECURVE_DMX32, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX32"));
	FlexGridSizer2->Add(ValueCurve_DMX32, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_DMX32 = new BulkEditTextCtrl(ChannelPanel4, IDD_TEXTCTRL_DMX32, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel4,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX32"));
	FlexGridSizer2->Add(TextCtrl_DMX32, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX33 = new wxStaticText(ChannelPanel4, ID_STATICTEXT_DMX33, _("Channel 33:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX33"));
	FlexGridSizer2->Add(Label_DMX33, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX33 = new BulkEditSlider(ChannelPanel4, ID_SLIDER_DMX33, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX33"));
	FlexGridSizer2->Add(Slider_DMX33, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX33 = new BulkEditValueCurveButton(ChannelPanel4, ID_VALUECURVE_DMX33, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX33"));
	FlexGridSizer2->Add(ValueCurve_DMX33, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_DMX33 = new BulkEditTextCtrl(ChannelPanel4, IDD_TEXTCTRL_DMX33, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel4,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX33"));
	FlexGridSizer2->Add(TextCtrl_DMX33, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX34 = new wxStaticText(ChannelPanel4, ID_STATICTEXT_DMX34, _("Channel 34:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX34"));
	FlexGridSizer2->Add(Label_DMX34, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX34 = new BulkEditSlider(ChannelPanel4, ID_SLIDER_DMX34, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX34"));
	FlexGridSizer2->Add(Slider_DMX34, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX34 = new BulkEditValueCurveButton(ChannelPanel4, ID_VALUECURVE_DMX34, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX34"));
	FlexGridSizer2->Add(ValueCurve_DMX34, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_DMX34 = new BulkEditTextCtrl(ChannelPanel4, IDD_TEXTCTRL_DMX34, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel4,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX34"));
	FlexGridSizer2->Add(TextCtrl_DMX34, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX35 = new wxStaticText(ChannelPanel4, ID_STATICTEXT_DMX35, _("Channel 35:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX35"));
	FlexGridSizer2->Add(Label_DMX35, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX35 = new BulkEditSlider(ChannelPanel4, ID_SLIDER_DMX35, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX35"));
	FlexGridSizer2->Add(Slider_DMX35, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX35 = new BulkEditValueCurveButton(ChannelPanel4, ID_VALUECURVE_DMX35, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX35"));
	FlexGridSizer2->Add(ValueCurve_DMX35, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_DMX35 = new BulkEditTextCtrl(ChannelPanel4, IDD_TEXTCTRL_DMX35, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel4,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX35"));
	FlexGridSizer2->Add(TextCtrl_DMX35, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX36 = new wxStaticText(ChannelPanel4, ID_STATICTEXT_DMX36, _("Channel 36:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX36"));
	FlexGridSizer2->Add(Label_DMX36, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX36 = new BulkEditSlider(ChannelPanel4, ID_SLIDER_DMX36, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX36"));
	FlexGridSizer2->Add(Slider_DMX36, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX36 = new BulkEditValueCurveButton(ChannelPanel4, ID_VALUECURVE_DMX36, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX36"));
	FlexGridSizer2->Add(ValueCurve_DMX36, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_DMX36 = new BulkEditTextCtrl(ChannelPanel4, IDD_TEXTCTRL_DMX36, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel4,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX36"));
	FlexGridSizer2->Add(TextCtrl_DMX36, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX37 = new wxStaticText(ChannelPanel4, ID_STATICTEXT_DMX37, _("Channel 37:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX37"));
	FlexGridSizer2->Add(Label_DMX37, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX37 = new BulkEditSlider(ChannelPanel4, ID_SLIDER_DMX37, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX37"));
	FlexGridSizer2->Add(Slider_DMX37, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX37 = new BulkEditValueCurveButton(ChannelPanel4, ID_VALUECURVE_DMX37, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX37"));
	FlexGridSizer2->Add(ValueCurve_DMX37, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_DMX37 = new BulkEditTextCtrl(ChannelPanel4, IDD_TEXTCTRL_DMX37, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel4,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX37"));
	FlexGridSizer2->Add(TextCtrl_DMX37, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX38 = new wxStaticText(ChannelPanel4, ID_STATICTEXT_DMX38, _("Channel 38:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX38"));
	FlexGridSizer2->Add(Label_DMX38, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX38 = new BulkEditSlider(ChannelPanel4, ID_SLIDER_DMX38, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX38"));
	FlexGridSizer2->Add(Slider_DMX38, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX38 = new BulkEditValueCurveButton(ChannelPanel4, ID_VALUECURVE_DMX38, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX38"));
	FlexGridSizer2->Add(ValueCurve_DMX38, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_DMX38 = new BulkEditTextCtrl(ChannelPanel4, IDD_TEXTCTRL_DMX38, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel4,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX38"));
	FlexGridSizer2->Add(TextCtrl_DMX38, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX39 = new wxStaticText(ChannelPanel4, ID_STATICTEXT_DMX39, _("Channel 39:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX39"));
	FlexGridSizer2->Add(Label_DMX39, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX39 = new BulkEditSlider(ChannelPanel4, ID_SLIDER_DMX39, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX39"));
	FlexGridSizer2->Add(Slider_DMX39, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX39 = new BulkEditValueCurveButton(ChannelPanel4, ID_VALUECURVE_DMX39, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX39"));
	FlexGridSizer2->Add(ValueCurve_DMX39, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_DMX39 = new BulkEditTextCtrl(ChannelPanel4, IDD_TEXTCTRL_DMX39, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel4,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX39"));
	FlexGridSizer2->Add(TextCtrl_DMX39, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	Label_DMX40 = new wxStaticText(ChannelPanel4, ID_STATICTEXT_DMX40, _("Channel 40:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMX40"));
	FlexGridSizer2->Add(Label_DMX40, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_DMX40 = new BulkEditSlider(ChannelPanel4, ID_SLIDER_DMX40, 0, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMX40"));
	FlexGridSizer2->Add(Slider_DMX40, 1, wxALL|wxEXPAND, 2);
	ValueCurve_DMX40 = new BulkEditValueCurveButton(ChannelPanel4, ID_VALUECURVE_DMX40, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_DMX40"));
	FlexGridSizer2->Add(ValueCurve_DMX40, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_DMX40 = new BulkEditTextCtrl(ChannelPanel4, IDD_TEXTCTRL_DMX40, _("0"), wxDefaultPosition, wxDLG_UNIT(ChannelPanel4,wxSize(20,-1)), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("IDD_TEXTCTRL_DMX40"));
	FlexGridSizer2->Add(TextCtrl_DMX40, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	ChannelPanel4->SetSizer(FlexGridSizer2);
	FlexGridSizer2->Fit(ChannelPanel4);
	FlexGridSizer2->SetSizeHints(ChannelPanel4);
	Notebook7->AddPage(ChannelPanel1, _("Channels 1-10"), false);
	Notebook7->AddPage(ChannelPanel2, _("Channels 11-20"), false);
	Notebook7->AddPage(ChannelPanel3, _("Channels 21-30"), false);
	Notebook7->AddPage(ChannelPanel4, _("Channels 31-40"), false);
	FlexGridSizer_Main->Add(Notebook7, 1, wxALL|wxEXPAND, 2);
	ButtonRemap = new wxButton(this, ID_BUTTON1, _("Remap Channels"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer_Main->Add(ButtonRemap, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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
	Connect(ID_VALUECURVE_DMX19,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_DMX20,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_DMX21,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_DMX22,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_DMX23,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_DMX24,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_DMX25,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_DMX26,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_DMX27,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_DMX28,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_DMX29,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_DMX30,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_DMX31,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_DMX32,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_DMX33,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_DMX34,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_DMX35,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_DMX36,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_DMX37,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_DMX38,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_DMX39,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_DMX40,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnVCButtonClick);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPanel::OnButtonRemapClick);
	//*)
    SetName("ID_PANEL_DMX");

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&DMXPanel::OnVCChanged, 0, this);
    Connect(ID_BUTTON1, wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&DMXPanel::OnButtonRemapRClick);

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
    ValueCurve_DMX19->GetValue()->SetLimits(DMX_MIN, DMX_MAX);
    ValueCurve_DMX20->GetValue()->SetLimits(DMX_MIN, DMX_MAX);
    ValueCurve_DMX21->GetValue()->SetLimits(DMX_MIN, DMX_MAX);
    ValueCurve_DMX22->GetValue()->SetLimits(DMX_MIN, DMX_MAX);
    ValueCurve_DMX23->GetValue()->SetLimits(DMX_MIN, DMX_MAX);
    ValueCurve_DMX24->GetValue()->SetLimits(DMX_MIN, DMX_MAX);
    ValueCurve_DMX25->GetValue()->SetLimits(DMX_MIN, DMX_MAX);
    ValueCurve_DMX26->GetValue()->SetLimits(DMX_MIN, DMX_MAX);
    ValueCurve_DMX27->GetValue()->SetLimits(DMX_MIN, DMX_MAX);
    ValueCurve_DMX28->GetValue()->SetLimits(DMX_MIN, DMX_MAX);
    ValueCurve_DMX29->GetValue()->SetLimits(DMX_MIN, DMX_MAX);
    ValueCurve_DMX30->GetValue()->SetLimits(DMX_MIN, DMX_MAX);
    ValueCurve_DMX31->GetValue()->SetLimits(DMX_MIN, DMX_MAX);
    ValueCurve_DMX32->GetValue()->SetLimits(DMX_MIN, DMX_MAX);
    ValueCurve_DMX33->GetValue()->SetLimits(DMX_MIN, DMX_MAX);
    ValueCurve_DMX34->GetValue()->SetLimits(DMX_MIN, DMX_MAX);
    ValueCurve_DMX35->GetValue()->SetLimits(DMX_MIN, DMX_MAX);
    ValueCurve_DMX36->GetValue()->SetLimits(DMX_MIN, DMX_MAX);
    ValueCurve_DMX37->GetValue()->SetLimits(DMX_MIN, DMX_MAX);
    ValueCurve_DMX38->GetValue()->SetLimits(DMX_MIN, DMX_MAX);
    ValueCurve_DMX39->GetValue()->SetLimits(DMX_MIN, DMX_MAX);
    ValueCurve_DMX40->GetValue()->SetLimits(DMX_MIN, DMX_MAX);
}

DMXPanel::~DMXPanel()
{
	//(*Destroy(DMXPanel)
	//*)
}

PANEL_EVENT_HANDLERS(DMXPanel)

void DMXPanel::OnButtonRemapClick(wxCommandEvent& event)
{
    RemapDMXChannelsDialog dlg(this);
    if (dlg.ShowModal() == wxID_OK)
    {
        std::vector<int> sliders(40);
        std::vector<std::string> curves(40);

        // save the current values
        for (int i = 0; i < 40; i++)
        {
            wxString slider_ctrl = wxString::Format("ID_SLIDER_DMX%d", i+1);
            wxSlider* slider = (wxSlider*)(this->FindWindowByName(slider_ctrl));
            wxASSERT(slider != nullptr);
            wxString vc_ctrl = wxString::Format("ID_VALUECURVE_DMX%d", i+1);
            ValueCurveButton* curve = (ValueCurveButton*)(this->FindWindowByName(vc_ctrl));
            wxASSERT(curve != nullptr);
            sliders[i] = slider->GetValue();
            curves[i] = curve->GetValue()->Serialise();
        }

        for (int i = 0; i < 40; i++)
        {
            if (dlg.Grid1->GetCellValue(i, 0) != dlg.Grid1->GetCellValue(i, 1))
            {
                int from = i;
                int to = wxAtoi(dlg.Grid1->GetCellValue(i, 1).AfterLast(' '));
                wxString slider_ctrl = wxString::Format("ID_SLIDER_DMX%d", to);
                wxSlider* slider = (wxSlider*)(this->FindWindowByName(slider_ctrl));
                wxASSERT(slider != nullptr);
                wxString vc_ctrl = wxString::Format("ID_VALUECURVE_DMX%d", to);
                ValueCurveButton* curve = (ValueCurveButton*)(this->FindWindowByName(vc_ctrl));
                wxASSERT(curve != nullptr);
                wxString text_ctrl = wxString::Format("IDD_TEXTCTRL_DMX%d", to);
                wxTextCtrl* text = (wxTextCtrl*)(this->FindWindowByName(text_ctrl));
                wxASSERT(text != nullptr);

                text->SetValue(wxString::Format("%d", sliders[from]));
                slider->SetValue(sliders[from]);
                curve->GetValue()->Deserialise(curves[from]);
                wxCommandEvent vcevent;
                vcevent.SetEventObject(curve);
                OnVCChanged(vcevent);
                curve->UpdateState();
            }
        }
    }
}

void DMXPanel::OnButtonRemapRClick(wxCommandEvent& event)
{
    if (xLightsApp::GetFrame()->GetMainSequencer() == nullptr) {
        return;
    }

    // i should only display the menu if at least one effect is selected
    int alleffects = xLightsApp::GetFrame()->GetMainSequencer()->GetSelectedEffectCount("");
    if (alleffects < 1)
    {
        return;
    }

    wxMenu mnu;
    mnu.Append(wxID_ANY, "Bulk Edit");
    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&DMXPanel::OnChoicePopup, nullptr, this);
    PopupMenu(&mnu);
}

void DMXPanel::OnChoicePopup(wxCommandEvent& event)
{
    RemapDMXChannelsDialog dlg(this);
    if (dlg.ShowModal() == wxID_OK)
    {
        std::vector<std::pair<int, int>> fromto;

        for (int i = 0; i < 40; i++)
        {
            if (dlg.Grid1->GetCellValue(i, 0) != dlg.Grid1->GetCellValue(i, 1))
            {
                fromto.push_back({ i + 1,wxAtoi(dlg.Grid1->GetCellValue(i, 1).AfterLast(' ')) });
            }
        }

        if (fromto.size() > 0)
        {
            xLightsApp::GetFrame()->GetMainSequencer()->RemapSelectedDMXEffectValues(fromto);
            // unselect and select effect to update the panel
            auto effect = xLightsApp::GetFrame()->GetMainSequencer()->GetSelectedEffect();
            if (effect != nullptr)
            {
                xLightsApp::GetFrame()->GetMainSequencer()->PanelEffectGrid->RaiseSelectedEffectChanged(effect, true, true);
            }
        }
    }
}