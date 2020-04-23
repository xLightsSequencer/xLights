/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "EventARTNetTriggerPanel.h"
#include "events/EventARTNetTrigger.h"

//(*InternalHeaders(EventARTNetTriggerPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(EventARTNetTriggerPanel)
const long EventARTNetTriggerPanel::ID_STATICTEXT1 = wxNewId();
const long EventARTNetTriggerPanel::ID_SPINCTRL1 = wxNewId();
const long EventARTNetTriggerPanel::ID_STATICTEXT2 = wxNewId();
const long EventARTNetTriggerPanel::ID_SPINCTRL2 = wxNewId();
const long EventARTNetTriggerPanel::ID_STATICTEXT3 = wxNewId();
const long EventARTNetTriggerPanel::ID_SPINCTRL3 = wxNewId();
//*)

BEGIN_EVENT_TABLE(EventARTNetTriggerPanel,wxPanel)
	//(*EventTable(EventARTNetTriggerPanel)
	//*)
END_EVENT_TABLE()

EventARTNetTriggerPanel::EventARTNetTriggerPanel(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(EventARTNetTriggerPanel)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("OEM:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_OEM = new wxSpinCtrl(this, ID_SPINCTRL1, _T("65535"), wxDefaultPosition, wxDefaultSize, 0, 0, 65535, 65535, _T("ID_SPINCTRL1"));
	SpinCtrl_OEM->SetValue(_T("65535"));
	FlexGridSizer1->Add(SpinCtrl_OEM, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Key:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Key = new wxSpinCtrl(this, ID_SPINCTRL2, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 0, 255, 1, _T("ID_SPINCTRL2"));
	SpinCtrl_Key->SetValue(_T("1"));
	FlexGridSizer1->Add(SpinCtrl_Key, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Sub Key:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_SubKey = new wxSpinCtrl(this, ID_SPINCTRL3, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 0, 255, 1, _T("ID_SPINCTRL3"));
	SpinCtrl_SubKey->SetValue(_T("1"));
	FlexGridSizer1->Add(SpinCtrl_SubKey, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	//*)
}

EventARTNetTriggerPanel::~EventARTNetTriggerPanel()
{
	//(*Destroy(EventARTNetTriggerPanel)
	//*)
}

bool EventARTNetTriggerPanel::ValidateWindow()
{
    return true;
}

void EventARTNetTriggerPanel::Save(EventBase* event)
{
    EventARTNetTrigger* e = (EventARTNetTrigger*)event;
    e->SetOEM(SpinCtrl_OEM->GetValue());
    e->SetKey(SpinCtrl_Key->GetValue());
    e->SetSubKey(SpinCtrl_SubKey->GetValue());
}

void EventARTNetTriggerPanel::Load(EventBase* event)
{
    EventARTNetTrigger* e = (EventARTNetTrigger*)event;
    SpinCtrl_OEM->SetValue(e->GetOEM());
    SpinCtrl_Key->SetValue(e->GetKey());
    SpinCtrl_SubKey->SetValue(e->GetSubKey());
}