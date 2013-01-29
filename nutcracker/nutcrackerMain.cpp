/***************************************************************
 * Name:      nutcrackerMain.cpp
 * Purpose:   Code for Application Frame
 * Author:    Sean Meighan (sean.meighan@oracle.com)
 * Created:   2013-01-28
 * Copyright: Sean Meighan (nutcracker123.com/nutcracker)
 * License:
 **************************************************************/

#include "wx_pch.h"
#include "nutcrackerMain.h"
#include <wx/msgdlg.h>

//(*InternalHeaders(nutcrackerFrame)
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/intl.h>
#include <wx/image.h>
#include <wx/string.h>
//*)

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

//(*IdInit(nutcrackerFrame)
const long nutcrackerFrame::idMenuQuit = wxNewId();
const long nutcrackerFrame::ID_MENUITEM1 = wxNewId();
const long nutcrackerFrame::ID_MENUITEM2 = wxNewId();
const long nutcrackerFrame::ID_MENUITEM4 = wxNewId();
const long nutcrackerFrame::ID_MENUITEM3 = wxNewId();
const long nutcrackerFrame::idMenuAbout = wxNewId();
const long nutcrackerFrame::ID_STATUSBAR1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(nutcrackerFrame,wxFrame)
    //(*EventTable(nutcrackerFrame)
    //*)
END_EVENT_TABLE()

nutcrackerFrame::nutcrackerFrame(wxWindow* parent,wxWindowID id)
{
    //(*Initialize(nutcrackerFrame)
    wxMenuItem* MenuItem2;
    wxMenuItem* MenuItem1;
    wxMenu* Menu1;
    wxMenuBar* MenuBar1;
    wxMenu* Menu2;

    Create(parent, id, _("Nutcracker"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("id"));
    SetClientSize(wxSize(869,290));
    {
    	wxIcon FrameIcon;
    	FrameIcon.CopyFromBitmap(wxBitmap(wxImage(_T("C:\\xampp\\htdocs\\nutcracker\\images\\IMG_7965_gray.png"))));
    	SetIcon(FrameIcon);
    }
    MenuBar1 = new wxMenuBar();
    Menu1 = new wxMenu();
    MenuItem1 = new wxMenuItem(Menu1, idMenuQuit, _("Quit\tAlt-F4"), _("Quit the application"), wxITEM_NORMAL);
    Menu1->Append(MenuItem1);
    MenuBar1->Append(Menu1, _("&File"));
    Menu3 = new wxMenu();
    MenuItem3 = new wxMenuItem(Menu3, ID_MENUITEM1, _("Create"), wxEmptyString, wxITEM_NORMAL);
    Menu3->Append(MenuItem3);
    MenuItem4 = new wxMenuItem(Menu3, ID_MENUITEM2, _("Edit"), wxEmptyString, wxITEM_NORMAL);
    Menu3->Append(MenuItem4);
    MenuBar1->Append(Menu3, _("Models"));
    Menu4 = new wxMenu();
    MenuBar1->Append(Menu4, _("Effects"));
    Menu5 = new wxMenu();
    MenuBar1->Append(Menu5, _("Projects"));
    Menu6 = new wxMenu();
    MenuBar1->Append(Menu6, _("Xmas Songs"));
    Menu7 = new wxMenu();
    MenuBar1->Append(Menu7, _("Gallery"));
    Menu8 = new wxMenu();
    MenuItem5 = new wxMenu();
    MenuItem6 = new wxMenuItem(MenuItem5, ID_MENUITEM4, _("Nutcracker Tutorials"), wxEmptyString, wxITEM_NORMAL);
    MenuItem5->Append(MenuItem6);
    Menu8->Append(ID_MENUITEM3, _("Cleanup Old Effects and Models"), MenuItem5, wxEmptyString);
    MenuBar1->Append(Menu8, _("Admin"));
    Menu9 = new wxMenu();
    MenuBar1->Append(Menu9, _("Help"));
    Menu2 = new wxMenu();
    MenuItem2 = new wxMenuItem(Menu2, idMenuAbout, _("About\tF1"), _("Show info about this application"), wxITEM_NORMAL);
    Menu2->Append(MenuItem2);
    MenuBar1->Append(Menu2, _("Help"));
    SetMenuBar(MenuBar1);
    StatusBar1 = new wxStatusBar(this, ID_STATUSBAR1, 0, _T("ID_STATUSBAR1"));
    int __wxStatusBarWidths_1[1] = { -1 };
    int __wxStatusBarStyles_1[1] = { wxSB_NORMAL };
    StatusBar1->SetFieldsCount(1,__wxStatusBarWidths_1);
    StatusBar1->SetStatusStyles(1,__wxStatusBarStyles_1);
    SetStatusBar(StatusBar1);

    Connect(idMenuQuit,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&nutcrackerFrame::OnQuit);
    Connect(idMenuAbout,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&nutcrackerFrame::OnAbout);
    Connect(wxID_ANY,wxEVT_CLOSE_WINDOW,(wxObjectEventFunction)&nutcrackerFrame::OnClose);
    //*)
}

nutcrackerFrame::~nutcrackerFrame()
{
    //(*Destroy(nutcrackerFrame)
    //*)
}

void nutcrackerFrame::OnQuit(wxCommandEvent& event)
{
    Close();
}

void nutcrackerFrame::OnAbout(wxCommandEvent& event)
{
    wxString msg = wxbuildinfo(long_f);
    wxMessageBox(msg, _("Welcome to..."));
}

void nutcrackerFrame::OnClose(wxCloseEvent& event)
{
    event.Skip(true);
}
