#include "FanPanel.h"
#include "../../include/padlock16x16-blue.xpm"
#include "EffectPanelUtils.h"
#include "FanEffect.h"

//(*InternalHeaders(FanPanel)
#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#include <wx/checkbox.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/notebook.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>
//*)

//(*IdInit(FanPanel)
const long FanPanel::ID_STATICTEXT_Fan_CenterX = wxNewId();
const long FanPanel::ID_SLIDER_Fan_CenterX = wxNewId();
const long FanPanel::ID_VALUECURVE_Fan_CenterX = wxNewId();
const long FanPanel::IDD_TEXTCTRL_Fan_CenterX = wxNewId();
const long FanPanel::ID_BITMAPBUTTON_SLIDER_Fan_CenterX = wxNewId();
const long FanPanel::ID_STATICTEXT_Fan_CenterY = wxNewId();
const long FanPanel::ID_SLIDER_Fan_CenterY = wxNewId();
const long FanPanel::ID_VALUECURVE_Fan_CenterY = wxNewId();
const long FanPanel::IDD_TEXTCTRL_Fan_CenterY = wxNewId();
const long FanPanel::ID_BITMAPBUTTON_SLIDER_Fan_CenterY = wxNewId();
const long FanPanel::ID_STATICTEXT_Fan_Start_Radius = wxNewId();
const long FanPanel::ID_SLIDER_Fan_Start_Radius = wxNewId();
const long FanPanel::ID_VALUECURVE_Fan_Start_Radius = wxNewId();
const long FanPanel::IDD_TEXTCTRL_Fan_Start_Radius = wxNewId();
const long FanPanel::ID_BITMAPBUTTON_SLIDER_Fan_Start_Radius = wxNewId();
const long FanPanel::ID_STATICTEXT_Fan_Start_Angle = wxNewId();
const long FanPanel::ID_SLIDER_Fan_Start_Angle = wxNewId();
const long FanPanel::ID_VALUECURVE_Fan_Start_Angle = wxNewId();
const long FanPanel::IDD_TEXTCTRL_Fan_Start_Angle = wxNewId();
const long FanPanel::ID_BITMAPBUTTON_SLIDER_Fan_Start_Angle = wxNewId();
const long FanPanel::ID_STATICTEXT_Fan_End_Radius = wxNewId();
const long FanPanel::ID_SLIDER_Fan_End_Radius = wxNewId();
const long FanPanel::ID_VALUECURVE_Fan_End_Radius = wxNewId();
const long FanPanel::IDD_TEXTCTRL_Fan_End_Radius = wxNewId();
const long FanPanel::ID_BITMAPBUTTON_SLIDER_Fan_End_Radius = wxNewId();
const long FanPanel::ID_STATICTEXT_Fan_Revolutions = wxNewId();
const long FanPanel::ID_SLIDER_Fan_Revolutions = wxNewId();
const long FanPanel::ID_VALUECURVE_Fan_Revolutions = wxNewId();
const long FanPanel::IDD_TEXTCTRL_Fan_Revolutions = wxNewId();
const long FanPanel::ID_BITMAPBUTTON_SLIDER_Fan_Revolutions = wxNewId();
const long FanPanel::ID_PANEL38 = wxNewId();
const long FanPanel::ID_STATICTEXT_Fan_Num_Blades = wxNewId();
const long FanPanel::ID_SLIDER_Fan_Num_Blades = wxNewId();
const long FanPanel::ID_VALUECURVE_Fan_Num_Blades = wxNewId();
const long FanPanel::IDD_TEXTCTRL_Fan_Num_Blades = wxNewId();
const long FanPanel::ID_BITMAPBUTTON_SLIDER_Fan_Num_Blades = wxNewId();
const long FanPanel::ID_STATICTEXT_Fan_Blade_Width = wxNewId();
const long FanPanel::ID_SLIDER_Fan_Blade_Width = wxNewId();
const long FanPanel::ID_VALUECURVE_Fan_Blade_Width = wxNewId();
const long FanPanel::IDD_TEXTCTRL_Fan_Blade_Width = wxNewId();
const long FanPanel::ID_BITMAPBUTTON_SLIDER_Fan_Blade_Width = wxNewId();
const long FanPanel::ID_STATICTEXT_Fan_Blade_Angle = wxNewId();
const long FanPanel::ID_SLIDER_Fan_Blade_Angle = wxNewId();
const long FanPanel::ID_VALUECURVE_Fan_Blade_Angle = wxNewId();
const long FanPanel::IDD_TEXTCTRL_Fan_Blade_Angle = wxNewId();
const long FanPanel::ID_BITMAPBUTTON_SLIDER_Fan_Blade_Angle = wxNewId();
const long FanPanel::ID_STATICTEXT_Fan_Num_Elements = wxNewId();
const long FanPanel::ID_SLIDER_Fan_Num_Elements = wxNewId();
const long FanPanel::ID_VALUECURVE_Fan_Num_Elements = wxNewId();
const long FanPanel::IDD_TEXTCTRL_Fan_Num_Elements = wxNewId();
const long FanPanel::ID_BITMAPBUTTON_SLIDER_Fan_Num_Elements = wxNewId();
const long FanPanel::ID_STATICTEXT_Fan_Element_Width = wxNewId();
const long FanPanel::ID_SLIDER_Fan_Element_Width = wxNewId();
const long FanPanel::ID_VALUECURVE_Fan_Element_Width = wxNewId();
const long FanPanel::IDD_TEXTCTRL_Fan_Element_Width = wxNewId();
const long FanPanel::ID_BITMAPBUTTON_SLIDER_Fan_Element_Width = wxNewId();
const long FanPanel::ID_PANEL42 = wxNewId();
const long FanPanel::ID_STATICTEXT_Fan_Duration = wxNewId();
const long FanPanel::ID_SLIDER_Fan_Duration = wxNewId();
const long FanPanel::ID_VALUECURVE_Fan_Duration = wxNewId();
const long FanPanel::IDD_TEXTCTRL_Fan_Duration = wxNewId();
const long FanPanel::ID_BITMAPBUTTON_SLIDER_Fan_Duration = wxNewId();
const long FanPanel::ID_STATICTEXT_Fan_Accel = wxNewId();
const long FanPanel::ID_SLIDER_Fan_Accel = wxNewId();
const long FanPanel::ID_VALUECURVE_Fan_Accel = wxNewId();
const long FanPanel::IDD_TEXTCTRL_Fan_Accel = wxNewId();
const long FanPanel::ID_BITMAPBUTTON_SLIDER_Fan_Accel = wxNewId();
const long FanPanel::ID_CHECKBOX_Fan_Reverse = wxNewId();
const long FanPanel::ID_CHECKBOX_Fan_Blend_Edges = wxNewId();
const long FanPanel::ID_PANEL41 = wxNewId();
const long FanPanel::ID_NOTEBOOK_Fan = wxNewId();
//*)

