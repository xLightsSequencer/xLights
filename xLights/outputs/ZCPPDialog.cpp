#include "ZCPPDialog.h"

//(*InternalHeaders(ZCPPDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "ZCPPOutput.h"
#include "OutputManager.h"
#include "../UtilFunctions.h"
#include "../ControllerVisualiseDialog.h"
#include "../controllers/ControllerUploadData.h"

//(*IdInit(ZCPPDialog)
const long ZCPPDialog::ID_STATICTEXT4 = wxNewId();
const long ZCPPDialog::ID_STATICTEXT1 = wxNewId();
const long ZCPPDialog::ID_TEXTCTRL_IP_ADDR = wxNewId();
const long ZCPPDialog::ID_STATICTEXT3 = wxNewId();
const long ZCPPDialog::ID_SPINCTRL2 = wxNewId();
const long ZCPPDialog::ID_STATICTEXT8 = wxNewId();
const long ZCPPDialog::ID_TEXTCTRL_DESCRIPTION = wxNewId();
const long ZCPPDialog::ID_STATICTEXT9 = wxNewId();
const long ZCPPDialog::ID_CHECKBOX2 = wxNewId();
const long ZCPPDialog::ID_STATICTEXT2 = wxNewId();
const long ZCPPDialog::ID_CHECKBOX1 = wxNewId();
const long ZCPPDialog::ID_STATICTEXT5 = wxNewId();
const long ZCPPDialog::ID_CHECKBOX3 = wxNewId();
const long ZCPPDialog::ID_STATICTEXT6 = wxNewId();
const long ZCPPDialog::ID_CHECKBOX4 = wxNewId();
const long ZCPPDialog::ID_STATICTEXT7 = wxNewId();
const long ZCPPDialog::ID_CHECKBOX5 = wxNewId();
const long ZCPPDialog::ID_STATICTEXT10 = wxNewId();
const long ZCPPDialog::ID_CHECKBOX6 = wxNewId();
const long ZCPPDialog::ID_STATICTEXT11 = wxNewId();
const long ZCPPDialog::ID_SPINCTRL1 = wxNewId();
const long ZCPPDialog::ID_BUTTON3 = wxNewId();
const long ZCPPDialog::ID_BUTTON1 = wxNewId();
const long ZCPPDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(ZCPPDialog,wxDialog)
    //(*EventTable(ZCPPDialog)
    //*)
END_EVENT_TABLE()

