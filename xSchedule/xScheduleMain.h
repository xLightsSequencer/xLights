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

class BrightnessControl;

//(*Headers(xScheduleFrame)
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/dirdlg.h>
#include <wx/filedlg.h>
#include <wx/frame.h>
#include <wx/listctrl.h>
#include <wx/menu.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/splitter.h>
#include <wx/statbmp.h>
#include <wx/stattext.h>
#include <wx/statusbr.h>
#include <wx/timer.h>
#include <wx/treectrl.h>
//*)

#include "../xLights/xLightsTimer.h"
#include "PluginManager.h"
#include <list>

class wxDebugReportCompress;
class ScheduleManager;
class PlayList;
class WebServer;
class Schedule;
class RunningSchedule;
class VolumeDisplay;
class Pinger;

wxDECLARE_EVENT(EVT_FRAMEMS, wxCommandEvent);
wxDECLARE_EVENT(EVT_STATUSMSG, wxCommandEvent);
wxDECLARE_EVENT(EVT_SCHEDULECHANGED, wxCommandEvent);
wxDECLARE_EVENT(EVT_SYNC, wxCommandEvent);
wxDECLARE_EVENT(EVT_DOCHECKSCHEDULE, wxCommandEvent);
wxDECLARE_EVENT(EVT_DOACTION, wxCommandEvent);
wxDECLARE_EVENT(EVT_STOP, wxCommandEvent);
wxDECLARE_EVENT(EVT_QUIT, wxCommandEvent);
wxDECLARE_EVENT(EVT_XYZZY, wxCommandEvent);
wxDECLARE_EVENT(EVT_XYZZYEVENT, wxCommandEvent);
wxDECLARE_EVENT(EVT_RUNACTION, wxCommandEvent);
wxDECLARE_EVENT(EVT_CHANGESHOWFOLDER, wxCommandEvent);

class xScheduleFrame : public wxFrame
{
    FILE* _f = nullptr;
    WebServer* _webServer = nullptr;
    static ScheduleManager* __schedule;
    std::string _showDir;
    wxDateTime _statusSetAt;
    bool _timerOutputFrame;
    bool _suspendOTL;
    Pinger* _pinger;
    wxBitmap _nowebicon;
    wxBitmap _webicon;
    wxBitmap _slowicon;
    bool _webIconDisplayed;
    bool _slowDisplayed;
    wxLongLong _lastSlow;
    PluginManager _pluginManager;

    void AddIPs();
    void LoadShowDir();
    void SaveShowDir() const;
    void UpdateTree() const;
    void DeleteSelectedItem();
    void ValidateWindow();
    void CreateButtons();
    void UpdateStatus(bool force = false);
    void UpdateSchedule();
    void SendReport(const wxString &loc, wxDebugReportCompress &report);
    std::string GetScheduleName(Schedule* schedule, const std::list<RunningSchedule*>& active) const;
    void LoadSchedule();
    bool HandleHotkeys(wxKeyEvent& event);
    bool HandleSpecialKeys(wxKeyEvent& event);
    void AddPlayList(bool  forceadvanced = false);
    void EditSelectedItem(bool  forceadvanced = false);
    void UpdateUI(bool force = false);
    void DoPaste();
    void DoCopy();
    void AddSchedule();
    void RemoteWarning();
    void ModeToUI();
    void UIToMode();

    wxBitmap _otlon;
    wxBitmap _otloff;
    wxBitmap _otlautoon;
    wxBitmap _otlautooff;
    wxBitmap _save;
    wxBitmap _scheduled;
    wxBitmap _queued;
    wxBitmap _notscheduled;
    wxBitmap _inactive;
    wxBitmap _pllooped;
    wxBitmap _plnotlooped;
    wxBitmap _plsteplooped;
    wxBitmap _plstepnotlooped;
    wxBitmap _playing;
    wxBitmap _idle;
    wxBitmap _paused;
    wxBitmap _random;
    wxBitmap _notrandom;
    wxBitmap _volumeup;
    wxBitmap _volumedown;
    wxBitmap _falconremote;

