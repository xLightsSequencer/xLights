#include "FireworksPanel.h"
#include "EffectPanelUtils.h"
#include "../sequencer/Effect.h"

//(*InternalHeaders(FireworksPanel)
#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>
//*)

#include "../ValueCurveButton.h"

//(*IdInit(FireworksPanel)
const long FireworksPanel::ID_STATICTEXT_Fireworks_Explosions = wxNewId();
const long FireworksPanel::ID_SLIDER_Fireworks_Explosions = wxNewId();
const long FireworksPanel::IDD_TEXTCTRL_Fireworks_Explosions = wxNewId();
const long FireworksPanel::ID_BITMAPBUTTON_SLIDER_Fireworks_Explosions = wxNewId();
const long FireworksPanel::ID_STATICTEXT_Fireworks_Count = wxNewId();
const long FireworksPanel::ID_SLIDER_Fireworks_Count = wxNewId();
const long FireworksPanel::IDD_TEXTCTRL_Fireworks_Count = wxNewId();
const long FireworksPanel::ID_BITMAPBUTTON_SLIDER_Fireworks_Count = wxNewId();
const long FireworksPanel::ID_STATICTEXT_Fireworks_Velocity = wxNewId();
const long FireworksPanel::ID_SLIDER_Fireworks_Velocity = wxNewId();
const long FireworksPanel::IDD_TEXTCTRL_Fireworks_Velocity = wxNewId();
const long FireworksPanel::ID_BITMAPBUTTON_SLIDER_Fireworks_Velocity = wxNewId();
const long FireworksPanel::ID_STATICTEXT_Fireworks_Fade = wxNewId();
const long FireworksPanel::ID_SLIDER_Fireworks_Fade = wxNewId();
const long FireworksPanel::IDD_TEXTCTRL_Fireworks_Fade = wxNewId();
const long FireworksPanel::ID_BITMAPBUTTON_SLIDER_Fireworks_Fade = wxNewId();
const long FireworksPanel::ID_CHECKBOX_Fireworks_UseMusic = wxNewId();
const long FireworksPanel::ID_BITMAPBUTTON_CHECKBOX_Fireworks_UseMusic = wxNewId();
const long FireworksPanel::ID_STATICTEXT_Fireworks_Sensitivity = wxNewId();
const long FireworksPanel::ID_SLIDER_Fireworks_Sensitivity = wxNewId();
const long FireworksPanel::IDD_TEXTCTRL_Fireworks_Sensitivity = wxNewId();
const long FireworksPanel::ID_BITMAPBUTTON_SLIDER_Fireworks_Sensitivity = wxNewId();
const long FireworksPanel::ID_CHECKBOX_FIRETIMING = wxNewId();
const long FireworksPanel::ID_BITMAPBUTTON_CHECKBOX_FIRETIMING = wxNewId();
const long FireworksPanel::ID_STATICTEXT_FIRETIMINGTRACK = wxNewId();
const long FireworksPanel::ID_CHOICE_FIRETIMINGTRACK = wxNewId();
const long FireworksPanel::ID_CHECKBOX_Fireworks_UseLocation = wxNewId();
const long FireworksPanel::ID_SLIDER_Fireworks_LocationX = wxNewId();
const long FireworksPanel::ID_SLIDER_Fireworks_LocationY = wxNewId();
const long FireworksPanel::IDD_TEXTCTRL_Fireworks_LocationX = wxNewId();
const long FireworksPanel::IDD_TEXTCTRL_Fireworks_LocationY = wxNewId();
const long FireworksPanel::ID_STATICTEXT_Fireworks_LocationX = wxNewId();
const long FireworksPanel::ID_STATICTEXT_Fireworks_LocationY = wxNewId();
const long FireworksPanel::ID_VALUECURVE_Fireworks_LocationX = wxNewId();
const long FireworksPanel::ID_VALUECURVE_Fireworks_LocationY = wxNewId();
const long FireworksPanel::ID_BITMAPBUTTON_CHECKBOX_Fireworks_UseLocation = wxNewId();
const long FireworksPanel::ID_BITMAPBUTTON_SLIDER_Fireworks_LocationX = wxNewId();
const long FireworksPanel::ID_BITMAPBUTTON_SLIDER_Fireworks_LocationY = wxNewId();
//*)

