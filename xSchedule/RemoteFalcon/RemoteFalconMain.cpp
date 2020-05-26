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

#include "../xSMSDaemon/Curl.h"

//(*InternalHeaders(RemoteFalconFrame)
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
#include "RemoteFalconMain.h"
#include "RemoteFalconSettingsDialog.h"
#include "RemoteFalconOptions.h"
#include "xSchedule.h"
#include "RemoteFalcon.h"

#include "../../include/xLights.xpm"
#include "../../include/xLights-16.xpm"
#include "../../include/xLights-32.xpm"
#include "../../include/xLights-64.xpm"
#include "../../include/xLights-128.xpm"

#include <log4cpp/Category.hh>

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

//(*IdInit(RemoteFalconFrame)
const long RemoteFalconFrame::ID_TEXTCTRL1 = wxNewId();
const long RemoteFalconFrame::ID_BUTTON2 = wxNewId();
const long RemoteFalconFrame::ID_MNU_OPTIONS = wxNewId();
const long RemoteFalconFrame::ID_MNU_VIEWLOG = wxNewId();
const long RemoteFalconFrame::idMenuAbout = wxNewId();
const long RemoteFalconFrame::ID_TIMER1 = wxNewId();
//*)

wxDEFINE_EVENT(EVT_ADDMESSAGE, wxCommandEvent);

BEGIN_EVENT_TABLE(RemoteFalconFrame,wxFrame)
    //(*EventTable(RemoteFalconFrame)
    //*)
    EVT_COMMAND(wxID_ANY, EVT_ADDMESSAGE, RemoteFalconFrame::HandleAddMessage)
    END_EVENT_TABLE()

bool RemoteFalconFrame::IsOptionsValid() const
{
    return _options.IsValid();
}

void RemoteFalconFrame::SendPlaylists()
{
    if (!_running) return;

    if (!_sendPlaylistFuture.valid() || _sendPlaylistFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
        _sendPlaylistFuture = std::async(std::launch::async, [this]() { DoSendPlaylists(); });
    }
}

void RemoteFalconFrame::DoSendPlaylists()
{
    if (_remoteFalcon == nullptr) return;

    auto plsteps = xSchedule::GetPlayListSteps(_playlist);

    if (plsteps == _oldSteps) {
        // no need to send
    }
    else {
        AddMessage("Uploading playlist to Remote Falcon.");
        auto res = _remoteFalcon->SyncPlayLists(_playlist, plsteps);
        AddMessage("    " + res);

        wxJSONReader reader;
        wxJSONValue val;
        reader.Parse(res, &val);

        if (!val.IsNull() || val["message"].AsString() == "Success") {
            _oldSteps = plsteps;
        }
        else {
            // did not update
        }
    }
}

void RemoteFalconFrame::AddMessage(const std::string& msg)
{
    wxCommandEvent e(EVT_ADDMESSAGE);
    e.SetString(msg);
    wxPostEvent(this, e);
    _toProcess++;
}

