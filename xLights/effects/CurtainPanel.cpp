#include "CurtainPanel.h"
#include "EffectPanelUtils.h"
#include "CurtainEffect.h"

//(*InternalHeaders(CurtainPanel)
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

//(*IdInit(CurtainPanel)
const long CurtainPanel::ID_STATICTEXT_Curtain_Edge = wxNewId();
const long CurtainPanel::ID_CHOICE_Curtain_Edge = wxNewId();
const long CurtainPanel::ID_BITMAPBUTTON_CHOICE_Curtain_Edge = wxNewId();
const long CurtainPanel::ID_STATICTEXT_Curtain_Effect = wxNewId();
const long CurtainPanel::ID_CHOICE_Curtain_Effect = wxNewId();
const long CurtainPanel::ID_BITMAPBUTTON_CHOICE_Curtain_Effect = wxNewId();
const long CurtainPanel::ID_STATICTEXT_Curtain_Swag = wxNewId();
const long CurtainPanel::ID_SLIDER_Curtain_Swag = wxNewId();
const long CurtainPanel::ID_VALUECURVE_Curtain_Swag = wxNewId();
const long CurtainPanel::IDD_TEXTCTRL_Curtain_Swag = wxNewId();
const long CurtainPanel::ID_BITMAPBUTTON_SLIDER_Curtain_Swag = wxNewId();
const long CurtainPanel::ID_STATICTEXT_Curtain_Speed = wxNewId();
const long CurtainPanel::IDD_SLIDER_Curtain_Speed = wxNewId();
const long CurtainPanel::ID_VALUECURVE_Curtain_Speed = wxNewId();
const long CurtainPanel::ID_TEXTCTRL_Curtain_Speed = wxNewId();
const long CurtainPanel::ID_CHECKBOX_Curtain_Repeat = wxNewId();
const long CurtainPanel::ID_BITMAPBUTTON_CHECKBOX_Curtain_Repeat = wxNewId();
//*)

BEGIN_EVENT_TABLE(CurtainPanel,wxPanel)
	//(*EventTable(CurtainPanel)
	//*)
END_EVENT_TABLE()

