/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "PlayListItemOSCPanel.h"
#include "PlayListDialog.h"
#include "PlayListItemOSC.h"

//(*InternalHeaders(PlayListItemOSCPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(PlayListItemOSCPanel)
const long PlayListItemOSCPanel::ID_STATICTEXT3 = wxNewId();
const long PlayListItemOSCPanel::ID_TEXTCTRL3 = wxNewId();
const long PlayListItemOSCPanel::ID_STATICTEXT1 = wxNewId();
const long PlayListItemOSCPanel::ID_TEXTCTRL1 = wxNewId();
const long PlayListItemOSCPanel::ID_STATICTEXT4 = wxNewId();
const long PlayListItemOSCPanel::ID_TEXTCTRL4 = wxNewId();
const long PlayListItemOSCPanel::ID_STATICTEXT5 = wxNewId();
const long PlayListItemOSCPanel::ID_SPINCTRL1 = wxNewId();
const long PlayListItemOSCPanel::ID_STATICTEXT6 = wxNewId();
const long PlayListItemOSCPanel::ID_CHOICE1 = wxNewId();
const long PlayListItemOSCPanel::ID_TEXTCTRL5 = wxNewId();
const long PlayListItemOSCPanel::ID_STATICTEXT7 = wxNewId();
const long PlayListItemOSCPanel::ID_CHOICE2 = wxNewId();
const long PlayListItemOSCPanel::ID_TEXTCTRL6 = wxNewId();
const long PlayListItemOSCPanel::ID_STATICTEXT8 = wxNewId();
const long PlayListItemOSCPanel::ID_CHOICE3 = wxNewId();
const long PlayListItemOSCPanel::ID_TEXTCTRL7 = wxNewId();
const long PlayListItemOSCPanel::ID_STATICTEXT9 = wxNewId();
const long PlayListItemOSCPanel::ID_CHOICE4 = wxNewId();
const long PlayListItemOSCPanel::ID_TEXTCTRL8 = wxNewId();
const long PlayListItemOSCPanel::ID_STATICTEXT10 = wxNewId();
const long PlayListItemOSCPanel::ID_CHOICE5 = wxNewId();
const long PlayListItemOSCPanel::ID_TEXTCTRL9 = wxNewId();
const long PlayListItemOSCPanel::ID_STATICTEXT2 = wxNewId();
const long PlayListItemOSCPanel::ID_TEXTCTRL2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PlayListItemOSCPanel,wxPanel)
	//(*EventTable(PlayListItemOSCPanel)
	//*)
END_EVENT_TABLE()

