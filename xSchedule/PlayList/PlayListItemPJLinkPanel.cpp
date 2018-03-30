#include "PlayListItemPJLinkPanel.h"
#include "PlayListItemPJLink.h"
#include "PlayListDialog.h"
#include "../Projector.h"

//(*InternalHeaders(PlayListItemPJLinkPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(PlayListItemPJLinkPanel)
const long PlayListItemPJLinkPanel::ID_STATICTEXT1 = wxNewId();
const long PlayListItemPJLinkPanel::ID_CHOICE2 = wxNewId();
const long PlayListItemPJLinkPanel::ID_STATICTEXT2 = wxNewId();
const long PlayListItemPJLinkPanel::ID_CHOICE3 = wxNewId();
const long PlayListItemPJLinkPanel::ID_STATICTEXT5 = wxNewId();
const long PlayListItemPJLinkPanel::ID_CHOICE1 = wxNewId();
const long PlayListItemPJLinkPanel::ID_STATICTEXT3 = wxNewId();
const long PlayListItemPJLinkPanel::ID_TEXTCTRL1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PlayListItemPJLinkPanel,wxPanel)
	//(*EventTable(PlayListItemPJLinkPanel)
	//*)
END_EVENT_TABLE()

PlayListItemPJLinkPanel::PlayListItemPJLinkPanel(wxWindow* parent, PlayListItemPJLink* PJLink, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _PJLink = PJLink;

	//(*Initialize(PlayListItemPJLinkPanel)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Projector:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Projector = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
	FlexGridSizer1->Add(Choice_Projector, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Command:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Command = new wxChoice(this, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE3"));
	Choice_Command->SetSelection( Choice_Command->Append(_("Power On")) );
	Choice_Command->Append(_("Power Off"));
	Choice_Command->Append(_("Change Input to RGB"));
	Choice_Command->Append(_("Change Input to VIDEO"));
	Choice_Command->Append(_("Change Input to NETWORK"));
	Choice_Command->Append(_("Change Input to DIGITAL"));
	Choice_Command->Append(_("Change Input to STORAGE"));
	Choice_Command->Append(_("Video Mute On"));
	Choice_Command->Append(_("Video Mute Off"));
	Choice_Command->Append(_("Audio Mute On"));
	Choice_Command->Append(_("Audio Mute Off"));
	FlexGridSizer1->Add(Choice_Command, 1, wxALL|wxEXPAND, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Parameter:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer1->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Parameter = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	Choice_Parameter->SetSelection( Choice_Parameter->Append(_("1")) );
	Choice_Parameter->Append(_("2"));
	Choice_Parameter->Append(_("3"));
	Choice_Parameter->Append(_("4"));
	Choice_Parameter->Append(_("5"));
	Choice_Parameter->Append(_("6"));
	Choice_Parameter->Append(_("7"));
	Choice_Parameter->Append(_("8"));
	Choice_Parameter->Append(_("9"));
	FlexGridSizer1->Add(Choice_Parameter, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Delay:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Delay = new wxTextCtrl(this, ID_TEXTCTRL1, _("0.000"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(TextCtrl_Delay, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_CHOICE2,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&PlayListItemPJLinkPanel::OnChoice_ProjectorSelect);
	Connect(ID_CHOICE3,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&PlayListItemPJLinkPanel::OnChoice_CommandSelect);
	//*)

    auto pjs = PJLink->GetProjectors();
    for (auto it = pjs.begin(); it != pjs.end(); ++it)
    {
        Choice_Projector->AppendString((*it)->GetName());
    }

    Choice_Command->SetStringSelection(PJLink->GetCommand());
    Choice_Parameter->SetStringSelection(PJLink->GetParameter());
    Choice_Projector->SetStringSelection(PJLink->GetProjector());

    ValidateWindow();
}

PlayListItemPJLinkPanel::~PlayListItemPJLinkPanel()
{
	//(*Destroy(PlayListItemPJLinkPanel)
	//*)
    _PJLink->SetProjector(Choice_Projector->GetStringSelection().ToStdString());
    _PJLink->SetCommand(Choice_Command->GetStringSelection().ToStdString());
    _PJLink->SetParameter(Choice_Parameter->GetStringSelection().ToStdString());
}

void PlayListItemPJLinkPanel::OnTextCtrl_DelayText(wxCommandEvent& event)
{
}

void PlayListItemPJLinkPanel::ValidateWindow()
{
    if (Choice_Command->GetStringSelection() == "Change Input")
    {
        Choice_Parameter->Enable(true);
    }
    else
    {
        Choice_Parameter->Enable(false);
    }
}

void PlayListItemPJLinkPanel::OnChoice_CommandSelect(wxCommandEvent& event)
{
    ValidateWindow();
}

void PlayListItemPJLinkPanel::OnChoice_ProjectorSelect(wxCommandEvent& event)
{
    ValidateWindow();
}
