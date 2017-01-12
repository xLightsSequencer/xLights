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
#include "ScheduleOptions.h"
#include "OptionsDialog.h"
#include "WebServer.h"
#include <log4cpp/Category.hh>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/mimetype.h>
#include "PlayList/PlayListStep.h"
#include <wx/bitmap.h>
#include "../xLights/xLightsVersion.h"
#include <wx/protocol/http.h>

#include "../include/xs_save.xpm"
#include "../include/xs_otlon.xpm"
#include "../include/xs_otloff.xpm"
#include "../include/xs_scheduled.xpm"
#include "../include/xs_notscheduled.xpm"
#include "../include/xs_inactive.xpm"
#include "../include/xs_pllooped.xpm"
#include "../include/xs_plnotlooped.xpm"
#include "../include/xs_plsteplooped.xpm"
#include "../include/xs_plstepnotlooped.xpm"
#include "../include/xs_playing.xpm"
#include "../include/xs_idle.xpm"
#include "../include/xs_paused.xpm"
#include "../include/xs_random.xpm"
#include "../include/xs_notrandom.xpm"

//(*InternalHeaders(xScheduleFrame)
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/debugrpt.h>
//*)

ScheduleManager* xScheduleFrame::__schedule = nullptr;

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
const long xScheduleFrame::ID_BITMAPBUTTON1 = wxNewId();
const long xScheduleFrame::ID_BITMAPBUTTON2 = wxNewId();
const long xScheduleFrame::ID_BITMAPBUTTON3 = wxNewId();
const long xScheduleFrame::ID_BITMAPBUTTON4 = wxNewId();
const long xScheduleFrame::ID_BITMAPBUTTON5 = wxNewId();
const long xScheduleFrame::ID_BITMAPBUTTON6 = wxNewId();
const long xScheduleFrame::ID_BITMAPBUTTON7 = wxNewId();
const long xScheduleFrame::ID_PANEL2 = wxNewId();
const long xScheduleFrame::ID_TREECTRL1 = wxNewId();
const long xScheduleFrame::ID_PANEL3 = wxNewId();
const long xScheduleFrame::ID_LISTVIEW1 = wxNewId();
const long xScheduleFrame::ID_PANEL5 = wxNewId();
const long xScheduleFrame::ID_SPLITTERWINDOW1 = wxNewId();
const long xScheduleFrame::ID_PANEL1 = wxNewId();
const long xScheduleFrame::ID_STATICTEXT1 = wxNewId();
const long xScheduleFrame::ID_STATICTEXT2 = wxNewId();
const long xScheduleFrame::ID_PANEL4 = wxNewId();
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
const long xScheduleFrame::ID_MNU_DUPLICATEPLAYLIST = wxNewId();
const long xScheduleFrame::ID_MNU_SCHEDULEPLAYLIST = wxNewId();
const long xScheduleFrame::ID_MNU_DELETE = wxNewId();
const long xScheduleFrame::ID_MNU_EDIT = wxNewId();
const long xScheduleFrame::ID_MNU_PLAYNOW = wxNewId();
const long xScheduleFrame::ID_BUTTON_USER = wxNewId();

wxDEFINE_EVENT(EVT_FRAMEMS, wxCommandEvent);

BEGIN_EVENT_TABLE(xScheduleFrame,wxFrame)
    //(*EventTable(xScheduleFrame)
    //*)
    EVT_COMMAND(wxID_ANY, EVT_FRAMEMS, xScheduleFrame::RateNotification)
END_EVENT_TABLE()

