/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "GalaxyPanel.h"
#include "EffectPanelUtils.h"
#include "GalaxyEffect.h"

//(*InternalHeaders(GalaxyPanel)
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

//(*IdInit(GalaxyPanel)
const long GalaxyPanel::ID_STATICTEXT_Galaxy_CenterX = wxNewId();
const long GalaxyPanel::ID_SLIDER_Galaxy_CenterX = wxNewId();
const long GalaxyPanel::ID_VALUECURVE_Galaxy_CenterX = wxNewId();
const long GalaxyPanel::IDD_TEXTCTRL_Galaxy_CenterX = wxNewId();
const long GalaxyPanel::ID_BITMAPBUTTON_SLIDER_Galaxy_CenterX = wxNewId();
const long GalaxyPanel::ID_STATICTEXT_Galaxy_CenterY = wxNewId();
const long GalaxyPanel::ID_SLIDER_Galaxy_CenterY = wxNewId();
const long GalaxyPanel::ID_VALUECURVE_Galaxy_CenterY = wxNewId();
const long GalaxyPanel::IDD_TEXTCTRL_Galaxy_CenterY = wxNewId();
const long GalaxyPanel::ID_BITMAPBUTTON_SLIDER_Galaxy_CenterY = wxNewId();
const long GalaxyPanel::ID_STATICTEXT_Galaxy_Start_Radius = wxNewId();
const long GalaxyPanel::ID_SLIDER_Galaxy_Start_Radius = wxNewId();
const long GalaxyPanel::ID_VALUECURVE_Galaxy_Start_Radius = wxNewId();
const long GalaxyPanel::IDD_TEXTCTRL_Galaxy_Start_Radius = wxNewId();
const long GalaxyPanel::ID_BITMAPBUTTON_SLIDER_Galaxy_Start_Radius = wxNewId();
const long GalaxyPanel::ID_STATICTEXT_Galaxy_Start_Width = wxNewId();
const long GalaxyPanel::ID_SLIDER_Galaxy_Start_Width = wxNewId();
const long GalaxyPanel::ID_VALUECURVE_Galaxy_Start_Width = wxNewId();
const long GalaxyPanel::IDD_TEXTCTRL_Galaxy_Start_Width = wxNewId();
const long GalaxyPanel::ID_BITMAPBUTTON_SLIDER_Galaxy_Start_Width = wxNewId();
const long GalaxyPanel::ID_STATICTEXT_Galaxy_Start_Angle = wxNewId();
const long GalaxyPanel::ID_SLIDER_Galaxy_Start_Angle = wxNewId();
const long GalaxyPanel::ID_VALUECURVE_Galaxy_Start_Angle = wxNewId();
const long GalaxyPanel::IDD_TEXTCTRL_Galaxy_Start_Angle = wxNewId();
const long GalaxyPanel::ID_BITMAPBUTTON_SLIDER_Galaxy_Start_Angle = wxNewId();
const long GalaxyPanel::ID_PANEL29 = wxNewId();
const long GalaxyPanel::ID_STATICTEXT_Galaxy_End_Radius = wxNewId();
const long GalaxyPanel::ID_SLIDER_Galaxy_End_Radius = wxNewId();
const long GalaxyPanel::ID_VALUECURVE_Galaxy_End_Radius = wxNewId();
const long GalaxyPanel::IDD_TEXTCTRL_Galaxy_End_Radius = wxNewId();
const long GalaxyPanel::ID_BITMAPBUTTON_SLIDER_Galaxy_End_Radius = wxNewId();
const long GalaxyPanel::ID_STATICTEXT_Galaxy_End_Width = wxNewId();
const long GalaxyPanel::ID_SLIDER_Galaxy_End_Width = wxNewId();
const long GalaxyPanel::ID_VALUECURVE_Galaxy_End_Width = wxNewId();
const long GalaxyPanel::IDD_TEXTCTRL_Galaxy_End_Width = wxNewId();
const long GalaxyPanel::ID_BITMAPBUTTON_SLIDER_Galaxy_End_Width = wxNewId();
const long GalaxyPanel::ID_STATICTEXT_Galaxy_Revolutions = wxNewId();
const long GalaxyPanel::ID_SLIDER_Galaxy_Revolutions = wxNewId();
const long GalaxyPanel::ID_VALUECURVE_Galaxy_Revolutions = wxNewId();
const long GalaxyPanel::IDD_TEXTCTRL_Galaxy_Revolutions = wxNewId();
const long GalaxyPanel::ID_BITMAPBUTTON_SLIDER_Galaxy_Revolutions = wxNewId();
const long GalaxyPanel::ID_PANEL31 = wxNewId();
const long GalaxyPanel::ID_STATICTEXT_Galaxy_Duration = wxNewId();
const long GalaxyPanel::ID_SLIDER_Galaxy_Duration = wxNewId();
const long GalaxyPanel::ID_VALUECURVE_Galaxy_Duration = wxNewId();
const long GalaxyPanel::IDD_TEXTCTRL_Galaxy_Duration = wxNewId();
const long GalaxyPanel::ID_BITMAPBUTTON_SLIDER_Galaxy_Duration = wxNewId();
const long GalaxyPanel::ID_STATICTEXT_Galaxy_Accel = wxNewId();
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

