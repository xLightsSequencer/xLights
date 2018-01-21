#include "SingleStrandPanel.h"
#include "../../include/padlock16x16-blue.xpm"
#include "EffectPanelUtils.h"
#include "SingleStrandEffect.h"

//(*InternalHeaders(SingleStrandPanel)
#include <wx/bmpbuttn.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/notebook.h>
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
const long SingleStrandPanel::IS_STATICTEXT_Skips_Advance = wxNewId();
const long SingleStrandPanel::ID_SLIDER_Skips_Advance = wxNewId();
const long SingleStrandPanel::IDD_TEXTCTRL_Skips_Advance = wxNewId();
const long SingleStrandPanel::ID_BITMAPBUTTON_SLIDER_Skips_Advance = wxNewId();
const long SingleStrandPanel::ID_STATICTEXT_Skips_Direction = wxNewId();
const long SingleStrandPanel::ID_CHOICE_Skips_Direction = wxNewId();
const long SingleStrandPanel::ID_BITMAPBUTTON_CHOICE_Skips_Direction = wxNewId();
const long SingleStrandPanel::ID_PANEL21 = wxNewId();
const long SingleStrandPanel::ID_NOTEBOOK_SSEFFECT_TYPE = wxNewId();
//*)

BEGIN_EVENT_TABLE(SingleStrandPanel,wxPanel)
	//(*EventTable(SingleStrandPanel)
	//*)
END_EVENT_TABLE()

