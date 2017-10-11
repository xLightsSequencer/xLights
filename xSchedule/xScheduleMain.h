/***************************************************************
 * Name:      xScheduleMain.h
 * Purpose:   Defines Application Frame
 * Author:    xLights ()
 * Created:   2016-12-30
 * Copyright: xLights (http://xlights.org)
 * License:
 **************************************************************/

#ifndef XSCHEDULEMAIN_H
#define XSCHEDULEMAIN_H

class BrightnessControl;

//(*Headers(xScheduleFrame)
#include <wx/listctrl.h>
#include <wx/treectrl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/menu.h>
#include <wx/splitter.h>
#include <wx/panel.h>
#include <wx/filedlg.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/dirdlg.h>
#include <wx/frame.h>
#include <wx/timer.h>
#include <wx/statusbr.h>
//*)

#include "../xLights/xLightsTimer.h"
#include <list>

class wxDebugReportCompress;
class ScheduleManager;
class PlayList;
class WebServer;
class Schedule;
class RunningSchedule;
class VolumeDisplay;

wxDECLARE_EVENT(EVT_FRAMEMS, wxCommandEvent);
wxDECLARE_EVENT(EVT_STATUSMSG, wxCommandEvent);
wxDECLARE_EVENT(EVT_SCHEDULECHANGED, wxCommandEvent);
wxDECLARE_EVENT(EVT_DOCHECKSCHEDULE, wxCommandEvent);
wxDECLARE_EVENT(EVT_XYZZY, wxCommandEvent);
wxDECLARE_EVENT(EVT_XYZZYEVENT, wxCommandEvent);
wxDECLARE_EVENT(EVT_RUNACTION, wxCommandEvent);

class xScheduleFrame : public wxFrame
{
    WebServer* _webServer;
    static ScheduleManager* __schedule;
    std::string _showDir;
    wxDateTime _statusSetAt;
    bool _timerOutputFrame;
    bool _suspendOTL;

    void LoadShowDir();
    void SaveShowDir() const;
    void UpdateTree() const;
    void DeleteSelectedItem();
    void ValidateWindow();
    void CreateButtons();
    void UpdateStatus();
    void UpdateSchedule();
    void SendReport(const wxString &loc, wxDebugReportCompress &report);
    std::string GetScheduleName(Schedule* schedule, const std::list<RunningSchedule*>& active) const;
    void LoadSchedule();
    bool HandleHotkeys(wxKeyEvent& event);
    bool HandleSpecialKeys(wxKeyEvent& event);
    void AddPlayList(bool  forceadvanced = false);
    void EditSelectedItem(bool  forceadvanced = false);
    void UpdateUI();
    void DoPaste();
    void DoCopy();
    void AddSchedule();

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

    private:

        //(*Handlers(xScheduleFrame)
        void OnQuit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        void OnTreeCtrl_PlayListsSchedulesItemMenu(wxTreeEvent& event);
        void OnTreeCtrl_PlayListsSchedulesSelectionChanged(wxTreeEvent& event);
        void OnTreeCtrl_PlayListsSchedulesKeyDown(wxTreeEvent& event);
        void OnMenuItem_SaveSelected(wxCommandEvent& event);
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
        //*)

