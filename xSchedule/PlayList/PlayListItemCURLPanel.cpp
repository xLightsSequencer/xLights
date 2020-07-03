/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "PlayListItemCURLPanel.h"
#include "PlayListDialog.h"
#include "PlayListItemCURL.h"

//(*InternalHeaders(PlayListItemCURLPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(PlayListItemCURLPanel)
const long PlayListItemCURLPanel::ID_STATICTEXT3 = wxNewId();
const long PlayListItemCURLPanel::ID_TEXTCTRL3 = wxNewId();
const long PlayListItemCURLPanel::ID_STATICTEXT1 = wxNewId();
const long PlayListItemCURLPanel::ID_TEXTCTRL1 = wxNewId();
const long PlayListItemCURLPanel::ID_STATICTEXT4 = wxNewId();
const long PlayListItemCURLPanel::ID_CHOICE1 = wxNewId();
const long PlayListItemCURLPanel::ID_STATICTEXT5 = wxNewId();
const long PlayListItemCURLPanel::ID_TEXTCTRL4 = wxNewId();
const long PlayListItemCURLPanel::ID_STATICTEXT6 = wxNewId();
const long PlayListItemCURLPanel::ID_CHOICE2 = wxNewId();
const long PlayListItemCURLPanel::ID_STATICTEXT2 = wxNewId();
const long PlayListItemCURLPanel::ID_TEXTCTRL2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PlayListItemCURLPanel,wxPanel)
	//(*EventTable(PlayListItemCURLPanel)
	//*)
END_EVENT_TABLE()

PlayListItemCURLPanel::PlayListItemCURLPanel(wxWindow* parent, PlayListItemCURL* curl, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _curl = curl;

	//(*Initialize(PlayListItemCURLPanel)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	FlexGridSizer1->AddGrowableRow(3);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_CURLName = new wxTextCtrl(this, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL3"));
	FlexGridSizer1->Add(TextCtrl_CURLName, 1, wxALL|wxEXPAND, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("URL:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_URL = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(TextCtrl_URL, 1, wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Post:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Type = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	Choice_Type->SetSelection( Choice_Type->Append(_("GET")) );
	Choice_Type->Append(_("POST"));
	FlexGridSizer1->Add(Choice_Type, 1, wxALL|wxEXPAND, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Body:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer1->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Body = new wxTextCtrl(this, ID_TEXTCTRL4, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE, wxDefaultValidator, _T("ID_TEXTCTRL4"));
	FlexGridSizer1->Add(TextCtrl_Body, 1, wxALL|wxEXPAND, 5);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Content Type:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer1->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_ContentType = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
	FlexGridSizer1->Add(Choice_ContentType, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Delay:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Delay = new wxTextCtrl(this, ID_TEXTCTRL2, _("0.000"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer1->Add(TextCtrl_Delay, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL3,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemCURLPanel::OnTextCtrl_CURLNameText);
	Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&PlayListItemCURLPanel::OnChoice_TypeSelect);
	Connect(ID_TEXTCTRL2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemCURLPanel::OnTextCtrl_DelayText);
	//*)

    Choice_ContentType->AppendString("TEXT");
    Choice_ContentType->AppendString("JSON");
    Choice_ContentType->AppendString("XML");
    Choice_ContentType->AppendString("TEXT XML");
    Choice_ContentType->AppendString("HTML");
    Choice_ContentType->AppendString("FORM");

    TextCtrl_CURLName->SetValue(curl->GetRawName());
    TextCtrl_URL->SetValue(curl->GetURL());
    TextCtrl_URL->SetToolTip(PlayListItemCURL::GetTooltip());
    TextCtrl_Body->SetValue(curl->GetBody());
    TextCtrl_Body->SetToolTip(PlayListItemCURL::GetTooltip());
    Choice_Type->SetStringSelection(curl->GetCURLType());
    TextCtrl_Delay->SetValue(wxString::Format(wxT("%.3f"), (float)curl->GetDelay() / 1000.0));
    if (curl->GetContentType() == "")
    {
        Choice_ContentType->SetSelection(0);
    }
    else
    {
        Choice_ContentType->SetStringSelection(curl->GetContentType());
    }

    ValidateWindow();
}

PlayListItemCURLPanel::~PlayListItemCURLPanel()
{
	//(*Destroy(PlayListItemCURLPanel)
	//*)
    _curl->SetName(TextCtrl_CURLName->GetValue().ToStdString());
    _curl->SetCURLType(Choice_Type->GetStringSelection().ToStdString());
    _curl->SetURL(TextCtrl_URL->GetValue().ToStdString());
    _curl->SetBody(TextCtrl_Body->GetValue().ToStdString());
    _curl->SetDelay(wxAtof(TextCtrl_Delay->GetValue())*1000);
    _curl->SetContentType(Choice_ContentType->GetStringSelection().ToStdString());
}

void PlayListItemCURLPanel::ValidateWindow()
{
    if (Choice_Type->GetStringSelection() == "GET")
    {
        TextCtrl_Body->Enable(false);
        Choice_ContentType->Enable(false);
    }
    else
    {
        TextCtrl_Body->Enable();
        Choice_ContentType->Enable();
    }
}

void PlayListItemCURLPanel::OnTextCtrl_DelayText(wxCommandEvent& event)
{
}

void PlayListItemCURLPanel::OnTextCtrl_CURLNameText(wxCommandEvent& event)
{
    _curl->SetName(TextCtrl_CURLName->GetValue().ToStdString());
    wxCommandEvent e(EVT_UPDATEITEMNAME);
    wxPostEvent(GetParent()->GetParent()->GetParent()->GetParent(), e);
}

void PlayListItemCURLPanel::OnChoice_TypeSelect(wxCommandEvent& event)
{
    ValidateWindow();
}