BEGIN_EVENT_TABLE(FanPanel,wxPanel)
	//(*EventTable(FanPanel)
	//*)
END_EVENT_TABLE()

FanPanel::FanPanel(wxWindow* parent)
{
	//(*Initialize(FanPanel)
	BulkEditTextCtrl* TextCtrl_Fan_Accel;
	BulkEditTextCtrl* TextCtrl_Fan_Blade_Angle;
	BulkEditTextCtrl* TextCtrl_Fan_Blade_Width;
	BulkEditTextCtrl* TextCtrl_Fan_CenterX;
	BulkEditTextCtrl* TextCtrl_Fan_CenterY;
	BulkEditTextCtrl* TextCtrl_Fan_Duration;
	BulkEditTextCtrl* TextCtrl_Fan_Element_Width;
	BulkEditTextCtrl* TextCtrl_Fan_End_Radius;
	BulkEditTextCtrl* TextCtrl_Fan_Num_Blades;
	BulkEditTextCtrl* TextCtrl_Fan_Num_Elements;
	BulkEditTextCtrl* TextCtrl_Fan_Start_Angle;
	BulkEditTextCtrl* TextCtrl_Fan_Start_Radius;
	BulkEditTextCtrlF360* TextCtrl_Fan_Revolutions;
	wxFlexGridSizer* FlexGridSizer103;
	wxFlexGridSizer* FlexGridSizer109;
	wxFlexGridSizer* GridBagSizerFanBlades;
	wxFlexGridSizer* GridBagSizerFanOptions;
	wxFlexGridSizer* GridBagSizerFanPosition;
	wxPanel* Panel11;
	wxPanel* Panel12;
	wxPanel* Panel9;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer109 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer109->AddGrowableCol(0);
	Notebook1 = new wxNotebook(this, ID_NOTEBOOK_Fan, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK_Fan"));
	Panel9 = new wxPanel(Notebook1, ID_PANEL38, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL38"));
	GridBagSizerFanPosition = new wxFlexGridSizer(0, 5, 0, 0);
	GridBagSizerFanPosition->AddGrowableCol(1);
	StaticText133 = new wxStaticText(Panel9, ID_STATICTEXT_Fan_CenterX, _("Center X:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fan_CenterX"));
	GridBagSizerFanPosition->Add(StaticText133, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fan_CenterX = new BulkEditSlider(Panel9, ID_SLIDER_Fan_CenterX, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fan_CenterX"));
	GridBagSizerFanPosition->Add(Slider_Fan_CenterX, 1, wxALL|wxEXPAND, 5);
	ValueCurve_Fan_CenterX = new BulkEditValueCurveButton(Panel9, ID_VALUECURVE_Fan_CenterX, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Fan_CenterX"));
	GridBagSizerFanPosition->Add(ValueCurve_Fan_CenterX, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Fan_CenterX = new BulkEditTextCtrl(Panel9, IDD_TEXTCTRL_Fan_CenterX, _("50"), wxDefaultPosition, wxDLG_UNIT(Panel9,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Fan_CenterX"));
	TextCtrl_Fan_CenterX->SetMaxLength(3);
	GridBagSizerFanPosition->Add(TextCtrl_Fan_CenterX, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, wxDLG_UNIT(Panel9,wxSize(5,0)).GetWidth());
	BitmapButton_Fan_CenterX = new wxBitmapButton(Panel9, ID_BITMAPBUTTON_SLIDER_Fan_CenterX, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fan_CenterX"));
	BitmapButton_Fan_CenterX->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	GridBagSizerFanPosition->Add(BitmapButton_Fan_CenterX, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText135 = new wxStaticText(Panel9, ID_STATICTEXT_Fan_CenterY, _("Center Y:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fan_CenterY"));
	GridBagSizerFanPosition->Add(StaticText135, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fan_CenterY = new BulkEditSlider(Panel9, ID_SLIDER_Fan_CenterY, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fan_CenterY"));
	GridBagSizerFanPosition->Add(Slider_Fan_CenterY, 1, wxALL|wxEXPAND, 5);
	ValueCurve_Fan_CenterY = new BulkEditValueCurveButton(Panel9, ID_VALUECURVE_Fan_CenterY, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Fan_CenterY"));
	GridBagSizerFanPosition->Add(ValueCurve_Fan_CenterY, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Fan_CenterY = new BulkEditTextCtrl(Panel9, IDD_TEXTCTRL_Fan_CenterY, _("50"), wxDefaultPosition, wxDLG_UNIT(Panel9,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Fan_CenterY"));
	TextCtrl_Fan_CenterY->SetMaxLength(3);
	GridBagSizerFanPosition->Add(TextCtrl_Fan_CenterY, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Fan_CenterY = new wxBitmapButton(Panel9, ID_BITMAPBUTTON_SLIDER_Fan_CenterY, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fan_CenterY"));
	BitmapButton_Fan_CenterY->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	GridBagSizerFanPosition->Add(BitmapButton_Fan_CenterY, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText143 = new wxStaticText(Panel9, ID_STATICTEXT_Fan_Start_Radius, _("Radius1:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fan_Start_Radius"));
	GridBagSizerFanPosition->Add(StaticText143, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fan_Start_Radius = new BulkEditSlider(Panel9, ID_SLIDER_Fan_Start_Radius, 1, 0, 500, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fan_Start_Radius"));
	GridBagSizerFanPosition->Add(Slider_Fan_Start_Radius, 1, wxALL|wxEXPAND, 5);
	ValueCurve_Fan_Start_Radius = new BulkEditValueCurveButton(Panel9, ID_VALUECURVE_Fan_Start_Radius, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Fan_Start_Radius"));
	GridBagSizerFanPosition->Add(ValueCurve_Fan_Start_Radius, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Fan_Start_Radius = new BulkEditTextCtrl(Panel9, IDD_TEXTCTRL_Fan_Start_Radius, _("1"), wxDefaultPosition, wxDLG_UNIT(Panel9,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Fan_Start_Radius"));
	TextCtrl_Fan_Start_Radius->SetMaxLength(3);
	GridBagSizerFanPosition->Add(TextCtrl_Fan_Start_Radius, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Fan_Start_Radius = new wxBitmapButton(Panel9, ID_BITMAPBUTTON_SLIDER_Fan_Start_Radius, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fan_Start_Radius"));
	BitmapButton_Fan_Start_Radius->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	GridBagSizerFanPosition->Add(BitmapButton_Fan_Start_Radius, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText145 = new wxStaticText(Panel9, ID_STATICTEXT_Fan_Start_Angle, _("Angle:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fan_Start_Angle"));
	GridBagSizerFanPosition->Add(StaticText145, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fan_Start_Angle = new BulkEditSlider(Panel9, ID_SLIDER_Fan_Start_Angle, 0, 0, 360, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fan_Start_Angle"));
	GridBagSizerFanPosition->Add(Slider_Fan_Start_Angle, 1, wxALL|wxEXPAND, 5);
	ValueCurve_Fan_Start_Angle = new BulkEditValueCurveButton(Panel9, ID_VALUECURVE_Fan_Start_Angle, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Fan_Start_Angle"));
	GridBagSizerFanPosition->Add(ValueCurve_Fan_Start_Angle, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Fan_Start_Angle = new BulkEditTextCtrl(Panel9, IDD_TEXTCTRL_Fan_Start_Angle, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel9,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Fan_Start_Angle"));
	TextCtrl_Fan_Start_Angle->SetMaxLength(3);
	GridBagSizerFanPosition->Add(TextCtrl_Fan_Start_Angle, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Fan_Start_Angle = new wxBitmapButton(Panel9, ID_BITMAPBUTTON_SLIDER_Fan_Start_Angle, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fan_Start_Angle"));
	BitmapButton_Fan_Start_Angle->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	GridBagSizerFanPosition->Add(BitmapButton_Fan_Start_Angle, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText144 = new wxStaticText(Panel9, ID_STATICTEXT_Fan_End_Radius, _("Radius2:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fan_End_Radius"));
	GridBagSizerFanPosition->Add(StaticText144, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fan_End_Radius = new BulkEditSlider(Panel9, ID_SLIDER_Fan_End_Radius, 10, 0, 500, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fan_End_Radius"));
	GridBagSizerFanPosition->Add(Slider_Fan_End_Radius, 1, wxALL|wxEXPAND, 5);
	ValueCurve_Fan_End_Radius = new BulkEditValueCurveButton(Panel9, ID_VALUECURVE_Fan_End_Radius, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Fan_End_Radius"));
	GridBagSizerFanPosition->Add(ValueCurve_Fan_End_Radius, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Fan_End_Radius = new BulkEditTextCtrl(Panel9, IDD_TEXTCTRL_Fan_End_Radius, _("10"), wxDefaultPosition, wxDLG_UNIT(Panel9,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Fan_End_Radius"));
	TextCtrl_Fan_End_Radius->SetMaxLength(3);
	GridBagSizerFanPosition->Add(TextCtrl_Fan_End_Radius, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Fan_End_Radius = new wxBitmapButton(Panel9, ID_BITMAPBUTTON_SLIDER_Fan_End_Radius, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fan_End_Radius"));
	BitmapButton_Fan_End_Radius->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	GridBagSizerFanPosition->Add(BitmapButton_Fan_End_Radius, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText148 = new wxStaticText(Panel9, ID_STATICTEXT_Fan_Revolutions, _("Rev\'s:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fan_Revolutions"));
	GridBagSizerFanPosition->Add(StaticText148, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fan_Revolutions = new BulkEditSliderF360(Panel9, ID_SLIDER_Fan_Revolutions, 720, 0, 3600, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fan_Revolutions"));
	GridBagSizerFanPosition->Add(Slider_Fan_Revolutions, 1, wxALL|wxEXPAND, 5);
	ValueCurve_Fan_Revolutions = new BulkEditValueCurveButton(Panel9, ID_VALUECURVE_Fan_Revolutions, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Fan_Revolutions"));
	GridBagSizerFanPosition->Add(ValueCurve_Fan_Revolutions, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Fan_Revolutions = new BulkEditTextCtrlF360(Panel9, IDD_TEXTCTRL_Fan_Revolutions, _("2.0"), wxDefaultPosition, wxDLG_UNIT(Panel9,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Fan_Revolutions"));
	TextCtrl_Fan_Revolutions->SetMaxLength(4);
	GridBagSizerFanPosition->Add(TextCtrl_Fan_Revolutions, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Fan_Revolutions = new wxBitmapButton(Panel9, ID_BITMAPBUTTON_SLIDER_Fan_Revolutions, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fan_Revolutions"));
	BitmapButton_Fan_Revolutions->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	GridBagSizerFanPosition->Add(BitmapButton_Fan_Revolutions, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel9->SetSizer(GridBagSizerFanPosition);
	GridBagSizerFanPosition->Fit(Panel9);
	GridBagSizerFanPosition->SetSizeHints(Panel9);
	Panel12 = new wxPanel(Notebook1, ID_PANEL42, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL42"));
	GridBagSizerFanBlades = new wxFlexGridSizer(0, 5, 0, 0);
	GridBagSizerFanBlades->AddGrowableCol(1);
	StaticText151 = new wxStaticText(Panel12, ID_STATICTEXT_Fan_Num_Blades, _("# Blades:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fan_Num_Blades"));
	GridBagSizerFanBlades->Add(StaticText151, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fan_Num_Blades = new BulkEditSlider(Panel12, ID_SLIDER_Fan_Num_Blades, 3, 1, 16, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fan_Num_Blades"));
	GridBagSizerFanBlades->Add(Slider_Fan_Num_Blades, 1, wxALL|wxEXPAND, 5);
	ValueCurve_Fan_Num_Blades = new BulkEditValueCurveButton(Panel12, ID_VALUECURVE_Fan_Num_Blades, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Fan_Num_Blades"));
	GridBagSizerFanBlades->Add(ValueCurve_Fan_Num_Blades, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Fan_Num_Blades = new BulkEditTextCtrl(Panel12, IDD_TEXTCTRL_Fan_Num_Blades, _("3"), wxDefaultPosition, wxDLG_UNIT(Panel12,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Fan_Num_Blades"));
	TextCtrl_Fan_Num_Blades->SetMaxLength(3);
	GridBagSizerFanBlades->Add(TextCtrl_Fan_Num_Blades, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Fan_Num_Blades = new wxBitmapButton(Panel12, ID_BITMAPBUTTON_SLIDER_Fan_Num_Blades, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fan_Num_Blades"));
	BitmapButton_Fan_Num_Blades->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	GridBagSizerFanBlades->Add(BitmapButton_Fan_Num_Blades, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText152 = new wxStaticText(Panel12, ID_STATICTEXT_Fan_Blade_Width, _("Width:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fan_Blade_Width"));
	GridBagSizerFanBlades->Add(StaticText152, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fan_Blade_Width = new BulkEditSlider(Panel12, ID_SLIDER_Fan_Blade_Width, 50, 5, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fan_Blade_Width"));
	GridBagSizerFanBlades->Add(Slider_Fan_Blade_Width, 1, wxALL|wxEXPAND, 5);
	ValueCurve_Fan_Blade_Width = new BulkEditValueCurveButton(Panel12, ID_VALUECURVE_Fan_Blade_Width, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Fan_Blade_Width"));
	GridBagSizerFanBlades->Add(ValueCurve_Fan_Blade_Width, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Fan_Blade_Width = new BulkEditTextCtrl(Panel12, IDD_TEXTCTRL_Fan_Blade_Width, _("50"), wxDefaultPosition, wxDLG_UNIT(Panel12,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Fan_Blade_Width"));
	TextCtrl_Fan_Blade_Width->SetMaxLength(3);
	GridBagSizerFanBlades->Add(TextCtrl_Fan_Blade_Width, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Fan_Blade_Width = new wxBitmapButton(Panel12, ID_BITMAPBUTTON_SLIDER_Fan_Blade_Width, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fan_Blade_Width"));
	BitmapButton_Fan_Blade_Width->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	GridBagSizerFanBlades->Add(BitmapButton_Fan_Blade_Width, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText153 = new wxStaticText(Panel12, ID_STATICTEXT_Fan_Blade_Angle, _("Angle:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fan_Blade_Angle"));
	GridBagSizerFanBlades->Add(StaticText153, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fan_Blade_Angle = new BulkEditSlider(Panel12, ID_SLIDER_Fan_Blade_Angle, 90, -360, 360, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fan_Blade_Angle"));
	GridBagSizerFanBlades->Add(Slider_Fan_Blade_Angle, 1, wxALL|wxEXPAND, 5);
	ValueCurve_Fan_Blade_Angle = new BulkEditValueCurveButton(Panel12, ID_VALUECURVE_Fan_Blade_Angle, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Fan_Blade_Angle"));
	GridBagSizerFanBlades->Add(ValueCurve_Fan_Blade_Angle, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Fan_Blade_Angle = new BulkEditTextCtrl(Panel12, IDD_TEXTCTRL_Fan_Blade_Angle, _("90"), wxDefaultPosition, wxDLG_UNIT(Panel12,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Fan_Blade_Angle"));
	TextCtrl_Fan_Blade_Angle->SetMaxLength(4);
	GridBagSizerFanBlades->Add(TextCtrl_Fan_Blade_Angle, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Fan_Blade_Angle = new wxBitmapButton(Panel12, ID_BITMAPBUTTON_SLIDER_Fan_Blade_Angle, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fan_Blade_Angle"));
	BitmapButton_Fan_Blade_Angle->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	GridBagSizerFanBlades->Add(BitmapButton_Fan_Blade_Angle, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText146 = new wxStaticText(Panel12, ID_STATICTEXT_Fan_Num_Elements, _("# Elements:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fan_Num_Elements"));
	GridBagSizerFanBlades->Add(StaticText146, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fan_Num_Elements = new BulkEditSlider(Panel12, ID_SLIDER_Fan_Num_Elements, 1, 1, 4, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fan_Num_Elements"));
	GridBagSizerFanBlades->Add(Slider_Fan_Num_Elements, 1, wxALL|wxEXPAND, 5);
	ValueCurve_Fan_Num_Elements = new BulkEditValueCurveButton(Panel12, ID_VALUECURVE_Fan_Num_Elements, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Fan_Num_Elements"));
	GridBagSizerFanBlades->Add(ValueCurve_Fan_Num_Elements, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Fan_Num_Elements = new BulkEditTextCtrl(Panel12, IDD_TEXTCTRL_Fan_Num_Elements, _("1"), wxDefaultPosition, wxDLG_UNIT(Panel12,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Fan_Num_Elements"));
	TextCtrl_Fan_Num_Elements->SetMaxLength(3);
	GridBagSizerFanBlades->Add(TextCtrl_Fan_Num_Elements, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Fan_Num_Elements = new wxBitmapButton(Panel12, ID_BITMAPBUTTON_SLIDER_Fan_Num_Elements, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fan_Num_Elements"));
	BitmapButton_Fan_Num_Elements->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	GridBagSizerFanBlades->Add(BitmapButton_Fan_Num_Elements, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText147 = new wxStaticText(Panel12, ID_STATICTEXT_Fan_Element_Width, _("Width:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fan_Element_Width"));
	GridBagSizerFanBlades->Add(StaticText147, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fan_Element_Width = new BulkEditSlider(Panel12, ID_SLIDER_Fan_Element_Width, 100, 5, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fan_Element_Width"));
	GridBagSizerFanBlades->Add(Slider_Fan_Element_Width, 1, wxALL|wxEXPAND, 5);
	ValueCurve_Fan_Element_Width = new BulkEditValueCurveButton(Panel12, ID_VALUECURVE_Fan_Element_Width, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Fan_Element_Width"));
	GridBagSizerFanBlades->Add(ValueCurve_Fan_Element_Width, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Fan_Element_Width = new BulkEditTextCtrl(Panel12, IDD_TEXTCTRL_Fan_Element_Width, _("100"), wxDefaultPosition, wxDLG_UNIT(Panel12,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Fan_Element_Width"));
	TextCtrl_Fan_Element_Width->SetMaxLength(3);
	GridBagSizerFanBlades->Add(TextCtrl_Fan_Element_Width, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Fan_Element_Width = new wxBitmapButton(Panel12, ID_BITMAPBUTTON_SLIDER_Fan_Element_Width, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fan_Element_Width"));
	BitmapButton_Fan_Element_Width->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	GridBagSizerFanBlades->Add(BitmapButton_Fan_Element_Width, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel12->SetSizer(GridBagSizerFanBlades);
	GridBagSizerFanBlades->Fit(Panel12);
	GridBagSizerFanBlades->SetSizeHints(Panel12);
	Panel11 = new wxPanel(Notebook1, ID_PANEL41, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL41"));
	GridBagSizerFanOptions = new wxFlexGridSizer(0, 5, 0, 0);
	GridBagSizerFanOptions->AddGrowableCol(1);
	StaticText149 = new wxStaticText(Panel11, ID_STATICTEXT_Fan_Duration, _("Duration:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fan_Duration"));
	GridBagSizerFanOptions->Add(StaticText149, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fan_Duration = new BulkEditSlider(Panel11, ID_SLIDER_Fan_Duration, 80, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fan_Duration"));
	GridBagSizerFanOptions->Add(Slider_Fan_Duration, 1, wxALL|wxEXPAND, 5);
	ValueCurve_Fan_Duration = new BulkEditValueCurveButton(Panel11, ID_VALUECURVE_Fan_Duration, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Fan_Duration"));
	GridBagSizerFanOptions->Add(ValueCurve_Fan_Duration, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Fan_Duration = new BulkEditTextCtrl(Panel11, IDD_TEXTCTRL_Fan_Duration, _("80"), wxDefaultPosition, wxDLG_UNIT(Panel11,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Fan_Duration"));
	TextCtrl_Fan_Duration->SetMaxLength(3);
	GridBagSizerFanOptions->Add(TextCtrl_Fan_Duration, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Fan_Duration = new wxBitmapButton(Panel11, ID_BITMAPBUTTON_SLIDER_Fan_Duration, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fan_Duration"));
	BitmapButton_Fan_Duration->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	GridBagSizerFanOptions->Add(BitmapButton_Fan_Duration, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText150 = new wxStaticText(Panel11, ID_STATICTEXT_Fan_Accel, _("Acceleration:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fan_Accel"));
	GridBagSizerFanOptions->Add(StaticText150, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fan_Accel = new BulkEditSlider(Panel11, ID_SLIDER_Fan_Accel, 0, -10, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fan_Accel"));
	GridBagSizerFanOptions->Add(Slider_Fan_Accel, 1, wxALL|wxEXPAND, 5);
	ValueCurve_Fan_Accel = new BulkEditValueCurveButton(Panel11, ID_VALUECURVE_Fan_Accel, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Fan_Accel"));
	GridBagSizerFanOptions->Add(ValueCurve_Fan_Accel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Fan_Accel = new BulkEditTextCtrl(Panel11, IDD_TEXTCTRL_Fan_Accel, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel11,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Fan_Accel"));
	TextCtrl_Fan_Accel->SetMaxLength(3);
	GridBagSizerFanOptions->Add(TextCtrl_Fan_Accel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Fan_Accel = new wxBitmapButton(Panel11, ID_BITMAPBUTTON_SLIDER_Fan_Accel, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fan_Accel"));
	BitmapButton_Fan_Accel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	GridBagSizerFanOptions->Add(BitmapButton_Fan_Accel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	GridBagSizerFanOptions->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer103 = new wxFlexGridSizer(0, 1, 0, 0);
	CheckBox_Fan_Reverse = new BulkEditCheckBox(Panel11, ID_CHECKBOX_Fan_Reverse, _("Reverse:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT, wxDefaultValidator, _T("ID_CHECKBOX_Fan_Reverse"));
	CheckBox_Fan_Reverse->SetValue(false);
	FlexGridSizer103->Add(CheckBox_Fan_Reverse, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Fan_Blend_Edges = new BulkEditCheckBox(Panel11, ID_CHECKBOX_Fan_Blend_Edges, _("Blend Edges:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT, wxDefaultValidator, _T("ID_CHECKBOX_Fan_Blend_Edges"));
	CheckBox_Fan_Blend_Edges->SetValue(true);
	FlexGridSizer103->Add(CheckBox_Fan_Blend_Edges, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	GridBagSizerFanOptions->Add(FlexGridSizer103, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel11->SetSizer(GridBagSizerFanOptions);
	GridBagSizerFanOptions->Fit(Panel11);
	GridBagSizerFanOptions->SetSizeHints(Panel11);
	Notebook1->AddPage(Panel9, _("Position"), false);
	Notebook1->AddPage(Panel12, _("Blades"), false);
	Notebook1->AddPage(Panel11, _("Options"), false);
	FlexGridSizer109->Add(Notebook1, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer109);
	FlexGridSizer109->Fit(this);
	FlexGridSizer109->SetSizeHints(this);

	Connect(ID_VALUECURVE_Fan_CenterX,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FanPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fan_CenterX,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FanPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Fan_CenterY,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FanPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fan_CenterY,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FanPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Fan_Start_Radius,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FanPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fan_Start_Radius,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FanPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Fan_Start_Angle,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FanPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fan_Start_Angle,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FanPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Fan_End_Radius,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FanPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fan_End_Radius,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FanPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Fan_Revolutions,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FanPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fan_Revolutions,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FanPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Fan_Num_Blades,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FanPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fan_Num_Blades,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FanPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Fan_Blade_Width,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FanPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fan_Blade_Width,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FanPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Fan_Blade_Angle,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FanPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fan_Blade_Angle,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FanPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Fan_Num_Elements,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FanPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fan_Num_Elements,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FanPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Fan_Element_Width,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FanPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fan_Element_Width,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FanPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Fan_Duration,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FanPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fan_Duration,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FanPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Fan_Accel,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FanPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fan_Accel,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FanPanel::OnLockButtonClick);
	//*)
    SetName("ID_PANEL_FAN");

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&FanPanel::OnVCChanged, nullptr, this);

    ValueCurve_Fan_CenterX->GetValue()->SetLimits(FAN_CENTREX_MIN, FAN_CENTREX_MAX);
    ValueCurve_Fan_CenterY->GetValue()->SetLimits(FAN_CENTREY_MIN, FAN_CENTREY_MAX);
    ValueCurve_Fan_Start_Radius->GetValue()->SetLimits(FAN_STARTRADIUS_MIN, FAN_STARTRADIUS_MAX);
    ValueCurve_Fan_Start_Angle->GetValue()->SetLimits(FAN_STARTANGLE_MIN, FAN_STARTANGLE_MAX);
    ValueCurve_Fan_End_Radius->GetValue()->SetLimits(FAN_ENDRADIUS_MIN, FAN_ENDRADIUS_MAX);
    ValueCurve_Fan_Revolutions->GetValue()->SetLimits(FAN_REVOLUTIONS_MIN, FAN_REVOLUTIONS_MAX);
    ValueCurve_Fan_Num_Blades->GetValue()->SetLimits(FAN_BLADES_MIN, FAN_BLADES_MAX);
    ValueCurve_Fan_Blade_Width->GetValue()->SetLimits(FAN_BLADEWIDTH_MIN, FAN_BLADEWIDTH_MAX);
    ValueCurve_Fan_Blade_Angle->GetValue()->SetLimits(FAN_BLADEANGLE_MIN, FAN_BLADEANGLE_MAX);
    ValueCurve_Fan_Num_Elements->GetValue()->SetLimits(FAN_NUMELEMENTS_MIN, FAN_NUMELEMENTS_MAX);
    ValueCurve_Fan_Element_Width->GetValue()->SetLimits(FAN_ELEMENTWIDTH_MIN, FAN_ELEMENTWIDTH_MAX);
    ValueCurve_Fan_Duration->GetValue()->SetLimits(FAN_DURATION_MIN, FAN_DURATION_MAX);
    ValueCurve_Fan_Accel->GetValue()->SetLimits(FAN_ACCEL_MIN, FAN_ACCEL_MAX);
}

FanPanel::~FanPanel()
{
	//(*Destroy(FanPanel)
	//*)
}

PANEL_EVENT_HANDLERS(FanPanel)
