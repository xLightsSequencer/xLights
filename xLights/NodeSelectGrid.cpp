/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

//(*InternalHeaders(NodeSelectGrid)
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/artprov.h>
#include <wx/tokenzr.h>
#include <wx/settings.h>
#include <wx/settings.h>
#include <wx/xml/xml.h>
#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <wx/config.h>
#include <wx/clipbrd.h>
#include <wx/numdlg.h>

#include "NodeSelectGrid.h"
#include "models/Model.h"
#include "models/CustomModel.h"
#include "UtilFunctions.h"
#include "ExternalHooks.h"
#include "outputs/OutputManager.h"

//(*IdInit(NodeSelectGrid)
const long NodeSelectGrid::ID_CHECKBOX1 = wxNewId();
const long NodeSelectGrid::ID_BUTTON_SELECT_ALL = wxNewId();
const long NodeSelectGrid::ID_BUTTON_SELECT_NONE = wxNewId();
const long NodeSelectGrid::ID_BUTTON_INVERT_SELECT = wxNewId();
const long NodeSelectGrid::ID_BUTTON_LOAD_MODEL = wxNewId();
const long NodeSelectGrid::ID_BUTTON_ZOOM_PLUS = wxNewId();
const long NodeSelectGrid::ID_BUTTON_ZOOM_MINUS = wxNewId();
const long NodeSelectGrid::ID_FILEPICKERCTRL1 = wxNewId();
const long NodeSelectGrid::ID_SLIDER_IMG_BRIGHTNESS = wxNewId();
const long NodeSelectGrid::ID_BITMAPBUTTON1 = wxNewId();
const long NodeSelectGrid::ID_CHECKBOX2 = wxNewId();
const long NodeSelectGrid::ID_BUTTON_NODE_SELECT_OK = wxNewId();
const long NodeSelectGrid::ID_BUTTON_NODE_SELECT_CANCEL = wxNewId();
const long NodeSelectGrid::ID_GRID_NODES = wxNewId();
const long NodeSelectGrid::ID_TEXTCTRL1 = wxNewId();
//*)
const long NodeSelectGrid::ID_TIMER1 = wxNewId();

