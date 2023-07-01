/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "OptionsDialog.h"

#include "ScheduleOptions.h"
#include "ButtonDetailsDialog.h"
#include "UserButton.h"
#include "CommandManager.h"
#include "../xLights/xLightsVersion.h"
#include <wx/xml/xml.h>
#include <wx/file.h>
#include "../xLights/AudioManager.h"
#include "City.h"
#include "../xLights/UtilFunctions.h"
#include "../xLights/outputs/IPOutput.h"
#include "PlayList/VideoWindowPositionDialog.h"
#include "xScheduleMain.h"
#include "ScheduleManager.h"

//(*InternalHeaders(OptionsDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)
#include <wx/config.h>

//(*IdInit(OptionsDialog)
const long OptionsDialog::ID_CHECKBOX4 = wxNewId();
const long OptionsDialog::ID_CHECKBOX3 = wxNewId();
const long OptionsDialog::ID_CHECKBOX5 = wxNewId();
const long OptionsDialog::ID_CHECKBOX2 = wxNewId();
const long OptionsDialog::ID_CHECKBOX6 = wxNewId();
const long OptionsDialog::ID_CHECKBOX7 = wxNewId();
const long OptionsDialog::ID_CHECKBOX8 = wxNewId();
const long OptionsDialog::ID_CHECKBOX9 = wxNewId();
const long OptionsDialog::ID_CHECKBOX10 = wxNewId();
const long OptionsDialog::ID_CHECKBOX11 = wxNewId();
const long OptionsDialog::ID_CHECKBOX12 = wxNewId();
const long OptionsDialog::ID_CHECKBOX14 = wxNewId();
const long OptionsDialog::ID_CHECKBOX15 = wxNewId();
const long OptionsDialog::ID_CHECKBOX16 = wxNewId();
const long OptionsDialog::ID_CHECKBOX17 = wxNewId();
const long OptionsDialog::ID_CHECKBOX18 = wxNewId();
const long OptionsDialog::ID_STATICTEXT2 = wxNewId();
const long OptionsDialog::ID_LISTVIEW1 = wxNewId();
const long OptionsDialog::ID_BUTTON5 = wxNewId();
const long OptionsDialog::ID_BUTTON6 = wxNewId();
const long OptionsDialog::ID_BUTTON7 = wxNewId();
const long OptionsDialog::ID_BUTTON10 = wxNewId();
const long OptionsDialog::ID_BUTTON9 = wxNewId();
const long OptionsDialog::ID_STATICTEXT3 = wxNewId();
const long OptionsDialog::ID_SPINCTRL1 = wxNewId();
const long OptionsDialog::ID_STATICTEXT4 = wxNewId();
const long OptionsDialog::ID_TEXTCTRL1 = wxNewId();
const long OptionsDialog::ID_STATICTEXT13 = wxNewId();
const long OptionsDialog::ID_TEXTCTRL3 = wxNewId();
const long OptionsDialog::ID_CHECKBOX1 = wxNewId();
const long OptionsDialog::ID_CHECKBOX13 = wxNewId();
const long OptionsDialog::ID_STATICTEXT5 = wxNewId();
const long OptionsDialog::ID_TEXTCTRL2 = wxNewId();
const long OptionsDialog::ID_STATICTEXT6 = wxNewId();
const long OptionsDialog::ID_SPINCTRL2 = wxNewId();
const long OptionsDialog::ID_STATICTEXT7 = wxNewId();
const long OptionsDialog::ID_CHOICE1 = wxNewId();
const long OptionsDialog::ID_STATICTEXT12 = wxNewId();
const long OptionsDialog::ID_CHOICE7 = wxNewId();
const long OptionsDialog::ID_STATICTEXT11 = wxNewId();
const long OptionsDialog::ID_CHOICE6 = wxNewId();
const long OptionsDialog::ID_STATICTEXT14 = wxNewId();
const long OptionsDialog::ID_CHOICE8 = wxNewId();
const long OptionsDialog::ID_STATICTEXT8 = wxNewId();
const long OptionsDialog::ID_CHOICE2 = wxNewId();
const long OptionsDialog::ID_STATICTEXT9 = wxNewId();
const long OptionsDialog::ID_CHOICE4 = wxNewId();
const long OptionsDialog::ID_STATICTEXT1 = wxNewId();
const long OptionsDialog::ID_CHOICE3 = wxNewId();
const long OptionsDialog::ID_STATICTEXT10 = wxNewId();
const long OptionsDialog::ID_CHOICE5 = wxNewId();
const long OptionsDialog::ID_BUTTONDEFAULTWINODOWLOC = wxNewId();
const long OptionsDialog::ID_BUTTON1 = wxNewId();
const long OptionsDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(OptionsDialog,wxDialog)
	//(*EventTable(OptionsDialog)
	//*)
END_EVENT_TABLE()

