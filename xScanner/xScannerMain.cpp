/***************************************************************
 * Name:      xScannerMain.cpp
 * Purpose:   Code for Application Frame
 * Author:    xLights ()
 * Created:   2016-12-30
 * Copyright: xLights (http://xlights.org)
 * License:
 **************************************************************/

#define ZERO 0
#define E131PORT 5568
#define ARTNETPORT 0x1936

#include <wx/wx.h>
#include <wx/msgdlg.h>
#include <wx/config.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/debugrpt.h>
#include <wx/protocol/http.h>
#include <wx/filedlg.h>
#include <wx/numdlg.h>
#include <wx/progdlg.h>
#include <wx/dataview.h>

#include "xScannerMain.h"
#include "Scanner.h"
#include "../xLights/xLightsVersion.h"
#include "../xLights/outputs/ControllerEthernet.h"
#include "../xLights/UtilFunctions.h"
#include "../xLights/controllers/BaseController.h"

#include <log4cpp/Category.hh>

#ifndef __WXMSW__
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include "../include/xLights.xpm"
#include "../include/xLights-16.xpm"
#include "../include/xLights-32.xpm"
#include "../include/xLights-64.xpm"
#include "../include/xLights-128.xpm"

//(*InternalHeaders(xScannerFrame)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//helper functions
enum wxbuildinfoformat {
    short_f, long_f };

wxString wxbuildinfo(wxbuildinfoformat format)
{
    wxString wxbuild(wxVERSION_STRING);

    if (format == long_f )
    {
#if defined(__WXMSW__)
        wxbuild << _T("-Windows");
#elif defined(__UNIX__)
        wxbuild << _T("-Linux");
#endif

#if wxUSE_UNICODE
        wxbuild << _T("-Unicode build");
#else
        wxbuild << _T("-ANSI build");
#endif // wxUSE_UNICODE
    }

    return wxbuild;
}

//(*IdInit(xScannerFrame)
const long xScannerFrame::ID_STATUSBAR1 = wxNewId();
const long xScannerFrame::Network = wxNewId();
//*)

const long xScannerFrame::ID_MNU_EXPORT = wxNewId();

wxDEFINE_EVENT(EVT_SCANPROGRESS, wxCommandEvent);

BEGIN_EVENT_TABLE(xScannerFrame,wxFrame)
    //(*EventTable(xScannerFrame)
    //*)
    EVT_COMMAND(wxID_ANY, EVT_SCANPROGRESS, xScannerFrame::ScanUpdate)
END_EVENT_TABLE()

void xScannerFrame::PurgeCollectedData()
{
}

xScannerFrame::xScannerFrame(wxWindow* parent, bool singleThreaded, wxWindowID id)
{
    // static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    //(*Initialize(xScannerFrame)
    wxFlexGridSizer* FlexGridSizer1;

    Create(parent, id, _("xLights Scanner"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("id"));
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer1->AddGrowableRow(0);
    SetSizer(FlexGridSizer1);
    StatusBar1 = new wxStatusBar(this, ID_STATUSBAR1, 0, _T("ID_STATUSBAR1"));
    int __wxStatusBarWidths_1[1] = { -10 };
    int __wxStatusBarStyles_1[1] = { wxSB_NORMAL };
    StatusBar1->SetFieldsCount(1,__wxStatusBarWidths_1);
    StatusBar1->SetStatusStyles(1,__wxStatusBarStyles_1);
    SetStatusBar(StatusBar1);
    MenuItemScan = new wxMenuItem((&Menu1), Network, _("Scan"), wxEmptyString, wxITEM_NORMAL);
    Menu1.Append(MenuItemScan);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    Connect(Network,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScannerFrame::OnMenuItemScanSelected);
    Connect(wxEVT_SIZE,(wxObjectEventFunction)&xScannerFrame::OnResize);
    //*)

    _tree = new wxTreeListCtrl(this, wxNewId());
    FlexGridSizer1->Add(_tree, 1, wxALL | wxEXPAND, 5);

    _tree->Connect(wxEVT_TREELIST_ITEM_ACTIVATED, (wxObjectEventFunction)&xScannerFrame::OnTreeItemActivated, nullptr, this);
    _tree->Connect(wxEVT_TREELIST_ITEM_CONTEXT_MENU, (wxObjectEventFunction)&xScannerFrame::OnTreeRClick, nullptr, this);

    _tree->AppendColumn("", 200);
    _tree->AppendColumn("", 200);
    _tree->AppendColumn("", 200);
    _tree->AppendColumn("", 200);
    _tree->AppendColumn("", 200);

    SetTitle("xLights Scanner " + GetDisplayVersionString());

    wxIconBundle icons;
    icons.AddIcon(wxIcon(xlights_16_xpm));
    icons.AddIcon(wxIcon(xlights_32_xpm));
    icons.AddIcon(wxIcon(xlights_64_xpm));
    icons.AddIcon(wxIcon(xlights_128_xpm));
    icons.AddIcon(wxIcon(xlights_xpm));
    SetIcons(icons);

    _scanner.SetSingleThreaded(singleThreaded);

    Scan();

    SetMinSize(wxSize(400, 300));
    SetSize(800, 600);

    ValidateWindow();
}

wxThread::ExitCode ScanThread::Entry()
{
    ((xScannerFrame*)_frame)->DoScan();

    return 0;
}

void xScannerFrame::DoScan()
{
    _scanner.Scan(this);
}

void xScannerFrame::Scan()
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    _tree->Disable();

    logger_base.debug("Launching scan ...");
    _progress = new wxProgressDialog("Scanning ...", "Commencing scan", 100, this);
    _progress->Show();

    // do the single threaded part of the scan
    _scanner.PreScan(this);

    _thread = new ScanThread(this);
    if (_thread->Run() != wxTHREAD_NO_ERROR) {
        logger_base.error("Can't create the thread!");
        delete _thread;
        _thread = nullptr;
    }
}