RemoteFalconFrame::RemoteFalconFrame(wxWindow* parent, const std::string& showDir, const std::string& xScheduleURL, p_xSchedule_Action action, wxWindowID id)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    _toProcess = 0;
    _running = false;
    _showDir = showDir;

    //(*Initialize(RemoteFalconFrame)
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer4;
    wxMenu* Menu2;
    wxMenuBar* MenuBar1;
    wxMenuItem* MenuItem2;

    Create(parent, wxID_ANY, _("RemoteFalcon"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("wxID_ANY"));
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer1->AddGrowableRow(0);
    FlexGridSizer4 = new wxFlexGridSizer(1, 1, 0, 0);
    FlexGridSizer4->AddGrowableCol(0);
    FlexGridSizer4->AddGrowableRow(0);
    TextCtrl_Log = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL1"));
    FlexGridSizer4->Add(TextCtrl_Log, 1, wxALL|wxEXPAND, 5);
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
    MenuBar1->Append(Menu1, _("Tools"));
    Menu2 = new wxMenu();
    MenuItem2 = new wxMenuItem(Menu2, idMenuAbout, _("About\tF1"), _("Show info about this application"), wxITEM_NORMAL);
    Menu2->Append(MenuItem2);
    MenuBar1->Append(Menu2, _("Help"));
    SetMenuBar(MenuBar1);
    Timer_UpdatePlaylist.SetOwner(this, ID_TIMER1);
    Timer_UpdatePlaylist.Start(10000, false);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&RemoteFalconFrame::OnButton_PauseClick);
    Connect(ID_MNU_OPTIONS,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&RemoteFalconFrame::OnMenuItem_OptionsSelected);
    Connect(ID_MNU_VIEWLOG,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&RemoteFalconFrame::OnMenuItem_ViewLogSelected);
    Connect(idMenuAbout,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&RemoteFalconFrame::OnAbout);
    Connect(ID_TIMER1,wxEVT_TIMER,(wxObjectEventFunction)&RemoteFalconFrame::OnTimer_UpdatePlaylistTrigger);
    Connect(wxID_ANY,wxEVT_CLOSE_WINDOW,(wxObjectEventFunction)&RemoteFalconFrame::OnClose);
    //*)

    // only start the timer when we start the service
    Timer_UpdatePlaylist.Stop();

    SetTitle("Remote Falcon " + GetDisplayVersionString());

    xSchedule::Initialise(action);

    wxIconBundle icons;
    icons.AddIcon(wxIcon(xlights_16_xpm));
    icons.AddIcon(wxIcon(xlights_32_xpm));
    icons.AddIcon(wxIcon(xlights_64_xpm));
    icons.AddIcon(wxIcon(xlights_128_xpm));
    icons.AddIcon(wxIcon(xlights_xpm));
    SetIcons(icons);

    wxConfigBase* config = wxConfigBase::Get();
    int x = config->ReadLong(_("RemoteFalconWindowPosX"), 50);
    int y = config->ReadLong(_("RemoteFalconWindowPosY"), 50);
    int w = config->ReadLong(_("RemoteFalconWindowPosW"), 800);
    int h = config->ReadLong(_("RemoteFalconWindowPosH"), 600);

    // limit weirdness
    if (x < -100) x = 0;
    if (x > 2000) x = 400;
    if (y < -100) y = 0;
    if (y > 2000) y = 400;

    SetPosition(wxPoint(x, y));
    SetSize(w, h);

    logger_base.debug("RemoteFalcon UI %d,%d %dx%d.", x, y, w, h);

    LoadOptions();

    _playlist = xSchedule::DecodePlayList(_options.GetPlaylist());

    _remoteFalcon = new RemoteFalcon(_options);

    Start();

    if (_sendPlaylistFuture.valid()) _sendPlaylistFuture.wait();

    AddMessage("Clearing remote falcon list of songs.");
    int tries = 100;
    bool done = false;
    do {
        auto res = _remoteFalcon->UpdatePlaylistQueue();
        AddMessage("    " + res);

        wxJSONReader reader;
        wxJSONValue val;
        reader.Parse(res, &val);
    
        if (!val.IsNull() && val["message"].AsString() == "Queue Empty") {
            done = true;
        }
        tries--;
    } while (!done && tries > 0);

    if (tries == 0) {
        logger_base.warn("RemoteFalcon failed to clear existing list of songs.");
    }

    ValidateWindow();
}

RemoteFalconFrame::~RemoteFalconFrame()
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("RemoteFalconFrame::~RemoteFalconFrame");

    Stop(true);

    if (_remoteFalcon != nullptr) {
        delete _remoteFalcon;
        _remoteFalcon = nullptr;
    }

    int x, y;
    GetPosition(&x, &y);

    int w, h;
    GetSize(&w, &h);

    wxConfigBase* config = wxConfigBase::Get();
    config->Write(_("RemoteFalconWindowPosX"), x);
    config->Write(_("RemoteFalconWindowPosY"), y);
    config->Write(_("RemoteFalconWindowPosW"), w);
    config->Write(_("RemoteFalconWindowPosH"), h);
    config->Flush();

    //(*Destroy(RemoteFalconFrame)
    //*)
}

