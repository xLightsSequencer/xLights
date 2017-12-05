#include "PlayListItemTestPanel.h"
#include "PlayListItemTest.h"
#include "PlayListDialog.h"
#include "../xScheduleMain.h"
#include "../ScheduleManager.h"
#include "../../xLights/outputs/OutputManager.h"

//(*InternalHeaders(PlayListItemTestPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(PlayListItemTestPanel)
const long PlayListItemTestPanel::ID_STATICTEXT1 = wxNewId();
const long PlayListItemTestPanel::ID_TEXTCTRL1 = wxNewId();
const long PlayListItemTestPanel::ID_STATICTEXT2 = wxNewId();
const long PlayListItemTestPanel::ID_CHOICE1 = wxNewId();
const long PlayListItemTestPanel::ID_STATICTEXT3 = wxNewId();
const long PlayListItemTestPanel::ID_SPINCTRL1 = wxNewId();
const long PlayListItemTestPanel::ID_STATICTEXT4 = wxNewId();
const long PlayListItemTestPanel::ID_SPINCTRL2 = wxNewId();
const long PlayListItemTestPanel::ID_STATICTEXT5 = wxNewId();
const long PlayListItemTestPanel::ID_TEXTCTRL4 = wxNewId();
const long PlayListItemTestPanel::ID_STATICTEXT9 = wxNewId();
const long PlayListItemTestPanel::ID_STATICTEXT6 = wxNewId();
const long PlayListItemTestPanel::ID_SPINCTRL4 = wxNewId();
const long PlayListItemTestPanel::ID_STATICTEXT7 = wxNewId();
const long PlayListItemTestPanel::ID_TEXTCTRL2 = wxNewId();
const long PlayListItemTestPanel::ID_STATICTEXT8 = wxNewId();
const long PlayListItemTestPanel::ID_TEXTCTRL3 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PlayListItemTestPanel,wxPanel)
	//(*EventTable(PlayListItemTestPanel)
	//*)
END_EVENT_TABLE()

void PlayListItemTestPanel::SetChoiceFromString(wxChoice* choice, std::string value)
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

PlayListItemTestPanel::PlayListItemTestPanel(wxWindow* parent, OutputManager* outputManager, PlayListItemTest* test, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _outputManager = outputManager;
    _test = test;

	//(*Initialize(PlayListItemTestPanel)
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer1;

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
	Choice_Mode->Append(_("Value 1"));
	Choice_Mode->SetSelection( Choice_Mode->Append(_("Value 2")) );
	Choice_Mode->Append(_("Alternate"));
	Choice_Mode->Append(_("A-B-C"));
	Choice_Mode->Append(_("A-B-C-All"));
	Choice_Mode->Append(_("None-A-B-C"));
	Choice_Mode->Append(_("Node-A-B-C-All"));
	FlexGridSizer1->Add(Choice_Mode, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Value 1:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Value1 = new wxSpinCtrl(this, ID_SPINCTRL1, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 0, 255, 0, _T("ID_SPINCTRL1"));
	SpinCtrl_Value1->SetValue(_T("0"));
	FlexGridSizer1->Add(SpinCtrl_Value1, 1, wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Value 2:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Value2 = new wxSpinCtrl(this, ID_SPINCTRL2, _T("255"), wxDefaultPosition, wxDefaultSize, 0, 0, 255, 255, _T("ID_SPINCTRL2"));
	SpinCtrl_Value2->SetValue(_T("255"));
	FlexGridSizer1->Add(SpinCtrl_Value2, 1, wxALL|wxEXPAND, 5);
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
	StaticText7 = new wxStaticText(this, ID_STATICTEXT7, _("Duration:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	FlexGridSizer1->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Duration = new wxTextCtrl(this, ID_TEXTCTRL2, _("60.000"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer1->Add(TextCtrl_Duration, 1, wxALL|wxEXPAND, 5);
	StaticText8 = new wxStaticText(this, ID_STATICTEXT8, _("Frame Duration:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
	FlexGridSizer1->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_FrameDuration = new wxTextCtrl(this, ID_TEXTCTRL3, _("0.500"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, wxDefaultValidator, _T("ID_TEXTCTRL3"));
	FlexGridSizer1->Add(TextCtrl_FrameDuration, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemTestPanel::OnTextCtrl_NameText);
	Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&PlayListItemTestPanel::OnChoice_ModeSelect);
	Connect(ID_TEXTCTRL4,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemTestPanel::OnTextCtrl_StartChannelText);
	//*)

    long channels = xScheduleFrame::GetScheduleManager()->GetTotalChannels();
    SpinCtrl_Channels->SetRange(1, channels);

    TextCtrl_Name->SetValue(test->GetRawName());
    SetChoiceFromString(Choice_Mode, test->GetMode());
    SpinCtrl_Value1->SetValue(test->GetValue1());
    SpinCtrl_Value2->SetValue(test->GetValue2());
    TextCtrl_StartChannel->SetValue(test->GetStartChannel());
    SpinCtrl_Channels->SetValue(test->GetChannels());
    TextCtrl_Duration->SetValue(wxString::Format(wxT("%.3f"), (float)test->GetDurationMS() / 1000));
    TextCtrl_FrameDuration->SetValue(wxString::Format(wxT("%.3f"), (float)test->GetFrameDuration() / 1000));

    ValidateWindow();
}

PlayListItemTestPanel::~PlayListItemTestPanel()
{
	//(*Destroy(PlayListItemTestPanel)
	//*)
    _test->SetName(TextCtrl_Name->GetValue().ToStdString());
    _test->SetMode(Choice_Mode->GetStringSelection().ToStdString());
    _test->SetValue1(SpinCtrl_Value1->GetValue());
    _test->SetValue2(SpinCtrl_Value2->GetValue());
    _test->SetStartChannel(TextCtrl_StartChannel->GetValue().ToStdString());
    _test->SetChannels(SpinCtrl_Channels->GetValue());
    _test->SetDuration(wxAtof(TextCtrl_Duration->GetValue()) * 1000);
    _test->SetFrameDuration(wxAtof(TextCtrl_FrameDuration->GetValue()) * 1000);
}

void PlayListItemTestPanel::OnTextCtrl_NameText(wxCommandEvent& event)
{
    _test->SetName(TextCtrl_Name->GetValue().ToStdString());
    ((PlayListDialog*)GetParent()->GetParent()->GetParent()->GetParent())->UpdateTree();
}

void PlayListItemTestPanel::OnChoice_ModeSelect(wxCommandEvent& event)
{
    ValidateWindow();
}

void PlayListItemTestPanel::ValidateWindow()
{
    std::string mode = Choice_Mode->GetStringSelection().ToStdString();

    if (mode == "Value 1")
    {
        SpinCtrl_Value1->Enable();
        SpinCtrl_Value2->Enable(false);
    }
    else if (mode == "Value 2")
    {
        SpinCtrl_Value1->Enable(false);
        SpinCtrl_Value2->Enable();
    }
    else if (mode == "Alternate")
    {
        SpinCtrl_Value1->Enable();
        SpinCtrl_Value2->Enable();
    }
    else
    {
        SpinCtrl_Value1->Enable(false);
        SpinCtrl_Value2->Enable(false);
    }
    StaticText_StartChannel->SetLabel(wxString::Format("%ld", _outputManager->DecodeStartChannel(TextCtrl_StartChannel->GetValue().ToStdString())));
}

void PlayListItemTestPanel::OnTextCtrl_StartChannelText(wxCommandEvent& event)
{
    ValidateWindow();
}
