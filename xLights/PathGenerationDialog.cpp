/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "PathGenerationDialog.h"

//(*InternalHeaders(PathGenerationDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/dcbuffer.h>
#include <wx/msgdlg.h>
#include <wx/log.h>

#include "ValueCurve.h"
#include "ExternalHooks.h"

#include <log4cpp/Category.hh>

//(*IdInit(PathGenerationDialog)
const long PathGenerationDialog::ID_PANEL1 = wxNewId();
const long PathGenerationDialog::ID_FILEPICKERCTRL1 = wxNewId();
const long PathGenerationDialog::ID_SLIDER1 = wxNewId();
const long PathGenerationDialog::ID_BUTTON4 = wxNewId();
const long PathGenerationDialog::ID_BUTTON5 = wxNewId();
const long PathGenerationDialog::ID_BUTTON6 = wxNewId();
const long PathGenerationDialog::ID_BUTTON3 = wxNewId();
const long PathGenerationDialog::ID_BUTTON1 = wxNewId();
const long PathGenerationDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PathGenerationDialog,wxDialog)
	//(*EventTable(PathGenerationDialog)
	//*)
END_EVENT_TABLE()

PathGenerationDialog::PathGenerationDialog(wxWindow* parent, const std::string& showFolder, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _showFolder = showFolder;
    _points.push_back({ 0.3, 0.3 });
    _points.push_back({ 0.6, 0.6 });
	//(*Initialize(PathGenerationDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer4;

	Create(parent, id, _("2D Path Generator"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	Panel1 = new wxPanel(this, ID_PANEL1, wxDefaultPosition, wxSize(400,300), wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	FlexGridSizer1->Add(Panel1, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	FilePickerCtrl1 = new wxFilePickerCtrl(this, ID_FILEPICKERCTRL1, wxEmptyString, _("Select an image file"), _T("*.jpg;*.gif;*.png;*.bmp;*.jpeg;*.webp"), wxDefaultPosition, wxDefaultSize, wxFLP_FILE_MUST_EXIST|wxFLP_OPEN|wxFLP_USE_TEXTCTRL, wxDefaultValidator, _T("ID_FILEPICKERCTRL1"));
	FlexGridSizer3->Add(FilePickerCtrl1, 1, wxALL|wxEXPAND, 5);
	Slider_Brightness = new wxSlider(this, ID_SLIDER1, 100, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER1"));
	FlexGridSizer3->Add(Slider_Brightness, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer4 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_FlipX = new wxButton(this, ID_BUTTON4, _("Flip X"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
	FlexGridSizer4->Add(Button_FlipX, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_FlipY = new wxButton(this, ID_BUTTON5, _("Flip Y"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON5"));
	FlexGridSizer4->Add(Button_FlipY, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Rotate = new wxButton(this, ID_BUTTON6, _("Rotate"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON6"));
	FlexGridSizer4->Add(Button_Rotate, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_Load = new wxButton(this, ID_BUTTON3, _("Load"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	FlexGridSizer2->Add(Button_Load, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Generate = new wxButton(this, ID_BUTTON1, _("Generate"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer2->Add(Button_Generate, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Close = new wxButton(this, ID_BUTTON2, _("Close"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer2->Add(Button_Close, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Panel1->Connect(wxEVT_PAINT,(wxObjectEventFunction)&PathGenerationDialog::OnPanel1Paint,0,this);
	Panel1->Connect(wxEVT_KEY_DOWN,(wxObjectEventFunction)&PathGenerationDialog::OnPanel1KeyDown,0,this);
	Panel1->Connect(wxEVT_LEFT_DOWN,(wxObjectEventFunction)&PathGenerationDialog::OnPanel1LeftDown,0,this);
	Panel1->Connect(wxEVT_LEFT_UP,(wxObjectEventFunction)&PathGenerationDialog::OnPanel1LeftUp1,0,this);
	Panel1->Connect(wxEVT_LEFT_DCLICK,(wxObjectEventFunction)&PathGenerationDialog::OnPanel1LeftDClick,0,this);
	Panel1->Connect(wxEVT_RIGHT_DOWN,(wxObjectEventFunction)&PathGenerationDialog::OnPanel1RightDown,0,this);
	Panel1->Connect(wxEVT_MOTION,(wxObjectEventFunction)&PathGenerationDialog::OnPanel1MouseMove,0,this);
	Panel1->Connect(wxEVT_SIZE,(wxObjectEventFunction)&PathGenerationDialog::OnPanel1Resize,0,this);
	Connect(ID_FILEPICKERCTRL1,wxEVT_COMMAND_FILEPICKER_CHANGED,(wxObjectEventFunction)&PathGenerationDialog::OnFilePickerCtrl1FileChanged);
	Connect(ID_SLIDER1,wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&PathGenerationDialog::OnSlider_BrightnessCmdScrollChanged);
	Connect(ID_SLIDER1,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&PathGenerationDialog::OnSlider_BrightnessCmdSliderUpdated);
	Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PathGenerationDialog::OnButton_FlipXClick);
	Connect(ID_BUTTON5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PathGenerationDialog::OnButton_FlipYClick);
	Connect(ID_BUTTON6,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PathGenerationDialog::OnButton_RotateClick);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PathGenerationDialog::OnButton_LoadClick);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PathGenerationDialog::OnButton_GenerateClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PathGenerationDialog::OnButton_CloseClick);
	//*)

    Slider_Brightness->SetValue(20); // start dim by default
    RegenerateImage();
    Panel1->SetBackgroundStyle(wxBG_STYLE_PAINT);
}

PathGenerationDialog::~PathGenerationDialog()
{
	//(*Destroy(PathGenerationDialog)
	//*)
}

void PathGenerationDialog::OnButton_LoadClick(wxCommandEvent& event)
{
    wxLogNull logNo;

    ValueCurve vc1("");
    ValueCurve vc2("");
    vc1.SetActive(true);
    vc2.SetActive(true);
    vc1.SetId("Dummy");
    vc2.SetId("Dummy");
    vc1.SetType("Custom");
    vc2.SetType("Custom");

    std::string vcf = ValueCurve::GetValueCurveFolder(_showFolder);

    wxString filename = wxFileSelector(_("Choose value curve file"), vcf, wxEmptyString, wxEmptyString, "Value Curve files (*.xvc)|*.xvc", wxFD_OPEN);
    if (filename.IsEmpty()) return;

    _points.clear();
    _selected = -1;
    _movefrom = {-1, -1};
    _undo.clear();

    wxFileName fn(filename);
    if (fn.GetName().Last() == 'Y')
    {
        vc2.LoadXVC(fn);
        wxFileName fn2 = fn;
        fn2.SetName(fn.GetName().Left(fn.GetName().Length() - 1) + "X");
        if (FileExists(fn2))
        {
            vc1.LoadXVC(fn2);
        }
    }
    else if (fn.GetName().Last() == 'X')
    {
        vc1.LoadXVC(fn);
        wxFileName fn2 = fn;
        fn2.SetName(fn.GetName().Left(fn.GetName().Length() - 1) + "Y");
        if (FileExists(fn2)) {
            vc2.LoadXVC(fn2);
        }
    }
    else
    {
        vc1.LoadXVC(fn);
    }

    auto p1 = vc1.GetPoints();
    auto p2 = vc2.GetPoints();

    auto itx = p1.begin();
    auto ity = p2.begin();

    while (itx != p1.end() || ity != p2.end())
    {
        if (itx != p1.end() && ity != p2.end())
        {
            if (itx->x == ity->x)
            {
                _points.emplace_back(std::pair<float, float>(itx->y, ity->y));
                ++itx;
                ++ity;
            }
            else if (itx->x < ity->x)
            {
                _points.emplace_back(std::pair<float,float>(itx->y, 0.0f));
                ++itx;
            }
            else
            {
                _points.emplace_back(std::pair<float, float>(0.0f, ity->y));
                ++ity;
            }
        }
        else if (itx != p1.end())
        {
            _points.emplace_back(std::pair<float, float>(itx->y, 0.0f));
            ++itx;
        }
        else if (ity != p2.end())
        {
            _points.emplace_back(std::pair<float, float>(0.0f, ity->y));
            ++ity;
        }
        else
        {
            wxASSERT(false);
        }
    }
    unsaved = false;
    Panel1->Refresh(true);
}

float PathGenerationDialog::TotalLength() const
{
    float len = 0.0;
    auto last = _points.begin();
    bool first = true;

    for (auto pt = begin(_points); pt != end(_points); ++pt)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            len += GetDistance(*last, *pt);
        }
        last = pt;
    }
    return len;
}

float PathGenerationDialog::GetDistance(const std::pair<float, float>& pt1, const std::pair<float, float>& pt2)
{
    return sqrt((pt2.first - pt1.first) * (pt2.first - pt1.first) + (pt2.second - pt1.second) * (pt2.second - pt1.second));
}

void PathGenerationDialog::OnButton_GenerateClick(wxCommandEvent& event)
{
    wxLogNull logNo;
    std::string vcf = ValueCurve::GetValueCurveFolder(_showFolder);

    wxString filename = wxFileSelector(_("Value curve file to save"), vcf, wxEmptyString, wxEmptyString, "Value Curve files (*.xvc)|*.xvc", wxFD_SAVE);
    if (filename.IsEmpty()) return;

    // if it ends in x or y ... remove it
    wxFileName fn(filename);
    if (fn.GetName().Lower().EndsWith("x") || fn.GetName().Lower().EndsWith("y"))
    {
        fn.SetName(fn.GetName().Left(fn.GetName().Length() - 1));
        filename = fn.GetFullPath();
    }

    wxFileName fn1(filename);
    fn1.SetName(fn1.GetName() + "X");
    wxFileName fn2(filename);
    fn2.SetName(fn2.GetName() + "Y");

    if (FileExists(fn1.GetFullPath()) || FileExists(fn2.GetFullPath()))
    {
        if (wxMessageBox("Overwrite existing curves?", "Overwite?", wxYES_NO, this) == wxNO) return;
    }

    ValueCurve vc1("");
    ValueCurve vc2("");
    vc1.SetActive(true);
    vc2.SetActive(true);
    vc1.SetType("Custom");
    vc2.SetType("Custom");

    float totallen = TotalLength();
    auto last = _points.begin();
    bool first = true;
    float x = 0.0;
    for (auto pt = begin(_points); pt != end(_points); ++pt)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            float thislen = GetDistance(*last, *pt);
            x += thislen / totallen;
            wxASSERT(x <= 1.001);
            if (x > 1.0) x = 1.0;
        }
        vc1.SetValueAt(x, pt->first);
        vc2.SetValueAt(x, pt->second);
        last = pt;
    }
    wxASSERT(x >= 0.999 && x <= 1.0);

    vc1.SaveXVC(fn1);
    vc2.SaveXVC(fn2);

    unsaved = false;
}

void PathGenerationDialog::OnButton_CloseClick(wxCommandEvent& event)
{
    if (unsaved)
    {
        if (wxMessageBox("Are you sure you want to exit?", "You have not saved your curve. Are you sure you want to exit?", wxYES_NO, this) == wxNO)
        {
            return;
        }
    }
    EndDialog(wxID_CLOSE);
}

#define BORDER 5
wxPoint PathGenerationDialog::CreatePoint(const std::pair<float, float>& pt) const
{
    auto size = Panel1->GetSize();
    return wxPoint(pt.first * (size.GetWidth() - 2 * BORDER) + BORDER,
                   size.GetHeight() - BORDER - pt.second * (size.GetHeight() - 2 * BORDER));
}

void PathGenerationDialog::OnPanel1Paint(wxPaintEvent& event)
{
    wxAutoBufferedPaintDC pdc(Panel1);

    pdc.SetPen(*wxWHITE_PEN);
    pdc.SetBrush(*wxWHITE_BRUSH);
    pdc.DrawRectangle(0,0,Panel1->GetSize().GetWidth(),Panel1->GetSize().GetHeight());

    pdc.SetPen(*wxLIGHT_GREY_PEN);
    pdc.DrawRectangle(BORDER, BORDER, Panel1->GetSize().GetWidth() - 2 * BORDER,
        Panel1->GetSize().GetHeight() - 2 * BORDER);

    if (_image.get() != nullptr)
    {
        pdc.DrawBitmap(*_image, 0, 0);
    }

    if (_points.size() > 1)
    {
        int l = 0;
        int p = 1;
        auto last = _points.begin();
        for (auto it = std::next(_points.begin()); it != _points.end(); ++it)
        {
            if (l == _selected || p == _selected)
            {
                pdc.SetPen(*wxGREEN_PEN);
            }
            else
            {
                pdc.SetPen(*wxBLACK_PEN);
            }
            pdc.DrawLine(CreatePoint(*last), CreatePoint(*it));
            last = it;
            ++l;
            ++p;
        }
    }

    for (auto pt = _points.begin(); pt != _points.end(); ++pt)
    {
        if (pt == _points.begin())
        {
            pdc.SetPen(*wxCYAN_PEN);
            pdc.SetBrush(*wxCYAN_BRUSH);
        }
        else
        {
            pdc.SetPen(*wxBLUE_PEN);
            pdc.SetBrush(*wxBLUE_BRUSH);
        }
        auto point = CreatePoint(*pt);
        pdc.DrawRectangle(point.x - 2, point.y - 2, 4, 4);
    }

    if (_selected >= 0)
    {
        pdc.SetPen(*wxRED_PEN);
        pdc.SetBrush(*wxRED_BRUSH);
        auto it = GetPoint(_selected);
        auto point = CreatePoint(*it);
        pdc.DrawRectangle(point.x - 2, point.y - 2, 4, 4);
    }
}

void PathGenerationDialog::OnPanel1KeyDown(wxKeyEvent& event)
{
    if (event.GetKeyCode() == WXK_DELETE)
    {
        unsaved = true;
        if (_selected >= 0)
        {
            auto it = GetPoint(_selected);
            _undo.push_back({ _selected, "delete", *it });
            _points.erase(it);
            Panel1->Refresh(true);
            _selected = -1;
        }
    }
    else if ((event.GetKeyCode() == 'Z' || event.GetKeyCode() == 'z') && (event.ControlDown() || event.CmdDown()))
    {
        unsaved = true;
        if (_undo.size() > 0)
        {
            auto undo = _undo.back();
            //auto[index, action, pos] = undo;
            int index;
            std::string action;
            std::pair<float, float> pos;
            std::tie(index, action, pos) = undo;
            _undo.pop_back();
            _selected = -1;
            if (action == "add")
            {
                auto it = GetPoint(index);
                _points.erase(it);
            }
            else if (action == "delete")
            {
                if (index == 0)
                {
                    _points.push_front(pos);
                }
                else
                {
                    auto it = GetPoint(index - 1);
                    _points.insert(next(it), pos);
                }
            }
            else if (action == "movefrom")
            {
                auto it = GetPoint(index);
                *it = pos;
            }
            Panel1->Refresh(true);
        }
    }
}

#define MOUSECLOSEPOINT 0.005f
int PathGenerationDialog::GetMouseOverPoint(const std::pair<float, float>& pt, int start) const
{
    int pos = start;
    auto it = _points.begin();
    if (start >= 0)
    {
        std::advance(it, start);
        ++it;
        pos++;
    }
    else
    {
        pos = 0;
    }

    while (it != _points.end())
    {
        if (abs(pt.first - it->first) < MOUSECLOSEPOINT && abs(pt.second - it->second) < MOUSECLOSEPOINT)
        {
            return pos;
        }
        ++it;
        pos++;
    }

    if (start > 0)
    {
        pos = 0;
        it = _points.begin();
        while (pos < start)
        {
            if (abs(pt.first - it->first) < MOUSECLOSEPOINT && abs(pt.second - it->second) < MOUSECLOSEPOINT)
            {
                return pos;
            }
            ++it;
            pos++;
        }
    }

    return -1;
}

#define MOUSECLOSELINE 0.005f
int PathGenerationDialog::GetMouseOverLine(const std::pair<float, float>& pt) const
{
    if (_points.size() < 2) return -1;

    int pos = 0;
    auto last = _points.begin();
    for (auto p = next(_points.begin()); p != _points.end(); ++p)
    {
        auto distance = PointDistanceFromLineThrough(pt, *last, *p);
        if (distance < MOUSECLOSELINE)
        {
            return pos;
        }
        last = p;
        pos++;
    }

    return -1;
}

float PathGenerationDialog::PointDistanceFromLineThrough(const std::pair<float, float>& pt, const std::pair<float, float>& lpt1, const std::pair<float, float>& lpt2)
{
    float distance = abs(pt.first*(lpt2.second - lpt1.second) - pt.second*(lpt2.first - lpt1.first) + lpt2.first * lpt1.second - lpt2.second * lpt1.first) / GetDistance(lpt1,lpt2);

    auto minx = std::min(lpt1.first, lpt2.first);
    auto miny = std::min(lpt1.second, lpt2.second);
    auto maxx = std::max(lpt1.first, lpt2.first);
    auto maxy = std::max(lpt1.second, lpt2.second);

    if (pt.first < minx || pt.first > maxx || pt.second < miny || pt.second > maxy)
    {
        // past the end of our points ... make our distance really large
        return 9999.0;
    }

    return distance;
}

std::list<std::pair<float,float>>::iterator PathGenerationDialog::GetPoint(int index)
{
    wxASSERT(index < _points.size());
    std::list<std::pair<float, float>>::iterator it = _points.begin();
    advance(it, index);
    return it;
}

#pragma region Mouse Handling

std::pair<float, float> PathGenerationDialog::ConvertMousePosition(const wxPoint& pt) const
{
    std::pair<float, float> res;
    auto size = Panel1->GetSize();
    res.first = static_cast<float>(pt.x - BORDER) / (size.GetWidth() - 2 * BORDER);
    res.second = static_cast<float>(size.GetHeight() - pt.y - BORDER) / (size.GetHeight() - 2 * BORDER);
    if (res.first < 0) res.first = 0;
    if (res.second < 0) res.second = 0;
    if (res.first > 1) res.first = 1;
    if (res.second > 1) res.second = 1;
    return res;
}

void PathGenerationDialog::OnPanel1LeftDown(wxMouseEvent& event)
{
    unsaved = true;
    auto pt = ConvertMousePosition(event.GetPosition());
    int closept = GetMouseOverPoint(pt, -1);

    // if we are close to an existing point then assume they meant that point
    if (closept >= 0)
    {
        auto it = GetPoint(closept);
        pt = *it;
        if (closept != _selected)
        {
            _selected = -1;
        }
        else
        {
            _movefrom = pt;
        }
    }
    else
    {
        _selected = -1;
    }

    auto overpt = GetMouseOverPoint(pt, _selected);
    if (overpt >= 0)
    {
        _selected = overpt;
        _movefrom = pt;
    }
    Panel1->Refresh(true);
}

void PathGenerationDialog::OnPanel1MouseMove(wxMouseEvent& event)
{
    auto pt = ConvertMousePosition(event.GetPosition());

    // selected so we need to move the selected point
    if (_selected >= 0 && event.m_leftDown)
    {
        if (pt.first >= 0 && pt.first <= 1 && pt.second >= 0 && pt.second <= 1)
        {
            auto closept = GetMouseOverPoint(pt, _selected);

            // magnetic attraction to close nodes
            if (closept >= 0 && closept != _selected)
            {
                auto cp = GetPoint(closept);
                pt.first = cp->first;
                pt.second = cp->second;
            }

            auto it = GetPoint(_selected);
            it->first = pt.first;
            it->second = pt.second;
            Panel1->Refresh(true);
        }
    }

    int overpt = GetMouseOverPoint(pt, -1);
    int overline = GetMouseOverLine(pt);

    if (overpt >= 0)
    {
        if (_selected >= 0 && event.m_leftDown)
        {
            Panel1->SetCursor(wxCURSOR_CLOSED_HAND);
        }
        else
        {
            Panel1->SetCursor(wxCURSOR_OPEN_HAND);
        }
    }
    else if (overline >= 0)
    {
        Panel1->SetCursor(wxCURSOR_BULLSEYE);
    }
    else
    {
        Panel1->SetCursor(wxCURSOR_CROSS);
    }
}

void PathGenerationDialog::OnPanel1RightDown(wxMouseEvent& event)
{
    _selected = -1;
    Panel1->Refresh(true);
}

void PathGenerationDialog::OnPanel1LeftDClick(wxMouseEvent& event)
{
    unsaved = true;
    _selected = -1;
    auto pt = ConvertMousePosition(event.GetPosition());
    auto overline = GetMouseOverLine(pt);
    auto overpoint = GetMouseOverPoint(pt, -1);

    if (overline >= 0 && overpoint < 0)
    {
        auto it = GetPoint(overline);
        _points.insert(next(it), pt);
        _selected = overline + 1;
        _movefrom = pt;
        _undo.push_back({ _selected, "add", pt });
    }
    else
    {
        _points.emplace_back(pt);
        _selected = _points.size() - 1;
        _movefrom = pt;
        _undo.push_back({_selected, "add", pt});
    }
    Panel1->Refresh(true);
}

void PathGenerationDialog::OnPanel1LeftUp1(wxMouseEvent& event)
{
    if (_selected >= 0)
    {
        auto it = GetPoint(_selected);
        if (*it != _movefrom)
        {
            _undo.push_back({ _selected, "movefrom", _movefrom });
        }
    }
}

#pragma endregion Mouse Handling

void PathGenerationDialog::OnPanel1Resize(wxSizeEvent& event)
{
    RegenerateImage();
    Panel1->Refresh(true);
}

void PathGenerationDialog::OnFilePickerCtrl1FileChanged(wxFileDirPickerEvent& event)
{
    RegenerateImage();
}

void PathGenerationDialog::OnSlider_BrightnessCmdScrollChanged(wxScrollEvent& event)
{
    RegenerateImage();
}

void PathGenerationDialog::OnSlider_BrightnessCmdSliderUpdated(wxScrollEvent& event)
{
    RegenerateImage();
}

void PathGenerationDialog::RegenerateImage()
{
    bool success = FileExists(FilePickerCtrl1->GetFileName().GetFullPath());
    if (success)
    {
        wxImage image(FilePickerCtrl1->GetFileName().GetFullPath());
        success = image.IsOk();

        if (success)
        {
            auto size = Panel1->GetSize();
            image.Rescale(size.GetWidth(), size.GetHeight());

            image.InitAlpha();
            int alpha = (Slider_Brightness->GetValue() * 255) / 100;

            for (int x = 0; x < image.GetWidth(); x++)
            {
                for (int y = 0; y < image.GetHeight(); y++)
                {
                    image.SetAlpha(x, y, alpha);
                }
            }

            _image = std::make_unique<wxBitmap>(wxBitmap(image));
            Panel1->Refresh(true);
        }
    }

    if (!success)
    {
        _image = nullptr;
    }
}

void PathGenerationDialog::OnButton_FlipXClick(wxCommandEvent& event)
{
    for (auto& pt : _points)
    {
        pt.first = 1.0 - pt.first;
    }
    Panel1->Refresh(true);
}

void PathGenerationDialog::OnButton_FlipYClick(wxCommandEvent& event)
{
    for (auto& pt : _points)
    {
        pt.second = 1.0 - pt.second;
    }
    Panel1->Refresh(true);
}

void PathGenerationDialog::OnButton_RotateClick(wxCommandEvent& event)
{
    // flip x
    OnButton_FlipXClick(event);
    // swap x and y
    for (auto& pt : _points)
    {
        std::swap(pt.first, pt.second);
    }
}
