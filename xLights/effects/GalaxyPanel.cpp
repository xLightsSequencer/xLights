#include "GalaxyPanel.h"
#include "../../include/padlock16x16-blue.xpm"
#include "EffectPanelUtils.h"
#include "GalaxyEffect.h"

//(*InternalHeaders(GalaxyPanel)
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
//*)

//(*IdInit(GalaxyPanel)
const long GalaxyPanel::ID_SLIDER_Galaxy_CenterX = wxNewId();
const long GalaxyPanel::ID_VALUECURVE_Galaxy_CenterX = wxNewId();
const long GalaxyPanel::IDD_TEXTCTRL_Galaxy_CenterX = wxNewId();
const long GalaxyPanel::ID_BITMAPBUTTON_SLIDER_Galaxy_CenterX = wxNewId();
const long GalaxyPanel::ID_STATICTEXT2 = wxNewId();
const long GalaxyPanel::ID_SLIDER_Galaxy_CenterY = wxNewId();
const long GalaxyPanel::ID_VALUECURVE_Galaxy_CenterY = wxNewId();
const long GalaxyPanel::IDD_TEXTCTRL_Galaxy_CenterY = wxNewId();
const long GalaxyPanel::ID_BITMAPBUTTON_SLIDER_Galaxy_CenterY = wxNewId();
const long GalaxyPanel::ID_STATICTEXT125 = wxNewId();
const long GalaxyPanel::ID_SLIDER_Galaxy_Start_Radius = wxNewId();
const long GalaxyPanel::ID_VALUECURVE_Galaxy_Start_Radius = wxNewId();
const long GalaxyPanel::IDD_TEXTCTRL_Galaxy_Start_Radius = wxNewId();
const long GalaxyPanel::ID_BITMAPBUTTON_SLIDER_Galaxy_Start_Radius = wxNewId();
const long GalaxyPanel::ID_STATICTEXT127 = wxNewId();
const long GalaxyPanel::ID_SLIDER_Galaxy_Start_Width = wxNewId();
const long GalaxyPanel::ID_VALUECURVE_Galaxy_Start_Width = wxNewId();
const long GalaxyPanel::IDD_TEXTCTRL_Galaxy_Start_Width = wxNewId();
const long GalaxyPanel::ID_BITMAPBUTTON_SLIDER_Galaxy_Start_Width = wxNewId();
const long GalaxyPanel::ID_STATICTEXT126 = wxNewId();
const long GalaxyPanel::ID_SLIDER_Galaxy_Start_Angle = wxNewId();
const long GalaxyPanel::ID_VALUECURVE_Galaxy_Start_Angle = wxNewId();
const long GalaxyPanel::IDD_TEXTCTRL_Galaxy_Start_Angle = wxNewId();
const long GalaxyPanel::ID_BITMAPBUTTON_SLIDER_Galaxy_Start_Angle = wxNewId();
const long GalaxyPanel::ID_PANEL29 = wxNewId();
const long GalaxyPanel::ID_STATICTEXT130 = wxNewId();
const long GalaxyPanel::ID_SLIDER_Galaxy_End_Radius = wxNewId();
const long GalaxyPanel::ID_VALUECURVE_Galaxy_End_Radius = wxNewId();
const long GalaxyPanel::IDD_TEXTCTRL_Galaxy_End_Radius = wxNewId();
const long GalaxyPanel::ID_BITMAPBUTTON_SLIDER_Galaxy_End_Radius = wxNewId();
const long GalaxyPanel::ID_STATICTEXT132 = wxNewId();
const long GalaxyPanel::ID_SLIDER_Galaxy_End_Width = wxNewId();
const long GalaxyPanel::ID_VALUECURVE_Galaxy_End_Width = wxNewId();
const long GalaxyPanel::IDD_TEXTCTRL_Galaxy_End_Width = wxNewId();
const long GalaxyPanel::ID_BITMAPBUTTON_SLIDER_Galaxy_End_Width = wxNewId();
const long GalaxyPanel::ID_STATICTEXT134 = wxNewId();
const long GalaxyPanel::ID_SLIDER_Galaxy_Revolutions = wxNewId();
const long GalaxyPanel::ID_VALUECURVE_Galaxy_Revolutions = wxNewId();
const long GalaxyPanel::IDD_TEXTCTRL_Galaxy_Revolutions = wxNewId();
const long GalaxyPanel::ID_BITMAPBUTTON_SLIDER_Galaxy_Revolutions = wxNewId();
const long GalaxyPanel::ID_PANEL31 = wxNewId();
const long GalaxyPanel::ID_STATICTEXT139 = wxNewId();
const long GalaxyPanel::ID_SLIDER_Galaxy_Duration = wxNewId();
const long GalaxyPanel::ID_VALUECURVE_Galaxy_Duration = wxNewId();
const long GalaxyPanel::IDD_TEXTCTRL_Galaxy_Duration = wxNewId();
const long GalaxyPanel::ID_BITMAPBUTTON_SLIDER_Galaxy_Duration = wxNewId();
const long GalaxyPanel::ID_STATICTEXT140 = wxNewId();
const long GalaxyPanel::ID_SLIDER_Galaxy_Accel = wxNewId();
const long GalaxyPanel::ID_VALUECURVE_Galaxy_Accel = wxNewId();
const long GalaxyPanel::IDD_TEXTCTRL_Galaxy_Accel = wxNewId();
const long GalaxyPanel::ID_BITMAPBUTTON_SLIDER_Galaxy_Accel = wxNewId();
const long GalaxyPanel::ID_CHECKBOX_Galaxy_Reverse = wxNewId();
const long GalaxyPanel::ID_CHECKBOX_Galaxy_Blend_Edges = wxNewId();
const long GalaxyPanel::ID_CHECKBOX_Galaxy_Inward = wxNewId();
const long GalaxyPanel::ID_PANEL32 = wxNewId();
const long GalaxyPanel::ID_NOTEBOOK_Galaxy = wxNewId();
//*)

