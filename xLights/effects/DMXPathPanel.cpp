/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

 //(*InternalHeaders(DMXPathPanel)
 #include <wx/bitmap.h>
 #include <wx/bmpbuttn.h>
 #include <wx/button.h>
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

#include <wx/textentry.h>

#include "DMXPathPanel.h"
#include "DMXPathEffect.h"
#include "../xLightsMain.h"
#include "../sequencer/MainSequencer.h"
#include "../sequencer/Effect.h"
#include "../sequencer/Element.h"
#include "../models/ModelGroup.h"
#include "RemapDMXChannelsDialog.h"
#include "xLightsApp.h"
#include "EffectPanelUtils.h"
#include "../UtilFunctions.h"

//(*IdInit(DMXPathPanel)
const long DMXPathPanel::ID_STATICTEXT1 = wxNewId();
const long DMXPathPanel::ID_CHOICE_DMXPath_Type = wxNewId();
const long DMXPathPanel::ID_BITMAPBUTTON_CHOICE_DMXPath_Type = wxNewId();
const long DMXPathPanel::ID_STATICTEXT_DMXPath_Width = wxNewId();
const long DMXPathPanel::ID_SLIDER_DMXPath_Width = wxNewId();
const long DMXPathPanel::IDD_TEXTCTRL_DMXPath_Width = wxNewId();
const long DMXPathPanel::ID_BITMAPBUTTON_SLIDER_DMXPath_Width = wxNewId();
const long DMXPathPanel::ID_STATICTEXT_DMXPath_Height = wxNewId();
const long DMXPathPanel::ID_SLIDER_DMXPath_Height = wxNewId();
const long DMXPathPanel::IDD_TEXTCTRL_DMXPath_Height = wxNewId();
const long DMXPathPanel::ID_BITMAPBUTTON_SLIDER_DMXPath_Height = wxNewId();
const long DMXPathPanel::ID_STATICTEXT_DMXPath_X = wxNewId();
const long DMXPathPanel::ID_SLIDER_DMXPath_X = wxNewId();
const long DMXPathPanel::IDD_TEXTCTRL_DMXPath_X = wxNewId();
const long DMXPathPanel::ID_BITMAPBUTTON_SLIDER_DMXPath_X = wxNewId();
const long DMXPathPanel::ID_STATICTEXT_DMXPath_Y = wxNewId();
const long DMXPathPanel::ID_SLIDER_DMXPath_Y = wxNewId();
const long DMXPathPanel::IDD_TEXTCTRL_DMXPath_Y = wxNewId();
const long DMXPathPanel::ID_BITMAPBUTTON_SLIDER_DMXPath_Y = wxNewId();
const long DMXPathPanel::ID_STATICTEXT_DMXPath_Rotation = wxNewId();
const long DMXPathPanel::ID_SLIDER_DMXPath_Rotation = wxNewId();
const long DMXPathPanel::ID_VALUECURVE_DMXPath_Rotation = wxNewId();
const long DMXPathPanel::IDD_TEXTCTRL_DMXPath_Rotation = wxNewId();
const long DMXPathPanel::ID_BITMAPBUTTON_SLIDER_DMXPath_Rotation = wxNewId();
const long DMXPathPanel::ID_STATICTEXT_DMXPath_Pan = wxNewId();
const long DMXPathPanel::ID_SLIDER_DMXPath_Pan = wxNewId();
const long DMXPathPanel::ID_VALUECURVE_DMXPath_Pan = wxNewId();
const long DMXPathPanel::IDD_TEXTCTRL_DMXPath_Pan = wxNewId();
const long DMXPathPanel::ID_BITMAPBUTTON_SLIDER_DMXPath_Pan = wxNewId();
const long DMXPathPanel::ID_STATICTEXT_DMXPath_Tilt = wxNewId();
const long DMXPathPanel::ID_SLIDER_DMXPath_Tilt = wxNewId();
const long DMXPathPanel::ID_VALUECURVE_DMXPath_Tilt = wxNewId();
const long DMXPathPanel::IDD_TEXTCTRL_DMXPath_Tilt = wxNewId();
const long DMXPathPanel::ID_BITMAPBUTTON_SLIDER_DMXPath_Tilt = wxNewId();
const long DMXPathPanel::ID_BUTTON_DMXPath_2D = wxNewId();
//*)

