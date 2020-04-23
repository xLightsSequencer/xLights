/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "PlayListItemSetColourPanel.h"
#include "PlayListItemSetColour.h"
#include "../xScheduleMain.h"
#include "../ScheduleManager.h"
#include "../../xLights/outputs/OutputManager.h"

//(*InternalHeaders(PlayListItemSetColourPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(PlayListItemSetColourPanel)
const long PlayListItemSetColourPanel::ID_STATICTEXT1 = wxNewId();
const long PlayListItemSetColourPanel::ID_TEXTCTRL1 = wxNewId();
const long PlayListItemSetColourPanel::ID_STATICTEXT3 = wxNewId();
const long PlayListItemSetColourPanel::ID_COLOURPICKERCTRL1 = wxNewId();
const long PlayListItemSetColourPanel::ID_CHECKBOX1 = wxNewId();
const long PlayListItemSetColourPanel::ID_STATICTEXT6 = wxNewId();
const long PlayListItemSetColourPanel::ID_TEXTCTRL3 = wxNewId();
const long PlayListItemSetColourPanel::ID_STATICTEXT8 = wxNewId();
const long PlayListItemSetColourPanel::ID_STATICTEXT7 = wxNewId();
const long PlayListItemSetColourPanel::ID_SPINCTRL4 = wxNewId();
const long PlayListItemSetColourPanel::ID_STATICTEXT4 = wxNewId();
const long PlayListItemSetColourPanel::ID_CHOICE1 = wxNewId();
const long PlayListItemSetColourPanel::ID_STATICTEXT5 = wxNewId();
const long PlayListItemSetColourPanel::ID_SPINCTRL2 = wxNewId();
const long PlayListItemSetColourPanel::ID_STATICTEXT2 = wxNewId();
const long PlayListItemSetColourPanel::ID_TEXTCTRL2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PlayListItemSetColourPanel,wxPanel)
	//(*EventTable(PlayListItemSetColourPanel)
	//*)
END_EVENT_TABLE()

