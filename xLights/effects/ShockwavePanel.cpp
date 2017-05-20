#include "ShockwavePanel.h"
#include "../../include/padlock16x16-blue.xpm"
#include "EffectPanelUtils.h"
#include "ShockwaveEffect.h"

//(*InternalHeaders(ShockwavePanel)
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/bitmap.h>
#include <wx/slider.h>
#include <wx/settings.h>
#include <wx/bmpbuttn.h>
#include <wx/intl.h>
#include <wx/image.h>
#include <wx/string.h>
#include "ShockwaveEffect.h"
//*)

//(*IdInit(ShockwavePanel)
const long ShockwavePanel::ID_SLIDER_Shockwave_CenterX = wxNewId();
const long ShockwavePanel::ID_VALUECURVE_Shockwave_CenterX = wxNewId();
const long ShockwavePanel::IDD_TEXTCTRL_Shockwave_CenterX = wxNewId();
const long ShockwavePanel::ID_BITMAPBUTTON_SLIDER_Shockwave_CenterX = wxNewId();
const long ShockwavePanel::ID_STATICTEXT151 = wxNewId();
const long ShockwavePanel::ID_SLIDER_Shockwave_CenterY = wxNewId();
const long ShockwavePanel::ID_VALUECURVE_Shockwave_CenterY = wxNewId();
const long ShockwavePanel::IDD_TEXTCTRL_Shockwave_CenterY = wxNewId();
const long ShockwavePanel::ID_BITMAPBUTTON_SLIDER_Shockwave_CenterY = wxNewId();
const long ShockwavePanel::ID_STATICTEXT152 = wxNewId();
const long ShockwavePanel::ID_SLIDER_Shockwave_Start_Radius = wxNewId();
const long ShockwavePanel::ID_VALUECURVE_Shockwave_Start_Radius = wxNewId();
const long ShockwavePanel::IDD_TEXTCTRL_Shockwave_Start_Radius = wxNewId();
const long ShockwavePanel::ID_BITMAPBUTTON_SLIDER_Shockwave_Start_Radius = wxNewId();
const long ShockwavePanel::ID_STATICTEXT154 = wxNewId();
const long ShockwavePanel::ID_SLIDER_Shockwave_End_Radius = wxNewId();
const long ShockwavePanel::ID_VALUECURVE_Shockwave_End_Radius = wxNewId();
const long ShockwavePanel::IDD_TEXTCTRL_Shockwave_End_Radius = wxNewId();
const long ShockwavePanel::ID_BITMAPBUTTON_SLIDER_Shockwave_End_Radius = wxNewId();
const long ShockwavePanel::ID_STATICTEXT153 = wxNewId();
const long ShockwavePanel::ID_SLIDER_Shockwave_Start_Width = wxNewId();
const long ShockwavePanel::ID_VALUECURVE_Shockwave_Start_Width = wxNewId();
const long ShockwavePanel::IDD_TEXTCTRL_Shockwave_Start_Width = wxNewId();
const long ShockwavePanel::ID_BITMAPBUTTON_SLIDER_Shockwave_Start_Width = wxNewId();
const long ShockwavePanel::ID_STATICTEXT155 = wxNewId();
const long ShockwavePanel::ID_SLIDER_Shockwave_End_Width = wxNewId();
const long ShockwavePanel::ID_VALUECURVE_Shockwave_End_Width = wxNewId();
const long ShockwavePanel::IDD_TEXTCTRL_Shockwave_End_Width = wxNewId();
const long ShockwavePanel::ID_BITMAPBUTTON_SLIDER_Shockwave_End_Width = wxNewId();
const long ShockwavePanel::ID_PANEL40 = wxNewId();
const long ShockwavePanel::ID_STATICTEXT162 = wxNewId();
const long ShockwavePanel::ID_SLIDER_Shockwave_Accel = wxNewId();
const long ShockwavePanel::IDD_TEXTCTRL_Shockwave_Accel = wxNewId();
const long ShockwavePanel::ID_BITMAPBUTTON_SLIDER_Shockwave_Accel = wxNewId();
const long ShockwavePanel::ID_CHECKBOX_Shockwave_Blend_Edges = wxNewId();
const long ShockwavePanel::ID_PANEL44 = wxNewId();
const long ShockwavePanel::ID_NOTEBOOK_Shockwave = wxNewId();
//*)

