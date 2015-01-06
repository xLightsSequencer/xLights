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
const long TimingPanel::ID_STATICTEXT1 = wxNewId();
const long TimingPanel::ID_SLIDER_Speed = wxNewId();
const long TimingPanel::ID_TEXTCTRL_Speed = wxNewId();
const long TimingPanel::ID_BITMAPBUTTON_SLIDER_Speed = wxNewId();
const long TimingPanel::ID_STATICTEXT4 = wxNewId();
const long TimingPanel::ID_CHECKBOX_LayerMorph = wxNewId();
const long TimingPanel::ID_BITMAPBUTTON_CHECKBOX_LayerMorph = wxNewId();
const long TimingPanel::ID_CHOICE_LayerMethod = wxNewId();
const long TimingPanel::ID_SLIDER_EffectLayerMix = wxNewId();
const long TimingPanel::ID_TEXTCTRL_LayerMix = wxNewId();
const long TimingPanel::ID_BITMAPBUTTON_SLIDER_EffectLayerMix = wxNewId();
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
//*)

BEGIN_EVENT_TABLE(TimingPanel,wxPanel)
	//(*EventTable(TimingPanel)
	//*)
END_EVENT_TABLE()

TimingPanel::TimingPanel(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(TimingPanel)
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer22;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer5 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 4, 0, 0);
	StaticText59 = new wxStaticText(this, ID_STATICTEXT1, _("Speed"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer2->Add(StaticText59, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Speed = new wxSlider(this, ID_SLIDER_Speed, 10, 1, 30, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL, wxDefaultValidator, _T("ID_SLIDER_Speed"));
	FlexGridSizer2->Add(Slider_Speed, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_Speed = new wxTextCtrl(this, ID_TEXTCTRL_Speed, _("10"), wxDefaultPosition, wxSize(30,20), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_Speed"));
	FlexGridSizer2->Add(TextCtrl_Speed, 1, wxBOTTOM|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Speed = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Speed, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Speed"));
	BitmapButton_Speed->SetDefault();
	BitmapButton_Speed->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer2->Add(BitmapButton_Speed, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT4, _("Blending:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer2->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer2->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer2->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	CheckBox_LayerMorph = new wxCheckBox(this, ID_CHECKBOX_LayerMorph, _("Morph"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_LayerMorph"));
	CheckBox_LayerMorph->SetValue(false);
	CheckBox_LayerMorph->SetToolTip(_("Gradual cross-fade from Effect1 to Effect2"));
	FlexGridSizer2->Add(CheckBox_LayerMorph, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer2->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_CheckBox_LayerMorph = new wxBitmapButton(this, ID_BITMAPBUTTON_CHECKBOX_LayerMorph, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_LayerMorph"));
	BitmapButton_CheckBox_LayerMorph->SetDefault();
	BitmapButton_CheckBox_LayerMorph->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	BitmapButton_CheckBox_LayerMorph->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	FlexGridSizer2->Add(BitmapButton_CheckBox_LayerMorph, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_LayerMethod = new wxChoice(this, ID_CHOICE_LayerMethod, wxDefaultPosition, wxDefaultSize, 0, 0, wxFULL_REPAINT_ON_RESIZE, wxDefaultValidator, _T("ID_CHOICE_LayerMethod"));
	Choice_LayerMethod->SetSelection( Choice_LayerMethod->Append(_("Effect 1")) );
	Choice_LayerMethod->Append(_("Effect 2"));
	Choice_LayerMethod->Append(_("1 is Mask"));
	Choice_LayerMethod->Append(_("2 is Mask"));
	Choice_LayerMethod->Append(_("1 is Unmask"));
	Choice_LayerMethod->Append(_("2 is Unmask"));
	Choice_LayerMethod->Append(_("1 reveals 2"));
	Choice_LayerMethod->Append(_("2 reveals 1"));
	Choice_LayerMethod->Append(_("Layered"));
	Choice_LayerMethod->Append(_("Average"));
	Choice_LayerMethod->Append(_("Bottom-Top"));
	Choice_LayerMethod->Append(_("Left-Right"));
	Choice_LayerMethod->SetToolTip(_("Layering defines how Effect 1 and Effect 2 will be mixed together.\nHere are the Choices\n* Effect 1: Shows only Effect 1. Slide the slider to the right to blend in some Effect 2. \n* Effect 2: Shows only Effect 2. Slide the slider to the right to blend in some Effect 1.\n* 1 is Mask: (Shadow) Effect 1 will cast a shadow onto Effect 2 for every Effect 1 pixel that has a non-black value.\n* 2 is Mask: (Shadow) Effect 2 will cast a shadow onto Effect 1 for every Effect 2 pixel that has a non-black value.\n* 1 is UnMask:  (Mask) Only allow Effect 2 to show through when Effect 1 has a non-black pixel.\n* 2 is UnMask:  (Mask) Only allow Effect 1 to show through when Effect 2 has a non-black pixel.\n* 1 reveals 2: (Superimpose) Effect 1 reveals Effect 2\n* 2 reveals 1: (Superimpose) Effect 2 reveals Effect 1\n* Average: Take value of Effect  and Add it to Value from Effect 2. Average the sum\n* Bottom-top: Effect 1 is put on bottom of model, Effect 2 is put on top in a plit screen display\n* Left-Right: Effect goes 1 goes on the left side, Effect 2 on the right. Split screen goes down middle of model."));
	FlexGridSizer2->Add(Choice_LayerMethod, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	Slider_EffectLayerMix = new wxSlider(this, ID_SLIDER_EffectLayerMix, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_EffectLayerMix"));
	FlexGridSizer2->Add(Slider_EffectLayerMix, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 1);
	txtCtlEffectMix = new wxTextCtrl(this, ID_TEXTCTRL_LayerMix, _("0"), wxDefaultPosition, wxSize(32,20), wxTE_PROCESS_ENTER|wxTAB_TRAVERSAL, wxDefaultValidator, _T("ID_TEXTCTRL_LayerMix"));
	FlexGridSizer2->Add(txtCtlEffectMix, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	BitmapButton_EffectLayerMix = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_EffectLayerMix, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_EffectLayerMix"));
	BitmapButton_EffectLayerMix->SetDefault();
	BitmapButton_EffectLayerMix->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	BitmapButton_EffectLayerMix->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	FlexGridSizer2->Add(BitmapButton_EffectLayerMix, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer5->Add(FlexGridSizer2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer22 = new wxFlexGridSizer(0, 4, 0, 0);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Fade In"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer22->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_Fadein = new wxTextCtrl(this, ID_TEXTCTRL_Fadein, _("0.00"), wxDefaultPosition, wxSize(37,20), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Fadein"));
	TextCtrl_Fadein->SetMaxLength(4);
	FlexGridSizer22->Add(TextCtrl_Fadein, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer22->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_FadeOut = new wxBitmapButton(this, ID_BITMAPBUTTON_TEXTCTRL_Fadein, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(21,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_TEXTCTRL_Fadein"));
	BitmapButton_FadeOut->SetDefault();
	BitmapButton_FadeOut->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer22->Add(BitmapButton_FadeOut, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Fade Out"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer22->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_Fadeout = new wxTextCtrl(this, ID_TEXTCTRL_Fadeout, _("0.00"), wxDefaultPosition, wxSize(37,20), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Fadeout"));
	TextCtrl_Fadeout->SetMaxLength(4);
	FlexGridSizer22->Add(TextCtrl_Fadeout, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer22->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_FadeIn = new wxBitmapButton(this, ID_BITMAPBUTTON_TEXTCTRL_Fadeout, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(21,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_TEXTCTRL_Fadeout"));
	BitmapButton_FadeIn->SetDefault();
	BitmapButton_FadeIn->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer22->Add(BitmapButton_FadeIn, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	CheckBox_FitToTime = new wxCheckBox(this, ID_CHECKBOX_FitToTime, _("Fit to time"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_FitToTime"));
	CheckBox_FitToTime->SetValue(false);
	FlexGridSizer22->Add(CheckBox_FitToTime, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer22->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer22->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	BitmapButton_FitToTime = new wxBitmapButton(this, ID_BITMAPBUTTON_CHECKBOX_FitToTime, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(21,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_FitToTime"));
	BitmapButton_FitToTime->SetDefault();
	BitmapButton_FitToTime->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer22->Add(BitmapButton_FitToTime, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	CheckBox_OverlayBkg = new wxCheckBox(this, ID_CHECKBOX_OverlayBkg, _("Persistent"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_OverlayBkg"));
	CheckBox_OverlayBkg->SetValue(false);
	FlexGridSizer22->Add(CheckBox_OverlayBkg, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer22->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer22->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_OverlayBkg = new wxBitmapButton(this, ID_BITMAPBUTTON_OverlayBkg, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(21,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_OverlayBkg"));
	BitmapButton_OverlayBkg->SetDefault();
	BitmapButton_OverlayBkg->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer22->Add(BitmapButton_OverlayBkg, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer5->Add(FlexGridSizer22, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer1->Add(FlexGridSizer5, 2, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_SLIDER_Speed,wxEVT_SCROLL_TOP|wxEVT_SCROLL_BOTTOM|wxEVT_SCROLL_LINEUP|wxEVT_SCROLL_LINEDOWN|wxEVT_SCROLL_PAGEUP|wxEVT_SCROLL_PAGEDOWN|wxEVT_SCROLL_THUMBTRACK|wxEVT_SCROLL_THUMBRELEASE|wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&TimingPanel::OnSlider_SpeedCmdScroll);
	Connect(ID_SLIDER_Speed,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&TimingPanel::OnSlider_SpeedCmdScroll);
	Connect(ID_BITMAPBUTTON_TEXTCTRL_Fadein,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TimingPanel::OnBitmapButton_FadeOutClick);
	Connect(ID_BITMAPBUTTON_TEXTCTRL_Fadeout,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TimingPanel::OnBitmapButton_FadeInClick);
	Connect(ID_CHECKBOX_FitToTime,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&TimingPanel::OnCheckBox_FitToTimeClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_FitToTime,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TimingPanel::OnBitmapButton_FitToTimeClick);
	Connect(ID_CHECKBOX_OverlayBkg,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&TimingPanel::OnCheckBox_OverlayBkgClick);
	Connect(ID_BITMAPBUTTON_OverlayBkg,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TimingPanel::OnBitmapButton_OverlayBkgClick);
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

void TimingPanel::OnResize(wxSizeEvent& event)
{


}
