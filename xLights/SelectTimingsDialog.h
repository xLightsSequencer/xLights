#ifndef SELECTTIMINGSDIALOG_H
#define SELECTTIMINGSDIALOG_H

//(*Headers(SelectTimingsDialog)
#include <wx/sizer.h>
#include <wx/checklst.h>
#include <wx/dialog.h>
//*)

class SelectTimingsDialog: public wxDialog
{
	public:

		SelectTimingsDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~SelectTimingsDialog();

		//(*Declarations(SelectTimingsDialog)
		wxCheckListBox* CheckListBox_Timings;
		//*)

	protected:

		//(*Identifiers(SelectTimingsDialog)
		static const long ID_CHECKLISTBOX1;
		//*)

	private:

		//(*Handlers(SelectTimingsDialog)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
