#include "CurtainPanel.h"
#include "../../include/padlock16x16-blue.xpm"
#include "EffectPanelUtils.h"

//(*InternalHeaders(CurtainPanel)
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
//*)

//(*IdInit(CurtainPanel)
const long CurtainPanel::ID_STATICTEXT6 = wxNewId();
const long CurtainPanel::ID_CHOICE_Curtain_Edge = wxNewId();
const long CurtainPanel::ID_BITMAPBUTTON_CHOICE_Curtain_Edge = wxNewId();
const long CurtainPanel::ID_STATICTEXT5 = wxNewId();
const long CurtainPanel::ID_CHOICE_Curtain_Effect = wxNewId();
const long CurtainPanel::ID_BITMAPBUTTON_CHOICE_Curtain_Effect = wxNewId();
const long CurtainPanel::ID_STATICTEXT7 = wxNewId();
const long CurtainPanel::ID_SLIDER_Curtain_Swag = wxNewId();
const long CurtainPanel::ID_VALUECURVE_Curtain_Swag = wxNewId();
const long CurtainPanel::IDD_TEXTCTRL_Curtain_Swag = wxNewId();
const long CurtainPanel::ID_BITMAPBUTTON_SLIDER_Curtain_Swag = wxNewId();
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
	wxFlexGridSizer* FlexGridSizer76;
	wxTextCtrl* TextCtrl41;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer7;
	wxFlexGridSizer* FlexGridSizer20;
	wxStaticText* StaticText178;
	wxFlexGridSizer* FlexGridSizer1;
	wxTextCtrl* TextCtrl40;
	wxSlider* Slider_Curtain_Swag;
	wxSlider* Slider14;

	Create(parent, wxID_ANY, wxPoint(150,14), wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer7 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer7->AddGrowableCol(0);
	FlexGridSizer20 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer20->AddGrowableCol(1);
	StaticText7 = new wxStaticText(this, ID_STATICTEXT6, _("Curtain Edge"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer20->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Curtain_Edge = new wxChoice(this, ID_CHOICE_Curtain_Edge, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Curtain_Edge"));
	Choice_Curtain_Edge->SetSelection( Choice_Curtain_Edge->Append(_("left")) );
	Choice_Curtain_Edge->Append(_("center"));
	Choice_Curtain_Edge->Append(_("right"));
	Choice_Curtain_Edge->Append(_("bottom"));
	Choice_Curtain_Edge->Append(_("middle"));
	Choice_Curtain_Edge->Append(_("top"));
	FlexGridSizer20->Add(Choice_Curtain_Edge, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer20->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_CurtainEdge = new wxBitmapButton(this, ID_BITMAPBUTTON_CHOICE_Curtain_Edge, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Curtain_Edge"));
	BitmapButton_CurtainEdge->SetDefault();
	BitmapButton_CurtainEdge->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer20->Add(BitmapButton_CurtainEdge, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT5, _("Effect"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer20->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Curtain_Effect = new wxChoice(this, ID_CHOICE_Curtain_Effect, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Curtain_Effect"));
	Choice_Curtain_Effect->SetSelection( Choice_Curtain_Effect->Append(_("open")) );
	Choice_Curtain_Effect->Append(_("close"));
	Choice_Curtain_Effect->Append(_("open then close"));
	Choice_Curtain_Effect->Append(_("close then open"));
	FlexGridSizer20->Add(Choice_Curtain_Effect, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer20->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_CurtainEffect = new wxBitmapButton(this, ID_BITMAPBUTTON_CHOICE_Curtain_Effect, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Curtain_Effect"));
	BitmapButton_CurtainEffect->SetDefault();
	BitmapButton_CurtainEffect->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer20->Add(BitmapButton_CurtainEffect, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText8 = new wxStaticText(this, ID_STATICTEXT7, _("Swag Width"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	FlexGridSizer20->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	Slider_Curtain_Swag = new wxSlider(this, ID_SLIDER_Curtain_Swag, 3, 0, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Curtain_Swag"));
	FlexGridSizer1->Add(Slider_Curtain_Swag, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Curtain_SwagVC = new ValueCurveButton(this, ID_VALUECURVE_Curtain_Swag, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Curtain_Swag"));
	FlexGridSizer1->Add(BitmapButton_Curtain_SwagVC, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer20->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 0);
	TextCtrl40 = new wxTextCtrl(this, IDD_TEXTCTRL_Curtain_Swag, _("3"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Curtain_Swag"));
	TextCtrl40->SetMaxLength(2);
	FlexGridSizer20->Add(TextCtrl40, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_CurtainSwag = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Curtain_Swag, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Curtain_Swag"));
	BitmapButton_CurtainSwag->SetDefault();
	BitmapButton_CurtainSwag->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer20->Add(BitmapButton_CurtainSwag, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText178 = new wxStaticText(this, wxID_ANY, _("Swag Speed"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer20->Add(StaticText178, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	Slider14 = new wxSlider(this, IDD_SLIDER_Curtain_Speed, 10, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Curtain_Speed"));
	FlexGridSizer2->Add(Slider14, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Curtain_SpeedVC = new ValueCurveButton(this, ID_VALUECURVE_Curtain_Speed, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Curtain_Speed"));
	FlexGridSizer2->Add(BitmapButton_Curtain_SpeedVC, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer20->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 0);
	TextCtrl41 = new wxTextCtrl(this, ID_TEXTCTRL_Curtain_Speed, _("1.0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Curtain_Speed"));
	TextCtrl41->SetMaxLength(4);
	FlexGridSizer20->Add(TextCtrl41, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer7->Add(FlexGridSizer20, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer76 = new wxFlexGridSizer(0, 3, 0, 0);
	CheckBox_Curtain_Repeat = new wxCheckBox(this, ID_CHECKBOX_Curtain_Repeat, _("Repeat"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Curtain_Repeat"));
	CheckBox_Curtain_Repeat->SetValue(false);
	FlexGridSizer76->Add(CheckBox_Curtain_Repeat, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer76->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	BitmapButton_CurtainRepeat = new wxBitmapButton(this, ID_BITMAPBUTTON_CHECKBOX_Curtain_Repeat, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Curtain_Repeat"));
	BitmapButton_CurtainRepeat->SetDefault();
	BitmapButton_CurtainRepeat->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer76->Add(BitmapButton_CurtainRepeat, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer7->Add(FlexGridSizer76, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer7);
	FlexGridSizer7->Fit(this);
	FlexGridSizer7->SetSizeHints(this);

	Connect(ID_BITMAPBUTTON_CHOICE_Curtain_Edge,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CurtainPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHOICE_Curtain_Effect,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CurtainPanel::OnLockButtonClick);
	Connect(ID_SLIDER_Curtain_Swag,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&CurtainPanel::UpdateLinkedTextCtrlVC);
	Connect(ID_VALUECURVE_Curtain_Swag,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CurtainPanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_Curtain_Swag,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&CurtainPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_SLIDER_Curtain_Swag,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CurtainPanel::OnLockButtonClick);
	Connect(IDD_SLIDER_Curtain_Speed,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&CurtainPanel::UpdateLinkedTextCtrlFloatVC);
	Connect(ID_VALUECURVE_Curtain_Speed,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CurtainPanel::OnVCButtonClick);
	Connect(ID_TEXTCTRL_Curtain_Speed,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&CurtainPanel::UpdateLinkedSliderFloat);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Curtain_Repeat,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CurtainPanel::OnLockButtonClick);
	//*)

    BitmapButton_Curtain_SpeedVC->GetValue()->SetLimits(0, 10);
    BitmapButton_Curtain_SwagVC->GetValue()->SetLimits(0, 10);

    SetName("ID_PANEL_CURTAIN");
}

CurtainPanel::~CurtainPanel()
{
	//(*Destroy(CurtainPanel)
	//*)
}

PANEL_EVENT_HANDLERS(CurtainPanel)
