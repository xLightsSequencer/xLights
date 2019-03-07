/***************************************************************
 * Name:      xSMSDaemonMain.cpp
 * Purpose:   Code for Application Frame
 * Author:    xLights ()
 * Created:   2016-12-30
 * Copyright: xLights (http://xlights.org)
 * License:
 **************************************************************/

#define ZERO 0

#include "Curl.h"

//(*InternalHeaders(xSMSDaemonFrame)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/mimetype.h>
#include <wx/msgdlg.h>
#include <wx/config.h>
#include <wx/file.h>
#include <wx/bitmap.h>
#include <wx/protocol/http.h>
#include <wx/debugrpt.h>
#include <wx/dirdlg.h>
#include <wx/url.h>

#include "../../xLights/xLightsVersion.h"
#include "xSMSDaemonMain.h"

#include "../../include/xLights.xpm"
#include "../../include/xLights-16.xpm"
#include "../../include/xLights-32.xpm"
#include "../../include/xLights-64.xpm"
#include "../../include/xLights-128.xpm"

#include <log4cpp/Category.hh>
#include <wx/filename.h>
#include "SMSSettingsDialog.h"
#include "SMSDaemonOptions.h"
#include "SMSService.h"
#include "TestMessagesDialog.h"
#include "Bandwidth.h"
#include "voip_ms.h"
#include "Twilio.h"

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

//(*IdInit(xSMSDaemonFrame)
const long xSMSDaemonFrame::ID_STATICTEXT9 = wxNewId();
const long xSMSDaemonFrame::ID_STATICTEXT10 = wxNewId();
const long xSMSDaemonFrame::ID_STATICTEXT1 = wxNewId();
const long xSMSDaemonFrame::ID_STATICTEXT2 = wxNewId();
const long xSMSDaemonFrame::ID_STATICTEXT3 = wxNewId();
const long xSMSDaemonFrame::ID_STATICTEXT4 = wxNewId();
const long xSMSDaemonFrame::ID_STATICTEXT5 = wxNewId();
const long xSMSDaemonFrame::ID_STATICTEXT6 = wxNewId();
const long xSMSDaemonFrame::ID_STATICTEXT7 = wxNewId();
const long xSMSDaemonFrame::ID_STATICTEXT8 = wxNewId();
const long xSMSDaemonFrame::ID_GRID1 = wxNewId();
const long xSMSDaemonFrame::ID_BUTTON2 = wxNewId();
const long xSMSDaemonFrame::ID_BUTTON1 = wxNewId();
const long xSMSDaemonFrame::ID_MNU_ShowFolder = wxNewId();
const long xSMSDaemonFrame::ID_MNU_OPTIONS = wxNewId();
const long xSMSDaemonFrame::ID_MNU_VIEWLOG = wxNewId();
const long xSMSDaemonFrame::ID_MNU_TESTMESSAGES = wxNewId();
const long xSMSDaemonFrame::idMenuAbout = wxNewId();
const long xSMSDaemonFrame::ID_TIMER1 = wxNewId();
const long xSMSDaemonFrame::ID_TIMER2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(xSMSDaemonFrame,wxFrame)
    //(*EventTable(xSMSDaemonFrame)
    //*)
    END_EVENT_TABLE()

bool xSMSDaemonFrame::IsOptionsValid() const
{
    return _options.IsValid();
}

