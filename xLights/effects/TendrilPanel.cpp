#include "TendrilPanel.h"
#include "../../include/padlock16x16-blue.xpm"
#include "EffectPanelUtils.h"

//(*InternalHeaders(TendrilPanel)
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

#include <wx/valnum.h>

//(*IdInit(TendrilPanel)
const long TendrilPanel::ID_STATICTEXT1 = wxNewId();
const long TendrilPanel::ID_CHOICE_Tendril_Movement = wxNewId();
const long TendrilPanel::ID_STATICTEXT9 = wxNewId();
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
const long TendrilPanel::ID_STATICTEXT10 = wxNewId();
const long TendrilPanel::IDD_SLIDER_Tendril_XOffset = wxNewId();
const long TendrilPanel::ID_TEXTCTRL_Tendril_XOffset = wxNewId();
const long TendrilPanel::ID_BITMAPBUTTON_Tendril_XOffset = wxNewId();
const long TendrilPanel::ID_STATICTEXT11 = wxNewId();
const long TendrilPanel::IDD_SLIDER_Tendril_YOffset = wxNewId();
const long TendrilPanel::ID_TEXTCTRL_Tendril_YOffset = wxNewId();
const long TendrilPanel::ID_BITMAPBUTTON_Tendril_YOffset = wxNewId();
//*)

BEGIN_EVENT_TABLE(TendrilPanel,wxPanel)
	//(*EventTable(TendrilPanel)
	//*)
END_EVENT_TABLE()

