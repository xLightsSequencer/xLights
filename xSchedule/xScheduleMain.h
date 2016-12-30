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
#include <wx/listctrl.h>
#include <wx/sizer.h>
#include <wx/menu.h>
#include <wx/splitter.h>
#include <wx/panel.h>
#include <wx/button.h>
#include <wx/frame.h>
#include <wx/statusbr.h>
//*)

class xScheduleFrame: public wxFrame
{
    public:

        xScheduleFrame(wxWindow* parent,wxWindowID id = -1);
        virtual ~xScheduleFrame();

    private:

        //(*Handlers(xScheduleFrame)
        void OnQuit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        void OnButton_PlayClick(wxCommandEvent& event);
        void OnButton_StopClick(wxCommandEvent& event);
        void OnListView_SequencesItemSelect(wxListEvent& event);
        void OnListView_SequencesItemActivated(wxListEvent& event);
        void OnListView_SequencesItemFocused(wxListEvent& event);
        //*)

        //(*Identifiers(xScheduleFrame)
        static const long ID_PANEL2;
        static const long ID_LISTVIEW1;
        static const long ID_PANEL3;
        static const long ID_LISTVIEW2;
        static const long ID_PANEL5;
        static const long ID_SPLITTERWINDOW1;
        static const long ID_BUTTON_PLAY;
        static const long ID_BUTTON_STOP;
        static const long ID_PANEL1;
        static const long idMenuQuit;
        static const long ID_MNU_PLAYLISTS;
        static const long ID_MNU_SCHEDULES;
        static const long ID_MNU_VIEW_LOG;
        static const long ID_MNU_CHECK_SCHEDULE;
        static const long ID_MNU_OPTIONS;
        static const long idMenuAbout;
        static const long ID_STATUSBAR1;
        //*)

        //(*Declarations(xScheduleFrame)
        wxPanel* Panel5;
        wxMenuItem* MenuItem7;
        wxMenuItem* MenuItem5;
        wxButton* Button_Stop;
        wxMenu* Menu3;
        wxListView* ListView2;
        wxMenuItem* MenuItem4;
        wxListView* ListView_Sequences;
        wxPanel* Panel1;
        wxButton* Button_Play;
        wxPanel* Panel3;
        wxMenuItem* MenuItem3;
        wxStatusBar* StatusBar1;
        wxMenuItem* MenuItem6;
        wxPanel* Panel2;
        wxSplitterWindow* SplitterWindow1;
        wxMenu* Menu4;
        //*)

        DECLARE_EVENT_TABLE()
};

#endif // XSCHEDULEMAIN_H
