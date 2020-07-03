/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "EmailDialog.h"
#include "UtilFunctions.h"

//(*InternalHeaders(EmailDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(EmailDialog)
const long EmailDialog::ID_STATICTEXT1 = wxNewId();
const long EmailDialog::ID_STATICTEXT2 = wxNewId();
const long EmailDialog::ID_TEXTCTRL1 = wxNewId();
const long EmailDialog::ID_BUTTON1 = wxNewId();
const long EmailDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(EmailDialog,wxDialog)
	//(*EventTable(EmailDialog)
	//*)
END_EVENT_TABLE()

EmailDialog::EmailDialog(wxWindow* parent, const std::string& email, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(EmailDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("From time to time xLights has been known to crash and sometimes we need more information from users when that happens to fix the problem.\nTo facilitate this, we would like you to set your email address here.\n\nOur policy regarding this is:\n\n1. Your email address will be stored on your machine ... not ours.\n2. Your email address will be written to the log file each time xLights starts.\n3. If you submit a crash report it will include the logs which will contain your email address.\n4. When xLights crashes and you click the OK button your email address will be included in the upload to our secure crash server.\n5. We won't use your email address for any other reason than contacting you about crashes and that won't be often.\n\nThank you for helping us improve xLights."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer3->AddGrowableCol(1);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Email Address:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer3->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Email = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer3->Add(TextCtrl_Email, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	Button_Ok->SetDefault();
	FlexGridSizer2->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_No = new wxButton(this, ID_BUTTON2, _("No Thanks"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer2->Add(Button_No, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&EmailDialog::OnTextCtrl_EmailText);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EmailDialog::OnButton_OkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EmailDialog::OnButton_NoClick);
	//*)

    SetEscapeId(ID_BUTTON2);
    TextCtrl_Email->SetValue(email);

	ValidateWindow();
}

EmailDialog::~EmailDialog()
{
	//(*Destroy(EmailDialog)
	//*)
}


void EmailDialog::OnButton_OkClick(wxCommandEvent& event)
{
    EndDialog(wxID_OK);
}

void EmailDialog::OnButton_NoClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void EmailDialog::OnTextCtrl_EmailText(wxCommandEvent& event)
{
    ValidateWindow();
}

void EmailDialog::ValidateWindow()
{
    if (IsEmailValid(TextCtrl_Email->GetValue().ToStdString()))
    {
        Button_Ok->Enable();
    }
    else
    {
        Button_Ok->Disable();
    }
}

std::string EmailDialog::GetEmail() const
{
    return TextCtrl_Email->GetValue().ToStdString();
}
