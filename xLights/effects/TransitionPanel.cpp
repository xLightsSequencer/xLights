#include "TransitionPanel.h"
#include "../../include/padlock16x16-blue.xpm"
#include "EffectPanelUtils.h"

//(*InternalHeaders(TransitionPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/slider.h>
#include <wx/settings.h>
#include <wx/choice.h>
#include <wx/bmpbuttn.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(TransitionPanel)
const long TransitionPanel::ID_STATICTEXT2 = wxNewId();
const long TransitionPanel::ID_CHOICE_Transition_Type = wxNewId();
const long TransitionPanel::ID_STATICTEXT5 = wxNewId();
const long TransitionPanel::ID_BITMAPBUTTON_CHOICE_Transition_Type = wxNewId();
const long TransitionPanel::ID_STATICTEXT3 = wxNewId();
const long TransitionPanel::ID_CHECKBOX_Transition_Reveal = wxNewId();
const long TransitionPanel::ID_STATICTEXT4 = wxNewId();
const long TransitionPanel::ID_BITMAPBUTTON_CHOICE_Transition_TimingTrack = wxNewId();
const long TransitionPanel::ID_STATICTEXT6 = wxNewId();
const long TransitionPanel::IDD_SLIDER_Transition_Adjust = wxNewId();
const long TransitionPanel::ID_TEXTCTRL_Transition_Adjust = wxNewId();
const long TransitionPanel::ID_BITMAPBUTTON_SLIDER_Transition_Sensitivity = wxNewId();
const long TransitionPanel::ID_STATICTEXT9 = wxNewId();
const long TransitionPanel::ID_CHECKBOX_Transition_Reverse = wxNewId();
const long TransitionPanel::ID_STATICTEXT10 = wxNewId();
const long TransitionPanel::ID_BITMAPBUTTON_CHECKBOX_Transition_SlowDownFalls = wxNewId();
//*)

BEGIN_EVENT_TABLE(TransitionPanel,wxPanel)
	//(*EventTable(TransitionPanel)
	//*)
END_EVENT_TABLE()

