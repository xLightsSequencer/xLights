#include "ColoursPanel.h"
#include "UtilFunctions.h"
#include "ColorCurve.h"
#include "xLightsMain.h"
#include "DragColoursBitmapButton.h"
#include "ColorPanel.h"

//(*InternalHeaders(ColoursPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/artprov.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>

#include <log4cpp/Category.hh>

//(*IdInit(ColoursPanel)
const long ColoursPanel::ID_SCROLLEDWINDOW1 = wxNewId();
const long ColoursPanel::ID_PANEL1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(ColoursPanel,wxPanel)
	//(*EventTable(ColoursPanel)
	//*)
END_EVENT_TABLE()

#define ITEMSIZE 33

int ColoursPanel::UpdateButtons()
{
    int added = 0;
    auto existing = ScrolledWindow1->GetChildren();
    for (const auto& c : _colours)
    {
        // dont add more than 200 colours
        if (added < 200)
        {
            bool found = false;
            for (const auto& it : existing)
            {
                //ScrolledWindow1 may have scroll bar children as well as buttons
                DragColoursBitmapButton* button = dynamic_cast<DragColoursBitmapButton*>(it);
                if (button && button->GetColour() == c)
                {
                    // already there
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                wxString iid = wxString::Format("ID_BITMAPBUTTON_%d", (int)GridSizer1->GetItemCount());
                DragColoursBitmapButton* bmb = new DragColoursBitmapButton(ScrolledWindow1, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize(30, 30),
                    wxBU_AUTODRAW | wxNO_BORDER, wxDefaultValidator, iid);
                bmb->SetColour(c);
                GridSizer1->Add(bmb);
                added++;
            }
        }
    }

    SendSizeEvent();

    return added;
}

void ColoursPanel::ProcessColourCurveDir(wxDir& directory, bool subdirs)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("ColoursPanel Scanning directory for *.xcc files: %s.", (const char*)directory.GetNameWithSep().c_str());

    int count = 0;

    wxString filename;
    bool cont = directory.GetFirst(&filename, "*.xcc", wxDIR_FILES);

    while (cont)
    {
        count++;
        wxFileName fn(directory.GetNameWithSep() + filename);
        if (fn.Exists())
        {
            ColorCurve cc;
            cc.LoadXCC(fn.GetFullPath());
            cc.SetId("ID_BUTTON_PaletteX");
            AddColour(cc.Serialise());
        }
        else
        {
            logger_base.warn("ColoursPanel::ProcessColourCurveDir Unable to load " + fn.GetFullPath());
        }

        cont = directory.GetNext(&filename);
    }
    logger_base.info("    Found %d.", count);

    if (subdirs)
    {
        cont = directory.GetFirst(&filename, "*", wxDIR_DIRS);
        while (cont)
        {
            wxDir dir(directory.GetNameWithSep() + filename);
            ProcessColourCurveDir(dir, subdirs);
            cont = directory.GetNext(&filename);
        }
    }
}

void ColoursPanel::ProcessPaletteDir(wxDir& directory, bool subdirs)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("ColoursPanel Scanning directory for *.xpalette files: %s.", (const char*)directory.GetNameWithSep().c_str());

    int count = 0;

    wxString filename;
    bool cont = directory.GetFirst(&filename, "*.xpalette", wxDIR_FILES);

    while (cont)
    {
        count++;
        wxFileName fn(directory.GetNameWithSep() + filename);

        wxFile f;
        if (f.Open(fn.GetFullPath()))
        {
            wxString p;
            f.ReadAll(&p);
            for (const auto& it : wxSplit(p, ','))
            {
                if (it != "") AddColour(it);
            }
        }
        else
        {
            logger_base.warn("ColoursPanel::ProcessPaletteDir Unable to load " + fn.GetFullPath());
        }

        cont = directory.GetNext(&filename);
    }
    logger_base.info("    Found %d.", count);

    if (subdirs)
    {
        cont = directory.GetFirst(&filename, "*", wxDIR_DIRS);
        while (cont)
        {
            wxDir dir(directory.GetNameWithSep() + filename);
            ProcessColourCurveDir(dir, subdirs);
            cont = directory.GetNext(&filename);
        }
    }
}