ZCPPDialog::ZCPPDialog(wxWindow* parent, ZCPPOutput* zcpp, OutputManager* outputManager, ModelManager* modelManager, wxWindowID id, const wxPoint& pos, const wxSize& size)
{
    _zcpp = zcpp;
    _outputManager = outputManager;
    _modelManager = modelManager;

    //(*Initialize(ZCPPDialog)
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer3;

    Create(parent, wxID_ANY, _("ZCPP Setup"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
    SetClientSize(wxDefaultSize);
    Move(wxDefaultPosition);
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Sets up a Zero Configuration Pixel controller over ethernet.\n\nSupported devices include those made\nby Falcon."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    FlexGridSizer1->Add(StaticText4, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer2->AddGrowableCol(1);
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
    FlexGridSizer2->Add(StaticText9, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_SuppressDuplicates = new wxCheckBox(this, ID_CHECKBOX2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
    CheckBox_SuppressDuplicates->SetValue(false);
    FlexGridSizer2->Add(CheckBox_SuppressDuplicates, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Autosize output"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    CheckBoxAutoSizeOutput = new wxCheckBox(this, ID_CHECKBOX1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
    CheckBoxAutoSizeOutput->SetValue(false);
    FlexGridSizer2->Add(CheckBoxAutoSizeOutput, 1, wxALL|wxEXPAND, 5);
    StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Support Virtual Strings"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
    FlexGridSizer2->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_SupportVirtualStrings = new wxCheckBox(this, ID_CHECKBOX3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX3"));
    CheckBox_SupportVirtualStrings->SetValue(false);
    FlexGridSizer2->Add(CheckBox_SupportVirtualStrings, 1, wxALL|wxEXPAND, 5);
    StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Support Smart Remotes"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
    FlexGridSizer2->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_SupportSmartRemotes = new wxCheckBox(this, ID_CHECKBOX4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX4"));
    CheckBox_SupportSmartRemotes->SetValue(false);
    FlexGridSizer2->Add(CheckBox_SupportSmartRemotes, 1, wxALL|wxEXPAND, 5);
    StaticText7 = new wxStaticText(this, ID_STATICTEXT7, _("Multicast "), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
    FlexGridSizer2->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_Multicast = new wxCheckBox(this, ID_CHECKBOX5, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX5"));
    CheckBox_Multicast->SetValue(false);
    FlexGridSizer2->Add(CheckBox_Multicast, 1, wxALL|wxEXPAND, 5);
    StaticText10 = new wxStaticText(this, ID_STATICTEXT10, _("Suppress sending confguration"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT10"));
    FlexGridSizer2->Add(StaticText10, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_SuppressSendingConfig = new wxCheckBox(this, ID_CHECKBOX6, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX6"));
    CheckBox_SuppressSendingConfig->SetValue(false);
    FlexGridSizer2->Add(CheckBox_SuppressSendingConfig, 1, wxALL|wxEXPAND, 5);
    StaticText11 = new wxStaticText(this, ID_STATICTEXT11, _("Priority"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT11"));
    FlexGridSizer2->Add(StaticText11, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    SpinCtrl_Priority = new wxSpinCtrl(this, ID_SPINCTRL1, _T("100"), wxDefaultPosition, wxDefaultSize, 0, 0, 255, 100, _T("ID_SPINCTRL1"));
    SpinCtrl_Priority->SetValue(_T("100"));
    FlexGridSizer2->Add(SpinCtrl_Priority, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Visualise = new wxButton(this, ID_BUTTON3, _("Visualise"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    FlexGridSizer2->Add(Button_Visualise, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
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
    Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ZCPPDialog::OnButton_VisualiseClick);
    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ZCPPDialog::OnButton_OkClick);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ZCPPDialog::OnButton_CancelClick);
    //*)

    CheckBox_SuppressDuplicates->SetValue(_zcpp->IsSuppressDuplicateFrames());
    SpinCtrl_Channels->SetValue(_zcpp->GetChannels());
    SpinCtrl_Priority->SetValue(_zcpp->GetPriority());
    TextCtrl_Description->SetValue(_zcpp->GetDescription());
    TextCtrlIpAddr->SetValue(_zcpp->GetIP());
    CheckBoxAutoSizeOutput->SetValue(_zcpp->GetAutoSize());
    CheckBox_SupportVirtualStrings->SetValue(_zcpp->IsSupportsVirtualStrings());
    CheckBox_SupportSmartRemotes->SetValue(_zcpp->IsSupportsSmartRemotes());
    CheckBox_SuppressSendingConfig->SetValue(_zcpp->IsDontConfigure());
    CheckBox_Multicast->SetValue(_zcpp->IsMulticast());

    Button_Ok->SetDefault();
    ValidateWindow();

    Layout();
}

ZCPPDialog::~ZCPPDialog()
{
    //(*Destroy(ZCPPDialog)
    //*)
}

void ZCPPDialog::OnTextCtrl_DescriptionText(wxCommandEvent& event)
{
    ValidateWindow();
}

void ZCPPDialog::OnTextCtrlIpAddrText(wxCommandEvent& event)
{
    ValidateWindow();
}

void ZCPPDialog::OnButton_OkClick(wxCommandEvent& event)
{
    _zcpp->SetIP(TextCtrlIpAddr->GetValue().ToStdString());
    _zcpp->SetChannels(SpinCtrl_Channels->GetValue());
    _zcpp->SetPriority(SpinCtrl_Priority->GetValue());
    _zcpp->SetDescription(TextCtrl_Description->GetValue().ToStdString());
    _zcpp->SetSuppressDuplicateFrames(CheckBox_SuppressDuplicates->IsChecked());
    _zcpp->SetAutoSize(CheckBoxAutoSizeOutput->IsChecked());
    _zcpp->SetSupportsVirtualStrings(CheckBox_SupportVirtualStrings->IsChecked());
    _zcpp->SetSupportsSmartRemotes(CheckBox_SupportSmartRemotes->IsChecked());
    _zcpp->SetMulticast(CheckBox_Multicast->IsChecked());
    _zcpp->SetDontConfigure(CheckBox_SuppressSendingConfig->IsChecked());

    EndDialog(wxID_OK);
}

void ZCPPDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

bool ZCPPDialog::IsUniqueDescription(const std::string& newDescription, ZCPPOutput* output, OutputManager* outputManager)
{
    for (auto it : outputManager->GetOutputs())
    {
        if (it->GetType() == "ZCPP" && it != output && newDescription == it->GetDescription())
        {
            return false;
        }
    }
    return true;
}

bool ZCPPDialog::IsUniqueIP(const std::string& newIP, ZCPPOutput* output, OutputManager* outputManager)
{
    for (auto it : outputManager->GetOutputs())
    {
        if (it->GetType() == "ZCPP" && it != output && newIP == it->GetIP())
        {
            return false;
        }
    }
    return true;
}

void ZCPPDialog::ValidateWindow()
{
    if (TextCtrlIpAddr->GetValue().IsEmpty() ||
        !IsIPValidOrHostname(TextCtrlIpAddr->GetValue().ToStdString(), true))
    {
        Button_Ok->Enable(false);
        CheckBox_Multicast->SetLabel("");
    }
    else
    {
        if (TextCtrlIpAddr->GetValue().Contains("."))
        {
            CheckBox_Multicast->SetLabel(wxString::Format("Multicast to 224.0.31.%s", TextCtrlIpAddr->GetValue().AfterLast('.')));
            Fit();
        }
        else
        {
            CheckBox_Multicast->SetLabel("");
        }

        if (TextCtrl_Description->GetValue() == "" || !IsUniqueDescription(TextCtrl_Description->GetValue(), _zcpp, _outputManager) || !IsUniqueIP(TextCtrlIpAddr->GetValue(), _zcpp, _outputManager))
        {
            Button_Ok->Enable(false);
        }
        else
        {
            Button_Ok->Enable();
        }
    }
}

void ZCPPDialog::OnSpinCtrl_ChannelsChange(wxSpinEvent& event)
{
}

void ZCPPDialog::OnButton_VisualiseClick(wxCommandEvent& event)
{
    std::string check;
    std::list<int> nullList;
    UDController cud(TextCtrlIpAddr->GetValue().ToStdString(), TextCtrlIpAddr->GetValue().ToStdString(), _modelManager, _outputManager, &nullList, check);
    ControllerVisualiseDialog dlg(this, cud);
    dlg.ShowModal();
}