const long NodeSelectGrid::NODESELECT_CUT = wxNewId();
const long NodeSelectGrid::NODESELECT_COPY = wxNewId();
const long NodeSelectGrid::NODESELECT_PASTE = wxNewId();
const long NodeSelectGrid::NODESELECT_FIND = wxNewId();


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
    void DoOnChar(wxKeyEvent& event)
    {
        wxChar uc = event.GetUnicodeKey();

        switch (uc) {
        case WXK_SPACE: 
            if (m_currentCellCoords.GetRow() >= 0 && m_currentCellCoords.GetCol() >= 0) {
                wxGridEvent gridEvent(GetId(), wxEVT_GRID_CELL_LEFT_DCLICK, this, m_currentCellCoords.GetRow(), m_currentCellCoords.GetCol());
                wxPostEvent(this, gridEvent);
            }
            break;
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

    void HandleOnMouseWheel(wxMouseEvent& event)
    {
        m_wheelRotation += event.GetWheelRotation();
        int lines = m_wheelRotation / event.GetWheelDelta();
        m_wheelRotation -= lines * event.GetWheelDelta();

        if (lines != 0) {
            wxScrollWinEvent newEvent;

            newEvent.SetPosition(0);
            newEvent.SetOrientation(event.GetWheelAxis() == 0 ? wxVERTICAL : wxHORIZONTAL);

            if (event.ShiftDown()) {
                if (newEvent.GetOrientation() == wxVERTICAL) {
                    newEvent.SetOrientation(wxHORIZONTAL);
                } else {
                    newEvent.SetOrientation(wxVERTICAL);
                }
            }

            newEvent.SetEventObject(m_win);

            if (event.GetWheelAxis() == wxMOUSE_WHEEL_HORIZONTAL)
                lines = -lines;

            if (event.IsPageScroll()) {
                if (lines > 0)
                    newEvent.SetEventType(wxEVT_SCROLLWIN_PAGEUP);
                else
                    newEvent.SetEventType(wxEVT_SCROLLWIN_PAGEDOWN);

                m_win->GetEventHandler()->ProcessEvent(newEvent);
            } else {
                lines *= event.GetLinesPerAction();
                if (lines > 0)
                    newEvent.SetEventType(wxEVT_SCROLLWIN_LINEUP);
                else
                    newEvent.SetEventType(wxEVT_SCROLLWIN_LINEDOWN);

                int times = abs(lines);
                for (; times > 0; --times)
                    m_win->GetEventHandler()->ProcessEvent(newEvent);
            }
        }
    }

public:
    DrawGrid(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name) :
        wxGrid(parent, id, pos, size, style, name)
    {
        //Connect(wxEVT_CHAR, (wxObjectEventFunction)&DrawGrid::DoOnChar, 0, this);
        Connect(wxEVT_KEY_DOWN, (wxObjectEventFunction)&DrawGrid::DoOnChar, 0, this);
        Connect(wxEVT_MOUSEWHEEL, (wxObjectEventFunction)&DrawGrid::HandleOnMouseWheel, 0, this);
    }

    virtual ~DrawGrid()
    {
    }
};

//overloading constructor
NodeSelectGrid::NodeSelectGrid(bool multiline, const wxString &title, Model *m, const wxString& row, OutputManager* om, wxWindow* parent, wxWindowID id)
    : NodeSelectGrid(multiline, title, m, std::vector<wxString>(1, row), om, parent, id)
{
}

NodeSelectGrid::NodeSelectGrid(bool multiline, const wxString& title, Model* m, const std::vector<wxString>& rows, OutputManager* om, wxWindow* parent, wxWindowID id) :
    model(m), _outputManager(om)
{
    unselectColor = wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXTEXT);
    unselectBackColor = wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX);

    if (IsDarkMode()) {
        selectColor = *wxBLACK;
        selectBackColor = *wxLIGHT_GREY;
    } else {
        selectColor = *wxWHITE;
        selectBackColor = wxColour("grey");
    }


    //(*Initialize(NodeSelectGrid)
    wxBoxSizer* BoxSizer1;
    wxBoxSizer* BoxSizer2;
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer4;
    wxFlexGridSizer* FlexGridSizer5;
    wxFlexGridSizer* wxFlexSizerMain;
    wxStaticBoxSizer* StaticBoxSizer1;
    wxStaticBoxSizer* StaticBoxSizer2;
    wxStaticBoxSizer* StaticBoxSizer3;

    Create(parent, wxID_ANY, _("Select Nodes"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxCLOSE_BOX|wxMAXIMIZE_BOX|wxMINIMIZE_BOX, _T("wxID_ANY"));
    SetClientSize(wxDLG_UNIT(parent,wxSize(432,192)));
    SetMinSize(wxSize(-1,-1));
    wxFlexSizerMain = new wxFlexGridSizer(0, 2, 0, 0);
    wxFlexSizerMain->AddGrowableCol(1);
    wxFlexSizerMain->AddGrowableRow(0);
    FlexGridSizer4 = new wxFlexGridSizer(0, 1, 0, 0);
    StaticBoxSizer3 = new wxStaticBoxSizer(wxVERTICAL, this, _("Selection"));
    FlexGridSizer5 = new wxFlexGridSizer(0, 1, 0, 0);
    CheckBox_OrderedSelection = new wxCheckBox(this, ID_CHECKBOX1, _("Ordered Selection"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
    CheckBox_OrderedSelection->SetValue(false);
    FlexGridSizer5->Add(CheckBox_OrderedSelection, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    ButtonSelectAll = new wxButton(this, ID_BUTTON_SELECT_ALL, _("Select All"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SELECT_ALL"));
    FlexGridSizer3->Add(ButtonSelectAll, 1, wxALL|wxEXPAND, 5);
    ButtonSelectNone = new wxButton(this, ID_BUTTON_SELECT_NONE, _("Select None"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SELECT_NONE"));
    FlexGridSizer3->Add(ButtonSelectNone, 1, wxALL|wxEXPAND, 5);
    ButtonInvertSelect = new wxButton(this, ID_BUTTON_INVERT_SELECT, _("Invert"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_INVERT_SELECT"));
    FlexGridSizer3->Add(ButtonInvertSelect, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonLoadModel = new wxButton(this, ID_BUTTON_LOAD_MODEL, _("From Model"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_LOAD_MODEL"));
    FlexGridSizer3->Add(ButtonLoadModel, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer5->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer3->Add(FlexGridSizer5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer4->Add(StaticBoxSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer2 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Zoom"));
    ButtonZoomPlus = new wxButton(this, ID_BUTTON_ZOOM_PLUS, _("+"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_ZOOM_PLUS"));
    StaticBoxSizer2->Add(ButtonZoomPlus, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ButtonZoomMinus = new wxButton(this, ID_BUTTON_ZOOM_MINUS, _("-"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_ZOOM_MINUS"));
    StaticBoxSizer2->Add(ButtonZoomMinus, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer4->Add(StaticBoxSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer1 = new wxStaticBoxSizer(wxVERTICAL, this, _("Background Image"));
    FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
    FilePickerCtrl1 = new ImageFilePickerCtrl(this, ID_FILEPICKERCTRL1, wxEmptyString, wxEmptyString, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxFLP_FILE_MUST_EXIST|wxFLP_OPEN|wxFLP_USE_TEXTCTRL, wxDefaultValidator, _T("ID_FILEPICKERCTRL1"));
    FlexGridSizer1->Add(FilePickerCtrl1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SliderImgBrightness = new wxSlider(this, ID_SLIDER_IMG_BRIGHTNESS, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_IMG_BRIGHTNESS"));
    FlexGridSizer1->Add(SliderImgBrightness, 1, wxALL|wxEXPAND, 2);
    BitmapButton1 = new wxBitmapButton(this, ID_BITMAPBUTTON1,  wxArtProvider::GetBitmapBundle("wxART_FIND", wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON1"));
    FlexGridSizer1->Add(BitmapButton1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer1->Add(FlexGridSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxSHAPED|wxFIXED_MINSIZE, 0);
    FlexGridSizer4->Add(StaticBoxSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_OutputToLights = new wxCheckBox(this, ID_CHECKBOX2, _("Output to Lights"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
    CheckBox_OutputToLights->SetValue(false);
    FlexGridSizer4->Add(CheckBox_OutputToLights, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    ButtonNodeSelectOK = new wxButton(this, ID_BUTTON_NODE_SELECT_OK, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_NODE_SELECT_OK"));
    BoxSizer1->Add(ButtonNodeSelectOK, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 5);
    ButtonNodeSelectCancel = new wxButton(this, ID_BUTTON_NODE_SELECT_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_NODE_SELECT_CANCEL"));
    BoxSizer1->Add(ButtonNodeSelectCancel, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 5);
    FlexGridSizer4->Add(BoxSizer1, 0, wxALL, 5);
    wxFlexSizerMain->Add(FlexGridSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BoxSizer2 = new wxBoxSizer(wxVERTICAL);
    GridNodes = new DrawGrid(this, ID_GRID_NODES, wxDefaultPosition, wxDefaultSize, 0, _T("ID_GRID_NODES"));
    GridNodes->CreateGrid(1,1);
    GridNodes->EnableEditing(false);
    GridNodes->EnableGridLines(true);
    GridNodes->SetColLabelSize(20);
    GridNodes->SetRowLabelSize(30);
    GridNodes->SetDefaultColSize(30, true);
    GridNodes->SetDefaultCellFont( GridNodes->GetFont() );
    GridNodes->SetDefaultCellTextColour( GridNodes->GetForegroundColour() );
    BoxSizer2->Add(GridNodes, 1, wxALL|wxEXPAND, 5);
    TextCtrl_Nodes = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
    BoxSizer2->Add(TextCtrl_Nodes, 0, wxALL|wxEXPAND, 5);
    wxFlexSizerMain->Add(BoxSizer2, 1, wxALL|wxEXPAND, 5);
    SetSizer(wxFlexSizerMain);
    SetSizer(wxFlexSizerMain);
    Layout();

    Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&NodeSelectGrid::OnCheckBox_OrderedSelectionClick);
    Connect(ID_BUTTON_SELECT_ALL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&NodeSelectGrid::OnButtonSelectAllClick);
    Connect(ID_BUTTON_SELECT_NONE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&NodeSelectGrid::OnButtonSelectNoneClick);
    Connect(ID_BUTTON_INVERT_SELECT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&NodeSelectGrid::OnButtonInvertSelectClick);
    Connect(ID_BUTTON_LOAD_MODEL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&NodeSelectGrid::OnButtonLoadModelClick);
    Connect(ID_BUTTON_ZOOM_PLUS,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&NodeSelectGrid::OnButtonZoomPlusClick);
    Connect(ID_BUTTON_ZOOM_MINUS,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&NodeSelectGrid::OnButtonZoomMinusClick);
    Connect(ID_FILEPICKERCTRL1,wxEVT_COMMAND_FILEPICKER_CHANGED,(wxObjectEventFunction)&NodeSelectGrid::OnFilePickerCtrl1FileChanged);
    Connect(ID_SLIDER_IMG_BRIGHTNESS,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&NodeSelectGrid::OnSliderImgBrightnessCmdScroll);
    Connect(ID_BITMAPBUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&NodeSelectGrid::OnBitmapButton1Click);
    Connect(ID_CHECKBOX2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&NodeSelectGrid::OnCheckBox_OutputToLightsClick);
    Connect(ID_BUTTON_NODE_SELECT_OK,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&NodeSelectGrid::OnButtonNodeSelectOKClick);
    Connect(ID_BUTTON_NODE_SELECT_CANCEL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&NodeSelectGrid::OnButtonNodeSelectCancelClick);
    Connect(ID_GRID_NODES,wxEVT_GRID_CELL_RIGHT_CLICK,(wxObjectEventFunction)&NodeSelectGrid::OnGridNodesCellRightClick);
    Connect(ID_GRID_NODES,wxEVT_GRID_CELL_LEFT_DCLICK,(wxObjectEventFunction)&NodeSelectGrid::OnGridNodesCellLeftDClick);
    Connect(ID_GRID_NODES,wxEVT_GRID_CELL_RIGHT_DCLICK,(wxObjectEventFunction)&NodeSelectGrid::OnGridNodesCellRightDClick);
    Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&NodeSelectGrid::OnTextCtrl_NodesText);
    //*)

    Connect(wxEVT_SIZE, (wxObjectEventFunction)&NodeSelectGrid::OnResize);

    TextCtrl_Nodes->Bind(wxEVT_KILL_FOCUS, &NodeSelectGrid::OnTextCtrl_NodesLoseFocus, this);

    GridNodes->Connect(wxEVT_TEXT_CUT, (wxObjectEventFunction)&NodeSelectGrid::OnCut, 0, this);
    GridNodes->Connect(wxEVT_TEXT_COPY, (wxObjectEventFunction)&NodeSelectGrid::OnCopy, 0, this);
    GridNodes->Connect(wxEVT_TEXT_PASTE, (wxObjectEventFunction)&NodeSelectGrid::OnPaste, 0, this);
    GridNodes->Connect(wxEVT_KEY_DOWN, (wxObjectEventFunction)&NodeSelectGrid::OnKeyDown, 0, this);

#if defined(EVT_GRID_CMD_RANGE_SELECTED)
    GridNodes->Connect(wxEVT_GRID_RANGE_SELECTED, (wxObjectEventFunction)&NodeSelectGrid::OnGridNodesCellSelect, 0, this);
#else
    GridNodes->Connect(wxEVT_GRID_RANGE_SELECT, (wxObjectEventFunction)&NodeSelectGrid::OnGridNodesCellSelect, 0, this);
#endif

    GridNodes->DisableDragColSize();
    GridNodes->DisableDragRowSize();

    GridNodes->SetSelectionMode(wxGrid::wxGridSelectCells);

    GridNodes->DisableCellEditControl();
    GridNodes->DisableDragGridSize();

    renderer = new wxModelGridCellRenderer(bkg_image, *GridNodes);
    GridNodes->SetDefaultRenderer(renderer);

    if (!title.IsEmpty())
        SetTitle(GetTitle() + " - " + title);

    if (multiline) {
        CheckBox_OrderedSelection->Disable();
    } else {
        bool checked = true;
        wxConfigBase* config = wxConfigBase::Get();
        if (config != nullptr) {
            checked = config->ReadBool("NodeSelectGridOrderedSelection", true);
        }
        CheckBox_OrderedSelection->SetValue(checked);
    }

    wxPoint loc;
    wxSize sz;
    LoadWindowPosition("xLightsNodeSelectDialogPosition", sz, loc);
    if (loc.x != -1) {
        if (sz.GetWidth() < 400)
            sz.SetWidth(400);
        if (sz.GetHeight() < 200)
            sz.SetHeight(200);
        SetPosition(loc);
        SetSize(sz);
        Layout();
    }
    EnsureWindowHeaderIsOnScreen(this);

    LoadGrid(rows);

    GridNodes->BeginBatch();

    wxFont font = GridNodes->GetLabelFont();
    int fontsize = LoadInt("xLightsNodeSelectDialogZoom", font.GetPointSize());
    if (fontsize == 0) {
        fontsize = 8;
    }

    font.SetPointSize(fontsize);
    GridNodes->SetLabelFont(font);

    font = GridNodes->GetDefaultCellFont();
    font.SetPointSize(fontsize);
    GridNodes->SetDefaultCellFont(font);

    GridNodes->EndBatch();

    SetGridSizeForFont(font);
    UpdateBackground();

    ValidateWindow();
}

NodeSelectGrid::~NodeSelectGrid()
{
    SaveWindowPosition("xLightsNodeSelectDialogPosition", this);
    auto font = GridNodes->GetDefaultCellFont();
    SaveInt("xLightsNodeSelectDialogZoom", font.GetPointSize());
	//(*Destroy(NodeSelectGrid)
	//*)

    StopOutputToLights();
}

void NodeSelectGrid::LoadGrid(const std::vector<wxString>& rows)
{
    const std::vector<int> prevValue = DecodeNodeList(rows);

    float minsx = 99999;
    float minsy = 99999;
    float maxsx = -1;
    float maxsy = -1;

    const uint32_t nodeCount = model->GetNodeCount();

    if (nodeCount == 0) {
        return;
    }
    for (uint32_t i = 0; i < nodeCount; ++i) {
        std::vector<wxPoint> pts;
        model->GetNodeCoords(i, pts);
        if (pts.size() > 0) {
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

    for (uint32_t i = 0; i < nodeCount; ++i) {
        std::vector<wxPoint> pts;
        model->GetNodeCoords(i, pts);
        if (pts.size() > 0)
        {
            GridNodes->SetCellValue(maxy - pts[0].y, pts[0].x - minx, wxString::Format("%i", i + 1));
            if (std::find(prevValue.begin(), prevValue.end(), i) != prevValue.end())
            {
                SelectNode(true, maxy - pts[0].y, pts[0].x - minx, wxAtoi(GridNodes->GetCellValue(maxy - pts[0].y, pts[0].x - minx)));
            }
        }
    }
    GridNodes->Refresh();
    if (CheckBox_OrderedSelection->IsChecked() && rows.size() == 1) {
        TextCtrl_Nodes->SetValue(ExpandNodes(rows.front()));
        TextCtrl_Nodes->SetInsertionPointEnd();
    } else {
        UpdateTextFromGrid();
    }
}

void NodeSelectGrid::ValidateWindow() const
{
    if (CheckBox_OrderedSelection->IsChecked()) {
        ButtonLoadModel->Disable();
        ButtonSelectAll->Disable();
        TextCtrl_Nodes->Enable();
    } else {
        ButtonLoadModel->Enable();
        ButtonSelectAll->Enable();
        TextCtrl_Nodes->Disable();
    }

    for (auto x = 0; x < GridNodes->GetNumberCols(); ++x) {
        for (auto y = 0; y < GridNodes->GetNumberRows(); ++y) {
            const wxString value = GridNodes->GetCellValue(y, x);
            if (!value.IsNull() && !value.IsEmpty()) {
                if (GridNodes->GetCellTextColour(y, x) == selectColor) {
                    ButtonNodeSelectOK->Enable(true);
                    return;
                }
            }
        }
    }
    ButtonNodeSelectOK->Enable(false);
}

void NodeSelectGrid::OnButtonSelectAllClick(wxCommandEvent& event)
{
    for (auto x = 0; x < GridNodes->GetNumberCols(); ++x) {
        for (auto y = 0; y < GridNodes->GetNumberRows(); ++y) {
            const wxString value = GridNodes->GetCellValue(y, x);
            if (!value.IsNull() && !value.IsEmpty()) {
                SelectNode(true, y, x, wxAtoi(value));
            }
        }
    }
    UpdateTextFromGrid();
    GridNodes->ClearSelection();
    GridNodes->Refresh();
    ValidateWindow();
}

void NodeSelectGrid::OnButtonSelectNoneClick(wxCommandEvent& event)
{
    for (auto x = 0; x < GridNodes->GetNumberCols(); ++x) {
        for (auto y = 0; y < GridNodes->GetNumberRows(); ++y) {
            const wxString value = GridNodes->GetCellValue(y, x);
            if (!value.IsNull() && !value.IsEmpty()) {
                SelectNode(false, y, x, wxAtoi(value));
            }
        }
    }
    TextCtrl_Nodes->SetValue("");
    GridNodes->ClearSelection();
    GridNodes->Refresh();
    ValidateWindow();
}

void NodeSelectGrid::OnButtonInvertSelectClick(wxCommandEvent& event)
{
    for (auto x = 0; x < GridNodes->GetNumberCols(); ++x) {
        for (auto y = 0; y < GridNodes->GetNumberRows(); ++y) {
            const wxString value = GridNodes->GetCellValue(y, x);
            if (!value.IsNull() && !value.IsEmpty()) {
                if (GridNodes->GetCellTextColour(y, x) == selectColor) {
                    SelectNode(false, y, x, wxAtoi(value));
                } else {
                    SelectNode(true, y, x, wxAtoi(value));
                }
            }
        }
    }
    UpdateTextFromGrid();
    GridNodes->ClearSelection();
    GridNodes->Refresh();
    ValidateWindow();
}

void NodeSelectGrid::OnButtonNodeSelectOKClick(wxCommandEvent& event)
{
    SaveSettings();
    EndDialog(wxID_OK);
}

void NodeSelectGrid::OnButtonNodeSelectCancelClick(wxCommandEvent& event)
{
    SaveSettings();
    EndDialog(wxID_CANCEL);
}

void NodeSelectGrid::OnGridNodesCellLeftDClick(wxGridEvent& event)
{
    const wxString value = GridNodes->GetCellValue(event.GetRow(), event.GetCol());
    if (!value.IsNull() && !value.IsEmpty()) {
        if (GridNodes->GetCellTextColour(event.GetRow(), event.GetCol()) == selectColor) {
            SelectNode(false, event.GetRow(), event.GetCol(), wxAtoi(value), true);
        } else {
            SelectNode(true, event.GetRow(), event.GetCol(), wxAtoi(value), true);
        }
        UpdateTextFromGrid();
        GridNodes->Refresh();
        ValidateWindow();
    }
}

void NodeSelectGrid::SelectNode(bool select, int row, int col, int node, bool addRemove)
{
    auto loc = std::find(begin(_selected), end(_selected), node);
    if (select) {
        GridNodes->SetCellTextColour(row, col, selectColor);
        GridNodes->SetCellBackgroundColour(row, col, selectBackColor);
        if (addRemove) AddNode(col, row);
        if (loc == end(_selected)) {
            _selected.push_back(node);
        }
    } else {
        GridNodes->SetCellTextColour(row, col, unselectColor);
        GridNodes->SetCellBackgroundColour(row, col, unselectBackColor);
        if (addRemove) RemoveNode(col, row);
        if (loc != end(_selected))
            _selected.erase(loc);
    }
}

void NodeSelectGrid::OnGridNodesCellRightDClick(wxGridEvent& event)
{
    const wxString value = GridNodes->GetCellValue(event.GetRow(), event.GetCol());
    if (!value.IsNull() && !value.IsEmpty()) {
        if (event.ShiftDown()) {
            SelectNode(false, event.GetRow(), event.GetCol(), wxAtoi(value), true);
        } else {
            SelectNode(true, event.GetRow(), event.GetCol(), wxAtoi(value), true);
        }

        UpdateTextFromGrid();
        GridNodes->Refresh();
        ValidateWindow();
    }
}

void NodeSelectGrid::OnGridNodesCellSelect(wxGridRangeSelectEvent& event)
{
    if (CheckBox_OrderedSelection->IsChecked()) {
        event.Skip();
        return;
    }
    //skip while mouse is dragging out
    if (!event.Selecting()) {
        event.Skip();
        return;
    }
    //skip selecting for Cut, Copy, & Paste
    if (event.CmdDown() || event.ControlDown() || event.ShiftDown()) {
        event.Skip();
        return;
    }

    //select highlighted cells
    for (auto x = event.GetLeftCol(); x <= event.GetRightCol(); ++x) {
        for (auto y = event.GetTopRow(); y <= event.GetBottomRow(); ++y) {
            const wxString value = GridNodes->GetCellValue(y, x);
            if (!value.IsNull() && !value.IsEmpty()) {
                if (GridNodes->GetCellTextColour(y, x) == selectColor) {
                    SelectNode(false, y, x, wxAtoi(value));
                } else {
                    SelectNode(true, y, x, wxAtoi(value));
                }
            }
        }
    }
    GridNodes->ClearSelection();
    GridNodes->Refresh();
    UpdateTextFromGrid();
    ValidateWindow();
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

void NodeSelectGrid::SetGridSizeForFont(const wxFont& font)
{
    GridNodes->Freeze();
    GridNodes->BeginBatch();
    GridNodes->SetRowMinimalAcceptableHeight(5); //don't need to read text, just see the shape
    GridNodes->SetColMinimalAcceptableWidth(5); //don't need to read text, just see the shape
    for (int c = 0; c < GridNodes->GetNumberCols(); ++c) {
        GridNodes->SetColSize(c, 2 * font.GetPixelSize().y); //GridCustom->GetColSize(c) * 5/4);
    }
    for (int r = 0; r < GridNodes->GetNumberRows(); ++r) {
        GridNodes->SetRowSize(r, int(1.5 * (float)font.GetPixelSize().y)); //GridCustom->GetRowSize(r) * 5/4);
    }

    GridNodes->SetDefaultColSize(2 * font.GetPixelSize().y);
    GridNodes->SetColLabelSize(int(1.5 * (float)font.GetPixelSize().y));
    GridNodes->SetRowLabelSize(int(2.5 * font.GetPixelSize().y));

    GridNodes->EndBatch();
    GridNodes->Thaw();
}

void NodeSelectGrid::OnButtonZoomPlusClick(wxCommandEvent& event)
{
    GridNodes->Freeze();
    GridNodes->BeginBatch();

    wxFont font = GridNodes->GetLabelFont();
    font.MakeLarger();
    GridNodes->SetLabelFont(font);
    auto cfs = font.GetPointSize();
    font = GridNodes->GetDefaultCellFont();
    font.SetPointSize(cfs);
    GridNodes->SetDefaultCellFont(font);

    GridNodes->EndBatch();
    GridNodes->Thaw();

    SetGridSizeForFont(font);

    UpdateBackground();

    Layout();
    Refresh();
}

void NodeSelectGrid::OnButtonZoomMinusClick(wxCommandEvent& event)
{
    GridNodes->Freeze();
    GridNodes->BeginBatch();

    auto font = GridNodes->GetDefaultCellFont();
    auto cfs = font.GetPointSize();
    font.MakeSmaller();
    if (font.GetPointSize() != cfs) {
        cfs = font.GetPointSize();
        GridNodes->SetDefaultCellFont(font);
        font = GridNodes->GetLabelFont();
        font.SetPointSize(cfs);
        GridNodes->SetLabelFont(font);
    }
    GridNodes->EndBatch();
    GridNodes->Thaw();

    font = GridNodes->GetDefaultCellFont();
    SetGridSizeForFont(font);

    UpdateBackground();

    Layout();
    Refresh();
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

    if (background_image != "") {
        if (FileExists(background_image)) {
            bkg_image = new wxImage(background_image);
        } else {
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

    for (auto x = 0; x < GridNodes->GetNumberCols(); ++x) {
        for (auto y = 0; y < GridNodes->GetNumberRows(); ++y) {
            const wxString value = GridNodes->GetCellValue(y, x);
            if (!value.IsNull() && !value.IsEmpty()) {
                if (GridNodes->GetCellTextColour(y, x) == selectColor) {
                    startx = std::min(startx, x);
                    starty = std::min(starty, y);
                    endx = std::max(endx, x);
                    endy = std::max(endy, y);
                }
            }
        }
    }

    for (auto y = starty; y <= endy; ++y) {
        wxString row;
        for (auto x = startx; x <= endx; ++x) {
            const wxString value = GridNodes->GetCellValue(y, x);
            if (!value.IsEmpty() && GridNodes->GetCellTextColour(y, x) == selectColor) {
                row = row + value + ",";
            } else {
                row = row + ",";
            }
        }
        row.erase(row.length() - 1, 1);
        returnValue.insert(returnValue.begin(), row);
    }
    return returnValue;
}

wxString NodeSelectGrid::GetNodeList(const bool sort)
{
    if (CheckBox_OrderedSelection->IsChecked())
        return CompressNodes(TextCtrl_Nodes->GetValue());

    // encode with dashes
    std::vector<wxString> nodeList;
    for (auto x = 0; x < GridNodes->GetNumberCols(); ++x) {
        for (auto y = 0; y < GridNodes->GetNumberRows(); ++y) {
            const wxString value = GridNodes->GetCellValue(y, x);
            if (!value.IsNull() && !value.IsEmpty() && GridNodes->GetCellTextColour(y, x) == selectColor) {
                nodeList.push_back(value);
            }
        }
    }
    // encode with dashes
    return EncodeNodeLine(nodeList, sort);
}

std::vector<int> NodeSelectGrid::DecodeNodeList(const std::vector<wxString>& rows) const
{
    std::vector<int> nodeList;
    for (const auto& row : rows) {
        wxStringTokenizer wtkz(row, ",");
        while (wtkz.HasMoreTokens()) {
            wxString valstr = wtkz.GetNextToken();

            int start2, end2;
            if (valstr.Contains("-")) {
                const int idx = valstr.Index('-');
                start2 = wxAtoi(valstr.Left(idx));
                end2 = wxAtoi(valstr.Right(valstr.size() - idx - 1));
            } else {
                start2 = end2 = wxAtoi(valstr);
            }
            --start2;
            --end2;
            auto done = false;
            auto n = start2;
            while (!done) {
                if (n < (int)model->GetNodeCount()) {
                    nodeList.push_back(n);
                }
                if (start2 > end2) {
                    --n;
                    done = n < end2;
                } else {
                    ++n;
                    done = n > end2;
                }
            }
        }
    }
    return nodeList;
}

//encode node list with dashes
wxString NodeSelectGrid::EncodeNodeLine(const std::vector<wxString>& nodes, const bool sort) const
{
    wxString rowValue;
    std::vector<int> iNodes;
    std::transform(nodes.begin(), nodes.end(), std::back_inserter(iNodes),
                   [](const std::string& str) { return std::stoi(str); });

    if (sort)
        std::sort(iNodes.begin(), iNodes.end());
    iNodes.erase(std::unique(iNodes.begin(), iNodes.end()), iNodes.end());

    int firstValue = -1;
    int prevValue = -1;
    for (auto& item : iNodes) {
        if (&item == &iNodes.front()) {
            firstValue = prevValue = item;
            continue;
        }

        if (item != prevValue + 1 && item != prevValue - 1) {
            if (firstValue != prevValue)
                rowValue += wxString::Format(wxT("%i-%i,"), firstValue, prevValue);
            else
                rowValue += wxString::Format(wxT("%i,"), prevValue);
            firstValue = item;
        }

        if (&item == &iNodes.back()) {
            if (item == prevValue + 1 || item == prevValue - 1)
                rowValue += wxString::Format(wxT("%i-%i"), firstValue, item);
            else
                rowValue += wxString::Format(wxT("%i"), item);
        }
        prevValue = item;
    }
    if (rowValue == "" && nodes.size() > 0)
        rowValue = nodes.front();
    return rowValue;
}

//Import Model Shape From xModel File
void NodeSelectGrid::ImportModel(const std::string& filename)
{
    wxXmlDocument doc(filename);

    if (doc.IsOk()) {
        wxXmlNode* root = doc.GetRoot();
        ImportModelXML(root);
    } else {
        DisplayError("Failure loading xModel file.");
    }
}

//Load Custom Model As Selection
void NodeSelectGrid::ImportModelXML(wxXmlNode* xmlData)
{
    if (xmlData->GetName() != "custommodel") {
        DisplayError("xModel file not a Custom Model.");
        return;
    }

    const auto customModel = xmlData->GetAttribute("CustomModel").ToStdString();
    const auto rows = wxSplit(customModel, ';');

    if (GridNodes->GetNumberRows() < rows.size()) {
        DisplayError("xModel file dimensions are too big.");
        return;
    }

    const int height = rows.size();
    const int gridheight = GridNodes->GetNumberRows();

    const int rowOffset = ((gridheight - height) / 2);

    int row = 0;
    for (const auto& rv : rows) {
        const wxArrayString cols = wxSplit(rv, ',');
        if (cols.size() > GridNodes->GetNumberCols()) {
            DisplayError("xModel file dimensions are too big.");
            return;
        }
        const int width = cols.size();
        const int gridhwidth = GridNodes->GetNumberCols();

        const int colOffset = ((gridhwidth - width) / 2);
        int col = 0;
        for (auto value : cols) {
            while (value.length() > 0 && value[0] == ' ') {
                value = value.substr(1);
            }

            if (!value.empty()) {
                const wxString cellval = GridNodes->GetCellValue(row + rowOffset, col + colOffset);
                if (!cellval.IsNull() && !cellval.IsEmpty()) {
                    SelectNode(true, row + rowOffset, col + colOffset, wxAtoi(value));
                }
            }
            ++col;
        }
        ++row;
    }
    UpdateTextFromGrid();
    GridNodes->Refresh();
    ValidateWindow();
}

void NodeSelectGrid::OnGridNodesCellRightClick(wxGridEvent& event)
{
    wxMenu mnu;
    if (!CheckBox_OrderedSelection->IsChecked()) {
        // Copy / Paste / Delete
        mnu.Append(NODESELECT_CUT, "Cut");
        mnu.Append(NODESELECT_COPY, "Copy");
        mnu.Append(NODESELECT_PASTE, "Paste");
        mnu.AppendSeparator();
    }

    mnu.Append(NODESELECT_FIND, "Find Node");

    mnu.Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&NodeSelectGrid::OnGridPopup, nullptr, this);
    PopupMenu(&mnu);
}

void NodeSelectGrid::OnResize(wxSizeEvent& event)
{
    Layout();
    Refresh();
}

void NodeSelectGrid::OnCut(wxCommandEvent& event)
{
    if (CheckBox_OrderedSelection->IsChecked()) return;
    CutOrCopyToClipboard(true);
}

void NodeSelectGrid::OnCopy(wxCommandEvent& event)
{
    if (CheckBox_OrderedSelection->IsChecked()) return;
    CutOrCopyToClipboard(false);
}

void NodeSelectGrid::OnPaste(wxCommandEvent& event)
{
    if (CheckBox_OrderedSelection->IsChecked()) return;
    Paste();
}

void NodeSelectGrid::OnGridPopup(wxCommandEvent& event)
{
    const auto id = event.GetId();
    if (id == NODESELECT_CUT) {
        CutOrCopyToClipboard(true);
    } else if (id == NODESELECT_COPY) {
        CutOrCopyToClipboard(false);
    } else if (id == NODESELECT_PASTE) {
        Paste();
    } else if (id == NODESELECT_FIND) {
        Find();
    }
}

void NodeSelectGrid::Find()
{
    long minNode;
    long maxNode;
    GetMinMaxNode(minNode, maxNode);

    if (minNode == 0) {
        wxMessageBox("No nodes present.");
        return;
    }

    wxNumberEntryDialog dlg(this, "Node to find.", "Node to find", "Node", 0, minNode, maxNode);
    if (dlg.ShowModal() == wxID_OK) {
        auto find = dlg.GetValue();

        for (auto c = 0; c < GridNodes->GetNumberCols(); c++) {
            for (auto r = 0; r < GridNodes->GetNumberRows(); ++r) {
                wxString s = GridNodes->GetCellValue(r, c);
                if (s.IsEmpty() == false) {
                    long v;
                    if (s.ToCLong(&v) == true) {
                        if (v == find) {
                            // make this sell active
                            GridNodes->SetGridCursor(r, c);
                            GridNodes->MakeCellVisible(r, c);
                            return;
                        }
                    }
                }
            }
        }
    }
}

void NodeSelectGrid::CutOrCopyToClipboard(bool isCut)
{
    wxString copy_data;

    for (int i = 0; i < GridNodes->GetNumberRows(); ++i) // step through all lines
    {
        bool something_in_this_line = false;                 // nothing found yet
        for (int k = 0; k < GridNodes->GetNumberCols(); ++k) // step through all columns
        {
            if (GridNodes->IsInSelection(i, k)) // this field is selected!!!
            {
                if (!something_in_this_line) // first field in this line => may need a linefeed
                {
                    if (!copy_data.IsEmpty()) // ... if it is not the very first field
                    {
                        copy_data += "\n"; // next LINE
                    }
                    something_in_this_line = true;
                } else // if not the first field in this line we need a field separator (TAB)
                {
                    copy_data += "\t"; // next COLUMN
                }
                if (GridNodes->GetCellTextColour(i, k) == selectColor) {
                    copy_data += "X"; // finally we need the field value
                    if (isCut) {
                        SelectNode(false, i, k, wxAtoi(GridNodes->GetCellValue(i, k)));
                    }
                }
            }
        }
    }

    if (isCut) {
        UpdateTextFromGrid();
    }

    if (wxTheClipboard->Open()) {
        if (!wxTheClipboard->SetData(new wxTextDataObject(copy_data))) {
            DisplayError(_("Unable to copy data to clipboard."));
        }
        wxTheClipboard->Close();
    } else {
        DisplayError(_("Error opening clipboard."));
    }
}

void NodeSelectGrid::Paste()
{
    wxString copy_data = "";

    // wxDF_TEXT gets a very strange formatted string from the clipboard if using Numbers
    // native ObjectC code can get the proper tab formatted version.
    copy_data = GetOSFormattedClipboardData();

    if (copy_data.empty()) {
        if (wxTheClipboard->Open()) {
            if (wxTheClipboard->IsSupported(wxDF_TEXT)) {
                wxTextDataObject data;

                if (wxTheClipboard->GetData(data)) {
                    copy_data = data.GetText();
                } else {
                    DisplayError(_("Unable to copy data from clipboard."));
                }
            } else {
                DisplayError(_("Non-Text data in clipboard."));
            }
            wxTheClipboard->Close();
        } else {
            DisplayError(_("Error opening clipboard."));
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

    do {
        wxString cur_line = copy_data.BeforeFirst('\n');
        copy_data = copy_data.AfterFirst('\n');
        wxArrayString fields = wxSplit(cur_line, (cur_line.Find(',') != wxNOT_FOUND) ? ',' : '\t'); // allow comma or tab delim -DJ
        for (int fieldnum = 0; fieldnum < fields.Count(); ++fieldnum) {
            if (i < numrows && k + fieldnum < numcols) {
                wxString field = fields[fieldnum].Trim(true).Trim(false);
                if (!field.IsEmpty()) {
                    const wxString cellval = GridNodes->GetCellValue(i, k + fieldnum);
                    if (!cellval.IsNull() && !cellval.IsEmpty()) {
                        SelectNode(true, i, k + fieldnum, wxAtoi(cellval));
                    }
                }
            }
        }
        ++i;
    } while (copy_data.IsEmpty() == false);

    UpdateTextFromGrid();
    GridNodes->Refresh();
    ValidateWindow();
}

void NodeSelectGrid::UpdateBackground()
{
    if (renderer != nullptr && bkg_image != nullptr) {
        renderer->UpdateSize(*GridNodes, bkgrd_active, SliderImgBrightness->GetValue());

        for (int i = 0; i < GridNodes->GetNumberRows(); ++i) // step through all lines
        {
            for (int k = 0; k < GridNodes->GetNumberCols(); ++k) // step through all columns
            {
                const wxString value = GridNodes->GetCellValue(i, k);
                if (!value.IsNull() && !value.IsEmpty()) {
                    if (GridNodes->GetCellTextColour(i, k) == selectColor) {
                        GridNodes->SetCellBackgroundColour(i, k, selectBackColor);
                    }
                }
            }
        }
    }
}

void NodeSelectGrid::OnCheckBox_OrderedSelectionClick(wxCommandEvent& event)
{
    if (CheckBox_OrderedSelection->IsChecked()) {
        wxString s;
        for (auto x = 0; x < GridNodes->GetNumberCols(); ++x) {
            for (auto y = 0; y < GridNodes->GetNumberRows(); ++y) {
                const wxString value = GridNodes->GetCellValue(y, x);
                if (!value.IsNull() && !value.IsEmpty() && GridNodes->GetCellTextColour(y, x) == selectColor) {
                    if (s != "")
                        s += ",";
                    s += value;
                }
            }
        }
        TextCtrl_Nodes->SetValue(s);
    } else {
        UpdateTextFromGrid();
    }
    ValidateWindow();
}

void NodeSelectGrid::OnTextCtrl_NodesText(wxCommandEvent& event)
{
    UpdateSelectedFromText();
}

void NodeSelectGrid::UpdateTextFromGrid()
{
    if (CheckBox_OrderedSelection->IsChecked()) return;
    if (!CheckBox_OrderedSelection->IsEnabled()) return;

    TextCtrl_Nodes->SetValue(GetNodeList(false));
}

void NodeSelectGrid::UpdateSelectedFromText()
{
    if (!CheckBox_OrderedSelection->IsChecked())
        return;

    wxArrayString nodes = wxSplit(TextCtrl_Nodes->GetValue(), ',');
    for (int x = 0; x < GridNodes->GetNumberCols(); ++x) {
        for (int y = 0; y < GridNodes->GetNumberRows(); ++y) {
            if (GridNodes->GetCellValue(y, x) != "") {
                if (std::find(begin(nodes), end(nodes), GridNodes->GetCellValue(y, x)) == end(nodes)) {
                    SelectNode(false, y, x, wxAtoi(GridNodes->GetCellValue(y,x)));
                } else {
                    SelectNode(true, y, x, wxAtoi(GridNodes->GetCellValue(y,x)));
                }
            }
        }
    }
    GridNodes->Refresh();
    ValidateWindow();
}

void NodeSelectGrid::RemoveNode(int col, int row)
{
    if (!CheckBox_OrderedSelection->IsChecked())
        return;

    auto cv = GridNodes->GetCellValue(row, col);
    auto s = wxSplit(TextCtrl_Nodes->GetValue(), ',');
    int pos = 0;
    for (auto it = s.begin(); it != s.end(); ++it) {
        if (*it == cv) {
            s.erase(it);
            break;
        }
        pos += it->size() + 1;
    }
    wxString ns;
    for (const auto& it : s) {
        if (ns != "")
            ns += ",";
        ns += it;
    }
    TextCtrl_Nodes->SetValue(ns);
    TextCtrl_Nodes->SetInsertionPoint(pos);
}

void NodeSelectGrid::GetMinMaxNode(long& min, long& max)
{
    max = 0;
    min = 99999999;

    for (auto c = 0; c < GridNodes->GetNumberCols(); c++) {
        for (auto r = 0; r < GridNodes->GetNumberRows(); ++r) {
            wxString s = GridNodes->GetCellValue(r, c);

            if (s.IsEmpty() == false) {
                long v;
                if (s.ToCLong(&v) == true) {
                    max = std::max(v, max);
                    min = std::min(v, min);
                }
            }
        }
    }
}

void NodeSelectGrid::AddNode(int col, int row)
{
    if (!CheckBox_OrderedSelection->IsChecked())
        return;

    auto s = TextCtrl_Nodes->GetValue();
    int ip = TextCtrl_Nodes->GetInsertionPoint();
    auto cv = GridNodes->GetCellValue(row, col);
    if (ip >= s.size()) {
        if (s != "" && !s.EndsWith(","))
            s += ",";
        s += cv;
    } else if (ip == 0) {
        if (s == "") {
            s = cv;
        } else {
            s = cv + "," + s;
        }
    } else {
        if (s[ip - 1] == ',') {
            ip--;
        } else if (s[ip] == ',') {
        } else {
            while (ip < s.size() && s[ip] != ',')
                ip++;
        }

        if (ip >= s.size()) {
            if (!s.EndsWith(","))
                s += ",";
            s += cv;
        } else {
            auto l = s.Left(ip);
            auto r = s.Right(s.size() - l.size());
            s = l + "," + cv + r;
        }
    }

    TextCtrl_Nodes->SetValue(s);
    TextCtrl_Nodes->SetInsertionPoint(ip + 1 + cv.size());
}

void NodeSelectGrid::OnTextCtrl_NodesLoseFocus(wxFocusEvent& event)
{
    TextCtrl_Nodes->SetValue(ExpandNodes(TextCtrl_Nodes->GetValue()));
    UpdateSelectedFromText();
}

void NodeSelectGrid::SaveSettings()
{
    wxConfigBase* config = wxConfigBase::Get();
    if (config != nullptr) {
        config->Write("NodeSelectGridOrderedSelection", CheckBox_OrderedSelection->GetValue());
    }
}

void NodeSelectGrid::OnKeyDown(wxKeyEvent& event)
{
    if (event.ControlDown()) {
        if (event.GetKeyCode() == 'F') {
            Find();
        } else if (event.GetKeyCode() == 'X') {
            CutOrCopyToClipboard(true);
        } else if (event.GetKeyCode() == 'C') {
            CutOrCopyToClipboard(false);
        } else if (event.GetKeyCode() == 'V') {
            Paste();
        } else {
            event.Skip(true);
        }
    } else {
        event.Skip(true);
    }
}

void NodeSelectGrid::OnTimer1Trigger(wxTimerEvent& event)
{
    wxASSERT(_outputManager->IsOutputting());
    _outputManager->StartFrame(0);
    for (uint32_t n = 0; n < model->GetNodeCount(); ++n) {
        auto ch = model->NodeStartChannel(n-1);
        if (std::find(begin(_selected), end(_selected), n) != end(_selected)) {
            for (uint8_t c = 0; c < model->GetChanCountPerNode(); ++c) {
                _outputManager->SetOneChannel(ch++, 30);
            }
        } else {
            for (uint8_t c = 0; c < model->GetChanCountPerNode(); ++c) {
                _outputManager->SetOneChannel(ch++, 0);
            }
        }
    }

    // make the currently focussed pixel red
    if (GridNodes->GetGridCursorRow() >= 0 && GridNodes->GetGridCursorCol() >= 0) {
        const wxString value = GridNodes->GetCellValue(GridNodes->GetGridCursorRow(), GridNodes->GetGridCursorCol());
        if (value != "") {
            auto v = wxAtoi(value);
            if (v > 0) {
                auto ch = model->NodeStartChannel(v - 1);
                _outputManager->SetOneChannel(ch++, 255);
                _outputManager->SetOneChannel(ch++, 0);
            }
        }
    }

    _outputManager->EndFrame();
}

void NodeSelectGrid::StartOutputToLights()
{
    if (!timer1.IsRunning()) {
        _outputManager->StartOutput();
        timer1.SetOwner(this, ID_TIMER1);
        Connect(ID_TIMER1, wxEVT_TIMER, (wxObjectEventFunction)&NodeSelectGrid::OnTimer1Trigger);
        timer1.Start(50, false);
    }
}

bool NodeSelectGrid::StopOutputToLights()
{
    if (timer1.IsRunning()) {
        timer1.Stop();
        _outputManager->StartFrame(0);
        _outputManager->AllOff();
        _outputManager->EndFrame();
        _outputManager->StopOutput();
        return true;
    }
    return false;
}

void NodeSelectGrid::OnCheckBox_OutputToLightsClick(wxCommandEvent& event)
{
    if (CheckBox_OutputToLights->IsChecked()) {
        StartOutputToLights();
    } else {
        StopOutputToLights();
    }
}
