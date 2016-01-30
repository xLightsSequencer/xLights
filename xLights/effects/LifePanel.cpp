#include "LifePanel.h"
#include "../../include/padlock16x16-blue.xpm"
#include "EffectPanelUtils.h"

//(*InternalHeaders(LifePanel)
#include <wx/bmpbuttn.h>
#include <wx/sizer.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/slider.h>
#include <wx/intl.h>
#include <wx/stattext.h>
#include <wx/bitmap.h>
#include <wx/textctrl.h>
#include <wx/image.h>
//*)

//(*IdInit(LifePanel)
const long LifePanel::ID_STATICTEXT36 = wxNewId();
const long LifePanel::ID_SLIDER_Life_Count = wxNewId();
const long LifePanel::IDD_TEXTCTRL_Life_Count = wxNewId();
const long LifePanel::ID_BITMAPBUTTON_SLIDER_Life_Count = wxNewId();
const long LifePanel::ID_STATICTEXT37 = wxNewId();
const long LifePanel::ID_SLIDER_Life_Seed = wxNewId();
const long LifePanel::IDD_TEXTCTRL_Life_Seed = wxNewId();
const long LifePanel::ID_BITMAPBUTTON_SLIDER_Life_Seed = wxNewId();
const long LifePanel::ID_SLIDER_Life_Speed = wxNewId();
const long LifePanel::IDD_TEXTCTRL_Life_Speed = wxNewId();
//*)

BEGIN_EVENT_TABLE(LifePanel,wxPanel)
	//(*EventTable(LifePanel)
	//*)
END_EVENT_TABLE()

LifePanel::LifePanel(wxWindow* parent)
{
	//(*Initialize(LifePanel)
	wxTextCtrl* TextCtrl32;
	wxFlexGridSizer* FlexGridSizer40;
	wxTextCtrl* TextCtrl31;
	wxSlider* Slider11;
	wxStaticText* StaticText134;
	wxTextCtrl* TextCtrl30;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer40 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer40->AddGrowableCol(1);
	StaticText35 = new wxStaticText(this, ID_STATICTEXT36, _("Cells to start"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT36"));
	FlexGridSizer40->Add(StaticText35, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Life_Count = new wxSlider(this, ID_SLIDER_Life_Count, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Life_Count"));
	FlexGridSizer40->Add(Slider_Life_Count, 1, wxALL|wxEXPAND, 2);
	TextCtrl32 = new wxTextCtrl(this, IDD_TEXTCTRL_Life_Count, _("50"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Life_Count"));
	TextCtrl32->SetMaxLength(3);
	FlexGridSizer40->Add(TextCtrl32, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_LifeCount = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Life_Count, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Life_Count"));
	BitmapButton_LifeCount->SetDefault();
	BitmapButton_LifeCount->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer40->Add(BitmapButton_LifeCount, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText37 = new wxStaticText(this, ID_STATICTEXT37, _("Type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT37"));
	FlexGridSizer40->Add(StaticText37, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Life_Seed = new wxSlider(this, ID_SLIDER_Life_Seed, 0, 0, 4, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Life_Seed"));
	FlexGridSizer40->Add(Slider_Life_Seed, 1, wxALL|wxEXPAND, 2);
	TextCtrl31 = new wxTextCtrl(this, IDD_TEXTCTRL_Life_Seed, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Life_Seed"));
	TextCtrl31->SetMaxLength(2);
	FlexGridSizer40->Add(TextCtrl31, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_LifeSpeed = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Life_Seed, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Life_Seed"));
	BitmapButton_LifeSpeed->SetDefault();
	BitmapButton_LifeSpeed->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer40->Add(BitmapButton_LifeSpeed, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText134 = new wxStaticText(this, wxID_ANY, _("Speed"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer40->Add(StaticText134, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider11 = new wxSlider(this, ID_SLIDER_Life_Speed, 10, 1, 30, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Life_Speed"));
	FlexGridSizer40->Add(Slider11, 1, wxALL|wxEXPAND, 2);
	TextCtrl30 = new wxTextCtrl(this, IDD_TEXTCTRL_Life_Speed, _("10"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Life_Speed"));
	TextCtrl30->SetMaxLength(2);
	FlexGridSizer40->Add(TextCtrl30, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	SetSizer(FlexGridSizer40);
	FlexGridSizer40->Fit(this);
	FlexGridSizer40->SetSizeHints(this);

	Connect(ID_SLIDER_Life_Count,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&LifePanel::UpdateLinkedTextCtrl);
	Connect(IDD_TEXTCTRL_Life_Count,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&LifePanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_SLIDER_Life_Count,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LifePanel::OnLockButtonClick);
	Connect(ID_SLIDER_Life_Seed,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&LifePanel::UpdateLinkedTextCtrl);
	Connect(IDD_TEXTCTRL_Life_Seed,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&LifePanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_SLIDER_Life_Seed,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LifePanel::OnLockButtonClick);
	Connect(ID_SLIDER_Life_Speed,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&LifePanel::UpdateLinkedTextCtrl);
	Connect(IDD_TEXTCTRL_Life_Speed,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&LifePanel::UpdateLinkedSlider);
	//*)
    SetName("ID_PANEL_LIFE");
}

LifePanel::~LifePanel()
{
	//(*Destroy(LifePanel)
	//*)
}

PANEL_EVENT_HANDLERS(LifePanel)