#ifndef BATCHRENDERDIALOG_H
#define BATCHRENDERDIALOG_H

//(*Headers(BatchRenderDialog)
#include <wx/dialog.h>
class wxStdDialogButtonSizer;
class wxFlexGridSizer;
class wxCheckListBox;
//*)

class wxMouseEvent;
class wxCommandEvent;

class BatchRenderDialog: public wxDialog
{
	public:

		BatchRenderDialog(wxWindow* parent);
		virtual ~BatchRenderDialog();

		//(*Declarations(BatchRenderDialog)
		wxCheckListBox* SequenceList;
		//*)

        bool Prepare(const wxString &dir);
        void OnPreviewRightDown(wxMouseEvent& event);
        void OnPopupCommand(wxCommandEvent &event);
    
        wxArrayString GetFileList();
	protected:

		//(*Identifiers(BatchRenderDialog)
		static const long ID_CHECKLISTBOX_SEQUENCES;
		//*)

	public:

		//(*Handlers(BatchRenderDialog)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
