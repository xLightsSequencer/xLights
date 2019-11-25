#include "PlayListItemSerialPanel.h"
#include "PlayListDialog.h"
#include "PlayListItemSerial.h"
#include "../../xLights/outputs/SerialOutput.h"

//(*InternalHeaders(PlayListItemSerialPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(PlayListItemSerialPanel)
const long PlayListItemSerialPanel::ID_STATICTEXT3 = wxNewId();
const long PlayListItemSerialPanel::ID_TEXTCTRL3 = wxNewId();
const long PlayListItemSerialPanel::ID_STATICTEXT1 = wxNewId();
const long PlayListItemSerialPanel::ID_CHOICE3 = wxNewId();
const long PlayListItemSerialPanel::ID_STATICTEXT4 = wxNewId();
const long PlayListItemSerialPanel::ID_CHOICE1 = wxNewId();
const long PlayListItemSerialPanel::ID_STATICTEXT6 = wxNewId();
const long PlayListItemSerialPanel::ID_CHOICE2 = wxNewId();
const long PlayListItemSerialPanel::ID_STATICTEXT5 = wxNewId();
const long PlayListItemSerialPanel::ID_TEXTCTRL4 = wxNewId();
const long PlayListItemSerialPanel::ID_STATICTEXT2 = wxNewId();
const long PlayListItemSerialPanel::ID_TEXTCTRL2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PlayListItemSerialPanel,wxPanel)
	//(*EventTable(PlayListItemSerialPanel)
	//*)
END_EVENT_TABLE()

PlayListItemSerialPanel::PlayListItemSerialPanel(wxWindow* parent, PlayListItemSerial* serial, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _serial = serial;

	//(*Initialize(PlayListItemSerialPanel)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	FlexGridSizer1->AddGrowableRow(4);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_SerialName = new wxTextCtrl(this, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL3"));
	FlexGridSizer1->Add(TextCtrl_SerialName, 1, wxALL|wxEXPAND, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Comm Port:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_CommPort = new wxChoice(this, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE3"));
	FlexGridSizer1->Add(Choice_CommPort, 1, wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Configuration:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Configuration = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	Choice_Configuration->SetSelection( Choice_Configuration->Append(_("8N1")) );
	Choice_Configuration->Append(_("8N2"));
	FlexGridSizer1->Add(Choice_Configuration, 1, wxALL|wxEXPAND, 5);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Speed:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer1->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Speed = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
	FlexGridSizer1->Add(Choice_Speed, 1, wxALL|wxEXPAND, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Data:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer1->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Data = new wxTextCtrl(this, ID_TEXTCTRL4, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE, wxDefaultValidator, _T("ID_TEXTCTRL4"));
	FlexGridSizer1->Add(TextCtrl_Data, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Delay:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Delay = new wxTextCtrl(this, ID_TEXTCTRL2, _("0.000"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer1->Add(TextCtrl_Delay, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL3,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemSerialPanel::OnTextCtrl_SerialNameText);
	Connect(ID_TEXTCTRL2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemSerialPanel::OnTextCtrl_DelayText);
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

    TextCtrl_SerialName->SetValue(serial->GetRawName());
    TextCtrl_Data->SetValue(serial->GetData());
    TextCtrl_Data->SetToolTip(PlayListItemSerial::GetTooltip());
    Choice_Speed->SetStringSelection(wxString::Format("%d", serial->GetSpeed()));
    Choice_CommPort->SetStringSelection(serial->GetCommPort());
    Choice_Configuration->SetStringSelection(serial->GetConfiguration());
    TextCtrl_Delay->SetValue(wxString::Format(wxT("%.3f"), (float)serial->GetDelay() / 1000.0));
}

PlayListItemSerialPanel::~PlayListItemSerialPanel()
{
	//(*Destroy(PlayListItemSerialPanel)
	//*)
    _serial->SetName(TextCtrl_SerialName->GetValue().ToStdString());
    _serial->SetCommPort(Choice_CommPort->GetStringSelection().ToStdString());
    _serial->SetConfiguration(Choice_Configuration->GetStringSelection().ToStdString());
    _serial->SetSpeed(wxAtoi(Choice_Speed->GetStringSelection()));
    _serial->SetData(TextCtrl_Data->GetValue().ToStdString());
    _serial->SetDelay(wxAtof(TextCtrl_Delay->GetValue())*1000);
}

void PlayListItemSerialPanel::OnTextCtrl_DelayText(wxCommandEvent& event)
{
}

void PlayListItemSerialPanel::OnTextCtrl_SerialNameText(wxCommandEvent& event)
{
    _serial->SetName(TextCtrl_SerialName->GetValue().ToStdString());
    wxCommandEvent e(EVT_UPDATEITEMNAME);
    wxPostEvent(GetParent()->GetParent()->GetParent()->GetParent(), e);
}
