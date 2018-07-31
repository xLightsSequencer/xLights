#include "UniverseEntryDialog.h"
#include "../xLights/UtilFunctions.h"

//(*InternalHeaders(UniverseEntryDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(UniverseEntryDialog)
const long UniverseEntryDialog::ID_STATICTEXT1 = wxNewId();
const long UniverseEntryDialog::ID_SPINCTRL1 = wxNewId();
const long UniverseEntryDialog::ID_STATICTEXT2 = wxNewId();
const long UniverseEntryDialog::ID_SPINCTRL2 = wxNewId();
const long UniverseEntryDialog::ID_STATICTEXT3 = wxNewId();
const long UniverseEntryDialog::ID_TEXTCTRL1 = wxNewId();
const long UniverseEntryDialog::ID_STATICTEXT5 = wxNewId();
const long UniverseEntryDialog::ID_CHOICE1 = wxNewId();
const long UniverseEntryDialog::ID_STATICTEXT4 = wxNewId();
const long UniverseEntryDialog::ID_TEXTCTRL2 = wxNewId();
const long UniverseEntryDialog::ID_BUTTON1 = wxNewId();
const long UniverseEntryDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(UniverseEntryDialog,wxDialog)
	//(*EventTable(UniverseEntryDialog)
	//*)
END_EVENT_TABLE()

UniverseEntryDialog::UniverseEntryDialog(wxWindow* parent, int start, int end, std::string ipaddress, std::string desc, std::string protocol, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(UniverseEntryDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxCAPTION, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 4, 0, 0);
	FlexGridSizer2->AddGrowableCol(3);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Start:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Start = new wxSpinCtrl(this, ID_SPINCTRL1, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 64000, 1, _T("ID_SPINCTRL1"));
	SpinCtrl_Start->SetValue(_T("1"));
	FlexGridSizer2->Add(SpinCtrl_Start, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("End:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_End = new wxSpinCtrl(this, ID_SPINCTRL2, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 64000, 1, _T("ID_SPINCTRL2"));
	SpinCtrl_End->SetValue(_T("1"));
	FlexGridSizer2->Add(SpinCtrl_End, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Destination IP:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer2->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_IPAddress = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer2->Add(TextCtrl_IPAddress, 1, wxALL|wxEXPAND, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Protocol:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer2->Add(StaticText5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Protocol = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	Choice_Protocol->SetSelection( Choice_Protocol->Append(_("As per input")) );
	Choice_Protocol->Append(_("E1.31"));
	Choice_Protocol->Append(_("ArtNET"));
	FlexGridSizer2->Add(Choice_Protocol, 1, wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Description:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer2->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Description = new wxTextCtrl(this, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer2->Add(TextCtrl_Description, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer3->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer3->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_SPINCTRL1,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&UniverseEntryDialog::OnSpinCtrl_StartChange);
	Connect(ID_SPINCTRL2,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&UniverseEntryDialog::OnSpinCtrl_EndChange);
	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&UniverseEntryDialog::OnTextCtrl_IPAddressText);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&UniverseEntryDialog::OnButton_OkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&UniverseEntryDialog::OnButton_CancelClick);
	//*)

    if (start != -1)
    {
        SpinCtrl_Start->SetValue(start);
        SpinCtrl_End->SetValue(end);
        TextCtrl_IPAddress->SetValue(ipaddress);
        TextCtrl_Description->SetValue(desc);
        Choice_Protocol->SetStringSelection(protocol);
    }

    ValidateWindow();
}

UniverseEntryDialog::~UniverseEntryDialog()
{
	//(*Destroy(UniverseEntryDialog)
	//*)
}

void UniverseEntryDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void UniverseEntryDialog::OnButton_OkClick(wxCommandEvent& event)
{
    EndDialog(wxID_OK);
}

void UniverseEntryDialog::OnSpinCtrl_StartChange(wxSpinEvent& event)
{
    if (SpinCtrl_Start->GetValue() > SpinCtrl_End->GetValue())
    {
        SpinCtrl_End->SetValue(SpinCtrl_Start->GetValue());
    }
}

void UniverseEntryDialog::OnSpinCtrl_EndChange(wxSpinEvent& event)
{
    if (SpinCtrl_End->GetValue() < SpinCtrl_Start->GetValue())
    {
        SpinCtrl_Start->SetValue(SpinCtrl_End->GetValue());
    }
}

void UniverseEntryDialog::OnTextCtrl_IPAddressText(wxCommandEvent& event)
{
    ValidateWindow();
}

void UniverseEntryDialog::ValidateWindow()
{
    if (IsIPValid(TextCtrl_IPAddress->GetValue().ToStdString()))
    {
        Button_Ok->Enable();
    }
    else
    {
        Button_Ok->Disable();
    }
}