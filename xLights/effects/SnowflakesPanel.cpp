#include "SnowflakesPanel.h"
#include "../../include/padlock16x16-blue.xpm"
#include "EffectPanelUtils.h"

//(*InternalHeaders(SnowflakesPanel)
#include <wx/bmpbuttn.h>
#include <wx/sizer.h>
#include <wx/string.h>
#include <wx/slider.h>
#include <wx/intl.h>
#include <wx/stattext.h>
#include <wx/bitmap.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/image.h>
//*)

//(*IdInit(SnowflakesPanel)
const long SnowflakesPanel::ID_STATICTEXT_Snowflakes_Count = wxNewId();
const long SnowflakesPanel::ID_SLIDER_Snowflakes_Count = wxNewId();
const long SnowflakesPanel::IDD_TEXTCTRL_Snowflakes_Count = wxNewId();
const long SnowflakesPanel::ID_BITMAPBUTTON_SLIDER_Snowflakes_Count = wxNewId();
const long SnowflakesPanel::ID_STATICTEXT_Snowflakes_Type = wxNewId();
const long SnowflakesPanel::ID_SLIDER_Snowflakes_Type = wxNewId();
const long SnowflakesPanel::IDD_TEXTCTRL_Snowflakes_Type = wxNewId();
const long SnowflakesPanel::ID_BITMAPBUTTON_SLIDER_Snowflakes_Type = wxNewId();
const long SnowflakesPanel::ID_STATICTEXT_Snowflakes_Speed = wxNewId();
const long SnowflakesPanel::ID_SLIDER_Snowflakes_Speed = wxNewId();
const long SnowflakesPanel::IDD_TEXTCTRL_Snowflakes_Speed = wxNewId();
const long SnowflakesPanel::ID_BITMAPBUTTON_SLIDER_Snowflakes_Speed = wxNewId();
const long SnowflakesPanel::ID_STATICTEXT_Falling = wxNewId();
const long SnowflakesPanel::ID_CHOICE_Falling = wxNewId();
const long SnowflakesPanel::ID_STATICTEXT2 = wxNewId();
const long SnowflakesPanel::ID_BITMAPBUTTON_CHOICE_Falling = wxNewId();
//*)

BEGIN_EVENT_TABLE(SnowflakesPanel,wxPanel)
	//(*EventTable(SnowflakesPanel)
	//*)
END_EVENT_TABLE()

SnowflakesPanel::SnowflakesPanel(wxWindow* parent)
{
	//(*Initialize(SnowflakesPanel)
	BulkEditTextCtrl* TextCtrl57;
	wxFlexGridSizer* FlexGridSizer43;
	BulkEditTextCtrl* TextCtrl58;
	BulkEditTextCtrl* TextCtrl56;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer43 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer43->AddGrowableCol(1);
	StaticText79 = new wxStaticText(this, ID_STATICTEXT_Snowflakes_Count, _("Max flakes"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Snowflakes_Count"));
	FlexGridSizer43->Add(StaticText79, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Snowflakes_Count = new BulkEditSlider(this, ID_SLIDER_Snowflakes_Count, 5, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Snowflakes_Count"));
	FlexGridSizer43->Add(Slider_Snowflakes_Count, 1, wxALL|wxEXPAND, 2);
	TextCtrl56 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Snowflakes_Count, _("5"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Snowflakes_Count"));
	TextCtrl56->SetMaxLength(3);
	FlexGridSizer43->Add(TextCtrl56, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_SnowflakesCount = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Snowflakes_Count, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Snowflakes_Count"));
	FlexGridSizer43->Add(BitmapButton_SnowflakesCount, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText80 = new wxStaticText(this, ID_STATICTEXT_Snowflakes_Type, _("Type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Snowflakes_Type"));
	FlexGridSizer43->Add(StaticText80, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Snowflakes_Type = new BulkEditSlider(this, ID_SLIDER_Snowflakes_Type, 1, 0, 5, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Snowflakes_Type"));
	FlexGridSizer43->Add(Slider_Snowflakes_Type, 1, wxALL|wxEXPAND, 2);
	TextCtrl57 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Snowflakes_Type, _("1"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Snowflakes_Type"));
	TextCtrl57->SetMaxLength(3);
	FlexGridSizer43->Add(TextCtrl57, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_SnowflakesType = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Snowflakes_Type, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Snowflakes_Type"));
	FlexGridSizer43->Add(BitmapButton_SnowflakesType, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText181 = new wxStaticText(this, ID_STATICTEXT_Snowflakes_Speed, _("Speed"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Snowflakes_Speed"));
	FlexGridSizer43->Add(StaticText181, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Snowflakes_Speed = new BulkEditSlider(this, ID_SLIDER_Snowflakes_Speed, 10, 0, 50, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Snowflakes_Speed"));
	FlexGridSizer43->Add(Slider_Snowflakes_Speed, 1, wxALL|wxEXPAND, 5);
	TextCtrl58 = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Snowflakes_Speed, _("10"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Snowflakes_Speed"));
	TextCtrl58->SetMaxLength(3);
	FlexGridSizer43->Add(TextCtrl58, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_SnowflakesSpeed = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Snowflakes_Speed, wxNullBitmap, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Snowflakes_Speed"));
	FlexGridSizer43->Add(BitmapButton_SnowflakesSpeed, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT_Falling, _("Falling"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Falling"));
	FlexGridSizer43->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Falling = new BulkEditChoice(this, ID_CHOICE_Falling, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Falling"));
	Choice_Falling->SetSelection( Choice_Falling->Append(_("Driving")) );
	Choice_Falling->Append(_("Falling"));
	Choice_Falling->Append(_("Falling & Accumulating"));
	FlexGridSizer43->Add(Choice_Falling, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer43->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Falling = new wxBitmapButton(this, ID_BITMAPBUTTON_CHOICE_Falling, wxNullBitmap, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Falling"));
	FlexGridSizer43->Add(BitmapButton_Falling, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer43);
	FlexGridSizer43->Fit(this);
	FlexGridSizer43->SetSizeHints(this);

	Connect(ID_BITMAPBUTTON_SLIDER_Snowflakes_Count,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SnowflakesPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Snowflakes_Type,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SnowflakesPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Snowflakes_Speed,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SnowflakesPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHOICE_Falling,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SnowflakesPanel::OnLockButtonClick);
	//*)
    SetName("ID_PANEL_SNOWFLAKES");
}

SnowflakesPanel::~SnowflakesPanel()
{
	//(*Destroy(SnowflakesPanel)
	//*)
}

PANEL_EVENT_HANDLERS(SnowflakesPanel)
