#include "PlayListItemFadePanel.h"
#include "PlayListItemFade.h"
#include "../xScheduleMain.h"
#include "../ScheduleManager.h"
#include "../../xLights/outputs/OutputManager.h"

//(*InternalHeaders(PlayListItemFadePanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(PlayListItemFadePanel)
const long PlayListItemFadePanel::ID_STATICTEXT1 = wxNewId();
const long PlayListItemFadePanel::ID_TEXTCTRL1 = wxNewId();
const long PlayListItemFadePanel::ID_CHECKBOX2 = wxNewId();
const long PlayListItemFadePanel::ID_STATICTEXT3 = wxNewId();
const long PlayListItemFadePanel::ID_CHOICE1 = wxNewId();
const long PlayListItemFadePanel::ID_CHECKBOX1 = wxNewId();
const long PlayListItemFadePanel::ID_STATICTEXT6 = wxNewId();
const long PlayListItemFadePanel::ID_TEXTCTRL3 = wxNewId();
const long PlayListItemFadePanel::ID_STATICTEXT8 = wxNewId();
const long PlayListItemFadePanel::ID_STATICTEXT7 = wxNewId();
const long PlayListItemFadePanel::ID_SPINCTRL4 = wxNewId();
const long PlayListItemFadePanel::ID_STATICTEXT5 = wxNewId();
const long PlayListItemFadePanel::ID_SPINCTRL2 = wxNewId();
const long PlayListItemFadePanel::ID_STATICTEXT2 = wxNewId();
const long PlayListItemFadePanel::ID_TEXTCTRL2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PlayListItemFadePanel,wxPanel)
	//(*EventTable(PlayListItemFadePanel)
	//*)
END_EVENT_TABLE()

