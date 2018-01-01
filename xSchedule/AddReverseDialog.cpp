#include "AddReverseDialog.h"
#include "xScheduleMain.h"
#include "ScheduleManager.h"
#include "../xLights/outputs/OutputManager.h"

//(*InternalHeaders(AddReverseDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(AddReverseDialog)
const long AddReverseDialog::ID_STATICTEXT1 = wxNewId();
const long AddReverseDialog::ID_TEXTCTRL2 = wxNewId();
const long AddReverseDialog::ID_STATICTEXT4 = wxNewId();
const long AddReverseDialog::ID_STATICTEXT2 = wxNewId();
const long AddReverseDialog::ID_SPINCTRL2 = wxNewId();
const long AddReverseDialog::ID_STATICTEXT3 = wxNewId();
const long AddReverseDialog::ID_TEXTCTRL1 = wxNewId();
const long AddReverseDialog::ID_CHECKBOX1 = wxNewId();
const long AddReverseDialog::ID_BUTTON1 = wxNewId();
const long AddReverseDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(AddReverseDialog,wxDialog)
	//(*EventTable(AddReverseDialog)
	//*)
END_EVENT_TABLE()

void AddReverseDialog::ValidateWindow()
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

AddReverseDialog::AddReverseDialog(wxWindow* parent, OutputManager* outputManager, std::string& startChannel, size_t& nodes, size_t& ignore, std::string& description, bool& enabled, wxWindowID id, const wxPoint& pos, const wxSize& size) : _startChannel(startChannel), _nodes(nodes), _description(description), _enabled(enabled)
{
    _outputManager = outputManager;

	//(*Initialize(AddReverseDialog)
	wxFlexGridSizer* FlexGridSizer2;
	wxBoxSizer* BoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, _("Reverse Nodes"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Start Channel:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	TextCtrl_StartChannel = new wxTextCtrl(this, ID_TEXTCTRL2, _("1"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer2->Add(TextCtrl_StartChannel, 1, wxALL|wxEXPAND, 5);
	StaticText_StartChannel = new wxStaticText(this, ID_STATICTEXT4, _("1"), wxDefaultPosition, wxSize(60,-1), 0, _T("ID_STATICTEXT4"));
	FlexGridSizer2->Add(StaticText_StartChannel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Nodes:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Nodes = new wxSpinCtrl(this, ID_SPINCTRL2, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 100, 1, _T("ID_SPINCTRL2"));
	SpinCtrl_Nodes->SetValue(_T("1"));
	FlexGridSizer1->Add(SpinCtrl_Nodes, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Description:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Description = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(TextCtrl_Description, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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

	Connect(ID_TEXTCTRL2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&AddReverseDialog::OnTextCtrl_StartChannelText);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&AddReverseDialog::OnButton_OkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&AddReverseDialog::OnButton_CancelClick);
	//*)

    long chs = xScheduleFrame::GetScheduleManager()->GetTotalChannels();
    SpinCtrl_Nodes->SetRange(1, chs / 3);

    TextCtrl_StartChannel->SetValue(_startChannel);
    SpinCtrl_Nodes->SetValue(_nodes);
    TextCtrl_Description->SetValue(_description);
    CheckBox_Enabled->SetValue(_enabled);

    SetEscapeId(Button_Cancel->GetId());
    SetAffirmativeId(Button_Ok->GetId());
    ValidateWindow();
}

AddReverseDialog::~AddReverseDialog()
{
	//(*Destroy(AddReverseDialog)
	//*)
}

void AddReverseDialog::OnButton_OkClick(wxCommandEvent& event)
{
    _startChannel = TextCtrl_StartChannel->GetValue();
    _nodes = SpinCtrl_Nodes->GetValue();
    _description = TextCtrl_Description->GetValue();
    _enabled = CheckBox_Enabled->IsChecked();

    EndDialog(wxID_OK);
}

void AddReverseDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void AddReverseDialog::OnTextCtrl_StartChannelText(wxCommandEvent& event)
{
    ValidateWindow();
}
