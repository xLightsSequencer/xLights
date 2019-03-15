#include "ZCPPVisualiseDialog.h"

//(*InternalHeaders(ZCPPVisualiseDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/dcclient.h>

//(*IdInit(ZCPPVisualiseDialog)
const long ZCPPVisualiseDialog::ID_PANEL1 = wxNewId();
const long ZCPPVisualiseDialog::ID_SCROLLBAR1 = wxNewId();
const long ZCPPVisualiseDialog::ID_SCROLLBAR2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(ZCPPVisualiseDialog,wxDialog)
	//(*EventTable(ZCPPVisualiseDialog)
	//*)
END_EVENT_TABLE()

#define TOP_BOTTOM_MARGIN 10
#define VERTICAL_GAP 5
#define VERTICAL_SIZE 20
#define LEFT_RIGHT_MARGIN TOP_BOTTOM_MARGIN
#define HORIZONTAL_GAP VERTICAL_GAP
#define HORIZONTAL_SIZE 100
#define CORNER_ROUNDING 5

ZCPPVisualiseDialog::ZCPPVisualiseDialog(wxWindow* parent, UDController& cud, wxWindowID id,const wxPoint& pos,const wxSize& size) : _cud(cud)
{
	//(*Initialize(ZCPPVisualiseDialog)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, _("ZCPP Visualiser"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxCLOSE_BOX|wxMAXIMIZE_BOX, _T("id"));
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
	FlexGridSizer1->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	SetSizer(FlexGridSizer1);
	SetSizer(FlexGridSizer1);
	Layout();

	Panel1->Connect(wxEVT_PAINT,(wxObjectEventFunction)&ZCPPVisualiseDialog::OnPanel1Paint,0,this);
	Panel1->Connect(wxEVT_SIZE,(wxObjectEventFunction)&ZCPPVisualiseDialog::OnPanel1Resize,0,this);
	Connect(ID_SCROLLBAR1,wxEVT_SCROLL_TOP|wxEVT_SCROLL_BOTTOM|wxEVT_SCROLL_LINEUP|wxEVT_SCROLL_LINEDOWN|wxEVT_SCROLL_PAGEUP|wxEVT_SCROLL_PAGEDOWN|wxEVT_SCROLL_THUMBTRACK|wxEVT_SCROLL_THUMBRELEASE|wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&ZCPPVisualiseDialog::OnScrollBar_VerticalScroll);
	Connect(ID_SCROLLBAR1,wxEVT_SCROLL_THUMBTRACK,(wxObjectEventFunction)&ZCPPVisualiseDialog::OnScrollBar_VerticalScrollThumbTrack);
	Connect(ID_SCROLLBAR1,wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&ZCPPVisualiseDialog::OnScrollBar_VerticalScrollChanged);
	Connect(ID_SCROLLBAR2,wxEVT_SCROLL_TOP|wxEVT_SCROLL_BOTTOM|wxEVT_SCROLL_LINEUP|wxEVT_SCROLL_LINEDOWN|wxEVT_SCROLL_PAGEUP|wxEVT_SCROLL_PAGEDOWN|wxEVT_SCROLL_THUMBTRACK|wxEVT_SCROLL_THUMBRELEASE|wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&ZCPPVisualiseDialog::OnScrollBar_HorizontalScroll);
	Connect(ID_SCROLLBAR2,wxEVT_SCROLL_THUMBTRACK,(wxObjectEventFunction)&ZCPPVisualiseDialog::OnScrollBar_HorizontalScrollThumbTrack);
	Connect(ID_SCROLLBAR2,wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&ZCPPVisualiseDialog::OnScrollBar_HorizontalScrollChanged);
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

ZCPPVisualiseDialog::~ZCPPVisualiseDialog()
{
	//(*Destroy(ZCPPVisualiseDialog)
	//*)
}

void ZCPPVisualiseDialog::OnPanel1Paint(wxPaintEvent& event)
{
    wxPaintDC dc(Panel1);

    int xOffset = ScrollBar_Horizontal->GetThumbPosition();
    int yOffset = ScrollBar_Vertical->GetThumbPosition();

    dc.SetDeviceOrigin(-xOffset, -yOffset);

    dc.SetTextForeground(*wxBLACK);

    int rowPos = TOP_BOTTOM_MARGIN;
    for (int i = 1; i <= _cud.GetMaxPixelPort(); i++)
    {
        int colPos = LEFT_RIGHT_MARGIN;
        dc.SetPen(*wxRED_PEN);
        dc.DrawRoundedRectangle(colPos, rowPos, HORIZONTAL_SIZE, VERTICAL_SIZE, CORNER_ROUNDING);
        dc.DrawText(wxString::Format("Pixel Port %d", i), colPos + 2, rowPos + 2);

        if (_cud.GetControllerPixelPort(i)->GetVirtualStringCount() > 0)
        {
            for (int j = 0; j < _cud.GetControllerPixelPort(i)->GetVirtualStringCount(); j++)
            {
                colPos = LEFT_RIGHT_MARGIN + HORIZONTAL_SIZE + HORIZONTAL_GAP;

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
                    dc.DrawRectangle(colPos, rowPos, HORIZONTAL_SIZE, VERTICAL_SIZE);
                    dc.DrawText(it->GetName(), colPos + 2, rowPos + 2);
                    colPos += HORIZONTAL_SIZE + HORIZONTAL_GAP;
                    dc.SetBrush(*wxWHITE_BRUSH);
                }
                rowPos += VERTICAL_SIZE + VERTICAL_GAP;
            }
        }
        else
        {
            rowPos += VERTICAL_SIZE + VERTICAL_GAP;
        }
    }
    for (int i = 1; i <= _cud.GetMaxSerialPort(); i++)
    {
        int colPos = LEFT_RIGHT_MARGIN;
            dc.SetPen(*wxGREEN_PEN);
        dc.DrawRoundedRectangle(colPos, rowPos, HORIZONTAL_SIZE, VERTICAL_SIZE, CORNER_ROUNDING);
        dc.DrawText(wxString::Format("Serial Port %d", i), colPos + 2, rowPos + 2);
        colPos += HORIZONTAL_SIZE + HORIZONTAL_GAP;

        for (auto it : _cud.GetControllerSerialPort(i)->GetModels())
        {
            dc.SetPen(*wxGREY_PEN);
            dc.DrawRectangle(colPos, rowPos, HORIZONTAL_SIZE, VERTICAL_SIZE);
            dc.DrawText(it->GetName(), colPos + 2, rowPos + 2);
            colPos += HORIZONTAL_SIZE + HORIZONTAL_GAP;
        }

        rowPos += VERTICAL_SIZE + VERTICAL_GAP;
    }
}

void ZCPPVisualiseDialog::OnScrollBar_VerticalScroll(wxScrollEvent& event)
{
    Panel1->Refresh();
}

void ZCPPVisualiseDialog::OnScrollBar_HorizontalScroll(wxScrollEvent& event)
{
    Panel1->Refresh();
}

void ZCPPVisualiseDialog::OnScrollBar_HorizontalScrollThumbTrack(wxScrollEvent& event)
{
    Panel1->Refresh();
}

void ZCPPVisualiseDialog::OnScrollBar_HorizontalScrollChanged(wxScrollEvent& event)
{
    Panel1->Refresh();
}

void ZCPPVisualiseDialog::OnScrollBar_VerticalScrollChanged(wxScrollEvent& event)
{
    Panel1->Refresh();
}

void ZCPPVisualiseDialog::OnScrollBar_VerticalScrollThumbTrack(wxScrollEvent& event)
{
    Panel1->Refresh();
}

void ZCPPVisualiseDialog::OnPanel1Resize(wxSizeEvent& event)
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