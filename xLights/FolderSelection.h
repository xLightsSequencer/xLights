#ifndef FOLDERSELECTION_H
#define FOLDERSELECTION_H

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

		FolderSelection(wxWindow* parent, const wxString &showDirectory, const wxString &mediaDirectory, const wxString &fseqDirectory,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~FolderSelection();

		//(*Declarations(FolderSelection)
		wxButton* ButtonFindFSEQDir;
		wxButton* ButtonFindMediaDir;
		wxButton* ButtonFolderSelectCancel;
		wxButton* ButtonFolderSelectOk;
		wxCheckBox* CheckBoxFSEQUseShow;
		wxCheckBox* CheckBoxMediaUseShow;
		wxTextCtrl* TextCtrlFSEQDirectory;
		wxTextCtrl* TextCtrlMediaDirectory;
		//*)

        wxString ShowDirectory;
        wxString MediaDirectory;
        wxString FseqDirectory;
        long LinkMediaDir;
        long LinkFSEQDir;

	protected:

		//(*Identifiers(FolderSelection)
		static const long ID_CHECKBOX_MEDIA_USE_SHOW;
		static const long ID_TEXTCTRL_MEDIA_DIRECTORY;
		static const long ID_BUTTON_FIND_MEDIA_DIR;
		static const long ID_CHECKBOX_FSEQ_USE_SHOW;
		static const long ID_TEXTCTRL_FSEQ_DIRECTORY;
		static const long ID_BUTTON_FIND_FSEQ_DIR;
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
		//*)



		DECLARE_EVENT_TABLE()
};

#endif
