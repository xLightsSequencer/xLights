#include "TimingPanel.h"
#include "../include/padlock16x16-green.xpm" //-DJ
#include "../include/padlock16x16-red.xpm" //-DJ
#include "../include/padlock16x16-blue.xpm" //-DJ

//(*InternalHeaders(TimingPanel)
#include <wx/bitmap.h>
#include <wx/settings.h>
#include <wx/intl.h>
#include <wx/image.h>
#include <wx/string.h>
//*)

//(*IdInit(TimingPanel)
const long TimingPanel::ID_STATICTEXT2 = wxNewId();
const long TimingPanel::ID_TEXTCTRL_Fadein = wxNewId();
const long TimingPanel::ID_BITMAPBUTTON_TEXTCTRL_Fadein = wxNewId();
const long TimingPanel::ID_STATICTEXT3 = wxNewId();
const long TimingPanel::ID_TEXTCTRL_Fadeout = wxNewId();
const long TimingPanel::ID_BITMAPBUTTON_TEXTCTRL_Fadeout = wxNewId();
const long TimingPanel::ID_CHECKBOX_FitToTime = wxNewId();
const long TimingPanel::ID_BITMAPBUTTON_CHECKBOX_FitToTime = wxNewId();
const long TimingPanel::ID_CHECKBOX_OverlayBkg = wxNewId();
const long TimingPanel::ID_BITMAPBUTTON_OverlayBkg = wxNewId();
const long TimingPanel::ID_STATICTEXT1 = wxNewId();
const long TimingPanel::ID_SLIDER_Speed = wxNewId();
const long TimingPanel::ID_BITMAPBUTTON_SLIDER_Speed = wxNewId();
const long TimingPanel::ID_TEXTCTRL_Speed = wxNewId();
//*)

BEGIN_EVENT_TABLE(TimingPanel,wxPanel)
	//(*EventTable(TimingPanel)
	//*)
END_EVENT_TABLE()