CurtainPanel::CurtainPanel(wxWindow* parent)
{
	//(*Initialize(CurtainPanel)
	BulkEditTextCtrl* TextCtrl40;
	BulkEditTextCtrlF1* TextCtrl41;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer20;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer76;
	wxFlexGridSizer* FlexGridSizer7;

	Create(parent, wxID_ANY, wxPoint(150,14), wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer7 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer7->AddGrowableCol(0);
	FlexGridSizer20 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer20->AddGrowableCol(1);
	StaticText7 = new wxStaticText(this, ID_STATICTEXT_Curtain_Edge, _("Curtain Edge"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Curtain_Edge"));
	FlexGridSizer20->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Curtain_Edge = new BulkEditChoice(this, ID_CHOICE_Curtain_Edge, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Curtain_Edge"));
	Choice_Curtain_Edge->SetSelection( Choice_Curtain_Edge->Append(_("left")) );
	Choice_Curtain_Edge->Append(_("center"));
	Choice_Curtain_Edge->Append(_("right"));
	Choice_Curtain_Edge->Append(_("bottom"));
	Choice_Curtain_Edge->Append(_("middle"));
	Choice_Curtain_Edge->Append(_("top"));
	FlexGridSizer20->Add(Choice_Curtain_Edge, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer20->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_CurtainEdge = new xlLockButton(this, ID_BITMAPBUTTON_CHOICE_Curtain_Edge, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Curtain_Edge"));
	BitmapButton_CurtainEdge->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer20->Add(BitmapButton_CurtainEdge, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT_Curtain_Effect, _("Effect"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Curtain_Effect"));
	FlexGridSizer20->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Curtain_Effect = new BulkEditChoice(this, ID_CHOICE_Curtain_Effect, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Curtain_Effect"));
	Choice_Curtain_Effect->SetSelection( Choice_Curtain_Effect->Append(_("open")) );
	Choice_Curtain_Effect->Append(_("close"));
	Choice_Curtain_Effect->Append(_("open then close"));
	Choice_Curtain_Effect->Append(_("close then open"));
	FlexGridSizer20->Add(Choice_Curtain_Effect, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer20->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_CurtainEffect = new xlLockButton(this, ID_BITMAPBUTTON_CHOICE_Curtain_Effect, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Curtain_Effect"));
	BitmapButton_CurtainEffect->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer20->Add(BitmapButton_CurtainEffect, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText8 = new wxStaticText(this, ID_STATICTEXT_Curtain_Swag, _("Swag Width"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Curtain_Swag"));
	FlexGridSizer20->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	Slider_Curtain_Swag = new BulkEditSlider(this, ID_SLIDER_Curtain_Swag, 3, 0, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Curtain_Swag"));
	FlexGridSizer1->Add(Slider_Curtain_Swag, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Curtain_SwagVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Curtain_Swag, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Curtain_Swag"));
	FlexGridSizer1->Add(BitmapButton_Curtain_SwagVC, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer20->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 0);
	TextCtrl40 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Curtain_Swag, _("3"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Curtain_Swag"));
	TextCtrl40->SetMaxLength(2);
	FlexGridSizer20->Add(TextCtrl40, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_CurtainSwag = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Curtain_Swag, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Curtain_Swag"));
	BitmapButton_CurtainSwag->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer20->Add(BitmapButton_CurtainSwag, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText178 = new wxStaticText(this, ID_STATICTEXT_Curtain_Speed, _("Swag Speed"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Curtain_Speed"));
	FlexGridSizer20->Add(StaticText178, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	Slider_Curtain_Speed = new BulkEditSliderF1(this, IDD_SLIDER_Curtain_Speed, 10, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Curtain_Speed"));
	FlexGridSizer2->Add(Slider_Curtain_Speed, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Curtain_SpeedVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Curtain_Speed, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Curtain_Speed"));
	FlexGridSizer2->Add(BitmapButton_Curtain_SpeedVC, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer20->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 0);
	TextCtrl41 = new BulkEditTextCtrlF1(this, ID_TEXTCTRL_Curtain_Speed, _("1.0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Curtain_Speed"));
	TextCtrl41->SetMaxLength(4);
	FlexGridSizer20->Add(TextCtrl41, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer7->Add(FlexGridSizer20, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer76 = new wxFlexGridSizer(0, 3, 0, 0);
	CheckBox_Curtain_Repeat = new BulkEditCheckBox(this, ID_CHECKBOX_Curtain_Repeat, _("Repeat"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Curtain_Repeat"));
	CheckBox_Curtain_Repeat->SetValue(false);
	FlexGridSizer76->Add(CheckBox_Curtain_Repeat, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer76->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	BitmapButton_CurtainRepeat = new xlLockButton(this, ID_BITMAPBUTTON_CHECKBOX_Curtain_Repeat, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Curtain_Repeat"));
	BitmapButton_CurtainRepeat->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer76->Add(BitmapButton_CurtainRepeat, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer7->Add(FlexGridSizer76, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer7);
	FlexGridSizer7->Fit(this);
	FlexGridSizer7->SetSizeHints(this);

	Connect(ID_BITMAPBUTTON_CHOICE_Curtain_Edge,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CurtainPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHOICE_Curtain_Effect,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CurtainPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Curtain_Swag,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CurtainPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Curtain_Swag,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CurtainPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Curtain_Speed,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CurtainPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Curtain_Repeat,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CurtainPanel::OnLockButtonClick);
	//*)

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&CurtainPanel::OnVCChanged, 0, this);

    BitmapButton_Curtain_SpeedVC->GetValue()->SetLimits(CURTAIN_SPEED_MIN, CURTAIN_SPEED_MAX);
    BitmapButton_Curtain_SwagVC->GetValue()->SetLimits(CURTAIN_SWAG_MIN, CURTAIN_SWAG_MAX);

    SetName("ID_PANEL_CURTAIN");
}

CurtainPanel::~CurtainPanel()
{
	//(*Destroy(CurtainPanel)
	//*)
}

PANEL_EVENT_HANDLERS(CurtainPanel)
