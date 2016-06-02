#include "BarsPanel.h"

#include "../../include/padlock16x16-blue.xpm"
#include "EffectPanelUtils.h"


//(*InternalHeaders(BarsPanel)
#include <wx/bmpbuttn.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/slider.h>
#include <wx/intl.h>
#include <wx/stattext.h>
#include <wx/bitmap.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/image.h>
//*)



//(*IdInit(BarsPanel)
const long BarsPanel::ID_SLIDER_Bars_BarCount = wxNewId();
const long BarsPanel::ID_VALUECURVE_Bars_BarCount = wxNewId();
const long BarsPanel::IDD_TEXTCTRL_Bars_BarCount = wxNewId();
const long BarsPanel::ID_BITMAPBUTTON_SLIDER_Bars_BarCount = wxNewId();
const long BarsPanel::IDD_SLIDER_Bars_Cycles = wxNewId();
const long BarsPanel::ID_VALUECURVE_Bars_Cycles = wxNewId();
const long BarsPanel::ID_TEXTCTRL_Bars_Cycles = wxNewId();
const long BarsPanel::ID_CHOICE_Bars_Direction = wxNewId();
const long BarsPanel::ID_BITMAPBUTTON_CHOICE_Bars_Direction = wxNewId();
const long BarsPanel::ID_CHECKBOX_Bars_Highlight = wxNewId();
const long BarsPanel::ID_BITMAPBUTTON_CHECKBOX_Bars_Highlight = wxNewId();
const long BarsPanel::ID_CHECKBOX_Bars_3D = wxNewId();
const long BarsPanel::ID_BITMAPBUTTON_CHECKBOX_Bars_3D = wxNewId();
const long BarsPanel::ID_CHECKBOX_Bars_Gradient = wxNewId();
const long BarsPanel::ID_BITMAPBUTTON_CHECKBOX_Bars_Gradient = wxNewId();
//*)

BEGIN_EVENT_TABLE(BarsPanel,wxPanel)
	//(*EventTable(BarsPanel)
	//*)
END_EVENT_TABLE()

