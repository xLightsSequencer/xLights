#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

 //(*Headers(ShowFolderSearchDialog)
 #include <wx/button.h>
 #include <wx/dialog.h>
 #include <wx/listctrl.h>
 #include <wx/sizer.h>
 #include <wx/stattext.h>
 //*)

 #include <wx/treelist.h>
 #include <wx/progdlg.h>

class wxMouseEvent;
class wxCommandEvent;
class xLightsFrame;

class ShowFolderSearchDialog: public wxDialog
{
    xLightsFrame* _xLights = nullptr;
    wxString _selectedFolder;

	public:
		ShowFolderSearchDialog(xLightsFrame* parent);
		virtual ~ShowFolderSearchDialog();

		//(*Declarations(ShowFolderSearchDialog)
		wxButton* ButtonOpen;
		wxButton* ButtonPermanent;
		wxButton* ButtonTemp;
		wxListCtrl* ListCtrl_Folders;
		wxStaticText* StaticTextBackUpFolder;
		//*)

protected:
		//(*Identifiers(ShowFolderSearchDialog)
		static const wxWindowID ID_LISTCTRL_FOLDERS;
		static const wxWindowID ID_STATICTEXT_BACKUPFOLDER;
		static const wxWindowID ID_BUTTON_TEMP;
		static const wxWindowID ID_BUTTON_PERMANENT;
		static const wxWindowID ID_BUTTON_OPEN;
		//*)
		

private:

        void ValidateWindow();
        wxString FindLogFolder() const;
        wxArrayString FindLogFiles(wxString const& folder) const;
        void FindShowFolders(wxArrayString const& logs, wxString const& folder);

    public:
        //(*Handlers(ShowFolderSearchDialog)
        void OnInit(wxInitDialogEvent& event);
        void OnListCtrl_FoldersItemSelect(wxListEvent& event);
        void OnButtonTempClick(wxCommandEvent& event);
        void OnButtonPermanentClick(wxCommandEvent& event);
        void OnButtonOpenClick(wxCommandEvent& event);
        //*)
        DECLARE_EVENT_TABLE()
};
