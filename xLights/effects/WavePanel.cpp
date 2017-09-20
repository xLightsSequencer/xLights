#include "WavePanel.h"
#include "../../include/padlock16x16-blue.xpm"
#include "../../include/padlock16x16-green.xpm"
#include "EffectPanelUtils.h"
#include "WaveEffect.h"

//(*InternalHeaders(WavePanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/bitmap.h>
#include <wx/slider.h>
#include <wx/settings.h>
#include <wx/choice.h>
#include <wx/bmpbuttn.h>
#include <wx/intl.h>
#include <wx/image.h>
#include <wx/string.h>
#include "WaveEffect.h"
//*)

//(*IdInit(WavePanel)
const long WavePanel::ID_STATICTEXT49 = wxNewId();
const long WavePanel::ID_CHOICE_Wave_Type = wxNewId();
const long WavePanel::ID_BITMAPBUTTON2 = wxNewId();
const long WavePanel::ID_STATICTEXT50 = wxNewId();
const long WavePanel::ID_CHOICE_Fill_Colors = wxNewId();
const long WavePanel::ID_BITMAPBUTTON3 = wxNewId();
const long WavePanel::ID_STATICTEXT52 = wxNewId();
const long WavePanel::ID_CHECKBOX_Mirror_Wave = wxNewId();
const long WavePanel::ID_BITMAPBUTTON4 = wxNewId();
const long WavePanel::ID_STATICTEXT55 = wxNewId();
const long WavePanel::ID_SLIDER_Number_Waves = wxNewId();
const long WavePanel::ID_VALUECURVE_Number_Waves = wxNewId();
const long WavePanel::IDD_TEXTCTRL_Number_Waves = wxNewId();
const long WavePanel::ID_BITMAPBUTTON5 = wxNewId();
const long WavePanel::ID_STATICTEXT56 = wxNewId();
const long WavePanel::ID_SLIDER_Thickness_Percentage = wxNewId();
const long WavePanel::ID_VALUECURVE_Thickness_Percentage = wxNewId();
const long WavePanel::IDD_TEXTCTRL_Thickness_Percentage = wxNewId();
const long WavePanel::ID_BITMAPBUTTON6 = wxNewId();
const long WavePanel::ID_STATICTEXT58 = wxNewId();
const long WavePanel::ID_SLIDER_Wave_Height = wxNewId();
const long WavePanel::ID_VALUECURVE_Wave_Height = wxNewId();
const long WavePanel::IDD_TEXTCTRL_Wave_Height = wxNewId();
const long WavePanel::ID_BITMAPBUTTON7 = wxNewId();
const long WavePanel::ID_STATICTEXT32 = wxNewId();
const long WavePanel::ID_SLIDER_Wave_Speed = wxNewId();
const long WavePanel::ID_VALUECURVE_Wave_Speed = wxNewId();
const long WavePanel::IDD_TEXTCTRL_Wave_Speed = wxNewId();
const long WavePanel::ID_BITMAPBUTTON31 = wxNewId();
const long WavePanel::ID_STATICTEXT59 = wxNewId();
const long WavePanel::ID_CHOICE_Wave_Direction = wxNewId();
const long WavePanel::ID_BITMAPBUTTON8 = wxNewId();
const long WavePanel::ID_STATICTEXT57 = wxNewId();
const long WavePanel::ID_SLIDER_Wave_Y_Position = wxNewId();
const long WavePanel::ID_VALUECURVE_Wave_Y_Position = wxNewId();
const long WavePanel::IDD_TEXTCTRL_Wave_Y_Position = wxNewId();
const long WavePanel::ID_BITMAPBUTTON57 = wxNewId();
//*)

BEGIN_EVENT_TABLE(WavePanel,wxPanel)
	//(*EventTable(WavePanel)
	//*)
END_EVENT_TABLE()

