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
class OutputManager;

class BatchRenderDialog: public wxDialog
{
	public:

		BatchRenderDialog(wxWindow* parent, OutputManager* outputManager);
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

        [[nodiscard]] bool Prepare(const wxString& dir);
        void SequenceListPopup(wxTreeListEvent& event);
        void OnPopupCommand(wxCommandEvent &event);
    
        [[nodiscard]] wxArrayString GetFileList() const;
        void GetSeqList(const wxString& folder);
        void GetFolderList(const wxString& folder) const;
        [[nodiscard]] bool isFileInFolder(const wxString& file) const;


protected:
        wxArrayString allFiles;
		wxString showDirectory;

		OutputManager* m_outputManager;

		//(*Identifiers(BatchRenderDialog)
		static const wxWindowID ID_CHOICE_FILTER;
		static const wxWindowID ID_CHOICE_FOLDER;
		static const wxWindowID ID_STATICTEXT1;
		static const wxWindowID ID_TEXTCTRL1;
		static const wxWindowID ID_CHECKBOX1;
		static const wxWindowID ID_PANEL_HOLDER;
		static const wxWindowID ID_BUTTON1;
		static const wxWindowID ID_BUTTON2;
		//*)
		
        static const long ID_MNU_SELECTALL;
        static const long ID_MNU_SELECTNONE;
        static const long ID_MNU_SELECTHIGH;
        static const long ID_MNU_DESELECTHIGH;
        static const long ID_MNU_SELECTFPP;

	public:

		//(*Handlers(BatchRenderDialog)
		void OnFilterChoiceSelect(wxCommandEvent& event);
        void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnFolderChoiceSelect(wxCommandEvent& event);
		void OnInit(wxInitDialogEvent& event);
		//*)

        void DisplayDateModified(std::string const& fileName, wxTreeListItem& index) const;
        void DisplayDateRendered(std::string const& fileName, wxTreeListItem& item) const;

        void ValidateWindow();

        void OnSequenceListToggled(wxDataViewEvent& event);
        void SaveSettings();

        DECLARE_EVENT_TABLE()

	private:
        void SelectFromFPPPlayList();
        uint32_t UpdateCount();
};
