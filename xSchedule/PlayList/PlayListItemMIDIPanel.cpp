/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "PlayListItemMIDIPanel.h"
#include "PlayListDialog.h"
#include "PlayListItemMIDI.h"

//(*InternalHeaders(PlayListItemMIDIPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(PlayListItemMIDIPanel)
const long PlayListItemMIDIPanel::ID_STATICTEXT5 = wxNewId();
const long PlayListItemMIDIPanel::ID_TEXTCTRL3 = wxNewId();
const long PlayListItemMIDIPanel::ID_STATICTEXT3 = wxNewId();
const long PlayListItemMIDIPanel::ID_CHOICE4 = wxNewId();
const long PlayListItemMIDIPanel::ID_STATICTEXT1 = wxNewId();
const long PlayListItemMIDIPanel::ID_CHOICE3 = wxNewId();
const long PlayListItemMIDIPanel::ID_STATICTEXT4 = wxNewId();
const long PlayListItemMIDIPanel::ID_CHOICE5 = wxNewId();
const long PlayListItemMIDIPanel::ID_STATICTEXT6 = wxNewId();
const long PlayListItemMIDIPanel::ID_CHOICE1 = wxNewId();
const long PlayListItemMIDIPanel::ID_STATICTEXT7 = wxNewId();
const long PlayListItemMIDIPanel::ID_CHOICE2 = wxNewId();
const long PlayListItemMIDIPanel::ID_STATICTEXT2 = wxNewId();
const long PlayListItemMIDIPanel::ID_TEXTCTRL2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PlayListItemMIDIPanel,wxPanel)
	//(*EventTable(PlayListItemMIDIPanel)
	//*)
END_EVENT_TABLE()

PlayListItemMIDIPanel::PlayListItemMIDIPanel(wxWindow* parent, PlayListItemMIDI* MIDI, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _MIDI = MIDI;

	//(*Initialize(PlayListItemMIDIPanel)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer1->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_MIDIName = new wxTextCtrl(this, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL3"));
	FlexGridSizer1->Add(TextCtrl_MIDIName, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Device:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Devices = new wxChoice(this, ID_CHOICE4, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE4"));
	FlexGridSizer1->Add(Choice_Devices, 1, wxALL|wxEXPAND, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Status:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Status = new wxChoice(this, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE3"));
	FlexGridSizer1->Add(Choice_Status, 1, wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Channel:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Channel = new wxChoice(this, ID_CHOICE5, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE5"));
	FlexGridSizer1->Add(Choice_Channel, 1, wxALL|wxEXPAND, 5);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Data 1:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer1->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Data1 = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	FlexGridSizer1->Add(Choice_Data1, 1, wxALL|wxEXPAND, 5);
	StaticText7 = new wxStaticText(this, ID_STATICTEXT7, _("Data 2:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	FlexGridSizer1->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Data2 = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
	FlexGridSizer1->Add(Choice_Data2, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Delay:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Delay = new wxTextCtrl(this, ID_TEXTCTRL2, _("0.000"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer1->Add(TextCtrl_Delay, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL3,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemMIDIPanel::OnTextCtrl_MIDINameText);
	Connect(ID_TEXTCTRL2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemMIDIPanel::OnTextCtrl_DelayText);
	//*)

    for (auto device : PlayListItemMIDI::GetDevices())
    {
        Choice_Devices->Append(device);
    }
    if (Choice_Devices->GetCount() > 0)
    {
        Choice_Devices->SetSelection(0);
    }

    Choice_Status->Append("0x8n - Note Off");
    Choice_Status->Append("0x9n - Note On");
    Choice_Status->Append("0xAn - Polyphonic Key Pressure");
    Choice_Status->Append("0xBn - Control Change");
    Choice_Status->Append("0xCn - Program Change");
    Choice_Status->Append("0xDn - Channel Pressure");
    Choice_Status->Append("0xEn - Pitch Bend");

    for (int i = 0; i < 16; i++)
    {
        Choice_Channel->Append(wxString::Format("0x0%X", i));
    }
    Choice_Channel->SetSelection(0);

    for (int i = 0; i < 256; i++)
    {
        Choice_Data1->Append(wxString::Format("0x%02X", i));
        Choice_Data2->Append(wxString::Format("0x%02X", i));
    }
    Choice_Data1->SetSelection(0);
    Choice_Data2->SetSelection(0);

    TextCtrl_MIDIName->SetValue(MIDI->GetRawName());
    Choice_Devices->SetStringSelection(MIDI->GetDevice());
    Choice_Status->SetStringSelection(MIDI->GetStatus());
    Choice_Data1->SetStringSelection(MIDI->GetData1());
    Choice_Data2->SetStringSelection(MIDI->GetData2());
    Choice_Channel->SetStringSelection(MIDI->GetChannel());
    TextCtrl_Delay->SetValue(wxString::Format(wxT("%.3f"), (float)MIDI->GetDelay() / 1000.0));
}

PlayListItemMIDIPanel::~PlayListItemMIDIPanel()
{
	//(*Destroy(PlayListItemMIDIPanel)
	//*)
    _MIDI->SetName(TextCtrl_MIDIName->GetValue().ToStdString());
    _MIDI->SetDevice(Choice_Devices->GetStringSelection().ToStdString());
    _MIDI->SetStatus(Choice_Status->GetStringSelection().ToStdString());
    _MIDI->SetData1(Choice_Data1->GetStringSelection().ToStdString());
    _MIDI->SetData2(Choice_Data2->GetStringSelection().ToStdString());
    _MIDI->SetChannel(Choice_Channel->GetStringSelection().ToStdString());
    _MIDI->SetDelay(wxAtof(TextCtrl_Delay->GetValue())*1000);
}

void PlayListItemMIDIPanel::OnTextCtrl_DelayText(wxCommandEvent& event)
{
}

void PlayListItemMIDIPanel::OnTextCtrl_MIDINameText(wxCommandEvent& event)
{
    _MIDI->SetName(TextCtrl_MIDIName->GetValue().ToStdString());
    wxCommandEvent e(EVT_UPDATEITEMNAME);
    wxPostEvent(GetParent()->GetParent()->GetParent()->GetParent(), e);
}
