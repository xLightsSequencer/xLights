#include "ProjectorDetailsDialog.h"
#include "../xLights/outputs/IPOutput.h"

//(*InternalHeaders(ProjectorDetailsDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(ProjectorDetailsDialog)
const long ProjectorDetailsDialog::ID_STATICTEXT1 = wxNewId();
const long ProjectorDetailsDialog::ID_TEXTCTRL1 = wxNewId();
const long ProjectorDetailsDialog::ID_STATICTEXT2 = wxNewId();
const long ProjectorDetailsDialog::ID_TEXTCTRL2 = wxNewId();
const long ProjectorDetailsDialog::ID_STATICTEXT3 = wxNewId();
const long ProjectorDetailsDialog::ID_TEXTCTRL3 = wxNewId();
const long ProjectorDetailsDialog::ID_BUTTON1 = wxNewId();
const long ProjectorDetailsDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(ProjectorDetailsDialog,wxDialog)
	//(*EventTable(ProjectorDetailsDialog)
	//*)
END_EVENT_TABLE()

ProjectorDetailsDialog::ProjectorDetailsDialog(wxWindow* parent, std::string& projector, std::string& ip, std::string& password, wxWindowID id,const wxPoint& pos,const wxSize& size) : _projector(projector), _ip(ip), _password(password)
{
	//(*Initialize(ProjectorDetailsDialog)
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, _("Projector Details"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Name = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(TextCtrl_Name, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("IP Address:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_IPAddress = new wxTextCtrl(this, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer1->Add(TextCtrl_IPAddress, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Password:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Password = new wxTextCtrl(this, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL3"));
	FlexGridSizer1->Add(TextCtrl_Password, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	Button_Ok->SetDefault();
	FlexGridSizer2->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer2->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ProjectorDetailsDialog::OnTextCtrl_NameText);
	Connect(ID_TEXTCTRL2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ProjectorDetailsDialog::OnTextCtrl_IPAddressText);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ProjectorDetailsDialog::OnButton_OkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ProjectorDetailsDialog::OnButton_CancelClick);
	//*)

    TextCtrl_Name->SetValue(_projector);
    TextCtrl_IPAddress->SetValue(_ip);
    TextCtrl_Password->SetValue(_password);

    ValidateWindow();
}

ProjectorDetailsDialog::~ProjectorDetailsDialog()
{
	//(*Destroy(ProjectorDetailsDialog)
	//*)
}


void ProjectorDetailsDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void ProjectorDetailsDialog::OnButton_OkClick(wxCommandEvent& event)
{
    _projector = TextCtrl_Name->GetValue().ToStdString();
    _ip = TextCtrl_IPAddress->GetValue().ToStdString();
    _password = TextCtrl_Password->GetValue().ToStdString();
    EndDialog(wxID_OK);
}

void ProjectorDetailsDialog::OnTextCtrl_IPAddressText(wxCommandEvent& event)
{
    ValidateWindow();
}

void ProjectorDetailsDialog::OnTextCtrl_NameText(wxCommandEvent& event)
{
    ValidateWindow();
}

void ProjectorDetailsDialog::ValidateWindow()
{
    if (TextCtrl_Name->GetValue() == "" &&
        IPOutput::IsIPValidOrHostname(TextCtrl_IPAddress->GetValue().ToStdString()))
    {
        Button_Ok->Enable(false);
    }
    else
    {
        Button_Ok->Enable(true);
    }
}
