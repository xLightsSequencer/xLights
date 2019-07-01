#include "EventDataPanel.h"
#include "EventDialog.h"
#include "events/EventData.h"
#include "xScheduleMain.h"
#include "ScheduleManager.h"
#include "../xLights/outputs/OutputManager.h"

//(*InternalHeaders(EventDataPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(EventDataPanel)
const long EventDataPanel::ID_STATICTEXT1 = wxNewId();
const long EventDataPanel::ID_TEXTCTRL1 = wxNewId();
const long EventDataPanel::ID_STATICTEXT4 = wxNewId();
const long EventDataPanel::ID_STATICTEXT2 = wxNewId();
const long EventDataPanel::ID_CHOICE2 = wxNewId();
const long EventDataPanel::ID_STATICTEXT3 = wxNewId();
const long EventDataPanel::ID_SPINCTRL3 = wxNewId();
//*)

BEGIN_EVENT_TABLE(EventDataPanel,wxPanel)
	//(*EventTable(EventDataPanel)
	//*)
END_EVENT_TABLE()

EventDataPanel::EventDataPanel(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(EventDataPanel)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Channel:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	TextCtrl_Channel = new wxTextCtrl(this, ID_TEXTCTRL1, _("1"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer2->Add(TextCtrl_Channel, 1, wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("1"), wxDefaultPosition, wxSize(60,-1), 0, _T("ID_STATICTEXT4"));
	FlexGridSizer2->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Test:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Test = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
	Choice_Test->SetSelection( Choice_Test->Append(_("Equals")) );
	Choice_Test->Append(_("Less Than"));
	Choice_Test->Append(_("Less Than or Equals"));
	Choice_Test->Append(_("Greater Than"));
	Choice_Test->Append(_("Greater Than or Equals"));
	Choice_Test->Append(_("Not Equals"));
	Choice_Test->Append(_("Continuous"));
	Choice_Test->Append(_("On Change"));
	FlexGridSizer1->Add(Choice_Test, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Value:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Value = new wxSpinCtrl(this, ID_SPINCTRL3, _T("255"), wxDefaultPosition, wxDefaultSize, 0, 0, 255, 255, _T("ID_SPINCTRL3"));
	SpinCtrl_Value->SetValue(_T("255"));
	FlexGridSizer1->Add(SpinCtrl_Value, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&EventDataPanel::OnTextCtrl_ChannelText);
	//*)
}

EventDataPanel::~EventDataPanel()
{
	//(*Destroy(EventDataPanel)
	//*)
}

bool EventDataPanel::ValidateWindow()
{
    if (Choice_Test->GetStringSelection() == "Continuous" || Choice_Test->GetStringSelection() == "On Change")
    {
        SpinCtrl_Value->Enable(false);
    }
    else
    {
        SpinCtrl_Value->Enable();
    }

    return true;
}

void EventDataPanel::Save(EventBase* event)
{
    EventData* e = (EventData*)event;
    e->SetChannel(TextCtrl_Channel->GetValue().ToStdString());
    e->SetCondition(Choice_Test->GetStringSelection().ToStdString());
    e->SetThreshold(SpinCtrl_Value->GetValue());
}

void EventDataPanel::Load(EventBase* event)
{
    EventData* e = (EventData*)event;
    TextCtrl_Channel->SetValue(e->GetChannel());
    Choice_Test->SetStringSelection(e->GetCondition());
    SpinCtrl_Value->SetValue(e->GetThreshold());
}


void EventDataPanel::OnTextCtrl_ChannelText(wxCommandEvent& event)
{
    OutputManager* outputManager = xScheduleFrame::GetScheduleManager()->GetOutputManager();
    long sc = outputManager->DecodeStartChannel(TextCtrl_Channel->GetValue().ToStdString());
    if (sc == 0 || sc > xScheduleFrame::GetScheduleManager()->GetTotalChannels())
    {
        StaticText4->SetLabel("Invalid");
    }
    else
    {
        StaticText4->SetLabel(wxString::Format("%ld", (long)sc));
    }
    ((EventDialog*)GetParent()->GetParent()->GetParent()->GetParent())->ValidateWindow();
}
