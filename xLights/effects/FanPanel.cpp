/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "FanPanel.h"
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
const wxWindowID FanPanel::ID_STATICTEXT_Fan_CenterX = wxNewId();
const wxWindowID FanPanel::ID_SLIDER_Fan_CenterX = wxNewId();
const wxWindowID FanPanel::ID_VALUECURVE_Fan_CenterX = wxNewId();
const wxWindowID FanPanel::IDD_TEXTCTRL_Fan_CenterX = wxNewId();
const wxWindowID FanPanel::ID_BITMAPBUTTON_SLIDER_Fan_CenterX = wxNewId();
const wxWindowID FanPanel::ID_STATICTEXT_Fan_CenterY = wxNewId();
const wxWindowID FanPanel::ID_SLIDER_Fan_CenterY = wxNewId();
const wxWindowID FanPanel::ID_VALUECURVE_Fan_CenterY = wxNewId();
const wxWindowID FanPanel::IDD_TEXTCTRL_Fan_CenterY = wxNewId();
const wxWindowID FanPanel::ID_BITMAPBUTTON_SLIDER_Fan_CenterY = wxNewId();
const wxWindowID FanPanel::ID_STATICTEXT_Fan_Start_Radius = wxNewId();
const wxWindowID FanPanel::ID_SLIDER_Fan_Start_Radius = wxNewId();
const wxWindowID FanPanel::ID_VALUECURVE_Fan_Start_Radius = wxNewId();
const wxWindowID FanPanel::IDD_TEXTCTRL_Fan_Start_Radius = wxNewId();
const wxWindowID FanPanel::ID_BITMAPBUTTON_SLIDER_Fan_Start_Radius = wxNewId();
const wxWindowID FanPanel::ID_STATICTEXT_Fan_Start_Angle = wxNewId();
const wxWindowID FanPanel::ID_SLIDER_Fan_Start_Angle = wxNewId();
const wxWindowID FanPanel::ID_VALUECURVE_Fan_Start_Angle = wxNewId();
const wxWindowID FanPanel::IDD_TEXTCTRL_Fan_Start_Angle = wxNewId();
const wxWindowID FanPanel::ID_BITMAPBUTTON_SLIDER_Fan_Start_Angle = wxNewId();
const wxWindowID FanPanel::ID_STATICTEXT_Fan_End_Radius = wxNewId();
const wxWindowID FanPanel::ID_SLIDER_Fan_End_Radius = wxNewId();
const wxWindowID FanPanel::ID_VALUECURVE_Fan_End_Radius = wxNewId();
const wxWindowID FanPanel::IDD_TEXTCTRL_Fan_End_Radius = wxNewId();
const wxWindowID FanPanel::ID_BITMAPBUTTON_SLIDER_Fan_End_Radius = wxNewId();
const wxWindowID FanPanel::ID_STATICTEXT_Fan_Revolutions = wxNewId();
const wxWindowID FanPanel::ID_SLIDER_Fan_Revolutions = wxNewId();
const wxWindowID FanPanel::ID_VALUECURVE_Fan_Revolutions = wxNewId();
const wxWindowID FanPanel::IDD_TEXTCTRL_Fan_Revolutions = wxNewId();
const wxWindowID FanPanel::ID_BITMAPBUTTON_SLIDER_Fan_Revolutions = wxNewId();
const wxWindowID FanPanel::ID_PANEL38 = wxNewId();
const wxWindowID FanPanel::ID_STATICTEXT_Fan_Num_Blades = wxNewId();
const wxWindowID FanPanel::ID_SLIDER_Fan_Num_Blades = wxNewId();
const wxWindowID FanPanel::ID_VALUECURVE_Fan_Num_Blades = wxNewId();
const wxWindowID FanPanel::IDD_TEXTCTRL_Fan_Num_Blades = wxNewId();
const wxWindowID FanPanel::ID_BITMAPBUTTON_SLIDER_Fan_Num_Blades = wxNewId();
const wxWindowID FanPanel::ID_STATICTEXT_Fan_Blade_Width = wxNewId();
const wxWindowID FanPanel::ID_SLIDER_Fan_Blade_Width = wxNewId();
const wxWindowID FanPanel::ID_VALUECURVE_Fan_Blade_Width = wxNewId();
const wxWindowID FanPanel::IDD_TEXTCTRL_Fan_Blade_Width = wxNewId();
const wxWindowID FanPanel::ID_BITMAPBUTTON_SLIDER_Fan_Blade_Width = wxNewId();
const wxWindowID FanPanel::ID_STATICTEXT_Fan_Blade_Angle = wxNewId();
const wxWindowID FanPanel::ID_SLIDER_Fan_Blade_Angle = wxNewId();
const wxWindowID FanPanel::ID_VALUECURVE_Fan_Blade_Angle = wxNewId();
const wxWindowID FanPanel::IDD_TEXTCTRL_Fan_Blade_Angle = wxNewId();
const wxWindowID FanPanel::ID_BITMAPBUTTON_SLIDER_Fan_Blade_Angle = wxNewId();
const wxWindowID FanPanel::ID_STATICTEXT_Fan_Num_Elements = wxNewId();
const wxWindowID FanPanel::ID_SLIDER_Fan_Num_Elements = wxNewId();
const wxWindowID FanPanel::ID_VALUECURVE_Fan_Num_Elements = wxNewId();
const wxWindowID FanPanel::IDD_TEXTCTRL_Fan_Num_Elements = wxNewId();
const wxWindowID FanPanel::ID_BITMAPBUTTON_SLIDER_Fan_Num_Elements = wxNewId();
const wxWindowID FanPanel::ID_STATICTEXT_Fan_Element_Width = wxNewId();
const wxWindowID FanPanel::ID_SLIDER_Fan_Element_Width = wxNewId();
const wxWindowID FanPanel::ID_VALUECURVE_Fan_Element_Width = wxNewId();
const wxWindowID FanPanel::IDD_TEXTCTRL_Fan_Element_Width = wxNewId();
const wxWindowID FanPanel::ID_BITMAPBUTTON_SLIDER_Fan_Element_Width = wxNewId();
const wxWindowID FanPanel::ID_PANEL42 = wxNewId();
const wxWindowID FanPanel::ID_STATICTEXT_Fan_Duration = wxNewId();
const wxWindowID FanPanel::ID_SLIDER_Fan_Duration = wxNewId();
const wxWindowID FanPanel::ID_VALUECURVE_Fan_Duration = wxNewId();
const wxWindowID FanPanel::IDD_TEXTCTRL_Fan_Duration = wxNewId();
const wxWindowID FanPanel::ID_BITMAPBUTTON_SLIDER_Fan_Duration = wxNewId();
const wxWindowID FanPanel::ID_STATICTEXT_Fan_Accel = wxNewId();
const wxWindowID FanPanel::ID_SLIDER_Fan_Accel = wxNewId();
const wxWindowID FanPanel::ID_VALUECURVE_Fan_Accel = wxNewId();
const wxWindowID FanPanel::IDD_TEXTCTRL_Fan_Accel = wxNewId();
const wxWindowID FanPanel::ID_BITMAPBUTTON_SLIDER_Fan_Accel = wxNewId();
const wxWindowID FanPanel::ID_CHECKBOX_Fan_Reverse = wxNewId();
const wxWindowID FanPanel::ID_CHECKBOX_Fan_Blend_Edges = wxNewId();
const wxWindowID FanPanel::ID_CHECKBOX_Fan_Scale = wxNewId();
const wxWindowID FanPanel::ID_PANEL41 = wxNewId();
const wxWindowID FanPanel::ID_NOTEBOOK_Fan = wxNewId();
//*)

