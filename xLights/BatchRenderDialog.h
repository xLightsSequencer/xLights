#ifndef BATCHRENDERDIALOG_H
#define BATCHRENDERDIALOG_H

//(*Headers(BatchRenderDialog)
#include <wx/dialog.h>
class wxStdDialogButtonSizer;
class wxStaticText;
class wxFlexGridSizer;
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
		wxChoice* FilterChoice;
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
		//*)

	public:

		//(*Handlers(BatchRenderDialog)
		void OnFilterChoiceSelect(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
