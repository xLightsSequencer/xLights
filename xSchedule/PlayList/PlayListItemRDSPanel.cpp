/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "PlayListItemRDSPanel.h"
#include "PlayListItemRDS.h"
#include "../../xLights/outputs/SerialOutput.h"

//(*InternalHeaders(PlayListItemRDSPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(PlayListItemRDSPanel)
const long PlayListItemRDSPanel::ID_STATICTEXT1 = wxNewId();
const long PlayListItemRDSPanel::ID_CHOICE1 = wxNewId();
const long PlayListItemRDSPanel::ID_STATICTEXT2 = wxNewId();
const long PlayListItemRDSPanel::ID_TEXTCTRL1 = wxNewId();
const long PlayListItemRDSPanel::ID_STATICTEXT5 = wxNewId();
const long PlayListItemRDSPanel::ID_TEXTCTRL2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PlayListItemRDSPanel,wxPanel)
	//(*EventTable(PlayListItemRDSPanel)
	//*)
END_EVENT_TABLE()

PlayListItemRDSPanel::PlayListItemRDSPanel(wxWindow* parent, PlayListItemRDS* rds, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _rds = rds;

	//(*Initialize(PlayListItemRDSPanel)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxDefaultPosition, wxSize(396,287), wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Serial Port"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_CommPort = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	FlexGridSizer1->Add(Choice_CommPort, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Station Name"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_StationName = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	TextCtrl_StationName->SetMaxLength(72);
	FlexGridSizer1->Add(TextCtrl_StationName, 1, wxALL|wxEXPAND, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Text"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer1->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Text = new wxTextCtrl(this, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	TextCtrl_Text->SetMaxLength(64);
	FlexGridSizer1->Add(TextCtrl_Text, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	SetSizer(FlexGridSizer1);
	Layout();

	Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&PlayListItemRDSPanel::OnChoice_ScrollModeSelect);
	//*)

    // populate the commport list
    auto ports = SerialOutput::GetPossibleSerialPorts();
    for (auto it = ports.begin(); it != ports.end(); ++it)
    {
        Choice_CommPort->AppendString(*it);
    }
    Choice_CommPort->SetSelection(0);

    TextCtrl_StationName->SetValue(_rds->GetStationName());
    TextCtrl_Text->SetValue(_rds->GetText());
    TextCtrl_Text->SetToolTip(PlayListItemRDS::GetTooltip());
    Choice_CommPort->SetStringSelection(_rds->GetCommPort());

    ValidateWindow();
}

PlayListItemRDSPanel::~PlayListItemRDSPanel()
{
	//(*Destroy(PlayListItemRDSPanel)
	//*)
    _rds->SetStationName(TextCtrl_StationName->GetValue().ToStdString());
    _rds->SetText(TextCtrl_Text->GetValue().ToStdString());
    _rds->SetCommPort(Choice_CommPort->GetStringSelection().ToStdString());
}

void PlayListItemRDSPanel::OnChoice_ScrollModeSelect(wxCommandEvent& event)
{
    ValidateWindow();
}

void PlayListItemRDSPanel::ValidateWindow()
{
}
