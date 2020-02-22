#include "ControllerModelDialog.h"

//(*InternalHeaders(ControllerModelDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/dcclient.h>
#include <wx/menu.h>
#include <wx/position.h>
#include <wx/dcmemory.h>
#include <wx/dcscreen.h>
#include <wx/image.h>
#include <wx/file.h>

#include "UtilFunctions.h"
#include "outputs/Output.h"
#include "controllers/ControllerUploadData.h"
#include "controllers/ControllerCaps.h"
#include "models/ModelManager.h"
#include "models/Model.h"

//(*IdInit(ControllerModelDialog)
const long ControllerModelDialog::ID_PANEL1 = wxNewId();
const long ControllerModelDialog::ID_SCROLLBAR1 = wxNewId();
const long ControllerModelDialog::ID_SCROLLBAR2 = wxNewId();
const long ControllerModelDialog::ID_PANEL3 = wxNewId();
const long ControllerModelDialog::ID_PANEL2 = wxNewId();
const long ControllerModelDialog::ID_SCROLLBAR3 = wxNewId();
const long ControllerModelDialog::ID_PANEL4 = wxNewId();
const long ControllerModelDialog::ID_SPLITTERWINDOW1 = wxNewId();
//*)

const long ControllerModelDialog::CONTROLLERModel_PRINT = wxNewId();
const long ControllerModelDialog::CONTROLLERModel_SAVE_CSV = wxNewId();

BEGIN_EVENT_TABLE(ControllerModelDialog,wxDialog)
	//(*EventTable(ControllerModelDialog)
	//*)
END_EVENT_TABLE()

#define TOP_BOTTOM_MARGIN 10
#define VERTICAL_GAP 5
#define VERTICAL_SIZE 40
#define LEFT_RIGHT_MARGIN TOP_BOTTOM_MARGIN
#define HORIZONTAL_GAP VERTICAL_GAP
#define HORIZONTAL_SIZE 120
#define CORNER_ROUNDING 5
#define PRINTSCALE 8.0

#pragma region Object Classes

class BaseCMObject
{
protected:
    bool _selectable = false;
    bool _selected = false;
    bool _dragging = false;
    wxPoint _location = wxPoint(0,0);
    wxSize _size = wxSize(100,40);
    UDController* _cud = nullptr;
    ControllerCaps* _caps = nullptr;

public:

    BaseCMObject(UDController* cud, ControllerCaps* caps, wxPoint location, wxSize size) {
        _cud = cud;
        _caps = caps;
        _location = location;
        _size = size;
    }
    virtual ~BaseCMObject() {}
    bool HitTest(wxPoint mouse)
    {
        return (mouse.x >= _location.x &&
            mouse.x <= _location.x + _size.x &&
            mouse.y >= _location.y &&
            mouse.y <= _location.y + _size.y);
    }
    virtual void Draw(wxDC& dc, wxPoint mouse, wxSize offset, int scale, bool printing = false) = 0;
    void UpdateCUD(UDController* cud) { _cud = cud; }
    virtual void AddRightClickMenu(wxMenu& mnu) {}
    virtual void HandlePopup(int id) {}
};

class PortCMObject : public BaseCMObject
{
public:
    enum class PORTTYPE { PIXEL, SERIAL };
protected:
    int _port = -1;
    PORTTYPE _type = PORTTYPE::PIXEL;
public:

