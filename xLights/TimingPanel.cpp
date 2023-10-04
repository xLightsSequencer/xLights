/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

//(*InternalHeaders(TimingPanel)
#include <wx/intl.h>
#include <wx/settings.h>
#include <wx/string.h>
//*)

#include <wx/tooltip.h>
#include <wx/mimetype.h>
#include <wx/msgdlg.h>
#include <wx/config.h>

#include "TimingPanel.h"
#include "models/Model.h"
#include "effects/EffectPanelUtils.h"
#include "LayerSelectDialog.h"
#include "xLightsMain.h"
#include "UtilFunctions.h"

#include <algorithm>
#include <vector>

//(*IdInit(TimingPanel)
const long TimingPanel::ID_CHECKBOX_ResetTimingPanel = wxNewId();
const long TimingPanel::ID_STATICTEXT1 = wxNewId();
const long TimingPanel::ID_SPINCTRL_SuppressEffectUntil = wxNewId();
const long TimingPanel::ID_STATICTEXT2 = wxNewId();
const long TimingPanel::ID_SPINCTRL_FreezeEffectAtFrame = wxNewId();
const long TimingPanel::ID_CHECKBOX_LayerMorph = wxNewId();
const long TimingPanel::ID_SLIDER_EffectLayerMix = wxNewId();
const long TimingPanel::IDD_TEXTCTRL_EffectLayerMix = wxNewId();
const long TimingPanel::ID_BITMAPBUTTON_CHECKBOX_LayerMorph = wxNewId();
const long TimingPanel::ID_CHOICE_LayerMethod = wxNewId();
const long TimingPanel::ID_BUTTON_ABOUT_LAYERS = wxNewId();
const long TimingPanel::ID_BITMAPBUTTON_SLIDER_EffectLayerMix = wxNewId();
const long TimingPanel::ID_CHECKBOX_Canvas = wxNewId();
const long TimingPanel::ID_BUTTON1 = wxNewId();
const long TimingPanel::ID_CHOICE_In_Transition_Type = wxNewId();
const long TimingPanel::ID_STATICTEXT_Fadein = wxNewId();
const long TimingPanel::ID_TEXTCTRL_Fadein = wxNewId();
const long TimingPanel::ID_STATICTEXT_In_Transition_Adjust = wxNewId();
const long TimingPanel::ID_SLIDER_In_Transition_Adjust = wxNewId();
const long TimingPanel::ID_VALUECURVE_In_Transition_Adjust = wxNewId();
const long TimingPanel::IDD_TEXTCTRL_In_Transition_Adjust = wxNewId();
const long TimingPanel::ID_CHECKBOX_In_Transition_Reverse = wxNewId();
const long TimingPanel::ID_PANEL2 = wxNewId();
const long TimingPanel::ID_CHOICE_Out_Transition_Type = wxNewId();
const long TimingPanel::ID_STATICTEXT_Fadeout = wxNewId();
const long TimingPanel::ID_TEXTCTRL_Fadeout = wxNewId();
const long TimingPanel::ID_STATICTEXT_Out_Transition_Adjust = wxNewId();
const long TimingPanel::ID_SLIDER_Out_Transition_Adjust = wxNewId();
const long TimingPanel::ID_VALUECURVE_Out_Transition_Adjust = wxNewId();
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

