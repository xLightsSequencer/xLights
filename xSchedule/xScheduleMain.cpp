/***************************************************************
 * Name:      xScheduleMain.cpp
 * Purpose:   Code for Application Frame
 * Author:    xLights ()
 * Created:   2016-12-30
 * Copyright: xLights (http://xlights.org)
 * License:
 **************************************************************/

//#define TRACE_SCHEDULE_PERFORMANCE 1

#define ZERO 0

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
#include <wx/debugrpt.h>
#include "RunningSchedule.h"
#include "UserButton.h"
#include "OutputProcessingDialog.h"
#include <wx/clipbrd.h>
#include "../xLights/osxMacUtils.h"
#include "BackgroundPlaylistDialog.h"
#include "MatricesDialog.h"
#include "VirtualMatricesDialog.h"
#include "FPPRemotesDialog.h"
#include "ConfigureOSC.h"
#include "Pinger.h"
#include "EventsDialog.h"
#include "../xLights/outputs/IPOutput.h"
#include "PlayList/PlayListItemOSC.h"
#include "../xLights/UtilFunctions.h"
#include "City.h"

//#include "../include/xs_xyzzy.xpm"
#include "../include/xs_save.xpm"
#include "../include/xs_otlon.xpm"
#include "../include/xs_otloff.xpm"
#include "../include/xs_otlautoon.xpm"
#include "../include/xs_otlautooff.xpm"
#include "../include/xs_scheduled.xpm"
#include "../include/xs_queued.xpm"
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
#include "../include/xs_volume_down.xpm"
#include "../include/xs_volume_up.xpm"
#include "../include/xs_falcon.xpm"

#include "../include/xLights.xpm"
#include "../include/xLights-16.xpm"
#include "../include/xLights-32.xpm"
#include "../include/xLights-64.xpm"
#include "../include/xLights-128.xpm"

#include "../include/web_icon.xpm"
#include "../include/no_web_icon.xpm"

//(*InternalHeaders(xScheduleFrame)
#include <wx/intl.h>
#include <wx/string.h>
#include "ConfigureMIDITimecodeDialog.h"
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
const long xScheduleFrame::ID_BITMAPBUTTON3 = wxNewId();
const long xScheduleFrame::ID_BITMAPBUTTON6 = wxNewId();
const long xScheduleFrame::ID_BITMAPBUTTON2 = wxNewId();
const long xScheduleFrame::ID_BITMAPBUTTON4 = wxNewId();
const long xScheduleFrame::ID_BITMAPBUTTON5 = wxNewId();
const long xScheduleFrame::ID_BITMAPBUTTON7 = wxNewId();
const long xScheduleFrame::ID_CUSTOM1 = wxNewId();
const long xScheduleFrame::ID_BITMAPBUTTON8 = wxNewId();
const long xScheduleFrame::ID_CUSTOM2 = wxNewId();
const long xScheduleFrame::ID_BITMAPBUTTON9 = wxNewId();
const long xScheduleFrame::ID_PANEL2 = wxNewId();
const long xScheduleFrame::ID_TREECTRL1 = wxNewId();
const long xScheduleFrame::ID_BUTTON1 = wxNewId();
const long xScheduleFrame::ID_BUTTON2 = wxNewId();
const long xScheduleFrame::ID_BUTTON3 = wxNewId();
const long xScheduleFrame::ID_BUTTON4 = wxNewId();
const long xScheduleFrame::ID_PANEL6 = wxNewId();
const long xScheduleFrame::ID_LISTVIEW2 = wxNewId();
const long xScheduleFrame::ID_PANEL7 = wxNewId();
const long xScheduleFrame::ID_SPLITTERWINDOW2 = wxNewId();
const long xScheduleFrame::ID_PANEL3 = wxNewId();
const long xScheduleFrame::ID_LISTVIEW1 = wxNewId();
const long xScheduleFrame::ID_PANEL5 = wxNewId();
const long xScheduleFrame::ID_SPLITTERWINDOW1 = wxNewId();
const long xScheduleFrame::ID_PANEL1 = wxNewId();
const long xScheduleFrame::ID_STATICTEXT1 = wxNewId();
const long xScheduleFrame::ID_STATICTEXT3 = wxNewId();
const long xScheduleFrame::ID_STATICTEXT4 = wxNewId();
const long xScheduleFrame::ID_STATICTEXT5 = wxNewId();
const long xScheduleFrame::ID_STATICTEXT2 = wxNewId();
const long xScheduleFrame::ID_STATICBITMAP1 = wxNewId();
const long xScheduleFrame::ID_PANEL4 = wxNewId();
const long xScheduleFrame::ID_MNU_SHOWFOLDER = wxNewId();
const long xScheduleFrame::ID_MNU_SAVE = wxNewId();
const long xScheduleFrame::idMenuQuit = wxNewId();
const long xScheduleFrame::ID_MNU_MNUADDPLAYLIST = wxNewId();
const long xScheduleFrame::ID_MENUITEM1 = wxNewId();
const long xScheduleFrame::ID_MNU_BACKGROUND = wxNewId();
const long xScheduleFrame::ID_MNU_MATRICES = wxNewId();
const long xScheduleFrame::ID_MNU_VIRTUALMATRICES = wxNewId();
const long xScheduleFrame::ID_MNU_EDITEVENTS = wxNewId();
const long xScheduleFrame::ID_MNU_OPTIONS = wxNewId();
const long xScheduleFrame::ID_MNU_VIEW_LOG = wxNewId();
const long xScheduleFrame::ID_MNU_CHECK_SCHEDULE = wxNewId();
const long xScheduleFrame::ID_MNU_WEBINTERFACE = wxNewId();
const long xScheduleFrame::ID_MNU_IMPORT = wxNewId();
const long xScheduleFrame::ID_MNU_CRASH = wxNewId();
const long xScheduleFrame::ID_MNU_MODENORMAL = wxNewId();
const long xScheduleFrame::ID_MNU_FPPMASTER = wxNewId();
const long xScheduleFrame::ID_MNU_OSCMASTER = wxNewId();
const long xScheduleFrame::ID_MNU_OSCFPPMASTER = wxNewId();
const long xScheduleFrame::IDM_MNU_ARTNETMASTER = wxNewId();
const long xScheduleFrame::MNU_MIDITIMECODE_MASTER = wxNewId();
const long xScheduleFrame::ID_MNU_FPPREMOTE = wxNewId();
const long xScheduleFrame::ID_MNU_OSCREMOTE = wxNewId();
const long xScheduleFrame::ID_MNU_ARTNETTIMECODESLAVE = wxNewId();
const long xScheduleFrame::MNU_MIDITIMECODEREMOTE = wxNewId();
const long xScheduleFrame::ID_MNU_FPPUNICASTREMOTE = wxNewId();
const long xScheduleFrame::ID_MNU_EDITFPPREMOTE = wxNewId();
const long xScheduleFrame::ID_MNU_OSCOPTION = wxNewId();
const long xScheduleFrame::MNU_CONFIGUREMIDITIMECODE = wxNewId();
const long xScheduleFrame::idMenuAbout = wxNewId();
const long xScheduleFrame::ID_STATUSBAR1 = wxNewId();
const long xScheduleFrame::ID_TIMER1 = wxNewId();
const long xScheduleFrame::ID_TIMER2 = wxNewId();
//*)

const long xScheduleFrame::ID_MNU_ADDPLAYLIST = wxNewId();
const long xScheduleFrame::ID_MNU_ADDADVPLAYLIST = wxNewId();
const long xScheduleFrame::ID_MNU_DUPLICATEPLAYLIST = wxNewId();
const long xScheduleFrame::ID_MNU_SCHEDULEPLAYLIST = wxNewId();
const long xScheduleFrame::ID_MNU_DELETE = wxNewId();
const long xScheduleFrame::ID_MNU_EDIT = wxNewId();
const long xScheduleFrame::ID_MNU_EDITADV = wxNewId();
const long xScheduleFrame::ID_BUTTON_USER = wxNewId();

class BrightnessControl : public wxControl
{
    int _value;

public:

    BrightnessControl(wxWindow *parent,
        wxWindowID id,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = 0,
        const wxValidator& validator = wxDefaultValidator,
        const wxString& name = wxStaticBitmapNameStr) : wxControl(parent, id, pos, size, style, validator, name)
    {
        _value = 100;
        Connect(wxEVT_PAINT, (wxObjectEventFunction)&BrightnessControl::OnPaint);
    }
    virtual ~BrightnessControl() {};
    virtual void SetValue(int value) {
        if (_value != value) { _value = value; Refresh(); }
    }
protected:
    void OnPaint(wxPaintEvent& event)
    {
        wxPaintDC dc(this);
        int grey = (_value * 255) / 100;
        wxColor c(grey, grey, grey, 255);
        dc.SetBrush(wxBrush(c));
        int w, h;
        GetSize(&w, &h);
        dc.DrawRectangle(0, 0, w, h);
        if (_value < 60)
        {
            dc.SetTextForeground(*wxWHITE);
        }
        else
        {
            dc.SetTextForeground(*wxBLACK);
        }
        wxString text = wxString::Format("%i%%", _value);
        int tw, th;
        dc.GetTextExtent(text, &tw, &th);
        dc.DrawText(text, (32 - tw) / 2, (32 - th)/ 2);
        dc.SetBrush(wxNullBrush);
        dc.SetPen(wxNullPen);
    }

private:
    //wxDECLARE_DYNAMIC_CLASS(BrightnessControl);
    wxDECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(BrightnessControl, wxControl)
//(*EventTable(PerspectivesPanel)
//*)
END_EVENT_TABLE()

class VolumeDisplay : public wxControl
{
    int _value;

public:

    VolumeDisplay(wxWindow *parent,
        wxWindowID id,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = 0,
        const wxValidator& validator = wxDefaultValidator,
        const wxString& name = wxStaticBitmapNameStr) : wxControl(parent, id, pos, size, style, validator, name)
    {
        _value = 100;
        Connect(wxEVT_PAINT, (wxObjectEventFunction)&VolumeDisplay::OnPaint);
    }
    virtual ~VolumeDisplay() {};
    virtual void SetValue(int value) {
        if (_value != value) { _value = value; Refresh(); }
    }
protected:
    void OnPaint(wxPaintEvent& event)
    {
        wxPaintDC dc(this);
        dc.SetBrush(wxBrush(*wxLIGHT_GREY));
        int w, h;
        GetSize(&w, &h);
        dc.DrawRectangle(0, 0, w, h);
        dc.SetBrush(wxBrush(*wxYELLOW));
        dc.DrawRectangle(0, h - ((h * _value) / 100), w, h);
        dc.SetTextForeground(*wxBLACK);
        wxString text = wxString::Format("%i%%", _value);
        int tw, th;
        dc.GetTextExtent(text, &tw, &th);
        dc.DrawText(text, (32 - tw) / 2, (32 - th) / 2);
        dc.SetBrush(wxNullBrush);
        dc.SetPen(wxNullPen);
    }

private:
    //wxDECLARE_DYNAMIC_CLASS(VolumeDisplay);
    wxDECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(VolumeDisplay, wxControl)
//(*EventTable(PerspectivesPanel)
//*)
END_EVENT_TABLE()

wxDEFINE_EVENT(EVT_FRAMEMS, wxCommandEvent);
wxDEFINE_EVENT(EVT_STATUSMSG, wxCommandEvent);
wxDEFINE_EVENT(EVT_RUNACTION, wxCommandEvent);
wxDEFINE_EVENT(EVT_SCHEDULECHANGED, wxCommandEvent);
wxDEFINE_EVENT(EVT_DOCHECKSCHEDULE, wxCommandEvent);
wxDEFINE_EVENT(EVT_DOACTION, wxCommandEvent);
wxDEFINE_EVENT(EVT_XYZZY, wxCommandEvent);
wxDEFINE_EVENT(EVT_XYZZYEVENT, wxCommandEvent);

BEGIN_EVENT_TABLE(xScheduleFrame,wxFrame)
    //(*EventTable(xScheduleFrame)
    //*)
    EVT_COMMAND(wxID_ANY, EVT_FRAMEMS, xScheduleFrame::RateNotification)
    EVT_COMMAND(wxID_ANY, EVT_STATUSMSG, xScheduleFrame::StatusMsgNotification)
    EVT_COMMAND(wxID_ANY, EVT_RUNACTION, xScheduleFrame::RunAction)
    EVT_COMMAND(wxID_ANY, EVT_SCHEDULECHANGED, xScheduleFrame::ScheduleChange)
    EVT_COMMAND(wxID_ANY, EVT_DOCHECKSCHEDULE, xScheduleFrame::DoCheckSchedule)
    EVT_COMMAND(wxID_ANY, EVT_DOACTION, xScheduleFrame::DoAction)
    EVT_COMMAND(wxID_ANY, EVT_XYZZY, xScheduleFrame::DoXyzzy)
    EVT_COMMAND(wxID_ANY, EVT_XYZZYEVENT, xScheduleFrame::DoXyzzyEvent)
    END_EVENT_TABLE()

xScheduleFrame::xScheduleFrame(wxWindow* parent, const std::string& showdir, const std::string& playlist, wxWindowID id)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    _pinger = nullptr;
    __schedule = nullptr;
    _statusSetAt = wxDateTime::Now();
    _webServer = nullptr;
    _timerOutputFrame = false;
    _suspendOTL = false;
    _nowebicon = wxBitmap(no_web_icon_24);
    _webicon = wxBitmap(web_icon_24);
    _webIconDisplayed = false;

    //(*Initialize(xScheduleFrame)
    wxBoxSizer* BoxSizer1;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer5;
    wxFlexGridSizer* FlexGridSizer6;
    wxFlexGridSizer* FlexGridSizer7;
    wxFlexGridSizer* FlexGridSizer8;
    wxMenu* Menu1;
    wxMenu* Menu2;
    wxMenuBar* MenuBar1;
    wxMenuItem* MenuItem1;
    wxMenuItem* MenuItem2;