    PortCMObject(PORTTYPE type, int port, UDController* cud, ControllerCaps* caps, wxPoint location, wxSize size) : BaseCMObject(cud, caps, location, size)
    {
        _port = port;
        _type = type;
    }
    virtual void Draw(wxDC& dc, wxPoint mouse, wxSize offset, int scale, bool printing = false) override
    {
        auto origBrush = dc.GetBrush();
        auto origPen = dc.GetPen();
        auto origText = dc.GetTextForeground();

        UDControllerPort* p = nullptr;
        if (_type == PORTTYPE::PIXEL) {
            dc.SetPen(*wxRED_PEN);
            p = _cud->GetControllerPixelPort(_port);
        }
        else {
            dc.SetPen(*wxGREEN_PEN);
            p = _cud->GetControllerSerialPort(_port);
        }
        if (HitTest(mouse) && !printing)
        {
            dc.SetBrush(*wxBLUE_BRUSH);
        }
        wxSize sz = _size.Scale(scale, scale);
        dc.DrawRoundedRectangle(_location + offset, sz, CORNER_ROUNDING * scale);

        wxPoint pt = _location + wxSize(2, 2) + offset;
        dc.DrawText(wxString::Format("Pixel Port %d", _port), pt);
        pt += wxSize(0, (VERTICAL_SIZE * scale) / 2);

        const std::string pixels = "Pixels: ";
        wxSize szp = dc.GetTextExtent(pixels);
        dc.DrawText(pixels, pt);

        pt += wxSize(szp.GetWidth(), 0);
        if (p->Channels() > _caps->GetMaxPixelPortChannels())
        {
            dc.SetTextForeground(*wxRED);
        }
        dc.DrawText(wxString::Format("%d", p->Channels()), pt);

        dc.SetBrush(origBrush);
        dc.SetPen(origPen);
        dc.SetTextForeground(origText);
    }
};

class ModelCMObject : BaseCMObject
{
protected:
    ModelManager* _mm;
    std::string _name;
public:
    ModelCMObject(const std::string& name, ModelManager* mm, UDController* cud, ControllerCaps* caps, wxPoint location, wxSize size) : BaseCMObject(cud, caps, location, size)
    {
        _name = name;
    }

    virtual void Draw(wxDC& dc, wxPoint mouse, wxSize offset, int scale, bool printing = false) override
    {
        auto origBrush = dc.GetBrush();
        auto origPen = dc.GetPen();
        auto origText = dc.GetTextForeground();

        Model* m = _mm->GetModel(_name);

        UDControllerPortModel* udcpm = _cud->GetControllerPortModel(_name);

        dc.SetPen(*wxGREY_PEN);

        if (udcpm != nullptr) {
            switch (udcpm->GetSmartRemote())
            {
            case 0:
                dc.SetBrush(*wxWHITE_BRUSH);
                break;
            case 1:
                dc.SetBrush(*wxGREEN_BRUSH);
                break;
            case 2:
                dc.SetBrush(*wxCYAN_BRUSH);
                break;
            case 3:
                dc.SetBrush(*wxLIGHT_GREY_BRUSH);
                break;
            }
        }

        if (_dragging && !printing)
        {
            dc.SetBrush(wxColour(255, 255, 128));
        }
        else if (HitTest(mouse) && !printing)
        {
            dc.SetPen(wxPen(dc.GetPen().GetColour(), 3));
        }

        wxSize sz = _size.Scale(scale, scale);
        dc.DrawRectangle(_location + offset, sz);
        wxPoint pt = _location + wxSize(2, 2) + offset;
        dc.DrawText(m->GetName(), pt);
        pt += wxSize(0, (VERTICAL_SIZE * scale) / 2);
        dc.DrawText(wxString::Format("Pixels: %d", m->GetChanCount() / 3), pt);

        dc.SetBrush(origBrush);
        dc.SetPen(origPen);
        dc.SetTextForeground(origText);
    }
    virtual void AddRightClickMenu(wxMenu& mnu) override
    {
        //wxMenuItem* mi = mnu.Append(CONTROLLERModel_SAVE_CSV, "Save As CSV...");
        //mi->SetRefData((wxObjectRefData*)this); // this a bit dodgy
    }
    virtual void HandlePopup(int id) override
    {
        //if (id == xxx)
        //{

        //}
    }
};

#pragma endregion

ControllerModelPrintout::ControllerModelPrintout(ControllerModelDialog* controllerDialog)
{
    _controllerDialog = controllerDialog;
}

