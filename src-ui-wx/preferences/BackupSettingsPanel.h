#pragma once

/***************************************************************
 * This source file comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/panel.h>
#include <wx/filepicker.h>

class wxCheckBox;
class wxChoice;
class wxDirPickerCtrl;
class wxCommandEvent;
class xLightsFrame;

class BackupSettingsPanel: public wxPanel
{
	bool ValidateWindow();

	public:

        BackupSettingsPanel(wxWindow* parent, xLightsFrame *frame, wxWindowID id=wxID_ANY);
		virtual ~BackupSettingsPanel();

        virtual bool TransferDataFromWindow() override;
        virtual bool TransferDataToWindow() override;

	private:
        xLightsFrame *frame;

        wxCheckBox* BackupOnLaunchCheckBox = nullptr;
        wxCheckBox* BackupOnSaveCheckBox = nullptr;
        wxCheckBox* BackupSubfoldersCheckBox = nullptr;
        wxCheckBox* CheckBox_Backup = nullptr;
        wxChoice* PurgeIntervalChoice = nullptr;
        wxDirPickerCtrl* DirPickerCtrl_AltBackup = nullptr;
        wxDirPickerCtrl* DirPickerCtrl_Backup = nullptr;

        void OnBackupOnSaveCheckBoxClick(wxCommandEvent& event);
        void OnBackupOnLaunchCheckBoxClick(wxCommandEvent& event);
        void OnBackupSubfoldersCheckBoxClick(wxCommandEvent& event);
        void OnPurgeIntervalChoiceSelect(wxCommandEvent& event);
        void OnCheckBox_BackupClick(wxCommandEvent& event);
        void OnDirPickerCtrl_BackupDirChanged(wxFileDirPickerEvent& event);
        void OnDirPickerCtrl_AltBackupDirChanged(wxFileDirPickerEvent& event);
};
