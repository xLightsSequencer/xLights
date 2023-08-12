/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "SingleStrandPanel.h"
#include "EffectPanelUtils.h"
#include "SingleStrandEffect.h"

//(*InternalHeaders(SingleStrandPanel)
#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
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

#define XLIGHTS_FX
#include "FX.h"

//(*IdInit(SingleStrandPanel)
const long SingleStrandPanel::ID_STATICTEXT_SingleStrand_Colors = wxNewId();
const long SingleStrandPanel::ID_CHOICE_SingleStrand_Colors = wxNewId();
const long SingleStrandPanel::ID_BITMAPBUTTON_CHOICE_SingleStrand_Colors = wxNewId();
const long SingleStrandPanel::ID_STATICTEXT_Number_Chases = wxNewId();
const long SingleStrandPanel::ID_SLIDER_Number_Chases = wxNewId();
const long SingleStrandPanel::ID_VALUECURVE_Number_Chases = wxNewId();
const long SingleStrandPanel::IDD_TEXTCTRL_Number_Chases = wxNewId();
const long SingleStrandPanel::ID_BITMAPBUTTON_SLIDER_Number_Chases = wxNewId();
const long SingleStrandPanel::ID_STATICTEXT_Color_Mix1 = wxNewId();
const long SingleStrandPanel::ID_SLIDER_Color_Mix1 = wxNewId();
const long SingleStrandPanel::ID_VALUECURVE_Color_Mix1 = wxNewId();
const long SingleStrandPanel::IDD_TEXTCTRL_Color_Mix1 = wxNewId();
const long SingleStrandPanel::ID_BITMAPBUTTON_SLIDER_Color_Mix1 = wxNewId();
const long SingleStrandPanel::ID_STATICTEXT_Chase_Rotations = wxNewId();
const long SingleStrandPanel::IDD_SLIDER_Chase_Rotations = wxNewId();
const long SingleStrandPanel::ID_VALUECURVE_Chase_Rotations = wxNewId();
const long SingleStrandPanel::ID_TEXTCTRL_Chase_Rotations = wxNewId();
const long SingleStrandPanel::ID_BITMAPBUTTON_SLIDER_Chase_Rotations = wxNewId();
const long SingleStrandPanel::ID_STATICTEXT5 = wxNewId();
const long SingleStrandPanel::IDD_SLIDER_Chase_Offset = wxNewId();
const long SingleStrandPanel::ID_VALUECURVE_Chase_Offset = wxNewId();
const long SingleStrandPanel::ID_TEXTCTRL_Chase_Offset = wxNewId();
const long SingleStrandPanel::ID_BITMAPBUTTON_SLIDER_Chase_Offset = wxNewId();
const long SingleStrandPanel::ID_STATICTEXT_Chase_Type1 = wxNewId();
const long SingleStrandPanel::ID_CHOICE_Chase_Type1 = wxNewId();
const long SingleStrandPanel::ID_BITMAPBUTTON_CHOICE_Chase_Type1 = wxNewId();
const long SingleStrandPanel::ID_CHECKBOX_Chase_3dFade1 = wxNewId();
const long SingleStrandPanel::ID_BITMAPBUTTON_CHECKBOX_Chase_3dFade1 = wxNewId();
const long SingleStrandPanel::ID_CHECKBOX_Chase_Group_All = wxNewId();
const long SingleStrandPanel::ID_BITMAPBUTTON_CHECKBOX_Chase_Group_All = wxNewId();
const long SingleStrandPanel::ID_PANEL3 = wxNewId();
const long SingleStrandPanel::ID_STATICTEXT_Skips_BandSize = wxNewId();
const long SingleStrandPanel::ID_SLIDER_Skips_BandSize = wxNewId();
const long SingleStrandPanel::IDD_TEXTCTRL_Skips_BandSize = wxNewId();
const long SingleStrandPanel::ID_BITMAPBUTTON_SLIDER_Skips_BandSize = wxNewId();
const long SingleStrandPanel::ID_STATICTEXT_Skips_SkipSize = wxNewId();
const long SingleStrandPanel::ID_SLIDER_Skips_SkipSize = wxNewId();
const long SingleStrandPanel::IDD_TEXTCTRL_Skips_SkipSize = wxNewId();
const long SingleStrandPanel::ID_BITMAPBUTTON_SLIDER_Skips_SkipSize = wxNewId();
const long SingleStrandPanel::ID_STATICTEXT_Skips_StartPos = wxNewId();
const long SingleStrandPanel::ID_SLIDER_Skips_StartPos = wxNewId();
const long SingleStrandPanel::IDD_TEXTCTRL_Skips_StartPos = wxNewId();
const long SingleStrandPanel::ID_BITMAPBUTTON_SLIDER_Skips_StartPos = wxNewId();
const long SingleStrandPanel::ID_STATICTEXT_Skips_Advance = wxNewId();
const long SingleStrandPanel::ID_SLIDER_Skips_Advance = wxNewId();
const long SingleStrandPanel::IDD_TEXTCTRL_Skips_Advance = wxNewId();
const long SingleStrandPanel::ID_BITMAPBUTTON_SLIDER_Skips_Advance = wxNewId();
const long SingleStrandPanel::ID_STATICTEXT_Skips_Direction = wxNewId();
const long SingleStrandPanel::ID_CHOICE_Skips_Direction = wxNewId();
const long SingleStrandPanel::ID_BITMAPBUTTON_CHOICE_Skips_Direction = wxNewId();
const long SingleStrandPanel::ID_PANEL21 = wxNewId();
const long SingleStrandPanel::ID_STATICTEXT2 = wxNewId();
const long SingleStrandPanel::ID_CHOICE_SingleStrand_FX = wxNewId();
const long SingleStrandPanel::ID_STATICTEXT4 = wxNewId();
const long SingleStrandPanel::ID_CHOICE_SingleStrand_FX_Palette = wxNewId();
const long SingleStrandPanel::ID_STATICTEXT1 = wxNewId();
const long SingleStrandPanel::ID_SLIDER_FX_Intensity = wxNewId();
const long SingleStrandPanel::ID_VALUECURVE_FX_Intensity = wxNewId();
const long SingleStrandPanel::IDD_TEXTCTRL_FX_Intensity = wxNewId();
const long SingleStrandPanel::ID_STATICTEXT3 = wxNewId();
const long SingleStrandPanel::ID_SLIDER_FX_Speed = wxNewId();
const long SingleStrandPanel::ID_VALUECURVE_FX_Speed = wxNewId();
const long SingleStrandPanel::IDD_TEXTCTRL_FX_Speed = wxNewId();
const long SingleStrandPanel::ID_PANEL1 = wxNewId();
const long SingleStrandPanel::ID_NOTEBOOK_SSEFFECT_TYPE = wxNewId();
//*)

