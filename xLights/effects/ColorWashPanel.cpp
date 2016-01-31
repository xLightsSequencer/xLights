#include "ColorWashPanel.h"
#include "../../include/padlock16x16-blue.xpm"
#include "EffectPanelUtils.h"

//(*InternalHeaders(ColorWashPanel)
#include <wx/bmpbuttn.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/notebook.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/slider.h>
#include <wx/intl.h>
#include <wx/gbsizer.h>
#include <wx/stattext.h>
#include <wx/bitmap.h>
#include <wx/textctrl.h>
#include <wx/image.h>
//*)

//(*IdInit(ColorWashPanel)
const long ColorWashPanel::ID_STATICTEXT21 = wxNewId();
const long ColorWashPanel::IDD_SLIDER_ColorWash_Cycles = wxNewId();
const long ColorWashPanel::ID_TEXTCTRL_ColorWash_Cycles = wxNewId();
const long ColorWashPanel::ID_BITMAPBUTTON_SLIDER_ColorWash_Count = wxNewId();
const long ColorWashPanel::ID_CHECKBOX_ColorWash_VFade = wxNewId();
const long ColorWashPanel::ID_BITMAPBUTTON_CHECKBOX_ColorWash_VFade = wxNewId();
const long ColorWashPanel::ID_CHECKBOX_ColorWash_HFade = wxNewId();
const long ColorWashPanel::ID_BITMAPBUTTON_CHECKBOX_ColorWash_HFade = wxNewId();
const long ColorWashPanel::ID_CHECKBOX_ColorWash_EntireModel = wxNewId();
const long ColorWashPanel::ID_CHECKBOX_ColorWash_Shimmer = wxNewId();
const long ColorWashPanel::ID_CHECKBOX_ColorWash_CircularPalette = wxNewId();
const long ColorWashPanel::ID_SLIDER_ColorWash_X1 = wxNewId();
const long ColorWashPanel::IDD_TEXTCTRL_ColorWash_X1 = wxNewId();
const long ColorWashPanel::IDD_TEXTCTRL_ColorWash_Y1 = wxNewId();
const long ColorWashPanel::ID_SLIDER_ColorWash_Y1 = wxNewId();
const long ColorWashPanel::ID_PANEL46 = wxNewId();
const long ColorWashPanel::ID_SLIDER_ColorWash_X2 = wxNewId();
const long ColorWashPanel::IDD_TEXTCTRL_ColorWash_X2 = wxNewId();
const long ColorWashPanel::IDD_TEXTCTRL_ColorWash_Y2 = wxNewId();
const long ColorWashPanel::ID_SLIDER_ColorWash_Y2 = wxNewId();
const long ColorWashPanel::ID_PANEL47 = wxNewId();
const long ColorWashPanel::IDD_NOTEBOOK_ColorWashRect = wxNewId();
//*)

BEGIN_EVENT_TABLE(ColorWashPanel,wxPanel)
	//(*EventTable(ColorWashPanel)
	//*)
END_EVENT_TABLE()