        bool IsPlayList(wxTreeItemId id) const;
        bool IsSchedule(wxTreeItemId id) const;
        void OnTreeCtrlMenu(wxCommandEvent &event);
        void OnButton_UserClick(wxCommandEvent& event);
        void RateNotification(wxCommandEvent& event);
        void StatusMsgNotification(wxCommandEvent& event);
        void RunAction(wxCommandEvent& event);
        void ScheduleChange(wxCommandEvent& event);
        void DoCheckSchedule(wxCommandEvent& event);
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
        static const long ID_BUTTON3;
        static const long ID_BUTTON4;
        static const long ID_PANEL3;
        static const long ID_LISTVIEW1;
        static const long ID_PANEL5;
        static const long ID_SPLITTERWINDOW1;
        static const long ID_PANEL1;
        static const long ID_STATICTEXT1;
        static const long ID_STATICTEXT3;
        static const long ID_STATICTEXT2;
        static const long ID_PANEL4;
        static const long ID_MNU_SHOWFOLDER;
        static const long ID_MNU_SAVE;
        static const long idMenuQuit;
        static const long ID_MNU_MNUADDPLAYLIST;
        static const long ID_MENUITEM1;
        static const long ID_MNU_BACKGROUND;
        static const long ID_MNU_MATRICES;
        static const long ID_MNU_VIRTUALMATRICES;
        static const long ID_MNU_OPTIONS;
        static const long ID_MNU_VIEW_LOG;
        static const long ID_MNU_CHECK_SCHEDULE;
        static const long ID_MNU_WEBINTERFACE;
        static const long ID_MNU_IMPORT;
        static const long ID_MNU_MODENORMAL;
        static const long ID_MNU_FPPMASTER;
        static const long ID_MNU_FPPREMOTE;
        static const long ID_MNU_EDITFPPREMOTE;
        static const long idMenuAbout;
        static const long ID_STATUSBAR1;
        static const long ID_TIMER1;
        static const long ID_TIMER2;
        //*)

        static const long ID_MNU_ADDPLAYLIST;
        static const long ID_MNU_ADDADVPLAYLIST;
        static const long ID_MNU_DUPLICATEPLAYLIST;
        static const long ID_MNU_SCHEDULEPLAYLIST;
        static const long ID_MNU_EDIT;
        static const long ID_MNU_EDITADV;
        static const long ID_MNU_DELETE;
        static const long ID_BUTTON_USER;

        //(*Declarations(xScheduleFrame)
        wxMenuItem* MenuItem_AddPlayList;
        wxFlexGridSizer* FlexGridSizer4;
        wxBitmapButton* BitmapButton_StepLoop;
        wxBitmapButton* BitmapButton_PLLoop;
        wxBitmapButton* BitmapButton_IsScheduled;
        wxStaticText* StaticText_Time;
        wxPanel* Panel5;
        wxBitmapButton* BitmapButton_VolumeUp;
        wxBitmapButton* BitmapButton_OutputToLights;
        wxBitmapButton* BitmapButton_Unsaved;
        wxMenuItem* MenuItem_FPPMaster;
        wxPanel* Panel4;
        wxMenu* Menu3;
        wxButton* Button_Schedule;
        wxMenuItem* MenuItem_Matrices;
        xLightsTimer _timer;
        wxMenuItem* MenuItem_Save;
        wxPanel* Panel1;
        wxFileDialog* FileDialog1;
        wxMenuItem* MenuItem_BackgroundPlaylist;
        wxPanel* Panel3;
        wxStaticText* StaticText_ShowDir;
        wxMenuItem* MenuItem_CheckSchedule;
        wxButton* Button_Delete;
        wxStatusBar* StatusBar1;
        wxDirDialog* DirDialog1;
        wxTimer _timerSchedule;
        wxListView* ListView_Running;
        wxMenuItem* MenuItem_FPPRemote;
        wxButton* Button_Edit;
        wxTreeCtrl* TreeCtrl_PlayListsSchedules;
        wxMenuItem* MenuItem_EditFPPRemotes;
        wxMenuItem* MenuItem_ShowFolder;
        wxBitmapButton* BitmapButton_Random;
        wxBitmapButton* BitmapButton_Playing;
        wxButton* Button_Add;
        VolumeDisplay* Custom_Volume;
        wxMenuItem* MenuItem_Standalone;
        wxPanel* Panel2;
        wxFlexGridSizer* FlexGridSizer1;
        wxSplitterWindow* SplitterWindow1;
        wxMenuItem* MenuItem_WebInterface;
        wxStaticText* StaticText_IP;
        wxMenuItem* MenuItem_ImportxLights;
        wxMenuItem* MenuItem_VirtualMatrices;
        wxMenuItem* MenuItem_Options;
        BrightnessControl* Brightness;
        wxMenuItem* Menu_OutputProcessing;
        wxMenuItem* MenuItem_ViewLog;
        wxMenu* Menu5;
        wxBitmapButton* BitmapButton_VolumeDown;
        wxMenu* Menu4;
        //*)

        DECLARE_EVENT_TABLE()
};

#endif // XSCHEDULEMAIN_H