void xScannerFrame::ScanUpdate(wxCommandEvent& event)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (event.GetInt() == 100) {

        _progress->Update(100);
        _progress = nullptr;
        _thread = nullptr;

        logger_base.debug("Scan done.");
        // we are done
        LoadScanResults();

        _tree->Enable();
    }
    else {
        if (_progress != nullptr) {
            _progress->Update(event.GetInt(), event.GetString());
        }
    }
}

void xScannerFrame::LoadScanResults()
{
    _tree->DeleteAllItems();

    auto comp = _tree->AppendItem(_tree->GetRootItem(), "Computer");
    _tree->SetItemText(comp, 1, _scanner._computer._name);

    if (_scanner._computer._xLightsFolder != "") {
        auto xs = _tree->AppendItem(comp, "xLights Show Folder");
        _tree->SetItemText(xs, 1, _scanner._computer._xLightsFolder);
        if (_scanner._showDir == _scanner._computer._xLightsFolder) {
            _tree->SetItemText(xs, 2, "SCANNED");
        }
    }

    if (_scanner._computer._xScheduleFolder != "") {
        auto xs = _tree->AppendItem(comp, "xSchedule Show Folder");
        if (_scanner._computer._xScheduleFolder == _scanner._computer._xLightsFolder) {
            _tree->SetItemText(xs, 1, "Same as xLights");
        }
        else {
            _tree->SetItemText(xs, 1, _scanner._computer._xScheduleFolder);
            if (_scanner._showDir == _scanner._computer._xScheduleFolder) {
                _tree->SetItemText(xs, 2, "SCANNED");
            }
        }
    }

    for (const auto& it : _scanner._computer._ips) {
        auto ip = _tree->AppendItem(comp, "Local: " + it);

        if (_scanner._xLights._forceIP == it) {
            _tree->SetItemText(ip, 1, "Output forced out this address");
        }

        for (auto& it : _scanner.GetIPsInSameSubnet(it)) {
            auto ipc = _tree->AppendItem(ip, it.GetDisplayIP());
            AddIP(ipc, it);
            _scanner.SetDisplayed(it);
        }
    }

    _tree->Expand(comp);

    for (const auto& it : _scanner._computer._routes) {
        auto ip = _tree->AppendItem(comp, "Route: " + it);
        for (auto& it : _scanner.GetIPsInSameSubnet(it)) {
            auto ipc = _tree->AppendItem(ip, it.GetDisplayIP());
            AddIP(ipc, it);
            _scanner.SetDisplayed(it);
        }
    }

    auto ur = _tree->AppendItem(comp, "Remote Network");
    for (auto& it : _scanner.GetUndisplayedIPs()) {
        auto ipc = _tree->AppendItem(ur, it.GetDisplayIP());
        AddIP(ipc, it);
        _scanner.SetDisplayed(it);
    }
}

