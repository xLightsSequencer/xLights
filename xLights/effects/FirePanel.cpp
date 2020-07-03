/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "FirePanel.h"
#include "EffectPanelUtils.h"
#include "FireEffect.h"

//(*InternalHeaders(FirePanel)
#include <wx/artprov.h>
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

//(*IdInit(FirePanel)
const long FirePanel::ID_STATICTEXT_Fire_Height = wxNewId();
const long FirePanel::ID_SLIDER_Fire_Height = wxNewId();
const long FirePanel::ID_VALUECURVE_Fire_Height = wxNewId();
const long FirePanel::IDD_TEXTCTRL_Fire_Height = wxNewId();
const long FirePanel::ID_BITMAPBUTTON_SLIDER_Fire_Height = wxNewId();
const long FirePanel::ID_STATICTEXT_Fire_HueShift = wxNewId();
const long FirePanel::ID_SLIDER_Fire_HueShift = wxNewId();
const long FirePanel::ID_VALUECURVE_Fire_HueShift = wxNewId();
const long FirePanel::IDD_TEXTCTRL_Fire_HueShift = wxNewId();
const long FirePanel::ID_BITMAPBUTTON_SLIDER_Fire_HueShift = wxNewId();
const long FirePanel::ID_STATICTEXT_Fire_GrowthCycles = wxNewId();
const long FirePanel::IDD_SLIDER_Fire_GrowthCycles = wxNewId();
const long FirePanel::ID_VALUECURVE_Fire_GrowthCycles = wxNewId();
const long FirePanel::ID_TEXTCTRL_Fire_GrowthCycles = wxNewId();
const long FirePanel::ID_BITMAPBUTTON_SLIDER_Fire_GrowthCycles = wxNewId();
const long FirePanel::ID_CHECKBOX_Fire_GrowWithMusic = wxNewId();
const long FirePanel::ID_BITMAPBUTTON_CHECKBOX_Fire_GrowWithMusic = wxNewId();
const long FirePanel::ID_STATICTEXT_Fire_Location = wxNewId();
const long FirePanel::ID_CHOICE_Fire_Location = wxNewId();
//*)

BEGIN_EVENT_TABLE(FirePanel,wxPanel)
	//(*EventTable(FirePanel)
	//*)
END_EVENT_TABLE()