SingleStrandPanel::SingleStrandPanel(wxWindow* parent)
{
	//(*Initialize(SingleStrandPanel)
	wxFlexGridSizer* FlexGridSizer21;
	wxFlexGridSizer* FlexGridSizer1;
	BulkEditTextCtrlF1* TextCtrl43;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer79;
	BulkEditTextCtrl* TextCtrl44;
	wxFlexGridSizer* FlexGridSizer81;
	BulkEditTextCtrl* TextCtrl4;
	wxFlexGridSizer* FlexGridSizer3;
	BulkEditTextCtrl* TextCtrl5;
	BulkEditTextCtrl* TextCtrl45;
	wxFlexGridSizer* FlexGridSizer83;
	wxFlexGridSizer* FlexGridSizer89;
	BulkEditTextCtrl* TextCtrl3;
	wxFlexGridSizer* FlexGridSizer84;
	BulkEditTextCtrl* TextCtrl42;
	wxFlexGridSizer* FlexGridSizer24;
	wxFlexGridSizer* FlexGridSizer82;

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
	BitmapButton_SingleStrand_Colors = new wxBitmapButton(Panel1, ID_BITMAPBUTTON_CHOICE_SingleStrand_Colors, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_SingleStrand_Colors"));
	BitmapButton_SingleStrand_Colors->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer24->Add(BitmapButton_SingleStrand_Colors, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText60 = new wxStaticText(Panel1, ID_STATICTEXT_Number_Chases, _("Number Chases"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Number_Chases"));
	FlexGridSizer24->Add(StaticText60, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	Slider_Number_Chases = new BulkEditSlider(Panel1, ID_SLIDER_Number_Chases, 1, 1, 20, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Number_Chases"));
	Slider_Number_Chases->SetSelection(1, 20);
	FlexGridSizer1->Add(Slider_Number_Chases, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Number_ChasesVC = new BulkEditValueCurveButton(Panel1, ID_VALUECURVE_Number_Chases, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Number_Chases"));
	FlexGridSizer1->Add(BitmapButton_Number_ChasesVC, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer24->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 0);
	TextCtrl44 = new BulkEditTextCtrl(Panel1, IDD_TEXTCTRL_Number_Chases, _("1"), wxDefaultPosition, wxDLG_UNIT(Panel1,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Number_Chases"));
	TextCtrl44->SetMaxLength(3);
	FlexGridSizer24->Add(TextCtrl44, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Number_Chases = new wxBitmapButton(Panel1, ID_BITMAPBUTTON_SLIDER_Number_Chases, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Number_Chases"));
	BitmapButton_Number_Chases->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer24->Add(BitmapButton_Number_Chases, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText9 = new wxStaticText(Panel1, ID_STATICTEXT_Color_Mix1, _("Chase Size"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Color_Mix1"));
	FlexGridSizer24->Add(StaticText9, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	Slider_Color_Mix1 = new BulkEditSlider(Panel1, ID_SLIDER_Color_Mix1, 10, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Color_Mix1"));
	FlexGridSizer2->Add(Slider_Color_Mix1, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Color_Mix1VC = new BulkEditValueCurveButton(Panel1, ID_VALUECURVE_Color_Mix1, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Color_Mix1"));
	FlexGridSizer2->Add(BitmapButton_Color_Mix1VC, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer24->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 0);
	TextCtrl45 = new BulkEditTextCtrl(Panel1, IDD_TEXTCTRL_Color_Mix1, _("10"), wxDefaultPosition, wxDLG_UNIT(Panel1,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Color_Mix1"));
	TextCtrl45->SetMaxLength(4);
	FlexGridSizer24->Add(TextCtrl45, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Color_Mix1 = new wxBitmapButton(Panel1, ID_BITMAPBUTTON_SLIDER_Color_Mix1, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Color_Mix1"));
	BitmapButton_Color_Mix1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer24->Add(BitmapButton_Color_Mix1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText10 = new wxStaticText(Panel1, ID_STATICTEXT_Chase_Rotations, _("Cycles"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Chase_Rotations"));
	FlexGridSizer24->Add(StaticText10, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	Slider_Chase_Rotations = new BulkEditSliderF1(Panel1, IDD_SLIDER_Chase_Rotations, 10, 1, 200, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Chase_Rotations"));
	FlexGridSizer3->Add(Slider_Chase_Rotations, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Chase_Rotations = new BulkEditValueCurveButton(Panel1, ID_VALUECURVE_Chase_Rotations, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Chase_Rotations"));
	FlexGridSizer3->Add(BitmapButton_Chase_Rotations, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer24->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 0);
	TextCtrl43 = new BulkEditTextCtrlF1(Panel1, ID_TEXTCTRL_Chase_Rotations, _("1.0"), wxDefaultPosition, wxDLG_UNIT(Panel1,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Chase_Rotations"));
	TextCtrl43->SetMaxLength(4);
	FlexGridSizer24->Add(TextCtrl43, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_ChaseRotations = new wxBitmapButton(Panel1, ID_BITMAPBUTTON_SLIDER_Chase_Rotations, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Chase_Rotations"));
	BitmapButton_ChaseRotations->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer24->Add(BitmapButton_ChaseRotations, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText11 = new wxStaticText(Panel1, ID_STATICTEXT_Chase_Type1, _("Chase Types"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Chase_Type1"));
	FlexGridSizer24->Add(StaticText11, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Chase_Type1 = new BulkEditChoice(Panel1, ID_CHOICE_Chase_Type1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Chase_Type1"));
	Choice_Chase_Type1->SetSelection( Choice_Chase_Type1->Append(_("Left-Right")) );
	Choice_Chase_Type1->Append(_("Right-Left"));
	Choice_Chase_Type1->Append(_("Bounce from Left"));
	Choice_Chase_Type1->Append(_("Bounce from Right"));
	Choice_Chase_Type1->Append(_("Dual Bounce"));
	Choice_Chase_Type1->Append(_("From Middle"));
	Choice_Chase_Type1->Append(_("To Middle"));
	FlexGridSizer24->Add(Choice_Chase_Type1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer24->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Chase_Type1 = new wxBitmapButton(Panel1, ID_BITMAPBUTTON_CHOICE_Chase_Type1, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Chase_Type1"));
	BitmapButton_Chase_Type1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer24->Add(BitmapButton_Chase_Type1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer24->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Chase_3dFade1 = new BulkEditCheckBox(Panel1, ID_CHECKBOX_Chase_3dFade1, _("3d Fade"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Chase_3dFade1"));
	CheckBox_Chase_3dFade1->SetValue(false);
	FlexGridSizer24->Add(CheckBox_Chase_3dFade1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer24->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Chase_3dFade1 = new wxBitmapButton(Panel1, ID_BITMAPBUTTON_CHECKBOX_Chase_3dFade1, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Chase_3dFade1"));
	BitmapButton_Chase_3dFade1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer24->Add(BitmapButton_Chase_3dFade1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer24->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Chase_Group_All = new BulkEditCheckBox(Panel1, ID_CHECKBOX_Chase_Group_All, _("Group All Arches"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Chase_Group_All"));
	CheckBox_Chase_Group_All->SetValue(false);
	FlexGridSizer24->Add(CheckBox_Chase_Group_All, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer24->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Chase_Group_All = new wxBitmapButton(Panel1, ID_BITMAPBUTTON_CHECKBOX_Chase_Group_All, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Chase_Group_All"));
	BitmapButton_Chase_Group_All->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
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
	BitmapButton_Skips_BandSize = new wxBitmapButton(Panel2, ID_BITMAPBUTTON_SLIDER_Skips_BandSize, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Skips_BandSize"));
	BitmapButton_Skips_BandSize->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
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
	BitmapButton_Skips_SkipSize = new wxBitmapButton(Panel2, ID_BITMAPBUTTON_SLIDER_Skips_SkipSize, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Skips_SkipSize"));
	BitmapButton_Skips_SkipSize->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
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
	BitmapButton_Skips_StartingPosition = new wxBitmapButton(Panel2, ID_BITMAPBUTTON_SLIDER_Skips_StartPos, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Skips_StartPos"));
	BitmapButton_Skips_StartingPosition->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer83->Add(BitmapButton_Skips_StartingPosition, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer79->Add(FlexGridSizer83, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer21 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer21->AddGrowableCol(1);
	StaticText179 = new wxStaticText(Panel2, IS_STATICTEXT_Skips_Advance, _("Number of Advances"), wxDefaultPosition, wxDefaultSize, 0, _T("IS_STATICTEXT_Skips_Advance"));
	FlexGridSizer21->Add(StaticText179, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Skips_Advance = new BulkEditSlider(Panel2, ID_SLIDER_Skips_Advance, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Skips_Advance"));
	FlexGridSizer21->Add(Slider_Skips_Advance, 1, wxALL|wxEXPAND, 5);
	TextCtrl42 = new BulkEditTextCtrl(Panel2, IDD_TEXTCTRL_Skips_Advance, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel2,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Skips_Advance"));
	TextCtrl42->SetMaxLength(3);
	FlexGridSizer21->Add(TextCtrl42, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_SkipsAdvance = new wxBitmapButton(Panel2, ID_BITMAPBUTTON_SLIDER_Skips_Advance, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Skips_Advance"));
	BitmapButton_SkipsAdvance->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
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
	BitmapButton_Skips_Direction = new wxBitmapButton(Panel2, ID_BITMAPBUTTON_CHOICE_Skips_Direction, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Skips_Direction"));
	BitmapButton_Skips_Direction->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer84->Add(BitmapButton_Skips_Direction, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer79->Add(FlexGridSizer84, 1, wxALL|wxEXPAND, 2);
	Panel2->SetSizer(FlexGridSizer79);
	FlexGridSizer79->Fit(Panel2);
	FlexGridSizer79->SetSizeHints(Panel2);
	SingleStrandEffectType->AddPage(Panel1, _("Chase"), false);
	SingleStrandEffectType->AddPage(Panel2, _("Skips"), false);
	FlexGridSizer89->Add(SingleStrandEffectType, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer89);
	FlexGridSizer89->Fit(this);
	FlexGridSizer89->SetSizeHints(this);

	Connect(ID_BITMAPBUTTON_CHOICE_SingleStrand_Colors,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SingleStrandPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Number_Chases,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SingleStrandPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Number_Chases,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SingleStrandPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Color_Mix1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SingleStrandPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Color_Mix1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SingleStrandPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Chase_Rotations,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SingleStrandPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Chase_Rotations,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SingleStrandPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHOICE_Chase_Type1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SingleStrandPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Chase_3dFade1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SingleStrandPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Chase_Group_All,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SingleStrandPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Skips_BandSize,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SingleStrandPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Skips_SkipSize,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SingleStrandPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Skips_StartPos,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SingleStrandPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Skips_Advance,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SingleStrandPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHOICE_Skips_Direction,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SingleStrandPanel::OnLockButtonClick);
	//*)

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&SingleStrandPanel::OnVCChanged, 0, this);

    BitmapButton_Chase_Rotations->GetValue()->SetLimits(SINGLESTRAND_ROTATIONS_MIN, SINGLESTRAND_ROTATIONS_MAX);
    BitmapButton_Chase_Rotations->GetValue()->SetDivisor(10);
    BitmapButton_Number_ChasesVC->GetValue()->SetLimits(SINGLESTRAND_CHASES_MIN, SINGLESTRAND_CHASES_MAX);
    BitmapButton_Color_Mix1VC->GetValue()->SetLimits(SINGLESTRAND_COLOURMIX_MIN, SINGLESTRAND_COLOURMIX_MAX);

    SetName("ID_PANEL_SINGLESTRAND");
}

SingleStrandPanel::~SingleStrandPanel()
{
	//(*Destroy(SingleStrandPanel)
	//*)
}

PANEL_EVENT_HANDLERS(SingleStrandPanel)