void xScannerFrame::AddIP(wxTreeListItem ti, const IPObject& ip)
{
    wxTreeListItem tt;
    if (ip._pinged) {
        tt = _tree->AppendItem(ti, "Ping OK");
    }
    else {
        tt = _tree->AppendItem(ti, "Ping FAILED!");
    }
    if (ip._viaProxy != "") {
        _tree->SetItemText(tt, 1, "Via proxy " + ip._viaProxy);
    }
    if (ip._port80) {
        _tree->AppendItem(ti, "Web Interface Available");
    }

    if (ip._type != "")         {
        auto t = ip._type;
        if (ip._mode != "") t += " (" + ip._mode + ")";
        _tree->SetItemText(ti, 3, t);
    }

    if (ip._xLightsController != nullptr) {
        _tree->SetItemText(ti, 2, "xLights Controller Tab");
        auto item = _tree->AppendItem(ti, "Name");
        if (ip._xLightsController->GetName() != "") {
            _tree->SetItemText(item, 1, ip._xLightsController->GetName());
        }
        else if (ip._name != "") {
            _tree->SetItemText(item, 1, ip._name);
        }
        item = _tree->AppendItem(ti, "Protocol");
        _tree->SetItemText(item, 1, ip._xLightsController->GetColumn1Label());
        item = _tree->AppendItem(ti, "Universes/Id");
        _tree->SetItemText(item, 1, ip._xLightsController->GetColumn3Label());
        item = _tree->AppendItem(ti, "Channels");
        _tree->SetItemText(item, 1, ip._xLightsController->GetColumn4Label());
        item = _tree->AppendItem(ti, "Vendor/Model/Variant");
        if (ip._xLightsController->GetVMV() != "") {
            _tree->SetItemText(item, 1, ip._xLightsController->GetVMV());
        }
        else if (ip._discovered != nullptr) {
            _tree->SetItemText(item, 1, ip._discovered->GetVMV());
        }
    }
    else if (ip._discovered != nullptr) {
        _tree->SetItemText(ti, 2, "Discovered");
        auto item = _tree->AppendItem(ti, "Name");
        _tree->SetItemText(item, 1, ip._discovered->GetName());

        if (ip._discovered->GetVMV() != "") {
            item = _tree->AppendItem(ti, "Vendor/Model/Variant");
            _tree->SetItemText(item, 1, ip._discovered->GetVMV());
        }
    }

    if (ip._mode != "")         {
        auto item = _tree->AppendItem(ti, "FPP Mode");
        _tree->SetItemText(item, 1, ip._mode);
    }

    if (ip._version != "") {
        auto item = _tree->AppendItem(ti, "Version");
        _tree->SetItemText(item, 1, ip._version);
    }

    if (ip._xSchedulePort != 0) {
        auto item = _tree->AppendItem(ti, "Web Port");
        _tree->SetItemText(item, 1, wxString::Format("%d", ip._xSchedulePort));
    }

    if (ip._banks != "") {
        auto item = _tree->AppendItem(ti, "Bank Sizes");
        _tree->SetItemText(item, 1, ip._banks);
    }

    if (ip._emittingData != "") {
        auto item = _tree->AppendItem(ti, "Sending data to");
        _tree->SetItemText(item, 1, ip._emittingData);
    }

    for (const auto& it3 : ip._otherIPs)         {
        auto item = _tree->AppendItem(ti, "Network Interface");
        _tree->SetItemText(item, 1, it3);

    }

    for (const auto& it3 : ip._otherData) {
        auto item = _tree->AppendItem(ti, it3.first);
        _tree->SetItemText(item, 1, it3.second);
    }

    if (ip._mac != "") {
        auto item = _tree->AppendItem(ti, "MAC");
        _tree->SetItemText(item, 1, ip._mac);
        _tree->SetItemText(item, 2, ip._macVendor);
    }

    for (auto& it : _scanner.GetProxiedBy(ip._ip)) {
        auto ipc = _tree->AppendItem(ti, it._ip);
        AddIP(ipc, it);
        it.Displayed();
    }
}

xScannerFrame::~xScannerFrame()
{
    PurgeCollectedData();

    //(*Destroy(xScannerFrame)
    //*)
}

void xScannerFrame::OnQuit(wxCommandEvent& event)
{
    Close();
}

void xScannerFrame::OnAbout(wxCommandEvent& event)
{
    auto about = wxString::Format(wxT("xScanner v%s, the xLights configuration scanner."), GetDisplayVersionString());
    wxMessageBox(about, _("Welcome to..."));
}

void xScannerFrame::ValidateWindow()
{
}

void xScannerFrame::OnResize(wxSizeEvent& event)
{
    Layout();
}

void xScannerFrame::CreateDebugReport(wxDebugReportCompress *report) {
    if (wxDebugReportPreviewStd().Show(*report)) {
        report->Process();
        SendReport("crashUpload", *report);
        wxMessageBox("Crash report saved to " + report->GetCompressedFileName());
    }
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.crit("Exiting after creating debug report: " + report->GetCompressedFileName());
    delete report;
    exit(1);
}

