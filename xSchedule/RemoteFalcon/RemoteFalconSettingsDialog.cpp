/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "RemoteFalconSettingsDialog.h"

//(*InternalHeaders(RemoteFalconSettingsDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "RemoteFalconOptions.h"
#include "xSchedule.h"
#include "../../xLights/UtilFunctions.h"

//(*IdInit(RemoteFalconSettingsDialog)
const long RemoteFalconSettingsDialog::ID_STATICTEXT5 = wxNewId();
const long RemoteFalconSettingsDialog::ID_TEXTCTRL3 = wxNewId();
const long RemoteFalconSettingsDialog::ID_STATICTEXT1 = wxNewId();
const long RemoteFalconSettingsDialog::ID_CHOICE1 = wxNewId();
const long RemoteFalconSettingsDialog::ID_CHECKBOX1 = wxNewId();
const long RemoteFalconSettingsDialog::ID_CHECKBOX2 = wxNewId();
const long RemoteFalconSettingsDialog::ID_CHECKBOX3 = wxNewId();
const long RemoteFalconSettingsDialog::ID_STATICTEXT2 = wxNewId();
const long RemoteFalconSettingsDialog::ID_SPINCTRL1 = wxNewId();
const long RemoteFalconSettingsDialog::ID_STATICTEXT3 = wxNewId();
const long RemoteFalconSettingsDialog::ID_CHECKLISTBOX1 = wxNewId();
const long RemoteFalconSettingsDialog::ID_BUTTON1 = wxNewId();
const long RemoteFalconSettingsDialog::ID_BUTTON2 = wxNewId();
//*)

const long RemoteFalconSettingsDialog::ID_MNU_SELECTALL = wxNewId();
const long RemoteFalconSettingsDialog::ID_MNU_SELECTNONE = wxNewId();
const long RemoteFalconSettingsDialog::ID_MNU_SELECTHIGH = wxNewId();
const long RemoteFalconSettingsDialog::ID_MNU_DESELECTHIGH = wxNewId();

BEGIN_EVENT_TABLE(RemoteFalconSettingsDialog,wxDialog)
	//(*EventTable(RemoteFalconSettingsDialog)
	//*)
END_EVENT_TABLE()

