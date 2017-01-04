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
#include "ScheduleManager.h"
#include "Schedule.h"

//(*InternalHeaders(xScheduleFrame)
#include <wx/intl.h>
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

//(*IdInit(xScheduleFrame)
const long xScheduleFrame::ID_PANEL2 = wxNewId();
const long xScheduleFrame::ID_TREECTRL1 = wxNewId();
const long xScheduleFrame::ID_PANEL3 = wxNewId();
const long xScheduleFrame::ID_PANEL5 = wxNewId();
const long xScheduleFrame::ID_SPLITTERWINDOW1 = wxNewId();
const long xScheduleFrame::ID_BUTTON_PLAY = wxNewId();
const long xScheduleFrame::ID_BUTTON_STOP = wxNewId();
const long xScheduleFrame::ID_STATICTEXT1 = wxNewId();
const long xScheduleFrame::ID_PANEL1 = wxNewId();
const long xScheduleFrame::ID_MNU_SHOWFOLDER = wxNewId();
const long xScheduleFrame::ID_MNU_SAVE = wxNewId();
const long xScheduleFrame::idMenuQuit = wxNewId();
const long xScheduleFrame::ID_MNU_VIEW_LOG = wxNewId();
const long xScheduleFrame::ID_MNU_CHECK_SCHEDULE = wxNewId();
const long xScheduleFrame::ID_MNU_OPTIONS = wxNewId();
const long xScheduleFrame::idMenuAbout = wxNewId();
const long xScheduleFrame::ID_STATUSBAR1 = wxNewId();
const long xScheduleFrame::ID_TIMER1 = wxNewId();
const long xScheduleFrame::ID_TIMER2 = wxNewId();
//*)

