#include "TendrilPanel.h"
#include "../../include/padlock16x16-blue.xpm"
#include "EffectPanelUtils.h"

//(*InternalHeaders(TendrilPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/slider.h>
#include <wx/settings.h>
#include <wx/bmpbuttn.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(TendrilPanel)
const long TendrilPanel::ID_STATICTEXT1 = wxNewId();
const long TendrilPanel::IDD_SLIDER_Tendril_Movement = wxNewId();
const long TendrilPanel::ID_TEXTCTRL_Tendril_Movement = wxNewId();
const long TendrilPanel::ID_BITMAPBUTTON_CHOICE_Tendril_Movement = wxNewId();
const long TendrilPanel::ID_STATICTEXT8 = wxNewId();
const long TendrilPanel::IDD_SLIDER_Tendril_TuneMovement = wxNewId();
const long TendrilPanel::ID_TEXTCTRL_Tendril_TuneMovement = wxNewId();
const long TendrilPanel::ID_BITMAPBUTTON_CHOICE_Tendril_TuneMovement = wxNewId();
const long TendrilPanel::ID_STATICTEXT7 = wxNewId();
const long TendrilPanel::IDD_SLIDER_Tendril_Thickness = wxNewId();
const long TendrilPanel::ID_TEXTCTRL_Tendril_Thickness = wxNewId();
const long TendrilPanel::ID_BITMAPBUTTON_CHOICE_Tendril_Thickness = wxNewId();
const long TendrilPanel::ID_STATICTEXT2 = wxNewId();
const long TendrilPanel::IDD_SLIDER_Tendril_Friction = wxNewId();
const long TendrilPanel::ID_TEXTCTRL_Tendril_Friction = wxNewId();
const long TendrilPanel::ID_BITMAPBUTTON_CHOICE_Tendril_Friction = wxNewId();
const long TendrilPanel::ID_STATICTEXT3 = wxNewId();
const long TendrilPanel::IDD_SLIDER_Tendril_Dampening = wxNewId();
const long TendrilPanel::ID_TEXTCTRL_Tendril_Dampening = wxNewId();
const long TendrilPanel::ID_BITMAPBUTTON_CHOICE_Tendril_Dampening = wxNewId();
const long TendrilPanel::ID_STATICTEXT4 = wxNewId();
const long TendrilPanel::IDD_SLIDER_Tendril_Tension = wxNewId();
const long TendrilPanel::ID_TEXTCTRL_Tendril_Tension = wxNewId();
const long TendrilPanel::ID_BITMAPBUTTON_CHOICE_Tendril_Tension = wxNewId();
const long TendrilPanel::ID_STATICTEXT5 = wxNewId();
const long TendrilPanel::IDD_SLIDER_Tendril_Trails = wxNewId();
const long TendrilPanel::ID_TEXTCTRL_Tendril_Trails = wxNewId();
const long TendrilPanel::ID_BITMAPBUTTON_CHOICE_Tendril_Trails = wxNewId();
const long TendrilPanel::ID_STATICTEXT6 = wxNewId();
const long TendrilPanel::IDD_SLIDER_Tendril_Length = wxNewId();
const long TendrilPanel::ID_TEXTCTRL_Tendril_Length = wxNewId();
const long TendrilPanel::ID_BITMAPBUTTON_CHOICE_Tendril_Length = wxNewId();
const long TendrilPanel::ID_STATICTEXT27 = wxNewId();
const long TendrilPanel::IDD_SLIDER_Tendril_Speed = wxNewId();
const long TendrilPanel::ID_TEXTCTRL_Tendril_Speed = wxNewId();
const long TendrilPanel::IID_BITMAPBUTTON_CHOICE_Tendril_Speed = wxNewId();
//*)

BEGIN_EVENT_TABLE(TendrilPanel,wxPanel)
	//(*EventTable(TendrilPanel)
	//*)
END_EVENT_TABLE()

