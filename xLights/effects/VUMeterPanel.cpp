#include "VUMeterPanel.h"
#include "../../include/padlock16x16-blue.xpm"
#include "EffectPanelUtils.h"

//(*InternalHeaders(VUMeterPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/slider.h>
#include <wx/settings.h>
#include <wx/choice.h>
#include <wx/bmpbuttn.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(VUMeterPanel)
const long VUMeterPanel::ID_STATICTEXT1 = wxNewId();
const long VUMeterPanel::IDD_SLIDER_VUMeter_Bars = wxNewId();
const long VUMeterPanel::ID_TEXTCTRL_VUMeter_Bars = wxNewId();
const long VUMeterPanel::ID_BITMAPBUTTON_CHOICE_VUMeter_Bars = wxNewId();
const long VUMeterPanel::ID_STATICTEXT2 = wxNewId();
const long VUMeterPanel::ID_CHOICE_VUMeter_Type = wxNewId();
const long VUMeterPanel::ID_STATICTEXT5 = wxNewId();
const long VUMeterPanel::ID_BITMAPBUTTON_CHOICE_VUMeter_Type = wxNewId();
const long VUMeterPanel::ID_STATICTEXT3 = wxNewId();
const long VUMeterPanel::ID_CHOICE_VUMeter_TimingTrack = wxNewId();
const long VUMeterPanel::ID_STATICTEXT4 = wxNewId();
const long VUMeterPanel::ID_BITMAPBUTTON_CHOICE_VUMeter_TimingTrack = wxNewId();
//*)

BEGIN_EVENT_TABLE(VUMeterPanel,wxPanel)
	//(*EventTable(VUMeterPanel)
	//*)
END_EVENT_TABLE()

VUMeterPanel::VUMeterPanel(wxWindow* parent)
{
	//(*Initialize(VUMeterPanel)
	wxFlexGridSizer* FlexGridSizer42;
	wxFlexGridSizer* FlexGridSizer31;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer42 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer42->AddGrowableCol(0);
	FlexGridSizer31 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer31->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Bars"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer31->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_VUMeter_Bars = new wxSlider(this, IDD_SLIDER_VUMeter_Bars, 6, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_VUMeter_Bars"));
	FlexGridSizer31->Add(Slider_VUMeter_Bars, 1, wxALL|wxEXPAND, 2);
	TextCtrl_VUMeter_Bars = new wxTextCtrl(this, ID_TEXTCTRL_VUMeter_Bars, _("6"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_VUMeter_Bars"));
	FlexGridSizer31->Add(TextCtrl_VUMeter_Bars, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_VUMeter_Bars = new wxBitmapButton(this, ID_BITMAPBUTTON_CHOICE_VUMeter_Bars, wxNullBitmap, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_VUMeter_Bars"));
	BitmapButton_VUMeter_Bars->SetDefault();
	BitmapButton_VUMeter_Bars->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer31->Add(BitmapButton_VUMeter_Bars, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer31->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_VUMeter_Type = new wxChoice(this, ID_CHOICE_VUMeter_Type, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_VUMeter_Type"));
	Choice_VUMeter_Type->Append(_("Spectrogram"));
	Choice_VUMeter_Type->Append(_("Volume Bars"));
	Choice_VUMeter_Type->SetSelection( Choice_VUMeter_Type->Append(_("Waveform")) );
	Choice_VUMeter_Type->Append(_("Timing Event Spike"));
	Choice_VUMeter_Type->Append(_("Timing Event Sweep"));
	Choice_VUMeter_Type->Append(_("On"));
	Choice_VUMeter_Type->Append(_("Pulse"));
	Choice_VUMeter_Type->Append(_("Intensity Wave"));
	Choice_VUMeter_Type->Append(_("Spectrogram with Gravity"));
	FlexGridSizer31->Add(Choice_VUMeter_Type, 1, wxALL|wxEXPAND, 2);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer31->Add(StaticText5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_VUMeter_Type = new wxBitmapButton(this, ID_BITMAPBUTTON_CHOICE_VUMeter_Type, wxNullBitmap, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_VUMeter_Type"));
	BitmapButton_VUMeter_Type->SetDefault();
	BitmapButton_VUMeter_Type->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer31->Add(BitmapButton_VUMeter_Type, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Timing Track"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer31->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_VUMeter_TimingTrack = new wxChoice(this, ID_CHOICE_VUMeter_TimingTrack, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_VUMeter_TimingTrack"));
	Choice_VUMeter_TimingTrack->Disable();
	FlexGridSizer31->Add(Choice_VUMeter_TimingTrack, 1, wxALL|wxEXPAND, 2);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer31->Add(StaticText4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_VUMeter_TimingTrack = new wxBitmapButton(this, ID_BITMAPBUTTON_CHOICE_VUMeter_TimingTrack, wxNullBitmap, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_VUMeter_TimingTrack"));
	BitmapButton_VUMeter_TimingTrack->SetDefault();
	BitmapButton_VUMeter_TimingTrack->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer31->Add(BitmapButton_VUMeter_TimingTrack, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer42->Add(FlexGridSizer31, 1, wxEXPAND, 2);
	SetSizer(FlexGridSizer42);
	FlexGridSizer42->Fit(this);
	FlexGridSizer42->SetSizeHints(this);

	Connect(IDD_SLIDER_VUMeter_Bars,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&VUMeterPanel::UpdateLinkedTextCtrl);
	Connect(ID_TEXTCTRL_VUMeter_Bars,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&VUMeterPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_CHOICE_VUMeter_Bars,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&VUMeterPanel::OnLockButtonClick);
	Connect(ID_CHOICE_VUMeter_Type,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&VUMeterPanel::OnChoice_VUMeter_TypeSelect);
	Connect(ID_BITMAPBUTTON_CHOICE_VUMeter_Type,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&VUMeterPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHOICE_VUMeter_TimingTrack,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&VUMeterPanel::OnLockButtonClick);
	//*)
    SetName("ID_PANEL_VUMeter");

	ValidateWindow();
}

VUMeterPanel::~VUMeterPanel()
{
	//(*Destroy(VUMeterPanel)
	//*)
}

PANEL_EVENT_HANDLERS(VUMeterPanel)

static inline void EnableControl(wxWindow *w, int id, bool e) {
    wxWindow *c = w->FindWindowById(id);
    if (c) c->Enable(e);
}

void VUMeterPanel::ValidateWindow()
{
	if (Choice_VUMeter_Type->GetStringSelection() == "Timing Event Spike" || Choice_VUMeter_Type->GetStringSelection() == "Timing Event Sweep" || Choice_VUMeter_Type->GetStringSelection() == "Pulse")
	{
		Choice_VUMeter_TimingTrack->Enable();
	}
	else
	{
		Choice_VUMeter_TimingTrack->Disable();
	}
}

void VUMeterPanel::OnChoice_VUMeter_TypeSelect(wxCommandEvent& event)
{
	ValidateWindow();
}