    void SendStatus();

public:

        static ScheduleManager* GetScheduleManager() { return __schedule; }
        xScheduleFrame(wxWindow* parent, const std::string& showdir = "", const std::string& playlist = "", wxWindowID id = -1);
        virtual ~xScheduleFrame();
        void CreateDebugReport(wxDebugReportCompress *report);
        void CreateButton(const std::string& label, const wxColor& c);
        void SetTempMessage(const std::string& msg);
        PluginManager& GetPluginManager() { return _pluginManager; }
        std::string GetWebPluginRequest(const std::string& request);
        wxString ProcessPluginRequest(const wxString& plugin, const wxString& command, const wxString& parameters, const wxString& data, const wxString& reference);
        void ManipulateBuffer(uint8_t* buffer, size_t bufferSize);
        std::string GetOurURL() const;

    private:

        //(*Handlers(xScheduleFrame)
        void OnQuit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        void OnTreeCtrl_PlayListsSchedulesItemMenu(wxTreeEvent& event);
        void OnTreeCtrl_PlayListsSchedulesSelectionChanged(wxTreeEvent& event);
        void OnTreeCtrl_PlayListsSchedulesKeyDown(wxTreeEvent& event);
        void OnMenuItem_SaveSelected(wxCommandEvent& event);
        bool SelectShowFolder();
        void OnMenuItem_ShowFolderSelected(wxCommandEvent& event);
        void OnTreeCtrl_PlayListsSchedulesItemActivated(wxTreeEvent& event);
        void On_timerTrigger(wxTimerEvent& event);
        void On_timerScheduleTrigger(wxTimerEvent& event);
        void OnMenuItem_OptionsSelected(wxCommandEvent& event);
        void OnMenuItem_ViewLogSelected(wxCommandEvent& event);
        void OnResize(wxSizeEvent& event);
        void OnListView_RunningItemActivated(wxListEvent& event);
        void OnBitmapButton_OutputToLightsClick(wxCommandEvent& event);
        void OnBitmapButton_RandomClick(wxCommandEvent& event);
        void OnBitmapButton_PlayingClick(wxCommandEvent& event);
        void OnBitmapButton_PLLoopClick(wxCommandEvent& event);
        void OnBitmapButton_StepLoopClick(wxCommandEvent& event);
        void OnBitmapButton_IsScheduledClick(wxCommandEvent& event);
        void OnBitmapButton_UnsavedClick(wxCommandEvent& event);
        void OnKeyDown(wxKeyEvent& event);
        void OnBitmapButton_VolumeDownClick(wxCommandEvent& event);
        void OnBitmapButton_VolumeUpClick(wxCommandEvent& event);
        void OnCustom_VolumeLeftDown(wxMouseEvent& event);
        void OnMenuItem_StandaloneSelected(wxCommandEvent& event);
        void OnMenuItem_FPPMasterSelected(wxCommandEvent& event);
        void OnMenuItem_FPPRemoteSelected(wxCommandEvent& event);
        void OnMenuItem_WebInterfaceSelected(wxCommandEvent& event);
        void OnMenuItem_AddPlayListSelected(wxCommandEvent& event);
        void OnButton_AddClick(wxCommandEvent& event);
        void OnButton_EditClick(wxCommandEvent& event);
        void OnButton_DeleteClick(wxCommandEvent& event);
        void OnMenu_OutputProcessingSelected(wxCommandEvent& event);
        void OnMenuItem_BackgroundPlaylistSelected(wxCommandEvent& event);
        void OnMenuItem_CheckScheduleSelected(wxCommandEvent& event);
        void OnButton_ScheduleClick(wxCommandEvent& event);
        void OnMenuItem_ImportxLightsSelected(wxCommandEvent& event);
        void OnMenuItem_MatricesSelected(wxCommandEvent& event);
        void OnMenuItem_VirtualMatricesSelected(wxCommandEvent& event);
        void OnMenuItem_EditFPPRemotesSelected(wxCommandEvent& event);
        void OnMenuItem_FPPUnicastRemoteSelected(wxCommandEvent& event);
        void OnMenuItem_ConfigureOSCSelected(wxCommandEvent& event);
        void OnMenuItem_OSCMasterSelected(wxCommandEvent& event);
        void OnMenuItem_OSCRemoteSelected(wxCommandEvent& event);
        void OnListView_PingItemActivated(wxListEvent& event);
        void OnListView_PingItemRClick(wxListEvent& event);
        void OnMenuItem_EditEventsSelected(wxCommandEvent& event);
        void OnMenuItem_ARTNetTimeCodeSlaveSelected(wxCommandEvent& event);
        void OnMenuItem_ARTNetTimeCodeMasterSelected(wxCommandEvent& event);
        void OnMenuItem_CrashSelected(wxCommandEvent& event);
        void OnMenuItem_MIDITimeCodeMasterSelected(wxCommandEvent& event);
        void OnMenuItem_MIDITimeCodeSlaveSelected(wxCommandEvent& event);
        void OnMenuItem5MenuItem_ConfigureMIDITimecodeSelected(wxCommandEvent& event);
        void OnMenuItem_UsexLightsFolderSelected(wxCommandEvent& event);
        void OnMenuItem_TestSelected(wxCommandEvent& event);
        void OnMenuItem_ConfigureTestSelected(wxCommandEvent& event);
        void OnMenuItem_ModeFPPUnicastMasterSelected(wxCommandEvent& event);
        void OnMenuItem_RemoteLatencySelected(wxCommandEvent& event);
        void OnButton_CloneClick(wxCommandEvent& event);
        void OnMenuItem_ModeFPPMulticastMasterSelected(wxCommandEvent& event);
        void OnMenuItem_ModeFPPUnicastCSVMasterSelected(wxCommandEvent& event);
        void OnMenuItem_ModeFPPCSVRemoteSelected(wxCommandEvent& event);
        void OnClose(wxCloseEvent& event);
        void OnMenuItem_SMPTESelected(wxCommandEvent& event);
        void OnMenuItem_ResetWindowLocationsSelected(wxCommandEvent& event);
        //*)

