/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "FireworksPanel.h"
#include "EffectPanelUtils.h"
#include "../sequencer/Effect.h"
#include "FireworksEffect.h"

//(*InternalHeaders(FireworksPanel)
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
const long FireworksPanel::ID_VALUECURVE_Fireworks_Count = wxNewId();
const long FireworksPanel::IDD_TEXTCTRL_Fireworks_Count = wxNewId();
const long FireworksPanel::ID_BITMAPBUTTON_SLIDER_Fireworks_Count = wxNewId();
const long FireworksPanel::ID_STATICTEXT_Fireworks_Velocity = wxNewId();
const long FireworksPanel::ID_SLIDER_Fireworks_Velocity = wxNewId();
const long FireworksPanel::ID_VALUECURVE_Fireworks_Velocity = wxNewId();
const long FireworksPanel::IDD_TEXTCTRL_Fireworks_Velocity = wxNewId();
const long FireworksPanel::ID_BITMAPBUTTON_SLIDER_Fireworks_Velocity = wxNewId();
const long FireworksPanel::ID_STATICTEXT1 = wxNewId();
const long FireworksPanel::ID_SLIDER_Fireworks_XVelocity = wxNewId();
const long FireworksPanel::ID_VALUECURVE_Fireworks_XVelocity = wxNewId();
const long FireworksPanel::ID_TEXTCTRL_Fireworks_XVelocity = wxNewId();
const long FireworksPanel::ID_STATICTEXT2 = wxNewId();
const long FireworksPanel::ID_SLIDER_Fireworks_YVelocity = wxNewId();
const long FireworksPanel::ID_VALUECURVE_Fireworks_YVelocity = wxNewId();
const long FireworksPanel::ID_TEXTCTRL_Fireworks_YVelocity = wxNewId();
const long FireworksPanel::ID_STATICTEXT3 = wxNewId();
const long FireworksPanel::ID_SLIDER_Fireworks_XLocation = wxNewId();
const long FireworksPanel::ID_VALUECURVE_Fireworks_XLocation = wxNewId();
const long FireworksPanel::ID_TEXTCTRL_Fireworks_XLocation = wxNewId();
const long FireworksPanel::ID_STATICTEXT4 = wxNewId();
const long FireworksPanel::ID_SLIDER_Fireworks_YLocation = wxNewId();
const long FireworksPanel::ID_VALUECURVE_Fireworks_YLocation = wxNewId();
const long FireworksPanel::ID_TEXTCTRL_Fireworks_YLocation = wxNewId();
const long FireworksPanel::ID_CHECKBOX_Fireworks_HoldColour = wxNewId();
const long FireworksPanel::ID_CHECKBOX_Fireworks_Gravity = wxNewId();
const long FireworksPanel::ID_STATICTEXT_Fireworks_Fade = wxNewId();
const long FireworksPanel::ID_SLIDER_Fireworks_Fade = wxNewId();
const long FireworksPanel::ID_VALUECURVE_Fireworks_Fade = wxNewId();
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
//*)

BEGIN_EVENT_TABLE(FireworksPanel,wxPanel)
	//(*EventTable(FireworksPanel)
	//*)
    EVT_COMMAND(wxID_ANY, EVT_SETTIMINGTRACKS, FireworksPanel::SetTimingTracks)
END_EVENT_TABLE()

