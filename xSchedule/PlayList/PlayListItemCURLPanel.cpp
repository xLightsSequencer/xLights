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
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Delay:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Delay = new wxTextCtrl(this, ID_TEXTCTRL2, _("0.000"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer1->Add(TextCtrl_Delay, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL3,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemCURLPanel::OnTextCtrl_CURLNameText);
	Connect(ID_TEXTCTRL2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemCURLPanel::OnTextCtrl_DelayText);
	//*)

    TextCtrl_CURLName->SetValue(curl->GetRawName());
    TextCtrl_URL->SetValue(curl->GetURL());
    TextCtrl_URL->SetToolTip(PlayListItemCURL::GetTooltip());
    TextCtrl_Body->SetValue(curl->GetBody());
    TextCtrl_Body->SetToolTip(PlayListItemCURL::GetTooltip());
    Choice_Type->SetStringSelection(curl->GetCURLType());
    TextCtrl_Delay->SetValue(wxString::Format(wxT("%.3f"), (float)curl->GetDelay() / 1000.0));
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
}

void PlayListItemCURLPanel::OnTextCtrl_DelayText(wxCommandEvent& event)
{
}

void PlayListItemCURLPanel::OnTextCtrl_CURLNameText(wxCommandEvent& event)
{
    _curl->SetName(TextCtrl_CURLName->GetValue().ToStdString());
    ((PlayListDialog*)GetParent()->GetParent()->GetParent()->GetParent())->UpdateTree();
}
