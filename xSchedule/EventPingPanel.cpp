#include "EventPingPanel.h"
#include "events/EventPing.h"
#include "../xLights/outputs/OutputManager.h"
#include "../xLights/outputs/Output.h"

//(*InternalHeaders(EventPingPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(EventPingPanel)
const long EventPingPanel::ID_STATICTEXT2 = wxNewId();
const long EventPingPanel::ID_CHOICE1 = wxNewId();
const long EventPingPanel::ID_STATICTEXT1 = wxNewId();
const long EventPingPanel::ID_SPINCTRL1 = wxNewId();
const long EventPingPanel::ID_CHECKBOX1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(EventPingPanel,wxPanel)
	//(*EventTable(EventPingPanel)
	//*)
END_EVENT_TABLE()

std::string GetPingID(Output* output)
{
    std::string id = output->GetIP();

    if (id == "") id = output->GetCommPort();

    return id;
}

EventPingPanel::EventPingPanel(wxWindow* parent, OutputManager* outputManager, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(EventPingPanel)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Output"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_IPs = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	FlexGridSizer1->Add(Choice_IPs, 1, wxALL|wxEXPAND, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Failures before trigger"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_PingFailures = new wxSpinCtrl(this, ID_SPINCTRL1, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 10, 1, _T("ID_SPINCTRL1"));
	SpinCtrl_PingFailures->SetValue(_T("1"));
	FlexGridSizer1->Add(SpinCtrl_PingFailures, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_FireOnce = new wxCheckBox(this, ID_CHECKBOX1, _("Fire once only"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_FireOnce->SetValue(true);
	FlexGridSizer1->Add(CheckBox_FireOnce, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	//*)

    Choice_IPs->Append("All");

    std::list<std::string> created;
    auto outputs = outputManager->GetOutputs();
    for (auto it = outputs.begin(); it != outputs.end(); ++it)
    {
        if ((*it)->CanPing())
        {
            // check if we have already seen it
            bool found = false;
            for (auto cit = created.begin(); cit != created.end(); ++cit)
            {
                if (*cit == GetPingID(*it))
                {
                    // we have seen it
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                created.push_back(GetPingID(*it));
                Choice_IPs->Append((*it)->GetPingDescription());
            }
        }
    }
    Choice_IPs->SetSelection(0);
}

EventPingPanel::~EventPingPanel()
{
	//(*Destroy(EventPingPanel)
	//*)
}

void EventPingPanel::Save(EventBase* event)
{
    EventPing* e = (EventPing*)event;
    e->SetFailures(SpinCtrl_PingFailures->GetValue());
    e->SetIP(Choice_IPs->GetStringSelection().ToStdString());
    e->SetOnceOnly(CheckBox_FireOnce->GetValue());
}

void EventPingPanel::Load(EventBase* event)
{
    EventPing* e = (EventPing*)event;
    SpinCtrl_PingFailures->SetValue(e->GetFailures());
    Choice_IPs->SetStringSelection(e->GetIP());
    if (Choice_IPs->GetSelection() < 0)
    {
        Choice_IPs->SetSelection(0);
    }
    CheckBox_FireOnce->SetValue(e->GetOnceOnly());
}
