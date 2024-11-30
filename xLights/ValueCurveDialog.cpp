/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

//(*InternalHeaders(ValueCurveDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/dcbuffer.h>
#include <wx/file.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/settings.h>
#include <wx/stdpaths.h>
#include <wx/textdlg.h>
#include <wx/valnum.h>

#include "ExternalHooks.h"
#include "UtilFunctions.h"
#include "ValueCurve.h"
#include "ValueCurveDialog.h"
#include "xLightsApp.h"
#include "xLightsMain.h"
#include "xLightsVersion.h"
#include "sequencer/MainSequencer.h"
#include "sequencer/SequenceElements.h"

#include <log4cpp/Category.hh>

BEGIN_EVENT_TABLE(ValueCurvePanel, wxWindow)
EVT_MOTION(ValueCurvePanel::mouseMoved)
EVT_LEFT_DOWN(ValueCurvePanel::mouseLeftDown)
EVT_LEFT_UP(ValueCurvePanel::mouseLeftUp)
EVT_ENTER_WINDOW(ValueCurvePanel::mouseEnter)
EVT_LEAVE_WINDOW(ValueCurvePanel::mouseLeave)
EVT_LEFT_DCLICK(ValueCurvePanel::mouseLeftDClick)
EVT_PAINT(ValueCurvePanel::Paint)
EVT_MOUSE_CAPTURE_LOST(ValueCurvePanel::mouseCaptureLost)
END_EVENT_TABLE()

ValueCurvePanel::ValueCurvePanel(wxWindow* parent, Element* timingElement, int start, int end, wxWindowID id, const wxPoint& pos, const wxSize& size, long style) :
    wxWindow(parent, id, pos, size, style, "ID_VCP"), xlCustomControl() {
    Connect(wxEVT_LEFT_DOWN, (wxObjectEventFunction)&ValueCurvePanel::mouseLeftDown, 0, this);
    Connect(wxEVT_LEFT_UP, (wxObjectEventFunction)&ValueCurvePanel::mouseLeftUp, 0, this);
    Connect(wxEVT_ENTER_WINDOW, (wxObjectEventFunction)&ValueCurvePanel::mouseEnter, 0, this);
    Connect(wxEVT_LEAVE_WINDOW, (wxObjectEventFunction)&ValueCurvePanel::mouseLeave, 0, this);
    Connect(wxEVT_MOTION, (wxObjectEventFunction)&ValueCurvePanel::mouseMoved, 0, this);
    Connect(wxEVT_PAINT, (wxObjectEventFunction)&ValueCurvePanel::Paint, 0, this);
    Connect(wxEVT_MOUSE_CAPTURE_LOST, (wxObjectEventFunction)&ValueCurvePanel::mouseCaptureLost, 0, this);
    Connect(wxEVT_LEFT_DCLICK, (wxObjectEventFunction)&ValueCurvePanel::mouseLeftDClick, 0, this);
    // ReSharper disable CppVirtualFunctionCallInsideCtor
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    // ReSharper restore CppVirtualFunctionCallInsideCtor
    _grabbedPoint = -1;
    _start = start;
    _end = end;
    _timingElement = timingElement;
}

#define X_VC_MARGIN 5.0
#define Y_VC_MARGIN 3.0

void ValueCurvePanel::Convert(float& x, float& y, wxMouseEvent& event) {
    wxSize size = GetSize();
    float startX = X_VC_MARGIN;
    float startY = Y_VC_MARGIN;
    float bw = size.GetWidth() - 2 * X_VC_MARGIN;
    float bh = size.GetHeight() - 2 * Y_VC_MARGIN;

    if (event.GetX() < X_VC_MARGIN) {
        x = 0;
    } else if (event.GetX() > X_VC_MARGIN + bw) {
        x = 1.0;
    } else {
        x = (event.GetX() - startX) / bw;
    }
    if (event.GetY() < Y_VC_MARGIN) {
        y = 1.0;
    } else if (event.GetY() > Y_VC_MARGIN + bh) {
        y = 0.0;
    } else {
        y = 1.0 - (event.GetY() - startY) / bh;
    }
}

//(*IdInit(ValueCurveDialog)
const long ValueCurveDialog::ID_STATICTEXT3 = wxNewId();
const long ValueCurveDialog::ID_STATICTEXT7 = wxNewId();
const long ValueCurveDialog::ID_STATICTEXT4 = wxNewId();
const long ValueCurveDialog::ID_CHOICE1 = wxNewId();
const long ValueCurveDialog::ID_STATICTEXT1 = wxNewId();
const long ValueCurveDialog::ID_SLIDER_Parameter1 = wxNewId();
const long ValueCurveDialog::IDD_TEXTCTRL_Parameter1 = wxNewId();
const long ValueCurveDialog::ID_STATICTEXT2 = wxNewId();
const long ValueCurveDialog::ID_SLIDER_Parameter2 = wxNewId();
const long ValueCurveDialog::IDD_TEXTCTRL_Parameter2 = wxNewId();
const long ValueCurveDialog::ID_STATICTEXT5 = wxNewId();
const long ValueCurveDialog::ID_SLIDER_Parameter3 = wxNewId();
const long ValueCurveDialog::IDD_TEXTCTRL_Parameter3 = wxNewId();
const long ValueCurveDialog::ID_STATICTEXT6 = wxNewId();
const long ValueCurveDialog::ID_SLIDER_Parameter4 = wxNewId();
const long ValueCurveDialog::IDD_TEXTCTRL_Parameter4 = wxNewId();
const long ValueCurveDialog::ID_CHECKBOX_WrapValues = wxNewId();
const long ValueCurveDialog::ID_BUTTON5 = wxNewId();
const long ValueCurveDialog::ID_BUTTON6 = wxNewId();
const long ValueCurveDialog::ID_STATICTEXT8 = wxNewId();
const long ValueCurveDialog::ID_SLIDER1 = wxNewId();
const long ValueCurveDialog::ID_TEXTCTRL1 = wxNewId();
const long ValueCurveDialog::ID_STATICTEXT9 = wxNewId();
const long ValueCurveDialog::ID_CHOICE2 = wxNewId();
const long ValueCurveDialog::ID_STATICTEXT10 = wxNewId();
const long ValueCurveDialog::ID_TEXTCTRL2 = wxNewId();
const long ValueCurveDialog::ID_CHECKBOX1 = wxNewId();
const long ValueCurveDialog::ID_BUTTON3 = wxNewId();
const long ValueCurveDialog::ID_BUTTON4 = wxNewId();
const long ValueCurveDialog::ID_BUTTON1 = wxNewId();
const long ValueCurveDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(ValueCurveDialog, wxDialog)
//(*EventTable(ValueCurveDialog)
//*)
END_EVENT_TABLE()

