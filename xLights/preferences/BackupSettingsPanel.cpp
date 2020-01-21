#include "BackupSettingsPanel.h"

//(*InternalHeaders(BackupSettingsPanel)
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/gbsizer.h>
#include <wx/intl.h>
#include <wx/stattext.h>
#include <wx/string.h>
//*)

#include <wx/preferences.h>
#include "../xLightsMain.h"

//(*IdInit(BackupSettingsPanel)
const long BackupSettingsPanel::ID_CHECKBOX1 = wxNewId();
const long BackupSettingsPanel::ID_CHECKBOX2 = wxNewId();
const long BackupSettingsPanel::ID_CHECKBOX3 = wxNewId();
const long BackupSettingsPanel::ID_CHOICE1 = wxNewId();
const long BackupSettingsPanel::ID_STATICTEXT1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(BackupSettingsPanel,wxPanel)
	//(*EventTable(BackupSettingsPanel)
	//*)
END_EVENT_TABLE()


BackupSettingsPanel::BackupSettingsPanel(wxWindow* parent, xLightsFrame *f, wxWindowID id) : frame(f)
{
	//(*Initialize(BackupSettingsPanel)
	wxGridBagSizer* GridBagSizer1;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	GridBagSizer1 = new wxGridBagSizer(0, 0);
	BackupOnSaveCheckBox = new wxCheckBox(this, ID_CHECKBOX1, _("Backup On Save"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	BackupOnSaveCheckBox->SetValue(false);
	GridBagSizer1->Add(BackupOnSaveCheckBox, wxGBPosition(0, 0), wxGBSpan(1, 2), wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	BackupOnLaunchCheckBox = new wxCheckBox(this, ID_CHECKBOX2, _("Backup On Launch"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
	BackupOnLaunchCheckBox->SetValue(false);
	GridBagSizer1->Add(BackupOnLaunchCheckBox, wxGBPosition(1, 0), wxGBSpan(1, 2), wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	BackupSubfoldersCheckBox = new wxCheckBox(this, ID_CHECKBOX3, _("Backup Subfolders"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX3"));
	BackupSubfoldersCheckBox->SetValue(false);
	GridBagSizer1->Add(BackupSubfoldersCheckBox, wxGBPosition(2, 0), wxGBSpan(1, 2), wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	PurgeIntervalChoice = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	PurgeIntervalChoice->SetSelection( PurgeIntervalChoice->Append(_("Never")) );
	PurgeIntervalChoice->Append(_("Older than 360 days"));
	PurgeIntervalChoice->Append(_("Older than 90 days"));
	PurgeIntervalChoice->Append(_("Older than 30 days"));
	PurgeIntervalChoice->Append(_("Older than 7 days"));
	GridBagSizer1->Add(PurgeIntervalChoice, wxGBPosition(3, 1), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Purge Backups"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	GridBagSizer1->Add(StaticText1, wxGBPosition(3, 0), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(GridBagSizer1);
	GridBagSizer1->Fit(this);
	GridBagSizer1->SetSizeHints(this);

	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&BackupSettingsPanel::OnBackupOnSaveCheckBoxClick);
	Connect(ID_CHECKBOX2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&BackupSettingsPanel::OnBackupOnLaunchCheckBoxClick);
	Connect(ID_CHECKBOX3,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&BackupSettingsPanel::OnBackupSubfoldersCheckBoxClick);
	Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&BackupSettingsPanel::OnPurgeIntervalChoiceSelect);
	//*)
}

BackupSettingsPanel::~BackupSettingsPanel()
{
	//(*Destroy(BackupSettingsPanel)
	//*)
}
bool BackupSettingsPanel::TransferDataToWindow() {
    BackupOnSaveCheckBox->SetValue(frame->BackupOnSave());
    BackupOnLaunchCheckBox->SetValue(frame->BackupOnLaunch());
    BackupSubfoldersCheckBox->SetValue(frame->BackupSubFolders());

    switch (frame->GetBackupPugeDays()) {
        case 365:
            PurgeIntervalChoice->Select(1);
            break;
        case 90:
            PurgeIntervalChoice->Select(2);
            break;
        case 31:
            PurgeIntervalChoice->Select(3);
            break;
        case 7:
            PurgeIntervalChoice->Select(4);
            break;
        default:
            PurgeIntervalChoice->Select(0);
            break;
    }
    return true;
}


bool BackupSettingsPanel::TransferDataFromWindow() {
    frame->SetBackupOnSave(BackupOnSaveCheckBox->IsChecked());
    frame->SetBackupOnLaunch(BackupOnLaunchCheckBox->IsChecked());
    frame->SetBackupSubFolders(BackupSubfoldersCheckBox->IsChecked());
    int bpi = 0;
    switch (PurgeIntervalChoice->GetSelection()) {
        case 0:
            bpi = 0;
            break;
        case 1:
            bpi = 365;
            break;
        case 2:
            bpi = 90;
            break;
        case 3:
            bpi = 31;
            break;
        case 4:
            bpi = 7;
            break;
    }
    frame->SetBackupPurgeDays(bpi);
    return true;
}


void BackupSettingsPanel::OnBackupOnSaveCheckBoxClick(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void BackupSettingsPanel::OnBackupOnLaunchCheckBoxClick(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void BackupSettingsPanel::OnBackupSubfoldersCheckBoxClick(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void BackupSettingsPanel::OnPurgeIntervalChoiceSelect(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}
