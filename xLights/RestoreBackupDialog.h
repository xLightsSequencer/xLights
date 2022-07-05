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

#include <wx/treelist.h>
#include <wx/treectrl.h>

//(*Headers(RestoreBackupDialog)
#include <wx/button.h>
#include <wx/checklst.h>
#include <wx/dialog.h>
#include <wx/listbox.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/splitter.h>
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
		wxCheckListBox* CheckListBoxLayout;
		wxCheckListBox* CheckListBoxSequences;
		wxListBox* ListBoxBackups;
		wxNotebook* Notebook1;
		wxPanel* Panel1;
		wxSplitterWindow* SplitterWindow1;
		wxSplitterWindow* SplitterWindow2;
		wxStaticText* StaticText1;
		wxStaticText* StaticTextBackUpFolder;
		wxTreeCtrl* TreeCtrlBackupLayout;
		//*)

		wxString GetRestoreFolder() { return _restoreDir; }
		wxStringList GetRestoreFiles() { return _restoreFiles; }

	protected:

		//(*Identifiers(RestoreBackupDialog)
		static const long ID_STATICTEXT1;
		static const long ID_LISTBOX_BACKUPS;
		static const long ID_PANEL1;
		static const long ID_CHECKLISTBOX_LAYOUT;
		static const long ID_TREECTRL_LAYOUT;
		static const long ID_SPLITTERWINDOW2;
		static const long ID_CHECKLISTBOX_SEQUENCES;
		static const long ID_NOTEBOOK1;
		static const long ID_SPLITTERWINDOW1;
		static const long ID_STATICTEXT_BACKUPFOLDER;
		static const long ID_BUTTON_RUN;
		//*)

	private:

		//(*Handlers(RestoreBackupDialog)
		void OnButtonRunClick(wxCommandEvent& event);
		void OnListBoxBackupsSelect(wxCommandEvent& event);
		void OnCheckListBoxLayoutToggled(wxCommandEvent& event);
		void OnCheckListBoxSequencesToggled(wxCommandEvent& event);
		//*)

        DECLARE_EVENT_TABLE()

        wxString _showDir;
        wxString _backupDir;
        wxString _restoreDir;
        wxStringList _restoreFiles;

        void ListBackupDir();
        void PopulateLayoutList(wxString const& folder);
        void PopulateLayoutDataBox(wxString const& folder);
        void PopulateSequenceList(wxString const& folder);
        [[nodiscard]] wxArrayString GetSeqList(wxString const& folder) const;
        [[nodiscard]] std::vector<BController> LoadNetworkFile(wxString const& folder) const;
        [[nodiscard]] std::vector<BModel> LoadRGBEffectsFile(wxString const& folder) const;

        void ValidateWindow();
};

