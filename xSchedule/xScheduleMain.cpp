/***************************************************************
 * Name:      xScheduleMain.cpp
 * Purpose:   Code for Application Frame
 * Author:    xLights ()
 * Created:   2016-12-30
 * Copyright: xLights (http://xlights.org)
 * License:
 **************************************************************/

#include "xScheduleMain.h"
#include <wx/msgdlg.h>
#include "PlayList/PlayList.h"
#include "MyTreeItemData.h"
#include <wx/config.h>

//(*InternalHeaders(xScheduleFrame)
#include <wx/intl.h>
#include <wx/string.h>
#include "ScheduleManager.h"

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

//(*IdInit(xScheduleFrame)
const long xScheduleFrame::ID_PANEL2 = wxNewId();
const long xScheduleFrame::ID_TREECTRL1 = wxNewId();
const long xScheduleFrame::ID_PANEL3 = wxNewId();
const long xScheduleFrame::ID_LISTVIEW2 = wxNewId();
const long xScheduleFrame::ID_PANEL5 = wxNewId();
const long xScheduleFrame::ID_SPLITTERWINDOW1 = wxNewId();
const long xScheduleFrame::ID_BUTTON_PLAY = wxNewId();
const long xScheduleFrame::ID_BUTTON_STOP = wxNewId();
const long xScheduleFrame::ID_PANEL1 = wxNewId();
const long xScheduleFrame::ID_MNU_SAVE = wxNewId();
const long xScheduleFrame::idMenuQuit = wxNewId();
const long xScheduleFrame::ID_MNU_PLAYLISTS = wxNewId();
const long xScheduleFrame::ID_MNU_SCHEDULES = wxNewId();
const long xScheduleFrame::ID_MNU_VIEW_LOG = wxNewId();
const long xScheduleFrame::ID_MNU_CHECK_SCHEDULE = wxNewId();
const long xScheduleFrame::ID_MNU_OPTIONS = wxNewId();
const long xScheduleFrame::idMenuAbout = wxNewId();
const long xScheduleFrame::ID_STATUSBAR1 = wxNewId();
//*)

const long xScheduleFrame::ID_MNU_ADDPLAYLIST = wxNewId();
const long xScheduleFrame::ID_MNU_SCHEDULEPLAYLIST = wxNewId();
const long xScheduleFrame::ID_MNU_DELETE = wxNewId();
const long xScheduleFrame::ID_MNU_EDIT = wxNewId();

BEGIN_EVENT_TABLE(xScheduleFrame,wxFrame)
    //(*EventTable(xScheduleFrame)
    //*)
END_EVENT_TABLE()

