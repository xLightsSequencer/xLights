/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "BackupSettingsPanel.h"

//(*InternalHeaders(BackupSettingsPanel)
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/filepicker.h>
#include <wx/gbsizer.h>
#include <wx/intl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/string.h>
//*)

#include <wx/preferences.h>
#include "../xLightsMain.h"

//(*IdInit(BackupSettingsPanel)
const long BackupSettingsPanel::ID_CHECKBOX1 = wxNewId();
const long BackupSettingsPanel::ID_CHECKBOX2 = wxNewId();
const long BackupSettingsPanel::ID_CHECKBOX3 = wxNewId();
const long BackupSettingsPanel::ID_STATICTEXT1 = wxNewId();
const long BackupSettingsPanel::ID_CHOICE1 = wxNewId();
const long BackupSettingsPanel::ID_CHECKBOX4 = wxNewId();
const long BackupSettingsPanel::ID_DIRPICKERCTRL1 = wxNewId();
const long BackupSettingsPanel::ID_DIRPICKERCTRL2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(BackupSettingsPanel,wxPanel)
	//(*EventTable(BackupSettingsPanel)
	//*)
END_EVENT_TABLE()


BackupSettingsPanel::BackupSettingsPanel(wxWindow* parent, xLightsFrame *f, wxWindowID id) : frame(f)
{
	//(*Initialize(BackupSettingsPanel)
	wxFlexGridSizer* FlexGridSizer1;
	wxGridBagSizer* GridBagSizer1;
	wxStaticBoxSizer* StaticBoxSizer1;
	wxStaticBoxSizer* StaticBoxSizer2;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	GridBagSizer1 = new wxGridBagSizer(0, 0);
	BackupOnSaveCheckBox = new wxCheckBox(this, ID_CHECKBOX1, _("Backup On Save"), wxDefaultPosition, wxSize(400,-1), 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	BackupOnSaveCheckBox->SetValue(false);
	GridBagSizer1->Add(BackupOnSaveCheckBox, wxGBPosition(0, 0), wxGBSpan(1, 2), wxALL|wxEXPAND, 5);
	BackupOnLaunchCheckBox = new wxCheckBox(this, ID_CHECKBOX2, _("Backup On Launch"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
	BackupOnLaunchCheckBox->SetValue(false);
	GridBagSizer1->Add(BackupOnLaunchCheckBox, wxGBPosition(1, 0), wxGBSpan(1, 2), wxALL|wxEXPAND, 5);
	BackupSubfoldersCheckBox = new wxCheckBox(this, ID_CHECKBOX3, _("Backup Subfolders"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX3"));
	BackupSubfoldersCheckBox->SetValue(false);
	GridBagSizer1->Add(BackupSubfoldersCheckBox, wxGBPosition(2, 0), wxGBSpan(1, 2), wxALL|wxEXPAND, 5);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Purge Backups"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	PurgeIntervalChoice = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	PurgeIntervalChoice->SetSelection( PurgeIntervalChoice->Append(_("Never")) );
	PurgeIntervalChoice->Append(_("Older than 360 days"));
	PurgeIntervalChoice->Append(_("Older than 90 days"));
	PurgeIntervalChoice->Append(_("Older than 30 days"));
	PurgeIntervalChoice->Append(_("Older than 7 days"));
	FlexGridSizer1->Add(PurgeIntervalChoice, 1, wxALL|wxEXPAND, 5);
	GridBagSizer1->Add(FlexGridSizer1, wxGBPosition(3, 0), wxDefaultSpan, wxALL|wxEXPAND, 5);
	StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Backup Directory"));
	CheckBox_Backup = new wxCheckBox(this, ID_CHECKBOX4, _("Use Show Folder"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX4"));
	CheckBox_Backup->SetValue(false);
	StaticBoxSizer1->Add(CheckBox_Backup, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	DirPickerCtrl_Backup = new wxDirPickerCtrl(this, ID_DIRPICKERCTRL1, wxEmptyString, wxEmptyString, wxDefaultPosition, wxSize(400,-1), wxDIRP_DIR_MUST_EXIST|wxDIRP_USE_TEXTCTRL, wxDefaultValidator, _T("ID_DIRPICKERCTRL1"));
	StaticBoxSizer1->Add(DirPickerCtrl_Backup, 1, wxALL|wxEXPAND, 5);
	GridBagSizer1->Add(StaticBoxSizer1, wxGBPosition(4, 0), wxDefaultSpan, wxALL|wxEXPAND, 5);
	StaticBoxSizer2 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Alternative Backup Directory"));
	DirPickerCtrl_AltBackup = new wxDirPickerCtrl(this, ID_DIRPICKERCTRL2, wxEmptyString, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDIRP_DIR_MUST_EXIST|wxDIRP_USE_TEXTCTRL, wxDefaultValidator, _T("ID_DIRPICKERCTRL2"));
	StaticBoxSizer2->Add(DirPickerCtrl_AltBackup, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	GridBagSizer1->Add(StaticBoxSizer2, wxGBPosition(5, 0), wxDefaultSpan, wxALL|wxEXPAND, 5);
	SetSizer(GridBagSizer1);
	GridBagSizer1->Fit(this);
	GridBagSizer1->SetSizeHints(this);

	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&BackupSettingsPanel::OnBackupOnSaveCheckBoxClick);
	Connect(ID_CHECKBOX2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&BackupSettingsPanel::OnBackupOnLaunchCheckBoxClick);
	Connect(ID_CHECKBOX3,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&BackupSettingsPanel::OnBackupSubfoldersCheckBoxClick);
	Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&BackupSettingsPanel::OnPurgeIntervalChoiceSelect);
	Connect(ID_CHECKBOX4,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&BackupSettingsPanel::OnCheckBox_BackupClick);
	Connect(ID_DIRPICKERCTRL1,wxEVT_COMMAND_DIRPICKER_CHANGED,(wxObjectEventFunction)&BackupSettingsPanel::OnDirPickerCtrl_BackupDirChanged);
	Connect(ID_DIRPICKERCTRL2,wxEVT_COMMAND_DIRPICKER_CHANGED,(wxObjectEventFunction)&BackupSettingsPanel::OnDirPickerCtrl_AltBackupDirChanged);
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
    bool cb;
    std::string folder;
    frame->GetBackupFolder(cb, folder);
    CheckBox_Backup->SetValue(cb);
    DirPickerCtrl_Backup->SetPath(folder);

    frame->GetAltBackupFolder(folder);
    DirPickerCtrl_AltBackup->SetPath(folder);

    ValidateWindow();

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
    frame->SetBackupFolder(CheckBox_Backup->GetValue(), DirPickerCtrl_Backup->GetPath());
    frame->SetAltBackupFolder(DirPickerCtrl_AltBackup->GetPath());

    ValidateWindow();

    return true;
}

bool BackupSettingsPanel::ValidateWindow()
{
    bool res = true;

    if (CheckBox_Backup->GetValue())
    {
        DirPickerCtrl_Backup->Enable(false);
    }
    else
    {
        if (!wxDir::Exists(DirPickerCtrl_Backup->GetPath())) res = false;
        DirPickerCtrl_Backup->Enable(true);
    }

    if (DirPickerCtrl_AltBackup->GetPath() != "" && !wxDir::Exists(DirPickerCtrl_AltBackup->GetPath())) res = false;

    return res;
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

void BackupSettingsPanel::OnCheckBox_BackupClick(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
    else
    {
        ValidateWindow();
    }
}

void BackupSettingsPanel::OnCheckBox_AltBackupClick(wxCommandEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void BackupSettingsPanel::OnDirPickerCtrl_BackupDirChanged(wxFileDirPickerEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}

void BackupSettingsPanel::OnDirPickerCtrl_AltBackupDirChanged(wxFileDirPickerEvent& event)
{
    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}