void RemoteFalconFrame::OnQuit(wxCommandEvent& event)
{
    event.Skip();
}

void RemoteFalconFrame::OnAbout(wxCommandEvent& event)
{
    auto about = wxString::Format(wxT("RemoteFalcon v%s."), GetDisplayVersionString());
    wxMessageBox(about, _("Welcome to..."));
}

void RemoteFalconFrame::ValidateWindow()
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

void RemoteFalconFrame::OnMenuItem_OptionsSelected(wxCommandEvent& event)
{
    RemoteFalconSettingsDialog dlg(this, &_options);

    Stop();
    if (dlg.ShowModal() == wxID_OK)
    {
        SaveOptions();
        LoadOptions();

        _playlist = xSchedule::DecodePlayList(_options.GetPlaylist());
        AddMessage("Playlist selected: " + _playlist);

        SendPlaylists();
    }
    Start();

    ValidateWindow();
}

void RemoteFalconFrame::OnButton_CloseClick(wxCommandEvent& event)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("RemoteFalconFrame::OnButton_CloseClick");
#ifdef __WXOSX__
    Hide();
#else
    Stop();
    Close();
#endif
}

void RemoteFalconFrame::OnButton_PauseClick(wxCommandEvent& event)
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

void RemoteFalconFrame::OnMenuItem_ViewLogSelected(wxCommandEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxString dir;
    wxString fileName = "RemoteFalcon_l4cpp.log";
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

void RemoteFalconFrame::LoadOptions()
{
    _options.Load();
}

void RemoteFalconFrame::SaveOptions()
{
    _options.Save();
}

void RemoteFalconFrame::Start()
{
    if (!_options.IsValid()) return;
    if (_remoteFalcon == nullptr) return;
    _running = true;

    GetMode();

    SendPlaylists();

    Timer_UpdatePlaylist.Start(10000);
    AddMessage("Started.");
}

void RemoteFalconFrame::Stop(bool suppressMessage)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("RemoteFalconFrame::Stop");

    Timer_UpdatePlaylist.Stop();
    _running = false;

    // ensure any other thread processing is complete
    if (_handleStatusFuture.valid()) _handleStatusFuture.wait();
    if (_sendPlaylistFuture.valid()) _sendPlaylistFuture.wait();
    if (_sendPlayingFuture.valid()) _sendPlayingFuture.wait();

    if (!suppressMessage) {
        AddMessage("Stopped.");
    }
}

void RemoteFalconFrame::DoSendPlayingSong(const std::string& playing)
{
    AddMessage("Updating remote falcon with the playing song: " + playing);
    AddMessage(_remoteFalcon->SendPlayingSong(playing));
}

void RemoteFalconFrame::SendPlayingSong(const std::string& playing)
{
    if (!_sendPlayingFuture.valid() || _sendPlayingFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
        _sendPlayingFuture = std::async(std::launch::async, [this, playing]() { DoSendPlayingSong(playing); });
    }
}

void RemoteFalconFrame::GetMode()
{
    AddMessage("Fetching current playing mode ...");
    auto res = _remoteFalcon->FetchCurrentPlayMode();
    AddMessage(res);

    _mode = "";

    wxJSONReader reader;
    wxJSONValue val;
    reader.Parse(res, &val);

    if (!val.IsNull()) {
        if (!val["viewerControlMode"].IsNull()) {
            _mode = val["viewerControlMode"].AsString();
        }
    }

    if (_mode == "")         {
        AddMessage("ERROR: Unknown mode so defaulting to JUKEBOX.");
        _mode = "jukebox";
    }
    else         {
        AddMessage("MODE: " + _mode);
    }
}

