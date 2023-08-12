/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "PinwheelPanel.h"
#include "EffectPanelUtils.h"
#include "PinwheelEffect.h"

//(*InternalHeaders(PinwheelPanel)
#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>
//*)

//(*IdInit(PinwheelPanel)
const long PinwheelPanel::ID_STATICTEXT_Pinwheel_Arms = wxNewId();
const long PinwheelPanel::ID_SLIDER_Pinwheel_Arms = wxNewId();
const long PinwheelPanel::IDD_TEXTCTRL_Pinwheel_Arms = wxNewId();
const long PinwheelPanel::ID_BITMAPBUTTON_SLIDER_Pinwheel_Arms = wxNewId();
const long PinwheelPanel::ID_STATICTEXT_Pinwheel_ArmSize = wxNewId();
const long PinwheelPanel::ID_SLIDER_Pinwheel_ArmSize = wxNewId();
const long PinwheelPanel::ID_VALUECURVE_Pinwheel_ArmSize = wxNewId();
const long PinwheelPanel::IDD_TEXTCTRL_Pinwheel_ArmSize = wxNewId();
const long PinwheelPanel::ID_BITMAPBUTTON_SLIDER_Pinwheel_ArmSize = wxNewId();
const long PinwheelPanel::ID_STATICTEXT_Pinwheel_Twist = wxNewId();
const long PinwheelPanel::ID_SLIDER_Pinwheel_Twist = wxNewId();
const long PinwheelPanel::ID_VALUECURVE_Pinwheel_Twist = wxNewId();
const long PinwheelPanel::IDD_TEXTCTRL_Pinwheel_Twist = wxNewId();
const long PinwheelPanel::ID_BITMAPBUTTON_SLIDER_Pinwheel_Twist = wxNewId();
const long PinwheelPanel::ID_STATICTEXT_Pinwheel_Thickness = wxNewId();
const long PinwheelPanel::ID_SLIDER_Pinwheel_Thickness = wxNewId();
const long PinwheelPanel::ID_VALUECURVE_Pinwheel_Thickness = wxNewId();
const long PinwheelPanel::IDD_TEXTCTRL_Pinwheel_Thickness = wxNewId();
const long PinwheelPanel::ID_BITMAPBUTTON_SLIDER_Pinwheel_Thickness = wxNewId();
const long PinwheelPanel::ID_STATICTEXT_Pinwheel_Speed = wxNewId();
const long PinwheelPanel::ID_SLIDER_Pinwheel_Speed = wxNewId();
const long PinwheelPanel::ID_VALUECURVE_Pinwheel_Speed = wxNewId();
const long PinwheelPanel::IDD_TEXTCTRL_Pinwheel_Speed = wxNewId();
const long PinwheelPanel::ID_BITMAPBUTTON_SLIDER_Pinwheel_Speed = wxNewId();
const long PinwheelPanel::ID_STATICTEXT_Pinwheel_Offset = wxNewId();
const long PinwheelPanel::ID_SLIDER_Pinwheel_Offset = wxNewId();
const long PinwheelPanel::ID_VALUECURVE_Pinwheel_Offset = wxNewId();
const long PinwheelPanel::IDD_TEXTCTRL_Pinwheel_Offset = wxNewId();
const long PinwheelPanel::ID_BITMAPBUTTON_SLIDER_Pinwheel_Offset = wxNewId();
const long PinwheelPanel::ID_STATICTEXT_Pinwheel_Style = wxNewId();
const long PinwheelPanel::ID_CHOICE_Pinwheel_Style = wxNewId();
const long PinwheelPanel::ID_STATICTEXT78 = wxNewId();
const long PinwheelPanel::ID_CHECKBOX_Pinwheel_Rotation = wxNewId();
const long PinwheelPanel::ID_BITMAPBUTTON_CHECKBOX_Pinwheel_Rotation = wxNewId();
const long PinwheelPanel::ID_STATICTEXT_Pinwheel_3D = wxNewId();
const long PinwheelPanel::ID_CHOICE_Pinwheel_3D = wxNewId();
const long PinwheelPanel::ID_BITMAPBUTTON_CHOICE_Pinwheel_3D = wxNewId();
const long PinwheelPanel::ID_STATICTEXT_PinwheelXC = wxNewId();
const long PinwheelPanel::ID_SLIDER_PinwheelXC = wxNewId();
const long PinwheelPanel::ID_VALUECURVE_PinwheelXC = wxNewId();
const long PinwheelPanel::IDD_TEXTCTRL_PinwheelXC = wxNewId();
const long PinwheelPanel::ID_STATICTEXT_PinwheelYC = wxNewId();
const long PinwheelPanel::ID_VALUECURVE_PinwheelYC = wxNewId();
const long PinwheelPanel::IDD_TEXTCTRL_PinwheelYC = wxNewId();
const long PinwheelPanel::ID_SLIDER_PinwheelYC = wxNewId();
//*)

