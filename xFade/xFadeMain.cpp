/***************************************************************
 * Name:      xFadeMain.cpp
 * Purpose:   Code for Application Frame
 * Author:    xLights ()
 * Created:   2016-12-30
 * Copyright: xLights (http://xlights.org)
 * License:
 **************************************************************/

 //(*InternalHeaders(xFadeFrame)
 #include <wx/intl.h>
 #include <wx/string.h>
 //*)

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
#include "MIDIListener.h"
#include "SettingsDialog.h"
#include "MIDIAssociateDialog.h"

#ifndef __WXMSW__
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include "../include/xLights.xpm"
#include "../include/xLights-16.xpm"
#include "../include/xLights-32.xpm"
#include "../include/xLights-64.xpm"
#include "../include/xLights-128.xpm"

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
const long xFadeFrame::ID_BUTTON_CONNECT = wxNewId();
const long xFadeFrame::ID_BUTTON3 = wxNewId();
const long xFadeFrame::ID_STATICLINE1 = wxNewId();
const long xFadeFrame::ID_STATICTEXT5 = wxNewId();
const long xFadeFrame::ID_SLIDER_LeftBrightness = wxNewId();
const long xFadeFrame::ID_STATICTEXT7 = wxNewId();
const long xFadeFrame::ID_SLIDER_RightBrightness = wxNewId();
const long xFadeFrame::ID_STATICTEXT8 = wxNewId();
const long xFadeFrame::ID_SLIDER_MasterBrightness = wxNewId();
const long xFadeFrame::ID_STATICLINE2 = wxNewId();
const long xFadeFrame::ID_STATICTEXT1 = wxNewId();
const long xFadeFrame::ID_TEXTCTRL_TIME = wxNewId();
const long xFadeFrame::ID_STATICTEXT2 = wxNewId();
const long xFadeFrame::ID_STATICTEXT3 = wxNewId();
const long xFadeFrame::ID_PANEL3 = wxNewId();
const long xFadeFrame::ID_BUTTON_MIDDLE = wxNewId();
const long xFadeFrame::ID_BUTTON_LEFT = wxNewId();
const long xFadeFrame::ID_SLIDER_FADE = wxNewId();
const long xFadeFrame::ID_BUTTON_RIGHT = wxNewId();
const long xFadeFrame::ID_BUTTON_ADVANCE = wxNewId();
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
const long xFadeFrame::ID_LED1 = wxNewId();
const long xFadeFrame::ID_LED2 = wxNewId();

BEGIN_EVENT_TABLE(xFadeFrame,wxFrame)
    //(*EventTable(xFadeFrame)
    //*)
    EVT_COMMAND(wxID_ANY, EVT_MIDI, xFadeFrame::OnMIDIEvent)
END_EVENT_TABLE()

