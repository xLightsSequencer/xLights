/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
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

#ifndef __WXOSX__
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
#endif

//(*IdInit(xSMSDaemonFrame)
const long xSMSDaemonFrame::ID_STATICTEXT9 = wxNewId();
const long xSMSDaemonFrame::ID_STATICTEXT10 = wxNewId();
const long xSMSDaemonFrame::ID_STATICTEXT3 = wxNewId();
const long xSMSDaemonFrame::ID_STATICTEXT4 = wxNewId();
const long xSMSDaemonFrame::ID_STATICTEXT5 = wxNewId();
const long xSMSDaemonFrame::ID_STATICTEXT6 = wxNewId();
const long xSMSDaemonFrame::ID_STATICTEXT7 = wxNewId();
const long xSMSDaemonFrame::ID_STATICTEXT8 = wxNewId();
const long xSMSDaemonFrame::ID_GRID1 = wxNewId();
const long xSMSDaemonFrame::ID_BUTTON2 = wxNewId();
const long xSMSDaemonFrame::ID_MNU_OPTIONS = wxNewId();
const long xSMSDaemonFrame::ID_MNU_VIEWLOG = wxNewId();
const long xSMSDaemonFrame::ID_MNU_TESTMESSAGES = wxNewId();
const long xSMSDaemonFrame::idMenuAbout = wxNewId();
const long xSMSDaemonFrame::ID_TIMER2 = wxNewId();
const long xSMSDaemonFrame::ID_TIMER_SECOND = wxNewId();
//*)

BEGIN_EVENT_TABLE(xSMSDaemonFrame,wxFrame)
    //(*EventTable(xSMSDaemonFrame)
    //*)
    END_EVENT_TABLE()

bool xSMSDaemonFrame::IsOptionsValid() const
{
    return _options.IsValid();
}

