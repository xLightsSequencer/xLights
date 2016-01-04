#include "FirePanel.h"
#include "../../include/padlock16x16-blue.xpm"
#include "EffectPanelUtils.h"

//(*InternalHeaders(FirePanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
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
const long FirePanel::IDD_TEXTCTRL_Fire_Height = wxNewId();
const long FirePanel::ID_BITMAPBUTTON_SLIDER_Fire_Height = wxNewId();
const long FirePanel::ID_STATICTEXT133 = wxNewId();
const long FirePanel::ID_SLIDER_Fire_HueShift = wxNewId();
const long FirePanel::IDD_TEXTCTRL_Fire_HueShift = wxNewId();
const long FirePanel::ID_BITMAPBUTTON_SLIDER_Fire_HueShift = wxNewId();
const long FirePanel::IDD_SLIDER_Fire_GrowthCycles = wxNewId();
const long FirePanel::ID_TEXTCTRL_Fire_GrowthCycles = wxNewId();
const long FirePanel::ID_BITMAPBUTTON_CHECKBOX_Fire_GrowFire = wxNewId();
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
	wxTextCtrl* TextCtrl26;
	wxTextCtrl* TextCtrl19;
	wxSlider* Slider9;
	wxTextCtrl* TextCtrl20;
	wxChoice* Choice2;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer38 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer38->AddGrowableCol(1);
	StaticText83 = new wxStaticText(this, ID_STATICTEXT84, _("Height"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT84"));
	FlexGridSizer38->Add(StaticText83, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Fire_Height = new wxSlider(this, ID_SLIDER_Fire_Height, 50, 10, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fire_Height"));
	FlexGridSizer38->Add(Slider_Fire_Height, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl19 = new wxTextCtrl(this, IDD_TEXTCTRL_Fire_Height, _("50"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Fire_Height"));
	TextCtrl19->SetMaxLength(3);
	FlexGridSizer38->Add(TextCtrl19, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_FireHeight = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Fire_Height, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fire_Height"));
	BitmapButton_FireHeight->SetDefault();
	BitmapButton_FireHeight->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer38->Add(BitmapButton_FireHeight, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText132 = new wxStaticText(this, ID_STATICTEXT133, _("Hue Shift"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT133"));
	FlexGridSizer38->Add(StaticText132, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Fire_HueShift = new wxSlider(this, ID_SLIDER_Fire_HueShift, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fire_HueShift"));
	FlexGridSizer38->Add(Slider_Fire_HueShift, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl20 = new wxTextCtrl(this, IDD_TEXTCTRL_Fire_HueShift, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Fire_HueShift"));
	TextCtrl20->SetMaxLength(3);
	FlexGridSizer38->Add(TextCtrl20, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_FireWidth = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Fire_HueShift, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fire_HueShift"));
	BitmapButton_FireWidth->SetDefault();
	BitmapButton_FireWidth->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer38->Add(BitmapButton_FireWidth, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText30 = new wxStaticText(this, wxID_ANY, _("Growth Cycles"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer38->Add(StaticText30, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider9 = new wxSlider(this, IDD_SLIDER_Fire_GrowthCycles, 0, 0, 200, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Fire_GrowthCycles"));
	FlexGridSizer38->Add(Slider9, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl26 = new wxTextCtrl(this, ID_TEXTCTRL_Fire_GrowthCycles, _("0.0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Fire_GrowthCycles"));
	TextCtrl26->SetMaxLength(3);
	FlexGridSizer38->Add(TextCtrl26, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_FireGrow = new wxBitmapButton(this, ID_BITMAPBUTTON_CHECKBOX_Fire_GrowFire, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Fire_GrowFire"));
	BitmapButton_FireGrow->SetDefault();
	BitmapButton_FireGrow->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer38->Add(BitmapButton_FireGrow, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
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

	Connect(ID_SLIDER_Fire_Height,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&FirePanel::UpdateLinkedTextCtrl);
	Connect(IDD_TEXTCTRL_Fire_Height,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&FirePanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_SLIDER_Fire_Height,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FirePanel::OnLockButtonClick);
	Connect(ID_SLIDER_Fire_HueShift,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&FirePanel::UpdateLinkedTextCtrl);
	Connect(IDD_TEXTCTRL_Fire_HueShift,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&FirePanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_SLIDER_Fire_HueShift,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FirePanel::OnLockButtonClick);
	Connect(IDD_SLIDER_Fire_GrowthCycles,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&FirePanel::UpdateLinkedTextCtrlFloat);
	Connect(ID_TEXTCTRL_Fire_GrowthCycles,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&FirePanel::UpdateLinkedSliderFloat);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Fire_GrowFire,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FirePanel::OnLockButtonClick);
	//*)
    SetName("ID_PANEL_FIRE");
}

FirePanel::~FirePanel()
{
	//(*Destroy(FirePanel)
	//*)
}

PANEL_EVENT_HANDLERS(FirePanel)