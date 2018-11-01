//(*InternalHeaders(NodeSelectGrid)
#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/intl.h>
#include <wx/image.h>
#include <wx/string.h>
//*)

#include <wx/tokenzr.h>
#include <wx/settings.h>
#include <wx/settings.h>
#include <wx/xml/xml.h>
#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <wx/clipbrd.h>

#include "NodeSelectGrid.h"
#include "models/Model.h"
#include "models/CustomModel.h"

//(*IdInit(NodeSelectGrid)
const long NodeSelectGrid::ID_CHECKBOX_FREE_HAND = wxNewId();
const long NodeSelectGrid::ID_BUTTON_SELECT = wxNewId();
const long NodeSelectGrid::ID_BUTTON_DESELECT = wxNewId();
const long NodeSelectGrid::ID_BUTTON_SELECT_ALL = wxNewId();
const long NodeSelectGrid::ID_BUTTON_SELECT_NONE = wxNewId();
const long NodeSelectGrid::ID_BUTTON_LOAD_MODEL = wxNewId();
const long NodeSelectGrid::ID_BUTTON_ZOOM_PLUS = wxNewId();
const long NodeSelectGrid::ID_BUTTON_ZOOM_MINUS = wxNewId();
const long NodeSelectGrid::ID_FILEPICKERCTRL1 = wxNewId();
const long NodeSelectGrid::ID_SLIDER_IMG_BRIGHTNESS = wxNewId();
const long NodeSelectGrid::ID_BITMAPBUTTON1 = wxNewId();
const long NodeSelectGrid::ID_BUTTON_NODE_SELECT_OK = wxNewId();
const long NodeSelectGrid::ID_BUTTON_NODE_SELECT_CANCEL = wxNewId();
const long NodeSelectGrid::ID_GRID_NODES = wxNewId();
//*)

const long NodeSelectGrid::NODESELECT_CUT = wxNewId();
const long NodeSelectGrid::NODESELECT_COPY = wxNewId();
const long NodeSelectGrid::NODESELECT_PASTE = wxNewId();


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

    void DoOnChar(wxKeyEvent& event)
    {
        wxChar uc = event.GetUnicodeKey();

        switch (uc)
        {
        case 'c':
        case 'C':
        case WXK_CONTROL_C:
            if (event.CmdDown() || event.ControlDown()) {
                wxCommandEvent pasteEvent(wxEVT_TEXT_COPY);
                wxPostEvent(this, pasteEvent);
                event.StopPropagation();
            }
            break;
        case 'x':
        case 'X':
        case WXK_CONTROL_X:
            if (event.CmdDown() || event.ControlDown()) {
                wxCommandEvent pasteEvent(wxEVT_TEXT_CUT);
                wxPostEvent(this, pasteEvent);
                event.StopPropagation();
            }
            break;
        case 'v':
        case 'V':
        case WXK_CONTROL_V:
            if (event.CmdDown() || event.ControlDown()) {
                wxCommandEvent pasteEvent(wxEVT_TEXT_PASTE);
                wxPostEvent(this, pasteEvent);
                event.StopPropagation();
            }
            break;
        default:
            wxGrid::OnChar(event);
            break;
        }
    }

