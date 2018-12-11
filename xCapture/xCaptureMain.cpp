/***************************************************************
 * Name:      xCaptureMain.cpp
 * Purpose:   Code for Application Frame
 * Author:    xLights ()
 * Created:   2016-12-30
 * Copyright: xLights (http://xlights.org)
 * License:
 **************************************************************/

#define ZERO 0
#define E131PORT 5568
#define ARTNETPORT 0x1936

#include "xCaptureMain.h"
#include <wx/msgdlg.h>
#include <wx/config.h>
#include <log4cpp/Category.hh>
#include <wx/file.h>
#include <wx/filename.h>
#include "../xLights/xLightsVersion.h"
#include <wx/debugrpt.h>
#include "../xLights/osxMacUtils.h"
#include <wx/protocol/http.h>
#include "UniverseEntryDialog.h"
#include <wx/filedlg.h>
#include <wx/numdlg.h>
#include "ResultDialog.h"
#include "../xLights/IPEntryDialog.h"

#ifndef __WXMSW__
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include "../include/xLights.xpm"
#include "../include/xLights-16.xpm"
#include "../include/xLights-32.xpm"
#include "../include/xLights-64.xpm"
#include "../include/xLights-128.xpm"

//(*InternalHeaders(xCaptureFrame)
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

//(*IdInit(xCaptureFrame)
const long xCaptureFrame::ID_CHECKBOX_TRIGGERONCHANNEL = wxNewId();
const long xCaptureFrame::ID_STATICTEXT1 = wxNewId();
const long xCaptureFrame::ID_SPINCTRL_UNIVERSE = wxNewId();
const long xCaptureFrame::ID_STATICTEXT2 = wxNewId();
const long xCaptureFrame::ID_SPINCTRL_CHANNEL = wxNewId();
const long xCaptureFrame::ID_STATICTEXT3 = wxNewId();
const long xCaptureFrame::ID_SPINCTRL_START = wxNewId();
const long xCaptureFrame::ID_STATICTEXT4 = wxNewId();
const long xCaptureFrame::ID_SPINCTRL_END = wxNewId();
const long xCaptureFrame::ID_STATICLINE1 = wxNewId();
const long xCaptureFrame::ID_STATICTEXT5 = wxNewId();
const long xCaptureFrame::ID_CHECKBOX_E131 = wxNewId();
const long xCaptureFrame::ID_CHECKBOX_ARTNET = wxNewId();
const long xCaptureFrame::ID_STATICTEXT7 = wxNewId();
const long xCaptureFrame::ID_STATICTEXT8 = wxNewId();
const long xCaptureFrame::ID_BUTTON9 = wxNewId();
const long xCaptureFrame::ID_STATICTEXT6 = wxNewId();
const long xCaptureFrame::ID_STATICLINE2 = wxNewId();
const long xCaptureFrame::ID_LISTVIEW_UNIVERSES = wxNewId();
const long xCaptureFrame::ID_BUTTON6 = wxNewId();
const long xCaptureFrame::ID_BUTTON3 = wxNewId();
const long xCaptureFrame::ID_BUTTON4 = wxNewId();
const long xCaptureFrame::ID_BUTTON5 = wxNewId();
const long xCaptureFrame::ID_STATICTEXT9 = wxNewId();
const long xCaptureFrame::ID_CHOICE1 = wxNewId();
const long xCaptureFrame::ID_SPINCTRL1 = wxNewId();
const long xCaptureFrame::ID_BUTTON1 = wxNewId();
const long xCaptureFrame::ID_BUTTON8 = wxNewId();
const long xCaptureFrame::ID_BUTTON2 = wxNewId();
const long xCaptureFrame::ID_BUTTON7 = wxNewId();
const long xCaptureFrame::ID_STATUSBAR1 = wxNewId();
const long xCaptureFrame::ID_TIMER1 = wxNewId();
//*)

const long xCaptureFrame::ID_E131SOCKET = wxNewId();
const long xCaptureFrame::ID_ARTNETSOCKET = wxNewId();

BEGIN_EVENT_TABLE(xCaptureFrame,wxFrame)
    //(*EventTable(xCaptureFrame)
    //*)
END_EVENT_TABLE()

void xCaptureFrame::PurgeCollectedData()
{
    while (_capturedData.size() > 0)
    {
        auto toDelete = _capturedData.front();
        _capturedData.pop_front();
        delete toDelete;
    }
}

void xCaptureFrame::StashPacket(long type, wxByte* packet, int len)
{
    int universe = -1;
    if (type == ID_E131SOCKET)
    {
        universe = ((int)packet[113] << 8) + (int)packet[114];
    }
    else if (type == ID_ARTNETSOCKET)
    {
        // we only handle artdmx packets
        if (packet[9] != 0x50) return;

        universe = ((int)packet[15] << 8) + (int)packet[14];
    }

    if (universe == -1) return;

    if (CheckBox_TriggerOnChannel->GetValue())
    {
        if (universe == SpinCtrl_Universe->GetValue())
        {
            wxByte c = packet[125 + SpinCtrl_Channel->GetValue()];

            if (c >= SpinCtrl_TriggerStart->GetValue())
            {
                _capturing = true;
                _capturedDesc = "";
                ValidateWindow();
            }
            else
            {
                _capturing = false;
                UpdateCaptureDesc();
                ValidateWindow();
            }
        }
    }

    if (!_capturing) return;

    for (auto it = _capturedData.begin(); it != _capturedData.end(); ++it)
    {
        if ((*it)->_protocol == type && (*it)->_universe == universe)
        {
            _capturedPackets++;
            (*it)->AddPacket(type, packet, len);
            return;
        }
    }

    // Doing thise here means we only need to check the list when it isnt already captured
    if (!IsUniverseToBeCaptured(universe)) return;

    Collector* c = new Collector(type, universe);
    _capturedData.push_back(c);
    c->AddPacket(type, packet, len);
    _capturedPackets++;
}

bool xCaptureFrame::IsUniverseToBeCaptured(int universe, bool ignoreall /*= false*/)
{
    if (ListView_Universes->GetItemCount() == 1 &&
        ListView_Universes->GetItemText(0) == "All" && !ignoreall)
    {
        return true;
    }

    for (int i = 0; i < ListView_Universes->GetItemCount(); i++)
    {
        int start = wxAtoi(ListView_Universes->GetItemText(i));
        int end = wxAtoi(ListView_Universes->GetItemText(i, 1));

        if (universe >= start && universe <= end) return true;
    }

    return false;
}

int xCaptureFrame::GuessFrameMS()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    Collector* c = _capturedData.front();

    bool first = true;
    wxDateTime last;
    double totalgap = 0;
    int count = 0;
    // look at the first 10 intervals
    for (auto it = c->_packets.begin(); count < 10 && it != c->_packets.end(); ++it)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            totalgap += ((*it)->_timeStamp - last).GetValue().ToDouble();
            count++;
        }
        last = (*it)->_timeStamp;
    }
    logger_base.debug("Guessing frame time. Total time %fms. Intervals %d, Average Frame %fms, Estimate %dms",
        totalgap,
        count,
        totalgap / count,
        ((int)((totalgap / count)) / 5) * 5);
    totalgap /= count;
    //totalgap += 2.5;
    return ((int)(totalgap / 5)) * 5;
}