const long xScheduleFrame::ID_MNU_ADDPLAYLIST = wxNewId();
const long xScheduleFrame::ID_MNU_SCHEDULEPLAYLIST = wxNewId();
const long xScheduleFrame::ID_MNU_DELETE = wxNewId();
const long xScheduleFrame::ID_MNU_EDIT = wxNewId();
const long xScheduleFrame::ID_MNU_PLAYNOW = wxNewId();

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
    StaticText_Status = new wxStaticText(Panel1, ID_STATICTEXT1, _("Not Playing"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer4->Add(StaticText_Status, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel1->SetSizer(FlexGridSizer4);
    FlexGridSizer4->Fit(Panel1);
    FlexGridSizer4->SetSizeHints(Panel1);
    FlexGridSizer1->Add(Panel1, 1, wxALL|wxEXPAND, 5);
    SetSizer(FlexGridSizer1);
    MenuBar1 = new wxMenuBar();
    Menu1 = new wxMenu();
    MenuItem_ShowFolder = new wxMenuItem(Menu1, ID_MNU_SHOWFOLDER, _("Show &Folder"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItem_ShowFolder);
    MenuItem_Save = new wxMenuItem(Menu1, ID_MNU_SAVE, _("&Save"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItem_Save);
    MenuItem1 = new wxMenuItem(Menu1, idMenuQuit, _("Quit\tAlt-F4"), _("Quit the application"), wxITEM_NORMAL);
    Menu1->Append(MenuItem1);
    MenuBar1->Append(Menu1, _("&File"));
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
    DirDialog1 = new wxDirDialog(this, _("Select show folder ..."), wxEmptyString, wxDD_DEFAULT_STYLE, wxDefaultPosition, wxDefaultSize, _T("wxDirDialog"));
    _timer.SetOwner(this, ID_TIMER1);
    _timer.Start(50, false);
    _timerSchedule.SetOwner(this, ID_TIMER2);
    _timerSchedule.Start(1000, false);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    Connect(ID_TREECTRL1,wxEVT_COMMAND_TREE_ITEM_ACTIVATED,(wxObjectEventFunction)&xScheduleFrame::OnTreeCtrl_PlayListsSchedulesItemActivated);
    Connect(ID_TREECTRL1,wxEVT_COMMAND_TREE_SEL_CHANGED,(wxObjectEventFunction)&xScheduleFrame::OnTreeCtrl_PlayListsSchedulesSelectionChanged);
    Connect(ID_TREECTRL1,wxEVT_COMMAND_TREE_KEY_DOWN,(wxObjectEventFunction)&xScheduleFrame::OnTreeCtrl_PlayListsSchedulesKeyDown);
    Connect(ID_TREECTRL1,wxEVT_COMMAND_TREE_ITEM_MENU,(wxObjectEventFunction)&xScheduleFrame::OnTreeCtrl_PlayListsSchedulesItemMenu);
    Connect(ID_BUTTON_PLAY,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xScheduleFrame::OnButton_PlayClick);
    Connect(ID_BUTTON_STOP,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xScheduleFrame::OnButton_StopClick);
    Connect(ID_MNU_SHOWFOLDER,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_ShowFolderSelected);
    Connect(ID_MNU_SAVE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_SaveSelected);
    Connect(idMenuQuit,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnQuit);
    Connect(idMenuAbout,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnAbout);
    Connect(ID_TIMER1,wxEVT_TIMER,(wxObjectEventFunction)&xScheduleFrame::On_timerTrigger);
    Connect(ID_TIMER2,wxEVT_TIMER,(wxObjectEventFunction)&xScheduleFrame::On_timerScheduleTrigger);
    //*)

    SetSize(600, 300);

    LoadShowDir();

    _schedule = new ScheduleManager(_showDir);

    UpdateTree();
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
    _schedule->StopAll();
}

bool xScheduleFrame::IsPlayList(wxTreeItemId id) const
{
    return (TreeCtrl_PlayListsSchedules->GetItemParent(id) == TreeCtrl_PlayListsSchedules->GetRootItem());
}

bool xScheduleFrame::IsSchedule(wxTreeItemId id) const
{
    return (TreeCtrl_PlayListsSchedules->GetItemParent(id) != TreeCtrl_PlayListsSchedules->GetRootItem() && TreeCtrl_PlayListsSchedules->GetItemParent(id) != nullptr);
}

void xScheduleFrame::OnTreeCtrl_PlayListsSchedulesItemMenu(wxTreeEvent& event)
{
    wxTreeItemId treeitem = TreeCtrl_PlayListsSchedules->HitTest(event.GetPoint());
    TreeCtrl_PlayListsSchedules->SelectItem(treeitem);

    wxMenu mnu;
    mnu.Append(ID_MNU_ADDPLAYLIST, "Add Playlist");

    wxMenuItem* sched = mnu.Append(ID_MNU_SCHEDULEPLAYLIST, "Schedule");
    if (!IsPlayList(treeitem))
    {
        sched->Enable(false);
    }

    wxMenuItem* del = mnu.Append(ID_MNU_DELETE, "Delete");
    wxMenuItem* edit = mnu.Append(ID_MNU_EDIT, "Edit");
    wxMenuItem* play = mnu.Append(ID_MNU_PLAYNOW, "Play Now");

    if (!IsPlayList(treeitem) && !IsSchedule(treeitem))
    {
        del->Enable(false);
        edit->Enable(false);
    }

    if (!IsPlayList(treeitem))
    {
        play->Enable(false);
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
        Schedule* schedule = new Schedule();
        if (schedule->Configure(this) == nullptr)
        {
            delete schedule;
        }
        else
        {
            wxTreeItemId  newitem = TreeCtrl_PlayListsSchedules->AppendItem(treeitem, schedule->GetName(), -1, -1, new MyTreeItemData(schedule));
            PlayList* playlist = (PlayList*)((MyTreeItemData*)TreeCtrl_PlayListsSchedules->GetItemData(treeitem))->GetData();
            TreeCtrl_PlayListsSchedules->Expand(treeitem);
            TreeCtrl_PlayListsSchedules->EnsureVisible(newitem);
            playlist->AddSchedule(schedule);
        }
    }
    else if (event.GetId() == ID_MNU_DELETE)
    {
        DeleteSelectedItem();
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
            Schedule* schedule = (Schedule*)((MyTreeItemData*)TreeCtrl_PlayListsSchedules->GetItemData(treeitem))->GetData();
            if (schedule->Configure(this) != nullptr)
            {
                TreeCtrl_PlayListsSchedules->SetItemText(treeitem, schedule->GetName());
            }
        }
    }
    else if (event.GetId() == ID_MNU_PLAYNOW)
    {
        PlayList* playlist = (PlayList*)((MyTreeItemData*)TreeCtrl_PlayListsSchedules->GetItemData(treeitem))->GetData();
        size_t rate = _schedule->PlayPlayList(playlist);

        // if the desired rate is different than the current rate then restart timer with the desired rate
        if (_timer.GetInterval() != rate)
        {
            _timer.Stop();
            _timer.Start(rate);
        }
    }
}

void xScheduleFrame::DeleteSelectedItem()
{
    wxTreeItemId treeitem = TreeCtrl_PlayListsSchedules->GetSelection();
    if (treeitem.IsOk() && (IsPlayList(treeitem) || IsSchedule(treeitem)))
    {
        if (wxMessageBox("Are you sure?", "Are you sure?", wxYES_NO) == wxYES)
        {
            wxTreeItemId parent = TreeCtrl_PlayListsSchedules->GetItemParent(treeitem);
            if (IsPlayList(treeitem))
            {
                PlayList* playlist = (PlayList*)((MyTreeItemData*)TreeCtrl_PlayListsSchedules->GetItemData(treeitem))->GetData();
                _schedule->RemovePlayList(playlist);
                delete playlist;

                TreeCtrl_PlayListsSchedules->Delete(treeitem);
            }
            else if (IsSchedule(treeitem))
            {
                PlayList* playlist = (PlayList*)((MyTreeItemData*)TreeCtrl_PlayListsSchedules->GetItemData(TreeCtrl_PlayListsSchedules->GetItemParent(treeitem)))->GetData();
                Schedule* schedule = (Schedule*)((MyTreeItemData*)TreeCtrl_PlayListsSchedules->GetItemData(treeitem))->GetData();
                playlist->RemoveSchedule(schedule);
                delete schedule;
                TreeCtrl_PlayListsSchedules->Delete(treeitem);
            }
            TreeCtrl_PlayListsSchedules->SelectItem(parent);
        }
    }
}

void xScheduleFrame::OnTreeCtrl_PlayListsSchedulesSelectionChanged(wxTreeEvent& event)
{
}

void xScheduleFrame::OnTreeCtrl_PlayListsSchedulesKeyDown(wxTreeEvent& event)
{
    if (event.GetKeyCode() == WXK_DELETE)
    {
        DeleteSelectedItem();
    }
}

void xScheduleFrame::OnMenuItem_SaveSelected(wxCommandEvent& event)
{
    _schedule->Save();
}

void xScheduleFrame::OnMenuItem_ShowFolderSelected(wxCommandEvent& event)
{
    DirDialog1->SetPath(_showDir);

    if (DirDialog1->ShowModal() == wxID_OK)
    {
        delete _schedule;
        _showDir = DirDialog1->GetPath().ToStdString();
        SaveShowDir();
        _schedule = new ScheduleManager(_showDir);
        UpdateTree();
    }
}

void xScheduleFrame::SaveShowDir() const
{
    wxConfigBase* config = wxConfigBase::Get();
    config->Write("SchedulerLastDir", wxString(_showDir));
}

void xScheduleFrame::LoadShowDir()
{
    // get the show directory
    wxConfigBase* config = wxConfigBase::Get();
    wxString showDir;
    if (!config->Read("SchedulerLastDir", &showDir))
    {
        if (!config->Read("LastDir", &showDir))
        {
            DirDialog1->SetPath(_showDir);

            if (DirDialog1->ShowModal() == wxID_OK)
            {
                _showDir = DirDialog1->GetPath().ToStdString();
                SaveShowDir();
            }
            else
            {
                _showDir = ".";
            }
        }
        else
        {
            _showDir = showDir.ToStdString();
            SaveShowDir();
        }
    }
    else
    {
        _showDir = showDir.ToStdString();
    }
}

void xScheduleFrame::UpdateTree() const
{
    TreeCtrl_PlayListsSchedules->DeleteAllItems();

    wxTreeItemId root = TreeCtrl_PlayListsSchedules->AddRoot("Playlists");

    auto pls = _schedule->GetPlayLists();

    for (auto it = pls.begin(); it != pls.end(); ++it)
    {
        auto pl = TreeCtrl_PlayListsSchedules->AppendItem(root, (*it)->GetName(), -1, -1, new MyTreeItemData(*it));

        auto schedules = (*it)->GetSchedules();
        for (auto it2 = schedules.begin(); it2 != schedules.end(); ++it2)
        {
            TreeCtrl_PlayListsSchedules->AppendItem(pl, (*it2)->GetName(), -1, -1, new MyTreeItemData(*it2));
        }
        TreeCtrl_PlayListsSchedules->Expand(pl);
    }
    TreeCtrl_PlayListsSchedules->Expand(root);
}


void xScheduleFrame::OnTreeCtrl_PlayListsSchedulesItemActivated(wxTreeEvent& event)
{
    wxTreeItemId treeitem = TreeCtrl_PlayListsSchedules->GetSelection();
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
        Schedule* schedule = (Schedule*)((MyTreeItemData*)TreeCtrl_PlayListsSchedules->GetItemData(treeitem))->GetData();
        if (schedule->Configure(this) != nullptr)
        {
            TreeCtrl_PlayListsSchedules->SetItemText(treeitem, schedule->GetName());
        }
    }
}

void xScheduleFrame::On_timerTrigger(wxTimerEvent& event)
{
    _schedule->Frame();
    StaticText_Status->SetLabel(_schedule->GetStatus());
}

void xScheduleFrame::On_timerScheduleTrigger(wxTimerEvent& event)
{
    int rate = _schedule->CheckSchedule();

    // if the desired rate is different than the current rate then restart timer with the desired rate
    if (_timer.GetInterval() != rate)
    {
        _timer.Stop();
        _timer.Start(rate);
    }
}
