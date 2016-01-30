#include "FireworksPanel.h"
#include "../../include/padlock16x16-blue.xpm"
#include "EffectPanelUtils.h"

//(*InternalHeaders(FireworksPanel)
#include <wx/bmpbuttn.h>
#include <wx/sizer.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/slider.h>
#include <wx/intl.h>
#include <wx/stattext.h>
#include <wx/bitmap.h>
#include <wx/image.h>
//*)

//(*IdInit(FireworksPanel)
const long FireworksPanel::ID_STATICTEXT95 = wxNewId();
const long FireworksPanel::ID_SLIDER_Fireworks_Explosions = wxNewId();
const long FireworksPanel::ID_BITMAPBUTTON_SLIDER_Fireworks_Number_Explosions = wxNewId();
const long FireworksPanel::ID_STATICTEXT92 = wxNewId();
const long FireworksPanel::ID_SLIDER_Fireworks_Count = wxNewId();
const long FireworksPanel::ID_BITMAPBUTTON_SLIDER_Fireworks_Count = wxNewId();
const long FireworksPanel::ID_STATICTEXT94 = wxNewId();
const long FireworksPanel::ID_SLIDER_Fireworks_Velocity = wxNewId();
const long FireworksPanel::ID_BITMAPBUTTON_SLIDER_Fireworks_Velocity = wxNewId();
const long FireworksPanel::ID_STATICTEXT96 = wxNewId();
const long FireworksPanel::ID_SLIDER_Fireworks_Fade = wxNewId();
const long FireworksPanel::ID_BITMAPBUTTON_SLIDER_Fireworks_Fade = wxNewId();
//*)

BEGIN_EVENT_TABLE(FireworksPanel,wxPanel)
	//(*EventTable(FireworksPanel)
	//*)
END_EVENT_TABLE()

FireworksPanel::FireworksPanel(wxWindow* parent)
{
	//(*Initialize(FireworksPanel)
	wxSlider* Slider_Fireworks_Num_Explosions;
	wxFlexGridSizer* FlexGridSizer73;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer73 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer73->AddGrowableCol(1);
	StaticText94 = new wxStaticText(this, ID_STATICTEXT95, _("Number of Explosions"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT95"));
	FlexGridSizer73->Add(StaticText94, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fireworks_Num_Explosions = new wxSlider(this, ID_SLIDER_Fireworks_Explosions, 5, 1, 50, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fireworks_Explosions"));
	FlexGridSizer73->Add(Slider_Fireworks_Num_Explosions, 1, wxALL|wxEXPAND, 5);
	BitmapButton_FireworksNumberExplosions = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Fireworks_Number_Explosions, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fireworks_Number_Explosions"));
	BitmapButton_FireworksNumberExplosions->SetDefault();
	BitmapButton_FireworksNumberExplosions->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer73->Add(BitmapButton_FireworksNumberExplosions, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText91 = new wxStaticText(this, ID_STATICTEXT92, _("Particles in Explosion"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT92"));
	FlexGridSizer73->Add(StaticText91, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fireworks_Count = new wxSlider(this, ID_SLIDER_Fireworks_Count, 50, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fireworks_Count"));
	FlexGridSizer73->Add(Slider_Fireworks_Count, 1, wxALL|wxEXPAND, 5);
	BitmapButton_FireworksCount = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Fireworks_Count, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fireworks_Count"));
	BitmapButton_FireworksCount->SetDefault();
	BitmapButton_FireworksCount->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer73->Add(BitmapButton_FireworksCount, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText93 = new wxStaticText(this, ID_STATICTEXT94, _("Velocity of Particles"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT94"));
	FlexGridSizer73->Add(StaticText93, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fireworks_Velocity = new wxSlider(this, ID_SLIDER_Fireworks_Velocity, 2, 1, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fireworks_Velocity"));
	FlexGridSizer73->Add(Slider_Fireworks_Velocity, 1, wxALL|wxEXPAND, 5);
	BitmapButton_FireworksVelocity = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Fireworks_Velocity, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fireworks_Velocity"));
	BitmapButton_FireworksVelocity->SetDefault();
	BitmapButton_FireworksVelocity->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer73->Add(BitmapButton_FireworksVelocity, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText95 = new wxStaticText(this, ID_STATICTEXT96, _("Particle Fade"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT96"));
	FlexGridSizer73->Add(StaticText95, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fireworks_Fade = new wxSlider(this, ID_SLIDER_Fireworks_Fade, 50, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fireworks_Fade"));
	FlexGridSizer73->Add(Slider_Fireworks_Fade, 1, wxALL|wxEXPAND, 5);
	BitmapButton_FireworksFade = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Fireworks_Fade, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fireworks_Fade"));
	BitmapButton_FireworksFade->SetDefault();
	BitmapButton_FireworksFade->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer73->Add(BitmapButton_FireworksFade, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	SetSizer(FlexGridSizer73);
	FlexGridSizer73->Fit(this);
	FlexGridSizer73->SetSizeHints(this);

	Connect(ID_BITMAPBUTTON_SLIDER_Fireworks_Number_Explosions,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fireworks_Count,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fireworks_Velocity,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fireworks_Fade,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnLockButtonClick);
	//*)
    SetName("ID_PANEL_FIREWORKS");
}

FireworksPanel::~FireworksPanel()
{
	//(*Destroy(FireworksPanel)
	//*)
}

PANEL_EVENT_HANDLERS(FireworksPanel)