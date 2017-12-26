#include "TimingPanel.h"
#include "../include/padlock16x16-blue.xpm" //-DJ
#include <wx/msgdlg.h>
#include <wx/config.h>
//(*InternalHeaders(TimingPanel)
#include <wx/bitmap.h>
#include <wx/settings.h>
#include <wx/intl.h>
#include <wx/image.h>
#include <wx/string.h>
//*)

#include "models/Model.h"
#include "effects/EffectPanelUtils.h"

//(*IdInit(TimingPanel)
const long TimingPanel::ID_CHECKBOX_ResetTimingPanel = wxNewId();
const long TimingPanel::ID_CHECKBOX_LayerMorph = wxNewId();
const long TimingPanel::ID_BITMAPBUTTON_CHECKBOX_LayerMorph = wxNewId();
const long TimingPanel::ID_CHOICE_LayerMethod = wxNewId();
const long TimingPanel::ID_SLIDER_EffectLayerMix = wxNewId();
const long TimingPanel::IDD_TEXTCTRL_EffectLayerMix = wxNewId();
const long TimingPanel::ID_BITMAPBUTTON_SLIDER_EffectLayerMix = wxNewId();
const long TimingPanel::ID_CHOICE_In_Transition_Type = wxNewId();
const long TimingPanel::ID_STATICTEXT_Fadein = wxNewId();
const long TimingPanel::ID_TEXTCTRL_Fadein = wxNewId();
const long TimingPanel::ID_STATICTEXT_In_Transition_Adjust = wxNewId();
const long TimingPanel::ID_SLIDER_In_Transition_Adjust = wxNewId();
const long TimingPanel::IDD_TEXTCTRL_In_Transition_Adjust = wxNewId();
const long TimingPanel::ID_CHECKBOX_In_Transition_Reverse = wxNewId();
const long TimingPanel::ID_PANEL2 = wxNewId();
const long TimingPanel::ID_CHOICE_Out_Transition_Type = wxNewId();
const long TimingPanel::ID_STATICTEXT_Fadeout = wxNewId();
const long TimingPanel::ID_TEXTCTRL_Fadeout = wxNewId();
const long TimingPanel::ID_STATICTEXT_Out_Transition_Adjust = wxNewId();
const long TimingPanel::ID_SLIDER_Out_Transition_Adjust = wxNewId();
const long TimingPanel::IDD_TEXTCTRL_Out_Transition_Adjust = wxNewId();
const long TimingPanel::ID_CHECKBOX_Out_Transition_Reverse = wxNewId();
const long TimingPanel::ID_PANEL3 = wxNewId();
const long TimingPanel::IDD_NOTEBOOK1 = wxNewId();
const long TimingPanel::ID_SCROLLEDWINDOW1 = wxNewId();
const long TimingPanel::ID_PANEL1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(TimingPanel,wxPanel)
	//(*EventTable(TimingPanel)
	//*)
END_EVENT_TABLE()

