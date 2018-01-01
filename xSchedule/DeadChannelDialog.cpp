#include "DeadChannelDialog.h"
#include "xScheduleMain.h"
#include "ScheduleManager.h"
#include "../xLights/outputs/OutputManager.h"

//(*InternalHeaders(DeadChannelDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(DeadChannelDialog)
const long DeadChannelDialog::ID_STATICTEXT1 = wxNewId();
const long DeadChannelDialog::ID_TEXTCTRL2 = wxNewId();
const long DeadChannelDialog::ID_STATICTEXT4 = wxNewId();
const long DeadChannelDialog::ID_STATICTEXT2 = wxNewId();
const long DeadChannelDialog::ID_CHOICE1 = wxNewId();
const long DeadChannelDialog::ID_STATICTEXT3 = wxNewId();
const long DeadChannelDialog::ID_TEXTCTRL1 = wxNewId();
const long DeadChannelDialog::ID_CHECKBOX1 = wxNewId();
const long DeadChannelDialog::ID_BUTTON1 = wxNewId();
const long DeadChannelDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(DeadChannelDialog,wxDialog)
	//(*EventTable(DeadChannelDialog)
	//*)
END_EVENT_TABLE()

DeadChannelDialog::DeadChannelDialog(wxWindow* parent, OutputManager* outputManager, std::string& nodeStartChannel, size_t& channel, std::string& description, bool& enabled, wxWindowID id,const wxPoint& pos,const wxSize& size) : _nodeStartChannel(nodeStartChannel), _channel(channel), _description(description), _enabled(enabled)
{
    _outputManager = outputManager;

	//(*Initialize(DeadChannelDialog)
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Node Start Channel"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	TextCtrl_StartChannel = new wxTextCtrl(this, ID_TEXTCTRL2, _("1"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer3->Add(TextCtrl_StartChannel, 1, wxALL|wxEXPAND, 5);
	StaticText_StartChannel = new wxStaticText(this, ID_STATICTEXT4, _("1"), wxDefaultPosition, wxSize(60,-1), 0, _T("ID_STATICTEXT4"));
	FlexGridSizer3->Add(StaticText_StartChannel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Channel"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Channel = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	Choice_Channel->SetSelection( Choice_Channel->Append(_("1")) );
	Choice_Channel->Append(_("2"));
	Choice_Channel->Append(_("3"));
	FlexGridSizer1->Add(Choice_Channel, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Description"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Description = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(TextCtrl_Description, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Enabled = new wxCheckBox(this, ID_CHECKBOX1, _("Enabled"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_Enabled->SetValue(false);
	FlexGridSizer1->Add(CheckBox_Enabled, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer2->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer2->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&DeadChannelDialog::OnTextCtrl_StartChannelText);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DeadChannelDialog::OnButton_OkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DeadChannelDialog::OnButton_CancelClick);
	//*)

    if (_channel == 0) _channel = 1;

    long chs = xScheduleFrame::GetScheduleManager()->GetTotalChannels();

    TextCtrl_StartChannel->SetValue(_nodeStartChannel);
    Choice_Channel->SetSelection(_channel - 1);
    TextCtrl_Description->SetValue(_description);
    CheckBox_Enabled->SetValue(_enabled);

    SetEscapeId(Button_Cancel->GetId());
    SetAffirmativeId(Button_Ok->GetId());

    ValidateWindow();
}

DeadChannelDialog::~DeadChannelDialog()
{
	//(*Destroy(DeadChannelDialog)
	//*)
}


void DeadChannelDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void DeadChannelDialog::OnButton_OkClick(wxCommandEvent& event)
{
    _nodeStartChannel = TextCtrl_StartChannel->GetValue().ToStdString();
    _channel = wxAtoi(Choice_Channel->GetStringSelection());
    _description = TextCtrl_Description->GetValue().ToStdString();
    _enabled = CheckBox_Enabled->IsChecked();

    EndDialog(wxID_OK);
}

void DeadChannelDialog::ValidateWindow()
{
    long sc = _outputManager->DecodeStartChannel(TextCtrl_StartChannel->GetValue().ToStdString());
    StaticText_StartChannel->SetLabel(wxString::Format("%ld", (long)sc));
    if (sc == 0 || sc > xScheduleFrame::GetScheduleManager()->GetTotalChannels())
    {
        Button_Ok->Enable(false);
    }
    else
    {
        Button_Ok->Enable(true);
    }
}

void DeadChannelDialog::OnTextCtrl_StartChannelText(wxCommandEvent& event)
{
    ValidateWindow();
}
