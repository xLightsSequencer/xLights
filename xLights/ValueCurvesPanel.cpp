#include "ValueCurvesPanel.h"
#include "UtilFunctions.h"
#include "ValueCurve.h"
#include "ValueCurveButton.h"
#include "xLightsApp.h"
#include "xLightsMain.h"

//(*InternalHeaders(ValueCurvesPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/artprov.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <log4cpp/Category.hh>

//(*IdInit(ValueCurvesPanel)
const long ValueCurvesPanel::ID_SCROLLEDWINDOW1 = wxNewId();
const long ValueCurvesPanel::ID_PANEL1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(ValueCurvesPanel,wxPanel)
	//(*EventTable(ValueCurvesPanel)
	//*)
END_EVENT_TABLE()

int ValueCurvesPanel::ProcessPresetDir(wxDir& directory, bool subdirs)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("Scanning directory for *.xvc files: %s.", (const char*)directory.GetNameWithSep().c_str());

    int added = 0;

    wxString filename;
    auto existing = GridSizer1->GetChildren();

    bool cont = directory.GetFirst(&filename, "*.xvc", wxDIR_FILES);

    while (cont)
    {
        wxFileName fn(directory.GetNameWithSep() + filename);
        bool found = false;
        for (const auto& it : existing)
        {
            if (it->GetWindow()->GetLabel() == fn.GetFullPath())
            {
                // already there
                found = true;
                break;
            }
        }
        if (!found)
        {
            ValueCurve vc("");
            vc.LoadXVC(fn);
            if (vc.IsOk())
            {
                long id = wxNewId();

                wxString iid = wxString::Format("ID_BITMAPBUTTON_%d", (int)GridSizer1->GetItemCount());
                DragValueCurveBitmapButton* bmb = new DragValueCurveBitmapButton(ScrolledWindow1, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize(30, 30),
                    wxBU_AUTODRAW | wxNO_BORDER, wxDefaultValidator, iid);
                bmb->SetLabel(fn.GetFullPath());
                bmb->SetToolTip(fn.GetFullPath());
                bmb->SetValueCurve(vc.Serialise());
                GridSizer1->Add(bmb);
                added++;
            }
            else
            {
                logger_base.warn("ValueCurvesPanel::ProcessPresetDir Unable to load " + fn.GetFullPath());
            }
        }

        cont = directory.GetNext(&filename);
    }

    if (subdirs)
    {
        cont = directory.GetFirst(&filename, "*", wxDIR_DIRS);
        while (cont)
        {
            wxDir dir(directory.GetNameWithSep() + filename);
            added += ProcessPresetDir(dir, subdirs);
            cont = directory.GetNext(&filename);
        }
    }

    return added;
}

void ValueCurvesPanel::UpdateValueCurveButtons(bool reload) {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (reload)
    {
        auto existing = ScrolledWindow1->GetChildren();
        for (const auto& it : existing)
        {
            GridSizer1->Detach(it);
        }
        ScrolledWindow1->DestroyChildren();
        wxASSERT(ScrolledWindow1->GetChildren().size() == 0);
    }

    int added = 0;

    auto existing = GridSizer1->GetChildren();
    bool found = false;
    for (const auto& it : existing)
    {
        if (it->GetWindow()->GetLabel() == "VALUECURVE_CLEAR")
        {
            // already there
            found = true;
            break;
        }
    }

    if (!found)
    {
        DragValueCurveBitmapButton* bmb = new DragValueCurveBitmapButton(ScrolledWindow1, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize(30, 30),
            wxBU_AUTODRAW | wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_0"));
        bmb->SetLabel("VALUECURVE_CLEAR");
        bmb->UnsetToolTip();
        bmb->SetValueCurve("VALUECURVE_CLEAR");
        bmb->SetBitmap(wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_colorpanel_delete_xpm"))));
        added++;
    }

    wxDir dir;

    if (wxDir::Exists(xLightsFrame::CurrentDir))
    {
        dir.Open(xLightsFrame::CurrentDir);
        added += ProcessPresetDir(dir, false);
    }

    wxString d = ValueCurve::GetValueCurveFolder(xLightsFrame::CurrentDir.ToStdString());
    if (wxDir::Exists(d))
    {
        dir.Open(d);
        added += ProcessPresetDir(dir, true);
    }
    else
    {
        logger_base.info("Directory for *.xvc files not found: %s.", (const char*)d.c_str());
    }

    wxStandardPaths stdp = wxStandardPaths::Get();

#ifndef __WXMSW__
    d = wxStandardPaths::Get().GetResourcesDir() + "/valuecurves";
#else
    d = wxFileName(stdp.GetExecutablePath()).GetPath() + "/valuecurves";
#endif
    if (wxDir::Exists(d))
    {
        dir.Open(d);
        added += ProcessPresetDir(dir, true);
    }
    else
    {
        logger_base.info("Directory for *.xvc files not found: %s.", (const char*)d.c_str());
    }

    if (added != 0 && xLightsApp::GetFrame() != nullptr)
    {
        wxCommandEvent e(EVT_VC_CHANGED);
        e.SetInt(added);
        wxPostEvent(xLightsApp::GetFrame(), e);
    }

    wxSizeEvent evt;
    OnResize(evt);
}

ValueCurvesPanel::ValueCurvesPanel(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(ValueCurvesPanel)
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

	Connect(wxEVT_SIZE,(wxObjectEventFunction)&ValueCurvesPanel::OnResize);
	//*)

    SetMinSize(wxSize(100, 100));

    ScrolledWindow1->SetScrollRate(0, 5);
    GridSizer1->SetCols(10);

    UpdateValueCurveButtons(false);
    
    wxSizeEvent evt;
    OnResize(evt);

    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);
}

ValueCurvesPanel::~ValueCurvesPanel()
{
	//(*Destroy(ValueCurvesPanel)
	//*)
}

void ValueCurvesPanel::OnResize(wxSizeEvent& event) {
    int cnt = GridSizer1->GetItemCount();
    if (cnt < 1) cnt = 1;

    wxSize wsz = GetSize();
    if (wsz.GetWidth() <= 10) {
        return;
    }

    Panel_Sizer->SetSize(wsz);
    Panel_Sizer->SetMinSize(wsz);
    Panel_Sizer->SetMaxSize(wsz);
    Panel_Sizer->Refresh();

    int itemsize = 33;
    int cols = (wsz.GetWidth()-20) / itemsize;
    if (cols == 0) cols = 1;
    GridSizer1->SetCols(cols);
    int rows = cnt / cols + 1;
    GridSizer1->SetDimension(0, 0, wsz.GetWidth() - 20, (itemsize + 5) * rows);
    GridSizer1->Layout();

    ScrolledWindow1->SetSize(wsz);
    ScrolledWindow1->SetMinSize(wsz);
    ScrolledWindow1->SetMaxSize(wsz);
    ScrolledWindow1->FitInside();
    ScrolledWindow1->SetScrollRate(0, 5);
    ScrolledWindow1->Refresh();
}