PlayListItemOSCPanel::PlayListItemOSCPanel(wxWindow* parent, PlayListItemOSC* osc, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _osc = osc;

	//(*Initialize(PlayListItemOSCPanel)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer6;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_OSCName = new wxTextCtrl(this, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL3"));
	FlexGridSizer1->Add(TextCtrl_OSCName, 1, wxALL|wxEXPAND, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Path:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Path = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(TextCtrl_Path, 1, wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("IP Address:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_IPAddress = new wxTextCtrl(this, ID_TEXTCTRL4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL4"));
	FlexGridSizer1->Add(TextCtrl_IPAddress, 1, wxALL|wxEXPAND, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Port:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer1->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Port = new wxSpinCtrl(this, ID_SPINCTRL1, _T("9000"), wxDefaultPosition, wxDefaultSize, 0, 1, 65535, 9000, _T("ID_SPINCTRL1"));
	SpinCtrl_Port->SetValue(_T("9000"));
	FlexGridSizer1->Add(SpinCtrl_Port, 1, wxALL|wxEXPAND, 5);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Parameter 1:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer1->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(1);
	Choice_P1Type = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	Choice_P1Type->SetSelection( Choice_P1Type->Append(_("None")) );
	Choice_P1Type->Append(_("Integer"));
	Choice_P1Type->Append(_("Float"));
	Choice_P1Type->Append(_("String"));
	FlexGridSizer2->Add(Choice_P1Type, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_P1Value = new wxTextCtrl(this, ID_TEXTCTRL5, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL5"));
	FlexGridSizer2->Add(TextCtrl_P1Value, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	StaticText7 = new wxStaticText(this, ID_STATICTEXT7, _("Parameter 2:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	FlexGridSizer1->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer3->AddGrowableCol(1);
	Choice_P2Type = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
	Choice_P2Type->SetSelection( Choice_P2Type->Append(_("None")) );
	Choice_P2Type->Append(_("Integer"));
	Choice_P2Type->Append(_("Float"));
	Choice_P2Type->Append(_("String"));
	FlexGridSizer3->Add(Choice_P2Type, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_P2Value = new wxTextCtrl(this, ID_TEXTCTRL6, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL6"));
	FlexGridSizer3->Add(TextCtrl_P2Value, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 5);
	StaticText8 = new wxStaticText(this, ID_STATICTEXT8, _("Parameter 3:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
	FlexGridSizer1->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer4->AddGrowableCol(1);
	Choice_P3Type = new wxChoice(this, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE3"));
	Choice_P3Type->SetSelection( Choice_P3Type->Append(_("None")) );
	Choice_P3Type->Append(_("Integer"));
	Choice_P3Type->Append(_("Float"));
	Choice_P3Type->Append(_("String"));
	FlexGridSizer4->Add(Choice_P3Type, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_P3Value = new wxTextCtrl(this, ID_TEXTCTRL7, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL7"));
	FlexGridSizer4->Add(TextCtrl_P3Value, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 5);
	StaticText9 = new wxStaticText(this, ID_STATICTEXT9, _("Parameter 4:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
	FlexGridSizer1->Add(StaticText9, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer5->AddGrowableCol(1);
	Choice_P4Type = new wxChoice(this, ID_CHOICE4, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE4"));
	Choice_P4Type->SetSelection( Choice_P4Type->Append(_("None")) );
	Choice_P4Type->Append(_("Integer"));
	Choice_P4Type->Append(_("Float"));
	Choice_P4Type->Append(_("String"));
	FlexGridSizer5->Add(Choice_P4Type, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_P4Value = new wxTextCtrl(this, ID_TEXTCTRL8, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL8"));
	FlexGridSizer5->Add(TextCtrl_P4Value, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer5, 1, wxALL|wxEXPAND, 5);
	StaticText10 = new wxStaticText(this, ID_STATICTEXT10, _("Parameter 5:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT10"));
	FlexGridSizer1->Add(StaticText10, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer6->AddGrowableCol(1);
	Choice_P5Type = new wxChoice(this, ID_CHOICE5, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE5"));
	Choice_P5Type->SetSelection( Choice_P5Type->Append(_("None")) );
	Choice_P5Type->Append(_("Integer"));
	Choice_P5Type->Append(_("Float"));
	Choice_P5Type->Append(_("String"));
	FlexGridSizer6->Add(Choice_P5Type, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_P5Value = new wxTextCtrl(this, ID_TEXTCTRL9, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL9"));
	FlexGridSizer6->Add(TextCtrl_P5Value, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer6, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Delay:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Delay = new wxTextCtrl(this, ID_TEXTCTRL2, _("0.000"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer1->Add(TextCtrl_Delay, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL3,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemOSCPanel::OnTextCtrl_RunProcessNameText);
	Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&PlayListItemOSCPanel::OnChoice_TypeSelect);
	Connect(ID_CHOICE2,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&PlayListItemOSCPanel::OnChoice_TypeSelect);
	Connect(ID_CHOICE3,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&PlayListItemOSCPanel::OnChoice_TypeSelect);
	Connect(ID_CHOICE4,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&PlayListItemOSCPanel::OnChoice_TypeSelect);
	Connect(ID_CHOICE5,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&PlayListItemOSCPanel::OnChoice_TypeSelect);
	Connect(ID_TEXTCTRL2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemOSCPanel::OnTextCtrl_DelayText);
	//*)

    TextCtrl_OSCName->SetValue(osc->GetRawName());
    TextCtrl_IPAddress->SetValue(osc->GetIPAddress());
    TextCtrl_Path->SetValue(osc->GetPath());
    TextCtrl_Path->SetToolTip(PlayListItemOSC::GetTooltip());
    SpinCtrl_Port->SetValue(osc->GetPort());
    Choice_P1Type->SetStringSelection(osc->GetTypeString(0));
    Choice_P2Type->SetStringSelection(osc->GetTypeString(1));
    Choice_P3Type->SetStringSelection(osc->GetTypeString(2));
    Choice_P4Type->SetStringSelection(osc->GetTypeString(3));
    Choice_P5Type->SetStringSelection(osc->GetTypeString(4));
    TextCtrl_P1Value->SetValue(osc->GetValue(0));
    TextCtrl_P2Value->SetValue(osc->GetValue(1));
    TextCtrl_P3Value->SetValue(osc->GetValue(2));
    TextCtrl_P4Value->SetValue(osc->GetValue(3));
    TextCtrl_P5Value->SetValue(osc->GetValue(4));
    TextCtrl_Delay->SetValue(wxString::Format(wxT("%.3f"), (float)osc->GetDelay() / 1000.0));

    ValidateWindow();
}

PlayListItemOSCPanel::~PlayListItemOSCPanel()
{
	//(*Destroy(PlayListItemOSCPanel)
	//*)
    _osc->SetName(TextCtrl_OSCName->GetValue().ToStdString());
    _osc->SetIPAddress(TextCtrl_IPAddress->GetValue().ToStdString());
    _osc->SetPath(TextCtrl_Path->GetValue().ToStdString());
    _osc->SetPort(SpinCtrl_Port->GetValue());
    _osc->SetOSCType(0, Choice_P1Type->GetStringSelection().ToStdString());
    _osc->SetOSCType(1, Choice_P2Type->GetStringSelection().ToStdString());
    _osc->SetOSCType(2, Choice_P3Type->GetStringSelection().ToStdString());
    _osc->SetOSCType(3, Choice_P4Type->GetStringSelection().ToStdString());
    _osc->SetOSCType(4, Choice_P5Type->GetStringSelection().ToStdString());
    _osc->SetValue(0, TextCtrl_P1Value->GetValue().ToStdString());
    _osc->SetValue(1, TextCtrl_P2Value->GetValue().ToStdString());
    _osc->SetValue(2, TextCtrl_P3Value->GetValue().ToStdString());
    _osc->SetValue(3, TextCtrl_P4Value->GetValue().ToStdString());
    _osc->SetValue(4, TextCtrl_P5Value->GetValue().ToStdString());
    _osc->SetDelay(wxAtof(TextCtrl_Delay->GetValue())*1000);
}

void PlayListItemOSCPanel::OnTextCtrl_DelayText(wxCommandEvent& event)
{
}

void PlayListItemOSCPanel::OnTextCtrl_RunProcessNameText(wxCommandEvent& event)
{
    _osc->SetName(TextCtrl_OSCName->GetValue().ToStdString());
    wxCommandEvent e(EVT_UPDATEITEMNAME);
    wxPostEvent(GetParent()->GetParent()->GetParent()->GetParent(), e);
}

void PlayListItemOSCPanel::OnChoice_TypeSelect(wxCommandEvent& event)
{
    ValidateWindow();
}

void PlayListItemOSCPanel::ValidateWindow()
{
    if (Choice_P1Type->GetStringSelection() == "None")
    {
        Choice_P2Type->Enable(false);
        Choice_P3Type->Enable(false);
        Choice_P4Type->Enable(false);
        Choice_P5Type->Enable(false);
        TextCtrl_P1Value->Enable(false);
        TextCtrl_P2Value->Enable(false);
        TextCtrl_P3Value->Enable(false);
        TextCtrl_P4Value->Enable(false);
        TextCtrl_P5Value->Enable(false);
    }
    else if (Choice_P2Type->GetStringSelection() == "None")
    {
        Choice_P2Type->Enable();
        TextCtrl_P1Value->Enable();

        Choice_P3Type->Enable(false);
        Choice_P4Type->Enable(false);
        Choice_P5Type->Enable(false);
        TextCtrl_P2Value->Enable(false);
        TextCtrl_P3Value->Enable(false);
        TextCtrl_P4Value->Enable(false);
        TextCtrl_P5Value->Enable(false);
    }
    else if (Choice_P3Type->GetStringSelection() == "None")
    {
        Choice_P2Type->Enable();
        Choice_P3Type->Enable();
        TextCtrl_P1Value->Enable();
        TextCtrl_P2Value->Enable();

        Choice_P4Type->Enable(false);
        Choice_P5Type->Enable(false);
        TextCtrl_P3Value->Enable(false);
        TextCtrl_P4Value->Enable(false);
        TextCtrl_P5Value->Enable(false);
    }
    else if (Choice_P4Type->GetStringSelection() == "None")
    {
        Choice_P2Type->Enable();
        Choice_P3Type->Enable();
        Choice_P4Type->Enable();
        TextCtrl_P1Value->Enable();
        TextCtrl_P2Value->Enable();
        TextCtrl_P3Value->Enable();

        Choice_P5Type->Enable(false);
        TextCtrl_P4Value->Enable(false);
        TextCtrl_P5Value->Enable(false);
    }
    else if (Choice_P5Type->GetStringSelection() == "None")
    {
        Choice_P2Type->Enable();
        Choice_P3Type->Enable();
        Choice_P4Type->Enable();
        Choice_P5Type->Enable();
        TextCtrl_P1Value->Enable();
        TextCtrl_P2Value->Enable();
        TextCtrl_P3Value->Enable();
        TextCtrl_P4Value->Enable();

        TextCtrl_P5Value->Enable(false);
    }
}