TendrilPanel::TendrilPanel(wxWindow* parent)
{
    wxIntegerValidator<int> _tuneMovement(&__tuneMovement, wxNUM_VAL_THOUSANDS_SEPARATOR);
    _tuneMovement.SetMin(0);
    _tuneMovement.SetMax(20);
    wxIntegerValidator<int> _thickness(&__thickness, wxNUM_VAL_THOUSANDS_SEPARATOR);
    _thickness.SetMin(1);
    _thickness.SetMax(20);
    wxIntegerValidator<int> _friction(&__friction, wxNUM_VAL_THOUSANDS_SEPARATOR);
    _friction.SetMin(0);
    _friction.SetMax(20);
    wxIntegerValidator<int> _dampening(&__dampening, wxNUM_VAL_THOUSANDS_SEPARATOR);
    _dampening.SetMin(0);
    _dampening.SetMax(20);
    wxIntegerValidator<int> _tension(&__tension, wxNUM_VAL_THOUSANDS_SEPARATOR);
    _tension.SetMin(0);
    _tension.SetMax(39);
    wxIntegerValidator<int> _trails(&__trails, wxNUM_VAL_THOUSANDS_SEPARATOR);
    _trails.SetMin(1);
    _trails.SetMax(20);
    wxIntegerValidator<int> _length(&__length, wxNUM_VAL_THOUSANDS_SEPARATOR);
    _length.SetMin(5);
    _length.SetMax(100);
    wxIntegerValidator<int> _speed(&__speed, wxNUM_VAL_THOUSANDS_SEPARATOR);
    _speed.SetMin(1);
    _speed.SetMax(10);
    wxIntegerValidator<int> _horizontalOffset(&__horizontalOffset, wxNUM_VAL_THOUSANDS_SEPARATOR);
    _horizontalOffset.SetMin(-100);
    _horizontalOffset.SetMax(100);
    wxIntegerValidator<int> _verticalOffset(&__verticalOffset, wxNUM_VAL_THOUSANDS_SEPARATOR);
    _verticalOffset.SetMin(-100);
    _verticalOffset.SetMax(100);

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
	Choice_Tendril_Movement = new wxChoice(this, ID_CHOICE_Tendril_Movement, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Tendril_Movement"));
	Choice_Tendril_Movement->Append(_("Random"));
	Choice_Tendril_Movement->Append(_("Square"));
	Choice_Tendril_Movement->SetSelection( Choice_Tendril_Movement->Append(_("Circle")) );
	Choice_Tendril_Movement->Append(_("Horizontal Zig Zag"));
	Choice_Tendril_Movement->Append(_("Horiz. Zig Zag Return"));
	Choice_Tendril_Movement->Append(_("Vertical Zig Zag"));
	Choice_Tendril_Movement->Append(_("Vert. Zig Zag Return"));
	Choice_Tendril_Movement->Append(_("Music Line"));
	Choice_Tendril_Movement->Append(_("Music Circle"));
	FlexGridSizer31->Add(Choice_Tendril_Movement, 1, wxALL|wxEXPAND, 2);
	StaticText9 = new wxStaticText(this, ID_STATICTEXT9, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
	FlexGridSizer31->Add(StaticText9, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Tendril_Movement = new wxBitmapButton(this, ID_BITMAPBUTTON_CHOICE_Tendril_Movement, wxNullBitmap, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Tendril_Movement"));
	BitmapButton_Tendril_Movement->SetDefault();
	BitmapButton_Tendril_Movement->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer31->Add(BitmapButton_Tendril_Movement, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText8 = new wxStaticText(this, ID_STATICTEXT8, _("Tune Movement"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
	FlexGridSizer31->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Tendril_TuneMovement = new wxSlider(this, IDD_SLIDER_Tendril_TuneMovement, 10, 0, 20, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Tendril_TuneMovement"));
	FlexGridSizer31->Add(Slider_Tendril_TuneMovement, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Tendril_TuneMovement = new wxTextCtrl(this, ID_TEXTCTRL_Tendril_TuneMovement, _("10"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, _tuneMovement, _T("ID_TEXTCTRL_Tendril_TuneMovement"));
	FlexGridSizer31->Add(TextCtrl_Tendril_TuneMovement, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Tendril_TuneMovement = new wxBitmapButton(this, ID_BITMAPBUTTON_CHOICE_Tendril_TuneMovement, wxNullBitmap, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Tendril_TuneMovement"));
	BitmapButton_Tendril_TuneMovement->SetDefault();
	BitmapButton_Tendril_TuneMovement->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer31->Add(BitmapButton_Tendril_TuneMovement, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText7 = new wxStaticText(this, ID_STATICTEXT7, _("Thickness"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	FlexGridSizer31->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Tendril_Thickness = new wxSlider(this, IDD_SLIDER_Tendril_Thickness, 3, 1, 20, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Tendril_Thickness"));
	FlexGridSizer31->Add(Slider_Tendril_Thickness, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Tendril_Thickness = new wxTextCtrl(this, ID_TEXTCTRL_Tendril_Thickness, _("3"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, _thickness, _T("ID_TEXTCTRL_Tendril_Thickness"));
	FlexGridSizer31->Add(TextCtrl_Tendril_Thickness, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Tendril_Thickness = new wxBitmapButton(this, ID_BITMAPBUTTON_CHOICE_Tendril_Thickness, wxNullBitmap, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Tendril_Thickness"));
	BitmapButton_Tendril_Thickness->SetDefault();
	BitmapButton_Tendril_Thickness->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer31->Add(BitmapButton_Tendril_Thickness, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Friction"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer31->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Tendril_Friction = new wxSlider(this, IDD_SLIDER_Tendril_Friction, 10, 0, 20, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Tendril_Friction"));
	FlexGridSizer31->Add(Slider_Tendril_Friction, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Tendril_Friction = new wxTextCtrl(this, ID_TEXTCTRL_Tendril_Friction, _("10"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, _friction, _T("ID_TEXTCTRL_Tendril_Friction"));
	FlexGridSizer31->Add(TextCtrl_Tendril_Friction, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_TendrilFriction = new wxBitmapButton(this, ID_BITMAPBUTTON_CHOICE_Tendril_Friction, wxNullBitmap, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Tendril_Friction"));
	BitmapButton_TendrilFriction->SetDefault();
	BitmapButton_TendrilFriction->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer31->Add(BitmapButton_TendrilFriction, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Dampening"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer31->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Tendril_Dampening = new wxSlider(this, IDD_SLIDER_Tendril_Dampening, 10, 0, 20, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Tendril_Dampening"));
	FlexGridSizer31->Add(Slider_Tendril_Dampening, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Tendril_Dampening = new wxTextCtrl(this, ID_TEXTCTRL_Tendril_Dampening, _("10"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, _dampening, _T("ID_TEXTCTRL_Tendril_Dampening"));
	FlexGridSizer31->Add(TextCtrl_Tendril_Dampening, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_TendrilDampening = new wxBitmapButton(this, ID_BITMAPBUTTON_CHOICE_Tendril_Dampening, wxNullBitmap, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Tendril_Dampening"));
	BitmapButton_TendrilDampening->SetDefault();
	BitmapButton_TendrilDampening->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer31->Add(BitmapButton_TendrilDampening, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Tension"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer31->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Tendril_Tension = new wxSlider(this, IDD_SLIDER_Tendril_Tension, 20, 0, 39, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Tendril_Tension"));
	FlexGridSizer31->Add(Slider_Tendril_Tension, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Tendril_Tension = new wxTextCtrl(this, ID_TEXTCTRL_Tendril_Tension, _("20"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, _tension, _T("ID_TEXTCTRL_Tendril_Tension"));
	FlexGridSizer31->Add(TextCtrl_Tendril_Tension, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_TendrilTension = new wxBitmapButton(this, ID_BITMAPBUTTON_CHOICE_Tendril_Tension, wxNullBitmap, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Tendril_Tension"));
	BitmapButton_TendrilTension->SetDefault();
	BitmapButton_TendrilTension->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer31->Add(BitmapButton_TendrilTension, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Trails"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer31->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Tendril_Trails = new wxSlider(this, IDD_SLIDER_Tendril_Trails, 1, 1, 20, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Tendril_Trails"));
	FlexGridSizer31->Add(Slider_Tendril_Trails, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Tendril_Trails = new wxTextCtrl(this, ID_TEXTCTRL_Tendril_Trails, _("1"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, _trails, _T("ID_TEXTCTRL_Tendril_Trails"));
	FlexGridSizer31->Add(TextCtrl_Tendril_Trails, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_TendrilTrails = new wxBitmapButton(this, ID_BITMAPBUTTON_CHOICE_Tendril_Trails, wxNullBitmap, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Tendril_Trails"));
	BitmapButton_TendrilTrails->SetDefault();
	BitmapButton_TendrilTrails->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer31->Add(BitmapButton_TendrilTrails, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Length"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer31->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Tendril_Length = new wxSlider(this, IDD_SLIDER_Tendril_Length, 60, 5, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Tendril_Length"));
	FlexGridSizer31->Add(Slider_Tendril_Length, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Tendril_Length = new wxTextCtrl(this, ID_TEXTCTRL_Tendril_Length, _("60"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, _length, _T("ID_TEXTCTRL_Tendril_Length"));
	FlexGridSizer31->Add(TextCtrl_Tendril_Length, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_TendrilLength = new wxBitmapButton(this, ID_BITMAPBUTTON_CHOICE_Tendril_Length, wxNullBitmap, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Tendril_Length"));
	BitmapButton_TendrilLength->SetDefault();
	BitmapButton_TendrilLength->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer31->Add(BitmapButton_TendrilLength, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText68 = new wxStaticText(this, ID_STATICTEXT27, _("Speed"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT27"));
	FlexGridSizer31->Add(StaticText68, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Tendril_Speed = new wxSlider(this, IDD_SLIDER_Tendril_Speed, 10, 1, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Tendril_Speed"));
	FlexGridSizer31->Add(Slider_Tendril_Speed, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Tendril_Speed = new wxTextCtrl(this, ID_TEXTCTRL_Tendril_Speed, _("10"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, _speed, _T("ID_TEXTCTRL_Tendril_Speed"));
	FlexGridSizer31->Add(TextCtrl_Tendril_Speed, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_TendrilSpeed = new wxBitmapButton(this, IID_BITMAPBUTTON_CHOICE_Tendril_Speed, wxNullBitmap, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("IID_BITMAPBUTTON_CHOICE_Tendril_Speed"));
	BitmapButton_TendrilSpeed->SetDefault();
	BitmapButton_TendrilSpeed->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer31->Add(BitmapButton_TendrilSpeed, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText10 = new wxStaticText(this, ID_STATICTEXT10, _("Horizontal Offset"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT10"));
	FlexGridSizer31->Add(StaticText10, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Tendril_XOffset = new wxSlider(this, IDD_SLIDER_Tendril_XOffset, 0, -100, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Tendril_XOffset"));
	FlexGridSizer31->Add(Slider_Tendril_XOffset, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Tendril_XOffset = new wxTextCtrl(this, ID_TEXTCTRL_Tendril_XOffset, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, _horizontalOffset, _T("ID_TEXTCTRL_Tendril_XOffset"));
	FlexGridSizer31->Add(TextCtrl_Tendril_XOffset, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Tendril_XOffset = new wxBitmapButton(this, ID_BITMAPBUTTON_Tendril_XOffset, wxNullBitmap, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Tendril_XOffset"));
	BitmapButton_Tendril_XOffset->SetDefault();
	BitmapButton_Tendril_XOffset->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer31->Add(BitmapButton_Tendril_XOffset, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText11 = new wxStaticText(this, ID_STATICTEXT11, _("Vertical Offset"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT11"));
	FlexGridSizer31->Add(StaticText11, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Tendril_YOffset = new wxSlider(this, IDD_SLIDER_Tendril_YOffset, 0, -100, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Tendril_YOffset"));
	FlexGridSizer31->Add(Slider_Tendril_YOffset, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Tendril_YOffset = new wxTextCtrl(this, ID_TEXTCTRL_Tendril_YOffset, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, _verticalOffset, _T("ID_TEXTCTRL_Tendril_YOffset"));
	FlexGridSizer31->Add(TextCtrl_Tendril_YOffset, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Tendril_YOffset = new wxBitmapButton(this, ID_BITMAPBUTTON_Tendril_YOffset, wxNullBitmap, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Tendril_YOffset"));
	BitmapButton_Tendril_YOffset->SetDefault();
	BitmapButton_Tendril_YOffset->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer31->Add(BitmapButton_Tendril_YOffset, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer42->Add(FlexGridSizer31, 1, wxEXPAND, 2);
	SetSizer(FlexGridSizer42);
	FlexGridSizer42->Fit(this);
	FlexGridSizer42->SetSizeHints(this);

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
	Connect(IDD_SLIDER_Tendril_XOffset,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&TendrilPanel::UpdateLinkedTextCtrl);
	Connect(ID_TEXTCTRL_Tendril_XOffset,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&TendrilPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_Tendril_XOffset,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TendrilPanel::OnLockButtonClick);
	Connect(IDD_SLIDER_Tendril_YOffset,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&TendrilPanel::UpdateLinkedTextCtrl);
	Connect(ID_TEXTCTRL_Tendril_YOffset,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&TendrilPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_Tendril_YOffset,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TendrilPanel::OnLockButtonClick);
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