xScheduleFrame::xScheduleFrame(wxWindow* parent,wxWindowID id)
{
    __schedule = nullptr;

    //(*Initialize(xScheduleFrame)
    wxMenuItem* MenuItem2;
    wxFlexGridSizer* FlexGridSizer3;
    wxMenuItem* MenuItem1;
    wxFlexGridSizer* FlexGridSizer5;
    wxFlexGridSizer* FlexGridSizer2;
    wxMenu* Menu1;
    wxMenuBar* MenuBar1;
    wxFlexGridSizer* FlexGridSizer6;
    wxMenu* Menu2;

    Create(parent, id, _("xLights Scheduler"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("id"));
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer1->AddGrowableRow(1);
    Panel2 = new wxPanel(this, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
    FlexGridSizer5 = new wxFlexGridSizer(0, 7, 0, 0);
    BitmapButton_OutputToLights = new wxBitmapButton(Panel2, ID_BITMAPBUTTON1, wxNullBitmap, wxDefaultPosition, wxSize(32,32), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON1"));
    FlexGridSizer5->Add(BitmapButton_OutputToLights, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_Random = new wxBitmapButton(Panel2, ID_BITMAPBUTTON2, wxNullBitmap, wxDefaultPosition, wxSize(32,32), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON2"));
    FlexGridSizer5->Add(BitmapButton_Random, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_Playing = new wxBitmapButton(Panel2, ID_BITMAPBUTTON3, wxNullBitmap, wxDefaultPosition, wxSize(32,32), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON3"));
    FlexGridSizer5->Add(BitmapButton_Playing, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_PLLoop = new wxBitmapButton(Panel2, ID_BITMAPBUTTON4, wxNullBitmap, wxDefaultPosition, wxSize(32,32), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON4"));
    FlexGridSizer5->Add(BitmapButton_PLLoop, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_StepLoop = new wxBitmapButton(Panel2, ID_BITMAPBUTTON5, wxNullBitmap, wxDefaultPosition, wxSize(32,32), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON5"));
    FlexGridSizer5->Add(BitmapButton_StepLoop, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_IsScheduled = new wxBitmapButton(Panel2, ID_BITMAPBUTTON6, wxNullBitmap, wxDefaultPosition, wxSize(32,32), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON6"));
    FlexGridSizer5->Add(BitmapButton_IsScheduled, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_Unsaved = new wxBitmapButton(Panel2, ID_BITMAPBUTTON7, wxNullBitmap, wxDefaultPosition, wxSize(32,32), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON7"));
    FlexGridSizer5->Add(BitmapButton_Unsaved, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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
    ListView_Running = new wxListView(Panel5, ID_LISTVIEW1, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL|wxLC_NO_SORT_HEADER, wxDefaultValidator, _T("ID_LISTVIEW1"));
    ListView_Running->SetMinSize(wxSize(300,300));
    FlexGridSizer3->Add(ListView_Running, 1, wxALL|wxEXPAND, 5);
    Panel5->SetSizer(FlexGridSizer3);
    FlexGridSizer3->Fit(Panel5);
    FlexGridSizer3->SetSizeHints(Panel5);
    SplitterWindow1->SplitVertically(Panel3, Panel5);
    FlexGridSizer1->Add(SplitterWindow1, 1, wxALL|wxEXPAND, 5);
    Panel1 = new wxPanel(this, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    FlexGridSizer4 = new wxFlexGridSizer(0, 0, 0, 0);
    Panel1->SetSizer(FlexGridSizer4);
    FlexGridSizer4->Fit(Panel1);
    FlexGridSizer4->SetSizeHints(Panel1);
    FlexGridSizer1->Add(Panel1, 1, wxALL|wxEXPAND, 5);
    Panel4 = new wxPanel(this, ID_PANEL4, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL4"));
    FlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer6->AddGrowableCol(1);
    StaticText_ShowDir = new wxStaticText(Panel4, ID_STATICTEXT1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer6->Add(StaticText_ShowDir, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_Status = new wxStaticText(Panel4, ID_STATICTEXT2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer6->Add(StaticText_Status, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Panel4->SetSizer(FlexGridSizer6);
    FlexGridSizer6->Fit(Panel4);
    FlexGridSizer6->SetSizeHints(Panel4);
    FlexGridSizer1->Add(Panel4, 1, wxALL|wxEXPAND, 5);
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
    MenuItem_ViewLog = new wxMenuItem(Menu3, ID_MNU_VIEW_LOG, _("&View Log"), wxEmptyString, wxITEM_NORMAL);
    Menu3->Append(MenuItem_ViewLog);
    MenuItem6 = new wxMenuItem(Menu3, ID_MNU_CHECK_SCHEDULE, _("&Check Schedule"), wxEmptyString, wxITEM_NORMAL);
    Menu3->Append(MenuItem6);
    MenuItem_Options = new wxMenuItem(Menu3, ID_MNU_OPTIONS, _("&Options"), wxEmptyString, wxITEM_NORMAL);
    Menu3->Append(MenuItem_Options);
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

    Connect(ID_BITMAPBUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xScheduleFrame::OnBitmapButton_OutputToLightsClick);
    Connect(ID_BITMAPBUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xScheduleFrame::OnBitmapButton_RandomClick);
    Connect(ID_BITMAPBUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xScheduleFrame::OnBitmapButton_PlayingClick);
    Connect(ID_BITMAPBUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xScheduleFrame::OnBitmapButton_PLLoopClick);
    Connect(ID_BITMAPBUTTON5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xScheduleFrame::OnBitmapButton_StepLoopClick);
    Connect(ID_BITMAPBUTTON6,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xScheduleFrame::OnBitmapButton_IsScheduledClick);
    Connect(ID_BITMAPBUTTON7,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xScheduleFrame::OnBitmapButton_UnsavedClick);
    Connect(ID_TREECTRL1,wxEVT_COMMAND_TREE_ITEM_ACTIVATED,(wxObjectEventFunction)&xScheduleFrame::OnTreeCtrl_PlayListsSchedulesItemActivated);
    Connect(ID_TREECTRL1,wxEVT_COMMAND_TREE_SEL_CHANGED,(wxObjectEventFunction)&xScheduleFrame::OnTreeCtrl_PlayListsSchedulesSelectionChanged);
    Connect(ID_TREECTRL1,wxEVT_COMMAND_TREE_KEY_DOWN,(wxObjectEventFunction)&xScheduleFrame::OnTreeCtrl_PlayListsSchedulesKeyDown);
    Connect(ID_TREECTRL1,wxEVT_COMMAND_TREE_ITEM_MENU,(wxObjectEventFunction)&xScheduleFrame::OnTreeCtrl_PlayListsSchedulesItemMenu);
    Connect(ID_LISTVIEW1,wxEVT_COMMAND_LIST_ITEM_ACTIVATED,(wxObjectEventFunction)&xScheduleFrame::OnListView_RunningItemActivated);
    Connect(ID_MNU_SHOWFOLDER,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_ShowFolderSelected);
    Connect(ID_MNU_SAVE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_SaveSelected);
    Connect(idMenuQuit,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnQuit);
    Connect(ID_MNU_VIEW_LOG,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_ViewLogSelected);
    Connect(ID_MNU_OPTIONS,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_OptionsSelected);
    Connect(idMenuAbout,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnAbout);
    Connect(ID_TIMER1,wxEVT_TIMER,(wxObjectEventFunction)&xScheduleFrame::On_timerTrigger);
    Connect(ID_TIMER2,wxEVT_TIMER,(wxObjectEventFunction)&xScheduleFrame::On_timerScheduleTrigger);
    Connect(wxEVT_SIZE,(wxObjectEventFunction)&xScheduleFrame::OnResize);
    //*)

    Connect(wxID_ANY, EVT_FRAMEMS, (wxObjectEventFunction)&xScheduleFrame::RateNotification);

    ListView_Running->AppendColumn("Step");
    ListView_Running->AppendColumn("Duration");
    ListView_Running->AppendColumn("");

    _otlon = wxBitmap(xs_otlon, 32, 32);
    _otloff = wxBitmap(xs_otloff, 32, 32);
    _save = wxBitmap(xs_save, 32, 32);
    _scheduled = wxBitmap(xs_scheduled, 32, 32);
    _notscheduled = wxBitmap(xs_notscheduled, 32, 32);
    _inactive = wxBitmap(xs_inactive, 32, 32);
    _pllooped = wxBitmap(xs_pllooped, 32, 32);
    _plnotlooped = wxBitmap(xs_plnotlooped, 32, 32);
    _plsteplooped = wxBitmap(xs_plsteplooped, 32, 32);
    _plstepnotlooped = wxBitmap(xs_plstepnotlooped, 32, 32);
    _playing = wxBitmap(xs_playing, 32, 32);
    _idle = wxBitmap(xs_idle, 32, 32);
    _paused = wxBitmap(xs_paused, 32, 32);
    _random = wxBitmap(xs_random, 32, 32);
    _notrandom = wxBitmap(xs_notrandom, 32, 32);

    SetSize(600, 300);

    LoadShowDir();

    StaticText_ShowDir->SetLabel(_showDir);

    __schedule = new ScheduleManager(_showDir);

    _webServer = new WebServer(__schedule->GetOptions()->GetWebServerPort());

    UpdateTree();

    CreateButtons();

    ValidateWindow();
}

xScheduleFrame::~xScheduleFrame()
{
    //(*Destroy(xScheduleFrame)
    //*)

    delete _webServer;
    _webServer = nullptr;

    delete __schedule;
    __schedule = nullptr;
}

void xScheduleFrame::OnQuit(wxCommandEvent& event)
{
    Close();
}

void xScheduleFrame::OnAbout(wxCommandEvent& event)
{
    wxString msg = wxbuildinfo(long_f);
    wxMessageBox(msg, _("Welcome to..."));
    ValidateWindow();
}

bool xScheduleFrame::IsPlayList(wxTreeItemId id) const
{
    if (!id.IsOk()) return false;

    return (TreeCtrl_PlayListsSchedules->GetItemParent(id) == TreeCtrl_PlayListsSchedules->GetRootItem());
}

bool xScheduleFrame::IsSchedule(wxTreeItemId id) const
{
    if (!id.IsOk()) return false;

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
    wxMenuItem* duplicate = mnu.Append(ID_MNU_DUPLICATEPLAYLIST, "Duplicate");
    wxMenuItem* edit = mnu.Append(ID_MNU_EDIT, "Edit");
    wxMenuItem* play = mnu.Append(ID_MNU_PLAYNOW, "Play Now");

    if (!IsPlayList(treeitem) && !IsSchedule(treeitem))
    {
        del->Enable(false);
        edit->Enable(false);
    }

    if (!IsPlayList(treeitem))
    {
        duplicate->Enable(false);
        play->Enable(false);
    }

    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&xScheduleFrame::OnTreeCtrlMenu, nullptr, this);
    PopupMenu(&mnu);
    ValidateWindow();
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
            __schedule->AddPlayList(playlist);
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
        size_t rate = 50;
        __schedule->PlayPlayList(playlist, rate);

        // if the desired rate is different than the current rate then restart timer with the desired rate
        if (_timer.GetInterval() != rate)
        {
            _timer.Stop();
            _timer.Start(rate);
        }
    }
    else if (event.GetId() == ID_MNU_DUPLICATEPLAYLIST)
    {
        PlayList* playlist = (PlayList*)((MyTreeItemData*)TreeCtrl_PlayListsSchedules->GetItemData(treeitem))->GetData();

        PlayList* newpl = new PlayList(*playlist);

        wxTreeItemId  newitem = TreeCtrl_PlayListsSchedules->AppendItem(TreeCtrl_PlayListsSchedules->GetRootItem(), playlist->GetName(), -1, -1, new MyTreeItemData(newpl));
        TreeCtrl_PlayListsSchedules->Expand(newitem);
        TreeCtrl_PlayListsSchedules->EnsureVisible(newitem);
        __schedule->AddPlayList(newpl);
    }
    ValidateWindow();
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
                __schedule->RemovePlayList(playlist);
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
    ValidateWindow();
}

void xScheduleFrame::OnTreeCtrl_PlayListsSchedulesKeyDown(wxTreeEvent& event)
{
    if (event.GetKeyCode() == WXK_DELETE)
    {
        DeleteSelectedItem();
    }
    ValidateWindow();
}

void xScheduleFrame::OnMenuItem_SaveSelected(wxCommandEvent& event)
{
    __schedule->Save();
    ValidateWindow();
}

void xScheduleFrame::OnMenuItem_ShowFolderSelected(wxCommandEvent& event)
{
    DirDialog1->SetPath(_showDir);

    if (DirDialog1->ShowModal() == wxID_OK)
    {
        delete __schedule;
        __schedule = nullptr;
        _showDir = DirDialog1->GetPath().ToStdString();
        SaveShowDir();
        __schedule = new ScheduleManager(_showDir);
        UpdateTree();
    }
    ValidateWindow();
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

    auto pls = __schedule->GetPlayLists();

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
    ValidateWindow();
}

void xScheduleFrame::On_timerTrigger(wxTimerEvent& event)
{
    if (__schedule == nullptr) return;
    __schedule->Frame();

    UpdateStatus();

    ValidateWindow();
}

void xScheduleFrame::On_timerScheduleTrigger(wxTimerEvent& event)
{
    int rate = __schedule->CheckSchedule();

    // if the desired rate is different than the current rate then restart timer with the desired rate
    if (_timer.GetInterval() != rate)
    {
        _timer.Stop();
        _timer.Start(rate);
    }
    ValidateWindow();
}

void xScheduleFrame::ValidateWindow()
{
}

void xScheduleFrame::OnMenuItem_OptionsSelected(wxCommandEvent& event)
{
    OptionsDialog dlg(this, __schedule->GetOptions());

    int oldport = __schedule->GetOptions()->GetWebServerPort();

    if (dlg.ShowModal() == wxID_OK)
    {
        if (oldport != __schedule->GetOptions()->GetWebServerPort())
        {
#pragma todo not sure this is working ... I think it causes a crash
            delete _webServer;
            _webServer = new WebServer(__schedule->GetOptions()->GetWebServerPort());
        }

        __schedule->OptionsChanged();
        CreateButtons();
    }
}

void xScheduleFrame::CreateButtons()
{
    auto buttons = Panel1->GetChildren();
    for (auto it = buttons.begin(); it != buttons.end(); ++it)
    {
        Disconnect((*it)->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xScheduleFrame::OnButton_UserClick);
        FlexGridSizer4->Detach(*it);
        delete *it;
    }

    auto bs = __schedule->GetOptions()->GetButtons();
    for (auto it = bs.begin(); it != bs.end(); ++it)
    {
        wxButton* b = new wxButton(Panel1, ID_BUTTON_USER, *it);
        FlexGridSizer4->Add(b, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
        Connect(ID_BUTTON_USER, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xScheduleFrame::OnButton_UserClick);
    }

    FlexGridSizer4->Fit(Panel1);
    FlexGridSizer4->SetSizeHints(Panel1);
    Layout();
}

void xScheduleFrame::RateNotification(wxCommandEvent& event)
{
    if (_timer.GetInterval() != event.GetInt())
    {
        _timer.Stop();
        _timer.Start(event.GetInt());
    }
}

void xScheduleFrame::OnButton_UserClick(wxCommandEvent& event)
{
    PlayList* playlist = nullptr;

    wxTreeItemId treeitem = TreeCtrl_PlayListsSchedules->GetSelection();

    if (IsPlayList(treeitem))
    {
        playlist = (PlayList*)((MyTreeItemData*)TreeCtrl_PlayListsSchedules->GetItemData(treeitem))->GetData();
    }

    size_t rate = _timer.GetInterval();
    std::string msg = "";
    __schedule->Action(((wxButton*)event.GetEventObject())->GetLabel().ToStdString(), playlist, rate, msg);

    if (rate != _timer.GetInterval())
    {
        _timer.Stop();
        _timer.Start(rate);
    }

    ValidateWindow();
}


void xScheduleFrame::OnMenuItem_ViewLogSelected(wxCommandEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxString dir;
    wxString fileName = "xSchedule_l4cpp.log";
#ifdef __WXMSW__
    wxGetEnv("APPDATA", &dir);
    wxString filename = dir + "/" + fileName;
#endif
#ifdef __WXOSX_MAC__
    wxFileName home;
    home.AssignHomeDir();
    dir = home.GetFullPath();
    wxString filename = dir + "/Library/Logs/" + fileName;
#endif
#ifdef __LINUX__
    wxString filename = "/tmp/" + fileName;
#endif
    wxString fn = "";
    if (wxFile::Exists(filename))
    {
        fn = filename;
    }
    else if (wxFile::Exists(wxFileName(_showDir, fileName).GetFullPath()))
    {
        fn = wxFileName(_showDir, fileName).GetFullPath();
    }
    else if (wxFile::Exists(wxFileName(wxGetCwd(), fileName).GetFullPath()))
    {
        fn = wxFileName(wxGetCwd(), fileName).GetFullPath();
    }

    wxFileType *ft = wxTheMimeTypesManager->GetFileTypeFromExtension("txt");
    if (fn != "" && ft)
    {
        wxString command = ft->GetOpenCommand("foo.txt");
        command.Replace("foo.txt", fn);

        logger_base.debug("Viewing log file %s.", (const char *)fn.c_str());

        wxExecute(command);
        delete ft;
    }
    else
    {
        logger_base.warn("Unable to view log file %s.", (const char *)fn.c_str());
        wxMessageBox(_("Unable to show log file."), _("Error"));
    }
}

void xScheduleFrame::OnResize(wxSizeEvent& event)
{
    bool done = false;

    int pw, ph;
    Panel1->GetSize(&pw, &ph);

    int n = 20;
    while (!done && n > 0)
    {
        auto buttons = Panel1->GetChildren();
        FlexGridSizer4->SetRows(buttons.size() / n + (buttons.size() % n > 0 ? 1 : 0));
        FlexGridSizer4->SetCols(n);
        FlexGridSizer4->Fit(Panel1);
        FlexGridSizer4->SetSizeHints(Panel1);

        bool changed = false;

        int lastx = 0;
        int lasty = 0;
        for (auto it = buttons.begin(); it != buttons.end(); ++it)
        {
            int x, y, w, h;
            (*it)->GetPosition(&x, &y);
            (*it)->GetSize(&w, &h);

            if ((x < lastx && y == lasty) || x+w > pw - 10)
            {
                n--;
                changed = true;
                lastx = 0;
                lasty = y;
                break;
            }

            lasty = y;
            lastx = x + w;
        }

        if (!changed)
        {
            break;
        }
    }

    Layout();
}

void xScheduleFrame::OnListView_RunningItemActivated(wxListEvent& event)
{
    int selected = ListView_Running->GetFirstSelected();

    PlayList* p = __schedule->GetRunningPlayList();

    if (selected > 0 && p != nullptr && p->GetRunningStep()->GetNameNoTime() != ListView_Running->GetItemText(selected, 0))
    {
        size_t rate;
        std::string msg;
        __schedule->Action("Jump to specified step in current playlist", ListView_Running->GetItemText(selected, 0).ToStdString(), p, rate, msg);
    }
}

std::string FormatTime(size_t timems, bool ms = false)
{
    if (ms)
    {
        return wxString::Format(wxT("%i:%02i.%03i"), timems / 60000, (timems % 60000) / 1000, timems % 1000).ToStdString();
    }
    else
    {
        return wxString::Format(wxT("%i:%02i"), timems / 60000, (timems % 60000) / 1000).ToStdString();
    }
}

void xScheduleFrame::UpdateStatus()
{
    PlayList* last = nullptr;
    PlayList* p = __schedule->GetRunningPlayList();

    if (p == nullptr)
    {
        wxTreeItemId treeitem = TreeCtrl_PlayListsSchedules->GetSelection();
        if (treeitem.IsOk() && IsPlayList(treeitem))
        {
            p = (PlayList*)((MyTreeItemData*)TreeCtrl_PlayListsSchedules->GetItemData(treeitem))->GetData();
        }
    }

    if (p == nullptr)
    {
        ListView_Running->DeleteAllItems();
    }
    else
    {
        if (p != last)
        {
            last = p;

            ListView_Running->DeleteAllItems();

            auto steps = p->GetSteps();

            int i = 0;
            for (auto it = steps.begin(); it != steps.end(); ++it)
            {
                ListView_Running->InsertItem(i, (*it)->GetNameNoTime());
                ListView_Running->SetItem(i, 1, FormatTime((*it)->GetLengthMS()));
                i++;
            }
        }

        PlayListStep* step = p->GetRunningStep();
        PlayListStep* next = nullptr;

        if (!p->IsRandom())
        {
            next = p->GetNextStep();
        }

        if (step != nullptr)
        {
            for (int i = 0; i < ListView_Running->GetItemCount(); i++)
            {
                if (ListView_Running->GetItemText(i, 0) == step->GetNameNoTime())
                {
                    ListView_Running->SetItem(i, 2, step->GetStatus());
                    ListView_Running->SetItemBackgroundColour(i, wxColor(146,244,155));
                }
                else
                {
                    if (next != nullptr && next->GetNameNoTime() == ListView_Running->GetItemText(i,0))
                    {
                        ListView_Running->SetItemBackgroundColour(i, wxColor(244,241,146));
                    }
                    else
                    {
                        ListView_Running->SetItemBackgroundColour(i, *wxWHITE);
                    }
                    ListView_Running->SetItem(i, 2, "");
                }
            }
        }

        ListView_Running->SetColumnWidth(0, wxLIST_AUTOSIZE);
        ListView_Running->SetColumnWidth(1, wxLIST_AUTOSIZE);
        ListView_Running->SetColumnWidth(2, wxLIST_AUTOSIZE);
    }

    static int saved = -1;
    static int otl = -1;
    static int scheduled = -1;
    static int random = -1;
    static int plloop = -1;
    static int steploop = -1;
    static int playing = -1;

    if (__schedule->IsOutputToLights())
    {
        if (otl != 1)
            BitmapButton_OutputToLights->SetBitmap(_otlon);
        BitmapButton_OutputToLights->SetToolTip("Lights output ON.");
        otl = 1;
    }
    else
    {
        if (otl != 0)
            BitmapButton_OutputToLights->SetBitmap(_otloff);
        BitmapButton_OutputToLights->SetToolTip("Lights output OFF.");
        otl = 0;
    }

    if (__schedule->IsDirty())
    {
        if (saved != 1)
            BitmapButton_Unsaved->SetBitmap(_save);
        BitmapButton_Unsaved->SetToolTip("Unsaved changes.");
        saved = 1;
    }
    else
    {
        if (saved != 0)
            BitmapButton_Unsaved->SetBitmap(_inactive);
        saved = 0;
    }

    if (p == nullptr)
    {
        if (scheduled != 0)
            BitmapButton_IsScheduled->SetBitmap(_inactive);
        scheduled = 0;

        if (playing != 0)
            BitmapButton_Playing->SetBitmap(_idle);
        BitmapButton_Playing->SetToolTip("Idle.");
        playing = 0;

        if (plloop != 2)
            BitmapButton_PLLoop->SetBitmap(_inactive);
        plloop = 2;

        if (steploop != 2)
            BitmapButton_StepLoop->SetBitmap(_inactive);
        steploop = 2;

        if (random != 2)
            BitmapButton_Random->SetBitmap(_inactive);
        random = 2;
    }
    else
    {
        if (__schedule->IsCurrentPlayListScheduled())
        {
            if (scheduled != 1)
                BitmapButton_IsScheduled->SetBitmap(_scheduled);
            BitmapButton_IsScheduled->SetToolTip("Scheduled playlist playing.");
            scheduled = 1;
        }
        else
        {
            if (scheduled != 2)
                BitmapButton_IsScheduled->SetBitmap(_notscheduled);
            BitmapButton_IsScheduled->SetToolTip("Manually started playlist playing.");
            scheduled = 2;
        }

        if (p->IsPaused())
        {
            if (playing != 2)
                BitmapButton_Playing->SetBitmap(_paused);
            BitmapButton_Playing->SetToolTip("Paused.");
            playing = 2;
        }
        else
        {
            if (playing != 1)
                BitmapButton_Playing->SetBitmap(_playing);
            BitmapButton_Playing->SetToolTip("Playing.");
            playing = 1;
        }

        if (p->IsLooping())
        {
            if (plloop != 1)
                BitmapButton_PLLoop->SetBitmap(_pllooped);
            BitmapButton_PLLoop->SetToolTip("Playlist looping.");
            plloop = 1;
        }
        else
        {
            if (plloop != 0)
                BitmapButton_PLLoop->SetBitmap(_plnotlooped);
            BitmapButton_PLLoop->SetToolTip("Playlist not looping.");
            plloop = 0;
        }

        if (p->IsStepLooping())
        {
            if (steploop != 1)
                BitmapButton_StepLoop->SetBitmap(_plsteplooped);
            BitmapButton_StepLoop->SetToolTip("Playlist step looping.");
            steploop = 1;
        }
        else
        {
            if (steploop != 0)
                BitmapButton_StepLoop->SetBitmap(_plstepnotlooped);
            BitmapButton_StepLoop->SetToolTip("Playlist step not looping.");
            steploop = 0;
        }

        if (p->IsRandom())
        {
            if (random != 1)
                BitmapButton_Random->SetBitmap(_random);
            BitmapButton_Random->SetToolTip("Randomly choosing steps.");
            random = 1;
        }
        else
        {
            if (random != 0)
                BitmapButton_Random->SetBitmap(_notrandom);
            BitmapButton_Random->SetToolTip("Sequentially played steps.");
            random = 0;
        }
    }
}

void xScheduleFrame::OnBitmapButton_OutputToLightsClick(wxCommandEvent& event)
{
    std::string msg = "";
    __schedule->ToggleOutputToLights(msg);
}

void xScheduleFrame::OnBitmapButton_RandomClick(wxCommandEvent& event)
{
    std::string msg = "";
    __schedule->ToggleCurrentPlayListRandom(msg);
}

void xScheduleFrame::OnBitmapButton_PlayingClick(wxCommandEvent& event)
{
    std::string msg = "";
    __schedule->ToggleCurrentPlayListPause(msg);
}

void xScheduleFrame::OnBitmapButton_PLLoopClick(wxCommandEvent& event)
{
    std::string msg = "";
    __schedule->ToggleCurrentPlayListLoop(msg);
}

void xScheduleFrame::OnBitmapButton_StepLoopClick(wxCommandEvent& event)
{
    std::string msg = "";
    __schedule->ToggleCurrentPlayListStepLoop(msg);
}

void xScheduleFrame::OnBitmapButton_IsScheduledClick(wxCommandEvent& event)
{
    // do nothing
}

void xScheduleFrame::OnBitmapButton_UnsavedClick(wxCommandEvent& event)
{
    __schedule->Save();
}

void xScheduleFrame::CreateDebugReport(wxDebugReportCompress *report) {
    if (wxDebugReportPreviewStd().Show(*report)) {
        report->Process();
        SendReport("crashUpload", *report);
        wxMessageBox("Crash report saved to " + report->GetCompressedFileName());
    }
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.crit("Exiting after creating debug report: " + report->GetCompressedFileName());
    delete report;
    exit(1);
}

void xScheduleFrame::SendReport(const wxString &loc, wxDebugReportCompress &report) {
    wxHTTP http;
    http.Connect("dankulp.com");

    const char *bound = "--------------------------b29a7c2fe47b9481";
    int i = wxGetUTCTimeMillis().GetLo();
    i &= 0xFFFFFFF;
    wxString fn = wxString::Format("xschedule-%s_%d_%s.zip", wxPlatformInfo::Get().GetOperatingSystemFamilyName().c_str(), i, xlights_version_string);
    const char *ct = "Content-Type: application/octet-stream\n";
    std::string cd = "Content-Disposition: form-data; name=\"userfile\"; filename=\"" + fn.ToStdString() + "\"\n\n";

    wxMemoryBuffer memBuff;
    memBuff.AppendData(bound, strlen(bound));
    memBuff.AppendData("\n", 1);
    memBuff.AppendData(ct, strlen(ct));
    memBuff.AppendData(cd.c_str(), strlen(cd.c_str()));


    wxFile f_in(report.GetCompressedFileName());
    wxFileOffset fLen = f_in.Length();
    void* tmp = memBuff.GetAppendBuf(fLen);
    size_t iRead = f_in.Read(tmp, fLen);
    memBuff.UngetAppendBuf(iRead);
    f_in.Close();

    memBuff.AppendData("\n", 1);
    memBuff.AppendData(bound, strlen(bound));
    memBuff.AppendData("--\n", 3);

    http.SetMethod("POST");
    http.SetPostBuffer("multipart/form-data; boundary=------------------------b29a7c2fe47b9481", memBuff);
    wxInputStream * is = http.GetInputStream("/" + loc + "/index.php");
    char buf[1024];
    is->Read(buf, 1024);
    //printf("%s\n", buf);
    delete is;
    http.Close();
}
