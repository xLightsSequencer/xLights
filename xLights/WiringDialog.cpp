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
#include <wx/config.h>
#include "models/Model.h"
#include <wx/cmndata.h>
#include <wx/prntbase.h>
#include <wx/msgdlg.h>
#include <log4cpp/Category.hh>

#define MINFONTSIZE 8
#define FONTSIZEINCREMENT 4

//(*IdInit(WiringDialog)
const long WiringDialog::ID_STATICBITMAP1 = wxNewId();
//*)

const long WiringDialog::ID_MNU_EXPORT = wxNewId();
const long WiringDialog::ID_MNU_EXPORTLARGE = wxNewId();
const long WiringDialog::ID_MNU_PRINT = wxNewId();
const long WiringDialog::ID_MNU_DARK = wxNewId();
const long WiringDialog::ID_MNU_LIGHT = wxNewId();
const long WiringDialog::ID_MNU_FRONT = wxNewId();
const long WiringDialog::ID_MNU_REAR = wxNewId();
const long WiringDialog::ID_MNU_FONTSMALLER = wxNewId();
const long WiringDialog::ID_MNU_FONTLARGER = wxNewId();

BEGIN_EVENT_TABLE(WiringDialog,wxDialog)
	//(*EventTable(WiringDialog)
	//*)
END_EVENT_TABLE()

WiringDialog::WiringDialog(wxWindow* parent, wxString modelname, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _dark = true;
    _rear = true;
    _multilight = false;
    _cols = 1;
    _rows = 1;

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

    wxConfigBase* config = wxConfigBase::Get();
    config->Read("xLightsWDFontSize", &_fontSize, 12);
}

void WiringDialog::SetData(Model* model)
{
    _multilight = false;
    int nodes = model->GetNodeCount();

    std::vector<NodeBaseClassPtr> nodeList;
    model->InitRenderBufferNodes("Per Preview", "2D", "None", nodeList, _cols, _rows);

    float minx = 999999;
    float miny = 999999;
    float maxx = 0;
    float maxy = 0;
    for (int i = 0; i < nodes; ++i)
    {
        auto points = nodeList[i]->Coords;

        for (auto it = points.begin(); it != points.end(); ++it)
        {
            float x = it->screenX;
            float y = it->screenY;

            if (x < minx) minx = x;
            if (x > maxx) maxx = x;
            if (y < miny) miny = y;
            if (y > maxy) maxy = y;
        }
    }

    // The 4 rather than 1 adds some extra space
    _cols = maxx - minx + 8;
    _rows = maxy - miny + 4;

    int string = 0;
    int stringnode = 1;
    std::map<int, std::list<wxPoint>> data;
    for (int i = 0; i < nodes; ++i)
    {
        if (model->GetNodeStringNumber(i) != string && model->GetDisplayAs() != "Custom")
        {
            _points[string] = data;
            data.clear();
            stringnode = 1;
            string = model->GetNodeStringNumber(i);
        }
        auto points = nodeList[i]->Coords;
        for (auto it = points.begin(); it != points.end(); ++it)
        {
            float x = it->screenX;
            float y = it->screenY;
            x = x - minx;
            y = y - miny + 2;
            if (model->GetDisplayAs() != "Icicles")
            {
                y = _rows - y;
            }
            wxASSERT(x >= 0 && x < _cols);
            wxASSERT(y >= 0 && y <= _rows);
            data[stringnode].push_back(wxPoint(x, y));
            if (!_multilight && data[stringnode].size() > 1) _multilight = true;
        }
        stringnode++;
    }
    _points[string] = data;
}

void WiringDialog::SetData(wxGrid* grid, bool reverse)
{
    _cols = grid->GetNumberCols();
    _rows = grid->GetNumberRows();

    _points[0] = ExtractPoints(grid, reverse);

    _multilight = false;
    for (auto itp = _points.begin(); itp != _points.end(); ++itp)
    {
        for (auto it = itp->second.begin(); it != itp->second.end(); ++it)
        {
            if (it->second.size() > 1)
            {
                _multilight = true;
                break;
            }
        }
    }

    Render();
}

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

