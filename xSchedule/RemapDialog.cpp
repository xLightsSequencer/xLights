#include "RemapDialog.h"
#include "xScheduleMain.h"
#include "ScheduleManager.h"

//(*InternalHeaders(RemapDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(RemapDialog)
const long RemapDialog::ID_STATICTEXT1 = wxNewId();
const long RemapDialog::ID_SPINCTRL1 = wxNewId();
const long RemapDialog::ID_STATICTEXT2 = wxNewId();
const long RemapDialog::ID_SPINCTRL2 = wxNewId();
const long RemapDialog::ID_STATICTEXT3 = wxNewId();
const long RemapDialog::ID_SPINCTRL3 = wxNewId();
const long RemapDialog::ID_STATICTEXT4 = wxNewId();
const long RemapDialog::ID_TEXTCTRL1 = wxNewId();
const long RemapDialog::ID_CHECKBOX1 = wxNewId();
const long RemapDialog::ID_BUTTON1 = wxNewId();
const long RemapDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(RemapDialog,wxDialog)
	//(*EventTable(RemapDialog)
	//*)
END_EVENT_TABLE()

RemapDialog::RemapDialog(wxWindow* parent, size_t& startChannel, size_t& to, size_t& channels, std::string& description, bool& enabled, wxWindowID id, const wxPoint& pos, const wxSize& size) : _from(startChannel), _to(to), _channels(channels), _description(description), _enabled(enabled)
{
    //(*Initialize(RemapDialog)
    wxBoxSizer* BoxSizer1;
    wxFlexGridSizer* FlexGridSizer1;

    Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
    SetClientSize(wxDefaultSize);
    Move(wxDefaultPosition);
    FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
    StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("From Channel:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    SpinCtrl_FromChannel = new wxSpinCtrl(this, ID_SPINCTRL1, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 100, 1, _T("ID_SPINCTRL1"));
    SpinCtrl_FromChannel->SetValue(_T("1"));
    FlexGridSizer1->Add(SpinCtrl_FromChannel, 1, wxALL|wxEXPAND, 5);
    StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("To Channel:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    SpinCtrl_ToChannel = new wxSpinCtrl(this, ID_SPINCTRL2, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 100, 1, _T("ID_SPINCTRL2"));
    SpinCtrl_ToChannel->SetValue(_T("1"));
    FlexGridSizer1->Add(SpinCtrl_ToChannel, 1, wxALL|wxEXPAND, 5);
    StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Channels:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    SpinCtrl_Channels = new wxSpinCtrl(this, ID_SPINCTRL3, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 100, 1, _T("ID_SPINCTRL3"));
    SpinCtrl_Channels->SetValue(_T("1"));
    FlexGridSizer1->Add(SpinCtrl_Channels, 1, wxALL|wxEXPAND, 5);
    StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Description:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_Description = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
    FlexGridSizer1->Add(TextCtrl_Description, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer1->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_Enabled = new wxCheckBox(this, ID_CHECKBOX1, _("Enabled"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
    CheckBox_Enabled->SetValue(false);
    FlexGridSizer1->Add(CheckBox_Enabled, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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

    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&RemapDialog::OnButton_OkClick);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&RemapDialog::OnButton_CancelClick);
    //*)

    long chs = xScheduleFrame::GetScheduleManager()->GetTotalChannels();
    SpinCtrl_FromChannel->SetRange(1, chs);
    SpinCtrl_ToChannel->SetRange(1, chs);
    SpinCtrl_Channels->SetRange(1, chs);

    SpinCtrl_FromChannel->SetValue(_from);
    SpinCtrl_ToChannel->SetValue(_to);
    SpinCtrl_Channels->SetValue(_channels);
    TextCtrl_Description->SetValue(_description);
    CheckBox_Enabled->SetValue(_enabled);

    SetEscapeId(Button_Cancel->GetId());
    SetAffirmativeId(Button_Ok->GetId());
}

RemapDialog::~RemapDialog()
{
	//(*Destroy(RemapDialog)
	//*)
}


void RemapDialog::OnButton_OkClick(wxCommandEvent& event)
{
    _from = SpinCtrl_FromChannel->GetValue();
    _to = SpinCtrl_ToChannel->GetValue();
    _channels = SpinCtrl_Channels->GetValue();
    _description = TextCtrl_Description->GetValue();
    _enabled = CheckBox_Enabled->IsChecked();

    EndDialog(wxID_OK);
}

void RemapDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}