BEGIN_EVENT_TABLE(PinwheelPanel,wxPanel)
	//(*EventTable(PinwheelPanel)
	//*)
END_EVENT_TABLE()

PinwheelPanel::PinwheelPanel(wxWindow* parent) : xlEffectPanel(parent)
{
	//(*Initialize(PinwheelPanel)
	BulkEditTextCtrl* TextCtrl1;
	BulkEditTextCtrl* TextCtrl64;
	BulkEditTextCtrl* TextCtrl65;
	BulkEditTextCtrl* TextCtrl66;
	BulkEditTextCtrl* TextCtrl67;
	BulkEditTextCtrl* TextCtrl68;
	BulkEditTextCtrl* TextCtrl69;
	BulkEditTextCtrl* TextCtrl70;
	wxBoxSizer* BoxSizer1;
	wxFlexGridSizer* FlexGridSizer128;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer55;
	wxFlexGridSizer* FlexGridSizer59;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer61;
	wxFlexGridSizer* FlexGridSizer62;
	wxFlexGridSizer* FlexGridSizer63;
	wxFlexGridSizer* FlexGridSizer64;
	wxFlexGridSizer* FlexGridSizer68;
	wxFlexGridSizer* FlexGridSizer6;
	wxFlexGridSizer* FlexGridSizer7;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer55 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer55->AddGrowableCol(0);
	FlexGridSizer55->AddGrowableRow(2);
	FlexGridSizer128 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer128->AddGrowableCol(1);
	StaticText63 = new wxStaticText(this, ID_STATICTEXT_Pinwheel_Arms, _("#Arms"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Pinwheel_Arms"));
	FlexGridSizer128->Add(StaticText63, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Pinwheel_Arms = new BulkEditSlider(this, ID_SLIDER_Pinwheel_Arms, 3, 1, 20, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Pinwheel_Arms"));
	FlexGridSizer128->Add(Slider_Pinwheel_Arms, 1, wxALL|wxEXPAND, 2);
	TextCtrl64 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Pinwheel_Arms, _("3"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Pinwheel_Arms"));
	TextCtrl64->SetMaxLength(3);
	FlexGridSizer128->Add(TextCtrl64, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_PinwheelNumberArms = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Pinwheel_Arms, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Pinwheel_Arms"));
	BitmapButton_PinwheelNumberArms->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer128->Add(BitmapButton_PinwheelNumberArms, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText84 = new wxStaticText(this, ID_STATICTEXT_Pinwheel_ArmSize, _("Size"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Pinwheel_ArmSize"));
	FlexGridSizer128->Add(StaticText84, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	Slider_Pinwheel_ArmSize = new BulkEditSlider(this, ID_SLIDER_Pinwheel_ArmSize, 100, 0, 400, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Pinwheel_ArmSize"));
	FlexGridSizer1->Add(Slider_Pinwheel_ArmSize, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Pinwheel_ArmSizeVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Pinwheel_ArmSize, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Pinwheel_ArmSize"));
	FlexGridSizer1->Add(BitmapButton_Pinwheel_ArmSizeVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer128->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 0);
	TextCtrl65 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Pinwheel_ArmSize, _("100"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Pinwheel_ArmSize"));
	TextCtrl65->SetMaxLength(3);
	FlexGridSizer128->Add(TextCtrl65, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Pinwheel_ArmSize = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Pinwheel_ArmSize, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Pinwheel_ArmSize"));
	BitmapButton_Pinwheel_ArmSize->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer128->Add(BitmapButton_Pinwheel_ArmSize, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText74 = new wxStaticText(this, ID_STATICTEXT_Pinwheel_Twist, _("Twist"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Pinwheel_Twist"));
	FlexGridSizer128->Add(StaticText74, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	Slider_Pinwheel_Twist = new BulkEditSlider(this, ID_SLIDER_Pinwheel_Twist, 0, -360, 360, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Pinwheel_Twist"));
	FlexGridSizer2->Add(Slider_Pinwheel_Twist, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Pinwheel_TwistVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Pinwheel_Twist, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Pinwheel_Twist"));
	FlexGridSizer2->Add(BitmapButton_Pinwheel_TwistVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer128->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 0);
	TextCtrl66 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Pinwheel_Twist, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Pinwheel_Twist"));
	TextCtrl66->SetMaxLength(4);
	FlexGridSizer128->Add(TextCtrl66, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_PinwheelTwist = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Pinwheel_Twist, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Pinwheel_Twist"));
	BitmapButton_PinwheelTwist->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer128->Add(BitmapButton_PinwheelTwist, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	StaticText75 = new wxStaticText(this, ID_STATICTEXT_Pinwheel_Thickness, _("Thick"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Pinwheel_Thickness"));
	FlexGridSizer128->Add(StaticText75, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	Slider_Pinwheel_Thickness = new BulkEditSlider(this, ID_SLIDER_Pinwheel_Thickness, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Pinwheel_Thickness"));
	FlexGridSizer3->Add(Slider_Pinwheel_Thickness, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Pinwheel_ThicknessVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Pinwheel_Thickness, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Pinwheel_Thickness"));
	FlexGridSizer3->Add(BitmapButton_Pinwheel_ThicknessVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer128->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 0);
	TextCtrl67 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Pinwheel_Thickness, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Pinwheel_Thickness"));
	TextCtrl67->SetMaxLength(3);
	FlexGridSizer128->Add(TextCtrl67, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_PinwheelThickness = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Pinwheel_Thickness, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Pinwheel_Thickness"));
	BitmapButton_PinwheelThickness->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer128->Add(BitmapButton_PinwheelThickness, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText184 = new wxStaticText(this, ID_STATICTEXT_Pinwheel_Speed, _("Speed"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Pinwheel_Speed"));
	FlexGridSizer128->Add(StaticText184, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer4->AddGrowableCol(0);
	Slider_Pinwheel_Speed = new BulkEditSlider(this, ID_SLIDER_Pinwheel_Speed, 10, 0, 50, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Pinwheel_Speed"));
	FlexGridSizer4->Add(Slider_Pinwheel_Speed, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Pinwheel_SpeedVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Pinwheel_Speed, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Pinwheel_Speed"));
	FlexGridSizer4->Add(BitmapButton_Pinwheel_SpeedVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer128->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 0);
	TextCtrl70 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Pinwheel_Speed, _("10"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Pinwheel_Speed"));
	TextCtrl70->SetMaxLength(3);
	FlexGridSizer128->Add(TextCtrl70, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_PinwheelSpeed = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Pinwheel_Speed, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Pinwheel_Speed"));
	BitmapButton_PinwheelSpeed->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer128->Add(BitmapButton_PinwheelSpeed, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT_Pinwheel_Offset, _("Offset"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Pinwheel_Offset"));
	FlexGridSizer128->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer7 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer7->AddGrowableCol(0);
	Slider_Pinwheel_Offset = new BulkEditSlider(this, ID_SLIDER_Pinwheel_Offset, 0, 0, 360, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Pinwheel_Offset"));
	FlexGridSizer7->Add(Slider_Pinwheel_Offset, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Pinwheel_OffsetVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Pinwheel_Offset, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Pinwheel_Offset"));
	FlexGridSizer7->Add(BitmapButton_Pinwheel_OffsetVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer128->Add(FlexGridSizer7, 1, wxALL|wxEXPAND, 0);
	TextCtrl1 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Pinwheel_Offset, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Pinwheel_Offset"));
	TextCtrl1->SetMaxLength(3);
	FlexGridSizer128->Add(TextCtrl1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_PinwheelOffset = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Pinwheel_Offset, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Pinwheel_Offset"));
	BitmapButton_PinwheelOffset->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer128->Add(BitmapButton_PinwheelOffset, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT_Pinwheel_Style, _("Style"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Pinwheel_Style"));
	FlexGridSizer128->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Pinwheel_Style = new BulkEditChoice(this, ID_CHOICE_Pinwheel_Style, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Pinwheel_Style"));
	Choice_Pinwheel_Style->SetSelection( Choice_Pinwheel_Style->Append(_("Old Render Method")) );
	Choice_Pinwheel_Style->Append(_("New Render Method"));
	FlexGridSizer128->Add(Choice_Pinwheel_Style, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer128->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer55->Add(FlexGridSizer128, 1, wxALL|wxEXPAND, 0);
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	FlexGridSizer59 = new wxFlexGridSizer(0, 4, 0, 0);
	StaticText76 = new wxStaticText(this, ID_STATICTEXT78, _("Rotation"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT78"));
	FlexGridSizer59->Add(StaticText76, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Pinwheel_Rotation = new BulkEditCheckBox(this, ID_CHECKBOX_Pinwheel_Rotation, _("CCW"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Pinwheel_Rotation"));
	CheckBox_Pinwheel_Rotation->SetValue(true);
	FlexGridSizer59->Add(CheckBox_Pinwheel_Rotation, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_PinwheelRotation = new xlLockButton(this, ID_BITMAPBUTTON_CHECKBOX_Pinwheel_Rotation, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Pinwheel_Rotation"));
	BitmapButton_PinwheelRotation->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer59->Add(BitmapButton_PinwheelRotation, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(FlexGridSizer59, 1, wxALL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer61 = new wxFlexGridSizer(0, 3, 0, 0);
	StaticText77 = new wxStaticText(this, ID_STATICTEXT_Pinwheel_3D, _("3D"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Pinwheel_3D"));
	FlexGridSizer61->Add(StaticText77, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Pinwheel_3D = new BulkEditChoice(this, ID_CHOICE_Pinwheel_3D, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Pinwheel_3D"));
	Choice_Pinwheel_3D->SetSelection( Choice_Pinwheel_3D->Append(_("None")) );
	Choice_Pinwheel_3D->Append(_("3D"));
	Choice_Pinwheel_3D->Append(_("3D Inverted"));
	Choice_Pinwheel_3D->Append(_("Sweep"));
	FlexGridSizer61->Add(Choice_Pinwheel_3D, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	BitmapButton_Pinwheel3D = new xlLockButton(this, ID_BITMAPBUTTON_CHOICE_Pinwheel_3D, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Pinwheel_3D"));
	BitmapButton_Pinwheel3D->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer61->Add(BitmapButton_Pinwheel3D, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(FlexGridSizer61, 1, wxALL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer55->Add(BoxSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer63 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer63->AddGrowableCol(0);
	FlexGridSizer63->AddGrowableRow(0);
	FlexGridSizer62 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer62->AddGrowableCol(1);
	StaticText82 = new wxStaticText(this, ID_STATICTEXT_PinwheelXC, _("X-axis Center"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_PinwheelXC"));
	FlexGridSizer62->Add(StaticText82, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_PinwheelXC = new BulkEditSlider(this, ID_SLIDER_PinwheelXC, 0, -100, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_PinwheelXC"));
	FlexGridSizer62->Add(Slider_PinwheelXC, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer62->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
	BitmapButton_PinwheelXCVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_PinwheelXC, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_PinwheelXC"));
	FlexGridSizer5->Add(BitmapButton_PinwheelXCVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl68 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_PinwheelXC, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_PinwheelXC"));
	TextCtrl68->SetMaxLength(4);
	FlexGridSizer5->Add(TextCtrl68, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer62->Add(FlexGridSizer5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer63->Add(FlexGridSizer62, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer68 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer68->AddGrowableRow(0);
	FlexGridSizer64 = new wxFlexGridSizer(0, 1, 0, 0);
	StaticText81 = new wxStaticText(this, ID_STATICTEXT_PinwheelYC, _("Y-axis center"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_PinwheelYC"));
	FlexGridSizer64->Add(StaticText81, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
	BitmapButton_PinwheelYCVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_PinwheelYC, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_PinwheelYC"));
	FlexGridSizer6->Add(BitmapButton_PinwheelYCVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl69 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_PinwheelYC, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_PinwheelYC"));
	TextCtrl69->SetMaxLength(4);
	FlexGridSizer6->Add(TextCtrl69, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer64->Add(FlexGridSizer6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer68->Add(FlexGridSizer64, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	Slider_PinwheelYC = new BulkEditSlider(this, ID_SLIDER_PinwheelYC, 0, -100, 100, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL|wxSL_INVERSE, wxDefaultValidator, _T("ID_SLIDER_PinwheelYC"));
	FlexGridSizer68->Add(Slider_PinwheelYC, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer63->Add(FlexGridSizer68, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer55->Add(FlexGridSizer63, 1, wxALL|wxEXPAND, 0);
	SetSizer(FlexGridSizer55);

	Connect(ID_BITMAPBUTTON_SLIDER_Pinwheel_Arms,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PinwheelPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Pinwheel_ArmSize,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PinwheelPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Pinwheel_ArmSize,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PinwheelPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Pinwheel_Twist,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PinwheelPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Pinwheel_Twist,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PinwheelPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Pinwheel_Thickness,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PinwheelPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Pinwheel_Thickness,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PinwheelPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Pinwheel_Speed,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PinwheelPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Pinwheel_Speed,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PinwheelPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Pinwheel_Offset,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PinwheelPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Pinwheel_Offset,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PinwheelPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Pinwheel_Rotation,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PinwheelPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHOICE_Pinwheel_3D,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PinwheelPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_PinwheelXC,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PinwheelPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_PinwheelYC,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PinwheelPanel::OnVCButtonClick);
	//*)
    SetName("ID_PANEL_PINWHEEL");

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&PinwheelPanel::OnVCChanged, 0, this);
    Connect(wxID_ANY, EVT_VALIDATEWINDOW, (wxObjectEventFunction)&PinwheelPanel::OnValidateWindow, 0, this);

    BitmapButton_PinwheelXCVC->GetValue()->SetLimits(PINWHEEL_X_MIN, PINWHEEL_X_MAX);
    BitmapButton_PinwheelYCVC->GetValue()->SetLimits(PINWHEEL_Y_MIN, PINWHEEL_Y_MAX);
    BitmapButton_Pinwheel_ArmSizeVC->GetValue()->SetLimits(PINWHEEL_ARMSIZE_MIN, PINWHEEL_ARMSIZE_MAX);
    BitmapButton_Pinwheel_SpeedVC->GetValue()->SetLimits(PINWHEEL_SPEED_MIN, PINWHEEL_SPEED_MAX);
    BitmapButton_Pinwheel_ThicknessVC->GetValue()->SetLimits(PINWHEEL_THICKNESS_MIN, PINWHEEL_THICKNESS_MAX);
    BitmapButton_Pinwheel_TwistVC->GetValue()->SetLimits(PINWHEEL_TWIST_MIN, PINWHEEL_TWIST_MAX);
    BitmapButton_Pinwheel_OffsetVC->GetValue()->SetLimits(PINWHEEL_OFFSET_MIN, PINWHEEL_OFFSET_MAX);

    ValidateWindow();
}

PinwheelPanel::~PinwheelPanel()
{
	//(*Destroy(PinwheelPanel)
	//*)
}

void PinwheelPanel::ValidateWindow()
{
}
