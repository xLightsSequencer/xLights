/***************************************************************
 * Name:      xFadeMain.cpp
 * Purpose:   Code for Application Frame
 * Author:    xLights ()
 * Created:   2016-12-30
 * Copyright: xLights (http://xlights.org)
 * License:
 **************************************************************/

#define ZERO 0
#define E131PORT 5568
#define ARTNETPORT 0x1936

#include "xFadeMain.h"
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
#include "../xLights/IPEntryDialog.h"
#include "Emitter.h"
#include "../xLights/UtilFunctions.h"

#ifndef __WXMSW__
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include "../include/xLights.xpm"
#include "../include/xLights-16.xpm"
#include "../include/xLights-32.xpm"
#include "../include/xLights-64.xpm"
#include "../include/xLights-128.xpm"

//(*InternalHeaders(xFadeFrame)
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

//(*IdInit(xFadeFrame)
const long xFadeFrame::ID_STATICTEXT6 = wxNewId();
const long xFadeFrame::ID_TEXTCTRL3 = wxNewId();
const long xFadeFrame::ID_STATICTEXT10 = wxNewId();
const long xFadeFrame::ID_TEXTCTRL4 = wxNewId();
const long xFadeFrame::ID_PANEL1 = wxNewId();
const long xFadeFrame::ID_BUTTON2 = wxNewId();
const long xFadeFrame::ID_STATICTEXT5 = wxNewId();
const long xFadeFrame::ID_CHECKBOX_E131 = wxNewId();
const long xFadeFrame::ID_CHECKBOX_ARTNET = wxNewId();
const long xFadeFrame::ID_STATICTEXT7 = wxNewId();
const long xFadeFrame::ID_STATICTEXT8 = wxNewId();
const long xFadeFrame::ID_BUTTON9 = wxNewId();
const long xFadeFrame::ID_STATICTEXT2 = wxNewId();
const long xFadeFrame::ID_STATICTEXT3 = wxNewId();
const long xFadeFrame::ID_BUTTON12 = wxNewId();
const long xFadeFrame::ID_STATICTEXT9 = wxNewId();
const long xFadeFrame::ID_CHOICE1 = wxNewId();
const long xFadeFrame::ID_STATICLINE1 = wxNewId();
const long xFadeFrame::ID_STATICTEXT1 = wxNewId();
const long xFadeFrame::ID_TEXTCTRL1 = wxNewId();
const long xFadeFrame::ID_STATICTEXT14 = wxNewId();
const long xFadeFrame::ID_PANEL3 = wxNewId();
const long xFadeFrame::ID_BUTTON1 = wxNewId();
const long xFadeFrame::ID_BUTTON10 = wxNewId();
const long xFadeFrame::ID_SLIDER1 = wxNewId();
const long xFadeFrame::ID_BUTTON11 = wxNewId();
const long xFadeFrame::ID_STATICLINE2 = wxNewId();
const long xFadeFrame::ID_LISTVIEW_UNIVERSES = wxNewId();
const long xFadeFrame::ID_BUTTON3 = wxNewId();
const long xFadeFrame::ID_BUTTON4 = wxNewId();
const long xFadeFrame::ID_BUTTON5 = wxNewId();
const long xFadeFrame::ID_STATICTEXT4 = wxNewId();
const long xFadeFrame::ID_TEXTCTRL2 = wxNewId();
const long xFadeFrame::ID_STATICTEXT11 = wxNewId();
const long xFadeFrame::ID_TEXTCTRL5 = wxNewId();
const long xFadeFrame::ID_PANEL2 = wxNewId();
const long xFadeFrame::ID_STATUSBAR1 = wxNewId();
const long xFadeFrame::ID_TIMER1 = wxNewId();
//*)

const long xFadeFrame::ID_E131SOCKET = wxNewId();
const long xFadeFrame::ID_ARTNETSOCKET = wxNewId();

BEGIN_EVENT_TABLE(xFadeFrame,wxFrame)
    //(*EventTable(xFadeFrame)
    //*)
END_EVENT_TABLE()

void xFadeFrame::StashPacket(long type, wxByte* packet, int len)
{
    bool left = IsLeft(type ,packet, len);
    bool right = IsRight(type, packet, len);
    int universe = -1;
    std::string source = "";
    if (type == ID_E131SOCKET)
    {
        universe = ((int)packet[113] << 8) + (int)packet[114];
        source = std::string((char*)&packet[11], 10);
    }
    else if (type == ID_ARTNETSOCKET)
    {
        universe = ((int)packet[15] << 8) + (int)packet[14];
        source = std::string((char*)&packet[11], 10);
    }

    if (universe == -1) return;

    if (!IsUniverseToBeCaptured(universe)) return;

    {
        std::unique_lock<std::mutex> mutLock(_lock);

        if (left)
        {
            _leftData[universe].Update(type, packet, len);
        }
        else if (right)
        {
            _rightData[universe].Update(type, packet, len);
        }
    }
}

bool xFadeFrame::IsLeft(long type, wxByte* packet, int len)
{
    if (type == ID_E131SOCKET)
    {
        std::string tag = ExtractE131Tag(packet);
        if (tag == _leftTag) return true;
        if (tag == _rightTag) return false;

        if (_leftTag == "")
        {
            _leftTag = tag;
            TextCtrl_LeftTag->SetValue(_leftTag);
            return true;
        }
    }
    else if (type == ID_ARTNETSOCKET)
    {
        if (_leftTag == "ARTNET") return true;
        if (_rightTag == "ARTNET") return false;
        if (_leftTag == "" && _rightTag != "")
        {
            _leftTag = "ARTNET";
            TextCtrl_LeftTag->SetValue(_leftTag);
            return true;
        }
    }
    return false;
}

bool xFadeFrame::IsRight(long type, wxByte* packet, int len)
{
    if (type == ID_E131SOCKET)
    {
        std::string tag = ExtractE131Tag(packet);
        if (tag == _rightTag) return true;
        if (tag == _leftTag) return false;

        if (_leftTag != "" && _rightTag == "")
        {
            _rightTag = tag;
            TextCtrl_RightTag->SetValue(_rightTag);
            return true;
        }
    }
    else if (type == ID_ARTNETSOCKET)
    {
        if (_leftTag == "ARTNET") return false;
        if (_rightTag == "ARTNET") return true;
        if (_rightTag == "")
        {
            _rightTag = "ARTNET";
            TextCtrl_RightTag->SetValue(_rightTag);
            return true;
        }
    }
    return false;
}

bool xFadeFrame::IsUniverseToBeCaptured(int universe)
{
    return _targetIP.find(universe) != _targetIP.end();
}

