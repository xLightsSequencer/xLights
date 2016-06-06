#include "PlasmaPanel.h"
#include "../../include/padlock16x16-blue.xpm"
#include "EffectPanelUtils.h"

//(*InternalHeaders(PlasmaPanel)
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

//(*IdInit(PlasmaPanel)
const long PlasmaPanel::ID_CHOICE_Plasma_Color = wxNewId();
const long PlasmaPanel::ID_BITMAPBUTTON35 = wxNewId();
const long PlasmaPanel::ID_SLIDER_Plasma_Style = wxNewId();
const long PlasmaPanel::IDD_TEXTCTRL_Plasma_Style = wxNewId();
const long PlasmaPanel::ID_BITMAPBUTTON36 = wxNewId();
const long PlasmaPanel::ID_SLIDER_Plasma_Line_Density = wxNewId();
const long PlasmaPanel::IDD_TEXTCTRL_Plasma_Line_Density = wxNewId();
const long PlasmaPanel::ID_BITMAPBUTTON37 = wxNewId();
const long PlasmaPanel::ID_SLIDER_Plasma_Speed = wxNewId();
const long PlasmaPanel::ID_VALUECURVE_Plasma_Speed = wxNewId();
const long PlasmaPanel::IDD_TEXTCTRL_Plasma_Speed = wxNewId();
const long PlasmaPanel::ID_BITMAPBUTTON39 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PlasmaPanel,wxPanel)
	//(*EventTable(PlasmaPanel)
	//*)
END_EVENT_TABLE()

PlasmaPanel::PlasmaPanel(wxWindow* parent)
{
	//(*Initialize(PlasmaPanel)
	wxStaticText* StaticText204;
	wxStaticText* StaticText202;
	wxBitmapButton* BitmapButton17;
	wxBitmapButton* BitmapButton19;
	wxFlexGridSizer* FlexGridSizer3;
	wxStaticText* StaticText64;
	wxTextCtrl* TextCtrl84;
	wxSlider* Slider34;
	wxBitmapButton* BitmapButton21;
	wxStaticText* StaticText197;
	wxBitmapButton* BitmapButton18;
	wxFlexGridSizer* FlexGridSizer139;
	wxTextCtrl* TextCtrl87;
	wxSlider* Slider33;
	wxTextCtrl* TextCtrl85;
	wxSlider* Slider36;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer139 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer139->AddGrowableCol(1);
	StaticText64 = new wxStaticText(this, wxID_ANY, _("Color Choice"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer139->Add(StaticText64, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Plasma_Color = new wxChoice(this, ID_CHOICE_Plasma_Color, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Plasma_Color"));
	Choice_Plasma_Color->SetSelection( Choice_Plasma_Color->Append(_("Normal")) );
	Choice_Plasma_Color->Append(_("Preset Colors 1"));
	Choice_Plasma_Color->Append(_("Preset Colors 2"));
	Choice_Plasma_Color->Append(_("Preset Colors 3"));
	Choice_Plasma_Color->Append(_("Preset Colors 4"));
	FlexGridSizer139->Add(Choice_Plasma_Color, 1, wxTOP|wxBOTTOM|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer139->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton17 = new wxBitmapButton(this, ID_BITMAPBUTTON35, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON35"));
	BitmapButton17->SetDefault();
	BitmapButton17->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer139->Add(BitmapButton17, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText197 = new wxStaticText(this, wxID_ANY, _("Add Twist to Effect"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer139->Add(StaticText197, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider33 = new wxSlider(this, ID_SLIDER_Plasma_Style, 1, 1, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Plasma_Style"));
	FlexGridSizer139->Add(Slider33, 1, wxALL|wxEXPAND, 2);
	TextCtrl84 = new wxTextCtrl(this, IDD_TEXTCTRL_Plasma_Style, _("1"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Plasma_Style"));
	TextCtrl84->SetMaxLength(1);
	FlexGridSizer139->Add(TextCtrl84, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton18 = new wxBitmapButton(this, ID_BITMAPBUTTON36, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON36"));
	BitmapButton18->SetDefault();
	BitmapButton18->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer139->Add(BitmapButton18, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText202 = new wxStaticText(this, wxID_ANY, _("Line Density"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer139->Add(StaticText202, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Slider34 = new wxSlider(this, ID_SLIDER_Plasma_Line_Density, 1, 1, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Plasma_Line_Density"));
	FlexGridSizer139->Add(Slider34, 1, wxALL|wxEXPAND, 2);
	TextCtrl85 = new wxTextCtrl(this, IDD_TEXTCTRL_Plasma_Line_Density, _("1"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Plasma_Line_Density"));
	TextCtrl85->SetMaxLength(2);
	FlexGridSizer139->Add(TextCtrl85, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton19 = new wxBitmapButton(this, ID_BITMAPBUTTON37, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON37"));
	BitmapButton19->SetDefault();
	BitmapButton19->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer139->Add(BitmapButton19, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText204 = new wxStaticText(this, wxID_ANY, _("Speed"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer139->Add(StaticText204, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	Slider36 = new wxSlider(this, ID_SLIDER_Plasma_Speed, 10, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Plasma_Speed"));
	FlexGridSizer3->Add(Slider36, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Plasma_SpeedVC = new ValueCurveButton(this, ID_VALUECURVE_Plasma_Speed, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Plasma_Speed"));
	FlexGridSizer3->Add(BitmapButton_Plasma_SpeedVC, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer139->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 2);
	TextCtrl87 = new wxTextCtrl(this, IDD_TEXTCTRL_Plasma_Speed, _("10"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Plasma_Speed"));
	TextCtrl87->SetMaxLength(3);
	FlexGridSizer139->Add(TextCtrl87, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton21 = new wxBitmapButton(this, ID_BITMAPBUTTON39, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON39"));
	BitmapButton21->SetDefault();
	BitmapButton21->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer139->Add(BitmapButton21, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	SetSizer(FlexGridSizer139);
	FlexGridSizer139->Fit(this);
	FlexGridSizer139->SetSizeHints(this);

	Connect(ID_BITMAPBUTTON35,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PlasmaPanel::OnLockButtonClick);
	Connect(ID_SLIDER_Plasma_Style,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&PlasmaPanel::UpdateLinkedTextCtrl);
	Connect(IDD_TEXTCTRL_Plasma_Style,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlasmaPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON36,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PlasmaPanel::OnLockButtonClick);
	Connect(ID_SLIDER_Plasma_Line_Density,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&PlasmaPanel::UpdateLinkedTextCtrl);
	Connect(IDD_TEXTCTRL_Plasma_Line_Density,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlasmaPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON37,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PlasmaPanel::OnLockButtonClick);
	Connect(ID_SLIDER_Plasma_Speed,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&PlasmaPanel::UpdateLinkedTextCtrlVC);
	Connect(ID_VALUECURVE_Plasma_Speed,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PlasmaPanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_Plasma_Speed,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlasmaPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON39,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PlasmaPanel::OnLockButtonClick);
	//*)

    BitmapButton_Plasma_SpeedVC->GetValue()->SetLimits(0, 100);

    SetName("ID_PANEL_PLASMA");
}

PlasmaPanel::~PlasmaPanel()
{
	//(*Destroy(PlasmaPanel)
	//*)
}

PANEL_EVENT_HANDLERS(PlasmaPanel)
