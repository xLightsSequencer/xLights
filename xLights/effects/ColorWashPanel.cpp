#include "ColorWashPanel.h"
#include "EffectPanelUtils.h"
#include "ColorWashEffect.h"

//(*InternalHeaders(ColorWashPanel)
#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#include <wx/checkbox.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>
//*)

//(*IdInit(ColorWashPanel)
const long ColorWashPanel::ID_STATICTEXT_ColorWash_Cycles = wxNewId();
const long ColorWashPanel::IDD_SLIDER_ColorWash_Cycles = wxNewId();
const long ColorWashPanel::ID_VALUECURVE_ColorWash_Cycles = wxNewId();
const long ColorWashPanel::ID_TEXTCTRL_ColorWash_Cycles = wxNewId();
const long ColorWashPanel::ID_BITMAPBUTTON_SLIDER_ColorWash_Cycles = wxNewId();
const long ColorWashPanel::ID_CHECKBOX_ColorWash_VFade = wxNewId();
const long ColorWashPanel::ID_BITMAPBUTTON_CHECKBOX_ColorWash_VFade = wxNewId();
const long ColorWashPanel::ID_CHECKBOX_ColorWash_HFade = wxNewId();
const long ColorWashPanel::ID_BITMAPBUTTON_CHECKBOX_ColorWash_HFade = wxNewId();
const long ColorWashPanel::ID_CHECKBOX_ColorWash_Shimmer = wxNewId();
const long ColorWashPanel::ID_CHECKBOX_ColorWash_CircularPalette = wxNewId();
//*)

BEGIN_EVENT_TABLE(ColorWashPanel,wxPanel)
	//(*EventTable(ColorWashPanel)
	//*)
END_EVENT_TABLE()

ColorWashPanel::ColorWashPanel(wxWindow* parent)
{
	//(*Initialize(ColorWashPanel)
	wxFlexGridSizer* FlexGridSizer114;
	wxFlexGridSizer* FlexGridSizer124;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer37;
	wxFlexGridSizer* FlexGridSizer75;
	wxFlexGridSizer* FlexGridSizer9;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer37 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer37->AddGrowableCol(0);
	FlexGridSizer9 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer9->AddGrowableCol(1);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT_ColorWash_Cycles, _("Count"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_ColorWash_Cycles"));
	FlexGridSizer9->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	SliderCycles = new BulkEditSliderF1(this, IDD_SLIDER_ColorWash_Cycles, 10, 1, 200, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_ColorWash_Cycles"));
	FlexGridSizer1->Add(SliderCycles, 1, wxALL|wxEXPAND, 2);
	BitmapButton_ColorWash_CyclesVC = new BulkEditValueCurveButton(this, ID_VALUECURVE_ColorWash_Cycles, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_ColorWash_Cycles"));
	FlexGridSizer1->Add(BitmapButton_ColorWash_CyclesVC, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer9->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 0);
	CyclesTextCtrl = new BulkEditTextCtrlF1(this, ID_TEXTCTRL_ColorWash_Cycles, _("1.0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_ColorWash_Cycles"));
	CyclesTextCtrl->SetMaxLength(3);
	FlexGridSizer9->Add(CyclesTextCtrl, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_ColorWashCount = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_ColorWash_Cycles, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_ColorWash_Cycles"));
	BitmapButton_ColorWashCount->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer9->Add(BitmapButton_ColorWashCount, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer37->Add(FlexGridSizer9, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer75 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer75->AddGrowableCol(2);
	VFadeCheckBox = new BulkEditCheckBox(this, ID_CHECKBOX_ColorWash_VFade, _("Vertical Fade"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_ColorWash_VFade"));
	VFadeCheckBox->SetValue(false);
	FlexGridSizer75->Add(VFadeCheckBox, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_ColorWashVFade = new xlLockButton(this, ID_BITMAPBUTTON_CHECKBOX_ColorWash_VFade, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_ColorWash_VFade"));
	BitmapButton_ColorWashVFade->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer75->Add(BitmapButton_ColorWashVFade, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	HFadeCheckBox = new BulkEditCheckBox(this, ID_CHECKBOX_ColorWash_HFade, _("Horizontal Fade"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_ColorWash_HFade"));
	HFadeCheckBox->SetValue(false);
	FlexGridSizer75->Add(HFadeCheckBox, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_ColorWashHFade = new xlLockButton(this, ID_BITMAPBUTTON_CHECKBOX_ColorWash_HFade, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_ColorWash_HFade"));
	BitmapButton_ColorWashHFade->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer75->Add(BitmapButton_ColorWashHFade, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer37->Add(FlexGridSizer75, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer114 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer114->AddGrowableCol(0);
	FlexGridSizer124 = new wxFlexGridSizer(0, 3, 0, 0);
	ShimmerCheckBox = new BulkEditCheckBox(this, ID_CHECKBOX_ColorWash_Shimmer, _("Shimmer"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_ColorWash_Shimmer"));
	ShimmerCheckBox->SetValue(false);
	FlexGridSizer124->Add(ShimmerCheckBox, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CircularPaletteCheckBox = new BulkEditCheckBox(this, ID_CHECKBOX_ColorWash_CircularPalette, _("Circular Palette"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_ColorWash_CircularPalette"));
	CircularPaletteCheckBox->SetValue(false);
	FlexGridSizer124->Add(CircularPaletteCheckBox, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer114->Add(FlexGridSizer124, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer37->Add(FlexGridSizer114, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer37);
	FlexGridSizer37->Fit(this);
	FlexGridSizer37->SetSizeHints(this);

	Connect(ID_VALUECURVE_ColorWash_Cycles,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorWashPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_ColorWash_Cycles,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorWashPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_ColorWash_VFade,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorWashPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_ColorWash_HFade,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorWashPanel::OnLockButtonClick);
	//*)

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&ColorWashPanel::OnVCChanged, 0, this);

    BitmapButton_ColorWash_CyclesVC->GetValue()->SetLimits(COLOURWASH_CYCLES_MIN, COLOURWASH_CYCLES_MAX);
    
    SetName("ID_PANEL_COLORWASH");
}

ColorWashPanel::~ColorWashPanel()
{
	//(*Destroy(ColorWashPanel)
	//*)
}

PANEL_EVENT_HANDLERS(ColorWashPanel)
