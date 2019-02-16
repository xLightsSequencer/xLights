#include "TestMessagesDialog.h"

//(*InternalHeaders(TestMessagesDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(TestMessagesDialog)
const long TestMessagesDialog::ID_STATICTEXT1 = wxNewId();
const long TestMessagesDialog::ID_TEXTCTRL1 = wxNewId();
const long TestMessagesDialog::ID_BUTTON1 = wxNewId();
const long TestMessagesDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(TestMessagesDialog,wxDialog)
	//(*EventTable(TestMessagesDialog)
	//*)
END_EVENT_TABLE()

TestMessagesDialog::TestMessagesDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(TestMessagesDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Insert one message per line:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Messages = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	TextCtrl_Messages->SetMinSize(wxSize(250,100));
	FlexGridSizer1->Add(TextCtrl_Messages, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer2->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer2->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&TestMessagesDialog::OnTextCtrl_MessagesText);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TestMessagesDialog::OnButton_OkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TestMessagesDialog::OnButton_CancelClick);
	//*)

    SetEscapeId(ID_BUTTON2);

    ValidateWindow();
}

TestMessagesDialog::~TestMessagesDialog()
{
	//(*Destroy(TestMessagesDialog)
	//*)
}


void TestMessagesDialog::OnButton_OkClick(wxCommandEvent& event)
{
    EndDialog(wxID_OK);
}

void TestMessagesDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void TestMessagesDialog::OnTextCtrl_MessagesText(wxCommandEvent& event)
{
    ValidateWindow();
}

void TestMessagesDialog::ValidateWindow()
{
    if (TextCtrl_Messages->GetValue() == "")
    {
        Button_Ok->Disable();
    }
    else
    {
        Button_Ok->Enable();
    }
}