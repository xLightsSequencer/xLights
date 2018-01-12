#include "EventFPPPanel.h"
#include "events/EventFPP.h"

//(*InternalHeaders(EventFPPPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(EventFPPPanel)
const long EventFPPPanel::ID_STATICTEXT1 = wxNewId();
const long EventFPPPanel::ID_SPINCTRL1 = wxNewId();
const long EventFPPPanel::ID_STATICTEXT2 = wxNewId();
const long EventFPPPanel::ID_SPINCTRL2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(EventFPPPanel,wxPanel)
	//(*EventTable(EventFPPPanel)
	//*)
END_EVENT_TABLE()

EventFPPPanel::EventFPPPanel(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(EventFPPPanel)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Id - Major:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Major = new wxSpinCtrl(this, ID_SPINCTRL1, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 24, 1, _T("ID_SPINCTRL1"));
	SpinCtrl_Major->SetValue(_T("1"));
	FlexGridSizer1->Add(SpinCtrl_Major, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Id - Minor:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Minor = new wxSpinCtrl(this, ID_SPINCTRL2, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 24, 1, _T("ID_SPINCTRL2"));
	SpinCtrl_Minor->SetValue(_T("1"));
	FlexGridSizer1->Add(SpinCtrl_Minor, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	//*)
}

EventFPPPanel::~EventFPPPanel()
{
	//(*Destroy(EventFPPPanel)
	//*)
}

void EventFPPPanel::Save(EventBase* event)
{
    EventFPP* e = (EventFPP*)event;
    e->SetEventId(wxString::Format("%02d_%02d", SpinCtrl_Major->GetValue(), SpinCtrl_Minor->GetValue()).ToStdString());
}

void EventFPPPanel::Load(EventBase* event)
{
    EventFPP* e = (EventFPP*)event;
    auto parts = wxSplit(e->GetEventId(), '_');
    if (parts.size() == 2)
    {
        SpinCtrl_Major->SetValue(wxAtoi(parts[0]));
        SpinCtrl_Minor->SetValue(wxAtoi(parts[1]));
    }
}
