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
const long RemoteFalconSettingsDialog::ID_BUTTON1 = wxNewId();
const long RemoteFalconSettingsDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(RemoteFalconSettingsDialog,wxDialog)
	//(*EventTable(RemoteFalconSettingsDialog)
	//*)
END_EVENT_TABLE()

RemoteFalconSettingsDialog::RemoteFalconSettingsDialog(wxWindow* parent, RemoteFalconOptions* options, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _options = options;

	//(*Initialize(RemoteFalconSettingsDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	StaticText_Token = new wxStaticText(this, ID_STATICTEXT5, _("Token"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer3->Add(StaticText_Token, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
	TextCtrl_Token = new wxTextCtrl(this, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD, wxDefaultValidator, _T("ID_TEXTCTRL3"));
	FlexGridSizer3->Add(TextCtrl_Token, 1, wxALL|wxEXPAND, 2);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Playlist"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer3->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Playlists = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	FlexGridSizer3->Add(Choice_Playlists, 1, wxALL|wxEXPAND, 5);
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
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&RemoteFalconSettingsDialog::OnButton_OkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&RemoteFalconSettingsDialog::OnButton_CancelClick);
	//*)

    SetEscapeId(ID_BUTTON2);

	int sel = -1;
	auto playlists = xSchedule::GetPlaylists();
	for (const auto& it : playlists) {
		Choice_Playlists->AppendString(it.first);
		_plids.push_back(it.second);
		if (it.second == options->GetPlaylist()) sel = Choice_Playlists->GetCount() - 1;
	}

    TextCtrl_Token->SetValue(options->GetToken());
	Choice_Playlists->SetSelection(sel);

    ValidateWindow();
}

RemoteFalconSettingsDialog::~RemoteFalconSettingsDialog()
{
	//(*Destroy(RemoteFalconSettingsDialog)
	//*)
}


void RemoteFalconSettingsDialog::OnButton_OkClick(wxCommandEvent& event)
{
    _options->SetToken(TextCtrl_Token->GetValue().ToStdString());
	_options->SetPlaylist(_plids[Choice_Playlists->GetSelection()]);

    EndDialog(wxID_OK);
}

void RemoteFalconSettingsDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void RemoteFalconSettingsDialog::ValidateWindow()
{
        if (TextCtrl_Token->GetValue() == "" || Choice_Playlists->GetStringSelection() == "")
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
