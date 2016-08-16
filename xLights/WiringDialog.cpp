#include "WiringDialog.h"

//(*InternalHeaders(WiringDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <map>
#include <list>
#include <wx/position.h>
#include <wx/dcmemory.h>
#include <wx/dcscreen.h>
#include <wx/image.h>

//(*IdInit(WiringDialog)
const long WiringDialog::ID_STATICBITMAP1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(WiringDialog,wxDialog)
	//(*EventTable(WiringDialog)
	//*)
END_EVENT_TABLE()

WiringDialog::WiringDialog(wxWindow* parent, wxGrid* grid, bool reverse, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(WiringDialog)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, _("Custom Model Wiring"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxCLOSE_BOX|wxMAXIMIZE_BOX, _T("id"));
	SetClientSize(wxSize(500,500));
	Move(wxDefaultPosition);
	SetMinSize(wxSize(500,500));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	StaticBitmap_Wiring = new wxStaticBitmap(this, ID_STATICBITMAP1, wxNullBitmap, wxDefaultPosition, wxSize(500,500), wxSIMPLE_BORDER, _T("ID_STATICBITMAP1"));
	FlexGridSizer1->Add(StaticBitmap_Wiring, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 5);
	SetSizer(FlexGridSizer1);
	SetSizer(FlexGridSizer1);
	Layout();

	Connect(wxEVT_SIZE,(wxObjectEventFunction)&WiringDialog::OnResize);
	//*)

    wxScreenDC sdc;
    wxSize s = sdc.GetSize();
    bmp.Create(s);

    std::map<int, std::list<wxPoint>> points = ExtractPoints(grid, reverse);

    bool multilight = false;
    for (auto it = points.begin(); it != points.end(); ++it)
    {
        if (it->second.size() > 1)
        {
            multilight = true;
            break;
        }
    }

    if (multilight)
    {
        RenderMultiLight(points, grid->GetNumberCols(), grid->GetNumberRows());
    }
    else
    {
        RenderNodes(points, grid->GetNumberCols(), grid->GetNumberRows());
    }
    ResizeBitmap();
    StaticBitmap_Wiring->SetBitmap(sizedbmp);
}

void WiringDialog::RenderNodes(std::map<int, std::list<wxPoint>>& points, int width, int height)
{

    wxMemoryDC dc;
    dc.SelectObject(bmp);

    dc.SetPen(*wxBLACK_PEN);
    dc.SetBrush(*wxBLACK_BRUSH);
    dc.DrawRectangle(wxPoint(0, 0), bmp.GetSize());

    int r = 0.6 * std::min(bmp.GetWidth() / width / 2, bmp.GetHeight() / height / 2);
    if (r == 0) r = 1;

    dc.SetTextForeground(*wxGREEN);
    dc.DrawText("CAUTION: Reverse view", 20, 20);

    int last = -10;
    wxPoint lastpt = wxPoint(0, 0);
    dc.SetTextForeground(*wxLIGHT_GREY);

    for (auto it = points.begin(); it != points.end(); ++it)
    {
        dc.SetBrush(*wxWHITE_BRUSH);
        dc.SetPen(*wxBLACK_PEN);
        int x = (width-it->second.front().x) * (bmp.GetWidth()-40) / width - 20;
        int y = 20 + it->second.front().y * (bmp.GetHeight()-40) / height;
        dc.DrawCircle(x, y, r);
        dc.DrawText(wxString::Format("%d", it->first), x + r + 2, y);

        if (it->first == last + 1)
        {
            dc.SetPen(*wxYELLOW_PEN);
            int lastx = (width - lastpt.x) * (bmp.GetWidth()-40) / width - 20;
            int lasty = 20 + lastpt.y * (bmp.GetHeight()-40) / height;
            dc.DrawLine(lastx, lasty, x, y);
        }

        last = it->first;
        lastpt = it->second.front();
    }
}

void WiringDialog::RenderMultiLight(std::map<int, std::list<wxPoint>>& points, int width, int height)
{
    static const wxColor* colors[] = { wxRED, wxBLUE, wxGREEN, wxYELLOW, wxLIGHT_GREY, wxCYAN, wxWHITE};
    static int colorcnt = sizeof(colors) / sizeof(wxColor*);
    wxMemoryDC dc;
    dc.SelectObject(bmp);

    dc.SetPen(*wxBLACK_PEN);
    dc.SetBrush(*wxBLACK_BRUSH);
    dc.DrawRectangle(wxPoint(0, 0), bmp.GetSize());

    dc.SetTextForeground(*wxGREEN);
    dc.DrawText("CAUTION: Reverse view", 20, 20);

    int cindex = 0;

    int r = 0.6 * std::min(bmp.GetWidth() / width / 2, bmp.GetHeight() / height / 2);
    if (r == 0) r = 1;
    dc.SetTextForeground(*wxLIGHT_GREY);

    for (auto it = points.begin(); it != points.end(); ++it)
    {
        dc.SetBrush(wxBrush(*colors[cindex], wxBRUSHSTYLE_SOLID));

        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
        {
            int x = (width - it2->x) * (bmp.GetWidth()-40) / width - 20;
            int y = 20 + it2->y * (bmp.GetHeight()-40) / height;
            dc.DrawCircle(x, y, r);
            dc.DrawText(wxString::Format("%d", it->first), x + r + 2, y);
        }

        cindex++;
        cindex %= colorcnt;
    }
}

std::map<int, std::list<wxPoint>> WiringDialog::ExtractPoints(wxGrid* grid, bool reverse)
{
    std::map<int, std::list<wxPoint>> res;

    for (size_t r = 0; r < grid->GetNumberRows(); r++)
    {
        if (reverse)
        {
            for (int c = grid->GetNumberCols() - 1; c >= 0; c--)
            {
                wxString val = grid->GetCellValue(r, grid->GetNumberCols() - 1 - c);
                if (val != "")
                {
                    res[wxAtoi(val)].push_back(wxPoint(c, r));
                }
            }
        }
        else
        {
            for (int c = 0; c < grid->GetNumberCols(); c++)
            {
                wxString val = grid->GetCellValue(r, c);
                if (val != "")
                {
                    res[wxAtoi(val)].push_back(wxPoint(c, r));
                }
            }
        }
    }

    return res;
}

WiringDialog::~WiringDialog()
{
	//(*Destroy(WiringDialog)
	//*)
}

void WiringDialog::ResizeBitmap(void)
{
    wxSize s = StaticBitmap_Wiring->GetSize();
    sizedbmp = bmp;

    wxImage img = bmp.ConvertToImage();
    img = img.Scale(s.x, s.y, wxIMAGE_QUALITY_HIGH);
    sizedbmp = wxBitmap(img);
}

void WiringDialog::OnResize(wxSizeEvent& event)
{
    ResizeBitmap();
    StaticBitmap_Wiring->SetBitmap(sizedbmp);
    event.Skip();
}
