/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "PlayListItemDelayPanel.h"
#include "PlayListItemDelay.h"

//(*InternalHeaders(PlayListItemDelayPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(PlayListItemDelayPanel)
const long PlayListItemDelayPanel::ID_STATICTEXT1 = wxNewId();
const long PlayListItemDelayPanel::ID_TEXTCTRL1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PlayListItemDelayPanel,wxPanel)
	//(*EventTable(PlayListItemDelayPanel)
	//*)
END_EVENT_TABLE()

PlayListItemDelayPanel::PlayListItemDelayPanel(wxWindow* parent, PlayListItemDelay* Delay, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _Delay = Delay;

	//(*Initialize(PlayListItemDelayPanel)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Duration:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_DelayDuration = new wxTextCtrl(this, ID_TEXTCTRL1, _("0.050"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(TextCtrl_DelayDuration, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemDelayPanel::OnTextCtrl_DelayDurationText);
	//*)

    TextCtrl_DelayDuration->SetValue(wxString::Format(wxT("%.3f"), (float)Delay->GetDuration() / 1000.0));
}

PlayListItemDelayPanel::~PlayListItemDelayPanel()
{
	//(*Destroy(PlayListItemDelayPanel)
	//*)
    _Delay->SetDuration(wxAtof(TextCtrl_DelayDuration->GetValue()) * 1000);
}

void PlayListItemDelayPanel::OnTextCtrl_DelayDurationText(wxCommandEvent& event)
{
    ValidateWindow();
}

void PlayListItemDelayPanel::ValidateWindow()
{
    
}
