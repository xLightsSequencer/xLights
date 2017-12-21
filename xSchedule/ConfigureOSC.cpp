#include "ConfigureOSC.h"
#include "ScheduleOptions.h"
#include "../xLights/outputs/IPOutput.h"

//(*InternalHeaders(ConfigureOSC)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(ConfigureOSC)
const long ConfigureOSC::ID_STATICTEXT4 = wxNewId();
const long ConfigureOSC::ID_RADIOBUTTON1 = wxNewId();
const long ConfigureOSC::ID_CHOICE1 = wxNewId();
const long ConfigureOSC::ID_RADIOBUTTON2 = wxNewId();
const long ConfigureOSC::ID_CHOICE2 = wxNewId();
const long ConfigureOSC::ID_STATICTEXT1 = wxNewId();
const long ConfigureOSC::ID_STATICTEXT5 = wxNewId();
const long ConfigureOSC::ID_TEXTCTRL2 = wxNewId();
const long ConfigureOSC::ID_CHECKBOX1 = wxNewId();
const long ConfigureOSC::ID_STATICTEXT3 = wxNewId();
const long ConfigureOSC::ID_TEXTCTRL1 = wxNewId();
const long ConfigureOSC::ID_STATICTEXT6 = wxNewId();
const long ConfigureOSC::ID_SPINCTRL1 = wxNewId();
const long ConfigureOSC::ID_STATICTEXT2 = wxNewId();
const long ConfigureOSC::ID_STATICTEXT8 = wxNewId();
const long ConfigureOSC::ID_CHOICE3 = wxNewId();
const long ConfigureOSC::ID_STATICTEXT7 = wxNewId();
const long ConfigureOSC::ID_SPINCTRL2 = wxNewId();
const long ConfigureOSC::ID_BUTTON1 = wxNewId();
const long ConfigureOSC::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(ConfigureOSC,wxDialog)
	//(*EventTable(ConfigureOSC)
	//*)
END_EVENT_TABLE()

