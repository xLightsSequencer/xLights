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

#include "xFadeMain.h"

#include <wx/msgdlg.h>
#include <wx/config.h>
#include <wx/protocol/http.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/debugrpt.h>

#include "../xLights/xLightsVersion.h"
#include "../xLights/UtilFunctions.h"
#include "../xLights/osxMacUtils.h"
#include "UniverseEntryDialog.h"
#include "Emitter.h"
#include "MIDIListener.h"
#include "SettingsDialog.h"
#include "MIDIAssociateDialog.h"
#include "UniverseData.h"
#include "E131Receiver.h"
#include "ArtNETReceiver.h"

#include <log4cpp/Category.hh>

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
const long xFadeFrame::ID_TIMER2 = wxNewId();
//*)

const long xFadeFrame::ID_LED1 = wxNewId();
const long xFadeFrame::ID_LED2 = wxNewId();

wxDEFINE_EVENT(EVT_FLASH, wxCommandEvent);
wxDEFINE_EVENT(EVT_TAG, wxCommandEvent);

BEGIN_EVENT_TABLE(xFadeFrame,wxFrame)
    //(*EventTable(xFadeFrame)
    //*)
    EVT_COMMAND(wxID_ANY, EVT_MIDI, xFadeFrame::OnMIDIEvent)
    EVT_COMMAND(wxID_ANY, EVT_FLASH, xFadeFrame::OnFlash)
    EVT_COMMAND(wxID_ANY, EVT_TAG, xFadeFrame::OnTag)
END_EVENT_TABLE()

