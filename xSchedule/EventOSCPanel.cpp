#include "EventOSCPanel.h"
#include "EventDialog.h"
#include "events/EventOSC.h"

//(*InternalHeaders(EventOSCPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(EventOSCPanel)
const long EventOSCPanel::ID_STATICTEXT1 = wxNewId();
const long EventOSCPanel::ID_TEXTCTRL1 = wxNewId();
const long EventOSCPanel::ID_STATICTEXT2 = wxNewId();
const long EventOSCPanel::ID_CHOICE1 = wxNewId();
const long EventOSCPanel::ID_STATICTEXT3 = wxNewId();
const long EventOSCPanel::ID_CHOICE2 = wxNewId();
const long EventOSCPanel::ID_STATICTEXT4 = wxNewId();
const long EventOSCPanel::ID_CHOICE3 = wxNewId();
//*)

BEGIN_EVENT_TABLE(EventOSCPanel,wxPanel)
	//(*EventTable(EventOSCPanel)
	//*)
END_EVENT_TABLE()

EventOSCPanel::EventOSCPanel(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(EventOSCPanel)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Path:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Path = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(TextCtrl_Path, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Parameter 1:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Parm1Type = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	Choice_Parm1Type->SetSelection( Choice_Parm1Type->Append(_("None")) );
	Choice_Parm1Type->Append(_("String"));
	Choice_Parm1Type->Append(_("Integer"));
	Choice_Parm1Type->Append(_("Float"));
	FlexGridSizer1->Add(Choice_Parm1Type, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Parameter 2:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Parm2Type = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
	Choice_Parm2Type->SetSelection( Choice_Parm2Type->Append(_("None")) );
	Choice_Parm2Type->Append(_("String"));
	Choice_Parm2Type->Append(_("Integer"));
	Choice_Parm2Type->Append(_("Float"));
	FlexGridSizer1->Add(Choice_Parm2Type, 1, wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Parameter 3:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Parm3Type = new wxChoice(this, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE3"));
	Choice_Parm3Type->SetSelection( Choice_Parm3Type->Append(_("None")) );
	Choice_Parm3Type->Append(_("String"));
	Choice_Parm3Type->Append(_("Integer"));
	Choice_Parm3Type->Append(_("Float"));
	FlexGridSizer1->Add(Choice_Parm3Type, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&EventOSCPanel::OnTextCtrl_PathText);
	Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&EventOSCPanel::OnChoice_Parm1TypeSelect);
	Connect(ID_CHOICE2,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&EventOSCPanel::OnChoice_Parm2TypeSelect);
	//*)
}

EventOSCPanel::~EventOSCPanel()
{
	//(*Destroy(EventOSCPanel)
	//*)
}

bool EventOSCPanel::ValidateWindow()
{
    if (Choice_Parm1Type->GetStringSelection() == "None")
    {
        Choice_Parm2Type->Enable(false);
        Choice_Parm3Type->Enable(false);
    }
    else
    {
        Choice_Parm2Type->Enable();
        if (Choice_Parm2Type->GetStringSelection() == "None")
        {
            Choice_Parm3Type->Enable(false);
        }
        else
        {
            Choice_Parm3Type->Enable();
        }
    }

    return (TextCtrl_Path->GetValue().Trim().Trim(false) != "" && TextCtrl_Path->GetValue()[0] == '/');
}

void EventOSCPanel::Save(EventBase* event)
{
    EventOSC* e = (EventOSC*)event;
    e->SetPath(TextCtrl_Path->GetValue().ToStdString());
    e->SetOSCParm1Type(Choice_Parm1Type->GetStringSelection().ToStdString());
    e->SetOSCParm2Type(Choice_Parm2Type->GetStringSelection().ToStdString());
    e->SetOSCParm3Type(Choice_Parm3Type->GetStringSelection().ToStdString());
}

void EventOSCPanel::Load(EventBase* event)
{
    EventOSC* e = (EventOSC*)event;
    TextCtrl_Path->SetValue(e->GetPath());
    Choice_Parm1Type->SetStringSelection(e->GetParm1Type());
    Choice_Parm2Type->SetStringSelection(e->GetParm2Type());
    Choice_Parm3Type->SetStringSelection(e->GetParm3Type());
}

void EventOSCPanel::OnTextCtrl_PathText(wxCommandEvent& event)
{
    ((EventDialog*)GetParent()->GetParent()->GetParent()->GetParent())->ValidateWindow();
}

void EventOSCPanel::OnChoice_Parm1TypeSelect(wxCommandEvent& event)
{
    ((EventDialog*)GetParent()->GetParent()->GetParent()->GetParent())->ValidateWindow();
}

void EventOSCPanel::OnChoice_Parm2TypeSelect(wxCommandEvent& event)
{
    ((EventDialog*)GetParent()->GetParent()->GetParent()->GetParent())->ValidateWindow();
}