xSMSDaemonFrame::xSMSDaemonFrame(wxWindow* parent, const std::string& showDir, const std::string& xScheduleURL, p_xSchedule_Action action, wxWindowID id)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    _showDir = showDir;
    //_xScheduleURL = xScheduleURL;
    _action = action;

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
    Grid1->CreateGrid(0,4);
    Grid1->EnableEditing(false);
    Grid1->EnableGridLines(true);
    Grid1->SetColLabelValue(0, _("Timestamp"));
    Grid1->SetColLabelValue(1, _("Status"));
    Grid1->SetColLabelValue(2, _("Message"));
    Grid1->SetColLabelValue(3, _("Moderate"));
    Grid1->SetDefaultCellFont( Grid1->GetFont() );
    Grid1->SetDefaultCellTextColour( Grid1->GetForegroundColour() );
    FlexGridSizer4->Add(Grid1, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer1->Add(FlexGridSizer4, 0, wxEXPAND, 2);
    FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
    Button_Pause = new wxButton(this, ID_BUTTON2, _("Pause"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    FlexGridSizer3->Add(Button_Pause, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizer1);
    MenuBar1 = new wxMenuBar();
    Menu5 = new wxMenu();
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
    SendTimer.SetOwner(this, ID_TIMER2);
    SendTimer.Start(60000, false);
    Timer_Second.SetOwner(this, ID_TIMER_SECOND);
    Timer_Second.Start(1000, false);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    Connect(ID_GRID1,wxEVT_GRID_CELL_CHANGED,(wxObjectEventFunction)&xSMSDaemonFrame::OnGrid1CellChanged);
    Connect(ID_GRID1,wxEVT_GRID_SELECT_CELL,(wxObjectEventFunction)&xSMSDaemonFrame::OnGrid1CellSelect);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xSMSDaemonFrame::OnButton_PauseClick);
    Connect(ID_MNU_OPTIONS,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xSMSDaemonFrame::OnMenuItem_OptionsSelected);
    Connect(ID_MNU_VIEWLOG,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xSMSDaemonFrame::OnMenuItem_ViewLogSelected);
    Connect(ID_MNU_TESTMESSAGES,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xSMSDaemonFrame::OnMenuItem_InsertTestMessagesSelected);
    Connect(idMenuAbout,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xSMSDaemonFrame::OnAbout);
    Connect(ID_TIMER2,wxEVT_TIMER,(wxObjectEventFunction)&xSMSDaemonFrame::OnSendTimerTrigger);
    Connect(ID_TIMER_SECOND,wxEVT_TIMER,(wxObjectEventFunction)&xSMSDaemonFrame::OnTimer_SecondTrigger);
    Connect(wxID_ANY,wxEVT_CLOSE_WINDOW,(wxObjectEventFunction)&xSMSDaemonFrame::OnClose);
    //*)

    //Timer_Second.SetName("xSMSDaemon second timer");
    //SendTimer.SetName("xSMSDaemon send timer");

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
    if (_showDir == "")
    {
        LoadShowDir();
    }
    else
    {
        LoadOptions();
        Start();
    }

    Grid1->SetColSize(1, 150);
    Grid1->SetColSize(2, 400);

    ValidateWindow();
}

xSMSDaemonFrame::~xSMSDaemonFrame()
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("xSMSDaemonFrame::~xSMSDaemonFrame");

    Stop();

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

bool xSMSDaemonFrame::Action(const std::string& command, const std::wstring& parameters, const std::wstring& data, const std::wstring& reference, std::wstring& response)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    auto c = wxString(command).Lower();
    // http://127.0.0.1/xScheduleCommand?Command=getsmsqueue&Parameters=
    if (c == "getsmsqueue")
    {
        if (_smsService != nullptr)
        {
            response = _("{\"result\":\"ok\",\"reference\":\"")+reference+_("\",\"messages\":[");
            auto msgs = _smsService->GetMessages();
            for (auto it : msgs)
            {
                response += _("{\"timestamp\":\"");
                response += it._timestamp.FromTimezone(wxDateTime::TZ::GMT0).FormatTime();
                response += _("\",\"status\":\"");
                std::string st = it.GetStatus();
                response += std::wstring(st.begin(), st.end());
                response += _("\",\"message\":\"");
                response += it.GetUIMessage();
                if (it._displayed)
                {
                    response += _("\",\"displayed\":\"true");
                }
                else
                {
                    response += _("\",\"displayed\":\"false");
                }
                if (_options.GetManualModeration())
                {
                    if (it.IsModeratedOk())
                    {
                        response += _("\",\"moderatedok\":\"true");
                    }
                    else
                    {
                        response += _("\",\"moderatedok\":\"false");
                    }
                    response += _("\",\"id\":\"") + wxString::Format("%d", it.GetId());
                }
                else
                {
                    response += _("\",\"moderatedok\":\"disabled");
                }
                response += _("\"},");
            }
            if (response[response.length() - 1] == ',')
            {
                response = response.substr(0, response.length() - 1);
            }
            response += _("]}");
            return true;
        }
        else
        {
            response = _("{\"result\":\"failed\",\"reference\":\"")+reference+_("\",\"command\":\"") +
                command + _("\",\"message\":\"SMS no service.\"}");
            return false;
        }
    }
    // http://127.0.0.1/xScheduleCommand?Command=moderatesms&Parameters=
    else if (c == "moderatesms")
    {
        if (_options.GetManualModeration())
        {
            if (data.size() < 3)
            {
                // not valid
                response = _("{\"result\":\"failed\",\"reference\":\"") + reference + _("\",\"command\":\"") +
                    command + _("\",\"message\":\"Moderation received invalid request.\"}");
                return false;
            }
            else
            {
                bool v = data[0] == 'y';
                int id = wxAtoi(data.substr(2));

                if (_smsService->Moderate(id, v))
                {
                    // we we de-moderated a message which is currently displayed ... immediately hide it
                    if (!v && _smsService->IsDisplayed(id))
                    {
                        Stop();
                        Start();
                    }

                    RefreshList();
                    response = _("{\"result\":\"ok\",\"reference\":\"") + reference + _("\",\"command\":\"") +
                        command + _("\"}");
                    return true;
                }
                else
                {
                    response = _("{\"result\":\"failed\",\"reference\":\"") + reference + _("\",\"command\":\"") +
                        command + _("\",\"message\":\"Moderation did not change the moderation state.\"}");
                    return false;
                }
            }
        }
        else
        {
            response = _("{\"result\":\"failed\",\"reference\":\"") + reference + _("\",\"command\":\"") +
                command + _("\",\"message\":\"Moderation received but moderation is not enabled.\"}");
            return false;
        }
    }

    response = _("{\"result\":\"failed\",\"reference\":\"") + reference + _("\",\"command\":\"") +
        command + _("\",\"message\":\"SMS no service.\"}");

    return false;
}

