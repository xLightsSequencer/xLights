#include "PlayListItemPluginPanel.h"
#include "PlayListDialog.h"
#include "PlayListItemPlugin.h"
#include "../xScheduleApp.h"
#include "../xScheduleMain.h"
#include "../PluginManager.h"

//(*InternalHeaders(PlayListItemPluginPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(PlayListItemPluginPanel)
const long PlayListItemPluginPanel::ID_STATICTEXT3 = wxNewId();
const long PlayListItemPluginPanel::ID_TEXTCTRL3 = wxNewId();
const long PlayListItemPluginPanel::ID_STATICTEXT4 = wxNewId();
const long PlayListItemPluginPanel::ID_COMBOBOX1 = wxNewId();
const long PlayListItemPluginPanel::ID_STATICTEXT5 = wxNewId();
const long PlayListItemPluginPanel::ID_CHOICE1 = wxNewId();
const long PlayListItemPluginPanel::ID_STATICTEXT1 = wxNewId();
const long PlayListItemPluginPanel::ID_TEXTCTRL4 = wxNewId();
const long PlayListItemPluginPanel::ID_STATICTEXT2 = wxNewId();
const long PlayListItemPluginPanel::ID_TEXTCTRL2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PlayListItemPluginPanel,wxPanel)
	//(*EventTable(PlayListItemPluginPanel)
	//*)
END_EVENT_TABLE()

PlayListItemPluginPanel::PlayListItemPluginPanel(wxWindow* parent, PlayListItemPlugin* plugin, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _plugin = plugin;

	//(*Initialize(PlayListItemPluginPanel)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	FlexGridSizer1->AddGrowableRow(2);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Name = new wxTextCtrl(this, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL3"));
	FlexGridSizer1->Add(TextCtrl_Name, 1, wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Plugin Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	ComboBox_PluginName = new wxComboBox(this, ID_COMBOBOX1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_COMBOBOX1"));
	FlexGridSizer1->Add(ComboBox_PluginName, 1, wxALL|wxEXPAND, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Action:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer1->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Action = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	FlexGridSizer1->Add(Choice_Action, 1, wxALL|wxEXPAND, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Event Parm:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_EventParm = new wxTextCtrl(this, ID_TEXTCTRL4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL4"));
	FlexGridSizer1->Add(TextCtrl_EventParm, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Delay:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Delay = new wxTextCtrl(this, ID_TEXTCTRL2, _("0.000"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer1->Add(TextCtrl_Delay, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL3,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemPluginPanel::OnTextCtrl_NameText);
	Connect(ID_TEXTCTRL2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemPluginPanel::OnTextCtrl_DelayText);
	//*)

    Choice_Action->AppendString("Start");
    Choice_Action->AppendString("Stop");
    Choice_Action->AppendString("Event");

    auto plugins = ((xScheduleApp*)wxTheApp)->GetFrame()->GetPluginManager().GetPlugins();
    for (auto it : plugins)
    {
        ComboBox_PluginName->AppendString(it);
    }

    TextCtrl_Name->SetValue(_plugin->GetRawName());
    ComboBox_PluginName->SetValue(_plugin->GetPlugin());
    Choice_Action->SetStringSelection(_plugin->GetAction());
    TextCtrl_EventParm->SetValue(_plugin->GetEventParm());
    TextCtrl_Delay->SetValue(wxString::Format(wxT("%.3f"), (float)_plugin->GetDelay() / 1000.0));

    ValidateWindow();
}

PlayListItemPluginPanel::~PlayListItemPluginPanel()
{
	//(*Destroy(PlayListItemPluginPanel)
	//*)
    _plugin->SetName(TextCtrl_Name->GetValue().ToStdString());
    _plugin->SetPlugin(ComboBox_PluginName->GetValue().ToStdString());
    _plugin->SetAction(Choice_Action->GetStringSelection().ToStdString());
    _plugin->SetEventParm(TextCtrl_EventParm->GetValue().ToStdString());
    _plugin->SetDelay(wxAtof(TextCtrl_Delay->GetValue())*1000);
}


void PlayListItemPluginPanel::OnTextCtrl_DelayText(wxCommandEvent& event)
{
}

void PlayListItemPluginPanel::OnChoice_ActionSelect(wxCommandEvent& event)
{
    ValidateWindow();
}

void PlayListItemPluginPanel::OnTextCtrl_NameText(wxCommandEvent& event)
{
    _plugin->SetName(TextCtrl_Name->GetValue().ToStdString());
    wxCommandEvent e(EVT_UPDATEITEMNAME);
    wxPostEvent(GetParent()->GetParent()->GetParent()->GetParent(), e);
}

void PlayListItemPluginPanel::ValidateWindow()
{
    if (Choice_Action->GetStringSelection() == "Event")
    {
        TextCtrl_EventParm->Enable();
    }
    else
    {
        TextCtrl_EventParm->Disable();
        TextCtrl_EventParm->SetValue("");
    }
}
