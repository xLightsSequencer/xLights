/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "IPEntryDialog.h"
#include "utils/ip_utils.h"

//(*InternalHeaders(IPEntryDialog)
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/sckaddr.h>
#include <wx/socket.h>
//*)

//(*IdInit(IPEntryDialog)
const long IPEntryDialog::ID_STATICTEXT1 = wxNewId();
const long IPEntryDialog::ID_TEXTCTRL1 = wxNewId();
const long IPEntryDialog::ID_BUTTON1 = wxNewId();
const long IPEntryDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(IPEntryDialog,wxDialog)
	//(*EventTable(IPEntryDialog)
	//*)
END_EVENT_TABLE()

IPEntryDialog::IPEntryDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(IPEntryDialog)
	wxBoxSizer* BoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, _("Enter IP Address"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("IP Address:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_IPAddress = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	TextCtrl_IPAddress->SetMaxLength(15);
	FlexGridSizer1->Add(TextCtrl_IPAddress, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	Button_Ok->SetDefault();
	BoxSizer1->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	BoxSizer1->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(BoxSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&IPEntryDialog::OnTextCtrl_IPAddressText);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&IPEntryDialog::OnButton_OkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&IPEntryDialog::OnButton_CancelClick);
	//*)

    SetAffirmativeId(Button_Ok->GetId());
    SetEscapeId(Button_Cancel->GetId());
    ValidateWindow();
}

IPEntryDialog::~IPEntryDialog()
{
	//(*Destroy(IPEntryDialog)
	//*)
}


void IPEntryDialog::OnTextCtrl_IPAddressText(wxCommandEvent& event)
{
    ValidateWindow();
}

void IPEntryDialog::OnButton_OkClick(wxCommandEvent& event)
{
    EndDialog(wxID_OK);
}

void IPEntryDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void IPEntryDialog::ValidateWindow()
{
    if (TextCtrl_IPAddress->GetValue() == "")
    {
        Button_Ok->Enable();
    }
    else if (ip_utils::IsIPValidOrHostname(TextCtrl_IPAddress->GetValue().ToStdString()))
    {
        wxIPV4address localaddr;
        localaddr.Hostname(TextCtrl_IPAddress->GetValue());

        auto d = new wxDatagramSocket(localaddr, wxSOCKET_NOWAIT);

        if (d != nullptr && d->IsOk() && !d->Error())
        {
            Button_Ok->Enable();
        }
        else
        {
            Button_Ok->Disable();
        }

        if (d != nullptr)
        {
            delete d;
        }
    }
    else
    {
        Button_Ok->Disable();
    }
}