xFadeFrame::xFadeFrame(wxWindow* parent, wxWindowID id)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    _e131SocketReceive = nullptr;
    _artNETSocketReceive = nullptr;
    _emitter = nullptr;

    //(*Initialize(xFadeFrame)
    wxBoxSizer* BoxSizer1;
    wxFlexGridSizer* FlexGridSizer10;
    wxFlexGridSizer* FlexGridSizer11;
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer4;
    wxFlexGridSizer* FlexGridSizer5;
    wxFlexGridSizer* FlexGridSizer6;
    wxFlexGridSizer* FlexGridSizer7;
    wxFlexGridSizer* FlexGridSizer8;
    wxFlexGridSizer* FlexGridSizer9;
    wxGridSizer* GridSizer_LeftJukeBox;
    wxGridSizer* GridSizer_RightJukebox;

    Create(parent, wxID_ANY, _("xLights Cross Fade"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("wxID_ANY"));
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer1->AddGrowableRow(0);
    BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    FlexGridSizer10 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer10->AddGrowableCol(0);
    FlexGridSizer10->AddGrowableRow(1);
    FlexGridSizer11 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer11->AddGrowableCol(1);
    StaticText4 = new wxStaticText(this, ID_STATICTEXT6, _("Sequence:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
    FlexGridSizer11->Add(StaticText4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_LeftSequence = new wxTextCtrl(this, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL3"));
    FlexGridSizer11->Add(TextCtrl_LeftSequence, 1, wxALL|wxEXPAND, 5);
    StaticText8 = new wxStaticText(this, ID_STATICTEXT10, _("Tag:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT10"));
    FlexGridSizer11->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_LeftTag = new wxTextCtrl(this, ID_TEXTCTRL4, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL4"));
    FlexGridSizer11->Add(TextCtrl_LeftTag, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer10->Add(FlexGridSizer11, 1, wxALL|wxEXPAND, 5);
    Panel_Left = new wxPanel(this, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    GridSizer_LeftJukeBox = new wxGridSizer(5, 10, 0, 0);
    Panel_Left->SetSizer(GridSizer_LeftJukeBox);
    GridSizer_LeftJukeBox->Fit(Panel_Left);
    GridSizer_LeftJukeBox->SetSizeHints(Panel_Left);
    FlexGridSizer10->Add(Panel_Left, 1, wxALL|wxEXPAND, 5);
    BoxSizer1->Add(FlexGridSizer10, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer2->AddGrowableCol(0);
    FlexGridSizer2->AddGrowableRow(5);
    Button_ConnectToxLights = new wxButton(this, ID_BUTTON2, _("Connect To xLights"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    FlexGridSizer2->Add(Button_ConnectToxLights, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer6 = new wxFlexGridSizer(0, 3, 0, 0);
    StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Protocols:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
    FlexGridSizer6->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_E131 = new wxCheckBox(this, ID_CHECKBOX_E131, _("E131"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_E131"));
    CheckBox_E131->SetValue(true);
    FlexGridSizer6->Add(CheckBox_E131, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_ArtNET = new wxCheckBox(this, ID_CHECKBOX_ARTNET, _("ArtNET"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_ARTNET"));
    CheckBox_ArtNET->SetValue(false);
    FlexGridSizer6->Add(CheckBox_ArtNET, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText7 = new wxStaticText(this, ID_STATICTEXT7, _("Input Interface:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
    FlexGridSizer6->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_InputIP = new wxStaticText(this, ID_STATICTEXT8, _("UNKNOWN"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
    FlexGridSizer6->Add(StaticText_InputIP, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_ForceInput = new wxButton(this, ID_BUTTON9, _("Force"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON9"));
    FlexGridSizer6->Add(Button_ForceInput, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Output Interface:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer6->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_OutputIP = new wxStaticText(this, ID_STATICTEXT3, _("UNKNOWN"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer6->Add(StaticText_OutputIP, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_ForceOutput = new wxButton(this, ID_BUTTON12, _("Force"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON12"));
    FlexGridSizer6->Add(Button_ForceOutput, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText6 = new wxStaticText(this, ID_STATICTEXT9, _("Frame Timing:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
    FlexGridSizer6->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Choice_FrameTiming = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
    Choice_FrameTiming->Append(_("25ms"));
    Choice_FrameTiming->Append(_("30ms"));
    Choice_FrameTiming->SetSelection( Choice_FrameTiming->Append(_("50ms")) );
    Choice_FrameTiming->Append(_("100ms"));
    FlexGridSizer6->Add(Choice_FrameTiming, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer6->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(FlexGridSizer6, 1, wxALL|wxEXPAND, 5);
    StaticLine1 = new wxStaticLine(this, ID_STATICLINE1, wxDefaultPosition, wxSize(10,-1), wxLI_HORIZONTAL, _T("ID_STATICLINE1"));
    FlexGridSizer2->Add(StaticLine1, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer8 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer8->AddGrowableCol(1);
    StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Cross Fade Time:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer8->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_CrossFadeTime = new wxTextCtrl(this, ID_TEXTCTRL1, _("1.00"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, wxDefaultValidator, _T("ID_TEXTCTRL1"));
    FlexGridSizer8->Add(TextCtrl_CrossFadeTime, 1, wxALL|wxEXPAND, 5);
    StaticText10 = new wxStaticText(this, ID_STATICTEXT14, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT14"));
    FlexGridSizer8->Add(StaticText10, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel_FadeTime = new wxPanel(this, ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL3"));
    GridSizer_TimePresets = new wxGridSizer(0, 5, 0, 0);
    Panel_FadeTime->SetSizer(GridSizer_TimePresets);
    GridSizer_TimePresets->Fit(Panel_FadeTime);
    GridSizer_TimePresets->SetSizeHints(Panel_FadeTime);
    FlexGridSizer8->Add(Panel_FadeTime, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer2->Add(FlexGridSizer8, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer3->AddGrowableCol(1);
    FlexGridSizer3->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Middle = new wxButton(this, ID_BUTTON1, _("v"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer3->Add(Button_Middle, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer3->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Left = new wxButton(this, ID_BUTTON10, _("<"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON10"));
    FlexGridSizer3->Add(Button_Left, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Slider1 = new wxSlider(this, ID_SLIDER1, 0, 0, 10000, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER1"));
    FlexGridSizer3->Add(Slider1, 1, wxALL|wxEXPAND, 5);
    Button_Right = new wxButton(this, ID_BUTTON11, _(">"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON11"));
    FlexGridSizer3->Add(Button_Right, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 5);
    StaticLine2 = new wxStaticLine(this, ID_STATICLINE2, wxDefaultPosition, wxSize(10,-1), wxLI_HORIZONTAL, _T("ID_STATICLINE2"));
    FlexGridSizer2->Add(StaticLine2, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer4->AddGrowableCol(0);
    FlexGridSizer4->AddGrowableRow(0);
    ListView_Universes = new wxListView(this, ID_LISTVIEW_UNIVERSES, wxDefaultPosition, wxDefaultSize, wxLC_REPORT, wxDefaultValidator, _T("ID_LISTVIEW_UNIVERSES"));
    FlexGridSizer4->Add(ListView_Universes, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer5 = new wxFlexGridSizer(0, 1, 0, 0);
    Button_Add = new wxButton(this, ID_BUTTON3, _("Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    FlexGridSizer5->Add(Button_Add, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Edit = new wxButton(this, ID_BUTTON4, _("Edit"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
    FlexGridSizer5->Add(Button_Edit, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Delete = new wxButton(this, ID_BUTTON5, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON5"));
    FlexGridSizer5->Add(Button_Delete, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer4->Add(FlexGridSizer5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 5);
    BoxSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer7 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer7->AddGrowableCol(0);
    FlexGridSizer7->AddGrowableRow(1);
    FlexGridSizer9 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer9->AddGrowableCol(1);
    StaticText3 = new wxStaticText(this, ID_STATICTEXT4, _("Sequence:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    FlexGridSizer9->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_RightSequence = new wxTextCtrl(this, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL2"));
    FlexGridSizer9->Add(TextCtrl_RightSequence, 1, wxALL|wxEXPAND, 5);
    StaticText9 = new wxStaticText(this, ID_STATICTEXT11, _("Tag:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT11"));
    FlexGridSizer9->Add(StaticText9, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_RightTag = new wxTextCtrl(this, ID_TEXTCTRL5, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL5"));
    FlexGridSizer9->Add(TextCtrl_RightTag, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer7->Add(FlexGridSizer9, 1, wxALL|wxEXPAND, 5);
    Panel_Right = new wxPanel(this, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
    GridSizer_RightJukebox = new wxGridSizer(5, 10, 0, 0);
    Panel_Right->SetSizer(GridSizer_RightJukebox);
    GridSizer_RightJukebox->Fit(Panel_Right);
    GridSizer_RightJukebox->SetSizeHints(Panel_Right);
    FlexGridSizer7->Add(Panel_Right, 1, wxALL|wxEXPAND, 5);
    BoxSizer1->Add(FlexGridSizer7, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer1->Add(BoxSizer1, 1, wxALL|wxEXPAND, 5);
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

    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xFadeFrame::OnButton_ConnectToxLightsClick);
    Connect(ID_CHECKBOX_E131,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&xFadeFrame::OnCheckBox_E131Click);
    Connect(ID_CHECKBOX_ARTNET,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&xFadeFrame::OnCheckBox_ArtNETClick);
    Connect(ID_BUTTON9,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xFadeFrame::OnButton_ForceInputClick);
    Connect(ID_BUTTON12,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xFadeFrame::OnButton_ForceOutputClick);
    Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&xFadeFrame::OnChoice_FrameTimingSelect);
    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xFadeFrame::OnButton_MiddleClick);
    Connect(ID_BUTTON10,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xFadeFrame::OnButton_LeftClick);
    Connect(ID_SLIDER1,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&xFadeFrame::OnSlider1CmdSliderUpdated);
    Connect(ID_BUTTON11,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xFadeFrame::OnButton_RightClick);
    Connect(ID_LISTVIEW_UNIVERSES,wxEVT_COMMAND_LIST_ITEM_SELECTED,(wxObjectEventFunction)&xFadeFrame::OnListView_UniversesItemSelect);
    Connect(ID_LISTVIEW_UNIVERSES,wxEVT_COMMAND_LIST_ITEM_DESELECTED,(wxObjectEventFunction)&xFadeFrame::OnListView_UniversesItemDeselect);
    Connect(ID_LISTVIEW_UNIVERSES,wxEVT_COMMAND_LIST_ITEM_ACTIVATED,(wxObjectEventFunction)&xFadeFrame::OnListView_UniversesItemActivated);
    Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xFadeFrame::OnButton_AddClick);
    Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xFadeFrame::OnButton_EditClick);
    Connect(ID_BUTTON5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xFadeFrame::OnButton_DeleteClick);
    Connect(ID_TIMER1,wxEVT_TIMER,(wxObjectEventFunction)&xFadeFrame::OnUITimerTrigger);
    //*)

    Connect(ID_E131SOCKET, wxEVT_SOCKET, (wxObjectEventFunction)&xFadeFrame::OnE131SocketEvent);
    Connect(ID_ARTNETSOCKET, wxEVT_SOCKET, (wxObjectEventFunction)&xFadeFrame::OnArtNETSocketEvent);

    Connect(Button_Left->GetId(), wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&xFadeFrame::OnButtonRClickFadeLeft);
    Connect(Button_Middle->GetId(), wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&xFadeFrame::OnButtonRClickFadeMiddle);
    Connect(Button_Right->GetId(), wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&xFadeFrame::OnButtonRClickFadeRight);
    Connect(TextCtrl_CrossFadeTime->GetId(), wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&xFadeFrame::OnTextCtrlRClickCrossFadeTime);

    SetTitle("xLights Fade " + xlights_version_string + " " + GetBitness());

    wxIconBundle icons;
    icons.AddIcon(wxIcon(xlights_16_xpm));
    icons.AddIcon(wxIcon(xlights_32_xpm));
    icons.AddIcon(wxIcon(xlights_64_xpm));
    icons.AddIcon(wxIcon(xlights_128_xpm));
    icons.AddIcon(wxIcon(xlights_xpm));
    SetIcons(icons);

    ListView_Universes->AppendColumn("Start");
    ListView_Universes->AppendColumn("End");
    ListView_Universes->AppendColumn("IP Address");
    ListView_Universes->AppendColumn("Description");
    ListView_Universes->AppendColumn("Protocol");

    wxIPV4address addr;
    wxString fullhostname = wxGetFullHostName();
    addr.AnyAddress();
    wxDatagramSocket* testSocket = new wxDatagramSocket(addr, wxSOCKET_NOWAIT);
    if (testSocket) delete testSocket;
    addr.Hostname(fullhostname);

    _defaultIP = addr.IPAddress();

    LoadState();

    StaticText_InputIP->SetLabel(_localInputIP);
    StaticText_OutputIP->SetLabel(_localOutputIP);

    _emitter = new Emitter(&_targetIP, &_leftData, &_rightData, &_targetProtocol, &_lock, _localOutputIP);

    for (int i = 0; i < JUKEBOXBUTTONS; i++)
    {
        wxButton* button = new wxButton(Panel_Left, wxID_ANY, wxString::Format("%d", i + 1), wxDefaultPosition, wxSize(BUTTONWIDTH, BUTTONHEIGHT),
            0, wxDefaultValidator, _T("ID_BITMAPBUTTON_JBL") + wxString::Format("%d", i + 1));
        button->SetMinSize(wxSize(BUTTONWIDTH, BUTTONHEIGHT));
        button->SetMaxSize(wxSize(BUTTONWIDTH, BUTTONHEIGHT));
        GridSizer_LeftJukeBox->Add(button, 1, wxALL | wxEXPAND);
        Connect(button->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xFadeFrame::OnButtonClickLeft);
        Connect(button->GetId(), wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&xFadeFrame::OnButtonRClickLeft);

        button = new wxButton(Panel_Right, wxID_ANY, wxString::Format("%d", i + 1), wxDefaultPosition, wxSize(BUTTONWIDTH, BUTTONHEIGHT),
            0, wxDefaultValidator, _T("ID_BITMAPBUTTON_JBR") + wxString::Format("%d", i + 1));
        button->SetMinSize(wxSize(BUTTONWIDTH, BUTTONHEIGHT));
        button->SetMaxSize(wxSize(BUTTONWIDTH, BUTTONHEIGHT));
        GridSizer_RightJukebox->Add(button, 1, wxALL | wxEXPAND);
        Connect(button->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xFadeFrame::OnButtonClickRight);
        Connect(button->GetId(), wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&xFadeFrame::OnButtonRClickRight);
        _defaultColour = button->GetBackgroundColour();
    }

    AddFadeTimeButton("0.5");
    AddFadeTimeButton("1.0");
    AddFadeTimeButton("1.5");
    AddFadeTimeButton("2.0");
    AddFadeTimeButton("2.5");
    AddFadeTimeButton("3.0");
    AddFadeTimeButton("5.0");

    Layout();

    _direction = -1;
    SetFade();
    SetTiming();

    if (CheckBox_ArtNET->GetValue()) CreateArtNETListener();
    if (CheckBox_E131->GetValue()) CreateE131Listener();

    TextCtrl_LeftSequence->SetValue("");
    TextCtrl_RightSequence->SetValue("");
    TextCtrl_LeftTag->SetValue("");
    TextCtrl_RightTag->SetValue("");

    ValidateWindow();
}

void xFadeFrame::LoadState()
{
    wxConfigBase* config = wxConfigBase::Get();
    int x = config->ReadLong(_("xfWindowPosX"), 50);
    int y = config->ReadLong(_("xfWindowPosY"), 50);
    int w = config->ReadLong(_("xfWindowPosW"), 800);
    int h = config->ReadLong(_("xfWindowPosH"), 600);

    // limit weirdness
    if (x < -100) x = 0;
    if (x > 2000) x = 400;
    if (y < -100) y = 0;
    if (y > 2000) y = 400;

    SetPosition(wxPoint(x, y));
    SetSize(w, h);
    Layout();

    wxString localInputIP = config->Read(_("xfLocalInputIP"), "");
    if (localInputIP != "")
    {
        _localInputIP = localInputIP;
    }
    else
    {
        _localInputIP = _defaultIP;
    }

    wxString localOutputIP = config->Read(_("xfLocalOutputIP"), "");
    if (localOutputIP != "")
    {
        _localOutputIP = localOutputIP;
    }
    else
    {
        _localOutputIP = _defaultIP;
    }

    wxString state = config->Read(_("xfState"), "");
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
                            win->SetItem(id, 2, values[2]);
                            win->SetItem(id, 3, values[3]);
                            if (values.size() > 4)
                            {
                                win->SetItem(id, 4, values[4]);
                            }
                            else
                            {
                                win->SetItem(id, 4, "As per input");
                            }

                            for (int i = wxAtoi(values[0]); i <= wxAtoi(values[1]); i++)
                            {
                                _targetIP[i] = values[2];
                                _targetDesc[i] = values[3];
                                if (values.size() > 4)
                                {
                                    _targetProtocol[i] = values[4];
                                }
                                else
                                {
                                    _targetProtocol[i] = "As per input";
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

xFadeFrame::~xFadeFrame()
{
    if (_emitter != nullptr)
    {
        _emitter->Stop();
    }

    SaveState();

    CloseSockets(true);

    if (_emitter != nullptr)
    {
        delete _emitter;
        _emitter = nullptr;
    }

    //(*Destroy(xFadeFrame)
    //*)
}

void xFadeFrame::SaveState()
{
    int x, y;
    GetPosition(&x, &y);

    int w, h;
    GetSize(&w, &h);

    wxConfigBase* config = wxConfigBase::Get();
    config->Write(_("xfWindowPosX"), x);
    config->Write(_("xfWindowPosY"), y);
    config->Write(_("xfWindowPosW"), w);
    config->Write(_("xfWindowPosH"), h);

    if (_localInputIP == _defaultIP)
    {
        config->DeleteEntry(_("xfLocalInputIP"));
    }
    else
    {
        config->Write(_("xfLocalInputIP"), _localInputIP);
    }

    if (_localOutputIP == _defaultIP)
    {
        config->DeleteEntry(_("xfLocalOutputIP"));
    }
    else
    {
        config->Write(_("xfLocalOutputIP"), _localOutputIP);
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
                lv += ((wxListView*)(*it))->GetItemText(i, 0) + ":" +
                    ((wxListView*)(*it))->GetItemText(i, 1) + ":" +
                    ((wxListView*)(*it))->GetItemText(i, 2) + ":" +
                    ((wxListView*)(*it))->GetItemText(i, 3) + ":" +
                    ((wxListView*)(*it))->GetItemText(i, 4) + "|";
            }
            state += (*it)->GetName() + "=" + lv + ",";
        }
    }
    config->Write(_("xfState"), state);
    config->Flush();
}

// close not required sockets
void xFadeFrame::CloseSockets(bool force)
{
    if (force || !CheckBox_E131->GetValue())
    {
        if (_e131SocketReceive != nullptr)
        {
            _e131SocketReceive->Close();
            delete _e131SocketReceive;
            _e131SocketReceive = nullptr;
        }
    }

    if (force || !CheckBox_ArtNET->GetValue())
    {
        if (_artNETSocketReceive != nullptr)
        {
            _artNETSocketReceive->Close();
            delete _artNETSocketReceive;
            _artNETSocketReceive = nullptr;
        }
    }
}

void xFadeFrame::AddFadeTimeButton(std::string label)
{
    wxButton* button = new wxButton(Panel_FadeTime, wxID_ANY, label, wxDefaultPosition, wxSize(2 * BUTTONWIDTH, BUTTONHEIGHT),
        0, wxDefaultValidator, _T("ID_BITMAPBUTTON_FBT") /*+ label*/);
    button->SetMinSize(wxSize(2*BUTTONWIDTH, BUTTONHEIGHT));
    button->SetMaxSize(wxSize(2*BUTTONWIDTH, BUTTONHEIGHT));
    GridSizer_TimePresets->Add(button, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);
    GridSizer_TimePresets->SetCols(GridSizer_TimePresets->GetItemCount());
    Connect(button->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xFadeFrame::OnButtonClickFT);
    Connect(button->GetId(), wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&xFadeFrame::OnButtonRClickFT);
}

void xFadeFrame::OnQuit(wxCommandEvent& event)
{
    if (_emitter != nullptr) _emitter->Stop();
    Close();
}

void xFadeFrame::OnAbout(wxCommandEvent& event)
{
    auto about = wxString::Format(wxT("xFade v%s %s, the xLights cross fader."), xlights_version_string, GetBitness());
    wxMessageBox(about, _("Welcome to..."));
}

PacketData::PacketData()
{
    _length = 0; 
    _type = 0; 
    _universe = 0;
    _tag = wxString::Format("xFade %d", wxGetProcessId()).ToStdString();
}

wxByte PacketData::GetData(int c)
{
    if (_type == xFadeFrame::ID_E131SOCKET)
    {
        if (_length >= c + E131_PACKET_HEADERLEN)
        {
            return _data[c + E131_PACKET_HEADERLEN];
        }
    }
    else if (_type == xFadeFrame::ID_ARTNETSOCKET)
    {
        if (_length >= c + ARTNET_PACKET_HEADERLEN)
        {
            return _data[c + ARTNET_PACKET_HEADERLEN];
        }
    }

    return 0;
}

wxByte* PacketData::GetDataPtr()
{
    if (_type == xFadeFrame::ID_E131SOCKET)
    {
        return &_data[E131_PACKET_HEADERLEN];
    }
    else if (_type == xFadeFrame::ID_ARTNETSOCKET)
    {
        return &_data[ARTNET_PACKET_HEADERLEN];
    }

    return nullptr;
}

void PacketData::SetData(int c, wxByte dd)
{
    if (_type == xFadeFrame::ID_E131SOCKET)
    {
        if (_length >= c + E131_PACKET_HEADERLEN)
        {
            _data[c + E131_PACKET_HEADERLEN] = dd;
        }
    }
    else if (_type == xFadeFrame::ID_ARTNETSOCKET)
    {
        if (_length >= c + ARTNET_PACKET_HEADERLEN)
        {
            _data[c + ARTNET_PACKET_HEADERLEN] = dd;
        }
    }
}

void PacketData::Update(long type, wxByte* packet, int len)
{
    if (type == xFadeFrame::ID_E131SOCKET)
    {
        // validate the packet
        if (len < E131_PACKET_HEADERLEN) return;
        if (packet[4] != 0x41) return;
        if (packet[5] != 0x53) return;
        if (packet[6] != 0x43) return;
        if (packet[7] != 0x2d) return;
        if (packet[8] != 0x45) return;
        if (packet[9] != 0x31) return;
        if (packet[10] != 0x2e) return;
        if (packet[11] != 0x31) return;
        if (packet[12] != 0x37) return;

        _type = type;
        _length = len;
        memcpy(_data, packet, len);
    }
    else if (type == xFadeFrame::ID_ARTNETSOCKET)
    {
        // validate the packet
        if (len < ARTNET_PACKET_HEADERLEN) return;
        if (packet[0] != 'A') return;
        if (packet[1] != 'r') return;
        if (packet[2] != 't') return;
        if (packet[3] != '-') return;
        if (packet[4] != 'N') return;
        if (packet[5] != 'e') return;
        if (packet[6] != 't') return;
        if (packet[9] != 0x50) return;

        _type = type;
        _length = len;
        memcpy(_data, packet, len);
    }
}

void PacketData::Send(std::string ip) const
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (_type == xFadeFrame::ID_E131SOCKET)
    {
        wxIPV4address localaddr;
        if (_localIP == "")
        {
            localaddr.AnyAddress();
        }
        else
        {
            localaddr.Hostname(_localIP);
        }

        wxIPV4address remoteaddr;

        wxDatagramSocket datagram(localaddr, wxSOCKET_NOWAIT);

        if (!datagram.IsOk())
        {
            logger_base.error("E131Output: Error opening datagram. Network may not be connected? OK : FALSE");
        }
        else if (datagram.Error() != wxSOCKET_NOERROR)
        {
            logger_base.error("Error creating E131 datagram => %d : %s.", datagram.LastError(), (const char *)DecodeIPError(datagram.LastError()).c_str());
        }
        else
        {
            if (wxString(ip).StartsWith("239.255.") || ip == "MULTICAST")
            {
                // multicast - universe number must be in lower 2 bytes
                wxString ipaddrWithUniv = wxString::Format("%d.%d.%d.%d", 239, 255, (int)UniverseHigh(), (int)UniverseLow());
                remoteaddr.Hostname(ipaddrWithUniv);
            }
            else
            {
                remoteaddr.Hostname(ip.c_str());
            }
            remoteaddr.Service(E131PORT);

            datagram.SendTo(remoteaddr, _data, _length);
        }
    }
    else if (_type == xFadeFrame::ID_ARTNETSOCKET)
    {
        wxIPV4address localaddr;
        wxIPV4address remoteaddr;
        localaddr.AnyAddress();

        wxDatagramSocket datagram(localaddr, wxSOCKET_NOWAIT);

        if (!datagram.IsOk())
        {
            logger_base.error("E131Output: Error opening datagram. Network may not be connected? OK : FALSE");
        }
        else if (datagram.Error() != wxSOCKET_NOERROR)
        {
            logger_base.error("Error creating E131 datagram => %d : %s.", datagram.LastError(), (const char *)DecodeIPError(datagram.LastError()).c_str());
        }
        else
        {
            remoteaddr.Hostname(ip.c_str());
            remoteaddr.Service(ARTNETPORT);

            datagram.SendTo(remoteaddr, _data, _length);
        }
    }
}

int PacketData::GetDataLength() const
{
    if (_type == xFadeFrame::ID_E131SOCKET)
    {
        return _length - E131_PACKET_HEADERLEN;
    }
    else if (_type == xFadeFrame::ID_ARTNETSOCKET)
    {
        return _length - ARTNET_PACKET_HEADERLEN;
    }
    return 0;
}

int PacketData::GetNextSequenceNum(int u)
{
    _sequenceNum[u] = _sequenceNum[u] == 255 ? 0 : _sequenceNum[u] + 1;
    return _sequenceNum[u];
}

void PacketData::InitialiseArtNETHeader()
{
    int channels = GetDataLength();

    _data[0] = 'A';   // ID[8]
    _data[1] = 'r';
    _data[2] = 't';
    _data[3] = '-';
    _data[4] = 'N';
    _data[5] = 'e';
    _data[6] = 't';
    _data[9] = 0x50;
    _data[11] = 0x0E; // Protocol version Low
    _data[14] = (_universe & 0xFF);
    _data[15] = ((_universe & 0xFF00) >> 8);
    _data[16] = 0x02; // we are going to send all 512 bytes
    _data[16] = (wxByte)(channels >> 8);  // Property value count (high)
    _data[17] = (wxByte)(channels & 0xff);  // Property value count (low)}
}

#define XLIGHTS_UUID "c0de0080-c69b-11e0-9572-0800200c9a66"

void PacketData::InitialiseE131Header()
{
    _data[1] = 0x10;   // RLP preamble size (low)
    _data[4] = 0x41;   // ACN Packet Identifier (12 bytes)
    _data[5] = 0x53;
    _data[6] = 0x43;
    _data[7] = 0x2d;
    _data[8] = 0x45;
    _data[9] = 0x31;
    _data[10] = 0x2e;
    _data[11] = 0x31;
    _data[12] = 0x37;
    _data[16] = 0x72;  // RLP Protocol flags and length (high)
    _data[17] = 0x6e;  // 0x26e = 638 - 16
    _data[21] = 0x04;

    // CID/UUID

    wxChar msb, lsb;
    wxString id = XLIGHTS_UUID;
    id.Replace("-", "");
    id.MakeLower();
    if (id.Len() != 32) throw "invalid CID";
    for (int i = 0, j = 22; i < 32; i += 2)
    {
        msb = id.GetChar(i);
        lsb = id.GetChar(i + 1);
        msb -= isdigit(msb) ? 0x30 : 0x57;
        lsb -= isdigit(lsb) ? 0x30 : 0x57;
        _data[j++] = (wxByte)((msb << 4) | lsb);
    }

    _data[38] = 0x72;  // Framing Protocol flags and length (high)
    _data[39] = 0x58;  // 0x258 = 638 - 38
    _data[43] = 0x02;
    // Source Name (64 bytes)
    strcpy((char*)&_data[44], _tag.c_str());
    _data[108] = 100;  // Priority
    _data[113] = _universe >> 8;  // Universe Number (high)
    _data[114] = _universe & 0xff;  // Universe Number (low)
    _data[115] = 0x72;  // DMP Protocol flags and length (high)
    _data[116] = 0x0b;  // 0x20b = 638 - 115
    _data[117] = 0x02;  // DMP Vector (Identifies DMP Set Property Message PDU)
    _data[118] = 0xa1;  // DMP Address Type & Data Type
    _data[122] = 0x01;  // Address Increment (low)
    _data[123] = 0x02;  // Property value count (high)
    _data[124] = 0x01;  // Property value count (low)

    int channels = GetDataLength();
    int i = channels;
    wxByte NumHi = (channels + 1) >> 8;   // Channels (high)
    wxByte NumLo = (channels + 1) & 0xff; // Channels (low)

    _data[123] = NumHi;  // Property value count (high)
    _data[124] = NumLo;  // Property value count (low)

    i = E131_PACKET_LEN - 16 - (512 - channels);
    wxByte hi = i >> 8;   // (high)
    wxByte lo = i & 0xff; // (low)

    _data[16] = hi + 0x70;  // RLP Protocol flags and length (high)
    _data[17] = lo;  // 0x26e = E131_PACKET_LEN - 16

    i = E131_PACKET_LEN - 38 - (512 - channels);
    hi = i >> 8;   // (high)
    lo = i & 0xff; // (low)
    _data[38] = hi + 0x70;  // Framing Protocol flags and length (high)
    _data[39] = lo;  // 0x258 = E131_PACKET_LEN - 38

    i = E131_PACKET_LEN - 115 - (512 - channels);
    hi = i >> 8;   // (high)
    lo = i & 0xff; // (low)
    _data[115] = hi + 0x70;  // DMP Protocol flags and length (high)
    _data[116] = lo;  // 0x20b = E131_PACKET_LEN - 115
}

void PacketData::CopyFrom(PacketData* source, long targetType)
{
    _length = 0;
    _type = targetType;
    _universe = source->_universe;
    _source = source->_source;

    if (source->_type == targetType)
    {
        memcpy(_data, source->_data, sizeof(_data));
        _length = source->_length;

        if (_type == xFadeFrame::ID_E131SOCKET)
        {
            strcpy((char*)&_data[44], _tag.c_str());
            _data[111] = GetNextSequenceNum(_universe);
        }
        else if (_type == xFadeFrame::ID_ARTNETSOCKET)
        {
            // nothing to do
            _data[12] = GetNextSequenceNum(_universe);
        }
    }
    else
    {
        if (_type == xFadeFrame::ID_E131SOCKET)
        {
            // converting from ARTNET
            _length = E131_PACKET_HEADERLEN + source->GetDataLength();
            InitialiseE131Header();
            memcpy(source->GetDataPtr(), GetDataPtr(), GetDataLength());
            strcpy((char*)&_data[44], _tag.c_str());
            _data[111] = GetNextSequenceNum(_universe);
        }
        else if (_type == xFadeFrame::ID_ARTNETSOCKET)
        {
            // converting from E131
            _length = ARTNET_PACKET_HEADERLEN + source->GetDataLength();
            InitialiseArtNETHeader();
            memcpy(source->GetDataPtr(), GetDataPtr(), GetDataLength());
            _data[12] = GetNextSequenceNum(_universe);
        }
    }
}

void xFadeFrame::ValidateWindow()
{
    if (ListView_Universes->GetSelectedItemCount() == 0)
    {
        Button_Edit->Enable(false);
        Button_Delete->Enable(false);
    }
    else
    {
        Button_Edit->Enable(true);
        Button_Delete->Enable(true);
    }
}

void xFadeFrame::CreateE131Listener()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (_e131SocketReceive != nullptr) return;

    //Local address to bind to
    wxIPV4address addr;
    addr.AnyAddress();
    addr.Service(E131PORT);
    //create and bind to the address above
    _e131SocketReceive = new wxDatagramSocket(addr);

    logger_base.debug("E131 listening on %s", (const char*)_localInputIP.c_str());

    for (auto it = _targetIP.begin(); it != _targetIP.end(); ++it)
    {
        struct ip_mreq mreq;
        wxString ip = wxString::Format("239.255.%d.%d", it->first >> 8, it->first & 0xFF);
        logger_base.debug("E131 registering for multicast on %s.", (const char *)ip.c_str());
        mreq.imr_multiaddr.s_addr = inet_addr(ip.c_str());
        mreq.imr_interface.s_addr = inet_addr(_localInputIP.c_str()); // this will only listen on the default interface
        if (!_e131SocketReceive->SetOption(IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char *)&mreq, sizeof(mreq)))
        {
            logger_base.warn("    Error opening E131 multicast listener %s.", (const char *)ip.c_str());
        }
    }

    //enable event handling
    _e131SocketReceive->SetEventHandler(*this, ID_E131SOCKET);
    //Notify us about incomming data
    _e131SocketReceive->SetNotify(wxSOCKET_INPUT_FLAG);
    //enable event handling
    _e131SocketReceive->Notify(true);
}

void xFadeFrame::CreateArtNETListener()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (_artNETSocketReceive != nullptr) return;

    //Local address to bind to
    wxIPV4address addr;
    addr.AnyAddress();
    addr.Service(ARTNETPORT);
    //create and bind to the address above
    _artNETSocketReceive = new wxDatagramSocket(addr);

    logger_base.debug("ARTNet listening on %s", (const char*)_localInputIP.c_str());

    for (auto it = _targetIP.begin(); it != _targetIP.end(); ++it)
    {
                struct ip_mreq mreq;
                wxString ip = wxString::Format("239.255.%d.%d", it->first >> 8, it->first & 0xFF);
                logger_base.debug("ARTNet registering for multicast on %s.", (const char *)ip.c_str());
                mreq.imr_multiaddr.s_addr = inet_addr(ip.c_str());
                mreq.imr_interface.s_addr = inet_addr(_localInputIP.c_str()); // this will only listen on the default interface
                if (!_artNETSocketReceive->SetOption(IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char *)&mreq, sizeof(mreq)))
                {
                    logger_base.warn("    Error opening ARTNet multicast listener %s.", (const char *)ip.c_str());
                }
    }

    //enable event handling
    _artNETSocketReceive->SetEventHandler(*this, ID_ARTNETSOCKET);
    //Notify us about incomming data
    _artNETSocketReceive->SetNotify(wxSOCKET_INPUT_FLAG);
    //enable event handling
    _artNETSocketReceive->Notify(true);
}

void xFadeFrame::LoadUniverses()
{
    int sel = ListView_Universes->GetFirstSelected();

    ListView_Universes->Freeze();

    ListView_Universes->DeleteAllItems();

    int startu = 0;
    int lastu = 0;
    std::string lastip = "";
    std::string lastdesc = "";
    std::string lastprotocol = "";

    auto itd = _targetDesc.begin();
    auto itp = _targetProtocol.begin();
    for (auto it = _targetIP.begin(); it != _targetIP.end(); ++it)
    {
        if (lastu != it->first-1 || lastip != it->second)
        {
            if (lastu != 0)
            {
                ListView_Universes->InsertItem(ListView_Universes->GetItemCount(), wxString::Format("%d", startu));
                ListView_Universes->SetItem(ListView_Universes->GetItemCount() - 1, 1, wxString::Format("%d", lastu));
                ListView_Universes->SetItem(ListView_Universes->GetItemCount() - 1, 2, lastip);
                ListView_Universes->SetItem(ListView_Universes->GetItemCount() - 1, 3, lastdesc);
                ListView_Universes->SetItem(ListView_Universes->GetItemCount() - 1, 4, lastprotocol);

                startu = it->first;
                lastu = it->first;
                lastip = it->second;
                lastdesc = itd->second;
                lastprotocol = itp->second;
            }
            else
            {
                startu = it->first;
                lastu = it->first;
                lastip = it->second;
                lastdesc = itd->second;
                lastprotocol = itp->second;
            }
        }
        else
        {
            lastu = it->first;
        }

        ++itd;
        ++itp;
    }

    if (startu != 0)
    {
        ListView_Universes->InsertItem(ListView_Universes->GetItemCount(), wxString::Format("%d", startu));
        ListView_Universes->SetItem(ListView_Universes->GetItemCount() - 1, 1, wxString::Format("%d", lastu));
        ListView_Universes->SetItem(ListView_Universes->GetItemCount() - 1, 2, lastip);
        ListView_Universes->SetItem(ListView_Universes->GetItemCount() - 1, 3, lastdesc);
        ListView_Universes->SetItem(ListView_Universes->GetItemCount() - 1, 4, lastprotocol);
    }

    ListView_Universes->Select(sel);

    ListView_Universes->Thaw();
}

void xFadeFrame::SetMIDIForControl(wxString controlName, float parm)
{
    SaveState();
}

void xFadeFrame::AddUniverseRange(int low, int high, std::string ipAddress, std::string desc, std::string protocol)
{
    for (int i = low; i <= high; i++)
    {
        _targetIP[i] = ipAddress;
        _targetDesc[i] = desc;
        _targetProtocol[i] = protocol;
    }

    LoadUniverses();

    // Need to redo sockets in case we have multicast
    RestartInterfaces();
    ValidateWindow();
}

void xFadeFrame::OnResize(wxSizeEvent& event)
{
    Layout();
}

void xFadeFrame::CreateDebugReport(wxDebugReportCompress *report) {
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

void xFadeFrame::SendReport(const wxString &loc, wxDebugReportCompress &report) {
    wxHTTP http;
    http.Connect("dankulp.com");

    const char *bound = "--------------------------b29a7c2fe47b9481";

    wxDateTime now = wxDateTime::Now();
    int millis = wxGetUTCTimeMillis().GetLo() % 1000;
    wxString ts = wxString::Format("%04d-%02d-%02d_%02d-%02d-%02d-%03d", now.GetYear(), now.GetMonth()+1, now.GetDay(), now.GetHour(), now.GetMinute(), now.GetSecond(), millis);

    wxString fn = wxString::Format("xFade-%s_%s_%s_%s.zip", wxPlatformInfo::Get().GetOperatingSystemFamilyName().c_str(), xlights_version_string, GetBitness(), ts);
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

void xFadeFrame::OnKeyDown(wxKeyEvent& event)
{
    ValidateWindow();
}

void xFadeFrame::OnListView_UniversesItemSelect(wxListEvent& event)
{
    ValidateWindow();
}

void xFadeFrame::OnListView_UniversesItemActivated(wxListEvent& event)
{
    if (ListView_Universes->GetSelectedItemCount() > 0)
    {
        int start = wxAtoi(ListView_Universes->GetItemText(ListView_Universes->GetFirstSelected()));
        int end = wxAtoi(ListView_Universes->GetItemText(ListView_Universes->GetFirstSelected(), 1));
        UniverseEntryDialog dlg(this, start, end, ListView_Universes->GetItemText(ListView_Universes->GetFirstSelected(), 2), ListView_Universes->GetItemText(ListView_Universes->GetFirstSelected(), 3), ListView_Universes->GetItemText(ListView_Universes->GetFirstSelected(), 4));
        if (dlg.ShowModal() == wxID_OK)
        {
            AddUniverseRange(dlg.SpinCtrl_Start->GetValue(), dlg.SpinCtrl_End->GetValue(), dlg.TextCtrl_IPAddress->GetValue().ToStdString(), dlg.TextCtrl_Description->GetValue().ToStdString(), dlg.Choice_Protocol->GetStringSelection().ToStdString());
            SaveState();
        }
    }
    ValidateWindow();
}

void xFadeFrame::OnCheckBox_E131Click(wxCommandEvent& event)
{
    CloseSockets();
    if (CheckBox_E131->GetValue())
    {
        CreateE131Listener();
    }
    SaveState();
    ValidateWindow();
}

void xFadeFrame::OnCheckBox_ArtNETClick(wxCommandEvent& event)
{
    CloseSockets();
    if (CheckBox_ArtNET->GetValue())
    {
        CreateArtNETListener();
    }
    SaveState();
    ValidateWindow();
}

void xFadeFrame::OnE131SocketEvent(wxSocketEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxIPV4address addr;
    addr.Service(E131PORT);
    wxByte buf[E131_PACKET_LEN];
    switch (event.GetSocketEvent())
    {
    case wxSOCKET_INPUT:
    {
        _e131SocketReceive->Notify(false);
        size_t n = _e131SocketReceive->RecvFrom(addr, buf, sizeof(buf)).LastCount();
        if (!n) {
            logger_base.error("ERROR: failed to receive E131 data");
            return;
        }
        //logger_base.debug("E131 packet received.");
        StashPacket(ID_E131SOCKET, buf, n);
        _e131SocketReceive->Notify(true);
    }
        break;
    default:
        logger_base.warn("OnE131SocketEvent: Unexpected event !");
        break;
    }
}

void xFadeFrame::OnArtNETSocketEvent(wxSocketEvent & event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxIPV4address addr;
    addr.Service(ARTNETPORT);
    wxByte buf[ARTNET_PACKET_LEN];
    switch (event.GetSocketEvent())
    {
    case wxSOCKET_INPUT:
    {
        _artNETSocketReceive->Notify(false);
        size_t n = _artNETSocketReceive->RecvFrom(addr, buf, sizeof(buf)).LastCount();
        if (!n) {
            logger_base.error("ERROR: failed to receive ArtNET data");
            return;
        }
        //logger_base.debug("ArtNet packet received.");
        StashPacket(ID_ARTNETSOCKET, buf, n);
        _artNETSocketReceive->Notify(true);
    }
        break;
    default:
        logger_base.warn("OnArtNETSocketEvent: Unexpected event !");
        break;
    }
}

void xFadeFrame::OnButton_AddClick(wxCommandEvent& event)
{
    UniverseEntryDialog dlg(this, -1, -1, "", "", "As per input");
    if (dlg.ShowModal() == wxID_OK)
    {
        AddUniverseRange(dlg.SpinCtrl_Start->GetValue(), dlg.SpinCtrl_End->GetValue(), dlg.TextCtrl_IPAddress->GetValue().ToStdString(), dlg.TextCtrl_Description->GetValue().ToStdString(), dlg.Choice_Protocol->GetStringSelection().ToStdString());
        SaveState();
    }
}

void xFadeFrame::OnButton_EditClick(wxCommandEvent& event)
{
    if (ListView_Universes->GetSelectedItemCount() > 0)
    {
        int start = wxAtoi(ListView_Universes->GetItemText(ListView_Universes->GetFirstSelected()));
        int end = wxAtoi(ListView_Universes->GetItemText(ListView_Universes->GetFirstSelected(), 1));
        UniverseEntryDialog dlg(this, start, end, ListView_Universes->GetItemText(ListView_Universes->GetFirstSelected(), 2), ListView_Universes->GetItemText(ListView_Universes->GetFirstSelected(), 3), ListView_Universes->GetItemText(ListView_Universes->GetFirstSelected(), 4));
        if (dlg.ShowModal() == wxID_OK)
        {
            AddUniverseRange(dlg.SpinCtrl_Start->GetValue(), dlg.SpinCtrl_End->GetValue(), dlg.TextCtrl_IPAddress->GetValue().ToStdString(), dlg.TextCtrl_Description->GetValue().ToStdString(), dlg.Choice_Protocol->GetStringSelection().ToStdString());
            SaveState();
        }
    }
}

void xFadeFrame::OnButton_DeleteClick(wxCommandEvent& event)
{
    if (ListView_Universes->GetSelectedItemCount() > 0)
    {
        int start = wxAtoi(ListView_Universes->GetItemText(ListView_Universes->GetFirstSelected()));
        int end = wxAtoi(ListView_Universes->GetItemText(ListView_Universes->GetFirstSelected(), 1));

        for (int i = start; i <= end; i++)
        {
            _targetIP.erase(i);
            _targetDesc.erase(i);
            _targetProtocol.erase(i);
        }
        LoadUniverses();
        SaveState();
    }
}

void xFadeFrame::OnUITimerTrigger(wxTimerEvent& event)
{
    // this is where i need to move the fade slider
    float fadeTime = wxAtof(TextCtrl_CrossFadeTime->GetValue());
    if (fadeTime == 0.0) fadeTime = 0.025;
    float per = 10000 / (fadeTime * 40);
    if (per < 1) per = 1;
    if (_direction < 0)
    {
        Slider1->SetValue(Slider1->GetValue() - per);
        if (Slider1->GetValue() == 0)
        {
            UITimer.Stop();
        }
    }
    else if (_direction > 0)
    {
        Slider1->SetValue(Slider1->GetValue() + per);
        if (Slider1->GetValue() == 10000)
        {
            UITimer.Stop();
        }
    }
    else
    {
        if (Slider1->GetValue() < 5000)
        {
            Slider1->SetValue(Slider1->GetValue() + per);
            if (Slider1->GetValue() >= 5000)
            {
                Slider1->SetValue(5000);
                UITimer.Stop();
            }
        }
        else
        {
            Slider1->SetValue(Slider1->GetValue() - per);
            if (Slider1->GetValue() <= 5000)
            {
                Slider1->SetValue(5000);
                UITimer.Stop();
            }
        }
    }

    SetFade();
}

std::string xFadeFrame::ExtractE131Tag(wxByte* packet)
{
    std::string res((char*)&packet[44]);
    res = res.substr(0, std::min(64, (int)res.size()));
    return res;
}

void xFadeFrame::SetFade()
{
    if (_emitter != nullptr)
    {
        _emitter->SetPos(Slider1->GetValue() / 10000.0);
    }
}

void xFadeFrame::SetTiming()
{
    if (_emitter != nullptr)
    {
        wxString frm = Choice_FrameTiming->GetStringSelection();
        if (frm == "25ms")
        {
            _emitter->SetFrameMS(25);
        }
        else if (frm == "30ms")
        {
            _emitter->SetFrameMS(30);
        }
        else if (frm == "100ms")
        {
            _emitter->SetFrameMS(100);
        }
        else
        {
            _emitter->SetFrameMS(50);
        }
    }
}

void xFadeFrame::RestartInterfaces()
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

void xFadeFrame::OnListView_UniversesItemDeselect(wxListEvent& event)
{
}

void xFadeFrame::OnButtonClickLeft(wxCommandEvent& event)
{
    int button = wxAtoi(((wxButton*)event.GetEventObject())->GetLabel());

    wxString result = xLightsRequest(1, "PLAY_JUKEBOX_BUTTON " + wxString::Format("%d", button));
}

void xFadeFrame::OnButtonRClickLeft(wxContextMenuEvent& event)
{
    int button = wxAtoi(((wxButton*)event.GetEventObject())->GetLabel());
    SetMIDIForControl(((wxWindow*)event.GetEventObject())->GetName(), button);
}

void xFadeFrame::OnButtonClickRight(wxCommandEvent& event)
{
    int button = wxAtoi(((wxButton*)event.GetEventObject())->GetLabel());

    wxString result = xLightsRequest(2, "PLAY_JUKEBOX_BUTTON " + wxString::Format("%d", button));
}

void xFadeFrame::OnButtonRClickRight(wxContextMenuEvent& event)
{
    int button = wxAtoi(((wxButton*)event.GetEventObject())->GetLabel());
    SetMIDIForControl(((wxWindow*)event.GetEventObject())->GetName(), button);
}

void xFadeFrame::OnTextCtrlRClickCrossFadeTime(wxContextMenuEvent& event)
{
    SetMIDIForControl(((wxWindow*)event.GetEventObject())->GetName());
}

void xFadeFrame::OnButtonRClickFadeLeft(wxContextMenuEvent& event)
{
    SetMIDIForControl(((wxWindow*)event.GetEventObject())->GetName());
}

void xFadeFrame::OnButtonRClickFadeMiddle(wxContextMenuEvent& event)
{
    SetMIDIForControl(((wxWindow*)event.GetEventObject())->GetName());
}

void xFadeFrame::OnButtonRClickFadeRight(wxContextMenuEvent& event)
{
    SetMIDIForControl(((wxWindow*)event.GetEventObject())->GetName());
}

void xFadeFrame::OnChoice_FrameTimingSelect(wxCommandEvent& event)
{
    SetTiming();
    SaveState();
}

void xFadeFrame::OnButton_ForceInputClick(wxCommandEvent& event)
{
    IPEntryDialog dlg(this);
    dlg.TextCtrl_IPAddress->SetValue(_localInputIP);

    if (dlg.ShowModal() == wxID_OK)
    {
        if (dlg.TextCtrl_IPAddress->GetValue() == "")
        {
            _localInputIP = _defaultIP;
        }
        else
        {
            _localInputIP = dlg.TextCtrl_IPAddress->GetValue();
        }
        StaticText_InputIP->SetLabel(_localInputIP);
        RestartInterfaces();
        SaveState();
    }
}

void xFadeFrame::OnButton_ForceOutputClick(wxCommandEvent& event)
{
    IPEntryDialog dlg(this);
    dlg.TextCtrl_IPAddress->SetValue(_localOutputIP);

    if (dlg.ShowModal() == wxID_OK)
    {
        if (dlg.TextCtrl_IPAddress->GetValue() == "")
        {
            _localOutputIP = _defaultIP;
        }
        else
        {
            _localOutputIP = dlg.TextCtrl_IPAddress->GetValue();
        }
        StaticText_OutputIP->SetLabel(_localOutputIP);
        RestartInterfaces();
        if (_emitter != nullptr)
        {
            _emitter->SetLocalIP(_localOutputIP);
            _emitter->Restart();
        }
        else
        {
            _emitter = new Emitter(&_targetIP, &_leftData, &_rightData, &_targetProtocol, &_lock, _localOutputIP);
        }
        SaveState();
    }
}

void xFadeFrame::OnButton_MiddleClick(wxCommandEvent& event)
{
    _direction = 0;
    UITimer.Start(25);
}

void xFadeFrame::OnButton_LeftClick(wxCommandEvent& event)
{
    _direction = -1;
    UITimer.Start(25);
}

void xFadeFrame::OnButton_RightClick(wxCommandEvent& event)
{
    _direction = 1;
    UITimer.Start(25);
}

void xFadeFrame::OnButtonClickFT(wxCommandEvent& event)
{
    TextCtrl_CrossFadeTime->SetValue(((wxButton*)event.GetEventObject())->GetLabel());
}

void xFadeFrame::OnButtonRClickFT(wxContextMenuEvent& event)
{
    float time = wxAtof(((wxButton*)event.GetEventObject())->GetLabel());
    SetMIDIForControl(((wxWindow*)event.GetEventObject())->GetName(), time);
}

void xFadeFrame::OnButton_ConnectToxLightsClick(wxCommandEvent& event)
{
    _leftTag = "";
    _rightTag = "";
    TextCtrl_LeftSequence->SetValue("");
    TextCtrl_RightSequence->SetValue("");
    TextCtrl_LeftTag->SetValue("");
    TextCtrl_LeftTag->SetValue("");

    wxString result = xLightsRequest(1, "GET_JUKEBOX_BUTTON_TOOLTIPS");
    if (result.StartsWith("SUCCESS"))
    {
        Panel_Left->Enable(true);
        auto tips = wxSplit(result, '|');

        int i = 0;
        for (auto it = tips.begin(); it != tips.end(); ++it)
        {
            if (i > 0)
            {
                wxString s = wxString::Format("%d", i);
                auto buttons = Panel_Left->GetChildren();
                for (auto b = buttons.begin(); b != buttons.end(); ++b)
                {
                    if ((*b)->GetLabel() == s)
                    {
                        if (*it == "")
                        {
                            (*b)->UnsetToolTip();
                        }
                        else
                        {
                            (*b)->SetToolTip(*it);
                        }
                    }
                }
            }
            i++;
        }

        result = xLightsRequest(1, "GET_JUKEBOX_BUTTON_EFFECTPRESENT");
        if (result.StartsWith("SUCCESS"))
        {
            Panel_Right->Enable(true);

            auto bs = wxSplit(result, '|');

            i = 0;
            for (auto it = bs.begin(); it != bs.end(); ++it)
            {
                if (i > 0)
                {
                    wxString s = wxString::Format("%d", i);
                    auto buttons = Panel_Left->GetChildren();
                    for (auto b = buttons.begin(); b != buttons.end(); ++b)
                    {
                        if ((*b)->GetLabel() == s)
                        {
                            if (*it == "")
                            {
                                (*b)->SetBackgroundColour(wxColour(255, 108, 108));
                            }
                            else
                            {
                                (*b)->SetBackgroundColour(_defaultColour);
                            }
                        }
                    }
                }
                i++;
            }
        }

        result = xLightsRequest(1, "GET_E131_TAG");
        if (result.StartsWith("SUCCESS "))
        {
            _leftTag = result.substr(sizeof("SUCCESS ") - 1);
        }
        else
        {
            _leftTag = "";
        }
        TextCtrl_LeftTag->SetValue(_leftTag);

        result = xLightsRequest(1, "GET_SEQUENCE_NAME");
        if (result.StartsWith("SUCCESS "))
        {
            TextCtrl_LeftSequence->SetValue(result.substr(sizeof("SUCCESS ") - 1));
        }
        else
        {
            TextCtrl_LeftSequence->SetValue("No sequence loaded.");
        }

        result = xLightsRequest(1, "TURN_LIGHTS_ON");
    }
    else
    {
        Panel_Left->Enable(false);
    }

    result = xLightsRequest(2, "GET_JUKEBOX_BUTTON_TOOLTIPS");
    if (result.StartsWith("SUCCESS"))
    {
        auto tips = wxSplit(result, '|');

        int i = 0;
        for (auto it = tips.begin(); it != tips.end(); ++it)
        {
            if (i > 0)
            {
                wxString s = wxString::Format("%d", i);
                auto buttons = Panel_Right->GetChildren();
                for (auto b = buttons.begin(); b != buttons.end(); ++b)
                {
                    if ((*b)->GetLabel() == s)
                    {
                        if (*it == "")
                        {
                            (*b)->UnsetToolTip();
                        }
                        else
                        {
                            (*b)->SetToolTip(*it);
                        }
                    }
                }
            }
            i++;
        }

        result = xLightsRequest(2, "GET_JUKEBOX_BUTTON_EFFECTPRESENT");
        if (result.StartsWith("SUCCESS"))
        {
            auto bs = wxSplit(result, '|');

            i = 0;
            for (auto it = bs.begin(); it != bs.end(); ++it)
            {
                if (i > 0)
                {
                    wxString s = wxString::Format("%d", i);
                    auto buttons = Panel_Right->GetChildren();
                    for (auto b = buttons.begin(); b != buttons.end(); ++b)
                    {
                        if ((*b)->GetLabel() == s)
                        {
                            if (*it == "")
                            {
                                (*b)->SetBackgroundColour(wxColour(255, 108, 108));
                            }
                            else
                            {
                                (*b)->SetBackgroundColour(_defaultColour);
                            }
                        }
                    }
                }
                i++;
            }
        }

        result = xLightsRequest(2, "GET_E131_TAG");
        if (result.StartsWith("SUCCESS "))
        {
            _rightTag = result.substr(sizeof("SUCCESS ") - 1);
        }
        else
        {
            _rightTag = "";
        }
        TextCtrl_RightTag->SetValue(_rightTag);

        result = xLightsRequest(2, "GET_SEQUENCE_NAME");
        if (result.StartsWith("SUCCESS "))
        {
            TextCtrl_RightSequence->SetValue(result.substr(sizeof("SUCCESS ") - 1));
        }
        else
        {
            TextCtrl_RightSequence->SetValue("No sequence loaded.");
        }

        result = xLightsRequest(2, "TURN_LIGHTS_ON");
    }
    else
    {
        Panel_Right->Enable(false);
    }
}

void xFadeFrame::OnSlider1CmdSliderUpdated(wxScrollEvent& event)
{
    SetFade();
}