BEGIN_EVENT_TABLE(ShockwavePanel,wxPanel)
	//(*EventTable(ShockwavePanel)
	//*)
END_EVENT_TABLE()

ShockwavePanel::ShockwavePanel(wxWindow* parent)
{
	//(*Initialize(ShockwavePanel)
	wxFlexGridSizer* FlexGridSizer110;
	wxTextCtrl* TextCtrl_Shockwave_Accel;
	wxTextCtrl* TextCtrl_Shockwave_CenterX;
	wxStaticText* StaticText154;
	wxTextCtrl* TextCtrl_Shockwave_CenterY;
	wxTextCtrl* TextCtrl_Shockwave_Start_Width;
	wxFlexGridSizer* FlexGridSizer100;
	wxTextCtrl* TextCtrl_Shockwave_Start_Radius;
	wxPanel* Panel14;
	wxPanel* Panel10;
	wxTextCtrl* TextCtrl_Shockwave_End_Width;
	wxTextCtrl* TextCtrl_Shockwave_End_Radius;
	wxFlexGridSizer* FlexGridSizer101;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer110 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer110->AddGrowableCol(0);
	Notebook3 = new wxNotebook(this, ID_NOTEBOOK_Shockwave, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK_Shockwave"));
	Panel10 = new wxPanel(Notebook3, ID_PANEL40, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL40"));
	FlexGridSizer100 = new wxFlexGridSizer(0, 5, 0, 0);
	FlexGridSizer100->AddGrowableCol(1);
	StaticText154 = new wxStaticText(Panel10, wxID_ANY, _("Center X:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer100->Add(StaticText154, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Shockwave_CenterX = new wxSlider(Panel10, ID_SLIDER_Shockwave_CenterX, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Shockwave_CenterX"));
	FlexGridSizer100->Add(Slider_Shockwave_CenterX, 1, wxALL|wxEXPAND, 5);
	BitmapButton_Shockwave_CenterX = new ValueCurveButton(Panel10, ID_VALUECURVE_Shockwave_CenterX, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Shockwave_CenterX"));
	FlexGridSizer100->Add(BitmapButton_Shockwave_CenterX, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Shockwave_CenterX = new wxTextCtrl(Panel10, IDD_TEXTCTRL_Shockwave_CenterX, _("50"), wxDefaultPosition, wxDLG_UNIT(Panel10,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Shockwave_CenterX"));
	TextCtrl_Shockwave_CenterX->SetMaxLength(3);
	FlexGridSizer100->Add(TextCtrl_Shockwave_CenterX, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, wxDLG_UNIT(Panel10,wxSize(5,0)).GetWidth());
	BitmapButton_Shockwave_Center_X = new wxBitmapButton(Panel10, ID_BITMAPBUTTON_SLIDER_Shockwave_CenterX, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Shockwave_CenterX"));
	BitmapButton_Shockwave_Center_X->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer100->Add(BitmapButton_Shockwave_Center_X, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText155 = new wxStaticText(Panel10, ID_STATICTEXT151, _("Center Y:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT151"));
	FlexGridSizer100->Add(StaticText155, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Shockwave_CenterY = new wxSlider(Panel10, ID_SLIDER_Shockwave_CenterY, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Shockwave_CenterY"));
	FlexGridSizer100->Add(Slider_Shockwave_CenterY, 1, wxALL|wxEXPAND, 5);
	BitmapButton_Shockwave_CenterY = new ValueCurveButton(Panel10, ID_VALUECURVE_Shockwave_CenterY, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Shockwave_CenterY"));
	FlexGridSizer100->Add(BitmapButton_Shockwave_CenterY, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Shockwave_CenterY = new wxTextCtrl(Panel10, IDD_TEXTCTRL_Shockwave_CenterY, _("50"), wxDefaultPosition, wxDLG_UNIT(Panel10,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Shockwave_CenterY"));
	TextCtrl_Shockwave_CenterY->SetMaxLength(3);
	FlexGridSizer100->Add(TextCtrl_Shockwave_CenterY, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Shockwave_Center_Y = new wxBitmapButton(Panel10, ID_BITMAPBUTTON_SLIDER_Shockwave_CenterY, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Shockwave_CenterY"));
	BitmapButton_Shockwave_Center_Y->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer100->Add(BitmapButton_Shockwave_Center_Y, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText156 = new wxStaticText(Panel10, ID_STATICTEXT152, _("Radius1:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT152"));
	FlexGridSizer100->Add(StaticText156, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Shockwave_Start_Radius = new wxSlider(Panel10, ID_SLIDER_Shockwave_Start_Radius, 1, 0, 250, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Shockwave_Start_Radius"));
	FlexGridSizer100->Add(Slider_Shockwave_Start_Radius, 1, wxALL|wxEXPAND, 5);
	BitmapButton_Shockwave_Start_Radius = new ValueCurveButton(Panel10, ID_VALUECURVE_Shockwave_Start_Radius, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Shockwave_Start_Radius"));
	FlexGridSizer100->Add(BitmapButton_Shockwave_Start_Radius, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Shockwave_Start_Radius = new wxTextCtrl(Panel10, IDD_TEXTCTRL_Shockwave_Start_Radius, _("1"), wxDefaultPosition, wxDLG_UNIT(Panel10,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Shockwave_Start_Radius"));
	TextCtrl_Shockwave_Start_Radius->SetMaxLength(3);
	FlexGridSizer100->Add(TextCtrl_Shockwave_Start_Radius, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Shockwave_StartRadius = new wxBitmapButton(Panel10, ID_BITMAPBUTTON_SLIDER_Shockwave_Start_Radius, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Shockwave_Start_Radius"));
	BitmapButton_Shockwave_StartRadius->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer100->Add(BitmapButton_Shockwave_StartRadius, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText158 = new wxStaticText(Panel10, ID_STATICTEXT154, _("Radius2:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT154"));
	FlexGridSizer100->Add(StaticText158, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Shockwave_End_Radius = new wxSlider(Panel10, ID_SLIDER_Shockwave_End_Radius, 10, 0, 250, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Shockwave_End_Radius"));
	FlexGridSizer100->Add(Slider_Shockwave_End_Radius, 1, wxALL|wxEXPAND, 5);
	BitmapButton_Shockwave_End_Radius = new ValueCurveButton(Panel10, ID_VALUECURVE_Shockwave_End_Radius, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Shockwave_End_Radius"));
	FlexGridSizer100->Add(BitmapButton_Shockwave_End_Radius, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Shockwave_End_Radius = new wxTextCtrl(Panel10, IDD_TEXTCTRL_Shockwave_End_Radius, _("10"), wxDefaultPosition, wxDLG_UNIT(Panel10,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Shockwave_End_Radius"));
	TextCtrl_Shockwave_End_Radius->SetMaxLength(3);
	FlexGridSizer100->Add(TextCtrl_Shockwave_End_Radius, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Shockwave_EndRadius = new wxBitmapButton(Panel10, ID_BITMAPBUTTON_SLIDER_Shockwave_End_Radius, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Shockwave_End_Radius"));
	BitmapButton_Shockwave_EndRadius->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer100->Add(BitmapButton_Shockwave_EndRadius, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText157 = new wxStaticText(Panel10, ID_STATICTEXT153, _("Width1:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT153"));
	FlexGridSizer100->Add(StaticText157, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Shockwave_Start_Width = new wxSlider(Panel10, ID_SLIDER_Shockwave_Start_Width, 5, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Shockwave_Start_Width"));
	FlexGridSizer100->Add(Slider_Shockwave_Start_Width, 1, wxALL|wxEXPAND, 5);
	BitmapButton_Shockwave_Start_Width = new ValueCurveButton(Panel10, ID_VALUECURVE_Shockwave_Start_Width, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Shockwave_Start_Width"));
	FlexGridSizer100->Add(BitmapButton_Shockwave_Start_Width, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Shockwave_Start_Width = new wxTextCtrl(Panel10, IDD_TEXTCTRL_Shockwave_Start_Width, _("5"), wxDefaultPosition, wxDLG_UNIT(Panel10,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Shockwave_Start_Width"));
	TextCtrl_Shockwave_Start_Width->SetMaxLength(3);
	FlexGridSizer100->Add(TextCtrl_Shockwave_Start_Width, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Shockwave_StartWidth = new wxBitmapButton(Panel10, ID_BITMAPBUTTON_SLIDER_Shockwave_Start_Width, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Shockwave_Start_Width"));
	BitmapButton_Shockwave_StartWidth->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer100->Add(BitmapButton_Shockwave_StartWidth, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText159 = new wxStaticText(Panel10, ID_STATICTEXT155, _("Width2:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT155"));
	FlexGridSizer100->Add(StaticText159, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Shockwave_End_Width = new wxSlider(Panel10, ID_SLIDER_Shockwave_End_Width, 10, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Shockwave_End_Width"));
	FlexGridSizer100->Add(Slider_Shockwave_End_Width, 1, wxALL|wxEXPAND, 5);
	BitmapButton_Shockwave_End_Width = new ValueCurveButton(Panel10, ID_VALUECURVE_Shockwave_End_Width, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Shockwave_End_Width"));
	FlexGridSizer100->Add(BitmapButton_Shockwave_End_Width, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Shockwave_End_Width = new wxTextCtrl(Panel10, IDD_TEXTCTRL_Shockwave_End_Width, _("10"), wxDefaultPosition, wxDLG_UNIT(Panel10,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Shockwave_End_Width"));
	TextCtrl_Shockwave_End_Width->SetMaxLength(3);
	FlexGridSizer100->Add(TextCtrl_Shockwave_End_Width, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Shockwave_EndWidth = new wxBitmapButton(Panel10, ID_BITMAPBUTTON_SLIDER_Shockwave_End_Width, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Shockwave_End_Width"));
	BitmapButton_Shockwave_EndWidth->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer100->Add(BitmapButton_Shockwave_EndWidth, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel10->SetSizer(FlexGridSizer100);
	FlexGridSizer100->Fit(Panel10);
	FlexGridSizer100->SetSizeHints(Panel10);
	Panel14 = new wxPanel(Notebook3, ID_PANEL44, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL44"));
	FlexGridSizer101 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer101->AddGrowableCol(1);
	StaticText166 = new wxStaticText(Panel14, ID_STATICTEXT162, _("Acceleration:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT162"));
	FlexGridSizer101->Add(StaticText166, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Shockwave_Accel = new wxSlider(Panel14, ID_SLIDER_Shockwave_Accel, 0, -10, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Shockwave_Accel"));
	FlexGridSizer101->Add(Slider_Shockwave_Accel, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Shockwave_Accel = new wxTextCtrl(Panel14, IDD_TEXTCTRL_Shockwave_Accel, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel14,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Shockwave_Accel"));
	TextCtrl_Shockwave_Accel->SetMaxLength(3);
	FlexGridSizer101->Add(TextCtrl_Shockwave_Accel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Shockwave_Accel = new wxBitmapButton(Panel14, ID_BITMAPBUTTON_SLIDER_Shockwave_Accel, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Shockwave_Accel"));
	BitmapButton_Shockwave_Accel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer101->Add(BitmapButton_Shockwave_Accel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Shockwave_Blend_Edges = new wxCheckBox(Panel14, ID_CHECKBOX_Shockwave_Blend_Edges, _("Blend Edges:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT, wxDefaultValidator, _T("ID_CHECKBOX_Shockwave_Blend_Edges"));
	CheckBox_Shockwave_Blend_Edges->SetValue(true);
	FlexGridSizer101->Add(CheckBox_Shockwave_Blend_Edges, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel14->SetSizer(FlexGridSizer101);
	FlexGridSizer101->Fit(Panel14);
	FlexGridSizer101->SetSizeHints(Panel14);
	Notebook3->AddPage(Panel10, _("Position"), false);
	Notebook3->AddPage(Panel14, _("Options"), false);
	FlexGridSizer110->Add(Notebook3, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer110);
	FlexGridSizer110->Fit(this);
	FlexGridSizer110->SetSizeHints(this);

	Connect(ID_SLIDER_Shockwave_CenterX,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&ShockwavePanel::UpdateLinkedTextCtrl);
	Connect(ID_VALUECURVE_Shockwave_CenterX,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ShockwavePanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_Shockwave_CenterX,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ShockwavePanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_SLIDER_Shockwave_CenterX,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ShockwavePanel::OnLockButtonClick);
	Connect(ID_SLIDER_Shockwave_CenterY,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&ShockwavePanel::UpdateLinkedTextCtrl);
	Connect(ID_VALUECURVE_Shockwave_CenterY,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ShockwavePanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_Shockwave_CenterY,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ShockwavePanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_SLIDER_Shockwave_CenterY,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ShockwavePanel::OnLockButtonClick);
	Connect(ID_SLIDER_Shockwave_Start_Radius,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&ShockwavePanel::UpdateLinkedTextCtrl);
	Connect(ID_VALUECURVE_Shockwave_Start_Radius,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ShockwavePanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_Shockwave_Start_Radius,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ShockwavePanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_SLIDER_Shockwave_Start_Radius,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ShockwavePanel::OnLockButtonClick);
	Connect(ID_SLIDER_Shockwave_End_Radius,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&ShockwavePanel::UpdateLinkedTextCtrl);
	Connect(ID_VALUECURVE_Shockwave_End_Radius,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ShockwavePanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_Shockwave_End_Radius,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ShockwavePanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_SLIDER_Shockwave_End_Radius,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ShockwavePanel::OnLockButtonClick);
	Connect(ID_SLIDER_Shockwave_Start_Width,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&ShockwavePanel::UpdateLinkedTextCtrl);
	Connect(ID_VALUECURVE_Shockwave_Start_Width,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ShockwavePanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_Shockwave_Start_Width,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ShockwavePanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_SLIDER_Shockwave_Start_Width,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ShockwavePanel::OnLockButtonClick);
	Connect(ID_SLIDER_Shockwave_End_Width,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&ShockwavePanel::UpdateLinkedTextCtrl);
	Connect(ID_VALUECURVE_Shockwave_End_Width,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ShockwavePanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_Shockwave_End_Width,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ShockwavePanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_SLIDER_Shockwave_End_Width,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ShockwavePanel::OnLockButtonClick);
	Connect(ID_SLIDER_Shockwave_Accel,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&ShockwavePanel::UpdateLinkedTextCtrl);
	Connect(IDD_TEXTCTRL_Shockwave_Accel,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ShockwavePanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_SLIDER_Shockwave_Accel,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ShockwavePanel::OnLockButtonClick);
	//*)
    SetName("ID_PANEL_SHOCKWAVE");

    BitmapButton_Shockwave_CenterX->GetValue()->SetLimits(SHOCKWAVE_X_MIN, SHOCKWAVE_X_MAX);
    BitmapButton_Shockwave_CenterY->GetValue()->SetLimits(SHOCKWAVE_Y_MIN, SHOCKWAVE_Y_MAX);
    BitmapButton_Shockwave_Start_Width->GetValue()->SetLimits(SHOCKWAVE_STARTWIDTH_MIN, SHOCKWAVE_STARTWIDTH_MAX);
    BitmapButton_Shockwave_End_Width->GetValue()->SetLimits(SHOCKWAVE_ENDWIDTH_MIN, SHOCKWAVE_ENDWIDTH_MAX);
    BitmapButton_Shockwave_Start_Radius->GetValue()->SetLimits(SHOCKWAVE_STARTRADIUS_MIN, SHOCKWAVE_STARTRADIUS_MAX);
    BitmapButton_Shockwave_End_Radius->GetValue()->SetLimits(SHOCKWAVE_ENDRADIUS_MIN, SHOCKWAVE_ENDRADIUS_MAX);
}

ShockwavePanel::~ShockwavePanel()
{
	//(*Destroy(ShockwavePanel)
	//*)
}

PANEL_EVENT_HANDLERS(ShockwavePanel)
