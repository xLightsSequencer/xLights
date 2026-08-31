/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

//(*InternalHeaders(WiringDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <map>
#include <list>
#include <cmath>

#include <wx/position.h>
#include <wx/dcmemory.h>
#include <wx/dcscreen.h>
#include <wx/image.h>
#include <wx/menu.h>
#include <wx/filepicker.h>
#include <wx/cmndata.h>
#include <wx/prntbase.h>
#include <wx/msgdlg.h>
#include <wx/filedlg.h>

#include "cad/DXFWriter.h"

#include "WiringDialog.h"
#include "models/Model.h"
#include "models/CustomModel.h"

#include "UtilFunctions.h"
#include "shared/utils/wxUtilities.h"
#include "settings/XLightsConfigAdapter.h"

#include <log.h>

#define MINFONTSIZE 8
#define MAXFONTSIZE 72
#define FONTSIZEINCREMENT 4

//(*IdInit(WiringDialog)
const long WiringDialog::ID_STATICBITMAP1 = wxNewId();
const long WiringDialog::ID_STATICTEXT_FONTSIZE = wxNewId();
const long WiringDialog::ID_SLIDER_FONTSIZE = wxNewId();
const long WiringDialog::ID_CHECKBOX_3D = wxNewId();
//*)

const long WiringDialog::ID_MNU_RESET = wxNewId();
const long WiringDialog::ID_MNU_EXPORT = wxNewId();
const long WiringDialog::ID_MNU_EXPORTLARGE = wxNewId();
const long WiringDialog::ID_MNU_EXPORTDXF = wxNewId();
const long WiringDialog::ID_MNU_PRINT = wxNewId();
const long WiringDialog::ID_MNU_DARK = wxNewId();
const long WiringDialog::ID_MNU_GRAY = wxNewId();
const long WiringDialog::ID_MNU_LIGHT = wxNewId();
const long WiringDialog::ID_MNU_FRONT = wxNewId();
const long WiringDialog::ID_MNU_REAR = wxNewId();
const long WiringDialog::ID_MNU_FONTSMALLER = wxNewId();
const long WiringDialog::ID_MNU_FONTLARGER = wxNewId();
const long WiringDialog::ID_MNU_ROTATE = wxNewId();

BEGIN_EVENT_TABLE(WiringDialog,wxDialog)
	//(*EventTable(WiringDialog)
	//*)
END_EVENT_TABLE()