OptionsDialog::OptionsDialog(wxWindow* parent, CommandManager* commandManager, ScheduleOptions* options, wxWindowID id, const wxPoint& pos, const wxSize& size) : _commandManager(commandManager)
{
    _options = options;
    _dragging = false;

    //(*Initialize(OptionsDialog)
    wxBoxSizer* BoxSizer1;
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer5;
    wxFlexGridSizer* FlexGridSizer6;
    wxFlexGridSizer* FlexGridSizer7;
    wxFlexGridSizer* FlexGridSizer8;

    Create(parent, wxID_ANY, _("Options"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("wxID_ANY"));
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer7 = new wxFlexGridSizer(0, 3, 0, 0);
    CheckBox_SimpleMode = new wxCheckBox(this, ID_CHECKBOX4, _("Advanced Mode"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX4"));
    CheckBox_SimpleMode->SetValue(false);
    FlexGridSizer7->Add(CheckBox_SimpleMode, 1, wxALL|wxEXPAND, 5);
    CheckBox_SendOffWhenNotRunning = new wxCheckBox(this, ID_CHECKBOX3, _("Send data when not running sequence"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX3"));
    CheckBox_SendOffWhenNotRunning->SetValue(false);
    FlexGridSizer7->Add(CheckBox_SendOffWhenNotRunning, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_RunBackground = new wxCheckBox(this, ID_CHECKBOX5, _("Run background playlist when not running sequence"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX5"));
    CheckBox_RunBackground->SetValue(false);
    FlexGridSizer7->Add(CheckBox_RunBackground, 1, wxALL|wxEXPAND, 5);
    CheckBox_Sync = new wxCheckBox(this, ID_CHECKBOX2, _("Use ArtNet/E1.31 Synchronisation Protocols"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
    CheckBox_Sync->SetValue(false);
    FlexGridSizer7->Add(CheckBox_Sync, 1, wxALL|wxEXPAND, 5);
    CheckBox_MultithreadedTransmission = new wxCheckBox(this, ID_CHECKBOX6, _("Multithreaded transmission"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX6"));
    CheckBox_MultithreadedTransmission->SetValue(false);
    FlexGridSizer7->Add(CheckBox_MultithreadedTransmission, 1, wxALL|wxEXPAND, 5);
    CheckBox_RetryOpen = new wxCheckBox(this, ID_CHECKBOX7, _("Continually try to open outputs"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX7"));
    CheckBox_RetryOpen->SetValue(false);
    FlexGridSizer7->Add(CheckBox_RetryOpen, 1, wxALL|wxEXPAND, 5);
    CheckBox_RemoteAllOff = new wxCheckBox(this, ID_CHECKBOX8, _("When in remote mode turn off lights when master stops"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX8"));
    CheckBox_RemoteAllOff->SetValue(true);
    FlexGridSizer7->Add(CheckBox_RemoteAllOff, 1, wxALL|wxEXPAND, 5);
    CheckBox_SuppressAudioOnRemotes = new wxCheckBox(this, ID_CHECKBOX9, _("Suppress audio on remotes"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX9"));
    CheckBox_SuppressAudioOnRemotes->SetValue(true);
    FlexGridSizer7->Add(CheckBox_SuppressAudioOnRemotes, 1, wxALL|wxEXPAND, 5);
    CheckBox_HWAcceleratedVideo = new wxCheckBox(this, ID_CHECKBOX10, _("Use hardware accelerated video"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX10"));
    CheckBox_HWAcceleratedVideo->SetValue(true);
    FlexGridSizer7->Add(CheckBox_HWAcceleratedVideo, 1, wxALL|wxEXPAND, 5);
    CheckBox_LastStartingSequenceUsesTime = new wxCheckBox(this, ID_CHECKBOX11, _("Late starting scheduled sequence uses time to determine start location"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX11"));
    CheckBox_LastStartingSequenceUsesTime->SetValue(false);
    FlexGridSizer7->Add(CheckBox_LastStartingSequenceUsesTime, 1, wxALL|wxEXPAND, 5);
    CheckBox_KeepScreenOn = new wxCheckBox(this, ID_CHECKBOX12, _("Keep computer screen on"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX12"));
    CheckBox_KeepScreenOn->SetValue(false);
    FlexGridSizer7->Add(CheckBox_KeepScreenOn, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_MinimiseUI = new wxCheckBox(this, ID_CHECKBOX14, _("Minimise runtime UI updates for performance"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX14"));
    CheckBox_MinimiseUI->SetValue(false);
    FlexGridSizer7->Add(CheckBox_MinimiseUI, 1, wxALL|wxEXPAND, 5);
    CheckBox_DisableOutputOnPingFailure = new wxCheckBox(this, ID_CHECKBOX15, _("Disable output on local ping failure"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX15"));
    CheckBox_DisableOutputOnPingFailure->SetValue(false);
    FlexGridSizer7->Add(CheckBox_DisableOutputOnPingFailure, 1, wxALL|wxEXPAND, 5);
    CheckBox_SongMMSSFormat = new wxCheckBox(this, ID_CHECKBOX16, _("Use Song#:MM:SS timecode format"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX16"));
    CheckBox_SongMMSSFormat->SetValue(false);
    FlexGridSizer7->Add(CheckBox_SongMMSSFormat, 1, wxALL|wxEXPAND, 5);
    CheckBox_TimecodeWaitForNextSong = new wxCheckBox(this, ID_CHECKBOX17, _("Timecode remote wait for next song"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX17"));
    CheckBox_TimecodeWaitForNextSong->SetValue(false);
    FlexGridSizer7->Add(CheckBox_TimecodeWaitForNextSong, 1, wxALL|wxEXPAND, 5);
    CheckBoxSuppressDarkMode = new wxCheckBox(this, ID_CHECKBOX18, _("Suppress Dark Mode"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX18"));
    CheckBoxSuppressDarkMode->SetValue(false);
    FlexGridSizer7->Add(CheckBoxSuppressDarkMode, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer1->Add(FlexGridSizer7, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer5 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer5->AddGrowableCol(1);
    FlexGridSizer5->AddGrowableRow(0);
    StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Buttons:    "), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer5->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_TOP, 5);
    ListView_Buttons = new wxListView(this, ID_LISTVIEW1, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL|wxLC_NO_SORT_HEADER|wxVSCROLL|wxALWAYS_SHOW_SB, wxDefaultValidator, _T("ID_LISTVIEW1"));
    FlexGridSizer5->Add(ListView_Buttons, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer6 = new wxFlexGridSizer(0, 1, 0, 0);
    Button_ButtonAdd = new wxButton(this, ID_BUTTON5, _("Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON5"));
    FlexGridSizer6->Add(Button_ButtonAdd, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    Button_ButtonEdit = new wxButton(this, ID_BUTTON6, _("Edit"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON6"));
    FlexGridSizer6->Add(Button_ButtonEdit, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    Button_ButtonDelete = new wxButton(this, ID_BUTTON7, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON7"));
    FlexGridSizer6->Add(Button_ButtonDelete, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    Button_Export = new wxButton(this, ID_BUTTON10, _("Export"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON10"));
    FlexGridSizer6->Add(Button_Export, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    Button_Import = new wxButton(this, ID_BUTTON9, _("Import"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON9"));
    FlexGridSizer6->Add(Button_Import, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer5->Add(FlexGridSizer6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer5, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer8 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer8->AddGrowableCol(1);
    StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Web Server Port:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer8->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    SpinCtrl_WebServerPort = new wxSpinCtrl(this, ID_SPINCTRL1, _T("80"), wxDefaultPosition, wxDefaultSize, 0, 1, 64000, 80, _T("ID_SPINCTRL1"));
    SpinCtrl_WebServerPort->SetValue(_T("80"));
    FlexGridSizer8->Add(SpinCtrl_WebServerPort, 1, wxALL|wxEXPAND, 5);
    StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Web Directory:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    FlexGridSizer8->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_wwwRoot = new wxTextCtrl(this, ID_TEXTCTRL1, _("xScheduleWeb"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
    TextCtrl_wwwRoot->SetHelpText(_("Folder containing the xSchedule website"));
    FlexGridSizer8->Add(TextCtrl_wwwRoot, 1, wxALL|wxEXPAND, 5);
    StaticText13 = new wxStaticText(this, ID_STATICTEXT13, _("Default Page:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT13"));
    FlexGridSizer8->Add(StaticText13, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_DefaultPage = new wxTextCtrl(this, ID_TEXTCTRL3, _("index.html"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL3"));
    FlexGridSizer8->Add(TextCtrl_DefaultPage, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer8->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    CheckBox_APIOnly = new wxCheckBox(this, ID_CHECKBOX1, _("API Only"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
    CheckBox_APIOnly->SetValue(false);
    BoxSizer1->Add(CheckBox_APIOnly, 1, wxALL|wxEXPAND, 5);
    CheckBox_AlllowPageBypass = new wxCheckBox(this, ID_CHECKBOX13, _("Allow guest page bypass of login."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX13"));
    CheckBox_AlllowPageBypass->SetValue(false);
    BoxSizer1->Add(CheckBox_AlllowPageBypass, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer8->Add(BoxSizer1, 1, wxALL|wxEXPAND, 5);
    StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Password:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
    FlexGridSizer8->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_Password = new wxTextCtrl(this, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
    TextCtrl_Password->SetHelpText(_("Website password."));
    FlexGridSizer8->Add(TextCtrl_Password, 1, wxALL|wxEXPAND, 5);
    StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Timeout (minutes):"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
    FlexGridSizer8->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    SpinCtrl_PasswordTimeout = new wxSpinCtrl(this, ID_SPINCTRL2, _T("30"), wxDefaultPosition, wxDefaultSize, 0, 1, 1440, 30, _T("ID_SPINCTRL2"));
    SpinCtrl_PasswordTimeout->SetValue(_T("30"));
    FlexGridSizer8->Add(SpinCtrl_PasswordTimeout, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer1->Add(FlexGridSizer8, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer3 = new wxFlexGridSizer(0, 4, 0, 0);
    FlexGridSizer3->AddGrowableCol(3);
    StaticText7 = new wxStaticText(this, ID_STATICTEXT7, _("Default Audio Output Device:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
    FlexGridSizer3->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Choice_AudioDevice = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
    Choice_AudioDevice->SetSelection( Choice_AudioDevice->Append(_("(Default)")) );
    FlexGridSizer3->Add(Choice_AudioDevice, 1, wxALL|wxEXPAND, 5);
    StaticText12 = new wxStaticText(this, ID_STATICTEXT12, _("Default Input Audio Device:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT12"));
    FlexGridSizer3->Add(StaticText12, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Choice_InputAudioDevice = new wxChoice(this, ID_CHOICE7, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE7"));
    FlexGridSizer3->Add(Choice_InputAudioDevice, 1, wxALL|wxEXPAND, 5);
    StaticText11 = new wxStaticText(this, ID_STATICTEXT11, _("SMPTE Timecode Frame Rate:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT11"));
    FlexGridSizer3->Add(StaticText11, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Choice_SMPTEFrameRate = new wxChoice(this, ID_CHOICE6, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE6"));
    FlexGridSizer3->Add(Choice_SMPTEFrameRate, 1, wxALL|wxEXPAND, 5);
    StaticText14 = new wxStaticText(this, ID_STATICTEXT14, _("SMPTE Input Device:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT14"));
    FlexGridSizer3->Add(StaticText14, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Choice_SMPTEDevice = new wxChoice(this, ID_CHOICE8, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE8"));
    FlexGridSizer3->Add(Choice_SMPTEDevice, 1, wxALL|wxEXPAND, 5);
    StaticText8 = new wxStaticText(this, ID_STATICTEXT8, _("ARTNet Time Code Format:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
    FlexGridSizer3->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Choice_ARTNetTimeCodeFormat = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
    Choice_ARTNetTimeCodeFormat->Append(_("Film - 24 fps"));
    Choice_ARTNetTimeCodeFormat->SetSelection( Choice_ARTNetTimeCodeFormat->Append(_("EBU - 25 fps")) );
    Choice_ARTNetTimeCodeFormat->Append(_("DF - 29.97 fps"));
    Choice_ARTNetTimeCodeFormat->Append(_("SMPTE - 30 fps"));
    FlexGridSizer3->Add(Choice_ARTNetTimeCodeFormat, 1, wxALL|wxEXPAND, 5);
    StaticText9 = new wxStaticText(this, ID_STATICTEXT9, _("Behaviour on crash:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
    FlexGridSizer3->Add(StaticText9, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Choice_OnCrash = new wxChoice(this, ID_CHOICE4, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE4"));
    Choice_OnCrash->SetSelection( Choice_OnCrash->Append(_("Prompt user")) );
    Choice_OnCrash->Append(_("Silently exit after sending crash log"));
    Choice_OnCrash->Append(_("Silently exit without sending crash log"));
    FlexGridSizer3->Add(Choice_OnCrash, 1, wxALL|wxEXPAND, 5);
    StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Location:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer3->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Choice_Location = new wxChoice(this, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE3"));
    FlexGridSizer3->Add(Choice_Location, 1, wxALL|wxEXPAND, 5);
    StaticText10 = new wxStaticText(this, ID_STATICTEXT10, _("Force Local IP:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT10"));
    FlexGridSizer3->Add(StaticText10, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Choice1 = new wxChoice(this, ID_CHOICE5, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE5"));
    Choice1->SetToolTip(_("When set this will override any forced IP on all controllers. If you want to set individual controllers to specific network cards then you must do this in xLights."));
    FlexGridSizer3->Add(Choice1, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer3->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_DefaultWindowLocation = new wxButton(this, ID_BUTTONDEFAULTWINODOWLOC, _("Set Default Video/Virtual Matrix Location"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTONDEFAULTWINODOWLOC"));
    FlexGridSizer3->Add(Button_DefaultWindowLocation, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
    Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    Button_Ok->SetDefault();
    FlexGridSizer2->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    FlexGridSizer2->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);
    Center();

    Connect(ID_LISTVIEW1,wxEVT_COMMAND_LIST_BEGIN_DRAG,(wxObjectEventFunction)&OptionsDialog::OnListView_ButtonsBeginDrag);
    Connect(ID_LISTVIEW1,wxEVT_COMMAND_LIST_ITEM_SELECTED,(wxObjectEventFunction)&OptionsDialog::OnListView_ButtonsItemSelect);
    Connect(ID_LISTVIEW1,wxEVT_COMMAND_LIST_ITEM_ACTIVATED,(wxObjectEventFunction)&OptionsDialog::OnListView_ButtonsItemActivated);
    Connect(ID_LISTVIEW1,wxEVT_COMMAND_LIST_KEY_DOWN,(wxObjectEventFunction)&OptionsDialog::OnListView_ButtonsKeyDown);
    Connect(ID_BUTTON5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OptionsDialog::OnButton_ButtonAddClick);
    Connect(ID_BUTTON6,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OptionsDialog::OnButton_ButtonEditClick);
    Connect(ID_BUTTON7,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OptionsDialog::OnButton_ButtonDeleteClick);
    Connect(ID_BUTTON10,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OptionsDialog::OnButton_ExportClick);
    Connect(ID_BUTTON9,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OptionsDialog::OnButton_ImportClick);
    Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&OptionsDialog::OnTextCtrl_wwwRootText);
    Connect(ID_BUTTONDEFAULTWINODOWLOC,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OptionsDialog::OnButton_DefaultWindowLocationClick);
    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OptionsDialog::OnButton_OkClick);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OptionsDialog::OnButton_CancelClick);
    //*)

    auto audioDevices = AudioManager::GetAudioDevices();
    for (const auto& it : audioDevices) {
        Choice_AudioDevice->Append(it);
    }

    Choice_InputAudioDevice->SetSelection(Choice_InputAudioDevice->Append(_("(Default)")));
    Choice_SMPTEDevice->SetSelection(Choice_SMPTEDevice->Append(_("(Default)")));
    auto inputAudioDevices = AudioManager::GetInputAudioDevices();
    for (const auto& it : inputAudioDevices) {
        Choice_InputAudioDevice->Append(it);
        Choice_SMPTEDevice->Append(it);
    }

    auto cities = City::GetCities();
    for (auto it = cities.begin(); it != cities.end(); ++it) {
        Choice_Location->Append(*it);
    }

    ListView_Buttons->AppendColumn("Label");
    ListView_Buttons->AppendColumn("Command");
    ListView_Buttons->AppendColumn("Parameters");
    ListView_Buttons->AppendColumn("Hotkey");
    ListView_Buttons->AppendColumn("Web Color");

    Choice_OnCrash->SetStringSelection(options->GetCrashBehaviour());
    CheckBox_SendOffWhenNotRunning->SetValue(options->IsSendOffWhenNotRunning());
    CheckBox_MultithreadedTransmission->SetValue(options->IsParallelTransmission());
    Choice_ARTNetTimeCodeFormat->SetSelection(static_cast<int>(options->GetARTNetTimeCodeFormat()));
    CheckBox_RunBackground->SetValue(options->IsSendBackgroundWhenNotRunning());
    CheckBox_Sync->SetValue(options->IsSync());
    CheckBox_APIOnly->SetValue(options->GetAPIOnly());
    CheckBox_SimpleMode->SetValue(options->IsAdvancedMode());
    CheckBox_RetryOpen->SetValue(options->IsRetryOpen());
    CheckBox_RemoteAllOff->SetValue(options->IsRemoteAllOff());
    CheckBox_KeepScreenOn->SetValue(options->IsKeepScreenOn());
    CheckBox_MinimiseUI->SetValue(options->IsMinimiseUIUpdates());
    CheckBox_SuppressAudioOnRemotes->SetValue(options->IsSuppressAudioOnRemotes());
    CheckBox_HWAcceleratedVideo->SetValue(options->IsHardwareAcceleratedVideo());
    CheckBox_LastStartingSequenceUsesTime->SetValue(options->IsLateStartingScheduleUsesTime());
    CheckBox_DisableOutputOnPingFailure->SetValue(options->IsDisableOutputOnPingFailure());
    CheckBox_SongMMSSFormat->SetValue(options->IsUseStepMMSSTimecodeFormat());
    CheckBox_TimecodeWaitForNextSong->SetValue(options->IsRemoteTimecodeStepAdvance());

    #ifdef __WXMSW__
        CheckBoxSuppressDarkMode->SetValue(IsSuppressDarkMode());
    #else
        CheckBoxSuppressDarkMode->Enable(false);
    #endif

    SpinCtrl_WebServerPort->SetValue(options->GetWebServerPort());
    SpinCtrl_PasswordTimeout->SetValue(options->GetPasswordTimeout());

    TextCtrl_wwwRoot->SetValue(options->GetWWWRoot());
    StaticText4->SetToolTip("Root Directory: " + options->GetDefaultRoot());
    TextCtrl_Password->SetValue(options->GetPassword());
    TextCtrl_DefaultPage->SetValue(options->GetDefaultPage());
    CheckBox_AlllowPageBypass->SetValue(options->GetAllowUnauth());
    Choice_Location->SetStringSelection(options->GetCity());
    Choice_AudioDevice->SetStringSelection(options->GetAudioDevice());
    if (Choice_AudioDevice->GetSelection() == -1) {
        Choice_AudioDevice->SetSelection(0);
    }
    Choice_InputAudioDevice->SetStringSelection(options->GetInputAudioDevice());
    if (Choice_InputAudioDevice->GetSelection() == -1) {
        Choice_InputAudioDevice->SetSelection(0);
    }
    Choice_SMPTEDevice->SetStringSelection(options->GetSMPTEDevice());
    if (Choice_SMPTEDevice->GetSelection() == -1) {
        Choice_SMPTEDevice->SetSelection(0);
    }

    Choice1->AppendString("");
    for (auto it : GetLocalIPs()) {
        Choice1->AppendString(it);
    }
    Choice1->SetStringSelection(((xScheduleFrame*)parent)->GetScheduleManager()->GetForceLocalIP());

    for (auto it : options->GetSPMTEModes()) {
        Choice_SMPTEFrameRate->AppendString(it);
    }
    Choice_SMPTEFrameRate->SetStringSelection(options->DecodeSMPTEMode(options->GetSMPTEMode()));

    LoadButtons();

    SetSize(800, 700);
    Layout();

    int namew = 150;
    int hotkeyw = 70;
    int w, h;
    int colorw = 150;
    ListView_Buttons->GetSize(&w, &h);
    ListView_Buttons->SetColumnWidth(0, namew);
    ListView_Buttons->SetColumnWidth(1, (w - colorw - namew - hotkeyw - 1) / 2);
    ListView_Buttons->SetColumnWidth(2, (w - colorw - namew - hotkeyw - 1) / 2);
    ListView_Buttons->SetColumnWidth(3, hotkeyw);
    ListView_Buttons->SetColumnWidth(4, colorw);

    SetEscapeId(Button_Cancel->GetId());
    SetAffirmativeId(Button_Ok->GetId());

#ifndef __WXMSW__
    CheckBox_KeepScreenOn->Enable(false);
#endif

    ValidateWindow();
}

void OptionsDialog::LoadButtons()
{
    ListView_Buttons->DeleteAllItems();
    auto ps = _options->GetButtons();
    int i = 0;
    for (const auto& it : ps) {
        ListView_Buttons->InsertItem(i, it->GetLabel());
        ListView_Buttons->SetItem(i, 1, it->GetCommand());
        ListView_Buttons->SetItem(i, 2, it->GetParameters());
        if (it->GetHotkey() == '~') {
            ListView_Buttons->SetItem(i, 3, "");
        }
        else {
            ListView_Buttons->SetItem(i, 3, it->GetHotkey());
        }
        ListView_Buttons->SetItem(i, 4, it->GetColorName());
        i++;
    }
}

OptionsDialog::~OptionsDialog()
{
	//(*Destroy(OptionsDialog)
	//*)
}

void OptionsDialog::OnButton_OkClick(wxCommandEvent& event)
{
    #ifdef __WXMSW__
    SetSuppressDarkMode(CheckBoxSuppressDarkMode->GetValue());
    #endif

    _options->SetSync(CheckBox_Sync->GetValue());
    _options->SetSendOffWhenNotRunning(CheckBox_SendOffWhenNotRunning->GetValue());
    _options->SetParallelTransmission(CheckBox_MultithreadedTransmission->GetValue());
    _options->SetHardwareAcceleratedVideo(CheckBox_HWAcceleratedVideo->GetValue());
    _options->SetRetryOutputOpen(CheckBox_RetryOpen->GetValue());
    _options->SetSendBackgroundWhenNotRunning(CheckBox_RunBackground->GetValue());
    _options->SetWebServerPort(SpinCtrl_WebServerPort->GetValue());
    _options->SetWWWRoot(TextCtrl_wwwRoot->GetValue().ToStdString());
    _options->SetAPIOnly(CheckBox_APIOnly->GetValue());
    _options->SetPassword(TextCtrl_Password->GetValue().ToStdString());
    _options->SetDefaultPage(TextCtrl_DefaultPage->GetValue().ToStdString());
    _options->SetAllowUnauth(CheckBox_AlllowPageBypass->GetValue());
    _options->SetPasswordTimeout(SpinCtrl_PasswordTimeout->GetValue());
    _options->SetAdvancedMode(CheckBox_SimpleMode->GetValue());
    _options->SetArtNetTimeCodeFormat(static_cast<TIMECODEFORMAT>(Choice_ARTNetTimeCodeFormat->GetSelection()));
    _options->SetCity(Choice_Location->GetStringSelection().ToStdString());
    _options->SetCrashBehaviour(Choice_OnCrash->GetStringSelection().ToStdString());
    _options->SetRemoteAllOff(CheckBox_RemoteAllOff->GetValue());
    _options->SetKeepScreenOn(CheckBox_KeepScreenOn->GetValue());
    _options->SetMinimiseUIUpdates(CheckBox_MinimiseUI->GetValue());
    _options->SetSuppressAudioOnRemotes(CheckBox_SuppressAudioOnRemotes->GetValue());
    _options->SetLateStartingScheduleUsesTime(CheckBox_LastStartingSequenceUsesTime->GetValue());
    _options->SetDisableOutputOnPingFailure(CheckBox_DisableOutputOnPingFailure->GetValue());
    _options->SetSMPTEMode(_options->EncodeSMPTEMode(Choice_SMPTEFrameRate->GetStringSelection()));
    _options->SetSMPTEDevice(Choice_SMPTEDevice->GetStringSelection());
    _options->SetStepMMSSTimecodeFormat(CheckBox_SongMMSSFormat->GetValue());
    _options->SetRemoteTimecodeStepAdvance(CheckBox_TimecodeWaitForNextSong->GetValue());

    if (Choice_AudioDevice->GetStringSelection() == "(Default)") {
        _options->SetAudioDevice("");
        AudioManager::GetSDLManager()->SetDefaultOutput("");
    }
    else {
        _options->SetAudioDevice(Choice_AudioDevice->GetStringSelection().ToStdString());
        AudioManager::GetSDLManager()->SetDefaultOutput(Choice_AudioDevice->GetStringSelection().ToStdString());
    }

    if (Choice_InputAudioDevice->GetStringSelection() == "(Default)") {
        _options->SetInputAudioDevice("");
        AudioManager::GetSDLManager()->SetDefaultInput("");
    }
    else {
        _options->SetInputAudioDevice(Choice_InputAudioDevice->GetStringSelection().ToStdString());
        AudioManager::GetSDLManager()->SetDefaultInput(Choice_InputAudioDevice->GetStringSelection().ToStdString());
    }

    _options->ClearButtons();
    for (int i = 0; i < ListView_Buttons->GetItemCount(); i++) {
        char hotkey = '~';
        if (ListView_Buttons->GetItemText(i, 3).Length() > 0) {
            hotkey = ListView_Buttons->GetItemText(i, 3)[0];
        }
        _options->AddButton(ListView_Buttons->GetItemText(i, 0).ToStdString(),
            ListView_Buttons->GetItemText(i, 1).ToStdString(),
            ListView_Buttons->GetItemText(i, 2).ToStdString(),
            hotkey,
            ListView_Buttons->GetItemText(i, 4).ToStdString(),
            _commandManager);
    }

    if (Choice1->GetStringSelection() != ((xScheduleFrame*)GetParent())->GetScheduleManager()->GetForceLocalIP()) {
        ((xScheduleFrame*)GetParent())->GetScheduleManager()->SetForceLocalIP(Choice1->GetStringSelection());
        wxMessageBox("Local IP changed. If you are outputting to lights please stop and restart light output");
    }

    EndDialog(wxID_OK);
}

void OptionsDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void OptionsDialog::OnButton_ButtonAddClick(wxCommandEvent& event)
{
    std::string label = "";
    std::string command = "";
    std::string parameter = "";
    std::string color = "default";
    char hotkey = '~';

    ButtonDetailsDialog dlg(this, _commandManager, label, command, color, parameter, hotkey);

    if (dlg.ShowModal() == wxID_OK) {
        int row = ListView_Buttons->GetItemCount();
        ListView_Buttons->InsertItem(row, label);
        ListView_Buttons->SetItem(row, 1, command);
        ListView_Buttons->SetItem(row, 2, parameter);
        ListView_Buttons->SetItem(row, 3, hotkey);
        ListView_Buttons->SetItem(row, 4, color);
    }

    ValidateWindow();
}

void OptionsDialog::OnButton_ButtonEditClick(wxCommandEvent& event)
{
    if (ListView_Buttons->GetSelectedItemCount() != 1) return;

    int row = ListView_Buttons->GetFirstSelected();

    EditButton(row);
}

void OptionsDialog::EditButton(int row)
{
    std::string label = ListView_Buttons->GetItemText(row, 0).ToStdString();
    std::string command = ListView_Buttons->GetItemText(row, 1).ToStdString();
    std::string parameter = ListView_Buttons->GetItemText(row, 2).ToStdString();
    char hotkey = '~';
    if (ListView_Buttons->GetItemText(row, 3).Length() > 0) {
        hotkey = ListView_Buttons->GetItemText(row, 3)[0];
    }
    std::string color = ListView_Buttons->GetItemText(row, 4).ToStdString();

    ButtonDetailsDialog dlg(this, _commandManager, label, command, color, parameter, hotkey);

    if (dlg.ShowModal() == wxID_OK) {
        ListView_Buttons->SetItemText(row, label);
        ListView_Buttons->SetItem(row, 1, command);
        ListView_Buttons->SetItem(row, 2, parameter);
        ListView_Buttons->SetItem(row, 3, hotkey);
        ListView_Buttons->SetItem(row, 4, color);
    }

    ValidateWindow();
}

void OptionsDialog::OnButton_ButtonDeleteClick(wxCommandEvent& event)
{
    if (ListView_Buttons->GetSelectedItemCount() != 1) return;

    ListView_Buttons->DeleteItem(ListView_Buttons->GetFirstSelected());

    ValidateWindow();
}

void OptionsDialog::ValidateWindow()
{
    if (ListView_Buttons->GetSelectedItemCount() == 1) {
        Button_ButtonDelete->Enable(true);
        Button_ButtonEdit->Enable(true);
        Button_Export->Enable(true);
    }
    else {
        Button_ButtonDelete->Enable(false);
        Button_ButtonEdit->Enable(false);
        Button_Export->Enable(false);
    }

    if (TextCtrl_wwwRoot->GetValue() == "") {
        Button_Ok->Enable(false);
    }
    else {
        Button_Ok->Enable();
    }
}

void OptionsDialog::OnTextCtrl_wwwRootText(wxCommandEvent& event)
{
    ValidateWindow();
}

void OptionsDialog::OnListView_ButtonsBeginDrag(wxListEvent& event)
{
    if (ListView_Buttons->GetSelectedItemCount() != 1) return;

    ListView_Buttons->Connect(wxEVT_LEFT_UP, wxMouseEventHandler(OptionsDialog::OnButtonsDragEnd), nullptr, this);
    // trigger when user leaves window to abort drag
    ListView_Buttons->Connect(wxEVT_LEAVE_WINDOW, wxMouseEventHandler(OptionsDialog::OnButtonsDragQuit), nullptr, this);
    // trigger when mouse moves
    ListView_Buttons->Connect(wxEVT_MOTION, wxMouseEventHandler(OptionsDialog::OnButtonsMouseMove), nullptr, this);

    _dragging = true;
    SetCursor(wxCURSOR_HAND);
}

// abort dragging a list item because user has left window
void OptionsDialog::OnButtonsDragQuit(wxMouseEvent& event)
{
    // restore cursor and disconnect unconditionally
    SetCursor(wxCURSOR_ARROW);
    ListView_Buttons->Disconnect(wxEVT_LEFT_UP, wxMouseEventHandler(OptionsDialog::OnButtonsDragEnd));
    ListView_Buttons->Disconnect(wxEVT_LEAVE_WINDOW, wxMouseEventHandler(OptionsDialog::OnButtonsDragQuit));
    ListView_Buttons->Disconnect(wxEVT_MOTION, wxMouseEventHandler(OptionsDialog::OnButtonsMouseMove));
    //HighlightDropItem(nullptr);
}

void OptionsDialog::OnButtonsDragEnd(wxMouseEvent& event)
{
    int flags = wxLIST_HITTEST_ONITEM;
    int dropitem = ListView_Buttons->HitTest(event.GetPosition(), flags);

    if (dropitem >= 0 && dropitem < ListView_Buttons->GetItemCount()) {
        int dragitem = ListView_Buttons->GetFirstSelected();

        if (dragitem >= 0 && dragitem < ListView_Buttons->GetItemCount()) {
            // move drag item below drop item
            if (dragitem != dropitem) {
                std::string label = ListView_Buttons->GetItemText(dragitem, 0).ToStdString();
                std::string command = ListView_Buttons->GetItemText(dragitem, 1).ToStdString();
                std::string parameters = ListView_Buttons->GetItemText(dragitem, 2).ToStdString();
                std::string hotkey = ListView_Buttons->GetItemText(dragitem, 3).ToStdString();
                std::string color = ListView_Buttons->GetItemText(dragitem, 4).ToStdString();

                ListView_Buttons->DeleteItem(dragitem);

                if (dropitem < 0) dropitem = 0;

                ListView_Buttons->InsertItem(dropitem, label);
                ListView_Buttons->SetItem(dropitem, 1, command);
                ListView_Buttons->SetItem(dropitem, 2, parameters);
                ListView_Buttons->SetItem(dropitem, 3, hotkey);
                ListView_Buttons->SetItem(dropitem, 4, color);

                ListView_Buttons->EnsureVisible(dropitem);

                if (dropitem == ListView_Buttons->GetItemCount() - 1) {
                    ListView_Buttons->ScrollLines(1);
                }
            }
        }
    }

    _dragging = false;
    SetCursor(wxCURSOR_ARROW);
    //HighlightDropItem(nullptr);

    // disconnect both functions
    ListView_Buttons->Disconnect(wxEVT_LEFT_UP, wxMouseEventHandler(OptionsDialog::OnButtonsDragEnd));
    ListView_Buttons->Disconnect(wxEVT_LEAVE_WINDOW, wxMouseEventHandler(OptionsDialog::OnButtonsDragQuit));
    ListView_Buttons->Disconnect(wxEVT_MOTION, wxMouseEventHandler(OptionsDialog::OnButtonsMouseMove));
}

void OptionsDialog::OnButtonsMouseMove(wxMouseEvent& event)
{
    int flags = wxLIST_HITTEST_ONITEM;
    int dropitem = ListView_Buttons->HitTest(event.GetPosition(), flags);
    int topitem = ListView_Buttons->GetTopItem();
    int bottomitem = topitem + ListView_Buttons->GetCountPerPage();

    if (topitem == dropitem && topitem > 0) {
        ListView_Buttons->EnsureVisible(topitem - 1);
    }
    else if (bottomitem == dropitem && dropitem < ListView_Buttons->GetItemCount() - 1) {
        ListView_Buttons->EnsureVisible(bottomitem + 1);
    }

    if (dropitem == ListView_Buttons->GetItemCount() - 1) {
        ListView_Buttons->ScrollLines(1);
    }

    SetCursor(wxCURSOR_HAND);
    //HighlightDropItem(&dropitem);
}

void OptionsDialog::OnListView_ButtonsItemSelect(wxListEvent& event)
{
    ValidateWindow();
}

void OptionsDialog::OnListView_ButtonsItemActivated(wxListEvent& event)
{
    if (ListView_Buttons->GetSelectedItemCount() >= 0) {
        EditButton(ListView_Buttons->GetFirstSelected());
    }
    ValidateWindow();
}

void OptionsDialog::OnListView_ButtonsKeyDown(wxListEvent& event)
{
    if (event.GetKeyCode() == WXK_DELETE) {
        if (ListView_Buttons->GetSelectedItemCount() >= 0) {
            ListView_Buttons->DeleteItem(ListView_Buttons->GetFirstSelected());
        }
    }
    ValidateWindow();
}

void OptionsDialog::OnButton_ImportClick(wxCommandEvent& event)
{
    wxFileDialog dlg(this, "Load button", wxEmptyString, wxEmptyString, "xSchedule Button Files (*.xbutton)|*.xbutton|All Files (*.)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dlg.ShowModal() == wxID_OK) {
        wxXmlDocument doc;
        if (doc.Load(dlg.GetPath())) {
            wxXmlNode* n = doc.GetRoot();
            if (n->GetName().Lower() == "xschedulebutton") {
                int row = ListView_Buttons->GetItemCount();
                ListView_Buttons->InsertItem(row, n->GetAttribute("label"));
                ListView_Buttons->SetItem(row, 1, n->GetAttribute("command"));
                ListView_Buttons->SetItem(row, 2, n->GetAttribute("parameter"));
                if (n->GetAttribute("hotkey") != "~") {
                    ListView_Buttons->SetItem(row, 3, n->GetAttribute("hotkey"));
                }
                ListView_Buttons->SetItem(row, 4, n->GetAttribute("colour"));
            }
        }
        else {
            wxMessageBox("Error loading button file.");
        }
    }
}

void OptionsDialog::OnButton_ExportClick(wxCommandEvent& event)
{
    if (ListView_Buttons->GetSelectedItemCount() != 1) return;

    int row = ListView_Buttons->GetFirstSelected();

    std::string label = ListView_Buttons->GetItemText(row, 0).ToStdString();
    std::string command = ListView_Buttons->GetItemText(row, 1).ToStdString();
    std::string parameter = ListView_Buttons->GetItemText(row, 2).ToStdString();
    char hotkey = '~';
    if (ListView_Buttons->GetItemText(row, 3).Length() > 0) {
        hotkey = ListView_Buttons->GetItemText(row, 3)[0];
    }
    std::string color = ListView_Buttons->GetItemText(row, 4).ToStdString();
    wxString v = xlights_version_string;

    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, label, wxEmptyString, "xSchedule Button files (*.xbutton)|*.xbutton", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (filename.IsEmpty()) return;

    wxFile f(filename);
    //    bool isnew = !wxFile::Exists(filename);
    if (!f.Create(filename, true) || !f.IsOpened()) {
        wxMessageBox(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()));
        return;
    }

    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xschedulebutton \n");
    f.Write(wxString::Format("label=\"%s\" ", label));
    f.Write(wxString::Format("command=\"%s\" ", command));
    f.Write(wxString::Format("parameter=\"%s\" ", parameter));
    f.Write(wxString::Format("hotkey=\"%c\" ", hotkey));
    f.Write(wxString::Format("colour=\"%s\" ", color));
    f.Write(wxString::Format("version=\"%s\" ", v));
    f.Write("/>");
    f.Close();
}

void OptionsDialog::OnButton_DefaultWindowLocationClick(wxCommandEvent& event)
{
    VideoWindowPositionDialog dlg(this, _options, wxID_ANY, _options->GetDefaultVideoPos(), _options->GetDefaultVideoSize());

    if (dlg.ShowModal() == wxID_OK) {
        _options->SetDefaultVideoPos(dlg.GetDesiredPosition());
        _options->SetDefaultVideoSize(dlg.GetDesiredSize());
    }
}