PlayListItemFadePanel::PlayListItemFadePanel(wxWindow* parent, OutputManager* outputManager, PlayListItemFade* Fade, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _outputManager = outputManager;
    _Fade = Fade;

	//(*Initialize(PlayListItemFadePanel)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Duration:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_FadeDuration = new wxTextCtrl(this, ID_TEXTCTRL1, _("0.050"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(TextCtrl_FadeDuration, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_RemainderOfStepTime = new wxCheckBox(this, ID_CHECKBOX2, _("Remainder of step time"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
	CheckBox_RemainderOfStepTime->SetValue(false);
	FlexGridSizer1->Add(CheckBox_RemainderOfStepTime, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Mode:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_FadeMode = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	Choice_FadeMode->Append(_("Fade In"));
	Choice_FadeMode->SetSelection( Choice_FadeMode->Append(_("Fade Out")) );
	FlexGridSizer1->Add(Choice_FadeMode, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_LimitChannels = new wxCheckBox(this, ID_CHECKBOX1, _("Limit Channels"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_LimitChannels->SetValue(false);
	FlexGridSizer1->Add(CheckBox_LimitChannels, 1, wxALL|wxEXPAND, 5);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Start Channel"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer1->Add(StaticText6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	TextCtrl_StartChannel = new wxTextCtrl(this, ID_TEXTCTRL3, _("1"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL3"));
	FlexGridSizer2->Add(TextCtrl_StartChannel, 1, wxALL|wxEXPAND, 5);
	StaticText_StartChannel = new wxStaticText(this, ID_STATICTEXT8, _("1"), wxDefaultPosition, wxSize(60,-1), 0, _T("ID_STATICTEXT8"));
	FlexGridSizer2->Add(StaticText_StartChannel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	StaticText7 = new wxStaticText(this, ID_STATICTEXT7, _("Channels"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	FlexGridSizer1->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Channels = new wxSpinCtrl(this, ID_SPINCTRL4, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 0, 100, 1, _T("ID_SPINCTRL4"));
	SpinCtrl_Channels->SetValue(_T("1"));
	FlexGridSizer1->Add(SpinCtrl_Channels, 1, wxALL|wxEXPAND, 5);
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

	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemFadePanel::OnTextCtrl_FadeDurationText);
	Connect(ID_CHECKBOX2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&PlayListItemFadePanel::OnCheckBox_RemainderOfStepTimeClick);
	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&PlayListItemFadePanel::OnCheckBox_LimitChannelsClick);
	Connect(ID_TEXTCTRL3,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemFadePanel::OnTextCtrl_StartChannelText);
	//*)

    TextCtrl_FadeDuration->SetValue(wxString::Format(wxT("%.3f"), (float)Fade->GetDuration() / 1000.0));
    TextCtrl_Delay->SetValue(wxString::Format(wxT("%.3f"), (float)Fade->GetDelay() / 1000.0));
	if (Fade->GetFadeDirection() == FADEDIRECTION::FADE_IN)
	{
		Choice_FadeMode->SetStringSelection("Fade In");
	}
	else
	{
		Choice_FadeMode->SetStringSelection("Fade Out");
	}
    SpinCtrl_Priority->SetValue(Fade->GetPriority());

    long channels = xScheduleFrame::GetScheduleManager()->GetTotalChannels();
    SpinCtrl_Channels->SetRange(1, channels);

    TextCtrl_StartChannel->SetValue(Fade->GetStartChannel());
    int chs = Fade->GetChannels();
    if (chs == 0)
    {
        CheckBox_LimitChannels->SetValue(false);
    }
    else
    {
        CheckBox_LimitChannels->SetValue(true);
        SpinCtrl_Channels->SetValue(Fade->GetChannels());
    }
    CheckBox_RemainderOfStepTime->SetValue(Fade->GetRestOfStep());

    ValidateWindow();
}

PlayListItemFadePanel::~PlayListItemFadePanel()
{
	//(*Destroy(PlayListItemFadePanel)
	//*)
    _Fade->SetDuration(wxAtof(TextCtrl_FadeDuration->GetValue()) * 1000);
	if (Choice_FadeMode->GetStringSelection() == "Fade In")
	{
		_Fade->SetFadeDirection(FADEDIRECTION::FADE_IN);
	}
	else
	{
		_Fade->SetFadeDirection(FADEDIRECTION::FADE_OUT);
	}
    _Fade->SetRestOfStep(CheckBox_RemainderOfStepTime->GetValue());
    _Fade->SetDelay(wxAtof(TextCtrl_Delay->GetValue()) * 1000);
    _Fade->SetPriority(SpinCtrl_Priority->GetValue());
    if (CheckBox_LimitChannels->GetValue())
    {
        _Fade->SetStartChannel(TextCtrl_StartChannel->GetValue().ToStdString());
        _Fade->SetChannels(SpinCtrl_Channels->GetValue());
    }
    else
    {
        _Fade->SetStartChannel("1");
        _Fade->SetChannels(0);
    }
}

void PlayListItemFadePanel::OnTextCtrl_FadeDurationText(wxCommandEvent& event)
{
}

void PlayListItemFadePanel::ValidateWindow()
{
    if (CheckBox_LimitChannels->GetValue())
    {
        TextCtrl_StartChannel->Enable();
        SpinCtrl_Channels->Enable();
    }
    else
    {
        TextCtrl_StartChannel->Enable(false);
        SpinCtrl_Channels->Enable(false);
    }
    StaticText_StartChannel->SetLabel(wxString::Format("%ld", _outputManager->DecodeStartChannel(TextCtrl_StartChannel->GetValue().ToStdString())));

    if (CheckBox_RemainderOfStepTime->GetValue())
    {
        TextCtrl_FadeDuration->Enable(false);
    }
    else
    {
        TextCtrl_FadeDuration->Enable();
    }
}

void PlayListItemFadePanel::OnCheckBox_LimitChannelsClick(wxCommandEvent& event)
{
    ValidateWindow();
}

void PlayListItemFadePanel::OnTextCtrl_StartChannelText(wxCommandEvent& event)
{
    ValidateWindow();
}

void PlayListItemFadePanel::OnCheckBox_RemainderOfStepTimeClick(wxCommandEvent& event)
{
    ValidateWindow();
}