TendrilPanel::TendrilPanel(wxWindow* parent)
{
	//(*Initialize(TendrilPanel)
	wxFlexGridSizer* FlexGridSizer42;
	wxFlexGridSizer* FlexGridSizer31;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer42 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer42->AddGrowableCol(0);
	FlexGridSizer31 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer31->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Movement"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer31->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Tendril_Movement = new wxSlider(this, IDD_SLIDER_Tendril_Movement, 1, 1, 5, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Tendril_Movement"));
	FlexGridSizer31->Add(Slider_Tendril_Movement, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_Tendril_Movement = new wxTextCtrl(this, ID_TEXTCTRL_Tendril_Movement, _("1"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Tendril_Movement"));
	FlexGridSizer31->Add(TextCtrl_Tendril_Movement, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Tendril_Movement = new wxBitmapButton(this, ID_BITMAPBUTTON_CHOICE_Tendril_Movement, wxNullBitmap, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Tendril_Movement"));
	BitmapButton_Tendril_Movement->SetDefault();
	BitmapButton_Tendril_Movement->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer31->Add(BitmapButton_Tendril_Movement, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText8 = new wxStaticText(this, ID_STATICTEXT8, _("Tune Movement"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
	FlexGridSizer31->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Tendril_TuneMovement = new wxSlider(this, IDD_SLIDER_Tendril_TuneMovement, 10, 0, 20, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Tendril_TuneMovement"));
	FlexGridSizer31->Add(Slider_Tendril_TuneMovement, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_Tendril_TuneMovement = new wxTextCtrl(this, ID_TEXTCTRL_Tendril_TuneMovement, _("10"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Tendril_TuneMovement"));
	FlexGridSizer31->Add(TextCtrl_Tendril_TuneMovement, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Tendril_TuneMovement = new wxBitmapButton(this, ID_BITMAPBUTTON_CHOICE_Tendril_TuneMovement, wxNullBitmap, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Tendril_TuneMovement"));
	BitmapButton_Tendril_TuneMovement->SetDefault();
	BitmapButton_Tendril_TuneMovement->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer31->Add(BitmapButton_Tendril_TuneMovement, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText7 = new wxStaticText(this, ID_STATICTEXT7, _("Thickness"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	FlexGridSizer31->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Tendril_Thickness = new wxSlider(this, IDD_SLIDER_Tendril_Thickness, 1, 1, 20, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Tendril_Thickness"));
	FlexGridSizer31->Add(Slider_Tendril_Thickness, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_Tendril_Thickness = new wxTextCtrl(this, ID_TEXTCTRL_Tendril_Thickness, _("1"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Tendril_Thickness"));
	FlexGridSizer31->Add(TextCtrl_Tendril_Thickness, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Tendril_Thickness = new wxBitmapButton(this, ID_BITMAPBUTTON_CHOICE_Tendril_Thickness, wxNullBitmap, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Tendril_Thickness"));
	BitmapButton_Tendril_Thickness->SetDefault();
	BitmapButton_Tendril_Thickness->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer31->Add(BitmapButton_Tendril_Thickness, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Friction"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer31->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Tendril_Friction = new wxSlider(this, IDD_SLIDER_Tendril_Friction, 10, 0, 20, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Tendril_Friction"));
	FlexGridSizer31->Add(Slider_Tendril_Friction, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_Tendril_Friction = new wxTextCtrl(this, ID_TEXTCTRL_Tendril_Friction, _("10"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Tendril_Friction"));
	FlexGridSizer31->Add(TextCtrl_Tendril_Friction, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_TendrilFriction = new wxBitmapButton(this, ID_BITMAPBUTTON_CHOICE_Tendril_Friction, wxNullBitmap, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Tendril_Friction"));
	BitmapButton_TendrilFriction->SetDefault();
	BitmapButton_TendrilFriction->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer31->Add(BitmapButton_TendrilFriction, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Dampening"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer31->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Tendril_Dampening = new wxSlider(this, IDD_SLIDER_Tendril_Dampening, 10, 0, 20, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Tendril_Dampening"));
	FlexGridSizer31->Add(Slider_Tendril_Dampening, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_Tendril_Dampening = new wxTextCtrl(this, ID_TEXTCTRL_Tendril_Dampening, _("10"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Tendril_Dampening"));
	FlexGridSizer31->Add(TextCtrl_Tendril_Dampening, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_TendrilDampening = new wxBitmapButton(this, ID_BITMAPBUTTON_CHOICE_Tendril_Dampening, wxNullBitmap, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Tendril_Dampening"));
	BitmapButton_TendrilDampening->SetDefault();
	BitmapButton_TendrilDampening->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer31->Add(BitmapButton_TendrilDampening, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Tension"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer31->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Tendril_Tension = new wxSlider(this, IDD_SLIDER_Tendril_Tension, 20, 0, 39, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Tendril_Tension"));
	FlexGridSizer31->Add(Slider_Tendril_Tension, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_Tendril_Tension = new wxTextCtrl(this, ID_TEXTCTRL_Tendril_Tension, _("20"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Tendril_Tension"));
	FlexGridSizer31->Add(TextCtrl_Tendril_Tension, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_TendrilTension = new wxBitmapButton(this, ID_BITMAPBUTTON_CHOICE_Tendril_Tension, wxNullBitmap, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Tendril_Tension"));
	BitmapButton_TendrilTension->SetDefault();
	BitmapButton_TendrilTension->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer31->Add(BitmapButton_TendrilTension, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Trails"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer31->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Tendril_Trails = new wxSlider(this, IDD_SLIDER_Tendril_Trails, 1, 1, 20, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Tendril_Trails"));
	FlexGridSizer31->Add(Slider_Tendril_Trails, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_Tendril_Trails = new wxTextCtrl(this, ID_TEXTCTRL_Tendril_Trails, _("1"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Tendril_Trails"));
	FlexGridSizer31->Add(TextCtrl_Tendril_Trails, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_TendrilTrails = new wxBitmapButton(this, ID_BITMAPBUTTON_CHOICE_Tendril_Trails, wxNullBitmap, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Tendril_Trails"));
	BitmapButton_TendrilTrails->SetDefault();
	BitmapButton_TendrilTrails->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer31->Add(BitmapButton_TendrilTrails, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Length"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer31->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Tendril_Length = new wxSlider(this, IDD_SLIDER_Tendril_Length, 60, 5, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Tendril_Length"));
	FlexGridSizer31->Add(Slider_Tendril_Length, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_Tendril_Length = new wxTextCtrl(this, ID_TEXTCTRL_Tendril_Length, _("60"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Tendril_Length"));
	FlexGridSizer31->Add(TextCtrl_Tendril_Length, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_TendrilLength = new wxBitmapButton(this, ID_BITMAPBUTTON_CHOICE_Tendril_Length, wxNullBitmap, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Tendril_Length"));
	BitmapButton_TendrilLength->SetDefault();
	BitmapButton_TendrilLength->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer31->Add(BitmapButton_TendrilLength, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText68 = new wxStaticText(this, ID_STATICTEXT27, _("Speed"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT27"));
	FlexGridSizer31->Add(StaticText68, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Tendril_Speed = new wxSlider(this, IDD_SLIDER_Tendril_Speed, 100, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Tendril_Speed"));
	FlexGridSizer31->Add(Slider_Tendril_Speed, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_Tendril_Speed = new wxTextCtrl(this, ID_TEXTCTRL_Tendril_Speed, _("100"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Tendril_Speed"));
	FlexGridSizer31->Add(TextCtrl_Tendril_Speed, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_TendrilSpeed = new wxBitmapButton(this, IID_BITMAPBUTTON_CHOICE_Tendril_Speed, wxNullBitmap, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("IID_BITMAPBUTTON_CHOICE_Tendril_Speed"));
	BitmapButton_TendrilSpeed->SetDefault();
	BitmapButton_TendrilSpeed->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer31->Add(BitmapButton_TendrilSpeed, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer42->Add(FlexGridSizer31, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	SetSizer(FlexGridSizer42);
	FlexGridSizer42->Fit(this);
	FlexGridSizer42->SetSizeHints(this);

	Connect(IDD_SLIDER_Tendril_Movement,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&TendrilPanel::UpdateLinkedTextCtrl);
	Connect(ID_TEXTCTRL_Tendril_Movement,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&TendrilPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_CHOICE_Tendril_Movement,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TendrilPanel::OnLockButtonClick);
	Connect(IDD_SLIDER_Tendril_TuneMovement,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&TendrilPanel::UpdateLinkedTextCtrl);
	Connect(ID_TEXTCTRL_Tendril_TuneMovement,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&TendrilPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_CHOICE_Tendril_TuneMovement,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TendrilPanel::OnLockButtonClick);
	Connect(IDD_SLIDER_Tendril_Thickness,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&TendrilPanel::UpdateLinkedTextCtrl);
	Connect(ID_TEXTCTRL_Tendril_Thickness,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&TendrilPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_CHOICE_Tendril_Thickness,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TendrilPanel::OnLockButtonClick);
	Connect(IDD_SLIDER_Tendril_Friction,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&TendrilPanel::UpdateLinkedTextCtrl);
	Connect(ID_TEXTCTRL_Tendril_Friction,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&TendrilPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_CHOICE_Tendril_Friction,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TendrilPanel::OnLockButtonClick);
	Connect(IDD_SLIDER_Tendril_Dampening,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&TendrilPanel::UpdateLinkedTextCtrl);
	Connect(ID_TEXTCTRL_Tendril_Dampening,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&TendrilPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_CHOICE_Tendril_Dampening,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TendrilPanel::OnLockButtonClick);
	Connect(IDD_SLIDER_Tendril_Tension,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&TendrilPanel::UpdateLinkedTextCtrl);
	Connect(ID_TEXTCTRL_Tendril_Tension,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&TendrilPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_CHOICE_Tendril_Tension,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TendrilPanel::OnLockButtonClick);
	Connect(IDD_SLIDER_Tendril_Trails,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&TendrilPanel::UpdateLinkedTextCtrl);
	Connect(ID_TEXTCTRL_Tendril_Trails,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&TendrilPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_CHOICE_Tendril_Trails,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TendrilPanel::OnLockButtonClick);
	Connect(IDD_SLIDER_Tendril_Length,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&TendrilPanel::UpdateLinkedTextCtrl);
	Connect(ID_TEXTCTRL_Tendril_Length,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&TendrilPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_CHOICE_Tendril_Length,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TendrilPanel::OnLockButtonClick);
	Connect(IDD_SLIDER_Tendril_Speed,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&TendrilPanel::UpdateLinkedTextCtrl);
	Connect(ID_TEXTCTRL_Tendril_Speed,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&TendrilPanel::UpdateLinkedSlider);
	Connect(IID_BITMAPBUTTON_CHOICE_Tendril_Speed,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TendrilPanel::OnLockButtonClick);
	//*)
    SetName("ID_PANEL_Tendril");
}

TendrilPanel::~TendrilPanel()
{
	//(*Destroy(TendrilPanel)
	//*)
}

PANEL_EVENT_HANDLERS(TendrilPanel)

static inline void EnableControl(wxWindow *w, int id, bool e) {
    wxWindow *c = w->FindWindowById(id);
    if (c) c->Enable(e);
}
