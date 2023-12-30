/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ExcludeDimDialog.h"
#include "xScheduleMain.h"
#include "ScheduleManager.h"
#include "../xLights/outputs/OutputManager.h"

//(*InternalHeaders(ExcludeDimDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(ExcludeDimDialog)
const long ExcludeDimDialog::ID_STATICTEXT1 = wxNewId();
const long ExcludeDimDialog::ID_TEXTCTRL2 = wxNewId();
const long ExcludeDimDialog::ID_STATICTEXT5 = wxNewId();
const long ExcludeDimDialog::ID_STATICTEXT2 = wxNewId();
const long ExcludeDimDialog::ID_SPINCTRL2 = wxNewId();
const long ExcludeDimDialog::ID_STATICTEXT4 = wxNewId();
const long ExcludeDimDialog::ID_TEXTCTRL1 = wxNewId();
const long ExcludeDimDialog::ID_CHECKBOX1 = wxNewId();
const long ExcludeDimDialog::ID_BUTTON1 = wxNewId();
const long ExcludeDimDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(ExcludeDimDialog,wxDialog)
	//(*EventTable(ExcludeDimDialog)
	//*)
END_EVENT_TABLE()

ExcludeDimDialog::ExcludeDimDialog(wxWindow* parent, OutputManager* outputManager, std::string& startChannel, size_t& channels, std::string& description, bool& enabled, wxWindowID id,const wxPoint& pos,const wxSize& size) : _startChannel(startChannel), _channels(channels), _description(description), _enabled(enabled)
{
    _outputManager = outputManager;

	//(*Initialize(ExcludeDimDialog)
	wxBoxSizer* BoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;

	Create(parent, id, _("Exclude Dim"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Start Channel:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	TextCtrl_StartChannel = new wxTextCtrl(this, ID_TEXTCTRL2, _("1"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer2->Add(TextCtrl_StartChannel, 1, wxALL|wxEXPAND, 5);
	StaticText_StartChannel = new wxStaticText(this, ID_STATICTEXT5, _("1"), wxDefaultPosition, wxSize(60,-1), 0, _T("ID_STATICTEXT5"));
	FlexGridSizer2->Add(StaticText_StartChannel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Channels:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Channels = new wxSpinCtrl(this, ID_SPINCTRL2, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 100, 1, _T("ID_SPINCTRL2"));
	SpinCtrl_Channels->SetValue(_T("1"));
	FlexGridSizer1->Add(SpinCtrl_Channels, 1, wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Description:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
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

	Connect(ID_TEXTCTRL2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ExcludeDimDialog::OnTextCtrl_StartChannelText);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ExcludeDimDialog::OnButton_OkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ExcludeDimDialog::OnButton_CancelClick);
	//*)

    long chs = xScheduleFrame::GetScheduleManager()->GetTotalChannels();
    SpinCtrl_Channels->SetRange(1, chs);

    TextCtrl_StartChannel->SetValue(_startChannel);
    SpinCtrl_Channels->SetValue(_channels);
    TextCtrl_Description->SetValue(_description);
    CheckBox_Enabled->SetValue(_enabled);

    SetEscapeId(Button_Cancel->GetId());
    SetAffirmativeId(Button_Ok->GetId());
    ValidateWindow();
}

void ExcludeDimDialog::ValidateWindow()
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

ExcludeDimDialog::~ExcludeDimDialog()
{
	//(*Destroy(ExcludeDimDialog)
	//*)
}

void ExcludeDimDialog::OnButton_OkClick(wxCommandEvent& event)
{
    _startChannel = TextCtrl_StartChannel->GetValue();
    _channels = SpinCtrl_Channels->GetValue();
    _description = TextCtrl_Description->GetValue();
    _enabled = CheckBox_Enabled->IsChecked();
    EndDialog(wxID_OK);
}

void ExcludeDimDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void ExcludeDimDialog::OnTextCtrl_StartChannelText(wxCommandEvent& event)
{
    ValidateWindow();
}