TimingPanel::TimingPanel(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size) :
    xlEffectPanel(parent)
{
    _startLayer = -1;
    _endLayer = -1;

    //(*Initialize(TimingPanel)
    wxFlexGridSizer* FlexGridSizer10;
    wxFlexGridSizer* FlexGridSizer11;
    wxFlexGridSizer* FlexGridSizer12;
    wxFlexGridSizer* FlexGridSizer13;
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer4;
    wxFlexGridSizer* FlexGridSizer5;
    wxFlexGridSizer* FlexGridSizer6;
    wxFlexGridSizer* FlexGridSizer7;
    wxFlexGridSizer* FlexGridSizer8;
    wxFlexGridSizer* FlexGridSizer9;
    wxPanel* Panel1;
    wxPanel* Panel2;

    Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
    FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer3->AddGrowableCol(2);
    Panel_Sizer = new wxPanel(this, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    FlexGridSizer4 = new wxFlexGridSizer(0, 3, 0, 0);
    ScrolledWindowTiming = new wxScrolledWindow(Panel_Sizer, ID_SCROLLEDWINDOW1, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL, _T("ID_SCROLLEDWINDOW1"));
    FlexGridSizer5 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer5->AddGrowableCol(0);
    CheckBox_ResetTimingPanel = new wxCheckBox(ScrolledWindowTiming, ID_CHECKBOX_ResetTimingPanel, _("Reset panel when changing effects"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_ResetTimingPanel"));
    CheckBox_ResetTimingPanel->SetValue(true);
    FlexGridSizer5->Add(CheckBox_ResetTimingPanel, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer2->AddGrowableCol(1);
    StaticText1 = new wxStaticText(ScrolledWindowTiming, ID_STATICTEXT1, _("Suppress Effect Until Frame"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    SpinCtrl_SuppressEffectUntil = new BulkEditSpinCtrl(ScrolledWindowTiming, ID_SPINCTRL_SuppressEffectUntil, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 0, 999999, 0, _T("ID_SPINCTRL_SuppressEffectUntil"));
    SpinCtrl_SuppressEffectUntil->SetValue(_T("0"));
    FlexGridSizer2->Add(SpinCtrl_SuppressEffectUntil, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    StaticText3 = new wxStaticText(ScrolledWindowTiming, ID_STATICTEXT2, _("Freeze Effect At Frame"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer2->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    SpinCtrl_FreezeEffectAtFrame = new BulkEditSpinCtrl(ScrolledWindowTiming, ID_SPINCTRL_FreezeEffectAtFrame, _T("999999"), wxDefaultPosition, wxDefaultSize, 0, 0, 999999, 999999, _T("ID_SPINCTRL_FreezeEffectAtFrame"));
    SpinCtrl_FreezeEffectAtFrame->SetValue(_T("999999"));
    FlexGridSizer2->Add(SpinCtrl_FreezeEffectAtFrame, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    CheckBox_LayerMorph = new BulkEditCheckBox(ScrolledWindowTiming, ID_CHECKBOX_LayerMorph, _("Morph"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_LayerMorph"));
    CheckBox_LayerMorph->SetValue(false);
    CheckBox_LayerMorph->SetToolTip(_("Gradual cross-fade from Effect1 to Effect2"));
    FlexGridSizer2->Add(CheckBox_LayerMorph, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    Slider_EffectLayerMix = new BulkEditSlider(ScrolledWindowTiming, ID_SLIDER_EffectLayerMix, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_EffectLayerMix"));
    Slider_EffectLayerMix->SetMinSize(wxDLG_UNIT(ScrolledWindowTiming,wxSize(30,-1)));
    FlexGridSizer1->Add(Slider_EffectLayerMix, 1, wxALL|wxEXPAND, 1);
    TextCtrl_EffectLayerMix = new BulkEditTextCtrl(ScrolledWindowTiming, IDD_TEXTCTRL_EffectLayerMix, _("0"), wxDefaultPosition, wxDLG_UNIT(ScrolledWindowTiming,wxSize(20,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_EffectLayerMix"));
    TextCtrl_EffectLayerMix->SetMaxLength(3);
    FlexGridSizer1->Add(TextCtrl_EffectLayerMix, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    FlexGridSizer2->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 2);
    BitmapButton_CheckBox_LayerMorph = new xlLockButton(ScrolledWindowTiming, ID_BITMAPBUTTON_CHECKBOX_LayerMorph, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_LayerMorph"));
    BitmapButton_CheckBox_LayerMorph->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
    BitmapButton_CheckBox_LayerMorph->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
    FlexGridSizer2->Add(BitmapButton_CheckBox_LayerMorph, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    Choice_LayerMethod = new BulkEditChoice(ScrolledWindowTiming, ID_CHOICE_LayerMethod, wxDefaultPosition, wxDefaultSize, 0, 0, wxFULL_REPAINT_ON_RESIZE, wxDefaultValidator, _T("ID_CHOICE_LayerMethod"));
    FlexGridSizer2->Add(Choice_LayerMethod, 1, wxALL|wxEXPAND, 2);
    Button_About_Layers = new wxButton(ScrolledWindowTiming, ID_BUTTON_ABOUT_LAYERS, _("\?"), wxDefaultPosition, wxSize(25,23), 0, wxDefaultValidator, _T("ID_BUTTON_ABOUT_LAYERS"));
    Button_About_Layers->SetToolTip(_("About Layer Blending Types"));
    FlexGridSizer2->Add(Button_About_Layers, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 2);
    BitmapButton_EffectLayerMix = new xlLockButton(ScrolledWindowTiming, ID_BITMAPBUTTON_SLIDER_EffectLayerMix, wxNullBitmap, wxDefaultPosition, wxSize(14,14), wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_EffectLayerMix"));
    BitmapButton_EffectLayerMix->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
    BitmapButton_EffectLayerMix->SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
    FlexGridSizer2->Add(BitmapButton_EffectLayerMix, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    CheckBox_Canvas = new BulkEditCheckBox(ScrolledWindowTiming, ID_CHECKBOX_Canvas, _("Canvas"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Canvas"));
    CheckBox_Canvas->SetValue(false);
    FlexGridSizer2->Add(CheckBox_Canvas, 1, wxALL|wxEXPAND, 5);
    Button_Layers = new wxButton(ScrolledWindowTiming, ID_BUTTON1, _("Layers ..."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer2->Add(Button_Layers, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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
    Choice_In_Transition_Type->Append(_("Fold"));
    Choice_In_Transition_Type->Append(_("Dissolve"));
    Choice_In_Transition_Type->Append(_("Circular Swirl"));
    Choice_In_Transition_Type->Append(_("Bow Tie"));
    Choice_In_Transition_Type->Append(_("Zoom"));
    Choice_In_Transition_Type->Append(_("Doorway"));
    Choice_In_Transition_Type->Append(_("Blobs"));
    Choice_In_Transition_Type->Append(_("Pinwheel"));
    Choice_In_Transition_Type->Append(_("Star"));
    Choice_In_Transition_Type->Append(_("Swap"));
    Choice_In_Transition_Type->Append(_("Shatter"));
    Choice_In_Transition_Type->Append(_("Circles"));
    FlexGridSizer10->Add(Choice_In_Transition_Type, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText2 = new wxStaticText(Panel1, ID_STATICTEXT_Fadein, _("Time (s)"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fadein"));
    FlexGridSizer10->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    TextCtrl_Fadein = new BulkEditComboBox(Panel1, ID_TEXTCTRL_Fadein, wxEmptyString, wxDefaultPosition, wxSize(100,-1), 0, 0, wxCB_SORT, wxDefaultValidator, _T("ID_TEXTCTRL_Fadein"));
    TextCtrl_Fadein->SetSelection( TextCtrl_Fadein->Append(_("0.00")) );
    TextCtrl_Fadein->Append(_("0.50"));
    TextCtrl_Fadein->Append(_("1.00"));
    TextCtrl_Fadein->Append(_("1.50"));
    TextCtrl_Fadein->Append(_("2.00"));
    FlexGridSizer10->Add(TextCtrl_Fadein, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer6->Add(FlexGridSizer10, 1, wxALL|wxEXPAND, 1);
    FlexGridSizer11 = new wxFlexGridSizer(0, 4, 0, 0);
    FlexGridSizer11->AddGrowableCol(1);
    InAdjustmentText = new wxStaticText(Panel1, ID_STATICTEXT_In_Transition_Adjust, _("Adjustment"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_In_Transition_Adjust"));
    FlexGridSizer11->Add(InAdjustmentText, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    Slider_In_Adjust = new BulkEditSlider(Panel1, ID_SLIDER_In_Transition_Adjust, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_In_Transition_Adjust"));
    Slider_In_Adjust->SetMinSize(wxDLG_UNIT(Panel1,wxSize(25,-1)));
    FlexGridSizer11->Add(Slider_In_Adjust, 1, wxALL|wxEXPAND, 0);
    BitmapButton_In_Transition_Adjust = new BulkEditValueCurveButton(Panel1, ID_VALUECURVE_In_Transition_Adjust, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_In_Transition_Adjust"));
    FlexGridSizer11->Add(BitmapButton_In_Transition_Adjust, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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
    Choice_Out_Transition_Type->Append(_("Fold"));
    Choice_Out_Transition_Type->Append(_("Dissolve"));
    Choice_Out_Transition_Type->Append(_("Circular Swirl"));
    Choice_Out_Transition_Type->Append(_("Bow Tie"));
    Choice_Out_Transition_Type->Append(_("Zoom"));
    Choice_Out_Transition_Type->Append(_("Doorway"));
    Choice_Out_Transition_Type->Append(_("Blobs"));
    Choice_Out_Transition_Type->Append(_("Pinwheel"));
    Choice_Out_Transition_Type->Append(_("Star"));
    Choice_Out_Transition_Type->Append(_("Swap"));
    Choice_Out_Transition_Type->Append(_("Shatter"));
    Choice_Out_Transition_Type->Append(_("Circles"));
    FlexGridSizer12->Add(Choice_Out_Transition_Type, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText4 = new wxStaticText(Panel2, ID_STATICTEXT_Fadeout, _("Time (s)"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Fadeout"));
    FlexGridSizer12->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    TextCtrl_Fadeout = new BulkEditComboBox(Panel2, ID_TEXTCTRL_Fadeout, wxEmptyString, wxDefaultPosition, wxSize(100,-1), 0, 0, wxCB_SORT, wxDefaultValidator, _T("ID_TEXTCTRL_Fadeout"));
    TextCtrl_Fadeout->SetSelection( TextCtrl_Fadeout->Append(_("0.00")) );
    TextCtrl_Fadeout->Append(_("0.50"));
    TextCtrl_Fadeout->Append(_("1.00"));
    TextCtrl_Fadeout->Append(_("1.50"));
    TextCtrl_Fadeout->Append(_("2.00"));
    FlexGridSizer12->Add(TextCtrl_Fadeout, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer8->Add(FlexGridSizer12, 1, wxALL|wxEXPAND, 1);
    FlexGridSizer9 = new wxFlexGridSizer(0, 4, 0, 0);
    FlexGridSizer9->AddGrowableCol(1);
    OutAdjustmentText = new wxStaticText(Panel2, ID_STATICTEXT_Out_Transition_Adjust, _("Adjustment"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Out_Transition_Adjust"));
    FlexGridSizer9->Add(OutAdjustmentText, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Out_Adjust = new BulkEditSlider(Panel2, ID_SLIDER_Out_Transition_Adjust, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Out_Transition_Adjust"));
    Slider_Out_Adjust->SetMinSize(wxDLG_UNIT(Panel2,wxSize(25,-1)));
    FlexGridSizer9->Add(Slider_Out_Adjust, 1, wxALL|wxEXPAND, 0);
    BitmapButton_Out_Transition_Adjust = new BulkEditValueCurveButton(Panel2, ID_VALUECURVE_Out_Transition_Adjust, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|wxBORDER_NONE, wxDefaultValidator, _T("ID_VALUECURVE_Out_Transition_Adjust"));
    FlexGridSizer9->Add(BitmapButton_Out_Transition_Adjust, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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
    FlexGridSizer3->Fit(this);
    FlexGridSizer3->SetSizeHints(this);

    Connect(ID_CHECKBOX_ResetTimingPanel,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&TimingPanel::OnCheckBox_ResetTimingPanelClick);
    Connect(ID_BITMAPBUTTON_CHECKBOX_LayerMorph,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TimingPanel::OnLockButtonClick);
    Connect(ID_CHOICE_LayerMethod,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&TimingPanel::OnChoice_LayerMethodSelect);
    Connect(ID_BUTTON_ABOUT_LAYERS,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TimingPanel::OnButton_AboutClick);
    Connect(ID_BITMAPBUTTON_SLIDER_EffectLayerMix,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TimingPanel::OnLockButtonClick);
    Connect(ID_CHECKBOX_Canvas,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&TimingPanel::OnCheckBox_CanvasClick);
    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TimingPanel::OnButton_LayersClick);
    Connect(ID_CHOICE_In_Transition_Type,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&TimingPanel::OnTransitionTypeSelect);
    Connect(ID_TEXTCTRL_Fadein,wxEVT_COMMAND_COMBOBOX_DROPDOWN,(wxObjectEventFunction)&TimingPanel::OnTextCtrl_FadeinDropdown);
    Connect(ID_TEXTCTRL_Fadein,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&TimingPanel::OnTextCtrl_FadeinText);
    Connect(ID_VALUECURVE_In_Transition_Adjust,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TimingPanel::OnVCButtonClick);
    Connect(ID_CHOICE_Out_Transition_Type,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&TimingPanel::OnTransitionTypeSelect);
    Connect(ID_TEXTCTRL_Fadeout,wxEVT_COMMAND_COMBOBOX_DROPDOWN,(wxObjectEventFunction)&TimingPanel::OnTextCtrl_FadeoutDropdown);
    Connect(ID_TEXTCTRL_Fadeout,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&TimingPanel::OnTextCtrl_FadeoutText);
    Connect(ID_VALUECURVE_Out_Transition_Adjust,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TimingPanel::OnVCButtonClick);
    Panel_Sizer->Connect(wxEVT_SIZE,(wxObjectEventFunction)&TimingPanel::OnResize,0,this);
    //*)

    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)&TimingPanel::OnVCChanged, 0, this);
    Connect(wxID_ANY, EVT_VALIDATEWINDOW, (wxObjectEventFunction)&TimingPanel::OnValidateWindow, 0, this);

    TextCtrl_Fadein->AppendDefault(_("0.00"));
    TextCtrl_Fadein->AppendDefault(_("0.25"));
    TextCtrl_Fadein->AppendDefault(_("0.50"));
    TextCtrl_Fadein->AppendDefault(_("0.75"));
    TextCtrl_Fadein->AppendDefault(_("1.00"));
    TextCtrl_Fadein->AppendDefault(_("1.50"));
    TextCtrl_Fadein->AppendDefault(_("2.00"));

    TextCtrl_Fadeout->AppendDefault(_("0.00"));
    TextCtrl_Fadeout->AppendDefault(_("0.25"));
    TextCtrl_Fadeout->AppendDefault(_("0.50"));
    TextCtrl_Fadeout->AppendDefault(_("0.75"));
    TextCtrl_Fadeout->AppendDefault(_("1.00"));
    TextCtrl_Fadeout->AppendDefault(_("1.50"));
    TextCtrl_Fadeout->AppendDefault(_("2.00"));

    BitmapButton_In_Transition_Adjust->SetLimits(IN_TRANSITION_MIN, IN_TRANSITION_MAX);
    BitmapButton_Out_Transition_Adjust->SetLimits(OUT_TRANSITION_MIN, OUT_TRANSITION_MAX);

    // If i pull this from the map it sorts them alphabetically and I dont think that is what we want
    Choice_LayerMethod->Append(_("Normal"));
    Choice_LayerMethod->Append(_("Effect 1"));
    Choice_LayerMethod->Append(_("Effect 2"));
    Choice_LayerMethod->Append(_("1 is Mask"));
    Choice_LayerMethod->Append(_("2 is Mask"));
    Choice_LayerMethod->Append(_("1 is Unmask"));
    Choice_LayerMethod->Append(_("2 is Unmask"));
    Choice_LayerMethod->Append(_("1 is True Unmask"));
    Choice_LayerMethod->Append(_("2 is True Unmask"));
    Choice_LayerMethod->Append(_("1 reveals 2"));
    Choice_LayerMethod->Append(_("2 reveals 1"));
    Choice_LayerMethod->Append(_("Shadow 1 on 2"));
    Choice_LayerMethod->Append(_("Shadow 2 on 1"));
    Choice_LayerMethod->Append(_("Layered"));
    Choice_LayerMethod->Append(_("Average"));
    Choice_LayerMethod->Append(_("Bottom-Top"));
    Choice_LayerMethod->Append(_("Left-Right"));
    Choice_LayerMethod->Append(_("Highlight"));
    Choice_LayerMethod->Append(_("Highlight Vibrant"));
    Choice_LayerMethod->Append(_("Additive"));
    Choice_LayerMethod->Append(_("Subtractive"));
    Choice_LayerMethod->Append(_("Brightness"));
    Choice_LayerMethod->Append(_("Max"));
    Choice_LayerMethod->Append(_("Min"));

    Choice_LayerMethod->SetStringSelection("Normal");

    Choice_LayerMethod->SetToolTip(_("Layering defines how Effect 1 and Effect 2 will be mixed together.\nHere are the Choices\n"
                                     "* Normal: This is the same as 1 reveals 2.\n"
                                     "* Effect 1: Shows only Effect 1. Slide the slider to the right to blend in some Effect 2. \n"
                                     "* Effect 2: Shows only Effect 2. Slide the slider to the right to blend in some Effect 1.\n"
                                     "* 1 is Mask: (Shadow) Effect 1 will cast a shadow onto Effect 2 for every Effect 1 pixel that has a non-black value.\n"
                                     "* 2 is Mask: (Shadow) Effect 2 will cast a shadow onto Effect 1 for every Effect 2 pixel that has a non-black value.\n"
                                     "* 1 is Unmask: Unmask like but colours are revealed with no fade. Black becomes white.\n"
                                     "* 2 is Unmask: Unmask like but colours are revealed with no fade. Black becomes white.\n"
                                     "* 1 is True Unmask: (Mask) Only allow Effect 2 to show through when Effect 1 has a non-black pixel.\n"
                                     "* 2 is True Unmask: (Mask) Only allow Effect 1 to show through when Effect 2 has a non-black pixel.\n"
                                     "* Shadow 1 on 2: Take brightness and Saturation from 1, use hue from 2\n"
                                     "* Shadow 2 on 1: Take brightness and Saturation from 2, use hue from 1\n"
                                     "* 1 reveals 2: (Superimpose) Effect 1 reveals Effect 2\n"
                                     "* 2 reveals 1: (Superimpose) Effect 2 reveals Effect 1\n"
                                     "* Layered: Effect 1 only shows in black regions of Effect 2.\n"
                                     "* Average: Take value of Effect  and Add it to Value from Effect 2. Average the sum\n"
                                     "* Bottom-top: Effect 1 is put on bottom of model, Effect 2 is put on top in a split screen display\n"
                                     "* Left-Right: Effect goes 1 goes on the left side, Effect 2 on the right. Split screen goes down middle of model.\n"
                                     "* Highlight - Creates highlights by showcasing Effect 1's color where available, while using Effect 2's color where Effect 1 lacks color\n"
                                     "* Highlight Vibrant - Intensifies Effect 2's color where Effect 1 has content, without affectinng black or dark areas\n"
                                     "* Additive - Take value of Effect 1 and Add it to Value from Effect 2.\n"
                                     "* Subtractive - Take value of Effect 1 and Subtract it from the Value from Effect 2.\n"
                                     "* Brightness - Multiply each colour channel value of both layers and divide by 255.\n"
                                     "* Max - Take the maximum value for each channel from both effects\n"
                                     "* Min - Take the minimum value for each channel from both effects\n"
                                     "* Canvas - Blend the selected layers into this layer"));

    SetName("Timing");

    wxConfigBase* config = wxConfigBase::Get();
    bool reset;
    config->Read("xLightsResetTimingPanel", &reset, true);
    CheckBox_ResetTimingPanel->SetValue(reset);

    ValidateWindow();
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

void TimingPanel::SetDefaultControls(const Model* model, bool optionbased)
{
    if (!optionbased || CheckBox_ResetTimingPanel->GetValue()) {
        _layersSelected = "";
        CheckBox_LayerMorph->SetValue(false);
        Choice_LayerMethod->SetStringSelection("Normal");
        CheckBox_Canvas->SetValue(false);
        TextCtrl_EffectLayerMix->SetValue("0");
        TextCtrl_Fadein->SetValue("0.00");
        TextCtrl_Fadeout->SetValue("0.00");
        Slider_In_Adjust->SetValue(50);
        Slider_Out_Adjust->SetValue(50);
        Choice_In_Transition_Type->SetSelection(0);
        Choice_Out_Transition_Type->SetSelection(0);
        SpinCtrl_FreezeEffectAtFrame->SetValue(999999);
        SpinCtrl_SuppressEffectUntil->SetValue(0);
        CheckBox_In_Reverse->SetValue(false);
        CheckBox_Out_Reverse->SetValue(false);

        BitmapButton_In_Transition_Adjust->GetValue()->SetDefault(0.0f, 100.0f);
        BitmapButton_In_Transition_Adjust->UpdateState();

        BitmapButton_Out_Transition_Adjust->GetValue()->SetDefault(0.0f, 100.0f);
        BitmapButton_Out_Transition_Adjust->UpdateState();
    }

    ValidateWindow();
}

wxString TimingPanel::GetTimingString()
{
    wxString s = "", ChildName, AttrName;
    // Layer Morph
    if (CheckBox_LayerMorph->IsChecked()) {
        s += "T_CHECKBOX_LayerMorph=1,";
    }
    // Layer Method
    if (Choice_LayerMethod->GetStringSelection() != "Normal") {
        s += wxString::Format("T_CHOICE_LayerMethod=%s,",
                              Choice_LayerMethod->GetString(Choice_LayerMethod->GetSelection()));
    }

    if (CheckBox_Canvas->GetValue()) {
        s += "T_CHECKBOX_Canvas=1,";
        if (_layersSelected.length() > 0) {
            s += "T_LayersSelected=";
            s += _layersSelected;
            s += ",";
        }
    }

    if (SpinCtrl_FreezeEffectAtFrame->GetValue() != 999999) {
        s += wxString::Format("T_SPINCTRL_FreezeEffectAtFrame=%d,", SpinCtrl_FreezeEffectAtFrame->GetValue());
    }

    if (SpinCtrl_SuppressEffectUntil->GetValue() != 0) {
        s += wxString::Format("T_SPINCTRL_SuppressEffectUntil=%d,", SpinCtrl_SuppressEffectUntil->GetValue());
    }

    // Effect Mix
    if (Slider_EffectLayerMix->GetValue() != 0) {
        s += wxString::Format("T_SLIDER_EffectLayerMix=%d,", Slider_EffectLayerMix->GetValue());
    }
    // Fade in
    if ("" != TextCtrl_Fadein->GetValue() && "0" != TextCtrl_Fadein->GetValue() && "0.0" != TextCtrl_Fadein->GetValue() && "0.00" != TextCtrl_Fadein->GetValue()) {
        s += "T_TEXTCTRL_Fadein=" + TextCtrl_Fadein->GetValue() + ",";
        if (Choice_In_Transition_Type->GetSelection() > 0) {
            s += "T_CHOICE_In_Transition_Type=";
            s += Choice_In_Transition_Type->GetStringSelection();
            s += ",";
        }
        if (CheckBox_In_Reverse->IsEnabled() && CheckBox_In_Reverse->GetValue()) {
            s += "T_CHECKBOX_In_Transition_Reverse=1,";
        }

        ValueCurve* pVC = BitmapButton_In_Transition_Adjust->GetValue();
        if (pVC->IsActive()) {
            std::string vc(BitmapButton_In_Transition_Adjust->GetValue()->Serialise());
            s += wxString::Format("T_VALUECURVE_In_Transition_Adjust=%s,", wxString(vc.c_str()));
        } else if (Slider_In_Adjust->IsEnabled()) {
            s += wxString::Format("T_SLIDER_In_Transition_Adjust=%d,", Slider_In_Adjust->GetValue());
        }
    }
    // Fade Out
    if ("" != TextCtrl_Fadeout->GetValue() && "0" != TextCtrl_Fadeout->GetValue() && "0.0" != TextCtrl_Fadeout->GetValue() && "0.00" != TextCtrl_Fadeout->GetValue()) {
        s += "T_TEXTCTRL_Fadeout=" + TextCtrl_Fadeout->GetValue() + ",";
        if (Choice_Out_Transition_Type->GetSelection() > 0) {
            s += "T_CHOICE_Out_Transition_Type=";
            s += Choice_Out_Transition_Type->GetStringSelection();
            s += ",";
        }
        if (CheckBox_Out_Reverse->IsEnabled() && CheckBox_Out_Reverse->GetValue()) {
            s += "T_CHECKBOX_Out_Transition_Reverse=1,";
        }

        ValueCurve* pVC = BitmapButton_Out_Transition_Adjust->GetValue();
        if (pVC->IsActive()) {
            std::string vc(BitmapButton_Out_Transition_Adjust->GetValue()->Serialise());
            s += wxString::Format("T_VALUECURVE_Out_Transition_Adjust=%s,", wxString(vc.c_str()));
        } else if (Slider_Out_Adjust->IsEnabled()) {
            s += wxString::Format("T_SLIDER_Out_Transition_Adjust=%d,", Slider_Out_Adjust->GetValue());
        }
    }
    return s;
}

namespace
{
    const std::vector<wxString> transitions_noReverse = {
        "Fade",
        "Slide Bars",
        "Blend",
        "Dissolve",
        "Circular Swirl",
        "Zoom",
        "Doorway",
        "Blobs",
        "Pinwheel",
        "Swap",
        "Shatter",
        "Circles"
    };

    const std::vector<wxString> transitions_noAdjust = {
        "Fade",
        "Square Explode",
        "Circle Explode",
        "Fold",
        "Dissolve",
        "Circular Swirl",
        "Zoom",
        "Doorway",
        "Swap",
        "Shatter"
    };
}

void TimingPanel::OnTransitionTypeSelect(wxCommandEvent& event)
{
	ValidateWindow();
}

void TimingPanel::OnCheckBox_ResetTimingPanelClick(wxCommandEvent& event)
{
    wxConfigBase* config = wxConfigBase::Get();
    config->Write("xLightsResetTimingPanel", CheckBox_ResetTimingPanel->IsChecked());
}

void TimingPanel::OnButton_LayersClick(wxCommandEvent& event)
{
    wxASSERT(_startLayer <= _endLayer);
    LayerSelectDialog dlg(this, _startLayer, _endLayer, _layersSelected, _layerWithEffect);
    OptimiseDialogPosition(&dlg);

    if (dlg.ShowModal() == wxID_OK) {
        _layersSelected = dlg.GetSelectedLayers();
        FireChangeEvent();
    }
}

void TimingPanel::OnChoice_LayerMethodSelect(wxCommandEvent& event)
{
    if (Choice_LayerMethod->GetStringSelection() == "Canvas") {
        // need to post event so it can call us back with the right number of layers
        wxCommandEvent eventUpdateEffect(EVT_UPDATE_EFFECT);
        wxPostEvent(GetParent(), eventUpdateEffect);
    }

    ValidateWindow();
}

void TimingPanel::ValidateWindow()
{
    if (CheckBox_Canvas->GetValue() && _startLayer != -1) {
        Button_Layers->Enable(true);
    } else {
        _layersSelected = "";
        Button_Layers->Enable(false);
    }

	bool inEnable = (wxAtof(TextCtrl_Fadein->GetValue()) != 0.0);
	bool outEnable = (wxAtof(TextCtrl_Fadeout->GetValue()) != 0.0);
	auto inTransitionType = Choice_In_Transition_Type->GetStringSelection();

    if (!inEnable || std::find(transitions_noReverse.cbegin(), transitions_noReverse.cend(), inTransitionType) != transitions_noReverse.cend()) {
		CheckBox_In_Reverse->Disable();
    } else {
		CheckBox_In_Reverse->Enable();
    }

	if (!inEnable || std::find(transitions_noAdjust.cbegin(), transitions_noAdjust.cend(), inTransitionType) != transitions_noAdjust.cend()) {
		Slider_In_Adjust->Disable();
		BitmapButton_In_Transition_Adjust->Disable();
		TextCtrl_In_Adjust->Disable();
	} else {
		Slider_In_Adjust->Enable();
		BitmapButton_In_Transition_Adjust->Enable();
		TextCtrl_In_Adjust->Enable();
	}

	auto outTransitionType = Choice_Out_Transition_Type->GetStringSelection();

	if (!outEnable || std::find(transitions_noReverse.cbegin(), transitions_noReverse.cend(), outTransitionType) != transitions_noReverse.cend())
		CheckBox_Out_Reverse->Disable();
	else
		CheckBox_Out_Reverse->Enable();

	if (!outEnable || std::find(transitions_noAdjust.cbegin(), transitions_noAdjust.cend(), outTransitionType) != transitions_noAdjust.cend()) {
		Slider_Out_Adjust->Disable();
		BitmapButton_Out_Transition_Adjust->Disable();
		TextCtrl_Out_Adjust->Disable();
	} else {
		Slider_Out_Adjust->Enable();
		BitmapButton_Out_Transition_Adjust->Enable();
		TextCtrl_Out_Adjust->Enable();
	}
}

void TimingPanel::OnCheckBox_CanvasClick(wxCommandEvent& event)
{
    ValidateWindow();
}

void TimingPanel::OnButton_AboutClick(wxCommandEvent& event)
{
	ViewTempFile(Choice_LayerMethod->GetToolTip()->GetTip(), "layerblendhelp");
}

void TimingPanel::OnTextCtrl_FadeinText(wxCommandEvent& event)
{
    ValidateWindow();
}

void TimingPanel::OnTextCtrl_FadeoutText(wxCommandEvent& event)
{
    ValidateWindow();
}

void TimingPanel::OnTextCtrl_FadeinDropdown(wxCommandEvent& event)
{
    TextCtrl_Fadein->PopulateComboBox();
}

void TimingPanel::OnTextCtrl_FadeoutDropdown(wxCommandEvent& event)
{
    TextCtrl_Fadeout->PopulateComboBox();
}
