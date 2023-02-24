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
 class wxCheckBox;
 class wxChoice;
 class wxFlexGridSizer;
 class wxPanel;
 class wxStaticText;
 class wxTextCtrl;
 //*)

 #include <wx/treelist.h>
 #include <wx/progdlg.h>

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
		wxCheckBox* CheckBox_ForceHighDefinition;
		wxChoice* FilterChoice;
		wxChoice* FolderChoice;
		wxPanel* CheckListBoxHolder;
		wxStaticText* StaticText3;
		wxTextCtrl* TextCtrl_Selected;
		//*)

		wxTreeListCtrl* CheckListBox_Sequences;

        bool Prepare(const wxString &dir);
        void SequenceListPopup(wxTreeListEvent& event);
        void OnPopupCommand(wxCommandEvent &event);
    
        wxArrayString GetFileList();
        void GetSeqList(const wxString& folder);
        void GetFolderList(const wxString& folder);
        bool isFileInFolder(const wxString &file) const;


protected:
        wxArrayString allFiles;
		wxString showDirectory;

		//(*Identifiers(BatchRenderDialog)
		static const long ID_CHOICE_FILTER;
		static const long ID_CHOICE_FOLDER;
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		static const long ID_CHECKBOX1;
		static const long ID_PANEL_HOLDER;
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
		void OnFolderChoiceSelect(wxCommandEvent& event);
		void OnInit(wxInitDialogEvent& event);
		//*)

		void DisplayDateModified(std::string const& fileName, wxTreeListItem& index) const;

        void ValidateWindow();
        uint32_t UpdateCount();
        void OnSequenceListToggled(wxDataViewEvent& event);
        void SaveSettings();

        DECLARE_EVENT_TABLE()
};
