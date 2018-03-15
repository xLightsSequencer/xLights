#include "ValueCurveDialog.h"
#include "xLightsMain.h"

//(*InternalHeaders(ValueCurveDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/dcbuffer.h>
#include <wx/valnum.h>
#include <wx/settings.h>
#include <wx/filedlg.h>
#include <wx/file.h>
#include <wx/msgdlg.h>
#include "ValueCurve.h"
#include <log4cpp/Category.hh>
#include "xLightsVersion.h"
#include "UtilFunctions.h"
#include "xLightsApp.h"

BEGIN_EVENT_TABLE(ValueCurvePanel, wxWindow)
    EVT_MOTION(ValueCurvePanel::mouseMoved)
    EVT_LEFT_DOWN(ValueCurvePanel::mouseLeftDown)
    EVT_LEFT_UP(ValueCurvePanel::mouseLeftUp)
    EVT_ENTER_WINDOW(ValueCurvePanel::mouseEnter)
    EVT_LEAVE_WINDOW(ValueCurvePanel::mouseLeave)
    EVT_PAINT(ValueCurvePanel::Paint)
    EVT_MOUSE_CAPTURE_LOST(ValueCurvePanel::mouseCaptureLost)
END_EVENT_TABLE()

ValueCurvePanel::ValueCurvePanel(wxWindow* parent, Element* timingElement, int start, int end, wxWindowID id, const wxPoint &pos, const wxSize &size, long style)
    : wxWindow(parent, id, pos, size, style, "ID_VCP"), xlCustomControl()
{
    Connect(wxEVT_LEFT_DOWN, (wxObjectEventFunction)&ValueCurvePanel::mouseLeftDown, 0, this);
    Connect(wxEVT_LEFT_UP, (wxObjectEventFunction)&ValueCurvePanel::mouseLeftUp, 0, this);
    Connect(wxEVT_ENTER_WINDOW, (wxObjectEventFunction)&ValueCurvePanel::mouseEnter, 0, this);
    Connect(wxEVT_LEAVE_WINDOW, (wxObjectEventFunction)&ValueCurvePanel::mouseLeave, 0, this);
    Connect(wxEVT_MOTION, (wxObjectEventFunction)&ValueCurvePanel::mouseMoved, 0, this);
    Connect(wxEVT_PAINT, (wxObjectEventFunction)&ValueCurvePanel::Paint, 0, this);
    Connect(wxEVT_MOUSE_CAPTURE_LOST, (wxObjectEventFunction)&ValueCurvePanel::mouseCaptureLost, 0, this);
    // ReSharper disable CppVirtualFunctionCallInsideCtor
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    // ReSharper restore CppVirtualFunctionCallInsideCtor
    _grabbedPoint = -1;
    _start = start;
    _end = end;
    _timingElement = timingElement;
}

void ValueCurvePanel::Convert(float &x, float &y, wxMouseEvent& event) {
    wxSize size = GetSize();
    float startX = 0.0; // size.GetWidth() / 10.0;
    float startY = 0.0; // size.GetHeight() / 10.0;
    float bw = size.GetWidth(); //  *0.8;
    float bh = size.GetHeight(); //  *0.8;

    x = (event.GetX() - startX) / bw;
    y = 1.0 - (event.GetY() - startY) / bh;
}

//(*IdInit(ValueCurveDialog)
const long ValueCurveDialog::ID_STATICTEXT3 = wxNewId();
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
const long ValueCurveDialog::ID_BUTTON3 = wxNewId();
const long ValueCurveDialog::ID_BUTTON4 = wxNewId();
const long ValueCurveDialog::ID_BUTTON1 = wxNewId();
const long ValueCurveDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(ValueCurveDialog,wxDialog)
    //(*EventTable(ValueCurveDialog)
    //*)
END_EVENT_TABLE()

