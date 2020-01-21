#ifndef BACKUPSETTINGSPANEL_H
#define BACKUPSETTINGSPANEL_H

//(*Headers(BackupSettingsPanel)
#include <wx/panel.h>
class wxCheckBox;
class wxChoice;
class wxGridBagSizer;
class wxStaticText;
//*)

class xLightsFrame;

class BackupSettingsPanel: public wxPanel
{
	public:

        BackupSettingsPanel(wxWindow* parent, xLightsFrame *frame, wxWindowID id=wxID_ANY);
		virtual ~BackupSettingsPanel();

		//(*Declarations(BackupSettingsPanel)
		wxCheckBox* BackupOnLaunchCheckBox;
		wxCheckBox* BackupOnSaveCheckBox;
		wxCheckBox* BackupSubfoldersCheckBox;
		wxChoice* PurgeIntervalChoice;
		wxStaticText* StaticText1;
		//*)

        virtual bool TransferDataFromWindow() override;
        virtual bool TransferDataToWindow() override;

	protected:

		//(*Identifiers(BackupSettingsPanel)
		static const long ID_CHECKBOX1;
		static const long ID_CHECKBOX2;
		static const long ID_CHECKBOX3;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT1;
		//*)

	private:
        xLightsFrame *frame;
    
		//(*Handlers(BackupSettingsPanel)
		void OnBackupOnSaveCheckBoxClick(wxCommandEvent& event);
		void OnBackupOnLaunchCheckBoxClick(wxCommandEvent& event);
		void OnBackupSubfoldersCheckBoxClick(wxCommandEvent& event);
		void OnPurgeIntervalChoiceSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
