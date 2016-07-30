#include "ColorCurveDialog.h"

//(*InternalHeaders(ColorCurveDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/dcbuffer.h>
#include <wx/valnum.h>
#include "ValueCurve.h"
#include <log4cpp/Category.hh>
#include <wx/colordlg.h>

BEGIN_EVENT_TABLE(ColorCurvePanel, wxWindow)
EVT_MOTION(ColorCurvePanel::mouseMoved)
EVT_LEFT_DOWN(ColorCurvePanel::mouseLeftDown)
EVT_LEFT_UP(ColorCurvePanel::mouseLeftUp)
EVT_ENTER_WINDOW(ColorCurvePanel::mouseEnter)
EVT_LEAVE_WINDOW(ColorCurvePanel::mouseLeave)
EVT_PAINT(ColorCurvePanel::Paint)
EVT_MOUSE_CAPTURE_LOST(ColorCurvePanel::mouseCaptureLost)
END_EVENT_TABLE()

ColorCurvePanel::ColorCurvePanel(wxWindow* parent, wxWindowID id, const wxPoint &pos, const wxSize &size, long style)
    : wxWindow(parent, id, pos, size, style, "ID_VCP"), xlCustomControl()
{
    Connect(wxEVT_LEFT_DOWN, (wxObjectEventFunction)&ColorCurvePanel::mouseLeftDown, 0, this);
    Connect(wxEVT_LEFT_UP, (wxObjectEventFunction)&ColorCurvePanel::mouseLeftUp, 0, this);
    Connect(wxEVT_ENTER_WINDOW, (wxObjectEventFunction)&ColorCurvePanel::mouseEnter, 0, this);
    Connect(wxEVT_LEAVE_WINDOW, (wxObjectEventFunction)&ColorCurvePanel::mouseLeave, 0, this);
    Connect(wxEVT_MOTION, (wxObjectEventFunction)&ColorCurvePanel::mouseMoved, 0, this);
    Connect(wxEVT_PAINT, (wxObjectEventFunction)&ColorCurvePanel::Paint, 0, this);
    Connect(wxEVT_MOUSE_CAPTURE_LOST, (wxObjectEventFunction)&ColorCurvePanel::mouseCaptureLost, 0, this);
    wxWindowBase::SetBackgroundStyle(wxBG_STYLE_PAINT);
    _grabbedPoint = -1;
}

void ColorCurvePanel::Select(float x)
{
    _grabbedPoint = x;
    Refresh();
}

void ColorCurvePanel::Convert(float &x, wxMouseEvent& event) const
{
    wxSize size = GetSize();
    float startX = 0.0; // size.GetWidth() / 10.0;
    float bw = size.GetWidth(); //  *0.8;

    x = (event.GetX() - startX) / bw;
}

//(*IdInit(ColorCurveDialog)
const long ColorCurveDialog::ID_STATICTEXT1 = wxNewId();
const long ColorCurveDialog::ID_CHOICE1 = wxNewId();
const long ColorCurveDialog::ID_GRID1 = wxNewId();
const long ColorCurveDialog::ID_BUTTON1 = wxNewId();
const long ColorCurveDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(ColorCurveDialog,wxDialog)
	//(*EventTable(ColorCurveDialog)
	//*)
END_EVENT_TABLE()

