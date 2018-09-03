//(*InternalHeaders(NodeSelectGrid)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/tokenzr.h>
#include <wx/settings.h>

#include "NodeSelectGrid.h"
#include "models/Model.h"

//(*IdInit(NodeSelectGrid)
const long NodeSelectGrid::ID_CHECKBOX_FREE_HAND = wxNewId();
const long NodeSelectGrid::ID_BUTTON_SELECT = wxNewId();
const long NodeSelectGrid::ID_BUTTON_DESELECT = wxNewId();
const long NodeSelectGrid::ID_BUTTON_SELECT_ALL = wxNewId();
const long NodeSelectGrid::ID_BUTTON_SELECT_NONE = wxNewId();
const long NodeSelectGrid::ID_GRID_NODES = wxNewId();
const long NodeSelectGrid::ID_BUTTON_NODE_SELECT_OK = wxNewId();
const long NodeSelectGrid::ID_BUTTON_NODE_SELECT_CANCEL = wxNewId();
//*)

BEGIN_EVENT_TABLE(NodeSelectGrid,wxDialog)
	//(*EventTable(NodeSelectGrid)
	//*)
END_EVENT_TABLE()

// define a single DRAW_GRID_CLICKED event type associated with the class
wxDEFINE_EVENT(DRAW_GRID_CLICKED, DrawGridEvent);

//use new event way not old macros
#define DrawGridEventHandler(func) (&func)

// define a new event class
class DrawGridEvent : public wxEvent
{
public:
    DrawGridEvent(wxEventType eventType, int winid, int row, int col, bool modifier)
        : wxEvent(winid, eventType),
        m_row(row),
        m_col(col),
        m_modifier(modifier)
    {
    }
    // accessors
    int GetRow() const { return m_row; }
    int GetCol() const { return m_col; }
    bool GetModifier() const { return m_modifier; }
    // implement the base class pure virtual
    virtual wxEvent *Clone() const { return new DrawGridEvent(*this); }
private:
    const int m_row;
    const int m_col;
    const bool m_modifier;
};

// Subclassing wxGrid
class DrawGrid : public wxGrid
{
    bool _drawMode = false;
    bool _mouseDown = false;
    int _prevRow = -1;
    int _prevCol = -1;
    void DisableRangeHandler(wxGridRangeSelectEvent &ev)
    {
        if (ev.Selecting() && (ev.GetTopRow() != ev.GetBottomRow()))
        {
            ev.StopPropagation();
            ev.Veto();
        }
    }

    void DisableCtrlMaiusHandler(wxGridEvent &ev)
    {
        ev.Skip();
    }

    void DisableDraggingHandler(wxMouseEvent &ev)
    {
        if (ev.Dragging())
        {
            ev.Skip(false);
        }
    }
    void OnMouseMove( wxMouseEvent& event )
    {
        if(_drawMode && _mouseDown)
        {
            const wxPoint realPoint = CalcUnscrolledPosition(event.GetPosition());
            wxGridCellCoords cell = XYToCell(realPoint.x, realPoint.y);

            if (cell.GetRow() == -1 || cell.GetCol() == -1)
                return;

            if (!(cell.GetRow() == _prevRow && cell.GetCol() == _prevCol))
            {
                DrawGridEvent newevent(DRAW_GRID_CLICKED, GetId(), cell.GetRow(), cell.GetCol(), event.HasAnyModifiers());
                newevent.SetEventObject(this);
                wxPostEvent(this->GetParent(), newevent);
                _prevRow = cell.GetRow();
                _prevCol = cell.GetCol();
            }
        }
    }

    void OnMouseLeftDown(wxMouseEvent& event)
    {
        _mouseDown = true;
    }

    void OnMouseLeftUp(wxMouseEvent& event)
    {
        _mouseDown = false;
    }

public:
    DrawGrid(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name) : wxGrid(parent, id, pos, size, style, name)
    {
    }

    virtual ~DrawGrid()
    {
    }

