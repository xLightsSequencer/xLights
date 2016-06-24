#include "FillPanel.h"

#include "../../include/padlock16x16-blue.xpm"
#include "EffectPanelUtils.h"

//(*InternalHeaders(FillPanel)
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

//(*IdInit(FillPanel)
const long FillPanel::ID_SLIDER_Fill_Position = wxNewId();
const long FillPanel::ID_VALUECURVE_Fill_Position = wxNewId();
const long FillPanel::IDD_TEXTCTRL_Fill_Position = wxNewId();
const long FillPanel::ID_BITMAPBUTTON_SLIDER_Fill_Position = wxNewId();
const long FillPanel::ID_CHOICE_Fill_Direction = wxNewId();
const long FillPanel::ID_BITMAPBUTTON_CHOICE_Fill_Direction = wxNewId();
//*)

BEGIN_EVENT_TABLE(FillPanel,wxPanel)
	//(*EventTable(FillPanel)
	//*)
END_EVENT_TABLE()

FillPanel::FillPanel(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(FillPanel)
	wxBitmapButton* BitmapButton_FillPosition;
	wxBitmapButton* BitmapButton_Direction;
	wxFlexGridSizer* FlexGridSizer123;
	wxStaticText* StaticText23;
	wxStaticText* StaticText24;
	wxTextCtrl* TextCtrl15;
	wxFlexGridSizer* FlexGridSizer35;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer35 = new wxFlexGridSizer(0, 3, wxDLG_UNIT(this,wxSize(0,0)).GetWidth(), 0);
	FlexGridSizer35->AddGrowableCol(1);
	StaticText23 = new wxStaticText(this, wxID_ANY, _("Position"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer35->Add(StaticText23, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer123 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer123->AddGrowableCol(0);
	Slider_Fill_Position = new wxSlider(this, ID_SLIDER_Fill_Position, 100, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fill_Position"));
	FlexGridSizer123->Add(Slider_Fill_Position, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Fill_Position = new ValueCurveButton(this, ID_VALUECURVE_Fill_Position, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Fill_Position"));
	FlexGridSizer123->Add(BitmapButton_Fill_Position, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	TextCtrl15 = new wxTextCtrl(this, IDD_TEXTCTRL_Fill_Position, _("100"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Fill_Position"));
	TextCtrl15->SetMaxLength(3);
	FlexGridSizer123->Add(TextCtrl15, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer35->Add(FlexGridSizer123, 1, wxALL|wxEXPAND, 0);
	BitmapButton_FillPosition = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Fill_Position, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fill_Position"));
	BitmapButton_FillPosition->SetDefault();
	BitmapButton_FillPosition->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer35->Add(BitmapButton_FillPosition, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText24 = new wxStaticText(this, wxID_ANY, _("Direction"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer35->Add(StaticText24, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Fill_Direction = new wxChoice(this, ID_CHOICE_Fill_Direction, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Fill_Direction"));
	Choice_Fill_Direction->SetSelection( Choice_Fill_Direction->Append(_("Up")) );
	Choice_Fill_Direction->Append(_("Down"));
	Choice_Fill_Direction->Append(_("Left"));
	Choice_Fill_Direction->Append(_("Right"));
	FlexGridSizer35->Add(Choice_Fill_Direction, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Direction = new wxBitmapButton(this, ID_BITMAPBUTTON_CHOICE_Fill_Direction, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Fill_Direction"));
	BitmapButton_Direction->SetDefault();
	BitmapButton_Direction->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer35->Add(BitmapButton_Direction, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	SetSizer(FlexGridSizer35);
	FlexGridSizer35->Fit(this);
	FlexGridSizer35->SetSizeHints(this);

	Connect(ID_SLIDER_Fill_Position,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&FillPanel::UpdateLinkedTextCtrlVC);
	Connect(ID_VALUECURVE_Fill_Position,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FillPanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_Fill_Position,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&FillPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_SLIDER_Fill_Position,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FillPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHOICE_Fill_Direction,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FillPanel::OnLockButtonClick);
	//*)

    SetName("ID_PANEL_FILL");

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&FillPanel::OnVCChanged, 0, this);

    BitmapButton_Fill_Position->GetValue()->SetLimits(0, 100);
}

FillPanel::~FillPanel()
{
	//(*Destroy(FillPanel)
	//*)
}

PANEL_EVENT_HANDLERS(FillPanel)

