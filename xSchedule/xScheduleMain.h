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

//(*Headers(xScheduleFrame)
#include <wx/treectrl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/menu.h>
#include <wx/splitter.h>
#include <wx/panel.h>
#include <wx/dirdlg.h>
#include <wx/frame.h>
#include <wx/timer.h>
#include <wx/statusbr.h>
//*)

#include "../xLights/xLightsTimer.h"

class ScheduleManager;
class PlayList;
class WebServer;

wxDECLARE_EVENT(EVT_FRAMEMS, wxCommandEvent);

class xScheduleFrame : public wxFrame
{
    WebServer* _webServer;
    static ScheduleManager* __schedule;
    std::string _showDir;
    void LoadShowDir();
    void SaveShowDir() const;
    void UpdateTree() const;
    void DeleteSelectedItem();
    void ValidateWindow();
    void CreateButtons();

public:

        static ScheduleManager* GetScheduleManager() { return __schedule; }
        xScheduleFrame(wxWindow* parent,wxWindowID id = -1);
        virtual ~xScheduleFrame();

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
        //*)

        bool IsPlayList(wxTreeItemId id) const;
        bool IsSchedule(wxTreeItemId id) const;
        void OnTreeCtrlMenu(wxCommandEvent &event);
        void OnButton_UserClick(wxCommandEvent& event);
        void RateNotification(wxCommandEvent& event);

        //(*Identifiers(xScheduleFrame)
        static const long ID_PANEL2;
        static const long ID_TREECTRL1;
        static const long ID_PANEL3;
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
        static const long ID_MNU_PLAYNOW;

        //(*Declarations(xScheduleFrame)
        wxFlexGridSizer* FlexGridSizer4;
        wxPanel* Panel5;
        wxPanel* Panel4;
        wxMenu* Menu3;
        xLightsTimer _timer;
        wxMenuItem* MenuItem_Save;
        wxPanel* Panel1;
        wxStaticText* StaticText_Status;
        wxPanel* Panel3;
        wxStaticText* StaticText_ShowDir;
        wxStatusBar* StatusBar1;
        wxDirDialog* DirDialog1;
        wxMenuItem* MenuItem6;
        wxTimer _timerSchedule;
        wxTreeCtrl* TreeCtrl_PlayListsSchedules;
        wxMenuItem* MenuItem_ShowFolder;
        wxPanel* Panel2;
        wxFlexGridSizer* FlexGridSizer1;
        wxSplitterWindow* SplitterWindow1;
        wxMenuItem* MenuItem_Options;
        wxMenuItem* MenuItem_ViewLog;
        //*)

        DECLARE_EVENT_TABLE()
};

#endif // XSCHEDULEMAIN_H
