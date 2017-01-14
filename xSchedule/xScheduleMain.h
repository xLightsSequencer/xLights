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
#include <wx/bmpbuttn.h>
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

wxDECLARE_EVENT(EVT_FRAMEMS, wxCommandEvent);

class xScheduleFrame : public wxFrame
{
    int _manualOTL;
    WebServer* _webServer;
    static ScheduleManager* __schedule;
    std::string _showDir;
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

    wxBitmap _otlon;
    wxBitmap _otloff;
    wxBitmap _otlautoon;
    wxBitmap _otlautooff;
    wxBitmap _save;
    wxBitmap _scheduled;
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

public:

        static ScheduleManager* GetScheduleManager() { return __schedule; }
        xScheduleFrame(wxWindow* parent,wxWindowID id = -1);
        virtual ~xScheduleFrame();
        void CreateDebugReport(wxDebugReportCompress *report);

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
        //*)

        bool IsPlayList(wxTreeItemId id) const;
        bool IsSchedule(wxTreeItemId id) const;
        void OnTreeCtrlMenu(wxCommandEvent &event);
        void OnButton_UserClick(wxCommandEvent& event);
        void RateNotification(wxCommandEvent& event);

        //(*Identifiers(xScheduleFrame)
        static const long ID_BITMAPBUTTON1;
        static const long ID_BITMAPBUTTON3;
        static const long ID_BITMAPBUTTON6;
        static const long ID_BITMAPBUTTON2;
        static const long ID_BITMAPBUTTON4;
        static const long ID_BITMAPBUTTON5;
        static const long ID_BITMAPBUTTON7;
        static const long ID_CUSTOM1;
        static const long ID_PANEL2;
        static const long ID_TREECTRL1;
        static const long ID_PANEL3;
        static const long ID_LISTVIEW1;
        static const long ID_PANEL5;
        static const long ID_SPLITTERWINDOW1;
        static const long ID_PANEL1;
        static const long ID_STATICTEXT1;
        static const long ID_STATICTEXT2;
        static const long ID_PANEL4;
        static const long ID_MNU_SHOWFOLDER;
        static const long ID_MNU_SAVE;
        static const long idMenuQuit;
        static const long ID_MNU_VIEW_LOG;
        static const long ID_MNU_CHECK_SCHEDULE;
        static const long ID_MNU_OPTIONS;
        static const long idMenuAbout;
        static const long ID_STATUSBAR1;
        static const long ID_TIMER1;
        static const long ID_TIMER2;
        //*)

        static const long ID_MNU_ADDPLAYLIST;
        static const long ID_MNU_DUPLICATEPLAYLIST;
        static const long ID_MNU_SCHEDULEPLAYLIST;
        static const long ID_MNU_EDIT;
        static const long ID_MNU_DELETE;
        static const long ID_BUTTON_USER;

        //(*Declarations(xScheduleFrame)
        wxFlexGridSizer* FlexGridSizer4;
        wxBitmapButton* BitmapButton_StepLoop;
        wxBitmapButton* BitmapButton_PLLoop;
        wxBitmapButton* BitmapButton_IsScheduled;
        wxStaticText* StaticText_Time;
        wxPanel* Panel5;
        wxBitmapButton* BitmapButton_OutputToLights;
        wxBitmapButton* BitmapButton_Unsaved;
        wxPanel* Panel4;
        wxMenu* Menu3;
        xLightsTimer _timer;
        wxMenuItem* MenuItem_Save;
        wxPanel* Panel1;
        wxPanel* Panel3;
        wxStaticText* StaticText_ShowDir;
        wxStatusBar* StatusBar1;
        wxDirDialog* DirDialog1;
        wxMenuItem* MenuItem6;
        wxTimer _timerSchedule;
        wxListView* ListView_Running;
        wxTreeCtrl* TreeCtrl_PlayListsSchedules;
        wxMenuItem* MenuItem_ShowFolder;
        wxBitmapButton* BitmapButton_Random;
        wxBitmapButton* BitmapButton_Playing;
        wxPanel* Panel2;
        wxFlexGridSizer* FlexGridSizer1;
        wxSplitterWindow* SplitterWindow1;
        wxMenuItem* MenuItem_Options;
        BrightnessControl* Brightness;
        wxMenuItem* MenuItem_ViewLog;
        //*)

        DECLARE_EVENT_TABLE()
};

#endif // XSCHEDULEMAIN_H
