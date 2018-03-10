#include "ZCPPDialog.h"

//(*InternalHeaders(ZCPPDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "ZCPPOutput.h"
#include "OutputManager.h"
#include "../UtilFunctions.h"

//(*IdInit(ZCPPDialog)
const long ZCPPDialog::ID_STATICTEXT4 = wxNewId();
const long ZCPPDialog::ID_STATICTEXT2 = wxNewId();
const long ZCPPDialog::ID_SPINCTRL1 = wxNewId();
const long ZCPPDialog::ID_STATICTEXT1 = wxNewId();
const long ZCPPDialog::ID_TEXTCTRL_IP_ADDR = wxNewId();
const long ZCPPDialog::ID_STATICTEXT3 = wxNewId();
const long ZCPPDialog::ID_SPINCTRL2 = wxNewId();
const long ZCPPDialog::ID_STATICTEXT8 = wxNewId();
const long ZCPPDialog::ID_TEXTCTRL_DESCRIPTION = wxNewId();
const long ZCPPDialog::ID_STATICTEXT9 = wxNewId();
const long ZCPPDialog::ID_CHECKBOX2 = wxNewId();
const long ZCPPDialog::ID_BUTTON1 = wxNewId();
const long ZCPPDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(ZCPPDialog,wxDialog)
    //(*EventTable(ZCPPDialog)
    //*)
END_EVENT_TABLE()

ZCPPDialog::ZCPPDialog(wxWindow* parent, ZCPPOutput* zcpp, OutputManager* outputManager, wxWindowID id, const wxPoint& pos, const wxSize& size)
{
    _zcpp = zcpp;
    _outputManager = outputManager;

    //(*Initialize(ZCPPDialog)
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer3;

    Create(parent, wxID_ANY, _("E1.31 Setup"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
    SetClientSize(wxDefaultSize);
    Move(wxDefaultPosition);
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Sets up a Zero Configuration Pixel controller over ethernet.\n\nSupported devices include those made\nby Falcon."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Id"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    SpinCtrl_Id = new wxSpinCtrl(this, ID_SPINCTRL1, _T("64001"), wxDefaultPosition, wxDefaultSize, 0, 1, 65535, 64001, _T("ID_SPINCTRL1"));
    SpinCtrl_Id->SetValue(_T("64001"));
    FlexGridSizer2->Add(SpinCtrl_Id, 1, wxALL|wxEXPAND, 5);
    StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("IP Address"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrlIpAddr = new wxTextCtrl(this, ID_TEXTCTRL_IP_ADDR, _("192.168.1.50"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_IP_ADDR"));
    FlexGridSizer2->Add(TextCtrlIpAddr, 1, wxALL|wxEXPAND, 5);
    StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("# of Channels"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer2->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    SpinCtrl_Channels = new wxSpinCtrl(this, ID_SPINCTRL2, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 16384, 1, _T("ID_SPINCTRL2"));
    SpinCtrl_Channels->SetValue(_T("1"));
    FlexGridSizer2->Add(SpinCtrl_Channels, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText8 = new wxStaticText(this, ID_STATICTEXT8, _("Description"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
    FlexGridSizer2->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_Description = new wxTextCtrl(this, ID_TEXTCTRL_DESCRIPTION, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_DESCRIPTION"));
    TextCtrl_Description->SetMaxLength(64);
    FlexGridSizer2->Add(TextCtrl_Description, 1, wxALL|wxEXPAND, 5);
    StaticText9 = new wxStaticText(this, ID_STATICTEXT9, _("Suppress duplicate frames"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
    FlexGridSizer2->Add(StaticText9, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_SuppressDuplicates = new wxCheckBox(this, ID_CHECKBOX2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
    CheckBox_SuppressDuplicates->SetValue(false);
    FlexGridSizer2->Add(CheckBox_SuppressDuplicates, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer3->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    FlexGridSizer3->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    Connect(ID_TEXTCTRL_IP_ADDR,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ZCPPDialog::OnTextCtrlIpAddrText);
    Connect(ID_TEXTCTRL_DESCRIPTION,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ZCPPDialog::OnTextCtrl_DescriptionText);
    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ZCPPDialog::OnButton_OkClick);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ZCPPDialog::OnButton_CancelClick);
    //*)

    CheckBox_SuppressDuplicates->SetValue(_zcpp->IsSuppressDuplicateFrames());
    SpinCtrl_Channels->SetValue(_zcpp->GetChannels());
    TextCtrl_Description->SetValue(_zcpp->GetDescription());
    TextCtrlIpAddr->SetValue(_zcpp->GetIP());
    SpinCtrl_Id->SetValue(_zcpp->GetId());

    Button_Ok->SetDefault();
    ValidateWindow();
}

ZCPPDialog::~ZCPPDialog()
{
    //(*Destroy(ZCPPDialog)
    //*)
}

void ZCPPDialog::OnTextCtrl_DescriptionText(wxCommandEvent& event)
{
}

void ZCPPDialog::OnTextCtrlIpAddrText(wxCommandEvent& event)
{
    ValidateWindow();
}

void ZCPPDialog::OnButton_OkClick(wxCommandEvent& event)
{
    _zcpp->SetIP(TextCtrlIpAddr->GetValue().ToStdString());
    _zcpp->SetChannels(SpinCtrl_Channels->GetValue());
    _zcpp->SetDescription(TextCtrl_Description->GetValue().ToStdString());
    _zcpp->SetSuppressDuplicateFrames(CheckBox_SuppressDuplicates->IsChecked());
    _zcpp->SetId(SpinCtrl_Id->GetValue());

    EndDialog(wxID_OK);
}

void ZCPPDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void ZCPPDialog::ValidateWindow()
{
    if (TextCtrlIpAddr->GetValue().IsEmpty() ||
        !IsIPValidOrHostname(TextCtrlIpAddr->GetValue().ToStdString(), true))
    {
        Button_Ok->Enable(false);
    }
    else
    {
        Button_Ok->Enable();
    }
}

void ZCPPDialog::OnSpinCtrl_ChannelsChange(wxSpinEvent& event)
{
}
