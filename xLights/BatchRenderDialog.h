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

 //(*Headers(BatchRenderDialog)
#include <wx/dialog.h>
class wxButton;
class wxCheckListBox;
class wxChoice;
class wxFlexGridSizer;
class wxStaticText;
//*)

class wxMouseEvent;
class wxCommandEvent;

class BatchRenderDialog: public wxDialog
{
	public:

		BatchRenderDialog(wxWindow* parent);
		virtual ~BatchRenderDialog();

		//(*Declarations(BatchRenderDialog)
		wxButton* Button_Cancel;
		wxButton* Button_Ok;
		wxCheckListBox* SequenceList;
		wxChoice* FilterChoice;
		wxChoice* FolderChoice;
		//*)

        bool Prepare(const wxString &dir);
        void OnPreviewRightDown(wxMouseEvent& event);
        void OnPopupCommand(wxCommandEvent &event);
    
        wxArrayString GetFileList();
        void GetSeqList(const wxString& folder);
        void GetFolderList(const wxString& folder);
        bool isFileInFolder(const wxString &file) const;


protected:
        wxArrayString allFiles;

		//(*Identifiers(BatchRenderDialog)
		static const long ID_CHOICE_FILTER;
		static const long ID_CHOICE_FOLDER;
		static const long ID_CHECKLISTBOX_SEQUENCES;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)
		
        static const long ID_MNU_SELECTALL;
        static const long ID_MNU_SELECTNONE;
        static const long ID_MNU_SELECTHIGH;
        static const long ID_MNU_DESELECTHIGH;

	public:

		//(*Handlers(BatchRenderDialog)
		void OnFilterChoiceSelect(wxCommandEvent& event);
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnSequenceListToggled(wxCommandEvent& event);
		void OnFolderChoiceSelect(wxCommandEvent& event);
		//*)

        void ValidateWindow();
            
        DECLARE_EVENT_TABLE()
};