ColorWashPanel::ColorWashPanel(wxWindow* parent)
{
	//(*Initialize(ColorWashPanel)
	wxFlexGridSizer* FlexGridSizer116;
	wxTextCtrl* TextCtrl10;
	wxStaticText* StaticText170;
	wxSlider* Slider4;
	wxStaticText* StaticText167;
	wxSlider* Slider3;
	wxFlexGridSizer* FlexGridSizer124;
	wxTextCtrl* TextCtrl9;
	wxStaticText* StaticText168;
	wxFlexGridSizer* FlexGridSizer115;
	wxFlexGridSizer* FlexGridSizer75;
	wxNotebook* Notebook5;
	wxFlexGridSizer* FlexGridSizer9;
	wxStaticText* StaticText169;
	wxSlider* Slider5;
	wxSlider* Slider_ColorWash_Count;
	wxFlexGridSizer* FlexGridSizer118;
	wxFlexGridSizer* FlexGridSizer37;
	wxFlexGridSizer* FlexGridSizer117;
	wxFlexGridSizer* FlexGridSizer114;
	wxTextCtrl* TextCtrl6;
	wxPanel* Panel15;
	wxPanel* Panel13;
	wxGridBagSizer* GridBagSizer1;
	wxCheckBox* CheckBox3;
	wxTextCtrl* TextCtrl8;
	wxGridBagSizer* GridBagSizer4;
	wxCheckBox* ColorWashEntireModelCheckbox;
	wxTextCtrl* TextCtrl7;
	wxSlider* Slider6;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer37 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer37->AddGrowableCol(0);
	FlexGridSizer9 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer9->AddGrowableCol(1);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT21, _("Count"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT21"));
	FlexGridSizer9->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Slider_ColorWash_Count = new wxSlider(this, IDD_SLIDER_ColorWash_Cycles, 10, 1, 200, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_ColorWash_Cycles"));
	FlexGridSizer9->Add(Slider_ColorWash_Count, 1, wxALL|wxEXPAND, 2);
	TextCtrl10 = new wxTextCtrl(this, ID_TEXTCTRL_ColorWash_Cycles, _("1.0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(20,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL_ColorWash_Cycles"));
	TextCtrl10->SetMaxLength(3);
	FlexGridSizer9->Add(TextCtrl10, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_ColorWashCount = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_ColorWash_Count, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_ColorWash_Count"));
	BitmapButton_ColorWashCount->SetDefault();
	BitmapButton_ColorWashCount->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer9->Add(BitmapButton_ColorWashCount, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer37->Add(FlexGridSizer9, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer75 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer75->AddGrowableCol(2);
	CheckBox_ColorWash_VFade = new wxCheckBox(this, ID_CHECKBOX_ColorWash_VFade, _("Vertical Fade"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_ColorWash_VFade"));
	CheckBox_ColorWash_VFade->SetValue(false);
	FlexGridSizer75->Add(CheckBox_ColorWash_VFade, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_ColorWashVFade = new wxBitmapButton(this, ID_BITMAPBUTTON_CHECKBOX_ColorWash_VFade, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_ColorWash_VFade"));
	BitmapButton_ColorWashVFade->SetDefault();
	BitmapButton_ColorWashVFade->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer75->Add(BitmapButton_ColorWashVFade, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	CheckBox_ColorWash_HFade = new wxCheckBox(this, ID_CHECKBOX_ColorWash_HFade, _("Horizontal Fade"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_ColorWash_HFade"));
	CheckBox_ColorWash_HFade->SetValue(false);
	FlexGridSizer75->Add(CheckBox_ColorWash_HFade, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton_ColorWashHFade = new wxBitmapButton(this, ID_BITMAPBUTTON_CHECKBOX_ColorWash_HFade, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_ColorWash_HFade"));
	BitmapButton_ColorWashHFade->SetDefault();
	BitmapButton_ColorWashHFade->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	FlexGridSizer75->Add(BitmapButton_ColorWashHFade, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer37->Add(FlexGridSizer75, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer114 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer114->AddGrowableCol(0);
	FlexGridSizer124 = new wxFlexGridSizer(0, 3, 0, 0);
	ColorWashEntireModelCheckbox = new wxCheckBox(this, ID_CHECKBOX_ColorWash_EntireModel, _("Entire Model"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_ColorWash_EntireModel"));
	ColorWashEntireModelCheckbox->SetValue(true);
	FlexGridSizer124->Add(ColorWashEntireModelCheckbox, 1, wxALL|wxALIGN_LEFT|wxALIGN_TOP, 5);
	CheckBox3 = new wxCheckBox(this, ID_CHECKBOX_ColorWash_Shimmer, _("Shimmer"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_ColorWash_Shimmer"));
	CheckBox3->SetValue(false);
	FlexGridSizer124->Add(CheckBox3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox4 = new wxCheckBox(this, ID_CHECKBOX_ColorWash_CircularPalette, _("Circular Palette"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_ColorWash_CircularPalette"));
	CheckBox4->SetValue(false);
	FlexGridSizer124->Add(CheckBox4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer114->Add(FlexGridSizer124, 1, wxALL|wxEXPAND, 0);
	Notebook5 = new wxNotebook(this, IDD_NOTEBOOK_ColorWashRect, wxDefaultPosition, wxDefaultSize, 0, _T("IDD_NOTEBOOK_ColorWashRect"));
	Notebook5->Disable();
	Panel13 = new wxPanel(Notebook5, ID_PANEL46, wxPoint(15,49), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL46"));
	FlexGridSizer115 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer115->AddGrowableCol(0);
	FlexGridSizer116 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer116->AddGrowableCol(1);
	StaticText167 = new wxStaticText(Panel13, wxID_ANY, _("X"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer116->Add(StaticText167, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider3 = new wxSlider(Panel13, ID_SLIDER_ColorWash_X1, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_ColorWash_X1"));
	FlexGridSizer116->Add(Slider3, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer116->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl6 = new wxTextCtrl(Panel13, IDD_TEXTCTRL_ColorWash_X1, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel13,wxSize(24,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_ColorWash_X1"));
	TextCtrl6->SetMaxLength(4);
	FlexGridSizer116->Add(TextCtrl6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer115->Add(FlexGridSizer116, 1, wxALL|wxEXPAND, 5);
	GridBagSizer1 = new wxGridBagSizer(0, 0);
	StaticText168 = new wxStaticText(Panel13, wxID_ANY, _("Y"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	GridBagSizer1->Add(StaticText168, wxGBPosition(1, 0), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl7 = new wxTextCtrl(Panel13, IDD_TEXTCTRL_ColorWash_Y1, _("0"), wxDefaultPosition, wxDLG_UNIT(Panel13,wxSize(24,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_ColorWash_Y1"));
	TextCtrl7->SetMaxLength(4);
	GridBagSizer1->Add(TextCtrl7, wxGBPosition(2, 0), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Slider4 = new wxSlider(Panel13, ID_SLIDER_ColorWash_Y1, 0, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL|wxSL_INVERSE, wxDefaultValidator, _T("ID_SLIDER_ColorWash_Y1"));
	GridBagSizer1->Add(Slider4, wxGBPosition(0, 1), wxGBSpan(4, 1), wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer115->Add(GridBagSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel13->SetSizer(FlexGridSizer115);
	FlexGridSizer115->Fit(Panel13);
	FlexGridSizer115->SetSizeHints(Panel13);
	Panel15 = new wxPanel(Notebook5, ID_PANEL47, wxPoint(104,13), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL47"));
	FlexGridSizer117 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer117->AddGrowableCol(0);
	FlexGridSizer118 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer118->AddGrowableCol(1);
	StaticText169 = new wxStaticText(Panel15, wxID_ANY, _("X"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer118->Add(StaticText169, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	Slider5 = new wxSlider(Panel15, ID_SLIDER_ColorWash_X2, 100, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_ColorWash_X2"));
	FlexGridSizer118->Add(Slider5, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer118->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl8 = new wxTextCtrl(Panel15, IDD_TEXTCTRL_ColorWash_X2, _("100"), wxDefaultPosition, wxDLG_UNIT(Panel15,wxSize(24,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_ColorWash_X2"));
	TextCtrl8->SetMaxLength(4);
	FlexGridSizer118->Add(TextCtrl8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer117->Add(FlexGridSizer118, 1, wxALL|wxEXPAND, 5);
	GridBagSizer4 = new wxGridBagSizer(0, 0);
	StaticText170 = new wxStaticText(Panel15, wxID_ANY, _("Y"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	GridBagSizer4->Add(StaticText170, wxGBPosition(1, 0), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl9 = new wxTextCtrl(Panel15, IDD_TEXTCTRL_ColorWash_Y2, _("100"), wxDefaultPosition, wxDLG_UNIT(Panel15,wxSize(24,-1)), wxTE_CENTRE, wxDefaultValidator, _T("IDD_TEXTCTRL_ColorWash_Y2"));
	TextCtrl9->SetMaxLength(4);
	GridBagSizer4->Add(TextCtrl9, wxGBPosition(2, 0), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Slider6 = new wxSlider(Panel15, ID_SLIDER_ColorWash_Y2, 100, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL|wxSL_INVERSE, wxDefaultValidator, _T("ID_SLIDER_ColorWash_Y2"));
	GridBagSizer4->Add(Slider6, wxGBPosition(0, 1), wxGBSpan(4, 1), wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer117->Add(GridBagSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel15->SetSizer(FlexGridSizer117);
	FlexGridSizer117->Fit(Panel15);
	FlexGridSizer117->SetSizeHints(Panel15);
	Notebook5->AddPage(Panel13, _("Corner 1"), false);
	Notebook5->AddPage(Panel15, _("Corner 2"), false);
	FlexGridSizer114->Add(Notebook5, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer37->Add(FlexGridSizer114, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer37);
	FlexGridSizer37->Fit(this);
	FlexGridSizer37->SetSizeHints(this);

	Connect(IDD_SLIDER_ColorWash_Cycles,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&ColorWashPanel::UpdateLinkedTextCtrlFloat);
	Connect(ID_TEXTCTRL_ColorWash_Cycles,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ColorWashPanel::UpdateLinkedSliderFloat);
	Connect(ID_BITMAPBUTTON_SLIDER_ColorWash_Count,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorWashPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_ColorWash_VFade,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorWashPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_ColorWash_HFade,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorWashPanel::OnLockButtonClick);
	Connect(ID_CHECKBOX_ColorWash_EntireModel,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ColorWashPanel::OnColorWashEntireModelClicked);
	Connect(ID_SLIDER_ColorWash_X1,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&ColorWashPanel::UpdateLinkedTextCtrl);
	Connect(IDD_TEXTCTRL_ColorWash_X1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ColorWashPanel::UpdateLinkedSlider);
	Connect(IDD_TEXTCTRL_ColorWash_Y1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ColorWashPanel::UpdateLinkedSlider);
	Connect(ID_SLIDER_ColorWash_Y1,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&ColorWashPanel::UpdateLinkedTextCtrl);
	Connect(ID_SLIDER_ColorWash_X2,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&ColorWashPanel::UpdateLinkedTextCtrl);
	Connect(IDD_TEXTCTRL_ColorWash_X2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ColorWashPanel::UpdateLinkedSlider);
	Connect(IDD_TEXTCTRL_ColorWash_Y2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ColorWashPanel::UpdateLinkedSlider);
	Connect(ID_SLIDER_ColorWash_Y2,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&ColorWashPanel::UpdateLinkedTextCtrl);
	//*)
    SetName("ID_PANEL_COLORWASH");
}

ColorWashPanel::~ColorWashPanel()
{
	//(*Destroy(ColorWashPanel)
	//*)
}

PANEL_EVENT_HANDLERS(ColorWashPanel)


static inline void EnableControl(wxWindow *w, int id, bool e) {
    wxWindow *c = w->FindWindowById(id);
    if (c) c->Enable(e);
}
void ColorWashPanel::OnColorWashEntireModelClicked(wxCommandEvent& event)
{
    bool on = ((wxCheckBox*)event.GetEventObject())->GetValue();
    EnableControl(this, IDD_NOTEBOOK_ColorWashRect, !on);
}