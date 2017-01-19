#include "PlayListItemRDSPanel.h"

//(*InternalHeaders(PlayListItemRDSPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(PlayListItemRDSPanel)
const long PlayListItemRDSPanel::ID_STATICTEXT1 = wxNewId();
const long PlayListItemRDSPanel::ID_CHOICE1 = wxNewId();
const long PlayListItemRDSPanel::ID_STATICTEXT2 = wxNewId();
const long PlayListItemRDSPanel::ID_TEXTCTRL1 = wxNewId();
const long PlayListItemRDSPanel::ID_STATICTEXT3 = wxNewId();
const long PlayListItemRDSPanel::ID_SPINCTRL1 = wxNewId();
const long PlayListItemRDSPanel::ID_STATICTEXT4 = wxNewId();
const long PlayListItemRDSPanel::ID_CHOICE2 = wxNewId();
const long PlayListItemRDSPanel::ID_STATICTEXT5 = wxNewId();
const long PlayListItemRDSPanel::ID_TEXTCTRL2 = wxNewId();
const long PlayListItemRDSPanel::ID_STATICTEXT6 = wxNewId();
const long PlayListItemRDSPanel::ID_SPINCTRL2 = wxNewId();
const long PlayListItemRDSPanel::ID_CHECKBOX1 = wxNewId();
const long PlayListItemRDSPanel::ID_STATICTEXT7 = wxNewId();
const long PlayListItemRDSPanel::ID_SPINCTRL3 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PlayListItemRDSPanel,wxPanel)
	//(*EventTable(PlayListItemRDSPanel)
	//*)
END_EVENT_TABLE()

PlayListItemRDSPanel::PlayListItemRDSPanel(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(PlayListItemRDSPanel)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxDefaultPosition, wxSize(396,287), wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	FlexGridSizer1->AddGrowableRow(4);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Serial Port"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_CommPort = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	FlexGridSizer1->Add(Choice_CommPort, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Station Name"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_StationName = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	TextCtrl_StationName->SetMaxLength(8);
	FlexGridSizer1->Add(TextCtrl_StationName, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Station Duration"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_StationDuration = new wxSpinCtrl(this, ID_SPINCTRL1, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 0, 255, 0, _T("ID_SPINCTRL1"));
	SpinCtrl_StationDuration->SetValue(_T("0"));
	FlexGridSizer1->Add(SpinCtrl_StationDuration, 1, wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Scrolling Mode"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_ScrollMode = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
	FlexGridSizer1->Add(Choice_ScrollMode, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Text"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer1->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Text = new wxTextCtrl(this, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	TextCtrl_Text->SetMaxLength(80);
	FlexGridSizer1->Add(TextCtrl_Text, 1, wxALL|wxEXPAND, 5);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Text Duration"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer1->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_TextDuration = new wxSpinCtrl(this, ID_SPINCTRL2, _T("255"), wxDefaultPosition, wxDefaultSize, 0, 0, 255, 255, _T("ID_SPINCTRL2"));
	SpinCtrl_TextDuration->SetValue(_T("255"));
	FlexGridSizer1->Add(SpinCtrl_TextDuration, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_HighSpeed = new wxCheckBox(this, ID_CHECKBOX1, _("High speed scrolling"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_HighSpeed->SetValue(false);
	FlexGridSizer1->Add(CheckBox_HighSpeed, 1, wxALL|wxEXPAND, 5);
	StaticText7 = new wxStaticText(this, ID_STATICTEXT7, _("Line Duration"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	FlexGridSizer1->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_LineDuration = new wxSpinCtrl(this, ID_SPINCTRL3, _T("2"), wxDefaultPosition, wxDefaultSize, 0, 0, 255, 2, _T("ID_SPINCTRL3"));
	SpinCtrl_LineDuration->SetValue(_T("2"));
	FlexGridSizer1->Add(SpinCtrl_LineDuration, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	SetSizer(FlexGridSizer1);
	Layout();

	Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&PlayListItemRDSPanel::OnChoice_ScrollModeSelect);
	Connect(ID_CHOICE2,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&PlayListItemRDSPanel::OnChoice_ScrollModeSelect);
	//*)
}

PlayListItemRDSPanel::~PlayListItemRDSPanel()
{
	//(*Destroy(PlayListItemRDSPanel)
	//*)
}


void PlayListItemRDSPanel::OnChoice_ScrollModeSelect(wxCommandEvent& event)
{
}