ConfigureOSC::ConfigureOSC(wxWindow* parent, OSCOptions* oscOptions,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _oscOptions = oscOptions;

	//(*Initialize(ConfigureOSC)
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer6;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, _("Open Sound Control Configuration"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Message Content"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer4->AddGrowableCol(0);
	RadioButton_Time = new wxRadioButton(this, ID_RADIOBUTTON1, _("Time"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON1"));
	RadioButton_Time->SetValue(true);
	FlexGridSizer4->Add(RadioButton_Time, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Time = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	Choice_Time->SetSelection( Choice_Time->Append(_("Seconds (float)")) );
	Choice_Time->Append(_("Milliseconds (int)"));
	FlexGridSizer4->Add(Choice_Time, 1, wxALL|wxEXPAND, 5);
	RadioButton_Frames = new wxRadioButton(this, ID_RADIOBUTTON2, _("Frames"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON2"));
	FlexGridSizer4->Add(RadioButton_Frames, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Frames = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
	Choice_Frames->SetSelection( Choice_Frames->Append(_("Default (int)")) );
	Choice_Frames->Append(_("24 fps (int)"));
	Choice_Frames->Append(_("25 fps (int)"));
	Choice_Frames->Append(_("29.97 fps (int)"));
	Choice_Frames->Append(_("30 fps (int)"));
	Choice_Frames->Append(_("60 fps (int)"));
	Choice_Frames->Append(_("Progress (float)"));
	FlexGridSizer4->Add(Choice_Frames, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Master"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(1);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Path"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer2->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_MasterPath = new wxTextCtrl(this, ID_TEXTCTRL2, _("/Timecode/%STEPNAME%"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer2->Add(TextCtrl_MasterPath, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_ServerBroadcast = new wxCheckBox(this, ID_CHECKBOX1, _("Broadcast"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_ServerBroadcast->SetValue(true);
	FlexGridSizer2->Add(CheckBox_ServerBroadcast, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("IP Address"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer2->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_ServerIP = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer2->Add(TextCtrl_ServerIP, 1, wxALL|wxEXPAND, 5);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Port"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer2->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_ServerPort = new wxSpinCtrl(this, ID_SPINCTRL1, _T("9000"), wxDefaultPosition, wxDefaultSize, 0, 1, 65535, 9000, _T("ID_SPINCTRL1"));
	SpinCtrl_ServerPort->SetValue(_T("9000"));
	FlexGridSizer2->Add(SpinCtrl_ServerPort, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Remote"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer3->AddGrowableCol(1);
	StaticText8 = new wxStaticText(this, ID_STATICTEXT8, _("Path"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
	FlexGridSizer3->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_RemotePath = new wxChoice(this, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE3"));
	Choice_RemotePath->SetSelection( Choice_RemotePath->Append(_("/Timecode/%STEPNAME%")) );
	Choice_RemotePath->Append(_("/Timecode/%TIMINGITEM%"));
	FlexGridSizer3->Add(Choice_RemotePath, 1, wxALL|wxEXPAND, 5);
	StaticText7 = new wxStaticText(this, ID_STATICTEXT7, _("Port"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	FlexGridSizer3->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_RemotePort = new wxSpinCtrl(this, ID_SPINCTRL2, _T("9000"), wxDefaultPosition, wxDefaultSize, 0, 1, 65535, 9000, _T("ID_SPINCTRL2"));
	SpinCtrl_RemotePort->SetValue(_T("9000"));
	FlexGridSizer3->Add(SpinCtrl_RemotePort, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
	Button_ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer6->Add(Button_ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer6->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_RADIOBUTTON1,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&ConfigureOSC::OnRadioButton_TimeSelect);
	Connect(ID_RADIOBUTTON2,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&ConfigureOSC::OnRadioButton_FramesSelect);
	Connect(ID_TEXTCTRL2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ConfigureOSC::OnTextCtrl_PathText);
	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ConfigureOSC::OnCheckBox_ServerBroadcastClick);
	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ConfigureOSC::OnTextCtrl_ServerIPText);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ConfigureOSC::OnButton_okClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ConfigureOSC::OnButton_CancelClick);
	//*)

    CheckBox_ServerBroadcast->SetValue(_oscOptions->IsBroadcast());
    TextCtrl_MasterPath->SetValue(_oscOptions->GetMasterPath());
    Choice_RemotePath->SetStringSelection(_oscOptions->GetRemotePath());
    TextCtrl_ServerIP->SetValue(_oscOptions->GetIPAddress());
    SpinCtrl_ServerPort->SetValue(_oscOptions->GetServerPort());
    SpinCtrl_RemotePort->SetValue(_oscOptions->GetClientPort());
    if (_oscOptions->IsTime())
    {
        RadioButton_Time->SetValue(true);
        RadioButton_Frames->SetValue(false);
    }
    else
    {
        RadioButton_Frames->SetValue(true);
        RadioButton_Time->SetValue(false);
    }
    Choice_Time->SetStringSelection(_oscOptions->GetTime());
    Choice_Frames->SetStringSelection(_oscOptions->GetFrame());

    ValidateWindow();
}

ConfigureOSC::~ConfigureOSC()
{
	//(*Destroy(ConfigureOSC)
	//*)
}


void ConfigureOSC::OnTextCtrl_PathText(wxCommandEvent& event)
{
    ValidateWindow();
}

void ConfigureOSC::OnRadioButton_TimeSelect(wxCommandEvent& event)
{
    ValidateWindow();
}

void ConfigureOSC::OnRadioButton_FramesSelect(wxCommandEvent& event)
{
    ValidateWindow();
}

void ConfigureOSC::OnCheckBox_ServerBroadcastClick(wxCommandEvent& event)
{
    ValidateWindow();
}

void ConfigureOSC::OnTextCtrl_ServerIPText(wxCommandEvent& event)
{
    ValidateWindow();
}

void ConfigureOSC::OnButton_okClick(wxCommandEvent& event)
{
    _oscOptions->SetMasterPath(TextCtrl_MasterPath->GetValue().ToStdString());
    _oscOptions->SetRemotePath(Choice_RemotePath->GetStringSelection().ToStdString());
    _oscOptions->SetIPAddress(TextCtrl_ServerIP->GetValue().ToStdString());
    if (CheckBox_ServerBroadcast->GetValue()) _oscOptions->SetBroadcast();
    _oscOptions->SetServerPort(SpinCtrl_ServerPort->GetValue());
    _oscOptions->SetClientPort(SpinCtrl_RemotePort->GetValue());
    if (RadioButton_Time->GetValue())
    {
        _oscOptions->SetTime();
    }
    else
    {
        _oscOptions->SetFrames();
    }
    _oscOptions->SetTime(Choice_Time->GetStringSelection().ToStdString());
    _oscOptions->SetFrame(Choice_Frames->GetStringSelection().ToStdString());
    EndDialog(wxID_OK);
}

void ConfigureOSC::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void ConfigureOSC::ValidateWindow()
{
    if (CheckBox_ServerBroadcast->GetValue())
    {
        TextCtrl_ServerIP->Enable(false);
    }
    else
    {
        TextCtrl_ServerIP->Enable(true);
    }

    if (RadioButton_Time->GetValue())
    {
        Choice_Time->Enable(true);
        Choice_Frames->Enable(false);
    }
    else
    {
        Choice_Time->Enable(false);
        Choice_Frames->Enable(true);
    }

    if (!IsValidPath() ||
        !CheckBox_ServerBroadcast->GetValue() && !IPOutput::IsIPValid(TextCtrl_ServerIP->GetValue().ToStdString()))
    {
        Button_ok->Enable(false);
    }
    else
    {
        Button_ok->Enable(true);
    }
}

bool ConfigureOSC::IsValidPath() const
{
    // to be valid it must start with /
    // must not end with /
    return TextCtrl_MasterPath->GetValue().StartsWith("/") && !TextCtrl_MasterPath->GetValue().EndsWith("/");
}
