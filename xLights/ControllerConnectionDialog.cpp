#include "ControllerConnectionDialog.h"
#include "models/Model.h"

//(*InternalHeaders(ControllerConnectionDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(ControllerConnectionDialog)
const long ControllerConnectionDialog::ID_STATICTEXT1 = wxNewId();
const long ControllerConnectionDialog::ID_CHOICE1 = wxNewId();
const long ControllerConnectionDialog::ID_STATICTEXT2 = wxNewId();
const long ControllerConnectionDialog::ID_SPINCTRL1 = wxNewId();
const long ControllerConnectionDialog::ID_BUTTON1 = wxNewId();
const long ControllerConnectionDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(ControllerConnectionDialog,wxDialog)
	//(*EventTable(ControllerConnectionDialog)
	//*)
END_EVENT_TABLE()

ControllerConnectionDialog::ControllerConnectionDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(ControllerConnectionDialog)
	wxBoxSizer* BoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Protocol"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Protocol = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	FlexGridSizer1->Add(Choice_Protocol, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Port"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Port = new wxSpinCtrl(this, ID_SPINCTRL1, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 100, 1, _T("ID_SPINCTRL1"));
	SpinCtrl_Port->SetValue(_T("1"));
	FlexGridSizer1->Add(SpinCtrl_Port, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	BoxSizer1->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	BoxSizer1->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(BoxSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ControllerConnectionDialog::OnButton_OkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ControllerConnectionDialog::OnButton_CancelClick);
	//*)

    auto protocols = Model::GetProtocols();
    for (auto it = protocols.begin(); it != protocols.end(); ++it)
    {
        Choice_Protocol->AppendString(wxString(it->c_str()));
    }
    Choice_Protocol->SetSelection(0);

    SetEscapeId(Button_Cancel->GetId());
    Button_Ok->SetDefault();
}

ControllerConnectionDialog::~ControllerConnectionDialog()
{
	//(*Destroy(ControllerConnectionDialog)
	//*)
}


void ControllerConnectionDialog::OnButton_OkClick(wxCommandEvent& event)
{
    EndDialog(wxID_OK);
}

void ControllerConnectionDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void ControllerConnectionDialog::Set(const wxString &s) {

    Choice_Protocol->SetSelection(0);
    SpinCtrl_Port->SetValue(1);

    wxArrayString cc = wxSplit(s, ':');

    if (cc.size() > 0)
    {
        for (int i = 0; i < Choice_Protocol->GetCount(); i++)
        {
            if (cc[0].Lower() == Choice_Protocol->GetString(i).Lower())
            {
                Choice_Protocol->SetSelection(i);
                break;
            }
        }
    }

    if (cc.size() > 1)
    {
        int port = wxAtoi(cc[1]);
        SpinCtrl_Port->SetValue(port);
    }
}

std::string ControllerConnectionDialog::Get() {

    std::string res = Choice_Protocol->GetStringSelection().ToStdString() + ":" + wxString::Format("%d", SpinCtrl_Port->GetValue()).ToStdString();

    return res;
}