BarsPanel::BarsPanel(wxWindow* parent)
{
	//(*Initialize(BarsPanel)
	wxBitmapButton* BitmapButton_Gradient;
	wxStaticText* StaticText24;
	wxCheckBox* CheckBox_Bars_Gradient;
	wxCheckBox* CheckBox_Bars_3D;
	wxTextCtrl* TextCtrl35;
	wxCheckBox* CheckBox_Bars_Highlight;
	wxFlexGridSizer* FlexGridSizer123;
	wxBitmapButton* BitmapButton_3D;
	wxSlider* Slider13;
	wxStaticText* StaticText23;
	wxBitmapButton* BitmapButton_Highlight;
	wxBitmapButton* BitmapButton_Direction;
	wxFlexGridSizer* FlexGridSizer10;
	wxFlexGridSizer* FlexGridSizer70;
	wxTextCtrl* TextCtrl15;
	wxBitmapButton* BitmapButton_PaletteRep;
	wxFlexGridSizer* FlexGridSizer35;
	wxStaticText* StaticText177;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer35 = new wxFlexGridSizer(0, 3, wxDLG_UNIT(this,wxSize(0,0)).GetWidth(), 0);
	FlexGridSizer35->AddGrowableCol(1);
	StaticText23 = new wxStaticText(this, wxID_ANY, _("Palette Rep"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer35->Add(StaticText23, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer123 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer123->AddGrowableCol(0);
	Slider_Bars_BarCount = new wxSlider(this, ID_SLIDER_Bars_BarCount, 1, 1, 5, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Bars_BarCount"));
	FlexGridSizer123->Add(Slider_Bars_BarCount, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Bars_BarCount = new ValueCurveButton(this, ID_VALUECURVE_Bars_BarCount, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Bars_BarCount"));
	FlexGridSizer123->Add(BitmapButton_Bars_BarCount, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	TextCtrl15 = new wxTextCtrl(this, IDD_TEXTCTRL_Bars_BarCount, _("1"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Bars_BarCount"));
	TextCtrl15->SetMaxLength(1);
	FlexGridSizer123->Add(TextCtrl15, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer35->Add(FlexGridSizer123, 1, wxALL|wxEXPAND, 0);
	BitmapButton_PaletteRep = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Bars_BarCount, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Bars_BarCount"));
	BitmapButton_PaletteRep->SetDefault();
	BitmapButton_PaletteRep->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer35->Add(BitmapButton_PaletteRep, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText177 = new wxStaticText(this, wxID_ANY, _("Cycles"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer35->Add(StaticText177, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer70 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer70->AddGrowableCol(0);
	Slider13 = new wxSlider(this, IDD_SLIDER_Bars_Cycles, 10, 0, 300, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Bars_Cycles"));
	FlexGridSizer70->Add(Slider13, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Bars_Cycles = new ValueCurveButton(this, ID_VALUECURVE_Bars_Cycles, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Bars_Cycles"));
	FlexGridSizer70->Add(BitmapButton_Bars_Cycles, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	TextCtrl35 = new wxTextCtrl(this, ID_TEXTCTRL_Bars_Cycles, _("1.0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Bars_Cycles"));
	TextCtrl35->SetMaxLength(4);
	FlexGridSizer70->Add(TextCtrl35, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer35->Add(FlexGridSizer70, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer35->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText24 = new wxStaticText(this, wxID_ANY, _("Direction"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer35->Add(StaticText24, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Bars_Direction = new wxChoice(this, ID_CHOICE_Bars_Direction, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Bars_Direction"));
	Choice_Bars_Direction->SetSelection( Choice_Bars_Direction->Append(_("up")) );
	Choice_Bars_Direction->Append(_("down"));
	Choice_Bars_Direction->Append(_("expand"));
	Choice_Bars_Direction->Append(_("compress"));
	Choice_Bars_Direction->Append(_("Left"));
	Choice_Bars_Direction->Append(_("Right"));
	Choice_Bars_Direction->Append(_("H-expand"));
	Choice_Bars_Direction->Append(_("H-compress"));
	Choice_Bars_Direction->Append(_("Alternate Up"));
	Choice_Bars_Direction->Append(_("Alternate Down"));
	Choice_Bars_Direction->Append(_("Alternate Left"));
	Choice_Bars_Direction->Append(_("Alternate Right"));
	FlexGridSizer35->Add(Choice_Bars_Direction, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Direction = new wxBitmapButton(this, ID_BITMAPBUTTON_CHOICE_Bars_Direction, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Bars_Direction"));
	BitmapButton_Direction->SetDefault();
	BitmapButton_Direction->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer35->Add(BitmapButton_Direction, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer35->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer10 = new wxFlexGridSizer(0, 8, 0, 0);
	CheckBox_Bars_Highlight = new wxCheckBox(this, ID_CHECKBOX_Bars_Highlight, _("Highlight"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Bars_Highlight"));
	CheckBox_Bars_Highlight->SetValue(false);
	FlexGridSizer10->Add(CheckBox_Bars_Highlight, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Highlight = new wxBitmapButton(this, ID_BITMAPBUTTON_CHECKBOX_Bars_Highlight, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Bars_Highlight"));
	BitmapButton_Highlight->SetDefault();
	BitmapButton_Highlight->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer10->Add(BitmapButton_Highlight, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer10->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Bars_3D = new wxCheckBox(this, ID_CHECKBOX_Bars_3D, _("3D"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Bars_3D"));
	CheckBox_Bars_3D->SetValue(false);
	FlexGridSizer10->Add(CheckBox_Bars_3D, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_3D = new wxBitmapButton(this, ID_BITMAPBUTTON_CHECKBOX_Bars_3D, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Bars_3D"));
	BitmapButton_3D->SetDefault();
	BitmapButton_3D->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer10->Add(BitmapButton_3D, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer10->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Bars_Gradient = new wxCheckBox(this, ID_CHECKBOX_Bars_Gradient, _("Gradient"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Bars_Gradient"));
	CheckBox_Bars_Gradient->SetValue(false);
	FlexGridSizer10->Add(CheckBox_Bars_Gradient, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Gradient = new wxBitmapButton(this, ID_BITMAPBUTTON_CHECKBOX_Bars_Gradient, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Bars_Gradient"));
	BitmapButton_Gradient->SetDefault();
	BitmapButton_Gradient->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer10->Add(BitmapButton_Gradient, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer35->Add(FlexGridSizer10, 1, wxALL|wxEXPAND, 0);
	SetSizer(FlexGridSizer35);
	FlexGridSizer35->Fit(this);
	FlexGridSizer35->SetSizeHints(this);

	Connect(ID_SLIDER_Bars_BarCount,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&BarsPanel::UpdateLinkedTextCtrlVC);
	Connect(ID_VALUECURVE_Bars_BarCount,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BarsPanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_Bars_BarCount,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&BarsPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_SLIDER_Bars_BarCount,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BarsPanel::OnLockButtonClick);
	Connect(IDD_SLIDER_Bars_Cycles,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&BarsPanel::UpdateLinkedTextCtrlFloatVC);
	Connect(ID_VALUECURVE_Bars_Cycles,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BarsPanel::OnVCButtonClick);
	Connect(ID_TEXTCTRL_Bars_Cycles,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&BarsPanel::UpdateLinkedSliderFloat);
	Connect(ID_BITMAPBUTTON_CHOICE_Bars_Direction,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BarsPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Bars_Highlight,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BarsPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Bars_3D,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BarsPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Bars_Gradient,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&BarsPanel::OnLockButtonClick);
	//*)

    SetName("ID_PANEL_BARS");

    BitmapButton_Bars_BarCount->GetValue()->SetLimits(1, 5);
    BitmapButton_Bars_Cycles->GetValue()->SetLimits(0, 300);
}

BarsPanel::~BarsPanel()
{
	//(*Destroy(BarsPanel)
	//*)
}

PANEL_EVENT_HANDLERS(BarsPanel)

