/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

 //(*InternalHeaders(EventMIDIPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "EventMIDIPanel.h"
#include "events/EventMIDI.h"
#include "xScheduleMain.h"
#include "ScheduleManager.h"
#include "events/ListenerManager.h"
#include "EventDialog.h"

//(*IdInit(EventMIDIPanel)
const long EventMIDIPanel::ID_STATICTEXT1 = wxNewId();
const long EventMIDIPanel::ID_CHOICE1 = wxNewId();
const long EventMIDIPanel::ID_STATICTEXT2 = wxNewId();
const long EventMIDIPanel::ID_CHOICE2 = wxNewId();
const long EventMIDIPanel::ID_STATICTEXT4 = wxNewId();
const long EventMIDIPanel::ID_CHOICE4 = wxNewId();
const long EventMIDIPanel::ID_STATICTEXT3 = wxNewId();
const long EventMIDIPanel::ID_CHOICE6 = wxNewId();
const long EventMIDIPanel::ID_SPINCTRL1 = wxNewId();
const long EventMIDIPanel::ID_STATICTEXT5 = wxNewId();
const long EventMIDIPanel::ID_CHOICE7 = wxNewId();
const long EventMIDIPanel::ID_SPINCTRL2 = wxNewId();
const long EventMIDIPanel::ID_BUTTON1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(EventMIDIPanel,wxPanel)
	//(*EventTable(EventMIDIPanel)
	//*)
    EVT_COMMAND(wxID_ANY, EVT_MIDI, EventMIDIPanel::OnMIDIEvent)
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
	Choice_TestData1 = new wxChoice(this, ID_CHOICE6, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE6"));
	Choice_TestData1->Append(_("Equals"));
	Choice_TestData1->Append(_("Less Than"));
	Choice_TestData1->Append(_("Less Than or Equals"));
	Choice_TestData1->Append(_("Greater Than"));
	Choice_TestData1->Append(_("Greater Than or Equals"));
	Choice_TestData1->Append(_("Not Equals"));
	Choice_TestData1->Append(_("On Change"));
	Choice_TestData1->SetSelection( Choice_TestData1->Append(_("Any")) );
	FlexGridSizer1->Add(Choice_TestData1, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Data1 = new wxSpinCtrl(this, ID_SPINCTRL1, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 0, 127, 0, _T("ID_SPINCTRL1"));
	SpinCtrl_Data1->SetValue(_T("0"));
	FlexGridSizer1->Add(SpinCtrl_Data1, 1, wxALL|wxEXPAND, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Data 2:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer1->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_TestData2 = new wxChoice(this, ID_CHOICE7, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE7"));
	Choice_TestData2->Append(_("Equals"));
	Choice_TestData2->Append(_("Less Than"));
	Choice_TestData2->Append(_("Less Than or Equals"));
	Choice_TestData2->Append(_("Greater Than"));
	Choice_TestData2->Append(_("Greater Than or Equals"));
	Choice_TestData2->Append(_("Not Equals"));
	Choice_TestData2->Append(_("On Change"));
	Choice_TestData2->SetSelection( Choice_TestData2->Append(_("Any")) );
	FlexGridSizer1->Add(Choice_TestData2, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Data2 = new wxSpinCtrl(this, ID_SPINCTRL2, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 0, 127, 0, _T("ID_SPINCTRL2"));
	SpinCtrl_Data2->SetValue(_T("0"));
	FlexGridSizer1->Add(SpinCtrl_Data2, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Scan = new wxButton(this, ID_BUTTON1, _("Scan"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer1->Add(Button_Scan, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_CHOICE6,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&EventMIDIPanel::OnChoice_TestData1Select);
	Connect(ID_CHOICE7,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&EventMIDIPanel::OnChoice_TestData2Select);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EventMIDIPanel::OnButton_ScanClick);
	//*)

    for (auto device : EventMIDI::GetDevices())
    {
        Choice_Devices->Append(device);
    }
    if (Choice_Devices->GetCount() > 0)
    {
        Choice_Devices->SetSelection(0);
    }
    else
    {
        Button_Scan->Enable(false);
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
    Choice_Channel->Append("ANY");
    Choice_Channel->SetSelection(0);

    Choice_TestData1->SetStringSelection("Any");
    Choice_TestData2->SetStringSelection("Any");
    ValidateWindow();
}

EventMIDIPanel::~EventMIDIPanel()
{
	//(*Destroy(EventMIDIPanel)
	//*)
    xScheduleFrame::GetScheduleManager()->GetListenerManager()->MidiRedirect(nullptr, -1);
}

bool EventMIDIPanel::ValidateWindow()
{
    if (Choice_TestData1->GetStringSelection() == "Any" || Choice_TestData1->GetStringSelection() == "On Change")
    {
        SpinCtrl_Data1->Disable();
    }
    else
    {
        SpinCtrl_Data1->Enable();
    }
    if (Choice_TestData2->GetStringSelection() == "Any" || Choice_TestData2->GetStringSelection() == "On Change")
    {
        SpinCtrl_Data2->Disable();
    }
    else
    {
        SpinCtrl_Data2->Enable();
    }

    return Choice_Devices->GetStringSelection() != "" && Button_Scan->IsEnabled();
}

void EventMIDIPanel::Save(EventBase* event)
{
    EventMIDI* e = (EventMIDI*)event;
    e->SetDevice(Choice_Devices->GetStringSelection().ToStdString());
    e->SetStatus(Choice_Status->GetStringSelection().ToStdString());
    e->SetChannel(Choice_Channel->GetStringSelection().ToStdString());
    e->SetTestData1(Choice_TestData1->GetStringSelection());
    e->SetTestData2(Choice_TestData2->GetStringSelection());
    e->SetData1(SpinCtrl_Data1->GetValue());
    e->SetData2(SpinCtrl_Data2->GetValue());
}

void EventMIDIPanel::Load(EventBase* event)
{
    EventMIDI* e = (EventMIDI*)event;
    Choice_Devices->SetStringSelection(e->GetDevice());
    Choice_Status->SetStringSelection(e->GetStatus());
    Choice_Channel->SetStringSelection(e->GetChannel());
    Choice_TestData1->SetStringSelection(e->GetTestData1());
    Choice_TestData2->SetStringSelection(e->GetTestData2());
    SpinCtrl_Data1->SetValue(e->GetData1());
    SpinCtrl_Data2->SetValue(e->GetData2());
}

void EventMIDIPanel::OnButton_ScanClick(wxCommandEvent& event)
{
    xScheduleFrame::GetScheduleManager()->GetListenerManager()->MidiRedirect(this, wxAtoi(Choice_Devices->GetStringSelection().AfterLast(' ')));
    Button_Scan->Enable(false);
}

void EventMIDIPanel::OnMIDIEvent(wxCommandEvent& event)
{
    if (Button_Scan->IsEnabled()) {
        event.Skip();
        return;
    }

    Button_Scan->Enable();

    xScheduleFrame::GetScheduleManager()->GetListenerManager()->MidiRedirect(nullptr, -1);

    uint8_t status = (event.GetInt() >> 24) & 0xFF;
    uint8_t channel = (event.GetInt() >> 16) & 0xFF;
    uint8_t data1 = (event.GetInt() >> 8) & 0xFF;
    uint8_t data2 = event.GetInt() & 0xFF;

    Choice_Status->SetSelection(((status & 0xF0) >> 4) - 8);
    Choice_Channel->SetSelection(channel);
    Choice_TestData1->SetStringSelection("Equals");
    SpinCtrl_Data1->SetValue(data1);
    Choice_TestData2->SetStringSelection("Any");

    ((EventDialog*)GetParent()->GetParent()->GetParent()->GetParent())->ValidateWindow();
}

void EventMIDIPanel::OnChoice_TestData1Select(wxCommandEvent& event)
{
    ValidateWindow();
}

void EventMIDIPanel::OnChoice_TestData2Select(wxCommandEvent& event)
{
    ValidateWindow();
}