    Create(parent, id, _("xSchedule"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("id"));
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer1->AddGrowableRow(1);
    Panel2 = new wxPanel(this, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxRAISED_BORDER|wxTAB_TRAVERSAL, _T("ID_PANEL2"));
    FlexGridSizer5 = new wxFlexGridSizer(0, 11, 0, 0);
    BitmapButton_OutputToLights = new wxBitmapButton(Panel2, ID_BITMAPBUTTON1, wxNullBitmap, wxDefaultPosition, wxSize(32,32), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON1"));
    FlexGridSizer5->Add(BitmapButton_OutputToLights, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_Playing = new wxBitmapButton(Panel2, ID_BITMAPBUTTON3, wxNullBitmap, wxDefaultPosition, wxSize(32,32), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON3"));
    FlexGridSizer5->Add(BitmapButton_Playing, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_IsScheduled = new wxBitmapButton(Panel2, ID_BITMAPBUTTON6, wxNullBitmap, wxDefaultPosition, wxSize(32,32), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON6"));
    FlexGridSizer5->Add(BitmapButton_IsScheduled, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_Random = new wxBitmapButton(Panel2, ID_BITMAPBUTTON2, wxNullBitmap, wxDefaultPosition, wxSize(32,32), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON2"));
    FlexGridSizer5->Add(BitmapButton_Random, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_PLLoop = new wxBitmapButton(Panel2, ID_BITMAPBUTTON4, wxNullBitmap, wxDefaultPosition, wxSize(32,32), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON4"));
    FlexGridSizer5->Add(BitmapButton_PLLoop, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_StepLoop = new wxBitmapButton(Panel2, ID_BITMAPBUTTON5, wxNullBitmap, wxDefaultPosition, wxSize(32,32), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON5"));
    FlexGridSizer5->Add(BitmapButton_StepLoop, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_Unsaved = new wxBitmapButton(Panel2, ID_BITMAPBUTTON7, wxNullBitmap, wxDefaultPosition, wxSize(32,32), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON7"));
    FlexGridSizer5->Add(BitmapButton_Unsaved, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Brightness = new BrightnessControl(Panel2,ID_CUSTOM1,wxDefaultPosition,wxSize(32,32),ZERO,wxDefaultValidator,_T("ID_CUSTOM1"));
    FlexGridSizer5->Add(Brightness, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_VolumeDown = new wxBitmapButton(Panel2, ID_BITMAPBUTTON8, wxNullBitmap, wxDefaultPosition, wxSize(32,32), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON8"));
    FlexGridSizer5->Add(BitmapButton_VolumeDown, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Custom_Volume = new VolumeDisplay(Panel2,ID_CUSTOM2,wxDefaultPosition,wxSize(32,32),ZERO,wxDefaultValidator,_T("ID_CUSTOM2"));
    FlexGridSizer5->Add(Custom_Volume, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_VolumeUp = new wxBitmapButton(Panel2, ID_BITMAPBUTTON9, wxNullBitmap, wxDefaultPosition, wxSize(32,32), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON9"));
    FlexGridSizer5->Add(BitmapButton_VolumeUp, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel2->SetSizer(FlexGridSizer5);
    FlexGridSizer5->Fit(Panel2);
    FlexGridSizer5->SetSizeHints(Panel2);
    FlexGridSizer1->Add(Panel2, 1, wxALL|wxEXPAND, 0);
    SplitterWindow1 = new wxSplitterWindow(this, ID_SPLITTERWINDOW1, wxDefaultPosition, wxSize(52,39), wxSP_3D, _T("ID_SPLITTERWINDOW1"));
    SplitterWindow1->SetMinSize(wxSize(10,10));
    SplitterWindow1->SetSashGravity(0.5);
    Panel3 = new wxPanel(SplitterWindow1, ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL3"));
    FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer2->AddGrowableCol(0);
    FlexGridSizer2->AddGrowableRow(0);
    SplitterWindow2 = new wxSplitterWindow(Panel3, ID_SPLITTERWINDOW2, wxDefaultPosition, wxDefaultSize, wxSP_3D, _T("ID_SPLITTERWINDOW2"));
    SplitterWindow2->SetMinSize(wxSize(10,10));
    SplitterWindow2->SetSashGravity(0.5);
    Panel6 = new wxPanel(SplitterWindow2, ID_PANEL6, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL6"));
    FlexGridSizer7 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer7->AddGrowableCol(0);
    FlexGridSizer7->AddGrowableRow(0);
    TreeCtrl_PlayListsSchedules = new wxTreeCtrl(Panel6, ID_TREECTRL1, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE, wxDefaultValidator, _T("ID_TREECTRL1"));
    TreeCtrl_PlayListsSchedules->SetMinSize(wxSize(300,300));
    FlexGridSizer7->Add(TreeCtrl_PlayListsSchedules, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 5);
    BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    Button_Add = new wxButton(Panel6, ID_BUTTON1, _("Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    BoxSizer1->Add(Button_Add, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Edit = new wxButton(Panel6, ID_BUTTON2, _("Edit"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    BoxSizer1->Add(Button_Edit, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Delete = new wxButton(Panel6, ID_BUTTON3, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    BoxSizer1->Add(Button_Delete, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Schedule = new wxButton(Panel6, ID_BUTTON4, _("Schedule"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
    BoxSizer1->Add(Button_Schedule, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer7->Add(BoxSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel6->SetSizer(FlexGridSizer7);
    FlexGridSizer7->Fit(Panel6);
    FlexGridSizer7->SetSizeHints(Panel6);
    Panel7 = new wxPanel(SplitterWindow2, ID_PANEL7, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL7"));
    FlexGridSizer8 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer8->AddGrowableCol(0);
    FlexGridSizer8->AddGrowableRow(0);
    ListView_Ping = new wxListView(Panel7, ID_LISTVIEW2, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL|wxLC_NO_SORT_HEADER|wxVSCROLL, wxDefaultValidator, _T("ID_LISTVIEW2"));
    FlexGridSizer8->Add(ListView_Ping, 1, wxALL|wxEXPAND, 5);
    Panel7->SetSizer(FlexGridSizer8);
    FlexGridSizer8->Fit(Panel7);
    FlexGridSizer8->SetSizeHints(Panel7);
    SplitterWindow2->SplitHorizontally(Panel6, Panel7);
    FlexGridSizer2->Add(SplitterWindow2, 1, wxALL|wxEXPAND, 5);
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
    FlexGridSizer1->Add(SplitterWindow1, 1, wxALL|wxEXPAND, 0);
    Panel1 = new wxPanel(this, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    FlexGridSizer4 = new wxFlexGridSizer(0, 0, 0, 0);
    Panel1->SetSizer(FlexGridSizer4);
    FlexGridSizer4->Fit(Panel1);
    FlexGridSizer4->SetSizeHints(Panel1);
    FlexGridSizer1->Add(Panel1, 1, wxALL|wxEXPAND, 0);
    Panel4 = new wxPanel(this, ID_PANEL4, wxDefaultPosition, wxDefaultSize, wxRAISED_BORDER|wxTAB_TRAVERSAL, _T("ID_PANEL4"));
    FlexGridSizer6 = new wxFlexGridSizer(0, 6, 0, 0);
    FlexGridSizer6->AddGrowableCol(1);
    StaticText_ShowDir = new wxStaticText(Panel4, ID_STATICTEXT1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer6->Add(StaticText_ShowDir, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_IP = new wxStaticText(Panel4, ID_STATICTEXT3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer6->Add(StaticText_IP, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_PacketsPerSec = new wxStaticText(Panel4, ID_STATICTEXT4, _("Packets/Sec: 0          "), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    FlexGridSizer6->Add(StaticText_PacketsPerSec, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText2 = new wxStaticText(Panel4, ID_STATICTEXT5, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
    FlexGridSizer6->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_Time = new wxStaticText(Panel4, ID_STATICTEXT2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer6->Add(StaticText_Time, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    StaticBitmap_WebIcon = new wxStaticBitmap(Panel4, ID_STATICBITMAP1, wxNullBitmap, wxDefaultPosition, wxSize(24,24), 0, _T("ID_STATICBITMAP1"));
    FlexGridSizer6->Add(StaticBitmap_WebIcon, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel4->SetSizer(FlexGridSizer6);
    FlexGridSizer6->Fit(Panel4);
    FlexGridSizer6->SetSizeHints(Panel4);
    FlexGridSizer1->Add(Panel4, 1, wxALL|wxEXPAND, 0);
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
    Menu5 = new wxMenu();
    MenuItem_AddPlayList = new wxMenuItem(Menu5, ID_MNU_MNUADDPLAYLIST, _("&Add Playlist"), wxEmptyString, wxITEM_NORMAL);
    Menu5->Append(MenuItem_AddPlayList);
    Menu_OutputProcessing = new wxMenuItem(Menu5, ID_MENUITEM1, _("&Output Processing"), wxEmptyString, wxITEM_NORMAL);
    Menu5->Append(Menu_OutputProcessing);
    MenuItem_BackgroundPlaylist = new wxMenuItem(Menu5, ID_MNU_BACKGROUND, _("&Background Playlist"), wxEmptyString, wxITEM_CHECK);
    Menu5->Append(MenuItem_BackgroundPlaylist);
    MenuItem_Matrices = new wxMenuItem(Menu5, ID_MNU_MATRICES, _("&Matrices"), wxEmptyString, wxITEM_NORMAL);
    Menu5->Append(MenuItem_Matrices);
    MenuItem_VirtualMatrices = new wxMenuItem(Menu5, ID_MNU_VIRTUALMATRICES, _("&Virtual Matrices"), wxEmptyString, wxITEM_NORMAL);
    Menu5->Append(MenuItem_VirtualMatrices);
    MenuItem_EditEvents = new wxMenuItem(Menu5, ID_MNU_EDITEVENTS, _("&Events"), wxEmptyString, wxITEM_NORMAL);
    Menu5->Append(MenuItem_EditEvents);
    MenuItem_Options = new wxMenuItem(Menu5, ID_MNU_OPTIONS, _("&Options"), wxEmptyString, wxITEM_NORMAL);
    Menu5->Append(MenuItem_Options);
    MenuBar1->Append(Menu5, _("&Edit"));
    Menu3 = new wxMenu();
    MenuItem_ViewLog = new wxMenuItem(Menu3, ID_MNU_VIEW_LOG, _("&View Log"), wxEmptyString, wxITEM_NORMAL);
    Menu3->Append(MenuItem_ViewLog);
    MenuItem_CheckSchedule = new wxMenuItem(Menu3, ID_MNU_CHECK_SCHEDULE, _("&Check Schedule"), wxEmptyString, wxITEM_NORMAL);
    Menu3->Append(MenuItem_CheckSchedule);
    MenuItem_WebInterface = new wxMenuItem(Menu3, ID_MNU_WEBINTERFACE, _("&Web Interface"), wxEmptyString, wxITEM_NORMAL);
    Menu3->Append(MenuItem_WebInterface);
    MenuItem_ImportxLights = new wxMenuItem(Menu3, ID_MNU_IMPORT, _("Import xLights Playlist"), wxEmptyString, wxITEM_NORMAL);
    Menu3->Append(MenuItem_ImportxLights);
    MenuItem_Crash = new wxMenuItem(Menu3, ID_MNU_CRASH, _("Crash"), _("Crash xSchedule"), wxITEM_NORMAL);
    Menu3->Append(MenuItem_Crash);
    MenuBar1->Append(Menu3, _("&Tools"));
    Menu4 = new wxMenu();
    MenuItem_Standalone = new wxMenuItem(Menu4, ID_MNU_MODENORMAL, _("Standalone"), wxEmptyString, wxITEM_RADIO);
    Menu4->Append(MenuItem_Standalone);
    MenuItem_FPPMaster = new wxMenuItem(Menu4, ID_MNU_FPPMASTER, _("FPP Master"), wxEmptyString, wxITEM_RADIO);
    Menu4->Append(MenuItem_FPPMaster);
    MenuItem_OSCMaster = new wxMenuItem(Menu4, ID_MNU_OSCMASTER, _("OSC Master"), wxEmptyString, wxITEM_RADIO);
    Menu4->Append(MenuItem_OSCMaster);
    MenuItem_FPPOSCMaster = new wxMenuItem(Menu4, ID_MNU_OSCFPPMASTER, _("FPP + OSC Master"), wxEmptyString, wxITEM_RADIO);
    Menu4->Append(MenuItem_FPPOSCMaster);
    MenuItem_ARTNetTimeCodeMaster = new wxMenuItem(Menu4, IDM_MNU_ARTNETMASTER, _("ARTNet Timecode Master"), wxEmptyString, wxITEM_RADIO);
    Menu4->Append(MenuItem_ARTNetTimeCodeMaster);
    MenuItem_MIDITimeCodeMaster = new wxMenuItem(Menu4, MNU_MIDITIMECODE_MASTER, _("MIDI Timecode Master"), wxEmptyString, wxITEM_RADIO);
    Menu4->Append(MenuItem_MIDITimeCodeMaster);
    MenuItem_FPPRemote = new wxMenuItem(Menu4, ID_MNU_FPPREMOTE, _("FPP Remote"), wxEmptyString, wxITEM_RADIO);
    Menu4->Append(MenuItem_FPPRemote);
    MenuItem_OSCRemote = new wxMenuItem(Menu4, ID_MNU_OSCREMOTE, _("OSC Remote"), wxEmptyString, wxITEM_RADIO);
    Menu4->Append(MenuItem_OSCRemote);
    MenuItem_ARTNetTimeCodeSlave = new wxMenuItem(Menu4, ID_MNU_ARTNETTIMECODESLAVE, _("ARTNet Timecode Slave"), wxEmptyString, wxITEM_RADIO);
    Menu4->Append(MenuItem_ARTNetTimeCodeSlave);
    MenuItem_MIDITimeCodeSlave = new wxMenuItem(Menu4, MNU_MIDITIMECODEREMOTE, _("MIDI Timecode Slave"), wxEmptyString, wxITEM_RADIO);
    Menu4->Append(MenuItem_MIDITimeCodeSlave);
    MenuItem_FPPUnicastRemote = new wxMenuItem(Menu4, ID_MNU_FPPUNICASTREMOTE, _("FPP Unicast Remote"), wxEmptyString, wxITEM_RADIO);
    Menu4->Append(MenuItem_FPPUnicastRemote);
    MenuItem_EditFPPRemotes = new wxMenuItem(Menu4, ID_MNU_EDITFPPREMOTE, _("Edit FPP Remotes"), _("Edit remotes to unicast sync packets to"), wxITEM_NORMAL);
    Menu4->Append(MenuItem_EditFPPRemotes);
    MenuItem_ConfigureOSC = new wxMenuItem(Menu4, ID_MNU_OSCOPTION, _("Configure OSC"), wxEmptyString, wxITEM_NORMAL);
    Menu4->Append(MenuItem_ConfigureOSC);
    MenuItem5MenuItem_ConfigureMIDITimecode = new wxMenuItem(Menu4, MNU_CONFIGUREMIDITIMECODE, _("Configure MIDI Timecode"), wxEmptyString, wxITEM_NORMAL);
    Menu4->Append(MenuItem5MenuItem_ConfigureMIDITimecode);
    MenuBar1->Append(Menu4, _("&Modes"));
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
    _timer.Start(500000, false);
    _timerSchedule.SetOwner(this, ID_TIMER2);
    _timerSchedule.Start(50000, false);
    FileDialog1 = new wxFileDialog(this, _("Select file"), wxEmptyString, _("xlights_schedule.xml"), _("xlights_schedule.xml"), wxFD_DEFAULT_STYLE|wxFD_OPEN|wxFD_FILE_MUST_EXIST, wxDefaultPosition, wxDefaultSize, _T("wxFileDialog"));
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    Connect(ID_BITMAPBUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xScheduleFrame::OnBitmapButton_OutputToLightsClick);
    Connect(ID_BITMAPBUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xScheduleFrame::OnBitmapButton_PlayingClick);
    Connect(ID_BITMAPBUTTON6,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xScheduleFrame::OnBitmapButton_IsScheduledClick);
    Connect(ID_BITMAPBUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xScheduleFrame::OnBitmapButton_RandomClick);
    Connect(ID_BITMAPBUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xScheduleFrame::OnBitmapButton_PLLoopClick);
    Connect(ID_BITMAPBUTTON5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xScheduleFrame::OnBitmapButton_StepLoopClick);
    Connect(ID_BITMAPBUTTON7,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xScheduleFrame::OnBitmapButton_UnsavedClick);
    Connect(ID_BITMAPBUTTON8,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xScheduleFrame::OnBitmapButton_VolumeDownClick);
    Custom_Volume->Connect(wxEVT_LEFT_DOWN,(wxObjectEventFunction)&xScheduleFrame::OnCustom_VolumeLeftDown,0,this);
    Connect(ID_BITMAPBUTTON9,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xScheduleFrame::OnBitmapButton_VolumeUpClick);
    Connect(ID_TREECTRL1,wxEVT_COMMAND_TREE_ITEM_ACTIVATED,(wxObjectEventFunction)&xScheduleFrame::OnTreeCtrl_PlayListsSchedulesItemActivated);
    Connect(ID_TREECTRL1,wxEVT_COMMAND_TREE_SEL_CHANGED,(wxObjectEventFunction)&xScheduleFrame::OnTreeCtrl_PlayListsSchedulesSelectionChanged);
    Connect(ID_TREECTRL1,wxEVT_COMMAND_TREE_KEY_DOWN,(wxObjectEventFunction)&xScheduleFrame::OnTreeCtrl_PlayListsSchedulesKeyDown);
    Connect(ID_TREECTRL1,wxEVT_COMMAND_TREE_ITEM_MENU,(wxObjectEventFunction)&xScheduleFrame::OnTreeCtrl_PlayListsSchedulesItemMenu);
    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xScheduleFrame::OnButton_AddClick);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xScheduleFrame::OnButton_EditClick);
    Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xScheduleFrame::OnButton_DeleteClick);
    Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xScheduleFrame::OnButton_ScheduleClick);
    Connect(ID_LISTVIEW2,wxEVT_COMMAND_LIST_ITEM_ACTIVATED,(wxObjectEventFunction)&xScheduleFrame::OnListView_PingItemActivated);
    Connect(ID_LISTVIEW2,wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK,(wxObjectEventFunction)&xScheduleFrame::OnListView_PingItemRClick);
    Connect(ID_LISTVIEW1,wxEVT_COMMAND_LIST_ITEM_ACTIVATED,(wxObjectEventFunction)&xScheduleFrame::OnListView_RunningItemActivated);
    Connect(ID_MNU_SHOWFOLDER,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_ShowFolderSelected);
    Connect(ID_MNU_SAVE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_SaveSelected);
    Connect(idMenuQuit,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnQuit);
    Connect(ID_MNU_MNUADDPLAYLIST,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_AddPlayListSelected);
    Connect(ID_MENUITEM1,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenu_OutputProcessingSelected);
    Connect(ID_MNU_BACKGROUND,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_BackgroundPlaylistSelected);
    Connect(ID_MNU_MATRICES,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_MatricesSelected);
    Connect(ID_MNU_VIRTUALMATRICES,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_VirtualMatricesSelected);
    Connect(ID_MNU_EDITEVENTS,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_EditEventsSelected);
    Connect(ID_MNU_OPTIONS,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_OptionsSelected);
    Connect(ID_MNU_VIEW_LOG,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_ViewLogSelected);
    Connect(ID_MNU_CHECK_SCHEDULE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_CheckScheduleSelected);
    Connect(ID_MNU_WEBINTERFACE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_WebInterfaceSelected);
    Connect(ID_MNU_IMPORT,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_ImportxLightsSelected);
    Connect(ID_MNU_CRASH,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_CrashSelected);
    Connect(ID_MNU_MODENORMAL,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_StandaloneSelected);
    Connect(ID_MNU_FPPMASTER,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_FPPMasterSelected);
    Connect(ID_MNU_OSCMASTER,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_OSCMasterSelected);
    Connect(ID_MNU_OSCFPPMASTER,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_FPPOSCMasterSelected);
    Connect(IDM_MNU_ARTNETMASTER,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_ARTNetTimeCodeMasterSelected);
    Connect(MNU_MIDITIMECODE_MASTER,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_MIDITimeCodeMasterSelected);
    Connect(ID_MNU_FPPREMOTE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_FPPRemoteSelected);
    Connect(ID_MNU_OSCREMOTE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_OSCRemoteSelected);
    Connect(ID_MNU_ARTNETTIMECODESLAVE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_ARTNetTimeCodeSlaveSelected);
    Connect(MNU_MIDITIMECODEREMOTE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_MIDITimeCodeSlaveSelected);
    Connect(ID_MNU_FPPUNICASTREMOTE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_FPPUnicastRemoteSelected);
    Connect(ID_MNU_EDITFPPREMOTE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_EditFPPRemotesSelected);
    Connect(ID_MNU_OSCOPTION,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_ConfigureOSCSelected);
    Connect(MNU_CONFIGUREMIDITIMECODE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem5MenuItem_ConfigureMIDITimecodeSelected);
    Connect(idMenuAbout,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnAbout);
    Connect(ID_TIMER1,wxEVT_TIMER,(wxObjectEventFunction)&xScheduleFrame::On_timerTrigger);
    Connect(ID_TIMER2,wxEVT_TIMER,(wxObjectEventFunction)&xScheduleFrame::On_timerScheduleTrigger);
    Connect(wxEVT_SIZE,(wxObjectEventFunction)&xScheduleFrame::OnResize);
    //*)

    Connect(wxID_ANY, EVT_FRAMEMS, (wxObjectEventFunction)&xScheduleFrame::RateNotification);
    Connect(wxID_ANY, EVT_STATUSMSG, (wxObjectEventFunction)&xScheduleFrame::StatusMsgNotification);
    Connect(wxID_ANY, EVT_RUNACTION, (wxObjectEventFunction)&xScheduleFrame::RunAction);
    Connect(wxID_ANY, EVT_SCHEDULECHANGED, (wxObjectEventFunction)&xScheduleFrame::ScheduleChange);
    Connect(wxID_ANY, EVT_DOCHECKSCHEDULE, (wxObjectEventFunction)&xScheduleFrame::DoCheckSchedule);
    Connect(wxID_ANY, EVT_DOACTION, (wxObjectEventFunction)&xScheduleFrame::DoAction);
    Connect(wxID_ANY, EVT_XYZZY, (wxObjectEventFunction)&xScheduleFrame::DoXyzzy);
    Connect(wxID_ANY, EVT_XYZZYEVENT, (wxObjectEventFunction)&xScheduleFrame::DoXyzzyEvent);
    Connect(wxID_ANY, wxEVT_CHAR_HOOK, (wxObjectEventFunction)&xScheduleFrame::OnKeyDown);

    SetTitle("xLights Scheduler " + xlights_version_string + " " + GetBitness());

    // Force initialise sockets
    wxIPV4address localaddr;
    localaddr.AnyAddress();
    auto sckt = new wxDatagramSocket(localaddr, wxSOCKET_BROADCAST);
    delete sckt;

    wxIconBundle icons;
    icons.AddIcon(wxIcon(xlights_16_xpm));
    icons.AddIcon(wxIcon(xlights_32_xpm));
    icons.AddIcon(wxIcon(xlights_64_xpm));
    icons.AddIcon(wxIcon(xlights_128_xpm));
    icons.AddIcon(wxIcon(xlights_xpm));
    SetIcons(icons);

    int x, y, w, h;
    wxConfigBase* config = wxConfigBase::Get();
    x = config->ReadLong(_("xsWindowPosX"), 50);
    y = config->ReadLong(_("xsWindowPosY"), 50);
    w = config->ReadLong(_("xsWindowPosW"), 800);
    h = config->ReadLong(_("xsWindowPosH"), 600);

    // limit weirdness
    if (x < -100) x = 0;
    if (x > 2000) x = 400;
    if (y < -100) y = 0;
    if (y > 2000) y = 400;

    SetPosition(wxPoint(x, y));
    SetSize(w, h);

    ListView_Running->AppendColumn("Step");
    ListView_Running->AppendColumn("Duration");
    ListView_Running->AppendColumn("");

    ListView_Ping->AppendColumn("Controller");

    _otlon = wxBitmap(xs_otlon);
    _otloff = wxBitmap(xs_otloff);
    _otlautooff = wxBitmap(xs_otlautooff);
    _otlautoon = wxBitmap(xs_otlautoon);
    _save = wxBitmap(xs_save);
    _scheduled = wxBitmap(xs_scheduled);
    _queued = wxBitmap(xs_queued);
    _notscheduled = wxBitmap(xs_notscheduled);
    _inactive = wxBitmap(xs_inactive);
    _pllooped = wxBitmap(xs_pllooped);
    _plnotlooped = wxBitmap(xs_plnotlooped);
    _plsteplooped = wxBitmap(xs_plsteplooped);
    _plstepnotlooped = wxBitmap(xs_plstepnotlooped);
    _playing = wxBitmap(xs_playing);
    _idle = wxBitmap(xs_idle);
    _paused = wxBitmap(xs_paused);
    _random = wxBitmap(xs_random);
    _notrandom = wxBitmap(xs_notrandom);
    _volumeup = wxBitmap(xs_volume_up);
    _volumedown = wxBitmap(xs_volume_down);
    _falconremote = wxBitmap(xs_falcon);

    Brightness->SetToolTip("Brightness");
    BitmapButton_VolumeUp->SetToolTip("Increase Volume");
    BitmapButton_VolumeDown->SetToolTip("Decrease Volume");
    Custom_Volume->SetToolTip("Volume");
    BitmapButton_VolumeDown->SetBitmap(_volumedown);
    BitmapButton_VolumeUp->SetBitmap(_volumeup);

    if (showdir == "")
    {
        LoadShowDir();
    }
    else
    {
        _showDir = showdir;
    }

    LoadSchedule();

    if (__schedule == nullptr)
    {
        logger_base.error("Error loading schedule.");
        Close();
    }

    // Uncomment this to run the MatrixMapper tests
    //MatrixMapper::Test(__schedule->GetOutputManager());

    // Uncomment this to run the sunrise/sunset tests
    // City::Test();

    // Uncomment this to run the schedule date tests
    //Schedule::Test();

    wxFrame::SendSizeEvent();
    size_t rate = 50;

    if (playlist != "")
    {
        auto p = __schedule->GetPlayList(playlist);
        __schedule->PlayPlayList(p, rate, true);
        UpdateUI();
    }

    _timer.Stop();
    if (rate == 0) rate = 50;
    _timer.Start(rate/2, false);
    _timerSchedule.Stop();
    _timerSchedule.Start(500, true);

    StaticText_IP->SetLabel("    " + __schedule->GetOurIP() + "   ");

    StaticBitmap_WebIcon->SetBitmap(_nowebicon);
    _webIconDisplayed = false;

    // This is for keith ... I like my debug version to be distinctive so I can tell it apart from the prior version
#ifndef NDEBUG
    logger_base.debug("xSchedule Crash Menu item not removed.");
    #ifdef _MSC_VER
        SetBackgroundColour(*wxGREEN);
    #endif
#else
    // only keep the crash option if the EnableCrash.txt file exists
    if (!wxFile::Exists("EnableCrash.txt"))
    {
        MenuItem_Crash->GetMenu()->Remove(MenuItem_Crash);
        MenuItem_Crash = nullptr;
    }
    else
    {
        logger_base.debug("xSchedule Crash Menu item not removed.");
    }
#endif

    UpdateUI();
    ValidateWindow();
}

void xScheduleFrame::LoadSchedule()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Loading schedule.");

    if (_pinger != nullptr)
    {
        __schedule->SetPinger(nullptr);
        delete _pinger;
        _pinger = nullptr;
    }
    ListView_Ping->ClearAll();
    ListView_Ping->AppendColumn("Controller");

    if (__schedule != nullptr)
    {
        delete __schedule;
        __schedule = nullptr;
    }

    __schedule = new ScheduleManager(this, _showDir);

    _pinger = new Pinger(__schedule->GetListenerManager(), __schedule->GetOutputManager());
    __schedule->SetPinger(_pinger);

    if (__schedule == nullptr) return;

    if (_webServer != nullptr)
    {
        delete _webServer;
        _webServer = nullptr;
    }
    _webServer = new WebServer(__schedule->GetOptions()->GetWebServerPort(), __schedule->GetOptions()->GetAPIOnly(), __schedule->GetOptions()->GetPassword(), __schedule->GetOptions()->GetPasswordTimeout());

    if (wxFile::Exists(_showDir + "/xlights_networks.xml"))
    {
        StaticText_ShowDir->SetLabel(_showDir);
        if (__schedule->ShowDirectoriesMatch())
        {
            StaticText_ShowDir->SetForegroundColour(*wxBLACK);
        }
        else
        {
            StaticText_ShowDir->SetForegroundColour(wxColour(255,128,0));
        }
    }
    else
    {
        StaticText_ShowDir->SetLabel(_showDir + " : Missing xlights_networks.xml");
        StaticText_ShowDir->SetForegroundColour(*wxRED);
    }

    logger_base.debug("Adding IPs to ping.");
    AddIPs();

    logger_base.debug("Update the playlist tree.");
    UpdateTree();

    logger_base.debug("Creating buttons.");
    CreateButtons();

    logger_base.debug("Schedule loaded.");
}

void xScheduleFrame::AddIPs()
{
    _pinger->RemoveNonOutputIPs();

    auto fppremotes = __schedule->GetOptions()->GetFPPRemotes();
    for (auto it = fppremotes.begin(); it != fppremotes.end(); ++it)
    {
        _pinger->AddIP(*it, "FPPRemote");
    }

    if (__schedule->GetOptions()->GetOSCOptions() != nullptr)
    {
        _pinger->AddIP(__schedule->GetOptions()->GetOSCOptions()->GetIPAddress(), "OSCTarget");
    }

    auto plis = __schedule->GetPlayListIps();
    for (auto it = plis.begin(); it != plis.end(); ++it)
    {
        if ((*it)->GetTitle() == "OSC")
        {
            PlayListItemOSC* osc = (PlayListItemOSC*)*it;
            _pinger->AddIP(osc->GetIP(), "OSC Play List Item");
        }
    }
}

xScheduleFrame::~xScheduleFrame()
{
    if (_pinger != nullptr)
    {
        delete _pinger;
        _pinger = nullptr;
    }

    int x, y;
    GetPosition(&x, &y);

    int w, h;
    GetSize(&w, &h);

    wxConfigBase* config = wxConfigBase::Get();
    config->Write(_("xsWindowPosX"), x);
    config->Write(_("xsWindowPosY"), y);
    config->Write(_("xsWindowPosW"), w);
    config->Write(_("xsWindowPosH"), h);
    config->Flush();

    //(*Destroy(xScheduleFrame)
    //*)

    if (_webServer != nullptr)
    {
        delete _webServer;
        _webServer = nullptr;
    }

    if (__schedule != nullptr)
    {
        delete __schedule;
        __schedule = nullptr;
    }
}

void xScheduleFrame::OnQuit(wxCommandEvent& event)
{
    if (__schedule->IsDirty())
    {
        if (wxMessageBox("Unsaved changes to the schedule. Save now?", "Unsaved changes", wxYES_NO) == wxYES)
        {
            __schedule->Save();
        }
        else
        {
            __schedule->ClearDirty();
        }
    }

    Close();
}

void xScheduleFrame::OnAbout(wxCommandEvent& event)
{
    auto about = wxString::Format(wxT("xSchedule v%s %s, the xLights scheduler."), xlights_version_string, GetBitness());
    wxMessageBox(about, _("Welcome to..."));
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
    if (!__schedule->GetOptions()->IsAdvancedMode())
    {
        mnu.Append(ID_MNU_ADDADVPLAYLIST, "Add Advanced Playlist");
    }
    wxMenuItem* sched = mnu.Append(ID_MNU_SCHEDULEPLAYLIST, "Schedule");
    if (!IsPlayList(treeitem))
    {
        sched->Enable(false);
    }

    wxMenuItem* del = mnu.Append(ID_MNU_DELETE, "Delete");
    wxMenuItem* duplicate = mnu.Append(ID_MNU_DUPLICATEPLAYLIST, "Duplicate");
    wxMenuItem* edit = mnu.Append(ID_MNU_EDIT, "Edit");
    if (!__schedule->GetOptions()->IsAdvancedMode())
    {
        if (IsPlayList(treeitem))
        {
            mnu.Append(ID_MNU_EDITADV, "Edit Advanced");
        }
    }

    if (!IsPlayList(treeitem) && !IsSchedule(treeitem))
    {
        del->Enable(false);
        edit->Enable(false);
    }

    if (!IsPlayList(treeitem))
    {
        duplicate->Enable(false);
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
        AddPlayList();
    }
    else if (event.GetId() == ID_MNU_ADDADVPLAYLIST)
    {
        AddPlayList(true);
    }
    else if (event.GetId() == ID_MNU_SCHEDULEPLAYLIST)
    {
        AddSchedule();
    }
    else if (event.GetId() == ID_MNU_DELETE)
    {
        DeleteSelectedItem();
    }
    else if (event.GetId() == ID_MNU_EDIT)
    {
        EditSelectedItem();
    }
    else if (event.GetId() == ID_MNU_EDITADV)
    {
        EditSelectedItem(true);
    }
    else if (event.GetId() == ID_MNU_DUPLICATEPLAYLIST)
    {
        PlayList* playlist = (PlayList*)((MyTreeItemData*)TreeCtrl_PlayListsSchedules->GetItemData(treeitem))->GetData();

        PlayList* newpl = new PlayList(*playlist, true);

        wxTreeItemId  newitem = TreeCtrl_PlayListsSchedules->AppendItem(TreeCtrl_PlayListsSchedules->GetRootItem(), playlist->GetName(), -1, -1, new MyTreeItemData(newpl));
        TreeCtrl_PlayListsSchedules->Expand(newitem);
        TreeCtrl_PlayListsSchedules->EnsureVisible(newitem);
        __schedule->AddPlayList(newpl);
    }
    UpdateUI();
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
    UpdateUI();
    ValidateWindow();
}

void xScheduleFrame::DoPaste()
{
    wxTextDataObject clip;
    if (wxTheClipboard->Open())
    {
        wxTheClipboard->GetData(clip);
        wxTheClipboard->Close();
    }
    std::string _copybuffer = clip.GetText().ToStdString();

    if (_copybuffer != "")
    {
        wxArrayString copy = wxSplit(_copybuffer, ',');

        if (copy[0] == "PL")
        {
            if (copy.Count() == 2)
            {
                PlayList* playlist = __schedule->GetPlayList(wxAtoi(copy[1]));
                if (playlist != nullptr)
                {
                    PlayList* newpl = new PlayList(*playlist, true);
                    __schedule->AddPlayList(newpl);
                    UpdateTree();
                }
            }
        }
        else if (copy[0] == "SC")
        {
            if (copy.Count() == 3)
            {
                PlayList* playlist = __schedule->GetPlayList(wxAtoi(copy[1]));
                if (playlist != nullptr)
                {
                    Schedule* schedule = playlist->GetSchedule(wxAtoi(copy[2]));
                    if (schedule != nullptr)
                    {
                        wxTreeItemId treeitem = TreeCtrl_PlayListsSchedules->GetSelection();
                        PlayList* to = nullptr;
                        if (IsSchedule(treeitem))
                        {
                            to = (PlayList*)((MyTreeItemData*)TreeCtrl_PlayListsSchedules->GetItemData(TreeCtrl_PlayListsSchedules->GetItemParent(treeitem)))->GetData();
                        }
                        else if (IsPlayList(treeitem))
                        {
                            to = (PlayList*)((MyTreeItemData*)TreeCtrl_PlayListsSchedules->GetItemData(treeitem))->GetData();
                        }

                        if (to != nullptr)
                        {
                            to->AddSchedule(new Schedule(*schedule, true));
                            UpdateTree();
                        }
                    }
                }
            }
        }
    }
}

void xScheduleFrame::DoCopy()
{
    std::string copybuffer = "";
    wxTreeItemId treeitem = TreeCtrl_PlayListsSchedules->GetSelection();
    if (IsPlayList(treeitem))
    {
        PlayList* playlist = (PlayList*)((MyTreeItemData*)TreeCtrl_PlayListsSchedules->GetItemData(treeitem))->GetData();
        copybuffer = "PL," + wxString::Format(wxT("%i"), playlist->GetId());
    }
    else if (IsSchedule(treeitem))
    {
        PlayList* playlist = (PlayList*)((MyTreeItemData*)TreeCtrl_PlayListsSchedules->GetItemData(TreeCtrl_PlayListsSchedules->GetItemParent(treeitem)))->GetData();
        Schedule* schedule = (Schedule*)((MyTreeItemData*)TreeCtrl_PlayListsSchedules->GetItemData(treeitem))->GetData();
        copybuffer = "SC," + wxString::Format(wxT("%i"), playlist->GetId()) + "," + wxString::Format(wxT("%i"), schedule->GetId());
    }

    if (wxTheClipboard->Open())
    {
        wxTheClipboard->Clear();
        wxTheClipboard->SetData(new wxTextDataObject(copybuffer));
        wxTheClipboard->Flush();
        wxTheClipboard->Close();
    }
}

void xScheduleFrame::OnTreeCtrl_PlayListsSchedulesKeyDown(wxTreeEvent& event)
{
}

void xScheduleFrame::OnMenuItem_SaveSelected(wxCommandEvent& event)
{
    __schedule->Save();
    UpdateUI();
    ValidateWindow();
}

void xScheduleFrame::OnMenuItem_ShowFolderSelected(wxCommandEvent& event)
{
    DirDialog1->SetPath(_showDir);

    if (DirDialog1->ShowModal() == wxID_OK)
    {
        _showDir = DirDialog1->GetPath().ToStdString();
        SaveShowDir();
        _timerSchedule.Stop();
        _timer.Stop();
        LoadSchedule();
        _timer.Start(50 / 2, false);
        _timerSchedule.Start(50, true);
    }
    ValidateWindow();
}

void xScheduleFrame::SaveShowDir() const
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxConfigBase* config = wxConfigBase::Get();
    auto sd = wxString(_showDir);
    logger_base.debug("Saving show folder location %s.", (const char *)sd.c_str());
    if (!config->Write(_("SchedulerLastDir"), sd))
    {
        logger_base.error("Error saving 'SchedulerLastDir'.");
    }
    config->Flush();
}

void xScheduleFrame::LoadShowDir()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxConfigBase* config = wxConfigBase::Get();
    logger_base.debug("Config: AppName '%s' Path '%s' Entries %d Groups %d Style %ld Vendor '%s'.", (const char *)config->GetAppName().c_str(), (const char *)config->GetPath().c_str(), (int)config->GetNumberOfEntries(), (int)config->GetNumberOfGroups(), config->GetStyle(), (const char*)config->GetVendorName().c_str());

    // get the show directory
    wxString showDir = ScheduleManager::xScheduleShowDir();
    if (showDir == "")
    {
        logger_base.debug("Could not read show folder from 'SchedulerLastDir'.");
        showDir = ScheduleManager::xLightsShowDir();
        if (showDir == "")
        {
            logger_base.debug("Could not read show folder from 'xLights::LastDir'.");
            DirDialog1->SetPath(_showDir);

            if (DirDialog1->ShowModal() == wxID_OK)
            {
                _showDir = DirDialog1->GetPath().ToStdString();
                logger_base.debug("User selected show folder '%s'.", (const char *)_showDir.c_str());
                SaveShowDir();
            }
            else
            {
                _showDir = ".";
            }
        }
        else
        {
            logger_base.debug("Read show folder from 'xLights::LastDir' location %s.", (const char *)showDir.c_str());
            _showDir = showDir.ToStdString();
            SaveShowDir();
        }
    }
    else
    {
        logger_base.debug("Read show folder from 'SchedulerLastDir' location %s.", (const char *)showDir.c_str());
        _showDir = showDir.ToStdString();
    }
}

void xScheduleFrame::UpdateTree() const
{
    TreeCtrl_PlayListsSchedules->Freeze();

    TreeCtrl_PlayListsSchedules->DeleteAllItems();

    wxTreeItemId root = TreeCtrl_PlayListsSchedules->AddRoot("Playlists");

    auto pls = __schedule->GetPlayLists();

    for (auto it = pls.begin(); it != pls.end(); ++it)
    {
        auto pl = TreeCtrl_PlayListsSchedules->AppendItem(root, (*it)->GetName(), -1, -1, new MyTreeItemData(*it));

        auto schedules = (*it)->GetSchedules();
        for (auto it2 = schedules.begin(); it2 != schedules.end(); ++it2)
        {
            TreeCtrl_PlayListsSchedules->AppendItem(pl, GetScheduleName(*it2, __schedule->GetRunningSchedules()), -1, -1, new MyTreeItemData(*it2));
        }
        TreeCtrl_PlayListsSchedules->Expand(pl);
    }
    TreeCtrl_PlayListsSchedules->Expand(root);

    TreeCtrl_PlayListsSchedules->Thaw();
    TreeCtrl_PlayListsSchedules->Refresh();
}

std::string xScheduleFrame::GetScheduleName(Schedule* schedule, const std::list<RunningSchedule*>& active) const
{
    for (auto it = active.begin(); it != active.end(); ++it)
    {
        if ((*it)->GetSchedule()->GetId() == schedule->GetId())
        {
            if ((*it)->IsStopped())
            {
                return schedule->GetName() + " [Stopped]";
            }
        }
    }

    if (schedule->GetNextTriggerTime() == "NOW!")
    {
        for (auto it = active.begin(); it != active.end(); ++it)
        {
            if ((*it)->GetSchedule()->GetId() == schedule->GetId())
            {
                return schedule->GetName() + " [NOW until " + (*it)->GetSchedule()->GetNextEndTime() + "]"; // +wxString::Format(" Id:%i", schedule->GetId()).ToStdString();
            }
        }
    }

    return schedule->GetName() + " [" + schedule->GetNextTriggerTime() + "]"; // +wxString::Format(" Id:%i", schedule->GetId()).ToStdString();
}

void xScheduleFrame::OnTreeCtrl_PlayListsSchedulesItemActivated(wxTreeEvent& event)
{
    EditSelectedItem();
    UpdateUI();
    ValidateWindow();
}

void xScheduleFrame::On_timerTrigger(wxTimerEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    static int last = -1;

    if (__schedule == nullptr) return;

    static long long lastms = 0;
    long long now = wxGetLocalTimeMillis().GetValue();
    if (now - lastms > _timer.GetInterval() * 4)
    {
        if (lastms != 0)
        {
            logger_base.warn("Frame interval greater than 200%% of what it should have been [%d] %d", _timer.GetInterval() * 2, (int)(now - lastms));
        }
    }
#ifdef TRACE_SCHEDULE_PERFORMANCE
    logger_base.debug("Start frame %d", (int)(now - lastms));
#endif
    lastms = now;

    wxDateTime frameStart = wxDateTime::UNow();

    int rate = __schedule->Frame(_timerOutputFrame);

    if (last != wxDateTime::Now().GetSecond() && _timerOutputFrame)
    {
        // This code must be commented out before release!!!
#ifdef TRACE_SCHEDULE_PERFORMANCE
        logger_base.debug("    Check schedule");
#endif
        last = wxDateTime::Now().GetSecond();
        wxCommandEvent event2(EVT_SCHEDULECHANGED);
        wxPostEvent(this, event2);
    }

    CorrectTimer(rate);

    wxDateTime frameEnd = wxDateTime::UNow();
    long ms = (frameEnd - frameStart).GetMilliseconds().ToLong();

    if (ms > _timer.GetInterval() / 2)
    {
        // we took too long so next frame has to be an output frame
        _timerOutputFrame = true;
#ifdef TRACE_SCHEDULE_PERFORMANCE
        logger_base.debug("Frame took too long %ld > %d so next frame forced to be output", ms, _timer.GetInterval() / 2);
#endif
    }
    else
    {
        // output only occurs on alternate timer events
        _timerOutputFrame = !_timerOutputFrame;
    }

#ifdef TRACE_SCHEDULE_PERFORMANCE
    logger_base.debug("Frame time %ld", ms);
#endif
}

void xScheduleFrame::UpdateSchedule()
{
    if (__schedule == nullptr) return;

#ifdef TRACE_SCHEDULE_PERFORMANCE
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxStopWatch sw;
    logger_base.debug("Updating the schedule.");
#endif

    TreeCtrl_PlayListsSchedules->Freeze();

    int rate = __schedule->CheckSchedule();

#ifdef TRACE_SCHEDULE_PERFORMANCE
    logger_base.debug("Schedule checked %ldms", sw.Time());
#endif

    // highlight the state of all schedule items in the tree
    wxTreeItemIdValue tid;
    auto root = TreeCtrl_PlayListsSchedules->GetRootItem();
    for (auto it = TreeCtrl_PlayListsSchedules->GetFirstChild(root, tid); it != nullptr; it = TreeCtrl_PlayListsSchedules->GetNextChild(root, tid))
    {
        //PlayList* playlist = (PlayList*)((MyTreeItemData*)TreeCtrl_PlayListsSchedules->GetItemData(it))->GetData();

        wxTreeItemIdValue tid2;
        for (auto it2 = TreeCtrl_PlayListsSchedules->GetFirstChild(it, tid2); it2 != nullptr; it2 = TreeCtrl_PlayListsSchedules->GetNextChild(it, tid2))
        {
            Schedule* schedule = (Schedule*)((MyTreeItemData*)TreeCtrl_PlayListsSchedules->GetItemData(it2))->GetData();

            TreeCtrl_PlayListsSchedules->SetItemText(it2, GetScheduleName(schedule, __schedule->GetRunningSchedules()));

            if (__schedule->IsScheduleActive(schedule))
            {
                RunningSchedule* rs = __schedule->GetRunningSchedule();
                if (rs != nullptr && rs->GetPlayList()->IsRunning() &&rs->GetSchedule()->GetId() == schedule->GetId())
                {
                    TreeCtrl_PlayListsSchedules->SetItemBackgroundColour(it2, wxColor(146, 244, 155));
                }
                else
                {
                    RunningSchedule* r = __schedule->GetRunningSchedule(schedule);
                    wxASSERT(r != nullptr);
                    if (r == nullptr || r->IsStopped())
                    {
                        // stopped
                        TreeCtrl_PlayListsSchedules->SetItemBackgroundColour(it2, wxColor(0xe7, 0x74, 0x71));
                    }
                    else
                    {
                        // waiting
                        TreeCtrl_PlayListsSchedules->SetItemBackgroundColour(it2, wxColor(244, 241, 146));
                    }
                }
            }
            else
            {
                TreeCtrl_PlayListsSchedules->SetItemBackgroundColour(it2, *wxWHITE);
            }
        }
    }

#ifdef TRACE_SCHEDULE_PERFORMANCE
    logger_base.debug("    Tree updated %ldms", sw.Time());
#endif

    CorrectTimer(rate);

    // Ensure I am firing on the minute
    if (wxDateTime::Now().GetSecond() != 0)
    {
        _timerSchedule.Stop();
        int time = (60 - wxDateTime::Now().GetSecond()) * 1000;
        if (time == 0) time = 1;
        _timerSchedule.Start(time, false);
    }
    else if (_timerSchedule.GetInterval() != 60000)
    {
        _timerSchedule.Stop();
        _timerSchedule.Start(60000, false);
    }

#ifdef TRACE_SCHEDULE_PERFORMANCE
    logger_base.debug("    Timers sorted %ldms", sw.Time());
#endif

    UpdateUI();

#ifdef TRACE_SCHEDULE_PERFORMANCE
    logger_base.debug("    UI updated %ldms", sw.Time());
#endif

    ValidateWindow();

    TreeCtrl_PlayListsSchedules->Thaw();
    TreeCtrl_PlayListsSchedules->Refresh();

#ifdef TRACE_SCHEDULE_PERFORMANCE
    logger_base.debug("    Schedule updated %ldms", sw.Time());
#endif
}

void xScheduleFrame::On_timerScheduleTrigger(wxTimerEvent& event)
{
    UpdateSchedule();
}

void xScheduleFrame::ValidateWindow()
{
    if (__schedule->GetOptions()->GetAPIOnly())
    {
        MenuItem_WebInterface->Enable(false);
    }
    else
    {
        MenuItem_WebInterface->Enable(true);
    }

    wxTreeItemId treeitem = TreeCtrl_PlayListsSchedules->GetSelection();
    if (IsPlayList(treeitem) || IsSchedule(treeitem))
    {
        Button_Delete->Enable();
        Button_Edit->Enable();
    }
    else
    {
        Button_Delete->Enable(false);
        Button_Edit->Enable(false);
    }

    if (IsPlayList(treeitem))
    {
        Button_Schedule->Enable();
    }
    else
    {
        Button_Schedule->Disable();
    }

    if (__schedule->GetBackgroundPlayList() == nullptr)
    {
        MenuItem_BackgroundPlaylist->Check(false);
    }
    else
    {
        MenuItem_BackgroundPlaylist->Check();
    }
}

void xScheduleFrame::OnMenuItem_OptionsSelected(wxCommandEvent& event)
{
    OptionsDialog dlg(this, __schedule->GetCommandManager(), __schedule->GetOptions());

    int oldport = __schedule->GetOptions()->GetWebServerPort();

    if (dlg.ShowModal() == wxID_OK)
    {
        Schedule::SetCity(__schedule->GetOptions()->GetCity());

        if (oldport != __schedule->GetOptions()->GetWebServerPort())
        {
            delete _webServer;
            _webServer = new WebServer(__schedule->GetOptions()->GetWebServerPort(), __schedule->GetOptions()->GetAPIOnly(),
                __schedule->GetOptions()->GetPassword(), __schedule->GetOptions()->GetPasswordTimeout());
        }
        else
        {
            _webServer->SetAPIOnly(__schedule->GetOptions()->GetAPIOnly());
            _webServer->SetPassword(__schedule->GetOptions()->GetPassword());
            _webServer->SetPasswordTimeout(__schedule->GetOptions()->GetPasswordTimeout());
        }

        __schedule->OptionsChanged();

        CreateButtons();
    }

    AddIPs();

    UpdateUI();
    ValidateWindow();
}

void xScheduleFrame::CreateButton(const std::string& label, const wxColor& c)
{
    wxButton* b = new wxButton(Panel1, ID_BUTTON_USER, label);
    //if (c != *wxBLACK)
    //{
    //    b->SetBackgroundColour(c);
    //}
    FlexGridSizer4->Add(b, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    Connect(ID_BUTTON_USER, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xScheduleFrame::OnButton_UserClick);
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

    // create some default buttons
    if (bs.size() == 0)
    {
        __schedule->GetOptions()->AddButton("Play Selected", "Play selected playlist", "", '~', "green", __schedule->GetCommandManager());
        __schedule->GetOptions()->AddButton("Stop All", "Stop all now", "", '~', "red", __schedule->GetCommandManager());
        __schedule->GetOptions()->AddButton("Reset All Schedules", "Restart all schedules", "", '~', "default", __schedule->GetCommandManager());
        __schedule->GetOptions()->AddButton("Next Step", "Next step in current playlist", "", '~', "default", __schedule->GetCommandManager());
        __schedule->GetOptions()->AddButton("End Gracefully", "Jump to play once at end at end of current step and then stop", "", '~', "red", __schedule->GetCommandManager());
        __schedule->GetOptions()->AddButton("Add 10 Mins To Schedule", "Add to the current schedule n minutes", "10", '~', "default", __schedule->GetCommandManager());

        bs = __schedule->GetOptions()->GetButtons();
    }

    FlexGridSizer4->SetCols(5);
    int rows = bs.size() / 5;
    if (bs.size() % 5 != 0) rows++;
    FlexGridSizer4->SetRows(rows);

    for (auto it = bs.begin(); it != bs.end(); ++it)
    {
        // only show not hidden buttons
        if (!wxString((*it)->GetLabel()).StartsWith("HIDE_"))
        {
            CreateButton((*it)->GetLabel(), (*it)->GetColor());
        }
    }

    SendSizeEvent();

    UpdateUI();
}

void xScheduleFrame::RateNotification(wxCommandEvent& event)
{
    CorrectTimer(event.GetInt());
}

void xScheduleFrame::StatusMsgNotification(wxCommandEvent& event)
{
    SetTempMessage(event.GetString().ToStdString());
}

void xScheduleFrame::RunAction(wxCommandEvent& event)
{
    wxArrayString a = wxSplit(event.GetString(), '|');

    if (a.Count() == 2)
    {
        size_t rate;
        std::string msg;
        __schedule->Action(a[0].ToStdString(), a[1].ToStdString(), "", nullptr, nullptr, rate, msg);
        if (msg != "")
        {
            SetTempMessage(msg);
        }
    }
}

void xScheduleFrame::OnButton_UserClick(wxCommandEvent& event)
{
    PlayList* playlist = nullptr;
    Schedule* schedule = nullptr;

    wxTreeItemId treeitem = TreeCtrl_PlayListsSchedules->GetSelection();
    if (IsPlayList(treeitem))
    {
        playlist = (PlayList*)((MyTreeItemData*)TreeCtrl_PlayListsSchedules->GetItemData(treeitem))->GetData();
    }
    else if (IsSchedule(treeitem))
    {
        schedule = (Schedule*)((MyTreeItemData*)TreeCtrl_PlayListsSchedules->GetItemData(treeitem))->GetData();
        playlist = (PlayList*)((MyTreeItemData*)TreeCtrl_PlayListsSchedules->GetItemData(TreeCtrl_PlayListsSchedules->GetItemParent(treeitem)))->GetData();
    }

    size_t rate = _timer.GetInterval();
    std::string msg = "";
    __schedule->Action(((wxButton*)event.GetEventObject())->GetLabel().ToStdString(), playlist, schedule, rate, msg);

    CorrectTimer(rate);

    UpdateSchedule();
    UpdateUI();
}

void xScheduleFrame::SetTempMessage(const std::string& msg)
{
    _statusSetAt = wxDateTime::Now();
    StatusBar1->SetStatusText(msg);
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

    if (selected >= 0 && p != nullptr && p->GetRunningStep()->GetNameNoTime() != ListView_Running->GetItemText(selected, 0))
    {
        size_t rate;
        std::string msg;
        __schedule->Action("Jump to specified step in current playlist", ListView_Running->GetItemText(selected, 0).ToStdString(), "", p, nullptr, rate, msg);
    }
}

std::string FormatTime(size_t timems, bool ms = false)
{
    if (ms)
    {
        return wxString::Format(wxT("%i:%02i.%03i"), (int)(timems / 60000), (int)((timems % 60000) / 1000), (int)(timems % 1000)).ToStdString();
    }
    else
    {
        return wxString::Format(wxT("%i:%02i"), (int)(timems / 60000), (int)((timems % 60000) / 1000)).ToStdString();
    }
}

void xScheduleFrame::UpdateStatus()
{
#ifdef TRACE_SCHEDULE_PERFORMANCE
    wxStopWatch sw;
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("            Update Status");
#endif

    ListView_Running->Freeze();

    if (StatusBar1->GetStatusText() != "" && (wxDateTime::Now() - _statusSetAt).GetMilliseconds() >  5000)
    {
        StatusBar1->SetStatusText("");
    }

#ifdef TRACE_SCHEDULE_PERFORMANCE
    logger_base.debug("            Status Text %ldms", sw.Time());
#endif

    static int lastcc = -1;
    static int lastid = -1;
    static int lastrunning = -1;
    static int laststeps = -1;
    PlayList* p = __schedule->GetRunningPlayList();

    if (p == nullptr)
    {
        wxTreeItemId treeitem = TreeCtrl_PlayListsSchedules->GetSelection();
        if (treeitem.IsOk() && IsPlayList(treeitem))
        {
            p = (PlayList*)((MyTreeItemData*)TreeCtrl_PlayListsSchedules->GetItemData(treeitem))->GetData();
        }
    }

#ifdef TRACE_SCHEDULE_PERFORMANCE
    logger_base.debug("            Got selected playlist %ldms", sw.Time());
#endif

    if (p == nullptr)
    {
        ListView_Running->DeleteAllItems();
        lastcc = -1;
        lastid = -1;
        lastrunning = -1;
        laststeps = -1;
    }
    else
    {
        if (p->GetId() != lastid ||
            p->GetChangeCount() != lastcc ||
            (int)p->IsRunning() != lastrunning ||
            p->GetSteps().size() != laststeps)
        {
            lastcc = p->GetChangeCount();
            lastid = p->GetId();
            lastrunning = (int)p->IsRunning();
            laststeps = p->GetSteps().size();

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
            bool didloop;
            next = p->GetNextStep(didloop);
        }

        bool currenthighlighted = false;
        bool nexthighlighted = false;

        if (step != nullptr)
        {
            for (int i = 0; i < ListView_Running->GetItemCount(); i++)
            {
                if (!currenthighlighted && ListView_Running->GetItemText(i, 0) == step->GetNameNoTime())
                {
                    currenthighlighted = true;
                    ListView_Running->SetItem(i, 2, step->GetStatus());
                    ListView_Running->SetItemBackgroundColour(i, wxColor(146,244,155));
                }
                else
                {
                    if (next != nullptr && !nexthighlighted && next->GetNameNoTime() == ListView_Running->GetItemText(i,0))
                    {
                        nexthighlighted = true;
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
        if (ListView_Running->GetColumnWidth(0) < 50)
            ListView_Running->SetColumnWidth(0, 50);
        ListView_Running->SetColumnWidth(1, wxLIST_AUTOSIZE);
        if (ListView_Running->GetColumnWidth(1) < 80)
            ListView_Running->SetColumnWidth(1, 80);
        ListView_Running->SetColumnWidth(2, wxLIST_AUTOSIZE);
        if (ListView_Running->GetColumnWidth(2) < 250)
            ListView_Running->SetColumnWidth(2, 250);
    }

    ListView_Running->Thaw();
    ListView_Running->Refresh();

#ifdef TRACE_SCHEDULE_PERFORMANCE
    logger_base.debug("            Updated running listview %ldms", sw.Time());
#endif

    static int saved = -1;
    static int otl = -1;
    static int scheduled = -1;
    static int random = -1;
    static int plloop = -1;
    static int steploop = -1;
    static int playing = -1;

    if (__schedule->IsOutputToLights())
    {
        if (__schedule->GetManualOutputToLights() == -1)
        {
            if (otl != 2)
                BitmapButton_OutputToLights->SetBitmap(_otlautoon);
            otl = 2;
        }
        else
        {
            if (otl != 1)
                BitmapButton_OutputToLights->SetBitmap(_otlon);
            otl = 1;
        }
        if (BitmapButton_OutputToLights->GetToolTipText() != "Lights output ON.")
            BitmapButton_OutputToLights->SetToolTip("Lights output ON.");
    }
    else
    {
        if (__schedule->GetManualOutputToLights() == -1)
        {
            if (otl != 3)
                BitmapButton_OutputToLights->SetBitmap(_otlautooff);
            otl = 3;
        }
        else
        {
            if (otl != 0)
                BitmapButton_OutputToLights->SetBitmap(_otloff);
            otl = 0;
        }
        if (BitmapButton_OutputToLights->GetToolTipText() != "Lights output OFF.")
            BitmapButton_OutputToLights->SetToolTip("Lights output OFF.");
    }

    if (__schedule->IsDirty())
    {
        if (saved != 1)
            BitmapButton_Unsaved->SetBitmap(_save);
        if (BitmapButton_Unsaved->GetToolTipText() != "Unsaved changes.")
            BitmapButton_Unsaved->SetToolTip("Unsaved changes.");
        saved = 1;
    }
    else
    {
        if (saved != 0)
            BitmapButton_Unsaved->SetBitmap(_inactive);
        saved = 0;
    }

    if (p == nullptr || !p->IsRunning())
    {
        if (scheduled != 0)
            BitmapButton_IsScheduled->SetBitmap(_inactive);
        BitmapButton_IsScheduled->SetToolTip("");
        scheduled = 0;

        if (playing != 0)
            BitmapButton_Playing->SetBitmap(_idle);
        if (BitmapButton_Playing->GetToolTipText() != "Idle.")
            BitmapButton_Playing->SetToolTip("Idle.");
        playing = 0;

        if (plloop != 2)
            BitmapButton_PLLoop->SetBitmap(_inactive);
        plloop = 2;
        BitmapButton_PLLoop->SetToolTip("");

        if (steploop != 2)
            BitmapButton_StepLoop->SetBitmap(_inactive);
        steploop = 2;
        BitmapButton_StepLoop->SetToolTip("");

        if (random != 2)
            BitmapButton_Random->SetBitmap(_inactive);
        BitmapButton_Random->SetToolTip("");
        random = 2;
    }
    else
    {
        if (__schedule->GetMode() == SYNCMODE::FPPSLAVE || __schedule->GetMode() == SYNCMODE::FPPUNICASTSLAVE || __schedule->GetMode() == SYNCMODE::OSCSLAVE || __schedule->GetMode() == SYNCMODE::MIDISLAVE)
        {
            if (scheduled != 13)
                BitmapButton_IsScheduled->SetBitmap(_falconremote);
            if (__schedule->GetMode() == SYNCMODE::OSCSLAVE)
            {
                if (BitmapButton_IsScheduled->GetToolTipText() != "OSC remote.")
                    BitmapButton_IsScheduled->SetToolTip("OSC remote.");
            }
            else if (__schedule->GetMode() == SYNCMODE::MIDISLAVE)
            {
                if (BitmapButton_IsScheduled->GetToolTipText() != "MIDI remote.")
                    BitmapButton_IsScheduled->SetToolTip("MIDI remote.");
            }
            else
            {
                if (BitmapButton_IsScheduled->GetToolTipText() != "FPP remote.")
                    BitmapButton_IsScheduled->SetToolTip("FPP remote.");
            }
            scheduled = 13;
        }
        else if (__schedule->IsCurrentPlayListScheduled())
        {
            if (scheduled != 1)
                BitmapButton_IsScheduled->SetBitmap(_scheduled);
            if (BitmapButton_IsScheduled->GetToolTipText() != "Scheduled playlist playing.")
                BitmapButton_IsScheduled->SetToolTip("Scheduled playlist playing.");
            scheduled = 1;
        }
        else if (__schedule->IsQueuedPlaylistRunning())
        {
            if (scheduled != 4)
                BitmapButton_IsScheduled->SetBitmap(_queued);
            if (BitmapButton_IsScheduled->GetToolTipText() != "Queued playlist playing.")
                BitmapButton_IsScheduled->SetToolTip("Queued playlist playing.");
            scheduled = 4;
        }
        else
        {
            if (scheduled != 2)
                BitmapButton_IsScheduled->SetBitmap(_notscheduled);
            if (BitmapButton_IsScheduled->GetToolTipText() != "Manually started playlist playing.")
                BitmapButton_IsScheduled->SetToolTip("Manually started playlist playing.");
            scheduled = 2;
        }

        if (p->IsPaused())
        {
            if (playing != 2)
                BitmapButton_Playing->SetBitmap(_paused);
            if (BitmapButton_Playing->GetToolTipText() != "Paused.")
                BitmapButton_Playing->SetToolTip("Paused.");
            playing = 2;
        }
        else
        {
            if (playing != 1)
                BitmapButton_Playing->SetBitmap(_playing);
            if (BitmapButton_Playing->GetToolTipText() != "Playing.")
                BitmapButton_Playing->SetToolTip("Playing.");
            playing = 1;
        }

        if (p->IsLooping())
        {
            if (plloop != 1)
                BitmapButton_PLLoop->SetBitmap(_pllooped);
            if (BitmapButton_PLLoop->GetToolTipText() != "Playlist looping.")
                BitmapButton_PLLoop->SetToolTip("Playlist looping.");
            plloop = 1;
        }
        else
        {
            if (plloop != 0)
                BitmapButton_PLLoop->SetBitmap(_plnotlooped);
            if (BitmapButton_PLLoop->GetToolTipText() != "Playlist not looping.")
                BitmapButton_PLLoop->SetToolTip("Playlist not looping.");
            plloop = 0;
        }

        if (p->IsStepLooping())
        {
            if (steploop != 1)
                BitmapButton_StepLoop->SetBitmap(_plsteplooped);
            if (BitmapButton_StepLoop->GetToolTipText() != "Playlist step looping.")
                BitmapButton_StepLoop->SetToolTip("Playlist step looping.");
            steploop = 1;
        }
        else
        {
            if (steploop != 0)
                BitmapButton_StepLoop->SetBitmap(_plstepnotlooped);
            if (BitmapButton_StepLoop->GetToolTipText() != "Playlist step not looping.")
                BitmapButton_StepLoop->SetToolTip("Playlist step not looping.");
            steploop = 0;
        }

        if (p->IsRandom())
        {
            if (random != 1)
                BitmapButton_Random->SetBitmap(_random);
            if (BitmapButton_Random->GetToolTipText() != "Randomly choosing steps.")
                BitmapButton_Random->SetToolTip("Randomly choosing steps.");
            random = 1;
        }
        else
        {
            if (random != 0)
                BitmapButton_Random->SetBitmap(_notrandom);
            if (BitmapButton_Random->GetToolTipText() != "Sequentially played steps.")
                BitmapButton_Random->SetToolTip("Sequentially played steps.");
            random = 0;
        }
    }

#ifdef TRACE_SCHEDULE_PERFORMANCE
    logger_base.debug("            Updated toolbar %ldms", sw.Time());
#endif

    // update each button based on current status

    PlayList* playlist = nullptr;
    Schedule* schedule = nullptr;

    wxTreeItemId treeitem = TreeCtrl_PlayListsSchedules->GetSelection();

    if (IsPlayList(treeitem))
    {
        playlist = (PlayList*)((MyTreeItemData*)TreeCtrl_PlayListsSchedules->GetItemData(treeitem))->GetData();
    }
    else if (IsSchedule(treeitem))
    {
        schedule = (Schedule*)((MyTreeItemData*)TreeCtrl_PlayListsSchedules->GetItemData(treeitem))->GetData();
        playlist = (PlayList*)((MyTreeItemData*)TreeCtrl_PlayListsSchedules->GetItemData(TreeCtrl_PlayListsSchedules->GetItemParent(treeitem)))->GetData();
    }

    auto buttons = Panel1->GetChildren();
    for (auto it = buttons.begin(); it != buttons.end(); ++it)
    {
        UserButton* b = __schedule->GetOptions()->GetButton((*it)->GetLabel().ToStdString());

        if (b != nullptr)
        {
            std::string command = b->GetCommand();
            std::string parameters = b->GetParameters();
            Command* c = b->GetCommandObj();
            std::string msg;
            if (c != nullptr && c->IsValid(parameters, playlist, schedule, __schedule, msg, __schedule->IsQueuedPlaylistRunning()))
            {
                (*it)->Enable();
            }
            else
            {
                (*it)->Enable(false);
            }
        }
        else
        {
            (*it)->Enable(false);
        }
    }

#ifdef TRACE_SCHEDULE_PERFORMANCE
    logger_base.debug("            Updated buttons %ldms", sw.Time());
#endif

    Custom_Volume->SetValue(__schedule->GetVolume());

    StaticText_Time->SetLabel(wxDateTime::Now().FormatTime());

    SendStatus();

#ifdef TRACE_SCHEDULE_PERFORMANCE
    logger_base.debug("            Status Sent %ldms", sw.Time());
#endif
}

void xScheduleFrame::OnBitmapButton_OutputToLightsClick(wxCommandEvent& event)
{
    __schedule->ManualOutputToLightsClick(this);
    UpdateUI();
}

void xScheduleFrame::OnBitmapButton_RandomClick(wxCommandEvent& event)
{
    std::string msg = "";
    __schedule->ToggleCurrentPlayListRandom(msg);
    UpdateUI();
}

void xScheduleFrame::OnBitmapButton_PlayingClick(wxCommandEvent& event)
{
    std::string msg = "";
    __schedule->ToggleCurrentPlayListPause(msg);
    UpdateUI();
}

void xScheduleFrame::OnBitmapButton_PLLoopClick(wxCommandEvent& event)
{
    std::string msg = "";
    __schedule->ToggleCurrentPlayListLoop(msg);
    UpdateUI();
}

void xScheduleFrame::OnBitmapButton_StepLoopClick(wxCommandEvent& event)
{
    std::string msg = "";
    __schedule->ToggleCurrentPlayListStepLoop(msg);
    UpdateUI();
}

void xScheduleFrame::OnBitmapButton_IsScheduledClick(wxCommandEvent& event)
{
    // do nothing
    UpdateUI();
}

void xScheduleFrame::OnBitmapButton_UnsavedClick(wxCommandEvent& event)
{
    __schedule->Save();
    UpdateUI();
}

void xScheduleFrame::CreateDebugReport(wxDebugReportCompress *report) {

    std::string cb = "Prompt user";
    if (__schedule != nullptr && __schedule->GetOptions() != nullptr)
    {
        cb = __schedule->GetOptions()->GetCrashBehaviour();
    }

    report->Process();

    if (cb == "Silently exit after sending crash log" || (cb == "Prompt user" && wxDebugReportPreviewStd().Show(*report))) {
        if (cb != "Silently exit after sending crash log")
        {
            wxMessageBox("Crash report saved to " + report->GetCompressedFileName());
        }
        SendReport("crashUpload", *report);
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

    wxDateTime now = wxDateTime::Now();
    int millis = wxGetUTCTimeMillis().GetLo() % 1000;
    wxString ts = wxString::Format("%04d-%02d-%02d_%02d-%02d-%02d-%03d", now.GetYear(), now.GetMonth()+1, now.GetDay(), now.GetHour(), now.GetMinute(), now.GetSecond(), millis);

    wxString fn = wxString::Format("xSchedule-%s_%s_%s_%s.zip", wxPlatformInfo::Get().GetOperatingSystemFamilyName().c_str(), xlights_version_string, GetBitness(), ts);
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

void xScheduleFrame::OnKeyDown(wxKeyEvent& event)
{
    if (HandleSpecialKeys(event) || HandleHotkeys(event))
    {
        UpdateUI();
        UpdateSchedule();
        ValidateWindow();
    }
    else
    {
        event.Skip();
    }
}

bool xScheduleFrame::HandleSpecialKeys(wxKeyEvent& event)
{
    if (event.GetSkipped()) return false;

    wxChar uc = event.GetUnicodeKey();

    if (event.GetKeyCode() == WXK_DELETE)
    {
        DeleteSelectedItem();
        return true;
    }
    else if (uc == (wxChar)WXK_CONTROL_C || uc == 'C' || uc == 'c')
    {
        if (event.CmdDown() || event.ControlDown()) {
            DoCopy();
            return true;
        }
    }
    else if (uc == (wxChar)WXK_CONTROL_V || uc == 'V' || uc == 'v')
    {
        if (event.CmdDown() || event.ControlDown()) {
            DoPaste();
            return true;
        }
    }

    return false;
}

bool xScheduleFrame::HandleHotkeys(wxKeyEvent& event)
{
    auto bs = __schedule->GetOptions()->GetButtons();

    for (auto it = bs.begin(); it != bs.end(); ++it)
    {
        auto hk = (*it)->GetHotkey();
        if (hk == (char)event.GetRawKeyCode())
        {
            PlayList* playlist = nullptr;
            Schedule* schedule = nullptr;

            wxTreeItemId treeitem = TreeCtrl_PlayListsSchedules->GetSelection();

            if (IsPlayList(treeitem))
            {
                playlist = (PlayList*)((MyTreeItemData*)TreeCtrl_PlayListsSchedules->GetItemData(treeitem))->GetData();
            }
            else if (IsSchedule(treeitem))
            {
                schedule = (Schedule*)((MyTreeItemData*)TreeCtrl_PlayListsSchedules->GetItemData(treeitem))->GetData();
            }

            size_t rate = _timer.GetInterval();
            std::string msg = "";
            __schedule->Action((*it)->GetLabel(), playlist, schedule, rate, msg);

            CorrectTimer(rate);

            UpdateSchedule();
            return true;
        }
    }

    return false;
}

void xScheduleFrame::CorrectTimer(int rate)
{
    if (rate == 0) rate = 50;
    if ((rate - __schedule->GetTimerAdjustment()) / 2 != _timer.GetInterval())
    {
        _timer.Stop();
        _timer.Start((rate - __schedule->GetTimerAdjustment()) / 2);
    }
}

void xScheduleFrame::OnBitmapButton_VolumeDownClick(wxCommandEvent& event)
{
    __schedule->AdjustVolumeBy(-10);
    UpdateUI();
}

void xScheduleFrame::OnBitmapButton_VolumeUpClick(wxCommandEvent& event)
{
    __schedule->AdjustVolumeBy(10);
    UpdateUI();
}

void xScheduleFrame::OnCustom_VolumeLeftDown(wxMouseEvent& event)
{
    __schedule->ToggleMute();
    UpdateUI();
}

void xScheduleFrame::OnMenuItem_StandaloneSelected(wxCommandEvent& event)
{
    __schedule->SetMode(SYNCMODE::STANDALONE);
    UpdateUI();
}

void xScheduleFrame::OnMenuItem_FPPMasterSelected(wxCommandEvent& event)
{
    __schedule->SetMode(SYNCMODE::FPPMASTER);
    UpdateUI();
}

void xScheduleFrame::OnMenuItem_FPPRemoteSelected(wxCommandEvent& event)
{
    __schedule->SetMode(SYNCMODE::FPPSLAVE);
    UpdateUI();
}

void xScheduleFrame::OnMenuItem_WebInterfaceSelected(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser(wxString::Format("http://localhost:%d", __schedule->GetOptions()->GetWebServerPort()));
}

void xScheduleFrame::OnMenuItem_AddPlayListSelected(wxCommandEvent& event)
{
    AddPlayList();
    UpdateUI();
    ValidateWindow();
}

void xScheduleFrame::AddPlayList(bool forceadvanced)
{
    PlayList* playlist = new PlayList();
    if (playlist->Configure(this, __schedule->GetOutputManager(), forceadvanced || __schedule->GetOptions()->IsAdvancedMode()) == nullptr)
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
    AddIPs();
}

void xScheduleFrame::OnButton_AddClick(wxCommandEvent& event)
{
    AddPlayList();
    UpdateUI();
    ValidateWindow();
}

void xScheduleFrame::OnButton_EditClick(wxCommandEvent& event)
{
    EditSelectedItem();
    UpdateUI();
    ValidateWindow();
}

void xScheduleFrame::OnButton_DeleteClick(wxCommandEvent& event)
{
    DeleteSelectedItem();
    UpdateUI();
    ValidateWindow();
}

void xScheduleFrame::EditSelectedItem(bool forceadvanced)
{
    wxTreeItemId treeitem = TreeCtrl_PlayListsSchedules->GetSelection();
    if (IsPlayList(treeitem))
    {
        PlayList* playlist = (PlayList*)((MyTreeItemData*)TreeCtrl_PlayListsSchedules->GetItemData(treeitem))->GetData();
        if (playlist->Configure(this, __schedule->GetOutputManager(), forceadvanced || __schedule->GetOptions()->IsAdvancedMode()) != nullptr)
        {
            TreeCtrl_PlayListsSchedules->SetItemText(treeitem, playlist->GetName());
        }
    }
    else if (IsSchedule(treeitem))
    {
        Schedule* schedule = (Schedule*)((MyTreeItemData*)TreeCtrl_PlayListsSchedules->GetItemData(treeitem))->GetData();
        if (schedule->Configure(this) != nullptr)
        {
            TreeCtrl_PlayListsSchedules->SetItemText(treeitem, GetScheduleName(schedule, __schedule->GetRunningSchedules()));
            auto rs = __schedule->GetRunningSchedule(schedule);
            if (rs != nullptr) rs->Reset();
        }
    }

    AddIPs();
}

void xScheduleFrame::OnMenu_OutputProcessingSelected(wxCommandEvent& event)
{
    OutputProcessingDialog dlg(this, __schedule->GetOutputManager(), __schedule->GetOutputProcessing());

    if (dlg.ShowModal() == wxID_OK)
    {
        __schedule->OutputProcessingChanged();
    }

    UpdateUI();
    ValidateWindow();
}

// This is called when anything interesting happens in schedule manager
void xScheduleFrame::ScheduleChange(wxCommandEvent& event)
{
    UpdateUI();
}

void xScheduleFrame::DoCheckSchedule(wxCommandEvent& event)
{
    UpdateSchedule();
    UpdateUI();
}

void xScheduleFrame::DoAction(wxCommandEvent& event)
{
    ActionMessageData* amd = (ActionMessageData*)event.GetClientData();

    PlayList* playlist = nullptr;
    Schedule* schedule = nullptr;

    wxTreeItemId treeitem = TreeCtrl_PlayListsSchedules->GetSelection();
    if (IsPlayList(treeitem))
    {
        playlist = (PlayList*)((MyTreeItemData*)TreeCtrl_PlayListsSchedules->GetItemData(treeitem))->GetData();
    }
    else if (IsSchedule(treeitem))
    {
        schedule = (Schedule*)((MyTreeItemData*)TreeCtrl_PlayListsSchedules->GetItemData(treeitem))->GetData();
        playlist = (PlayList*)((MyTreeItemData*)TreeCtrl_PlayListsSchedules->GetItemData(TreeCtrl_PlayListsSchedules->GetItemParent(treeitem)))->GetData();
    }

    size_t rate = _timer.GetInterval();
    std::string msg = "";

    __schedule->Action(amd->_command, amd->_parameters, amd->_data, playlist, schedule, rate, msg);

    delete amd;
}

void xScheduleFrame::UpdateUI()
{
#ifdef TRACE_SCHEDULE_PERFORMANCE
    wxStopWatch sw;
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("        Update UI");
#endif

    StaticText_PacketsPerSec->SetLabel(wxString::Format("Packets/Sec: %d", __schedule->GetPPS()));

    UpdateStatus();

#ifdef TRACE_SCHEDULE_PERFORMANCE
    logger_base.debug("        Status updated %ldms", sw.Time());
#endif

    Brightness->SetValue(__schedule->GetBrightness());

    if (__schedule->GetWebRequestToggle())
    {
        if (!_webIconDisplayed)
        {
            StaticBitmap_WebIcon->SetBitmap(_webicon);
            _webIconDisplayed = true;
        }
    }
    else
    {
        if (_webIconDisplayed)
        {
            StaticBitmap_WebIcon->SetBitmap(_nowebicon);
            _webIconDisplayed = false;
        }
    }

#ifdef TRACE_SCHEDULE_PERFORMANCE
    logger_base.debug("        Web request status updated %ldms", sw.Time());
#endif

    if (!_suspendOTL)
    {
        if (!__schedule->GetOptions()->IsSendOffWhenNotRunning() && __schedule->GetManualOutputToLights() == -1)
        {
            if (__schedule->GetRunningPlayList() == nullptr && !__schedule->IsXyzzy())
            {
                if (__schedule->IsOutputToLights())
                    __schedule->SetOutputToLights(this, false, false);
            }
            else
            {
                if (!__schedule->IsOutputToLights())
                    __schedule->SetOutputToLights(this, true, false);
            }
        }
        else
        {
            if (__schedule->GetManualOutputToLights() == 0)
            {
                if (__schedule->IsOutputToLights())
                    __schedule->SetOutputToLights(this, false, false);
            }
            else if (__schedule->GetManualOutputToLights() == 1)
            {
                if (!__schedule->IsOutputToLights())
                    __schedule->SetOutputToLights(this, true, false);
            }
        }
    }
    else
    {
        if (__schedule->IsOutputToLights())
            __schedule->SetOutputToLights(this, false, false);
    }

#ifdef TRACE_SCHEDULE_PERFORMANCE
    logger_base.debug("        Managed output to lights %ldms", sw.Time());
#endif

    if (__schedule->GetMode() == SYNCMODE::FPPMASTER)
    {
        MenuItem_FPPMaster->Check(true);
        MenuItem_FPPRemote->Check(false);
        MenuItem_Standalone->Check(false);
        MenuItem_FPPUnicastRemote->Check(false);
        MenuItem_OSCMaster->Check(false);
        MenuItem_FPPOSCMaster->Check(false);
        MenuItem_OSCRemote->Check(false);
        MenuItem_ARTNetTimeCodeMaster->Check(false);
        MenuItem_ARTNetTimeCodeSlave->Check(false);
        MenuItem_MIDITimeCodeSlave->Check(false);
        MenuItem_MIDITimeCodeMaster->Check(false);
    }
    else if (__schedule->GetMode() == SYNCMODE::FPPOSCMASTER)
    {
        MenuItem_FPPMaster->Check(false);
        MenuItem_FPPRemote->Check(false);
        MenuItem_Standalone->Check(false);
        MenuItem_FPPUnicastRemote->Check(false);
        MenuItem_OSCMaster->Check(false);
        MenuItem_FPPOSCMaster->Check(true);
        MenuItem_OSCRemote->Check(false);
        MenuItem_ARTNetTimeCodeMaster->Check(false);
        MenuItem_ARTNetTimeCodeSlave->Check(false);
        MenuItem_MIDITimeCodeSlave->Check(false);
        MenuItem_MIDITimeCodeMaster->Check(false);
    }
    else if (__schedule->GetMode() == SYNCMODE::OSCMASTER)
    {
        MenuItem_FPPMaster->Check(false);
        MenuItem_FPPRemote->Check(false);
        MenuItem_Standalone->Check(false);
        MenuItem_FPPUnicastRemote->Check(false);
        MenuItem_OSCMaster->Check(true);
        MenuItem_FPPOSCMaster->Check(false);
        MenuItem_OSCRemote->Check(false);
        MenuItem_ARTNetTimeCodeMaster->Check(false);
        MenuItem_ARTNetTimeCodeSlave->Check(false);
        MenuItem_MIDITimeCodeSlave->Check(false);
        MenuItem_MIDITimeCodeMaster->Check(false);
    }
    else if (__schedule->GetMode() == SYNCMODE::MIDIMASTER)
    {
        MenuItem_FPPMaster->Check(false);
        MenuItem_FPPRemote->Check(false);
        MenuItem_Standalone->Check(false);
        MenuItem_FPPUnicastRemote->Check(false);
        MenuItem_OSCMaster->Check(false);
        MenuItem_FPPOSCMaster->Check(false);
        MenuItem_OSCRemote->Check(false);
        MenuItem_ARTNetTimeCodeMaster->Check(false);
        MenuItem_ARTNetTimeCodeSlave->Check(false);
        MenuItem_MIDITimeCodeSlave->Check(false);
        MenuItem_MIDITimeCodeMaster->Check(true);
    }
    else if (__schedule->GetMode() == SYNCMODE::MIDISLAVE)
    {
        MenuItem_FPPMaster->Check(false);
        MenuItem_FPPRemote->Check(false);
        MenuItem_Standalone->Check(false);
        MenuItem_FPPUnicastRemote->Check(false);
        MenuItem_OSCMaster->Check(false);
        MenuItem_FPPOSCMaster->Check(false);
        MenuItem_OSCRemote->Check(false);
        MenuItem_ARTNetTimeCodeMaster->Check(false);
        MenuItem_ARTNetTimeCodeSlave->Check(false);
        MenuItem_MIDITimeCodeSlave->Check(true);
        MenuItem_MIDITimeCodeMaster->Check(false);
    }
    else if (__schedule->GetMode() == SYNCMODE::FPPSLAVE)
    {
        MenuItem_FPPMaster->Check(false);
        MenuItem_FPPRemote->Check(true);
        MenuItem_Standalone->Check(false);
        MenuItem_FPPUnicastRemote->Check(false);
        MenuItem_OSCMaster->Check(false);
        MenuItem_FPPOSCMaster->Check(false);
        MenuItem_OSCRemote->Check(false);
        MenuItem_ARTNetTimeCodeMaster->Check(false);
        MenuItem_ARTNetTimeCodeSlave->Check(false);
        MenuItem_MIDITimeCodeSlave->Check(false);
        MenuItem_MIDITimeCodeMaster->Check(false);
    }
    else if (__schedule->GetMode() == SYNCMODE::OSCSLAVE)
    {
        MenuItem_FPPMaster->Check(false);
        MenuItem_FPPRemote->Check(false);
        MenuItem_Standalone->Check(false);
        MenuItem_FPPUnicastRemote->Check(false);
        MenuItem_OSCMaster->Check(false);
        MenuItem_FPPOSCMaster->Check(false);
        MenuItem_OSCRemote->Check(true);
        MenuItem_ARTNetTimeCodeMaster->Check(false);
        MenuItem_ARTNetTimeCodeSlave->Check(false);
        MenuItem_MIDITimeCodeSlave->Check(false);
        MenuItem_MIDITimeCodeMaster->Check(false);
    }
    else if (__schedule->GetMode() == SYNCMODE::FPPUNICASTSLAVE)
    {
        MenuItem_FPPMaster->Check(false);
        MenuItem_FPPRemote->Check(false);
        MenuItem_Standalone->Check(false);
        MenuItem_FPPUnicastRemote->Check(true);
        MenuItem_OSCMaster->Check(false);
        MenuItem_FPPOSCMaster->Check(false);
        MenuItem_OSCRemote->Check(false);
        MenuItem_ARTNetTimeCodeMaster->Check(false);
        MenuItem_ARTNetTimeCodeSlave->Check(false);
        MenuItem_MIDITimeCodeSlave->Check(false);
        MenuItem_MIDITimeCodeMaster->Check(false);
    }
    else if (__schedule->GetMode() == SYNCMODE::ARTNETSLAVE)
    {
        MenuItem_FPPMaster->Check(false);
        MenuItem_FPPRemote->Check(false);
        MenuItem_Standalone->Check(false);
        MenuItem_FPPUnicastRemote->Check(false);
        MenuItem_OSCMaster->Check(false);
        MenuItem_FPPOSCMaster->Check(false);
        MenuItem_OSCRemote->Check(false);
        MenuItem_ARTNetTimeCodeMaster->Check(false);
        MenuItem_ARTNetTimeCodeSlave->Check(true);
        MenuItem_MIDITimeCodeSlave->Check(false);
        MenuItem_MIDITimeCodeMaster->Check(false);
    }
    else if (__schedule->GetMode() == SYNCMODE::ARTNETMASTER)
    {
        MenuItem_FPPMaster->Check(false);
        MenuItem_FPPRemote->Check(false);
        MenuItem_Standalone->Check(false);
        MenuItem_FPPUnicastRemote->Check(false);
        MenuItem_OSCMaster->Check(false);
        MenuItem_FPPOSCMaster->Check(false);
        MenuItem_OSCRemote->Check(false);
        MenuItem_ARTNetTimeCodeMaster->Check(true);
        MenuItem_ARTNetTimeCodeSlave->Check(false);
        MenuItem_MIDITimeCodeSlave->Check(false);
        MenuItem_MIDITimeCodeMaster->Check(false);
    }
    else
    {
        MenuItem_FPPMaster->Check(false);
        MenuItem_FPPRemote->Check(false);
        MenuItem_Standalone->Check(true);
        MenuItem_FPPUnicastRemote->Check(false);
        MenuItem_OSCMaster->Check(false);
        MenuItem_FPPOSCMaster->Check(false);
        MenuItem_OSCRemote->Check(false);
        MenuItem_ARTNetTimeCodeMaster->Check(false);
        MenuItem_ARTNetTimeCodeSlave->Check(false);
        MenuItem_MIDITimeCodeSlave->Check(false);
        MenuItem_MIDITimeCodeMaster->Check(false);
    }

#ifdef TRACE_SCHEDULE_PERFORMANCE
    logger_base.debug("        Updated mode %ldms", sw.Time());
#endif

    if (_pinger != nullptr)
    {
        auto pingresults = _pinger->GetPingers();
        for (auto it = pingresults.begin(); it != pingresults.end(); ++it)
        {
            // find it in the list
            int item = -1;
            for (int i = 0; i < ListView_Ping->GetItemCount(); i++)
            {
                if (ListView_Ping->GetItemText(i) == (*it)->GetName())
                {
                    item = i;
                    break;
                }
            }
            // if not there ... add it
            if (item == -1)
            {
                item = ListView_Ping->GetItemCount();
                ListView_Ping->InsertItem(ListView_Ping->GetItemCount(), (*it)->GetName());
                ListView_Ping->SetColumnWidth(0, wxLIST_AUTOSIZE);
            }

            // update the colour
            if (item >= 0)
            {
                switch ((*it)->GetPingResult())
                {
                case PING_OK:
                case PING_OPEN:
                case PING_OPENED:
                case PING_WEBOK:
                    ListView_Ping->SetItemTextColour(item, *wxBLACK);
                    ListView_Ping->SetItemBackgroundColour(item, *wxGREEN);
                    break;
                case PING_ALLFAILED:
                    ListView_Ping->SetItemTextColour(item, *wxWHITE);
                    ListView_Ping->SetItemBackgroundColour(item, *wxRED);
                    break;
                case PING_UNAVAILABLE:
                    ListView_Ping->SetItemTextColour(item, *wxBLACK);
                    ListView_Ping->SetItemBackgroundColour(item, *wxWHITE);
                    break;
                case PING_UNKNOWN:
                    ListView_Ping->SetItemTextColour(item, *wxBLACK);
                    ListView_Ping->SetItemBackgroundColour(item, wxColour(255, 128, 0));
                    break;
                }
            }
        }

        // remove anything in the tree which isnt in the results
        for (int i = 0; i < ListView_Ping->GetItemCount(); i ++)
        {
            bool found = false;
            for (auto it = pingresults.begin(); !found && it != pingresults.end(); ++it)
            {
                if (ListView_Ping->GetItemText(i) == (*it)->GetName())
                {
                    found = true;
                }
            }
            if (!found)
            {
                ListView_Ping->DeleteItem(i);
                i--;
            }
        }
    }

#ifdef TRACE_SCHEDULE_PERFORMANCE
    logger_base.debug("        Updated ping status %ldms", sw.Time());
#endif

    ValidateWindow();

// this may be the performance issue cause if it triggers an update event which is then slow !!!!!!
//    Refresh();
}

void xScheduleFrame::OnMenuItem_BackgroundPlaylistSelected(wxCommandEvent& event)
{
    int bid = -1;
    if (__schedule->GetBackgroundPlayList() != nullptr)
    {
        bid = __schedule->GetBackgroundPlayList()->GetId();
    }

    BackgroundPlaylistDialog dlg(this, bid, __schedule->GetPlayLists());

    if (dlg.ShowModal() == wxID_OK)
    {
        __schedule->SetBackgroundPlayList(__schedule->GetPlayList(bid));
    }

    ValidateWindow();
}

void xScheduleFrame::OnMenuItem_CheckScheduleSelected(wxCommandEvent& event)
{
    __schedule->CheckScheduleIntegrity(true);
}

void xScheduleFrame::OnButton_ScheduleClick(wxCommandEvent& event)
{
    AddSchedule();
}

void xScheduleFrame::AddSchedule()
{
    wxTreeItemId treeitem = TreeCtrl_PlayListsSchedules->GetSelection();

    if (IsPlayList(treeitem))
    {
        Schedule* schedule = new Schedule();
        if (schedule->Configure(this) == nullptr)
        {
            delete schedule;
        }
        else
        {
            wxTreeItemId  newitem = TreeCtrl_PlayListsSchedules->AppendItem(treeitem, GetScheduleName(schedule, __schedule->GetRunningSchedules()), -1, -1, new MyTreeItemData(schedule));
            PlayList* playlist = (PlayList*)((MyTreeItemData*)TreeCtrl_PlayListsSchedules->GetItemData(treeitem))->GetData();
            TreeCtrl_PlayListsSchedules->Expand(treeitem);
            TreeCtrl_PlayListsSchedules->EnsureVisible(newitem);
            playlist->AddSchedule(schedule);
        }
	}
    UpdateSchedule();
    UpdateUI();
}

void xScheduleFrame::OnMenuItem_MatricesSelected(wxCommandEvent& event)
{
    auto matrices = __schedule->GetOptions()->GetMatrices();
    MatricesDialog dlg(this, __schedule->GetOutputManager(), matrices);

    if (dlg.ShowModal() == wxID_OK)
    {
        // it is updated directly
        __schedule->SetDirty();
    }

    UpdateUI();
    ValidateWindow();
}

void xScheduleFrame::OnMenuItem_ImportxLightsSelected(wxCommandEvent& event)
{
    if (FileDialog1->ShowModal() == wxID_OK)
    {
        __schedule->ImportxLightsSchedule(FileDialog1->GetPath().ToStdString());
        UpdateTree();
        UpdateUI();
        ValidateWindow();
    }
}

void xScheduleFrame::OnMenuItem_VirtualMatricesSelected(wxCommandEvent& event)
{
    // suspend output to lights as we may make a change not compatible with outputting to lights
    bool ol = __schedule->IsOutputToLights();
    _suspendOTL = true;
    if (ol)
    {
        __schedule->SetOutputToLights(this, false, true);
    }

    auto vmatrices = __schedule->GetOptions()->GetVirtualMatrices();
    VirtualMatricesDialog dlg(this, __schedule->GetOutputManager(), vmatrices);

    if (dlg.ShowModal() == wxID_OK)
    {
        // it is updated directly
        __schedule->SetDirty();
    }

    _suspendOTL = false;
    if (ol)
    {
        __schedule->SetOutputToLights(this, true, true);
    }

    UpdateUI();
    ValidateWindow();
}

void xScheduleFrame::SendStatus()
{
    if (_webServer != nullptr && __schedule != nullptr)
    {
        std::string result;
        if (__schedule->IsXyzzy())
        {
            __schedule->DoXyzzy("q", "", result, "");
        }
        else
        {
            std::string msg;
            __schedule->Query("GetPlayingStatus", "", result, msg, "", "");
        }
        _webServer->SendMessageToAllWebSockets(result);
    }
}

void xScheduleFrame::DoXyzzy(wxCommandEvent& event)
{
    if (__schedule->IsXyzzy())
    {
        SendStatus();
    }
}

void xScheduleFrame::DoXyzzyEvent(wxCommandEvent& event)
{
    if (_webServer != nullptr && __schedule->IsXyzzy())
    {
        std::string result = "{\"xyzzyevent\":\"" + event.GetString().ToStdString() + "\"}";
        _webServer->SendMessageToAllWebSockets(result);
    }
}

void xScheduleFrame::OnMenuItem_EditFPPRemotesSelected(wxCommandEvent& event)
{
    FPPRemotesDialog dlg(this, __schedule->GetOptions()->GetFPPRemotes());

    dlg.ShowModal();

    __schedule->GetOptions()->SetFPPRemotes(dlg.GetRemotes());

    if (__schedule->GetMode() == SYNCMODE::FPPMASTER) {
        __schedule->OpenFPPSyncSendSocket();
    }

    AddIPs();
}

void xScheduleFrame::OnMenuItem_FPPUnicastRemoteSelected(wxCommandEvent& event)
{
    __schedule->SetMode(SYNCMODE::FPPUNICASTSLAVE);
    UpdateUI();
}

void xScheduleFrame::OnMenuItem_ConfigureOSCSelected(wxCommandEvent& event)
{
    if (__schedule->GetOptions()->GetOSCOptions() != nullptr)
    {
        ConfigureOSC dlg(this, __schedule->GetOptions()->GetOSCOptions());
        if (dlg.ShowModal() == wxID_OK)
        {
            auto m = __schedule->GetMode();
            __schedule->SetMode(SYNCMODE::STANDALONE);
            __schedule->SetMode(m);
        }
    }

    AddIPs();
}

void xScheduleFrame::OnMenuItem_FPPOSCMasterSelected(wxCommandEvent& event)
{
    __schedule->SetMode(SYNCMODE::FPPOSCMASTER);
    UpdateUI();
}

void xScheduleFrame::OnMenuItem_OSCMasterSelected(wxCommandEvent& event)
{
    __schedule->SetMode(SYNCMODE::OSCMASTER);
    UpdateUI();
}

void xScheduleFrame::OnMenuItem_OSCRemoteSelected(wxCommandEvent& event)
{
    __schedule->SetMode(SYNCMODE::OSCSLAVE);
    UpdateUI();
}

void xScheduleFrame::OnListView_PingItemActivated(wxListEvent& event)
{
    std::string ip = event.GetItem().GetText().SubString(0, event.GetItem().GetText().Find(' ')-1).ToStdString();
    if (IsIPValidOrHostname(ip))
    {
        ::wxLaunchDefaultBrowser("http://" + ip);
    }
}

void xScheduleFrame::OnListView_PingItemRClick(wxListEvent& event)
{
}

void xScheduleFrame::OnMenuItem_EditEventsSelected(wxCommandEvent& event)
{
    EventsDialog dlg(this, __schedule->GetOutputManager(), __schedule->GetOptions());

    dlg.ShowModal();

    __schedule->StartListeners();
}

void xScheduleFrame::OnMenuItem_ARTNetTimeCodeSlaveSelected(wxCommandEvent& event)
{
    __schedule->SetMode(SYNCMODE::ARTNETSLAVE);
    UpdateUI();
}

void xScheduleFrame::OnMenuItem_ARTNetTimeCodeMasterSelected(wxCommandEvent& event)
{
    __schedule->SetMode(SYNCMODE::ARTNETMASTER);
    UpdateUI();
}

void xScheduleFrame::OnMenuItem_CrashSelected(wxCommandEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.crit("^^^^^ xSchedule crashing on purpose ... bye bye cruel world.");
    int *p = nullptr;
    *p = 0xFFFFFFFF;
}

void xScheduleFrame::OnMenuItem_MIDITimeCodeMasterSelected(wxCommandEvent& event)
{
    __schedule->SetMode(SYNCMODE::MIDIMASTER);
    UpdateUI();
}

void xScheduleFrame::OnMenuItem_MIDITimeCodeSlaveSelected(wxCommandEvent& event)
{
    __schedule->SetMode(SYNCMODE::MIDISLAVE);
    UpdateUI();
}

void xScheduleFrame::OnMenuItem5MenuItem_ConfigureMIDITimecodeSelected(wxCommandEvent& event)
{
    ConfigureMIDITimecodeDialog dlg(this, __schedule->GetOptions()->GetMIDITimecodeDevice(), __schedule->GetOptions()->GetMIDITimecodeFormat());
    if (dlg.ShowModal() == wxID_OK)
    {
        if (dlg.GetMIDI() != __schedule->GetOptions()->GetMIDITimecodeDevice() ||
            dlg.GetFormat() != __schedule->GetOptions()->GetMIDITimecodeFormat())
        {
            __schedule->GetOptions()->SetMIDITimecodeDevice(dlg.GetMIDI());
            __schedule->GetOptions()->SetMIDITimecodeFormat(dlg.GetFormat());

            auto m = __schedule->GetMode();
            __schedule->SetMode(SYNCMODE::STANDALONE);
            __schedule->SetMode(m);
        }
    }
}