void xSMSDaemonFrame::OnQuit(wxCommandEvent& event)
{
    event.Skip();
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

    if (_smsService == nullptr)
    {
        MenuItem_InsertTestMessages->Enable(false);
    }
    else
    {
        MenuItem_InsertTestMessages->Enable();
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

void xSMSDaemonFrame::OnButton_CloseClick(wxCommandEvent& event)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("xSMSDaemonFrame::OnButton_CloseClick");
#ifdef __WXOSX__
    Hide();
#else
    Stop();
    Close();
#endif
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
    wxString filename = dir + wxFileName::GetPathSeparator() + fileName;
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
            _smsService->Reset(_options);
        }
    }

    if (_smsService == nullptr)
    {
        if (_options.GetSMSService() == "Bandwidth")
        {
            _smsService = std::make_unique<Bandwidth>(_options);
        }
        else if (_options.GetSMSService() == "Voip.ms")
        {
            _smsService = std::make_unique<Voip_ms>(_options);
        }
        else if (_options.GetSMSService() == "Twilio")
        {
            _smsService = std::make_unique<Twilio>(_options);
        }
        else if (_options.GetSMSService() == "Test")
        {
            _smsService = std::make_unique<TestService>(_options);
        }
    }

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

    wxTimerEvent e(SendTimer);
    OnSendTimerTrigger(e);

    SendTimer.Start(_options.GetDisplayDuration() * 1000, false); //  , "xSMSDaemon send timer");
}

void xSMSDaemonFrame::Stop()
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("xSMSDaemonFrame::Stop");

    SendTimer.Stop();
    SetAllText("");
}

bool xSMSDaemonFrame::SetText(const std::string& t, const std::string& text, const std::wstring& wtext)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    bool ok = false;
    if (_options.IsValid())
    {
        char result[4096];

        std::wstring s;
        if (wtext != "")
        {
            s = wtext;
        }
        else
        {
            s = std::wstring(text.begin(), text.end());
        }
        std::wstring p = t + "," + s + ",";
        _action("Set current text", (const wchar_t*)p.c_str(), "", result, sizeof(result));
        std::string res(result);

        if (Contains(res, _("result\":\"ok")) && text != "")
        {
            StaticText_LastDisplayed->SetLabel(wxDateTime::Now().FormatTime());
            ok = true;
        }
        else
        {
            logger_base.debug("SetText failed:");
            logger_base.debug("   res: %s", (const char *)res.c_str());
        }
    }
    else
    {
        logger_base.warn("Cant set text as settings are not valid.");
    }
    return ok;
}

