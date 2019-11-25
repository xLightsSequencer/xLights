#include "PlayListItemFilePanel.h"
#include "PlayListDialog.h"
#include "PlayListItemFile.h"

//(*InternalHeaders(PlayListItemFilePanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(PlayListItemFilePanel)
const long PlayListItemFilePanel::ID_STATICTEXT3 = wxNewId();
const long PlayListItemFilePanel::ID_TEXTCTRL3 = wxNewId();
const long PlayListItemFilePanel::ID_STATICTEXT4 = wxNewId();
const long PlayListItemFilePanel::ID_TEXTCTRL4 = wxNewId();
const long PlayListItemFilePanel::ID_STATICTEXT1 = wxNewId();
const long PlayListItemFilePanel::ID_TEXTCTRL1 = wxNewId();
const long PlayListItemFilePanel::ID_STATICTEXT2 = wxNewId();
const long PlayListItemFilePanel::ID_TEXTCTRL2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PlayListItemFilePanel,wxPanel)
	//(*EventTable(PlayListItemFilePanel)
	//*)
END_EVENT_TABLE()

PlayListItemFilePanel::PlayListItemFilePanel(wxWindow* parent, PlayListItemFile* file, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _file = file;

	//(*Initialize(PlayListItemFilePanel)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	FlexGridSizer1->AddGrowableRow(2);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_FileName = new wxTextCtrl(this, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL3"));
	FlexGridSizer1->Add(TextCtrl_FileName, 1, wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("File Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_FName = new wxTextCtrl(this, ID_TEXTCTRL4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL4"));
	FlexGridSizer1->Add(TextCtrl_FName, 1, wxALL|wxEXPAND, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Content:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Content = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(TextCtrl_Content, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Delay:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Delay = new wxTextCtrl(this, ID_TEXTCTRL2, _("0.000"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer1->Add(TextCtrl_Delay, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL3,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemFilePanel::OnTextCtrl_FileNameText);
	Connect(ID_TEXTCTRL4,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemFilePanel::OnTextCtrl_FNameText);
	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemFilePanel::OnTextCtrl_ContentText);
	Connect(ID_TEXTCTRL2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemFilePanel::OnTextCtrl_DelayText);
	//*)

    TextCtrl_FileName->SetValue(file->GetRawName());
    TextCtrl_Content->SetValue(file->GetContent());
    TextCtrl_FName->SetValue(file->GetFileName());
    TextCtrl_Content->SetToolTip(PlayListItemFile::GetTooltip());
    TextCtrl_FName->SetToolTip(PlayListItemFile::GetTooltip());
    TextCtrl_Delay->SetValue(wxString::Format(wxT("%.3f"), (float)file->GetDelay() / 1000.0));
}

PlayListItemFilePanel::~PlayListItemFilePanel()
{
	//(*Destroy(PlayListItemFilePanel)
	//*)
    _file->SetName(TextCtrl_FileName->GetValue().ToStdString());
    _file->SetFileName(TextCtrl_FName->GetValue().ToStdString());
    _file->SetContent(TextCtrl_Content->GetValue().ToStdString());
    _file->SetDelay(wxAtof(TextCtrl_Delay->GetValue())*1000);
}


void PlayListItemFilePanel::OnTextCtrl_DelayText(wxCommandEvent& event)
{
}

void PlayListItemFilePanel::OnTextCtrl_ContentText(wxCommandEvent& event)
{
}

void PlayListItemFilePanel::OnTextCtrl_FileNameText(wxCommandEvent& event)
{
    _file->SetName(TextCtrl_FileName->GetValue().ToStdString());
    wxCommandEvent e(EVT_UPDATEITEMNAME);
    wxPostEvent(GetParent()->GetParent()->GetParent()->GetParent(), e);
}

void PlayListItemFilePanel::OnTextCtrl_FNameText(wxCommandEvent& event)
{
}
