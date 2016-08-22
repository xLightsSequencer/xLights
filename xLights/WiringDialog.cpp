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
#include <wx/menu.h>
#include <wx/filepicker.h>

//(*IdInit(WiringDialog)
const long WiringDialog::ID_STATICBITMAP1 = wxNewId();
//*)

const long WiringDialog::ID_MNU_EXPORT = wxNewId();

BEGIN_EVENT_TABLE(WiringDialog,wxDialog)
	//(*EventTable(WiringDialog)
	//*)
END_EVENT_TABLE()

WiringDialog::WiringDialog(wxWindow* parent, wxGrid* grid, bool reverse, wxString modelname, wxWindowID id,const wxPoint& pos,const wxSize& size)
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

    Connect(ID_STATICBITMAP1, wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&WiringDialog::RightClick);

    _modelname = modelname;

    wxScreenDC sdc;
    wxSize s = sdc.GetSize();
    bmp.CreateScaled(s.GetWidth(), s.GetHeight(), wxBITMAP_SCREEN_DEPTH, GetContentScaleFactor());

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

//wxBitmap WiringDialog::RenderBitmap(std::map<int, std::list<wxPoint>>& points, int w, int h)
//{
    
//}


void RenderText(const wxString& text, wxMemoryDC& dc, int x, int y, wxColor fore, wxColor back)
{
    dc.SetTextForeground(back);
    dc.DrawText(text, x - 1, y);
    dc.DrawText(text, x+1, y);
    dc.DrawText(text, x, y-1);
    dc.DrawText(text, x, y+1);

    dc.SetTextForeground(fore);
    dc.DrawText(text, x, y);
}


void WiringDialog::RenderNodes(std::map<int, std::list<wxPoint>>& points, int width, int height)
{
    wxMemoryDC dc;
    dc.SelectObject(bmp);

    dc.SetPen(*wxBLACK_PEN);
    dc.SetBrush(*wxBLACK_BRUSH);
    dc.DrawRectangle(wxPoint(0, 0), bmp.GetScaledSize());

    int r = 0.6 * std::min(bmp.GetScaledWidth() / width / 2, bmp.GetScaledHeight() / height / 2);
    if (r == 0) r = 1;
    if (r > 5) r = 5;

    wxFont font(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString, wxFONTENCODING_DEFAULT);
    dc.SetFont(font);

    int last = -10;
    wxPoint lastpt = wxPoint(0, 0);
    wxPen yellowPen(*wxYELLOW, 2);

    // draw the lines
    for (auto it = points.begin(); it != points.end(); ++it)
    {
        dc.SetBrush(*wxWHITE_BRUSH);
        dc.SetPen(*wxBLACK_PEN);
        int x = (width - it->second.front().x) * (bmp.GetScaledWidth() - 40) / width - 20;
        int y = 20 + it->second.front().y * (bmp.GetScaledHeight() - 40) / height;

        if (it->first == last + 1)
        {
            dc.SetPen(yellowPen);
            int lastx = (width - lastpt.x) * (bmp.GetScaledWidth() - 40) / width - 20;
            int lasty = 20 + lastpt.y * (bmp.GetScaledHeight() - 40) / height;
            dc.DrawLine(lastx, lasty, x, y);
        }

        last = it->first;
        lastpt = it->second.front();
    }

    // now the circles
    for (auto it = points.begin(); it != points.end(); ++it)
    {
        int x = (width - it->second.front().x) * (bmp.GetScaledWidth() - 40) / width - 20;
        int y = 20 + it->second.front().y * (bmp.GetScaledHeight() - 40) / height;
        dc.DrawCircle(x, y, r);
    }

    // render the text after the lines so the text is not drawn over
    for (auto it = points.begin(); it != points.end(); ++it)
    {
        int x = (width - it->second.front().x) * (bmp.GetScaledWidth() - 40) / width - 20;
        int y = 20 + it->second.front().y * (bmp.GetScaledHeight() - 40) / height;
        RenderText(wxString::Format("%d", it->first), dc, x + r + 2, y, *wxLIGHT_GREY, *wxBLACK);
    }

    RenderText("CAUTION: Reverse view", dc, 20, 20, *wxGREEN, *wxBLACK);
    RenderText("Model: " + _modelname, dc, 20, 40, *wxGREEN, *wxBLACK);
}

void WiringDialog::RenderMultiLight(std::map<int, std::list<wxPoint>>& points, int width, int height)
{
    static wxColor magenta(255, 0, 255);
    static const wxColor* colors[] = { wxRED, wxBLUE, wxGREEN, wxYELLOW, wxLIGHT_GREY, wxCYAN, wxWHITE, &magenta};
    static int colorcnt = sizeof(colors) / sizeof(wxColor*);
    wxMemoryDC dc;
    dc.SelectObject(bmp);

    dc.SetPen(*wxBLACK_PEN);
    dc.SetBrush(*wxBLACK_BRUSH);
    dc.DrawRectangle(wxPoint(0, 0), bmp.GetScaledSize());

    wxFont font(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString, wxFONTENCODING_DEFAULT);
    dc.SetFont(font);

    int cindex = 0;

    int r = 0.8 * std::min(bmp.GetScaledWidth() / width / 2, bmp.GetScaledHeight() / height / 2);
    if (r == 0) r = 1;
    if (r > 5) r = 5;

    for (auto it = points.begin(); it != points.end(); ++it)
    {
        dc.SetBrush(wxBrush(*colors[cindex], wxBRUSHSTYLE_SOLID));

        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
        {
            int x = (width - it2->x) * (bmp.GetScaledWidth()-40) / width - 20;
            int y = 20 + it2->y * (bmp.GetScaledHeight()-40) / height;
            dc.DrawCircle(x, y, r);
        }

        cindex++;
        cindex %= colorcnt;
    }

    for (auto it = points.begin(); it != points.end(); ++it)
    {
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
        {
            int x = (width - it2->x) * (bmp.GetScaledWidth() - 40) / width - 20;
            int y = 20 + it2->y * (bmp.GetScaledHeight() - 40) / height;

            RenderText(wxString::Format("%d", it->first), dc, x + r + 2, y, *wxLIGHT_GREY, *wxBLACK);
        }
    }

    RenderText("CAUTION: Reverse view", dc, 20, 20, *wxGREEN, *wxBLACK);
    RenderText("Model: " + _modelname, dc, 20, 40, *wxGREEN, *wxBLACK);
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
    float factor = GetContentScaleFactor();
    img = img.Scale(s.x * factor, s.y * factor, wxIMAGE_QUALITY_HIGH);
    sizedbmp = wxBitmap(img, -1 , factor);
}

void WiringDialog::OnResize(wxSizeEvent& event)
{
    ResizeBitmap();
    StaticBitmap_Wiring->SetBitmap(sizedbmp);
    event.Skip();
}

void WiringDialog::RightClick(wxContextMenuEvent& event)
{
    wxMenu mnuLayer;
    mnuLayer.Append(ID_MNU_EXPORT, "Export");
    mnuLayer.Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&WiringDialog::OnPopup, nullptr, this);
    PopupMenu(&mnuLayer);
}

void WiringDialog::OnPopup(wxCommandEvent& event)
{
    int id = event.GetId();
    if (id == ID_MNU_EXPORT)
    {
        wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, _modelname, wxEmptyString, "PNG File (*.png)|*.png", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (filename != "")
        {
            wxImage img = bmp.ConvertToImage();
            img.SaveFile(filename, wxBITMAP_TYPE_PNG);
        }
    }
}