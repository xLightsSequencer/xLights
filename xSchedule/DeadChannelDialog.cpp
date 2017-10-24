#include "DeadChannelDialog.h"
#include "xScheduleMain.h"
#include "ScheduleManager.h"

//(*InternalHeaders(DeadChannelDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(DeadChannelDialog)
const long DeadChannelDialog::ID_STATICTEXT1 = wxNewId();
const long DeadChannelDialog::ID_SPINCTRL1 = wxNewId();
const long DeadChannelDialog::ID_STATICTEXT2 = wxNewId();
const long DeadChannelDialog::ID_CHOICE1 = wxNewId();
const long DeadChannelDialog::ID_STATICTEXT3 = wxNewId();
const long DeadChannelDialog::ID_TEXTCTRL1 = wxNewId();
const long DeadChannelDialog::ID_BUTTON1 = wxNewId();
const long DeadChannelDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(DeadChannelDialog,wxDialog)
	//(*EventTable(DeadChannelDialog)
	//*)
END_EVENT_TABLE()

DeadChannelDialog::DeadChannelDialog(wxWindow* parent, size_t& nodeStartChannel, size_t& channel, std::string& description, wxWindowID id,const wxPoint& pos,const wxSize& size) : _nodeStartChannel(nodeStartChannel), _channel(channel), _description(description)
{
	//(*Initialize(DeadChannelDialog)
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Node Start Channel"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_NodeStartChannel = new wxSpinCtrl(this, ID_SPINCTRL1, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 100, 1, _T("ID_SPINCTRL1"));
	SpinCtrl_NodeStartChannel->SetValue(_T("1"));
	FlexGridSizer1->Add(SpinCtrl_NodeStartChannel, 1, wxALL|wxEXPAND, 5);
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
	FlexGridSizer1->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer2->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer2->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DeadChannelDialog::OnButton_OkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DeadChannelDialog::OnButton_CancelClick);
	//*)

    if (_channel == 0) _channel = 1;

    long chs = xScheduleFrame::GetScheduleManager()->GetTotalChannels();
    SpinCtrl_NodeStartChannel->SetRange(1, chs - 3);

    SpinCtrl_NodeStartChannel->SetValue(_nodeStartChannel);
    Choice_Channel->SetSelection(_channel - 1);
    TextCtrl_Description->SetValue(_description);

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
    _nodeStartChannel = SpinCtrl_NodeStartChannel->GetValue();
    _channel = wxAtoi(Choice_Channel->GetStringSelection());
    _description = TextCtrl_Description->GetValue().ToStdString();

    EndDialog(wxID_OK);
}

void DeadChannelDialog::ValidateWindow()
{
}
