#ifndef BACKUPSETTINGSPANEL_H
#define BACKUPSETTINGSPANEL_H

//(*Headers(BackupSettingsPanel)
#include <wx/panel.h>
class wxCheckBox;
class wxChoice;
class wxDirPickerCtrl;
class wxFlexGridSizer;
class wxGridBagSizer;
class wxStaticBoxSizer;
class wxStaticText;
//*)

#include <wx/filepicker.h>

class xLightsFrame;

class BackupSettingsPanel: public wxPanel
{
	bool ValidateWindow();

	public:

        BackupSettingsPanel(wxWindow* parent, xLightsFrame *frame, wxWindowID id=wxID_ANY);
		virtual ~BackupSettingsPanel();

		//(*Declarations(BackupSettingsPanel)
		wxCheckBox* BackupOnLaunchCheckBox;
		wxCheckBox* BackupOnSaveCheckBox;
		wxCheckBox* BackupSubfoldersCheckBox;
		wxCheckBox* CheckBox_Backup;
		wxChoice* PurgeIntervalChoice;
		wxDirPickerCtrl* DirPickerCtrl_AltBackup;
		wxDirPickerCtrl* DirPickerCtrl_Backup;
		wxStaticText* StaticText1;
		//*)

        virtual bool TransferDataFromWindow() override;
        virtual bool TransferDataToWindow() override;

	protected:

		//(*Identifiers(BackupSettingsPanel)
		static const long ID_CHECKBOX1;
		static const long ID_CHECKBOX2;
		static const long ID_CHECKBOX3;
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE1;
		static const long ID_CHECKBOX4;
		static const long ID_DIRPICKERCTRL1;
		static const long ID_DIRPICKERCTRL2;
		//*)

	private:
        xLightsFrame *frame;
    
		//(*Handlers(BackupSettingsPanel)
		void OnBackupOnSaveCheckBoxClick(wxCommandEvent& event);
		void OnBackupOnLaunchCheckBoxClick(wxCommandEvent& event);
		void OnBackupSubfoldersCheckBoxClick(wxCommandEvent& event);
		void OnPurgeIntervalChoiceSelect(wxCommandEvent& event);
		void OnCheckBox_BackupClick(wxCommandEvent& event);
		void OnCheckBox_AltBackupClick(wxCommandEvent& event);
		void OnDirPickerCtrl_BackupDirChanged(wxFileDirPickerEvent& event);
		void OnDirPickerCtrl_AltBackupDirChanged(wxFileDirPickerEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