void RemoteFalconFrame::GetAndPlaySong(const std::string& playing)
{
    if (_remoteFalcon == nullptr) return;

    AddMessage("Asking remote falcon for the song we should be playing.");
    std::string song;
    
    if (_mode == "voting") {
        song = _remoteFalcon->FetchHighestVotedPlaylist();
    }
    else {
        song = _remoteFalcon->FetchCurrentPlaylistFromQueue();
    }
    AddMessage("    " + song);

    wxJSONReader reader;
    wxJSONValue val;
    reader.Parse(song, &val);

    std::string nextSong = "";
    if (!val.IsNull())         {
        if (_mode == "voting") {
            if (!val["winningPlaylist"].IsNull()) {
                nextSong = val["winningPlaylist"].AsString();
            }
        }
        else {
            if (!val["nextPlaylist"].IsNull()) {
                nextSong = val["nextPlaylist"].AsString();
            }
        }
    }

    if (nextSong != "" && nextSong != "null" && playing != nextSong) {
        AddMessage("Asking xSchedule to play " + nextSong);
        auto result = xSchedule::EnqueuePlaylistStep(_playlist, nextSong);
        if (_mode != "voting") {
            //auto result = xSchedule::PlayPlayListStep(_playlist, nextSong);
            AddMessage("    " + result);
            if (result == "{\"result\":\"ok\"}") {
                AddMessage("Asking remote falcon to take the song off the queue as it is now playing.");
                AddMessage("    " + _remoteFalcon->UpdatePlaylistQueue());
            }
            else {
                AddMessage("Asking remote falcon to take the song off the queue as it caused an error.");
                AddMessage("    " + _remoteFalcon->UpdatePlaylistQueue());
            }
        }
    }
}

#define GRACE_SECONDS_TO_GRAB_NEXT_SONG 5

void RemoteFalconFrame::DoNotifyStatus(const std::string& status)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    static std::string _lastPlaying;

    wxJSONReader reader;
    wxJSONValue val;
    reader.Parse(status, &val);

    if (!val.IsNull()) {

        auto playing = val["step"].AsString();
        if (_lastPlaying != playing) {
            SendPlayingSong(playing);
            _lastPlaying = playing;
        }

        auto trigger = val["trigger"].AsString();

        // Only play songs if a schedule is playing
        if (trigger == "scheduled" || trigger == "queued") {
            int queueLength = wxAtoi(val["queuelength"].AsString());
            auto lefts = wxAtol(val["leftms"].AsString()) / 1000;

            if (queueLength == 0 || (queueLength == 1 && lefts <= GRACE_SECONDS_TO_GRAB_NEXT_SONG)) {
                GetAndPlaySong(playing);
            }
        }
    }
}

void RemoteFalconFrame::NotifyStatus(const std::string& status)
{
    if (_running) {
        auto now = wxGetUTCTime();

        static auto __lastTime = wxGetUTCTime();

        // process this every 2 seconds
        if (now - __lastTime > 2) {
            if (!_handleStatusFuture.valid() || _handleStatusFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                _handleStatusFuture = std::async(std::launch::async, [this, status]() { DoNotifyStatus(status); });
            }
            __lastTime = now;
        }
    }

    while (_toProcess > 0) {
        ProcessPendingEvents();
    }
}

void RemoteFalconFrame::OnClose(wxCloseEvent& event)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("RemoteFalconFrame::OnClose");
    if (event.CanVeto()) {
        event.Veto();
    } else{
        event.Skip();
    }
}

void RemoteFalconFrame::HandleAddMessage(wxCommandEvent& event)
{
    // while not technically an issue this likely means we are in the middle of exiting
    if (_remoteFalcon == nullptr) return;
    if (_toProcess == 0) return;

    DoAddMessage(event.GetString());
    _toProcess--;
}

void RemoteFalconFrame::DoAddMessage(const std::string& msg)
{
    TextCtrl_Log->SetValue(TextCtrl_Log->GetValue() + "\n" + msg);
    TextCtrl_Log->SetScrollPos(wxVERTICAL, TextCtrl_Log->GetScrollRange(wxVERTICAL));
    TextCtrl_Log->SetInsertionPoint(-1);
}

void RemoteFalconFrame::OnTimer_UpdatePlaylistTrigger(wxTimerEvent& event)
{
    if (!_running) return;
    SendPlaylists();
}
