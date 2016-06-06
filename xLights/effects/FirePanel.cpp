#include "FirePanel.h"
#include "../../include/padlock16x16-blue.xpm"
#include "EffectPanelUtils.h"

//(*InternalHeaders(FirePanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/bitmap.h>
#include <wx/slider.h>
#include <wx/settings.h>
#include <wx/choice.h>
#include <wx/bmpbuttn.h>
#include <wx/intl.h>
#include <wx/image.h>
#include <wx/string.h>
//*)

//(*IdInit(FirePanel)
const long FirePanel::ID_STATICTEXT84 = wxNewId();
const long FirePanel::ID_SLIDER_Fire_Height = wxNewId();
const long FirePanel::ID_VALUECURVE_Fire_Height = wxNewId();
const long FirePanel::IDD_TEXTCTRL_Fire_Height = wxNewId();
const long FirePanel::ID_BITMAPBUTTON_SLIDER_Fire_Height = wxNewId();
const long FirePanel::ID_STATICTEXT133 = wxNewId();
const long FirePanel::ID_SLIDER_Fire_HueShift = wxNewId();
const long FirePanel::ID_VALUECURVE_Fire_HueShift = wxNewId();
const long FirePanel::IDD_TEXTCTRL_Fire_HueShift = wxNewId();
const long FirePanel::ID_BITMAPBUTTON_SLIDER_Fire_HueShift = wxNewId();
const long FirePanel::IDD_SLIDER_Fire_GrowthCycles = wxNewId();
const long FirePanel::ID_VALUECURVE_Fire_GrowthCycles = wxNewId();
const long FirePanel::ID_TEXTCTRL_Fire_GrowthCycles = wxNewId();
const long FirePanel::ID_BITMAPBUTTON_CHECKBOX_Fire_GrowFire = wxNewId();
const long FirePanel::ID_CHECKBOX_Fire_GrowWithMusic = wxNewId();
const long FirePanel::ID_BITMAPBUTTON_Fire_GrowWithMusic = wxNewId();
const long FirePanel::ID_CHOICE_Fire_Location = wxNewId();
//*)

BEGIN_EVENT_TABLE(FirePanel,wxPanel)
	//(*EventTable(FirePanel)
	//*)
END_EVENT_TABLE()