        void OnListView_RunningItemSelected(wxListEvent& event);
        bool IsPlayList(wxTreeItemId id) const;
        bool IsSchedule(wxTreeItemId id) const;
        void OnTreeCtrlMenu(wxCommandEvent &event);
        void OnPingPopup(wxCommandEvent &event);
        void OnButton_UserClick(wxCommandEvent& event);
        void RateNotification(wxCommandEvent& event);
        void StatusMsgNotification(wxCommandEvent& event);
        void RunAction(wxCommandEvent& event);
        void ChangeShowFolder(wxCommandEvent& event);
        void ScheduleChange(wxCommandEvent& event);
        void DoCheckSchedule(wxCommandEvent& event);
        void Sync(wxCommandEvent& event);
        void DoAction(wxCommandEvent& event);
        void DoStop(wxCommandEvent& event);
        void DoXyzzy(wxCommandEvent& event);
        void DoXyzzyEvent(wxCommandEvent& event);
        void CorrectTimer(int rate);

        //(*Identifiers(xScheduleFrame)
        static const long ID_BITMAPBUTTON1;
        static const long ID_BITMAPBUTTON3;
        static const long ID_BITMAPBUTTON6;
        static const long ID_BITMAPBUTTON2;
        static const long ID_BITMAPBUTTON4;
        static const long ID_BITMAPBUTTON5;
        static const long ID_BITMAPBUTTON7;
        static const long ID_CUSTOM1;
        static const long ID_BITMAPBUTTON8;
        static const long ID_CUSTOM2;
        static const long ID_BITMAPBUTTON9;
        static const long ID_PANEL2;
        static const long ID_TREECTRL1;
        static const long ID_BUTTON1;
        static const long ID_BUTTON2;
        static const long ID_BUTTON5;
        static const long ID_BUTTON3;
        static const long ID_BUTTON4;
        static const long ID_PANEL6;
        static const long ID_LISTVIEW2;
        static const long ID_PANEL7;
        static const long ID_SPLITTERWINDOW2;
        static const long ID_PANEL3;
        static const long ID_LISTVIEW1;
        static const long ID_PANEL5;
        static const long ID_SPLITTERWINDOW1;
        static const long ID_PANEL1;
        static const long ID_STATICTEXT1;
        static const long ID_STATICTEXT3;
        static const long ID_STATICTEXT6;
        static const long ID_STATICTEXT4;
        static const long ID_STATICTEXT5;
        static const long ID_STATICTEXT2;
        static const long ID_STATICBITMAP2;
        static const long ID_STATICBITMAP1;
        static const long ID_PANEL4;
        static const long ID_MNU_SHOWFOLDER;
        static const long ID_MNU_USEXLIGHTSFOLDER;
        static const long ID_MNU_SAVE;
        static const long idMenuQuit;
        static const long ID_MNU_MNUADDPLAYLIST;
        static const long ID_MENUITEM1;
        static const long ID_MNU_BACKGROUND;
        static const long ID_MNU_MATRICES;
        static const long ID_MNU_VIRTUALMATRICES;
        static const long ID_MNU_EDITEVENTS;
        static const long ID_MNU_OPTIONS;
        static const long ID_MNU_VIEW_LOG;
        static const long ID_MNU_CHECK_SCHEDULE;
        static const long ID_MNU_WEBINTERFACE;
        static const long ID_MNU_IMPORT;
        static const long ID_MNU_RESETWINDOWS;
        static const long ID_MNU_CRASH;
        static const long ID_MNU_TEST;
        static const long ID_MNU_FPP_BROADCASTMASTER;
        static const long ID_MNU_FPP_MULTICAST;
        static const long ID_MNU_FPP_UNICASTMASTER;
        static const long ID_MNU_FPP_UNICASTCSVMASTER;
        static const long IDM_MNU_ARTNETMASTER;
        static const long ID_MNU_OSCMASTER;
        static const long MNU_MIDITIMECODE_MASTER;
        static const long ID_MNU_MASTER;
        static const long ID_MNU_MODENORMAL;
        static const long ID_MNU_FPPREMOTE;
        static const long ID_MNU_FPPCSVREMOTE;
        static const long ID_MNU_ARTNETTIMECODESLAVE;
        static const long ID_MNU_OSCREMOTE;
        static const long MNU_MIDITIMECODEREMOTE;
        static const long ID_MNU_SMPTE;
        static const long ID_MNU_REMOTECONFIGURE;
        static const long ID_MNU_REMOTE;
        static const long ID_MNU_EDITFPPREMOTE;
        static const long ID_MNU_OSCOPTION;
        static const long MNU_CONFIGUREMIDITIMECODE;
        static const long ID_MNU_CONFIGURE_TEST;
        static const long idMenuAbout;
        static const long ID_STATUSBAR1;
        static const long ID_TIMER1;
        static const long ID_TIMER2;
        //*)

