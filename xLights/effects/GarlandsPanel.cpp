#include "GarlandsPanel.h"
#include "../../include/padlock16x16-blue.xpm"
#include "EffectPanelUtils.h"

//(*InternalHeaders(GarlandsPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/bitmap.h>
#include <wx/slider.h>
#include <wx/settings.h>
#include <wx/choice.h>
#include <wx/bmpbuttn.h>
#include <wx/intl.h>
#include <wx/image.h>
#include <wx/string.h>
//*)

//(*IdInit(GarlandsPanel)
const long GarlandsPanel::ID_STATICTEXT34 = wxNewId();
const long GarlandsPanel::ID_SLIDER_Garlands_Type = wxNewId();
const long GarlandsPanel::IDD_TEXTCTRL_Garlands_Type = wxNewId();
const long GarlandsPanel::ID_BITMAPBUTTON_SLIDER_Garlands_Type = wxNewId();
const long GarlandsPanel::ID_STATICTEXT35 = wxNewId();
const long GarlandsPanel::ID_SLIDER_Garlands_Spacing = wxNewId();
const long GarlandsPanel::ID_VALUECURVE_Garlands_Spacing = wxNewId();
const long GarlandsPanel::IDD_TEXTCTRL_Garlands_Spacing = wxNewId();
const long GarlandsPanel::ID_BITMAPBUTTON_SLIDER_Garlands_Spacing = wxNewId();
const long GarlandsPanel::IDD_SLIDER_Garlands_Cycles = wxNewId();
const long GarlandsPanel::ID_VALUECURVE_Garlands_Cycles = wxNewId();
const long GarlandsPanel::ID_TEXTCTRL_Garlands_Cycles = wxNewId();
const long GarlandsPanel::ID_STATICTEXT40 = wxNewId();
const long GarlandsPanel::ID_CHOICE_Garlands_Direction = wxNewId();
//*)

BEGIN_EVENT_TABLE(GarlandsPanel,wxPanel)
	//(*EventTable(GarlandsPanel)
	//*)
END_EVENT_TABLE()