BEGIN_EVENT_TABLE(DMXPathPanel,wxPanel)
	//(*EventTable(DMXPathPanel)
	//*)
END_EVENT_TABLE()

DMXPathPanel::DMXPathPanel(wxWindow* parent) : xlEffectPanel(parent)
{
	//(*Initialize(DMXPathPanel)
	BulkEditTextCtrl* TextCtrl_DMXPath_Height;
	BulkEditTextCtrl* TextCtrl_DMXPath_Width;
	BulkEditTextCtrl* TextCtrl_DMXPath_X;
	BulkEditTextCtrl* TextCtrl_DMXPath_Y;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer_Main = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer_Main->AddGrowableCol(0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer_Main->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_DMXPath_Type = new BulkEditChoice(this, ID_CHOICE_DMXPath_Type, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_DMXPath_Type"));
	Choice_DMXPath_Type->SetSelection( Choice_DMXPath_Type->Append(_("Circle")) );
	Choice_DMXPath_Type->Append(_("Square"));
	Choice_DMXPath_Type->Append(_("Diamond"));
	Choice_DMXPath_Type->Append(_("Line"));
	Choice_DMXPath_Type->Append(_("Leaf"));
	Choice_DMXPath_Type->Append(_("Eight"));
	Choice_DMXPath_Type->Append(_("Custom"));
	FlexGridSizer_Main->Add(Choice_DMXPath_Type, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer_Main->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_DMXPathType = new xlLockButton(this, ID_BITMAPBUTTON_CHOICE_DMXPath_Type, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_DMXPath_Type"));
	BitmapButton_DMXPathType->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer_Main->Add(BitmapButton_DMXPathType, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText178 = new wxStaticText(this, ID_STATICTEXT_DMXPath_Width, _("Width"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMXPath_Width"));
	FlexGridSizer_Main->Add(StaticText178, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_DMXPath_Width = new BulkEditSlider(this, ID_SLIDER_DMXPath_Width, 30, 0, 180, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMXPath_Width"));
	FlexGridSizer_Main->Add(Slider_DMXPath_Width, 1, wxALL|wxEXPAND, 2);
	TextCtrl_DMXPath_Width = new BulkEditTextCtrl(this, IDD_TEXTCTRL_DMXPath_Width, _("30"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_DMXPath_Width"));
	TextCtrl_DMXPath_Width->SetMaxLength(3);
	FlexGridSizer_Main->Add(TextCtrl_DMXPath_Width, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_DMXPathWidth = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_DMXPath_Width, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_DMXPath_Width"));
	BitmapButton_DMXPathWidth->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer_Main->Add(BitmapButton_DMXPathWidth, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT_DMXPath_Height, _("Height"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMXPath_Height"));
	FlexGridSizer_Main->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_DMXPath_Height = new BulkEditSlider(this, ID_SLIDER_DMXPath_Height, 30, 0, 180, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMXPath_Height"));
	FlexGridSizer_Main->Add(Slider_DMXPath_Height, 1, wxALL|wxEXPAND, 2);
	TextCtrl_DMXPath_Height = new BulkEditTextCtrl(this, IDD_TEXTCTRL_DMXPath_Height, _("30"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_DMXPath_Height"));
	TextCtrl_DMXPath_Height->SetMaxLength(3);
	FlexGridSizer_Main->Add(TextCtrl_DMXPath_Height, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_DMXPathHeight = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_DMXPath_Height, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_DMXPath_Height"));
	BitmapButton_DMXPathHeight->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer_Main->Add(BitmapButton_DMXPathHeight, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT_DMXPath_X, _("X Center"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMXPath_X"));
	FlexGridSizer_Main->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_DMXPath_X = new BulkEditSlider(this, ID_SLIDER_DMXPath_X, 0, -180, 180, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMXPath_X"));
	FlexGridSizer_Main->Add(Slider_DMXPath_X, 1, wxALL|wxEXPAND, 2);
	TextCtrl_DMXPath_X = new BulkEditTextCtrl(this, IDD_TEXTCTRL_DMXPath_X, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_DMXPath_X"));
	TextCtrl_DMXPath_X->SetMaxLength(3);
	FlexGridSizer_Main->Add(TextCtrl_DMXPath_X, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_DMXPathX = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_DMXPath_X, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_DMXPath_X"));
	BitmapButton_DMXPathX->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer_Main->Add(BitmapButton_DMXPathX, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT_DMXPath_Y, _("Y Center"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMXPath_Y"));
	FlexGridSizer_Main->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_DMXPath_Y = new BulkEditSlider(this, ID_SLIDER_DMXPath_Y, 0, -180, 180, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMXPath_Y"));
	FlexGridSizer_Main->Add(Slider_DMXPath_Y, 1, wxALL|wxEXPAND, 2);
	TextCtrl_DMXPath_Y = new BulkEditTextCtrl(this, IDD_TEXTCTRL_DMXPath_Y, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_DMXPath_Y"));
	TextCtrl_DMXPath_Y->SetMaxLength(3);
	FlexGridSizer_Main->Add(TextCtrl_DMXPath_Y, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_DMXPathY = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_DMXPath_Y, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_DMXPath_Y"));
	BitmapButton_DMXPathY->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer_Main->Add(BitmapButton_DMXPathY, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText8 = new wxStaticText(this, ID_STATICTEXT_DMXPath_Rotation, _("Rotation"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMXPath_Rotation"));
	FlexGridSizer_Main->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	Slider_DMXPath_Rotation = new BulkEditSlider(this, ID_SLIDER_DMXPath_Rotation, 0, 0, 360, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMXPath_Rotation"));
	FlexGridSizer1->Add(Slider_DMXPath_Rotation, 1, wxALL|wxEXPAND, 2);
	BitmapButton_DMXPath_RotationVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_DMXPath_Rotation, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_DMXPath_Rotation"));
	FlexGridSizer1->Add(BitmapButton_DMXPath_RotationVC, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer_Main->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 0);
	TextCtrl_DMXPath_Rotation = new BulkEditTextCtrl(this, IDD_TEXTCTRL_DMXPath_Rotation, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_DMXPath_Rotation"));
	TextCtrl_DMXPath_Rotation->SetMaxLength(3);
	FlexGridSizer_Main->Add(TextCtrl_DMXPath_Rotation, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_DMXPath_Rotation = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_DMXPath_Rotation, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_DMXPath_Rotation"));
	BitmapButton_DMXPath_Rotation->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer_Main->Add(BitmapButton_DMXPath_Rotation, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText9 = new wxStaticText(this, ID_STATICTEXT_DMXPath_Pan, _("Pan"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMXPath_Pan"));
	FlexGridSizer_Main->Add(StaticText9, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	Slider_DMXPath_Pan = new BulkEditSlider(this, ID_SLIDER_DMXPath_Pan, 0, -180, 180, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMXPath_Pan"));
	FlexGridSizer2->Add(Slider_DMXPath_Pan, 1, wxALL|wxEXPAND, 2);
	BitmapButton_DMXPath_PanVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_DMXPath_Pan, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_DMXPath_Pan"));
	FlexGridSizer2->Add(BitmapButton_DMXPath_PanVC, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer_Main->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 0);
	TextCtrl_DMXPath_Pan = new BulkEditTextCtrl(this, IDD_TEXTCTRL_DMXPath_Pan, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_DMXPath_Pan"));
	TextCtrl_DMXPath_Pan->SetMaxLength(3);
	FlexGridSizer_Main->Add(TextCtrl_DMXPath_Pan, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_DMXPath_Pan = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_DMXPath_Pan, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_DMXPath_Pan"));
	BitmapButton_DMXPath_Pan->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer_Main->Add(BitmapButton_DMXPath_Pan, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText10 = new wxStaticText(this, ID_STATICTEXT_DMXPath_Tilt, _("Tilt"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_DMXPath_Tilt"));
	FlexGridSizer_Main->Add(StaticText10, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	Slider_DMXPath_Tilt = new BulkEditSlider(this, ID_SLIDER_DMXPath_Tilt, 0, -180, 180, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_DMXPath_Tilt"));
	FlexGridSizer3->Add(Slider_DMXPath_Tilt, 1, wxALL|wxEXPAND, 2);
	BitmapButton_DMXPath_TiltVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_DMXPath_Tilt, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_DMXPath_Tilt"));
	FlexGridSizer3->Add(BitmapButton_DMXPath_TiltVC, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer_Main->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 0);
	TextCtrl_DMXPath_Tilt = new BulkEditTextCtrl(this, IDD_TEXTCTRL_DMXPath_Tilt, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_DMXPath_Tilt"));
	TextCtrl_DMXPath_Tilt->SetMaxLength(3);
	FlexGridSizer_Main->Add(TextCtrl_DMXPath_Tilt, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_DMXPath_Tilt = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_DMXPath_Tilt, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_DMXPath_Tilt"));
	BitmapButton_DMXPath_Tilt->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer_Main->Add(BitmapButton_DMXPath_Tilt, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer_Main->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_DMXPath_2D = new wxButton(this, ID_BUTTON_DMXPath_2D, _("2D Path"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_DMXPath_2D"));
	FlexGridSizer_Main->Add(Button_DMXPath_2D, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer_Main);
	FlexGridSizer_Main->Fit(this);
	FlexGridSizer_Main->SetSizeHints(this);

	Connect(ID_CHOICE_DMXPath_Type,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&DMXPathPanel::OnChoice_DMXPath_TypeSelect);
	Connect(ID_BITMAPBUTTON_CHOICE_DMXPath_Type,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPathPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_DMXPath_Width,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPathPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_DMXPath_X,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPathPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_DMXPath_Y,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPathPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_DMXPath_Rotation,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPathPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_DMXPath_Pan,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPathPanel::OnVCButtonClick);
	Connect(ID_VALUECURVE_DMXPath_Tilt,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DMXPathPanel::OnVCButtonClick);
	//*)

	Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&DMXPathPanel::OnVCChanged, 0, this);
	Connect(wxID_ANY, EVT_VALIDATEWINDOW, (wxObjectEventFunction)&DMXPathPanel::OnValidateWindow, 0, this);

	BitmapButton_DMXPath_PanVC->SetLimits(DMXPATH_PAN_MIN, DMXPATH_PAN_MAX);
    BitmapButton_DMXPath_TiltVC->SetLimits(DMXPATH_TILT_MIN, DMXPATH_TILT_MAX);
    BitmapButton_DMXPath_RotationVC->SetLimits(DMXPATH_ROTATION_MIN, DMXPATH_ROTATION_MIN);
    SetName("ID_PANEL_DMX");

	ValidateWindow();
}

DMXPathPanel::~DMXPathPanel()
{
	//(*Destroy(DMXPathPanel)
	//*)
}

void DMXPathPanel::ValidateWindow()
{
    if (Choice_DMXPath_Type->GetStringSelection() == "Circle" ) {
        Slider_DMXPath_Rotation->Disable();
        TextCtrl_DMXPath_Rotation->Disable();
        BitmapButton_DMXPath_Rotation->Disable();
    } else {
        Slider_DMXPath_Rotation->Enable();
        TextCtrl_DMXPath_Rotation->Enable();
        BitmapButton_DMXPath_Rotation->Enable();
    }

	if (Choice_DMXPath_Type->GetStringSelection() == "Custom") {
        Slider_DMXPath_Pan->Enable();
        TextCtrl_DMXPath_Pan->Enable();
        BitmapButton_DMXPath_Pan->Enable();
        Slider_DMXPath_Tilt->Enable();
        TextCtrl_DMXPath_Tilt->Enable();
        BitmapButton_DMXPath_Tilt->Enable();
        Button_DMXPath_2D->Enable();
    } else {
        Slider_DMXPath_Pan->Disable();
        TextCtrl_DMXPath_Pan->Disable();
        BitmapButton_DMXPath_Pan->Disable();
        Slider_DMXPath_Tilt->Disable();
        TextCtrl_DMXPath_Tilt->Disable();
        BitmapButton_DMXPath_Tilt->Disable();
        Button_DMXPath_2D->Disable();
    }
}

void DMXPathPanel::OnChoice_DMXPath_TypeSelect(wxCommandEvent& event)
{
    ValidateWindow();
}

void DMXPathPanel::OnButton_DMXPath_2DClick(wxCommandEvent& event)
{

}
