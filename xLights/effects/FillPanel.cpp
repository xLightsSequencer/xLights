#include "FillPanel.h"
#include "FillEffect.h"
#include "../../include/padlock16x16-blue.xpm"
#include "EffectPanelUtils.h"

//(*InternalHeaders(FillPanel)
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

//(*IdInit(FillPanel)
const long FillPanel::ID_SLIDER_Fill_Position = wxNewId();
const long FillPanel::ID_VALUECURVE_Fill_Position = wxNewId();
const long FillPanel::IDD_TEXTCTRL_Fill_Position = wxNewId();
const long FillPanel::ID_BITMAPBUTTON_SLIDER_Fill_Position = wxNewId();
const long FillPanel::ID_SLIDER_Fill_Band_Size = wxNewId();
const long FillPanel::ID_VALUECURVE_Fill_Band_Size = wxNewId();
const long FillPanel::IDD_TEXTCTRL_Fill_Band_Size = wxNewId();
const long FillPanel::ID_BITMAPBUTTON_SLIDER_Fill_Band_Size = wxNewId();
const long FillPanel::ID_SLIDER_Fill_Skip_Size = wxNewId();
const long FillPanel::ID_VALUECURVE_Fill_Skip_Size = wxNewId();
const long FillPanel::IDD_TEXTCTRL_Fill_Skip_Size = wxNewId();
const long FillPanel::ID_BITMAPBUTTON_SLIDER_Fill_Skip_Size = wxNewId();
const long FillPanel::ID_SLIDER_Fill_Offset = wxNewId();
const long FillPanel::ID_VALUECURVE_Fill_Offset = wxNewId();
const long FillPanel::IDD_TEXTCTRL_Fill_Offset = wxNewId();
const long FillPanel::ID_BITMAPBUTTON_SLIDER_Fill_Offset = wxNewId();
const long FillPanel::ID_CHECKBOX_Fill_Offset_In_Pixels = wxNewId();
const long FillPanel::ID_CHECKBOX_Fill_Color_Time = wxNewId();
const long FillPanel::ID_CHECKBOX_Fill_Wrap = wxNewId();
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
	wxBitmapButton* BitmapButton_FillOffset;
	wxStaticText* StaticText2;
	wxFlexGridSizer* FlexGridSizer3;
	wxBitmapButton* BitmapButton_FillPosition;
	wxFlexGridSizer* FlexGridSizer2;
	wxTextCtrl* TextCtrl_Fill_Offset;
	wxStaticText* StaticText1;
	wxBitmapButton* BitmapButton_Direction;
	wxStaticText* StaticText3;
	wxFlexGridSizer* FlexGridSizer123;
	wxStaticText* StaticText23;
	wxStaticText* StaticText24;
	wxTextCtrl* TextCtrl_Fill_Position;
	wxBitmapButton* BitmapButton_FillBand_Size;
	wxTextCtrl* TextCtrl_Fill_Band_Size;
	wxFlexGridSizer* FlexGridSizer35;
	wxTextCtrl* TextCtrl_Fill_Skip_Size;
	wxFlexGridSizer* FlexGridSizer1;
	wxBitmapButton* BitmapButton_FillSkip_Size;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer35 = new wxFlexGridSizer(0, 3, wxDLG_UNIT(this,wxSize(0,0)).GetWidth(), 0);
	FlexGridSizer35->AddGrowableCol(1);
	StaticText23 = new wxStaticText(this, wxID_ANY, _("Position:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer35->Add(StaticText23, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer123 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer123->AddGrowableCol(0);
	Slider_Fill_Position = new wxSlider(this, ID_SLIDER_Fill_Position, 100, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fill_Position"));
	FlexGridSizer123->Add(Slider_Fill_Position, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Fill_Position = new ValueCurveButton(this, ID_VALUECURVE_Fill_Position, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Fill_Position"));
	FlexGridSizer123->Add(BitmapButton_Fill_Position, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	TextCtrl_Fill_Position = new wxTextCtrl(this, IDD_TEXTCTRL_Fill_Position, _("100"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Fill_Position"));
	TextCtrl_Fill_Position->SetMaxLength(3);
	FlexGridSizer123->Add(TextCtrl_Fill_Position, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer35->Add(FlexGridSizer123, 1, wxALL|wxEXPAND, 0);
	BitmapButton_FillPosition = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Fill_Position, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fill_Position"));
	BitmapButton_FillPosition->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer35->Add(BitmapButton_FillPosition, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText1 = new wxStaticText(this, wxID_ANY, _("Band Size:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer35->Add(StaticText1, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer1 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	Slider_Fill_Band_Size = new wxSlider(this, ID_SLIDER_Fill_Band_Size, 0, 0, 250, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fill_Band_Size"));
	FlexGridSizer1->Add(Slider_Fill_Band_Size, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Fill_Band_Size = new ValueCurveButton(this, ID_VALUECURVE_Fill_Band_Size, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Fill_Band_Size"));
	FlexGridSizer1->Add(BitmapButton_Fill_Band_Size, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	TextCtrl_Fill_Band_Size = new wxTextCtrl(this, IDD_TEXTCTRL_Fill_Band_Size, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Fill_Band_Size"));
	TextCtrl_Fill_Band_Size->SetMaxLength(3);
	FlexGridSizer1->Add(TextCtrl_Fill_Band_Size, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer35->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 0);
	BitmapButton_FillBand_Size = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Fill_Band_Size, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fill_Band_Size"));
	BitmapButton_FillBand_Size->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer35->Add(BitmapButton_FillBand_Size, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, wxID_ANY, _("Skip Size:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer35->Add(StaticText2, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	Slider_Fill_Skip_Size = new wxSlider(this, ID_SLIDER_Fill_Skip_Size, 0, 0, 250, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fill_Skip_Size"));
	FlexGridSizer2->Add(Slider_Fill_Skip_Size, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Fill_Skip_Size = new ValueCurveButton(this, ID_VALUECURVE_Fill_Skip_Size, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Fill_Skip_Size"));
	FlexGridSizer2->Add(BitmapButton_Fill_Skip_Size, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	TextCtrl_Fill_Skip_Size = new wxTextCtrl(this, IDD_TEXTCTRL_Fill_Skip_Size, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Fill_Skip_Size"));
	TextCtrl_Fill_Skip_Size->SetMaxLength(3);
	FlexGridSizer2->Add(TextCtrl_Fill_Skip_Size, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer35->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 0);
	BitmapButton_FillSkip_Size = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Fill_Skip_Size, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fill_Skip_Size"));
	BitmapButton_FillSkip_Size->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer35->Add(BitmapButton_FillSkip_Size, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText3 = new wxStaticText(this, wxID_ANY, _("Offset:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer35->Add(StaticText3, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	Slider_Fill_Offset = new wxSlider(this, ID_SLIDER_Fill_Offset, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fill_Offset"));
	FlexGridSizer3->Add(Slider_Fill_Offset, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Fill_Offset = new ValueCurveButton(this, ID_VALUECURVE_Fill_Offset, valuecurvenotselected_24, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Fill_Offset"));
	FlexGridSizer3->Add(BitmapButton_Fill_Offset, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	TextCtrl_Fill_Offset = new wxTextCtrl(this, IDD_TEXTCTRL_Fill_Offset, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Fill_Offset"));
	TextCtrl_Fill_Offset->SetMaxLength(3);
	FlexGridSizer3->Add(TextCtrl_Fill_Offset, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer35->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 0);
	BitmapButton_FillOffset = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Fill_Offset, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fill_Offset"));
	BitmapButton_FillOffset->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer35->Add(BitmapButton_FillOffset, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer35->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Fill_Offset_In_Pixels = new wxCheckBox(this, ID_CHECKBOX_Fill_Offset_In_Pixels, _("Offset in Pixels"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Fill_Offset_In_Pixels"));
	CheckBox_Fill_Offset_In_Pixels->SetValue(true);
	FlexGridSizer35->Add(CheckBox_Fill_Offset_In_Pixels, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer35->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer35->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Fill_Color_Time = new wxCheckBox(this, ID_CHECKBOX_Fill_Color_Time, _("Change Color based on Time"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Fill_Color_Time"));
	CheckBox_Fill_Color_Time->SetValue(false);
	FlexGridSizer35->Add(CheckBox_Fill_Color_Time, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer35->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer35->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Fill_Wrap = new wxCheckBox(this, ID_CHECKBOX_Fill_Wrap, _("Wrap"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Fill_Wrap"));
	CheckBox_Fill_Wrap->SetValue(true);
	FlexGridSizer35->Add(CheckBox_Fill_Wrap, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer35->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText24 = new wxStaticText(this, wxID_ANY, _("Direction:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer35->Add(StaticText24, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Fill_Direction = new wxChoice(this, ID_CHOICE_Fill_Direction, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Fill_Direction"));
	Choice_Fill_Direction->SetSelection( Choice_Fill_Direction->Append(_("Up")) );
	Choice_Fill_Direction->Append(_("Down"));
	Choice_Fill_Direction->Append(_("Left"));
	Choice_Fill_Direction->Append(_("Right"));
	FlexGridSizer35->Add(Choice_Fill_Direction, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Direction = new wxBitmapButton(this, ID_BITMAPBUTTON_CHOICE_Fill_Direction, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Fill_Direction"));
	BitmapButton_Direction->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer35->Add(BitmapButton_Direction, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	SetSizer(FlexGridSizer35);
	FlexGridSizer35->Fit(this);
	FlexGridSizer35->SetSizeHints(this);

	Connect(ID_SLIDER_Fill_Position,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&FillPanel::UpdateLinkedTextCtrlVC);
	Connect(ID_VALUECURVE_Fill_Position,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FillPanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_Fill_Position,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&FillPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_SLIDER_Fill_Position,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FillPanel::OnLockButtonClick);
	Connect(ID_SLIDER_Fill_Band_Size,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&FillPanel::UpdateLinkedTextCtrlVC);
	Connect(ID_VALUECURVE_Fill_Band_Size,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FillPanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_Fill_Band_Size,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&FillPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_SLIDER_Fill_Band_Size,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FillPanel::OnLockButtonClick);
	Connect(ID_SLIDER_Fill_Skip_Size,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&FillPanel::UpdateLinkedTextCtrlVC);
	Connect(ID_VALUECURVE_Fill_Skip_Size,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FillPanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_Fill_Skip_Size,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&FillPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_SLIDER_Fill_Skip_Size,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FillPanel::OnLockButtonClick);
	Connect(ID_SLIDER_Fill_Offset,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&FillPanel::UpdateLinkedTextCtrlVC);
	Connect(ID_VALUECURVE_Fill_Offset,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FillPanel::OnVCButtonClick);
	Connect(IDD_TEXTCTRL_Fill_Offset,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&FillPanel::UpdateLinkedSlider);
	Connect(ID_BITMAPBUTTON_SLIDER_Fill_Offset,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FillPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHOICE_Fill_Direction,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FillPanel::OnLockButtonClick);
	//*)

    SetName("ID_PANEL_FILL");

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&FillPanel::OnVCChanged, 0, this);

    BitmapButton_Fill_Position->GetValue()->SetLimits(FILL_POSITION_MIN, FILL_POSITION_MAX);
    BitmapButton_Fill_Band_Size->GetValue()->SetLimits(FILL_BANDSIZE_MIN, FILL_BANDSIZE_MAX);
    BitmapButton_Fill_Skip_Size->GetValue()->SetLimits(FILL_SKIPSIZE_MIN, FILL_SKIPSIZE_MAX);
    BitmapButton_Fill_Offset->GetValue()->SetLimits(FILL_OFFSET_MIN, FILL_OFFSET_MAX);
}

FillPanel::~FillPanel()
{
	//(*Destroy(FillPanel)
	//*)
}

PANEL_EVENT_HANDLERS(FillPanel)