#define ADJUST_WIDTH 40.0
#define ADJUST_HEIGHT 80.0
#define FRONT_X_ADJUST (ADJUST_WIDTH / 2.0)
#define SCALE_WIDTH 0.8
#define SCALE_HEIGHT 0.8
#define PRINTSCALE 6.0

int AdjustX(int x, bool printer)
{
    return x + ADJUST_WIDTH / 2.0 + (printer ? 50 : 0);
}

int AdjustY(int y)
{
    return y + ADJUST_HEIGHT / 2.0;
}

void WiringDialog::RenderNodes(wxBitmap& bitmap, std::map<int, std::map<int, std::list<wxPoint>>>& points, int width, int height, bool printer)
{
    wxMemoryDC dc;
    dc.SelectObject(bitmap);

    int pageWidth = bitmap.GetScaledWidth() * SCALE_WIDTH;
    int pageHeight = bitmap.GetScaledHeight() * SCALE_HEIGHT;

    if (_dark && !printer)
    {
        dc.SetPen(*wxBLACK_PEN);
        dc.SetBrush(*wxBLACK_BRUSH);
    }
    else
    {
        dc.SetPen(*wxWHITE_PEN);
        dc.SetBrush(*wxWHITE_BRUSH);
    }
    dc.DrawRectangle(wxPoint(0, 0), bitmap.GetScaledSize());

    int r = 0.6 * std::min(pageWidth / width / 2, pageHeight / height / 2);
    if (r == 0) r = 1;

    if (!printer)
    {
        if (r > 5) r = 5;
        if (r < 3) r = 3;
    }

    int printScale = 1;
    int fontSize = _fontSize;
    if (printer)
    {
        fontSize *= PRINTSCALE;
        printScale = PRINTSCALE;
    }

    wxFont font(fontSize, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString, wxFONTENCODING_DEFAULT);
    dc.SetFont(font);

    wxPen* pen;
    if (_dark && !printer)
    {
        pen = new wxPen(*wxYELLOW, 2);
    }
    else
    {
        if (printer)
        {
            pen = new wxPen(*wxBLACK, 5);
        }
        else
        {
            pen = new wxPen(*wxBLACK, 2);
        }
    }

    // draw the lines
    for (auto itp = points.begin(); itp != points.end(); ++itp)
    {
        int last = -10;
        wxPoint lastpt = wxPoint(0, 0);

        for (auto it = itp->second.begin(); it != itp->second.end(); ++it)
        {
            dc.SetBrush(*wxWHITE_BRUSH);
            dc.SetPen(*wxBLACK_PEN);
            int x = (width - it->second.front().x) * pageWidth / width;
            if (!_rear)
            {
                x = pageWidth - x + FRONT_X_ADJUST;
            }
            int y = it->second.front().y * pageHeight / height;

            if (it->first == last + 1)
            {
                dc.SetPen(*pen);
                int lastx = (width - lastpt.x) * pageWidth / width;
                if (!_rear)
                {
                    lastx = pageWidth - lastx + FRONT_X_ADJUST;
                }
                int lasty = lastpt.y * pageHeight / height;
                dc.DrawLine(AdjustX(lastx, printer), AdjustY(lasty), AdjustX(x, printer), AdjustY(y));
            }

            last = it->first;
            lastpt = it->second.front();
        }
    }

    // now the circles
    for (auto itp = points.begin(); itp != points.end(); ++itp)
    {
        for (auto it = itp->second.begin(); it != itp->second.end(); ++it)
        {
            int x = (width - it->second.front().x) * pageWidth / width;
            if (!_rear)
            {
                x = pageWidth - x + FRONT_X_ADJUST;
            }
            int y = it->second.front().y * pageHeight / height;
            dc.DrawCircle(AdjustX(x, printer), AdjustY(y), r);
        }
    }

    // render the text after the lines so the text is not drawn over
    int string = 1;
    for (auto itp = points.begin(); itp != points.end(); ++itp)
    {
        for (auto it = itp->second.begin(); it != itp->second.end(); ++it)
        {
            int x = (width - it->second.front().x) * pageWidth / width;
            if (!_rear)
            {
                x = pageWidth - x + FRONT_X_ADJUST;
            }
            int y = it->second.front().y * pageHeight / height;

            std::string label;
            if(points.size() == 1)
            {
                label = wxString::Format("%d", it->first).ToStdString();
            }
            else
            {
                label = wxString::Format("%d:%d", string, it->first).ToStdString();
            }

            if (_dark && !printer)
            {
                RenderText(label, dc, AdjustX(x + r + 2, printer), AdjustY(y), *wxLIGHT_GREY, *wxBLACK);
            }
            else
            {
                RenderText(label, dc, AdjustX(x + r + 2, printer), AdjustY(y), *wxBLACK, *wxWHITE);
            }
        }
        string++;
    }

    if (_dark && !printer)
    {
        if (_rear)
        {
            RenderText("CAUTION: Reverse view", dc, AdjustX(0, printer), 20, *wxGREEN, *wxBLACK);
        }
        else
        {
            RenderText("CAUTION: Front view", dc, AdjustX(0, printer), 20, *wxBLUE, *wxBLACK);
        }
        RenderText("Model: " + _modelname, dc, AdjustX(0, printer), 20 + fontSize + 4 * printScale, *wxGREEN, *wxBLACK);
    }
    else
    {
        if (_rear)
        {
            RenderText("CAUTION: Reverse view", dc, AdjustX(0, printer), 20, *wxBLACK, *wxWHITE);
        }
        else
        {
            RenderText("CAUTION: Front view", dc, AdjustX(0, printer), 20, *wxBLUE, *wxWHITE);
        }
        RenderText("Model: " + _modelname, dc, AdjustX(0, printer), 20 + fontSize + 4 * printScale, *wxBLACK, *wxWHITE);
    }

    dc.SetPen(*wxBLACK_PEN);
    delete pen;
}