ColorCurveDialog::ColorCurveDialog(wxWindow* parent, ColorCurve* cc, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _cc = cc;

    //(*Initialize(ColorCurveDialog)
    wxFlexGridSizer* FlexGridSizer4;
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer6;
    wxFlexGridSizer* FlexGridSizer1;

    Create(parent, id, _("Value Curve"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("id"));
    SetClientSize(wxDefaultSize);
    Move(wxDefaultPosition);
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer1->AddGrowableRow(0);
    FlexGridSizer6 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer6->AddGrowableCol(0);
    FlexGridSizer6->AddGrowableRow(0);
    FlexGridSizer1->Add(FlexGridSizer6, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer2->AddGrowableCol(0);
    FlexGridSizer2->AddGrowableRow(1);
    FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer4->AddGrowableCol(1);
    StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Blend Mode"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer4->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Choice1 = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
    Choice1->SetSelection( Choice1->Append(_("None")) );
    Choice1->Append(_("Gradient"));
    FlexGridSizer4->Add(Choice1, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer2->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 5);
    Grid1 = new wxGrid(this, ID_GRID1, wxDefaultPosition, wxDefaultSize, wxVSCROLL, _T("ID_GRID1"));
    Grid1->CreateGrid(1,2);
    Grid1->EnableEditing(true);
    Grid1->EnableGridLines(true);
    Grid1->SetColLabelValue(0, _("Position"));
    Grid1->SetColLabelValue(1, _("Color"));
    Grid1->SetDefaultCellFont( Grid1->GetFont() );
    Grid1->SetDefaultCellTextColour( Grid1->GetForegroundColour() );
    FlexGridSizer2->Add(Grid1, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
    Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer3->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    FlexGridSizer3->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizer1);
    ColourDialog1 = new wxColourDialog(this);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&ColorCurveDialog::OnChoice1Select);
    Connect(ID_GRID1,wxEVT_GRID_CELL_LEFT_CLICK,(wxObjectEventFunction)&ColorCurveDialog::OnGrid1CellLeftClick);
    Connect(ID_GRID1,wxEVT_GRID_CELL_LEFT_DCLICK,(wxObjectEventFunction)&ColorCurveDialog::OnGrid1CellLeftDClick);
    Connect(ID_GRID1,wxEVT_GRID_SELECT_CELL,(wxObjectEventFunction)&ColorCurveDialog::OnGrid1CellSelect);
    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorCurveDialog::OnButton_OkClick);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ColorCurveDialog::OnButton_CancelClick);
    Connect(wxEVT_SIZE,(wxObjectEventFunction)&ColorCurveDialog::OnResize);
    //*)

    Connect(wxID_ANY, wxEVT_CHAR_HOOK, wxKeyEventHandler(ColorCurveDialog::OnChar), (wxObject*)NULL, this);

    _ccp = new ColorCurvePanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER);
    _ccp->SetMinSize(wxSize(200, 100));
    _ccp->SetValue(_cc);
    _ccp->SetType(_cc->GetType());
    FlexGridSizer6->Add(_ccp, 1, wxALL | wxEXPAND, 2);
    Layout();
    Fit();

    _backup = *_cc;

    Choice1->SetStringSelection(wxString(_cc->GetType().c_str()));
    Choice1->SetFocus();
    ValidateWindow();
}

ColorCurveDialog::~ColorCurveDialog()
{
	//(*Destroy(ColorCurveDialog)
	//*)
}


void ColorCurveDialog::OnButton_OkClick(wxCommandEvent& event)
{
    EndDialog(wxOK);
}

void ColorCurveDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    *_cc = _backup;
    EndDialog(wxCANCEL);
}

void ColorCurveDialog::OnChoice1Select(wxCommandEvent& event)
{
    _ccp->SetType(std::string(Choice1->GetStringSelection().c_str()));
    _cc->SetType(std::string(Choice1->GetStringSelection().c_str()));
    _ccp->Refresh();
    ValidateWindow();
}

#pragma region Mouse Control

void ColorCurvePanel::Undo()
{
    if (_undo.size() > 0)
    {
        float x = _undo.back();
        _undo.pop_back();

        if (x < 0)
        {
            _cc->DeletePoint(x);
        }
        else
        {
            //_cc->SetValueAt(x);
        }
    }
}

void ColorCurvePanel::SaveUndo(float x)
{
    _undo.push_back(x);
}

void ColorCurvePanel::SaveUndoSelected()
{
    if (_cc->IsSetPoint(_grabbedPoint))
    {
        SaveUndo(_grabbedPoint);
    }
    else
    {
        SaveUndo(_grabbedPoint);
    }
}

void ColorCurvePanel::mouseLeftDown(wxMouseEvent& event)
{
    float x;
    Convert(x, event);
    _grabbedPoint = x;
    if (_grabbedPoint < 0.0f)
    {
        _grabbedPoint = 0.0f;
    }
    else if (_grabbedPoint > 1.0f)
    {
        _grabbedPoint = 1.0f;
    }
    _grabbedPoint = ccSortableColorPoint::Normalise(_grabbedPoint);
    _originalGrabbedPoint = _grabbedPoint;
    SaveUndoSelected();
    CaptureMouse();
    mouseMoved(event);
    Refresh();
}

void ColorCurvePanel::mouseCaptureLost(wxMouseCaptureLostEvent& event)
{
    Refresh();
}

