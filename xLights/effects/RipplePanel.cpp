/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "RipplePanel.h"
#include "EffectPanelUtils.h"
#include "RippleEffect.h"

//(*InternalHeaders(RipplePanel)
#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/filepicker.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>
//*)

//(*IdInit(RipplePanel)
const long RipplePanel::ID_STATICTEXT_Ripple_Draw_Style = wxNewId();
const long RipplePanel::ID_CHOICE_Ripple_Draw_Style = wxNewId();
const long RipplePanel::ID_BITMAPBUTTON_CHOICE_Ripple_Draw_Style = wxNewId();
const long RipplePanel::ID_STATICTEXT_Ripple_Object_To_Draw = wxNewId();
const long RipplePanel::ID_CHOICE_Ripple_Object_To_Draw = wxNewId();
const long RipplePanel::ID_BITMAPBUTTON_CHOICE_Ripple_Object_To_Draw = wxNewId();
const long RipplePanel::ID_FILEPICKERCTRL_Ripple_SVG = wxNewId();
const long RipplePanel::ID_STATICTEXT_Ripple_Movement = wxNewId();
const long RipplePanel::ID_CHOICE_Ripple_Movement = wxNewId();
const long RipplePanel::ID_BITMAPBUTTON_CHOICE_Ripple_Movement = wxNewId();
const long RipplePanel::ID_STATICTEXT6 = wxNewId();
const long RipplePanel::ID_SLIDER_Ripple_Scale = wxNewId();
const long RipplePanel::ID_VALUECURVE_Ripple_Scale = wxNewId();
const long RipplePanel::IDD_TEXTCTRL_Ripple_Scale = wxNewId();
const long RipplePanel::ID_BITMAPBUTTON_SLIDER_Ripple_Scale = wxNewId();
const long RipplePanel::IDD_SLIDER_Ripple_Outline = wxNewId();
const long RipplePanel::ID_VALUECURVE_Ripple_Outline = wxNewId();
const long RipplePanel::ID_TEXTCTRL_Ripple_Outline = wxNewId();
const long RipplePanel::ID_BITMAPBUTTON_SLIDER_Ripple_Outline = wxNewId();
const long RipplePanel::ID_STATICTEXT_Ripple_Thickness = wxNewId();
const long RipplePanel::ID_SLIDER_Ripple_Thickness = wxNewId();
const long RipplePanel::ID_VALUECURVE_Ripple_Thickness = wxNewId();
const long RipplePanel::IDD_TEXTCTRL_Ripple_Thickness = wxNewId();
const long RipplePanel::ID_BITMAPBUTTON_SLIDER_Ripple_Thickness = wxNewId();
const long RipplePanel::ID_STATICTEXT4 = wxNewId();
const long RipplePanel::IDD_SLIDER_Ripple_Spacing = wxNewId();
const long RipplePanel::ID_VALUECURVE_Ripple_Spacing = wxNewId();
const long RipplePanel::ID_TEXTCTRL_Ripple_Spacing = wxNewId();
const long RipplePanel::ID_BITMAPBUTTON_SLIDER_Ripple_Spacing = wxNewId();
const long RipplePanel::ID_STATICTEXT_Ripple_Cycles = wxNewId();
const long RipplePanel::IDD_SLIDER_Ripple_Cycles = wxNewId();
const long RipplePanel::ID_VALUECURVE_Ripple_Cycles = wxNewId();
const long RipplePanel::ID_TEXTCTRL_Ripple_Cycles = wxNewId();
const long RipplePanel::ID_STATICTEXT1 = wxNewId();
const long RipplePanel::ID_SLIDER_RIPPLE_POINTS = wxNewId();
const long RipplePanel::IDD_TEXTCTRL_RIPPLE_POINTS = wxNewId();
const long RipplePanel::ID_BITMAPBUTTON_SLIDER_RIPPLE_POINTS = wxNewId();
const long RipplePanel::ID_STATICTEXT_Ripple_Rotation = wxNewId();
const long RipplePanel::ID_SLIDER_Ripple_Rotation = wxNewId();
const long RipplePanel::ID_VALUECURVE_Ripple_Rotation = wxNewId();
const long RipplePanel::IDD_TEXTCTRL_Ripple_Rotation = wxNewId();
const long RipplePanel::ID_BITMAPBUTTON_SLIDER_Ripple_Rotation = wxNewId();
const long RipplePanel::ID_STATICTEXT5 = wxNewId();
const long RipplePanel::IDD_SLIDER_Ripple_Twist = wxNewId();
const long RipplePanel::ID_VALUECURVE_Ripple_Twist = wxNewId();
const long RipplePanel::ID_TEXTCTRL_Ripple_Twist = wxNewId();
const long RipplePanel::ID_BITMAPBUTTON_SLIDER_Ripple_Twist = wxNewId();
const long RipplePanel::ID_STATICTEXT2 = wxNewId();
const long RipplePanel::ID_SLIDER_Ripple_XC = wxNewId();
const long RipplePanel::ID_VALUECURVE_Ripple_XC = wxNewId();
const long RipplePanel::IDD_TEXTCTRL_Ripple_XC = wxNewId();
const long RipplePanel::ID_STATICTEXT3 = wxNewId();
const long RipplePanel::ID_SLIDER_Ripple_YC = wxNewId();
const long RipplePanel::ID_VALUECURVE_Ripple_YC = wxNewId();
const long RipplePanel::IDD_TEXTCTRL_Ripple_YC = wxNewId();
const long RipplePanel::ID_STATICTEXT7 = wxNewId();
const long RipplePanel::IDD_SLIDER_Ripple_Velocity = wxNewId();
const long RipplePanel::ID_VALUECURVE_Ripple_Velocity = wxNewId();
const long RipplePanel::ID_TEXTCTRL_Ripple_Velocity = wxNewId();
const long RipplePanel::ID_BITMAPBUTTON_SLIDER_Ripple_Velocity = wxNewId();
const long RipplePanel::ID_STATICTEXT8 = wxNewId();
const long RipplePanel::ID_SLIDER_Ripple_Direction = wxNewId();
const long RipplePanel::ID_VALUECURVE_Ripple_Direction = wxNewId();
const long RipplePanel::IDD_TEXTCTRL_Ripple_Direction = wxNewId();
const long RipplePanel::ID_BITMAPBUTTON_SLIDER_Ripple_Direction = wxNewId();
const long RipplePanel::ID_CHECKBOX_Ripple3D = wxNewId();
const long RipplePanel::ID_BITMAPBUTTON_CHECKBOX_Ripple3D = wxNewId();
//*)