    void SetDrawMode(bool enabled)
    {
        if (_drawMode == enabled)
        {
            return;
        }
        if (enabled)
        {
            Bind(wxEVT_GRID_RANGE_SELECT, &DrawGrid::DisableRangeHandler, this);
            Bind(wxEVT_GRID_CELL_LEFT_CLICK, &DrawGrid::DisableCtrlMaiusHandler, this);
            GetGridWindow()->Bind(wxEVT_MOTION, &DrawGrid::DisableDraggingHandler, this);
            ((wxWindow *)m_gridWin)->Connect(wxEVT_MOTION, wxMouseEventHandler(DrawGrid::OnMouseMove), 0, this);
            ((wxWindow *)m_gridWin)->Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(DrawGrid::OnMouseLeftDown), 0, this);
            ((wxWindow *)m_gridWin)->Connect(wxEVT_LEFT_UP, wxMouseEventHandler(DrawGrid::OnMouseLeftUp), 0, this);
            _drawMode = true;
        }
        else
        {
            Unbind(wxEVT_GRID_RANGE_SELECT, &DrawGrid::DisableRangeHandler, this);
            //Unbind(wxEVT_GRID_SELECT_CELL, &DrawGrid::DisableRangeSelectCellHandler, this);
            Unbind(wxEVT_GRID_CELL_LEFT_CLICK, &DrawGrid::DisableCtrlMaiusHandler, this);
            GetGridWindow()->Unbind(wxEVT_MOTION, &DrawGrid::DisableDraggingHandler, this);
            ((wxWindow *)m_gridWin)->Disconnect(wxEVT_MOTION, wxMouseEventHandler(DrawGrid::OnMouseMove), 0, this);
            ((wxWindow *)m_gridWin)->Disconnect(wxEVT_LEFT_DOWN, wxMouseEventHandler(DrawGrid::OnMouseLeftDown), 0, this);
            ((wxWindow *)m_gridWin)->Disconnect(wxEVT_LEFT_UP, wxMouseEventHandler(DrawGrid::OnMouseLeftUp), 0, this);
            _drawMode = false;
        }
    }

    void SetMouseDown(bool down)
    {
        _mouseDown = down;
    }
};

//overloading contructor
NodeSelectGrid::NodeSelectGrid(Model *m, wxString row, wxWindow* parent, wxWindowID id)
    : NodeSelectGrid(m, std::vector<wxString>(1, row), parent, id)
{

}

NodeSelectGrid::NodeSelectGrid(Model *m, std::vector<wxString> rows, wxWindow* parent, wxWindowID id)

