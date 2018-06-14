#include "CirclesPanel.h"
#include "EffectPanelUtils.h"
#include "CirclesEffect.h"

//(*InternalHeaders(CirclesPanel)
#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#include <wx/checkbox.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>
//*)

//(*IdInit(CirclesPanel)
const long CirclesPanel::ID_STATICTEXT_Circles_Count = wxNewId();
const long CirclesPanel::ID_SLIDER_Circles_Count = wxNewId();
const long CirclesPanel::ID_VALUECURVE_Circles_Count = wxNewId();
const long CirclesPanel::IDD_TEXTCTRL_Circles_Count = wxNewId();
const long CirclesPanel::ID_BITMAPBUTTON_SLIDER_Circles_Count = wxNewId();
const long CirclesPanel::ID_STATICTEXT_Circles_Size = wxNewId();
const long CirclesPanel::ID_SLIDER_Circles_Size = wxNewId();
const long CirclesPanel::ID_VALUECURVE_Circles_Size = wxNewId();
const long CirclesPanel::IDD_TEXTCTRL_Circles_Size = wxNewId();
const long CirclesPanel::ID_BITMAPBUTTON_SLIDER_Circles_Size = wxNewId();
const long CirclesPanel::ID_STATICTEXT_Circles_Speed = wxNewId();
const long CirclesPanel::ID_SLIDER_Circles_Speed = wxNewId();
const long CirclesPanel::ID_VALUECURVE_Circles_Speed = wxNewId();
const long CirclesPanel::IDD_TEXTCTRL_Circles_Speed = wxNewId();
const long CirclesPanel::ID_BITMAPBUTTON_SLIDER_Circles_Speed = wxNewId();
const long CirclesPanel::ID_CHECKBOX_Circles_Bounce = wxNewId();
const long CirclesPanel::ID_BITMAPBUTTON_CHECKBOX_Circles_Bounce = wxNewId();
const long CirclesPanel::ID_CHECKBOX_Circles_Radial = wxNewId();
const long CirclesPanel::ID_BITMAPBUTTON_CHECKBOX_Circles_Radial = wxNewId();
const long CirclesPanel::ID_CHECKBOX_Circles_Plasma = wxNewId();
const long CirclesPanel::ID_BITMAPBUTTON_CHECKBOX_Circles_Plasma = wxNewId();
const long CirclesPanel::ID_CHECKBOX_Circles_Radial_3D = wxNewId();
const long CirclesPanel::ID_BITMAPBUTTON_CHECKBOX_Circles_Radial_3D = wxNewId();
const long CirclesPanel::ID_CHECKBOX_Circles_Bubbles = wxNewId();
const long CirclesPanel::ID_BITMAPBUTTON_CHECKBOX_Circles_Bubbles = wxNewId();
const long CirclesPanel::ID_CHECKBOX_Circles_Collide = wxNewId();
const long CirclesPanel::ID_BITMAPBUTTON_CHECKBOX_Circles_Collide = wxNewId();
const long CirclesPanel::ID_CHECKBOX_Circles_Random_m = wxNewId();
const long CirclesPanel::ID_BITMAPBUTTON_CHECKBOX_Circles_Random_m = wxNewId();
const long CirclesPanel::ID_CHECKBOX_Circles_Linear_Fade = wxNewId();
const long CirclesPanel::ID_BITMAPBUTTON_CHECKBOX_Circles_Linear_Fade = wxNewId();
//*)

BEGIN_EVENT_TABLE(CirclesPanel,wxPanel)
	//(*EventTable(CirclesPanel)
	//*)
END_EVENT_TABLE()