WavePanel::WavePanel(wxWindow* parent)
{
	//(*Initialize(WavePanel)
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer51;
	wxFlexGridSizer* FlexGridSizer3;
	wxTextCtrl* TextCtrl78;
	wxTextCtrl* TextCtrl76;
	wxTextCtrl* TextCtrl75;
	wxFlexGridSizer* FlexGridSizer2;
	wxTextCtrl* TextCtrl77;
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer5;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer51 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer51->AddGrowableCol(1);
	StaticText47 = new wxStaticText(this, ID_STATICTEXT49, _("Wave Type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT49"));
	FlexGridSizer51->Add(StaticText47, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Wave_Type = new wxChoice(this, ID_CHOICE_Wave_Type, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Wave_Type"));
	Choice_Wave_Type->SetSelection( Choice_Wave_Type->Append(_("Sine")) );
	Choice_Wave_Type->Append(_("Triangle"));
	Choice_Wave_Type->Append(_("Square"));
	Choice_Wave_Type->Append(_("Decaying Sine"));
	Choice_Wave_Type->Append(_("Fractal/ivy"));
	FlexGridSizer51->Add(Choice_Wave_Type, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer51->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_WaveType = new wxBitmapButton(this, ID_BITMAPBUTTON2, padlock16x16_green_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON2"));
	BitmapButton_WaveType->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer51->Add(BitmapButton_WaveType, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText48 = new wxStaticText(this, ID_STATICTEXT50, _("Fill Colors"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT50"));
	FlexGridSizer51->Add(StaticText48, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Fill_Colors = new wxChoice(this, ID_CHOICE_Fill_Colors, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Fill_Colors"));
	Choice_Fill_Colors->SetSelection( Choice_Fill_Colors->Append(_("None")) );
	Choice_Fill_Colors->Append(_("Rainbow"));
	Choice_Fill_Colors->Append(_("Palette"));
	FlexGridSizer51->Add(Choice_Fill_Colors, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer51->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Wave_FillColors = new wxBitmapButton(this, ID_BITMAPBUTTON3, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON3"));
	BitmapButton_Wave_FillColors->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer51->Add(BitmapButton_Wave_FillColors, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText49 = new wxStaticText(this, ID_STATICTEXT52, _("Mirror Wave"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT52"));
	FlexGridSizer51->Add(StaticText49, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Mirror_Wave = new wxCheckBox(this, ID_CHECKBOX_Mirror_Wave, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Mirror_Wave"));
	CheckBox_Mirror_Wave->SetValue(false);
	FlexGridSizer51->Add(CheckBox_Mirror_Wave, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer51->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Wave_MirrorWave = new wxBitmapButton(this, ID_BITMAPBUTTON4, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON4"));
	BitmapButton_Wave_MirrorWave->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer51->Add(BitmapButton_Wave_MirrorWave, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText50 = new wxStaticText(this, ID_STATICTEXT55, _("Number of Waves"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT55"));
	FlexGridSizer51->Add(StaticText50, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	Slider_Number_Waves = new wxSlider(this, ID_SLIDER_Number_Waves, 900, 180, 3600, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Number_Waves"));
	FlexGridSizer1->Add(Slider_Number_Waves, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Number_WavesVC = new ValueCurveButton(this, ID_VALUECURVE_Number_Waves, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Number_Waves"));
	FlexGridSizer1->Add(BitmapButton_Number_WavesVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer51->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 2);
	TextCtrl75 = new wxTextCtrl(this, IDD_TEXTCTRL_Number_Waves, _("900"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Number_Waves"));
	TextCtrl75->SetMaxLength(4);
	FlexGridSizer51->Add(TextCtrl75, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_NumberWaves = new wxBitmapButton(this, ID_BITMAPBUTTON5, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON5"));
	BitmapButton_NumberWaves->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer51->Add(BitmapButton_NumberWaves, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText52 = new wxStaticText(this, ID_STATICTEXT56, _("Thickness of Wave"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT56"));
	FlexGridSizer51->Add(StaticText52, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	Slider_Thickness_Percentage = new wxSlider(this, ID_SLIDER_Thickness_Percentage, 5, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Thickness_Percentage"));
	FlexGridSizer2->Add(Slider_Thickness_Percentage, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Thickness_PercentageVC = new ValueCurveButton(this, ID_VALUECURVE_Thickness_Percentage, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Thickness_Percentage"));
	FlexGridSizer2->Add(BitmapButton_Thickness_PercentageVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer51->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 2);
	TextCtrl76 = new wxTextCtrl(this, IDD_TEXTCTRL_Thickness_Percentage, _("5"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Thickness_Percentage"));
	TextCtrl76->SetMaxLength(3);
	FlexGridSizer51->Add(TextCtrl76, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Wave_Thickness = new wxBitmapButton(this, ID_BITMAPBUTTON6, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON6"));
	BitmapButton_Wave_Thickness->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer51->Add(BitmapButton_Wave_Thickness, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText55 = new wxStaticText(this, ID_STATICTEXT58, _("Scale Height of Wave"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT58"));
	FlexGridSizer51->Add(StaticText55, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	Slider_Wave_Height = new wxSlider(this, ID_SLIDER_Wave_Height, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Wave_Height"));
	FlexGridSizer3->Add(Slider_Wave_Height, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Wave_HeightVC = new ValueCurveButton(this, ID_VALUECURVE_Wave_Height, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Wave_Height"));
	FlexGridSizer3->Add(BitmapButton_Wave_HeightVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer51->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 2);
	TextCtrl77 = new wxTextCtrl(this, IDD_TEXTCTRL_Wave_Height, _("50"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Wave_Height"));
	TextCtrl77->SetMaxLength(3);
	FlexGridSizer51->Add(TextCtrl77, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Wave_Height = new wxBitmapButton(this, ID_BITMAPBUTTON7, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON7"));
	BitmapButton_Wave_Height->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer51->Add(BitmapButton_Wave_Height, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    StaticText189 = new wxStaticText(this, ID_STATICTEXT32, _("Speed"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT32"));
	FlexGridSizer51->Add(StaticText189, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer4->AddGrowableCol(0);
	Slider_Wave_Speed = new wxSlider(this, ID_SLIDER_Wave_Speed, 10, 0, 50, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Wave_Speed"));
	FlexGridSizer4->Add(Slider_Wave_Speed, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Wave_SpeedVC = new ValueCurveButton(this, ID_VALUECURVE_Wave_Speed, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Wave_Speed"));
	FlexGridSizer4->Add(BitmapButton_Wave_SpeedVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer51->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 2);
    TextCtrl78 = new wxTextCtrl(this, IDD_TEXTCTRL_Wave_Speed, _("10"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Wave_Speed"));
	TextCtrl78->SetMaxLength(3);
	FlexGridSizer51->Add(TextCtrl78, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton13 = new wxBitmapButton(this, ID_BITMAPBUTTON31, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON31"));
	BitmapButton13->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer51->Add(BitmapButton13, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText57 = new wxStaticText(this, ID_STATICTEXT57, _("Y Position"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT57"));
    FlexGridSizer51->Add(StaticText57, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer5->AddGrowableCol(0);
    Slider_Wave_Y_Position = new wxSlider(this, ID_SLIDER_Wave_Y_Position, 0, WAVE_Y_POSITION_MIN, WAVE_Y_POSITION_MAX, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Wave_Y_Position"));
    FlexGridSizer5->Add(Slider_Wave_Y_Position, 1, wxALL|wxEXPAND, 2);
    BitmapButton_Wave_Y_PositionVC = new ValueCurveButton(this, ID_VALUECURVE_Wave_Y_Position, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Wave_Y_Position"));
    FlexGridSizer5->Add(BitmapButton_Wave_Y_PositionVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer51->Add(FlexGridSizer5, 1, wxALL|wxEXPAND, 2);
    TextCtrl_Wave_Y_Position = new wxTextCtrl(this, IDD_TEXTCTRL_Wave_Y_Position, _("10"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Wave_Y_Position"));
    TextCtrl_Wave_Y_Position->SetMaxLength(3);
    FlexGridSizer51->Add(TextCtrl_Wave_Y_Position, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_Wave_Y_Position = new wxBitmapButton(this, ID_BITMAPBUTTON57, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON57"));
    BitmapButton_Wave_Y_Position->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer51->Add(BitmapButton_Wave_Y_Position, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText56 = new wxStaticText(this, ID_STATICTEXT59, _("Wave Direction"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT59"));
	FlexGridSizer51->Add(StaticText56, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Wave_Direction = new wxChoice(this, ID_CHOICE_Wave_Direction, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Wave_Direction"));
	Choice_Wave_Direction->SetSelection( Choice_Wave_Direction->Append(_("Right to Left")) );
	Choice_Wave_Direction->Append(_("Left to Right"));
	FlexGridSizer51->Add(Choice_Wave_Direction, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer51->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Wave_Direction = new wxBitmapButton(this, ID_BITMAPBUTTON8, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON8"));
	BitmapButton_Wave_Direction->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer51->Add(BitmapButton_Wave_Direction, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
        SetSizer(FlexGridSizer51);
	FlexGridSizer51->Fit(this);
	FlexGridSizer51->SetSizeHints(this);
    
	Connect(ID_BITMAPBUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&WavePanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&WavePanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&WavePanel::OnLockButtonClick);
	Connect(ID_SLIDER_Number_Waves,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&WavePanel::UpdateLinkedTextCtrlVC);
	Connect(ID_VALUECURVE_Number_Waves,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&WavePanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_Number_Waves,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&WavePanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&WavePanel::OnLockButtonClick);
	Connect(ID_SLIDER_Thickness_Percentage,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&WavePanel::UpdateLinkedTextCtrlVC);
	Connect(ID_VALUECURVE_Thickness_Percentage,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&WavePanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_Thickness_Percentage,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&WavePanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON6,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&WavePanel::OnLockButtonClick);
	Connect(ID_SLIDER_Wave_Height,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&WavePanel::UpdateLinkedTextCtrlVC);
	Connect(ID_VALUECURVE_Wave_Height,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&WavePanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_Wave_Height,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&WavePanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON7,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&WavePanel::OnLockButtonClick);
	Connect(ID_SLIDER_Wave_Speed,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&WavePanel::UpdateLinkedTextCtrlVC);
	Connect(ID_VALUECURVE_Wave_Speed,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&WavePanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_Wave_Speed,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&WavePanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON31,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&WavePanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON8,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&WavePanel::OnLockButtonClick);
    Connect(ID_SLIDER_Wave_Y_Position,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&WavePanel::UpdateLinkedTextCtrlVC);
    Connect(ID_VALUECURVE_Wave_Y_Position,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&WavePanel::OnVCButtonClick);
    Connect(IDD_TEXTCTRL_Wave_Y_Position,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&WavePanel::UpdateLinkedSlider);
	//*)
    SetName("ID_PANEL_WAVE");

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&WavePanel::OnVCChanged, 0, this);

    BitmapButton_Number_WavesVC->GetValue()->SetLimits(WAVE_NUMBER_MIN, WAVE_NUMBER_MAX);
    BitmapButton_Thickness_PercentageVC->GetValue()->SetLimits(WAVE_THICKNESS_MIN, WAVE_THICKNESS_MAX);
    BitmapButton_Wave_HeightVC->GetValue()->SetLimits(WAVE_HEIGHT_MIN, WAVE_HEIGHT_MAX);
    BitmapButton_Wave_SpeedVC->GetValue()->SetLimits(WAVE_SPEED_MIN, WAVE_SPEED_MAX);
    BitmapButton_Wave_Y_PositionVC->GetValue()->SetLimits(WAVE_Y_POSITION_MIN, WAVE_Y_POSITION_MAX);

    ValidateWindow();
}

WavePanel::~WavePanel()
{
	//(*Destroy(WavePanel)
	//*)
}

PANEL_EVENT_HANDLERS(WavePanel)

void WavePanel::ValidateWindow()
{
    wxString type = Choice_Wave_Type->GetStringSelection();
    if (type == "Fractal / ivy")
    {
        BitmapButton_Number_WavesVC->GetValue()->SetActive(false);
        BitmapButton_Number_WavesVC->Disable();
    }
    else
    {
        BitmapButton_Number_WavesVC->Enable();
    }
}

void WavePanel::OnChoice_Wave_TypeSelect(wxCommandEvent& event)
{
    ValidateWindow();
}
