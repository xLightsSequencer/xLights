#include "PlayListItemMicrophonePanel.h"
#include "PlayListItemMicrophone.h"
#include "PlayListDialog.h"
#include "../xScheduleMain.h"
#include "../ScheduleManager.h"
#include "../../xLights/outputs/OutputManager.h"

//(*InternalHeaders(PlayListItemMicrophonePanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(PlayListItemMicrophonePanel)
const long PlayListItemMicrophonePanel::ID_STATICTEXT1 = wxNewId();
const long PlayListItemMicrophonePanel::ID_TEXTCTRL1 = wxNewId();
const long PlayListItemMicrophonePanel::ID_STATICTEXT2 = wxNewId();
const long PlayListItemMicrophonePanel::ID_CHOICE1 = wxNewId();
const long PlayListItemMicrophonePanel::ID_STATICTEXT3 = wxNewId();
const long PlayListItemMicrophonePanel::ID_COLOURPICKERCTRL1 = wxNewId();
const long PlayListItemMicrophonePanel::ID_STATICTEXT10 = wxNewId();
const long PlayListItemMicrophonePanel::ID_CHOICE4 = wxNewId();
const long PlayListItemMicrophonePanel::ID_STATICTEXT5 = wxNewId();
const long PlayListItemMicrophonePanel::ID_TEXTCTRL4 = wxNewId();
const long PlayListItemMicrophonePanel::ID_STATICTEXT9 = wxNewId();
const long PlayListItemMicrophonePanel::ID_STATICTEXT6 = wxNewId();
const long PlayListItemMicrophonePanel::ID_SPINCTRL4 = wxNewId();
const long PlayListItemMicrophonePanel::ID_STATICTEXT4 = wxNewId();
const long PlayListItemMicrophonePanel::ID_SPINCTRL1 = wxNewId();
const long PlayListItemMicrophonePanel::ID_STATICTEXT7 = wxNewId();
const long PlayListItemMicrophonePanel::ID_TEXTCTRL2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PlayListItemMicrophonePanel,wxPanel)
	//(*EventTable(PlayListItemMicrophonePanel)
	//*)
END_EVENT_TABLE()