std::string ColoursPanel::GetPaletteFolder(const std::string& showFolder)
{
    std::string pf = showFolder + "/palettes";
    if (!wxDir::Exists(pf))
    {
        wxMkdir(pf);
        if (!wxDir::Exists(pf))
        {
            return "";
        }
    }
    return pf;
}

void ColoursPanel::ParsePalette(const std::string& pal)
{
    wxArrayString cs = wxSplit(pal, ',');
    for (const auto& it : cs)
    {
        if (it.StartsWith("C_BUTTON_Palette"))
        {
            auto v = it.AfterFirst('=');
            if (v != "") AddColour(v);
        }
    }
}

void ColoursPanel::UpdateColourButtons(bool reload, xLightsFrame* xlights) {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (reload)
    {
        _colours.clear();
        _colourscmp.clear();

        auto existing = ScrolledWindow1->GetChildren();
        for (const auto& it : existing) {
            DragColoursBitmapButton* button = dynamic_cast<DragColoursBitmapButton*>(it);
            if (button) {
                delete button;
            }
        }
    }

    AddBaseColours();

    wxDir dir;
    if (wxDir::Exists(xLightsFrame::CurrentDir))
    {
        dir.Open(xLightsFrame::CurrentDir);
        ProcessColourCurveDir(dir, false);
        ProcessPaletteDir(dir, false);
    }

    wxString d = ColorCurve::GetColorCurveFolder(xLightsFrame::CurrentDir.ToStdString());
    if (wxDir::Exists(d))
    {
        dir.Open(d);
        ProcessColourCurveDir(dir, true);
    }
    else
    {
        logger_base.info("Directory for *.xcc files not found: %s.", (const char*)d.c_str());
    }

    d = GetPaletteFolder(xLightsFrame::CurrentDir.ToStdString());
    if (wxDir::Exists(d))
    {
        dir.Open(d);
        ProcessPaletteDir(dir, true);
    }
    else
    {
        logger_base.info("Directory for *.xpalette files not found: %s.", (const char*)d.c_str());
    }

    wxStandardPaths stdp = wxStandardPaths::Get();

#ifndef __WXMSW__
    d = wxStandardPaths::Get().GetResourcesDir() + "/colorcurves";
#else
    d = wxFileName(stdp.GetExecutablePath()).GetPath() + "/colorcurves";
#endif
    if (wxDir::Exists(d))
    {
        dir.Open(d);
        ProcessColourCurveDir(dir, true);
    }
    else
    {
        logger_base.info("Directory for *.xcc files not found: %s.", (const char*)d.c_str());
    }

#ifndef __WXMSW__
    d = wxStandardPaths::Get().GetResourcesDir() + "/palettes";
#else
    d = wxFileName(stdp.GetExecutablePath()).GetPath() + "/palettes";
#endif
    if (wxDir::Exists(d))
    {
        dir.Open(d);
        ProcessPaletteDir(dir, true);
    }
    else
    {
        logger_base.info("Directory for *.xpalette files not found: %s.", (const char*)d.c_str());
    }

    if (xlights != nullptr)
    {
        if (xlights->CurrentSeqXmlFile != nullptr)
        {
            for (auto n = xlights->CurrentSeqXmlFile->GetPalettesNode()->GetChildren(); n != nullptr; n = n->GetNext())
            {
                if (n->GetName() == "ColorPalette" && n->GetChildren() != nullptr)
                {
                    ParsePalette(n->GetChildren()->GetContent());
                }
            }
        }

        if (xlights->GetColorPanel() != nullptr)
        {
            ParsePalette(xlights->GetColorPanel()->GetColorString(true));
        }
    }

    int added = UpdateButtons();

    if (added != 0 && xlights != nullptr)
    {
        wxCommandEvent e(EVT_COLOUR_CHANGED);
        e.SetInt(added);
        wxPostEvent(xlights, e);
    }
}

