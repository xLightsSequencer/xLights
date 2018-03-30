#include "EventMIDIPanel.h"
#include "events/EventMIDI.h"

//(*InternalHeaders(EventMIDIPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(EventMIDIPanel)
const long EventMIDIPanel::ID_STATICTEXT1 = wxNewId();
const long EventMIDIPanel::ID_CHOICE1 = wxNewId();
const long EventMIDIPanel::ID_STATICTEXT2 = wxNewId();
const long EventMIDIPanel::ID_CHOICE2 = wxNewId();
const long EventMIDIPanel::ID_STATICTEXT4 = wxNewId();
const long EventMIDIPanel::ID_CHOICE4 = wxNewId();
const long EventMIDIPanel::ID_STATICTEXT3 = wxNewId();
const long EventMIDIPanel::ID_CHOICE3 = wxNewId();
//*)

BEGIN_EVENT_TABLE(EventMIDIPanel,wxPanel)
	//(*EventTable(EventMIDIPanel)
	//*)
END_EVENT_TABLE()

EventMIDIPanel::EventMIDIPanel(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(EventMIDIPanel)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Device:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Devices = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	FlexGridSizer1->Add(Choice_Devices, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Status:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Status = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
	FlexGridSizer1->Add(Choice_Status, 1, wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Channel:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Channel = new wxChoice(this, ID_CHOICE4, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE4"));
	FlexGridSizer1->Add(Choice_Channel, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Data 1:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Data1 = new wxChoice(this, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE3"));
	FlexGridSizer1->Add(Choice_Data1, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	//*)

    auto devices = EventMIDI::GetDevices();
    for (auto it = devices.begin(); it != devices.end(); ++it)
    {
        Choice_Devices->Append(*it);
    }
    Choice_Devices->SetSelection(0);

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
    Choice_Channel->Append("ANY");
    Choice_Channel->SetSelection(0);

    for (int i = 0; i < 256; i++)
    {
        Choice_Data1->Append(wxString::Format("0x%02X", i));
    }
    Choice_Data1->Append("ANY");
    Choice_Data1->SetSelection(0);
}

EventMIDIPanel::~EventMIDIPanel()
{
	//(*Destroy(EventMIDIPanel)
	//*)
}

bool EventMIDIPanel::ValidateWindow()
{
    return Choice_Devices->GetStringSelection() != "";
}

void EventMIDIPanel::Save(EventBase* event)
{
    EventMIDI* e = (EventMIDI*)event;
    e->SetDevice(Choice_Devices->GetStringSelection().ToStdString());
    e->SetStatus(Choice_Status->GetStringSelection().ToStdString());
    e->SetChannel(Choice_Channel->GetStringSelection().ToStdString());
    e->SetData1(Choice_Data1->GetStringSelection().ToStdString());
}

void EventMIDIPanel::Load(EventBase* event)
{
    EventMIDI* e = (EventMIDI*)event;
    Choice_Devices->SetStringSelection(e->GetDevice());
    Choice_Status->SetStringSelection(e->GetStatus());
    Choice_Channel->SetStringSelection(e->GetChannel());
    Choice_Data1->SetStringSelection(e->GetData1());
}
