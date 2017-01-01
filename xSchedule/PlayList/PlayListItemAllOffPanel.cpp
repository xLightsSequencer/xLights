#include "PlayListItemAllOffPanel.h"
#include "PlayListItemAllOff.h"

//(*InternalHeaders(PlayListItemAllOffPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(PlayListItemAllOffPanel)
const long PlayListItemAllOffPanel::ID_STATICTEXT1 = wxNewId();
const long PlayListItemAllOffPanel::ID_TEXTCTRL1 = wxNewId();
const long PlayListItemAllOffPanel::ID_STATICTEXT2 = wxNewId();
const long PlayListItemAllOffPanel::ID_TEXTCTRL2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PlayListItemAllOffPanel,wxPanel)
	//(*EventTable(PlayListItemAllOffPanel)
	//*)
END_EVENT_TABLE()

PlayListItemAllOffPanel::PlayListItemAllOffPanel(wxWindow* parent, PlayListItemAllOff* alloff, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _alloff = alloff;

	//(*Initialize(PlayListItemAllOffPanel)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Duration:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_AllOffDuration = new wxTextCtrl(this, ID_TEXTCTRL1, _("0.050"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(TextCtrl_AllOffDuration, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Delay:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Delay = new wxTextCtrl(this, ID_TEXTCTRL2, _("0.000"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer1->Add(TextCtrl_Delay, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemAllOffPanel::OnTextCtrl_AllOffDurationText);
	//*)

    TextCtrl_AllOffDuration->SetValue(wxString::Format(wxT("%.3f"), (float)alloff->GetDuration() / 1000.0));
    TextCtrl_Delay->SetValue(wxString::Format(wxT("%.3f"), (float)alloff->GetDelay() / 1000.0));
}

PlayListItemAllOffPanel::~PlayListItemAllOffPanel()
{
	//(*Destroy(PlayListItemAllOffPanel)
	//*)
    _alloff->SetDuration(wxAtof(TextCtrl_AllOffDuration->GetValue()) * 1000);
    _alloff->SetDelay(wxAtof(TextCtrl_Delay->GetValue()) * 1000);
}

void PlayListItemAllOffPanel::OnTextCtrl_AllOffDurationText(wxCommandEvent& event)
{
}