BEGIN_EVENT_TABLE(FanPanel,wxPanel)
	//(*EventTable(FanPanel)
	//*)
END_EVENT_TABLE()

FanPanel::FanPanel(wxWindow* parent) : xlEffectPanel(parent)
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
	GridBagSizerFanPosition->Add(Slider_Fan_CenterX, 1, wxALL|wxEXPAND, 1);
	ValueCurve_Fan_CenterX = new BulkEditValueCurveButton(Panel9, ID_VALUECURVE_Fan_CenterX, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Fan_CenterX"));
	GridBagSizerFanPosition->Add(ValueCurve_Fan_CenterX, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_Fan_CenterX = new BulkEditTextCtrl(Panel9, IDD_TEXTCTRL_Fan_CenterX, _T("50"), wxDefaultPosition, wxDLG_UNIT(Panel9,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Fan_CenterX"));
	TextCtrl_Fan_CenterX->SetMaxLength(3);
	GridBagSizerFanPosition->Add(TextCtrl_Fan_CenterX, 1, wxALL|wxEXPAND, wxDLG_UNIT(Panel9,wxSize(1,0)).GetWidth());
	BitmapButton_Fan_CenterX = new xlLockButton(Panel9, ID_BITMAPBUTTON_SLIDER_Fan_CenterX, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fan_CenterX"));
	BitmapButton_Fan_CenterX->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	GridBagSizerFanPosition->Add(BitmapButton_Fan_CenterX, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText135 = new wxStaticText(Panel9, ID_STATICTEXT_Fan_CenterY, _("Center Y:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fan_CenterY"));
	GridBagSizerFanPosition->Add(StaticText135, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fan_CenterY = new BulkEditSlider(Panel9, ID_SLIDER_Fan_CenterY, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fan_CenterY"));
	GridBagSizerFanPosition->Add(Slider_Fan_CenterY, 1, wxALL|wxEXPAND, 1);
	ValueCurve_Fan_CenterY = new BulkEditValueCurveButton(Panel9, ID_VALUECURVE_Fan_CenterY, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Fan_CenterY"));
	GridBagSizerFanPosition->Add(ValueCurve_Fan_CenterY, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_Fan_CenterY = new BulkEditTextCtrl(Panel9, IDD_TEXTCTRL_Fan_CenterY, _T("50"), wxDefaultPosition, wxDLG_UNIT(Panel9,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Fan_CenterY"));
	TextCtrl_Fan_CenterY->SetMaxLength(3);
	GridBagSizerFanPosition->Add(TextCtrl_Fan_CenterY, 1, wxALL|wxEXPAND, 1);
	BitmapButton_Fan_CenterY = new xlLockButton(Panel9, ID_BITMAPBUTTON_SLIDER_Fan_CenterY, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fan_CenterY"));
	BitmapButton_Fan_CenterY->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	GridBagSizerFanPosition->Add(BitmapButton_Fan_CenterY, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText143 = new wxStaticText(Panel9, ID_STATICTEXT_Fan_Start_Radius, _("Radius1:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fan_Start_Radius"));
	GridBagSizerFanPosition->Add(StaticText143, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fan_Start_Radius = new BulkEditSlider(Panel9, ID_SLIDER_Fan_Start_Radius, 1, 0, 2500, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fan_Start_Radius"));
	GridBagSizerFanPosition->Add(Slider_Fan_Start_Radius, 1, wxALL|wxEXPAND, 1);
	ValueCurve_Fan_Start_Radius = new BulkEditValueCurveButton(Panel9, ID_VALUECURVE_Fan_Start_Radius, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Fan_Start_Radius"));
	GridBagSizerFanPosition->Add(ValueCurve_Fan_Start_Radius, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_Fan_Start_Radius = new BulkEditTextCtrl(Panel9, IDD_TEXTCTRL_Fan_Start_Radius, _T("1"), wxDefaultPosition, wxDLG_UNIT(Panel9,wxSize(30,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Fan_Start_Radius"));
	TextCtrl_Fan_Start_Radius->SetMaxLength(4);
	GridBagSizerFanPosition->Add(TextCtrl_Fan_Start_Radius, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	BitmapButton_Fan_Start_Radius = new xlLockButton(Panel9, ID_BITMAPBUTTON_SLIDER_Fan_Start_Radius, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fan_Start_Radius"));
	BitmapButton_Fan_Start_Radius->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	GridBagSizerFanPosition->Add(BitmapButton_Fan_Start_Radius, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText145 = new wxStaticText(Panel9, ID_STATICTEXT_Fan_Start_Angle, _("Angle:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fan_Start_Angle"));
	GridBagSizerFanPosition->Add(StaticText145, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fan_Start_Angle = new BulkEditSlider(Panel9, ID_SLIDER_Fan_Start_Angle, 0, 0, 360, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fan_Start_Angle"));
	GridBagSizerFanPosition->Add(Slider_Fan_Start_Angle, 1, wxALL|wxEXPAND, 1);
	ValueCurve_Fan_Start_Angle = new BulkEditValueCurveButton(Panel9, ID_VALUECURVE_Fan_Start_Angle, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Fan_Start_Angle"));
	GridBagSizerFanPosition->Add(ValueCurve_Fan_Start_Angle, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_Fan_Start_Angle = new BulkEditTextCtrl(Panel9, IDD_TEXTCTRL_Fan_Start_Angle, _T("0"), wxDefaultPosition, wxDLG_UNIT(Panel9,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Fan_Start_Angle"));
	TextCtrl_Fan_Start_Angle->SetMaxLength(3);
	GridBagSizerFanPosition->Add(TextCtrl_Fan_Start_Angle, 1, wxALL|wxEXPAND, 1);
	BitmapButton_Fan_Start_Angle = new xlLockButton(Panel9, ID_BITMAPBUTTON_SLIDER_Fan_Start_Angle, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fan_Start_Angle"));
	BitmapButton_Fan_Start_Angle->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	GridBagSizerFanPosition->Add(BitmapButton_Fan_Start_Angle, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText144 = new wxStaticText(Panel9, ID_STATICTEXT_Fan_End_Radius, _("Radius2:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fan_End_Radius"));
	GridBagSizerFanPosition->Add(StaticText144, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fan_End_Radius = new BulkEditSlider(Panel9, ID_SLIDER_Fan_End_Radius, 10, 0, 2500, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fan_End_Radius"));
	GridBagSizerFanPosition->Add(Slider_Fan_End_Radius, 1, wxALL|wxEXPAND, 1);
	ValueCurve_Fan_End_Radius = new BulkEditValueCurveButton(Panel9, ID_VALUECURVE_Fan_End_Radius, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Fan_End_Radius"));
	GridBagSizerFanPosition->Add(ValueCurve_Fan_End_Radius, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_Fan_End_Radius = new BulkEditTextCtrl(Panel9, IDD_TEXTCTRL_Fan_End_Radius, _T("10"), wxDefaultPosition, wxDLG_UNIT(Panel9,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Fan_End_Radius"));
	TextCtrl_Fan_End_Radius->SetMaxLength(4);
	GridBagSizerFanPosition->Add(TextCtrl_Fan_End_Radius, 1, wxALL|wxEXPAND, 1);
	BitmapButton_Fan_End_Radius = new xlLockButton(Panel9, ID_BITMAPBUTTON_SLIDER_Fan_End_Radius, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fan_End_Radius"));
	BitmapButton_Fan_End_Radius->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	GridBagSizerFanPosition->Add(BitmapButton_Fan_End_Radius, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText148 = new wxStaticText(Panel9, ID_STATICTEXT_Fan_Revolutions, _("Rev\'s:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fan_Revolutions"));
	GridBagSizerFanPosition->Add(StaticText148, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fan_Revolutions = new BulkEditSliderF360(Panel9, ID_SLIDER_Fan_Revolutions, 720, 0, 3600, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fan_Revolutions"));
	GridBagSizerFanPosition->Add(Slider_Fan_Revolutions, 1, wxALL|wxEXPAND, 1);
	ValueCurve_Fan_Revolutions = new BulkEditValueCurveButton(Panel9, ID_VALUECURVE_Fan_Revolutions, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Fan_Revolutions"));
	GridBagSizerFanPosition->Add(ValueCurve_Fan_Revolutions, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_Fan_Revolutions = new BulkEditTextCtrlF360(Panel9, IDD_TEXTCTRL_Fan_Revolutions, _T("2.0"), wxDefaultPosition, wxDLG_UNIT(Panel9,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Fan_Revolutions"));
	TextCtrl_Fan_Revolutions->SetMaxLength(4);
	GridBagSizerFanPosition->Add(TextCtrl_Fan_Revolutions, 1, wxALL|wxEXPAND, 1);
	BitmapButton_Fan_Revolutions = new xlLockButton(Panel9, ID_BITMAPBUTTON_SLIDER_Fan_Revolutions, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fan_Revolutions"));
	BitmapButton_Fan_Revolutions->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	GridBagSizerFanPosition->Add(BitmapButton_Fan_Revolutions, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel9->SetSizer(GridBagSizerFanPosition);
	Panel12 = new wxPanel(Notebook1, ID_PANEL42, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL42"));
	GridBagSizerFanBlades = new wxFlexGridSizer(0, 5, 0, 0);
	GridBagSizerFanBlades->AddGrowableCol(1);
	StaticText151 = new wxStaticText(Panel12, ID_STATICTEXT_Fan_Num_Blades, _("# Blades:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fan_Num_Blades"));
	GridBagSizerFanBlades->Add(StaticText151, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fan_Num_Blades = new BulkEditSlider(Panel12, ID_SLIDER_Fan_Num_Blades, 3, 1, 16, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fan_Num_Blades"));
	GridBagSizerFanBlades->Add(Slider_Fan_Num_Blades, 1, wxALL|wxEXPAND, 1);
	ValueCurve_Fan_Num_Blades = new BulkEditValueCurveButton(Panel12, ID_VALUECURVE_Fan_Num_Blades, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Fan_Num_Blades"));
	GridBagSizerFanBlades->Add(ValueCurve_Fan_Num_Blades, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_Fan_Num_Blades = new BulkEditTextCtrl(Panel12, IDD_TEXTCTRL_Fan_Num_Blades, _T("3"), wxDefaultPosition, wxDLG_UNIT(Panel12,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Fan_Num_Blades"));
	TextCtrl_Fan_Num_Blades->SetMaxLength(3);
	GridBagSizerFanBlades->Add(TextCtrl_Fan_Num_Blades, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	BitmapButton_Fan_Num_Blades = new xlLockButton(Panel12, ID_BITMAPBUTTON_SLIDER_Fan_Num_Blades, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fan_Num_Blades"));
	BitmapButton_Fan_Num_Blades->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	GridBagSizerFanBlades->Add(BitmapButton_Fan_Num_Blades, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText152 = new wxStaticText(Panel12, ID_STATICTEXT_Fan_Blade_Width, _("Width:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fan_Blade_Width"));
	GridBagSizerFanBlades->Add(StaticText152, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fan_Blade_Width = new BulkEditSlider(Panel12, ID_SLIDER_Fan_Blade_Width, 50, 5, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fan_Blade_Width"));
	GridBagSizerFanBlades->Add(Slider_Fan_Blade_Width, 1, wxALL|wxEXPAND, 1);
	ValueCurve_Fan_Blade_Width = new BulkEditValueCurveButton(Panel12, ID_VALUECURVE_Fan_Blade_Width, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Fan_Blade_Width"));
	GridBagSizerFanBlades->Add(ValueCurve_Fan_Blade_Width, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_Fan_Blade_Width = new BulkEditTextCtrl(Panel12, IDD_TEXTCTRL_Fan_Blade_Width, _T("50"), wxDefaultPosition, wxDLG_UNIT(Panel12,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Fan_Blade_Width"));
	TextCtrl_Fan_Blade_Width->SetMaxLength(3);
	GridBagSizerFanBlades->Add(TextCtrl_Fan_Blade_Width, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	BitmapButton_Fan_Blade_Width = new xlLockButton(Panel12, ID_BITMAPBUTTON_SLIDER_Fan_Blade_Width, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fan_Blade_Width"));
	BitmapButton_Fan_Blade_Width->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	GridBagSizerFanBlades->Add(BitmapButton_Fan_Blade_Width, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText153 = new wxStaticText(Panel12, ID_STATICTEXT_Fan_Blade_Angle, _("Angle:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fan_Blade_Angle"));
	GridBagSizerFanBlades->Add(StaticText153, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fan_Blade_Angle = new BulkEditSlider(Panel12, ID_SLIDER_Fan_Blade_Angle, 90, -360, 360, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fan_Blade_Angle"));
	GridBagSizerFanBlades->Add(Slider_Fan_Blade_Angle, 1, wxALL|wxEXPAND, 1);
	ValueCurve_Fan_Blade_Angle = new BulkEditValueCurveButton(Panel12, ID_VALUECURVE_Fan_Blade_Angle, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Fan_Blade_Angle"));
	GridBagSizerFanBlades->Add(ValueCurve_Fan_Blade_Angle, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_Fan_Blade_Angle = new BulkEditTextCtrl(Panel12, IDD_TEXTCTRL_Fan_Blade_Angle, _T("90"), wxDefaultPosition, wxDLG_UNIT(Panel12,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Fan_Blade_Angle"));
	TextCtrl_Fan_Blade_Angle->SetMaxLength(4);
	GridBagSizerFanBlades->Add(TextCtrl_Fan_Blade_Angle, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	BitmapButton_Fan_Blade_Angle = new xlLockButton(Panel12, ID_BITMAPBUTTON_SLIDER_Fan_Blade_Angle, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fan_Blade_Angle"));
	BitmapButton_Fan_Blade_Angle->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	GridBagSizerFanBlades->Add(BitmapButton_Fan_Blade_Angle, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText146 = new wxStaticText(Panel12, ID_STATICTEXT_Fan_Num_Elements, _("# Elements:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fan_Num_Elements"));
	GridBagSizerFanBlades->Add(StaticText146, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fan_Num_Elements = new BulkEditSlider(Panel12, ID_SLIDER_Fan_Num_Elements, 1, 1, 4, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fan_Num_Elements"));
	GridBagSizerFanBlades->Add(Slider_Fan_Num_Elements, 1, wxALL|wxEXPAND, 1);
	ValueCurve_Fan_Num_Elements = new BulkEditValueCurveButton(Panel12, ID_VALUECURVE_Fan_Num_Elements, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Fan_Num_Elements"));
	GridBagSizerFanBlades->Add(ValueCurve_Fan_Num_Elements, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_Fan_Num_Elements = new BulkEditTextCtrl(Panel12, IDD_TEXTCTRL_Fan_Num_Elements, _T("1"), wxDefaultPosition, wxDLG_UNIT(Panel12,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Fan_Num_Elements"));
	TextCtrl_Fan_Num_Elements->SetMaxLength(3);
	GridBagSizerFanBlades->Add(TextCtrl_Fan_Num_Elements, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	BitmapButton_Fan_Num_Elements = new xlLockButton(Panel12, ID_BITMAPBUTTON_SLIDER_Fan_Num_Elements, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fan_Num_Elements"));
	BitmapButton_Fan_Num_Elements->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	GridBagSizerFanBlades->Add(BitmapButton_Fan_Num_Elements, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText147 = new wxStaticText(Panel12, ID_STATICTEXT_Fan_Element_Width, _("Width:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fan_Element_Width"));
	GridBagSizerFanBlades->Add(StaticText147, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fan_Element_Width = new BulkEditSlider(Panel12, ID_SLIDER_Fan_Element_Width, 100, 5, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fan_Element_Width"));
	GridBagSizerFanBlades->Add(Slider_Fan_Element_Width, 1, wxALL|wxEXPAND, 1);
	ValueCurve_Fan_Element_Width = new BulkEditValueCurveButton(Panel12, ID_VALUECURVE_Fan_Element_Width, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Fan_Element_Width"));
	GridBagSizerFanBlades->Add(ValueCurve_Fan_Element_Width, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_Fan_Element_Width = new BulkEditTextCtrl(Panel12, IDD_TEXTCTRL_Fan_Element_Width, _T("100"), wxDefaultPosition, wxDLG_UNIT(Panel12,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Fan_Element_Width"));
	TextCtrl_Fan_Element_Width->SetMaxLength(3);
	GridBagSizerFanBlades->Add(TextCtrl_Fan_Element_Width, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	BitmapButton_Fan_Element_Width = new xlLockButton(Panel12, ID_BITMAPBUTTON_SLIDER_Fan_Element_Width, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fan_Element_Width"));
	BitmapButton_Fan_Element_Width->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	GridBagSizerFanBlades->Add(BitmapButton_Fan_Element_Width, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel12->SetSizer(GridBagSizerFanBlades);
	Panel11 = new wxPanel(Notebook1, ID_PANEL41, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL41"));
	GridBagSizerFanOptions = new wxFlexGridSizer(0, 5, 0, 0);
	GridBagSizerFanOptions->AddGrowableCol(1);
	StaticText149 = new wxStaticText(Panel11, ID_STATICTEXT_Fan_Duration, _("Duration:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fan_Duration"));
	GridBagSizerFanOptions->Add(StaticText149, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fan_Duration = new BulkEditSlider(Panel11, ID_SLIDER_Fan_Duration, 80, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fan_Duration"));
	GridBagSizerFanOptions->Add(Slider_Fan_Duration, 1, wxALL|wxEXPAND, 1);
	ValueCurve_Fan_Duration = new BulkEditValueCurveButton(Panel11, ID_VALUECURVE_Fan_Duration, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Fan_Duration"));
	GridBagSizerFanOptions->Add(ValueCurve_Fan_Duration, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_Fan_Duration = new BulkEditTextCtrl(Panel11, IDD_TEXTCTRL_Fan_Duration, _T("80"), wxDefaultPosition, wxDLG_UNIT(Panel11,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Fan_Duration"));
	TextCtrl_Fan_Duration->SetMaxLength(3);
	GridBagSizerFanOptions->Add(TextCtrl_Fan_Duration, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	BitmapButton_Fan_Duration = new xlLockButton(Panel11, ID_BITMAPBUTTON_SLIDER_Fan_Duration, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fan_Duration"));
	BitmapButton_Fan_Duration->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	GridBagSizerFanOptions->Add(BitmapButton_Fan_Duration, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText150 = new wxStaticText(Panel11, ID_STATICTEXT_Fan_Accel, _("Acceleration:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fan_Accel"));
	GridBagSizerFanOptions->Add(StaticText150, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Fan_Accel = new BulkEditSlider(Panel11, ID_SLIDER_Fan_Accel, 0, -10, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fan_Accel"));
	GridBagSizerFanOptions->Add(Slider_Fan_Accel, 1, wxALL|wxEXPAND, 1);
	ValueCurve_Fan_Accel = new BulkEditValueCurveButton(Panel11, ID_VALUECURVE_Fan_Accel, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Fan_Accel"));
	GridBagSizerFanOptions->Add(ValueCurve_Fan_Accel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	TextCtrl_Fan_Accel = new BulkEditTextCtrl(Panel11, IDD_TEXTCTRL_Fan_Accel, _T("0"), wxDefaultPosition, wxDLG_UNIT(Panel11,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Fan_Accel"));
	TextCtrl_Fan_Accel->SetMaxLength(3);
	GridBagSizerFanOptions->Add(TextCtrl_Fan_Accel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	BitmapButton_Fan_Accel = new xlLockButton(Panel11, ID_BITMAPBUTTON_SLIDER_Fan_Accel, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fan_Accel"));
	BitmapButton_Fan_Accel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	GridBagSizerFanOptions->Add(BitmapButton_Fan_Accel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	GridBagSizerFanOptions->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer103 = new wxFlexGridSizer(0, 1, 0, 0);
	CheckBox_Fan_Reverse = new BulkEditCheckBox(Panel11, ID_CHECKBOX_Fan_Reverse, _("Reverse:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT, wxDefaultValidator, _T("ID_CHECKBOX_Fan_Reverse"));
	CheckBox_Fan_Reverse->SetValue(false);
	FlexGridSizer103->Add(CheckBox_Fan_Reverse, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Fan_Blend_Edges = new BulkEditCheckBox(Panel11, ID_CHECKBOX_Fan_Blend_Edges, _("Blend Edges:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT, wxDefaultValidator, _T("ID_CHECKBOX_Fan_Blend_Edges"));
	CheckBox_Fan_Blend_Edges->SetValue(true);
	FlexGridSizer103->Add(CheckBox_Fan_Blend_Edges, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Fan_Scale = new BulkEditCheckBox(Panel11, ID_CHECKBOX_Fan_Scale, _("Scale to Buffer:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT, wxDefaultValidator, _T("ID_CHECKBOX_Fan_Scale"));
	CheckBox_Fan_Scale->SetValue(false);
	FlexGridSizer103->Add(CheckBox_Fan_Scale, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	GridBagSizerFanOptions->Add(FlexGridSizer103, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel11->SetSizer(GridBagSizerFanOptions);
	Notebook1->AddPage(Panel9, _("Position"), false);
	Notebook1->AddPage(Panel12, _("Blades"), false);
	Notebook1->AddPage(Panel11, _("Options"), false);
	FlexGridSizer109->Add(Notebook1, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer109);

	Connect(ID_VALUECURVE_Fan_CenterX, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FanPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fan_CenterX, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FanPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Fan_CenterY, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FanPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fan_CenterY, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FanPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Fan_Start_Radius, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FanPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fan_Start_Radius, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FanPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Fan_Start_Angle, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FanPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fan_Start_Angle, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FanPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Fan_End_Radius, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FanPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fan_End_Radius, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FanPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Fan_Revolutions, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FanPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fan_Revolutions, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FanPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Fan_Num_Blades, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FanPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fan_Num_Blades, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FanPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Fan_Blade_Width, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FanPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fan_Blade_Width, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FanPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Fan_Blade_Angle, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FanPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fan_Blade_Angle, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FanPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Fan_Num_Elements, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FanPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fan_Num_Elements, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FanPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Fan_Element_Width, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FanPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fan_Element_Width, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FanPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Fan_Duration, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FanPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fan_Duration, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FanPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Fan_Accel, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FanPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fan_Accel, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&FanPanel::OnLockButtonClick);
	//*)
    SetName("ID_PANEL_FAN");

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&FanPanel::OnVCChanged, nullptr, this);
	Connect(wxID_ANY, EVT_VALIDATEWINDOW, (wxObjectEventFunction)&FanPanel::OnValidateWindow, 0, this);

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

	ValidateWindow();
}

FanPanel::~FanPanel()
{
	//(*Destroy(FanPanel)
	//*)
}

void FanPanel::ValidateWindow()
{
	
}