BEGIN_EVENT_TABLE(GalaxyPanel,wxPanel)
	//(*EventTable(GalaxyPanel)
	//*)
END_EVENT_TABLE()

GalaxyPanel::GalaxyPanel(wxWindow* parent)
{
	//(*Initialize(GalaxyPanel)
	wxFlexGridSizer* FlexGridSizer108;
	wxFlexGridSizer* GridBagSizerGalaxyOptions;
	wxStaticText* StaticText125;
	wxFlexGridSizer* GridBagSizerGalaxyEnd;
	wxPanel* Panel8;
	wxTextCtrl* TextCtrl_Galaxy_CenterX;
	wxPanel* Panel6;
	wxTextCtrl* TextCtrl_Galaxy_CenterY;
	wxPanel* Panel7;
	wxTextCtrl* TextCtrl_Galaxy_Duration;
	wxTextCtrl* TextCtrl_Galaxy_Accel;
	wxTextCtrl* TextCtrl_Galaxy_Start_Width;
	wxTextCtrl* TextCtrl_Galaxy_End_Width;
	wxTextCtrl* TextCtrl_Galaxy_Start_Radius;
	wxTextCtrl* TextCtrl_Galaxy_Revolutions;
	wxTextCtrl* TextCtrl_Galaxy_End_Radius;
	wxTextCtrl* TextCtrl_Galaxy_Start_Angle;
	wxFlexGridSizer* GridBagSizerGalaxyStart;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer108 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer108->AddGrowableCol(0);
	Notebook_Galaxy = new wxNotebook(this, ID_NOTEBOOK_Galaxy, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK_Galaxy"));
	Panel6 = new wxPanel(Notebook_Galaxy, ID_PANEL29, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL29"));
	GridBagSizerGalaxyStart = new wxFlexGridSizer(0, 5, 0, 0);
	GridBagSizerGalaxyStart->AddGrowableCol(1);
	StaticText125 = new wxStaticText(Panel6, wxID_ANY, _("Center X:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	GridBagSizerGalaxyStart->Add(StaticText125, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Galaxy_CenterX = new wxSlider(Panel6, ID_SLIDER_Galaxy_CenterX, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Galaxy_CenterX"));
	GridBagSizerGalaxyStart->Add(Slider_Galaxy_CenterX, 1, wxALL|wxEXPAND, 5);
	ValueCurve_Galaxy_CenterX = new ValueCurveButton(Panel6, ID_VALUECURVE_Galaxy_CenterX, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Galaxy_CenterX"));
	GridBagSizerGalaxyStart->Add(ValueCurve_Galaxy_CenterX, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Galaxy_CenterX = new wxTextCtrl(Panel6, IDD_TEXTCTRL_Galaxy_CenterX, _("50"), wxDefaultPosition, wxDLG_UNIT(Panel6,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Galaxy_CenterX"));
	TextCtrl_Galaxy_CenterX->SetMaxLength(3);
	GridBagSizerGalaxyStart->Add(TextCtrl_Galaxy_CenterX, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, wxDLG_UNIT(Panel6,wxSize(5,0)).GetWidth());
	BitmapButton_Galaxy_CenterX = new wxBitmapButton(Panel6, ID_BITMAPBUTTON_SLIDER_Galaxy_CenterX, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Galaxy_CenterX"));
	BitmapButton_Galaxy_CenterX->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	GridBagSizerGalaxyStart->Add(BitmapButton_Galaxy_CenterX, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText126 = new wxStaticText(Panel6, ID_STATICTEXT2, _("Center Y:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	GridBagSizerGalaxyStart->Add(StaticText126, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Galaxy_CenterY = new wxSlider(Panel6, ID_SLIDER_Galaxy_CenterY, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Galaxy_CenterY"));
	GridBagSizerGalaxyStart->Add(Slider_Galaxy_CenterY, 1, wxALL|wxEXPAND, 5);
	ValueCurve_Galaxy_CenterY = new ValueCurveButton(Panel6, ID_VALUECURVE_Galaxy_CenterY, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Galaxy_CenterY"));
	GridBagSizerGalaxyStart->Add(ValueCurve_Galaxy_CenterY, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Galaxy_CenterY = new wxTextCtrl(Panel6, IDD_TEXTCTRL_Galaxy_CenterY, _("50"), wxDefaultPosition, wxDLG_UNIT(Panel6,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Galaxy_CenterY"));
	TextCtrl_Galaxy_CenterY->SetMaxLength(3);
	GridBagSizerGalaxyStart->Add(TextCtrl_Galaxy_CenterY, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Galaxy_CenterY = new wxBitmapButton(Panel6, ID_BITMAPBUTTON_SLIDER_Galaxy_CenterY, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Galaxy_CenterY"));
	BitmapButton_Galaxy_CenterY->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	GridBagSizerGalaxyStart->Add(BitmapButton_Galaxy_CenterY, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText127 = new wxStaticText(Panel6, ID_STATICTEXT125, _("Radius:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT125"));
	GridBagSizerGalaxyStart->Add(StaticText127, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Galaxy_Start_Radius = new wxSlider(Panel6, ID_SLIDER_Galaxy_Start_Radius, 1, 0, 250, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Galaxy_Start_Radius"));
	GridBagSizerGalaxyStart->Add(Slider_Galaxy_Start_Radius, 1, wxALL|wxEXPAND, 5);
	ValueCurve_Galaxy_Start_Radius = new ValueCurveButton(Panel6, ID_VALUECURVE_Galaxy_Start_Radius, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Galaxy_Start_Radius"));
	GridBagSizerGalaxyStart->Add(ValueCurve_Galaxy_Start_Radius, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Galaxy_Start_Radius = new wxTextCtrl(Panel6, IDD_TEXTCTRL_Galaxy_Start_Radius, _("1"), wxDefaultPosition, wxDLG_UNIT(Panel6,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Galaxy_Start_Radius"));
	TextCtrl_Galaxy_Start_Radius->SetMaxLength(3);
	GridBagSizerGalaxyStart->Add(TextCtrl_Galaxy_Start_Radius, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Galaxy_Start_Radius = new wxBitmapButton(Panel6, ID_BITMAPBUTTON_SLIDER_Galaxy_Start_Radius, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Galaxy_Start_Radius"));
	BitmapButton_Galaxy_Start_Radius->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	GridBagSizerGalaxyStart->Add(BitmapButton_Galaxy_Start_Radius, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText131 = new wxStaticText(Panel6, ID_STATICTEXT127, _("Width:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT127"));
	GridBagSizerGalaxyStart->Add(StaticText131, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Galaxy_Start_Width = new wxSlider(Panel6, ID_SLIDER_Galaxy_Start_Width, 5, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Galaxy_Start_Width"));
	GridBagSizerGalaxyStart->Add(Slider_Galaxy_Start_Width, 1, wxALL|wxEXPAND, 5);
	ValueCurve_Galaxy_Start_Width = new ValueCurveButton(Panel6, ID_VALUECURVE_Galaxy_Start_Width, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Galaxy_Start_Width"));
	GridBagSizerGalaxyStart->Add(ValueCurve_Galaxy_Start_Width, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Galaxy_Start_Width = new wxTextCtrl(Panel6, IDD_TEXTCTRL_Galaxy_Start_Width, _("5"), wxDefaultPosition, wxDLG_UNIT(Panel6,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Galaxy_Start_Width"));
	TextCtrl_Galaxy_Start_Width->SetMaxLength(3);
	GridBagSizerGalaxyStart->Add(TextCtrl_Galaxy_Start_Width, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Galaxy_Start_Width = new wxBitmapButton(Panel6, ID_BITMAPBUTTON_SLIDER_Galaxy_Start_Width, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Galaxy_Start_Width"));
	BitmapButton_Galaxy_Start_Width->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	GridBagSizerGalaxyStart->Add(BitmapButton_Galaxy_Start_Width, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText129 = new wxStaticText(Panel6, ID_STATICTEXT126, _("Angle:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT126"));
	GridBagSizerGalaxyStart->Add(StaticText129, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Galaxy_Start_Angle = new wxSlider(Panel6, ID_SLIDER_Galaxy_Start_Angle, 0, 0, 360, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Galaxy_Start_Angle"));
	GridBagSizerGalaxyStart->Add(Slider_Galaxy_Start_Angle, 1, wxALL|wxEXPAND, 5);
	ValueCurve_Galaxy_Start_Angle = new ValueCurveButton(Panel6, ID_VALUECURVE_Galaxy_Start_Angle, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Galaxy_Start_Angle"));
	GridBagSizerGalaxyStart->Add(ValueCurve_Galaxy_Start_Angle, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Galaxy_Start_Angle = new wxTextCtrl(Panel6, IDD_TEXTCTRL_Galaxy_Start_Angle, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel6,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Galaxy_Start_Angle"));
	TextCtrl_Galaxy_Start_Angle->SetMaxLength(3);
	GridBagSizerGalaxyStart->Add(TextCtrl_Galaxy_Start_Angle, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Galaxy_Start_Angle = new wxBitmapButton(Panel6, ID_BITMAPBUTTON_SLIDER_Galaxy_Start_Angle, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Galaxy_Start_Angle"));
	BitmapButton_Galaxy_Start_Angle->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	GridBagSizerGalaxyStart->Add(BitmapButton_Galaxy_Start_Angle, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel6->SetSizer(GridBagSizerGalaxyStart);
	GridBagSizerGalaxyStart->Fit(Panel6);
	GridBagSizerGalaxyStart->SetSizeHints(Panel6);
	Panel7 = new wxPanel(Notebook_Galaxy, ID_PANEL31, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL31"));
	GridBagSizerGalaxyEnd = new wxFlexGridSizer(0, 5, 0, 0);
	GridBagSizerGalaxyEnd->AddGrowableCol(1);
	StaticText138 = new wxStaticText(Panel7, ID_STATICTEXT130, _("Radius:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT130"));
	GridBagSizerGalaxyEnd->Add(StaticText138, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Galaxy_End_Radius = new wxSlider(Panel7, ID_SLIDER_Galaxy_End_Radius, 10, 0, 250, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Galaxy_End_Radius"));
	GridBagSizerGalaxyEnd->Add(Slider_Galaxy_End_Radius, 1, wxALL|wxEXPAND, 5);
	ValueCurve_Galaxy_End_Radius = new ValueCurveButton(Panel7, ID_VALUECURVE_Galaxy_End_Radius, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Galaxy_End_Radius"));
	GridBagSizerGalaxyEnd->Add(ValueCurve_Galaxy_End_Radius, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Galaxy_End_Radius = new wxTextCtrl(Panel7, IDD_TEXTCTRL_Galaxy_End_Radius, _("10"), wxDefaultPosition, wxDLG_UNIT(Panel7,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Galaxy_End_Radius"));
	TextCtrl_Galaxy_End_Radius->SetMaxLength(3);
	GridBagSizerGalaxyEnd->Add(TextCtrl_Galaxy_End_Radius, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Galaxy_End_Radius = new wxBitmapButton(Panel7, ID_BITMAPBUTTON_SLIDER_Galaxy_End_Radius, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Galaxy_End_Radius"));
	BitmapButton_Galaxy_End_Radius->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	GridBagSizerGalaxyEnd->Add(BitmapButton_Galaxy_End_Radius, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText139 = new wxStaticText(Panel7, ID_STATICTEXT132, _("Width:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT132"));
	GridBagSizerGalaxyEnd->Add(StaticText139, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Galaxy_End_Width = new wxSlider(Panel7, ID_SLIDER_Galaxy_End_Width, 5, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Galaxy_End_Width"));
	GridBagSizerGalaxyEnd->Add(Slider_Galaxy_End_Width, 1, wxALL|wxEXPAND, 5);
	ValueCurve_Galaxy_End_Width = new ValueCurveButton(Panel7, ID_VALUECURVE_Galaxy_End_Width, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Galaxy_End_Width"));
	GridBagSizerGalaxyEnd->Add(ValueCurve_Galaxy_End_Width, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Galaxy_End_Width = new wxTextCtrl(Panel7, IDD_TEXTCTRL_Galaxy_End_Width, _("5"), wxDefaultPosition, wxDLG_UNIT(Panel7,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Galaxy_End_Width"));
	TextCtrl_Galaxy_End_Width->SetMaxLength(3);
	GridBagSizerGalaxyEnd->Add(TextCtrl_Galaxy_End_Width, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Galaxy_End_Width = new wxBitmapButton(Panel7, ID_BITMAPBUTTON_SLIDER_Galaxy_End_Width, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Galaxy_End_Width"));
	BitmapButton_Galaxy_End_Width->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	GridBagSizerGalaxyEnd->Add(BitmapButton_Galaxy_End_Width, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText140 = new wxStaticText(Panel7, ID_STATICTEXT134, _("Rev\'s:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT134"));
	GridBagSizerGalaxyEnd->Add(StaticText140, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Galaxy_Revolutions = new wxSlider(Panel7, ID_SLIDER_Galaxy_Revolutions, 1440, 0, 3600, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Galaxy_Revolutions"));
	GridBagSizerGalaxyEnd->Add(Slider_Galaxy_Revolutions, 1, wxALL|wxEXPAND, 5);
	ValueCurve_Galaxy_Revolutions = new ValueCurveButton(Panel7, ID_VALUECURVE_Galaxy_Revolutions, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Galaxy_Revolutions"));
	GridBagSizerGalaxyEnd->Add(ValueCurve_Galaxy_Revolutions, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Galaxy_Revolutions = new wxTextCtrl(Panel7, IDD_TEXTCTRL_Galaxy_Revolutions, _("4.0"), wxDefaultPosition, wxDLG_UNIT(Panel7,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Galaxy_Revolutions"));
	TextCtrl_Galaxy_Revolutions->SetMaxLength(4);
	GridBagSizerGalaxyEnd->Add(TextCtrl_Galaxy_Revolutions, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Galaxy_Revolutions = new wxBitmapButton(Panel7, ID_BITMAPBUTTON_SLIDER_Galaxy_Revolutions, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Galaxy_Revolutions"));
	BitmapButton_Galaxy_Revolutions->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	GridBagSizerGalaxyEnd->Add(BitmapButton_Galaxy_Revolutions, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel7->SetSizer(GridBagSizerGalaxyEnd);
	GridBagSizerGalaxyEnd->Fit(Panel7);
	GridBagSizerGalaxyEnd->SetSizeHints(Panel7);
	Panel8 = new wxPanel(Notebook_Galaxy, ID_PANEL32, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL32"));
	GridBagSizerGalaxyOptions = new wxFlexGridSizer(0, 5, 0, 0);
	GridBagSizerGalaxyOptions->AddGrowableCol(1);
	StaticText141 = new wxStaticText(Panel8, ID_STATICTEXT139, _("Head Duration:"), wxDefaultPosition, wxSize(-1,-1), 0, _T("ID_STATICTEXT139"));
	GridBagSizerGalaxyOptions->Add(StaticText141, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Galaxy_Duration = new wxSlider(Panel8, ID_SLIDER_Galaxy_Duration, 20, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Galaxy_Duration"));
	GridBagSizerGalaxyOptions->Add(Slider_Galaxy_Duration, 1, wxALL|wxEXPAND, 5);
	ValueCurve_Galaxy_Duration = new ValueCurveButton(Panel8, ID_VALUECURVE_Galaxy_Duration, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Galaxy_Duration"));
	GridBagSizerGalaxyOptions->Add(ValueCurve_Galaxy_Duration, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Galaxy_Duration = new wxTextCtrl(Panel8, IDD_TEXTCTRL_Galaxy_Duration, _("20"), wxDefaultPosition, wxDLG_UNIT(Panel8,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Galaxy_Duration"));
	TextCtrl_Galaxy_Duration->SetMaxLength(3);
	GridBagSizerGalaxyOptions->Add(TextCtrl_Galaxy_Duration, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Galaxy_Duration = new wxBitmapButton(Panel8, ID_BITMAPBUTTON_SLIDER_Galaxy_Duration, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Galaxy_Duration"));
	BitmapButton_Galaxy_Duration->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	GridBagSizerGalaxyOptions->Add(BitmapButton_Galaxy_Duration, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText142 = new wxStaticText(Panel8, ID_STATICTEXT140, _("Acceleration:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT140"));
	GridBagSizerGalaxyOptions->Add(StaticText142, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Galaxy_Accel = new wxSlider(Panel8, ID_SLIDER_Galaxy_Accel, 0, -10, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Galaxy_Accel"));
	GridBagSizerGalaxyOptions->Add(Slider_Galaxy_Accel, 1, wxALL|wxEXPAND, 5);
	ValueCurve_Galaxy_Accel = new ValueCurveButton(Panel8, ID_VALUECURVE_Galaxy_Accel, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Galaxy_Accel"));
	GridBagSizerGalaxyOptions->Add(ValueCurve_Galaxy_Accel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Galaxy_Accel = new wxTextCtrl(Panel8, IDD_TEXTCTRL_Galaxy_Accel, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel8,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Galaxy_Accel"));
	TextCtrl_Galaxy_Accel->SetMaxLength(3);
	GridBagSizerGalaxyOptions->Add(TextCtrl_Galaxy_Accel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Galaxy_Accel = new wxBitmapButton(Panel8, ID_BITMAPBUTTON_SLIDER_Galaxy_Accel, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Galaxy_Accel"));
	BitmapButton_Galaxy_Accel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	GridBagSizerGalaxyOptions->Add(BitmapButton_Galaxy_Accel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Galaxy_Reverse = new wxCheckBox(Panel8, ID_CHECKBOX_Galaxy_Reverse, _("Reverse:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT, wxDefaultValidator, _T("ID_CHECKBOX_Galaxy_Reverse"));
	CheckBox_Galaxy_Reverse->SetValue(false);
	GridBagSizerGalaxyOptions->Add(CheckBox_Galaxy_Reverse, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	GridBagSizerGalaxyOptions->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	GridBagSizerGalaxyOptions->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	GridBagSizerGalaxyOptions->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	GridBagSizerGalaxyOptions->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Galaxy_Blend_Edges = new wxCheckBox(Panel8, ID_CHECKBOX_Galaxy_Blend_Edges, _("Blend Edges:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT, wxDefaultValidator, _T("ID_CHECKBOX_Galaxy_Blend_Edges"));
	CheckBox_Galaxy_Blend_Edges->SetValue(true);
	GridBagSizerGalaxyOptions->Add(CheckBox_Galaxy_Blend_Edges, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	GridBagSizerGalaxyOptions->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	GridBagSizerGalaxyOptions->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	GridBagSizerGalaxyOptions->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	GridBagSizerGalaxyOptions->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Galaxy_Inward = new wxCheckBox(Panel8, ID_CHECKBOX_Galaxy_Inward, _("Inward:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT, wxDefaultValidator, _T("ID_CHECKBOX_Galaxy_Inward"));
	CheckBox_Galaxy_Inward->SetValue(false);
	GridBagSizerGalaxyOptions->Add(CheckBox_Galaxy_Inward, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Panel8->SetSizer(GridBagSizerGalaxyOptions);
	GridBagSizerGalaxyOptions->Fit(Panel8);
	GridBagSizerGalaxyOptions->SetSizeHints(Panel8);
	Notebook_Galaxy->AddPage(Panel6, _("Start"), false);
	Notebook_Galaxy->AddPage(Panel7, _("End"), false);
	Notebook_Galaxy->AddPage(Panel8, _("Options"), false);
	FlexGridSizer108->Add(Notebook_Galaxy, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer108);
	FlexGridSizer108->Fit(this);
	FlexGridSizer108->SetSizeHints(this);

	Connect(ID_SLIDER_Galaxy_CenterX,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&GalaxyPanel::UpdateLinkedTextCtrlVC);
	Connect(ID_VALUECURVE_Galaxy_CenterX,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GalaxyPanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_Galaxy_CenterX,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&GalaxyPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_SLIDER_Galaxy_CenterX,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GalaxyPanel::OnLockButtonClick);
	Connect(ID_SLIDER_Galaxy_CenterY,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&GalaxyPanel::UpdateLinkedTextCtrlVC);
	Connect(ID_VALUECURVE_Galaxy_CenterY,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GalaxyPanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_Galaxy_CenterY,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&GalaxyPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_SLIDER_Galaxy_CenterY,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GalaxyPanel::OnLockButtonClick);
	Connect(ID_SLIDER_Galaxy_Start_Radius,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&GalaxyPanel::UpdateLinkedTextCtrlVC);
	Connect(ID_VALUECURVE_Galaxy_Start_Radius,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GalaxyPanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_Galaxy_Start_Radius,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&GalaxyPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_SLIDER_Galaxy_Start_Radius,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GalaxyPanel::OnLockButtonClick);
	Connect(ID_SLIDER_Galaxy_Start_Width,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&GalaxyPanel::UpdateLinkedTextCtrlVC);
	Connect(ID_VALUECURVE_Galaxy_Start_Width,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GalaxyPanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_Galaxy_Start_Width,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&GalaxyPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_SLIDER_Galaxy_Start_Width,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GalaxyPanel::OnLockButtonClick);
	Connect(ID_SLIDER_Galaxy_Start_Angle,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&GalaxyPanel::UpdateLinkedTextCtrlVC);
	Connect(ID_VALUECURVE_Galaxy_Start_Angle,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GalaxyPanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_Galaxy_Start_Angle,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&GalaxyPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_SLIDER_Galaxy_Start_Angle,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GalaxyPanel::OnLockButtonClick);
	Connect(ID_SLIDER_Galaxy_End_Radius,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&GalaxyPanel::UpdateLinkedTextCtrlVC);
	Connect(ID_VALUECURVE_Galaxy_End_Radius,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GalaxyPanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_Galaxy_End_Radius,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&GalaxyPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_SLIDER_Galaxy_End_Radius,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GalaxyPanel::OnLockButtonClick);
	Connect(ID_SLIDER_Galaxy_End_Width,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&GalaxyPanel::UpdateLinkedTextCtrlVC);
	Connect(ID_VALUECURVE_Galaxy_End_Width,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GalaxyPanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_Galaxy_End_Width,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&GalaxyPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_SLIDER_Galaxy_End_Width,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GalaxyPanel::OnLockButtonClick);
	Connect(ID_SLIDER_Galaxy_Revolutions,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&GalaxyPanel::UpdateLinkedTextCtrlFloatVC);
	Connect(ID_VALUECURVE_Galaxy_Revolutions,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GalaxyPanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_Galaxy_Revolutions,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&GalaxyPanel::UpdateLinkedSlider360);
	Connect(ID_BITMAPBUTTON_SLIDER_Galaxy_Revolutions,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GalaxyPanel::OnLockButtonClick);
	Connect(ID_SLIDER_Galaxy_Duration,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&GalaxyPanel::UpdateLinkedTextCtrlVC);
	Connect(ID_VALUECURVE_Galaxy_Duration,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GalaxyPanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_Galaxy_Duration,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&GalaxyPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_SLIDER_Galaxy_Duration,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GalaxyPanel::OnLockButtonClick);
	Connect(ID_SLIDER_Galaxy_Accel,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&GalaxyPanel::UpdateLinkedTextCtrlVC);
	Connect(ID_VALUECURVE_Galaxy_Accel,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GalaxyPanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_Galaxy_Accel,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&GalaxyPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_SLIDER_Galaxy_Accel,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GalaxyPanel::OnLockButtonClick);
	//*)
    SetName("ID_PANEL_GALAXY");

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&GalaxyPanel::OnVCChanged, 0, this);

    ValueCurve_Galaxy_CenterX->GetValue()->SetLimits(GALAXY_CENTREX_MIN, GALAXY_CENTREX_MAX);
    ValueCurve_Galaxy_CenterY->GetValue()->SetLimits(GALAXY_CENTREY_MIN, GALAXY_CENTREY_MAX);
    ValueCurve_Galaxy_Start_Radius->GetValue()->SetLimits(GALAXY_STARTRADIUS_MIN, GALAXY_STARTRADIUS_MAX);
    ValueCurve_Galaxy_End_Radius->GetValue()->SetLimits(GALAXY_ENDRADIUS_MIN, GALAXY_ENDRADIUS_MAX);
    ValueCurve_Galaxy_Start_Angle->GetValue()->SetLimits(GALAXY_STARTANGLE_MIN, GALAXY_STARTANGLE_MAX);
    ValueCurve_Galaxy_Revolutions->GetValue()->SetLimits(GALAXY_REVOLUTIONS_MIN, GALAXY_REVOLUTIONS_MAX);
    ValueCurve_Galaxy_Revolutions->GetValue()->SetDivisor(360);
    ValueCurve_Galaxy_Start_Width->GetValue()->SetLimits(GALAXY_STARTWIDTH_MIN, GALAXY_STARTWIDTH_MAX);
    ValueCurve_Galaxy_End_Width->GetValue()->SetLimits(GALAXY_ENDWIDTH_MIN, GALAXY_ENDWIDTH_MAX);
    ValueCurve_Galaxy_Duration->GetValue()->SetLimits(GALAXY_DURATION_MIN, GALAXY_DURATION_MAX);
    ValueCurve_Galaxy_Accel->GetValue()->SetLimits(GALAXY_ACCEL_MIN, GALAXY_ACCEL_MAX);
}

GalaxyPanel::~GalaxyPanel()
{
	//(*Destroy(GalaxyPanel)
	//*)
}

PANEL_EVENT_HANDLERS(GalaxyPanel)
