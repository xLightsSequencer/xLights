#ifndef RENDERPROGRESSDIALOG_H
#define RENDERPROGRESSDIALOG_H

//(*Headers(RenderProgressDialog)
#include <wx/dialog.h>
class wxStdDialogButtonSizer;
class wxScrolledWindow;
class wxFlexGridSizer;
//*)

class RenderProgressDialog: public wxDialog
{
	public:

		RenderProgressDialog(wxWindow* parent);
		virtual ~RenderProgressDialog();

		//(*Declarations(RenderProgressDialog)
		wxFlexGridSizer* scrolledWindowSizer;
		wxScrolledWindow* scrolledWindow;
		//*)

	protected:

		//(*Identifiers(RenderProgressDialog)
		static const long ID_SCROLLEDWINDOW1;
		//*)

	public:

		//(*Handlers(RenderProgressDialog)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
