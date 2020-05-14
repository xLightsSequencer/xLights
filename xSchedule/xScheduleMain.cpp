/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#define ZERO 0

// When defined web status is sent every frame ... great for finding issues with connectivity to web clients.
// Must be commented out in release builds
// #define WEBOVERLOAD

//(*InternalHeaders(xScheduleFrame)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/msgdlg.h>
#include <wx/config.h>
#include <wx/file.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/mimetype.h>
#include <wx/bitmap.h>
#include <wx/protocol/http.h>
#include <wx/debugrpt.h>
#include <wx/numdlg.h>

#include "xScheduleMain.h"
#include "PlayList/PlayList.h"
#include "MyTreeItemData.h"
#include "ScheduleManager.h"
#include "Schedule.h"
#include "ScheduleOptions.h"
#include "OptionsDialog.h"
#include "WebServer.h"
#include "PlayList/PlayListStep.h"
#include "../xLights/xLightsVersion.h"
#include "../xLights/outputs/OutputManager.h"
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
#include "ConfigureTest.h"
#include "Pinger.h"
#include "EventsDialog.h"
#include "../xLights/outputs/IPOutput.h"
#include "PlayList/PlayListItemOSC.h"
#include "../xLights/UtilFunctions.h"
#include "ConfigureMIDITimecodeDialog.h"
#include "City.h"
#include "events/ListenerManager.h"
#include "ExtraIPsDialog.h"
#include "SyncFPP.h"
#include "../xLights/VideoReader.h"
#include "../xLights/SpecialOptions.h"
#include "../xLights/outputs/Output.h"
#include "RemoteModeConfigDialog.h"

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

#include "../include/slow.xpm"
#include "../include/web_icon.xpm"
#include "../include/no_web_icon.xpm"

#include <log4cpp/Category.hh>

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
const long xScheduleFrame::ID_BUTTON5 = wxNewId();
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
const long xScheduleFrame::ID_STATICTEXT6 = wxNewId();
const long xScheduleFrame::ID_STATICTEXT4 = wxNewId();
const long xScheduleFrame::ID_STATICTEXT5 = wxNewId();
const long xScheduleFrame::ID_STATICTEXT2 = wxNewId();
const long xScheduleFrame::ID_STATICBITMAP2 = wxNewId();
const long xScheduleFrame::ID_STATICBITMAP1 = wxNewId();
const long xScheduleFrame::ID_PANEL4 = wxNewId();
const long xScheduleFrame::ID_MNU_SHOWFOLDER = wxNewId();
const long xScheduleFrame::ID_MNU_USEXLIGHTSFOLDER = wxNewId();
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
const long xScheduleFrame::ID_MNU_RESETWINDOWS = wxNewId();
const long xScheduleFrame::ID_MNU_CRASH = wxNewId();
const long xScheduleFrame::ID_MNU_TEST = wxNewId();
const long xScheduleFrame::ID_MNU_FPP_BROADCASTMASTER = wxNewId();
const long xScheduleFrame::ID_MNU_FPP_MULTICAST = wxNewId();
const long xScheduleFrame::ID_MNU_FPP_UNICASTMASTER = wxNewId();
const long xScheduleFrame::ID_MNU_FPP_UNICASTCSVMASTER = wxNewId();
const long xScheduleFrame::IDM_MNU_ARTNETMASTER = wxNewId();
const long xScheduleFrame::ID_MNU_OSCMASTER = wxNewId();
const long xScheduleFrame::MNU_MIDITIMECODE_MASTER = wxNewId();
const long xScheduleFrame::ID_MNU_MASTER = wxNewId();
const long xScheduleFrame::ID_MNU_MODENORMAL = wxNewId();
const long xScheduleFrame::ID_MNU_FPPREMOTE = wxNewId();
const long xScheduleFrame::ID_MNU_FPPCSVREMOTE = wxNewId();
const long xScheduleFrame::ID_MNU_ARTNETTIMECODESLAVE = wxNewId();
const long xScheduleFrame::ID_MNU_OSCREMOTE = wxNewId();
const long xScheduleFrame::MNU_MIDITIMECODEREMOTE = wxNewId();
const long xScheduleFrame::ID_MNU_SMPTE = wxNewId();
const long xScheduleFrame::ID_MNU_REMOTECONFIGURE = wxNewId();
const long xScheduleFrame::ID_MNU_REMOTE = wxNewId();
const long xScheduleFrame::ID_MNU_EDITFPPREMOTE = wxNewId();
const long xScheduleFrame::ID_MNU_OSCOPTION = wxNewId();
const long xScheduleFrame::MNU_CONFIGUREMIDITIMECODE = wxNewId();
const long xScheduleFrame::ID_MNU_CONFIGURE_TEST = wxNewId();
const long xScheduleFrame::idMenuAbout = wxNewId();
const long xScheduleFrame::ID_STATUSBAR1 = wxNewId();
const long xScheduleFrame::ID_TIMER1 = wxNewId();
const long xScheduleFrame::ID_TIMER2 = wxNewId();
//*)

const long xScheduleFrame::ID_MNU_EDIT_ADDITIONAL_IPS = wxNewId();
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
wxDEFINE_EVENT(EVT_CHANGESHOWFOLDER, wxCommandEvent);
wxDEFINE_EVENT(EVT_SCHEDULECHANGED, wxCommandEvent);
wxDEFINE_EVENT(EVT_SYNC, wxCommandEvent);
wxDEFINE_EVENT(EVT_DOCHECKSCHEDULE, wxCommandEvent);
wxDEFINE_EVENT(EVT_DOACTION, wxCommandEvent);
wxDEFINE_EVENT(EVT_STOP, wxCommandEvent);
wxDEFINE_EVENT(EVT_QUIT, wxCommandEvent);
wxDEFINE_EVENT(EVT_XYZZY, wxCommandEvent);
wxDEFINE_EVENT(EVT_XYZZYEVENT, wxCommandEvent);

BEGIN_EVENT_TABLE(xScheduleFrame,wxFrame)
    //(*EventTable(xScheduleFrame)
    //*)
    EVT_COMMAND(wxID_ANY, EVT_FRAMEMS, xScheduleFrame::RateNotification)
    EVT_COMMAND(wxID_ANY, EVT_STATUSMSG, xScheduleFrame::StatusMsgNotification)
    EVT_COMMAND(wxID_ANY, EVT_RUNACTION, xScheduleFrame::RunAction)
    EVT_COMMAND(wxID_ANY, EVT_CHANGESHOWFOLDER, xScheduleFrame::ChangeShowFolder)
    EVT_COMMAND(wxID_ANY, EVT_SCHEDULECHANGED, xScheduleFrame::ScheduleChange)
    EVT_COMMAND(wxID_ANY, EVT_SYNC, xScheduleFrame::Sync)
    EVT_COMMAND(wxID_ANY, EVT_DOCHECKSCHEDULE, xScheduleFrame::DoCheckSchedule)
    EVT_COMMAND(wxID_ANY, EVT_DOACTION, xScheduleFrame::DoAction)
    EVT_COMMAND(wxID_ANY, EVT_STOP, xScheduleFrame::DoStop)
    EVT_COMMAND(wxID_ANY, EVT_QUIT, xScheduleFrame::OnQuit)
    EVT_COMMAND(wxID_ANY, EVT_XYZZY, xScheduleFrame::DoXyzzy)
    EVT_COMMAND(wxID_ANY, EVT_XYZZYEVENT, xScheduleFrame::DoXyzzyEvent)
    END_EVENT_TABLE()

// Number of MS after a slow event to show the slow icon for
#define SLOW_FOR_MS 1500

