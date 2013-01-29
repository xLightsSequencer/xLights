/***************************************************************
 * Name:      nutcrackerMain.h
 * Purpose:   Defines Application Frame
 * Author:    Sean Meighan (sean.meighan@oracle.com)
 * Created:   2013-01-28
 * Copyright: Sean Meighan (nutcracker123.com/nutcracker)
 * License:
 **************************************************************/

#ifndef NUTCRACKERMAIN_H
#define NUTCRACKERMAIN_H

//(*Headers(nutcrackerFrame)
#include <wx/menu.h>
#include <wx/frame.h>
#include <wx/statusbr.h>
//*)

class nutcrackerFrame: public wxFrame
{
    public:

        nutcrackerFrame(wxWindow* parent,wxWindowID id = -1);
        virtual ~nutcrackerFrame();

    private:

        //(*Handlers(nutcrackerFrame)
        void OnQuit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        void OnClose(wxCloseEvent& event);
        //*)

        //(*Identifiers(nutcrackerFrame)
        static const long idMenuQuit;
        static const long ID_MENUITEM1;
        static const long ID_MENUITEM2;
        static const long ID_MENUITEM4;
        static const long ID_MENUITEM3;
        static const long idMenuAbout;
        static const long ID_STATUSBAR1;
        //*)

        //(*Declarations(nutcrackerFrame)
        wxMenu* Menu3;
        wxMenuItem* MenuItem4;
        wxMenuItem* MenuItem3;
        wxMenu* Menu7;
        wxStatusBar* StatusBar1;
        wxMenuItem* MenuItem6;
        wxMenu* MenuItem5;
        wxMenu* Menu8;
        wxMenu* Menu6;
        wxMenu* Menu9;
        wxMenu* Menu5;
        wxMenu* Menu4;
        //*)

        DECLARE_EVENT_TABLE()
};

#endif // NUTCRACKERMAIN_H