bool ControllerModelPrintout::OnPrintPage(int pageNum)
{
    if (pageNum != 1) return false;

    wxRect rect = GetLogicalPageRect();
    wxBitmap bmp;
    bmp.Create(rect.GetWidth() * 0.95f, rect.GetHeight() * 0.95f);
    _controllerDialog->RenderPicture(bmp, true);

    wxDC* dc = GetDC();
    dc->DrawBitmap(bmp, 0, 0);

    return true;
}

ControllerModelDialog::ControllerModelDialog(wxWindow* parent, UDController& cud, const std::string& ip, const std::string& description, wxWindowID id,const wxPoint& pos,const wxSize& size) : _cud(cud), _ip(ip), _description(description)
{
	//(*Initialize(ControllerModelDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer5;

	Create(parent, wxID_ANY, _("Controller Visualiser"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxCLOSE_BOX|wxMAXIMIZE_BOX, _T("wxID_ANY"));
	SetClientSize(wxSize(500,500));
	FlexGridSizer1 = new wxFlexGridSizer(2, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	SplitterWindow1 = new wxSplitterWindow(this, ID_SPLITTERWINDOW1, wxDefaultPosition, wxDefaultSize, wxSP_3D|wxSP_BORDER|wxSP_LIVE_UPDATE, _T("ID_SPLITTERWINDOW1"));
	SplitterWindow1->SetMinimumPaneSize(150);
	SplitterWindow1->SetSashGravity(0.5);
	Panel3 = new wxPanel(SplitterWindow1, ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL3"));
	FlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer5->AddGrowableCol(0);
	FlexGridSizer5->AddGrowableRow(0);
	PanelController = new wxPanel(Panel3, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER, _T("ID_PANEL1"));
	FlexGridSizer5->Add(PanelController, 1, wxALL|wxEXPAND, 0);
	ScrollBar_Controller_V = new wxScrollBar(Panel3, ID_SCROLLBAR1, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL, wxDefaultValidator, _T("ID_SCROLLBAR1"));
	ScrollBar_Controller_V->SetScrollbar(0, 1, 10, 1);
	FlexGridSizer5->Add(ScrollBar_Controller_V, 1, wxALL|wxEXPAND, 0);
	ScrollBar_Controller_H = new wxScrollBar(Panel3, ID_SCROLLBAR2, wxDefaultPosition, wxDefaultSize, wxSB_HORIZONTAL, wxDefaultValidator, _T("ID_SCROLLBAR2"));
	ScrollBar_Controller_H->SetScrollbar(0, 1, 10, 1);
	FlexGridSizer5->Add(ScrollBar_Controller_H, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer5->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	Panel3->SetSizer(FlexGridSizer5);
	FlexGridSizer5->Fit(Panel3);
	FlexGridSizer5->SetSizeHints(Panel3);
	Panel4 = new wxPanel(SplitterWindow1, ID_PANEL4, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL4"));
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	FlexGridSizer3->AddGrowableRow(0);
	PanelModels = new wxPanel(Panel4, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER, _T("ID_PANEL2"));
	FlexGridSizer3->Add(PanelModels, 1, wxALL|wxEXPAND, 0);
	ScrollBar_Models = new wxScrollBar(Panel4, ID_SCROLLBAR3, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL, wxDefaultValidator, _T("ID_SCROLLBAR3"));
	ScrollBar_Models->SetScrollbar(0, 1, 10, 1);
	FlexGridSizer3->Add(ScrollBar_Models, 1, wxALL|wxEXPAND, 0);
	Panel4->SetSizer(FlexGridSizer3);
	FlexGridSizer3->Fit(Panel4);
	FlexGridSizer3->SetSizeHints(Panel4);
	SplitterWindow1->SplitVertically(Panel3, Panel4);
	SplitterWindow1->SetSashPosition(200);
	FlexGridSizer1->Add(SplitterWindow1, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	SetSizer(FlexGridSizer1);
	Layout();

	PanelController->Connect(wxEVT_PAINT,(wxObjectEventFunction)&ControllerModelDialog::OnPanelControllerPaint,0,this);
	PanelController->Connect(wxEVT_KEY_DOWN,(wxObjectEventFunction)&ControllerModelDialog::OnPanelControllerKeyDown,0,this);
	PanelController->Connect(wxEVT_LEFT_DOWN,(wxObjectEventFunction)&ControllerModelDialog::OnPanelControllerLeftDown,0,this);
	PanelController->Connect(wxEVT_LEFT_UP,(wxObjectEventFunction)&ControllerModelDialog::OnPanelControllerLeftUp,0,this);
	PanelController->Connect(wxEVT_LEFT_DCLICK,(wxObjectEventFunction)&ControllerModelDialog::OnPanelControllerLeftDClick,0,this);
	PanelController->Connect(wxEVT_RIGHT_DOWN,(wxObjectEventFunction)&ControllerModelDialog::OnPanelControllerRightDown,0,this);
	PanelController->Connect(wxEVT_MOTION,(wxObjectEventFunction)&ControllerModelDialog::OnPanelControllerMouseMove,0,this);
	PanelController->Connect(wxEVT_ENTER_WINDOW,(wxObjectEventFunction)&ControllerModelDialog::OnPanelControllerMouseEnter,0,this);
	PanelController->Connect(wxEVT_LEAVE_WINDOW,(wxObjectEventFunction)&ControllerModelDialog::OnPanelControllerMouseLeave,0,this);
	PanelController->Connect(wxEVT_MOUSEWHEEL,(wxObjectEventFunction)&ControllerModelDialog::OnPanelControllerMouseWheel,0,this);
	PanelController->Connect(wxEVT_SIZE,(wxObjectEventFunction)&ControllerModelDialog::OnPanelControllerResize,0,this);
	Connect(ID_SCROLLBAR1,wxEVT_SCROLL_TOP|wxEVT_SCROLL_BOTTOM|wxEVT_SCROLL_LINEUP|wxEVT_SCROLL_LINEDOWN|wxEVT_SCROLL_PAGEUP|wxEVT_SCROLL_PAGEDOWN|wxEVT_SCROLL_THUMBTRACK|wxEVT_SCROLL_THUMBRELEASE|wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&ControllerModelDialog::OnScrollBar_Controller_VScroll);
	Connect(ID_SCROLLBAR1,wxEVT_SCROLL_THUMBTRACK,(wxObjectEventFunction)&ControllerModelDialog::OnScrollBar_Controller_VScrollThumbTrack);
	Connect(ID_SCROLLBAR1,wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&ControllerModelDialog::OnScrollBar_Controller_VScrollChanged);
	Connect(ID_SCROLLBAR2,wxEVT_SCROLL_TOP|wxEVT_SCROLL_BOTTOM|wxEVT_SCROLL_LINEUP|wxEVT_SCROLL_LINEDOWN|wxEVT_SCROLL_PAGEUP|wxEVT_SCROLL_PAGEDOWN|wxEVT_SCROLL_THUMBTRACK|wxEVT_SCROLL_THUMBRELEASE|wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&ControllerModelDialog::OnScrollBar_Controller_HScroll);
	Connect(ID_SCROLLBAR2,wxEVT_SCROLL_THUMBTRACK,(wxObjectEventFunction)&ControllerModelDialog::OnScrollBar_Controller_HScrollThumbTrack);
	Connect(ID_SCROLLBAR2,wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&ControllerModelDialog::OnScrollBar_Controller_HScrollChanged);
	PanelModels->Connect(wxEVT_PAINT,(wxObjectEventFunction)&ControllerModelDialog::OnPanelModelsPaint,0,this);
	PanelModels->Connect(wxEVT_KEY_DOWN,(wxObjectEventFunction)&ControllerModelDialog::OnPanelModelsKeyDown,0,this);
	PanelModels->Connect(wxEVT_LEFT_DOWN,(wxObjectEventFunction)&ControllerModelDialog::OnPanelModelsLeftDown,0,this);
	PanelModels->Connect(wxEVT_LEFT_UP,(wxObjectEventFunction)&ControllerModelDialog::OnPanelModelsLeftUp,0,this);
	PanelModels->Connect(wxEVT_LEFT_DCLICK,(wxObjectEventFunction)&ControllerModelDialog::OnPanelModelsLeftDClick,0,this);
	PanelModels->Connect(wxEVT_RIGHT_DOWN,(wxObjectEventFunction)&ControllerModelDialog::OnPanelModelsRightDown,0,this);
	PanelModels->Connect(wxEVT_MOTION,(wxObjectEventFunction)&ControllerModelDialog::OnPanelModelsMouseMove,0,this);
	PanelModels->Connect(wxEVT_ENTER_WINDOW,(wxObjectEventFunction)&ControllerModelDialog::OnPanelModelsMouseEnter,0,this);
	PanelModels->Connect(wxEVT_LEAVE_WINDOW,(wxObjectEventFunction)&ControllerModelDialog::OnPanelModelsMouseLeave,0,this);
	PanelModels->Connect(wxEVT_MOUSEWHEEL,(wxObjectEventFunction)&ControllerModelDialog::OnPanelModelsMouseWheel,0,this);
	PanelModels->Connect(wxEVT_SIZE,(wxObjectEventFunction)&ControllerModelDialog::OnPanelModelsResize,0,this);
	Connect(ID_SCROLLBAR3,wxEVT_SCROLL_TOP|wxEVT_SCROLL_BOTTOM|wxEVT_SCROLL_LINEUP|wxEVT_SCROLL_LINEDOWN|wxEVT_SCROLL_PAGEUP|wxEVT_SCROLL_PAGEDOWN|wxEVT_SCROLL_THUMBTRACK|wxEVT_SCROLL_THUMBRELEASE|wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&ControllerModelDialog::OnScrollBar_ModelsScroll);
	Connect(ID_SCROLLBAR3,wxEVT_SCROLL_THUMBTRACK,(wxObjectEventFunction)&ControllerModelDialog::OnScrollBar_ModelsScrollThumbTrack);
	Connect(ID_SCROLLBAR3,wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&ControllerModelDialog::OnScrollBar_ModelsScrollChanged);
	//*)

    SetLabel(GetLabel() + " " + _ip + " " + _description);

    int countlines = 0;
    int countcolumns = 1;
    for (int i = 1; i <= _cud.GetMaxPixelPort(); i++)
    {
        _cud.GetControllerPixelPort(i)->CreateVirtualStrings(true);
        if (_cud.GetControllerPixelPort(i)->GetVirtualStringCount() == 0)
        {
            countlines++;
        }
        else
        {
            countlines += _cud.GetControllerPixelPort(i)->GetVirtualStringCount();
            int thiscols = 0;
            for (int j = 0; j < _cud.GetControllerPixelPort(i)->GetVirtualStringCount(); j++)
            {
                thiscols = std::max(thiscols, (int)_cud.GetControllerPixelPort(i)->GetVirtualString(j)->_models.size());
            }
            countcolumns = std::max(countcolumns, 1 + thiscols);
        }
    }
    countlines += _cud.GetMaxSerialPort();
    for (int i = 1; i <= _cud.GetMaxSerialPort(); i++)
    {
        countcolumns = std::max(countcolumns, 1 + (int)_cud.GetControllerSerialPort(i)->GetModels().size());
    }

    int height = 2 * TOP_BOTTOM_MARGIN + countlines * VERTICAL_SIZE + (countlines - 1) * VERTICAL_GAP;
    int width = 2 * LEFT_RIGHT_MARGIN + countcolumns * HORIZONTAL_SIZE + (countcolumns - 1) * HORIZONTAL_GAP;

    ScrollBar_Controller_V->SetRange(height);
    ScrollBar_Controller_H->SetRange(width);
    ScrollBar_Controller_V->SetPageSize(height / 10);
    ScrollBar_Controller_H->SetPageSize(width / 10);
    Layout();
}

ControllerModelDialog::~ControllerModelDialog()
{
	//(*Destroy(ControllerModelDialog)
	//*)
}

void ControllerModelDialog::OnPopupCommand(wxCommandEvent &event)
{
    int id = event.GetId();
    if (id == CONTROLLERModel_PRINT)
    {
        static wxPrintDialogData printDialogData;
        wxPrinter printer(&printDialogData);

        ControllerModelPrintout printout(this);

        if (!printer.Print(this, &printout, true))
        {
            if (wxPrinter::GetLastError() == wxPRINTER_ERROR)
            {
                DisplayError(wxString::Format("Problem printing wiring. %d", wxPrinter::GetLastError()).ToStdString(), this);
            }
        }
        else
        {
            printDialogData = printer.GetPrintDialogData();
        }
    }
    else if (id == CONTROLLERModel_SAVE_CSV)
    {
        SaveCSV();
    }
    else
    {
        BaseCMObject* cm = (BaseCMObject *)event.GetEventObject()->GetRefData();
        cm->HandlePopup(id);
    }
}


void ControllerModelDialog::RenderPicture(wxBitmap& bitmap, bool printer)
{
    wxMemoryDC dc;
    dc.SelectObject(bitmap);

    dc.SetTextForeground(*wxWHITE);

    dc.SetPen(*wxWHITE_PEN);
    dc.SetBrush(*wxWHITE_BRUSH);

    dc.DrawRectangle(wxPoint(0, 0), bitmap.GetScaledSize());

    dc.SetDeviceOrigin(0, 0);

    int fontSize = 10 * PRINTSCALE;
    wxFont font = wxFont(fontSize, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, _T("Arial"), wxFONTENCODING_DEFAULT);
    dc.SetFont(font);

    int rowPos = TOP_BOTTOM_MARGIN * PRINTSCALE;
    dc.DrawText(_ip + " " + _description, LEFT_RIGHT_MARGIN * PRINTSCALE, rowPos);
    rowPos += ((VERTICAL_SIZE / 2) * PRINTSCALE) + (VERTICAL_GAP * PRINTSCALE);

    for (const auto& it : _controllers)
    {
        it->Draw(dc, wxPoint(0, 0), wxSize(0, rowPos), PRINTSCALE, true);
    }
}

void ControllerModelDialog::SaveCSV()
{
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, _ip, wxEmptyString, "Export files (*.csv)|*.csv", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (filename.IsEmpty()) return;

    std::vector<wxString> lines;
    int columnSize = 0;

    for (int i = 1; i <= _cud.GetMaxPixelPort(); i++)
    {
        wxString line = wxString::Format("Pixel Port %d,", i);

        if (columnSize < _cud.GetControllerPixelPort(i)->GetModels().size())
            columnSize = _cud.GetControllerPixelPort(i)->GetModels().size();

        for (const auto& it : _cud.GetControllerPixelPort(i)->GetModels())
        {
            if (it->GetSmartRemote() > 0)
            {
                char remote = ('@' + it->GetSmartRemote());
                line += "Remote ";
                line += remote;
                line += ":";
            }
            line += it->GetName();
            line += ",";
        }

        line += "\n";
        lines.push_back(line);
    }
       lines.push_back("\n");
    for (int i = 1; i <= _cud.GetMaxSerialPort(); i++)
    {
        if (columnSize < _cud.GetControllerSerialPort(i)->GetModels().size())
            columnSize = _cud.GetControllerSerialPort(i)->GetModels().size();

        wxString line = wxString::Format("Serial Port %d,", i);
        for (const auto& it : _cud.GetControllerSerialPort(i)->GetModels())
        {
            line += it->GetName();
            line += ",";
        }
        line += "\n";
        lines.push_back(line);
    }

    wxFile f(filename);

    if (!f.Create(filename, true) || !f.IsOpened())
    {
        DisplayError(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString());
        return;
    }

    wxString header = _ip + " " + _description + "\nOutput,";
    for (int i = 1; i <= columnSize; i++)
    {
        header += wxString::Format("Model %d,", i);
    }
    header += "\n";
    f.Write(header);
    for (const auto& line : lines)
    {
        f.Write(line);
    }
    f.Close();
}

void ControllerModelDialog::OnPanelControllerLeftDown(wxMouseEvent& event)
{
}

void ControllerModelDialog::OnPanelControllerKeyDown(wxKeyEvent& event)
{
}

void ControllerModelDialog::OnPanelControllerLeftUp(wxMouseEvent& event)
{
}

void ControllerModelDialog::OnPanelControllerLeftDClick(wxMouseEvent& event)
{
}

void ControllerModelDialog::OnPanelControllerMouseMove(wxMouseEvent& event)
{
}

void ControllerModelDialog::OnPanelControllerMouseEnter(wxMouseEvent& event)
{
}

void ControllerModelDialog::OnPanelControllerMouseLeave(wxMouseEvent& event)
{
}

void ControllerModelDialog::OnPanelControllerRightDown(wxMouseEvent& event)
{
    wxMenu mnu;
    mnu.Append(CONTROLLERModel_PRINT, "Print");
    mnu.Append(CONTROLLERModel_SAVE_CSV, "Save As CSV...");

    BaseCMObject* cm = GetControllerCMObjectAt(event.GetPosition());
    if (cm != nullptr)
    {
        cm->AddRightClickMenu(mnu);
    }

    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&ControllerModelDialog::OnPopupCommand, nullptr, this);
    PopupMenu(&mnu);
}

void ControllerModelDialog::OnPanelControllerPaint(wxPaintEvent& event)
{
    wxPaintDC dc(PanelController);

    int xOffset = ScrollBar_Controller_H->GetThumbPosition();
    int yOffset = ScrollBar_Controller_V->GetThumbPosition();

    dc.SetDeviceOrigin(-xOffset, -yOffset);

    for (const auto& it : _controllers)
    {
        it->Draw(dc, wxGetMousePosition(), wxSize(0,0), 1, false);
    }
}

void ControllerModelDialog::OnScrollBar_Controller_HScroll(wxScrollEvent& event)
{
    PanelController->Refresh();
}

void ControllerModelDialog::OnScrollBar_Controller_HScrollThumbTrack(wxScrollEvent& event)
{
    PanelController->Refresh();
}

void ControllerModelDialog::OnScrollBar_Controller_HScrollChanged(wxScrollEvent& event)
{
    PanelController->Refresh();
}

void ControllerModelDialog::OnScrollBar_Controller_VScroll(wxScrollEvent& event)
{
    PanelController->Refresh();
}

void ControllerModelDialog::OnScrollBar_Controller_VScrollThumbTrack(wxScrollEvent& event)
{
    PanelController->Refresh();
}

void ControllerModelDialog::OnScrollBar_Controller_VScrollChanged(wxScrollEvent& event)
{
    PanelController->Refresh();
}

#pragma region Models Panel
void ControllerModelDialog::OnScrollBar_ModelsScroll(wxScrollEvent& event)
{
    PanelModels->Refresh();
}

void ControllerModelDialog::OnScrollBar_ModelsScrollThumbTrack(wxScrollEvent& event)
{
    PanelModels->Refresh();
}

void ControllerModelDialog::OnScrollBar_ModelsScrollChanged(wxScrollEvent& event)
{
    PanelModels->Refresh();
}

void ControllerModelDialog::OnPanelModelsPaint(wxPaintEvent& event)
{
    wxPaintDC dc(PanelController);

    int yOffset = ScrollBar_Models->GetThumbPosition();

    dc.SetDeviceOrigin(0, -yOffset);

    for (const auto& it : _models)
    {
        it->Draw(dc, wxGetMousePosition(), wxSize(0, 0), 1, false);
    }
}

void ControllerModelDialog::OnPanelModelsKeyDown(wxKeyEvent& event)
{
}

void ControllerModelDialog::OnPanelModelsLeftDown(wxMouseEvent& event)
{
}

void ControllerModelDialog::OnPanelModelsLeftUp(wxMouseEvent& event)
{
}

void ControllerModelDialog::OnPanelModelsLeftDClick(wxMouseEvent& event)
{
}

void ControllerModelDialog::OnPanelModelsRightDown(wxMouseEvent& event)
{
    wxMenu mnu;

    BaseCMObject* cm = GetModelsCMObjectAt(event.GetPosition());
    if (cm != nullptr)
    {
        cm->AddRightClickMenu(mnu);
    }

    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&ControllerModelDialog::OnPopupCommand, nullptr, this);
    PopupMenu(&mnu);
}

void ControllerModelDialog::OnPanelModelsMouseMove(wxMouseEvent& event)
{
}

void ControllerModelDialog::OnPanelModelsMouseWheel(wxMouseEvent& event)
{
    int position = ScrollBar_Models->GetThumbPosition();
    position -= event.GetWheelRotation();
    ScrollBar_Models->SetThumbPosition(position);
    PanelModels->Refresh();
}

void ControllerModelDialog::OnPanelModelsResize(wxSizeEvent& event)
{
    int pw;
    int ph;
    PanelModels->GetSize(&pw, &ph);
    int h = (float)ph / ScrollBar_Controller_V->GetRange() * (ph - 80);
    int w = (float)pw / ScrollBar_Controller_H->GetRange() * (pw - 80);
    ScrollBar_Controller_H->SetThumbSize(w);
    ScrollBar_Controller_V->SetThumbSize(h);
    PanelController->Refresh();
}

void ControllerModelDialog::OnPanelModelsMouseEnter(wxMouseEvent& event)
{
}

void ControllerModelDialog::OnPanelModelsMouseLeave(wxMouseEvent& event)
{
}
#pragma endregion

#pragma region Controller Ports Panel
void ControllerModelDialog::OnPanelControllerMouseWheel(wxMouseEvent& event)
{
    if (event.ShiftDown())
    {
        int position = ScrollBar_Controller_H->GetThumbPosition();
        position -= event.GetWheelRotation();
        ScrollBar_Controller_H->SetThumbPosition(position);
    }
    else
    {
        int position = ScrollBar_Controller_V->GetThumbPosition();
        position -= event.GetWheelRotation();
        ScrollBar_Controller_V->SetThumbPosition(position);
    }

    PanelController->Refresh();
}

void ControllerModelDialog::OnPanelControllerResize(wxSizeEvent& event)
{
    int pw;
    int ph;
    PanelController->GetSize(&pw, &ph);
    int h = (float)ph / ScrollBar_Controller_V->GetRange() * (ph - 80);
    int w = (float)pw / ScrollBar_Controller_H->GetRange() * (pw - 80);
    ScrollBar_Controller_H->SetThumbSize(w);
    ScrollBar_Controller_V->SetThumbSize(h);
    PanelController->Refresh();
}
#pragma endregion

BaseCMObject* ControllerModelDialog::GetControllerCMObjectAt(wxPoint mouse)
{
    for (const auto& it : _controllers)
    {
        if (it->HitTest(mouse)) return it;
    }
    return nullptr;
}

BaseCMObject* ControllerModelDialog::GetModelsCMObjectAt(wxPoint mouse)
{
    for (const auto& it : _models)
    {
        if (it->HitTest(mouse)) return it;
    }
    return nullptr;
}