WiringPrintout::WiringPrintout(WiringDialog* wiringDialog)
{
    _wiringDialog = wiringDialog;
}

bool WiringPrintout::OnPrintPage(int pageNum)
{
    if (pageNum != 1) return false;

    wxRect rect = GetLogicalPageRect();
    wxBitmap bmp;
    bmp.Create(rect.GetWidth() * 0.95f, rect.GetHeight() * 0.95f);
    _wiringDialog->DrawBitmap(bmp, true);

    wxDC* dc = GetDC();
    dc->DrawBitmap(bmp, 0, 0);

    return true;
}

void WiringDialog::RenderMultiLight(wxBitmap& bitmap, std::map<int, std::map<int, std::list<wxPoint>>>& points, int width, int height, bool printer)
{
    static wxColor magenta(255, 0, 255);
    static const wxColor* colors[] = { wxRED, wxBLUE, wxGREEN, wxYELLOW, wxLIGHT_GREY, wxCYAN, wxWHITE, &magenta };
    static const wxColor* lightColors[] = { wxRED, wxBLUE, wxGREEN, wxYELLOW, wxLIGHT_GREY, wxCYAN, wxBLACK, &magenta };
    static int colorcnt = sizeof(colors) / sizeof(wxColor*);
    wxMemoryDC dc;
    dc.SelectObject(bitmap);

    int pageWidth = bitmap.GetScaledWidth() * SCALE_WIDTH;
    int pageHeight = bitmap.GetScaledHeight() * SCALE_HEIGHT;

    if (_dark && !printer)
    {
        dc.SetPen(*wxBLACK_PEN);
        dc.SetBrush(*wxBLACK_BRUSH);
    }
    else
    {
        dc.SetPen(*wxWHITE_PEN);
        dc.SetBrush(*wxWHITE_BRUSH);
    }
    dc.DrawRectangle(wxPoint(0, 0), bitmap.GetScaledSize());

    int printScale = 1;
    int fontSize = _fontSize;
    if (printer)
    {
        fontSize *= PRINTSCALE;
        printScale = PRINTSCALE;
    }

    wxFont font(fontSize, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString, wxFONTENCODING_DEFAULT);
    dc.SetFont(font);

    int cindex = 0;

    int r = 0.6 * std::min(pageWidth / width / 2, pageHeight / height / 2);
    if (r == 0) r = 1;

    if (!printer)
    {
        if (r > 5) r = 5;
        if (r < 3) r = 3;
    }

    for (auto itp = points.begin(); itp != points.end(); ++itp)
    {
        for (auto it = itp->second.begin(); it != itp->second.end(); ++it)
        {
            if (_dark && !printer)
            {
                dc.SetBrush(wxBrush(*colors[cindex], wxBRUSHSTYLE_SOLID));
            }
            else
            {
                dc.SetBrush(wxBrush(*lightColors[cindex], wxBRUSHSTYLE_SOLID));
            }

            for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
            {
                int x = (width - it2->x) * pageWidth / width;
                if (!_rear)
                {
                    x = pageWidth - x + FRONT_X_ADJUST;
                }
                int y = it2->y * pageHeight / height;
                dc.DrawCircle(AdjustX(x, printer), AdjustY(y), r);
            }

            cindex++;
            cindex %= colorcnt;
        }
    }

    int string = 1;
    for (auto itp = points.begin(); itp != points.end(); ++itp)
    {
        for (auto it = itp->second.begin(); it != itp->second.end(); ++it)
        {
            for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
            {
                int x = (width - it2->x) * pageWidth / width;
                if (!_rear)
                {
                    x = pageWidth - x + FRONT_X_ADJUST;
                }
                int y = it2->y * pageHeight / height;

                std::string label;
                if (points.size() == 1)
                {
                    label = wxString::Format("%d", it->first).ToStdString();
                }
                else
                {
                    label = wxString::Format("%d:%d", string, it->first).ToStdString();
                }

                if (_dark && !printer)
                {
                    RenderText(label, dc, AdjustX(x + r + 2, printer), AdjustY(y), *wxLIGHT_GREY, *wxBLACK);
                }
                else
                {
                    RenderText(label, dc, AdjustX(x + r + 2, printer), AdjustY(y), *wxBLACK, *wxWHITE);
                }
            }
        }
        string++;
    }

    if (_dark && !printer)
    {
        if (_rear)
        {
            RenderText("CAUTION: Reverse view", dc, AdjustX(0, printer), 20, *wxGREEN, *wxBLACK);
        }
        else
        {
            RenderText("CAUTION: Front view", dc, AdjustX(0, printer), 20, *wxBLUE, *wxBLACK);
        }
        RenderText("Model: " + _modelname, dc, AdjustX(0, printer), 20 + fontSize + 4 * printScale, *wxGREEN, *wxBLACK);
    }
    else
    {
        if (_rear)
        {
            RenderText("CAUTION: Reverse view", dc, AdjustX(0, printer), 20, *wxBLACK, *wxWHITE);
        }
        else
        {
            RenderText("CAUTION: Front view", dc, AdjustX(0, printer), 20, *wxBLUE, *wxWHITE);
        }
        RenderText("Model: " + _modelname, dc, AdjustX(0, printer), 20 + fontSize + 4 * printScale, *wxBLACK, *wxWHITE);
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

void WiringDialog::OnResize(wxSizeEvent& event)
{
    Render();
    event.Skip();
}

void WiringDialog::RightClick(wxContextMenuEvent& event)
{
    wxMenu mnuLayer;
    mnuLayer.Append(ID_MNU_EXPORT, "Export");
    mnuLayer.Append(ID_MNU_EXPORTLARGE, "Export Large");
    mnuLayer.Append(ID_MNU_PRINT, "Print");
    auto dark = mnuLayer.Append(ID_MNU_DARK, "Dark", "", wxITEM_RADIO);
    auto light = mnuLayer.Append(ID_MNU_LIGHT, "Light", "", wxITEM_RADIO);
    if (_dark)
    {
        dark->Check();
    }
    else
    {
        light->Check();
    }
    auto fontSmaller = mnuLayer.Append(ID_MNU_FONTSMALLER, "Smaller Font");
    if (_fontSize <= MINFONTSIZE) fontSmaller->Enable(false);
    mnuLayer.Append(ID_MNU_FONTLARGER, "Larger Font");
    auto front = mnuLayer.Append(ID_MNU_FRONT, "Front", "", wxITEM_RADIO);
    auto rear = mnuLayer.Append(ID_MNU_REAR, "Rear", "", wxITEM_RADIO);
    if (_rear)
    {
        rear->Check();
    }
    else
    {
        front->Check();
    }
    mnuLayer.Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&WiringDialog::OnPopup, nullptr, this);
    PopupMenu(&mnuLayer);
}

void WiringDialog::OnPopup(wxCommandEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    int id = event.GetId();
    if (id == ID_MNU_EXPORT)
    {
        wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, _modelname, wxEmptyString, "PNG File (*.png)|*.png", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (filename != "")
        {
            wxImage img = _bmp.ConvertToImage();
            img.SaveFile(filename, wxBITMAP_TYPE_PNG);
        }
    }
    else if (id == ID_MNU_EXPORTLARGE)
    {
        wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, _modelname, wxEmptyString, "PNG File (*.png)|*.png", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (filename != "")
        {
            wxImage img = Render(4096, 2048).ConvertToImage();
            img.SaveFile(filename, wxBITMAP_TYPE_PNG);
        }
    }
    else if (id == ID_MNU_PRINT)
    {
        static wxPrintDialogData printDialogData;
        wxPrinter printer(&printDialogData);

        WiringPrintout printout(this);

        if (!printer.Print(this, &printout, true))
        {
            if (wxPrinter::GetLastError() == wxPRINTER_ERROR)
            {
                logger_base.error("Problem printing. %d", wxPrinter::GetLastError());
                wxMessageBox("Problem printing.");
            }
        }
        else
        {
            printDialogData = printer.GetPrintDialogData();
        }
    }
    else if (id == ID_MNU_DARK)
    {
        _dark = true;
        Render();
    }
    else if (id == ID_MNU_LIGHT)
    {
        _dark = false;
        Render();
    }
    else if (id == ID_MNU_FRONT)
    {
        _rear = false;
        Render();
    }
    else if (id == ID_MNU_REAR)
    {
        _rear = true;
        Render();
    }
    else if (id == ID_MNU_FONTLARGER)
    {
        _fontSize += FONTSIZEINCREMENT;
        wxConfigBase* config = wxConfigBase::Get();
        config->Write(_("xLightsWDFontSize"), _fontSize);
        Render();
    }
    else if (id == ID_MNU_FONTSMALLER)
    {
        _fontSize -= FONTSIZEINCREMENT;
        if (_fontSize < MINFONTSIZE) _fontSize = MINFONTSIZE;
        wxConfigBase* config = wxConfigBase::Get();
        config->Write(_("xLightsWDFontSize"), _fontSize);
        Render();
    }
}

wxBitmap WiringDialog::Render(int w, int h)
{
    wxBitmap bmp;
    bmp.CreateScaled(w, h, wxBITMAP_SCREEN_DEPTH, 1.0);

    DrawBitmap(bmp);

    return bmp;
}

void WiringDialog::Render()
{
    int w, h;
    GetClientSize(&w, &h);

    _bmp.CreateScaled(w, h, wxBITMAP_SCREEN_DEPTH, 1.0);  // Using GetContentScaleFactor() was causing it to scale too much on some Windows systems.

    DrawBitmap(_bmp);

    StaticBitmap_Wiring->SetBitmap(_bmp);
    Refresh();
}

void WiringDialog::DrawBitmap(wxBitmap& bitmap, bool printer)
{
    if (_multilight)
    {
        RenderMultiLight(bitmap, _points, _cols, _rows, printer);
    }
    else
    {
        RenderNodes(bitmap, _points, _cols, _rows, printer);
    }
}
