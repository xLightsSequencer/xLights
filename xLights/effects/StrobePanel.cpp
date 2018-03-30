#include "StrobePanel.h"
#include "../../include/padlock16x16-blue.xpm"
#include "EffectPanelUtils.h"

//(*InternalHeaders(StrobePanel)
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

//(*IdInit(StrobePanel)
const long StrobePanel::ID_STATICTEXT_Number_Strobes = wxNewId();
const long StrobePanel::ID_SLIDER_Number_Strobes = wxNewId();
const long StrobePanel::IDD_TEXTCTRL_Number_Strobes = wxNewId();
const long StrobePanel::ID_BITMAPBUTTON_SLIDER_Number_Strobes = wxNewId();
const long StrobePanel::ID_STATICTEXT_Strobe_Duration = wxNewId();
const long StrobePanel::ID_SLIDER_Strobe_Duration = wxNewId();
const long StrobePanel::IDD_TEXTCTRL_Strobe_Duration = wxNewId();
const long StrobePanel::ID_BITMAPBUTTON_SLIDER_Strobe_Duration = wxNewId();
const long StrobePanel::ID_STATICTEXT_Strobe_Type = wxNewId();
const long StrobePanel::ID_SLIDER_Strobe_Type = wxNewId();
const long StrobePanel::IDD_TEXTCTRL_Strobe_Type = wxNewId();
const long StrobePanel::ID_BITMAPBUTTON_SLIDER_Strobe_Type = wxNewId();
const long StrobePanel::ID_CHECKBOX_Strobe_Music = wxNewId();
const long StrobePanel::ID_BITMAPBUTTON_CHECKBOX_Strobe_Music = wxNewId();
//*)

BEGIN_EVENT_TABLE(StrobePanel,wxPanel)
	//(*EventTable(StrobePanel)
	//*)
END_EVENT_TABLE()

StrobePanel::StrobePanel(wxWindow* parent)
{
	//(*Initialize(StrobePanel)
	wxFlexGridSizer* FlexGridSizer85;
	wxFlexGridSizer* FlexGridSizer87;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer85 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer85->AddGrowableCol(0);
	FlexGridSizer87 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer87->AddGrowableCol(1);
	StaticText110 = new wxStaticText(this, ID_STATICTEXT_Number_Strobes, _("Number Strobes"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Number_Strobes"));
	FlexGridSizer87->Add(StaticText110, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Number_Strobes = new BulkEditSlider(this, ID_SLIDER_Number_Strobes, 3, 1, 300, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Number_Strobes"));
	FlexGridSizer87->Add(Slider_Number_Strobes, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Number_Strobes = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Number_Strobes, _("3"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Number_Strobes"));
	TextCtrl_Number_Strobes->SetMaxLength(3);
	FlexGridSizer87->Add(TextCtrl_Number_Strobes, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_NumberStrobes = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Number_Strobes, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Number_Strobes"));
	BitmapButton_NumberStrobes->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer87->Add(BitmapButton_NumberStrobes, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText111 = new wxStaticText(this, ID_STATICTEXT_Strobe_Duration, _("Strobe Duration"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Strobe_Duration"));
	FlexGridSizer87->Add(StaticText111, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Strobe_Duration = new BulkEditSlider(this, ID_SLIDER_Strobe_Duration, 10, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Strobe_Duration"));
	FlexGridSizer87->Add(Slider_Strobe_Duration, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Strobe_Duration = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Strobe_Duration, _("10"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Strobe_Duration"));
	TextCtrl_Strobe_Duration->SetMaxLength(3);
	FlexGridSizer87->Add(TextCtrl_Strobe_Duration, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_StrobeDuration = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Strobe_Duration, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Strobe_Duration"));
	BitmapButton_StrobeDuration->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer87->Add(BitmapButton_StrobeDuration, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText112 = new wxStaticText(this, ID_STATICTEXT_Strobe_Type, _("Strobe Type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Strobe_Type"));
	FlexGridSizer87->Add(StaticText112, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Strobe_Type = new BulkEditSlider(this, ID_SLIDER_Strobe_Type, 1, 1, 4, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Strobe_Type"));
	FlexGridSizer87->Add(Slider_Strobe_Type, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Strobe_Type = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Strobe_Type, _("1"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Strobe_Type"));
	TextCtrl_Strobe_Type->SetMaxLength(1);
	FlexGridSizer87->Add(TextCtrl_Strobe_Type, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Strobe_Type = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Strobe_Type, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Strobe_Type"));
	BitmapButton_Strobe_Type->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer87->Add(BitmapButton_Strobe_Type, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer87->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Strobe_Music = new BulkEditCheckBox(this, ID_CHECKBOX_Strobe_Music, _("Reacts to music"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Strobe_Music"));
	CheckBox_Strobe_Music->SetValue(false);
	FlexGridSizer87->Add(CheckBox_Strobe_Music, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer87->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Strobe_Music = new wxBitmapButton(this, ID_BITMAPBUTTON_CHECKBOX_Strobe_Music, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Strobe_Music"));
	BitmapButton_Strobe_Music->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer87->Add(BitmapButton_Strobe_Music, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer85->Add(FlexGridSizer87, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer85);
	FlexGridSizer85->Fit(this);
	FlexGridSizer85->SetSizeHints(this);

	Connect(ID_BITMAPBUTTON_SLIDER_Number_Strobes,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&StrobePanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Strobe_Duration,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&StrobePanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Strobe_Type,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&StrobePanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_Strobe_Music,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&StrobePanel::OnLockButtonClick);
	//*)
    SetName("ID_PANEL_STROBE");
}

StrobePanel::~StrobePanel()
{
	//(*Destroy(StrobePanel)
	//*)
}

PANEL_EVENT_HANDLERS(StrobePanel)
void StrobePanel::OnSlider_Strobe_TypeCmdScroll(wxScrollEvent& event)
{
}