BEGIN_EVENT_TABLE(FireworksPanel,wxPanel)
	//(*EventTable(FireworksPanel)
	//*)
    EVT_COMMAND(wxID_ANY, EVT_SETTIMINGTRACKS, FireworksPanel::SetTimingTracks)
END_EVENT_TABLE()

FireworksPanel::FireworksPanel(wxWindow* parent)
{
	//(*Initialize(FireworksPanel)
	wxFlexGridSizer* FlexGridSizer73;
    wxFlexGridSizer* FlexGridSizer74;
    wxFlexGridSizer* FlexGridSizer75;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer73 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer73->AddGrowableCol(1);
	StaticText94 = new wxStaticText(this, ID_STATICTEXT_Fireworks_Explosions, _("Number of Explosions"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fireworks_Explosions"));
	FlexGridSizer73->Add(StaticText94, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fireworks_Num_Explosions = new BulkEditSlider(this, ID_SLIDER_Fireworks_Explosions, 16, 1, 50, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fireworks_Explosions"));
	FlexGridSizer73->Add(Slider_Fireworks_Num_Explosions, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Fireworks_Explosions = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Fireworks_Explosions, _("16"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Fireworks_Explosions"));
	TextCtrl_Fireworks_Explosions->SetMaxLength(2);
	FlexGridSizer73->Add(TextCtrl_Fireworks_Explosions, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_FireworksNumberExplosions = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Fireworks_Explosions, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fireworks_Explosions"));
	BitmapButton_FireworksNumberExplosions->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer73->Add(BitmapButton_FireworksNumberExplosions, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText91 = new wxStaticText(this, ID_STATICTEXT_Fireworks_Count, _("Particles in Explosion"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fireworks_Count"));
	FlexGridSizer73->Add(StaticText91, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fireworks_Count = new BulkEditSlider(this, ID_SLIDER_Fireworks_Count, 50, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fireworks_Count"));
	FlexGridSizer73->Add(Slider_Fireworks_Count, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Fireworks_Count = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Fireworks_Count, _("50"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Fireworks_Count"));
	TextCtrl_Fireworks_Count->SetMaxLength(3);
	FlexGridSizer73->Add(TextCtrl_Fireworks_Count, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_FireworksCount = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Fireworks_Count, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fireworks_Count"));
	BitmapButton_FireworksCount->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer73->Add(BitmapButton_FireworksCount, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText93 = new wxStaticText(this, ID_STATICTEXT_Fireworks_Velocity, _("Velocity of Particles"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fireworks_Velocity"));
	FlexGridSizer73->Add(StaticText93, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fireworks_Velocity = new BulkEditSlider(this, ID_SLIDER_Fireworks_Velocity, 2, 1, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fireworks_Velocity"));
	FlexGridSizer73->Add(Slider_Fireworks_Velocity, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Fireworks_Velocity = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Fireworks_Velocity, _("2"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Fireworks_Velocity"));
	TextCtrl_Fireworks_Velocity->SetMaxLength(2);
	FlexGridSizer73->Add(TextCtrl_Fireworks_Velocity, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_FireworksVelocity = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Fireworks_Velocity, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fireworks_Velocity"));
	BitmapButton_FireworksVelocity->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer73->Add(BitmapButton_FireworksVelocity, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText95 = new wxStaticText(this, ID_STATICTEXT_Fireworks_Fade, _("Particle Fade"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fireworks_Fade"));
	FlexGridSizer73->Add(StaticText95, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fireworks_Fade = new BulkEditSlider(this, ID_SLIDER_Fireworks_Fade, 50, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fireworks_Fade"));
	FlexGridSizer73->Add(Slider_Fireworks_Fade, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Fireworks_Fade = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Fireworks_Fade, _("50"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Fireworks_Fade"));
	TextCtrl_Fireworks_Fade->SetMaxLength(3);
	FlexGridSizer73->Add(TextCtrl_Fireworks_Fade, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_FireworksFade = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Fireworks_Fade, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fireworks_Fade"));
	BitmapButton_FireworksFade->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer73->Add(BitmapButton_FireworksFade, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer73->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Fireworks_UseMusic = new BulkEditCheckBox(this, ID_CHECKBOX_Fireworks_UseMusic, _("Fire with music"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Fireworks_UseMusic"));
	CheckBox_Fireworks_UseMusic->SetValue(false);
	FlexGridSizer73->Add(CheckBox_Fireworks_UseMusic, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer73->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Fireworks_UseMusic = new xlLockButton(this, ID_BITMAPBUTTON_CHECKBOX_Fireworks_UseMusic, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Fireworks_UseMusic"));
	BitmapButton_Fireworks_UseMusic->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer73->Add(BitmapButton_Fireworks_UseMusic, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT_Fireworks_Sensitivity, _("Trigger level"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fireworks_Sensitivity"));
	FlexGridSizer73->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fireworks_Sensitivity = new BulkEditSlider(this, ID_SLIDER_Fireworks_Sensitivity, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fireworks_Sensitivity"));
	FlexGridSizer73->Add(Slider_Fireworks_Sensitivity, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Fireworks_Sensitivity = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Fireworks_Sensitivity, _("50"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Fireworks_Sensitivity"));
	TextCtrl_Fireworks_Sensitivity->SetMaxLength(3);
	FlexGridSizer73->Add(TextCtrl_Fireworks_Sensitivity, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Fireworks_Sensitivity = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Fireworks_Sensitivity, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fireworks_Sensitivity"));
	BitmapButton_Fireworks_Sensitivity->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer73->Add(BitmapButton_Fireworks_Sensitivity, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer73->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_FireTiming = new BulkEditCheckBox(this, ID_CHECKBOX_FIRETIMING, _("Fire with timing track"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_FIRETIMING"));
	CheckBox_FireTiming->SetValue(false);
	FlexGridSizer73->Add(CheckBox_FireTiming, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer73->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton1 = new xlLockButton(this, ID_BITMAPBUTTON_CHECKBOX_FIRETIMING, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_FIRETIMING"));
	BitmapButton1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer73->Add(BitmapButton1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT_FIRETIMINGTRACK, _("Timing Track"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_FIRETIMINGTRACK"));
	FlexGridSizer73->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_TimingTrack = new BulkEditChoice(this, ID_CHOICE_FIRETIMINGTRACK, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_FIRETIMINGTRACK"));
	FlexGridSizer73->Add(Choice_TimingTrack, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer73->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer73->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    
    FlexGridSizer73->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_Fireworks_UseLocation = new BulkEditCheckBox(this, ID_CHECKBOX_Fireworks_UseLocation, _("Manual explosion location"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Fireworks_UseLocation"));
    CheckBox_Fireworks_UseLocation->SetValue(false);
    FlexGridSizer73->Add(CheckBox_Fireworks_UseLocation, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer73->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_Fireworks_UseLocation = new xlLockButton(this, ID_BITMAPBUTTON_CHECKBOX_Fireworks_UseLocation, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Fireworks_UseLocation"));
    BitmapButton_Fireworks_UseLocation->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
    FlexGridSizer73->Add(BitmapButton_Fireworks_UseLocation, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    
    
    StaticText100 = new wxStaticText(this, ID_STATICTEXT_Fireworks_LocationX, _("Location X"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fireworks_LocationX"));
    FlexGridSizer73->Add(StaticText100, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    
    FlexGridSizer74 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer74->AddGrowableCol(0);
    Slider_Fireworks_LocationX = new BulkEditSlider(this, ID_SLIDER_Fireworks_LocationX, 0, FIREWORKS_POSITIONX_MIN, FIREWORKS_POSITIONX_MAX, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fireworks_LocationX"));
    FlexGridSizer74->Add(Slider_Fireworks_LocationX, 1, wxALL|wxEXPAND, 2);
    
    ValueCurve_Fireworks_LocationX = new BulkEditValueCurveButton(this, ID_VALUECURVE_Fireworks_LocationX, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Fireworks_LocationX"));
    FlexGridSizer74->Add(ValueCurve_Fireworks_LocationX, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    FlexGridSizer73->Add(FlexGridSizer74, 1, wxALL|wxEXPAND, 0);
    
    TextCtrl_Fireworks_LocationX = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Fireworks_LocationX, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Fireworks_LocationX"));
    TextCtrl_Fireworks_LocationX->SetMaxLength(3);
    FlexGridSizer73->Add(TextCtrl_Fireworks_LocationX, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    
    BitmapButton_Fireworks_LocationX = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Fireworks_LocationX, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fireworks_LocationX"));
    BitmapButton_Fireworks_LocationX->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
    FlexGridSizer73->Add(BitmapButton_Fireworks_LocationX, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    
    
    
    
    StaticText170 = new wxStaticText(this, ID_STATICTEXT_Fireworks_LocationY, _("Location Y"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fireworks_LocationY"));
    FlexGridSizer73->Add(StaticText170, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    
    FlexGridSizer75 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer75->AddGrowableCol(0);
    Slider_Fireworks_LocationY = new BulkEditSlider(this, ID_SLIDER_Fireworks_LocationY, 0, FIREWORKS_POSITIONY_MIN, FIREWORKS_POSITIONY_MAX, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fireworks_LocationY"));
    FlexGridSizer75->Add(Slider_Fireworks_LocationY, 1, wxALL|wxEXPAND, 2);
    
    ValueCurve_Fireworks_LocationY = new BulkEditValueCurveButton(this, ID_VALUECURVE_Fireworks_LocationY, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Fireworks_LocationY"));
    FlexGridSizer75->Add(ValueCurve_Fireworks_LocationY, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    FlexGridSizer73->Add(FlexGridSizer75, 1, wxALL|wxEXPAND, 0);
    
    TextCtrl_Fireworks_LocationY = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Fireworks_LocationY, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Fireworks_LocationY"));
    TextCtrl_Fireworks_LocationY->SetMaxLength(3);
    FlexGridSizer73->Add(TextCtrl_Fireworks_LocationY, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    
    BitmapButton_Fireworks_LocationY = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Fireworks_LocationY, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fireworks_LocationY"));
    BitmapButton_Fireworks_LocationY->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
    FlexGridSizer73->Add(BitmapButton_Fireworks_LocationY, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    
    
	
	SetSizer(FlexGridSizer73);
	FlexGridSizer73->Fit(this);
	FlexGridSizer73->SetSizeHints(this);

	Connect(ID_BITMAPBUTTON_SLIDER_Fireworks_Explosions,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fireworks_Count,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fireworks_Velocity,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fireworks_Fade,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnLockButtonClick);
	Connect(ID_CHECKBOX_Fireworks_UseMusic,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnCheckBox_Fireworks_UseMusicClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Fireworks_UseMusic,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fireworks_Sensitivity,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnLockButtonClick);
	Connect(ID_CHECKBOX_FIRETIMING,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnCheckBox_FireTimingClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_FIRETIMING,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnLockButtonClick);
	Connect(ID_CHOICE_FIRETIMINGTRACK,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&FireworksPanel::OnChoice_TimingTrackSelect);
    
    Connect(ID_CHECKBOX_Fireworks_UseLocation,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnCheckBox_UseLocationClick);
    Connect(ID_BITMAPBUTTON_CHECKBOX_Fireworks_UseLocation,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnLockButtonClick);
    Connect(ID_BITMAPBUTTON_SLIDER_Fireworks_LocationX,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnLockButtonClick);
    Connect(ID_BITMAPBUTTON_SLIDER_Fireworks_LocationY,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnLockButtonClick);
    Connect(ID_VALUECURVE_Fireworks_LocationX,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnVCButtonClick);
    Connect(ID_VALUECURVE_Fireworks_LocationY,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnVCButtonClick);
	//*)

    SetName("ID_PANEL_FIREWORKS");

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&FireworksPanel::OnVCChanged, 0, this);
    
    ValueCurve_Fireworks_LocationX->GetValue()->SetLimits(FIREWORKS_POSITIONX_MIN, FIREWORKS_POSITIONY_MAX);
    ValueCurve_Fireworks_LocationY->GetValue()->SetLimits(FIREWORKS_POSITIONY_MIN, FIREWORKS_POSITIONY_MAX);

    ValidateWindow();
}

FireworksPanel::~FireworksPanel()
{
	//(*Destroy(FireworksPanel)
	//*)
}

void FireworksPanel::ValidateWindow()
{
    if (CheckBox_Fireworks_UseMusic->GetValue())
    {
        Slider_Fireworks_Sensitivity->Enable(true);
    }
    else
    {
        Slider_Fireworks_Sensitivity->Enable(false);
    }
    if (CheckBox_FireTiming->GetValue())
    {
        Choice_TimingTrack->Enable(true);
    }
    else
    {
        Choice_TimingTrack->Enable(false);
    }
    
    if (CheckBox_Fireworks_UseLocation->GetValue())
    {
        Slider_Fireworks_LocationX->Enable();
        Slider_Fireworks_LocationY->Enable();
        ValueCurve_Fireworks_LocationX->Enable();
        ValueCurve_Fireworks_LocationY->Enable();
        TextCtrl_Fireworks_LocationX->Enable();
        TextCtrl_Fireworks_LocationY->Enable();
        BitmapButton_Fireworks_LocationX->Enable();
        BitmapButton_Fireworks_LocationY->Enable();
    }
    else
    {
        Slider_Fireworks_LocationX->Disable();
        Slider_Fireworks_LocationY->Disable();
        ValueCurve_Fireworks_LocationX->Disable();
        ValueCurve_Fireworks_LocationY->Disable();
        TextCtrl_Fireworks_LocationX->Disable();
        TextCtrl_Fireworks_LocationY->Disable();
        BitmapButton_Fireworks_LocationX->Disable();
        BitmapButton_Fireworks_LocationY->Disable();
    }
}

PANEL_EVENT_HANDLERS(FireworksPanel)

void FireworksPanel::OnCheckBox_Fireworks_UseMusicClick(wxCommandEvent& event)
{
    ValidateWindow();
}

void FireworksPanel::OnCheckBox_FireTimingClick(wxCommandEvent& event)
{
    ValidateWindow();
}

void FireworksPanel::OnCheckBox_UseLocationClick(wxCommandEvent& event)
{
    ValidateWindow();
}

void FireworksPanel::OnChoice_TimingTrackSelect(wxCommandEvent& event)
{
    ValidateWindow();
}

void FireworksPanel::SetTimingTracks(wxCommandEvent& event)
{
    auto timingtracks = wxSplit(event.GetString(), '|');

    wxString selection = Choice_TimingTrack->GetStringSelection();

    // check if anything has been removed ... if it has clear the list and we will have to rebuild it as you cant delete items from a combo box
    bool removed = false;
    for (int i = 0; i < Choice_TimingTrack->GetCount(); i++)
    {
        bool found = false;
        for (auto it = timingtracks.begin(); it != timingtracks.end(); ++it)
        {
            if (*it == Choice_TimingTrack->GetString(i))
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            Choice_TimingTrack->Clear();
            removed = true;
            break;
        }
    }

    // add any new timing tracks
    for (auto it = timingtracks.begin(); it != timingtracks.end(); ++it)
    {
        bool found = false;
        for (size_t i = 0; i < Choice_TimingTrack->GetCount(); i++)
        {
            if (*it == Choice_TimingTrack->GetString(i))
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            Choice_TimingTrack->Append(*it);
        }
    }

    if (removed && Choice_TimingTrack->GetCount() > 0)
    {
        // go through the list and see if our selected item is there
        bool found = false;
        for (size_t i = 0; i < Choice_TimingTrack->GetCount(); i++)
        {
            if (selection == Choice_TimingTrack->GetString(i))
            {
                found = true;
                Choice_TimingTrack->SetSelection(i);
                break;
            }
        }
        if (!found)
        {
            Choice_TimingTrack->SetSelection(0);
        }
    }
    ValidateWindow();
}