xScheduleFrame::xScheduleFrame(wxWindow* parent, const std::string& showdir, const std::string& playlist, wxWindowID id)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    OutputManager::SetInteractive(false);
    _pinger = nullptr;
    __schedule = nullptr;
    _statusSetAt = wxDateTime::Now();
    _webServer = nullptr;
    _timerOutputFrame = false;
    _suspendOTL = false;
    _nowebicon = wxBitmap(no_web_icon_24);
    _webicon = wxBitmap(web_icon_24);
    _slowicon = wxBitmap(slow_32);
    _webIconDisplayed = false;
    _slowDisplayed = false;
    _lastSlow = 0;

    static log4cpp::Category &logger_frame = log4cpp::Category::getInstance(std::string("log_frame"));
    _timer.SetLog((logger_frame.getPriority() == log4cpp::Priority::DEBUG));

    //(*Initialize(xScheduleFrame)
    wxBoxSizer* BoxSizer1;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer5;
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
    Button_Clone = new wxButton(Panel6, ID_BUTTON5, _("Clone"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON5"));
    BoxSizer1->Add(Button_Clone, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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
    FlexGridSizer6 = new wxFlexGridSizer(0, 8, 0, 0);
    FlexGridSizer6->AddGrowableCol(1);
    StaticText_ShowDir = new wxStaticText(Panel4, ID_STATICTEXT1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer6->Add(StaticText_ShowDir, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_IP = new wxStaticText(Panel4, ID_STATICTEXT3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer6->Add(StaticText_IP, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_RemoteWarning = new wxStaticText(Panel4, ID_STATICTEXT6, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
    StaticText_RemoteWarning->SetForegroundColour(wxColour(255,0,0));
    FlexGridSizer6->Add(StaticText_RemoteWarning, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_PacketsPerSec = new wxStaticText(Panel4, ID_STATICTEXT4, _("Packets/Sec: 0          "), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    FlexGridSizer6->Add(StaticText_PacketsPerSec, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText2 = new wxStaticText(Panel4, ID_STATICTEXT5, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
    FlexGridSizer6->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_Time = new wxStaticText(Panel4, ID_STATICTEXT2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer6->Add(StaticText_Time, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    StaticBitmap_Slow = new wxStaticBitmap(Panel4, ID_STATICBITMAP2, wxNullBitmap, wxDefaultPosition, wxSize(24,24), 0, _T("ID_STATICBITMAP2"));
    FlexGridSizer6->Add(StaticBitmap_Slow, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBitmap_WebIcon = new wxStaticBitmap(Panel4, ID_STATICBITMAP1, wxNullBitmap, wxDefaultPosition, wxSize(24,24), 0, _T("ID_STATICBITMAP1"));
    FlexGridSizer6->Add(StaticBitmap_WebIcon, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel4->SetSizer(FlexGridSizer6);
    FlexGridSizer6->Fit(Panel4);
    FlexGridSizer6->SetSizeHints(Panel4);
    FlexGridSizer1->Add(Panel4, 1, wxALL|wxEXPAND, 0);
    SetSizer(FlexGridSizer1);
    MenuBar1 = new wxMenuBar();
    Menu1 = new wxMenu();
    MenuItem_ShowFolder = new wxMenuItem(Menu1, ID_MNU_SHOWFOLDER, _("Select Show &Folder"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItem_ShowFolder);
    MenuItem_UsexLightsFolder = new wxMenuItem(Menu1, ID_MNU_USEXLIGHTSFOLDER, _("Switch to xLights Folder"), _("Switch to xLights Show Folder"), wxITEM_NORMAL);
    Menu1->Append(MenuItem_UsexLightsFolder);
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
    ToolsMenu = new wxMenu();
    MenuItem_ViewLog = new wxMenuItem(ToolsMenu, ID_MNU_VIEW_LOG, _("&View Log"), wxEmptyString, wxITEM_NORMAL);
    ToolsMenu->Append(MenuItem_ViewLog);
    MenuItem_CheckSchedule = new wxMenuItem(ToolsMenu, ID_MNU_CHECK_SCHEDULE, _("&Check Schedule"), wxEmptyString, wxITEM_NORMAL);
    ToolsMenu->Append(MenuItem_CheckSchedule);
    MenuItem_WebInterface = new wxMenuItem(ToolsMenu, ID_MNU_WEBINTERFACE, _("&Web Interface"), wxEmptyString, wxITEM_NORMAL);
    ToolsMenu->Append(MenuItem_WebInterface);
    MenuItem_ImportxLights = new wxMenuItem(ToolsMenu, ID_MNU_IMPORT, _("Import xLights Playlist"), wxEmptyString, wxITEM_NORMAL);
    ToolsMenu->Append(MenuItem_ImportxLights);
    MenuItem_ResetWindowLocations = new wxMenuItem(ToolsMenu, ID_MNU_RESETWINDOWS, _("Reset Window Locations"), wxEmptyString, wxITEM_NORMAL);
    ToolsMenu->Append(MenuItem_ResetWindowLocations);
    MenuItem_Crash = new wxMenuItem(ToolsMenu, ID_MNU_CRASH, _("Crash"), _("Crash xSchedule"), wxITEM_NORMAL);
    ToolsMenu->Append(MenuItem_Crash);
    MenuBar1->Append(ToolsMenu, _("&Tools"));
    Menu4 = new wxMenu();
    MenuItem_ModeTest = new wxMenuItem(Menu4, ID_MNU_TEST, _("Test"), wxEmptyString, wxITEM_CHECK);
    Menu4->Append(MenuItem_ModeTest);
    MenuItem3 = new wxMenu();
    MenuItem_ModeFPPBroadcastMaster = new wxMenuItem(MenuItem3, ID_MNU_FPP_BROADCASTMASTER, _("FPP Broadcast"), wxEmptyString, wxITEM_CHECK);
    MenuItem3->Append(MenuItem_ModeFPPBroadcastMaster);
    MenuItem_ModeFPPMulticastMaster = new wxMenuItem(MenuItem3, ID_MNU_FPP_MULTICAST, _("FPP Multicast"), wxEmptyString, wxITEM_CHECK);
    MenuItem3->Append(MenuItem_ModeFPPMulticastMaster);
    MenuItem_ModeFPPUnicastMaster = new wxMenuItem(MenuItem3, ID_MNU_FPP_UNICASTMASTER, _("FPP Unicast"), wxEmptyString, wxITEM_CHECK);
    MenuItem3->Append(MenuItem_ModeFPPUnicastMaster);
    MenuItem_ModeFPPUnicastCSVMaster = new wxMenuItem(MenuItem3, ID_MNU_FPP_UNICASTCSVMASTER, _("FPP Unicast CSV"), wxEmptyString, wxITEM_CHECK);
    MenuItem3->Append(MenuItem_ModeFPPUnicastCSVMaster);
    MenuItem_ModeArtNetMaster = new wxMenuItem(MenuItem3, IDM_MNU_ARTNETMASTER, _("ARTNet Timecode"), wxEmptyString, wxITEM_CHECK);
    MenuItem3->Append(MenuItem_ModeArtNetMaster);
    MenuItem_ModeOSCMaster = new wxMenuItem(MenuItem3, ID_MNU_OSCMASTER, _("OSC"), wxEmptyString, wxITEM_CHECK);
    MenuItem3->Append(MenuItem_ModeOSCMaster);
    MenuItem_ModeMIDIMaster = new wxMenuItem(MenuItem3, MNU_MIDITIMECODE_MASTER, _("MIDI Timecode"), wxEmptyString, wxITEM_CHECK);
    MenuItem3->Append(MenuItem_ModeMIDIMaster);
    Menu4->Append(ID_MNU_MASTER, _("Master"), MenuItem3, wxEmptyString);
    Menu6 = new wxMenu();
    MenuItem_ModeRemoteDisabled = new wxMenuItem(Menu6, ID_MNU_MODENORMAL, _("Disabled"), wxEmptyString, wxITEM_RADIO);
    Menu6->Append(MenuItem_ModeRemoteDisabled);
    MenuItem_ModeFPPRemote = new wxMenuItem(Menu6, ID_MNU_FPPREMOTE, _("FPP"), wxEmptyString, wxITEM_RADIO);
    Menu6->Append(MenuItem_ModeFPPRemote);
    MenuItem_ModeFPPCSVRemote = new wxMenuItem(Menu6, ID_MNU_FPPCSVREMOTE, _("FPP CSV"), wxEmptyString, wxITEM_RADIO);
    Menu6->Append(MenuItem_ModeFPPCSVRemote);
    MenuItem_ModeArtNetSlave = new wxMenuItem(Menu6, ID_MNU_ARTNETTIMECODESLAVE, _("ARTNet Timecode"), wxEmptyString, wxITEM_RADIO);
    Menu6->Append(MenuItem_ModeArtNetSlave);
    MenuItem_ModeOSCRemote = new wxMenuItem(Menu6, ID_MNU_OSCREMOTE, _("OSC"), wxEmptyString, wxITEM_RADIO);
    Menu6->Append(MenuItem_ModeOSCRemote);
    MenuItem_ModeMIDISlave = new wxMenuItem(Menu6, MNU_MIDITIMECODEREMOTE, _("MIDI Timecode"), wxEmptyString, wxITEM_RADIO);
    Menu6->Append(MenuItem_ModeMIDISlave);
    MenuItem_SMPTE = new wxMenuItem(Menu6, ID_MNU_SMPTE, _("SMPTE LTC"), wxEmptyString, wxITEM_RADIO);
    Menu6->Append(MenuItem_SMPTE);
    MenuItem_RemoteLatency = new wxMenuItem(Menu6, ID_MNU_REMOTECONFIGURE, _("Configure"), wxEmptyString, wxITEM_NORMAL);
    Menu6->Append(MenuItem_RemoteLatency);
    Menu4->Append(ID_MNU_REMOTE, _("Remote"), Menu6, wxEmptyString);
    MenuItem_EditFPPRemotes = new wxMenuItem(Menu4, ID_MNU_EDITFPPREMOTE, _("Configure FPP Remotes"), _("Edit remotes to unicast sync packets to"), wxITEM_NORMAL);
    Menu4->Append(MenuItem_EditFPPRemotes);
    MenuItem_ConfigureOSC = new wxMenuItem(Menu4, ID_MNU_OSCOPTION, _("Configure OSC"), wxEmptyString, wxITEM_NORMAL);
    Menu4->Append(MenuItem_ConfigureOSC);
    MenuItem5MenuItem_ConfigureMIDITimecode = new wxMenuItem(Menu4, MNU_CONFIGUREMIDITIMECODE, _("Configure MIDI Timecode"), wxEmptyString, wxITEM_NORMAL);
    Menu4->Append(MenuItem5MenuItem_ConfigureMIDITimecode);
    MenuItem_ConfigureTest = new wxMenuItem(Menu4, ID_MNU_CONFIGURE_TEST, _("Configure Test"), wxEmptyString, wxITEM_NORMAL);
    Menu4->Append(MenuItem_ConfigureTest);
    MenuBar1->Append(Menu4, _("&Modes"));
    Menu_Plugins = new wxMenu();
    MenuBar1->Append(Menu_Plugins, _("Plugins"));
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
    Connect(ID_BUTTON5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xScheduleFrame::OnButton_CloneClick);
    Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xScheduleFrame::OnButton_DeleteClick);
    Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xScheduleFrame::OnButton_ScheduleClick);
    Connect(ID_LISTVIEW2,wxEVT_COMMAND_LIST_ITEM_ACTIVATED,(wxObjectEventFunction)&xScheduleFrame::OnListView_PingItemActivated);
    Connect(ID_LISTVIEW2,wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK,(wxObjectEventFunction)&xScheduleFrame::OnListView_PingItemRClick);
    Connect(ID_LISTVIEW1,wxEVT_COMMAND_LIST_ITEM_ACTIVATED,(wxObjectEventFunction)&xScheduleFrame::OnListView_RunningItemActivated);
    Connect(ID_MNU_SHOWFOLDER,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_ShowFolderSelected);
    Connect(ID_MNU_USEXLIGHTSFOLDER,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_UsexLightsFolderSelected);
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
    Connect(ID_MNU_RESETWINDOWS,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_ResetWindowLocationsSelected);
    Connect(ID_MNU_CRASH,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_CrashSelected);
    Connect(ID_MNU_TEST,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_TestSelected);
    Connect(ID_MNU_FPP_BROADCASTMASTER,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_FPPMasterSelected);
    Connect(ID_MNU_FPP_MULTICAST,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_ModeFPPMulticastMasterSelected);
    Connect(ID_MNU_FPP_UNICASTMASTER,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_ModeFPPUnicastMasterSelected);
    Connect(ID_MNU_FPP_UNICASTCSVMASTER,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_ModeFPPUnicastCSVMasterSelected);
    Connect(IDM_MNU_ARTNETMASTER,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_ARTNetTimeCodeMasterSelected);
    Connect(ID_MNU_OSCMASTER,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_OSCMasterSelected);
    Connect(MNU_MIDITIMECODE_MASTER,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_MIDITimeCodeMasterSelected);
    Connect(ID_MNU_MODENORMAL,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_StandaloneSelected);
    Connect(ID_MNU_FPPREMOTE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_FPPRemoteSelected);
    Connect(ID_MNU_FPPCSVREMOTE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_ModeFPPCSVRemoteSelected);
    Connect(ID_MNU_ARTNETTIMECODESLAVE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_ARTNetTimeCodeSlaveSelected);
    Connect(ID_MNU_OSCREMOTE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_OSCRemoteSelected);
    Connect(MNU_MIDITIMECODEREMOTE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_MIDITimeCodeSlaveSelected);
    Connect(ID_MNU_SMPTE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_SMPTESelected);
    Connect(ID_MNU_REMOTECONFIGURE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_RemoteLatencySelected);
    Connect(ID_MNU_EDITFPPREMOTE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_EditFPPRemotesSelected);
    Connect(ID_MNU_OSCOPTION,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_ConfigureOSCSelected);
    Connect(MNU_CONFIGUREMIDITIMECODE,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem5MenuItem_ConfigureMIDITimecodeSelected);
    Connect(ID_MNU_CONFIGURE_TEST,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnMenuItem_ConfigureTestSelected);
    Connect(idMenuAbout,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&xScheduleFrame::OnAbout);
    Connect(ID_TIMER1,wxEVT_TIMER,(wxObjectEventFunction)&xScheduleFrame::On_timerTrigger);
    Connect(ID_TIMER2,wxEVT_TIMER,(wxObjectEventFunction)&xScheduleFrame::On_timerScheduleTrigger);
    Connect(wxID_ANY,wxEVT_CLOSE_WINDOW,(wxObjectEventFunction)&xScheduleFrame::OnClose);
    Connect(wxEVT_SIZE,(wxObjectEventFunction)&xScheduleFrame::OnResize);
    //*)

    Connect(ID_LISTVIEW1, wxEVT_COMMAND_LIST_ITEM_SELECTED, (wxObjectEventFunction)&xScheduleFrame::OnListView_RunningItemSelected);

    Connect(wxID_ANY, EVT_FRAMEMS, (wxObjectEventFunction)&xScheduleFrame::RateNotification);
    Connect(wxID_ANY, EVT_STATUSMSG, (wxObjectEventFunction)&xScheduleFrame::StatusMsgNotification);
    Connect(wxID_ANY, EVT_RUNACTION, (wxObjectEventFunction)&xScheduleFrame::RunAction);
    Connect(wxID_ANY, EVT_CHANGESHOWFOLDER, (wxObjectEventFunction)&xScheduleFrame::ChangeShowFolder);
    Connect(wxID_ANY, EVT_SCHEDULECHANGED, (wxObjectEventFunction)&xScheduleFrame::ScheduleChange);
    Connect(wxID_ANY, EVT_SYNC, (wxObjectEventFunction)&xScheduleFrame::Sync);
    Connect(wxID_ANY, EVT_DOCHECKSCHEDULE, (wxObjectEventFunction)&xScheduleFrame::DoCheckSchedule);
    Connect(wxID_ANY, EVT_DOACTION, (wxObjectEventFunction)&xScheduleFrame::DoAction);
    Connect(wxID_ANY, EVT_STOP, (wxObjectEventFunction)&xScheduleFrame::DoStop);
    Connect(wxID_ANY, EVT_QUIT, (wxObjectEventFunction)&xScheduleFrame::OnQuit);
    Connect(wxID_ANY, EVT_XYZZY, (wxObjectEventFunction)&xScheduleFrame::DoXyzzy);
    Connect(wxID_ANY, EVT_XYZZYEVENT, (wxObjectEventFunction)&xScheduleFrame::DoXyzzyEvent);
    Connect(wxID_ANY, wxEVT_CHAR_HOOK, (wxObjectEventFunction)&xScheduleFrame::OnKeyDown);

    wxString userEmail;
    wxConfig* xlconfig = new wxConfig(_("xLights"));
    if (xlconfig != nullptr)
    {
        xlconfig->Read("xLightsUserEmail", &userEmail, "");
        if (userEmail != "noone@nowhere.xlights.org" && userEmail != "") logger_base.debug("User email address: <email>%s</email>", (const char*)userEmail.c_str());
        delete xlconfig;
        xlconfig = nullptr;
    }

    _timer.SetName("xSchedule frame timer");
    _timerSchedule.SetName("xSchedule schedule timer");

    SetTitle("xLights Scheduler " + GetDisplayVersionString());

    StaticText_RemoteWarning->SetFont(wxFont(14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                                             wxFONTWEIGHT_BOLD, false, wxEmptyString, wxFONTENCODING_DEFAULT));

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
    EnsureWindowHeaderIsOnScreen(this);

    logger_base.debug("xSchedule UI %d,%d %dx%d.", x, y, w, h);

    ListView_Running->AppendColumn("Step");
    ListView_Running->AppendColumn("Offset", wxLIST_FORMAT_RIGHT);
    ListView_Running->AppendColumn("Duration", wxLIST_FORMAT_RIGHT);
    ListView_Running->AppendColumn("");

    ListView_Ping->AppendColumn("Controller");
    ListView_Ping->AppendColumn("Failures", wxLIST_FORMAT_RIGHT);
    ListView_Ping->SetColumnWidth(1, wxLIST_AUTOSIZE_USEHEADER);
    if (ListView_Ping->GetColumnWidth(1) > 80)
        ListView_Ping->SetColumnWidth(1, 80);

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

    logger_base.debug("Loading show folder.");
    if (showdir == "")
    {
        LoadShowDir();
    }
    else
    {
        _showDir = showdir;
    }

    while (!wxDir::Exists(_showDir))
    {
        SelectShowFolder();
    }

    logger_base.debug("Loading schedule.");
    LoadSchedule();

    if (__schedule == nullptr)
    {
        logger_base.error("Error loading schedule. Closing xSchedule.");
        Close();
    }

    VideoReader::InitHWAcceleration();

    __schedule->GetListenerManager()->ProcessPacket("State", "Startup");

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

    if (rate == 0) rate = 50;
    _timer.Start(rate / 2, false, "FrameTimer");
    _timerSchedule.Start(500, false, "ScheduleTimer");

    StaticText_IP->SetLabel("    " + __schedule->GetOurIP() + "   ");

    StaticBitmap_WebIcon->SetBitmap(_nowebicon);
    StaticBitmap_Slow->SetBitmap(_nowebicon);
    _webIconDisplayed = false;
    _slowDisplayed = false;

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

#ifdef __WXOSX__
    Menu6->Remove(MenuItem_SMPTE);
#endif

    RemoteWarning();

    UpdateUI(true);

    _pluginManager.Initialise(_showDir);

    for (auto it : _pluginManager.GetPlugins())
    {
        wxMenuItem* mi = Menu_Plugins->Append(_pluginManager.GetId(it), _pluginManager.GetMenuLabel(it), nullptr);
        mi->SetCheckable(true);
        Connect(_pluginManager.GetId(it), wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)& xScheduleFrame::OnPluginMenu);
        if (config->ReadBool(_("Plugin") + it, false))
        {
            if (_pluginManager.StartPlugin(it, _showDir, GetOurURL()))
            {
                mi->Check(true);
            }
        }
    }
}

void xScheduleFrame::LoadSchedule()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxASSERT(wxThread::IsMain());

    // reset our special options
    SpecialOptions::StashShowDir(_showDir);
    SpecialOptions::GetOption("", "");

    if (_f != nullptr)
    {
        wxLog::SetActiveTarget(nullptr);
        fclose(_f);
		_f = nullptr;
        wxLog::SetLogLevel(wxLogLevelValues::wxLOG_Error);
    }

    if (SpecialOptions::GetOption("wxLogging", "false") == "true")
    {
        _f = fopen("log.txt", "w");
        wxLog::SetLogLevel(wxLogLevelValues::wxLOG_Debug);
        wxLog::SetActiveTarget(new wxLogStderr(_f));
    }
	else
	{
        wxLog::SetActiveTarget(new wxLogStderr());
	}

    logger_base.debug("Loading schedule.");

    if (_pinger != nullptr)
    {
        __schedule->SetPinger(nullptr);
        delete _pinger;
        _pinger = nullptr;
    }
    ListView_Ping->ClearAll();
    ListView_Ping->AppendColumn("Controller");
    ListView_Ping->AppendColumn("Failures", wxLIST_FORMAT_RIGHT);
    ListView_Ping->SetColumnWidth(1, wxLIST_AUTOSIZE_USEHEADER);
    if (ListView_Ping->GetColumnWidth(1) > 80)
        ListView_Ping->SetColumnWidth(1, 80);

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
            StaticText_ShowDir->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                wxFONTWEIGHT_NORMAL, false, wxEmptyString, wxFONTENCODING_DEFAULT));
            MenuItem_UsexLightsFolder->Enable(false);
        }
        else
        {
            StaticText_ShowDir->SetForegroundColour(wxColour(255,128,0));
            StaticText_ShowDir->SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                wxFONTWEIGHT_BOLD, false, wxEmptyString, wxFONTENCODING_DEFAULT));
            MenuItem_UsexLightsFolder->Enable(true);
        }
    }
    else
    {
        StaticText_ShowDir->SetLabel(_showDir + " : Missing xlights_networks.xml");
        StaticText_ShowDir->SetForegroundColour(*wxRED);
        StaticText_ShowDir->SetFont(wxFont(14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
            wxFONTWEIGHT_BOLD, false, wxEmptyString, wxFONTENCODING_DEFAULT));
        MenuItem_UsexLightsFolder->Enable(true);
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
    if (_pinger != nullptr)
    {
        //_pinger->RemoveNonOutputIPs();

        auto fppremotes = __schedule->GetOptions()->GetFPPRemotes();
        for (const auto& it : fppremotes)
        {
            _pinger->AddIP(it, "FPPRemote");
        }

        if (__schedule->GetOptions()->GetOSCOptions() != nullptr)
        {
            _pinger->AddIP(__schedule->GetOptions()->GetOSCOptions()->GetIPAddress(), "OSCTarget");
        }

        auto plis = __schedule->GetPlayListIps();
        for (const auto& it : plis)
        {
            if (it->GetTitle() == "OSC")
            {
                PlayListItemOSC* osc = (PlayListItemOSC*)it;
                _pinger->AddIP(osc->GetIP(), "OSC Play List Item");
            }
        }

        auto extras = __schedule->GetOptions()->GetExtraIPs();
        for (auto it : *extras)
        {
            _pinger->AddIP(it->GetIP(), it->GetDescription());
        }
    }
}

xScheduleFrame::~xScheduleFrame()
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("xScheduleFrame destructor start.");

    // stop the timers immediately
    _timer.Stop();
    _timerSchedule.Stop();

    // give them plenty of time to stop
    wxMilliSleep(100);

    _pluginManager.StopPlugins();
    _pluginManager.Uninitialise();

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
    if (config != nullptr)
    {
        config->Write(_("xsWindowPosX"), x);
        config->Write(_("xsWindowPosY"), y);
        config->Write(_("xsWindowPosW"), w);
        config->Write(_("xsWindowPosH"), h);
        config->Flush();
    }

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

    if (_f != nullptr)
    {
        fclose(_f);
    }

    logger_base.debug("xScheduleFrame destructor end.");
}

void xScheduleFrame::OnQuit(wxCommandEvent& event)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("OnQuit called.");

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

    //__schedule->GetListenerManager()->ProcessPacket("State", "Shutdown");

    Close();
}

void xScheduleFrame::OnAbout(wxCommandEvent& event)
{
    auto about = wxString::Format(wxT("xSchedule v%s, the xLights scheduler."), GetDisplayVersionString());
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
    UpdateUI(true);
}

void xScheduleFrame::DeleteSelectedItem()
{
    wxTreeItemId treeitem = TreeCtrl_PlayListsSchedules->GetSelection();
    if (treeitem.IsOk() && (IsPlayList(treeitem) || IsSchedule(treeitem)))
    {
        if (wxMessageBox(wxString::Format("Are you sure you want to delete '%s'?", TreeCtrl_PlayListsSchedules->GetItemText(treeitem)),
            "Are you sure?", wxYES_NO) == wxYES)
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
    UpdateUI(true);
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
}

bool xScheduleFrame::SelectShowFolder()
{
    DirDialog1->SetPath(_showDir);
    if (DirDialog1->ShowModal() == wxID_OK)
    {
        _showDir = DirDialog1->GetPath().ToStdString();
        SaveShowDir();
        return true;
    }
    return false;
}

void xScheduleFrame::OnMenuItem_ShowFolderSelected(wxCommandEvent& event)
{
    if (SelectShowFolder())
    {
        _pluginManager.Uninitialise();
        _timerSchedule.Stop();
        _timer.Stop();
        LoadSchedule();
        wxASSERT(__schedule != nullptr);
        _timer.Start(50 / 2, false);
        _timerSchedule.Start(500, false);
        _pluginManager.Initialise(_showDir);
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
    UpdateUI(true);
}

void xScheduleFrame::On_timerTrigger(wxTimerEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    static log4cpp::Category &logger_frame = log4cpp::Category::getInstance(std::string("log_frame"));
    static int last = -1;

    if (__schedule == nullptr) return;

    static long long lastms = wxGetLocalTimeMillis().GetValue() - 25;
    long long now = wxGetLocalTimeMillis().GetValue();
    int elapsed = (int)(now - lastms);

    if (elapsed < _timer.GetInterval() / 2)
    {
        // this is premature ... maybe it is a backed up timer event ... lets skip it
        logger_frame.warn("Timer: Frame event fire interval %dms less than 1/4 frame time %dms", elapsed, _timer.GetInterval() / 2);
        return;
    }

    logger_frame.info("Timer: Start frame %d", elapsed);
    if (elapsed > _timer.GetInterval() * 4)
    {
        if (lastms != 0 && __schedule->IsOutputToLights())
        {
            logger_base.warn("Frame interval greater than 200%% of what it should have been [%d] %d", _timer.GetInterval() * 2, (int)(now - lastms));
            _lastSlow = wxGetUTCTimeMillis();
        }
    }
    lastms = now;

    wxDateTime frameStart = wxDateTime::UNow();

    int rate = __schedule->Frame(_timerOutputFrame, this);

#ifndef WEBOVERLOAD
    if (last != wxDateTime::Now().GetSecond() && _timerOutputFrame)
#endif
    {
        // This code must be commented out before release!!!
        logger_frame.debug("    Check schedule");
        last = wxDateTime::Now().GetSecond();
        wxCommandEvent event2(EVT_SCHEDULECHANGED);
        wxPostEvent(this, event2);
    }

    CorrectTimer(rate);

    wxDateTime frameEnd = wxDateTime::UNow();
    long ms = (frameEnd - frameStart).GetMilliseconds().ToLong();

    if (ms > _timer.GetInterval())
    {
        // we took too long so next frame has to be an output frame
        _timerOutputFrame = true;
        logger_frame.debug("Timer: Frame took too long %ld > %d so next frame forced to be output", ms, _timer.GetInterval());
    }
    else
    {
        // output only occurs on alternate timer events
        _timerOutputFrame = !_timerOutputFrame;
    }

    logger_frame.info("Timer: Frame time %ld", ms);
}

void xScheduleFrame::UpdateSchedule()
{
    if (__schedule == nullptr) return;

    wxASSERT(wxThread::IsMain());

    static log4cpp::Category &logger_frame = log4cpp::Category::getInstance(std::string("log_frame"));
    wxStopWatch sw;
    logger_frame.debug("Updating the schedule.");

    TreeCtrl_PlayListsSchedules->Freeze();

    int rate = __schedule->CheckSchedule();

    logger_frame.debug("Schedule checked %ldms", sw.Time());

    PlayList* nextpl = nullptr;
    Schedule* nextsch = nullptr;
    __schedule->GetNextScheduledPlayList(&nextpl, &nextsch);

    // highlight the state of all schedule items in the tree
    wxTreeItemIdValue tid;
    auto root = TreeCtrl_PlayListsSchedules->GetRootItem();
    if (root.IsOk())
    {
        for (auto it = TreeCtrl_PlayListsSchedules->GetFirstChild(root, tid); it != nullptr; it = TreeCtrl_PlayListsSchedules->GetNextChild(root, tid))
        {
            wxTreeItemIdValue tid2;
            for (auto it2 = TreeCtrl_PlayListsSchedules->GetFirstChild(it, tid2); it2 != nullptr; it2 = TreeCtrl_PlayListsSchedules->GetNextChild(it, tid2))
            {
                Schedule* schedule = (Schedule*)((MyTreeItemData*)TreeCtrl_PlayListsSchedules->GetItemData(it2))->GetData();

                TreeCtrl_PlayListsSchedules->SetItemText(it2, GetScheduleName(schedule, __schedule->GetRunningSchedules()));

                if (__schedule->IsScheduleActive(schedule))
                {
                    RunningSchedule* rs = __schedule->GetRunningSchedule();
                    if (rs != nullptr && rs->GetPlayList()->IsRunning() && rs->GetSchedule()->GetId() == schedule->GetId())
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
                    TreeCtrl_PlayListsSchedules->SetItemBackgroundColour(it2, wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
                }

                if (nextsch != nullptr)
                {
                    if (schedule->GetId() == nextsch->GetId())
                    {
                        TreeCtrl_PlayListsSchedules->SetItemTextColour(it2, *wxBLUE);
                    }
                    else
                    {
                        TreeCtrl_PlayListsSchedules->SetItemTextColour(it2, wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
                    }
                }
                else
                {
                    TreeCtrl_PlayListsSchedules->SetItemTextColour(it2, wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
                }
            }
        }
    }

    logger_frame.debug("    Tree updated %ldms", sw.Time());

    CorrectTimer(rate);

    // Ensure I am firing on the minute
    if (wxDateTime::Now().GetSecond() != 0)
    {
        int time = (60 - wxDateTime::Now().GetSecond()) * 1000;
        if (time == 0) time = 1;
        _timerSchedule.Start(time, false);
    }
    else if (_timerSchedule.GetInterval() != 60000)
    {
        _timerSchedule.Start(60000, false);
    }

    logger_frame.debug("    Timers sorted %ldms", sw.Time());

    UpdateUI();

    logger_frame.debug("    UI updated %ldms", sw.Time());

    TreeCtrl_PlayListsSchedules->Thaw();
    TreeCtrl_PlayListsSchedules->Refresh();

    logger_frame.debug("    Schedule updated %ldms", sw.Time());
}

void xScheduleFrame::On_timerScheduleTrigger(wxTimerEvent& event)
{
    if (__schedule->IsFPPRemoteOrMaster())
    {
        SyncFPP::Ping(__schedule->IsSlave());
    }
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
        Button_Clone->Enable();
        Button_Delete->Enable();
        Button_Edit->Enable();
    }
    else
    {
        Button_Clone->Enable(false);
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
        if (oldport != __schedule->GetOptions()->GetWebServerPort() || _webServer == nullptr)
        {
            if (_webServer != nullptr)
            {
                delete _webServer;
            }
            _webServer = new WebServer(__schedule->GetOptions()->GetWebServerPort(), __schedule->GetOptions()->GetAPIOnly(),
                __schedule->GetOptions()->GetPassword(), __schedule->GetOptions()->GetPasswordTimeout());
        }
        else
        {
            _webServer->SetAPIOnly(__schedule->GetOptions()->GetAPIOnly());
            _webServer->SetPassword(__schedule->GetOptions()->GetPassword());
            _webServer->SetPasswordTimeout(__schedule->GetOptions()->GetPasswordTimeout());
        }

        Schedule::SetCity(__schedule->GetOptions()->GetCity());
        __schedule->GetOutputManager()->SetParallelTransmission(__schedule->GetOptions()->IsParallelTransmission());
        OutputManager::SetRetryOpen(__schedule->GetOptions()->IsRetryOpen());
        __schedule->GetOutputManager()->SetSyncEnabled(__schedule->GetOptions()->IsSync());

        VideoReader::SetHardwareAcceleratedVideo(__schedule->GetOptions()->IsHardwareAcceleratedVideo());

        __schedule->OptionsChanged();

        CreateButtons();
    }

    AddIPs();

    UpdateUI();
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
    for (auto it : buttons)
    {
        Disconnect(it->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xScheduleFrame::OnButton_UserClick);
        FlexGridSizer4->Detach(it);
        delete it;
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

    for (auto it : bs)
    {
        // only show not hidden buttons
        if (!wxString(it->GetLabel()).StartsWith("HIDE_") && __schedule->GetCommand(it->GetCommand()) != nullptr)
        {
            CreateButton(it->GetLabel(), it->GetColor());
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
        size_t rate = 0;
        wxString msg;
        __schedule->Action(a[0], a[1], "", nullptr, nullptr, nullptr, rate, msg);
        if (msg != "")
        {
            SetTempMessage(msg);
        }
    }
}

void xScheduleFrame::ChangeShowFolder(wxCommandEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    auto newShowFolder = event.GetString();

    if (newShowFolder == "")
    {
        newShowFolder = __schedule->GetShowDir();
    }

    logger_base.debug("Changing show folder to %s.", (const char *)newShowFolder.c_str());
    _showDir = newShowFolder.ToStdString();
    SaveShowDir();
    _timerSchedule.Stop();
    _timer.Stop();
    LoadSchedule();
    _timer.Start(50 / 2, false);
    _timerSchedule.Start(500, false);
    ValidateWindow();
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
    if (playlist == nullptr && __schedule->GetRunningPlayList() != nullptr) playlist = __schedule->GetPlayList(__schedule->GetRunningPlayList()->GetId());

    PlayListStep* step = nullptr;
    if (playlist != nullptr) {
        int selected = ListView_Running->GetFirstSelected();
        int stepid = (int)ListView_Running->GetItemData(selected);
        if (selected >= 0 && stepid >= 0) {
            step = playlist->GetStep(stepid);
        }
    }

    size_t rate = _timer.GetInterval();
    wxString msg = "";
    __schedule->Action(((wxButton*)event.GetEventObject())->GetLabel(), playlist, step, schedule, rate, msg);

    CorrectTimer(rate);

    UpdateSchedule();
    UpdateUI();
}

void xScheduleFrame::SetTempMessage(const std::string& msg)
{
    _statusSetAt = wxDateTime::Now();
    StatusBar1->SetStatusText(msg);
}

// returns the name of the plugin associated with the given web folder or blank if not running
std::string xScheduleFrame::GetWebPluginRequest(const std::string& request)
{
    for (auto it : _pluginManager.GetPlugins())
    {
        if (wxString(_pluginManager.GetVirtualWebFolder(it)).Lower() == request)
        {
            return it;
        }
    }
    return "";
}

void xScheduleFrame::ManipulateBuffer(uint8_t* buffer, size_t bufferSize)
{
    _pluginManager.ManipulateBuffer(buffer, bufferSize);
}

wxString xScheduleFrame::ProcessPluginRequest(const wxString& plugin, const wxString& command, const wxString& parameters, const wxString& data, const wxString& reference)
{
    std::wstring res;
    _pluginManager.HandleWeb(plugin.ToStdString(), command.ToStdString(), parameters, data, reference, res);
    return wxString(res);
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
        wxMessageBox(wxString::Format("Unable to show log file '%s'.", fn.c_str()), _("Error"));
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
        for (auto it : buttons)
        {
            int x, y, w, h;
            it->GetPosition(&x, &y);
            it->GetSize(&w, &h);

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

void xScheduleFrame::OnListView_RunningItemSelected(wxListEvent& event)
{
    UpdateUI(false);
}

void xScheduleFrame::OnListView_RunningItemActivated(wxListEvent& event)
{
    int selected = ListView_Running->GetFirstSelected();

    PlayList* p = __schedule->GetRunningPlayList();

    if (selected >= 0 && p != nullptr && p->GetRunningStep()->GetId() != ListView_Running->GetItemData(selected))
    {
        size_t rate = 0;
        wxString msg;
        __schedule->Action("Jump to specified step in current playlist", PlayListStep::GetStepNameWithId((int)ListView_Running->GetItemData(selected)), "", p, nullptr, nullptr, rate, msg);
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

void xScheduleFrame::UpdateStatus(bool force)
{
    wxASSERT(wxThread::IsMain());

    wxStopWatch sw;
    static log4cpp::Category &logger_frame = log4cpp::Category::getInstance(std::string("log_frame"));
    logger_frame.debug("            Update Status");

    ListView_Running->Freeze();

    if (StatusBar1->GetStatusText() != "" && (wxDateTime::Now() - _statusSetAt).GetMilliseconds() >  5000)
    {
        StatusBar1->SetStatusText("");
    }

    logger_frame.debug("            Status Text %ldms", sw.Time());

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

    logger_frame.debug("            Got selected playlist %ldms", sw.Time());

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
        if (force ||
            p->GetId() != lastid ||
            p->GetChangeCount() != lastcc ||
            (int)p->IsRunning() != lastrunning ||
            p->GetSteps().size() != laststeps)
        {
            lastcc = p->GetChangeCount();
            lastid = p->GetId();
            lastrunning = (int)p->IsRunning();
            laststeps = p->GetSteps().size();

            ListView_Running->DeleteAllItems();

            int i = 0;
            for (const auto& it : p->GetSteps())
            {
                if (!it->GetEveryStep()) {
                    ListView_Running->InsertItem(i, it->GetNameNoTime());
                    ListView_Running->SetItem(i, 1, it->GetStartTime(p));
                    ListView_Running->SetItem(i, 2, FormatTime(it->GetLengthMS()));
                    ListView_Running->SetItemData(i, it->GetId());
                    i++;
                }
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
                if (!currenthighlighted && ListView_Running->GetItemData(i) == step->GetId())
                {
                    currenthighlighted = true;
                    ListView_Running->SetItem(i, 3, step->GetStatus());
                    ListView_Running->SetItemBackgroundColour(i, wxColor(146,244,155));
                }
                else
                {
                    if (next != nullptr && !nexthighlighted && next->GetId() == ListView_Running->GetItemData(i))
                    {
                        nexthighlighted = true;
                        ListView_Running->SetItemBackgroundColour(i, wxColor(244,241,146));
                    }
                    else
                    {
                        ListView_Running->SetItemBackgroundColour(i, wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
                    }
                    ListView_Running->SetItem(i, 3, "");
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
        if (ListView_Running->GetColumnWidth(2) < 80)
            ListView_Running->SetColumnWidth(2, 80);
        ListView_Running->SetColumnWidth(3, wxLIST_AUTOSIZE);
        if (ListView_Running->GetColumnWidth(3) < 250)
            ListView_Running->SetColumnWidth(3, 250);
    }

    ListView_Running->Thaw();
    ListView_Running->Refresh();

    logger_frame.debug("            Updated running listview %ldms", sw.Time());

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
        if (__schedule->IsSlave())
        {
            if (scheduled != 13)
                BitmapButton_IsScheduled->SetBitmap(_falconremote);
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

    logger_frame.debug("            Updated toolbar %ldms", sw.Time());

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

    if (playlist == nullptr && __schedule->GetRunningPlayList() != nullptr) playlist = __schedule->GetPlayList(__schedule->GetRunningPlayList()->GetId());

    PlayListStep* step = nullptr;
    if (playlist != nullptr) {
        int selected = ListView_Running->GetFirstSelected();
        int stepid = (int)ListView_Running->GetItemData(selected);
        if (selected >= 0 && stepid >= 0) {
            step = playlist->GetStep(stepid);
        }
    }

    auto buttons = Panel1->GetChildren();
    for (auto it = buttons.begin(); it != buttons.end(); ++it)
    {
        UserButton* b = __schedule->GetOptions()->GetButton((*it)->GetLabel().ToStdString());

        if (b != nullptr)
        {
            wxString command = b->GetCommand();
            wxString parameters = b->GetParameters();
            Command* c = b->GetCommandObj();
            wxString msg;
            if (c != nullptr && c->IsValid(parameters, playlist, step, schedule, __schedule, msg, __schedule->IsQueuedPlaylistRunning()))
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

    logger_frame.debug("            Updated buttons %ldms", sw.Time());

    Custom_Volume->SetValue(__schedule->GetVolume());

    StaticText_Time->SetLabel(wxDateTime::Now().FormatTime());

    SendStatus();

    logger_frame.debug("            Status Sent %ldms", sw.Time());
}

void xScheduleFrame::OnBitmapButton_OutputToLightsClick(wxCommandEvent& event)
{
    __schedule->ManualOutputToLightsClick(this);
    UpdateUI();
}

void xScheduleFrame::OnBitmapButton_RandomClick(wxCommandEvent& event)
{
    wxString msg = "";
    __schedule->ToggleCurrentPlayListRandom(msg);
    UpdateUI();
}

void xScheduleFrame::OnBitmapButton_PlayingClick(wxCommandEvent& event)
{
    wxString msg = "";
    __schedule->ToggleCurrentPlayListPause(msg);
    UpdateUI();
}

void xScheduleFrame::OnBitmapButton_PLLoopClick(wxCommandEvent& event)
{
    wxString msg = "";
    __schedule->ToggleCurrentPlayListLoop(msg);
    UpdateUI();
}

void xScheduleFrame::OnBitmapButton_StepLoopClick(wxCommandEvent& event)
{
    wxString msg = "";
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
            if (playlist == nullptr && __schedule->GetRunningPlayList() != nullptr) playlist = __schedule->GetPlayList(__schedule->GetRunningPlayList()->GetId());

            PlayListStep* step = nullptr;
            if (playlist != nullptr) {
                int selected = ListView_Running->GetFirstSelected();
                int stepid = (int)ListView_Running->GetItemData(selected);
                if (selected >= 0 && stepid >= 0) {
                    step = playlist->GetStep(stepid);
                }
            }

            size_t rate = _timer.GetInterval();
            wxString msg = "";
            __schedule->Action((*it)->GetLabel(), playlist, step, schedule, rate, msg);

            CorrectTimer(rate);

            UpdateSchedule();
            return true;
        }
    }

    return false;
}

void xScheduleFrame::CorrectTimer(int rate)
{
    static log4cpp::Category &logger_frame = log4cpp::Category::getInstance(std::string("log_frame"));
    if (rate == 0) rate = 50;
    if ((rate - __schedule->GetTimerAdjustment()) / 2 != _timer.GetInterval())
    {
        logger_frame.debug("Timer corrected %d", (rate - __schedule->GetTimerAdjustment()) / 2);

        _timer.Start((rate - __schedule->GetTimerAdjustment()) / 2);
    }
}

std::string xScheduleFrame::GetOurURL() const
{
    return "http://127.0.0.1:" + wxString::Format("%d", __schedule->GetOptions()->GetWebServerPort());
}

void xScheduleFrame::OnPluginMenu(wxCommandEvent& event)
{
    std::string plugin = _pluginManager.GetPluginFromId(event.GetId());
    wxConfigBase* config = wxConfigBase::Get();

    if (((wxMenu*)event.GetEventObject())->IsChecked(event.GetId()))
    {
        if (!_pluginManager.StartPlugin(plugin, _showDir, GetOurURL()))
        {
            ((wxMenu*)event.GetEventObject())->Check(event.GetId(), false);
            config->Write(_("Plugin") + plugin, false);
        }
        else
        {
            config->Write(_("Plugin") + plugin, true);
        }
    }
    else {
        _pluginManager.StopPlugin(plugin);
        config->Write(_("Plugin") + plugin, false);
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

#pragma region Modes

void xScheduleFrame::OnMenuItem_StandaloneSelected(wxCommandEvent& event)
{
    UIToMode();
}

void xScheduleFrame::OnMenuItem_FPPRemoteSelected(wxCommandEvent& event)
{
    UIToMode();
}

void xScheduleFrame::OnMenuItem_OSCRemoteSelected(wxCommandEvent& event)
{
    UIToMode();
}

void xScheduleFrame::OnMenuItem_FPPUnicastRemoteSelected(wxCommandEvent& event)
{
    UIToMode();
}

void xScheduleFrame::OnMenuItem_MIDITimeCodeSlaveSelected(wxCommandEvent& event)
{
    UIToMode();
}

void xScheduleFrame::OnMenuItem_ARTNetTimeCodeSlaveSelected(wxCommandEvent& event)
{
    UIToMode();
}

void xScheduleFrame::OnMenuItem_MIDITimeCodeMasterSelected(wxCommandEvent& event)
{
    UIToMode();
}

void xScheduleFrame::OnMenuItem_ModeFPPUnicastMasterSelected(wxCommandEvent& event)
{
    UIToMode();
}

void xScheduleFrame::OnMenuItem_ModeFPPMulticastMasterSelected(wxCommandEvent& event)
{
    UIToMode();
}

void xScheduleFrame::OnMenuItem_FPPMasterSelected(wxCommandEvent& event)
{
    UIToMode();
}

void xScheduleFrame::OnMenuItem_OSCMasterSelected(wxCommandEvent& event)
{
    UIToMode();
}

void xScheduleFrame::OnMenuItem_ARTNetTimeCodeMasterSelected(wxCommandEvent& event)
{
    UIToMode();
}

void xScheduleFrame::OnMenuItem_TestSelected(wxCommandEvent& event)
{
    static int mode = (int)SYNCMODE::STANDALONE;
    static REMOTEMODE remoteMode = REMOTEMODE::DISABLED;

    if (event.IsChecked())
    {
        __schedule->GetMode(mode, remoteMode);
    }
    else
    {
        __schedule->SetMode(mode, remoteMode);
        __schedule->SetTestMode(false);
        ModeToUI();
    }

    UIToMode();
}

void xScheduleFrame::OnMenuItem_EditFPPRemotesSelected(wxCommandEvent& event)
{
    FPPRemotesDialog dlg(this, __schedule->GetOptions()->GetFPPRemotes());

    dlg.ShowModal();

    __schedule->GetOptions()->SetFPPRemotes(dlg.GetRemotes());

    UIToMode();

    AddIPs();
}

void xScheduleFrame::OnMenuItem_ConfigureOSCSelected(wxCommandEvent& event)
{
    if (__schedule->GetOptions()->GetOSCOptions() != nullptr)
    {
        ConfigureOSC dlg(this, __schedule->GetOptions()->GetOSCOptions());
        if (dlg.ShowModal() == wxID_OK)
        {
            UIToMode();
        }
    }

    AddIPs();
}

void xScheduleFrame::OnMenuItem5MenuItem_ConfigureMIDITimecodeSelected(wxCommandEvent& event)
{
    ConfigureMIDITimecodeDialog dlg(this, __schedule->GetOptions()->GetMIDITimecodeDevice(), __schedule->GetOptions()->GetMIDITimecodeFormat(), __schedule->GetOptions()->GetMIDITimecodeOffset());
    if (dlg.ShowModal() == wxID_OK)
    {
        if (dlg.GetMIDI() != __schedule->GetOptions()->GetMIDITimecodeDevice() ||
            dlg.GetFormat() != __schedule->GetOptions()->GetMIDITimecodeFormat() ||
            dlg.GetOffset() != __schedule->GetOptions()->GetMIDITimecodeOffset()
            )
        {
            __schedule->GetOptions()->SetMIDITimecodeDevice(dlg.GetMIDI());
            __schedule->GetOptions()->SetMIDITimecodeFormat(dlg.GetFormat());
            __schedule->GetOptions()->SetMIDITimecodeOffset(dlg.GetOffset());

            UIToMode();
        }
    }
}

void xScheduleFrame::OnMenuItem_ConfigureTestSelected(wxCommandEvent& event)
{
    if (__schedule->GetOptions()->GetTestOptions() != nullptr)
    {
        ConfigureTest dlg(this, __schedule->GetOptions()->GetTestOptions());
        dlg.ShowModal();
    }
}

void xScheduleFrame::ModeToUI()
{
    int mode;
    REMOTEMODE remoteMode;
    __schedule->GetMode(mode, remoteMode);

    if (__schedule->IsTest())
    {
        MenuItem_ModeTest->Check();

        MenuItem_ModeArtNetMaster->Check(false);
        MenuItem_ModeOSCMaster->Check(false);
        MenuItem_ModeFPPBroadcastMaster->Check(false);
        MenuItem_ModeFPPUnicastMaster->Check(false);
        MenuItem_ModeFPPUnicastCSVMaster->Check(false);
        MenuItem_ModeFPPMulticastMaster->Check(false);
        MenuItem_ModeMIDIMaster->Check(false);

        MenuItem_ModeRemoteDisabled->Check();
        MenuItem_ModeFPPRemote->Check(false);
        MenuItem_ModeFPPCSVRemote->Check(false);
        MenuItem_ModeOSCRemote->Check(false);
        MenuItem_ModeArtNetSlave->Check(false);
        MenuItem_ModeMIDISlave->Check(false);
        MenuItem_SMPTE->Check(false);
    }
    else
    {
        MenuItem_ModeTest->Check(false);

        if (mode & static_cast<int>(SYNCMODE::ARTNETMASTER))
        {
            MenuItem_ModeArtNetMaster->Check();
        }
        else
        {
            MenuItem_ModeArtNetMaster->Check(false);
        }

        if (mode & static_cast<int>(SYNCMODE::FPPBROADCASTMASTER))
        {
            MenuItem_ModeFPPBroadcastMaster->Check();
        }
        else
        {
            MenuItem_ModeFPPBroadcastMaster->Check(false);
        }

        if (mode & static_cast<int>(SYNCMODE::FPPMULTICASTMASTER))
        {
            MenuItem_ModeFPPMulticastMaster->Check();
        }
        else
        {
            MenuItem_ModeFPPMulticastMaster->Check(false);
        }

        if (mode & static_cast<int>(SYNCMODE::FPPUNICASTMASTER))
        {
            MenuItem_ModeFPPUnicastMaster->Check();
        }
        else
        {
            MenuItem_ModeFPPUnicastMaster->Check(false);
        }

        if (mode & static_cast<int>(SYNCMODE::FPPUNICASTCSVMASTER))
        {
            MenuItem_ModeFPPUnicastCSVMaster->Check();
        }
        else
        {
            MenuItem_ModeFPPUnicastCSVMaster->Check(false);
        }

        if (mode & static_cast<int>(SYNCMODE::OSCMASTER))
        {
            MenuItem_ModeOSCMaster->Check();
        }
        else
        {
            MenuItem_ModeOSCMaster->Check(false);
        }

        if (mode & static_cast<int>(SYNCMODE::MIDIMASTER))
        {
            MenuItem_ModeMIDIMaster->Check();
        }
        else
        {
            MenuItem_ModeMIDIMaster->Check(false);
        }

        switch (remoteMode)
        {
        case REMOTEMODE::ARTNETSLAVE:
            MenuItem_ModeArtNetSlave->Check(true);
            if (BitmapButton_IsScheduled->GetToolTipText() != "ArtNET remote.")
                BitmapButton_IsScheduled->SetToolTip("ArtNET remote.");
            break;
        case REMOTEMODE::OSCSLAVE:
            MenuItem_ModeOSCRemote->Check(true);
            if (BitmapButton_IsScheduled->GetToolTipText() != "OSC remote.")
                BitmapButton_IsScheduled->SetToolTip("OSC remote.");
            break;
        case REMOTEMODE::MIDISLAVE:
            MenuItem_ModeMIDISlave->Check(true);
            if (BitmapButton_IsScheduled->GetToolTipText() != "MIDI remote.")
                BitmapButton_IsScheduled->SetToolTip("MIDI remote.");
            break;
        case REMOTEMODE::FPPSLAVE:
            MenuItem_ModeFPPRemote->Check(true);
            if (BitmapButton_IsScheduled->GetToolTipText() != "FPP remote.")
                BitmapButton_IsScheduled->SetToolTip("FPP remote.");
            break;
        case REMOTEMODE::SMPTESLAVE:
            MenuItem_SMPTE->Check(true);
            if (BitmapButton_IsScheduled->GetToolTipText() != "SMPTE LTC remote.")
                BitmapButton_IsScheduled->SetToolTip("SMPTE LTC remote.");
            break;
        case REMOTEMODE::FPPCSVSLAVE:
            MenuItem_ModeFPPCSVRemote->Check(true);
            if (BitmapButton_IsScheduled->GetToolTipText() != "FPP CSV remote.")
                BitmapButton_IsScheduled->SetToolTip("FPP CSV remote.");
            break;
        default:
            MenuItem_ModeRemoteDisabled->Check(true);
            break;
        }
    }

    RemoteWarning();
}

void xScheduleFrame::UIToMode()
{
    __schedule->SetTestMode(MenuItem_ModeTest->IsChecked());

    int mode = (int)SYNCMODE::STANDALONE;
    mode |= MenuItem_ModeArtNetMaster->IsChecked() ? (int)SYNCMODE::ARTNETMASTER : 0;
    mode |= MenuItem_ModeFPPBroadcastMaster->IsChecked() ? (int)SYNCMODE::FPPBROADCASTMASTER : 0;
    mode |= MenuItem_ModeFPPMulticastMaster->IsChecked() ? (int)SYNCMODE::FPPMULTICASTMASTER : 0;
    mode |= MenuItem_ModeFPPUnicastMaster->IsChecked() ? (int)SYNCMODE::FPPUNICASTMASTER : 0;
    mode |= MenuItem_ModeFPPUnicastCSVMaster->IsChecked() ? (int)SYNCMODE::FPPUNICASTCSVMASTER : 0;
    mode |= MenuItem_ModeOSCMaster->IsChecked() ? (int)SYNCMODE::OSCMASTER : 0;
    mode |= MenuItem_ModeMIDIMaster->IsChecked() ? (int)SYNCMODE::MIDIMASTER : 0;

    if (MenuItem_ModeOSCRemote->IsChecked())
    {
        __schedule->SetMode(mode, REMOTEMODE::OSCSLAVE);
    }
    else if (MenuItem_ModeMIDISlave->IsChecked())
    {
        __schedule->SetMode(mode, REMOTEMODE::MIDISLAVE);
    }
    else if (MenuItem_ModeArtNetSlave->IsChecked())
    {
        __schedule->SetMode(mode, REMOTEMODE::ARTNETSLAVE);
    }
    else if (MenuItem_ModeFPPRemote->IsChecked())
    {
        __schedule->SetMode(mode, REMOTEMODE::FPPSLAVE);
    }
    else if (MenuItem_ModeFPPCSVRemote->IsChecked())
    {
        __schedule->SetMode(mode, REMOTEMODE::FPPCSVSLAVE);
    }
    else if (MenuItem_SMPTE->IsChecked())
    {
        __schedule->SetMode(mode, REMOTEMODE::SMPTESLAVE);
    }
    else
    {
        __schedule->SetMode(mode, REMOTEMODE::DISABLED);
    }

    RemoteWarning();
    UpdateUI();
}

void xScheduleFrame::RemoteWarning()
{
    if (__schedule->IsSlave())
    {
        StaticText_RemoteWarning->SetLabel("Remote");
    }
    else if (__schedule->IsTest())
    {
        StaticText_RemoteWarning->SetLabel("Test");
    }
    else
    {
        StaticText_RemoteWarning->SetLabel("");
    }
    FlexGridSizer6->Layout();
}
#pragma endregion Modes


void xScheduleFrame::OnMenuItem_WebInterfaceSelected(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser(wxString::Format("http://localhost:%d", __schedule->GetOptions()->GetWebServerPort()));
}

void xScheduleFrame::OnMenuItem_AddPlayListSelected(wxCommandEvent& event)
{
    AddPlayList();
    UpdateUI(true);
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
    UpdateUI(true);
}

void xScheduleFrame::OnButton_EditClick(wxCommandEvent& event)
{
    EditSelectedItem();
    UpdateUI(true);
}

void xScheduleFrame::OnButton_DeleteClick(wxCommandEvent& event)
{
    DeleteSelectedItem();
    UpdateUI(true);
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
}

// This is called when anything interesting happens in schedule manager
void xScheduleFrame::ScheduleChange(wxCommandEvent& event)
{
    UpdateUI();
}

void xScheduleFrame::Sync(wxCommandEvent& event)
{
    __schedule->DoSync(event.GetString().ToStdString(), event.GetInt());
}

void xScheduleFrame::DoCheckSchedule(wxCommandEvent& event)
{
    UpdateSchedule();
    UpdateUI();
}

void xScheduleFrame::DoStop(wxCommandEvent& event)
{
    bool end = false;
	bool sustain = false;
    if (event.GetString() == "end")
    {
        end = true;
    }
	else if (event.GetString() == "sustain")
	{
		sustain = true;
	}

    if (event.GetInt() == -1)
    {
        __schedule->StopAll(sustain);
    }
    else
    {
        PlayList* p = __schedule->GetRunningPlayList(event.GetInt());
        if (p != nullptr)
        {
            __schedule->StopPlayList(p, end, sustain);
        }
    }
    UpdateSchedule();
    UpdateUI();
}

void xScheduleFrame::DoAction(wxCommandEvent& event)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    ActionMessageData* amd = (ActionMessageData*)event.GetClientData();

    logger_base.debug("    Thread switched command = %s", (const char*)amd->_command.c_str());

    PlayList* playlist = nullptr;
    Schedule* schedule = nullptr;
    PlayListStep* step = nullptr;

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
    if (playlist == nullptr && __schedule->GetRunningPlayList() != nullptr) playlist = __schedule->GetPlayList(__schedule->GetRunningPlayList()->GetId());

    if (playlist != nullptr) {
        int selected = ListView_Running->GetFirstSelected();
        int stepid = (int)ListView_Running->GetItemData(selected);
        if (selected >= 0 && stepid >= 0) {
            step = playlist->GetStep(stepid);
        }
    }

    size_t rate = _timer.GetInterval();
    wxString msg = "";

    __schedule->Action(amd->_command, amd->_parameters, amd->_data, playlist, step, schedule, rate, msg);

    delete amd;
}

void xScheduleFrame::UpdateUI(bool force)
{
    wxASSERT(wxThread::IsMain());

    wxStopWatch sw;
    static log4cpp::Category &logger_frame = log4cpp::Category::getInstance(std::string("log_frame"));
    logger_frame.debug("        Update UI");

    StaticText_PacketsPerSec->SetLabel(wxString::Format("Packets/Sec: %d", __schedule->GetPPS()));

    UpdateStatus(force);

    logger_frame.debug("        Status updated %ldms", sw.Time());

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

    if (wxGetUTCTimeMillis() - _lastSlow < SLOW_FOR_MS)
    {
        if (!_slowDisplayed)
        {
            StaticBitmap_Slow->SetBitmap(_slowicon);
            _slowDisplayed = true;
        }
    }
    else
    {
        if (_slowDisplayed)
        {
            StaticBitmap_Slow->SetBitmap(_nowebicon);
            _slowDisplayed = false;
        }
    }

    logger_frame.debug("        Web request status updated %ldms", sw.Time());

    if (!_suspendOTL)
    {
        if (!__schedule->GetOptions()->IsSendOffWhenNotRunning() && __schedule->GetManualOutputToLights() == -1)
        {
            if (__schedule->GetRunningPlayList() == nullptr && !__schedule->IsXyzzy() && !__schedule->IsTest())
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

    logger_frame.debug("        Managed output to lights %ldms", sw.Time());

    ModeToUI();

    logger_frame.debug("        Updated mode %ldms", sw.Time());

    if (_pinger != nullptr)
    {
        ListView_Ping->Freeze();
        auto pingresults = _pinger->GetPingers();
        for (auto it : pingresults) {
            // find it in the list
            int item = -1;
            for (int i = 0; i < ListView_Ping->GetItemCount(); i++) {
                if (ListView_Ping->GetItemText(i) == it->GetName()) {
                    item = i;
                    break;
                }
            }

            wxASSERT(ListView_Ping->GetColumnCount() == 2);

            // if not there ... add it
            if (item == -1) {
                item = ListView_Ping->GetItemCount();
                item = ListView_Ping->InsertItem(item, it->GetName());
                ListView_Ping->SetItem(item, 1, "");
                ListView_Ping->SetColumnWidth(0, wxLIST_AUTOSIZE);
            }

            // update the colour
            if (item >= 0) {
                if (it->GetFailCount() == 0) {
                    ListView_Ping->SetItem(item, 1, "");
                }
                else {
                    ListView_Ping->SetItem(item, 1, wxString::Format("%d", it->GetFailCount()));
                }

                switch (it->GetPingResult()) {
                case Output::PINGSTATE::PING_OK:
                case Output::PINGSTATE::PING_OPEN:
                case Output::PINGSTATE::PING_OPENED:
                case Output::PINGSTATE::PING_WEBOK:
                    ListView_Ping->SetItemBackgroundColour(item, *wxGREEN);
                    ListView_Ping->SetItemTextColour(item, *wxBLACK);
                    break;
                case Output::PINGSTATE::PING_ALLFAILED:
                    ListView_Ping->SetItemBackgroundColour(item, *wxRED);
                    ListView_Ping->SetItemTextColour(item, *wxWHITE);
                    break;
                case Output::PINGSTATE::PING_UNAVAILABLE:
                    ListView_Ping->SetItemTextColour(item, *wxBLACK);
                    ListView_Ping->SetItemBackgroundColour(item, *wxWHITE);
                    break;
                case Output::PINGSTATE::PING_UNKNOWN:
                    ListView_Ping->SetItemBackgroundColour(item, wxColour(255, 128, 0));
                    ListView_Ping->SetItemTextColour(item, *wxBLACK);
                    break;
                }
            }
        }

        // remove anything in the tree which isnt in the results
        for (int i = 0; i < ListView_Ping->GetItemCount(); i ++)
        {
            bool found = false;
            for (auto it : pingresults)
            {
                if (ListView_Ping->GetItemText(i) == it->GetName())
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
        ListView_Ping->Thaw();
    }

    logger_frame.debug("        Updated ping status %ldms", sw.Time());

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
}

void xScheduleFrame::OnMenuItem_ImportxLightsSelected(wxCommandEvent& event)
{
    if (FileDialog1->ShowModal() == wxID_OK)
    {
        __schedule->ImportxLightsSchedule(FileDialog1->GetPath().ToStdString());
        UpdateTree();
        UpdateUI();
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
}

void xScheduleFrame::SendStatus()
{
    if (__schedule != nullptr)
    {
        wxString msg;
        wxString result;
        __schedule->Query("GetPlayingStatus", "", result, msg, "", "");

        if (_webServer != nullptr)
        {
            if (_webServer->IsSomeoneListening())
            {
                if (__schedule->IsXyzzy())
                {
                    __schedule->DoXyzzy("q", "", result, "");
                }

                _webServer->SendMessageToAllWebSockets(result);
            }
        }

        _pluginManager.NotifyStatus(result.ToStdString());
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
        wxString result = "{\"xyzzyevent\":\"" + event.GetString().ToStdString() + "\"}";
        _webServer->SendMessageToAllWebSockets(result);
    }
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
    wxMenu mnu;
    mnu.Append(ID_MNU_EDIT_ADDITIONAL_IPS, "Edit additional IPs");
    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&xScheduleFrame::OnPingPopup, nullptr, this);
    PopupMenu(&mnu);
    ValidateWindow();
}

void xScheduleFrame::OnPingPopup(wxCommandEvent &event)
{
    if (event.GetId() == ID_MNU_EDIT_ADDITIONAL_IPS)
    {
        ExtraIPsDialog dlg(this, __schedule->GetOptions()->GetExtraIPs());
        dlg.ShowModal();
        AddIPs();
        UpdateUI();
    }
}

void xScheduleFrame::OnMenuItem_EditEventsSelected(wxCommandEvent& event)
{
    EventsDialog dlg(this, __schedule->GetOutputManager(), __schedule->GetOptions());

    dlg.ShowModal();

    __schedule->StartListeners();
}

void xScheduleFrame::OnMenuItem_CrashSelected(wxCommandEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.crit("^^^^^ xSchedule crashing on purpose ... bye bye cruel world.");
    int *p = nullptr;
    *p = 0xFFFFFFFF;
}

void xScheduleFrame::OnMenuItem_UsexLightsFolderSelected(wxCommandEvent& event)
{
    _showDir = ScheduleManager::xLightsShowDir();
    SaveShowDir();
    _timerSchedule.Stop();
    _timer.Stop();
    LoadSchedule();
    _timer.Start(50 / 2, false);
    _timerSchedule.Start(500, false);

    ValidateWindow();
}

void xScheduleFrame::OnMenuItem_RemoteLatencySelected(wxCommandEvent& event)
{
    RemoteModeConfigDialog dlg(this, __schedule->GetOptions()->GetRemoteLatency(), __schedule->GetOptions()->GetRemoteAcceptableJitter());
    if (dlg.ShowModal() == wxID_OK)
    {
        __schedule->GetOptions()->SetRemoteLatency(dlg.GetLatency());
        __schedule->GetOptions()->SetRemoteAcceptableJitter(dlg.GetJitter());
    }
}
void xScheduleFrame::OnButton_CloneClick(wxCommandEvent& event)
{
    wxTreeItemId treeitem = TreeCtrl_PlayListsSchedules->GetSelection();
    if (treeitem.IsOk())
    {
        if (IsPlayList(treeitem))
        {
            PlayList* playlist = (PlayList*)((MyTreeItemData*)TreeCtrl_PlayListsSchedules->GetItemData(treeitem))->GetData();
            PlayList* newpl = new PlayList(*playlist, true);
            wxTreeItemId  newitem = TreeCtrl_PlayListsSchedules->AppendItem(TreeCtrl_PlayListsSchedules->GetRootItem(), newpl->GetName(), -1, -1, new MyTreeItemData(newpl));
            TreeCtrl_PlayListsSchedules->Expand(newitem);
            TreeCtrl_PlayListsSchedules->EnsureVisible(newitem);
            __schedule->AddPlayList(newpl);
        }
        else if (IsSchedule(treeitem))
        {
            wxTreeItemId plid = TreeCtrl_PlayListsSchedules->GetItemParent(treeitem);
            Schedule* schedule = (Schedule*)((MyTreeItemData*)TreeCtrl_PlayListsSchedules->GetItemData(treeitem))->GetData();
            if (plid.IsOk())
            {
                PlayList* playlist = (PlayList*)((MyTreeItemData*)TreeCtrl_PlayListsSchedules->GetItemData(plid))->GetData();
                Schedule* newSchedule = new Schedule(*schedule, true);
                wxTreeItemId  newitem = TreeCtrl_PlayListsSchedules->AppendItem(plid, GetScheduleName(newSchedule, __schedule->GetRunningSchedules()), -1, -1, new MyTreeItemData(newSchedule));
                TreeCtrl_PlayListsSchedules->Expand(plid);
                TreeCtrl_PlayListsSchedules->EnsureVisible(newitem);
                playlist->AddSchedule(newSchedule);
            }
        }
        UpdateSchedule();
        UpdateUI();
    }
}

void xScheduleFrame::OnMenuItem_ModeFPPUnicastCSVMasterSelected(wxCommandEvent& event)
{
    UIToMode();
}

void xScheduleFrame::OnMenuItem_ModeFPPCSVRemoteSelected(wxCommandEvent& event)
{
    UIToMode();
}

void xScheduleFrame::OnClose(wxCloseEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("xSchedule frame close.");
    Destroy();
}

void xScheduleFrame::OnMenuItem_SMPTESelected(wxCommandEvent& event)
{
    UIToMode();
}

void xScheduleFrame::OnMenuItem_ResetWindowLocationsSelected(wxCommandEvent& event)
{
    wxConfigBase* config = wxConfigBase::Get();
    config->DeleteEntry(_("xsPLWindowPosX"));
    config->DeleteEntry(_("xsPLWindowPosY"));
    config->DeleteEntry(_("xsPLWindowPosW"));
    config->DeleteEntry(_("xsPLWindowPosH"));
    config->DeleteEntry(_("xsWindowPosX"));
    config->DeleteEntry(_("xsWindowPosY"));
    config->DeleteEntry(_("xsWindowPosW"));
    config->DeleteEntry(_("xsWindowPosH"));
}