FireworksPanel::FireworksPanel(wxWindow* parent) : xlEffectPanel(parent)
{
	//(*Initialize(FireworksPanel)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer6;
	wxFlexGridSizer* FlexGridSizer73;
	wxFlexGridSizer* FlexGridSizer7;

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
	BitmapButton_FireworksNumberExplosions = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Fireworks_Explosions, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fireworks_Explosions"));
	BitmapButton_FireworksNumberExplosions->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer73->Add(BitmapButton_FireworksNumberExplosions, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText91 = new wxStaticText(this, ID_STATICTEXT_Fireworks_Count, _("Particles in Explosion"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fireworks_Count"));
	FlexGridSizer73->Add(StaticText91, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	Slider_Fireworks_Count = new BulkEditSlider(this, ID_SLIDER_Fireworks_Count, 50, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fireworks_Count"));
	FlexGridSizer1->Add(Slider_Fireworks_Count, 1, wxALL|wxEXPAND, 5);
	BitmapButton_Fireworks_Count = new BulkEditValueCurveButton(this, ID_VALUECURVE_Fireworks_Count, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Fireworks_Count"));
	FlexGridSizer1->Add(BitmapButton_Fireworks_Count, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer73->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Fireworks_Count = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Fireworks_Count, _("50"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Fireworks_Count"));
	TextCtrl_Fireworks_Count->SetMaxLength(3);
	FlexGridSizer73->Add(TextCtrl_Fireworks_Count, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_FireworksCount = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Fireworks_Count, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fireworks_Count"));
	BitmapButton_FireworksCount->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer73->Add(BitmapButton_FireworksCount, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText93 = new wxStaticText(this, ID_STATICTEXT_Fireworks_Velocity, _("Velocity of Particles"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fireworks_Velocity"));
	FlexGridSizer73->Add(StaticText93, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	Slider_Fireworks_Velocity = new BulkEditSlider(this, ID_SLIDER_Fireworks_Velocity, 2, 1, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fireworks_Velocity"));
	FlexGridSizer2->Add(Slider_Fireworks_Velocity, 1, wxALL|wxEXPAND, 5);
	BitmapButton_Fireworks_Velocity = new BulkEditValueCurveButton(this, ID_VALUECURVE_Fireworks_Velocity, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Fireworks_Velocity"));
	FlexGridSizer2->Add(BitmapButton_Fireworks_Velocity, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer73->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Fireworks_Velocity = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Fireworks_Velocity, _("2"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Fireworks_Velocity"));
	TextCtrl_Fireworks_Velocity->SetMaxLength(2);
	FlexGridSizer73->Add(TextCtrl_Fireworks_Velocity, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_FireworksVelocity = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Fireworks_Velocity, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fireworks_Velocity"));
	BitmapButton_FireworksVelocity->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer73->Add(BitmapButton_FireworksVelocity, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT1, _("X Velocity"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer73->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	Slider_Fireworks_XVelocity = new BulkEditSlider(this, ID_SLIDER_Fireworks_XVelocity, 0, -100, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fireworks_XVelocity"));
	FlexGridSizer3->Add(Slider_Fireworks_XVelocity, 1, wxALL|wxEXPAND, 5);
	BitmapButton_Fireworks_XVelocity = new BulkEditValueCurveButton(this, ID_VALUECURVE_Fireworks_XVelocity, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Fireworks_XVelocity"));
	FlexGridSizer3->Add(BitmapButton_Fireworks_XVelocity, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer73->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 5);
	TextCtrl1 = new BulkEditTextCtrl(this, ID_TEXTCTRL_Fireworks_XVelocity, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Fireworks_XVelocity"));
	TextCtrl1->SetMaxLength(3);
	FlexGridSizer73->Add(TextCtrl1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer73->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT2, _("Y Velocity"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer73->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer4->AddGrowableCol(0);
	Slider_Fireworks_YVelocity = new BulkEditSlider(this, ID_SLIDER_Fireworks_YVelocity, 0, -100, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fireworks_YVelocity"));
	FlexGridSizer4->Add(Slider_Fireworks_YVelocity, 1, wxALL|wxEXPAND, 5);
	BitmapButton_Fireworks_YVelocity = new BulkEditValueCurveButton(this, ID_VALUECURVE_Fireworks_YVelocity, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Fireworks_YVelocity"));
	FlexGridSizer4->Add(BitmapButton_Fireworks_YVelocity, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer73->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 5);
	TextCtrl2 = new BulkEditTextCtrl(this, ID_TEXTCTRL_Fireworks_YVelocity, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Fireworks_YVelocity"));
	TextCtrl2->SetMaxLength(3);
	FlexGridSizer73->Add(TextCtrl2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer73->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT3, _("X Location"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer73->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer5->AddGrowableCol(0);
	Slider_Fireworks_XLocation = new BulkEditSlider(this, ID_SLIDER_Fireworks_XLocation, -1, -1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fireworks_XLocation"));
	FlexGridSizer5->Add(Slider_Fireworks_XLocation, 1, wxALL|wxEXPAND, 5);
	BitmapButton_Fireworks_XLocation = new BulkEditValueCurveButton(this, ID_VALUECURVE_Fireworks_XLocation, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Fireworks_XLocation"));
	FlexGridSizer5->Add(BitmapButton_Fireworks_XLocation, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer73->Add(FlexGridSizer5, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Fireworks_XLocation = new BulkEditTextCtrl(this, ID_TEXTCTRL_Fireworks_XLocation, _("-1"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Fireworks_XLocation"));
	TextCtrl_Fireworks_XLocation->SetMaxLength(3);
	FlexGridSizer73->Add(TextCtrl_Fireworks_XLocation, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer73->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT4, _("Y Location"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer73->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer6->AddGrowableCol(0);
	Slider_Fireworks_YLocation = new BulkEditSlider(this, ID_SLIDER_Fireworks_YLocation, -1, -1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fireworks_YLocation"));
	FlexGridSizer6->Add(Slider_Fireworks_YLocation, 1, wxALL|wxEXPAND, 5);
	BitmapButton_Fireworks_YLocation = new BulkEditValueCurveButton(this, ID_VALUECURVE_Fireworks_YLocation, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Fireworks_YLocation"));
	FlexGridSizer6->Add(BitmapButton_Fireworks_YLocation, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer73->Add(FlexGridSizer6, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Fireworks_YLocation = new BulkEditTextCtrl(this, ID_TEXTCTRL_Fireworks_YLocation, _("-1"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Fireworks_YLocation"));
	TextCtrl_Fireworks_YLocation->SetMaxLength(3);
	FlexGridSizer73->Add(TextCtrl_Fireworks_YLocation, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer73->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer73->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Fireworks_HoldColor = new BulkEditCheckBox(this, ID_CHECKBOX_Fireworks_HoldColour, _("Hold color"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Fireworks_HoldColour"));
	CheckBox_Fireworks_HoldColor->SetValue(true);
	FlexGridSizer73->Add(CheckBox_Fireworks_HoldColor, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer73->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer73->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer73->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Fireworks_Gravity = new BulkEditCheckBox(this, ID_CHECKBOX_Fireworks_Gravity, _("Gravity"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Fireworks_Gravity"));
	CheckBox_Fireworks_Gravity->SetValue(true);
	FlexGridSizer73->Add(CheckBox_Fireworks_Gravity, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer73->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer73->Add(-1,-1,1, wxALL, 5);
	StaticText95 = new wxStaticText(this, ID_STATICTEXT_Fireworks_Fade, _("Particle Fade"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fireworks_Fade"));
	FlexGridSizer73->Add(StaticText95, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer7 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer7->AddGrowableCol(0);
	Slider_Fireworks_Fade = new BulkEditSlider(this, ID_SLIDER_Fireworks_Fade, 50, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fireworks_Fade"));
	FlexGridSizer7->Add(Slider_Fireworks_Fade, 1, wxALL|wxEXPAND, 5);
	BitmapButton_Fireworks_Fade = new BulkEditValueCurveButton(this, ID_VALUECURVE_Fireworks_Fade, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Fireworks_Fade"));
	FlexGridSizer7->Add(BitmapButton_Fireworks_Fade, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer73->Add(FlexGridSizer7, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Fireworks_Fade = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Fireworks_Fade, _("50"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Fireworks_Fade"));
	TextCtrl_Fireworks_Fade->SetMaxLength(3);
	FlexGridSizer73->Add(TextCtrl_Fireworks_Fade, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_FireworksFade = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Fireworks_Fade, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fireworks_Fade"));
	BitmapButton_FireworksFade->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer73->Add(BitmapButton_FireworksFade, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer73->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Fireworks_UseMusic = new BulkEditCheckBox(this, ID_CHECKBOX_Fireworks_UseMusic, _("Fire with music"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Fireworks_UseMusic"));
	CheckBox_Fireworks_UseMusic->SetValue(false);
	FlexGridSizer73->Add(CheckBox_Fireworks_UseMusic, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer73->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Fireworks_UseMusic = new xlLockButton(this, ID_BITMAPBUTTON_CHECKBOX_Fireworks_UseMusic, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Fireworks_UseMusic"));
	BitmapButton_Fireworks_UseMusic->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer73->Add(BitmapButton_Fireworks_UseMusic, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT_Fireworks_Sensitivity, _("Trigger level"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fireworks_Sensitivity"));
	FlexGridSizer73->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fireworks_Sensitivity = new BulkEditSlider(this, ID_SLIDER_Fireworks_Sensitivity, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fireworks_Sensitivity"));
	FlexGridSizer73->Add(Slider_Fireworks_Sensitivity, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Fireworks_Sensitivity = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Fireworks_Sensitivity, _("50"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Fireworks_Sensitivity"));
	TextCtrl_Fireworks_Sensitivity->SetMaxLength(3);
	FlexGridSizer73->Add(TextCtrl_Fireworks_Sensitivity, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Fireworks_Sensitivity = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Fireworks_Sensitivity, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fireworks_Sensitivity"));
	BitmapButton_Fireworks_Sensitivity->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer73->Add(BitmapButton_Fireworks_Sensitivity, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer73->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_FireTiming = new BulkEditCheckBox(this, ID_CHECKBOX_FIRETIMING, _("Fire with timing track"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_FIRETIMING"));
	CheckBox_FireTiming->SetValue(false);
	FlexGridSizer73->Add(CheckBox_FireTiming, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer73->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton1 = new xlLockButton(this, ID_BITMAPBUTTON_CHECKBOX_FIRETIMING, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_FIRETIMING"));
	BitmapButton1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer73->Add(BitmapButton1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT_FIRETIMINGTRACK, _("Timing Track"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_FIRETIMINGTRACK"));
	FlexGridSizer73->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_TimingTrack = new BulkEditChoice(this, ID_CHOICE_FIRETIMINGTRACK, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_FIRETIMINGTRACK"));
	FlexGridSizer73->Add(Choice_TimingTrack, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer73->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer73->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer73->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer73);
	FlexGridSizer73->Fit(this);
	FlexGridSizer73->SetSizeHints(this);

	Connect(ID_BITMAPBUTTON_SLIDER_Fireworks_Explosions,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Fireworks_Count,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fireworks_Count,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Fireworks_Velocity,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fireworks_Velocity,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Fireworks_XVelocity,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_Fireworks_YVelocity,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_Fireworks_XLocation,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_Fireworks_YLocation,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_Fireworks_Fade,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fireworks_Fade,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnLockButtonClick);
	Connect(ID_CHECKBOX_Fireworks_UseMusic,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnCheckBox_Fireworks_UseMusicClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Fireworks_UseMusic,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fireworks_Sensitivity,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnLockButtonClick);
	Connect(ID_CHECKBOX_FIRETIMING,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnCheckBox_FireTimingClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_FIRETIMING,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FireworksPanel::OnLockButtonClick);
	Connect(ID_CHOICE_FIRETIMINGTRACK,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&FireworksPanel::OnChoice_TimingTrackSelect);
	//*)

    SetName("ID_PANEL_FIREWORKS");

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&FireworksPanel::OnVCChanged, 0, this);
   Connect(wxID_ANY, EVT_VALIDATEWINDOW, (wxObjectEventFunction)&FireworksPanel::OnValidateWindow, 0, this);

    BitmapButton_Fireworks_Count->GetValue()->SetLimits(FIREWORKSCOUNT_MIN, FIREWORKSCOUNT_MAX);
    BitmapButton_Fireworks_Velocity->GetValue()->SetLimits(FIREWORKSVELOCITY_MIN, FIREWORKSVELOCITY_MAX);
    BitmapButton_Fireworks_XVelocity->GetValue()->SetLimits(FIREWORKSXVELOCITY_MIN, FIREWORKSXVELOCITY_MAX);
    BitmapButton_Fireworks_YVelocity->GetValue()->SetLimits(FIREWORKSXVELOCITY_MIN, FIREWORKSYVELOCITY_MAX);
    BitmapButton_Fireworks_XLocation->GetValue()->SetLimits(FIREWORKSXLOCATION_MIN, FIREWORKSXLOCATION_MAX);
    BitmapButton_Fireworks_YLocation->GetValue()->SetLimits(FIREWORKSXLOCATION_MIN, FIREWORKSYLOCATION_MAX);
    BitmapButton_Fireworks_Fade->GetValue()->SetLimits(FIREWORKSFADE_MIN, FIREWORKSFADE_MAX);

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

    if (CheckBox_FireTiming->GetValue() || CheckBox_Fireworks_UseMusic->GetValue())
    {
        Slider_Fireworks_Num_Explosions->Disable();
        TextCtrl_Fireworks_Explosions->Disable();
    }
    else
    {
        Slider_Fireworks_Num_Explosions->Enable();
        TextCtrl_Fireworks_Explosions->Enable();
    }
}

void FireworksPanel::OnCheckBox_Fireworks_UseMusicClick(wxCommandEvent& event)
{
    ValidateWindow();
}

void FireworksPanel::OnCheckBox_FireTimingClick(wxCommandEvent& event)
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

    for (size_t i = 0; i < Choice_TimingTrack->GetCount(); i++)
    {
        bool found = false;
        for (const auto& it : timingtracks)
        {
            if (it == Choice_TimingTrack->GetString(i))
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            Choice_TimingTrack->Clear();
            break;
        }
    }

    // add any new timing tracks
    for (const auto& it : timingtracks)
    {
        bool found = false;
        for (size_t i = 0; i < Choice_TimingTrack->GetCount(); i++)
        {
            if (it == Choice_TimingTrack->GetString(i))
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            Choice_TimingTrack->Append(it);
        }
    }

    if (selection != "")
    {
        Choice_TimingTrack->SetStringSelection(selection);
    }

    if (Choice_TimingTrack->GetStringSelection() == "" && Choice_TimingTrack->GetCount() > 0)
    {
        Choice_TimingTrack->SetSelection(0);
    }

    ValidateWindow();
}