CirclesPanel::CirclesPanel(wxWindow* parent)
{
	//(*Initialize(CirclesPanel)
	BulkEditTextCtrl* TextCtrl27;
	BulkEditTextCtrl* TextCtrl28;
	BulkEditTextCtrl* TextCtrl29;
	wxFlexGridSizer* FlexGridSizer17;
	wxFlexGridSizer* FlexGridSizer18;
	wxFlexGridSizer* FlexGridSizer22;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer80;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer80 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer80->AddGrowableCol(0);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	FlexGridSizer17 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer17->AddGrowableCol(1);
	StaticText136 = new wxStaticText(this, ID_STATICTEXT_Circles_Count, _("Number of Circles"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Circles_Count"));
	FlexGridSizer17->Add(StaticText136, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Circles_Count = new BulkEditSlider(this, ID_SLIDER_Circles_Count, 3, 1, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Circles_Count"));
	FlexGridSizer17->Add(Slider_Circles_Count, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Circles_Count = new BulkEditValueCurveButton(this, ID_VALUECURVE_Circles_Count, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Circles_Count"));
	FlexGridSizer17->Add(BitmapButton_Circles_Count, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	TextCtrl27 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Circles_Count, _("3"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Circles_Count"));
	TextCtrl27->SetMaxLength(2);
	FlexGridSizer17->Add(TextCtrl27, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer3->Add(FlexGridSizer17, 1, wxALL|wxEXPAND, 0);
	BitmapButton_CirclesCount = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Circles_Count, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Circles_Count"));
	BitmapButton_CirclesCount->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer3->Add(BitmapButton_CirclesCount, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer18 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer18->AddGrowableCol(1);
	StaticText137 = new wxStaticText(this, ID_STATICTEXT_Circles_Size, _("Size"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Circles_Size"));
	FlexGridSizer18->Add(StaticText137, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Circles_Size = new BulkEditSlider(this, ID_SLIDER_Circles_Size, 5, 1, 20, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Circles_Size"));
	FlexGridSizer18->Add(Slider_Circles_Size, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Circles_Size = new BulkEditValueCurveButton(this, ID_VALUECURVE_Circles_Size, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Circles_Size"));
	FlexGridSizer18->Add(BitmapButton_Circles_Size, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	TextCtrl28 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Circles_Size, _("5"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Circles_Size"));
	TextCtrl28->SetMaxLength(2);
	FlexGridSizer18->Add(TextCtrl28, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer3->Add(FlexGridSizer18, 1, wxALL|wxEXPAND, 0);
	BitmapButton_CirclesSize = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Circles_Size, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Circles_Size"));
	BitmapButton_CirclesSize->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer3->Add(BitmapButton_CirclesSize, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer22 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer22->AddGrowableCol(1);
	StaticText31 = new wxStaticText(this, ID_STATICTEXT_Circles_Speed, _("Speed"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Circles_Speed"));
	FlexGridSizer22->Add(StaticText31, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Circles_Speed = new BulkEditSlider(this, ID_SLIDER_Circles_Speed, 10, 1, 30, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Circles_Speed"));
	FlexGridSizer22->Add(Slider_Circles_Speed, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Circles_Speed = new BulkEditValueCurveButton(this, ID_VALUECURVE_Circles_Speed, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Circles_Speed"));
	FlexGridSizer22->Add(BitmapButton_Circles_Speed, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	TextCtrl29 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Circles_Speed, _("10"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Circles_Speed"));
	TextCtrl29->SetMaxLength(2);
	FlexGridSizer22->Add(TextCtrl29, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer3->Add(FlexGridSizer22, 1, wxALL|wxEXPAND, 0);
	BitmapButton2 = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Circles_Speed, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Circles_Speed"));
	BitmapButton2->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer3->Add(BitmapButton2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	CheckBox_Circles_Bounce = new BulkEditCheckBox(this, ID_CHECKBOX_Circles_Bounce, _("Bounce"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Circles_Bounce"));
	CheckBox_Circles_Bounce->SetValue(false);
	FlexGridSizer3->Add(CheckBox_Circles_Bounce, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_CirclesBounce = new xlLockButton(this, ID_BITMAPBUTTON_CHECKBOX_Circles_Bounce, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Circles_Bounce"));
	BitmapButton_CirclesBounce->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer3->Add(BitmapButton_CirclesBounce, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	CheckBox_Circles_Radial = new BulkEditCheckBox(this, ID_CHECKBOX_Circles_Radial, _("Radial"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Circles_Radial"));
	CheckBox_Circles_Radial->SetValue(false);
	FlexGridSizer3->Add(CheckBox_Circles_Radial, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_CirclesRadial = new xlLockButton(this, ID_BITMAPBUTTON_CHECKBOX_Circles_Radial, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Circles_Radial"));
	BitmapButton_CirclesRadial->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer3->Add(BitmapButton_CirclesRadial, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	CheckBox_Circles_Plasma = new BulkEditCheckBox(this, ID_CHECKBOX_Circles_Plasma, _("Plasma"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Circles_Plasma"));
	CheckBox_Circles_Plasma->SetValue(false);
	FlexGridSizer3->Add(CheckBox_Circles_Plasma, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_CirclesPlasma = new xlLockButton(this, ID_BITMAPBUTTON_CHECKBOX_Circles_Plasma, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Circles_Plasma"));
	BitmapButton_CirclesPlasma->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer3->Add(BitmapButton_CirclesPlasma, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	CheckBox_Circles_Radial_3D = new BulkEditCheckBox(this, ID_CHECKBOX_Circles_Radial_3D, _("Radial 3D"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Circles_Radial_3D"));
	CheckBox_Circles_Radial_3D->SetValue(false);
	FlexGridSizer3->Add(CheckBox_Circles_Radial_3D, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_CirclesRadial3D = new xlLockButton(this, ID_BITMAPBUTTON_CHECKBOX_Circles_Radial_3D, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Circles_Radial_3D"));
	BitmapButton_CirclesRadial3D->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer3->Add(BitmapButton_CirclesRadial3D, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	CheckBox_Circles_Bubbles = new BulkEditCheckBox(this, ID_CHECKBOX_Circles_Bubbles, _("Bubbles"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Circles_Bubbles"));
	CheckBox_Circles_Bubbles->SetValue(false);
	FlexGridSizer3->Add(CheckBox_Circles_Bubbles, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_CirclesBubbles = new xlLockButton(this, ID_BITMAPBUTTON_CHECKBOX_Circles_Bubbles, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Circles_Bubbles"));
	BitmapButton_CirclesBubbles->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer3->Add(BitmapButton_CirclesBubbles, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	CheckBox_Circles_Collide = new BulkEditCheckBox(this, ID_CHECKBOX_Circles_Collide, _("Collide"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Circles_Collide"));
	CheckBox_Circles_Collide->SetValue(false);
	CheckBox_Circles_Collide->Hide();
	FlexGridSizer3->Add(CheckBox_Circles_Collide, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_CirclesCollide = new xlLockButton(this, ID_BITMAPBUTTON_CHECKBOX_Circles_Collide, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Circles_Collide"));
	BitmapButton_CirclesCollide->Hide();
	BitmapButton_CirclesCollide->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer3->Add(BitmapButton_CirclesCollide, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	CheckBox_Circles_Random_m = new BulkEditCheckBox(this, ID_CHECKBOX_Circles_Random_m, _("Random Motion"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Circles_Random_m"));
	CheckBox_Circles_Random_m->SetValue(false);
	CheckBox_Circles_Random_m->Hide();
	FlexGridSizer3->Add(CheckBox_Circles_Random_m, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_RandomMotion = new xlLockButton(this, ID_BITMAPBUTTON_CHECKBOX_Circles_Random_m, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Circles_Random_m"));
	BitmapButton_RandomMotion->Hide();
	BitmapButton_RandomMotion->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer3->Add(BitmapButton_RandomMotion, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	CheckBox_Circles_Linear_Fade = new BulkEditCheckBox(this, ID_CHECKBOX_Circles_Linear_Fade, _("Linear Fade"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Circles_Linear_Fade"));
	CheckBox_Circles_Linear_Fade->SetValue(false);
	FlexGridSizer3->Add(CheckBox_Circles_Linear_Fade, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Circles_Linear_Fade = new xlLockButton(this, ID_BITMAPBUTTON_CHECKBOX_Circles_Linear_Fade, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Circles_Linear_Fade"));
	BitmapButton_Circles_Linear_Fade->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer3->Add(BitmapButton_Circles_Linear_Fade, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer80->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 2);
	SetSizer(FlexGridSizer80);
	FlexGridSizer80->Fit(this);
	FlexGridSizer80->SetSizeHints(this);

	Connect(ID_VALUECURVE_Circles_Count,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CirclesPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Circles_Count,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CirclesPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Circles_Size,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CirclesPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Circles_Size,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CirclesPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Circles_Speed,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CirclesPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Circles_Speed,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CirclesPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Circles_Bounce,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CirclesPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Circles_Radial,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CirclesPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Circles_Plasma,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CirclesPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Circles_Radial_3D,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CirclesPanel::OnLockButtonClick);
	Connect(ID_CHECKBOX_Circles_Bubbles,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&CirclesPanel::OnCheckBox_Circles_BubblesClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Circles_Bubbles,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CirclesPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Circles_Collide,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CirclesPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Circles_Random_m,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CirclesPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Circles_Linear_Fade,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CirclesPanel::OnLockButtonClick);
	//*)

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&CirclesPanel::OnVCChanged, 0, this);

    BitmapButton_Circles_Count->GetValue()->SetLimits(CIRCLES_COUNT_MIN, CIRCLES_COUNT_MAX);
    BitmapButton_Circles_Size->GetValue()->SetLimits(CIRCLES_SIZE_MIN, CIRCLES_SIZE_MAX);
    BitmapButton_Circles_Speed->GetValue()->SetLimits(CIRCLES_SPEED_MIN, CIRCLES_SPEED_MAX);

    SetName("ID_PANEL_CIRCLES");
}

CirclesPanel::~CirclesPanel()
{
	//(*Destroy(CirclesPanel)
	//*)
}

PANEL_EVENT_HANDLERS(CirclesPanel)

void CirclesPanel::OnCheckBox_Circles_BubblesClick(wxCommandEvent& event)
{
}