void xSMSDaemonFrame::SetAllText(const std::string& text, const std::wstring& wtext)
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

        std::vector<SMSMessage> msgs;
        if (_options.GetManualModeration())
        {
            msgs = _smsService->GetModeratedMessages();
        }
        else
        {
            msgs = _smsService->GetMessages();
        }
        if (msgs.size() > 0)
        {
            wxArrayString texts = wxSplit(_options.GetTextItem(), ',');

            int i = 0;
            for (auto it : texts)
            {
                if (msgs.size() > i)
                {
                    auto msg = msgs[i];
                    if (_options.GetMaxTimesToDisplay() == 0 || msg._displayCount < _options.GetMaxTimesToDisplay())
                    {
                        if (SetText(it, msg._message, msg._wmessage))
                        {
                            _smsService->Display(msg);
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
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (_smsService != nullptr)
    {
        TestMessagesDialog dlg(this);

        if (dlg.ShowModal() == wxID_OK)
        {
            auto msgs = dlg.TextCtrl_Messages->GetValue();
            auto ms = wxSplit(msgs, '\n');
            logger_base.debug("Inserting %d test messages.", (int)ms.size());
            _smsService->AddTestMessages(ms, true);
            RefreshList();
        }
    }
}

void xSMSDaemonFrame::RefreshList()
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    _rowIds.clear();
    if (_smsService != nullptr)
    {
        auto msgs = _smsService->GetMessages();
        if (msgs.size() > 0)
        {
            Grid1->Freeze();
            if (Grid1->GetNumberRows() > 0)
            {
                logger_base.debug("Deleting %d rows", (int)Grid1->GetNumberRows());
                Grid1->DeleteRows(0, Grid1->GetNumberRows());
            }
            logger_base.debug("Adding %d rows", (int)msgs.size());
            for (auto it : msgs)
            {
                _suppressGridUpdate = true;
                Grid1->AppendRows(1);
                _suppressGridUpdate = false;;
                int row = Grid1->GetNumberRows() - 1;
                _rowIds.push_back(it.GetId());
                Grid1->SetCellValue(row, 0, it._timestamp.FromTimezone(wxDateTime::TZ::GMT0).FormatTime());
                Grid1->SetReadOnly(row, 0);
                Grid1->SetCellValue(row, 1, it.GetStatus());
                Grid1->SetReadOnly(row, 1);
                Grid1->SetCellValue(row, 2, it.GetUIMessage());
                Grid1->SetReadOnly(row, 2);

                if (_options.GetManualModeration())
                {
                    Grid1->SetCellRenderer(row, 3, new wxGridCellBoolRenderer);
                    Grid1->SetCellEditor(row, 3, new wxGridCellBoolEditor);
                    Grid1->SetCellValue(row, 3, it.IsModeratedOk() ? _("1") : _(""));
                    if (!it.IsModeratedOk())
                    {
                        Grid1->SetCellBackgroundColour(row, 0, *wxLIGHT_GREY);
                        Grid1->SetCellBackgroundColour(row, 1, *wxLIGHT_GREY);
                        Grid1->SetCellBackgroundColour(row, 2, *wxLIGHT_GREY);
                    }
                    Grid1->SetReadOnly(row, 3, false);
                }
                if (it._displayed)
                {
                    Grid1->SetCellBackgroundColour(row, 0, *wxYELLOW);
                    Grid1->SetCellBackgroundColour(row, 1, *wxYELLOW);
                    Grid1->SetCellBackgroundColour(row, 2, *wxYELLOW);
                }
            }

            if (_options.GetManualModeration())
            {
                Grid1->EnableEditing(true);
            }
            else
            {
                Grid1->EnableEditing(false);
            }

            Grid1->Thaw();
        }
        else
        {
            if (Grid1->GetNumberRows() > 0)
            {
                logger_base.debug("Deleting %d rows", (int)Grid1->GetNumberRows());
                Grid1->DeleteRows(0, Grid1->GetNumberRows());
            }
        }

    }
    else
    {
        if (Grid1->GetNumberRows() > 0)
        {
            logger_base.debug("Deleting %d rows", (int)Grid1->GetNumberRows());
            Grid1->DeleteRows(0, Grid1->GetNumberRows());
        }
    }
}

void xSMSDaemonFrame::OnTimer_SecondTrigger(wxTimerEvent& event)
{
    if (_smsService != nullptr)
    {
        if (StaticText_LastRetrieved->GetLabel() != _smsService->GetLastRetrieved())
        {
            StaticText_LastRetrieved->SetLabel(_smsService->GetLastRetrieved());
        }
    }
}

void xSMSDaemonFrame::OnClose(wxCloseEvent& event)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("xSMSDaemonFrame::OnClose");

    if (event.CanVeto()) {
        event.Veto();
    } else{
        event.Skip();
    }
}

void xSMSDaemonFrame::OnGrid1CellChanged(wxGridEvent& event)
{
    if (_suppressGridUpdate) return;
    UpdateModeration();
}

void xSMSDaemonFrame::OnGrid1CellSelect(wxGridEvent& event)
{
    if (_suppressGridUpdate) return;
    UpdateModeration();
}

void xSMSDaemonFrame::UpdateModeration()
{
    bool changed = false;
    for (size_t i = 0; i < Grid1->GetNumberRows(); ++i)
    {
        int id = _rowIds[i];
        bool c = _smsService->Moderate(id, Grid1->GetCellValue(i, 3) == "1");

        // we we de-moderated a message which is currently displayed ... immediately hide it
        if (c && Grid1->GetCellValue(i, 3) != "1" &&_smsService->IsDisplayed(id))
        {
            Stop();
            Start();
        }

        changed |= c;
    }

    if (changed)
    {
        RefreshList();
    }
}