void xFadeFrame::StashPacket(long type, wxByte* packet, int len)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    bool left = IsLeft(type ,packet, len);
    bool right = IsRight(type, packet, len);
    int universe = -1;

    if (type == ID_E131SOCKET)
    {
        universe = ((int)packet[113] << 8) + (int)packet[114];
    }
    else if (type == ID_ARTNETSOCKET)
    {
        universe = ((int)packet[15] << 8) + (int)packet[14];
    }

    if (universe == -1) return;

    if (!IsUniverseToBeCaptured(universe)) return;

    {
        std::unique_lock<std::mutex> mutLock(_lock);

        if (left)
        {
            if (!_leftData[universe].Update(type, packet, len))
            {
                logger_base.debug("Invalid packet.");
            }
            else
            {
                if (_leftData[universe].GetSequenceNum() % 10 == 0)
                {
                    Led_Left->Enable(!Led_Left->IsEnabled());
                }
            }
        }
        else if (right)
        {
            if (!_rightData[universe].Update(type, packet, len))
            {
                logger_base.debug("Invalid packet.");
            }
            else
            {
                if (_rightData[universe].GetSequenceNum() % 10 == 0)
                {
                    Led_Right->Enable(!Led_Right->IsEnabled());
                }
            }
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
    return _settings._targetIP.find(universe) != _settings._targetIP.end();
}

xFadeFrame::xFadeFrame(wxWindow* parent, wxWindowID id)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    _e131SocketReceive = nullptr;
    _artNETSocketReceive = nullptr;
    _emitter = nullptr;
    _selectedButtonFont = new wxFont(14, wxFONTFAMILY_SWISS, wxFontStyle::wxFONTSTYLE_NORMAL, wxFontWeight::wxFONTWEIGHT_BOLD);

    //(*Initialize(xFadeFrame)
    wxBoxSizer* BoxSizer1;
    wxFlexGridSizer* FlexGridSizer10;
    wxFlexGridSizer* FlexGridSizer11;
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer4;
    wxFlexGridSizer* FlexGridSizer5;
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
    Button_ConnectToxLights = new wxButton(this, ID_BUTTON_CONNECT, _("Connect To xLights"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CONNECT"));
    FlexGridSizer2->Add(Button_ConnectToxLights, 1, wxALL|wxEXPAND, 5);
    Button_Configure = new wxButton(this, ID_BUTTON3, _("Configure"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    FlexGridSizer2->Add(Button_Configure, 1, wxALL|wxEXPAND, 5);
    StaticLine1 = new wxStaticLine(this, ID_STATICLINE1, wxDefaultPosition, wxSize(10,-1), wxLI_HORIZONTAL, _T("ID_STATICLINE1"));
    FlexGridSizer2->Add(StaticLine1, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer5->AddGrowableCol(1);
    StaticText6 = new wxStaticText(this, ID_STATICTEXT5, _("Left brightness:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
    FlexGridSizer5->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_LeftBrightness = new wxSlider(this, ID_SLIDER_LeftBrightness, 100, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_LeftBrightness"));
    FlexGridSizer5->Add(Slider_LeftBrightness, 1, wxALL|wxEXPAND, 5);
    StaticText7 = new wxStaticText(this, ID_STATICTEXT7, _("Right brightness:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
    FlexGridSizer5->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_RightBrightness = new wxSlider(this, ID_SLIDER_RightBrightness, 100, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_RightBrightness"));
    FlexGridSizer5->Add(Slider_RightBrightness, 1, wxALL|wxEXPAND, 5);
    StaticText10 = new wxStaticText(this, ID_STATICTEXT8, _("Master brightness:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
    FlexGridSizer5->Add(StaticText10, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_MasterBrightness = new wxSlider(this, ID_SLIDER_MasterBrightness, 100, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_MasterBrightness"));
    FlexGridSizer5->Add(Slider_MasterBrightness, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer2->Add(FlexGridSizer5, 1, wxALL|wxEXPAND, 5);
    StaticLine2 = new wxStaticLine(this, ID_STATICLINE2, wxDefaultPosition, wxSize(10,-1), wxLI_HORIZONTAL, _T("ID_STATICLINE2"));
    FlexGridSizer2->Add(StaticLine2, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer8 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer8->AddGrowableCol(1);
    StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Cross Fade Time:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer8->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_CrossFadeTime = new wxTextCtrl(this, ID_TEXTCTRL_TIME, _("1.00"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, wxDefaultValidator, _T("ID_TEXTCTRL_TIME"));
    FlexGridSizer8->Add(TextCtrl_CrossFadeTime, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer2->Add(FlexGridSizer8, 1, wxALL|wxEXPAND, 5);
    Panel_FadeTime = new wxPanel(this, ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL3"));
    Panel_FadeTime->SetMinSize(wxSize(-1,20));
    FlexGridSizer4 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer4->AddGrowableCol(1);
    StaticText2 = new wxStaticText(Panel_FadeTime, ID_STATICTEXT2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer4->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    GridSizer_TimePresets = new wxGridSizer(0, 5, 0, 0);
    FlexGridSizer4->Add(GridSizer_TimePresets, 0, wxEXPAND, 0);
    StaticText5 = new wxStaticText(Panel_FadeTime, ID_STATICTEXT3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer4->Add(StaticText5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel_FadeTime->SetSizer(FlexGridSizer4);
    FlexGridSizer4->Fit(Panel_FadeTime);
    FlexGridSizer4->SetSizeHints(Panel_FadeTime);
    FlexGridSizer2->Add(Panel_FadeTime, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer3->AddGrowableCol(1);
    Button_Middle = new wxButton(this, ID_BUTTON_MIDDLE, _("v"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_MIDDLE"));
    FlexGridSizer3->Add(Button_Middle, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Left = new wxButton(this, ID_BUTTON_LEFT, _("<"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_LEFT"));
    FlexGridSizer3->Add(Button_Left, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Slider1 = new wxSlider(this, ID_SLIDER_FADE, 0, 0, 10000, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_FADE"));
    FlexGridSizer3->Add(Slider1, 1, wxALL|wxEXPAND, 5);
    Button_Right = new wxButton(this, ID_BUTTON_RIGHT, _(">"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_RIGHT"));
    FlexGridSizer3->Add(Button_Right, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer3->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Advance = new wxButton(this, ID_BUTTON_ADVANCE, _("Advance"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_ADVANCE"));
    FlexGridSizer3->Add(Button_Advance, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer3->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 5);
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

    Connect(ID_BUTTON_CONNECT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xFadeFrame::OnButton_ConnectToxLightsClick);
    Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xFadeFrame::OnButton_ConfigureClick);
    Connect(ID_SLIDER_LeftBrightness,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&xFadeFrame::OnSlider_LeftBrightnessCmdSliderUpdated);
    Connect(ID_SLIDER_RightBrightness,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&xFadeFrame::OnSlider_RightBrightnessCmdSliderUpdated);
    Connect(ID_SLIDER_MasterBrightness,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&xFadeFrame::OnSlider_MasterBrightnessCmdSliderUpdated);
    Connect(ID_TEXTCTRL_TIME,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&xFadeFrame::OnTextCtrl_CrossFadeTimeText);
    Connect(ID_BUTTON_MIDDLE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xFadeFrame::OnButton_MiddleClick);
    Connect(ID_BUTTON_LEFT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xFadeFrame::OnButton_LeftClick);
    Connect(ID_SLIDER_FADE,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&xFadeFrame::OnSlider1CmdSliderUpdated);
    Connect(ID_BUTTON_RIGHT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xFadeFrame::OnButton_RightClick);
    Connect(ID_BUTTON_ADVANCE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&xFadeFrame::OnButton_AdvanceClick);
    Connect(ID_TIMER1,wxEVT_TIMER,(wxObjectEventFunction)&xFadeFrame::OnUITimerTrigger);
    //*)

    Led_Left = new wxLed(this, ID_LED1, "808080", wxDefaultPosition, wxDefaultSize);
    Led_Left->Disable();
    Led_Left->SetColor("00FF00");
    FlexGridSizer3->Insert(0, Led_Left, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    Led_Right = new wxLed(this, ID_LED2, "808080", wxDefaultPosition, wxDefaultSize);
    Led_Right->SetColor("00FF00");
    Led_Right->Disable();
    FlexGridSizer3->Insert(2, Led_Right, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);

    Connect(ID_E131SOCKET, wxEVT_SOCKET, (wxObjectEventFunction)&xFadeFrame::OnE131SocketEvent);
    Connect(ID_ARTNETSOCKET, wxEVT_SOCKET, (wxObjectEventFunction)&xFadeFrame::OnArtNETSocketEvent);

    Connect(Button_Advance->GetId(), wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&xFadeFrame::OnButtonRClickAdvance);
    Connect(Button_Left->GetId(), wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&xFadeFrame::OnButtonRClickFadeLeft);
    Connect(Button_Middle->GetId(), wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&xFadeFrame::OnButtonRClickFadeMiddle);
    Connect(Button_Right->GetId(), wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&xFadeFrame::OnButtonRClickFadeRight);
    Connect(TextCtrl_CrossFadeTime->GetId(), wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&xFadeFrame::OnTextCtrlRClickCrossFadeTime);
    Connect(Slider1->GetId(), wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&xFadeFrame::OnSliderRClickFade);
    Connect(Slider_LeftBrightness->GetId(), wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&xFadeFrame::OnSliderRClickLeftBrightness);
    Connect(Slider_RightBrightness->GetId(), wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&xFadeFrame::OnSliderRClickRightBrightness);
    Connect(Slider_MasterBrightness->GetId(), wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&xFadeFrame::OnSliderRClickMasterBrightness);

    SetTitle("xLights Fade " + xlights_version_string + " " + GetBitness());

    wxIconBundle icons;
    icons.AddIcon(wxIcon(xlights_16_xpm));
    icons.AddIcon(wxIcon(xlights_32_xpm));
    icons.AddIcon(wxIcon(xlights_64_xpm));
    icons.AddIcon(wxIcon(xlights_128_xpm));
    icons.AddIcon(wxIcon(xlights_xpm));
    SetIcons(icons);

    LoadState();

    _emitter = new Emitter(&_settings._targetIP, &_leftData, &_rightData, &_settings._targetProtocol, &_lock, _settings._localOutputIP);

    for (int i = 0; i < JUKEBOXBUTTONS; i++)
    {
        wxButton* button = new wxButton(Panel_Left, wxID_ANY, wxString::Format("%d", i + 1), wxDefaultPosition, wxSize(BUTTONWIDTH, BUTTONHEIGHT),
            0, wxDefaultValidator, _T("ID_BUTTON_JBL") + wxString::Format("%d", i + 1));
        button->SetMinSize(wxSize(BUTTONWIDTH, BUTTONHEIGHT));
        button->SetMaxSize(wxSize(BUTTONWIDTH, BUTTONHEIGHT));
        GridSizer_LeftJukeBox->Add(button, 1, wxALL | wxEXPAND);
        Connect(button->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xFadeFrame::OnButtonClickLeft);
        Connect(button->GetId(), wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&xFadeFrame::OnButtonRClickLeft);

        button = new wxButton(Panel_Right, wxID_ANY, wxString::Format("%d", i + 1), wxDefaultPosition, wxSize(BUTTONWIDTH, BUTTONHEIGHT),
            0, wxDefaultValidator, _T("ID_BUTTON_JBR") + wxString::Format("%d", i + 1));
        button->SetMinSize(wxSize(BUTTONWIDTH, BUTTONHEIGHT));
        button->SetMaxSize(wxSize(BUTTONWIDTH, BUTTONHEIGHT));
        GridSizer_RightJukebox->Add(button, 1, wxALL | wxEXPAND);
        Connect(button->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&xFadeFrame::OnButtonClickRight);
        Connect(button->GetId(), wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&xFadeFrame::OnButtonRClickRight);
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

    if (_settings._ArtNET) CreateArtNETListener();
    if (_settings._E131) CreateE131Listener();

    TextCtrl_LeftSequence->SetValue("");
    TextCtrl_RightSequence->SetValue("");
    TextCtrl_LeftTag->SetValue("");
    TextCtrl_RightTag->SetValue("");

    _midiListener = new MIDIListener(_settings.GetMIDIDeviceId(), this);

    ValidateWindow();
}

void xFadeFrame::ValidateWindow()
{
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

    wxString settings = config->Read(_("xfSettings"), "");
    _settings.Load(settings);
}

xFadeFrame::~xFadeFrame()
{
    if (_midiListener != nullptr)
    {
        _midiListener->Stop();
        //delete _midiListener;
    }

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

    if (_selectedButtonFont != nullptr)
    {
        delete _selectedButtonFont;
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

    config->Write(_("xfSettings"), wxString(_settings.Serialise()));
    config->Flush();
}

// close not required sockets
void xFadeFrame::CloseSockets(bool force)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (force || !_settings._E131)
    {
        if (_e131SocketReceive != nullptr)
        {
            logger_base.debug("Closing E131 receive socket.");
            _e131SocketReceive->Notify(false);
            wxYield(); // let any pending events process
            _e131SocketReceive->Close();
            delete _e131SocketReceive;
            _e131SocketReceive = nullptr;
        }
    }

    if (force || !_settings._ArtNET)
    {
        if (_artNETSocketReceive != nullptr)
        {
            logger_base.debug("Closing ArtNET receive socket.");
            _artNETSocketReceive->Notify(false);
            wxYield(); // let any pending events process
            _artNETSocketReceive->Close();
            delete _artNETSocketReceive;
            _artNETSocketReceive = nullptr;
        }
    }
}

void xFadeFrame::AddFadeTimeButton(std::string label)
{
    wxString l = label;
    l.Replace(".", "_");
    wxButton* button = new wxButton(Panel_FadeTime, wxID_ANY, label, wxDefaultPosition, wxSize(2 * BUTTONWIDTH, BUTTONHEIGHT),
        0, wxDefaultValidator, _T("ID_BUTTON_FBT") + l);
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
    memset(_data, 0x00, sizeof(_data));
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

bool PacketData::Update(long type, wxByte* packet, int len)
{
    if (type == xFadeFrame::ID_E131SOCKET)
    {
        // validate the packet
        if (len < E131_PACKET_HEADERLEN) return false;
        if (packet[4] != 0x41) return false;
        if (packet[5] != 0x53) return false;
        if (packet[6] != 0x43) return false;
        if (packet[7] != 0x2d) return false;
        if (packet[8] != 0x45) return false;
        if (packet[9] != 0x31) return false;
        if (packet[10] != 0x2e) return false;
        if (packet[11] != 0x31) return false;
        if (packet[12] != 0x37) return false;

        _universe = ((int)_data[113] << 8) + (int)_data[114];
        _type = type;
        _length = len;
        memcpy(_data, packet, len);
    }
    else if (type == xFadeFrame::ID_ARTNETSOCKET)
    {
        // validate the packet
        if (len < ARTNET_PACKET_HEADERLEN) return false;
        if (packet[0] != 'A') return false;
        if (packet[1] != 'r') return false;
        if (packet[2] != 't') return false;
        if (packet[3] != '-') return false;
        if (packet[4] != 'N') return false;
        if (packet[5] != 'e') return false;
        if (packet[6] != 't') return false;
        if (packet[9] != 0x50) return true; // pretend success as otherwise I will log excessively

        _universe = ((int)_data[15] << 8) + (int)_data[14];
        _type = type;
        _length = len;
        memcpy(_data, packet, len);
    }
 
    return true;
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

        wxDatagramSocket datagram(localaddr, wxSOCKET_NOWAIT);

        if (!datagram.IsOk())
        {
            logger_base.error("E131 Output: Error opening datagram. Network may not be connected? OK : FALSE, Universe %d, From %s", _universe, (const char *)localaddr.IPAddress().c_str());
        }
        else if (datagram.Error() != wxSOCKET_NOERROR)
        {
            logger_base.error("Error creating E131 datagram => %d : %s, Universe %d from %s.", datagram.LastError(), (const char *)DecodeIPError(datagram.LastError()).c_str(), _universe, (const char *)localaddr.IPAddress().c_str());
        }
        else
        {
            wxIPV4address remoteaddr;
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
        if (_localIP == "")
        {
            localaddr.AnyAddress();
        }
        else
        {
            localaddr.Hostname(_localIP);
        }

        wxDatagramSocket datagram(localaddr, wxSOCKET_NOWAIT);

        if (!datagram.IsOk())
        {
            logger_base.error("ArtNET Output: Error opening datagram. Network may not be connected? OK : FALSE, Universe %d from %s", _universe, (const char *)localaddr.IPAddress().c_str());
        }
        else if (datagram.Error() != wxSOCKET_NOERROR)
        {
            logger_base.error("Error creating ArtNET datagram => %d : %s, Universe %d from %s.", datagram.LastError(), (const char *)DecodeIPError(datagram.LastError()).c_str(), _universe, (const char *)localaddr.IPAddress().c_str());
        }
        else
        {
            wxIPV4address remoteaddr;
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

int PacketData::GetSequenceNum() const
{
    if (_length <= 0) return -1;

    if (_type == xFadeFrame::ID_E131SOCKET)
    {
        return (int)_data[111];
    }
    else if (_type == xFadeFrame::ID_ARTNETSOCKET)
    {
        return (int)_data[12];
    }
    return -1;
}

void PacketData::InitialiseLength(long type, int length, int universe)
{
    _type = type;
    _length = length;
    _universe = universe;

    if (_type == xFadeFrame::ID_E131SOCKET)
    {
        InitialiseE131Header();
    }
    else if (_type == xFadeFrame::ID_ARTNETSOCKET)
    {
        InitialiseArtNETHeader();
    }
}

void PacketData::ApplyBrightness(int brightness)
{
    if (brightness == 100) return;

    if (brightness == 0)
    {
        memset(GetDataPtr(), 0x00, GetDataLength());
    }
    else
    {
        wxByte* p = GetDataPtr();
        for (int i = 0; i < GetDataLength(); i++)
        {
            *(p + i) = (wxByte)((int)*(p+i) * brightness / 100);
        }
    }
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

    if (_e131SocketReceive == nullptr)
    {
        logger_base.error("Problem listening for e131.");
        return;
    }
    else if (!_e131SocketReceive->IsOk())
    {
        logger_base.error("Problem listening for e131.");
        delete _e131SocketReceive;
        _e131SocketReceive = nullptr;
        return;
    }

    logger_base.debug("E131 listening on %s", (const char*)_settings._localInputIP.c_str());

    for (auto it = _settings._targetIP.begin(); it != _settings._targetIP.end(); ++it)
    {
        struct ip_mreq mreq;
        wxString ip = wxString::Format("239.255.%d.%d", it->first >> 8, it->first & 0xFF);
        logger_base.debug("E131 registering for multicast on %s.", (const char *)ip.c_str());
        mreq.imr_multiaddr.s_addr = inet_addr(ip.c_str());
        mreq.imr_interface.s_addr = inet_addr(_settings._localInputIP.c_str()); // this will only listen on the default interface
        if (!_e131SocketReceive->SetOption(IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char *)&mreq, sizeof(mreq)))
        {
            logger_base.warn("    Error opening E131 multicast listener %s.", (const char *)ip.c_str());
        }
        else
        {
            logger_base.debug("    E131 multicast listener %s registered.", (const char *)ip.c_str());
        }
    }

    //enable event handling
    logger_base.debug("    Setting E131 event handler.");
    _e131SocketReceive->SetEventHandler(*this, ID_E131SOCKET);

    //Notify us about incomming data
    logger_base.debug("    Turning on E131 notifications.");
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

    if (_artNETSocketReceive == nullptr)
    {
        logger_base.error("Problem listening for ArtNET.");
        return;
    }
    else if (!_artNETSocketReceive->IsOk())
    {
        logger_base.error("Problem listening for ArtNET.");
        delete _artNETSocketReceive;
        _artNETSocketReceive = nullptr;
        return;
    }

    logger_base.debug("ARTNet listening on %s", (const char*)_settings._localInputIP.c_str());

    for (auto it = _settings._targetIP.begin(); it != _settings._targetIP.end(); ++it)
    {
        struct ip_mreq mreq;
        wxString ip = wxString::Format("239.255.%d.%d", it->first >> 8, it->first & 0xFF);
        logger_base.debug("ARTNet registering for multicast on %s.", (const char *)ip.c_str());
        mreq.imr_multiaddr.s_addr = inet_addr(ip.c_str());
        mreq.imr_interface.s_addr = inet_addr(_settings._localInputIP.c_str()); // this will only listen on the default interface
        if (!_artNETSocketReceive->SetOption(IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char *)&mreq, sizeof(mreq)))
        {
            logger_base.warn("    Error opening ARTNet multicast listener %s.", (const char *)ip.c_str());
        }
        else
        {
            logger_base.warn("    ARTNet multicast listener %s registered.", (const char *)ip.c_str());
        }
    }

    //enable event handling
    logger_base.debug("    Setting ArtNET event handler.");
    _artNETSocketReceive->SetEventHandler(*this, ID_ARTNETSOCKET);

    //Notify us about incomming data
    logger_base.debug("    Turning on ArtNET notifications.");
    _artNETSocketReceive->SetNotify(wxSOCKET_INPUT_FLAG);
    //enable event handling
    _artNETSocketReceive->Notify(true);
}

void xFadeFrame::OnMIDIEvent(wxCommandEvent& event)
{
    wxByte status = (event.GetInt() >> 24) & 0xFF;
    wxByte channel = (event.GetInt() >> 16) & 0xFF;
    wxByte data1 = (event.GetInt() >> 8) & 0xFF;
    wxByte data2 = event.GetInt() & 0xFF;

    wxString controlName = _settings.LookupMIDI(status, channel, data1);

    if (controlName == "") return;

    wxWindow* control = wxFindWindowByName(controlName, this);

    if (control == nullptr) return;

    if (controlName.StartsWith("ID_TEXTCTRL"))
    {
        // this is the time field
        float value = (float)data2 * 10.0 / 127.0;
        ((wxTextCtrl*)control)->SetValue(wxString::Format("%.1f", value));
        wxCommandEvent e(wxEVT_TEXT);
        e.SetEventObject(control);
        e.SetId(control->GetId());
        wxPostEvent(this, e);
    }
    else if (controlName.StartsWith("ID_SLIDER"))
    {
        wxSlider* slider = (wxSlider*)control;
        int low = slider->GetMin();
        int max = slider->GetMax();
        float value = ((float)data2 * ((float)max - (float)low)) / 127.0 + (float)low;
        slider->SetValue(value);
        wxCommandEvent e(wxEVT_COMMAND_SLIDER_UPDATED);
        e.SetEventObject(control);
        e.SetId(control->GetId());
        wxPostEvent(this, e);
    }
    else if (controlName.StartsWith("ID_BUTTON"))
    {
        wxCommandEvent e(wxEVT_COMMAND_BUTTON_CLICKED);
        e.SetEventObject(control);
        e.SetId(control->GetId());
        wxPostEvent(this, e);
    }
}

void xFadeFrame::SetMIDIForControl(wxString controlName, float parm)
{
    int status, channel, data1;
    _settings.LookupMIDI(controlName, status, channel, data1);

    MIDIAssociateDialog dlg(this, controlName, _midiListener, status, channel, data1);
    if (dlg.ShowModal() == wxID_OK)
    {
        _settings.SetMIDIControl(controlName, (dlg.Choice_Status->GetSelection() << 4) + 0x80, dlg.Choice_Channel->GetSelection(), dlg.Choice_Data1->GetSelection());
        SaveState();
    }
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

void xFadeFrame::OnE131SocketEvent(wxSocketEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_e131SocketReceive == nullptr)
    {
        logger_base.error("ERROR: E131 socket received a packet but socket is null");
        wxASSERT(false);
        return;
    }

    wxASSERT(event.GetSocket() == _e131SocketReceive);

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
        wxASSERT(false);
        logger_base.warn("OnE131SocketEvent: Unexpected event !");
        break;
    }
}

void xFadeFrame::OnArtNETSocketEvent(wxSocketEvent & event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_artNETSocketReceive == nullptr)
    {
        logger_base.error("ERROR: ArtNET socket received a packet but socket is null");
        wxASSERT(false);
        return;
    }

    wxASSERT(event.GetSocket() == _artNETSocketReceive);

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
        wxASSERT(false);
        logger_base.warn("OnArtNETSocketEvent: Unexpected event !");
        break;
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
        _emitter->SetFrameMS(_settings._frameMS);
    }
}

void xFadeFrame::RestartInterfaces()
{
    CloseSockets(true);
    if (_settings._E131)
    {
        CreateE131Listener();
    }
    if (_settings._ArtNET)
    {
        CreateArtNETListener();
    }
    ValidateWindow();
}

void xFadeFrame::OnButtonClickLeft(wxCommandEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    int button = wxAtoi(((wxButton*)event.GetEventObject())->GetLabel());

    logger_base.debug("Playing jukebox left. %d", button);

    PressJukeboxButton(button, true);
}

void xFadeFrame::OnButtonRClickLeft(wxContextMenuEvent& event)
{
    int button = wxAtoi(((wxButton*)event.GetEventObject())->GetLabel());
    SetMIDIForControl(((wxWindow*)event.GetEventObject())->GetName(), button);
}

void xFadeFrame::PressJukeboxButton(int button, bool left)
{
    wxWindow* panel;
    int port = 0;
    if (left)
    {
        panel = Panel_Left;
        port = 1;
    }
    else
    {
        panel = Panel_Right;
        port = 2;
    }

    wxButton* b = GetJukeboxButton(button, panel);

    if (b != nullptr)
    {
        wxString result = xLightsRequest(port, "PLAY_JUKEBOX_BUTTON " + wxString::Format("%d", button));

        if (result.StartsWith("SUCCESS"))
        {
            auto buttons = panel->GetChildren();
            for (auto bb = buttons.begin(); bb != buttons.end(); ++bb)
            {
                ((wxButton*)*bb)->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
                ((wxButton*)*bb)->SetFont(wxNullFont);
            }

            b->SetForegroundColour(*wxBLUE);
            b->SetFont(_selectedButtonFont->Underlined());
        }
    }
}

int xFadeFrame::GetActiveButton(wxWindow* panel)
{
    auto buttons = panel->GetChildren();
    for (auto bb = buttons.begin(); bb != buttons.end(); ++bb)
    {
        if ((*bb)->GetForegroundColour() == *wxBLUE)
        {
            return wxAtoi((*bb)->GetLabel());
        }
    }

    return 0;
}

wxButton* xFadeFrame::GetJukeboxButton(int button, wxWindow* panel)
{
    wxString s = wxString::Format("%d", button);
    auto buttons = panel->GetChildren();
    for (auto bb = buttons.begin(); bb != buttons.end(); ++bb)
    {
        if ((*bb)->GetLabel() == s)
        {
            return (wxButton*)*bb;
        }
    }

    return nullptr;
}

void xFadeFrame::OnButtonClickRight(wxCommandEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    int button = wxAtoi(((wxButton*)event.GetEventObject())->GetLabel());

    logger_base.debug("Playing jukebox right. %d", button);

    PressJukeboxButton(button, false);
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

void xFadeFrame::OnSliderRClickFade(wxContextMenuEvent& event)
{
    SetMIDIForControl(((wxWindow*)event.GetEventObject())->GetName());
}

void xFadeFrame::OnSliderRClickLeftBrightness(wxContextMenuEvent& event)
{
    SetMIDIForControl(((wxWindow*)event.GetEventObject())->GetName());
}

void xFadeFrame::OnSliderRClickRightBrightness(wxContextMenuEvent& event)
{
    SetMIDIForControl(((wxWindow*)event.GetEventObject())->GetName());
}

void xFadeFrame::OnSliderRClickMasterBrightness(wxContextMenuEvent& event)
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

void xFadeFrame::OnButtonRClickAdvance(wxContextMenuEvent& event)
{
    SetMIDIForControl(((wxWindow*)event.GetEventObject())->GetName());
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
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    auto label = ((wxButton*)event.GetEventObject())->GetLabel();
    logger_base.debug("Cross fade time button clicked. %s", (const char *)label.c_str());
    TextCtrl_CrossFadeTime->SetValue(label);
}

void xFadeFrame::OnButtonRClickFT(wxContextMenuEvent& event)
{
    float time = wxAtof(((wxButton*)event.GetEventObject())->GetLabel());
    SetMIDIForControl(((wxWindow*)event.GetEventObject())->GetName(), time);
}

void xFadeFrame::OnButton_ConnectToxLightsClick(wxCommandEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Connecting to xLights ...");

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
                                (*b)->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
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
                                (*b)->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
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

    logger_base.debug("    Connecting to xLights done!");
}

void xFadeFrame::OnSlider1CmdSliderUpdated(wxScrollEvent& event)
{
    SetFade();
}

void xFadeFrame::OnButton_ConfigureClick(wxCommandEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Configure ...");

    SettingsDialog dlg(this, &_settings);

    CloseSockets();

    _emitter->Stop();
    delete _emitter;
    _emitter = nullptr;

    _midiListener->Stop();
    delete _midiListener;
    _midiListener = nullptr;

    if (dlg.ShowModal() == wxID_OK)
    {
        SaveState();
    }

    _emitter = new Emitter(&_settings._targetIP, &_leftData, &_rightData, &_settings._targetProtocol, &_lock, _settings._localOutputIP);
    _emitter->SetLeftBrightness(Slider_LeftBrightness->GetValue());
    _emitter->SetRightBrightness(Slider_RightBrightness->GetValue());
    SetTiming();
    SetFade();
    _midiListener = new MIDIListener(_settings.GetMIDIDeviceId(), this);
    RestartInterfaces();

    logger_base.debug("    Configuring done!");
}

void xFadeFrame::OnSlider_LeftBrightnessCmdSliderUpdated(wxScrollEvent& event)
{
    _emitter->SetLeftBrightness(Slider_LeftBrightness->GetValue());
}

void xFadeFrame::OnSlider_RightBrightnessCmdSliderUpdated(wxScrollEvent& event)
{
    _emitter->SetRightBrightness(Slider_RightBrightness->GetValue());
}

void xFadeFrame::OnSlider_MasterBrightnessCmdSliderUpdated(wxScrollEvent& event)
{
    Slider_LeftBrightness->SetValue(Slider_MasterBrightness->GetValue());
    Slider_RightBrightness->SetValue(Slider_MasterBrightness->GetValue());
    _emitter->SetLeftBrightness(Slider_LeftBrightness->GetValue());
    _emitter->SetRightBrightness(Slider_RightBrightness->GetValue());
}

void xFadeFrame::OnTextCtrl_CrossFadeTimeText(wxCommandEvent& event)
{
}

void xFadeFrame::OnButton_AdvanceClick(wxCommandEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    bool onLeft = ((_direction == -1 && UITimer.IsRunning()) || Slider1->GetValue() <= 5000);

    if (onLeft)
    {
        // get the button to press on the right
        int activeButton = GetActiveButton(Panel_Right);
        activeButton++;
        if (activeButton > JUKEBOXBUTTONS) activeButton = 1;

        logger_base.debug("Advance moving to button %d on right.", activeButton);

        // press active button
        PressJukeboxButton(activeButton, false);

        // initiate transition to right
        _direction = 1;
        UITimer.Start(25);
    }
    else
    {
        // get the button to press on the left
        int activeButton = GetActiveButton(Panel_Left);
        activeButton++;
        if (activeButton > JUKEBOXBUTTONS) activeButton = 1;

        logger_base.debug("Advance moving to button %d on left.", activeButton);

        // press active button
        PressJukeboxButton(activeButton, true);

        // initiate transition to left
        _direction = -1;
        UITimer.Start(25);
    }
}