xSMSDaemonFrame::xSMSDaemonFrame(wxWindow* parent, const std::string& showdir, const std::string& playlist, wxWindowID id)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    //(*Initialize(xSMSDaemonFrame)
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer4;
    wxMenu* Menu2;
    wxMenuBar* MenuBar1;
    wxMenuItem* MenuItem2;

    Create(parent, wxID_ANY, _("xSMSDaemon"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("wxID_ANY"));
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer1->AddGrowableRow(1);
    FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer2->AddGrowableCol(1);
    StaticText4 = new wxStaticText(this, ID_STATICTEXT9, _("Phone"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
    FlexGridSizer2->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_Phone = new wxStaticText(this, ID_STATICTEXT10, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER, _T("ID_STATICTEXT10"));
    FlexGridSizer2->Add(StaticText_Phone, 1, wxALL|wxEXPAND, 5);
    StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("xSchedule IP Address"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_IPAddress = new wxStaticText(this, ID_STATICTEXT2, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER, _T("ID_STATICTEXT2"));
    FlexGridSizer2->Add(StaticText_IPAddress, 1, wxALL|wxEXPAND, 5);
    StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Text Item"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer2->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_TextItemName = new wxStaticText(this, ID_STATICTEXT4, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER, _T("ID_STATICTEXT4"));
    FlexGridSizer2->Add(StaticText_TextItemName, 1, wxALL|wxEXPAND, 5);
    StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Messages last retrieved"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
    FlexGridSizer2->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_LastRetrieved = new wxStaticText(this, ID_STATICTEXT6, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER, _T("ID_STATICTEXT6"));
    FlexGridSizer2->Add(StaticText_LastRetrieved, 1, wxALL|wxEXPAND, 5);
    StaticText2 = new wxStaticText(this, ID_STATICTEXT7, _("Message last displayed"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
    FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_LastDisplayed = new wxStaticText(this, ID_STATICTEXT8, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER, _T("ID_STATICTEXT8"));
    FlexGridSizer2->Add(StaticText_LastDisplayed, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer4 = new wxFlexGridSizer(1, 1, 0, 0);
    FlexGridSizer4->AddGrowableCol(0);
    FlexGridSizer4->AddGrowableRow(0);
    Grid1 = new wxGrid(this, ID_GRID1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_GRID1"));
    Grid1->CreateGrid(0,3);
    Grid1->EnableEditing(false);
    Grid1->EnableGridLines(true);
    Grid1->SetColLabelValue(0, _("Timestamp"));
    Grid1->SetColLabelValue(1, _("Status"));
    Grid1->SetColLabelValue(2, _("Message"));
    Grid1->SetDefaultCellFont( Grid1->GetFont() );
    Grid1->SetDefaultCellTextColour( Grid1->GetForegroundColour() );
    FlexGridSizer4->Add(Grid1, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer1->Add(FlexGridSizer4, 0, wxEXPAND, 2);
    FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
    Button_Pause = new wxButton(this, ID_BUTTON2, _("Pause"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    FlexGridSizer3->Add(Button_Pause, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Close = new wxButton(this, ID_BUTTON1, _("Close"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer3->Add(Button_Close, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizer1);
    MenuBar1 = new wxMenuBar();
    Menu5 = new wxMenu();
    MenuItem_ShowFolder = new wxMenuItem(Menu5, ID_MNU_ShowFolder, _("Show Folder"), wxEmptyString, wxITEM_NORMAL);
    Menu5->Append(MenuItem_ShowFolder);
    MenuItem_Options = new wxMenuItem(Menu5, ID_MNU_OPTIONS, _("&Options"), wxEmptyString, wxITEM_NORMAL);
    Menu5->Append(MenuItem_Options);
    MenuBar1->Append(Menu5, _("&Edit"));
    Menu1 = new wxMenu();
    MenuItem_ViewLog = new wxMenuItem(Menu1, ID_MNU_VIEWLOG, _("View Log"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItem_ViewLog);
    MenuItem_InsertTestMessages = new wxMenuItem(Menu1, ID_MNU_TESTMESSAGES, _("Insert Test Messages"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItem_InsertTestMessages);
    MenuBar1->Append(Menu1, _("Tools"));
    Menu2 = new wxMenu();
    MenuItem2 = new wxMenuItem(Menu2, idMenuAbout, _("About\tF1"), _("Show info about this application"), wxITEM_NORMAL);
    Menu2->Append(MenuItem2);
    MenuBar1->Append(Menu2, _("Help"));
    SetMenuBar(MenuBar1);
    RetrieveTimer.SetOwner(this, ID_TIMER1);
    RetrieveTimer.Start(60000, false);
    SendTimer.SetOwner(this, ID_TIMER2);
    SendTimer.Start(60000, false);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xSMSDaemonFrame::OnButton_PauseClick);
    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xSMSDaemonFrame::OnButton_CloseClick);
    Connect(ID_MNU_ShowFolder,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xSMSDaemonFrame::OnMenuItem_ShowFolderSelected);
    Connect(ID_MNU_OPTIONS,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xSMSDaemonFrame::OnMenuItem_OptionsSelected);
    Connect(ID_MNU_VIEWLOG,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xSMSDaemonFrame::OnMenuItem_ViewLogSelected);
    Connect(ID_MNU_TESTMESSAGES,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xSMSDaemonFrame::OnMenuItem_InsertTestMessagesSelected);
    Connect(idMenuAbout,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xSMSDaemonFrame::OnAbout);
    Connect(ID_TIMER1,wxEVT_TIMER,(wxObjectEventFunction)&xSMSDaemonFrame::OnRetrieveTimerTrigger);
    Connect(ID_TIMER2,wxEVT_TIMER,(wxObjectEventFunction)&xSMSDaemonFrame::OnSendTimerTrigger);
    //*)

    SetTitle("xLights SMS Daemon " + GetDisplayVersionString());

    wxIconBundle icons;
    icons.AddIcon(wxIcon(xlights_16_xpm));
    icons.AddIcon(wxIcon(xlights_32_xpm));
    icons.AddIcon(wxIcon(xlights_64_xpm));
    icons.AddIcon(wxIcon(xlights_128_xpm));
    icons.AddIcon(wxIcon(xlights_xpm));
    SetIcons(icons);

    wxConfigBase* config = wxConfigBase::Get();
    int x = config->ReadLong(_("xsmsWindowPosX"), 50);
    int y = config->ReadLong(_("xsmsWindowPosY"), 50);
    int w = config->ReadLong(_("xsmsWindowPosW"), 800);
    int h = config->ReadLong(_("xsmsWindowPosH"), 600);

    // limit weirdness
    if (x < -100) x = 0;
    if (x > 2000) x = 400;
    if (y < -100) y = 0;
    if (y > 2000) y = 400;

    SetPosition(wxPoint(x, y));
    SetSize(w, h);

    logger_base.debug("xSMSDaemon UI %d,%d %dx%d.", x, y, w, h);

    logger_base.debug("Loading show folder.");
    if (showdir == "")
    {
        LoadShowDir();
    }
    else
    {
        _showDir = showdir;
    }

    LoadOptions();
    Stop();
    Start();

    Grid1->SetColSize(1, 150);
    Grid1->SetColSize(2, 400);

    ValidateWindow();
}

xSMSDaemonFrame::~xSMSDaemonFrame()
{
    int x, y;
    GetPosition(&x, &y);

    int w, h;
    GetSize(&w, &h);

    wxConfigBase* config = wxConfigBase::Get();
    config->Write(_("xsmsWindowPosX"), x);
    config->Write(_("xsmsWindowPosY"), y);
    config->Write(_("xsmsWindowPosW"), w);
    config->Write(_("xsmsWindowPosH"), h);
    config->Flush();

    //(*Destroy(xSMSDaemonFrame)
    //*)
}

void xSMSDaemonFrame::OnQuit(wxCommandEvent& event)
{
    Stop();
    Close();
}

void xSMSDaemonFrame::OnAbout(wxCommandEvent& event)
{
    auto about = wxString::Format(wxT("xSMSDaemon v%s."), GetDisplayVersionString());
    wxMessageBox(about, _("Welcome to..."));
}

void xSMSDaemonFrame::ValidateWindow()
{
    if (IsOptionsValid())
    {
        Button_Pause->Enable();
    }
    else
    {
        Button_Pause->Disable();
    }
}

void xSMSDaemonFrame::OnMenuItem_OptionsSelected(wxCommandEvent& event)
{
    SMSSettingsDialog dlg(this, &_options);

    Stop();
    if (dlg.ShowModal() == wxID_OK)
    {
        SaveOptions();
        LoadOptions();
    }
    Start();

    ValidateWindow();
}

void xSMSDaemonFrame::CreateDebugReport(wxDebugReportCompress *report) {

    report->Process();

    if (wxDebugReportPreviewStd().Show(*report)) {
        wxMessageBox("Crash report saved to " + report->GetCompressedFileName());
        SendReport("crashUpload", *report);
    }

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.crit("Exiting after creating debug report: " + report->GetCompressedFileName());
    delete report;
    exit(1);
}

void xSMSDaemonFrame::SendReport(const wxString &loc, wxDebugReportCompress &report) {
    wxHTTP http;
    http.Connect("dankulp.com");

    const char *bound = "--------------------------b29a7c2fe47b9481";

    wxDateTime now = wxDateTime::Now();
    int millis = wxGetUTCTimeMillis().GetLo() % 1000;
    wxString ts = wxString::Format("%04d-%02d-%02d_%02d-%02d-%02d-%03d", now.GetYear(), now.GetMonth()+1, now.GetDay(), now.GetHour(), now.GetMinute(), now.GetSecond(), millis);

    wxString fn = wxString::Format("xSMSDaemon-%s_%s_%s_%s.zip", wxPlatformInfo::Get().GetOperatingSystemFamilyName().c_str(), xlights_version_string, GetBitness(), ts);
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

void xSMSDaemonFrame::OnButton_CloseClick(wxCommandEvent& event)
{
    Stop();
    Close();
}

void xSMSDaemonFrame::OnButton_PauseClick(wxCommandEvent& event)
{
    if (Button_Pause->GetLabel() == "Pause")
    {
        Stop();
        Button_Pause->SetLabel("Start");
    }
    else
    {
        Start();
        Button_Pause->SetLabel("Pause");
    }
}


void xSMSDaemonFrame::OnMenuItem_ShowFolderSelected(wxCommandEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    Stop();
    auto nd = wxDirSelector("Select show folder.", _showDir);
    if (nd != "")
    {
        _showDir = nd;
        logger_base.debug("User selected show folder '%s'.", (const char *)_showDir.c_str());
        SaveShowDir();
        LoadOptions();
    }
    Start();
}

void xSMSDaemonFrame::OnMenuItem_ViewLogSelected(wxCommandEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxString dir;
    wxString fileName = "xSMSDaemon_l4cpp.log";
#ifdef __WXMSW__
    wxGetEnv("APPDATA", &dir);
    wxString filename = dir + "/" + fileName;
#endif
#ifdef __WXOSX_MAC__
    wxFileName home;
    home.AssignHomeDir();
    dir = home.GetFullPath();
    wxString filename = dir + "/Library/Logs/" + fileName;
#endif
#ifdef __LINUX__
    wxString filename = "/tmp/" + fileName;
#endif
    wxString fn = "";
    if (wxFile::Exists(filename))
    {
        fn = filename;
    }
    else if (wxFile::Exists(wxFileName(_showDir, fileName).GetFullPath()))
    {
        fn = wxFileName(_showDir, fileName).GetFullPath();
    }
    else if (wxFile::Exists(wxFileName(wxGetCwd(), fileName).GetFullPath()))
    {
        fn = wxFileName(wxGetCwd(), fileName).GetFullPath();
    }

    wxFileType *ft = wxTheMimeTypesManager->GetFileTypeFromExtension("txt");
    if (fn != "" && ft)
    {
        wxString command = ft->GetOpenCommand("foo.txt");
        command.Replace("foo.txt", fn);

        logger_base.debug("Viewing log file %s.", (const char *)fn.c_str());

        wxExecute(command);
        delete ft;
    }
    else
    {
        logger_base.warn("Unable to view log file %s.", (const char *)fn.c_str());
        wxMessageBox(_("Unable to show log file."), _("Error"));
    }
}

std::string xSMSDaemonFrame::xScheduleShowDir()
{
    wxString showDir = "";

    wxConfig *xsconfig = new wxConfig(_("xSchedule"));
    if (xsconfig != nullptr)
    {
        xsconfig->Read(_("SchedulerLastDir"), &showDir);
    }

    return showDir.ToStdString();
}

std::string xSMSDaemonFrame::xSMSDaemonShowDir()
{
    wxString showDir = "";

    wxConfigBase* config = wxConfigBase::Get();
    if (config != nullptr)
    {
        config->Read(_("SMSDaemonDir"), &showDir);
    }

    return showDir.ToStdString();
}

void xSMSDaemonFrame::LoadShowDir()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxConfigBase* config = wxConfigBase::Get();
    logger_base.debug("Config: AppName '%s' Path '%s' Entries %d Groups %d Style %ld Vendor '%s'.", (const char *)config->GetAppName().c_str(), (const char *)config->GetPath().c_str(), (int)config->GetNumberOfEntries(), (int)config->GetNumberOfGroups(), config->GetStyle(), (const char*)config->GetVendorName().c_str());

    // get the show directory
    wxString showDir = xSMSDaemonShowDir();
    if (showDir == "")
    {
        logger_base.debug("Could not read show folder from 'SMSDaemonDir'.");
        showDir = xScheduleShowDir();
        if (showDir == "")
        {
            logger_base.debug("Could not read show folder from 'xSchedule::SchedulerLastDir'.");
            auto nd = wxDirSelector("Select show folder.", _showDir);

            if (nd != "")
            {
                _showDir = nd;
                logger_base.debug("User selected show folder '%s'.", (const char *)_showDir.c_str());
                SaveShowDir();
            }
            else
            {
                _showDir = ".";
            }
        }
        else
        {
            logger_base.debug("Read show folder from 'xSchedule::SchedulerLastDir' location %s.", (const char *)showDir.c_str());
            _showDir = showDir.ToStdString();
            SaveShowDir();
        }
    }
    else
    {
        logger_base.debug("Read show folder from 'SMSDaemonDir' location %s.", (const char *)showDir.c_str());
        _showDir = showDir.ToStdString();
    }

    Stop();
    LoadOptions();
    Start();
}

void xSMSDaemonFrame::SaveShowDir() const
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxConfigBase* config = wxConfigBase::Get();
    auto sd = wxString(_showDir);
    logger_base.debug("Saving show folder location %s.", (const char *)sd.c_str());
    if (!config->Write(_("SMSDaemonDir"), sd))
    {
        logger_base.error("Error saving 'SMSDaemonDir'.");
    }
    config->Flush();
}

void xSMSDaemonFrame::LoadOptions()
{
    _options.Load(_showDir);

    if (_smsService != nullptr)
    {
        if (_options.GetSMSService() != _smsService->GetServiceName())
        {
            _smsService = nullptr;
        }
        else
        {
            _smsService->Reset();
        }
    }

    if (_smsService == nullptr)
    {
        if (_options.GetSMSService() == "Bandwidth")
        {
            _smsService = std::make_unique<Bandwidth>();
        }
        else if (_options.GetSMSService() == "Voip.ms")
        {
            _smsService = std::make_unique<Voip_ms>();
        }
        else if (_options.GetSMSService() == "Twilio")
        {
            _smsService = std::make_unique<Twilio>();
        }
    }

    if (_smsService != nullptr)
    {
        _smsService->SetUser(_options.GetUser());
        _smsService->SetSID(_options.GetSID());
        _smsService->SetToken(_options.GetToken());
        _smsService->SetPhone(_options.GetPhone());
    }

    StaticText_IPAddress->SetLabel(_options.GetXScheduleIP() + ":" + wxString::Format("%d", _options.GetXSchedulePort()));
    StaticText_TextItemName->SetLabel(_options.GetTextItem());
    StaticText_Phone->SetLabel(_options.GetPhone());
}

void xSMSDaemonFrame::SaveOptions()
{
    _options.Save(_showDir);
}

void xSMSDaemonFrame::Start()
{
    if (!_options.IsValid()) return;

    wxTimerEvent e;
    OnRetrieveTimerTrigger(e);
    OnSendTimerTrigger(e);

    RetrieveTimer.Start(_options.GetRetrieveInterval() * 1000, false, "Retrieve");
    SendTimer.Start(_options.GetDisplayDuration() * 1000, false, "Display");
}

void xSMSDaemonFrame::Stop()
{
    SetAllText("");
    SendTimer.Stop();
    RetrieveTimer.Stop();
}

bool xSMSDaemonFrame::SetText(const std::string& t, const std::string& text, const std::wstring wtext)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    bool ok = false;
    if (_options.IsValid())
    {
        wxURI url;
        if (wtext != "")
        {
            auto wtt = wtext;
            Replace(wtt, _("%"), _("%25"));
            Replace(wtt, _(","), _(" "));
            Replace(wtt, _("&"), _("%26"));
            Replace(wtt, _("="), _("%3D"));
            Replace(wtt, _("?"), _("%3F"));

            url.Create("http://" + _options.GetXScheduleIP() + ":" + wxString::Format("%d", _options.GetXSchedulePort()) + "/xScheduleCommand?Command=Set%20current%20text&Parameters=" + t + "," + wtt + ",");
        }
        else
        {
            auto tt = text;
            Replace(tt, "%", "%25");
            Replace(tt, ",", " ");
            Replace(tt, "&", "%26");
            Replace(tt, "=", "%3D");
            Replace(tt, "?", "%3F");

            url.Create("http://" + _options.GetXScheduleIP() + ":" + wxString::Format("%d", _options.GetXSchedulePort()) + "/xScheduleCommand?Command=Set%20current%20text&Parameters=" + t + "," + tt + ",");
        }
        auto u = url.BuildURI().ToStdString();
        auto res = Curl::HTTPSGet(u);
        if (Contains(res, _("result\":\"ok")) && text != "")
        {
            StaticText_LastDisplayed->SetLabel(wxDateTime::Now().FormatTime());
            ok = true;
        }
        else
        {
            if (text != "")
            {
                logger_base.debug("%s", (const char *)u.c_str());
                logger_base.debug("%s", (const char *)res.c_str());
            }
        }
    }
    else
    {
        logger_base.warn("Cant set text as settings are not valid.");
    }
    return ok;
}

void xSMSDaemonFrame::OnRetrieveTimerTrigger(wxTimerEvent& event)
{
    if (_smsService != nullptr)
    {
        if (_smsService->RetrieveMessages(_options))
        {
            StaticText_LastRetrieved->SetLabel(wxDateTime::Now().FormatTime());
            RefreshList();
        }
    }
}

void xSMSDaemonFrame::SetAllText(const std::string& text, const std::wstring wtext)
{
    wxArrayString texts = wxSplit(_options.GetTextItem(), ',');

    for (auto it : texts)
    {
        SetText(it, text, wtext);
    }
}

void xSMSDaemonFrame::OnSendTimerTrigger(wxTimerEvent& event)
{
    if (_smsService != nullptr)
    {
        _smsService->ClearDisplayed();
        _smsService->PrepareMessages(_options.GetMaxMessageAge());
        auto& msgs = _smsService->GetMessages();
        if (msgs.size() > 0)
        {
            wxArrayString texts = wxSplit(_options.GetTextItem(), ',');

            int i = 0;
            for (auto it : texts)
            {
                if (msgs.size() > i)
                {
                    auto& msg = msgs[i];
                    if (_options.GetMaxTimesToDisplay() == 0 || msg._displayCount < _options.GetMaxTimesToDisplay())
                    {
                        if (SetText(it, msg._message, msg._wmessage))
                        {
                            msg._displayCount++;
                            msg._displayed = true;
                        }
                    }
                    else
                    {
                        SetText(it, _options.GetDefaultMessage());
                    }
                }
                else
                {
                    SetText(it, _options.GetDefaultMessage());
                }
                i++;
            }
        }
        else
        {
            SetAllText(_options.GetDefaultMessage());
        }
    }
    else
    {
        SetAllText(_options.GetDefaultMessage());
    }
    RefreshList();
}

void xSMSDaemonFrame::OnMenuItem_InsertTestMessagesSelected(wxCommandEvent& event)
{
    if (_smsService != nullptr)
    {
        TestMessagesDialog dlg(this);

        if (dlg.ShowModal() == wxID_OK)
        {
            auto msgs = dlg.TextCtrl_Messages->GetValue();
            auto ms = wxSplit(msgs, '\n');
            _smsService->AddTestMessages(ms, _options);
            RefreshList();
        }
    }
}

void xSMSDaemonFrame::RefreshList()
{
    if (_smsService != nullptr)
    {
        auto& msgs = _smsService->GetMessages();
        if (msgs.size() > 0)
        {
            Grid1->Freeze();
            if (Grid1->GetNumberRows() > 0)
            {
                Grid1->DeleteRows(0, Grid1->GetNumberRows());
            }
            for (auto it : msgs)
            {
                Grid1->AppendRows(1);
                int row = Grid1->GetNumberRows() - 1;
                Grid1->SetCellValue(row, 0, it._timestamp.FromTimezone(wxDateTime::TZ::GMT0).FormatTime());
                Grid1->SetCellValue(row, 1, it.GetStatus());
                if (it._wmessage != "")
                {
                    Grid1->SetCellValue(row, 2, wxString(it._from) + ": " + wxString(it._wmessage));
                }
                else
                {
                    Grid1->SetCellValue(row, 2, it._from + ": " + it._message);
                }
                if (it._displayed)
                {
                    Grid1->SetCellBackgroundColour(row, 0, *wxYELLOW);
                    Grid1->SetCellBackgroundColour(row, 1, *wxYELLOW);
                    Grid1->SetCellBackgroundColour(row, 2, *wxYELLOW);
                }
            }
            Grid1->Thaw();
        }
        else
        {
            if (Grid1->GetNumberRows() > 0)
            {
                Grid1->DeleteRows(0, Grid1->GetNumberRows());
            }
        }

    }
    else
    {
        if (Grid1->GetNumberRows() > 0)
        {
            Grid1->DeleteRows(0, Grid1->GetNumberRows());
        }
    }
}