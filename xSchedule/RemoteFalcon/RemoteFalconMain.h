#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#ifdef _MSC_VER

#include <stdlib.h>

//#define VISUALSTUDIO_MEMORYLEAKDETECTION
#ifdef VISUALSTUDIO_MEMORYLEAKDETECTION
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#endif

//(*Headers(RemoteFalconFrame)
#include <wx/button.h>
#include <wx/frame.h>
#include <wx/hyperlink.h>
#include <wx/menu.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/timer.h>
//*)

#include "../../xLights/xLightsTimer.h"
#include "RemoteFalconOptions.h"
#include "RemoteFalconApp.h"

#include <memory>
#include <future>

class wxDebugReportCompress;
class RemoteFalcon;

wxDECLARE_EVENT(EVT_ADDMESSAGE, wxCommandEvent);

class RemoteFalconFrame : public wxFrame
{
    RemoteFalcon* _remoteFalcon = nullptr;
    RemoteFalconOptions _options;
    std::string _showDir;
    std::atomic<bool> _running;
    std::string _oldSteps;
    std::string _playlist;
    std::string _playingPlaylist;
    std::string _mode;
    std::string _subdomain;
    std::future<void> _sendPlaylistFuture;
    std::future<void> _sendPlayingFuture;
    std::future<void> _handleStatusFuture;
    std::atomic<int> _toProcess;
    bool _viewerControlEnabled = false;

    void ValidateWindow();
    void SendReport(const wxString &loc, wxDebugReportCompress &report);
    void AddMessage(const std::string& msg);
    void DoAddMessage(const std::string& msg);
    void DoNotifyStatus(const std::string& status);
    void DoSendPlayingSong(const std::string& playing);
    void SendPlayingSong(const std::string& playing);

    void GetMode();
    void LoadOptions();
    void SaveOptions();
    void Start();
    void Stop(bool suppressMessage = false);
    bool IsOptionsValid() const;
    void SendPlaylists();
    void DoSendPlaylists();
    void GetAndPlaySong(const std::string& playing);

public:

        RemoteFalconFrame(wxWindow* parent, const std::string& showdir, const std::string& xScheduleURL, p_xSchedule_Action action, wxWindowID id = -1);
        virtual ~RemoteFalconFrame();
        void NotifyStatus(const std::string& status);
        bool FireEvent(const std::string& eventType, const std::string& eventParameter) { return false; }
        bool SendCommand(const std::string& command, const std::string& parameters, std::string& msg);

    private:

        //(*Handlers(RemoteFalconFrame)
        void OnQuit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        void OnMenuItem_OptionsSelected(wxCommandEvent& event);
        void OnMenuItem_ViewLogSelected(wxCommandEvent& event);
        void OnButton_PauseClick(wxCommandEvent& event);
        void OnButton_CloseClick(wxCommandEvent& event);
        void OnClose(wxCloseEvent& event);
        void OnTimer_UpdatePlaylistTrigger(wxTimerEvent& event);
        void OnMenuItem_RFWebSelected(wxCommandEvent& event);
        void OnMenuItem_VisitorWebPageSelected(wxCommandEvent& event);
        void OnHyperlinkCtrl1Click(wxCommandEvent& event);
        //*)
        void HandleAddMessage(wxCommandEvent& event);

        //(*Identifiers(RemoteFalconFrame)
        static const long ID_TEXTCTRL1;
        static const long ID_HYPERLINKCTRL1;
        static const long ID_BUTTON2;
        static const long ID_MNU_OPTIONS;
        static const long ID_MNU_VIEWLOG;
        static const long ID_MNU_RFWEBSITE;
        static const long ID_MNU_VISITORPAGE;
        static const long idMenuAbout;
        static const long ID_TIMER1;
        //*)

        //(*Declarations(RemoteFalconFrame)
        wxButton* Button_Pause;
        wxFlexGridSizer* FlexGridSizer1;
        wxHyperlinkCtrl* HyperlinkCtrl1;
        wxMenu* Menu1;
        wxMenu* Menu5;
        wxMenuItem* MenuItem_Options;
        wxMenuItem* MenuItem_RFWeb;
        wxMenuItem* MenuItem_ViewLog;
        wxMenuItem* MenuItem_VisitorWebPage;
        wxTextCtrl* TextCtrl_Log;
        wxTimer Timer_UpdatePlaylist;
        //*)

        DECLARE_EVENT_TABLE()
};
