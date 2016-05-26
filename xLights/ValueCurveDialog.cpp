#include "ValueCurveDialog.h"

//(*InternalHeaders(ValueCurveDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/dcbuffer.h>
#include <wx/valnum.h>
#include <wx/settings.h>
#include "ValueCurve.h"
#include <log4cpp/Category.hh>

BEGIN_EVENT_TABLE(ValueCurvePanel, wxWindow)
EVT_MOTION(ValueCurvePanel::mouseMoved)
EVT_LEFT_DOWN(ValueCurvePanel::mouseLeftDown)
EVT_LEFT_UP(ValueCurvePanel::mouseLeftUp)
EVT_PAINT(ValueCurvePanel::Paint)
EVT_MOUSE_CAPTURE_LOST(ValueCurvePanel::mouseCaptureLost)
END_EVENT_TABLE()

ValueCurvePanel::ValueCurvePanel(wxWindow* parent, wxWindowID id, const wxPoint &pos, const wxSize &size, long style)
    : wxWindow(parent, id, pos, size, style, "ID_VCP"), xlCustomControl()
{
    Connect(wxEVT_LEFT_DOWN, (wxObjectEventFunction)&ValueCurvePanel::mouseLeftDown, 0, this);
    Connect(wxEVT_LEFT_UP, (wxObjectEventFunction)&ValueCurvePanel::mouseLeftUp, 0, this);
    Connect(wxEVT_MOTION, (wxObjectEventFunction)&ValueCurvePanel::mouseMoved, 0, this);
    Connect(wxEVT_PAINT, (wxObjectEventFunction)&ValueCurvePanel::Paint, 0, this);
    Connect(wxEVT_MOUSE_CAPTURE_LOST, (wxObjectEventFunction)&ValueCurvePanel::mouseCaptureLost, 0, this);
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    _grabbedPoint = -1;
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
const long ValueCurveDialog::IDD_SLIDER_Parameter1 = wxNewId();
const long ValueCurveDialog::ID_TEXTCTRL_Parameter1 = wxNewId();
const long ValueCurveDialog::ID_STATICTEXT2 = wxNewId();
const long ValueCurveDialog::IDD_SLIDER_Parameter2 = wxNewId();
const long ValueCurveDialog::ID_TEXTCTRL_Parameter2 = wxNewId();
const long ValueCurveDialog::ID_STATICTEXT5 = wxNewId();
const long ValueCurveDialog::IDD_SLIDER_Parameter3 = wxNewId();
const long ValueCurveDialog::ID_TEXTCTRL_Parameter3 = wxNewId();
const long ValueCurveDialog::ID_STATICTEXT6 = wxNewId();
const long ValueCurveDialog::ID_SLIDER_Parameter4 = wxNewId();
const long ValueCurveDialog::ID_TEXTCTRL_Parameter4 = wxNewId();
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

    __p1 = 0;
    __p2 = 0;
    __p3 = 0;
    __p4 = 0;
    wxIntegerValidator<int> _p1validator(&__p1, wxNUM_VAL_THOUSANDS_SEPARATOR);
    _p1validator.SetMin(0);
    _p1validator.SetMax(100);
    wxIntegerValidator<int> _p2validator(&__p2, wxNUM_VAL_THOUSANDS_SEPARATOR);
    _p2validator.SetMin(0);
    _p2validator.SetMax(100);
    wxIntegerValidator<int> _p3validator(&__p3, wxNUM_VAL_THOUSANDS_SEPARATOR);
    _p3validator.SetMin(0);
    _p3validator.SetMax(100);
    wxIntegerValidator<int> _p4validator(&__p4, wxNUM_VAL_THOUSANDS_SEPARATOR);
    _p4validator.SetMin(0);
    _p4validator.SetMax(100);

    //(*Initialize(ValueCurveDialog)
    wxFlexGridSizer* FlexGridSizer4;
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer5;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer6;
    wxFlexGridSizer* FlexGridSizer1;

    Create(parent, id, _("Value Curve"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("id"));
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
    StaticText_TopValue = new wxStaticText(this, ID_STATICTEXT3, _("100"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer5->Add(StaticText_TopValue, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_BottomValue = new wxStaticText(this, ID_STATICTEXT4, _("0"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    FlexGridSizer5->Add(StaticText_BottomValue, 1, wxALL|wxALIGN_RIGHT|wxALIGN_BOTTOM, 2);
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
    Choice1->Append(_("Custom"));
    FlexGridSizer2->Add(Choice1, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_P1 = new wxStaticText(this, ID_STATICTEXT1, _("XXXXXXXXXXXXXXX"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer2->Add(StaticText_P1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Parameter1 = new wxSlider(this, IDD_SLIDER_Parameter1, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Parameter1"));
    FlexGridSizer2->Add(Slider_Parameter1, 1, wxALL|wxEXPAND, 2);
    TextCtrl_Parameter1 = new wxTextCtrl(this, ID_TEXTCTRL_Parameter1, _("0"), wxDefaultPosition, wxSize(40,24), 0, _p1validator, _T("ID_TEXTCTRL_Parameter1"));
    FlexGridSizer2->Add(TextCtrl_Parameter1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText_P2 = new wxStaticText(this, ID_STATICTEXT2, _("Parameter 2"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer2->Add(StaticText_P2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Parameter2 = new wxSlider(this, IDD_SLIDER_Parameter2, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Parameter2"));
    FlexGridSizer2->Add(Slider_Parameter2, 1, wxALL|wxEXPAND, 2);
    TextCtrl_Parameter2 = new wxTextCtrl(this, ID_TEXTCTRL_Parameter2, _("0"), wxDefaultPosition, wxSize(40,24), 0, _p2validator, _T("ID_TEXTCTRL_Parameter2"));
    FlexGridSizer2->Add(TextCtrl_Parameter2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText_P3 = new wxStaticText(this, ID_STATICTEXT5, _("Parameter 3"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
    FlexGridSizer2->Add(StaticText_P3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Parameter3 = new wxSlider(this, IDD_SLIDER_Parameter3, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Parameter3"));
    FlexGridSizer2->Add(Slider_Parameter3, 1, wxALL|wxEXPAND, 2);
    TextCtrl_Parameter3 = new wxTextCtrl(this, ID_TEXTCTRL_Parameter3, _("0"), wxDefaultPosition, wxSize(40,24), 0, _p3validator, _T("ID_TEXTCTRL_Parameter3"));
    FlexGridSizer2->Add(TextCtrl_Parameter3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText_P4 = new wxStaticText(this, ID_STATICTEXT6, _("Label"), wxDefaultPosition, wxSize(121,16), 0, _T("ID_STATICTEXT6"));
    FlexGridSizer2->Add(StaticText_P4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Parameter4 = new wxSlider(this, ID_SLIDER_Parameter4, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Parameter4"));
    FlexGridSizer2->Add(Slider_Parameter4, 1, wxALL|wxEXPAND, 2);
    TextCtrl_Parameter4 = new wxTextCtrl(this, ID_TEXTCTRL_Parameter4, _("0"), wxDefaultPosition, wxSize(40,24), 0, _p4validator, _T("ID_TEXTCTRL_Parameter4"));
    FlexGridSizer2->Add(TextCtrl_Parameter4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
    Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer3->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    FlexGridSizer3->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&ValueCurveDialog::OnChoice1Select);
    Connect(IDD_SLIDER_Parameter1,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&ValueCurveDialog::OnSlider_Parameter1CmdSliderUpdated);
    Connect(ID_TEXTCTRL_Parameter1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ValueCurveDialog::OnTextCtrl_Parameter1Text);
    Connect(IDD_SLIDER_Parameter2,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&ValueCurveDialog::OnSlider_Parameter2CmdSliderUpdated);
    Connect(ID_TEXTCTRL_Parameter2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ValueCurveDialog::OnTextCtrl_Parameter2Text);
    Connect(IDD_SLIDER_Parameter3,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&ValueCurveDialog::OnSlider_Parameter3CmdSliderUpdated);
    Connect(ID_TEXTCTRL_Parameter3,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ValueCurveDialog::OnTextCtrl_Parameter3Text);
    Connect(ID_SLIDER_Parameter4,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&ValueCurveDialog::OnSlider_Parameter4CmdSliderUpdated);
    Connect(ID_TEXTCTRL_Parameter4,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ValueCurveDialog::OnTextCtrl_Parameter4Text);
    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ValueCurveDialog::OnButton_OkClick);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ValueCurveDialog::OnButton_CancelClick);
    //*)

    Connect(wxID_ANY, wxEVT_CHAR_HOOK, wxKeyEventHandler(ValueCurveDialog::OnChar), (wxObject*)NULL, this);

    _vcp = new ValueCurvePanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER);
    _vcp->SetMinSize(wxSize(200, 100));
    _vcp->SetValue(_vc);
    FlexGridSizer4->Add(_vcp, 1, wxALL | wxEXPAND, 2);
    Layout();
    Fit();

    _backup = *_vc;

    Slider_Parameter1->SetValue((int)_vc->GetParameter1());
    Slider_Parameter2->SetValue((int)_vc->GetParameter2());
    Slider_Parameter3->SetValue((int)_vc->GetParameter3());
    Slider_Parameter4->SetValue((int)_vc->GetParameter4());
    TextCtrl_Parameter1->SetValue(wxString::Format("%d", (int)_vc->GetParameter1()));
    TextCtrl_Parameter2->SetValue(wxString::Format("%d", (int)_vc->GetParameter2()));
    TextCtrl_Parameter3->SetValue(wxString::Format("%d", (int)_vc->GetParameter3()));
    TextCtrl_Parameter4->SetValue(wxString::Format("%d", (int)_vc->GetParameter4()));
    Choice1->SetStringSelection(wxString(_vc->GetType().c_str()));
    Choice1->SetFocus();
    ValidateWindow();
}

ValueCurveDialog::~ValueCurveDialog()
{
	//(*Destroy(ValueCurveDialog)
	//*)
}


void ValueCurveDialog::OnButton_OkClick(wxCommandEvent& event)
{
    EndDialog(wxOK);
}

void ValueCurveDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    *_vc = _backup;
    EndDialog(wxCANCEL);
}

void ValueCurveDialog::OnChoice1Select(wxCommandEvent& event)
{
    _vcp->SetType(std::string(Choice1->GetStringSelection().c_str()));
    _vc->SetType(std::string(Choice1->GetStringSelection().c_str()));
    _vcp->Refresh();
    ValidateWindow();
}

#pragma region Mouse Control

float NormaliseGrabbedPoint(float g)
{
    return std::round(g * 40.0) / 40.0;
}

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
        _grabbedPoint = NormaliseGrabbedPoint(_grabbedPoint);
        SaveUndoSelected();
        CaptureMouse();
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

void ValueCurvePanel::mouseMoved(wxMouseEvent& event)
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
        Refresh();
    }
}
#pragma endregion Mouse Control

#pragma region Sliders and TextCtrls
void ValueCurveDialog::OnTextCtrl_Parameter1Text(wxCommandEvent& event)
{
    int i = wxAtoi(TextCtrl_Parameter1->GetValue());
    if (i != (int)_vc->GetParameter1())
    {
        _vc->SetParameter1(i);
        Slider_Parameter1->SetValue(_vc->GetParameter1());
        _vcp->Refresh();
    }
}
void ValueCurveDialog::OnSlider_Parameter1CmdSliderUpdated(wxScrollEvent& event)
{
    int i = Slider_Parameter1->GetValue();
    if (i != (int)_vc->GetParameter1())
    {
        _vc->SetParameter1(i);
        TextCtrl_Parameter1->SetValue(wxString::Format("%d", (int)_vc->GetParameter1()));
        _vcp->Refresh();
    }
}

void ValueCurveDialog::OnTextCtrl_Parameter2Text(wxCommandEvent& event)
{
    int i = wxAtoi(TextCtrl_Parameter2->GetValue());
    if (i != (int)_vc->GetParameter2())
    {
        _vc->SetParameter1(i);
        Slider_Parameter2->SetValue(_vc->GetParameter2());
        _vcp->Refresh();
    }
}

void ValueCurveDialog::OnSlider_Parameter2CmdSliderUpdated(wxScrollEvent& event)
{
    int i = Slider_Parameter2->GetValue();
    if (i != (int)_vc->GetParameter2())
    {
        _vc->SetParameter2(i);
        TextCtrl_Parameter2->SetValue(wxString::Format("%d", (int)_vc->GetParameter2()));
        _vcp->Refresh();
    }
}

void ValueCurveDialog::OnSlider_Parameter3CmdSliderUpdated(wxScrollEvent& event)
{
    int i = Slider_Parameter3->GetValue();
    if (i != (int)_vc->GetParameter3())
    {
        _vc->SetParameter3(i);
        TextCtrl_Parameter3->SetValue(wxString::Format("%d", (int)_vc->GetParameter3()));
        _vcp->Refresh();
    }
}

void ValueCurveDialog::OnTextCtrl_Parameter3Text(wxCommandEvent& event)
{
    int i = wxAtoi(TextCtrl_Parameter3->GetValue());
    if (i != (int)_vc->GetParameter3())
    {
        _vc->SetParameter3(i);
        Slider_Parameter3->SetValue(_vc->GetParameter3());
        _vcp->Refresh();
    }
}
void ValueCurveDialog::OnSlider_Parameter4CmdSliderUpdated(wxScrollEvent& event)
{
    int i = Slider_Parameter4->GetValue();
    if (i != (int)_vc->GetParameter4())
    {
        _vc->SetParameter4(i);
        TextCtrl_Parameter4->SetValue(wxString::Format("%d", (int)_vc->GetParameter4()));
        _vcp->Refresh();
    }
}

void ValueCurveDialog::OnTextCtrl_Parameter4Text(wxCommandEvent& event)
{
    int i = wxAtoi(TextCtrl_Parameter4->GetValue());
    if (i != (int)_vc->GetParameter4())
    {
        _vc->SetParameter4(i);
        Slider_Parameter4->SetValue(_vc->GetParameter4());
        _vcp->Refresh();
    }
}
#pragma endregion Sliders and TextCtrls

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

    pdc.SetBrush(*wxTRANSPARENT_BRUSH);
    if (_vc != NULL)
    {
        pdc.SetPen(wxPen(*wxGREEN, 3, wxPENSTYLE_LONG_DASH));
        std::list<vcSortablePoint> pts = _vc->GetPoints();

        if (pts.size() > 1)
        {
            std::list<vcSortablePoint>::iterator last = pts.begin();
            for (auto p = pts.begin()++; p != pts.end(); p++)
            {
                pdc.DrawLine(last->x * w, h - last->y * h, p->x * w, h - p->y * h);
                last = p;
            }
        }
        pdc.SetPen(wxPen(*wxRED, 2, wxPENSTYLE_SOLID));
        for (auto it = pts.begin(); it != pts.end(); it++)
        {
            pdc.DrawRectangle((it->x * w) - 2, h - (it->y * h) - 2, 5, 5);
        }
        if (_grabbedPoint >= 0)
        {
            pdc.SetPen(wxPen(*wxBLUE, 2, wxPENSTYLE_SOLID));
            pdc.DrawRectangle((_grabbedPoint * w) - 2, h - (_vc->GetValueAt(_grabbedPoint) * h) - 2, 5, 5);
        }
    }
}

void ValueCurveDialog::ValidateWindow()
{
    wxString type = Choice1->GetStringSelection();

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
    else if (type == "Saw Tooth" || type == "Ramp Up/Down Hold" || type == "Ramp Up/Down")
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
    }
    else if (type == "Ramp")
    {
        StaticText_P1->SetLabel("Start Level");
        StaticText_P2->SetLabel("End Level");
        StaticText_P3->SetLabel("N/A");
        StaticText_P4->SetLabel("N/A");
    }
    else if (type == "Ramp Up/Down")
    {
        StaticText_P1->SetLabel("Start Level");
        StaticText_P2->SetLabel("Mid Level");
        StaticText_P3->SetLabel("End Level");
        StaticText_P4->SetLabel("N/A");
    }
    else if (type == "Ramp Up/Down Hold")
    {
        StaticText_P1->SetLabel("Start/End Level");
        StaticText_P2->SetLabel("Mid Level");
        StaticText_P3->SetLabel("Mid Level Time");
        StaticText_P4->SetLabel("N/A");
    }
    else if (type == "Saw Tooth")
    {
        StaticText_P1->SetLabel("Start Level");
        StaticText_P2->SetLabel("End Level");
        StaticText_P3->SetLabel("Cycles");
        StaticText_P4->SetLabel("N/A");
    }
    else if (type == "Parabolic Down")
    {
        StaticText_P1->SetLabel("Slope");
        StaticText_P2->SetLabel("Low");
        StaticText_P3->SetLabel("N/A");
        StaticText_P4->SetLabel("N/A");
    }
    else if (type == "Parabolic Up")
    {
        StaticText_P1->SetLabel("Slope");
        StaticText_P2->SetLabel("High");
        StaticText_P3->SetLabel("N/A");
        StaticText_P4->SetLabel("N/A");
    }
    else if (type == "Logarithmic Up")
    {
        StaticText_P1->SetLabel("Rate");
        StaticText_P2->SetLabel("Vertical Offset");
        StaticText_P3->SetLabel("N/A");
        StaticText_P4->SetLabel("N/A");
    }
    else if (type == "Logarithmic Down")
    {
        StaticText_P1->SetLabel("Rate");
        StaticText_P2->SetLabel("Vertical Offset");
        StaticText_P3->SetLabel("N/A");
        StaticText_P4->SetLabel("N/A");
    }
    else if (type == "Exponential Up" || type == "Exponential Down")
    {
        StaticText_P1->SetLabel("Rate");
        StaticText_P2->SetLabel("Vertical Offset");
        StaticText_P3->SetLabel("N/A");
        StaticText_P4->SetLabel("N/A");
    }
    else if (type == "Sine")
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
    else if (type == "Custom")
    {
        StaticText_P1->SetLabel("N/A");
        StaticText_P2->SetLabel("N/A");
        StaticText_P3->SetLabel("N/A");
        StaticText_P4->SetLabel("N/A");
    }
}

void ValueCurveDialog::OnChar(wxKeyEvent& event)
{
    wxChar uc = event.GetUnicodeKey();
    if (uc == WXK_DELETE)
    {
        _vcp->SaveUndoSelected();
        _vcp->Delete();
    }
    else if ((uc == 'Z' || uc == 'z') && event.ControlDown())
    {
        _vcp->Undo();
    }
    Refresh();
}
