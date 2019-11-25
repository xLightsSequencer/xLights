#include "PlayListItemDimPanel.h"
#include "PlayListItemDim.h"
#include "PlayListDialog.h"
#include "../xScheduleMain.h"
#include "../ScheduleManager.h"
#include "../../xLights/outputs/OutputManager.h"

//(*InternalHeaders(PlayListItemDimPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(PlayListItemDimPanel)
const long PlayListItemDimPanel::ID_STATICTEXT1 = wxNewId();
const long PlayListItemDimPanel::ID_TEXTCTRL1 = wxNewId();
const long PlayListItemDimPanel::ID_STATICTEXT3 = wxNewId();
const long PlayListItemDimPanel::ID_SPINCTRL1 = wxNewId();
const long PlayListItemDimPanel::ID_STATICTEXT5 = wxNewId();
const long PlayListItemDimPanel::ID_TEXTCTRL4 = wxNewId();
const long PlayListItemDimPanel::ID_STATICTEXT9 = wxNewId();
const long PlayListItemDimPanel::ID_STATICTEXT6 = wxNewId();
const long PlayListItemDimPanel::ID_SPINCTRL4 = wxNewId();
const long PlayListItemDimPanel::ID_STATICTEXT4 = wxNewId();
const long PlayListItemDimPanel::ID_SPINCTRL2 = wxNewId();
const long PlayListItemDimPanel::ID_STATICTEXT7 = wxNewId();
const long PlayListItemDimPanel::ID_TEXTCTRL2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PlayListItemDimPanel,wxPanel)
	//(*EventTable(PlayListItemDimPanel)
	//*)
END_EVENT_TABLE()

PlayListItemDimPanel::PlayListItemDimPanel(wxWindow* parent, OutputManager* outputManager, PlayListItemDim* dim, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _outputManager = outputManager;
    _dim = dim;

	//(*Initialize(PlayListItemDimPanel)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Name = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(TextCtrl_Name, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Dim:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Dim = new wxSpinCtrl(this, ID_SPINCTRL1, _T("100"), wxDefaultPosition, wxDefaultSize, 0, 0, 100, 100, _T("ID_SPINCTRL1"));
	SpinCtrl_Dim->SetValue(_T("100"));
	FlexGridSizer1->Add(SpinCtrl_Dim, 1, wxALL|wxEXPAND, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Start Channel:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer1->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	TextCtrl_StartChannel = new wxTextCtrl(this, ID_TEXTCTRL4, _("1"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL4"));
	FlexGridSizer2->Add(TextCtrl_StartChannel, 1, wxALL|wxEXPAND, 5);
	StaticText_StartChannel = new wxStaticText(this, ID_STATICTEXT9, _("1"), wxDefaultPosition, wxSize(60,-1), 0, _T("ID_STATICTEXT9"));
	FlexGridSizer2->Add(StaticText_StartChannel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Channels:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer1->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Channels = new wxSpinCtrl(this, ID_SPINCTRL4, _T("100"), wxDefaultPosition, wxDefaultSize, 0, 1, 100, 100, _T("ID_SPINCTRL4"));
	SpinCtrl_Channels->SetValue(_T("100"));
	FlexGridSizer1->Add(SpinCtrl_Channels, 1, wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Priority:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Priority = new wxSpinCtrl(this, ID_SPINCTRL2, _T("5"), wxDefaultPosition, wxDefaultSize, 0, 1, 10, 5, _T("ID_SPINCTRL2"));
	SpinCtrl_Priority->SetValue(_T("5"));
	FlexGridSizer1->Add(SpinCtrl_Priority, 1, wxALL|wxEXPAND, 5);
	StaticText7 = new wxStaticText(this, ID_STATICTEXT7, _("Duration:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	FlexGridSizer1->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Duration = new wxTextCtrl(this, ID_TEXTCTRL2, _("60.000"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer1->Add(TextCtrl_Duration, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemDimPanel::OnTextCtrl_NameText);
	Connect(ID_TEXTCTRL4,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemDimPanel::OnTextCtrl_StartChannelText);
	//*)

    long channels = xScheduleFrame::GetScheduleManager()->GetTotalChannels();
    SpinCtrl_Channels->SetRange(1, channels);

    TextCtrl_Name->SetValue(dim->GetRawName());
    SpinCtrl_Dim->SetValue(dim->GetDim());
    TextCtrl_StartChannel->SetValue(dim->GetStartChannel());
    SpinCtrl_Channels->SetValue(dim->GetChannels());
    TextCtrl_Duration->SetValue(wxString::Format(wxT("%.3f"), (float)dim->GetDurationMS() / 1000));
    SpinCtrl_Priority->SetValue(dim->GetPriority());

    ValidateWindow();
}

PlayListItemDimPanel::~PlayListItemDimPanel()
{
	//(*Destroy(PlayListItemDimPanel)
	//*)
    _dim->SetName(TextCtrl_Name->GetValue().ToStdString());
    _dim->SetDim(SpinCtrl_Dim->GetValue());
    _dim->SetStartChannel(TextCtrl_StartChannel->GetValue().ToStdString());
    _dim->SetChannels(SpinCtrl_Channels->GetValue());
    _dim->SetDuration(wxAtof(TextCtrl_Duration->GetValue()) * 1000);
    _dim->SetPriority(SpinCtrl_Priority->GetValue());
}

void PlayListItemDimPanel::OnTextCtrl_NameText(wxCommandEvent& event)
{
    _dim->SetName(TextCtrl_Name->GetValue().ToStdString());
    wxCommandEvent e(EVT_UPDATEITEMNAME);
    wxPostEvent(GetParent()->GetParent()->GetParent()->GetParent(), e);
}

void PlayListItemDimPanel::OnChoice_ModeSelect(wxCommandEvent& event)
{
    ValidateWindow();
}

void PlayListItemDimPanel::ValidateWindow()
{
    StaticText_StartChannel->SetLabel(wxString::Format("%ld", _outputManager->DecodeStartChannel(TextCtrl_StartChannel->GetValue().ToStdString())));
}

void PlayListItemDimPanel::OnTextCtrl_StartChannelText(wxCommandEvent& event)
{
    ValidateWindow();
}
