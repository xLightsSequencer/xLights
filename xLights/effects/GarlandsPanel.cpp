/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "GarlandsPanel.h"
#include "EffectPanelUtils.h"
#include "GarlandsEffect.h"

//(*InternalHeaders(GarlandsPanel)
#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
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

//(*IdInit(GarlandsPanel)
const long GarlandsPanel::ID_STATICTEXT_Garlands_Type = wxNewId();
const long GarlandsPanel::ID_SLIDER_Garlands_Type = wxNewId();
const long GarlandsPanel::IDD_TEXTCTRL_Garlands_Type = wxNewId();
const long GarlandsPanel::ID_BITMAPBUTTON_SLIDER_Garlands_Type = wxNewId();
const long GarlandsPanel::ID_STATICTEXT_Garlands_Spacing = wxNewId();
const long GarlandsPanel::ID_SLIDER_Garlands_Spacing = wxNewId();
const long GarlandsPanel::ID_VALUECURVE_Garlands_Spacing = wxNewId();
const long GarlandsPanel::IDD_TEXTCTRL_Garlands_Spacing = wxNewId();
const long GarlandsPanel::ID_BITMAPBUTTON_SLIDER_Garlands_Spacing = wxNewId();
const long GarlandsPanel::ID_STATICTEXT_Garlands_Cycles = wxNewId();
const long GarlandsPanel::IDD_SLIDER_Garlands_Cycles = wxNewId();
const long GarlandsPanel::ID_VALUECURVE_Garlands_Cycles = wxNewId();
const long GarlandsPanel::ID_TEXTCTRL_Garlands_Cycles = wxNewId();
const long GarlandsPanel::ID_STATICTEXT40 = wxNewId();
const long GarlandsPanel::ID_CHOICE_Garlands_Direction = wxNewId();
//*)

BEGIN_EVENT_TABLE(GarlandsPanel,wxPanel)
	//(*EventTable(GarlandsPanel)
	//*)
END_EVENT_TABLE()

GarlandsPanel::GarlandsPanel(wxWindow* parent) : xlEffectPanel(parent)
{
	//(*Initialize(GarlandsPanel)
	BulkEditTextCtrl* TextCtrl61;
	BulkEditTextCtrl* TextCtrl62;
	BulkEditTextCtrlF1* TextCtrl63;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer39;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer39 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer39->AddGrowableCol(1);
	StaticText32 = new wxStaticText(this, ID_STATICTEXT_Garlands_Type, _("Garland Type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Garlands_Type"));
	FlexGridSizer39->Add(StaticText32, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Garlands_Type = new BulkEditSlider(this, ID_SLIDER_Garlands_Type, 0, 0, 4, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Garlands_Type"));
	FlexGridSizer39->Add(Slider_Garlands_Type, 1, wxALL|wxEXPAND, 2);
	TextCtrl61 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Garlands_Type, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Garlands_Type"));
	TextCtrl61->SetMaxLength(3);
	FlexGridSizer39->Add(TextCtrl61, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_GarlandsType = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Garlands_Type, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Garlands_Type"));
	BitmapButton_GarlandsType->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer39->Add(BitmapButton_GarlandsType, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText33 = new wxStaticText(this, ID_STATICTEXT_Garlands_Spacing, _("Spacing"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Garlands_Spacing"));
	FlexGridSizer39->Add(StaticText33, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	Slider_Garlands_Spacing = new BulkEditSlider(this, ID_SLIDER_Garlands_Spacing, 10, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Garlands_Spacing"));
	FlexGridSizer1->Add(Slider_Garlands_Spacing, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Garlands_SpacingVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Garlands_Spacing, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Garlands_Spacing"));
	FlexGridSizer1->Add(BitmapButton_Garlands_SpacingVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer39->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 0);
	TextCtrl62 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Garlands_Spacing, _("10"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_Garlands_Spacing"));
	TextCtrl62->SetMaxLength(3);
	FlexGridSizer39->Add(TextCtrl62, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_GarlandsSpacing = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Garlands_Spacing, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Garlands_Spacing"));
	BitmapButton_GarlandsSpacing->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer39->Add(BitmapButton_GarlandsSpacing, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText183 = new wxStaticText(this, ID_STATICTEXT_Garlands_Cycles, _("Cycles"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Garlands_Cycles"));
	FlexGridSizer39->Add(StaticText183, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	Slider_Garlands_Cycles = new BulkEditSliderF1(this, IDD_SLIDER_Garlands_Cycles, 10, 0, 200, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Garlands_Cycles"));
	FlexGridSizer2->Add(Slider_Garlands_Cycles, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Garlands_CyclesVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_Garlands_Cycles, GetValueCurveNotSelectedBitmap(), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Garlands_Cycles"));
	FlexGridSizer2->Add(BitmapButton_Garlands_CyclesVC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer39->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 0);
	TextCtrl63 = new BulkEditTextCtrlF1(this, ID_TEXTCTRL_Garlands_Cycles, _("1.0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("ID_TEXTCTRL_Garlands_Cycles"));
	TextCtrl63->SetMaxLength(4);
	FlexGridSizer39->Add(TextCtrl63, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer39->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText192 = new wxStaticText(this, ID_STATICTEXT40, _("Stack Direction"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT40"));
	FlexGridSizer39->Add(StaticText192, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Garlands_Direction = new BulkEditChoice(this, ID_CHOICE_Garlands_Direction, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Garlands_Direction"));
	Choice_Garlands_Direction->SetSelection( Choice_Garlands_Direction->Append(_("Up")) );
	Choice_Garlands_Direction->Append(_("Down"));
	Choice_Garlands_Direction->Append(_("Left"));
	Choice_Garlands_Direction->Append(_("Right"));
	Choice_Garlands_Direction->Append(_("Up then Down"));
	Choice_Garlands_Direction->Append(_("Down then Up"));
	Choice_Garlands_Direction->Append(_("Left then Right"));
	Choice_Garlands_Direction->Append(_("Right then Left"));
	FlexGridSizer39->Add(Choice_Garlands_Direction, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer39);
	FlexGridSizer39->Fit(this);
	FlexGridSizer39->SetSizeHints(this);

	Connect(ID_BITMAPBUTTON_SLIDER_Garlands_Type,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GarlandsPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Garlands_Spacing,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GarlandsPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Garlands_Spacing,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GarlandsPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Garlands_Cycles,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GarlandsPanel::OnVCButtonClick);
	//*)

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&GarlandsPanel::OnVCChanged, 0, this);
    Connect(wxID_ANY, EVT_VALIDATEWINDOW, (wxObjectEventFunction)&GarlandsPanel::OnValidateWindow, 0, this);

    BitmapButton_Garlands_CyclesVC->GetValue()->SetLimits(GARLANDS_CYCLES_MIN, GARLANDS_CYCLES_MAX);
    BitmapButton_Garlands_SpacingVC->GetValue()->SetLimits(GARLANDS_SPACING_MIN, GARLANDS_SPACING_MAX);

    SetName("ID_PANEL_GARLANDS");

    ValidateWindow();
}

GarlandsPanel::~GarlandsPanel()
{
	//(*Destroy(GarlandsPanel)
	//*)
}

void GarlandsPanel::ValidateWindow()
{
}