ValueCurveDialog::ValueCurveDialog(wxWindow* parent, ValueCurve* vc, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _vc = vc;

    //(*Initialize(ValueCurveDialog)
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer4;
    wxFlexGridSizer* FlexGridSizer5;
    wxFlexGridSizer* FlexGridSizer6;
    wxFlexGridSizer* FlexGridSizer7;
    wxFlexGridSizer* FlexGridSizer8;

    Create(parent, id, _("Value Curve"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxCLOSE_BOX, _T("id"));
    SetClientSize(wxDefaultSize);
    Move(wxDefaultPosition);
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer1->AddGrowableRow(0);
    FlexGridSizer6 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer6->AddGrowableCol(1);
    FlexGridSizer6->AddGrowableRow(0);
    FlexGridSizer5 = new wxFlexGridSizer(2, 1, 0, 0);
    FlexGridSizer5->AddGrowableRow(1);
    StaticText_TopValue = new wxStaticText(this, ID_STATICTEXT3, _("100"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT, _T("ID_STATICTEXT3"));
    FlexGridSizer5->Add(StaticText_TopValue, 1, wxALL|wxEXPAND, 2);
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
    Choice1->Append(_("Custom"));
    FlexGridSizer2->Add(Choice1, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_P1 = new wxStaticText(this, ID_STATICTEXT1, _("XXXXXXXXXXXXXXX"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer2->Add(StaticText_P1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Parameter1 = new wxSlider(this, ID_SLIDER_Parameter1, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Parameter1"));
    FlexGridSizer2->Add(Slider_Parameter1, 1, wxALL|wxEXPAND, 2);
    TextCtrl_Parameter1 = new wxTextCtrl(this, IDD_TEXTCTRL_Parameter1, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Parameter1"));
    TextCtrl_Parameter1->SetMaxLength(5);
    FlexGridSizer2->Add(TextCtrl_Parameter1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText_P2 = new wxStaticText(this, ID_STATICTEXT2, _("Parameter 2"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer2->Add(StaticText_P2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Parameter2 = new wxSlider(this, ID_SLIDER_Parameter2, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Parameter2"));
    FlexGridSizer2->Add(Slider_Parameter2, 1, wxALL|wxEXPAND, 2);
    TextCtrl_Parameter2 = new wxTextCtrl(this, IDD_TEXTCTRL_Parameter2, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Parameter2"));
    TextCtrl_Parameter2->SetMaxLength(5);
    FlexGridSizer2->Add(TextCtrl_Parameter2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText_P3 = new wxStaticText(this, ID_STATICTEXT5, _("Parameter 3"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
    FlexGridSizer2->Add(StaticText_P3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Parameter3 = new wxSlider(this, ID_SLIDER_Parameter3, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Parameter3"));
    FlexGridSizer2->Add(Slider_Parameter3, 1, wxALL|wxEXPAND, 2);
    TextCtrl_Parameter3 = new wxTextCtrl(this, IDD_TEXTCTRL_Parameter3, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Parameter3"));
    TextCtrl_Parameter3->SetMaxLength(5);
    FlexGridSizer2->Add(TextCtrl_Parameter3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText_P4 = new wxStaticText(this, ID_STATICTEXT6, _("Label"), wxDefaultPosition, wxSize(121,16), 0, _T("ID_STATICTEXT6"));
    FlexGridSizer2->Add(StaticText_P4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Parameter4 = new wxSlider(this, ID_SLIDER_Parameter4, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Parameter4"));
    FlexGridSizer2->Add(Slider_Parameter4, 1, wxALL|wxEXPAND, 2);
    TextCtrl_Parameter4 = new wxTextCtrl(this, IDD_TEXTCTRL_Parameter4, _("0"), wxDefaultPosition, wxDLG_UNIT(this,wxSize(25,-1)), 0, wxDefaultValidator, _T("IDD_TEXTCTRL_Parameter4"));
    TextCtrl_Parameter4->SetMaxLength(5);
    FlexGridSizer2->Add(TextCtrl_Parameter4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_WrapValues = new wxCheckBox(this, ID_CHECKBOX_WrapValues, _("Wrap Values"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_WrapValues"));
    CheckBox_WrapValues->SetValue(false);
    FlexGridSizer2->Add(CheckBox_WrapValues, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Reverse = new wxButton(this, ID_BUTTON5, _("Reverse"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON5"));
    FlexGridSizer2->Add(Button_Reverse, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 2);
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
    Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ValueCurveDialog::OnButtonLoadClick);
    Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ValueCurveDialog::OnButtonExportClick);
    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ValueCurveDialog::OnButton_OkClick);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ValueCurveDialog::OnButton_CancelClick);
    Connect(wxEVT_SIZE,(wxObjectEventFunction)&ValueCurveDialog::OnResize);
    //*)

    Connect(wxID_ANY, wxEVT_CHAR_HOOK, wxKeyEventHandler(ValueCurveDialog::OnChar), (wxObject*)nullptr, this);

    int start = -1;
    int end = -1;
    Effect* eff = xLightsApp::GetFrame()->GetMainSequencer()->GetSelectedEffect();
    if (eff != nullptr)
    {
        start = eff->GetStartTimeMS();
        end = eff->GetEndTimeMS();
    }

    Element* timingElement = xLightsApp::GetFrame()->GetMainSequencer()->PanelEffectGrid->GetActiveTimingElement();

    _vcp = new ValueCurvePanel(this, timingElement, start, end, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER);
    _vcp->SetMinSize(wxSize(200, 100));
    _vcp->SetValue(_vc);
    _vcp->SetType(_vc->GetType());
    FlexGridSizer4->Add(_vcp, 1, wxALL | wxEXPAND, 2);
    // ReSharper disable once CppVirtualFunctionCallInsideCtor
    Layout();
    // ReSharper disable once CppVirtualFunctionCallInsideCtor
    Fit();

    _backup = *_vc;

    wxString sMin; // = "";
    wxString sMax; // = "";

    switch(_vc->GetDivisor())
    {
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
    SetParameter(1, _vc->GetParameter1());
    SetParameter(2, _vc->GetParameter2());
    SetParameter(3, _vc->GetParameter3());
    SetParameter(4, _vc->GetParameter4());
    SetTextCtrlsFromSliders();
    Choice1->SetFocus();

    PopulatePresets();

    Layout();
    Fit();

    ValidateWindow();
}

ValueCurveDialog::~ValueCurveDialog()
{
    //(*Destroy(ValueCurveDialog)
    //*)
}


void ValueCurveDialog::OnButton_OkClick(wxCommandEvent& event)
{
    _vc->SetRealValue();
    EndDialog(wxOK);
}

void ValueCurveDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    *_vc = _backup;
    EndDialog(wxCANCEL);
}

void ValueCurveDialog::SetParameter(int p, float v)
{
    switch (p)
    {
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

void ValueCurveDialog::SetSliderMinMax()
{
    float min, max;

    ValueCurve::GetRangeParm1(_vc->GetType(), min, max);
    if (min == MINVOID)
    {
        Slider_Parameter1->SetMin(_vc->GetMin());
        Slider_Parameter1->SetMax(_vc->GetMax());
    }
    else
    {
        Slider_Parameter1->SetMin(min);
        Slider_Parameter1->SetMax(max);
    }

    ValueCurve::GetRangeParm2(_vc->GetType(), min, max);
    if (min == MINVOID)
    {
        Slider_Parameter2->SetMin(_vc->GetMin());
        Slider_Parameter2->SetMax(_vc->GetMax());
    }
    else
    {
        Slider_Parameter2->SetMin(min);
        Slider_Parameter2->SetMax(max);
    }

    ValueCurve::GetRangeParm3(_vc->GetType(), min, max);
    if (min == MINVOID)
    {
        Slider_Parameter3->SetMin(_vc->GetMin());
        Slider_Parameter3->SetMax(_vc->GetMax());
    }
    else
    {
        Slider_Parameter3->SetMin(min);
        Slider_Parameter3->SetMax(max);
    }
    ValueCurve::GetRangeParm4(_vc->GetType(), min, max);
    if (min == MINVOID)
    {
        Slider_Parameter4->SetMin(_vc->GetMin());
        Slider_Parameter4->SetMax(_vc->GetMax());
    }
    else
    {
        Slider_Parameter4->SetMin(min);
        Slider_Parameter4->SetMax(max);
    }
}

void ValueCurveDialog::OnChoice1Select(wxCommandEvent& event)
{
    _vcp->SetType(std::string(Choice1->GetStringSelection().c_str()));
    _vc->SetType(std::string(Choice1->GetStringSelection().c_str()));
    _vcp->Refresh();
    ValidateWindow();

    SetSliderMinMax();

    wxString type = Choice1->GetStringSelection();
    if (type == "Flat")
    {
        // Dont change anything
    }
    else if (type == "Ramp")
    {
        if (_vc->GetParameter1() < 50)
        {
            SetParameter(2, 100);
        }
        else
        {
            SetParameter(2, 0);
        }
    }
    else if (type == "Ramp Up/Down")
    {
        SetParameter(1, 0);
        SetParameter(2, 100);
        SetParameter(3, 0);
    }
    else if (type == "Ramp Up/Down Hold")
    {
        SetParameter(1, 0);
        SetParameter(2, 100);
        SetParameter(3, 80);
    }
    else if (type == "Saw Tooth")
    {
        SetParameter(1, 0);
        SetParameter(2, 100);
        SetParameter(3, 2);
    }
    else if (type == "Parabolic Down")
    {
        SetParameter(1, 4);
        SetParameter(2, 0);
    }
    else if (type == "Parabolic Up")
    {
        SetParameter(1, 4);
        SetParameter(2, 100);
    }
    else if (type == "Logarithmic Up")
    {
        SetParameter(1, 4);
        SetParameter(2, 100);
    }
    else if (type == "Logarithmic Down")
    {
        SetParameter(1, 15);
        SetParameter(2, 50);
    }
    else if (type == "Exponential Up")
    {
        SetParameter(1, 100);
        SetParameter(2, 50);
    }
    else if (type == "Exponential Down")
    {
        SetParameter(1, 100);
        SetParameter(2, 50);
    }
    else if (type == "Sine")
    {
        SetParameter(1, 75);
        SetParameter(2, 100);
        SetParameter(3, 10);
        SetParameter(4, 50);
    }
    else if (type == "Decaying Sine")
    {
        SetParameter(1, 75);
        SetParameter(2, 100);
        SetParameter(3, 10);
        SetParameter(4, 50);
    }
    else if (type == "Abs Sine")
    {
        SetParameter(1, 0);
        SetParameter(2, 100);
        SetParameter(3, 10);
        SetParameter(4, 50);
    }
    else if (type == "Square")
    {
        SetParameter(1, 0);
        SetParameter(2, 100);
        SetParameter(3, 1);
        SetParameter(4, 0);
    }
    else if (type == "Custom")
    {
        // Dont do anything
    }
}

#pragma region Mouse Control

void ValueCurvePanel::Undo()
{
    if (_undo.size() > 0)
    {
        wxRealPoint p = _undo.back();
        _undo.pop_back();

        if (p.y < 0)
        {
            _vc->DeletePoint(p.x);
        }
        else
        {
            _vc->SetValueAt(p.x, p.y);
        }
    }
}

void ValueCurvePanel::SaveUndo(float x, float y)
{
    wxRealPoint p(x, y);
    _undo.push_back(p);
}

void ValueCurvePanel::SaveUndoSelected()
{
    if (_vc->IsSetPoint(_grabbedPoint))
    {
        SaveUndo(_grabbedPoint, _vc->GetValueAt(_grabbedPoint));
    }
    else
    {
        SaveUndo(_grabbedPoint, -1.0f);
    }
}

void ValueCurvePanel::mouseLeftDown(wxMouseEvent& event)
{
    if (_type == "Custom")
    {
        float x, y;
        Convert(x, y, event);
        _grabbedPoint = x;
        if (_grabbedPoint < 0.0f)
        {
            _grabbedPoint = 0.0f;
        }
        else if (_grabbedPoint > 1.0f)
        {
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
void ValueCurvePanel::mouseCaptureLost(wxMouseCaptureLostEvent& event)
{
    if (_type == "Custom")
    {
        // lets not do anything
    }
    Refresh();
}
void ValueCurvePanel::mouseLeftUp(wxMouseEvent& event)
{
    if (_type == "Custom" && HasCapture())
    {
        float x, y;
        Convert(x, y, event);
        if (y < 0.0f)
        {
            y = 0.0f;
        }
        else if (y > 1.0f)
        {
            y = 1.0f;
        }
        _vc->SetValueAt(_grabbedPoint, y);
        //_grabbedPoint = -1;
        ReleaseMouse();
    }
    Refresh();
}

void ValueCurvePanel::Delete()
{
    if (_grabbedPoint >= 0)
    {
        _vc->DeletePoint(_grabbedPoint);
        _grabbedPoint = -1;
        Refresh();
    }
}

void ValueCurvePanel::mouseEnter(wxMouseEvent& event)
{
    if (_type == "Custom")
    {
        SetCursor(wxCURSOR_CROSS);
        mouseMoved(event);
    }
}

void ValueCurvePanel::mouseLeave(wxMouseEvent& event)
{
    SetCursor(wxCURSOR_DEFAULT);
}

void ValueCurvePanel::mouseMoved(wxMouseEvent& event)
{
    if (_type == "Custom")
    {
        float x, y;
        Convert(x, y, event);
        if (y < 0.0f)
        {
            y = 0.0f;
        }
        else if (y > 1.0f)
        {
            y = 1.0f;
        }

        if (_vc->NearCustomPoint(x, y))
        {
            SetCursor(wxCURSOR_SIZENWSE);
        }
        else
        {
            SetCursor(wxCURSOR_CROSS);
        }

        if (HasCapture())
        {
            if (x <= _minGrabbedPoint)
            {
                x = _minGrabbedPoint;
            }
            else if (x > _maxGrabbedPoint)
            {
                x = _maxGrabbedPoint;
            }

            if (_originalGrabbedPoint == 0 || _originalGrabbedPoint == 1.0)
            {
                // dont allow x to change
            }
            else
            {
                _vc->DeletePoint(_grabbedPoint);
                _grabbedPoint = x;
            }
            _vc->SetValueAt(_grabbedPoint, y);

            Refresh();
        }

        std::string time = "";
        if (_start != -1)
        {
            time = std::string(FORMATTIME((int)(_start + (_end - _start) * x))) + ", ";
        }

        SetToolTip(wxString::Format(wxT("%s%.2f,%.1f"), time, x, _vc->GetOutputValue(y)));
    }
    else
    {
        float x, y;
        Convert(x, y, event);
        if (y < 0.0f)
        {
            y = 0.0f;
        }
        else if (y > 1.0f)
        {
            y = 1.0f;
        }

        std::string time = "";
        if (_start != -1)
        {
            time = std::string(FORMATTIME((int)(_start + (_end - _start) * x))) + ", ";
        }

        SetToolTip(wxString::Format(wxT("%s%.2f,%.1f"), time, x, _vc->GetOutputValue(y)));
    }
}
#pragma endregion Mouse Control

#pragma region Sliders and TextCtrls

void ValueCurveDialog::UpdateLinkedSlider(wxCommandEvent& event)
{
    wxTextCtrl * txt = (wxTextCtrl*)event.GetEventObject();
    wxString name = txt->GetName();
    if (name.Contains("IDD_")) {
        name.Replace("IDD_TEXTCTRL_", "ID_SLIDER_");
    }
    else {
        name.Replace("ID_TEXTCTRL_", "IDD_SLIDER_");
    }
    wxSlider *slider = (wxSlider*)txt->GetParent()->FindWindowByName(name);
    if (slider == nullptr) {
        return;
    }
    int value = wxAtoi(txt->GetValue());

    if (value < slider->GetMin()) {
        value = slider->GetMin();
        wxString val_str;
        val_str << value;
        txt->ChangeValue(val_str);
    }
    else if (value > slider->GetMax()) {
        value = slider->GetMax();
        wxString val_str;
        val_str << value;
        txt->ChangeValue(val_str);
    }
    slider->SetValue(value);
}

void ValueCurveDialog::UpdateLinkedTextCtrl(wxScrollEvent& event)
{
    SetTextCtrlsFromSliders();
}

void ValueCurveDialog::SetTextCtrlFromSlider(int parm, wxTextCtrl* txt, int value)
{
    std::string type = Choice1->GetStringSelection().ToStdString();
    float low, high;
    ValueCurve::GetRangeParm(parm, type, low, high);

    float v = value;

    int d = _vc->GetDivisor();

    if (low == MINVOID)
    {
        v /= d;
    }
    else
    {
        d = 1;
    }

    switch(d)
    {
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

void ValueCurveDialog::SetTextCtrlsFromSliders()
{
    SetTextCtrlFromSlider(1, TextCtrl_Parameter1, Slider_Parameter1->GetValue());
    SetTextCtrlFromSlider(2, TextCtrl_Parameter2, Slider_Parameter2->GetValue());
    SetTextCtrlFromSlider(3, TextCtrl_Parameter3, Slider_Parameter3->GetValue());
    SetTextCtrlFromSlider(4, TextCtrl_Parameter4, Slider_Parameter4->GetValue());
}

void ValueCurveDialog::OnTextCtrl_Parameter1Text(wxCommandEvent& event)
{
    UpdateLinkedSlider(event);
    float i = wxAtof(TextCtrl_Parameter1->GetValue());
    float low, high;
    ValueCurve::GetRangeParm1(Choice1->GetStringSelection().ToStdString(), low, high);
    if (low == MINVOID)
        i *= _vc->GetDivisor();
    _vc->SetParameter1(i);
    _vcp->Refresh();
}
void ValueCurveDialog::OnSlider_Parameter1CmdSliderUpdated(wxScrollEvent& event)
{
    UpdateLinkedTextCtrl(event);
    int i = Slider_Parameter1->GetValue();
    _vc->SetParameter1(i);
    _vcp->Refresh();
}

void ValueCurveDialog::OnTextCtrl_Parameter2Text(wxCommandEvent& event)
{
    UpdateLinkedSlider(event);
    float i = wxAtof(TextCtrl_Parameter2->GetValue());
    float low, high;
    ValueCurve::GetRangeParm2(Choice1->GetStringSelection().ToStdString(), low, high);
    if (low == MINVOID)
        i *= _vc->GetDivisor();
    _vc->SetParameter2(i);
    _vcp->Refresh();
}

void ValueCurveDialog::OnSlider_Parameter2CmdSliderUpdated(wxScrollEvent& event)
{
    UpdateLinkedTextCtrl(event);
    int i = Slider_Parameter2->GetValue();
    _vc->SetParameter2(i);
    _vcp->Refresh();
}

void ValueCurveDialog::OnTextCtrl_Parameter3Text(wxCommandEvent& event)
{
    UpdateLinkedSlider(event);
    float i = wxAtof(TextCtrl_Parameter3->GetValue());
    float low, high;
    ValueCurve::GetRangeParm3(Choice1->GetStringSelection().ToStdString(), low, high);
    if (low == MINVOID)
        i *= _vc->GetDivisor();
    _vc->SetParameter3(i);
    _vcp->Refresh();
}

void ValueCurveDialog::OnSlider_Parameter3CmdSliderUpdated(wxScrollEvent& event)
{
    UpdateLinkedTextCtrl(event);
    int i = Slider_Parameter3->GetValue();
    _vc->SetParameter3(i);
    _vcp->Refresh();
}

void ValueCurveDialog::OnTextCtrl_Parameter4Text(wxCommandEvent& event)
{
    UpdateLinkedSlider(event);
    float i = wxAtof(TextCtrl_Parameter4->GetValue());
    float low, high;
    ValueCurve::GetRangeParm4(Choice1->GetStringSelection().ToStdString(), low, high);
    if (low == MINVOID)
        i *= _vc->GetDivisor();
    _vc->SetParameter4(i);
    _vcp->Refresh();
}

void ValueCurveDialog::OnSlider_Parameter4CmdSliderUpdated(wxScrollEvent& event)
{
    UpdateLinkedTextCtrl(event);
    int i = Slider_Parameter4->GetValue();
    _vc->SetParameter4(i);
    _vcp->Refresh();
}

#pragma endregion Sliders and TextCtrls

void ValueCurvePanel::DrawTiming(wxAutoBufferedPaintDC& pdc, long timeMS)
{
    wxSize s = GetSize();
    long interval = _end - _start;
    float pos = (float)(timeMS - _start) / (float)interval;
    int x = pos * s.GetWidth();

    pdc.SetPen(*wxBLUE);
    pdc.DrawLine(x, 0, x, s.GetHeight());
}

void ValueCurvePanel::DrawTiming(wxAutoBufferedPaintDC& pdc)
{
    if (_timingElement == nullptr) return;

    EffectLayer* el = _timingElement->GetEffectLayer(0);

    for (int i = 0; i < el->GetEffectCount(); i++)
    {
        Effect* e = el->GetEffect(i);
        if (e->GetStartTimeMS() >= _start || e->GetStartTimeMS() <= _end)
        {
            DrawTiming(pdc, e->GetStartTimeMS());
        }
        if (e->GetEndTimeMS() >= _start || e->GetEndTimeMS() <= _end)
        {
            DrawTiming(pdc, e->GetEndTimeMS());
        }
    }
}

void ValueCurvePanel::Paint(wxPaintEvent& event)
{
    //wxPaintDC pdc(this);
    wxAutoBufferedPaintDC pdc(this);
    pdc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK)));
    pdc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK)));
    wxSize size = GetSize();
    float w = size.GetWidth();
    float h = size.GetHeight();
    pdc.DrawRectangle(0, 0, size.GetWidth(), size.GetHeight());

    DrawTiming(pdc);

    pdc.SetBrush(*wxTRANSPARENT_BRUSH);
    if (_vc != nullptr)
    {
        pdc.SetPen(wxPen(*wxGREEN, 3, wxPENSTYLE_LONG_DASH));
        std::list<vcSortablePoint> pts = _vc->GetPoints();

        if (pts.size() > 1)
        {
            std::list<vcSortablePoint>::iterator last = pts.begin();
            for (auto p = pts.begin()++; p != pts.end(); ++p)
            {
                if (last->IsWrapped() == p->IsWrapped())
                {
                    pdc.DrawLine(last->x * w, h - last->y * h, p->x * w, h - p->y * h);
                }
                else
                {
                    pdc.DrawLine(last->x * w, h - p->y * h, p->x * w, h - p->y * h);
                }
                last = p;
            }
        }

        pdc.SetPen(wxPen(*wxRED, 2, wxPENSTYLE_SOLID));
        for (auto it = pts.begin(); it != pts.end(); ++it)
        {
            pdc.DrawRectangle((it->x * w) - 2, h - (it->y * h) - 2, 5, 5);
        }

        if (_grabbedPoint != -1)
        {
            pdc.SetPen(wxPen(*wxBLUE, 2, wxPENSTYLE_SOLID));
            pdc.DrawRectangle((_grabbedPoint * w) - 2, h - (_vc->GetValueAt(_grabbedPoint) * h) - 2, 5, 5);
        }
    }
}

void ValueCurveDialog::ValidateWindow()
{
    wxString type = Choice1->GetStringSelection();

    float min = 0;
    float max = 100;
    ValueCurve::GetRangeParm1(type.ToStdString(), min, max);
    if (min == MINVOID) min = _vc->GetMin();
    if (max == MAXVOID) max = _vc->GetMax();
    if (Slider_Parameter1->GetMin() != min || Slider_Parameter1->GetMax() != max) Slider_Parameter1->SetRange(min, max);

    ValueCurve::GetRangeParm2(type.ToStdString(), min, max);
    if (min == MINVOID) min = _vc->GetMin();
    if (max == MAXVOID) max = _vc->GetMax();
    if (Slider_Parameter2->GetMin() != min || Slider_Parameter2->GetMax() != max) Slider_Parameter2->SetRange(min, max);

    ValueCurve::GetRangeParm3(type.ToStdString(), min, max);
    if (min == MINVOID) min = _vc->GetMin();
    if (max == MAXVOID) max = _vc->GetMax();
    if (Slider_Parameter3->GetMin() != min || Slider_Parameter3->GetMax() != max) Slider_Parameter3->SetRange(min, max);

    ValueCurve::GetRangeParm4(type.ToStdString(), min, max);
    if (min == MINVOID) min = _vc->GetMin();
    if (max == MAXVOID) max = _vc->GetMax();
    if (Slider_Parameter4->GetMin() != min || Slider_Parameter4->GetMax() != max) Slider_Parameter4->SetRange(min, max);

    if (type == "Custom")
    {
        Slider_Parameter1->Disable();
        TextCtrl_Parameter1->Disable();
        Slider_Parameter2->Disable();
        TextCtrl_Parameter2->Disable();
        Slider_Parameter3->Disable();
        TextCtrl_Parameter3->Disable();
        Slider_Parameter4->Disable();
        TextCtrl_Parameter4->Disable();
    }
    else if (type == "Flat")
    {
        Slider_Parameter1->Enable();
        TextCtrl_Parameter1->Enable();
        Slider_Parameter2->Disable();
        TextCtrl_Parameter2->Disable();
        Slider_Parameter3->Disable();
        TextCtrl_Parameter3->Disable();
        Slider_Parameter4->Disable();
        TextCtrl_Parameter4->Disable();
    }
    else if (type == "Ramp" || type == "Parabolic Down" || type == "Parabolic Up" || type == "Logarithmic Up" || type == "Logarithmic Down" || type == "Exponential Up" || type == "Exponential Down")
    {
        Slider_Parameter1->Enable();
        TextCtrl_Parameter1->Enable();
        Slider_Parameter2->Enable();
        TextCtrl_Parameter2->Enable();
        Slider_Parameter3->Disable();
        TextCtrl_Parameter3->Disable();
        Slider_Parameter4->Disable();
        TextCtrl_Parameter4->Disable();
    }
    else if (type == "Saw Tooth" || type == "Ramp Up/Down Hold" || type == "Ramp Up/Down" || type == "Square")
    {
        Slider_Parameter1->Enable();
        TextCtrl_Parameter1->Enable();
        Slider_Parameter2->Enable();
        TextCtrl_Parameter2->Enable();
        Slider_Parameter3->Enable();
        TextCtrl_Parameter3->Enable();
        Slider_Parameter4->Disable();
        TextCtrl_Parameter4->Disable();
    }
    else
    {
        Slider_Parameter1->Enable();
        TextCtrl_Parameter1->Enable();
        Slider_Parameter2->Enable();
        TextCtrl_Parameter2->Enable();
        Slider_Parameter3->Enable();
        TextCtrl_Parameter3->Enable();
        Slider_Parameter4->Enable();
        TextCtrl_Parameter4->Enable();
    }

    if (type == "Flat")
    {
        StaticText_P1->SetLabel("Level");
        StaticText_P2->SetLabel("N/A");
        StaticText_P3->SetLabel("N/A");
        StaticText_P4->SetLabel("N/A");
        _vc->SetParameter2(0);
        _vc->SetParameter3(0);
        _vc->SetParameter4(0);
    }
    else if (type == "Ramp")
    {
        StaticText_P1->SetLabel("Start Level");
        StaticText_P2->SetLabel("End Level");
        StaticText_P3->SetLabel("N/A");
        StaticText_P4->SetLabel("N/A");
        _vc->SetParameter3(0);
        _vc->SetParameter4(0);
    }
    else if (type == "Ramp Up/Down")
    {
        StaticText_P1->SetLabel("Start Level");
        StaticText_P2->SetLabel("Mid Level");
        StaticText_P3->SetLabel("End Level");
        StaticText_P4->SetLabel("N/A");
        _vc->SetParameter4(0);
    }
    else if (type == "Ramp Up/Down Hold")
    {
        StaticText_P1->SetLabel("Start/End Level");
        StaticText_P2->SetLabel("Mid Level");
        StaticText_P3->SetLabel("Mid Level Time");
        StaticText_P4->SetLabel("N/A");
        _vc->SetParameter4(0);
    }
    else if (type == "Saw Tooth")
    {
        StaticText_P1->SetLabel("Start Level");
        StaticText_P2->SetLabel("End Level");
        StaticText_P3->SetLabel("Cycles");
        StaticText_P4->SetLabel("N/A");
        _vc->SetParameter4(0);
    }
    else if (type == "Parabolic Down")
    {
        StaticText_P1->SetLabel("Slope");
        StaticText_P2->SetLabel("Low");
        StaticText_P3->SetLabel("N/A");
        StaticText_P4->SetLabel("N/A");
        _vc->SetParameter3(0);
        _vc->SetParameter4(0);
    }
    else if (type == "Parabolic Up")
    {
        StaticText_P1->SetLabel("Slope");
        StaticText_P2->SetLabel("High");
        StaticText_P3->SetLabel("N/A");
        StaticText_P4->SetLabel("N/A");
        _vc->SetParameter3(0);
        _vc->SetParameter4(0);
    }
    else if (type == "Logarithmic Up")
    {
        StaticText_P1->SetLabel("Rate");
        StaticText_P2->SetLabel("Vertical Offset");
        StaticText_P3->SetLabel("N/A");
        StaticText_P4->SetLabel("N/A");
        _vc->SetParameter3(0);
        _vc->SetParameter4(0);
    }
    else if (type == "Logarithmic Down")
    {
        StaticText_P1->SetLabel("Rate");
        StaticText_P2->SetLabel("Vertical Offset");
        StaticText_P3->SetLabel("N/A");
        StaticText_P4->SetLabel("N/A");
        _vc->SetParameter3(0);
        _vc->SetParameter4(0);
    }
    else if (type == "Exponential Up" || type == "Exponential Down")
    {
        StaticText_P1->SetLabel("Rate");
        StaticText_P2->SetLabel("Vertical Offset");
        StaticText_P3->SetLabel("N/A");
        StaticText_P4->SetLabel("N/A");
        _vc->SetParameter3(0);
        _vc->SetParameter4(0);
    }
    else if (type == "Sine")
    {
        StaticText_P1->SetLabel("Start");
        StaticText_P2->SetLabel("Amplitude");
        StaticText_P3->SetLabel("Cycles");
        StaticText_P4->SetLabel("Vertical Offset");
    }
    else if (type == "Decaying Sine")
    {
        StaticText_P1->SetLabel("Start");
        StaticText_P2->SetLabel("Amplitude");
        StaticText_P3->SetLabel("Cycles");
        StaticText_P4->SetLabel("Vertical Offset");
    }
    else if (type == "Abs Sine")
    {
        StaticText_P1->SetLabel("Start");
        StaticText_P2->SetLabel("Amplitude");
        StaticText_P3->SetLabel("Cycles");
        StaticText_P4->SetLabel("Vertical Offset");
    }
    else if (type == "Square")
    {
        StaticText_P1->SetLabel("Start Level");
        StaticText_P2->SetLabel("End Level");
        StaticText_P3->SetLabel("Cycles");
        StaticText_P4->SetLabel("N/A");
        _vc->SetParameter4(0);
    }
    else if (type == "Custom")
    {
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
        type == "Exponential Down")
    {
        Button_Reverse->Enable(false);
    }
    else
    {
        Button_Reverse->Enable();
    }
}

void ValueCurveDialog::OnChar(wxKeyEvent& event)
{
    wxChar uc = event.GetUnicodeKey();
    if (_vc->GetType() == "Custom" && (int)uc == (int)WXK_DELETE && _vcp->HasSelected())
    {
        _vcp->SaveUndoSelected();
        _vcp->Delete();
    }
    else if (_vc->GetType() == "Custom" && (uc == 'Z' || uc == 'z') && event.ControlDown() && _vcp->IsDirty())
    {
        _vcp->Undo();
    }
    else
    {
        event.Skip();
    }
    Refresh();
}

void ValueCurveDialog::OnCheckBox_WrapValuesClick(wxCommandEvent& event)
{
    _vc->SetWrap(CheckBox_WrapValues->IsChecked());
    Refresh();
}

void ValueCurveDialog::OnResize(wxSizeEvent& event)
{
    OnSize(event);
    if (PresetSizer->GetChildren().GetCount() > 0)
    {
        int cols = (PresetSizer->GetSize().GetWidth() - 10) / 32;
        if (cols == 0) cols = 1;
        int rows = PresetSizer->GetChildren().GetCount() / cols + 1;
        PresetSizer->SetRows(std::max(1, rows));
        PresetSizer->SetCols(cols);
        PresetSizer->Layout();
    }
    Refresh();
}

void ValueCurveDialog::OnButtonLoadClick(wxCommandEvent& event)
{
    std::string id = _vc->GetId(); // save if because it will be overwritten

    wxString filename = wxFileSelector(_("Choose value curve file"), wxEmptyString, wxEmptyString, wxEmptyString, "Value Curve files (*.xvc)|*.xvc", wxFD_OPEN);
    if (filename.IsEmpty()) return;

    float min = _vc->GetMin();
    float max = _vc->GetMax();
    int div = _vc->GetDivisor();
    _vc->SetLimits(0, 100);
    _vc->SetDivisor(1);
    LoadXVC(_vc, filename);
    _vc->SetLimits(min, max);
    _vc->SetDivisor(div);
    SetSliderMinMax();
    _vc->FixChangedScale(min, max, 1);
    _vcp->Refresh();
    _vcp->ClearUndo();
    _vc->SetId(id);
    Choice1->SetStringSelection(wxString(_vc->GetType().c_str()));
    CheckBox_WrapValues->SetValue(_vc->GetWrap());
    SetParameter(1, _vc->GetParameter1());
    SetParameter(2, _vc->GetParameter2());
    SetParameter(3, _vc->GetParameter3());
    SetParameter(4, _vc->GetParameter4());
    SetTextCtrlsFromSliders();
    _vcp->SetType(_vc->GetType());
    ValidateWindow();
}

void ValueCurveDialog::OnButtonExportClick(wxCommandEvent& event)
{
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, "ValueCurve", wxEmptyString, "Value Curves (*.xvc)|*.xvc", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (filename.IsEmpty()) return;

    wxFile f(filename);
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("Saving to xvc file %s.", (const char *)filename.c_str());

    if (!f.Create(filename, true) || !f.IsOpened())
    {
        logger_base.info("Unable to create file %s. Error %d\n", (const char *)filename.c_str(), f.GetLastError());
        wxMessageBox(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()));
        return;
    }

    _vc->SetActive(true);

    wxString v = xlights_version_string;
    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<valuecurve \n");
    ValueCurve vc(_vc->Serialise());
    vc.SetId("ID_VALUECURVE_XVC");
    vc.SetLimits(0, 100);
    vc.UnFixChangedScale(_vc->GetMin(), _vc->GetMax());
    f.Write(wxString::Format("data=\"%s\" ", (const char *)vc.Serialise().c_str()));
    f.Write(wxString::Format("SourceVersion=\"%s\" ", v));
    f.Write(" >\n");
    f.Write("</valuecurve>");
    f.Close();

    _vcp->ClearUndo();

    PopulatePresets();
}

void ValueCurveDialog::ProcessPresetDir(wxDir& directory, bool subdirs)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("Scanning directory for *.xvc files: %s.", (const char *)directory.GetNameWithSep().c_str());

    wxString filename;
    auto existing = PresetSizer->GetChildren();

    bool cont = directory.GetFirst(&filename, "*.xvc", wxDIR_FILES);

    while (cont)
    {
        wxFileName fn(directory.GetNameWithSep() + filename);
        bool found = false;
        for (auto it : existing)
        {
            if (it->GetWindow()->GetLabel() == fn.GetFullPath())
            {
                // already there
                found = true;
                break;
            }
        }
        if (!found)
        {
            ValueCurve vc("");
            LoadXVC(&vc, fn.GetFullPath());
            long id = wxNewId();
            wxBitmapButton* bmb = new wxBitmapButton(this, id, vc.GetImage(30, 30, GetContentScaleFactor()), wxDefaultPosition,
                                                     wxSize(30, 30), wxBU_AUTODRAW | wxNO_BORDER);
            bmb->SetLabel(fn.GetFullPath());
            PresetSizer->Add(bmb);
            Connect(id, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ValueCurveDialog::OnButtonPresetClick);
        }

        cont = directory.GetNext(&filename);
    }

    if (subdirs)
    {
        cont = directory.GetFirst(&filename, "*", wxDIR_DIRS);
        while (cont)
        {
            wxDir dir(directory.GetNameWithSep() + filename);
            ProcessPresetDir(dir, subdirs);
            cont = directory.GetNext(&filename);
        }
    }
}

void ValueCurveDialog::PopulatePresets()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxDir dir(xLightsFrame::CurrentDir);

    ProcessPresetDir(dir, false);

    wxString d = xLightsFrame::CurrentDir + "/valuecurves";

    if (wxDir::Exists(d))
    {
        dir.Open(d);
        ProcessPresetDir(dir, true);
    }
    else
    {
        logger_base.info("Directory for *.xvc files not found: %s.", (const char *)d.c_str());
    }

    wxStandardPaths stdp = wxStandardPaths::Get();

#ifndef __WXMSW__
    d = wxStandardPaths::Get().GetResourcesDir() + "/valuecurves";
#else
    d = wxFileName(stdp.GetExecutablePath()).GetPath() + "/valuecurves";
#endif
    if (wxDir::Exists(d))
    {
        dir.Open(d);
        ProcessPresetDir(dir, true);
    }
    else
    {
        logger_base.info("Directory for *.xvc files not found: %s.", (const char *)d.c_str());
    }

    PresetSizer->Layout();
    Layout();
    //Fit();
}

void ValueCurveDialog::LoadXVC(ValueCurve* vc, const wxString& filename)
{
    wxXmlDocument doc(filename);

    if (doc.IsOk())
    {
        wxXmlNode* root = doc.GetRoot();

        if (root->GetName() == "valuecurve")
        {
            wxString data = root->GetAttribute("data");
            wxString v = root->GetAttribute("SourceVersion");

            // Add any valuecurve version conversion logic here
            // Source version will be the program version that created the custom model

            vc->Deserialise(data.ToStdString(), true);

            if (vc->GetId() == "ID_VALUECURVE_XVC")
            {
                // this should already have the 0-100 scale
            }
            else
            {
                // need to fudge it
                float min = vc->GetMin();
                float max = vc->GetMax();
                vc->SetLimits(0, 100);
                vc->FixChangedScale(min, max, 1);
            }

            vc->SetActive(true);
        }
        else
        {
            wxMessageBox("Failure loading value curve file " + filename + ".");
        }
    }
    else
    {
        wxMessageBox("Failure loading value curve file " + filename + ".");
    }
}

void ValueCurveDialog::OnButtonPresetClick(wxCommandEvent& event)
{
    if (_vcp->IsDirty())
    {
        if (wxMessageBox("Are you sure you want to discard your current value curve?", "Are you sure?", wxYES_NO | wxCENTER, this) == wxNO)
        {
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
    LoadXVC(_vc, filename);
    _vc->SetLimits(min, max);
    _vc->SetDivisor(div);
    SetSliderMinMax();
    _vc->FixChangedScale(min, max, 1);
    _vcp->Refresh();
    _vcp->ClearUndo();
    _vc->SetId(id);
    Choice1->SetStringSelection(wxString(_vc->GetType().c_str()));
    CheckBox_WrapValues->SetValue(_vc->GetWrap());
    SetParameter(1, _vc->GetParameter1());
    SetParameter(2, _vc->GetParameter2());
    SetParameter(3, _vc->GetParameter3());
    SetParameter(4, _vc->GetParameter4());
    SetTextCtrlsFromSliders();
    _vcp->SetType(_vc->GetType());
    ValidateWindow();
}

void ValueCurveDialog::OnButton_ReverseClick(wxCommandEvent& event)
{
    _vc->Reverse();
    Choice1->SetStringSelection(wxString(_vc->GetType().c_str()));
    SetParameter(1, _vc->GetParameter1());
    SetParameter(2, _vc->GetParameter2());
    SetParameter(3, _vc->GetParameter3());
    SetParameter(4, _vc->GetParameter4());
    SetTextCtrlsFromSliders();
    _vcp->Refresh();
    _vcp->SetType(_vc->GetType());
    _vcp->ClearUndo();
    ValidateWindow();
}
