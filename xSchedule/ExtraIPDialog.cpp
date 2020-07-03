/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "ExtraIPDialog.h"

//(*InternalHeaders(ExtraIPDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "../xLights/UtilFunctions.h"

//(*IdInit(ExtraIPDialog)
const long ExtraIPDialog::ID_STATICTEXT1 = wxNewId();
const long ExtraIPDialog::ID_TEXTCTRL1 = wxNewId();
const long ExtraIPDialog::ID_STATICTEXT2 = wxNewId();
const long ExtraIPDialog::ID_TEXTCTRL2 = wxNewId();
const long ExtraIPDialog::ID_BUTTON1 = wxNewId();
const long ExtraIPDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(ExtraIPDialog,wxDialog)
	//(*EventTable(ExtraIPDialog)
	//*)
END_EVENT_TABLE()

ExtraIPDialog::ExtraIPDialog(wxWindow* parent, std::list<ExtraIP*>* extraIPs, ExtraIP* sel,wxWindowID id,const wxPoint& pos,const wxSize& size) : _extraIPs(extraIPs), _sel(sel)
{
	//(*Initialize(ExtraIPDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;

	Create(parent, id, _("IP Address to Monitor"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("IP"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Ip = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(TextCtrl_Ip, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Description"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Description = new wxTextCtrl(this, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer1->Add(TextCtrl_Description, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer2->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer2->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ExtraIPDialog::OnTextCtrl_IpText);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ExtraIPDialog::OnButton_OkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ExtraIPDialog::OnButton_CancelClick);
	//*)

    SetEscapeId(ID_BUTTON2);

    if (sel != nullptr)
    {
        TextCtrl_Ip->SetValue(sel->GetIP());
        TextCtrl_Description->SetValue(sel->GetDescription());
    }
    ValidateWindow();
}

ExtraIPDialog::~ExtraIPDialog()
{
	//(*Destroy(ExtraIPDialog)
	//*)
}


void ExtraIPDialog::OnButton_OkClick(wxCommandEvent& event)
{
    EndDialog(wxID_OK);
}

void ExtraIPDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void ExtraIPDialog::OnTextCtrl_IpText(wxCommandEvent& event)
{
    ValidateWindow();
}

void ExtraIPDialog::ValidateWindow()
{
    std::string ip = TextCtrl_Ip->GetValue().ToStdString();

    if (IsIPValid(ip))
    {
        bool clash = false;
        for (auto it = _extraIPs->begin(); it != _extraIPs->end(); ++it)
        {
            if ((*it)->GetIP() == ip)
            {
                if (_sel == *it)
                {
                    // this is ok
                }
                else
                {
                    clash = true;
                    break;
                }
            }
        }

        if (clash)
        {
            Button_Ok->Disable();
        }
        else
        {
            Button_Ok->Enable();
        }
    }
    else
    {
        Button_Ok->Disable();
    }
}