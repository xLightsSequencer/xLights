#include "FillPanel.h"
#include "FillEffect.h"
#include "EffectPanelUtils.h"

//(*InternalHeaders(FillPanel)
#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>
//*)

//(*IdInit(FillPanel)
const long FillPanel::ID_STATICTEXT_Fill_Position = wxNewId();
const long FillPanel::ID_SLIDER_Fill_Position = wxNewId();
const long FillPanel::ID_VALUECURVE_Fill_Position = wxNewId();
const long FillPanel::IDD_TEXTCTRL_Fill_Position = wxNewId();
const long FillPanel::ID_BITMAPBUTTON_SLIDER_Fill_Position = wxNewId();
const long FillPanel::ID_STATICTEXT_Fill_Band_Size = wxNewId();
const long FillPanel::ID_SLIDER_Fill_Band_Size = wxNewId();
const long FillPanel::ID_VALUECURVE_Fill_Band_Size = wxNewId();
const long FillPanel::IDD_TEXTCTRL_Fill_Band_Size = wxNewId();
const long FillPanel::ID_BITMAPBUTTON_SLIDER_Fill_Band_Size = wxNewId();
const long FillPanel::ID_STATICTEXT_Fill_Skip_Size = wxNewId();
const long FillPanel::ID_SLIDER_Fill_Skip_Size = wxNewId();
const long FillPanel::ID_VALUECURVE_Fill_Skip_Size = wxNewId();
const long FillPanel::IDD_TEXTCTRL_Fill_Skip_Size = wxNewId();
const long FillPanel::ID_BITMAPBUTTON_SLIDER_Fill_Skip_Size = wxNewId();
const long FillPanel::ID_STATICTEXT_Fill_Offset = wxNewId();
const long FillPanel::ID_SLIDER_Fill_Offset = wxNewId();
const long FillPanel::ID_VALUECURVE_Fill_Offset = wxNewId();
const long FillPanel::IDD_TEXTCTRL_Fill_Offset = wxNewId();
const long FillPanel::ID_BITMAPBUTTON_SLIDER_Fill_Offset = wxNewId();
const long FillPanel::ID_CHECKBOX_Fill_Offset_In_Pixels = wxNewId();
const long FillPanel::ID_CHECKBOX_Fill_Color_Time = wxNewId();
const long FillPanel::ID_CHECKBOX_Fill_Wrap = wxNewId();
const long FillPanel::ID_STATICTEXT_Fill_Direction = wxNewId();
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
	BulkEditTextCtrl* TextCtrl_Fill_Band_Size;
	BulkEditTextCtrl* TextCtrl_Fill_Offset;
	BulkEditTextCtrl* TextCtrl_Fill_Position;
	BulkEditTextCtrl* TextCtrl_Fill_Skip_Size;
	wxFlexGridSizer* FlexGridSizer123;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer35;
	wxFlexGridSizer* FlexGridSizer3;
	xlLockButton* BitmapButton_Direction;
	xlLockButton* BitmapButton_FillBand_Size;
	xlLockButton* BitmapButton_FillOffset;
	xlLockButton* BitmapButton_FillPosition;
	xlLockButton* BitmapButton_FillSkip_Size;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer35 = new wxFlexGridSizer(0, 3, wxDLG_UNIT(this,wxSize(0,0)).GetWidth(), 0);
	FlexGridSizer35->AddGrowableCol(1);
	StaticText23 = new wxStaticText(this, ID_STATICTEXT_Fill_Position, _("Position:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fill_Position"));
	FlexGridSizer35->Add(StaticText23, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer123 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer123->AddGrowableCol(0);
	Slider_Fill_Position = new BulkEditSlider(this, ID_SLIDER_Fill_Position, 100, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fill_Position"));
	FlexGridSizer123->Add(Slider_Fill_Position, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Fill_Position = new BulkEditValueCurveButton(this, ID_VALUECURVE_Fill_Position, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Fill_Position"));
	FlexGridSizer123->Add(BitmapButton_Fill_Position, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	TextCtrl_Fill_Position = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Fill_Position, _("100"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Fill_Position"));
	TextCtrl_Fill_Position->SetMaxLength(3);
	FlexGridSizer123->Add(TextCtrl_Fill_Position, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer35->Add(FlexGridSizer123, 1, wxALL|wxEXPAND, 0);
	BitmapButton_FillPosition = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Fill_Position, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fill_Position"));
	BitmapButton_FillPosition->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer35->Add(BitmapButton_FillPosition, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT_Fill_Band_Size, _("Band Size:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fill_Band_Size"));
	FlexGridSizer35->Add(StaticText1, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer1 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	Slider_Fill_Band_Size = new BulkEditSlider(this, ID_SLIDER_Fill_Band_Size, 0, 0, 250, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fill_Band_Size"));
	FlexGridSizer1->Add(Slider_Fill_Band_Size, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Fill_Band_Size = new BulkEditValueCurveButton(this, ID_VALUECURVE_Fill_Band_Size, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Fill_Band_Size"));
	FlexGridSizer1->Add(BitmapButton_Fill_Band_Size, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	TextCtrl_Fill_Band_Size = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Fill_Band_Size, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Fill_Band_Size"));
	TextCtrl_Fill_Band_Size->SetMaxLength(3);
	FlexGridSizer1->Add(TextCtrl_Fill_Band_Size, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer35->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 0);
	BitmapButton_FillBand_Size = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Fill_Band_Size, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fill_Band_Size"));
	BitmapButton_FillBand_Size->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer35->Add(BitmapButton_FillBand_Size, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT_Fill_Skip_Size, _("Skip Size:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fill_Skip_Size"));
	FlexGridSizer35->Add(StaticText2, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	Slider_Fill_Skip_Size = new BulkEditSlider(this, ID_SLIDER_Fill_Skip_Size, 0, 0, 250, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fill_Skip_Size"));
	FlexGridSizer2->Add(Slider_Fill_Skip_Size, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Fill_Skip_Size = new BulkEditValueCurveButton(this, ID_VALUECURVE_Fill_Skip_Size, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Fill_Skip_Size"));
	FlexGridSizer2->Add(BitmapButton_Fill_Skip_Size, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	TextCtrl_Fill_Skip_Size = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Fill_Skip_Size, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Fill_Skip_Size"));
	TextCtrl_Fill_Skip_Size->SetMaxLength(3);
	FlexGridSizer2->Add(TextCtrl_Fill_Skip_Size, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer35->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 0);
	BitmapButton_FillSkip_Size = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Fill_Skip_Size, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fill_Skip_Size"));
	BitmapButton_FillSkip_Size->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer35->Add(BitmapButton_FillSkip_Size, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT_Fill_Offset, _("Offset:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fill_Offset"));
	FlexGridSizer35->Add(StaticText3, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	Slider_Fill_Offset = new BulkEditSlider(this, ID_SLIDER_Fill_Offset, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fill_Offset"));
	FlexGridSizer3->Add(Slider_Fill_Offset, 1, wxALL|wxEXPAND, 2);
	BitmapButton_Fill_Offset = new BulkEditValueCurveButton(this, ID_VALUECURVE_Fill_Offset, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_VALUECURVE_Fill_Offset"));
	FlexGridSizer3->Add(BitmapButton_Fill_Offset, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	TextCtrl_Fill_Offset = new BulkEditTextCtrl(this, IDD_TEXTCTRL_Fill_Offset, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Fill_Offset"));
	TextCtrl_Fill_Offset->SetMaxLength(3);
	FlexGridSizer3->Add(TextCtrl_Fill_Offset, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer35->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 0);
	BitmapButton_FillOffset = new xlLockButton(this, ID_BITMAPBUTTON_SLIDER_Fill_Offset, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fill_Offset"));
	BitmapButton_FillOffset->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer35->Add(BitmapButton_FillOffset, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer35->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Fill_Offset_In_Pixels = new BulkEditCheckBox(this, ID_CHECKBOX_Fill_Offset_In_Pixels, _("Offset in Pixels"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Fill_Offset_In_Pixels"));
	CheckBox_Fill_Offset_In_Pixels->SetValue(true);
	FlexGridSizer35->Add(CheckBox_Fill_Offset_In_Pixels, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer35->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer35->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Fill_Color_Time = new BulkEditCheckBox(this, ID_CHECKBOX_Fill_Color_Time, _("Change Color based on Time"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Fill_Color_Time"));
	CheckBox_Fill_Color_Time->SetValue(false);
	FlexGridSizer35->Add(CheckBox_Fill_Color_Time, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer35->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer35->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Fill_Wrap = new BulkEditCheckBox(this, ID_CHECKBOX_Fill_Wrap, _("Wrap"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Fill_Wrap"));
	CheckBox_Fill_Wrap->SetValue(true);
	FlexGridSizer35->Add(CheckBox_Fill_Wrap, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer35->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText24 = new wxStaticText(this, ID_STATICTEXT_Fill_Direction, _("Direction:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fill_Direction"));
	FlexGridSizer35->Add(StaticText24, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Fill_Direction = new BulkEditChoice(this, ID_CHOICE_Fill_Direction, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Fill_Direction"));
	Choice_Fill_Direction->SetSelection( Choice_Fill_Direction->Append(_("Up")) );
	Choice_Fill_Direction->Append(_("Down"));
	Choice_Fill_Direction->Append(_("Left"));
	Choice_Fill_Direction->Append(_("Right"));
	FlexGridSizer35->Add(Choice_Fill_Direction, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_Direction = new xlLockButton(this, ID_BITMAPBUTTON_CHOICE_Fill_Direction, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_PADLOCK_OPEN")),wxART_BUTTON), wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Fill_Direction"));
	BitmapButton_Direction->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
	FlexGridSizer35->Add(BitmapButton_Direction, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	SetSizer(FlexGridSizer35);
	FlexGridSizer35->Fit(this);
	FlexGridSizer35->SetSizeHints(this);

	Connect(ID_VALUECURVE_Fill_Position,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FillPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fill_Position,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FillPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Fill_Band_Size,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FillPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fill_Band_Size,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FillPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Fill_Skip_Size,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FillPanel::OnVCButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Fill_Skip_Size,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FillPanel::OnLockButtonClick);
	Connect(ID_VALUECURVE_Fill_Offset,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FillPanel::OnVCButtonClick);
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