TransitionPanel::TransitionPanel(wxWindow* parent)
{
	//(*Initialize(TransitionPanel)
	wxFlexGridSizer* FlexGridSizer42;
	wxFlexGridSizer* FlexGridSizer31;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer42 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer42->AddGrowableCol(0);
	FlexGridSizer31 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer31->AddGrowableCol(1);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer31->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Transition_Type = new wxChoice(this, ID_CHOICE_Transition_Type, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Transition_Type"));
	Choice_Transition_Type->SetSelection( Choice_Transition_Type->Append(_("Wipe")) );
	Choice_Transition_Type->Append(_("Clockwise"));
	Choice_Transition_Type->Append(_("From Middle"));
	Choice_Transition_Type->Append(_("Square Explode"));
	Choice_Transition_Type->Append(_("Circle Explode"));
	Choice_Transition_Type->Append(_("Blinds"));
	Choice_Transition_Type->Append(_("Blend"));
	Choice_Transition_Type->Append(_("Slide Checks"));
	Choice_Transition_Type->Append(_("Slide Bars"));
	FlexGridSizer31->Add(Choice_Transition_Type, 1, wxALL|wxEXPAND, 2);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer31->Add(StaticText5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Transition_Type = new wxBitmapButton(this, ID_BITMAPBUTTON_CHOICE_Transition_Type, wxNullBitmap, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Transition_Type"));
	BitmapButton_Transition_Type->SetDefault();
	BitmapButton_Transition_Type->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer31->Add(BitmapButton_Transition_Type, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer31->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	CheckBox_Transition_Reveal = new wxCheckBox(this, ID_CHECKBOX_Transition_Reveal, _("Reveal"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Transition_Reveal"));
	CheckBox_Transition_Reveal->SetValue(true);
	FlexGridSizer31->Add(CheckBox_Transition_Reveal, 1, wxALL|wxEXPAND, 2);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer31->Add(StaticText4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Transition_TimingTrack = new wxBitmapButton(this, ID_BITMAPBUTTON_CHOICE_Transition_TimingTrack, wxNullBitmap, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Transition_TimingTrack"));
	BitmapButton_Transition_TimingTrack->SetDefault();
	BitmapButton_Transition_TimingTrack->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer31->Add(BitmapButton_Transition_TimingTrack, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Adjust"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer31->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Transition_Adjust = new wxSlider(this, IDD_SLIDER_Transition_Adjust, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Transition_Adjust"));
	FlexGridSizer31->Add(Slider_Transition_Adjust, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Transition_Adjust = new wxTextCtrl(this, ID_TEXTCTRL_Transition_Adjust, _("50"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Transition_Adjust"));
	FlexGridSizer31->Add(TextCtrl_Transition_Adjust, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Transition_Sensitivity = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Transition_Sensitivity, wxNullBitmap, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Transition_Sensitivity"));
	BitmapButton_Transition_Sensitivity->SetDefault();
	BitmapButton_Transition_Sensitivity->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer31->Add(BitmapButton_Transition_Sensitivity, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText9 = new wxStaticText(this, ID_STATICTEXT9, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
	FlexGridSizer31->Add(StaticText9, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	CheckBox_Transition_Reverse = new wxCheckBox(this, ID_CHECKBOX_Transition_Reverse, _("Reverse"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Transition_Reverse"));
	CheckBox_Transition_Reverse->SetValue(false);
	FlexGridSizer31->Add(CheckBox_Transition_Reverse, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText10 = new wxStaticText(this, ID_STATICTEXT10, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT10"));
	FlexGridSizer31->Add(StaticText10, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Transition_SlowDownFalls = new wxBitmapButton(this, ID_BITMAPBUTTON_CHECKBOX_Transition_SlowDownFalls, wxNullBitmap, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Transition_SlowDownFalls"));
	BitmapButton_Transition_SlowDownFalls->SetDefault();
	BitmapButton_Transition_SlowDownFalls->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer31->Add(BitmapButton_Transition_SlowDownFalls, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer42->Add(FlexGridSizer31, 1, wxEXPAND, 2);
	SetSizer(FlexGridSizer42);
	FlexGridSizer42->Fit(this);
	FlexGridSizer42->SetSizeHints(this);

	Connect(ID_CHOICE_Transition_Type,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&TransitionPanel::OnChoice_Transition_TypeSelect);
	Connect(ID_BITMAPBUTTON_CHOICE_Transition_Type,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TransitionPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHOICE_Transition_TimingTrack,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TransitionPanel::OnLockButtonClick);
	Connect(IDD_SLIDER_Transition_Adjust,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&TransitionPanel::UpdateLinkedTextCtrl);
	Connect(ID_TEXTCTRL_Transition_Adjust,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&TransitionPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_SLIDER_Transition_Sensitivity,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TransitionPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Transition_SlowDownFalls,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TransitionPanel::OnLockButtonClick);
	//*)
    SetName("ID_PANEL_Transition");

	ValidateWindow();
}

TransitionPanel::~TransitionPanel()
{
	//(*Destroy(TransitionPanel)
	//*)
}

PANEL_EVENT_HANDLERS(TransitionPanel)

static inline void EnableControl(wxWindow *w, int id, bool e) {
    wxWindow *c = w->FindWindowById(id);
    if (c) c->Enable(e);
}

void TransitionPanel::ValidateWindow()
{
	if (Choice_Transition_Type->GetStringSelection() == "Wipe" || Choice_Transition_Type->GetStringSelection() == "Blinds")
	{
		CheckBox_Transition_Reverse->Disable();
	}
	else
	{
		CheckBox_Transition_Reverse->Enable();
	}

	if (Choice_Transition_Type->GetStringSelection() == "From Middle" || 
		Choice_Transition_Type->GetStringSelection() == "Square Explode" ||
		Choice_Transition_Type->GetStringSelection() == "Circle Explode")
	{
		Slider_Transition_Adjust->Disable();
		TextCtrl_Transition_Adjust->Disable();
	}
	else
	{
		Slider_Transition_Adjust->Enable();
		TextCtrl_Transition_Adjust->Enable();
	}
}

void TransitionPanel::OnChoice_Transition_TypeSelect(wxCommandEvent& event)
{
	ValidateWindow();
}
