#include "ControllerVisualiseDialog.h"

//(*InternalHeaders(ControllerVisualiseDialog)
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

//(*IdInit(ControllerVisualiseDialog)
const long ControllerVisualiseDialog::ID_PANEL1 = wxNewId();
const long ControllerVisualiseDialog::ID_SCROLLBAR1 = wxNewId();
const long ControllerVisualiseDialog::ID_SCROLLBAR2 = wxNewId();
//*)

const long ControllerVisualiseDialog::CONTROLLERVISUALISE_PRINT = wxNewId();
const long ControllerVisualiseDialog::CONTROLLERVISUALISE_SAVE_CSV = wxNewId();

BEGIN_EVENT_TABLE(ControllerVisualiseDialog,wxDialog)
	//(*EventTable(ControllerVisualiseDialog)
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

ControllerVisualisePrintout::ControllerVisualisePrintout(ControllerVisualiseDialog* controllerDialog)
{
    _controllerDialog = controllerDialog;
}

bool ControllerVisualisePrintout::OnPrintPage(int pageNum)
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

ControllerVisualiseDialog::ControllerVisualiseDialog(wxWindow* parent, UDController& cud, wxWindowID id,const wxPoint& pos,const wxSize& size) : _cud(cud)
{
	//(*Initialize(ControllerVisualiseDialog)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, _("Controller Visualiser"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxCLOSE_BOX|wxMAXIMIZE_BOX, _T("id"));
	SetClientSize(wxSize(500,500));
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(2, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	Panel1 = new wxPanel(this, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	FlexGridSizer1->Add(Panel1, 1, wxALL|wxEXPAND, 0);
	ScrollBar_Vertical = new wxScrollBar(this, ID_SCROLLBAR1, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL, wxDefaultValidator, _T("ID_SCROLLBAR1"));
	ScrollBar_Vertical->SetScrollbar(0, 1, 10, 1);
	FlexGridSizer1->Add(ScrollBar_Vertical, 1, wxALL|wxEXPAND, 0);
	ScrollBar_Horizontal = new wxScrollBar(this, ID_SCROLLBAR2, wxDefaultPosition, wxDefaultSize, wxSB_HORIZONTAL, wxDefaultValidator, _T("ID_SCROLLBAR2"));
	ScrollBar_Horizontal->SetScrollbar(0, 1, 10, 1);
	FlexGridSizer1->Add(ScrollBar_Horizontal, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	SetSizer(FlexGridSizer1);
	SetSizer(FlexGridSizer1);
	Layout();

	Panel1->Connect(wxEVT_PAINT,(wxObjectEventFunction)&ControllerVisualiseDialog::OnPanel1Paint,0,this);
	Panel1->Connect(wxEVT_RIGHT_DOWN,(wxObjectEventFunction)&ControllerVisualiseDialog::OnPanel1RightDown,0,this);
	Panel1->Connect(wxEVT_SIZE,(wxObjectEventFunction)&ControllerVisualiseDialog::OnPanel1Resize,0,this);
	Connect(ID_SCROLLBAR1,wxEVT_SCROLL_TOP|wxEVT_SCROLL_BOTTOM|wxEVT_SCROLL_LINEUP|wxEVT_SCROLL_LINEDOWN|wxEVT_SCROLL_PAGEUP|wxEVT_SCROLL_PAGEDOWN|wxEVT_SCROLL_THUMBTRACK|wxEVT_SCROLL_THUMBRELEASE|wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&ControllerVisualiseDialog::OnScrollBar_VerticalScroll);
	Connect(ID_SCROLLBAR1,wxEVT_SCROLL_THUMBTRACK,(wxObjectEventFunction)&ControllerVisualiseDialog::OnScrollBar_VerticalScrollThumbTrack);
	Connect(ID_SCROLLBAR1,wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&ControllerVisualiseDialog::OnScrollBar_VerticalScrollChanged);
	Connect(ID_SCROLLBAR2,wxEVT_SCROLL_TOP|wxEVT_SCROLL_BOTTOM|wxEVT_SCROLL_LINEUP|wxEVT_SCROLL_LINEDOWN|wxEVT_SCROLL_PAGEUP|wxEVT_SCROLL_PAGEDOWN|wxEVT_SCROLL_THUMBTRACK|wxEVT_SCROLL_THUMBRELEASE|wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&ControllerVisualiseDialog::OnScrollBar_HorizontalScroll);
	Connect(ID_SCROLLBAR2,wxEVT_SCROLL_THUMBTRACK,(wxObjectEventFunction)&ControllerVisualiseDialog::OnScrollBar_HorizontalScrollThumbTrack);
	Connect(ID_SCROLLBAR2,wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&ControllerVisualiseDialog::OnScrollBar_HorizontalScrollChanged);
	//*)

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

    ScrollBar_Vertical->SetRange(height);
    ScrollBar_Horizontal->SetRange(width);
    ScrollBar_Vertical->SetPageSize(height / 10);
    ScrollBar_Horizontal->SetPageSize(width / 10);
    Layout();
}

ControllerVisualiseDialog::~ControllerVisualiseDialog()
{
	//(*Destroy(ControllerVisualiseDialog)
	//*)
}

void ControllerVisualiseDialog::OnPanel1Paint(wxPaintEvent& event)
{
    wxPaintDC dc(Panel1);

    int xOffset = ScrollBar_Horizontal->GetThumbPosition();
    int yOffset = ScrollBar_Vertical->GetThumbPosition();

    dc.SetDeviceOrigin(-xOffset, -yOffset);

	RenderDiagram(dc);
}

void ControllerVisualiseDialog::OnScrollBar_VerticalScroll(wxScrollEvent& event)
{
    Panel1->Refresh();
}

void ControllerVisualiseDialog::OnScrollBar_HorizontalScroll(wxScrollEvent& event)
{
    Panel1->Refresh();
}

void ControllerVisualiseDialog::OnScrollBar_HorizontalScrollThumbTrack(wxScrollEvent& event)
{
    Panel1->Refresh();
}

void ControllerVisualiseDialog::OnScrollBar_HorizontalScrollChanged(wxScrollEvent& event)
{
    Panel1->Refresh();
}

void ControllerVisualiseDialog::OnScrollBar_VerticalScrollChanged(wxScrollEvent& event)
{
    Panel1->Refresh();
}

void ControllerVisualiseDialog::OnScrollBar_VerticalScrollThumbTrack(wxScrollEvent& event)
{
    Panel1->Refresh();
}

void ControllerVisualiseDialog::OnPanel1Resize(wxSizeEvent& event)
{
    int pw;
    int ph;
    GetSize(&pw, &ph);
    int h = (float)ph / ScrollBar_Vertical->GetRange() * (ph - 80);
    int w = (float)pw / ScrollBar_Horizontal->GetRange() * (pw - 80);
    ScrollBar_Horizontal->SetThumbSize(w);
    ScrollBar_Vertical->SetThumbSize(h);
    Panel1->Refresh();
}

void ControllerVisualiseDialog::OnPanel1RightDown(wxMouseEvent& event)
{
    wxMenu mnu;
    mnu.Append(CONTROLLERVISUALISE_PRINT, "Print");
    mnu.Append(CONTROLLERVISUALISE_SAVE_CSV, "Save As CSV...");

    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&ControllerVisualiseDialog::OnPopupCommand, nullptr, this);
    PopupMenu(&mnu);
}

void ControllerVisualiseDialog::OnPopupCommand(wxCommandEvent &event)
{
    int id = event.GetId();
    if (id == CONTROLLERVISUALISE_PRINT)
    {
        static wxPrintDialogData printDialogData;
        wxPrinter printer(&printDialogData);

        ControllerVisualisePrintout printout(this);

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
    else if (id == CONTROLLERVISUALISE_SAVE_CSV)
    {
		SaveCSV();
    }
}


void ControllerVisualiseDialog::RenderPicture(wxBitmap& bitmap, bool printer)
{
	wxMemoryDC dc;
	dc.SelectObject(bitmap);

	dc.SetTextForeground(*wxWHITE);

	dc.SetPen(*wxWHITE_PEN);
	dc.SetBrush(*wxWHITE_BRUSH);

	dc.DrawRectangle(wxPoint(0, 0), bitmap.GetScaledSize());

	dc.SetDeviceOrigin(0, 0);

	RenderDiagram(dc, PRINTSCALE);
}

void ControllerVisualiseDialog::RenderDiagram(wxDC& dc, int scale)
{
    if (scale != 1)
    {
		int fontSize = 10 * scale;
		wxFont font = wxFont(fontSize, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, _T("Arial"), wxFONTENCODING_DEFAULT);
		dc.SetFont(font);
    }

    dc.SetTextForeground(*wxBLACK);

    int rowPos = TOP_BOTTOM_MARGIN * scale;
    for (int i = 1; i <= _cud.GetMaxPixelPort(); i++)
    {
        int colPos = LEFT_RIGHT_MARGIN * scale;
        dc.SetPen(*wxRED_PEN);
        dc.DrawRoundedRectangle(colPos, rowPos, HORIZONTAL_SIZE * scale, VERTICAL_SIZE * scale, CORNER_ROUNDING * scale);
        dc.DrawText(wxString::Format("Pixel Port %d", i), colPos + 2, rowPos + 2);
        dc.DrawText(wxString::Format("Pixels: %d", _cud.GetControllerPixelPort(i)->Channels() / 3), colPos + 2, rowPos + 2 + (VERTICAL_SIZE * scale) / 2);

        if (_cud.GetControllerPixelPort(i)->GetVirtualStringCount() > 0)
        {
            for (int j = 0; j < _cud.GetControllerPixelPort(i)->GetVirtualStringCount(); j++)
            {
                colPos = (LEFT_RIGHT_MARGIN * scale) + (HORIZONTAL_SIZE * scale) + (HORIZONTAL_GAP * scale);

                for (auto it : _cud.GetControllerPixelPort(i)->GetVirtualString(j)->_models)
                {
                    switch (it->GetSmartRemote())
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
                    dc.SetPen(*wxGREY_PEN);
                    dc.DrawRectangle(colPos, rowPos, (HORIZONTAL_SIZE * scale), VERTICAL_SIZE * scale);
                    dc.DrawText(it->GetName(), colPos + 2, rowPos + 2);
                    dc.DrawText(wxString::Format("Pixels: %d", it->Channels() / 3), colPos + 2, rowPos + 2 + (VERTICAL_SIZE * scale) / 2);
                    colPos += (HORIZONTAL_SIZE * scale) + (HORIZONTAL_GAP * scale);
                    dc.SetBrush(*wxWHITE_BRUSH);
                }
                rowPos += (VERTICAL_SIZE * scale) + (VERTICAL_GAP * scale);
            }
        }
        else
        {
            rowPos += (VERTICAL_SIZE * scale) + (VERTICAL_GAP * scale);
        }
    }
    for (int i = 1; i <= _cud.GetMaxSerialPort(); i++)
    {
        int colPos = (LEFT_RIGHT_MARGIN * scale);
        dc.SetPen(*wxGREEN_PEN);
        dc.DrawRoundedRectangle(colPos, rowPos, (HORIZONTAL_SIZE * scale), (VERTICAL_SIZE * scale), (CORNER_ROUNDING * scale));
        dc.DrawText(wxString::Format("Serial Port %d", i), colPos + 2, rowPos + 2);
        dc.DrawText(wxString::Format("Channels: %d", _cud.GetControllerSerialPort(i)->Channels()), colPos + 2, rowPos + 2 + (VERTICAL_SIZE * scale) / 2);
        colPos += (HORIZONTAL_SIZE * scale) + (HORIZONTAL_GAP * scale);

        for (auto it : _cud.GetControllerSerialPort(i)->GetModels())
        {
            dc.SetPen(*wxGREY_PEN);
            dc.DrawRectangle(colPos, rowPos, (HORIZONTAL_SIZE * scale), (VERTICAL_SIZE * scale));
            dc.DrawText(it->GetName(), colPos + 2, rowPos + 2);
            dc.DrawText(wxString::Format("Channels: %d", it->Channels()), colPos + 2, rowPos + 2 + (VERTICAL_SIZE * scale) / 2);
            colPos += (HORIZONTAL_SIZE * scale) + (HORIZONTAL_GAP * scale);
        }

        rowPos += (VERTICAL_SIZE * scale) + (VERTICAL_GAP * scale);
    }
}

void ControllerVisualiseDialog::SaveCSV()
{
	wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
	wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, wxEmptyString, wxEmptyString, "Export files (*.csv)|*.csv", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	if (filename.IsEmpty()) return;	

	std::vector<wxString> lines;
	int columnSize = 0;

	for (int i = 1; i <= _cud.GetMaxPixelPort(); i++)
	{
		wxString line = wxString::Format("Pixel Port %d,", i);

		if (columnSize < _cud.GetControllerPixelPort(i)->GetModels().size())
			columnSize = _cud.GetControllerPixelPort(i)->GetModels().size();

		for (auto it : _cud.GetControllerPixelPort(i)->GetModels())
		{
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
		for (auto it : _cud.GetControllerSerialPort(i)->GetModels())
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

	wxString header = "Output,";
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