ValueCurveDialog::ValueCurveDialog(wxWindow* parent, ValueCurve* vc, bool slideridd, SequenceElements* sequenceElements, wxWindowID id, const wxPoint& pos, const wxSize& size) {
    _vc = vc;
    _slideridd = slideridd;

    //(*Initialize(ValueCurveDialog)
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer4;
    wxFlexGridSizer* FlexGridSizer5;
    wxFlexGridSizer* FlexGridSizer6;
    wxFlexGridSizer* FlexGridSizer7;
    wxFlexGridSizer* FlexGridSizer8;
    wxFlexGridSizer* FlexGridSizer9;

    Create(parent, id, _("Value Curve"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxCLOSE_BOX, _T("id"));
    SetClientSize(wxDefaultSize);
    Move(wxDefaultPosition);
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer1->AddGrowableRow(0);
    FlexGridSizer6 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer6->AddGrowableCol(1);
    FlexGridSizer6->AddGrowableRow(0);
    FlexGridSizer5 = new wxFlexGridSizer(3, 1, 0, 0);
    FlexGridSizer5->AddGrowableCol(0);
    FlexGridSizer5->AddGrowableRow(1);
    StaticText_TopValue = new wxStaticText(this, ID_STATICTEXT3, _("100"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT, _T("ID_STATICTEXT3"));
    FlexGridSizer5->Add(StaticText_TopValue, 1, wxALL|wxEXPAND, 2);
    StaticText1 = new wxStaticText(this, ID_STATICTEXT7, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
    FlexGridSizer5->Add(StaticText1, 1, wxALL|wxEXPAND, 5);
    StaticText_BottomValue = new wxStaticText(this, ID_STATICTEXT4, _("0"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT, _T("ID_STATICTEXT4"));
    FlexGridSizer5->Add(StaticText_BottomValue, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer6->Add(FlexGridSizer5, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer4->AddGrowableCol(0);
    FlexGridSizer4->AddGrowableRow(0);
    FlexGridSizer6->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer1->Add(FlexGridSizer6, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer2->AddGrowableCol(1);
    FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Choice1 = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
    Choice1->SetSelection( Choice1->Append(_("Flat")) );
    Choice1->Append(_("Ramp"));
    Choice1->Append(_("Ramp Up/Down"));
    Choice1->Append(_("Ramp Up/Down Hold"));
    Choice1->Append(_("Saw Tooth"));
    Choice1->Append(_("Parabolic Down"));
    Choice1->Append(_("Parabolic Up"));
    Choice1->Append(_("Logarithmic Up"));
    Choice1->Append(_("Logarithmic Down"));
    Choice1->Append(_("Exponential Up"));
    Choice1->Append(_("Exponential Down"));
    Choice1->Append(_("Sine"));
    Choice1->Append(_("Abs Sine"));
    Choice1->Append(_("Decaying Sine"));
    Choice1->Append(_("Square"));
    Choice1->Append(_("Random"));
    Choice1->Append(_("Music"));
    Choice1->Append(_("Inverted Music"));
    Choice1->Append(_("Music Trigger Fade"));
    Choice1->Append(_("Timing Track Toggle"));
    Choice1->Append(_("Timing Track Fade Fixed"));
    Choice1->Append(_("Timing Track Fade Proportional"));
    Choice1->Append(_("Custom"));
    FlexGridSizer2->Add(Choice1, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_P1 = new wxStaticText(this, ID_STATICTEXT1, _("XXXXXXXXXXXXXXX"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer2->Add(StaticText_P1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Parameter1 = new wxSlider(this, ID_SLIDER_Parameter1, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Parameter1"));
    FlexGridSizer2->Add(Slider_Parameter1, 1, wxALL|wxEXPAND, 2);
    TextCtrl_Parameter1 = new wxTextCtrl(this, IDD_TEXTCTRL_Parameter1, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(30,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Parameter1"));
    TextCtrl_Parameter1->SetMaxLength(6);
    FlexGridSizer2->Add(TextCtrl_Parameter1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText_P2 = new wxStaticText(this, ID_STATICTEXT2, _("Parameter 2"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer2->Add(StaticText_P2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Parameter2 = new wxSlider(this, ID_SLIDER_Parameter2, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Parameter2"));
    FlexGridSizer2->Add(Slider_Parameter2, 1, wxALL|wxEXPAND, 2);
    TextCtrl_Parameter2 = new wxTextCtrl(this, IDD_TEXTCTRL_Parameter2, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(30,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Parameter2"));
    TextCtrl_Parameter2->SetMaxLength(6);
    FlexGridSizer2->Add(TextCtrl_Parameter2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText_P3 = new wxStaticText(this, ID_STATICTEXT5, _("Parameter 3"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
    FlexGridSizer2->Add(StaticText_P3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Parameter3 = new wxSlider(this, ID_SLIDER_Parameter3, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Parameter3"));
    FlexGridSizer2->Add(Slider_Parameter3, 1, wxALL|wxEXPAND, 2);
    TextCtrl_Parameter3 = new wxTextCtrl(this, IDD_TEXTCTRL_Parameter3, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(30,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Parameter3"));
    TextCtrl_Parameter3->SetMaxLength(6);
    FlexGridSizer2->Add(TextCtrl_Parameter3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText_P4 = new wxStaticText(this, ID_STATICTEXT6, _("Label"), wxDefaultPosition, wxSize(121,16), 0, _T("ID_STATICTEXT6"));
    FlexGridSizer2->Add(StaticText_P4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Parameter4 = new wxSlider(this, ID_SLIDER_Parameter4, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Parameter4"));
    FlexGridSizer2->Add(Slider_Parameter4, 1, wxALL|wxEXPAND, 2);
    TextCtrl_Parameter4 = new wxTextCtrl(this, IDD_TEXTCTRL_Parameter4, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(30,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Parameter4"));
    TextCtrl_Parameter4->SetMaxLength(6);
    FlexGridSizer2->Add(TextCtrl_Parameter4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_WrapValues = new wxCheckBox(this, ID_CHECKBOX_WrapValues, _("Wrap Values"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_WrapValues"));
    CheckBox_WrapValues->SetValue(false);
    FlexGridSizer2->Add(CheckBox_WrapValues, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer9 = new wxFlexGridSizer(0, 3, 0, 0);
    Button_Reverse = new wxButton(this, ID_BUTTON5, _("Reverse"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON5"));
    FlexGridSizer9->Add(Button_Reverse, 1, wxALL|wxEXPAND, 2);
    Button_Flip = new wxButton(this, ID_BUTTON6, _("Flip"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON6"));
    FlexGridSizer9->Add(Button_Flip, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer2->Add(FlexGridSizer9, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText2 = new wxStaticText(this, ID_STATICTEXT8, _("Time Offset:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
    FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_TimeOffset = new wxSlider(this, ID_SLIDER1, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER1"));
    FlexGridSizer2->Add(Slider_TimeOffset, 1, wxALL|wxEXPAND, 5);
    TextCtrl_TimeOffset = new wxTextCtrl(this, ID_TEXTCTRL1, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
    TextCtrl_TimeOffset->SetMaxLength(5);
    FlexGridSizer2->Add(TextCtrl_TimeOffset, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText3 = new wxStaticText(this, ID_STATICTEXT9, _("Timing Track:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
    FlexGridSizer2->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Choice_TimingTrack = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
    FlexGridSizer2->Add(Choice_TimingTrack, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText4 = new wxStaticText(this, ID_STATICTEXT10, _("Filter Label:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT10"));
    FlexGridSizer2->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_FilterLabel = new wxTextCtrl(this, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
    FlexGridSizer2->Add(TextCtrl_FilterLabel, 1, wxALL|wxEXPAND, 5);
    CheckBox_FilterLabelRegex = new wxCheckBox(this, ID_CHECKBOX1, _("Regex"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
    CheckBox_FilterLabelRegex->SetValue(false);
    FlexGridSizer2->Add(CheckBox_FilterLabelRegex, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer7 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer7->AddGrowableCol(0);
    FlexGridSizer7->AddGrowableRow(0);
    PresetSizer = new wxFlexGridSizer(0, 5, 0, 0);
    FlexGridSizer7->Add(PresetSizer, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer8 = new wxFlexGridSizer(0, 1, 0, 0);
    ButtonLoad = new wxButton(this, ID_BUTTON3, _("Load"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    FlexGridSizer8->Add(ButtonLoad, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonExport = new wxButton(this, ID_BUTTON4, _("Export"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
    FlexGridSizer8->Add(ButtonExport, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer7->Add(FlexGridSizer8, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer1->Add(FlexGridSizer7, 1, wxALL|wxEXPAND, 1);
    FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
    Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer3->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    FlexGridSizer3->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&ValueCurveDialog::OnChoice1Select);
    Connect(ID_SLIDER_Parameter1,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&ValueCurveDialog::OnSlider_Parameter1CmdSliderUpdated);
    Connect(IDD_TEXTCTRL_Parameter1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ValueCurveDialog::OnTextCtrl_Parameter1Text);
    Connect(ID_SLIDER_Parameter2,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&ValueCurveDialog::OnSlider_Parameter2CmdSliderUpdated);
    Connect(IDD_TEXTCTRL_Parameter2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ValueCurveDialog::OnTextCtrl_Parameter2Text);
    Connect(ID_SLIDER_Parameter3,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&ValueCurveDialog::OnSlider_Parameter3CmdSliderUpdated);
    Connect(IDD_TEXTCTRL_Parameter3,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ValueCurveDialog::OnTextCtrl_Parameter3Text);
    Connect(ID_SLIDER_Parameter4,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&ValueCurveDialog::OnSlider_Parameter4CmdSliderUpdated);
    Connect(IDD_TEXTCTRL_Parameter4,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ValueCurveDialog::OnTextCtrl_Parameter4Text);
    Connect(ID_CHECKBOX_WrapValues,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ValueCurveDialog::OnCheckBox_WrapValuesClick);
    Connect(ID_BUTTON5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ValueCurveDialog::OnButton_ReverseClick);
    Connect(ID_BUTTON6,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ValueCurveDialog::OnButton_FlipClick);
    Connect(ID_SLIDER1,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&ValueCurveDialog::OnSlider_TimeOffsetCmdSliderUpdated);
    Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ValueCurveDialog::OnTextCtrl_TimeOffsetText);
    Connect(ID_CHOICE2,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&ValueCurveDialog::OnChoice_TimingTrackSelect);
    Connect(ID_TEXTCTRL2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ValueCurveDialog::OnTextCtrl_FilterLabelText);
    Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ValueCurveDialog::OnCheckBox_FilterLabelRegexClick);
    Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ValueCurveDialog::OnButtonLoadClick);
    Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ValueCurveDialog::OnButtonExportClick);
    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ValueCurveDialog::OnButton_OkClick);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ValueCurveDialog::OnButton_CancelClick);
    Connect(wxEVT_SIZE,(wxObjectEventFunction)&ValueCurveDialog::OnResize);
    //*)

    Connect(wxID_ANY, wxEVT_CHAR_HOOK, wxKeyEventHandler(ValueCurveDialog::OnChar), (wxObject*)nullptr, this);

    TextCtrl_FilterLabel->SetToolTip("Only trigger on timing events which contain this token in their text. Blank matches all. Multiple tokens can be ; separated in non-regex mode.");

    Button_Ok->SetDefault();
    SetEscapeId(ID_BUTTON2);

    int start = -1;
    int end = -1;
    Effect* eff = xLightsApp::GetFrame()->GetMainSequencer()->GetSelectedEffect();
    if (eff != nullptr) {
        start = eff->GetStartTimeMS();
        end = eff->GetEndTimeMS();
    }

    for (int i = 0; i < sequenceElements->GetNumberOfTimingElements(); i++) {
        auto te = sequenceElements->GetTimingElement(i);
        Choice_TimingTrack->Append(te->GetName());
    }

    Element* timingElement = xLightsApp::GetFrame()->GetMainSequencer()->PanelEffectGrid->GetActiveTimingElement();

    _vcp = new ValueCurvePanel(this, timingElement, start, end, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    _vcp->SetMinSize(wxSize(200, 100));
    _vcp->SetValue(_vc);
    _vcp->SetType(_vc->GetType());
    _vcp->SetTimeOffset(_vc->GetTimeOffset());
    FlexGridSizer4->Add(_vcp, 1, wxALL | wxEXPAND, 2);
    // ReSharper disable once CppVirtualFunctionCallInsideCtor
    Layout();
    // ReSharper disable once CppVirtualFunctionCallInsideCtor
    Fit();

    _backup = *_vc;

    wxString sMin; // = "";
    wxString sMax; // = "";

    switch (_vc->GetDivisor()) {
    case 360:
        sMin = wxString::Format("%.2f", _vc->GetMin() / 360.0);
        sMax = wxString::Format("%.2f", _vc->GetMax() / 360.0);
        break;
    case 100:
        sMin = wxString::Format("%.2f", _vc->GetMin() / 100.0);
        sMax = wxString::Format("%.2f", _vc->GetMax() / 100.0);
        break;
    case 10:
        sMin = wxString::Format("%.1f", _vc->GetMin() / 10.0);
        sMax = wxString::Format("%.1f", _vc->GetMax() / 10.0);
        break;
    default:
        sMin = wxString::Format("%i", (int)_vc->GetMin());
        sMax = wxString::Format("%i", (int)_vc->GetMax());
        break;
    }

    SetSliderMinMax();

    StaticText_BottomValue->SetLabel(sMin);
    StaticText_TopValue->SetLabel(sMax);

    CheckBox_WrapValues->SetValue(_vc->GetWrap());
    Choice1->SetStringSelection(wxString(_vc->GetType().c_str()));
    Choice_TimingTrack->SetStringSelection(_vc->GetTimingTrack());
    Slider_TimeOffset->SetValue(_vc->GetTimeOffset());
    SetParameter(1, _vc->GetParameter1());
    SetParameter(2, _vc->GetParameter2());
    SetParameter(3, _vc->GetParameter3());
    SetParameter(4, _vc->GetParameter4());
    SetTextCtrlsFromSliders();
    Choice1->SetFocus();
    TextCtrl_FilterLabel->SetValue(_vc->GetFilterLabelText());
    CheckBox_FilterLabelRegex->SetValue(_vc->IsFilterLabelRegex());

    PopulatePresets();

    Layout();
    Fit();

    wxSize sz = GetSize();

    if (sz.x < 500) {
        sz.x = 800;
    }
    if (sz.y < 400) {
        sz.y = 600;
    }
    SetSize(sz);

    ValidateWindow();
}

ValueCurveDialog::~ValueCurveDialog() {
    //(*Destroy(ValueCurveDialog)
    //*)
}

void ValueCurveDialog::OnButton_OkClick(wxCommandEvent& event) {
    _vc->SetRealValue();
    EndDialog(wxOK);
}

void ValueCurveDialog::OnButton_CancelClick(wxCommandEvent& event) {
    *_vc = _backup;
    EndDialog(wxCANCEL);
}

void ValueCurveDialog::SetParameter100(int p, float v) {
    float low, high;
    _vc->GetRangeParm(p, Choice1->GetStringSelection().ToStdString(), low, high);

    if (low == MINVOID) {
        low = _vc->GetMin();
    }
    if (high == MAXVOID) {
        high = _vc->GetMax();
    }

    float vv = (high - low) * v / 100.0 + low;
    SetParameter(p, vv);
}

void ValueCurveDialog::SetParameter(int p, float v) {
    switch (p) {
    case 1:
        _vc->SetParameter1(v);
        Slider_Parameter1->SetValue(std::round(v));
        break;
    case 2:
        _vc->SetParameter2(v);
        Slider_Parameter2->SetValue(std::round(v));
        break;
    case 3:
        _vc->SetParameter3(v);
        Slider_Parameter3->SetValue(std::round(v));
        break;
    case 4:
        _vc->SetParameter4(v);
        Slider_Parameter4->SetValue(std::round(v));
        break;
    default:
        break;
    }
    SetTextCtrlsFromSliders();
}

void ValueCurveDialog::SetSliderMinMax() {
    float min, max;

    ValueCurve::GetRangeParm1(_vc->GetType(), min, max);
    if (min == MINVOID) {
        Slider_Parameter1->SetMin(_vc->GetMin());
        Slider_Parameter1->SetMax(_vc->GetMax());
    } else {
        Slider_Parameter1->SetMin(min);
        Slider_Parameter1->SetMax(max);
    }

    ValueCurve::GetRangeParm2(_vc->GetType(), min, max);
    if (min == MINVOID) {
        Slider_Parameter2->SetMin(_vc->GetMin());
        Slider_Parameter2->SetMax(_vc->GetMax());
    } else {
        Slider_Parameter2->SetMin(min);
        Slider_Parameter2->SetMax(max);
    }

    ValueCurve::GetRangeParm3(_vc->GetType(), min, max);
    if (min == MINVOID) {
        Slider_Parameter3->SetMin(_vc->GetMin());
        Slider_Parameter3->SetMax(_vc->GetMax());
    } else {
        Slider_Parameter3->SetMin(min);
        Slider_Parameter3->SetMax(max);
    }
    ValueCurve::GetRangeParm4(_vc->GetType(), min, max);
    if (min == MINVOID) {
        Slider_Parameter4->SetMin(_vc->GetMin());
        Slider_Parameter4->SetMax(_vc->GetMax());
    } else {
        Slider_Parameter4->SetMin(min);
        Slider_Parameter4->SetMax(max);
    }
}

void ValueCurveDialog::OnChoice1Select(wxCommandEvent& event) {
    _vcp->SetType(std::string(Choice1->GetStringSelection().c_str()));
    _vcp->SetTimeOffset(Slider_TimeOffset->GetValue());
    _vc->SetType(std::string(Choice1->GetStringSelection().c_str()));
    _vcp->Refresh();
    ValidateWindow();

    SetSliderMinMax();

    wxString type = Choice1->GetStringSelection();
    if (type == "Flat") {
        // Dont change anything
    } else if (type == "Ramp") {
        if (_vc->GetParameter1_100() < 50) {
            SetParameter100(2, 100);
        } else {
            SetParameter100(2, 0);
        }
    } else if (type == "Ramp Up/Down") {
        SetParameter100(1, 0);
        SetParameter100(2, 100);
        SetParameter100(3, 0);
    } else if (type == "Music" || type == "Inverted Music") {
        SetParameter100(1, 0);
        SetParameter100(2, 100);
        SetParameter100(3, 50);
    } else if (type == "Music Trigger Fade") {
        SetParameter100(1, 0);
        SetParameter100(2, 100);
        SetParameter100(3, 50);
        SetParameter100(4, 10);
    } else if (type == "Timing Track Toggle") {
        SetParameter100(1, 0);
        SetParameter100(2, 100);
        if (Choice_TimingTrack->GetCount() > 0) {
            Choice_TimingTrack->SetSelection(0);
            _vc->SetTimingTrack(Choice_TimingTrack->GetStringSelection());
        }
    } else if (type == "Timing Track Fade Fixed") {
        SetParameter100(1, 0);
        SetParameter100(2, 100);
        SetParameter(3, 10);
        if (Choice_TimingTrack->GetCount() > 0) {
            Choice_TimingTrack->SetSelection(0);
            _vc->SetTimingTrack(Choice_TimingTrack->GetStringSelection());
        }
    } else if (type == "Timing Track Fade Proportional") {
        SetParameter100(1, 0);
        SetParameter100(2, 100);
        SetParameter(3, 50);
        if (Choice_TimingTrack->GetCount() > 0) {
            Choice_TimingTrack->SetSelection(0);
            _vc->SetTimingTrack(Choice_TimingTrack->GetStringSelection());
        }
    } else if (type == "Ramp Up/Down Hold") {
        SetParameter100(1, 0);
        SetParameter100(2, 100);
        SetParameter100(3, 80);
    } else if (type == "Saw Tooth") {
        SetParameter100(1, 0);
        SetParameter100(2, 100);
        SetParameter100(3, 2);
    } else if (type == "Parabolic Down") {
        SetParameter100(1, 4);
        SetParameter100(2, 0);
    } else if (type == "Parabolic Up") {
        SetParameter100(1, 4);
        SetParameter100(2, 100);
    } else if (type == "Logarithmic Up") {
        SetParameter100(1, 4);
        SetParameter100(2, 100);
    } else if (type == "Logarithmic Down") {
        SetParameter100(1, 15);
        SetParameter100(2, 50);
    } else if (type == "Exponential Up") {
        SetParameter100(1, 100);
        SetParameter100(2, 50);
    } else if (type == "Exponential Down") {
        SetParameter100(1, 100);
        SetParameter100(2, 50);
    } else if (type == "Sine") {
        SetParameter100(1, 75);
        _vc->SetTimeOffset(0);
        SetParameter100(2, 100);
        SetParameter100(3, 10);
        SetParameter100(4, 50);
    } else if (type == "Decaying Sine") {
        SetParameter100(1, 75);
        _vc->SetTimeOffset(0);
        SetParameter100(2, 100);
        SetParameter100(3, 10);
        SetParameter100(4, 50);
    } else if (type == "Random") {
        SetParameter100(1, 0);
        SetParameter100(2, 100);
        SetParameter100(3, 5);
        SetParameter100(4, 0);
    } else if (type == "Abs Sine") {
        SetParameter100(1, 0);
        _vc->SetTimeOffset(0);
        SetParameter100(2, 100);
        SetParameter100(3, 10);
        SetParameter100(4, 50);
    } else if (type == "Square") {
        SetParameter100(1, 0);
        SetParameter100(2, 100);
        SetParameter100(3, 1);
        SetParameter100(4, 0);
    } else if (type == "Custom") {
        // If there are no points add some
        if (_vc->GetPoints().size() == 0) {
            _vc->SetValueAt(0.0, 0.5);
            _vc->SetValueAt(1.0, 0.5);
        }
    }

    ValidateWindow();
}

#pragma region Mouse Control

void ValueCurvePanel::Undo() {
    if (_undo.size() > 0) {
        wxRealPoint p = _undo.back();
        _undo.pop_back();

        if (p.y < 0) {
            _vc->DeletePoint(p.x);
        } else {
            _vc->SetValueAt(p.x, p.y);
        }
    }
}

void ValueCurvePanel::SaveUndo(float x, float y) {
    wxRealPoint p(x, y);
    _undo.push_back(p);
}

void ValueCurvePanel::SaveUndoSelected() {
    if (_vc->IsSetPoint(_grabbedPoint)) {
        SaveUndo(_grabbedPoint, _vc->GetValueAt(_grabbedPoint, 0, 1));
    } else {
        SaveUndo(_grabbedPoint, -1.0f);
    }
}

float ValueCurvePanel::ToReal(float y) {
    return _vc->GetMin() + y * (_vc->GetMax() - _vc->GetMin());
}

float ValueCurvePanel::ToNormalised(float y) {
    return (y - _vc->GetMin()) / (_vc->GetMax() - _vc->GetMin());
}

void ValueCurvePanel::mouseLeftDClick(wxMouseEvent& event) {
    if (_type == "Custom" && _timeOffset == 0) {
        float x, y;
        Convert(x, y, event);

        if (_vc->NearCustomPoint(x, y)) {
            auto point = vcSortablePoint::Normalise(x);
            wxTextEntryDialog dlg(GetParent(), "Enter value:", "Manual entry", std::to_string(ToReal(_vc->GetPointAt(point))));
            if (dlg.ShowModal() == wxID_OK) {
                float v = wxAtof(dlg.GetValue());
                if (v < _vc->GetMin())
                    v = _vc->GetMin();
                if (v > _vc->GetMax())
                    v = _vc->GetMax();
                SaveUndoSelected();
                _vc->SetValueAt(x, ToNormalised(v));
                Refresh();
            }
        }
    }
}
void ValueCurvePanel::mouseLeftDown(wxMouseEvent& event) {
    if (_type == "Custom" && _timeOffset == 0) {
        float x, y;
        Convert(x, y, event);
        _grabbedPoint = x;
        if (_grabbedPoint < 0.0f) {
            _grabbedPoint = 0.0f;
        } else if (_grabbedPoint > 1.0f) {
            _grabbedPoint = 1.0f;
        }
        _grabbedPoint = vcSortablePoint::Normalise(_grabbedPoint);
        _originalGrabbedPoint = _grabbedPoint;
        _minGrabbedPoint = _vc->FindMinPointLessThan(_grabbedPoint);
        _maxGrabbedPoint = _vc->FindMaxPointGreaterThan(_grabbedPoint);
        SaveUndoSelected();
        CaptureMouse();
        mouseMoved(event);
        Refresh();
    }
}
void ValueCurvePanel::mouseCaptureLost(wxMouseCaptureLostEvent& event) {
    if (_type == "Custom" && _timeOffset == 0) {
        // lets not do anything
    }
    Refresh();
}
void ValueCurvePanel::mouseLeftUp(wxMouseEvent& event) {
    if (_type == "Custom" && _timeOffset == 0 && HasCapture()) {
        float x, y;
        Convert(x, y, event);
        if (y < 0.0f) {
            y = 0.0f;
        } else if (y > 1.0f) {
            y = 1.0f;
        }
        _vc->SetValueAt(_grabbedPoint, y);
        //_grabbedPoint = -1;
        ReleaseMouse();
    }
    Refresh();
}

void ValueCurvePanel::Delete() {
    if (_grabbedPoint > 0.0 && _grabbedPoint < 1.0) {
        _vc->DeletePoint(_grabbedPoint);
        _grabbedPoint = -1;
        Refresh();
    } else {
        wxBell();
    }
}

void ValueCurvePanel::mouseEnter(wxMouseEvent& event) {
    if (_type == "Custom" && _timeOffset == 0) {
        SetCursor(wxCURSOR_CROSS);
        mouseMoved(event);
    }
}

void ValueCurvePanel::mouseLeave(wxMouseEvent& event) {
    SetCursor(wxCURSOR_DEFAULT);
}

void ValueCurvePanel::mouseMoved(wxMouseEvent& event) {
    if (_type == "Custom" && _timeOffset == 0) {
        float x, y;
        Convert(x, y, event);
        if (y < 0.0f) {
            y = 0.0f;
        } else if (y > 1.0f) {
            y = 1.0f;
        }

        if (_vc->NearCustomPoint(x, y)) {
            SetCursor(wxCURSOR_HAND);
        } else {
            SetCursor(wxCURSOR_CROSS);
        }

        if (HasCapture()) {
            if (x <= _minGrabbedPoint) {
                x = _minGrabbedPoint;
            } else if (x > _maxGrabbedPoint) {
                x = _maxGrabbedPoint;
            }

            if (_originalGrabbedPoint == 0 || _originalGrabbedPoint == 1.0) {
                // dont allow x to change
            } else {
                _vc->DeletePoint(_grabbedPoint);
                _grabbedPoint = x;
            }
            _vc->SetValueAt(_grabbedPoint, y);

            Refresh();
        }

        std::string time = "";
        if (_start != -1) {
            time = std::string(FORMATTIME((int)(_start + (_end - _start) * x))) + ", ";
        }

        if (_vc->IsSetPoint(vcSortablePoint::Normalise(x))) {
            SetToolTip(wxString::Format(wxT("%s%.2f,%.1f -> %.1f"), time, x, _vc->GetScaledValue(y), ToReal(_vc->GetPointAt(vcSortablePoint::Normalise(x)))));
        } else {
            SetToolTip(wxString::Format(wxT("%s%.2f,%.1f"), time, x, _vc->GetScaledValue(y)));
        }
    } else {
        float x, y;
        Convert(x, y, event);
        if (y < 0.0f) {
            y = 0.0f;
        } else if (y > 1.0f) {
            y = 1.0f;
        }

        std::string time = "";
        if (_start != -1) {
            time = std::string(FORMATTIME((int)(_start + (_end - _start) * x))) + ", ";
        }

        SetToolTip(wxString::Format(wxT("%s%.2f,%.1f"), time, x, _vc->GetScaledValue(y)));
    }
}
#pragma endregion Mouse Control

#pragma region Sliders and TextCtrls

void ValueCurveDialog::UpdateLinkedSlider(wxCommandEvent& event) {
    SetSlidersFromTextCtrls();
}

void ValueCurveDialog::UpdateLinkedTextCtrl(wxScrollEvent& event) {
    SetTextCtrlsFromSliders();
}

void ValueCurveDialog::SetTextCtrlFromSlider(int parm, wxTextCtrl* txt, int value) {
    std::string type = Choice1->GetStringSelection().ToStdString();
    float low, high;
    ValueCurve::GetRangeParm(parm, type, low, high);

    float v = value;
    int d = _vc->GetDivisor();
    if (low == MINVOID) {
        v /= d;
    } else {
        d = 1;
    }

    switch (d) {
    case 10:
    case 360: // this kinda sucks ... its not really the same
        txt->ChangeValue(wxString::Format("%.1f", v));
        break;
    case 100:
        txt->ChangeValue(wxString::Format("%.2f", v));
        break;
    default:
        txt->ChangeValue(wxString::Format("%d", value));
        break;
    }
}

void ValueCurveDialog::SetSliderFromTextCtrl(int parm, wxSlider* slider, float value) {
    std::string type = Choice1->GetStringSelection().ToStdString();
    float low, high;
    ValueCurve::GetRangeParm(parm, type, low, high);

    float v = value;
    int d = _vc->GetDivisor();
    if (low == MINVOID) {
        v *= d;
    }

    slider->SetValue(v);
}

void ValueCurveDialog::SetTextCtrlsFromSliders() {
    SetTextCtrlFromSlider(1, TextCtrl_Parameter1, Slider_Parameter1->GetValue());
    SetTextCtrlFromSlider(2, TextCtrl_Parameter2, Slider_Parameter2->GetValue());
    SetTextCtrlFromSlider(3, TextCtrl_Parameter3, Slider_Parameter3->GetValue());
    SetTextCtrlFromSlider(4, TextCtrl_Parameter4, Slider_Parameter4->GetValue());
    TextCtrl_TimeOffset->SetValue(wxString::Format("%d", Slider_TimeOffset->GetValue()));
}

void ValueCurveDialog::SetSlidersFromTextCtrls() {
    SetSliderFromTextCtrl(1, Slider_Parameter1, wxAtof(TextCtrl_Parameter1->GetValue()));
    SetSliderFromTextCtrl(2, Slider_Parameter2, wxAtof(TextCtrl_Parameter2->GetValue()));
    SetSliderFromTextCtrl(3, Slider_Parameter3, wxAtof(TextCtrl_Parameter3->GetValue()));
    SetSliderFromTextCtrl(4, Slider_Parameter4, wxAtof(TextCtrl_Parameter4->GetValue()));

    Slider_TimeOffset->SetValue(wxAtoi(TextCtrl_TimeOffset->GetValue()));
}

void ValueCurveDialog::OnTextCtrl_Parameter1Text(wxCommandEvent& event) {
    UpdateLinkedSlider(event);
    float i = wxAtof(TextCtrl_Parameter1->GetValue());
    float low, high;
    ValueCurve::GetRangeParm1(Choice1->GetStringSelection().ToStdString(), low, high);
    if (low == MINVOID)
        i *= _vc->GetDivisor();
    _vc->SetParameter1(i);
    _vcp->Refresh();
}

void ValueCurveDialog::OnSlider_Parameter1CmdSliderUpdated(wxScrollEvent& event) {
    UpdateLinkedTextCtrl(event);
    float i = Slider_Parameter1->GetValue();
    _vc->SetParameter1(i);
    _vcp->Refresh();
}

void ValueCurveDialog::OnTextCtrl_Parameter2Text(wxCommandEvent& event) {
    UpdateLinkedSlider(event);
    float i = wxAtof(TextCtrl_Parameter2->GetValue());
    float low, high;
    ValueCurve::GetRangeParm2(Choice1->GetStringSelection().ToStdString(), low, high);
    if (low == MINVOID)
        i *= _vc->GetDivisor();
    _vc->SetParameter2(i);
    _vcp->Refresh();
}

void ValueCurveDialog::OnSlider_Parameter2CmdSliderUpdated(wxScrollEvent& event) {
    UpdateLinkedTextCtrl(event);
    float i = Slider_Parameter2->GetValue();
    _vc->SetParameter2(i);
    _vcp->Refresh();
}

void ValueCurveDialog::OnTextCtrl_Parameter3Text(wxCommandEvent& event) {
    UpdateLinkedSlider(event);
    float i = wxAtof(TextCtrl_Parameter3->GetValue());
    float low, high;
    ValueCurve::GetRangeParm3(Choice1->GetStringSelection().ToStdString(), low, high);
    if (low == MINVOID)
        i *= _vc->GetDivisor();
    _vc->SetParameter3(i);
    _vcp->Refresh();
}

void ValueCurveDialog::OnSlider_Parameter3CmdSliderUpdated(wxScrollEvent& event) {
    UpdateLinkedTextCtrl(event);
    float i = Slider_Parameter3->GetValue();
    _vc->SetParameter3(i);
    _vcp->Refresh();
}

void ValueCurveDialog::OnTextCtrl_Parameter4Text(wxCommandEvent& event) {
    UpdateLinkedSlider(event);
    float i = wxAtof(TextCtrl_Parameter4->GetValue());
    float low, high;
    ValueCurve::GetRangeParm4(Choice1->GetStringSelection().ToStdString(), low, high);
    if (low == MINVOID)
        i *= _vc->GetDivisor();
    _vc->SetParameter4(i);
    _vcp->Refresh();
}

void ValueCurveDialog::OnSlider_Parameter4CmdSliderUpdated(wxScrollEvent& event) {
    UpdateLinkedTextCtrl(event);
    float i = Slider_Parameter4->GetValue();
    _vc->SetParameter4(i);
    _vcp->Refresh();
}

#pragma endregion Sliders and TextCtrls

void ValueCurvePanel::DrawTiming(wxAutoBufferedPaintDC& pdc, long timeMS) {
    wxSize s = GetSize();
    long interval = _end - _start;
    float pos = (float)(timeMS - _start) / (float)interval;
    int x = pos * s.GetWidth() + X_VC_MARGIN;

    pdc.SetPen(*wxBLUE);
    pdc.DrawLine(x, 0, x, s.GetHeight());
}

void ValueCurvePanel::DrawTiming(wxAutoBufferedPaintDC& pdc) {
    if (_timingElement == nullptr) {
        return;
    }

    EffectLayer* el = _timingElement->GetEffectLayer(0);

    for (int i = 0; i < el->GetEffectCount(); i++) {
        Effect* e = el->GetEffect(i);
        if (e->GetStartTimeMS() >= _start || e->GetStartTimeMS() <= _end) {
            DrawTiming(pdc, e->GetStartTimeMS());
        }
        if (e->GetEndTimeMS() >= _start || e->GetEndTimeMS() <= _end) {
            DrawTiming(pdc, e->GetEndTimeMS());
        }
    }
}

void ValueCurvePanel::Paint(wxPaintEvent& event) {
    // wxPaintDC pdc(this);
    wxAutoBufferedPaintDC pdc(this);
    pdc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT)));
    pdc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK)));
    wxSize size = GetSize();
    float w = size.GetWidth() - 2 * X_VC_MARGIN;
    float th = size.GetHeight();
    float h = size.GetHeight() - 2 * Y_VC_MARGIN;

    pdc.DrawRectangle(0, 0, size.GetWidth(), size.GetHeight());

    DrawTiming(pdc);

    pdc.SetBrush(*wxTRANSPARENT_BRUSH);
    if (_vc != nullptr) {
        pdc.SetPen(wxPen(*wxGREEN, 2, wxPENSTYLE_LONG_DASH));
        std::list<vcSortablePoint> pts = _vc->GetPoints();

        if (pts.size() > 1) {
            std::list<vcSortablePoint>::iterator last = pts.begin();
            for (auto p = ++pts.begin(); p != pts.end(); ++p) {
                double x = p->x;
                x += (double)_timeOffset / 100.0;

                double lastx = last->x;
                lastx += (double)_timeOffset / 100.0;

                if (x > 1.0 && lastx <= 1.0) {
                    x -= 1.0;

                    double yat1 = (1.0 - lastx) * (p->y - last->y) + last->y;
                    if (last->IsWrapped() == p->IsWrapped()) {
                        pdc.DrawLine(lastx * w + X_VC_MARGIN, th - last->y * h - Y_VC_MARGIN,
                                     1.0 * w + X_VC_MARGIN, th - yat1 * h - Y_VC_MARGIN);
                    } else {
                        pdc.DrawLine(lastx * w + X_VC_MARGIN, th - p->y * h - Y_VC_MARGIN,
                                     1.0 * w + X_VC_MARGIN, th - p->y * h - Y_VC_MARGIN);
                    }

                    if (last->IsWrapped() == p->IsWrapped()) {
                        pdc.DrawLine(0.0 * w + X_VC_MARGIN, th - yat1 * h - Y_VC_MARGIN,
                                     x * w + X_VC_MARGIN, th - p->y * h - Y_VC_MARGIN);
                    } else {
                        pdc.DrawLine(0.0 * w + X_VC_MARGIN, th - p->y * h - Y_VC_MARGIN,
                                     x * w + X_VC_MARGIN, th - p->y * h - Y_VC_MARGIN);
                    }
                } else {
                    if (x > 1.0) {
                        x -= 1.0;
                    }
                    if (lastx > 1.0) {
                        lastx -= 1.0;
                    }

                    if (last->IsWrapped() == p->IsWrapped()) {
                        pdc.DrawLine(lastx * w + X_VC_MARGIN, th - last->y * h - Y_VC_MARGIN,
                                     x * w + X_VC_MARGIN, th - p->y * h - Y_VC_MARGIN);
                    } else {
                        pdc.DrawLine(lastx * w + X_VC_MARGIN, th - p->y * h - Y_VC_MARGIN,
                                     x * w + X_VC_MARGIN, th - p->y * h - Y_VC_MARGIN);
                    }
                }
                last = p;
            }
        }

        pdc.SetPen(wxPen(*wxRED, 2, wxPENSTYLE_SOLID));
        for (auto it = pts.begin(); it != pts.end(); ++it) {
            double x = it->x;
            x += (double)_timeOffset / 100.0;
            if (x > 1.0) {
                x -= 1.0;
            }
            pdc.DrawRectangle((x * w) - 2 + X_VC_MARGIN, th - (it->y * h) - 2 - Y_VC_MARGIN, 5, 5);
        }

        if (_grabbedPoint != -1 && _type == "Custom" && _timeOffset == 0) {
            pdc.SetPen(wxPen(*wxBLUE, 2, wxPENSTYLE_SOLID));
            pdc.DrawRectangle((_grabbedPoint * w) - 2 + X_VC_MARGIN, th - (_vc->GetValueAt(_grabbedPoint, 0, 1) * h) - 2 - Y_VC_MARGIN, 5, 5);
        }
    }
}

void ValueCurveDialog::ValidateWindow() {
    wxString type = Choice1->GetStringSelection();

    float min = 0;
    float max = 100;
    ValueCurve::GetRangeParm1(type.ToStdString(), min, max);
    if (min == MINVOID)
        min = _vc->GetMin();
    if (max == MAXVOID)
        max = _vc->GetMax();
    if (Slider_Parameter1->GetMin() != min || Slider_Parameter1->GetMax() != max)
        Slider_Parameter1->SetRange(min, max);

    ValueCurve::GetRangeParm2(type.ToStdString(), min, max);
    if (min == MINVOID)
        min = _vc->GetMin();
    if (max == MAXVOID)
        max = _vc->GetMax();
    if (Slider_Parameter2->GetMin() != min || Slider_Parameter2->GetMax() != max)
        Slider_Parameter2->SetRange(min, max);

    ValueCurve::GetRangeParm3(type.ToStdString(), min, max);
    if (min == MINVOID)
        min = _vc->GetMin();
    if (max == MAXVOID)
        max = _vc->GetMax();
    if (Slider_Parameter3->GetMin() != min || Slider_Parameter3->GetMax() != max)
        Slider_Parameter3->SetRange(min, max);

    ValueCurve::GetRangeParm4(type.ToStdString(), min, max);
    if (min == MINVOID)
        min = _vc->GetMin();
    if (max == MAXVOID)
        max = _vc->GetMax();
    if (Slider_Parameter4->GetMin() != min || Slider_Parameter4->GetMax() != max)
        Slider_Parameter4->SetRange(min, max);

    if (type == "Custom") {
        Slider_Parameter1->Disable();
        TextCtrl_Parameter1->Disable();
        Slider_Parameter2->Disable();
        TextCtrl_Parameter2->Disable();
        Slider_Parameter3->Disable();
        TextCtrl_Parameter3->Disable();
        Slider_Parameter4->Disable();
        TextCtrl_Parameter4->Disable();
    } else if (type == "Sine" || type == "Abs Sine" || type == "Decaying Sine" || type == "Music Trigger Fade") {
        Slider_Parameter1->Enable();
        TextCtrl_Parameter1->Enable();
        Slider_Parameter2->Enable();
        TextCtrl_Parameter2->Enable();
        Slider_Parameter3->Enable();
        TextCtrl_Parameter3->Enable();
        Slider_Parameter4->Enable();
        TextCtrl_Parameter4->Enable();
    } else if (type == "Flat") {
        Slider_Parameter1->Enable();
        TextCtrl_Parameter1->Enable();
        Slider_Parameter2->Disable();
        TextCtrl_Parameter2->Disable();
        Slider_Parameter3->Disable();
        TextCtrl_Parameter3->Disable();
        Slider_Parameter4->Disable();
        TextCtrl_Parameter4->Disable();
    } else if (type == "Ramp" || type == "Parabolic Down" || type == "Parabolic Up" || type == "Logarithmic Up" || type == "Logarithmic Down" || type == "Exponential Up" || type == "Exponential Down" || type == "Timing Track Toggle") {
        Slider_Parameter1->Enable();
        TextCtrl_Parameter1->Enable();
        Slider_Parameter2->Enable();
        TextCtrl_Parameter2->Enable();
        Slider_Parameter3->Disable();
        TextCtrl_Parameter3->Disable();
        Slider_Parameter4->Disable();
        TextCtrl_Parameter4->Disable();
    } else if (type == "Saw Tooth" || type == "Ramp Up/Down Hold" || type == "Ramp Up/Down" || type == "Square" || type == "Random" || type == "Music" || type == "Inverted Music" || type == "Timing Track Fade Fixed" || type == "Timing Track Fade Proportional") {
        Slider_Parameter1->Enable();
        TextCtrl_Parameter1->Enable();
        Slider_Parameter2->Enable();
        TextCtrl_Parameter2->Enable();
        Slider_Parameter3->Enable();
        TextCtrl_Parameter3->Enable();
        Slider_Parameter4->Disable();
        TextCtrl_Parameter4->Disable();
    } else {
        Slider_Parameter1->Enable();
        TextCtrl_Parameter1->Enable();
        Slider_Parameter2->Enable();
        TextCtrl_Parameter2->Enable();
        Slider_Parameter3->Enable();
        TextCtrl_Parameter3->Enable();
        Slider_Parameter4->Enable();
        TextCtrl_Parameter4->Enable();
    }

    if (type == "Flat") {
        StaticText_P1->SetLabel("Level");
        StaticText_P2->SetLabel("N/A");
        StaticText_P3->SetLabel("N/A");
        StaticText_P4->SetLabel("N/A");
        _vc->SetParameter2(0);
        _vc->SetParameter3(0);
        _vc->SetParameter4(0);
    } else if (type == "Ramp") {
        StaticText_P1->SetLabel("Start Level");
        StaticText_P2->SetLabel("End Level");
        StaticText_P3->SetLabel("N/A");
        StaticText_P4->SetLabel("N/A");
        _vc->SetParameter3(0);
        _vc->SetParameter4(0);
    } else if (type == "Timing Track Toggle") {
        StaticText_P1->SetLabel("Low");
        StaticText_P2->SetLabel("High");
        StaticText_P3->SetLabel("N/A");
        StaticText_P4->SetLabel("N/A");
        _vc->SetParameter3(0);
        _vc->SetParameter4(0);
    } else if (type == "Timing Track Fade Fixed") {
        StaticText_P1->SetLabel("Low");
        StaticText_P2->SetLabel("High");
        StaticText_P3->SetLabel("Frames");
        StaticText_P4->SetLabel("N/A");
        _vc->SetParameter4(0);
    } else if (type == "Timing Track Fade Proportional") {
        StaticText_P1->SetLabel("Low");
        StaticText_P2->SetLabel("High");
        StaticText_P3->SetLabel("Prportion");
        StaticText_P4->SetLabel("N/A");
        _vc->SetParameter4(0);
    } else if (type == "Music" || type == "Inverted Music") {
        StaticText_P1->SetLabel("Low");
        StaticText_P2->SetLabel("High");
        StaticText_P3->SetLabel("Gain");
        StaticText_P4->SetLabel("N/A");
        _vc->SetParameter4(0);
    } else if (type == "Music Trigger Fade") {
        StaticText_P1->SetLabel("Low");
        StaticText_P2->SetLabel("High");
        StaticText_P3->SetLabel("Trigger");
        StaticText_P4->SetLabel("Fade");
    } else if (type == "Ramp Up/Down") {
        StaticText_P1->SetLabel("Start Level");
        StaticText_P2->SetLabel("Mid Level");
        StaticText_P3->SetLabel("End Level");
        StaticText_P4->SetLabel("N/A");
        _vc->SetParameter4(0);
    } else if (type == "Ramp Up/Down Hold") {
        StaticText_P1->SetLabel("Start/End Level");
        StaticText_P2->SetLabel("Mid Level");
        StaticText_P3->SetLabel("Mid Level Time");
        StaticText_P4->SetLabel("N/A");
        _vc->SetParameter4(0);
    } else if (type == "Saw Tooth") {
        StaticText_P1->SetLabel("Start Level");
        StaticText_P2->SetLabel("End Level");
        StaticText_P3->SetLabel("Cycles");
        StaticText_P4->SetLabel("N/A");
        _vc->SetParameter4(0);
    } else if (type == "Parabolic Down") {
        StaticText_P1->SetLabel("Slope");
        StaticText_P2->SetLabel("Low");
        StaticText_P3->SetLabel("N/A");
        StaticText_P4->SetLabel("N/A");
        _vc->SetParameter3(0);
        _vc->SetParameter4(0);
    } else if (type == "Parabolic Up") {
        StaticText_P1->SetLabel("Slope");
        StaticText_P2->SetLabel("High");
        StaticText_P3->SetLabel("N/A");
        StaticText_P4->SetLabel("N/A");
        _vc->SetParameter3(0);
        _vc->SetParameter4(0);
    } else if (type == "Logarithmic Up") {
        StaticText_P1->SetLabel("Rate");
        StaticText_P2->SetLabel("Vertical Offset");
        StaticText_P3->SetLabel("N/A");
        StaticText_P4->SetLabel("N/A");
        _vc->SetParameter3(0);
        _vc->SetParameter4(0);
    } else if (type == "Logarithmic Down") {
        StaticText_P1->SetLabel("Rate");
        StaticText_P2->SetLabel("Vertical Offset");
        StaticText_P3->SetLabel("N/A");
        StaticText_P4->SetLabel("N/A");
        _vc->SetParameter3(0);
        _vc->SetParameter4(0);
    } else if (type == "Exponential Up" || type == "Exponential Down") {
        StaticText_P1->SetLabel("Rate");
        StaticText_P2->SetLabel("Vertical Offset");
        StaticText_P3->SetLabel("N/A");
        StaticText_P4->SetLabel("N/A");
        _vc->SetParameter3(0);
        _vc->SetParameter4(0);
    } else if (type == "Sine") {
        StaticText_P1->SetLabel("Start");
        StaticText_P2->SetLabel("Amplitude");
        StaticText_P3->SetLabel("Cycles");
        StaticText_P4->SetLabel("Vertical Offset");
    } else if (type == "Decaying Sine") {
        StaticText_P1->SetLabel("Start");
        StaticText_P2->SetLabel("Amplitude");
        StaticText_P3->SetLabel("Cycles");
        StaticText_P4->SetLabel("Vertical Offset");
    } else if (type == "Random") {
        StaticText_P1->SetLabel("Minimum");
        StaticText_P2->SetLabel("Maximum");
        StaticText_P3->SetLabel("Points");
        StaticText_P4->SetLabel("N/A");
    } else if (type == "Abs Sine") {
        StaticText_P1->SetLabel("Start");
        StaticText_P2->SetLabel("Amplitude");
        StaticText_P3->SetLabel("Cycles");
        StaticText_P4->SetLabel("Vertical Offset");
    } else if (type == "Square") {
        StaticText_P1->SetLabel("Start Level");
        StaticText_P2->SetLabel("End Level");
        StaticText_P3->SetLabel("Cycles");
        StaticText_P4->SetLabel("N/A");
        _vc->SetParameter4(0);
    } else if (type == "Custom") {
        StaticText_P1->SetLabel("N/A");
        StaticText_P2->SetLabel("N/A");
        StaticText_P3->SetLabel("N/A");
        StaticText_P4->SetLabel("N/A");
        _vc->SetParameter1(0);
        _vc->SetParameter2(0);
        _vc->SetParameter3(0);
        _vc->SetParameter4(0);
    }

    if (type == "Flat" ||
        type == "Saw Tooth" ||
        type == "Ramp Up/Down Hold" ||
        type == "Parabolic Down" ||
        type == "Parabolic Up" ||
        type == "Decaying Sine" ||
        type == "Logarithmic Up" ||
        type == "Logarithmic Down" ||
        type == "Exponential Up" ||
        type == "Random" ||
        type == "Music" ||
        type == "Inverted Music" ||
        type == "Music Trigger Fade" ||
        type == "Timing Track Toggle" ||
        type == "Timing Track Fade Fixed" ||
        type == "Timing Track Fade Proportional" ||
        type == "Exponential Down") {
        Button_Reverse->Enable(false);
    } else {
        Button_Reverse->Enable();
    }
    if (type == "Logarithmic Up" ||
        type == "Logarithmic Down" ||
        type == "Sine" ||
        type == "Abs Sine" ||
        type == "Music" ||
        type == "Inverted Music" ||
        type == "Music Trigger Fade" ||
        type == "Timing Track Toggle" ||
        type == "Timing Track Fade Fixed" ||
        type == "Timing Track Fade Proportional" ||
        type == "Decaying Sine") {
        Button_Flip->Enable(false);
    } else {
        Button_Flip->Enable();
    }

    if (type == "Timing Track Toggle" ||
        type == "Timing Track Fade Fixed" ||
        type == "Timing Track Fade Proportional") {
        Choice_TimingTrack->Enable();
        TextCtrl_FilterLabel->Enable();
        CheckBox_FilterLabelRegex->Enable();

        if (Choice_TimingTrack->GetStringSelection() == "") {
            Button_Ok->Enable(false);
        } else {
            Button_Ok->Enable();
        }
    } else {
        Button_Ok->Enable();
        Choice_TimingTrack->Enable(false);
        TextCtrl_FilterLabel->Enable(false);
        CheckBox_FilterLabelRegex->Enable(false);
        _vc->SetTimingTrack("");
    }
}

void ValueCurveDialog::OnChar(wxKeyEvent& event) {
    wxChar uc = event.GetUnicodeKey();
    if (_vc->GetType() == "Custom" && Slider_TimeOffset->GetValue() == 0 && (int)uc == (int)WXK_DELETE && _vcp->HasSelected()) {
        _vcp->SaveUndoSelected();
        _vcp->Delete();
    } else if (_vc->GetType() == "Custom" && Slider_TimeOffset->GetValue() == 0 && (uc == 'Z' || uc == 'z') && event.ControlDown() && _vcp->IsDirty()) {
        _vcp->Undo();
    } else {
        event.Skip();
    }
    Refresh();
}

void ValueCurveDialog::OnCheckBox_WrapValuesClick(wxCommandEvent& event) {
    _vc->SetWrap(CheckBox_WrapValues->IsChecked());
    Refresh();
}

void ValueCurveDialog::OnResize(wxSizeEvent& event) {
    OnSize(event);
    if (PresetSizer->GetChildren().GetCount() > 0) {
        int cols = (PresetSizer->GetSize().GetWidth() - 10) / 32;
        if (cols == 0)
            cols = 1;
        int rows = PresetSizer->GetChildren().GetCount() / cols + 1;
        PresetSizer->SetRows(std::max(1, rows));
        PresetSizer->SetCols(cols);
        PresetSizer->Layout();
    }
    Refresh();
}

void ValueCurveDialog::OnButtonLoadClick(wxCommandEvent& event) {
    std::string id = _vc->GetId(); // save if because it will be overwritten

    wxString filename = wxFileSelector(_("Choose value curve file"), wxEmptyString, wxEmptyString, wxEmptyString, "Value Curve files (*.xvc)|*.xvc", wxFD_OPEN);
    if (filename.IsEmpty())
        return;

    float min = _vc->GetMin();
    float max = _vc->GetMax();
    int div = _vc->GetDivisor();
    _vc->SetLimits(0, 100);
    _vc->SetDivisor(1);
    _vc->LoadXVC(filename.ToStdString());
    _vc->SetLimits(min, max);
    _vc->SetDivisor(div);
    SetSliderMinMax();
    _vc->FixChangedScale(min, max, 1);
    _vcp->Refresh();
    _vcp->ClearUndo();
    _vc->SetId(id);
    Choice1->SetStringSelection(wxString(_vc->GetType().c_str()));
    Slider_TimeOffset->SetValue(_vc->GetTimeOffset());
    CheckBox_WrapValues->SetValue(_vc->GetWrap());
    SetParameter(1, _vc->GetParameter1());
    SetParameter(2, _vc->GetParameter2());
    SetParameter(3, _vc->GetParameter3());
    SetParameter(4, _vc->GetParameter4());
    SetTextCtrlsFromSliders();
    _vcp->SetType(_vc->GetType());
    _vcp->SetTimeOffset(_vc->GetTimeOffset());
    ValidateWindow();
}

void ValueCurveDialog::OnButtonExportClick(wxCommandEvent& event) {
    wxLogNull logNo; // kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, "ValueCurve", wxEmptyString, "Value Curves (*.xvc)|*.xvc", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (filename.IsEmpty())
        return;

    _vc->SaveXVC(filename.ToStdString());

    _vcp->ClearUndo();

    PopulatePresets();

    _exported = true;
}

void ValueCurveDialog::ProcessPresetDir(wxDir& directory, bool subdirs) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("ValueCurveDialog Scanning directory for *.xvc files: %s.", (const char*)directory.GetNameWithSep().c_str());

    auto existing = PresetSizer->GetChildren();

    wxArrayString files;
    GetAllFilesInDir(directory.GetNameWithSep(), files, "*.xvc");
    int count = 0;
    for (auto& filename : files) {
        wxFileName fn(filename);
        count++;
        bool found = false;
        for (const auto& it : existing) {
            if (it->GetWindow()->GetLabel() == fn.GetFullPath()) {
                // already there
                found = true;
                break;
            }
        }
        if (!found) {
            ValueCurve vc("");
            vc.LoadXVC(fn);
            long id = wxNewId();
            wxBitmapButton* bmb = new wxBitmapButton(this, id, vc.GetImage(30, 30, GetContentScaleFactor()), wxDefaultPosition,
                                                     wxSize(30, 30), wxBU_AUTODRAW | wxNO_BORDER);
            bmb->SetLabel(fn.GetFullPath());
            bmb->SetToolTip(fn.GetFullPath());
            PresetSizer->Add(bmb);
            Connect(id, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ValueCurveDialog::OnButtonPresetClick);
        }
    }
    logger_base.info("    Found %d.", count);

    if (subdirs) {
        wxString filename;
        bool cont = directory.GetFirst(&filename, "*", wxDIR_DIRS);
        while (cont) {
            wxDir dir(directory.GetNameWithSep() + filename);
            ProcessPresetDir(dir, subdirs);
            cont = directory.GetNext(&filename);
        }
    }
}

void ValueCurveDialog::PopulatePresets() {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxDir dir(xLightsFrame::CurrentDir);

    ProcessPresetDir(dir, false);

    wxString d = ValueCurve::GetValueCurveFolder(xLightsFrame::CurrentDir.ToStdString());

    if (wxDir::Exists(d)) {
        dir.Open(d);
        ProcessPresetDir(dir, true);
    } else {
        logger_base.info("Directory for *.xvc files not found: %s.", (const char*)d.c_str());
    }

    wxStandardPaths stdp = wxStandardPaths::Get();

#ifndef __WXMSW__
    d = wxStandardPaths::Get().GetResourcesDir() + "/valuecurves";
#else
    d = wxFileName(stdp.GetExecutablePath()).GetPath() + "/valuecurves";
#endif
    if (wxDir::Exists(d)) {
        dir.Open(d);
        ProcessPresetDir(dir, true);
    } else {
        logger_base.info("Directory for *.xvc files not found: %s.", (const char*)d.c_str());
    }

    PresetSizer->Layout();
    Layout();
    // Fit();
}

void ValueCurveDialog::OnButtonPresetClick(wxCommandEvent& event) {
    if (_vcp->IsDirty()) {
        if (wxMessageBox("Are you sure you want to discard your current value curve?", "Are you sure?", wxYES_NO | wxCENTER, this) == wxNO) {
            return;
        }
    }

    wxString filename = ((wxBitmapButton*)event.GetEventObject())->GetLabel();

    std::string id = _vc->GetId(); // save if because it will be overwritten
    float min = _vc->GetMin();
    float max = _vc->GetMax();
    int div = _vc->GetDivisor();
    _vc->SetLimits(0, 100);
    _vc->SetDivisor(1);
    _vc->LoadXVC(filename.ToStdString());
    _vc->SetLimits(min, max);
    _vc->SetDivisor(div);
    SetSliderMinMax();
    _vc->FixChangedScale(min, max, 1);
    _vcp->Refresh();
    _vcp->ClearUndo();
    _vc->SetId(id);
    Choice1->SetStringSelection(wxString(_vc->GetType().c_str()));
    Slider_TimeOffset->SetValue(_vc->GetTimeOffset());
    CheckBox_WrapValues->SetValue(_vc->GetWrap());
    SetParameter(1, _vc->GetParameter1());
    SetParameter(2, _vc->GetParameter2());
    SetParameter(3, _vc->GetParameter3());
    SetParameter(4, _vc->GetParameter4());
    SetTextCtrlsFromSliders();
    _vcp->SetType(_vc->GetType());
    _vcp->SetTimeOffset(_vc->GetTimeOffset());
    ValidateWindow();
}

void ValueCurveDialog::OnButton_ReverseClick(wxCommandEvent& event) {
    _vc->Reverse();
    Choice1->SetStringSelection(wxString(_vc->GetType().c_str()));
    Slider_TimeOffset->SetValue(_vc->GetTimeOffset());
    SetParameter(1, _vc->GetParameter1());
    SetParameter(2, _vc->GetParameter2());
    SetParameter(3, _vc->GetParameter3());
    SetParameter(4, _vc->GetParameter4());
    SetTextCtrlsFromSliders();
    _vcp->Refresh();
    _vcp->SetType(_vc->GetType());
    _vcp->SetTimeOffset(_vc->GetTimeOffset());
    _vcp->ClearUndo();
    ValidateWindow();
}

void ValueCurveDialog::OnSlider_TimeOffsetCmdSliderUpdated(wxScrollEvent& event) {
    int i = Slider_TimeOffset->GetValue();
    wxString s = wxString::Format("%d", i);
    if (TextCtrl_TimeOffset->GetValue() != s) {
        TextCtrl_TimeOffset->SetValue(s);
    }
    _vc->SetTimeOffset(i);
    _vcp->SetTimeOffset(Slider_TimeOffset->GetValue());
    _vcp->Refresh();
}

void ValueCurveDialog::OnTextCtrl_TimeOffsetText(wxCommandEvent& event) {
    int i = wxAtoi(TextCtrl_TimeOffset->GetValue());
    if (Slider_TimeOffset->GetValue() != i) {
        Slider_TimeOffset->SetValue(i);
    }
    _vc->SetTimeOffset(i);
    _vcp->SetTimeOffset(Slider_TimeOffset->GetValue());
    _vcp->Refresh();
}

void ValueCurveDialog::OnButton_FlipClick(wxCommandEvent& event) {
    _vc->Flip();
    Choice1->SetStringSelection(wxString(_vc->GetType().c_str()));
    Slider_TimeOffset->SetValue(_vc->GetTimeOffset());
    SetParameter(1, _vc->GetParameter1());
    SetParameter(2, _vc->GetParameter2());
    SetParameter(3, _vc->GetParameter3());
    SetParameter(4, _vc->GetParameter4());
    SetTextCtrlsFromSliders();
    _vcp->Refresh();
    _vcp->SetType(_vc->GetType());
    _vcp->SetTimeOffset(_vc->GetTimeOffset());
    _vcp->ClearUndo();
    ValidateWindow();
}

void ValueCurveDialog::OnChoice_TimingTrackSelect(wxCommandEvent& event) {
    _vc->SetTimingTrack(Choice_TimingTrack->GetStringSelection());
    ValidateWindow();
}

void ValueCurveDialog::OnTextCtrl_FilterLabelText(wxCommandEvent& event)
{
    _vc->SetFilterLabelText(TextCtrl_FilterLabel->GetValue().ToStdString());
    ValidateWindow();
}

void ValueCurveDialog::OnCheckBox_FilterLabelRegexClick(wxCommandEvent& event)
{
    _vc->SetFilterLabelRegex(CheckBox_FilterLabelRegex->IsChecked());
}
