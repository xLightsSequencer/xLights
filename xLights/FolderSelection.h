#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

//(*Headers(FolderSelection)
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
//*)

class FolderSelection: public wxDialog
{
	public:

		FolderSelection(wxWindow* parent, const wxString &showDirectory, const wxString &mediaDirectory, const wxString &fseqDirectory, const wxString& renderCacheDirectory,
			const wxString &backupDirectory, const wxString &altBackupDirectory, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
		virtual ~FolderSelection();

		//(*Declarations(FolderSelection)
		wxButton* ButtonFindAltBackupDirectory;
		wxButton* ButtonFindBackupDirectory;
		wxButton* ButtonFindFSEQDir;
		wxButton* ButtonFindMediaDir;
		wxButton* ButtonFindRenderCacheDir;
		wxButton* ButtonFolderSelectCancel;
		wxButton* ButtonFolderSelectOk;
		wxCheckBox* CheckBoxBackupUseShow;
		wxCheckBox* CheckBoxEnableAltBackup;
		wxCheckBox* CheckBoxFSEQUseShow;
		wxCheckBox* CheckBoxMediaUseShow;
		wxCheckBox* CheckBoxRenderCacheUseShow;
		wxTextCtrl* TextCtrlAltBackupDirectory;
		wxTextCtrl* TextCtrlBackupDirectory;
		wxTextCtrl* TextCtrlFSEQDirectory;
		wxTextCtrl* TextCtrlMediaDirectory;
		wxTextCtrl* TextCtrlRenderCacheDirectory;
		//*)

        wxString ShowDirectory;
        wxString MediaDirectory;
        wxString FseqDirectory;
        wxString RenderCacheDirectory;
        wxString BackupDirectory;
        wxString AltBackupDirectory;
        long LinkMediaDir;

	protected:

		//(*Identifiers(FolderSelection)
		static const long ID_CHECKBOX_MEDIA_USE_SHOW;
		static const long ID_TEXTCTRL_MEDIA_DIRECTORY;
		static const long ID_BUTTON_FIND_MEDIA_DIR;
		static const long ID_CHECKBOX_FSEQ_USE_SHOW;
		static const long ID_TEXTCTRL_FSEQ_DIRECTORY;
		static const long ID_BUTTON_FIND_FSEQ_DIR;
		static const long ID_CHECKBOX_RENDERCACHE_USE_SHOW;
		static const long ID_TEXTCTRL_RENDERCACHE_DIRECTORY;
		static const long ID_BUTTON_FIND_RENDERCACHE_DIR;
		static const long ID_CHECKBOX_BACKUP_USE_SHOW;
		static const long ID_TEXTCTRL_BACKUP_DIRECTORY;
		static const long ID_BUTTON_FIND_BACKUP_DIRECTORY;
		static const long ID_CHECKBOX_ENABLE_ALT_BACKUP;
		static const long ID_TEXTCTRL_ALT_BACKUP_DIRECTORY;
		static const long ID_BUTTON_FIND_ALT_BACKUP_DIRECTORY;
		static const long ID_BUTTON_FOLDER_SELECT_OK;
		static const long ID_BUTTON_FOLDER_SELECT_CANCEL;
		//*)

	private:

		//(*Handlers(FolderSelection)
		void OnButtonFolderSelectOkClick(wxCommandEvent& event);
		void OnButtonFolderSelectCancelClick(wxCommandEvent& event);
		void OnButtonFindMediaDirClick(wxCommandEvent& event);
		void OnButtonFindFSEQDirClick(wxCommandEvent& event);
		void OnCheckBoxFSEQUseShowClick(wxCommandEvent& event);
		void OnCheckBoxMediaUseShowClick(wxCommandEvent& event);
		void OnCheckBoxBackupUseShowClick(wxCommandEvent& event);
		void OnButtonFindBackupDirectoryClick(wxCommandEvent& event);
		void OnTextCtrlMediaDirectoryText(wxCommandEvent& event);
		void OnTextCtrlFSEQDirectoryText(wxCommandEvent& event);
		void OnTextCtrlBackupDirectoryText(wxCommandEvent& event);
		void OnCheckBoxEnableAltBackupClick(wxCommandEvent& event);
		void OnTextCtrlAltBackupDirectoryText(wxCommandEvent& event);
		void OnButtonFindAltBackupDirectoryClick(wxCommandEvent& event);
		void OnTextCtrlRenderCacheDirectoryText(wxCommandEvent& event);
		void OnCheckBoxRenderCacheUseShowClick(wxCommandEvent& event);
		void OnButtonFindRenderCacheDirClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