public:
    DrawGrid(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name) : wxGrid(parent, id, pos, size, style, name)
    {
        Connect(wxEVT_CHAR, (wxObjectEventFunction)&DrawGrid::DoOnChar, 0, this);
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
NodeSelectGrid::NodeSelectGrid(Model *m, const wxString& row, wxWindow* parent, wxWindowID id)
    : NodeSelectGrid(m, std::vector<wxString>(1, row), parent, id)
{

};

NodeSelectGrid::NodeSelectGrid(Model *m, const std::vector<wxString>& rows, wxWindow* parent, wxWindowID id)
: model(m),
  bkg_image(nullptr),
  renderer(nullptr),
  bkgrd_active(true),
  selectColor(wxColour("white")),
  unselectColor(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXTEXT)),
  selectBackColor(wxColour("grey")),
  unselectBackColor(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX))
{
	//(*Initialize(NodeSelectGrid)
	wxStaticBoxSizer* StaticBoxSizer2;
	wxFlexGridSizer* FlexGridSizer4;
	wxBoxSizer* wxBoxSizerMain;
	wxFlexGridSizer* FlexGridSizer2;
	wxStaticBoxSizer* StaticBoxSizer3;
	wxBoxSizer* BoxSizer1;
	wxStaticBoxSizer* StaticBoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, wxID_ANY, _("Select Nodes"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxCLOSE_BOX|wxMAXIMIZE_BOX|wxMINIMIZE_BOX, _T("wxID_ANY"));
	SetClientSize(wxDLG_UNIT(parent,wxSize(500,450)));
	SetMinSize(wxSize(-1,-1));
	wxBoxSizerMain = new wxBoxSizer(wxVERTICAL);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(1);
	FlexGridSizer2->AddGrowableRow(0);
	FlexGridSizer4 = new wxFlexGridSizer(0, 1, 0, 0);
	StaticBoxSizer3 = new wxStaticBoxSizer(wxVERTICAL, this, _("Selection"));
	CheckBoxFreeHand = new wxCheckBox(this, ID_CHECKBOX_FREE_HAND, _("Free Hand"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_FREE_HAND"));
	CheckBoxFreeHand->SetValue(false);
	StaticBoxSizer3->Add(CheckBoxFreeHand, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Select = new wxButton(this, ID_BUTTON_SELECT, _("Select"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SELECT"));
	StaticBoxSizer3->Add(Button_Select, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonDeselect = new wxButton(this, ID_BUTTON_DESELECT, _("De-Select"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_DESELECT"));
	StaticBoxSizer3->Add(ButtonDeselect, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonSelectAll = new wxButton(this, ID_BUTTON_SELECT_ALL, _("Select All"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SELECT_ALL"));
	StaticBoxSizer3->Add(ButtonSelectAll, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonSelectNone = new wxButton(this, ID_BUTTON_SELECT_NONE, _("Select None"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SELECT_NONE"));
	StaticBoxSizer3->Add(ButtonSelectNone, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonLoadModel = new wxButton(this, ID_BUTTON_LOAD_MODEL, _("From Model"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_LOAD_MODEL"));
	StaticBoxSizer3->Add(ButtonLoadModel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4->Add(StaticBoxSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer2 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Zoom"));
	ButtonZoomPlus = new wxButton(this, ID_BUTTON_ZOOM_PLUS, _("+"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_ZOOM_PLUS"));
	ButtonZoomPlus->SetMinSize(wxSize(30,-1));
	StaticBoxSizer2->Add(ButtonZoomPlus, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonZoomMinus = new wxButton(this, ID_BUTTON_ZOOM_MINUS, _("-"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_ZOOM_MINUS"));
	ButtonZoomMinus->SetMinSize(wxSize(30,-1));
	StaticBoxSizer2->Add(ButtonZoomMinus, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4->Add(StaticBoxSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer1 = new wxStaticBoxSizer(wxVERTICAL, this, _("Background Image"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FilePickerCtrl1 = new wxFilePickerCtrl(this, ID_FILEPICKERCTRL1, wxEmptyString, wxEmptyString, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxFLP_FILE_MUST_EXIST|wxFLP_OPEN|wxFLP_USE_TEXTCTRL, wxDefaultValidator, _T("ID_FILEPICKERCTRL1"));
	FlexGridSizer1->Add(FilePickerCtrl1, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SliderImgBrightness = new wxSlider(this, ID_SLIDER_IMG_BRIGHTNESS, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_IMG_BRIGHTNESS"));
	FlexGridSizer1->Add(SliderImgBrightness, 1, wxALL|wxEXPAND, 2);
	BitmapButton1 = new wxBitmapButton(this, ID_BITMAPBUTTON1, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_FIND")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON1"));
	FlexGridSizer1->Add(BitmapButton1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer1->Add(FlexGridSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4->Add(StaticBoxSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	ButtonNodeSelectOK = new wxButton(this, ID_BUTTON_NODE_SELECT_OK, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_NODE_SELECT_OK"));
	BoxSizer1->Add(ButtonNodeSelectOK, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 5);
	ButtonNodeSelectCancel = new wxButton(this, ID_BUTTON_NODE_SELECT_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_NODE_SELECT_CANCEL"));
	BoxSizer1->Add(ButtonNodeSelectCancel, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 5);
	FlexGridSizer4->Add(BoxSizer1, 0, wxALL|wxFIXED_MINSIZE, 5);
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
	SetSizer(wxBoxSizerMain);
	SetSizer(wxBoxSizerMain);
	Layout();

	Connect(ID_CHECKBOX_FREE_HAND,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&NodeSelectGrid::OnCheckBoxFreeHandClick);
	Connect(ID_BUTTON_SELECT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&NodeSelectGrid::OnButton_SelectClick);
	Connect(ID_BUTTON_DESELECT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&NodeSelectGrid::OnButtonDeselectClick);
	Connect(ID_BUTTON_SELECT_ALL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&NodeSelectGrid::OnButtonSelectAllClick);
	Connect(ID_BUTTON_SELECT_NONE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&NodeSelectGrid::OnButtonSelectNoneClick);
	Connect(ID_BUTTON_LOAD_MODEL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&NodeSelectGrid::OnButtonLoadModelClick);
	Connect(ID_BUTTON_ZOOM_PLUS,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&NodeSelectGrid::OnButtonZoomPlusClick);
	Connect(ID_BUTTON_ZOOM_MINUS,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&NodeSelectGrid::OnButtonZoomMinusClick);
	Connect(ID_FILEPICKERCTRL1,wxEVT_COMMAND_FILEPICKER_CHANGED,(wxObjectEventFunction)&NodeSelectGrid::OnFilePickerCtrl1FileChanged);
	Connect(ID_SLIDER_IMG_BRIGHTNESS,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&NodeSelectGrid::OnSliderImgBrightnessCmdScroll);
	Connect(ID_BITMAPBUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&NodeSelectGrid::OnBitmapButton1Click);
	Connect(ID_BUTTON_NODE_SELECT_OK,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&NodeSelectGrid::OnButtonNodeSelectOKClick);
	Connect(ID_BUTTON_NODE_SELECT_CANCEL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&NodeSelectGrid::OnButtonNodeSelectCancelClick);
	Connect(ID_GRID_NODES,wxEVT_GRID_CELL_RIGHT_CLICK,(wxObjectEventFunction)&NodeSelectGrid::OnGridNodesCellRightClick);
	Connect(ID_GRID_NODES,wxEVT_GRID_CELL_LEFT_DCLICK,(wxObjectEventFunction)&NodeSelectGrid::OnGridNodesCellLeftDClick);
	Connect(ID_GRID_NODES,wxEVT_GRID_CELL_RIGHT_DCLICK,(wxObjectEventFunction)&NodeSelectGrid::OnGridNodesCellRightDClick);
	//*)

    GridNodes->Connect(wxEVT_TEXT_CUT, (wxObjectEventFunction)&NodeSelectGrid::OnCut, 0, this);
    GridNodes->Connect(wxEVT_TEXT_COPY, (wxObjectEventFunction)&NodeSelectGrid::OnCopy, 0, this);
    GridNodes->Connect(wxEVT_TEXT_PASTE, (wxObjectEventFunction)&NodeSelectGrid::OnPaste, 0, this);

    Bind(DRAW_GRID_CLICKED, &NodeSelectGrid::OnDrawGridEvent, this, ID_GRID_NODES);

    GridNodes->EnableDragGridSize(false);
    GridNodes->DisableDragColSize();
    GridNodes->DisableDragRowSize();
    GridNodes->SetCellHighlightPenWidth(0);

    GridNodes->SetSelectionMode(wxGrid::wxGridSelectCells);

    renderer = new wxModelGridCellRenderer(bkg_image, *GridNodes);
    GridNodes->SetDefaultRenderer(renderer);

    LoadGrid(rows);

    GridNodes->BeginBatch();
    wxFont font = GridNodes->GetLabelFont();
    GridNodes->SetRowMinimalAcceptableHeight(5); //don't need to read text, just see the shape
    GridNodes->SetColMinimalAcceptableWidth(5); //don't need to read text, just see the shape
    GridNodes->SetLabelFont(font);
    font = GridNodes->GetDefaultCellFont();
    GridNodes->SetDefaultCellFont(font);
    for (int c = 0; c < GridNodes->GetNumberCols(); ++c)
        GridNodes->SetColSize(c, 2 * font.GetPixelSize().y); //GridCustom->GetColSize(c) * 5/4);
    for (int r = 0; r < GridNodes->GetNumberRows(); ++r)
        GridNodes->SetRowSize(r, int(1.5 * (float)font.GetPixelSize().y)); //GridCustom->GetRowSize(r) * 5/4);
    GridNodes->EndBatch();
    UpdateBackground();

    ValidateWindow();
}

NodeSelectGrid::~NodeSelectGrid()
{
	//(*Destroy(NodeSelectGrid)
	//*)
}

void NodeSelectGrid::LoadGrid(const std::vector<wxString>& rows)
{
    const std::vector<int> prevValue = DecodeNodeList(rows);

    float minsx = 99999;
    float minsy = 99999;
    float maxsx = -1;
    float maxsy = -1;

    const auto nodeCount = model->GetNodeCount();
    for (auto i = 0; i < nodeCount; i++)
    {
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
            GridNodes->SetCellValue(maxy - pts[0].y, pts[0].x - minx, wxString::Format("%i", i + 1));
            if (std::find(prevValue.begin(), prevValue.end(), i) != prevValue.end())
            {
                GridNodes->SetCellTextColour(maxy - pts[0].y, pts[0].x - minx, selectColor);
                GridNodes->SetCellBackgroundColour(maxy - pts[0].y, pts[0].x - minx, selectBackColor);
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
            const wxString value = GridNodes->GetCellValue(y, x);
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
            const wxString value = GridNodes->GetCellValue(y, x);
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
            const wxString value = GridNodes->GetCellValue(y, x);
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
            const wxString value = GridNodes->GetCellValue(y, x);
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
    const wxString value = GridNodes->GetCellValue(event.GetRow(), event.GetCol());
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
    const wxString value = GridNodes->GetCellValue(event.GetRow(), event.GetCol());
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
    const wxString value = GridNodes->GetCellValue(event.GetRow(), event.GetCol());
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

void NodeSelectGrid::OnButtonLoadModelClick(wxCommandEvent& event)
{
    const wxString filename = wxFileSelector(_("Choose Model file"), wxEmptyString, wxEmptyString, wxEmptyString, "xmodel files (*.xmodel)|*.xmodel", wxFD_OPEN);
    if (filename.IsEmpty()) return;
    ImportModel(filename);
}

void NodeSelectGrid::OnSliderImgBrightnessCmdScroll(wxScrollEvent& event)
{
    UpdateBackground();
    GridNodes->Refresh();
}

void NodeSelectGrid::OnButtonZoomPlusClick(wxCommandEvent& event)
{
    GridNodes->BeginBatch();
    wxFont font = GridNodes->GetLabelFont();
    font.MakeLarger();
    GridNodes->SetLabelFont(font);
    font = GridNodes->GetDefaultCellFont();
    font.MakeLarger();
    GridNodes->SetDefaultCellFont(font);
    for (int c = 0; c < GridNodes->GetNumberCols(); ++c)
        GridNodes->SetColSize(c, 2 * font.GetPixelSize().y); //GridCustom->GetColSize(c) * 5/4);
    for (int r = 0; r < GridNodes->GetNumberRows(); ++r)
        GridNodes->SetRowSize(r, int(1.5 * (float)font.GetPixelSize().y)); //GridCustom->GetRowSize(r) * 5/4);
    GridNodes->EndBatch();
    UpdateBackground();
}

void NodeSelectGrid::OnButtonZoomMinusClick(wxCommandEvent& event)
{
    GridNodes->BeginBatch();
    wxFont font = GridNodes->GetLabelFont();
    font.MakeSmaller();
    GridNodes->SetLabelFont(font);
    font = GridNodes->GetDefaultCellFont();
    font.MakeSmaller();
    GridNodes->SetDefaultCellFont(font);
    GridNodes->SetRowMinimalAcceptableHeight(5); //don't need to read text, just see the shape
    GridNodes->SetColMinimalAcceptableWidth(5); //don't need to read text, just see the shape
    for (int c = 0; c < GridNodes->GetNumberCols(); ++c)
        GridNodes->SetColSize(c, 2 * font.GetPixelSize().y); //GridCustom->GetColSize(c) * 4/5);
    for (int r = 0; r < GridNodes->GetNumberRows(); ++r)
        GridNodes->SetRowSize(r, int(1.5 * (float)font.GetPixelSize().y)); //GridCustom->GetRowSize(r) * 4/5);
    GridNodes->EndBatch();
    UpdateBackground();
}

void NodeSelectGrid::OnBitmapButton1Click(wxCommandEvent& event)
{
    bkgrd_active = !bkgrd_active;
    GridNodes->Refresh();
    UpdateBackground();
}

void NodeSelectGrid::OnFilePickerCtrl1FileChanged(wxFileDirPickerEvent& event)
{
    const wxString background_image = FilePickerCtrl1->GetFileName().GetFullPath();

    if (background_image != "")
    {
        if (wxFile::Exists(background_image))
        {
            bkg_image = new wxImage(background_image);
        }
        else
        {
            bkg_image = nullptr;
        }
        renderer->SetImage(bkg_image);
        UpdateBackground();
        GridNodes->Refresh();
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
            const wxString value = GridNodes->GetCellValue(y, x);
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
            const wxString value = GridNodes->GetCellValue(y, x);
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
        returnValue.insert(returnValue.begin(), row);
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
            const wxString value = GridNodes->GetCellValue(y, x);
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
        while (wtkz.HasMoreTokens())
        {
            wxString valstr = wtkz.GetNextToken();

            int start2, end2;
            if (valstr.Contains("-"))
            {
                const int idx = valstr.Index('-');
                start2 = wxAtoi(valstr.Left(idx));
                end2 = wxAtoi(valstr.Right(valstr.size() - idx - 1));
            }
            else
            {
                start2 = end2 = wxAtoi(valstr);
            }
            start2--;
            end2--;
            auto done = false;
            auto n = start2;
            while (!done)
            {
                if (n < model->GetNodeCount())
                {
                    nodeList.push_back(n);
                }
                if (start2 > end2)
                {
                    n--;
                    done = n < end2;
                }
                else
                {
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

        if (item != prevValue + 1)
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

//Import Model Shape From xModel File
void NodeSelectGrid::ImportModel(const std::string &filename)
{
    wxXmlDocument doc(filename);

    if (doc.IsOk())
    {
        wxXmlNode* root = doc.GetRoot();
        ImportModelXML(root);
    }
    else
    {
        wxMessageBox("Failure loading xModel file.");
    }
}

//Load Custom Model As Selection
void NodeSelectGrid::ImportModelXML(wxXmlNode* xmlData)
{
    if (xmlData->GetName() != "custommodel")
    {
        wxMessageBox("xModel file not a Custom Model.");
        return;
    }

    const auto customModel = xmlData->GetAttribute("CustomModel").ToStdString();
    const auto rows = wxSplit(customModel, ';');

    if (GridNodes->GetNumberRows() < rows.size())
    {
        wxMessageBox("xModel file dimensions are too big.");
        return;
    }

    const int height = rows.size();
    const int gridheight = GridNodes->GetNumberRows();

    const int rowOffset = ((gridheight - height) / 2);

    int row = 0;
    for (const auto& rv : rows)
    {
        const wxArrayString cols = wxSplit(rv, ',');
        if (cols.size() > GridNodes->GetNumberCols())
        {
            wxMessageBox("xModel file dimensions are too big.");
            return;
        }
        const int width = cols.size();
        const int gridhwidth = GridNodes->GetNumberCols();

        const int colOffset = ((gridhwidth - width) / 2);
        int col = 0;
        for (auto value : cols)
        {
            while (value.length() > 0 && value[0] == ' ')
            {
                value = value.substr(1);
            }

            if (!value.empty())
            {
                const wxString cellval = GridNodes->GetCellValue(row + rowOffset, col + colOffset);
                if (!cellval.IsNull() && !cellval.IsEmpty())
                {
                    GridNodes->SetCellTextColour(row + rowOffset, col + colOffset, selectColor);
                    GridNodes->SetCellBackgroundColour(row + rowOffset, col + colOffset, selectBackColor);
                }
            }
            col++;
        }
        row++;
    }
    GridNodes->Refresh();
    ValidateWindow();
}

void NodeSelectGrid::OnGridNodesCellRightClick(wxGridEvent& event)
{
    wxMenu mnu;
    // Copy / Paste / Delete
    wxMenuItem* menu_cut = mnu.Append(NODESELECT_CUT, "Cut");
    wxMenuItem* menu_copy = mnu.Append(NODESELECT_COPY, "Copy");
    wxMenuItem* menu_paste = mnu.Append(NODESELECT_PASTE, "Paste");

    mnu.Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&NodeSelectGrid::OnGridPopup, nullptr, this);
    PopupMenu(&mnu);
}

void NodeSelectGrid::OnCut(wxCommandEvent& event)
{
    CutOrCopyToClipboard(true);
}

void NodeSelectGrid::OnCopy(wxCommandEvent& event)
{
    CutOrCopyToClipboard(false);
}

void NodeSelectGrid::OnPaste(wxCommandEvent& event)
{
    Paste();
}

void NodeSelectGrid::OnGridPopup(wxCommandEvent& event)
{
    const auto id = event.GetId();
    if (id == NODESELECT_CUT)
    {
        CutOrCopyToClipboard(true);
    }
    else if (id == NODESELECT_COPY)
    {
        CutOrCopyToClipboard(false);
    }
    else if (id == NODESELECT_PASTE)
    {
        Paste();
    }
}

void NodeSelectGrid::CutOrCopyToClipboard(bool isCut)
{
    wxString copy_data;

    for (int i = 0; i< GridNodes->GetNumberRows(); i++)        // step through all lines
    {
        bool something_in_this_line = false;             // nothing found yet
        for (int k = 0; k<GridNodes->GetNumberCols(); k++)     // step through all colums
        {
            if (GridNodes->IsInSelection(i, k))     // this field is selected!!!
            {
                if (!something_in_this_line)        // first field in this line => may need a linefeed
                {
                    if (!copy_data.IsEmpty())       // ... if it is not the very first field
                    {
                        copy_data += "\n";     // next LINE
                    }
                    something_in_this_line = true;
                }
                else                                    // if not the first field in this line we need a field seperator (TAB)
                {
                    copy_data += "\t";  // next COLUMN
                }
                if (GridNodes->GetCellTextColour(i, k) == selectColor)
                {
                    copy_data += "X";    // finally we need the field value
                    if (isCut)
                    {
                        GridNodes->SetCellTextColour(i, k, unselectColor);
                        GridNodes->SetCellBackgroundColour(i, k, unselectBackColor);
                    }
                }
            }
        }
    }

    if (wxTheClipboard->Open())
    {
        if (!wxTheClipboard->SetData(new wxTextDataObject(copy_data)))
        {
            wxMessageBox(_("Unable to copy data to clipboard."), _("Error"));
        }
        wxTheClipboard->Close();
    }
    else
    {
        wxMessageBox(_("Error opening clipboard."), _("Error"));
    }
}

void NodeSelectGrid::Paste()
{
    wxString copy_data = "";

#ifdef __WXOSX__
    //wxDF_TEXT gets a very strange formatted string from the clipboard if using Numbers
    //native ObjectC code can get the proper tab formatted version.
    copy_data = GetOSXFormattedClipboardData();
#endif

    if (copy_data.empty())
    {
        if (wxTheClipboard->Open())
        {
            if (wxTheClipboard->IsSupported(wxDF_TEXT))
            {
                wxTextDataObject data;

                if (wxTheClipboard->GetData(data))
                {
                    copy_data = data.GetText();
                }
                else
                {
                    wxMessageBox(_("Unable to copy data from clipboard."), _("Error"));
                }
            }
            else
            {
                wxMessageBox(_("Non-Text data in clipboard."), _("Error"));
            }
            wxTheClipboard->Close();
        }
        else
        {
            wxMessageBox(_("Error opening clipboard."), _("Error"));
            return;
        }
    }

    int i = GridNodes->GetGridCursorRow();
    int k = GridNodes->GetGridCursorCol();
    const int numrows = GridNodes->GetNumberRows();
    const int numcols = GridNodes->GetNumberCols();

    copy_data.Replace("\r\r", "\n");
    copy_data.Replace("\r\n", "\n");
    copy_data.Replace("\r", "\n");

    do
    {
        wxString cur_line = copy_data.BeforeFirst('\n');
        copy_data = copy_data.AfterFirst('\n');
        wxArrayString fields = wxSplit(cur_line, (cur_line.Find(',') != wxNOT_FOUND) ? ',' : '\t'); //allow comma or tab delim -DJ
        for (int fieldnum = 0; fieldnum < fields.Count(); fieldnum++)
        {
            if (i < numrows && k + fieldnum < numcols)
            {
                wxString field = fields[fieldnum].Trim(true).Trim(false);
                if (!field.IsEmpty())
                {
                    //GridNodes->SetCellValue(i, k + fieldnum, fields[fieldnum].Trim(true).Trim(false)); //strip surrounding spaces -DJ
                    const wxString cellval = GridNodes->GetCellValue(i, k + fieldnum);
                    if (!cellval.IsNull() && !cellval.IsEmpty())
                    {
                        GridNodes->SetCellTextColour(i, k + fieldnum, selectColor);
                        GridNodes->SetCellBackgroundColour(i, k + fieldnum, selectBackColor);
                    }
                }
            }
        }
        i++;
    } while (copy_data.IsEmpty() == false);

    GridNodes->Refresh();
    ValidateWindow();
}

void NodeSelectGrid::UpdateBackground()
{
    if (renderer != nullptr && bkg_image != nullptr)
    {
        renderer->UpdateSize(*GridNodes, bkgrd_active, SliderImgBrightness->GetValue());
    }
}