void xScannerFrame::SendReport(const wxString &loc, wxDebugReportCompress &report) {
    wxHTTP http;
    http.Connect("dankulp.com");

    const char *bound = "--------------------------b29a7c2fe47b9481";

    wxDateTime now = wxDateTime::Now();
    int millis = wxGetUTCTimeMillis().GetLo() % 1000;
    wxString ts = wxString::Format("%04d-%02d-%02d_%02d-%02d-%02d-%03d", now.GetYear(), now.GetMonth()+1, now.GetDay(), now.GetHour(), now.GetMinute(), now.GetSecond(), millis);

    wxString fn = wxString::Format("xScanner-%s_%s_%s_%s.zip", wxPlatformInfo::Get().GetOperatingSystemFamilyName().c_str(), xlights_version_string, GetBitness(), ts);
    const char *ct = "Content-Type: application/octet-stream\n";
    std::string cd = "Content-Disposition: form-data; name=\"userfile\"; filename=\"" + fn.ToStdString() + "\"\n\n";

    wxMemoryBuffer memBuff;
    memBuff.AppendData(bound, strlen(bound));
    memBuff.AppendData("\n", 1);
    memBuff.AppendData(ct, strlen(ct));
    memBuff.AppendData(cd.c_str(), strlen(cd.c_str()));


    wxFile f_in(report.GetCompressedFileName());
    wxFileOffset fLen = f_in.Length();
    void* tmp = memBuff.GetAppendBuf(fLen);
    size_t iRead = f_in.Read(tmp, fLen);
    memBuff.UngetAppendBuf(iRead);
    f_in.Close();

    memBuff.AppendData("\n", 1);
    memBuff.AppendData(bound, strlen(bound));
    memBuff.AppendData("--\n", 3);

    http.SetMethod("POST");
    http.SetPostBuffer("multipart/form-data; boundary=------------------------b29a7c2fe47b9481", memBuff);
    wxInputStream * is = http.GetInputStream("/" + loc + "/index.php");
    char buf[1024];
    is->Read(buf, 1024);
    //printf("%s\n", buf);
    delete is;
    http.Close();
}

void xScannerFrame::OnKeyDown(wxKeyEvent& event)
{
    ValidateWindow();
}

void xScannerFrame::OnMenuItemScanSelected(wxCommandEvent& event)
{
    Scan();
}

void xScannerFrame::OnTreeItemActivated(wxTreeListEvent& event)
{
    auto iptxt = _tree->GetItemText(event.GetItem()).BeforeFirst(' ');
    if (::IsIPValid(iptxt)) {
        ::wxLaunchDefaultBrowser(iptxt);
    }
}

void xScannerFrame::OnTreeRClick(wxTreeListEvent& event)
{
    wxMenu mnuLayer;
    mnuLayer.Append(ID_MNU_EXPORT, "Export to CSV");
    mnuLayer.Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&xScannerFrame::OnPopup, nullptr, this);
    PopupMenu(&mnuLayer);
}

void xScannerFrame::ExportItem(int skip, wxTreeListItem& item, wxFile& f)
{
    wxString out;
    for (int i = 0; i < skip; i++) out += ",";

    auto i1 = _tree->GetItemText(item, 0);
    auto i2 = _tree->GetItemText(item, 1);
    auto i3 = _tree->GetItemText(item, 2);
    auto i4 = _tree->GetItemText(item, 3);

    if (i4 != "") out += i1 + "," + i2 + "," + i3 + "," + i4;
    else if (i3 != "") out += i1 + "," + i2 + "," + i3;
    else if (i2 != "") out += i1 + "," + i2;
    else if (i1 != "") out += i1;

    out += "\r\n";

    f.Write(out);

    skip++;

    for (auto i = _tree->GetFirstChild(item); i.IsOk(); i = _tree->GetNextSibling(i)) {
        ExportItem(skip, i, f);
    }
}

void xScannerFrame::OnPopup(wxCommandEvent& event)
{
    if (event.GetId() == ID_MNU_EXPORT) {
        wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
        wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, "xScanner.csv", wxEmptyString, "Export files (*.csv)|*.csv", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

        if (filename.IsEmpty()) return;

        wxFile f(filename);

        if (!f.Create(filename, true) || !f.IsOpened()) {
            wxMessageBox(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString(), "Error", 5L, this);
            return;
        }

        auto item = _tree->GetRootItem();
        ExportItem(0, item, f);
    }
}