FirePanel::FirePanel(wxWindow* parent)
{
	//(*Initialize(FirePanel)
	wxStaticText* StaticText193;
	wxSlider* Slider_Fire_HueShift;
	wxFlexGridSizer* FlexGridSizer38;
	wxStaticText* StaticText30;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer2;
	wxTextCtrl* TextCtrl20;
	wxFlexGridSizer* FlexGridSizer1;
	wxChoice* Choice2;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer38 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer38->AddGrowableCol(1);
	StaticText83 = new wxStaticText(this, ID_STATICTEXT84, _("Height"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT84"));
	FlexGridSizer38->Add(StaticText83, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	Slider_Fire_Height = new wxSlider(this, ID_SLIDER_Fire_Height, 50, 10, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fire_Height"));
	FlexGridSizer1->Add(Slider_Fire_Height, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Fire_HeightVC = new ValueCurveButton(this, ID_VALUECURVE_Fire_Height, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Fire_Height"));
	FlexGridSizer1->Add(BitmapButton_Fire_HeightVC, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer38->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 0);
	TextCtrl_Fire_Height = new wxTextCtrl(this, IDD_TEXTCTRL_Fire_Height, _("50"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Fire_Height"));
	TextCtrl_Fire_Height->SetMaxLength(3);
	FlexGridSizer38->Add(TextCtrl_Fire_Height, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_FireHeight = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Fire_Height, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fire_Height"));
	BitmapButton_FireHeight->SetDefault();
	BitmapButton_FireHeight->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer38->Add(BitmapButton_FireHeight, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText132 = new wxStaticText(this, ID_STATICTEXT133, _("Hue Shift"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT133"));
	FlexGridSizer38->Add(StaticText132, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	Slider_Fire_HueShift = new wxSlider(this, ID_SLIDER_Fire_HueShift, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fire_HueShift"));
	FlexGridSizer2->Add(Slider_Fire_HueShift, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Fire_HueShiftVC = new ValueCurveButton(this, ID_VALUECURVE_Fire_HueShift, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Fire_HueShift"));
	FlexGridSizer2->Add(BitmapButton_Fire_HueShiftVC, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer38->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 0);
	TextCtrl20 = new wxTextCtrl(this, IDD_TEXTCTRL_Fire_HueShift, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Fire_HueShift"));
	TextCtrl20->SetMaxLength(3);
	FlexGridSizer38->Add(TextCtrl20, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_FireWidth = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Fire_HueShift, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fire_HueShift"));
	BitmapButton_FireWidth->SetDefault();
	BitmapButton_FireWidth->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer38->Add(BitmapButton_FireWidth, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText30 = new wxStaticText(this, wxID_ANY, _("Growth Cycles"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer38->Add(StaticText30, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	Slider_Fire_GrowthCycles = new wxSlider(this, IDD_SLIDER_Fire_GrowthCycles, 0, 0, 200, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Fire_GrowthCycles"));
	FlexGridSizer3->Add(Slider_Fire_GrowthCycles, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Fire_GrowthCyclesVC = new ValueCurveButton(this, ID_VALUECURVE_Fire_GrowthCycles, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Fire_GrowthCycles"));
	FlexGridSizer3->Add(BitmapButton_Fire_GrowthCyclesVC, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer38->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 0);
	TextCtrl_Fire_GrowthCycles = new wxTextCtrl(this, ID_TEXTCTRL_Fire_GrowthCycles, _("0.0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Fire_GrowthCycles"));
	TextCtrl_Fire_GrowthCycles->SetMaxLength(3);
	FlexGridSizer38->Add(TextCtrl_Fire_GrowthCycles, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_FireGrow = new wxBitmapButton(this, ID_BITMAPBUTTON_CHECKBOX_Fire_GrowFire, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Fire_GrowFire"));
	BitmapButton_FireGrow->SetDefault();
	BitmapButton_FireGrow->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer38->Add(BitmapButton_FireGrow, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer38->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Fire_GrowWithMusic = new wxCheckBox(this, ID_CHECKBOX_Fire_GrowWithMusic, _("Grow with music"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Fire_GrowWithMusic"));
	CheckBox_Fire_GrowWithMusic->SetValue(false);
	FlexGridSizer38->Add(CheckBox_Fire_GrowWithMusic, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer38->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Fire_GrowWithMusic = new wxBitmapButton(this, ID_BITMAPBUTTON_Fire_GrowWithMusic, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Fire_GrowWithMusic"));
	BitmapButton_Fire_GrowWithMusic->SetDefault();
	BitmapButton_Fire_GrowWithMusic->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer38->Add(BitmapButton_Fire_GrowWithMusic, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText193 = new wxStaticText(this, wxID_ANY, _("Location"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer38->Add(StaticText193, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice2 = new wxChoice(this, ID_CHOICE_Fire_Location, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Fire_Location"));
	Choice2->SetSelection( Choice2->Append(_("Bottom")) );
	Choice2->Append(_("Top"));
	Choice2->Append(_("Left"));
	Choice2->Append(_("Right"));
	FlexGridSizer38->Add(Choice2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer38);
	FlexGridSizer38->Fit(this);
	FlexGridSizer38->SetSizeHints(this);

	Connect(ID_SLIDER_Fire_Height,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&FirePanel::UpdateLinkedTextCtrlVC);
	Connect(ID_VALUECURVE_Fire_Height,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FirePanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_Fire_Height,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&FirePanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_SLIDER_Fire_Height,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FirePanel::OnLockButtonClick);
	Connect(ID_SLIDER_Fire_HueShift,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&FirePanel::UpdateLinkedTextCtrlVC);
	Connect(ID_VALUECURVE_Fire_HueShift,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FirePanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_Fire_HueShift,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&FirePanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_SLIDER_Fire_HueShift,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FirePanel::OnLockButtonClick);
	Connect(IDD_SLIDER_Fire_GrowthCycles,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&FirePanel::UpdateLinkedTextCtrlFloatVC);
	Connect(ID_VALUECURVE_Fire_GrowthCycles,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FirePanel::OnVCButtonClick);
	Connect(ID_TEXTCTRL_Fire_GrowthCycles,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&FirePanel::UpdateLinkedSliderFloat);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Fire_GrowFire,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FirePanel::OnLockButtonClick);
	Connect(ID_CHECKBOX_Fire_GrowWithMusic,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&FirePanel::OnCheckBox_Fire_GrowWithMusicClick);
	Connect(ID_BITMAPBUTTON_Fire_GrowWithMusic,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FirePanel::OnLockButtonClick);
	//*)

    BitmapButton_Fire_GrowthCyclesVC->GetValue()->SetLimits(0, 20);
    BitmapButton_Fire_HeightVC->GetValue()->SetLimits(10, 100);
    BitmapButton_Fire_HueShiftVC->GetValue()->SetLimits(0, 100);

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
