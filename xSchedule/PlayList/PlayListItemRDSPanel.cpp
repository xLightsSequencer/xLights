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
const long PlayListItemRDSPanel::ID_STATICTEXT8 = wxNewId();
const long PlayListItemRDSPanel::ID_CHOICE3 = wxNewId();
const long PlayListItemRDSPanel::ID_STATICTEXT2 = wxNewId();
const long PlayListItemRDSPanel::ID_TEXTCTRL1 = wxNewId();
const long PlayListItemRDSPanel::ID_STATICTEXT3 = wxNewId();
const long PlayListItemRDSPanel::ID_SPINCTRL1 = wxNewId();
const long PlayListItemRDSPanel::ID_STATICTEXT10 = wxNewId();
const long PlayListItemRDSPanel::ID_STATICTEXT4 = wxNewId();
const long PlayListItemRDSPanel::ID_CHOICE2 = wxNewId();
const long PlayListItemRDSPanel::ID_STATICTEXT5 = wxNewId();
const long PlayListItemRDSPanel::ID_TEXTCTRL2 = wxNewId();
const long PlayListItemRDSPanel::ID_STATICTEXT6 = wxNewId();
const long PlayListItemRDSPanel::ID_SPINCTRL2 = wxNewId();
const long PlayListItemRDSPanel::ID_CHECKBOX1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PlayListItemRDSPanel,wxPanel)
	//(*EventTable(PlayListItemRDSPanel)
	//*)
END_EVENT_TABLE()

void PlayListItemRDSPanel::SetChoiceFromString(wxChoice* choice, std::string value)
{
    int sel = choice->GetSelection();

    choice->SetSelection(-1);
    for (size_t i = 0; i < choice->GetCount(); i++)
    {
        if (choice->GetString(i) == value)
        {
            choice->SetSelection(i);
            return;
        }
    }

    choice->SetSelection(sel);
}