GarlandsPanel::GarlandsPanel(wxWindow* parent)
{
	//(*Initialize(GarlandsPanel)
	wxSlider* Slider23;
	wxStaticText* StaticText183;
	wxFlexGridSizer* FlexGridSizer2;
	wxTextCtrl* TextCtrl61;
	wxTextCtrl* TextCtrl63;
	wxFlexGridSizer* FlexGridSizer1;
	wxChoice* Choice1;
	wxFlexGridSizer* FlexGridSizer39;
	wxTextCtrl* TextCtrl62;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer39 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer39->AddGrowableCol(1);
	StaticText32 = new wxStaticText(this, ID_STATICTEXT34, _("Garland Type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT34"));
	FlexGridSizer39->Add(StaticText32, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Garlands_Type = new wxSlider(this, ID_SLIDER_Garlands_Type, 0, 0, 4, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Garlands_Type"));
	FlexGridSizer39->Add(Slider_Garlands_Type, 1, wxALL|wxEXPAND, 2);
	TextCtrl61 = new wxTextCtrl(this, IDD_TEXTCTRL_Garlands_Type, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Garlands_Type"));
	TextCtrl61->SetMaxLength(3);
	FlexGridSizer39->Add(TextCtrl61, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_GarlandsType = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Garlands_Type, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Garlands_Type"));
	BitmapButton_GarlandsType->SetDefault();
	BitmapButton_GarlandsType->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer39->Add(BitmapButton_GarlandsType, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText33 = new wxStaticText(this, ID_STATICTEXT35, _("Spacing"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT35"));
	FlexGridSizer39->Add(StaticText33, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	Slider_Garlands_Spacing = new wxSlider(this, ID_SLIDER_Garlands_Spacing, 10, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Garlands_Spacing"));
	FlexGridSizer1->Add(Slider_Garlands_Spacing, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Garlands_SpacingVC = new ValueCurveButton(this, ID_VALUECURVE_Garlands_Spacing, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Garlands_Spacing"));
	FlexGridSizer1->Add(BitmapButton_Garlands_SpacingVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer39->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 0);
	TextCtrl62 = new wxTextCtrl(this, IDD_TEXTCTRL_Garlands_Spacing, _("10"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Garlands_Spacing"));
	TextCtrl62->SetMaxLength(3);
	FlexGridSizer39->Add(TextCtrl62, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_GarlandsSpacing = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Garlands_Spacing, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Garlands_Spacing"));
	BitmapButton_GarlandsSpacing->SetDefault();
	BitmapButton_GarlandsSpacing->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer39->Add(BitmapButton_GarlandsSpacing, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText183 = new wxStaticText(this, wxID_ANY, _("Cycles"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer39->Add(StaticText183, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	Slider23 = new wxSlider(this, IDD_SLIDER_Garlands_Cycles, 10, 0, 200, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Garlands_Cycles"));
	FlexGridSizer2->Add(Slider23, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Garlands_CyclesVC = new ValueCurveButton(this, ID_VALUECURVE_Garlands_Cycles, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Garlands_Cycles"));
	FlexGridSizer2->Add(BitmapButton_Garlands_CyclesVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer39->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 0);
	TextCtrl63 = new wxTextCtrl(this, ID_TEXTCTRL_Garlands_Cycles, _("1.0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("ID_TEXTCTRL_Garlands_Cycles"));
	TextCtrl63->SetMaxLength(4);
	FlexGridSizer39->Add(TextCtrl63, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer39->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText192 = new wxStaticText(this, ID_STATICTEXT40, _("Stack Direction"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT40"));
	FlexGridSizer39->Add(StaticText192, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice1 = new wxChoice(this, ID_CHOICE_Garlands_Direction, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Garlands_Direction"));
	Choice1->SetSelection( Choice1->Append(_("Up")) );
	Choice1->Append(_("Down"));
	Choice1->Append(_("Left"));
	Choice1->Append(_("Right"));
	Choice1->Append(_("Up then Down"));
	Choice1->Append(_("Down then Up"));
	Choice1->Append(_("Left then Right"));
	Choice1->Append(_("Right then Left"));
	FlexGridSizer39->Add(Choice1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer39);
	FlexGridSizer39->Fit(this);
	FlexGridSizer39->SetSizeHints(this);

	Connect(ID_SLIDER_Garlands_Type,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&GarlandsPanel::UpdateLinkedTextCtrl);
	Connect(IDD_TEXTCTRL_Garlands_Type,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&GarlandsPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_SLIDER_Garlands_Type,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GarlandsPanel::OnLockButtonClick);
	Connect(ID_SLIDER_Garlands_Spacing,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&GarlandsPanel::UpdateLinkedTextCtrlVC);
	Connect(ID_VALUECURVE_Garlands_Spacing,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GarlandsPanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_Garlands_Spacing,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&GarlandsPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_SLIDER_Garlands_Spacing,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GarlandsPanel::OnLockButtonClick);
	Connect(IDD_SLIDER_Garlands_Cycles,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&GarlandsPanel::UpdateLinkedTextCtrlFloatVC);
	Connect(ID_VALUECURVE_Garlands_Cycles,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GarlandsPanel::OnVCButtonClick);
	Connect(ID_TEXTCTRL_Garlands_Cycles,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&GarlandsPanel::UpdateLinkedSliderFloat);
	//*)

    BitmapButton_Garlands_CyclesVC->GetValue()->SetLimits(0, 20);
    BitmapButton_Garlands_SpacingVC->GetValue()->SetLimits(1, 100);

    SetName("ID_PANEL_GARLANDS");
}

GarlandsPanel::~GarlandsPanel()
{
	//(*Destroy(GarlandsPanel)
	//*)
}

PANEL_EVENT_HANDLERS(GarlandsPanel)