xFadeFrame::xFadeFrame(wxWindow* parent, wxWindowID id)
{
    // static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    _e131Receiver = nullptr;
    _artNETReceiver = nullptr;
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
    int __wxStatusBarWidths_1[3] = { -10, -10, -10 };
    int __wxStatusBarStyles_1[3] = { wxSB_NORMAL, wxSB_NORMAL, wxSB_NORMAL };
    StatusBar1->SetFieldsCount(3,__wxStatusBarWidths_1);
    StatusBar1->SetStatusStyles(3,__wxStatusBarStyles_1);
    SetStatusBar(StatusBar1);
    UITimer.SetOwner(this, ID_TIMER1);
    UITimer.Start(1000, false);
    Timer_Status.SetOwner(this, ID_TIMER2);
    Timer_Status.Start(1000, false);
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
    Connect(ID_TIMER2,wxEVT_TIMER,(wxObjectEventFunction)&xFadeFrame::OnTimer_StatusTrigger);
    //*)

    Led_Left = new wxLed(this, ID_LED1, "808080", wxDefaultPosition, wxDefaultSize);
    Led_Left->Disable();
    Led_Left->SetColor("00FF00");
    FlexGridSizer3->Insert(0, Led_Left, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    Led_Right = new wxLed(this, ID_LED2, "808080", wxDefaultPosition, wxDefaultSize);
    Led_Right->SetColor("00FF00");
    Led_Right->Disable();
    FlexGridSizer3->Insert(2, Led_Right, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);

    Connect(Button_Advance->GetId(), wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&xFadeFrame::OnButtonRClickAdvance);
    Connect(Button_Left->GetId(), wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&xFadeFrame::OnButtonRClickFadeLeft);
    Connect(Button_Middle->GetId(), wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&xFadeFrame::OnButtonRClickFadeMiddle);
    Connect(Button_Right->GetId(), wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&xFadeFrame::OnButtonRClickFadeRight);
    Connect(TextCtrl_CrossFadeTime->GetId(), wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&xFadeFrame::OnTextCtrlRClickCrossFadeTime);
    Connect(Slider1->GetId(), wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&xFadeFrame::OnSliderRClickFade);
    Connect(Slider_LeftBrightness->GetId(), wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&xFadeFrame::OnSliderRClickLeftBrightness);
    Connect(Slider_RightBrightness->GetId(), wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&xFadeFrame::OnSliderRClickRightBrightness);
    Connect(Slider_MasterBrightness->GetId(), wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&xFadeFrame::OnSliderRClickMasterBrightness);

    SetTitle("xLights Fade " + GetDisplayVersionString());

    wxIconBundle icons;
    icons.AddIcon(wxIcon(xlights_16_xpm));
    icons.AddIcon(wxIcon(xlights_32_xpm));
    icons.AddIcon(wxIcon(xlights_64_xpm));
    icons.AddIcon(wxIcon(xlights_128_xpm));
    icons.AddIcon(wxIcon(xlights_xpm));
    SetIcons(icons);

    LoadState();

    OpenAll();

    InitialiseLEDs();

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

    TextCtrl_LeftSequence->SetValue("");
    TextCtrl_RightSequence->SetValue("");
    TextCtrl_LeftTag->SetValue("");
    TextCtrl_RightTag->SetValue("");

    StartMIDIListeners();

    ValidateWindow();
}

void xFadeFrame::InitialiseLEDs()
{
    if (_settings._minimiseUIUpdates)
    {
        Led_Left->SetColor("FFFF00");
        Led_Right->SetColor("FFFF00");
        Led_Left->Enable();
        Led_Right->Enable();
        StatusBar1->SetStatusText("", 0);
        if (_emitter == nullptr)
        {
            StatusBar1->SetStatusText("UI Updates Minimised - Sending Disabled", 1);
        }
        else
        {
            StatusBar1->SetStatusText("UI Updates Minimised", 1);
        }
        StatusBar1->SetStatusText("", 2);
    }
    else
    {
        Led_Left->SetColor("00FF00");
        Led_Right->SetColor("00FF00");
        Led_Left->Disable();
        Led_Right->Disable();
    }
}

void xFadeFrame::CloseAll()
{
    if (_emitter != nullptr)
    {
        _emitter->Stop();
        wxMilliSleep(100);
        delete _emitter;
        _emitter = nullptr;
    }

    if (_e131Receiver != nullptr)
    {
        _e131Receiver->Stop();
        wxMilliSleep(100);
        delete _e131Receiver;
        _e131Receiver = nullptr;
    }

    if (_artNETReceiver != nullptr)
    {
        _artNETReceiver->Stop();
        wxMilliSleep(100);
        delete _artNETReceiver;
        _artNETReceiver = nullptr;
    }

    for (auto it : _universeData)
    {
        delete it.second;
    }
    _universeData.clear();
}

void xFadeFrame::OpenAll()
{
    CloseAll();

    UniverseData::ClearTags();

    for (auto it : _settings._targetIP)
    {
        _universeData[it.first] = new UniverseData(it.first, it.second, _settings._targetProtocol[it.first], _settings.GetExcludeChannels(it.first));
    }

    if (_settings._ArtNET)
    {
        _artNETReceiver = new ArtNETReceiver(this, _universeData, _settings._localInputIP);
    }
    if (_settings._E131)
    {
        _e131Receiver = new E131Receiver(this, _universeData, _settings._localInputIP);
    }
    _emitter = new Emitter(_universeData, _settings._localOutputIP, &_settings);
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
    for (const auto& it: _midiListeners)
    {
        it->Stop();
        //delete it;
    }

    wxMilliSleep(100);

    CloseAll();

    SaveState();

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
    CloseAll();
    Close();
}

void xFadeFrame::OnAbout(wxCommandEvent& event)
{
    auto about = wxString::Format(wxT("xFade v%s, the xLights cross fader."), GetDisplayVersionString());
    wxMessageBox(about, _("Welcome to..."));
}

void xFadeFrame::OnFlash(wxCommandEvent& event)
{
    if (!_settings._minimiseUIUpdates)
    {
        if (event.GetInt() == 0)
        {
            Led_Left->Enable(!Led_Left->IsEnabled());
        }
        else
        {
            Led_Right->Enable(!Led_Right->IsEnabled());
        }
    }
}

void xFadeFrame::OnTag(wxCommandEvent& event)
{
    if (event.GetInt() == 0)
    {
        TextCtrl_LeftTag->SetValue(event.GetString());
    }
    else
    {
        TextCtrl_RightTag->SetValue(event.GetString());
    }
}

void xFadeFrame::OnMIDIEvent(wxCommandEvent& event)
{
    int device = wxAtoi(event.GetString());
    uint8_t status = (event.GetInt() >> 24) & 0xFF;
    uint8_t channel = (event.GetInt() >> 16) & 0xFF;
    uint8_t data1 = (event.GetInt() >> 8) & 0xFF;
    uint8_t data2 = event.GetInt() & 0xFF;

    wxString controlName = _settings.LookupMIDI(device, status, channel, data1, data2);

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
    int status, channel, data1, device, data2;
    _settings.LookupMIDI(controlName, device, status, channel, data1, data2);

    MIDIAssociateDialog dlg(this, _midiListeners, controlName, status, channel, data1, data2, Settings::GetMIDIDeviceName(device));
    if (dlg.ShowModal() == wxID_OK)
    {
        _settings.SetMIDIControl(dlg.Choice_MIDIDevice->GetStringSelection(), controlName, (dlg.Choice_Status->GetSelection() << 4) + 0x80, dlg.Choice_Channel->GetSelection(), dlg.Choice_Data1->GetSelection(), dlg.Choice_Data2->GetSelection());
        SaveState();
    }
    // This removes any unnecessary listeners the associate dialog added
    StartMIDIListeners();
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

void xFadeFrame::OnUITimerTrigger(wxTimerEvent& event)
{
    // this is where i need to move the fade slider
    float fadeTime = wxAtof(TextCtrl_CrossFadeTime->GetValue());
    if (fadeTime == 0.0f) fadeTime = 0.025f;
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

void xFadeFrame::StartMIDIListeners()
{
    auto devs = _settings.GetUsedMIDIDevices();
    for (auto it = begin(_midiListeners); it != end(_midiListeners); )
    {
        if (std::find(begin(devs), end(devs), (*it)->GetDeviceId()) == devs.end())
        {
            (*it)->Stop();
            delete *it;
            it = _midiListeners.erase(it);
        }
        else
        {
            ++it;
        }
    }
    for (const auto& it : _settings.GetUsedMIDIDevices())
    {
        bool found = false;
        for (const auto& it2 : _midiListeners)
        {
            if (it2->GetDeviceId() == it)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            _midiListeners.push_back(new MIDIListener(it, this));
        }
    }
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
    std::string ip = "127.0.0.1";
    if (left)
    {
        panel = Panel_Left;
        port = 1;
        ip = _settings._leftIP;
    }
    else
    {
        panel = Panel_Right;
        port = 2;
        ip = _settings._rightIP;
    }

    wxButton* b = GetJukeboxButton(button, panel);

    if (b != nullptr)
    {
        wxString result = xLightsRequest(port, "PLAY_JUKEBOX_BUTTON " + wxString::Format("%d", button), ip);

        if (result.StartsWith("SUCCESS"))
        {
            auto buttons = panel->GetChildren();
            for (const auto& bb : buttons)
            {
                ((wxButton*)bb)->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
                ((wxButton*)bb)->SetFont(wxNullFont);
            }

            b->SetForegroundColour(*wxBLUE);
            b->SetFont(_selectedButtonFont->Underlined());
        }
    }
}

int xFadeFrame::GetActiveButton(wxWindow* panel)
{
    auto buttons = panel->GetChildren();
    for (const auto& bb : buttons)
    {
        if (bb->GetForegroundColour() == *wxBLUE)
        {
            return wxAtoi(bb->GetLabel());
        }
    }

    return 0;
}

wxButton* xFadeFrame::GetJukeboxButton(int button, wxWindow* panel)
{
    wxString s = wxString::Format("%d", button);
    auto buttons = panel->GetChildren();
    for (const auto& bb : buttons)
    {
        if (bb->GetLabel() == s)
        {
            return (wxButton*)bb;
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
    UITimer.Start(25, wxTIMER_CONTINUOUS);
}

void xFadeFrame::OnButton_LeftClick(wxCommandEvent& event)
{
    _direction = -1;
    UITimer.Start(25, wxTIMER_CONTINUOUS);
}

void xFadeFrame::OnButton_RightClick(wxCommandEvent& event)
{
    _direction = 1;
    UITimer.Start(25, wxTIMER_CONTINUOUS);
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

    // suspend listening for packets
    if (_e131Receiver != nullptr) _e131Receiver->Suspend(true);
    if (_artNETReceiver != nullptr) _artNETReceiver->Suspend(true);

    _leftTag = "";
    _rightTag = "";
    TextCtrl_LeftSequence->SetValue("");
    TextCtrl_RightSequence->SetValue("");
    TextCtrl_LeftTag->SetValue("");
    TextCtrl_RightTag->SetValue("");

    wxString result = xLightsRequest(1, "GET_JUKEBOX_BUTTON_TOOLTIPS", _settings._leftIP);
    if (result.StartsWith("SUCCESS"))
    {
        Panel_Left->Enable(true);
        auto tips = wxSplit(result, '|');

        int i = 0;
        for (const auto& it : tips)
        {
            if (i > 0)
            {
                wxString s = wxString::Format("%d", i);
                auto buttons = Panel_Left->GetChildren();
                for (const auto& b : buttons)
                {
                    if (b->GetLabel() == s)
                    {
                        if (it == "")
                        {
                            b->UnsetToolTip();
                        }
                        else
                        {
                            b->SetToolTip(it);
                        }
                    }
                }
            }
            i++;
        }

        result = xLightsRequest(1, "GET_JUKEBOX_BUTTON_EFFECTPRESENT", _settings._leftIP);
        if (result.StartsWith("SUCCESS"))
        {
            auto bs = wxSplit(result, '|');

            i = 0;
            for (const auto& it : bs)
            {
                if (i > 0)
                {
                    wxString s = wxString::Format("%d", i);
                    auto buttons = Panel_Left->GetChildren();
                    for (const auto& b : buttons)
                    {
                        if (b->GetLabel() == s)
                        {
                            if (it == "")
                            {
                                b->SetBackgroundColour(wxColour(255, 108, 108));
                            }
                            else
                            {
                                b->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
                            }
                        }
                    }
                }
                i++;
            }
        }

        result = xLightsRequest(1, "GET_E131_TAG", _settings._leftIP);
        if (result.StartsWith("SUCCESS "))
        {
            _leftTag = result.substr(sizeof("SUCCESS ") - 1);
        }
        else
        {
            _leftTag = "";
        }
        TextCtrl_LeftTag->SetValue(_leftTag);

        result = xLightsRequest(1, "GET_SEQUENCE_NAME", _settings._leftIP);
        if (result.StartsWith("SUCCESS "))
        {
            TextCtrl_LeftSequence->SetValue(result.substr(sizeof("SUCCESS ") - 1));
        }
        else
        {
            TextCtrl_LeftSequence->SetValue("No sequence loaded.");
        }

        result = xLightsRequest(1, "TURN_LIGHTS_ON", _settings._leftIP);
    }
    else
    {
        Panel_Left->Enable(false);
    }

    result = xLightsRequest(2, "GET_JUKEBOX_BUTTON_TOOLTIPS", _settings._rightIP);
    if (result.StartsWith("SUCCESS"))
    {
        Panel_Right->Enable(true);

        auto tips = wxSplit(result, '|');

        int i = 0;
        for (const auto& it : tips)
        {
            if (i > 0)
            {
                wxString s = wxString::Format("%d", i);
                auto buttons = Panel_Right->GetChildren();
                for (const auto& b : buttons)
                {
                    if (b->GetLabel() == s)
                    {
                        if (it == "")
                        {
                            b->UnsetToolTip();
                        }
                        else
                        {
                            b->SetToolTip(it);
                        }
                    }
                }
            }
            i++;
        }

        result = xLightsRequest(2, "GET_JUKEBOX_BUTTON_EFFECTPRESENT", _settings._rightIP);
        if (result.StartsWith("SUCCESS"))
        {
            auto bs = wxSplit(result, '|');

            i = 0;
            for (const auto& it : bs)
            {
                if (i > 0)
                {
                    wxString s = wxString::Format("%d", i);
                    auto buttons = Panel_Right->GetChildren();
                    for (const auto& b : buttons)
                    {
                        if (b->GetLabel() == s)
                        {
                            if (it == "")
                            {
                                b->SetBackgroundColour(wxColour(255, 108, 108));
                            }
                            else
                            {
                                b->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
                            }
                        }
                    }
                }
                i++;
            }
        }

        result = xLightsRequest(2, "GET_E131_TAG", _settings._rightIP);
        if (result.StartsWith("SUCCESS "))
        {
            _rightTag = result.substr(sizeof("SUCCESS ") - 1);
        }
        else
        {
            _rightTag = "";
        }
        TextCtrl_RightTag->SetValue(_rightTag);

        result = xLightsRequest(2, "GET_SEQUENCE_NAME", _settings._rightIP);
        if (result.StartsWith("SUCCESS "))
        {
            TextCtrl_RightSequence->SetValue(result.substr(sizeof("SUCCESS ") - 1));
        }
        else
        {
            TextCtrl_RightSequence->SetValue("No sequence loaded.");
        }

        result = xLightsRequest(2, "TURN_LIGHTS_ON", _settings._rightIP);
    }
    else
    {
        Panel_Right->Enable(false);
    }

    if (_e131Receiver != nullptr) _e131Receiver->ZeroReceived();
    if (_artNETReceiver != nullptr) _artNETReceiver->ZeroReceived();
    _emitter->ZeroSent();
    InitialiseLEDs();

    // un suspend listening for packets
    if (_e131Receiver != nullptr) _e131Receiver->Suspend(false);
    if (_artNETReceiver != nullptr) _artNETReceiver->Suspend(false);

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

    if (_e131Receiver != nullptr) _e131Receiver->Suspend(true);
    if (_artNETReceiver != nullptr) _artNETReceiver->Suspend(true);

    SettingsDialog dlg(this, &_settings);

    CloseAll();

    if (dlg.ShowModal() == wxID_OK)
    {
        SaveState();
    }

    OpenAll();

    _emitter->SetLeftBrightness(Slider_LeftBrightness->GetValue());
    _emitter->SetRightBrightness(Slider_RightBrightness->GetValue());
    SetTiming();
    SetFade();

    if (_e131Receiver != nullptr) _e131Receiver->ZeroReceived();
    if (_artNETReceiver != nullptr) _artNETReceiver->ZeroReceived();
    _emitter->ZeroSent();
    InitialiseLEDs();

    if (_e131Receiver != nullptr) _e131Receiver->Suspend(false);
    if (_artNETReceiver != nullptr) _artNETReceiver->Suspend(false);

    logger_base.debug("    Configuring done!");

    ValidateWindow();
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
        UITimer.Start(25, wxTIMER_CONTINUOUS);
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
        UITimer.Start(25, wxTIMER_CONTINUOUS);
    }
}

void xFadeFrame::OnTimer_StatusTrigger(wxTimerEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_settings._minimiseUIUpdates) return;

    static uint32_t count = 0;
    count++;
    uint32_t leftReceived = (_e131Receiver != nullptr ? _e131Receiver->GetLeftReceived() : 0) + (_artNETReceiver != nullptr ? _artNETReceiver->GetLeftReceived() : 0);
    uint32_t rightReceived = (_e131Receiver != nullptr ? _e131Receiver->GetRightReceived() : 0) + (_artNETReceiver != nullptr ? _artNETReceiver->GetRightReceived() : 0);
    StatusBar1->SetStatusText(wxString::Format("Left: %lu", leftReceived), 0);
    StatusBar1->SetStatusText(wxString::Format("Right: %lu", rightReceived), 2);
    if (_emitter != nullptr)
    {
        StatusBar1->SetStatusText(wxString::Format("Sent: %lu", _emitter->GetSent()), 1);
        if (count % 60 == 0)
        {
            logger_base.debug("Activity - Left Received %lu, Right Received %lu, Sent %lu.", leftReceived, rightReceived, _emitter->GetSent());
        }
    }
    else
    {
        StatusBar1->SetStatusText("Sending disabled", 1);
        if (count % 60 == 0)
        {
            logger_base.debug("Activity - Left Received %lu, Right Received %lu, Sent DISABLED.", leftReceived, rightReceived);
        }
    }
}