PlayListItemRDSPanel::PlayListItemRDSPanel(wxWindow* parent, PlayListItemRDS* rds, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _rds = rds;

	//(*Initialize(PlayListItemRDSPanel)
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxDefaultPosition, wxSize(396,287), wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Serial Port"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_CommPort = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	FlexGridSizer1->Add(Choice_CommPort, 1, wxALL|wxEXPAND, 5);
	StaticText8 = new wxStaticText(this, ID_STATICTEXT8, _("Speed"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
	FlexGridSizer1->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_SerialSpeed = new wxChoice(this, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE3"));
	Choice_SerialSpeed->Append(_("2400"));
	Choice_SerialSpeed->Append(_("4800"));
	Choice_SerialSpeed->Append(_("9600"));
	Choice_SerialSpeed->SetSelection( Choice_SerialSpeed->Append(_("19200")) );
	FlexGridSizer1->Add(Choice_SerialSpeed, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Station Name"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_StationName = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	TextCtrl_StationName->SetMaxLength(8);
	FlexGridSizer1->Add(TextCtrl_StationName, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Station Duration"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	SpinCtrl_StationDuration = new wxSpinCtrl(this, ID_SPINCTRL1, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 0, 255, 0, _T("ID_SPINCTRL1"));
	SpinCtrl_StationDuration->SetValue(_T("0"));
	FlexGridSizer2->Add(SpinCtrl_StationDuration, 1, wxALL|wxEXPAND, 5);
	StaticText_StationDuration = new wxStaticText(this, ID_STATICTEXT10, _("0 Secs"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT10"));
	FlexGridSizer2->Add(StaticText_StationDuration, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Scrolling Mode"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_ScrollMode = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
	Choice_ScrollMode->Append(_("Mode 0 - Fixed 8 Characters"));
	Choice_ScrollMode->SetSelection( Choice_ScrollMode->Append(_("Mode 1 - Scroll 1 Character")) );
	Choice_ScrollMode->Append(_("Mode 2 - Scroll Word Aligned"));
	Choice_ScrollMode->Append(_("Mode 3 - Scroll 1 Character Leading Spaces"));
	FlexGridSizer1->Add(Choice_ScrollMode, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Text"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer1->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Text = new wxTextCtrl(this, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	TextCtrl_Text->SetMaxLength(80);
	FlexGridSizer1->Add(TextCtrl_Text, 1, wxALL|wxEXPAND, 5);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Line Duration"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer1->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	SpinCtrl_TextDuration = new wxSpinCtrl(this, ID_SPINCTRL2, _T("2"), wxDefaultPosition, wxDefaultSize, 0, 0, 255, 2, _T("ID_SPINCTRL2"));
	SpinCtrl_TextDuration->SetValue(_T("2"));
	FlexGridSizer3->Add(SpinCtrl_TextDuration, 1, wxALL|wxEXPAND, 5);
	StaticText_TextDuration = new wxStaticText(this, wxID_ANY, _("1.08 secs"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer3->Add(StaticText_TextDuration, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_HighSpeed = new wxCheckBox(this, ID_CHECKBOX1, _("High speed scrolling"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_HighSpeed->SetValue(false);
	FlexGridSizer1->Add(CheckBox_HighSpeed, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	SetSizer(FlexGridSizer1);
	Layout();

	Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&PlayListItemRDSPanel::OnChoice_ScrollModeSelect);
	Connect(ID_CHOICE2,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&PlayListItemRDSPanel::OnChoice_ScrollModeSelect);
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
    SpinCtrl_StationDuration->SetValue(_rds->GetStationDuration());
    SpinCtrl_TextDuration->SetValue(_rds->GetLineDuration());
    SetChoiceFromString(Choice_CommPort, _rds->GetCommPort());
    SetChoiceFromString(Choice_ScrollMode, _rds->GetMode());
    SetChoiceFromString(Choice_SerialSpeed, wxString::Format(wxT("%i"), _rds->GetSerialSpeed()).ToStdString());
    CheckBox_HighSpeed->SetValue(_rds->GetHighSpeed());

    ValidateWindow();
}

PlayListItemRDSPanel::~PlayListItemRDSPanel()
{
	//(*Destroy(PlayListItemRDSPanel)
	//*)
    _rds->SetStationName(TextCtrl_StationName->GetValue().ToStdString());
    _rds->SetText(TextCtrl_Text->GetValue().ToStdString());
    _rds->SetStationDuration(SpinCtrl_StationDuration->GetValue());
    _rds->SetLineDuration(SpinCtrl_TextDuration->GetValue());
    _rds->SetCommPort(Choice_CommPort->GetStringSelection().ToStdString());
    _rds->SetMode(Choice_ScrollMode->GetStringSelection().ToStdString());
    _rds->SetSerialSpeed(wxAtoi(Choice_SerialSpeed->GetStringSelection()));
    _rds->SetHighSpeed(CheckBox_HighSpeed->GetValue());
}

void PlayListItemRDSPanel::OnChoice_ScrollModeSelect(wxCommandEvent& event)
{
    ValidateWindow();
}

void PlayListItemRDSPanel::OnSpinCtrl_StationDurationChange(wxSpinEvent& event)
{
    StaticText_StationDuration->SetLabel(wxString::Format(wxT("%.1f secs"), SpinCtrl_StationDuration->GetValue() * 2.7f));
    ValidateWindow();
}

void PlayListItemRDSPanel::OnSpinCtrl_TextDurationChange(wxSpinEvent& event)
{
    StaticText_TextDuration->SetLabel(wxString::Format(wxT("%.2f secs"), SpinCtrl_TextDuration->GetValue() * 0.54f));
    ValidateWindow();
}

void PlayListItemRDSPanel::ValidateWindow()
{
    if (Choice_ScrollMode->GetStringSelection() == "Mode 1 - Scroll 1 Character" ||
        Choice_ScrollMode->GetStringSelection() == "Mode 3 - Scroll 1 Character Leading Spaces")
    {
        CheckBox_HighSpeed->Enable();
        SpinCtrl_TextDuration->Enable(false);
    }
    else
    {
        SpinCtrl_TextDuration->Enable();
        CheckBox_HighSpeed->Enable(false);
    }
}