xScheduleFrame::xScheduleFrame(wxWindow* parent,wxWindowID id)
{
    //(*Initialize(xScheduleFrame)
    wxFlexGridSizer* FlexGridSizer4;
    wxMenuItem* MenuItem2;
    wxFlexGridSizer* FlexGridSizer3;
    wxMenuItem* MenuItem1;
    wxFlexGridSizer* FlexGridSizer5;
    wxFlexGridSizer* FlexGridSizer2;
    wxMenu* Menu1;
    wxMenuBar* MenuBar1;
    wxFlexGridSizer* FlexGridSizer1;
    wxMenu* Menu2;

    Create(parent, id, _("xLights Scheduler"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("id"));
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer1->AddGrowableRow(1);
    Panel2 = new wxPanel(this, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
    FlexGridSizer5 = new wxFlexGridSizer(0, 3, 0, 0);
    Panel2->SetSizer(FlexGridSizer5);
    FlexGridSizer5->Fit(Panel2);
    FlexGridSizer5->SetSizeHints(Panel2);
    FlexGridSizer1->Add(Panel2, 1, wxALL|wxEXPAND, 5);
    SplitterWindow1 = new wxSplitterWindow(this, ID_SPLITTERWINDOW1, wxDefaultPosition, wxSize(52,39), wxSP_3D, _T("ID_SPLITTERWINDOW1"));
    SplitterWindow1->SetMinSize(wxSize(10,10));
    SplitterWindow1->SetMinimumPaneSize(10);
    SplitterWindow1->SetSashGravity(0.5);
    Panel3 = new wxPanel(SplitterWindow1, ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL3"));
    FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer2->AddGrowableCol(0);
    FlexGridSizer2->AddGrowableRow(0);
    TreeCtrl_PlayListsSchedules = new wxTreeCtrl(Panel3, ID_TREECTRL1, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE, wxDefaultValidator, _T("ID_TREECTRL1"));
    TreeCtrl_PlayListsSchedules->SetMinSize(wxSize(300,300));
    FlexGridSizer2->Add(TreeCtrl_PlayListsSchedules, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 5);
    Panel3->SetSizer(FlexGridSizer2);
    FlexGridSizer2->Fit(Panel3);
    FlexGridSizer2->SetSizeHints(Panel3);
    Panel5 = new wxPanel(SplitterWindow1, ID_PANEL5, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL5"));
    FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer3->AddGrowableCol(0);
    FlexGridSizer3->AddGrowableRow(0);
    ListView2 = new wxListView(Panel5, ID_LISTVIEW2, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_LISTVIEW2"));
    ListView2->SetMinSize(wxSize(300,300));
    FlexGridSizer3->Add(ListView2, 1, wxALL|wxEXPAND, 5);
    Panel5->SetSizer(FlexGridSizer3);
    FlexGridSizer3->Fit(Panel5);
    FlexGridSizer3->SetSizeHints(Panel5);
    SplitterWindow1->SplitVertically(Panel3, Panel5);
    FlexGridSizer1->Add(SplitterWindow1, 1, wxALL|wxEXPAND, 5);
    Panel1 = new wxPanel(this, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    FlexGridSizer4 = new wxFlexGridSizer(0, 3, 0, 0);
    Button_Play = new wxButton(Panel1, ID_BUTTON_PLAY, _("Play"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_PLAY"));
    FlexGridSizer4->Add(Button_Play, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Stop = new wxButton(Panel1, ID_BUTTON_STOP, _("Stop"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_STOP"));
    FlexGridSizer4->Add(Button_Stop, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel1->SetSizer(FlexGridSizer4);
    FlexGridSizer4->Fit(Panel1);
    FlexGridSizer4->SetSizeHints(Panel1);
    FlexGridSizer1->Add(Panel1, 1, wxALL|wxEXPAND, 5);
    SetSizer(FlexGridSizer1);
    MenuBar1 = new wxMenuBar();
    Menu1 = new wxMenu();
    MenuItem_Save = new wxMenuItem(Menu1, ID_MNU_SAVE, _("&Save"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItem_Save);
    MenuItem1 = new wxMenuItem(Menu1, idMenuQuit, _("Quit\tAlt-F4"), _("Quit the application"), wxITEM_NORMAL);
    Menu1->Append(MenuItem1);
    MenuBar1->Append(Menu1, _("&File"));
    Menu4 = new wxMenu();
    MenuItem3 = new wxMenuItem(Menu4, ID_MNU_PLAYLISTS, _("&Playlists"), wxEmptyString, wxITEM_NORMAL);
    Menu4->Append(MenuItem3);
    MenuItem4 = new wxMenuItem(Menu4, ID_MNU_SCHEDULES, _("&Schedules"), wxEmptyString, wxITEM_NORMAL);
    Menu4->Append(MenuItem4);
    MenuBar1->Append(Menu4, _("&Edit"));
    Menu3 = new wxMenu();
    MenuItem5 = new wxMenuItem(Menu3, ID_MNU_VIEW_LOG, _("&View Log"), wxEmptyString, wxITEM_NORMAL);
    Menu3->Append(MenuItem5);
    MenuItem6 = new wxMenuItem(Menu3, ID_MNU_CHECK_SCHEDULE, _("&Check Schedule"), wxEmptyString, wxITEM_NORMAL);
    Menu3->Append(MenuItem6);
    MenuItem7 = new wxMenuItem(Menu3, ID_MNU_OPTIONS, _("&Options"), wxEmptyString, wxITEM_NORMAL);
    Menu3->Append(MenuItem7);
    MenuBar1->Append(Menu3, _("&Tools"));
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
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    Connect(ID_TREECTRL1,wxEVT_COMMAND_TREE_SEL_CHANGED,(wxObjectEventFunction)&xScheduleFrame::OnTreeCtrl_PlayListsSchedulesSelectionChanged);
    Connect(ID_TREECTRL1,wxEVT_COMMAND_TREE_KEY_DOWN,(wxObjectEventFunction)&xScheduleFrame::OnTreeCtrl_PlayListsSchedulesKeyDown);
    Connect(ID_TREECTRL1,wxEVT_COMMAND_TREE_ITEM_MENU,(wxObjectEventFunction)&xScheduleFrame::OnTreeCtrl_PlayListsSchedulesItemMenu);
    Connect(ID_BUTTON_PLAY,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xScheduleFrame::OnButton_PlayClick);
    Connect(ID_BUTTON_STOP,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xScheduleFrame::OnButton_StopClick);
    Connect(ID_MNU_SAVE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_SaveSelected);
    Connect(idMenuQuit,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnQuit);
    Connect(idMenuAbout,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnAbout);
    //*)

    SetSize(600, 300);

    TreeCtrl_PlayListsSchedules->AddRoot("Playlists");

    // get the show directory
    wxConfigBase* config = wxConfigBase::Get();
    wxString showDir;
    if (!config->Read("LastDir", &showDir))
    {
        // need to prompt for show dir
        showDir = ".";
    }

    _schedule = new ScheduleManager(showDir.ToStdString());
}

xScheduleFrame::~xScheduleFrame()
{
    //(*Destroy(xScheduleFrame)
    //*)

    delete _schedule;
}

void xScheduleFrame::OnQuit(wxCommandEvent& event)
{
    Close();
}

void xScheduleFrame::OnAbout(wxCommandEvent& event)
{
    wxString msg = wxbuildinfo(long_f);
    wxMessageBox(msg, _("Welcome to..."));
}

void xScheduleFrame::OnButton_PlayClick(wxCommandEvent& event)
{
}

void xScheduleFrame::OnButton_StopClick(wxCommandEvent& event)
{
}

void xScheduleFrame::OnListView_SequencesItemSelect(wxListEvent& event)
{
}

void xScheduleFrame::OnListView_SequencesItemActivated(wxListEvent& event)
{
}

void xScheduleFrame::OnListView_SequencesItemFocused(wxListEvent& event)
{
}

bool xScheduleFrame::IsPlayList(wxTreeItemId id)
{
    return (TreeCtrl_PlayListsSchedules->GetItemParent(id) == TreeCtrl_PlayListsSchedules->GetRootItem());
}

bool xScheduleFrame::IsSchedule(wxTreeItemId id)
{
    return (TreeCtrl_PlayListsSchedules->GetItemParent(id) != TreeCtrl_PlayListsSchedules->GetRootItem() && TreeCtrl_PlayListsSchedules->GetItemParent(id) != nullptr);
}

void xScheduleFrame::OnTreeCtrl_PlayListsSchedulesItemMenu(wxTreeEvent& event)
{
    wxTreeItemId treeitem = TreeCtrl_PlayListsSchedules->GetSelection();

    wxMenu mnu;
    mnu.Append(ID_MNU_ADDPLAYLIST, "Add Playlist");

    wxMenuItem* sched = mnu.Append(ID_MNU_SCHEDULEPLAYLIST, "Schedule");
    if (!IsPlayList(treeitem))
    {
        sched->Enable(false);
    }

    wxMenuItem* del = mnu.Append(ID_MNU_DELETE, "Delete");
    wxMenuItem* edit = mnu.Append(ID_MNU_EDIT, "Edit");

    if (!IsPlayList(treeitem) && !IsSchedule(treeitem))
    {
        del->Enable(false);
        edit->Enable(false);
    }

    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&xScheduleFrame::OnTreeCtrlMenu, nullptr, this);
    PopupMenu(&mnu);
}

void xScheduleFrame::OnTreeCtrlMenu(wxCommandEvent &event)
{
    wxTreeItemId treeitem = TreeCtrl_PlayListsSchedules->GetSelection();
    if (event.GetId() == ID_MNU_ADDPLAYLIST)
    {
        PlayList* playlist = new PlayList();
        if (playlist->Configure(this) == nullptr)
        {
            delete playlist;
        }
        else
        {
            wxTreeItemId  newitem = TreeCtrl_PlayListsSchedules->AppendItem(TreeCtrl_PlayListsSchedules->GetRootItem(), playlist->GetName(), -1, -1, new MyTreeItemData(playlist));
            TreeCtrl_PlayListsSchedules->Expand(newitem);
            TreeCtrl_PlayListsSchedules->EnsureVisible(newitem);
            _schedule->AddPlayList(playlist);
        }
    }
    else if (event.GetId() == ID_MNU_SCHEDULEPLAYLIST)
    {
    }
    else if (event.GetId() == ID_MNU_DELETE)
    {
        if (wxMessageBox("Are you sure?", "Are you sure?", wxYES_NO) == wxYES)
        {
            if (IsPlayList(treeitem))
            {
                PlayList* playlist = (PlayList*)((MyTreeItemData*)TreeCtrl_PlayListsSchedules->GetItemData(treeitem))->GetData();

                wxTreeItemIdValue tid;
                for (auto it = TreeCtrl_PlayListsSchedules->GetFirstChild(treeitem, tid); it != nullptr; it = TreeCtrl_PlayListsSchedules->GetNextChild(treeitem, tid))
                {
                    //Schedule* schedule = (Schedule*)((MyTreeItemData*)TreeCtrl_PlayListsSchedules->GetItemData(it))->GetData();
                    //delete schedule;
                }

                delete playlist;

                TreeCtrl_PlayListsSchedules->Delete(treeitem);
            }
            else if (IsSchedule(treeitem))
            {
#pragma todo
            }
        }
    }
    else if (event.GetId() == ID_MNU_EDIT)
    {
        if (IsPlayList(treeitem))
        {
            PlayList* playlist = (PlayList*)((MyTreeItemData*)TreeCtrl_PlayListsSchedules->GetItemData(treeitem))->GetData();
            if (playlist->Configure(this) != nullptr)
            {
                TreeCtrl_PlayListsSchedules->SetItemText(treeitem, playlist->GetName());
            }
        }
        else if (IsSchedule(treeitem))
        {
#pragma todo
        }
    }
}

void xScheduleFrame::OnTreeCtrl_PlayListsSchedulesSelectionChanged(wxTreeEvent& event)
{
}

void xScheduleFrame::OnTreeCtrl_PlayListsSchedulesKeyDown(wxTreeEvent& event)
{
}

void xScheduleFrame::OnMenuItem_SaveSelected(wxCommandEvent& event)
{
    _schedule->Save();
}
