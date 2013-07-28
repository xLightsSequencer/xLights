#ifndef CUSTOMMODELDIALOG_H
#define CUSTOMMODELDIALOG_H

//(*Headers(CustomModel)
#include <wx/sizer.h>
#include <wx/grid.h>
#include <wx/dialog.h>
//*)

class CustomModelDialog: public wxDialog
{
	public:

		CustomModelDialog(wxWindow* parent,wxWindowID id=wxID_ANY);
		virtual ~CustomModelDialog();

		//(*Declarations(CustomModel)
		wxGrid* gdModelChans;
		//*)

	protected:

		//(*Identifiers(CustomModel)
		static const long ID_GRID1;
		//*)

	private:

		//(*Handlers(CustomModel)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