BEGIN_EVENT_TABLE(SingleStrandPanel,wxPanel)
	//(*EventTable(SingleStrandPanel)
	//*)
END_EVENT_TABLE()

SingleStrandPanel::SingleStrandPanel(wxWindow* parent) : xlEffectPanel(parent)
{
	//(*Initialize(SingleStrandPanel)
	BulkEditTextCtrl* TextCtrl3;
	BulkEditTextCtrl* TextCtrl42;
	BulkEditTextCtrl* TextCtrl44;
	BulkEditTextCtrl* TextCtrl45;
	BulkEditTextCtrl* TextCtrl4;
	BulkEditTextCtrl* TextCtrl5;
	BulkEditTextCtrl* TextCtrl_FX_Intensity;
	BulkEditTextCtrl* TextCtrl_FX_Speed;
	BulkEditTextCtrlF1* TextCtrl43;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer21;
	wxFlexGridSizer* FlexGridSizer24;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer6;
	wxFlexGridSizer* FlexGridSizer79;
	wxFlexGridSizer* FlexGridSizer7;
	wxFlexGridSizer* FlexGridSizer81;
	wxFlexGridSizer* FlexGridSizer82;
	wxFlexGridSizer* FlexGridSizer83;
	wxFlexGridSizer* FlexGridSizer84;
	wxFlexGridSizer* FlexGridSizer89;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer89 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer89->AddGrowableCol(0);
	SingleStrandEffectType = new wxNotebook(this, ID_NOTEBOOK_SSEFFECT_TYPE, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK_SSEFFECT_TYPE"));
	Panel1 = new wxPanel(SingleStrandEffectType, ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL3"));
	FlexGridSizer24 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer24->AddGrowableCol(1);
	StaticText61 = new wxStaticText(Panel1, ID_STATICTEXT_SingleStrand_Colors, _("Colors  "), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_SingleStrand_Colors"));
	FlexGridSizer24->Add(StaticText61, 1, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_SingleStrand_Colors = new BulkEditChoice(Panel1, ID_CHOICE_SingleStrand_Colors, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_SingleStrand_Colors"));
	Choice_SingleStrand_Colors->Append(_("Rainbow"));
	Choice_SingleStrand_Colors->SetSelection( Choice_SingleStrand_Colors->Append(_("Palette")) );
	FlexGridSizer24->Add(Choice_SingleStrand_Colors, 1, wxALIGN_LEFT|wxALIGN_TOP, 5);
	FlexGridSizer24->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_SingleStrand_Colors = new xlLockButton(Panel1, ID_BITMAPBUTTON_CHOICE_SingleStrand_Colors, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_SingleStrand_Colors"));
	BitmapButton_SingleStrand_Colors->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer24->Add(BitmapButton_SingleStrand_Colors, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText60 = new wxStaticText(Panel1, ID_STATICTEXT_Number_Chases, _("Number Chases"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Number_Chases"));
	FlexGridSizer24->Add(StaticText60, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	Slider_Number_Chases = new BulkEditSlider(Panel1, ID_SLIDER_Number_Chases, 1, 1, 20, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Number_Chases"));
	Slider_Number_Chases->SetSelection(1, 20);
	FlexGridSizer1->Add(Slider_Number_Chases, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Number_ChasesVC = new BulkEditValueCurveButton(Panel1, ID_VALUECURVE_Number_Chases, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Number_Chases"));
	FlexGridSizer1->Add(BitmapButton_Number_ChasesVC, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer24->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 0);
	TextCtrl44 = new BulkEditTextCtrl(Panel1, IDD_TEXTCTRL_Number_Chases, _("1"), wxDefaultPosition, wxDLG_UNIT(Panel1,wxSize(30,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Number_Chases"));
	TextCtrl44->SetMaxLength(3);
	FlexGridSizer24->Add(TextCtrl44, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Number_Chases = new xlLockButton(Panel1, ID_BITMAPBUTTON_SLIDER_Number_Chases, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Number_Chases"));
	BitmapButton_Number_Chases->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer24->Add(BitmapButton_Number_Chases, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText9 = new wxStaticText(Panel1, ID_STATICTEXT_Color_Mix1, _("Chase Size"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Color_Mix1"));
	FlexGridSizer24->Add(StaticText9, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	Slider_Color_Mix1 = new BulkEditSlider(Panel1, ID_SLIDER_Color_Mix1, 10, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Color_Mix1"));
	FlexGridSizer2->Add(Slider_Color_Mix1, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Color_Mix1VC = new BulkEditValueCurveButton(Panel1, ID_VALUECURVE_Color_Mix1, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Color_Mix1"));
	FlexGridSizer2->Add(BitmapButton_Color_Mix1VC, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer24->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 0);
	TextCtrl45 = new BulkEditTextCtrl(Panel1, IDD_TEXTCTRL_Color_Mix1, _("10"), wxDefaultPosition, wxDLG_UNIT(Panel1,wxSize(30,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Color_Mix1"));
	TextCtrl45->SetMaxLength(4);
	FlexGridSizer24->Add(TextCtrl45, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Color_Mix1 = new xlLockButton(Panel1, ID_BITMAPBUTTON_SLIDER_Color_Mix1, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Color_Mix1"));
	BitmapButton_Color_Mix1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer24->Add(BitmapButton_Color_Mix1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText10 = new wxStaticText(Panel1, ID_STATICTEXT_Chase_Rotations, _("Cycles"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Chase_Rotations"));
	FlexGridSizer24->Add(StaticText10, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	Slider_Chase_Rotations = new BulkEditSliderF1(Panel1, IDD_SLIDER_Chase_Rotations, 10, 1, 500, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Chase_Rotations"));
	FlexGridSizer3->Add(Slider_Chase_Rotations, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Chase_Rotations = new BulkEditValueCurveButton(Panel1, ID_VALUECURVE_Chase_Rotations, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Chase_Rotations"));
	FlexGridSizer3->Add(BitmapButton_Chase_Rotations, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer24->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 0);
	TextCtrl43 = new BulkEditTextCtrlF1(Panel1, ID_TEXTCTRL_Chase_Rotations, _("1.0"), wxDefaultPosition, wxDLG_UNIT(Panel1,wxSize(30,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Chase_Rotations"));
	TextCtrl43->SetMaxLength(4);
	FlexGridSizer24->Add(TextCtrl43, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_ChaseRotations = new xlLockButton(Panel1, ID_BITMAPBUTTON_SLIDER_Chase_Rotations, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Chase_Rotations"));
	BitmapButton_ChaseRotations->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer24->Add(BitmapButton_ChaseRotations, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText5 = new wxStaticText(Panel1, ID_STATICTEXT5, _("Offset"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer24->Add(StaticText5, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer7 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer7->AddGrowableCol(0);
	Slider_Chase_Offset = new BulkEditSliderF1(Panel1, IDD_SLIDER_Chase_Offset, 0, -5000, 5000, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Chase_Offset"));
	FlexGridSizer7->Add(Slider_Chase_Offset, 1, wxALL|wxEXPAND, 2);
    BitmapButton_Chase_OffsetVC = new BulkEditValueCurveButton(Panel1, ID_VALUECURVE_Chase_Offset, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW | wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Chase_Offset"));
	FlexGridSizer7->Add(BitmapButton_Chase_OffsetVC, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer24->Add(FlexGridSizer7, 1, wxALL|wxEXPAND, 0);
	TextCtrl_ChaseOffset = new BulkEditTextCtrlF1(Panel1, ID_TEXTCTRL_Chase_Offset, _("0.0"), wxDefaultPosition, wxDLG_UNIT(Panel1,wxSize(30,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Chase_Offset"));
	TextCtrl_ChaseOffset->SetMaxLength(6);
	FlexGridSizer24->Add(TextCtrl_ChaseOffset, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Chase_Offset = new xlLockButton(Panel1, ID_BITMAPBUTTON_SLIDER_Chase_Offset, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Chase_Offset"));
	BitmapButton_Chase_Offset->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer24->Add(BitmapButton_Chase_Offset, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText11 = new wxStaticText(Panel1, ID_STATICTEXT_Chase_Type1, _("Chase Types"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Chase_Type1"));
	FlexGridSizer24->Add(StaticText11, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Chase_Type1 = new BulkEditChoice(Panel1, ID_CHOICE_Chase_Type1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Chase_Type1"));
	Choice_Chase_Type1->SetSelection( Choice_Chase_Type1->Append(_("Left-Right")) );
	Choice_Chase_Type1->Append(_("Right-Left"));
	Choice_Chase_Type1->Append(_("Bounce from Left"));
	Choice_Chase_Type1->Append(_("Bounce from Right"));
	Choice_Chase_Type1->Append(_("Dual Chase"));
	Choice_Chase_Type1->Append(_("From Middle"));
	Choice_Chase_Type1->Append(_("To Middle"));
	Choice_Chase_Type1->Append(_("Bounce to Middle"));
	Choice_Chase_Type1->Append(_("Bounce from Middle"));
	Choice_Chase_Type1->Append(_("Static Left-Right"));
	Choice_Chase_Type1->Append(_("Static Right-Left"));
	Choice_Chase_Type1->Append(_("Static Dual"));
	Choice_Chase_Type1->Append(_("Static From Middle"));
	Choice_Chase_Type1->Append(_("Static To Middle"));
	Choice_Chase_Type1->Append(_("Static Double-Ended"));
	FlexGridSizer24->Add(Choice_Chase_Type1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer24->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Chase_Type1 = new xlLockButton(Panel1, ID_BITMAPBUTTON_CHOICE_Chase_Type1, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Chase_Type1"));
	BitmapButton_Chase_Type1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer24->Add(BitmapButton_Chase_Type1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer24->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Chase_3dFade1 = new BulkEditCheckBox(Panel1, ID_CHECKBOX_Chase_3dFade1, _("3d Fade"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Chase_3dFade1"));
	CheckBox_Chase_3dFade1->SetValue(false);
	FlexGridSizer24->Add(CheckBox_Chase_3dFade1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer24->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Chase_3dFade1 = new xlLockButton(Panel1, ID_BITMAPBUTTON_CHECKBOX_Chase_3dFade1, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Chase_3dFade1"));
	BitmapButton_Chase_3dFade1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer24->Add(BitmapButton_Chase_3dFade1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer24->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Chase_Group_All = new BulkEditCheckBox(Panel1, ID_CHECKBOX_Chase_Group_All, _("Group All Arches"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Chase_Group_All"));
	CheckBox_Chase_Group_All->SetValue(false);
	FlexGridSizer24->Add(CheckBox_Chase_Group_All, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer24->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Chase_Group_All = new xlLockButton(Panel1, ID_BITMAPBUTTON_CHECKBOX_Chase_Group_All, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Chase_Group_All"));
	BitmapButton_Chase_Group_All->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer24->Add(BitmapButton_Chase_Group_All, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	Panel1->SetSizer(FlexGridSizer24);
	FlexGridSizer24->Fit(Panel1);
	FlexGridSizer24->SetSizeHints(Panel1);
	Panel2 = new wxPanel(SingleStrandEffectType, ID_PANEL21, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL21"));
	FlexGridSizer79 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer79->AddGrowableCol(0);
	FlexGridSizer81 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer81->AddGrowableCol(1);
	StaticText101 = new wxStaticText(Panel2, ID_STATICTEXT_Skips_BandSize, _("Band size"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Skips_BandSize"));
	FlexGridSizer81->Add(StaticText101, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Skips_BandSize = new BulkEditSlider(Panel2, ID_SLIDER_Skips_BandSize, 1, 1, 20, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Skips_BandSize"));
	FlexGridSizer81->Add(Slider_Skips_BandSize, 1, wxALL|wxEXPAND, 5);
	TextCtrl3 = new BulkEditTextCtrl(Panel2, IDD_TEXTCTRL_Skips_BandSize, _("1"), wxDefaultPosition, wxDLG_UNIT(Panel2,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Skips_BandSize"));
	TextCtrl3->SetMaxLength(3);
	FlexGridSizer81->Add(TextCtrl3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Skips_BandSize = new xlLockButton(Panel2, ID_BITMAPBUTTON_SLIDER_Skips_BandSize, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Skips_BandSize"));
	BitmapButton_Skips_BandSize->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer81->Add(BitmapButton_Skips_BandSize, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer79->Add(FlexGridSizer81, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer82 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer82->AddGrowableCol(1);
	StaticText105 = new wxStaticText(Panel2, ID_STATICTEXT_Skips_SkipSize, _("Skip size"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Skips_SkipSize"));
	FlexGridSizer82->Add(StaticText105, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Skips_SkipSize = new BulkEditSlider(Panel2, ID_SLIDER_Skips_SkipSize, 1, 0, 20, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Skips_SkipSize"));
	FlexGridSizer82->Add(Slider_Skips_SkipSize, 1, wxALL|wxEXPAND, 5);
	TextCtrl4 = new BulkEditTextCtrl(Panel2, IDD_TEXTCTRL_Skips_SkipSize, _("1"), wxDefaultPosition, wxDLG_UNIT(Panel2,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Skips_SkipSize"));
	TextCtrl4->SetMaxLength(3);
	FlexGridSizer82->Add(TextCtrl4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Skips_SkipSize = new xlLockButton(Panel2, ID_BITMAPBUTTON_SLIDER_Skips_SkipSize, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Skips_SkipSize"));
	BitmapButton_Skips_SkipSize->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer82->Add(BitmapButton_Skips_SkipSize, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer79->Add(FlexGridSizer82, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer83 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer83->AddGrowableCol(1);
	StaticText106 = new wxStaticText(Panel2, ID_STATICTEXT_Skips_StartPos, _("Starting Position"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Skips_StartPos"));
	FlexGridSizer83->Add(StaticText106, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Skips_StartPos = new BulkEditSlider(Panel2, ID_SLIDER_Skips_StartPos, 1, 1, 20, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Skips_StartPos"));
	FlexGridSizer83->Add(Slider_Skips_StartPos, 1, wxALL|wxEXPAND, 5);
	TextCtrl5 = new BulkEditTextCtrl(Panel2, IDD_TEXTCTRL_Skips_StartPos, _("1"), wxDefaultPosition, wxDLG_UNIT(Panel2,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Skips_StartPos"));
	TextCtrl5->SetMaxLength(3);
	FlexGridSizer83->Add(TextCtrl5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Skips_StartingPosition = new xlLockButton(Panel2, ID_BITMAPBUTTON_SLIDER_Skips_StartPos, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Skips_StartPos"));
	BitmapButton_Skips_StartingPosition->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer83->Add(BitmapButton_Skips_StartingPosition, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer79->Add(FlexGridSizer83, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer21 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer21->AddGrowableCol(1);
	StaticText179 = new wxStaticText(Panel2, ID_STATICTEXT_Skips_Advance, _("Number of Advances"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Skips_Advance"));
	FlexGridSizer21->Add(StaticText179, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Skips_Advance = new BulkEditSlider(Panel2, ID_SLIDER_Skips_Advance, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Skips_Advance"));
	FlexGridSizer21->Add(Slider_Skips_Advance, 1, wxALL|wxEXPAND, 5);
	TextCtrl42 = new BulkEditTextCtrl(Panel2, IDD_TEXTCTRL_Skips_Advance, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel2,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Skips_Advance"));
	TextCtrl42->SetMaxLength(3);
	FlexGridSizer21->Add(TextCtrl42, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_SkipsAdvance = new xlLockButton(Panel2, ID_BITMAPBUTTON_SLIDER_Skips_Advance, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Skips_Advance"));
	BitmapButton_SkipsAdvance->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer21->Add(BitmapButton_SkipsAdvance, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer79->Add(FlexGridSizer21, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer84 = new wxFlexGridSizer(0, 3, 0, 0);
	StaticText109 = new wxStaticText(Panel2, ID_STATICTEXT_Skips_Direction, _("Direction"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Skips_Direction"));
	FlexGridSizer84->Add(StaticText109, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Skips_Direction = new BulkEditChoice(Panel2, ID_CHOICE_Skips_Direction, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Skips_Direction"));
	Choice_Skips_Direction->SetSelection( Choice_Skips_Direction->Append(_("Left")) );
	Choice_Skips_Direction->Append(_("Right"));
	Choice_Skips_Direction->Append(_("From Middle"));
	Choice_Skips_Direction->Append(_("To Middle"));
	FlexGridSizer84->Add(Choice_Skips_Direction, 1, wxALL|wxEXPAND, 5);
	BitmapButton_Skips_Direction = new xlLockButton(Panel2, ID_BITMAPBUTTON_CHOICE_Skips_Direction, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Skips_Direction"));
	BitmapButton_Skips_Direction->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer84->Add(BitmapButton_Skips_Direction, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer79->Add(FlexGridSizer84, 1, wxALL|wxEXPAND, 2);
	Panel2->SetSizer(FlexGridSizer79);
	FlexGridSizer79->Fit(Panel2);
	FlexGridSizer79->SetSizeHints(Panel2);
	Panel_FX = new wxPanel(SingleStrandEffectType, ID_PANEL1, wxPoint(104,13), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	FlexGridSizer4 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer4->AddGrowableCol(1);
	StaticText2 = new wxStaticText(Panel_FX, ID_STATICTEXT2, _("FX"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer4->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_SingleStrand_FX = new BulkEditChoice(Panel_FX, ID_CHOICE_SingleStrand_FX, wxDefaultPosition, wxDefaultSize, 0, 0, wxCB_SORT, wxDefaultValidator, _T("ID_CHOICE_SingleStrand_FX"));
	FlexGridSizer4->Add(Choice_SingleStrand_FX, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer4->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText4 = new wxStaticText(Panel_FX, ID_STATICTEXT4, _("Palette"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer4->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_FX_Palette = new BulkEditChoice(Panel_FX, ID_CHOICE_SingleStrand_FX_Palette, wxDefaultPosition, wxDefaultSize, 0, 0, wxCB_SORT, wxDefaultValidator, _T("ID_CHOICE_SingleStrand_FX_Palette"));
	FlexGridSizer4->Add(Choice_FX_Palette, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer4->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText1 = new wxStaticText(Panel_FX, ID_STATICTEXT1, _("Intensity"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer4->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer5->AddGrowableCol(0);
	Slider_FX_Intensity = new BulkEditSlider(Panel_FX, ID_SLIDER_FX_Intensity, 128, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_FX_Intensity"));
	Slider_FX_Intensity->SetSelection(1, 20);
	FlexGridSizer5->Add(Slider_FX_Intensity, 1, wxALL|wxEXPAND, 5);
	BitmapButton_FX_IntensityVC = new BulkEditValueCurveButton(Panel_FX, ID_VALUECURVE_FX_Intensity, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxSize(18,30), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_FX_Intensity"));
	FlexGridSizer5->Add(BitmapButton_FX_IntensityVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4->Add(FlexGridSizer5, 1, wxALL|wxEXPAND, 5);
	TextCtrl_FX_Intensity = new BulkEditTextCtrl(Panel_FX, IDD_TEXTCTRL_FX_Intensity, _("128"), wxDefaultPosition, wxDLG_UNIT(Panel_FX,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_FX_Intensity"));
	TextCtrl_FX_Intensity->SetMaxLength(3);
	FlexGridSizer4->Add(TextCtrl_FX_Intensity, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(Panel_FX, ID_STATICTEXT3, _("Speed"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer4->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer6->AddGrowableCol(0);
	Slider_FX_Speed = new BulkEditSlider(Panel_FX, ID_SLIDER_FX_Speed, 128, 0, 255, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_FX_Speed"));
	Slider_FX_Speed->SetSelection(1, 20);
	FlexGridSizer6->Add(Slider_FX_Speed, 1, wxALL|wxEXPAND, 5);
	BitmapButton_FX_SpeedVC = new BulkEditValueCurveButton(Panel_FX, ID_VALUECURVE_FX_Speed, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxSize(18,30), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_FX_Speed"));
	FlexGridSizer6->Add(BitmapButton_FX_SpeedVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4->Add(FlexGridSizer6, 1, wxALL|wxEXPAND, 5);
	TextCtrl_FX_Speed = new BulkEditTextCtrl(Panel_FX, IDD_TEXTCTRL_FX_Speed, _("128"), wxDefaultPosition, wxDLG_UNIT(Panel_FX,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_FX_Speed"));
	TextCtrl_FX_Speed->SetMaxLength(3);
	FlexGridSizer4->Add(TextCtrl_FX_Speed, 1, wxALL|wxEXPAND, 5);
	Panel_FX->SetSizer(FlexGridSizer4);
	FlexGridSizer4->Fit(Panel_FX);
	FlexGridSizer4->SetSizeHints(Panel_FX);
	SingleStrandEffectType->AddPage(Panel1, _("Chase"), false);
	SingleStrandEffectType->AddPage(Panel2, _("Skips"), false);
	SingleStrandEffectType->AddPage(Panel_FX, _("FX"), false);
	FlexGridSizer89->Add(SingleStrandEffectType, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer89);
	FlexGridSizer89->Fit(this);
	FlexGridSizer89->SetSizeHints(this);

	Connect(ID_BITMAPBUTTON_CHOICE_SingleStrand_Colors,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SingleStrandPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Number_Chases,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SingleStrandPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Color_Mix1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SingleStrandPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Color_Mix1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SingleStrandPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHOICE_Chase_Type1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SingleStrandPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Chase_3dFade1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SingleStrandPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Chase_Group_All,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SingleStrandPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Skips_BandSize,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SingleStrandPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Skips_SkipSize,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SingleStrandPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Skips_StartPos,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SingleStrandPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Skips_Advance,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SingleStrandPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHOICE_Skips_Direction,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SingleStrandPanel::OnLockButtonClick);
	//*)

    Connect(ID_VALUECURVE_Chase_Rotations, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SingleStrandPanel::OnVCButtonClick);
    Connect(ID_VALUECURVE_Number_Chases, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SingleStrandPanel::OnVCButtonClick);
    Connect(ID_VALUECURVE_Chase_Offset, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SingleStrandPanel::OnVCButtonClick);
    Connect(ID_VALUECURVE_Color_Mix1, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SingleStrandPanel::OnVCButtonClick);
    Connect(ID_VALUECURVE_FX_Intensity, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SingleStrandPanel::OnVCButtonClick);
    Connect(ID_VALUECURVE_FX_Speed, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SingleStrandPanel::OnVCButtonClick);

	Connect(ID_BITMAPBUTTON_SLIDER_Chase_Rotations, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SingleStrandPanel::OnLockButtonClick);
    Connect(ID_BITMAPBUTTON_SLIDER_Chase_Offset, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SingleStrandPanel::OnLockButtonClick);

    Connect(ID_CHOICE_Chase_Type1, wxEVT_COMMAND_CHOICE_SELECTED, (wxObjectEventFunction)&SingleStrandPanel::OnChoice_Chase_Type_Select);

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&SingleStrandPanel::OnVCChanged, 0, this);
    Connect(wxID_ANY, EVT_VALIDATEWINDOW, (wxObjectEventFunction)&SingleStrandPanel::OnValidateWindow, 0, this);

    SetName("ID_PANEL_SINGLESTRAND");

	Slider_FX_Intensity->SetMin(SINGLESTRAND_FXINTENSITY_MIN);
    Slider_FX_Intensity->SetMax(SINGLESTRAND_FXINTENSITY_MAX);

	Slider_FX_Speed->SetMin(SINGLESTRAND_FXSPEED_MIN);
    Slider_FX_Speed->SetMax(SINGLESTRAND_FXSPEED_MAX);

    BitmapButton_Chase_Rotations->GetValue()->SetLimits(SINGLESTRAND_ROTATIONS_MIN, SINGLESTRAND_ROTATIONS_MAX);
    BitmapButton_Chase_Rotations->GetValue()->SetDivisor(SINGLESTRAND_ROTATIONS_DIVISOR);
    BitmapButton_Chase_OffsetVC->GetValue()->SetLimits(SINGLESTRAND_OFFSET_MIN, SINGLESTRAND_OFFSET_MAX);
    BitmapButton_Chase_OffsetVC->GetValue()->SetDivisor(SINGLESTRAND_OFFSET_DIVISOR);
    BitmapButton_Number_ChasesVC->GetValue()->SetLimits(SINGLESTRAND_CHASES_MIN, SINGLESTRAND_CHASES_MAX);
    BitmapButton_Color_Mix1VC->GetValue()->SetLimits(SINGLESTRAND_COLOURMIX_MIN, SINGLESTRAND_COLOURMIX_MAX);
    BitmapButton_FX_IntensityVC->GetValue()->SetLimits(SINGLESTRAND_FXINTENSITY_MIN, SINGLESTRAND_FXINTENSITY_MAX);
    BitmapButton_FX_SpeedVC->GetValue()->SetLimits(SINGLESTRAND_FXSPEED_MIN, SINGLESTRAND_FXSPEED_MAX);

	wxString names = JSON_mode_names;
    names.Replace("\n", "");
    names.Replace("\"", "");
    names.Replace("[", "");
    names.Replace("]", "");
    auto nn = wxSplit(names, ',');

	for (const auto& n : nn) {
        Choice_SingleStrand_FX->AppendString(n);
	}
    Choice_SingleStrand_FX->SetSelection(0);

	names = JSON_palette_names;
    names.Replace("\n", "");
    names.Replace("\"", "");
    names.Replace("[", "");
    names.Replace("]", "");
    nn = wxSplit(names, ',');

    for (const auto& n : nn) {
        Choice_FX_Palette->AppendString(n);
    }
	// This will use the colour panel colours by default
    Choice_FX_Palette->SetStringSelection("* Colors Only");

    ValidateWindow();
}

SingleStrandPanel::~SingleStrandPanel()
{
	//(*Destroy(SingleStrandPanel)
	//*)
}

void SingleStrandPanel::ValidateWindow()
{
    if (Choice_Chase_Type1->GetStringSelection().StartsWith("Static")) {
        Slider_Chase_Rotations->Disable();
        BitmapButton_Chase_Rotations->Disable();
    } else {
        Slider_Chase_Rotations->Enable();
        BitmapButton_Chase_Rotations->Enable();
    }
}

void SingleStrandPanel::OnChoice_Chase_Type_Select(wxCommandEvent& event)
{
    ValidateWindow();
}
