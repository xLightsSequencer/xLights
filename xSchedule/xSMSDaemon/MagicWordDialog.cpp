/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "MagicWordDialog.h"
#include "MagicWord.h"

//(*InternalHeaders(MagicWordDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(MagicWordDialog)
const long MagicWordDialog::ID_STATICTEXT5 = wxNewId();
const long MagicWordDialog::ID_TEXTCTRL4 = wxNewId();
const long MagicWordDialog::ID_STATICTEXT1 = wxNewId();
const long MagicWordDialog::ID_CHOICE1 = wxNewId();
const long MagicWordDialog::ID_STATICTEXT4 = wxNewId();
const long MagicWordDialog::ID_TEXTCTRL1 = wxNewId();
const long MagicWordDialog::ID_STATICTEXT3 = wxNewId();
const long MagicWordDialog::ID_TEXTCTRL2 = wxNewId();
const long MagicWordDialog::ID_STATICTEXT2 = wxNewId();
const long MagicWordDialog::ID_TEXTCTRL3 = wxNewId();
const long MagicWordDialog::ID_BUTTON1 = wxNewId();
const long MagicWordDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(MagicWordDialog,wxDialog)
	//(*EventTable(MagicWordDialog)
	//*)
END_EVENT_TABLE()

MagicWordDialog::MagicWordDialog(wxWindow* parent, MagicWord* magicWord, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _magicWord = magicWord;

	//(*Initialize(MagicWordDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;

	Create(parent, wxID_ANY, _("Magic Word"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("wxID_ANY"));
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(1);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Magic Words:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer2->Add(StaticText5, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Name = new wxTextCtrl(this, ID_TEXTCTRL4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL4"));
	FlexGridSizer2->Add(TextCtrl_Name, 1, wxALL|wxEXPAND, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Command :"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxEXPAND, 5);
	Choice_Command = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	Choice_Command->SetMinSize(wxSize(300,-1));
	FlexGridSizer2->Add(Choice_Command, 1, wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Parameter 1 :"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer2->Add(StaticText4, 1, wxALL|wxEXPAND, 5);
	TextCtrl_P1 = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer2->Add(TextCtrl_P1, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Parameter 2 :"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer2->Add(StaticText3, 1, wxALL|wxEXPAND, 5);
	TextCtrl_P2 = new wxTextCtrl(this, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer2->Add(TextCtrl_P2, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Parameter 3 :"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer2->Add(StaticText2, 1, wxALL|wxEXPAND, 5);
	TextCtrl_P3 = new wxTextCtrl(this, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL3"));
	FlexGridSizer2->Add(TextCtrl_P3, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer2->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	Button_Ok->SetDefault();
	FlexGridSizer1->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer1->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2->Add(FlexGridSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer2);
	FlexGridSizer2->Fit(this);
	FlexGridSizer2->SetSizeHints(this);

	Connect(ID_TEXTCTRL4,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&MagicWordDialog::OnTextCtrl_NameText);
	Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&MagicWordDialog::OnChoice_CommandSelect);
	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&MagicWordDialog::OnTextCtrl_P1Text);
	Connect(ID_TEXTCTRL2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&MagicWordDialog::OnTextCtrl_P2Text);
	Connect(ID_TEXTCTRL3,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&MagicWordDialog::OnTextCtrl_P3Text);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MagicWordDialog::OnButton_OkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MagicWordDialog::OnButton_CancelClick);
	//*)

    auto commands = _commandManager.GetCommands();
    for (const auto& it : commands)
    {
        if (it->IsUserSelectable())
        {
            Choice_Command->AppendString(it->_command);
        }
    }

    if (_magicWord != nullptr)
    {
        Choice_Command->SetStringSelection(_magicWord->GetCommand());
        TextCtrl_P1->SetValue(_magicWord->GetP1());
        TextCtrl_P2->SetValue(_magicWord->GetP2());
        TextCtrl_P3->SetValue(_magicWord->GetP3());
        TextCtrl_Name->SetValue(_magicWord->GetMagicWord());
    }

    SetMinSize(wxSize(800, 600));
    SetSize(800, 600);

    ValidateWindow();
}

MagicWordDialog::~MagicWordDialog()
{
	//(*Destroy(MagicWordDialog)
	//*)
}

void MagicWordDialog::OnButton_OkClick(wxCommandEvent& event)
{
    if (_magicWord == nullptr) {
        _magicWord = new MagicWord();
    }
    _magicWord->SetCommand(Choice_Command->GetStringSelection().ToStdString());
    _magicWord->SetP1(TextCtrl_P1->GetValue().ToStdString());
    _magicWord->SetP2(TextCtrl_P2->GetValue().ToStdString());
    _magicWord->SetP3(TextCtrl_P3->GetValue().ToStdString());
    _magicWord->SetMagicWord(TextCtrl_Name->GetValue().ToStdString());

    EndDialog(wxID_OK);
}

void MagicWordDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void MagicWordDialog::OnChoice_CommandSelect(wxCommandEvent& event)
{
    Command* c = _commandManager.GetCommand(Choice_Command->GetStringSelection().ToStdString());
    Choice_Command->SetToolTip(c->GetParametersTip());
    ValidateWindow();
}

void MagicWordDialog::OnTextCtrl_P1Text(wxCommandEvent& event)
{
    ValidateWindow();
}

void MagicWordDialog::OnTextCtrl_P2Text(wxCommandEvent& event)
{
    ValidateWindow();
}

void MagicWordDialog::OnTextCtrl_P3Text(wxCommandEvent& event)
{
    ValidateWindow();
}

void MagicWordDialog::ValidateWindow()
{
    Command* c = _commandManager.GetCommand(Choice_Command->GetStringSelection().ToStdString());

    if (c != nullptr)
    {
        if (c->_parms == 0)
        {
            TextCtrl_P1->Enable(false);
            TextCtrl_P2->Enable(false);
            TextCtrl_P3->Enable(false);
        }
        else if (c->_parms == 1)
        {
            TextCtrl_P1->Enable(true);
            TextCtrl_P2->Enable(false);
            TextCtrl_P3->Enable(false);
        }
        else if (c->_parms == 2)
        {
            TextCtrl_P1->Enable(true);
            TextCtrl_P2->Enable(true);
            TextCtrl_P3->Enable(false);
        }
        else if (c->_parms >= 3)
        {
            TextCtrl_P1->Enable(true);
            TextCtrl_P2->Enable(true);
            TextCtrl_P3->Enable(true);
        }
    }

    if (!TextCtrl_P1->IsEnabled())
    {
        Disconnect(ID_TEXTCTRL1, wxEVT_COMMAND_TEXT_UPDATED, (wxObjectEventFunction)&MagicWordDialog::OnTextCtrl_P1Text);
        TextCtrl_P1->Clear();
        Connect(ID_TEXTCTRL1, wxEVT_COMMAND_TEXT_UPDATED, (wxObjectEventFunction)&MagicWordDialog::OnTextCtrl_P1Text);
    }
    if (!TextCtrl_P2->IsEnabled())
    {
        Disconnect(ID_TEXTCTRL2, wxEVT_COMMAND_TEXT_UPDATED, (wxObjectEventFunction)&MagicWordDialog::OnTextCtrl_P2Text);
        TextCtrl_P2->Clear();
        Connect(ID_TEXTCTRL2, wxEVT_COMMAND_TEXT_UPDATED, (wxObjectEventFunction)&MagicWordDialog::OnTextCtrl_P2Text);
    }
    if (!TextCtrl_P3->IsEnabled())
    {
        Disconnect(ID_TEXTCTRL3, wxEVT_COMMAND_TEXT_UPDATED, (wxObjectEventFunction)&MagicWordDialog::OnTextCtrl_P3Text);
        TextCtrl_P3->Clear();
        Connect(ID_TEXTCTRL3, wxEVT_COMMAND_TEXT_UPDATED, (wxObjectEventFunction)&MagicWordDialog::OnTextCtrl_P3Text);
    }

    if ((TextCtrl_P1->IsEnabled() && TextCtrl_P1->GetValue().Trim().Trim(false) == "") ||
        (TextCtrl_P2->IsEnabled() && TextCtrl_P2->GetValue().Trim().Trim(false) == "") ||
        (TextCtrl_P3->IsEnabled() && TextCtrl_P3->GetValue().Trim().Trim(false) == "") ||
        TextCtrl_Name->GetValue().Trim().Trim(false) == ""
    )
    {
        Button_Ok->Enable(false);
    }
    else
    {
        Button_Ok->Enable();
    }
}

void MagicWordDialog::OnTextCtrl_NameText(wxCommandEvent& event)
{
    ValidateWindow();
}
