#include "PlayListItemColourOrganPanel.h"
#include "PlayListItemColourOrgan.h"
#include "PlayListDialog.h"
#include "../xScheduleMain.h"
#include "../ScheduleManager.h"
#include "../../xLights/outputs/OutputManager.h"

//(*InternalHeaders(PlayListItemColourOrganPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(PlayListItemColourOrganPanel)
const long PlayListItemColourOrganPanel::ID_STATICTEXT1 = wxNewId();
const long PlayListItemColourOrganPanel::ID_TEXTCTRL1 = wxNewId();
const long PlayListItemColourOrganPanel::ID_STATICTEXT2 = wxNewId();
const long PlayListItemColourOrganPanel::ID_CHOICE1 = wxNewId();
const long PlayListItemColourOrganPanel::ID_STATICTEXT3 = wxNewId();
const long PlayListItemColourOrganPanel::ID_COLOURPICKERCTRL1 = wxNewId();
const long PlayListItemColourOrganPanel::ID_STATICTEXT10 = wxNewId();
const long PlayListItemColourOrganPanel::ID_CHOICE4 = wxNewId();
const long PlayListItemColourOrganPanel::ID_STATICTEXT5 = wxNewId();
const long PlayListItemColourOrganPanel::ID_TEXTCTRL4 = wxNewId();
const long PlayListItemColourOrganPanel::ID_STATICTEXT9 = wxNewId();
const long PlayListItemColourOrganPanel::ID_STATICTEXT6 = wxNewId();
const long PlayListItemColourOrganPanel::ID_SPINCTRL4 = wxNewId();
const long PlayListItemColourOrganPanel::ID_STATICTEXT4 = wxNewId();
const long PlayListItemColourOrganPanel::ID_SPINCTRL1 = wxNewId();
const long PlayListItemColourOrganPanel::ID_STATICTEXT8 = wxNewId();
const long PlayListItemColourOrganPanel::ID_SPINCTRL2 = wxNewId();
const long PlayListItemColourOrganPanel::ID_STATICTEXT11 = wxNewId();
const long PlayListItemColourOrganPanel::ID_SPINCTRL3 = wxNewId();
const long PlayListItemColourOrganPanel::ID_STATICTEXT12 = wxNewId();
const long PlayListItemColourOrganPanel::ID_SPINCTRL5 = wxNewId();
const long PlayListItemColourOrganPanel::ID_STATICTEXT7 = wxNewId();
const long PlayListItemColourOrganPanel::ID_TEXTCTRL2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PlayListItemColourOrganPanel,wxPanel)
	//(*EventTable(PlayListItemColourOrganPanel)
	//*)
END_EVENT_TABLE()