{
    selectColor = wxColour("white");
    unselectColor =  wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXTEXT);
    selectBackColor = wxColour("grey");
    unselectBackColor = wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX);

    model = m;
	//(*Initialize(NodeSelectGrid)
	wxBoxSizer* BoxSizer1;
	wxBoxSizer* wxBoxSizerMain;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer4;

	Create(parent, id, _("Select Nodes"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxCLOSE_BOX|wxMAXIMIZE_BOX|wxMINIMIZE_BOX, _T("id"));
	SetClientSize(wxSize(500,400));
	SetMinSize(wxSize(-1,-1));
	wxBoxSizerMain = new wxBoxSizer(wxVERTICAL);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(1);
	FlexGridSizer2->AddGrowableRow(0);
	FlexGridSizer4 = new wxFlexGridSizer(0, 1, 0, 0);
	CheckBoxFreeHand = new wxCheckBox(this, ID_CHECKBOX_FREE_HAND, _("Free Hand"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_FREE_HAND"));
	CheckBoxFreeHand->SetValue(false);
	FlexGridSizer4->Add(CheckBoxFreeHand, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Select = new wxButton(this, ID_BUTTON_SELECT, _("Select"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SELECT"));
	FlexGridSizer4->Add(Button_Select, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonDeselect = new wxButton(this, ID_BUTTON_DESELECT, _("De-Select"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_DESELECT"));
	FlexGridSizer4->Add(ButtonDeselect, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonSelectAll = new wxButton(this, ID_BUTTON_SELECT_ALL, _("Select All"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SELECT_ALL"));
	FlexGridSizer4->Add(ButtonSelectAll, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonSelectNone = new wxButton(this, ID_BUTTON_SELECT_NONE, _("Select None"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SELECT_NONE"));
	FlexGridSizer4->Add(ButtonSelectNone, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2->Add(FlexGridSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	GridNodes = new DrawGrid(this, ID_GRID_NODES, wxDefaultPosition, wxDefaultSize, 0, _T("ID_GRID_NODES"));
	GridNodes->CreateGrid(1,1);
	GridNodes->EnableEditing(false);
	GridNodes->EnableGridLines(true);
	GridNodes->SetColLabelSize(20);
	GridNodes->SetRowLabelSize(30);
	GridNodes->SetDefaultColSize(30, true);
	GridNodes->SetDefaultCellFont( GridNodes->GetFont() );
	GridNodes->SetDefaultCellTextColour( GridNodes->GetForegroundColour() );
	FlexGridSizer2->Add(GridNodes, 1, wxALL|wxEXPAND, 5);
	wxBoxSizerMain->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	ButtonNodeSelectOK = new wxButton(this, ID_BUTTON_NODE_SELECT_OK, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_NODE_SELECT_OK"));
	BoxSizer1->Add(ButtonNodeSelectOK, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 5);
	ButtonNodeSelectCancel = new wxButton(this, ID_BUTTON_NODE_SELECT_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_NODE_SELECT_CANCEL"));
	BoxSizer1->Add(ButtonNodeSelectCancel, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 5);
	wxBoxSizerMain->Add(BoxSizer1, 0, wxALL|wxFIXED_MINSIZE, 5);
	SetSizer(wxBoxSizerMain);
	SetSizer(wxBoxSizerMain);
	Layout();

	Connect(ID_CHECKBOX_FREE_HAND,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&NodeSelectGrid::OnCheckBoxFreeHandClick);
	Connect(ID_BUTTON_SELECT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&NodeSelectGrid::OnButton_SelectClick);
	Connect(ID_BUTTON_DESELECT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&NodeSelectGrid::OnButtonDeselectClick);
	Connect(ID_BUTTON_SELECT_ALL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&NodeSelectGrid::OnButtonSelectAllClick);
	Connect(ID_BUTTON_SELECT_NONE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&NodeSelectGrid::OnButtonSelectNoneClick);
	Connect(ID_GRID_NODES,wxEVT_GRID_CELL_LEFT_DCLICK,(wxObjectEventFunction)&NodeSelectGrid::OnGridNodesCellLeftDClick);
	Connect(ID_GRID_NODES,wxEVT_GRID_CELL_RIGHT_DCLICK,(wxObjectEventFunction)&NodeSelectGrid::OnGridNodesCellRightDClick);
	Connect(ID_BUTTON_NODE_SELECT_OK,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&NodeSelectGrid::OnButtonNodeSelectOKClick);
	Connect(ID_BUTTON_NODE_SELECT_CANCEL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&NodeSelectGrid::OnButtonNodeSelectCancelClick);
	//*)

    Bind(DRAW_GRID_CLICKED, &NodeSelectGrid::OnDrawGridEvent, this, ID_GRID_NODES);

    GridNodes->EnableDragGridSize(false);
    GridNodes->DisableDragColSize();
    GridNodes->DisableDragRowSize();
    GridNodes->SetCellHighlightPenWidth(0);

    GridNodes->SetSelectionMode(wxGrid::wxGridSelectCells);

    LoadGrid(rows);
    ValidateWindow();
}

NodeSelectGrid::~NodeSelectGrid()
{
	//(*Destroy(NodeSelectGrid)
	//*)
}


void NodeSelectGrid::LoadGrid(const std::vector<wxString> rows)
{
    std::vector<int> prevValue = DecodeNodeList(rows);

    float minsx = 99999;
    float minsy = 99999;
    float maxsx = -1;
    float maxsy = -1;

    const auto nodeCount = model->GetNodeCount();
    for (auto i = 0; i < nodeCount; i++) {
        std::vector<wxPoint> pts;
        model->GetNodeCoords(i, pts);
        if (pts.size() > 0)
        {
            float Sbufx = pts[0].x;
            float Sbufy = pts[0].y;
            if (Sbufx < minsx) minsx = Sbufx;
            if (Sbufx > maxsx) maxsx = Sbufx;
            if (Sbufy < minsy) minsy = Sbufy;
            if (Sbufy > maxsy) maxsy = Sbufy;
        }
    }

    int minx = std::floor(minsx);
    int miny = std::floor(minsy);
    int maxx = std::ceil(maxsx);
    int maxy = std::ceil(maxsy);
    int sizex = maxx - minx;
    int sizey = maxy - miny;

    GridNodes->AppendCols(sizex);
    GridNodes->AppendRows(sizey);

    for (auto i = 0; i < nodeCount; i++)
    {
        std::vector<wxPoint> pts;
        model->GetNodeCoords(i, pts);
        if (pts.size() > 0)
        {
            GridNodes->SetCellValue(sizey - pts[0].y, pts[0].x, wxString::Format("%i", i + 1));
            if (std::find(prevValue.begin(), prevValue.end(), i) != prevValue.end())
            {
                GridNodes->SetCellTextColour(sizey - pts[0].y, pts[0].x, selectColor);
                GridNodes->SetCellBackgroundColour(sizey - pts[0].y, pts[0].x, selectBackColor);
            }
        }
    }
    GridNodes->Refresh();
}

void NodeSelectGrid::ValidateWindow() const
{
    for (auto x = 0; x < GridNodes->GetNumberCols(); x++)
    {
        for (auto y = 0; y < GridNodes->GetNumberRows(); y++)
        {
            wxString value = GridNodes->GetCellValue(y, x);
            if (!value.IsNull() && !value.IsEmpty())
            {
                if (GridNodes->GetCellTextColour(y, x) == selectColor)
                {
                    ButtonNodeSelectOK->Enable(true);
                    return;
                }
            }
        }
    }
    ButtonNodeSelectOK->Enable(false);
}

void NodeSelectGrid::OnButton_SelectClick(wxCommandEvent& event)
{
    for (auto x = 0; x < GridNodes->GetNumberCols(); x++)
    {
        for (auto y = 0; y < GridNodes->GetNumberRows(); y++)
        {
            wxString value = GridNodes->GetCellValue(y, x);
            if (!value.IsNull() && !value.IsEmpty())
            {
                if (GridNodes->IsInSelection(y, x))
                {
                    GridNodes->SetCellTextColour(y, x, selectColor);
                    GridNodes->SetCellBackgroundColour(y, x, selectBackColor);
                }
            }
        }
    }
    GridNodes->ClearSelection();
    GridNodes->Refresh();
    ValidateWindow();
}

void NodeSelectGrid::OnButtonSelectAllClick(wxCommandEvent& event)
{
    for (auto x = 0; x < GridNodes->GetNumberCols(); x++)
    {
        for (auto y = 0; y < GridNodes->GetNumberRows(); y++)
        {
            wxString value = GridNodes->GetCellValue(y, x);
            if (!value.IsNull() && !value.IsEmpty())
            {
                GridNodes->SetCellTextColour(y, x, selectColor);
                GridNodes->SetCellBackgroundColour(y, x, selectBackColor);
            }
        }
    }
    GridNodes->ClearSelection();
    GridNodes->Refresh();
    ValidateWindow();
}

void NodeSelectGrid::OnButtonSelectNoneClick(wxCommandEvent& event)
{
    for (auto x = 0; x < GridNodes->GetNumberCols(); x++)
    {
        for (auto y = 0; y < GridNodes->GetNumberRows(); y++)
        {
            GridNodes->SetCellTextColour(y, x, unselectColor);
            GridNodes->SetCellBackgroundColour(y, x, unselectBackColor);
        }
    }
    GridNodes->ClearSelection();
    GridNodes->Refresh();
    ValidateWindow();
}

void NodeSelectGrid::OnButtonDeselectClick(wxCommandEvent& event)
{
    for (auto x = 0; x < GridNodes->GetNumberCols(); x++)
    {
        for (auto y = 0; y < GridNodes->GetNumberRows(); y++)
        {
            wxString value = GridNodes->GetCellValue(y, x);
            if (!value.IsNull() && !value.IsEmpty())
            {
                if (GridNodes->IsInSelection(y, x))
                {
                    GridNodes->SetCellTextColour(y, x, unselectColor);
                    GridNodes->SetCellBackgroundColour(y, x, unselectBackColor);
                }
            }
        }
    }
    GridNodes->ClearSelection();
    GridNodes->Refresh();
    ValidateWindow();
}

void NodeSelectGrid::OnButtonNodeSelectOKClick(wxCommandEvent& event)
{
    EndDialog(wxID_OK);
}

void NodeSelectGrid::OnButtonNodeSelectCancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void NodeSelectGrid::OnCheckBoxFreeHandClick(wxCommandEvent& event)
{
    GridNodes->ClearSelection();
    GridNodes->SetDrawMode(event.IsChecked());
}

void NodeSelectGrid::OnGridNodesCellLeftDClick(wxGridEvent& event)
{
    wxString value = GridNodes->GetCellValue(event.GetRow(), event.GetCol());
    if (!value.IsNull() && !value.IsEmpty())
    {
        if (GridNodes->GetCellTextColour(event.GetRow(), event.GetCol()) == selectColor)
        {
            GridNodes->SetCellTextColour(event.GetRow(), event.GetCol(), unselectColor);
            GridNodes->SetCellBackgroundColour(event.GetRow(), event.GetCol(), unselectBackColor);
        }
        else
        {
            GridNodes->SetCellTextColour(event.GetRow(), event.GetCol(), selectColor);
            GridNodes->SetCellBackgroundColour(event.GetRow(), event.GetCol(), selectBackColor);
        }
        GridNodes->Refresh();
        ValidateWindow();
    }
}

void NodeSelectGrid::OnGridNodesCellRightDClick(wxGridEvent& event)
{
    wxString value = GridNodes->GetCellValue(event.GetRow(), event.GetCol());
    if (!value.IsNull() && !value.IsEmpty())
    {
        if (event.ShiftDown())
        {
            GridNodes->SetCellTextColour(event.GetRow(), event.GetCol(), unselectColor);
            GridNodes->SetCellBackgroundColour(event.GetRow(), event.GetCol(), unselectBackColor);
        }
        else
        {
            GridNodes->SetCellTextColour(event.GetRow(), event.GetCol(), selectColor);
            GridNodes->SetCellBackgroundColour(event.GetRow(), event.GetCol(), selectBackColor);
        }
        GridNodes->Refresh();
        ValidateWindow();
    }
}

void NodeSelectGrid::OnDrawGridEvent(DrawGridEvent& event)
{
    wxString value = GridNodes->GetCellValue(event.GetRow(), event.GetCol());
    if (!value.IsNull() && !value.IsEmpty())
    {
        if (event.GetModifier())
        {
            GridNodes->SetCellTextColour(event.GetRow(), event.GetCol(), unselectColor);
            GridNodes->SetCellBackgroundColour(event.GetRow(), event.GetCol(), unselectBackColor);
        }
        else
        {
            GridNodes->SetCellTextColour(event.GetRow(), event.GetCol(), selectColor);
            GridNodes->SetCellBackgroundColour(event.GetRow(), event.GetCol(), selectBackColor);
        }
        GridNodes->Refresh();
        ValidateWindow();
    }
}

std::vector<wxString> NodeSelectGrid::GetRowData()
{
    std::vector<wxString> returnValue;
    int startx = INT32_MAX;
    int starty = INT32_MAX;
    int endx = 0;
    int endy = 0;

    for (auto x = 0; x < GridNodes->GetNumberCols(); x++)
    {
        for (auto y = 0; y < GridNodes->GetNumberRows(); y++)
        {
            wxString value = GridNodes->GetCellValue(y, x);
            if (!value.IsNull() && !value.IsEmpty())
            {
                if (GridNodes->GetCellTextColour(y, x) == selectColor)
                {
                    startx = std::min(startx, x);
                    starty = std::min(starty, y);
                    endx = std::max(endx, x);
                    endy = std::max(endy, y);
                }
            }
        }
    }

    for (auto y = starty; y <= endy; y++)
    {
        wxString row;
        for (auto x = startx; x <= endx; x++)
        {
            wxString value = GridNodes->GetCellValue(y, x);
            if (!value.IsEmpty() && GridNodes->GetCellTextColour(y, x) == selectColor)
            {
                row = row + value + ",";
            }
            else
            {
                row = row + ",";
            }
        }
        row.erase(row.length() - 1, 1);
        returnValue.insert(returnValue.begin(),row);
    }
    return returnValue;
}

wxString NodeSelectGrid::GetNodeList()
{
    //encode with dashs
    std::vector<wxString> nodeList;
    for (auto x = 0; x < GridNodes->GetNumberCols(); x++)
    {
        for (auto y = 0; y < GridNodes->GetNumberRows(); y++)
        {
            wxString value = GridNodes->GetCellValue(y, x);
            if (!value.IsNull() && !value.IsEmpty() && GridNodes->GetCellTextColour(y, x) == selectColor)
            {
                nodeList.push_back(value);
            }
        }
    }
    //encode with dashs
    return EncodeNodeLine(nodeList);
}

std::vector<int> NodeSelectGrid::DecodeNodeList(const std::vector<wxString> &rows) const
{
    std::vector<int> nodeList;
    for (const auto& row : rows)
    {
        wxStringTokenizer wtkz(row, ",");
        while (wtkz.HasMoreTokens()) {
            wxString valstr = wtkz.GetNextToken();

            int start2, end2;
            if (valstr.Contains("-")) {
                const int idx = valstr.Index('-');
                start2 = wxAtoi(valstr.Left(idx));
                end2 = wxAtoi(valstr.Right(valstr.size() - idx - 1));
            }
            else {
                start2 = end2 = wxAtoi(valstr);
            }
            start2--;
            end2--;
            auto done = false;
            auto n = start2;
            while (!done) {
                if (n < model->GetNodeCount()) {
                    nodeList.push_back(n);
                }
                if (start2 > end2) {
                    n--;
                    done = n < end2;
                }
                else {
                    n++;
                    done = n > end2;
                }
            }
        }
    }
    return nodeList;
}

//encode node list with dashs
wxString NodeSelectGrid::EncodeNodeLine(const std::vector<wxString> &nodes) const
{
    wxString rowValue;
    std::vector<int> iNodes;
    std::transform(nodes.begin(), nodes.end(), std::back_inserter(iNodes),
        [](const std::string& str) { return std::stoi(str); });

    std::sort(iNodes.begin(), iNodes.end());
    iNodes.erase(std::unique(iNodes.begin(), iNodes.end()), iNodes.end());

    int firstValue = -1;;
    int prevValue = -1;;
    for (auto& item : iNodes)
    {
        if (&item == &iNodes.front())
        {
            firstValue = prevValue = item;
            continue;
        }

        if(item != prevValue + 1)
        {
            if (firstValue != prevValue)
                rowValue += wxString::Format(wxT("%i-%i,"), firstValue, prevValue);
            else
                rowValue += wxString::Format(wxT("%i,"), prevValue);
            firstValue = item;
        }

        if (&item == &iNodes.back())
        {
            if (item == prevValue + 1)
                rowValue += wxString::Format(wxT("%i-%i"), firstValue, item);
            else
                rowValue += wxString::Format(wxT("%i"), item);
        }
        prevValue = item;
    }
    return rowValue;
}