int ColoursPanel::AddColour(const std::string& colour)
{
    if (colour == "") return 0;

    std::string c(colour);

    if (Contains(c, "ID_BUTTON_Palette"))
    {
        int loc = c.find("ID_BUTTON_Palette");
        c = c.substr(0, loc) + c.substr(loc + 18);
    }

    std::string lc = ::Lower(c);

    if (std::find(_colourscmp.begin(), _colourscmp.end(), lc) != _colourscmp.end())
    {
        // already there
        return 0;
    }
    _colours.push_back(c);
    _colourscmp.push_back(lc);
    return 1;
}

int ColoursPanel::AddBaseColours()
{
    int added = 0;

    added += AddColour(wxBLACK->GetAsString(wxC2S_HTML_SYNTAX));
    for (int i = 31; i < 256; i += 32)
    {
        AddColour(wxColor(i, i, i).GetAsString(wxC2S_HTML_SYNTAX));
    }
    added += AddColour(wxRED->GetAsString(wxC2S_HTML_SYNTAX));
    added += AddColour(wxGREEN->GetAsString(wxC2S_HTML_SYNTAX));
    added += AddColour(wxBLUE->GetAsString(wxC2S_HTML_SYNTAX));
    added += AddColour(wxYELLOW->GetAsString(wxC2S_HTML_SYNTAX));
    added += AddColour(wxCYAN->GetAsString(wxC2S_HTML_SYNTAX));
    added += AddColour(wxColour(255, 0, 255).GetAsString(wxC2S_HTML_SYNTAX));
    return added;
}

ColoursPanel::ColoursPanel(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(ColoursPanel)
	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(1, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	Panel_Sizer = new wxPanel(this, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	FlexGridSizer2->AddGrowableRow(0);
	ScrolledWindow1 = new wxScrolledWindow(Panel_Sizer, ID_SCROLLEDWINDOW1, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL, _T("ID_SCROLLEDWINDOW1"));
	GridSizer1 = new wxGridSizer(0, 3, 0, 0);
	ScrolledWindow1->SetSizer(GridSizer1);
	GridSizer1->Fit(ScrolledWindow1);
	GridSizer1->SetSizeHints(ScrolledWindow1);
	FlexGridSizer2->Add(ScrolledWindow1, 1, wxALL|wxEXPAND, 5);
	Panel_Sizer->SetSizer(FlexGridSizer2);
	FlexGridSizer2->Fit(Panel_Sizer);
	FlexGridSizer2->SetSizeHints(Panel_Sizer);
	FlexGridSizer1->Add(Panel_Sizer, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(wxEVT_SIZE,(wxObjectEventFunction)&ColoursPanel::OnResize);
	//*)

    SetMinSize(wxSize(100, 100));

    ScrolledWindow1->SetScrollRate(0, 5);
    GridSizer1->SetCols(10);
    
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    UpdateColourButtons(false, nullptr);
}

ColoursPanel::~ColoursPanel()
{
	//(*Destroy(ColoursPanel)
	//*)
}

void ColoursPanel::OnResize(wxSizeEvent& event) {

    int cnt = GridSizer1->GetItemCount();

    if (cnt < 1) cnt = 1;

    wxSize wsz = GetSize();
    if (wsz.GetWidth() <= 50) {
        return;
    }

    Panel_Sizer->SetSize(wsz);
    Panel_Sizer->SetMinSize(wsz);
    Panel_Sizer->SetMaxSize(wsz);
    Panel_Sizer->Refresh();

    int cols = (wsz.GetWidth()-20) / ITEMSIZE;
    if (cols < 1) cols = 1;
    GridSizer1->SetCols(cols);
    int rows = cnt / cols + 1;

    ScrolledWindow1->SetSize(wsz);
    ScrolledWindow1->SetMinSize(wsz);
    ScrolledWindow1->SetMaxSize(wsz);
    ScrolledWindow1->FitInside();
    ScrolledWindow1->SetScrollRate(0, 5);
    ScrolledWindow1->Refresh();
    GridSizer1->SetDimension(0, 0, wsz.GetWidth() - 20, (ITEMSIZE + 5) * rows);
    Layout();
}