FirePanel::FirePanel(wxWindow* parent)
{
	//(*Initialize(FirePanel)
	BulkEditTextCtrl* TextCtrl20;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer38;
	wxFlexGridSizer* FlexGridSizer3;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer38 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer38->AddGrowableCol(1);
	StaticText83 = new wxStaticText(this, ID_STATICTEXT_Fire_Height, _("Height"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fire_Height"));
	FlexGridSizer38->Add(StaticText83, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	Slider_Fire_Height = new BulkEditSlider(this, ID_SLIDER_Fire_Height, 50, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fire_Height"));
	FlexGridSizer1->Add(Slider_Fire_Height, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Fire_HeightVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Fire_Height, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Fire_Height"));
	FlexGridSizer1->Add(BitmapButton_Fire_HeightVC, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer38->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 0);
	TextCtrl_Fire_Height = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Fire_Height, _("50"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Fire_Height"));
	TextCtrl_Fire_Height->SetMaxLength(3);
	FlexGridSizer38->Add(TextCtrl_Fire_Height, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_FireHeight = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Fire_Height, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fire_Height"));
	BitmapButton_FireHeight->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer38->Add(BitmapButton_FireHeight, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText132 = new wxStaticText(this, ID_STATICTEXT_Fire_HueShift, _("Hue Shift"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fire_HueShift"));
	FlexGridSizer38->Add(StaticText132, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	Slider_Fire_HueShift = new BulkEditSlider(this, ID_SLIDER_Fire_HueShift, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fire_HueShift"));
	FlexGridSizer2->Add(Slider_Fire_HueShift, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Fire_HueShiftVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Fire_HueShift, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Fire_HueShift"));
	FlexGridSizer2->Add(BitmapButton_Fire_HueShiftVC, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer38->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 0);
	TextCtrl20 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Fire_HueShift, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Fire_HueShift"));
	TextCtrl20->SetMaxLength(3);
	FlexGridSizer38->Add(TextCtrl20, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_FireWidth = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Fire_HueShift, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fire_HueShift"));
	BitmapButton_FireWidth->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer38->Add(BitmapButton_FireWidth, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText30 = new wxStaticText(this, ID_STATICTEXT_Fire_GrowthCycles, _("Growth Cycles"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fire_GrowthCycles"));
	FlexGridSizer38->Add(StaticText30, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	Slider_Fire_GrowthCycles = new BulkEditSliderF1(this, IDD_SLIDER_Fire_GrowthCycles, 0, 0, 200, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Fire_GrowthCycles"));
	FlexGridSizer3->Add(Slider_Fire_GrowthCycles, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Fire_GrowthCyclesVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Fire_GrowthCycles, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Fire_GrowthCycles"));
	FlexGridSizer3->Add(BitmapButton_Fire_GrowthCyclesVC, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer38->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 0);
	TextCtrl_Fire_GrowthCycles = new BulkEditTextCtrlF1(this, ID_TEXTCTRL_Fire_GrowthCycles, _("0.0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Fire_GrowthCycles"));
	TextCtrl_Fire_GrowthCycles->SetMaxLength(3);
	FlexGridSizer38->Add(TextCtrl_Fire_GrowthCycles, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_FireGrow = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Fire_GrowthCycles, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fire_GrowthCycles"));
	BitmapButton_FireGrow->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer38->Add(BitmapButton_FireGrow, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer38->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Fire_GrowWithMusic = new BulkEditCheckBox(this, ID_CHECKBOX_Fire_GrowWithMusic, _("Grow with music"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Fire_GrowWithMusic"));
	CheckBox_Fire_GrowWithMusic->SetValue(false);
	FlexGridSizer38->Add(CheckBox_Fire_GrowWithMusic, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer38->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Fire_GrowWithMusic = new xlLockButton(this, ID_BITMAPBUTTON_CHECKBOX_Fire_GrowWithMusic, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Fire_GrowWithMusic"));
	BitmapButton_Fire_GrowWithMusic->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer38->Add(BitmapButton_Fire_GrowWithMusic, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText193 = new wxStaticText(this, ID_STATICTEXT_Fire_Location, _("Location"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fire_Location"));
	FlexGridSizer38->Add(StaticText193, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Fire_Location = new BulkEditChoice(this, ID_CHOICE_Fire_Location, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Fire_Location"));
	Choice_Fire_Location->SetSelection( Choice_Fire_Location->Append(_("Bottom")) );
	Choice_Fire_Location->Append(_("Top"));
	Choice_Fire_Location->Append(_("Left"));
	Choice_Fire_Location->Append(_("Right"));
	FlexGridSizer38->Add(Choice_Fire_Location, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer38);
	FlexGridSizer38->Fit(this);
	FlexGridSizer38->SetSizeHints(this);

	Connect(ID_VALUECURVE_Fire_Height,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FirePanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fire_Height,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FirePanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Fire_HueShift,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FirePanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fire_HueShift,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FirePanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Fire_GrowthCycles,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FirePanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fire_GrowthCycles,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FirePanel::OnLockButtonClick);
	Connect(ID_CHECKBOX_Fire_GrowWithMusic,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&FirePanel::OnCheckBox_Fire_GrowWithMusicClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Fire_GrowWithMusic,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FirePanel::OnLockButtonClick);
	//*)

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&FirePanel::OnVCChanged, 0, this);

    BitmapButton_Fire_GrowthCyclesVC->GetValue()->SetLimits(FIRE_GROWTHCYCLES_MIN, FIRE_GROWTHCYCLES_MAX);
    BitmapButton_Fire_GrowthCyclesVC->GetValue()->SetDivisor(FIRE_GROWTHCYCLES_DIVISOR);
    BitmapButton_Fire_HeightVC->GetValue()->SetLimits(FIRE_HEIGHT_MIN, FIRE_HEIGHT_MAX);
    BitmapButton_Fire_HueShiftVC->GetValue()->SetLimits(FIRE_HUE_MIN, FIRE_HUE_MAX);

    SetName("ID_PANEL_FIRE");
    ValidateWindow();
}

FirePanel::~FirePanel()
{
	//(*Destroy(FirePanel)
	//*)
}

PANEL_EVENT_HANDLERS(FirePanel)
void FirePanel::OnCheckBox_Fire_GrowWithMusicClick(wxCommandEvent& event)
{
    ValidateWindow();
}

void FirePanel::ValidateWindow()
{
    if (CheckBox_Fire_GrowWithMusic->GetValue())
    {
        Slider_Fire_Height->Disable();
        Slider_Fire_GrowthCycles->Disable();
        TextCtrl_Fire_Height->Disable();
        TextCtrl_Fire_GrowthCycles->Disable();
    }
    else
    {
        Slider_Fire_Height->Enable();
        Slider_Fire_GrowthCycles->Enable();
        TextCtrl_Fire_Height->Enable();
        TextCtrl_Fire_GrowthCycles->Enable();
    }
}
