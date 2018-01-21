#include "SnowstormPanel.h"
#include "../../include/padlock16x16-blue.xpm"
#include "EffectPanelUtils.h"

//(*InternalHeaders(SnowstormPanel)
#include <wx/bmpbuttn.h>
#include <wx/sizer.h>
#include <wx/string.h>
#include <wx/slider.h>
#include <wx/intl.h>
#include <wx/stattext.h>
#include <wx/bitmap.h>
#include <wx/textctrl.h>
#include <wx/image.h>
//*)

//(*IdInit(SnowstormPanel)
const long SnowstormPanel::ID_STATICTEXT_Snowstorm_Count = wxNewId();
const long SnowstormPanel::ID_SLIDER_Snowstorm_Count = wxNewId();
const long SnowstormPanel::IDD_TEXTCTRL_Snowstorm_Count = wxNewId();
const long SnowstormPanel::ID_BITMAPBUTTON_SLIDER_Snowstorm_Count = wxNewId();
const long SnowstormPanel::ID_STATICTEXT_Snowstorm_Length = wxNewId();
const long SnowstormPanel::ID_SLIDER_Snowstorm_Length = wxNewId();
const long SnowstormPanel::IDD_TEXTCTRL_Snowstorm_Length = wxNewId();
const long SnowstormPanel::ID_BITMAPBUTTON_SLIDER_Snowstorm_Length = wxNewId();
const long SnowstormPanel::ID_STATICTEXT_Snowstorm_Speed = wxNewId();
const long SnowstormPanel::ID_SLIDER_Snowstorm_Speed = wxNewId();
const long SnowstormPanel::IDD_TEXTCTRL_Snowstorm_Speed = wxNewId();
const long SnowstormPanel::ID_BITMAPBUTTON_SLIDER_Snowstorm_Speed = wxNewId();
//*)

BEGIN_EVENT_TABLE(SnowstormPanel,wxPanel)
	//(*EventTable(SnowstormPanel)
	//*)
END_EVENT_TABLE()

SnowstormPanel::SnowstormPanel(wxWindow* parent)
{
	//(*Initialize(SnowstormPanel)
	BulkEditTextCtrl* TextCtrl54;
	BulkEditTextCtrl* TextCtrl53;
	wxFlexGridSizer* FlexGridSizer44;
	BulkEditTextCtrl* TextCtrl55;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer44 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer44->AddGrowableCol(1);
	StaticText45 = new wxStaticText(this, ID_STATICTEXT_Snowstorm_Count, _("Max flakes"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Snowstorm_Count"));
	FlexGridSizer44->Add(StaticText45, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Snowstorm_Count = new BulkEditSlider(this, ID_SLIDER_Snowstorm_Count, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Snowstorm_Count"));
	FlexGridSizer44->Add(Slider_Snowstorm_Count, 1, wxALL|wxEXPAND, 2);
	TextCtrl53 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Snowstorm_Count, _("50"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Snowstorm_Count"));
	TextCtrl53->SetMaxLength(3);
	FlexGridSizer44->Add(TextCtrl53, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_SnowstormCount = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Snowstorm_Count, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Snowstorm_Count"));
	FlexGridSizer44->Add(BitmapButton_SnowstormCount, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText51 = new wxStaticText(this, ID_STATICTEXT_Snowstorm_Length, _("Trail Length"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Snowstorm_Length"));
	FlexGridSizer44->Add(StaticText51, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Snowstorm_Length = new BulkEditSlider(this, ID_SLIDER_Snowstorm_Length, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Snowstorm_Length"));
	FlexGridSizer44->Add(Slider_Snowstorm_Length, 1, wxALL|wxEXPAND, 2);
	TextCtrl54 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Snowstorm_Length, _("50"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Snowstorm_Length"));
	TextCtrl54->SetMaxLength(3);
	FlexGridSizer44->Add(TextCtrl54, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_SnowstormLength = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Snowstorm_Length, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Snowstorm_Length"));
	FlexGridSizer44->Add(BitmapButton_SnowstormLength, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText180 = new wxStaticText(this, ID_STATICTEXT_Snowstorm_Speed, _("Speed"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Snowstorm_Speed"));
	FlexGridSizer44->Add(StaticText180, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Snowstorm_Speed = new BulkEditSlider(this, ID_SLIDER_Snowstorm_Speed, 10, 1, 50, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Snowstorm_Speed"));
	FlexGridSizer44->Add(Slider_Snowstorm_Speed, 1, wxALL|wxEXPAND, 5);
	TextCtrl55 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Snowstorm_Speed, _("10"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Snowstorm_Speed"));
	TextCtrl55->SetMaxLength(3);
	FlexGridSizer44->Add(TextCtrl55, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_SnowstormSpeed = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Snowstorm_Speed, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Snowstorm_Speed"));
	FlexGridSizer44->Add(BitmapButton_SnowstormSpeed, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer44);
	FlexGridSizer44->Fit(this);
	FlexGridSizer44->SetSizeHints(this);

	Connect(ID_BITMAPBUTTON_SLIDER_Snowstorm_Count,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SnowstormPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Snowstorm_Length,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SnowstormPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Snowstorm_Speed,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SnowstormPanel::OnLockButtonClick);
	//*)
    SetName("ID_PANEL_SNOWSTORM");

}

SnowstormPanel::~SnowstormPanel()
{
	//(*Destroy(SnowstormPanel)
	//*)
}

PANEL_EVENT_HANDLERS(SnowstormPanel)