TimingPanel::TimingPanel(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(TimingPanel)
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer10;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer9;
	wxPanel* Panel1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer7;
	wxFlexGridSizer* FlexGridSizer8;
	wxFlexGridSizer* FlexGridSizer13;
	wxFlexGridSizer* FlexGridSizer12;
	wxFlexGridSizer* FlexGridSizer6;
	wxPanel* Panel2;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer11;

	Create(parent, wxID_ANY, wxDefaultPosition, wxSize(301,262), wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer3->AddGrowableCol(2);
	Panel_Sizer = new wxPanel(this, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	FlexGridSizer4 = new wxFlexGridSizer(0, 3, 0, 0);
	ScrolledWindowTiming = new wxScrolledWindow(Panel_Sizer, ID_SCROLLEDWINDOW1, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL, _T("ID_SCROLLEDWINDOW1"));
	FlexGridSizer5 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer5->AddGrowableCol(0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer2->AddGrowableCol(1);
	CheckBox_ResetTimingPanel = new wxCheckBox(ScrolledWindowTiming, ID_CHECKBOX_ResetTimingPanel, _("Reset panel when changing effects"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_ResetTimingPanel"));
	CheckBox_ResetTimingPanel->SetValue(true);
	FlexGridSizer2->Add(CheckBox_ResetTimingPanel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_LayerMorph = new wxCheckBox(ScrolledWindowTiming, ID_CHECKBOX_LayerMorph, _("Morph"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_LayerMorph"));
	CheckBox_LayerMorph->SetValue(false);
	CheckBox_LayerMorph->SetToolTip(_("Gradual cross-fade from Effect1 to Effect2"));
	FlexGridSizer2->Add(CheckBox_LayerMorph, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	BitmapButton_CheckBox_LayerMorph = new wxBitmapButton(ScrolledWindowTiming, ID_BITMAPBUTTON_CHECKBOX_LayerMorph, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_LayerMorph"));
	BitmapButton_CheckBox_LayerMorph->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	BitmapButton_CheckBox_LayerMorph->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	FlexGridSizer2->Add(BitmapButton_CheckBox_LayerMorph, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	Choice_LayerMethod = new BulkEditChoice(ScrolledWindowTiming, ID_CHOICE_LayerMethod, wxDefaultPosition, wxDefaultSize, 0, 0, wxFULL_REPAINT_ON_RESIZE, wxDefaultValidator, _T("ID_CHOICE_LayerMethod"));
	Choice_LayerMethod->SetSelection( Choice_LayerMethod->Append(_("Normal")) );
	Choice_LayerMethod->Append(_("Effect 1"));
	Choice_LayerMethod->Append(_("Effect 2"));
	Choice_LayerMethod->Append(_("1 is Mask"));
	Choice_LayerMethod->Append(_("2 is Mask"));
	Choice_LayerMethod->Append(_("1 is Unmask"));
	Choice_LayerMethod->Append(_("2 is Unmask"));
	Choice_LayerMethod->Append(_("1 reveals 2"));
	Choice_LayerMethod->Append(_("2 reveals 1"));
	Choice_LayerMethod->Append(_("Shadow 1 on 2"));
	Choice_LayerMethod->Append(_("Shadow 2 on 1"));
	Choice_LayerMethod->Append(_("Layered"));
	Choice_LayerMethod->Append(_("Average"));
	Choice_LayerMethod->Append(_("Bottom-Top"));
	Choice_LayerMethod->Append(_("Left-Right"));
	Choice_LayerMethod->Append(_("Additive"));
	Choice_LayerMethod->Append(_("Subtractive"));
	Choice_LayerMethod->Append(_("Max"));
	Choice_LayerMethod->Append(_("Min"));
	Choice_LayerMethod->SetToolTip(_("Layering defines how Effect 1 and Effect 2 will be mixed together.\nHere are the Choices\n* Effect 1: Shows only Effect 1. Slide the slider to the right to blend in some Effect 2. \n* Effect 2: Shows only Effect 2. Slide the slider to the right to blend in some Effect 1.\n* 1 is Mask: (Shadow) Effect 1 will cast a shadow onto Effect 2 for every Effect 1 pixel that has a non-black value.\n* 2 is Mask: (Shadow) Effect 2 will cast a shadow onto Effect 1 for every Effect 2 pixel that has a non-black value.\n* 1 is UnMask:  (Mask) Only allow Effect 2 to show through when Effect 1 has a non-black pixel.\n* 2 is UnMask:  (Mask) Only allow Effect 1 to show through when Effect 2 has a non-black pixel.\n* Shadow 1 on 2: Take brightness and Saturation from 1, use hue from 2\n* Shadow 2 on 1: Take brightness and Saturation from 2, use hue from 1\n* 1 reveals 2: (Superimpose) Effect 1 reveals Effect 2\n* 2 reveals 1: (Superimpose) Effect 2 reveals Effect 1\n* Layered: Effect 1 only shows in black regions of Effect 2.\n* Average: Take value of Effect  and Add it to Value from Effect 2. Average the sum\n* Bottom-top: Effect 1 is put on bottom of model, Effect 2 is put on top in a plit screen display\n* Left-Right: Effect goes 1 goes on the left side, Effect 2 on the right. Split screen goes down middle of model.\n* Additive -  Take value of Effect 1  and Add it to Value from Effect 2.\n* Subtractive -  Take value of Effect 1  and Subtract it from the Value from Effect 2.\n* Max - Take the maximum value for each channel from both effects\n* Min - Take the minimum value for each channel from both effects"));
	FlexGridSizer2->Add(Choice_LayerMethod, 1, wxALL, 2);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	Slider_EffectLayerMix = new BulkEditSlider(ScrolledWindowTiming, ID_SLIDER_EffectLayerMix, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_EffectLayerMix"));
	FlexGridSizer1->Add(Slider_EffectLayerMix, 1, wxALL|wxEXPAND, 1);
	TextCtrl_EffectLayerMix = new BulkEditTextCtrl(ScrolledWindowTiming, IDD_TEXTCTRL_EffectLayerMix, _("0"), wxDefaultPosition, wxDLG_UNIT(ScrolledWindowTiming,wxSize(20,-1)), wxTE_PROCESS_ENTER|wxTAB_TRAVERSAL, wxDefaultValidator, _T("IDD_TEXTCTRL_EffectLayerMix"));
	TextCtrl_EffectLayerMix->SetMaxLength(3);
	FlexGridSizer1->Add(TextCtrl_EffectLayerMix, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer2->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 2);
	BitmapButton_EffectLayerMix = new wxBitmapButton(ScrolledWindowTiming, ID_BITMAPBUTTON_SLIDER_EffectLayerMix, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_EffectLayerMix"));
	BitmapButton_EffectLayerMix->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	BitmapButton_EffectLayerMix->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
	FlexGridSizer2->Add(BitmapButton_EffectLayerMix, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer5->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 2);
	Notebook1 = new wxNotebook(ScrolledWindowTiming, IDD_NOTEBOOK1, wxDefaultPosition, wxDefaultSize, 0, _T("IDD_NOTEBOOK1"));
	Panel1 = new wxPanel(Notebook1, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
	FlexGridSizer6 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer6->AddGrowableCol(0);
	FlexGridSizer10 = new wxFlexGridSizer(0, 4, 0, 0);
	Choice_In_Transition_Type = new BulkEditChoice(Panel1, ID_CHOICE_In_Transition_Type, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_In_Transition_Type"));
	Choice_In_Transition_Type->SetSelection( Choice_In_Transition_Type->Append(_("Fade")) );
	Choice_In_Transition_Type->Append(_("Wipe"));
	Choice_In_Transition_Type->Append(_("Clock"));
	Choice_In_Transition_Type->Append(_("From Middle"));
	Choice_In_Transition_Type->Append(_("Square Explode"));
	Choice_In_Transition_Type->Append(_("Circle Explode"));
	Choice_In_Transition_Type->Append(_("Blinds"));
	Choice_In_Transition_Type->Append(_("Blend"));
	Choice_In_Transition_Type->Append(_("Slide Checks"));
	Choice_In_Transition_Type->Append(_("Slide Bars"));
	FlexGridSizer10->Add(Choice_In_Transition_Type, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText2 = new wxStaticText(Panel1, ID_STATICTEXT_Fadein, _("Time (s)"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fadein"));
	FlexGridSizer10->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_Fadein = new BulkEditTextCtrl(Panel1, ID_TEXTCTRL_Fadein, _("0.00"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_Fadein"));
	TextCtrl_Fadein->SetMaxLength(4);
	FlexGridSizer10->Add(TextCtrl_Fadein, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer6->Add(FlexGridSizer10, 1, wxALL|wxEXPAND, 1);
	FlexGridSizer11 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer11->AddGrowableCol(1);
	InAdjustmentText = new wxStaticText(Panel1, ID_STATICTEXT_In_Transition_Adjust, _("Adjustment"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_In_Transition_Adjust"));
	FlexGridSizer11->Add(InAdjustmentText, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	Slider_In_Adjust = new BulkEditSlider(Panel1, ID_SLIDER_In_Transition_Adjust, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_In_Transition_Adjust"));
	FlexGridSizer11->Add(Slider_In_Adjust, 1, wxALL|wxEXPAND, 1);
	TextCtrl_In_Adjust = new BulkEditTextCtrl(Panel1, IDD_TEXTCTRL_In_Transition_Adjust, _("50"), wxDefaultPosition, wxDLG_UNIT(Panel1,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_In_Transition_Adjust"));
	FlexGridSizer11->Add(TextCtrl_In_Adjust, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer6->Add(FlexGridSizer11, 1, wxALL|wxEXPAND, 1);
	FlexGridSizer7 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer7->AddGrowableCol(0);
	CheckBox_In_Reverse = new BulkEditCheckBox(Panel1, ID_CHECKBOX_In_Transition_Reverse, _("Reverse"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_In_Transition_Reverse"));
	CheckBox_In_Reverse->SetValue(false);
	FlexGridSizer7->Add(CheckBox_In_Reverse, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer6->Add(FlexGridSizer7, 1, wxALL|wxEXPAND, 1);
	Panel1->SetSizer(FlexGridSizer6);
	FlexGridSizer6->Fit(Panel1);
	FlexGridSizer6->SetSizeHints(Panel1);
	Panel2 = new wxPanel(Notebook1, ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL3"));
	FlexGridSizer8 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer8->AddGrowableCol(0);
	FlexGridSizer12 = new wxFlexGridSizer(0, 3, 0, 0);
	Choice_Out_Transition_Type = new BulkEditChoice(Panel2, ID_CHOICE_Out_Transition_Type, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Out_Transition_Type"));
	Choice_Out_Transition_Type->SetSelection( Choice_Out_Transition_Type->Append(_("Fade")) );
	Choice_Out_Transition_Type->Append(_("Wipe"));
	Choice_Out_Transition_Type->Append(_("Clock"));
	Choice_Out_Transition_Type->Append(_("From Middle"));
	Choice_Out_Transition_Type->Append(_("Square Explode"));
	Choice_Out_Transition_Type->Append(_("Circle Explode"));
	Choice_Out_Transition_Type->Append(_("Blinds"));
	Choice_Out_Transition_Type->Append(_("Blend"));
	Choice_Out_Transition_Type->Append(_("Slide Checks"));
	Choice_Out_Transition_Type->Append(_("Slide Bars"));
	FlexGridSizer12->Add(Choice_Out_Transition_Type, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	StaticText4 = new wxStaticText(Panel2, ID_STATICTEXT_Fadeout, _("Time (s)"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fadeout"));
	FlexGridSizer12->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_Fadeout = new BulkEditTextCtrl(Panel2, ID_TEXTCTRL_Fadeout, _("0.00"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_Fadeout"));
	TextCtrl_Fadeout->SetMaxLength(4);
	FlexGridSizer12->Add(TextCtrl_Fadeout, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer8->Add(FlexGridSizer12, 1, wxALL|wxEXPAND, 1);
	FlexGridSizer9 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer9->AddGrowableCol(1);
	OutAdjustmentText = new wxStaticText(Panel2, ID_STATICTEXT_Out_Transition_Adjust, _("Adjustment"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Out_Transition_Adjust"));
	FlexGridSizer9->Add(OutAdjustmentText, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	Slider_Out_Adjust = new BulkEditSlider(Panel2, ID_SLIDER_Out_Transition_Adjust, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Out_Transition_Adjust"));
	FlexGridSizer9->Add(Slider_Out_Adjust, 1, wxALL|wxEXPAND, 2);
	TextCtrl_Out_Adjust = new BulkEditTextCtrl(Panel2, IDD_TEXTCTRL_Out_Transition_Adjust, _("50"), wxDefaultPosition, wxDLG_UNIT(Panel2,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Out_Transition_Adjust"));
	FlexGridSizer9->Add(TextCtrl_Out_Adjust, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer8->Add(FlexGridSizer9, 1, wxALL|wxEXPAND, 1);
	FlexGridSizer13 = new wxFlexGridSizer(0, 3, 0, 0);
	CheckBox_Out_Reverse = new BulkEditCheckBox(Panel2, ID_CHECKBOX_Out_Transition_Reverse, _("Reverse"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Out_Transition_Reverse"));
	CheckBox_Out_Reverse->SetValue(false);
	FlexGridSizer13->Add(CheckBox_Out_Reverse, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	FlexGridSizer8->Add(FlexGridSizer13, 1, wxALL|wxEXPAND, 1);
	Panel2->SetSizer(FlexGridSizer8);
	FlexGridSizer8->Fit(Panel2);
	FlexGridSizer8->SetSizeHints(Panel2);
	Notebook1->AddPage(Panel1, _("In Transition"), false);
	Notebook1->AddPage(Panel2, _("Out Transition"), false);
	FlexGridSizer5->Add(Notebook1, 1, wxALL|wxEXPAND, 0);
	ScrolledWindowTiming->SetSizer(FlexGridSizer5);
	FlexGridSizer5->Fit(ScrolledWindowTiming);
	FlexGridSizer5->SetSizeHints(ScrolledWindowTiming);
	FlexGridSizer4->Add(ScrolledWindowTiming, 0, wxALIGN_LEFT, 0);
	Panel_Sizer->SetSizer(FlexGridSizer4);
	FlexGridSizer4->Fit(Panel_Sizer);
	FlexGridSizer4->SetSizeHints(Panel_Sizer);
	FlexGridSizer3->Add(Panel_Sizer, 0, wxALIGN_LEFT, 0);
	SetSizer(FlexGridSizer3);
	SetSizer(FlexGridSizer3);
	Layout();

	Connect(ID_CHECKBOX_ResetTimingPanel,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&TimingPanel::OnCheckBox_ResetTimingPanelClick);
	Connect(ID_BITMAPBUTTON_CHECKBOX_LayerMorph,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TimingPanel::OnLockButtonClick);
	Connect(ID_BITMAPBUTTON_SLIDER_EffectLayerMix,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TimingPanel::OnLockButtonClick);
	Connect(ID_CHOICE_In_Transition_Type,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&TimingPanel::OnTransitionTypeSelect);
	Connect(ID_CHOICE_Out_Transition_Type,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&TimingPanel::OnTransitionTypeSelect);
	Panel_Sizer->Connect(wxEVT_SIZE,(wxObjectEventFunction)&TimingPanel::OnResize,0,this);
	//*)

    // Turn off bulk edit for this setting
    Slider_EffectLayerMix->SetSupportsBulkEdit(false);
    TextCtrl_EffectLayerMix->SetSupportsBulkEdit(false);

    SetName("Timing");

    wxConfigBase* config = wxConfigBase::Get();
    bool reset;
    config->Read("xLightsResetTimingPanel", &reset, false);
    CheckBox_ResetTimingPanel->SetValue(reset);
}

TimingPanel::~TimingPanel()
{
	//(*Destroy(TimingPanel)
	//*)
}

void TimingPanel::OnResize(wxSizeEvent& event)
{
    wxSize s = GetSize();
    Panel_Sizer->SetSize(s);
    Panel_Sizer->SetMinSize(s);
    Panel_Sizer->SetMaxSize(s);
    Panel_Sizer->Refresh();

    ScrolledWindowTiming->SetSize(s);
    ScrolledWindowTiming->SetMinSize(s);
    ScrolledWindowTiming->SetMaxSize(s);

    ScrolledWindowTiming->FitInside();
    ScrolledWindowTiming->SetScrollRate(5, 5);
    ScrolledWindowTiming->Refresh();
}

void TimingPanel::SetDefaultControls(const Model *model, bool optionbased) {
    if (!optionbased || CheckBox_ResetTimingPanel->GetValue())
    {
        CheckBox_LayerMorph->SetValue(false);
        Choice_LayerMethod->SetSelection(0);
        Slider_EffectLayerMix->SetValue(0);
	    TextCtrl_EffectLayerMix->SetValue("0");
        TextCtrl_Fadein->SetValue("0.00");
        TextCtrl_Fadeout->SetValue("0.00");
        Slider_In_Adjust->SetValue(50);
        Slider_Out_Adjust->SetValue(50);
        Choice_In_Transition_Type->SetSelection(0);
        Choice_Out_Transition_Type->SetSelection(0);

        CheckBox_In_Reverse->SetValue(false);
        CheckBox_In_Reverse->Enable(false);
        CheckBox_Out_Reverse->SetValue(false);
        CheckBox_Out_Reverse->Enable(false);
        Slider_Out_Adjust->Enable(false);
        Slider_In_Adjust->Enable(false);
        TextCtrl_In_Adjust->Enable(false);
        TextCtrl_Out_Adjust->Enable(false);
    }
}

wxString TimingPanel::GetTimingString()
{
    wxString s = "",ChildName,AttrName;
    // Layer Morph
    if (CheckBox_LayerMorph->IsChecked()) {
        s += "T_CHECKBOX_LayerMorph=1,";
    }
    // Layer Method
    if (Choice_LayerMethod->GetSelection() != 0) {
        s += wxString::Format("T_CHOICE_LayerMethod=%s,",
                              Choice_LayerMethod->GetString(Choice_LayerMethod->GetSelection()));
    }
    // Effect Mix
    if (Slider_EffectLayerMix->GetValue() != 0) {
        s += wxString::Format("T_SLIDER_EffectLayerMix=%d,",Slider_EffectLayerMix->GetValue());
    }
    // Fade in
    if ("" != TextCtrl_Fadein->GetValue()
        && "0" != TextCtrl_Fadein->GetValue()
        && "0.0" != TextCtrl_Fadein->GetValue()
        && "0.00" != TextCtrl_Fadein->GetValue()) {
        s+="T_TEXTCTRL_Fadein=" + TextCtrl_Fadein->GetValue() + ",";
        if (Choice_In_Transition_Type->GetSelection() > 0) {
            s+="T_CHOICE_In_Transition_Type=";
            s+=Choice_In_Transition_Type->GetStringSelection();
            s+=",";
        }
        if (CheckBox_In_Reverse->IsEnabled() && CheckBox_In_Reverse->GetValue()) {
            s+="T_CHECKBOX_In_Transition_Reverse=1,";
        }
        if (Slider_In_Adjust->IsEnabled()) {
            s+=wxString::Format("T_SLIDER_In_Transition_Adjust=%d,", Slider_In_Adjust->GetValue());
        }
    }
    // Fade Out
    if ("" != TextCtrl_Fadeout->GetValue()
        && "0" != TextCtrl_Fadeout->GetValue()
        && "0.0" != TextCtrl_Fadeout->GetValue()
        && "0.00" != TextCtrl_Fadeout->GetValue()) {
        s += "T_TEXTCTRL_Fadeout=" + TextCtrl_Fadeout->GetValue() + ",";
        if (Choice_Out_Transition_Type->GetSelection() > 0) {
            s+="T_CHOICE_Out_Transition_Type=";
            s+=Choice_Out_Transition_Type->GetStringSelection();
            s+=",";
        }
        if (CheckBox_Out_Reverse->IsEnabled() && CheckBox_Out_Reverse->GetValue()) {
            s+="T_CHECKBOX_Out_Transition_Reverse=1,";
        }
        if (Slider_Out_Adjust->IsEnabled()) {
            s+=wxString::Format("T_SLIDER_Out_Transition_Adjust=%d,", Slider_Out_Adjust->GetValue());
        }
    }
    return s;
}

PANEL_EVENT_HANDLERS(TimingPanel)

void TimingPanel::OnTransitionTypeSelect(wxCommandEvent& event)
{
    if (Choice_In_Transition_Type->GetStringSelection() == "Fade" ||
        Choice_In_Transition_Type->GetStringSelection() == "Slide Bars" ||
        Choice_In_Transition_Type->GetStringSelection() == "Blend") {
        CheckBox_In_Reverse->Disable();
    } else {
        CheckBox_In_Reverse->Enable();
    }
    if (Choice_In_Transition_Type->GetStringSelection() == "Fade" ||
        Choice_In_Transition_Type->GetStringSelection() == "From Middle" ||
        Choice_In_Transition_Type->GetStringSelection() == "Square Explode" ||
        Choice_In_Transition_Type->GetStringSelection() == "Circle Explode") {
        Slider_In_Adjust->Disable();
        TextCtrl_In_Adjust->Disable();
    } else {
        Slider_In_Adjust->Enable();
        TextCtrl_In_Adjust->Enable();
    }

    if (Choice_Out_Transition_Type->GetStringSelection() == "Fade" ||
        Choice_Out_Transition_Type->GetStringSelection() == "Slide Bars" ||
        Choice_Out_Transition_Type->GetStringSelection() == "Blend") {
        CheckBox_Out_Reverse->Disable();
    } else {
        CheckBox_Out_Reverse->Enable();
    }
    if (Choice_Out_Transition_Type->GetStringSelection() == "Fade" ||
        Choice_Out_Transition_Type->GetStringSelection() == "From Middle" ||
        Choice_Out_Transition_Type->GetStringSelection() == "Square Explode" ||
        Choice_Out_Transition_Type->GetStringSelection() == "Circle Explode") {
        Slider_Out_Adjust->Disable();
        TextCtrl_Out_Adjust->Disable();
    } else {
        Slider_Out_Adjust->Enable();
        TextCtrl_Out_Adjust->Enable();
    }
}

void TimingPanel::OnCheckBox_ResetTimingPanelClick(wxCommandEvent& event)
{
    wxConfigBase* config = wxConfigBase::Get();
    config->Write("xLightsResetTimingPanel", CheckBox_ResetTimingPanel->IsChecked());
}