PlayListItemSetColourPanel::PlayListItemSetColourPanel(wxWindow* parent, OutputManager* outputManager, PlayListItemSetColour* SetColour, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _outputManager = outputManager;
    _SetColour = SetColour;

	//(*Initialize(PlayListItemSetColourPanel)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Duration:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_SetColourDuration = new wxTextCtrl(this, ID_TEXTCTRL1, _("0.050"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(TextCtrl_SetColourDuration, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Value:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	ColourPickerCtrl1 = new wxColourPickerCtrl(this, ID_COLOURPICKERCTRL1, wxColour(0,0,0), wxDefaultPosition, wxDefaultSize, wxCLRP_SHOW_LABEL, wxDefaultValidator, _T("ID_COLOURPICKERCTRL1"));
	FlexGridSizer1->Add(ColourPickerCtrl1, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_LimitNodes = new wxCheckBox(this, ID_CHECKBOX1, _("Limit Nodes"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_LimitNodes->SetValue(false);
	FlexGridSizer1->Add(CheckBox_LimitNodes, 1, wxALL|wxEXPAND, 5);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Start Channel"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer1->Add(StaticText6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	TextCtrl_StartChannel = new wxTextCtrl(this, ID_TEXTCTRL3, _("1"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL3"));
	FlexGridSizer2->Add(TextCtrl_StartChannel, 1, wxALL|wxEXPAND, 5);
	StaticText_StartChannel = new wxStaticText(this, ID_STATICTEXT8, _("1"), wxDefaultPosition, wxSize(60,-1), 0, _T("ID_STATICTEXT8"));
	FlexGridSizer2->Add(StaticText_StartChannel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	StaticText7 = new wxStaticText(this, ID_STATICTEXT7, _("Nodes"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	FlexGridSizer1->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Nodes = new wxSpinCtrl(this, ID_SPINCTRL4, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 0, 100, 1, _T("ID_SPINCTRL4"));
	SpinCtrl_Nodes->SetValue(_T("1"));
	FlexGridSizer1->Add(SpinCtrl_Nodes, 1, wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Blend Mode:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_BlendMode = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	FlexGridSizer1->Add(Choice_BlendMode, 1, wxALL|wxEXPAND, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Priority:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer1->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Priority = new wxSpinCtrl(this, ID_SPINCTRL2, _T("5"), wxDefaultPosition, wxDefaultSize, 0, 1, 10, 5, _T("ID_SPINCTRL2"));
	SpinCtrl_Priority->SetValue(_T("5"));
	FlexGridSizer1->Add(SpinCtrl_Priority, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Delay:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Delay = new wxTextCtrl(this, ID_TEXTCTRL2, _("0.000"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer1->Add(TextCtrl_Delay, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemSetColourPanel::OnTextCtrl_SetColourDurationText);
	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&PlayListItemSetColourPanel::OnCheckBox_LimitChannelsClick);
	Connect(ID_TEXTCTRL3,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemSetColourPanel::OnTextCtrl_StartChannelText);
	//*)

    PopulateBlendModes(Choice_BlendMode);
    Choice_BlendMode->SetSelection(0);

    TextCtrl_SetColourDuration->SetValue(wxString::Format(wxT("%.3f"), (float)SetColour->GetDuration() / 1000.0));
    TextCtrl_Delay->SetValue(wxString::Format(wxT("%.3f"), (float)SetColour->GetDelay() / 1000.0));
    ColourPickerCtrl1->SetColour(SetColour->GetValue());
    SpinCtrl_Priority->SetValue(SetColour->GetPriority());
    Choice_BlendMode->SetSelection(SetColour->GetBlendMode());

    long channels = xScheduleFrame::GetScheduleManager()->GetTotalChannels();
    SpinCtrl_Nodes->SetRange(1, channels / 3);

    TextCtrl_StartChannel->SetValue(SetColour->GetStartChannel());
    int chs = SetColour->GetNodes();
    if (chs == 0)
    {
        CheckBox_LimitNodes->SetValue(false);
    }
    else
    {
        CheckBox_LimitNodes->SetValue(true);
        SpinCtrl_Nodes->SetValue(SetColour->GetNodes());
    }

    ValidateWindow();
}

PlayListItemSetColourPanel::~PlayListItemSetColourPanel()
{
	//(*Destroy(PlayListItemSetColourPanel)
	//*)
    _SetColour->SetDuration(wxAtof(TextCtrl_SetColourDuration->GetValue()) * 1000);
    _SetColour->SetValue(ColourPickerCtrl1->GetColour());
    _SetColour->SetDelay(wxAtof(TextCtrl_Delay->GetValue()) * 1000);
    _SetColour->SetBlendMode(Choice_BlendMode->GetStringSelection().ToStdString());
    _SetColour->SetPriority(SpinCtrl_Priority->GetValue());
    if (CheckBox_LimitNodes->GetValue())
    {
        _SetColour->SetStartChannel(TextCtrl_StartChannel->GetValue().ToStdString());
        _SetColour->SetNodes(SpinCtrl_Nodes->GetValue());
    }
    else
    {
        _SetColour->SetStartChannel("1");
        _SetColour->SetNodes(0);
    }
}

void PlayListItemSetColourPanel::OnTextCtrl_SetColourDurationText(wxCommandEvent& event)
{
}

void PlayListItemSetColourPanel::ValidateWindow()
{
    if (CheckBox_LimitNodes->GetValue())
    {
        TextCtrl_StartChannel->Enable();
        SpinCtrl_Nodes->Enable();
    }
    else
    {
        TextCtrl_StartChannel->Enable(false);
        SpinCtrl_Nodes->Enable(false);
    }
    StaticText_StartChannel->SetLabel(wxString::Format("%ld", _outputManager->DecodeStartChannel(TextCtrl_StartChannel->GetValue().ToStdString())));
}
void PlayListItemSetColourPanel::OnCheckBox_LimitChannelsClick(wxCommandEvent& event)
{
    ValidateWindow();
}

void PlayListItemSetColourPanel::OnTextCtrl_StartChannelText(wxCommandEvent& event)
{
    ValidateWindow();
}