GalaxyPanel::GalaxyPanel(wxWindow* parent) : xlEffectPanel(parent)
{
	//(*Initialize(GalaxyPanel)
	BulkEditTextCtrl* TextCtrl_Galaxy_Accel;
	BulkEditTextCtrl* TextCtrl_Galaxy_CenterX;
	BulkEditTextCtrl* TextCtrl_Galaxy_CenterY;
	BulkEditTextCtrl* TextCtrl_Galaxy_Duration;
	BulkEditTextCtrl* TextCtrl_Galaxy_End_Radius;
	BulkEditTextCtrl* TextCtrl_Galaxy_End_Width;
	BulkEditTextCtrl* TextCtrl_Galaxy_Start_Angle;
	BulkEditTextCtrl* TextCtrl_Galaxy_Start_Radius;
	BulkEditTextCtrl* TextCtrl_Galaxy_Start_Width;
	BulkEditTextCtrlF360* TextCtrl_Galaxy_Revolutions;
	wxFlexGridSizer* FlexGridSizer108;
	wxFlexGridSizer* GridBagSizerGalaxyEnd;
	wxFlexGridSizer* GridBagSizerGalaxyOptions;
	wxFlexGridSizer* GridBagSizerGalaxyStart;
	wxPanel* Panel6;
	wxPanel* Panel7;
	wxPanel* Panel8;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer108 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer108->AddGrowableCol(0);
	Notebook_Galaxy = new wxNotebook(this, ID_NOTEBOOK_Galaxy, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK_Galaxy"));
	Panel6 = new wxPanel(Notebook_Galaxy, ID_PANEL29, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL29"));
	GridBagSizerGalaxyStart = new wxFlexGridSizer(0, 5, 0, 0);
	GridBagSizerGalaxyStart->AddGrowableCol(1);
	StaticText125 = new wxStaticText(Panel6, ID_STATICTEXT_Galaxy_CenterX, _("Center X:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Galaxy_CenterX"));
	GridBagSizerGalaxyStart->Add(StaticText125, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Galaxy_CenterX = new BulkEditSlider(Panel6, ID_SLIDER_Galaxy_CenterX, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Galaxy_CenterX"));
	GridBagSizerGalaxyStart->Add(Slider_Galaxy_CenterX, 1, wxALL|wxEXPAND, 5);
	ValueCurve_Galaxy_CenterX = new BulkEditValueCurveButton(Panel6, ID_VALUECURVE_Galaxy_CenterX, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Galaxy_CenterX"));
	GridBagSizerGalaxyStart->Add(ValueCurve_Galaxy_CenterX, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Galaxy_CenterX = new BulkEditTextCtrl(Panel6, IDD_TEXTCTRL_Galaxy_CenterX, _("50"), wxDefaultPosition, wxDLG_UNIT(Panel6,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Galaxy_CenterX"));
	TextCtrl_Galaxy_CenterX->SetMaxLength(3);
	GridBagSizerGalaxyStart->Add(TextCtrl_Galaxy_CenterX, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, wxDLG_UNIT(Panel6,wxSize(5,0)).GetWidth());
	BitmapButton_Galaxy_CenterX = new xlLockButton(Panel6, ID_BITMAPBUTTON_SLIDER_Galaxy_CenterX, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Galaxy_CenterX"));
	BitmapButton_Galaxy_CenterX->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	GridBagSizerGalaxyStart->Add(BitmapButton_Galaxy_CenterX, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText126 = new wxStaticText(Panel6, ID_STATICTEXT_Galaxy_CenterY, _("Center Y:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Galaxy_CenterY"));
	GridBagSizerGalaxyStart->Add(StaticText126, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Galaxy_CenterY = new BulkEditSlider(Panel6, ID_SLIDER_Galaxy_CenterY, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Galaxy_CenterY"));
	GridBagSizerGalaxyStart->Add(Slider_Galaxy_CenterY, 1, wxALL|wxEXPAND, 5);
	ValueCurve_Galaxy_CenterY = new BulkEditValueCurveButton(Panel6, ID_VALUECURVE_Galaxy_CenterY, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Galaxy_CenterY"));
	GridBagSizerGalaxyStart->Add(ValueCurve_Galaxy_CenterY, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Galaxy_CenterY = new BulkEditTextCtrl(Panel6, IDD_TEXTCTRL_Galaxy_CenterY, _("50"), wxDefaultPosition, wxDLG_UNIT(Panel6,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Galaxy_CenterY"));
	TextCtrl_Galaxy_CenterY->SetMaxLength(3);
	GridBagSizerGalaxyStart->Add(TextCtrl_Galaxy_CenterY, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Galaxy_CenterY = new xlLockButton(Panel6, ID_BITMAPBUTTON_SLIDER_Galaxy_CenterY, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Galaxy_CenterY"));
	BitmapButton_Galaxy_CenterY->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	GridBagSizerGalaxyStart->Add(BitmapButton_Galaxy_CenterY, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText127 = new wxStaticText(Panel6, ID_STATICTEXT_Galaxy_Start_Radius, _("Radius:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Galaxy_Start_Radius"));
	GridBagSizerGalaxyStart->Add(StaticText127, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Galaxy_Start_Radius = new BulkEditSlider(Panel6, ID_SLIDER_Galaxy_Start_Radius, 1, 0, 250, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Galaxy_Start_Radius"));
	GridBagSizerGalaxyStart->Add(Slider_Galaxy_Start_Radius, 1, wxALL|wxEXPAND, 5);
	ValueCurve_Galaxy_Start_Radius = new BulkEditValueCurveButton(Panel6, ID_VALUECURVE_Galaxy_Start_Radius, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Galaxy_Start_Radius"));
	GridBagSizerGalaxyStart->Add(ValueCurve_Galaxy_Start_Radius, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Galaxy_Start_Radius = new BulkEditTextCtrl(Panel6, IDD_TEXTCTRL_Galaxy_Start_Radius, _("1"), wxDefaultPosition, wxDLG_UNIT(Panel6,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Galaxy_Start_Radius"));
	TextCtrl_Galaxy_Start_Radius->SetMaxLength(3);
	GridBagSizerGalaxyStart->Add(TextCtrl_Galaxy_Start_Radius, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Galaxy_Start_Radius = new xlLockButton(Panel6, ID_BITMAPBUTTON_SLIDER_Galaxy_Start_Radius, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Galaxy_Start_Radius"));
	BitmapButton_Galaxy_Start_Radius->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	GridBagSizerGalaxyStart->Add(BitmapButton_Galaxy_Start_Radius, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText131 = new wxStaticText(Panel6, ID_STATICTEXT_Galaxy_Start_Width, _("Width:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Galaxy_Start_Width"));
	GridBagSizerGalaxyStart->Add(StaticText131, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Galaxy_Start_Width = new BulkEditSlider(Panel6, ID_SLIDER_Galaxy_Start_Width, 5, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Galaxy_Start_Width"));
	GridBagSizerGalaxyStart->Add(Slider_Galaxy_Start_Width, 1, wxALL|wxEXPAND, 5);
	ValueCurve_Galaxy_Start_Width = new BulkEditValueCurveButton(Panel6, ID_VALUECURVE_Galaxy_Start_Width, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Galaxy_Start_Width"));
	GridBagSizerGalaxyStart->Add(ValueCurve_Galaxy_Start_Width, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Galaxy_Start_Width = new BulkEditTextCtrl(Panel6, IDD_TEXTCTRL_Galaxy_Start_Width, _("5"), wxDefaultPosition, wxDLG_UNIT(Panel6,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Galaxy_Start_Width"));
	TextCtrl_Galaxy_Start_Width->SetMaxLength(3);
	GridBagSizerGalaxyStart->Add(TextCtrl_Galaxy_Start_Width, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Galaxy_Start_Width = new xlLockButton(Panel6, ID_BITMAPBUTTON_SLIDER_Galaxy_Start_Width, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Galaxy_Start_Width"));
	BitmapButton_Galaxy_Start_Width->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	GridBagSizerGalaxyStart->Add(BitmapButton_Galaxy_Start_Width, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText129 = new wxStaticText(Panel6, ID_STATICTEXT_Galaxy_Start_Angle, _("Angle:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Galaxy_Start_Angle"));
	GridBagSizerGalaxyStart->Add(StaticText129, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Galaxy_Start_Angle = new BulkEditSlider(Panel6, ID_SLIDER_Galaxy_Start_Angle, 0, 0, 360, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Galaxy_Start_Angle"));
	GridBagSizerGalaxyStart->Add(Slider_Galaxy_Start_Angle, 1, wxALL|wxEXPAND, 5);
	ValueCurve_Galaxy_Start_Angle = new BulkEditValueCurveButton(Panel6, ID_VALUECURVE_Galaxy_Start_Angle, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Galaxy_Start_Angle"));
	GridBagSizerGalaxyStart->Add(ValueCurve_Galaxy_Start_Angle, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Galaxy_Start_Angle = new BulkEditTextCtrl(Panel6, IDD_TEXTCTRL_Galaxy_Start_Angle, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel6,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Galaxy_Start_Angle"));
	TextCtrl_Galaxy_Start_Angle->SetMaxLength(3);
	GridBagSizerGalaxyStart->Add(TextCtrl_Galaxy_Start_Angle, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Galaxy_Start_Angle = new xlLockButton(Panel6, ID_BITMAPBUTTON_SLIDER_Galaxy_Start_Angle, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Galaxy_Start_Angle"));
	BitmapButton_Galaxy_Start_Angle->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	GridBagSizerGalaxyStart->Add(BitmapButton_Galaxy_Start_Angle, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel6->SetSizer(GridBagSizerGalaxyStart);
	GridBagSizerGalaxyStart->Fit(Panel6);
	GridBagSizerGalaxyStart->SetSizeHints(Panel6);
	Panel7 = new wxPanel(Notebook_Galaxy, ID_PANEL31, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL31"));
	GridBagSizerGalaxyEnd = new wxFlexGridSizer(0, 5, 0, 0);
	GridBagSizerGalaxyEnd->AddGrowableCol(1);
	StaticText138 = new wxStaticText(Panel7, ID_STATICTEXT_Galaxy_End_Radius, _("Radius:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Galaxy_End_Radius"));
	GridBagSizerGalaxyEnd->Add(StaticText138, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Galaxy_End_Radius = new BulkEditSlider(Panel7, ID_SLIDER_Galaxy_End_Radius, 10, 0, 250, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Galaxy_End_Radius"));
	GridBagSizerGalaxyEnd->Add(Slider_Galaxy_End_Radius, 1, wxALL|wxEXPAND, 5);
	ValueCurve_Galaxy_End_Radius = new BulkEditValueCurveButton(Panel7, ID_VALUECURVE_Galaxy_End_Radius, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Galaxy_End_Radius"));
	GridBagSizerGalaxyEnd->Add(ValueCurve_Galaxy_End_Radius, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Galaxy_End_Radius = new BulkEditTextCtrl(Panel7, IDD_TEXTCTRL_Galaxy_End_Radius, _("10"), wxDefaultPosition, wxDLG_UNIT(Panel7,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Galaxy_End_Radius"));
	TextCtrl_Galaxy_End_Radius->SetMaxLength(3);
	GridBagSizerGalaxyEnd->Add(TextCtrl_Galaxy_End_Radius, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Galaxy_End_Radius = new xlLockButton(Panel7, ID_BITMAPBUTTON_SLIDER_Galaxy_End_Radius, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Galaxy_End_Radius"));
	BitmapButton_Galaxy_End_Radius->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	GridBagSizerGalaxyEnd->Add(BitmapButton_Galaxy_End_Radius, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText139 = new wxStaticText(Panel7, ID_STATICTEXT_Galaxy_End_Width, _("Width:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Galaxy_End_Width"));
	GridBagSizerGalaxyEnd->Add(StaticText139, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Galaxy_End_Width = new BulkEditSlider(Panel7, ID_SLIDER_Galaxy_End_Width, 5, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Galaxy_End_Width"));
	GridBagSizerGalaxyEnd->Add(Slider_Galaxy_End_Width, 1, wxALL|wxEXPAND, 5);
	ValueCurve_Galaxy_End_Width = new BulkEditValueCurveButton(Panel7, ID_VALUECURVE_Galaxy_End_Width, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Galaxy_End_Width"));
	GridBagSizerGalaxyEnd->Add(ValueCurve_Galaxy_End_Width, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Galaxy_End_Width = new BulkEditTextCtrl(Panel7, IDD_TEXTCTRL_Galaxy_End_Width, _("5"), wxDefaultPosition, wxDLG_UNIT(Panel7,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Galaxy_End_Width"));
	TextCtrl_Galaxy_End_Width->SetMaxLength(3);
	GridBagSizerGalaxyEnd->Add(TextCtrl_Galaxy_End_Width, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Galaxy_End_Width = new xlLockButton(Panel7, ID_BITMAPBUTTON_SLIDER_Galaxy_End_Width, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Galaxy_End_Width"));
	BitmapButton_Galaxy_End_Width->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	GridBagSizerGalaxyEnd->Add(BitmapButton_Galaxy_End_Width, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText140 = new wxStaticText(Panel7, ID_STATICTEXT_Galaxy_Revolutions, _("Rev\'s:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Galaxy_Revolutions"));
	GridBagSizerGalaxyEnd->Add(StaticText140, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Galaxy_Revolutions = new BulkEditSliderF360(Panel7, ID_SLIDER_Galaxy_Revolutions, 1440, 0, 3600, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Galaxy_Revolutions"));
	GridBagSizerGalaxyEnd->Add(Slider_Galaxy_Revolutions, 1, wxALL|wxEXPAND, 5);
	ValueCurve_Galaxy_Revolutions = new BulkEditValueCurveButton(Panel7, ID_VALUECURVE_Galaxy_Revolutions, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Galaxy_Revolutions"));
	GridBagSizerGalaxyEnd->Add(ValueCurve_Galaxy_Revolutions, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Galaxy_Revolutions = new BulkEditTextCtrlF360(Panel7, IDD_TEXTCTRL_Galaxy_Revolutions, _("4.0"), wxDefaultPosition, wxDLG_UNIT(Panel7,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Galaxy_Revolutions"));
	TextCtrl_Galaxy_Revolutions->SetMaxLength(4);
	GridBagSizerGalaxyEnd->Add(TextCtrl_Galaxy_Revolutions, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Galaxy_Revolutions = new xlLockButton(Panel7, ID_BITMAPBUTTON_SLIDER_Galaxy_Revolutions, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Galaxy_Revolutions"));
	BitmapButton_Galaxy_Revolutions->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	GridBagSizerGalaxyEnd->Add(BitmapButton_Galaxy_Revolutions, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel7->SetSizer(GridBagSizerGalaxyEnd);
	GridBagSizerGalaxyEnd->Fit(Panel7);
	GridBagSizerGalaxyEnd->SetSizeHints(Panel7);
	Panel8 = new wxPanel(Notebook_Galaxy, ID_PANEL32, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL32"));
	GridBagSizerGalaxyOptions = new wxFlexGridSizer(0, 5, 0, 0);
	GridBagSizerGalaxyOptions->AddGrowableCol(1);
	StaticText141 = new wxStaticText(Panel8, ID_STATICTEXT_Galaxy_Duration, _("Head Duration:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Galaxy_Duration"));
	GridBagSizerGalaxyOptions->Add(StaticText141, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Galaxy_Duration = new BulkEditSlider(Panel8, ID_SLIDER_Galaxy_Duration, 20, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Galaxy_Duration"));
	GridBagSizerGalaxyOptions->Add(Slider_Galaxy_Duration, 1, wxALL|wxEXPAND, 5);
	ValueCurve_Galaxy_Duration = new BulkEditValueCurveButton(Panel8, ID_VALUECURVE_Galaxy_Duration, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Galaxy_Duration"));
	GridBagSizerGalaxyOptions->Add(ValueCurve_Galaxy_Duration, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Galaxy_Duration = new BulkEditTextCtrl(Panel8, IDD_TEXTCTRL_Galaxy_Duration, _("20"), wxDefaultPosition, wxDLG_UNIT(Panel8,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Galaxy_Duration"));
	TextCtrl_Galaxy_Duration->SetMaxLength(3);
	GridBagSizerGalaxyOptions->Add(TextCtrl_Galaxy_Duration, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Galaxy_Duration = new xlLockButton(Panel8, ID_BITMAPBUTTON_SLIDER_Galaxy_Duration, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Galaxy_Duration"));
	BitmapButton_Galaxy_Duration->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	GridBagSizerGalaxyOptions->Add(BitmapButton_Galaxy_Duration, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText142 = new wxStaticText(Panel8, ID_STATICTEXT_Galaxy_Accel, _("Acceleration:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Galaxy_Accel"));
	GridBagSizerGalaxyOptions->Add(StaticText142, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Galaxy_Accel = new BulkEditSlider(Panel8, ID_SLIDER_Galaxy_Accel, 0, -10, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Galaxy_Accel"));
	GridBagSizerGalaxyOptions->Add(Slider_Galaxy_Accel, 1, wxALL|wxEXPAND, 5);
	ValueCurve_Galaxy_Accel = new BulkEditValueCurveButton(Panel8, ID_VALUECURVE_Galaxy_Accel, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Galaxy_Accel"));
	GridBagSizerGalaxyOptions->Add(ValueCurve_Galaxy_Accel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Galaxy_Accel = new BulkEditTextCtrl(Panel8, IDD_TEXTCTRL_Galaxy_Accel, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel8,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Galaxy_Accel"));
	TextCtrl_Galaxy_Accel->SetMaxLength(3);
	GridBagSizerGalaxyOptions->Add(TextCtrl_Galaxy_Accel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Galaxy_Accel = new xlLockButton(Panel8, ID_BITMAPBUTTON_SLIDER_Galaxy_Accel, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Galaxy_Accel"));
	BitmapButton_Galaxy_Accel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	GridBagSizerGalaxyOptions->Add(BitmapButton_Galaxy_Accel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Galaxy_Reverse = new BulkEditCheckBox(Panel8, ID_CHECKBOX_Galaxy_Reverse, _("Reverse:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT, wxDefaultValidator, _T("ID_CHECKBOX_Galaxy_Reverse"));
	CheckBox_Galaxy_Reverse->SetValue(false);
	GridBagSizerGalaxyOptions->Add(CheckBox_Galaxy_Reverse, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	GridBagSizerGalaxyOptions->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	GridBagSizerGalaxyOptions->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	GridBagSizerGalaxyOptions->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	GridBagSizerGalaxyOptions->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Galaxy_Blend_Edges = new BulkEditCheckBox(Panel8, ID_CHECKBOX_Galaxy_Blend_Edges, _("Blend Edges:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT, wxDefaultValidator, _T("ID_CHECKBOX_Galaxy_Blend_Edges"));
	CheckBox_Galaxy_Blend_Edges->SetValue(true);
	GridBagSizerGalaxyOptions->Add(CheckBox_Galaxy_Blend_Edges, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	GridBagSizerGalaxyOptions->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	GridBagSizerGalaxyOptions->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	GridBagSizerGalaxyOptions->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	GridBagSizerGalaxyOptions->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Galaxy_Inward = new BulkEditCheckBox(Panel8, ID_CHECKBOX_Galaxy_Inward, _("Inward:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT, wxDefaultValidator, _T("ID_CHECKBOX_Galaxy_Inward"));
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

	Connect(ID_VALUECURVE_Galaxy_CenterX,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GalaxyPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Galaxy_CenterX,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GalaxyPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Galaxy_CenterY,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GalaxyPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Galaxy_CenterY,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GalaxyPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Galaxy_Start_Radius,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GalaxyPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Galaxy_Start_Radius,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GalaxyPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Galaxy_Start_Width,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GalaxyPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Galaxy_Start_Width,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GalaxyPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Galaxy_Start_Angle,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GalaxyPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Galaxy_Start_Angle,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GalaxyPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Galaxy_End_Radius,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GalaxyPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Galaxy_End_Radius,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GalaxyPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Galaxy_End_Width,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GalaxyPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Galaxy_End_Width,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GalaxyPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Galaxy_Revolutions,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GalaxyPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Galaxy_Revolutions,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GalaxyPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Galaxy_Duration,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GalaxyPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Galaxy_Duration,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GalaxyPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Galaxy_Accel,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GalaxyPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Galaxy_Accel,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GalaxyPanel::OnLockButtonClick);
	//*)
    SetName("ID_PANEL_GALAXY");

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&GalaxyPanel::OnVCChanged, 0, this);
    Connect(wxID_ANY, EVT_VALIDATEWINDOW, (wxObjectEventFunction)&GalaxyPanel::OnValidateWindow, 0, this);

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

    ValidateWindow();
}

GalaxyPanel::~GalaxyPanel()
{
	//(*Destroy(GalaxyPanel)
	//*)
}

void GalaxyPanel::ValidateWindow()
{
}
