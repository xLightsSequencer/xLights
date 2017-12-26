#include "MarqueePanel.h"
#include "../../include/padlock16x16-blue.xpm"
#include "EffectPanelUtils.h"

//(*InternalHeaders(MarqueePanel)
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/bitmap.h>
#include <wx/statline.h>
#include <wx/slider.h>
#include <wx/settings.h>
#include <wx/bmpbuttn.h>
#include <wx/intl.h>
#include <wx/gbsizer.h>
#include <wx/image.h>
#include <wx/string.h>
//*)

//(*IdInit(MarqueePanel)
const long MarqueePanel::ID_STATICTEXT_Marquee_Band_Size = wxNewId();
const long MarqueePanel::ID_SLIDER_Marquee_Band_Size = wxNewId();
const long MarqueePanel::IDD_TEXTCTRL_Marquee_Band_Size = wxNewId();
const long MarqueePanel::ID_BITMAPBUTTON_SLIDER_Marquee_Band_Size = wxNewId();
const long MarqueePanel::ID_STATICTEXT_Marquee_Skip_Size = wxNewId();
const long MarqueePanel::ID_SLIDER_Marquee_Skip_Size = wxNewId();
const long MarqueePanel::IDD_TEXTCTRL_Marquee_Skip_Size = wxNewId();
const long MarqueePanel::ID_BITMAPBUTTON_SLIDER_Marquee_Skip_Size = wxNewId();
const long MarqueePanel::ID_STATICTEXT_Marquee_Thickness = wxNewId();
const long MarqueePanel::ID_SLIDER_Marquee_Thickness = wxNewId();
const long MarqueePanel::IDD_TEXTCTRL_Marquee_Thickness = wxNewId();
const long MarqueePanel::ID_BITMAPBUTTON_SLIDER_Marquee_Thickness = wxNewId();
const long MarqueePanel::ID_STATICTEXT_Marquee_Stagger = wxNewId();
const long MarqueePanel::ID_SLIDER_Marquee_Stagger = wxNewId();
const long MarqueePanel::IDD_TEXTCTRL_Marquee_Stagger = wxNewId();
const long MarqueePanel::ID_BITMAPBUTTON_SLIDER_Marquee_Stagger = wxNewId();
const long MarqueePanel::ID_STATICTEXT_Marquee_Speed = wxNewId();
const long MarqueePanel::ID_SLIDER_Marquee_Speed = wxNewId();
const long MarqueePanel::IDD_TEXTCTRL_Marquee_Speed = wxNewId();
const long MarqueePanel::ID_BITMAPBUTTON_SLIDER_Marquee_Speed = wxNewId();
const long MarqueePanel::ID_STATICTEXT_Marquee_Start = wxNewId();
const long MarqueePanel::ID_SLIDER_Marquee_Start = wxNewId();
const long MarqueePanel::IDD_TEXTCTRL_Marquee_Start = wxNewId();
const long MarqueePanel::ID_BITMAPBUTTON_SLIDER_Marquee_Start = wxNewId();
const long MarqueePanel::ID_CHECKBOX_Marquee_Reverse = wxNewId();
const long MarqueePanel::ID_PANEL_Marquee_Settings = wxNewId();
const long MarqueePanel::ID_STATICTEXT_Marquee_ScaleX = wxNewId();
const long MarqueePanel::ID_SLIDER_Marquee_ScaleX = wxNewId();
const long MarqueePanel::IDD_TEXTCTRL_Marquee_ScaleX = wxNewId();
const long MarqueePanel::ID_BITMAPBUTTON_SLIDER_Marquee_ScaleX = wxNewId();
const long MarqueePanel::ID_STATICTEXT_Marquee_ScaleY = wxNewId();
const long MarqueePanel::ID_SLIDER_Marquee_ScaleY = wxNewId();
const long MarqueePanel::IDD_TEXTCTRL_Marquee_ScaleY = wxNewId();
const long MarqueePanel::ID_BITMAPBUTTON_SLIDER_Marquee_ScaleY = wxNewId();
const long MarqueePanel::ID_STATICLINE1 = wxNewId();
const long MarqueePanel::ID_CHECKBOX_Marquee_PixelOffsets = wxNewId();
const long MarqueePanel::ID_STATICTEXT_MarqueeXC = wxNewId();
const long MarqueePanel::ID_SLIDER_MarqueeXC = wxNewId();
const long MarqueePanel::ID_CHECKBOX_Marquee_WrapX = wxNewId();
const long MarqueePanel::IDD_TEXTCTRL_MarqueeXC = wxNewId();
const long MarqueePanel::ID_STATICTEXT_MarqueeYC = wxNewId();
const long MarqueePanel::IDD_TEXTCTRL_MarqueeYC = wxNewId();
const long MarqueePanel::ID_SLIDER_MarqueeYC = wxNewId();
const long MarqueePanel::ID_PANEL_Marquee_Position = wxNewId();
const long MarqueePanel::ID_NOTEBOOK_Marquee = wxNewId();
//*)