TimingPanel::TimingPanel(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(TimingPanel)
	wxFlexGridSizer* FlexGridSizer23;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer22;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer5 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer22 = new wxFlexGridSizer(0, 3, 0, 0);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Fade In"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer22->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_Fadein = new wxTextCtrl(this, ID_TEXTCTRL_Fadein, _("0.00"), wxDefaultPosition, wxSize(37,20), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Fadein"));
	TextCtrl_Fadein->SetMaxLength(4);
	FlexGridSizer22->Add(TextCtrl_Fadein, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_FadeOut = new wxBitmapButton(this, ID_BITMAPBUTTON_TEXTCTRL_Fadein, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(21,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_TEXTCTRL_Fadein"));
	BitmapButton_FadeOut->SetDefault();
	BitmapButton_FadeOut->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer22->Add(BitmapButton_FadeOut, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Fade Out"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer22->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_Fadeout = new wxTextCtrl(this, ID_TEXTCTRL_Fadeout, _("0.00"), wxDefaultPosition, wxSize(37,20), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Fadeout"));
	TextCtrl_Fadeout->SetMaxLength(4);
	FlexGridSizer22->Add(TextCtrl_Fadeout, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_FadeIn = new wxBitmapButton(this, ID_BITMAPBUTTON_TEXTCTRL_Fadeout, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(21,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_TEXTCTRL_Fadeout"));
	BitmapButton_FadeIn->SetDefault();
	BitmapButton_FadeIn->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer22->Add(BitmapButton_FadeIn, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer5->Add(FlexGridSizer22, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer23 = new wxFlexGridSizer(0, 3, 0, 0);
	CheckBox_FitToTime = new wxCheckBox(this, ID_CHECKBOX_FitToTime, _("Fit to time"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_FitToTime"));
	CheckBox_FitToTime->SetValue(false);
	FlexGridSizer23->Add(CheckBox_FitToTime, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer23->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	BitmapButton_FitToTime = new wxBitmapButton(this, ID_BITMAPBUTTON_CHECKBOX_FitToTime, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(21,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_FitToTime"));
	BitmapButton_FitToTime->SetDefault();
	BitmapButton_FitToTime->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer23->Add(BitmapButton_FitToTime, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	CheckBox_OverlayBkg = new wxCheckBox(this, ID_CHECKBOX_OverlayBkg, _("Persistent"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_OverlayBkg"));
	CheckBox_OverlayBkg->SetValue(false);
	FlexGridSizer23->Add(CheckBox_OverlayBkg, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer23->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_OverlayBkg = new wxBitmapButton(this, ID_BITMAPBUTTON_OverlayBkg, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(21,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_OverlayBkg"));
	BitmapButton_OverlayBkg->SetDefault();
	BitmapButton_OverlayBkg->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer23->Add(BitmapButton_OverlayBkg, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer5->Add(FlexGridSizer23, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer1->Add(FlexGridSizer5, 2, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
	StaticText59 = new wxStaticText(this, ID_STATICTEXT1, _("Speed"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer3->Add(StaticText59, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Speed = new wxSlider(this, ID_SLIDER_Speed, 10, 1, 30, wxDefaultPosition, wxSize(30,100), wxSL_VERTICAL|wxSL_INVERSE, wxDefaultValidator, _T("ID_SLIDER_Speed"));
	FlexGridSizer3->Add(Slider_Speed, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Speed = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Speed, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Speed"));
	BitmapButton_Speed->SetDefault();
	BitmapButton_Speed->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer3->Add(BitmapButton_Speed, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_Speed = new wxTextCtrl(this, ID_TEXTCTRL_Speed, _("10"), wxDefaultPosition, wxSize(30,20), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_Speed"));
	FlexGridSizer3->Add(TextCtrl_Speed, 1, wxBOTTOM|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer1->Add(FlexGridSizer3, 2, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_BITMAPBUTTON_TEXTCTRL_Fadein,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TimingPanel::OnBitmapButton_FadeOutClick);
	Connect(ID_BITMAPBUTTON_TEXTCTRL_Fadeout,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TimingPanel::OnBitmapButton_FadeInClick);
	Connect(ID_CHECKBOX_FitToTime,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&TimingPanel::OnCheckBox_FitToTimeClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_FitToTime,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TimingPanel::OnBitmapButton_FitToTimeClick);
	Connect(ID_CHECKBOX_OverlayBkg,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&TimingPanel::OnCheckBox_OverlayBkgClick);
	Connect(ID_BITMAPBUTTON_OverlayBkg,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TimingPanel::OnBitmapButton_OverlayBkgClick);
	Connect(ID_SLIDER_Speed,wxEVT_SCROLL_TOP|wxEVT_SCROLL_BOTTOM|wxEVT_SCROLL_LINEUP|wxEVT_SCROLL_LINEDOWN|wxEVT_SCROLL_PAGEUP|wxEVT_SCROLL_PAGEDOWN|wxEVT_SCROLL_THUMBTRACK|wxEVT_SCROLL_THUMBRELEASE|wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&TimingPanel::OnSlider_SpeedCmdScroll);
	Connect(ID_SLIDER_Speed,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&TimingPanel::OnSlider_SpeedCmdScroll);
	//*)
}

TimingPanel::~TimingPanel()
{
	//(*Destroy(TimingPanel)
	//*)
}


void TimingPanel::OnSlider_SpeedCmdScroll(wxScrollEvent& event)
{
    UpdateSpeedText();
}

void TimingPanel::UpdateSpeedText()
{
    TextCtrl_Speed->SetValue(wxString::Format("%d",Slider_Speed->GetValue()));
}

void TimingPanel::OnCheckBox_FitToTimeClick(wxCommandEvent& event)
{
}

void TimingPanel::OnCheckBox_OverlayBkgClick(wxCommandEvent& event)
{
}

void TimingPanel::OnBitmapButton_FadeOutClick(wxCommandEvent& event)
{
}

void TimingPanel::OnBitmapButton_FadeInClick(wxCommandEvent& event)
{
}

void TimingPanel::OnBitmapButton_FitToTimeClick(wxCommandEvent& event)
{
}

void TimingPanel::OnBitmapButton_OverlayBkgClick(wxCommandEvent& event)
{
}