RemoteFalconSettingsDialog::RemoteFalconSettingsDialog(wxWindow* parent, RemoteFalconOptions* options, wxWindowID id, const wxPoint& pos, const wxSize& size)
{
	_options = options;

	//(*Initialize(RemoteFalconSettingsDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer3->AddGrowableCol(1);
	FlexGridSizer3->AddGrowableRow(6);
	StaticText_Token = new wxStaticText(this, ID_STATICTEXT5, _("Token:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer3->Add(StaticText_Token, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_Token = new wxTextCtrl(this, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD, wxDefaultValidator, _T("ID_TEXTCTRL3"));
	FlexGridSizer3->Add(TextCtrl_Token, 1, wxALL|wxEXPAND, 2);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Playlist:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer3->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	Choice_Playlists = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	FlexGridSizer3->Add(Choice_Playlists, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer3->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_ImmediatelyInterrupt = new wxCheckBox(this, ID_CHECKBOX1, _("Immediately interrupt schedule"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_ImmediatelyInterrupt->SetValue(true);
	FlexGridSizer3->Add(CheckBox_ImmediatelyInterrupt, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer3->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_ClearQueue = new wxCheckBox(this, ID_CHECKBOX2, _("Clear Remote Falcon queue on start"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
	CheckBox_ClearQueue->SetValue(false);
	FlexGridSizer3->Add(CheckBox_ClearQueue, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer3->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_SendEnableDisable = new wxCheckBox(this, ID_CHECKBOX3, _("Enable/Disable Remote Falcon Website"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX3"));
	CheckBox_SendEnableDisable->SetValue(false);
	FlexGridSizer3->Add(CheckBox_SendEnableDisable, 1, wxALL|wxEXPAND, 2);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Time to grab next song (seconds):"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer3->Add(StaticText2, 1, wxALL|wxEXPAND, 2);
	SpinCtrl_LeadTime = new wxSpinCtrl(this, ID_SPINCTRL1, _T("5"), wxDefaultPosition, wxDefaultSize, 0, 1, 20, 5, _T("ID_SPINCTRL1"));
	SpinCtrl_LeadTime->SetValue(_T("5"));
	FlexGridSizer3->Add(SpinCtrl_LeadTime, 1, wxALL|wxEXPAND, 2);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Play during playlists:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer3->Add(StaticText3, 1, wxALL|wxEXPAND, 5);
	CheckListBox_Playlists = new wxCheckListBox(this, ID_CHECKLISTBOX1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHECKLISTBOX1"));
	FlexGridSizer3->Add(CheckListBox_Playlists, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer2->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer2->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL3,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&RemoteFalconSettingsDialog::OnTextCtrl_TokenText);
	Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&RemoteFalconSettingsDialog::OnChoice_PlaylistsSelect);
	Connect(ID_CHECKLISTBOX1,wxEVT_COMMAND_CHECKLISTBOX_TOGGLED,(wxObjectEventFunction)&RemoteFalconSettingsDialog::OnCheckListBox_PlaylistsToggled);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&RemoteFalconSettingsDialog::OnButton_OkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&RemoteFalconSettingsDialog::OnButton_CancelClick);
	//*)

	Connect(ID_CHECKLISTBOX1, wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&RemoteFalconSettingsDialog::OnPreviewRightDown);

	SetEscapeId(ID_BUTTON2);

	int sel = -1;
	_playlists = xSchedule::GetPlaylists();
	for (const auto& it : _playlists) {
		int x = CheckListBox_Playlists->Append(it.first);
		if (options->IsPlayDuring(it.first)) {
			CheckListBox_Playlists->Check(x, true);
		}
		Choice_Playlists->AppendString(it.first);
		_plids.push_back(it.second);
		if (it.second == options->GetPlaylist()) sel = Choice_Playlists->GetCount() - 1;
	}

	TextCtrl_Token->SetValue(options->GetToken());
	Choice_Playlists->SetSelection(sel);
	CheckBox_ImmediatelyInterrupt->SetValue(options->GetImmediatelyInterrupt());
	CheckBox_ClearQueue->SetValue(options->GetClearQueueOnStart());
	SpinCtrl_LeadTime->SetValue(options->GetLeadTime());
	CheckBox_SendEnableDisable->SetValue(options->IsEnableDisable());

	ValidateWindow();
}

RemoteFalconSettingsDialog::~RemoteFalconSettingsDialog()
{
	//(*Destroy(RemoteFalconSettingsDialog)
	//*)
}

void RemoteFalconSettingsDialog::OnPreviewRightDown(wxMouseEvent& event)
{
	wxMenu mnu;
	mnu.Append(ID_MNU_SELECTALL, "Select All");
	mnu.Append(ID_MNU_SELECTNONE, "Select None");
	mnu.Append(ID_MNU_SELECTHIGH, "Select Highlighted");
	mnu.Append(ID_MNU_DESELECTHIGH, "Deselect Highlighted");

	mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&RemoteFalconSettingsDialog::OnPopupCommand, nullptr, this);
	PopupMenu(&mnu);
}

void RemoteFalconSettingsDialog::OnPopupCommand(wxCommandEvent& event)
{
	if (event.GetId() == ID_MNU_SELECTALL || event.GetId() == ID_MNU_SELECTNONE) {
		for (size_t x = 0; x < CheckListBox_Playlists->GetCount(); x++) {
			CheckListBox_Playlists->Check(x, event.GetId() == ID_MNU_SELECTALL);
		}
	}
	else if (event.GetId() == ID_MNU_SELECTHIGH || event.GetId() == ID_MNU_DESELECTHIGH) {
		for (size_t x = 0; x < CheckListBox_Playlists->GetCount(); x++) {
			if (CheckListBox_Playlists->IsSelected(x)) {
				CheckListBox_Playlists->Check(x, event.GetId() == ID_MNU_SELECTHIGH);
			}
		}
	}
	ValidateWindow();
}

void RemoteFalconSettingsDialog::OnButton_OkClick(wxCommandEvent& event)
{
    _options->SetToken(TextCtrl_Token->GetValue().ToStdString());
	_options->SetPlaylist(_plids[Choice_Playlists->GetSelection()]);
	_options->SetLeadTime(SpinCtrl_LeadTime->GetValue());
	_options->SetImmediatelyInterrupt(CheckBox_ImmediatelyInterrupt->IsChecked());
	_options->SetClearQueueOnStart(CheckBox_ClearQueue->IsChecked());
	_options->SetEnableDisable(CheckBox_SendEnableDisable->IsChecked());

	wxArrayInt checked;
	CheckListBox_Playlists->GetCheckedItems(checked);

	if (checked.size() == CheckListBox_Playlists->GetCount()) 		{
		_options->SetPlayDuring("");
	}
	else 		{
		std::string pd = "|";
		for (const auto& it : checked) 			{
			pd += CheckListBox_Playlists->GetString(it) + "|";
		}
		_options->SetPlayDuring(pd);
	}

    EndDialog(wxID_OK);
}

void RemoteFalconSettingsDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void RemoteFalconSettingsDialog::ValidateWindow()
{
	wxArrayInt checked;
	CheckListBox_Playlists->GetCheckedItems(checked);
    if (TextCtrl_Token->GetValue() == "" || Choice_Playlists->GetStringSelection() == "" || checked.size() == 0)
    {
        Button_Ok->Disable();
    }
    else
    {
        Button_Ok->Enable();
    }
}

void RemoteFalconSettingsDialog::OnTextCtrl_TokenText(wxCommandEvent& event)
{
    ValidateWindow();
}

void RemoteFalconSettingsDialog::OnChoice_PlaylistsSelect(wxCommandEvent& event)
{
    ValidateWindow();
}

void RemoteFalconSettingsDialog::OnCheckListBox_PlaylistsToggled(wxCommandEvent& event)
{
    ValidateWindow();
}