xCaptureFrame::xCaptureFrame(wxWindow* parent, const std::string& showdir, const std::string& playlist, wxWindowID id)
{
    // static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    _e131Socket = nullptr;
    _artNETSocket = nullptr;
    _capturing = false;
    _capturedPackets = 0;
    _capturedDesc = "";

    //(*Initialize(xCaptureFrame)
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer4;
    wxFlexGridSizer* FlexGridSizer5;
    wxFlexGridSizer* FlexGridSizer6;
    wxFlexGridSizer* FlexGridSizer7;

    Create(parent, id, _("xLights Capture"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("id"));
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer1->AddGrowableRow(5);
    FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer3->AddGrowableCol(1);
    CheckBox_TriggerOnChannel = new wxCheckBox(this, ID_CHECKBOX_TRIGGERONCHANNEL, _("Trigger on channel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_TRIGGERONCHANNEL"));
    CheckBox_TriggerOnChannel->SetValue(false);
    FlexGridSizer3->Add(CheckBox_TriggerOnChannel, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer3->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Universe:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer3->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    SpinCtrl_Universe = new wxSpinCtrl(this, ID_SPINCTRL_UNIVERSE, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 64000, 1, _T("ID_SPINCTRL_UNIVERSE"));
    SpinCtrl_Universe->SetValue(_T("1"));
    FlexGridSizer3->Add(SpinCtrl_Universe, 1, wxALL|wxEXPAND, 5);
    StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Channel:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer3->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    SpinCtrl_Channel = new wxSpinCtrl(this, ID_SPINCTRL_CHANNEL, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 512, 1, _T("ID_SPINCTRL_CHANNEL"));
    SpinCtrl_Channel->SetValue(_T("1"));
    FlexGridSizer3->Add(SpinCtrl_Channel, 1, wxALL|wxEXPAND, 5);
    StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Start when greater than or equal to:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer3->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    SpinCtrl_TriggerStart = new wxSpinCtrl(this, ID_SPINCTRL_START, _T("128"), wxDefaultPosition, wxDefaultSize, 0, 0, 255, 128, _T("ID_SPINCTRL_START"));
    SpinCtrl_TriggerStart->SetValue(_T("128"));
    FlexGridSizer3->Add(SpinCtrl_TriggerStart, 1, wxALL|wxEXPAND, 5);
    StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Stop when less than:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    FlexGridSizer3->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    SpinCtrl_TriggerStop = new wxSpinCtrl(this, ID_SPINCTRL_END, _T("128"), wxDefaultPosition, wxDefaultSize, 0, 1, 255, 128, _T("ID_SPINCTRL_END"));
    SpinCtrl_TriggerStop->SetValue(_T("128"));
    FlexGridSizer3->Add(SpinCtrl_TriggerStop, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 5);
    StaticLine1 = new wxStaticLine(this, ID_STATICLINE1, wxDefaultPosition, wxSize(10,-1), wxLI_HORIZONTAL, _T("ID_STATICLINE1"));
    FlexGridSizer1->Add(StaticLine1, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer6 = new wxFlexGridSizer(0, 3, 0, 0);
    StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Protocols:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
    FlexGridSizer6->Add(StaticText5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_E131 = new wxCheckBox(this, ID_CHECKBOX_E131, _("E131"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_E131"));
    CheckBox_E131->SetValue(true);
    FlexGridSizer6->Add(CheckBox_E131, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_ArtNET = new wxCheckBox(this, ID_CHECKBOX_ARTNET, _("ArtNET"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_ARTNET"));
    CheckBox_ArtNET->SetValue(false);
    FlexGridSizer6->Add(CheckBox_ArtNET, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText7 = new wxStaticText(this, ID_STATICTEXT7, _("Interface:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
    FlexGridSizer6->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_IP = new wxStaticText(this, ID_STATICTEXT8, _("UNKNOWN"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
    FlexGridSizer6->Add(StaticText_IP, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button1 = new wxButton(this, ID_BUTTON9, _("Force"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON9"));
    FlexGridSizer6->Add(Button1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer6, 1, wxALL|wxEXPAND, 5);
    StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Note: If you want to record multicast then you must list the universes\nto listen to and must have the right interface specified."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
    FlexGridSizer1->Add(StaticText6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticLine2 = new wxStaticLine(this, ID_STATICLINE2, wxDefaultPosition, wxSize(10,-1), wxLI_HORIZONTAL, _T("ID_STATICLINE2"));
    FlexGridSizer1->Add(StaticLine2, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer4->AddGrowableCol(0);
    FlexGridSizer4->AddGrowableRow(0);
    ListView_Universes = new wxListView(this, ID_LISTVIEW_UNIVERSES, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL|wxLC_NO_SORT_HEADER, wxDefaultValidator, _T("ID_LISTVIEW_UNIVERSES"));
    FlexGridSizer4->Add(ListView_Universes, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer5 = new wxFlexGridSizer(0, 1, 0, 0);
    Button_All = new wxButton(this, ID_BUTTON6, _("All"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON6"));
    FlexGridSizer5->Add(Button_All, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Add = new wxButton(this, ID_BUTTON3, _("Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    FlexGridSizer5->Add(Button_Add, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Edit = new wxButton(this, ID_BUTTON4, _("Edit"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
    FlexGridSizer5->Add(Button_Edit, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Delete = new wxButton(this, ID_BUTTON5, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON5"));
    FlexGridSizer5->Add(Button_Delete, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer4->Add(FlexGridSizer5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer7 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer7->AddGrowableCol(1);
    StaticText8 = new wxStaticText(this, ID_STATICTEXT9, _("Frame timing (ms):"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
    FlexGridSizer7->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Choice_Timing = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
    Choice_Timing->Append(_("25"));
    Choice_Timing->Append(_("30"));
    Choice_Timing->Append(_("33"));
    Choice_Timing->Append(_("50"));
    Choice_Timing->Append(_("100"));
    Choice_Timing->SetSelection( Choice_Timing->Append(_("xCapture Detected (rounded to nearest 5ms)")) );
    Choice_Timing->Append(_("Manual"));
    FlexGridSizer7->Add(Choice_Timing, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SpinCtrl_ManualTime = new wxSpinCtrl(this, ID_SPINCTRL1, _T("50"), wxDefaultPosition, wxDefaultSize, 0, 20, 1000, 50, _T("ID_SPINCTRL1"));
    SpinCtrl_ManualTime->SetValue(_T("50"));
    FlexGridSizer7->Add(SpinCtrl_ManualTime, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer7, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer2 = new wxFlexGridSizer(0, 4, 0, 0);
    Button_StartStop = new wxButton(this, ID_BUTTON1, _("Start Capture"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer2->Add(Button_StartStop, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Analyse = new wxButton(this, ID_BUTTON8, _("Analyse"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON8"));
    FlexGridSizer2->Add(Button_Analyse, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Save = new wxButton(this, ID_BUTTON2, _("Save"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    FlexGridSizer2->Add(Button_Save, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Clear = new wxButton(this, ID_BUTTON7, _("Clear"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON7"));
    FlexGridSizer2->Add(Button_Clear, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizer1);
    StatusBar1 = new wxStatusBar(this, ID_STATUSBAR1, 0, _T("ID_STATUSBAR1"));
    int __wxStatusBarWidths_1[1] = { -10 };
    int __wxStatusBarStyles_1[1] = { wxSB_NORMAL };
    StatusBar1->SetFieldsCount(1,__wxStatusBarWidths_1);
    StatusBar1->SetStatusStyles(1,__wxStatusBarStyles_1);
    SetStatusBar(StatusBar1);
    UITimer.SetOwner(this, ID_TIMER1);
    UITimer.Start(1000, false);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    Connect(ID_CHECKBOX_TRIGGERONCHANNEL,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&xCaptureFrame::OnCheckBox_TriggerOnChannelClick);
    Connect(ID_CHECKBOX_E131,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&xCaptureFrame::OnCheckBox_E131Click);
    Connect(ID_CHECKBOX_ARTNET,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&xCaptureFrame::OnCheckBox_ArtNETClick);
    Connect(ID_BUTTON9,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xCaptureFrame::OnButton1Click);
    Connect(ID_LISTVIEW_UNIVERSES,wxEVT_COMMAND_LIST_ITEM_SELECTED,(wxObjectEventFunction)&xCaptureFrame::OnListView_UniversesItemSelect);
    Connect(ID_LISTVIEW_UNIVERSES,wxEVT_COMMAND_LIST_ITEM_ACTIVATED,(wxObjectEventFunction)&xCaptureFrame::OnListView_UniversesItemActivated);
    Connect(ID_BUTTON6,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xCaptureFrame::OnButton_AllClick);
    Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xCaptureFrame::OnButton_AddClick);
    Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xCaptureFrame::OnButton_EditClick);
    Connect(ID_BUTTON5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xCaptureFrame::OnButton_DeleteClick);
    Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&xCaptureFrame::OnChoice_TimingSelect);
    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xCaptureFrame::OnButton_StartStopClick);
    Connect(ID_BUTTON8,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xCaptureFrame::OnButton_AnalyseClick);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xCaptureFrame::OnButton_SaveClick);
    Connect(ID_BUTTON7,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xCaptureFrame::OnButton_ClearClick);
    Connect(ID_TIMER1,wxEVT_TIMER,(wxObjectEventFunction)&xCaptureFrame::OnUITimerTrigger);
    Connect(wxEVT_SIZE,(wxObjectEventFunction)&xCaptureFrame::OnResize);
    //*)

    Connect(ID_E131SOCKET, wxEVT_SOCKET, (wxObjectEventFunction)&xCaptureFrame::OnE131SocketEvent);
    Connect(ID_ARTNETSOCKET, wxEVT_SOCKET, (wxObjectEventFunction)&xCaptureFrame::OnArtNETSocketEvent);

    SetTitle("xLights Capture " + xlights_version_string + " " + GetBitness());

    wxIconBundle icons;
    icons.AddIcon(wxIcon(xlights_16_xpm));
    icons.AddIcon(wxIcon(xlights_32_xpm));
    icons.AddIcon(wxIcon(xlights_64_xpm));
    icons.AddIcon(wxIcon(xlights_128_xpm));
    icons.AddIcon(wxIcon(xlights_xpm));
    SetIcons(icons);

    ListView_Universes->AppendColumn("Start");
    ListView_Universes->AppendColumn("  End  ");

    wxIPV4address addr;
    wxString fullhostname = wxGetFullHostName();
    addr.AnyAddress();
    wxDatagramSocket* testSocket = new wxDatagramSocket(addr, wxSOCKET_NOWAIT);
    if (testSocket) delete testSocket;
    addr.Hostname(fullhostname);

    _defaultIP = addr.IPAddress();

    LoadState();

    StaticText_IP->SetLabel(_localIP);

    UITimer.Start(1000, wxTIMER_CONTINUOUS);

    if (CheckBox_ArtNET->GetValue()) CreateArtNETListener();
    if (CheckBox_E131->GetValue()) CreateE131Listener();

    Button_StartStop->SetLabel("Start");

    ValidateWindow();
}

void xCaptureFrame::LoadState()
{
    wxConfigBase* config = wxConfigBase::Get();
    int x = config->ReadLong(_("xcWindowPosX"), 50);
    int y = config->ReadLong(_("xcWindowPosY"), 50);
    int w = config->ReadLong(_("xcWindowPosW"), 800);
    int h = config->ReadLong(_("xcWindowPosH"), 600);

    // limit weirdness
    if (x < -100) x = 0;
    if (x > 2000) x = 400;
    if (y < -100) y = 0;
    if (y > 2000) y = 400;

    SetPosition(wxPoint(x, y));
    SetSize(w, h);

    wxFrame::SendSizeEvent();

    wxString localIP = config->Read(_("xcLocalIP"), "");
    if (localIP != "")
    {
        _localIP = localIP;
    }
    else
    {
        _localIP = _defaultIP;
    }

    wxString state = config->Read(_("xcState"), "");
    wxArrayString items = wxSplit(state, ',');
    for (auto it = items.begin(); it != items.end(); ++it)
    {
        if (*it != "")
        {
            wxArrayString pr = wxSplit(*it, '=');
            if (pr[0].StartsWith("ID_TEXTCTRL"))
            {
                wxTextCtrl* win = (wxTextCtrl*)FindWindowByName(pr[0], this);
                win->SetValue(pr[1]);
            }
            else if (pr[0].StartsWith("ID_CHECKBOX"))
            {
                wxCheckBox* win = (wxCheckBox*)FindWindowByName(pr[0], this);
                win->SetValue(pr[1] == "TRUE");
            }
            else if (pr[0].StartsWith("ID_SPINCTRL"))
            {
                wxSpinCtrl* win = (wxSpinCtrl*)FindWindowByName(pr[0], this);
                win->SetValue(wxAtoi(pr[1]));
            }
            else if (pr[0].StartsWith("ID_LISTVIEW"))
            {
                wxListView* win = (wxListView*)FindWindowByName(pr[0], this);
                if (pr[1] != "")
                {
                    wxArrayString rows = wxSplit(pr[1], '|');
                    for (auto it2 = rows.begin(); it2 != rows.end(); ++it2)
                    {
                        if (*it2 != "")
                        {
                            wxArrayString values = wxSplit(*it2, ':');
                            long id = win->InsertItem(win->GetItemCount(), values[0]);
                            win->SetItem(id, 1, values[1]);
                        }
                    }
                }
            }
        }
    }
}

xCaptureFrame::~xCaptureFrame()
{
    SaveState();

    CloseSockets(true);

    PurgeCollectedData();

    //(*Destroy(xCaptureFrame)
    //*)
}

void xCaptureFrame::SaveState()
{
    int x, y;
    GetPosition(&x, &y);

    int w, h;
    GetSize(&w, &h);

    wxConfigBase* config = wxConfigBase::Get();
    config->Write(_("xcWindowPosX"), x);
    config->Write(_("xcWindowPosY"), y);
    config->Write(_("xcWindowPosW"), w);
    config->Write(_("xcWindowPosH"), h);

    if (_localIP == _defaultIP)
    {
        config->DeleteEntry(_("xcLocalIP"));
    }
    else
    {
        config->Write(_("xcLocalIP"), _localIP);
    }

    wxString state;
    auto windows = GetChildren();
    for (auto it = windows.begin(); it != windows.end(); ++it)
    {
        if ((*it)->GetName().StartsWith("ID_TEXTCTRL"))
        {
            state += (*it)->GetName() + "=" + ((wxTextCtrl*)(*it))->GetValue() + ",";
        }
        else if ((*it)->GetName().StartsWith("ID_CHECKBOX"))
        {
            state += (*it)->GetName() + "=" + (((wxCheckBox*)(*it))->GetValue() ? "TRUE," : "FALSE,");
        }
        else if ((*it)->GetName().StartsWith("ID_SPINCTRL"))
        {
            state += (*it)->GetName() + "=" + wxString::Format("%d,", ((wxSpinCtrl*)(*it))->GetValue());
        }
        else if ((*it)->GetName().StartsWith("ID_LISTVIEW"))
        {
            wxString lv;
            for (int i = 0; i < ((wxListView*)(*it))->GetItemCount(); i++)
            {
                lv += ((wxListView*)(*it))->GetItemText(i, 0) + ":" + ((wxListView*)(*it))->GetItemText(i, 1) + "|";
            }
            state += (*it)->GetName() + "=" + lv + ",";
        }
    }
    config->Write(_("xcState"), state);
    config->Flush();
}

// close not required sockets
void xCaptureFrame::CloseSockets(bool force)
{
    if (force || !CheckBox_E131->GetValue())
    {
        if (_e131Socket != nullptr)
        {
            _e131Socket->Close();
            delete _e131Socket;
            _e131Socket = nullptr;
        }
    }

    if (force || !CheckBox_ArtNET->GetValue())
    {
        if (_artNETSocket != nullptr)
        {
            _artNETSocket->Close();
            delete _artNETSocket;
            _artNETSocket = nullptr;
        }
    }
}

void xCaptureFrame::OnQuit(wxCommandEvent& event)
{
    Close();
}

void xCaptureFrame::OnAbout(wxCommandEvent& event)
{
    auto about = wxString::Format(wxT("xCapture v%s %s, the xLights packet capturer."), xlights_version_string, GetBitness());
    wxMessageBox(about, _("Welcome to..."));
}

PacketData::PacketData(long type, wxByte* packet, int len)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    _timeStamp = wxDateTime::UNow();
    _frameTimeMS = -1;
    _seq = 0;
    _length = 0;
    _pdata = nullptr;

    if (type == xCaptureFrame::ID_E131SOCKET)
    {
        // validate the packet
        if (len < 126) return;
        if (packet[4] != 0x41) return;
        if (packet[5] != 0x53) return;
        if (packet[6] != 0x43) return;
        if (packet[7] != 0x2d) return;
        if (packet[8] != 0x45) return;
        if (packet[9] != 0x31) return;
        if (packet[10] != 0x2e) return;
        if (packet[11] != 0x31) return;
        if (packet[12] != 0x37) return;

        _seq = (int)packet[111];
        _length = (((int)packet[115] - 0x70) << 8) + (int)packet[116] - 11;
        if (_length > len - 126)
        {
            logger_base.warn("E131 packet of claimed length %d truncated to actual packet length %d.", _length, len - 126);
            logger_base.warn("    Packet looks unlikely to be valid.");
            _length = len - 126;
        }
        _pdata = (wxByte*)malloc(_length);
        memcpy(_pdata, &packet[126], _length);
    }
    else if (type == xCaptureFrame::ID_ARTNETSOCKET)
    {
        // validate the packet
        if (len < 18) return;
        if (packet[0] != 'A') return;
        if (packet[1] != 'r') return;
        if (packet[2] != 't') return;
        if (packet[3] != '-') return;
        if (packet[4] != 'N') return;
        if (packet[5] != 'e') return;
        if (packet[6] != 't') return;
        if (packet[9] != 0x50) return;

        _seq = (int)packet[12];
        _length = ((int)packet[16] << 8) + (int)packet[17];
        if (_length > len - 18)
        {
            logger_base.warn("ArtNet packet of claimed length %d truncated to actual packet length %d.", _length, len - 18);
            logger_base.warn("    Packet looks unlikely to be valid.");
            _length = len - 18;
        }
        _pdata = (wxByte*)malloc(_length);
        memcpy(_pdata, &packet[18], _length);
    }
}

Collector::~Collector()
{
    while (_packets.size() > 0)
    {
        auto toDelete = _packets.front();
        _packets.pop_front();
        delete toDelete;
    }
}

// relies on missing sequence numbers to detect missing frames
void Collector::CalculateFrames(wxDateTime startTime, int frameMS)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    bool first = true;
    int ms = 0;
    int lastseq = 255;

    // rebase the start time to the start time in this universe if possible
    if (_packets.size() > 0)
    {
        double rawFrameMS = (_packets.front()->_timeStamp - startTime).GetValue().ToDouble();
        ms = ((int)(rawFrameMS / frameMS)) * frameMS;
        lastseq = _packets.front()->_seq - 1;
        if (lastseq < 0) lastseq = 255;
    }

    //logger_base.debug("Allocating frames for universe %d", _universe);
    for (auto it = _packets.begin(); it != _packets.end(); ++it)
    {
        lastseq += 1;
        if (lastseq > 255) lastseq = 0;

        if (lastseq != (*it)->_seq)
        {
            // a frame is missing
            // check it is only one
            auto next = it;
            ++next;

            if (next != _packets.end() && (*next)->_seq == lastseq)
            {
                logger_base.warn("Universe %d missing one packet sequence lastSeq %d", _universe, lastseq);
                // only one frame was missing so assume it was lost
                ms += frameMS;
                lastseq += 1;
                if (lastseq > 255) lastseq = 0;
            }
            else
            {
                if (!first)
                {
                    logger_base.warn("Universe %d missing multiple packets from sequence %d", _universe, lastseq);
                }
                lastseq = (*it)->_seq;
            }
        }
        (*it)->_frameTimeMS = ms;
        ms += frameMS;
        first = false;
    }
}

PacketData* Collector::GetPacket(long ms)
{
    auto it = _packets.begin();
    while (it != _packets.end() && ms > (*it)->_frameTimeMS)
    {
        ++it;
    }

    if (it != _packets.end() && ms == (*it)->_frameTimeMS)
    {
        return (*it);
    }

    return nullptr;
}

bool Collector::operator<(const Collector& c) const
{
    if (_universe == c._universe)
    {
        if (_protocol == xCaptureFrame::ID_E131SOCKET)
        {
            return true;
        }
        return false;
    }

    return _universe < c._universe;
}

void xCaptureFrame::ValidateWindow()
{
    if (Choice_Timing->GetStringSelection() == "Manual")
    {
        SpinCtrl_ManualTime->Enable();
    }
    else
    {
        SpinCtrl_ManualTime->Disable();
    }

    if (CheckBox_TriggerOnChannel->GetValue())
    {
        SpinCtrl_Universe->Enable(true);
        SpinCtrl_Channel->Enable(true);
        SpinCtrl_TriggerStart->Enable(true);
        SpinCtrl_TriggerStop->Enable(true);
        Button_StartStop->Enable(false);
    }
    else
    {
        SpinCtrl_Universe->Enable(false);
        SpinCtrl_Channel->Enable(false);
        SpinCtrl_TriggerStart->Enable(false);
        SpinCtrl_TriggerStop->Enable(false);
        Button_StartStop->Enable(true);
    }

    if (_artNETSocket == nullptr && _e131Socket == nullptr)
    {
        Button_StartStop->Enable(false);
    }

    if (_capturedData.size() > 0 && !_capturing)
    {
        Button_Save->Enable(true);
        Button_Analyse->Enable(true);
        Button_Clear->Enable(true);
    }
    else
    {
        Button_Save->Enable(false);
        Button_Analyse->Enable(false);
        Button_Clear->Enable(false);
    }

    if (ListView_Universes->GetSelectedItemCount() == 0)
    {
        Button_Edit->Enable(false);
        Button_Delete->Enable(false);
    }
    else
    {
        if (ListView_Universes->GetItemText(ListView_Universes->GetFirstSelected()) == "All")
        {
            Button_Edit->Enable(false);
        }
        else
        {
            Button_Edit->Enable(true);
        }
        Button_Delete->Enable(true);
    }
}

void xCaptureFrame::CreateE131Listener()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (_e131Socket != nullptr) return;

    //Local address to bind to
    wxIPV4address addr;
    addr.AnyAddress();
    addr.Service(E131PORT);
    //create and bind to the address above
    _e131Socket = new wxDatagramSocket(addr);

    if (_e131Socket->IsOk())
    {
        logger_base.debug("E131 listening on %s", (const char*)_localIP.c_str());

        for (int i = 0; i < ListView_Universes->GetItemCount(); i++)
        {
            if (ListView_Universes->GetItemText(i) != "All")
            {
                int start = wxAtoi(ListView_Universes->GetItemText(i));
                int end = wxAtoi(ListView_Universes->GetItemText(i, 1));
                for (int u = start; u <= end; u++)
                {
                    struct ip_mreq mreq;
                    wxString ip = wxString::Format("239.255.%d.%d", u >> 8, u & 0xFF);
                    logger_base.debug("E131 registering for multicast on %s.", (const char *)ip.c_str());
                    mreq.imr_multiaddr.s_addr = inet_addr(ip.c_str());
                    mreq.imr_interface.s_addr = inet_addr(_localIP.c_str()); // this will only listen on the default interface
                    if (!_e131Socket->SetOption(IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char *)&mreq, sizeof(mreq)))
                    {
                        logger_base.warn("    Error opening E131 multicast listener %s.", (const char *)ip.c_str());
                    }
                }
            }
        }

        //enable event handling
        _e131Socket->SetEventHandler(*this, ID_E131SOCKET);
        //Notify us about incomming data
        _e131Socket->SetNotify(wxSOCKET_INPUT_FLAG);
        //enable event handling
        _e131Socket->Notify(true);
    }
    else
    {
        delete _e131Socket;
        _e131Socket = nullptr;
        logger_base.warn("Error opening socket to listen for e131 data");
        wxMessageBox("Error listening for E1.31 data.");
    }
}

void xCaptureFrame::CreateArtNETListener()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (_artNETSocket != nullptr) return;

    //Local address to bind to
    wxIPV4address addr;
    addr.AnyAddress();
    addr.Service(ARTNETPORT);
    //create and bind to the address above
    _artNETSocket = new wxDatagramSocket(addr);

    if (_artNETSocket->IsOk())
    {
        logger_base.debug("ARTNet listening on %s", (const char*)_localIP.c_str());

        for (int i = 0; i < ListView_Universes->GetItemCount(); i++)
        {
            if (ListView_Universes->GetItemText(i) != "All")
            {
                int start = wxAtoi(ListView_Universes->GetItemText(i));
                int end = wxAtoi(ListView_Universes->GetItemText(i, 1));
                for (int u = start; u <= end; u++)
                {
                    struct ip_mreq mreq;
                    wxString ip = wxString::Format("239.255.%d.%d", u >> 8, u & 0xFF);
                    logger_base.debug("ARTNet registering for multicast on %s.", (const char *)ip.c_str());
                    mreq.imr_multiaddr.s_addr = inet_addr(ip.c_str());
                    mreq.imr_interface.s_addr = inet_addr(_localIP.c_str()); // this will only listen on the default interface
                    if (!_artNETSocket->SetOption(IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char *)&mreq, sizeof(mreq)))
                    {
                        logger_base.warn("    Error opening ARTNet multicast listener %s.", (const char *)ip.c_str());
                    }
                }
            }
        }

        //enable event handling
        _artNETSocket->SetEventHandler(*this, ID_ARTNETSOCKET);
        //Notify us about incomming data
        _artNETSocket->SetNotify(wxSOCKET_INPUT_FLAG);
        //enable event handling
        _artNETSocket->Notify(true);
    }
    else
    {
        delete _artNETSocket;
        _artNETSocket = nullptr;
        logger_base.warn("Error opening socket to listen for ArtNET data");
        wxMessageBox("Error listening for E1.ArtNET data.");
    }
}

void xCaptureFrame::AddUniverseRange(int low, int high)
{
    if (ListView_Universes->GetItemCount() == 1 &&
        ListView_Universes->GetItemText(0) == "All")
    {
        ListView_Universes->DeleteItem(0);
    }

    bool updated = false;
    int insertat = -1;
    for (int i = 0; i < ListView_Universes->GetItemCount(); i++)
    {
        int start = wxAtoi(ListView_Universes->GetItemText(i));
        int end = wxAtoi(ListView_Universes->GetItemText(i, 1));

        if (low >= start && high <= end)
        {
            updated = true;
        }
        else if (high < start - 1)
        {
            if (!updated)
            {
                ListView_Universes->InsertItem(i, wxString::Format("%d", low));
                ListView_Universes->SetItem(i, 1, wxString::Format("%d", high));
                i++;
                updated = true;
            }
        }
        else if (low < start && high <= end)
        {
            if (!updated)
            {
                ListView_Universes->SetItem(i, 0, wxString::Format("%d", low));
                updated = true;
            }
            else
            {
                ListView_Universes->DeleteItem(i);
                i--;
            }
        }
        else if (low <= end && high > end)
        {
            if (!updated)
            {
                if (low < start)
                {
                    ListView_Universes->SetItem(i, 0, wxString::Format("%d", low));
                }
                ListView_Universes->SetItem(i, 1, wxString::Format("%d", high));
                updated = true;
            }
            else
            {
                ListView_Universes->DeleteItem(i);
                i--;
            }
        }
        else if (insertat == -1 && high < start)
        {
            insertat = i;
        }
    }

    if (!updated)
    {
        if (insertat == -1) insertat = ListView_Universes->GetItemCount();
        ListView_Universes->InsertItem(insertat, wxString::Format("%d", low));
        ListView_Universes->SetItem(insertat, 1, wxString::Format("%d", high));
    }

    // Need to redo sockets in case we have multicast
    RestartInterfaces();
    ValidateWindow();
}

void xCaptureFrame::OnResize(wxSizeEvent& event)
{
    Layout();
}

void xCaptureFrame::CreateDebugReport(wxDebugReportCompress *report) {
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

void xCaptureFrame::SendReport(const wxString &loc, wxDebugReportCompress &report) {
    wxHTTP http;
    http.Connect("dankulp.com");

    const char *bound = "--------------------------b29a7c2fe47b9481";

    wxDateTime now = wxDateTime::Now();
    int millis = wxGetUTCTimeMillis().GetLo() % 1000;
    wxString ts = wxString::Format("%04d-%02d-%02d_%02d-%02d-%02d-%03d", now.GetYear(), now.GetMonth()+1, now.GetDay(), now.GetHour(), now.GetMinute(), now.GetSecond(), millis);

    wxString fn = wxString::Format("xCapture-%s_%s_%s_%s.zip", wxPlatformInfo::Get().GetOperatingSystemFamilyName().c_str(), xlights_version_string, GetBitness(), ts);
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

void xCaptureFrame::OnKeyDown(wxKeyEvent& event)
{
    ValidateWindow();
}

void xCaptureFrame::OnButton_StartStopClick(wxCommandEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    _capturing = !_capturing;
    if (_capturing)
    {
        _capturedDesc = "";
        _capturedPackets = 0;
        PurgeCollectedData();
        Button_StartStop->SetLabel("Stop");
        _capturedDesc = "";
    }
    else
    {
        Button_StartStop->SetLabel("Start");
        UpdateCaptureDesc();

        logger_base.debug("Capture stopped.");
        for (auto it = _capturedData.begin(); it != _capturedData.end(); ++it)
        {
            logger_base.debug("    Protocol %s, Universe %d, Size %d, Frames %d",
                (*it)->_protocol == ID_E131SOCKET ? "E131" : "ArtNET",
                (*it)->_universe,
                (*it)->_packets.size() > 0 ? (*it)->_packets.front()->_length : 0,
                (int)(*it)->_packets.size()
            );
        }
    }
    ValidateWindow();
}

bool cmp(const Collector *a, const Collector *b)
{
    return *a < *b;
}

inline long RoundTo4(long i) {
    long remainder = i % 4;
    if (remainder == 0) {
        return i;
    }
    return i + 4 - remainder;
}

void xCaptureFrame::OnButton_SaveClick(wxCommandEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    // make sure the collectors are sorted
    _capturedData.sort(cmp);

    wxFileDialog dlg(this, _("Save sequence"), "", "",
        "FSEQ (*.fseq)|*.fseq|ESEQ (*.eseq)|*.eseq", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxFileName fn(dlg.GetDirectory() + "/" + dlg.GetFilename());
        wxString log = "Saving to "+ fn.GetExt().Upper() + " file " + fn.GetFullName() + "\n";

        int frameMS = GuessFrameMS();
        log += wxString::Format("Frame Time: %dms\n", frameMS);

        log += wxString::Format("Universes: %d\n", (int)_capturedData.size());

        long channelsPerFrame = RoundTo4(GetChannelsPerFrame());
        log += wxString::Format("Channels Per Frame: %ld\n", channelsPerFrame);

        wxDateTime startTime = GetStartTime();

        int frames = GetFrames();
        log += wxString::Format("Frames: %d\n", frames);

        log += wxString::Format("Channel Structure Start:\n");
        for (auto it = _capturedData.begin(); it != _capturedData.end(); ++it)
        {
            (*it)->CalculateFrames(startTime, frameMS);

            log += wxString::Format("Channel %ld, Protocol %s, Universe %d, Size %d, Frames %d, StartFrameMS %dms, EndFrameMS %dms\n",
                (*it)->_startChannel, (*it)->_protocol == ID_E131SOCKET ? "E131" : "ArtNET",
                (*it)->_universe, (*it)->_packets.size() > 0 ? (*it)->_packets.front()->_length : 0,
                (int)(*it)->_packets.size(), (*it)->_packets.size() > 0 ? (*it)->_packets.front()->_frameTimeMS : -1,
                (*it)->_packets.size() > 0 ? (*it)->_packets.back()->_frameTimeMS : -1);
        }
        log += wxString::Format("Channel Structure End!\n");

        if (fn.GetExt().Lower() == "fseq")
        {
            SaveFSEQ(fn.GetFullPath(), frameMS, channelsPerFrame, frames ,log);
        }
        else
        {
            SaveESEQ(fn.GetFullPath(), frameMS, channelsPerFrame, frames, log);
        }

        logger_base.debug(log);

        ResultDialog dlgLog(this, log);
        dlgLog.ShowModal();
    }
}

long xCaptureFrame::GetChannelsPerFrame()
{
    long size = 0;
    for (auto it = _capturedData.begin(); it != _capturedData.end(); ++it)
    {
        (*it)->_startChannel = size + 1;
        if ((*it)->_packets.size() > 0)
        {
            size += (*it)->_packets.front()->_length;
        }
    }

    return size;
}

wxDateTime xCaptureFrame::GetStartTime()
{
    wxDateTime startTime = wxDateTime::Now();
    for (auto it = _capturedData.begin(); it != _capturedData.end(); ++it)
    {
        if ((*it)->_packets.size() > 0)
        {
            if ((*it)->_packets.front()->_timeStamp < startTime)
            {
                startTime = (*it)->_packets.front()->_timeStamp;
            }
        }
    }

    return startTime;
}

void xCaptureFrame::OnCheckBox_TriggerOnChannelClick(wxCommandEvent& event)
{
    ValidateWindow();
}

void xCaptureFrame::OnListView_UniversesItemSelect(wxListEvent& event)
{
    ValidateWindow();
}

void xCaptureFrame::OnListView_UniversesItemActivated(wxListEvent& event)
{
    if (ListView_Universes->GetSelectedItemCount() > 0)
    {
        int start = wxAtoi(ListView_Universes->GetItemText(ListView_Universes->GetFirstSelected()));
        int end = wxAtoi(ListView_Universes->GetItemText(ListView_Universes->GetFirstSelected(), 1));
        UniverseEntryDialog dlg(this, start, end);
        if (dlg.ShowModal() == wxID_OK)
        {
            ListView_Universes->DeleteItem(ListView_Universes->GetFirstSelected());
            AddUniverseRange(dlg.SpinCtrl_Start->GetValue(), dlg.SpinCtrl_End->GetValue());
        }
    }
    ValidateWindow();
}

void xCaptureFrame::OnButton_AllClick(wxCommandEvent& event)
{
    while (ListView_Universes->GetItemCount() > 0)
    {
        ListView_Universes->DeleteItem(0);
    }

    ListView_Universes->InsertItem(0, "All");

    ValidateWindow();
}

void xCaptureFrame::OnButton_ClearClick(wxCommandEvent& event)
{
    _capturedDesc = "";
    _capturedPackets = 0;
    PurgeCollectedData();
    ValidateWindow();
}

void xCaptureFrame::OnCheckBox_E131Click(wxCommandEvent& event)
{
    CloseSockets();
    if (CheckBox_E131->GetValue())
    {
        CreateE131Listener();
    }
    ValidateWindow();
}

void xCaptureFrame::OnCheckBox_ArtNETClick(wxCommandEvent& event)
{
    CloseSockets();
    if (CheckBox_ArtNET->GetValue())
    {
        CreateArtNETListener();
    }
    ValidateWindow();
}

void xCaptureFrame::OnE131SocketEvent(wxSocketEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxIPV4address addr;
    addr.Service(E131PORT);
    wxByte buf[126+512];
    switch (event.GetSocketEvent())
    {
    case wxSOCKET_INPUT:
    {
        _e131Socket->Notify(false);
        size_t n = _e131Socket->RecvFrom(addr, buf, sizeof(buf)).LastCount();
        if (!n) {
            logger_base.error("ERROR: failed to receive E131 data");
            return;
        }
        //logger_base.debug("E131 packet received.");
        StashPacket(ID_E131SOCKET, buf, n);
        _e131Socket->Notify(true);
    }
        break;
    default:
        logger_base.warn("OnE131SocketEvent: Unexpected event !");
        break;
    }
}

void xCaptureFrame::OnArtNETSocketEvent(wxSocketEvent & event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxIPV4address addr;
    addr.Service(ARTNETPORT);
    wxByte buf[18 + 512];
    switch (event.GetSocketEvent())
    {
    case wxSOCKET_INPUT:
    {
        _artNETSocket->Notify(false);
        size_t n = _artNETSocket->RecvFrom(addr, buf, sizeof(buf)).LastCount();
        if (!n) {
            logger_base.error("ERROR: failed to receive ArtNET data");
            return;
        }
        //logger_base.debug("ArtNet packet received.");
        StashPacket(ID_ARTNETSOCKET, buf, n);
        _artNETSocket->Notify(true);
    }
        break;
    default:
        logger_base.warn("OnArtNETSocketEvent: Unexpected event !");
        break;
    }
}

void xCaptureFrame::OnButton_AddClick(wxCommandEvent& event)
{
    UniverseEntryDialog dlg(this, -1, -1);
    if (dlg.ShowModal() == wxID_OK)
    {
        AddUniverseRange(dlg.SpinCtrl_Start->GetValue(), dlg.SpinCtrl_End->GetValue());
    }
}

void xCaptureFrame::OnButton_EditClick(wxCommandEvent& event)
{
    if (ListView_Universes->GetSelectedItemCount() > 0)
    {
        int start = wxAtoi(ListView_Universes->GetItemText(ListView_Universes->GetFirstSelected()));
        int end = wxAtoi(ListView_Universes->GetItemText(ListView_Universes->GetFirstSelected(), 1));
        UniverseEntryDialog dlg(this, start, end);
        if (dlg.ShowModal() == wxID_OK)
        {
            ListView_Universes->DeleteItem(ListView_Universes->GetFirstSelected());
            AddUniverseRange(dlg.SpinCtrl_Start->GetValue(), dlg.SpinCtrl_End->GetValue());
        }
    }
}

void xCaptureFrame::OnButton_DeleteClick(wxCommandEvent& event)
{
    if (ListView_Universes->GetSelectedItemCount() > 0)
    {
        ListView_Universes->DeleteItem(ListView_Universes->GetFirstSelected());
    }
}

void xCaptureFrame::OnUITimerTrigger(wxTimerEvent& event)
{
    StatusBar1->SetStatusText(wxString::Format("Universes: %d Total Packets: %ld %s", (int)_capturedData.size(), _capturedPackets, _capturedDesc));
}

void xCaptureFrame::SaveFSEQ(wxString file, int frameMS, long channelsPerFrame, int frames, wxString& log)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxUint8 vMinor = 0;
    wxUint8 vMajor = 1;
    wxUint16 fixedHeaderLength = 28;
    wxUint32 stepSize = channelsPerFrame;
    wxUint16 stepTime = frameMS;
    wxUint16 numUniverses = 0;
    wxUint16 universeSize = 0;
    wxUint8 gamma = 1;
    wxUint8 colorEncoding = 2;

    int overrideFrameMS = 0;
    if (Choice_Timing->GetStringSelection() == "Manual")
    {
        overrideFrameMS = SpinCtrl_ManualTime->GetValue();
    }
    else
    {
        overrideFrameMS = wxAtoi(Choice_Timing->GetStringSelection());
    }

    if (overrideFrameMS != 0)
    {
        logger_base.debug("Frame time overriden to %s->%d. It was %d.", (const char*)Choice_Timing->GetStringSelection().c_str(), overrideFrameMS, stepTime);
        log += "Frame time override to " + wxString::Format("%d", overrideFrameMS) + "ms";
        stepTime = overrideFrameMS;
    }

    wxFile f;

    if (f.Create(file, true))
    {
        int bufsize = stepSize < 1024 ? 1024 : stepSize;
        wxUint8* buf = (wxUint8 *)calloc(sizeof(wxUint8), bufsize);
        memset(buf, 0x00, bufsize);

        buf[0] = 'P';
        buf[1] = 'S';
        buf[2] = 'E';
        buf[3] = 'Q';

        buf[6] = vMinor;
        buf[7] = vMajor;
        // Fixed header length
        buf[8] = (wxUint8)(fixedHeaderLength % 256);
        buf[9] = (wxUint8)(fixedHeaderLength / 256);
        // Step Size
        buf[10] = (wxUint8)(stepSize & 0xFF);
        buf[11] = (wxUint8)((stepSize >> 8) & 0xFF);
        buf[12] = (wxUint8)((stepSize >> 16) & 0xFF);
        buf[13] = (wxUint8)((stepSize >> 24) & 0xFF);
        // Number of Steps
        buf[14] = (wxUint8)(frames & 0xFF);
        buf[15] = (wxUint8)((frames >> 8) & 0xFF);
        buf[16] = (wxUint8)((frames >> 16) & 0xFF);
        buf[17] = (wxUint8)((frames >> 24) & 0xFF);
        // Step time in ms
        buf[18] = (wxUint8)(stepTime & 0xFF);
        buf[19] = (wxUint8)((stepTime >> 8) & 0xFF);
        // universe count
        buf[20] = (wxUint8)(numUniverses & 0xFF);
        buf[21] = (wxUint8)((numUniverses >> 8) & 0xFF);
        // universe Size
        buf[22] = (wxUint8)(universeSize & 0xFF);
        buf[23] = (wxUint8)((universeSize >> 8) & 0xFF);
        // universe Size
        buf[24] = gamma;
        // universe Size
        buf[25] = colorEncoding;
        buf[26] = 0;
        buf[27] = 0;

        buf[4] = (wxUint8)(fixedHeaderLength % 256);
        buf[5] = (wxUint8)(fixedHeaderLength / 256);
        f.Write(buf, fixedHeaderLength);

        for (int i = 0; i < frames; i++)
        {
            for (auto it = _capturedData.begin(); it != _capturedData.end(); ++it)
            {
                PacketData* p = (*it)->GetPacket(i * frameMS);
                if (p != nullptr)
                {
                    memcpy(buf + (*it)->_startChannel - 1, p->_pdata, p->_length);
                }
                else
                {
                    logger_base.debug("   No data found uni %d ch %ld ", (*it)->_universe, (*it)->_startChannel);
                }
            }
            f.Write(buf, stepSize);
        }

        f.Close();
        free(buf);
    }
    else
    {
        log += "ERROR: Unable to create file.\n";
    }
}

void xCaptureFrame::RestartInterfaces()
{
    CloseSockets(true);
    if (CheckBox_E131->GetValue())
    {
        CreateE131Listener();
    }
    if (CheckBox_ArtNET->GetValue())
    {
        CreateArtNETListener();
    }
    ValidateWindow();
}

void xCaptureFrame::UpdateCaptureDesc()
{
    if (_capturedData.size() == 0)
    {
        _capturedDesc = "";
    }
    else
    {
        int frameMS = GuessFrameMS();
        int frames = GetFrames();
        _capturedDesc = wxString::Format("Frame Interval %dms Frames %d",
            frameMS, frames).ToStdString();
    }
}

void xCaptureFrame::SaveESEQ(wxString file, int frameMS, long channelsPerFrame, int frames, wxString& log)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    long startAddr = wxGetNumberFromUser("Start channel for the model", "", "Start Channel", 1, 1, 10000000, this);
    if (startAddr == -1)
    {
        startAddr = 1;
    }

    int overrideFrameMS = 0;
    if (Choice_Timing->GetStringSelection() == "Manual")
    {
        overrideFrameMS = SpinCtrl_ManualTime->GetValue();
    }
    else
    {
        overrideFrameMS = wxAtoi(Choice_Timing->GetStringSelection());
    }

    if (overrideFrameMS != 0)
    {
        logger_base.debug("Frame time overriden to %s->%d. It was detected as %d", (const char*)Choice_Timing->GetStringSelection().c_str(), overrideFrameMS, frameMS);
        log += "Frame time override to " + wxString::Format("%d", overrideFrameMS) + "ms";
        frameMS = overrideFrameMS;
    }

    wxUint16 fixedHeaderLength = 20;
    wxUint32 modelSize = channelsPerFrame;
    wxUint32 frameSize = RoundTo4(channelsPerFrame);
    wxFile f;

    if (f.Create(file, true))
    {
        int bufsize = frameSize < 1024 ? 1024 : frameSize;
        wxUint8* buf = (wxUint8 *)calloc(sizeof(wxUint8), bufsize);
        memset(buf, 0x00, bufsize);

        buf[0] = 'E';
        buf[1] = 'S';
        buf[2] = 'E';
        buf[3] = 'Q';
        // Data offset
        buf[4] = (wxUint8)1; //Hard coded to export a single model for now
        buf[5] = 0; //Pad byte
        buf[6] = 0; //Pad byte
        buf[7] = 0; //Pad byte
                    // Step Size
        buf[8] = (wxUint8)(frameSize & 0xFF);
        buf[9] = (wxUint8)((frameSize >> 8) & 0xFF);
        buf[10] = (wxUint8)((frameSize >> 16) & 0xFF);
        buf[11] = (wxUint8)((frameSize >> 24) & 0xFF);
        //Model Start address
        buf[12] = (wxUint8)(startAddr & 0xFF);
        buf[13] = (wxUint8)((startAddr >> 8) & 0xFF);
        buf[14] = (wxUint8)((startAddr >> 16) & 0xFF);
        buf[15] = (wxUint8)((startAddr >> 24) & 0xFF);
        // Model Size
        buf[16] = (wxUint8)(modelSize & 0xFF);
        buf[17] = (wxUint8)((modelSize >> 8) & 0xFF);
        buf[18] = (wxUint8)((modelSize >> 16) & 0xFF);
        buf[19] = (wxUint8)((modelSize >> 24) & 0xFF);
        f.Write(buf, fixedHeaderLength);

        for (int i = 0; i < frames; i++)
        {
            //logger_base.debug("Writing frame %d %dms", i + 1, i * frameMS);
            for (auto it = _capturedData.begin(); it != _capturedData.end(); ++it)
            {
                PacketData* p = (*it)->GetPacket(i * frameMS);
                if (p != nullptr)
                {
                    //logger_base.debug("   Adding data uni %d ch %ld time %dms len %d seq %d time %d.%03d", (*it)->_universe, (*it)->_startChannel, p->_frameTimeMS, p->_length, p->_seq, p->_timeStamp.GetSecond(), p->_timeStamp.GetMillisecond());
                    memcpy(buf + (*it)->_startChannel - 1, p->_pdata, p->_length);
                }
                else
                {
                    logger_base.debug("   No data found uni %d ch %ld ", (*it)->_universe, (*it)->_startChannel);
                }
            }
            f.Write(buf, frameSize);
        }

        f.Close();
        free(buf);
    }
    else
    {
        log += "ERROR: Unable to create file.\n";
    }
}

int xCaptureFrame::GetFrames()
{
    int frames = 0;
    for (auto it = _capturedData.begin(); it != _capturedData.end(); ++it)
    {
        frames = std::max(frames, (int)(*it)->_packets.size());
    }
    return frames;
}

void xCaptureFrame::OnButton_AnalyseClick(wxCommandEvent& event)
{
    int frameMS = GuessFrameMS();
    wxString log = wxString::Format("Frame Time: %dms\n", frameMS);

    log += wxString::Format("Universes: %d\n", (int)_capturedData.size());

    long channelsPerFrame = RoundTo4(GetChannelsPerFrame());
    log += wxString::Format("Channels Per Frame: %ld\n", channelsPerFrame);

    wxDateTime startTime = GetStartTime();

    int frames = GetFrames();
    log += wxString::Format("Frames: %d\n", frames);

    log += wxString::Format("Channel Structure Start:\n");
    for (auto it = _capturedData.begin(); it != _capturedData.end(); ++it)
    {
        (*it)->CalculateFrames(startTime, frameMS);

        log += wxString::Format("Channel %ld, Protocol %s, Universe %d, Size %d, Frames %d, StartFrameMS %dms, EndFrameMS %dms\n",
            (*it)->_startChannel, (*it)->_protocol == ID_E131SOCKET ? "E131" : "ArtNET",
            (*it)->_universe, (*it)->_packets.size() > 0 ? (*it)->_packets.front()->_length : 0,
            (int)(*it)->_packets.size(), (*it)->_packets.size() > 0 ? (*it)->_packets.front()->_frameTimeMS : -1,
            (*it)->_packets.size() > 0 ? (*it)->_packets.back()->_frameTimeMS : -1);
    }
    log += wxString::Format("Channel Structure End!\n");

    ResultDialog dlgLog(this, log);
    dlgLog.ShowModal();
}

void xCaptureFrame::OnButton1Click(wxCommandEvent& event)
{
    IPEntryDialog dlg(this);
    dlg.TextCtrl_IPAddress->SetValue(_localIP);

    if (dlg.ShowModal() == wxID_OK)
    {
        if (dlg.TextCtrl_IPAddress->GetValue() == "")
        {
            _localIP = _defaultIP;
        }
        else
        {
            _localIP = dlg.TextCtrl_IPAddress->GetValue();
        }
        StaticText_IP->SetLabel(_localIP);
        RestartInterfaces();
    }
}

void xCaptureFrame::OnChoice_TimingSelect(wxCommandEvent& event)
{
    if (Choice_Timing->GetStringSelection() == "25")
    {
        SpinCtrl_ManualTime->SetValue(25);
    }
    else if (Choice_Timing->GetStringSelection() == "30")
    {
        SpinCtrl_ManualTime->SetValue(30);
    }
    else if (Choice_Timing->GetStringSelection() == "33")
    {
        SpinCtrl_ManualTime->SetValue(33);
    }
    else if (Choice_Timing->GetStringSelection() == "50")
    {
        SpinCtrl_ManualTime->SetValue(50);
    }
    else if (Choice_Timing->GetStringSelection() == "100")
    {
        SpinCtrl_ManualTime->SetValue(100);
    }
    ValidateWindow();
}
