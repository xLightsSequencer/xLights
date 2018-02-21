#include "EventLorPanel.h"
#include "events/EventLor.h"
#include "../xLights/outputs/SerialOutput.h"

//(*InternalHeaders(EventLorPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(EventLorPanel)
const long EventLorPanel::ID_STATICTEXT1 = wxNewId();
const long EventLorPanel::ID_CHOICE_UNITID = wxNewId();
const long EventLorPanel::ID_STATICTEXT2 = wxNewId();
const long EventLorPanel::ID_CHOICE2 = wxNewId();
const long EventLorPanel::ID_STATICTEXT3 = wxNewId();
const long EventLorPanel::ID_CHOICE3 = wxNewId();
const long EventLorPanel::ID_STATICTEXT5 = wxNewId();
const long EventLorPanel::ID_SPINCTRL1 = wxNewId();
const long EventLorPanel::ID_STATICTEXT6 = wxNewId();
const long EventLorPanel::ID_CHOICE4 = wxNewId();
const long EventLorPanel::ID_STATICTEXT7 = wxNewId();
const long EventLorPanel::ID_SPINCTRL2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(EventLorPanel,wxPanel)
	//(*EventTable(EventLorPanel)
	//*)
END_EVENT_TABLE()

EventLorPanel::EventLorPanel(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(EventLorPanel)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Unit Id:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Choice_UnitId = new wxChoice(this, ID_CHOICE_UNITID, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_UNITID"));
	Choice_UnitId->Append(_("0x1"));
	Choice_UnitId->Append(_("0x2"));
	Choice_UnitId->Append(_("0x3"));
	Choice_UnitId->Append(_("0x4"));
	Choice_UnitId->Append(_("0x5"));
	Choice_UnitId->Append(_("0x6"));
	Choice_UnitId->Append(_("0x7"));
	Choice_UnitId->Append(_("0x8"));
	Choice_UnitId->Append(_("0x9"));
	Choice_UnitId->Append(_("0xA"));
	Choice_UnitId->Append(_("0xB"));
	Choice_UnitId->Append(_("0xC"));
	Choice_UnitId->Append(_("0xD"));
	Choice_UnitId->Append(_("0xE"));
	Choice_UnitId->Append(_("0xF"));
	Choice_UnitId->Append(_("0x10"));
	Choice_UnitId->Append(_("0x11"));
	Choice_UnitId->Append(_("0x12"));
	Choice_UnitId->Append(_("0x13"));
	Choice_UnitId->Append(_("0x14"));
	Choice_UnitId->Append(_("0x15"));
	Choice_UnitId->Append(_("0x16"));
	Choice_UnitId->Append(_("0x17"));
	Choice_UnitId->Append(_("0x18"));
	Choice_UnitId->Append(_("0x19"));
	Choice_UnitId->Append(_("0x1A"));
	Choice_UnitId->Append(_("0x1B"));
	Choice_UnitId->Append(_("0x1C"));
	Choice_UnitId->Append(_("0x1D"));
	Choice_UnitId->Append(_("0x1E"));
	Choice_UnitId->Append(_("0x1F"));
	Choice_UnitId->Append(_("0x20"));
	Choice_UnitId->Append(_("0x21"));
	Choice_UnitId->Append(_("0x22"));
	Choice_UnitId->Append(_("0x23"));
	Choice_UnitId->Append(_("0x24"));
	Choice_UnitId->Append(_("0x25"));
	Choice_UnitId->Append(_("0x26"));
	Choice_UnitId->Append(_("0x27"));
	Choice_UnitId->Append(_("0x28"));
	Choice_UnitId->Append(_("0x29"));
	Choice_UnitId->Append(_("0x2A"));
	Choice_UnitId->Append(_("0x2B"));
	Choice_UnitId->Append(_("0x2C"));
	Choice_UnitId->Append(_("0x2D"));
	Choice_UnitId->Append(_("0x2E"));
	Choice_UnitId->Append(_("0x2F"));
	Choice_UnitId->Append(_("0x30"));
	Choice_UnitId->Append(_("0x31"));
	Choice_UnitId->Append(_("0x32"));
	Choice_UnitId->Append(_("0x33"));
	Choice_UnitId->Append(_("0x34"));
	Choice_UnitId->Append(_("0x35"));
	Choice_UnitId->Append(_("0x36"));
	Choice_UnitId->Append(_("0x37"));
	Choice_UnitId->Append(_("0x38"));
	Choice_UnitId->Append(_("0x39"));
	Choice_UnitId->Append(_("0x3A"));
	Choice_UnitId->Append(_("0x3B"));
	Choice_UnitId->Append(_("0x3C"));
	Choice_UnitId->Append(_("0x3D"));
	Choice_UnitId->Append(_("0x3E"));
	Choice_UnitId->Append(_("0x3F"));
	Choice_UnitId->Append(_("0x40"));
	Choice_UnitId->Append(_("0x41"));
	Choice_UnitId->Append(_("0x42"));
	Choice_UnitId->Append(_("0x43"));
	Choice_UnitId->Append(_("0x44"));
	Choice_UnitId->Append(_("0x45"));
	Choice_UnitId->Append(_("0x46"));
	Choice_UnitId->Append(_("0x47"));
	Choice_UnitId->Append(_("0x48"));
	Choice_UnitId->Append(_("0x49"));
	Choice_UnitId->Append(_("0x4A"));
	Choice_UnitId->Append(_("0x4B"));
	Choice_UnitId->Append(_("0x4C"));
	Choice_UnitId->Append(_("0x4D"));
	Choice_UnitId->Append(_("0x4E"));
	Choice_UnitId->Append(_("0x4F"));
	Choice_UnitId->Append(_("0x50"));
	Choice_UnitId->Append(_("0x51"));
	Choice_UnitId->Append(_("0x52"));
	Choice_UnitId->Append(_("0x53"));
	Choice_UnitId->Append(_("0x54"));
	Choice_UnitId->Append(_("0x55"));
	Choice_UnitId->Append(_("0x56"));
	Choice_UnitId->Append(_("0x57"));
	Choice_UnitId->Append(_("0x58"));
	Choice_UnitId->Append(_("0x59"));
	Choice_UnitId->Append(_("0x5A"));
	Choice_UnitId->Append(_("0x5B"));
	Choice_UnitId->Append(_("0x5C"));
	Choice_UnitId->Append(_("0x5D"));
	Choice_UnitId->Append(_("0x5E"));
	Choice_UnitId->Append(_("0x5F"));
	Choice_UnitId->Append(_("0x60"));
	Choice_UnitId->Append(_("0x61"));
	Choice_UnitId->Append(_("0x62"));
	Choice_UnitId->Append(_("0x63"));
	Choice_UnitId->Append(_("0x64"));
	Choice_UnitId->Append(_("0x65"));
	Choice_UnitId->Append(_("0x66"));
	Choice_UnitId->Append(_("0x67"));
	Choice_UnitId->Append(_("0x68"));
	Choice_UnitId->Append(_("0x69"));
	Choice_UnitId->Append(_("0x6A"));
	Choice_UnitId->Append(_("0x6B"));
	Choice_UnitId->Append(_("0x6C"));
	Choice_UnitId->Append(_("0x6D"));
	Choice_UnitId->Append(_("0x6E"));
	Choice_UnitId->Append(_("0x6F"));
	Choice_UnitId->Append(_("0x70"));
	Choice_UnitId->Append(_("0x71"));
	Choice_UnitId->Append(_("0x72"));
	Choice_UnitId->Append(_("0x73"));
	Choice_UnitId->Append(_("0x74"));
	Choice_UnitId->Append(_("0x75"));
	Choice_UnitId->Append(_("0x76"));
	Choice_UnitId->Append(_("0x77"));
	Choice_UnitId->Append(_("0x78"));
	Choice_UnitId->Append(_("0x79"));
	Choice_UnitId->Append(_("0x7A"));
	Choice_UnitId->Append(_("0x7B"));
	Choice_UnitId->Append(_("0x7C"));
	Choice_UnitId->Append(_("0x7D"));
	Choice_UnitId->Append(_("0x7E"));
	Choice_UnitId->Append(_("0x7F"));
	Choice_UnitId->Append(_("0x80"));
	Choice_UnitId->Append(_("0x81"));
	Choice_UnitId->Append(_("0x82"));
	Choice_UnitId->Append(_("0x83"));
	Choice_UnitId->Append(_("0x84"));
	Choice_UnitId->Append(_("0x85"));
	Choice_UnitId->Append(_("0x86"));
	Choice_UnitId->Append(_("0x87"));
	Choice_UnitId->Append(_("0x88"));
	Choice_UnitId->Append(_("0x89"));
	Choice_UnitId->Append(_("0x8A"));
	Choice_UnitId->Append(_("0x8B"));
	Choice_UnitId->Append(_("0x8C"));
	Choice_UnitId->Append(_("0x8D"));
	Choice_UnitId->Append(_("0x8E"));
	Choice_UnitId->Append(_("0x8F"));
	Choice_UnitId->Append(_("0x90"));
	Choice_UnitId->Append(_("0x91"));
	Choice_UnitId->Append(_("0x92"));
	Choice_UnitId->Append(_("0x93"));
	Choice_UnitId->Append(_("0x94"));
	Choice_UnitId->Append(_("0x95"));
	Choice_UnitId->Append(_("0x96"));
	Choice_UnitId->Append(_("0x97"));
	Choice_UnitId->Append(_("0x98"));
	Choice_UnitId->Append(_("0x99"));
	Choice_UnitId->Append(_("0x9A"));
	Choice_UnitId->Append(_("0x9B"));
	Choice_UnitId->Append(_("0x9C"));
	Choice_UnitId->Append(_("0x9D"));
	Choice_UnitId->Append(_("0x9E"));
	Choice_UnitId->Append(_("0x9F"));
	Choice_UnitId->Append(_("0xA0"));
	Choice_UnitId->Append(_("0xA1"));
	Choice_UnitId->Append(_("0xA2"));
	Choice_UnitId->Append(_("0xA3"));
	Choice_UnitId->Append(_("0xA4"));
	Choice_UnitId->Append(_("0xA5"));
	Choice_UnitId->Append(_("0xA6"));
	Choice_UnitId->Append(_("0xA7"));
	Choice_UnitId->Append(_("0xA8"));
	Choice_UnitId->Append(_("0xA9"));
	Choice_UnitId->Append(_("0xAA"));
	Choice_UnitId->Append(_("0xAB"));
	Choice_UnitId->Append(_("0xAC"));
	Choice_UnitId->Append(_("0xAD"));
	Choice_UnitId->Append(_("0xAE"));
	Choice_UnitId->Append(_("0xAF"));
	Choice_UnitId->Append(_("0xB0"));
	Choice_UnitId->Append(_("0xB1"));
	Choice_UnitId->Append(_("0xB2"));
	Choice_UnitId->Append(_("0xB3"));
	Choice_UnitId->Append(_("0xB4"));
	Choice_UnitId->Append(_("0xB5"));
	Choice_UnitId->Append(_("0xB6"));
	Choice_UnitId->Append(_("0xB7"));
	Choice_UnitId->Append(_("0xB8"));
	Choice_UnitId->Append(_("0xB9"));
	Choice_UnitId->Append(_("0xBA"));
	Choice_UnitId->Append(_("0xBB"));
	Choice_UnitId->Append(_("0xBC"));
	Choice_UnitId->Append(_("0xBD"));
	Choice_UnitId->Append(_("0xBE"));
	Choice_UnitId->Append(_("0xBF"));
	Choice_UnitId->Append(_("0xC0"));
	Choice_UnitId->Append(_("0xC1"));
	Choice_UnitId->Append(_("0xC2"));
	Choice_UnitId->Append(_("0xC3"));
	Choice_UnitId->Append(_("0xC4"));
	Choice_UnitId->Append(_("0xC5"));
	Choice_UnitId->Append(_("0xC6"));
	Choice_UnitId->Append(_("0xC7"));
	Choice_UnitId->Append(_("0xC8"));
	Choice_UnitId->Append(_("0xC9"));
	Choice_UnitId->Append(_("0xCA"));
	Choice_UnitId->Append(_("0xCB"));
	Choice_UnitId->Append(_("0xCC"));
	Choice_UnitId->Append(_("0xCD"));
	Choice_UnitId->Append(_("0xCE"));
	Choice_UnitId->Append(_("0xCF"));
	Choice_UnitId->Append(_("0xD0"));
	Choice_UnitId->Append(_("0xD1"));
	Choice_UnitId->Append(_("0xD2"));
	Choice_UnitId->Append(_("0xD3"));
	Choice_UnitId->Append(_("0xD4"));
	Choice_UnitId->Append(_("0xD5"));
	Choice_UnitId->Append(_("0xD6"));
	Choice_UnitId->Append(_("0xD7"));
	Choice_UnitId->Append(_("0xD8"));
	Choice_UnitId->Append(_("0xD9"));
	Choice_UnitId->Append(_("0xDA"));
	Choice_UnitId->Append(_("0xDB"));
	Choice_UnitId->Append(_("0xDC"));
	Choice_UnitId->Append(_("0xDD"));
	Choice_UnitId->Append(_("0xDE"));
	Choice_UnitId->Append(_("0xDF"));
	Choice_UnitId->Append(_("0xE0"));
	Choice_UnitId->Append(_("0xE1"));
	Choice_UnitId->Append(_("0xE2"));
	Choice_UnitId->Append(_("0xE3"));
	Choice_UnitId->Append(_("0xE4"));
	Choice_UnitId->Append(_("0xE5"));
	Choice_UnitId->Append(_("0xE6"));
	Choice_UnitId->Append(_("0xE7"));
	Choice_UnitId->Append(_("0xE8"));
	Choice_UnitId->Append(_("0xE9"));
	Choice_UnitId->Append(_("0xEA"));
	Choice_UnitId->Append(_("0xEB"));
	Choice_UnitId->Append(_("0xEC"));
	Choice_UnitId->Append(_("0xED"));
	Choice_UnitId->Append(_("0xEE"));
	Choice_UnitId->Append(_("0xEF"));
	Choice_UnitId->Append(_("0XF0"));
	FlexGridSizer1->Add(Choice_UnitId, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Port:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_CommPort = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
	FlexGridSizer1->Add(Choice_CommPort, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Speed:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Speed = new wxChoice(this, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE3"));
	FlexGridSizer1->Add(Choice_Speed, 1, wxALL|wxEXPAND, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Channel:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer1->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Channel = new wxSpinCtrl(this, ID_SPINCTRL1, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 8, 1, _T("ID_SPINCTRL1"));
	SpinCtrl_Channel->SetValue(_T("1"));
	FlexGridSizer1->Add(SpinCtrl_Channel, 1, wxALL|wxEXPAND, 5);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Test:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer1->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Test = new wxChoice(this, ID_CHOICE4, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE4"));
	Choice_Test->SetSelection( Choice_Test->Append(_("Equals")) );
	Choice_Test->Append(_("Continuous"));
	Choice_Test->Append(_("On Change"));
	FlexGridSizer1->Add(Choice_Test, 1, wxALL|wxEXPAND, 5);
	StaticText7 = new wxStaticText(this, ID_STATICTEXT7, _("Value:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	FlexGridSizer1->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Value = new wxSpinCtrl(this, ID_SPINCTRL2, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 0, 1, 1, _T("ID_SPINCTRL2"));
	SpinCtrl_Value->SetValue(_T("1"));
	FlexGridSizer1->Add(SpinCtrl_Value, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	//*)

    auto ports = SerialOutput::GetPossibleSerialPorts();
    for (auto it = ports.begin(); it != ports.end(); ++it)
    {
        Choice_CommPort->Append(*it);
    }
    Choice_CommPort->SetSelection(0);

    auto speeds = SerialOutput::GetPossibleBaudRates();
    for (auto it = speeds.begin(); it != speeds.end(); ++it)
    {
        Choice_Speed->Append(*it);
    }
    Choice_Speed->SetStringSelection("19200");
}

EventLorPanel::~EventLorPanel()
{
	//(*Destroy(EventLorPanel)
	//*)
}

bool EventLorPanel::ValidateWindow()
{
    return true;
}

void EventLorPanel::Save(EventBase* event)
{
    EventLor* e = (EventLor*)event;
    e->SetUnitId(Choice_UnitId->GetStringSelection().ToStdString());
    e->SetCommPort(Choice_CommPort->GetStringSelection().ToStdString());
    e->SetCondition(Choice_Test->GetStringSelection().ToStdString());
    e->SetSpeed(wxAtoi(Choice_Speed->GetStringSelection()));
    e->SetChannel(SpinCtrl_Channel->GetValue());
    e->SetThreshold(SpinCtrl_Value->GetValue());
}

void EventLorPanel::Load(EventBase* event)
{
    EventLor* e = (EventLor*)event;
    Choice_UnitId->SetStringSelection(e->GetUnitId());
    Choice_CommPort->SetStringSelection(e->GetCommPort());
    Choice_Test->SetStringSelection(e->GetCondition());
    Choice_Speed->SetStringSelection(wxString::Format("%d", e->GetSpeed()));
    SpinCtrl_Channel->SetValue(e->GetChannel());
    SpinCtrl_Value->SetValue(e->GetThreshold());
}

