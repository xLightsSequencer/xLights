//(*InternalHeaders(EventStatePanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "EventStatePanel.h"
#include "events/EventState.h"

//(*IdInit(EventStatePanel)
const long EventStatePanel::ID_STATICTEXT2 = wxNewId();
const long EventStatePanel::ID_CHOICE1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(EventStatePanel,wxPanel)
	//(*EventTable(EventStatePanel)
	//*)
END_EVENT_TABLE()

EventStatePanel::EventStatePanel(wxWindow* parent, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(EventStatePanel)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("State"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_States = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	FlexGridSizer1->Add(Choice_States, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	//*)

    auto states = EventState::GetStates();
    for (auto it: states)
    {
        Choice_States->Append(it);
    }
    Choice_States->SetSelection(0);
}

EventStatePanel::~EventStatePanel()
{
	//(*Destroy(EventStatePanel)
	//*)
}

void EventStatePanel::Save(EventBase* event)
{
    EventState* e = (EventState*)event;
    e->SetState(Choice_States->GetStringSelection());
}

void EventStatePanel::Load(EventBase* event)
{
    EventState* e = (EventState*)event;
    Choice_States->SetStringSelection(e->GetState());
}
