#include "StrobePanel.h"
#include "../../include/padlock16x16-blue.xpm"
#include "EffectPanelUtils.h"

//(*InternalHeaders(StrobePanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/bitmap.h>
#include <wx/slider.h>
#include <wx/settings.h>
#include <wx/bmpbuttn.h>
#include <wx/intl.h>
#include <wx/image.h>
#include <wx/string.h>
//*)

//(*IdInit(StrobePanel)
const long StrobePanel::ID_STATICTEXT112 = wxNewId();
const long StrobePanel::ID_SLIDER_Number_Strobes = wxNewId();
const long StrobePanel::ID_BITMAPBUTTON49 = wxNewId();
const long StrobePanel::ID_STATICTEXT113 = wxNewId();
const long StrobePanel::ID_SLIDER_Strobe_Duration = wxNewId();
const long StrobePanel::ID_BITMAPBUTTON51 = wxNewId();
const long StrobePanel::ID_STATICTEXT114 = wxNewId();
const long StrobePanel::ID_SLIDER_Strobe_Type = wxNewId();
const long StrobePanel::ID_BITMAPBUTTON50 = wxNewId();
//*)

BEGIN_EVENT_TABLE(StrobePanel,wxPanel)
	//(*EventTable(StrobePanel)
	//*)
END_EVENT_TABLE()

StrobePanel::StrobePanel(wxWindow* parent)
{
	//(*Initialize(StrobePanel)
	wxFlexGridSizer* FlexGridSizer85;
	wxFlexGridSizer* FlexGridSizer90;
	wxFlexGridSizer* FlexGridSizer88;
	wxFlexGridSizer* FlexGridSizer87;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer85 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer85->AddGrowableCol(0);
	FlexGridSizer87 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer87->AddGrowableCol(1);
	StaticText110 = new wxStaticText(this, ID_STATICTEXT112, _("Number Strobes"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT112"));
	FlexGridSizer87->Add(StaticText110, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Number_Strobes = new wxSlider(this, ID_SLIDER_Number_Strobes, 3, 1, 300, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Number_Strobes"));
	FlexGridSizer87->Add(Slider_Number_Strobes, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_NumberStrobes = new wxBitmapButton(this, ID_BITMAPBUTTON49, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON49"));
	BitmapButton_NumberStrobes->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer87->Add(BitmapButton_NumberStrobes, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer85->Add(FlexGridSizer87, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer88 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer88->AddGrowableCol(1);
	StaticText111 = new wxStaticText(this, ID_STATICTEXT113, _("Strobe Duration"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT113"));
	FlexGridSizer88->Add(StaticText111, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Strobe_Duration = new wxSlider(this, ID_SLIDER_Strobe_Duration, 10, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Strobe_Duration"));
	FlexGridSizer88->Add(Slider_Strobe_Duration, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_StrobeDuration = new wxBitmapButton(this, ID_BITMAPBUTTON51, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON51"));
	BitmapButton_StrobeDuration->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer88->Add(BitmapButton_StrobeDuration, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer85->Add(FlexGridSizer88, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer90 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer90->AddGrowableCol(1);
	StaticText112 = new wxStaticText(this, ID_STATICTEXT114, _("Strobe Type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT114"));
	FlexGridSizer90->Add(StaticText112, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Strobe_Type = new wxSlider(this, ID_SLIDER_Strobe_Type, 1, 1, 4, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Strobe_Type"));
	FlexGridSizer90->Add(Slider_Strobe_Type, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Strobe_Type = new wxBitmapButton(this, ID_BITMAPBUTTON50, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON50"));
	BitmapButton_Strobe_Type->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer90->Add(BitmapButton_Strobe_Type, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer85->Add(FlexGridSizer90, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer85);
	FlexGridSizer85->Fit(this);
	FlexGridSizer85->SetSizeHints(this);

	Connect(ID_BITMAPBUTTON49,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&StrobePanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON51,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&StrobePanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON50,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&StrobePanel::OnLockButtonClick);
	//*)
    SetName("ID_PANEL_STROBE");
}

StrobePanel::~StrobePanel()
{
	//(*Destroy(StrobePanel)
	//*)
}

PANEL_EVENT_HANDLERS(StrobePanel)