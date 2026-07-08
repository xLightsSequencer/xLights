/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "BackupSettingsPanel.h"
#include "PrefPanelUtils.h"

#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/dir.h>
#include <wx/filepicker.h>
#include <wx/intl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/string.h>

#include <wx/preferences.h>
#include "xLightsMain.h"

BackupSettingsPanel::BackupSettingsPanel(wxWindow* parent, xLightsFrame *f, wxWindowID id) : frame(f)
{
    Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));

    auto* sizer = new wxBoxSizer(wxVERTICAL);

    BackupOnSaveCheckBox = new wxCheckBox(this, wxID_ANY, _("Backup On Save"));
    sizer->Add(BackupOnSaveCheckBox, 0, wxLEFT | wxTOP, 5);
    sizer->Add(MakePreferenceHint(this, _("Make a backup copy of the show folder every time you save.")), 0, wxLEFT | wxBOTTOM, 24);

    BackupOnLaunchCheckBox = new wxCheckBox(this, wxID_ANY, _("Backup On Launch"));
    sizer->Add(BackupOnLaunchCheckBox, 0, wxLEFT | wxTOP, 5);
    sizer->Add(MakePreferenceHint(this, _("Make a backup copy of the show folder each time xLights starts.")), 0, wxLEFT | wxBOTTOM, 24);

    BackupSubfoldersCheckBox = new wxCheckBox(this, wxID_ANY, _("Backup Subfolders"));
    sizer->Add(BackupSubfoldersCheckBox, 0, wxLEFT | wxTOP, 5);
    sizer->Add(MakePreferenceHint(this, _("Include the show folder's subfolders in the backup.")), 0, wxLEFT | wxBOTTOM, 24);

    auto* purgeRow = new wxBoxSizer(wxHORIZONTAL);
    purgeRow->Add(new wxStaticText(this, wxID_ANY, _("Purge Backups")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    PurgeIntervalChoice = new wxChoice(this, wxID_ANY);
    PurgeIntervalChoice->SetSelection(PurgeIntervalChoice->Append(_("Never")));
    PurgeIntervalChoice->Append(_("Older than 360 days"));
    PurgeIntervalChoice->Append(_("Older than 90 days"));
    PurgeIntervalChoice->Append(_("Older than 30 days"));
    PurgeIntervalChoice->Append(_("Older than 7 days"));
    purgeRow->Add(PurgeIntervalChoice, 0, wxEXPAND);
    sizer->Add(purgeRow, 0, wxLEFT | wxTOP, 5);
    sizer->Add(MakePreferenceHint(this, _("Automatically delete backups older than the chosen age.")), 0, wxLEFT | wxBOTTOM, 5);

    sizer->Add(MakePreferenceSectionHeader(this, _("Backup Directory")), 0, wxLEFT | wxTOP, 10);
    sizer->Add(MakePreferenceHint(this, _("Where backups are written. Use the show folder, or untick to choose a custom location.")), 0, wxLEFT, 16);
    auto* backupRow = new wxBoxSizer(wxHORIZONTAL);
    CheckBox_Backup = new wxCheckBox(this, wxID_ANY, _("Use Show Folder"));
    backupRow->Add(CheckBox_Backup, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    DirPickerCtrl_Backup = new wxDirPickerCtrl(this, wxID_ANY, wxEmptyString, wxEmptyString, wxDefaultPosition, wxSize(400, -1), wxDIRP_DIR_MUST_EXIST | wxDIRP_USE_TEXTCTRL);
    backupRow->Add(DirPickerCtrl_Backup, 1, wxEXPAND);
    sizer->Add(backupRow, 0, wxEXPAND | wxLEFT | wxTOP | wxBOTTOM, 16);

    sizer->Add(MakePreferenceSectionHeader(this, _("Alternative Backup Directory")), 0, wxLEFT | wxTOP, 10);
    sizer->Add(MakePreferenceHint(this, _("Optional second location backups are also copied to (e.g. an external or cloud drive).")), 0, wxLEFT, 16);
    DirPickerCtrl_AltBackup = new wxDirPickerCtrl(this, wxID_ANY, wxEmptyString, wxEmptyString, wxDefaultPosition, wxSize(400, -1), wxDIRP_DIR_MUST_EXIST | wxDIRP_USE_TEXTCTRL);
    sizer->Add(DirPickerCtrl_AltBackup, 0, wxEXPAND | wxLEFT | wxTOP | wxBOTTOM, 16);

    SetSizer(sizer);
    sizer->SetSizeHints(this);

    BackupOnSaveCheckBox->Bind(wxEVT_CHECKBOX, &BackupSettingsPanel::OnBackupOnSaveCheckBoxClick, this);
    BackupOnLaunchCheckBox->Bind(wxEVT_CHECKBOX, &BackupSettingsPanel::OnBackupOnLaunchCheckBoxClick, this);
    BackupSubfoldersCheckBox->Bind(wxEVT_CHECKBOX, &BackupSettingsPanel::OnBackupSubfoldersCheckBoxClick, this);
    PurgeIntervalChoice->Bind(wxEVT_CHOICE, &BackupSettingsPanel::OnPurgeIntervalChoiceSelect, this);
    CheckBox_Backup->Bind(wxEVT_CHECKBOX, &BackupSettingsPanel::OnCheckBox_BackupClick, this);
    DirPickerCtrl_Backup->Bind(wxEVT_DIRPICKER_CHANGED, &BackupSettingsPanel::OnDirPickerCtrl_BackupDirChanged, this);
    DirPickerCtrl_AltBackup->Bind(wxEVT_DIRPICKER_CHANGED, &BackupSettingsPanel::OnDirPickerCtrl_AltBackupDirChanged, this);

#ifdef _MSC_VER
    MSWDisableComposited();
#endif
}

BackupSettingsPanel::~BackupSettingsPanel()
{
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