WiringDialog::WiringDialog(wxWindow* parent, wxString modelname, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _rear = true;
    _multilight = false;
    _cols = 1;
    _rows = 1;
    _rotated = false;
    _rotation = 0;

    SetColorTheme(COLORTHEMETYPE::DARK);

	//(*Initialize(WiringDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxBoxSizer* BoxSizer_Toolbar;

	Create(parent, id, _("Custom Model Wiring"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxCLOSE_BOX|wxMAXIMIZE_BOX, _T("id"));
	SetClientSize(wxSize(500,500));
	Move(wxDefaultPosition);
	SetMinSize(wxSize(500,500));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(1);
	BoxSizer_Toolbar = new wxBoxSizer(wxHORIZONTAL);
	StaticText_FontSize = new wxStaticText(this, ID_STATICTEXT_FONTSIZE, _("Text Size:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_FONTSIZE"));
	BoxSizer_Toolbar->Add(StaticText_FontSize, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
	Slider_FontSize = new wxSlider(this, ID_SLIDER_FONTSIZE, 12, MINFONTSIZE, MAXFONTSIZE, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL, wxDefaultValidator, _T("ID_SLIDER_FONTSIZE"));
	BoxSizer_Toolbar->Add(Slider_FontSize, 1, wxALL|wxEXPAND, 5);
	CheckBox_3D = new wxCheckBox(this, ID_CHECKBOX_3D, _("3D (drag to rotate)"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_3D"));
	CheckBox_3D->SetValue(false);
	BoxSizer_Toolbar->Add(CheckBox_3D, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
	FlexGridSizer1->Add(BoxSizer_Toolbar, 1, wxEXPAND, 0);
	StaticBitmap_Wiring = new WiringStaticBitmap(this, ID_STATICBITMAP1, wxNullBitmap, wxDefaultPosition, wxSize(500,500), wxSIMPLE_BORDER, _T("ID_STATICBITMAP1"));
	FlexGridSizer1->Add(StaticBitmap_Wiring, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 5);
	SetSizer(FlexGridSizer1);
	SetSizer(FlexGridSizer1);
	Layout();

	Connect(wxEVT_SIZE,(wxObjectEventFunction)&WiringDialog::OnResize);
	Connect(ID_SLIDER_FONTSIZE,wxEVT_SLIDER,(wxObjectEventFunction)&WiringDialog::OnFontSizeSliderChanged);
	Connect(ID_CHECKBOX_3D,wxEVT_CHECKBOX,(wxObjectEventFunction)&WiringDialog::On3DCheckBoxChanged);
	//*)

    Connect(ID_STATICBITMAP1, wxEVT_CONTEXT_MENU, (wxObjectEventFunction)& WiringDialog::RightClick);

    // We redraw the whole bitmap on every update, so suppress the default
    // background erase to avoid a white flash between the erase and repaint.
    StaticBitmap_Wiring->SetBackgroundStyle(wxBG_STYLE_PAINT);

    // We always hand it a bitmap already sized to match the control's client
    // area, so it should never need to scale/stretch what we give it.
    StaticBitmap_Wiring->SetScaleMode(wxStaticBitmapBase::Scale_None);

    // Pan and zoom events
    StaticBitmap_Wiring->Connect(wxEVT_LEFT_DOWN, (wxObjectEventFunction)& WiringDialog::LeftDown, nullptr, this);
    StaticBitmap_Wiring->Connect(wxEVT_LEFT_UP, (wxObjectEventFunction)& WiringDialog::LeftUp, nullptr, this);
    StaticBitmap_Wiring->Connect(wxEVT_MOTION, (wxObjectEventFunction)& WiringDialog::Motion, nullptr, this);
    StaticBitmap_Wiring->Connect(wxEVT_MOUSE_CAPTURE_LOST, (wxObjectEventFunction)& WiringDialog::CaptureLost, nullptr, this);
    StaticBitmap_Wiring->Connect(wxEVT_MOUSEWHEEL, (wxObjectEventFunction)& WiringDialog::MouseWheel, nullptr, this);
    StaticBitmap_Wiring->Connect(wxEVT_MAGNIFY, (wxObjectEventFunction)& WiringDialog::Magnify, nullptr, this);
    StaticBitmap_Wiring->Connect(wxEVT_LEFT_DCLICK, (wxObjectEventFunction)& WiringDialog::LeftDClick, nullptr, this);

    _modelname = modelname;

    auto* config = GetXLightsConfig();
    config->Read("xLightsWDFontSize", &_fontSize, 12);
    if (_fontSize < MINFONTSIZE) _fontSize = MINFONTSIZE;
    if (_fontSize > MAXFONTSIZE) _fontSize = MAXFONTSIZE;
    Slider_FontSize->SetValue(_fontSize);

    wxPoint loc;
    wxSize sz;
    LoadWindowPosition("xLightsWiringDialogPosition", sz, loc);
    if (loc.x != -1) {
        if (sz.GetWidth() < 500) sz.SetWidth(500);
        if (sz.GetHeight() < 500) sz.SetHeight(500);
        SetPosition(loc);
        SetSize(sz);
        Layout();
    }
    EnsureWindowHeaderIsOnScreen(this);
}

void WiringDialog::SetColorTheme(COLORTHEMETYPE themeType) {
    _selectedTheme.type = themeType;

    switch (themeType) {
        case COLORTHEMETYPE::DARK:
            _selectedTheme.multiLightDark = true;
            _selectedTheme.background = *wxBLACK;
            _selectedTheme.messageFill = *wxWHITE;
            _selectedTheme.messageAltFill = *wxBLUE;
            _selectedTheme.messageOutline = *wxBLACK;
            _selectedTheme.wiringFill =  *wxWHITE;
            _selectedTheme.wiringOutline = *wxYELLOW;
            _selectedTheme.nodeFill = *wxWHITE;
            _selectedTheme.nodeOutline = *wxYELLOW;
            _selectedTheme.labelFill = *wxLIGHT_GREY;
            _selectedTheme.labelOutline = *wxBLACK;
            break;
        case COLORTHEMETYPE::GRAY:
            _selectedTheme.multiLightDark = true;
            _selectedTheme.background = wxColour(48,48,48);
            _selectedTheme.messageFill = *wxWHITE;
            _selectedTheme.messageAltFill = *wxYELLOW;
            _selectedTheme.messageOutline = wxColour(48,48,48);
            _selectedTheme.wiringFill = wxColour(255, 255, 255, 128);
            _selectedTheme.wiringOutline = wxColour(255, 255, 255, 128);
            _selectedTheme.nodeFill = wxColour(255, 255, 255, 128);
            _selectedTheme.nodeOutline = wxColour(255, 255, 255, 128);
            _selectedTheme.labelFill = wxColour(255, 255, 255, 179);
            _selectedTheme.labelOutline = wxColour(48,48,48);
            break;
        case COLORTHEMETYPE::LIGHT:
            _selectedTheme.multiLightDark = false;
            _selectedTheme.background = *wxWHITE;
            _selectedTheme.messageFill = *wxBLACK;
            _selectedTheme.messageAltFill = *wxBLUE;
            _selectedTheme.messageOutline = *wxWHITE;
            _selectedTheme.wiringFill =  *wxWHITE;
            _selectedTheme.wiringOutline = *wxBLACK;
            _selectedTheme.nodeFill = *wxWHITE;
            _selectedTheme.nodeOutline = *wxBLACK;
            _selectedTheme.labelFill = *wxBLACK;
            _selectedTheme.labelOutline = *wxWHITE;
        break;
    }
}

void WiringDialog::SetData(Model* model)
{
    _multilight = false;
    int nodes = model->GetNodeCount();

    std::vector<NodeBaseClassPtr> nodeList;
    model->InitRenderBufferNodes("Per Preview", "2D", "None", nodeList, _cols, _rows, 0);

    float minx = 999999;
    float miny = 999999;
    float maxx = 0;
    float maxy = 0;
    for (int i = 0; i < nodes; ++i)
    {
        auto points = nodeList[i]->Coords;

        for (const auto& it : points)
        {
            float x = it.screenX;
            float y = it.screenY;

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
    std::map<int, std::list<WiringPoint>> data;
    for (int i = 0; i < nodes; ++i)
    {
        if (model->GetNodeStringNumber(i) != string && model->GetDisplayAs() != DisplayAsType::Custom)
        {
            _points[string] = data;
            data.clear();
            stringnode = 1;
            string = model->GetNodeStringNumber(i);
        } else if (model->GetDisplayAs() == DisplayAsType::Custom) {
            // because a custom mdoel can skip nodes we need to reverse engineer the node number
            stringnode = 1 + (nodeList[i]->ActChan - nodeList[0]->ActChan) / nodeList[0]->GetChanCount();
            CustomModel* cm = dynamic_cast<CustomModel*>(model);
            if(cm) {
                if (cm->GetCustomNodeStringNumber(stringnode) - 1 != string) {
                    _points[string] = data;
                    data.clear();
                    string = cm->GetCustomNodeStringNumber(stringnode) - 1;
                }
            }
        }

        auto points = nodeList[i]->Coords;
        for (const auto& it : points)
        {
            float x = it.screenX;
            float y = it.screenY;
            float z = it.screenZ;
            x = x - minx;
            y = y - miny + 2;
            if (model->GetDisplayAs() != DisplayAsType::Icicles)
            {
                y = _rows - y;
            }
            wxASSERT(x >= 0 && x < _cols);
            wxASSERT(y >= 0 && y <= _rows);
            data[stringnode].push_back(WiringPoint(x, y, z));
            if (!_multilight && data[stringnode].size() > 1) _multilight = true;
        }
        stringnode++;
    }
    _points[string] = data;

    // Cube models are hard to make sense of flattened to 2D, so default
    // straight into the 3D view for them. Other model types can carry some
    // incidental z variation even when their wiring is conventionally viewed
    // flat (e.g. a Tree's spiral wrap), so this is deliberately scoped to
    // Cube specifically rather than a generic "has any depth" heuristic.
    _is3D = model->GetDisplayAs() == DisplayAsType::Cube;
    CheckBox_3D->SetValue(_is3D);
}

void WiringDialog::SetData(wxGrid* grid, bool reverse)
{
    _cols = grid->GetNumberCols();
    _rows = grid->GetNumberRows();

    _points[0] = ExtractPoints(grid, reverse);

    _multilight = false;
    for (const auto& itp : _points)
    {
        for (const auto& it : itp.second)
        {
            if (it.second.size() > 1)
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
#define MIN_ZOOM 0.1f
#define MAX_ZOOM 20.0f

int AdjustX(int x, bool printer)
{
    return x + ADJUST_WIDTH / 2.0 + (printer ? 50 : 0);
}

int AdjustY(int y)
{
    return y + ADJUST_HEIGHT / 2.0;
}


// Rotates each point's model-local (x,y,z) around the layout's centre by the
// current drag-controlled yaw/pitch and re-projects it back down to (x,y) via
// a simple orthographic projection (z is kept, but nothing downstream reads
// it yet - it's there for a future depth cue). Returns the input unchanged
// when 3D mode is off, so callers can treat this as a no-op pass-through.
std::map<int, std::map<int, std::list<WiringPoint>>> WiringDialog::Apply3DRotation(const std::map<int, std::map<int, std::list<WiringPoint>>>& points) const
{
    if (!_is3D) return points;

    const double cx = _cols / 2.0;
    const double cy = _rows / 2.0;
    const double cosY = std::cos(_rotY), sinY = std::sin(_rotY);
    const double cosX = std::cos(_rotX), sinX = std::sin(_rotX);

    std::map<int, std::map<int, std::list<WiringPoint>>> result;
    for (const auto& itp : points) {
        auto& outStrings = result[itp.first];
        for (const auto& it : itp.second) {
            auto& outList = outStrings[it.first];
            for (const auto& p : it.second) {
                double px = p.x - cx;
                double py = p.y - cy;
                double pz = p.z;

                // yaw around the vertical (y) axis
                double x1 = px * cosY + pz * sinY;
                double z1 = -px * sinY + pz * cosY;

                // pitch around the horizontal (x) axis
                double y2 = py * cosX - z1 * sinX;
                double z2 = py * sinX + z1 * cosX;

                outList.push_back(WiringPoint(x1 + cx, y2 + cy, z2));
            }
        }
    }
    return result;
}

void WiringDialog::RenderNodes(wxBitmap& bitmap, std::map<int, std::map<int, std::list<WiringPoint>>>& origPoints, int width, int height, bool printer) {
    // Avoid copying the whole point set when 3D is off - only bind a rotated
    // copy when actually needed, otherwise reference origPoints directly.
    std::map<int, std::map<int, std::list<WiringPoint>>> rotatedStorage;
    if (_is3D) rotatedStorage = Apply3DRotation(origPoints);
    const std::map<int, std::map<int, std::list<WiringPoint>>>& points = _is3D ? rotatedStorage : origPoints;
    wxMemoryDC dc;
    dc.SelectObject(bitmap);

    int pageWidth = bitmap.GetScaledWidth() * SCALE_WIDTH;
    int pageHeight = bitmap.GetScaledHeight() * SCALE_HEIGHT;

    // Use a single uniform scale (the tighter-fitting axis) for both x and y
    // so the model keeps its aspect ratio instead of being stretched to fill
    // a canvas whose aspect ratio doesn't match the model's.
    double pageScale = std::min((double)pageWidth / width, (double)pageHeight / height);

    dc.SetPen(wxPen(_selectedTheme.background));
    dc.SetBrush(wxBrush(_selectedTheme.background));

    dc.DrawRectangle(wxPoint(0, 0), bitmap.GetScaledSize());

    int r = 0.6 * pageScale / 2;
    if (r == 0)
        r = 1;

    if (!printer) {
        if (r > 5)
            r = 5;
        if (r < 3)
            r = 3;
    }

    int printScale = 1;
    int fontSize = _fontSize;
    if (printer) {
        fontSize *= PRINTSCALE;
        printScale = PRINTSCALE;
    }

    wxFont font(fontSize, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString, wxFONTENCODING_DEFAULT);
    dc.SetFont(font);

    int penWidth = 2;
    if (printer) {
        penWidth = 5;
    }

    // draw the lines
    int string = 1;
    for (const auto& itp : points) {
        int last = -10;
        WiringPoint lastpt = WiringPoint(0.0, 0.0);

        for (const auto& it : itp.second) {
            if (string % 2 == 0) {
                dc.SetBrush(wxBrush(_selectedTheme.wiringFill));
                dc.SetPen(wxPen(_selectedTheme.labelFill, penWidth));
            } else {
                dc.SetBrush(wxBrush(_selectedTheme.wiringFill));
                dc.SetPen(wxPen(_selectedTheme.wiringOutline, penWidth));
            }

            int x = (width - it.second.front().x) * pageScale;
            if (!_rear) {
                x = pageWidth - x + FRONT_X_ADJUST;
            }
            int y = it.second.front().y * pageScale;

            if (it.first == last + 1) {
                int lastx = (width - lastpt.x) * pageScale;
                if (!_rear) {
                    lastx = pageWidth - lastx + FRONT_X_ADJUST;
                }
                int lasty = lastpt.y * pageScale;
                dc.DrawLine((AdjustX(lastx, printer) * _zoom) + _start.x, (AdjustY(lasty) * _zoom) + _start.y,
                            (AdjustX(x, printer) * _zoom) + _start.x, (AdjustY(y) * _zoom) + _start.y);
            }

            last = it.first;
            lastpt = it.second.front();
        }
        string++;
    }

    // now the circles
    for (const auto& itp : points) {
        bool first = true;
        dc.SetBrush(wxBrush(_selectedTheme.messageAltFill));
        dc.SetPen(wxPen(_selectedTheme.messageAltFill, penWidth));
        for (const auto& it : itp.second) {
            int x = (width - it.second.front().x) * pageScale;
            if (!_rear) {
                x = pageWidth - x + FRONT_X_ADJUST;
            }
            int y = it.second.front().y * pageScale;
            dc.DrawCircle((AdjustX(x, printer) * _zoom) + _start.x, (AdjustY(y) * _zoom) + _start.y, r);
            if(first) {
                dc.SetBrush(wxBrush(_selectedTheme.nodeFill));
                dc.SetPen(wxPen(_selectedTheme.nodeOutline, penWidth));
                first = false;
            }
        }
    }

    // render the text after the lines so the text is not drawn over
    string = 1;
    bool useStringNodeFormat = true;

    // Check if the first node of the second string is 1 (indicating overlapping ranges)
    if (points.size() > 1) {
        auto it = std::next(points.begin()); // Get second string's data
        if (!it->second.empty() && it->second.begin()->first == 1) {
            useStringNodeFormat = false; // First example: use "string:node"
        }
    }

    for (const auto& itp : points) {
        for (const auto& it : itp.second) {
            int x = (width - it.second.front().x) * pageScale;
            if (!_rear) {
                x = pageWidth - x + FRONT_X_ADJUST;
            }
            int y = it.second.front().y * pageScale;

            std::string label;
            if (useStringNodeFormat) { // and the first point on the second string is > 1
                label = wxString::Format("%d", it.first).ToStdString();
            } else {
                label = wxString::Format("%d:%d", string, it.first).ToStdString();
            }

            if (printer) {
                RenderText(label, dc, AdjustX(x + r + 2, printer) + _start.x, AdjustY(y) + _start.y, *wxBLACK, *wxWHITE);
            } else {
                RenderText(label, dc, (AdjustX(x + r + 2, printer) * _zoom) + _start.x, (AdjustY(y) * _zoom) + _start.y, _selectedTheme.labelFill, _selectedTheme.labelOutline);
            }
        }
        string++;
    }

    // Line spacing must scale with the actual font size (now user-adjustable
    // up to MAXFONTSIZE via the slider), not a fixed gap sized for the old
    // small default - otherwise a larger font overlaps the line below it.
    int headerLineHeight = fontSize + 4 * printScale + 3;
    int headerY = 20 + _start.y;
    if (_rear) {
        RenderText("CAUTION: Reverse view", dc, AdjustX(0, printer) + _start.x, headerY, _selectedTheme.messageFill, _selectedTheme.messageOutline);
    } else {
        RenderText("CAUTION: Front view", dc, AdjustX(0, printer) + _start.x, headerY, _selectedTheme.messageAltFill, _selectedTheme.messageOutline);
    }
    headerY += headerLineHeight;

    RenderText("Model: " + _modelname, dc, AdjustX(0, printer) + _start.x, headerY, _selectedTheme.messageFill, _selectedTheme.messageOutline);
    headerY += headerLineHeight;
    RenderText("Rotation: " + std::to_string(_rotation), dc, AdjustX(0, printer) + _start.x, headerY, _selectedTheme.messageFill, _selectedTheme.messageOutline);
    headerY += headerLineHeight;

    size_t totalNodes = 0;
    for (const auto& itp : points) {
        totalNodes += itp.second.size();
    }
    RenderText("Total Nodes: " + std::to_string(totalNodes), dc, AdjustX(0, printer) + _start.x, headerY, _selectedTheme.messageFill, _selectedTheme.messageOutline);

    dc.SetPen(*wxBLACK_PEN);
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

void WiringDialog::RenderMultiLight(wxBitmap& bitmap, std::map<int, std::map<int, std::list<WiringPoint>>>& origPoints, int width, int height, bool printer)
{
    std::map<int, std::map<int, std::list<WiringPoint>>> rotatedStorage;
    if (_is3D) rotatedStorage = Apply3DRotation(origPoints);
    const std::map<int, std::map<int, std::list<WiringPoint>>>& points = _is3D ? rotatedStorage : origPoints;

    static wxColor magenta(255, 0, 255);
    static const wxColor* colors[] = { wxRED, wxBLUE, wxGREEN, wxYELLOW, wxLIGHT_GREY, wxCYAN, wxWHITE, &magenta };
    static const wxColor* lightColors[] = { wxRED, wxBLUE, wxGREEN, wxYELLOW, wxLIGHT_GREY, wxCYAN, wxBLACK, &magenta };
    static int colorcnt = sizeof(colors) / sizeof(wxColor*);
    wxMemoryDC dc;
    dc.SelectObject(bitmap);

    int pageWidth = bitmap.GetScaledWidth() * SCALE_WIDTH;
    int pageHeight = bitmap.GetScaledHeight() * SCALE_HEIGHT;

    // Use a single uniform scale (the tighter-fitting axis) for both x and y
    // so the model keeps its aspect ratio instead of being stretched to fill
    // a canvas whose aspect ratio doesn't match the model's.
    double pageScale = std::min((double)pageWidth / width, (double)pageHeight / height);

    dc.SetPen(wxPen(_selectedTheme.background));
    dc.SetBrush(wxBrush(_selectedTheme.background));

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

    int r = 0.6 * pageScale / 2;
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
            if (_selectedTheme.multiLightDark)
            {
                dc.SetBrush(wxBrush(*colors[cindex], wxBRUSHSTYLE_SOLID));
            }
            else
            {
                dc.SetBrush(wxBrush(*lightColors[cindex], wxBRUSHSTYLE_SOLID));
            }

            for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
            {
                int x = (width - it2->x) * pageScale;
                if (!_rear)
                {
                    x = pageWidth - x + FRONT_X_ADJUST;
                }
                int y = it2->y * pageScale;
                dc.DrawCircle((AdjustX(x, printer) * _zoom) + _start.x, (AdjustY(y) * _zoom) + _start.y, r);
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
                int x = (width - it2->x) * pageScale;
                if (!_rear)
                {
                    x = pageWidth - x + FRONT_X_ADJUST;
                }
                int y = it2->y * pageScale;

                std::string label;
                if (points.size() == 1)
                {
                    label = wxString::Format("%d", it->first).ToStdString();
                }
                else
                {
                    label = wxString::Format("%d:%d", string, it->first).ToStdString();
                }

                if (printer) {
                    RenderText(label, dc, AdjustX(x + r + 2, printer) + _start.x, AdjustY(y) + _start.y, *wxBLACK, *wxWHITE);
                } else {
                    RenderText(label, dc, (AdjustX(x + r + 2, printer) * _zoom) + _start.x, (AdjustY(y) * _zoom) + _start.y, _selectedTheme.labelFill, _selectedTheme.labelOutline);
                }
            }
        }
        string++;
    }

    int headerLineHeight = fontSize + 4 * printScale + 3;
    int headerY = 20 + _start.y;
    if (_rear) {
        RenderText("CAUTION: Reverse view", dc, AdjustX(0, printer) + _start.x, headerY, _selectedTheme.messageFill, _selectedTheme.messageOutline);
    } else {
        RenderText("CAUTION: Front view", dc, AdjustX(0, printer) + _start.x, headerY, _selectedTheme.messageAltFill, _selectedTheme.messageOutline);
    }
    headerY += headerLineHeight;

    RenderText("Model: " + _modelname, dc, AdjustX(0, printer) + _start.x, headerY, _selectedTheme.messageFill, _selectedTheme.messageOutline);
    headerY += headerLineHeight;
    RenderText("Rotation: " + std::to_string(_rotation), dc, AdjustX(0, printer) + _start.x, headerY, _selectedTheme.messageFill, _selectedTheme.messageOutline);
    headerY += headerLineHeight;

    size_t totalNodes = 0;
    for (const auto& itp : points) {
        totalNodes += itp.second.size();
    }
    RenderText("Total Nodes: " + std::to_string(totalNodes), dc, AdjustX(0, printer) + _start.x, headerY, _selectedTheme.messageFill, _selectedTheme.messageOutline);
}

std::map<int, std::list<WiringPoint>> WiringDialog::ExtractPoints(wxGrid* grid, bool reverse)
{
    std::map<int, std::list<WiringPoint>> res;

    for (size_t r = 0; r < (size_t)grid->GetNumberRows(); r++)
    {
        if (reverse)
        {
            for (int c = grid->GetNumberCols() - 1; c >= 0; c--)
            {
                wxString val = grid->GetCellValue(r, grid->GetNumberCols() - 1 - c);
                if (val != "")
                {
                    res[wxAtoi(val)].push_back(WiringPoint(c, r));
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
                    res[wxAtoi(val)].push_back(WiringPoint(c, r));
                }
            }
        }
    }

    return res;
}

void WiringDialog::RotatePoints(int rotateBy) {
    _points.clear();

    float newCenterX;
    float newCenterY;

    if (_rotation == 0) {
        newCenterX = static_cast<float>(_cols) / 2;
        newCenterY = static_cast<float>(_rows) / 2;
    } else {
        newCenterX = static_cast<float>(_rows - 3) / 2;
        newCenterY = static_cast<float>(_cols - 4) / 2;
    }

    double radians = (M_PI / 180) * rotateBy;
    double c = cos(radians);
    double s = sin(radians);
    std::map<int, std::list<WiringPoint>> data;

    int string = 0;
    for (auto itp = _originalPoints.begin(); itp != _originalPoints.end(); ++itp) {
        for (auto it = itp->second.begin(); it != itp->second.end(); ++it) {
            if (_multilight) {
                for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
                    float x = it2->x;
                    float y = it2->y;
                    float newX = (c * (x - newCenterX)) + (s * (y - newCenterY)) + newCenterX;
                    float newY = (c * (y - newCenterY)) - (s * (x - newCenterX)) + newCenterY;
                    data[it->first].push_back(WiringPoint(newX, newY, it2->z));
                }
            } else {
                float x = it->second.front().x;
                float y = it->second.front().y;
                float newX = (c * (x - newCenterX)) + (s * (y - newCenterY)) + newCenterX;
                float newY = (c * (y - newCenterY)) - (s * (x - newCenterX)) + newCenterY;
                data[it->first].push_back(WiringPoint(newX, newY, it->second.front().z));
            }
        }
        _points[string] = data;
        data.clear();
        string++;
    }
}

WiringDialog::~WiringDialog()
{
    //(*Destroy(WiringDialog)
    //*)
    SaveWindowPosition("xLightsWiringDialogPosition", this);
}

void WiringDialog::OnResize(wxSizeEvent& event)
{
    Render();
    event.Skip();
}

void WiringDialog::OnFontSizeSliderChanged(wxCommandEvent& event)
{
    _fontSize = Slider_FontSize->GetValue();
    auto* config = GetXLightsConfig();
    config->Write("xLightsWDFontSize", _fontSize);
    Render();
}

void WiringDialog::On3DCheckBoxChanged(wxCommandEvent& event)
{
    _is3D = CheckBox_3D->GetValue();
    Render();
}

void WiringDialog::RightClick(wxContextMenuEvent& event)
{
    wxMenu mnuLayer;
    mnuLayer.Append(ID_MNU_RESET, "Reset");
    mnuLayer.AppendSeparator();
    mnuLayer.Append(ID_MNU_EXPORT, "Export");
    mnuLayer.Append(ID_MNU_EXPORTLARGE, "Export Large");
#ifdef _DEBUG
    mnuLayer.Append(ID_MNU_EXPORTDXF, "Export as DXF");
#endif
    mnuLayer.Append(ID_MNU_PRINT, "Print");
    mnuLayer.AppendSeparator();
    auto dark = mnuLayer.Append(ID_MNU_DARK, "Dark", "", wxITEM_RADIO);
    auto gray = mnuLayer.Append(ID_MNU_GRAY, "Gray", "", wxITEM_RADIO);
    auto light = mnuLayer.Append(ID_MNU_LIGHT, "Light", "", wxITEM_RADIO);
    
    if (_selectedTheme.type == COLORTHEMETYPE::DARK) {
        dark->Check();
    } else if (_selectedTheme.type == COLORTHEMETYPE::GRAY) {
        gray->Check();
    } else if (_selectedTheme.type == COLORTHEMETYPE::LIGHT) {
        light->Check();
    } else {
        dark->Check();
    }
    
    mnuLayer.AppendSeparator();
    auto fontSmaller = mnuLayer.Append(ID_MNU_FONTSMALLER, "Smaller Font");
    if (_fontSize <= MINFONTSIZE) fontSmaller->Enable(false);
    mnuLayer.Append(ID_MNU_FONTLARGER, "Larger Font");
    mnuLayer.AppendSeparator();
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
    mnuLayer.Append(ID_MNU_ROTATE, "Rotate 90");
    mnuLayer.Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&WiringDialog::OnPopup, nullptr, this);
    PopupMenu(&mnuLayer);
}

void WiringDialog::OnPopup(wxCommandEvent& event)
{
    int id = event.GetId();
    if (id == ID_MNU_RESET)
    {
        _zoom = 1.0f;
        _start = wxRealPoint(0, 0);
        _rotX = -0.4;
        _rotY = 0.6;
        if (_rotated) {
            _rotation = 0;
            _rotated = false;
            _points.clear();
            _points.insert(_originalPoints.begin(), _originalPoints.end());
        }
        Render();
    }
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
    else if (id == ID_MNU_EXPORTDXF)
    {
        Export_DXF();
    }
    else if (id == ID_MNU_PRINT)
    {
        wxPrintData printdata;
        static wxPrintDialogData printDialogData(printdata);
        wxPrinter printer(&printDialogData);

        ColorTheme _prePrintTheme = _selectedTheme;
        SetColorTheme(COLORTHEMETYPE::LIGHT);
        WiringPrintout printout(this);

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
        
        _selectedTheme = _prePrintTheme;
    }
    else if (id == ID_MNU_DARK)
    {
        SetColorTheme(COLORTHEMETYPE::DARK);
        Render();
    }
    else if (id == ID_MNU_GRAY)
    {
        SetColorTheme(COLORTHEMETYPE::GRAY);
        Render();
    }
    else if (id == ID_MNU_LIGHT)
    {
        SetColorTheme(COLORTHEMETYPE::LIGHT);
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
        if (_fontSize > MAXFONTSIZE) _fontSize = MAXFONTSIZE;
        Slider_FontSize->SetValue(_fontSize);
        auto* config = GetXLightsConfig();
        config->Write("xLightsWDFontSize", _fontSize);
        Render();
    }
    else if (id == ID_MNU_FONTSMALLER)
    {
        _fontSize -= FONTSIZEINCREMENT;
        if (_fontSize < MINFONTSIZE) _fontSize = MINFONTSIZE;
        Slider_FontSize->SetValue(_fontSize);
        auto* config = GetXLightsConfig();
        config->Write("xLightsWDFontSize", _fontSize);
        Render();
    }
    else if (id == ID_MNU_ROTATE)
    {
        _rotation += 90;
        if (_rotation == 90 && !_rotated) {
            _originalPoints.insert(_points.begin(), _points.end());
            _rotated = true;
        } else if (_rotation == 360) {
            _rotation = 0;
        }
        RotatePoints(_rotation);
        _zoom = 1.0f;
        _start = wxRealPoint(0, 0);
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
    Layout(); // make sure StaticBitmap_Wiring has been sized for the dialog's current size before we read it

    int w, h;
    StaticBitmap_Wiring->GetClientSize(&w, &h);

    int dw, dh;
    GetClientSize(&dw, &dh);
    wxSize winSize = GetSize();
    spdlog::info("WiringDialog::Render dialogWinSize=({},{}) dialogClientSize=({},{}) staticBitmapClientSize=({},{}) zoom={} start=({},{})",
        winSize.GetWidth(), winSize.GetHeight(), dw, dh, w, h, _zoom, _start.x, _start.y);

    _bmp.CreateScaled(w, h, wxBITMAP_SCREEN_DEPTH, 1.0);  // Using GetContentScaleFactor() was causing it to scale too much on some Windows systems.

    DrawBitmap(_bmp);

    StaticBitmap_Wiring->SetBitmap(_bmp);
    Refresh(false);
}

void WiringDialog::DrawBitmap(wxBitmap& bitmap, bool printer)
{
    wxRealPoint oldStart = _start;
    float oldZoom = _zoom;
    if (printer) {
        _start = wxRealPoint(0, 0);
        _zoom = 1.0f;
    }
    if (_multilight)
    {
        RenderMultiLight(bitmap, _points, _cols, _rows, printer);
    }
    else
    {
        RenderNodes(bitmap, _points, _cols, _rows, printer);
    }
    _start = oldStart;
    _zoom = oldZoom;
}

void WiringDialog::LeftDown(wxMouseEvent& event)
{
    if (!StaticBitmap_Wiring->HasCapture())
    {
        StaticBitmap_Wiring->CaptureMouse();
    }
    _lastMouse = event.GetPosition();
}

void WiringDialog::LeftUp(wxMouseEvent& event)
{
    if (StaticBitmap_Wiring->HasCapture())
    {
        StaticBitmap_Wiring->ReleaseMouse();
    }
}

void WiringDialog::Motion(wxMouseEvent& event)
{
    if (StaticBitmap_Wiring->HasCapture())
    {
        wxPoint delta = event.GetPosition() - _lastMouse;
        if (_is3D) {
            _rotY += delta.x * 0.01;
            _rotX += delta.y * 0.01;
            if (_rotX > 1.5) _rotX = 1.5;
            if (_rotX < -1.5) _rotX = -1.5;
        } else {
            _start = wxRealPoint(_start.x + delta.x, _start.y + delta.y);
        }
        Render();
    }
    _lastMouse = event.GetPosition();
}

void WiringDialog::AdjustZoom(float by, wxPoint mousePos)
{
    float newZoom = _zoom + by;
    if (newZoom < MIN_ZOOM) newZoom = MIN_ZOOM;
    if (newZoom > MAX_ZOOM) newZoom = MAX_ZOOM;
    if (newZoom == _zoom) return;

    // attempt to adjust start so we zoom on where the mouse is

    // mouse real pos under priort zoom
    float mx = ((float)(mousePos.x - _start.x) / _zoom);
    float my = ((float)(mousePos.y - _start.y) / _zoom);

    _zoom = newZoom;

    // work out start which would have that point at the current mouse position with the new zoom value
    float sx = -1 * ((mx * _zoom) - mousePos.x);
    float sy = -1 * ((my * _zoom) - mousePos.y);

    _start = wxRealPoint(sx, sy);
}

void WiringDialog::MouseWheel(wxMouseEvent& event)
{
    if (event.GetWheelRotation() == 0) {
        //rotation of 0 is sometimes generated for other gestures (pinch/zoom), ignore
        return;
    }

    // Raw wheel events on MSW can carry the cursor position in physical
    // screen pixels rather than the DIP client coordinates everything else
    // here uses, which drifts on scaled displays. Query the cursor directly
    // and convert it ourselves instead of trusting event.GetPosition().
    wxPoint mousePos = StaticBitmap_Wiring->ScreenToClient(wxGetMousePosition());

    if (event.GetWheelRotation() > 0) {
        AdjustZoom(0.1f, mousePos);
    }
    else
    {
        AdjustZoom(-0.1f, mousePos);
    }
    Render();
}

void WiringDialog::Magnify(wxMouseEvent& event)
{
    if (event.GetWheelRotation() == 0 || event.GetMagnification() == 0.0f) {
        //magnification of 0 is sometimes generated for other gestures (pinch/zoom), ignore
        return;
    }

    wxPoint mousePos = StaticBitmap_Wiring->ScreenToClient(wxGetMousePosition());

    if (event.GetWheelRotation() > 0) {
        AdjustZoom(0.1f, mousePos);
    }
    else
    {
        AdjustZoom(-0.1f, mousePos);
    }
    Render();
}

void WiringDialog::LeftDClick(wxMouseEvent& event)
{
    _zoom = 1.0;
    _start = wxRealPoint(0, 0);
    Render();
}

void WiringDialog::Export_DXF()
{
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, _modelname + "_wiring", wxEmptyString, "DXF File (*.dxf)|*.dxf", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (filename != "") {
        DXFWriter dxfFile(filename.ToStdString());
        if (!dxfFile.Open()) {
            DisplayError("Unable to create file : " + filename);
            return;
        }
        dxfFile.WriteHeader();

        double minX = 0;
        // draw the lines
        for (auto itp = _points.begin(); itp != _points.end(); ++itp) {
            int last = -10;
            WiringPoint lastpt = WiringPoint(0.0, 0.0);

            for (auto it = itp->second.begin(); it != itp->second.end(); ++it) {
                int x = it->second.front().x;

                int y = it->second.front().y ;

                if (it->first == last + 1) {
                    int lastx = (lastpt.x) ;

                    int lasty = lastpt.y ;
                    dxfFile.WriteWire(CADPoint( _start.x - lastx, _start.y - lasty, 0),
                        CADPoint( _start.x - x,  _start.y - y, 0), 0);
                }

                last = it->first;
                lastpt = it->second.front();
            }
        }

        // now the circles
        for (auto itp = _points.begin(); itp != _points.end(); ++itp) {
            for (auto it = itp->second.begin(); it != itp->second.end(); ++it) {
                int x = it->second.front().x;
                int y = it->second.front().y ;
                dxfFile.WriteNode(CADPoint( _start.x - x, _start.y - y, 0), 0);
                minX = std::min(_start.x - x, minX);
            }
        }

        // render the text after the lines so the text is not drawn over
        int string = 1;
        for (auto itp = _points.begin(); itp != _points.end(); ++itp) {
            for (auto it = itp->second.begin(); it != itp->second.end(); ++it) {
                int x = it->second.front().x;
                int y = it->second.front().y ;

                std::string label;
                if (_points.size() == 1) {
                    label = wxString::Format("%d", it->first).ToStdString();
                } else {
                    label = wxString::Format("%d:%d", string, it->first).ToStdString();
                }

                dxfFile.WriteText(CADPoint( _start.x - x, _start.y - y, 0), label, 0.5 , 0);
            }
            string++;
        }

        dxfFile.WriteText(CADPoint(minX, _start.y + 2  , 0), "Model: " + _modelname, 1, 0);
        dxfFile.WriteText(CADPoint(minX, _start.y, 0), "CAUTION: Rear view", 1, 0);

        dxfFile.WriteEndOfFile();
    }
}
