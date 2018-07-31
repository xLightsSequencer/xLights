#include "FadeExcludeDialog.h"
#include "Settings.h"

//(*InternalHeaders(FadeExcludeDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(FadeExcludeDialog)
const long FadeExcludeDialog::ID_STATICTEXT1 = wxNewId();
const long FadeExcludeDialog::ID_CHOICE1 = wxNewId();
const long FadeExcludeDialog::ID_STATICTEXT2 = wxNewId();
const long FadeExcludeDialog::ID_SPINCTRL1 = wxNewId();
const long FadeExcludeDialog::ID_BUTTON1 = wxNewId();
const long FadeExcludeDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(FadeExcludeDialog,wxDialog)
	//(*EventTable(FadeExcludeDialog)
	//*)
END_EVENT_TABLE()

FadeExcludeDialog::FadeExcludeDialog(wxWindow* parent, Settings* settings, std::string ch, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _settings = settings;

	//(*Initialize(FadeExcludeDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Universe:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Universe = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	FlexGridSizer1->Add(Choice_Universe, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Channel:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Channel = new wxSpinCtrl(this, ID_SPINCTRL1, _T("1"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT, 1, 512, 1, _T("ID_SPINCTRL1"));
	SpinCtrl_Channel->SetValue(_T("1"));
	FlexGridSizer1->Add(SpinCtrl_Channel, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer2->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonCancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer2->Add(ButtonCancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FadeExcludeDialog::OnButton_OkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FadeExcludeDialog::OnButtonCancelClick);
	//*)

    for (auto it = _settings->_targetIP.begin(); it != _settings->_targetIP.end(); ++it)
    {
        Choice_Universe->AppendString(wxString::Format("%d", it->first));
    }

    if (Choice_Universe->GetCount() == 0)
    {
        Button_Ok->Disable();
    }
    else
    {
        Choice_Universe->SetSelection(0);
    }

    if (ch != "")
    {
        int u, sc;
        Settings::DecodeUSC(ch, u, sc);

        Choice_Universe->SetStringSelection(wxString::Format("%d", u));
        SpinCtrl_Channel->SetValue(sc);
    }
}

FadeExcludeDialog::~FadeExcludeDialog()
{
	//(*Destroy(FadeExcludeDialog)
	//*)
}

std::string FadeExcludeDialog::GetChannel() const
{
    return ("#" + Choice_Universe->GetStringSelection() + ":" + wxString::Format("%d", SpinCtrl_Channel->GetValue())).ToStdString();
}

void FadeExcludeDialog::OnButton_OkClick(wxCommandEvent& event)
{
    EndDialog(wxID_OK);
}

void FadeExcludeDialog::OnButtonCancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}