        static const long ID_MNU_EDIT_ADDITIONAL_IPS;
        static const long ID_MNU_ADDPLAYLIST;
        static const long ID_MNU_ADDADVPLAYLIST;
        static const long ID_MNU_DUPLICATEPLAYLIST;
        static const long ID_MNU_SCHEDULEPLAYLIST;
        static const long ID_MNU_EDIT;
        static const long ID_MNU_EDITADV;
        static const long ID_MNU_DELETE;
        static const long ID_BUTTON_USER;

        //(*Declarations(xScheduleFrame)
        BrightnessControl* Brightness;
        VolumeDisplay* Custom_Volume;
        wxBitmapButton* BitmapButton_IsScheduled;
        wxBitmapButton* BitmapButton_OutputToLights;
        wxBitmapButton* BitmapButton_PLLoop;
        wxBitmapButton* BitmapButton_Playing;
        wxBitmapButton* BitmapButton_Random;
        wxBitmapButton* BitmapButton_StepLoop;
        wxBitmapButton* BitmapButton_Unsaved;
        wxBitmapButton* BitmapButton_VolumeDown;
        wxBitmapButton* BitmapButton_VolumeUp;
        wxButton* Button_Add;
        wxButton* Button_Clone;
        wxButton* Button_Delete;
        wxButton* Button_Edit;
        wxButton* Button_Schedule;
        wxDirDialog* DirDialog1;
        wxFileDialog* FileDialog1;
        wxFlexGridSizer* FlexGridSizer1;
        wxFlexGridSizer* FlexGridSizer4;
        wxFlexGridSizer* FlexGridSizer6;
        wxListView* ListView_Ping;
        wxListView* ListView_Running;
        wxMenu* Menu4;
        wxMenu* Menu5;
        wxMenu* Menu6;
        wxMenu* MenuItem3;
        wxMenu* Menu_Plugins;
        wxMenu* ToolsMenu;
        wxMenuItem* MenuItem5MenuItem_ConfigureMIDITimecode;
        wxMenuItem* MenuItem_AddPlayList;
        wxMenuItem* MenuItem_BackgroundPlaylist;
        wxMenuItem* MenuItem_CheckSchedule;
        wxMenuItem* MenuItem_ConfigureOSC;
        wxMenuItem* MenuItem_ConfigureTest;
        wxMenuItem* MenuItem_Crash;
        wxMenuItem* MenuItem_EditEvents;
        wxMenuItem* MenuItem_EditFPPRemotes;
        wxMenuItem* MenuItem_ImportxLights;
        wxMenuItem* MenuItem_Matrices;
        wxMenuItem* MenuItem_ModeArtNetMaster;
        wxMenuItem* MenuItem_ModeArtNetSlave;
        wxMenuItem* MenuItem_ModeFPPBroadcastMaster;
        wxMenuItem* MenuItem_ModeFPPCSVRemote;
        wxMenuItem* MenuItem_ModeFPPMulticastMaster;
        wxMenuItem* MenuItem_ModeFPPRemote;
        wxMenuItem* MenuItem_ModeFPPUnicastCSVMaster;
        wxMenuItem* MenuItem_ModeFPPUnicastMaster;
        wxMenuItem* MenuItem_ModeMIDIMaster;
        wxMenuItem* MenuItem_ModeMIDISlave;
        wxMenuItem* MenuItem_ModeOSCMaster;
        wxMenuItem* MenuItem_ModeOSCRemote;
        wxMenuItem* MenuItem_ModeRemoteDisabled;
        wxMenuItem* MenuItem_ModeTest;
        wxMenuItem* MenuItem_Options;
        wxMenuItem* MenuItem_RemoteLatency;
        wxMenuItem* MenuItem_ResetWindowLocations;
        wxMenuItem* MenuItem_SMPTE;
        wxMenuItem* MenuItem_Save;
        wxMenuItem* MenuItem_ShowFolder;
        wxMenuItem* MenuItem_UsexLightsFolder;
        wxMenuItem* MenuItem_ViewLog;
        wxMenuItem* MenuItem_VirtualMatrices;
        wxMenuItem* MenuItem_WebInterface;
        wxMenuItem* Menu_OutputProcessing;
        wxPanel* Panel1;
        wxPanel* Panel2;
        wxPanel* Panel3;
        wxPanel* Panel4;
        wxPanel* Panel5;
        wxPanel* Panel6;
        wxPanel* Panel7;
        wxSplitterWindow* SplitterWindow1;
        wxSplitterWindow* SplitterWindow2;
        wxStaticBitmap* StaticBitmap_Slow;
        wxStaticBitmap* StaticBitmap_WebIcon;
        wxStaticText* StaticText2;
        wxStaticText* StaticText_IP;
        wxStaticText* StaticText_PacketsPerSec;
        wxStaticText* StaticText_RemoteWarning;
        wxStaticText* StaticText_ShowDir;
        wxStaticText* StaticText_Time;
        wxStatusBar* StatusBar1;
        wxTreeCtrl* TreeCtrl_PlayListsSchedules;
        xLightsTimer _timer;
        xLightsTimer _timerSchedule;
        //*)

        DECLARE_EVENT_TABLE()

        void OnPluginMenu(wxCommandEvent& event);
};