BEGIN_EVENT_TABLE(RipplePanel,wxPanel)
	//(*EventTable(RipplePanel)
	//*)
END_EVENT_TABLE()

RipplePanel::RipplePanel(wxWindow* parent) : xlEffectPanel(parent)
{
	//(*Initialize(RipplePanel)
	BulkEditTextCtrl* TextCtrl33;
	BulkEditTextCtrlF1* TextCtrl34;
	wxFlexGridSizer* FlexGridSizer10;
	wxFlexGridSizer* FlexGridSizer11;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer57;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer60;
	wxFlexGridSizer* FlexGridSizer6;
	wxFlexGridSizer* FlexGridSizer7;
	wxFlexGridSizer* FlexGridSizer8;
	wxFlexGridSizer* FlexGridSizer9;
	wxStaticText* StaticText_Outline;
	wxStaticText* StaticText_SVG;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer57 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer57->AddGrowableCol(1);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT_Ripple_Draw_Style, _("Draw Style"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Ripple_Draw_Style"));
	FlexGridSizer57->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Ripple_Draw_Style = new BulkEditChoice(this, ID_CHOICE_Ripple_Draw_Style, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Ripple_Draw_Style"));
	Choice_Ripple_Draw_Style->SetSelection( Choice_Ripple_Draw_Style->Append(_("Old")) );
	Choice_Ripple_Draw_Style->Append(_("Lines Inward"));
	Choice_Ripple_Draw_Style->Append(_("Lines Outward"));
	Choice_Ripple_Draw_Style->Append(_("Lines Both"));
	Choice_Ripple_Draw_Style->Append(_("Lines Inward Ripple"));
	Choice_Ripple_Draw_Style->Append(_("Lines Outward Ripple"));
	Choice_Ripple_Draw_Style->Append(_("Lines Both Ripple"));
	Choice_Ripple_Draw_Style->Append(_("Solid Inward"));
	Choice_Ripple_Draw_Style->Append(_("Solid Outward"));
	Choice_Ripple_Draw_Style->Append(_("Solid Both"));
	Choice_Ripple_Draw_Style->Append(_("Solid Inward Ripple"));
	Choice_Ripple_Draw_Style->Append(_("Solid Outward Ripple"));
	Choice_Ripple_Draw_Style->Append(_("Solid Both Ripple"));
	Choice_Ripple_Draw_Style->Append(_("Highlight Inward"));
	Choice_Ripple_Draw_Style->Append(_("Highlight Outward"));
	Choice_Ripple_Draw_Style->Append(_("Highlight Both"));
	Choice_Ripple_Draw_Style->Append(_("Highlight Inward Ripple"));
	Choice_Ripple_Draw_Style->Append(_("Highlight Outward Ripple"));
	Choice_Ripple_Draw_Style->Append(_("Highlight Both Ripple"));
	FlexGridSizer57->Add(Choice_Ripple_Draw_Style, 1, wxALL|wxALIGN_LEFT, 5);
	FlexGridSizer57->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Ripple_Draw_Style = new xlLockButton(this, ID_BITMAPBUTTON_CHOICE_Ripple_Draw_Style, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Ripple_Draw_Style"));
	BitmapButton_Ripple_Draw_Style->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer57->Add(BitmapButton_Ripple_Draw_Style, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText69 = new wxStaticText(this, ID_STATICTEXT_Ripple_Object_To_Draw, _("Object"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Ripple_Object_To_Draw"));
	FlexGridSizer57->Add(StaticText69, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Ripple_Object_To_Draw = new BulkEditChoice(this, ID_CHOICE_Ripple_Object_To_Draw, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Ripple_Object_To_Draw"));
	Choice_Ripple_Object_To_Draw->SetSelection( Choice_Ripple_Object_To_Draw->Append(_("Circle")) );
	Choice_Ripple_Object_To_Draw->Append(_("Square"));
	Choice_Ripple_Object_To_Draw->Append(_("Triangle"));
	Choice_Ripple_Object_To_Draw->Append(_("Star"));
	Choice_Ripple_Object_To_Draw->Append(_("Polygon"));
	Choice_Ripple_Object_To_Draw->Append(_("Heart"));
	Choice_Ripple_Object_To_Draw->Append(_("Tree"));
	Choice_Ripple_Object_To_Draw->Append(_("Candy Cane"));
	Choice_Ripple_Object_To_Draw->Append(_("Snow Flake"));
	Choice_Ripple_Object_To_Draw->Append(_("Crucifix"));
	Choice_Ripple_Object_To_Draw->Append(_("Present"));
	Choice_Ripple_Object_To_Draw->Append(_("SVG"));
	FlexGridSizer57->Add(Choice_Ripple_Object_To_Draw, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer57->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Ripple_Object_To_Draw = new xlLockButton(this, ID_BITMAPBUTTON_CHOICE_Ripple_Object_To_Draw, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Ripple_Object_To_Draw"));
	BitmapButton_Ripple_Object_To_Draw->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer57->Add(BitmapButton_Ripple_Object_To_Draw, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 1);
	StaticText_SVG = new wxStaticText(this, wxID_ANY, _("SVG"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer57->Add(StaticText_SVG, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FilePickerCtrl_Ripple_SVG = new BulkEditFilePickerCtrl(this, ID_FILEPICKERCTRL_Ripple_SVG, wxEmptyString, _("Choose a SIMPLE svg file"), _T("*.svg"), wxDefaultPosition, wxDefaultSize, wxFLP_FILE_MUST_EXIST|wxFLP_OPEN|wxFLP_USE_TEXTCTRL, wxDefaultValidator, _T("ID_FILEPICKERCTRL_Ripple_SVG"));
	FlexGridSizer57->Add(FilePickerCtrl_Ripple_SVG, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer57->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer57->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText70 = new wxStaticText(this, ID_STATICTEXT_Ripple_Movement, _("Movement"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Ripple_Movement"));
	FlexGridSizer57->Add(StaticText70, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Ripple_Movement = new BulkEditChoice(this, ID_CHOICE_Ripple_Movement, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Ripple_Movement"));
	Choice_Ripple_Movement->SetSelection( Choice_Ripple_Movement->Append(_("Explode")) );
	Choice_Ripple_Movement->Append(_("Implode"));
	Choice_Ripple_Movement->Append(_("None"));
	FlexGridSizer57->Add(Choice_Ripple_Movement, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer57->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Ripple_Movement = new xlLockButton(this, ID_BITMAPBUTTON_CHOICE_Ripple_Movement, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Ripple_Movement"));
	BitmapButton_Ripple_Movement->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer57->Add(BitmapButton_Ripple_Movement, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText7 = new wxStaticText(this, ID_STATICTEXT6, _("Scale"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer57->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer8 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer8->AddGrowableCol(0);
	Slider_Ripple_Scale = new BulkEditSlider(this, ID_SLIDER_Ripple_Scale, 100, 0, 500, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Ripple_Scale"));
	FlexGridSizer8->Add(Slider_Ripple_Scale, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Ripple_ScaleVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Ripple_Scale, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Ripple_Scale"));
	FlexGridSizer8->Add(BitmapButton_Ripple_ScaleVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer57->Add(FlexGridSizer8, 1, wxALL|wxEXPAND, 0);
	TextCtrl_Ripple_Scale = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Ripple_Scale, _("100"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Ripple_Scale"));
	TextCtrl_Ripple_Scale->SetMaxLength(3);
	FlexGridSizer57->Add(TextCtrl_Ripple_Scale, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Ripple_Scale = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Ripple_Scale, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Ripple_Scale"));
	BitmapButton_Ripple_Scale->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer57->Add(BitmapButton_Ripple_Scale, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText_Outline = new wxStaticText(this, wxID_ANY, _("Outline"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer57->Add(StaticText_Outline, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer11 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer11->AddGrowableCol(0);
	Slider_Ripple_Outline = new BulkEditSliderF1(this, IDD_SLIDER_Ripple_Outline, 10, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Ripple_Outline"));
	FlexGridSizer11->Add(Slider_Ripple_Outline, 1, wxALL|wxEXPAND, 5);
	BitmapButton_Ripple_OutlineVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Ripple_Outline, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Ripple_Outline"));
	FlexGridSizer11->Add(BitmapButton_Ripple_OutlineVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer57->Add(FlexGridSizer11, 1, wxALL|wxEXPAND, 0);
	TextCtrl_Ripple_Outline = new BulkEditTextCtrlF1(this, ID_TEXTCTRL_Ripple_Outline, _("1.0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Ripple_Outline"));
	TextCtrl_Ripple_Outline->SetMaxLength(4);
	FlexGridSizer57->Add(TextCtrl_Ripple_Outline, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Ripple_Outline = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Ripple_Outline, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Ripple_Outline"));
	BitmapButton_Ripple_Outline->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer57->Add(BitmapButton_Ripple_Outline, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText72 = new wxStaticText(this, ID_STATICTEXT_Ripple_Thickness, _("Ripples"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Ripple_Thickness"));
	FlexGridSizer57->Add(StaticText72, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	Slider_Ripple_Thickness = new BulkEditSlider(this, ID_SLIDER_Ripple_Thickness, 3, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Ripple_Thickness"));
	FlexGridSizer1->Add(Slider_Ripple_Thickness, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Ripple_ThicknessVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Ripple_Thickness, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Ripple_Thickness"));
	FlexGridSizer1->Add(BitmapButton_Ripple_ThicknessVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer57->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 0);
	TextCtrl33 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Ripple_Thickness, _("3"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Ripple_Thickness"));
	TextCtrl33->SetMaxLength(3);
	FlexGridSizer57->Add(TextCtrl33, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_RippleThickness = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Ripple_Thickness, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Ripple_Thickness"));
	BitmapButton_RippleThickness->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer57->Add(BitmapButton_RippleThickness, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Spacing"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer57->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer6->AddGrowableCol(0);
	Slider_Ripple_Spacing = new BulkEditSliderF1(this, IDD_SLIDER_Ripple_Spacing, 10, 1, 400, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Ripple_Spacing"));
	FlexGridSizer6->Add(Slider_Ripple_Spacing, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Ripple_SpacingVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Ripple_Spacing, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Ripple_Spacing"));
	FlexGridSizer6->Add(BitmapButton_Ripple_SpacingVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer57->Add(FlexGridSizer6, 1, wxALL|wxEXPAND, 0);
	TextCtrl_Ripple_Spacing = new BulkEditTextCtrlF1(this, ID_TEXTCTRL_Ripple_Spacing, _("1.0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Ripple_Spacing"));
	TextCtrl_Ripple_Spacing->SetMaxLength(4);
	FlexGridSizer57->Add(TextCtrl_Ripple_Spacing, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Ripple_Spacing = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Ripple_Spacing, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Ripple_Spacing"));
	BitmapButton_Ripple_Spacing->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer57->Add(BitmapButton_Ripple_Spacing, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText176 = new wxStaticText(this, ID_STATICTEXT_Ripple_Cycles, _("Cycle Cnt"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Ripple_Cycles"));
	FlexGridSizer57->Add(StaticText176, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	Slider_Ripple_Cycles = new BulkEditSliderF1(this, IDD_SLIDER_Ripple_Cycles, 10, 0, 300, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Ripple_Cycles"));
	FlexGridSizer2->Add(Slider_Ripple_Cycles, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Ripple_CyclesVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Ripple_Cycles, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Ripple_Cycles"));
	FlexGridSizer2->Add(BitmapButton_Ripple_CyclesVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer57->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 0);
	TextCtrl34 = new BulkEditTextCtrlF1(this, ID_TEXTCTRL_Ripple_Cycles, _("1.0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Ripple_Cycles"));
	TextCtrl34->SetMaxLength(4);
	FlexGridSizer57->Add(TextCtrl34, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer57->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Points"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer57->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Ripple_Points = new BulkEditSlider(this, ID_SLIDER_RIPPLE_POINTS, 5, 3, 8, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_RIPPLE_POINTS"));
	FlexGridSizer57->Add(Slider_Ripple_Points, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Ripple_Points = new BulkEditTextCtrl(this, IDD_TEXTCTRL_RIPPLE_POINTS, _("5"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_RIPPLE_POINTS"));
	TextCtrl_Ripple_Points->SetMaxLength(1);
	FlexGridSizer57->Add(TextCtrl_Ripple_Points, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Ripple_Points = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_RIPPLE_POINTS, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_RIPPLE_POINTS"));
	BitmapButton_Ripple_Points->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer57->Add(BitmapButton_Ripple_Points, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText_Ripple_Rotation = new wxStaticText(this, ID_STATICTEXT_Ripple_Rotation, _("Rotation"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Ripple_Rotation"));
	FlexGridSizer57->Add(StaticText_Ripple_Rotation, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	Slider_Ripple_Rotation = new BulkEditSlider(this, ID_SLIDER_Ripple_Rotation, 0, -360, 360, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Ripple_Rotation"));
	FlexGridSizer3->Add(Slider_Ripple_Rotation, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Ripple_RotationVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Ripple_Rotation, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Ripple_Rotation"));
	FlexGridSizer3->Add(BitmapButton_Ripple_RotationVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer57->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 0);
	TextCtrl_Ripple_Rotation = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Ripple_Rotation, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Ripple_Rotation"));
	TextCtrl_Ripple_Rotation->SetMaxLength(3);
	FlexGridSizer57->Add(TextCtrl_Ripple_Rotation, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Ripple_Rotation = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Ripple_Rotation, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Ripple_Rotation"));
	BitmapButton_Ripple_Rotation->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer57->Add(BitmapButton_Ripple_Rotation, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT5, _("Twist"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer57->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer7 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer7->AddGrowableCol(0);
	Slider_Ripple_Twist = new BulkEditSliderF1(this, IDD_SLIDER_Ripple_Twist, 0, -450, 450, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Ripple_Twist"));
	FlexGridSizer7->Add(Slider_Ripple_Twist, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Ripple_TwistVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Ripple_Twist, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Ripple_Twist"));
	FlexGridSizer7->Add(BitmapButton_Ripple_TwistVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer57->Add(FlexGridSizer7, 1, wxALL|wxEXPAND, 0);
	TextCtrl_Ripple_Twist = new BulkEditTextCtrlF1(this, ID_TEXTCTRL_Ripple_Twist, _("0.0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Ripple_Twist"));
	TextCtrl_Ripple_Twist->SetMaxLength(4);
	FlexGridSizer57->Add(TextCtrl_Ripple_Twist, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Ripple_Twist = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Ripple_Twist, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Ripple_Twist"));
	BitmapButton_Ripple_Twist->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer57->Add(BitmapButton_Ripple_Twist, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("X Center"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer57->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer4->AddGrowableCol(0);
	Slider_Ripple_XC = new BulkEditSlider(this, ID_SLIDER_Ripple_XC, 0, -100, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Ripple_XC"));
	FlexGridSizer4->Add(Slider_Ripple_XC, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Ripple_XCVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Ripple_XC, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Ripple_XC"));
	FlexGridSizer4->Add(BitmapButton_Ripple_XCVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer57->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 0);
	TextCtrl_Ripple_XC = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Ripple_XC, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Ripple_XC"));
	TextCtrl_Ripple_XC->SetMaxLength(3);
	FlexGridSizer57->Add(TextCtrl_Ripple_XC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer57->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Y Center"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer57->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer5->AddGrowableCol(0);
	Slider_Ripple_YC = new BulkEditSlider(this, ID_SLIDER_Ripple_YC, 0, -100, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Ripple_YC"));
	FlexGridSizer5->Add(Slider_Ripple_YC, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Ripple_YCVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Ripple_YC, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Ripple_YC"));
	FlexGridSizer5->Add(BitmapButton_Ripple_YCVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer57->Add(FlexGridSizer5, 1, wxALL|wxEXPAND, 0);
	TextCtrl_Ripple_YC = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Ripple_YC, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Ripple_YC"));
	TextCtrl_Ripple_YC->SetMaxLength(3);
	FlexGridSizer57->Add(TextCtrl_Ripple_YC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer57->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText8 = new wxStaticText(this, ID_STATICTEXT7, _("Velocity"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	FlexGridSizer57->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer9 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer9->AddGrowableCol(0);
	Slider_Ripple_Velocity = new BulkEditSliderF1(this, IDD_SLIDER_Ripple_Velocity, 0, 0, 300, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Ripple_Velocity"));
	FlexGridSizer9->Add(Slider_Ripple_Velocity, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Ripple_VelocityVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Ripple_Velocity, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Ripple_Velocity"));
	FlexGridSizer9->Add(BitmapButton_Ripple_VelocityVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer57->Add(FlexGridSizer9, 1, wxALL|wxEXPAND, 0);
	TextCtrl_Ripple_Velocity = new BulkEditTextCtrlF1(this, ID_TEXTCTRL_Ripple_Velocity, _("0.0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Ripple_Velocity"));
	TextCtrl_Ripple_Velocity->SetMaxLength(4);
	FlexGridSizer57->Add(TextCtrl_Ripple_Velocity, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Ripple_Velocity = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Ripple_Velocity, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Ripple_Velocity"));
	BitmapButton_Ripple_Velocity->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer57->Add(BitmapButton_Ripple_Velocity, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText9 = new wxStaticText(this, ID_STATICTEXT8, _("Direction"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
	FlexGridSizer57->Add(StaticText9, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer10 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer10->AddGrowableCol(0);
	Slider_Ripple_Direction = new BulkEditSlider(this, ID_SLIDER_Ripple_Direction, 0, -360, 360, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Ripple_Direction"));
	FlexGridSizer10->Add(Slider_Ripple_Direction, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Ripple_DirectionVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Ripple_Direction, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Ripple_Direction"));
	FlexGridSizer10->Add(BitmapButton_Ripple_DirectionVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer57->Add(FlexGridSizer10, 1, wxALL|wxEXPAND, 0);
	TextCtrl_Ripple_Direction = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Ripple_Direction, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Ripple_Direction"));
	TextCtrl_Ripple_Direction->SetMaxLength(3);
	FlexGridSizer57->Add(TextCtrl_Ripple_Direction, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Ripple_Direction = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Ripple_Direction, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Ripple_Direction"));
	BitmapButton_Ripple_Direction->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer57->Add(BitmapButton_Ripple_Direction, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer57->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer60 = new wxFlexGridSizer(0, 4, 0, 0);
	CheckBox_Ripple3D = new BulkEditCheckBox(this, ID_CHECKBOX_Ripple3D, _("3D"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Ripple3D"));
	CheckBox_Ripple3D->SetValue(false);
	FlexGridSizer60->Add(CheckBox_Ripple3D, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Ripple3D = new xlLockButton(this, ID_BITMAPBUTTON_CHECKBOX_Ripple3D, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Ripple3D"));
	BitmapButton_Ripple3D->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer60->Add(BitmapButton_Ripple3D, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer57->Add(FlexGridSizer60, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer57->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer57->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer57);
	FlexGridSizer57->Fit(this);
	FlexGridSizer57->SetSizeHints(this);

	Connect(ID_CHOICE_Ripple_Object_To_Draw,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&RipplePanel::OnChoice_Ripple_Object_To_DrawSelect);
	Connect(ID_BITMAPBUTTON_CHOICE_Ripple_Movement,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&RipplePanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Ripple_Cycles,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&RipplePanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_RIPPLE_POINTS,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&RipplePanel::OnLockButtonClick);
	//*)
    SetName("ID_PANEL_RIPPLE");

	Connect(ID_BITMAPBUTTON_SLIDER_Ripple_Thickness, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&RipplePanel::OnLockButtonClick);
    Connect(ID_VALUECURVE_Ripple_Thickness, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&RipplePanel::OnVCButtonClick);
    Connect(ID_BITMAPBUTTON_CHECKBOX_Ripple3D, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&RipplePanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHOICE_Ripple_Object_To_Draw, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&RipplePanel::OnLockButtonClick);
    Connect(ID_BITMAPBUTTON_CHOICE_Ripple_Draw_Style, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&RipplePanel::OnLockButtonClick);
    Connect(ID_CHOICE_Ripple_Draw_Style, wxEVT_COMMAND_CHOICE_SELECTED, (wxObjectEventFunction)&RipplePanel::OnChoice_Ripple_Draw_StyleSelect);
    Connect(ID_BITMAPBUTTON_SLIDER_Ripple_Rotation, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&RipplePanel::OnLockButtonClick);

    Connect(ID_BITMAPBUTTON_SLIDER_Ripple_Scale, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&RipplePanel::OnLockButtonClick);
    Connect(ID_VALUECURVE_Ripple_Scale, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&RipplePanel::OnVCButtonClick);
    Connect(ID_BITMAPBUTTON_SLIDER_Ripple_Spacing, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&RipplePanel::OnLockButtonClick);
    Connect(ID_VALUECURVE_Ripple_Spacing, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&RipplePanel::OnVCButtonClick);
    Connect(ID_BITMAPBUTTON_SLIDER_Ripple_Velocity, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&RipplePanel::OnLockButtonClick);
    Connect(ID_VALUECURVE_Ripple_Velocity, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&RipplePanel::OnVCButtonClick);
    Connect(ID_BITMAPBUTTON_SLIDER_Ripple_Direction, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&RipplePanel::OnLockButtonClick);
    Connect(ID_VALUECURVE_Ripple_Direction, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&RipplePanel::OnVCButtonClick);
    Connect(ID_BITMAPBUTTON_SLIDER_Ripple_Twist, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&RipplePanel::OnLockButtonClick);
    Connect(ID_VALUECURVE_Ripple_Twist, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&RipplePanel::OnVCButtonClick);
    Connect(ID_BITMAPBUTTON_SLIDER_Ripple_Outline, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&RipplePanel::OnLockButtonClick);
    Connect(ID_VALUECURVE_Ripple_Outline, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&RipplePanel::OnVCButtonClick);

	Connect(ID_VALUECURVE_Ripple_XC, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&RipplePanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_Ripple_YC, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&RipplePanel::OnVCButtonClick);
    Connect(ID_VALUECURVE_Ripple_Rotation, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&RipplePanel::OnVCButtonClick);

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&RipplePanel::OnVCChanged, 0, this);
    Connect(wxID_ANY, EVT_VALIDATEWINDOW, (wxObjectEventFunction)&RipplePanel::OnValidateWindow, 0, this);

	BitmapButton_Ripple_XCVC->SetLimits(RIPPLE_XC_MIN, RIPPLE_XC_MAX);
	BitmapButton_Ripple_YCVC->SetLimits(RIPPLE_YC_MIN, RIPPLE_YC_MAX);
    BitmapButton_Ripple_RotationVC->SetLimits(RIPPLE_ROTATION_MIN, RIPPLE_ROTATION_MAX);
	BitmapButton_Ripple_CyclesVC->SetLimits(RIPPLE_CYCLES_MIN, RIPPLE_CYCLES_MAX);
    BitmapButton_Ripple_CyclesVC->GetValue()->SetDivisor(RIPPLE_CYCLES_DIVISOR);
    BitmapButton_Ripple_ThicknessVC->SetLimits(RIPPLE_THICKNESS_MIN, RIPPLE_THICKNESS_MAX);
    BitmapButton_Ripple_ScaleVC->SetLimits(RIPPLE_SCALE_MIN, RIPPLE_SCALE_MAX);
    BitmapButton_Ripple_SpacingVC->SetLimits(RIPPLE_SPACING_MIN, RIPPLE_SPACING_MAX);
    BitmapButton_Ripple_SpacingVC->GetValue()->SetDivisor(RIPPLE_SPACING_DIVISOR);
    BitmapButton_Ripple_OutlineVC->SetLimits(RIPPLE_OUTLINE_MIN, RIPPLE_OUTLINE_MAX);
    BitmapButton_Ripple_OutlineVC->GetValue()->SetDivisor(RIPPLE_OUTLINE_DIVISOR);
    BitmapButton_Ripple_TwistVC->SetLimits(RIPPLE_TWIST_MIN, RIPPLE_TWIST_MAX);
    BitmapButton_Ripple_TwistVC->GetValue()->SetDivisor(RIPPLE_TWIST_DIVISOR);
    BitmapButton_Ripple_VelocityVC->SetLimits(RIPPLE_VELOCITY_MIN, RIPPLE_VELOCITY_MAX);
    BitmapButton_Ripple_VelocityVC->GetValue()->SetDivisor(RIPPLE_VELOCITY_DIVISOR);
    BitmapButton_Ripple_DirectionVC->SetLimits(RIPPLE_DIRECTION_MIN, RIPPLE_DIRECTION_MAX);

    ValidateWindow();
}

RipplePanel::~RipplePanel()
{
	//(*Destroy(RipplePanel)
	//*)
}

void RipplePanel::OnChoice_Ripple_Object_To_DrawSelect(wxCommandEvent& event)
{
    ValidateWindow();
}

void RipplePanel::ValidateWindow()
{
    if (Choice_Ripple_Object_To_Draw->GetStringSelection() == "Star" || Choice_Ripple_Object_To_Draw->GetStringSelection() == "Snow Flake" || Choice_Ripple_Object_To_Draw->GetStringSelection() == "Polygon")
    {
        Slider_Ripple_Points->Enable();
        TextCtrl_Ripple_Points->Enable();
        BitmapButton_Ripple_Points->Enable();

		BitmapButton_Ripple_Rotation->Enable();
		Slider_Ripple_Rotation->Enable();
		TextCtrl_Ripple_Rotation->Enable();
        BitmapButton_Ripple_RotationVC->Enable();
    }
    else
    {
        Slider_Ripple_Points->Disable();
        TextCtrl_Ripple_Points->Disable();
        BitmapButton_Ripple_Points->Disable();

		BitmapButton_Ripple_Rotation->Disable();
		Slider_Ripple_Rotation->Disable();
		TextCtrl_Ripple_Rotation->Disable();
        BitmapButton_Ripple_RotationVC->Disable();
    }

    bool newFeatures = false;
    if (Choice_Ripple_Draw_Style->GetStringSelection() != "Old") {
        BitmapButton_Ripple_Rotation->Enable();
        Slider_Ripple_Rotation->Enable();
        TextCtrl_Ripple_Rotation->Enable();
        BitmapButton_Ripple_RotationVC->Enable();

        newFeatures = true;
    }

	if (!newFeatures && Choice_Ripple_Object_To_Draw->GetStringSelection() == "SVG") {
        Choice_Ripple_Object_To_Draw->SetStringSelection("Circle");
    }

	if (newFeatures && Choice_Ripple_Object_To_Draw->GetStringSelection() == "SVG") {
        FilePickerCtrl_Ripple_SVG->Enable();
    } else {
        FilePickerCtrl_Ripple_SVG->Enable(false);
        FilePickerCtrl_Ripple_SVG->SetFileName(wxFileName(""));
    }

	BitmapButton_Ripple_Scale->Enable(newFeatures);
    Slider_Ripple_Scale->Enable(newFeatures);
    TextCtrl_Ripple_Scale->Enable(newFeatures);
    BitmapButton_Ripple_ScaleVC->Enable(newFeatures);

    BitmapButton_Ripple_Spacing->Enable(newFeatures);
    Slider_Ripple_Spacing->Enable(newFeatures);
    TextCtrl_Ripple_Spacing->Enable(newFeatures);
    BitmapButton_Ripple_SpacingVC->Enable(newFeatures);

    BitmapButton_Ripple_Twist->Enable(newFeatures);
    Slider_Ripple_Twist->Enable(newFeatures);
    TextCtrl_Ripple_Twist->Enable(newFeatures);
    BitmapButton_Ripple_TwistVC->Enable(newFeatures);

    BitmapButton_Ripple_Direction->Enable(newFeatures);
    Slider_Ripple_Direction->Enable(newFeatures);
    TextCtrl_Ripple_Direction->Enable(newFeatures);
    BitmapButton_Ripple_DirectionVC->Enable(newFeatures);

    BitmapButton_Ripple_Velocity->Enable(newFeatures);
    Slider_Ripple_Velocity->Enable(newFeatures);
    TextCtrl_Ripple_Velocity->Enable(newFeatures);
    BitmapButton_Ripple_VelocityVC->Enable(newFeatures);

	BitmapButton_Ripple_Outline->Enable(newFeatures);
    Slider_Ripple_Outline->Enable(newFeatures);
    TextCtrl_Ripple_Outline->Enable(newFeatures);
    BitmapButton_Ripple_OutlineVC->Enable(newFeatures);
}

void RipplePanel::OnChoice_Ripple_Draw_StyleSelect(wxCommandEvent& event)
{
    ValidateWindow();
}

