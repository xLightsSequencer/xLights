#include "ValueCurveDialog.h"

//(*InternalHeaders(ValueCurveDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/dcclient.h>
#include <wx/valnum.h>
#include "ValueCurve.h"

//(*IdInit(ValueCurveDialog)
const long ValueCurveDialog::ID_STATICTEXT3 = wxNewId();
const long ValueCurveDialog::ID_STATICTEXT4 = wxNewId();
const long ValueCurveDialog::ID_PANEL1 = wxNewId();
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
    _grabbedPoint = -1;
    wxIntegerValidator<int> _p1validator(&__p1, wxNUM_VAL_THOUSANDS_SEPARATOR);
    _p1validator.SetMin(0);
    _p1validator.SetMax(100);
    wxIntegerValidator<int> _p2validator(&__p2, wxNUM_VAL_THOUSANDS_SEPARATOR);
    _p2validator.SetMin(0);
    _p2validator.SetMax(100);
    wxIntegerValidator<int> _p3validator(&__p3, wxNUM_VAL_THOUSANDS_SEPARATOR);
    _p3validator.SetMin(0);
    _p3validator.SetMax(100);

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
    Panel_Graph = new wxPanel(this, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER|wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    Panel_Graph->SetMinSize(wxSize(200,100));
    FlexGridSizer4->Add(Panel_Graph, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer6->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer1->Add(FlexGridSizer6, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer2->AddGrowableCol(1);
    FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Choice1 = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
    Choice1->SetSelection( Choice1->Append(_("Flat")) );
    Choice1->Append(_("Ramp"));
    Choice1->Append(_("Ramp Up/Down"));
    Choice1->Append(_("Saw Tooth"));
    Choice1->Append(_("Parabolic Down"));
    Choice1->Append(_("Parabolic Up"));
    Choice1->Append(_("Logarithmic Up"));
    Choice1->Append(_("Logarithmic Down"));
    Choice1->Append(_("Custom"));
    FlexGridSizer2->Add(Choice1, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Parameter 1"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Parameter1 = new wxSlider(this, IDD_SLIDER_Parameter1, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Parameter1"));
    FlexGridSizer2->Add(Slider_Parameter1, 1, wxALL|wxEXPAND, 2);
    TextCtrl_Parameter1 = new wxTextCtrl(this, ID_TEXTCTRL_Parameter1, _("0"), wxDefaultPosition, wxSize(40,24), 0, _p1validator, _T("ID_TEXTCTRL_Parameter1"));
    FlexGridSizer2->Add(TextCtrl_Parameter1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Parameter 2"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Parameter2 = new wxSlider(this, IDD_SLIDER_Parameter2, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Parameter2"));
    FlexGridSizer2->Add(Slider_Parameter2, 1, wxALL|wxEXPAND, 2);
    TextCtrl_Parameter2 = new wxTextCtrl(this, ID_TEXTCTRL_Parameter2, _("0"), wxDefaultPosition, wxSize(40,24), 0, _p2validator, _T("ID_TEXTCTRL_Parameter2"));
    FlexGridSizer2->Add(TextCtrl_Parameter2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText3 = new wxStaticText(this, ID_STATICTEXT5, _("Parameter 3"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
    FlexGridSizer2->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Parameter3 = new wxSlider(this, IDD_SLIDER_Parameter3, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_Parameter3"));
    FlexGridSizer2->Add(Slider_Parameter3, 1, wxALL|wxEXPAND, 2);
    TextCtrl_Parameter3 = new wxTextCtrl(this, ID_TEXTCTRL_Parameter3, _("0"), wxDefaultPosition, wxSize(40,24), 0, _p3validator, _T("ID_TEXTCTRL_Parameter3"));
    FlexGridSizer2->Add(TextCtrl_Parameter3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
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

    Panel_Graph->Connect(wxEVT_PAINT,(wxObjectEventFunction)&ValueCurveDialog::OnPanel_GraphPaint,0,this);
    Panel_Graph->Connect(wxEVT_LEFT_DOWN,(wxObjectEventFunction)&ValueCurveDialog::OnPanel_GraphLeftDown,0,this);
    Panel_Graph->Connect(wxEVT_LEFT_UP,(wxObjectEventFunction)&ValueCurveDialog::OnPanel_GraphLeftUp,0,this);
    Panel_Graph->Connect(wxEVT_MOTION,(wxObjectEventFunction)&ValueCurveDialog::OnPanel_GraphMouseMove,0,this);
    Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&ValueCurveDialog::OnChoice1Select);
    Connect(IDD_SLIDER_Parameter1,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&ValueCurveDialog::OnSlider_Parameter1CmdSliderUpdated);
    Connect(ID_TEXTCTRL_Parameter1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ValueCurveDialog::OnTextCtrl_Parameter1Text);
    Connect(IDD_SLIDER_Parameter2,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&ValueCurveDialog::OnSlider_Parameter2CmdSliderUpdated);
    Connect(ID_TEXTCTRL_Parameter2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ValueCurveDialog::OnTextCtrl_Parameter2Text);
    Connect(IDD_SLIDER_Parameter3,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&ValueCurveDialog::OnSlider_Parameter2CmdSliderUpdated);
    Connect(ID_TEXTCTRL_Parameter3,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ValueCurveDialog::OnTextCtrl_Parameter2Text);
    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ValueCurveDialog::OnButton_OkClick);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ValueCurveDialog::OnButton_CancelClick);
    //*)

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
    EndDialog(wxCANCEL);
}

void ValueCurveDialog::OnChoice1Select(wxCommandEvent& event)
{
    ValidateWindow();
}

#pragma region Mouse Control
void ValueCurveDialog::OnPanel_GraphLeftDown(wxMouseEvent& event)
{
    if (Choice1->GetStringSelection() == "Custom")
    {
        _grabbedPoint = Panel_Graph->GetSize().GetWidth() / event.GetX();
        if (_grabbedPoint < 0.0f)
        {
            _grabbedPoint = 0.0f;
        }
        else if (_grabbedPoint > 1.0f)
        {
            _grabbedPoint = 1.0f;
        }
        CaptureMouse();
    }
}
void ValueCurveDialog::OnPanel_GraphLeftUp(wxMouseEvent& event)
{
    if (Choice1->GetStringSelection() == "Custom")
    {
        _grabbedPoint = -1;
        ReleaseMouse();
    }
}

void ValueCurveDialog::OnPanel_GraphMouseMove(wxMouseEvent& event)
{
    if (Choice1->GetStringSelection() == "Custom" && _grabbedPoint >= 0)
    {
        float y = 0.0f;
        if (event.GetY() > 0)
        {
            y = Panel_Graph->GetSize().GetHeight() / event.GetY();
            if (y < 0.0f)
            {
                y = 0.0f;
            }
            else if (y > 1.0f)
            {
                y = 1.0f;
            }
        }

        _vc->SetValueAt(_grabbedPoint, y);
    }
}
#pragma endregion Mouse Control

#pragma region Sliders and TextCtrls
void ValueCurveDialog::OnTextCtrl_Parameter1Text(wxCommandEvent& event)
{
    Slider_Parameter1->SetValue(wxAtoi(TextCtrl_Parameter1->GetValue()));
}

void ValueCurveDialog::OnTextCtrl_Parameter2Text(wxCommandEvent& event)
{
    Slider_Parameter2->SetValue(wxAtoi(TextCtrl_Parameter2->GetValue()));
}

void ValueCurveDialog::OnSlider_Parameter1CmdSliderUpdated(wxScrollEvent& event)
{
    TextCtrl_Parameter1->SetValue(wxString::Format("%d", Slider_Parameter1->GetValue()));
}

void ValueCurveDialog::OnSlider_Parameter2CmdSliderUpdated(wxScrollEvent& event)
{
    TextCtrl_Parameter2->SetValue(wxString::Format("%d", Slider_Parameter2->GetValue()));
}
#pragma endregion Sliders and TextCtrls

void ValueCurveDialog::OnPanel_GraphPaint(wxPaintEvent& event)
{
    wxPaintDC pdc(this);
    pdc.Clear();
    pdc.SetPen(wxPen(*wxBLACK, 2, wxPENSTYLE_LONG_DASH));
    float p1y = _vc->GetValueAt(0);
    float p1x = 0.0f;
    int w = Panel_Graph->GetSize().GetWidth();
    int h = Panel_Graph->GetSize().GetHeight();
    for (float p = 1.0f / (float)w; p <= 1.0f; p += 1.0f / (float)Panel_Graph->GetSize().GetWidth())
    {
        float p2y = _vc->GetValueAt(p);
        pdc.DrawLine(p1x * w, p1y * h, p * w, p2y * h);
        p1x = p;
        p1y = p2y;
    }
    std::list<vcSortablePoint> pts = _vc->GetPoints();
    pdc.SetPen(wxPen(*wxRED, 2, wxPENSTYLE_SOLID));
    for (auto it = pts.begin(); it != pts.end(); it++)
    {
        pdc.DrawRectangle(it->x - 2, it->y - 2, 5, 5);
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
    }
    else if (type == "Flat" || type == "Logarithmic Up" || type == "Logarithmic Down")
    {
        Slider_Parameter1->Enable();
        TextCtrl_Parameter1->Enable();
        Slider_Parameter2->Disable();
        TextCtrl_Parameter2->Disable();
        Slider_Parameter3->Disable();
        TextCtrl_Parameter3->Disable();
    }
    else if (type == "Ramp" || type == "Parabolic Down")
    {
        Slider_Parameter1->Enable();
        TextCtrl_Parameter1->Enable();
        Slider_Parameter2->Enable();
        TextCtrl_Parameter2->Enable();
        Slider_Parameter3->Disable();
        TextCtrl_Parameter3->Disable();
    }
    else
    {
        Slider_Parameter1->Enable();
        TextCtrl_Parameter1->Enable();
        Slider_Parameter2->Enable();
        TextCtrl_Parameter2->Enable();
        Slider_Parameter3->Enable();
        TextCtrl_Parameter3->Enable();
    }
}
