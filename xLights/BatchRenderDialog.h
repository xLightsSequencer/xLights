#ifndef BATCHRENDERDIALOG_H
#define BATCHRENDERDIALOG_H

//(*Headers(BatchRenderDialog)
#include <wx/dialog.h>
class wxStaticText;
class wxFlexGridSizer;
class wxButton;
class wxCheckListBox;
class wxChoice;
//*)

class wxMouseEvent;
class wxCommandEvent;

class BatchRenderDialog: public wxDialog
{
	public:

		BatchRenderDialog(wxWindow* parent);
		virtual ~BatchRenderDialog();

		//(*Declarations(BatchRenderDialog)
		wxButton* Button_Ok;
		wxChoice* FilterChoice;
		wxButton* Button_Cancel;
		wxCheckListBox* SequenceList;
		//*)

        bool Prepare(const wxString &dir);
        void OnPreviewRightDown(wxMouseEvent& event);
        void OnPopupCommand(wxCommandEvent &event);
    
        wxArrayString GetFileList();
	protected:
        wxArrayString allFiles;

		//(*Identifiers(BatchRenderDialog)
		static const long ID_CHOICE_FILTER;
		static const long ID_CHECKLISTBOX_SEQUENCES;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	public:

		//(*Handlers(BatchRenderDialog)
		void OnFilterChoiceSelect(wxCommandEvent& event);
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnSequenceListToggled(wxCommandEvent& event);
		//*)

        void ValidateWindow();
            
        DECLARE_EVENT_TABLE()
};

#endif