PlayListItemMicrophonePanel::PlayListItemMicrophonePanel(wxWindow* parent, OutputManager* outputManager, PlayListItemMicrophone* microphone, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _outputManager = outputManager;
    _microphone = microphone;

	//(*Initialize(PlayListItemMicrophonePanel)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Name = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(TextCtrl_Name, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Mode:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Mode = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	Choice_Mode->SetSelection( Choice_Mode->Append(_("Maximum")) );
	FlexGridSizer1->Add(Choice_Mode, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Colour:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	ColourPickerCtrl1 = new wxColourPickerCtrl(this, ID_COLOURPICKERCTRL1, wxColour(255,255,255), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_COLOURPICKERCTRL1"));
	FlexGridSizer1->Add(ColourPickerCtrl1, 1, wxALL|wxEXPAND, 5);
	StaticText10 = new wxStaticText(this, ID_STATICTEXT10, _("Blend Mode:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT10"));
	FlexGridSizer1->Add(StaticText10, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_BlendMode = new wxChoice(this, ID_CHOICE4, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE4"));
	FlexGridSizer1->Add(Choice_BlendMode, 1, wxALL|wxEXPAND, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Start Channel:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer1->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	TextCtrl_StartChannel = new wxTextCtrl(this, ID_TEXTCTRL4, _("1"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL4"));
	FlexGridSizer2->Add(TextCtrl_StartChannel, 1, wxALL|wxEXPAND, 5);
	StaticText_StartChannel = new wxStaticText(this, ID_STATICTEXT9, _("1"), wxDefaultPosition, wxSize(60,-1), 0, _T("ID_STATICTEXT9"));
	FlexGridSizer2->Add(StaticText_StartChannel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Pixels:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer1->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Channels = new wxSpinCtrl(this, ID_SPINCTRL4, _T("100"), wxDefaultPosition, wxDefaultSize, 0, 1, 100, 100, _T("ID_SPINCTRL4"));
	SpinCtrl_Channels->SetValue(_T("100"));
	FlexGridSizer1->Add(SpinCtrl_Channels, 1, wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Priority:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Priority = new wxSpinCtrl(this, ID_SPINCTRL1, _T("5"), wxDefaultPosition, wxDefaultSize, 0, 1, 10, 5, _T("ID_SPINCTRL1"));
	SpinCtrl_Priority->SetValue(_T("5"));
	FlexGridSizer1->Add(SpinCtrl_Priority, 1, wxALL|wxEXPAND, 5);
	StaticText7 = new wxStaticText(this, ID_STATICTEXT7, _("Duration:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	FlexGridSizer1->Add(StaticText7, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Duration = new wxTextCtrl(this, ID_TEXTCTRL2, _("60.000"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer1->Add(TextCtrl_Duration, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemMicrophonePanel::OnTextCtrl_NameText);
	Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&PlayListItemMicrophonePanel::OnChoice_ModeSelect);
	Connect(ID_TEXTCTRL4,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemMicrophonePanel::OnTextCtrl_StartChannelText);
	//*)

    PopulateBlendModes(Choice_BlendMode);
    Choice_BlendMode->SetSelection(0);

    long channels = xScheduleFrame::GetScheduleManager()->GetTotalChannels();
    SpinCtrl_Channels->SetRange(1, channels / 3);

    TextCtrl_Name->SetValue(_microphone->GetRawName());
    Choice_Mode->SetStringSelection(_microphone->GetMode());
    ColourPickerCtrl1->SetColour(_microphone->GetColour());
    TextCtrl_StartChannel->SetValue(_microphone->GetStartChannel());
    SpinCtrl_Channels->SetValue(_microphone->GetPixels());
    TextCtrl_Duration->SetValue(wxString::Format(wxT("%.3f"), (float)_microphone->GetDurationMS() / 1000));
    Choice_BlendMode->SetSelection(_microphone->GetBlendMode());
	SpinCtrl_Priority->SetValue(_microphone->GetPriority());

    ValidateWindow();
}

PlayListItemMicrophonePanel::~PlayListItemMicrophonePanel()
{
	//(*Destroy(PlayListItemMicrophonePanel)
	//*)
    _microphone->SetName(TextCtrl_Name->GetValue().ToStdString());
    _microphone->SetMode(Choice_Mode->GetStringSelection().ToStdString());
    _microphone->SetColour(ColourPickerCtrl1->GetColour());
    _microphone->SetStartChannel(TextCtrl_StartChannel->GetValue().ToStdString());
    _microphone->SetPixels(SpinCtrl_Channels->GetValue());
    _microphone->SetDuration(wxAtof(TextCtrl_Duration->GetValue()) * 1000);
    _microphone->SetBlendMode(Choice_BlendMode->GetStringSelection().ToStdString());
	_microphone->SetPriority(SpinCtrl_Priority->GetValue());
}

void PlayListItemMicrophonePanel::OnTextCtrl_NameText(wxCommandEvent& event)
{
    _microphone->SetName(TextCtrl_Name->GetValue().ToStdString());
    wxCommandEvent e(EVT_UPDATEITEMNAME);
    wxPostEvent(GetParent()->GetParent()->GetParent()->GetParent(), e);
}

void PlayListItemMicrophonePanel::OnChoice_ModeSelect(wxCommandEvent& event)
{
    ValidateWindow();
}

void PlayListItemMicrophonePanel::ValidateWindow()
{
    std::string mode = Choice_Mode->GetStringSelection().ToStdString();

    StaticText_StartChannel->SetLabel(wxString::Format("%ld", _outputManager->DecodeStartChannel(TextCtrl_StartChannel->GetValue().ToStdString())));
}

void PlayListItemMicrophonePanel::OnTextCtrl_StartChannelText(wxCommandEvent& event)
{
    ValidateWindow();
}
