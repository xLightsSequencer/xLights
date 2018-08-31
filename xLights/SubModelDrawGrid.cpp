//(*InternalHeaders(SubModelDrawGrid)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/tokenzr.h>
#include <wx/settings.h>

#include "SubModelDrawGrid.h"
#include "models/Model.h"

//(*IdInit(SubModelDrawGrid)
const long SubModelDrawGrid::ID_BUTTON_SELECT = wxNewId();
const long SubModelDrawGrid::ID_BUTTON_DESELECT = wxNewId();
const long SubModelDrawGrid::ID_BUTTON_SELECT_ALL = wxNewId();
const long SubModelDrawGrid::ID_BUTTON_SELECT_NONE = wxNewId();
const long SubModelDrawGrid::ID_GRID_NODES = wxNewId();
const long SubModelDrawGrid::ID_BUTTON_SUB_DRAW_OK = wxNewId();
const long SubModelDrawGrid::ID_BUTTON_SUB_DRAW_CANCEL = wxNewId();
//*)

BEGIN_EVENT_TABLE(SubModelDrawGrid,wxDialog)
	//(*EventTable(SubModelDrawGrid)
	//*)
END_EVENT_TABLE()

SubModelDrawGrid::SubModelDrawGrid(Model *m, std::vector<wxString> rows, wxWindow* parent, wxWindowID id)
{
    selectColor = wxColour("white");
    unselectColor =  wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXTEXT);
    selectBackColor = wxColour("grey");
    unselectBackColor = wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX);

    model = m;
	//(*Initialize(SubModelDrawGrid)
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
	Button_Select = new wxButton(this, ID_BUTTON_SELECT, _("Select"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SELECT"));
	FlexGridSizer4->Add(Button_Select, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonDeselect = new wxButton(this, ID_BUTTON_DESELECT, _("De-Select"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_DESELECT"));
	FlexGridSizer4->Add(ButtonDeselect, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonSelectAll = new wxButton(this, ID_BUTTON_SELECT_ALL, _("Select All"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SELECT_ALL"));
	FlexGridSizer4->Add(ButtonSelectAll, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonSelectNone = new wxButton(this, ID_BUTTON_SELECT_NONE, _("Select None"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SELECT_NONE"));
	FlexGridSizer4->Add(ButtonSelectNone, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2->Add(FlexGridSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	GridNodes = new wxGrid(this, ID_GRID_NODES, wxDefaultPosition, wxDefaultSize, 0, _T("ID_GRID_NODES"));
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
	ButtonSubDrawOK = new wxButton(this, ID_BUTTON_SUB_DRAW_OK, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SUB_DRAW_OK"));
	BoxSizer1->Add(ButtonSubDrawOK, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 5);
	ButtonSunDrawCancel = new wxButton(this, ID_BUTTON_SUB_DRAW_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SUB_DRAW_CANCEL"));
	BoxSizer1->Add(ButtonSunDrawCancel, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 5);
	wxBoxSizerMain->Add(BoxSizer1, 0, wxALL|wxFIXED_MINSIZE, 5);
	SetSizer(wxBoxSizerMain);
	SetSizer(wxBoxSizerMain);
	Layout();

	Connect(ID_BUTTON_SELECT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelDrawGrid::OnButton_SelectClick);
	Connect(ID_BUTTON_DESELECT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelDrawGrid::OnButtonDeselectClick);
	Connect(ID_BUTTON_SELECT_ALL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelDrawGrid::OnButtonSelectAllClick);
	Connect(ID_BUTTON_SELECT_NONE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelDrawGrid::OnButtonSelectNoneClick);
	Connect(ID_GRID_NODES,wxEVT_GRID_CELL_LEFT_DCLICK,(wxObjectEventFunction)&SubModelDrawGrid::OnGridNodesCellLeftDClick);
	Connect(ID_BUTTON_SUB_DRAW_OK,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelDrawGrid::OnButtonSubDrawOKClick);
	Connect(ID_BUTTON_SUB_DRAW_CANCEL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelDrawGrid::OnButtonSubDrawCancelClick);
	//*)

    GridNodes->EnableDragGridSize(false);
    GridNodes->DisableDragColSize();
    GridNodes->DisableDragRowSize();

    LoadGrid(rows);
    ValidateWindow();
}

SubModelDrawGrid::~SubModelDrawGrid()
{
	//(*Destroy(SubModelDrawGrid)
	//*)
}


void SubModelDrawGrid::LoadGrid(const std::vector<wxString> rows)
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

void SubModelDrawGrid::ValidateWindow() const
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
                    ButtonSubDrawOK->Enable(true);
                    return;
                }
            }
        }
    }
    ButtonSubDrawOK->Enable(false);
}

void SubModelDrawGrid::OnButton_SelectClick(wxCommandEvent& event)
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
    GridNodes->Refresh();
    ValidateWindow();
}

void SubModelDrawGrid::OnButtonSelectAllClick(wxCommandEvent& event)
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
    GridNodes->Refresh();
    ValidateWindow();
}

void SubModelDrawGrid::OnButtonSelectNoneClick(wxCommandEvent& event)
{
    for (auto x = 0; x < GridNodes->GetNumberCols(); x++)
    {
        for (auto y = 0; y < GridNodes->GetNumberRows(); y++)
        {
            GridNodes->SetCellTextColour(y, x, unselectColor);
            GridNodes->SetCellBackgroundColour(y, x, unselectBackColor);
        }
    }
    GridNodes->Refresh();
    ValidateWindow();
}

void SubModelDrawGrid::OnButtonDeselectClick(wxCommandEvent& event)
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
    GridNodes->Refresh();
    ValidateWindow();
}

void SubModelDrawGrid::OnButtonSubDrawOKClick(wxCommandEvent& event)
{
    EndDialog(wxID_OK);
}

void SubModelDrawGrid::OnButtonSubDrawCancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void SubModelDrawGrid::OnGridNodesCellLeftDClick(wxGridEvent& event)
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

std::vector<wxString> SubModelDrawGrid::GetRowData()
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

std::vector<int> SubModelDrawGrid::DecodeNodeList(const std::vector<wxString> &rows) const
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
