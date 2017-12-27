#include "TwinklePanel.h"
#include "../../include/padlock16x16-blue.xpm"
#include "EffectPanelUtils.h" 

//(*InternalHeaders(TwinklePanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/bitmap.h>
#include <wx/slider.h>
#include <wx/settings.h>
#include <wx/bmpbuttn.h>
#include <wx/intl.h>
#include <wx/image.h>
#include <wx/string.h>
//*)

//(*IdInit(TwinklePanel)
const long TwinklePanel::ID_STATICTEXT_Twinkle_Count = wxNewId();
const long TwinklePanel::ID_SLIDER_Twinkle_Count = wxNewId();
const long TwinklePanel::IDD_TEXTCTRL_Twinkle_Count = wxNewId();
const long TwinklePanel::ID_BITMAPBUTTON_SLIDER_Twinkle_Count = wxNewId();
const long TwinklePanel::ID_STATICTEXT_Twinkle_Steps = wxNewId();
const long TwinklePanel::ID_SLIDER_Twinkle_Steps = wxNewId();
const long TwinklePanel::IDD_TEXTCTRL_Twinkle_Steps = wxNewId();
const long TwinklePanel::ID_BITMAPBUTTON_SLIDER_Twinkle_Steps = wxNewId();
const long TwinklePanel::ID_CHECKBOX_Twinkle_Strobe = wxNewId();
const long TwinklePanel::ID_BITMAPBUTTON_CHECKBOX_Twinkle_Strobe = wxNewId();
const long TwinklePanel::ID_CHECKBOX_Twinkle_ReRandom = wxNewId();
const long TwinklePanel::ID_BITMAPBUTTON_CHECKBOX_Twinkle_ReRandom = wxNewId();
//*)

BEGIN_EVENT_TABLE(TwinklePanel,wxPanel)
	//(*EventTable(TwinklePanel)
	//*)
END_EVENT_TABLE()

TwinklePanel::TwinklePanel(wxWindow* parent)
{
	//(*Initialize(TwinklePanel)
	wxFlexGridSizer* FlexGridSizer4;
	BulkEditTextCtrl* TextCtrl80;
	BulkEditTextCtrl* TextCtrl79;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer4 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer4->AddGrowableCol(1);
	StaticText85 = new wxStaticText(this, ID_STATICTEXT_Twinkle_Count, _("Number Lights"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Twinkle_Count"));
	FlexGridSizer4->Add(StaticText85, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Twinkle_Count = new BulkEditSlider(this, ID_SLIDER_Twinkle_Count, 3, 2, 100, wxDefaultPosition, wxSize(117,24), 0, wxDefaultValidator, _T("ID_SLIDER_Twinkle_Count"));
	FlexGridSizer4->Add(Slider_Twinkle_Count, 1, wxALL|wxEXPAND, 5);
	TextCtrl79 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Twinkle_Count, _("3"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Twinkle_Count"));
	TextCtrl79->SetMaxLength(3);
	FlexGridSizer4->Add(TextCtrl79, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_TwinkleCount = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Twinkle_Count, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Twinkle_Count"));
	BitmapButton_TwinkleCount->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer4->Add(BitmapButton_TwinkleCount, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText104 = new wxStaticText(this, ID_STATICTEXT_Twinkle_Steps, _("Twinkle Steps"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Twinkle_Steps"));
	FlexGridSizer4->Add(StaticText104, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Twinkle_Steps = new BulkEditSlider(this, ID_SLIDER_Twinkle_Steps, 30, 2, 200, wxDefaultPosition, wxSize(119,24), 0, wxDefaultValidator, _T("ID_SLIDER_Twinkle_Steps"));
	FlexGridSizer4->Add(Slider_Twinkle_Steps, 1, wxALL|wxEXPAND, 5);
	TextCtrl80 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Twinkle_Steps, _("30"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Twinkle_Steps"));
	TextCtrl80->SetMaxLength(3);
	FlexGridSizer4->Add(TextCtrl80, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_TwinkleSteps = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Twinkle_Steps, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Twinkle_Steps"));
	BitmapButton_TwinkleSteps->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer4->Add(BitmapButton_TwinkleSteps, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer4->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Twinkle_Strobe = new BulkEditCheckBox(this, ID_CHECKBOX_Twinkle_Strobe, _("Strobe"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Twinkle_Strobe"));
	CheckBox_Twinkle_Strobe->SetValue(false);
	FlexGridSizer4->Add(CheckBox_Twinkle_Strobe, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_TwinkleStrobe = new wxBitmapButton(this, ID_BITMAPBUTTON_CHECKBOX_Twinkle_Strobe, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Twinkle_Strobe"));
	BitmapButton_TwinkleStrobe->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer4->Add(BitmapButton_TwinkleStrobe, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer4->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Twinkle_ReRandom = new BulkEditCheckBox(this, ID_CHECKBOX_Twinkle_ReRandom, _("Re-Randomize after twinkle"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Twinkle_ReRandom"));
	CheckBox_Twinkle_ReRandom->SetValue(false);
	FlexGridSizer4->Add(CheckBox_Twinkle_ReRandom, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton24 = new wxBitmapButton(this, ID_BITMAPBUTTON_CHECKBOX_Twinkle_ReRandom, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Twinkle_ReRandom"));
	BitmapButton24->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer4->Add(BitmapButton24, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer4);
	FlexGridSizer4->Fit(this);
	FlexGridSizer4->SetSizeHints(this);

	Connect(ID_BITMAPBUTTON_SLIDER_Twinkle_Count,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TwinklePanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Twinkle_Steps,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TwinklePanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Twinkle_Strobe,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TwinklePanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Twinkle_ReRandom,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TwinklePanel::OnLockButtonClick);
	//*)
    SetName("ID_PANEL_TWINKLE");
}

TwinklePanel::~TwinklePanel()
{
	//(*Destroy(TwinklePanel)
	//*)
}

PANEL_EVENT_HANDLERS(TwinklePanel)