PlayListItemColourOrganPanel::PlayListItemColourOrganPanel(wxWindow* parent, OutputManager* outputManager, PlayListItemColourOrgan* ColourOrgan, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _outputManager = outputManager;
    _ColourOrgan = ColourOrgan;

	//(*Initialize(PlayListItemColourOrganPanel)
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
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Start Note:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_StartNote = new wxSpinCtrl(this, ID_SPINCTRL1, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 127, 1, _T("ID_SPINCTRL1"));
	SpinCtrl_StartNote->SetValue(_T("1"));
	FlexGridSizer1->Add(SpinCtrl_StartNote, 1, wxALL|wxEXPAND, 5);
	StaticText8 = new wxStaticText(this, ID_STATICTEXT8, _("End Note:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
	FlexGridSizer1->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_EndNote = new wxSpinCtrl(this, ID_SPINCTRL2, _T("127"), wxDefaultPosition, wxDefaultSize, 0, 1, 127, 127, _T("ID_SPINCTRL2"));
	SpinCtrl_EndNote->SetValue(_T("127"));
	FlexGridSizer1->Add(SpinCtrl_EndNote, 1, wxALL|wxEXPAND, 5);
	StaticText9 = new wxStaticText(this, ID_STATICTEXT11, _("Fade Frames:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT11"));
	FlexGridSizer1->Add(StaticText9, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_FadeFrames = new wxSpinCtrl(this, ID_SPINCTRL3, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 0, 1000, 0, _T("ID_SPINCTRL3"));
	SpinCtrl_FadeFrames->SetValue(_T("0"));
	FlexGridSizer1->Add(SpinCtrl_FadeFrames, 1, wxALL|wxEXPAND, 5);
	StaticText11 = new wxStaticText(this, ID_STATICTEXT12, _("Priority:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT12"));
	FlexGridSizer1->Add(StaticText11, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Priority = new wxSpinCtrl(this, ID_SPINCTRL5, _T("5"), wxDefaultPosition, wxDefaultSize, 0, 1, 10, 5, _T("ID_SPINCTRL5"));
	SpinCtrl_Priority->SetValue(_T("5"));
	FlexGridSizer1->Add(SpinCtrl_Priority, 1, wxALL|wxEXPAND, 5);
	StaticText7 = new wxStaticText(this, ID_STATICTEXT7, _("Duration:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	FlexGridSizer1->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Duration = new wxTextCtrl(this, ID_TEXTCTRL2, _("60.000"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer1->Add(TextCtrl_Duration, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemColourOrganPanel::OnTextCtrl_NameText);
	Connect(ID_TEXTCTRL4,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemColourOrganPanel::OnTextCtrl_StartChannelText);
	//*)

    PopulateBlendModes(Choice_BlendMode);
    Choice_BlendMode->SetSelection(0);

    long channels = xScheduleFrame::GetScheduleManager()->GetTotalChannels();
    SpinCtrl_Channels->SetRange(1, channels / 3);

    TextCtrl_Name->SetValue(_ColourOrgan->GetRawName());
    Choice_Mode->SetStringSelection(_ColourOrgan->GetMode());
    ColourPickerCtrl1->SetColour(_ColourOrgan->GetColour());
    TextCtrl_StartChannel->SetValue(_ColourOrgan->GetStartChannel());
    SpinCtrl_Channels->SetValue(_ColourOrgan->GetPixels());
    TextCtrl_Duration->SetValue(wxString::Format(wxT("%.3f"), (float)_ColourOrgan->GetDurationMS() / 1000));
    Choice_BlendMode->SetSelection(_ColourOrgan->GetBlendMode());
    SpinCtrl_StartNote->SetValue(_ColourOrgan->GetStartNote());
    SpinCtrl_EndNote->SetValue(_ColourOrgan->GetEndNote());
    SpinCtrl_FadeFrames->SetValue(_ColourOrgan->GetFadeFrames());
	SpinCtrl_Priority->SetValue(_ColourOrgan->GetPriority());

    ValidateWindow();
}

PlayListItemColourOrganPanel::~PlayListItemColourOrganPanel()
{
	//(*Destroy(PlayListItemColourOrganPanel)
	//*)
    _ColourOrgan->SetName(TextCtrl_Name->GetValue().ToStdString());
    _ColourOrgan->SetMode(Choice_Mode->GetStringSelection().ToStdString());
    _ColourOrgan->SetColour(ColourPickerCtrl1->GetColour());
    _ColourOrgan->SetStartChannel(TextCtrl_StartChannel->GetValue().ToStdString());
    _ColourOrgan->SetPixels(SpinCtrl_Channels->GetValue());
    _ColourOrgan->SetDuration(wxAtof(TextCtrl_Duration->GetValue()) * 1000);
    _ColourOrgan->SetBlendMode(Choice_BlendMode->GetStringSelection().ToStdString());
    _ColourOrgan->SetStartNote(SpinCtrl_StartNote->GetValue());
    _ColourOrgan->SetEndNote(SpinCtrl_EndNote->GetValue());
    _ColourOrgan->SetFadeFrames(SpinCtrl_FadeFrames->GetValue());
	_ColourOrgan->SetPriority(SpinCtrl_Priority->GetValue());
}

void PlayListItemColourOrganPanel::OnTextCtrl_NameText(wxCommandEvent& event)
{
    _ColourOrgan->SetName(TextCtrl_Name->GetValue().ToStdString());
    wxCommandEvent e(EVT_UPDATEITEMNAME);
    wxPostEvent(GetParent()->GetParent()->GetParent()->GetParent(), e);
}

void PlayListItemColourOrganPanel::OnChoice_ModeSelect(wxCommandEvent& event)
{
    ValidateWindow();
}

void PlayListItemColourOrganPanel::ValidateWindow()
{
    std::string mode = Choice_Mode->GetStringSelection().ToStdString();

    StaticText_StartChannel->SetLabel(wxString::Format("%ld", _outputManager->DecodeStartChannel(TextCtrl_StartChannel->GetValue().ToStdString())));
}

void PlayListItemColourOrganPanel::OnTextCtrl_StartChannelText(wxCommandEvent& event)
{
    ValidateWindow();
}

void PlayListItemColourOrganPanel::OnSpinCtrl_ChannelsChange(wxSpinEvent& event)
{
    ValidateWindow();
}

void PlayListItemColourOrganPanel::OnSpinCtrl_StartNoteChange(wxSpinEvent& event)
{
    if (SpinCtrl_StartNote->GetValue() > SpinCtrl_EndNote->GetValue()) SpinCtrl_EndNote->SetValue(SpinCtrl_StartNote->GetValue());
    ValidateWindow();
}

void PlayListItemColourOrganPanel::OnSpinCtrl_EndNoteChange(wxSpinEvent& event)
{
    if (SpinCtrl_EndNote->GetValue() < SpinCtrl_StartNote->GetValue()) SpinCtrl_StartNote->SetValue(SpinCtrl_EndNote->GetValue());
    ValidateWindow();
}