BEGIN_EVENT_TABLE(MarqueePanel,wxPanel)
	//(*EventTable(MarqueePanel)
	//*)
END_EVENT_TABLE()

MarqueePanel::MarqueePanel(wxWindow* parent)
{
	//(*Initialize(MarqueePanel)
	wxFlexGridSizer* FlexGridSizer138;
	wxFlexGridSizer* FlexGridSizer23;
	BulkEditTextCtrl* TextCtrl_Marquee_Speed;
	BulkEditTextCtrl* TextCtrl_MarqueeXC;
	BulkEditTextCtrl* TextCtrl_Marquee_Thickness;
	wxFlexGridSizer* FlexGridSizer135;
	wxBitmapButton* BitmapButton_Marquee_Band_Size;
	BulkEditTextCtrl* TextCtrl_Marquee_Skip_Size;
	BulkEditTextCtrl* TextCtrl_Marquee_ScaleY;
	BulkEditTextCtrl* TextCtrl_Marquee_ScaleX;
	wxFlexGridSizer* FlexGridSizer137;
	BulkEditTextCtrl* TextCtrl_Marquee_Start;
	BulkEditTextCtrl* TextCtrl_Marquee_Stagger;
	BulkEditTextCtrl* TextCtrl_Marquee_Band_Size;
	wxGridBagSizer* GridBagSizer5;
	wxFlexGridSizer* FlexGridSizer136;
	wxFlexGridSizer* FlexGridSizer58;
	BulkEditTextCtrl* TextCtrl_MarqueeYC;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer23 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer23->AddGrowableCol(0);
	Notebook_Marquee = new wxNotebook(this, ID_NOTEBOOK_Marquee, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK_Marquee"));
	Panel_Marquee_Settings = new wxPanel(Notebook_Marquee, ID_PANEL_Marquee_Settings, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_Marquee_Settings"));
	FlexGridSizer58 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer58->AddGrowableCol(1);
	StaticText116 = new wxStaticText(Panel_Marquee_Settings, ID_STATICTEXT_Marquee_Band_Size, _("Band Size:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Marquee_Band_Size"));
	FlexGridSizer58->Add(StaticText116, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Marquee_Band_Size = new BulkEditSlider(Panel_Marquee_Settings, ID_SLIDER_Marquee_Band_Size, 3, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Marquee_Band_Size"));
	FlexGridSizer58->Add(Slider_Marquee_Band_Size, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Marquee_Band_Size = new BulkEditTextCtrl(Panel_Marquee_Settings, IDD_TEXTCTRL_Marquee_Band_Size, _("3"), wxDefaultPosition, wxDLG_UNIT(Panel_Marquee_Settings,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Marquee_Band_Size"));
	TextCtrl_Marquee_Band_Size->SetMaxLength(3);
	FlexGridSizer58->Add(TextCtrl_Marquee_Band_Size, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Marquee_Band_Size = new wxBitmapButton(Panel_Marquee_Settings, ID_BITMAPBUTTON_SLIDER_Marquee_Band_Size, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Marquee_Band_Size"));
	BitmapButton_Marquee_Band_Size->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer58->Add(BitmapButton_Marquee_Band_Size, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText194 = new wxStaticText(Panel_Marquee_Settings, ID_STATICTEXT_Marquee_Skip_Size, _("Skip Size:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Marquee_Skip_Size"));
	FlexGridSizer58->Add(StaticText194, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Marquee_Skip_Size = new BulkEditSlider(Panel_Marquee_Settings, ID_SLIDER_Marquee_Skip_Size, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Marquee_Skip_Size"));
	FlexGridSizer58->Add(Slider_Marquee_Skip_Size, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Marquee_Skip_Size = new BulkEditTextCtrl(Panel_Marquee_Settings, IDD_TEXTCTRL_Marquee_Skip_Size, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel_Marquee_Settings,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Marquee_Skip_Size"));
	TextCtrl_Marquee_Skip_Size->SetMaxLength(3);
	FlexGridSizer58->Add(TextCtrl_Marquee_Skip_Size, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Marquee_Skip_Size = new wxBitmapButton(Panel_Marquee_Settings, ID_BITMAPBUTTON_SLIDER_Marquee_Skip_Size, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Marquee_Skip_Size"));
	BitmapButton_Marquee_Skip_Size->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer58->Add(BitmapButton_Marquee_Skip_Size, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText195 = new wxStaticText(Panel_Marquee_Settings, ID_STATICTEXT_Marquee_Thickness, _("Thickness:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Marquee_Thickness"));
	FlexGridSizer58->Add(StaticText195, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Marquee_Thickness = new BulkEditSlider(Panel_Marquee_Settings, ID_SLIDER_Marquee_Thickness, 1, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Marquee_Thickness"));
	FlexGridSizer58->Add(Slider_Marquee_Thickness, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Marquee_Thickness = new BulkEditTextCtrl(Panel_Marquee_Settings, IDD_TEXTCTRL_Marquee_Thickness, _("1"), wxDefaultPosition, wxDLG_UNIT(Panel_Marquee_Settings,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Marquee_Thickness"));
	TextCtrl_Marquee_Thickness->SetMaxLength(3);
	FlexGridSizer58->Add(TextCtrl_Marquee_Thickness, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Marquee_Thickness = new wxBitmapButton(Panel_Marquee_Settings, ID_BITMAPBUTTON_SLIDER_Marquee_Thickness, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Marquee_Thickness"));
	BitmapButton_Marquee_Thickness->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer58->Add(BitmapButton_Marquee_Thickness, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText62 = new wxStaticText(Panel_Marquee_Settings, ID_STATICTEXT_Marquee_Stagger, _("Stagger:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Marquee_Stagger"));
	FlexGridSizer58->Add(StaticText62, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Marquee_Stagger = new BulkEditSlider(Panel_Marquee_Settings, ID_SLIDER_Marquee_Stagger, 0, 0, 50, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Marquee_Stagger"));
	FlexGridSizer58->Add(Slider_Marquee_Stagger, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Marquee_Stagger = new BulkEditTextCtrl(Panel_Marquee_Settings, IDD_TEXTCTRL_Marquee_Stagger, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel_Marquee_Settings,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Marquee_Stagger"));
	TextCtrl_Marquee_Stagger->SetMaxLength(3);
	FlexGridSizer58->Add(TextCtrl_Marquee_Stagger, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Marquee_Stagger = new wxBitmapButton(Panel_Marquee_Settings, ID_BITMAPBUTTON_SLIDER_Marquee_Stagger, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Marquee_Stagger"));
	BitmapButton_Marquee_Stagger->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer58->Add(BitmapButton_Marquee_Stagger, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText196 = new wxStaticText(Panel_Marquee_Settings, ID_STATICTEXT_Marquee_Speed, _("Speed:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Marquee_Speed"));
	FlexGridSizer58->Add(StaticText196, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Marquee_Speed = new BulkEditSlider(Panel_Marquee_Settings, ID_SLIDER_Marquee_Speed, 3, 0, 50, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Marquee_Speed"));
	FlexGridSizer58->Add(Slider_Marquee_Speed, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Marquee_Speed = new BulkEditTextCtrl(Panel_Marquee_Settings, IDD_TEXTCTRL_Marquee_Speed, _("3"), wxDefaultPosition, wxDLG_UNIT(Panel_Marquee_Settings,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Marquee_Speed"));
	TextCtrl_Marquee_Speed->SetMaxLength(3);
	FlexGridSizer58->Add(TextCtrl_Marquee_Speed, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Marquee_Speed = new wxBitmapButton(Panel_Marquee_Settings, ID_BITMAPBUTTON_SLIDER_Marquee_Speed, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Marquee_Speed"));
	BitmapButton_Marquee_Speed->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer58->Add(BitmapButton_Marquee_Speed, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText244 = new wxStaticText(Panel_Marquee_Settings, ID_STATICTEXT_Marquee_Start, _("Start Pos:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Marquee_Start"));
	FlexGridSizer58->Add(StaticText244, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Marquee_Start = new BulkEditSlider(Panel_Marquee_Settings, ID_SLIDER_Marquee_Start, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Marquee_Start"));
	FlexGridSizer58->Add(Slider_Marquee_Start, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Marquee_Start = new BulkEditTextCtrl(Panel_Marquee_Settings, IDD_TEXTCTRL_Marquee_Start, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel_Marquee_Settings,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Marquee_Start"));
	TextCtrl_Marquee_Start->SetMaxLength(3);
	FlexGridSizer58->Add(TextCtrl_Marquee_Start, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Marquee_Start = new wxBitmapButton(Panel_Marquee_Settings, ID_BITMAPBUTTON_SLIDER_Marquee_Start, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Marquee_Start"));
	BitmapButton_Marquee_Start->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer58->Add(BitmapButton_Marquee_Start, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer58->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Marquee_Reverse = new BulkEditCheckBox(Panel_Marquee_Settings, ID_CHECKBOX_Marquee_Reverse, _("Reverse:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT, wxDefaultValidator, _T("ID_CHECKBOX_Marquee_Reverse"));
	CheckBox_Marquee_Reverse->SetValue(false);
	FlexGridSizer58->Add(CheckBox_Marquee_Reverse, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Panel_Marquee_Settings->SetSizer(FlexGridSizer58);
	FlexGridSizer58->Fit(Panel_Marquee_Settings);
	FlexGridSizer58->SetSizeHints(Panel_Marquee_Settings);
	Panel_Marquee_Position = new wxPanel(Notebook_Marquee, ID_PANEL_Marquee_Position, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_Marquee_Position"));
	FlexGridSizer135 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer138 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer138->AddGrowableCol(1);
	StaticText198 = new wxStaticText(Panel_Marquee_Position, ID_STATICTEXT_Marquee_ScaleX, _("Scale X:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Marquee_ScaleX"));
	FlexGridSizer138->Add(StaticText198, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Marquee_ScaleX = new BulkEditSlider(Panel_Marquee_Position, ID_SLIDER_Marquee_ScaleX, 100, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Marquee_ScaleX"));
	FlexGridSizer138->Add(Slider_Marquee_ScaleX, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Marquee_ScaleX = new BulkEditTextCtrl(Panel_Marquee_Position, IDD_TEXTCTRL_Marquee_ScaleX, _("100"), wxDefaultPosition, wxDLG_UNIT(Panel_Marquee_Position,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Marquee_ScaleX"));
	TextCtrl_Marquee_ScaleX->SetMaxLength(3);
	FlexGridSizer138->Add(TextCtrl_Marquee_ScaleX, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Marquee_ScaleX = new wxBitmapButton(Panel_Marquee_Position, ID_BITMAPBUTTON_SLIDER_Marquee_ScaleX, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Marquee_ScaleX"));
	BitmapButton_Marquee_ScaleX->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer138->Add(BitmapButton_Marquee_ScaleX, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText199 = new wxStaticText(Panel_Marquee_Position, ID_STATICTEXT_Marquee_ScaleY, _("Scale Y:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Marquee_ScaleY"));
	FlexGridSizer138->Add(StaticText199, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_Marquee_ScaleY = new BulkEditSlider(Panel_Marquee_Position, ID_SLIDER_Marquee_ScaleY, 100, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Marquee_ScaleY"));
	FlexGridSizer138->Add(Slider_Marquee_ScaleY, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Marquee_ScaleY = new BulkEditTextCtrl(Panel_Marquee_Position, IDD_TEXTCTRL_Marquee_ScaleY, _("100"), wxDefaultPosition, wxDLG_UNIT(Panel_Marquee_Position,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Marquee_ScaleY"));
	TextCtrl_Marquee_ScaleY->SetMaxLength(3);
	FlexGridSizer138->Add(TextCtrl_Marquee_ScaleY, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_Marquee_ScaleY = new wxBitmapButton(Panel_Marquee_Position, ID_BITMAPBUTTON_SLIDER_Marquee_ScaleY, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Marquee_ScaleY"));
	BitmapButton_Marquee_ScaleY->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer138->Add(BitmapButton_Marquee_ScaleY, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer135->Add(FlexGridSizer138, 1, wxALL|wxEXPAND, 5);
	StaticLine1 = new wxStaticLine(Panel_Marquee_Position, ID_STATICLINE1, wxDefaultPosition, wxSize(10,-1), wxLI_HORIZONTAL, _T("ID_STATICLINE1"));
	FlexGridSizer135->Add(StaticLine1, 1, wxALL|wxEXPAND, 5);
	CheckBox_Marquee_PixelOffsets = new BulkEditCheckBox(Panel_Marquee_Position, ID_CHECKBOX_Marquee_PixelOffsets, _("Offsets In Pixels"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Marquee_PixelOffsets"));
	CheckBox_Marquee_PixelOffsets->SetValue(false);
	FlexGridSizer135->Add(CheckBox_Marquee_PixelOffsets, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer136 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer136->AddGrowableCol(0);
	FlexGridSizer137 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer137->AddGrowableCol(1);
	StaticText200 = new wxStaticText(Panel_Marquee_Position, ID_STATICTEXT_MarqueeXC, _("X-axis Center"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_MarqueeXC"));
	FlexGridSizer137->Add(StaticText200, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_MarqueeXC = new BulkEditSlider(Panel_Marquee_Position, ID_SLIDER_MarqueeXC, 0, -100, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_MarqueeXC"));
	FlexGridSizer137->Add(Slider_MarqueeXC, 1, wxALL|wxEXPAND, 5);
	CheckBox_Marquee_WrapX = new BulkEditCheckBox(Panel_Marquee_Position, ID_CHECKBOX_Marquee_WrapX, _("Wrap X"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Marquee_WrapX"));
	CheckBox_Marquee_WrapX->SetValue(false);
	FlexGridSizer137->Add(CheckBox_Marquee_WrapX, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_MarqueeXC = new BulkEditTextCtrl(Panel_Marquee_Position, IDD_TEXTCTRL_MarqueeXC, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel_Marquee_Position,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_MarqueeXC"));
	TextCtrl_MarqueeXC->SetMaxLength(3);
	FlexGridSizer137->Add(TextCtrl_MarqueeXC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer136->Add(FlexGridSizer137, 1, wxALL|wxEXPAND, 5);
	GridBagSizer5 = new wxGridBagSizer(0, 0);
	StaticText201 = new wxStaticText(Panel_Marquee_Position, ID_STATICTEXT_MarqueeYC, _("Y-axis Center"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_MarqueeYC"));
	GridBagSizer5->Add(StaticText201, wxGBPosition(1, 0), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_MarqueeYC = new BulkEditTextCtrl(Panel_Marquee_Position, IDD_TEXTCTRL_MarqueeYC, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel_Marquee_Position,wxSize(20,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_MarqueeYC"));
	TextCtrl_MarqueeYC->SetMaxLength(3);
	GridBagSizer5->Add(TextCtrl_MarqueeYC, wxGBPosition(2, 0), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Slider_MarqueeYC = new BulkEditSlider(Panel_Marquee_Position, ID_SLIDER_MarqueeYC, 0, -100, 100, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL|wxSL_INVERSE, wxDefaultValidator, _T("ID_SLIDER_MarqueeYC"));
	GridBagSizer5->Add(Slider_MarqueeYC, wxGBPosition(0, 1), wxGBSpan(4, 1), wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer136->Add(GridBagSizer5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer135->Add(FlexGridSizer136, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel_Marquee_Position->SetSizer(FlexGridSizer135);
	FlexGridSizer135->Fit(Panel_Marquee_Position);
	FlexGridSizer135->SetSizeHints(Panel_Marquee_Position);
	Notebook_Marquee->AddPage(Panel_Marquee_Settings, _("Settings"), false);
	Notebook_Marquee->AddPage(Panel_Marquee_Position, _("Position"), false);
	FlexGridSizer23->Add(Notebook_Marquee, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer23);
	FlexGridSizer23->Fit(this);
	FlexGridSizer23->SetSizeHints(this);

	Connect(ID_BITMAPBUTTON_SLIDER_Marquee_Band_Size,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MarqueePanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Marquee_Skip_Size,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MarqueePanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Marquee_Thickness,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MarqueePanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Marquee_Stagger,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MarqueePanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Marquee_Speed,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MarqueePanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Marquee_Start,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MarqueePanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Marquee_ScaleX,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MarqueePanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_Marquee_ScaleY,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MarqueePanel::OnLockButtonClick);
	//*)
    SetName("ID_PANEL_MARQUEE");
}

MarqueePanel::~MarqueePanel()
{
	//(*Destroy(MarqueePanel)
	//*)
}

PANEL_EVENT_HANDLERS(MarqueePanel)