void ColorCurvePanel::mouseLeftUp(wxMouseEvent& event)
{
    if (HasCapture())
    {
        float x;
        Convert(x, event);
        //_cc->SetValueAt(_grabbedPoint);
        //_grabbedPoint = -1;
        ReleaseMouse();
    }
    Refresh();
}

void ColorCurvePanel::Delete()
{
    if (_grabbedPoint >= 0)
    {
        _cc->DeletePoint(_grabbedPoint);
        _grabbedPoint = -1;
        Refresh();
    }
}

void ColorCurvePanel::mouseEnter(wxMouseEvent& event)
{
    SetCursor(wxCURSOR_CROSS);
    mouseMoved(event);
}

void ColorCurvePanel::mouseLeave(wxMouseEvent& event)
{
    SetCursor(wxCURSOR_DEFAULT);
}

void ColorCurvePanel::mouseMoved(wxMouseEvent& event)
{
    float x;
    Convert(x, event);

    if (_cc->NearPoint(x))
    {
        SetCursor(wxCURSOR_SIZEWE);
    }
    else
    {
        SetCursor(wxCURSOR_CROSS);
    }

    if (HasCapture())
    {
        if (_originalGrabbedPoint == 0 || _originalGrabbedPoint == 1.0)
        {
            // dont allow x to change
        }
        else
        {
            _cc->DeletePoint(_grabbedPoint);
            _grabbedPoint = x;
        }
        //_cc->SetValueAt(_grabbedPoint);

        Refresh();
    }
}
#pragma endregion Mouse Control

void ColorCurvePanel::Paint(wxPaintEvent& event)
{
    //wxPaintDC pdc(this);
    wxAutoBufferedPaintDC pdc(this);
    wxSize s = GetSize();
    if (_cc != NULL)
    {
        wxBitmap bmp = _cc->GetImage(s.GetWidth(), s.GetHeight());
        pdc.DrawBitmap(bmp, 0, 0);

        pdc.SetPen(wxPen(*wxRED, 2, wxPENSTYLE_SOLID));
        pdc.SetLogicalFunction(wxXOR);

        std::list<ccSortableColorPoint> pts = _cc->GetPoints();

        if (pts.size() > 1)
        {
            std::list<ccSortableColorPoint>::iterator last = pts.begin();
            for (auto p = pts.begin()++; p != pts.end(); p++)
            {
                pdc.DrawLine(p->x, 0, p->x, s.GetHeight());
            }
        }

        if (HasCapture())
        {
            pdc.SetPen(wxPen(*wxBLUE, 2, wxPENSTYLE_SOLID));
            pdc.DrawLine(_grabbedPoint, 0, _grabbedPoint, s.GetHeight());
        }
    }
}

void ColorCurveDialog::ValidateWindow()
{
}

void ColorCurveDialog::OnChar(wxKeyEvent& event)
{
    wxChar uc = event.GetUnicodeKey();
    if (uc == WXK_DELETE)
    {
        _ccp->SaveUndoSelected();
        _ccp->Delete();
    }
    else if ((uc == 'Z' || uc == 'z') && event.ControlDown())
    {
        _ccp->Undo();
    }
    Refresh();
}

void ColorCurveDialog::OnResize(wxSizeEvent& event)
{
    OnSize(event);
    Refresh();
}

void ColorCurveDialog::OnGrid1CellSelect(wxGridEvent& event)
{
    _ccp->Select(wxAtof(Grid1->GetCellValue(event.GetRow(), 1)));
}

void ColorCurveDialog::OnGrid1CellLeftDClick(wxGridEvent& event)
{
    _ccp->Select(wxAtof(Grid1->GetCellValue(event.GetRow(), 1)));
    if (event.GetCol() == 1)
    {
        wxColourData& c = ColourDialog1->GetColourData();
        c.SetColour(Grid1->GetCellBackgroundColour(event.GetRow(), 1));
        if (ColourDialog1->ShowModal() == wxID_OK)
        {
            Grid1->SetCellBackgroundColour(event.GetRow(), event.GetCol(), c.GetColour());
            _cc->SetValueAt(wxAtof(Grid1->GetCellValue(event.GetRow(), 1)), c.GetColour());
            Grid1->Refresh();
        }
    }
    _ccp->Refresh();
}

void ColorCurveDialog::OnGrid1CellLeftClick(wxGridEvent& event)
{
    _ccp->Select(wxAtof(Grid1->GetCellValue(event.GetRow(), 1)));
    _ccp->Refresh();
}
