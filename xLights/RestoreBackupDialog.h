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

//(*Headers(RestoreBackupDialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/listbox.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/treectrl.h>
//*)

struct BController;
struct BModel;

class RestoreBackupDialog: public wxDialog
{
	public:

		RestoreBackupDialog(wxString const& showDir, wxString const& backupDir, wxWindow* parent, wxWindowID id = wxID_ANY);
		virtual ~RestoreBackupDialog();

		//(*Declarations(RestoreBackupDialog)
		wxButton* ButtonRun;
		wxListBox* ListBoxBackups;
		wxStaticText* StaticTextBackUpFolder;
		wxStaticText* StaticTextShowFolder;
		wxTreeCtrl* TreeCtrlBackupData;
		//*)

		wxString GetRestoreDir() { return _restoreDir; }

	protected:

		//(*Identifiers(RestoreBackupDialog)
		static const long ID_LISTBOX_BACKUPS;
		static const long ID_TREECTRL_BACKUP_DATA;
		static const long ID_BUTTON_RUN;
		static const long ID_STATICTEXT_BACKUPFOLDER;
		static const long ID_STATICTEXT_SHOWFOLDER;
		//*)

	private:

		//(*Handlers(RestoreBackupDialog)
		void OnButtonRunClick(wxCommandEvent& event);
		void OnListBoxBackupsSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()

		wxString _showDir;
        wxString _backupDir;
		wxString _restoreDir;

        void ListBackupDir();
        void PopulateDataBox(wxString const& folder);
        [[nodiscard]] wxArrayString GetSeqList(wxString const& folder) const;
        [[nodiscard]] std::vector<BController> LoadNetworkFile(wxString const& folder) const;
        [[nodiscard]] std::vector<BModel> LoadRGBEffectsFile(wxString const& folder) const;

		void AddFolder(wxString path, wxTreeItemId parent);
        wxTreeItemId FindTreeItem(wxTreeItemId parent, wxString name) const;
};

